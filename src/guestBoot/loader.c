#include "common/debug.h"
#include "common/string.h"

#include "cpuArch/armv7.h"

#include "exceptions/exceptionHandlers.h"

#include "guestBoot/loader.h"

#include "instructionEmu/scanner.h"

#include "memoryManager/addressing.h"


/*
 * The list of tags must start with an ATAG_CORE node
 */
#define ATAG_CORE      0x54410001
/*
 * The list of tags ends with an ATAG_NONE node
 */
#define ATAG_NONE      0x00000000

/*
 * Kernel command line
 */
#define ATAG_CMDLINE  0x54410009
/*
 * ATAG_INITRD and ATAG_INITRD2 describe where the compressed ramdisk image lives:
 * - ATAG_INITRD: by virtual address (deprecated);
 * - ATAG_INITRD2: by physical address.
 */
#define ATAG_INITRD    0x54410005
#define ATAG_INITRD2   0x54420005
/*
 * The tag list may contain multiple ATAG_MEM nodes
 */
#define ATAG_MEM       0x54410002
/*
 * Board revision
 */
#define ATAG_REVISION  0x54410007
/*
 * Other tags:
 * For initial values for vesafb-type framebuffers. see struct screen_info in include/linux/tty.h
 */

/*
 * Base address of the tag list
 */
#define TAG_LIST_ADDRESS   0x80000100

/*
 * Board revision as used in ATAG_REVISION
 */
#define BOARD_REVISION           0x20

/*
 * Board machine ID passed to the guest kernel
 */
#define BOARD_MACHINE_ID        0x60A


/*
 * Hide top 16 MB of memory from the guest; 0x8f000000--0x8fffffff is for hypervisor use.
 */
#define DRAM_BANK_1_START  0x80000000
#define DRAM_BANK_1_SIZE   0x08000000
#define DRAM_BANK_2_START  0x88000000
#define DRAM_BANK_2_SIZE   0x07000000


#define TAG_NEXT(t)     ((struct tag *)((u32int *)(t) + (t)->hdr.size))
#define TAG_SIZE(type)  ((sizeof(struct tag_header) + sizeof(struct type)) >> 2)


extern void callKernel(s32int, s32int, struct tag *tagList, u32int entryPoint) __attribute__((noreturn));


void bootGuest(GCONTXT *context, u32int entryPoint)
{
  DEBUG(STARTUP, "bootGuest: entryPoint = %#.8x" EOL, entryPoint);
  /*
   * TODO: add virtual addressing startup for the new VM here.
   * Need to create a Global Page table/map for the VM and add mappings for where the kernel is to
   * be copied?
   */
  createVirtualMachineGPAtoRPA(context);
  /*
   * Reset exception counters
   */
  resetDataAbortCounter();
  resetIrqCounter();
  resetSvcCounter();
  /*
   * Scan initial block
   */
  setScanBlockCallSource(SCANNER_CALL_SOURCE_BOOT);
  resetScanBlockCounter();
  scanBlock(context, entryPoint);

  cleanupBeforeBoot();

#ifdef CONFIG_BLOCK_COPY
  //execution shouldn't be started at hdrEntryPoint any more!
  //The code from the blockCache should be executed  :  getGuestContext()->blockCopyCache
  //But first entry in blockCopyCache is backpointer -> next entry (blockCopyCache is u32int => +4)
  entryPoint = context->blockCopyCache + 4;
#endif

  DEBUG(STARTUP, "bootGuest: callKernel" EOL);
  callKernel(0, BOARD_MACHINE_ID, getTagListBaseAddress(), entryPoint);
}

struct tag *getTagListBaseAddress()
{
  return (struct tag *)TAG_LIST_ADDRESS;
}

void setupCommandLineTag(struct tag **tag, const char *commandLine)
{
  /*
   * Do not set up a tag for NULL and empty command line strings, so the target kernel will use its
   * default (built-in) command line.
   */
  if (!commandLine)
  {
    return;
  }
  while (*commandLine == ' ')
  {
    commandLine++;
  }
  if (!*commandLine)
  {
    return;
  }
  /*
   * Command line string is not empty.
   */
  (*tag)->hdr.tag = ATAG_CMDLINE;
  (*tag)->hdr.size = (sizeof(struct tag_header) + strlen(commandLine) + 1 + 4) >> 2;
  strcpy((*tag)->u.cmdline.cmdline, commandLine);

  *tag = TAG_NEXT(*tag);
}

void setupEndTag(struct tag **tag)
{
  (*tag)->hdr.tag = ATAG_NONE;
  (*tag)->hdr.size = 0;
}

void setupInitrdTag(struct tag **tag, u32int initrdStart, u32int initrdEnd)
{
  (*tag)->hdr.tag = ATAG_INITRD2;
  (*tag)->hdr.size = TAG_SIZE(tag_initrd);

  (*tag)->u.initrd.start = initrdStart;
  (*tag)->u.initrd.size = initrdEnd - initrdStart;

  *tag = TAG_NEXT(*tag);
}

void setupMemoryTag(struct tag **tag, u32int startAddress, u32int size)
{
  (*tag)->hdr.tag = ATAG_MEM;
  (*tag)->hdr.size = TAG_SIZE(tag_mem32);

  (*tag)->u.mem.start = startAddress;
  (*tag)->u.mem.size = size;

  *tag = TAG_NEXT(*tag);
}

void setupMemoryTags(struct tag **tag)
{
  setupMemoryTag(tag, DRAM_BANK_1_START, DRAM_BANK_1_SIZE);
  setupMemoryTag(tag, DRAM_BANK_2_START, DRAM_BANK_2_SIZE);
}

void setupRevisionTag(struct tag **tag)
{
  u32int rev = BOARD_REVISION;

  (*tag)->hdr.tag = ATAG_REVISION;
  (*tag)->hdr.size = TAG_SIZE(tag_revision);
  (*tag)->u.revision.rev = rev;

  (*tag) = TAG_NEXT(*tag);
}

void setupStartTag(struct tag **tag)
{
  (*tag)->hdr.tag = ATAG_CORE;
  (*tag)->hdr.size = TAG_SIZE(tag_core);

  (*tag)->u.core.flags = 0;
  (*tag)->u.core.pagesize = 0;
  (*tag)->u.core.rootdev = 0;

  *tag = TAG_NEXT(*tag);
}
