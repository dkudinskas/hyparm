#include "common/debug.h"

#include "instructionEmu/asm-dis.h"
#include "instructionEmu/decoder.h"
#include "instructionEmu/coprocInstructions.h"
#include "instructionEmu/dataMoveInstr.h"
#include "instructionEmu/dataProcessInstr.h"
#include "instructionEmu/miscInstructions.h"


#define DECODER_DEBUG

u16int * currAddress = 0;

extern GCONTXT * getGuestContext(void);

/* Top level instruction categories */
struct TopLevelCategory categories[] = {
// identify unconditionals first!
{UNCONDITIONALS_CATEGORY,         0xF0000000, 0xF0000000},
{DATA_PROC_AND_MISC_CATEGORY,     0x0C000000, 0x00000000},
{LOAD_STORE_WORD_BYTE_CATEGORY,   0x0E000000, 0x04000000},
{LOAD_STORE_WORD_BYTE_CATEGORY,   0x0E000010, 0x06000000},
{MEDIA_CATEGORY,                  0x0E000010, 0x06000010},
{BRANCH_BLOCK_TRANSFER_CATEGORY,  0x0C000000, 0x08000000},
{SVC_COPROCESSOR_CATEGORY,        0x0C000000, 0x0c000000},
// if we hit this, something went BAD.
{UNDEFINED_CATEGORY,              0x00000000, 0x00000000}
};

//Thumb 16-bit
struct TopLevelCategory t16categories[] = {
{T16_UNCONDITIONALS_CATEGORY,			0xF800, 0xE000},
{T16_CONDITIONAL_BRANCH_AND_SVC_CALL,	0xF000, 0xD000},
{T16_LOAD_MULTIPLE_REGISTERS,			0xF800, 0xC800},
{T16_STORE_MULTIPLE_REGISTERS,			0xF800, 0xC000},
{T16_MISC,								0xF000, 0xB000},
{T16_SP_ADDR,							0xF800, 0xA800},
{T16_PC_ADDR,							0xF800, 0xA000},
{T16_LOAD_STORE,						0xE000, 0x8000},
{T16_LOAD_STORE,						0xE000, 0x6000},
{T16_LOAD_STORE,						0xE000, 0x5000},
{T16_LDR,								0xF800, 0x4800},
{T16_SPECIAL_AND_BRANCH_EXCHANGE,		0xFC00, 0x4400},
{T16_DATA_PROC,							0xFC00, 0x4000},
{T16_ARITHMETIC,						0xC000, 0x0000}
};


//Thumb 32-bit. Instructions are treated as 2 x halfwords
struct TopLevelCategory t32categories[] = {
{T32_LOAD_STORE_MULTIPLE,		0xFE400000,0xE8000000},
{T32_LOAD_STORE_DOUBLE_EXCLUSIVE,	0xFE400000,0xE8400000},
{T32_BRANCH_AND_MISC,		0xF8008000,0xF0008000},
{T32_DATA_PROC,				0xFE000000,0xEA000000},
{T32_COPROC,				0xFC000000,0xEC000000},
{T32_DATA_PROC,				0xFA008000,0xF0000000},
{T32_DATA_PROC,				0xFA008000,0xF2000000},
{T32_STORE_SINGLE,			0xFF100000,0xF8000000},
{T32_SIMD_STRUCT_LOAD_STORE, 0xFF100000,0xF9000000},
{T32_LOAD_BYTE,				0xFE700000,0xF8100000},
{T32_LOAD_HALFWORD,			0xFE700000,0xF8300000},
{T32_LOAD_WORD,				0xF8500000,0xF8500000},
{T32_UNDEFINED,				0xF8700000,0xF8700000},
{T32_DATA_PROC,				0xFF000000,0xFA000000},
{T32_MULTIPLY,				0xFF000000,0xFB000000},
{T32_LONG_MULTIPLY,			0xFF800000,0xFB800000},
{T32_COPROC,				0xFC000000,0xFC000000},
};


struct instruction32bit unconditionalInstructions[] = {
// UNIMPLEMENTED: data memory barrier
{1,  &dmbInstruction,       0xf57ff050, 0xfffffff0, "dmb\t%U"},
// sync barriers
{1,  &dsbInstruction,       0xf57ff040, 0xfffffff0, "DSB"},
{1,  &isbInstruction,       0xf57ff060, 0xfffffff0, "ISB"},
// UNIMPLEMENTED: CLREX clear exclusive
{1,  &clrexInstruction,     0xf57ff01f, 0xffffffff, "clrex"},
// CPS: change processor state
{1,  &cpsieInstruction,     0xf1080000, 0xfffffe3f, "CPSIE"},
{1,  &cpsieInstruction,     0xf10a0000, 0xfffffe20, "CPSIE"},
{1,  &cpsidInstruction,     0xf10C0000, 0xfffffe3f, "CPSID"},
{1,  &cpsidInstruction,     0xf10e0000, 0xfffffe20, "CPSID"},
{1,  &cpsInstruction,       0xf1000000, 0xfff1fe20, "CPS"},
// UNIMPLEMENTED: RFE return from exception
{1,  &rfeInstruction,       0xf8100a00, 0xfe50ffff, "rfe%23?id%24?ba\t%16-19r%21'!"},
// UNIMPLEMENTED: SETEND set endianess
{1,  &setendInstruction,    0xf1010000, 0xfffffc00, "setend\t%9?ble"},
// UNIMPLEMENTED: SRS store return state
{1,  &srsInstruction,       0xf84d0500, 0xfe5fffe0, "srs%23?id%24?ba\t%16-19r%21'!, #%0-4d"},
// BLX: branch and link to Thumb subroutine
{1,  &blxInstruction,       0xfa000000, 0xfe000000, "BLX #imm24"},
// PLD: preload data
{1,  &pldInstruction,       0xf450f000, 0xfc70f000, "PLD"},
// PLI: preload instruction
{1,  &pliInstruction,       0xf450f000, 0xfd70f000, "PLI"},
{-1, &undefinedInstruction, 0x00000000, 0x00000000, UNDEFINED_INSTRUCTION}
};

