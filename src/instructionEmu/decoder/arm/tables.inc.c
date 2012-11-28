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
  ENTRY(IRC_REPLACE, armStmInstruction,         NULL,                   0x08400000, 0x0e500000, "STM.. {regList}^"),
  ENTRY(IRC_SAFE,    armStmInstruction,         armStmPCInstruction,    0x08000000, 0x0e500000, "STM.. {regList}"),
  // POP LDM syntax, only care if PC in reglist
  ENTRY(IRC_REPLACE, armLdmInstruction,         NULL,                   0x08bd8000, 0x0fff8000, "LDM SP, {...r15}"),
  ENTRY(IRC_SAFE,    armLdmInstruction,         NULL,                   0x08bd0000, 0x0fff0000, "LDM SP, {reglist}"),
  // LDM traps if ^ postfix and/or PC is in register list, otherwise pass through
  ENTRY(IRC_REPLACE, armLdmInstruction,         NULL,                   0x08500000, 0x0e500000, "LDM Rn, {regList}^"),
  ENTRY(IRC_REPLACE, armLdmInstruction,         NULL,                   0x08108000, 0x0e108000, "LDM Rn, {..r15}"),
  ENTRY(IRC_SAFE,    armLdmInstruction,         NULL,                   0x08900000, 0x0f900000, "LDMIA Rn, {regList}"),
  ENTRY(IRC_SAFE,    armLdmInstruction,         NULL,                   0x08100000, 0x0e100000, "LDM Rn, {regList}"),
  // B/BL: always hypercall! obviously.
  ENTRY(IRC_REPLACE, armBInstruction,           NULL,                   0x0a000000, 0x0e000000, "BRANCH"),
  ENTRY(IRC_REPLACE, undefinedInstruction,      NULL,                   0x00000000, 0x00000000, "branchBlockTransferInstructions")
};

