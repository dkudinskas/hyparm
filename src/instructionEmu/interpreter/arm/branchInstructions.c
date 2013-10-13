#include "common/bit.h"

#include "instructionEmu/interpreter/internals.h"

#include "instructionEmu/interpreter/arm/branchInstructions.h"


enum
{
  BLX_RM_INDEX = 0
};


u32int armBInstruction(GCONTXT *context, u32int instruction)
{
#ifdef CONFIG_CONTEXT_SWITCH_COUNTERS
  if ((instruction & 0xF0000000) == 0xE0000000)
  {
    context->branchNonconditional++;
  }
  else
  {
    context->branchConditional++;
  }
  context->branchImmediate++;

  if ((instruction & 0x0F000000) == 0x0B000000)
  {
    context->branchLink++;
  }
  else
  {
    context->branchNonlink++;
  }

#endif

  if (!evaluateConditionCode(context, ARM_EXTRACT_CONDITION_CODE(instruction)))
  {
    return context->lastGuestPC + ARM_INSTRUCTION_SIZE;
  }

  DEBUG(INTERPRETER_ARM_BRANCH, "armBInstruction: %#.8x @ %#.8x" EOL, instruction, context->R15);

  u32int link = instruction & 0x0F000000;
  u32int offset = signExtend((instruction & 0x00FFFFFF) << 2, 26);
  u32int currPC = context->lastGuestPC + ARM_INSTRUCTION_SIZE;
  if (link == 0x0B000000)
  {
    setGPRegister(context, GPR_LR, currPC);
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

  u32int currPC = context->lastGuestPC + ARM_INSTRUCTION_SIZE;
  u32int offset = signExtend(((instruction & 0x00FFFFFF) << 2) | (instruction & 0x01000000) >> 23, 26);

  context->CPSR |= PSR_T_BIT;
  setGPRegister(context, GPR_LR, currPC);

  return currPC + ARM_INSTRUCTION_SIZE + offset;
#else
  DIE_NOW(context, "armBlxImmediateInstruction: Thumb is disabled (CONFIG_THUMB2 not set)");
#endif
}


u32int armBlxRegisterInstruction(GCONTXT *context, u32int instruction)
{
#ifdef CONFIG_CONTEXT_SWITCH_COUNTERS
  if ((instruction & 0xF0000000) == 0xE0000000)
  {
    context->branchNonconditional++;
  }
  else
  {
    context->branchConditional++;
  }
  context->branchRegister++;

  context->branchLink++;
#endif

  if (!evaluateConditionCode(context, ARM_EXTRACT_CONDITION_CODE(instruction)))
  {
    return context->lastGuestPC + ARM_INSTRUCTION_SIZE;
  }

  DEBUG(INTERPRETER_ARM_BRANCH, "armBlxRegisterInstruction: %#.8x @ %#.8x" EOL, instruction,
        context->R15);

  // Rm holds target address and mode bit for interworking branch
  const u32int targetRegister = ARM_EXTRACT_REGISTER(instruction, BLX_RM_INDEX);
  ASSERT(targetRegister != GPR_PC, ERROR_UNPREDICTABLE_INSTRUCTION);
  u32int destinationAddress = getGPRegister(context, targetRegister);

  setGPRegister(context, GPR_LR, context->lastGuestPC + ARM_INSTRUCTION_SIZE);

  if (destinationAddress & 1)
  {
#ifdef CONFIG_THUMB2
    context->CPSR |= PSR_T_BIT;
    destinationAddress ^= 1;
#else
    DIE_NOW(context, "Thumb is disabled (CONFIG_THUMB2 not set)");
#endif
  }
  else
  {
    // Branch to unaligned ARM address!
    ASSERT((destinationAddress & 2) == 0, ERROR_UNPREDICTABLE_INSTRUCTION)
  }

  return destinationAddress;
}


u32int armBxInstruction(GCONTXT *context, u32int instruction)
{
#ifdef CONFIG_CONTEXT_SWITCH_COUNTERS
  if ((instruction & 0xF0000000) == 0xE0000000)
  {
    context->branchNonconditional++;
  }
  else
  {
    context->branchConditional++;
  }

  context->branchNonlink++;
  context->branchRegister++;
#endif

  if (!evaluateConditionCode(context, ARM_EXTRACT_CONDITION_CODE(instruction)))
  {
    return context->lastGuestPC + ARM_INSTRUCTION_SIZE;
  }

  DEBUG(INTERPRETER_ARM_BRANCH, "armBxInstruction: %#.8x @ %#.8x" EOL, instruction, context->R15);

  // Rm holds target address and mode bit for interworking branch -- Rm can be PC!
  const u32int targetRegister = ARM_EXTRACT_REGISTER(instruction, BLX_RM_INDEX);
  u32int destinationAddress = getGPRegister(context, targetRegister);

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
  else
  {
    // Branch to unaligned ARM address!
    ASSERT((destinationAddress & 2) == 0, ERROR_UNPREDICTABLE_INSTRUCTION);
  }

  return destinationAddress;
}


u32int armBxjInstruction(GCONTXT *context, u32int instruction)
{
  DEBUG(INTERPRETER_ARM_BRANCH, "armBxjInstruction: %#.8x @ %#.8x" EOL, instruction, context->R15);

  DIE_NOW(context, ERROR_NOT_IMPLEMENTED);
}
