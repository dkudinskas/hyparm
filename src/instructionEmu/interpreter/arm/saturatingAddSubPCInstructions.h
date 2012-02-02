#ifndef __INSTRUCTION_EMU__INTERPRETER__ARM__SATURATING_ADD_SUB_PC_INSTRUCTIONS_H__
#define __INSTRUCTION_EMU__INTERPRETER__ARM__SATURATING_ADD_SUB_PC_INSTRUCTIONS_H__

#include "common/types.h"

#include "guestManager/guestContext.h"


u32int *armQaddPCInstruction(GCONTXT *context, u32int *instructionAddr, u32int *currBlockCopyCacheAddr, u32int *blockCopyCacheStartAddress);
u32int *armQsubPCInstruction(GCONTXT *context, u32int *instructionAddr, u32int *currBlockCopyCacheAddr, u32int *blockCopyCacheStartAddress);

u32int *armQdaddPCInstruction(GCONTXT *context, u32int *instructionAddr, u32int *currBlockCopyCacheAddr, u32int *blockCopyCacheStartAddress);
u32int *armQdsubPCInstruction(GCONTXT *context, u32int *instructionAddr, u32int *currBlockCopyCacheAddr, u32int *blockCopyCacheStartAddress);

#endif
