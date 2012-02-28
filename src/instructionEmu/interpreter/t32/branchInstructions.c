#include "common/bit.h"

#include "cpuArch/constants.h"

#include "instructionEmu/interpreter/internals.h"

#include "instructionEmu/interpreter/t32/branchInstructions.h"

u32int t32MRSInstruction(GCONTXT *context, u32int instruction)
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
          DIE_NOW(context, "mrsInstruction: cannot request spsr in user/system mode");
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

u32int t32BImmediate17Instruction(GCONTXT *context, u32int instruction)
{
  /*
   * NOTE: this Thumb instruction contains a condition code field!
   */
  DEBUG_TRACE(INTERPRETER_T32_BRANCH, context, instruction);
  if (!evaluateConditionCode(context, (instruction & 0x03c00000) >> 20))
  {
    return context->R15 + T32_INSTRUCTION_SIZE;
  }
  u32int sign = (instruction & 0x04000000) >> 6;
  u32int bitJ1 = (instruction & 0x00002000) << 5;
  u32int bitJ2 = (instruction & 0x00000800) << 8;
  u32int offset = sign | bitJ2 | bitJ1
      | /* imm6 */  ((instruction & 0x003F0000) >> 4)
      | /* imm11 */ ((instruction & 0x000007FF) << 1);
  offset = signExtend(offset, 20);

  return context->R15 + T32_INSTRUCTION_SIZE + offset;
}

u32int t32BImmediate21Instruction(GCONTXT *context, u32int instruction)
{
  DEBUG_TRACE(INTERPRETER_T32_BRANCH, context, instruction);
  u32int sign = (instruction & 0x04000000) >> 2;
  u32int bitI1 = (instruction & 0x00002000) << 11;
  u32int bitI2 = (instruction & 0x00000800) << 13;
  bitI1 = ((~(bitI1 ^ sign)) & 0x01000000) >> 1;
  bitI2 = ((~(bitI2 ^ sign)) & 0x01000000) >> 2;
  u32int offset = sign | bitI1 | bitI2
      | /* imm10 */ ((instruction & 0x03FF0000) >> 4)
      | /* imm11 */ ((instruction & 0x000007FF) << 1);
  offset = signExtend(offset, 24);

  return context->R15 + T32_INSTRUCTION_SIZE + offset;
}

u32int t32BlInstruction(GCONTXT *context, u32int instruction)
{
  DEBUG_TRACE(INTERPRETER_T32_BRANCH, context, instruction);
  u32int sign = (instruction & 0x04000000) >> 2;
  u32int bitI1 = (instruction & 0x00002000) << 11;
  u32int bitI2 = (instruction & 0x00000800) << 13;
  bitI1 = ((~(bitI1 ^ sign)) & 0x01000000) >> 1;
  bitI2 = ((~(bitI2 ^ sign)) & 0x01000000) >> 2;
  u32int offset = sign | bitI1 | bitI2
      | /* imm10 */ ((instruction & 0x03FF0000) >> 4)
      | /* imm11 */ ((instruction & 0x000007FF) << 1);
  offset = signExtend(offset, 24);

  u32int currentPC = context->R15 + T32_INSTRUCTION_SIZE;
  storeGuestGPR(14, currentPC | 1, context);
  return currentPC + offset;
}

u32int t32BlxImmediateInstruction(GCONTXT *context, u32int instruction)
{
  /*
   * NOTE: this instruction always switches to ARM mode.
   */
  DEBUG_TRACE(INTERPRETER_T32_BRANCH, context, instruction);
  u32int sign = (instruction & 0x04000000) >> 2;
  u32int bitI1 = (instruction & 0x00002000) << 11;
  u32int bitI2 = (instruction & 0x00000800) << 13;
  bitI1 = ((~(bitI1 ^ sign)) & 0x01000000) >> 1;
  bitI2 = ((~(bitI2 ^ sign)) & 0x01000000) >> 2;
  u32int offset = sign | bitI1 | bitI2
      /* imm10H */ | (instruction & 0x03FF0000) >> 4
      /* imm10L */ | (instruction & 0x000007FE) << 1;
  offset = signExtend(offset, 23);
  /*
   * Switch to ARM mode
   */
  context->CPSR ^= PSR_T_BIT;
  /*
   * In Thumb-32, R15 points to the first halfword, so LR must be 4+1(T) bytes ahead
   */
  u32int currPC = context->R15 + T16_INSTRUCTION_SIZE;
  storeGuestGPR(GPR_LR, currPC + 1, context);
  /*
   * Make sure to return a word-aligned address
   */
  return (currPC & ~2) + offset;
}