// register/shifter register cases, etc
static struct decodingTableEntry armDataProcMiscInstructions_op0[] =
{
  // NOP is just a nop...
  ENTRY(IRC_SAFE,    nopInstruction,            NULL,                   0xe1a00000, 0xffffffff, "NOP"),
  // UNIMPLEMENTED: SWP swap
  ENTRY(IRC_REPLACE, armSwpInstruction,         NULL,                   0x01000090, 0x0fb00ff0, "SWP"),
  // store halfword and translate
  ENTRY(IRC_REPLACE, armStrhtInstruction,       NULL,                   0x006000b0, 0x0f7000f0, "STRHT instruction"),
  // load halfword and translate
  ENTRY(IRC_REPLACE, armLdrhtInstruction,       NULL,                   0x003000b0, 0x0f3000f0, "LDRHT instruction"),
  // store and load exclusive: must be emulated - user mode faults
  ENTRY(IRC_REPLACE, armLdrexbInstruction,      NULL,                   0x01d00f9f, 0x0ff00fff, "LDREXB"),
  ENTRY(IRC_REPLACE, armLdrexdInstruction,      NULL,                   0x01b00f9f, 0x0ff00fff, "LDREXD"),
  ENTRY(IRC_REPLACE, armLdrexhInstruction,      NULL,                   0x01f00f9f, 0x0ff00fff, "LDREXH"),
  ENTRY(IRC_REPLACE, armStrexbInstruction,      NULL,                   0x01c00f90, 0x0ff00ff0, "STREXB"),
  ENTRY(IRC_REPLACE, armStrexdInstruction,      NULL,                   0x01a00f90, 0x0ff00ff0, "STREXD"),
  ENTRY(IRC_REPLACE, armStrexhInstruction,      NULL,                   0x01e00f90, 0x0ff00ff0, "STREXH"),
  ENTRY(IRC_REPLACE, armLdrexInstruction,       NULL,                   0x01900f9f, 0x0ff00fff, "LDREX"),
  ENTRY(IRC_REPLACE, armStrexInstruction,       NULL,                   0x01800f90, 0x0ff00ff0, "STREX"),
  // SMULL - signed multiply, PC cannot be used as any destination
  ENTRY(IRC_SAFE,    armSmullInstruction,       NULL,                   0x00800090, 0x0fa000f0, "SMULL"),
  // SMLAL - signed multiply and accumulate, PC cannot be used as any destination
  ENTRY(IRC_SAFE,    armSmlalInstruction,       NULL,                   0x00a00090, 0x0fa000f0, "SMLAL"),
  // MUL: Rd = Rm * Rn; Rd != PC. pass through
  ENTRY(IRC_SAFE,    armMulInstruction,         NULL,                   0x00000090, 0x0fe000f0, "MUL Rd, Rm, Rn"),
  // MLA: Rd = Rm * Rn + Ra; Rd != PC. pass through
  ENTRY(IRC_SAFE,    armMlaInstruction,         NULL,                   0x00200090, 0x0fe000f0, "MLA Rd, Rm, Rn, Ra"),
  // MLS: Rd = Rm * Rn - Ra; Rd != PC, pass through
  ENTRY(IRC_SAFE,    armMlsInstruction,         NULL,                   0x00600090, 0x0ff000f0, "MLS Rd, Rm, Rn, Ra"),
  // Branch and try to exchange to ARM mode.
  ENTRY(IRC_REPLACE, armBxInstruction,          NULL,                   0x012FFF10, 0x0ffffff0, "bx%c\t%0-3r"),
  // Branch and link and try to exchange to Jazelle mode.
  ENTRY(IRC_REPLACE, armBxjInstruction,         NULL,                   0x012fff20, 0x0ffffff0, "BXJ Rm"),
  // Software breakpoint instruction... not sure yet.
  ENTRY(IRC_REPLACE, armBkptInstruction,        NULL,                   0xe1200070, 0xfff000f0, "BKPT #imm8"),
  // UNIMPLEMENTED: SMC, previously SMI: secure monitor call
  ENTRY(IRC_REPLACE, armSmcInstruction,         NULL,                   0x01600070, 0x0ff000f0, "smc%c\t%e"),
  // Branch and link and try to exchange with Thumb mode.
  ENTRY(IRC_REPLACE, armBlxRegisterInstruction, NULL,                   0x012fff30, 0x0ffffff0, "BLX Rm"),
  // CLZ: Count leading zeroes - Rd, Rm != PC, pass through
  ENTRY(IRC_SAFE,    armClzInstruction,         NULL,                   0x016f0f10, 0x0fff0ff0, "CLZ Rd, Rm"),
  // Q(D){ADD,SUB}: saturated add/subtract/doubleAdd/doubleSubtract, unpredictable when any R=PC
  ENTRY(IRC_SAFE,    armQaddInstruction,        NULL,                   0x01000050, 0x0ff00ff0, "qadd%c\t%12-15r, %0-3r, %16-19r"),
  ENTRY(IRC_SAFE,    armQdaddInstruction,       NULL,                   0x01400050, 0x0ff00ff0, "qdadd%c\t%12-15r, %0-3r, %16-19r"),
  ENTRY(IRC_SAFE,    armQsubInstruction,        NULL,                   0x01200050, 0x0ff00ff0, "qsub%c\t%12-15r, %0-3r, %16-19r"),
  ENTRY(IRC_SAFE,    armQdsubInstruction,       NULL,                   0x01600050, 0x0ff00ff0, "qdsub%c\t%12-15r, %0-3r, %16-19r"),
  // LDRD: Rt1 must be even numbered and NOT 14, thus Rt2 cannot be PC. pass. (Rn,#imm can be Rn=PC, this is the literal variant)
  ENTRY(IRC_SAFE,    armLdrdInstruction,        armLdrPCInstruction,    0x004000d0, 0x0e5000f0, "LDRD Rt, [Rn, #imm]"),
  ENTRY(IRC_SAFE,    armLdrdInstruction,        armLdrPCInstruction,    0x000000d0, 0x0e500ff0, "LDRD Rt, [Rn, Rm]"),
  // STRD: pass through, let them fail!
  ENTRY(IRC_SAFE,    armStrdInstruction,        armStrPCInstruction,    0x004000f0, 0x0e5000f0, "STRD Rt, [Rn, #imm]"),
  ENTRY(IRC_SAFE,    armStrdInstruction,        armStrPCInstruction,    0x000000f0, 0x0e500ff0, "STRD Rt, [Rn, Rm]"),
  // signed 16 bit multiply, 32 bit accumulate, unpredictable if any R=PC
  ENTRY(IRC_SAFE,    armSmlabbInstruction,      NULL,                   0x01000080, 0x0ff000f0, "smlabb%c\t%16-19r, %0-3r, %8-11r, %12-15r"),
  ENTRY(IRC_SAFE,    armSmlatbInstruction,      NULL,                   0x010000a0, 0x0ff000f0, "smlatb%c\t%16-19r, %0-3r, %8-11r, %12-15r"),
  ENTRY(IRC_SAFE,    armSmlabtInstruction,      NULL,                   0x010000c0, 0x0ff000f0, "smlabt%c\t%16-19r, %0-3r, %8-11r, %12-15r"),
  ENTRY(IRC_SAFE,    armSmlattInstruction,      NULL,                   0x010000e0, 0x0ff000f0, "smlatt%c\t%16-19r, %0-3r, %8-11r, %12-15r"),
  // signed 16 bit x 32 bit multiply, 32 bit accumulate, unpredictable if any R=PC
  ENTRY(IRC_SAFE,    armSmlawbInstruction,      NULL,                   0x01200080, 0x0ff000f0, "smlawb%c\t%16-19r, %0-3r, %8-11r, %12-15r"),
  ENTRY(IRC_SAFE,    armSmlawtInstruction,      NULL,                   0x012000c0, 0x0ff000f0, "smlawt%c\t%16-19r, %0-3r, %8-11r, %12-15r"),
  // signed 16 bit multiply, 64 bit accumulate, unpredictable if any R=PC
  ENTRY(IRC_SAFE,    armSmlalbbInstruction,     NULL,                   0x01400080, 0x0ff000f0, "smlalbb%c\t%12-15r, %16-19r, %0-3r, %8-11r"),
  ENTRY(IRC_SAFE,    armSmlaltbInstruction,     NULL,                   0x014000a0, 0x0ff000f0, "smlaltb%c\t%12-15r, %16-19r, %0-3r, %8-11r"),
  ENTRY(IRC_SAFE,    armSmlalbtInstruction,     NULL,                   0x014000c0, 0x0ff000f0, "smlalbt%c\t%12-15r, %16-19r, %0-3r, %8-11r"),
  ENTRY(IRC_SAFE,    armSmlalttInstruction,     NULL,                   0x014000e0, 0x0ff000f0, "smlaltt%c\t%12-15r, %16-19r, %0-3r, %8-11r"),
  // signed 16 bit multiply, 32 bit result, any R=PC is unpredictable
  ENTRY(IRC_SAFE,    armSmulbbInstruction,      NULL,                   0x01600080, 0x0ff0f0f0, "SMULBB Rd, Rn, RM"),
  ENTRY(IRC_SAFE,    armSmultbInstruction,      NULL,                   0x016000a0, 0x0ff0f0f0, "smultb%c\t%16-19r, %0-3r, %8-11r"),
  ENTRY(IRC_SAFE,    armSmulbtInstruction,      NULL,                   0x016000c0, 0x0ff0f0f0, "smulbt%c\t%16-19r, %0-3r, %8-11r"),
  ENTRY(IRC_SAFE,    armSmulttInstruction,      NULL,                   0x016000e0, 0x0ff0f0f0, "smultt%c\t%16-19r, %0-3r, %8-11r"),
  // sigENTRYned 16 bit x 32 bit multiply, 32 bit result, any R=PC is unpredictable
  ENTRY(IRC_SAFE,    armSmulwbInstruction,      NULL,                   0x012000a0, 0x0ff0f0f0, "smulwb%c\t%16-19r, %0-3r, %8-11r"),
  ENTRY(IRC_SAFE,    armSmulwtInstruction,      NULL,                   0x012000e0, 0x0ff0f0f0, "smulwt%c\t%16-19r, %0-3r, %8-11r"),
  // STRH: passthrough, will data abort if something wrong
  ENTRY(IRC_SAFE,    armStrhInstruction,        armStrPCInstruction,    0x004000b0, 0x0e5000f0, "STRH Rt, [Rn, +-imm8]"),
  ENTRY(IRC_SAFE,    armStrhInstruction,        armStrPCInstruction,    0x000000b0, 0x0e500ff0, "STRH Rt, [Rn], +-Rm"),
  // LDRH cant load halfword to PC, passthrough
  ENTRY(IRC_SAFE,    armLdrhInstruction,        armLdrPCInstruction,    0x00500090, 0x0e500090, "LDRH Rt, [Rn, +-imm8]"),
  ENTRY(IRC_SAFE,    armLdrhInstruction,        armLdrPCInstruction,    0x00100090, 0x0e500f90, "LDRH Rt, [Rn], +-Rm"),
  // AND: Rd = PC end block, others are fine
  ENTRY(IRC_REPLACE, armAndInstruction,         NULL,                   0x0000f000, 0x0fe0f010, "AND PC, Rn, Rm, #shamt"),
  ENTRY(IRC_REPLACE, armAndInstruction,         NULL,                   0x0000f010, 0x0fe0f090, "AND PC, Rn, Rm, Rshamt"),
  ENTRY(IRC_SAFE,    armAndInstruction,         armDPImmRegRSR,         0x00000000, 0x0fe00010, "AND Rd, Rn, Rm, #shamt"),
  ENTRY(IRC_SAFE,    armAndInstruction,         NULL,                   0x00000010, 0x0fe00090, "AND Rd, Rn, Rm, Rshamt"),
  // EOR: Rd = PC end block, others are fine
  ENTRY(IRC_REPLACE, armEorInstruction,         NULL,                   0x0020f000, 0x0fe0f010, "EOR PC, Rn, Rm, #shamt"),
  ENTRY(IRC_REPLACE, armEorInstruction,         NULL,                   0x0020f010, 0x0fe0f090, "EOR PC, Rn, Rm, Rshamt"),
  ENTRY(IRC_SAFE,    armEorInstruction,         armDPImmRegRSR,         0x00200000, 0x0fe00010, "EOR Rd, Rn, Rm, #shamt"),
  ENTRY(IRC_SAFE,    armEorInstruction,         NULL,                   0x00200010, 0x0fe00090, "EOR Rd, Rn, Rm, Rshamt"),
  // SUB: Rd = PC end block, others are fine
  ENTRY(IRC_REPLACE, armSubInstruction,         NULL,                   0x0040f000, 0x0fe0f010, "SUB PC, Rn, Rm, #shamt"),
  ENTRY(IRC_REPLACE, armSubInstruction,         NULL,                   0x0040f010, 0x0fe0f090, "SUB PC, Rn, Rm, Rshamt"),
  ENTRY(IRC_SAFE,    armSubInstruction,         armDPImmRegRSR,         0x00400000, 0x0fe00010, "SUB Rd, Rn, Rm, #shamt"),
  ENTRY(IRC_SAFE,    armSubInstruction,         NULL,                   0x00400010, 0x0fe00090, "SUB Rd, Rn, Rm, Rshamt"),
  // RSB: Rd = PC end block, others are fine
  ENTRY(IRC_REPLACE, armRsbInstruction,         NULL,                   0x0060f000, 0x0fe0f010, "RSB PC, Rn, Rm, #shamt"),
  ENTRY(IRC_REPLACE, armRsbInstruction,         NULL,                   0x0060f010, 0x0fe0f090, "RSB PC, Rn, Rm, Rshamt"),
  ENTRY(IRC_SAFE,    armRsbInstruction,         armDPImmRegRSR,         0x00600000, 0x0fe00010, "RSB Rd, Rn, Rm, #shamt"),
  ENTRY(IRC_SAFE,    armRsbInstruction,         NULL,                   0x00600010, 0x0fe00090, "RSB Rd, Rn, Rm, Rshamt"),
  // ADD: Rd = PC end block, others are fine
  ENTRY(IRC_REPLACE, armAddInstruction,         NULL,                   0x0080f000, 0x0fe0f010, "ADD PC, Rn, Rm, #shamt"),
  ENTRY(IRC_REPLACE, armAddInstruction,         NULL,                   0x0080f010, 0x0fe0f090, "ADD PC, Rn, Rm, Rshamt"),
  ENTRY(IRC_SAFE,    armAddInstruction,         armDPImmRegRSR,         0x00800000, 0x0fe00010, "ADD Rd, Rn, Rm, #shamt"),
  ENTRY(IRC_SAFE,    armAddInstruction,         NULL,                   0x00800010, 0x0fe00090, "ADD Rd, Rn, Rm, Rshamt"),
  // ADC: Rd = PC end block, others are fine
  ENTRY(IRC_REPLACE, armAdcInstruction,         NULL,                   0x00a0f000, 0x0fe0f010, "ADC PC, Rn, Rm, #shamt"),
  ENTRY(IRC_REPLACE, armAdcInstruction,         NULL,                   0x00a0f010, 0x0fe0f090, "ADC PC, Rn, Rm, Rshamt"),
  ENTRY(IRC_SAFE,    armAdcInstruction,         armDPImmRegRSR,         0x00a00000, 0x0fe00010, "ADC Rd, Rn, Rm, #shamt"),
  ENTRY(IRC_SAFE,    armAdcInstruction,         NULL,                   0x00a00010, 0x0fe00090, "ADC Rd, Rn, Rm, Rshamt"),
  // SBC: Rd = PC end block, others are fine
  ENTRY(IRC_REPLACE, armSbcInstruction,         NULL,                   0x00c0f000, 0x0fe0f010, "SBC Rd, Rn, Rm, #shamt"),
  ENTRY(IRC_REPLACE, armSbcInstruction,         NULL,                   0x00c0f010, 0x0fe0f090, "SBC Rd, Rn, Rm, Rshamt"),
  ENTRY(IRC_SAFE,    armSbcInstruction,         armDPImmRegRSR,         0x00c00000, 0x0fe00010, "SBC Rd, Rn, Rm, #shamt"),
  ENTRY(IRC_SAFE,    armSbcInstruction,         NULL,                   0x00c00010, 0x0fe00090, "SBC Rd, Rn, Rm, Rshamt"),
  // RSC: Rd = PC end block, others are fine
  ENTRY(IRC_REPLACE, armRscInstruction,         NULL,                   0x00e0f000, 0x0fe0f010, "RSC PC, Rn, Rm, #shamt"),
  ENTRY(IRC_REPLACE, armRscInstruction,         NULL,                   0x00e0f010, 0x0fe0f090, "RSC PC, Rn, Rm, Rshamt"),
  ENTRY(IRC_SAFE,    armRscInstruction,         armDPImmRegRSR,         0x00e00000, 0x0fe00010, "RSC Rd, Rn, Rm, #shamt"),
  ENTRY(IRC_SAFE,    armRscInstruction,         NULL,                   0x00e00010, 0x0fe00090, "RSC Rd, Rn, Rm, Rshamt"),
  // MSR/MRS: always hypercall! we must hide the real state from guest.
  ENTRY(IRC_REPLACE, armMsrInstruction,         NULL,                   0x0120f000, 0x0fb0fff0, "MSR, s/cpsr, Rn"),
  ENTRY(IRC_REPLACE, armMrsInstruction,         NULL,                   0x010f0000, 0x0fbf0fff, "MRS, Rn, s/cpsr"),
  // TST instructions are all fine
  ENTRY(IRC_SAFE,    armTstInstruction,         armDPImmRegRSRNoDest,   0x01000000, 0x0fe00010, "TST Rn, Rm, #shamt"),
  ENTRY(IRC_SAFE,    armTstInstruction,         NULL,                   0x01000010, 0x0fe00090, "TST Rn, Rm, Rshift"),
  // TEQ instructions are all fine
  ENTRY(IRC_SAFE,    armTeqInstruction,         armDPImmRegRSRNoDest,   0x01200000, 0x0fe00010, "TEQ Rn, Rm, #shamt"),
  ENTRY(IRC_SAFE,    armTeqInstruction,         NULL,                   0x01200010, 0x0fe00090, "TEQ Rn, Rm, Rshift"),
  // CMP instructions are all fine
  ENTRY(IRC_SAFE,    armCmpInstruction,         armDPImmRegRSRNoDest,   0x01400000, 0x0fe00010, "CMP Rn, Rm, #shamt"),
  ENTRY(IRC_SAFE,    armCmpInstruction,         NULL,                   0x01400010, 0x0fe00090, "CMP Rn, Rm, Rshamt"),
  // CMN instructions are all fine
  ENTRY(IRC_SAFE,    armCmnInstruction,         armDPImmRegRSRNoDest,   0x01600000, 0x0fe00010, "CMN Rn, Rm, #shamt"),
  ENTRY(IRC_SAFE,    armCmnInstruction,         NULL,                   0x01600010, 0x0fe00090, "CMN Rn, Rm, Rshamt"),
  // ORR: Rd = PC end block, other are fine
  ENTRY(IRC_REPLACE, armOrrInstruction,         NULL,                   0x0180f000, 0x0fe0f010, "ORR PC, Rn, Rm, #shamt"),
  ENTRY(IRC_REPLACE, armOrrInstruction,         NULL,                   0x0180f010, 0x0fe0f090, "ORR PC, Rn, Rm, Rshamt"),
  ENTRY(IRC_SAFE,    armOrrInstruction,         armDPImmRegRSR,         0x01800000, 0x0fe00010, "ORR Rd, Rn, Rm, #shamt"),
  ENTRY(IRC_SAFE,    armOrrInstruction,         NULL,                   0x01800010, 0x0fe00090, "ORR Rd, Rn, Rm, Rshamt"),
  // MOV with Rd = PC end block. MOV reg, <shifted reg> is a pseudo instr for shift instructions
  ENTRY(IRC_REPLACE, armMovInstruction,         NULL,                   0x01a0f000, 0x0deffff0, "MOV PC, Rm"),
  ENTRY(IRC_SAFE,    armMovInstruction,         armMovPCInstruction,    0x01a00000, 0x0def0ff0, "MOV Rn, Rm"),
  // LSL: Rd = PC and shamt = #imm, then end block. Otherwise fine
  ENTRY(IRC_REPLACE, armLslInstruction,         NULL,                   0x01a0f000, 0x0feff070, "LSL Rd, Rm, #shamt"),
  ENTRY(IRC_REPLACE, armLslInstruction,         NULL,                   0x01a0f010, 0x0feff0f0, "LSL Rd, Rm, Rshamt"),
  ENTRY(IRC_SAFE,    armLslInstruction,         armShiftPCInstruction,  0x01a00000, 0x0fef0070, "LSL Rd, Rm, #shamt"),
  ENTRY(IRC_SAFE,    armLslInstruction,         NULL,                   0x01a00010, 0x0fef00f0, "LSL Rd, Rm, Rshamt"),
  // LSR: Rd = PC and shamt = #imm, then end block. Otherwise fine
  ENTRY(IRC_REPLACE, armLsrInstruction,         NULL,                   0x01a0f020, 0x0feff070, "LSR Rd, Rm, #shamt"),
  ENTRY(IRC_REPLACE, armLsrInstruction,         NULL,                   0x01a0f030, 0x0feff0f0, "LSR Rd, Rm, Rshamt"),
  ENTRY(IRC_SAFE,    armLsrInstruction,         armShiftPCInstruction,  0x01a00020, 0x0fef0070, "LSR Rd, Rm, #shamt"),
  ENTRY(IRC_SAFE,    armLsrInstruction,         NULL,                   0x01a00030, 0x0fef00f0, "LSR Rd, Rm, Rshamt"),
  // ASR: Rd = PC and shamt = #imm, then end block. Otherwise fine
  ENTRY(IRC_REPLACE, armAsrInstruction,         NULL,                   0x01a0f040, 0x0feff070, "ASR Rd, Rm, #shamt"),
  ENTRY(IRC_REPLACE, armAsrInstruction,         NULL,                   0x01a0f050, 0x0feff0f0, "ASR Rd, Rm, Rshamt"),
  ENTRY(IRC_SAFE,    armAsrInstruction,         armShiftPCInstruction,  0x01a00040, 0x0fef0070, "ASR Rd, Rm, #shamt"),
  ENTRY(IRC_SAFE,    armAsrInstruction,         NULL,                   0x01a00050, 0x0fef00f0, "ASR Rd, Rm, Rshamt"),
  // RRX: shift right and extend, Rd can be PC
  ENTRY(IRC_REPLACE, armRrxInstruction,         NULL,                   0x01a0f060, 0x0feffff0, "RRX PC, Rm"),
  ENTRY(IRC_SAFE,    armRrxInstruction,         armShiftPCInstruction,  0x01a00060, 0x0fef0ff0, "RRX Rd, Rm"),
  // ROR: reg case destination unpredictable. imm case dest can be PC.
  ENTRY(IRC_SAFE,    armRorInstruction,         NULL,                   0x01a00070, 0x0fef00f0, "ROR Rd, Rm, Rn"),
  ENTRY(IRC_REPLACE, armRorInstruction,         NULL,                   0x01a0f060, 0x0feff070, "ROR PC, Rm, #imm"),
  ENTRY(IRC_SAFE,    armRorInstruction,         armShiftPCInstruction,  0x01a00060, 0x0fef0070, "ROR Rd, Rm, #imm"),
  // BIC with Rd = PC end block, other are fine.
  ENTRY(IRC_REPLACE, armBicInstruction,         NULL,                   0x01c0f000, 0x0fe0f010, "BIC PC, Rn, Rm, #shamt"),
  ENTRY(IRC_REPLACE, armBicInstruction,         NULL,                   0x01c0f010, 0x0fe0f090, "BIC PC, Rn, Rm, Rshamt"),
  ENTRY(IRC_SAFE,    armBicInstruction,         armDPImmRegRSR,         0x01c00000, 0x0fe00010, "BIC Rd, Rn, Rm, #shamt"),
  ENTRY(IRC_SAFE,    armBicInstruction,         NULL,                   0x01c00010, 0x0fe00090, "BIC Rd, Rn, Rm, Rshamt"),
  // MVN with Rd = PC end block, other are fine.
  ENTRY(IRC_REPLACE, armMvnInstruction,         NULL,                   0x01e0f000, 0x0fe0f010, "MVN Rd, Rm, #shamt"),
  ENTRY(IRC_REPLACE, armMvnInstruction,         NULL,                   0x01e0f010, 0x0fe0f090, "MVN Rd, Rm, Rshamt"),
  ENTRY(IRC_SAFE,    armMvnInstruction,         armShiftPCInstruction,  0x01e00000, 0x0fe00010, "MVN Rd, Rm, #shamt"),
  ENTRY(IRC_SAFE,    armMvnInstruction,         NULL,                   0x01e00010, 0x0fe00090, "MVN Rd, Rm, Rshamt"),

