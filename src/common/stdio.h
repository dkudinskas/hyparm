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


int
#ifdef TEST
  test_sscanf
#else
  sscanf
#endif
  (const char *s, const char *format, ...);

int
#ifdef TEST
  test_vsscanf
#else
  vsscanf
#endif
  (const char *str, const char *format, va_list args);


#endif
