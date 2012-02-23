#include "common/debug.h"
#include "common/stddef.h"
#include "common/string.h"

#include "guestManager/blockCache.h"
#include "guestManager/guestContext.h"

#include "instructionEmu/scanner.h"


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


#ifdef CONFIG_BLOCK_COPY

void addToBlockCache(BCENTRY *blockCache, u32int index, u32int startAddress, u32int endAddress,
    u32int hypInstruction, void *hdlFunct, u32int blockCopyCacheSize, u32int blockCopyCacheAddress)
{
  DEBUG(BLOCK_CACHE, "addToBlockCache: index = %#.2x @ %#.8x--%#.8x, handler = %p, eobInstr = "
      "%#.8x, blockCopyCacheSize = %#.8x, blockCopyCacheAt = %#.8x" EOL, index, startAddress,
      endAddress, hdlFunct, hypInstruction, blockCopyCacheSize, blockCopyCacheAddress);

  if (blockCache[index].type != BCENTRY_TYPE_INVALID)
  {
    // somebody has been sleeping in our cache location!
    resolveCacheConflict(blockCache, index);
  }
  //store the new entry data...
  //Last bit of blkStartAddr is used to indicate that a reserved word is present in blockCopyCache (see scanner.c)  
  blockCache[index].endAddress = endAddress;
  blockCache[index].hyperedInstruction = hypInstruction;
  blockCache[index].hdlFunct = hdlFunct;
  blockCache[index].type = BCENTRY_TYPE_ARM;
  blockCache[index].blockCopyCacheAddress = blockCopyCacheAddress & ~1;
  blockCache[index].blockCopyCacheSize = blockCopyCacheSize;
  blockCache[index].reservedWord = blockCopyCacheAddress & 1;

#else

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

#endif /* CONFIG_BLOCK_COPY */

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
  /*
   * Comment by Peter on block copy:
   * Now it is also possible to set last used line of blockCopyCache back to the word just before
   * the blockCopyCache (this way the blockCopyCache will start again from the beginning -> Not
   * absolutely necessary however.
   */
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
#ifdef CONFIG_BLOCK_COPY
  printf(
      "dumpBlockCacheEntry: blockCopyCacheAddress = %#.8x, blockCopyCacheSize = %#.8x" EOL,
      blockCache[index].blockCopyCacheAddress, blockCache[index].blockCopyCacheSize
      );
#endif
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
    removeCacheEntry(blockCache, i);
  }
}

