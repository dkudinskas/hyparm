#include "common/debug.h"
#include "common/memFunctions.h"

#include "guestManager/blockCache.h"

#include "instructionEmu/decoder.h"


#ifdef CONFIG_BLOCK_CACHE_COLLISION_COUNTER

u64int collisionCounter;

static inline u64int getCollisionCounter(void);
static inline void incrementCollisionCounter(void);
static inline void resetCollisionCounter(void);

static inline u64int getCollisionCounter()
{
  return collisionCounter;
}

static inline void incrementCollisionCounter()
{
  collisionCounter++;
}

static inline void resetCollisionCounter()
{
  collisionCounter = 0;
}

#else

#define getCollisionCounter()        (0ULL)
#define incrementCollisionCounter()
#define resetCollisionCounter()

#endif


#define NUMBER_OF_BITMAPS       16
#define MEMORY_PER_BITMAP       0x10000000
#define MEMORY_PER_BITMAP_BIT   (MEMORY_PER_BITMAP / 32) // should be 8 megabytes


static u32int execBitMap[NUMBER_OF_BITMAPS];


static void clearExecBitMap(u32int address);
static u32int findBlockCacheEntry(BCENTRY *blockCache, u32int address);
static bool isBitmapSetForAddress(u32int address);
static void removeCacheEntries(BCENTRY *blockCache);
static void removeCacheEntry(BCENTRY *blockCache, u32int index);
static void resolveCacheConflict(BCENTRY *blockCache, u32int index);
static void restoreReplacedInstruction(BCENTRY *blockCache, u32int index);
static void setExecBitMap(u32int address);


void addToBlockCache(BCENTRY *blockCache, u32int index, u32int startAddress, u32int endAddress,
    u32int hypInstruction, u32int type, void *hdlFunct)
{
  DEBUG(BLOCK_CACHE, "addToBlockCache: index = %#.2x @ %#.8x--%#.8x, handler = %p, eobInstr = "
      "%#.8x" EOL, index, startAddress, endAddress, hdlFunct, hypInstruction);

  if (blockCache[index].type != BCENTRY_TYPE_INVALID)
  {
    if (blockCache[index].endAddress != endAddress)
    {
      // somebody has been sleeping in our cache location!
      resolveCacheConflict(blockCache, index);
      // now that we resolved the conflict, we can store the new entry data...
      blockCache[index].endAddress = endAddress;
      blockCache[index].hyperedInstruction = hypInstruction;
      blockCache[index].hdlFunct = hdlFunct;
      blockCache[index].type = type;
    }
    /* NOTE: if entry valid, but blkEndAddress is the same as new block to add      *
     * then the block starts at another address but ends on the same instruction    *
     * and by chance - has the same index. just modify existing entry, don't remove */
  }
  else
  {
    blockCache[index].endAddress = endAddress;
    blockCache[index].hyperedInstruction = hypInstruction;
    blockCache[index].hdlFunct = hdlFunct;
    blockCache[index].type = type;
  }
  blockCache[index].startAddress = startAddress;

  // set bitmap entry to executed
  setExecBitMap((u32int)endAddress);
}

bool checkBlockCache(BCENTRY *blockCache, u32int index, u32int startAddress)
{
  DEBUG(BLOCK_CACHE, "checkBlockCache: index = %#x" EOL, index);
  return blockCache[index].type != BCENTRY_TYPE_INVALID
      && blockCache[index].startAddress == startAddress;
}

void clearBlockCache(BCENTRY *blockCache)
{
  removeCacheEntries(blockCache);
  memset(execBitMap, 0, sizeof(u32int) * NUMBER_OF_BITMAPS);
}

static void clearExecBitMap(u32int address)
{
  u32int index = address / MEMORY_PER_BITMAP;
  u32int bitNumber = (address & 0x0FFFFFFF) / MEMORY_PER_BITMAP_BIT;
  execBitMap[index] = execBitMap[index] & ~(1 << bitNumber);
}

void dumpBlockCacheEntry(BCENTRY *blockCache, u32int index)
{
  printf(
      "dumpBlockCacheEntry: entry #%#.2x: " EOL
      "dumpBlockCacheEntry: startAddress = %#.8x, endAddress = %#.8x, type = %x" EOL
      "dumpBlockCacheEntry: EOBinstr = %#.8x, handlerFunction = %p" EOL,
      index,
      blockCache[index].startAddress, blockCache[index].endAddress, blockCache[index].type,
      blockCache[index].hyperedInstruction, blockCache[index].hdlFunct
      );
}

