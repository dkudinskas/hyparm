#ifndef __INSTRUCTION_EMU__TABLE_SEARCH_DECODER_H__
#define __INSTRUCTION_EMU__TABLE_SEARCH_DECODER_H__

#include "common/types.h"


// Uncomment me for decoder debug: #define DECODER_DEBUG  

#define UNDEFINED_INSTRUCTION            0x0

#define UNDEFINED_CATEGORY               0x0
#define DATA_PROC_AND_MISC_CATEGORY      0x1
#define LOAD_STORE_WORD_BYTE_CATEGORY    0x2
#define MEDIA_CATEGORY                   0x3
#define BRANCH_BLOCK_TRANSFER_CATEGORY   0x4
#define SVC_COPROCESSOR_CATEGORY         0x5
#define UNCONDITIONALS_CATEGORY          0x6


#define T16_UNCONDITIONALS_CATEGORY			0x0
#define T16_CONDITIONAL_BRANCH_AND_SVC_CALL	0x1
#define T16_LOAD_MULTIPLE_REGISTERS			0x2
#define T16_STORE_MULTIPLE_REGISTERS		0x3
#define T16_MISC							0x4
#define T16_SP_ADDR							0x5
#define	T16_PC_ADDR							0x6
#define T16_LOAD_STORE						0x7
#define T16_LDR								0x8
#define T16_SPECIAL_AND_BRANCH_EXCHANGE		0x9
#define T16_DATA_PROC						0xA
#define T16_SHIFT							0xB

#define T32_LOAD_STORE_MULTIPLE				0x0
#define T32_LOAD_STORE_DOUBLE_EXCLUSIVE		0x1
#define T32_DATA_PROC						0x2
#define T32_COPROC							0x3
#define T32_BRANCH_AND_MISC					0x4
#define T32_STORE_SINGLE					0x5
#define T32_SIMD_STRUCT_LOAD_STORE			0x6
#define T32_LOAD_BYTE						0x7
#define T32_LOAD_HALFWORD					0x8
#define T32_LOAD_WORD						0x9
#define T32_UNDEFINED						0xA
#define T32_MULTIPLY						0xB
#define T32_LONG_MULTIPLY					0xC


//#define DECODER_DEBUG

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

struct instruction16bit
{
	s8int replaceCode;    /* SVC on 16-bit omits the last 8 bits */
	instructionHandler hdlFunct;
	u16int	value;
	u16int	mask;
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

// Thumb-2 32-bit instructions
struct instruction32bit * t32decodeLoadStoreMultiple(u32int instr);
struct instruction32bit * t32decodeLoadStoreDoubleExclusive(u32int instr);
struct instruction32bit * t32decodeDataProc(u32int instr);
struct instruction32bit * t32decodeCoproc(u32int instr);
struct instruction32bit * t32decodeBranchMisc(u32int instr);
struct instruction32bit * t32decodeStoreSingle(u32int instr);
struct instruction32bit * t32decodeSimdStructLoadStore(u32int instr);
struct instruction32bit * t32decodeLoadByte(u32int instr);
struct instruction32bit * t32decodeLoadHalfWord(u32int instr);
struct instruction32bit * t32decodeLoadWord(u32int instr);
struct instruction32bit * t32decodeMultiply(u32int instr);
struct instruction32bit * t32decodeLongMultiply(u32int instr);

// Thumb-2 16-bit instructions
struct instruction32bit * t16decodeUnconditionals(u16int instr);
struct instruction32bit * t16decodeConditionalBranchSVC(u16int instr);
struct instruction32bit * t16decodeLoadMultipleRegisters(u16int instr);
struct instruction32bit * t16decodeStoreMultipleRegisters(u16int instr);
struct instruction32bit * t16decodeMisc(u16int instr);
struct instruction32bit * t16decodeSPAddr(u16int instr);
struct instruction32bit * t16decodePCAddr(u16int instr);
struct instruction32bit * t16decodeLoadStore(u16int instr);
struct instruction32bit * t16decodeLDR(u16int instr);
struct instruction32bit * t16decodeSpecialBranchExchange(u16int instr);
struct instruction32bit * t16decodeDataProc(u16int instr);
struct instruction32bit * t16decodeShift(u16int instr);

struct instruction16bit * t32tot16(u32int instr);
#endif
