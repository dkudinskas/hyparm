#ifndef __INSTRUCTION_EMU__TRANSLATOR_H__
#define __INSTRUCTION_EMU__TRANSLATOR_H__

#include "common/types.h"

#include "guestManager/basicBlockStore.h"
#include "guestManager/guestContext.h"

void putBranch(u32int branchLocation, u32int branchTarget, u32int condition);

bool isBranch(Instruction instr);
bool branchLinks(Instruction instr);
bool isServiceCall(Instruction instr);
bool isConditional(Instruction instr);

u32int findBlockIndexNumber(GCONTXT *context, u32int hostPC);

u32int hostpcToGuestpc(GCONTXT* context);

#endif
