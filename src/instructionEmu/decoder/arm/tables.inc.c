/*
 * ARM decoding tables for the table search decoder
 */


static struct instruction32bit armBranchBlockTransferInstructions[] =
{
  // STM traps if ^ postfix, otherwise pass through
  { 0x1,  &armStmInstruction,    0x08400000, 0x0e500000, "STM {regList}^" },
  { 0x0, &armStmInstruction,    0x08800000, 0x0ff00000, "STMIA {regList}" },
  { 0x0, &armStmInstruction,    0x08000000, 0x0e100000, "STM {regList}" },
  // POP LDM syntax, only care if PC in reglist
  { 0x1,  &armLdmInstruction,    0x08bd8000, 0x0fff8000, "LDM SP, {...r15}" },
  { 0x0, &armLdmInstruction,    0x08bd0000, 0x0fff0000, "LDM SP, {reglist}" },
  // LDM traps if ^ postfix and/or PC is in register list, otherwise pass through
  { 0x1,  &armLdmInstruction,    0x08500000, 0x0e500000, "LDM Rn, {regList}^" },
  { 0x1,  &armLdmInstruction,    0x08108000, 0x0e108000, "LDM Rn, {..r15}" },
  { 0x0, &armLdmInstruction,    0x08900000, 0x0f900000, "LDMIA Rn, {regList}" },
  { 0x0, &armLdmInstruction,    0x08100000, 0x0e100000, "LDM Rn, {regList}" },
  // B/BL: always hypercall! obviously.
  { 0x1,  &armBInstruction,      0x0a000000, 0x0e000000, "BRANCH" },
  { 0x1,  &undefinedInstruction, 0x00000000, 0x00000000, "branchBlockTransferInstructions" }
};

