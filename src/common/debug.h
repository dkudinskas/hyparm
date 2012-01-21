#ifndef __COMMON__DEBUG_H__
#define __COMMON__DEBUG_H__

#include "common/types.h"

#include "guestManager/guestContext.h"

#ifndef va_arg

typedef int s32; //assume int is 2 Bytes
typedef s32 acpi_native_int;

#ifndef _VALIST
#define _VALIST
typedef char *va_list;
#endif                          /* _VALIST */

/*
 * Storage alignment properties
 */
#define  _AUPBND                (sizeof (acpi_native_int) - 1)
#define  _ADNBND                (sizeof (acpi_native_int) - 1)

/*
 * Variable argument list macro definitions
 */
#define _bnd(X, bnd)            (((sizeof (X)) + (bnd)) & (~(bnd)))
#define va_arg(ap, T)           (*(T *)(((ap) += (_bnd (T, _AUPBND))) - (_bnd (T,_ADNBND))))
#define va_end(ap)              (void) 0
#define va_start(ap, A)         (void) ((ap) = (((char *) &(A)) + (_bnd (A,_AUPBND))))

#endif                          /* va_arg */


//#define DIE_NOW_SCANNER_COUNTER

__attribute((noreturn)) void DIE_NOW(GCONTXT * context, char* msg);

#define LINE_FEED              0xA
#define CARRIAGE_RETURN        0xD
#define PRINTABLE_START        0x20
#define PRINTABLE_END          0x7E

u32int printf(const char *fmt, ...);
u32int vsprintf(char *buf, const char *fmt, va_list args);

void DEBUG_CHAR(char c);
void DEBUG_STRING(char * c);
void DEBUG_NEWLINE(void);
void DEBUG_INT(u32int nr);
void DEBUG_INT_NOZEROS(u32int nr);
void DEBUG_BYTE(u8int nr);
int printableChar(char c);

#endif
