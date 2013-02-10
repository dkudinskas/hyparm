#include "common/debug.h"
#include "common/stddef.h"
#include "common/stdlib.h"
#include "common/string.h"

#include "common/helpers.h"


#define POLY 0x8408


u32int min (u32int a, u32int b)
{
  return (a < b) ? a : b;
}


u16int crc16(u8int* dataPtr, u16int length)
{
  u8int i;
  u32int data;
  u32int crc = 0xffff;
  
  if (length == 0)
  {
    return (~crc);
  }
  
  do
  {
    for (i=0, data=(u32int)0xff & *dataPtr++; i < 8; i++, data >>= 1)
    {
      if ((crc & 0x0001) ^ (data & 0x0001))
      {
        crc = (crc >> 1) ^ POLY;
      }
      else
      {
        crc >>= 1;
      }
    }
  } while (--length);
  
  crc = ~crc;
  data = crc;
  crc = (crc << 8) | (data >> 8 & 0xff);
  
  return (crc);
}
