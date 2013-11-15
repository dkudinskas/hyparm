#ifndef __INSTRUCTION_EMU__INTERPRETER__COMMON_H__
#define __INSTRUCTION_EMU__INTERPRETER__COMMON_H__

#include "common/types.h"

#include "instructionEmu/interpreter/internals.h"

#include "guestManager/guestContext.h"

bool BigEndian(GCONTXT* context);

void BranchWritePC(GCONTXT* context, u32int address);

void BXWritePC(GCONTXT* context, u32int target);

void ClearExclusiveLocal(void);

bool ConditionPassed(ConditionCode instructionCode);

InstructionSet CurrentInstrSet(void);

bool ExclusiveMonitorsPass(u32int address, ACCESS_SIZE size);

void SelectInstrSet(InstructionSet iset);

void SetExclusiveMonitors(u32int address, ACCESS_SIZE size);

void UNPREDICTABLE(void);

/************** inlines ******************************/
__macro__ bool BigEndian(GCONTXT* context)
{
  // STARFIX: This should be a dynamic check:
  // return (ENDIANSTATE == '1');
  // but we don't support big endian yet really. didnt see it used anywhere
  return FALSE;
}

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


__macro__ void ClearExclusiveLocal()
{
  // STARFIX: implement exclusive monitors. should check the flags here
}


__macro__ bool ConditionPassed(ConditionCode instructionCode)
{
  if ((instructionCode == AL) || (instructionCode == NV))
    return TRUE;
  else
  {
    return evaluateConditionCode(getActiveGuestContext(), instructionCode);
  }
}


__macro__ InstructionSet CurrentInstrSet()
{
  // STARFIX: just return ARM for now. others not supported
  return InstrSet_ARM;
}


__macro__ bool ExclusiveMonitorsPass(u32int address, ACCESS_SIZE size)
{
  // STARFIX: implement exclusive monitors. should check the flag for given
  // address here.
  return TRUE;
}


__macro__ void SelectInstrSet(InstructionSet iset)
{
  // STARFIX: other isets not supported yet
  // do nothing
}


__macro__ void SetExclusiveMonitors(u32int address, ACCESS_SIZE size)
{
  // STARFIX: implement exclusive monitors. This should be a flag set per
  // given address.
}


__macro__ void UNPREDICTABLE()
{
  // STARFIX: this should be a config switch making UNPREDICTABLE() die or
  // do nothing
}

#endif /* __INSTRUCTION_EMU__INTERPRETER__COMMON_H__ */
