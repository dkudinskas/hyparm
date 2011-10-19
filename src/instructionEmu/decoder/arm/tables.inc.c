/*
 * ARM decoding tables for the table search decoder
 */


static struct instruction32bit armBranchBlockTransferInstructions[] =
{
  // STM traps if ^ postfix, otherwise pass through
  { TRUE,  &armStmInstruction,    0x08400000, 0x0e500000, "STM {regList}^" },
  { FALSE, &armStmInstruction,    0x08800000, 0x0ff00000, "STMIA {regList}" },
  { FALSE, &armStmInstruction,    0x08000000, 0x0e100000, "STM {regList}" },
  // POP LDM syntax, only care if PC in reglist
  { TRUE,  &armLdmInstruction,    0x08bd8000, 0x0fff8000, "LDM SP, {...r15}" },
  { FALSE, &armLdmInstruction,    0x08bd0000, 0x0fff0000, "LDM SP, {reglist}" },
  // LDM traps if ^ postfix and/or PC is in register list, otherwise pass through
  { TRUE,  &armLdmInstruction,    0x08500000, 0x0e500000, "LDM Rn, {regList}^" },
  { TRUE,  &armLdmInstruction,    0x08108000, 0x0e108000, "LDM Rn, {..r15}" },
  { FALSE, &armLdmInstruction,    0x08900000, 0x0f900000, "LDMIA Rn, {regList}" },
  { FALSE, &armLdmInstruction,    0x08100000, 0x0e100000, "LDM Rn, {regList}" },
  // B/BL: always hypercall! obviously.
  { TRUE,  &armBInstruction,      0x0a000000, 0x0e000000, "BRANCH" },
  { TRUE,  &undefinedInstruction, 0x00000000, 0x00000000, "branchBlockTransferInstructions" }
};

