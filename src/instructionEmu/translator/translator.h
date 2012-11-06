#ifndef __INSTRUCTION_EMU__TRANSLATOR_H__
#define __INSTRUCTION_EMU__TRANSLATOR_H__

#include "common/types.h"

#include "guestManager/basicBlockStore.h"
#include "guestManager/guestContext.h"


void translate(GCONTXT* context, BasicBlock* block, DecodedInstruction* decoding, u32int instruction);

#endif
