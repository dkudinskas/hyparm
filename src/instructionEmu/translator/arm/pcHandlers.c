#include "instructionEmu/translator/blockCopy.h"
#include "instructionEmu/translator/arm/pcHandlers.h"

#include "instructionEmu/decoder/arm/structs.h"

#include "instructionEmu/interpreter/internals.h"


/*
 * Bit offsets in instruction words
 */
enum
{
  DATA_PROC_IMMEDIATE_FORM_BIT = 1 << 25,

  LDR_RT_INDEX = 12,
  LDR_LDRB_FORM_BITS = 0b11 << 25,
  LDR_LDRB_IMMEDIATE_FORM_BITS = 0b10 << 25,
  LDR_LDRB_REGISTER_FORM_BITS = LDR_LDRB_FORM_BITS,
  LDRH_LDRD_IMMEDIATE_FORM_BIT = 1 << 22,
  LDRD_BIT = 1 << 6,

  STM_ADDRESS_MODE_B_BIT = 1 << 24,
  STM_ADDRESS_MODE_I_BIT = 1 << 23,
  STM_REGISTERS_MASK = 0xFFFF,
  STM_REGISTERS_PC_BIT = 1 << GPR_PC,
  STM_WRITEBACK_BIT = 1 << 21,

  STR_RT_INDEX = LDR_RT_INDEX,
  STR_STRB_FORM_BITS = LDR_LDRB_FORM_BITS,
  STR_STRB_IMMEDIATE_FORM_BITS = LDR_LDRB_IMMEDIATE_FORM_BITS,
  STR_STRB_REGISTER_FORM_BITS = LDR_LDRB_REGISTER_FORM_BITS,
  STRH_STRD_IMMEDIATE_FORM_BIT = LDRH_LDRD_IMMEDIATE_FORM_BIT,
  STRD_BIT = LDRD_BIT,

  RD_INDEX = 12,
  RM_INDEX = 0,
  RN_INDEX = 16,

  SETFLAGS_BIT = 1 << 20,
};

/*
 * Translates common ALU instructions, imm/reg/reg-shifted-reg cases
 */
void armALUImmRegRSR(TranslationStore* ts, BasicBlock *block,
                     u32int pc, u32int instruction)
{
  ARM_ALU_imm instr = {.value = instruction};
  if (instr.fields.I)
  {
    armALUimm(ts, block, pc, instruction);
  }
  else
  {
    armALUreg(ts, block, pc, instruction);
  }
}


/*
 * Translates {CMN,CMP,TEQ,TST}: ALU ops that discard the result, hence don't 
 * have a dead register
 */
void armALUImmRegRSRNoDest(TranslationStore* ts, BasicBlock *block, u32int pc, u32int instruction)
{
  ARM_ALU_imm instr = {.value = instruction};
  if (instr.fields.I)
  {
    armALUimmNoDest(ts, block, pc, instruction);
  }
  else
  {
    armALUregNoDest(ts, block, pc, instruction);
  }

}


/*
 * Translates common ALU instructions, imm case
 */
void armALUimm(TranslationStore *ts, BasicBlock *block, u32int pc, u32int instruction)
{
  ARM_ALU_imm instr = {.value = instruction};
  if (instr.fields.Rn == GPR_PC)
  {
    DEBUG(TRANSLATION, "armALUimm: translating %08x @ %08x with cond=%x, Rd=%x, Rn=%x"
          EOL, instruction, pc, instr.fields.cond, instr.fields.Rd, instr.fields.Rn);
    armWritePCToRegister(ts, block, instr.fields.cond, instr.fields.Rd, pc);
    instr.fields.Rn = instr.fields.Rd;
  }
  addInstructionToBlock(ts, block, instr.value);
}


/*
 * Translates common ALU instructions, reg case
 */
