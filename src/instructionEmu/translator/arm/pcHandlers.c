#include "instructionEmu/translator.h"

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
 * Translates {ADC,ADD,AND,BIC,EOR,ORR,RSB,RSC,SBC,SUB} for which Rd!=PC, in both immediate and
 * register flavours. All these instructions have 3 flavours. The third one is register-shifted
 * register, which is unpredictable if any register is the PC.
 *
 * The immediate flavor has only one operand register (Rn at RN_INDEX). The register flavor has an
 * extra operand register (Rm at RM_INDEX).
 *
 * All these instructions are data processing instructions which store their result into a some Rd.
 */
void armDPImmRegRSR(TranslationCache *tc, ARMTranslationInfo *block, u32int pc, u32int instruction)
{
  const u32int conditionCode = ARM_EXTRACT_CONDITION_CODE(instruction);
  const bool immediateForm = (instruction >> 25 & DATA_PROC_IMMEDIATE_FORM_BIT);
  const u32int destinationRegister = ARM_EXTRACT_REGISTER(instruction, RD_INDEX);
  const u32int operandNRegister = ARM_EXTRACT_REGISTER(instruction, RN_INDEX);
  const u32int operandMRegister = ARM_EXTRACT_REGISTER(instruction, RM_INDEX);

  u32int pcRegister = destinationRegister;
  bool replaceN = operandNRegister == GPR_PC;
  bool replaceM = !immediateForm && operandMRegister == GPR_PC;
  bool restoreFromSpill = FALSE;

  if (replaceN || replaceM)
  {
    // This implementation expects Rd=PC to trap
    ASSERT(destinationRegister != GPR_PC, ERROR_NOT_IMPLEMENTED);

    DEBUG(TRANSLATION, "armDPImmRegRSR: translating %#.8x @ %#.8x with cond=%x, immediateForm=%x, "
          "Rd=%x, Rn=%x, Rm=%x" EOL, instruction, pc, conditionCode, immediateForm,
          destinationRegister, operandNRegister, operandMRegister);

    /*
     * For the immediate case, e.g. ADD Rd,Rn,#imm, we cannot have Rn=Rd because Rn=PC and Rd=PC
     * must trap. For the register case however, we can have ADD Rd,Rn,Rm with
     * - Rn=PC, Rm=Rd
     * - Rn=Rd, Rm=PC
     * In this case the instruction does not contain a 'dead' register!
     */
    if (!immediateForm && (operandNRegister == destinationRegister || operandMRegister == destinationRegister))
    {
      const u32int scratchRegister = getOtherRegisterOf3(destinationRegister, operandNRegister, operandMRegister);
      armBackupRegisterToSpill(tc, block, conditionCode, scratchRegister);
      pcRegister = scratchRegister;
      restoreFromSpill = TRUE;
    }

    armWritePCToRegister(tc, block, conditionCode, pcRegister, pc);

    if (replaceN)
    {
      instruction = ARM_SET_REGISTER(instruction, RN_INDEX, pcRegister);
    }

    if (replaceM)
    {
      instruction = ARM_SET_REGISTER(instruction, RM_INDEX, pcRegister);
    }
  }

  armWriteToCodeCache(tc, block, instruction);

  if (restoreFromSpill)
  {
    armRestoreRegisterFromSpill(tc, block, conditionCode, pcRegister);
  }
}

/*
 * Translates {CMN,CMP,TEQ,TST}.
 *
 * All these instructions are data processing instructions which update the condition flags in the
 * PSR and discard their result. Because there is no destination register, we need to spill some
 * register for the PC...
 */
