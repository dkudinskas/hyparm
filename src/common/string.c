#include "common/string.h"


/**
 * strcpy - Copy a NULL terminated string
 */
char *strcpy(char *dest, const char *src)
{
  char *p = dest;
  while ((*p++ = *src++));
  return dest;
}

/**
 * strcmp - compare strings
 */
int strcmp(const char *s1, const char *s2)
{
  for(; *s1 == *s2; ++s1, ++s2)
  {
    if (!*s1)
    {
      return 0;
    }
  }
  return *(const uchar *)s1 - *(const uchar *)s2;
}

int strncmp(const char *s1, const char *s2, s32int n)
{
  s32int index;
  for (index = 0; index < n; ++index)
  {
    if (s1[index] != s2[index])
    {
      return *(const uchar *)s1 - *(const uchar *)s2;
    }
  }
  return 0;
}

/**
 * strlen - Find the length of a string
 */
u32int strlen(const char *s)
{
  const char *p = s;
  while (*p++)
  {
   /*
    * Do nothing as long as p points to a non-null character.
    */
  }
  return p - s;
}
