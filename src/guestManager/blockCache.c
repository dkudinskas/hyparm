#include "common/debug.h"

#include "guestManager/blockCache.h"


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
#define MEMORY_PER_BITMAP_BIT  (MEMORY_PER_BITMAP / 32) // should be 8 megabytes


static u32int execBitMap[NUMBER_OF_BITMAPS];


void initialiseBlockCache(BCENTRY *bcache)
{
  int i = 0;

  resetCollisionCounter();

  DEBUG(BLOCK_CACHE, "initialiseBlockCache: @ %p" EOL, bcache);

  for (i = 0; i < BLOCK_CACHE_SIZE; i++)
  {
    bcache[i].startAddress = 0;
    bcache[i].endAddress = 0;
    bcache[i].hyperedInstruction = 0;
#ifdef CONFIG_THUMB2
    bcache[i].halfhyperedInstruction = 0;
#endif
    bcache[i].valid = FALSE;
    bcache[i].hdlFunct = 0;
  }

  for (i = 0; i < NUMBER_OF_BITMAPS; i++)
  {
    execBitMap[i] = 0;
  }
}

bool checkBlockCache(u32int blkStartAddr, u32int bcIndex, BCENTRY *bcAddr)
{
  DEBUG(BLOCK_CACHE, "checkBlockCache: index = %#x" EOL, bcIndex);
  return bcAddr[bcIndex].valid && bcAddr[bcIndex].startAddress == blkStartAddr;
}

