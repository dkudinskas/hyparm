#include "instructionEmu/interpreter/blockCopy.h"
#include "instructionEmu/interpreter/internals.h"

#include "instructionEmu/interpreter/arm/dataProcessPCInstructions.h"


enum
{
  RD_INDEX = 12,
  RM_INDEX = 0
};


/*
 * Translates {ASR,LSL,LSR,ROR,RRX} in immediate forms for which Rd!=PC
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

  ASSERT(destinationRegister != GPR_PC, "MOV PC must trap");

  if (sourceRegister == GPR_PC)
  {
    currBlockCopyCacheAddr = armWritePCToRegister(tc, currBlockCopyCacheAddr, ARM_EXTRACT_CONDITION_CODE(*instructionAddr), destinationRegister, pc);
  }
  else
  {
    currBlockCopyCacheAddr = updateCodeCachePointer(tc, currBlockCopyCacheAddr);
    *(currBlockCopyCacheAddr++) = instruction;
  }
  return currBlockCopyCacheAddr;
}

u32int *armMvnPCInstruction(TranslationCache *tc, u32int *instructionAddr, u32int *currBlockCopyCacheAddr, u32int *blockCopyCacheStartAddress)
{
  u32int instruction = *instructionAddr;
  u32int destReg = (instruction >> 12) & 0xF;
  u32int instr2Copy = instruction;
  bool immediate = (instruction >> 25 & 0b1) == 0b1;
  bool replaceReg1 = FALSE;
  u32int conditionCode = ARM_EXTRACT_CONDITION_CODE(*instructionAddr);

  if (immediate)
  {
    // FIXME
    DIE_NOW(NULL, "Peter: this is safe. Niels: i dont think so...");
    //Always safe do nothing replaceReg1 is already false
  }
  else
  {
    if ((instruction & 0xF) == 0xF)
    {
      replaceReg1 = TRUE;
    }

    if (replaceReg1)
    {
      //step 1 Copy PC (=instructionAddr2) to desReg
      currBlockCopyCacheAddr = armWritePCToRegister(tc, currBlockCopyCacheAddr, conditionCode, destReg, (u32int)instructionAddr);
      if (replaceReg1)
      {
        //Step 2 modify eorInstruction
        //Clear PC source Register
        instr2Copy = (instruction & ~0xF) | destReg;
      }
    }
  }

  currBlockCopyCacheAddr = updateCodeCachePointer(tc, currBlockCopyCacheAddr);
  *(currBlockCopyCacheAddr++) = instr2Copy;

  return currBlockCopyCacheAddr;
}
