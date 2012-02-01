#include "common/debug.h"
#include "common/stddef.h"

#include "instructionEmu/decoder.h"
#include "instructionEmu/coprocInstructions.h"
#include "instructionEmu/dataProcessInstr.h"
#include "instructionEmu/miscInstructions.h"
#include "instructionEmu/tableSearchBlockCopyDecoder.h"

#include "instructionEmu/interpreter/branchInstructions.h"
#include "instructionEmu/interpreter/branchPCInstructions.h"
#include "instructionEmu/interpreter/loadInstructions.h"
#include "instructionEmu/interpreter/loadPCInstructions.h"
#include "instructionEmu/interpreter/storeInstructions.h"
#include "instructionEmu/interpreter/storePCInstructions.h"


static u32int decodeTopLevelCategory(u32int instr);


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


struct instruction32bit unconditionalInstructions[] = {
// UNIMPLEMENTED: data memory barrier
{1, dmbInstruction, dmbPCInstruction,         0xf57ff050, 0xfffffff0, "dmb\t%U"},
// sync barriers
{1, dsbInstruction, dsbPCInstruction,         0xf57ff040, 0xfffffff0, "DSB"},
{1, isbInstruction, isbPCInstruction,         0xf57ff060, 0xfffffff0, "ISB"},
// UNIMPLEMENTED: CLREX clear exclusive
{1, clrexInstruction, clrexPCInstruction,       0xf57ff01f, 0xffffffff, "clrex"},
// CPS: change processor state
{1, cpsInstruction, cpsiePCInstruction,       0xf1080000, 0xfffffe3f, "CPSIE"},
{1, cpsInstruction, cpsiePCInstruction,       0xf10a0000, 0xfffffe20, "CPSIE"},
{1, cpsInstruction, cpsidPCInstruction,       0xf10C0000, 0xfffffe3f, "CPSID"},
{1, cpsInstruction, cpsidPCInstruction,       0xf10e0000, 0xfffffe20, "CPSID"},
{1, cpsInstruction, cpsPCInstruction,         0xf1000000, 0xfff1fe20, "CPS"},
// UNIMPLEMENTED: RFE return from exception
{1, rfeInstruction, rfePCInstruction,         0xf8100a00, 0xfe50ffff, "rfe%23?id%24?ba\t%16-19r%21'!"},
// UNIMPLEMENTED: SETEND set endianess
{1, setendInstruction, setendPCInstruction,      0xf1010000, 0xfffffc00, "setend\t%9?ble"},
// UNIMPLEMENTED: SRS store return state
{1, srsInstruction, srsPCInstruction,         0xf84d0500, 0xfe5fffe0, "srs%23?id%24?ba\t%16-19r%21'!, #%0-4d"},
// BLX: branch and link to Thumb subroutine
{1, armBlxImmediateInstruction, armBlxPCInstruction,         0xfa000000, 0xfe000000, "BLX #imm24"},
// PLD: preload data
{1, pldInstruction, pldPCInstruction,         0xf450f000, 0xfc70f000, "PLD"},
// PLI: preload instruction
{1, pliInstruction, pliPCInstruction,         0xf450f000, 0xfd70f000, "PLI"},
{1, undefinedInstruction, undefinedPCInstruction,   0x00000000, 0x00000000, UNDEFINED_INSTRUCTION}
};

