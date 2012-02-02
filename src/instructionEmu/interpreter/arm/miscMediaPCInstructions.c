#include "instructionEmu/interpreter/internals.h"

#include "instructionEmu/interpreter/arm/miscMediaPCInstructions.h"


u32int *armBfcPCInstruction(GCONTXT *context, u32int *instructionAddr, u32int *currBlockCopyCacheAddr, u32int *blockCopyCacheStartAddress)
{
  /* Normally always safe*/
  u32int instruction = *instructionAddr;
  u32int destReg = (instruction>>12) & 0xF;
  if(destReg == 0xF)
  {
    DIE_NOW(context,"bfc PC: with Rd == PC -> UNPREDICTABLE");
  }

  //Other fields are safe
  currBlockCopyCacheAddr=checkAndClearBlockCopyCacheAddress(currBlockCopyCacheAddr,context->blockCache,(u32int*)context->blockCopyCache,(u32int*)context->blockCopyCacheEnd);
  *(currBlockCopyCacheAddr++)=instruction;

  return currBlockCopyCacheAddr;
}

u32int *armBfiPCInstruction(GCONTXT *context, u32int *instructionAddr, u32int *currBlockCopyCacheAddr, u32int *blockCopyCacheStartAddress)
{
  DIE_NOW(context, "not implemented");
}

u32int *armPkhbtPCInstruction(GCONTXT *context, u32int *instructionAddr, u32int *currBlockCopyCacheAddr, u32int *blockCopyCacheStartAddress)
{
  DIE_NOW(context, "not implemented");
}

u32int *armPkhtbPCInstruction(GCONTXT *context, u32int *instructionAddr, u32int *currBlockCopyCacheAddr, u32int *blockCopyCacheStartAddress)
{
  DIE_NOW(context, "not implemented");
}

u32int *armRbitPCInstruction(GCONTXT *context, u32int *instructionAddr, u32int *currBlockCopyCacheAddr, u32int *blockCopyCacheStartAddress)
{
  DIE_NOW(context, "not implemented");
}

u32int *armRevPCInstruction(GCONTXT *context, u32int *instructionAddr, u32int *currBlockCopyCacheAddr, u32int *blockCopyCacheStartAddress)
{
  DIE_NOW(context, "not implemented");
}

u32int *armRev16PCInstruction(GCONTXT *context, u32int *instructionAddr, u32int *currBlockCopyCacheAddr, u32int *blockCopyCacheStartAddress)
{
  DIE_NOW(context, "not implemented");
}

u32int *armRevshPCInstruction(GCONTXT *context, u32int *instructionAddr, u32int *currBlockCopyCacheAddr, u32int *blockCopyCacheStartAddress)
{
  DIE_NOW(context, "not implemented");
}

u32int *armSbfxPCInstruction(GCONTXT *context, u32int *instructionAddr, u32int *currBlockCopyCacheAddr, u32int *blockCopyCacheStartAddress)
{
  return armUbfxPCInstruction(context, instructionAddr, currBlockCopyCacheAddr, blockCopyCacheStartAddress);
}

u32int *armSelPCInstruction(GCONTXT *context, u32int *instructionAddr, u32int *currBlockCopyCacheAddr, u32int *blockCopyCacheStartAddress)
{
  DIE_NOW(context, "not implemented");
}

u32int *armSsatPCInstruction(GCONTXT *context, u32int *instructionAddr, u32int *currBlockCopyCacheAddr, u32int *blockCopyCacheStartAddress)
{
  DIE_NOW(context, "not implemented");
}

u32int *armSsat16PCInstruction(GCONTXT *context, u32int *instructionAddr, u32int *currBlockCopyCacheAddr, u32int *blockCopyCacheStartAddress)
{
  DIE_NOW(context, "not implemented");
}

u32int *armSxtabPCInstruction(GCONTXT *context, u32int *instructionAddr, u32int *currBlockCopyCacheAddr, u32int *blockCopyCacheStartAddress)
{
  DIE_NOW(context, "not implemented");
}

u32int *armSxtab16PCInstruction(GCONTXT *context, u32int *instructionAddr, u32int *currBlockCopyCacheAddr, u32int *blockCopyCacheStartAddress)
{
  DIE_NOW(context, "not implemented");
}

u32int *armSxtahPCInstruction(GCONTXT *context, u32int *instructionAddr, u32int *currBlockCopyCacheAddr, u32int *blockCopyCacheStartAddress)
{
  DIE_NOW(context, "not implemented");
}

u32int *armSxtbPCInstruction(GCONTXT *context, u32int *instructionAddr, u32int *currBlockCopyCacheAddr, u32int *blockCopyCacheStartAddress)
{
  u32int instruction = *instructionAddr;

  if((instruction & 0xF)==0xF)
  {
    //ARM ARM p.752
    DIE_NOW(context, "stxb PC: Rm = PC -> UNPREDICTABLE!");
  }
  //Always safe when not unpredictable
  currBlockCopyCacheAddr=checkAndClearBlockCopyCacheAddress(currBlockCopyCacheAddr,context->blockCache,(u32int*)context->blockCopyCache,(u32int*)context->blockCopyCacheEnd);
  *(currBlockCopyCacheAddr++)=instruction;

  return currBlockCopyCacheAddr;
}

