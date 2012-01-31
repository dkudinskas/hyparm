#include "common/ctype.h"


s32int isalnum(s32int c)
{
  return isalpha(c) || isdigit(c);
}

s32int isalpha(s32int c)
{
  return islower(c) || isupper(c);
}

s32int iscntrl(s32int c)
{
  return (c >= ASCII_CONTROL_BEGIN && c <= ASCII_CONTROL_END) || (c == ASCII_DEL);
}

s32int isdigit(s32int c)
{
  return c >= ASCII('0') && c <= ASCII('9');
}

s32int isgraph(s32int c)
{
  return isprint(c) && !isspace(c);
}

s32int islower(s32int c)
{
  return c >= ASCII('a') && c <= ASCII('z');
}

s32int isprint(s32int c)
{
  return c >= ASCII_PRINTABLE_BEGIN && c <= ASCII_PRINTABLE_END;
}

s32int ispunct(s32int c)
{
  switch (c)
  {
    /*
     * Punctuation range 1: 0x21..0x2F
     */
    case ASCII('!'):
    case ASCII('"'):
    case ASCII('#'):
    case ASCII('$'):
    case ASCII('%'):
    case ASCII('&'):
    case ASCII('\''):
    case ASCII('('):
    case ASCII(')'):
    case ASCII('*'):
    case ASCII('+'):
    case ASCII(','):
    case ASCII('-'):
    case ASCII('.'):
    case ASCII('/'):
    /*
     * Punctuation range 2: 0x3A..0x40
     */
    case ASCII(':'):
    case ASCII(';'):
    case ASCII('<'):
    case ASCII('='):
    case ASCII('>'):
    case ASCII('?'):
    case ASCII('@'):
    /*
     * Punctuation range 3: 0x5B..0x60
     */
    case ASCII('['):
    case ASCII('\\'):
    case ASCII(']'):
    case ASCII('^'):
    case ASCII('_'):
    case ASCII('`'):
    /*
     * Punctuation range 4: 0x7B..0x7E
     */
    case ASCII('{'):
    case ASCII('|'):
    case ASCII('}'):
    case ASCII('~'):
      return TRUE;
    default:
      return FALSE;
  }
}

s32int isspace(s32int c)
{
  switch (c)
  {
    case ASCII('\t'):
    case ASCII('\n'):
    case ASCII('\v'):
    case ASCII('\f'):
    case ASCII('\r'):
    case ASCII(' '):
      return TRUE;
    default:
      return FALSE;
  }
}

s32int isupper(s32int c)
{
  return c >= ASCII('A') && c <= ASCII('Z');
}

s32int isxdigit(s32int c)
{
  switch (c)
  {
    case ASCII('0'):
    case ASCII('1'):
    case ASCII('2'):
    case ASCII('3'):
    case ASCII('4'):
    case ASCII('5'):
    case ASCII('6'):
    case ASCII('7'):
    case ASCII('8'):
    case ASCII('9'):
    case ASCII('A'):
    case ASCII('B'):
    case ASCII('C'):
    case ASCII('D'):
    case ASCII('E'):
    case ASCII('F'):
    case ASCII('a'):
    case ASCII('b'):
    case ASCII('c'):
    case ASCII('d'):
    case ASCII('e'):
    case ASCII('f'):
      return TRUE;
    default:
      return FALSE;
  }
}

s32int tolower(s32int c)
{
  return isupper(c) ? c - ASCII('A') + ASCII('a') : c;
}

s32int toupper(s32int c)
{
  return islower(c) ? c - ASCII('a') + ASCII('A') : c;
}
