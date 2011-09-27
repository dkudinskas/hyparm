#ifndef __COMMON__BIT_H__
#define __COMMON__BIT_H__

#include "common/types.h"


#define TEST_BITS_EQUAL(subject, highBit, lowBit)                                                  \
  ((bool)((~(subject >> (__builtin_clz(lowBit) - __builtin_clz(highBit))) ^ subject) & lowBit))

#define TEST_BITS_NOT_EQUAL(subject, highBit, lowBit)                                              \
  ((bool)(((subject >> (__builtin_clz(lowBit) - __builtin_clz(highBit))) ^ subject) & lowBit))


/*
 * countBitsSet
 * Counting bits set, Brian Kernighan's way
 *
 * Source: "Bit Twiddling Hacks" by Sean Eron Anderson
 * License: public domain.
 */
extern inline __attribute__((always_inline,gnu_inline)) u32int countBitsSet(u32int word)
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

#endif
