#ifndef __COMMON__DEBUG_H__
#define __COMMON__DEBUG_H__

#include "common/types.h"

#include "guestManager/guestContext.h"


//#define DIE_NOW_SCANNER_COUNTER

__attribute((noreturn)) void DIE_NOW(GCONTXT * context, char* msg);

#endif