void armDPImmRegRSRNoDest(TranslationCache *tc, ARMTranslationInfo *block, u32int pc, u32int instruction)
{
  const u32int conditionCode = ARM_EXTRACT_CONDITION_CODE(instruction);
  const bool immediateForm = (instruction >> 25 & DATA_PROC_IMMEDIATE_FORM_BIT);
  const u32int operandNRegister = ARM_EXTRACT_REGISTER(instruction, RN_INDEX);
  const u32int operandMRegister = ARM_EXTRACT_REGISTER(instruction, RM_INDEX);

  u32int pcRegister;
  bool replaceN = operandNRegister == GPR_PC;
  bool replaceM = !immediateForm && operandMRegister == GPR_PC;

  if (replaceN || replaceM)
  {
    DEBUG(TRANSLATION, "armDPImmRegRSRNoDest: translating %#.8x @ %#.8x with cond=%x, "
          "immediateForm=%x, Rn=%x, Rm=%x" EOL, instruction, pc, conditionCode, immediateForm,
          operandNRegister, operandMRegister);

    pcRegister = immediateForm ? 0 : getOtherRegisterOf2(operandNRegister, operandMRegister);
    armBackupRegisterToSpill(tc, block, conditionCode, pcRegister);
    armWritePCToRegister(tc, block, conditionCode, pcRegister, pc);

    if (replaceN)
    {
      instruction = ARM_SET_REGISTER(instruction, RN_INDEX, pcRegister);
    }

    if (replaceM)
    {
      instruction = ARM_SET_REGISTER(instruction, RM_INDEX, pcRegister);
    }
  }

  armWriteToCodeCache(tc, block, instruction);

  if (replaceN || replaceM)
  {
    armRestoreRegisterFromSpill(tc, block, conditionCode, pcRegister);
  }
}

/*
 * Translates LDR{,B,H,D} in register & immediate forms for which Rd!=PC
 */
void armLdrPCInstruction(TranslationCache *tc, ARMTranslationInfo *block, u32int pc, u32int instruction)
{
  const u32int conditionCode = ARM_EXTRACT_CONDITION_CODE(instruction);
  const u32int destinationRegister = ARM_EXTRACT_REGISTER(instruction, LDR_RT_INDEX);
  const u32int baseRegister = ARM_EXTRACT_REGISTER(instruction, RN_INDEX);
  const u32int offsetRegister = ARM_EXTRACT_REGISTER(instruction, RM_INDEX);

  u32int pcRegister = destinationRegister;
  bool spill = FALSE;

  if (baseRegister == GPR_PC)
  {
    ASSERT(destinationRegister != GPR_PC, "Rd=PC: trap (LDR) or unpredictable (LDR[BHD])");

    DEBUG(TRANSLATION, "armLdrPCInstruction: translating %#.8x @ %#.8x with cond=%x, Rd=%x, "
          "Rt=%x" EOL, instruction, pc, conditionCode, destinationRegister, baseRegister);

    /*
     * In here, we know that Rd!=PC and Rn=PC; hence Rd!=Rn. For the register case we still need to
     * check whether Rm=Rd, since Rd cannot be used to store the PC in that case.
     */
    switch (instruction & LDR_LDRB_REGISTER_FORM_BITS)
    {
      case 0:
      {
        if ((instruction & LDRD_BIT))
        {
          // LDRD writes to 2 registers; if the first is LR the next is PC: unpredictable!
          ASSERT(destinationRegister != GPR_LR, ERROR_UNPREDICTABLE_INSTRUCTION);
        }
        if ((instruction & LDRH_LDRD_IMMEDIATE_FORM_BIT))
        {
          break;
        }
      }
      /* no break */
      case LDR_LDRB_REGISTER_FORM_BITS:
      {
        ASSERT(offsetRegister != GPR_PC, ERROR_UNPREDICTABLE_INSTRUCTION);
        spill = offsetRegister == destinationRegister;
        break;
      }
    }

    if (spill)
    {
      const u32int scratchRegister = getOtherRegisterOf2(destinationRegister, offsetRegister);
      armBackupRegisterToSpill(tc, block, conditionCode, scratchRegister);
      pcRegister = scratchRegister;
    }

    armWritePCToRegister(tc, block, conditionCode, pcRegister, pc);
    instruction = ARM_SET_REGISTER(instruction, RN_INDEX, pcRegister);
  }

  armWriteToCodeCache(tc, block, instruction);

  if (spill)
  {
    armRestoreRegisterFromSpill(tc, block, conditionCode, pcRegister);
  }
}

