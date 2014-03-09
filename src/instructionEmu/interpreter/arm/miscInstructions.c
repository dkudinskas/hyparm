#include "guestManager/scheduler.h"

#include "instructionEmu/interpreter/common.h"
#include "instructionEmu/interpreter/internals.h"
#include "instructionEmu/interpreter/arm/miscInstructions.h"

#include "perf/contextSwitchCounters.h"

#include "vm/omap35xx/intc.h"

u32int armBkptInstruction(GCONTXT *context, Instruction instr)
{
  if (unlikely(context->os == GUEST_OS_TEST))
  {
    evaluateBreakpointValue(context, instr.bkpt.imm12 << 4 | instr.bkpt.imm4);
    return context->R15 + ARM_INSTRUCTION_SIZE;
  }
  DIE_NOW(context, ERROR_NOT_IMPLEMENTED);
}

u32int armClzInstruction(GCONTXT *context, Instruction instr)
{
  DIE_NOW(context, ERROR_NOT_IMPLEMENTED);
}

u32int armCpsInstruction(GCONTXT *context, Instruction instr)
{
  DEBUG_TRACE(INTERPRETER_ARM_SYSTEM, context, instr.raw);
  countCps(&(context->counters), instr);
  if ((instr.cps.mode != 0) && (instr.cps.M == 0))
    UNPREDICTABLE();
  if (((instr.cps.imod & 2) == 2) && (instr.cps.A == 0) && 
     (instr.cps.I == 0) && (instr.cps.F == 0))
    UNPREDICTABLE();
  if (((instr.cps.imod & 2) == 0) && ((instr.cps.A != 0) || 
     (instr.cps.I != 0) || (instr.cps.F != 0)))
    UNPREDICTABLE();
  if (((instr.cps.imod == 0) && (instr.cps.M == 0)) || (instr.cps.imod == 01))
    UNPREDICTABLE();

  bool enable  = instr.cps.imod == 2;
  bool disable = instr.cps.imod == 3;
  bool changemode = instr.cps.M;
  bool affectA = instr.cps.A;
  bool affectI = instr.cps.I;
  bool affectF = instr.cps.F;

  // CurrentModeIsNotUser always returns TRUE atm: we dont scan user mode code
  if (CurrentModeIsNotUser(context))
  {
    CPSRreg cpsr_val = context->CPSR;
    if (enable)
    {
      if (affectA) cpsr_val.bits.A = 0;
      if (affectI) cpsr_val.bits.I = 0;
      if (affectF) cpsr_val.bits.F = 0;
    }
    if (disable)
    {
      if (affectA) cpsr_val.bits.A = 1;
      if (affectI) cpsr_val.bits.I = 1;
      if (affectF) cpsr_val.bits.F = 1;
    }
    if (changemode)
    {
      cpsr_val.bits.mode = instr.cps.mode;
    }

    CPSRWriteByInstr(context, cpsr_val.value, 0xF, FALSE);
  }
  return context->R15 + ARM_INSTRUCTION_SIZE;
}

u32int armDbgInstruction(GCONTXT *context, Instruction instr)
{
  DIE_NOW(context, ERROR_NOT_IMPLEMENTED);
}


u32int armMrsInstruction(GCONTXT *context, Instruction instr)
{
  DEBUG_TRACE(INTERPRETER_ARM_SYSTEM, context, instr.raw);
  countMrs(&(context->counters));
  if (ConditionPassed(instr.mrs.cc))
  {
    u8int Rd = instr.mrs.Rd;
    bool readSpsr = instr.mrs.R;

    if (Rd == GPR_PC)
      UNPREDICTABLE();

    if (readSpsr)
    {
      if (!CurrentModeIsUserOrSystem(context))
      {
        setGPRegister(context, Rd, SPSR(context).value);
      }
      else
      {
        UNPREDICTABLE();
      }
    }
    else
    {
      // CPSR is read with execution state bits other than E masked out.
      setGPRegister(context, Rd, (context->CPSR.value & 0xf8ff03df));
    }
  }
  return context->R15 + ARM_INSTRUCTION_SIZE;
}


