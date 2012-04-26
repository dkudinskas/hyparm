#include "common/bit.h"
#include "common/string.h"

#include "cpuArch/constants.h"

#include "guestManager/translationCache.h"

#include "memoryManager/mmu.h"


#ifdef CONFIG_BLOCK_CACHE_COLLISION_COUNTER

static inline u64int getCollisionCounter(const TranslationCache *tc);
static inline void incrementCollisionCounter(TranslationCache *tc);

static inline u64int getCollisionCounter(const TranslationCache *tc)
{
  return tc->collisionCounter;
}

static inline void incrementCollisionCounter(TranslationCache *tc)
{
  tc->collisionCounter++;
}

#else

#define getCollisionCounter(tc)        (0ULL)
#define incrementCollisionCounter(tc)

#endif /* CONFIG_BLOCK_CACHE_COLLISION_COUNTER */


static void clearExecBitMap(TranslationCache *tc, u32int address);
static void clearMetaCache(TranslationCache *tc);
static u32int findMetaCacheEntry(const TranslationCache *tc, u32int address);
static bool isBitmapSetForAddress(const TranslationCache *tc, u32int address);
static u32int *mergeCodeBlock(TranslationCache *tc, u32int *startOfBlock1, u32int *endOfBlock2);
static void removeCacheEntry(TranslationCache *tc, MetaCacheEntry *entry);
static void removeCodeCacheEntry(const TranslationCache *tc, u32int *code, u32int codeSize);
static void resolveCacheConflict(TranslationCache *tc, u32int metaIndex);
static void restoreReplacedInstruction(MetaCacheEntry *entry);
static void setExecBitMap(TranslationCache *tc, u32int address);


#ifdef CONFIG_BLOCK_COPY

void addMetaCacheEntry(TranslationCache *tc, u32int index, u32int *start, u32int *end,
    u32int hypInstruction, InstructionHandler hdlFunct, u32int codeSize, u32int *code)
{
  DEBUG(BLOCK_CACHE, "addToBlockCache: index = %#.2x @ %p--%p, handler = %p, eobInstr = "
      "%#.8x, codeSize = %#.8x, code @ %p" EOL, index, start,
      end, hdlFunct, hypInstruction, codeSize, code);

  if (tc->metaCache[index].type != MCE_TYPE_INVALID)
  {
    // somebody has been sleeping in our cache location!
    resolveCacheConflict(tc, index);
  }
  //store the new entry data...
  //Last bit of blkStartAddr is used to indicate that a reserved word is present in blockCopyCache (see scanner.c)
  tc->metaCache[index].startAddress = (u32int)start;
  tc->metaCache[index].endAddress = (u32int)end;
  tc->metaCache[index].hyperedInstruction = hypInstruction;
  tc->metaCache[index].hdlFunct = hdlFunct;
  tc->metaCache[index].type = MCE_TYPE_ARM;
  tc->metaCache[index].code = (u32int *)((u32int)code & ~1);
  tc->metaCache[index].codeSize = codeSize;
  tc->metaCache[index].reservedWord = (u32int)code & 1;

  // set bitmap entry to executed
  setExecBitMap(tc, (u32int)end);
}

#else

void addMetaCacheEntry(TranslationCache *tc, u32int index, u32int *start, u32int *end,
    u32int hypInstruction, u32int type, void *hdlFunct)
{
  DEBUG(BLOCK_CACHE, "addToBlockCache: index = %#.2x @ %p--%p, handler = %p, eobInstr = "
        "%#.8x" EOL, index, start, end, hdlFunct, hypInstruction);

  if (tc->metaCache[index].type != MCE_TYPE_INVALID)
  {
    if (tc->metaCache[index].endAddress != (u32int)end)
    {
      // somebody has been sleeping in our cache location!
      resolveCacheConflict(tc, index);
      // now that we resolved the conflict, we can store the new entry data...
      tc->metaCache[index].endAddress = (u32int)end;
      tc->metaCache[index].hyperedInstruction = hypInstruction;
      tc->metaCache[index].hdlFunct = hdlFunct;
      tc->metaCache[index].type = type;
    }
    /* NOTE: if entry valid, but blkEndAddress is the same as new block to add      *
     * then the block starts at another address but ends on the same instruction    *
     * and by chance - has the same index. just modify existing entry, don't remove */
  }
  else
  {
    tc->metaCache[index].endAddress = (u32int)end;
    tc->metaCache[index].hyperedInstruction = hypInstruction;
    tc->metaCache[index].hdlFunct = hdlFunct;
    tc->metaCache[index].type = type;
  }

  tc->metaCache[index].startAddress = (u32int)start;

  // set bitmap entry to executed
  setExecBitMap(tc, (u32int)end);
}
#endif /* CONFIG_BLOCK_COPY */


