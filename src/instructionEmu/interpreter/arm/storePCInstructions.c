#include "common/debug.h"

#include "cpuArch/constants.h"

#include "instructionEmu/interpreter/blockCopy.h"
#include "instructionEmu/interpreter/internals.h"

#include "instructionEmu/interpreter/arm/storePCInstructions.h"


u32int *armStrPCInstruction(TranslationCache *tc, u32int *instructionAddr, u32int *currBlockCopyCacheAddr, u32int *blockCopyCacheStartAddress)
{
  u32int instruction = *instructionAddr;
  const u32int srcPCRegLoc = 16; //This is where the PC is in the instruction (if immediate always at bit 16 if not can be at bit 0)
  u32int destReg = (instruction >> 12) & 0xF;
  u32int instr2Copy = instruction;
  u32int conditionCode = ARM_EXTRACT_CONDITION_CODE(instruction);

  if (((instruction >> 25 & 0b1) == 0b1) & ((instruction & 0xF) == 0xF))
  { //bit 25 is 1 when there are 2 source registers
    DIE_NOW(NULL, "str PCFunct: str (register) cannot have Rm as PC -> UNPREDICTABLE");
  }

  if ((instruction >> srcPCRegLoc & 0xF) == 0xF) //There only have to be taken measures if Rn is PC
  {
    //step 1 Copy PC (=instructionAddr2) to desReg
    currBlockCopyCacheAddr = armWritePCToRegister(tc, currBlockCopyCacheAddr, conditionCode, destReg, (u32int)instructionAddr);

    //Step 2 modify strInstruction
    //Clear PC source Register
    instr2Copy = (instruction & ~(0xF << srcPCRegLoc)) | (destReg << srcPCRegLoc);
  }

  currBlockCopyCacheAddr = updateCodeCachePointer(tc, currBlockCopyCacheAddr);
  *(currBlockCopyCacheAddr++) = instr2Copy;

  return currBlockCopyCacheAddr;
}

u32int *armStrbPCInstruction(TranslationCache *tc, u32int *instructionAddr, u32int *currBlockCopyCacheAddr, u32int *blockCopyCacheStartAddress)
{
  DIE_NOW(NULL, "strb PCFunct unfinished\n");
}

u32int *armStrhPCInstruction(TranslationCache *tc, u32int *instructionAddr, u32int *currBlockCopyCacheAddr, u32int *blockCopyCacheStartAddress)
{
  DIE_NOW(NULL, "strh PCFunct unfinished\n");
}

u32int *armStrdPCInstruction(TranslationCache *tc, u32int *instructionAddr, u32int *currBlockCopyCacheAddr, u32int *blockCopyCacheStartAddress)
{
  DIE_NOW(NULL, "strd PCFunct unfinished\n");
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
