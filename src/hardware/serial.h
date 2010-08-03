#ifndef __SERIAL_H__
#define __SERIAL_H__

#include "types.h"

#define TRANSMIT_HOLDING_REG   0x49020000

#define LINE_STATUS_REG        0x49020014
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

void serial_ERROR(char * msg) __attribute((noreturn));

#endif
