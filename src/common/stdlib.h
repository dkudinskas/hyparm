#ifndef __COMMON__STDLIB_H__
#define __COMMON__STDLIB_H__

#include "common/types.h"


#ifdef TEST

#include <stdlib.h>

#else

#define abs(n)    __builtin_abs(n)

#define llabs(n)  __builtin_llabs(n)

#endif /* TEST */


void free(void *ptr);

void *malloc(u32int size);

#define mallocBytes(size)  malloc(size)

void *memalign(u32int alignment, u32int size);

void *realloc(void *ptr, u32int size);

#endif /* __COMMON__STDLIB_H__ */
