#include "instructionEmu/interpreter/blockCopy.h"
#include "instructionEmu/interpreter/internals.h"

#include "instructionEmu/interpreter/arm/pcHandlers.h"


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

  SETFLAGS_BIT = 1 << 20
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
    ASSERT(destinationRegister != GPR_PC, "Rd=PC must trap");

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
      block->code = updateCodeCachePointer(tc, block->code);
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

  block->code = updateCodeCachePointer(tc, block->code);
  *block->code = instruction;
  block->code++;

  if (restoreFromSpill)
  {
    armRestoreRegisterFromSpill(tc, block, conditionCode, pcRegister);
  }

  block->metaEntry.pcRemapBitmap |= PC_REMAP_INCREMENT << block->pcRemapBitmapShift;
  block->pcRemapBitmapShift += PC_REMAP_BIT_COUNT;
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

  block->code = updateCodeCachePointer(tc, block->code);
  *block->code = instruction;
  block->code++;

  if (replaceN || replaceM)
  {
    armRestoreRegisterFromSpill(tc, block, conditionCode, pcRegister);
  }

  block->metaEntry.pcRemapBitmap |= PC_REMAP_INCREMENT << block->pcRemapBitmapShift;
  block->pcRemapBitmapShift += PC_REMAP_BIT_COUNT;
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
          ASSERT(destinationRegister != GPR_LR, "Rt2=PC unpredictable");
        }
        if ((instruction & LDRH_LDRD_IMMEDIATE_FORM_BIT))
        {
          break;
        }
      }
      /* no break */
      case LDR_LDRB_REGISTER_FORM_BITS:
      {
        ASSERT(offsetRegister != GPR_PC, "Rm=PC unpredictable");
        spill = offsetRegister == destinationRegister;
        break;
      }
    }

    if (spill)
    {
      const u32int scratchRegister = getOtherRegisterOf2(destinationRegister, offsetRegister);
      armBackupRegisterToSpill(tc, block, conditionCode, scratchRegister);
      block->code = updateCodeCachePointer(tc, block->code);
      pcRegister = scratchRegister;
    }

    armWritePCToRegister(tc, block, conditionCode, pcRegister, pc);
    instruction = ARM_SET_REGISTER(instruction, RN_INDEX, pcRegister);
  }

  block->code = updateCodeCachePointer(tc, block->code);
  *block->code = instruction;
  block->code++;

  if (spill)
  {
    armRestoreRegisterFromSpill(tc, block, conditionCode, pcRegister);
  }

  block->metaEntry.pcRemapBitmap |= PC_REMAP_INCREMENT << block->pcRemapBitmapShift;
  block->pcRemapBitmapShift += PC_REMAP_BIT_COUNT;
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
    ASSERT(destinationRegister != GPR_PC, "Rd=PC must trap");

    DEBUG(TRANSLATION, "armMovPCInstruction: translating %#.8x @ %#.8x with cond=%x, S=%x, Rd=%x, "
          "Rm=%x" EOL, instruction, pc, conditionCode, setFlags, destinationRegister,
          sourceRegister);

    armWritePCToRegister(tc, block, conditionCode, destinationRegister, pc);
    if (!(instruction & SETFLAGS_BIT))
    {
      block->code++;
      block->metaEntry.pcRemapBitmap |= PC_REMAP_INCREMENT << (block->pcRemapBitmapShift - PC_REMAP_BIT_COUNT);
      return;
    }
    instruction = ARM_SET_REGISTER(instruction, RM_INDEX, destinationRegister);
  }

  block->code = updateCodeCachePointer(tc, block->code);
  *block->code = instruction;
  block->code++;
  block->metaEntry.pcRemapBitmap |= PC_REMAP_INCREMENT << block->pcRemapBitmapShift;
  block->pcRemapBitmapShift += PC_REMAP_BIT_COUNT;
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
    ASSERT(destinationRegister != GPR_PC, "Rd=PC must trap");

    DEBUG(TRANSLATION, "armShiftPCInstruction: translating %#.8x @ %#.8x with cond=%x, Rd=%x, "
          "Rm=%x" EOL, instruction, pc, conditionCode, destinationRegister, operandRegister);

    armWritePCToRegister(tc, block, conditionCode, destinationRegister, pc);
    instruction = ARM_SET_REGISTER(instruction, RM_INDEX, destinationRegister);
  }

  block->code = updateCodeCachePointer(tc, block->code);
  *block->code = instruction;
  block->code++;
  block->metaEntry.pcRemapBitmap |= PC_REMAP_INCREMENT << block->pcRemapBitmapShift;
  block->pcRemapBitmapShift += PC_REMAP_BIT_COUNT;
}