// register/shifter register cases, etc
struct instruction32bit dataProcMiscInstructions_op0[] = {
// NOP is just a nop...
{0, nopInstruction, nopPCInstruction,         0xe1a00000, 0xffffffff, "NOP"},
// UNIMPLEMENTED: SWP swap
{1, swpInstruction, swpPCInstruction,         0x01000090, 0x0fb00ff0, "SWP"},
// UNIMPLEMENTED:
{1, armStrhtInstruction, armStrhtPCInstruction,       0x006000b0, 0x0f7000f0, "STRHT instruction"},
{1, armLdrhtInstruction, armLdrhtPCInstruction,       0x003000b0, 0x0f3000f0, "LDRHT instruction"},
// store and load exclusive: must be emulated - user mode faults
{1, armLdrexbInstruction, armLdrexbPCInstruction,      0x01d00f9f, 0x0ff00fff, "LDREXB"},
{1, armLdrexdInstruction, armLdrexdPCInstruction,      0x01b00f9f, 0x0ff00fff, "LDREXD"},
{1, armLdrexhInstruction, armLdrexhPCInstruction,      0x01f00f9f, 0x0ff00fff, "LDREXH"},
{1, armStrexbInstruction, armStrexbPCInstruction,      0x01c00f90, 0x0ff00ff0, "STREXB"},
{1, armStrexdInstruction, armStrexdPCInstruction,      0x01a00f90, 0x0ff00ff0, "STREXD"},
{1, armStrexhInstruction, armStrexhPCInstruction,      0x01e00f90, 0x0ff00ff0, "STREXH"},
{1, armLdrexInstruction, armLdrexPCInstruction,       0x01900f9f, 0x0ff00fff, "LDREX"},
{1, armStrexInstruction, armStrexPCInstruction,       0x01800f90, 0x0ff00ff0, "STREX"},
// SMULL - signed multiply, PC cannot be used as any destination
{0, sumullInstruction, sumullPCInstruction,      0x00800090, 0x0fa000f0, "SMULL"},
// SMLAL - signed multiply and accumulate, PC cannot be used as any destination
{0, sumlalInstruction, sumlalPCInstruction,      0x00a00090, 0x0fa000f0, "SMLAL"},
// MUL: Rd = Rm * Rn; Rd != PC. pass through
{0, mulInstruction, mulPCInstruction,         0x00000090, 0x0fe000f0, "MUL Rd, Rm, Rn"},
// MLA: Rd = Rm * Rn + Ra; Rd != PC. pass through
{0, mlaInstruction, mlaPCInstruction,         0x00200090, 0x0fe000f0, "MLA Rd, Rm, Rn, Ra"},
// MLS: Rd = Rm * Rn - Ra; Rd != PC, pass through
{0, mlsInstruction, mlsPCInstruction,         0x00600090, 0x0ff000f0, "MLS Rd, Rm, Rn, Ra"},
// Branch and try to exchange to ARM mode.
{1, armBxInstruction, armBxPCInstruction,          0x012FFF10, 0x0ffffff0, "bx%c\t%0-3r"},
// Branch and link and try to exchange to Jazelle mode.
{1, armBxjInstruction, armBxjPCInstruction,         0x012fff20, 0x0ffffff0, "BXJ Rm"},
// Software breakpoint instruction... not sure yet.
{1, bkptInstruction, bkptPCInstruction,        0xe1200070, 0xfff000f0, "BKPT #imm8"},
// UNIMPLEMENTED: SMC, previously SMI: secure monitor call
{1, smcInstruction, smcPCInstruction,         0x01600070, 0x0ff000f0, "smc%c\t%e"},
// Branch and link and try to exchange with Thumb mode.
{1, armBlxRegisterInstruction, armBlxPCInstruction,         0x012fff30, 0x0ffffff0, "BLX Rm"},
// CLZ: Count leading zeroes - Rd, Rm != PC, pass through
{0, clzInstruction, clzPCInstruction,         0x016f0f10, 0x0fff0ff0, "CLZ Rd, Rm"},
// UNIMPLEMENTED: saturated add/subtract/doubleAdd/doubleSubtract
{1, qaddInstruction, qaddPCInstruction,        0x01000050, 0x0ff00ff0, "qadd%c\t%12-15r, %0-3r, %16-19r"},
{1, qdaddInstruction, qdaddPCInstruction,       0x01400050, 0x0ff00ff0, "qdadd%c\t%12-15r, %0-3r, %16-19r"},
{1, qsubInstruction, qsubPCInstruction,        0x01200050, 0x0ff00ff0, "qsub%c\t%12-15r, %0-3r, %16-19r"},
{1, qdsubInstruction, qdsubPCInstruction,       0x01600050, 0x0ff00ff0, "qdsub%c\t%12-15r, %0-3r, %16-19r"},
// LDRD: Rt1 must be even numbered and NOT 14, thus Rt2 cannot be PC. pass.
{0, armLdrdInstruction, armLdrdPCInstruction,        0x004000d0, 0x0e5000f0, "LDRD Rt, [Rn, #imm]"},
{0, armLdrdInstruction, armLdrdPCInstruction,        0x000000d0, 0x0e500ff0, "LDRD Rt, [Rn, Rm]"},
// STRD: pass through, let them fail!
{0, armStrdInstruction, armStrdPCInstruction,        0x004000f0, 0x0e5000f0, "STRD Rt, [Rn, #imm]"},
{0, armStrdInstruction, armStrdPCInstruction,        0x000000f0, 0x0e500ff0, "STRD Rt, [Rn, Rm]"},
/* ALL UNIMPLEMENTED: smlabs, smulbs etc */
// signed 16 bit multiply, 32 bit accumulate
{1, smlabbInstruction, smlabbPCInstruction,      0x01000080, 0x0ff000f0, "smlabb%c\t%16-19r, %0-3r, %8-11r, %12-15r"},
{1, smlatbInstruction, smlatbPCInstruction,      0x010000a0, 0x0ff000f0, "smlatb%c\t%16-19r, %0-3r, %8-11r, %12-15r"},
{1, smlabtInstruction, smlabtPCInstruction,      0x010000c0, 0x0ff000f0, "smlabt%c\t%16-19r, %0-3r, %8-11r, %12-15r"},
{1, smlattInstruction, smlattPCInstruction,      0x010000e0, 0x0ff000f0, "smlatt%c\t%16-19r, %0-3r, %8-11r, %12-15r"},
// signed 16 bit x 32 bit multiply, 32 bit accumulate
{1, smlawbInstruction, smlawbPCInstruction,      0x01200080, 0x0ff000f0, "smlawb%c\t%16-19r, %0-3r, %8-11r, %12-15r"},
{1, smlawtInstruction, smlawtPCInstruction,      0x012000c0, 0x0ff000f0, "smlawt%c\t%16-19r, %0-3r, %8-11r, %12-15r"},
// signed 16 bit multiply, 64 bit accumulate
{1, smlalbbInstruction, smlalbbPCInstruction,     0x01400080, 0x0ff000f0, "smlalbb%c\t%12-15r, %16-19r, %0-3r, %8-11r"},
{1, smlaltbInstruction, smlaltbPCInstruction,     0x014000a0, 0x0ff000f0, "smlaltb%c\t%12-15r, %16-19r, %0-3r, %8-11r"},
{1, smlalbtInstruction, smlalbtPCInstruction,     0x014000c0, 0x0ff000f0, "smlalbt%c\t%12-15r, %16-19r, %0-3r, %8-11r"},
{1, smlalttInstruction, smlalttPCInstruction,     0x014000e0, 0x0ff000f0, "smlaltt%c\t%12-15r, %16-19r, %0-3r, %8-11r"},
// SMULBB: multiply signed bottom halfwords of ops. Rd can't be PC.
{0, smulbbInstruction, smulbbPCInstruction,      0x01600080, 0x0ff0f0f0, "SMULBB Rd, Rn, RM"},
// signed 16 bit multiply, 32 bit result
{1, smultbInstruction, smultbPCInstruction,      0x016000a0, 0x0ff0f0f0, "smultb%c\t%16-19r, %0-3r, %8-11r"},
{1, smulbtInstruction, smulbtPCInstruction,      0x016000c0, 0x0ff0f0f0, "smulbt%c\t%16-19r, %0-3r, %8-11r"},
{1, smulttInstruction, smulttPCInstruction,      0x016000e0, 0x0ff0f0f0, "smultt%c\t%16-19r, %0-3r, %8-11r"},
// signed 16 bit x 32 bit multiply, 32 bit result
{1, smulwbInstruction, smulwbPCInstruction,      0x012000a0, 0x0ff0f0f0, "smulwb%c\t%16-19r, %0-3r, %8-11r"},
{1, smulwtInstruction, smulwtPCInstruction,      0x012000e0, 0x0ff0f0f0, "smulwt%c\t%16-19r, %0-3r, %8-11r"},
// STRH: passthrough, will data abort if something wrong
{0, armStrhInstruction, armStrhPCInstruction,        0x004000b0, 0x0e5000f0, "STRH Rt, [Rn, +-imm8]"},
{0, armStrhInstruction, armStrhPCInstruction,        0x000000b0, 0x0e500ff0, "STRH Rt, [Rn], +-Rm"},
// LDRH cant load halfword to PC, passthrough
{0, armLdrhInstruction, armLdrhPCInstruction,        0x00500090, 0x0e500090, "LDRH Rt, [Rn, +-imm8]"},
{0, armLdrhInstruction, armLdrhPCInstruction,        0x00100090, 0x0e500f90, "LDRH Rt, [Rn], +-Rm"},
// AND: Rd = PC end block, others are fine
{1, andInstruction, andPCInstruction,         0x0000f000, 0x0fe0f010, "AND PC, Rn, Rm, #shamt"},
{1, andInstruction, andPCInstruction,         0x0000f010, 0x0fe0f090, "AND PC, Rn, Rm, Rshamt"},
{0, andInstruction, andPCInstruction,         0x00000000, 0x0fe00010, "AND Rd, Rn, Rm, #shamt"},
{0, andInstruction, andPCInstruction,         0x00000010, 0x0fe00090, "AND Rd, Rn, Rm, Rshamt"},
// EOR: Rd = PC end block, others are fine
{1, eorInstruction, eorPCInstruction,         0x0020f000, 0x0fe0f010, "EOR PC, Rn, Rm, #shamt"},
{1, eorInstruction, eorPCInstruction,         0x0020f010, 0x0fe0f090, "EOR PC, Rn, Rm, Rshamt"},
{0, eorInstruction, eorPCInstruction,         0x00200000, 0x0fe00010, "EOR Rd, Rn, Rm, #shamt"},
{0, eorInstruction, eorPCInstruction,         0x00200010, 0x0fe00090, "EOR Rd, Rn, Rm, Rshamt"},
// SUB: Rd = PC end block, others are fine
{1, subInstruction, subPCInstruction,         0x0040f000, 0x0fe0f010, "SUB PC, Rn, Rm, #shamt"},
{1, subInstruction, subPCInstruction,         0x0040f010, 0x0fe0f090, "SUB PC, Rn, Rm, Rshamt"},
{0, subInstruction, subPCInstruction,         0x00400000, 0x0fe00010, "SUB Rd, Rn, Rm, #shamt"},
{0, subInstruction, subPCInstruction,         0x00400010, 0x0fe00090, "SUB Rd, Rn, Rm, Rshamt"},
// RSB: Rd = PC end block, others are fine
{1, rsbInstruction, rsbPCInstruction,         0x0060f000, 0x0fe0f010, "RSB PC, Rn, Rm, #shamt"},
{1, rsbInstruction, rsbPCInstruction,         0x0060f010, 0x0fe0f090, "RSB PC, Rn, Rm, Rshamt"},
{0, rsbInstruction, rsbPCInstruction,         0x00600000, 0x0fe00010, "RSB Rd, Rn, Rm, #shamt"},
{0, rsbInstruction, rsbPCInstruction,         0x00600010, 0x0fe00090, "RSB Rd, Rn, Rm, Rshamt"},
// ADD: Rd = PC end block, others are fine
{1, addInstruction, addPCInstruction,         0x0080f000, 0x0fe0f010, "ADD PC, Rn, Rm, #shamt"},
{1, addInstruction, addPCInstruction,         0x0080f010, 0x0fe0f090, "ADD PC, Rn, Rm, Rshamt"},
{0, addInstruction, addPCInstruction,         0x00800000, 0x0fe00010, "ADD Rd, Rn, Rm, #shamt"},
{0, addInstruction, addPCInstruction,         0x00800010, 0x0fe00090, "ADD Rd, Rn, Rm, Rshamt"},
// ADC: Rd = PC end block, others are fine
{1, adcInstruction, adcPCInstruction,         0x00a0f000, 0x0fe0f010, "ADC PC, Rn, Rm, #shamt"},
{1, adcInstruction, adcPCInstruction,         0x00a0f010, 0x0fe0f090, "ADC PC, Rn, Rm, Rshamt"},
{0, adcInstruction, adcPCInstruction,         0x00a00000, 0x0fe00010, "ADC Rd, Rn, Rm, #shamt"},
{0, adcInstruction, adcPCInstruction,         0x00a00010, 0x0fe00090, "ADC Rd, Rn, Rm, Rshamt"},
// SBC: Rd = PC end block, others are fine
{1, sbcInstruction, sbcPCInstruction,         0x00c0f000, 0x0fe0f010, "SBC Rd, Rn, Rm, #shamt"},
{1, sbcInstruction, sbcPCInstruction,         0x00c0f010, 0x0fe0f090, "SBC Rd, Rn, Rm, Rshamt"},
{0, sbcInstruction, sbcPCInstruction,         0x00c00000, 0x0fe00010, "SBC Rd, Rn, Rm, #shamt"},
{0, sbcInstruction, sbcPCInstruction,         0x00c00010, 0x0fe00090, "SBC Rd, Rn, Rm, Rshamt"},
// RSC: Rd = PC end block, others are fine
{1, rscInstruction, rscPCInstruction,         0x00e0f000, 0x0fe0f010, "RSC PC, Rn, Rm, #shamt"},
{1, rscInstruction, rscPCInstruction,         0x00e0f010, 0x0fe0f090, "RSC PC, Rn, Rm, Rshamt"},
{0, rscInstruction, rscPCInstruction,         0x00e00000, 0x0fe00010, "RSC Rd, Rn, Rm, #shamt"},
{0, rscInstruction, rscPCInstruction,         0x00e00010, 0x0fe00090, "RSC Rd, Rn, Rm, Rshamt"},
// MSR/MRS: always hypercall! we must hide the real state from guest.
{1, msrInstruction, msrPCInstruction,         0x0120f000, 0x0fb0fff0, "MSR, s/cpsr, Rn"},
{1, mrsInstruction, mrsPCInstruction,         0x010f0000, 0x0fbf0fff, "MRS, Rn, s/cpsr"},
// TST instructions are all fine
{0, tstInstruction, tstPCInstruction,         0x01000000, 0x0fe00010, "TST Rn, Rm, #shamt"},
{0, tstInstruction, tstPCInstruction,         0x01000010, 0x0fe00090, "TST Rn, Rm, Rshift"},
// TEQ instructions are all fine
{0, teqInstruction, teqPCInstruction,         0x01200000, 0x0fe00010, "TEQ Rn, Rm, #shamt"},
{0, teqInstruction, teqPCInstruction,         0x01200010, 0x0fe00090, "TEQ Rn, Rm, Rshift"},
// CMP instructions are all fine
{0, cmpInstruction, cmpPCInstruction,         0x01400000, 0x0fe00010, "CMP Rn, Rm, #shamt"},
{0, cmpInstruction, cmpPCInstruction,         0x01400010, 0x0fe00090, "CMP Rn, Rm, Rshamt"},
// CMN instructions are all fine
{0, cmnInstruction, cmnPCInstruction,         0x01600000, 0x0fe00010, "CMN Rn, Rm, #shamt"},
{0, cmnInstruction, cmnPCInstruction,         0x01600010, 0x0fe00090, "CMN Rn, Rm, Rshamt"},
// ORR: Rd = PC end block, other are fine
{1, orrInstruction, orrPCInstruction,         0x0180f000, 0x0fe0f010, "ORR PC, Rn, Rm, #shamt"},
{1, orrInstruction, orrPCInstruction,         0x0180f010, 0x0fe0f090, "ORR PC, Rn, Rm, Rshamt"},
{0, orrInstruction, orrPCInstruction,         0x01800000, 0x0fe00010, "ORR Rd, Rn, Rm, #shamt"},
{0, orrInstruction, orrPCInstruction,         0x01800010, 0x0fe00090, "ORR Rd, Rn, Rm, Rshamt"},
// MOV with Rd = PC end block. MOV reg, <shifted reg> is a pseudo instr for shift instructions
{1, movInstruction, movPCInstruction,         0x01a0f000, 0x0deffff0, "MOV PC, Rm"},
{0, movInstruction, movPCInstruction,         0x01a00000, 0x0def0ff0, "MOV Rn, Rm"},
// LSL: Rd = PC and shamt = #imm, then end block. Otherwise fine
{1, lslInstruction, lslPCInstruction,         0x01a0f000, 0x0feff070, "LSL Rd, Rm, #shamt"},
{1, lslInstruction, lslPCInstruction,         0x01a0f010, 0x0feff0f0, "LSL Rd, Rm, Rshamt"},
{0, lslInstruction, lslPCInstruction,         0x01a00000, 0x0fef0070, "LSL Rd, Rm, #shamt"},
{0, lslInstruction, lslPCInstruction,         0x01a00010, 0x0fef00f0, "LSL Rd, Rm, Rshamt"},
// LSR: Rd = PC and shamt = #imm, then end block. Otherwise fine
{1, lsrInstruction, lsrPCInstruction,         0x01a0f020, 0x0feff070, "LSR Rd, Rm, #shamt"},
{1, lsrInstruction, lsrPCInstruction,         0x01a0f030, 0x0feff0f0, "LSR Rd, Rm, Rshamt"},
{0, lsrInstruction, lsrPCInstruction,         0x01a00020, 0x0fef0070, "LSR Rd, Rm, #shamt"},
{0, lsrInstruction, lsrPCInstruction,         0x01a00030, 0x0fef00f0, "LSR Rd, Rm, Rshamt"},
// ASR: Rd = PC and shamt = #imm, then end block. Otherwise fine
{1, asrInstruction, asrPCInstruction,         0x01a0f040, 0x0feff070, "ASR Rd, Rm, #shamt"},
{1, asrInstruction, asrPCInstruction,         0x01a0f050, 0x0feff0f0, "ASR Rd, Rm, Rshamt"},
{0, asrInstruction, asrPCInstruction,         0x01a00040, 0x0fef0070, "ASR Rd, Rm, #shamt"},
{0, asrInstruction, asrPCInstruction,         0x01a00050, 0x0fef00f0, "ASR Rd, Rm, Rshamt"},
// RRX: shift right and extend, Rd can be PC
{1, rrxInstruction, rrxPCInstruction,         0x01a0f060, 0x0feffff0, "RRX PC, Rm"},
{0, rrxInstruction, rrxPCInstruction,         0x01a00060, 0x0fef0ff0, "RRX Rd, Rm"},
// ROR: reg case destination unpredictable. imm case dest can be PC.
{0, rorInstruction, rorPCInstruction,         0x01a00070, 0x0fef00f0, "ROR Rd, Rm, Rn"},
{1, rorInstruction, rorPCInstruction,         0x01a0f060, 0x0feff070, "ROR PC, Rm, #imm"},
{0, rorInstruction, rorPCInstruction,         0x01a00060, 0x0fef0070, "ROR Rd, Rm, #imm"},
// BIC with Rd = PC end block, other are fine.
{1, bicInstruction, bicPCInstruction,         0x01c0f000, 0x0fe0f010, "BIC PC, Rn, Rm, #shamt"},
{1, bicInstruction, bicPCInstruction,         0x01c0f010, 0x0fe0f090, "BIC PC, Rn, Rm, Rshamt"},
{0, bicInstruction, bicPCInstruction,         0x01c00000, 0x0fe00010, "BIC Rd, Rn, Rm, #shamt"},
{0, bicInstruction, bicPCInstruction,         0x01c00010, 0x0fe00090, "BIC Rd, Rn, Rm, Rshamt"},
// MVN with Rd = PC end block, other are fine.
{1, mvnInstruction, mvnPCInstruction,         0x01e0f000, 0x0fe0f010, "MVN Rd, Rm, #shamt"},
{1, mvnInstruction, mvnPCInstruction,         0x01e0f010, 0x0fe0f090, "MVN Rd, Rm, Rshamt"},
{0, mvnInstruction, mvnPCInstruction,         0x01e00000, 0x0fe00010, "MVN Rd, Rm, #shamt"},
{0, mvnInstruction, mvnPCInstruction,         0x01e00010, 0x0fe00090, "MVN Rd, Rm, Rshamt"},

{1, undefinedInstruction, undefinedPCInstruction,   0x00000000, 0x00000000, UNDEFINED_INSTRUCTION}
};

