#include "vm/omap35xx/led.h"


int turnOff()
{
  int y = 0;
  while (y < 100000)
  {
    y++;
    if (y == 99999)
    {
      *(volatile u32int *)(DATA_SET_REG) = LED1 | LED2;
    }
  }
  return y;
}

int turnOn()
{
  int y = 0;
  while (y < 100000)
  {
    y++;
    if (y == 99999)
    {
      *(volatile u32int *)(DATA_CLEAR_REG) = LED1 | LED2;
    }
  }
  return y;
}
