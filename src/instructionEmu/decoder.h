#ifndef __INSTRUCTION_EMU__DECODER_H__
#define __INSTRUCTION_EMU__DECODER_H__

#include "common/types.h"

#include "guestManager/guestContext.h"


#ifndef CONFIG_BLOCK_COPY

typedef enum
{
  IRC_SAFE = 0,
  IRC_REPLACE = 1
} instructionReplaceCode;


instructionReplaceCode decodeArmInstruction(u32int instruction, instructionHandler *handler);

#ifdef CONFIG_THUMB2

instructionReplaceCode decodeThumbInstruction(u32int instruction, instructionHandler *handler);

#endif /* CONFIG_THUMB2 */

#else

struct decodingTableEntry
{
  s16int replace;
  instructionHandler handler;
  pcInstructionHandler PCFunct;
  u32int value;            /* If arch == 0 then value is a sentinel.  */
  u32int mask;             /* Recognise inst if (op & mask) == value.  */
  const char *instructionString; /* How to disassemble this insn.  */
};


struct decodingTableEntry *decodeArmInstruction(u32int instruction);

#endif /* CONFIG_BLOCK_COPY */

#endif
