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

#endif /* CONFIG_BLOCK_COPY */


#define BCENTRY_TYPE_INVALID  0
#define BCENTRY_TYPE_ARM      1
#define BCENTRY_TYPE_THUMB    2


struct blockCacheEntry
{
  u32int startAddress;
  u32int endAddress;
  u32int hyperedInstruction;
  u32int type;
#ifdef CONFIG_BLOCK_COPY
  u32int reservedWord:1; //reservedWord is a flag that indicates that after the backpointer there will be 1 word that is reserved for saving
                         //a temporary value of a PC. This means code execution will start @ startAddress+8 (skip backpointer & reserved word)
  u32int blockCopyCacheSize:22; // blockCopyCacheSize will be rather limited
                            // there are 8 bits left -> can be used for profiling
  u32int blockCopyCacheAddress; // This is the address were the instructions with hypercall will reside
#endif
  void *hdlFunct;
};

typedef struct blockCacheEntry BCENTRY;


#ifdef CONFIG_BLOCK_COPY

void addToBlockCache(BCENTRY *blockCache, u32int index, u32int startAddress, u32int endAddress,
    u32int hypInstruction, void *hdlFunct, u32int blockCopyCacheSize, u32int blockCopyCacheAddress);

/* checkAndClearBlockCopyCacheAddress will check an address you provide and return a valid address.  Always use the returned address!! */
u32int *checkAndClearBlockCopyCacheAddress(u32int *Addr,BCENTRY *bcStartAddr,u32int* blockCopyCache,u32int* blockCopyCacheEnd);

u32int *checkAndMergeBlock(u32int* startOfBlock2, u32int* endOfBlock2, BCENTRY * blockCache,u32int* startOfBlock1,u32int* endOfBlock1);

u32int *updateCurrBlockCopyCacheAddr(u32int* oldAddr, u32int nrOfAddedInstr,u32int* blockCopyCacheEnd);

//Remove the copied instructions
void removeBlockCopyCacheEntry(void *context, u32int blockCopyCacheAddress, u32int blockCopyCacheSize);

#else

void addToBlockCache(BCENTRY *blockCache, u32int index, u32int startAddress, u32int endAddress,
    u32int hypInstruction, u32int type, void *hdlFunct);

#endif /* CONFIG_BLOCK_COPY */

bool checkBlockCache(BCENTRY *blockCache, u32int index, u32int startAddress);

void clearBlockCache(BCENTRY *blockCache);

void dumpBlockCacheEntry(BCENTRY *blockCache, u32int index);

BCENTRY *getBlockCacheEntry(BCENTRY *blockCache, u32int index);

void initialiseBlockCache(BCENTRY *blockCache);

void validateCacheMultiPreChange(BCENTRY * bcache, u32int startAddress, u32int endAddress);

void validateCachePreChange(BCENTRY * bcache, u32int address);


#endif /* __GUEST_MANAGER__BLOCK_CACHE_H__ */
