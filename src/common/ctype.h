#ifndef __COMMON__CTYPE_H__
#define __COMMON__CTYPE_H__ 1

#define ASCII_SOH    0x01
#define ASCII_STX    0x02
#define ASCII_ETX    0x03
#define ASCII_EOT    0x04
#define ASCII_ENQ    0x05
#define ASCII_ACK    0x06
#define ASCII_BEL    0x07
#define ASCII_SO     0x0E
#define ASCII_SI     0x0F
#define ASCII_DLE    0x10
#define ASCII_DC1    0x11
#define ASCII_DC2    0x12
#define ASCII_DC3    0x13
#define ASCII_DC4    0x14
#define ASCII_NAK    0x15
#define ASCII_SYN    0x16
#define ASCII_ETB    0x17
#define ASCII_CAN    0x18
#define ASCII_EM     0x19
#define ASCII_SUB    0x1A
#define ASCII_ESC    0x1B
#define ASCII_FS     0x1C
#define ASCII_GS     0x1D
#define ASCII_RS     0x1E
#define ASCII_US     0x1F
#define ASCII_SPACE  0x20
#define ASCII_DEL    0x7E


int isalnum(int c);

int isalpha(int c);

int iscntrl(int c);

int isdigit(int c);

int isgraph(int c);

int islower(int c);

int isprint(int c);

int ispunct(int c);

int isspace(int c);

int isupper(int c);

int isxdigit(int c);

int tolower(int c);

int toupper(int c);

#endif

