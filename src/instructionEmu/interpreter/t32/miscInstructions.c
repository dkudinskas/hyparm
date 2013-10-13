#include "common/bit.h"

#include "cpuArch/constants.h"

#include "instructionEmu/interpreter/internals.h"

#include "instructionEmu/interpreter/t32/miscInstructions.h"


u32int t32MrsInstruction(GCONTXT *context, u32int instruction)
{
  u32int readSpsr = instruction & 0x00100000;
  u32int reg = (instruction & 0x0F00) >> 8;

  u32int value = 0;

  if (context->CPSR.bits.mode == USR_MODE)
  {
    ASSERT(!readSpsr, "SPSR read bit can not be set in USR mode");
    value = context->CPSR.value & PSR_APSR;
  }
  else
  {
    if (readSpsr)
    {
      switch (context->CPSR.bits.mode)
      {
        case FIQ_MODE:
          value = context->SPSR_FIQ;
          break;
        case IRQ_MODE:
          value = context->SPSR_IRQ;
          break;
        case SVC_MODE:
          value = context->SPSR_SVC;
          break;
        case ABT_MODE:
          value = context->SPSR_ABT;
          break;
        case UND_MODE:
          value = context->SPSR_UND;
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

  setGPRegister(context, reg, value);

  return context->R15 + T32_INSTRUCTION_SIZE;
}
