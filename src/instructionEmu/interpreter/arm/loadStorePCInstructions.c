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
  LDRD_BIT = 1 << 6
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

u32int *armPopLdmPCInstruction(TranslationCache *tc, u32int *instructionAddr, u32int *currBlockCopyCacheAddr, u32int *blockCopyCacheStartAddress)
{
  DIE_NOW(NULL, "popLdm PCFunct unfinished\n");
}

u32int* armStmPCInstruction(TranslationCache *tc, u32int *instructionAddr, u32int *currBlockCopyCacheAddr, u32int *blockCopyCacheStartAddress)
{
  u32int instruction = *instructionAddr;
  if ((instruction & 0xF0000) == 0xF0000)
  {
    // According to ARM ARM: source register = PC ->  UNPREDICTABLE
    DIE_NOW(NULL, "stm PC had PC as Rn -> UNPREDICTABLE?!");
  }
  else
  {
    // Niels: FIXME
    DIE_NOW(NULL, "BUG pC CAN BE USED");
    //Stores multiple registers to consecutive memory locations
    //PC is not used -> instruction is save to execute just copy it to blockCopyCache
    currBlockCopyCacheAddr = updateCodeCachePointer(tc, currBlockCopyCacheAddr);
    *(currBlockCopyCacheAddr++) = instruction;

    return currBlockCopyCacheAddr;
  }
}
