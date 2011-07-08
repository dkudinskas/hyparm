#ifndef __INSTRUCTION_EMU__ASM_DIS_H__
#define __INSTRUCTION_EMU__ASM_DIS_H__

#include "common/types.h"

#include "guestManager/guestContext.h"

#define INDEX_OF(x)	(sizeof(x)/sizeof(x[0]))

#define UNDEFINED_INSTRUCTION    0x0

struct opcode32
{
  s16int replaceCode;
  u32int (*hdlFunct)(GCONTXT * context);
  u32int value;            /* If arch == 0 then value is a sentinel.  */
  u32int mask;             /* Recognise insn if (op & mask) == value.  */
  const char * instructionString; /* How to disassemble this insn.  */
};

struct opcode32 * decodeInstruction(u32int instr);

void dumpInstrString(GCONTXT * context, u32int instr);

#endif
