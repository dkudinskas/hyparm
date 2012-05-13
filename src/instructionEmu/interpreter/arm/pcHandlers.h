#ifndef __INSTRUCTION_EMU__INTERPRETER__ARM__PC_HANDLERS_H__
#define __INSTRUCTION_EMU__INTERPRETER__ARM__PC_HANDLERS_H__

#include "guestManager/translationCache.h"


/*
 * Data processing instructions
 */
u32int *armDPImmRegRSR(TranslationCache *tc, u32int *instructionAddr, u32int *currBlockCopyCacheAddr, u32int *blockCopyCacheStartAddress);
u32int *armDPImmRegRSRNoDest(TranslationCache *tc, u32int *instructionAddr, u32int *currBlockCopyCacheAddr, u32int *blockCopyCacheStartAddress);
u32int* armMovPCInstruction(TranslationCache *tc, u32int *instructionAddr, u32int *currBlockCopyCacheAddr, u32int *blockCopyCacheStartAddress);
u32int *armShiftPCInstruction(TranslationCache *tc, u32int *instructionAddr, u32int *currBlockCopyCacheAddr, u32int *blockCopyCacheStartAddress);

/*
 * Load / store instructions
 */
u32int *armLdrStrPCInstruction(TranslationCache *tc, u32int *instructionAddr, u32int *currBlockCopyCacheAddr, u32int *blockCopyCacheStartAddress);
u32int *armStmPCInstruction(TranslationCache *tc, u32int *instructionAddr, u32int *currBlockCopyCacheAddr, u32int *blockCopyCacheStartAddress);

#endif /* __INSTRUCTION_EMU__INTERPRETER__ARM__DATA_PROCESS_INSTRUCTIONS_H__ */
