/*
 * ARM decoding tables for the table search decoder
 */

#define ENTRY(_code, _handler, _pcHandler, _value, _mask, _instructionString)     \
  {                                                                               \
    .code = _code,                                                                \
    .handler = _handler,                                                          \
    .pcHandler = _pcHandler,                                                      \
    .value = _value,                                                              \
    .mask = _mask,                                                                \
    .instructionString = _instructionString                                       \
  }


static struct decodingTableEntry armBranchBlockTransferInstructions[] =
{
  // STM traps if ^ postfix, otherwise pass through
  ENTRY(IRC_REPLACE, armStmUserInstruction,     NULL,                   0x08400000, 0x0e500000, "STM.. {regList}^"),
  ENTRY(IRC_PATCH_PC,armStmInstruction,         armStmPC,               0x08000000, 0x0e500000, "STM.. {regList}"),
  // LDM exception return: trap
  ENTRY(IRC_REPLACE, armLdmExcRetInstruction,   NULL,                   0x08508000, 0x0e508000, "LDM Rn, {..., PC}^"),
  // LDM user mode: trap
  ENTRY(IRC_REPLACE, armLdmUserInstruction,     NULL,                   0x08500000, 0x0e508000, "LDM Rn, {...}^"),
  // LDM: if PC in register list, hypercall
  ENTRY(IRC_REPLACE, armLdmInstruction,         NULL,                   0x08108000, 0x0e508000, "LDM Rn, {..r15}"),
  // B: always hypercall! obviously.
  ENTRY(IRC_REPLACE, armBInstruction,           NULL,                   0x0a000000, 0x0f000000, "B imm24"),
  // BL: always hypercall! obviously.
  ENTRY(IRC_REPLACE, armBlInstruction,          NULL,                   0x0b000000, 0x0f000000, "BL imm24"),
  ENTRY(IRC_SAFE,    undefinedInstruction,      NULL,                   0x00000000, 0x00000000, "branchBlockTransferInstructions")
};

