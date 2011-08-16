#ifdef TEST
#include <stdio.h>
#include <string.h>
#else
#include <common/debug.h>
#include <common/stringFunctions.h>
#endif

#include "common/ctype.h"
#include "common/stdio.h"
#include "common/types.h"


/*
 * WARNING: beware of parametric and variadic macros.
 *
 * DO NOT
 *  - panic;
 *  - try to be clever and circumvent the use of macros by performing expensive 64-bit arithmetic;
 *  - use anything other than constant expressions or variable names as arguments to macros;
 *  - use auto-increment or auto-decrement operations anywhere in macro arguments.
 */


#define MAX_INTEGER_64_DECIMAL_WIDTH      20
#define MAX_STRING_TOKEN_WIDTH           255


#ifdef TEST
#define TEST_PRINTF(...)  printf(__VA_ARGS__)
#else
#define TEST_PRINTF(...)
#endif


#define VSPRINTF_FLAG_ADJUST_LEFT    0x01
#define VSPRINTF_FLAG_ALTERNATE      0x02


#define VSSCANF_DECIMAL(specifierString, integralType, isSigned) \
  { \
    if (ignore) \
    { \
      while (width > 0 && isdigit(*s)) \
      { \
        TEST_PRINTF("Consumed '%c'" EOL, *s); \
        ++s; \
      } \
    } \
    else \
    { \
      integralType value = 0; \
      while (width > 0 && isdigit(*s)) \
      { \
        value = (value * 10) + (integralType)(ASCII(*s) - ASCII('0')); \
        TEST_PRINTF("Consumed '%c'" EOL, *s); \
        ++s; \
      } \
      integralType *pointer = va_arg(args, integralType *); \
      if (isSigned) \
      { \
        value *= (integralType)sign; \
      } \
      TEST_PRINTF("Read value: %" specifierString EOL, value); \
      *pointer = value; \
      ++convertedItemCount; \
    } \
  }


#define VSSCANF_HEXADECIMAL(modifierString, unsignedIntegralType) \
  { \
    if (ignore) \
    { \
      while (width > 0 && isxdigit(*s)) \
      { \
        TEST_PRINTF("Consumed '%c'" EOL, *s); \
        ++s; \
      } \
    } \
    else \
    { \
      unsignedIntegralType value = 0; \
      while (width > 0) \
      { \
        switch (*s) \
        { \
          case '0': \
          case '1': \
          case '2': \
          case '3': \
          case '4': \
          case '5': \
          case '6': \
          case '7': \
          case '8': \
          case '9': \
            value = (value << 4) | (unsignedIntegralType)(ASCII(*s) - ASCII('0')); \
            TEST_PRINTF("Consumed '%c'" EOL, *s); \
            ++s; \
            break; \
          case 'a': \
          case 'b': \
          case 'c': \
          case 'd': \
          case 'e': \
          case 'f': \
            value = (value << 4) | (unsignedIntegralType)(10 + ASCII(*s) - ASCII('a')); \
            TEST_PRINTF("Consumed '%c'" EOL, *s); \
            ++s; \
            break; \
          case 'A': \
          case 'B': \
          case 'C': \
          case 'D': \
          case 'E': \
          case 'F': \
            value = (value << 4) | (unsignedIntegralType)(10 + ASCII(*s) - ASCII('A')); \
            TEST_PRINTF("Consumed '%c'" EOL, *s); \
            ++s; \
            break; \
          default: \
            width = 0; \
            break; \
        } \
      } \
      unsignedIntegralType *pointer = va_arg(args, unsignedIntegralType *); \
      TEST_PRINTF("Read value: %" modifierString "x" EOL, value); \
      *pointer = value; \
      ++convertedItemCount; \
    } \
  }


#define VSSCANF_OCTAL(modifierString, unsignedIntegralType) \
  { \
    if (ignore) \
    { \
      while (width > 0 && *s >= ASCII('0') && *s <= ASCII('7')) \
      { \
        TEST_PRINTF("Consumed '%c'" EOL, *s); \
        ++s; \
      } \
    } \
    else \
    { \
      unsignedIntegralType value = 0; \
      while (width > 0 && *s >= ASCII('0') && *s <= ASCII('7')) \
      { \
        value = (value << 3) | (unsignedIntegralType)(ASCII(*s) - ASCII('0')); \
        TEST_PRINTF("Consumed '%c'" EOL, *s); \
        ++s; \
      } \
      unsignedIntegralType *pointer = va_arg(args, unsignedIntegralType *); \
      TEST_PRINTF("Read value: %" modifierString "o" EOL, value); \
      *pointer = value; \
      ++convertedItemCount; \
    } \
  }


