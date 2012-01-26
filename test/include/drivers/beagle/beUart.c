#ifndef __DRIVERS__BEAGLE__BE_UART_H__
#define __DRIVERS__BEAGLE__BE_UART_H__

#include <stdio.h>

char serialGetc()
{
  return '\0';
}

void serialPuts(char *c)
{
  while (*c)
  {
    putchar(*c++);
  }
}

void serialPutc(char c)
{
  putchar(c);
}

#endif
