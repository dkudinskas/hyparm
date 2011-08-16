#ifndef __COMMON__DEBUG_H__
#define __COMMON__DEBUG_H__

#include "common/stdarg.h"
#include "common/stdio.h"
#include "common/types.h"

#include "guestManager/guestContext.h"


__attribute((noreturn)) void DIE_NOW(GCONTXT * context, char* msg);


/* output to serial */
u32int printf(const char *fmt, ...);

#ifdef CONFIG_MMC
/* output to mmc */
u32int fprintf(const char *fmt, ...);
#endif


#endif