static inline int _vsscanf(const char *s, const char *format, va_list args);


int
#ifdef TEST
  test_sscanf
#else
  sscanf
#endif
  (const char *s, const char *format, ...)
{
  va_list args;
  int result;

  va_start(args, format);
  result =
#ifdef TEST
    test_vsscanf
#else
    vsscanf
#endif
    (s, format, args);
  va_end(args);

  return result;
}

int
#ifdef TEST
  test_vsprintf
#else
  vsprintf
#endif
  (char *s, const char *format, va_list args)
{
  char *const sOrigin = s;
  bool fixMeHadCarriageReturn = FALSE;
  while (*format)
  {
    /*
     * FIXME: DIRTY!!!!!!!!!!!!!
     */
    if (*format == '\r')
    {
      fixMeHadCarriageReturn = TRUE;
    }
    else
    {
      fixMeHadCarriageReturn = FALSE;
    }
    if (*format == '\n')
    {
      if (!fixMeHadCarriageReturn)
      {
        *s++ = '\r';
      }
    }
    /*
     * END OF FIXME
     */
    if (*format != '%')
    {
      /*
       * Any character other than the percentage sign is copied as is.
       */
      *s++ = *format++;
    }
    else
    {
      char defaultSign = 0;
      u8int flags = 0;
      bool isFlag = TRUE;
      char modifier = 0;
      const char *formatOrigin = format;
      char padChar = ' ';
      s32int precision = -1;
      s32int width = 0;
      /*
       * The percentage sign is the start of a format specifier.
       */
      TEST_PRINTF("Hit start of format specifier" EOL);
      ++format;
      /*
       * If followed by another percentage sign, print a literal percentage sign.
       */
      if (*format == '%')
      {
        TEST_PRINTF("Found '%%%%'; printing literal '%%'" EOL);
        *s++ = *format++;
        continue;
      }
      /*
       * Syntax: '%' [flags] [width] ['.' precision] [modifier_character] specifier_character
       *
       * Flags are optional, not mutually exclusive, and may appear in any order. Valid flags are:
       *  - '-': adjust left;
       *  - '+': always print sign;
       *  - ' ': if the first character is not a sign, print a space;
       *  - '0': pad with zero instead of space in numeric fields;
       *  - '#': alternate output form (adds octal and hexadecimal prefix to non-zero numbers, ...).
       */
      do
      {
        switch (*format)
        {
          case '-':
            TEST_PRINTF("Read flag '-': adjust left" EOL);
            flags |= VSPRINTF_FLAG_ADJUST_LEFT;
            ++format;
            break;
          case '+':
            TEST_PRINTF("Read flag '+': always print sign" EOL);
            defaultSign = '+';
            ++format;
            break;
          case ' ':
            TEST_PRINTF("Read flag ' ': if the first character is not a sign, print a space" EOL);
            if (!defaultSign)
            {
              defaultSign = ' ';
            }
            ++format;
            break;
          case '0':
            TEST_PRINTF("Read flag '0': pad with zero instead of space in numeric fields" EOL);
            padChar = '0';
            ++format;
            break;
          case '#':
            TEST_PRINTF("Read flag '#': alternate output form" EOL);
            flags |= VSPRINTF_FLAG_ALTERNATE;
            ++format;
            break;
          default:
            isFlag = FALSE;
            continue;
        } /* switch (*format) */
      } while (isFlag);
      TEST_PRINTF("Flags: %x" EOL, flags);
      if (defaultSign)
      {
        TEST_PRINTF("Default sign: '%c'" EOL, defaultSign);
      }
      TEST_PRINTF("Padding character: '%c'" EOL, padChar);
      /*
       * The next character(s), if digits or a single '*', specify the minimum width.
       */
      if (isdigit(*format))
      {
        /*
         * Read width; this specifies the maximum number of characters to be read from the input string.
         */
        do
        {
          width = (width * 10) + (s32int)(ASCII(*format) - ASCII('0'));
          ++format;
        } while (isdigit(*format));
        TEST_PRINTF("Read width specifier: %d" EOL, width);
      }
      else if (*format == '*')
      {
        /*
         * Width must be read from next argument.
         */
        width = va_arg(args, s32int);
      }
      /*
       * The next character(s), if digits preceded by a dot, specify the precision.
       * Be tolerant: if there is a dot, but no digits following the dot, ignore the dot.
       */
      if (*format == '.')
      {
        ++format;
        if (isdigit(*format))
        {
          /*
           * Read precision.
           */
          precision = 0;
          do
          {
            precision = (precision * 10) + (s32int)(ASCII(*format) - ASCII('0'));
            ++format;
          } while (isdigit(*format));
          TEST_PRINTF("Read precision specifier: %d" EOL, precision);
        }
        else if (*format == '*')
        {
          /*
           * Precision must be read from next argument.
           */
          precision = va_arg(args, s32int);
        }
        else
        {
          TEST_PRINTF("Warning: incomplete precision specifier!" EOL);
        }
      }
      /*
       * The next character is either a modifier character or a specifier character. Check for
       * a modifier character.
       *
       * Valid modifiers are: 'h', 'l' and 'L'. In ANSI C, L is only used with floating point types.
       */
      if (
#if 0
          *format == 'h' || *format == 'l'
#ifdef CONFIG_STDIO_FP
            || *format == 'L'
#endif
#else
          0 /* TODO */
#endif
        )
      {
        modifier = *format;
        ++format;
        TEST_PRINTF("Modifier '%c' is set for this format specifier" EOL, modifier);
      }
      /*
       * Switch on specifier character.
       */
      TEST_PRINTF("Specifier character is '%c'" EOL, *format);
      char specifierChar = *format;
      switch (*format++)
      {
        case 'c':
        {
          /*
           * Single character passed as int.
           */
          unsigned char value = (unsigned char)(va_arg(args, s32int));
          if (!(flags & VSPRINTF_FLAG_ADJUST_LEFT))
          {
            *s++ = (char)value;
          }
          while (--width > 0)
          {
            *s++ = ' ';
          }
          if ((flags & VSPRINTF_FLAG_ADJUST_LEFT))
          {
            *s++ = (char)value;
          }
          break;
        }
        /*case 'd':
        case 'i':
        {*/
          /*
           * Signed decimal.
           */
          //s32int value = va_arg(args, s32int);
          /*
           * TODO
           */
        /*  break;
        }*/
        case 'n':
        {
          /*
           * Number of characters written so far (stored in argument; not printed).
           */
          s32int *pointer = va_arg(args, s32int *);
          *pointer = (s32int)((u32int)s - (u32int)sOrigin);
          break;
        }
        /*case 'o':
        {*/
          /*
           * Unsigned octal notation.
           */
          //u32int value = va_arg(args, u32int);
          /*
           * TODO
           */
        /*  break;
        }
        case 'u':
        {*/
          /*
           * Unsigned decimal.
           */
          //s32int value = va_arg(args, s32int);
          /*
           * TODO
           */
        /*  break;
        }*/
        case 's':
        {
          /*
           * Null-terminated string.
           */
          char *string = va_arg(args, char *);
          if (width < 1)
          {
            /*
             * Width, padding and alignment are of no importance.
             */
            if (precision < 0)
            {
              /*
               * If precision is not set, just copy the string.
               */
              while (*string)
              {
                *s++ = *string++;
              }
            }
            else
            {
              /*
               * Print up to precision characters of the string.
               */
              while (*string && precision > 0)
              {
                *s++ = *string++;
                --precision;
              }
            }
          }
          else
          {
            /*
             * Calculate string length (bounded by precision).
             */
            s32int length = strlen(string);
            if (precision >= 0 && precision < length)
            {
              length = precision;
            }
            else
            {
              precision = length;
            }
            /*
             * Default is to align right; pad left if VSPRINTF_FLAG_ADJUST_LEFT is not set.
             */
            if (!(flags & VSPRINTF_FLAG_ADJUST_LEFT))
            {
              while (length < width)
              {
                *s++ = ' ';
                ++length;
              }
            }
            /*
             * Copy up to length characters of string (precision always equals length at this point,
             * even if not set a priori).
             */
            while (*string && precision > 0)
            {
              *s++ = *string++;
              --precision;
            }
            /*
             * If VSPRINTF_FLAG_ADJUST_LEFT is set, the alignment is left so pad right. The test for
             * the flag can be omitted as length >= width in case it was not set.
             */
            while (length < width)
            {
              *s++ = ' ';
              ++length;
            }
          }
          break;
        }
        case 'p':
        {
          /*
           * Pointer.
           *
           * A pointer (of type void *, according to the ANSI C specification) can be safely treated
           * as an u32int, hence we can fall through to the case for unsigned hexadecimal, by
           * forcing no modifier.
           */
          modifier = 0;
          /*
           * Print the hexadecimal prefix "0x", and use zero padding.
           */
          flags |= VSPRINTF_FLAG_ALTERNATE;
          padChar = '0';
          specifierChar = 'x';
          /*
           * Also force minimum number of digits to be printed...
           */
          precision = (sizeof(u32int) << 1);
          /*
           * Fall-through!
           */
        }
        case 'x':
        case 'X':
        {
          /*
           * Unsigned hexadecimal notation
           *
           * TODO check modifier and do it for 16/64-bit
           */
          u32int value = va_arg(args, u32int);
          /*
           * The digit width of the hexadecimal string representation is calculated as follows:
           *  - if value is zero, it can be represented by a single '0' character, so the digit
           *    width is 1;
           *  - if value is not zero, the number of significant digits can be computed by
           *    substracting the the number of leading zeros divided by the number of bits per
           *    digit, from the total number of digits.
           */
          s32int digitWidth;
          if (value == 0)
          {
            digitWidth = 1;
            flags &= ~VSPRINTF_FLAG_ALTERNATE;
          }
          else
          {
            digitWidth = (sizeof(u32int) << 1) - (__builtin_clz(value) >> 2);
          }
          /*
           * The precision specifies the minimum number of digits we must print. Zero padding must
           * be added if the initial width is less than precision!
           */
          if (digitWidth < precision)
          {
            digitWidth = precision;
          }
          /*
           * Now calculate the width including the prefix. Precision excludes the prefix string!
           * Since VSPRINTF_FLAG_ALTERNATE == 2, the prefix length will be added if the flag is set.
           */
          s32int prefixedWidth = digitWidth + (flags & VSPRINTF_FLAG_ALTERNATE);
          /*
           * Default is to align right; pad left if VSPRINTF_FLAG_ADJUST_LEFT is not set.
           */
          if (!(flags & VSPRINTF_FLAG_ADJUST_LEFT))
          {
            while (prefixedWidth < width)
            {
              *s++ = padChar;
              ++prefixedWidth;
            }
          }
          /*
           * Print prefix, if requested.
           */
          if ((flags & VSPRINTF_FLAG_ALTERNATE))
          {
            *s++ = '0';
            *s++ = specifierChar;
          }
          /*
           * Print up to digitWidth digits.
           */
          s32int i;
          for (i = digitWidth - 1; i >= 0; --i)
          {
            u8int nibble = (value >> (i << 2)) & 0xF;
            if (nibble <= 9)
            {
              *s++ = (char)(ASCII('0') + nibble);
            }
            else
            {
              *s++ = (char)(ASCII('A') + (specifierChar - ASCII('X')) + (nibble - 10));
            }
          }
          /*
           * If VSPRINTF_FLAG_ADJUST_LEFT is set, the alignment is left so pad right. The test for
           * the flag can be omitted as length >= width in case it was not set.
           */
          while (prefixedWidth < width)
          {
            *s++ = padChar;
            ++prefixedWidth;
          }
          break;
        }
        default:
          /*
           * Unrecognized specifier character.
           */
          TEST_PRINTF("Warning: unrecognized specifier character '%c'" EOL, specifierChar);
          /*
           * If we hit a %<NULL>, make sure we don't end the string with a double NULL, and don't
           * copy beyond the <NULL> either.
           */
          if (!specifierChar)
          {
            --format;
          }
          /*
           * Print the whole format specifier as is to the destination string.
           */
          while (formatOrigin < format)
          {
            *s = *formatOrigin;
            ++s;
            ++formatOrigin;
          }
          break;
      }
    }
  }
  /*
   * Correct behavior is to return the length of the string, this does not include the terminating
   * NULL character, so do not increment s.
   */
  *s = 0;
  return (s32int)((u32int)s - (u32int)sOrigin);
}

