#include "common/bit.h"
#include "common/compiler.h"
#include "common/debug.h"
#include "common/stddef.h"

/*
 * USAGE NOTES
 *
 * The routines in this file are NOT to be called directly; instead they are here to allow the use
 * of the / and % operators cleanly in the rest of the codebase.
 */


/*
 * Vector types. These are by convention returned in r0-r3. Subscripting vectors is not supported
 * by every version of GCC, hence we need to check. Do not alter the following definitions.
 */

#ifdef COMPILER_HAS_GCC_VECTOR_TYPES
typedef u32int u32intPair __attribute__((vector_size(8)));
#else
typedef u64int u32intPair;
#endif
typedef u64int u64intPair __attribute__((vector_size(16)));


/*
 * ARM EABI-defined division routines.
 * Do not alter the signature of the following functions.
 */

u32int __aeabi_uidiv(u32int dividend, u32int divisor) __attribute__((externally_visible));
u32intPair __aeabi_uidivmod(u32int dividend, u32int divisor) __attribute__((externally_visible));
u64intPair __aeabi_uldivmod(u64int dividend, u64int divisor) __attribute__((externally_visible));


/*
 * Helper functions.
 */

static u32int uidiv_recursive(u32int dividend, u32int divisor, u32int acc);
static u64int uldiv_recursive(u64int dividend, u64int divisor, u64int acc);


/*
 * Helper macros.
 */

#define UDIV(integralType, vectorType, resultDef, log2Func, recursiveFunc, returnExpr)             \
  {                                                                                                \
    vectorType resultDef;                                                                          \
    if (divisor == (integralType)0)                                                                \
    {                                                                                              \
      DIE_NOW(NULL, "Division by zero");                                                              \
    }                                                                                              \
    if (dividend < divisor)                                                                        \
    {                                                                                              \
      result[0] = 0;                                                                               \
      result[1] = dividend;                                                                        \
      return returnExpr;                                                                           \
    }                                                                                              \
    /*                                                                                             \
     * Check if divisor is a power of 2; if so, use bit shift instead of division. This is only    \
     * useful in cases where the compiler cannot optimize the division away upfront.               \
     */                                                                                            \
    if (!(divisor & (divisor - 1)))                                                                \
    {                                                                                              \
      s32int powerOf2 = log2Func(divisor);                                                         \
      result[0] = dividend >> powerOf2;                                                            \
      result[1] = dividend - (result[0] << powerOf2);                                              \
      return returnExpr;                                                                           \
    }                                                                                              \
    /*                                                                                             \
     * Divide recursively                                                                          \
     */                                                                                            \
    result[0] = recursiveFunc(dividend, divisor, 0);                                               \
    result[1] = dividend - result[0] * divisor;                                                    \
    return returnExpr;                                                                             \
  }

#define UDIV_RECURSIVE(integralType, self)                                                         \
  {                                                                                                \
    integralType res, tmp;                                                                         \
    res = (integralType)1;                                                                         \
    tmp = divisor;                                                                                 \
    while (dividend > tmp)                                                                         \
    {                                                                                              \
      /*                                                                                           \
       * We need to be careful here that we don't shift past the last bit.                         \
       */                                                                                          \
      if (tmp & ((integralType)1 << ((sizeof(integralType) << 3) - 1)))                            \
      {                                                                                            \
        break;                                                                                     \
      }                                                                                            \
      tmp <<= 1;                                                                                   \
      if (tmp > dividend)                                                                          \
      {                                                                                            \
        /*                                                                                         \
         * Shift it back for later                                                                 \
         */                                                                                        \
        tmp >>= 1;                                                                                 \
        break;                                                                                     \
      }                                                                                            \
      res <<= 1;                                                                                   \
    }                                                                                              \
    dividend -= tmp;                                                                               \
    acc += res;                                                                                    \
    return dividend < divisor ? acc : self(dividend, divisor, acc);                                \
  }


/*
 * 32-bit unsigned integer division. The quotient is returned in r0.
 */
u32int __aeabi_uidiv(u32int dividend, u32int divisor)
{
#ifdef COMPILER_HAS_GCC_VECTOR_TYPES
  UDIV(u32int, u32intPair, result, countTrailingZeros, uidiv_recursive, result[0]);
#else
  UDIV(u32int, u32int, result[2], countTrailingZeros, uidiv_recursive, result[0]);
#endif
}

/*
 * 32-bit unsigned integer division with remainder. The quotient is returned in r0; the remainder
 * is returned in r1.
 *
 * In the EABI reference documentation, a structure similar to uidiv_t is used as return type.
 * However, we cannot make GCC return such structure in r0 and r1 -- generally, structs are
 * returned on the stack. There is a compiler flag to allow returning structs in registers, but
 * that is only a hint towards the optimizer. In order to force returning quotient and remainder
 * in r0 and r1, they are packed in a double word (u64int).
 *
 * WARNING: This code depends on the endianness of data in memory.
 */
u32intPair __aeabi_uidivmod(u32int dividend, u32int divisor)
{
#ifdef COMPILER_HAS_GCC_VECTOR_TYPES
  UDIV(u32int, u32intPair, result, countTrailingZeros, uidiv_recursive, result);
#else
  UDIV(u32int, u32int, result[2], countTrailingZeros, uidiv_recursive, *(u32intPair *)&result);
#endif
}

/*
 * 64-bit unsigned integer division with remainder. The quotient is returned in r0 and r1; the
 * remainder is returned in r2 and r3.
 */
u64intPair __aeabi_uldivmod(u64int dividend, u64int divisor)
{
#ifdef COMPILER_HAS_GCC_VECTOR_TYPES
  UDIV(u64int, u64intPair, result, countTrailingZeros64, uldiv_recursive, result);
#else
# pragma message "*** WARNING *** __aeabi_uldivmod has no implementation for this compiler!"
  DIE_NOW(NULL, ERROR_NOT_IMPLEMENTED);
#endif
}

/*
 * 32-bit recursive unsigned integer division, to be invoked from __aeabi_uidiv{mod} ONLY.
 */
static u32int uidiv_recursive(u32int dividend, u32int divisor, u32int acc)
{
  UDIV_RECURSIVE(u32int, uidiv_recursive);
}

/*
 * 64-bit recursive unsigned integer division, to be invoked from __aeabi_uldivmod ONLY.
 */
static u64int uldiv_recursive(u64int dividend, u64int divisor, u64int acc)
{
  UDIV_RECURSIVE(u64int, uldiv_recursive);
}

