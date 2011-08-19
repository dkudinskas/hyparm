#include "common/debug.h"

#include "guestManager/blockCache.h"


#ifdef DUMP_COLLISION_COUNTER
static u32int collisionCounter = 0;
#endif

#define NUMBER_OF_BITMAPS       16
#define MEMORY_PER_BITMAP       0x10000000
#define MEMORY_PER_BITMAP_BIT  (MEMORY_PER_BITMAP / 32) // should be 8 megabytes

static u32int execBitMap[NUMBER_OF_BITMAPS];

void initialiseBlockCache(BCENTRY * bcache)
{
  int i = 0;
#ifdef DUMP_COLLISION_COUNTER
  collisionCounter = 0;
#endif
#ifdef BLOCK_CACHE_DBG
  printf("Initialising basic block cache @ 0x%x\n", (u32int)bcache);
#endif

  for (i = 0; i < BLOCK_CACHE_SIZE; i++)
  {
    bcache[i].startAddress = 0;
    bcache[i].endAddress = 0;
    bcache[i].hyperedInstruction = 0;
    bcache[i].valid = FALSE;
    bcache[i].hdlFunct = 0;
  }
  
  for (i = 0; i < NUMBER_OF_BITMAPS; i++)
  {
    execBitMap[i] = 0;
  }
}

bool checkBlockCache(u32int blkStartAddr, u32int bcIndex, BCENTRY * bcAddr)
{
#ifdef BLOCK_CACHE_DBG
  printf("blockCache: checkBlockCache index %x\n", bcIndex);
#endif

  if (bcAddr[bcIndex].valid == FALSE)
  {
    // cache entry invalid
    return FALSE;
  }
  else
  {
    // cache entry valid, but is this a collision?
    if (bcAddr[bcIndex].startAddress != blkStartAddr)
    {
      return FALSE;
    }
    else
    {
      return TRUE;
    }
  }
}

void addToBlockCache(u32int blkStartAddr, u32int hypInstruction, u32int blkEndAddr,
                     u32int index, u32int hdlFunct, BCENTRY * bcAddr)
{
#ifdef BLOCK_CACHE_DBG
  printf("blockCache: ADD[%02x] start @ %x end @ %x hdlPtr %x eobInstr %08x\n",
         index, blkStartAddr, blkEndAddr, hdlFunct, hypInstruction);
#endif
  if ((bcAddr[index].valid == TRUE) && (bcAddr[index].endAddress != blkEndAddr) )
  {
    // somebody has been sleeping in our cache location!
    resolveCacheConflict(index, bcAddr);
    // now that we resolved the conflict, we can store the new entry data...
    bcAddr[index].startAddress = blkStartAddr;
    bcAddr[index].endAddress = blkEndAddr;
    bcAddr[index].hyperedInstruction = hypInstruction;
    bcAddr[index].hdlFunct = hdlFunct;
    bcAddr[index].valid = TRUE;
  }
  else if ((bcAddr[index].valid == TRUE) && (bcAddr[index].endAddress == blkEndAddr) )
  {
    /* NOTE: if entry valid, but blkEndAddress is the same as new block to add      *
     * then the block starts at another address but ends on the same instruction    *
     * and by chance - has the same index. just modify existing entry, don't remove */
    bcAddr[index].startAddress = blkStartAddr;
  }
  else
  {
    bcAddr[index].startAddress = blkStartAddr;
    bcAddr[index].endAddress = blkEndAddr;
    bcAddr[index].hyperedInstruction = hypInstruction;
    bcAddr[index].hdlFunct = hdlFunct;
    bcAddr[index].valid = TRUE;
  }
  
  // set bitmap entry to executed
  setExecBitMap(blkEndAddr);
}

BCENTRY * getBlockCacheEntry(u32int index, BCENTRY * bcAddr)
{
#ifdef BLOCK_CACHE_DBG
  printf("getBlockCacheEntry index %x\n", index);
#endif
  return &bcAddr[index];
}


/* input: any address, might be start, end of block or somewhere in the middle... */
/* output: first cache entry index for the block where this address falls into */
/* output: if no such block, return -1 (0xFFFFFFFF) */
u32int findEntryForAddress(BCENTRY * bcAddr, u32int addr)
{
  int i = 0;
  for (i = 0; i < BLOCK_CACHE_SIZE; i++)
  {
    if (bcAddr[i].valid == TRUE)
    {
      // entry valid
      if ( (bcAddr[i].startAddress <= addr) && (bcAddr[i].endAddress >= addr) )
      {
        // addr falls in-between start-end inclusive. found a matching entry.
#ifdef BLOCK_CACHE_DBG
        printf("findEntryForAddress: found bCache entry for address %x\n", addr);
        printf("findEntryForAddress: block start %x end %x index %x",
               bcAddr[i].startAddress, bcAddr[i].endAddress, i);
#endif
        return i;
      }
    }
  }
  return (u32int)-1;
}

/* remove a specific cache entry */
void removeCacheEntry(BCENTRY * bcAddr, u32int cacheIndex)
{
  // restore replaced end of block instruction
  *((u32int*)(bcAddr[cacheIndex].endAddress)) = bcAddr[cacheIndex].hyperedInstruction;
  bcAddr[cacheIndex].valid = FALSE;
  bcAddr[cacheIndex].startAddress = 0;
  bcAddr[cacheIndex].endAddress = 0;
  bcAddr[cacheIndex].hdlFunct = 0;
  bcAddr[cacheIndex].hyperedInstruction = 0;
  return;
}

