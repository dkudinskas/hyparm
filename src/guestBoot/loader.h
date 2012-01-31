#ifndef __GUEST_BOOT__LOADER_H__
#define __GUEST_BOOT__LOADER_H__ 1

#include "common/types.h"

#include "guestManager/guestContext.h"


struct MemoryBank;
struct tag;


void bootGuest(GCONTXT *context, u32int entryPoint) __attribute__((noreturn));

struct tag *getTagListBaseAddress(void);

void setupStartTag(struct tag **tag);
void setupRevisionTag(struct tag **tag);
void setupInitrdTag(struct tag **tag, u32int initrdStart, u32int initrdEnd);
void setupMemoryTag(struct tag **tag, u32int startAddress, u32int size);
void setupMemoryTags(struct tag **tag);
void setupCommandLineTag(struct tag **tag, const char *commandLine);
void setupEndTag(struct tag **tag);


struct tag_header {
  u32int size;
  u32int tag;
};

struct tag_core {
  u32int flags;  /* bit 0 = read-only */
  u32int pagesize;
  u32int rootdev;
};

struct tag_mem32 {
  u32int size;
  u32int start;  /* physical start address */
};

struct tag_videotext {
  u8int  x;
  u8int  y;
  u16int video_page;
  u8int  video_mode;
  u8int  video_cols;
  u16int video_ega_bx;
  u8int  video_lines;
  u8int  video_isvga;
  u16int video_points;
};

struct tag_ramdisk
{
  u32int flags;  /* bit 0 = load, bit 1 = prompt */
  u32int size;   /* decompressed ramdisk size in _kilo_ bytes */
  u32int start;  /* starting block of floppy-based RAM disk image */
};

struct tag_initrd
{
  u32int start;  /* physical start address */
  u32int size;   /* size of compressed ramdisk image in bytes */
};

struct tag_serialnr
{
  u32int low;
  u32int high;
};

struct tag_revision
{
  u32int rev;
};

struct tag_videolfb
{
  u16int lfb_width;
  u16int lfb_height;
  u16int lfb_depth;
  u16int lfb_linelength;
  u32int lfb_base;
  u32int lfb_size;
  u8int  red_size;
  u8int  red_pos;
  u8int  green_size;
  u8int  green_pos;
  u8int  blue_size;
  u8int  blue_pos;
  u8int  rsvd_size;
  u8int  rsvd_pos;
};

struct tag_cmdline {
  char   cmdline[1];  /* this is the minimum size */
};

struct tag_acorn
{
  u32int memc_control_reg;
  u32int vram_pages;
  u8int  sounddefault;
  u8int  adfsdrives;
};

struct tag_memclk
{
  u32int fmemclk;
};

struct tag
{
  struct tag_header hdr;
  union
  {
    struct tag_core      core;
    struct tag_mem32     mem;
    struct tag_videotext videotext;
    struct tag_ramdisk   ramdisk;
    struct tag_initrd    initrd;
    struct tag_serialnr  serialnr;
    struct tag_revision  revision;
    struct tag_videolfb  videolfb;
    struct tag_cmdline   cmdline;
    /*
     * Acorn specific
     */
    struct tag_acorn     acorn;
    /*
     * DC21285 specific
     */
    struct tag_memclk    memclk;
  } u;
};

#endif