// register/shifter register cases, etc
struct instruction32bit dataProcMiscInstructions_op0[] = {
// NOP is just a nop...
{0,  &nopInstruction,       0xe1a00000, 0xffffffff, "NOP"},
// UNIMPLEMENTED: SWP swap
{1,  &swpInstruction,       0x01000090, 0x0fb00ff0, "SWP"},
// UNIMPLEMENTED:
{1,  &strhtInstruction,     0x006000b0, 0x0f7000f0, "STRHT instruction"},
{1,  &ldrhtInstruction,     0x003000b0, 0x0f3000f0, "LDRHT instruction"},
// store and load exclusive: must be emulated - user mode faults
{1,  &ldrexbInstruction,    0x01d00f9f, 0x0ff00fff, "LDREXB"},
{1,  &ldrexdInstruction,    0x01b00f9f, 0x0ff00fff, "LDREXD"},
{1,  &ldrexhInstruction,    0x01f00f9f, 0x0ff00fff, "LDREXH"},
{1,  &strexbInstruction,    0x01c00f90, 0x0ff00ff0, "STREXB"},
{1,  &strexdInstruction,    0x01a00f90, 0x0ff00ff0, "STREXD"},
{1,  &strexhInstruction,    0x01e00f90, 0x0ff00ff0, "STREXH"},
{1,  &ldrexInstruction,     0x01900f9f, 0x0ff00fff, "LDREX"},
{1,  &strexInstruction,     0x01800f90, 0x0ff00ff0, "STREX"},
// SMULL - signed multiply, PC cannot be used as any destination
{0,  &sumullInstruction,    0x00800090, 0x0fa000f0, "SMULL"},
// SMLAL - signed multiply and accumulate, PC cannot be used as any destination 
{0,  &sumlalInstruction,    0x00a00090, 0x0fa000f0, "SMLAL"},
// MUL: Rd = Rm * Rn; Rd != PC. pass through
{0,  &mulInstruction,       0x00000090, 0x0fe000f0, "MUL Rd, Rm, Rn"},
// MLA: Rd = Rm * Rn + Ra; Rd != PC. pass through
{0,  &mlaInstruction,       0x00200090, 0x0fe000f0, "MLA Rd, Rm, Rn, Ra"},
// MLS: Rd = Rm * Rn - Ra; Rd != PC, pass through
{0,  &mlsInstruction,       0x00600090, 0x0ff000f0, "MLS Rd, Rm, Rn, Ra"},
// Branch and try to exchange to ARM mode.
{1,  &bxInstruction,        0x012FFF10, 0x0ffffff0, "bx%c\t%0-3r"},
// Branch and link and try to exchange to Jazelle mode.
{1,  &bxjInstruction,       0x012fff20, 0x0ffffff0, "BXJ Rm"},
// Software breakpoint instruction... not sure yet.
{1,  &bkptInstruction,      0xe1200070, 0xfff000f0, "BKPT #imm8"},
// UNIMPLEMENTED: SMC, previously SMI: secure monitor call
{1,  &smcInstruction,       0x01600070, 0x0ff000f0, "smc%c\t%e"},
// Branch and link and try to exchange with Thumb mode.
{1,  &blxInstruction,       0x012fff30, 0x0ffffff0, "BLX Rm"},
// CLZ: Count leading zeroes - Rd, Rm != PC, pass through
{0,  &clzInstruction,       0x016f0f10, 0x0fff0ff0, "CLZ Rd, Rm"},
// UNIMPLEMENTED: saturated add/subtract/doubleAdd/doubleSubtract
{1,  &qaddInstruction,      0x01000050, 0x0ff00ff0, "qadd%c\t%12-15r, %0-3r, %16-19r"},
{1,  &qdaddInstruction,     0x01400050, 0x0ff00ff0, "qdadd%c\t%12-15r, %0-3r, %16-19r"},
{1,  &qsubInstruction,      0x01200050, 0x0ff00ff0, "qsub%c\t%12-15r, %0-3r, %16-19r"},
{1,  &qdsubInstruction,     0x01600050, 0x0ff00ff0, "qdsub%c\t%12-15r, %0-3r, %16-19r"},
// LDRD: Rt1 must be even numbered and NOT 14, thus Rt2 cannot be PC. pass.
{0,  &ldrdInstruction,      0x004000d0, 0x0e5000f0, "LDRD Rt, [Rn, #imm]"},
{0,  &ldrdInstruction,      0x000000d0, 0x0e500ff0, "LDRD Rt, [Rn, Rm]"},
// STRD: pass through, let them fail!
{0,  &strdInstruction,      0x004000f0, 0x0e5000f0, "STRD Rt, [Rn, #imm]"},
{0,  &strdInstruction,      0x000000f0, 0x0e500ff0, "STRD Rt, [Rn, Rm]"},
/* ALL UNIMPLEMENTED: smlabs, smulbs etc */
// signed 16 bit multiply, 32 bit accumulate
{1,  &smlabbInstruction,    0x01000080, 0x0ff000f0, "smlabb%c\t%16-19r, %0-3r, %8-11r, %12-15r"},
{1,  &smlatbInstruction,    0x010000a0, 0x0ff000f0, "smlatb%c\t%16-19r, %0-3r, %8-11r, %12-15r"},
{1,  &smlabtInstruction,    0x010000c0, 0x0ff000f0, "smlabt%c\t%16-19r, %0-3r, %8-11r, %12-15r"},
{1,  &smlattInstruction,    0x010000e0, 0x0ff000f0, "smlatt%c\t%16-19r, %0-3r, %8-11r, %12-15r"},
// signed 16 bit x 32 bit multiply, 32 bit accumulate
{1,  &smlawbInstruction,    0x01200080, 0x0ff000f0, "smlawb%c\t%16-19r, %0-3r, %8-11r, %12-15r"},
{1,  &smlawtInstruction,    0x012000c0, 0x0ff000f0, "smlawt%c\t%16-19r, %0-3r, %8-11r, %12-15r"},
// signed 16 bit multiply, 64 bit accumulate
{1,  &smlalbbInstruction,   0x01400080, 0x0ff000f0, "smlalbb%c\t%12-15r, %16-19r, %0-3r, %8-11r"},
{1,  &smlaltbInstruction,   0x014000a0, 0x0ff000f0, "smlaltb%c\t%12-15r, %16-19r, %0-3r, %8-11r"},
{1,  &smlalbtInstruction,   0x014000c0, 0x0ff000f0, "smlalbt%c\t%12-15r, %16-19r, %0-3r, %8-11r"},
{1,  &smlalttInstruction,   0x014000e0, 0x0ff000f0, "smlaltt%c\t%12-15r, %16-19r, %0-3r, %8-11r"},
// SMULBB: multiply signed bottom halfwords of ops. Rd can't be PC.
{0,  &smulbbInstruction,    0x01600080, 0x0ff0f0f0, "SMULBB Rd, Rn, RM"},
// signed 16 bit multiply, 32 bit result
{1,  &smultbInstruction,    0x016000a0, 0x0ff0f0f0, "smultb%c\t%16-19r, %0-3r, %8-11r"},
{1,  &smulbtInstruction,    0x016000c0, 0x0ff0f0f0, "smulbt%c\t%16-19r, %0-3r, %8-11r"},
{1,  &smulttInstruction,    0x016000e0, 0x0ff0f0f0, "smultt%c\t%16-19r, %0-3r, %8-11r"},
// signed 16 bit x 32 bit multiply, 32 bit result
{1,  &smulwbInstruction,    0x012000a0, 0x0ff0f0f0, "smulwb%c\t%16-19r, %0-3r, %8-11r"},
{1,  &smulwtInstruction,    0x012000e0, 0x0ff0f0f0, "smulwt%c\t%16-19r, %0-3r, %8-11r"},
// STRH: passthrough, will data abort if something wrong
{0,  &strhInstruction,      0x004000b0, 0x0e5000f0, "STRH Rt, [Rn, +-imm8]"},
{0,  &strhInstruction,      0x000000b0, 0x0e500ff0, "STRH Rt, [Rn], +-Rm"},
// LDRH cant load halfword to PC, passthrough
{0,  &ldrhInstruction,      0x00500090, 0x0e500090, "LDRH Rt, [Rn, +-imm8]"},
{0,  &ldrhInstruction,      0x00100090, 0x0e500f90, "LDRH Rt, [Rn], +-Rm"},
// AND: Rd = PC end block, others are fine
{1,  &andInstruction,       0x0000f000, 0x0fe0f010, "AND PC, Rn, Rm, #shamt"},
{1,  &andInstruction,       0x0000f010, 0x0fe0f090, "AND PC, Rn, Rm, Rshamt"},
{0,  &andInstruction,       0x00000000, 0x0fe00010, "AND Rd, Rn, Rm, #shamt"},
{0,  &andInstruction,       0x00000010, 0x0fe00090, "AND Rd, Rn, Rm, Rshamt"},
// EOR: Rd = PC end block, others are fine
{1,  &eorInstruction,       0x0020f000, 0x0fe0f010, "EOR PC, Rn, Rm, #shamt"},
{1,  &eorInstruction,       0x0020f010, 0x0fe0f090, "EOR PC, Rn, Rm, Rshamt"},
{0,  &eorInstruction,       0x00200000, 0x0fe00010, "EOR Rd, Rn, Rm, #shamt"},
{0,  &eorInstruction,       0x00200010, 0x0fe00090, "EOR Rd, Rn, Rm, Rshamt"},
// SUB: Rd = PC end block, others are fine
{1,  &subInstruction,       0x0040f000, 0x0fe0f010, "SUB PC, Rn, Rm, #shamt"},
{1,  &subInstruction,       0x0040f010, 0x0fe0f090, "SUB PC, Rn, Rm, Rshamt"},
{0,  &subInstruction,       0x00400000, 0x0fe00010, "SUB Rd, Rn, Rm, #shamt"},
{0,  &subInstruction,       0x00400010, 0x0fe00090, "SUB Rd, Rn, Rm, Rshamt"},
// RSB: Rd = PC end block, others are fine
{1,  &rsbInstruction,       0x0060f000, 0x0fe0f010, "RSB PC, Rn, Rm, #shamt"},
{1,  &rsbInstruction,       0x0060f010, 0x0fe0f090, "RSB PC, Rn, Rm, Rshamt"},
{0,  &rsbInstruction,       0x00600000, 0x0fe00010, "RSB Rd, Rn, Rm, #shamt"},
{0,  &rsbInstruction,       0x00600010, 0x0fe00090, "RSB Rd, Rn, Rm, Rshamt"},
// ADD: Rd = PC end block, others are fine
{1,  &addInstruction,       0x0080f000, 0x0fe0f010, "ADD PC, Rn, Rm, #shamt"},
{1,  &addInstruction,       0x0080f010, 0x0fe0f090, "ADD PC, Rn, Rm, Rshamt"},
{0,  &addInstruction,       0x00800000, 0x0fe00010, "ADD Rd, Rn, Rm, #shamt"},
{0,  &addInstruction,       0x00800010, 0x0fe00090, "ADD Rd, Rn, Rm, Rshamt"},
// ADC: Rd = PC end block, others are fine
{1,  &adcInstruction,       0x00a0f000, 0x0fe0f010, "ADC PC, Rn, Rm, #shamt"},
{1,  &adcInstruction,       0x00a0f010, 0x0fe0f090, "ADC PC, Rn, Rm, Rshamt"},
{0,  &adcInstruction,       0x00a00000, 0x0fe00010, "ADC Rd, Rn, Rm, #shamt"},
{0,  &adcInstruction,       0x00a00010, 0x0fe00090, "ADC Rd, Rn, Rm, Rshamt"},
// SBC: Rd = PC end block, others are fine
{1,  &sbcInstruction,       0x00c0f000, 0x0fe0f010, "SBC Rd, Rn, Rm, #shamt"},
{1,  &sbcInstruction,       0x00c0f010, 0x0fe0f090, "SBC Rd, Rn, Rm, Rshamt"},
{0,  &sbcInstruction,       0x00c00000, 0x0fe00010, "SBC Rd, Rn, Rm, #shamt"},
{0,  &sbcInstruction,       0x00c00010, 0x0fe00090, "SBC Rd, Rn, Rm, Rshamt"},
// RSC: Rd = PC end block, others are fine
{1,  &rscInstruction,       0x00e0f000, 0x0fe0f010, "RSC PC, Rn, Rm, #shamt"},
{1,  &rscInstruction,       0x00e0f010, 0x0fe0f090, "RSC PC, Rn, Rm, Rshamt"},
{0,  &rscInstruction,       0x00e00000, 0x0fe00010, "RSC Rd, Rn, Rm, #shamt"},
{0,  &rscInstruction,       0x00e00010, 0x0fe00090, "RSC Rd, Rn, Rm, Rshamt"},
// MSR/MRS: always hypercall! we must hide the real state from guest.
{2,  &msrInstruction,       0x0120f000, 0x0fb0fff0, "MSR, s/cpsr, Rn"},
{2,  &mrsInstruction,       0x010f0000, 0x0fbf0fff, "MRS, Rn, s/cpsr"},
// TST instructions are all fine
{0,  &tstInstruction,       0x01000000, 0x0fe00010, "TST Rn, Rm, #shamt"},
{0,  &tstInstruction,       0x01000010, 0x0fe00090, "TST Rn, Rm, Rshift"},
// TEQ instructions are all fine
{0,  &teqInstruction,       0x01200000, 0x0fe00010, "TEQ Rn, Rm, #shamt"},
{0,  &teqInstruction,       0x01200010, 0x0fe00090, "TEQ Rn, Rm, Rshift"},
// CMP instructions are all fine
{0,  &cmpInstruction,       0x01400000, 0x0fe00010, "CMP Rn, Rm, #shamt"},
{0,  &cmpInstruction,       0x01400010, 0x0fe00090, "CMP Rn, Rm, Rshamt"},
// CMN instructions are all fine
{0,  &cmnInstruction,       0x01600000, 0x0fe00010, "CMN Rn, Rm, #shamt"},
{0,  &cmnInstruction,       0x01600010, 0x0fe00090, "CMN Rn, Rm, Rshamt"},
// ORR: Rd = PC end block, other are fine
{1,  &orrInstruction,       0x0180f000, 0x0fe0f010, "ORR PC, Rn, Rm, #shamt"},
{1,  &orrInstruction,       0x0180f010, 0x0fe0f090, "ORR PC, Rn, Rm, Rshamt"},
{0,  &orrInstruction,       0x01800000, 0x0fe00010, "ORR Rd, Rn, Rm, #shamt"},
{0,  &orrInstruction,       0x01800010, 0x0fe00090, "ORR Rd, Rn, Rm, Rshamt"},
// MOV with Rd = PC end block. MOV reg, <shifted reg> is a pseudo instr for shift instructions
{1,  &movInstruction,       0x01a0f000, 0x0deffff0, "MOV PC, Rm"},
{0,  &movInstruction,       0x01a00000, 0x0def0ff0, "MOV Rn, Rm"},
// LSL: Rd = PC and shamt = #imm, then end block. Otherwise fine
{1,  &lslInstruction,       0x01a0f000, 0x0feff070, "LSL Rd, Rm, #shamt"},
{1,  &lslInstruction,       0x01a0f010, 0x0feff0f0, "LSL Rd, Rm, Rshamt"},
{0,  &lslInstruction,       0x01a00000, 0x0fef0070, "LSL Rd, Rm, #shamt"},
{0,  &lslInstruction,       0x01a00010, 0x0fef00f0, "LSL Rd, Rm, Rshamt"},
// LSR: Rd = PC and shamt = #imm, then end block. Otherwise fine
{1,  &lsrInstruction,       0x01a0f020, 0x0feff070, "LSR Rd, Rm, #shamt"},
{1,  &lsrInstruction,       0x01a0f030, 0x0feff0f0, "LSR Rd, Rm, Rshamt"},
{0,  &lsrInstruction,       0x01a00020, 0x0fef0070, "LSR Rd, Rm, #shamt"},
{0,  &lsrInstruction,       0x01a00030, 0x0fef00f0, "LSR Rd, Rm, Rshamt"},
// ASR: Rd = PC and shamt = #imm, then end block. Otherwise fine
{1,  &asrInstruction,       0x01a0f040, 0x0feff070, "ASR Rd, Rm, #shamt"},
{1,  &asrInstruction,       0x01a0f050, 0x0feff0f0, "ASR Rd, Rm, Rshamt"},
{0,  &asrInstruction,       0x01a00040, 0x0fef0070, "ASR Rd, Rm, #shamt"},
{0,  &asrInstruction,       0x01a00050, 0x0fef00f0, "ASR Rd, Rm, Rshamt"},
// RRX: shift right and extend, Rd can be PC
{1,  &rrxInstruction,       0x01a0f060, 0x0feffff0, "RRX PC, Rm"},
{0,  &rrxInstruction,       0x01a00060, 0x0fef0ff0, "RRX Rd, Rm"},
// ROR: reg case destination unpredictable. imm case dest can be PC.
{0,  &rorInstruction,       0x01a00070, 0x0fef00f0, "ROR Rd, Rm, Rn"},
{1,  &rorInstruction,       0x01a0f060, 0x0feff070, "ROR PC, Rm, #imm"},
{0,  &rorInstruction,       0x01a00060, 0x0fef0070, "ROR Rd, Rm, #imm"},
// BIC with Rd = PC end block, other are fine.
{1,  &bicInstruction,       0x01c0f000, 0x0fe0f010, "BIC PC, Rn, Rm, #shamt"},
{1,  &bicInstruction,       0x01c0f010, 0x0fe0f090, "BIC PC, Rn, Rm, Rshamt"},
{0,  &bicInstruction,       0x01c00000, 0x0fe00010, "BIC Rd, Rn, Rm, #shamt"},
{0,  &bicInstruction,       0x01c00010, 0x0fe00090, "BIC Rd, Rn, Rm, Rshamt"},
// MVN with Rd = PC end block, other are fine.
{1,  &mvnInstruction,       0x01e0f000, 0x0fe0f010, "MVN Rd, Rm, #shamt"},
{1,  &mvnInstruction,       0x01e0f010, 0x0fe0f090, "MVN Rd, Rm, Rshamt"},
{0,  &mvnInstruction,       0x01e00000, 0x0fe00010, "MVN Rd, Rm, #shamt"},
{0,  &mvnInstruction,       0x01e00010, 0x0fe00090, "MVN Rd, Rm, Rshamt"},

{-1, &undefinedInstruction, 0x00000000, 0x00000000, UNDEFINED_INSTRUCTION}
};

