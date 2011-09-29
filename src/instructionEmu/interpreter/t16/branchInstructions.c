#include "common/debug.h"

#include "cpuArch/constants.h"

#include "instructionEmu/commonInstrFunctions.h"

#include "instructionEmu/interpreter/branchInstructions.h"


u32int t16BImmediate8Instruction(GCONTXT *context, u32int instruction)
{
  /*
   * NOTE: this Thumb instruction contains a condition code field!
   */
  DEBUG(INTERPRETER_T16_BRANCH, "t16BImmediate8Instruction: %#.4x @ %#.8x" EOL, instruction,
      context->R15);

  u32int conditionCode = (instruction >> 8) & 0xF;
  if (conditionCode == 0xE)
  {
    DIE_NOW(0, "t16BImmediate8Instruction: UNDEFINED");
  }
  if (!evaluateConditionCode(context, conditionCode))
  {
    return context->R15 + T16_INSTRUCTION_SIZE;
  }
  u32int offset = (instruction & 0x00FF) << 1;
  u32int sign = offset >> 8;
  if (sign)
  {
    offset |= 0xFFFFFE00;
  }
  return context->R15 + (2 * T16_INSTRUCTION_SIZE) + offset;
}

u32int t16BImmediate11Instruction(GCONTXT *context, u32int instruction)
{
  DEBUG(INTERPRETER_T16_BRANCH, "t16BImmediate11Instruction: %#.4x @ %#.8x" EOL, instruction,
      context->R15);

  u32int offset = (instruction & 0x07FF) << 1;
  u32int sign = offset >> 11;
  if (sign)
  {
    offset |= 0xFFFFF000;
  }
  return context->R15 + (2 * T16_INSTRUCTION_SIZE) + offset;
}

u32int t16BlxRegisterInstruction(GCONTXT *context, u32int instruction)
{
  DEBUG(INTERPRETER_T16_BRANCH, "t16BlxRegisterInstruction: %#.4x @ %#.8x" EOL, instruction,
      context->R15);

  DIE_NOW(context, "t16BlxRegisterInstruction not implemented");
}

u32int t16BxInstruction(GCONTXT *context, u32int instruction)
{
  DEBUG(INTERPRETER_T16_BRANCH, "t16BxInstruction: %#.4x @ %#.8x" EOL, instruction, context->R15);

  u32int regDest = (instruction & 0x0078) >> 3;
  u32int destinationAddress = loadGuestGPR(regDest, context);

  /*
   * Return to ARM mode if needed
   */
  if (destinationAddress & 1)
  {
    destinationAddress ^= 1;
  }
  else if (destinationAddress & 2)
  {
    DIE_NOW(context, "t16BxInstruction: branch to unaligned ARM address");
  }
  else
  {
    context->CPSR ^= PSR_T_BIT;
  }

  return destinationAddress;
}
