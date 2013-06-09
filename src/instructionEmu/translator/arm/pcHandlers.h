#ifndef __INSTRUCTION_EMU__TRANSLATOR__ARM__PC_HANDLERS_H__
#define __INSTRUCTION_EMU__TRANSLATOR__ARM__PC_HANDLERS_H__

#include "guestManager/translationStore.h"


/*
 * Data processing instructions
 */
void armALUImmRegRSR(TranslationStore *ts, BasicBlock *block, u32int pc, u32int instruction);
void armALUImmRegRSRNoDest(TranslationStore *ts, BasicBlock *block, u32int pc, u32int instruction);

void armALUimm(TranslationStore *ts, BasicBlock *block, u32int pc, u32int instruction);
void armALUimmNoDest(TranslationStore* ts, BasicBlock *block, u32int pc, u32int instruction);
void armALUreg(TranslationStore *ts, BasicBlock *block, u32int pc, u32int instruction);
void armALUregNoDest(TranslationStore* ts, BasicBlock *block, u32int pc, u32int instruction);
void armMovPCInstruction(TranslationStore *ts, BasicBlock *block, u32int pc, u32int instruction);
void armShiftPCImm(TranslationStore* ts, BasicBlock *block, u32int pc, u32int instruction);

/*
 * Load / store instructions
 */
void armLdrPCReg(TranslationStore* ts, BasicBlock *block, u32int pc, u32int instruction);
void armLdrPCImm(TranslationStore* ts, BasicBlock *block, u32int pc, u32int instruction);
void armLdrdhPCInstruction(TranslationStore* ts, BasicBlock *block, u32int pc, u32int instruction);
void armLdrPCInstruction(TranslationStore *ts, BasicBlock *block, u32int pc, u32int instruction);
void armStrPCInstruction(TranslationStore *ts, BasicBlock *block, u32int pc, u32int instruction);
void armStrtPCInstruction(TranslationStore *ts, BasicBlock *block, u32int pc, u32int instruction);

void armStmPC(TranslationStore *ts, BasicBlock *block, u32int pc, u32int instruction);

#endif /* __INSTRUCTION_EMU__TRANSLATOR__ARM__PC_HANDLERS_H__ */
