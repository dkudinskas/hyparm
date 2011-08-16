#include <stdio.h>
#include <string.h>

#include "common/stdio.h"

int testsSucceeded = 0;
int testsFailed = 0;

void test(const char *format, ...)
{
  char ourBuffer[255];
  char libCBuffer[255];
  int ourResult, libCResult;
  {
    va_list args;
    va_start(args, format);
    ourResult = test_vsprintf(ourBuffer, format, args);
    va_end(args);
  }
  {
    va_list args;
    va_start(args, format);
    libCResult = vsprintf(libCBuffer, format, args);
    va_end(args);
  }


  printf("Format string: \"%s\"\nOur result: %10d '%s'\tLibC result: %10d '%s'\t",
      format, ourResult, ourBuffer, libCResult, libCBuffer
    );

  if (ourResult == libCResult && strcmp(ourBuffer,libCBuffer) == 0)
  {
    ++testsSucceeded;
    puts("OK");
  }
  else
  {
    ++testsFailed;
    puts("FAIL");
  }
}

int main()
{
  test("this is %Y ju%%st a torture test%");
  test("%s", "this is a string");
  test("%10s", "right");
  test("%-10s", "left");
  test("%2s", "oversized");
  test("%8s", "");
  test("%-8s", "");
  test("%1s", "");
  test("%-1s", "");
  test("%1s", "");
  test("%-1s", "");
  test("%0s", "");
  test("%-0s", "");
  test("%0s", "abc");
  test("%-0s", "abc");
  test("%-s", "test");
  test("%x", 0xBAD);
  test("%X", 0xBAD);
  test("%#x", 0xBAD);
  test("%#X", 0xBAD);
  test("%10x", 0xBAD);
  test("%10X", 0xBAD);
  test("%#10x", 0xBAD);
  test("%#10X", 0xBAD);
  test("% 10x", 0xBAD);
  test("% 10X", 0xBAD);
  test("% #10x", 0xBAD);
  test("% #10X", 0xBAD);
  test("%# 10x", 0xBAD);
  test("%# 10X", 0xBAD);
  test("%-10x", 0xBAD);
  test("%- 10x", 0xBAD);
  test("%#- 10x", 0xBAD);
  test("%4x", 0xBAD123);
  test("% 4x", 0xBAD123);
  test("%# 4x", 0xBAD123);
  test("%-4x", 0xBAD123);
  test("%- 4x", 0xBAD123);
  test("%# 4x", 0xBAD123);
  test("%-# 4x", 0xBAD123);
  test("%10.8x", 0xBAD123);
  test("% 10.8x", 0xBAD123);
  test("%#10.8x", 0xBAD123);
  test("%# 10.8x", 0xBAD123);

  printf("\n%12s%10d\n%12s%10d\n\n", "SUCCEEDED:", testsSucceeded, "FAILED:", testsFailed);

  return 0;
}