// register/shifter register cases, etc
static struct instruction32bit armDataProcMiscInstructions_op0[] =
{
  // NOP is just a nop...
  { FALSE, &nopInstruction,       0xe1a00000, 0xffffffff, "NOP" },
  // UNIMPLEMENTED: SWP swap
  { TRUE,  &armSwpInstruction,       0x01000090, 0x0fb00ff0, "SWP" },
  // UNIMPLEMENTED:
  { TRUE,  &armStrhtInstruction,  0x006000b0, 0x0f7000f0, "STRHT instruction" },
  { TRUE,  &armLdrhtInstruction,  0x003000b0, 0x0f3000f0, "LDRHT instruction" },
  // store and load exclusive: must be emulated - user mode faults
  { TRUE,  &armLdrexbInstruction, 0x01d00f9f, 0x0ff00fff, "LDREXB" },
  { TRUE,  &armLdrexdInstruction, 0x01b00f9f, 0x0ff00fff, "LDREXD" },
  { TRUE,  &armLdrexhInstruction, 0x01f00f9f, 0x0ff00fff, "LDREXH" },
  { TRUE,  &armStrexbInstruction, 0x01c00f90, 0x0ff00ff0, "STREXB" },
  { TRUE,  &armStrexdInstruction, 0x01a00f90, 0x0ff00ff0, "STREXD" },
  { TRUE,  &armStrexhInstruction, 0x01e00f90, 0x0ff00ff0, "STREXH" },
  { TRUE,  &armLdrexInstruction,  0x01900f9f, 0x0ff00fff, "LDREX" },
  { TRUE,  &armStrexInstruction,  0x01800f90, 0x0ff00ff0, "STREX" },
  // SMULL - signed multiply, PC cannot be used as any destination
  { FALSE, &armSmullInstruction,    0x00800090, 0x0fa000f0, "SMULL" },
  // SMLAL - signed multiply and accumulate, PC cannot be used as any destination
  { FALSE, &armSmlalInstruction,    0x00a00090, 0x0fa000f0, "SMLAL" },
  // MUL: Rd = Rm * Rn; Rd != PC. pass through
  { FALSE, &armMulInstruction,       0x00000090, 0x0fe000f0, "MUL Rd, Rm, Rn" },
  // MLA: Rd = Rm * Rn + Ra; Rd != PC. pass through
  { FALSE, &armMlaInstruction,       0x00200090, 0x0fe000f0, "MLA Rd, Rm, Rn, Ra" },
  // MLS: Rd = Rm * Rn - Ra; Rd != PC, pass through
  { FALSE, &armMlsInstruction,       0x00600090, 0x0ff000f0, "MLS Rd, Rm, Rn, Ra" },
  // Branch and try to exchange to ARM mode.
  { TRUE,  &armBxInstruction,     0x012FFF10, 0x0ffffff0, "bx%c\t%0-3r" },
  // Branch and link and try to exchange to Jazelle mode.
  { TRUE,  &armBxjInstruction,    0x012fff20, 0x0ffffff0, "BXJ Rm" },
  // Software breakpoint instruction... not sure yet.
  { TRUE,  &armBkptInstruction,      0xe1200070, 0xfff000f0, "BKPT #imm8" },
  // UNIMPLEMENTED: SMC, previously SMI: secure monitor call
  { TRUE,  &armSmcInstruction,       0x01600070, 0x0ff000f0, "smc%c\t%e" },
  // Branch and link and try to exchange with Thumb mode.
  { TRUE,  &armBlxRegisterInstruction,       0x012fff30, 0x0ffffff0, "BLX Rm" },
  // CLZ: Count leading zeroes - Rd, Rm != PC, pass through
  { FALSE, &armClzInstruction,       0x016f0f10, 0x0fff0ff0, "CLZ Rd, Rm" },
  // Q(D){ADD,SUB}: saturated add/subtract/doubleAdd/doubleSubtract, unpredictable when any R=PC
  { FALSE, &armQaddInstruction,      0x01000050, 0x0ff00ff0, "qadd%c\t%12-15r, %0-3r, %16-19r" },
  { FALSE, &armQdaddInstruction,     0x01400050, 0x0ff00ff0, "qdadd%c\t%12-15r, %0-3r, %16-19r" },
  { FALSE, &armQsubInstruction,      0x01200050, 0x0ff00ff0, "qsub%c\t%12-15r, %0-3r, %16-19r" },
  { FALSE, &armQdsubInstruction,     0x01600050, 0x0ff00ff0, "qdsub%c\t%12-15r, %0-3r, %16-19r" },
  // LDRD: Rt1 must be even numbered and NOT 14, thus Rt2 cannot be PC. pass.
  { FALSE, &armLdrdInstruction,   0x004000d0, 0x0e5000f0, "LDRD Rt, [Rn, #imm]" },
  { FALSE, &armLdrdInstruction,   0x000000d0, 0x0e500ff0, "LDRD Rt, [Rn, Rm]" },
  // STRD: pass through, let them fail!
  { FALSE, &armStrdInstruction,   0x004000f0, 0x0e5000f0, "STRD Rt, [Rn, #imm]" },
  { FALSE, &armStrdInstruction,   0x000000f0, 0x0e500ff0, "STRD Rt, [Rn, Rm]" },
  // signed 16 bit multiply, 32 bit accumulate, unpredictable if any R=PC
  { FALSE, &armSmlabbInstruction,    0x01000080, 0x0ff000f0, "smlabb%c\t%16-19r, %0-3r, %8-11r, %12-15r" },
  { FALSE, &armSmlatbInstruction,    0x010000a0, 0x0ff000f0, "smlatb%c\t%16-19r, %0-3r, %8-11r, %12-15r" },
  { FALSE, &armSmlabtInstruction,    0x010000c0, 0x0ff000f0, "smlabt%c\t%16-19r, %0-3r, %8-11r, %12-15r" },
  { FALSE, &armSmlattInstruction,    0x010000e0, 0x0ff000f0, "smlatt%c\t%16-19r, %0-3r, %8-11r, %12-15r" },
  // signed 16 bit x 32 bit multiply, 32 bit accumulate, unpredictable if any R=PC
  { FALSE, &armSmlawbInstruction,    0x01200080, 0x0ff000f0, "smlawb%c\t%16-19r, %0-3r, %8-11r, %12-15r" },
  { FALSE, &armSmlawtInstruction,    0x012000c0, 0x0ff000f0, "smlawt%c\t%16-19r, %0-3r, %8-11r, %12-15r" },
  // signed 16 bit multiply, 64 bit accumulate, unpredictable if any R=PC
  { FALSE, &armSmlalbbInstruction,   0x01400080, 0x0ff000f0, "smlalbb%c\t%12-15r, %16-19r, %0-3r, %8-11r" },
  { FALSE, &armSmlaltbInstruction,   0x014000a0, 0x0ff000f0, "smlaltb%c\t%12-15r, %16-19r, %0-3r, %8-11r" },
  { FALSE, &armSmlalbtInstruction,   0x014000c0, 0x0ff000f0, "smlalbt%c\t%12-15r, %16-19r, %0-3r, %8-11r" },
  { FALSE, &armSmlalttInstruction,   0x014000e0, 0x0ff000f0, "smlaltt%c\t%12-15r, %16-19r, %0-3r, %8-11r" },
  // signed 16 bit multiply, 32 bit result, any R=PC is unpredictable
  { FALSE, &armSmulbbInstruction,    0x01600080, 0x0ff0f0f0, "SMULBB Rd, Rn, RM" },
  { FALSE, &armSmultbInstruction,    0x016000a0, 0x0ff0f0f0, "smultb%c\t%16-19r, %0-3r, %8-11r" },
  { FALSE, &armSmulbtInstruction,    0x016000c0, 0x0ff0f0f0, "smulbt%c\t%16-19r, %0-3r, %8-11r" },
  { FALSE, &armSmulttInstruction,    0x016000e0, 0x0ff0f0f0, "smultt%c\t%16-19r, %0-3r, %8-11r" },
  // signed 16 bit x 32 bit multiply, 32 bit result, any R=PC is unpredictable
  { FALSE, &armSmulwbInstruction,    0x012000a0, 0x0ff0f0f0, "smulwb%c\t%16-19r, %0-3r, %8-11r" },
  { FALSE, &armSmulwtInstruction,    0x012000e0, 0x0ff0f0f0, "smulwt%c\t%16-19r, %0-3r, %8-11r" },
  // STRH: passthrough, will data abort if something wrong
  { FALSE, &armStrhInstruction,   0x004000b0, 0x0e5000f0, "STRH Rt, [Rn, +-imm8]" },
  { FALSE, &armStrhInstruction,   0x000000b0, 0x0e500ff0, "STRH Rt, [Rn], +-Rm" },
  // LDRH cant load halfword to PC, passthrough
  { FALSE, &armLdrhInstruction,   0x00500090, 0x0e500090, "LDRH Rt, [Rn, +-imm8]" },
  { FALSE, &armLdrhInstruction,   0x00100090, 0x0e500f90, "LDRH Rt, [Rn], +-Rm" },
  // AND: Rd = PC end block, others are fine
  { TRUE,  &armAndInstruction,       0x0000f000, 0x0fe0f010, "AND PC, Rn, Rm, #shamt" },
  { TRUE,  &armAndInstruction,       0x0000f010, 0x0fe0f090, "AND PC, Rn, Rm, Rshamt" },
  { FALSE, &armAndInstruction,       0x00000000, 0x0fe00010, "AND Rd, Rn, Rm, #shamt" },
  { FALSE, &armAndInstruction,       0x00000010, 0x0fe00090, "AND Rd, Rn, Rm, Rshamt" },
  // EOR: Rd = PC end block, others are fine
  { TRUE,  &armEorInstruction,       0x0020f000, 0x0fe0f010, "EOR PC, Rn, Rm, #shamt" },
  { TRUE,  &armEorInstruction,       0x0020f010, 0x0fe0f090, "EOR PC, Rn, Rm, Rshamt" },
  { FALSE, &armEorInstruction,       0x00200000, 0x0fe00010, "EOR Rd, Rn, Rm, #shamt" },
  { FALSE, &armEorInstruction,       0x00200010, 0x0fe00090, "EOR Rd, Rn, Rm, Rshamt" },
  // SUB: Rd = PC end block, others are fine
  { TRUE,  &armSubInstruction,       0x0040f000, 0x0fe0f010, "SUB PC, Rn, Rm, #shamt" },
  { TRUE,  &armSubInstruction,       0x0040f010, 0x0fe0f090, "SUB PC, Rn, Rm, Rshamt" },
  { FALSE, &armSubInstruction,       0x00400000, 0x0fe00010, "SUB Rd, Rn, Rm, #shamt" },
  { FALSE, &armSubInstruction,       0x00400010, 0x0fe00090, "SUB Rd, Rn, Rm, Rshamt" },
  // RSB: Rd = PC end block, others are fine
  { TRUE,  &armRsbInstruction,       0x0060f000, 0x0fe0f010, "RSB PC, Rn, Rm, #shamt" },
  { TRUE,  &armRsbInstruction,       0x0060f010, 0x0fe0f090, "RSB PC, Rn, Rm, Rshamt" },
  { FALSE, &armRsbInstruction,       0x00600000, 0x0fe00010, "RSB Rd, Rn, Rm, #shamt" },
  { FALSE, &armRsbInstruction,       0x00600010, 0x0fe00090, "RSB Rd, Rn, Rm, Rshamt" },
  // ADD: Rd = PC end block, others are fine
  { TRUE,  &armAddInstruction,       0x0080f000, 0x0fe0f010, "ADD PC, Rn, Rm, #shamt" },
  { TRUE,  &armAddInstruction,       0x0080f010, 0x0fe0f090, "ADD PC, Rn, Rm, Rshamt" },
  { FALSE, &armAddInstruction,       0x00800000, 0x0fe00010, "ADD Rd, Rn, Rm, #shamt" },
  { FALSE, &armAddInstruction,       0x00800010, 0x0fe00090, "ADD Rd, Rn, Rm, Rshamt" },
  // ADC: Rd = PC end block, others are fine
  { TRUE,  &armAdcInstruction,       0x00a0f000, 0x0fe0f010, "ADC PC, Rn, Rm, #shamt" },
  { TRUE,  &armAdcInstruction,       0x00a0f010, 0x0fe0f090, "ADC PC, Rn, Rm, Rshamt" },
  { FALSE, &armAdcInstruction,       0x00a00000, 0x0fe00010, "ADC Rd, Rn, Rm, #shamt" },
  { FALSE, &armAdcInstruction,       0x00a00010, 0x0fe00090, "ADC Rd, Rn, Rm, Rshamt" },
  // SBC: Rd = PC end block, others are fine
  { TRUE,  &armSbcInstruction,       0x00c0f000, 0x0fe0f010, "SBC Rd, Rn, Rm, #shamt" },
  { TRUE,  &armSbcInstruction,       0x00c0f010, 0x0fe0f090, "SBC Rd, Rn, Rm, Rshamt" },
  { FALSE, &armSbcInstruction,       0x00c00000, 0x0fe00010, "SBC Rd, Rn, Rm, #shamt" },
  { FALSE, &armSbcInstruction,       0x00c00010, 0x0fe00090, "SBC Rd, Rn, Rm, Rshamt" },
  // RSC: Rd = PC end block, others are fine
  { TRUE,  &armRscInstruction,       0x00e0f000, 0x0fe0f010, "RSC PC, Rn, Rm, #shamt" },
  { TRUE,  &armRscInstruction,       0x00e0f010, 0x0fe0f090, "RSC PC, Rn, Rm, Rshamt" },
  { FALSE, &armRscInstruction,       0x00e00000, 0x0fe00010, "RSC Rd, Rn, Rm, #shamt" },
  { FALSE, &armRscInstruction,       0x00e00010, 0x0fe00090, "RSC Rd, Rn, Rm, Rshamt" },
  // MSR/MRS: always hypercall! we must hide the real state from guest.
  { TRUE,  &armMsrInstruction,       0x0120f000, 0x0fb0fff0, "MSR, s/cpsr, Rn" },
  { TRUE,  &armMrsInstruction,       0x010f0000, 0x0fbf0fff, "MRS, Rn, s/cpsr" },
  // TST instructions are all fine
  { FALSE, &armTstInstruction,       0x01000000, 0x0fe00010, "TST Rn, Rm, #shamt" },
  { FALSE, &armTstInstruction,       0x01000010, 0x0fe00090, "TST Rn, Rm, Rshift" },
  // TEQ instructions are all fine
  { FALSE, &armTeqInstruction,       0x01200000, 0x0fe00010, "TEQ Rn, Rm, #shamt" },
  { FALSE, &armTeqInstruction,       0x01200010, 0x0fe00090, "TEQ Rn, Rm, Rshift" },
  // CMP instructions are all fine
  { FALSE, &armCmpInstruction,       0x01400000, 0x0fe00010, "CMP Rn, Rm, #shamt" },
  { FALSE, &armCmpInstruction,       0x01400010, 0x0fe00090, "CMP Rn, Rm, Rshamt" },
  // CMN instructions are all fine
  { FALSE, &armCmnInstruction,       0x01600000, 0x0fe00010, "CMN Rn, Rm, #shamt" },
  { FALSE, &armCmnInstruction,       0x01600010, 0x0fe00090, "CMN Rn, Rm, Rshamt" },
  // ORR: Rd = PC end block, other are fine
  { TRUE,  &armOrrInstruction,       0x0180f000, 0x0fe0f010, "ORR PC, Rn, Rm, #shamt" },
  { TRUE,  &armOrrInstruction,       0x0180f010, 0x0fe0f090, "ORR PC, Rn, Rm, Rshamt" },
  { FALSE, &armOrrInstruction,       0x01800000, 0x0fe00010, "ORR Rd, Rn, Rm, #shamt" },
  { FALSE, &armOrrInstruction,       0x01800010, 0x0fe00090, "ORR Rd, Rn, Rm, Rshamt" },
  // MOV with Rd = PC end block. MOV reg, <shifted reg> is a pseudo instr for shift instructions
  { TRUE,  &armMovInstruction,       0x01a0f000, 0x0deffff0, "MOV PC, Rm" },
  { FALSE, &armMovInstruction,       0x01a00000, 0x0def0ff0, "MOV Rn, Rm" },
  // LSL: Rd = PC and shamt = #imm, then end block. Otherwise fine
  { TRUE,  &armLslInstruction,       0x01a0f000, 0x0feff070, "LSL Rd, Rm, #shamt" },
  { TRUE,  &armLslInstruction,       0x01a0f010, 0x0feff0f0, "LSL Rd, Rm, Rshamt" },
  { FALSE, &armLslInstruction,       0x01a00000, 0x0fef0070, "LSL Rd, Rm, #shamt" },
  { FALSE, &armLslInstruction,       0x01a00010, 0x0fef00f0, "LSL Rd, Rm, Rshamt" },
  // LSR: Rd = PC and shamt = #imm, then end block. Otherwise fine
  { TRUE,  &armLsrInstruction,       0x01a0f020, 0x0feff070, "LSR Rd, Rm, #shamt" },
  { TRUE,  &armLsrInstruction,       0x01a0f030, 0x0feff0f0, "LSR Rd, Rm, Rshamt" },
  { FALSE, &armLsrInstruction,       0x01a00020, 0x0fef0070, "LSR Rd, Rm, #shamt" },
  { FALSE, &armLsrInstruction,       0x01a00030, 0x0fef00f0, "LSR Rd, Rm, Rshamt" },
  // ASR: Rd = PC and shamt = #imm, then end block. Otherwise fine
  { TRUE,  &armAsrInstruction,       0x01a0f040, 0x0feff070, "ASR Rd, Rm, #shamt" },
  { TRUE,  &armAsrInstruction,       0x01a0f050, 0x0feff0f0, "ASR Rd, Rm, Rshamt" },
  { FALSE, &armAsrInstruction,       0x01a00040, 0x0fef0070, "ASR Rd, Rm, #shamt" },
  { FALSE, &armAsrInstruction,       0x01a00050, 0x0fef00f0, "ASR Rd, Rm, Rshamt" },
  // RRX: shift right and extend, Rd can be PC
  { TRUE,  &armRrxInstruction,       0x01a0f060, 0x0feffff0, "RRX PC, Rm" },
  { FALSE, &armRrxInstruction,       0x01a00060, 0x0fef0ff0, "RRX Rd, Rm" },
  // ROR: reg case destination unpredictable. imm case dest can be PC.
  { FALSE, &armRorInstruction,       0x01a00070, 0x0fef00f0, "ROR Rd, Rm, Rn" },
  { TRUE,  &armRorInstruction,       0x01a0f060, 0x0feff070, "ROR PC, Rm, #imm" },
  { FALSE, &armRorInstruction,       0x01a00060, 0x0fef0070, "ROR Rd, Rm, #imm" },
  // BIC with Rd = PC end block, other are fine.
  { TRUE,  &armBicInstruction,       0x01c0f000, 0x0fe0f010, "BIC PC, Rn, Rm, #shamt" },
  { TRUE,  &armBicInstruction,       0x01c0f010, 0x0fe0f090, "BIC PC, Rn, Rm, Rshamt" },
  { FALSE, &armBicInstruction,       0x01c00000, 0x0fe00010, "BIC Rd, Rn, Rm, #shamt" },
  { FALSE, &armBicInstruction,       0x01c00010, 0x0fe00090, "BIC Rd, Rn, Rm, Rshamt" },
  // MVN with Rd = PC end block, other are fine.
  { TRUE,  &armMvnInstruction,       0x01e0f000, 0x0fe0f010, "MVN Rd, Rm, #shamt" },
  { TRUE,  &armMvnInstruction,       0x01e0f010, 0x0fe0f090, "MVN Rd, Rm, Rshamt" },
  { FALSE, &armMvnInstruction,       0x01e00000, 0x0fe00010, "MVN Rd, Rm, #shamt" },
  { FALSE, &armMvnInstruction,       0x01e00010, 0x0fe00090, "MVN Rd, Rm, Rshamt" },

