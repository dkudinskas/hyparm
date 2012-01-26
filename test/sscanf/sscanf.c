#include <stdio.h>

#include "common/stdio.h"

void test(const char *s, const char *format, ...)
{
  int ourResult, libCResult;
  {
    va_list args;
    va_start(args, format);
    ourResult = test_vsscanf(s, format, args);
    va_end(args);
  }
  {
    va_list args;
    va_start(args, format);
    libCResult = vsscanf(s, format, args);
    va_end(args);
  }
  printf("Test input: \"%s\"\nFormat string: \"%s\"\nOur result: %10d\tLibC result: %10d\t%s\n",
      s, format, ourResult, libCResult, (ourResult == libCResult ? "OK" : "FAIL"));
}

int main()
{
  {
    char buffer[255];
    test("this is a string", "%s", buffer, buffer);
    test("this is a string", "%s%s", buffer, buffer);
    test("this is a string", "%s %s", buffer, buffer);
    test("this is a string", "%3s", buffer);
    test("this is a string", "%*s %s", buffer);
    test("this is a string", "%5c", buffer);
    test("this is a string", "%5c %s", buffer, buffer); // FAIL
  }
  {
    int number;
    test("00ABCDEF", "%x", &number);
    test("0x00ABCDEF", "%x", &number);
    test("0X00ABCDEF", "%x", &number);
    test("0x00ABCDEF", "%3x", &number);
    test("97897097", "%d", &number);
    test("084085", "%o", &number);
    test("20 4A 888", "%2o %*2x %3d", &number, &number);
    test("20 4A", "%2o %2*x", &number);
    test("20   4A%abc", "%2o %*2x%%abc", &number, &number);
    test("40 abcdef", "%d %", &number, &number);
  }
  return 0;
}
