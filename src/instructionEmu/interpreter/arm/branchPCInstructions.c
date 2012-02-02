#include "common/debug.h"

#include "instructionEmu/interpreter/arm/branchPCInstructions.h"


u32int *armBPCInstruction(GCONTXT *context, u32int *instructionAddr, u32int *currBlockCopyCacheAddr, u32int *blockCopyCacheStartAddress)
{
  //Currently a bInstruction is always replaced by an SVC -> do nothing and check for PC in handleFunct
  return 0;
}

u32int *armBlxPCInstruction(GCONTXT *context, u32int *instructionAddr, u32int *currBlockCopyCacheAddr, u32int *blockCopyCacheStartAddress)
{
  DIE_NOW(context, "blx PCFunct unfinished\n");
}

u32int *armBxPCInstruction(GCONTXT *context, u32int *instructionAddr, u32int *currBlockCopyCacheAddr, u32int *blockCopyCacheStartAddress)
{
  DIE_NOW(context, "bx PCFunct unfinished\n");
}

u32int *armBxjPCInstruction(GCONTXT *context, u32int *instructionAddr, u32int *currBlockCopyCacheAddr, u32int *blockCopyCacheStartAddress)
{
  DIE_NOW(context, "bxj PCFunct unfinished\n");
}
