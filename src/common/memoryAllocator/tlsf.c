/*
 * Two level segregated fit (TLSF) dynamic storage allocator for real-time systems.
 *
 * This allocator exhibits time-bounded O(1) behavior.
 *
 * The code is adapted from an implementation by Matthew Conte (matt@baisoku.org) released into
 * the public domain (see http://tlsf.baisoku.org).
 *
 * Features:
 *  - O(1) cost for malloc, free, realloc, memalign
 *  - Extremely low overhead per allocation (4 bytes)
 *  - Low overhead per pool (~3kB)
 *  - Low fragmentation
 *  - Compiles to only a few kB of code and data
 *
 * Caveats:
 *  - Currently, assumes architecture can make 4-byte aligned accesses
 *  - Not designed to be thread safe; the user must provide this
 *
 * Known Issues:
 * Due to the internal block structure size and the implementation details of tlsf_memalign, there
 * is worst-case behavior when requesting small (<16 byte) blocks aligned to 8-byte boundaries.
 * Overuse of memalign will generally increase fragmentation, but this particular case will leave
 * lots of unusable "holes" in the heap. The solution would be to internally align all blocks to
 * 8 bytes, but this will require significant changes to the implementation.
 */


#include "common/debug.h"
#include "common/stddef.h"
#include "common/stdlib.h"
#include "common/string.h"

#include "common/memoryAllocator/allocator.h"
#include "common/memoryAllocator/tlsf.h"


// TODO use bit.h
/*
** NOTE: TLSF spec relies on ffs/fls returning value 0..31.
** ffs/fls return 1-32 by default, returning 0 for error.
*/
static int tlsf_fls(unsigned int word)
{
                                const int bit = word ? 32 - __builtin_clz(word) : 0;
                                return bit - 1;
}

static int tlsf_ffs(unsigned int word)
{
                                return __builtin_ffs(word) - 1;
}


/*
** The size of the block header exposed to used blocks is the size field.
** The prev_phys_block field is stored *inside* the previous free block.
*/
static const u32int BLOCK_HEADER_OVERHEAD = sizeof(u32int);

/* User data starts directly after the size field in a used block. */
static const u32int BLOCK_START_OFFSET = offsetof(struct blockHeader, size) + sizeof(u32int);

/*
** A free block must be large enough to store its header minus the size of
** the prev_phys_block field, and no larger than the number of addressable
** bits for FL_INDEX.
*/
static const u32int BLOCK_SIZE_MIN = sizeof(struct blockHeader) - sizeof(struct blockHeader *);
static const u32int BLOCK_SIZE_MAX = 1U << FL_INDEX_MAX;


static struct blockHeader *absorbBlock(struct blockHeader *previousBlock, struct blockHeader *block);
static u32int adjustRequestSize(u32int size, u32int align);
static inline u32int alignDown(u32int x, u32int align);
static void *alignPointer(const void *ptr, u32int align);
static inline u32int alignUp(u32int x, u32int align);
static int canSplitBlock(struct blockHeader *block, u32int size);
static void *createPool(void *start, u32int bytes);
static struct blockHeader *getNextBlock(const struct blockHeader *block);
static inline u32int getOverhead(void);
static void insertBlock(struct pool *p, struct blockHeader *block);
static void insertFreeBlock(struct pool *p, struct blockHeader *block, int firstLevelIndex, int secondLevelIndex);
static void insertMapping(u32int size, int *firstLevelIndex, int *secondLevelIndex);
static int isLastBlock(const struct blockHeader *block);
static struct blockHeader *linkBlockWithNext(struct blockHeader *block);
static struct blockHeader *locateFreeBlock(struct pool *p, u32int size);
static void markBlockFree(struct blockHeader *block);
static void markBlockUsed(struct blockHeader *block);
static struct blockHeader *mergeBlockWithPrevious(struct pool *p, struct blockHeader *block);
static struct blockHeader *mergeBlockWithNext(struct pool *p, struct blockHeader *block);
static void *prepareBlockForUse(struct pool *p, struct blockHeader *block, u32int size);
static void removeBlock(struct pool *p, struct blockHeader *block);
static void removeFreeBlock(struct pool *p, struct blockHeader *block, int firstLevelIndex, int secondLevelIndex);
static void searchMapping(u32int size, int *firstLevelIndex, int *secondLevelIndex);
static struct blockHeader *searchSuitableBlock(struct pool *p, int *firstLevelIndex, int *secondLevelIndex);
static struct blockHeader *splitBlock(struct blockHeader *block, u32int size);
static void *tlsfAlign(struct pool *staticPool, u32int alignment, u32int bytes);
static void *tlsfAllocate(struct pool *p, u32int size);
static inline void tlsfFree(struct pool *p, void *ptr);
static void *tlsfReallocate(struct pool *pool, void *ptr, u32int size);
static void trimFreeBlock(struct pool *p, struct blockHeader *block, u32int size);
static struct blockHeader *trimLeadingFreeBlock(struct pool *p, struct blockHeader *block, u32int size);
static void trimUsedBlock(struct pool *p, struct blockHeader *block, u32int size);