/*
 * Translates MOV for which Rd!=PC.
 */
void armMovPCInstruction(TranslationCache *tc, ARMTranslationInfo *block, u32int pc, u32int instruction)
{
  const u32int conditionCode = ARM_EXTRACT_CONDITION_CODE(instruction);
  const bool setFlags = instruction & SETFLAGS_BIT;
  const u32int destinationRegister = ARM_EXTRACT_REGISTER(instruction, RD_INDEX);
  const u32int sourceRegister = ARM_EXTRACT_REGISTER(instruction, RM_INDEX);

  /*
   * A MOV Rd,PC can be translated to only MOVT & MOVW. MOVS requires an update of the condition
   * flags and since MOVT does not support setting flags, an extra MOVS Rd,Rd is required.
   */
  if (sourceRegister == GPR_PC)
  {
    // This implementation expects Rd=PC to trap
    ASSERT(destinationRegister != GPR_PC, ERROR_NOT_IMPLEMENTED);

    DEBUG(TRANSLATION, "armMovPCInstruction: translating %#.8x @ %#.8x with cond=%x, S=%x, Rd=%x, "
          "Rm=%x" EOL, instruction, pc, conditionCode, setFlags, destinationRegister,
          sourceRegister);

    armWritePCToRegister(tc, block, conditionCode, destinationRegister, pc);
    if (!(instruction & SETFLAGS_BIT))
    {
      return;
    }
    instruction = ARM_SET_REGISTER(instruction, RM_INDEX, destinationRegister);
  }

  armWriteToCodeCache(tc, block, instruction);
}

/*
 * Translates {ASR,LSL,LSR,MVN,ROR,RRX} in immediate forms for which Rd!=PC.
 *
 * These instructions are all of the form
 * cond | 0001 | 101S | (0000) | Rd | imm5 | t2 | 0 | Rm
 * The shift type is determined by 2 bits (t2). For RRX, imm5 is always 00000.
 */
void armShiftPCInstruction(TranslationCache *tc, ARMTranslationInfo *block, u32int pc, u32int instruction)
{
  const u32int conditionCode = ARM_EXTRACT_CONDITION_CODE(instruction);
  const u32int destinationRegister = ARM_EXTRACT_REGISTER(instruction, RD_INDEX);
  const u32int operandRegister = ARM_EXTRACT_REGISTER(instruction, RM_INDEX);

  if (operandRegister == GPR_PC)
  {
    // This implementation expects Rd=PC to trap
    ASSERT(destinationRegister != GPR_PC, ERROR_NOT_IMPLEMENTED);

    DEBUG(TRANSLATION, "armShiftPCInstruction: translating %#.8x @ %#.8x with cond=%x, Rd=%x, "
          "Rm=%x" EOL, instruction, pc, conditionCode, destinationRegister, operandRegister);

    armWritePCToRegister(tc, block, conditionCode, destinationRegister, pc);
    instruction = ARM_SET_REGISTER(instruction, RM_INDEX, destinationRegister);
  }

  armWriteToCodeCache(tc, block, instruction);
}