  { TRUE,  &undefinedInstruction, 0x00000000, 0x00000000, "dataProcMiscInstructions_op0" }
};

// immediate cases and random hints
static struct instruction32bit armDataProcMiscInstructions_op1[] =
{
  // UNIMPLEMENTED: yield hint
  { TRUE,  &armYieldInstruction,     0x0320f001, 0x0fffffff, "yield%c" },
  // UNIMPLEMENTED: wait for event hint
  { TRUE,  &armWfeInstruction,       0x0320f002, 0x0fffffff, "wfe%c" },
  // UNIMPLEMENTED: wait for interrupt hint
  { TRUE,  &armWfiInstruction,       0x0320f003, 0x0fffffff, "wfi%c" },
  // UNIMPLEMENTED: send event hint
  { TRUE,  &armSevInstruction,       0x0320f004, 0x0fffffff, "sev%c" },
  { FALSE, &nopInstruction,       0x0320f000, 0x0fffff00, "NOP" },
  // UNIMPLEMENTED: debug hint
  { TRUE,  &armDbgInstruction,       0x0320f0f0, 0x0ffffff0, "dbg%c\t#%0-3d" },
  // MOVW - indication of 'wide' - to select ARM encoding. Rd cant be PC, pass through.
  { FALSE, &armMovwInstruction,      0x03000000, 0x0ff00000, "MOVW Rd, Rn" },
  { FALSE, &armMovtInstruction,      0x03400000, 0x0ff00000, "MOVT Rd, Rn" },
  // AND: Rd = PC end block, others are fine
  { TRUE,  &armAndInstruction,       0x0200f000, 0x0fe0f000, "AND PC, Rn, #imm" },
  { FALSE, &armAndInstruction,       0x02000000, 0x0fe00000, "AND Rd, Rn, #imm" },
  // EOR: Rd = PC end block, others are fine
  { TRUE,  &armEorInstruction,       0x0220f000, 0x0fe0f000, "EOR PC, Rn, Rm/#imm" },
  { FALSE, &armEorInstruction,       0x02200000, 0x0fe00000, "EOR Rd, Rn, #imm" },
  // SUB: Rd = PC end block, others are fine
  { TRUE,  &armSubInstruction,       0x0240f000, 0x0fe0f000, "SUB PC, Rn, Rm/imm" },
  { FALSE, &armSubInstruction,       0x02400000, 0x0fe00000, "SUB Rd, Rn, #imm" },
  // RSB: Rd = PC end block, others are fine
  { TRUE,  &armRsbInstruction,       0x0260f000, 0x0fe0f000, "RSB PC, Rn, Rm/imm" },
  { FALSE, &armRsbInstruction,       0x02600000, 0x0fe00000, "RSB Rd, Rn, #imm" },
  // ADD: Rd = PC end block, others are fine
  { TRUE,  &armAddInstruction,       0x0280f000, 0x0fe0f000, "ADD PC, Rn, #imm" },
  { FALSE, &armAddInstruction,       0x02800000, 0x0fe00000, "ADD Rd, Rn, #imm" },
  // ADC: Rd = PC end block, others are fine
  { TRUE,  &armAdcInstruction,       0x02a0f000, 0x0fe0f000, "ADC PC, Rn/#imm" },
  { FALSE, &armAdcInstruction,       0x02a00000, 0x0fe00000, "ADC Rd, Rn, #imm" },
  // SBC: Rd = PC end block, others are fine
  { TRUE,  &armSbcInstruction,       0x02c0f000, 0x0fe0f000, "SBC PC, Rn/#imm" },
  { FALSE, &armSbcInstruction,       0x02c00000, 0x0fe00000, "SBC Rd, Rn, #imm" },
  // RSC: Rd = PC end block, others are fine
  { TRUE,  &armRscInstruction,       0x02e0f000, 0x0fe0f000, "RSC PC, Rn/#imm" },
  { FALSE, &armRscInstruction,       0x02e00000, 0x0fe00000, "RSC Rd, Rn, #imm" },
  // MSR: always hypercall! we must hide the real state from guest.
  { TRUE,  &armMsrInstruction,       0x0320f000, 0x0fb0f000, "MSR, s/cpsr, #imm" },
  // TST instructions are all fine
  { FALSE, &armTstInstruction,       0x03000000, 0x0fe00000, "TST Rn, #imm" },
  // TEQ instructions are all fine
  { FALSE, &armTeqInstruction,       0x03200000, 0x0fe00000, "TEQ Rn, #imm" },
  // CMP instructions are all fine
  { FALSE, &armCmpInstruction,       0x03400000, 0x0fe00000, "CMP Rn, #imm" },
  // CMN instructions are all fine
  { FALSE, &armCmnInstruction,       0x03600000, 0x0fe00000, "CMN Rn, #imm" },
  // ORR: Rd = PC end block, other are fine
  { TRUE,  &armOrrInstruction,       0x0380f000, 0x0fe0f000, "ORR Rd, Rn, #imm" },
  { FALSE, &armOrrInstruction,       0x03800000, 0x0fe00000, "ORR Rd, Rn, #imm" },
  // MOV with Rd = PC end block. MOV <shifted reg> is a pseudo instr..
  { TRUE,  &armMovInstruction,       0x03a0f000, 0x0feff000, "MOV PC, #imm" },
  { FALSE, &armMovInstruction,       0x03a00000, 0x0fef0000, "MOV Rn, #imm" },
  // BIC with Rd = PC end block, other are fine.
  { TRUE,  &armBicInstruction,       0x03c0f000, 0x0fe0f000, "BIC PC, Rn, #imm" },
  { FALSE, &armBicInstruction,       0x03c00000, 0x0fe00000, "BIC Rd, Rn, #imm" },
  // MVN with Rd = PC end block, other are fine.
  { TRUE,  &armMvnInstruction,       0x03e0f000, 0x0fe0f000, "MVN PC, Rn, #imm" },
  { FALSE, &armMvnInstruction,       0x03e00000, 0x0fe00000, "MVN Rd, Rn, #imm" },