static struct pool *staticPool;


/* Absorb a free block's storage into an adjacent previous free block. */
static struct blockHeader *absorbBlock(struct blockHeader *previousBlock, struct blockHeader *block)
{
  ASSERT(!isLastBlock(previousBlock), "previous block can't be last");
  /* Note: Leaves flags untouched. */
  previousBlock->size += (block->size & BLOCK_HEADER_SIZE_BITS) + BLOCK_HEADER_OVERHEAD;
  linkBlockWithNext(previousBlock);
  return previousBlock;
}

static u32int adjustRequestSize(u32int size, u32int align)
{
  u32int adjust = 0;
  if (size && size < BLOCK_SIZE_MAX)
  {
    const u32int aligned = alignUp(size, align);
    adjust = aligned > BLOCK_SIZE_MIN ? aligned : BLOCK_SIZE_MIN;
  }
  return adjust;
}

static inline u32int alignDown(u32int x, u32int align)
{
  ASSERT(0 == (align & (align - 1)), "must align to a power of two");
  return x - (x & (align - 1));
}

static void *alignPointer(const void *ptr, u32int align)
{
  const u32int aligned = (((u32int) ptr) + (align - 1)) & ~(align - 1);
  ASSERT(0 == (align & (align - 1)), "must align to a power of two");
  return (void *)aligned;
}

static inline u32int alignUp(u32int x, u32int align)
{
  ASSERT(0 == (align & (align - 1)), "must align to a power of two");
  return (x + (align - 1)) & ~(align - 1);
}

static int canSplitBlock(struct blockHeader *block, u32int size)
{
  return (block->size & BLOCK_HEADER_SIZE_BITS) >= sizeof(struct blockHeader) + size;
}

static void *createPool(void *start, u32int bytes)
{
  const u32int poolOverhead = getOverhead();
  const u32int poolBytes = alignDown(bytes - poolOverhead, ALIGN_SIZE);

  ASSERT(poolBytes >= BLOCK_SIZE_MIN && poolBytes <= BLOCK_SIZE_MAX, "invalid pool size");

  /* Construct a valid pool object. */
  int i, j;
  struct pool *p = (struct pool *)start;
  p->nullBlock.nextFree = &p->nullBlock;
  p->nullBlock.previousFree = &p->nullBlock;
  p->firstLevelBitmap = 0;
  for (i = 0; i < FL_INDEX_COUNT; ++i)
  {
    p->secondLevelBitmap[i] = 0;
    for (j = 0; j < SL_INDEX_COUNT; ++j)
    {
      p->blocks[i][j] = &p->nullBlock;
    }
  }

  /*
  ** Create the main free block. Offset the start of the block slightly
  ** so that the prev_phys_block field falls inside of the pool
  ** structure - it will never be used.
  */
  struct blockHeader *block = (struct blockHeader *)((u32int)p + (sizeof(struct pool) - BLOCK_HEADER_OVERHEAD));
  block->size = poolBytes | BLOCK_HEADER_FREE_BIT;
  insertBlock(p, block);

  /* Split the block to create a zero-size pool sentinel block. */
  struct blockHeader *next = linkBlockWithNext(block);
  next->size = BLOCK_HEADER_PREV_FREE_BIT;

  return p;
}

void free(void *ptr)
{
  tlsfFree(staticPool, ptr);
}

