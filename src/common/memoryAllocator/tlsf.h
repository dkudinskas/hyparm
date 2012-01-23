#ifndef __COMMON__MEMORY_ALLOCATOR__TLSF_H__
#define __COMMON__MEMORY_ALLOCATOR__TLSF_H__

/*
 * Private header for the TLSF memory allocator.
 */


#include "common/types.h"


/* Public constants: may be modified. */
enum tlsfPublicConstants
{
  /* log2 of number of linear subdivisions of block sizes. */
  SL_INDEX_COUNT_LOG2 = 5,
};

/* Private constants: do not modify. */
enum tlsfPrivateConstants
{
  /* All allocation sizes and addresses are aligned to 4 bytes. */
  ALIGN_SIZE_LOG2 = 2,
  ALIGN_SIZE = (1 << ALIGN_SIZE_LOG2),

  /*
  ** We support allocations of sizes up to (1 << FL_INDEX_MAX) bits.
  ** However, because we linearly subdivide the second-level lists, and
  ** our minimum size granularity is 4 bytes, it doesn't make sense to
  ** create first-level lists for sizes smaller than SL_INDEX_COUNT * 4,
  ** or (1 << (SL_INDEX_COUNT_LOG2 + 2)) bytes, as there we will be
  ** trying to split size ranges into more slots than we have available.
  ** Instead, we calculate the minimum threshold size, and place all
  ** blocks below that size into the 0th first-level list.
  */

  FL_INDEX_MAX = 30,
  SL_INDEX_COUNT = (1 << SL_INDEX_COUNT_LOG2),
  FL_INDEX_SHIFT = (SL_INDEX_COUNT_LOG2 + ALIGN_SIZE_LOG2),
  FL_INDEX_COUNT = (FL_INDEX_MAX - FL_INDEX_SHIFT + 1),

  SMALL_BLOCK_SIZE = (1 << FL_INDEX_SHIFT),
};


/*
 * Since block sizes are always at least a multiple of 4, the two least
 * significant bits of the size field are used to store the block status:
 * - bit 0: whether block is busy or free
 * - bit 1: whether previous block is busy or free
 */
#define BLOCK_HEADER_FREE_BIT       0x1
#define BLOCK_HEADER_PREV_FREE_BIT  0x2
#define BLOCK_HEADER_SIZE_BITS      ~(BLOCK_HEADER_PREV_FREE_BIT | BLOCK_HEADER_FREE_BIT)


struct blockHeader
{
  /* Points to the previous physical block. */
  struct blockHeader *previous;

  /* The size of this block, excluding the block header. */
  u32int size;

  /* Next and previous free blocks. */
  struct blockHeader *nextFree;
  struct blockHeader *previousFree;
};

struct pool
{
  /* Empty lists point at this block to indicate they are free. */
  struct blockHeader nullBlock;

  /* Bitmaps for free lists. */
  u32int firstLevelBitmap;
  u32int secondLevelBitmap[FL_INDEX_COUNT];

  /* Head of free lists. */
  struct blockHeader *blocks[FL_INDEX_COUNT][SL_INDEX_COUNT];
};

#endif /* __COMMON__MEMORY_ALLOCATOR__TLSF_H__ */