  { TRUE,  &undefinedInstruction, 0x00000000, 0x00000000, "dataProcMiscInstructions_op1" }
};

static struct instruction32bit armLoadStoreWordByteInstructions[] =
{
  // STR imm12 and reg are pass-through
  { FALSE, &armStrInstruction,    0x04000000, 0x0e100000, "STR Rt, [Rn, +-imm12]" },
  { FALSE, &armStrInstruction,    0x06000000, 0x0e100ff0, "STR Rt, [Rn], +-Rm" },
  { FALSE, &armStrInstruction,    0x04000000, 0x0c100010, "STR any? dont get this." },
  { FALSE, &armStrbInstruction,   0x04400000, 0x0e500000, "STRB Rt, [Rn, +-imm12]" },
  { FALSE, &armStrbInstruction,   0x06400000, 0x0e500010, "STRB Rt, [Rn], +-Rm" },
  // LDR traps if dest = PC, otherwise pass through
  { TRUE,  &armLdrInstruction,    0x0410f000, 0x0c10f000, "LDR PC, Rn/#imm12" },
  { FALSE, &armLdrInstruction,    0x04100000, 0x0c100000, "LDR Rd, Rn/#imm12" },
  { TRUE,  &undefinedInstruction, 0x00000000, 0x00000000, "loadStoreWordByteInstructions" }
};

