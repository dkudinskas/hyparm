#include "common/string.h"


/*
 * ARM EABI-defined routines.
 * Do not alter the signature of the following functions.
 */
void __aeabi_memcpy(void *destination, const void *source, u32int count);


void __aeabi_memcpy(void *destination, const void *source, u32int count)
{
  memcpy(destination, source, count);
}
