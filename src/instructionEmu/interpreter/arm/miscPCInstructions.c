#include "instructionEmu/interpreter/internals.h"

#include "instructionEmu/interpreter/arm/miscPCInstructions.h"


u32int *armBkptPCInstruction(GCONTXT *context, u32int *instructionAddr, u32int *currBlockCopyCacheAddr, u32int *blockCopyCacheStartAddress)
{
  DIE_NOW(context, "not implemented");
}

u32int *armClzPCInstruction(GCONTXT *context, u32int *instructionAddr, u32int *currBlockCopyCacheAddr, u32int *blockCopyCacheStartAddress)
{
  u32int instruction = *instructionAddr;

  if((instruction & 0xF) == 0xF || ((instruction>>12 & 0xF)==0xF))
  {
    //see ARM ARM clz p.384
    DIE_NOW(context, "clz PCFunct: bits 0-3 = 0xF -> UNPREDICTABLE BEHAVIOR\n");
  }
  //Since no register can be PC the instruction is save to execute -> copy it to blockCopyCache
  currBlockCopyCacheAddr=checkAndClearBlockCopyCacheAddress(currBlockCopyCacheAddr,context->blockCache,(u32int*)context->blockCopyCache,(u32int*)context->blockCopyCacheEnd);
  *(currBlockCopyCacheAddr++)=instruction;

  return currBlockCopyCacheAddr;
}

u32int *armCpsPCInstruction(GCONTXT *context, u32int *instructionAddr, u32int *currBlockCopyCacheAddr, u32int *blockCopyCacheStartAddress)
{
  DIE_NOW(context, "not implemented");
}

u32int *armDbgPCInstruction(GCONTXT *context, u32int *instructionAddr, u32int *currBlockCopyCacheAddr, u32int *blockCopyCacheStartAddress)
{
  DIE_NOW(context, "not implemented");
}

u32int *armDmbPCInstruction(GCONTXT *context, u32int *instructionAddr, u32int *currBlockCopyCacheAddr, u32int *blockCopyCacheStartAddress)
{
  DIE_NOW(context, "not implemented");
}

u32int *armDsbPCInstruction(GCONTXT *context, u32int *instructionAddr, u32int *currBlockCopyCacheAddr, u32int *blockCopyCacheStartAddress)
{
  DIE_NOW(context, "not implemented");
}

u32int *armIsbPCInstruction(GCONTXT *context, u32int *instructionAddr, u32int *currBlockCopyCacheAddr, u32int *blockCopyCacheStartAddress)
{
  DIE_NOW(context, "isb PCFunct shouldn't be used since isbInstructions are always emulated!!\n");
}

u32int *armMrsPCInstruction(GCONTXT *context, u32int *instructionAddr, u32int *currBlockCopyCacheAddr, u32int *blockCopyCacheStartAddress)
{
  DIE_NOW(context, "not implemented");
}

u32int *armMsrPCInstruction(GCONTXT *context, u32int *instructionAddr, u32int *currBlockCopyCacheAddr, u32int *blockCopyCacheStartAddress)
{
  DIE_NOW(context, "not implemented");
}

u32int *armPldPCInstruction(GCONTXT *context, u32int *instructionAddr, u32int *currBlockCopyCacheAddr, u32int *blockCopyCacheStartAddress)
{
  DIE_NOW(context, "not implemented");
}

u32int *armPliPCInstruction(GCONTXT *context, u32int *instructionAddr, u32int *currBlockCopyCacheAddr, u32int *blockCopyCacheStartAddress)
{
  DIE_NOW(context, "not implemented");
}

u32int *armRfePCInstruction(GCONTXT *context, u32int *instructionAddr, u32int *currBlockCopyCacheAddr, u32int *blockCopyCacheStartAddress)
{
  DIE_NOW(context, "not implemented");
}

u32int *armSetendPCInstruction(GCONTXT *context, u32int *instructionAddr, u32int *currBlockCopyCacheAddr, u32int *blockCopyCacheStartAddress)
{
  DIE_NOW(context, "not implemented");
}

u32int *armSevPCInstruction(GCONTXT *context, u32int *instructionAddr, u32int *currBlockCopyCacheAddr, u32int *blockCopyCacheStartAddress)
{
  DIE_NOW(context, "not implemented");
}

u32int *armSmcPCInstruction(GCONTXT *context, u32int *instructionAddr, u32int *currBlockCopyCacheAddr, u32int *blockCopyCacheStartAddress)
{
  DIE_NOW(context, "not implemented");
}

u32int *armSrsPCInstruction(GCONTXT *context, u32int *instructionAddr, u32int *currBlockCopyCacheAddr, u32int *blockCopyCacheStartAddress)
{
  DIE_NOW(context, "not implemented");
}

u32int *armWfePCInstruction(GCONTXT *context, u32int *instructionAddr, u32int *currBlockCopyCacheAddr, u32int *blockCopyCacheStartAddress)
{
  DIE_NOW(context, "not implemented");
}

u32int *armWfiPCInstruction(GCONTXT *context, u32int *instructionAddr, u32int *currBlockCopyCacheAddr, u32int *blockCopyCacheStartAddress)
{
  DIE_NOW(context, "not implemented");
}

u32int *armYieldPCInstruction(GCONTXT *context, u32int *instructionAddr, u32int *currBlockCopyCacheAddr, u32int *blockCopyCacheStartAddress)
{
  DIE_NOW(context, "not implemented");
}
