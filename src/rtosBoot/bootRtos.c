#include "common/memFunctions.h"
#include "common/string.h"

#include "cpuArch/cpu.h"

#include "instructionEmu/scanner.h"

#include "linuxBoot/bootLinux.h"

#include "memoryManager/addressing.h"
#include "memoryManager/pageTable.h"

#include "rtosBoot/bootRtos.h"


extern void callKernel(int, int, u32int, u32int) __attribute__((noreturn));

static void setup_start_tag(void);
static void setup_revision_tag(void);
static void setup_memory_tags(void);
static void setup_commandline_tag(const char *commandline);
static void setup_end_tag(void);

static struct tag * paramTag;
static struct RamConfig dramBanks[NR_DRAM_BANKS];


void doRtosBoot(GCONTXT *context, ulong loadAddr)
{
  ulong targetAddr = loadAddr;
  const char * commandline = "";

  paramTag = (struct tag *)BOARD_PARAMS;

  /* TODO: add virtual addressing startup for the new VM here
  need to create a Global Page table/map for the VM and add mappings for where the kernel is to be copied?
  */
  createVirtualMachineGPAtoRPA(context);

  populateDramBanks();

  setup_start_tag();
  setup_revision_tag();
  setup_memory_tags();
  setup_commandline_tag(commandline);
  setup_end_tag();

#ifdef CONFIG_DEBUG_SCANNER_CALL_SOURCE
  setScanBlockCallSource(SCANNER_CALL_SOURCE_BOOT);
#endif
#ifdef CONFIG_DEBUG_SCANNER_COUNT_BLOCKS
  resetScannerCounter();
#endif
  scanBlock(context, targetAddr);

  /* This seems to be OS independent */
  cleanupBeforeLinux();

  /* does not return */
  callKernel(0, (u32int)BOARD_MACHINE_ID, (u32int)BOARD_PARAMS, targetAddr);
}

static void setup_start_tag()
{
  paramTag->hdr.tag = ATAG_CORE;
  paramTag->hdr.size = tag_size (tag_core);

  paramTag->u.core.flags = 0;
  paramTag->u.core.pagesize = 0;
  paramTag->u.core.rootdev = 0;

  paramTag = tag_next(paramTag);
}

void setup_revision_tag(void)
{
  u32int rev = BOARD_REVISION;

  paramTag->hdr.tag = ATAG_REVISION;
  paramTag->hdr.size = tag_size (tag_revision);
  paramTag->u.revision.rev = rev;
  paramTag = tag_next(paramTag);
}

static void setup_memory_tags()
{
  int i;

  for (i = 0; i < NR_DRAM_BANKS; i++)
  {
    paramTag->hdr.tag = ATAG_MEM;
    paramTag->hdr.size = tag_size (tag_mem32);

    paramTag->u.mem.start = dramBanks[i].start;
    paramTag->u.mem.size = dramBanks[i].size;

    paramTag = tag_next (paramTag);
  }
}

static void setup_commandline_tag(const char *commandline)
{
  const char *p;

  if (!commandline)
  {
    return;
  }

  /* eat leading white space */
  for (p = commandline; *p == ' '; p++);

  /* skip non-existent command lines so the kernel will still
   * use its default command line.
   */
  if (*p == '\0')
  {
    return;
  }

  paramTag->hdr.tag = ATAG_CMDLINE;
  paramTag->hdr.size = (sizeof (struct tag_header) + strlen(p) + 1 + 4) >> 2;

  strcpy(paramTag->u.cmdline.cmdline, p);

  paramTag = tag_next(paramTag);
}

static void setup_end_tag ()
{
  paramTag->hdr.tag = ATAG_NONE;
  paramTag->hdr.size = 0;
}