  ENTRY(IRC_REPLACE, undefinedInstruction,      NULL,                   0x00000000, 0x00000000, "dataProcMiscInstructions_op0")
};

// immediate cases and random hints
static struct decodingTableEntry armDataProcMiscInstructions_op1[] =
{
  // UNIMPLEMENTED: yield hint
  ENTRY(IRC_REPLACE, armYieldInstruction,       NULL,                   0x0320f001, 0x0fffffff, "yield%c"),
  // UNIMPLEMENTED: wait for event hint
  ENTRY(IRC_REPLACE, armWfeInstruction,         NULL,                   0x0320f002, 0x0fffffff, "wfe%c"),
  // UNIMPLEMENTED: wait for interrupt hint
  ENTRY(IRC_REPLACE, armWfiInstruction,         NULL,                   0x0320f003, 0x0fffffff, "wfi%c"),
  // UNIMPLEMENTED: send event hint
  ENTRY(IRC_REPLACE, armSevInstruction,         NULL,                   0x0320f004, 0x0fffffff, "sev%c"),
  ENTRY(IRC_SAFE,    nopInstruction,            NULL,                   0x0320f000, 0x0fffff00, "NOP"),
  // UNIMPLEMENTED: debug hint
  ENTRY(IRC_REPLACE, armDbgInstruction,         NULL,                   0x0320f0f0, 0x0ffffff0, "dbg%c\t#%0-3d"),
  // MOVW - indication of 'wide' - to select ARM encoding. Rd cant be PC, pass through.
  ENTRY(IRC_SAFE,    armMovwInstruction,        NULL,                   0x03000000, 0x0ff00000, "MOVW Rd, Rn"),
  ENTRY(IRC_SAFE,    armMovtInstruction,        NULL,                   0x03400000, 0x0ff00000, "MOVT Rd, Rn"),
  // AND: Rd = PC end block, others are fine
  ENTRY(IRC_REPLACE, armAndInstruction,         NULL,                   0x0200f000, 0x0fe0f000, "AND PC, Rn, #imm"),
  ENTRY(IRC_SAFE,    armAndInstruction,         armDPImmRegRSR,         0x02000000, 0x0fe00000, "AND Rd, Rn, #imm"),
  // EOR: Rd = PC end block, others are fine
  ENTRY(IRC_REPLACE, armEorInstruction,         NULL,                   0x0220f000, 0x0fe0f000, "EOR PC, Rn, Rm/#imm"),
  ENTRY(IRC_SAFE,    armEorInstruction,         armDPImmRegRSR,         0x02200000, 0x0fe00000, "EOR Rd, Rn, #imm"),
  // SUB: Rd = PC end block, others are fine
  ENTRY(IRC_REPLACE, armSubInstruction,         NULL,                   0x0240f000, 0x0fe0f000, "SUB PC, Rn, Rm/imm"),
  ENTRY(IRC_SAFE,    armSubInstruction,         armDPImmRegRSR,         0x02400000, 0x0fe00000, "SUB Rd, Rn, #imm"),
  // RSB: Rd = PC end block, others are fine
  ENTRY(IRC_REPLACE, armRsbInstruction,         NULL,                   0x0260f000, 0x0fe0f000, "RSB PC, Rn, Rm/imm"),
  ENTRY(IRC_SAFE,    armRsbInstruction,         armDPImmRegRSR,         0x02600000, 0x0fe00000, "RSB Rd, Rn, #imm"),
  // ADD: Rd = PC end block, others are fine
  ENTRY(IRC_REPLACE, armAddInstruction,         NULL,                   0x0280f000, 0x0fe0f000, "ADD PC, Rn, #imm"),
  ENTRY(IRC_SAFE,    armAddInstruction,         armDPImmRegRSR,         0x02800000, 0x0fe00000, "ADD Rd, Rn, #imm"),
  // ADC: Rd = PC end block, others are fine
  ENTRY(IRC_REPLACE, armAdcInstruction,         NULL,                   0x02a0f000, 0x0fe0f000, "ADC PC, Rn/#imm"),
  ENTRY(IRC_SAFE,    armAdcInstruction,         armDPImmRegRSR,         0x02a00000, 0x0fe00000, "ADC Rd, Rn, #imm"),
  // SBC: Rd = PC end block, others are fine
  ENTRY(IRC_REPLACE, armSbcInstruction,         NULL,                   0x02c0f000, 0x0fe0f000, "SBC PC, Rn/#imm"),
  ENTRY(IRC_SAFE,    armSbcInstruction,         armDPImmRegRSR,         0x02c00000, 0x0fe00000, "SBC Rd, Rn, #imm"),
  // RSC: Rd = PC end block, others are fine
  ENTRY(IRC_REPLACE, armRscInstruction,         NULL,                   0x02e0f000, 0x0fe0f000, "RSC PC, Rn/#imm"),
  ENTRY(IRC_SAFE,    armRscInstruction,         armDPImmRegRSR,         0x02e00000, 0x0fe00000, "RSC Rd, Rn, #imm"),
  // MSR: always hypercall! we must hide the real state from guest.
  ENTRY(IRC_REPLACE, armMsrInstruction,         NULL,                   0x0320f000, 0x0fb0f000, "MSR, s/cpsr, #imm"),
  // TST instructions are all fine
  ENTRY(IRC_SAFE,    armTstInstruction,         armDPImmRegRSRNoDest,   0x03000000, 0x0fe00000, "TST Rn, #imm"),
  // TEQ instructions are all fine
  ENTRY(IRC_SAFE,    armTeqInstruction,         armDPImmRegRSRNoDest,   0x03200000, 0x0fe00000, "TEQ Rn, #imm"),
  // CMP instructions are all fine
  ENTRY(IRC_SAFE,    armCmpInstruction,         armDPImmRegRSRNoDest,   0x03400000, 0x0fe00000, "CMP Rn, #imm"),
  // CMN instructions are all fine
  ENTRY(IRC_SAFE,    armCmnInstruction,         armDPImmRegRSRNoDest,   0x03600000, 0x0fe00000, "CMN Rn, #imm"),
  // ORR: Rd = PC end block, other are fine
  ENTRY(IRC_REPLACE, armOrrInstruction,         NULL,                   0x0380f000, 0x0fe0f000, "ORR Rd, Rn, #imm"),
  ENTRY(IRC_SAFE,    armOrrInstruction,         armDPImmRegRSR,         0x03800000, 0x0fe00000, "ORR Rd, Rn, #imm"),
  // MOV with Rd = PC end block. MOV <shifted reg> is a pseudo instr..
  ENTRY(IRC_REPLACE, armMovInstruction,         NULL,                   0x03a0f000, 0x0feff000, "MOV PC, #imm"),
  ENTRY(IRC_SAFE,    armMovInstruction,         NULL,                   0x03a00000, 0x0fef0000, "MOV Rn, #imm"),
  // BIC with Rd = PC end block, other are fine.
  ENTRY(IRC_REPLACE, armBicInstruction,         NULL,                   0x03c0f000, 0x0fe0f000, "BIC PC, Rn, #imm"),
  ENTRY(IRC_SAFE,    armBicInstruction,         armDPImmRegRSR,         0x03c00000, 0x0fe00000, "BIC Rd, Rn, #imm"),
  // MVN with Rd = PC end block, other are fine.
  ENTRY(IRC_REPLACE, armMvnInstruction,         NULL,                   0x03e0f000, 0x0fe0f000, "MVN PC, #imm"),
  ENTRY(IRC_SAFE,    armMvnInstruction,         NULL,                   0x03e00000, 0x0fe00000, "MVN Rd, #imm"),

