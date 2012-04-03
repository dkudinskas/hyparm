/*
 * ARM decoding tables for the table search decoder
 */


static struct instruction32bit armBranchBlockTransferInstructions[] =
{
  // STM traps if ^ postfix, otherwise pass through
  { IRC_REPLACE,  armStmInstruction,    0x08400000, 0x0e500000, "STM {regList}^" },
  { IRC_SAFE,     armStmInstruction,    0x08800000, 0x0ff00000, "STMIA {regList}" },
  { IRC_SAFE,     armStmInstruction,    0x08000000, 0x0e100000, "STM {regList}" },
  // POP LDM syntax, only care if PC in reglist
  { IRC_REPLACE,  armLdmInstruction,    0x08bd8000, 0x0fff8000, "LDM SP, {...r15}" },
  { IRC_SAFE,     armLdmInstruction,    0x08bd0000, 0x0fff0000, "LDM SP, {reglist}" },
  // LDM traps if ^ postfix and/or PC is in register list, otherwise pass through
  { IRC_REPLACE,  armLdmInstruction,    0x08500000, 0x0e500000, "LDM Rn, {regList}^" },
  { IRC_REPLACE,  armLdmInstruction,    0x08108000, 0x0e108000, "LDM Rn, {..r15}" },
  { IRC_SAFE,     armLdmInstruction,    0x08900000, 0x0f900000, "LDMIA Rn, {regList}" },
  { IRC_SAFE,     armLdmInstruction,    0x08100000, 0x0e100000, "LDM Rn, {regList}" },
  // B/BL: always hypercall! obviously.
  { IRC_REPLACE,  armBInstruction,      0x0a000000, 0x0e000000, "BRANCH" },
  { IRC_REPLACE,  undefinedInstruction, 0x00000000, 0x00000000, "branchBlockTransferInstructions" }
};

