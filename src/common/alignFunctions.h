#ifndef __COMMON__ALIGN_FUNCTIONS_H__
#define __COMMON__ALIGN_FUNCTIONS_H__

#include "common/types.h"


u32int uaLoadWord(char bytes[]);

u16int uaLoadHWord(char bytes[]);

u32int uaLoadWordNoSwp(char bytes[]);

#endif
