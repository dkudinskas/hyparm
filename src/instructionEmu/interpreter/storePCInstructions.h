#ifndef __INSTRUCTION_EMU__EMULATOR__STORE_PC_INSTRUCTIONS_H__
#define __INSTRUCTION_EMU__EMULATOR__STORE_PC_INSTRUCTIONS_H__

#include "common/types.h"

#include "guestManager/guestContext.h"


u32int *armStrPCInstruction(GCONTXT *context, u32int *instructionAddr, u32int *currBlockCopyCacheAddr, u32int *blockCopyCacheStartAddress);
u32int *armStrbPCInstruction(GCONTXT *context, u32int *instructionAddr, u32int *currBlockCopyCacheAddr, u32int *blockCopyCacheStartAddress);
u32int *armStrhPCInstruction(GCONTXT *context, u32int *instructionAddr, u32int *currBlockCopyCacheAddr, u32int *blockCopyCacheStartAddress);
u32int *armStrdPCInstruction(GCONTXT *context, u32int *instructionAddr, u32int *currBlockCopyCacheAddr, u32int *blockCopyCacheStartAddress);

u32int *armStrhtPCInstruction(GCONTXT *context, u32int *instructionAddr, u32int *currBlockCopyCacheAddr, u32int *blockCopyCacheStartAddress);

u32int *armStrexPCInstruction(GCONTXT *context, u32int *instructionAddr, u32int *currBlockCopyCacheAddr, u32int *blockCopyCacheStartAddress);
u32int *armStrexbPCInstruction(GCONTXT *context, u32int *instructionAddr, u32int *currBlockCopyCacheAddr, u32int *blockCopyCacheStartAddress);
u32int *armStrexhPCInstruction(GCONTXT *context, u32int *instructionAddr, u32int *currBlockCopyCacheAddr, u32int *blockCopyCacheStartAddress);
u32int *armStrexdPCInstruction(GCONTXT *context, u32int *instructionAddr, u32int *currBlockCopyCacheAddr, u32int *blockCopyCacheStartAddress);

u32int *armStmPCInstruction(GCONTXT *context, u32int *instructionAddr, u32int *currBlockCopyCacheAddr, u32int *blockCopyCacheStartAddress);

#endif
