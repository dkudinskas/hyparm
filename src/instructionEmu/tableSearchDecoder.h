#ifndef __INSTRUCTION_EMU__TABLE_SEARCH_DECODER_H__
#define __INSTRUCTION_EMU__TABLE_SEARCH_DECODER_H__

#include "common/types.h"


// Uncomment me for decoder debug: #define DECODER_DEBUG  

#define DECODER_DEBUG

#define UNDEFINED_INSTRUCTION            0x0

#define UNDEFINED_CATEGORY               0x0
#define DATA_PROC_AND_MISC_CATEGORY      0x1
#define LOAD_STORE_WORD_BYTE_CATEGORY    0x2
#define MEDIA_CATEGORY                   0x3
#define BRANCH_BLOCK_TRANSFER_CATEGORY   0x4
#define SVC_COPROCESSOR_CATEGORY         0x5
#define UNCONDITIONALS_CATEGORY          0x6

struct TopLevelCategory
{
  u32int categoryCode;
  u32int mask;             /* Recognise if (instr & mask) == value.  */
  u32int value;
};


struct instruction32bit
{
  s16int replaceCode;
  instructionHandler hdlFunct;
  u32int value;            /* If arch == 0 then value is a sentinel.  */
  u32int mask;             /* Recognise inst if (op & mask) == value.  */
  const char * instructionString; /* How to disassemble this insn.  */
};

struct instruction32bit * decodeInstr(u32int instr);

u32int decodeTopLevelCategory(u32int instr);

struct instruction32bit * decodeDataProcMisc(u32int instr);
struct instruction32bit * decodeLoadStoreWordByte(u32int instr);
struct instruction32bit * decodeMedia(u32int instr);
struct instruction32bit * decodeBranchBlockTransfer(u32int instr);
struct instruction32bit * decodeSvcCoproc(u32int instr);
struct instruction32bit * decodeUnconditional(u32int instr);

void dumpInstruction(char * msg, u32int instr);

#endif