// register/shifter register cases, etc
static struct instruction32bit armDataProcMiscInstructions_op0[] =
{
  // NOP is just a nop...
  { 0x0, &nopInstruction,       0xe1a00000, 0xffffffff, "NOP" },
  // UNIMPLEMENTED: SWP swap
  { 0x1,  &armSwpInstruction,       0x01000090, 0x0fb00ff0, "SWP" },
  // UNIMPLEMENTED:
  { 0x2,  &armStrhtInstruction,  0x006000b0, 0x0f7000f0, "STRHT instruction" },
  { 0x2,  &armLdrhtInstruction,  0x003000b0, 0x0f3000f0, "LDRHT instruction" },
  // store and load exclusive: must be emulated - user mode faults
  { 0x1,  &armLdrexbInstruction, 0x01d00f9f, 0x0ff00fff, "LDREXB" },
  { 0x1,  &armLdrexdInstruction, 0x01b00f9f, 0x0ff00fff, "LDREXD" },
  { 0x1,  &armLdrexhInstruction, 0x01f00f9f, 0x0ff00fff, "LDREXH" },
  { 0x1,  &armStrexbInstruction, 0x01c00f90, 0x0ff00ff0, "STREXB" },
  { 0x1,  &armStrexdInstruction, 0x01a00f90, 0x0ff00ff0, "STREXD" },
  { 0x1,  &armStrexhInstruction, 0x01e00f90, 0x0ff00ff0, "STREXH" },
  { 0x1,  &armLdrexInstruction,  0x01900f9f, 0x0ff00fff, "LDREX" },
  { 0x1,  &armStrexInstruction,  0x01800f90, 0x0ff00ff0, "STREX" },
  // SMULL - signed multiply, PC cannot be used as any destination
  { 0x0, &armSmullInstruction,    0x00800090, 0x0fa000f0, "SMULL" },
  // SMLAL - signed multiply and accumulate, PC cannot be used as any destination
  { 0x0, &armSmlalInstruction,    0x00a00090, 0x0fa000f0, "SMLAL" },
  // MUL: Rd = Rm * Rn; Rd != PC. pass through
  { 0x0, &armMulInstruction,       0x00000090, 0x0fe000f0, "MUL Rd, Rm, Rn" },
  // MLA: Rd = Rm * Rn + Ra; Rd != PC. pass through
  { 0x0, &armMlaInstruction,       0x00200090, 0x0fe000f0, "MLA Rd, Rm, Rn, Ra" },
  // MLS: Rd = Rm * Rn - Ra; Rd != PC, pass through
  { 0x0, &armMlsInstruction,       0x00600090, 0x0ff000f0, "MLS Rd, Rm, Rn, Ra" },
  // Branch and try to exchange to ARM mode.
  { 0x1,  &armBxInstruction,     0x012FFF10, 0x0ffffff0, "bx%c\t%0-3r" },
  // Branch and link and try to exchange to Jazelle mode.
  { 0x1,  &armBxjInstruction,    0x012fff20, 0x0ffffff0, "BXJ Rm" },
  // Software breakpoint instruction... not sure yet.
  { 0x1,  &armBkptInstruction,      0xe1200070, 0xfff000f0, "BKPT #imm8" },
  // UNIMPLEMENTED: SMC, previously SMI: secure monitor call
  { 0x1,  &armSmcInstruction,       0x01600070, 0x0ff000f0, "smc%c\t%e" },
  // Branch and link and try to exchange with Thumb mode.
  { 0x1,  &armBlxRegisterInstruction,       0x012fff30, 0x0ffffff0, "BLX Rm" },
  // CLZ: Count leading zeroes - Rd, Rm != PC, pass through
  { 0x0, &armClzInstruction,       0x016f0f10, 0x0fff0ff0, "CLZ Rd, Rm" },
  // Q(D){ADD,SUB}: saturated add/subtract/doubleAdd/doubleSubtract, unpredictable when any R=PC
  { 0x0, &armQaddInstruction,      0x01000050, 0x0ff00ff0, "qadd%c\t%12-15r, %0-3r, %16-19r" },
  { 0x0, &armQdaddInstruction,     0x01400050, 0x0ff00ff0, "qdadd%c\t%12-15r, %0-3r, %16-19r" },
  { 0x0, &armQsubInstruction,      0x01200050, 0x0ff00ff0, "qsub%c\t%12-15r, %0-3r, %16-19r" },
  { 0x0, &armQdsubInstruction,     0x01600050, 0x0ff00ff0, "qdsub%c\t%12-15r, %0-3r, %16-19r" },
  // LDRD: Rt1 must be even numbered and NOT 14, thus Rt2 cannot be PC. pass.
  { 0x0, &armLdrdInstruction,   0x004000d0, 0x0e5000f0, "LDRD Rt, [Rn, #imm]" },
  { 0x0, &armLdrdInstruction,   0x000000d0, 0x0e500ff0, "LDRD Rt, [Rn, Rm]" },
  // STRD: pass through, let them fail!
  { 0x0, &armStrdInstruction,   0x004000f0, 0x0e5000f0, "STRD Rt, [Rn, #imm]" },
  { 0x0, &armStrdInstruction,   0x000000f0, 0x0e500ff0, "STRD Rt, [Rn, Rm]" },
  // signed 16 bit multiply, 32 bit accumulate, unpredictable if any R=PC
  { 0x0, &armSmlabbInstruction,    0x01000080, 0x0ff000f0, "smlabb%c\t%16-19r, %0-3r, %8-11r, %12-15r" },
  { 0x0, &armSmlatbInstruction,    0x010000a0, 0x0ff000f0, "smlatb%c\t%16-19r, %0-3r, %8-11r, %12-15r" },
  { 0x0, &armSmlabtInstruction,    0x010000c0, 0x0ff000f0, "smlabt%c\t%16-19r, %0-3r, %8-11r, %12-15r" },
  { 0x0, &armSmlattInstruction,    0x010000e0, 0x0ff000f0, "smlatt%c\t%16-19r, %0-3r, %8-11r, %12-15r" },
  // signed 16 bit x 32 bit multiply, 32 bit accumulate, unpredictable if any R=PC
  { 0x0, &armSmlawbInstruction,    0x01200080, 0x0ff000f0, "smlawb%c\t%16-19r, %0-3r, %8-11r, %12-15r" },
  { 0x0, &armSmlawtInstruction,    0x012000c0, 0x0ff000f0, "smlawt%c\t%16-19r, %0-3r, %8-11r, %12-15r" },
  // signed 16 bit multiply, 64 bit accumulate, unpredictable if any R=PC
  { 0x0, &armSmlalbbInstruction,   0x01400080, 0x0ff000f0, "smlalbb%c\t%12-15r, %16-19r, %0-3r, %8-11r" },
  { 0x0, &armSmlaltbInstruction,   0x014000a0, 0x0ff000f0, "smlaltb%c\t%12-15r, %16-19r, %0-3r, %8-11r" },
  { 0x0, &armSmlalbtInstruction,   0x014000c0, 0x0ff000f0, "smlalbt%c\t%12-15r, %16-19r, %0-3r, %8-11r" },
  { 0x0, &armSmlalttInstruction,   0x014000e0, 0x0ff000f0, "smlaltt%c\t%12-15r, %16-19r, %0-3r, %8-11r" },
  // signed 16 bit multiply, 32 bit result, any R=PC is unpredictable
  { 0x0, &armSmulbbInstruction,    0x01600080, 0x0ff0f0f0, "SMULBB Rd, Rn, RM" },
  { 0x0, &armSmultbInstruction,    0x016000a0, 0x0ff0f0f0, "smultb%c\t%16-19r, %0-3r, %8-11r" },
  { 0x0, &armSmulbtInstruction,    0x016000c0, 0x0ff0f0f0, "smulbt%c\t%16-19r, %0-3r, %8-11r" },
  { 0x0, &armSmulttInstruction,    0x016000e0, 0x0ff0f0f0, "smultt%c\t%16-19r, %0-3r, %8-11r" },
  // signed 16 bit x 32 bit multiply, 32 bit result, any R=PC is unpredictable
  { 0x0, &armSmulwbInstruction,    0x012000a0, 0x0ff0f0f0, "smulwb%c\t%16-19r, %0-3r, %8-11r" },
  { 0x0, &armSmulwtInstruction,    0x012000e0, 0x0ff0f0f0, "smulwt%c\t%16-19r, %0-3r, %8-11r" },
  // STRH: passthrough, will data abort if something wrong
  { 0x0, &armStrhInstruction,   0x004000b0, 0x0e5000f0, "STRH Rt, [Rn, +-imm8]" },
  { 0x0, &armStrhInstruction,   0x000000b0, 0x0e500ff0, "STRH Rt, [Rn], +-Rm" },
  // LDRH cant load halfword to PC, passthrough
  { 0x0, &armLdrhInstruction,   0x00500090, 0x0e500090, "LDRH Rt, [Rn, +-imm8]" },
  { 0x0, &armLdrhInstruction,   0x00100090, 0x0e500f90, "LDRH Rt, [Rn], +-Rm" },
  // AND: Rd = PC end block, others are fine
  { 0x1,  &armAndInstruction,       0x0000f000, 0x0fe0f010, "AND PC, Rn, Rm, #shamt" },
  { 0x1,  &armAndInstruction,       0x0000f010, 0x0fe0f090, "AND PC, Rn, Rm, Rshamt" },
  { 0x0, &armAndInstruction,       0x00000000, 0x0fe00010, "AND Rd, Rn, Rm, #shamt" },
  { 0x0, &armAndInstruction,       0x00000010, 0x0fe00090, "AND Rd, Rn, Rm, Rshamt" },
  // EOR: Rd = PC end block, others are fine
  { 0x1,  &armEorInstruction,       0x0020f000, 0x0fe0f010, "EOR PC, Rn, Rm, #shamt" },
  { 0x1,  &armEorInstruction,       0x0020f010, 0x0fe0f090, "EOR PC, Rn, Rm, Rshamt" },
  { 0x0, &armEorInstruction,       0x00200000, 0x0fe00010, "EOR Rd, Rn, Rm, #shamt" },
  { 0x0, &armEorInstruction,       0x00200010, 0x0fe00090, "EOR Rd, Rn, Rm, Rshamt" },
  // SUB: Rd = PC end block, others are fine
  { 0x1,  &armSubInstruction,       0x0040f000, 0x0fe0f010, "SUB PC, Rn, Rm, #shamt" },
  { 0x1,  &armSubInstruction,       0x0040f010, 0x0fe0f090, "SUB PC, Rn, Rm, Rshamt" },
  { 0x0, &armSubInstruction,       0x00400000, 0x0fe00010, "SUB Rd, Rn, Rm, #shamt" },
  { 0x0, &armSubInstruction,       0x00400010, 0x0fe00090, "SUB Rd, Rn, Rm, Rshamt" },
  // RSB: Rd = PC end block, others are fine
  { 0x1,  &armRsbInstruction,       0x0060f000, 0x0fe0f010, "RSB PC, Rn, Rm, #shamt" },
  { 0x1,  &armRsbInstruction,       0x0060f010, 0x0fe0f090, "RSB PC, Rn, Rm, Rshamt" },
  { 0x0, &armRsbInstruction,       0x00600000, 0x0fe00010, "RSB Rd, Rn, Rm, #shamt" },
  { 0x0, &armRsbInstruction,       0x00600010, 0x0fe00090, "RSB Rd, Rn, Rm, Rshamt" },
  // ADD: Rd = PC end block, others are fine
  { 0x1,  &armAddInstruction,       0x0080f000, 0x0fe0f010, "ADD PC, Rn, Rm, #shamt" },
  { 0x1,  &armAddInstruction,       0x0080f010, 0x0fe0f090, "ADD PC, Rn, Rm, Rshamt" },
  { 0x0, &armAddInstruction,       0x00800000, 0x0fe00010, "ADD Rd, Rn, Rm, #shamt" },
  { 0x0, &armAddInstruction,       0x00800010, 0x0fe00090, "ADD Rd, Rn, Rm, Rshamt" },
  // ADC: Rd = PC end block, others are fine
  { 0x1,  &armAdcInstruction,       0x00a0f000, 0x0fe0f010, "ADC PC, Rn, Rm, #shamt" },
  { 0x1,  &armAdcInstruction,       0x00a0f010, 0x0fe0f090, "ADC PC, Rn, Rm, Rshamt" },
  { 0x0, &armAdcInstruction,       0x00a00000, 0x0fe00010, "ADC Rd, Rn, Rm, #shamt" },
  { 0x0, &armAdcInstruction,       0x00a00010, 0x0fe00090, "ADC Rd, Rn, Rm, Rshamt" },
  // SBC: Rd = PC end block, others are fine
  { 0x1,  &armSbcInstruction,       0x00c0f000, 0x0fe0f010, "SBC Rd, Rn, Rm, #shamt" },
  { 0x1,  &armSbcInstruction,       0x00c0f010, 0x0fe0f090, "SBC Rd, Rn, Rm, Rshamt" },
  { 0x0, &armSbcInstruction,       0x00c00000, 0x0fe00010, "SBC Rd, Rn, Rm, #shamt" },
  { 0x0, &armSbcInstruction,       0x00c00010, 0x0fe00090, "SBC Rd, Rn, Rm, Rshamt" },
  // RSC: Rd = PC end block, others are fine
  { 0x1,  &armRscInstruction,       0x00e0f000, 0x0fe0f010, "RSC PC, Rn, Rm, #shamt" },
  { 0x1,  &armRscInstruction,       0x00e0f010, 0x0fe0f090, "RSC PC, Rn, Rm, Rshamt" },
  { 0x0, &armRscInstruction,       0x00e00000, 0x0fe00010, "RSC Rd, Rn, Rm, #shamt" },
  { 0x0, &armRscInstruction,       0x00e00010, 0x0fe00090, "RSC Rd, Rn, Rm, Rshamt" },
  // MSR/MRS: always hypercall! we must hide the real state from guest.
  { 0x1,  &armMsrInstruction,       0x0120f000, 0x0fb0fff0, "MSR, s/cpsr, Rn" },
  { 0x1,  &armMrsInstruction,       0x010f0000, 0x0fbf0fff, "MRS, Rn, s/cpsr" },
  // TST instructions are all fine
  { 0x0, &armTstInstruction,       0x01000000, 0x0fe00010, "TST Rn, Rm, #shamt" },
  { 0x0, &armTstInstruction,       0x01000010, 0x0fe00090, "TST Rn, Rm, Rshift" },
  // TEQ instructions are all fine
  { 0x0, &armTeqInstruction,       0x01200000, 0x0fe00010, "TEQ Rn, Rm, #shamt" },
  { 0x0, &armTeqInstruction,       0x01200010, 0x0fe00090, "TEQ Rn, Rm, Rshift" },
  // CMP instructions are all fine
  { 0x0, &armCmpInstruction,       0x01400000, 0x0fe00010, "CMP Rn, Rm, #shamt" },
  { 0x0, &armCmpInstruction,       0x01400010, 0x0fe00090, "CMP Rn, Rm, Rshamt" },
  // CMN instructions are all fine
  { 0x0, &armCmnInstruction,       0x01600000, 0x0fe00010, "CMN Rn, Rm, #shamt" },
  { 0x0, &armCmnInstruction,       0x01600010, 0x0fe00090, "CMN Rn, Rm, Rshamt" },
  // ORR: Rd = PC end block, other are fine
  { 0x1,  &armOrrInstruction,       0x0180f000, 0x0fe0f010, "ORR PC, Rn, Rm, #shamt" },
  { 0x1,  &armOrrInstruction,       0x0180f010, 0x0fe0f090, "ORR PC, Rn, Rm, Rshamt" },
  { 0x0, &armOrrInstruction,       0x01800000, 0x0fe00010, "ORR Rd, Rn, Rm, #shamt" },
  { 0x0, &armOrrInstruction,       0x01800010, 0x0fe00090, "ORR Rd, Rn, Rm, Rshamt" },
  // MOV with Rd = PC end block. MOV reg, <shifted reg> is a pseudo instr for shift instructions
  { 0x1,  &armMovInstruction,       0x01a0f000, 0x0deffff0, "MOV PC, Rm" },
  { 0x0, &armMovInstruction,       0x01a00000, 0x0def0ff0, "MOV Rn, Rm" },
  // LSL: Rd = PC and shamt = #imm, then end block. Otherwise fine
  { 0x1,  &armLslInstruction,       0x01a0f000, 0x0feff070, "LSL Rd, Rm, #shamt" },
  { 0x1,  &armLslInstruction,       0x01a0f010, 0x0feff0f0, "LSL Rd, Rm, Rshamt" },
  { 0x0, &armLslInstruction,       0x01a00000, 0x0fef0070, "LSL Rd, Rm, #shamt" },
  { 0x0, &armLslInstruction,       0x01a00010, 0x0fef00f0, "LSL Rd, Rm, Rshamt" },
  // LSR: Rd = PC and shamt = #imm, then end block. Otherwise fine
  { 0x1,  &armLsrInstruction,       0x01a0f020, 0x0feff070, "LSR Rd, Rm, #shamt" },
  { 0x1,  &armLsrInstruction,       0x01a0f030, 0x0feff0f0, "LSR Rd, Rm, Rshamt" },
  { 0x0, &armLsrInstruction,       0x01a00020, 0x0fef0070, "LSR Rd, Rm, #shamt" },
  { 0x0, &armLsrInstruction,       0x01a00030, 0x0fef00f0, "LSR Rd, Rm, Rshamt" },
  // ASR: Rd = PC and shamt = #imm, then end block. Otherwise fine
  { 0x1,  &armAsrInstruction,       0x01a0f040, 0x0feff070, "ASR Rd, Rm, #shamt" },
  { 0x1,  &armAsrInstruction,       0x01a0f050, 0x0feff0f0, "ASR Rd, Rm, Rshamt" },
  { 0x0, &armAsrInstruction,       0x01a00040, 0x0fef0070, "ASR Rd, Rm, #shamt" },
  { 0x0, &armAsrInstruction,       0x01a00050, 0x0fef00f0, "ASR Rd, Rm, Rshamt" },
  // RRX: shift right and extend, Rd can be PC
  { 0x1,  &armRrxInstruction,       0x01a0f060, 0x0feffff0, "RRX PC, Rm" },
  { 0x0, &armRrxInstruction,       0x01a00060, 0x0fef0ff0, "RRX Rd, Rm" },
  // ROR: reg case destination unpredictable. imm case dest can be PC.
  { 0x0, &armRorInstruction,       0x01a00070, 0x0fef00f0, "ROR Rd, Rm, Rn" },
  { 0x1,  &armRorInstruction,       0x01a0f060, 0x0feff070, "ROR PC, Rm, #imm" },
  { 0x0, &armRorInstruction,       0x01a00060, 0x0fef0070, "ROR Rd, Rm, #imm" },
  // BIC with Rd = PC end block, other are fine.
  { 0x1,  &armBicInstruction,       0x01c0f000, 0x0fe0f010, "BIC PC, Rn, Rm, #shamt" },
  { 0x1,  &armBicInstruction,       0x01c0f010, 0x0fe0f090, "BIC PC, Rn, Rm, Rshamt" },
  { 0x0, &armBicInstruction,       0x01c00000, 0x0fe00010, "BIC Rd, Rn, Rm, #shamt" },
  { 0x0, &armBicInstruction,       0x01c00010, 0x0fe00090, "BIC Rd, Rn, Rm, Rshamt" },
  // MVN with Rd = PC end block, other are fine.
  { 0x1,  &armMvnInstruction,       0x01e0f000, 0x0fe0f010, "MVN Rd, Rm, #shamt" },
  { 0x1,  &armMvnInstruction,       0x01e0f010, 0x0fe0f090, "MVN Rd, Rm, Rshamt" },
  { 0x0, &armMvnInstruction,       0x01e00000, 0x0fe00010, "MVN Rd, Rm, #shamt" },
  { 0x0, &armMvnInstruction,       0x01e00010, 0x0fe00090, "MVN Rd, Rm, Rshamt" },

