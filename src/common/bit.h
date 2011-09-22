#ifndef __COMMON__BIT_H__
#define __COMMON__BIT_H__

#define TEST_BITS_EQUAL(subject, highBit, lowBit)                                                  \
  ((bool)((~(subject >> (__builtin_clz(lowBit) - __builtin_clz(highBit))) ^ subject) & lowBit))

#define TEST_BITS_NOT_EQUAL(subject, highBit, lowBit)                                              \
  ((bool)(((subject >> (__builtin_clz(lowBit) - __builtin_clz(highBit))) ^ subject) & lowBit))

#endif