static void removeCacheEntry(BCENTRY *blockCache, u32int index)
{
#ifdef CONFIG_BLOCK_COPY
  /*
   * Invalidate a single cache entry
   */
  u32int address = blockCache[index].blockCopyCacheAddress;
  u32int size = blockCache[index].blockCopyCacheSize;
  DEBUG(BLOCK_CACHE, "removeCacheEntry: entry @ %p, block copy cache entry @ %#.8x size %#.8x" EOL,
    blockCache + index, address, size);
  removeBlockCopyCacheEntry(getGuestContext(), address, size);
  blockCache[index].blockCopyCacheSize = 0;
  blockCache[index].blockCopyCacheAddress = 0;
#else
  /*
   * Restore and invalidate a single cache entry
   */
  restoreReplacedInstruction(blockCache, index);
#endif /* CONFIG_BLOCK_COPY */
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
    Steps to Carry out with block copy:
    1. Remove entry in blockCache (The copied instructions)
    2. Remove log book(the original blockCache) entry
    Since a different startAddress means a different block of Code.
    There will be exactly 1 block in the block cache corresponding with this block
   */
  DEBUG(BLOCK_CACHE, "resolveCacheConflict: collision at index %#x" EOL, index);

  incrementCollisionCounter();

#ifdef CONFIG_BLOCK_COPY
  removeBlockCopyCacheEntry(getGuestContext(), blockCache[index].blockCopyCacheAddress,blockCache[index].blockCopyCacheSize);
#endif

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
      if (txxIsThumb32(blockCache[index].hyperedInstruction))
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


#ifdef CONFIG_BLOCK_COPY
/*
 * This code will check if a block that is split (splitting occurs due to 1 part being at the end of the blockCopyCache and another at the
 * begin of the block.)  When a block is split up the code will also check if the second part wants to make use of a reserved word.  If that is
 * the case the block will result in erroneous behavior and should be merged. This code will perform the merge and will return a pointer indicating
 * the end of the merged block (the word just after the block, this is similar to blockCopyCurrCacheAddress). After the merge the merged block
 * will be completely at the start of the blockCopyCache.
 */
u32int* checkAndMergeBlock(u32int* startOfBlock2, u32int* endOfBlock2, BCENTRY * blockCache,u32int* startOfBlock1,u32int* endOfBlock1)
{
  u32int* wordStepper = endOfBlock2-1;
  u32int instruction=0;
  bool patchCode=0;
#ifdef BLOCK_COPY_CACHE_DEBUG
  serial_putstring("checkAndMergeBlock with endOfBlock2 = ");
  serial_putint((u32int)endOfBlock2);
  serial_putstring(" & startOfBlock1 = ");
  serial_putint((u32int)startOfBlock1);
  serial_newline();
#endif
  while(wordStepper > (startOfBlock2 - 1)  )
  {
    /*
     * If there is a ldr instruction that reads from PC + offset than that will be a load that we installed that wants to read
     * the reserved word.  This is not possible since the reserved word will be somewhere near the end of the blockCopyCache
     * Therefore we need to patch the code.  If no such ldr instruction is present than there won't be a problem.
     * We are sure that we need to patch the code because the code is already translated and cannot read from PC if it was an original instruction
     * Instruction will be a load literal 0xe51f????
     */
    instruction = *wordStepper;
    if( (instruction & 0xe51f0000 )== 0xe51f0000 )
    {
      /* We have found a problem*/
      patchCode=1;
      break;
    }
    wordStepper--;
  }
  if(patchCode==1)
  {
    u32int nrInstructions2Move = 0;  /* These are the instructions that are currently at end of blockCopyCache */
    u32int nrInstructions2Shift = 0; /* These are the instructions that are alread at start of blockCopyCache */
    u32int* pointerDest=endOfBlock2;
    u32int* blockCopyLast;
    u32int offset=0;
    u32int* pointerSrc=0;
    u32int k=0;
   /*
    * 1)First we have to make sure that there is enough place freed up to place the new block
    * 2)Then we can copy all instructions (from last instruction to first instruction (not the other way around because than we overwrite instructions
    * 3)Clear the instructions that were placed at the end of the blockCopyCache!!
    * 4)Make sure that block is safed correctly in blockCopyCache
    */
    /*
     * Step 1 make room for new block.  The instructions that are at the end of the blockCopyCache will be placed at the start so we need
     * room for the number of instructions that are at the end
     */
    nrInstructions2Move = ((u32int)endOfBlock1 - ( (u32int)startOfBlock1 & 0xFFFFFFFE) ) >> 2;
    nrInstructions2Shift = ((u32int)endOfBlock2 - (u32int)startOfBlock2) >> 2;
#ifdef BLOCK_COPY_CACHE_DEBUG
    serial_putstring("nrInstructions2Move = ");
    serial_putint(nrInstructions2Move);
    serial_newline();
    serial_putstring("nrInstructions2Shift = ");
    serial_putint(nrInstructions2Shift);
    serial_newline();
#endif
    k=nrInstructions2Move;
    /**
     * Make place for the instructions that have to be moved
     */
    while(k>0)
    {
      /* We can use startOfBlock2 instead of gc->blockCopyCache & endOfBlock1 instead of gc->blockCopyCacheEnd*/
      pointerDest=checkAndClearBlockCopyCacheAddress(pointerDest,blockCache,startOfBlock2,endOfBlock1);
      pointerDest++;
      k--;
    }
    blockCopyLast=pointerDest;
#ifdef BLOCK_COPY_CACHE_DEBUG
    serial_putstring("Blockcache cleared till the end = ");
    serial_putint((u32int)blockCopyLast);
    serial_newline();
#endif

    /*
     * Copy The instructions first the one that are at the start of the blockCopyCache (there the str and ldr instructions need to be rewritten)
     * Than the one at the end of the blockCopyCache (including the reserved Word & backpointer) -> no rewrites needed
     */
    k=nrInstructions2Shift;
    pointerSrc = endOfBlock2; /* We have to start with last instruction otherwise we might overwrite instruction!*/
    while(k>0)
    {
      instruction = *(--pointerSrc);
      if(((instruction & 0xe51f0000 )== 0xe51f0000) || ((instruction & 0xe50f0000 )== 0xe50f0000))
      {
        /* Offset of instruction needs to be changed + 2 because PC is 2 behind
         * gc->blockCopyCache is address of SVC
         * startOfBlock2 +1 is address of reserved Word */
        offset=((pointerDest-1) - (startOfBlock2 + 1) + 2) << 2;
        if(offset> 0xFFFF)
        {
          DIE_NOW(NULL, "Offset is to big -> instruction will get corrupted");
        }
        instruction = instruction & 0xFFFF0000;
        instruction = instruction + offset;
      }
      *(--pointerDest)=instruction;
      k--;
    }
#ifdef BLOCK_COPY_CACHE_DEBUG
    serial_putstring("Instructions block2 shifted");
#endif
    /* Copy other block of instructions*/
    k=nrInstructions2Move;
    pointerSrc=endOfBlock1;
    while(k>0)
    {
      *(--pointerDest) = *(--pointerSrc); /* Offsets are still correct, as reserved Word takes the same translation*/
      /* And Clear the memory*/
      *(pointerSrc)=0x0;
      k--;
    }
#ifdef BLOCK_COPY_CACHE_DEBUG
    serial_putstring("Instructions block1 Copied");
#endif
    /*
     * Make sure that block is safed correctly in blockCopyCache
     */
    return blockCopyLast;
    /* patching needs to be done: set blockCopyCacheSize -> must be done in scanBlock (caller of this function) */
  }
  else
  {
    return endOfBlock2;
    /* No patching needs to be done just set blockCopyCacheSize -> must be done in scanBlock (caller of this function) */
  }
}

void removeBlockCopyCacheEntry(void *contextPtr, u32int blockCopyCacheAddress, u32int blockCopyCacheSize)
{
  GCONTXT *context = (GCONTXT *)contextPtr;
  //First we must check if we have to remove a coninuous block that it is split up (i.e.: if it was to close to the end)
  u32int endOfBlock = (blockCopyCacheAddress+(blockCopyCacheSize<<2));//End of the block that has to be removed
  u32int lastUsableBlockCopyCacheAddress = context->blockCopyCacheEnd-4; //see comment next rule
  //Warning last adress of blockCopyCache is a backpointer and might not be erased therefore
  if (endOfBlock > lastUsableBlockCopyCacheAddress)
  {
    u32int difference = endOfBlock-lastUsableBlockCopyCacheAddress-4;
#ifdef  BLOCK_COPY_CACHE_DEBUG
    serial_putstring("REMOVEBLOCKCOPYCACHEENTRY: ENDOFBLOCK:");
    serial_newline();
    serial_putstring("difference = ");
    serial_putint(difference);
    serial_newline();
#endif
    memset((u32int *)blockCopyCacheAddress,0,(blockCopyCacheSize<<2)-(difference));
    memset((u32int *)context->blockCopyCache,0,difference); //The rest of the block is at the start of BlockCopyCache remove it there
  }
  else //safe to remove all at once
  {
    memset((u32int *)blockCopyCacheAddress,0,blockCopyCacheSize<<2);//blockCopyCacheSize is number of u32int entries
  }
}

/* This function will check if the content at Addr == 0x0 if not make it free -> the address that must be used is returned*/
u32int * checkAndClearBlockCopyCacheAddress(u32int *Addr,BCENTRY *bcStartAddr,u32int* blockCopyCache,u32int* blockCopyCacheEnd){
  if(Addr >= blockCopyCacheEnd){//Last address of BlockCopyCacheAddr
    //Continue at beginning of blockCopyCacheAddress
    Addr=blockCopyCache;
# ifdef BLOCK_COPY_CACHE_DEBUG
  serial_putstring("blockCopyCache: End exceeded continue at start=0x");
  serial_putint((u32int)Addr);
  serial_newline();
# endif
  }
  if(*Addr == 0x0){
    //Do nothing -> address is usable
    return Addr;
  }else{
# ifdef BLOCK_COPY_CACHE_DEBUG
  serial_putstring("blockCopyCache: The contents of 0x");
  serial_putint((u32int)Addr);
  serial_putstring(" differs from 0x0 -> clean ");
  serial_newline();
# endif
    //The address will be the first occurrence of a non 0x0 entry => the contents will be a backpointer to a log entry
    BCENTRY * bcEntry = (BCENTRY*)(*Addr);
    u32int index = bcEntry-bcStartAddr;
# ifdef BLOCK_COPY_CACHE_DEBUG
  serial_putstring("block with blockcache id=");
  serial_putint(index);
  serial_putstring(" gets cleaned");
  serial_newline();
# endif
    //Remove cacheEntry In both blockCache & blockCopyCache
    removeCacheEntry(bcStartAddr, index);
  }
  return Addr;//Return Address that should be used to place next instruction
}

u32int *updateCurrBlockCopyCacheAddr(u32int *oldAddr, u32int nrOfAddedInstr, u32int *blockCopyCacheEnd)
{
  oldAddr += nrOfAddedInstr;
  if (oldAddr >= blockCopyCacheEnd)
  {
    //oldAddr=currBlockCopyCacheAddress blockCopyCacheAddresses will be used in a  cyclic manner
    //-> if end of blockCopyCache is passed blockCopyCacheCurrAddress must be updated
    oldAddr -= (BLOCK_COPY_CACHE_SIZE - 1);
  }
  return oldAddr;
}

#endif