  { 0x1,  &undefinedInstruction, 0x00000000, 0x00000000, "dataProcMiscInstructions_op0" }
};

// immediate cases and random hints
static struct instruction32bit armDataProcMiscInstructions_op1[] =
{
  // UNIMPLEMENTED: yield hint
  { 0x1,  &armYieldInstruction,     0x0320f001, 0x0fffffff, "yield%c" },
  // UNIMPLEMENTED: wait for event hint
  { 0x1,  &armWfeInstruction,       0x0320f002, 0x0fffffff, "wfe%c" },
  // UNIMPLEMENTED: wait for interrupt hint
  { 0x1,  &armWfiInstruction,       0x0320f003, 0x0fffffff, "wfi%c" },
  // UNIMPLEMENTED: send event hint
  { 0x1,  &armSevInstruction,       0x0320f004, 0x0fffffff, "sev%c" },
  { 0x0, &nopInstruction,       0x0320f000, 0x0fffff00, "NOP" },
  // UNIMPLEMENTED: debug hint
  { 0x1,  &armDbgInstruction,       0x0320f0f0, 0x0ffffff0, "dbg%c\t#%0-3d" },
  // MOVW - indication of 'wide' - to select ARM encoding. Rd cant be PC, pass through.
  { 0x0, &armMovwInstruction,      0x03000000, 0x0ff00000, "MOVW Rd, Rn" },
  { 0x0, &armMovtInstruction,      0x03400000, 0x0ff00000, "MOVT Rd, Rn" },
  // AND: Rd = PC end block, others are fine
  { 0x1,  &armAndInstruction,       0x0200f000, 0x0fe0f000, "AND PC, Rn, #imm" },
  { 0x0, &armAndInstruction,       0x02000000, 0x0fe00000, "AND Rd, Rn, #imm" },
  // EOR: Rd = PC end block, others are fine
  { 0x1,  &armEorInstruction,       0x0220f000, 0x0fe0f000, "EOR PC, Rn, Rm/#imm" },
  { 0x0, &armEorInstruction,       0x02200000, 0x0fe00000, "EOR Rd, Rn, #imm" },
  // SUB: Rd = PC end block, others are fine
  { 0x1,  &armSubInstruction,       0x0240f000, 0x0fe0f000, "SUB PC, Rn, Rm/imm" },
  { 0x0, &armSubInstruction,       0x02400000, 0x0fe00000, "SUB Rd, Rn, #imm" },
  // RSB: Rd = PC end block, others are fine
  { 0x1,  &armRsbInstruction,       0x0260f000, 0x0fe0f000, "RSB PC, Rn, Rm/imm" },
  { 0x0, &armRsbInstruction,       0x02600000, 0x0fe00000, "RSB Rd, Rn, #imm" },
  // ADD: Rd = PC end block, others are fine
  { 0x1,  &armAddInstruction,       0x0280f000, 0x0fe0f000, "ADD PC, Rn, #imm" },
  { 0x0, &armAddInstruction,       0x02800000, 0x0fe00000, "ADD Rd, Rn, #imm" },
  // ADC: Rd = PC end block, others are fine
  { 0x1,  &armAdcInstruction,       0x02a0f000, 0x0fe0f000, "ADC PC, Rn/#imm" },
  { 0x0, &armAdcInstruction,       0x02a00000, 0x0fe00000, "ADC Rd, Rn, #imm" },
  // SBC: Rd = PC end block, others are fine
  { 0x1,  &armSbcInstruction,       0x02c0f000, 0x0fe0f000, "SBC PC, Rn/#imm" },
  { 0x0, &armSbcInstruction,       0x02c00000, 0x0fe00000, "SBC Rd, Rn, #imm" },
  // RSC: Rd = PC end block, others are fine
  { 0x1,  &armRscInstruction,       0x02e0f000, 0x0fe0f000, "RSC PC, Rn/#imm" },
  { 0x0, &armRscInstruction,       0x02e00000, 0x0fe00000, "RSC Rd, Rn, #imm" },
  // MSR: always hypercall! we must hide the real state from guest.
  { 0x1,  &armMsrInstruction,       0x0320f000, 0x0fb0f000, "MSR, s/cpsr, #imm" },
  // TST instructions are all fine
  { 0x0, &armTstInstruction,       0x03000000, 0x0fe00000, "TST Rn, #imm" },
  // TEQ instructions are all fine
  { 0x0, &armTeqInstruction,       0x03200000, 0x0fe00000, "TEQ Rn, #imm" },
  // CMP instructions are all fine
  { 0x0, &armCmpInstruction,       0x03400000, 0x0fe00000, "CMP Rn, #imm" },
  // CMN instructions are all fine
  { 0x0, &armCmnInstruction,       0x03600000, 0x0fe00000, "CMN Rn, #imm" },
  // ORR: Rd = PC end block, other are fine
  { 0x1,  &armOrrInstruction,       0x0380f000, 0x0fe0f000, "ORR Rd, Rn, #imm" },
  { 0x0, &armOrrInstruction,       0x03800000, 0x0fe00000, "ORR Rd, Rn, #imm" },
  // MOV with Rd = PC end block. MOV <shifted reg> is a pseudo instr..
  { 0x1,  &armMovInstruction,       0x03a0f000, 0x0feff000, "MOV PC, #imm" },
  { 0x0, &armMovInstruction,       0x03a00000, 0x0fef0000, "MOV Rn, #imm" },
  // BIC with Rd = PC end block, other are fine.
  { 0x1,  &armBicInstruction,       0x03c0f000, 0x0fe0f000, "BIC PC, Rn, #imm" },
  { 0x0, &armBicInstruction,       0x03c00000, 0x0fe00000, "BIC Rd, Rn, #imm" },
  // MVN with Rd = PC end block, other are fine.
  { 0x1,  &armMvnInstruction,       0x03e0f000, 0x0fe0f000, "MVN PC, Rn, #imm" },
  { 0x0, &armMvnInstruction,       0x03e00000, 0x0fe00000, "MVN Rd, Rn, #imm" },

