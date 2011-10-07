#ifndef __BLOCK_H__
#define __BLOCK_H__

#include "common/types.h"


typedef struct blockDev
{
  // total number of blocks
  u64int lba;
  u32int blockSize;
  int devID;
  u32int (*blockRead)(int devid, u32int start, u64int blockCount, void *dst);
  u32int (*blockWrite)(int devid, u32int start, u64int blockCount, const void *src);
} blockDevice;


#endif
