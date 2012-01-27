#ifndef __RTOS_BOOT__BOOT_RTOS_H__
#define __RTOS_BOOT__BOOT_RTOS_H__

#include "common/types.h"


void doRtosBoot(ulong loadAddr)  __attribute__((noreturn));

#endif
