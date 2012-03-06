#include "common/bit.h"

#include "cpuArch/constants.h"

#include "instructionEmu/interpreter/internals.h"

#include "instructionEmu/interpreter/t32/miscInstructions.h"


u32int t32MrsInstruction(GCONTXT *context, u32int instruction)
{
  u32int reg = (instruction & 0x0F00) >> 8;
  if (context->CPSR & PSR_USR_MODE)
  {
    if (instruction & 0x00100000)
    {
      DIE_NOW(context, "SPSR read bit can not be set in USR mode");
    }
    u32int APSR = context->CPSR & (PSR_CC_FLAGS_NZCV | PSR_Q_BIT | PSR_SIMD_FLAGS_GE);
    storeGuestGPR(reg, APSR, context);
  }
  else
  {
    if (instruction & 0x00100000)
    {
      u32int value = 0;
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
          DIE_NOW(context, "cannot request spsr in user/system mode");
      }
      storeGuestGPR(reg, value, context);
    }
    else
    {
      u32int maskedCPSR = context->CPSR & ~(PSR_ITSTATE_1_0 | PSR_J_BIT | PSR_ITSTATE_7_2 | PSR_T_BIT);
      storeGuestGPR(reg, maskedCPSR, context);
    }
  }

  return context->R15 + T32_INSTRUCTION_SIZE;
}
