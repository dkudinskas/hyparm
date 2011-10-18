#ifndef __COMMON__DEBUG_H__
#define __COMMON__DEBUG_H__

#include "common/stdarg.h"
#include "common/stdio.h"
#include "common/types.h"

#include "guestManager/guestContext.h"


#define DEBUG(what, ...)                                                                           \
  {                                                                                                \
    if (CONFIG_DEBUG_ ## what)                                                                     \
    {                                                                                              \
      printf(__VA_ARGS__);                                                                         \
    }                                                                                              \
  }

#define DEBUG_MMC(what, ...)                                                                       \
  {                                                                                                \
    if (CONFIG_DEBUG_ ## what)                                                                     \
    {                                                                                              \
      fprintf(__VA_ARGS__);                                                                        \
    }                                                                                              \
  }

#define DIE_NOW(context, msg)  dieNow(context, __func__, msg)


void dieNow(GCONTXT *context, const char *caller, const char *msg)
  __attribute__((noreturn));

/* output to serial */
u32int printf(const char *fmt, ...)
  __attribute__((format(__printf__, 1, 2)));


#ifdef CONFIG_MMC

/* output to mmc */
u32int fprintf(const char *fmt, ...)
  __attribute__((format(__printf__, 1, 2)));

#else

/* fall back to serial */
#define fprintf  printf

#endif /* CONFIG_MMC */

#endif /* __COMMON__DEBUG_H__ */