static struct instruction32bit armMediaInstructions[] =
{
  // BFC: bit field clear, dest PC not allowed.
  { FALSE, &armBfcInstruction,       0x07c0001f, 0x0fe0007f, "BFC Rd, #LSB, #width" },
  // BFI: bit field insert, dest PC not allowed.
  { FALSE, &armBfiInstruction,       0x07c00010, 0x0fe00070, "BFI Rd, #LSB, #width" },
  // RBIT: reverse bits, if Rd or Rm = 15 then unpredictable.
  { FALSE, &armRbitInstruction,      0x06ff0f30, 0x0fff0ff0, "RBIT Rd,Rm" },
  // UBFX: extract bit field - destination 15 unpredictable
  { FALSE, &armUbfxInstruction,     0x07a00050, 0x0fa00070, "UBFX Rd, Rn, width" },
  // PKH (pack halfword), if Rd or Rn or Rm = 15 then unpredictable
  { FALSE, &armPkhbtInstruction,     0x06800010, 0x0ff00ff0, "PKHBT Rd,Rn,Rm" },
  { FALSE, &armPkhbtInstruction,     0x06800010, 0x0ff00070, "PKHBT Rd,Rn,Rm,LSL #imm" },
  { FALSE, &armPkhtbInstruction,     0x06800050, 0x0ff00ff0, "PKHTB Rd,Rn,Rm,ASR #32" },
  { FALSE, &armPkhtbInstruction,     0x06800050, 0x0ff00070, "PKHTB Rd,Rn,Rm,ASR #imm" },
  // QADD8,QADD16: saturating add 16 bit and 8 bit, if Rd or Rn or Rm = 15 then unpredictable
  { FALSE, &armQadd16Instruction,    0x06200f10, 0x0ff00ff0, "QADD16 Rd,Rn,Rm" },
  { FALSE, &armQadd8Instruction,     0x06200f90, 0x0ff00ff0, "QADD8 Rd,Rn,Rm" },
  // QASX: saturating add and subtract with exchange if Rd or Rn or Rm = 15 then unpredictable
  { FALSE, &armQasxInstruction,  0x06200f30, 0x0ff00ff0, "QASX Rd,Rn,Rm" },
  // QSUB8,QSUB16: saturating subtract 16 and 8 bit if Rd or Rn or Rm = 15 then unpredictable
  { FALSE, &armQsub16Instruction,    0x06200f70, 0x0ff00ff0, "QSUB16 Rd,Rn,Rm" },
  { FALSE, &armQsub8Instruction,     0x06200ff0, 0x0ff00ff0, "QSUB8 Rd,Rn,Rm" },
  // QSAX: saturating subtract and add with exchange if Rd or Rn or Rm = 15 then unpredictable
  { FALSE, &armQsaxInstruction,  0x06200f50, 0x0ff00ff0, "QSAX Rd,Rn,Rm" },
  // SADD8,SADD16: signed add 16 bit and 8 bit, if Rd or Rn or Rm = 15 then unpredictable
  { FALSE, &armSadd16Instruction,    0x06100f10, 0x0ff00ff0, "SADD16 Rd,Rn,Rm" },
  { FALSE, &armSadd8Instruction,     0x06100f90, 0x0ff00ff0, "SADD8 Rd,Rn,Rm" },
  // SASX: signed add and subtract with exchange, if Rd or Rn or Rm = 15 then unpredictable
  { FALSE, &armSasxInstruction,  0x06100f30, 0x0ff00ff0, "SASX Rd,Rn,Rm" },
  // SSUB8,SSUB16: signed subtract 16 bit and 8 bit, if Rd or Rn or Rm = 15 then unpredictable
  { FALSE, &armSsub16Instruction,    0x06100f70, 0x0ff00ff0, "SSUB16 Rd,Rn,Rm" },
  { FALSE, &armSsub8Instruction,     0x06100ff0, 0x0ff00ff0, "SSUB8 Rd,Rn,Rm" },
  // SSAX: signed subtract and add with exchange,if Rd or Rn or Rm = 15 then unpredictable
  { FALSE, &armSsaxInstruction,  0x06100f50, 0x0ff00ff0, "SSAX Rd,Rn,Rm" },
  // SHADD8,SHADD16: signed halvign add 16 bit and 8 bit, if Rd or Rn or Rm = 15 then unpredictable
  { FALSE, &armShadd16Instruction,   0x06300f10, 0x0ff00ff0, "SHADD16 Rd,Rn,Rm" },
  { FALSE, &armShadd8Instruction,    0x06300f90, 0x0ff00ff0, "SHADD8 Rd,Rn,Rm" },
  // SHASX: signed halving add and subtract with exchange, if Rd or Rn or Rm = 15 then unpredictable
  { FALSE, &armShasxInstruction, 0x06300f30, 0x0ff00ff0, "SHASX Rd,Rn,Rm" },
  // SHSUB8,SHSUB16: signed halvign subtract 16 bit and 8 bit, if Rd or Rn or Rm = 15 then unpredictable
  { FALSE, &armShsub16Instruction,   0x06300f70, 0x0ff00ff0, "SHSUB16 Rd,Rn,Rm" },
  { FALSE, &armShsub8Instruction,    0x06300ff0, 0x0ff00ff0, "SHSUB8 Rd,Rn,Rm" },
  // SHSAX: signed halving subtract and add with exchange, if Rd or Rn or Rm = 15 then unpredictable
  { FALSE, &armShsaxInstruction, 0x06300f50, 0x0ff00ff0, "SHSAX Rd,Rn,Rm" },
  // UADD8,UADD16: unsigned add 16 bit and 8 bit, if Rd or Rn or Rm = 15 then unpredictable
  { FALSE, &armUadd16Instruction,    0x06500f10, 0x0ff00ff0, "UADD16 Rd,Rn,Rm" },
  { FALSE, &armUadd8Instruction,     0x06500f90, 0x0ff00ff0, "UADD8 Rd,Rn,Rm" },
  // UASX: unsigned add and subtract with exchange, if Rd or Rn or Rm = 15 then unpredictable
  { FALSE, &armUasxInstruction,  0x06500f30, 0x0ff00ff0, "UASX Rd,Rn,Rm" },
  // USUB8,USUB16: unsigned subtract 16 bit and 8 bit, if Rd or Rn or Rm = 15 then unpredictable
  { FALSE, &armUsub16Instruction,    0x06500f70, 0x0ff00ff0, "USUB16 Rd,Rn,Rm" },
  { FALSE, &armUsub8Instruction,     0x06500ff0, 0x0ff00ff0, "USUB8 Rd,Rn,Rm" },
  // USAX: unsigned subtract and add with exchange, if Rd or Rn or Rm = 15 then unpredictable
  { FALSE, &armUsaxInstruction,  0x06500f50, 0x0ff00ff0, "USAX Rd,Rn,Rm" },
  // UHADD8,UHADD16: unsigned halving add 16 bit and 8 bit, if Rd or Rn or Rm = 15 then unpredictable
  { FALSE, &armUhadd16Instruction,   0x06700f10, 0x0ff00ff0, "UHADD16 Rd,Rn,Rm" },
  { FALSE, &armUhadd8Instruction,    0x06700f90, 0x0ff00ff0, "UHADD8 Rd,Rn,Rm" },
  // UHASX: unsigned halving add and subtract with exchange, if Rd or Rn or Rm = 15 then unpredictable
  { FALSE, &armUhasxInstruction, 0x06700f30, 0x0ff00ff0, "UHASX Rd,Rn,Rm" },
  // UHSUB8,UHSUB16: unsigned halving subtract 16 bit and 8 bit, if Rd or Rn or Rm = 15 then unpredictable
  { FALSE, &armUhsub16Instruction,   0x06700f70, 0x0ff00ff0, "UHSUB16 Rd,Rn,Rm" },
  { FALSE, &armUhsub8Instruction,    0x06700ff0, 0x0ff00ff0, "UHSUB8 Rd,Rn,Rm" },
  // UHSAX: unsigned halving subtract and add with exchange, if Rd or Rn or Rm = 15 then unpredictable
  { FALSE, &armUhsaxInstruction, 0x06700f50, 0x0ff00ff0, "UHSAX Rd,Rn,Rm" },
  // UQADD8,UQADD16:  unsigned saturating add 16 bit and 8 bit, if Rd or Rn or Rm = 15 then unpredictable
  { FALSE, &armUqadd16Instruction,   0x06600f10, 0x0ff00ff0, "UQADD16 Rd,Rn,Rm" },
  { FALSE, &armUqadd8Instruction,    0x06600f90, 0x0ff00ff0, "UQADD8 Rd,Rn,Rm" },
  // UQASX: unsigned saturating add and subtract with exchange, if Rd or Rn or Rm = 15 then unpredictable
  { FALSE, &armUqasxInstruction, 0x06600f30, 0x0ff00ff0, "UQASX Rd,Rn,Rm" },
  // UQSUB8,UQSUB16: unsigned saturating subtract 16 bit and 8 bit, if Rd or Rn or Rm = 15 then unpredictable
  { FALSE, &armUqsub16Instruction,   0x06600f70, 0x0ff00ff0, "UQSUB16 Rd,Rn,Rm" },
  { FALSE, &armUqsub8Instruction,    0x06600ff0, 0x0ff00ff0, "UQSUB8 Rd,Rn,Rm" },
  // UQSAX: unsigned saturating subtract and add with exchange, if Rd or Rn or Rm = 15 then unpredictable
  { FALSE, &armUqsaxInstruction, 0x06600f50, 0x0ff00ff0, "UQSAX Rd,Rn,Rm" },
  // REV (byte-reverse word), if Rd or Rm = 15 then unpredictable.
  { FALSE, &armRevInstruction,       0x06bf0f30, 0x0fff0ff0, "REV Rd,Rm" },
  // REV16 (byte-reverse packed halfword), if Rd or Rm = 15 then unpredictable.
  { FALSE, &armRev16Instruction,     0x06bf0fb0, 0x0fff0ff0, "REV16 Rd,Rm" },
  // REVSH (byte-reverse signed halfword), if Rd or Rm = 15 then unpredictable.
  { FALSE, &armRevshInstruction,     0x06ff0fb0, 0x0fff0ff0, "REVSH Rd,Rm" },
  // SXTH (sign-extend halfword), if Rd or Rm = 15 then unpredictable.
  { FALSE, &armSxthInstruction,      0x06bf0070, 0x0fff0ff0, "SXTH Rd,Rm" },
  { FALSE, &armSxthInstruction,      0x06bf0470, 0x0fff0ff0, "SXTH Rd,Rm,ROR #8" },
  { FALSE, &armSxthInstruction,      0x06bf0870, 0x0fff0ff0, "SXTH Rd,Rm,ROR #16" },
  { FALSE, &armSxthInstruction,      0x06bf0c70, 0x0fff0ff0, "SXTH Rd,Rm,ROR #24" },
  // SXTB16: sign-extend byte 16, if Rd or Rm = 15 then unpredictable.
  { FALSE, &armSxtb16Instruction,    0x068f0070, 0x0fff0ff0, "SXTB16 Rd,Rm" },
  { FALSE, &armSxtb16Instruction,    0x068f0470, 0x0fff0ff0, "SXTB16 Rd,Rm,ROR #8" },
  { FALSE, &armSxtb16Instruction,    0x068f0870, 0x0fff0ff0, "SXTB16 Rd,Rm,ROR #16" },
  { FALSE, &armSxtb16Instruction,    0x068f0c70, 0x0fff0ff0, "SXTB16 Rd,Rm,ROR #24" },
  // SXTB sign extend byte, if Rd or Rn = 15 then unpredictable.
  { FALSE, &armSxtbInstruction,      0x06af0070, 0x0fff0ff0, "SXTB Rd,Rm" },
  { FALSE, &armSxtbInstruction,      0x06af0470, 0x0fff0ff0, "SXTB Rd,Rm,ROR #8" },
  { FALSE, &armSxtbInstruction,      0x06af0870, 0x0fff0ff0, "SXTB Rd,Rm,ROR #16" },
  { FALSE, &armSxtbInstruction,      0x06af0c70, 0x0fff0ff0, "SXTB Rd,Rm,ROR #24" },
  // UXTH permitted, if Rd or Rn = 15 then unpredictable.
  { FALSE, &armUxthInstruction,      0x06ff0070, 0x0fff0ff0, "UXTH Rd,Rm" },
  { FALSE, &armUxthInstruction,      0x06ff0470, 0x0fff0ff0, "UXTH Rd,Rm,ROR #8" },
  { FALSE, &armUxthInstruction,      0x06ff0870, 0x0fff0ff0, "UXTH Rd,Rm,ROR #16" },
  { FALSE, &armUxthInstruction,      0x06ff0c70, 0x0fff0ff0, "UXTH Rd,Rm,ROR #24" },
  // UXTB16 permitted, if Rd or Rn = 15 then unpredictable.
  { FALSE, &armUxtb16Instruction,    0x06cf0070, 0x0fff0ff0, "UXTB16 Rd,Rm" },
  { FALSE, &armUxtb16Instruction,    0x06cf0470, 0x0fff0ff0, "UXTB16 Rd,Rm,ROR #8" },
  { FALSE, &armUxtb16Instruction,    0x06cf0870, 0x0fff0ff0, "UXTB16 Rd,Rm,ROR #16" },
  { FALSE, &armUxtb16Instruction,    0x06cf0c70, 0x0fff0ff0, "UXTB16 Rd,Rm,ROR #24" },
  // UXTB permitted, if Rd or Rn = 15 then unpredictable.
  { FALSE, &armUxtbInstruction,      0x06ef0070, 0x0fff0ff0, "UXTB Rd,Rm" },
  { FALSE, &armUxtbInstruction,      0x06ef0470, 0x0fff0ff0, "UXTB Rd,Rm,ROR #8" },
  { FALSE, &armUxtbInstruction,      0x06ef0870, 0x0fff0ff0, "UXTB Rd,Rm,ROR #16" },
  { FALSE, &armUxtbInstruction,      0x06ef0c70, 0x0fff0ff0, "UXTB Rd,Rm,ROR #24" },
  // SXTAH (sign-extend and add halfword), if Rd or Rm = 15 then unpredictable, if Rn = 15 see SXTH.
  { FALSE, &armSxtahInstruction,     0x06b00070, 0x0ff00ff0, "SXTAH Rd,Rn,Rm" },
  { FALSE, &armSxtahInstruction,     0x06b00470, 0x0ff00ff0, "SXTAH Rd,Rn,Rm,ROR #8" },
  { FALSE, &armSxtahInstruction,     0x06b00870, 0x0ff00ff0, "SXTAH Rd,Rn,Rm,ROR #16" },
  { FALSE, &armSxtahInstruction,     0x06b00c70, 0x0ff00ff0, "SXTAH Rd,Rn,Rm,ROR #24" },
  // SXTAB16 (sign-extend and add byte 16), if Rd or Rm = 15 then unpredictable, if Rn = 15 see SXTB16.
  { FALSE, &armSxtab16Instruction,   0x06800070, 0x0ff00ff0, "SXTAB16 Rd,Rn,Rm" },
  { FALSE, &armSxtab16Instruction,   0x06800470, 0x0ff00ff0, "SXTAB16 Rd,Rn,Rm,ROR #8" },
  { FALSE, &armSxtab16Instruction,   0x06800870, 0x0ff00ff0, "SXTAB16 Rd,Rn,Rm,ROR #16" },
  { FALSE, &armSxtab16Instruction,   0x06800c70, 0x0ff00ff0, "SXTAB16 Rd,Rn,Rm,ROR #24" },
  // SXTAB (sign-extend and add byte), if Rd or Rm = 15 then unpredictable, if Rn = 15 see SXTB.
  { FALSE, &armSxtabInstruction,     0x06a00070, 0x0ff00ff0, "SXTAB Rd,Rn,Rm" },
  { FALSE, &armSxtabInstruction,     0x06a00470, 0x0ff00ff0, "SXTAB Rd,Rn,Rm,ROR #8" },
  { FALSE, &armSxtabInstruction,     0x06a00870, 0x0ff00ff0, "SXTAB Rd,Rn,Rm,ROR #16" },
  { FALSE, &armSxtabInstruction,     0x06a00c70, 0x0ff00ff0, "SXTAB Rd,Rn,Rm,ROR #24" },
  // UXTAH (unsigned extend and add halfword), if Rd or Rm = 15 then unpredictable, if Rn = 15 see UXTH.
  { FALSE, &armUxtahInstruction,     0x06f00070, 0x0ff00ff0, "UXTAH Rd,Rn,Rm" },
  { FALSE, &armUxtahInstruction,     0x06f00470, 0x0ff00ff0, "UXTAH Rd,Rn,Rm,ROR #8" },
  { FALSE, &armUxtahInstruction,     0x06f00870, 0x0ff00ff0, "UXTAH Rd,Rn,Rm,ROR #16" },
  { FALSE, &armUxtahInstruction,     0x06f00c70, 0x0ff00ff0, "UXTAH Rd,Rn,Rm,ROR #24" },
  // UXTAB16 (unsigned extend and add byte 16), if Rd or Rm = 15 then unpredictable, if Rn = 15 see UXTB16.
  { FALSE, &armUxtab16Instruction,   0x06c00070, 0x0ff00ff0, "UXTAB16 Rd,Rn,Rm" },
  { FALSE, &armUxtab16Instruction,   0x06c00470, 0x0ff00ff0, "UXTAB16 Rd,Rn,Rm,ROR #8" },
  { FALSE, &armUxtab16Instruction,   0x06c00870, 0x0ff00ff0, "UXTAB16 Rd,Rn,Rm,ROR #16" },
  { FALSE, &armUxtab16Instruction,   0x06c00c70, 0x0ff00ff0, "UXTAB16 Rd,Rn,Rm,ROR #24" },
  // UXTAB (unsigned extend and add byte), if Rd or Rm = 15 then unpredictable, if Rn = 15 see UXTB.
  { FALSE, &armUxtabInstruction,     0x06e00070, 0x0ff00ff0, "UXTAB Rd,Rn,Rm" },
  { FALSE, &armUxtabInstruction,     0x06e00470, 0x0ff00ff0, "UXTAB Rd,Rn,Rm,ROR #8" },
  { FALSE, &armUxtabInstruction,     0x06e00870, 0x0ff00ff0, "UXTAB Rd,Rn,Rm,ROR #16" },
  { FALSE, &armUxtabInstruction,     0x06e00c70, 0x0ff00ff0, "UXTAB Rd,Rn,Rm,ROR #24" },
  // SEL (select bytes), if Rd or Rm or Rn = 15 then unpredictable.
  { FALSE, &armSelInstruction,       0x06800fb0, 0x0ff00ff0, "SEL Rd,Rn,Rm" },
  // SMUAD (signed dual multiply add), if Rd or Rm or Rn = 15 then unpredictable.
  { FALSE, &armSmuadInstruction,     0x0700f010, 0x0ff0f0d0, "SMUAD{X} Rd,Rn,Rm" },
  // SMUSD: signed dual multiply subtract, if Rd or Rn or Rm = 15 then unpredictable
  { FALSE, &armSmusdInstruction,     0x0700f050, 0x0ff0f0d0, "SMUSD{X} Rd,Rn,Rm" },
  // SMLAD: signed multiply accumulate dual, if Rd or Rn or Rm = 15 then unpredictable, Ra = 15 see SMUAD
  { FALSE, &armSmladInstruction,     0x07000010, 0x0ff000d0, "SMLAD{X} Rd,Rn,Rm,Ra" },
  // SMLALD: signed multiply accumulate long dual, if any R = 15 then unpredictable
  { FALSE, &armSmlaldInstruction,    0x07400010, 0x0ff000d0, "SMLALD{X} RdLo,RdHi,Rn,Rm" },
  // SMLSD: signed multiply subtract dual, if Rd or Rn or Rm = 15 then unpredictable, Ra = 15 see SMUSD
  { FALSE, &armSmlsdInstruction,     0x07000050, 0x0ff000d0, "SMLSD{X} Rd,Rn,Rm,Ra" },
  // SMLSLD: signed multiply subtract long dual, if any R = 15 then unpredictable
  { FALSE, &armSmlsldInstruction,    0x07400050, 0x0ff000d0, "SMLSLD{X} RdLo,RdHi,Rn,Rm" },
  // SMMUL: signed most significant word multiply, if Rd or Rn or Rm = 15 then unpredictable
  { FALSE, &armSmmulInstruction,     0x0750f010, 0x0ff0f0d0, "SMMUL{R} Rd,Rn,Rm" },
  // SMMLA: signed most significant word multiply accumulate, if Rd or Rn or Rm = 15 then unpredictable, Ra = 15 see SMMUL
  { FALSE, &armSmmlaInstruction,     0x07500010, 0x0ff000d0, "SMMLA{R} Rd,Rn,Rm,Ra" },
  // SMMLS: signed most significant word multiply subtract, if any R = 15 then unpredictable
  { FALSE, &armSmmlsInstruction,     0x075000d0, 0x0ff000d0, "SMMLS{R} Rd,Rn,Rm,Ra" },
  // SSAT,SSAT16,USAT,USAT16: (un)signed saturate, if Rd or Rn = 15 then unpredictable
  { FALSE, &armSsatInstruction,      0x06a00010, 0x0fe00ff0, "SSAT Rd,#sat_imm,Rn" },
  { FALSE, &armSsatInstruction,      0x06a00010, 0x0fe00070, "SSAT Rd,#sat_imm,Rn,LSL #imm" },
  { FALSE, &armSsatInstruction,      0x06a00050, 0x0fe00070, "SSAT Rd,#sat_imm,Rn,ASR #imm" },
  { FALSE, &armSsat16Instruction,    0x06a00f30, 0x0ff00ff0, "SSAT16 Rd,#imm,Rn" },
  { FALSE, &armUsatInstruction,      0x06e00010, 0x0fe00ff0, "USAT Rd,#sat_imm,Rn" },
  { FALSE, &armUsatInstruction,      0x06e00010, 0x0fe00070, "USAT Rd,#sat_imm,Rn,LSL #imm" },
  { FALSE, &armUsatInstruction,      0x06e00050, 0x0fe00070, "USAT Rd,#sat_imm,Rn,ASR #imm" },
  { FALSE, &armUsat16Instruction,    0x06e00f30, 0x0ff00ff0, "USAT16 Rd,#imm,Rn" },

