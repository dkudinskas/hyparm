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

#define HIDDEN_RAM_START   0x8f000000
#define HIDDEN_RAM_SIZE    0x01000000 // 16 MB


extern void startupHypervisor(void);
extern void registerGuestPointer(u32int gContext);

void printUsage(void);
int parseCommandline(int argc, char *argv[]);
void registerGuestContext(u32int gcAddr);

fatfs mainFilesystem;
partitionTable primaryPartitionTable;
struct mmc *mmcDevice;
file * debugStream;
u32int kernAddr;
u32int initrdAddr;

int main(int argc, char *argv[])
{
  kernAddr = 0;
  initrdAddr = 0;
  GCONTXT * gContext = 0;

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
  startupHypervisor();

  /* initialize guest context */
  gContext = allocateGuest();
  registerGuestPointer((u32int)gContext);

  /* Setup MMU for Hypervisor */
  initialiseVirtualAddressing();

  if (parseCommandline(argc, argv) < 0)
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

  u32int err = 0;
  if ((err = mmcMainInit()) != 0)
  {
    DIE_NOW(0, "Failed to initialize mmc code.\n");
  }

  if ((err = partTableRead(&mmcDevice->blockDev, &primaryPartitionTable)) != 0)
  {
    DIE_NOW(0, "Failed to read partition table.\n");
  }
  
  if ((err = fatMount(&mainFilesystem, &mmcDevice->blockDev, 1)) != 0)
  {
    DIE_NOW(0, "Failed to mount FAT partition.\n");
  }

  debugStream = fopen(&mainFilesystem, "debug");
  if (debugStream == 0)
  {
    DIE_NOW(0, "Failed to open (create) debug steam file.\n");
  }

  // does not return
  doLinuxBoot(&imageHeader, kernAddr, initrdAddr);
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
