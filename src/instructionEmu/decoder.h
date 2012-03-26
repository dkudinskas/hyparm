#ifndef __INSTRUCTION_EMU__DECODER_H__
#define __INSTRUCTION_EMU__DECODER_H__

#include "common/types.h"

#include "guestManager/guestContext.h"

struct TopLevelCategory
{
  u32int mask;             /* Recognise if (instr & mask) == value.  */
  u32int value;
  struct instruction32bit *table;
};

struct instruction32bit
{
  s16int replace;
  instructionHandler handler;
  u32int value;            /* If arch == 0 then value is a sentinel.  */
  u32int mask;             /* Recognise inst if (op & mask) == value.  */
  const char *instructionString; /* How to disassemble this insn.  */
};
typedef struct instruction32bit armInstruction; 

armInstruction* decodeArmInstruction(u32int instruction);


#ifdef CONFIG_THUMB2

instructionHandler decodeThumbInstruction(u32int instruction);

#endif /* CONFIG_THUMB2 */

#endif