// register/shifter register cases, etc
static struct decodingTableEntry armDataProcMiscInstructions_op0[] =
{
  // UNIMPLEMENTED: SWP swap
  ENTRY(IRC_REPLACE, armSwpInstruction,         NULL,                   0x01000090, 0x0fb00ff0, "SWP"),
  // store halfword and translate
  ENTRY(IRC_REPLACE, armStrhtImmInstruction,    NULL,                   0x006000b0, 0x0f7000f0, "STRHT Rt, [Rn, #imm]"),
  ENTRY(IRC_REPLACE, armStrhtRegInstruction,    NULL,                   0x004000b0, 0x0f700ff0, "STRHT Rt, [Rn, Rm]"),
  // load halfword and translate
  ENTRY(IRC_REPLACE, armLdrhtImmInstruction,    NULL,                   0x007000b0, 0x0f7000f0, "LDRHT Rt, [Rn, #imm]"),
  ENTRY(IRC_REPLACE, armLdrhtRegInstruction,    NULL,                   0x003000b0, 0x0f700ff0, "LDRHT Rt, [Rn, Rm]"),
  // Branch and try to exchange to ARM mode.
  ENTRY(IRC_REPLACE, armBxInstruction,          NULL,                   0x012FFF10, 0x0ffffff0, "BX"),
  // Branch and link and try to exchange to Jazelle mode.
  ENTRY(IRC_REPLACE, armBxjInstruction,         NULL,                   0x012fff20, 0x0ffffff0, "BXJ Rm"),
  // Software breakpoint instruction... not sure yet.
  ENTRY(IRC_REPLACE, armBkptInstruction,        NULL,                   0xe1200070, 0xfff000f0, "BKPT #imm8"),
  // UNIMPLEMENTED: SMC, previously SMI: secure monitor call
  ENTRY(IRC_REPLACE, armSmcInstruction,         NULL,                   0x01600070, 0x0ffffff0, "SMC"),
  // Branch and link and try to exchange with Thumb mode.
  ENTRY(IRC_REPLACE, armBlxRegisterInstruction, NULL,                   0x012fff30, 0x0ffffff0, "BLX Rm"),
  // LDRD: imm/reg case Rn = PC case patch
  ENTRY(IRC_PATCH_PC,armLdrdImmInstruction,     armLdrdhPCInstruction,  0x014f00d0, 0x0f7f00f0, "LDRD Rt, [Rn, #imm]"),
  ENTRY(IRC_PATCH_PC,armLdrdRegInstruction,     armLdrdhPCInstruction,  0x000f00d0, 0x0e5f0ff0, "LDRD Rt, [Rn, Rm]"),
  // STRD: imm/reg case Rn = PC case patch
  ENTRY(IRC_PATCH_PC,armStrdImmInstruction,     armStrPCInstruction,    0x004f00f0, 0x0e5f00f0, "STRD Rt, [Rn, #imm]"),
  ENTRY(IRC_PATCH_PC,armStrdRegInstruction,     armStrPCInstruction,    0x000f00f0, 0x0e5f0ff0, "STRD Rt, [Rn, Rm]"),
  // STRH: imm/reg case Rn = PC case patch
  ENTRY(IRC_PATCH_PC,armStrhImmInstruction,     armStrPCInstruction,    0x004f00b0, 0x0e5f00f0, "STRH Rt, [Rn, #imm8]"),
  ENTRY(IRC_PATCH_PC,armStrhRegInstruction,     armStrPCInstruction,    0x000f00b0, 0x0e5f0ff0, "STRH Rt, [Rn], Rm"),
  // LDRH: imm/reg case Rn = PC case patch
  ENTRY(IRC_PATCH_PC,armLdrhImmInstruction,     armLdrdhPCInstruction,  0x005f00b0, 0x0e5f00f0, "LDRH Rt, [PC, #imm8]"),
  ENTRY(IRC_PATCH_PC,armLdrhRegInstruction,     armLdrdhPCInstruction,  0x001f00b0, 0x0e5f0ff0, "LDRH Rt, [Rn], Rm"),
  // AND: reg case Rd = PC replace, Rn = PC patch
  ENTRY(IRC_REPLACE, armAndInstruction,         NULL,                   0x0000f000, 0x0fe0f010, "AND PC, Rn, Rm, #shamt"),
  ENTRY(IRC_PATCH_PC,armAndInstruction,         armALUImmRegRSR,        0x000f0000, 0x0fef0010, "AND Rd, Rn, Rm, #shamt"),
  // EOR: reg case Rd = PC replace, Rn = PC patch
  ENTRY(IRC_REPLACE, armEorInstruction,         NULL,                   0x0020f000, 0x0fe0f010, "EOR PC, Rn, Rm, #shamt"),
  ENTRY(IRC_PATCH_PC,armEorInstruction,         armALUImmRegRSR,        0x002f0000, 0x0fef0010, "EOR Rd, Rn, Rm, #shamt"),
  // SUB: reg case Rd = PC replace, Rn = PC patch
  ENTRY(IRC_REPLACE, armSubInstruction,         NULL,                   0x0040f000, 0x0fe0f010, "SUB PC, Rn, Rm, #shamt"),
  ENTRY(IRC_PATCH_PC,armSubInstruction,         armALUImmRegRSR,        0x004f0000, 0x0fef0010, "SUB Rd, Rn, Rm, #shamt"),
  // RSB: reg case Rd = PC replace, Rn = PC patch
  ENTRY(IRC_REPLACE, armRsbInstruction,         NULL,                   0x0060f000, 0x0fe0f010, "RSB PC, Rn, Rm, #shamt"),
  ENTRY(IRC_PATCH_PC,armRsbInstruction,         armALUImmRegRSR,        0x006f0000, 0x0fef0010, "RSB Rd, Rn, Rm, #shamt"),
  // ADD: reg case Rd = PC replace, Rn = PC patch
  ENTRY(IRC_REPLACE, armAddInstruction,         NULL,                   0x0080f000, 0x0fe0f010, "ADD PC, Rn, Rm, #shamt"),
  ENTRY(IRC_PATCH_PC,armAddInstruction,         armALUImmRegRSR,        0x008f0000, 0x0fef0010, "ADD Rd, Rn, Rm, #shamt"),
  // ADC: reg case Rd = PC replace, Rn = PC patch
  ENTRY(IRC_REPLACE, armAdcInstruction,         NULL,                   0x00a0f000, 0x0fe0f010, "ADC PC, Rn, Rm, #shamt"),
  ENTRY(IRC_PATCH_PC,armAdcInstruction,         armALUImmRegRSR,        0x00af0000, 0x0fef0010, "ADC Rd, Rn, Rm, #shamt"),
  // SBC: reg case Rd = PC replace, Rn = PC patch
  ENTRY(IRC_REPLACE, armSbcInstruction,         NULL,                   0x00c0f000, 0x0fe0f010, "SBC Rd, Rn, Rm, #shamt"),
  ENTRY(IRC_PATCH_PC,armSbcInstruction,         armALUImmRegRSR,        0x00cf0000, 0x0fef0010, "SBC Rd, Rn, Rm, #shamt"),
  // RSC: reg case Rd = PC replace, Rn = PC patch
  ENTRY(IRC_REPLACE, armRscInstruction,         NULL,                   0x00e0f000, 0x0fe0f010, "RSC PC, Rn, Rm, #shamt"),
  ENTRY(IRC_PATCH_PC,armRscInstruction,         armALUImmRegRSR,        0x00ef0000, 0x0fef0010, "RSC Rd, Rn, Rm, #shamt"),
  // MSR/MRS: always hypercall! we must hide the real state from guest.
  ENTRY(IRC_REPLACE, armMsrRegInstruction,      NULL,                   0x0120f000, 0x0fb0fff0, "MSR, s/cpsr, Rn"),
  ENTRY(IRC_REPLACE, armMrsInstruction,         NULL,                   0x010f0000, 0x0fbf0fff, "MRS, Rn, s/cpsr"),
  // TST: Rn/Rm can be PC; patch
  ENTRY(IRC_PATCH_PC,armTstInstruction,         armALUregNoDest,        0x011f0000, 0x0fff0010, "TST PC, Rm, #shamt"),
  ENTRY(IRC_PATCH_PC,armTstInstruction,         armALUregNoDest,        0x0110000f, 0x0ff0001f, "TST Rn, PC, #shamt"),
  // TEQ: Rn/Rm can be PC; patch
  ENTRY(IRC_PATCH_PC,armTeqInstruction,         armALUregNoDest,        0x013f0000, 0x0fff0010, "TEQ PC, Rm, #shamt"),
  ENTRY(IRC_PATCH_PC,armTeqInstruction,         armALUregNoDest,        0x0130000f, 0x0ff0001f, "TEQ Rn, PC, #shamt"),
  // CMP: Rn/Rm can be PC; patch
  ENTRY(IRC_PATCH_PC,armCmpInstruction,         armALUregNoDest,        0x015f0000, 0x0fff0010, "CMP PC, Rm, #shamt"),
  ENTRY(IRC_PATCH_PC,armCmpInstruction,         armALUregNoDest,        0x0150000f, 0x0ff0001f, "CMP Rn, PC, #shamt"),
  // CMN: Rn/Rm can be PC; patch
  ENTRY(IRC_PATCH_PC,armCmnInstruction,         armALUregNoDest,        0x016f0000, 0x0fff0010, "CMN PC, Rm, #shamt"),
  ENTRY(IRC_PATCH_PC,armCmnInstruction,         armALUregNoDest,        0x0160000f, 0x0ff0001f, "CMN Rn, PC, #shamt"),
  // ORR: Rd = PC end block, Rn/Rm can be PC - patch
  ENTRY(IRC_REPLACE, armOrrInstruction,         NULL,                   0x0180f000, 0x0fe0f010, "ORR PC, Rn, Rm, #shamt"),
  ENTRY(IRC_PATCH_PC,armOrrInstruction,         armALUImmRegRSR,        0x018f0000, 0x0fef0010, "ORR Rd, PC, Rm, #shamt"),
  ENTRY(IRC_PATCH_PC,armOrrInstruction,         armALUImmRegRSR,        0x0180000f, 0x0fe0001f, "ORR Rd, Rn, PC, #shamt"),
  // MOV with Rd = PC replace; Rm = PC patch
  ENTRY(IRC_REPLACE, armMovInstruction,         NULL,                   0x01a0f000, 0x0feffff0, "MOV PC, Rm"),
  ENTRY(IRC_PATCH_PC,armMovInstruction,         armMovPCInstruction,    0x01a0000f, 0x0fef0fff, "MOV Rn, PC"),
  // LSL: Rd = PC replace; Rm = PC patch
  ENTRY(IRC_REPLACE, armLslInstruction,         NULL,                   0x01a0f000, 0x0feff070, "LSL PC, Rm, #shamt"),
  ENTRY(IRC_PATCH_PC,armLslInstruction,         armShiftPCImm,          0x01a0000f, 0x0fef007f, "LSL Rd, PC, #shamt"),
  // LSR: Rd = PC replace; Rm = PC patch
  ENTRY(IRC_REPLACE, armLsrInstruction,         NULL,                   0x01a0f020, 0x0feff070, "LSR PC, Rm, #shamt"),
  ENTRY(IRC_PATCH_PC,armLsrInstruction,         armShiftPCImm,          0x01a0002f, 0x0fef007f, "LSR Rd, PC, #shamt"),
  // ASR: Rd = PC replace; Rm = PC patch
  ENTRY(IRC_REPLACE, armAsrInstruction,         NULL,                   0x01a0f040, 0x0feff070, "ASR PC, Rm, #shamt"),
  ENTRY(IRC_PATCH_PC,armAsrInstruction,         armShiftPCImm,          0x01a0004f, 0x0fef007f, "ASR Rd, PC, #shamt"),
  // RRX: Rd = PC replace; Rm = PC patch
  ENTRY(IRC_REPLACE, armRrxInstruction,         NULL,                   0x01a0f060, 0x0feffff0, "RRX PC, Rm"),
  ENTRY(IRC_PATCH_PC,armRrxInstruction,         armShiftPCImm,          0x01a0006f, 0x0fef0fff, "RRX Rd, PC"),
  // ROR: reg case destination unpredictable. imm case dest can be PC.
  ENTRY(IRC_REPLACE, armRorInstruction,         NULL,                   0x01a0f060, 0x0feff070, "ROR PC, Rm, #imm"),
  ENTRY(IRC_PATCH_PC,armRorInstruction,         armShiftPCImm,          0x01a0006f, 0x0fef007f, "ROR Rd, PC, #imm"),
  // BIC with Rd = PC end block, other are fine.
  ENTRY(IRC_REPLACE, armBicInstruction,         NULL,                   0x01c0f000, 0x0fe0f010, "BIC PC, Rn, Rm, #shamt"),
  ENTRY(IRC_PATCH_PC,armBicInstruction,         armALUImmRegRSR,        0x01cf0000, 0x0fef0010, "BIC Rd, PC, Rm, #shamt"),
  ENTRY(IRC_PATCH_PC,armBicInstruction,         armALUImmRegRSR,        0x01c0000f, 0x0fe0001f, "BIC Rd, Rn, PC, #shamt"),
  // MVN with Rd = PC end block, other are fine.
  ENTRY(IRC_REPLACE, armMvnInstruction,         NULL,                   0x01e0f000, 0x0fe0f010, "MVN PC, Rm, #shamt"),
  ENTRY(IRC_PATCH_PC,armMvnInstruction,         armShiftPCImm,          0x01e0000f, 0x0fe0001f, "MVN Rd, PC, #shamt"),