/* Return location of next existing block. */
static struct blockHeader *getNextBlock(const struct blockHeader *block)
{
  ASSERT(!isLastBlock(block), "block cannot be last");
  return (struct blockHeader *)((u32int)block + BLOCK_START_OFFSET + (block->size & BLOCK_HEADER_SIZE_BITS) - BLOCK_HEADER_OVERHEAD);
}

/*
** Overhead of the TLSF structures in a given memory block passed to
** tlsf_create, equal to the size of a pool_t plus overhead of the initial
** free block and the sentinel block.
*/
static inline u32int getOverhead()
{
  return sizeof(struct pool) + 2 * BLOCK_HEADER_OVERHEAD;
}

void initialiseAllocator(u32int startAddress, u32int bytes)
{
  staticPool = createPool((void *)startAddress, bytes);
}

/* Insert a given block into the free list. */
static void insertBlock(struct pool *p, struct blockHeader *block)
{
  int firstLevelIndex, secondLevelIndex;
  insertMapping(block->size & BLOCK_HEADER_SIZE_BITS, &firstLevelIndex, &secondLevelIndex);
  insertFreeBlock(p, block, firstLevelIndex, secondLevelIndex);
}

/* Insert a free block into the free block list. */
static void insertFreeBlock(struct pool *p, struct blockHeader *block, int firstLevelIndex, int secondLevelIndex)
{
  struct blockHeader *current = p->blocks[firstLevelIndex][secondLevelIndex];
  ASSERT(current, "free list cannot have a null entry");
  ASSERT(block, "cannot insert a null entry into the free list");
  block->nextFree = current;
  block->previousFree = &p->nullBlock;
  current->previousFree = block;

  char *blockPtr = (char *)block + BLOCK_START_OFFSET;
  ASSERT(blockPtr == alignPointer(blockPtr, ALIGN_SIZE), "block not aligned properly");

  /*
  ** Insert the new block at the head of the list, and mark the first-
  ** and second-level bitmaps appropriately.
  */
  p->blocks[firstLevelIndex][secondLevelIndex] = block;
  p->firstLevelBitmap |= (1 << firstLevelIndex);
  p->secondLevelBitmap[firstLevelIndex] |= (1 << secondLevelIndex);
}

static void insertMapping(u32int size, int *firstLevelIndex, int *secondLevelIndex)
{
  if (size < SMALL_BLOCK_SIZE)
  {
    /* Store small blocks in first list. */
    *firstLevelIndex = 0;
    *secondLevelIndex = ((int)size) / (SMALL_BLOCK_SIZE / SL_INDEX_COUNT);
  }
  else
  {
    *firstLevelIndex = tlsf_fls(size);
    *secondLevelIndex = (int)(size >> (*firstLevelIndex - SL_INDEX_COUNT_LOG2)) ^ (1 << SL_INDEX_COUNT_LOG2);
    *firstLevelIndex -= (FL_INDEX_SHIFT - 1);
  }
}

static int isLastBlock(const struct blockHeader *block)
{
  return 0 == (block->size & BLOCK_HEADER_SIZE_BITS);
}

/* Link a new block with its physical neighbor, return the neighbor. */
static struct blockHeader *linkBlockWithNext(struct blockHeader *block)
{
  struct blockHeader *next = getNextBlock(block);
  next->previous = block;
  return next;
}

static struct blockHeader *locateFreeBlock(struct pool *p, u32int size)
{
  int firstLevelIndex = 0, secondLevelIndex = 0;
  struct blockHeader *block = 0;

  if (size)
  {
    searchMapping(size, &firstLevelIndex, &secondLevelIndex);
    block = searchSuitableBlock(p, &firstLevelIndex, &secondLevelIndex);
  }

  if (block)
  {
    ASSERT((block->size & BLOCK_HEADER_SIZE_BITS) >= size, "block size too small");
    removeFreeBlock(p, block, firstLevelIndex, secondLevelIndex);
  }

  return block;
}

void *malloc(u32int size)
{
  return tlsfAllocate(staticPool, size);
}

static void markBlockFree(struct blockHeader *block)
{
  /* Link the block to the next block, first. */
  struct blockHeader *next = linkBlockWithNext(block);
  next->size |= BLOCK_HEADER_PREV_FREE_BIT;
  block->size |= BLOCK_HEADER_FREE_BIT;
}

