#ifdef CONFIG_CLI
#include "cli/cli.h"
#endif

#include "common/commandLine.h"
#include "common/debug.h"
#include "common/linker.h"
#include "common/stdarg.h"
#include "common/stddef.h"
#include "common/string.h"
#include "common/memoryAllocator/allocator.h"
#ifdef CONFIG_PROFILER
#include "common/profiler.h"
#endif

#include "cpuArch/armv7.h"

#include "drivers/beagle/beIntc.h"
#include "drivers/beagle/beGPTimer.h"
#include "drivers/beagle/beClockMan.h"
#include "drivers/beagle/beUart.h"

#ifndef CONFIG_NO_MMC
#include "drivers/beagle/beMMC.h"
#endif

#include "guestBoot/image.h"
#include "guestBoot/linux.h"
#include "guestBoot/test.h"

#include "guestManager/guestContext.h"

#include "instructionEmu/scanner.h"

#ifndef CONFIG_NO_MMC
#include "io/mmc.h"
#endif
#ifdef CONFIG_MMC_LOG
#include "io/partitions.h"
#include "io/fs/fat.h"
#endif

#ifdef CONFIG_GUEST_FREERTOS
#include "guestBoot/freertos.h"
#endif

#include "memoryManager/addressing.h"

#include "vm/omap35xx/cp15coproc.h"
#include "vm/omap35xx/hardwareLibrary.h"


#define CL_OPTION_FAST_UART          1
#define CL_OPTION_GUEST_OS           2
#define CL_OPTION_GUEST_KERNEL       3
#define CL_OPTION_GUEST_INITRD       4
#define CL_OPTION_GUEST_KCMDLINE     5

#define CL_VALUE_GUEST_OS_FREERTOS   "freertos"
#define CL_VALUE_GUEST_OS_LINUX      "linux"
#define CL_VALUE_GUEST_OS_TEST       "test"


struct runtimeConfiguration
{
  enum guestOSType guestOS;
  u32int guestKernelAddress;
  u32int guestInitialRAMDiskAddress;
  const char *guestKernelCmdLine;
};


static void dumpRuntimeConfiguration(struct runtimeConfiguration *config) __cold__;
void main(s32int argc, char *argv[]) __cold__;
static void processCommandLine(struct runtimeConfiguration *config, s32int argc, char *argv[]) __cold__;


#ifndef CONFIG_NO_MMC
struct mmc *mmcDevice;
#endif

#ifdef CONFIG_MMC_LOG
fatfs mainFilesystem;
partitionTable primaryPartitionTable;
file *debugStream;
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

#ifndef CONFIG_NO_MMC
  mmcDevice = NULL;
#endif
#ifdef CONFIG_MMC_LOG
  debugStream = NULL;
#endif

  /* save power: cut the clocks to the display subsystem */
  cmDisableDssClocks();

  initialiseAllocator(RAM_XN_POOL_BEGIN, RAM_XN_POOL_END - RAM_XN_POOL_BEGIN);

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
   * Print the bounds of the hypervisor image in RAM.
   */
  DEBUG(STARTUP, "Hypervisor @ %#.8x -- %#.8x" EOL, HYPERVISOR_BEGIN_ADDRESS, HYPERVISOR_END_ADDRESS);
  DEBUG(STARTUP, "Code store @ %#.8x -- %#.8x" EOL, RAM_CODE_CACHE_POOL_BEGIN, RAM_CODE_CACHE_POOL_END);
  DEBUG(STARTUP, "Malloc Region @ %#.8x -- %#.8x" EOL, RAM_XN_POOL_BEGIN, RAM_XN_POOL_END);

  /*
   * Use command line arguments passed by U-Boot to update the runtime configuration structure. The
   * first argument is always the load address of the hypervisor; skip it.
   */
  processCommandLine(&config, argc - 1, argv + 1);
  dumpRuntimeConfiguration(&config);

  /* initialize guest context */
  GCONTXT *context = createGuestContext();
  activeGuestContext = context;

  /* Setup MMU for Hypervisor
   * NOTE: assumes guest context is set up in full (PT info + T$) */
  initVirtualAddressing(context);

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

