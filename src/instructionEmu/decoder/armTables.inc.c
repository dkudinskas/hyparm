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
  { TRUE,  &swpInstruction,       0x01000090, 0x0fb00ff0, "SWP" },
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
  { FALSE, &sumullInstruction,    0x00800090, 0x0fa000f0, "SMULL" },
  // SMLAL - signed multiply and accumulate, PC cannot be used as any destination
  { FALSE, &sumlalInstruction,    0x00a00090, 0x0fa000f0, "SMLAL" },
  // MUL: Rd = Rm * Rn; Rd != PC. pass through
  { FALSE, &mulInstruction,       0x00000090, 0x0fe000f0, "MUL Rd, Rm, Rn" },
  // MLA: Rd = Rm * Rn + Ra; Rd != PC. pass through
  { FALSE, &mlaInstruction,       0x00200090, 0x0fe000f0, "MLA Rd, Rm, Rn, Ra" },
  // MLS: Rd = Rm * Rn - Ra; Rd != PC, pass through
  { FALSE, &mlsInstruction,       0x00600090, 0x0ff000f0, "MLS Rd, Rm, Rn, Ra" },
  // Branch and try to exchange to ARM mode.
  { TRUE,  &armBxInstruction,     0x012FFF10, 0x0ffffff0, "bx%c\t%0-3r" },
  // Branch and link and try to exchange to Jazelle mode.
  { TRUE,  &armBxjInstruction,    0x012fff20, 0x0ffffff0, "BXJ Rm" },
  // Software breakpoint instruction... not sure yet.
  { TRUE,  &bkptInstruction,      0xe1200070, 0xfff000f0, "BKPT #imm8" },
  // UNIMPLEMENTED: SMC, previously SMI: secure monitor call
  { TRUE,  &smcInstruction,       0x01600070, 0x0ff000f0, "smc%c\t%e" },
  // Branch and link and try to exchange with Thumb mode.
  { TRUE,  &armBlxRegisterInstruction,       0x012fff30, 0x0ffffff0, "BLX Rm" },
  // CLZ: Count leading zeroes - Rd, Rm != PC, pass through
  { FALSE, &clzInstruction,       0x016f0f10, 0x0fff0ff0, "CLZ Rd, Rm" },
  // UNIMPLEMENTED: saturated add/subtract/doubleAdd/doubleSubtract
  { TRUE,  &qaddInstruction,      0x01000050, 0x0ff00ff0, "qadd%c\t%12-15r, %0-3r, %16-19r" },
  { TRUE,  &qdaddInstruction,     0x01400050, 0x0ff00ff0, "qdadd%c\t%12-15r, %0-3r, %16-19r" },
  { TRUE,  &qsubInstruction,      0x01200050, 0x0ff00ff0, "qsub%c\t%12-15r, %0-3r, %16-19r" },
  { TRUE,  &qdsubInstruction,     0x01600050, 0x0ff00ff0, "qdsub%c\t%12-15r, %0-3r, %16-19r" },
  // LDRD: Rt1 must be even numbered and NOT 14, thus Rt2 cannot be PC. pass.
  { FALSE, &armLdrdInstruction,   0x004000d0, 0x0e5000f0, "LDRD Rt, [Rn, #imm]" },
  { FALSE, &armLdrdInstruction,   0x000000d0, 0x0e500ff0, "LDRD Rt, [Rn, Rm]" },
  // STRD: pass through, let them fail!
  { FALSE, &armStrdInstruction,   0x004000f0, 0x0e5000f0, "STRD Rt, [Rn, #imm]" },
  { FALSE, &armStrdInstruction,   0x000000f0, 0x0e500ff0, "STRD Rt, [Rn, Rm]" },
  /* ALL UNIMPLEMENTED: smlabs, smulbs etc */
  // signed 16 bit multiply, 32 bit accumulate
  { TRUE,  &smlabbInstruction,    0x01000080, 0x0ff000f0, "smlabb%c\t%16-19r, %0-3r, %8-11r, %12-15r" },
  { TRUE,  &smlatbInstruction,    0x010000a0, 0x0ff000f0, "smlatb%c\t%16-19r, %0-3r, %8-11r, %12-15r" },
  { TRUE,  &smlabtInstruction,    0x010000c0, 0x0ff000f0, "smlabt%c\t%16-19r, %0-3r, %8-11r, %12-15r" },
  { TRUE,  &smlattInstruction,    0x010000e0, 0x0ff000f0, "smlatt%c\t%16-19r, %0-3r, %8-11r, %12-15r" },
  // signed 16 bit x 32 bit multiply, 32 bit accumulate
  { TRUE,  &smlawbInstruction,    0x01200080, 0x0ff000f0, "smlawb%c\t%16-19r, %0-3r, %8-11r, %12-15r" },
  { TRUE,  &smlawtInstruction,    0x012000c0, 0x0ff000f0, "smlawt%c\t%16-19r, %0-3r, %8-11r, %12-15r" },
  // signed 16 bit multiply, 64 bit accumulate
  { TRUE,  &smlalbbInstruction,   0x01400080, 0x0ff000f0, "smlalbb%c\t%12-15r, %16-19r, %0-3r, %8-11r" },
  { TRUE,  &smlaltbInstruction,   0x014000a0, 0x0ff000f0, "smlaltb%c\t%12-15r, %16-19r, %0-3r, %8-11r" },
  { TRUE,  &smlalbtInstruction,   0x014000c0, 0x0ff000f0, "smlalbt%c\t%12-15r, %16-19r, %0-3r, %8-11r" },
  { TRUE,  &smlalttInstruction,   0x014000e0, 0x0ff000f0, "smlaltt%c\t%12-15r, %16-19r, %0-3r, %8-11r" },
  // SMULBB: multiply signed bottom halfwords of ops. Rd can't be PC.
  { FALSE, &smulbbInstruction,    0x01600080, 0x0ff0f0f0, "SMULBB Rd, Rn, RM" },
  // signed 16 bit multiply, 32 bit result
  { TRUE,  &smultbInstruction,    0x016000a0, 0x0ff0f0f0, "smultb%c\t%16-19r, %0-3r, %8-11r" },
  { TRUE,  &smulbtInstruction,    0x016000c0, 0x0ff0f0f0, "smulbt%c\t%16-19r, %0-3r, %8-11r" },
  { TRUE,  &smulttInstruction,    0x016000e0, 0x0ff0f0f0, "smultt%c\t%16-19r, %0-3r, %8-11r" },
  // signed 16 bit x 32 bit multiply, 32 bit result
  { TRUE,  &smulwbInstruction,    0x012000a0, 0x0ff0f0f0, "smulwb%c\t%16-19r, %0-3r, %8-11r" },
  { TRUE,  &smulwtInstruction,    0x012000e0, 0x0ff0f0f0, "smulwt%c\t%16-19r, %0-3r, %8-11r" },
  // STRH: passthrough, will data abort if something wrong
  { FALSE, &armStrhInstruction,   0x004000b0, 0x0e5000f0, "STRH Rt, [Rn, +-imm8]" },
  { FALSE, &armStrhInstruction,   0x000000b0, 0x0e500ff0, "STRH Rt, [Rn], +-Rm" },
  // LDRH cant load halfword to PC, passthrough
  { FALSE, &armLdrhInstruction,   0x00500090, 0x0e500090, "LDRH Rt, [Rn, +-imm8]" },
  { FALSE, &armLdrhInstruction,   0x00100090, 0x0e500f90, "LDRH Rt, [Rn], +-Rm" },
  // AND: Rd = PC end block, others are fine
  { TRUE,  &andInstruction,       0x0000f000, 0x0fe0f010, "AND PC, Rn, Rm, #shamt" },
  { TRUE,  &andInstruction,       0x0000f010, 0x0fe0f090, "AND PC, Rn, Rm, Rshamt" },
  { FALSE, &andInstruction,       0x00000000, 0x0fe00010, "AND Rd, Rn, Rm, #shamt" },
  { FALSE, &andInstruction,       0x00000010, 0x0fe00090, "AND Rd, Rn, Rm, Rshamt" },
  // EOR: Rd = PC end block, others are fine
  { TRUE,  &eorInstruction,       0x0020f000, 0x0fe0f010, "EOR PC, Rn, Rm, #shamt" },
  { TRUE,  &eorInstruction,       0x0020f010, 0x0fe0f090, "EOR PC, Rn, Rm, Rshamt" },
  { FALSE, &eorInstruction,       0x00200000, 0x0fe00010, "EOR Rd, Rn, Rm, #shamt" },
  { FALSE, &eorInstruction,       0x00200010, 0x0fe00090, "EOR Rd, Rn, Rm, Rshamt" },
  // SUB: Rd = PC end block, others are fine
  { TRUE,  &subInstruction,       0x0040f000, 0x0fe0f010, "SUB PC, Rn, Rm, #shamt" },
  { TRUE,  &subInstruction,       0x0040f010, 0x0fe0f090, "SUB PC, Rn, Rm, Rshamt" },
  { FALSE, &subInstruction,       0x00400000, 0x0fe00010, "SUB Rd, Rn, Rm, #shamt" },
  { FALSE, &subInstruction,       0x00400010, 0x0fe00090, "SUB Rd, Rn, Rm, Rshamt" },
  // RSB: Rd = PC end block, others are fine
  { TRUE,  &rsbInstruction,       0x0060f000, 0x0fe0f010, "RSB PC, Rn, Rm, #shamt" },
  { TRUE,  &rsbInstruction,       0x0060f010, 0x0fe0f090, "RSB PC, Rn, Rm, Rshamt" },
  { FALSE, &rsbInstruction,       0x00600000, 0x0fe00010, "RSB Rd, Rn, Rm, #shamt" },
  { FALSE, &rsbInstruction,       0x00600010, 0x0fe00090, "RSB Rd, Rn, Rm, Rshamt" },
  // ADD: Rd = PC end block, others are fine
  { TRUE,  &addInstruction,       0x0080f000, 0x0fe0f010, "ADD PC, Rn, Rm, #shamt" },
  { TRUE,  &addInstruction,       0x0080f010, 0x0fe0f090, "ADD PC, Rn, Rm, Rshamt" },
  { FALSE, &addInstruction,       0x00800000, 0x0fe00010, "ADD Rd, Rn, Rm, #shamt" },
  { FALSE, &addInstruction,       0x00800010, 0x0fe00090, "ADD Rd, Rn, Rm, Rshamt" },
  // ADC: Rd = PC end block, others are fine
  { TRUE,  &adcInstruction,       0x00a0f000, 0x0fe0f010, "ADC PC, Rn, Rm, #shamt" },
  { TRUE,  &adcInstruction,       0x00a0f010, 0x0fe0f090, "ADC PC, Rn, Rm, Rshamt" },
  { FALSE, &adcInstruction,       0x00a00000, 0x0fe00010, "ADC Rd, Rn, Rm, #shamt" },
  { FALSE, &adcInstruction,       0x00a00010, 0x0fe00090, "ADC Rd, Rn, Rm, Rshamt" },
  // SBC: Rd = PC end block, others are fine
  { TRUE,  &sbcInstruction,       0x00c0f000, 0x0fe0f010, "SBC Rd, Rn, Rm, #shamt" },
  { TRUE,  &sbcInstruction,       0x00c0f010, 0x0fe0f090, "SBC Rd, Rn, Rm, Rshamt" },
  { FALSE, &sbcInstruction,       0x00c00000, 0x0fe00010, "SBC Rd, Rn, Rm, #shamt" },
  { FALSE, &sbcInstruction,       0x00c00010, 0x0fe00090, "SBC Rd, Rn, Rm, Rshamt" },
  // RSC: Rd = PC end block, others are fine
  { TRUE,  &rscInstruction,       0x00e0f000, 0x0fe0f010, "RSC PC, Rn, Rm, #shamt" },
  { TRUE,  &rscInstruction,       0x00e0f010, 0x0fe0f090, "RSC PC, Rn, Rm, Rshamt" },
  { FALSE, &rscInstruction,       0x00e00000, 0x0fe00010, "RSC Rd, Rn, Rm, #shamt" },
  { FALSE, &rscInstruction,       0x00e00010, 0x0fe00090, "RSC Rd, Rn, Rm, Rshamt" },
  // MSR/MRS: always hypercall! we must hide the real state from guest.
  { TRUE,  &msrInstruction,       0x0120f000, 0x0fb0fff0, "MSR, s/cpsr, Rn" },
  { TRUE,  &mrsInstruction,       0x010f0000, 0x0fbf0fff, "MRS, Rn, s/cpsr" },
  // TST instructions are all fine
  { FALSE, &tstInstruction,       0x01000000, 0x0fe00010, "TST Rn, Rm, #shamt" },
  { FALSE, &tstInstruction,       0x01000010, 0x0fe00090, "TST Rn, Rm, Rshift" },
  // TEQ instructions are all fine
  { FALSE, &teqInstruction,       0x01200000, 0x0fe00010, "TEQ Rn, Rm, #shamt" },
  { FALSE, &teqInstruction,       0x01200010, 0x0fe00090, "TEQ Rn, Rm, Rshift" },
  // CMP instructions are all fine
  { FALSE, &cmpInstruction,       0x01400000, 0x0fe00010, "CMP Rn, Rm, #shamt" },
  { FALSE, &cmpInstruction,       0x01400010, 0x0fe00090, "CMP Rn, Rm, Rshamt" },
  // CMN instructions are all fine
  { FALSE, &cmnInstruction,       0x01600000, 0x0fe00010, "CMN Rn, Rm, #shamt" },
  { FALSE, &cmnInstruction,       0x01600010, 0x0fe00090, "CMN Rn, Rm, Rshamt" },
  // ORR: Rd = PC end block, other are fine
  { TRUE,  &orrInstruction,       0x0180f000, 0x0fe0f010, "ORR PC, Rn, Rm, #shamt" },
  { TRUE,  &orrInstruction,       0x0180f010, 0x0fe0f090, "ORR PC, Rn, Rm, Rshamt" },
  { FALSE, &orrInstruction,       0x01800000, 0x0fe00010, "ORR Rd, Rn, Rm, #shamt" },
  { FALSE, &orrInstruction,       0x01800010, 0x0fe00090, "ORR Rd, Rn, Rm, Rshamt" },
  // MOV with Rd = PC end block. MOV reg, <shifted reg> is a pseudo instr for shift instructions
  { TRUE,  &movInstruction,       0x01a0f000, 0x0deffff0, "MOV PC, Rm" },
  { FALSE, &movInstruction,       0x01a00000, 0x0def0ff0, "MOV Rn, Rm" },
  // LSL: Rd = PC and shamt = #imm, then end block. Otherwise fine
  { TRUE,  &lslInstruction,       0x01a0f000, 0x0feff070, "LSL Rd, Rm, #shamt" },
  { TRUE,  &lslInstruction,       0x01a0f010, 0x0feff0f0, "LSL Rd, Rm, Rshamt" },
  { FALSE, &lslInstruction,       0x01a00000, 0x0fef0070, "LSL Rd, Rm, #shamt" },
  { FALSE, &lslInstruction,       0x01a00010, 0x0fef00f0, "LSL Rd, Rm, Rshamt" },
  // LSR: Rd = PC and shamt = #imm, then end block. Otherwise fine
  { TRUE,  &lsrInstruction,       0x01a0f020, 0x0feff070, "LSR Rd, Rm, #shamt" },
  { TRUE,  &lsrInstruction,       0x01a0f030, 0x0feff0f0, "LSR Rd, Rm, Rshamt" },
  { FALSE, &lsrInstruction,       0x01a00020, 0x0fef0070, "LSR Rd, Rm, #shamt" },
  { FALSE, &lsrInstruction,       0x01a00030, 0x0fef00f0, "LSR Rd, Rm, Rshamt" },
  // ASR: Rd = PC and shamt = #imm, then end block. Otherwise fine
  { TRUE,  &asrInstruction,       0x01a0f040, 0x0feff070, "ASR Rd, Rm, #shamt" },
  { TRUE,  &asrInstruction,       0x01a0f050, 0x0feff0f0, "ASR Rd, Rm, Rshamt" },
  { FALSE, &asrInstruction,       0x01a00040, 0x0fef0070, "ASR Rd, Rm, #shamt" },
  { FALSE, &asrInstruction,       0x01a00050, 0x0fef00f0, "ASR Rd, Rm, Rshamt" },
  // RRX: shift right and extend, Rd can be PC
  { TRUE,  &rrxInstruction,       0x01a0f060, 0x0feffff0, "RRX PC, Rm" },
  { FALSE, &rrxInstruction,       0x01a00060, 0x0fef0ff0, "RRX Rd, Rm" },
  // ROR: reg case destination unpredictable. imm case dest can be PC.
  { FALSE, &rorInstruction,       0x01a00070, 0x0fef00f0, "ROR Rd, Rm, Rn" },
  { TRUE,  &rorInstruction,       0x01a0f060, 0x0feff070, "ROR PC, Rm, #imm" },
  { FALSE, &rorInstruction,       0x01a00060, 0x0fef0070, "ROR Rd, Rm, #imm" },
  // BIC with Rd = PC end block, other are fine.
  { TRUE,  &bicInstruction,       0x01c0f000, 0x0fe0f010, "BIC PC, Rn, Rm, #shamt" },
  { TRUE,  &bicInstruction,       0x01c0f010, 0x0fe0f090, "BIC PC, Rn, Rm, Rshamt" },
  { FALSE, &bicInstruction,       0x01c00000, 0x0fe00010, "BIC Rd, Rn, Rm, #shamt" },
  { FALSE, &bicInstruction,       0x01c00010, 0x0fe00090, "BIC Rd, Rn, Rm, Rshamt" },
  // MVN with Rd = PC end block, other are fine.
  { TRUE,  &mvnInstruction,       0x01e0f000, 0x0fe0f010, "MVN Rd, Rm, #shamt" },
  { TRUE,  &mvnInstruction,       0x01e0f010, 0x0fe0f090, "MVN Rd, Rm, Rshamt" },
  { FALSE, &mvnInstruction,       0x01e00000, 0x0fe00010, "MVN Rd, Rm, #shamt" },
  { FALSE, &mvnInstruction,       0x01e00010, 0x0fe00090, "MVN Rd, Rm, Rshamt" },

  { TRUE,  &undefinedInstruction, 0x00000000, 0x00000000, "dataProcMiscInstructions_op0" }
};

