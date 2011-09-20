#ifndef __GUEST_BOOT__LINUX_H__
#define __GUEST_BOOT__LINUX_H__

#include "common/types.h"

#include "guestManager/guestContext.h"


void bootLinux(GCONTXT *context, u32int loadAddress, u32int initrdAddress)
  __attribute__((noreturn));

#endif
