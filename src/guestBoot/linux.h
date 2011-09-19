#ifndef __GUEST_BOOT__LINUX_H__
#define __GUEST_BOOT__LINUX_H__ 1

#include "common/types.h"

#include "guestManager/guestContext.h"

#include "guestBoot/image.h"


void bootLinux(GCONTXT *context, image_header_t *imageHeader, u32int loadAddress, u32int initrdAddress)
  __attribute__((noreturn));

#endif
