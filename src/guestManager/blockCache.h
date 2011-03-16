#ifndef __BLOCK_CACHE_H__
#define __BLOCK_CACHE_H__

#include "types.h"
#include "common.h"

#define BLOCK_CACHE_SIZE    128
#define BLOCK_COPY_CACHE_SIZE_IN_BYTES   (44 * BLOCK_CACHE_SIZE) // Here the assumption is taken that on average 10% of the instructions
                                                                 // will be critical. -> on average there are 10 instructions per block
                                                                 // cache.  + 1 backpointer -> 4B * 11 = 44 B
#define BLOCK_COPY_CACHE_SIZE (BLOCK_COPY_CACHE_SIZE_IN_BYTES / 4) // size in words (32 bits)

// uncomment me for block cache debug: #define BLOCK_CACHE_DEBUG

// uncomment me for collision debug: #define DUMP_COLLISION_COUNTER

struct blockCacheEntry
{
  u32int startAddress;
  u32int endAddress;
  u32int hyperedInstruction;
  u32int valid:1; //valid is a flag and needs only 1 bit
  u32int reservedWord:1; //reservedWord is a flag that indicates that after the backpointer there will be 1 word that is reserved for saving
                         //a temporary value of a PC. This means code execution will start @ startAddress+8 (skip backpointer & reserved word)
  u32int blockCopyCacheSize:22; // blockCopyCacheSize will be rather limited
                            // there are 8 bits left -> can be used for profiling
  u32int blockCopyCacheAddress; // This is the address were the instructions with hypercall will reside
  u32int hdlFunct;
};

typedef struct blockCacheEntry BCENTRY;

void initialiseBlockCache(BCENTRY * bcache);

bool checkBlockCache(u32int blkStartAddr, u32int bcIndex, BCENTRY * bcAddr);

void addToBlockCache(u32int blkStartAddr, u32int blkEndAddr,
                     u32int index, u32int blockCopyCacheSize, u32int blockCopyCacheAddress,u32int hypInstruction,u32int hdlFunct,BCENTRY * bcAddr);

/* checkAndClearBlockCopyCacheAddress will check an address you provide and return a valid address.  Always use the returned address!! */
u32int * checkAndClearBlockCopyCacheAddress(u32int *Addr,BCENTRY *bcStartAddr,u32int* blockCopyCache,u32int* blockCopyCacheEnd);

BCENTRY * getBlockCacheEntry(u32int index, BCENTRY * bcAddr);

u32int findEntryForAddress(BCENTRY * bcAddr, u32int addr);

void removeCacheEntry(BCENTRY * bcAddr, u32int cacheIndex);

//Remove the copied instructions
void removeBlockCopyCacheEntry(u32int blockCopyCacheAddress,u32int blockCopyCacheSize);

void resolveCacheConflict(u32int index, BCENTRY * bcAddr);

//Remove all cache entries.  All logbook entries and all copied instructions will be removed.
void explodeCache(BCENTRY * bcache);

void validateCachePreChange(BCENTRY * bcache, u32int address);

void validateCacheMultiPreChange(BCENTRY * bcache, u32int startAddress, u32int endAddress);

void dumpBlockCacheEntry(u32int index, BCENTRY * bcache);

void setExecBitMap(u32int addr);

void clearExecBitMap(u32int addr);

bool isBitmapSetForAddress(u32int addr);

u32int* updateCurrBlockCopyCacheAddr(u32int* oldAddr, u32int nrOfAddedInstr,u32int* blockCopyCacheEnd);

#endif