void armStmPCInstruction(TranslationCache *tc, ARMTranslationInfo *block, u32int pc, u32int instruction)
{
  if ((instruction & STM_REGISTERS_PC_BIT))
  {
    const u32int baseRegister = ARM_EXTRACT_REGISTER(instruction, RN_INDEX);
    const u32int conditionCode = ARM_EXTRACT_CONDITION_CODE(instruction);
    ASSERT(baseRegister != GPR_PC, ERROR_UNPREDICTABLE_INSTRUCTION);
    u32int scratchRegister;
    /*
     * The highest-numbered register is always stored to the highest memory address. Given that the
     * PC is included in the list, it is the PC that will be stored at the highest memory address.
     * If any lower-numbered register not in the list is higher-numbered than all the registers in
     * the list, we can use it as substitute for the PC. However, we cannot make use of the base
     * register. Hence, we look for the lowest available register, and increment it if we hit the
     * base register. If that's still lower-numbered than the PC, we have a suitable substitute.
     */
    const u32int remainingRegisters = instruction & STM_REGISTERS_MASK & ~STM_REGISTERS_PC_BIT;
    scratchRegister = findLastBitSet(remainingRegisters);
    if (scratchRegister == baseRegister)
    {
      scratchRegister++;
    }
    if (scratchRegister < GPR_PC)
    {
      /*
       * We have found a substitute register for the PC, different from the base register.
       */
      DEBUG(TRANSLATION, "armStmPCInstruction: substituting r%u for PC" EOL, scratchRegister);
      armBackupRegisterToSpill(tc, block, conditionCode, scratchRegister);
      armWritePCToRegister(tc, block, conditionCode, scratchRegister, pc);
      armWriteToCodeCache(tc, block, (instruction & ~STM_REGISTERS_PC_BIT) | (1 << scratchRegister));
    }
    else
    {
      /*
       * We could not find a suitable substitute register for the PC. The most generic solution
       * here is to split the STM instruction into an equivalent sequence of STM without the PC and
       * STR of the PC value. Using STM for storing the PC value is possible but complicates things
       * as STM cannot take an immediate offset, which we'd require for the case without writeback.
       * This can in turn be solved by adding extra ADD/SUB instructions as required, but those can
       * mostly be avoided when using STR.
       *
       * NOTE: if we end up here, there MUST be registers other than PC in the list (otherwise, we
       *       would have found a substitute register).
       *
       * The exact sequence in which an STM is split depends on the memory address mode encoded in
       * the instruction (DA/DB/IA/IB).
       */
      const bool before = instruction & STM_ADDRESS_MODE_B_BIT;
      const bool writeback = instruction & STM_WRITEBACK_BIT;
      DEBUG(TRANSLATION, "armStmPCInstruction: before=%x, writeback=%x, Rn=r%u, remaining=%#.4x "
            "(cannot substitute PC)" EOL, before, writeback, baseRegister, remainingRegisters);
      if (instruction & STM_ADDRESS_MODE_I_BIT)
      {
        /*
         * For STMIA/STMIB (increment after or before):
         * - the lowest-numbered register is written at the address in Rn (IA) or Rn + 4 (IB);
         * - each next register is written at offset 4 of the last one;
         * - if writeback is set, Rn is incremented by 4*countBitsSet(registers).
         *
         * First perform STMIA/STMIB for all remaining registers:
         */
        armWriteToCodeCache(tc, block, instruction & ~STM_REGISTERS_PC_BIT);
        /*
         * Find another register, spill it and use it for the PC value
         */
        scratchRegister = getOtherRegister(baseRegister);
        DEBUG(TRANSLATION, "armStmPCInstruction: spilling and reusing r%u" EOL, scratchRegister);
        armBackupRegisterToSpill(tc, block, conditionCode, scratchRegister);
        armWritePCToRegister(tc, block, conditionCode, scratchRegister, pc);
        /*
         * Add an STR instruction to write the PC value now stored in the scratch register. We need
         * to consider four different cases here:
         *
         *                     Where to write PC:    Final value of Rn:    STR (immediate) variant:
         * STMIA Rn,  regs:    R[n]'     + 4*(#r-1)  keep                  offset       (P=1, W=0)
         * STMIA Rn!, regs:    R[n]'                 add 4                 post-indexed (P=0, W=0)
         * STMIB Rn,  regs:    R[n]' + 4 + 4*(#r-1)  keep                  offset       (P=1, W=0)
         * STMIB Rn!  regs:    R[n]' + 4             add 4                 pre-indexed  (P=1, W=1)
         *
         * R[n]' denotes the value in Rn after performing the STM with (#r-1) registers.
         */
        ARMStrImmediateInstruction strPCInstruction = { .value = STR_IMMEDIATE_BASE_VALUE };
        strPCInstruction.fields.add = 1;
        strPCInstruction.fields.baseRegister = baseRegister;
        strPCInstruction.fields.conditionCode = conditionCode;
        strPCInstruction.fields.immediate = (before || writeback ? 4 : 0)
                                          + (writeback ? 0 : 4 * countBitsSet(remainingRegisters));
        strPCInstruction.fields.index = before || !writeback;
        strPCInstruction.fields.sourceRegister = scratchRegister;
        strPCInstruction.fields.writeBackIfNotIndex = before && writeback;
        armWriteToCodeCache(tc, block, strPCInstruction.value);
        DEBUG(TRANSLATION, "armStmPCInstruction: split off PC in STR instruction: %#.8x" EOL,
              strPCInstruction.value);
      }
      else
      {
        /*
         * For STMDA/STMDB (decrement after or before):
         * - the lowest-numbered register is written at the address in
         *   > Rn - 4*countBitsSet(registers) + 4 (DA)
         *   > Rn - 4*countBitsSet(registers)     (DB)
         * - each next register is written at offset 4 of the last one;
         * - if writeback is set, Rn is decremented by 4*countBitsSet(registers).
         *
         * When we omit the PC from the list of registers, the starting address is increased by 4!
         * Hence, we have to modify the starting address upfront.
         *
         * Where to write PC relative to R[n] (original) and R[n]' (after writeback, given that the
         * starting address was modified to be correct):
         *
         *                     Rel. R[n]    Relative to R[n]'     Final value
         * STMDA Rn,  regs:    R[n]         R[n]'                 R[n]' = R[n]
         * STMDA Rn!, regs:    R[n]         R[n]'     + 4 * #r    R[n]' = R[n] - 4 * #r
         * STMDB Rn,  regs:    R[n] - 4     R[n]' - 4             R[n]' = R[n]
         * STMDB Rn!  regs:    R[n] - 4     R[n]' - 4 + 4 * #r    R[n]' = R[n] - 4 * #r
         *
         * If there are remaining registers, we need to:
         * - Subtract 4 from Rn;
         * - Perform STMDA/STMDB with remaining registers;
         * - For the writeback case, Rn will now be correct, otherwise add 4 to Rn.
         * Even though we could use the STR to do the subtraction upfront, to preserve the order of
         * memory accesses observed by the guest we put the STR last. When the PC is written on a
         * page different from the other registers, the guest would otherwise be able to observe
         * our alteration by causing and inspecting data aborts on those pages.
         *
         * First, we add 'SUB Rn, Rn, #4':
         */
        ARMSubImmediateInstruction adjustBaseRegisterInstruction = { .value = SUB_IMMEDIATE_BASE_VALUE };
        adjustBaseRegisterInstruction.fields.conditionCode = conditionCode;
        adjustBaseRegisterInstruction.fields.destinationRegister = baseRegister;
        adjustBaseRegisterInstruction.fields.immediate = 4;
        adjustBaseRegisterInstruction.fields.operandRegister = baseRegister;
        armWriteToCodeCache(tc, block, adjustBaseRegisterInstruction.value);
        DEBUG(TRANSLATION, "armStmPCInstruction: adjusting base address with SUB Rn, Rn, #4: %#.8x"
              EOL, adjustBaseRegisterInstruction.value);
        /*
         * Now perform STMDA/STMDB on remaining registers
         */
        armWriteToCodeCache(tc, block, instruction & ~STM_REGISTERS_PC_BIT);
        /*
         * Find another register, spill it and use it for the PC value
         */
        scratchRegister = getOtherRegister(baseRegister);
        DEBUG(TRANSLATION, "armStmPCInstruction: spilling and reusing r%u" EOL, scratchRegister);
        armBackupRegisterToSpill(tc, block, conditionCode, scratchRegister);
        armWritePCToRegister(tc, block, conditionCode, scratchRegister, pc);
        /*
         * Add an STR instruction to write the PC value now stored in the scratch register. We need
         * to consider four different cases here:
         *
         *                   Write PC at:      Immediate  Final Rn:         STR (imm.) variant:
         * STMDA Rn,  regs:  R[n]'         +4   4         R[n]'   +4(S)     pre-indexed  (P=1, W=1)
         * STMDA Rn!, regs:  R[n]'   +4*#r      4*(#r)    R[n]'             offset       (P=1, W=0)
         * STMDB Rn,  regs:  R[n]'-4       +4   4         R[n]'   +4(S)     post-indexed (P=0, W=0)
         * STMDB Rn!  regs:  R[n]'-4 +4*#r      4*(#r-1)  R[n]'             offset       (P=1, W=0)
         *
         * R[n]' denotes the value in Rn after performing the above STM with (#r-1) registers.
         */
        ARMStrImmediateInstruction strPCInstruction = { .value = STR_IMMEDIATE_BASE_VALUE };
        strPCInstruction.fields.add = TRUE;
        strPCInstruction.fields.baseRegister = baseRegister;
        strPCInstruction.fields.conditionCode = conditionCode;
        strPCInstruction.fields.immediate = (!before || !writeback ? 4 : 0)
                                          + (writeback ? 4 * countBitsSet(remainingRegisters) : 0);
        strPCInstruction.fields.index = !before || writeback;
        strPCInstruction.fields.sourceRegister = scratchRegister;
        strPCInstruction.fields.writeBackIfNotIndex = !before && !writeback;
        armWriteToCodeCache(tc, block, strPCInstruction.value);
        DEBUG(TRANSLATION, "armStmPCInstruction: split off PC in STR instruction: %#.8x" EOL,
              strPCInstruction.value);
      }
    }
    /*
     * Restore register; done.
     */
    armRestoreRegisterFromSpill(tc, block, conditionCode, scratchRegister);
  }
  else
  {
    armWriteToCodeCache(tc, block, instruction);
  }
}