// register/shifter register cases, etc
static struct instruction32bit armDataProcMiscInstructions_op0[] =
{
  // NOP is just a nop...
  { IRC_SAFE,     nopInstruction,       0xe1a00000, 0xffffffff, "NOP" },
  // UNIMPLEMENTED: SWP swap
  { IRC_REPLACE,  armSwpInstruction,       0x01000090, 0x0fb00ff0, "SWP" },
  // UNIMPLEMENTED:
  { 0x2,  armStrhtInstruction,  0x006000b0, 0x0f7000f0, "STRHT instruction" },
  { 0x2,  armLdrhtInstruction,  0x003000b0, 0x0f3000f0, "LDRHT instruction" },
  // store and load exclusive: must be emulated - user mode faults
  { IRC_REPLACE,  armLdrexbInstruction, 0x01d00f9f, 0x0ff00fff, "LDREXB" },
  { IRC_REPLACE,  armLdrexdInstruction, 0x01b00f9f, 0x0ff00fff, "LDREXD" },
  { IRC_REPLACE,  armLdrexhInstruction, 0x01f00f9f, 0x0ff00fff, "LDREXH" },
  { IRC_REPLACE,  armStrexbInstruction, 0x01c00f90, 0x0ff00ff0, "STREXB" },
  { IRC_REPLACE,  armStrexdInstruction, 0x01a00f90, 0x0ff00ff0, "STREXD" },
  { IRC_REPLACE,  armStrexhInstruction, 0x01e00f90, 0x0ff00ff0, "STREXH" },
  { IRC_REPLACE,  armLdrexInstruction,  0x01900f9f, 0x0ff00fff, "LDREX" },
  { IRC_REPLACE,  armStrexInstruction,  0x01800f90, 0x0ff00ff0, "STREX" },
  // SMULL - signed multiply, PC cannot be used as any destination
  { IRC_SAFE,     armSmullInstruction,    0x00800090, 0x0fa000f0, "SMULL" },
  // SMLAL - signed multiply and accumulate, PC cannot be used as any destination
  { IRC_SAFE,     armSmlalInstruction,    0x00a00090, 0x0fa000f0, "SMLAL" },
  // MUL: Rd = Rm * Rn; Rd != PC. pass through
  { IRC_SAFE,     armMulInstruction,       0x00000090, 0x0fe000f0, "MUL Rd, Rm, Rn" },
  // MLA: Rd = Rm * Rn + Ra; Rd != PC. pass through
  { IRC_SAFE,     armMlaInstruction,       0x00200090, 0x0fe000f0, "MLA Rd, Rm, Rn, Ra" },
  // MLS: Rd = Rm * Rn - Ra; Rd != PC, pass through
  { IRC_SAFE,     armMlsInstruction,       0x00600090, 0x0ff000f0, "MLS Rd, Rm, Rn, Ra" },
  // Branch and try to exchange to ARM mode.
  { IRC_REPLACE,  armBxInstruction,     0x012FFF10, 0x0ffffff0, "bx%c\t%0-3r" },
  // Branch and link and try to exchange to Jazelle mode.
  { IRC_REPLACE,  armBxjInstruction,    0x012fff20, 0x0ffffff0, "BXJ Rm" },
  // Software breakpoint instruction... not sure yet.
  { IRC_REPLACE,  armBkptInstruction,      0xe1200070, 0xfff000f0, "BKPT #imm8" },
  // UNIMPLEMENTED: SMC, previously SMI: secure monitor call
  { IRC_REPLACE,  armSmcInstruction,       0x01600070, 0x0ff000f0, "smc%c\t%e" },
  // Branch and link and try to exchange with Thumb mode.
  { IRC_REPLACE,  armBlxRegisterInstruction,       0x012fff30, 0x0ffffff0, "BLX Rm" },
  // CLZ: Count leading zeroes - Rd, Rm != PC, pass through
  { IRC_SAFE,     armClzInstruction,       0x016f0f10, 0x0fff0ff0, "CLZ Rd, Rm" },
  // Q(D){ADD,SUB}: saturated add/subtract/doubleAdd/doubleSubtract, unpredictable when any R=PC
  { IRC_SAFE,     armQaddInstruction,      0x01000050, 0x0ff00ff0, "qadd%c\t%12-15r, %0-3r, %16-19r" },
  { IRC_SAFE,     armQdaddInstruction,     0x01400050, 0x0ff00ff0, "qdadd%c\t%12-15r, %0-3r, %16-19r" },
  { IRC_SAFE,     armQsubInstruction,      0x01200050, 0x0ff00ff0, "qsub%c\t%12-15r, %0-3r, %16-19r" },
  { IRC_SAFE,     armQdsubInstruction,     0x01600050, 0x0ff00ff0, "qdsub%c\t%12-15r, %0-3r, %16-19r" },
  // LDRD: Rt1 must be even numbered and NOT 14, thus Rt2 cannot be PC. pass.
  { IRC_SAFE,     armLdrdInstruction,   0x004000d0, 0x0e5000f0, "LDRD Rt, [Rn, #imm]" },
  { IRC_SAFE,     armLdrdInstruction,   0x000000d0, 0x0e500ff0, "LDRD Rt, [Rn, Rm]" },
  // STRD: pass through, let them fail!
  { IRC_SAFE,     armStrdInstruction,   0x004000f0, 0x0e5000f0, "STRD Rt, [Rn, #imm]" },
  { IRC_SAFE,     armStrdInstruction,   0x000000f0, 0x0e500ff0, "STRD Rt, [Rn, Rm]" },
  // signed 16 bit multiply, 32 bit accumulate, unpredictable if any R=PC
  { IRC_SAFE,     armSmlabbInstruction,    0x01000080, 0x0ff000f0, "smlabb%c\t%16-19r, %0-3r, %8-11r, %12-15r" },
  { IRC_SAFE,     armSmlatbInstruction,    0x010000a0, 0x0ff000f0, "smlatb%c\t%16-19r, %0-3r, %8-11r, %12-15r" },
  { IRC_SAFE,     armSmlabtInstruction,    0x010000c0, 0x0ff000f0, "smlabt%c\t%16-19r, %0-3r, %8-11r, %12-15r" },
  { IRC_SAFE,     armSmlattInstruction,    0x010000e0, 0x0ff000f0, "smlatt%c\t%16-19r, %0-3r, %8-11r, %12-15r" },
  // signed 16 bit x 32 bit multiply, 32 bit accumulate, unpredictable if any R=PC
  { IRC_SAFE,     armSmlawbInstruction,    0x01200080, 0x0ff000f0, "smlawb%c\t%16-19r, %0-3r, %8-11r, %12-15r" },
  { IRC_SAFE,     armSmlawtInstruction,    0x012000c0, 0x0ff000f0, "smlawt%c\t%16-19r, %0-3r, %8-11r, %12-15r" },
  // signed 16 bit multiply, 64 bit accumulate, unpredictable if any R=PC
  { IRC_SAFE,     armSmlalbbInstruction,   0x01400080, 0x0ff000f0, "smlalbb%c\t%12-15r, %16-19r, %0-3r, %8-11r" },
  { IRC_SAFE,     armSmlaltbInstruction,   0x014000a0, 0x0ff000f0, "smlaltb%c\t%12-15r, %16-19r, %0-3r, %8-11r" },
  { IRC_SAFE,     armSmlalbtInstruction,   0x014000c0, 0x0ff000f0, "smlalbt%c\t%12-15r, %16-19r, %0-3r, %8-11r" },
  { IRC_SAFE,     armSmlalttInstruction,   0x014000e0, 0x0ff000f0, "smlaltt%c\t%12-15r, %16-19r, %0-3r, %8-11r" },
  // signed 16 bit multiply, 32 bit result, any R=PC is unpredictable
  { IRC_SAFE,     armSmulbbInstruction,    0x01600080, 0x0ff0f0f0, "SMULBB Rd, Rn, RM" },
  { IRC_SAFE,     armSmultbInstruction,    0x016000a0, 0x0ff0f0f0, "smultb%c\t%16-19r, %0-3r, %8-11r" },
  { IRC_SAFE,     armSmulbtInstruction,    0x016000c0, 0x0ff0f0f0, "smulbt%c\t%16-19r, %0-3r, %8-11r" },
  { IRC_SAFE,     armSmulttInstruction,    0x016000e0, 0x0ff0f0f0, "smultt%c\t%16-19r, %0-3r, %8-11r" },
  // signed 16 bit x 32 bit multiply, 32 bit result, any R=PC is unpredictable
  { IRC_SAFE,     armSmulwbInstruction,    0x012000a0, 0x0ff0f0f0, "smulwb%c\t%16-19r, %0-3r, %8-11r" },
  { IRC_SAFE,     armSmulwtInstruction,    0x012000e0, 0x0ff0f0f0, "smulwt%c\t%16-19r, %0-3r, %8-11r" },
  // STRH: passthrough, will data abort if something wrong
  { IRC_SAFE,     armStrhInstruction,   0x004000b0, 0x0e5000f0, "STRH Rt, [Rn, +-imm8]" },
  { IRC_SAFE,     armStrhInstruction,   0x000000b0, 0x0e500ff0, "STRH Rt, [Rn], +-Rm" },
  // LDRH cant load halfword to PC, passthrough
  { IRC_SAFE,     armLdrhInstruction,   0x00500090, 0x0e500090, "LDRH Rt, [Rn, +-imm8]" },
  { IRC_SAFE,     armLdrhInstruction,   0x00100090, 0x0e500f90, "LDRH Rt, [Rn], +-Rm" },
  // AND: Rd = PC end block, others are fine
  { IRC_REPLACE,  armAndInstruction,       0x0000f000, 0x0fe0f010, "AND PC, Rn, Rm, #shamt" },
  { IRC_REPLACE,  armAndInstruction,       0x0000f010, 0x0fe0f090, "AND PC, Rn, Rm, Rshamt" },
  { IRC_SAFE,     armAndInstruction,       0x00000000, 0x0fe00010, "AND Rd, Rn, Rm, #shamt" },
  { IRC_SAFE,     armAndInstruction,       0x00000010, 0x0fe00090, "AND Rd, Rn, Rm, Rshamt" },
  // EOR: Rd = PC end block, others are fine
  { IRC_REPLACE,  armEorInstruction,       0x0020f000, 0x0fe0f010, "EOR PC, Rn, Rm, #shamt" },
  { IRC_REPLACE,  armEorInstruction,       0x0020f010, 0x0fe0f090, "EOR PC, Rn, Rm, Rshamt" },
  { IRC_SAFE,     armEorInstruction,       0x00200000, 0x0fe00010, "EOR Rd, Rn, Rm, #shamt" },
  { IRC_SAFE,     armEorInstruction,       0x00200010, 0x0fe00090, "EOR Rd, Rn, Rm, Rshamt" },
  // SUB: Rd = PC end block, others are fine
  { IRC_REPLACE,  armSubInstruction,       0x0040f000, 0x0fe0f010, "SUB PC, Rn, Rm, #shamt" },
  { IRC_REPLACE,  armSubInstruction,       0x0040f010, 0x0fe0f090, "SUB PC, Rn, Rm, Rshamt" },
  { IRC_SAFE,     armSubInstruction,       0x00400000, 0x0fe00010, "SUB Rd, Rn, Rm, #shamt" },
  { IRC_SAFE,     armSubInstruction,       0x00400010, 0x0fe00090, "SUB Rd, Rn, Rm, Rshamt" },
  // RSB: Rd = PC end block, others are fine
  { IRC_REPLACE,  armRsbInstruction,       0x0060f000, 0x0fe0f010, "RSB PC, Rn, Rm, #shamt" },
  { IRC_REPLACE,  armRsbInstruction,       0x0060f010, 0x0fe0f090, "RSB PC, Rn, Rm, Rshamt" },
  { IRC_SAFE,     armRsbInstruction,       0x00600000, 0x0fe00010, "RSB Rd, Rn, Rm, #shamt" },
  { IRC_SAFE,     armRsbInstruction,       0x00600010, 0x0fe00090, "RSB Rd, Rn, Rm, Rshamt" },
  // ADD: Rd = PC end block, others are fine
  { IRC_REPLACE,  armAddInstruction,       0x0080f000, 0x0fe0f010, "ADD PC, Rn, Rm, #shamt" },
  { IRC_REPLACE,  armAddInstruction,       0x0080f010, 0x0fe0f090, "ADD PC, Rn, Rm, Rshamt" },
  { IRC_SAFE,     armAddInstruction,       0x00800000, 0x0fe00010, "ADD Rd, Rn, Rm, #shamt" },
  { IRC_SAFE,     armAddInstruction,       0x00800010, 0x0fe00090, "ADD Rd, Rn, Rm, Rshamt" },
  // ADC: Rd = PC end block, others are fine
  { IRC_REPLACE,  armAdcInstruction,       0x00a0f000, 0x0fe0f010, "ADC PC, Rn, Rm, #shamt" },
  { IRC_REPLACE,  armAdcInstruction,       0x00a0f010, 0x0fe0f090, "ADC PC, Rn, Rm, Rshamt" },
  { IRC_SAFE,     armAdcInstruction,       0x00a00000, 0x0fe00010, "ADC Rd, Rn, Rm, #shamt" },
  { IRC_SAFE,     armAdcInstruction,       0x00a00010, 0x0fe00090, "ADC Rd, Rn, Rm, Rshamt" },
  // SBC: Rd = PC end block, others are fine
  { IRC_REPLACE,  armSbcInstruction,       0x00c0f000, 0x0fe0f010, "SBC Rd, Rn, Rm, #shamt" },
  { IRC_REPLACE,  armSbcInstruction,       0x00c0f010, 0x0fe0f090, "SBC Rd, Rn, Rm, Rshamt" },
  { IRC_SAFE,     armSbcInstruction,       0x00c00000, 0x0fe00010, "SBC Rd, Rn, Rm, #shamt" },
  { IRC_SAFE,     armSbcInstruction,       0x00c00010, 0x0fe00090, "SBC Rd, Rn, Rm, Rshamt" },
  // RSC: Rd = PC end block, others are fine
  { IRC_REPLACE,  armRscInstruction,       0x00e0f000, 0x0fe0f010, "RSC PC, Rn, Rm, #shamt" },
  { IRC_REPLACE,  armRscInstruction,       0x00e0f010, 0x0fe0f090, "RSC PC, Rn, Rm, Rshamt" },
  { IRC_SAFE,     armRscInstruction,       0x00e00000, 0x0fe00010, "RSC Rd, Rn, Rm, #shamt" },
  { IRC_SAFE,     armRscInstruction,       0x00e00010, 0x0fe00090, "RSC Rd, Rn, Rm, Rshamt" },
  // MSR/MRS: always hypercall! we must hide the real state from guest.
  { IRC_REPLACE,  armMsrInstruction,       0x0120f000, 0x0fb0fff0, "MSR, s/cpsr, Rn" },
  { IRC_REPLACE,  armMrsInstruction,       0x010f0000, 0x0fbf0fff, "MRS, Rn, s/cpsr" },
  // TST instructions are all fine
  { IRC_SAFE,     armTstInstruction,       0x01000000, 0x0fe00010, "TST Rn, Rm, #shamt" },
  { IRC_SAFE,     armTstInstruction,       0x01000010, 0x0fe00090, "TST Rn, Rm, Rshift" },
  // TEQ instructions are all fine
  { IRC_SAFE,     armTeqInstruction,       0x01200000, 0x0fe00010, "TEQ Rn, Rm, #shamt" },
  { IRC_SAFE,     armTeqInstruction,       0x01200010, 0x0fe00090, "TEQ Rn, Rm, Rshift" },
  // CMP instructions are all fine
  { IRC_SAFE,     armCmpInstruction,       0x01400000, 0x0fe00010, "CMP Rn, Rm, #shamt" },
  { IRC_SAFE,     armCmpInstruction,       0x01400010, 0x0fe00090, "CMP Rn, Rm, Rshamt" },
  // CMN instructions are all fine
  { IRC_SAFE,     armCmnInstruction,       0x01600000, 0x0fe00010, "CMN Rn, Rm, #shamt" },
  { IRC_SAFE,     armCmnInstruction,       0x01600010, 0x0fe00090, "CMN Rn, Rm, Rshamt" },
  // ORR: Rd = PC end block, other are fine
  { IRC_REPLACE,  armOrrInstruction,       0x0180f000, 0x0fe0f010, "ORR PC, Rn, Rm, #shamt" },
  { IRC_REPLACE,  armOrrInstruction,       0x0180f010, 0x0fe0f090, "ORR PC, Rn, Rm, Rshamt" },
  { IRC_SAFE,     armOrrInstruction,       0x01800000, 0x0fe00010, "ORR Rd, Rn, Rm, #shamt" },
  { IRC_SAFE,     armOrrInstruction,       0x01800010, 0x0fe00090, "ORR Rd, Rn, Rm, Rshamt" },
  // MOV with Rd = PC end block. MOV reg, <shifted reg> is a pseudo instr for shift instructions
  { IRC_REPLACE,  armMovInstruction,       0x01a0f000, 0x0deffff0, "MOV PC, Rm" },
  { IRC_SAFE,     armMovInstruction,       0x01a00000, 0x0def0ff0, "MOV Rn, Rm" },
  // LSL: Rd = PC and shamt = #imm, then end block. Otherwise fine
  { IRC_REPLACE,  armLslInstruction,       0x01a0f000, 0x0feff070, "LSL Rd, Rm, #shamt" },
  { IRC_REPLACE,  armLslInstruction,       0x01a0f010, 0x0feff0f0, "LSL Rd, Rm, Rshamt" },
  { IRC_SAFE,     armLslInstruction,       0x01a00000, 0x0fef0070, "LSL Rd, Rm, #shamt" },
  { IRC_SAFE,     armLslInstruction,       0x01a00010, 0x0fef00f0, "LSL Rd, Rm, Rshamt" },
  // LSR: Rd = PC and shamt = #imm, then end block. Otherwise fine
  { IRC_REPLACE,  armLsrInstruction,       0x01a0f020, 0x0feff070, "LSR Rd, Rm, #shamt" },
  { IRC_REPLACE,  armLsrInstruction,       0x01a0f030, 0x0feff0f0, "LSR Rd, Rm, Rshamt" },
  { IRC_SAFE,     armLsrInstruction,       0x01a00020, 0x0fef0070, "LSR Rd, Rm, #shamt" },
  { IRC_SAFE,     armLsrInstruction,       0x01a00030, 0x0fef00f0, "LSR Rd, Rm, Rshamt" },
  // ASR: Rd = PC and shamt = #imm, then end block. Otherwise fine
  { IRC_REPLACE,  armAsrInstruction,       0x01a0f040, 0x0feff070, "ASR Rd, Rm, #shamt" },
  { IRC_REPLACE,  armAsrInstruction,       0x01a0f050, 0x0feff0f0, "ASR Rd, Rm, Rshamt" },
  { IRC_SAFE,     armAsrInstruction,       0x01a00040, 0x0fef0070, "ASR Rd, Rm, #shamt" },
  { IRC_SAFE,     armAsrInstruction,       0x01a00050, 0x0fef00f0, "ASR Rd, Rm, Rshamt" },
  // RRX: shift right and extend, Rd can be PC
  { IRC_REPLACE,  armRrxInstruction,       0x01a0f060, 0x0feffff0, "RRX PC, Rm" },
  { IRC_SAFE,     armRrxInstruction,       0x01a00060, 0x0fef0ff0, "RRX Rd, Rm" },
  // ROR: reg case destination unpredictable. imm case dest can be PC.
  { IRC_SAFE,     armRorInstruction,       0x01a00070, 0x0fef00f0, "ROR Rd, Rm, Rn" },
  { IRC_REPLACE,  armRorInstruction,       0x01a0f060, 0x0feff070, "ROR PC, Rm, #imm" },
  { IRC_SAFE,     armRorInstruction,       0x01a00060, 0x0fef0070, "ROR Rd, Rm, #imm" },
  // BIC with Rd = PC end block, other are fine.
  { IRC_REPLACE,  armBicInstruction,       0x01c0f000, 0x0fe0f010, "BIC PC, Rn, Rm, #shamt" },
  { IRC_REPLACE,  armBicInstruction,       0x01c0f010, 0x0fe0f090, "BIC PC, Rn, Rm, Rshamt" },
  { IRC_SAFE,     armBicInstruction,       0x01c00000, 0x0fe00010, "BIC Rd, Rn, Rm, #shamt" },
  { IRC_SAFE,     armBicInstruction,       0x01c00010, 0x0fe00090, "BIC Rd, Rn, Rm, Rshamt" },
  // MVN with Rd = PC end block, other are fine.
  { IRC_REPLACE,  armMvnInstruction,       0x01e0f000, 0x0fe0f010, "MVN Rd, Rm, #shamt" },
  { IRC_REPLACE,  armMvnInstruction,       0x01e0f010, 0x0fe0f090, "MVN Rd, Rm, Rshamt" },
  { IRC_SAFE,     armMvnInstruction,       0x01e00000, 0x0fe00010, "MVN Rd, Rm, #shamt" },
  { IRC_SAFE,     armMvnInstruction,       0x01e00010, 0x0fe00090, "MVN Rd, Rm, Rshamt" },

