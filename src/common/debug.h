#ifndef __COMMON__DEBUG_H__
#define __COMMON__DEBUG_H__

#include "common/stdarg.h"
#include "common/stdio.h"
#include "common/types.h"

#include "guestManager/guestContext.h"


void DIE_NOW(GCONTXT *context, const char *msg)
  __attribute__((noreturn));


/* output to serial */
u32int printf(const char *fmt, ...)
  __attribute__((format(__printf__, 1, 2)));


#ifdef CONFIG_MMC

/* output to mmc */
u32int fprintf(const char *fmt, ...)
  __attribute__((format(__printf__, 1, 2)));

#endif /* CONFIG_MMC */

#endif /* __COMMON__DEBUG_H__ */
