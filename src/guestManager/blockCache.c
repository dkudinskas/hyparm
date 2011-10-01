#include "common/debug.h"
#include "common/memFunctions.h"

#include "guestManager/blockCache.h"

#include "instructionEmu/commonInstrFunctions.h"


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
static void resolveSWI(u32int index, u32int *endAddress);
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
#ifdef CONFIG_THUMB2
      /* found a valid entry in the cache, that BB ends @ the same address as
      * the block of the entry that we collided with.
      * Call the resolve function to ensure that conflicts between Thumb and ARM SWIs
      * will be resolved as approprate otherwise say 'hello' to segfault ^_^
      * ARM: SWI 0xEF<code> is replaced with SWI<newcode> where newcode points new entry
      * Thumb: SWI 0xDF<code> is replaced with SWI<newcode> where newcode points new entry
      */
      resolveSWI(i, (u32int*)blockCache[index].endAddress);
#else
      // found a valid entry in the cache, that BB ends @ the same address as
      // the block of the entry that we collided with.
      u32int hypercallSWI = *((u32int*)(blockCache[index].endAddress));
      // SWI 0xEF<code> is replaced with SWI<newcode> where newcode points new entry
      hypercallSWI = (hypercallSWI & 0xFF000000) | ((i + 1) << 8);
      DEBUG(BLOCK_CACHE, "resolveCacheConflict: found another block ending at the same address" EOL);
      DEBUG(BLOCK_CACHE, "resolveCacheConflict: replacing hypercall with %#.8x" EOL, hypercallSWI);
      *((u32int*)(blockCache[index].endAddress)) = hypercallSWI;
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











#ifdef CONFIG_THUMB2

static void resolveSWI( u32int index, u32int * endAddress)
{
  u32int hypercall = 0;
  // Ok so endAddress holds the SWI we collided with. Check if it is word aligned
  if(((u32int)endAddress & 0x3) >= 0x2)
  {
    //Not word aligned. It must be a Thumb SWI
    u16int * halfendAddress = (u16int*)endAddress;
    //fetch the SWI
    hypercall = *halfendAddress;
    //adjust the hypercall
    hypercall = ( (hypercall & 0xFF00) | (index+1));

    //store it
    *halfendAddress = hypercall;
  }
  // Tricky. It can be a Thumb or an ARM SWI. Check!
  else
  {
    u16int * halfendAddress = (u16int*)endAddress;
    u16int halfinstruction = 0;
    halfinstruction = *halfendAddress;
    // SVC 0 is not ours. Do not touch it but die instead and let me think on how to fix it :/
    if( ( (halfinstruction & 0xDFFF) > INSTR_SWI_THUMB) && ( (halfinstruction & 0xDFFF) <= 0xDFFF ) )
    {
      //Ok so this is a thumb SWI
      hypercall = *halfendAddress;
      //adjust the hypercall
      hypercall = ( (hypercall & 0xFF00) | (index+1));
      * halfendAddress = hypercall;
    }
    else if ( (halfinstruction & 0xDFFF) == INSTR_SWI_THUMB)
    {
      DIE_NOW(0,"Damn. Seems like you hit a guest SVC. Fix me");
    }
    else // OK this is an ARM SWI
    {
      hypercall = *endAddress;
      hypercall = (hypercall & 0xFF000000) | ((index + 1) << 8);
      *endAddress = hypercall;
    }
  }
  DEBUG(BLOCK_CACHE, "resolveCacheConflict: found another BB to end at same address; "
      "replace hypercall with %#x" EOL, hypercall);
}

#endif