  { IRC_REPLACE,  &undefinedInstruction, 0x00000000, 0x00000000, "dataProcMiscInstructions_op0" }
};

// immediate cases and random hints
static struct instruction32bit armDataProcMiscInstructions_op1[] =
{
  // UNIMPLEMENTED: yield hint
  { IRC_REPLACE,  armYieldInstruction,     0x0320f001, 0x0fffffff, "yield%c" },
  // UNIMPLEMENTED: wait for event hint
  { IRC_REPLACE,  armWfeInstruction,       0x0320f002, 0x0fffffff, "wfe%c" },
  // UNIMPLEMENTED: wait for interrupt hint
  { IRC_REPLACE,  armWfiInstruction,       0x0320f003, 0x0fffffff, "wfi%c" },
  // UNIMPLEMENTED: send event hint
  { IRC_REPLACE,  armSevInstruction,       0x0320f004, 0x0fffffff, "sev%c" },
  { IRC_SAFE,     nopInstruction,       0x0320f000, 0x0fffff00, "NOP" },
  // UNIMPLEMENTED: debug hint
  { IRC_REPLACE,  armDbgInstruction,       0x0320f0f0, 0x0ffffff0, "dbg%c\t#%0-3d" },
  // MOVW - indication of 'wide' - to select ARM encoding. Rd cant be PC, pass through.
  { IRC_SAFE,     armMovwInstruction,      0x03000000, 0x0ff00000, "MOVW Rd, Rn" },
  { IRC_SAFE,     armMovtInstruction,      0x03400000, 0x0ff00000, "MOVT Rd, Rn" },
  // AND: Rd = PC end block, others are fine
  { IRC_REPLACE,  armAndInstruction,       0x0200f000, 0x0fe0f000, "AND PC, Rn, #imm" },
  { IRC_SAFE,     armAndInstruction,       0x02000000, 0x0fe00000, "AND Rd, Rn, #imm" },
  // EOR: Rd = PC end block, others are fine
  { IRC_REPLACE,  armEorInstruction,       0x0220f000, 0x0fe0f000, "EOR PC, Rn, Rm/#imm" },
  { IRC_SAFE,     armEorInstruction,       0x02200000, 0x0fe00000, "EOR Rd, Rn, #imm" },
  // SUB: Rd = PC end block, others are fine
  { IRC_REPLACE,  armSubInstruction,       0x0240f000, 0x0fe0f000, "SUB PC, Rn, Rm/imm" },
  { IRC_SAFE,     armSubInstruction,       0x02400000, 0x0fe00000, "SUB Rd, Rn, #imm" },
  // RSB: Rd = PC end block, others are fine
  { IRC_REPLACE,  armRsbInstruction,       0x0260f000, 0x0fe0f000, "RSB PC, Rn, Rm/imm" },
  { IRC_SAFE,     armRsbInstruction,       0x02600000, 0x0fe00000, "RSB Rd, Rn, #imm" },
  // ADD: Rd = PC end block, others are fine
  { IRC_REPLACE,  armAddInstruction,       0x0280f000, 0x0fe0f000, "ADD PC, Rn, #imm" },
  { IRC_SAFE,     armAddInstruction,       0x02800000, 0x0fe00000, "ADD Rd, Rn, #imm" },
  // ADC: Rd = PC end block, others are fine
  { IRC_REPLACE,  armAdcInstruction,       0x02a0f000, 0x0fe0f000, "ADC PC, Rn/#imm" },
  { IRC_SAFE,     armAdcInstruction,       0x02a00000, 0x0fe00000, "ADC Rd, Rn, #imm" },
  // SBC: Rd = PC end block, others are fine
  { IRC_REPLACE,  armSbcInstruction,       0x02c0f000, 0x0fe0f000, "SBC PC, Rn/#imm" },
  { IRC_SAFE,     armSbcInstruction,       0x02c00000, 0x0fe00000, "SBC Rd, Rn, #imm" },
  // RSC: Rd = PC end block, others are fine
  { IRC_REPLACE,  armRscInstruction,       0x02e0f000, 0x0fe0f000, "RSC PC, Rn/#imm" },
  { IRC_SAFE,     armRscInstruction,       0x02e00000, 0x0fe00000, "RSC Rd, Rn, #imm" },
  // MSR: always hypercall! we must hide the real state from guest.
  { IRC_REPLACE,  armMsrInstruction,       0x0320f000, 0x0fb0f000, "MSR, s/cpsr, #imm" },
  // TST instructions are all fine
  { IRC_SAFE,     armTstInstruction,       0x03000000, 0x0fe00000, "TST Rn, #imm" },
  // TEQ instructions are all fine
  { IRC_SAFE,     armTeqInstruction,       0x03200000, 0x0fe00000, "TEQ Rn, #imm" },
  // CMP instructions are all fine
  { IRC_SAFE,     armCmpInstruction,       0x03400000, 0x0fe00000, "CMP Rn, #imm" },
  // CMN instructions are all fine
  { IRC_SAFE,     armCmnInstruction,       0x03600000, 0x0fe00000, "CMN Rn, #imm" },
  // ORR: Rd = PC end block, other are fine
  { IRC_REPLACE,  armOrrInstruction,       0x0380f000, 0x0fe0f000, "ORR Rd, Rn, #imm" },
  { IRC_SAFE,     armOrrInstruction,       0x03800000, 0x0fe00000, "ORR Rd, Rn, #imm" },
  // MOV with Rd = PC end block. MOV <shifted reg> is a pseudo instr..
  { IRC_REPLACE,  armMovInstruction,       0x03a0f000, 0x0feff000, "MOV PC, #imm" },
  { IRC_SAFE,     armMovInstruction,       0x03a00000, 0x0fef0000, "MOV Rn, #imm" },
  // BIC with Rd = PC end block, other are fine.
  { IRC_REPLACE,  armBicInstruction,       0x03c0f000, 0x0fe0f000, "BIC PC, Rn, #imm" },
  { IRC_SAFE,     armBicInstruction,       0x03c00000, 0x0fe00000, "BIC Rd, Rn, #imm" },
  // MVN with Rd = PC end block, other are fine.
  { IRC_REPLACE,  armMvnInstruction,       0x03e0f000, 0x0fe0f000, "MVN PC, Rn, #imm" },
  { IRC_SAFE,     armMvnInstruction,       0x03e00000, 0x0fe00000, "MVN Rd, Rn, #imm" },