/* input: any address, might be start, end of block or somewhere in the middle... */
/* output: first cache entry index for the block where this address falls into */
/* output: if no such block, return -1 (0xFFFFFFFF) */
static u32int findBlockCacheEntry(BCENTRY *blockCache, u32int address)
{
  u32int i;
  for (i = 0; i < BLOCK_CACHE_SIZE; i++)
  {
    if (blockCache[i].type != BCENTRY_TYPE_INVALID
        && blockCache[i].startAddress <= address && blockCache[i].endAddress >= address)
    {
      // addr falls in-between start-end inclusive. found a matching entry.
      DEBUG(BLOCK_CACHE, "findEntryForAddress: found entry for address %#.8x @ %#.8x--%#.8x, "
          "index = %#x" EOL, address, blockCache[i].startAddress, blockCache[i].endAddress, i);
      return i;
    }
  }
  return (u32int)-1;
}

BCENTRY *getBlockCacheEntry(BCENTRY *blockCache, u32int index)
{
  DEBUG(BLOCK_CACHE, "getBlockCacheEntry: index = %#x" EOL, index);
  return &blockCache[index];
}

void initialiseBlockCache(BCENTRY *blockCache)
{
  resetCollisionCounter();

  DEBUG(BLOCK_CACHE, "initialiseBlockCache: @ %p" EOL, blockCache);

  memset(blockCache, 0, sizeof(BCENTRY) * BLOCK_CACHE_SIZE);
  memset(execBitMap, 0, sizeof(u32int) * NUMBER_OF_BITMAPS);
}


static bool isBitmapSetForAddress(u32int address)
{
  u32int index = address / MEMORY_PER_BITMAP;
  u32int bitNumber = (address & 0x0FFFFFFF) / MEMORY_PER_BITMAP_BIT;
  return execBitMap[index] & (1 << bitNumber);
}

static void removeCacheEntries(BCENTRY *blockCache)
{
  /*
   * Restore and invalidate all cache entries
   */
  u32int i;
  for (i = 0; i < BLOCK_CACHE_SIZE; i++)
  {
    restoreReplacedInstruction(blockCache, i);
    blockCache[i].type = BCENTRY_TYPE_INVALID;
  }
}

static void removeCacheEntry(BCENTRY *blockCache, u32int index)
{
  /*
   * Restore and invalidate a single cache entry
   */
  restoreReplacedInstruction(blockCache, index);
  blockCache[index].type = BCENTRY_TYPE_INVALID;
}

static void resolveCacheConflict(BCENTRY *blockCache, u32int index)
{
  /*
    Replacement policy: SIMPLE REPLACE
    Collision: new block is trying to replace old block in cache
    Steps to Carry out:
    1. scan the cache for any other blocks that end with the same instruction
    as the old block in the cache
    2.1. if found, get hypercall, and update SWIcode to point to found entry
    2.2. if not found, restore hypered instruction back!
   */
  DEBUG(BLOCK_CACHE, "resolveCacheConflict: collision at index %#x" EOL, index);

  incrementCollisionCounter();

  u32int i;
  for (i = 0; i < BLOCK_CACHE_SIZE; i++)
  {
    if (blockCache[i].type != BCENTRY_TYPE_INVALID
        && blockCache[i].endAddress == blockCache[index].endAddress && i != index)
    {
      /*
       * Found a valid entry in the cache, for which the block ends at the same address as the block
       * of the entry we collided with. Update the SVC code of the new entry to the old index and
       * return.
       *
       * We assume that both blocks are of the same type. Since we cannot switch between ARM and
       * Thumb within a block, this assumption is valid. Hence, we only have to check the type of
       * one of the entries to figure out whether we are dealing with ARM or Thumb entries. We use
       * blockCache[index] because it yields the smallest compiled code (less spilling).
       */
      DEBUG(BLOCK_CACHE, "resolveCacheConflict: found another block ending at the same address" EOL);
#ifdef CONFIG_THUMB2
      if (blockCache[index].type == BCENTRY_TYPE_ARM)
      {
#endif
        u32int hyperCall = (*(u32int *)blockCache[index].endAddress & 0xFF000000) | ((i + 1) << 8);
        DEBUG(BLOCK_CACHE, "resolveCacheConflict: replacing ARM hypercall with %#.8x" EOL,
            hyperCall);
        *(u32int *)blockCache[index].endAddress = hyperCall;
#ifdef CONFIG_THUMB2
      }
      else
      {
        u16int hyperCall = (*(u16int *)blockCache[index].endAddress & 0x0000FF00) | (i + 1);
        DEBUG(BLOCK_CACHE, "resolveCacheConflict: replacing T16 hypercall with %#.4x" EOL,
            hyperCall);
        *(u16int *)blockCache[index].endAddress = hyperCall;
      }
#endif
      return;
    }
  }
  DEBUG(BLOCK_CACHE, "resolveCacheConflict: no other block ends at the same address" EOL);

  /*
   * Restore replaced instruction (hyperedInstruction) back to its original location in memory.
   */
  restoreReplacedInstruction(blockCache, index);
}

