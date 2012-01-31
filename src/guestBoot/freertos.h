#ifndef __GUEST_BOOT__FREE_RTOS_H__
#define __GUEST_BOOT__FREE_RTOS_H__ 1

#include "common/types.h"

#include "guestManager/guestContext.h"


void bootFreeRtos(GCONTXT *context, u32int loadAddress)  __attribute__((noreturn));

#endif
