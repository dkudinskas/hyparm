#ifndef __GUEST_BOOT__FREE_RTOS_H__
#define __GUEST_BOOT__FREE_RTOS_H__

#include "common/compiler.h"
#include "common/types.h"

#include "guestManager/guestContext.h"


void bootFreeRtos(GCONTXT *context, u32int loadAddress) __cold__ __attribute__((noreturn));

#endif
