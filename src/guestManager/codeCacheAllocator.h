#ifndef __GUEST_MANAGER__CODE_CACHE_ALLOCATOR_H__
#define __GUEST_MANAGER__CODE_CACHE_ALLOCATOR_H__

#include "common/types.h"

#include "memoryManager/pageTable.h"


#define CODE_CACHE_MIN_SIZE  128u
#define CODE_CACHE_MAX_SIZE  SMALL_PAGE_SIZE


u32int *allocateCodeCache(void);
void freeCodeCache(u32int *translationCache);

#endif /* __GUEST_MANAGER__TRANSLATION_CACHE_ALLOCATOR_H__ */
