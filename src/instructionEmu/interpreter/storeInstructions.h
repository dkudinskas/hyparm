#ifndef __INSTRUCTION_EMU__EMULATOR__STORE_INSTRUCTIONS_H__
#define __INSTRUCTION_EMU__EMULATOR__STORE_INSTRUCTIONS_H__

#include "common/types.h"

#include "guestManager/guestContext.h"


// uncomment me for LOAD/STORE instruction trace: #define DATA_MOVE_TRACE


u32int armStrInstruction(GCONTXT *context, u32int instruction);
u32int armStrbInstruction(GCONTXT *context, u32int instruction);
u32int armStrhInstruction(GCONTXT *context, u32int instruction);
u32int armStrdInstruction(GCONTXT *context, u32int instruction);

u32int armStrhtInstruction(GCONTXT *context, u32int instruction);

u32int armStrexInstruction(GCONTXT *context, u32int instruction);
u32int armStrexbInstruction(GCONTXT *context, u32int instruction);
u32int armStrexhInstruction(GCONTXT *context, u32int instruction);
u32int armStrexdInstruction(GCONTXT *context, u32int instruction);

u32int armStmInstruction(GCONTXT *context, u32int instruction);


u32int t16StrInstruction(GCONTXT *context, u32int instruction);
u32int t16StrSpInstruction(GCONTXT *context, u32int instruction);
u32int t16StrbInstruction(GCONTXT *context, u32int instruction);
u32int t16StrhInstruction(GCONTXT *context, u32int instruction);

u32int t16PushInstruction(GCONTXT *context, u32int instruction);


u32int t32StrbInstruction(GCONTXT *context, u32int instruction);
u32int t32StrhImmediateInstruction(GCONTXT *context, u32int instruction);
u32int t32StrhRegisterInstruction(GCONTXT *context, u32int instruction);
u32int t32StrdImmediateInstruction(GCONTXT *context, u32int instruction);

u32int t32StrhtInstruction(GCONTXT *context, u32int instruction);

#endif /* __INSTRUCTION_EMU__EMULATOR__STORE_INSTRUCTIONS_H__ */
