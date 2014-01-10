#ifndef __INSTRUCTION_EMU__TRANSLATOR_H__
#define __INSTRUCTION_EMU__TRANSLATOR_H__

#include "common/types.h"

#include "guestManager/basicBlockStore.h"
#include "guestManager/guestContext.h"

void putBranch(u32int branchLocation, u32int branchTarget, u32int condition);

bool isBranch(u32int instruction);
bool branchLinks(u32int instruction);
bool isServiceCall(u32int instruction);
bool isConditional(u32int instruction);

u32int findBlockIndexNumber(GCONTXT *context, u32int hostPC);

u32int hostpcToGuestpc(GCONTXT* context);

#endif
