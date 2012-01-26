#ifndef __COMMON__STDIO_H__
#define __COMMON__STDIO_H__ 1

#ifdef TEST
#include <stdarg.h>
#else
#include "common/stdarg.h"
#endif


#ifndef EOF
#define EOF  -1
#endif


#define EOL  "\r\n"

#ifndef TEST

int getchar(void);

int putchar(int c);

#endif

int
#ifdef TEST
  test_sscanf
#else
  sscanf
#endif
  (const char *s, const char *format, ...) __attribute__((warn_unused_result));

int
#ifdef TEST
  test_vsscanf
#else
  vsscanf
#endif
  (const char *str, const char *format, va_list args) __attribute__((warn_unused_result));

int
#ifdef TEST
  test_vsprintf
#else
  vsprintf
#endif
  (char *buf, const char *fmt, va_list args);

#endif
