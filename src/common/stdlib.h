#ifndef __COMMON__STDLIB_H__
#define __COMMON__STDLIB_H__

#include "common/compiler.h"
#include "common/stddef.h"
#include "common/string.h"
#include "common/types.h"


#ifdef TEST

#include <stdlib.h>

#else

#define abs(n)    __builtin_abs(n)

#define llabs(n)  __builtin_llabs(n)

#endif /* TEST */


void free(void *ptr);

__macro__ void *calloc(u32int numberOfElements, u32int elementSize);

void *malloc(u32int size);

void *memalign(u32int alignment, u32int size);

void *realloc(void *ptr, u32int size);


/*
 * calloc
 * Allocate and initialize to zero. Parameters are in accordance with the standard, with separate
 * parameters for the number of elements and the element size. Since malloc and memset both act on
 * total size, we need a multiplication here. By using a macro-like function to implement calloc,
 * this multiplication can be optimized away by the compiler in most cases.
 */
__macro__ void *calloc(u32int numberOfElements, u32int elementSize)
{
  u32int size = numberOfElements * elementSize;
  void *ptr = malloc(size);
  if (ptr != NULL)
  {
    memset(ptr, 0, size);
  }
  return ptr;
}


#endif /* __COMMON__STDLIB_H__ */
