#ifndef __INSTRUCTION_EMU__TRANSLATOR__ARM__PC_HANDLERS_H__
#define __INSTRUCTION_EMU__TRANSLATOR__ARM__PC_HANDLERS_H__

#include "guestManager/translationStore.h"


/*
 * Data processing instructions
 */
void armDPImmRegRSR(TranslationStore *ts, BasicBlock *block, u32int pc, u32int instruction);
void armDPImmRegRSRNoDest(TranslationStore *ts, BasicBlock *block, u32int pc, u32int instruction);
void armMovPCInstruction(TranslationStore *ts, BasicBlock *block, u32int pc, u32int instruction);
void armShiftPCInstruction(TranslationStore *ts, BasicBlock *block, u32int pc, u32int instruction);

/*
 * Load / store instructions
 */
void armLdrPCInstruction(TranslationStore *ts, BasicBlock *block, u32int pc, u32int instruction);
void armStrPCInstruction(TranslationStore *ts, BasicBlock *block, u32int pc, u32int instruction);
void armStrtPCInstruction(TranslationStore *ts, BasicBlock *block, u32int pc, u32int instruction);

void armStmPCInstruction(TranslationStore *ts, BasicBlock *block, u32int pc, u32int instruction);

#endif /* __INSTRUCTION_EMU__TRANSLATOR__ARM__PC_HANDLERS_H__ */
