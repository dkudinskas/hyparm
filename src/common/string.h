#ifndef __COMMON__STRING_H__
#define __COMMON__STRING_H__

#include "common/types.h"


char *strcpy(char *dest, const char *src);

char *strncpy(char *dest, const char *src, s32int n);

s32int strcmp(const char *s1, const char *s2);

s32int strncmp(const char *s1, const char *s2, s32int n);

u32int strlen(const char *s);

#endif