int
#ifdef TEST
  test_vsscanf
#else
  vsscanf
#endif
  (const char *s, const char *format, va_list args)
{
  /*
   * This function is a wrapper around _vsscanf to avoid duplicating the code to compute the return value.
   */
  TEST_PRINTF("vsscanf wrapper: string = '%s'; format = '%s'" EOL, s, format);
  int result = _vsscanf(s, format, args);
  return result < 1 ? EOF : result;
}

static inline int _vsscanf(const char *s, const char *format, va_list args)
{
  const char *const sOrigin = s;
  s32int convertedItemCount = 0;
  while (*format && *s)
  {
    /*
     * Check for whitespace, percentage sign or other non-whitespace character in format string
     */
    if (isspace(*format))
    {
      TEST_PRINTF("Format contains whitespace!" EOL);
      /*
       * Any amount of whitespace in the format string maps to any amount of whitespace in the input string.
       */
      ++format;
      if (isspace(*s))
      {
        /*
         * Skip all subsequent whitespace characters in the input string at once.
         */
        ++s;
        for (; isspace(*s); ++s);
      }
      else
      {
        TEST_PRINTF("Error: input string does not contain expected whitespace!" EOL);
        return convertedItemCount;
      }
      /*
       * Skip all subsequent whitespace characters in format at once.
       */
      for (; isspace(*format); ++format);
    } /* if (isspace(*format)) */
    else if (*format == '%')
    {
      bool ignore = FALSE;
      char modifier = 0;
      s32int sign = 1;
      s32int width = -1;
      /*
       * The percentage sign is the start of a format specifier.
       */
      TEST_PRINTF("Hit start of format specifier" EOL);
      ++format;
      /*
       * The next character may be
       *  - '%': the original string should contain a literal percentage sign;
       *  - '*': the input from this specifier must not be assigned to a variable;
       *  - the beginning of the width field, a modifier character, or a specifier character.
       */
      switch (*format)
      {
        case '%':
          if (*s != '%')
          {
            TEST_PRINTF("Error: input string does not contain expected character '%%'!" EOL);
            return convertedItemCount;
          }
          ++s;
          ++format;
          TEST_PRINTF("Consumed '%%' character with '%%%%' specifier" EOL);
          continue; /* while (*format) */
        case '*':
          ++format;
          ignore = TRUE;
          TEST_PRINTF("Ignore is set for this format specifier" EOL);
          break;
      }
      /*
       * The next character(s), if digits, specify the width. Otherwise, the next character must
       * be either a modifier character or a specifier character.
       */
      if (isdigit(*format))
      {
        /*
         * Read width; this specifies the maximum number of characters to be read from the input string.
         */
        width = 0;
        do
        {
          width = (width * 10) + (s32int)(ASCII(*format) - ASCII('0'));
          ++format;
        } while (isdigit(*format));
        if (width == 0)
        {
          TEST_PRINTF("Error: when specified, width must be at least 1" EOL);
          return convertedItemCount;
        }
        TEST_PRINTF("Read width specifier: %d" EOL, width);
      }
      /*
       * The next character is either a modifier character or a specifier character. Check for
       * a modifier character.
       *
       * Valid modifiers are: 'h', 'l' and 'L'. In ANSI C, L is only used with floating point types.
       *
       * WARNING: adding a custom modifier here requires modification of ALL tests on the modifier
       * below, as without CONFIG_STDIO_FP, the only possible values of modifier are 0, 'h', and
       * 'l', and the default case will be compiled out! Not adapting these tests will break
       * consistent handling of erroneous format strings.
       */
      if (
          *format == 'h' || *format == 'l'
#ifdef CONFIG_STDIO_FP
            || *format == 'L'
#endif
        )
      {
        modifier = *format++;
        TEST_PRINTF("Modifier '%c' is set for this format specifier" EOL, modifier);
      }
      /*
       * Switch on specifier character.
       */
      TEST_PRINTF("Specifier character is '%c'\n", *format);
      switch (*format)
      {
        case 'c':
          /*
           * Character: a single character, unless a width is given, then [width] characters are read.
           */
          if (modifier != 0)
          {
            TEST_PRINTF("Error: no modifiers supported for specifier '%%c'" EOL);
            return convertedItemCount;
          }
          /*
           * Default width is one character.
           */
          if (width < 0)
          {
            width = 1;
            TEST_PRINTF("No width given; reverting to default width for specifier '%%c': %d" EOL, width);
          }
          TEST_PRINTF("Consuming up to %d characters" EOL, width);
          if (ignore)
          {
            while (width > 0 && *s)
            {
              TEST_PRINTF("Consumed '%c'" EOL, *s);
              ++s;
              --width;
            }
          }
          else
          {
            char *pointer = va_arg(args, char *);
            while (width > 0 && *s)
            {
              TEST_PRINTF("Consumed and stored '%c'" EOL, *s);
              *pointer++ = *s++;
              --width;
            }
            ++convertedItemCount;
          }
          break; /* format specifier: 'c' */
        case 'd':
          /*
           * Decimal integer: number optionally preceded by '+' or '-'.
           */
          if (width < 0)
          {
            width = MAX_INTEGER_64_DECIMAL_WIDTH;
            TEST_PRINTF("No width given; reverting to default width for specifier '%%d': %d" EOL, width);
          }
          if (*s == '+' || *s == '-')
          {
            sign = ASCII(',') - ASCII(*s);
            ++s;
            if (--width == 0)
            {
              TEST_PRINTF("Error: zero width for specifier '%%d' after reading sign" EOL);
              return convertedItemCount;
            }
          }
          switch (modifier)
          {
            case 0:
              VSSCANF_DECIMAL("d", s32int, 1);
              break;
            case 'h':
              VSSCANF_DECIMAL("hd", s16int, 1);
              break;
            case 'l':
              VSSCANF_DECIMAL("ld", s64int, 1);
              break;
#ifdef CONFIG_STDIO_FP
            default:
              TEST_PRINTF("Error: invalid modifier '%c' for specifier '%%d'" EOL, modifier);
              return convertedItemCount;
#endif
          }
          break; /* format specifier: 'd' */
        case 'n':
          /*
           * Assign number of characters read so far.
           */
          if (!ignore && !modifier && width < 0)
          {
            u32int *pointer = va_arg(args, u32int *);
            *pointer = (u32int)s - (u32int)sOrigin;
          }
          else
          {
            if (ignore)
            {
              TEST_PRINTF("Error: ignoring the assignment of '%%n' is pointless" EOL);
            }
            if (modifier)
            {
              TEST_PRINTF("Error: no modifiers supported for specifier '%%n'" EOL);
            }
            if (width < 0)
            {
              TEST_PRINTF("Error: setting width for '%%n' is pointless" EOL);
            }
            return convertedItemCount;
          }
          break; /* format specifier: 'n' */
        case 'o':
          /*
           * Octal unsigned integer.
           */
          if (width < 0)
          {
            width = 2 + (sizeof(u64int) << 1);
            TEST_PRINTF("No width given; reverting to default width for specifier '%%o': %d" EOL, width);
          }
          /*
           * There is no need to check for the prefix here, as it is optional and we can safely use the leading zero.
           */
          switch (modifier)
          {
            case 0:
              VSSCANF_OCTAL("", u32int);
              break;
            case 'h':
              VSSCANF_OCTAL("h", u16int);
              break;
            case 'l':
              VSSCANF_OCTAL("l", u64int);
              break;
#ifdef CONFIG_STDIO_FP
            default:
              TEST_PRINTF("Error: invalid modifier '%c' for specifier '%%o'" EOL, modifier);
              return convertedItemCount;
#endif
          }
          break; /* format specifier: 'o' */
        case 's':
          /*
           * String: read subsequent characters until a whitespace is found.
           */
          if (modifier != 0)
          {
            TEST_PRINTF("Error: no modifiers supported for specifier '%%s'" EOL);
            return convertedItemCount;
          }
          if (width < 0)
          {
            width = MAX_STRING_TOKEN_WIDTH;
            TEST_PRINTF("No width given; reverting to default width for specifier '%%s': %d" EOL, width);
          }
          TEST_PRINTF("Consuming up to %d characters..." EOL, width);
          if (ignore)
          {
            while (width > 0 && *s && !isspace(*s))
            {
              TEST_PRINTF("Consumed '%c'" EOL, *s);
              ++s;
              --width;
            }
          }
          else
          {
            char *pointer = va_arg(args, char *);
            while (width > 0 && *s && !isspace(*s))
            {
              TEST_PRINTF("Consumed and stored '%c'" EOL, *s);
              *pointer++ = *s++;
              --width;
            }
            *pointer = 0;
            ++convertedItemCount;
          }
          break; /* format specifier: 's' */
        case 'u':
          /*
           * Unsigned decimal integer.
           */
          if (width < 0)
          {
            width = MAX_INTEGER_64_DECIMAL_WIDTH;
            TEST_PRINTF("No width given; reverting to default width for specifier '%%u': %d" EOL, width);
          }
          switch (modifier)
          {
            case 0:
              VSSCANF_DECIMAL("u", u32int, 0);
              break;
            case 'h':
              VSSCANF_DECIMAL("hu", u16int, 0);
              break;
            case 'l':
              VSSCANF_DECIMAL("lu", u64int, 0);
              break;
#ifdef CONFIG_STDIO_FP
            default:
              TEST_PRINTF("Error: invalid modifier '%c' for specifier '%%u'" EOL, modifier);
              return convertedItemCount;
#endif
          }
          break; /* format specifier: 'u' */
        case 'p':
          /*
           * Pointer: like unsigned hexadecimal integer, but without modifier.
           */
          if (modifier != 0)
          {
            TEST_PRINTF("Error: no modifiers supported for specifier '%%p'" EOL);
            return convertedItemCount;
          }
          /*
           * Fall through...
           */
        case 'x':
          /*
           * Unsigned hexadecimal integer, with or without leading "0x" or "0X".
           */
          if (width < 0)
          {
            width = 2 + (sizeof(u64int) << 1);
            TEST_PRINTF("No width given; reverting to default width for specifier '%%%c': %d" EOL, *format, width);
          }
          /*
           * Check for prefix (we can safely skip a leading zero) if the set maximum width allows for one..
           */
          if (width > 2 && *s == '0')
          {
            ++s;
            --width;
            if (*s == 'x' || *s == 'X')
            {
              ++s;
              --width;
              TEST_PRINTF("Found hexadecimal prefix" EOL);
            }
          }
          switch (modifier)
          {
            case 0:
              VSSCANF_HEXADECIMAL("", u32int);
              break;
            case 'h':
              VSSCANF_HEXADECIMAL("h", u16int);
              break;
            case 'l':
              VSSCANF_HEXADECIMAL("l", u64int);
              break;
#ifdef CONFIG_STDIO_FP
            default:
              TEST_PRINTF("Error: invalid modifier '%c' for specifier '%%%c'" EOL, modifier, *format);
              return convertedItemCount;
#endif
          }
          break; /* format specifier: 'p','x' */
        default:
          /*
           * Valid ANSI C specifiers not supported here are:
           *  - [...], [^...]: string match;
           *  - e, f, g: these are for floating point and we don't do floating point here;
           *  - i: denotes any integer -- you can avoid it if you really want to.
           */
          TEST_PRINTF("Error: unsupported specifier '%%%c'" EOL, *format);
          return convertedItemCount;
      }
      /*
       * Don't forget to move forward in the format string.
       */
      ++format;
      TEST_PRINTF("Done processing specifier; characters consumed: %d; converted and assigned: %d" EOL, s - sOrigin, convertedItemCount);
    }
    else
    {
      /*
       * Any non-whitespace character in the format string, other than the percentage sign, must match the next character in the input string.
       */
      if (*s == *format)
      {
        ++s;
        ++format;
      }
      else
      {
        /*
         * Error: the input string does not contain the same non-whitespace character as the format string!
         */
        TEST_PRINTF("Error: the input string does not contain the same non-whitespace character as the format string ('%%%c' vs. '%%%c')" EOL, *s, *format);
        return convertedItemCount;
      }
    }
  } /* while (*format) */
  return convertedItemCount;
}
