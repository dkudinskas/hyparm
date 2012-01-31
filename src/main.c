#ifdef CONFIG_CLI
#include "cli/cli.h"
#endif

#include "common/debug.h"
#include "common/memFunctions.h"
#include "common/stdarg.h"
#include "common/string.h"

#include "cpuArch/cpu.h"

#include "drivers/beagle/beIntc.h"
#include "drivers/beagle/beGPTimer.h"
#include "drivers/beagle/beClockMan.h"
#include "drivers/beagle/beUart.h"

#ifdef CONFIG_GUEST_FREERTOS
#include "drivers/beagle/beGPIO.h"
#endif

#ifdef CONFIG_MMC
#include "drivers/beagle/beMMC.h"
#endif

#include "guestManager/guestContext.h"
#include "guestManager/blockCache.h"

#include "instructionEmu/scanner.h"

#ifdef CONFIG_MMC
#include "io/mmc.h"
#include "io/partitions.h"
#include "io/fs/fat.h"
#endif

#ifdef CONFIG_GUEST_FREERTOS
#include "guestBoot/freertos.h"
#endif
#include "guestBoot/linux.h"
#include "guestBoot/image.h"

#include "memoryManager/addressing.h" /* For virtual addressing initialisation */
#include "memoryManager/cp15coproc.h"
#include "memoryManager/frameAllocator.h"

#include "vm/omap35xx/hardwareLibrary.h"
#include "vm/omap35xx/LED.h"


//uncomment me to enable block copy cache debug (installation of backpointer): #define BLOCK_COPY_DEBUG

#define HIDDEN_RAM_START   0x8f000000
#define HIDDEN_RAM_SIZE    0x01000000 // 16 MB

extern void setGuestContext(GCONTXT *gContext);

static void printUsage(void);
static int parseCommandline(int argc, char *argv[], u32int *kernAddr, u32int *initrdAddr);

#ifdef CONFIG_MMC
fatfs mainFilesystem;
partitionTable primaryPartitionTable;
struct mmc *mmcDevice;
file * debugStream;
#endif

#ifdef CONFIG_GUEST_FREERTOS
bool rtos;
#endif


void main(s32int argc, char *argv[])
{
  u32int kernAddr = 0;
  u32int initrdAddr = 0;
  int ret = 0;

#ifdef CONFIG_GUEST_FREERTOS
  rtos = FALSE;
#endif

  /* save power: cut the clocks to the display subsystem */
  cmDisableDssClocks();

  mallocInit(HIDDEN_RAM_START, HIDDEN_RAM_SIZE);

  /* initialise uart backend, important to be before any debug output. */
  /* init function initialises UARTs in disabled mode. */
  /* startup fuction starts operation and enables RX IRQ generation */
  beUartInit(1);
  beUartInit(2);
  beUartInit(3);
#ifdef CONFIG_UART_FAST
  beUartStartup(3, 500000);
#else
  beUartStartup(3, 115200);
#endif

#ifndef CONFIG_CLI
  if ((ret = parseCommandline(argc, argv, &kernAddr, &initrdAddr)) < 0)
  {
    printUsage();
    DIE_NOW(0, "Hypervisor startup aborted.");
  }
#endif /* CONFIG_CLI */

  /* create the frametable from which we can alloc memory */
  initialiseFrameTable();

  /* initialize guest context */
  GCONTXT *context = allocateGuest();
  setGuestContext(context);

#ifdef CONFIG_BLOCK_COPY
  //Install jump instruction
  /* initialise block(copy) cache -> place for copied instructions*/
  u32int * blockCopyCache = (u32int*)mallocBytes(BLOCK_COPY_CACHE_SIZE_IN_BYTES);  //BLOCK_COPY_CACHE_SIZE_IN_BYTES
  if (blockCopyCache == 0)
  {
    DIE_NOW(0, "Failed to allocate block copy cache.");
  }
  else
  {
    memset((void*)blockCopyCache, 0x0, BLOCK_COPY_CACHE_SIZE_IN_BYTES);
    u32int blockCopyCacheSize= BLOCK_COPY_CACHE_SIZE -1;
    //Jump offset is in multiples of 4 (thus words) (see ARM ARM p.357)
    //And PC is 2 words behind & When jump is taken it means that instruction before jump is not critical and that block is not finished.
    //This will result in a continuation of the block at the beginning of blockCopyCache. (-> there is a backpointer but the initial block will
    //be removed to make place for the rest of the block that was being scanned!).
    u32int jumpOffset=blockCopyCacheSize+2;
    u32int index = 0;
    u32int result = 0;
    int i;
    while((jumpOffset & (1<<index)) == 0){//Find index of first bit that is set to 1
#ifdef BLOCK_COPY_DEBUG
    serial_putint((1<<index));
    serial_putstring(",");
    serial_newline();
#endif
      index ++;
    }
#ifdef BLOCK_COPY_DEBUG
    serial_putstring("Index of first 1bit");
    serial_putint((u32int)index);
    serial_newline();
#endif
    //unconditional branch => first 8 bits = 11101010 = 234 = EA
    result=234;
#ifdef BLOCK_COPY_DEBUG
    serial_putstring("Bits inverted:");
    serial_newline();
#endif
    for(i=23;i>=0;i--){
      result = result<<1;

      if(i>index){
        //add bits inverted
        //result += (1-(jumpOffset & 1<<i)); -> ! wrong result -> jumpOffset & 1<<i can be 1,2,4,8,...
        result += (1-(jumpOffset>>i & 1));
#ifdef BLOCK_COPY_DEBUG
    serial_putstring("i");
    serial_putint_nozeros((1-(jumpOffset>>i & 1)));
#endif
      }else{
        //add bits normal
        result += (jumpOffset>>i & 1);
#ifdef BLOCK_COPY_DEBUG
    serial_putstring("n");
    serial_putint_nozeros((jumpOffset>>i & 1));
#endif
      }

    }
#ifdef BLOCK_COPY_DEBUG
    serial_newline();
#endif
    //Install unconditional jump to beginning of blockCopyCache at end of blockCopyCache
    *(blockCopyCache + blockCopyCacheSize)=result;
#ifdef STARTUP_DEBUG
    serial_putstring("Block copy cache at 0x");
    serial_putint((u32int)blockCopyCache);
    serial_newline();
#endif
  }
  registerBlockCopyCache(context, blockCopyCache, BLOCK_COPY_CACHE_SIZE);
#endif //CONFIG_BLOCK_COPY

  /* Setup MMU for Hypervisor */
  initialiseVirtualAddressing();

#ifdef CONFIG_CLI
  /*
   * FIXME: deal with guest context and VIRTUAL ADDRESSING!!!
   */
  enterCliLoop();
#else

  /* initialise physical interrupt controller */
  intcBEInit();

  /* now we can unmask first interrupt - UART */
  unmaskInterruptBE(UART3_IRQ);

  /* initialise physical clock manager */
  clkManBEInit();

  /* initialise phyiscal GPT2, dedicated to guest1 */
  gptBEInit(2);

#ifdef CONFIG_MMC
  u32int err = 0;
  if ((err = mmcMainInit()) != 0)
  {
    DIE_NOW(context, "Failed to initialize mmc code.\n");
  }

  if ((err = partTableRead(&mmcDevice->blockDev, &primaryPartitionTable)) != 0)
  {
    DIE_NOW(context, "Failed to read partition table.\n");
  }

  if ((err = fatMount(&mainFilesystem, &mmcDevice->blockDev, 1)) != 0)
  {
    DIE_NOW(context, "Failed to mount FAT partition.\n");
  }

  debugStream = fopen(&mainFilesystem, "debug");
  if (debugStream == 0)
  {
    DIE_NOW(context, "Failed to open (create) debug stream file.\n");
  }
#endif /* CONFIG_MMC */

#ifdef CONFIG_BLOCK_COPY_NO_IRQ
  disableInterrupts();
#endif

  // does not return
#ifdef CONFIG_GUEST_FREERTOS
  if (ret == 2)
  {
    DEBUG(STARTUP, "RTOS address: %x\n", kernAddr);
    rtos = TRUE;
    bootFreeRtos(context, kernAddr);
  }
  else
#endif
  {
    DEBUG(STARTUP, "Kernel address: %x, Initrd address: %x" EOL, kernAddr, initrdAddr);
    bootLinux(context, kernAddr, initrdAddr);
  }
#endif /* CONFIG_CLI */
}