  ENTRY(IRC_SAFE,    undefinedInstruction,      NULL,                   0x00000000, 0x00000000, "dataProcMiscInstructions_op0")
};

// immediate cases and random hints
static struct decodingTableEntry armDataProcMiscInstructions_op1[] =
{
  // WFE: indicates this guest should sleep for a while
  ENTRY(IRC_REPLACE, armWfeInstruction,         NULL,                   0x0320f002, 0x0fffffff, "wfe%c"),
  // WFI: indicates this guest should sleep for a while
  ENTRY(IRC_REPLACE, armWfiInstruction,         NULL,                   0x0320f003, 0x0fffffff, "wfi%c"),
  // SEV: wake somebody up
  ENTRY(IRC_REPLACE, armSevInstruction,         NULL,                   0x0320f004, 0x0fffffff, "sev%c"),
  // UNIMPLEMENTED: debug hint
  ENTRY(IRC_REPLACE, armDbgInstruction,         NULL,                   0x0320f0f0, 0x0ffffff0, "dbg%c\t#%0-3d"),
  // AND: Rd = PC end block, others are fine
  ENTRY(IRC_REPLACE, armAndInstruction,         NULL,                   0x0200f000, 0x0fe0f000, "AND PC, Rn, #imm"),
  ENTRY(IRC_PATCH_PC,armAndInstruction,         armALUImmRegRSR,        0x020f0000, 0x0fef0000, "AND Rd, PC, #imm"),
  // EOR: Rd = PC end block, others are fine
  ENTRY(IRC_REPLACE, armEorInstruction,         NULL,                   0x0220f000, 0x0fe0f000, "EOR PC, Rn, Rm/#imm"),
  ENTRY(IRC_PATCH_PC,armEorInstruction,         armALUImmRegRSR,        0x02200000, 0x0fe00000, "EOR Rd, PC, #imm"),
  // SUB: Rd = PC end block, others are fine
  ENTRY(IRC_REPLACE, armSubInstruction,         NULL,                   0x0240f000, 0x0fe0f000, "SUB PC, Rn, Rm/imm"),
  ENTRY(IRC_PATCH_PC,armSubInstruction,         armALUImmRegRSR,        0x024f0000, 0x0fef0000, "SUB Rd, PC, #imm"),
  // RSB: Rd = PC end block, others are fine
  ENTRY(IRC_REPLACE, armRsbInstruction,         NULL,                   0x0260f000, 0x0fe0f000, "RSB PC, Rn, Rm/imm"),
  ENTRY(IRC_PATCH_PC,armRsbInstruction,         armALUImmRegRSR,        0x026f0000, 0x0fef0000, "RSB Rd, PC, #imm"),
  // ADD: Rd = PC end block, others are fine
  ENTRY(IRC_REPLACE, armAddInstruction,         NULL,                   0x0280f000, 0x0fe0f000, "ADD PC, Rn, #imm"),
  ENTRY(IRC_PATCH_PC,armAddInstruction,         armALUImmRegRSR,        0x028f0000, 0x0fef0000, "ADD Rd, PC, #imm"),
  // ADC: Rd = PC end block, others are fine
  ENTRY(IRC_REPLACE, armAdcInstruction,         NULL,                   0x02a0f000, 0x0fe0f000, "ADC PC, Rn/#imm"),
  ENTRY(IRC_PATCH_PC,armAdcInstruction,         armALUImmRegRSR,        0x02af0000, 0x0fef0000, "ADC Rd, PC, #imm"),
  // SBC: Rd = PC end block, others are fine
  ENTRY(IRC_REPLACE, armSbcInstruction,         NULL,                   0x02c0f000, 0x0fe0f000, "SBC PC, Rn/#imm"),
  ENTRY(IRC_PATCH_PC,armSbcInstruction,         armALUImmRegRSR,        0x02cf0000, 0x0fef0000, "SBC Rd, PC, #imm"),
  // RSC: Rd = PC end block, others are fine
  ENTRY(IRC_REPLACE, armRscInstruction,         NULL,                   0x02e0f000, 0x0fe0f000, "RSC PC, Rn/#imm"),
  ENTRY(IRC_PATCH_PC,armRscInstruction,         armALUImmRegRSR,        0x02ef0000, 0x0fef0000, "RSC Rd, PC, #imm"),
  // MSR: always hypercall! we must hide the real state from guest.
  ENTRY(IRC_REPLACE, armMsrImmInstruction,      NULL,                   0x0320f000, 0x0fb0f000, "MSR, s/cpsr, #imm"),
  // TST Rn = PC patch
  ENTRY(IRC_PATCH_PC,armTstInstruction,         armALUimmNoDest,        0x031f0000, 0x0fff0000, "TST PC, #imm"),
  // TEQ Rn = PC patch
  ENTRY(IRC_PATCH_PC,armTeqInstruction,         armALUimmNoDest,        0x033f0000, 0x0fff0000, "TEQ PC, #imm"),
  // CMP Rn = PC patch
  ENTRY(IRC_PATCH_PC,armCmpInstruction,         armALUimmNoDest,        0x035f0000, 0x0fff0000, "CMP PC, #imm"),
  // CMN Rn = PC patch
  ENTRY(IRC_PATCH_PC,armCmnInstruction,         armALUimmNoDest,        0x037f0000, 0x0fff0000, "CMN PC, #imm"),
  // ORR: Rd = PC end block, Rn = PC patch
  ENTRY(IRC_REPLACE, armOrrInstruction,         NULL,                   0x0380f000, 0x0fe0f000, "ORR PC, Rn, #imm"),
  ENTRY(IRC_PATCH_PC,armOrrInstruction,         armALUImmRegRSR,        0x038f0000, 0x0fef0000, "ORR Rd, PC, #imm"),
  // MOV with Rd = PC end block. MOV <shifted reg> is a pseudo instr..
  ENTRY(IRC_REPLACE, armMovInstruction,         NULL,                   0x03a0f000, 0x0feff000, "MOV PC, #imm"),
  // BIC with Rd = PC end block, other are fine.
  ENTRY(IRC_REPLACE, armBicInstruction,         NULL,                   0x03c0f000, 0x0fe0f000, "BIC PC, Rn, #imm"),
  ENTRY(IRC_PATCH_PC,armBicInstruction,         armALUImmRegRSR,        0x03cf0000, 0x0fef0000, "BIC Rd, PC, #imm"),
  // MVN with Rd = PC end block, other are fine.
  ENTRY(IRC_REPLACE, armMvnInstruction,         NULL,                   0x03e0f000, 0x0fe0f000, "MVN PC, #imm"),

