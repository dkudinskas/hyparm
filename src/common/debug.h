#ifndef __COMMON_DEBUG_H_
#define __COMMON_DEBUG_H_

#include "types.h"
#include "guestContext.h"

__attribute((noreturn)) void DIE_NOW(GCONTXT * context, char* msg);

#endif