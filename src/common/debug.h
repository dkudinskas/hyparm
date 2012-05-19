#ifndef __COMMON__DEBUG_H__
#define __COMMON__DEBUG_H__

#include "common/stdarg.h"
#include "common/stdio.h"
#include "common/types.h"


#define EXPAND_TO_STRING(s)    TO_STRING(s)
#define TO_STRING(s)           #s


#ifdef CONFIG_ASSERT
#define ASSERT(cond, msg)                                                                          \
  {                                                                                                \
    if (unlikely(!(cond)))                                                                         \
    {                                                                                              \
      dieNow2(__FILE__, __LINE__, __func__,                                                        \
              "assertion (" #cond ") failed:" EOL, msg);                                           \
    }                                                                                              \
  }
#else
#define ASSERT(cond, msg)
#endif /* CONFIG_ASSERT */

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

#define DIE_NOW(context, msg)  dieNow(__FILE__, __LINE__, __func__, msg)


extern const char *const ERROR_BAD_ACCESS_SIZE;
extern const char *const ERROR_BAD_ARGUMENTS;
extern const char *const ERROR_NO_SUCH_REGISTER;
extern const char *const ERROR_NOT_IMPLEMENTED;
extern const char *const ERROR_UNPREDICTABLE_INSTRUCTION;


void dieNow(const char *file, u32int line, const char *caller, const char *message)
            __attribute__((noreturn));

void dieNow2(const char *file, u32int line, const char *caller, const char *message1,
             const char *message2) __attribute__((noreturn));

void dieNowF(const char *file, u32int line, const char *caller, const char *format, ...)
             __attribute__((format(__printf__, 4, 5))) __attribute__((noreturn));

void dumpStack(void) __attribute__((naked));

/* output to serial */
u32int printf(const char *fmt, ...) __attribute__((format(__printf__, 1, 2)));


#ifdef CONFIG_MMC

/* output to mmc */
u32int fprintf(const char *fmt, ...) __attribute__((format(__printf__, 1, 2)));

#else

/* fall back to serial */
#define fprintf  printf

#endif /* CONFIG_MMC */

#endif /* __COMMON__DEBUG_H__ */
