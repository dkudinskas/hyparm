#include "common/bit.h"

#include "instructionEmu/interpreter/internals.h"

#include "instructionEmu/interpreter/t16/branchInstructions.h"


u32int t16BImmediate8Instruction(GCONTXT *context, u32int instruction)
{
  /*
   * NOTE: this Thumb instruction contains a condition code field!
   */
  DEBUG_TRACE(INTERPRETER_T16_BRANCH, context, instruction);
  u32int conditionCode = (instruction >> 8) & 0xF;
  if (conditionCode == 0xE)
  {
    DIE_NOW(context, "UNDEFINED");
  }
  if (!evaluateConditionCode(context, conditionCode))
  {
    return context->R15 + T16_INSTRUCTION_SIZE;
  }
  u32int offset = signExtend((instruction & 0x00FF) << 1, 9);
  return context->R15 + (2 * T16_INSTRUCTION_SIZE) + offset;
}

u32int t16BImmediate11Instruction(GCONTXT *context, u32int instruction)
{
  DEBUG_TRACE(INTERPRETER_T16_BRANCH, context, instruction);
  u32int offset = signExtend((instruction & 0x07FF) << 1, 12);
  return context->R15 + (2 * T16_INSTRUCTION_SIZE) + offset;
}

u32int t16BlxRegisterInstruction(GCONTXT *context, u32int instruction)
{
  DEBUG_TRACE(INTERPRETER_T16_BRANCH, context, instruction);
  DIE_NOW(context, "not implemented");
}

u32int t16BxInstruction(GCONTXT *context, u32int instruction)
{
  DEBUG_TRACE(INTERPRETER_T16_BRANCH, context, instruction);
  u32int regDest = (instruction & 0x0078) >> 3;
  u32int destinationAddress = loadGuestGPR(regDest, context);
  /*
   * Return to ARM mode if the LSB is not set; also make sure the target address is word-aligned.
   */
  if (destinationAddress & 1)
  {
    destinationAddress ^= 1;
  }
  else if (destinationAddress & 2)
  {
    DIE_NOW(context, "unpredictable branch to unaligned ARM address");
  }
  else
  {
    context->CPSR ^= PSR_T_BIT;
  }
  return destinationAddress;
}
