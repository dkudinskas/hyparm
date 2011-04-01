#ifndef __GUEST_MANAGER__BLOCK_CACHE_H__
#define __GUEST_MANAGER__BLOCK_CACHE_H__

#include "common/types.h"


#define BLOCK_CACHE_SIZE    96

// uncomment me for block cache debug: #define BLOCK_CACHE_DBG

// uncomment me for collision debug: #define DUMP_COLLISION_COUNTER

struct blockCacheEntry
{
  u32int startAddress;
  u32int endAddress;
  u32int hyperedInstruction;
  bool valid;
  u32int hdlFunct;
};

typedef struct blockCacheEntry BCENTRY;

void initialiseBlockCache(BCENTRY * bcache);

bool checkBlockCache(u32int blkStartAddr, u32int bcIndex, BCENTRY * bcAddr);

void addToBlockCache(u32int blkStartAddr, u32int hypInstruction, u32int blkEndAddr,
                     u32int index, u32int hdlFunct, BCENTRY * bcAddr);

BCENTRY * getBlockCacheEntry(u32int index, BCENTRY * bcAddr);

u32int findEntryForAddress(BCENTRY * bcAddr, u32int addr);

void removeCacheEntry(BCENTRY * bcAddr, u32int cacheIndex);

void resolveCacheConflict(u32int index, BCENTRY * bcAddr);

void explodeCache(BCENTRY * bcache);

void validateCachePreChange(BCENTRY * bcache, u32int address);

void validateCacheMultiPreChange(BCENTRY * bcache, u32int startAddress, u32int endAddress);

void dumpBlockCacheEntry(u32int index, BCENTRY * bcache);

void setExecBitMap(u32int addr);

void clearExecBitMap(u32int addr);

bool isBitmapSetForAddress(u32int addr);

#endif
