#include "instructionEmu/interpreter/blockCopy.h"
#include "instructionEmu/interpreter/internals.h"

#include "instructionEmu/interpreter/arm/dataProcessPCInstructions.h"


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
  //Destination is surely not PC
  u32int instruction = *instructionAddr;
  u32int destReg = (instruction >> 12) & 0xF;
  u32int instr2Copy = instruction;
  u32int conditionCode = ARM_EXTRACT_CONDITION_CODE(*instructionAddr);

  if (((instruction >> 25) & 0b1) != 1)
  {
    //bit 25 != 1 -> there can be registers, PC can possibly be read
    if ((instruction & 0xF) != 0xF)
    {
      DIE_NOW(NULL, "mov PCFunct: movPCFunct can only be called if last 4 bits are 1111\n");
    }
    else
    {
      //step 1 Copy PC (=instructionAddr2) to desReg
      currBlockCopyCacheAddr = armWritePCToRegister(tc, currBlockCopyCacheAddr, conditionCode, destReg, (u32int)instructionAddr);
      //Step 2 modify ldrInstruction
      //Clear PC source Register
      instr2Copy = (instruction & ~0xF) | destReg; //set last 4 bits so correct register is used
    }
  }

  currBlockCopyCacheAddr = updateCodeCachePointer(tc, currBlockCopyCacheAddr);
  *(currBlockCopyCacheAddr++) = instr2Copy;

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