// immediate cases and random hints
struct instruction32bit dataProcMiscInstructions_op1[] = {
// UNIMPLEMENTED: yield hint
{1, yieldInstruction, yieldPCInstruction,       0x0320f001, 0x0fffffff, "yield%c"},
// UNIMPLEMENTED: wait for event hint
{1, wfeInstruction, wfePCInstruction,         0x0320f002, 0x0fffffff, "wfe%c"},
// UNIMPLEMENTED: wait for interrupt hint
{1, wfiInstruction, wfiPCInstruction,         0x0320f003, 0x0fffffff, "wfi%c"},
// UNIMPLEMENTED: send event hint
{1, sevInstruction, sevPCInstruction,         0x0320f004, 0x0fffffff, "sev%c"},
// UNIMPLEMENTED: nop hint
{1, nopInstruction, nopPCInstruction,         0x0320f000, 0x0fffff00, "nop%c\t{%0-7d}"},
// UNIMPLEMENTED: debug hint
{1, dbgInstruction, dbgPCInstruction,         0x0320f0f0, 0x0ffffff0, "dbg%c\t#%0-3d"},
// MOVW - indication of 'wide' - to select ARM encoding. Rd cant be PC, pass through.
{0, movwInstruction, movwPCInstruction,        0x03000000, 0x0ff00000, "MOVW Rd, Rn"},
// UNIMPLEMENTED: MOVT
{1, movtInstruction, movtPCInstruction,        0x03400000, 0x0ff00000, "movt%c\t%12-15r, %V"},
// AND: Rd = PC end block, others are fine
{1, andInstruction, andPCInstruction,         0x0200f000, 0x0fe0f000, "AND PC, Rn, #imm"},
{0, andInstruction, andPCInstruction,         0x02000000, 0x0fe00000, "AND Rd, Rn, #imm"},
// EOR: Rd = PC end block, others are fine
{1, eorInstruction, eorPCInstruction,         0x0220f000, 0x0fe0f000, "EOR PC, Rn, Rm/#imm"},
{0, eorInstruction, eorPCInstruction,         0x02200000, 0x0fe00000, "EOR Rd, Rn, #imm"},
// SUB: Rd = PC end block, others are fine
{1, subInstruction, subPCInstruction,         0x0240f000, 0x0fe0f000, "SUB PC, Rn, Rm/imm"},
{0, subInstruction, subPCInstruction,         0x02400000, 0x0fe00000, "SUB Rd, Rn, #imm"},
// RSB: Rd = PC end block, others are fine
{1, rsbInstruction, rsbPCInstruction,         0x0260f000, 0x0fe0f000, "RSB PC, Rn, Rm/imm"},
{0, rsbInstruction, rsbPCInstruction,         0x02600000, 0x0fe00000, "RSB Rd, Rn, #imm"},
// ADD: Rd = PC end block, others are fine
{1, addInstruction, addPCInstruction,         0x0280f000, 0x0fe0f000, "ADD PC, Rn, #imm"},
{0, addInstruction, addPCInstruction,         0x02800000, 0x0fe00000, "ADD Rd, Rn, #imm"},
// ADC: Rd = PC end block, others are fine
{1, adcInstruction, adcPCInstruction,         0x02a0f000, 0x0fe0f000, "ADC PC, Rn/#imm"},
{0, adcInstruction, adcPCInstruction,         0x02a00000, 0x0fe00000, "ADC Rd, Rn, #imm"},
// SBC: Rd = PC end block, others are fine
{1, sbcInstruction, sbcPCInstruction,         0x02c0f000, 0x0fe0f000, "SBC PC, Rn/#imm"},
{0, sbcInstruction, sbcPCInstruction,         0x02c00000, 0x0fe00000, "SBC Rd, Rn, #imm"},
// RSC: Rd = PC end block, others are fine
{1, rscInstruction, rscPCInstruction,         0x02e0f000, 0x0fe0f000, "RSC PC, Rn/#imm"},
{0, rscInstruction, rscPCInstruction,         0x02e00000, 0x0fe00000, "RSC Rd, Rn, #imm"},
// MSR: always hypercall! we must hide the real state from guest.
{1, msrInstruction, msrPCInstruction,         0x0320f000, 0x0fb0f000, "MSR, s/cpsr, #imm"},
// TST instructions are all fine
{0, tstInstruction, tstPCInstruction,         0x03000000, 0x0fe00000, "TST Rn, #imm"},
// TEQ instructions are all fine
{0, teqInstruction, teqPCInstruction,         0x03200000, 0x0fe00000, "TEQ Rn, #imm"},
// CMP instructions are all fine
{0, cmpInstruction, cmpPCInstruction,         0x03400000, 0x0fe00000, "CMP Rn, #imm"},
// CMN instructions are all fine
{0, cmnInstruction, cmnPCInstruction,         0x03600000, 0x0fe00000, "CMN Rn, #imm"},
// ORR: Rd = PC end block, other are fine
{1, orrInstruction, orrPCInstruction,         0x0380f000, 0x0fe0f000, "ORR Rd, Rn, #imm"},
{0, orrInstruction, orrPCInstruction,         0x03800000, 0x0fe00000, "ORR Rd, Rn, #imm"},
// MOV with Rd = PC end block. MOV <shifted reg> is a pseudo instr..
{1, movInstruction, movPCInstruction,         0x03a0f000, 0x0feff000, "MOV PC, #imm"},
{0, movInstruction, movPCInstruction,         0x03a00000, 0x0fef0000, "MOV Rn, #imm"},
// BIC with Rd = PC end block, other are fine.
{1, bicInstruction, bicPCInstruction,         0x03c0f000, 0x0fe0f000, "BIC PC, Rn, #imm"},
{0, bicInstruction, bicPCInstruction,         0x03c00000, 0x0fe00000, "BIC Rd, Rn, #imm"},
// MVN with Rd = PC end block, other are fine.
{1, mvnInstruction, mvnPCInstruction,         0x03e0f000, 0x0fe0f000, "MVN PC, Rn, #imm"},
{0, mvnInstruction, mvnPCInstruction,         0x03e00000, 0x0fe00000, "MVN Rd, Rn, #imm"},

{1, undefinedInstruction, undefinedPCInstruction,   0x00000000, 0x00000000, UNDEFINED_INSTRUCTION}
};

