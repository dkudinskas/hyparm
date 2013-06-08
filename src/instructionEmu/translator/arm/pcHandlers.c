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


void armALUImmRegRSR(TranslationStore* ts, BasicBlock *block,
                     u32int pc, u32int instruction)
{
  bool immediateForm = (instruction & DATA_PROC_IMMEDIATE_FORM_BIT);
  if (immediateForm)
  {
    armALUimm(ts, block, pc, instruction);
  }
  else
  {
    armALUreg(ts, block, pc, instruction);
  }
}


void armALUimm(TranslationStore *ts, BasicBlock *block, u32int pc, u32int instruction)
{
  ARM_ALU_imm instr;
  instr.value = instruction;

  u32int Rd   = instr.fields.Rd;
  u32int Rn   = instr.fields.Rn;
  u32int cond = instr.fields.cond;

  if (Rn == GPR_PC)
  {
    DEBUG(TRANSLATION, "armALUimm: translating %08x @ %08x with cond=%x, Rd=%x, Rn=%x"
          EOL, instruction, pc, cond, Rd, Rn);

    armWritePCToRegister(ts, block, cond, Rd, pc);
    instruction = ARM_SET_REGISTER(instruction, RN_INDEX, Rd);
  }
  addInstructionToBlock(ts, block, instruction);
}


void armALUreg(TranslationStore *ts, BasicBlock *block, u32int pc, u32int instruction)
{
  ARM_ALU_reg instr;
  instr.value = instruction;

  u32int Rd   = instr.fields.Rd;
  u32int Rn   = instr.fields.Rn;
  u32int Rm   = instr.fields.Rm;
  u32int cond = instr.fields.cond;
  u32int conditionCode = ARM_EXTRACT_CONDITION_CODE(instruction);

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
          "Rd=%x, Rn=%x, Rm=%x" EOL, instruction, pc, conditionCode, Rd, Rn, Rm);

    /* For the imm case, e.g. ADD Rd,Rn,#imm, we cannot have Rn=Rd because Rn=PC and Rd=PC
     * must trap. For the register case however, we can have ADD Rd,Rn,Rm with
     * - Rn=PC, Rm=Rd
     * - Rn=Rd, Rm=PC
     * In this case the instruction does not contain a 'dead' register! */
    if ((Rn == Rd) || (Rm == Rd))
    {
      scratchRegister = getOtherRegisterOf3(Rd, Rn, Rm);

      // register to push in priv mode
      u32int tempReg = (scratchRegister != GPR_R0) ? GPR_R0 : GPR_R1;

      // spill register
      printf("armALUreg: armSpillRegister\n");
      armSpillRegister(ts, block, conditionCode, scratchRegister, tempReg);

      pcRegister = scratchRegister;

      spill = TRUE;
    }

    armWritePCToRegister(ts, block, conditionCode, pcRegister, pc);

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
    armRestoreRegister(ts, block, conditionCode, scratchRegister);
  }
}


/*
 * Translates {CMN,CMP,TEQ,TST}.
 *
 * All these instructions are data processing instructions which update the condition flags in the
 * PSR and discard their result. Because there is no destination register, we need to spill some
 * register for the PC...
 */
void armDPImmRegRSRNoDest(TranslationStore* ts, BasicBlock *block, u32int pc, u32int instruction)
{
  const u32int conditionCode = ARM_EXTRACT_CONDITION_CODE(instruction);
  const bool immediateForm = (instruction & DATA_PROC_IMMEDIATE_FORM_BIT);
  const u32int operandNRegister = ARM_EXTRACT_REGISTER(instruction, RN_INDEX);
  const u32int operandMRegister = ARM_EXTRACT_REGISTER(instruction, RM_INDEX);

  u32int pcRegister;
  bool replaceN = operandNRegister == GPR_PC;
  bool replaceM = !immediateForm && (operandMRegister == GPR_PC);

  if (replaceN || replaceM)
  {
    DEBUG(TRANSLATION, "armDPImmRegRSRNoDest: translating %#.8x @ %#.8x with cond=%x, "
          "immediateForm=%x, Rn=%x, Rm=%x" EOL, instruction, pc, conditionCode, immediateForm,
          operandNRegister, operandMRegister);

    pcRegister = immediateForm ? 0 : getOtherRegisterOf2(operandNRegister, operandMRegister);
    DIE_NOW(0, "armDPImmRegRSRNoDest: backupRegisterToSpill stop point.\n");
//    armBackupRegisterToSpill(ts, block, conditionCode, pcRegister);
//    armWritePCToRegister(ts, block, conditionCode, pcRegister, pc);

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
    DIE_NOW(context, "armDPImmRegRSRNoDest: armRestoreRegisterFromSpill stop point.\n");
//    armRestoreRegisterFromSpill(ts, block, conditionCode, pcRegister);
  }
}


