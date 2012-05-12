#include "instructionEmu/interpreter/blockCopy.h"
#include "instructionEmu/interpreter/internals.h"

#include "instructionEmu/interpreter/arm/dataProcessPCInstructions.h"


enum
{
  RD_INDEX = 12,
  RM_INDEX = 0,
  SETFLAGS_BIT = 1 << 20
};


/*
 * Translates {ASR,LSL,LSR,MVN,ROR,RRX} in immediate forms for which Rd!=PC
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