struct instruction32bit loadStoreWordByteInstructions[] = {
// STR imm12 and reg are pass-through
{0, armStrInstruction, armStrPCInstruction,         0x04000000, 0x0e100000, "STR Rt, [Rn, +-imm12]"},
{0, armStrInstruction, armStrPCInstruction,         0x06000000, 0x0e100ff0, "STR Rt, [Rn], +-Rm"},
{0, armStrInstruction, armStrPCInstruction,         0x04000000, 0x0c100010, "STR any? dont get this."},
{0, armStrbInstruction, armStrbPCInstruction,        0x04400000, 0x0e500000, "STRB Rt, [Rn, +-imm12]"},
{0, armStrbInstruction, armStrbPCInstruction,        0x06400000, 0x0e500010, "STRB Rt, [Rn], +-Rm"},
// LDR traps if dest = PC, otherwise pass through
{1, armLdrInstruction, armLdrPCInstruction,         0x0410f000, 0x0c10f000, "LDR PC, Rn/#imm12"},
{0, armLdrInstruction, armLdrPCInstruction,         0x04100000, 0x0c100000, "LDR Rd, Rn/#imm12"},

{1, undefinedInstruction, undefinedPCInstruction,   0x00000000, 0x00000000, UNDEFINED_INSTRUCTION}
};