void armALUreg(TranslationStore *ts, BasicBlock *block, u32int pc, u32int instruction)
{
  DIE_NOW(0, "armALUreg unimplemented\n");
/*  ARM_ALU_reg instr = {.value = instruction};

  u32int Rn = instr.fields.Rn;
  u32int Rm = instr.fields.Rm;
  u32int Rd = instr.fields.Rd;
  u32int cond = instr.fields.cond;
  u32int scratchRegister = 0;
  u32int pcRegister = Rd;
  bool replaceN = (Rn == GPR_PC);
  bool replaceM = (Rm == GPR_PC);
  bool spill = FALSE;

  if (replaceN || replaceM)
  {
    // This implementation expects Rd=PC to trap
    ASSERT(Rd != GPR_PC, ERROR_NOT_IMPLEMENTED);

    DEBUG(TRANSLATION, "armALUreg: translating %08x @ %08x with cond=%x, "
          "Rd=%x, Rn=%x, Rm=%x" EOL, instr.value, pc, cond, Rd, Rn, Rm);
    DIE_NOW(0, "armALUreg: unimplemented\n");
*/
    /* For the imm case, e.g. ADD Rd,Rn,#imm, we cannot have Rn=Rd because Rn=PC and Rd=PC
     * must trap. For the register case however, we can have ADD Rd,Rn,Rm with
     * - Rn=PC, Rm=Rd
     * - Rn=Rd, Rm=PC
     * In this case the instruction does not contain a 'dead' register! */
/*    if ((Rn == Rd) || (Rm == Rd))
    {
      scratchRegister = getOtherRegisterOf3(Rd, Rn, Rm);
      // register to push in priv mode
      u32int tempReg = (scratchRegister != GPR_R0) ? GPR_R0 : GPR_R1;

      // spill register
      armSpillRegister(ts, block, cond, scratchRegister, tempReg);

      pcRegister = scratchRegister;

      spill = TRUE;
    }

    armWritePCToRegister(ts, block, cond, pcRegister, pc);

    if (replaceN)
    {
      instruction = ARM_SET_REGISTER(instruction, RN_INDEX, pcRegister);
    }

    if (replaceM)
    {
      instruction = ARM_SET_REGISTER(instruction, RM_INDEX, pcRegister);
    }
  }

  addInstructionToBlock(ts, block, instruction);

  if (spill)
  {
    armRestoreRegister(ts, block, cond, scratchRegister);
  }*/
}


/*
 * Translates ALU instructions without Rd, imm case
 */
void armALUimmNoDest(TranslationStore* ts, BasicBlock *block, u32int pc, u32int instruction)
{
  ARM_ALU_imm instr = {.value = instruction};
  u32int cond = instr.fields.cond;
  u32int Rn = instr.fields.cond;

  if (instr.fields.Rn == GPR_PC)
  {
    printf("armALUimmNoDest: translating %08x @ %08x cc=%x Rn=%x\n", instr.value, pc, cond, Rn);
    DIE_NOW(0, "armALUimmNoDest UNIMPLEMENTED\n");
    // must spill a register to put correct PC value in
    // since there is no destination register to be 'dead'
  }
  addInstructionToBlock(ts, block, instruction);
}


/*
 * Translates ALU instructions without Rd, reg case
 */
void armALUregNoDest(TranslationStore* ts, BasicBlock *block, u32int pc, u32int instruction)
{
  const u32int conditionCode = ARM_EXTRACT_CONDITION_CODE(instruction);
  const bool immediateForm = (instruction & DATA_PROC_IMMEDIATE_FORM_BIT);
  const u32int operandNRegister = ARM_EXTRACT_REGISTER(instruction, RN_INDEX);
  const u32int operandMRegister = ARM_EXTRACT_REGISTER(instruction, RM_INDEX);

  DEBUG(TRANSLATION, "armALUregNoDest: translating %#.8x @ %#.8x with cond=%x, "
        "immediateForm=%x, Rn=%x, Rm=%x" EOL, instruction, pc, conditionCode, immediateForm,
        operandNRegister, operandMRegister);
  DIE_NOW(0, "armALUregNoDest UNIMPLEMENTED\n");
/*

  u32int pcRegister;
  bool replaceN = operandNRegister == GPR_PC;
  bool replaceM = !immediateForm && (operandMRegister == GPR_PC);

  if (replaceN || replaceM)
  {
    DEBUG(TRANSLATION, "armDPImmRegRSRNoDest: translating %#.8x @ %#.8x with cond=%x, "
          "immediateForm=%x, Rn=%x, Rm=%x" EOL, instruction, pc, conditionCode, immediateForm,
          operandNRegister, operandMRegister);

    pcRegister = immediateForm ? 0 : getOtherRegisterOf2(operandNRegister, operandMRegister);
    DIE_NOW(0, "armDPImmRegRSRNoDest: backupRegisterToSpill unimplemented\n");

    if (replaceN)
    {
      instruction = ARM_SET_REGISTER(instruction, RN_INDEX, pcRegister);
    }

    if (replaceM)
    {
      instruction = ARM_SET_REGISTER(instruction, RM_INDEX, pcRegister);
    }
  }

  addInstructionToBlock(ts, block, instruction);

  if (replaceN || replaceM)
  {
    DIE_NOW(context, "armDPImmRegRSRNoDest: unimplemented\n");
  }*/
}



