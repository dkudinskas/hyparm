#include "LED.h"
#include "serial.h"
#include "stringFunctions.h"
#include "image.h"
#include "bootLinux.h"
#include "cp15coproc.h"
#include "guestContext.h"
#include "blockCache.h"
#include "addressing.h" /* For virtual addressing initialisation */
#include "frameAllocator.h"
#include "hardwareLibrary.h"
#include "memFunctions.h"
#include "cpu.h"
#include "beIntc.h"
#include "beGPTimer.h"
#include "beClockMan.h"
#include "debug.h"
#include "scanner.h"


// uncomment me to enable startup debug: #define STARTUP_DEBUG

//uncomment me to enable block copy cache debug (installation of backpointer): #define BLOCK_COPY_DEBUG

#define HIDDEN_RAM_START   0x8f000000
#define HIDDEN_RAM_SIZE    0x01000000 // 16 MB

extern void startup_hypervisor(void);

void printUsage(void);
int parseCommandline(int argc, char *argv[]);
void registerGuestContext(u32int gcAddr);
GCONTXT * getGuestContext(void);
u32int scannerReqCounter;

unsigned long kernAddr;
unsigned long initrdAddr;

// guest context
GCONTXT * gContext;

int main(int argc, char *argv[])
{

  scannerReqCounter = 0;
  /* save power: cut the clocks to the display subsystem */
  cmDisableDssClocks();
  
  int ret = 0;
  kernAddr = 0;
  initrdAddr = 0;

  mallocInit(HIDDEN_RAM_START, HIDDEN_RAM_SIZE);
  
  /* create the frametable from which we can alloc memory */
  initialiseFrameTable();

  /* sets up stack addresses and exception handlers */
  startup_hypervisor();

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
    serial_putstring("Coprocessor register bank at 0x");
    serial_putint((u32int)coprocRegBank);
    serial_newline();
#endif
  }
  registerCrb(gContext, coprocRegBank);
  
  /* initialise block cache (logbook) */
  BCENTRY * blockCache = (BCENTRY*)mallocBytes(BLOCK_CACHE_SIZE * sizeof(BCENTRY));
  if (blockCache == 0)
  {
    DIE_NOW(0, "Failed to allocate basic block cache.");
  }
  else
  {
    memset((void*)blockCache, 0x0, BLOCK_CACHE_SIZE * sizeof(BCENTRY));
#ifdef STARTUP_DEBUG
    serial_putstring("Basic block cache at 0x");
    serial_putint((u32int)blockCache);
    serial_newline();
#endif
  }
  registerBlockCache(gContext, blockCache);

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
    //TODO CHECK IF THIS Branch is installed correctly!
#ifdef STARTUP_DEBUG
    serial_putstring("Block copy cache at 0x");
    serial_putint((u32int)blockCopyCache);
    serial_newline();
#endif
  }
  registerBlockCopyCache(gContext, blockCopyCache, BLOCK_COPY_CACHE_SIZE);



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
  serial_putstring("Kernel address: ");
  serial_putlong(kernAddr);
  serial_newline();
  serial_putstring("Initrd address: ");
  serial_putlong(initrdAddr);
  serial_newline();
#endif

  image_header_t imageHeader = getImageHeader(kernAddr);
#ifdef STARTUP_DEBUG
  dumpHdrInfo(&imageHeader);
#endif
  //Till here everything is ok

  /* initialise physical interrupt controller */
  intcBEInit();

  /* initialise physical clock manager */
  clkManBEInit();

  /* initialise phyiscal GPT2, dedicated to guest1 */
  gptBEInit(2);
/*
  setClockSource(2, FALSE);
  toggleTimerFclk(2, TRUE);
  gptBEEnableOverflowInterrupt(2);
  gptBESet10msTick(2);
  unmaskInterruptBE(GPT2_IRQ);
  enableInterrupts();
  gptBEStart(2);*/

  // does not return
  doLinuxBoot(&imageHeader, kernAddr, initrdAddr);
}

void registerGuestContext(u32int gcAddr)
{
  gContext = (GCONTXT *)gcAddr;
  initGuestContext(gContext);
}

GCONTXT * getGuestContext()
{
  return gContext;
}

void printUsage(void)
{
  serial_putstring("Loader usage:");
  serial_newline();
  serial_putstring ("go <loaderAddr> -kernel <kernAddress> -initrd <initrdAddr>");
  serial_newline();
  serial_putstring("kernel: address of kernel in hex format (0xXXXXXXXX)");
  serial_newline();
  serial_putstring("initrd: address of external initrd in hex format (0xXXXXXXXX)");
  serial_newline();
  return;
}

int parseCommandline(int argc, char *argv[])
{
  int cmpFlag = 0;
  /***************** Check given arguments ************************/
#ifdef STARTUP_DEBUG
  int i = 0;
  serial_putstring("Number of args: ");
  serial_putchar(argc + 0x30);
  serial_newline();
  for (i = 0; i < argc; i++)
  {
    serial_putstring("Arg ");
    serial_putchar(i + 0x30);
    serial_putstring(": ");
    serial_putstring(argv[i]);
    serial_newline();
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
    serial_putstring("Parameter -kernel not found.");
    serial_newline();
    return -1;
  }
  kernAddr = stringToLong(argv[2]);
  if (kernAddr < 0)
  {
    serial_putstring("Invalid kernel address.");
    serial_newline();
    return -1;
  }

  /***************** Check INITRD address parameter ****************/
  cmpFlag = stringncmp("-initrd", argv[3], 7);
  if (cmpFlag < 0)
  {
    serial_putstring("Parameter -initrd not found.");
    serial_newline();
    return -1;
  }
  initrdAddr = stringToLong(argv[4]);
  if (initrdAddr < 0)
  {
    serial_putstring("Invalid initrd address.");
    serial_newline();
    return -1;
  }
  return 1;
}
