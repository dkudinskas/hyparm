#include "debug.h"

/* Support functions for division routines */

/* Check if a uint is a power of two.
 *    If so, return the power value.
 *       If not a power, returns -1 */
static int __ui_po2(unsigned int n)
{
  return (n && !(n & (n - 1)));
}

/* Assuming n is a power of 2, find the power */
static int __ui_get_po2(unsigned int n)
{
  int i;
  for (i = 0; i < sizeof(unsigned int)*8; i++)
  {
    if (n & 0x1)
      return i;
    else
      n = n >> 1;
  }

  //if we get here, then n was 0. return error
  return -1;
}

/* End support functions */


unsigned int __aeabi_uidiv_recursive(unsigned int a, unsigned int b, unsigned int acc);

/* ARMEABI defined division routines */

/* Simple unsigned int division with minimal optimizations.
 * For best results, use a tail-call optimizing seasoning.*/
unsigned int __aeabi_uidiv(unsigned int a, unsigned int b)
{
  if (b == 0)
  {
    DIE_NOW(0, "Divide by 0!");
  }
  
  if (a < b)
    return 0;
  
  /* check if divisor is a power of 2 */
  if (__ui_po2(b))
  {
    return a >>  __ui_get_po2(b);
  }
  
  return __aeabi_uidiv_recursive(a, b, 0);
}

/* Recursive case for uidiv */
unsigned int __aeabi_uidiv_recursive(unsigned int a, unsigned int b, unsigned int acc)
{
  if (a < b)
    return acc;

  unsigned int res = 1, tmp = b;
  
  while (a > tmp)
  {
    /* we need to be careful here that we don't shift past the last bit */
    if (tmp & (1 << (sizeof(unsigned int)*8-1)))
      break;

    tmp = tmp << 1;
    if (tmp > a)
    {
      tmp = tmp >> 1; //shift it back for later
      break;
    }
    res *= 2;
  }

  return __aeabi_uidiv_recursive(a - tmp, b, acc + res);
}