/* this translates LDR(B) immediate case */
void armLdrPCImm(TranslationStore* ts, BasicBlock *block, u32int pc, u32int instruction)
{
  ARM_ldr_str_imm instr = {.value = instruction};
  u32int cond = instr.fields.cond;
  u32int Rt = instr.fields.Rt;
  u32int Rn = instr.fields.Rn;

  // Rt = PC -> hypercalls
  // Rn left to check:
  if (Rn == GPR_PC)
  {
    // Rn is PC, and Rn != Rt; so we can put correct value into Rt
    DEBUG(TRANSLATION, "armLdrPCImm: translating %#.8x @ %#.8x with cond=%x, Rt=%x, "
                       "Rn=%x" EOL, instruction, pc, cond, Rt, Rn);
    armWritePCToRegister(ts, block, cond, Rt, pc);
    instr.fields.Rn = Rt;
  }
  addInstructionToBlock(ts, block, instr.value);
}


/* this translates LDR(B) register case */
void armLdrPCReg(TranslationStore* ts, BasicBlock *block, u32int pc, u32int instruction)
{
  ARM_ldr_str_reg instr = {.value = instruction};
  u32int cond = instr.fields.cond;
  u32int Rt = instr.fields.Rt;
  u32int Rn = instr.fields.Rn;
  u32int Rm = instr.fields.Rm;

  u32int scratch = 0;
  u32int pcRegister = Rt;
  bool spill = FALSE;

  // Rm = PC -> unpredictable
  // Rt = PC -> hypercalls
  // Rn left to check:
  if (Rn == GPR_PC)
  {
    // Rn is PC, have to put correct value somewhere
    DEBUG(TRANSLATION, "armLdrPCReg: translating %#.8x @ %#.8x with cond=%x, Rt=%x, "
          "Rn=%x Rm=%x" EOL, instruction, pc, cond, Rt, Rn, Rm);

    // if Rm = Rt we can't use Rd to store PC value in, and need to spill a reg
    if (Rt == Rm)
    {
      scratch = getOtherRegisterOf2(Rt, Rm);
      armSpillRegister(ts, block, cond, scratch, 0);
      pcRegister = scratch;
      spill = TRUE;
    }

    armWritePCToRegister(ts, block, cond, pcRegister, pc);
    instr.fields.Rn = pcRegister;
  }

  addInstructionToBlock(ts, block, instr.value);

  if (spill)
  {
    armRestoreRegister(ts, block, cond, scratch);
  }
}


/*
 * Translates LDR(B) in register & immediate forms for which Rd!=PC
 */
void armLdrPCInstruction(TranslationStore* ts, BasicBlock *block, u32int pc, u32int instruction)
{
  ARM_ldr_str_imm instr = {.value = instruction};
  if (instr.fields.I == 0)
  {
    armLdrPCImm(ts, block, pc, instruction);
  }
  else
  {
    // register case
    armLdrPCReg(ts, block, pc, instruction);
  }
}



void armLdrdhPCInstruction(TranslationStore* ts, BasicBlock *block, u32int pc, u32int instruction)
{
  printf("armLdrdhPCInstruction: pc %08x instr %08x\n", pc, instruction);
  DIE_NOW(0, "armLdrdhPCInstruction unimplemented\n");
}


/*
 * Translates MOV for which Rd!=PC.
 */
void armMovPCInstruction(TranslationStore* ts, BasicBlock *block, u32int pc, u32int instruction)
{
  ARM_ALU_reg instr = {.value = instruction};
  u32int cond = instr.fields.cond;
  u32int Rd = instr.fields.Rd;
  u32int Rm = instr.fields.Rm;
  bool S = instr.fields.S;

  // A MOV Rd,PC can be translated to only MOVT & MOVW.
  // MOVS requires an update of the condition flags and 
  // since MOVT does not support setting flags, an extra MOVS Rd,Rd is required.
  if (Rm == GPR_PC)
  {
    // This implementation expects Rd=PC to trap
    ASSERT(Rd != GPR_PC, ERROR_NOT_IMPLEMENTED);

    DEBUG(TRANSLATION, "armMovPCInstruction: translating %#.8x @ %#.8x with cond=%x, S=%x, Rd=%x, "
          "Rm=%x" EOL, instruction, pc, cond, S, Rd, Rm);

    armWritePCToRegister(ts, block, cond, Rd, pc);

    if (!S)
    {
      return;
    }
    instr.fields.Rm = Rd;
  }

  addInstructionToBlock(ts, block, instr.value);
}


