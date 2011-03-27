#include "common.h"
#include "types.h"

u32int bs32(u32int number)
{
  u32int retVal = 0;
  retVal |= ((number & 0xFF000000) >> 24);
  retVal |= ((number & 0x00FF0000) >>  8);
  retVal |= ((number & 0x0000FF00) <<  8);
  retVal |= ((number & 0x000000FF) << 24);
  return retVal;
}

// http://www.concentric.net/~Ttwang/tech/inthash.htm
// 32bit mix function
u32int getHash(u32int key)
{
  key = ~key + (key << 15); // key = (key << 15) - key - 1;
  key = key ^ (key >> 12);
  key = key + (key << 2);
  key = key ^ (key >> 4);
  key = key * 2057; // key = (key + (key << 3)) + (key << 11);
  key = key ^ (key >> 16);
  return key >> 2;
}

void delay(u32int count)
{
  volatile u32int i = 0;
  
  while (i < count)
  {
    i++;
  }
}