/*
 * Translates LDR{,B,H,D} in register & immediate forms for which Rd!=PC
 */
void armLdrPCInstruction(TranslationStore* ts, BasicBlock *block, u32int pc, u32int instruction)
{
  const u32int conditionCode = ARM_EXTRACT_CONDITION_CODE(instruction); 
  const u32int regD = ARM_EXTRACT_REGISTER(instruction, LDR_RT_INDEX);
  const u32int regN = ARM_EXTRACT_REGISTER(instruction, RN_INDEX);
  const u32int regM = ARM_EXTRACT_REGISTER(instruction, RM_INDEX);

  u32int scratchRegister = 0;
  u32int pcRegister = regD;
  bool spill = FALSE;

  if (regN == GPR_PC)
  {
    ASSERT(regD != GPR_PC, "Rd=PC: trap (LDR) or unpredictable (LDR[BHD])");

    DEBUG(TRANSLATION, "armLdrPCInstruction: translating %#.8x @ %#.8x with cond=%x, Rd=%x, "
          "Rt=%x" EOL, instruction, pc, conditionCode, regD, regN);

    /* In here, we know that Rd!=PC and Rn=PC; hence Rd!=Rn. 
     * For the register case we still need to check whether Rm=Rd
     * since Rd cannot be used to store the PC in that case. */
    switch (instruction & LDR_LDRB_REGISTER_FORM_BITS)
    {
      case 0:
      {
        if ((instruction & LDRD_BIT))
        {
          // LDRD writes to 2 registers; if the first is LR the next is PC: unpredictable!
          ASSERT(regD != GPR_LR, ERROR_UNPREDICTABLE_INSTRUCTION);
        }
        if ((instruction & LDRH_LDRD_IMMEDIATE_FORM_BIT))
        {
          break;
        }
      }
      // no break
      case LDR_LDRB_REGISTER_FORM_BITS:
      {
        ASSERT(regM != GPR_PC, ERROR_UNPREDICTABLE_INSTRUCTION);
        spill = (regM == regD);
        break;
      }
    }

    if (spill)
    {
      scratchRegister = getOtherRegisterOf2(regD, regM);

      // register to push in priv mode
      u32int tempReg = (scratchRegister != GPR_R0) ? GPR_R0 : GPR_R1;

      // spill register
      printf("armLdrPCInstruction: armSpillRegister\n");
      armSpillRegister(ts, block, conditionCode, scratchRegister, tempReg);

      pcRegister = scratchRegister;
    }

    armWritePCToRegister(ts, block, conditionCode, pcRegister, pc);
    instruction = ARM_SET_REGISTER(instruction, RN_INDEX, pcRegister);
  }

  addInstructionToBlock(ts, block, instruction);

  if (spill)
  {
    // Restore register; done.
    armRestoreRegister(ts, block, conditionCode, scratchRegister);
  }
}


/*
 * Translates MOV for which Rd!=PC.
 */