// immediate cases and random hints
struct instruction32bit dataProcMiscInstructions_op1[] = {
// UNIMPLEMENTED: yield hint
{1,  &yieldInstruction,     0x0320f001, 0x0fffffff, "yield%c"},
// UNIMPLEMENTED: wait for event hint
{1,  &wfeInstruction,       0x0320f002, 0x0fffffff, "wfe%c"},
// UNIMPLEMENTED: wait for interrupt hint
{1,  &wfiInstruction,       0x0320f003, 0x0fffffff, "wfi%c"},
// UNIMPLEMENTED: send event hint
{1,  &sevInstruction,       0x0320f004, 0x0fffffff, "sev%c"},
{0,  &nopInstruction,       0x0320f000, 0x0fffff00, "NOP"},
// UNIMPLEMENTED: debug hint
{1,  &dbgInstruction,       0x0320f0f0, 0x0ffffff0, "dbg%c\t#%0-3d"},
// MOVW - indication of 'wide' - to select ARM encoding. Rd cant be PC, pass through.
{0,  &movwInstruction,      0x03000000, 0x0ff00000, "MOVW Rd, Rn"},
{0,  &movtInstruction,      0x03400000, 0x0ff00000, "MOVT Rd, Rn"},
// AND: Rd = PC end block, others are fine
{1,  &andInstruction,       0x0200f000, 0x0fe0f000, "AND PC, Rn, #imm"},
{0,  &andInstruction,       0x02000000, 0x0fe00000, "AND Rd, Rn, #imm"},
// EOR: Rd = PC end block, others are fine
{1,  &eorInstruction,       0x0220f000, 0x0fe0f000, "EOR PC, Rn, Rm/#imm"},
{0,  &eorInstruction,       0x02200000, 0x0fe00000, "EOR Rd, Rn, #imm"},
// SUB: Rd = PC end block, others are fine
{1,  &subInstruction,       0x0240f000, 0x0fe0f000, "SUB PC, Rn, Rm/imm"},
{0,  &subInstruction,       0x02400000, 0x0fe00000, "SUB Rd, Rn, #imm"},
// RSB: Rd = PC end block, others are fine
{1,  &rsbInstruction,       0x0260f000, 0x0fe0f000, "RSB PC, Rn, Rm/imm"},
{0,  &rsbInstruction,       0x02600000, 0x0fe00000, "RSB Rd, Rn, #imm"},
// ADD: Rd = PC end block, others are fine
{1,  &addInstruction,       0x0280f000, 0x0fe0f000, "ADD PC, Rn, #imm"},
{0,  &addInstruction,       0x02800000, 0x0fe00000, "ADD Rd, Rn, #imm"},
// ADC: Rd = PC end block, others are fine
{1,  &adcInstruction,       0x02a0f000, 0x0fe0f000, "ADC PC, Rn/#imm"},
{0,  &adcInstruction,       0x02a00000, 0x0fe00000, "ADC Rd, Rn, #imm"},
// SBC: Rd = PC end block, others are fine
{1,  &sbcInstruction,       0x02c0f000, 0x0fe0f000, "SBC PC, Rn/#imm"},
{0,  &sbcInstruction,       0x02c00000, 0x0fe00000, "SBC Rd, Rn, #imm"},
// RSC: Rd = PC end block, others are fine
{1,  &rscInstruction,       0x02e0f000, 0x0fe0f000, "RSC PC, Rn/#imm"},
{0,  &rscInstruction,       0x02e00000, 0x0fe00000, "RSC Rd, Rn, #imm"},
// MSR: always hypercall! we must hide the real state from guest.
{2,  &msrInstruction,       0x0320f000, 0x0fb0f000, "MSR, s/cpsr, #imm"},
// TST instructions are all fine
{0,  &tstInstruction,       0x03000000, 0x0fe00000, "TST Rn, #imm"},
// TEQ instructions are all fine
{0,  &teqInstruction,       0x03200000, 0x0fe00000, "TEQ Rn, #imm"},
// CMP instructions are all fine
{0,  &cmpInstruction,       0x03400000, 0x0fe00000, "CMP Rn, #imm"},
// CMN instructions are all fine
{0,  &cmnInstruction,       0x03600000, 0x0fe00000, "CMN Rn, #imm"},
// ORR: Rd = PC end block, other are fine
{1,  &orrInstruction,       0x0380f000, 0x0fe0f000, "ORR Rd, Rn, #imm"},
{0,  &orrInstruction,       0x03800000, 0x0fe00000, "ORR Rd, Rn, #imm"},
// MOV with Rd = PC end block. MOV <shifted reg> is a pseudo instr..
{1,  &movInstruction,       0x03a0f000, 0x0feff000, "MOV PC, #imm"},
{0,  &movInstruction,       0x03a00000, 0x0fef0000, "MOV Rn, #imm"},
// BIC with Rd = PC end block, other are fine.
{1,  &bicInstruction,       0x03c0f000, 0x0fe0f000, "BIC PC, Rn, #imm"},
{0,  &bicInstruction,       0x03c00000, 0x0fe00000, "BIC Rd, Rn, #imm"},
// MVN with Rd = PC end block, other are fine.
{1,  &mvnInstruction,       0x03e0f000, 0x0fe0f000, "MVN PC, Rn, #imm"},
{0,  &mvnInstruction,       0x03e00000, 0x0fe00000, "MVN Rd, Rn, #imm"},

{-1, &undefinedInstruction, 0x00000000, 0x00000000, UNDEFINED_INSTRUCTION}
};

struct instruction32bit loadStoreWordByteInstructions[] = {
// STR imm12 and reg are pass-through
{0,  &strInstruction,       0x04000000, 0x0e100000, "STR Rt, [Rn, +-imm12]"},
{0,  &strInstruction,       0x06000000, 0x0e100ff0, "STR Rt, [Rn], +-Rm"},
{0,  &strInstruction,       0x04000000, 0x0c100010, "STR any? dont get this."},
{0,  &strbInstruction,      0x04400000, 0x0e500000, "STRB Rt, [Rn, +-imm12]"},
{0,  &strbInstruction,      0x06400000, 0x0e500010, "STRB Rt, [Rn], +-Rm"},
// LDR traps if dest = PC, otherwise pass through
{1,  &ldrInstruction,       0x0410f000, 0x0c10f000, "LDR PC, Rn/#imm12"},
{0,  &ldrInstruction,       0x04100000, 0x0c100000, "LDR Rd, Rn/#imm12"},
{-1, &undefinedInstruction, 0x00000000, 0x00000000, UNDEFINED_INSTRUCTION}
};

