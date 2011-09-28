#include "common/debug.h"

#include "cpuArch/constants.h"

#include "instructionEmu/commonInstrFunctions.h"

#include "instructionEmu/interpreter/branchInstructions.h"


u32int armBInstruction(GCONTXT *context, u32int instruction)
{
  if (!evaluateConditionCode(context, ARM_EXTRACT_CONDITION_CODE(instruction)))
  {
    return context->R15 + ARM_INSTRUCTION_SIZE;
  }

#ifdef ARM_INSTR_TRACE
  printf("Branch instr %08x @ %08x\n", instruction, context->R15);
#endif

  u32int sign = instruction & 0x00800000;
  u32int link = instruction & 0x0F000000;
  u32int offset = (instruction & 0x00FFFFFF) << 2;
  /*
   * Sign extend 26-bit offset to 32-bit
   */
  if (sign)
  {
    offset |= 0xFC000000;
  }
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
  u32int offset = ((instruction & 0x00FFFFFF) << 2) | (instruction & 0x01000000) >> 23;
  u32int sign = offset >> 25;

  if (sign)
  {
    offset |= 0xFC000000;
  }

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

  u32int regDest = (instruction & 0x0000000F); // holds dest addr and mode bit

  if (regDest == GPR_PC)
  {
    DIE_NOW(context, "armBlxRegisterInstruction: use of PC is unpredictable");
  }

  u32int destinationAddress = loadGuestGPR(regDest, context);
  storeGuestGPR(GPR_LR, context->R15 + ARM_INSTRUCTION_SIZE, context);

  if (destinationAddress & 1)
  {
#ifdef CONFIG_THUMB2
    context->CPSR |= PSR_T_BIT;
    destinationAddress ^= 1;
#else
    DIE_NOW(context, "armBlxRegisterInstruction: Thumb is disabled (CONFIG_THUMB2 not set)");
#endif
  }
  else if (destinationAddress & 2)
  {
    DIE_NOW(context, "armBlxRegisterInstruction: branch to unaligned ARM address");
  }

  return destinationAddress;
}

u32int armBxInstruction(GCONTXT *context, u32int instruction)
{
  if (!evaluateConditionCode(context, ARM_EXTRACT_CONDITION_CODE(instruction)))
  {
    return context->R15 + ARM_INSTRUCTION_SIZE;
  }

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
    DIE_NOW(context, "armBxInstruction: Thumb is disabled (CONFIG_THUMB2 not set)");
#endif
  }
  else if (destinationAddress & 2)
  {
    DIE_NOW(context, "armBxInstruction: branch to unaligned ARM address");
  }

  return destinationAddress;
}

u32int armBxjInstruction(GCONTXT *context, u32int instruction)
{
  DIE_NOW(context, "BXJ not implemented");
}
