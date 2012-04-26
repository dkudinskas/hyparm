#include "instructionEmu/interpreter/blockCopy.h"
#include "instructionEmu/interpreter/internals.h"

#include "instructionEmu/interpreter/arm/dataProcessPCInstructions.h"


u32int *armAsrPCInstruction(TranslationCache *tc, u32int *instructionAddr, u32int *currBlockCopyCacheAddr, u32int *blockCopyCacheStartAddress)
{
  if (ARM_EXTRACT_CONDITION_CODE(*instructionAddr) != CC_AL)
  {
    DIE_NOW(NULL, "asrPCInstruction conditional");
  }

  u32int instr2Copy = *instructionAddr;
  if ((*instructionAddr & 0xF) == 0xF)
  {
    u32int destReg = (*instructionAddr >> 12) & 0xF;
    //inputRegister = PC
    //step 1 Copy PC (=instructionAddr2) to desReg
    currBlockCopyCacheAddr = savePCInReg(tc, instructionAddr, currBlockCopyCacheAddr, destReg);
    //Step 2 modify ldrInstruction
    //Clear PC source Register
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
  bool conditionAlways = (instruction >> 28 & 0xF) == 0xE;
  //bits 16-19 are zero so Rm or Rn is PC

  if (conditionAlways)
  {
    //Ready to do shift
    //save PC
    if ((instruction & 0xF) == 0xF)
    {
      currBlockCopyCacheAddr = savePCInReg(tc, instructionAddr, currBlockCopyCacheAddr, destReg);

      //Step 2 modify ldrInstruction
      //Clear PC source Register
      instr2Copy = (instruction & ~0xF) | destReg;
    }
    currBlockCopyCacheAddr = updateCodeCachePointer(tc, currBlockCopyCacheAddr);
    *(currBlockCopyCacheAddr++) = instr2Copy;

    return currBlockCopyCacheAddr;
  }
  else
  {
    /*lsrPC Funct conditional*/
    DIE_NOW(NULL, "lsrPCFunct conditional is not yet implemented");
  }
}

u32int *armMovPCInstruction(TranslationCache *tc, u32int *instructionAddr, u32int *currBlockCopyCacheAddr, u32int *blockCopyCacheStartAddress)
{
  //Destination is surely not PC
  u32int instruction = *instructionAddr;
  u32int destReg = (instruction >> 12) & 0xF;
  u32int instr2Copy = instruction;
  bool conditionAlways = (instruction >> 28 & 0xF) == 0xE;

  if (conditionAlways)
  {
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
        currBlockCopyCacheAddr = savePCInReg(tc, instructionAddr, currBlockCopyCacheAddr, destReg);
        //Step 2 modify ldrInstruction
        //Clear PC source Register
        instr2Copy = (instruction & ~0xF) | destReg; //set last 4 bits so correct register is used
      }
    }

    currBlockCopyCacheAddr = updateCodeCachePointer(tc, currBlockCopyCacheAddr);
    *(currBlockCopyCacheAddr++) = instr2Copy;

    return currBlockCopyCacheAddr;
  }
  else
  {
    //condition != always
    if (((instruction >> 25) & 0b1) == 0b0)
    {
      //bit 25 != 1 -> there can be registers, PC can possibly be read
      if ((instruction & 0xF) != 0xF)
      {
        DIE_NOW(NULL, "mov PCFunct: movPCFunct can only be called if last 4 bits are 1111\n");
      }
      else
      {
        //Make instruction safe and return
        /* conditional instruction thus sometimes not executed */
        /*Instruction has to be changed to a PC safe instructionstream withouth using destReg. */
        u32int srcReg = instruction & 0xF;
        u32int scratchReg = getOtherRegisterOf2(srcReg, destReg);
        /* place 'Backup scratchReg' instruction */
        currBlockCopyCacheAddr = backupRegister(tc, scratchReg, currBlockCopyCacheAddr, blockCopyCacheStartAddress);
        currBlockCopyCacheAddr = savePCInReg(tc, instructionAddr, currBlockCopyCacheAddr, scratchReg);

        instr2Copy = (instr2Copy & ~0xF) | scratchReg;

        currBlockCopyCacheAddr = updateCodeCachePointer(tc, currBlockCopyCacheAddr);
        *(currBlockCopyCacheAddr++) = instr2Copy;

        /* place 'restore scratchReg' instruction */
        currBlockCopyCacheAddr = restoreRegister(tc, scratchReg, currBlockCopyCacheAddr, blockCopyCacheStartAddress);
        /* Make sure scanner sees that we need a word to store the register*/
        currBlockCopyCacheAddr = (u32int*) (((u32int) currBlockCopyCacheAddr) | 0b1);

        return currBlockCopyCacheAddr;
      }
    }

    //if function hasn't returned at this point -> instruction is safe
    currBlockCopyCacheAddr = updateCodeCachePointer(tc, currBlockCopyCacheAddr);
    *(currBlockCopyCacheAddr++) = instruction;

    return currBlockCopyCacheAddr;
  }
}

u32int *armMvnPCInstruction(TranslationCache *tc, u32int *instructionAddr, u32int *currBlockCopyCacheAddr, u32int *blockCopyCacheStartAddress)
{
  u32int instruction = *instructionAddr;
  u32int destReg = (instruction >> 12) & 0xF;
  u32int instr2Copy = instruction;
  bool immediate = (instruction >> 25 & 0b1) == 0b1;
  bool replaceReg1 = FALSE;
  bool conditionAlways = (instruction >> 28 & 0xF) == 0xE;

  if (immediate)
  {
    // FIXME
    DIE_NOW(NULL, "Peter: this is safe. Niels: i dont think so...");
    //Always safe do nothing replaceReg1 is already false
  }
  else
  {
    if (conditionAlways)
    {
      bool registerShifted = ((instruction >> 4 & 0b1) == 0b1) && ((instruction >> 4 & 0b1) == 0b0);
      //Here we know it is register or register-shifted register
      if (registerShifted)
      {
        DIE_NOW(NULL, "MVNPC (register-shifted register) -> UNPREDICTABLE");
      }
      else
      {
        //eor (register)
        if ((instruction & 0xF) == 0xF)
        {
          replaceReg1 = TRUE;
        }
      }
      if (replaceReg1)
      {
        //step 1 Copy PC (=instructionAddr2) to desReg
        currBlockCopyCacheAddr = savePCInReg(tc, instructionAddr, currBlockCopyCacheAddr, destReg);
        if (replaceReg1)
        {
          //Step 2 modify eorInstruction
          //Clear PC source Register
          instr2Copy = (instruction & ~0xF) | destReg;
        }
      }
    }
    else
    {
      /* mvn with condition code != ALWAYS*/
      DIE_NOW(NULL, "conditional mvn PCFunct not yet implemented");
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

