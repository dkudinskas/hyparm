#include "instructionEmu/interpreter/blockCopy.h"
#include "instructionEmu/interpreter/internals.h"

#include "instructionEmu/interpreter/arm/dataProcessPCInstructions.h"


u32int *armAdcPCInstruction(GCONTXT *context, u32int *instructionAddr, u32int *currBlockCopyCacheAddr, u32int *blockCopyCacheStartAddress)
{
  return standardImmRegRSR(context, instructionAddr, currBlockCopyCacheAddr, blockCopyCacheStartAddress);
}

u32int *armAddPCInstruction(GCONTXT *context, u32int *instructionAddr, u32int *currBlockCopyCacheAddr, u32int *blockCopyCacheStartAddress)
{
  return standardImmRegRSR(context, instructionAddr, currBlockCopyCacheAddr, blockCopyCacheStartAddress);
}

u32int *armAndPCInstruction(GCONTXT *context, u32int *instructionAddr, u32int *currBlockCopyCacheAddr, u32int *blockCopyCacheStartAddress)
{
  return standardImmRegRSR(context, instructionAddr, currBlockCopyCacheAddr, blockCopyCacheStartAddress);
}

u32int *armAsrPCInstruction(GCONTXT *context, u32int *instructionAddr, u32int *currBlockCopyCacheAddr, u32int *blockCopyCacheStartAddress)
{
  u32int instruction = *instructionAddr;
  bool immediate = ((instruction >> 4 & 0x7) == 0x4);
  u32int destReg = (instruction >> 12) & 0xF;
  u32int instr2Copy = instruction;
  bool conditionAlways = (instruction >> 28 & 0xF) == 0xE;

  if (conditionAlways)
  {
    if (immediate)
    {
      if ((instruction & 0xF) == 0xF)
      { //inputRegister = PC
        //step 1 Copy PC (=instructionAddr2) to desReg
        currBlockCopyCacheAddr = savePCInReg(context, instructionAddr, currBlockCopyCacheAddr, destReg);

        //Step 2 modify ldrInstruction
        //Clear PC source Register
        instr2Copy = zeroBits(instruction, 0);
        instr2Copy = instr2Copy | (destReg);
      }
    }
    else
    {
      //ARM p 352
      DIE_NOW(context, "asrPCInstruction: ASR(register) cannot take PC as input!->UNPREDICTABLE");
    }

    currBlockCopyCacheAddr = checkAndClearBlockCopyCacheAddress(currBlockCopyCacheAddr, context->blockCache, (u32int*) context->blockCopyCache, (u32int*) context->blockCopyCacheEnd);
    *(currBlockCopyCacheAddr++) = instr2Copy;

    return currBlockCopyCacheAddr;
  }
  else
  {
    DIE_NOW(context, "asrPCInstruction conditional");
  }
}

u32int *armBicPCInstruction(GCONTXT *context, u32int *instructionAddr, u32int *currBlockCopyCacheAddr, u32int *blockCopyCacheStartAddress)
{
  return standardImmRegRSR(context, instructionAddr, currBlockCopyCacheAddr, blockCopyCacheStartAddress);
}

u32int *armCmnPCInstruction(GCONTXT *context, u32int *instructionAddr, u32int *currBlockCopyCacheAddr, u32int *blockCopyCacheStartAddress)
{
  return standardImmRegRSRNoDest(context, instructionAddr, currBlockCopyCacheAddr, blockCopyCacheStartAddress);
}

u32int *armCmpPCInstruction(GCONTXT *context, u32int *instructionAddr, u32int *currBlockCopyCacheAddr, u32int *blockCopyCacheStartAddress)
{
  return standardImmRegRSRNoDest(context, instructionAddr, currBlockCopyCacheAddr, blockCopyCacheStartAddress);
}

u32int *armEorPCInstruction(GCONTXT *context, u32int *instructionAddr, u32int *currBlockCopyCacheAddr, u32int *blockCopyCacheStartAddress)
{
  return standardImmRegRSR(context, instructionAddr, currBlockCopyCacheAddr, blockCopyCacheStartAddress);
}

u32int *armLslPCInstruction(GCONTXT *context, u32int *instructionAddr, u32int *currBlockCopyCacheAddr, u32int *blockCopyCacheStartAddress)
{
  //This is the same as lsrPCInstruction only direction has changed -> only bit 5 differs
  return armLsrPCInstruction(context, instructionAddr, currBlockCopyCacheAddr, blockCopyCacheStartAddress);
}