  { 0x1,  &undefinedInstruction, 0x00000000, 0x00000000, "dataProcMiscInstructions_op1" }
};

static struct instruction32bit armLoadStoreWordByteInstructions[] =
{
  // STRT - all trap
  { 0x2, &armStrtInstruction,   0x04200000, 0x0f700000, "STRT Rt, [Rn], +-imm12" },
  { 0x2, &armStrtInstruction,   0x06200000, 0x0f700010, "STRT Rt, [Rn], +-Rm" },
  // LDRT - all trap
  { 0x2, &armLdrtInstruction,   0x04300000, 0x0f700000, "LDRT Rd, [Rn], +-imm12" },
  { 0x2, &armLdrtInstruction,   0x06300000, 0x0f700010, "LDRT Rd, [Rn], +-Rm" },
  // STRBT - all trap
  { 0x2, &armStrbtInstruction,  0x04600000, 0x0f700000, "STRBT Rt, [Rn, +-imm12]" },
  { 0x2, &armStrbtInstruction,  0x06600000, 0x0f700010, "STRBT Rt, [Rn], +-Rm" },
  // LDRBT - all trap
  { 0x2, &armLdrbtInstruction,  0x04700000, 0x0f700000, "LDRBT Rd, [Rn], +-imm12" },
  { 0x2, &armLdrbtInstruction,  0x06700000, 0x0f700010, "LDRBT Rd, [Rn], +-Rm" },
  // STR - all pass-through
  { 0x0, &armStrInstruction,    0x04000000, 0x0e500000, "STR Rt, [Rn, +-imm12]" },
  { 0x0, &armStrInstruction,    0x06000000, 0x0e500010, "STR Rt, [Rn], +-Rm" },
  // LDR traps if dest = PC, otherwise pass through
  { 0x1, &armLdrInstruction,    0x0410f000, 0x0e50f000, "LDR PC, [Rn], +-imm12" },
  { 0x0, &armLdrInstruction,    0x04100000, 0x0e500000, "LDR Rd, [Rn], +-imm12" },
  { 0x1, &armLdrInstruction,    0x0610f000, 0x0e50f010, "LDR PC, [Rn], +-Rm" },
  { 0x0, &armLdrInstruction,    0x06100000, 0x0e500010, "LDR Rd, [Rn], +-Rm" },
  // STRB pass through
  { 0x0, &armStrbInstruction,   0x04400000, 0x0e500000, "STRB Rt, [Rn, +-imm12]" },
  { 0x0, &armStrbInstruction,   0x06400000, 0x0e500010, "STRB Rt, [Rn], +-Rm" },
  // LDRB - pass through, dest can't be PC
  { 0x0, &armLdrbInstruction,   0x04500000, 0x0e500000, "LDRB Rd, [Rn], +-imm12" },
  { 0x0, &armLdrbInstruction,   0x06500000, 0x0e500010, "LDRB Rd, [Rn], +-Rm" },