static void restoreReplacedInstruction(BCENTRY *blockCache, u32int index)
{
  switch (blockCache[index].type)
  {
    case BCENTRY_TYPE_ARM:
      DEBUG(BLOCK_CACHE, "restoreReplacedInstruction: restoring ARM %#.8x @ %#.8x" EOL,
          blockCache[index].hyperedInstruction, blockCache[index].endAddress);
      *((u32int*)(blockCache[index].endAddress)) = blockCache[index].hyperedInstruction;
      break;
#ifdef CONFIG_THUMB2
    case BCENTRY_TYPE_THUMB:
      if (TXX_IS_T32(blockCache[index].hyperedInstruction))
      {
        /*
         * Restore Thumb 32-bit instruction. Word-alignment is not guaranteed, so we must perform
         * two halfword-size stores!
         */
        DEBUG(BLOCK_CACHE, "resolveCacheConflict: restoring T32 %#.8x @ %#.8x",
            blockCache[index].hyperedInstruction, blockCache[index].endAddress);
        u16int *bpointer = (u16int *)(blockCache[index].endAddress);
        *bpointer = (u16int)(blockCache[index].hyperedInstruction & 0xFFFF);
        bpointer--;
        *bpointer = (u16int)(blockCache[index].hyperedInstruction >> 16);
      }
      else
      {
        /*
         * Restore Thumb 16-bit instruction.
         */
        DEBUG(BLOCK_CACHE, "resolveCacheConflict: restoring T16 %#.4x @ %#.8x" EOL,
            blockCache[index].hyperedInstruction, blockCache[index].endAddress);
        *((u16int *)(blockCache[index].endAddress)) = (u16int)blockCache[index].hyperedInstruction;
      }
      break;
#endif
  }
}

static void setExecBitMap(u32int address)
{
  u32int index = address / MEMORY_PER_BITMAP;
  u32int bitNumber = (address & 0x0FFFFFFF) / MEMORY_PER_BITMAP_BIT;
  execBitMap[index] = execBitMap[index] | (1 << bitNumber);
}

// finds and clears block cache entries within the given address range
void validateCacheMultiPreChange(BCENTRY *bcache, u32int startAddress, u32int endAddress)
{
  DEBUG(BLOCK_CACHE, "validateCacheMultiPreChange: %#.8x--%#.8x" EOL, startAddress, endAddress);
  u32int i;
  for (i = 0; i < BLOCK_CACHE_SIZE; i++)
  {
    if (bcache[i].type != BCENTRY_TYPE_INVALID && bcache[i].endAddress >= startAddress
        && bcache[i].endAddress <= endAddress)
    {
      //We only care if the end address of the block falls inside the address validation range
      removeCacheEntry(bcache, i);
    }
  }
}

// finds block cache entries that include a given address, clears them
void validateCachePreChange(BCENTRY *bcache, u32int address)
{
  if (isBitmapSetForAddress(address))
  {
    u32int cacheIndex = 0;
    while((cacheIndex = findBlockCacheEntry(bcache, address)) != (u32int)-1)
    {
      removeCacheEntry(bcache, cacheIndex);
    }
  }
}
