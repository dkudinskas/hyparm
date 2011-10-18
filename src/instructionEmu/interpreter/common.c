#include "instructionEmu/interpreter/common.h"
#include "instructionEmu/interpreter/internals.h"


u32int nopInstruction(GCONTXT *context, u32int instruction)
{
  TRACE(context, instruction);
  DIE_NOW(context, "nopInstruction: should not trap");
}

u32int svcInstruction(GCONTXT *context, u32int instruction)
{
  TRACE(context, instruction);
  DIE_NOW(context, "svcInstruction: should not invoke interpreter");
}

u32int undefinedInstruction(GCONTXT *context, u32int instruction)
{
  TRACE(context, instruction);
  DIE_NOW(context, "undefined instruction");
}
