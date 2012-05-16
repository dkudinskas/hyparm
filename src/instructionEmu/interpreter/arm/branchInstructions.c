#include "common/bit.h"

#include "instructionEmu/interpreter/internals.h"

#include "instructionEmu/interpreter/arm/branchInstructions.h"


u32int armBInstruction(GCONTXT *context, u32int instruction)
{
  if (!evaluateConditionCode(context, ARM_EXTRACT_CONDITION_CODE(instruction)))
  {
    return context->R15 + ARM_INSTRUCTION_SIZE;
  }

  DEBUG(INTERPRETER_ARM_BRANCH, "armBInstruction: %#.8x @ %#.8x" EOL, instruction, context->R15);

  u32int link = instruction & 0x0F000000;
  u32int offset = signExtend((instruction & 0x00FFFFFF) << 2, 26);
  u32int currPC = context->R15 + ARM_INSTRUCTION_SIZE;
  if (link == 0x0B000000)
  {
    storeGuestGPR(14, currPC, context);
  }
  return currPC + ARM_INSTRUCTION_SIZE + offset;
}

u32int armBlxImmediateInstruction(GCONTXT *context, u32int instruction)
{
  /*
   * NOTE: this instruction is unconditional and always switches to Thumb mode.
   */
#ifdef CONFIG_THUMB2

  DEBUG(INTERPRETER_ARM_BRANCH, "armBlxImmediateInstruction: %#.8x @ %#.8x" EOL, instruction,
      context->R15);

  u32int offset = signExtend(((instruction & 0x00FFFFFF) << 2) | (instruction & 0x01000000) >> 23, 26);

  context->CPSR |= PSR_T_BIT;
  storeGuestGPR(GPR_LR, context->R15 + ARM_INSTRUCTION_SIZE, context);

  return context->R15 + (2 * ARM_INSTRUCTION_SIZE) + offset;
#else
  DIE_NOW(context, "armBlxImmediateInstruction: Thumb is disabled (CONFIG_THUMB2 not set)");
#endif
}

u32int armBlxRegisterInstruction(GCONTXT *context, u32int instruction)
{
  if (!evaluateConditionCode(context, ARM_EXTRACT_CONDITION_CODE(instruction)))
  {
    return context->R15 + ARM_INSTRUCTION_SIZE;
  }

  DEBUG(INTERPRETER_ARM_BRANCH, "armBlxRegisterInstruction: %#.8x @ %#.8x" EOL, instruction,
        context->R15);

  u32int regDest = (instruction & 0x0000000F); // holds dest addr and mode bit

  if (regDest == GPR_PC)
  {
    DIE_NOW(context, "use of PC is unpredictable");
  }

  u32int destinationAddress = loadGuestGPR(regDest, context);
  storeGuestGPR(GPR_LR, context->R15 + ARM_INSTRUCTION_SIZE, context);

  if (destinationAddress & 1)
  {
#ifdef CONFIG_THUMB2
    context->CPSR |= PSR_T_BIT;
    destinationAddress ^= 1;
#else
    DIE_NOW(context, "Thumb is disabled (CONFIG_THUMB2 not set)");
#endif
  }
  else if (destinationAddress & 2)
  {
    DIE_NOW(context, "branch to unaligned ARM address");
  }

  return destinationAddress;
}

u32int armBxInstruction(GCONTXT *context, u32int instruction)
{
  if (!evaluateConditionCode(context, ARM_EXTRACT_CONDITION_CODE(instruction)))
  {
    return context->R15 + ARM_INSTRUCTION_SIZE;
  }

  DEBUG(INTERPRETER_ARM_BRANCH, "armBxInstruction: %#.8x @ %#.8x" EOL, instruction, context->R15);

  u32int regDest = instruction & 0x0000000F;
  u32int destinationAddress = loadGuestGPR(regDest, context);

  /*
   * Check if switching to thumb mode
   */
  if (destinationAddress & 1)
  {
#ifdef CONFIG_THUMB2
    context->CPSR |= PSR_T_BIT;
    destinationAddress ^= 1;
#else
    DIE_NOW(context, "Thumb is disabled (CONFIG_THUMB2 not set)");
#endif
  }
  else if (destinationAddress & 2)
  {
    DIE_NOW(context, "branch to unaligned ARM address");
  }

  return destinationAddress;
}

u32int armBxjInstruction(GCONTXT *context, u32int instruction)
{
  DEBUG(INTERPRETER_ARM_BRANCH, "armBxjInstruction: %#.8x @ %#.8x" EOL, instruction, context->R15);

  DIE_NOW(context, ERROR_NOT_IMPLEMENTED);
}