  ENTRY(IRC_SAFE,    undefinedInstruction,      NULL,                   0x00000000, 0x00000000, "armDataProcMiscInstructions_op1")
};

static struct decodingTableEntry armLoadStoreWordByteInstructions[] =
{
  // STRT - all trap
  ENTRY(IRC_REPLACE, armStrtImmInstruction,      armStrtPCInstruction,  0x04200000, 0x0f700000, "STRT Rt, [Rn], +-imm12"),
  ENTRY(IRC_REPLACE, armStrtRegInstruction,      armStrtPCInstruction,  0x06200000, 0x0f700010, "STRT Rt, [Rn], +-Rm"),
  // LDRT - all trap
  ENTRY(IRC_REPLACE, armLdrtImmInstruction,      NULL,                  0x04300000, 0x0f700000, "LDRT Rd, [Rn], +-imm12"),
  ENTRY(IRC_REPLACE, armLdrtRegInstruction,      NULL,                  0x06300000, 0x0f700010, "LDRT Rd, [Rn], +-Rm"),
  // STRBT - all trap
  ENTRY(IRC_REPLACE, armStrbtImmInstruction,     NULL,                  0x04600000, 0x0f700000, "STRBT Rt, [Rn, +-imm12]"),
  ENTRY(IRC_REPLACE, armStrbtRegInstruction,     NULL,                  0x06600000, 0x0f700010, "STRBT Rt, [Rn], +-Rm"),
  // LDRBT - all trap
  ENTRY(IRC_REPLACE, armLdrbtImmInstruction,     NULL,                  0x04700000, 0x0f700000, "LDRBT Rd, [Rn], +-imm12"),
  ENTRY(IRC_REPLACE, armLdrbtRegInstruction,     NULL,                  0x06700000, 0x0f700010, "LDRBT Rd, [Rn], +-Rm"),
  // STR imm: Rn/Rt can be PC, patch
  ENTRY(IRC_PATCH_PC,armStrImmInstruction,       armStrPCInstruction,   0x0400f000, 0x0e50f000, "STR PC, [Rn, +-imm12]"),
  ENTRY(IRC_PATCH_PC,armStrImmInstruction,       armStrPCInstruction,   0x040f0000, 0x0e5f0000, "STR Rt, [PC, +-imm12]"),
  // STR reg: Rn/Rt can be PC, patch, Rm != PC
  ENTRY(IRC_PATCH_PC,armStrRegInstruction,       armStrPCInstruction,   0x0600f000, 0x0e50f010, "STR PC, [Rn], +-Rm"),
  ENTRY(IRC_PATCH_PC,armStrRegInstruction,       armStrPCInstruction,   0x060f0000, 0x0e5f0010, "STR Rt, [PC], +-Rm"),
  // LDR imm: Rt = PC trap, if Rn = PC - literal case
  ENTRY(IRC_REPLACE, armLdrImmInstruction,       NULL,                  0x0410f000, 0x0e50f000, "LDR PC, [Rn], +-imm12"),
  ENTRY(IRC_PATCH_PC,armLdrImmInstruction,       armLdrPCInstruction,   0x051f0000, 0x0f7f0000, "LDR Rt, [PC], +-imm12"),
  // LDR reg: Rt = PC trap; Rn can be PC - patch; Rm != pc
  ENTRY(IRC_REPLACE, armLdrRegInstruction,       NULL,                  0x0610f000, 0x0e50f010, "LDR PC, [Rn], +-Rm"),
  ENTRY(IRC_PATCH_PC,armLdrRegInstruction,       armLdrPCInstruction,   0x061f0000, 0x0e5f0010, "LDR Rd, [PC], +-Rm"),
  // STRB imm: Rt != PC; Rn = PC patch
  ENTRY(IRC_PATCH_PC,armStrbImmInstruction,      armStrPCInstruction,   0x044f0000, 0x0e5f0000, "STRB Rt, [PC, +-imm12]"),
  // STRB reg: Rt/Rm != PC; Rn = PC patch
  ENTRY(IRC_PATCH_PC,armStrbRegInstruction,      armStrPCInstruction,   0x064f0000, 0x0e5f0010, "STRB Rt, [PC], +-Rm"),
  // LDRB imm: Rt != PC; Rn = PC patch (ldrb literal)
  ENTRY(IRC_PATCH_PC,armLdrbImmInstruction,      armLdrPCInstruction,   0x055f0000, 0x0f7f0000, "LDRB Rt, [PC], +-imm12"),
  // LDRB reg: Rt/Rm != PC; Rn = PC patch
  ENTRY(IRC_PATCH_PC,armLdrbRegInstruction,      armLdrPCInstruction,   0x065f0000, 0x0e5f0010, "LDRB Rt, [PC], +-Rm"),