struct instruction32bit mediaInstructions[] = {
// BFC: bit field clear, dest PC not allowed.
{0, bfcInstruction, bfcPCInstruction,         0x07c0001f, 0x0fe0007f, "BFC Rd, #LSB, #width"},
// BFI: bit field insert, dest PC not allowed.
{0, bfiInstruction, bfiPCInstruction,         0x07c00010, 0x0fe00070, "BFI Rd, #LSB, #width"},
// UNIMPLEMENTED: reverse bits
{1, rbitInstruction, rbitPCInstruction,        0x06ff0f30, 0x0fff0ff0, "rbit%c\t%12-15r, %0-3r"},
// UBFX: extract bit field - destination 15 unpredictable
{0, usbfxInstruction, usbfxPCInstruction,       0x07a00050, 0x0fa00070, "UBFX Rd, Rn, width"},
// UNIMPLEMENTED: pack halfword
{1, pkhbtInstruction, pkhbtPCInstruction,       0x06800010, 0x0ff00ff0, "pkhbt%c\t%12-15r, %16-19r, %0-3r"},
{1, pkhbtInstruction, pkhbtPCInstruction,       0x06800010, 0x0ff00070, "pkhbt%c\t%12-15r, %16-19r, %0-3r, lsl #%7-11d"},
{1, pkhtbInstruction, pkhtbPCInstruction,       0x06800050, 0x0ff00ff0, "pkhtb%c\t%12-15r, %16-19r, %0-3r, asr #32"},
{1, pkhtbInstruction, pkhtbPCInstruction,       0x06800050, 0x0ff00070, "pkhtb%c\t%12-15r, %16-19r, %0-3r, asr #%7-11d"},
// UNIMPLEMENTED: saturating add 16 bit and 8 bit
{1, qadd16Instruction, qadd16PCInstruction,     0x06200f10, 0x0ff00ff0, "qadd16%c\t%12-15r, %16-19r, %0-3r"},
{1, qadd8Instruction, qadd8PCInstruction,      0x06200f90, 0x0ff00ff0, "qadd8%c\t%12-15r, %16-19r, %0-3r"},
// UNIMPLEMENTED: saturating add and subtract with exchange
{1, qaddsubxInstruction, qaddsubxPCInstruction,    0x06200f30, 0x0ff00ff0, "qaddsubx%c\t%12-15r, %16-19r, %0-3r"},
// UNIMPLEMENTED: saturating subtract 16 and 8 bit
{1, qsub16Instruction, qsub16PCInstruction,     0x06200f70, 0x0ff00ff0, "qsub16%c\t%12-15r, %16-19r, %0-3r"},
{1, qsub8Instruction, qsub8PCInstruction,      0x06200ff0, 0x0ff00ff0, "qsub8%c\t%12-15r, %16-19r, %0-3r"},
// UNIMPLEMENTED: saturating subtract and add with exchange
{1, qsubaddxInstruction, qsubaddxPCInstruction,    0x06200f50, 0x0ff00ff0, "qsubaddx%c\t%12-15r, %16-19r, %0-3r"},
// UNIMPLEMENTED: signed add 16 bit and 8 bit
{1, sadd16Instruction, sadd16PCInstruction,     0x06100f10, 0x0ff00ff0, "sadd16%c\t%12-15r, %16-19r, %0-3r"},
{1, sadd8Instruction, sadd8PCInstruction,      0x06100f90, 0x0ff00ff0, "sadd8%c\t%12-15r, %16-19r, %0-3r"},
// signed add and subtract with exchange
{1, saddsubxInstruction, saddsubxPCInstruction,    0x06100f30, 0x0ff00ff0, "saddsubx%c\t%12-15r, %16-19r, %0-3r"},
// signed subtract 16 bit and 8 bit
{1, ssub16Instruction, ssub16PCInstruction,     0x06100f70, 0x0ff00ff0, "ssub16%c\t%12-15r, %16-19r, %0-3r"},
{1, ssub8Instruction, ssub8PCInstruction,      0x06100ff0, 0x0ff00ff0, "ssub8%c\t%12-15r, %16-19r, %0-3r"},
// signed subtract and add with exchange
{1, ssubaddxInstruction, ssubaddxPCInstruction,    0x06100f50, 0x0ff00ff0, "ssubaddx%c\t%12-15r, %16-19r, %0-3r"},
// UNIMPLEMENTED: signed halvign add 16 bit and 8 bit
{1, shadd16Instruction, shadd16PCInstruction,    0x06300f10, 0x0ff00ff0, "shadd16%c\t%12-15r, %16-19r, %0-3r"},
{1, shadd8Instruction, shadd8PCInstruction,     0x06300f90, 0x0ff00ff0, "shadd8%c\t%12-15r, %16-19r, %0-3r"},
// UNIMPLEMENTED: signed halving add and subtract with exchange
{1, shaddsubxInstruction, shaddsubxPCInstruction,   0x06300f30, 0x0ff00ff0, "shaddsubx%c\t%12-15r, %16-19r, %0-3r"},
// UNIMPLEMENTED: signed halvign subtract 16 bit and 8 bit
{1, shsub16Instruction, shsub16PCInstruction,    0x06300f70, 0x0ff00ff0, "shsub16%c\t%12-15r, %16-19r, %0-3r"},
{1, shsub8Instruction, shsub8PCInstruction,     0x06300ff0, 0x0ff00ff0, "shsub8%c\t%12-15r, %16-19r, %0-3r"},
// UNIMPLEMENTED: signed halving subtract and add with exchange
{1, shsubaddxInstruction, shsubaddxPCInstruction,   0x06300f50, 0x0ff00ff0, "shsubaddx%c\t%12-15r, %16-19r, %0-3r"},
// UNIMPLEMENTED: unsigned add 16 bit and 8 bit
{1, uadd16Instruction, uadd16PCInstruction,     0x06500f10, 0x0ff00ff0, "uadd16%c\t%12-15r, %16-19r, %0-3r"},
{1, uadd8Instruction, uadd8PCInstruction,      0x06500f90, 0x0ff00ff0, "uadd8%c\t%12-15r, %16-19r, %0-3r"},
// UNIMPLEMENTED: unsigned add and subtract with exchange
{1, uaddsubxInstruction, uaddsubxPCInstruction,    0x06500f30, 0x0ff00ff0, "uaddsubx%c\t%12-15r, %16-19r, %0-3r"},
// UNIMPLEMENTED: unsigned subtract 16 bit and 8 bit
{1, usub16Instruction, usub16PCInstruction,     0x06500f70, 0x0ff00ff0, "usub16%c\t%12-15r, %16-19r, %0-3r"},
{1, usub8Instruction, usub8PCInstruction,      0x06500ff0, 0x0ff00ff0, "usub8%c\t%12-15r, %16-19r, %0-3r"},
// UNIMPLEMENTED: unsigned subtract and add with exchange
{1, usubaddxInstruction, usubaddxPCInstruction,    0x06500f50, 0x0ff00ff0, "usubaddx%c\t%12-15r, %16-19r, %0-3r"},
// UNIMPLEMENTED: unsigned halving add 16 bit and 8 bit
{1, uhadd16Instruction, uhadd16PCInstruction,    0x06700f10, 0x0ff00ff0, "uhadd16%c\t%12-15r, %16-19r, %0-3r"},
{1, uhadd8Instruction, uhadd8PCInstruction,     0x06700f90, 0x0ff00ff0, "uhadd8%c\t%12-15r, %16-19r, %0-3r"},
// UNIMPLEMENTED: unsigned halving add and subtract with exchange
{1, uhaddsubxInstruction, uhaddsubxPCInstruction,   0x06700f30, 0x0ff00ff0, "uhaddsubxc\t%12-15r, %16-19r, %0-3r"},
// UNIMPLEMENTED: unsigned halving subtract 16 bit and 8 bit
{1, uhsub16Instruction, uhsub16PCInstruction,    0x06700f70, 0x0ff00ff0, "uhsub16%c\t%12-15r, %16-19r, %0-3r"},
{1, uhsub8Instruction, uhsub8PCInstruction,     0x06700ff0, 0x0ff00ff0, "uhsub8%c\t%12-15r, %16-19r, %0-3r"},
// UNIMPLEMENTED: unsigned halving subtract and add with exchange
{1, uhsubaddxInstruction, uhsubaddxPCInstruction,   0x06700f50, 0x0ff00ff0, "uhsubaddx%c\t%12-15r, %16-19r, %0-3r"},
// UNIMPLEMENTED:  unsigned saturating add 16 bit and 8 bit
{1, uqadd16Instruction, uqadd16PCInstruction,    0x06600f10, 0x0ff00ff0, "uqadd16%c\t%12-15r, %16-19r, %0-3r"},
{1, uqadd8Instruction, uqadd8PCInstruction,     0x06600f90, 0x0ff00ff0, "uqadd8%c\t%12-15r, %16-19r, %0-3r"},
// UNIMPLEMENTED: unsigned saturating add and subtract with exchange
{1, uqaddsubxInstruction, uqaddsubxPCInstruction,   0x06600f30, 0x0ff00ff0, "uqaddsubx%c\t%12-15r, %16-19r, %0-3r"},
// UNIMPLEMENTED: unsigned saturating subtract 16 bit and 8 bit
{1, uqsub16Instruction, uqsub16PCInstruction,    0x06600f70, 0x0ff00ff0, "uqsub16%c\t%12-15r, %16-19r, %0-3r"},
{1, uqsub8Instruction, uqsub8PCInstruction,     0x06600ff0, 0x0ff00ff0, "uqsub8%c\t%12-15r, %16-19r, %0-3r"},
// UNIMPLEMENTED: unsigned saturating subtract and add with exchange
{1, uqsubaddxInstruction, uqsubaddxPCInstruction,   0x06600f50, 0x0ff00ff0, "uqsubaddx%c\t%12-15r, %16-19r, %0-3r"},
// UNIMPLEMENTED: byte-reverse word
{1, revInstruction, revPCInstruction,         0x06bf0f30, 0x0fff0ff0, "rev%c\t%12-15r, %0-3r"},
// UNIMPLEMENTED:  byte-reverse packed halfword
{1, rev16Instruction, rev16PCInstruction,      0x06bf0fb0, 0x0fff0ff0, "rev16%c\t%12-15r, %0-3r"},
// UNIMPLEMENTED:  byte-reverse signed halfword
{1, revshInstruction, revshPCInstruction,       0x06ff0fb0, 0x0fff0ff0, "revsh%c\t%12-15r, %0-3r"},
// UNIMPLEMENTED: sign-extend halfword
{1, sxthInstruction, sxthPCInstruction,        0x06bf0070, 0x0fff0ff0, "sxth%c\t%12-15r, %0-3r"},
{1, sxthInstruction, sxthPCInstruction,        0x06bf0470, 0x0fff0ff0, "sxth%c\t%12-15r, %0-3r, ror #8"},
{1, sxthInstruction, sxthPCInstruction,        0x06bf0870, 0x0fff0ff0, "sxth%c\t%12-15r, %0-3r, ror #16"},
{1, sxthInstruction, sxthPCInstruction,        0x06bf0c70, 0x0fff0ff0, "sxth%c\t%12-15r, %0-3r, ror #24"},
// UNIMPLEMENTED: sign-extend byte 16
{1, sxtb16Instruction, sxtb16PCInstruction,     0x068f0070, 0x0fff0ff0, "sxtb16%c\t%12-15r, %0-3r"},
{1, sxtb16Instruction, sxtb16PCInstruction,     0x068f0470, 0x0fff0ff0, "sxtb16%c\t%12-15r, %0-3r, ror #8"},
{1, sxtb16Instruction, sxtb16PCInstruction,     0x068f0870, 0x0fff0ff0, "sxtb16%c\t%12-15r, %0-3r, ror #16"},
{1, sxtb16Instruction, sxtb16PCInstruction,     0x068f0c70, 0x0fff0ff0, "sxtb16%c\t%12-15r, %0-3r, ror #24"},
// SXTB: destination register PC not allowed,
{0, sxtbInstruction, sxtbPCInstruction,        0x06af0070, 0x0fff0ff0, "sxtb%c\t%12-15r, %0-3r"},
{0, sxtbInstruction, sxtbPCInstruction,        0x06af0470, 0x0fff0ff0, "sxtb%c\t%12-15r, %0-3r, ror #8"},
{0, sxtbInstruction, sxtbPCInstruction,        0x06af0870, 0x0fff0ff0, "sxtb%c\t%12-15r, %0-3r, ror #16"},
{0, sxtbInstruction, sxtbPCInstruction,        0x06af0c70, 0x0fff0ff0, "sxtb%c\t%12-15r, %0-3r, ror #24"},
// UXTH permitted, if Rd or Rn = 15 then unpredictable.
{0, uxthInstruction, uxthPCInstruction,        0x06ff0070, 0x0fff0ff0, "UXTH Rd, Rn"},
{0, uxthInstruction, uxthPCInstruction,        0x06ff0470, 0x0fff0ff0, "UXTH Rd, Rn, ROR #8"},
{0, uxthInstruction, uxthPCInstruction,        0x06ff0870, 0x0fff0ff0, "UXTH Rd, Rn, ROR #16"},
{0, uxthInstruction, uxthPCInstruction,        0x06ff0c70, 0x0fff0ff0, "UXTH Rd, Rn, ROR #24"},
// UXTB16 permitted, if Rd or Rn = 15 then unpredictable.
{0, uxtb16Instruction, uxtb16PCInstruction,     0x06cf0070, 0x0fff0ff0, "UXTB16 Rd, Rn"},
{0, uxtb16Instruction, uxtb16PCInstruction,     0x06cf0470, 0x0fff0ff0, "UXTB16 Rd, Rn, ROR #8"},
{0, uxtb16Instruction, uxtb16PCInstruction,     0x06cf0870, 0x0fff0ff0, "UXTB16 Rd, Rn, ROR #16"},
{0, uxtb16Instruction, uxtb16PCInstruction,     0x06cf0c70, 0x0fff0ff0, "UXTB16 Rd, Rn, ROR #24"},
// UXTB permitted, if Rd or Rn = 15 then unpredictable.
{0, uxtbInstruction, uxtbPCInstruction,        0x06ef0070, 0x0fff0ff0, "UXTB Rd, Rn"},
{0, uxtbInstruction, uxtbPCInstruction,        0x06ef0470, 0x0fff0ff0, "UXTB Rd, Rn, ROR #8"},
{0, uxtbInstruction, uxtbPCInstruction,        0x06ef0870, 0x0fff0ff0, "UXTB Rd, Rn, ROR #16"},
{0, uxtbInstruction, uxtbPCInstruction,        0x06ef0c70, 0x0fff0ff0, "UXTB Rd, Rn, ROR #24"},
// UNIMPLEMENTED: sign-extend and add halfword
{1, sxtahInstruction, sxtahPCInstruction,       0x06b00070, 0x0ff00ff0, "sxtah%c\t%12-15r, %16-19r, %0-3r"},
{1, sxtahInstruction, sxtahPCInstruction,       0x06b00470, 0x0ff00ff0, "sxtah%c\t%12-15r, %16-19r, %0-3r, ror #8"},
{1, sxtahInstruction, sxtahPCInstruction,       0x06b00870, 0x0ff00ff0, "sxtah%c\t%12-15r, %16-19r, %0-3r, ror #16"},
{1, sxtahInstruction, sxtahPCInstruction,       0x06b00c70, 0x0ff00ff0, "sxtah%c\t%12-15r, %16-19r, %0-3r, ror #24"},
// UNIMPLEMENTED: sign-extend and add byte 16
{1, sxtab16Instruction, sxtab16PCInstruction,    0x06800070, 0x0ff00ff0, "sxtab16%c\t%12-15r, %16-19r, %0-3r"},
{1, sxtab16Instruction, sxtab16PCInstruction,    0x06800470, 0x0ff00ff0, "sxtab16%c\t%12-15r, %16-19r, %0-3r, ror #8"},
{1, sxtab16Instruction, sxtab16PCInstruction,    0x06800870, 0x0ff00ff0, "sxtab16%c\t%12-15r, %16-19r, %0-3r, ror #16"},
{1, sxtab16Instruction, sxtab16PCInstruction,    0x06800c70, 0x0ff00ff0, "sxtab16%c\t%12-15r, %16-19r, %0-3r, ror #24"},
// UNIMPLEMENTED: sign-extend and add byte
{1, sxtabInstruction, sxtabPCInstruction,       0x06a00070, 0x0ff00ff0, "sxtab%c\t%12-15r, %16-19r, %0-3r"},
{1, sxtabInstruction, sxtabPCInstruction,       0x06a00470, 0x0ff00ff0, "sxtab%c\t%12-15r, %16-19r, %0-3r, ror #8"},
{1, sxtabInstruction, sxtabPCInstruction,       0x06a00870, 0x0ff00ff0, "sxtab%c\t%12-15r, %16-19r, %0-3r, ror #16"},
{1, sxtabInstruction, sxtabPCInstruction,       0x06a00c70, 0x0ff00ff0, "sxtab%c\t%12-15r, %16-19r, %0-3r, ror #24"},
// UNIMPLEMENTED: unsigned extend and add halfword
{1,  &uxtahInstruction, uxtahPCInstruction,      0x06f0f070, 0x0ff0fff0, "uxtah%c\t%12-15r, %16-19r, %0-3r"},
{0,  &uxtahInstruction, uxtahPCInstruction,      0x06f00070, 0x0ff00ff0, "uxtah%c\t%12-15r, %16-19r, %0-3r"},
{1,  &uxtahInstruction, uxtahPCInstruction,      0x06f0f470, 0x0ff0fff0, "uxtah%c\t%12-15r, %16-19r, %0-3r, ror #8"},
{0,  &uxtahInstruction, uxtahPCInstruction,      0x06f00470, 0x0ff00ff0, "uxtah%c\t%12-15r, %16-19r, %0-3r, ror #8"},
{1,  &uxtahInstruction, uxtahPCInstruction,      0x06f0f870, 0x0ff0fff0, "uxtah%c\t%12-15r, %16-19r, %0-3r, ror #16"},
{0,  &uxtahInstruction, uxtahPCInstruction,      0x06f00870, 0x0ff00ff0, "uxtah%c\t%12-15r, %16-19r, %0-3r, ror #16"},
{1,  &uxtahInstruction, uxtahPCInstruction,      0x06f0fc70, 0x0ff0fff0, "uxtah%c\t%12-15r, %16-19r, %0-3r, ror #24"},
{0,  &uxtahInstruction, uxtahPCInstruction,      0x06f00c70, 0x0ff00ff0, "uxtah%c\t%12-15r, %16-19r, %0-3r, ror #24"},
// UNIMPLEMENTED: unsigned extend and add byte 16
{1, uxtab16Instruction, uxtab16PCInstruction,    0x06c00070, 0x0ff00ff0, "uxtab16%c\t%12-15r, %16-19r, %0-3r"},
{1, uxtab16Instruction, uxtab16PCInstruction,    0x06c00470, 0x0ff00ff0, "uxtab16%c\t%12-15r, %16-19r, %0-3r, ror #8"},
{1, uxtab16Instruction, uxtab16PCInstruction,    0x06c00870, 0x0ff00ff0, "uxtab16%c\t%12-15r, %16-19r, %0-3r, ror #16"},
{1, uxtab16Instruction, uxtab16PCInstruction,    0x06c00c70, 0x0ff00ff0, "uxtab16%c\t%12-15r, %16-19r, %0-3r, ROR #24"},
// UNIMPLEMENTED: unsigned extend and add byte
{1, uxtabInstruction, uxtabPCInstruction,       0x06e00070, 0x0ff00ff0, "uxtab%c\t%12-15r, %16-19r, %0-3r"},
{1, uxtabInstruction, uxtabPCInstruction,       0x06e00470, 0x0ff00ff0, "uxtab%c\t%12-15r, %16-19r, %0-3r, ror #8"},
{1, uxtabInstruction, uxtabPCInstruction,       0x06e00870, 0x0ff00ff0, "uxtab%c\t%12-15r, %16-19r, %0-3r, ror #16"},
{1, uxtabInstruction, uxtabPCInstruction,       0x06e00c70, 0x0ff00ff0, "uxtab%c\t%12-15r, %16-19r, %0-3r, ror #24"},
// UNIMPLEMENTED: select bytes
{1, selInstruction, selPCInstruction,         0x06800fb0, 0x0ff00ff0, "sel%c\t%12-15r, %16-19r, %0-3r"},
// UNIMPLEMENTED: signed dual multiply add
{1, smuadInstruction, smuadPCInstruction,       0x0700f010, 0x0ff0f0d0, "smuad%5'x%c\t%16-19r, %0-3r, %8-11r"},
// UNIMPLEMENTED: signed dual multiply subtract
{1, smusdInstruction, smusdPCInstruction,       0x0700f050, 0x0ff0f0d0, "smusd%5'x%c\t%16-19r, %0-3r, %8-11r"},
// UNIMPLEMENTED: signed multiply accumulate dual
{1, smladInstruction, smladPCInstruction,       0x07000010, 0x0ff000d0, "smlad%5'x%c\t%16-19r, %0-3r, %8-11r, %12-15r"},
// UNIMPLEMENTED: signed multiply accumulate long dual
{1, smlaldInstruction, smlaldPCInstruction,      0x07400010, 0x0ff000d0, "smlald%5'x%c\t%12-15r, %16-19r, %0-3r, %8-11r"},
// UNIMPLEMENTED: signed multiply subtract dual
{1, smlsdInstruction, smlsdPCInstruction,       0x07000050, 0x0ff000d0, "smlsd%5'x%c\t%16-19r, %0-3r, %8-11r, %12-15r"},
// UNIMPLEMENTED: signed multiply subtract long dual
{1, smlsldInstruction, smlsldPCInstruction,      0x07400050, 0x0ff000d0, "smlsld%5'x%c\t%12-15r, %16-19r, %0-3r, %8-11r"},
// UNIMPLEMENTED: signed most significant word multiply
{1, smmulInstruction, smmulPCInstruction,       0x0750f010, 0x0ff0f0d0, "smmul%5'r%c\t%16-19r, %0-3r, %8-11r"},
// UNIMPLEMENTED: signed most significant word multiply accumulate
{1, smmlaInstruction, smmlaPCInstruction,       0x07500010, 0x0ff000d0, "smmla%5'r%c\t%16-19r, %0-3r, %8-11r, %12-15r"},
// UNIMPLEMENTED: signed most significant word multiply subtract
{1, smmlsInstruction, smmlsPCInstruction,       0x075000d0, 0x0ff000d0, "smmls%5'r%c\t%16-19r, %0-3r, %8-11r, %12-15r"},
// UNIMPLEMENTED: signed saturate
{1, ssatInstruction, ssatPCInstruction,        0x06a00010, 0x0fe00ff0, "ssat%c\t%12-15r, #%16-20W, %0-3r"},
{1, ssatInstruction, ssatPCInstruction,        0x06a00010, 0x0fe00070, "ssat%c\t%12-15r, #%16-20W, %0-3r, lsl #%7-11d"},
{1, ssatInstruction, ssatPCInstruction,        0x06a00050, 0x0fe00070, "ssat%c\t%12-15r, #%16-20W, %0-3r, asr #%7-11d"},
// UNIMPLEMENTED: signed saturate 16
{1, ssat16Instruction, ssat16PCInstruction,     0x06a00f30, 0x0ff00ff0, "ssat16%c\t%12-15r, #%16-19W, %0-3r"},
// UNIMPLEMENTED: unsigned saturate
{1, usatInstruction, usatPCInstruction,        0x06e00010, 0x0fe00ff0, "usat%c\t%12-15r, #%16-20d, %0-3r"},
{1, usatInstruction, usatPCInstruction,        0x06e00010, 0x0fe00070, "usat%c\t%12-15r, #%16-20d, %0-3r, lsl #%7-11d"},
{1, usatInstruction, usatPCInstruction,        0x06e00050, 0x0fe00070, "usat%c\t%12-15r, #%16-20d, %0-3r, asr #%7-11d"},
// UNIMPLEMENTED: unsigned saturate 16
{1, usat16Instruction, usat16PCInstruction,     0x06e00f30, 0x0ff00ff0, "usat16%c\t%12-15r, #%16-19d, %0-3r"},

{1, undefinedInstruction, undefinedPCInstruction,   0x00000000, 0x00000000, UNDEFINED_INSTRUCTION}
};

