#ifndef __RTOS_BOOT__RTOS_LINUX_H__
#define __RTOS_BOOT__RTOS_LINUX_H__

#include "common/types.h"
#include "linuxBoot/bootLinux.h"

void doRtosBoot(ulong loadAddr)  __attribute__((noreturn));

#endif
