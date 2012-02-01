#include "common/debug.h"

#ifdef CONFIG_BLOCK_COPY
#include "common/memFunctions.h"
#endif

#include "guestManager/blockCache.h"
#include "guestManager/guestContext.h"


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
#ifdef CONFIG_BLOCK_COPY
    bcache[i].blockCopyCacheSize = 0;
    bcache[i].blockCopyCacheAddress = 0;
#endif
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

#ifdef CONFIG_BLOCK_COPY  //function is too different
void addToBlockCache(u32int blkStartAddr, u32int blkEndAddr,
                     u32int index, u32int blockCopyCacheSize, u32int blockCopyCacheAddress,u32int hypInstruction,void *hdlFunct,BCENTRY * bcAddr)
{
#ifdef BLOCK_COPY_CACHE_DEBUG
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
  serial_putstring(" blockCopyCacheSize ");
  serial_putint(blockCopyCacheSize);
  serial_putstring(" blockCopyCache@");
  serial_putint((blockCopyCacheAddress & 0xFFFFFFFE));
  serial_newline();
#endif
  if (bcAddr[index].valid == TRUE)
  {
    // somebody has been sleeping in our cache location!
    resolveCacheConflict(index, bcAddr);
    // now that we resolved the conflict, we arrive at situation where bcAddr[index].valid==false
  }
  //store the new entry data...
  if( (blockCopyCacheAddress & 0b1) == 0b1)//Last bit of blkStartAddr is used to indicate that a reserved word is present in blockCopyCache (see scanner.c)
  {
    bcAddr[index].reservedWord = 1;//Set reservedWord to true
    bcAddr[index].blockCopyCacheAddress = blockCopyCacheAddress & 0xFFFFFFFE;//Set last bit back to zero!!
    bcAddr[index].blockCopyCacheSize = blockCopyCacheSize;/* Space is also made for blockCopyCache so we don't need to change it */
  }
  else
  {
    bcAddr[index].reservedWord = 0;//Set reservedWord to true
    bcAddr[index].blockCopyCacheAddress = blockCopyCacheAddress;
    bcAddr[index].blockCopyCacheSize = blockCopyCacheSize; /* Size doesn't change*/
  }
  bcAddr[index].startAddress = blkStartAddr;
  bcAddr[index].endAddress = blkEndAddr;
  bcAddr[index].hyperedInstruction = hypInstruction;
  bcAddr[index].hdlFunct = hdlFunct;
  bcAddr[index].valid = TRUE;
  
  // set bitmap entry to executed
  setExecBitMap(blkEndAddr);
}
#else  //original function