struct instruction32bit branchBlockTransferInstructions[] = {
// STM traps if ^ postfix, otherwise pass through
{1, armStmInstruction, armStmPCInstruction,         0x08400000, 0x0e500000, "STM {regList}^"},
{0, armStmInstruction, armStmPCInstruction,         0x08800000, 0x0ff00000, "STMIA {regList}"},
{0, armStmInstruction, armStmPCInstruction,         0x08000000, 0x0e100000, "STM {regList}"},
// POP LDM syntax, only care if PC in reglist
{1, armLdmInstruction, armPopLdmPCInstruction,         0x08bd8000, 0x0fff8000, "LDM SP, {...r15}"},
{0, armLdmInstruction, armPopLdmPCInstruction,        0x08bd0000, 0x0fff0000, "LDM SP, {reglist}"},
// LDM traps if ^ postfix and/or PC is in register list, otherwise pass through
{1, armLdmInstruction, armLdmPCInstruction,         0x08500000, 0x0e500000, "LDM Rn, {regList}^"},
{1, armLdmInstruction, armLdmPCInstruction,         0x08108000, 0x0e108000, "LDM Rn, {..r15}"},
{0, armLdmInstruction, armLdmPCInstruction,         0x08900000, 0x0f900000, "LDMIA Rn, {regList}"},
{0, armLdmInstruction, armLdmPCInstruction,         0x08100000, 0x0e100000, "LDM Rn, {regList}"},
// B/BL: always hypercall! obviously.
{1, armBInstruction, armBPCInstruction,         0x0a000000, 0x0e000000, "BRANCH"},

{1, undefinedInstruction, undefinedPCInstruction,   0x00000000, 0x00000000, UNDEFINED_INSTRUCTION}
};

