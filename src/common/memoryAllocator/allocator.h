#ifndef __COMMON__MEMORY_ALLOCATOR__ALLOCATOR_H__
#define __COMMON__MEMORY_ALLOCATOR__ALLOCATOR_H__

/*
 * Shared header for all memory allocators.
 */


#include "common/compiler.h"
#include "common/types.h"


void initialiseAllocator(u32int startAddress, u32int bytes) __cold__;

void dumpAllocatorInternals(void);


#endif