  ENTRY(IRC_SAFE,    undefinedInstruction,       NULL,                  0x00000000, 0x00000000, "armLoadStoreWordByteInstructions")
};

static struct decodingTableEntry armMediaInstructions[] =
{
  // joyfully all media instructions cannot use PC as any operand.
  ENTRY(IRC_SAFE,    undefinedInstruction,       NULL,                  0x00000000, 0x00000000, "armMediaInstructions")
};

static struct decodingTableEntry armSvcCoprocInstructions[] =
{
  // well obviously.
  ENTRY(IRC_REPLACE, svcInstruction,             NULL,                  0x0f000000, 0x0f000000, "SWI code"),
  // Generic coprocessor instructions.
  ENTRY(IRC_REPLACE, armMrcInstruction,          NULL,                  0x0e100010, 0x0f100010, "MRC"),
  ENTRY(IRC_REPLACE, armMcrInstruction,          NULL,                  0x0e000010, 0x0f100010, "MCR"),

  ENTRY(IRC_SAFE,    undefinedInstruction,       NULL,                  0x00000000, 0x00000000, "armSvcCoprocInstructions")
};

static struct decodingTableEntry armUnconditionalInstructions[] =
{
  // UNIMPLEMENTED: data memory barrier
  ENTRY(IRC_REMOVE,  armDmbInstruction,          NULL,                  0xf57ff050, 0xfffffff0, "dmb\t%U"),
  // sync barriers
  ENTRY(IRC_REMOVE,  armDsbInstruction,          NULL,                  0xf57ff040, 0xfffffff0, "DSB"),
  ENTRY(IRC_REMOVE,  armIsbInstruction,          NULL,                  0xf57ff060, 0xfffffff0, "ISB"),
  // CPS: change processor state
  ENTRY(IRC_REPLACE, armCpsInstruction,          NULL,                  0xf1000000, 0xfff1fe20, "CPS"),
  // UNIMPLEMENTED: RFE return from exception
  ENTRY(IRC_REPLACE, armRfeInstruction,          NULL,                  0xf8100a00, 0xfe50ffff, "RFE"),
  // UNIMPLEMENTED: SETEND set endianess
  ENTRY(IRC_REPLACE, armSetendInstruction,       NULL,                  0xf1010000, 0xfffffc00, "setend"),
  // UNIMPLEMENTED: SRS store return state
  ENTRY(IRC_REPLACE, armSrsInstruction,          NULL,                  0xf84d0500, 0xfe5fffe0, "srs"),
  // BLX: branch and link to Thumb subroutine
  ENTRY(IRC_REPLACE, armBlxImmediateInstruction, NULL,                  0xfa000000, 0xfe000000, "BLX #imm24"),
  // PLD: preload data
  ENTRY(IRC_REMOVE, armPldInstruction,           NULL,                  0xf450f000, 0xfc70f000, "PLD"),
  // PLI: preload instruction
  ENTRY(IRC_REMOVE, armPliInstruction,           NULL,                  0xf450f000, 0xfd70f000, "PLI"),
  ENTRY(IRC_SAFE,   undefinedInstruction,        NULL,                  0x00000000, 0x00000000, "armUnconditionalInstructions")
};

#undef ENTRY


/*
 * Top level instruction categories.
 *
 * Identify unconditional instructions first, and terminate the table with a category that catches
 * all instructions, to detect unimplemented and (truly) undefined instructions.
 */
static struct decodingTable armCategories[] =
{
  { 0xF0000000, 0xF0000000, armUnconditionalInstructions },
  { 0x0E000000, 0x00000000, armDataProcMiscInstructions_op0 },
  { 0x0E000000, 0x02000000, armDataProcMiscInstructions_op1 },
  { 0x0E000000, 0x04000000, armLoadStoreWordByteInstructions },
  { 0x0E000010, 0x06000000, armLoadStoreWordByteInstructions },
  { 0x0E000010, 0x06000010, armMediaInstructions },
  { 0x0C000000, 0x08000000, armBranchBlockTransferInstructions },
  { 0x0C000000, 0x0C000000, armSvcCoprocInstructions },
  { 0x00000000, 0x00000000, NULL }
};
