#include "common/debug.h"
#include "common/memFunctions.h"
#include "common/stringFunctions.h"

#include "cpuArch/cpu.h"

#include "drivers/beagle/beIntc.h"
#include "drivers/beagle/beGPTimer.h"
#include "drivers/beagle/beClockMan.h"

#include "vm/omap35xx/hardwareLibrary.h"
#include "vm/omap35xx/LED.h"
#include "vm/omap35xx/serial.h"

#include "linuxBoot/bootLinux.h"
#include "linuxBoot/image.h"

#include "guestManager/guestContext.h"
#include "guestManager/blockCache.h"

#include "memoryManager/addressing.h" /* For virtual addressing initialisation */
#include "memoryManager/cp15coproc.h"
#include "memoryManager/frameAllocator.h"


// uncomment me to enable startup debug: #define STARTUP_DEBUG

#define HIDDEN_RAM_START   0x8f000000
#define HIDDEN_RAM_SIZE    0x01000000 // 16 MB

extern void startup_hypervisor(void);

void printUsage(void);
int parseCommandline(int argc, char *argv[]);
void registerGuestContext(u32int gcAddr);
GCONTXT * getGuestContext(void);

unsigned long kernAddr;
unsigned long initrdAddr;

// guest context
GCONTXT * gContext;

int main(int argc, char *argv[])
{
  int ret = 0;
  kernAddr = 0;
  initrdAddr = 0;
  gContext = 0;

  /* save power: cut the clocks to the display subsystem */
  cmDisableDssClocks();
  
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
  
  /* initialise block cache */
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
