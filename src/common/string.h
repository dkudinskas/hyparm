#ifndef __COMMON__STRING_H__
#define __COMMON__STRING_H__

#include "common/compiler.h"
#include "common/types.h"


void *memcpy(void *destination, const void *source, u32int count);

void *memmove(void *destination, const void *source, u32int count);

void *memset(void *destination, u32int value, u32int count);

char *strcpy(char *dest, const char *src);

char *strncpy(char *dest, const char *src, s32int n);

s32int strcmp(const char *s1, const char *s2) __pure__;

s32int strncmp(const char *s1, const char *s2, s32int n) __pure__;

u32int strlen(const char *s) __pure__;

#endif