// immediate cases and random hints
static struct instruction32bit armDataProcMiscInstructions_op1[] =
{
  // UNIMPLEMENTED: yield hint
  { TRUE,  &yieldInstruction,     0x0320f001, 0x0fffffff, "yield%c" },
  // UNIMPLEMENTED: wait for event hint
  { TRUE,  &wfeInstruction,       0x0320f002, 0x0fffffff, "wfe%c" },
  // UNIMPLEMENTED: wait for interrupt hint
  { TRUE,  &wfiInstruction,       0x0320f003, 0x0fffffff, "wfi%c" },
  // UNIMPLEMENTED: send event hint
  { TRUE,  &sevInstruction,       0x0320f004, 0x0fffffff, "sev%c" },
  { FALSE, &nopInstruction,       0x0320f000, 0x0fffff00, "NOP" },
  // UNIMPLEMENTED: debug hint
  { TRUE,  &dbgInstruction,       0x0320f0f0, 0x0ffffff0, "dbg%c\t#%0-3d" },
  // MOVW - indication of 'wide' - to select ARM encoding. Rd cant be PC, pass through.
  { FALSE, &movwInstruction,      0x03000000, 0x0ff00000, "MOVW Rd, Rn" },
  { FALSE, &movtInstruction,      0x03400000, 0x0ff00000, "MOVT Rd, Rn" },
  // AND: Rd = PC end block, others are fine
  { TRUE,  &andInstruction,       0x0200f000, 0x0fe0f000, "AND PC, Rn, #imm" },
  { FALSE, &andInstruction,       0x02000000, 0x0fe00000, "AND Rd, Rn, #imm" },
  // EOR: Rd = PC end block, others are fine
  { TRUE,  &eorInstruction,       0x0220f000, 0x0fe0f000, "EOR PC, Rn, Rm/#imm" },
  { FALSE, &eorInstruction,       0x02200000, 0x0fe00000, "EOR Rd, Rn, #imm" },
  // SUB: Rd = PC end block, others are fine
  { TRUE,  &subInstruction,       0x0240f000, 0x0fe0f000, "SUB PC, Rn, Rm/imm" },
  { FALSE, &subInstruction,       0x02400000, 0x0fe00000, "SUB Rd, Rn, #imm" },
  // RSB: Rd = PC end block, others are fine
  { TRUE,  &rsbInstruction,       0x0260f000, 0x0fe0f000, "RSB PC, Rn, Rm/imm" },
  { FALSE, &rsbInstruction,       0x02600000, 0x0fe00000, "RSB Rd, Rn, #imm" },
  // ADD: Rd = PC end block, others are fine
  { TRUE,  &addInstruction,       0x0280f000, 0x0fe0f000, "ADD PC, Rn, #imm" },
  { FALSE, &addInstruction,       0x02800000, 0x0fe00000, "ADD Rd, Rn, #imm" },
  // ADC: Rd = PC end block, others are fine
  { TRUE,  &adcInstruction,       0x02a0f000, 0x0fe0f000, "ADC PC, Rn/#imm" },
  { FALSE, &adcInstruction,       0x02a00000, 0x0fe00000, "ADC Rd, Rn, #imm" },
  // SBC: Rd = PC end block, others are fine
  { TRUE,  &sbcInstruction,       0x02c0f000, 0x0fe0f000, "SBC PC, Rn/#imm" },
  { FALSE, &sbcInstruction,       0x02c00000, 0x0fe00000, "SBC Rd, Rn, #imm" },
  // RSC: Rd = PC end block, others are fine
  { TRUE,  &rscInstruction,       0x02e0f000, 0x0fe0f000, "RSC PC, Rn/#imm" },
  { FALSE, &rscInstruction,       0x02e00000, 0x0fe00000, "RSC Rd, Rn, #imm" },
  // MSR: always hypercall! we must hide the real state from guest.
  { TRUE,  &msrInstruction,       0x0320f000, 0x0fb0f000, "MSR, s/cpsr, #imm" },
  // TST instructions are all fine
  { FALSE, &tstInstruction,       0x03000000, 0x0fe00000, "TST Rn, #imm" },
  // TEQ instructions are all fine
  { FALSE, &teqInstruction,       0x03200000, 0x0fe00000, "TEQ Rn, #imm" },
  // CMP instructions are all fine
  { FALSE, &cmpInstruction,       0x03400000, 0x0fe00000, "CMP Rn, #imm" },
  // CMN instructions are all fine
  { FALSE, &cmnInstruction,       0x03600000, 0x0fe00000, "CMN Rn, #imm" },
  // ORR: Rd = PC end block, other are fine
  { TRUE,  &orrInstruction,       0x0380f000, 0x0fe0f000, "ORR Rd, Rn, #imm" },
  { FALSE, &orrInstruction,       0x03800000, 0x0fe00000, "ORR Rd, Rn, #imm" },
  // MOV with Rd = PC end block. MOV <shifted reg> is a pseudo instr..
  { TRUE,  &movInstruction,       0x03a0f000, 0x0feff000, "MOV PC, #imm" },
  { FALSE, &movInstruction,       0x03a00000, 0x0fef0000, "MOV Rn, #imm" },
  // BIC with Rd = PC end block, other are fine.
  { TRUE,  &bicInstruction,       0x03c0f000, 0x0fe0f000, "BIC PC, Rn, #imm" },
  { FALSE, &bicInstruction,       0x03c00000, 0x0fe00000, "BIC Rd, Rn, #imm" },
  // MVN with Rd = PC end block, other are fine.
  { TRUE,  &mvnInstruction,       0x03e0f000, 0x0fe0f000, "MVN PC, Rn, #imm" },
  { FALSE, &mvnInstruction,       0x03e00000, 0x0fe00000, "MVN Rd, Rn, #imm" },

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
  { FALSE, &bfcInstruction,       0x07c0001f, 0x0fe0007f, "BFC Rd, #LSB, #width" },
  // BFI: bit field insert, dest PC not allowed.
  { FALSE, &bfiInstruction,       0x07c00010, 0x0fe00070, "BFI Rd, #LSB, #width" },
  // UNIMPLEMENTED: reverse bits
  { TRUE,  &rbitInstruction,      0x06ff0f30, 0x0fff0ff0, "rbit%c\t%12-15r, %0-3r" },
  // UBFX: extract bit field - destination 15 unpredictable
  { FALSE, &usbfxInstruction,     0x07a00050, 0x0fa00070, "UBFX Rd, Rn, width" },
  // UNIMPLEMENTED: pack halfword
  { TRUE,  &pkhbtInstruction,     0x06800010, 0x0ff00ff0, "pkhbt%c\t%12-15r, %16-19r, %0-3r" },
  { TRUE,  &pkhbtInstruction,     0x06800010, 0x0ff00070, "pkhbt%c\t%12-15r, %16-19r, %0-3r, lsl #%7-11d" },
  { TRUE,  &pkhtbInstruction,     0x06800050, 0x0ff00ff0, "pkhtb%c\t%12-15r, %16-19r, %0-3r, asr #32" },
  { TRUE,  &pkhtbInstruction,     0x06800050, 0x0ff00070, "pkhtb%c\t%12-15r, %16-19r, %0-3r, asr #%7-11d" },
  // UNIMPLEMENTED: saturating add 16 bit and 8 bit
  { TRUE,  &qadd16Instruction,    0x06200f10, 0x0ff00ff0, "qadd16%c\t%12-15r, %16-19r, %0-3r" },
  { TRUE,  &qadd8Instruction,     0x06200f90, 0x0ff00ff0, "qadd8%c\t%12-15r, %16-19r, %0-3r" },
  // UNIMPLEMENTED: saturating add and subtract with exchange
  { TRUE,  &qaddsubxInstruction,  0x06200f30, 0x0ff00ff0, "qaddsubx%c\t%12-15r, %16-19r, %0-3r" },
  // UNIMPLEMENTED: saturating subtract 16 and 8 bit
  { TRUE,  &qsub16Instruction,    0x06200f70, 0x0ff00ff0, "qsub16%c\t%12-15r, %16-19r, %0-3r" },
  { TRUE,  &qsub8Instruction,     0x06200ff0, 0x0ff00ff0, "qsub8%c\t%12-15r, %16-19r, %0-3r" },
  // UNIMPLEMENTED: saturating subtract and add with exchange
  { TRUE,  &qsubaddxInstruction,  0x06200f50, 0x0ff00ff0, "qsubaddx%c\t%12-15r, %16-19r, %0-3r" },
  // UNIMPLEMENTED: signed add 16 bit and 8 bit
  { TRUE,  &sadd16Instruction,    0x06100f10, 0x0ff00ff0, "sadd16%c\t%12-15r, %16-19r, %0-3r" },
  { TRUE,  &sadd8Instruction,     0x06100f90, 0x0ff00ff0, "sadd8%c\t%12-15r, %16-19r, %0-3r" },
  // signed add and subtract with exchange
  { TRUE,  &saddsubxInstruction,  0x06100f30, 0x0ff00ff0, "saddsubx%c\t%12-15r, %16-19r, %0-3r" },
  // signed subtract 16 bit and 8 bit
  { TRUE,  &ssub16Instruction,    0x06100f70, 0x0ff00ff0, "ssub16%c\t%12-15r, %16-19r, %0-3r" },
  { TRUE,  &ssub8Instruction,     0x06100ff0, 0x0ff00ff0, "ssub8%c\t%12-15r, %16-19r, %0-3r" },
  // signed subtract and add with exchange
  { TRUE,  &ssubaddxInstruction,  0x06100f50, 0x0ff00ff0, "ssubaddx%c\t%12-15r, %16-19r, %0-3r" },
  // UNIMPLEMENTED: signed halvign add 16 bit and 8 bit
  { TRUE,  &shadd16Instruction,   0x06300f10, 0x0ff00ff0, "shadd16%c\t%12-15r, %16-19r, %0-3r" },
  { TRUE,  &shadd8Instruction,    0x06300f90, 0x0ff00ff0, "shadd8%c\t%12-15r, %16-19r, %0-3r" },
  // UNIMPLEMENTED: signed halving add and subtract with exchange
  { TRUE,  &shaddsubxInstruction, 0x06300f30, 0x0ff00ff0, "shaddsubx%c\t%12-15r, %16-19r, %0-3r" },
  // UNIMPLEMENTED: signed halvign subtract 16 bit and 8 bit
  { TRUE,  &shsub16Instruction,   0x06300f70, 0x0ff00ff0, "shsub16%c\t%12-15r, %16-19r, %0-3r" },
  { TRUE,  &shsub8Instruction,    0x06300ff0, 0x0ff00ff0, "shsub8%c\t%12-15r, %16-19r, %0-3r" },
  // UNIMPLEMENTED: signed halving subtract and add with exchange
  { TRUE,  &shsubaddxInstruction, 0x06300f50, 0x0ff00ff0, "shsubaddx%c\t%12-15r, %16-19r, %0-3r" },
  // UNIMPLEMENTED: unsigned add 16 bit and 8 bit
  { TRUE,  &uadd16Instruction,    0x06500f10, 0x0ff00ff0, "uadd16%c\t%12-15r, %16-19r, %0-3r" },
  { TRUE,  &uadd8Instruction,     0x06500f90, 0x0ff00ff0, "uadd8%c\t%12-15r, %16-19r, %0-3r" },
  // UNIMPLEMENTED: unsigned add and subtract with exchange
  { TRUE,  &uaddsubxInstruction,  0x06500f30, 0x0ff00ff0, "uaddsubx%c\t%12-15r, %16-19r, %0-3r" },
  // UNIMPLEMENTED: unsigned subtract 16 bit and 8 bit
  { TRUE,  &usub16Instruction,    0x06500f70, 0x0ff00ff0, "usub16%c\t%12-15r, %16-19r, %0-3r" },
  { TRUE,  &usub8Instruction,     0x06500ff0, 0x0ff00ff0, "usub8%c\t%12-15r, %16-19r, %0-3r" },
  // UNIMPLEMENTED: unsigned subtract and add with exchange
  { TRUE,  &usubaddxInstruction,  0x06500f50, 0x0ff00ff0, "usubaddx%c\t%12-15r, %16-19r, %0-3r" },
  // UNIMPLEMENTED: unsigned halving add 16 bit and 8 bit
  { TRUE,  &uhadd16Instruction,   0x06700f10, 0x0ff00ff0, "uhadd16%c\t%12-15r, %16-19r, %0-3r" },
  { TRUE,  &uhadd8Instruction,    0x06700f90, 0x0ff00ff0, "uhadd8%c\t%12-15r, %16-19r, %0-3r" },
  // UNIMPLEMENTED: unsigned halving add and subtract with exchange
  { TRUE,  &uhaddsubxInstruction, 0x06700f30, 0x0ff00ff0, "uhaddsubxc\t%12-15r, %16-19r, %0-3r" },
  // UNIMPLEMENTED: unsigned halving subtract 16 bit and 8 bit
  { TRUE,  &uhsub16Instruction,   0x06700f70, 0x0ff00ff0, "uhsub16%c\t%12-15r, %16-19r, %0-3r" },
  { TRUE,  &uhsub8Instruction,    0x06700ff0, 0x0ff00ff0, "uhsub8%c\t%12-15r, %16-19r, %0-3r" },
  // UNIMPLEMENTED: unsigned halving subtract and add with exchange
  { TRUE,  &uhsubaddxInstruction, 0x06700f50, 0x0ff00ff0, "uhsubaddx%c\t%12-15r, %16-19r, %0-3r" },
  // UNIMPLEMENTED:  unsigned saturating add 16 bit and 8 bit
  { TRUE,  &uqadd16Instruction,   0x06600f10, 0x0ff00ff0, "uqadd16%c\t%12-15r, %16-19r, %0-3r" },
  { TRUE,  &uqadd8Instruction,    0x06600f90, 0x0ff00ff0, "uqadd8%c\t%12-15r, %16-19r, %0-3r" },
  // UNIMPLEMENTED: unsigned saturating add and subtract with exchange
  { TRUE,  &uqaddsubxInstruction, 0x06600f30, 0x0ff00ff0, "uqaddsubx%c\t%12-15r, %16-19r, %0-3r" },
  // UNIMPLEMENTED: unsigned saturating subtract 16 bit and 8 bit
  { TRUE,  &uqsub16Instruction,   0x06600f70, 0x0ff00ff0, "uqsub16%c\t%12-15r, %16-19r, %0-3r" },
  { TRUE,  &uqsub8Instruction,    0x06600ff0, 0x0ff00ff0, "uqsub8%c\t%12-15r, %16-19r, %0-3r" },
  // UNIMPLEMENTED: unsigned saturating subtract and add with exchange
  { TRUE,  &uqsubaddxInstruction, 0x06600f50, 0x0ff00ff0, "uqsubaddx%c\t%12-15r, %16-19r, %0-3r" },
  // UNIMPLEMENTED: byte-reverse word
  { TRUE,  &revInstruction,       0x06bf0f30, 0x0fff0ff0, "rev%c\t%12-15r, %0-3r" },
  // UNIMPLEMENTED:  byte-reverse packed halfword
  { TRUE,  &rev16Instruction,     0x06bf0fb0, 0x0fff0ff0, "rev16%c\t%12-15r, %0-3r" },
  // UNIMPLEMENTED:  byte-reverse signed halfword
  { TRUE,  &revshInstruction,     0x06ff0fb0, 0x0fff0ff0, "revsh%c\t%12-15r, %0-3r" },
  // UNIMPLEMENTED: sign-extend halfword
  { TRUE,  &sxthInstruction,      0x06bf0070, 0x0fff0ff0, "sxth%c\t%12-15r, %0-3r" },
  { TRUE,  &sxthInstruction,      0x06bf0470, 0x0fff0ff0, "sxth%c\t%12-15r, %0-3r, ror #8" },
  { TRUE,  &sxthInstruction,      0x06bf0870, 0x0fff0ff0, "sxth%c\t%12-15r, %0-3r, ror #16" },
  { TRUE,  &sxthInstruction,      0x06bf0c70, 0x0fff0ff0, "sxth%c\t%12-15r, %0-3r, ror #24" },
  // UNIMPLEMENTED: sign-extend byte 16
  { TRUE,  &sxtb16Instruction,    0x068f0070, 0x0fff0ff0, "sxtb16%c\t%12-15r, %0-3r" },
  { TRUE,  &sxtb16Instruction,    0x068f0470, 0x0fff0ff0, "sxtb16%c\t%12-15r, %0-3r, ror #8" },
  { TRUE,  &sxtb16Instruction,    0x068f0870, 0x0fff0ff0, "sxtb16%c\t%12-15r, %0-3r, ror #16" },
  { TRUE,  &sxtb16Instruction,    0x068f0c70, 0x0fff0ff0, "sxtb16%c\t%12-15r, %0-3r, ror #24" },
  // SXTB: destination register PC not allowed,
  { FALSE, &sxtbInstruction,      0x06af0070, 0x0fff0ff0, "sxtb%c\t%12-15r, %0-3r" },
  { FALSE, &sxtbInstruction,      0x06af0470, 0x0fff0ff0, "sxtb%c\t%12-15r, %0-3r, ror #8" },
  { FALSE, &sxtbInstruction,      0x06af0870, 0x0fff0ff0, "sxtb%c\t%12-15r, %0-3r, ror #16" },
  { FALSE, &sxtbInstruction,      0x06af0c70, 0x0fff0ff0, "sxtb%c\t%12-15r, %0-3r, ror #24" },
  // UXTH permitted, if Rd or Rn = 15 then unpredictable.
  { FALSE, &uxthInstruction,      0x06ff0070, 0x0fff0ff0, "UXTH Rd, Rn" },
  { FALSE, &uxthInstruction,      0x06ff0470, 0x0fff0ff0, "UXTH Rd, Rn, ROR #8" },
  { FALSE, &uxthInstruction,      0x06ff0870, 0x0fff0ff0, "UXTH Rd, Rn, ROR #16" },
  { FALSE, &uxthInstruction,      0x06ff0c70, 0x0fff0ff0, "UXTH Rd, Rn, ROR #24" },
  // UXTB16 permitted, if Rd or Rn = 15 then unpredictable.
  { FALSE, &uxtb16Instruction,    0x06cf0070, 0x0fff0ff0, "UXTB16 Rd, Rn" },
  { FALSE, &uxtb16Instruction,    0x06cf0470, 0x0fff0ff0, "UXTB16 Rd, Rn, ROR #8" },
  { FALSE, &uxtb16Instruction,    0x06cf0870, 0x0fff0ff0, "UXTB16 Rd, Rn, ROR #16" },
  { FALSE, &uxtb16Instruction,    0x06cf0c70, 0x0fff0ff0, "UXTB16 Rd, Rn, ROR #24" },
  // UXTB permitted, if Rd or Rn = 15 then unpredictable.
  { FALSE, &uxtbInstruction,      0x06ef0070, 0x0fff0ff0, "UXTB Rd, Rn" },
  { FALSE, &uxtbInstruction,      0x06ef0470, 0x0fff0ff0, "UXTB Rd, Rn, ROR #8" },
  { FALSE, &uxtbInstruction,      0x06ef0870, 0x0fff0ff0, "UXTB Rd, Rn, ROR #16" },
  { FALSE, &uxtbInstruction,      0x06ef0c70, 0x0fff0ff0, "UXTB Rd, Rn, ROR #24" },
  // UNIMPLEMENTED: sign-extend and add halfword
  { TRUE,  &sxtahInstruction,     0x06b00070, 0x0ff00ff0, "sxtah%c\t%12-15r, %16-19r, %0-3r" },
  { TRUE,  &sxtahInstruction,     0x06b00470, 0x0ff00ff0, "sxtah%c\t%12-15r, %16-19r, %0-3r, ror #8" },
  { TRUE,  &sxtahInstruction,     0x06b00870, 0x0ff00ff0, "sxtah%c\t%12-15r, %16-19r, %0-3r, ror #16" },
  { TRUE,  &sxtahInstruction,     0x06b00c70, 0x0ff00ff0, "sxtah%c\t%12-15r, %16-19r, %0-3r, ror #24" },
  // UNIMPLEMENTED: sign-extend and add byte 16
  { TRUE,  &sxtab16Instruction,   0x06800070, 0x0ff00ff0, "sxtab16%c\t%12-15r, %16-19r, %0-3r" },
  { TRUE,  &sxtab16Instruction,   0x06800470, 0x0ff00ff0, "sxtab16%c\t%12-15r, %16-19r, %0-3r, ror #8" },
  { TRUE,  &sxtab16Instruction,   0x06800870, 0x0ff00ff0, "sxtab16%c\t%12-15r, %16-19r, %0-3r, ror #16" },
  { TRUE,  &sxtab16Instruction,   0x06800c70, 0x0ff00ff0, "sxtab16%c\t%12-15r, %16-19r, %0-3r, ror #24" },
  // UNIMPLEMENTED: sign-extend and add byte
  { TRUE,  &sxtabInstruction,     0x06a00070, 0x0ff00ff0, "sxtab%c\t%12-15r, %16-19r, %0-3r" },
  { TRUE,  &sxtabInstruction,     0x06a00470, 0x0ff00ff0, "sxtab%c\t%12-15r, %16-19r, %0-3r, ror #8" },
  { TRUE,  &sxtabInstruction,     0x06a00870, 0x0ff00ff0, "sxtab%c\t%12-15r, %16-19r, %0-3r, ror #16" },
  { TRUE,  &sxtabInstruction,     0x06a00c70, 0x0ff00ff0, "sxtab%c\t%12-15r, %16-19r, %0-3r, ror #24" },
  // UNIMPLEMENTED: unsigned extend and add halfword
  { TRUE,  &uxtahInstruction,     0x06f00070, 0x0ff00ff0, "uxtah%c\t%12-15r, %16-19r, %0-3r" },
  { TRUE,  &uxtahInstruction,     0x06f00470, 0x0ff00ff0, "uxtah%c\t%12-15r, %16-19r, %0-3r, ror #8" },
  { TRUE,  &uxtahInstruction,     0x06f00870, 0x0ff00ff0, "uxtah%c\t%12-15r, %16-19r, %0-3r, ror #16" },
  { TRUE,  &uxtahInstruction,     0x06f00c70, 0x0ff00ff0, "uxtah%c\t%12-15r, %16-19r, %0-3r, ror #24" },
  // UNIMPLEMENTED: unsigned extend and add byte 16
  { TRUE,  &uxtab16Instruction,   0x06c00070, 0x0ff00ff0, "uxtab16%c\t%12-15r, %16-19r, %0-3r" },
  { TRUE,  &uxtab16Instruction,   0x06c00470, 0x0ff00ff0, "uxtab16%c\t%12-15r, %16-19r, %0-3r, ror #8" },
  { TRUE,  &uxtab16Instruction,   0x06c00870, 0x0ff00ff0, "uxtab16%c\t%12-15r, %16-19r, %0-3r, ror #16" },
  { TRUE,  &uxtab16Instruction,   0x06c00c70, 0x0ff00ff0, "uxtab16%c\t%12-15r, %16-19r, %0-3r, ROR #24" },
  // UNIMPLEMENTED: unsigned extend and add byte
  { TRUE,  &uxtabInstruction,     0x06e00070, 0x0ff00ff0, "uxtab%c\t%12-15r, %16-19r, %0-3r" },
  { TRUE,  &uxtabInstruction,     0x06e00470, 0x0ff00ff0, "uxtab%c\t%12-15r, %16-19r, %0-3r, ror #8" },
  { TRUE,  &uxtabInstruction,     0x06e00870, 0x0ff00ff0, "uxtab%c\t%12-15r, %16-19r, %0-3r, ror #16" },
  { TRUE,  &uxtabInstruction,     0x06e00c70, 0x0ff00ff0, "uxtab%c\t%12-15r, %16-19r, %0-3r, ror #24" },
  // UNIMPLEMENTED: select bytes
  { TRUE,  &selInstruction,       0x06800fb0, 0x0ff00ff0, "sel%c\t%12-15r, %16-19r, %0-3r" },
  // UNIMPLEMENTED: signed dual multiply add
  { TRUE,  &smuadInstruction,     0x0700f010, 0x0ff0f0d0, "smuad%5'x%c\t%16-19r, %0-3r, %8-11r" },
  // UNIMPLEMENTED: signed dual multiply subtract
  { TRUE,  &smusdInstruction,     0x0700f050, 0x0ff0f0d0, "smusd%5'x%c\t%16-19r, %0-3r, %8-11r" },
  // UNIMPLEMENTED: signed multiply accumulate dual
  { TRUE,  &smladInstruction,     0x07000010, 0x0ff000d0, "smlad%5'x%c\t%16-19r, %0-3r, %8-11r, %12-15r" },
  // UNIMPLEMENTED: signed multiply accumulate long dual
  { TRUE,  &smlaldInstruction,    0x07400010, 0x0ff000d0, "smlald%5'x%c\t%12-15r, %16-19r, %0-3r, %8-11r" },
  // UNIMPLEMENTED: signed multiply subtract dual
  { TRUE,  &smlsdInstruction,     0x07000050, 0x0ff000d0, "smlsd%5'x%c\t%16-19r, %0-3r, %8-11r, %12-15r" },
  // UNIMPLEMENTED: signed multiply subtract long dual
  { TRUE,  &smlsldInstruction,    0x07400050, 0x0ff000d0, "smlsld%5'x%c\t%12-15r, %16-19r, %0-3r, %8-11r" },
  // UNIMPLEMENTED: signed most significant word multiply
  { TRUE,  &smmulInstruction,     0x0750f010, 0x0ff0f0d0, "smmul%5'r%c\t%16-19r, %0-3r, %8-11r" },
  // UNIMPLEMENTED: signed most significant word multiply accumulate
  { TRUE,  &smmlaInstruction,     0x07500010, 0x0ff000d0, "smmla%5'r%c\t%16-19r, %0-3r, %8-11r, %12-15r" },
  // UNIMPLEMENTED: signed most significant word multiply subtract
  { TRUE,  &smmlsInstruction,     0x075000d0, 0x0ff000d0, "smmls%5'r%c\t%16-19r, %0-3r, %8-11r, %12-15r" },
  // UNIMPLEMENTED: signed saturate
  { TRUE,  &ssatInstruction,      0x06a00010, 0x0fe00ff0, "ssat%c\t%12-15r, #%16-20W, %0-3r" },
  { TRUE,  &ssatInstruction,      0x06a00010, 0x0fe00070, "ssat%c\t%12-15r, #%16-20W, %0-3r, lsl #%7-11d" },
  { TRUE,  &ssatInstruction,      0x06a00050, 0x0fe00070, "ssat%c\t%12-15r, #%16-20W, %0-3r, asr #%7-11d" },
  // UNIMPLEMENTED: signed saturate 16
  { TRUE,  &ssat16Instruction,    0x06a00f30, 0x0ff00ff0, "ssat16%c\t%12-15r, #%16-19W, %0-3r" },
  // UNIMPLEMENTED: unsigned saturate
  { TRUE,  &usatInstruction,      0x06e00010, 0x0fe00ff0, "usat%c\t%12-15r, #%16-20d, %0-3r" },
  { TRUE,  &usatInstruction,      0x06e00010, 0x0fe00070, "usat%c\t%12-15r, #%16-20d, %0-3r, lsl #%7-11d" },
  { TRUE,  &usatInstruction,      0x06e00050, 0x0fe00070, "usat%c\t%12-15r, #%16-20d, %0-3r, asr #%7-11d" },
  // UNIMPLEMENTED: unsigned saturate 16
  { TRUE,  &usat16Instruction,    0x06e00f30, 0x0ff00ff0, "usat16%c\t%12-15r, #%16-19d, %0-3r" },

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
  { TRUE,  &dmbInstruction,       0xf57ff050, 0xfffffff0, "dmb\t%U" },
  // sync barriers
  { TRUE,  &dsbInstruction,       0xf57ff040, 0xfffffff0, "DSB" },
  { TRUE,  &isbInstruction,       0xf57ff060, 0xfffffff0, "ISB" },
  // UNIMPLEMENTED: CLREX clear exclusive
  { TRUE,  &clrexInstruction,     0xf57ff01f, 0xffffffff, "clrex" },
  // CPS: change processor state
  { TRUE,  &cpsInstruction,       0xf1080000, 0xfffffe3f, "CPSIE" },
  { TRUE,  &cpsInstruction,       0xf10a0000, 0xfffffe20, "CPSIE" },
  { TRUE,  &cpsInstruction,       0xf10C0000, 0xfffffe3f, "CPSID" },
  { TRUE,  &cpsInstruction,       0xf10e0000, 0xfffffe20, "CPSID" },
  { TRUE,  &cpsInstruction,       0xf1000000, 0xfff1fe20, "CPS" },
  // UNIMPLEMENTED: RFE return from exception
  { TRUE,  &rfeInstruction,       0xf8100a00, 0xfe50ffff, "rfe%23?id%24?ba\t%16-19r%21'!" },
  // UNIMPLEMENTED: SETEND set endianess
  { TRUE,  &setendInstruction,    0xf1010000, 0xfffffc00, "setend\t%9?ble" },
  // UNIMPLEMENTED: SRS store return state
  { TRUE,  &srsInstruction,       0xf84d0500, 0xfe5fffe0, "srs%23?id%24?ba\t%16-19r%21'!, #%0-4d" },
  // BLX: branch and link to Thumb subroutine
  { TRUE,  &armBlxImmediateInstruction,    0xfa000000, 0xfe000000, "BLX #imm24" },
  // PLD: preload data
  { TRUE,  &pldInstruction,       0xf450f000, 0xfc70f000, "PLD" },
  // PLI: preload instruction
  { TRUE,  &pliInstruction,       0xf450f000, 0xfd70f000, "PLI" },
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
