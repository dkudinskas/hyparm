#include "common/alignFunctions.h"
#include "common/types.h"

/* Loads a word from a buffer, allowing unaligned accesses.
   WARNING: Assumes data is little-endian in memory */
u32int uaLoadWord(char bytes[])
{
  return (u32int)bytes[0] |
          ((u32int)bytes[1] << 8) |
          ((u32int)bytes[2] << 16) |
          ((u32int)bytes[3] << 24);
}

u16int uaLoadHWord(char bytes[])
{
  return (u16int)bytes[0] | (u16int)bytes[1] << 8;
}

u32int uaLoadWordNoSwp(char bytes[])
{
  return (u32int)bytes[0] << 24 |
         (u32int)bytes[1] << 16 |
         (u32int)bytes[2] << 8 |
         (u32int)bytes[3];
}