struct instruction32bit svcCoprocInstructions[] = {
// well obviously.
{1, svcInstruction, svcPCInstruction,         0x0f000000, 0x0f000000, "SWI code"},
// Generic coprocessor instructions.
{1, mrcInstruction, mrcPCInstruction,     0x0e100010, 0x0f100010, "MRC"},
{1, mcrInstruction, mcrPCInstruction,     0x0e000010, 0x0f100010, "MCR"},

{1, undefinedInstruction, undefinedPCInstruction,   0x00000000, 0x00000000, UNDEFINED_INSTRUCTION}
};



struct instruction32bit * decodeInstr(GCONTXT *context, u32int instr)
{
  u32int catCode = decodeTopLevelCategory(instr);
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
      dumpGuestContext(context);
      DIE_NOW(context, "decoder: UNDEFINED category");
  }

  return 0;
}

static u32int decodeTopLevelCategory(u32int instr)
{
  int index = 0;
  /* LOOP through all ARM instruction encoding categories */
  while (TRUE)
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
    while (TRUE)
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
    while (TRUE)
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
  DIE_NOW(NULL, "decoder: decodeDataProcMisc unimplemented");
}


struct instruction32bit * decodeLoadStoreWordByte(u32int instr)
{
#ifdef DECODER_DEBUG
  dumpInstruction("decodeLoadStoreWordByte", instr);
#endif
  u32int index = 0;
  while (TRUE)
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
  DIE_NOW(NULL, "decoder: decodeBranchBlockTransfer unimplemented");
}


