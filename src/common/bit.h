#ifndef __COMMON__BIT_H__
#define __COMMON__BIT_H__

#include "common/compiler.h"
#include "common/types.h"


__macro__ u32int countBitsSet(u32int word)  __constant__;
__macro__ s32int countLeadingZeros(u32int x);
__macro__ s32int countLeadingZeros64(u64int x);
__macro__ s32int countTrailingZeros(u32int x);
__macro__ s32int countTrailingZeros64(u64int x);
__macro__ u32int findFirstBitSet(u32int x);
__macro__ u32int findLastBitSet(u32int x);
__macro__ bool isAlignedToBits(void *pointer, u32int bits);
__macro__ bool isAlignedToBitsN(u32int value, u32int bits);
__macro__ bool isAlignedToMask(void *pointer, u32int mask);
__macro__ bool isAlignedToMaskN(u32int value, u32int mask);
__macro__ u32int maskedBitShift(u32int value, u32int mask);
__macro__ u32int signExtend(u32int value, u32int bits);
__macro__ bool testBitsEqual(u32int subject, u32int highBit, u32int lowBit);
__macro__ bool testBitsNotEqual(u32int subject, u32int highBit, u32int lowBit);


/*
 * countBitsSet
 * Counting bits set, Brian Kernighan's way
 *
 * Source: "Bit Twiddling Hacks" by Sean Eron Anderson
 * License: public domain.
 */
__macro__ u32int countBitsSet(u32int word)
{
  u32int count;
  for (count = 0; word; count++)
  {
    /*
     * Clear the least significant bit set
     */
    word &= word - 1;
  }
  return count;
}

/*
 * countLeadingZeros
 * Returns the number of leading 0-bits in the 32-bit unsigned integer x, starting at the most
 * significant bit position. If x is 0, the result is undefined.
 */
__macro__ s32int countLeadingZeros(u32int x)
{
  return __builtin_clz(x);
}

/*
 * countLeadingZeros64
 * Returns the number of leading 0-bits in the 64-bit unsigned integer x, starting at the most
 * significant bit position. If x is 0, the result is undefined.
 */
__macro__ s32int countLeadingZeros64(u64int x)
{
  return __builtin_clzll(x);
}

/*
 * countTrailingZeros
 * Returns the number of trailing 0-bits in the 32-bit unsigned integer x, starting at the least
 * significant bit position. If x is 0, the result is undefined.
 */
__macro__ s32int countTrailingZeros(u32int x)
{
  return __builtin_ctz(x);
}

/*
 * countTrailingZeros64
 * Returns the number of trailing 0-bits in the 64-bit unsigned integer x, starting at the least
 * significant bit position. If x is 0, the result is undefined.
 */
__macro__ s32int countTrailingZeros64(u64int x)
{
  u32int loWord = x & 0xFFFFFFFF;
  if (loWord)
  {
    return __builtin_ctz(loWord);
  }
  return __builtin_ctz((u32int)(x >> 32)) + 32;
}

/*
 * findFirstBitSet
 * Returns one plus the index of the least-significant 1-bit.
 */
__macro__ u32int findFirstBitSet(u32int x)
{
  return __builtin_ffs(x);
}

/*
 * findLastBitSet
 * Returns one plus the index of the most significant 1-bit.
 */
__macro__ u32int findLastBitSet(u32int x)
{
  return x ? (sizeof(u32int) << 3) - countLeadingZeros(x) : 0;
}

/*
 * isAlignedToBits
 * Returns whether the last n < 32 bits of the specified pointer are zero.
 */
__macro__ bool isAlignedToBits(void *pointer, u32int bits)
{
  return isAlignedToBitsN((u32int)pointer, bits);
}

__macro__ bool isAlignedToBitsN(u32int value, u32int bits)
{
  return findFirstBitSet(value) > bits;
  // Alternative:
  // return (((u32int)pointer) & ((1 << bits) - 1)) == 0;
}

/*
 * isAlignedToMask
 * Returns whether the specified pointer has the same alignment as the specified mask.
 */
__macro__ bool isAlignedToMask(void *pointer, u32int mask)
{
  return isAlignedToMaskN((u32int)pointer, mask);
}

__macro__ bool isAlignedToMaskN(u32int value, u32int mask)
{
  return (value & ~mask) == 0;
}

/*
 * maskedBitShift
 * Shifts the given value left as many positions as there are trailing zeros in the given mask, and
 * then applies the mask to the shifted value. If mask is 0, the result is undefined.
 */
__macro__ u32int maskedBitShift(u32int value, u32int mask)
{
  return (value << countTrailingZeros(mask)) & mask;
}

/*
 * signExtend
 * Converts an n-bit signed value (0 < n < 32) into a 32-bit signed value through sign extension.
 */
__macro__ u32int signExtend(u32int value, u32int bits)
{
  /*
   * An n-bit signed value (n = bits), passed as u32int, is extended to a 32-bit signed number by
   * performing shift operations. First, shift the value to the left for (32 - bits) bits to make
   * sure all non-significant are cleared and the sign bit is the MSB. Next, perform an arithmetic
   * shift to the right to extend the sign bit and to restore the original position.
   */
  bits = 32 - bits;
  value <<= bits;
  *(s32int *)&value >>= bits;
  return value;
}

/*
 * testBitsEqual
 * Tests whether two bits in the same value are both on or both off.
 */
__macro__ bool testBitsEqual(u32int subject, u32int highBit, u32int lowBit)
{
  return (bool)((~(subject >> (countLeadingZeros(lowBit) - countLeadingZeros(highBit))) ^ subject) & lowBit);
}

/*
 * testBitsNotEqual
 * Tests whether two bits in the same value are not both set and not both unset.
 */
__macro__ bool testBitsNotEqual(u32int subject, u32int highBit, u32int lowBit)
{
  return (bool)(((subject >> (countLeadingZeros(lowBit) - countLeadingZeros(highBit))) ^ subject) & lowBit);
}

#endif