#ifdef CONFIG_MMC_LOG
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
#endif /* CONFIG_MMC_LOG */

#ifdef CONFIG_PROFILER
  profilerInit();
  printf("Profiler started\n");
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
      bootLinux(context, config.guestKernelAddress, config.guestInitialRAMDiskAddress, config.guestKernelCmdLine);
      break;
    case GUEST_OS_TEST:
      bootTest(context, config.guestKernelAddress);
      break;
    default:
      printf("Error: unsupported guest OS %#x", config.guestOS);
      break;
  }
  DIE_NOW(context, "guest boot failed");

#endif /* CONFIG_CLI */
}


#ifdef CONFIG_HARDCODED_CMDLINE

static void processCommandLine(struct runtimeConfiguration *config, s32int argc, char *argv[])
{
  UNUSED(argc);
  UNUSED(argv);

#if defined(CONFIG_HARDCODED_CMDLINE_GUEST_LINUX)
  config->guestOS = GUEST_OS_LINUX;
#elif defined(CONFIG_HARDCODED_CMDLINE_GUEST_TEST)
  config->guestOS = GUEST_OS_TEST;
#else
#error config->guestOS undefined
#endif

  config->guestKernelAddress = CONFIG_HARDCODED_CMDLINE_KERNEL;

#ifdef CONFIG_HARDCODED_CMDLINE_INITRD
  config->guestInitialRAMDiskAddress = CONFIG_HARDCODED_CMDLINE_INITRD_ADDRESS;
#endif

  config->guestKernelCmdLine = CONFIG_HARDCODED_CMDLINE_KCMDLINE;
}

#else

static bool stringToAddress(const char *str, u32int *address) __cold__;

static void processCommandLine(struct runtimeConfiguration *config, s32int argc, char *argv[])
{
  bool success = TRUE;
  struct commandLine *commandLine, *p;
  struct commandLineOption *options = NULL;
  options = addCommandLineOption(options, "guest", "Guest operating system type", TRUE, FALSE, CL_OPTION_GUEST_OS);
  options = addCommandLineOption(options, "kernel", "Address of the kernel in memory", TRUE, TRUE, CL_OPTION_GUEST_KERNEL);
  options = addCommandLineOption(options, "initrd", "Address of an initial RAM disk in memory", TRUE, FALSE, CL_OPTION_GUEST_INITRD);
  options = addCommandLineOption(options, "kcmdline", "Kernel command line", TRUE, FALSE, CL_OPTION_GUEST_KCMDLINE);
  commandLine = parseCommandLine(options, argc, argv);
  bool hadGuestOption = FALSE;
  for (p = commandLine; p; p = p->next)
  {
    switch (p->argumentId)
    {
      case CL_OPTION_GUEST_OS:
      {
        if (hadGuestOption)
        {
          printf("Error: duplicate option: guest OS '%s'" EOL, p->value);
          success = FALSE;
        }
        else if (strcmp(p->value, CL_VALUE_GUEST_OS_FREERTOS) == 0)
        {
          config->guestOS = GUEST_OS_FREERTOS;
        }
        else if (strcmp(p->value, CL_VALUE_GUEST_OS_LINUX) == 0)
        {
          config->guestOS = GUEST_OS_LINUX;
        }
        else if (strcmp(p->value, CL_VALUE_GUEST_OS_TEST) == 0)
        {
          config->guestOS = GUEST_OS_TEST;
        }
        else
        {
          printf("Error: invalid guest OS '%s'" EOL, p->value);
          success = FALSE;
        }
        hadGuestOption = TRUE;
        break;
      }
      case CL_OPTION_GUEST_KERNEL:
      {
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
      }
      case CL_OPTION_GUEST_INITRD:
      {
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
      }
      case CL_OPTION_GUEST_KCMDLINE:
      {
        if (config->guestKernelCmdLine)
        {
          printf("Error: duplicate option: kernel command line" EOL);
          success = FALSE;
        }
        else
        {
          config->guestKernelCmdLine = p->value;
        }
        break;
      }
      default:
      {
        printf("Error: unrecognized option '%s'" EOL, p->value);
        success = FALSE;
        break;
      }
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

#endif /* CONFIG_HARDCODED_CMDLINE */
