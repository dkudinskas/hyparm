#ifndef __INSTRUCTION_EMU__INTERPRETER__ARM__LOAD_PC_INSTRUCTIONS_H__
#define __INSTRUCTION_EMU__INTERPRETER__ARM__LOAD_PC_INSTRUCTIONS_H__

#include "guestManager/translationCache.h"


u32int *armLdrPCInstruction(TranslationCache *tc, u32int *instructionAddr, u32int *currBlockCopyCacheAddr, u32int *blockCopyCacheStartAddress);
u32int *armLdrbPCInstruction(TranslationCache *tc, u32int *instructionAddr, u32int *currBlockCopyCacheAddr, u32int *blockCopyCacheStartAddress);
u32int *armLdrhPCInstruction(TranslationCache *tc, u32int *instructionAddr, u32int *currBlockCopyCacheAddr, u32int *blockCopyCacheStartAddress);
u32int *armLdrdPCInstruction(TranslationCache *tc, u32int *instructionAddr, u32int *currBlockCopyCacheAddr, u32int *blockCopyCacheStartAddress);

u32int *armPopLdmPCInstruction(TranslationCache *tc, u32int *instructionAddr, u32int *currBlockCopyCacheAddr, u32int *blockCopyCacheStartAddress);

#endif
