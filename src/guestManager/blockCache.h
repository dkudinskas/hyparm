#ifndef __GUEST_MANAGER__BLOCK_CACHE_H__
#define __GUEST_MANAGER__BLOCK_CACHE_H__

#ifdef CONFIG_THUMB2
# include "common/thumbdefs.h"
#endif

#include "common/types.h"

#ifdef CONFIG_BLOCK_COPY

#define BLOCK_CACHE_SIZE    128
#define BLOCK_COPY_CACHE_SIZE_IN_BYTES   (44 * BLOCK_CACHE_SIZE) // Here the assumption is taken that on average 10% of the instructions
                                                                 // will be critical. -> on average there are 10 instructions per block
                                                                 // cache.  + 1 backpointer -> 4B * 11 = 44 B
#define BLOCK_COPY_CACHE_SIZE (BLOCK_COPY_CACHE_SIZE_IN_BYTES / 4) // size in words (32 bits)

//uncomment to enable debugging: #define BLOCK_COPY_CACHE_DEBUG 1

#else

#define BLOCK_CACHE_SIZE    96

#endif

// uncomment me for block cache debug: #define BLOCK_CACHE_DBG

// uncomment me for collision debug: #define DUMP_COLLISION_COUNTER

struct blockCacheEntry
{
  u32int startAddress;
  u32int endAddress;
  u32int hyperedInstruction;
#ifdef CONFIG_BLOCK_COPY
  u32int reservedWord:1; //reservedWord is a flag that indicates that after the backpointer there will be 1 word that is reserved for saving
                         //a temporary value of a PC. This means code execution will start @ startAddress+8 (skip backpointer & reserved word)
  u32int blockCopyCacheSize:22; // blockCopyCacheSize will be rather limited
                            // there are 8 bits left -> can be used for profiling
  u32int blockCopyCacheAddress; // This is the address were the instructions with hypercall will reside
#endif
#ifdef CONFIG_THUMB2
  u32int halfhyperedInstruction;
#endif
  bool valid;
  u32int hdlFunct;
};


#ifdef CONFIG_THUMB2

struct thumbEntry
{
  u16int first;
  u16int second;
  bool isthumb;
};

#endif


typedef struct blockCacheEntry BCENTRY;

void initialiseBlockCache(BCENTRY * bcache);

bool checkBlockCache(u32int blkStartAddr, u32int bcIndex, BCENTRY * bcAddr);

#if defined(CONFIG_BLOCK_COPY)

void addToBlockCache(u32int blkStartAddr, u32int blkEndAddr, u32int index, u32int blockCopyCacheSize,
  u32int blockCopyCacheAddress,u32int hypInstruction,u32int hdlFunct,BCENTRY * bcAddr);

/* checkAndClearBlockCopyCacheAddress will check an address you provide and return a valid address.  Always use the returned address!! */
u32int *checkAndClearBlockCopyCacheAddress(u32int *Addr,BCENTRY *bcStartAddr,u32int* blockCopyCache,u32int* blockCopyCacheEnd);

u32int *checkAndMergeBlock(u32int* startOfBlock2, u32int* endOfBlock2, BCENTRY * blockCache,u32int* startOfBlock1,u32int* endOfBlock1);

u32int* updateCurrBlockCopyCacheAddr(u32int* oldAddr, u32int nrOfAddedInstr,u32int* blockCopyCacheEnd);

#elif defined(CONFIG_THUMB2)

void addToBlockCache(u32int blkStartAddr, u32int hypInstruction, u16int HalfhypInstruction, u32int blkEndAddr,
  u32int index, u32int hdlFunct, BCENTRY * bcAddr);

#else

void addToBlockCache(u32int blkStartAddr, u32int hypInstruction, u32int blkEndAddr,
  u32int index, u32int hdlFunct, BCENTRY * bcAddr);

#endif

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

#ifdef CONFIG_THUMB2

struct thumbEntry BreakDownThumb(BCENTRY *bcAddr, u32int bcIndex);

void resolveSWI(u32int index, u32int* endAddress);

#endif

#endif /* __GUEST_MANAGER__BLOCK_CACHE_H__ */