static void clearExecBitMap(TranslationCache *tc, u32int address)
{
  u32int index = address / TRANSLATION_CACHE_MEMORY_PER_BITMAP;
  tc->execBitMap[index] &= ~(1 << ((address & 0x0FFFFFFF) / TRANSLATION_CACHE_MEMORY_PER_BITMAP_BIT));
}

static void clearMetaCache(TranslationCache *tc)
{
  /*
   * Restore and invalidate all M$ entries
   */
  for (u32int i = 0; i < TRANSLATION_CACHE_META_SIZE_N; i++)
  {
    removeCacheEntry(tc, &tc->metaCache[i]);
  }
}

void clearTranslationCache(TranslationCache *tc)
{
  clearMetaCache(tc);
  memset(tc->execBitMap, 0, sizeof(u32int) * TRANSLATION_CACHE_NUMBER_OF_BITMAPS);
}

void clearTranslationCacheByAddress(TranslationCache *tc, u32int address)
{
  if (isBitmapSetForAddress(tc, address))
  {
    u32int cacheIndex;
    while ((cacheIndex = findMetaCacheEntry(tc, address)) != (u32int)-1)
    {
      removeCacheEntry(tc, &tc->metaCache[cacheIndex]);
    }
  }
}

void clearTranslationCacheByAddressRange(TranslationCache *tc, u32int startAddress, u32int endAddress)
{
  DEBUG(BLOCK_CACHE, "validateCacheMultiPreChange: %#.8x--%#.8x" EOL, startAddress, endAddress);
  for (u32int i = 0; i < TRANSLATION_CACHE_META_SIZE_N; i++)
  {
    if (tc->metaCache[i].type != MCE_TYPE_INVALID && tc->metaCache[i].endAddress >= startAddress
        && tc->metaCache[i].endAddress <= endAddress)
    {
      //We only care if the end address of the block falls inside the address validation range
      removeCacheEntry(tc, &tc->metaCache[i]);
    }
  }
}

void dumpMetaCacheEntry(MetaCacheEntry *entry)
{
  printf("dumpMetaCacheEntry @ %p: %#.8x--%#.8x type=%x instr=%#.8x handler=%p" EOL, entry,
         entry->startAddress, entry->endAddress, entry->type, entry->hyperedInstruction,
         entry->hdlFunct);
#ifdef CONFIG_BLOCK_COPY
  printf("dumpMetaCacheEntry @ p: code @ %p size=%#.8x" EOL, entry->code, entry->codeSize);
#endif
}

void dumpMetaCacheEntryByIndex(TranslationCache *tc, u32int metaIndex)
{
  MetaCacheEntry *entry = &tc->metaCache[metaIndex];
  printf("dumpMetaCacheEntryByIndex: M$ index = %#.3x @ %p" EOL, metaIndex, entry);
  dumpMetaCacheEntry(entry);
}

