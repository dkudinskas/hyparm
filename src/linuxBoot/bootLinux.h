#ifndef __LINUX_BOOT__BOOT_LINUX_H__
#define __LINUX_BOOT__BOOT_LINUX_H__

#include "common/types.h"

#include "linuxBoot/image.h"


//#define STARTUP_DEBUG

#define BOARD_MACHINE_ID        0x60A
#define BOARD_REVISION           0x20
#define NR_DRAM_BANKS               2
#define DRAM_BANK_1_START  0x80000000
#define DRAM_BANK_1_SIZE   0x08000000
#define DRAM_BANK_2_START  0x88000000
// TELL LINUX we have less memory than we actually do
// top 16MB (0x8f000000 - 0x8fffffff) is for hypervisor use.
#define DRAM_BANK_2_SIZE   0x07000000
#define BOARD_PARAMS       0x80000100
// hardcoded initrd len?!
#define BOARD_INITRD_LEN     0x800000

#define tag_next(t)  ((struct tag *)((u32int *)(t) + (t)->hdr.size))
#define tag_size(type)  ((sizeof(struct tag_header) + sizeof(struct type)) >> 2)

/* The list ends with an ATAG_NONE node. */
#define ATAG_NONE  0x00000000
/* The list must start with an ATAG_CORE node */
#define ATAG_CORE  0x54410001
/* board revision */
#define ATAG_REVISION  0x54410007
/* it is allowed to have multiple ATAG_MEM nodes */
#define ATAG_MEM  0x54410002
/* VGA text type displays */
/* describes where the compressed ramdisk image lives (virtual address) */
/*
 * this one accidentally used virtual addresses - as such,
 * its depreciated.
 */
#define ATAG_INITRD  0x54410005
/* describes where the compressed ramdisk image lives (physical address) */
#define ATAG_INITRD2  0x54420005
/* initial values for vesafb-type framebuffers. see struct screen_info
 * in include/linux/tty.h
 */
#define ATAG_CMDLINE  0x54410009

void doLinuxBoot(image_header_t * imageHeader, ulong loadAddr, ulong initrdAddr)  __attribute__((noreturn));

void populateDramBanks(void);

struct RamConfig
{
  ulong start;
  ulong size;
};

struct tag_header {
  u32int size;
  u32int tag;
};

struct tag_core {
  u32int flags;    /* bit 0 = read-only */
  u32int pagesize;
  u32int rootdev;
};

struct tag_mem32 {
  u32int  size;
  u32int  start;  /* physical start address */
};

struct tag_videotext {
  u8int    x;
  u8int    y;
  u16int    video_page;
  u8int    video_mode;
  u8int    video_cols;
  u16int    video_ega_bx;
  u8int    video_lines;
  u8int    video_isvga;
  u16int    video_points;
};

struct tag_ramdisk {
  u32int flags;  /* bit 0 = load, bit 1 = prompt */
  u32int size;  /* decompressed ramdisk size in _kilo_ bytes */
  u32int start;  /* starting block of floppy-based RAM disk image */
};

struct tag_initrd {
  u32int start;  /* physical start address */
  u32int size;  /* size of compressed ramdisk image in bytes */
};

struct tag_serialnr {
  u32int low;
  u32int high;
};

struct tag_revision {
  u32int rev;
};

struct tag_videolfb {
  u16int    lfb_width;
  u16int    lfb_height;
  u16int    lfb_depth;
  u16int    lfb_linelength;
  u32int    lfb_base;
  u32int    lfb_size;
  u8int    red_size;
  u8int    red_pos;
  u8int    green_size;
  u8int    green_pos;
  u8int    blue_size;
  u8int    blue_pos;
  u8int    rsvd_size;
  u8int    rsvd_pos;
};

struct tag_cmdline {
  char  cmdline[1];  /* this is the minimum size */
};

struct tag_acorn {
  u32int memc_control_reg;
  u32int vram_pages;
  u8int sounddefault;
  u8int adfsdrives;
};

struct tag_memclk {
  u32int fmemclk;
};

struct tag {
  struct tag_header hdr;
  union {
    struct tag_core    core;
    struct tag_mem32  mem;
    struct tag_videotext  videotext;
    struct tag_ramdisk  ramdisk;
    struct tag_initrd  initrd;
    struct tag_serialnr  serialnr;
    struct tag_revision  revision;
    struct tag_videolfb  videolfb;
    struct tag_cmdline  cmdline;

    /*
     * Acorn specific
     */
    struct tag_acorn  acorn;

    /*
     * DC21285 specific
     */
    struct tag_memclk  memclk;
  } u;
};

#endif