  { IRC_REPLACE,  &undefinedInstruction, 0x00000000, 0x00000000, "dataProcMiscInstructions_op1" }
};

static struct instruction32bit armLoadStoreWordByteInstructions[] =
{
  // STRT - all trap
  { 0x2, armStrtInstruction,   0x04200000, 0x0f700000, "STRT Rt, [Rn], +-imm12" },
  { 0x2, armStrtInstruction,   0x06200000, 0x0f700010, "STRT Rt, [Rn], +-Rm" },
  // LDRT - all trap
  { 0x2, armLdrtInstruction,   0x04300000, 0x0f700000, "LDRT Rd, [Rn], +-imm12" },
  { 0x2, armLdrtInstruction,   0x06300000, 0x0f700010, "LDRT Rd, [Rn], +-Rm" },
  // STRBT - all trap
  { 0x2, armStrbtInstruction,  0x04600000, 0x0f700000, "STRBT Rt, [Rn, +-imm12]" },
  { 0x2, armStrbtInstruction,  0x06600000, 0x0f700010, "STRBT Rt, [Rn], +-Rm" },
  // LDRBT - all trap
  { 0x2, armLdrbtInstruction,  0x04700000, 0x0f700000, "LDRBT Rd, [Rn], +-imm12" },
  { 0x2, armLdrbtInstruction,  0x06700000, 0x0f700010, "LDRBT Rd, [Rn], +-Rm" },
  // STR - all pass-through
  { IRC_SAFE,     armStrInstruction,    0x04000000, 0x0e500000, "STR Rt, [Rn, +-imm12]" },
  { IRC_SAFE,     armStrInstruction,    0x06000000, 0x0e500010, "STR Rt, [Rn], +-Rm" },
  // LDR traps if dest = PC, otherwise pass through
  { IRC_REPLACE,  armLdrInstruction,    0x0410f000, 0x0e50f000, "LDR PC, [Rn], +-imm12" },
  { IRC_SAFE,     armLdrInstruction,    0x04100000, 0x0e500000, "LDR Rd, [Rn], +-imm12" },
  { IRC_REPLACE,  armLdrInstruction,    0x0610f000, 0x0e50f010, "LDR PC, [Rn], +-Rm" },
  { IRC_SAFE,     armLdrInstruction,    0x06100000, 0x0e500010, "LDR Rd, [Rn], +-Rm" },
  // STRB pass through
  { IRC_SAFE,     armStrbInstruction,   0x04400000, 0x0e500000, "STRB Rt, [Rn, +-imm12]" },
  { IRC_SAFE,     armStrbInstruction,   0x06400000, 0x0e500010, "STRB Rt, [Rn], +-Rm" },
  // LDRB - pass through, dest can't be PC
  { IRC_SAFE,     armLdrbInstruction,   0x04500000, 0x0e500000, "LDRB Rd, [Rn], +-imm12" },
  { IRC_SAFE,     armLdrbInstruction,   0x06500000, 0x0e500010, "LDRB Rd, [Rn], +-Rm" },

