#ifndef __STRING_FUNCTIONS_H__
#define __STRING_FUNCTIONS_H__

#include "types.h"

int stringncmp(char * str1, char * str2, int n);
ulong stringToLong(char * str);
u32int stringlen(char * s);
char * stringcpy(char * dest, char *src);

#endif