static void printUsage(void)
{
  printf(
      "Loader usage:" EOL
      "go <loaderAddr> -kernel <kernAddress> -initrd <initrdAddr>" EOL
      "kernel: address of kernel in hexadecimal representation" EOL
      "initrd: address of external initrd in hexadecimal representation" EOL
#ifdef CONFIG_GUEST_FREERTOS
      "rtos: address of rtos in hex format (0xXXXXXXXX)" EOL
#endif
    );
  return;
}

static int parseCommandline(s32int argc, char *argv[], u32int *kernAddr, u32int *initrdAddr)
{
  s32int i;
#ifdef CONFIG_GUEST_FREERTOS
  bool rtos;
#endif

  DEBUG(STARTUP, "parseCommandline: argc = %d" EOL, argc);
  for (i = 0; i < argc; ++i)
  {
    DEBUG(STARTUP, "parseCommandline: argv[%d] = %p" EOL, i, argv[i]);
  }

  if (argc < 3)
  {
    return -1;
  }

  if (strcmp("-kernel", argv[1]) == 0)
  {
#ifdef CONFIG_GUEST_FREERTOS
    rtos = FALSE;
  }
  else if (strcmp("-rtos", argv[1]) == 0)
  {
    rtos = TRUE;
#endif
  }
  else
  {
    DEBUG(STARTUP, "parseCommandline: parameter -kernel or -rtos not found" EOL);
    return -1;
  }

  if (sscanf(argv[2], "%x", kernAddr) != 1)
  {
    DEBUG(STARTUP, "parseCommandline: parameter value for %s is not a valid address: '%s'" EOL, argv[1], argv[2]);
    return -1;
  }

#ifdef CONFIG_GUEST_FREERTOS
  if (rtos)
  {
    return 2;
  }
#endif

  if (argc != 5)
  {
    return -1;
  }

  if (strcmp("-initrd", argv[3]) != 0)
  {
    DEBUG(STARTUP, "parseCommandline: parameter -initrd not found" EOL);
    return -1;
  }

  if (sscanf(argv[4], "%x", initrdAddr) != 1)
  {
    DEBUG(STARTUP, "parseCommandline: parameter value for -initrd is not a valid address: '%s'" EOL, argv[4]);
    return -1;
  }

  return 1;
}