void armStmPCInstruction(TranslationCache *tc, ARMTranslationInfo *block, u32int pc, u32int instruction)
{
  if ((instruction & STM_REGISTERS_PC_BIT))
  {
    ASSERT(ARM_EXTRACT_REGISTER(instruction, RN_INDEX) != GPR_PC, "Rn=PC unpredictable");

    /*
     * How to perform similar STM on all registers except PC, if any:
     *
     * instruction &= ~STM_REGISTERS_PC_BIT;
     * if ((instruction & STM_REGISTERS_MASK) != 0)
     * {
     *   currBlockCopyCacheAddr = updateCodeCachePointer(tc, currBlockCopyCacheAddr);
     *   *currBlockCopyCacheAddr = instruction;
     * }
     *
     * We must also store PC. How all of this works together depends on STM variant (IA/DA/DB/IB)!
     * All translated instructions inherit condition code from original.
     *
     * regcnt := countBitsSet(*instructionAddr & STM_REGISTERS_MASK)
     *
     * STMIA:
     * perform STMIA Rn, registers without PC
     * backup Rtemp, set to PC
     * if W=0: STR Rtemp, [Rn, #(4*(regcnt-1))] (offset)
     * if W=1: STR Rtemp, [Rn], #4 (post-indexed)
     * restore Rtemp
     *
     * STMDA (double check this):
     * perform SUB Rn, Rn, #4
     * perform STMDA Rn, registers without PC
     * backup Rtemp, set to PC
     * if W=0: STR Rtemp, [Rn, #4]! (pre-indexed)
     * if W=1: STR Rtemp, [Rn, #(4*regcnt)] (offset)
     * restore Rtemp
     * or ALTERNATIVE without SUB: replace STMDA by STMDB this will start at one word lower anyway...
     *                             be careful with writeback, offsets must be correct!!!
     *
     * STMDB is similar to STMDA, but it starts writing at address= Rn-4*regcnt
     *                            instead of Rn-4*regcnt+4 ==> 1 word LOWER in memory
     * perform SUB Rn, Rn, #4 (required in this case)
     * perform STMDB Rn, registers without PC
     * backup Rtemp, set to PC
     * if W=0: ...
     * if W=1: ...
     * restore Rtemp
     *
     * STMIB similar to STMIA but starts at Rn+4
     * perform STMIB Rn, registers without PC
     * backup Rtemp, set to PC
     * if W=0: STR offset
     * if W=1: STR PRE-indexed
     * restore Rtemp
     */
    DIE_NOW(NULL, "STM.. Rn, ..-PC not implemented");
  }

  block->code = updateCodeCachePointer(tc, block->code);
  *block->code = instruction;
  block->code++;
  block->metaEntry.pcRemapBitmap |= PC_REMAP_INCREMENT << block->pcRemapBitmapShift;
  block->pcRemapBitmapShift += PC_REMAP_BIT_COUNT;
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
    block->code = updateCodeCachePointer(tc, block->code);
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

  block->code = updateCodeCachePointer(tc, block->code);
  *block->code = instruction;
  block->code++;

  if (replaceSource || replaceBase)
  {
    armRestoreRegisterFromSpill(tc, block, conditionCode, scratchRegister);
  }

  block->metaEntry.pcRemapBitmap |= PC_REMAP_INCREMENT << block->pcRemapBitmapShift;
  block->pcRemapBitmapShift += PC_REMAP_BIT_COUNT;
}
