#ifndef __INSTRUCTION_EMU__INTERPRETER__ARM__DATA_PROCESS_PC_INSTRUCTIONS_H__
#define __INSTRUCTION_EMU__INTERPRETER__ARM__DATA_PROCESS_PC_INSTRUCTIONS_H__

#include "guestManager/translationCache.h"


/*
 * Bitwise operations
 */

u32int* armMvnPCInstruction(TranslationCache *tc, u32int *instructionAddr, u32int *currBlockCopyCacheAddr, u32int *blockCopyCacheStartAddress);

u32int* armBicPCInstruction(TranslationCache *tc, u32int *instructionAddr, u32int *currBlockCopyCacheAddr, u32int *blockCopyCacheStartAddress);


/*
 * Move and shift register operations
 */

u32int* armMovPCInstruction(TranslationCache *tc, u32int *instructionAddr, u32int *currBlockCopyCacheAddr, u32int *blockCopyCacheStartAddress);

u32int *armAsrPCInstruction(TranslationCache *tc, u32int *instructionAddr, u32int *currBlockCopyCacheAddr, u32int *blockCopyCacheStartAddress);
u32int *armLslrPCInstruction(TranslationCache *tc, u32int *instructionAddr, u32int *currBlockCopyCacheAddr, u32int *blockCopyCacheStartAddress);
u32int *armRorPCInstruction(TranslationCache *tc, u32int *instructionAddr, u32int *currBlockCopyCacheAddr, u32int *blockCopyCacheStartAddress);
u32int *armRrxPCInstruction(TranslationCache *tc, u32int *instructionAddr, u32int *currBlockCopyCacheAddr, u32int *blockCopyCacheStartAddress);

#endif /* __INSTRUCTION_EMU__INTERPRETER__ARM__DATA_PROCESS_INSTRUCTIONS_H__ */
