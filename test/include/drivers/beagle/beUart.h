#ifndef __DRIVERS__BEAGLE__BE_UART_H__
#define __DRIVERS__BEAGLE__BE_UART_H__

#include <stdio.h>

char serialGetc();
void serialPuts(char *c);
void serialPutc(char c);

#endif
