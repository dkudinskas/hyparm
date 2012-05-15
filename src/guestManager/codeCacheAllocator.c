/*
 * Simple slot-based allocator for code caches.
 *
 * Code caches cannot be mapped in the non-executable RAM pool and are visible to guests. Ideally,
 * execution beyond the borders of this cache is prohibited, and guests MUST NOT be able to see
 * each other's translation caches.
 */

#include "common/bit.h"
#include "common/linker.h"
#include "common/stddef.h"

#include "guestManager/codeCacheAllocator.h"


#define MAX_CACHES  ((u32int)((RAM_CODE_CACHE_POOL_END - RAM_CODE_CACHE_POOL_BEGIN) / SMALL_PAGE_SIZE / 2))


static u32int allocatedCaches = 0;


bool allocateCodeCache(GCONTXT *context)
{
  u32int nextIndex = findFirstBitSet(~allocatedCaches) - 1;
  if (nextIndex > MAX_CACHES)
  {
    return FALSE;
  }

  nextIndex *= 2;
  context->translationCache.codeCache = (u32int *)(RAM_CODE_CACHE_POOL_BEGIN + (nextIndex * SMALL_PAGE_SIZE));
  context->translationCache.spillPage = (u32int *)(RAM_CODE_CACHE_POOL_BEGIN + ((nextIndex + 1) * SMALL_PAGE_SIZE));

  return TRUE;
}

void freeCodeCache(GCONTXT *context)
{
  //allocatedCaches &= ~(1 << (((u32int)translationCache - RAM_X_POOL_BEGIN) >> findFirstBitSet(SMALL_PAGE_MASK)));
}
