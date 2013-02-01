#ifndef __GUEST_BOOT__LINUX_H__
#define __GUEST_BOOT__LINUX_H__

#include "common/compiler.h"
#include "common/types.h"

#include "guestManager/guestContext.h"


void bootLinux(GCONTXT *context, u32int loadAddress, u32int initrdAddress, const char *arguments)
  __cold__ __attribute__((noreturn));

#endif