struct instruction32bit mediaInstructions[] = {
// BFC: bit field clear, dest PC not allowed.
{0,  &bfcInstruction,       0x07c0001f, 0x0fe0007f, "BFC Rd, #LSB, #width"},
// BFI: bit field insert, dest PC not allowed.
{0,  &bfiInstruction,       0x07c00010, 0x0fe00070, "BFI Rd, #LSB, #width"},
// UNIMPLEMENTED: reverse bits
{1,  &rbitInstruction,      0x06ff0f30, 0x0fff0ff0, "rbit%c\t%12-15r, %0-3r"},
// UBFX: extract bit field - destination 15 unpredictable 
{0,  &usbfxInstruction,     0x07a00050, 0x0fa00070, "UBFX Rd, Rn, width"},
// UNIMPLEMENTED: pack halfword
{1,  &pkhbtInstruction,     0x06800010, 0x0ff00ff0, "pkhbt%c\t%12-15r, %16-19r, %0-3r"},
{1,  &pkhbtInstruction,     0x06800010, 0x0ff00070, "pkhbt%c\t%12-15r, %16-19r, %0-3r, lsl #%7-11d"},
{1,  &pkhtbInstruction,     0x06800050, 0x0ff00ff0, "pkhtb%c\t%12-15r, %16-19r, %0-3r, asr #32"},
{1,  &pkhtbInstruction,     0x06800050, 0x0ff00070, "pkhtb%c\t%12-15r, %16-19r, %0-3r, asr #%7-11d"},
// UNIMPLEMENTED: saturating add 16 bit and 8 bit
{1,  &qadd16Instruction,    0x06200f10, 0x0ff00ff0, "qadd16%c\t%12-15r, %16-19r, %0-3r"},
{1,  &qadd8Instruction,     0x06200f90, 0x0ff00ff0, "qadd8%c\t%12-15r, %16-19r, %0-3r"},
// UNIMPLEMENTED: saturating add and subtract with exchange
{1,  &qaddsubxInstruction,  0x06200f30, 0x0ff00ff0, "qaddsubx%c\t%12-15r, %16-19r, %0-3r"},
// UNIMPLEMENTED: saturating subtract 16 and 8 bit
{1,  &qsub16Instruction,    0x06200f70, 0x0ff00ff0, "qsub16%c\t%12-15r, %16-19r, %0-3r"},
{1,  &qsub8Instruction,     0x06200ff0, 0x0ff00ff0, "qsub8%c\t%12-15r, %16-19r, %0-3r"},
// UNIMPLEMENTED: saturating subtract and add with exchange
{1,  &qsubaddxInstruction,  0x06200f50, 0x0ff00ff0, "qsubaddx%c\t%12-15r, %16-19r, %0-3r"},
// UNIMPLEMENTED: signed add 16 bit and 8 bit 
{1,  &sadd16Instruction,    0x06100f10, 0x0ff00ff0, "sadd16%c\t%12-15r, %16-19r, %0-3r"},
{1,  &sadd8Instruction,     0x06100f90, 0x0ff00ff0, "sadd8%c\t%12-15r, %16-19r, %0-3r"},
// signed add and subtract with exchange
{1,  &saddsubxInstruction,  0x06100f30, 0x0ff00ff0, "saddsubx%c\t%12-15r, %16-19r, %0-3r"},
// signed subtract 16 bit and 8 bit
{1,  &ssub16Instruction,    0x06100f70, 0x0ff00ff0, "ssub16%c\t%12-15r, %16-19r, %0-3r"},
{1,  &ssub8Instruction,     0x06100ff0, 0x0ff00ff0, "ssub8%c\t%12-15r, %16-19r, %0-3r"},
// signed subtract and add with exchange
{1,  &ssubaddxInstruction,  0x06100f50, 0x0ff00ff0, "ssubaddx%c\t%12-15r, %16-19r, %0-3r"},
// UNIMPLEMENTED: signed halvign add 16 bit and 8 bit 
{1,  &shadd16Instruction,   0x06300f10, 0x0ff00ff0, "shadd16%c\t%12-15r, %16-19r, %0-3r"},
{1,  &shadd8Instruction,    0x06300f90, 0x0ff00ff0, "shadd8%c\t%12-15r, %16-19r, %0-3r"},
// UNIMPLEMENTED: signed halving add and subtract with exchange 
{1,  &shaddsubxInstruction, 0x06300f30, 0x0ff00ff0, "shaddsubx%c\t%12-15r, %16-19r, %0-3r"},
// UNIMPLEMENTED: signed halvign subtract 16 bit and 8 bit 
{1,  &shsub16Instruction,   0x06300f70, 0x0ff00ff0, "shsub16%c\t%12-15r, %16-19r, %0-3r"},
{1,  &shsub8Instruction,    0x06300ff0, 0x0ff00ff0, "shsub8%c\t%12-15r, %16-19r, %0-3r"},
// UNIMPLEMENTED: signed halving subtract and add with exchange 
{1,  &shsubaddxInstruction, 0x06300f50, 0x0ff00ff0, "shsubaddx%c\t%12-15r, %16-19r, %0-3r"},
// UNIMPLEMENTED: unsigned add 16 bit and 8 bit
{1,  &uadd16Instruction,    0x06500f10, 0x0ff00ff0, "uadd16%c\t%12-15r, %16-19r, %0-3r"},
{1,  &uadd8Instruction,     0x06500f90, 0x0ff00ff0, "uadd8%c\t%12-15r, %16-19r, %0-3r"},
// UNIMPLEMENTED: unsigned add and subtract with exchange
{1,  &uaddsubxInstruction,  0x06500f30, 0x0ff00ff0, "uaddsubx%c\t%12-15r, %16-19r, %0-3r"},
// UNIMPLEMENTED: unsigned subtract 16 bit and 8 bit
{1,  &usub16Instruction,    0x06500f70, 0x0ff00ff0, "usub16%c\t%12-15r, %16-19r, %0-3r"},
{1,  &usub8Instruction,     0x06500ff0, 0x0ff00ff0, "usub8%c\t%12-15r, %16-19r, %0-3r"},
// UNIMPLEMENTED: unsigned subtract and add with exchange 
{1,  &usubaddxInstruction,  0x06500f50, 0x0ff00ff0, "usubaddx%c\t%12-15r, %16-19r, %0-3r"},
// UNIMPLEMENTED: unsigned halving add 16 bit and 8 bit
{1,  &uhadd16Instruction,   0x06700f10, 0x0ff00ff0, "uhadd16%c\t%12-15r, %16-19r, %0-3r"},
{1,  &uhadd8Instruction,    0x06700f90, 0x0ff00ff0, "uhadd8%c\t%12-15r, %16-19r, %0-3r"},
// UNIMPLEMENTED: unsigned halving add and subtract with exchange
{1,  &uhaddsubxInstruction, 0x06700f30, 0x0ff00ff0, "uhaddsubxc\t%12-15r, %16-19r, %0-3r"},
// UNIMPLEMENTED: unsigned halving subtract 16 bit and 8 bit
{1,  &uhsub16Instruction,   0x06700f70, 0x0ff00ff0, "uhsub16%c\t%12-15r, %16-19r, %0-3r"},
{1,  &uhsub8Instruction,    0x06700ff0, 0x0ff00ff0, "uhsub8%c\t%12-15r, %16-19r, %0-3r"},
// UNIMPLEMENTED: unsigned halving subtract and add with exchange
{1,  &uhsubaddxInstruction, 0x06700f50, 0x0ff00ff0, "uhsubaddx%c\t%12-15r, %16-19r, %0-3r"},
// UNIMPLEMENTED:  unsigned saturating add 16 bit and 8 bit
{1,  &uqadd16Instruction,   0x06600f10, 0x0ff00ff0, "uqadd16%c\t%12-15r, %16-19r, %0-3r"},
{1,  &uqadd8Instruction,    0x06600f90, 0x0ff00ff0, "uqadd8%c\t%12-15r, %16-19r, %0-3r"},
// UNIMPLEMENTED: unsigned saturating add and subtract with exchange
{1,  &uqaddsubxInstruction, 0x06600f30, 0x0ff00ff0, "uqaddsubx%c\t%12-15r, %16-19r, %0-3r"},
// UNIMPLEMENTED: unsigned saturating subtract 16 bit and 8 bit
{1,  &uqsub16Instruction,   0x06600f70, 0x0ff00ff0, "uqsub16%c\t%12-15r, %16-19r, %0-3r"},
{1,  &uqsub8Instruction,    0x06600ff0, 0x0ff00ff0, "uqsub8%c\t%12-15r, %16-19r, %0-3r"},
// UNIMPLEMENTED: unsigned saturating subtract and add with exchange
{1,  &uqsubaddxInstruction, 0x06600f50, 0x0ff00ff0, "uqsubaddx%c\t%12-15r, %16-19r, %0-3r"},
// UNIMPLEMENTED: byte-reverse word
{1,  &revInstruction,       0x06bf0f30, 0x0fff0ff0, "rev%c\t%12-15r, %0-3r"},
// UNIMPLEMENTED:  byte-reverse packed halfword
{1,  &rev16Instruction,     0x06bf0fb0, 0x0fff0ff0, "rev16%c\t%12-15r, %0-3r"},
// UNIMPLEMENTED:  byte-reverse signed halfword
{1,  &revshInstruction,     0x06ff0fb0, 0x0fff0ff0, "revsh%c\t%12-15r, %0-3r"},
// UNIMPLEMENTED: sign-extend halfword
{1,  &sxthInstruction,      0x06bf0070, 0x0fff0ff0, "sxth%c\t%12-15r, %0-3r"},
{1,  &sxthInstruction,      0x06bf0470, 0x0fff0ff0, "sxth%c\t%12-15r, %0-3r, ror #8"},
{1,  &sxthInstruction,      0x06bf0870, 0x0fff0ff0, "sxth%c\t%12-15r, %0-3r, ror #16"},
{1,  &sxthInstruction,      0x06bf0c70, 0x0fff0ff0, "sxth%c\t%12-15r, %0-3r, ror #24"},
// UNIMPLEMENTED: sign-extend byte 16
{1,  &sxtb16Instruction,    0x068f0070, 0x0fff0ff0, "sxtb16%c\t%12-15r, %0-3r"},
{1,  &sxtb16Instruction,    0x068f0470, 0x0fff0ff0, "sxtb16%c\t%12-15r, %0-3r, ror #8"},
{1,  &sxtb16Instruction,    0x068f0870, 0x0fff0ff0, "sxtb16%c\t%12-15r, %0-3r, ror #16"},
{1,  &sxtb16Instruction,    0x068f0c70, 0x0fff0ff0, "sxtb16%c\t%12-15r, %0-3r, ror #24"},
// SXTB: destination register PC not allowed,
{0,  &sxtbInstruction,      0x06af0070, 0x0fff0ff0, "sxtb%c\t%12-15r, %0-3r"},
{0,  &sxtbInstruction,      0x06af0470, 0x0fff0ff0, "sxtb%c\t%12-15r, %0-3r, ror #8"},
{0,  &sxtbInstruction,      0x06af0870, 0x0fff0ff0, "sxtb%c\t%12-15r, %0-3r, ror #16"},
{0,  &sxtbInstruction,      0x06af0c70, 0x0fff0ff0, "sxtb%c\t%12-15r, %0-3r, ror #24"},
// UXTH permitted, if Rd or Rn = 15 then unpredictable.
{0,  &uxthInstruction,      0x06ff0070, 0x0fff0ff0, "UXTH Rd, Rn"},
{0,  &uxthInstruction,      0x06ff0470, 0x0fff0ff0, "UXTH Rd, Rn, ROR #8"},
{0,  &uxthInstruction,      0x06ff0870, 0x0fff0ff0, "UXTH Rd, Rn, ROR #16"},
{0,  &uxthInstruction,      0x06ff0c70, 0x0fff0ff0, "UXTH Rd, Rn, ROR #24"},
// UXTB16 permitted, if Rd or Rn = 15 then unpredictable.
{0,  &uxtb16Instruction,    0x06cf0070, 0x0fff0ff0, "UXTB16 Rd, Rn"},
{0,  &uxtb16Instruction,    0x06cf0470, 0x0fff0ff0, "UXTB16 Rd, Rn, ROR #8"},
{0,  &uxtb16Instruction,    0x06cf0870, 0x0fff0ff0, "UXTB16 Rd, Rn, ROR #16"},
{0,  &uxtb16Instruction,    0x06cf0c70, 0x0fff0ff0, "UXTB16 Rd, Rn, ROR #24"},
// UXTB permitted, if Rd or Rn = 15 then unpredictable.
{0,  &uxtbInstruction,      0x06ef0070, 0x0fff0ff0, "UXTB Rd, Rn"},
{0,  &uxtbInstruction,      0x06ef0470, 0x0fff0ff0, "UXTB Rd, Rn, ROR #8"},
{0,  &uxtbInstruction,      0x06ef0870, 0x0fff0ff0, "UXTB Rd, Rn, ROR #16"},
{0,  &uxtbInstruction,      0x06ef0c70, 0x0fff0ff0, "UXTB Rd, Rn, ROR #24"},
// UNIMPLEMENTED: sign-extend and add halfword
{1,  &sxtahInstruction,     0x06b00070, 0x0ff00ff0, "sxtah%c\t%12-15r, %16-19r, %0-3r"},
{1,  &sxtahInstruction,     0x06b00470, 0x0ff00ff0, "sxtah%c\t%12-15r, %16-19r, %0-3r, ror #8"},
{1,  &sxtahInstruction,     0x06b00870, 0x0ff00ff0, "sxtah%c\t%12-15r, %16-19r, %0-3r, ror #16"},
{1,  &sxtahInstruction,     0x06b00c70, 0x0ff00ff0, "sxtah%c\t%12-15r, %16-19r, %0-3r, ror #24"},
// UNIMPLEMENTED: sign-extend and add byte 16
{1,  &sxtab16Instruction,   0x06800070, 0x0ff00ff0, "sxtab16%c\t%12-15r, %16-19r, %0-3r"},
{1,  &sxtab16Instruction,   0x06800470, 0x0ff00ff0, "sxtab16%c\t%12-15r, %16-19r, %0-3r, ror #8"},
{1,  &sxtab16Instruction,   0x06800870, 0x0ff00ff0, "sxtab16%c\t%12-15r, %16-19r, %0-3r, ror #16"},
{1,  &sxtab16Instruction,   0x06800c70, 0x0ff00ff0, "sxtab16%c\t%12-15r, %16-19r, %0-3r, ror #24"},
// UNIMPLEMENTED: sign-extend and add byte
{1,  &sxtabInstruction,     0x06a00070, 0x0ff00ff0, "sxtab%c\t%12-15r, %16-19r, %0-3r"},
{1,  &sxtabInstruction,     0x06a00470, 0x0ff00ff0, "sxtab%c\t%12-15r, %16-19r, %0-3r, ror #8"},
{1,  &sxtabInstruction,     0x06a00870, 0x0ff00ff0, "sxtab%c\t%12-15r, %16-19r, %0-3r, ror #16"},
{1,  &sxtabInstruction,     0x06a00c70, 0x0ff00ff0, "sxtab%c\t%12-15r, %16-19r, %0-3r, ror #24"},
// UNIMPLEMENTED: unsigned extend and add halfword
{1,  &uxtahInstruction,     0x06f00070, 0x0ff00ff0, "uxtah%c\t%12-15r, %16-19r, %0-3r"},
{1,  &uxtahInstruction,     0x06f00470, 0x0ff00ff0, "uxtah%c\t%12-15r, %16-19r, %0-3r, ror #8"},
{1,  &uxtahInstruction,     0x06f00870, 0x0ff00ff0, "uxtah%c\t%12-15r, %16-19r, %0-3r, ror #16"},
{1,  &uxtahInstruction,     0x06f00c70, 0x0ff00ff0, "uxtah%c\t%12-15r, %16-19r, %0-3r, ror #24"},
// UNIMPLEMENTED: unsigned extend and add byte 16
{1,  &uxtab16Instruction,   0x06c00070, 0x0ff00ff0, "uxtab16%c\t%12-15r, %16-19r, %0-3r"},
{1,  &uxtab16Instruction,   0x06c00470, 0x0ff00ff0, "uxtab16%c\t%12-15r, %16-19r, %0-3r, ror #8"},
{1,  &uxtab16Instruction,   0x06c00870, 0x0ff00ff0, "uxtab16%c\t%12-15r, %16-19r, %0-3r, ror #16"},
{1,  &uxtab16Instruction,   0x06c00c70, 0x0ff00ff0, "uxtab16%c\t%12-15r, %16-19r, %0-3r, ROR #24"},
// UNIMPLEMENTED: unsigned extend and add byte
{1,  &uxtabInstruction,     0x06e00070, 0x0ff00ff0, "uxtab%c\t%12-15r, %16-19r, %0-3r"},
{1,  &uxtabInstruction,     0x06e00470, 0x0ff00ff0, "uxtab%c\t%12-15r, %16-19r, %0-3r, ror #8"},
{1,  &uxtabInstruction,     0x06e00870, 0x0ff00ff0, "uxtab%c\t%12-15r, %16-19r, %0-3r, ror #16"},
{1,  &uxtabInstruction,     0x06e00c70, 0x0ff00ff0, "uxtab%c\t%12-15r, %16-19r, %0-3r, ror #24"},
// UNIMPLEMENTED: select bytes
{1,  &selInstruction,       0x06800fb0, 0x0ff00ff0, "sel%c\t%12-15r, %16-19r, %0-3r"},
// UNIMPLEMENTED: signed dual multiply add 
{1,  &smuadInstruction,     0x0700f010, 0x0ff0f0d0, "smuad%5'x%c\t%16-19r, %0-3r, %8-11r"},
// UNIMPLEMENTED: signed dual multiply subtract 
{1,  &smusdInstruction,     0x0700f050, 0x0ff0f0d0, "smusd%5'x%c\t%16-19r, %0-3r, %8-11r"},
// UNIMPLEMENTED: signed multiply accumulate dual
{1,  &smladInstruction,     0x07000010, 0x0ff000d0, "smlad%5'x%c\t%16-19r, %0-3r, %8-11r, %12-15r"},
// UNIMPLEMENTED: signed multiply accumulate long dual
{1,  &smlaldInstruction,    0x07400010, 0x0ff000d0, "smlald%5'x%c\t%12-15r, %16-19r, %0-3r, %8-11r"},
// UNIMPLEMENTED: signed multiply subtract dual
{1,  &smlsdInstruction,     0x07000050, 0x0ff000d0, "smlsd%5'x%c\t%16-19r, %0-3r, %8-11r, %12-15r"},
// UNIMPLEMENTED: signed multiply subtract long dual
{1,  &smlsldInstruction,    0x07400050, 0x0ff000d0, "smlsld%5'x%c\t%12-15r, %16-19r, %0-3r, %8-11r"},
// UNIMPLEMENTED: signed most significant word multiply 
{1,  &smmulInstruction,     0x0750f010, 0x0ff0f0d0, "smmul%5'r%c\t%16-19r, %0-3r, %8-11r"},
// UNIMPLEMENTED: signed most significant word multiply accumulate 
{1,  &smmlaInstruction,     0x07500010, 0x0ff000d0, "smmla%5'r%c\t%16-19r, %0-3r, %8-11r, %12-15r"},
// UNIMPLEMENTED: signed most significant word multiply subtract
{1,  &smmlsInstruction,     0x075000d0, 0x0ff000d0, "smmls%5'r%c\t%16-19r, %0-3r, %8-11r, %12-15r"},
// UNIMPLEMENTED: signed saturate
{1,  &ssatInstruction,      0x06a00010, 0x0fe00ff0, "ssat%c\t%12-15r, #%16-20W, %0-3r"},
{1,  &ssatInstruction,      0x06a00010, 0x0fe00070, "ssat%c\t%12-15r, #%16-20W, %0-3r, lsl #%7-11d"},
{1,  &ssatInstruction,      0x06a00050, 0x0fe00070, "ssat%c\t%12-15r, #%16-20W, %0-3r, asr #%7-11d"},
// UNIMPLEMENTED: signed saturate 16
{1,  &ssat16Instruction,    0x06a00f30, 0x0ff00ff0, "ssat16%c\t%12-15r, #%16-19W, %0-3r"},
// UNIMPLEMENTED: unsigned saturate
{1,  &usatInstruction,      0x06e00010, 0x0fe00ff0, "usat%c\t%12-15r, #%16-20d, %0-3r"},
{1,  &usatInstruction,      0x06e00010, 0x0fe00070, "usat%c\t%12-15r, #%16-20d, %0-3r, lsl #%7-11d"},
{1,  &usatInstruction,      0x06e00050, 0x0fe00070, "usat%c\t%12-15r, #%16-20d, %0-3r, asr #%7-11d"},
// UNIMPLEMENTED: unsigned saturate 16
{1,  &usat16Instruction,    0x06e00f30, 0x0ff00ff0, "usat16%c\t%12-15r, #%16-19d, %0-3r"},

{-1, &undefinedInstruction, 0x00000000, 0x00000000, UNDEFINED_INSTRUCTION}
};

