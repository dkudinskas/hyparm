#ifndef __DATA_MOVE_INSTR__
#define __DATA_MOVE_INSTR__

#include "types.h"
#include "serial.h"
#include "guestContext.h"
#include "globalMemoryMapper.h"

// uncomment me for LOAD/STORE instruction trace: #define DATA_MOVE_TRACE

void invalidDataProcTrap(char * msg, GCONTXT * gc);

u32int strInstruction(GCONTXT * context);
u32int strbInstruction(GCONTXT * context);
u32int strhInstruction(GCONTXT * context);
u32int stmInstruction(GCONTXT * context);
u32int strdInstruction(GCONTXT * context);
u32int strexInstruction(GCONTXT * context);
u32int strexbInstruction(GCONTXT * context);
u32int strexdInstruction(GCONTXT * context);
u32int strexhInstruction(GCONTXT * context);

u32int ldrhInstruction(GCONTXT * context);
u32int ldrInstruction(GCONTXT * context);
u32int ldrdInstruction(GCONTXT * context);
u32int ldrbInstruction(GCONTXT * context);
u32int popLdrInstruction(GCONTXT * context);
u32int popLdmInstruction(GCONTXT * context);
u32int ldmInstruction(GCONTXT * context);
u32int ldrexInstruction(GCONTXT * context);
u32int ldrexbInstruction(GCONTXT * context);
u32int ldrexdInstruction(GCONTXT * context);
u32int ldrexhInstruction(GCONTXT * context);


#endif