void armMovPCInstruction(TranslationStore* ts, BasicBlock *block, u32int pc, u32int instruction)
{
  const u32int conditionCode = ARM_EXTRACT_CONDITION_CODE(instruction);
  const bool setFlags = instruction & SETFLAGS_BIT;
  const u32int destinationRegister = ARM_EXTRACT_REGISTER(instruction, RD_INDEX);
  const u32int sourceRegister = ARM_EXTRACT_REGISTER(instruction, RM_INDEX);

  // A MOV Rd,PC can be translated to only MOVT & MOVW.
  // MOVS requires an update of the condition flags and 
  // since MOVT does not support setting flags, an extra MOVS Rd,Rd is required.
  if (sourceRegister == GPR_PC)
  {
    // This implementation expects Rd=PC to trap
    ASSERT(destinationRegister != GPR_PC, ERROR_NOT_IMPLEMENTED);

    DEBUG(TRANSLATION, "armMovPCInstruction: translating %#.8x @ %#.8x with cond=%x, S=%x, Rd=%x, "
          "Rm=%x" EOL, instruction, pc, conditionCode, setFlags, destinationRegister,
          sourceRegister);

    armWritePCToRegister(ts, block, conditionCode, destinationRegister, pc);
    if (!(instruction & SETFLAGS_BIT))
    {
      return;
    }
    instruction = ARM_SET_REGISTER(instruction, RM_INDEX, destinationRegister);
  }

  addInstructionToBlock(ts, block, instruction);
}


/*
 * Translates {ASR,LSL,LSR,MVN,ROR,RRX} in immediate forms for which Rd!=PC.
 *
 * These instructions are all of the form
 * cond | 0001 | 101S | (0000) | Rd | imm5 | t2 | 0 | Rm
 * The shift type is determined by 2 bits (t2). For RRX, imm5 is always 00000.
 */
void armShiftPCInstruction(TranslationStore* ts, BasicBlock *block, u32int pc, u32int instruction)
{
  const u32int conditionCode = ARM_EXTRACT_CONDITION_CODE(instruction);
  const u32int destinationRegister = ARM_EXTRACT_REGISTER(instruction, RD_INDEX);
  const u32int operandRegister = ARM_EXTRACT_REGISTER(instruction, RM_INDEX);

  if (operandRegister == GPR_PC)
  {
    // This implementation expects Rd=PC to trap
    ASSERT(destinationRegister != GPR_PC, ERROR_NOT_IMPLEMENTED);

    DEBUG(TRANSLATION, "armShiftPCInstruction: translating %08x @ %08x with cond=%x, Rd=%x, "
          "Rm=%x" EOL, instruction, pc, conditionCode, destinationRegister, operandRegister);

    armWritePCToRegister(ts, block, conditionCode, destinationRegister, pc);
    instruction = ARM_SET_REGISTER(instruction, RM_INDEX, destinationRegister);
  }

  addInstructionToBlock(ts, block, instruction);
}