#ifdef CONFIG_THUMB2
void addToBlockCache(void *start, u32int hypInstruction, u16int halfhypInstruction, u32int blkEndAddr,
#else
void addToBlockCache(void *start, u32int hypInstruction, u32int blkEndAddr,
#endif
                     u32int index, void *hdlFunct, BCENTRY * bcAddr)
{
  u32int blkStartAddr = (u32int)start;
  DEBUG(BLOCK_CACHE, "addToBlockCache: index = %#x,@ %#.8x--%#.8x, handler = %p, eobInstr = "
      "%#.8x" EOL, index, blkStartAddr, blkEndAddr, hdlFunct, hypInstruction);

  if (bcAddr[index].valid)
  {
    if (bcAddr[index].endAddress != blkEndAddr)
    {
      // somebody has been sleeping in our cache location!
      resolveCacheConflict(index, bcAddr);
      // now that we resolved the conflict, we can store the new entry data...
      bcAddr[index].startAddress = blkStartAddr;
      bcAddr[index].endAddress = blkEndAddr;
#ifdef CONFIG_THUMB2
      bcAddr[index].halfhyperedInstruction = halfhypInstruction;
#endif
      bcAddr[index].hyperedInstruction = hypInstruction;
      bcAddr[index].hdlFunct = hdlFunct;
      bcAddr[index].valid = TRUE;
    }
    else
    {
      /* NOTE: if entry valid, but blkEndAddress is the same as new block to add      *
       * then the block starts at another address but ends on the same instruction    *
       * and by chance - has the same index. just modify existing entry, don't remove */
      bcAddr[index].startAddress = blkStartAddr;
    }
  }
  else
  {
    bcAddr[index].startAddress = blkStartAddr;
    bcAddr[index].endAddress = blkEndAddr;
    bcAddr[index].hyperedInstruction = hypInstruction;
#ifdef CONFIG_THUMB2
    bcAddr[index].halfhyperedInstruction = halfhypInstruction;
#endif
    bcAddr[index].hdlFunct = hdlFunct;
    bcAddr[index].valid = TRUE;
  }

  // set bitmap entry to executed
  setExecBitMap(blkEndAddr);
}

BCENTRY *getBlockCacheEntry(u32int index, BCENTRY *bcAddr)
{
  DEBUG(BLOCK_CACHE, "getBlockCacheEntry: index = %#x" EOL, index);
  return &bcAddr[index];
}


/* input: any address, might be start, end of block or somewhere in the middle... */
/* output: first cache entry index for the block where this address falls into */
/* output: if no such block, return -1 (0xFFFFFFFF) */
u32int findEntryForAddress(BCENTRY *bcAddr, u32int addr)
{
  u32int i = 0;
  for (i = 0; i < BLOCK_CACHE_SIZE; i++)
  {
    if (bcAddr[i].valid)
    {
      if (bcAddr[i].startAddress <= addr && bcAddr[i].endAddress >= addr)
      {
        // addr falls in-between start-end inclusive. found a matching entry.
        DEBUG(BLOCK_CACHE, "findEntryForAddress: found bCache entry for address %#.8x "
            "@ %#.8x--%#.8x, index = %#x" EOL,
            addr, bcAddr[i].startAddress, bcAddr[i].endAddress, i);
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
#ifdef CONFIG_THUMB2
  bcAddr[cacheIndex].halfhyperedInstruction = 0;
#endif
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
#ifdef CONFIG_THUMB2
  struct thumbEntry tb;
#endif
  DEBUG(BLOCK_CACHE, "resolveCacheConflict: collision at index %#x" EOL, index);

  incrementCollisionCounter();

  for (i = 0; i < BLOCK_CACHE_SIZE; i++)
  {
    if ((bcAddr[i].valid == TRUE) &&
        (bcAddr[i].endAddress == bcAddr[index].endAddress) &&
        (i != index) )
    {
#ifdef CONFIG_THUMB2
      /* found a valid entry in the cache, that BB ends @ the same address as
      * the block of the entry that we collided with.
      * Call the resolve function to ensure that conflicts between Thumb and ARM SWIs
      * will be resolved as approprate otherwise say 'hello' to segfault ^_^
      * ARM: SWI 0xEF<code> is replaced with SWI<newcode> where newcode points new entry
      * Thumb: SWI 0xDF<code> is replaced with SWI<newcode> where newcode points new entry
      */
      resolveSWI(i, (u32int*)bcAddr[index].endAddress);
#else
      // found a valid entry in the cache, that BB ends @ the same address as
      // the block of the entry that we collided with.
      u32int hypercallSWI = *((u32int*)(bcAddr[index].endAddress));
      // SWI 0xEF<code> is replaced with SWI<newcode> where newcode points new entry
      hypercallSWI = (hypercallSWI & 0xFF000000) | ((i + 1) << 8);
      DEBUG(BLOCK_CACHE, "resolveCacheConflict: found another BB to end at same address" EOL);
      DEBUG(BLOCK_CACHE, "resolveCacheConflict: replace hypercall with %#x" EOL, hypercallSWI);
      *((u32int*)(bcAddr[index].endAddress)) = hypercallSWI;
#endif
      return;
    }
  }

  DEBUG(BLOCK_CACHE, "resolveCacheConflict: no other BB ends at same address" EOL);
  // restore hypered instruction back!
#ifndef CONFIG_THUMB2
  /*
   * FIXME Markos: this (=DEBUG) is broken for thumb. I have to fix it
   */
  DEBUG(BLOCK_CACHE, "resolveCacheConflict: restoring hypercall %#x back to %#.8x" EOL,
      *((u32int *)(bcAddr[index].endAddress)), bcAddr[index].hyperedInstruction);
  *((u32int*)(bcAddr[index].endAddress)) = bcAddr[index].hyperedInstruction;
#else
  tb = BreakDownThumb(bcAddr,index);
  if (tb.isthumb==0)
  {
    DEBUG(BLOCK_CACHE, "Restoring %#.8x @ %#.8x" EOL, bcAddr[index].hyperedInstruction, bcAddr[index].endAddress);
    *((u32int *)(bcAddr[index].endAddress)) = bcAddr[index].hyperedInstruction;
  }
  else
  {
    //Assuming endAddress points to the end address of the block then...
    if(tb.second==0)// this is a thumb 16
    {
      DEBUG(BLOCK_CACHE, "Restoring %#.8x @ %#.8x" EOL,tb.first,(bcAddr[index].endAddress));
      *((u16int *)(bcAddr[index].endAddress)) = tb.first;
    }
    else
    {
      u16int *bpointer = 0;
      DEBUG(BLOCK_CACHE, "Restoring %#.8x @ %#.8x",tb.second,(bcAddr[index].endAddress));
      bpointer = (u16int *)(bcAddr[index].endAddress);
      *bpointer = tb.second;
      bpointer--;
      DEBUG(BLOCK_CACHE, " and %#.8x @ %p" EOL, tb.first, bpointer);
      *bpointer = tb.first;
    }
  }
#endif
}


void explodeCache(BCENTRY *bcache)
{
  DEBUG(BLOCK_CACHE, "========BLOCK CACHE EXPLODE!!!=========\n");

  int i;
  for (i = 0; i < BLOCK_CACHE_SIZE; i++)
  {
    if (bcache[i].valid)
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
void validateCachePreChange(BCENTRY *bcache, u32int address)
{
  if (isBitmapSetForAddress(address))
  {
    u32int cacheIndex = 0;
    while((cacheIndex = findEntryForAddress(bcache, address)) != (u32int)-1)
    {
      removeCacheEntry(bcache, cacheIndex);
    }
  }
}

// finds and clears block cache entries within the given address range
void validateCacheMultiPreChange(BCENTRY *bcache, u32int startAddress, u32int endAddress)
{
  DEBUG(BLOCK_CACHE, "validateCacheMultiPreChange: %#.8x--%#.8x" EOL, startAddress, endAddress);
  u32int i;
  for (i = 0; i < BLOCK_CACHE_SIZE; i++)
  {
    if (bcache[i].valid && bcache[i].endAddress >= startAddress && bcache[i].endAddress <= endAddress)
    {
      //We only care if the end address of the block falls inside the address validation range
      removeCacheEntry(bcache, i);
    }
  }
}


void dumpBlockCacheEntry(u32int index, BCENTRY *bcache)
{
  printf("dumpBlockCacheEntry: entry #%#.2x: ", index);
  printf("dumpBlockCacheEntry: startAddress = %#.8x, endAddress = %#.8x, valid = %x" EOL,
         bcache[index].startAddress, bcache[index].endAddress, bcache[index].valid);
  printf("dumpBlockCacheEntry: EOBinstr = %#.8x, handlerFunction = %p",
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


#ifdef CONFIG_THUMB2

struct thumbEntry BreakDownThumb(BCENTRY *bcAddr, u32int index)
{
  struct thumbEntry tb;
  /*
   * FIXME: Won't this struct be initialized to zero by default as per ANSI C spec?
   */
  tb.first = 0;
  tb.second = 0;
  DEBUG(BLOCK_CACHE, "BreakDownThumb: i will restore %#.8x[%#.8x]" EOL, bcAddr[index].hyperedInstruction, index);
  switch(bcAddr[index].halfhyperedInstruction)
  {
    case 0: // this is an ARM entry
      tb.isthumb=FALSE;
      break;
    case THUMB16:
      tb.isthumb=TRUE;
      tb.first=bcAddr[index].hyperedInstruction;
      tb.second = 0;
      break;
    case THUMB32:
      tb.isthumb=TRUE;
      tb.first = ( (bcAddr[index].hyperedInstruction) & 0xFFFF0000) >> 16;
      tb.second =(bcAddr[index].hyperedInstruction) & 0x0000FFFF;
      break;
    default:
      DIE_NOW(0,"BreakDownThumb is b0rked. Fix me");
  }
  return tb;
}

void resolveSWI( u32int index, u32int * endAddress)
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

