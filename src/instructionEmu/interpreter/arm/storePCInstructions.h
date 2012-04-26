#ifndef __INSTRUCTION_EMU__INTERPRETER__ARM__STORE_PC_INSTRUCTIONS_H__
#define __INSTRUCTION_EMU__INTERPRETER__ARM__STORE_PC_INSTRUCTIONS_H__

#include "guestManager/translationCache.h"


u32int *armStrPCInstruction(TranslationCache *tc, u32int *instructionAddr, u32int *currBlockCopyCacheAddr, u32int *blockCopyCacheStartAddress);
u32int *armStrbPCInstruction(TranslationCache *tc, u32int *instructionAddr, u32int *currBlockCopyCacheAddr, u32int *blockCopyCacheStartAddress);
u32int *armStrhPCInstruction(TranslationCache *tc, u32int *instructionAddr, u32int *currBlockCopyCacheAddr, u32int *blockCopyCacheStartAddress);
u32int *armStrdPCInstruction(TranslationCache *tc, u32int *instructionAddr, u32int *currBlockCopyCacheAddr, u32int *blockCopyCacheStartAddress);

u32int *armStmPCInstruction(TranslationCache *tc, u32int *instructionAddr, u32int *currBlockCopyCacheAddr, u32int *blockCopyCacheStartAddress);

#endif
