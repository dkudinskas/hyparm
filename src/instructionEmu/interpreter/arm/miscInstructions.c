#include "guestManager/scheduler.h"

#include "instructionEmu/interpreter/internals.h"

#include "instructionEmu/interpreter/arm/miscInstructions.h"

#include "vm/omap35xx/intc.h"

u32int armBkptInstruction(GCONTXT *context, u32int instruction)
{
#ifdef CONFIG_GUEST_TEST
  u32int imm4 = instruction & 0x0000000F;
  u32int imm12 = (instruction & 0x000FFF00) >> 4;
  u32int val = imm12 | imm4;

  evalBkptVal(context, val);
  return context->R15 + ARM_INSTRUCTION_SIZE;
#else
  DIE_NOW(context, ERROR_NOT_IMPLEMENTED);
#endif
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

  if ( ((imod == 0) && (changeMode == 0)) || (imod == 1) )
  {
    DIE_NOW(context, "unpredictable");
  }

  if ((context->CPSR & PSR_MODE) != PSR_USR_MODE)
  {
    u32int oldCpsr = context->CPSR;
    if (imod == 0x2) // enable
    {
#ifdef ARM_INSTR_TRACE
      printf("IMod: enable case" EOL);
#endif
      if (affectA != 0)
      {
        if ((oldCpsr & PSR_A_BIT) != 0)
        {
          DIE_NOW(context, "Guest enabling async aborts globally!");
        }
        oldCpsr &= ~PSR_A_BIT;
      }
      if (affectI)
      {
        if ((oldCpsr & PSR_I_BIT) != 0)
        {
#ifdef ARM_INSTR_TRACE
          printf("Guest enabling irqs globally!" EOL);
#endif
          // chech interrupt controller if there is an interrupt pending
          if (isIrqPending())
          {
            context->guestIrqPending = TRUE;
          }
        }
        oldCpsr &= ~PSR_I_BIT;
      }
      if (affectF)
      {
        if ((oldCpsr & PSR_F_BIT) != 0)
        {
#ifdef ARM_INSTR_TRACE
          printf("Guest enabling FIQs globally!" EOL);
#endif
          // chech interrupt controller if there is an interrupt pending
          if (isFiqPending())
          {
            // context->guestFiqPending = TRUE; : IMPLEMENT!!
            DIE_NOW(context, ERROR_NOT_IMPLEMENTED);
          }
        }
        oldCpsr &= ~PSR_F_BIT;
      }
    }
    else if (imod == 3) // disable
    {
      if (affectA)
      {
        if (!(oldCpsr & PSR_A_BIT))
        {
          DIE_NOW(context, "Guest disabling async aborts globally!");
        }
        oldCpsr |= PSR_A_BIT;
      }
      if (affectI)
      {
        if (!(oldCpsr & PSR_I_BIT)) // were enabled, now disabled
        {
          // chech interrupt controller if there is an interrupt pending
          if (context->guestIrqPending)
          {
            /*
             * FIXME: Niels: wtf? why do we need the if?
             */
            context->guestIrqPending = FALSE;
          }
        }
        oldCpsr |= PSR_I_BIT;
      }
      if (affectF)
      {
        if (!(oldCpsr & PSR_F_BIT))
        {
          DIE_NOW(context, "Guest disabling fiqs globally!");
        }
        oldCpsr |= PSR_F_BIT;
      }
    }
    else
    {
      DIE_NOW(context, "CPS invalid IMOD");
    }
    // ARE we switching modes?
    if (changeMode)
    {
      oldCpsr &= ~PSR_MODE;
      oldCpsr |= newMode;
      DIE_NOW(context, "guest is changing execution modes. To What?");
    }
    context->CPSR = oldCpsr;
  }
  else
  {
    // guest is not in privileged mode! cps should behave as a nop, but lets see what went wrong.
    DIE_NOW(context, "CPS instruction: executed in guest user mode.");
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
  int readSpsr =  instruction & 0x00400000;
  int regDest  = (instruction & 0x0000F000) >> 12;

#ifdef ARM_INSTR_TRACE
  printf("MRS instr %08x @ %08x" EOL, instruction, context->R15);
#endif

  if (regDest == 0xF)
  {
    DIE_NOW(context, "mrsInstruction: cannot use PC as destination");
  }

  if (evaluateConditionCode(context, ARM_EXTRACT_CONDITION_CODE(instruction)))
  {
    u32int value;

    if ((context->CPSR & PSR_MODE) == PSR_USR_MODE)
    {
      if (readSpsr)
      {
        DIE_NOW(context, "SPSR read bit can not be set in USR mode");
      }
      value = context->CPSR & PSR_APSR;
    }
    else
    {
      if (readSpsr)
      {
        switch(context->CPSR & PSR_MODE)
        {
          case PSR_FIQ_MODE:
            value = context->SPSR_FIQ;
            break;
          case PSR_IRQ_MODE:
            value = context->SPSR_IRQ;
            break;
          case PSR_SVC_MODE:
            value = context->SPSR_SVC;
            break;
          case PSR_ABT_MODE:
            value = context->SPSR_ABT;
            break;
          case PSR_UND_MODE:
            value = context->SPSR_UND;
            break;
          case PSR_USR_MODE:
          case PSR_SYS_MODE:
          default:
            DIE_NOW(context, "mrsInstruction: cannot request spsr in user/system mode");
        }
      }
      else
      {
        // CPSR is read with execution state bits other than E masked out
        value = context->CPSR & ~PSR_EXEC_BITS;
      }
    }
    storeGuestGPR(regDest, value, context);
  } // condition met ends

  return context->R15 + ARM_INSTRUCTION_SIZE;
}

u32int armMsrInstruction(GCONTXT *context, u32int instruction)
{
  u32int instrCC =    (instruction & 0xF0000000) >> 28;
  u32int regOrImm =   (instruction & 0x02000000); // if 1 then imm12, 0 then Reg
  u32int cpsrOrSpsr = (instruction & 0x00400000); // if 0 then cpsr, !0 then spsr
  u32int fieldMsk =   (instruction & 0x000F0000) >> 16;

  u32int value = 0;
  u32int nextPC = 0;

  if (!evaluateConditionCode(context, instrCC))
  {
    nextPC = context->R15 + 4;
    return nextPC;
  }

  if (regOrImm == 0)
  {
    // register case
    u32int regSrc = instruction & 0x0000000F;
    if (regSrc == 0xF)
    {
      DIE_NOW(context, "msrInstruction: cannot use PC as source register");
    }
    value = loadGuestGPR(regSrc, context);
  }
  else
  {
    // immediate case
    u32int immediate = instruction & 0x00000FFF;
    value = armExpandImm12(immediate);
  }

  u32int oldValue = 0;
  if (cpsrOrSpsr == 0)
  {
    // CPSR!
    oldValue = context->CPSR;
  }
  else
  {
    // SPSR! which?... depends what mode we are in...
    switch (context->CPSR & PSR_MODE)
    {
      case PSR_FIQ_MODE:
        oldValue = context->SPSR_FIQ;
        break;
      case PSR_IRQ_MODE:
        oldValue = context->SPSR_IRQ;
        break;
      case PSR_SVC_MODE:
        oldValue = context->SPSR_SVC;
        break;
      case PSR_ABT_MODE:
        oldValue = context->SPSR_ABT;
        break;
      case PSR_UND_MODE:
        oldValue = context->SPSR_UND;
        break;
      default:
        DIE_NOW(context, "MSR: invalid SPSR write for current guest mode.");
    }
  }

  // [3:0] field mask:
  // - bit 0: set control field (mode bits/interrupt bits)
  // - bit 1: set extension field (??? [15:8] of cpsr)
  // - bit 2: set status field (??? [23:16] of cpsr)
  // - bit 3: set condition flags of cpsr

  // control field [7-0] set.
  if (((fieldMsk & 0x1) == 0x1) && (context->CPSR & PSR_MODE) != PSR_USR_MODE)
  {
#ifndef CONFIG_THUMB2
    // check for thumb toggle!
    if ((oldValue & PSR_T_BIT) != (value & PSR_T_BIT))
    {
      DIE_NOW(context, "MSR toggle THUMB bit.");
    }
#endif

    // separate the field we're gonna update from new value
    u32int appliedValue = (value & 0x000000FF);
    // clear old fields!
    oldValue &= 0xFFFFFF00;
    // update old value...
    oldValue |= appliedValue;
  }
  if ( ((fieldMsk & 0x2) == 0x2) && (context->CPSR & PSR_MODE) != PSR_USR_MODE)
  {
    // extension field: async abt, endianness, IT[7:2]
    // check for endiannes toggle!
    if ((oldValue & PSR_E_BIT) != (value & PSR_E_BIT))
    {
      DIE_NOW(context, "MSR toggle endianess bit.");
    }
    // separate the field we're gonna update from new value
    u32int appliedValue = (value & 0x0000FF00);
    // clear old fields!
    oldValue &= 0xFFFF00FF;
    // update old value...
    oldValue |= appliedValue;
  }
  if ( ((fieldMsk & 0x4) == 0x4) && (context->CPSR & PSR_MODE) != PSR_USR_MODE)
  {
    // status field: reserved and GE[3:0]
    // separate the field we're gonna update from new value
    u32int appliedValue = (value & 0x00FF0000);
    // clear old fields!
    oldValue &= 0xFF00FFFF;
    // update old value...
    oldValue |= appliedValue;
  }
  if ((fieldMsk & 0x8) == 0x8)
  {
    // condition flags, q, it, J. Dont need to be priv to change those thus no check
    // separate the field we're gonna update from new value
    u32int appliedValue = (value & 0xFF000000);
    // clear old fields!
    oldValue &= 0x00FFFFFF;
    // update old value...
    oldValue |= appliedValue;
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
    switch (context->CPSR & PSR_MODE)
    {
      case PSR_FIQ_MODE:
        context->SPSR_FIQ = oldValue;
        break;
      case PSR_IRQ_MODE:
        context->SPSR_IRQ = oldValue;
        break;
      case PSR_SVC_MODE:
        context->SPSR_SVC = oldValue;
        break;
      case PSR_ABT_MODE:
        context->SPSR_ABT = oldValue;
        break;
      case PSR_UND_MODE:
        context->SPSR_UND = oldValue;
        break;
      default:
        DIE_NOW(context, "MSR: invalid SPSR write for current guest mode.");
    }
  }

  nextPC = context->R15 + 4;
  return nextPC;
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
