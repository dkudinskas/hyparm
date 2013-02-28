#ifndef __COMMON__STDIO_H__
#define __COMMON__STDIO_H__

/*
 * USAGE NOTES
 *
 * The functions declared in this header try to mimic ANSI C behavior as much as possible. Hence,
 * take the following note into account when using them:
 *
 * The 'l' modifier is used to print of scan data of type 'long int'. On ARM, both 'int' and 'long'
 * are 32-bit in size. 64-bit numbers on ARM must be represented using 'long long int'. The standard
 * does not deal with these numbers. The solution adopted here is the same as found in contemporary
 * C libraries. In order to print or scan data of type 'long long int', the 'L' modifier must be
 * used.
 *
 * This approach makes it possible for GCC to verify that the arguments we pass to our own printf,
 * scanf, and similar functions are of the right type according to the format string. This checking
 * is enabled by setting the 'format' attribute on each of these functions. Type checking will only
 * occur on the '...' parameter.
 */


#ifdef TEST
#include <stdarg.h>
#else
#include "common/stdarg.h"
#endif

#include "common/types.h"


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
  (const char *s, const char *format, ...)
  __attribute__((format(__scanf__, 2, 3), warn_unused_result));

int
#ifdef TEST
  test_vsscanf
#else
  vsscanf
#endif
  (const char *s, const char *format, va_list args) __flatten__
  __attribute__((format(__scanf__, 2, 0), warn_unused_result));

int
#ifdef TEST
  test_vsprintf
#else
  vsprintf
#endif
  (char *buf, const char *fmt, va_list args)
  __attribute__((format(__printf__, 2, 0)));

int
#ifdef TEST
  test_vsnprintf
#else
  vsnprintf
#endif
  (char *buf, u32int count, const char *fmt, va_list args)
  __attribute__((format(__printf__, 3, 0)));

#endif