struct instruction32bit * decodeMedia(u32int instr)
{
#ifdef DECODER_DEBUG
  dumpInstruction("decodeMedia", instr);
#endif
  u32int index = 0;
  while (TRUE)
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
  DIE_NOW(NULL, "decoder: decodeMedia unimplemented");
}


struct instruction32bit * decodeBranchBlockTransfer(u32int instr)
{
#ifdef DECODER_DEBUG
  dumpInstruction("decodeBranchBlockTransfer", instr);
#endif
  u32int index = 0;
  while (TRUE)
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
  DIE_NOW(NULL, "decoder: decodeBranchBlockTransfer unimplemented");
}


struct instruction32bit * decodeSvcCoproc(u32int instr)
{
#ifdef DECODER_DEBUG
  dumpInstruction("decodeSvcCoproc", instr);
#endif
  u32int index = 0;
  while (TRUE)
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
  DIE_NOW(NULL, "decoder: decodeSvcCoproc unimplemented");
}

struct instruction32bit * decodeUnconditional(u32int instr)
{
#ifdef DECODER_DEBUG
  dumpInstruction("decodeUnconditional", instr);
#endif
  u32int index = 0;
  while (TRUE)
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

  DIE_NOW(NULL, "decoder: decodeUnconditional unimplemented");
}

void dumpInstruction(const char * msg, u32int instr)
{
  printf("%s: Instruction: %#.8x \n", msg, instr);
}