  { IRC_REPLACE,  &undefinedInstruction, 0x00000000, 0x00000000, "loadStoreWordByteInstructions" }
};

static struct instruction32bit armMediaInstructions[] =
{
  // BFC: bit field clear, dest PC not allowed.
  { IRC_SAFE,     armBfcInstruction,       0x07c0001f, 0x0fe0007f, "BFC Rd, #LSB, #width" },
  // BFI: bit field insert, dest PC not allowed.
  { IRC_SAFE,     armBfiInstruction,       0x07c00010, 0x0fe00070, "BFI Rd, #LSB, #width" },
  // RBIT: reverse bits, if Rd or Rm = 15 then unpredictable.
  { IRC_SAFE,     armRbitInstruction,      0x06ff0f30, 0x0fff0ff0, "RBIT Rd,Rm" },
  // UBFX: extract bit field - destination 15 unpredictable
  { IRC_SAFE,     armUbfxInstruction,     0x07a00050, 0x0fa00070, "UBFX Rd, Rn, width" },
  // PKH (pack halfword), if Rd or Rn or Rm = 15 then unpredictable
  { IRC_SAFE,     armPkhbtInstruction,     0x06800010, 0x0ff00ff0, "PKHBT Rd,Rn,Rm" },
  { IRC_SAFE,     armPkhbtInstruction,     0x06800010, 0x0ff00070, "PKHBT Rd,Rn,Rm,LSL #imm" },
  { IRC_SAFE,     armPkhtbInstruction,     0x06800050, 0x0ff00ff0, "PKHTB Rd,Rn,Rm,ASR #32" },
  { IRC_SAFE,     armPkhtbInstruction,     0x06800050, 0x0ff00070, "PKHTB Rd,Rn,Rm,ASR #imm" },
  // QADD8,QADD16: saturating add 16 bit and 8 bit, if Rd or Rn or Rm = 15 then unpredictable
  { IRC_SAFE,     armQadd16Instruction,    0x06200f10, 0x0ff00ff0, "QADD16 Rd,Rn,Rm" },
  { IRC_SAFE,     armQadd8Instruction,     0x06200f90, 0x0ff00ff0, "QADD8 Rd,Rn,Rm" },
  // QASX: saturating add and subtract with exchange if Rd or Rn or Rm = 15 then unpredictable
  { IRC_SAFE,     armQasxInstruction,  0x06200f30, 0x0ff00ff0, "QASX Rd,Rn,Rm" },
  // QSUB8,QSUB16: saturating subtract 16 and 8 bit if Rd or Rn or Rm = 15 then unpredictable
  { IRC_SAFE,     armQsub16Instruction,    0x06200f70, 0x0ff00ff0, "QSUB16 Rd,Rn,Rm" },
  { IRC_SAFE,     armQsub8Instruction,     0x06200ff0, 0x0ff00ff0, "QSUB8 Rd,Rn,Rm" },
  // QSAX: saturating subtract and add with exchange if Rd or Rn or Rm = 15 then unpredictable
  { IRC_SAFE,     armQsaxInstruction,  0x06200f50, 0x0ff00ff0, "QSAX Rd,Rn,Rm" },
  // SADD8,SADD16: signed add 16 bit and 8 bit, if Rd or Rn or Rm = 15 then unpredictable
  { IRC_SAFE,     armSadd16Instruction,    0x06100f10, 0x0ff00ff0, "SADD16 Rd,Rn,Rm" },
  { IRC_SAFE,     armSadd8Instruction,     0x06100f90, 0x0ff00ff0, "SADD8 Rd,Rn,Rm" },
  // SASX: signed add and subtract with exchange, if Rd or Rn or Rm = 15 then unpredictable
  { IRC_SAFE,     armSasxInstruction,  0x06100f30, 0x0ff00ff0, "SASX Rd,Rn,Rm" },
  // SSUB8,SSUB16: signed subtract 16 bit and 8 bit, if Rd or Rn or Rm = 15 then unpredictable
  { IRC_SAFE,     armSsub16Instruction,    0x06100f70, 0x0ff00ff0, "SSUB16 Rd,Rn,Rm" },
  { IRC_SAFE,     armSsub8Instruction,     0x06100ff0, 0x0ff00ff0, "SSUB8 Rd,Rn,Rm" },
  // SSAX: signed subtract and add with exchange,if Rd or Rn or Rm = 15 then unpredictable
  { IRC_SAFE,     armSsaxInstruction,  0x06100f50, 0x0ff00ff0, "SSAX Rd,Rn,Rm" },
  // SHADD8,SHADD16: signed halvign add 16 bit and 8 bit, if Rd or Rn or Rm = 15 then unpredictable
  { IRC_SAFE,     armShadd16Instruction,   0x06300f10, 0x0ff00ff0, "SHADD16 Rd,Rn,Rm" },
  { IRC_SAFE,     armShadd8Instruction,    0x06300f90, 0x0ff00ff0, "SHADD8 Rd,Rn,Rm" },
  // SHASX: signed halving add and subtract with exchange, if Rd or Rn or Rm = 15 then unpredictable
  { IRC_SAFE,     armShasxInstruction, 0x06300f30, 0x0ff00ff0, "SHASX Rd,Rn,Rm" },
  // SHSUB8,SHSUB16: signed halvign subtract 16 bit and 8 bit, if Rd or Rn or Rm = 15 then unpredictable
  { IRC_SAFE,     armShsub16Instruction,   0x06300f70, 0x0ff00ff0, "SHSUB16 Rd,Rn,Rm" },
  { IRC_SAFE,     armShsub8Instruction,    0x06300ff0, 0x0ff00ff0, "SHSUB8 Rd,Rn,Rm" },
  // SHSAX: signed halving subtract and add with exchange, if Rd or Rn or Rm = 15 then unpredictable
  { IRC_SAFE,     armShsaxInstruction, 0x06300f50, 0x0ff00ff0, "SHSAX Rd,Rn,Rm" },
  // UADD8,UADD16: unsigned add 16 bit and 8 bit, if Rd or Rn or Rm = 15 then unpredictable
  { IRC_SAFE,     armUadd16Instruction,    0x06500f10, 0x0ff00ff0, "UADD16 Rd,Rn,Rm" },
  { IRC_SAFE,     armUadd8Instruction,     0x06500f90, 0x0ff00ff0, "UADD8 Rd,Rn,Rm" },
  // UASX: unsigned add and subtract with exchange, if Rd or Rn or Rm = 15 then unpredictable
  { IRC_SAFE,     armUasxInstruction,  0x06500f30, 0x0ff00ff0, "UASX Rd,Rn,Rm" },
  // USUB8,USUB16: unsigned subtract 16 bit and 8 bit, if Rd or Rn or Rm = 15 then unpredictable
  { IRC_SAFE,     armUsub16Instruction,    0x06500f70, 0x0ff00ff0, "USUB16 Rd,Rn,Rm" },
  { IRC_SAFE,     armUsub8Instruction,     0x06500ff0, 0x0ff00ff0, "USUB8 Rd,Rn,Rm" },
  // USAX: unsigned subtract and add with exchange, if Rd or Rn or Rm = 15 then unpredictable
  { IRC_SAFE,     armUsaxInstruction,  0x06500f50, 0x0ff00ff0, "USAX Rd,Rn,Rm" },
  // UHADD8,UHADD16: unsigned halving add 16 bit and 8 bit, if Rd or Rn or Rm = 15 then unpredictable
  { IRC_SAFE,     armUhadd16Instruction,   0x06700f10, 0x0ff00ff0, "UHADD16 Rd,Rn,Rm" },
  { IRC_SAFE,     armUhadd8Instruction,    0x06700f90, 0x0ff00ff0, "UHADD8 Rd,Rn,Rm" },
  // UHASX: unsigned halving add and subtract with exchange, if Rd or Rn or Rm = 15 then unpredictable
  { IRC_SAFE,     armUhasxInstruction, 0x06700f30, 0x0ff00ff0, "UHASX Rd,Rn,Rm" },
  // UHSUB8,UHSUB16: unsigned halving subtract 16 bit and 8 bit, if Rd or Rn or Rm = 15 then unpredictable
  { IRC_SAFE,     armUhsub16Instruction,   0x06700f70, 0x0ff00ff0, "UHSUB16 Rd,Rn,Rm" },
  { IRC_SAFE,     armUhsub8Instruction,    0x06700ff0, 0x0ff00ff0, "UHSUB8 Rd,Rn,Rm" },
  // UHSAX: unsigned halving subtract and add with exchange, if Rd or Rn or Rm = 15 then unpredictable
  { IRC_SAFE,     armUhsaxInstruction, 0x06700f50, 0x0ff00ff0, "UHSAX Rd,Rn,Rm" },
  // UQADD8,UQADD16:  unsigned saturating add 16 bit and 8 bit, if Rd or Rn or Rm = 15 then unpredictable
  { IRC_SAFE,     armUqadd16Instruction,   0x06600f10, 0x0ff00ff0, "UQADD16 Rd,Rn,Rm" },
  { IRC_SAFE,     armUqadd8Instruction,    0x06600f90, 0x0ff00ff0, "UQADD8 Rd,Rn,Rm" },
  // UQASX: unsigned saturating add and subtract with exchange, if Rd or Rn or Rm = 15 then unpredictable
  { IRC_SAFE,     armUqasxInstruction, 0x06600f30, 0x0ff00ff0, "UQASX Rd,Rn,Rm" },
  // UQSUB8,UQSUB16: unsigned saturating subtract 16 bit and 8 bit, if Rd or Rn or Rm = 15 then unpredictable
  { IRC_SAFE,     armUqsub16Instruction,   0x06600f70, 0x0ff00ff0, "UQSUB16 Rd,Rn,Rm" },
  { IRC_SAFE,     armUqsub8Instruction,    0x06600ff0, 0x0ff00ff0, "UQSUB8 Rd,Rn,Rm" },
  // UQSAX: unsigned saturating subtract and add with exchange, if Rd or Rn or Rm = 15 then unpredictable
  { IRC_SAFE,     armUqsaxInstruction, 0x06600f50, 0x0ff00ff0, "UQSAX Rd,Rn,Rm" },
  // REV (byte-reverse word), if Rd or Rm = 15 then unpredictable.
  { IRC_SAFE,     armRevInstruction,       0x06bf0f30, 0x0fff0ff0, "REV Rd,Rm" },
  // REV16 (byte-reverse packed halfword), if Rd or Rm = 15 then unpredictable.
  { IRC_SAFE,     armRev16Instruction,     0x06bf0fb0, 0x0fff0ff0, "REV16 Rd,Rm" },
  // REVSH (byte-reverse signed halfword), if Rd or Rm = 15 then unpredictable.
  { IRC_SAFE,     armRevshInstruction,     0x06ff0fb0, 0x0fff0ff0, "REVSH Rd,Rm" },
  // SXTH (sign-extend halfword), if Rd or Rm = 15 then unpredictable.
  { IRC_SAFE,     armSxthInstruction,      0x06bf0070, 0x0fff0ff0, "SXTH Rd,Rm" },
  { IRC_SAFE,     armSxthInstruction,      0x06bf0470, 0x0fff0ff0, "SXTH Rd,Rm,ROR #8" },
  { IRC_SAFE,     armSxthInstruction,      0x06bf0870, 0x0fff0ff0, "SXTH Rd,Rm,ROR #16" },
  { IRC_SAFE,     armSxthInstruction,      0x06bf0c70, 0x0fff0ff0, "SXTH Rd,Rm,ROR #24" },
  // SXTB16: sign-extend byte 16, if Rd or Rm = 15 then unpredictable.
  { IRC_SAFE,     armSxtb16Instruction,    0x068f0070, 0x0fff0ff0, "SXTB16 Rd,Rm" },
  { IRC_SAFE,     armSxtb16Instruction,    0x068f0470, 0x0fff0ff0, "SXTB16 Rd,Rm,ROR #8" },
  { IRC_SAFE,     armSxtb16Instruction,    0x068f0870, 0x0fff0ff0, "SXTB16 Rd,Rm,ROR #16" },
  { IRC_SAFE,     armSxtb16Instruction,    0x068f0c70, 0x0fff0ff0, "SXTB16 Rd,Rm,ROR #24" },
  // SXTB sign extend byte, if Rd or Rn = 15 then unpredictable.
  { IRC_SAFE,     armSxtbInstruction,      0x06af0070, 0x0fff0ff0, "SXTB Rd,Rm" },
  { IRC_SAFE,     armSxtbInstruction,      0x06af0470, 0x0fff0ff0, "SXTB Rd,Rm,ROR #8" },
  { IRC_SAFE,     armSxtbInstruction,      0x06af0870, 0x0fff0ff0, "SXTB Rd,Rm,ROR #16" },
  { IRC_SAFE,     armSxtbInstruction,      0x06af0c70, 0x0fff0ff0, "SXTB Rd,Rm,ROR #24" },
  // UXTH permitted, if Rd or Rn = 15 then unpredictable.
  { IRC_SAFE,     armUxthInstruction,      0x06ff0070, 0x0fff0ff0, "UXTH Rd,Rm" },
  { IRC_SAFE,     armUxthInstruction,      0x06ff0470, 0x0fff0ff0, "UXTH Rd,Rm,ROR #8" },
  { IRC_SAFE,     armUxthInstruction,      0x06ff0870, 0x0fff0ff0, "UXTH Rd,Rm,ROR #16" },
  { IRC_SAFE,     armUxthInstruction,      0x06ff0c70, 0x0fff0ff0, "UXTH Rd,Rm,ROR #24" },
  // UXTB16 permitted, if Rd or Rn = 15 then unpredictable.
  { IRC_SAFE,     armUxtb16Instruction,    0x06cf0070, 0x0fff0ff0, "UXTB16 Rd,Rm" },
  { IRC_SAFE,     armUxtb16Instruction,    0x06cf0470, 0x0fff0ff0, "UXTB16 Rd,Rm,ROR #8" },
  { IRC_SAFE,     armUxtb16Instruction,    0x06cf0870, 0x0fff0ff0, "UXTB16 Rd,Rm,ROR #16" },
  { IRC_SAFE,     armUxtb16Instruction,    0x06cf0c70, 0x0fff0ff0, "UXTB16 Rd,Rm,ROR #24" },
  // UXTB permitted, if Rd or Rn = 15 then unpredictable.
  { IRC_SAFE,     armUxtbInstruction,      0x06ef0070, 0x0fff0ff0, "UXTB Rd,Rm" },
  { IRC_SAFE,     armUxtbInstruction,      0x06ef0470, 0x0fff0ff0, "UXTB Rd,Rm,ROR #8" },
  { IRC_SAFE,     armUxtbInstruction,      0x06ef0870, 0x0fff0ff0, "UXTB Rd,Rm,ROR #16" },
  { IRC_SAFE,     armUxtbInstruction,      0x06ef0c70, 0x0fff0ff0, "UXTB Rd,Rm,ROR #24" },
  // SXTAH (sign-extend and add halfword), if Rd or Rm = 15 then unpredictable, if Rn = 15 see SXTH.
  { IRC_SAFE,     armSxtahInstruction,     0x06b00070, 0x0ff00ff0, "SXTAH Rd,Rn,Rm" },
  { IRC_SAFE,     armSxtahInstruction,     0x06b00470, 0x0ff00ff0, "SXTAH Rd,Rn,Rm,ROR #8" },
  { IRC_SAFE,     armSxtahInstruction,     0x06b00870, 0x0ff00ff0, "SXTAH Rd,Rn,Rm,ROR #16" },
  { IRC_SAFE,     armSxtahInstruction,     0x06b00c70, 0x0ff00ff0, "SXTAH Rd,Rn,Rm,ROR #24" },
  // SXTAB16 (sign-extend and add byte 16), if Rd or Rm = 15 then unpredictable, if Rn = 15 see SXTB16.
  { IRC_SAFE,     armSxtab16Instruction,   0x06800070, 0x0ff00ff0, "SXTAB16 Rd,Rn,Rm" },
  { IRC_SAFE,     armSxtab16Instruction,   0x06800470, 0x0ff00ff0, "SXTAB16 Rd,Rn,Rm,ROR #8" },
  { IRC_SAFE,     armSxtab16Instruction,   0x06800870, 0x0ff00ff0, "SXTAB16 Rd,Rn,Rm,ROR #16" },
  { IRC_SAFE,     armSxtab16Instruction,   0x06800c70, 0x0ff00ff0, "SXTAB16 Rd,Rn,Rm,ROR #24" },
  // SXTAB (sign-extend and add byte), if Rd or Rm = 15 then unpredictable, if Rn = 15 see SXTB.
  { IRC_SAFE,     armSxtabInstruction,     0x06a00070, 0x0ff00ff0, "SXTAB Rd,Rn,Rm" },
  { IRC_SAFE,     armSxtabInstruction,     0x06a00470, 0x0ff00ff0, "SXTAB Rd,Rn,Rm,ROR #8" },
  { IRC_SAFE,     armSxtabInstruction,     0x06a00870, 0x0ff00ff0, "SXTAB Rd,Rn,Rm,ROR #16" },
  { IRC_SAFE,     armSxtabInstruction,     0x06a00c70, 0x0ff00ff0, "SXTAB Rd,Rn,Rm,ROR #24" },
  // UXTAH (unsigned extend and add halfword), if Rd or Rm = 15 then unpredictable, if Rn = 15 see UXTH.
  { IRC_SAFE,     armUxtahInstruction,     0x06f00070, 0x0ff00ff0, "UXTAH Rd,Rn,Rm" },
  { IRC_SAFE,     armUxtahInstruction,     0x06f00470, 0x0ff00ff0, "UXTAH Rd,Rn,Rm,ROR #8" },
  { IRC_SAFE,     armUxtahInstruction,     0x06f00870, 0x0ff00ff0, "UXTAH Rd,Rn,Rm,ROR #16" },
  { IRC_SAFE,     armUxtahInstruction,     0x06f00c70, 0x0ff00ff0, "UXTAH Rd,Rn,Rm,ROR #24" },
  // UXTAB16 (unsigned extend and add byte 16), if Rd or Rm = 15 then unpredictable, if Rn = 15 see UXTB16.
  { IRC_SAFE,     armUxtab16Instruction,   0x06c00070, 0x0ff00ff0, "UXTAB16 Rd,Rn,Rm" },
  { IRC_SAFE,     armUxtab16Instruction,   0x06c00470, 0x0ff00ff0, "UXTAB16 Rd,Rn,Rm,ROR #8" },
  { IRC_SAFE,     armUxtab16Instruction,   0x06c00870, 0x0ff00ff0, "UXTAB16 Rd,Rn,Rm,ROR #16" },
  { IRC_SAFE,     armUxtab16Instruction,   0x06c00c70, 0x0ff00ff0, "UXTAB16 Rd,Rn,Rm,ROR #24" },
  // UXTAB (unsigned extend and add byte), if Rd or Rm = 15 then unpredictable, if Rn = 15 see UXTB.
  { IRC_SAFE,     armUxtabInstruction,     0x06e00070, 0x0ff00ff0, "UXTAB Rd,Rn,Rm" },
  { IRC_SAFE,     armUxtabInstruction,     0x06e00470, 0x0ff00ff0, "UXTAB Rd,Rn,Rm,ROR #8" },
  { IRC_SAFE,     armUxtabInstruction,     0x06e00870, 0x0ff00ff0, "UXTAB Rd,Rn,Rm,ROR #16" },
  { IRC_SAFE,     armUxtabInstruction,     0x06e00c70, 0x0ff00ff0, "UXTAB Rd,Rn,Rm,ROR #24" },
  // SEL (select bytes), if Rd or Rm or Rn = 15 then unpredictable.
  { IRC_SAFE,     armSelInstruction,       0x06800fb0, 0x0ff00ff0, "SEL Rd,Rn,Rm" },
  // SMUAD (signed dual multiply add), if Rd or Rm or Rn = 15 then unpredictable.
  { IRC_SAFE,     armSmuadInstruction,     0x0700f010, 0x0ff0f0d0, "SMUAD{X} Rd,Rn,Rm" },
  // SMUSD: signed dual multiply subtract, if Rd or Rn or Rm = 15 then unpredictable
  { IRC_SAFE,     armSmusdInstruction,     0x0700f050, 0x0ff0f0d0, "SMUSD{X} Rd,Rn,Rm" },
  // SMLAD: signed multiply accumulate dual, if Rd or Rn or Rm = 15 then unpredictable, Ra = 15 see SMUAD
  { IRC_SAFE,     armSmladInstruction,     0x07000010, 0x0ff000d0, "SMLAD{X} Rd,Rn,Rm,Ra" },
  // SMLALD: signed multiply accumulate long dual, if any R = 15 then unpredictable
  { IRC_SAFE,     armSmlaldInstruction,    0x07400010, 0x0ff000d0, "SMLALD{X} RdLo,RdHi,Rn,Rm" },
  // SMLSD: signed multiply subtract dual, if Rd or Rn or Rm = 15 then unpredictable, Ra = 15 see SMUSD
  { IRC_SAFE,     armSmlsdInstruction,     0x07000050, 0x0ff000d0, "SMLSD{X} Rd,Rn,Rm,Ra" },
  // SMLSLD: signed multiply subtract long dual, if any R = 15 then unpredictable
  { IRC_SAFE,     armSmlsldInstruction,    0x07400050, 0x0ff000d0, "SMLSLD{X} RdLo,RdHi,Rn,Rm" },
  // SMMUL: signed most significant word multiply, if Rd or Rn or Rm = 15 then unpredictable
  { IRC_SAFE,     armSmmulInstruction,     0x0750f010, 0x0ff0f0d0, "SMMUL{R} Rd,Rn,Rm" },
  // SMMLA: signed most significant word multiply accumulate, if Rd or Rn or Rm = 15 then unpredictable, Ra = 15 see SMMUL
  { IRC_SAFE,     armSmmlaInstruction,     0x07500010, 0x0ff000d0, "SMMLA{R} Rd,Rn,Rm,Ra" },
  // SMMLS: signed most significant word multiply subtract, if any R = 15 then unpredictable
  { IRC_SAFE,     armSmmlsInstruction,     0x075000d0, 0x0ff000d0, "SMMLS{R} Rd,Rn,Rm,Ra" },
  // SSAT,SSAT16,USAT,USAT16: (un)signed saturate, if Rd or Rn = 15 then unpredictable
  { IRC_SAFE,     armSsatInstruction,      0x06a00010, 0x0fe00ff0, "SSAT Rd,#sat_imm,Rn" },
  { IRC_SAFE,     armSsatInstruction,      0x06a00010, 0x0fe00070, "SSAT Rd,#sat_imm,Rn,LSL #imm" },
  { IRC_SAFE,     armSsatInstruction,      0x06a00050, 0x0fe00070, "SSAT Rd,#sat_imm,Rn,ASR #imm" },
  { IRC_SAFE,     armSsat16Instruction,    0x06a00f30, 0x0ff00ff0, "SSAT16 Rd,#imm,Rn" },
  { IRC_SAFE,     armUsatInstruction,      0x06e00010, 0x0fe00ff0, "USAT Rd,#sat_imm,Rn" },
  { IRC_SAFE,     armUsatInstruction,      0x06e00010, 0x0fe00070, "USAT Rd,#sat_imm,Rn,LSL #imm" },
  { IRC_SAFE,     armUsatInstruction,      0x06e00050, 0x0fe00070, "USAT Rd,#sat_imm,Rn,ASR #imm" },
  { IRC_SAFE,     armUsat16Instruction,    0x06e00f30, 0x0ff00ff0, "USAT16 Rd,#imm,Rn" },

