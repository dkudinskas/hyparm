#include "common/debug.h"
#include "common/memFunctions.h"
#include "common/stringFunctions.h"

#include "cpuArch/cpu.h"

#include "drivers/beagle/beIntc.h"
#include "drivers/beagle/beGPTimer.h"
#include "drivers/beagle/beClockMan.h"
#include "drivers/beagle/beUart.h"
#include "drivers/beagle/beMMC.h"

#include "vm/omap35xx/hardwareLibrary.h"
#include "vm/omap35xx/LED.h"

#include "instructionEmu/scanner.h"

#include "linuxBoot/bootLinux.h"
#include "linuxBoot/image.h"

#include "guestManager/guestContext.h"
#include "guestManager/blockCache.h"

#include "memoryManager/addressing.h" /* For virtual addressing initialisation */
#include "memoryManager/cp15coproc.h"
#include "memoryManager/frameAllocator.h"

#include "io/mmc.h"
#include "io/partitions.h"
#include "io/fs/fat.h"


// uncomment me to enable startup debug: #define STARTUP_DEBUG

//uncomment me to enable block copy cache debug (installation of backpointer): #define BLOCK_COPY_DEBUG

#define HIDDEN_RAM_START   0x8f000000
#define HIDDEN_RAM_SIZE    0x01000000 // 16 MB

extern void startup_hypervisor(void);
extern void registerGuestPointer(u32int gContext);

void printUsage(void);
int parseCommandline(int argc, char *argv[]);
void registerGuestContext(u32int gcAddr);
GCONTXT * getGuestContext(void);
u32int scannerReqCounter;

u32int kernAddr;
u32int initrdAddr;

// guest context
GCONTXT * gContext;

extern struct mmc *mmcDevice;
extern fatfs mainFilesystem;
extern partitionTable primaryPartitionTable;

int main(int argc, char *argv[])
{
  int ret = 0;
  kernAddr = 0;
  initrdAddr = 0;
  gContext = 0;
#ifdef SCANNER_COUNTER  
  scannerReqCounter = 0;
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

  /* create the frametable from which we can alloc memory */
  initialiseFrameTable();

  /* sets up stack addresses and exception handlers */
  startup_hypervisor();

  /* initialize guest context */
  gContext = (GCONTXT*)mallocBytes(sizeof(GCONTXT));
  if (gContext == 0)
  {
    DIE_NOW(0, "Failed to allocate guest context.");
  }
#ifdef STARTUP_DEBUG
  else
  {
    printf("Guest context at%x\n", (u32int)gContext);
  }
#endif
  registerGuestPointer((u32int)gContext);
  initGuestContext(gContext);

  /* initialise coprocessor register bank */
  CREG * coprocRegBank = (CREG*)mallocBytes(MAX_CRB_SIZE * sizeof(CREG));
  if (coprocRegBank == 0)
  {
    DIE_NOW(0, "Failed to allocate coprocessor register bank.");
  }
  else
  {
    memset((void*)coprocRegBank, 0x0, MAX_CRB_SIZE * sizeof(CREG));
#ifdef STARTUP_DEBUG
    printf("Coprocessor register bank at %x\n", (u32int)coprocRegBank);
#endif
  }
  registerCrb(gContext, coprocRegBank);
  
  /* initialise block cache (logbook) */
  BCENTRY * blockCache = (BCENTRY*)mallocBytes(BLOCK_CACHE_SIZE * sizeof(BCENTRY));
  if (blockCache == 0)
  {
    DIE_NOW(0, "Failed to allocate basic block cache.\n");
  }
  else
  {
    memset((void*)blockCache, 0x0, BLOCK_CACHE_SIZE * sizeof(BCENTRY));
#ifdef STARTUP_DEBUG
    printf("Basic block cache at %x\n", (u32int)blockCache);
#endif
  }
  registerBlockCache(gContext, blockCache);

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
  registerBlockCopyCache(gContext, blockCopyCache, BLOCK_COPY_CACHE_SIZE);
#endif //CONFIG_BLOCK_COPY


  /* initialise virtual hardware devices */
  device * libraryPtr;
  if ((libraryPtr = initialiseHardwareLibrary()) != 0)
  {
    /* success. register with guest context */
    registerHardwareLibrary(gContext, libraryPtr);
  }
  else
  {
    DIE_NOW(0, "Hardware library initialisation failed.");
  }

  /* Setup MMU for Hypervisor */
  initialiseVirtualAddressing();

  /* Setup guest memory protection */
  registerMemoryProtection(gContext);



  ret = parseCommandline(argc, argv);
  if ( ret < 0 )
  {
    printUsage();
    DIE_NOW(0, "Hypervisor startup aborted.");
  }

#ifdef STARTUP_DEBUG
  printf("Kernel address: %x, Initrd address: %x\n", kernAddr, initrdAddr);
#endif

  image_header_t imageHeader = getImageHeader(kernAddr);
#ifdef STARTUP_DEBUG
  dumpHdrInfo(&imageHeader);
#endif

  /* initialise physical interrupt controller */
  intcBEInit();

  /* now we can umkask first interrupt - UART */
  unmaskInterruptBE(UART3_IRQ);

  /* initialise physical clock manager */
  clkManBEInit();

  /* initialise phyiscal GPT2, dedicated to guest1 */
  gptBEInit(2);

  u32int err = mmcMainInit();
  printf("mmcMainInit ret %x\n", err);
  
  err = partTableRead(&mmcDevice->blockDev, &primaryPartitionTable);
  printf("partTableRead ret %x\n", err);

  err = fatMount(&mainFilesystem, &mmcDevice->blockDev, 1);
  printf("fatMount ret %x\n", err);

/*  char * filename = "testfile";
  char * writeText = "The quick brown fox jumps over the lazy dog.\n";
  u32int len = fatWriteFile(&mainFilesystem, filename, writeText, stringlen(writeText));
  printf("writen %x bytes to '%s'\n", len, filename);*/

  fatRootLs(&mainFilesystem);

  // does not return
#ifdef CONFIG_BLOCK_COPY_NO_IRQ
  disableInterrupts();
#endif
  doLinuxBoot(&imageHeader, kernAddr, initrdAddr);
}

