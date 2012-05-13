#ifndef __INSTRUCTION_EMU__INTERPRETER__ARM__PC_HANDLERS_H__
#define __INSTRUCTION_EMU__INTERPRETER__ARM__PC_HANDLERS_H__

#include "guestManager/translationCache.h"


/*
 * Data processing instructions
 */
u32int *armDPImmRegRSR(TranslationCache *tc, u32int *code, u32int pc, u32int instruction);
u32int *armDPImmRegRSRNoDest(TranslationCache *tc, u32int *code, u32int pc, u32int instruction);
u32int* armMovPCInstruction(TranslationCache *tc, u32int *code, u32int pc, u32int instruction);
u32int *armShiftPCInstruction(TranslationCache *tc, u32int *code, u32int pc, u32int instruction);

/*
 * Load / store instructions
 */
u32int *armLdrStrPCInstruction(TranslationCache *tc, u32int *code, u32int pc, u32int instruction);
u32int *armStmPCInstruction(TranslationCache *tc, u32int *code, u32int pc, u32int instruction);

#endif /* __INSTRUCTION_EMU__INTERPRETER__ARM__PC_HANDLERS_H__ */
