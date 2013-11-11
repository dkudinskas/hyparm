#include "instructionEmu/decoder/arm/structs.h"
#include "instructionEmu/interpreter/common.h"
#include "instructionEmu/interpreter/internals.h"

#include "instructionEmu/interpreter/arm/syncInstructions.h"


u32int armClrexInstruction(GCONTXT *context, u32int instruction)
{
  DEBUG_TRACE(INTERPRETER_ARM_SYNC, context, instruction);

  ClearExclusiveLocal();
  return context->R15 + ARM_INSTRUCTION_SIZE;
}

u32int armLdrexInstruction(GCONTXT *context, u32int instruction)
{
  Instruction instr = {.raw = instruction};
  DEBUG_TRACE(INTERPRETER_ARM_LOAD_SYNC, context, instruction);

  if (ConditionPassed(instr.ldrex.cc))
  {
    if ((instr.ldrex.Rt == GPR_PC) || (instr.ldrex.Rn == GPR_PC))
      UNPREDICTABLE();

    u32int address = getGPRegister(context, instr.ldrex.Rn);
    SetExclusiveMonitors(address,4);
    setGPRegister(context, instr.ldrex.Rt, vmLoad(context, WORD, address));
  }
  return context->R15 + ARM_INSTRUCTION_SIZE;
}

u32int armLdrexbInstruction(GCONTXT *context, u32int instruction)
{
  if (!evaluateConditionCode(context, ARM_EXTRACT_CONDITION_CODE(instruction)))
  {
    return context->R15 + ARM_INSTRUCTION_SIZE;
  }

  DEBUG_TRACE(INTERPRETER_ARM_LOAD_SYNC, context, instruction);

  u32int baseReg = ARM_EXTRACT_REGISTER(instruction, 16);
  u32int regDest = ARM_EXTRACT_REGISTER(instruction, 12);

  ASSERT(baseReg != GPR_PC, ERROR_UNPREDICTABLE_INSTRUCTION);
  ASSERT(regDest != GPR_PC, ERROR_UNPREDICTABLE_INSTRUCTION);

  u32int baseVal = getGPRegister(context, baseReg);
  // byte zero extended to word...
  u32int value = vmLoad(context, BYTE, baseVal) & 0xFF;
  setGPRegister(context, regDest, value);

  return context->R15 + ARM_INSTRUCTION_SIZE;
}

u32int armLdrexhInstruction(GCONTXT *context, u32int instruction)
{
  if (!evaluateConditionCode(context, ARM_EXTRACT_CONDITION_CODE(instruction)))
  {
    return context->R15 + ARM_INSTRUCTION_SIZE;
  }

  DEBUG_TRACE(INTERPRETER_ARM_LOAD_SYNC, context, instruction);

  u32int baseReg = ARM_EXTRACT_REGISTER(instruction, 16);
  u32int regDest = ARM_EXTRACT_REGISTER(instruction, 12);

  ASSERT(baseReg != GPR_PC, ERROR_UNPREDICTABLE_INSTRUCTION);
  ASSERT(regDest != GPR_PC, ERROR_UNPREDICTABLE_INSTRUCTION);

  u32int baseVal = getGPRegister(context, baseReg);
  // halfword zero extended to word...
  u32int value = vmLoad(context, HALFWORD, baseVal) & 0xFFFF;
  setGPRegister(context, regDest, value);

  return context->R15 + ARM_INSTRUCTION_SIZE;
}

u32int armLdrexdInstruction(GCONTXT *context, u32int instruction)
{
  if (!evaluateConditionCode(context, ARM_EXTRACT_CONDITION_CODE(instruction)))
  {
    return context->R15 + ARM_INSTRUCTION_SIZE;
  }

  DEBUG_TRACE(INTERPRETER_ARM_LOAD_SYNC, context, instruction);

  u32int baseReg = ARM_EXTRACT_REGISTER(instruction, 16);
  u32int regDest = ARM_EXTRACT_REGISTER(instruction, 12);

  // must not be PC, destination must be even and not link register
  ASSERT(baseReg != GPR_PC, ERROR_UNPREDICTABLE_INSTRUCTION);
  ASSERT((regDest & 1) == 0, ERROR_UNPREDICTABLE_INSTRUCTION);
  ASSERT(regDest != GPR_LR, ERROR_UNPREDICTABLE_INSTRUCTION);

  u32int baseVal = getGPRegister(context, baseReg);

  u32int value = vmLoad(context, WORD, baseVal);
  u32int value2 = vmLoad(context, WORD, baseVal + 4);
  setGPRegister(context, regDest, value);
  setGPRegister(context, regDest + 1, value2);

  return context->R15 + ARM_INSTRUCTION_SIZE;
}


u32int armStrexInstruction(GCONTXT *context, u32int instruction)
{
  Instruction instr = {.raw = instruction};
  DEBUG_TRACE(INTERPRETER_ARM_STORE_SYNC, context, instruction);

  if (ConditionPassed(instr.strex.cc))
  {
    if ((instr.strex.Rt == GPR_PC) || (instr.strex.Rn == GPR_PC) || (instr.strex.Rd == GPR_PC))
      UNPREDICTABLE();
    if ((instr.strex.Rd == instr.strex.Rn) || (instr.strex.Rd == instr.strex.Rt))
      UNPREDICTABLE();

    u32int address = getGPRegister(context, instr.strex.Rn);
    if (ExclusiveMonitorsPass(address,4))
    {
      vmStore(context, WORD, address, getGPRegister(context, instr.strex.Rt));
      setGPRegister(context, instr.strex.Rd, 0);
    }
    else
    {
      setGPRegister(context, instr.strex.Rd, 1);
    }
  }
  return context->R15 + ARM_INSTRUCTION_SIZE;
}

