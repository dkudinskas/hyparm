#ifndef __COMMON__ASSERT_H__
#define __COMMON__ASSERT_H__

#include "common/compiler.h"


#ifdef COMPILER_HAS_STATIC_ASSERT
#define COMPILE_TIME_ASSERT(exp, name)  _Static_assert(exp, #name)
#else
#define COMPILE_TIME_ASSERT(exp, name)  typedef char assertion_failed##name [ (exp ) ? 1 : -1 ]
#endif

#endif /* COMPILER_HAS_STATIC_ASSERT */