u32int *armLsrPCInstruction(GCONTXT *context, u32int *instructionAddr, u32int *currBlockCopyCacheAddr, u32int *blockCopyCacheStartAddress)
{
  u32int instruction = *instructionAddr;
  u32int srcPCRegLoc = 0; //This is where the PC is in the instruction
  u32int destReg = (instruction >> 12) & 0xF;
  u32int instr2Copy = instruction;
  bool conditionAlways = (instruction >> 28 & 0xF) == 0xE;
  //bits 16-19 are zero so Rm or Rn is PC

  if (conditionAlways)
  {
    if ((instruction >> 4 & 0b1) == 1)
    { //Bit 4 is 1 if extra register (LSR(register))
      DIE_NOW(context, "lsrPCInstruction LSR(register) with a srcReg==PC is UNPREDICTABLE?");
    }
    //Ready to do shift
    //save PC
    if ((instruction & 0xF) == 0xF)
    {
      currBlockCopyCacheAddr = savePCInReg(context, instructionAddr, currBlockCopyCacheAddr, destReg);

      //Step 2 modify ldrInstruction
      //Clear PC source Register
      instr2Copy = zeroBits(instruction, srcPCRegLoc);
      instr2Copy = instr2Copy | (destReg << srcPCRegLoc);
    }
    currBlockCopyCacheAddr = checkAndClearBlockCopyCacheAddress(currBlockCopyCacheAddr, context->blockCache, (u32int*) context->blockCopyCache, (u32int*) context->blockCopyCacheEnd);
    *(currBlockCopyCacheAddr++) = instr2Copy;

    return currBlockCopyCacheAddr;
  }
  else
  {
    /*lsrPC Funct conditional*/
    DIE_NOW(context, "lsrPCFunct conditional is not yet implemented");
  }
}

u32int *armMovPCInstruction(GCONTXT *context, u32int *instructionAddr, u32int *currBlockCopyCacheAddr, u32int *blockCopyCacheStartAddress)
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
        DIE_NOW(context, "mov PCFunct: movPCFunct can only be called if last 4 bits are 1111\n");
      }
      else
      {
        //step 1 Copy PC (=instructionAddr2) to desReg
        currBlockCopyCacheAddr = savePCInReg(context, instructionAddr, currBlockCopyCacheAddr, destReg);
        //Step 2 modify ldrInstruction
        //Clear PC source Register
        instr2Copy = zeroBits(instruction, 0); //set last 4 bits equal to zero
        instr2Copy = instr2Copy | (destReg); //set last 4 bits so correct register is used
      }
    }

    currBlockCopyCacheAddr = checkAndClearBlockCopyCacheAddress(currBlockCopyCacheAddr, context->blockCache, (u32int*) context->blockCopyCache, (u32int*) context->blockCopyCacheEnd);
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
        DIE_NOW(context, "mov PCFunct: movPCFunct can only be called if last 4 bits are 1111\n");
      }
      else
      {
        //Make instruction safe and return
        /* conditional instruction thus sometimes not executed */
        /*Instruction has to be changed to a PC safe instructionstream withouth using destReg. */
        u32int srcReg = instruction & 0xF;
        u32int srcPCRegLoc = 0;
        u32int scratchReg = findUnusedRegister(srcReg, destReg, -1);
        /* place 'Backup scratchReg' instruction */
        currBlockCopyCacheAddr = backupRegister(scratchReg, currBlockCopyCacheAddr, blockCopyCacheStartAddress);
        currBlockCopyCacheAddr = savePCInReg(context, instructionAddr, currBlockCopyCacheAddr, scratchReg);

        instr2Copy = zeroBits(instr2Copy, srcPCRegLoc);
        instr2Copy = instr2Copy | scratchReg << srcPCRegLoc;

        currBlockCopyCacheAddr = checkAndClearBlockCopyCacheAddress(currBlockCopyCacheAddr, context->blockCache, (u32int*) context->blockCopyCache, (u32int*) context->blockCopyCacheEnd);
        *(currBlockCopyCacheAddr++) = instr2Copy;

        /* place 'restore scratchReg' instruction */
        currBlockCopyCacheAddr = restoreRegister(scratchReg, currBlockCopyCacheAddr, blockCopyCacheStartAddress);
        /* Make sure scanner sees that we need a word to store the register*/
        currBlockCopyCacheAddr = (u32int*) (((u32int) currBlockCopyCacheAddr) | 0b1);

        return currBlockCopyCacheAddr;
      }
    }

    //if function hasn't returned at this point -> instruction is safe
    currBlockCopyCacheAddr = checkAndClearBlockCopyCacheAddress(currBlockCopyCacheAddr, context->blockCache, (u32int*) context->blockCopyCache, (u32int*) context->blockCopyCacheEnd);
    *(currBlockCopyCacheAddr++) = instruction;

    return currBlockCopyCacheAddr;
  }
}

u32int *armMovtPCInstruction(GCONTXT *context, u32int *instructionAddr, u32int *currBlockCopyCacheAddr, u32int *blockCopyCacheStartAddress)
{
  DIE_NOW(context, "movt PCFunct unfinished\n");
}

