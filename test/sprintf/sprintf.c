#include <limits.h>
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


  printf("Format string: %20s\tOur result: %10d '%s'\tLibC result: %10d '%s'\t",
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

void signedDecimalTest(int number)
{
  test("%.d", number);
  test("%1.d", number);
  test("%2.d", number);
  test("%3.d", number);
  test("%.0d", number);
  test("%1.0d", number);
  test("%2.0d", number);
  test("%3.0d", number);
  test("%.1d", number);
  test("%1.1d", number);
  test("%2.1d", number);
  test("%3.1d", number);
  test("%.2d", number);
  test("%1.2d", number);
  test("%2.2d", number);
  test("%3.2d", number);
  test("%.3d", number);
  test("%1.3d", number);
  test("%2.3d", number);
  test("%3.3d", number);

  test("%+.d", number);
  test("%+1.d", number);
  test("%+2.d", number);
  test("%+3.d", number);
  test("%+.0d", number);
  test("%+1.0d", number);
  test("%+2.0d", number);
  test("%+3.0d", number);
  test("%+.1d", number);
  test("%+1.1d", number);
  test("%+2.1d", number);
  test("%+3.1d", number);
  test("%+.2d", number);
  test("%+1.2d", number);
  test("%+2.2d", number);
  test("%+3.2d", number);
  test("%+.3d", number);
  test("%+1.3d", number);
  test("%+2.3d", number);
  test("%+3.3d", number);

  test("% .d", number);
  test("% 1.d", number);
  test("% 2.d", number);
  test("% 3.d", number);
  test("% .0d", number);
  test("% 1.0d", number);
  test("% 2.0d", number);
  test("% 3.0d", number);
  test("% .1d", number);
  test("% 1.1d", number);
  test("% 2.1d", number);
  test("% 3.1d", number);
  test("% .2d", number);
  test("% 1.2d", number);
  test("% 2.2d", number);
  test("% 3.2d", number);
  test("% .3d", number);
  test("% 1.3d", number);
  test("% 2.3d", number);
  test("% 3.3d", number);

  test("%+ .d", number);
  test("%+ 1.d", number);
  test("%+ 2.d", number);
  test("%+ 3.d", number);
  test("%+ .0d", number);
  test("%+ 1.0d", number);
  test("%+ 2.0d", number);
  test("%+ 3.0d", number);
  test("%+ .1d", number);
  test("%+ 1.1d", number);
  test("%+ 2.1d", number);
  test("%+ 3.1d", number);
  test("%+ .2d", number);
  test("%+ 1.2d", number);
  test("%+ 2.2d", number);
  test("%+ 3.2d", number);
  test("%+ .3d", number);
  test("%+ 1.3d", number);
  test("%+ 2.3d", number);
  test("%+ 3.3d", number);

  test("%0.d", number);
  test("%01.d", number);
  test("%02.d", number);
  test("%03.d", number);
  test("%0.0d", number);
  test("%01.0d", number);
  test("%02.0d", number);
  test("%03.0d", number);
  test("%0.1d", number);
  test("%01.1d", number);
  test("%02.1d", number);
  test("%03.1d", number);
  test("%0.2d", number);
  test("%01.2d", number);
  test("%02.2d", number);
  test("%03.2d", number);
  test("%0.3d", number);
  test("%01.3d", number);
  test("%02.3d", number);
  test("%03.3d", number);

  test("%0+.d", number);
  test("%0+1.d", number);
  test("%0+2.d", number);
  test("%0+3.d", number);
  test("%0+.0d", number);
  test("%0+1.0d", number);
  test("%0+2.0d", number);
  test("%0+3.0d", number);
  test("%0+.1d", number);
  test("%0+1.1d", number);
  test("%0+2.1d", number);
  test("%0+3.1d", number);
  test("%0+.2d", number);
  test("%0+1.2d", number);
  test("%0+2.2d", number);
  test("%0+3.2d", number);
  test("%0+.3d", number);
  test("%0+1.3d", number);
  test("%0+2.3d", number);
  test("%0+3.3d", number);

  test("%0 .d", number);
  test("%0 1.d", number);
  test("%0 2.d", number);
  test("%0 3.d", number);
  test("%0 .0d", number);
  test("%0 1.0d", number);
  test("%0 2.0d", number);
  test("%0 3.0d", number);
  test("%0 .1d", number);
  test("%0 1.1d", number);
  test("%0 2.1d", number);
  test("%0 3.1d", number);
  test("%0 .2d", number);
  test("%0 1.2d", number);
  test("%0 2.2d", number);
  test("%0 3.2d", number);
  test("%0 .3d", number);
  test("%0 1.3d", number);
  test("%0 2.3d", number);
  test("%0 3.3d", number);

  test("%0+ .d", number);
  test("%0+ 1.d", number);
  test("%0+ 2.d", number);
  test("%0+ 3.d", number);
  test("%0+ .0d", number);
  test("%0+ 1.0d", number);
  test("%0+ 2.0d", number);
  test("%0+ 3.0d", number);
  test("%0+ .1d", number);
  test("%0+ 1.1d", number);
  test("%0+ 2.1d", number);
  test("%0+ 3.1d", number);
  test("%0+ .2d", number);
  test("%0+ 1.2d", number);
  test("%0+ 2.2d", number);
  test("%0+ 3.2d", number);
  test("%0+ .3d", number);
  test("%0+ 1.3d", number);
  test("%0+ 2.3d", number);
  test("%0+ 3.3d", number);
}

void unsignedDecimalTest(unsigned int number)
{
  test("%.u", number);
  test("%1.u", number);
  test("%2.u", number);
  test("%3.u", number);
  test("%.0u", number);
  test("%1.0u", number);
  test("%2.0u", number);
  test("%3.0u", number);
  test("%.1u", number);
  test("%1.1u", number);
  test("%2.1u", number);
  test("%3.1u", number);
  test("%.2u", number);
  test("%1.2u", number);
  test("%2.2u", number);
  test("%3.2u", number);
  test("%.3u", number);
  test("%1.3u", number);
  test("%2.3u", number);
  test("%3.3u", number);

  test("%+.u", number);
  test("%+1.u", number);
  test("%+2.u", number);
  test("%+3.u", number);
  test("%+.0u", number);
  test("%+1.0u", number);
  test("%+2.0u", number);
  test("%+3.0u", number);
  test("%+.1u", number);
  test("%+1.1u", number);
  test("%+2.1u", number);
  test("%+3.1u", number);
  test("%+.2u", number);
  test("%+1.2u", number);
  test("%+2.2u", number);
  test("%+3.2u", number);
  test("%+.3u", number);
  test("%+1.3u", number);
  test("%+2.3u", number);
  test("%+3.3u", number);

  test("% .u", number);
  test("% 1.u", number);
  test("% 2.u", number);
  test("% 3.u", number);
  test("% .0u", number);
  test("% 1.0u", number);
  test("% 2.0u", number);
  test("% 3.0u", number);
  test("% .1u", number);
  test("% 1.1u", number);
  test("% 2.1u", number);
  test("% 3.1u", number);
  test("% .2u", number);
  test("% 1.2u", number);
  test("% 2.2u", number);
  test("% 3.2u", number);
  test("% .3u", number);
  test("% 1.3u", number);
  test("% 2.3u", number);
  test("% 3.3u", number);

  test("%+ .u", number);
  test("%+ 1.u", number);
  test("%+ 2.u", number);
  test("%+ 3.u", number);
  test("%+ .0u", number);
  test("%+ 1.0u", number);
  test("%+ 2.0u", number);
  test("%+ 3.0u", number);
  test("%+ .1u", number);
  test("%+ 1.1u", number);
  test("%+ 2.1u", number);
  test("%+ 3.1u", number);
  test("%+ .2u", number);
  test("%+ 1.2u", number);
  test("%+ 2.2u", number);
  test("%+ 3.2u", number);
  test("%+ .3u", number);
  test("%+ 1.3u", number);
  test("%+ 2.3u", number);
  test("%+ 3.3u", number);

  test("%0.u", number);
  test("%01.u", number);
  test("%02.u", number);
  test("%03.u", number);
  test("%0.0u", number);
  test("%01.0u", number);
  test("%02.0u", number);
  test("%03.0u", number);
  test("%0.1u", number);
  test("%01.1u", number);
  test("%02.1u", number);
  test("%03.1u", number);
  test("%0.2u", number);
  test("%01.2u", number);
  test("%02.2u", number);
  test("%03.2u", number);
  test("%0.3u", number);
  test("%01.3u", number);
  test("%02.3u", number);
  test("%03.3u", number);

  test("%0+.u", number);
  test("%0+1.u", number);
  test("%0+2.u", number);
  test("%0+3.u", number);
  test("%0+.0u", number);
  test("%0+1.0u", number);
  test("%0+2.0u", number);
  test("%0+3.0u", number);
  test("%0+.1u", number);
  test("%0+1.1u", number);
  test("%0+2.1u", number);
  test("%0+3.1u", number);
  test("%0+.2u", number);
  test("%0+1.2u", number);
  test("%0+2.2u", number);
  test("%0+3.2u", number);
  test("%0+.3u", number);
  test("%0+1.3u", number);
  test("%0+2.3u", number);
  test("%0+3.3u", number);

  test("%0 .u", number);
  test("%0 1.u", number);
  test("%0 2.u", number);
  test("%0 3.u", number);
  test("%0 .0u", number);
  test("%0 1.0u", number);
  test("%0 2.0u", number);
  test("%0 3.0u", number);
  test("%0 .1u", number);
  test("%0 1.1u", number);
  test("%0 2.1u", number);
  test("%0 3.1u", number);
  test("%0 .2u", number);
  test("%0 1.2u", number);
  test("%0 2.2u", number);
  test("%0 3.2u", number);
  test("%0 .3u", number);
  test("%0 1.3u", number);
  test("%0 2.3u", number);
  test("%0 3.3u", number);

  test("%0+ .u", number);
  test("%0+ 1.u", number);
  test("%0+ 2.u", number);
  test("%0+ 3.u", number);
  test("%0+ .0u", number);
  test("%0+ 1.0u", number);
  test("%0+ 2.0u", number);
  test("%0+ 3.0u", number);
  test("%0+ .1u", number);
  test("%0+ 1.1u", number);
  test("%0+ 2.1u", number);
  test("%0+ 3.1u", number);
  test("%0+ .2u", number);
  test("%0+ 1.2u", number);
  test("%0+ 2.2u", number);
  test("%0+ 3.2u", number);
  test("%0+ .3u", number);
  test("%0+ 1.3u", number);
  test("%0+ 2.3u", number);
  test("%0+ 3.3u", number);
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
  test("%#.8x", 0xBAD123);
  test("%#+02.6x", 0x12);
  test("%#+020.6x", 0x12);
  test("%+020.6x", 0x12);
  test("%d", 99);
  test("%d", -103);
  test("%5d", 1);
  test("% 5.0x", 0);
  test("%#5.0x", 0);
  test("% 5.x", 0);
  test("%#5.x", 0);
  test("% 5x", 0);
  test("%#5x", 0);

  test("%#Lx", (unsigned long long int) 0ULL);
  test("%#Lx", (unsigned long long int) 0x00000000FFFFFFFEULL);
  test("%#Lx", (unsigned long long int) 0x00000000FFFFFFFFULL);
  test("%#Lx", (unsigned long long int) 0x0000000100000000ULL);
  test("%#Lx", (unsigned long long int) 0x0000FFFF00FFFFFEULL);
  test("%#Lx", (unsigned long long int) 0xFFFFFFFFFFFFFFFEULL);
  test("%#Lx", (unsigned long long int) 0xFFFFFFFFFFFFFFFFULL);

  signedDecimalTest(INT_MIN);
  signedDecimalTest(-1);
  signedDecimalTest(0);
  signedDecimalTest(1);
  signedDecimalTest(INT_MAX);

  unsignedDecimalTest(INT_MIN);
  unsignedDecimalTest(-1);
  unsignedDecimalTest(0);
  unsignedDecimalTest(1);
  unsignedDecimalTest(INT_MAX);
  unsignedDecimalTest(UINT_MAX);

  printf("\n%12s%10d\n%12s%10d\n\n", "SUCCEEDED:", testsSucceeded, "FAILED:", testsFailed);

  return 0;
}
