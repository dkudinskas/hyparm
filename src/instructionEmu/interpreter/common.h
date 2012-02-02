#ifndef __INSTRUCTION_EMU__INTERPRETER__COMMON_H__
#define __INSTRUCTION_EMU__INTERPRETER__COMMON_H__

#include "common/types.h"

#include "guestManager/guestContext.h"


u32int nopInstruction(GCONTXT *context, u32int instruction);

u32int svcInstruction(GCONTXT *context, u32int instruction);

u32int undefinedInstruction(GCONTXT *context, u32int instruction);


#ifdef CONFIG_BLOCK_COPY

u32int *nopPCInstruction(GCONTXT *context, u32int *instructionAddr, u32int *currBlockCopyCacheAddr, u32int *blockCopyCacheStartAddress);

u32int* svcPCInstruction(GCONTXT *context, u32int *instructionAddr, u32int *currBlockCopyCacheAddr, u32int *blockCopyCacheStartAddress);

u32int* undefinedPCInstruction(GCONTXT *context, u32int *instructionAddr, u32int *currBlockCopyCacheAddr, u32int *blockCopyCacheStartAddress);

#endif /* CONFIG_BLOCK_COPY */

#endif /* __INSTRUCTION_EMU__INTERPRETER__COMMON_H__ */
