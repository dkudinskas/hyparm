#ifndef __INSTRUCTION_EMU__INTERPRETER__COMMON_H__
#define __INSTRUCTION_EMU__INTERPRETER__COMMON_H__

#include "common/types.h"

#include "guestManager/guestContext.h"

void BranchWritePC(GCONTXT* context, u32int address);

InstructionSet CurrentInstrSet(void);

void SelectInstrSet(InstructionSet iset);


/************** inlines ******************************/
__macro__ void BranchWritePC(GCONTXT* context, u32int address)
{
  if (CurrentInstrSet() == InstrSet_ARM)
  {
    context->R15 = (address & ~0x3);
  }
  else
  {
    DIE_NOW(context, "BranchWritePC: other instruction sets unimplemented");
  }
}


__macro__ InstructionSet CurrentInstrSet()
{
  // STARFIX: just return ARM for now. others not supported
  return InstrSet_ARM;
}

__macro__ void SelectInstrSet(InstructionSet iset)
{
  // STARFIX: other isets not supported yet
  // do nothing
}

#endif /* __INSTRUCTION_EMU__INTERPRETER__COMMON_H__ */