/*
 * Translates STR{,B,H,D} in register & immediate forms
 */
void armStrPCInstruction(TranslationCache *tc, ARMTranslationInfo *block, u32int pc, u32int instruction)
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
    DEBUG(TRANSLATION, "armStrPCInstruction: translating %#.8x @ %#.8x with cond=%x, Rd=%x, "
          "Rt=%x" EOL, instruction, pc, conditionCode, sourceRegister, baseRegister);

    if (!(instruction & STR_STRB_FORM_BITS) && (instruction & STRD_BIT))
    {
      /*
       * Instruction is STRD immediate or register.
       * The immediate form uses 3 registers while the register form uses 4 of them! To arrive
       * here, we have Rn=PC and Rt!=PC, so we omit the base register from getOtherRegisterOfN.
       */
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
      /*
       * Instruction is STR{,B,H} immediate: only 2 registers used!
       */
      scratchRegister = getOtherRegisterOf2(sourceRegister, baseRegister);
    }
    else
    {
      ASSERT((instruction & STR_STRB_FORM_BITS) == STR_STRB_REGISTER_FORM_BITS ||
             (!(instruction & STR_STRB_FORM_BITS) && !(instruction & STRH_STRD_IMMEDIATE_FORM_BIT)),
             ERROR_BAD_ARGUMENTS);
      /*
       * Instruction is STR{,B,H} register: 3 registers used.
       */
      ASSERT(offsetRegister != GPR_PC, ERROR_UNPREDICTABLE_INSTRUCTION);
      scratchRegister = getOtherRegisterOf3(sourceRegister, baseRegister, offsetRegister);
    }

    armBackupRegisterToSpill(tc, block, conditionCode, scratchRegister);
    armWritePCToRegister(tc, block, conditionCode, scratchRegister, pc);

    if (replaceSource)
    {
      instruction = ARM_SET_REGISTER(instruction, STR_RT_INDEX, scratchRegister);
    }

    if (replaceBase)
    {
      instruction = ARM_SET_REGISTER(instruction, RN_INDEX, scratchRegister);
    }
  }

  armWriteToCodeCache(tc, block, instruction);

  if (replaceSource || replaceBase)
  {
    armRestoreRegisterFromSpill(tc, block, conditionCode, scratchRegister);
  }
}
