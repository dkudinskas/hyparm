#include "common/debug.h"

#include "instructionEmu/commonInstrFunctions.h"

#include "instructionEmu/interpreter/arm/storePCInstructions.h"


u32int *armStrPCInstruction(GCONTXT *context, u32int *instructionAddr, u32int *currBlockCopyCacheAddr, u32int *blockCopyCacheStartAddress)
{
  u32int instruction = *instructionAddr;
  u32int srcPCRegLoc = 16; //This is where the PC is in the instruction (if immediate always at bit 16 if not can be at bit 0)
  u32int destReg = (instruction >> 12) & 0xF;
  u32int instr2Copy = instruction;
  bool conditionAlways = (instruction >> 28 & 0xF) == 0xE;

  if (((instruction >> 25 & 0b1) == 0b1) & ((instruction & 0xF) == 0xF))
  { //bit 25 is 1 when there are 2 source registers
    DIE_NOW(context, "str PCFunct: str (register) cannot have Rm as PC -> UNPREDICTABLE");
  }

  if (conditionAlways)
  {
    if ((instruction >> srcPCRegLoc & 0xF) == 0xF) //There only have to be taken measures if Rn is PC
    {
      //step 1 Copy PC (=instructionAddr2) to desReg
      currBlockCopyCacheAddr = savePCInReg(context, instructionAddr, currBlockCopyCacheAddr, destReg);

      //Step 2 modify strInstruction
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
    /* condition might be false */
    DIE_NOW(context, "conditional strPCFunct not yet implemented");
  }
}

u32int *armStrbPCInstruction(GCONTXT *context, u32int *instructionAddr, u32int *currBlockCopyCacheAddr, u32int *blockCopyCacheStartAddress)
{
  DIE_NOW(context, "strb PCFunct unfinished\n");
}

u32int *armStrhPCInstruction(GCONTXT *context, u32int *instructionAddr, u32int *currBlockCopyCacheAddr, u32int *blockCopyCacheStartAddress)
{
  DIE_NOW(context, "strh PCFunct unfinished\n");
}

u32int *armStrdPCInstruction(GCONTXT *context, u32int *instructionAddr, u32int *currBlockCopyCacheAddr, u32int *blockCopyCacheStartAddress)
{
  DIE_NOW(context, "strd PCFunct unfinished\n");
}

u32int *armStrhtPCInstruction(GCONTXT *context, u32int *instructionAddr, u32int *currBlockCopyCacheAddr, u32int *blockCopyCacheStartAddress)
{
  DIE_NOW(context, "strht PCFunct unfinished\n");
}

u32int* armStmPCInstruction(GCONTXT *context, u32int *instructionAddr, u32int *currBlockCopyCacheAddr, u32int *blockCopyCacheStartAddress)
{
  u32int instruction = *instructionAddr;
  if ((instruction & 0xF0000) == 0xF0000)
  {
    // According to ARM ARM: source register = PC ->  UNPREDICTABLE
    DIE_NOW(context, "stm PC had PC as Rn -> UNPREDICTABLE?!\n");
  }
  else
  {
    //Stores multiple registers to consecutive memory locations
    //PC is not used -> instruction is save to execute just copy it to blockCopyCache
    currBlockCopyCacheAddr = checkAndClearBlockCopyCacheAddress(currBlockCopyCacheAddr, context->blockCache, (u32int*) context->blockCopyCache, (u32int*) context->blockCopyCacheEnd);
    *(currBlockCopyCacheAddr++) = instruction;

    return currBlockCopyCacheAddr;
  }
}
