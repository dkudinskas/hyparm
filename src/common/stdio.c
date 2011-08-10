#ifdef TEST
#include <stdio.h>
#endif

#include "common/ctype.h"
#include "common/stdio.h"
#include "common/types.h"


#define MAX_INTEGER_64_DECIMAL_WIDTH      20
#define MAX_STRING_TOKEN_WIDTH           255


#ifdef TEST
#define TEST_PRINTF(...)  printf(__VA_ARGS__)
#else
#define TEST_PRINTF(...)
#endif


#define VSSCANF_DECIMAL(specifierString, integralType, isSigned) \
  { \
    if (ignore) \
    { \
      while (width > 0 && isdigit(*s)) \
      { \
        TEST_PRINTF("Consumed '%c'\n", *s); \
        ++s; \
      } \
    } \
    else \
    { \
      integralType value = 0; \
      while (width > 0 && isdigit(*s)) \
      { \
        value = (value * 10) + (integralType)(ASCII(*s) - ASCII('0')); \
        TEST_PRINTF("Consumed '%c'\n", *s); \
        ++s; \
      } \
      integralType *pointer = va_arg(args, integralType *); \
      if (isSigned) \
      { \
        value *= (integralType)sign; \
      } \
      TEST_PRINTF("Read value: %" specifierString "\n", value); \
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
        TEST_PRINTF("Consumed '%c'\n", *s); \
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
            TEST_PRINTF("Consumed '%c'\n", *s); \
            ++s; \
            break; \
          case 'a': \
          case 'b': \
          case 'c': \
          case 'd': \
          case 'e': \
          case 'f': \
            value = (value << 4) | (unsignedIntegralType)(10 + ASCII(*s) - ASCII('a')); \
            TEST_PRINTF("Consumed '%c'\n", *s); \
            ++s; \
            break; \
          case 'A': \
          case 'B': \
          case 'C': \
          case 'D': \
          case 'E': \
          case 'F': \
            value = (value << 4) | (unsignedIntegralType)(10 + ASCII(*s) - ASCII('A')); \
            TEST_PRINTF("Consumed '%c'\n", *s); \
            ++s; \
            break; \
          default: \
            width = 0; \
            break; \
        } \
      } \
      unsignedIntegralType *pointer = va_arg(args, unsignedIntegralType *); \
      TEST_PRINTF("Read value: %" modifierString "x\n", value); \
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
        TEST_PRINTF("Consumed '%c'\n", *s); \
        ++s; \
      } \
    } \
    else \
    { \
      unsignedIntegralType value = 0; \
      while (width > 0 && *s >= ASCII('0') && *s <= ASCII('7')) \
      { \
        value = (value << 3) | (unsignedIntegralType)(ASCII(*s) - ASCII('0')); \
        TEST_PRINTF("Consumed '%c'\n", *s); \
        ++s; \
      } \
      unsignedIntegralType *pointer = va_arg(args, unsignedIntegralType *); \
      TEST_PRINTF("Read value: %" modifierString "o\n", value); \
      *pointer = value; \
      ++convertedItemCount; \
    } \
  }


static int _vsscanf(const char *s, const char *format, va_list args);


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
  test_vsscanf
#else
  vsscanf
#endif
  (const char *s, const char *format, va_list args)
{
  /*
   * This function is a wrapper around _vsscanf to avoid duplicating the code to compute the return value.
   */
  int result = _vsscanf(s, format, args);
  return result < 1 ? EOF : result;
}

