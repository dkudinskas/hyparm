#ifndef __VM__OMAP_35XX__DMTIMER_H__
#define __VM__OMAP_35XX__DMTIMER_H__

#include "common/compiler.h"
#include "common/types.h"

#include "vm/omap35xx/hardwareLibrary.h"


void initDmTimer(void) __cold__;

/* top load function */
u32int loadDmTimer(device * dev, ACCESS_SIZE size, u32int virtAddr, u32int phyAddr);

/* top store function */
void storeDmTimer(device * dev, ACCESS_SIZE size, u32int virtAddr, u32int phyAddr, u32int value);


#endif /* __VM__OMAP_35XX__DMTIMER_H__ */

