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
  Instruction instr = {.raw = instruction};
  DEBUG_TRACE(INTERPRETER_ARM_LOAD_SYNC, context, instruction);

  if (ConditionPassed(instr.ldrex.cc))
  {
    if ((instr.ldrex.Rt == GPR_PC) || (instr.ldrex.Rn == GPR_PC))
      UNPREDICTABLE();

    u32int address = getGPRegister(context, instr.ldrex.Rn);
    SetExclusiveMonitors(address, 1);
    setGPRegister(context, instr.ldrex.Rt, vmLoad(context, BYTE, address) & 0xFF);
  }
  return context->R15 + ARM_INSTRUCTION_SIZE;
}


u32int armLdrexhInstruction(GCONTXT *context, u32int instruction)
{
  Instruction instr = {.raw = instruction};
  DEBUG_TRACE(INTERPRETER_ARM_LOAD_SYNC, context, instruction);

  if (ConditionPassed(instr.ldrex.cc))
  {
    if ((instr.ldrex.Rt == GPR_PC) || (instr.ldrex.Rn == GPR_PC))
      UNPREDICTABLE();

    u32int address = getGPRegister(context, instr.ldrex.Rn);
    SetExclusiveMonitors(address, 2);
    setGPRegister(context, instr.ldrex.Rt, vmLoad(context, HALFWORD, address) & 0xFFFF);
  }
  return context->R15 + ARM_INSTRUCTION_SIZE;
}


u32int armLdrexdInstruction(GCONTXT *context, u32int instruction)
{
  Instruction instr = {.raw = instruction};
  DEBUG_TRACE(INTERPRETER_ARM_LOAD_SYNC, context, instruction);

  if (ConditionPassed(instr.ldrex.cc))
  {
    u8int Rt = instr.ldrex.Rt;
    u8int Rn = instr.ldrex.Rn;
    if ( ((Rt & 1) == 1) || (Rt == GPR_LR) || (Rn == GPR_PC) )
      UNPREDICTABLE();

    u32int address = getGPRegister(context, Rn);
    u32int value = vmLoad(context, WORD, address);
    u32int value2 = vmLoad(context, WORD, address + 4);

    SetExclusiveMonitors(address, 8);

    // STARFIX: strictly speaking this is incorrect - atomicity problem
    if (BigEndian(context))
    {
      setGPRegister(context, Rt, value);
      setGPRegister(context, Rt + 1, value2);
    }
    else
    {
      setGPRegister(context, Rt, value);
      setGPRegister(context, Rt + 1, value2);
    }
  }
  return context->R15 + ARM_INSTRUCTION_SIZE;
}


u32int armStrexInstruction(GCONTXT *context, u32int instruction)
{
  Instruction instr = {.raw = instruction};
  DEBUG_TRACE(INTERPRETER_ARM_STORE_SYNC, context, instruction);

  if (ConditionPassed(instr.strex.cc))
  {
    u8int Rt = instr.strex.Rt;
    u8int Rn = instr.strex.Rn;
    u8int Rd = instr.strex.Rd;
    if ((Rt == GPR_PC) || (Rn == GPR_PC) || (Rd == GPR_PC))
      UNPREDICTABLE();
    if ((Rd == Rn) || (Rd == Rt))
      UNPREDICTABLE();

    u32int address = getGPRegister(context, Rn);
    if (ExclusiveMonitorsPass(address,4))
    {
      vmStore(context, WORD, address, getGPRegister(context, Rt));
      setGPRegister(context, Rd, 0);
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
  Instruction instr = {.raw = instruction};
  DEBUG_TRACE(INTERPRETER_ARM_STORE_SYNC, context, instruction);

  if (ConditionPassed(instr.strex.cc))
  {
    u8int Rt = instr.strex.Rt;
    u8int Rn = instr.strex.Rn;
    u8int Rd = instr.strex.Rd;
    if ((Rt == GPR_PC) || (Rn == GPR_PC) || (Rd == GPR_PC))
      UNPREDICTABLE();
    if ((Rd == Rn) || (Rd == Rt))
      UNPREDICTABLE();

    u32int address = getGPRegister(context, Rn);
    if (ExclusiveMonitorsPass(address,1))
    {
      vmStore(context, BYTE, address, getGPRegister(context, Rt) & 0xFF);
      setGPRegister(context, Rd, 0);
    }
    else
    {
      setGPRegister(context, Rd, 1);
    }
  }
  return context->R15 + ARM_INSTRUCTION_SIZE;
}


u32int armStrexhInstruction(GCONTXT *context, u32int instruction)
{
  Instruction instr = {.raw = instruction};
  DEBUG_TRACE(INTERPRETER_ARM_STORE_SYNC, context, instruction);

  if (ConditionPassed(instr.strex.cc))
  {
    u8int Rt = instr.strex.Rt;
    u8int Rn = instr.strex.Rn;
    u8int Rd = instr.strex.Rd;
    if ((Rt == GPR_PC) || (Rn == GPR_PC) || (Rd == GPR_PC))
      UNPREDICTABLE();
    if ((Rd == Rn) || (Rd == Rt))
      UNPREDICTABLE();

    u32int address = getGPRegister(context, Rn);
    if (ExclusiveMonitorsPass(address, 2))
    {
      vmStore(context, HALFWORD, address, getGPRegister(context, Rt) & 0xFFFF);
      setGPRegister(context, Rd, 0);
    }
    else
    {
      setGPRegister(context, Rd, 1);
    }
  }
  return context->R15 + ARM_INSTRUCTION_SIZE;
}


u32int armStrexdInstruction(GCONTXT *context, u32int instruction)
{
  Instruction instr = {.raw = instruction};
  DEBUG_TRACE(INTERPRETER_ARM_STORE_SYNC, context, instruction);

  if (ConditionPassed(instr.strex.cc))
  {
    u8int Rt = instr.strex.Rt;
    u8int Rn = instr.strex.Rn;
    u8int Rd = instr.strex.Rd;
    if ( ((Rt & 1) == 1) || (Rt == GPR_LR) || (Rn == GPR_PC) || (Rd == GPR_PC))
      UNPREDICTABLE();
    if ((Rd == Rn) || (Rd == Rt) || (Rd == Rt+1))
      UNPREDICTABLE();

    u32int address = getGPRegister(context, Rn);
    if (ExclusiveMonitorsPass(address, 8))
    {
      u32int valToStore1 = getGPRegister(context, Rt);
      u32int valToStore2 = getGPRegister(context, Rt + 1);
      vmStore(context, HALFWORD, address, getGPRegister(context, Rt) & 0xFFFF);
      setGPRegister(context, Rd, 0);
      // STARFIX: strictly speaking this is incorrect - atomicity problem
      if (BigEndian(context))
      {
        vmStore(context, WORD, address, valToStore1);
        vmStore(context, WORD, address+4, valToStore2);
      }
      else
      {
        vmStore(context, WORD, address, valToStore2);
        vmStore(context, WORD, address+4, valToStore1);
      }
    }
    else
    {
      setGPRegister(context, Rd, 1);
    }
  }
  return context->R15 + ARM_INSTRUCTION_SIZE;
}

u32int armSwpInstruction(GCONTXT *context, u32int instruction)
{
  DIE_NOW(context, ERROR_NOT_IMPLEMENTED);
}