/* input: any address, might be start, end of block or somewhere in the middle... */
/* output: first cache entry index for the block where this address falls into */
/* output: if no such block, return -1 (0xFFFFFFFF) */
static u32int findMetaCacheEntry(const TranslationCache *tc, u32int address)
{
  for (u32int i = 0; i < TRANSLATION_CACHE_META_SIZE_N; i++)
  {
    if (tc->metaCache[i].type != MCE_TYPE_INVALID
        && tc->metaCache[i].startAddress <= address && tc->metaCache[i].endAddress >= address)
    {
      // addr falls in-between start-end inclusive. found a matching entry.
      DEBUG(BLOCK_CACHE, "findEntryForAddress: found entry for address %#.8x @ %#.8x--%#.8x, "
            "index = %#x" EOL, address, tc->metaCache[i].startAddress, tc->metaCache[i].endAddress, i);
      return i;
    }
  }
  return (u32int)-1;
}

void initialiseTranslationCache(TranslationCache *tc)
{
  /*
   * We could memset zero the entire M$ here but it's useless since we ALWAYS allocate with calloc.
   */
  DEBUG(BLOCK_CACHE, "initialiseTranslationCache: M$ @ %p" EOL, &tc->metaCache);
#ifdef CONFIG_BLOCK_COPY
  /*
   * Allocate the code cache from the executable pool through a dedicated allocator.
   */
  tc->codeCache = tc->codeCacheNextEntry = allocateCodeCache();
  if (tc->codeCache == NULL)
  {
    DIE_NOW(NULL, "failed to allocate C$");
  }
  DEBUG(BLOCK_CACHE, "initialiseTranslationCache: C$ @ %p" EOL, tc->codeCache);
  /*
   * The entire C$ (up to max. size) will be visible to the guest, so zero it out completely.
   */
  memset(tc->codeCache, 0, CODE_CACHE_MAX_SIZE);
  /*
   * Configure the pointer to the last entry to reflect the requested size.
   */
  tc->codeCacheLastEntry = (u32int *)((u32int)tc->codeCache + TRANSLATION_CACHE_CODE_SIZE_B) - 1;
  /*
   * Install unconditional branch to the beginning at the end of the block copy cache.
   * TODO: place this instruction as needed, so we can support Thumb too.
   */
  s32int branchOffset = -(s32int)(TRANSLATION_CACHE_CODE_SIZE_B + 1);
  *(tc->codeCacheLastEntry) = (CC_AL << 28) | (0b1010 << 24) | (*(u32int *)&branchOffset & 0xFFFFFF);
#endif
}

static bool isBitmapSetForAddress(const TranslationCache *tc, u32int address)
{
  u32int index = address / TRANSLATION_CACHE_MEMORY_PER_BITMAP;
  u32int bitNumber = (address & 0x0FFFFFFF) / TRANSLATION_CACHE_MEMORY_PER_BITMAP_BIT;
  return (tc->execBitMap[index] & (1 << bitNumber));
}

static void removeCacheEntry(TranslationCache *tc, MetaCacheEntry *entry)
{
#ifdef CONFIG_BLOCK_COPY
  /*
   * Invalidate a single cache entry
   */
  DEBUG(BLOCK_CACHE, "removeCacheEntry: entry @ %p, block copy cache entry @ %p size %#.8x" EOL,
    entry, entry->code, entry->codeSize);
  removeCodeCacheEntry(tc, entry->code, entry->codeSize);
#else
  /*
   * Restore and invalidate a single cache entry
   */
  restoreReplacedInstruction(entry);
#endif /* CONFIG_BLOCK_COPY */
  entry->type = MCE_TYPE_INVALID;
}

static void resolveCacheConflict(TranslationCache *tc, u32int metaIndex)
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
  DEBUG(BLOCK_CACHE, "resolveCacheConflict: M$ collision at index %#x" EOL, metaIndex);

  incrementCollisionCounter();

#ifdef CONFIG_BLOCK_COPY
  removeCodeCacheEntry(tc, tc->metaCache[metaIndex].code, tc->metaCache[metaIndex].codeSize);
