#include "guestManager/scheduler.h"

#include "instructionEmu/decoder/arm/structs.h"
#include "instructionEmu/interpreter/common.h"
#include "instructionEmu/interpreter/internals.h"
#include "instructionEmu/interpreter/arm/miscInstructions.h"

#include "vm/omap35xx/intc.h"

u32int armBkptInstruction(GCONTXT *context, u32int instruction)
{
  if (unlikely(context->os == GUEST_OS_TEST))
  {
    u32int imm4 = instruction & 0x0000000F;
    u32int imm12 = (instruction & 0x000FFF00) >> 4;
    u32int val = imm12 | imm4;

    evaluateBreakpointValue(context, val);
    return context->R15 + ARM_INSTRUCTION_SIZE;
  }
  DIE_NOW(context, ERROR_NOT_IMPLEMENTED);
}

u32int armClzInstruction(GCONTXT *context, u32int instruction)
{
  DIE_NOW(context, ERROR_NOT_IMPLEMENTED);
}

u32int armCpsInstruction(GCONTXT *context, u32int instruction)
{
  Instruction instr = {.raw = instruction};
  DEBUG_TRACE(INTERPRETER_ARM_SYSTEM, context, instruction);

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

u32int armDbgInstruction(GCONTXT *context, u32int instruction)
{
  DIE_NOW(context, ERROR_NOT_IMPLEMENTED);
}

u32int armDmbInstruction(GCONTXT *context, u32int instruction)
{
#ifdef ARM_INSTR_TRACE
  /* DMB is weaker than DSB */
  printf("Warning: DMB (ignored)!" EOL);
#endif
  return context->R15 + ARM_INSTRUCTION_SIZE;
}

u32int armDsbInstruction(GCONTXT *context, u32int instruction)
{
#ifdef ARM_INSTR_TRACE
  printf("Warning: DSB (ignored)!" EOL);
#endif
  return context->R15 + ARM_INSTRUCTION_SIZE;
}

u32int armIsbInstruction(GCONTXT *context, u32int instruction)
{
#ifdef ARM_INSTR_TRACE
  printf("Warning: ISB (ignored)!" EOL);
#endif
  return context->R15 + ARM_INSTRUCTION_SIZE;
}


u32int armMrsInstruction(GCONTXT *context, u32int instruction)
{
  Instruction instr = {.raw = instruction};
  DEBUG_TRACE(INTERPRETER_ARM_SYSTEM, context, instruction);

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


u32int armMsrRegInstruction(GCONTXT *context, u32int instruction)
{
  Instruction instr = {.raw = instruction};
  DEBUG_TRACE(INTERPRETER_ARM_SYSTEM, context, instruction);

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
      if ((value & PSR_MODE) != context->CPSR.bits.mode)
      {
        // changing modes; adjust context appropriatelly
        guestChangeMode(context, value & PSR_MODE);
      }
      // Does not affect execution state bits other than E
      CPSRWriteByInstr(context, value, instr.msrReg.mask, FALSE);
      // we dont support hypervisor mode yet
      // if ((context->CPSR.mode == HYP_MODE) && (context->CPSR.bits.J) && (context->CPSR.bits.T))
      //   UNPREDICTABLE();
    }
  }
  return context->R15 + ARM_INSTRUCTION_SIZE;
}


u32int armMsrImmInstruction(GCONTXT *context, u32int instruction)
{
  Instruction instr = {.raw = instruction};
  DEBUG_TRACE(INTERPRETER_ARM_SYSTEM, context, instruction);

  if (ConditionPassed(instr.msrReg.cc))
  {
    if ((instr.msrImm.mask == 0) && (instr.msrImm.R == 0))
    {
      printf("msr: see related encodings\n");
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
      if ((imm32 & PSR_MODE) != context->CPSR.bits.mode)
      {
        // changing modes; adjust context appropriatelly
        guestChangeMode(context, imm32 & PSR_MODE);
      }
      // Does not affect execution state bits other than E
      CPSRWriteByInstr(context, imm32, instr.msrImm.mask, FALSE);
      // we dont support hypervisor mode yet
      // if ((context->CPSR.mode == HYP_MODE) && (context->CPSR.bits.J) && (context->CPSR.bits.T))
      //   UNPREDICTABLE();
    }
  }
  return context->R15 + ARM_INSTRUCTION_SIZE;
}


u32int armPldInstruction(GCONTXT *context, u32int instruction)
{
#ifdef ARM_INSTR_TRACE
  printf("Warning: PLD!" EOL);
#endif
  return context->R15 + ARM_INSTRUCTION_SIZE;
}

u32int armPliInstruction(GCONTXT *context, u32int instruction)
{
#ifdef ARM_INSTR_TRACE
  printf("Warning: PLI!" EOL);
#endif
  return context->R15 + ARM_INSTRUCTION_SIZE;
}

u32int armRfeInstruction(GCONTXT *context, u32int instruction)
{
  DIE_NOW(context, ERROR_NOT_IMPLEMENTED);
}

u32int armSetendInstruction(GCONTXT *context, u32int instruction)
{
  DIE_NOW(context, ERROR_NOT_IMPLEMENTED);
}

u32int armSevInstruction(GCONTXT *context, u32int instruction)
{
  DIE_NOW(context, ERROR_NOT_IMPLEMENTED);
}

u32int armSmcInstruction(GCONTXT *context, u32int instruction)
{
  DIE_NOW(context, ERROR_NOT_IMPLEMENTED);
}

u32int armSrsInstruction(GCONTXT *context, u32int instruction)
{
  DIE_NOW(context, ERROR_NOT_IMPLEMENTED);
}

u32int armWfeInstruction(GCONTXT *context, u32int instruction)
{
  DIE_NOW(context, ERROR_NOT_IMPLEMENTED);
}

u32int armWfiInstruction(GCONTXT *context, u32int instruction)
{
  // stop guest execution...
  guestIdle(context);
  return context->R15 + ARM_INSTRUCTION_SIZE;
}

u32int armYieldInstruction(GCONTXT *context, u32int instruction)
{
  DIE_NOW(context, ERROR_NOT_IMPLEMENTED);
}


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
