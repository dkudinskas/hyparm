#include "common/ctype.h"
#include "common/stringFunctions.h"


int stringncmp(char * str1, char * str2, int n)
{
  int index = 0;
  while (index < n)
  {
    if (str1[index] != str2[index])
    {
      return -1;
    }
    index++;
  }
  return 0;
}

u32int strtoi(char * str)
{
  int  length = 8;
  int  bitsInLong = 32;
  int  index = 0;
  char digitChar = 0;
  ulong digitInt = 0;
  ulong retVal = 0;

  while (index < length)
  {
    digitChar = str[index];
    if (isdigit(digitChar))
    {
      digitInt |= digitChar - 0x30;
      digitInt = digitInt << ( bitsInLong - ((index + 1) * 4) );
      retVal = retVal | digitInt;
    }
    else
    {
      return -1;
    }
    index = index + 1;
  } // while ends
  return retVal;
}

/**
 * strlen - Find the length of a string
 */
u32int stringlen(char * s)
{
  char *sc;

  for (sc = s; *sc != '\0'; ++sc)
  {
    // do nothing
  }
  return sc - s;
}

/**
 * strcpy - Copy a NULL terminated string
 */
char * stringcpy(char * dest, char *src)
{
  char *tmp = dest;

  while ((*dest++ = *src++) != '\0')
  {
    // do nothing
  }
  return tmp;
}

/**
 * strcmp - compare strings
 */
int strcmp(const char *s1, const char *s2)
{
  for(; *s1 == *s2; ++s1, ++s2)
  {
    if (*s1 == '\0')
    {
      return 0;
    }
  }
  return *(const unsigned char *)s1 - *(const unsigned char *)s2;
}
