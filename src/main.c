#ifdef CONFIG_CLI
#include "cli/cli.h"
#endif

#include "common/debug.h"
#include "common/memFunctions.h"
#include "common/string.h"

#include "cpuArch/cpu.h"

#include "drivers/beagle/beIntc.h"
#include "drivers/beagle/beGPTimer.h"
#include "drivers/beagle/beClockMan.h"
#include "drivers/beagle/beUart.h"

#ifdef CONFIG_MMC
#include "drivers/beagle/beMMC.h"
#endif

#include "vm/omap35xx/hardwareLibrary.h"
#include "vm/omap35xx/LED.h"

#include "linuxBoot/bootLinux.h"
#include "linuxBoot/image.h"

#include "guestManager/guestContext.h"
#include "guestManager/blockCache.h"

#include "memoryManager/addressing.h" /* For virtual addressing initialisation */
#include "memoryManager/cp15coproc.h"
#include "memoryManager/frameAllocator.h"

#ifdef CONFIG_MMC
#include "io/mmc.h"
#include "io/partitions.h"
#include "io/fs/fat.h"
#endif

// uncomment me to enable startup debug: #define STARTUP_DEBUG

#define HIDDEN_RAM_START   0x8f000000
#define HIDDEN_RAM_SIZE    0x01000000 // 16 MB

extern void registerGuestPointer(u32int gContext);

static void printUsage(void);
static int parseCommandline(int argc, char *argv[], u32int *kernAddr, u32int *initrdAddr);
void registerGuestContext(u32int gcAddr);

#ifdef CONFIG_MMC
fatfs mainFilesystem;
partitionTable primaryPartitionTable;
struct mmc *mmcDevice;
file * debugStream;
#endif


int main(int argc, char *argv[])
{
  u32int kernAddr = 0;
  u32int initrdAddr = 0;
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

  /* initialize guest context */
  gContext = allocateGuest();
  registerGuestPointer((u32int)gContext);

  /* Setup MMU for Hypervisor */
/*
 * FIXME: setting up MMU might remap argv !!
 * Parse arguments before setting up MMU!!!!
 */
  initialiseVirtualAddressing();

#ifdef CONFIG_CLI

/*
 * FIXME: deal with guest context and VIRTUAL ADDRESSING!!!
 */

  enterCliLoop();

#else

  if (parseCommandline(argc, argv, &kernAddr, &initrdAddr) < 0)
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

#ifdef CONFIG_MMC
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
    DIE_NOW(0, "Failed to open (create) debug stream file.\n");
  }
#endif

  // does not return
  doLinuxBoot(&imageHeader, kernAddr, initrdAddr);

#endif /* CONFIG_CLI */
}

static void printUsage(void)
{
  printf("Loader usage:\n");
  printf("go <loaderAddr> -kernel <kernAddress> -initrd <initrdAddr>\n");
  printf("kernel: address of kernel in hex format (0xXXXXXXXX)\n");
  printf("initrd: address of external initrd in hex format (0xXXXXXXXX)\n");
  return;
}

static int parseCommandline(s32int argc, char *argv[], u32int *kernAddr, u32int *initrdAddr)
{
#ifdef STARTUP_DEBUG
  s32int i;
  printf("Number of args: %c\n", argc+0x30);
  for (i = 0; i < argc; i++)
  {
    printf("Arg %c: %x\n", i+0x30, argv[i]);
  }
#endif

  if (argc != 5)
  {
    return -1;
  }

  if (strcmp("-kernel", argv[1]) != 0)
  {
#ifdef STARTUP_DEBUG
    printf("Parameter -kernel not found.");
#endif
    return -1;
  }

  if (sscanf(argv[2], "%x", kernAddr) != 1)
  {
#ifdef STARTUP_DEBUG
    printf("Parameter -kernel has invalid address '%s'.", arg);
#endif
    return -1;
  }

  if (strcmp("-initrd", argv[3]) != 0)
  {
#ifdef STARTUP_DEBUG
    printf("Parameter -initrd not found.");
#endif
    return -1;
  }

  if (sscanf(argv[4], "%x", initrdAddr) != 1)
  {
#ifdef STARTUP_DEBUG
    printf("Parameter -initrd has invalid address '%s'.", arg);
#endif
    return -1;
  }

  return 1;
}