#endif

  for (u32int i = 0; i < TRANSLATION_CACHE_META_SIZE_N; i++)
  {
    if (tc->metaCache[i].type != MCE_TYPE_INVALID
        && tc->metaCache[i].endAddress == tc->metaCache[metaIndex].endAddress && i != metaIndex)
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
      if (tc->metaCache[metaIndex].type == MCE_TYPE_ARM)
      {
        u32int hyperCall = (*(u32int *)tc->metaCache[metaIndex].endAddress & 0xFF000000) | ((i + 1) << 8);
        DEBUG(BLOCK_CACHE, "resolveCacheConflict: replacing ARM hypercall with %#.8x" EOL,
            hyperCall);
        *(u32int *)tc->metaCache[metaIndex].endAddress = hyperCall;
        mmuInvIcacheByMVAtoPOU(tc->metaCache[metaIndex].endAddress);
        mmuCleanDcacheByMVAtoPOC(tc->metaCache[metaIndex].endAddress);
      }
#ifdef CONFIG_THUMB2
      else if (tc->metaCache[metaIndex].type == MCE_TYPE_THUMB)
      {
        u16int hyperCall = (*(u16int *)tc->metaCache[metaIndex].endAddress & 0x0000FF00) | (i + 1);
        DEBUG(BLOCK_CACHE, "resolveCacheConflict: replacing T16 hypercall with %#.4x" EOL, hyperCall);
        *(u16int *)tc->metaCache[metaIndex].endAddress = hyperCall;
        mmuInvIcacheByMVAtoPOU(tc->metaCache[metaIndex].endAddress);
        mmuCleanDcacheByMVAtoPOC(tc->metaCache[metaIndex].endAddress);
      }
#endif
      else
      {
        DIE_NOW(NULL, "invalid entry in M$");
      }
      return;
    }
  }
  DEBUG(BLOCK_CACHE, "resolveCacheConflict: no other block ends at the same address" EOL);

  /*
   * Restore replaced instruction (hyperedInstruction) back to its original location in memory.
   */
  restoreReplacedInstruction(&tc->metaCache[metaIndex]);
}

static void restoreReplacedInstruction(MetaCacheEntry *entry)
{
  switch (entry->type)
  {
    case MCE_TYPE_INVALID:
      break;
    case MCE_TYPE_ARM:
      DEBUG(BLOCK_CACHE, "restoreReplacedInstruction: restoring ARM %#.8x @ %#.8x" EOL,
          entry->hyperedInstruction, entry->endAddress);
      *((u32int *)(entry->endAddress)) = entry->hyperedInstruction;
      mmuInvIcacheByMVAtoPOU(entry->endAddress);
      mmuCleanDcacheByMVAtoPOC(entry->endAddress);
      break;
#ifdef CONFIG_THUMB2
    case MCE_TYPE_THUMB:
      if (txxIsThumb32(entry->hyperedInstruction))
      {
        /*
         * Restore Thumb 32-bit instruction. Word-alignment is not guaranteed, so we must perform
         * two halfword-size stores!
         */
        DEBUG(BLOCK_CACHE, "restoreReplacedInstruction: restoring T32 %#.8x @ %#.8x",
            entry->hyperedInstruction, entry->endAddress);
        u16int *bpointer = (u16int *)(entry->endAddress);
        *bpointer = (u16int)(entry->hyperedInstruction >> 16);
        bpointer++;
        *bpointer = (u16int)(entry->hyperedInstruction & 0xFFFF);
        // FIXME: CPU CACHE NOT INVALIDATED (may require invalidation of 2 addresses because of halfword align?)
        printf("FIXME: CPU CACHE NOT INVALIDATED ON T32 RESTORE" EOL);
      }
      else
      {
        /*
         * Restore Thumb 16-bit instruction.
         */
        DEBUG(BLOCK_CACHE, "restoreReplacedInstruction: restoring T16 %#.4x @ %#.8x" EOL,
            entry->hyperedInstruction, entry->endAddress);
        *((u16int *)(entry->endAddress)) = (u16int)entry->hyperedInstruction;
        mmuInvIcacheByMVAtoPOU(entry->endAddress);
        mmuCleanDcacheByMVAtoPOC(entry->endAddress);
      }
      break;
#endif
    default:
      DIE_NOW(NULL, "invalid entry in M$");
  }
}

