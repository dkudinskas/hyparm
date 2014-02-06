#ifndef __COMMON__TYPES_H__
#define __COMMON__TYPES_H__

#include "common/assert.h"
#include "common/limits.h"


#define TRUE     1
#define FALSE    0

typedef _Bool               bool;

typedef signed char         schar;
typedef unsigned char       uchar;

typedef signed long         slong;
typedef unsigned long       ulong;

typedef signed char         s8int;
typedef signed short        s16int;
typedef signed int          s32int;
typedef signed long long    s64int;

typedef unsigned char       u8int;
typedef unsigned short      u16int;
typedef unsigned int        u32int;
typedef unsigned long long  u64int;

// Signed type of difference of two pointers
typedef __PTRDIFF_TYPE__  ptrdiff_t;

// Unsigned type of `sizeof' something.
typedef __SIZE_TYPE__     size_t;

// Null pointer constant (C only) and type (C++11 only)
#ifndef __cplusplus
#define NULL              ((void *)0)
#else
typedef decltype(nullptr) nullptr_t;
#endif

// Macro to get member offset
#define offsetof(type, member)  __builtin_offsetof(type, member)


typedef struct
{
  u32int val;
  bool carry;
} u32int_c;


typedef enum armShiftType
{
  SHIFT_TYPE_LSL,
  SHIFT_TYPE_LSR,
  SHIFT_TYPE_ASR,
  SHIFT_TYPE_RRX,
  SHIFT_TYPE_ROR,
} ShiftType;


typedef enum loadStoreAccessSize
{
  BYTE,
  HALFWORD,
  WORD,
} ACCESS_SIZE;

COMPILE_TIME_ASSERT(CHAR_BIT == 8, __char_not_8bit);

COMPILE_TIME_ASSERT(sizeof(void *) == 4, __pointers_not_32bit);

COMPILE_TIME_ASSERT(sizeof(s8int)  == 1, __s8int_not_8bit);
COMPILE_TIME_ASSERT(sizeof(s16int) == 2, __s16int_not_16bit);
COMPILE_TIME_ASSERT(sizeof(s32int) == 4, __s32int_not_32bit);
COMPILE_TIME_ASSERT(sizeof(s64int) == 8, __s64int_not_64bit);

COMPILE_TIME_ASSERT(sizeof(u8int)  == 1, __u8int_not_8bit);
COMPILE_TIME_ASSERT(sizeof(u16int) == 2, __u16int_not_16bit);
COMPILE_TIME_ASSERT(sizeof(u32int) == 4, __u32int_not_32bit);
COMPILE_TIME_ASSERT(sizeof(u64int) == 8, __u64int_not_64bit);

COMPILE_TIME_ASSERT(sizeof(void *) == 4, __pointer_not_32bit);

#endif
