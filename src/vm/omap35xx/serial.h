#ifndef __HARDWARE__SERIAL_H__
#define __HARDWARE__SERIAL_H__

#include "common/types.h"


#ifdef TARGET_BEAGLE
#define SERIAL_BASE            0x49020000
#elif TARGET_TEGRA250
#define SERIAL_BASE            0x70006300
#else
#error unknown target
#endif

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
