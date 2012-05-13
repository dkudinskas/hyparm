#include "instructionEmu/interpreter/blockCopy.h"
#include "instructionEmu/interpreter/internals.h"

#include "instructionEmu/interpreter/arm/pcHandlers.h"


/*
 * Bit offsets in instruction words (LDR and STR have same offsets)
 */
enum
{
  DATA_PROC_IMMEDIATE_FORM_BIT = 1 << 25,

  LDR_RT_INDEX = 12,
  LDR_LDRB_REGISTER_FORM_BITS = 0b11 << 25,
  LDRH_LDRD_IMMEDIATE_FORM_BIT = 1 << 22,
  LDRD_BIT = 1 << 6,

  STM_REGISTERS_MASK = 0xFFFF,
  STM_REGISTERS_PC_BIT = 1 << GPR_PC,
  STM_WRITEBACK_BIT = 1 << 21,

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
u32int *armDPImmRegRSR(TranslationCache *tc, u32int *instructionAddr, u32int *currBlockCopyCacheAddr, u32int *blockCopyCacheStartAddress)
{
  u32int instruction = *instructionAddr;
  const u32int pc = (u32int)instructionAddr;
  const u32int conditionCode = ARM_EXTRACT_CONDITION_CODE(instruction);
  const bool immediateForm = (instruction >> 25 & DATA_PROC_IMMEDIATE_FORM_BIT);
  const u32int destinationRegister = ARM_EXTRACT_REGISTER(instruction, RD_INDEX);
  const u32int operandNRegister = ARM_EXTRACT_REGISTER(instruction, RN_INDEX);
  const u32int operandMRegister = ARM_EXTRACT_REGISTER(instruction, RM_INDEX);

  ASSERT(destinationRegister != GPR_PC, "Rd=PC must trap");

  u32int pcRegister = destinationRegister;
  bool replaceN = operandNRegister == GPR_PC;
  bool replaceM = !immediateForm && operandMRegister == GPR_PC;
  bool restoreFromSpill = FALSE;

  if (replaceN || replaceM)
  {
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
      currBlockCopyCacheAddr = armBackupRegisterToSpill(tc, currBlockCopyCacheAddr, conditionCode, scratchRegister);
      currBlockCopyCacheAddr = updateCodeCachePointer(tc, currBlockCopyCacheAddr);
      pcRegister = scratchRegister;
      restoreFromSpill = TRUE;
    }

    currBlockCopyCacheAddr = armWritePCToRegister(tc, currBlockCopyCacheAddr, conditionCode, pcRegister, pc);

    if (replaceN)
    {
      instruction = ARM_SET_REGISTER(instruction, RN_INDEX, pcRegister);
    }

    if (replaceM)
    {
      instruction = ARM_SET_REGISTER(instruction, RM_INDEX, pcRegister);
    }
  }

  currBlockCopyCacheAddr = updateCodeCachePointer(tc, currBlockCopyCacheAddr);
  *(currBlockCopyCacheAddr++) = instruction;

  if (restoreFromSpill)
  {
    currBlockCopyCacheAddr = armRestoreRegisterFromSpill(tc, currBlockCopyCacheAddr, conditionCode, pcRegister);
  }

  return currBlockCopyCacheAddr;
}

/*
 * Translates {CMN,CMP,TEQ,TST}.
 *
 * All these instructions are data processing instructions which update the condition flags in the
 * PSR and discard their result. Because there is no destination register, we need to spill some
 * register for the PC...
 */
u32int *armDPImmRegRSRNoDest(TranslationCache *tc, u32int *instructionAddr, u32int *currBlockCopyCacheAddr, u32int *blockCopyCacheStartAddress)
{
  u32int instruction = *instructionAddr;
  const u32int pc = (u32int)instructionAddr;
  const u32int conditionCode = ARM_EXTRACT_CONDITION_CODE(instruction);
  const bool immediateForm = (instruction >> 25 & DATA_PROC_IMMEDIATE_FORM_BIT);
  const u32int operandNRegister = ARM_EXTRACT_REGISTER(instruction, RN_INDEX);
  const u32int operandMRegister = ARM_EXTRACT_REGISTER(instruction, RM_INDEX);

  u32int pcRegister;
  bool replaceN = operandNRegister == GPR_PC;
  bool replaceM = !immediateForm && operandMRegister == GPR_PC;

  if (replaceN || replaceM)
  {
    pcRegister = getOtherRegisterOf2(operandNRegister, operandMRegister);
    currBlockCopyCacheAddr = armBackupRegisterToSpill(tc, currBlockCopyCacheAddr, conditionCode, pcRegister);
    currBlockCopyCacheAddr = armWritePCToRegister(tc, currBlockCopyCacheAddr, conditionCode, pcRegister, pc);

    if (replaceN)
    {
      instruction = ARM_SET_REGISTER(instruction, RN_INDEX, pcRegister);
    }

    if (replaceM)
    {
      instruction = ARM_SET_REGISTER(instruction, RM_INDEX, pcRegister);
    }
  }

  currBlockCopyCacheAddr = updateCodeCachePointer(tc, currBlockCopyCacheAddr);
  *(currBlockCopyCacheAddr++) = instruction;

  if (replaceN || replaceM)
  {
    currBlockCopyCacheAddr = armRestoreRegisterFromSpill(tc, currBlockCopyCacheAddr, conditionCode, pcRegister);
  }

  return currBlockCopyCacheAddr;
}

