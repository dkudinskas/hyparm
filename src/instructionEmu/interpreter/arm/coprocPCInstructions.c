#include "common/debug.h"

#include "instructionEmu/interpreter/arm/coprocPCInstructions.h"


u32int *armCdpPCInstruction(GCONTXT *context, u32int *instructionAddr, u32int *currBlockCopyCacheAddr, u32int *blockCopyCacheStartAddress)
{
  DIE_NOW(context, "cdp PCFunct unfinished\n");
}

u32int *armCdp2PCInstruction(GCONTXT *context, u32int *instructionAddr, u32int *currBlockCopyCacheAddr, u32int *blockCopyCacheStartAddress)
{
  DIE_NOW(context, "cdp2 PCFunct unfinished\n");
}

u32int *armLdcPCInstruction(GCONTXT *context, u32int *instructionAddr, u32int *currBlockCopyCacheAddr, u32int *blockCopyCacheStartAddress)
{
  DIE_NOW(context, "ldc PCFunct unfinished\n");
}

u32int *armLdc2PCInstruction(GCONTXT *context, u32int *instructionAddr, u32int *currBlockCopyCacheAddr, u32int *blockCopyCacheStartAddress)
{
  DIE_NOW(context, "ldc2 PCFunct unfinished\n");
}

u32int *armMcrPCInstruction(GCONTXT *context, u32int *instructionAddr, u32int *currBlockCopyCacheAddr, u32int *blockCopyCacheStartAddress)
{
  DIE_NOW(context, "mcr PCFunct shouldn't be launched since mcrPCInstruction is always emulated!\n");
}

u32int *armMcr2PCInstruction(GCONTXT *context, u32int *instructionAddr, u32int *currBlockCopyCacheAddr, u32int *blockCopyCacheStartAddress)
{
  DIE_NOW(context, "mcr2 PCFunct unfinished\n");
}

u32int *armMcrrPCInstruction(GCONTXT *context, u32int *instructionAddr, u32int *currBlockCopyCacheAddr, u32int *blockCopyCacheStartAddress)
{
  DIE_NOW(context, "mcrr PCFunct unfinished\n");
}

u32int *armMcrr2PCInstruction(GCONTXT *context, u32int *instructionAddr, u32int *currBlockCopyCacheAddr, u32int *blockCopyCacheStartAddress)
{
  DIE_NOW(context, "mcrr2 PCFunct unfinished\n");
}

u32int *armMrcPCInstruction(GCONTXT *context, u32int *instructionAddr, u32int *currBlockCopyCacheAddr, u32int *blockCopyCacheStartAddress)
{
  DIE_NOW(context, "mrcPCInstruction is called but should always be critical");
}

u32int *armMrc2PCInstruction(GCONTXT *context, u32int *instructionAddr, u32int *currBlockCopyCacheAddr, u32int *blockCopyCacheStartAddress)
{
  DIE_NOW(context, "mrc2 PCFunct unfinished\n");
}

u32int *armMrrcPCInstruction(GCONTXT *context, u32int *instructionAddr, u32int *currBlockCopyCacheAddr, u32int *blockCopyCacheStartAddress)
{
  DIE_NOW(context, "mrrc PCFunct unfinished\n");
}

u32int *armMrrc2PCInstruction(GCONTXT *context, u32int *instructionAddr, u32int *currBlockCopyCacheAddr, u32int *blockCopyCacheStartAddress)
{
  DIE_NOW(context, "mrrc2 PCFunct unfinished\n");
}

u32int *armStcPCInstruction(GCONTXT *context, u32int *instructionAddr, u32int *currBlockCopyCacheAddr, u32int *blockCopyCacheStartAddress)
{
  DIE_NOW(context, "stc PCFunct unfinished\n");
}

u32int *armStc2PCInstruction(GCONTXT *context, u32int *instructionAddr, u32int *currBlockCopyCacheAddr, u32int *blockCopyCacheStartAddress)
{
  DIE_NOW(context, "stc2 PCFunct unfinished\n");
}