  { 0x1,  &undefinedInstruction, 0x00000000, 0x00000000, "loadStoreWordByteInstructions" }
};

static struct instruction32bit armMediaInstructions[] =
{
  // BFC: bit field clear, dest PC not allowed.
  { 0x0, &armBfcInstruction,       0x07c0001f, 0x0fe0007f, "BFC Rd, #LSB, #width" },
  // BFI: bit field insert, dest PC not allowed.
  { 0x0, &armBfiInstruction,       0x07c00010, 0x0fe00070, "BFI Rd, #LSB, #width" },
  // RBIT: reverse bits, if Rd or Rm = 15 then unpredictable.
  { 0x0, &armRbitInstruction,      0x06ff0f30, 0x0fff0ff0, "RBIT Rd,Rm" },
  // UBFX: extract bit field - destination 15 unpredictable
  { 0x0, &armUbfxInstruction,     0x07a00050, 0x0fa00070, "UBFX Rd, Rn, width" },
  // PKH (pack halfword), if Rd or Rn or Rm = 15 then unpredictable
  { 0x0, &armPkhbtInstruction,     0x06800010, 0x0ff00ff0, "PKHBT Rd,Rn,Rm" },
  { 0x0, &armPkhbtInstruction,     0x06800010, 0x0ff00070, "PKHBT Rd,Rn,Rm,LSL #imm" },
  { 0x0, &armPkhtbInstruction,     0x06800050, 0x0ff00ff0, "PKHTB Rd,Rn,Rm,ASR #32" },
  { 0x0, &armPkhtbInstruction,     0x06800050, 0x0ff00070, "PKHTB Rd,Rn,Rm,ASR #imm" },
  // QADD8,QADD16: saturating add 16 bit and 8 bit, if Rd or Rn or Rm = 15 then unpredictable
  { 0x0, &armQadd16Instruction,    0x06200f10, 0x0ff00ff0, "QADD16 Rd,Rn,Rm" },
  { 0x0, &armQadd8Instruction,     0x06200f90, 0x0ff00ff0, "QADD8 Rd,Rn,Rm" },
  // QASX: saturating add and subtract with exchange if Rd or Rn or Rm = 15 then unpredictable
  { 0x0, &armQasxInstruction,  0x06200f30, 0x0ff00ff0, "QASX Rd,Rn,Rm" },
  // QSUB8,QSUB16: saturating subtract 16 and 8 bit if Rd or Rn or Rm = 15 then unpredictable
  { 0x0, &armQsub16Instruction,    0x06200f70, 0x0ff00ff0, "QSUB16 Rd,Rn,Rm" },
  { 0x0, &armQsub8Instruction,     0x06200ff0, 0x0ff00ff0, "QSUB8 Rd,Rn,Rm" },
  // QSAX: saturating subtract and add with exchange if Rd or Rn or Rm = 15 then unpredictable
  { 0x0, &armQsaxInstruction,  0x06200f50, 0x0ff00ff0, "QSAX Rd,Rn,Rm" },
  // SADD8,SADD16: signed add 16 bit and 8 bit, if Rd or Rn or Rm = 15 then unpredictable
  { 0x0, &armSadd16Instruction,    0x06100f10, 0x0ff00ff0, "SADD16 Rd,Rn,Rm" },
  { 0x0, &armSadd8Instruction,     0x06100f90, 0x0ff00ff0, "SADD8 Rd,Rn,Rm" },
  // SASX: signed add and subtract with exchange, if Rd or Rn or Rm = 15 then unpredictable
  { 0x0, &armSasxInstruction,  0x06100f30, 0x0ff00ff0, "SASX Rd,Rn,Rm" },
  // SSUB8,SSUB16: signed subtract 16 bit and 8 bit, if Rd or Rn or Rm = 15 then unpredictable
  { 0x0, &armSsub16Instruction,    0x06100f70, 0x0ff00ff0, "SSUB16 Rd,Rn,Rm" },
  { 0x0, &armSsub8Instruction,     0x06100ff0, 0x0ff00ff0, "SSUB8 Rd,Rn,Rm" },
  // SSAX: signed subtract and add with exchange,if Rd or Rn or Rm = 15 then unpredictable
  { 0x0, &armSsaxInstruction,  0x06100f50, 0x0ff00ff0, "SSAX Rd,Rn,Rm" },
  // SHADD8,SHADD16: signed halvign add 16 bit and 8 bit, if Rd or Rn or Rm = 15 then unpredictable
  { 0x0, &armShadd16Instruction,   0x06300f10, 0x0ff00ff0, "SHADD16 Rd,Rn,Rm" },
  { 0x0, &armShadd8Instruction,    0x06300f90, 0x0ff00ff0, "SHADD8 Rd,Rn,Rm" },
  // SHASX: signed halving add and subtract with exchange, if Rd or Rn or Rm = 15 then unpredictable
  { 0x0, &armShasxInstruction, 0x06300f30, 0x0ff00ff0, "SHASX Rd,Rn,Rm" },
  // SHSUB8,SHSUB16: signed halvign subtract 16 bit and 8 bit, if Rd or Rn or Rm = 15 then unpredictable
  { 0x0, &armShsub16Instruction,   0x06300f70, 0x0ff00ff0, "SHSUB16 Rd,Rn,Rm" },
  { 0x0, &armShsub8Instruction,    0x06300ff0, 0x0ff00ff0, "SHSUB8 Rd,Rn,Rm" },
  // SHSAX: signed halving subtract and add with exchange, if Rd or Rn or Rm = 15 then unpredictable
  { 0x0, &armShsaxInstruction, 0x06300f50, 0x0ff00ff0, "SHSAX Rd,Rn,Rm" },
  // UADD8,UADD16: unsigned add 16 bit and 8 bit, if Rd or Rn or Rm = 15 then unpredictable
  { 0x0, &armUadd16Instruction,    0x06500f10, 0x0ff00ff0, "UADD16 Rd,Rn,Rm" },
  { 0x0, &armUadd8Instruction,     0x06500f90, 0x0ff00ff0, "UADD8 Rd,Rn,Rm" },
  // UASX: unsigned add and subtract with exchange, if Rd or Rn or Rm = 15 then unpredictable
  { 0x0, &armUasxInstruction,  0x06500f30, 0x0ff00ff0, "UASX Rd,Rn,Rm" },
  // USUB8,USUB16: unsigned subtract 16 bit and 8 bit, if Rd or Rn or Rm = 15 then unpredictable
  { 0x0, &armUsub16Instruction,    0x06500f70, 0x0ff00ff0, "USUB16 Rd,Rn,Rm" },
  { 0x0, &armUsub8Instruction,     0x06500ff0, 0x0ff00ff0, "USUB8 Rd,Rn,Rm" },
  // USAX: unsigned subtract and add with exchange, if Rd or Rn or Rm = 15 then unpredictable
  { 0x0, &armUsaxInstruction,  0x06500f50, 0x0ff00ff0, "USAX Rd,Rn,Rm" },
  // UHADD8,UHADD16: unsigned halving add 16 bit and 8 bit, if Rd or Rn or Rm = 15 then unpredictable
  { 0x0, &armUhadd16Instruction,   0x06700f10, 0x0ff00ff0, "UHADD16 Rd,Rn,Rm" },
  { 0x0, &armUhadd8Instruction,    0x06700f90, 0x0ff00ff0, "UHADD8 Rd,Rn,Rm" },
  // UHASX: unsigned halving add and subtract with exchange, if Rd or Rn or Rm = 15 then unpredictable
  { 0x0, &armUhasxInstruction, 0x06700f30, 0x0ff00ff0, "UHASX Rd,Rn,Rm" },
  // UHSUB8,UHSUB16: unsigned halving subtract 16 bit and 8 bit, if Rd or Rn or Rm = 15 then unpredictable
  { 0x0, &armUhsub16Instruction,   0x06700f70, 0x0ff00ff0, "UHSUB16 Rd,Rn,Rm" },
  { 0x0, &armUhsub8Instruction,    0x06700ff0, 0x0ff00ff0, "UHSUB8 Rd,Rn,Rm" },
  // UHSAX: unsigned halving subtract and add with exchange, if Rd or Rn or Rm = 15 then unpredictable
  { 0x0, &armUhsaxInstruction, 0x06700f50, 0x0ff00ff0, "UHSAX Rd,Rn,Rm" },
  // UQADD8,UQADD16:  unsigned saturating add 16 bit and 8 bit, if Rd or Rn or Rm = 15 then unpredictable
  { 0x0, &armUqadd16Instruction,   0x06600f10, 0x0ff00ff0, "UQADD16 Rd,Rn,Rm" },
  { 0x0, &armUqadd8Instruction,    0x06600f90, 0x0ff00ff0, "UQADD8 Rd,Rn,Rm" },
  // UQASX: unsigned saturating add and subtract with exchange, if Rd or Rn or Rm = 15 then unpredictable
  { 0x0, &armUqasxInstruction, 0x06600f30, 0x0ff00ff0, "UQASX Rd,Rn,Rm" },
  // UQSUB8,UQSUB16: unsigned saturating subtract 16 bit and 8 bit, if Rd or Rn or Rm = 15 then unpredictable
  { 0x0, &armUqsub16Instruction,   0x06600f70, 0x0ff00ff0, "UQSUB16 Rd,Rn,Rm" },
  { 0x0, &armUqsub8Instruction,    0x06600ff0, 0x0ff00ff0, "UQSUB8 Rd,Rn,Rm" },
  // UQSAX: unsigned saturating subtract and add with exchange, if Rd or Rn or Rm = 15 then unpredictable
  { 0x0, &armUqsaxInstruction, 0x06600f50, 0x0ff00ff0, "UQSAX Rd,Rn,Rm" },
  // REV (byte-reverse word), if Rd or Rm = 15 then unpredictable.
  { 0x0, &armRevInstruction,       0x06bf0f30, 0x0fff0ff0, "REV Rd,Rm" },
  // REV16 (byte-reverse packed halfword), if Rd or Rm = 15 then unpredictable.
  { 0x0, &armRev16Instruction,     0x06bf0fb0, 0x0fff0ff0, "REV16 Rd,Rm" },
  // REVSH (byte-reverse signed halfword), if Rd or Rm = 15 then unpredictable.
  { 0x0, &armRevshInstruction,     0x06ff0fb0, 0x0fff0ff0, "REVSH Rd,Rm" },
  // SXTH (sign-extend halfword), if Rd or Rm = 15 then unpredictable.
  { 0x0, &armSxthInstruction,      0x06bf0070, 0x0fff0ff0, "SXTH Rd,Rm" },
  { 0x0, &armSxthInstruction,      0x06bf0470, 0x0fff0ff0, "SXTH Rd,Rm,ROR #8" },
  { 0x0, &armSxthInstruction,      0x06bf0870, 0x0fff0ff0, "SXTH Rd,Rm,ROR #16" },
  { 0x0, &armSxthInstruction,      0x06bf0c70, 0x0fff0ff0, "SXTH Rd,Rm,ROR #24" },
  // SXTB16: sign-extend byte 16, if Rd or Rm = 15 then unpredictable.
  { 0x0, &armSxtb16Instruction,    0x068f0070, 0x0fff0ff0, "SXTB16 Rd,Rm" },
  { 0x0, &armSxtb16Instruction,    0x068f0470, 0x0fff0ff0, "SXTB16 Rd,Rm,ROR #8" },
  { 0x0, &armSxtb16Instruction,    0x068f0870, 0x0fff0ff0, "SXTB16 Rd,Rm,ROR #16" },
  { 0x0, &armSxtb16Instruction,    0x068f0c70, 0x0fff0ff0, "SXTB16 Rd,Rm,ROR #24" },
  // SXTB sign extend byte, if Rd or Rn = 15 then unpredictable.
  { 0x0, &armSxtbInstruction,      0x06af0070, 0x0fff0ff0, "SXTB Rd,Rm" },
  { 0x0, &armSxtbInstruction,      0x06af0470, 0x0fff0ff0, "SXTB Rd,Rm,ROR #8" },
  { 0x0, &armSxtbInstruction,      0x06af0870, 0x0fff0ff0, "SXTB Rd,Rm,ROR #16" },
  { 0x0, &armSxtbInstruction,      0x06af0c70, 0x0fff0ff0, "SXTB Rd,Rm,ROR #24" },
  // UXTH permitted, if Rd or Rn = 15 then unpredictable.
  { 0x0, &armUxthInstruction,      0x06ff0070, 0x0fff0ff0, "UXTH Rd,Rm" },
  { 0x0, &armUxthInstruction,      0x06ff0470, 0x0fff0ff0, "UXTH Rd,Rm,ROR #8" },
  { 0x0, &armUxthInstruction,      0x06ff0870, 0x0fff0ff0, "UXTH Rd,Rm,ROR #16" },
  { 0x0, &armUxthInstruction,      0x06ff0c70, 0x0fff0ff0, "UXTH Rd,Rm,ROR #24" },
  // UXTB16 permitted, if Rd or Rn = 15 then unpredictable.
  { 0x0, &armUxtb16Instruction,    0x06cf0070, 0x0fff0ff0, "UXTB16 Rd,Rm" },
  { 0x0, &armUxtb16Instruction,    0x06cf0470, 0x0fff0ff0, "UXTB16 Rd,Rm,ROR #8" },
  { 0x0, &armUxtb16Instruction,    0x06cf0870, 0x0fff0ff0, "UXTB16 Rd,Rm,ROR #16" },
  { 0x0, &armUxtb16Instruction,    0x06cf0c70, 0x0fff0ff0, "UXTB16 Rd,Rm,ROR #24" },
  // UXTB permitted, if Rd or Rn = 15 then unpredictable.
  { 0x0, &armUxtbInstruction,      0x06ef0070, 0x0fff0ff0, "UXTB Rd,Rm" },
  { 0x0, &armUxtbInstruction,      0x06ef0470, 0x0fff0ff0, "UXTB Rd,Rm,ROR #8" },
  { 0x0, &armUxtbInstruction,      0x06ef0870, 0x0fff0ff0, "UXTB Rd,Rm,ROR #16" },
  { 0x0, &armUxtbInstruction,      0x06ef0c70, 0x0fff0ff0, "UXTB Rd,Rm,ROR #24" },
  // SXTAH (sign-extend and add halfword), if Rd or Rm = 15 then unpredictable, if Rn = 15 see SXTH.
  { 0x0, &armSxtahInstruction,     0x06b00070, 0x0ff00ff0, "SXTAH Rd,Rn,Rm" },
  { 0x0, &armSxtahInstruction,     0x06b00470, 0x0ff00ff0, "SXTAH Rd,Rn,Rm,ROR #8" },
  { 0x0, &armSxtahInstruction,     0x06b00870, 0x0ff00ff0, "SXTAH Rd,Rn,Rm,ROR #16" },
  { 0x0, &armSxtahInstruction,     0x06b00c70, 0x0ff00ff0, "SXTAH Rd,Rn,Rm,ROR #24" },
  // SXTAB16 (sign-extend and add byte 16), if Rd or Rm = 15 then unpredictable, if Rn = 15 see SXTB16.
  { 0x0, &armSxtab16Instruction,   0x06800070, 0x0ff00ff0, "SXTAB16 Rd,Rn,Rm" },
  { 0x0, &armSxtab16Instruction,   0x06800470, 0x0ff00ff0, "SXTAB16 Rd,Rn,Rm,ROR #8" },
  { 0x0, &armSxtab16Instruction,   0x06800870, 0x0ff00ff0, "SXTAB16 Rd,Rn,Rm,ROR #16" },
  { 0x0, &armSxtab16Instruction,   0x06800c70, 0x0ff00ff0, "SXTAB16 Rd,Rn,Rm,ROR #24" },
  // SXTAB (sign-extend and add byte), if Rd or Rm = 15 then unpredictable, if Rn = 15 see SXTB.
  { 0x0, &armSxtabInstruction,     0x06a00070, 0x0ff00ff0, "SXTAB Rd,Rn,Rm" },
  { 0x0, &armSxtabInstruction,     0x06a00470, 0x0ff00ff0, "SXTAB Rd,Rn,Rm,ROR #8" },
  { 0x0, &armSxtabInstruction,     0x06a00870, 0x0ff00ff0, "SXTAB Rd,Rn,Rm,ROR #16" },
  { 0x0, &armSxtabInstruction,     0x06a00c70, 0x0ff00ff0, "SXTAB Rd,Rn,Rm,ROR #24" },
  // UXTAH (unsigned extend and add halfword), if Rd or Rm = 15 then unpredictable, if Rn = 15 see UXTH.
  { 0x0, &armUxtahInstruction,     0x06f00070, 0x0ff00ff0, "UXTAH Rd,Rn,Rm" },
  { 0x0, &armUxtahInstruction,     0x06f00470, 0x0ff00ff0, "UXTAH Rd,Rn,Rm,ROR #8" },
  { 0x0, &armUxtahInstruction,     0x06f00870, 0x0ff00ff0, "UXTAH Rd,Rn,Rm,ROR #16" },
  { 0x0, &armUxtahInstruction,     0x06f00c70, 0x0ff00ff0, "UXTAH Rd,Rn,Rm,ROR #24" },
  // UXTAB16 (unsigned extend and add byte 16), if Rd or Rm = 15 then unpredictable, if Rn = 15 see UXTB16.
  { 0x0, &armUxtab16Instruction,   0x06c00070, 0x0ff00ff0, "UXTAB16 Rd,Rn,Rm" },
  { 0x0, &armUxtab16Instruction,   0x06c00470, 0x0ff00ff0, "UXTAB16 Rd,Rn,Rm,ROR #8" },
  { 0x0, &armUxtab16Instruction,   0x06c00870, 0x0ff00ff0, "UXTAB16 Rd,Rn,Rm,ROR #16" },
  { 0x0, &armUxtab16Instruction,   0x06c00c70, 0x0ff00ff0, "UXTAB16 Rd,Rn,Rm,ROR #24" },
  // UXTAB (unsigned extend and add byte), if Rd or Rm = 15 then unpredictable, if Rn = 15 see UXTB.
  { 0x0, &armUxtabInstruction,     0x06e00070, 0x0ff00ff0, "UXTAB Rd,Rn,Rm" },
  { 0x0, &armUxtabInstruction,     0x06e00470, 0x0ff00ff0, "UXTAB Rd,Rn,Rm,ROR #8" },
  { 0x0, &armUxtabInstruction,     0x06e00870, 0x0ff00ff0, "UXTAB Rd,Rn,Rm,ROR #16" },
  { 0x0, &armUxtabInstruction,     0x06e00c70, 0x0ff00ff0, "UXTAB Rd,Rn,Rm,ROR #24" },
  // SEL (select bytes), if Rd or Rm or Rn = 15 then unpredictable.
  { 0x0, &armSelInstruction,       0x06800fb0, 0x0ff00ff0, "SEL Rd,Rn,Rm" },
  // SMUAD (signed dual multiply add), if Rd or Rm or Rn = 15 then unpredictable.
  { 0x0, &armSmuadInstruction,     0x0700f010, 0x0ff0f0d0, "SMUAD{X} Rd,Rn,Rm" },
  // SMUSD: signed dual multiply subtract, if Rd or Rn or Rm = 15 then unpredictable
  { 0x0, &armSmusdInstruction,     0x0700f050, 0x0ff0f0d0, "SMUSD{X} Rd,Rn,Rm" },
  // SMLAD: signed multiply accumulate dual, if Rd or Rn or Rm = 15 then unpredictable, Ra = 15 see SMUAD
  { 0x0, &armSmladInstruction,     0x07000010, 0x0ff000d0, "SMLAD{X} Rd,Rn,Rm,Ra" },
  // SMLALD: signed multiply accumulate long dual, if any R = 15 then unpredictable
  { 0x0, &armSmlaldInstruction,    0x07400010, 0x0ff000d0, "SMLALD{X} RdLo,RdHi,Rn,Rm" },
  // SMLSD: signed multiply subtract dual, if Rd or Rn or Rm = 15 then unpredictable, Ra = 15 see SMUSD
  { 0x0, &armSmlsdInstruction,     0x07000050, 0x0ff000d0, "SMLSD{X} Rd,Rn,Rm,Ra" },
  // SMLSLD: signed multiply subtract long dual, if any R = 15 then unpredictable
  { 0x0, &armSmlsldInstruction,    0x07400050, 0x0ff000d0, "SMLSLD{X} RdLo,RdHi,Rn,Rm" },
  // SMMUL: signed most significant word multiply, if Rd or Rn or Rm = 15 then unpredictable
  { 0x0, &armSmmulInstruction,     0x0750f010, 0x0ff0f0d0, "SMMUL{R} Rd,Rn,Rm" },
  // SMMLA: signed most significant word multiply accumulate, if Rd or Rn or Rm = 15 then unpredictable, Ra = 15 see SMMUL
  { 0x0, &armSmmlaInstruction,     0x07500010, 0x0ff000d0, "SMMLA{R} Rd,Rn,Rm,Ra" },
  // SMMLS: signed most significant word multiply subtract, if any R = 15 then unpredictable
  { 0x0, &armSmmlsInstruction,     0x075000d0, 0x0ff000d0, "SMMLS{R} Rd,Rn,Rm,Ra" },
  // SSAT,SSAT16,USAT,USAT16: (un)signed saturate, if Rd or Rn = 15 then unpredictable
  { 0x0, &armSsatInstruction,      0x06a00010, 0x0fe00ff0, "SSAT Rd,#sat_imm,Rn" },
  { 0x0, &armSsatInstruction,      0x06a00010, 0x0fe00070, "SSAT Rd,#sat_imm,Rn,LSL #imm" },
  { 0x0, &armSsatInstruction,      0x06a00050, 0x0fe00070, "SSAT Rd,#sat_imm,Rn,ASR #imm" },
  { 0x0, &armSsat16Instruction,    0x06a00f30, 0x0ff00ff0, "SSAT16 Rd,#imm,Rn" },
  { 0x0, &armUsatInstruction,      0x06e00010, 0x0fe00ff0, "USAT Rd,#sat_imm,Rn" },
  { 0x0, &armUsatInstruction,      0x06e00010, 0x0fe00070, "USAT Rd,#sat_imm,Rn,LSL #imm" },
  { 0x0, &armUsatInstruction,      0x06e00050, 0x0fe00070, "USAT Rd,#sat_imm,Rn,ASR #imm" },
  { 0x0, &armUsat16Instruction,    0x06e00f30, 0x0ff00ff0, "USAT16 Rd,#imm,Rn" },

