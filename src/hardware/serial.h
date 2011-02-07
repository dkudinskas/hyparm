#ifndef __SERIAL_H__
#define __SERIAL_H__

#include "types.h"

#ifdef TARGET_BEAGLE
#define SERIAL_BASE            0x49020000
#elif TARGET_TEGRA250
#define SERIAL_BASE            0x70006300
#else
#error unknown target
#endif

#define TRANSMIT_HOLDING_REG_OFFSET 0x00000000
#define LINE_STATUS_REG_OFFSET 0x00000014

#define LINE_STATUS_REG_TX_HOLD     0x20

#define LINE_FEED              0xA
#define CARRIAGE_RETURN        0xD
#define PRINTABLE_START        0x20
#define PRINTABLE_END          0x7E

void serial_putchar(char c);
void serial_putstring(char * c);
void serial_newline(void);
void serial_putlong(ulong nr);
void serial_putint(u32int nr);
void serial_putint_nozeros(u32int nr);
void serial_putbyte(u8int nr);
int printableChar(char c);

#endif
