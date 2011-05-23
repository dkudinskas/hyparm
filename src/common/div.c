#include "common/debug.h"
#include "common/types.h"

/* These division/modulus routines are NOT to be called directly, instead
   they are here to allow the use of the / and % C operators cleanly in the 
   rest of the codebase. */

/* Support functions for division routines */

/* Check if a uint is a power of two.
 * If so, return the power value.
 * If not a power, returns -1 */
static int isPowerOf2(u32int n)
{
  return (n && !(n & (n - 1)));
}

/* Assuming n is a power of 2, find the power */
static int getPowerOf2(u32int n)
{
  int i;
  for (i = 0; i < sizeof(u32int)*8; i++)
  {
    if (n & 0x1)
    {
      return i;
    }
    else
    {
      n = n >> 1;
    }
  }

  //if we get here, then n was 0. return error
  return -1;
}

/* End support functions */

static u32int __aeabi_uidiv_recursive(u32int dividend, u32int divisor, u32int acc);
static u64int __aeabi_uldivmod_recursive(u64int dividend, u64int divisor, u64int acc);

/* ARMEABI defined division routines */

/* Simple unsigned int division with minimal optimizations.
 * For best results, use a tail-call optimizing seasoning.*/
u32int __aeabi_uidiv(u32int dividend, u32int divisor)
{
  if (divisor == 0)
  {
    DIE_NOW(0, "Divide by 0!");
  }
  
  if (dividend < divisor)
  {
    return 0;
  }
  
  /* check if divisor is a power of 2 */
  if (isPowerOf2(divisor))
  {
    return dividend >> getPowerOf2(divisor);
  }
  return __aeabi_uidiv_recursive(dividend, divisor, 0);
}

/* Recursive case for uidiv */
static u32int __aeabi_uidiv_recursive(u32int dividend, u32int divisor, u32int acc)
{
  if (dividend < divisor)
  {
    return acc;
  }

  u32int res = 1, tmp = divisor;
  
  while (dividend > tmp)
  {
    /* we need to be careful here that we don't shift past the last bit */
    if (tmp & (1 << (sizeof(u32int)*8-1)))
    {
      break;
    }

    tmp = tmp << 1;
    if (tmp > dividend)
    {
      tmp = tmp >> 1; //shift it back for later
      break;
    }
    res *= 2;
  }

  return __aeabi_uidiv_recursive(dividend - tmp, divisor, acc + res);
}

/* The layout of this structure must not be altered. 
   The return type and member order is defined by
   the ARM eabi. Quotient is returned in r0-r1 and remainder in r2-r3. */
typedef struct
{
  u64int quot;
  u64int rem;
} ulldiv_t;

/* Does 64 bit unsigned int division, returning a struct
   containing the quotient and remainder */
ulldiv_t __aeabi_uldivmod(u64int dividend, u64int divisor)
{
  ulldiv_t ret;

  if (divisor == 0)
  {
    DIE_NOW(0, "Divide by 0!");
  }

  if (dividend < divisor)
  {
    ret.quot = 0;
    ret.rem = dividend;
    return ret;
  }

  /* check if divisor is a power of 2 */
  if (isPowerOf2(divisor))
  {
    // this func call just returns a number of bits, no need for ull
    ret.quot = dividend >> getPowerOf2(divisor);
    ret.rem = dividend - ret.quot * divisor;
    return ret;
  }

  ret.quot = __aeabi_uldivmod_recursive(dividend, divisor, 0);
  ret.rem = dividend - ret.quot * divisor;
  return ret;
}

/* recursive case returning just the quotient */
static u64int __aeabi_uldivmod_recursive(u64int dividend, u64int divisor, u64int acc)
{
  if (dividend < divisor)
  {
    return acc;
  }

  u64int res = 1, tmp = divisor;

  while (dividend > tmp)
  {
    /* we need to be careful here that we don't shift past the last bit */
    if (tmp & (1ULL << (sizeof(u64int)*8-1)))
    {
      break;
    }

    tmp = tmp << 1;
    if (tmp > dividend)
    {
      tmp = tmp >> 1; //shift it back for later
      break;
    }
    res *= 2;
  }

  return __aeabi_uldivmod_recursive(dividend - tmp, divisor, acc + res);
}

