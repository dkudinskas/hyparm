#ifndef __VM__OMAP35XX__CONTROL_MODULE_H__
#define __VM__OMAP35XX__CONTROL_MODULE_H__

#include "common/compiler.h"
#include "common/types.h"

#include "vm/types.h"


u32int loadControlModule(GCONTXT *context, ACCESS_SIZE size, u32int virtAddr, u32int phyAddr);
void storeControlModule(GCONTXT *context, ACCESS_SIZE size, u32int virtAddr, u32int phyAddr, u32int value);

#endif /* __VM__OMAP35XX__CONTROL_MODULE_H__ */