void armStmPCInstruction(TranslationStore* ts, BasicBlock *block, u32int pc, u32int instruction)
{
  DEBUG(TRANSLATION, "armStmPCInstruction: PC=%x, instruction %08x" EOL, pc, instruction);
  DEBUG(TRANSLATION, "armStmPCInstruction: block %p csStart %p" EOL, block, block->codeStoreStart);

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
        DIE_NOW(0, "armStmPCInstruction: add unimplemented\n");
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
          DIE_NOW(0, "armStmPCInstruction: post unimplemented\n");
          // STMDA (post) address offset after transfer
          // the highest register in the list (PC) is stored in Rn
        }
      } // U = 0
    } // back
    else
    {
      DIE_NOW(0, "armStmPcInstruction: wback 0 unimplemented\n");
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
void armStrPCInstruction(TranslationStore* ts, BasicBlock *block, u32int pc, u32int instruction)
{
  const u32int conditionCode = ARM_EXTRACT_CONDITION_CODE(instruction);
  const u32int sourceRegister = ARM_EXTRACT_REGISTER(instruction, STR_RT_INDEX);
  const u32int baseRegister = ARM_EXTRACT_REGISTER(instruction, RN_INDEX);
  const u32int offsetRegister = ARM_EXTRACT_REGISTER(instruction, RM_INDEX);

  const bool replaceSource = sourceRegister == GPR_PC;
  const bool replaceBase = baseRegister == GPR_PC;
  u32int scratchRegister;

  if (replaceSource || replaceBase)
  {
    printf("armStrPCInstruction: translating %#.8x @ %#.8x with cond=%x, Rd=%x, "
          "Rt=%x" EOL, instruction, pc, conditionCode, sourceRegister, baseRegister);
    DIE_NOW(0, "armStrPCInstruction: translate, stub\n");
    /*
    DEBUG(TRANSLATION, "armStrPCInstruction: translating %#.8x @ %#.8x with cond=%x, Rd=%x, "
          "Rt=%x" EOL, instruction, pc, conditionCode, sourceRegister, baseRegister);

    if (!(instruction & STR_STRB_FORM_BITS) && (instruction & STRD_BIT))
    {
      // Instruction is STRD immediate or register.
      // The immediate form uses 3 registers while the register form uses 4 of them! To arrive
      // here, we have Rn=PC and Rt!=PC, so we omit the base register from getOtherRegisterOfN.
      //
      ASSERT((sourceRegister & 1) == 0, ERROR_UNPREDICTABLE_INSTRUCTION);
      ASSERT(sourceRegister != GPR_LR, ERROR_UNPREDICTABLE_INSTRUCTION);

      if (instruction & STRH_STRD_IMMEDIATE_FORM_BIT)
      {
        scratchRegister = getOtherRegisterOf2(sourceRegister, sourceRegister + 1);
      }
      else
      {
        ASSERT(offsetRegister != GPR_PC, ERROR_UNPREDICTABLE_INSTRUCTION);
        scratchRegister = getOtherRegisterOf3(sourceRegister, sourceRegister + 1, offsetRegister);
      }
    }
    else if ((instruction & STR_STRB_FORM_BITS) == STR_STRB_IMMEDIATE_FORM_BITS
             || (!(instruction & STR_STRB_FORM_BITS) && (instruction & STRH_STRD_IMMEDIATE_FORM_BIT)))
    {
      //Instruction is STR{,B,H} immediate: only 2 registers used!
      scratchRegister = getOtherRegisterOf2(sourceRegister, baseRegister);
    }
    else
    {
      ASSERT((instruction & STR_STRB_FORM_BITS) == STR_STRB_REGISTER_FORM_BITS ||
             (!(instruction & STR_STRB_FORM_BITS) && !(instruction & STRH_STRD_IMMEDIATE_FORM_BIT)),
             ERROR_BAD_ARGUMENTS);
      // Instruction is STR{,B,H} register: 3 registers used.
      ASSERT(offsetRegister != GPR_PC, ERROR_UNPREDICTABLE_INSTRUCTION);
      scratchRegister = getOtherRegisterOf3(sourceRegister, baseRegister, offsetRegister);
    }

    armBackupRegisterToSpill(ts, block, conditionCode, scratchRegister);
    armWritePCToRegister(ts, block, conditionCode, scratchRegister, pc);

    if (replaceSource)
    {
      instruction = ARM_SET_REGISTER(instruction, STR_RT_INDEX, scratchRegister);
    }

    if (replaceBase)
    {
      instruction = ARM_SET_REGISTER(instruction, RN_INDEX, scratchRegister);
    }*/
  }

  addInstructionToBlock(ts, block, instruction);

  if (replaceSource || replaceBase)
  {
    DIE_NOW(0, "armStrPCInstruction: restore register stub.\n");
    // Restore register; done.
    armRestoreRegister(ts, block, conditionCode, scratchRegister);
  }
}


void armStrtPCInstruction(TranslationStore* ts, BasicBlock *block, u32int pc, u32int instruction)
{
  const u32int conditionCode = ARM_EXTRACT_CONDITION_CODE(instruction);
  const u32int sourceRegister = ARM_EXTRACT_REGISTER(instruction, STR_RT_INDEX);
  const u32int baseRegister = ARM_EXTRACT_REGISTER(instruction, RN_INDEX);
  const u32int offsetRegister = ARM_EXTRACT_REGISTER(instruction, RM_INDEX);

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