  { 0x1,  &undefinedInstruction, 0x00000000, 0x00000000, "mediaInstructions" }
};

static struct instruction32bit armSvcCoprocInstructions[] =
{
  // well obviously.
  { 0x1,  &svcInstruction,       0x0f000000, 0x0f000000, "SWI code" },
  // Generic coprocessor instructions.
  { 0x1,  &armMrcInstruction,    0x0e100010, 0x0f100010, "MRC" },
  { 0x1,  &armMcrInstruction,    0x0e000010, 0x0f100010, "MCR" },
  { 0x1,  &undefinedInstruction, 0x00000000, 0x00000000, "svcCoprocInstructions" }
};

static struct instruction32bit armUnconditionalInstructions[] =
{
  // UNIMPLEMENTED: data memory barrier
  { 0x1,  &armDmbInstruction,       0xf57ff050, 0xfffffff0, "dmb\t%U" },
  // sync barriers
  { 0x1,  &armDsbInstruction,       0xf57ff040, 0xfffffff0, "DSB" },
  { 0x1,  &armIsbInstruction,       0xf57ff060, 0xfffffff0, "ISB" },
  // UNIMPLEMENTED: CLREX clear exclusive
  { 0x1,  &armClrexInstruction,     0xf57ff01f, 0xffffffff, "clrex" },
  // CPS: change processor state
  { 0x1,  &armCpsInstruction,       0xf1080000, 0xfffffe3f, "CPSIE" },
  { 0x1,  &armCpsInstruction,       0xf10a0000, 0xfffffe20, "CPSIE" },
  { 0x1,  &armCpsInstruction,       0xf10C0000, 0xfffffe3f, "CPSID" },
  { 0x1,  &armCpsInstruction,       0xf10e0000, 0xfffffe20, "CPSID" },
  { 0x1,  &armCpsInstruction,       0xf1000000, 0xfff1fe20, "CPS" },
  // UNIMPLEMENTED: RFE return from exception
  { 0x1,  &armRfeInstruction,       0xf8100a00, 0xfe50ffff, "rfe%23?id%24?ba\t%16-19r%21'!" },
  // UNIMPLEMENTED: SETEND set endianess
  { 0x1,  &armSetendInstruction,    0xf1010000, 0xfffffc00, "setend\t%9?ble" },
  // UNIMPLEMENTED: SRS store return state
  { 0x1,  &armSrsInstruction,       0xf84d0500, 0xfe5fffe0, "srs%23?id%24?ba\t%16-19r%21'!, #%0-4d" },
  // BLX: branch and link to Thumb subroutine
  { 0x1,  &armBlxImmediateInstruction,    0xfa000000, 0xfe000000, "BLX #imm24" },
  // PLD: preload data
  { 0x1,  &armPldInstruction,       0xf450f000, 0xfc70f000, "PLD" },
  // PLI: preload instruction
  { 0x1,  &armPliInstruction,       0xf450f000, 0xfd70f000, "PLI" },
  { 0x1,  &undefinedInstruction, 0x00000000, 0x00000000, "armUnconditionalInstructions" }
};


/*
 * Top level instruction categories.
 *
 * Identify unconditional instructions first, and terminate the table with a category that catches
 * all instructions, to detect unimplemented and (truly) undefined instructions.
 */
static struct TopLevelCategory armCategories[] =
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
