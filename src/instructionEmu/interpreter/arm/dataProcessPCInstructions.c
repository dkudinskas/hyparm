#include "instructionEmu/interpreter/blockCopy.h"
#include "instructionEmu/interpreter/internals.h"

#include "instructionEmu/interpreter/arm/dataProcessPCInstructions.h"


enum
{
  MOV_RD_INDEX = 12,
  MOV_RM_INDEX = 0
};


u32int *armAsrPCInstruction(TranslationCache *tc, u32int *instructionAddr, u32int *currBlockCopyCacheAddr, u32int *blockCopyCacheStartAddress)
{
  u32int conditionCode = ARM_EXTRACT_CONDITION_CODE(*instructionAddr);

  u32int instr2Copy = *instructionAddr;
  if ((*instructionAddr & 0xF) == GPR_PC)
  {
    u32int destReg = (*instructionAddr >> 12) & 0xF;
    /*
     * Put PC in destination register and patch MVN Rd,PC to MVN Rd,Rd
     */
    currBlockCopyCacheAddr = armWritePCToRegister(tc, currBlockCopyCacheAddr, conditionCode, destReg, (u32int)instructionAddr);
    instr2Copy = (instr2Copy & ~0xF) | destReg;
  }

  currBlockCopyCacheAddr = updateCodeCachePointer(tc, currBlockCopyCacheAddr);
  *(currBlockCopyCacheAddr++) = instr2Copy;
  return currBlockCopyCacheAddr;
}

u32int *armLslrPCInstruction(TranslationCache *tc, u32int *instructionAddr, u32int *currBlockCopyCacheAddr, u32int *blockCopyCacheStartAddress)
{
  // LSL is the same as LSR only direction has changed -> only bit 5 differs
  u32int instruction = *instructionAddr;
  u32int destReg = (instruction >> 12) & 0xF;
  u32int instr2Copy = instruction;
  u32int conditionCode = ARM_EXTRACT_CONDITION_CODE(*instructionAddr);
  //bits 16-19 are zero so Rm or Rn is PC

  //Ready to do shift
  //save PC
  if ((instruction & 0xF) == 0xF)
  {
    currBlockCopyCacheAddr = armWritePCToRegister(tc, currBlockCopyCacheAddr, conditionCode, destReg, (u32int)instructionAddr);

    //Step 2 modify ldrInstruction
    //Clear PC source Register
    instr2Copy = (instruction & ~0xF) | destReg;
  }
  currBlockCopyCacheAddr = updateCodeCachePointer(tc, currBlockCopyCacheAddr);
  *(currBlockCopyCacheAddr++) = instr2Copy;

  return currBlockCopyCacheAddr;
}

u32int *armMovPCInstruction(TranslationCache *tc, u32int *instructionAddr, u32int *currBlockCopyCacheAddr, u32int *blockCopyCacheStartAddress)
{
  u32int instruction = *instructionAddr;
  const u32int pc = (u32int)instructionAddr;
  const u32int destinationRegister = ARM_EXTRACT_REGISTER(instruction, MOV_RD_INDEX);
  const u32int sourceRegister = ARM_EXTRACT_REGISTER(instruction, MOV_RM_INDEX);

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

u32int *armRorPCInstruction(TranslationCache *tc, u32int *instructionAddr, u32int *currBlockCopyCacheAddr, u32int *blockCopyCacheStartAddress)
{
  DIE_NOW(NULL, "ror PCFunct unfinished\n");
}

u32int *armRrxPCInstruction(TranslationCache *tc, u32int *instructionAddr, u32int *currBlockCopyCacheAddr, u32int *blockCopyCacheStartAddress)
{
  DIE_NOW(NULL, "rrx PCFunct unfinished\n");
}

