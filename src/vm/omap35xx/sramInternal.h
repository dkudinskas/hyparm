#ifndef __VM__OMAP_35XX__SRAM_INTERNAL_H__
#define __VM__OMAP_35XX__SRAM_INTERNAL_H__

#include "common/types.h"

#include "guestManager/guestContext.h"

#include "vm/types.h"


u32int loadSramInternal(GCONTXT *context, device *dev, ACCESS_SIZE size, u32int virtAddr, u32int phyAddr);
void storeSramInternal(GCONTXT *context, device *dev, ACCESS_SIZE size, u32int virtAddr, u32int phyAddr, u32int value);

#endif

