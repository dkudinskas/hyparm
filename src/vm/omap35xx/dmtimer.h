#ifndef __VM__OMAP_35XX__DMTIMER_H__
#define __VM__OMAP_35XX__DMTIMER_H__

#include "common/types.h"

#include "vm/types.h"


u32int loadDmTimer(GCONTXT *context, ACCESS_SIZE size, u32int virtAddr, u32int phyAddr);
void storeDmTimer(GCONTXT *context, ACCESS_SIZE size, u32int virtAddr, u32int phyAddr, u32int value);


#endif /* __VM__OMAP_35XX__DMTIMER_H__ */

