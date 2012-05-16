#ifndef __GUEST_MANAGER__CODE_CACHE_ALLOCATOR_H__
#define __GUEST_MANAGER__CODE_CACHE_ALLOCATOR_H__

#include "common/types.h"

#include "guestManager/guestContext.h"

#include "memoryManager/pageTable.h"


#define CODE_CACHE_MIN_SIZE  128u
#define CODE_CACHE_MAX_SIZE  SMALL_PAGE_SIZE


bool allocateCodeCache(GCONTXT *context);
void freeCodeCache(GCONTXT *context);

#endif /* __GUEST_MANAGER__TRANSLATION_CACHE_ALLOCATOR_H__ */
