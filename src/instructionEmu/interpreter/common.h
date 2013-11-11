#ifndef __INSTRUCTION_EMU__INTERPRETER__COMMON_H__
#define __INSTRUCTION_EMU__INTERPRETER__COMMON_H__

#include "common/types.h"

#include "guestManager/guestContext.h"

void BranchWritePC(GCONTXT* context, u32int address);

void BXWritePC(GCONTXT* context, u32int target);

InstructionSet CurrentInstrSet(void);

void SelectInstrSet(InstructionSet iset);

void UNPREDICTABLE(void);

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


__macro__ void BXWritePC(GCONTXT* context, u32int address)
{
  if (CurrentInstrSet() == InstrSet_ThumbEE)
  {
    if ((address & 1) == 1)
      context->R15 = (address & ~0x1); // Remaining in ThumbEE state
    else
      UNPREDICTABLE();
  }
  else
  {
    if ((address & 1) == 1)
    {
      SelectInstrSet(InstrSet_Thumb);
      context->R15 = (address & ~0x1);
    }
    else if ((address & 1) == 1)
    {
      SelectInstrSet(InstrSet_ARM);
      context->R15 = address;
    }
    else
      UNPREDICTABLE();
  }


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


__macro__ void UNPREDICTABLE()
{
  // STARFIX: this should be a config switch making UNPREDICTABLE() die or
  // do nothing
}

#endif /* __INSTRUCTION_EMU__INTERPRETER__COMMON_H__ */
