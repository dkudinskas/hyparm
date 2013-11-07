#include "guestManager/scheduler.h"

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
  u32int imod       = (instruction & 0x000C0000) >> 18;
  u32int changeMode = (instruction & 0x00020000) >> 17;
  u32int affectA    = (instruction & 0x00000100) >>  8;
  u32int affectI    = (instruction & 0x00000080) >>  7;
  u32int affectF    = (instruction & 0x00000040) >>  6;
  u32int newMode    =  instruction & 0x0000001F;
#ifdef ARM_INSTR_TRACE
  printf("CPS instr %08x @ %08x" EOL, instruction, context->R15);
#endif

  ASSERT(imod != 0 || changeMode != 0, ERROR_UNPREDICTABLE_INSTRUCTION);
  ASSERT(imod != 1, ERROR_UNPREDICTABLE_INSTRUCTION);

  // guest is not in privileged mode! cps should behave as a nop, but lets see what went wrong.
  ASSERT(context->CPSR.bits.mode != USR_MODE, "CPS trapped in guest user mode");

  CPSRreg oldCpsr = context->CPSR;
  if (imod == 0x2) // enable
  {
#ifdef ARM_INSTR_TRACE
    printf("IMod: enable case" EOL);
#endif
    if (affectA != 0)
    {
      ASSERT(oldCpsr.bits.A == 0, "Guest enabling async aborts globally!");
      oldCpsr.bits.A = 0;
    }
    if (affectI)
    {
      if (oldCpsr.bits.I)
      {
#ifdef ARM_INSTR_TRACE
        printf("Guest enabling irqs globally!" EOL);
#endif
#ifndef CONFIG_HW_PASSTHROUGH
        // check interrupt controller if there is an interrupt pending
        if (isIrqPending(context->vm.irqController))
        {
          context->guestIrqPending = TRUE;
        }
#endif
      }
      oldCpsr.bits.I = 0;;
    }
    if (affectF)
    {
      if (oldCpsr.bits.I)
      {
#ifdef ARM_INSTR_TRACE
        printf("Guest enabling FIQs globally!" EOL);
#endif
#ifndef CONFIG_HW_PASSTHROUGH
        // chech interrupt controller if there is an interrupt pending
        if (isFiqPending(context->vm.irqController))
        {
          // context->guestFiqPending = TRUE; : IMPLEMENT!!
          DIE_NOW(context, ERROR_NOT_IMPLEMENTED);
        }
#endif
      }
      oldCpsr.bits.F = 0;
    }
  }
  else if (imod == 3) // disable
  {
    if (affectA)
    {
      ASSERT(oldCpsr.bits.A, "Guest disabling async aborts globally!");
      oldCpsr.bits.A = 1;
    }
    if (affectI)
    {
      oldCpsr.bits.I = 1;
    }
    if (affectF)
    {
      oldCpsr.bits.F = 1;
    }
  }
  else
  {
    DIE_NOW(context, "CPS invalid IMOD");
  }
  // ARE we switching modes?
  if (unlikely(changeMode))
  {
    oldCpsr.bits.mode = newMode;
    DIE_NOW(context, "guest is changing execution modes. To What?");
  }
  context->CPSR = oldCpsr;

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
  int readSpsr =  instruction & 0x00400000;
  int regDest  = (instruction & 0x0000F000) >> 12;

#ifdef ARM_INSTR_TRACE
  printf("MRS instr %08x @ %08x" EOL, instruction, context->R15);
#endif

  ASSERT(regDest != GPR_PC, ERROR_UNPREDICTABLE_INSTRUCTION);

  if (evaluateConditionCode(context, ARM_EXTRACT_CONDITION_CODE(instruction)))
  {
    u32int value;

    if (context->CPSR.bits.mode ==USR_MODE)
    {
      //SPSR read bit can not be set in USR mode
      ASSERT(!readSpsr, ERROR_UNPREDICTABLE_INSTRUCTION);
      value = context->CPSR.value & PSR_APSR;
    }
    else
    {
      if (readSpsr)
      {
        switch(context->CPSR.bits.mode)
        {
          case FIQ_MODE:
            value = context->SPSR_FIQ.value;
            break;
          case IRQ_MODE:
            value = context->SPSR_IRQ.value;
            break;
          case SVC_MODE:
            value = context->SPSR_SVC.value;
            break;
          case ABT_MODE:
            value = context->SPSR_ABT.value;
            break;
          case UND_MODE:
            value = context->SPSR_UND.value;
            break;
          case USR_MODE:
          case SYS_MODE:
          default:
            DIE_NOW(context, "cannot request spsr in user/system mode");
        }
      }
      else
      {
        // CPSR is read with execution state bits other than E masked out
        value = context->CPSR.value & ~PSR_EXEC_BITS;
      }
    }
    setGPRegister(context, regDest, value);
  } // condition met ends

  return context->R15 + ARM_INSTRUCTION_SIZE;
}

