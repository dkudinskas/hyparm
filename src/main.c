#ifdef CONFIG_CLI
#include "cli/cli.h"
#endif

#include "common/commandLine.h"
#include "common/debug.h"
#include "common/stdarg.h"
#include "common/stddef.h"
#include "common/string.h"

#include "common/memoryAllocator/allocator.h"

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

  initialiseAllocator(HIDDEN_RAM_START, HIDDEN_RAM_SIZE);

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
  bool hadGuestOption = FALSE;
  for (p = commandLine; p; p = p->next)
  {
    switch (p->argumentId)
    {
      case CL_OPTION_GUEST_OS:
        if (hadGuestOption)
        {
          printf("Error: duplicate option: guest OS '%s'" EOL, p->value);
          success = FALSE;
        }
        else if (strcmp(p->value, CL_VALUE_GUEST_OS_FREERTOS) == 0)
        {
          printf("set guest=freertos" EOL);
          config->guestOS = GUEST_OS_FREERTOS;
        }
        else if (strcmp(p->value, CL_VALUE_GUEST_OS_LINUX) == 0)
        {
          printf("set guest=linux" EOL);
          config->guestOS = GUEST_OS_LINUX;
        }
        else
        {
          printf("Error: invalid guest OS '%s'" EOL, p->value);
          success = FALSE;
        }
        hadGuestOption = TRUE;
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