static void setExecBitMap(TranslationCache *tc, u32int address)
{
  u32int index = address / TRANSLATION_CACHE_MEMORY_PER_BITMAP;
  u32int bitNumber = (address & 0x0FFFFFFF) / TRANSLATION_CACHE_MEMORY_PER_BITMAP_BIT;
  tc->execBitMap[index] = tc->execBitMap[index] | (1 << bitNumber);
}


#ifdef CONFIG_BLOCK_COPY

static u32int *mergeCodeBlock(TranslationCache *tc, u32int *startOfBlock1, u32int *endOfBlock2)
{
  printf("WARNING: magic numbers in checkAndMergeBlock do not match mergeCodeBlock" EOL);
  printf("TODO: clean up mergeCodeBlock both strategy and implementation look horribly inefficient and unreadable" EOL);
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
  /* These are the instructions that are currently at end of blockCopyCache */
  u32int nrInstructions2Move = ((u32int)tc->codeCacheLastEntry - ((u32int)startOfBlock1 & ~1)) >> 2;
  DEBUG(BLOCK_CACHE, "nrInstructions2Move = %#.8x" EOL, nrInstructions2Move);
  /* These are the instructions that are alread at start of blockCopyCache */
  u32int nrInstructions2Shift = endOfBlock2 - tc->codeCache;
  DEBUG(BLOCK_CACHE, "nrInstructions2Shift = %#.8x" EOL, nrInstructions2Shift);
  /**
   * Make place for the instructions that have to be moved
   */
  u32int *destination = endOfBlock2;
  for (u32int k = nrInstructions2Move; k > 0; destination++, k--)
  {
    /* We can use startOfBlock2 instead of gc->blockCopyCache & endOfBlock1 instead of gc->blockCopyCacheEnd*/
    destination = updateCodeCachePointer(tc, destination);
  }
  u32int* blockCopyLast = destination;
#ifdef BLOCK_COPY_CACHE_DEBUG
  printf("Blockcache cleared till the end = %p", blockCopyLast);
#endif
  /*
   * Copy The instructions first the one that are at the start of the blockCopyCache (there the str and ldr instructions need to be rewritten)
   * Than the one at the end of the blockCopyCache (including the reserved Word & backpointer) -> no rewrites needed
   */
  for (u32int k = nrInstructions2Shift; k > 0; k--)
  {
    u32int instruction = *--endOfBlock2;
    destination--;
    if ((instruction & 0xffef0000) == 0xe50f0000)
    {
      /* Offset of instruction needs to be changed + 2 because PC is 2 behind
       * gc->blockCopyCache is address of SVC
       * startOfBlock2 +1 is address of reserved Word */
      u32int offset = (destination - (tc->codeCache + 1) + 2) << 2;
      if (offset > 0xFFFF)
      {
        DIE_NOW(NULL, "Offset is to big -> instruction will get corrupted");
      }
      instruction = (instruction & 0xFFFF0000) | offset;
    }
    *destination = instruction;
  }
  DEBUG(BLOCK_CACHE, "Instructions block2 shifted" EOL);
  /* Copy other block of instructions*/
  u32int *endOfBlock1 = tc->codeCacheLastEntry;
  for (u32int k = nrInstructions2Move; k > 0; k--)
  {
    *(--destination) = *(--endOfBlock1); /* Offsets are still correct, as reserved Word takes the same translation*/
    /* And Clear the memory*/
    *endOfBlock1 = 0;
  }
  DEBUG(BLOCK_CACHE, "Instructions block1 Copied" EOL);
  /*
   * Make sure that block is safed correctly in blockCopyCache
   */
  return blockCopyLast;
}

/*
 * This code will check if a block that is split (splitting occurs due to 1 part being at the end of the blockCopyCache and another at the
 * begin of the block.)  When a block is split up the code will also check if the second part wants to make use of a reserved word.  If that is
 * the case the block will result in erroneous behavior and should be merged. This code will perform the merge and will return a pointer indicating
 * the end of the merged block (the word just after the block, this is similar to blockCopyCurrCacheAddress). After the merge the merged block
 * will be completely at the start of the blockCopyCache.
 */