u32int armMsrInstruction(GCONTXT *context, u32int instruction)
{
  u32int regOrImm =   (instruction & 0x02000000); // if 1 then imm12, 0 then Reg
  u32int cpsrOrSpsr = (instruction & 0x00400000); // if 0 then cpsr, !0 then spsr
  u32int fieldMsk =   (instruction & 0x000F0000) >> 16;

  u32int value = 0;
  
  if (!evaluateConditionCode(context, ARM_EXTRACT_CONDITION_CODE(instruction)))
  {
    return context->R15 + ARM_INSTRUCTION_SIZE;
  }

  if (regOrImm == 0)
  {
    // register case
    u32int regSrc = instruction & 0x0000000F;
    ASSERT(regSrc != GPR_PC, ERROR_UNPREDICTABLE_INSTRUCTION);
    value = getGPRegister(context, regSrc);
  }
  else
  {
    // immediate case
    u32int immediate = instruction & 0x00000FFF;
    value = armExpandImm12(immediate);
  }

  CPSRreg oldValue = {.value = 0};
  if (cpsrOrSpsr == 0)
  {
    // CPSR!
    oldValue = context->CPSR;
  }
  else
  {
    // SPSR! which?... depends what mode we are in...
    switch (context->CPSR.bits.mode)
    {
      case FIQ_MODE:
        oldValue.value = context->SPSR_FIQ.value;
        break;
      case IRQ_MODE:
        oldValue.value = context->SPSR_IRQ.value;
        break;
      case SVC_MODE:
        oldValue.value = context->SPSR_SVC.value;
        break;
      case ABT_MODE:
        oldValue.value = context->SPSR_ABT.value;
        break;
      case UND_MODE:
        oldValue.value = context->SPSR_UND.value;
        break;
      default:
        DIE_NOW(context, "invalid SPSR write for current guest mode");
    }
  }

  // [3:0] field mask:
  // - bit 0: set control field (mode bits/interrupt bits)
  // - bit 1: set extension field (??? [15:8] of cpsr)
  // - bit 2: set status field (??? [23:16] of cpsr)
  // - bit 3: set condition flags of cpsr

  // control field [7-0] set.
  if (((fieldMsk & 0x1) == 0x1) && (context->CPSR.bits.mode != USR_MODE))
  {
#ifndef CONFIG_THUMB2
    // check for thumb toggle!
    ASSERT(oldValue.bits.T == (value & PSR_T_BIT), "MSR toggle THUMB bit");
#endif

    if ((value & PSR_MODE) != oldValue.bits.mode)
    {
      // changing modes. if in CPSR, adjust context appropriatelly
      if (cpsrOrSpsr == 0)
      {
        guestChangeMode(context, value & PSR_MODE);
      }
    }
    // separate the field we're gonna update from new value
    u32int appliedValue = (value & 0x000000FF);
    // clear old fields!
    oldValue.value &= 0xFFFFFF00;
    // update old value...
    oldValue.value |= appliedValue;
  }
  if ( ((fieldMsk & 0x2) == 0x2) && (context->CPSR.bits.mode != USR_MODE))
  {
    // extension field: async abt, endianness, IT[7:2]
    // check for endiannes toggle!
    ASSERT(oldValue.bits.E == (value & PSR_E_BIT), "MSR toggle endianess bit");
    // separate the field we're gonna update from new value
    u32int appliedValue = (value & 0x0000FF00);
    // clear old fields!
    oldValue.value &= 0xFFFF00FF;
    // update old value...
    oldValue.value |= appliedValue;
  }
  if ( ((fieldMsk & 0x4) == 0x4) && (context->CPSR.bits.mode != USR_MODE))
  {
    // status field: reserved and GE[3:0]
    // separate the field we're gonna update from new value
    u32int appliedValue = (value & 0x00FF0000);
    // clear old fields!
    oldValue.value &= 0xFF00FFFF;
    // update old value...
    oldValue.value |= appliedValue;
  }
  if ((fieldMsk & 0x8) == 0x8)
  {
    // condition flags, q, it, J. Dont need to be priv to change those thus no check
    // separate the field we're gonna update from new value
    u32int appliedValue = (value & 0xFF000000);
    // clear old fields!
    oldValue.value &= 0x00FFFFFF;
    // update old value...
    oldValue.value |= appliedValue;
  }

#ifdef ARM_INSTR_TRACE
  printf("MSR instr %08x @ %08x" EOL, instruction, context->R15);
#endif
  // got the final value to write in u32int oldValue. where do we write it thou..?
  if (cpsrOrSpsr == 0)
  {
    // CPSR!
    context->CPSR = oldValue;
  }
  else
  {
    // SPSR! which?... depends what mode we are in...
    switch (context->CPSR.bits.mode)
    {
      case FIQ_MODE:
        context->SPSR_FIQ.value = oldValue.value;
        break;
      case IRQ_MODE:
        context->SPSR_IRQ.value = oldValue.value;
        break;
      case SVC_MODE:
        context->SPSR_SVC.value = oldValue.value;
        break;
      case ABT_MODE:
        context->SPSR_ABT.value = oldValue.value;
        break;
      case UND_MODE:
        context->SPSR_UND.value = oldValue.value;
        break;
      default:
        DIE_NOW(context, "MSR: invalid SPSR write for current guest mode.");
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