static void markBlockUsed(struct blockHeader *block)
{
  struct blockHeader *next = getNextBlock(block);
  next->size &= ~BLOCK_HEADER_PREV_FREE_BIT;
  block->size &= ~BLOCK_HEADER_FREE_BIT;
}

void *memalign(u32int alignment, u32int size)
{
  return tlsfAlign(staticPool, alignment, size);
}

/* Merge a just-freed block with an adjacent previous free block. */
static struct blockHeader *mergeBlockWithPrevious(struct pool *p, struct blockHeader *block)
{
  if ((block->size & BLOCK_HEADER_PREV_FREE_BIT))
  {
    struct blockHeader *previousBlock = block->previous;
    ASSERT(previousBlock, "prev physical block can't be null");
    ASSERT((previousBlock->size & BLOCK_HEADER_FREE_BIT), "prev block is not free though marked as such");
    removeBlock(p, previousBlock);
    block = absorbBlock(previousBlock, block);
  }

  return block;
}

/* Merge a just-freed block with an adjacent free block. */
static struct blockHeader *mergeBlockWithNext(struct pool *p, struct blockHeader *block)
{
  struct blockHeader *next = getNextBlock(block);
  ASSERT(next, "next physical block can't be null");

  if ((next->size & BLOCK_HEADER_FREE_BIT))
  {
    ASSERT(!isLastBlock(block), "previous block can't be last!");
    removeBlock(p, next);
    block = absorbBlock(block, next);
  }

  return block;
}

static void *prepareBlockForUse(struct pool *p, struct blockHeader *block, u32int size)
{
  void *ptr = 0;
  if (block)
  {
    trimFreeBlock(p, block, size);
    markBlockUsed(block);
    ptr = (void *)((unsigned char *)(block) + BLOCK_START_OFFSET);
  }
  return ptr;
}

void *realloc(void *ptr, u32int size)
{
  return tlsfReallocate(staticPool, ptr, size);
}

/* Remove a given block from the free list. */
static void removeBlock(struct pool *p, struct blockHeader *block)
{
  int firstLevelIndex, secondLevelIndex;
  insertMapping((block->size & BLOCK_HEADER_SIZE_BITS), &firstLevelIndex, &secondLevelIndex);
  removeFreeBlock(p, block, firstLevelIndex, secondLevelIndex);
}

/* Remove a free block from the free list.*/
static void removeFreeBlock(struct pool *p, struct blockHeader *block, int firstLevelIndex, int secondLevelIndex)
{
  ASSERT(block->previousFree, "previousFree field can not be null");
  ASSERT(block->nextFree, "nextFree field can not be null");

  /* If this block is the head of the free list, set new head. */
  if (p->blocks[firstLevelIndex][secondLevelIndex] == block)
  {
    p->blocks[firstLevelIndex][secondLevelIndex] = block->nextFree;

    /* If the new head is null, clear the bitmap. */
    if (block->nextFree == &p->nullBlock)
    {
      p->secondLevelBitmap[firstLevelIndex] &= ~(1 << secondLevelIndex);

      /* If the second bitmap is now empty, clear the first level bitmap. */
      if (!p->secondLevelBitmap[firstLevelIndex])
      {
        p->firstLevelBitmap &= ~(1 << firstLevelIndex);
      }
    }
  }
}

/* This version rounds up to the next block size (for allocations) */
static void searchMapping(u32int size, int *firstLevelIndex, int *secondLevelIndex)
{
  if (size >= (1 << SL_INDEX_COUNT_LOG2))
  {
    size += (1 << (tlsf_fls(size) - SL_INDEX_COUNT_LOG2)) - 1;
  }
  insertMapping(size, firstLevelIndex, secondLevelIndex);
}

static struct blockHeader *searchSuitableBlock(struct pool *p, int *firstLevelIndex, int *secondLevelIndex)
{
  /*
  ** First, search for a block in the list associated with the given
  ** fl/sl index.
  */
  u32int secondLevelBitmap = p->secondLevelBitmap[*firstLevelIndex] & (~0 << *secondLevelIndex);
  if (!secondLevelBitmap)
  {
    /* No block exists. Search in the next largest first-level list. */
    const u32int firstLevelBitmap = p->firstLevelBitmap & (~0 << (*firstLevelIndex + 1));
    if (!firstLevelBitmap)
    {
      /* No free blocks available, memory has been exhausted. */
      return 0;
    }

    *firstLevelIndex = tlsf_ffs(firstLevelBitmap);
    secondLevelBitmap = p->secondLevelBitmap[*firstLevelIndex];
  }
  ASSERT(secondLevelBitmap, "internal error - second level bitmap is null");
  *secondLevelIndex = tlsf_ffs(secondLevelBitmap);