struct instruction32bit branchBlockTransferInstructions[] = {
// STM traps if ^ postfix, otherwise pass through
{1,  &stmInstruction,       0x08400000, 0x0e500000, "STM {regList}^"},
{0,  &stmInstruction,       0x08800000, 0x0ff00000, "STMIA {regList}"},
{0,  &stmInstruction,       0x08000000, 0x0e100000, "STM {regList}"},
// POP LDM syntax, only care if PC in reglist
{1,  &popLdmInstruction,       0x08bd8000, 0x0fff8000, "LDM SP, {...r15}"},
{0,  &popLdmInstruction,       0x08bd0000, 0x0fff0000, "LDM SP, {reglist}"},
// LDM traps if ^ postfix and/or PC is in register list, otherwise pass through
{1,  &ldmInstruction,       0x08500000, 0x0e500000, "LDM Rn, {regList}^"},
{1,  &ldmInstruction,       0x08108000, 0x0e108000, "LDM Rn, {..r15}"},
{0,  &ldmInstruction,       0x08900000, 0x0f900000, "LDMIA Rn, {regList}"},
{0,  &ldmInstruction,       0x08100000, 0x0e100000, "LDM Rn, {regList}"},
// B/BL: always hypercall! obviously.
{1,  &bInstruction,         0x0a000000, 0x0e000000, "BRANCH"},

{-1, &undefinedInstruction, 0x00000000, 0x00000000, UNDEFINED_INSTRUCTION}
};

struct instruction32bit svcCoprocInstructions[] = {
// well obviously.
{1,  &svcInstruction,       0x0f000000, 0x0f000000, "SWI code"},
// Generic coprocessor instructions.
{3,  &mrcInstruction,   0x0e100010, 0x0f100010, "MRC"},
{3,  &mcrInstruction,   0x0e000010, 0x0f100010, "MCR"},

{-1, &undefinedInstruction, 0x00000000, 0x00000000, UNDEFINED_INSTRUCTION}
};

//-----------------------	THUMB INSTRUCTIONS	--------------------------//

struct instruction32bit t16SPInstructions[] = {
{0,	&addInstruction, 0xA800, 0xF800, "ADD <Rd>, SP, #<imm>"},
{0, &addInstruction, 0xB000, 0xFF80, "ADD SP, SP, #<imm>"}
};

struct instruction32bit t16PCRelInstructions[] = {
// ADR instruction is add or sub but in any case it does not trap
{0, &addInstruction, 0xA000,0xF800, "ADR <Rd>, <label>"}
};

struct instruction32bit t16ConditionalBranchSvcInstructions[] = {
{1, &svcInstruction, 0xDF00, 0xFF00, "SVC call"},
{1, &bInstruction, 0xD000, 0xF000, "B<c> <label>"}
};

struct instruction32bit t16ldrInstructions[] = {
{0, &ldrInstruction, 0x6800, 0xF800, "LDR<c> <Rt>, [<Rn>{,#<imm5>}]"},
{0, &ldrInstruction, 0x9800, 0xF800, "LDR<c> <Rt>, [SP{,#<imm8>}]"},
{0, &ldrInstruction, 0x4800, 0xF800, "LDR<c> <Rt>, <label>"},
{0, &ldrInstruction, 0x5800, 0xFE00, "LDR<c> <Rt>, [<Rn>,<Rm>]"}
};

struct instruction32bit t16DataProcInstructions[] = {
{0, &mvnInstruction, 0x43C0, 0xFFC0, "MVN <Rd>, <Rm>"},
{0, &cmpInstruction, 0x4280, 0xFFC0, "CMP <Rn>, <Rm>"},
{0, &cmpInstruction, 0x4500, 0xFF00, "CMP <Rn>, <Rm>"}
};

struct instruction32bit t16SpecialBranchInstructions[] = {
{1, &bxInstruction, 0x4700, 0xFF80, "BX<c> <Rm>"},
{1, &blxInstruction, 0x4780, 0xFF80, "BLX<c> <Rm>"},
{1, &movInstruction, 0x4607, 0xFF07, "MOV PC, Rm"},
{0, &movInstruction, 0x4600, 0xFF00,  "MOV Rd, Rm"},
{0, &addInstruction, 0x1800, 0xFE00, "ADD <Rd>, <Rn>, <Rm>"},
{0, &addInstruction, 0x4400, 0xFF00, "ADD <Rdn>, <Rm>"}
};

struct instruction32bit t16MiscInstructions[] = {
{0, &stmInstruction, 0xB400, 0xFE00, "PUSH {reglist}"},
{0, &subInstruction, 0xB080, 0xFF80, "SUB SP,SP,<imm7"},
{0, &uxtbInstruction, 0xB2C0, 0xFFC0, "UXTB<c> <Rd>, <Rm>"},
{0, &uxthInstruction, 0xB280, 0xFFC0, "UXTH<C> <Rd>, <Rm>"},
{0, &addInstruction, 0xA800, 0xF800, "ADD <Rd>, SP, #<imm8>"},
{0, &addInstruction, 0xB000, 0xFF80, "ADD SP, SP, #<imm>"},
// trap if PC is on POP reglist
{1, &popLdmInstruction, 0xBD00, 0xFF00, "POP <reglist+PC>"},
{0, &popLdmInstruction, 0xBC00, 0xFE00, "POP <reglist>"},
{0, &nopInstruction, 0xBF00,0xFFFF, "NOP"},
// I think that the If-Then-Else Instruction does not need to trap -.-
{0, &itInstruction, 0xBF00,0xFF00, "IT"}
};

struct instruction32bit t16LoadStoreInstructions[] = {
{0, &strInstruction, 0x6000, 0xF800, "STR Rd, [<Rn>, {#<imm5>}]"},
{0, &strInstruction, 0x9000, 0xF800, "STR Rd, [SP,#<imm8]"},
{0, &ldrInstruction, 0x6800, 0xF800, "LDR Rd, [<Rn> {,#<imm5>}]"},
{0, &ldrInstruction, 0x9800, 0xF800, "LDR Rd, [SP {,#<imm8>}]"},
{0, &ldrInstruction, 0x7800, 0x7800, "LDRB Rd, #<imm5>"},
{0, &ldrhInstruction, 0x8800, 0xF800, "LDRH <Rt>, [<Rn>{, #<imm5>}]"},
{0, &strbInstruction, 0x7000, 0xF800, "STRB <Rt>, [<Rn>, #<imm5>]"},
{0, &strbInstruction, 0xA400, 0xFE00, "STRB <Rt>, [<Rn>, <Rm>]"},
{0, &strhInstruction, 0x8000, 0xF800, "STRH <Rt>,[<Rn>,{,#<imm>}]"},
{0, &strhInstruction, 0x5200, 0xFE00, "STRH <Rt>, [<Rn>,{,<Rm>}"}
};

struct instruction32bit t16UnconditionalInstructions[] = {
{1, &bInstruction, 0xE000, 0xE000, "B<c> #<imm8>"}
};

