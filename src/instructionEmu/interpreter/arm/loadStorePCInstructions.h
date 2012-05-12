#ifndef __INSTRUCTION_EMU__INTERPRETER__ARM__LOAD_STORE_PC_INSTRUCTIONS_H__
#define __INSTRUCTION_EMU__INTERPRETER__ARM__LOAD_STORE_PC_INSTRUCTIONS_H__

#include "guestManager/translationCache.h"


u32int *armLdrStrPCInstruction(TranslationCache *tc, u32int *instructionAddr, u32int *currBlockCopyCacheAddr, u32int *blockCopyCacheStartAddress);

u32int *armStmPCInstruction(TranslationCache *tc, u32int *instructionAddr, u32int *currBlockCopyCacheAddr, u32int *blockCopyCacheStartAddress);

#endif