  /* Return the first block in the free list. */
  return p->blocks[*firstLevelIndex][*secondLevelIndex];
}

/* Split a block into two, the second of which is free. */
static struct blockHeader *splitBlock(struct blockHeader *block, u32int size)
{
  /* Calculate the amount of space left in the remaining block. */
  struct blockHeader *remainingBlock = (struct blockHeader *)(((u32int)((void*)((unsigned char*)block + BLOCK_START_OFFSET)) + (size - BLOCK_HEADER_OVERHEAD)));

  const u32int remainingSize = (block->size & BLOCK_HEADER_SIZE_BITS) - (size + BLOCK_HEADER_OVERHEAD);

  void *remainingBlockPtr = (void *)((unsigned char *)remainingBlock + BLOCK_START_OFFSET);
  ASSERT(remainingBlockPtr == alignPointer(remainingBlockPtr, ALIGN_SIZE), "remaining block not aligned properly");

  ASSERT((block->size & BLOCK_HEADER_SIZE_BITS) == remainingSize + size + BLOCK_HEADER_OVERHEAD, "invalid split block size");

  remainingBlock->size &= ~BLOCK_HEADER_SIZE_BITS;
  remainingBlock->size |= remainingSize;

  ASSERT((remainingBlock->size & BLOCK_HEADER_SIZE_BITS) >= BLOCK_SIZE_MIN, "block split with invalid size");

  block->size &= ~BLOCK_HEADER_SIZE_BITS;
  block->size |= size;
  markBlockFree(remainingBlock);

  return remainingBlock;
}

static void *tlsfAlign(struct pool *pool, u32int alignment, u32int size)
{
  const u32int adjustedSize = adjustRequestSize(size, ALIGN_SIZE);

  /*
  ** We must allocate an additional minimum block size bytes so that if
  ** our free block will leave an alignment gap which is smaller, we can
  ** trim a leading free block and release it back to the heap. We must
  ** do this because the previous physical block is in use, therefore
  ** the previous field is not valid, and we can't simply adjust
  ** the size of that block.
  */
  const u32int minimumGap = sizeof(struct blockHeader);
  const u32int sizeWithGap = adjustRequestSize(adjustedSize + alignment + minimumGap, alignment);

  /* If alignment is less than or equals base alignment, we're done. */
  const u32int alignedSize = (alignment <= ALIGN_SIZE) ? adjustedSize : sizeWithGap;

  struct blockHeader *block = locateFreeBlock(pool, alignedSize);

  /* This can't be a static assert. */
  ASSERT(sizeof(struct blockHeader) == BLOCK_SIZE_MIN + BLOCK_HEADER_OVERHEAD, "block size bug");

  if (block)
  {
    void *ptr = (void *)((unsigned char *)block + BLOCK_START_OFFSET);

    void *aligned = alignPointer(ptr, alignment);
    u32int gap = (u32int)aligned - (u32int)ptr;

    /* If gap size is too small, offset to next aligned boundary. */
    if (gap && gap < minimumGap)
    {
      const u32int gapRemainder = minimumGap - gap;
      const u32int offset = gapRemainder > alignment ? gapRemainder : alignment;
      const void *nextAligned = (void *)((u32int)aligned + offset);

      aligned = alignPointer(nextAligned, alignment);
      gap = (u32int)aligned - (u32int)ptr;
    }

    if (gap)
    {
      ASSERT(gap >= minimumGap, "gap size too small");
      block = trimLeadingFreeBlock(pool, block, gap);
    }
  }

  return prepareBlockForUse(pool, block, adjustedSize);
}

static void *tlsfAllocate(struct pool *p, u32int size)
{
  const u32int adjust = adjustRequestSize(size, ALIGN_SIZE);
  return prepareBlockForUse(p, locateFreeBlock(p, adjust), adjust);
}

