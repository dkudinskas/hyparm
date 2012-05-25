#ifndef __INSTRUCTION_EMU__TRANSLATOR__ARM__PC_HANDLERS_H__
#define __INSTRUCTION_EMU__TRANSLATOR__ARM__PC_HANDLERS_H__

#include "instructionEmu/translationInfo.h"


/*
 * Data processing instructions
 */
void armDPImmRegRSR(TranslationCache *tc, ARMTranslationInfo *block, u32int pc, u32int instruction);
void armDPImmRegRSRNoDest(TranslationCache *tc, ARMTranslationInfo *block, u32int pc, u32int instruction);
void armMovPCInstruction(TranslationCache *tc, ARMTranslationInfo *block, u32int pc, u32int instruction);
void armShiftPCInstruction(TranslationCache *tc, ARMTranslationInfo *block, u32int pc, u32int instruction);

/*
 * Load / store instructions
 */
void armLdrPCInstruction(TranslationCache *tc, ARMTranslationInfo *block, u32int pc, u32int instruction);
void armStrPCInstruction(TranslationCache *tc, ARMTranslationInfo *block, u32int pc, u32int instruction);
void armStmPCInstruction(TranslationCache *tc, ARMTranslationInfo *block, u32int pc, u32int instruction);

#endif /* __INSTRUCTION_EMU__TRANSLATOR__ARM__PC_HANDLERS_H__ */