u32int armMsrRegInstruction(GCONTXT *context, Instruction instr)
{
  DEBUG_TRACE(INTERPRETER_ARM_SYSTEM, context, instr.raw);
  countMsrReg(&(context->counters));
  if (ConditionPassed(instr.msrReg.cc))
  {
    u8int Rn = instr.msrReg.Rn;
    bool write_spsr = (instr.msrReg.R == 1);
    if (instr.msrReg.mask == 0)
      UNPREDICTABLE();
    if (Rn == GPR_PC)
      UNPREDICTABLE();

    u32int value = getGPRegister(context, Rn);
    if (write_spsr)
    {
      SPSRWriteByInstr(context, value, instr.msrReg.mask);
    }
    else
    {
      // Does not affect execution state bits other than E
      CPSRWriteByInstr(context, value, instr.msrReg.mask, FALSE);
      // we dont support hypervisor mode yet
      // if ((context->CPSR.mode == HYP_MODE) && (context->CPSR.bits.J) && (context->CPSR.bits.T))
      //   UNPREDICTABLE();
    }
  }
  return context->R15 + ARM_INSTRUCTION_SIZE;
}


u32int armMsrImmInstruction(GCONTXT *context, Instruction instr)
{
  DEBUG_TRACE(INTERPRETER_ARM_SYSTEM, context, instr.raw);
  countMsrImm(&(context->counters));
  if (ConditionPassed(instr.msrReg.cc))
  {
    if ((instr.msrImm.mask == 0) && (instr.msrImm.R == 0))
    {
      printf("msr: %08x @ %x; see related encodings\n", instr.raw, context->R15);
      DIE_NOW(context, "msr unimplemented.");
    }
    u32int imm32 = armExpandImm(instr.msrImm.imm12, context->CPSR.bits.C);
    bool write_spsr = instr.msrImm.R;
    if (instr.msrImm.mask == 0)
      UNPREDICTABLE();

    if (write_spsr)
    {
      SPSRWriteByInstr(context, imm32, instr.msrImm.mask);
    }
    else
    {
      // Does not affect execution state bits other than E
      CPSRWriteByInstr(context, imm32, instr.msrImm.mask, FALSE);
      // we dont support hypervisor mode yet
      // if ((context->CPSR.mode == HYP_MODE) && (context->CPSR.bits.J) && (context->CPSR.bits.T))
      //   UNPREDICTABLE();
    }
  }
  return context->R15 + ARM_INSTRUCTION_SIZE;
}

u32int armRfeInstruction(GCONTXT *context, Instruction instr)
{
  DIE_NOW(context, ERROR_NOT_IMPLEMENTED);
}

u32int armSetendInstruction(GCONTXT *context, Instruction instr)
{
  DIE_NOW(context, ERROR_NOT_IMPLEMENTED);
}

u32int armSevInstruction(GCONTXT *context, Instruction instr)
{
  DIE_NOW(context, ERROR_NOT_IMPLEMENTED);
}

u32int armSmcInstruction(GCONTXT *context, Instruction instr)
{
  DIE_NOW(context, ERROR_NOT_IMPLEMENTED);
}

u32int armSrsInstruction(GCONTXT *context, Instruction instr)
{
  DIE_NOW(context, ERROR_NOT_IMPLEMENTED);
}

u32int armWfeInstruction(GCONTXT *context, Instruction instr)
{
  DIE_NOW(context, ERROR_NOT_IMPLEMENTED);
}

u32int armWfiInstruction(GCONTXT *context, Instruction instr)
{
  // stop guest execution...
  countWfi(&(context->counters));
  guestIdle(context);
  return context->R15 + ARM_INSTRUCTION_SIZE;
}

u32int armYieldInstruction(GCONTXT *context, Instruction instr)
{
  DIE_NOW(context, ERROR_NOT_IMPLEMENTED);
}

u32int svcInstruction(GCONTXT *context, Instruction instr)
{
  TRACE(context, instr.raw);
  DIE_NOW(context, "svcInstruction: should not invoke interpreter");
}

u32int undefinedInstruction(GCONTXT *context, Instruction instr)
{
  TRACE(context, instr.raw);
  DIE_NOW(context, "undefined instruction");
}