  ENTRY(IRC_REPLACE, undefinedInstruction,      NULL,                   0x00000000, 0x00000000, "armDataProcMiscInstructions_op1")
};

static struct decodingTableEntry armLoadStoreWordByteInstructions[] =
{
  // STRT - all trap
  ENTRY(IRC_REPLACE, armStrtInstruction,         armStrtPCInstruction,  0x04200000, 0x0f700000, "STRT Rt, [Rn], +-imm12"),
  ENTRY(IRC_REPLACE, armStrtInstruction,         armStrtPCInstruction,  0x06200000, 0x0f700010, "STRT Rt, [Rn], +-Rm"),
  // LDRT - all trap
  ENTRY(IRC_REPLACE, armLdrtInstruction,         NULL,                  0x04300000, 0x0f700000, "LDRT Rd, [Rn], +-imm12"),
  ENTRY(IRC_REPLACE, armLdrtInstruction,         NULL,                  0x06300000, 0x0f700010, "LDRT Rd, [Rn], +-Rm"),
  // STRBT - all trap
  ENTRY(IRC_REPLACE, armStrbtInstruction,        NULL,                  0x04600000, 0x0f700000, "STRBT Rt, [Rn, +-imm12]"),
  ENTRY(IRC_REPLACE, armStrbtInstruction,        NULL,                  0x06600000, 0x0f700010, "STRBT Rt, [Rn], +-Rm"),
  // LDRBT - all trap
  ENTRY(IRC_REPLACE, armLdrbtInstruction,        NULL,                  0x04700000, 0x0f700000, "LDRBT Rd, [Rn], +-imm12"),
  ENTRY(IRC_REPLACE, armLdrbtInstruction,        NULL,                  0x06700000, 0x0f700010, "LDRBT Rd, [Rn], +-Rm"),
  // STR - all pass-through
  ENTRY(IRC_SAFE,    armStrInstruction,          armStrPCInstruction,   0x04000000, 0x0e500000, "STR Rt, [Rn, +-imm12]"),
  ENTRY(IRC_SAFE,    armStrInstruction,          armStrPCInstruction,   0x06000000, 0x0e500010, "STR Rt, [Rn], +-Rm"),
  // LDR traps if dest = PC, otherwise pass through
  ENTRY(IRC_REPLACE, armLdrInstruction,          NULL,                  0x0410f000, 0x0e50f000, "LDR PC, [Rn], +-imm12"),
  ENTRY(IRC_SAFE,    armLdrInstruction,          armLdrPCInstruction,   0x04100000, 0x0e500000, "LDR Rd, [Rn], +-imm12"),
  ENTRY(IRC_REPLACE, armLdrInstruction,          NULL,                  0x0610f000, 0x0e50f010, "LDR PC, [Rn], +-Rm"),
  ENTRY(IRC_SAFE,    armLdrInstruction,          armLdrPCInstruction,   0x06100000, 0x0e500010, "LDR Rd, [Rn], +-Rm"),
  // STRB pass through
  ENTRY(IRC_SAFE,    armStrbInstruction,         armStrPCInstruction,   0x04400000, 0x0e500000, "STRB Rt, [Rn, +-imm12]"),
  ENTRY(IRC_SAFE,    armStrbInstruction,         armStrPCInstruction,   0x06400000, 0x0e500010, "STRB Rt, [Rn], +-Rm"),
  // LDRB - pass through, dest can't be PC
  ENTRY(IRC_SAFE,    armLdrbInstruction,         armLdrPCInstruction,   0x04500000, 0x0e500000, "LDRB Rd, [Rn], +-imm12"),
  ENTRY(IRC_SAFE,    armLdrbInstruction,         armLdrPCInstruction,   0x06500000, 0x0e500010, "LDRB Rd, [Rn], +-Rm"),

