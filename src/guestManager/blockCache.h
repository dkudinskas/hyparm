#ifndef __GUEST_MANAGER__BLOCK_CACHE_H__
#define __GUEST_MANAGER__BLOCK_CACHE_H__

#include "common/types.h"


#define BLOCK_CACHE_SIZE    96


#define BCENTRY_TYPE_INVALID  0
#define BCENTRY_TYPE_ARM      1
#define BCENTRY_TYPE_THUMB    2


struct blockCacheEntry
{
  u32int startAddress;
  u32int endAddress;
  u32int hyperedInstruction;
  u32int type;
  void *hdlFunct;
};

typedef struct blockCacheEntry BCENTRY;


void addToBlockCache(BCENTRY *blockCache, u32int index, u32int startAddress, u32int endAddress,
    u32int hypInstruction, u32int type, void *hdlFunct);

bool checkBlockCache(BCENTRY *blockCache, u32int index, u32int startAddress);

void clearBlockCache(BCENTRY *blockCache);

void dumpBlockCacheEntry(BCENTRY *blockCache, u32int index);

BCENTRY *getBlockCacheEntry(BCENTRY *blockCache, u32int index);

void initialiseBlockCache(BCENTRY *blockCache);

void validateCacheMultiPreChange(BCENTRY * bcache, u32int startAddress, u32int endAddress);

void validateCachePreChange(BCENTRY * bcache, u32int address);


#endif