GCONTXT * getGuestContext()
{
  return gContext;
}

void printUsage(void)
{
  printf("Loader usage:\n");
  printf("go <loaderAddr> -kernel <kernAddress> -initrd <initrdAddr>\n");
  printf("kernel: address of kernel in hex format (0xXXXXXXXX)\n");
  printf("initrd: address of external initrd in hex format (0xXXXXXXXX)\n");
  return;
}

int parseCommandline(int argc, char *argv[])
{
  /* TEMPORARY HACK BECAUSE OF BOOTSCRIPT */
  kernAddr   = 0x80300000;
  initrdAddr = 0x81600000;
  return 1;
  /* END OF TEMPORARY HACK*/

  int cmpFlag = 0;
  /***************** Check given arguments ************************/
#ifdef STARTUP_DEBUG
  int i = 0;
  printf("Number of args: %c\n", argc+0x30);
  for (i = 0; i < argc; i++)
  {
    printf("Arg %c: %x\n", i+0x30, argv[i]);
  }
#endif

  if ( argc != 5 )
  {
    return -1;
  }

  /***************** Check KERNEL address parameter ****************/
  cmpFlag = stringncmp("-kernel", argv[1], 7);
  if (cmpFlag < 0)
  {
    printf("Parameter -kernel not found.");
    return -1;
  }
  kernAddr = strtoi(argv[2]);
  if (kernAddr < 0)
  {
    printf("Invalid kernel address.");
    return -1;
  }

  /***************** Check INITRD address parameter ****************/
  cmpFlag = stringncmp("-initrd", argv[3], 7);
  if (cmpFlag < 0)
  {
    printf("Parameter -initrd not found.");
    return -1;
  }
  initrdAddr = strtoi(argv[4]);
  if (initrdAddr < 0)
  {
    printf("Invalid initrd address.");
    return -1;
  }
  return 1;
}
