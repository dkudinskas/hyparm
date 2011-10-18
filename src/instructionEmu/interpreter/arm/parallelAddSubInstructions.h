#ifndef __INSTRUCTION_EMU__INTERPRETER__ARM__PARALLEL_ADD_SUB_INSTRUCTIONS_H__
#define __INSTRUCTION_EMU__INTERPRETER__ARM__PARALLEL_ADD_SUB_INSTRUCTIONS_H__

#include "common/types.h"

#include "guestManager/guestContext.h"


u32int armSadd16Instruction(GCONTXT *context, u32int instruction);
u32int armSadd8Instruction(GCONTXT *context, u32int instruction);
u32int armSasxInstruction(GCONTXT *context, u32int instruction);
u32int armSsaxInstruction(GCONTXT *context, u32int instruction);
u32int armSsub16Instruction(GCONTXT *context, u32int instruction);
u32int armSsub8Instruction(GCONTXT *context, u32int instruction);

u32int armQadd16Instruction(GCONTXT *context, u32int instruction);
u32int armQadd8Instruction(GCONTXT *context, u32int instruction);
u32int armQasxInstruction(GCONTXT *context, u32int instruction);
u32int armQsaxInstruction(GCONTXT *context, u32int instruction);
u32int armQsub16Instruction(GCONTXT *context, u32int instruction);
u32int armQsub8Instruction(GCONTXT *context, u32int instruction);

u32int armShadd16Instruction(GCONTXT *context, u32int instruction);
u32int armShadd8Instruction(GCONTXT *context, u32int instruction);
u32int armShasxInstruction(GCONTXT *context, u32int instruction);
u32int armShsaxInstruction(GCONTXT *context, u32int instruction);
u32int armShsub16Instruction(GCONTXT *context, u32int instruction);
u32int armShsub8Instruction(GCONTXT *context, u32int instruction);

u32int armUadd16Instruction(GCONTXT *context, u32int instruction);
u32int armUadd8Instruction(GCONTXT *context, u32int instruction);
u32int armUasxInstruction(GCONTXT *context, u32int instruction);
u32int armUsaxInstruction(GCONTXT *context, u32int instruction);
u32int armUsub16Instruction(GCONTXT *context, u32int instruction);
u32int armUsub8Instruction(GCONTXT *context, u32int instruction);

u32int armUqadd16Instruction(GCONTXT *context, u32int instruction);
u32int armUqadd8Instruction(GCONTXT *context, u32int instruction);
u32int armUqasxInstruction(GCONTXT *context, u32int instruction);
u32int armUqsaxInstruction(GCONTXT *context, u32int instruction);
u32int armUqsub16Instruction(GCONTXT *context, u32int instruction);
u32int armUqsub8Instruction(GCONTXT *context, u32int instruction);

u32int armUhadd16Instruction(GCONTXT *context, u32int instruction);
u32int armUhadd8Instruction(GCONTXT *context, u32int instruction);
u32int armUhasxInstruction(GCONTXT *context, u32int instruction);
u32int armUhsaxInstruction(GCONTXT *context, u32int instruction);
u32int armUhsub16Instruction(GCONTXT *context, u32int instruction);
u32int armUhsub8Instruction(GCONTXT *context, u32int instruction);

#endif /* __INSTRUCTION_EMU__INTERPRETER__ARM__PARALLEL_MEDIA_INSTRUCTIONS_H__ */