void resolveCacheConflict(u32int index, BCENTRY * bcAddr)
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
  u32int i = 0;
#ifdef BLOCK_CACHE_DBG
  printf("resolveCacheConflict: collision at index %x\n", index);
#endif
#ifdef DUMP_COLLISION_COUNTER
  collisionCounter++;
  if ((collisionCounter % 10) == 9)
  {
    DEBUG_INT_NOZEROS(collisionCounter);
    DEBUG_STRING(" ");
    if ((collisionCounter % 200) == 199)
    {
      DEBUG_NEWLINE();
    }
  }
#endif

  for (i = 0; i < BLOCK_CACHE_SIZE; i++)
  {
    if ((bcAddr[i].valid == TRUE) &&
        (bcAddr[i].endAddress == bcAddr[index].endAddress) &&
        (i != index) )
    {
      // found a valid entry in the cache, that BB ends @ the same address as
      // the block of the entry that we collided with.
      u32int hypercallSWI = *((u32int*)(bcAddr[index].endAddress));
      // SWI 0xEF<code> is replaced with SWI<newcode> where newcode points new entry
      hypercallSWI = (hypercallSWI & 0xFF000000) | ((i + 1) << 8);
#ifdef BLOCK_CACHE_DBG
      printf("resolveCacheConflict: found another BB to end at same address.\n");
      printf("resolveCacheConflict: replace hypercall with %x\n", hypercallSWI);
#endif
      *((u32int*)(bcAddr[index].endAddress)) = hypercallSWI;
      return;
    }
  }
#ifdef BLOCK_CACHE_DBG
  printf("resolveCacheConflict: no other BB ends at same address.\n");
  // restore hypered instruction back!
  printf("resolveCacheConflict: restoring hypercall %x back to %08x\n",
        *((u32int*)(bcAddr[index].endAddress)), bcAddr[index].hyperedInstruction);
#endif
  *((u32int*)(bcAddr[index].endAddress)) = bcAddr[index].hyperedInstruction;
}


void explodeCache(BCENTRY * bcache)
{
#ifdef BLOCK_CACHE_DBG
  printf("========BLOCK CACHE EXPLODE!!!=========\n");
#endif

  int i = 0;

  for (i = 0; i < BLOCK_CACHE_SIZE; i++)
  {
    if (bcache[i].valid == TRUE)
    {
      removeCacheEntry(bcache, i);
    }
  }
  for (i = 0; i < NUMBER_OF_BITMAPS; i++)
  {
    execBitMap[i] = 0;
  }
}

// finds block cache entries that include a given address, clears them
void validateCachePreChange(BCENTRY * bcache, u32int address)
{
  if (isBitmapSetForAddress(address))
  {
    u32int cacheIndex = 0;
    while( (cacheIndex = findEntryForAddress(bcache, address)) != (u32int)-1 )
    {
      removeCacheEntry(bcache, cacheIndex);
    }
  }
}

// finds and clears block cache entries within the given address range
void validateCacheMultiPreChange(BCENTRY * bcache, u32int startAddress, u32int endAddress)
{
#ifdef BLOCK_CACHE_DBG
  printf("validateCacheMultiPreChange start %x end %x\n", startAddress, endAddress);
#endif
  u32int i;
  for (i = 0; i < BLOCK_CACHE_SIZE; i++)
  {
    if (bcache[i].valid == TRUE)
    {
      //We only care if the end address of the block falls inside the address validation range
      if ( (bcache[i].endAddress >= startAddress) && (bcache[i].endAddress <= endAddress) )
      {
        // cached entry falls within given address range
        removeCacheEntry(bcache, i);
      }
    }
  }
}


void dumpBlockCacheEntry(u32int index, BCENTRY * bcache)
{
  printf("dumpBlockCacheEntry: entry #%02x: ", index);
  printf("dumpBlockCacheEntry: startAddress = %x, endAddress = %x, valid = %x\n",
         bcache[index].startAddress, bcache[index].endAddress, bcache[index].valid);
  printf("dumpBlockCacheEntry: EOBinstr = %08x, handlerFunction = %x",
         bcache[index].hyperedInstruction, bcache[index].hdlFunct);
}

void setExecBitMap(u32int addr)
{
  u32int index = addr / MEMORY_PER_BITMAP; 
  u32int bitNumber = (addr & 0x0FFFFFFF) / MEMORY_PER_BITMAP_BIT;

  execBitMap[index] = execBitMap[index] | (1 << bitNumber);
}

void clearExecBitMap(u32int addr)
{
  u32int index = addr / MEMORY_PER_BITMAP; 
  u32int bitNumber = (addr & 0x0FFFFFFF) / MEMORY_PER_BITMAP_BIT;

  execBitMap[index] = execBitMap[index] & ~(1 << bitNumber);
}

bool isBitmapSetForAddress(u32int addr)
{
  u32int index = addr / MEMORY_PER_BITMAP; 
  u32int bitNumber = (addr & 0x0FFFFFFF) / MEMORY_PER_BITMAP_BIT;
  u32int bitResult = execBitMap[index] & (1 << bitNumber); 
  return (bitResult != 0);
}

