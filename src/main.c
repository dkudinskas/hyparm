#ifdef CONFIG_CLI
#include "cli/cli.h"
#endif

#include "common/commandLine.h"
#include "common/debug.h"
#include "common/memFunctions.h"
#include "common/stdarg.h"
#include "common/stddef.h"
#include "common/string.h"

#include "cpuArch/armv7.h"

#include "drivers/beagle/beIntc.h"
#include "drivers/beagle/beGPTimer.h"
#include "drivers/beagle/beClockMan.h"
#include "drivers/beagle/beUart.h"

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


//uncomment me to enable block copy cache debug (installation of backpointer): #define BLOCK_COPY_DEBUG

#define HIDDEN_RAM_START   0x8f000000
#define HIDDEN_RAM_SIZE    0x01000000 // 16 MB

#define CL_OPTION_FAST_UART          1
#define CL_OPTION_GUEST_OS           2
#define CL_OPTION_GUEST_KERNEL       3
#define CL_OPTION_GUEST_INITRD       4

#define CL_VALUE_GUEST_OS_FREERTOS   "freertos"
#define CL_VALUE_GUEST_OS_LINUX      "linux"


struct runtimeConfiguration
{
  enum guestOSType guestOS;
  u32int guestKernelAddress;
  u32int guestInitialRAMDiskAddress;
};


static void dumpRuntimeConfiguration(struct runtimeConfiguration *config);
static void processCommandLine(struct runtimeConfiguration *config, s32int argc, char *argv[]);
static bool stringToAddress(const char *str, u32int *address);

extern void setGuestContext(GCONTXT *gContext);


#ifdef CONFIG_MMC
fatfs mainFilesystem;
partitionTable primaryPartitionTable;
struct mmc *mmcDevice;
file * debugStream;
#endif


static void dumpRuntimeConfiguration(struct runtimeConfiguration *config)
{
  DEBUG(STARTUP, "Guest OS: %#x" EOL, config->guestOS);
  DEBUG(STARTUP, "Guest OS kernel address: %#.8x" EOL, config->guestKernelAddress);
  DEBUG(STARTUP, "Guest OS initial RAM disk address: %#.8x" EOL, config->guestInitialRAMDiskAddress);
}

void main(s32int argc, char *argv[])
{
  struct runtimeConfiguration config;
  memset(&config, 0, sizeof(struct runtimeConfiguration));
  config.guestOS = GUEST_OS_LINUX;

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

  /*
   * Use command line arguments passed by U-Boot to update the runtime configuration structure. The
   * first argument is always the load address of the hypervisor; skip it.
   */
  processCommandLine(&config, argc - 1, argv + 1);
  dumpRuntimeConfiguration(&config);

  /* create the frametable from which we can alloc memory */
  initialiseFrameTable();

  /* initialize guest context */
  GCONTXT *context = createGuestContext();
  setGuestContext(context);

#ifdef CONFIG_BLOCK_COPY
  //Install jump instruction
  /* initialise block(copy) cache -> place for copied instructions*/
  u32int * blockCopyCache = (u32int*)mallocBytes(BLOCK_COPY_CACHE_SIZE_IN_BYTES);  //BLOCK_COPY_CACHE_SIZE_IN_BYTES
  if (blockCopyCache == 0)
  {
    DIE_NOW(NULL, "Failed to allocate block copy cache.");
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
    DIE_NOW(context, "Failed to initialize mmc code.");
  }

  if ((err = partTableRead(&mmcDevice->blockDev, &primaryPartitionTable)) != 0)
  {
    DIE_NOW(context, "Failed to read partition table.");
  }

  if ((err = fatMount(&mainFilesystem, &mmcDevice->blockDev, 1)) != 0)
  {
    DIE_NOW(context, "Failed to mount FAT partition.");
  }

  debugStream = fopen(&mainFilesystem, "debug");
  if (debugStream == 0)
  {
    DIE_NOW(context, "Failed to open (create) debug stream file.");
  }
#endif /* CONFIG_MMC */

#ifdef CONFIG_BLOCK_COPY_NO_IRQ
  disableInterrupts();
#endif

  // does not return
  switch (config.guestOS)
  {
#ifdef CONFIG_GUEST_FREERTOS
    case GUEST_OS_FREERTOS:
      bootFreeRtos(context, config.guestKernelAddress);
      break;
#endif
    case GUEST_OS_LINUX:
      bootLinux(context, config.guestKernelAddress, config.guestInitialRAMDiskAddress);
      break;
    default:
      printf("Error: unsupported guest OS %#x", config.guestOS);
      break;
  }
  DIE_NOW(context, "guest boot failed");

#endif /* CONFIG_CLI */
}

static void processCommandLine(struct runtimeConfiguration *config, s32int argc, char *argv[])
{
  bool success = TRUE;
  struct commandLine *commandLine, *p;
  struct commandLineOption *options = NULL;
  options = addCommandLineOption(options, "guest", "Guest operating system type", TRUE, FALSE, CL_OPTION_GUEST_OS);
  options = addCommandLineOption(options, "kernel", "Address of the kernel in memory", TRUE, TRUE, CL_OPTION_GUEST_KERNEL);
  options = addCommandLineOption(options, "initrd", "Address of an initial RAM disk in memory", TRUE, FALSE, CL_OPTION_GUEST_INITRD);
  commandLine = parseCommandLine(options, argc, argv);
  for (p = commandLine; p; p = p->next)
  {
    switch (p->argumentId)
    {
      case CL_OPTION_GUEST_OS:
        if (config->guestOS)
        {
          printf("Error: duplicate option: guest OS '%s'" EOL, p->value);
          success = FALSE;
        }
        else if (strcmp(p->value, CL_VALUE_GUEST_OS_FREERTOS))
        {
          config->guestOS = GUEST_OS_FREERTOS;
        }
        else if (strcmp(p->value, CL_VALUE_GUEST_OS_LINUX))
        {
          config->guestOS = GUEST_OS_LINUX;
        }
        else
        {
          printf("Error: invalid guest OS '%s'" EOL, p->value);
          success = FALSE;
        }
        break;
      case CL_OPTION_GUEST_KERNEL:
        if (config->guestKernelAddress)
        {
          printf("Error: duplicate option: kernel address '%s'" EOL, p->value);
          success = FALSE;
        }
        else if (!stringToAddress(p->value, &(config->guestKernelAddress)) || !config->guestKernelAddress)
        {
          printf("Error: invalid kernel address '%s'" EOL, p->value);
          success = FALSE;
        }
        break;
      case CL_OPTION_GUEST_INITRD:
        if (config->guestInitialRAMDiskAddress)
        {
          printf("Error: duplicate option: RAM disk address '%s'" EOL, p->value);
          success = FALSE;
        }
        else if (!stringToAddress(p->value, &(config->guestInitialRAMDiskAddress)) || !config->guestInitialRAMDiskAddress)
        {
          printf("Error: invalid initial RAM disk address '%s'" EOL, p->value);
          success = FALSE;
        }
        break;
      default:
        printf("Error: unrecognized option '%s'" EOL, p->value);
        success = FALSE;
        break;
    }
  }
  freeCommandLine(commandLine);
  if (!success)
  {
    printCommandLineHelp(options);
    DIE_NOW(NULL, "bad command line");
  }
  freeCommandLineOptions(options);
}

static bool stringToAddress(const char *str, u32int *address)
{
  u32int value;
  s32int length;
  if (sscanf(str, "%x%n", &value, &length) == 1 && (u32int)length == strlen(str))
  {
    *address = value;
    return TRUE;
  }
  return FALSE;
}
