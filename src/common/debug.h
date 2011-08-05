#ifndef __COMMON__DEBUG_H__
#define __COMMON__DEBUG_H__

#include "common/types.h"

#include "guestManager/guestContext.h"


typedef char *va_list;

#define  ALIGNBND           (sizeof (signed int) - 1)
#define bnd(X, bnd)         (((sizeof (X)) + (bnd)) & (~(bnd)))
#define va_arg(ap, T)       (*(T *)(((ap) += (bnd (T, ALIGNBND))) - (bnd (T,ALIGNBND))))
#define va_end(ap)          (void) 0
#define va_start(ap, A)     (void) ((ap) = (((char *) &(A)) + (bnd (A,ALIGNBND))))


__attribute((noreturn)) void DIE_NOW(GCONTXT * context, char* msg);


/* output to serial */
u32int printf(const char *fmt, ...);

#ifdef CONFIG_MMC
/* output to mmc */
u32int fprintf(const char *fmt, ...);
#endif

u32int vsprintf(char *buf, const char *fmt, va_list args);

#endif
