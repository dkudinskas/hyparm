#ifndef __COMMON__CTYPE_H__
#define __COMMON__CTYPE_H__ 1

#include "common/types.h"


#define ASCII(c)               ((s32int)(uchar)(c))


#define ASCII_NUL              0x00
#define ASCII_SOH              0x01
#define ASCII_STX              0x02
#define ASCII_ETX              0x03
#define ASCII_EOT              0x04
#define ASCII_ENQ              0x05
#define ASCII_ACK              0x06
#define ASCII_BEL              0x07
#define ASCII_SO               0x0E
#define ASCII_SI               0x0F
#define ASCII_DLE              0x10
#define ASCII_DC1              0x11
#define ASCII_DC2              0x12
#define ASCII_DC3              0x13
#define ASCII_DC4              0x14
#define ASCII_NAK              0x15
#define ASCII_SYN              0x16
#define ASCII_ETB              0x17
#define ASCII_CAN              0x18
#define ASCII_EM               0x19
#define ASCII_SUB              0x1A
#define ASCII_ESC              0x1B
#define ASCII_FS               0x1C
#define ASCII_GS               0x1D
#define ASCII_RS               0x1E
#define ASCII_US               0x1F
#define ASCII_SPACE            0x20
#define ASCII_DEL              0x7E


#define ASCII_CONTROL_BEGIN    ASCII_NUL
#define ASCII_CONTROL_END      ASCII_US
#define ASCII_PRINTABLE_BEGIN  ASCII_SPACE
#define ASCII_PRINTABLE_END    ASCII('~')


s32int isalnum(s32int c);

s32int isalpha(s32int c);

s32int iscntrl(s32int c);

s32int isdigit(s32int c);

s32int isgraph(s32int c);

s32int islower(s32int c);

s32int isprint(s32int c);

s32int ispunct(s32int c);

s32int isspace(s32int c);

s32int isupper(s32int c);

s32int isxdigit(s32int c);

s32int tolower(s32int c);

s32int toupper(s32int c);

#endif /* __COMMON__CTYPE_H__ */