u32int *mergeCodeBlockAsNeeded(TranslationCache *tc, u32int *startOfBlock1, u32int *endOfBlock2)
{
  DEBUG(BLOCK_CACHE, "checkAndMergeBlock with endOfBlock2 = %p & startOfBlock1 = %p" EOL, endOfBlock2, startOfBlock1);

  printf("WARNING: magic numbers in checkAndMergeBlock do not match mergeCodeBlock" EOL);

  for (u32int *p = endOfBlock2 - 1; p > (tc->codeCache - 1); p--)
  {
    /*
     * If there is a ldr instruction that reads from PC + offset than that will be a load that we installed that wants to read
     * the reserved word.  This is not possible since the reserved word will be somewhere near the end of the blockCopyCache
     * Therefore we need to patch the code.  If no such ldr instruction is present than there won't be a problem.
     * We are sure that we need to patch the code because the code is already translated and cannot read from PC if it was an original instruction
     * Instruction will be a load literal 0xe51f????
     */
    if ((*p & 0xffff0000) == 0xe51f0000)
    {
      /* We have found a problem*/
      /* patching needs to be done: set codeSize -> must be done in scanBlock (caller of this function) */
      return mergeCodeBlock(tc, startOfBlock1, endOfBlock2);
    }
  }
  /* No patching needs to be done just set codeSize -> must be done in scanBlock (caller of this function) */
  return endOfBlock2;
}

// WARNING: code size MUST BE IN BYTES
static void removeCodeCacheEntry(const TranslationCache *tc, u32int *code, u32int codeSize)
{
  /*
   * Is the block contiguous or split?
   */
  const u32int codeEndAddress = (u32int)code + codeSize;
  if (codeEndAddress < (u32int)tc->codeCacheLastEntry)
  {
    /*
     * Contiguous. Zero-fill the whole block at once.
     */
    memset(code, 0, codeSize);
  }
  else
  {
    /*
     * Split block!
     */
    u32int firstChunkSize = (u32int)tc->codeCacheLastEntry - codeEndAddress;
    memset(code, 0, firstChunkSize);
    memset(tc->codeCache, 0, codeSize - firstChunkSize);
  }
}

/*
 * updateCodeCachePointerInRange
 * Frees up space in the block cache within the specified range and returns the next usable pointer
 * to put an instruction.
 */
u32int *updateCodeCachePointer(TranslationCache *tc, u32int *pointer)
{
  /*
   * Check if pointer is within the given bounds; if not, wrap around.
   */
  if (pointer >= tc->codeCacheLastEntry)
  {
    DEBUG(BLOCK_CACHE, "C$ wrap-around at %p" EOL, tc->codeCacheLastEntry);
    pointer = tc->codeCache;
  }
  /*
   * If the cache contains an entry at this location, free it.
   * The entry should contain a backpointer into the M$. Use it to remove the M$ entry too.
   *
   * TODO: memsetting all the time is probably making this SLOOOOOOOOOOOOOW. Do this another way.
   * + We don't need the full 32 bits for the index to be here.
   * + Do we really need the backpointer to be in user-readable memory ? side-channel!!
   */
  if (*pointer == 0)
  {
    return pointer;
  }
  DEBUG(BLOCK_CACHE, "C$ not clean @ %p" EOL, pointer);
  MetaCacheEntry *entry = (MetaCacheEntry *)(*pointer);
  ASSERT(entry >= tc->metaCache && entry < (tc->metaCache + TRANSLATION_CACHE_META_SIZE_N),
         "invalid backpointer");
  DEBUG(BLOCK_CACHE, "cleaning block with meta index %#.3x" EOL, entry - tc->metaCache);
  removeCacheEntry(tc, entry);
  return pointer;
}

#endif /* CONFIG_BLOCK_COPY */