  ENTRY(IRC_REPLACE, undefinedInstruction,       NULL,                  0x00000000, 0x00000000, "armLoadStoreWordByteInstructions")
};

static struct decodingTableEntry armMediaInstructions[] =
{
  // BFC: bit field clear, dest PC not allowed.
  ENTRY(IRC_SAFE,    armBfcInstruction,          NULL,                  0x07c0001f, 0x0fe0007f, "BFC Rd, #LSB, #width"),
  // BFI: bit field insert, dest PC not allowed.
  ENTRY(IRC_SAFE,    armBfiInstruction,          NULL,                  0x07c00010, 0x0fe00070, "BFI Rd, #LSB, #width"),
  // RBIT: reverse bits, if Rd or Rm = 15 then unpredictable.
  ENTRY(IRC_SAFE,    armRbitInstruction,         NULL,                  0x06ff0f30, 0x0fff0ff0, "RBIT Rd,Rm"),
  // UBFX: extract bit field - destination 15 unpredictable
  ENTRY(IRC_SAFE,    armUbfxInstruction,         NULL,                  0x07a00050, 0x0fa00070, "UBFX Rd, Rn, width"),
  // PKH (pack halfword), if Rd or Rn or Rm = 15 then unpredictable
  ENTRY(IRC_SAFE,    armPkhbtInstruction,        NULL,                  0x06800010, 0x0ff00ff0, "PKHBT Rd,Rn,Rm"),
  ENTRY(IRC_SAFE,    armPkhbtInstruction,        NULL,                  0x06800010, 0x0ff00070, "PKHBT Rd,Rn,Rm,LSL #imm"),
  ENTRY(IRC_SAFE,    armPkhtbInstruction,        NULL,                  0x06800050, 0x0ff00ff0, "PKHTB Rd,Rn,Rm,ASR #32"),
  ENTRY(IRC_SAFE,    armPkhtbInstruction,        NULL,                  0x06800050, 0x0ff00070, "PKHTB Rd,Rn,Rm,ASR #imm"),
  // QADD8,QADD16: saturating add 16 bit and 8 bit, if Rd or Rn or Rm = 15 then unpredictable
  ENTRY(IRC_SAFE,    armQadd16Instruction,       NULL,                  0x06200f10, 0x0ff00ff0, "QADD16 Rd,Rn,Rm"),
  ENTRY(IRC_SAFE,    armQadd8Instruction,        NULL,                  0x06200f90, 0x0ff00ff0, "QADD8 Rd,Rn,Rm"),
  // QASX: saturating add and subtract with exchange if Rd or Rn or Rm = 15 then unpredictable
  ENTRY(IRC_SAFE,    armQasxInstruction,         NULL,                  0x06200f30, 0x0ff00ff0, "QASX Rd,Rn,Rm"),
  // QSUB8,QSUB16: saturating subtract 16 and 8 bit if Rd or Rn or Rm = 15 then unpredictable
  ENTRY(IRC_SAFE,    armQsub16Instruction,       NULL,                  0x06200f70, 0x0ff00ff0, "QSUB16 Rd,Rn,Rm"),
  ENTRY(IRC_SAFE,    armQsub8Instruction,        NULL,                  0x06200ff0, 0x0ff00ff0, "QSUB8 Rd,Rn,Rm"),
  // QSAX: saturating subtract and add with exchange if Rd or Rn or Rm = 15 then unpredictable
  ENTRY(IRC_SAFE,    armQsaxInstruction,         NULL,                  0x06200f50, 0x0ff00ff0, "QSAX Rd,Rn,Rm"),
  // SADD8,SADD16: signed add 16 bit and 8 bit, if Rd or Rn or Rm = 15 then unpredictable
  ENTRY(IRC_SAFE,    armSadd16Instruction,       NULL,                  0x06100f10, 0x0ff00ff0, "SADD16 Rd,Rn,Rm"),
  ENTRY(IRC_SAFE,    armSadd8Instruction,        NULL,                  0x06100f90, 0x0ff00ff0, "SADD8 Rd,Rn,Rm"),
  // SASX: signed add and subtract with exchange, if Rd or Rn or Rm = 15 then unpredictable
  ENTRY(IRC_SAFE,    armSasxInstruction,         NULL,                  0x06100f30, 0x0ff00ff0, "SASX Rd,Rn,Rm"),
  // SSUB8,SSUB16: signed subtract 16 bit and 8 bit, if Rd or Rn or Rm = 15 then unpredictable
  ENTRY(IRC_SAFE,    armSsub16Instruction,       NULL,                  0x06100f70, 0x0ff00ff0, "SSUB16 Rd,Rn,Rm"),
  ENTRY(IRC_SAFE,    armSsub8Instruction,        NULL,                  0x06100ff0, 0x0ff00ff0, "SSUB8 Rd,Rn,Rm"),
  // SSAX: signed subtract and add with exchange,if Rd or Rn or Rm = 15 then unpredictable
  ENTRY(IRC_SAFE,    armSsaxInstruction,         NULL,                  0x06100f50, 0x0ff00ff0, "SSAX Rd,Rn,Rm"),
  // SHADD8,SHADD16: signed halvign add 16 bit and 8 bit, if Rd or Rn or Rm = 15 then unpredictable
  ENTRY(IRC_SAFE,    armShadd16Instruction,      NULL,                  0x06300f10, 0x0ff00ff0, "SHADD16 Rd,Rn,Rm"),
  ENTRY(IRC_SAFE,    armShadd8Instruction,       NULL,                  0x06300f90, 0x0ff00ff0, "SHADD8 Rd,Rn,Rm"),
  // SHASX: signed halving add and subtract with exchange, if Rd or Rn or Rm = 15 then unpredictable
  ENTRY(IRC_SAFE,    armShasxInstruction,        NULL,                  0x06300f30, 0x0ff00ff0, "SHASX Rd,Rn,Rm"),
  // SHSUB8,SHSUB16: signed halvign subtract 16 bit and 8 bit, if Rd or Rn or Rm = 15 then unpredictable
  ENTRY(IRC_SAFE,    armShsub16Instruction,      NULL,                  0x06300f70, 0x0ff00ff0, "SHSUB16 Rd,Rn,Rm"),
  ENTRY(IRC_SAFE,    armShsub8Instruction,       NULL,                  0x06300ff0, 0x0ff00ff0, "SHSUB8 Rd,Rn,Rm"),
  // SHSAX: signed halving subtract and add with exchange, if Rd or Rn or Rm = 15 then unpredictable
  ENTRY(IRC_SAFE,    armShsaxInstruction,        NULL,                  0x06300f50, 0x0ff00ff0, "SHSAX Rd,Rn,Rm"),
  // UADD8,UADD16: unsigned add 16 bit and 8 bit, if Rd or Rn or Rm = 15 then unpredictable
  ENTRY(IRC_SAFE,    armUadd16Instruction,       NULL,                  0x06500f10, 0x0ff00ff0, "UADD16 Rd,Rn,Rm"),
  ENTRY(IRC_SAFE,    armUadd8Instruction,        NULL,                  0x06500f90, 0x0ff00ff0, "UADD8 Rd,Rn,Rm"),
  // UASX: unsigned add and subtract with exchange, if Rd or Rn or Rm = 15 then unpredictable
  ENTRY(IRC_SAFE,    armUasxInstruction,         NULL,                  0x06500f30, 0x0ff00ff0, "UASX Rd,Rn,Rm"),
  // USUB8,USUB16: unsigned subtract 16 bit and 8 bit, if Rd or Rn or Rm = 15 then unpredictable
  ENTRY(IRC_SAFE,    armUsub16Instruction,       NULL,                  0x06500f70, 0x0ff00ff0, "USUB16 Rd,Rn,Rm"),
  ENTRY(IRC_SAFE,    armUsub8Instruction,        NULL,                  0x06500ff0, 0x0ff00ff0, "USUB8 Rd,Rn,Rm"),
  // USAX: unsigned subtract and add with exchange, if Rd or Rn or Rm = 15 then unpredictable
  ENTRY(IRC_SAFE,    armUsaxInstruction,         NULL,                  0x06500f50, 0x0ff00ff0, "USAX Rd,Rn,Rm"),
  // UHADD8,UHADD16: unsigned halving add 16 bit and 8 bit, if Rd or Rn or Rm = 15 then unpredictable
  ENTRY(IRC_SAFE,    armUhadd16Instruction,      NULL,                  0x06700f10, 0x0ff00ff0, "UHADD16 Rd,Rn,Rm"),
  ENTRY(IRC_SAFE,    armUhadd8Instruction,       NULL,                  0x06700f90, 0x0ff00ff0, "UHADD8 Rd,Rn,Rm"),
  // UHASX: unsigned halving add and subtract with exchange, if Rd or Rn or Rm = 15 then unpredictable
  ENTRY(IRC_SAFE,    armUhasxInstruction,        NULL,                  0x06700f30, 0x0ff00ff0, "UHASX Rd,Rn,Rm"),
  // UHSUB8,UHSUB16: unsigned halving subtract 16 bit and 8 bit, if Rd or Rn or Rm = 15 then unpredictable
  ENTRY(IRC_SAFE,    armUhsub16Instruction,      NULL,                  0x06700f70, 0x0ff00ff0, "UHSUB16 Rd,Rn,Rm"),
  ENTRY(IRC_SAFE,    armUhsub8Instruction,       NULL,                  0x06700ff0, 0x0ff00ff0, "UHSUB8 Rd,Rn,Rm"),
  // UHSAX: unsigned halving subtract and add with exchange, if Rd or Rn or Rm = 15 then unpredictable
  ENTRY(IRC_SAFE,    armUhsaxInstruction,        NULL,                  0x06700f50, 0x0ff00ff0, "UHSAX Rd,Rn,Rm"),
  // UQADD8,UQADD16:  unsigned saturating add 16 bit and 8 bit, if Rd or Rn or Rm = 15 then unpredictable
  ENTRY(IRC_SAFE,    armUqadd16Instruction,      NULL,                  0x06600f10, 0x0ff00ff0, "UQADD16 Rd,Rn,Rm"),
  ENTRY(IRC_SAFE,    armUqadd8Instruction,       NULL,                  0x06600f90, 0x0ff00ff0, "UQADD8 Rd,Rn,Rm"),
  // UQASX: unsigned saturating add and subtract with exchange, if Rd or Rn or Rm = 15 then unpredictable
  ENTRY(IRC_SAFE,    armUqasxInstruction,        NULL,                  0x06600f30, 0x0ff00ff0, "UQASX Rd,Rn,Rm"),
  // UQSUB8,UQSUB16: unsigned saturating subtract 16 bit and 8 bit, if Rd or Rn or Rm = 15 then unpredictable
  ENTRY(IRC_SAFE,    armUqsub16Instruction,      NULL,                  0x06600f70, 0x0ff00ff0, "UQSUB16 Rd,Rn,Rm"),
  ENTRY(IRC_SAFE,    armUqsub8Instruction,       NULL,                  0x06600ff0, 0x0ff00ff0, "UQSUB8 Rd,Rn,Rm"),
  // UQSAX: unsigned saturating subtract and add with exchange, if Rd or Rn or Rm = 15 then unpredictable
  ENTRY(IRC_SAFE,    armUqsaxInstruction,        NULL,                  0x06600f50, 0x0ff00ff0, "UQSAX Rd,Rn,Rm"),
  // REV (byte-reverse word), if Rd or Rm = 15 then unpredictable.
  ENTRY(IRC_SAFE,    armRevInstruction,          NULL,                  0x06bf0f30, 0x0fff0ff0, "REV Rd,Rm"),
  // REV16 (byte-reverse packed halfword), if Rd or Rm = 15 then unpredictable.
  ENTRY(IRC_SAFE,    armRev16Instruction,        NULL,                  0x06bf0fb0, 0x0fff0ff0, "REV16 Rd,Rm"),
  // REVSH (byte-reverse signed halfword), if Rd or Rm = 15 then unpredictable.
  ENTRY(IRC_SAFE,    armRevshInstruction,        NULL,                  0x06ff0fb0, 0x0fff0ff0, "REVSH Rd,Rm"),
  // SXTH (sign-extend halfword), if Rd or Rm = 15 then unpredictable.
  ENTRY(IRC_SAFE,    armSxthInstruction,         NULL,                  0x06bf0070, 0x0fff0ff0, "SXTH Rd,Rm"),
  ENTRY(IRC_SAFE,    armSxthInstruction,         NULL,                  0x06bf0470, 0x0fff0ff0, "SXTH Rd,Rm,ROR #8"),
  ENTRY(IRC_SAFE,    armSxthInstruction,         NULL,                  0x06bf0870, 0x0fff0ff0, "SXTH Rd,Rm,ROR #16"),
  ENTRY(IRC_SAFE,    armSxthInstruction,         NULL,                  0x06bf0c70, 0x0fff0ff0, "SXTH Rd,Rm,ROR #24"),
  // SXTB16: sign-extend byte 16, if Rd or Rm = 15 then unpredictable.
  ENTRY(IRC_SAFE,    armSxtb16Instruction,       NULL,                  0x068f0070, 0x0fff0ff0, "SXTB16 Rd,Rm"),
  ENTRY(IRC_SAFE,    armSxtb16Instruction,       NULL,                  0x068f0470, 0x0fff0ff0, "SXTB16 Rd,Rm,ROR #8"),
  ENTRY(IRC_SAFE,    armSxtb16Instruction,       NULL,                  0x068f0870, 0x0fff0ff0, "SXTB16 Rd,Rm,ROR #16"),
  ENTRY(IRC_SAFE,    armSxtb16Instruction,       NULL,                  0x068f0c70, 0x0fff0ff0, "SXTB16 Rd,Rm,ROR #24"),
  // SXTB sign extend byte, if Rd or Rn = 15 then unpredictable.
  ENTRY(IRC_SAFE,    armSxtbInstruction,         NULL,                  0x06af0070, 0x0fff0ff0, "SXTB Rd,Rm"),
  ENTRY(IRC_SAFE,    armSxtbInstruction,         NULL,                  0x06af0470, 0x0fff0ff0, "SXTB Rd,Rm,ROR #8"),
  ENTRY(IRC_SAFE,    armSxtbInstruction,         NULL,                  0x06af0870, 0x0fff0ff0, "SXTB Rd,Rm,ROR #16"),
  ENTRY(IRC_SAFE,    armSxtbInstruction,         NULL,                  0x06af0c70, 0x0fff0ff0, "SXTB Rd,Rm,ROR #24"),
  // UXTH permitted, if Rd or Rn = 15 then unpredictable.
  ENTRY(IRC_SAFE,    armUxthInstruction,         NULL,                  0x06ff0070, 0x0fff0ff0, "UXTH Rd,Rm"),
  ENTRY(IRC_SAFE,    armUxthInstruction,         NULL,                  0x06ff0470, 0x0fff0ff0, "UXTH Rd,Rm,ROR #8"),
  ENTRY(IRC_SAFE,    armUxthInstruction,         NULL,                  0x06ff0870, 0x0fff0ff0, "UXTH Rd,Rm,ROR #16"),
  ENTRY(IRC_SAFE,    armUxthInstruction,         NULL,                  0x06ff0c70, 0x0fff0ff0, "UXTH Rd,Rm,ROR #24"),
  // UXTB16 permitted, if Rd or Rn = 15 then unpredictable.
  ENTRY(IRC_SAFE,    armUxtb16Instruction,       NULL,                  0x06cf0070, 0x0fff0ff0, "UXTB16 Rd,Rm"),
  ENTRY(IRC_SAFE,    armUxtb16Instruction,       NULL,                  0x06cf0470, 0x0fff0ff0, "UXTB16 Rd,Rm,ROR #8"),
  ENTRY(IRC_SAFE,    armUxtb16Instruction,       NULL,                  0x06cf0870, 0x0fff0ff0, "UXTB16 Rd,Rm,ROR #16"),
  ENTRY(IRC_SAFE,    armUxtb16Instruction,       NULL,                  0x06cf0c70, 0x0fff0ff0, "UXTB16 Rd,Rm,ROR #24"),
  // UXTB permitted, if Rd or Rn = 15 then unpredictable.
  ENTRY(IRC_SAFE,    armUxtbInstruction,         NULL,                  0x06ef0070, 0x0fff0ff0, "UXTB Rd,Rm"),
  ENTRY(IRC_SAFE,    armUxtbInstruction,         NULL,                  0x06ef0470, 0x0fff0ff0, "UXTB Rd,Rm,ROR #8"),
  ENTRY(IRC_SAFE,    armUxtbInstruction,         NULL,                  0x06ef0870, 0x0fff0ff0, "UXTB Rd,Rm,ROR #16"),
  ENTRY(IRC_SAFE,    armUxtbInstruction,         NULL,                  0x06ef0c70, 0x0fff0ff0, "UXTB Rd,Rm,ROR #24"),
  // SXTAH (sign-extend and add halfword), if Rd or Rm = 15 then unpredictable, if Rn = 15 see SXTH.
  ENTRY(IRC_SAFE,    armSxtahInstruction,        NULL,                  0x06b00070, 0x0ff00ff0, "SXTAH Rd,Rn,Rm"),
  ENTRY(IRC_SAFE,    armSxtahInstruction,        NULL,                  0x06b00470, 0x0ff00ff0, "SXTAH Rd,Rn,Rm,ROR #8"),
  ENTRY(IRC_SAFE,    armSxtahInstruction,        NULL,                  0x06b00870, 0x0ff00ff0, "SXTAH Rd,Rn,Rm,ROR #16"),
  ENTRY(IRC_SAFE,    armSxtahInstruction,        NULL,                  0x06b00c70, 0x0ff00ff0, "SXTAH Rd,Rn,Rm,ROR #24"),
  // SXTAB16 (sign-extend and add byte 16), if Rd or Rm = 15 then unpredictable, if Rn = 15 see SXTB16.
  ENTRY(IRC_SAFE,    armSxtab16Instruction,      NULL,                  0x06800070, 0x0ff00ff0, "SXTAB16 Rd,Rn,Rm"),
  ENTRY(IRC_SAFE,    armSxtab16Instruction,      NULL,                  0x06800470, 0x0ff00ff0, "SXTAB16 Rd,Rn,Rm,ROR #8"),
  ENTRY(IRC_SAFE,    armSxtab16Instruction,      NULL,                  0x06800870, 0x0ff00ff0, "SXTAB16 Rd,Rn,Rm,ROR #16"),
  ENTRY(IRC_SAFE,    armSxtab16Instruction,      NULL,                  0x06800c70, 0x0ff00ff0, "SXTAB16 Rd,Rn,Rm,ROR #24"),
  // SXTAB (sign-extend and add byte), if Rd or Rm = 15 then unpredictable, if Rn = 15 see SXTB.
  ENTRY(IRC_SAFE,    armSxtabInstruction,        NULL,                  0x06a00070, 0x0ff00ff0, "SXTAB Rd,Rn,Rm"),
  ENTRY(IRC_SAFE,    armSxtabInstruction,        NULL,                  0x06a00470, 0x0ff00ff0, "SXTAB Rd,Rn,Rm,ROR #8"),
  ENTRY(IRC_SAFE,    armSxtabInstruction,        NULL,                  0x06a00870, 0x0ff00ff0, "SXTAB Rd,Rn,Rm,ROR #16"),
  ENTRY(IRC_SAFE,    armSxtabInstruction,        NULL,                  0x06a00c70, 0x0ff00ff0, "SXTAB Rd,Rn,Rm,ROR #24"),
  // UXTAH (unsigned extend and add halfword), if Rd or Rm = 15 then unpredictable, if Rn = 15 see UXTH.
  ENTRY(IRC_SAFE,    armUxtahInstruction,        NULL,                  0x06f00070, 0x0ff00ff0, "UXTAH Rd,Rn,Rm"),
  ENTRY(IRC_SAFE,    armUxtahInstruction,        NULL,                  0x06f00470, 0x0ff00ff0, "UXTAH Rd,Rn,Rm,ROR #8"),
  ENTRY(IRC_SAFE,    armUxtahInstruction,        NULL,                  0x06f00870, 0x0ff00ff0, "UXTAH Rd,Rn,Rm,ROR #16"),
  ENTRY(IRC_SAFE,    armUxtahInstruction,        NULL,                  0x06f00c70, 0x0ff00ff0, "UXTAH Rd,Rn,Rm,ROR #24"),
  // UXTAB16 (unsigned extend and add byte 16), if Rd or Rm = 15 then unpredictable, if Rn = 15 see UXTB16.
  ENTRY(IRC_SAFE,    armUxtab16Instruction,      NULL,                  0x06c00070, 0x0ff00ff0, "UXTAB16 Rd,Rn,Rm"),
  ENTRY(IRC_SAFE,    armUxtab16Instruction,      NULL,                  0x06c00470, 0x0ff00ff0, "UXTAB16 Rd,Rn,Rm,ROR #8"),
  ENTRY(IRC_SAFE,    armUxtab16Instruction,      NULL,                  0x06c00870, 0x0ff00ff0, "UXTAB16 Rd,Rn,Rm,ROR #16"),
  ENTRY(IRC_SAFE,    armUxtab16Instruction,      NULL,                  0x06c00c70, 0x0ff00ff0, "UXTAB16 Rd,Rn,Rm,ROR #24"),
  // UXTAB (unsigned extend and add byte), if Rd or Rm = 15 then unpredictable, if Rn = 15 see UXTB.
  ENTRY(IRC_SAFE,    armUxtabInstruction,        NULL,                  0x06e00070, 0x0ff00ff0, "UXTAB Rd,Rn,Rm"),
  ENTRY(IRC_SAFE,    armUxtabInstruction,        NULL,                  0x06e00470, 0x0ff00ff0, "UXTAB Rd,Rn,Rm,ROR #8"),
  ENTRY(IRC_SAFE,    armUxtabInstruction,        NULL,                  0x06e00870, 0x0ff00ff0, "UXTAB Rd,Rn,Rm,ROR #16"),
  ENTRY(IRC_SAFE,    armUxtabInstruction,        NULL,                  0x06e00c70, 0x0ff00ff0, "UXTAB Rd,Rn,Rm,ROR #24"),
  // SEL (select bytes), if Rd or Rm or Rn = 15 then unpredictable.
  ENTRY(IRC_SAFE,    armSelInstruction,          NULL,                  0x06800fb0, 0x0ff00ff0, "SEL Rd,Rn,Rm"),
  // SMUAD (signed dual multiply add), if Rd or Rm or Rn = 15 then unpredictable.
  ENTRY(IRC_SAFE,    armSmuadInstruction,        NULL,                  0x0700f010, 0x0ff0f0d0, "SMUAD{X} Rd,Rn,Rm"),
  // SMUSD: signed dual multiply subtract, if Rd or Rn or Rm = 15 then unpredictable
  ENTRY(IRC_SAFE,    armSmusdInstruction,        NULL,                  0x0700f050, 0x0ff0f0d0, "SMUSD{X} Rd,Rn,Rm"),
  // SMLAD: signed multiply accumulate dual, if Rd or Rn or Rm = 15 then unpredictable, Ra = 15 see SMUAD
  ENTRY(IRC_SAFE,    armSmladInstruction,        NULL,                  0x07000010, 0x0ff000d0, "SMLAD{X} Rd,Rn,Rm,Ra"),
  // SMLALD: signed multiply accumulate long dual, if any R = 15 then unpredictable
  ENTRY(IRC_SAFE,    armSmlaldInstruction,       NULL,                  0x07400010, 0x0ff000d0, "SMLALD{X} RdLo,RdHi,Rn,Rm"),
  // SMLSD: signed multiply subtract dual, if Rd or Rn or Rm = 15 then unpredictable, Ra = 15 see SMUSD
  ENTRY(IRC_SAFE,    armSmlsdInstruction,        NULL,                  0x07000050, 0x0ff000d0, "SMLSD{X} Rd,Rn,Rm,Ra"),
  // SMLSLD: signed multiply subtract long dual, if any R = 15 then unpredictable
  ENTRY(IRC_SAFE,    armSmlsldInstruction,       NULL,                  0x07400050, 0x0ff000d0, "SMLSLD{X} RdLo,RdHi,Rn,Rm"),
  // SMMUL: signed most significant word multiply, if Rd or Rn or Rm = 15 then unpredictable
  ENTRY(IRC_SAFE,    armSmmulInstruction,        NULL,                  0x0750f010, 0x0ff0f0d0, "SMMUL{R} Rd,Rn,Rm"),
  // SMMLA: signed most significant word multiply accumulate, if Rd or Rn or Rm = 15 then unpredictable, Ra = 15 see SMMUL
  ENTRY(IRC_SAFE,    armSmmlaInstruction,        NULL,                  0x07500010, 0x0ff000d0, "SMMLA{R} Rd,Rn,Rm,Ra"),
  // SMMLS: signed most significant word multiply subtract, if any R = 15 then unpredictable
  ENTRY(IRC_SAFE,    armSmmlsInstruction,        NULL,                  0x075000d0, 0x0ff000d0, "SMMLS{R} Rd,Rn,Rm,Ra"),
  // SSAT,SSAT16,USAT,USAT16: (un)signed saturate, if Rd or Rn = 15 then unpredictable
  ENTRY(IRC_SAFE,    armSsatInstruction,         NULL,                  0x06a00010, 0x0fe00ff0, "SSAT Rd,#sat_imm,Rn"),
  ENTRY(IRC_SAFE,    armSsatInstruction,         NULL,                  0x06a00010, 0x0fe00070, "SSAT Rd,#sat_imm,Rn,LSL #imm"),
  ENTRY(IRC_SAFE,    armSsatInstruction,         NULL,                  0x06a00050, 0x0fe00070, "SSAT Rd,#sat_imm,Rn,ASR #imm"),
  ENTRY(IRC_SAFE,    armSsat16Instruction,       NULL,                  0x06a00f30, 0x0ff00ff0, "SSAT16 Rd,#imm,Rn"),
  ENTRY(IRC_SAFE,    armUsatInstruction,         NULL,                  0x06e00010, 0x0fe00ff0, "USAT Rd,#sat_imm,Rn"),
  ENTRY(IRC_SAFE,    armUsatInstruction,         NULL,                  0x06e00010, 0x0fe00070, "USAT Rd,#sat_imm,Rn,LSL #imm"),
  ENTRY(IRC_SAFE,    armUsatInstruction,         NULL,                  0x06e00050, 0x0fe00070, "USAT Rd,#sat_imm,Rn,ASR #imm"),
  ENTRY(IRC_SAFE,    armUsat16Instruction,       NULL,                  0x06e00f30, 0x0ff00ff0, "USAT16 Rd,#imm,Rn"),