static inline void tlsfFree(struct pool *p, void *ptr)
{
  struct blockHeader *block = (struct blockHeader *)((char *)ptr - BLOCK_START_OFFSET);
  markBlockFree(block);
  block = mergeBlockWithPrevious(p, block);
  block = mergeBlockWithNext(p, block);
  insertBlock(p, block);
}

/*
** The TLSF block information provides us with enough information to
** provide a reasonably intelligent implementation of realloc, growing or
** shrinking the currently allocated block as required.
**
** This routine handles the somewhat esoteric edge cases of realloc:
** - a non-zero size with a null pointer will behave like malloc
** - a zero size with a non-null pointer will behave like free
** - a request that cannot be satisfied will leave the original buffer
**   untouched
** - an extended buffer size will leave the newly-allocated area with
**   contents undefined
*/
void *tlsfReallocate(struct pool *pool, void *ptr, u32int size)
{
  void *p = 0;

  /* Zero-size requests are treated as free. */
  if (ptr && size == 0)
  {
    tlsfFree(pool, ptr);
  }
  /* Requests with NULL pointers are treated as malloc. */
  else if (!ptr)
  {
    p = tlsfAllocate(pool, size);
  }
  else
  {
    struct blockHeader *block = (struct blockHeader *)((unsigned char *)ptr - BLOCK_START_OFFSET);
    struct blockHeader *next = getNextBlock(block);

    const u32int currentSize = (block->size & BLOCK_HEADER_SIZE_BITS);
    const u32int combinedSize = currentSize + (next->size & BLOCK_HEADER_SIZE_BITS) + BLOCK_HEADER_OVERHEAD;
    const u32int adjustedSize = adjustRequestSize(size, ALIGN_SIZE);

    /*
    ** If the next block is used, or when combined with the current
    ** block, does not offer enough space, we must reallocate and copy.
    */
    if (adjustedSize > currentSize && (!(next->size & BLOCK_HEADER_FREE_BIT) || adjustedSize > combinedSize))
    {
      p = tlsfAllocate(pool, size);
      if (p)
      {
        memcpy(p, ptr, currentSize < size ? currentSize : size);
        tlsfFree(pool, ptr);
      }
    }
    else
    {
      /* Do we need to expand to the next block? */
      if (adjustedSize > currentSize)
      {
        mergeBlockWithNext(pool, block);
        markBlockUsed(block);
      }

      /* Trim the resulting block and return the original pointer. */
      trimUsedBlock(pool, block, adjustedSize);
      p = ptr;
    }
  }

  return p;
}

/* Trim any trailing block space off the end of a block, return to pool. */
static void trimFreeBlock(struct pool *p, struct blockHeader *block, u32int size)
{
  ASSERT((block->size & BLOCK_HEADER_FREE_BIT), "block must be free");
  if (canSplitBlock(block, size))
  {
    struct blockHeader *remainingBlock = splitBlock(block, size);
    linkBlockWithNext(block);
    remainingBlock->size |= BLOCK_HEADER_PREV_FREE_BIT;
    insertBlock(p, remainingBlock);
  }
}

static struct blockHeader *trimLeadingFreeBlock(struct pool *p, struct blockHeader *block, u32int size)
{
  struct blockHeader *remainingBlock = block;
  if (canSplitBlock(block, size))
  {
    /* We want the 2nd block. */
    remainingBlock = splitBlock(block, size - BLOCK_HEADER_OVERHEAD);
    remainingBlock->size |= BLOCK_HEADER_PREV_FREE_BIT;

    linkBlockWithNext(block);
    insertBlock(p, block);
  }
  return remainingBlock;
}

/* Trim any trailing block space off the end of a used block, return to pool. */
static void trimUsedBlock(struct pool *p, struct blockHeader *block, u32int size)
{
  ASSERT(!(block->size & BLOCK_HEADER_FREE_BIT), "block must be used");
  if (canSplitBlock(block, size))
  {
    /* If the next block is free, we must coalesce. */
    struct blockHeader *remainingBlock = splitBlock(block, size);
    remainingBlock->size |= BLOCK_HEADER_PREV_FREE_BIT;
    remainingBlock = mergeBlockWithNext(p, remainingBlock);
    insertBlock(p, remainingBlock);
  }
}
