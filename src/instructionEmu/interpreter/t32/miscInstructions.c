#include "common/bit.h"

#include "cpuArch/constants.h"

#include "instructionEmu/interpreter/internals.h"

#include "instructionEmu/interpreter/t32/miscInstructions.h"

u32int t32MRSInstruction(GCONTXT *context, u32int instruction)
{
  u32int readSpsr = instruction & 0x00100000;
  u32int reg = (instruction & 0x0F00) >> 8;

  u32int value = 0;

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
      switch (context->CPSR & PSR_MODE)
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

  storeGuestGPR(reg, value, context);

  return context->R15 + T32_INSTRUCTION_SIZE;
}
