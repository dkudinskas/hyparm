#include "common/debug.h"

#include "instructionEmu/commonInstrFunctions.h"

#include "instructionEmu/interpreter/loadPCInstructions.h"


/*
 * ldrPCInstruction is only called when destReg != PC
 */
u32int *armLdrPCInstruction(GCONTXT *context, u32int *instructionAddr, u32int *currBlockCopyCacheAddr, u32int *blockCopyCacheStartAddress)
{
  u32int instruction = *instructionAddr;
  u32int srcPCRegLoc = 16; //This is where the PC is in the instruction (if immediate always at bit 16 if not can be at bit 0)
  u32int srcReg1 = (instruction >> srcPCRegLoc) & 0xF;
  bool srcReg1IsPC = (srcReg1) == 0xF;
  u32int destReg = (instruction >> 12) & 0xF;
  u32int instr2Copy = instruction;
  bool conditionAlways = (instruction >> 28 & 0xF) == 0xE;
  u32int scratchReg;
  if (((instruction >> 25 & 0b1) == 1) && ((instruction & 0xF) == 0xF))
  { //bit 25 is 1 when there are 2 source registers
    //see ARM ARM p 436 Rm cannot be PC
    DIE_NOW(context, "ldr PCFunct (register) with Rm = PC -> UNPREDICTABLE\n");
  }
  if (!srcReg1IsPC)
  {
    //It is safe to just copy the instruction
    currBlockCopyCacheAddr = checkAndClearBlockCopyCacheAddress(currBlockCopyCacheAddr, context->blockCache, (u32int*) context->blockCopyCache, (u32int*) context->blockCopyCacheEnd);
    *(currBlockCopyCacheAddr++) = instr2Copy;
    return currBlockCopyCacheAddr;
  }
  if (conditionAlways)
  {
    //Here starts the general procedure.  For this srcPCRegLoc must be set correctly
    //step 1 Copy PC (=instructionAddr2) to desReg
    currBlockCopyCacheAddr = savePCInReg(context, instructionAddr, currBlockCopyCacheAddr, destReg);

    //Step 2 modify ldrInstruction
    //Clear PC source Register
    instr2Copy = zeroBits(instruction, srcPCRegLoc);
    instr2Copy = instr2Copy | (destReg << srcPCRegLoc);

    currBlockCopyCacheAddr = checkAndClearBlockCopyCacheAddress(currBlockCopyCacheAddr, context->blockCache, (u32int*) context->blockCopyCache, (u32int*) context->blockCopyCacheEnd);
    *(currBlockCopyCacheAddr++) = instr2Copy;
    return currBlockCopyCacheAddr;
  }
  else
  {
    /* conditional instruction thus sometimes not executed */
    /*Instruction has to be changed to a PC safe instructionstream withouth using destReg. */
    scratchReg = findUnusedRegister(srcReg1, destReg, -1);
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

u32int *armLdrbPCInstruction(GCONTXT *context, u32int *instructionAddr, u32int *currBlockCopyCacheAddr, u32int *blockCopyCacheStartAddress)
{
  DIE_NOW(context, "ldrb PCFunct unfinished\n");
}

u32int *armLdrhPCInstruction(GCONTXT *context, u32int *instructionAddr, u32int *currBlockCopyCacheAddr, u32int *blockCopyCacheStartAddress)
{
  DIE_NOW(context, "ldrh PCFunct unfinished\n");
}

u32int *armLdrdPCInstruction(GCONTXT *context, u32int *instructionAddr, u32int *currBlockCopyCacheAddr, u32int *blockCopyCacheStartAddress)
{
  DIE_NOW(context, "ldrd PCFunct unfinished\n");
}

u32int *armLdrhtPCInstruction(GCONTXT *context, u32int *instructionAddr, u32int *currBlockCopyCacheAddr, u32int *blockCopyCacheStartAddress)
{
  DIE_NOW(context, "ldrht PCFunct unfinished\n");
}

u32int *armLdrexPCInstruction(GCONTXT *context, u32int *instructionAddr, u32int *currBlockCopyCacheAddr, u32int *blockCopyCacheStartAddress)
{
  DIE_NOW(context, "ldrex PCFunct unfinished\n");
}

u32int *armLdrexbPCInstruction(GCONTXT *context, u32int *instructionAddr, u32int *currBlockCopyCacheAddr, u32int *blockCopyCacheStartAddress)
{
  DIE_NOW(context, "ldrexb PCFunct unfinished\n");
}

u32int *armLdrexhPCInstruction(GCONTXT *context, u32int *instructionAddr, u32int *currBlockCopyCacheAddr, u32int *blockCopyCacheStartAddress)
{
  DIE_NOW(context, "ldrexh PCFunct unfinished\n");
}

u32int *armLdrexdPCInstruction(GCONTXT *context, u32int *instructionAddr, u32int *currBlockCopyCacheAddr, u32int *blockCopyCacheStartAddress)
{
  DIE_NOW(context, "ldrexd PCFunct unfinished\n");
}

u32int *armLdmPCInstruction(GCONTXT *context, u32int *instructionAddr, u32int *currBlockCopyCacheAddr, u32int *blockCopyCacheStartAddress)
{
  u32int instruction = *instructionAddr;
  if ((instruction >> 16 & 0xF) == 0xF)
  {
    DIE_NOW(context, "ldm that is using PC is UNPREDICTABLE"); //see ARM ARM P.424-428
  }
  //This means that instruction is always save
  currBlockCopyCacheAddr = checkAndClearBlockCopyCacheAddress(currBlockCopyCacheAddr, context->blockCache, (u32int*) context->blockCopyCache, (u32int*) context->blockCopyCacheEnd);
  *(currBlockCopyCacheAddr++) = instruction;

  return currBlockCopyCacheAddr;
}

u32int *armPopLdrPCInstruction(GCONTXT *context, u32int *instructionAddr, u32int *currBlockCopyCacheAddr, u32int *blockCopyCacheStartAddress)
{
  DIE_NOW(context, "popLdr PCFunct unfinished\n");
}

u32int *armPopLdmPCInstruction(GCONTXT *context, u32int *instructionAddr, u32int *currBlockCopyCacheAddr, u32int *blockCopyCacheStartAddress)
{
  DIE_NOW(context, "popLdm PCFunct unfinished\n");
}