  { IRC_REPLACE,  undefinedInstruction, 0x00000000, 0x00000000, "mediaInstructions" }
};

static struct instruction32bit armSvcCoprocInstructions[] =
{
  // well obviously.
  { IRC_REPLACE,  svcInstruction,       0x0f000000, 0x0f000000, "SWI code" },
  // Generic coprocessor instructions.
  { IRC_REPLACE,  armMrcInstruction,       0x0e100010, 0x0f100010, "MRC" },
  { IRC_REPLACE,  armMcrInstruction,       0x0e000010, 0x0f100010, "MCR" },
  { IRC_REPLACE,  undefinedInstruction,    0x00000000, 0x00000000, "svcCoprocInstructions" }
};

static struct instruction32bit armUnconditionalInstructions[] =
{
  // UNIMPLEMENTED: data memory barrier
  { IRC_REPLACE,  armDmbInstruction,       0xf57ff050, 0xfffffff0, "dmb\t%U" },
  // sync barriers
  { IRC_REPLACE,  armDsbInstruction,       0xf57ff040, 0xfffffff0, "DSB" },
  { IRC_REPLACE,  armIsbInstruction,       0xf57ff060, 0xfffffff0, "ISB" },
  // UNIMPLEMENTED: CLREX clear exclusive
  { IRC_REPLACE,  armClrexInstruction,     0xf57ff01f, 0xffffffff, "clrex" },
  // CPS: change processor state
  { IRC_REPLACE,  armCpsInstruction,       0xf1080000, 0xfffffe3f, "CPSIE" },
  { IRC_REPLACE,  armCpsInstruction,       0xf10a0000, 0xfffffe20, "CPSIE" },
  { IRC_REPLACE,  armCpsInstruction,       0xf10C0000, 0xfffffe3f, "CPSID" },
  { IRC_REPLACE,  armCpsInstruction,       0xf10e0000, 0xfffffe20, "CPSID" },
  { IRC_REPLACE,  armCpsInstruction,       0xf1000000, 0xfff1fe20, "CPS" },
  // UNIMPLEMENTED: RFE return from exception
  { IRC_REPLACE,  armRfeInstruction,       0xf8100a00, 0xfe50ffff, "rfe%23?id%24?ba\t%16-19r%21'!" },
  // UNIMPLEMENTED: SETEND set endianess
  { IRC_REPLACE,  armSetendInstruction,    0xf1010000, 0xfffffc00, "setend\t%9?ble" },
  // UNIMPLEMENTED: SRS store return state
  { IRC_REPLACE,  armSrsInstruction,       0xf84d0500, 0xfe5fffe0, "srs%23?id%24?ba\t%16-19r%21'!, #%0-4d" },
  // BLX: branch and link to Thumb subroutine
  { IRC_REPLACE,  armBlxImmediateInstruction, 0xfa000000, 0xfe000000, "BLX #imm24" },
  // PLD: preload data
  { IRC_REPLACE,  armPldInstruction,          0xf450f000, 0xfc70f000, "PLD" },
  // PLI: preload instruction
  { IRC_REPLACE,  armPliInstruction,          0xf450f000, 0xfd70f000, "PLI" },
  { IRC_REPLACE,  undefinedInstruction,       0x00000000, 0x00000000, "armUnconditionalInstructions" }
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