u32int *armSxtb16PCInstruction(GCONTXT *context, u32int *instructionAddr, u32int *currBlockCopyCacheAddr, u32int *blockCopyCacheStartAddress)
{
  DIE_NOW(context, "not implemented");
}

u32int *armSxthPCInstruction(GCONTXT *context, u32int *instructionAddr, u32int *currBlockCopyCacheAddr, u32int *blockCopyCacheStartAddress)
{
  DIE_NOW(context, "not implemented");
}

u32int *armUbfxPCInstruction(GCONTXT *context, u32int *instructionAddr, u32int *currBlockCopyCacheAddr, u32int *blockCopyCacheStartAddress)
{
  //This cannot be a dangerous function see ARM ARM p. 778
  u32int instruction = *instructionAddr;
  if((instruction & 0xF) == 0xF || ((instruction>>12)&0xF)==0xF){
    DIE_NOW(context, "ubfx PCFunct: with Rd or Rn == PC -> UNPREDICTABLE");
  }
  //Just copy the instruction
  currBlockCopyCacheAddr=checkAndClearBlockCopyCacheAddress(currBlockCopyCacheAddr,context->blockCache,(u32int*)context->blockCopyCache,(u32int*)context->blockCopyCacheEnd);
  *(currBlockCopyCacheAddr++)=instruction;

  return currBlockCopyCacheAddr;
}

u32int *armUsad8PCInstruction(GCONTXT *context, u32int *instructionAddr, u32int *currBlockCopyCacheAddr, u32int *blockCopyCacheStartAddress)
{
  DIE_NOW(context, "not implemented");
}

u32int *armUsada8PCInstruction(GCONTXT *context, u32int *instructionAddr, u32int *currBlockCopyCacheAddr, u32int *blockCopyCacheStartAddress)
{
  DIE_NOW(context, "not implemented");
}

u32int *armUsatPCInstruction(GCONTXT *context, u32int *instructionAddr, u32int *currBlockCopyCacheAddr, u32int *blockCopyCacheStartAddress)
{
  DIE_NOW(context, "not implemented");
}

u32int *armUsat16PCInstruction(GCONTXT *context, u32int *instructionAddr, u32int *currBlockCopyCacheAddr, u32int *blockCopyCacheStartAddress)
{
  DIE_NOW(context, "not implemented");
}

u32int *armUxtabPCInstruction(GCONTXT *context, u32int *instructionAddr, u32int *currBlockCopyCacheAddr, u32int *blockCopyCacheStartAddress)
{
  DIE_NOW(context, "not implemented");
}

u32int *armUxtab16PCInstruction(GCONTXT *context, u32int *instructionAddr, u32int *currBlockCopyCacheAddr, u32int *blockCopyCacheStartAddress)
{
  DIE_NOW(context, "not implemented");
}

u32int *armUxtahPCInstruction(GCONTXT *context, u32int *instructionAddr, u32int *currBlockCopyCacheAddr, u32int *blockCopyCacheStartAddress)
{
  DIE_NOW(context, "not implemented");
}

u32int *armUxtbPCInstruction(GCONTXT *context, u32int *instructionAddr, u32int *currBlockCopyCacheAddr, u32int *blockCopyCacheStartAddress)
{
  //Can be optimized -> this instruction is always safe!
  u32int instruction = *instructionAddr;
  if( (instruction & 0xF) == 0xF)
  {
    DIE_NOW(context,"uxtbPCInstruction: Rm is PC -> unpredictable");
  }

  currBlockCopyCacheAddr=checkAndClearBlockCopyCacheAddress(currBlockCopyCacheAddr,context->blockCache,(u32int*)context->blockCopyCache,(u32int*)context->blockCopyCacheEnd);
  *(currBlockCopyCacheAddr++)=instruction;

  return currBlockCopyCacheAddr;
}

u32int *armUxtb16PCInstruction(GCONTXT *context, u32int *instructionAddr, u32int *currBlockCopyCacheAddr, u32int *blockCopyCacheStartAddress)
{
  DIE_NOW(context, "not implemented");
}

u32int *armUxthPCInstruction(GCONTXT *context, u32int *instructionAddr, u32int *currBlockCopyCacheAddr, u32int *blockCopyCacheStartAddress)
{
  u32int instruction = *instructionAddr;

  if((instruction & 0xF)==0xF)
  {
    //ARM ARM p.836
    DIE_NOW(context, "Rm = PC -> UNPREDICTABLE!");
  }
  //Always safe when not unpredictable
  currBlockCopyCacheAddr=checkAndClearBlockCopyCacheAddress(currBlockCopyCacheAddr,context->blockCache,(u32int*)context->blockCopyCache,(u32int*)context->blockCopyCacheEnd);
  *(currBlockCopyCacheAddr++)=instruction;

  return currBlockCopyCacheAddr;
}