struct instruction32bit t16ArithmeticInstructions[] = {
// According to Thumb-2 spec, R15 cannot be used in arithmetic
// instructions. So dont trap
{0, &addInstruction, 0x1800, 0xFF00, "ADD Rd, Rn, Rm"},
// page 310. Rd can be R15
{1, &addInstruction, 0x4487, 0x4487, "ADD<c> R15, Rm"},
// Otherwise don't trap
{0, &addInstruction, 0x4400, 0x4400, "ADD<c> Rd, Rm"},
// CMP is fine
{0, &cmpInstruction, 0x2800, 0xF800, "CMP<c> Rn, #<imm8>"},
{0, &subInstruction, 0x1E00, 0xFE00, "SUB{S} <Rd>, <Rn>,#<imm3>"},
{0, &subInstruction, 0x3800, 0xF800, "SUB{S} <Rdn>, #<imm8>"},
{0, &subInstruction, 0x1A00, 0xFE00, "SUB{S} <Rd>, <Rn> <Rm>"},
{0, &movInstruction, 0x2000, 0xF800, "MOV<c> <Rd>, #<imm8>"}
};

struct instruction32bit t32DataProcInstructions[] = {
// this is for T2 encoding, page 480
{0, &movInstruction, 0xF04F0000, 0xFBEF1000, "MOVW<c> <Rd>, #<imm12>"},
// this is for T3 encoding
{0, &movInstruction, 0xF2400000, 0xFBF00000, "MOVW<c> <Rd>, #<imm16>"},
{0, &movInstruction, 0xF04F0000, 0xFBEF8000, "MOV{S}/W <Rd>, #<immt12>"},
{0, &movInstruction, 0xF2400000, 0xFBF08000, "MOVW <Rd>, #<imm16>"},
{0, &movtInstruction, 0xF2C00000, 0xFBF08000, "MOVT<c> <Rd>, #<imm16>"},
{0, &orrInstruction, 0xEA400000, 0xFFE08000, "ORR{S} <Rd>, <Rn>{,<shift>}"},
{0, &orrInstruction, 0xF0400000, 0xFBE08000, "ORR{S} <Rd>, <Rn>,#<imm12>"},
{0, &andInstruction, 0xEA000000, 0xFFF00000, "AND{S}.W <Rd>. <Rn>. <Rm>"},
//trap for RD=15
{1, &andInstruction, 0xF0000F00, 0xFBE08F00, "AND{S}<c> PC, <Rm>, #<imm12>"},
{0, &andInstruction, 0xF0000000, 0xFBE08000, "AND{S}<c> <Rd>, <Rm>, #<imm12>"},
{0, &addInstruction, 0xF1000000, 0xFBE08000, "ADD{S}.W <Rd>, <Rn>, #<imm8>"},
{0, &addInstruction, 0xEB000000, 0xFFE08000, "ADD{S}.W <Rd>, <Rn>, <Rm>{, <shift>}"},
//ADD -> RD=PC -> CMN page 306
{1, addInstruction, 0xF1000F00, 0xFBE08F00, "ADD{S}.W PC, <Rn>, #<imm8>"},
// RN=SP -> unimplemented. Should be OK
{0, &addInstruction, 0xF10D8000, 0xFBEF8000, "ADD{S}.W <Rd>, SP, #<imm8>"},
// Encoding T4
{0, &addInstruction, 0xF2000000, 0xF2008000, "ADDW <Rd>, <Rn>, #<imm12>"},
// RC=PC
{1, &addInstruction, 0xF20F0000, 0xF20F8000, "ADDW PC, <Rn>, #<imm12>"},
// RN=SP -> should be ok
{0, &addInstruction, 0xF20D0000, 0xF20F8000, "ADDW <Rd>, SP, #<imm8>"},
{0, &bicInstruction, 0xF0200000, 0xFBE08000, "BIC{S} <Rd>, <Rn>, #<imm12>"},
{0, &rsbInstruction, 0xF1C00000, 0xFBE08000, "RSB <Rd>, <Rn>, #<imm12>"},
{0, &rsbInstruction, 0xEBC00000, 0xFFE08000, "RSB <Rd>, <Rn>, <Rm>{,<shift>}"},
{0, &subInstruction, 0xF1A00000, 0xFBE08000, "SUB{S}.W <Rd>, <Rn>, #<imm12>"},
{0, &subInstruction, 0xF2A00000, 0xFBE08000, "SUB{S}W <Rd>, <Rn>, #<imm12>"},
{0, &subInstruction, 0xEBA00000, 0xFFE08000, "SUB{S}.W <Rd>, <Rn>, <Rm>{,<shitft>}"},
// RN = SP -> should be ok
{0, &subInstruction, 0xF2AB0000, 0xFBEF8000, "SUBW <Rd>, SP, #<imm12>"},
{0, &subInstruction, 0xEBAB0000, 0xFFEF8000, "SUB{S} <Rd>, SP, <Rm>{,<shift>}"},
{0, &mvnInstruction, 0xEA6F0000, 0xFFEF8000, "MVN<c> <Rd>, <Rm>{,<shift>}"},
{0, &mvnInstruction, 0xE0AF0000, 0xFBEF8000, "MVN<c> <Rd>, #<imm12>"}
};

struct instruction32bit t32SingleStoreInstructions[] = {
{0, &strbInstruction, 0xF8800000, 0xFFF00000, "STRB Rt, [Rn, #<imm12>]"},
{0, &strbInstruction, 0xF8000800, 0xFFF00800, "STRB Rt, [Rn, +-#<imm8>]"},

{0, &strhInstruction, 0xF8A00000, 0xFFF00000, "STRH.W <Rt> [<Rn>, #<imm12>}]"},
{0, &strhInstruction, 0xF8200000, 0xFFF00000, "STRH.W <Rt> [<Rn>, #<imm8>}]!"}
};

struct instruction32bit t32LoadByteInstructions[] = {
{1, &ldrbInstruction, 0xF81FF000, 0xFEFFFFC0, "LDRB<c> PC ,#<label>"},
{0, &ldrbInstruction, 0xF8900000, 0xFFF00000, "LDRB<c> Rt, [Rn{,#<imm12>}]"},
{0, &ldrbInstruction, 0xF8100900, 0xFFF00900, "LDRB<c> Rt, [Rn,{#<imm12>}]"},
{0, &ldrbInstruction, 0xF8100C00, 0xFFF00E00, "LDRB<c> Rt, [Rn,{#<imm12>}]"}
};

struct instruction32bit t32LoadHalfWordInstructions[] = {
{0, &ldrhInstruction, 0xF8B00000, 0xFFF00000, "LDRH.W <Rt>, [<Rn>{. #<imm32>}]"},
{0, &ldrhInstruction, 0xF83F0000, 0xFEFF0000, "LDRH <Rt>, <label>"},
{0, &ldrhInstruction, 0xF8300000, 0xFFF00FE0, "LDRH, <Rt>, [<Rn>{,LSL #<imm2>}]"},
{0, &ldrhInstruction, 0xF9B00000, 0xFFF00000, "LDRSH<c> <Rt>, [<Rn>, #<imm12>]"},
{0, &ldrhInstruction, 0xF9300800, 0xFFFF0800, "LDRSH<c> <Rt>, [<Rn>, #<Rn>, +/-#imm8]"}, // Page 454, A8-168
{0, &ldrhInstruction, 0xF9300FC0, 0xFFF00FC0, "LDRSH<c> <Rt>, [<Rn>, <Rm>]"}
};

struct instruction32bit t32MultiplyInstructions[] = {
{0, &mulInstruction, 0xFB00F000, 0xFFF0F0F0, "MULW <Rd>, <Rn>, <Rm>"},
{0, &smullInstruction, 0xFB800000, 0xFFE000F0, "SMULL <RdLo>, <RdHi>, <Rn>, <Rm>"}
};

struct instruction32bit t32BranchMiscInstructions[] = {
{1, &bInstruction, 0xF0008000, 0xF800D000, "B <imm17>"},
{1, &bInstruction, 0xF0009000, 0xF800D000, "B <imm21>"},
{1, &bInstruction, 0xF000D000, 0xF800D000, "BL, #<imm21>"},
{1, &blxInstruction, 0xF000C000, 0xF800D000, "BLX, #<imm21>"}
};

struct instruction32bit t32LoadStoreDoubleExclusiveInstructions[] = {
{0, &ldrdInstruction, 0xE8500000, 0xFE500000, "LDRD <Rt>, <Rt2>, [<Rn>,{,#+/-<imm>}]"},
{0, &ldrdInstruction, 0xE85F0000, 0xFE7F0000, "LDRD <Rt>, <Rt2>, <label>"},
{0, &strdInstruction, 0xE8400000, 0xFE500000, "STRD <Rt>, <Rt2>, [<Rn>,{,#+/-<imm>}]"}
};


struct instruction32bit * decodeInstr(u32int instr,u16int *currInstrAddress)
{
  
  GCONTXT* gc = getGuestContext();
  currAddress = currInstrAddress;
  u32int catCode = decodeTopLevelCategory(instr, currAddress); 
  // Check the status of T bit again
  if(gc->CPSR & T_BIT)
  {
	switch(instr & THUMB32<<16)
	{
		// instr is 32-bit so THUMB32 defs need to be extended to 32-bit
		case THUMB32_1<<16:
		case THUMB32_2<<16:
		case THUMB32_3<<16:
		{
			switch(catCode){
				case T32_LOAD_STORE_MULTIPLE:
					return t32decodeLoadStoreMultiple(instr);
					break;
				case T32_LOAD_STORE_DOUBLE_EXCLUSIVE:
					return t32decodeLoadStoreDoubleExclusive(instr);
					break;
				case T32_DATA_PROC:
					return t32decodeDataProc(instr);
					break;
				case T32_COPROC:
					return t32decodeCoproc(instr);
					break;
				case T32_BRANCH_AND_MISC:
					return t32decodeBranchMisc(instr);
					break;
				case T32_STORE_SINGLE:
					return t32decodeStoreSingle(instr);
					break;
				case T32_SIMD_STRUCT_LOAD_STORE:
					return t32decodeSimdStructLoadStore(instr);
					break;
				case T32_LOAD_BYTE:
					return t32decodeLoadByte(instr);
					break;
				case T32_LOAD_HALFWORD:
					return t32decodeLoadHalfWord(instr);
					break;
				case T32_LOAD_WORD:
					return t32decodeLoadWord(instr);
					break;
				case T32_MULTIPLY:
					return t32decodeMultiply(instr);
					break;
				case T32_LONG_MULTIPLY:
					return t32decodeLongMultiply(instr);
					break;
				case T32_UNDEFINED:
				default:
					DIE_NOW(0,"Thumb-2 undefined 32-bit instruction");
			}
		}
		// 16-bit instruction
		default:
		{
			switch(catCode)
			{
				case T16_UNCONDITIONALS_CATEGORY:
					return t16decodeUnconditionals(instr);
					break;
				case T16_CONDITIONAL_BRANCH_AND_SVC_CALL:
					return t16decodeConditionalBranchSVC(instr);
					break;
				case T16_LOAD_MULTIPLE_REGISTERS:
					return t16decodeLoadMultipleRegisters(instr);
					break;
				case T16_STORE_MULTIPLE_REGISTERS:
					return t16decodeStoreMultipleRegisters(instr);
					break;
				case T16_MISC:
					return t16decodeMisc(instr);
					break;
				case T16_SP_ADDR:
					return t16decodeSPAddr(instr);
					break;
				case T16_PC_ADDR:
					return	t16decodePCAddr(instr);
					break;
				case T16_LOAD_STORE:
					return t16decodeLoadStore(instr);
					break;
				case T16_LDR:
					return	t16decodeLDR(instr);
					break;
				case T16_SPECIAL_AND_BRANCH_EXCHANGE:
					return t16decodeSpecialBranchExchange(instr);
					break;
				case T16_DATA_PROC:
					return t16decodeDataProc(instr);
					break;
				case T16_ARITHMETIC:
					return t16decodeArithmetic(instr);
					break;
				default:
					DIE_NOW(0,"Thumb-2 undefined 16-bit instruction");
			}
		}
	}
  }
  else
  {
  	switch(catCode)
	  {
    	case DATA_PROC_AND_MISC_CATEGORY:
   	    	return decodeDataProcMisc(instr);
   	    	break;
    	case LOAD_STORE_WORD_BYTE_CATEGORY:
      		return decodeLoadStoreWordByte(instr);
      		break;
    	case MEDIA_CATEGORY:
      		return decodeMedia(instr);
      		break;
	    case BRANCH_BLOCK_TRANSFER_CATEGORY:
    	    return decodeBranchBlockTransfer(instr);
		    break;
	    case SVC_COPROCESSOR_CATEGORY:
    		return decodeSvcCoproc(instr);
  		    break;
	    case UNCONDITIONALS_CATEGORY:
    		return decodeUnconditional(instr);
		    break;
	    case UNDEFINED_CATEGORY:
    	default:
		    DIE_NOW(getGuestContext(), "decoder: UNDEFINED category");
  	}
}

