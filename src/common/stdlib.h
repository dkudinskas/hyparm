#ifndef __COMMON__STDLIB_H__
#define __COMMON__STDLIB_H__

#include "common/types.h"


#ifdef TEST

#include <stdlib.h>

#else

#define abs(n)    __builtin_abs(n)

#define llabs(n)  __builtin_llabs(n)

#endif /* TEST */


#endif /* __COMMON__STDLIB_H__ */
