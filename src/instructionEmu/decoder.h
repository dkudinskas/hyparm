#ifndef __INSTRUCTION_EMU__DECODER_H__
#define __INSTRUCTION_EMU__DECODER_H__

#include "common/types.h"

struct guestContext;
struct TranslationStore;
struct BasicBlockEntry;

typedef u32int (*InstructionHandler)(struct guestContext *context, u32int instruction);

typedef void (*PCInstructionHandler)(struct TranslationStore *ts, struct BasicBlockEntry *block,
                                     u32int pc, u32int instruction);

typedef enum
{
  IRC_SAFE = 0,
  IRC_REPLACE = 1,
  IRC_REMOVE = 2,
  IRC_PATCH_PC = 4
} TranslateCode;

#ifdef CONFIG_DECODER_AUTO

typedef union
{
  void *barePtr;
  InstructionHandler handler;
  PCInstructionHandler pcHandler;
} AnyHandler;

TranslateCode decodeArmInstruction(u32int instruction, AnyHandler *handler);

#else

struct decodingTableEntry
{
  TranslateCode code;
  InstructionHandler handler;
  PCInstructionHandler pcHandler;
  u32int value;            /* If arch == 0 then value is a sentinel.  */
  u32int mask;             /* Recognise inst if (op & mask) == value.  */
  const char *instructionString; /* How to disassemble this insn.  */
};
typedef struct decodingTableEntry DecodedInstruction;

struct decodingTableEntry *decodeArmInstruction(u32int instruction);

#endif

#endif
