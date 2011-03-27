#include "guestManager/blockCache.h"

#include "vm/omap35xx/serial.h"


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
#ifdef BLOCK_CACHE_DEBUG
  serial_putstring("Initialising basic block cache @ address ");
  serial_putint((u32int)bcache);
  serial_newline();
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
#ifdef BLOCK_CACHE_DEBUG
  serial_putstring("blockCache: checkBlockCache index ");
  serial_putint(bcIndex);
  serial_newline();
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
#ifdef BLOCK_CACHE_DEBUG
  serial_putstring("blockCache: ADD[");
  serial_putint(index);
  serial_putstring("] start@");
  serial_putint(blkStartAddr);
  serial_putstring(" end@");
  serial_putint(blkEndAddr);
  serial_putstring(" hdlPtr ");
  serial_putint(hdlFunct);
  serial_putstring(" eobInstr ");
  serial_putint(hypInstruction);
  serial_newline();
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
#ifdef BLOCK_CACHE_DEBUG
  serial_putstring("blockCache: getBlockCacheEntry index ");
  serial_putint(index);
  serial_newline();
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
#ifdef BLOCK_CACHE_DEBUG
        serial_putstring("blockCache: found entry for address.");
        serial_newline();
        serial_putstring("blockCache: block start ");
        serial_putint(bcAddr[i].startAddress);
        serial_putstring(" block end ");
        serial_putint(bcAddr[i].endAddress);
        serial_putstring(" index ");
        serial_putint(i);
        serial_newline();
#endif
        return i;
      }
    }
  }
  return -1;
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
  int i = 0;
#ifdef BLOCK_CACHE_DEBUG
  serial_putstring("blockCache: COLLISION!!!");
  serial_newline();
#endif
#ifdef DUMP_COLLISION_COUNTER
  collisionCounter++;
  if ((collisionCounter % 10) == 9)
  {
    serial_putint_nozeros(collisionCounter);
    serial_putstring(" ");
    if ((collisionCounter % 200) == 199)
    {
      serial_newline();
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
#ifdef BLOCK_CACHE_DEBUG
      serial_putstring("blockCache: found another BB to end at same address.");
      serial_newline();
      serial_putstring("blockCache: replace hypercall with ");
      serial_putint(hypercallSWI);
      serial_newline();
#endif
      *((u32int*)(bcAddr[index].endAddress)) = hypercallSWI;
      return;
    }
  }
#ifdef BLOCK_CACHE_DEBUG
  serial_putstring("blockCache: no other BB ends at same address.");
  serial_newline();
  // restore hypered instruction back!
  serial_putstring("blockCache: restoring hypercall ");
  serial_putint(*((u32int*)(bcAddr[index].endAddress)));
  serial_putstring(" back to ");
  serial_putint(bcAddr[index].hyperedInstruction);
  serial_newline();
#endif
  *((u32int*)(bcAddr[index].endAddress)) = bcAddr[index].hyperedInstruction;
}


void explodeCache(BCENTRY * bcache)
{
  serial_putstring("========EXPLODE!!!=========");
  serial_newline();

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
    while( (cacheIndex = findEntryForAddress(bcache, address)) != -1 )
    {
      removeCacheEntry(bcache, cacheIndex);
    }
  }
}

// finds and clears block cache entries within the given address range
void validateCacheMultiPreChange(BCENTRY * bcache, u32int startAddress, u32int endAddress)
{
#ifdef BLOCK_CACHE_DEBUG
  serial_putstring("blockCache: validateCacheMultiPreChange start ");
  serial_putint(startAddress);
  serial_putstring(" end ");
  serial_putint(endAddress);
  serial_newline();
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
  serial_putstring("BlockCache: entry #0x ");
  serial_putint_nozeros(index);
  serial_putstring(":");
  serial_newline();

  serial_putstring("BlockCache: startAddress = ");
  serial_putint(bcache[index].startAddress);
  serial_putstring(" endAddress = ");
  serial_putint(bcache[index].endAddress);
  serial_newline();

  serial_putstring("BlockCache: valid = ");
  serial_putint_nozeros(bcache[index].valid);
  serial_putstring(" EOBinstr = ");
  serial_putint(bcache[index].hyperedInstruction);
  serial_putstring(" hdlFunct = ");
  serial_putint(bcache[index].hdlFunct);
  serial_newline();
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