  return 0;
}

u32int decodeTopLevelCategory(u32int instr,u16int *currAddress)
{
  
  GCONTXT* gc = getGuestContext();
  int index = 0;
  /* LOOP through all ARM and Thumb instruction encoding categories */
#ifdef DECODER_DEBUG
  printf("Decoding: %08x@%08x\n", instr,(u32int)currAddress);
#endif
  while (TRUE)
  {
	if(gc->CPSR & T_BIT)
  	{
  		// we are in Thumb mode
		switch(instr & THUMB32<<16){
			// Extend definitions to 32-bit
			case THUMB32_1<<16:
			case THUMB32_2<<16:
			case THUMB32_3<<16:
			{
				if ( (instr & t32categories[index].mask) == t32categories[index].value)
				{
					return t32categories[index].categoryCode;
				}
				else
				{
					index++;
				}
				break;
			}
			default:
			{
				// instr is 32 bit but we need to fetch only the lower half word
				instr=0x0000FFFF & instr;
				if ( (instr & t16categories[index].mask) == t16categories[index].value)
				{
					return t16categories[index].categoryCode;
				}
				else
				{
					index++;
				}
				break;
			}
		}
  	}
	// We should be in ARM mode 
	else
	{
	    if ( (instr & categories[index].mask) == categories[index].value)
    	{
	      return categories[index].categoryCode;
    	}
	    else
    	{
	      index = index + 1;
    	}
	}
   }
	return UNDEFINED_CATEGORY;
}

struct instruction32bit * decodeDataProcMisc(u32int instr)
{
#ifdef DECODER_DEBUG
  dumpInstruction("decodeDataProcMisc", instr);
#endif
  u32int op = instr & 0x02000000;
  u32int index = 0;
  if (op == 0)
  {
    while (index < INDEX_OF(dataProcMiscInstructions_op0))
    {
      if ( (instr & dataProcMiscInstructions_op0[index].mask) == dataProcMiscInstructions_op0[index].value)
      {
        if (dataProcMiscInstructions_op0[index].mask == 0)
        {
          dumpInstruction("decodeDataProcMisc_0", instr);
        }
        return &dataProcMiscInstructions_op0[index];
      }
      else
      {
        index++;
      }
    }
  }
  else
  {
    while (index < INDEX_OF(dataProcMiscInstructions_op0))
    {
      if ( (instr & dataProcMiscInstructions_op1[index].mask) == dataProcMiscInstructions_op1[index].value)
      {
        if (dataProcMiscInstructions_op1[index].mask == 0)
        {
          dumpInstruction("decodeDataProcMisc_1", instr);
        }
        return &dataProcMiscInstructions_op1[index];
      }
      else
      {
        index = index + 1;
      }
    }
  }
  DIE_NOW(getGuestContext(), "decoder: decodeDataProcMisc unimplemented");
  return 0;
}


struct instruction32bit * decodeLoadStoreWordByte(u32int instr)
{
#ifdef DECODER_DEBUG
  dumpInstruction("decodeLoadStoreWordByte", instr);
#endif
  u32int index = 0;
  while (index < INDEX_OF(loadStoreWordByteInstructions))
  {
    if ( (instr & loadStoreWordByteInstructions[index].mask) == loadStoreWordByteInstructions[index].value)
    {
      if (loadStoreWordByteInstructions[index].mask == 0)
      {
        dumpInstruction("decodeLoadStoreWordByte", instr);
      }
      return &loadStoreWordByteInstructions[index];
    }
    else
    {
      index = index + 1;
    }
  }
  DIE_NOW(getGuestContext(), "decoder: decodeBranchBlockTransfer unimplemented");
  return 0;
}


struct instruction32bit * decodeMedia(u32int instr)
{
#ifdef DECODER_DEBUG
  dumpInstruction("decodeMedia", instr);
#endif
  u32int index = 0;
  while (index < INDEX_OF(mediaInstructions))
  {
    if ( (instr & mediaInstructions[index].mask) == mediaInstructions[index].value)
    {
      if (mediaInstructions[index].mask == 0)
      {
        dumpInstruction("decodeMedia", instr);
      }
      return &mediaInstructions[index];
    }
    else
    {
      index = index + 1;
    }
  }
  DIE_NOW(getGuestContext(), "decoder: decodeMedia unimplemented");
  return 0;
}


struct instruction32bit * decodeBranchBlockTransfer(u32int instr)
{
#ifdef DECODER_DEBUG
  dumpInstruction("decodeBranchBlockTransfer", instr);
#endif
  u32int index = 0;
  while (index < INDEX_OF(branchBlockTransferInstructions))
  {
    if ( (instr & branchBlockTransferInstructions[index].mask) == branchBlockTransferInstructions[index].value)
    {
      if (branchBlockTransferInstructions[index].mask == 0)
      {
        dumpInstruction("decodeBranchBlockTransfer", instr);
      }
      return &branchBlockTransferInstructions[index];
    }
    else
    {
      index = index + 1;
    }
  }
  DIE_NOW(getGuestContext(), "decoder: decodeBranchBlockTransfer unimplemented");
  return 0;
}


struct instruction32bit * decodeSvcCoproc(u32int instr)
{
#ifdef DECODER_DEBUG
  dumpInstruction("decodeSvcCoproc", instr);
#endif
  u32int index = 0;
  while (index < INDEX_OF(svcCoprocInstructions))
  {
    if ( (instr & svcCoprocInstructions[index].mask) == svcCoprocInstructions[index].value)
    {
      if (svcCoprocInstructions[index].mask == 0)
      {
        dumpInstruction("decodeSvcCoproc", instr);
      }
      return &svcCoprocInstructions[index];
    }
    else
    {
      index = index + 1;
    }
  }
  DIE_NOW(getGuestContext(), "decoder: decodeSvcCoproc unimplemented");
  return 0;
}

struct instruction32bit * decodeUnconditional(u32int instr)
{
#ifdef DECODER_DEBUG
  dumpInstruction("decodeUnconditional", instr);
#endif
  u32int index = 0;
  while (index < INDEX_OF(unconditionalInstructions))
  {
    if ( (instr & unconditionalInstructions[index].mask) == unconditionalInstructions[index].value)
    {
      if (unconditionalInstructions[index].mask == 0)
      {
        dumpInstruction("decodeUnconditional", instr);
      }
      return &unconditionalInstructions[index];
    }
    else
    {
      index = index + 1;
    }
  }
  
  DIE_NOW(getGuestContext(), "decoder: decodeUnconditional unimplemented");
  return 0;
}

void dumpInstruction(char * msg, u32int instr)
{
  GCONTXT* gc = getGuestContext();
  printf(msg);
  printf(": Instruction: %08x ", instr);
  dumpInstrString(gc, instr);
  printf("\n");
}

// Thumb-2 32-bit functions
struct instruction32bit * t32decodeLoadStoreMultiple(u32int instr)
{
	printf("Unimplemented: %08x@%08x\n", instr,(u32int)currAddress);
	DIE_NOW(0,"Unimplemented 1");
}

struct instruction32bit * t32decodeLoadStoreDoubleExclusive(u32int instr)
{
#ifdef DECODER_DEBUG
  dumpInstruction("t32decodeLoadStoreDoubleExclusivec %08x\n", instr);
#endif
  u32int index = 0;
  while (index < INDEX_OF(t32LoadStoreDoubleExclusiveInstructions))
  {
	if ( (instr & t32LoadStoreDoubleExclusiveInstructions[index].mask) == t32LoadStoreDoubleExclusiveInstructions[index].value)
    {
      if (t32LoadStoreDoubleExclusiveInstructions[index].mask == 0)
      {
        dumpInstruction("t32LoadStoreDoubleExclusive", instr);
      }
      return &t32LoadStoreDoubleExclusiveInstructions[index];
    }
    else
    {
      index = index + 1;
    }
  }
  printf("Unimplemented: %08x@%08x\n", instr,(u32int)currAddress);
  DIE_NOW(getGuestContext(), "decoder: t32LoadStoreDoubleExclusive unimplemented");
  return 0;
}

struct instruction32bit * t32decodeDataProc(u32int instr)
{

#ifdef DECODER_DEBUG
  dumpInstruction("t32decodeDataProc", instr);
#endif
  u32int index = 0;
  while (index < INDEX_OF(t32DataProcInstructions))
  {
	if ( (instr & t32DataProcInstructions[index].mask) == t32DataProcInstructions[index].value)
    {
      if (t32DataProcInstructions[index].mask == 0)
      {
        dumpInstruction("t32decodeDataProc", instr);
      }
      return &t32DataProcInstructions[index];
    }
    else
    {
      index = index + 1;
    }
  }
  printf("Unimplemented: %08x@%08x\n", instr,(u32int)currAddress);
  DIE_NOW(getGuestContext(), "decoder: t32decodeDataProc unimplemented");
  return 0;
}

struct instruction32bit * t32decodeCoproc(u32int instr)
{
	printf("Unimplemented: %08x@%08x\n", instr,(u32int)currAddress);
	DIE_NOW(0,"Unimplemented 4");
}

struct instruction32bit * t32decodeBranchMisc(u32int instr)
{
#ifdef DECODER_DEBUG
  dumpInstruction("t32BranchMisc", instr);
#endif
  u32int index = 0;
  while (index < INDEX_OF(t32BranchMiscInstructions))
  {
    if ( (instr & t32BranchMiscInstructions[index].mask) == t32BranchMiscInstructions[index].value)
    {
      if (t32BranchMiscInstructions[index].mask == 0)
      {
        dumpInstruction("decodet32BranchMisc", instr);
      }
      return &t32BranchMiscInstructions[index];
    }
    else
    {
      index = index + 1;
    }
  }
  printf("Unimplemented: %08x@%08x\n", instr,(u32int)currAddress);
  DIE_NOW(getGuestContext(), "decoder: t32BranchMiscInstruction unimplemented");
  return 0;
}

struct instruction32bit * t32decodeStoreSingle(u32int instr)
{

#ifdef DECODER_DEBUG
  dumpInstruction("t32decodeSingleStore", instr);
#endif
  u32int index = 0;
  while (index < INDEX_OF(t32SingleStoreInstructions))
  {
	if ( (instr & t32SingleStoreInstructions[index].mask) == t32SingleStoreInstructions[index].value)
    {
      if (t32SingleStoreInstructions[index].mask == 0)
      {
        dumpInstruction("t32decodeSingleStore", instr);
      }
      return &t32SingleStoreInstructions[index];
    }
    else
    {
      index = index + 1;
    }
  }
  printf("Unimplemented: %08x@%08x\n", instr,(u32int)currAddress);
  DIE_NOW(getGuestContext(), "decoder: t32decodeSingleStore unimplemented");
  return 0;
}

