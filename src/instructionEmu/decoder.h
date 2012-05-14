#ifndef __INSTRUCTION_EMU__DECODER_H__
#define __INSTRUCTION_EMU__DECODER_H__

#include "common/types.h"


struct armTranslationInfo;
struct guestContext;
struct translationCache;

typedef u32int (*InstructionHandler)(struct guestContext *context, u32int instruction);

typedef void (*PCInstructionHandler)(struct translationCache *tc, struct armTranslationInfo *block,
                                     u32int pc, u32int instruction);

typedef enum
{
  IRC_SAFE = 0,
  IRC_REPLACE = 1,
  IRC_LS_USER = 2
} instructionReplaceCode;


#ifndef CONFIG_BLOCK_COPY

instructionReplaceCode decodeArmInstruction(u32int instruction, InstructionHandler *handler);

#ifdef CONFIG_THUMB2

instructionReplaceCode decodeThumbInstruction(u32int instruction, InstructionHandler *handler);

#endif /* CONFIG_THUMB2 */

#else

struct decodingTableEntry
{
  instructionReplaceCode replace;
  InstructionHandler handler;
  PCInstructionHandler pcHandler;
  u32int value;            /* If arch == 0 then value is a sentinel.  */
  u32int mask;             /* Recognise inst if (op & mask) == value.  */
  const char *instructionString; /* How to disassemble this insn.  */
};


struct decodingTableEntry *decodeArmInstruction(u32int instruction);

#endif /* CONFIG_BLOCK_COPY */

#endif
