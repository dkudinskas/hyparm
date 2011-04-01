#include "common/debug.h"
#include "common/memFunctions.h"
#include "common/stringFunctions.h"

#include "cpuArch/cpu.h"

#include "drivers/beagle/beIntc.h"
#include "drivers/beagle/beGPTimer.h"
#include "drivers/beagle/beClockMan.h"
#include "drivers/beagle/beUart.h"

#include "vm/omap35xx/hardwareLibrary.h"
#include "vm/omap35xx/LED.h"

#include "linuxBoot/bootLinux.h"
#include "linuxBoot/image.h"

#include "rtosBoot/bootRtos.h"

#include "guestManager/guestContext.h"
#include "guestManager/blockCache.h"

#include "memoryManager/addressing.h" /* For virtual addressing initialisation */
#include "memoryManager/cp15coproc.h"
#include "memoryManager/frameAllocator.h"


// uncomment me to enable startup debug: #define STARTUP_DEBUG

#define STARTUP_DEBUG

#define HIDDEN_RAM_START   0x8f000000
#define HIDDEN_RAM_SIZE    0x01000000 // 16 MB

extern void startup_hypervisor(void);

void printUsage(void);
int parseCommandline(int argc, char *argv[]);
void registerGuestContext(u32int gcAddr);
GCONTXT * getGuestContext(void);

u32int kernAddr;
u32int initrdAddr;

// guest context
GCONTXT * gContext;

int main(int argc, char *argv[])
{
  int ret = 0;
  kernAddr = 0;
  initrdAddr = 0;
  gContext = 0;
  image_header_t imageHeader;
  /* save power: cut the clocks to the display subsystem */
  cmDisableDssClocks();
  
  mallocInit(HIDDEN_RAM_START, HIDDEN_RAM_SIZE);

  /* initialise uart backend, important to be before any debug output. */
  /* init function initialises UARTs in disabled mode. */
  /* startup fuction starts operation and enables RX IRQ generation */
  beUartInit(1);
  beUartInit(2);
  beUartInit(3);
  beUartStartup(3);

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
    printf("Coprocessor register bank at %x\n", (u32int)coprocRegBank);
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
    printf("Basic block cache at %x\n", (u32int)blockCache);
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
  else if(ret==1){
#ifdef STARTUP_DEBUG
<<<<<<< HEAD
	serial_putstring("Kernel address: ");
	serial_putlong(kernAddr);
	serial_newline();
	serial_putstring("Initrd address: ");
	serial_putlong(initrdAddr);
	serial_newline();
=======
  printf("Kernel address: %x, Initrd address: %x\n", kernAddr, initrdAddr);
>>>>>>> a24883a72b9c3ad47fa49a11cb6baea3399e34ca
#endif
  imageHeader = getImageHeader(kernAddr);
#ifdef STARTUP_DEBUG
  dumpHdrInfo(&imageHeader);
#endif
  }
  else if(ret==2){
#ifdef STARTUP_DEBUG
	serial_putstring("RTOS address: ");
	serial_putlong(kernAddr);
	serial_newline();
#endif
  }
  /* initialise physical interrupt controller */
  intcBEInit();

  /* now we can umkask first interrupt - UART */
  unmaskInterruptBE(UART3_IRQ);

  /* initialise physical clock manager */
  clkManBEInit();

  /* initialise phyiscal GPT2, dedicated to guest1 */
  gptBEInit(2);

  // does not return
  if(ret==2)doRtosBoot(kernAddr);
  else doLinuxBoot(&imageHeader, kernAddr, initrdAddr);
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
<<<<<<< HEAD
  serial_putstring("Loader usage:");
  serial_newline();
  serial_putstring ("go <loaderAddr> -kernel <kernAddress> -initrd <initrdAddr>");
  serial_newline();
  serial_putstring("kernel: address of kernel in hex format (0xXXXXXXXX)");
  serial_newline();
  serial_putstring("initrd: address of external initrd in hex format (0xXXXXXXXX)");
  serial_newline();
  serial_putstring("For RTOS: go <loaderAddr> -rtos <rtosAddr>");
  serial_newline();
  serial_putstring("rtos: address of rtos in hex format (0xXXXXXXXX)");
  serial_newline();
=======
  printf("Loader usage:\n");
  printf("go <loaderAddr> -kernel <kernAddress> -initrd <initrdAddr>\n");
  printf("kernel: address of kernel in hex format (0xXXXXXXXX)\n");
  printf("initrd: address of external initrd in hex format (0xXXXXXXXX)\n");
>>>>>>> a24883a72b9c3ad47fa49a11cb6baea3399e34ca
  return;
}

int parseCommandline(int argc, char *argv[])
{
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

  /***************** Check KERNEL address parameter ****************/
  cmpFlag = stringncmp("-kernel", argv[1], 7);
  if (cmpFlag < 0)
  {
<<<<<<< HEAD
    serial_putstring("Parameter -kernel not found.");
    serial_newline();
    /* Check for -rtos */
    cmpFlag = stringncmp("-rtos",argv[1],5);
    if ( cmpFlag < 0){
    	serial_putstring("Parameter -rtos not found.");
	serial_newline();
	return -1; // nothing useful was found
    }
    else{
    	kernAddr = stringToLong(argv[2]);
	return 2; // state that -rtos was found
    }

=======
    printf("Parameter -kernel not found.");
    return -1;
>>>>>>> a24883a72b9c3ad47fa49a11cb6baea3399e34ca
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
