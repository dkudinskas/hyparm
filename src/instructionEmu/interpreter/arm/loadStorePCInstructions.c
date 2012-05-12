#include "common/bit.h"
#include "common/debug.h"

#include "cpuArch/constants.h"

#include "instructionEmu/interpreter/blockCopy.h"
#include "instructionEmu/interpreter/internals.h"

#include "instructionEmu/interpreter/arm/loadStorePCInstructions.h"


/*
 * Bit offsets in instruction words (LDR and STR have same offsets)
 */
enum
{
  LDR_RM_INDEX = 0,
  LDR_RN_INDEX = 16,
  LDR_RT_INDEX = 12,

  LDR_LDRB_REGISTER_FORM_BITS = 0b11 << 25,
  LDRH_LDRD_IMMEDIATE_FORM_BIT = 1 << 22,
  LDRD_BIT = 1 << 6,

  STM_RN_INDEX = 16,
  STM_REGISTERS_MASK = 0xFFFF,
  STM_REGISTERS_PC_BIT = 1 << GPR_PC,
  STM_WRITEBACK_BIT = 1 << 21
};


/*
 * Translates {LDR,STR}{,B,H,D} in register & immediate forms for which Rd!=PC
 */
u32int *armLdrStrPCInstruction(TranslationCache *tc, u32int *instructionAddr, u32int *currBlockCopyCacheAddr, u32int *blockCopyCacheStartAddress)
{
  u32int instruction = *instructionAddr;
  const u32int pc = (u32int)instructionAddr;
  const u32int destinationRegister = ARM_EXTRACT_REGISTER(instruction, LDR_RT_INDEX);
  const u32int baseRegister = ARM_EXTRACT_REGISTER(instruction, LDR_RN_INDEX);

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
      ASSERT(ARM_EXTRACT_REGISTER(instruction, LDR_RM_INDEX) != GPR_PC, "Rm=PC unpredictable");
      break;
    }
  }

  if (baseRegister == GPR_PC)
  {
    currBlockCopyCacheAddr = armWritePCToRegister(tc, currBlockCopyCacheAddr, ARM_EXTRACT_CONDITION_CODE(instruction), destinationRegister, pc);
    instruction = ARM_SET_REGISTER(instruction, LDR_RN_INDEX, destinationRegister);
  }

  currBlockCopyCacheAddr = updateCodeCachePointer(tc, currBlockCopyCacheAddr);
  *currBlockCopyCacheAddr = instruction;
  return ++currBlockCopyCacheAddr;
}

u32int* armStmPCInstruction(TranslationCache *tc, u32int *instructionAddr, u32int *currBlockCopyCacheAddr, u32int *blockCopyCacheStartAddress)
{
  u32int instruction = *instructionAddr;

  ASSERT(ARM_EXTRACT_REGISTER(instruction, STM_RN_INDEX) != GPR_PC, "Rn=PC unpredictable");

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