u32int *armMovwPCInstruction(GCONTXT *context, u32int *instructionAddr, u32int *currBlockCopyCacheAddr, u32int *blockCopyCacheStartAddress)
{
  //Can be optimized -> this instruction is always safe!
  currBlockCopyCacheAddr = checkAndClearBlockCopyCacheAddress(currBlockCopyCacheAddr, context->blockCache, (u32int*) context->blockCopyCache, (u32int*) context->blockCopyCacheEnd);
  *(currBlockCopyCacheAddr++) = (*instructionAddr);
  return currBlockCopyCacheAddr;
}

u32int *armMvnPCInstruction(GCONTXT *context, u32int *instructionAddr, u32int *currBlockCopyCacheAddr, u32int *blockCopyCacheStartAddress)
{
  u32int instruction = *instructionAddr;
  u32int destReg = (instruction >> 12) & 0xF;
  u32int instr2Copy = instruction;
  bool immediate = (instruction >> 25 & 0b1) == 0b1;
  bool replaceReg1 = FALSE;
  bool conditionAlways = (instruction >> 28 & 0xF) == 0xE;

  if (immediate)
  {
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
        DIE_NOW(context, "MVNPC (register-shifted register) -> UNPREDICTABLE");
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
        currBlockCopyCacheAddr = savePCInReg(context, instructionAddr, currBlockCopyCacheAddr, destReg);
        if (replaceReg1)
        {
          //Step 2 modify eorInstruction
          //Clear PC source Register
          instr2Copy = zeroBits(instruction, 0);
          instr2Copy = instr2Copy | (destReg);
        }
      }
    }
    else
    {
      /* mvn with condition code != ALWAYS*/
      DIE_NOW(context, "conditional mvn PCFunct not yet implemented");
    }
  }

  currBlockCopyCacheAddr = checkAndClearBlockCopyCacheAddress(currBlockCopyCacheAddr, context->blockCache, (u32int*) context->blockCopyCache, (u32int*) context->blockCopyCacheEnd);
  *(currBlockCopyCacheAddr++) = instr2Copy;

  return currBlockCopyCacheAddr;
}

u32int *armOrrPCInstruction(GCONTXT *context, u32int *instructionAddr, u32int *currBlockCopyCacheAddr, u32int *blockCopyCacheStartAddress)
{
  return standardImmRegRSR(context, instructionAddr, currBlockCopyCacheAddr, blockCopyCacheStartAddress);
}

u32int *armRorPCInstruction(GCONTXT *context, u32int *instructionAddr, u32int *currBlockCopyCacheAddr, u32int *blockCopyCacheStartAddress)
{
  DIE_NOW(context, "ror PCFunct unfinished\n");
}

u32int *armRrxPCInstruction(GCONTXT *context, u32int *instructionAddr, u32int *currBlockCopyCacheAddr, u32int *blockCopyCacheStartAddress)
{
  DIE_NOW(context, "rrx PCFunct unfinished\n");
}

u32int *armRsbPCInstruction(GCONTXT *context, u32int *instructionAddr, u32int *currBlockCopyCacheAddr, u32int *blockCopyCacheStartAddress)
{
  return standardImmRegRSR(context, instructionAddr, currBlockCopyCacheAddr, blockCopyCacheStartAddress);
}

u32int *armRscPCInstruction(GCONTXT *context, u32int *instructionAddr, u32int *currBlockCopyCacheAddr, u32int *blockCopyCacheStartAddress)
{
  return standardImmRegRSR(context, instructionAddr, currBlockCopyCacheAddr, blockCopyCacheStartAddress);
}

u32int *armSbcPCInstruction(GCONTXT *context, u32int *instructionAddr, u32int *currBlockCopyCacheAddr, u32int *blockCopyCacheStartAddress)
{
  return standardImmRegRSR(context, instructionAddr, currBlockCopyCacheAddr, blockCopyCacheStartAddress);
}

u32int *armSubPCInstruction(GCONTXT *context, u32int *instructionAddr, u32int *currBlockCopyCacheAddr, u32int *blockCopyCacheStartAddress)
{
  return standardImmRegRSR(context, instructionAddr, currBlockCopyCacheAddr, blockCopyCacheStartAddress);
}

u32int *armTeqPCInstruction(GCONTXT *context, u32int *instructionAddr, u32int *currBlockCopyCacheAddr, u32int *blockCopyCacheStartAddress)
{
  return standardImmRegRSRNoDest(context, instructionAddr, currBlockCopyCacheAddr, blockCopyCacheStartAddress);
}

u32int *armTstPCInstruction(GCONTXT *context, u32int *instructionAddr, u32int *currBlockCopyCacheAddr, u32int *blockCopyCacheStartAddress)
{
  return standardImmRegRSRNoDest(context, instructionAddr, currBlockCopyCacheAddr, blockCopyCacheStartAddress);
}