/*
 * Translates {LDR,STR}{,B,H,D} in register & immediate forms for which Rd!=PC
 */
u32int *armLdrStrPCInstruction(TranslationCache *tc, u32int *instructionAddr, u32int *currBlockCopyCacheAddr, u32int *blockCopyCacheStartAddress)
{
  u32int instruction = *instructionAddr;
  const u32int pc = (u32int)instructionAddr;
  const u32int destinationRegister = ARM_EXTRACT_REGISTER(instruction, LDR_RT_INDEX);
  const u32int baseRegister = ARM_EXTRACT_REGISTER(instruction, RN_INDEX);

  ASSERT(destinationRegister != GPR_PC, "Rd=PC: trap (LDR/STR) or unpredictable (B/H/D)");

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
      ASSERT(ARM_EXTRACT_REGISTER(instruction, RM_INDEX) != GPR_PC, "Rm=PC unpredictable");
      break;
    }
  }

  if (baseRegister == GPR_PC)
  {
    currBlockCopyCacheAddr = armWritePCToRegister(tc, currBlockCopyCacheAddr, ARM_EXTRACT_CONDITION_CODE(instruction), destinationRegister, pc);
    instruction = ARM_SET_REGISTER(instruction, RN_INDEX, destinationRegister);
  }

  currBlockCopyCacheAddr = updateCodeCachePointer(tc, currBlockCopyCacheAddr);
  *currBlockCopyCacheAddr = instruction;
  return ++currBlockCopyCacheAddr;
}

/*
 * Translates MOV for which Rd!=PC.
 */
u32int *armMovPCInstruction(TranslationCache *tc, u32int *instructionAddr, u32int *currBlockCopyCacheAddr, u32int *blockCopyCacheStartAddress)
{
  u32int instruction = *instructionAddr;
  const u32int pc = (u32int)instructionAddr;
  const u32int destinationRegister = ARM_EXTRACT_REGISTER(instruction, RD_INDEX);
  const u32int sourceRegister = ARM_EXTRACT_REGISTER(instruction, RM_INDEX);

  ASSERT(destinationRegister != GPR_PC, "Rd=PC must trap");

  /*
   * A MOV Rd,PC can be translated to only MOVT & MOVW. MOVS requires an update of the condition
   * flags and since MOVT does not support setting flags, an extra MOVS Rd,Rd is required.
   */
  if (sourceRegister == GPR_PC)
  {
    currBlockCopyCacheAddr = armWritePCToRegister(tc, currBlockCopyCacheAddr, ARM_EXTRACT_CONDITION_CODE(*instructionAddr), destinationRegister, pc);
    if (!(instruction & SETFLAGS_BIT))
    {
      return ++currBlockCopyCacheAddr;
    }
    instruction = ARM_SET_REGISTER(instruction, RM_INDEX, destinationRegister);
  }

  currBlockCopyCacheAddr = updateCodeCachePointer(tc, currBlockCopyCacheAddr);
  *(currBlockCopyCacheAddr++) = instruction;
  return currBlockCopyCacheAddr;
}

/*
 * Translates {ASR,LSL,LSR,MVN,ROR,RRX} in immediate forms for which Rd!=PC.
 *
 * These instructions are all of the form
 * cond | 0001 | 101S | (0000) | Rd | imm5 | t2 | 0 | Rm
 * The shift type is determined by 2 bits (t2). For RRX, imm5 is always 00000.
 */
u32int *armShiftPCInstruction(TranslationCache *tc, u32int *instructionAddr, u32int *currBlockCopyCacheAddr, u32int *blockCopyCacheStartAddress)
{
  u32int instruction = *instructionAddr;
  const u32int pc = (u32int)instructionAddr;
  const u32int destinationRegister = ARM_EXTRACT_REGISTER(instruction, RD_INDEX);
  const u32int operandRegister = ARM_EXTRACT_REGISTER(instruction, RM_INDEX);

  if (operandRegister == GPR_PC)
  {
    currBlockCopyCacheAddr = armWritePCToRegister(tc, currBlockCopyCacheAddr, ARM_EXTRACT_CONDITION_CODE(*instructionAddr), destinationRegister, pc);
    instruction = ARM_SET_REGISTER(instruction, RM_INDEX, destinationRegister);
  }

  currBlockCopyCacheAddr = updateCodeCachePointer(tc, currBlockCopyCacheAddr);
  *(currBlockCopyCacheAddr++) = instruction;
  return currBlockCopyCacheAddr;
}

u32int* armStmPCInstruction(TranslationCache *tc, u32int *instructionAddr, u32int *currBlockCopyCacheAddr, u32int *blockCopyCacheStartAddress)
{
  u32int instruction = *instructionAddr;

  ASSERT(ARM_EXTRACT_REGISTER(instruction, RN_INDEX) != GPR_PC, "Rn=PC unpredictable");

  if ((instruction & STM_REGISTERS_PC_BIT))
  {
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

  currBlockCopyCacheAddr = updateCodeCachePointer(tc, currBlockCopyCacheAddr);
  *currBlockCopyCacheAddr = instruction;
  return ++currBlockCopyCacheAddr;
}
