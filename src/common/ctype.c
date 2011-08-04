#include "common/ctype.h"

#define ASCII(c)    ((int) (unsigned char) (c))


int isalnum(int c)
{
  return isalpha(c) || isdigit(c);
}

int isalpha(int c)
{
  return islower(c) || isupper(c);
}

int iscntrl(int c)
{
  return (c >= ASCII_NUL && c <= ASCII_US) || (c == ASCII_DEL);
}

int isdigit(int c)
{
  return c >= ASCII('0') && c <= ASCII('9');
}

int isgraph(int c)
{
  return isprint(c) && !isspace(c);
}

int islower(int c)
{
  return c >= ASCII('a') && c <= ASCII('z');
}

int isprint(int c)
{
  return c >= ASCII_SPACE && c <= ASCII('~');
}

int ispunct(int c)
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
    return 1;
  default:
    return 0;
  }
}

int isspace(int c)
{
  switch (c)
  {
  case ASCII('\t'):
  case ASCII('\n'):
  case ASCII('\v'):
  case ASCII('\f'):
  case ASCII('\r'):
  case ASCII(' '):
    return 1;
  default:
    return 0;
  }
}

int isupper(int c)
{
  return c >= ASCII('A') && c <= ASCII('Z');
}

int isxdigit(int c)
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
    return 1;
  default:
    return 0;
  }
}

int tolower(int c)
{
  return isupper(c) ? c - ASCII('A') + ASCII('a') : c;
}

int toupper(int c)
{
  return islower(c) ? c - ASCII('a') + ASCII('A') : c;
}

#endif

