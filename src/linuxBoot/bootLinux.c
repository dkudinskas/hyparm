#include "common/memFunctions.h"
#include "common/stringFunctions.h"

#include "cpuArch/cpu.h"

#include "vm/omap35xx/serial.h"

#include "instructionEmu/scanner.h"

#include "linuxBoot/bootLinux.h"

#include "memoryManager/addressing.h"
#include "memoryManager/pageTable.h"


extern GCONTXT * getGuestContext(void);
extern void callKernel(int, int, u32int, u32int) __attribute__((noreturn));

static void setup_start_tag(void);
static void setup_revision_tag(void);
static void setup_initrd_tag(ulong initrd_start, ulong initrd_end);
static void setup_memory_tags(void);
static void setup_commandline_tag(char *commandline);
static void setup_end_tag(void);

static struct tag * paramTag;
static struct RamConfig dramBanks[NR_DRAM_BANKS];

static int builtInInitrd = 0;

void doLinuxBoot(image_header_t * imageHeader, ulong loadAddr, ulong initrdAddr)
{
  ulong currentAddress = loadAddr + sizeof(image_header_t);
  u32int targetAddress = imageHeader->ih_load;
  u32int entryPoint = imageHeader->ih_ep;
  u32int sizeInBytes = imageHeader->ih_size;
  char * commandline = "\0";

  paramTag = (struct tag *)BOARD_PARAMS;

#ifdef STARTUP_DEBUG
  serial_putstring("Current address = ");
  serial_putint(currentAddress);
  serial_newline();
  serial_putstring("Load address    = ");
  serial_putint(targetAddress);
  serial_newline();
  serial_putstring("Entry point     = ");
  serial_putint(entryPoint);
  serial_newline();
#endif

  /* TODO: add virtual addressing startup for the new VM here
  need to create a Global Page table/map for the VM and add mappings for where the kernel is to be copied?
  */
  createVirtualMachineGPAtoRPA(getGuestContext());
#ifdef STARTUP_DEBUG
  serial_putstring("Creating of Virtual Machine Global Page table for VM done");
  serial_newline();
#endif


  if (currentAddress != targetAddress)
  {
    memmove((void*)targetAddress, (const void*)currentAddress, sizeInBytes);
  }
  populateDramBanks();

  setup_start_tag();
  setup_revision_tag();
  setup_memory_tags();
  setup_commandline_tag(commandline);
  if (builtInInitrd)
  {
    setup_initrd_tag(initrdAddr, initrdAddr + BOARD_INITRD_LEN);
  }
  setup_end_tag();

#ifdef DUMP_SCANNER_COUNTER
  resetScannerCounter();
#endif
  scanBlock(getGuestContext(), entryPoint);

  cleanupBeforeLinux();
#ifdef CONFIG_BLOCK_COPY
  //execution shouldn't be started at hdrEntryPoint any more!
  //The code from the blockCache should be executed  :  getGuestContext()->blockCopyCache
  //But first entry in blockCopyCache is backpointer -> next entry (blockCopyCache is u32int => +4)
  callKernel(0, (u32int)BOARD_MACHINE_ID, (u32int)BOARD_PARAMS, (getGuestContext()->blockCopyCache+4));
#else
  /* does not return */
  callKernel(0, (u32int)BOARD_MACHINE_ID, (u32int)BOARD_PARAMS, entryPoint);
#endif
}

void populateDramBanks()
{
  dramBanks[0].start = DRAM_BANK_1_START;
  dramBanks[0].size  = DRAM_BANK_1_SIZE;
  dramBanks[1].start = DRAM_BANK_2_START;
  dramBanks[1].size  = DRAM_BANK_2_SIZE;
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

static void setup_commandline_tag(char *commandline)
{
  char *p;

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
  paramTag->hdr.size = (sizeof (struct tag_header) + stringlen(p) + 1 + 4) >> 2;

  stringcpy(paramTag->u.cmdline.cmdline, p);

  paramTag = tag_next(paramTag);
}

static void setup_initrd_tag(ulong initrd_start, ulong initrd_end)
{
  /* an ATAG_INITRD node tells the kernel where the compressed
   * ramdisk can be found. ATAG_RDIMG is a better name, actually.
   */
  paramTag->hdr.tag = ATAG_INITRD2;
  paramTag->hdr.size = tag_size (tag_initrd);

  paramTag->u.initrd.start = initrd_start;
  paramTag->u.initrd.size = initrd_end - initrd_start;

  paramTag = tag_next (paramTag);
}

static void setup_end_tag ()
{
  paramTag->hdr.tag = ATAG_NONE;
  paramTag->hdr.size = 0;
}