/* Translates {ASR,LSL,LSR,MVN,ROR,RRX} in immediate forms for which Rd!=PC. */
void armShiftPCImm(TranslationStore* ts, BasicBlock *block, u32int pc, u32int instruction)
{
  ARM_ALU_reg instr = {.value = instruction};

  u32int cond = instr.fields.cond;
  u32int Rd = instr.fields.Rd;
  u32int Rm = instr.fields.Rm;

  if (Rm == GPR_PC)
  {
    ASSERT(Rd != GPR_PC, ERROR_NOT_IMPLEMENTED);

    DEBUG(TRANSLATION, "armShiftPCImm: translating %08x @ %08x with cond=%x, Rd=%x, "
          "Rm=%x" EOL, instr.value, pc, cond, Rd, Rm);

    armWritePCToRegister(ts, block, cond, Rd, pc);
    instr.fields.Rm = Rd;
  }
  addInstructionToBlock(ts, block, instr.value);
}


void armStmPC(TranslationStore* ts, BasicBlock *block, u32int pc, u32int instruction)
{
  DEBUG(TRANSLATION, "armStmPC: PC=%x, instruction %08x" EOL, pc, instruction);
  DEBUG(TRANSLATION, "armStmPC: block %p csStart %p" EOL, block, block->codeStoreStart);

  ARM_ldm_stm stm;
  stm.value = instruction;

  // first, simply add the instruction, which might store an incorrect PC value
  addInstructionToBlock(ts, block, instruction);

  if ((stm.fields.register_list & 0x8000) != 0)
  {
    // PC was in list. we have to fix up the incorrectly stored value.
    u16int regList = stm.fields.register_list;
    u32int Rn      = stm.fields.Rn;
    bool wback     = stm.fields.W;
    bool add       = stm.fields.U;
    bool index     = stm.fields.P;
    u32int cond    = stm.fields.cond;
    u32int scratch, Rn_offs;

    // set scratch register to be different from base register of STM
    scratch = (Rn == 0) ? 1 : 0;

    // push scratch register.
    ARM_ldm_stm push;
    push.value = LDM_STM_BASE_VALUE;
    push.fields.register_list = 1 << scratch;
    push.fields.Rn = 13;
    push.fields.L  = 0;
    push.fields.W  = 1;
    push.fields.S  = 0;
    push.fields.U  = 0;
    push.fields.P  = 1;
    push.fields.cond = cond;
    addInstructionToBlock(ts, block, push.value);

    // put correct guest PC into scratch
    armWritePCToRegister(ts, block, cond, scratch, pc);

    bool addOffset = FALSE;
    // now we must calculate the offset where to store correct PC
    if (wback)
    {
      if (add)
      {
        DIE_NOW(0, "armStmPC: add unimplemented\n");
      }
      else
      {
        if (index)
        {
          // STMDB (pre) address offset before transfer
          // the highest register in the list (PC) is stored in old Rn-4
          // since this is writeback case, current Rn = R[n] - 4*BitCount(registers);
          Rn_offs = countBitsSet(regList)*4 - 4;
          addOffset = TRUE;

          if (Rn == 13)
          {
            // guest push'ing, and we also pushed 1 more word onto guest stack
            // adjust offset to account for it. +4
            Rn_offs += 4;
          }
        }
        else
        {
          DIE_NOW(0, "armStmPC: post unimplemented\n");
          // STMDA (post) address offset after transfer
          // the highest register in the list (PC) is stored in Rn
        }
      } // U = 0
    } // back
    else
    {
      DIE_NOW(0, "armStmPC: wback 0 unimplemented\n");
    }

    // we know where to put the correct PC value: correct it!
    ARM_ldr_str_imm str;
    str.value = STR_IMMEDIATE_BASE_VALUE;
    str.fields.imm12 = Rn_offs;
    str.fields.Rt = scratch;
    str.fields.Rn = Rn;
    str.fields.L  = 0;
    str.fields.W  = 0;
    str.fields.B  = 0;
    str.fields.U  = addOffset;
    str.fields.P  = 1;
    str.fields.I  = 0;
    str.fields.cond = cond;
    addInstructionToBlock(ts, block, str.value);

    // pop scratch register.
    ARM_ldm_stm pop;
    pop.value = LDM_STM_BASE_VALUE;
    pop.fields.register_list = 1 << scratch;
    pop.fields.Rn = 13;
    pop.fields.L  = 1;
    pop.fields.W  = 1;
    pop.fields.S  = 0;
    pop.fields.U  = 1;
    pop.fields.P  = 0;
    pop.fields.cond = cond;
    addInstructionToBlock(ts, block, pop.value);
  }
}


