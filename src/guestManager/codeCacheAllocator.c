#include "common/bit.h"
#include "common/linker.h"
#include "common/stddef.h"

#include "guestManager/codeCacheAllocator.h"


/*
 * Simple slot-based allocator for code caches.
 *
 * Code caches cannot be mapped in the non-executable RAM pool and are visible to guests. Ideally,
 * execution beyond the borders of this cache is prohibited, and guests MUST NOT be able to see
 * each other's translation caches.
 */


#define MAX_CACHES  ((u32int)((RAM_X_POOL_END - RAM_X_POOL_BEGIN) / SMALL_PAGE_SIZE))


static u32int allocatedCaches = 0;


u32int *allocateCodeCache()
{
  u32int nextIndex = findFirstBitSet(~allocatedCaches) - 1;
  if (nextIndex > MAX_CACHES)
  {
    return NULL;
  }
  return (u32int *)(RAM_X_POOL_BEGIN + (nextIndex * SMALL_PAGE_SIZE));
}

void freeCodeCache(u32int *translationCache)
{
  allocatedCaches &= ~(1 << (((u32int)translationCache - RAM_X_POOL_BEGIN) >> findFirstBitSet(SMALL_PAGE_MASK)));
}
