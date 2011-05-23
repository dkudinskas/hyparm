#ifndef __ALIGNFUNCS_H__
#define __ALIGNFUNCS_H__

#include "types.h"


u32int uaLoadWord(char bytes[]);

u16int uaLoadHWord(char bytes[]);

u32int uaLoadWordNoSwp(char bytes[]);

#endif
