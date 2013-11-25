#ifndef __INSTRUCTION_EMU__INTERPRETER__COMMON_H__
#define __INSTRUCTION_EMU__INTERPRETER__COMMON_H__

#include "common/types.h"

#include "instructionEmu/interpreter/internals.h"

#include "guestManager/guestContext.h"

bool BadMode(CPSRmode mode);

bool BigEndian(GCONTXT* context);

void BranchWritePC(GCONTXT* context, u32int address);

void BXWritePC(GCONTXT* context, u32int target);

void ClearExclusiveLocal(void);

bool ConditionPassed(ConditionCode instructionCode);

void CPSRWriteByInstr(GCONTXT* context, CPSRreg val, u8int bytemask, bool is_exc_ret);

InstructionSet CurrentInstrSet(void);

bool CurrentModeIsNotUser(GCONTXT* context);

bool CurrentModeIsUserOrSystem(GCONTXT* context);

bool ExclusiveMonitorsPass(u32int address, ACCESS_SIZE size);

bool HaveVirtExt(void);

void SelectInstrSet(InstructionSet iset);

void SetExclusiveMonitors(u32int address, ACCESS_SIZE size);

CPSRreg SPSR(GCONTXT* context);

void UNPREDICTABLE(void);

/************** inlines ******************************/
__macro__ bool BadMode(CPSRmode mode)
{
  switch (mode)
  {
    case USR_MODE:
    case FIQ_MODE:
    case IRQ_MODE:
    case SVC_MODE:
    case MON_MODE:
    case ABT_MODE:
    case UND_MODE:
    case SYS_MODE:
    {
      return FALSE;
    }
    case HYP_MODE:
    {
      return !HaveVirtExt();
    }
    default:
      return TRUE;
  }
}


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


__macro__ bool CurrentModeIsNotUser(GCONTXT* context)
{
  // this returns TRUE without checking for now: we dont scan user mode code
  return TRUE;
}


__macro__ bool CurrentModeIsUserOrSystem(GCONTXT* context)
{
  return (context->CPSR.bits.mode == USR_MODE) ||
         (context->CPSR.bits.mode == SYS_MODE);
}


__macro__ bool ExclusiveMonitorsPass(u32int address, ACCESS_SIZE size)
{
  // STARFIX: implement exclusive monitors. should check the flag for given
  // address here.
  return TRUE;
}


__macro__ bool HaveVirtExt()
{
  return FALSE;
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


__macro__ CPSRreg SPSR(GCONTXT* context)
{
  switch (context->CPSR.bits.mode)
  {
    case USR_MODE: DIE_NOW(context, "SPSR: read in user mode");
    case SYS_MODE: DIE_NOW(context, "SPSR: read in system mode");
    case FIQ_MODE: return context->SPSR_FIQ;
    case IRQ_MODE: return context->SPSR_IRQ;
    case SVC_MODE: return context->SPSR_SVC;
    case MON_MODE: DIE_NOW(context, "SPSR mon unimplemented");
    case ABT_MODE: return context->SPSR_ABT;
    case HYP_MODE: DIE_NOW(context, "SPSR hyp unimplemented");
    case UND_MODE: return context->SPSR_UND;
    default:
      DIE_NOW(context, "SPSR: invalid mode\n");
  }
}

__macro__ void UNPREDICTABLE()
{
  // STARFIX: this should be a config switch making UNPREDICTABLE() die or
  // do nothing
}

#endif /* __INSTRUCTION_EMU__INTERPRETER__COMMON_H__ */