  ENTRY(IRC_REPLACE, undefinedInstruction,       NULL,                  0x00000000, 0x00000000, "armMediaInstructions")
};

static struct decodingTableEntry armSvcCoprocInstructions[] =
{
  // well obviously.
  ENTRY(IRC_REPLACE, svcInstruction,             NULL,                  0x0f000000, 0x0f000000, "SWI code"),
  // Generic coprocessor instructions.
  ENTRY(IRC_REPLACE, armMrcInstruction,          NULL,                  0x0e100010, 0x0f100010, "MRC"),
  ENTRY(IRC_REPLACE, armMcrInstruction,          NULL,                  0x0e000010, 0x0f100010, "MCR"),
  ENTRY(IRC_REPLACE, undefinedInstruction,       NULL,                  0x00000000, 0x00000000, "armSvcCoprocInstructions")
};

static struct decodingTableEntry armUnconditionalInstructions[] =
{
  // UNIMPLEMENTED: data memory barrier
  ENTRY(IRC_REPLACE, armDmbInstruction,          NULL,                  0xf57ff050, 0xfffffff0, "dmb\t%U"),
  // sync barriers
  ENTRY(IRC_REPLACE, armDsbInstruction,          NULL,                  0xf57ff040, 0xfffffff0, "DSB"),
  ENTRY(IRC_REPLACE, armIsbInstruction,          NULL,                  0xf57ff060, 0xfffffff0, "ISB"),
  // UNIMPLEMENTED: CLREX clear exclusive
  ENTRY(IRC_REPLACE, armClrexInstruction,        NULL,                  0xf57ff01f, 0xffffffff, "clrex"),
  // CPS: change processor state
  ENTRY(IRC_REPLACE, armCpsInstruction,          NULL,                  0xf1080000, 0xfffffe3f, "CPSIE"),
  ENTRY(IRC_REPLACE, armCpsInstruction,          NULL,                  0xf10a0000, 0xfffffe20, "CPSIE"),
  ENTRY(IRC_REPLACE, armCpsInstruction,          NULL,                  0xf10C0000, 0xfffffe3f, "CPSID"),
  ENTRY(IRC_REPLACE, armCpsInstruction,          NULL,                  0xf10e0000, 0xfffffe20, "CPSID"),
  ENTRY(IRC_REPLACE, armCpsInstruction,          NULL,                  0xf1000000, 0xfff1fe20, "CPS"),
  // UNIMPLEMENTED: RFE return from exception
  ENTRY(IRC_REPLACE, armRfeInstruction,          NULL,                  0xf8100a00, 0xfe50ffff, "rfe%23?id%24?ba\t%16-19r%21'!"),
  // UNIMPLEMENTED: SETEND set endianess
  ENTRY(IRC_REPLACE, armSetendInstruction,       NULL,                  0xf1010000, 0xfffffc00, "setend\t%9?ble"),
  // UNIMPLEMENTED: SRS store return state
  ENTRY(IRC_REPLACE, armSrsInstruction,          NULL,                  0xf84d0500, 0xfe5fffe0, "srs%23?id%24?ba\t%16-19r%21'!, #%0-4d"),
  // BLX: branch and link to Thumb subroutine
  ENTRY(IRC_REPLACE, armBlxImmediateInstruction, NULL,                  0xfa000000, 0xfe000000, "BLX #imm24"),
  // PLD: preload data
  ENTRY(IRC_REMOVE, armPldInstruction,          NULL,                  0xf450f000, 0xfc70f000, "PLD"),
  // PLI: preload instruction
  ENTRY(IRC_REMOVE, armPliInstruction,          NULL,                  0xf450f000, 0xfd70f000, "PLI"),
  ENTRY(IRC_REPLACE, undefinedInstruction,       NULL,                  0x00000000, 0x00000000, "armUnconditionalInstructions")
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
