#include "common/bit.h"

#include "instructionEmu/interpreter/common.h"
#include "instructionEmu/interpreter/internals.h"
#include "instructionEmu/interpreter/arm/branchInstructions.h"

#include "perf/contextSwitchCounters.h"


u32int armBInstruction(GCONTXT *context, Instruction instr)
{
  countBranch(context, instr);
  DEBUG(INTERPRETER_ARM_BRANCH, "armBInstruction: %08x @ %08x\n", instr.raw, context->R15);

  if (ConditionPassed(instr.branch.cc))
  {
    u32int imm32 = signExtend(instr.branch.imm24 << 2, 26);
    BranchWritePC(context, PC(context) + imm32);
    return context->R15;
  }
  return context->R15 + ARM_INSTRUCTION_SIZE;
}


u32int armBlInstruction(GCONTXT *context, Instruction instr)
{
  countBL(context, instr);
  DEBUG(INTERPRETER_ARM_BRANCH, "armBLInstruction: %08x @ %08x\n",
                                         instr.raw, context->R15);

  if (ConditionPassed(instr.branch.cc))
  {
    u32int imm32 = signExtend(instr.branch.imm24 << 2, 26);
    u32int pc = PC(context);
    setGPRegister(context, GPR_LR, pc-4);
    SelectInstrSet(InstrSet_ARM);
    BranchWritePC(context, pc + imm32);
    return context->R15;
  }
  return context->R15 + ARM_INSTRUCTION_SIZE;
}


/*
 * NOTE: this instruction is unconditional and always switches to Thumb mode.
 */
u32int armBlxImmediateInstruction(GCONTXT *context, Instruction instr)
{
#ifdef CONFIG_THUMB2

  DEBUG(INTERPRETER_ARM_BRANCH, "armBlxImmediateInstruction: %08x @ %#08x\n",
                                instr.raw, context->R15);

  u32int currPC = context->R15 + ARM_INSTRUCTION_SIZE;
  u32int offset = signExtend(((instruction & 0x00FFFFFF) << 2) | (instruction & 0x01000000) >> 23, 26);

  context->CPSR.bits.T = 1;
  setGPRegister(context, GPR_LR, currPC);

  return currPC + ARM_INSTRUCTION_SIZE + offset;
#else
  DIE_NOW(context, "armBlxImmediateInstruction: Thumb is disabled (CONFIG_THUMB2 not set)");
#endif
}


u32int armBlxRegisterInstruction(GCONTXT *context, Instruction instr)
{
  countBLXreg(context, instr);
  DEBUG(INTERPRETER_ARM_BRANCH, "armBlxRegisterInstruction: %08x @ %08x\n",
                                 instr.raw, context->R15);
  if (ConditionPassed(instr.BxReg.cc))
  {
    if (instr.BxReg.Rm == GPR_PC)
      UNPREDICTABLE();

    u32int target = getGPRegister(context, instr.BxReg.Rm);
    if (CurrentInstrSet() == InstrSet_ARM)
      setGPRegister(context, GPR_LR, PC(context) - 4);
    else
      setGPRegister(context, GPR_LR, (PC(context) - 2) | 1);

    BXWritePC(context, target);
    return context->R15;
  }
  return context->R15 + ARM_INSTRUCTION_SIZE;
}


u32int armBxInstruction(GCONTXT *context, Instruction instr)
{
  countBX(context, instr);
  DEBUG(INTERPRETER_ARM_BRANCH, "armBxInstruction: %08x @ %08x\n",
                                         instr.raw, context->R15);

  if (ConditionPassed(instr.BxReg.cc))
  {
    BXWritePC(context, getGPRegister(context, instr.BxReg.Rm));
    return context->R15;
  }
  return context->R15 + ARM_INSTRUCTION_SIZE;
}


u32int armBxjInstruction(GCONTXT *context, Instruction instr)
{
  DEBUG(INTERPRETER_ARM_BRANCH, "armBxjInstruction: %08x @ %08x\n",
                                instr.raw, context->R15);

  DIE_NOW(context, ERROR_NOT_IMPLEMENTED);
}
