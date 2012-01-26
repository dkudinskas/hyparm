#ifndef __COMMON__ASSERT_H__
#define __COMMON__ASSERT_H__

#define COMPILE_TIME_ASSERT( exp, name ) \
  typedef char assertion_failed##name [ (exp ) ? 1 : -1 ];

#endif
