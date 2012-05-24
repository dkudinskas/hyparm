#ifndef __COMMON__STDLIB_H__
#define __COMMON__STDLIB_H__

#include "common/compiler.h"
#include "common/debug.h"
#include "common/stddef.h"
#include "common/string.h"
#include "common/types.h"


#ifdef TEST

#include <stdlib.h>

#else

#define abs(n)    __builtin_abs(n)

#define llabs(n)  __builtin_llabs(n)

#endif /* TEST */


#ifndef CONFIG_MEMORY_ALLOCATOR_BOOKKEEPING

#define calloc(numberOfElements, elementSize)  uncheckedCalloc(numberOfElements, elementSize)
#define free(ptr)                              uncheckedFree(ptr)
#define malloc(size)                           uncheckedMalloc(size)
#define memalign(alignment, size)              uncheckedMemalign(alignment, size)
#define realloc(ptr, size)                     uncheckedRealloc(ptr, size)

#else

__macro__ void *checkedCalloc(const char *file, const char *line, const char *function,
                              u32int numberOfElements, u32int elementSize)  __malloc__;
void checkedFree(const char *file, const char *line, const char *function, void *ptr);
void *checkedMalloc(const char *file, const char *line, const char *function, u32int size)
                   __malloc__;
void *checkedMemalign(const char *file, const char *line, const char *function, u32int alignment,
                      u32int size) __malloc__;
void *checkedRealloc(const char *file, const char *line, const char *function, void *ptr,
                     u32int size);

#define calloc(numberOfElements, elementSize)                                                     \
  checkedCalloc(__FILE__, EXPAND_TO_STRING(__LINE__), __func__, numberOfElements, elementSize)
#define free(ptr)                                                                                 \
  checkedFree(__FILE__, EXPAND_TO_STRING(__LINE__), __func__, ptr)
#define malloc(size)                                                                              \
  checkedMalloc(__FILE__, EXPAND_TO_STRING(__LINE__), __func__, size)
#define memalign(alignment, size)                                                                 \
  checkedMemalign(__FILE__, EXPAND_TO_STRING(__LINE__), __func__, alignment, size)
#define realloc(ptr, size)                                                                        \
  checkedRealloc(__FILE__, EXPAND_TO_STRING(__LINE__), __func__, ptr, size)

#endif /* !CONFIG_MEMORY_ALLOCATOR_BOOKKEEPING */


/*
 * WARNING: do NOT invoke the following functions directly!
 */
__macro__ void *uncheckedCalloc(u32int numberOfElements, u32int elementSize) __malloc__;
void uncheckedFree(void *ptr);
void *uncheckedMalloc(u32int size) __malloc__;
void *uncheckedMemalign(u32int alignment, u32int size) __malloc__;
void *uncheckedRealloc(void *ptr, u32int size);


/*
 * uncheckedCalloc
 * Allocate and initialize to zero. Parameters are in accordance with the standard, with separate
 * parameters for the number of elements and the element size. Since malloc and memset both act on
 * total size, we need a multiplication here. By using a macro-like function to implement calloc,
 * this multiplication can be optimized away by the compiler in most cases.
 */
__macro__ void *uncheckedCalloc(u32int numberOfElements, u32int elementSize)
{
  u32int size = numberOfElements * elementSize;
  void *ptr = malloc(size);
  if (ptr != NULL)
  {
    memset(ptr, 0, size);
  }
  return ptr;
}


#ifdef CONFIG_MEMORY_ALLOCATOR_BOOKKEEPING

__macro__ void *checkedCalloc(const char *file, const char *line, const char *function,
                             u32int numberOfElements, u32int elementSize)
{
  u32int size = numberOfElements * elementSize;
  void *ptr = checkedMalloc(file, line, function, size);
  if (ptr != NULL)
  {
    memset(ptr, 0, size);
  }
  return ptr;
}

#endif /* CONFIG_MEMORY_ALLOCATOR_BOOKKEEPING */

#endif /* __COMMON__STDLIB_H__ */