struct instruction32bit * t32decodeSimdStructLoadStore(u32int instr)
{
	printf("Unimplemented: %08x@%08x\n", instr,(u32int)currAddress);
	DIE_NOW(0,"Unimplemented 7");
}

struct instruction32bit * t32decodeLoadByte(u32int instr)
{

#ifdef DECODER_DEBUG
  dumpInstruction("t32decodeLoadByte", instr);
#endif
  u32int index = 0;
  while (index < INDEX_OF(t32LoadByteInstructions))
  {
	if ( (instr & t32LoadByteInstructions[index].mask) == t32LoadByteInstructions[index].value)
    {
      if (t32LoadByteInstructions[index].mask == 0)
      {
        dumpInstruction("t32LoadByte", instr);
      }
      return &t32LoadByteInstructions[index];
    }
    else
    {
      index = index + 1;
    }
  }
  printf("Unimplemented: %08x@%08x\n", instr,(u32int)currAddress);
  DIE_NOW(getGuestContext(), "decoder: t32decodeLoadByte unimplemented");
  return 0;
}

struct instruction32bit * t32decodeLoadHalfWord(u32int instr)
{
#ifdef DECODER_DEBUG
  dumpInstruction("t32LoadHalfWord", instr);
#endif
  u32int index = 0;
  while (index < INDEX_OF(t32LoadHalfWordInstructions))
  {
    if ( (instr & t32LoadHalfWordInstructions[index].mask) == t32LoadHalfWordInstructions[index].value)
    {
      if (t32LoadHalfWordInstructions[index].mask == 0)
      {
        dumpInstruction("decodet32BranchMisc", instr);
      }
      return &t32LoadHalfWordInstructions[index];
    }
    else
    {
      index = index + 1;
    }
  }
  printf("Unimplemented: %08x@%08x\n", instr,(u32int)currAddress);
  DIE_NOW(getGuestContext(), "decoder: t32LoadHalfWordInstruction unimplemented");
  return 0;
}

struct instruction32bit * t32decodeLoadWord(u32int instr)
{
	printf("Unimplemented: %08x@%08x\n", instr,(u32int)currAddress);
	DIE_NOW(0,"Unimplemented 10");
}

struct instruction32bit * t32decodeMultiply(u32int instr)
{
#ifdef DECODER_DEBUG
  dumpInstruction("t32decodeMultiply", instr);
#endif
  u32int index = 0;
  while (index < INDEX_OF(t32MultiplyInstructions))
  {
    if ( (instr & t32MultiplyInstructions[index].mask) == t32MultiplyInstructions[index].value)
    {
      if (t32MultiplyInstructions[index].mask == 0)
      {
        dumpInstruction("decodet32Multiply", instr);
      }
      return &t32MultiplyInstructions[index];
    }
    else
    {
      index = index + 1;
    }
  }
  printf("Unimplemented: %08x@%08x\n", instr,(u32int)currAddress);
  DIE_NOW(getGuestContext(), "decoder: t32MultiplyInstruction unimplemented");
  return 0;
}

struct instruction32bit * t32decodeLongMultiply(u32int instr)
{
	printf("Unimplemented: %08x@%08x\n", instr,(u32int)currAddress);
	DIE_NOW(0,"Unimplemented 12");
}


// Thumb-2 16-bit functions
struct instruction32bit * t16decodeUnconditionals(u16int instr)
{
#ifdef DECODER_DEBUG
  dumpInstruction("t16UnConditional", instr);
#endif
  u32int index = 0;
  while (index < INDEX_OF(t16UnconditionalInstructions))
  {
    if ( (instr & t16UnconditionalInstructions[index].mask) == t16UnconditionalInstructions[index].value)
    {
      if (t16UnconditionalInstructions[index].mask == 0)
      {
        dumpInstruction("decodet16UnConditional", instr);
      }
      return &t16UnconditionalInstructions[index];
    }
    else
    {
      index = index + 1;
    }
  }
  printf("Unimplemented: %08x@%08x\n", instr,(u32int)currAddress);
  DIE_NOW(getGuestContext(), "decoder: t16UnconditionalInstruction unimplemented");
  return 0;
}

struct instruction32bit * t16decodeConditionalBranchSVC(u16int instr)
{
#ifdef DECODER_DEBUG
  dumpInstruction("t16ConditionalBranchSvc", instr);
#endif
  u32int index = 0;
  while (index < INDEX_OF(t16ConditionalBranchSvcInstructions))
  {
    if ( (instr & t16ConditionalBranchSvcInstructions[index].mask) == t16ConditionalBranchSvcInstructions[index].value)
    {
      if (t16ConditionalBranchSvcInstructions[index].mask == 0)
      {
        dumpInstruction("decodet16ConditionalBranchSvc", instr);
      }
      return &t16ConditionalBranchSvcInstructions[index];
    }
    else
    {
      index = index + 1;
    }
  }
  printf("Unimplemented: %08x@%08x\n", instr,(u32int)currAddress);
  DIE_NOW(getGuestContext(), "decoder: t16BranchSvcInstruction unimplemented");
  return 0;
}

struct instruction32bit * t16decodeLoadMultipleRegisters(u16int instr)
{
	printf("Unimplemented: %08x@%08x\n", instr,(u32int)currAddress);
	DIE_NOW(0,"Unimplemented 15");
}

struct instruction32bit * t16decodeStoreMultipleRegisters(u16int instr)
{
	printf("Unimplemented: %08x@%08x\n", instr,(u32int)currAddress);
	DIE_NOW(0,"Unimplemented 16");
}

struct instruction32bit * t16decodeMisc(u16int instr)
{

#ifdef DECODER_DEBUG
  dumpInstruction("t16decodeMisc", instr);
#endif
  u32int index = 0;
  while (index < INDEX_OF(t16MiscInstructions))
  {
    if ( (instr & t16MiscInstructions[index].mask) == t16MiscInstructions[index].value)
    {
      if (t16MiscInstructions[index].mask == 0)
      {
        dumpInstruction("decodet16Misc", instr);
      }
      return &t16MiscInstructions[index];
    }
    else
    {
      index = index + 1;
    }
  }
  printf("Unimplemented: %08x@%08x\n", instr,(u32int)currAddress);
  DIE_NOW(getGuestContext(), "decoder: t16MiscInstruction unimplemented");
  return 0;
}

struct instruction32bit * t16decodeSPAddr(u16int instr)
{
#ifdef DECODER_DEBUG
	dumpInstruction("t16SPInstruction", instr);
#endif
  u32int index = 0;
  while (index < INDEX_OF(t16SPInstructions))
  {
    if ( (instr & t16SPInstructions[index].mask) == t16SPInstructions[index].value)
    {
      if (t16SPInstructions[index].mask == 0)
      {
        dumpInstruction("decodet16SPInstruction", instr);
      }
      return &t16SPInstructions[index];
    }
    else
    {
      index = index + 1;
    }
  }
  printf("Unimplemented: %08x@%08x\n", instr,(u32int)currAddress);
  DIE_NOW(getGuestContext(), "decoder: t16SPInstruction unimplemented");
  return 0;
}

struct instruction32bit * t16decodePCAddr(u16int instr)
{
#ifdef DECODER_DEBUG
  dumpInstruction("t16decodePCAddr", instr);
#endif
  u32int index = 0;
  while (index < INDEX_OF(t16PCRelInstructions))
  {
    if ( (instr & t16PCRelInstructions[index].mask) == t16PCRelInstructions[index].value)
    {
      if (t16PCRelInstructions[index].mask == 0)
      {
        dumpInstruction("decodet16PCRel", instr);
      }
      return &t16PCRelInstructions[index];
    }
    else
    {
      index = index + 1;
    }
  }
  printf("Unimplemented: %08x@%08x\n", instr,(u32int)currAddress);
  DIE_NOW(getGuestContext(), "decoder: t16PCRel unimplemented");
  return 0;
}

struct instruction32bit * t16decodeLoadStore(u16int instr)
{
#ifdef DECODER_DEBUG
  dumpInstruction("t16decodeLoadStore", instr);
#endif
  u32int index = 0;
  while (index < INDEX_OF(t16LoadStoreInstructions))
  {
    if ( (instr & t16LoadStoreInstructions[index].mask) == t16LoadStoreInstructions[index].value)
    {
      if (t16LoadStoreInstructions[index].mask == 0)
      {
        dumpInstruction("decodet16LoadStore", instr);
      }
      return &t16LoadStoreInstructions[index];
    }
    else
    {
      index = index + 1;
    }
  }
  printf("Unimplemented: %08x@%08x\n", instr,(u32int)currAddress);
  DIE_NOW(getGuestContext(), "decoder: t16LoadStore unimplemented");
  return 0;
}

struct instruction32bit * t16decodeLDR(u16int instr)
{
#ifdef DECODER_DEBUG
  dumpInstruction("t16decodeLDR", instr);
#endif
  u32int index = 0;
  while (index < INDEX_OF(t16ldrInstructions))
  {
    if ( (instr & t16ldrInstructions[index].mask) == t16ldrInstructions[index].value)
    {
      if (t16ldrInstructions[index].mask == 0)
      {
        dumpInstruction("decodet16LDR", instr);
      }
      return &t16ldrInstructions[index];
    }
    else
    {
      index = index + 1;
    }
  }
  printf("Unimplemented: %08x@%08x\n", instr,(u32int)currAddress);
  DIE_NOW(getGuestContext(), "decoder: t16LDR unimplemented");
  return 0;
}

struct instruction32bit * t16decodeSpecialBranchExchange(u16int instr)
{

#ifdef DECODER_DEBUG
  dumpInstruction("t16decodeSpecialBranch", instr);
#endif
  u32int index = 0;
  while (index < INDEX_OF(t16SpecialBranchInstructions))
  {
	if ( (instr & t16SpecialBranchInstructions[index].mask) == t16SpecialBranchInstructions[index].value)
    {
      if (t16SpecialBranchInstructions[index].mask == 0)
      {
        dumpInstruction("decodet16SpecialBranch", instr);
      }
      return &t16SpecialBranchInstructions[index];
    }
    else
    {
      index = index + 1;
    }
  }
  printf("Unimplemented: %08x@%08x\n", instr,(u32int)currAddress);
  DIE_NOW(getGuestContext(), "decoder: t16SpecialBranch unimplemented");
  return 0;
}

struct instruction32bit * t16decodeDataProc(u16int instr)
{
#ifdef DECODER_DEBUG
  dumpInstruction("t16decodeDataProc", instr);
#endif
  u32int index = 0;
  while (index < INDEX_OF(t16DataProcInstructions))
  {
    if ( (instr & t16DataProcInstructions[index].mask) == t16DataProcInstructions[index].value)
    {
      if (t16DataProcInstructions[index].mask == 0)
      {
        dumpInstruction("decodet16DataProc", instr);
      }
      return &t16DataProcInstructions[index];
    }
    else
    {
      index = index + 1;
    }
  }
  printf("Unimplemented: %08x@%08x\n", instr,(u32int)currAddress);
  DIE_NOW(getGuestContext(), "decoder: t16DataProc unimplemented");
  return 0;
}

struct instruction32bit * t16decodeArithmetic(u16int instr)
{
#ifdef DECODER_DEBUG
  dumpInstruction("t16decodeArithmetic", instr);
#endif
  u32int index = 0;
  while (index < INDEX_OF(t16ArithmeticInstructions))
  {
    if ( (instr & t16ArithmeticInstructions[index].mask) == t16ArithmeticInstructions[index].value)
    {
      if (t16ArithmeticInstructions[index].mask == 0)
      {
        dumpInstruction("decodet16Arithmetic", instr);
      }
      return &t16ArithmeticInstructions[index];
    }
    else
    {
      index = index + 1;
    }
  }
  printf("Unimplemented: %08x@%08x\n", instr,(u32int)currAddress);
  DIE_NOW(getGuestContext(), "decoder: t16Arithmetic unimplemented");
  return 0;
}
