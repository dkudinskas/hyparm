#include "common/bit.h"
#include "common/linker.h"
#include "common/stddef.h"

#include "guestManager/codeCacheAllocator.h"

#include "memoryManager/pageTable.h"


/*
 * Simple slot-based allocator for code caches.
 *
 * Code caches cannot be mapped in the non-executable RAM pool and are visible to guests. Ideally,
 * execution beyond the borders of this cache is prohibited, and guests MUST NOT be able to see
 * each other's translation caches.
 */


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

  const u32int codeAddress = (u32int)context->translationCache.codeCache;
  const u32int spillAddress = (u32int)context->translationCache.spillPage;
  mapRange(context->pageTables->shadowPriv, codeAddress, codeAddress,
           codeAddress + SMALL_PAGE_SIZE, GUEST_ACCESS_DOMAIN, PRIV_RW_USR_RO, FALSE, FALSE, 0,
           FALSE);
  mapRange(context->pageTables->shadowPriv, spillAddress, spillAddress,
           spillAddress + SMALL_PAGE_SIZE, GUEST_ACCESS_DOMAIN, PRIV_RW_USR_RW, FALSE, FALSE, 0,
           TRUE);

  return TRUE;
}


void freeCodeCache(GCONTXT *context)
{
  //allocatedCaches &= ~(1 << (((u32int)translationCache - RAM_X_POOL_BEGIN) >> findFirstBitSet(SMALL_PAGE_MASK)));
}