#ifdef CONFIG_THUMB2
void addToBlockCache(u32int blkStartAddr, u32int hypInstruction, u16int halfhypInstruction, u32int blkEndAddr,
#else
void addToBlockCache(u32int blkStartAddr, u32int hypInstruction, u32int blkEndAddr,
#endif
                     u32int index, void *hdlFunct, BCENTRY * bcAddr)
{
  DEBUG(BLOCK_CACHE, "addToBlockCache: index = %#x,@ %#.8x--%#.8x, handler = %#.8x, eobInstr = "
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
#endif /* CONFIG_BLOCK_COPY */

BCENTRY *getBlockCacheEntry(u32int index, BCENTRY *bcAddr)
{
  DEBUG(BLOCK_CACHE, "getBlockCacheEntry: index = %#x" EOL, index);
  return &bcAddr[index];
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
          DIE_NOW(0, "Offset is to big -> instruction will get corrupted");
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
#endif

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
  //The copied code has to be cleaned up
#ifdef CONFIG_BLOCK_COPY
#ifdef BLOCK_COPY_CACHE_DEBUG
  serial_putstring("removeCache entered.  Address of cache(logbook) entry:");
  serial_putint((u32int) (bcAddr+cacheIndex));
  serial_putstring(" gets cleaned");
  serial_newline();
#endif
  u32int blockCopyCacheAddressOfEntry = bcAddr[cacheIndex].blockCopyCacheAddress;
  u32int blockCopyCacheSizeOfEntry = bcAddr[cacheIndex].blockCopyCacheSize;
#ifdef BLOCK_COPY_CACHE_DEBUG
  serial_putstring("removeBlockCopyCache entry: startAddress:");
  serial_putint((u32int)blockCopyCacheAddressOfEntry);
  serial_putstring(" size:");
  serial_putint(blockCopyCacheSizeOfEntry);
  serial_newline();
#endif
  removeBlockCopyCacheEntry(getGuestContext(), blockCopyCacheAddressOfEntry, blockCopyCacheSizeOfEntry);
  bcAddr[cacheIndex].blockCopyCacheSize = 0;
  bcAddr[cacheIndex].blockCopyCacheAddress = 0;
#else
  // restore replaced end of block instruction
  *((u32int*)(bcAddr[cacheIndex].endAddress)) = bcAddr[cacheIndex].hyperedInstruction;
#endif /* CONFIG_BLOCK_COPY */
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
    Steps to Carry out with block copy:
    1. Remove entry in blockCache (The copied instructions)
    2. Remove log book(the original blockCache) entry
    Since a different startAddress means a different block of Code.
    There will be exactly 1 block in the block cache corresponding with this block
   */
  u32int i = 0;
#ifdef CONFIG_THUMB2
  struct thumbEntry tb;
#endif
  DEBUG(BLOCK_CACHE, "resolveCacheConflict: collision at index %#x" EOL, index);
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

#ifdef CONFIG_BLOCK_COPY
  removeBlockCopyCacheEntry(getGuestContext(), bcAddr[index].blockCopyCacheAddress,bcAddr[index].blockCopyCacheSize);
#endif
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

#ifdef CONFIG_BLOCK_COPY
void removeBlockCopyCacheEntry(void *contextPtr, u32int blockCopyCacheAddress,u32int blockCopyCacheSize){
  GCONTXT *context = (GCONTXT *)contextPtr;
  //First we must check if we have to remove a coninuous block that it is split up (i.e.: if it was to close to the end)
  u32int endOfBlock = (blockCopyCacheAddress+(blockCopyCacheSize<<2));//End of the block that has to be removed
  u32int lastUsableBlockCopyCacheAddress = context->blockCopyCacheEnd-4; //see comment next rule
  //Warning last adress of blockCopyCache is a backpointer and might not be erased therefore
  if( endOfBlock > lastUsableBlockCopyCacheAddress)
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
    /*serial_putstring("REMOVEBLOCKCOPYCACHEENTRY"); */
    memset((u32int *)blockCopyCacheAddress,0,blockCopyCacheSize<<2);//blockCopyCacheSize is number of u32int entries
  }
}
#endif /* CONFIG_BLOCK_COPY */

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
  //Now it is also possible to set last used line of blockCopyCache back to the word just before the blockCopyCache (this way the blockCopyCache
  //will start again from the beginning -> Not absolutely necessary however.
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
  printf("dumpBlockCacheEntry: entry #%02x: ", index);
  printf("dumpBlockCacheEntry: startAddress = %x, endAddress = %x, valid = %x\n",
         bcache[index].startAddress, bcache[index].endAddress, bcache[index].valid);
#ifdef CONFIG_BLOCK_COPY
  printf("BlockCopyCacheAddress = %08x, BlockCopyCache size = = %08x",
    bcache[index].blockCopyCacheAddress, bcache[index].blockCopyCacheSize);
#endif
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


#ifdef CONFIG_BLOCK_COPY
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

u32int* updateCurrBlockCopyCacheAddr(u32int* oldAddr, u32int nrOfAddedInstr,u32int* blockCopyCacheEnd){
  oldAddr=oldAddr+nrOfAddedInstr;
  if(oldAddr >= blockCopyCacheEnd ){//oldAddr=currBlockCopyCacheAddress blockCopyCacheAddresses will be used in a  cyclic manner
                                    //-> if end of blockCopyCache is passed blockCopyCacheCurrAddress must be updated
    oldAddr=oldAddr - (BLOCK_COPY_CACHE_SIZE-1);
  }
  return oldAddr;
}
#endif


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