  { TRUE,  &undefinedInstruction, 0x00000000, 0x00000000, "mediaInstructions" }
};

static struct instruction32bit armSvcCoprocInstructions[] =
{
  // well obviously.
  { TRUE,  &svcInstruction,       0x0f000000, 0x0f000000, "SWI code" },
  // Generic coprocessor instructions.
  { TRUE,  &armMrcInstruction,    0x0e100010, 0x0f100010, "MRC" },
  { TRUE,  &armMcrInstruction,    0x0e000010, 0x0f100010, "MCR" },
  { TRUE,  &undefinedInstruction, 0x00000000, 0x00000000, "svcCoprocInstructions" }
};

static struct instruction32bit armUnconditionalInstructions[] =
{
  // UNIMPLEMENTED: data memory barrier
  { TRUE,  &armDmbInstruction,       0xf57ff050, 0xfffffff0, "dmb\t%U" },
  // sync barriers
  { TRUE,  &armDsbInstruction,       0xf57ff040, 0xfffffff0, "DSB" },
  { TRUE,  &armIsbInstruction,       0xf57ff060, 0xfffffff0, "ISB" },
  // UNIMPLEMENTED: CLREX clear exclusive
  { TRUE,  &armClrexInstruction,     0xf57ff01f, 0xffffffff, "clrex" },
  // CPS: change processor state
  { TRUE,  &armCpsInstruction,       0xf1080000, 0xfffffe3f, "CPSIE" },
  { TRUE,  &armCpsInstruction,       0xf10a0000, 0xfffffe20, "CPSIE" },
  { TRUE,  &armCpsInstruction,       0xf10C0000, 0xfffffe3f, "CPSID" },
  { TRUE,  &armCpsInstruction,       0xf10e0000, 0xfffffe20, "CPSID" },
  { TRUE,  &armCpsInstruction,       0xf1000000, 0xfff1fe20, "CPS" },
  // UNIMPLEMENTED: RFE return from exception
  { TRUE,  &armRfeInstruction,       0xf8100a00, 0xfe50ffff, "rfe%23?id%24?ba\t%16-19r%21'!" },
  // UNIMPLEMENTED: SETEND set endianess
  { TRUE,  &armSetendInstruction,    0xf1010000, 0xfffffc00, "setend\t%9?ble" },
  // UNIMPLEMENTED: SRS store return state
  { TRUE,  &armSrsInstruction,       0xf84d0500, 0xfe5fffe0, "srs%23?id%24?ba\t%16-19r%21'!, #%0-4d" },
  // BLX: branch and link to Thumb subroutine
  { TRUE,  &armBlxImmediateInstruction,    0xfa000000, 0xfe000000, "BLX #imm24" },
  // PLD: preload data
  { TRUE,  &armPldInstruction,       0xf450f000, 0xfc70f000, "PLD" },
  // PLI: preload instruction
  { TRUE,  &armPliInstruction,       0xf450f000, 0xfd70f000, "PLI" },
  { TRUE,  &undefinedInstruction, 0x00000000, 0x00000000, "armUnconditionalInstructions" }
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