static int _vsscanf(const char *s, const char *format, va_list args)
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
      TEST_PRINTF("Format contains whitespace!\n");
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
        TEST_PRINTF("Error: input string does not contain expected whitespace!\n");
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
      TEST_PRINTF("Hit start of format specifier\n");
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
            TEST_PRINTF("Error: input string does not contain expected character '%%'!\n");
            return convertedItemCount;
          }
          ++s;
          ++format;
          TEST_PRINTF("Consumed '%%' character with '%%%%' specifier\n");
          continue; /* while (*format) */
        case '*':
          ++format;
          ignore = TRUE;
          TEST_PRINTF("Ignore is set for this format specifier\n");
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
          TEST_PRINTF("Error: when specified, width must be at least 1\n");
          return convertedItemCount;
        }
        TEST_PRINTF("Read width specifier: %d\n", width);
      }
      /*
       * The next character is either a modifier character or a specifier character. Check for
       * a modifier character.
       */
      if (*format == 'h' || *format == 'l' || *format == 'L')
      {
        modifier = *format;
        ++format;
        TEST_PRINTF("Modifier '%c' is set for this format specifier\n", modifier);
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
            TEST_PRINTF("Error: no modifiers supported for specifier '%%c'\n");
            return convertedItemCount;
          }
          /*
           * Default width is one character.
           */
          if (width < 0)
          {
            width = 1;
            TEST_PRINTF("No width given; reverting to default width for specifier '%%c': %d\n", width);
          }
          TEST_PRINTF("Consuming up to %d characters\n", width);
          if (ignore)
          {
            while (width > 0 && *s)
            {
              TEST_PRINTF("Consumed '%c'\n", *s);
              ++s;
              --width;
            }
          }
          else
          {
            char *pointer = va_arg(args, char *);
            while (width > 0 && *s)
            {
              TEST_PRINTF("Consumed and stored '%c'\n", *s);
              *pointer = *s;
              ++pointer;
              ++s;
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
            TEST_PRINTF("No width given; reverting to default width for specifier '%%d': %d\n", width);
          }
          if (*s == '+' || *s == '-')
          {
            sign = ASCII(',') - ASCII(*s);
            ++s;
            if (--width == 0)
            {
              TEST_PRINTF("Error: zero width for specifier '%%d' after reading sign\n");
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
            default:
              /*
               * Error: invalid modifier.
               */
              TEST_PRINTF("Error: invalid modifier '%c' for specifier '%%d'\n", modifier);
              return convertedItemCount;
          }
          break; /* format specifier: 'd' */
        case 'n':
          /*
           * Assign number of characters read so far.
           */
          if (!ignore && !modifier && width < 0)
          {
            u32int *pointer = va_arg(args, u32int *);
            *pointer = s - sOrigin;
          }
          else
          {
            if (ignore)
            {
              TEST_PRINTF("Error: ignoring the assignment of '%%n' is pointless\n");
            }
            if (modifier)
            {
              TEST_PRINTF("Error: no modifiers supported for specifier '%%n'\n");
            }
            if (width < 0)
            {
              TEST_PRINTF("Error: setting width for '%%n' is pointless\n");
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
            TEST_PRINTF("No width given; reverting to default width for specifier '%%o': %d\n", width);
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
            default:
              TEST_PRINTF("Error: invalid modifier '%c' for specifier '%%o'\n", modifier);
              return convertedItemCount;
          }
          break; /* format specifier: 'o' */
        case 's':
          /*
           * String: read subsequent characters until a whitespace is found.
           */
          if (modifier != 0)
          {
            TEST_PRINTF("Error: no modifiers supported for specifier '%%s'\n");
            return convertedItemCount;
          }
          if (width < 0)
          {
            width = MAX_STRING_TOKEN_WIDTH;
            TEST_PRINTF("No width given; reverting to default width for specifier '%%s': %d\n", width);
          }
          TEST_PRINTF("Consuming up to %d characters...\n", width);
          if (ignore)
          {
            while (width > 0 && *s && !isspace(*s))
            {
              TEST_PRINTF("Consumed '%c'\n", *s);
              ++s;
              --width;
            }
          }
          else
          {
            char *pointer = va_arg(args, char *);
            while (width > 0 && *s && !isspace(*s))
            {
              TEST_PRINTF("Consumed and stored '%c'\n", *s);
              *pointer = *s;
              ++pointer;
              ++s;
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
            TEST_PRINTF("No width given; reverting to default width for specifier '%%u': %d\n", width);
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
            default:
              TEST_PRINTF("Error: invalid modifier '%c' for specifier '%%u'\n", modifier);
              return convertedItemCount;
          }
          break; /* format specifier: 'u' */
        case 'p':
          /*
           * Pointer: like unsigned hexadecimal integer, but without modifier.
           */
          if (modifier != 0)
          {
            TEST_PRINTF("Error: no modifiers supported for specifier '%%p'\n");
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
            TEST_PRINTF("No width given; reverting to default width for specifier '%%%c': %d\n", *format, width);
          }
          /*
           * Check for prefix (we can safely skip a leading zero).
           */
          if (*s == '0')
          {
            ++s;
            if (*s == 'x' || *s == 'X')
            {
              ++s;
              TEST_PRINTF("Found hexadecimal prefix\n");
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
            default:
              TEST_PRINTF("Error: invalid modifier '%c' for specifier '%%%c'\n", modifier, *format);
              return convertedItemCount;
          }
          break; /* format specifier: 'p','x' */
        default:
          /*
           * Valid ANSI C specifiers not supported here are:
           *  - [...], [^...]: string match;
           *  - e, f, g: these are for floating point and we don't do floating point here;
           *  - i: denotes any integer -- you can avoid it if you really want to.
           */
          TEST_PRINTF("Error: unsupported specifier '%%%c'\n", *format);
          return convertedItemCount;
      }
      /*
       * Don't forget to move forward in the format string.
       */
      ++format;
      TEST_PRINTF("Done processing specifier; characters consumed: %d; converted and assigned: %d\n", s - sOrigin, convertedItemCount);
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
        TEST_PRINTF("Error: the input string does not contain the same non-whitespace character as the format string ('%%%c' vs. '%%%c')\n", *s, *format);
        return convertedItemCount;
      }
    }
  } /* while (*format) */
  return convertedItemCount;
}