/*
 * Translates STR{,B,H,D} in register & immediate forms
 */
void armStrPCImm(TranslationStore* ts, BasicBlock *block, u32int pc, u32int instruction)
{
  ARM_ldr_str_imm instr = {.value = instruction};
  u32int cond = instr.fields.cond;
  u32int Rt = instr.fields.Rt;
  u32int Rn = instr.fields.Rn;

  u32int scratch = 0;
  u32int pcRegister = Rt;
  bool spill = FALSE;

  const bool replaceRt = (Rt == GPR_PC);
  const bool replaceRn = (Rn == GPR_PC);

  if (replaceRt || replaceRn)
  {
    if (!(instruction & STR_STRB_FORM_BITS) && (instruction & STRD_BIT))
    {
      DIE_NOW(0, "strd");
    }
    else
    {
      //Instruction is STR{,B,H} immediate: only 2 registers used!
      scratch = getOtherRegisterOf2(Rt, Rn);
      armSpillRegister(ts, block, cond, scratch, 0);
      pcRegister = scratch;
      spill = TRUE;
    }

    armWritePCToRegister(ts, block, cond, pcRegister, pc);

    if (replaceRt)
    {
      instr.fields.Rt = pcRegister;
    }
    if (replaceRn)
    {
      instr.fields.Rn = pcRegister;
    }
  }

  addInstructionToBlock(ts, block, instr.value);

  if (spill)
  {
    armRestoreRegister(ts, block, cond, scratch);
  }
}


void armStrPCReg(TranslationStore* ts, BasicBlock *block, u32int pc, u32int instruction)
{
  ARM_ldr_str_reg instr = {.value = instruction};
  u32int Rt = instr.fields.Rt;
  u32int Rn = instr.fields.Rn;

  const bool replaceRt = (Rt == GPR_PC);
  const bool replaceRn = (Rn == GPR_PC);

  if (replaceRt || replaceRn)
  {
    DIE_NOW(0, "armStrPCInstruction: spill register stub.\n");
  }

  addInstructionToBlock(ts, block, instruction);

  if (replaceRt || replaceRn)
  {
    DIE_NOW(0, "armStrPCInstruction: restore register stub.\n");
  }
}


/*
 * Translates STR(B) in register & immediate forms for which Rd!=PC
 */
void armStrPCInstruction(TranslationStore* ts, BasicBlock *block, u32int pc, u32int instruction)
{
  ARM_ldr_str_imm instr = {.value = instruction};
  if (instr.fields.I == 0)
  {
    armStrPCImm(ts, block, pc, instruction);
  }
  else
  {
    // register case
    armStrPCReg(ts, block, pc, instruction);
  }
}



void armStrtPCInstruction(TranslationStore* ts, BasicBlock *block, u32int pc, u32int instruction)
{
  DIE_NOW(0, "armStrPCInstruction unimplemented\n");
  const u32int conditionCode = ARM_EXTRACT_CONDITION_CODE(instruction);
  const u32int sourceRegister = ARM_EXTRACT_REGISTER(instruction, STR_RT_INDEX);
  const u32int baseRegister = ARM_EXTRACT_REGISTER(instruction, RN_INDEX);

  const bool replaceSource = sourceRegister == GPR_PC;
  const bool replaceBase = baseRegister == GPR_PC;
  u32int scratchRegister;

  if (replaceSource || replaceBase)
  {
    printf("armStrPCInstruction: translating %#.8x @ %#.8x with cond=%x, Rd=%x, "
          "Rt=%x" EOL, instruction, pc, conditionCode, sourceRegister, baseRegister);
    DIE_NOW(0, "armStrPCInstruction: translate, stub\n");
  }

  addInstructionToBlock(ts, block, instruction);

  if (replaceSource || replaceBase)
  {
    DIE_NOW(0, "armStrPCInstruction: restore register stub.\n");
    // Restore register; done.
    armRestoreRegister(ts, block, conditionCode, scratchRegister);
  }
}

