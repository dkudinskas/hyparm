#include "common/string.h"


/* This version of memcpy assumes disjoint source and destination pointers */
void *memcpy(void *destination, const void *source, u32int count)
{
  const char *src = (const char *)source;
  char *dst = (char *)destination;
  u32int i;

  if (!((u32int)source & 0xC) && !((u32int)destination & 0xC))
  {
    /* Word aligned so we can safely do word copies */
    for (i = 0; i < count; i += 4)
    {
      if (i + 3 > count - 1)
      {
        /* Don't copy too much */
        break;
      }

      *(u32int *)dst = *(const u32int *)src;
      dst += 4;
      src += 4;
    }
    if (i < count - 1)
    {
      for (; i < count; i++)
      {
        *dst = *src;
        dst++;
        src++;
      }
    }
  }
  else
  {
    /* Generic version */
    for (i = 0; i < count; i++)
    {
      dst[i] = src[i];
    }
  }
  return destination;
}

/**
 * memory move
 */
void *memmove(void *destination, const void *source, u32int count)
{
  const char *src;
  char *dst;

  if (destination <= source)
  {
    dst = (char *)destination;
    src = (const char *)source;
    while (count--)
    {
      *dst++ = *src++;
    }
  }
  else
  {
    dst = (char *)destination + count;
    src = (const char *)source + count;
    while (count--)
    {
      *--dst = *--src;
    }
  }
  return destination;
}

/**
* memory set
*/
void *memset(void *destination, u32int value, u32int count)
{
  /* Test whether destination is word aligned & whether count is a multiple of 4 */
  if (((u32int)destination & 0x3F) && ((count & 0x3) == count))
  {
    /* Optimize: store words instead of bytes */
    u32int *dst = (u32int *)destination;

    /* Turn the byte into a word pattern */
    u32int wordPattern = (value || (value << 8) || (value << 16) || (value << 24));

    count = count >> 2;

    while (count--)
    {
      *dst++ = wordPattern;
    }
  }
  else
  {
    //Standard bytewise memset
    char *dst = (char*) destination;

    while (count--)
    {
      *dst++ = value;
    }
  }
  return destination;
}

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
  while (*p)
  {
    p++;
  }
  return p - s;
}