u32int armStrexbInstruction(GCONTXT *context, u32int instruction)
{
  if (!evaluateConditionCode(context, ARM_EXTRACT_CONDITION_CODE(instruction)))
  {
    return context->R15 + ARM_INSTRUCTION_SIZE;
  }

  DEBUG_TRACE(INTERPRETER_ARM_STORE_SYNC, context, instruction);

  u32int regN = ARM_EXTRACT_REGISTER(instruction, 16);
  u32int regD = ARM_EXTRACT_REGISTER(instruction, 12);
  u32int regT = ARM_EXTRACT_REGISTER(instruction, 0);

  ASSERT(regN != GPR_PC, ERROR_UNPREDICTABLE_INSTRUCTION);
  ASSERT(regD != GPR_PC, ERROR_UNPREDICTABLE_INSTRUCTION);
  ASSERT(regT != GPR_PC, ERROR_UNPREDICTABLE_INSTRUCTION);
  ASSERT(regD != regN, ERROR_UNPREDICTABLE_INSTRUCTION);
  ASSERT(regD != regT, ERROR_UNPREDICTABLE_INSTRUCTION);

  u32int address = getGPRegister(context, regN);

  u32int valToStore = getGPRegister(context, regT);
  vmStore(context, BYTE, address, valToStore & 0xFF);

  setGPRegister(context, regD, 0);

  return context->R15 + ARM_INSTRUCTION_SIZE;
}

u32int armStrexhInstruction(GCONTXT *context, u32int instruction)
{
  if (!evaluateConditionCode(context, ARM_EXTRACT_CONDITION_CODE(instruction)))
  {
    return context->R15 + ARM_INSTRUCTION_SIZE;
  }

  DEBUG_TRACE(INTERPRETER_ARM_STORE_SYNC, context, instruction);

  u32int regN = ARM_EXTRACT_REGISTER(instruction, 16);
  u32int regD = ARM_EXTRACT_REGISTER(instruction, 12);
  u32int regT = ARM_EXTRACT_REGISTER(instruction, 0);

  if (!evaluateConditionCode(context, ARM_EXTRACT_CONDITION_CODE(instruction)))
  {
    // condition not met! allright, we're done here. next instruction...
    return context->R15 + ARM_INSTRUCTION_SIZE;
  }

  ASSERT(regN != GPR_PC, ERROR_UNPREDICTABLE_INSTRUCTION);
  ASSERT(regD != GPR_PC, ERROR_UNPREDICTABLE_INSTRUCTION);
  ASSERT(regT != GPR_PC, ERROR_UNPREDICTABLE_INSTRUCTION);
  ASSERT(regD != regN, ERROR_UNPREDICTABLE_INSTRUCTION);
  ASSERT(regD != regT, ERROR_UNPREDICTABLE_INSTRUCTION);

  u32int address = getGPRegister(context, regN);

  u32int valToStore = getGPRegister(context, regT);
  vmStore(context, HALFWORD, address, valToStore & 0xFFFF);
  setGPRegister(context, regD, 0);

  return context->R15 + ARM_INSTRUCTION_SIZE;
}

u32int armStrexdInstruction(GCONTXT *context, u32int instruction)
{
  if (!evaluateConditionCode(context, ARM_EXTRACT_CONDITION_CODE(instruction)))
  {
    return context->R15 + ARM_INSTRUCTION_SIZE;
  }

  DEBUG_TRACE(INTERPRETER_ARM_STORE_SYNC, context, instruction);

  u32int regN = ARM_EXTRACT_REGISTER(instruction, 16);
  u32int regD = ARM_EXTRACT_REGISTER(instruction, 12);
  u32int regT = ARM_EXTRACT_REGISTER(instruction, 0);

  if (!evaluateConditionCode(context, ARM_EXTRACT_CONDITION_CODE(instruction)))
  {
    // condition not met! allright, we're done here. next instruction...
    return context->R15 + ARM_INSTRUCTION_SIZE;
  }

  ASSERT(regN != GPR_PC, ERROR_UNPREDICTABLE_INSTRUCTION);
  ASSERT(regD != GPR_PC, ERROR_UNPREDICTABLE_INSTRUCTION);
  ASSERT((regT & 1) == 0, ERROR_UNPREDICTABLE_INSTRUCTION);
  ASSERT(regT != GPR_LR, ERROR_UNPREDICTABLE_INSTRUCTION);
  ASSERT(regD != regN, ERROR_UNPREDICTABLE_INSTRUCTION);
  ASSERT(regD != regT, ERROR_UNPREDICTABLE_INSTRUCTION);
  ASSERT(regD != (regT + 1), ERROR_UNPREDICTABLE_INSTRUCTION);

  u32int address = getGPRegister(context, regN);

  // Create doubleword to store such that R[t] will be stored at addr and R[t2] at addr+4.
  u32int valToStore1 = getGPRegister(context, regT);
  u32int valToStore2 = getGPRegister(context, regT + 1);

  /*
   * FIXME STREXD: assuming littl endian
   */
  DIE_NOW(context, "assuming littlendian!");

  bool littleEndian = TRUE;
  if (littleEndian)
  {
    vmStore(context, WORD, address, valToStore2);
    vmStore(context, WORD, address+4, valToStore1);
  }
  else
  {
    vmStore(context, WORD, address, valToStore1);
    vmStore(context, WORD, address+4, valToStore2);
  }
  setGPRegister(context, regD, 0);

  return context->R15 + ARM_INSTRUCTION_SIZE;
}

u32int armSwpInstruction(GCONTXT *context, u32int instruction)
{
  DIE_NOW(context, ERROR_NOT_IMPLEMENTED);
}
