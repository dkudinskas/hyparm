#include "common/debug.h"

#include "cpuArch/constants.h"

#include "instructionEmu/interpreter/blockCopy.h"
#include "instructionEmu/interpreter/internals.h"

#include "instructionEmu/interpreter/arm/loadPCInstructions.h"


/*
 * ldrPCInstruction is only called when destReg != PC
 */
u32int *armLdrPCInstruction(TranslationCache *tc, u32int *instructionAddr, u32int *currBlockCopyCacheAddr, u32int *blockCopyCacheStartAddress)
{
  //This is where the PC is in the instruction (if immediate always at bit 16 if not can be at bit 0)
  const u32int RS_PC_INDEX = 16;

  u32int instruction = *instructionAddr;
  u32int srcReg1 = (instruction >> RS_PC_INDEX) & 0xF;
  u32int destReg = (instruction >> 12) & 0xF;
  u32int instr2Copy = instruction;

  if (((instruction >> 25 & 1) == 1) && ((instruction & 0xF) == GPR_PC))
  { //bit 25 is 1 when there are 2 source registers
    //see ARM ARM p 436 Rm cannot be PC
    DIE_NOW(NULL, "ldr PCFunct (register) with Rm = PC -> UNPREDICTABLE\n");
  }
  if (srcReg1 != GPR_PC)
  {
    //It is safe to just copy the instruction
    currBlockCopyCacheAddr = updateCodeCachePointer(tc, currBlockCopyCacheAddr);
    *(currBlockCopyCacheAddr++) = instr2Copy;
    return currBlockCopyCacheAddr;
  }
  if (ARM_EXTRACT_CONDITION_CODE(instruction) != CC_AL)
  {
    //Here starts the general procedure.  For this srcPCRegLoc must be set correctly
    //step 1 Copy PC (=instructionAddr2) to desReg
    currBlockCopyCacheAddr = savePCInReg(tc, instructionAddr, currBlockCopyCacheAddr, destReg);

    //Step 2 modify ldrInstruction
    //Clear PC source Register
    instr2Copy = (instruction & ~(0xF << RS_PC_INDEX)) | (destReg << RS_PC_INDEX);

    currBlockCopyCacheAddr = updateCodeCachePointer(tc, currBlockCopyCacheAddr);
    *(currBlockCopyCacheAddr++) = instr2Copy;
    return currBlockCopyCacheAddr;
  }

  /* conditional instruction thus sometimes not executed */
  /*Instruction has to be changed to a PC safe instructionstream withouth using destReg. */
  u32int scratchReg = getOtherRegisterOf2(srcReg1, destReg);
  /* place 'Backup scratchReg' instruction */
  currBlockCopyCacheAddr = backupRegister(tc, scratchReg, currBlockCopyCacheAddr, blockCopyCacheStartAddress);
  currBlockCopyCacheAddr = savePCInReg(tc, instructionAddr, currBlockCopyCacheAddr, scratchReg);

  instr2Copy = (instruction & ~(0xF << RS_PC_INDEX)) | (scratchReg << RS_PC_INDEX);

  currBlockCopyCacheAddr = updateCodeCachePointer(tc, currBlockCopyCacheAddr);
  *(currBlockCopyCacheAddr++) = instr2Copy;

  /* place 'restore scratchReg' instruction */
  currBlockCopyCacheAddr = restoreRegister(tc, scratchReg, currBlockCopyCacheAddr, blockCopyCacheStartAddress);
  /* Make sure scanner sees that we need a word to store the register*/
  currBlockCopyCacheAddr = (u32int*) (((u32int) currBlockCopyCacheAddr) | 0b1);

  return currBlockCopyCacheAddr;
}

u32int *armLdrbPCInstruction(TranslationCache *tc, u32int *instructionAddr, u32int *currBlockCopyCacheAddr, u32int *blockCopyCacheStartAddress)
{
  DIE_NOW(NULL, "ldrh PCFunct unfinished\n");
}

u32int *armLdrhPCInstruction(TranslationCache *tc, u32int *instructionAddr, u32int *currBlockCopyCacheAddr, u32int *blockCopyCacheStartAddress)
{
  DIE_NOW(NULL, "ldrh PCFunct unfinished\n");
}

u32int *armLdrdPCInstruction(TranslationCache *tc, u32int *instructionAddr, u32int *currBlockCopyCacheAddr, u32int *blockCopyCacheStartAddress)
{
  DIE_NOW(NULL, "ldrd PCFunct unfinished\n");
}

u32int *armPopLdmPCInstruction(TranslationCache *tc, u32int *instructionAddr, u32int *currBlockCopyCacheAddr, u32int *blockCopyCacheStartAddress)
{
  DIE_NOW(NULL, "popLdm PCFunct unfinished\n");
}
