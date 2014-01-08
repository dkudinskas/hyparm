#include "common/bit.h"

#include "instructionEmu/interpreter/common.h"
#include "instructionEmu/interpreter/internals.h"
#include "instructionEmu/interpreter/arm/loadInstructions.h"

#include "guestManager/guestExceptions.h"

#include "memoryManager/memoryProtection.h"

#include "perf/contextSwitchCounters.h"

/************************************************************/
/************************** BYTE loads***********************/
u32int armLdrbImmInstruction(GCONTXT* context, u32int instruction)
{
  Instruction instr = {.raw = instruction};
  DEBUG_TRACE(INTERPRETER_ARM_LOAD, context, instruction);
  countLdrbImm(context);
  if (ConditionPassed(instr.ldStImm.cc))
  {
    u8int Rt = instr.ldStImm.Rt, Rn = instr.ldStImm.Rn;
    u32int imm32 = instr.ldStImm.imm12 & 0xfff;
    bool index = instr.ldStImm.P, add = instr.ldStImm.U;
    bool wback = (!index) || instr.ldStImm.W;
    if ((Rt == GPR_PC) || (wback && (Rt == Rn)))
      UNPREDICTABLE();

    u32int base = getGPRegister(context, Rn);
    u32int offsetAddr = add ? (base + imm32) : (base - imm32);
    u32int address = index ? offsetAddr : base;

    setGPRegister(context, Rt, vmLoad(context, BYTE, address));

    if (wback)
      setGPRegister(context, Rn, offsetAddr);
  }
  return context->R15 + ARM_INSTRUCTION_SIZE;
}


u32int armLdrbRegInstruction(GCONTXT* context, u32int instruction)
{
  Instruction instr = {.raw = instruction};
  DEBUG_TRACE(INTERPRETER_ARM_LOAD, context, instruction);
  countLdrbReg(context);
  if (ConditionPassed(instr.ldStReg.cc))
  {
    u8int Rt = instr.ldStReg.Rt, Rn = instr.ldStReg.Rn, Rm = instr.ldStReg.Rm;
    bool index = instr.ldStReg.P, add = instr.ldStReg.U;
    bool wback = (!index) || instr.ldStReg.W;
    u32int shiftAmount = 0;
    // this call also sets shiftAmount
    u32int shiftType = decodeShiftImmediate(instr.ldStReg.type, 
                             instr.ldStReg.imm5, &shiftAmount);
    if ((Rt == GPR_PC) || (Rm == GPR_PC))
      UNPREDICTABLE();
    if (wback && ((Rn == GPR_PC) || (Rn == Rt)))
      UNPREDICTABLE();

    u32int base = getGPRegister(context, Rn);
    u32int offset = shiftVal(getGPRegister(context, Rm), shiftType, shiftAmount);
    u32int offsetAddr = add ? (base + offset) : (base - offset);
    u32int address = index ? offsetAddr : base;

    setGPRegister(context, Rt, vmLoad(context, BYTE, address));

    if (wback)
      setGPRegister(context, Rn, offsetAddr);

  }
  return context->R15 + ARM_INSTRUCTION_SIZE;
}


u32int armLdrbtImmInstruction(GCONTXT* context, u32int instruction)
{
  Instruction instr = {.raw = instruction};
  DEBUG_TRACE(INTERPRETER_ARM_LOAD, context, instruction);
  countLdrbtImm(context);
  if (ConditionPassed(instr.ldStImm.cc))
  {
    u8int Rt = instr.ldStImm.Rt, Rn = instr.ldStImm.Rn;
    u32int imm32 = instr.ldStImm.imm12 & 0xfff;
    bool add = instr.ldStImm.U;
    if ((Rt == GPR_PC) || (Rn == GPR_PC) || (Rt == Rn))
      UNPREDICTABLE();

    u32int address = getGPRegister(context, Rn);
    if (shouldDataAbort(context, FALSE, FALSE, address))
      return context->R15; // abort!

    u32int data = vmLoad(context, BYTE, address);
    setGPRegister(context, Rn, add ? (address + imm32) : (address - imm32));

    setGPRegister(context, Rt, data);
  }
  return context->R15 + ARM_INSTRUCTION_SIZE;
}


u32int armLdrbtRegInstruction(GCONTXT* context, u32int instruction)
{
  Instruction instr = {.raw = instruction};
  DEBUG_TRACE(INTERPRETER_ARM_LOAD, context, instruction);
  countLdrbtReg(context);
  if (ConditionPassed(instr.ldStReg.cc))
  {
    u8int Rt = instr.ldStReg.Rt, Rn = instr.ldStReg.Rn, Rm = instr.ldStReg.Rm;
    bool add = instr.ldStReg.U;
    u32int shiftAmount = 0;
    // this call also sets shiftAmount
    u32int shiftType = decodeShiftImmediate(instr.ldStReg.type, 
                             instr.ldStReg.imm5, &shiftAmount);
    if ((Rt == GPR_PC) || (Rn == GPR_PC) || (Rm == GPR_PC) || (Rt == Rn))
      UNPREDICTABLE();

    u32int address = getGPRegister(context, Rn);
    if (shouldDataAbort(context, FALSE, FALSE, address))
      return context->R15; // abort!

    u32int data = vmLoad(context, BYTE, address);
    u32int offset = shiftVal(getGPRegister(context, Rm), shiftType, shiftAmount);
    // writeback address
    setGPRegister(context, Rn, add ? (address + offset) : (address - offset));
    // write data
    setGPRegister(context, Rt, data);
  }
  return context->R15 + ARM_INSTRUCTION_SIZE;
}


/****************************************************************/
/************************** HALFWORD loads***********************/
u32int armLdrhImmInstruction(GCONTXT* context, u32int instruction)
{
  Instruction instr = {.raw = instruction};
  DEBUG_TRACE(INTERPRETER_ARM_LOAD, context, instruction);
  countLdrhImm(context);
  if (ConditionPassed(instr.ldStImm2.cc))
  {
    u8int Rt = instr.ldStImm2.Rt, Rn = instr.ldStImm2.Rn;
    u32int imm32 = instr.ldStImm2.imm4H << 4 | instr.ldStImm2.imm4L;
    bool index = instr.ldStImm2.P, add = instr.ldStImm2.U;
    bool wback = (!index) || instr.ldStImm2.W;
    if ((Rt == GPR_PC) || (wback && (Rt == Rn)))
      UNPREDICTABLE();

    u32int base = getGPRegister(context, Rn);
    u32int offsetAddr = add ? (base + imm32) : (base - imm32);
    u32int address = index ? offsetAddr : base;

    setGPRegister(context, Rt, vmLoad(context, HALFWORD, address));

    if (wback)
      setGPRegister(context, Rn, offsetAddr);
  }
  return context->R15 + ARM_INSTRUCTION_SIZE;
}


u32int armLdrhRegInstruction(GCONTXT* context, u32int instruction)
{
  Instruction instr = {.raw = instruction};
  DEBUG_TRACE(INTERPRETER_ARM_LOAD, context, instruction);
  countLdrhReg(context);
  if (ConditionPassed(instr.ldStReg2.cc))
  {
    u8int Rt = instr.ldStReg2.Rt, Rn = instr.ldStReg2.Rn, Rm = instr.ldStReg2.Rm;
    bool index = instr.ldStReg2.P, add = instr.ldStReg2.U;
    bool wback = (!index) || instr.ldStReg2.W;
    u32int shiftAmount = 0;
    // this call also sets shiftAmount
    u32int shiftType = decodeShiftImmediate(SHIFT_TYPE_LSL, 0, &shiftAmount);
    if ((Rt == GPR_PC) || (Rm == GPR_PC))
      UNPREDICTABLE();
    if (wback && ((Rn == GPR_PC) || (Rn == Rt)))
      UNPREDICTABLE();

    u32int base = getGPRegister(context, Rn);
    u32int offset = shiftVal(getGPRegister(context, Rm), shiftType, shiftAmount);
    u32int offsetAddr = add ? (base + offset) : (base - offset);
    u32int address = index ? offsetAddr : base;

    setGPRegister(context, Rt, vmLoad(context, HALFWORD, address));

    if (wback)
      setGPRegister(context, Rn, offsetAddr);

  }
  return context->R15 + ARM_INSTRUCTION_SIZE;
}


u32int armLdrhtImmInstruction(GCONTXT* context, u32int instruction)
{
  Instruction instr = {.raw = instruction};
  DEBUG_TRACE(INTERPRETER_ARM_LOAD, context, instruction);
  countLdrhtImm(context);
  if (ConditionPassed(instr.ldStImm2.cc))
  {
    u8int Rt = instr.ldStImm2.Rt, Rn = instr.ldStImm2.Rn;
    bool add = instr.ldStImm2.U;
    u32int imm32 = instr.ldStImm2.imm4H << 4 | instr.ldStImm2.imm4L;

    if ((Rt == GPR_PC) || (Rn == GPR_PC) || (Rn == Rt))
      UNPREDICTABLE();


    u32int address = getGPRegister(context, Rn);
    if (shouldDataAbort(context, FALSE, FALSE, address))
      return context->R15; // abort!

    u32int data = vmLoad(context, BYTE, address);

    // writeback address
    setGPRegister(context, Rn, add ? (address + imm32) : (address - imm32));
    // write data
    setGPRegister(context, Rt, data);
  }
  return context->R15 + ARM_INSTRUCTION_SIZE;
}


u32int armLdrhtRegInstruction(GCONTXT* context, u32int instruction)
{
  Instruction instr = {.raw = instruction};
  DEBUG_TRACE(INTERPRETER_ARM_LOAD, context, instruction);
  countLdrhtReg(context);
  if (ConditionPassed(instr.ldStReg2.cc))
  {
    u8int Rt = instr.ldStReg2.Rt, Rn = instr.ldStReg2.Rn, Rm = instr.ldStReg2.Rm;
    bool add = instr.ldStReg2.U;

    if ((Rt == GPR_PC) || (Rn == GPR_PC) || (Rm == GPR_PC) || (Rn == Rt))
      UNPREDICTABLE();

    u32int address = getGPRegister(context, Rn);
    if (shouldDataAbort(context, FALSE, FALSE, address))
      return context->R15; // abort!

    u32int data = vmLoad(context, BYTE, address);
    u32int offset = getGPRegister(context, Rm);

    // writeback address
    setGPRegister(context, Rn, add ? (address + offset) : (address - offset));
    // write data
    setGPRegister(context, Rt, data);
  }
  return context->R15 + ARM_INSTRUCTION_SIZE;
}


/****************************************************************/
/************************** WORD loads *************************/
u32int armLdrImmInstruction(GCONTXT* context, u32int instruction)
{
  Instruction instr = {.raw = instruction};
  DEBUG_TRACE(INTERPRETER_ARM_LOAD, context, instruction);
  countLdrImm(context);
  if (ConditionPassed(instr.ldStImm.cc))
  {
    u8int Rt = instr.ldStImm.Rt, Rn = instr.ldStImm.Rn;
    u32int imm32 = instr.ldStImm.imm12 & 0xfff;
    bool index = instr.ldStImm.P, add = instr.ldStImm.U;
    bool wback = (!index) || instr.ldStImm.W;
    if (wback && (Rt == Rn))
      UNPREDICTABLE();

    u32int base = getGPRegister(context, Rn);
    u32int offsetAddr = add ? (base + imm32) : (base - imm32);
    u32int address = index ? offsetAddr : base;
    u32int data = vmLoad(context, WORD, address);
    if (wback)
      setGPRegister(context, Rn, offsetAddr);

    if (Rt == GPR_PC)
    {
      if ((address & 3) == 0)
      {
        LoadWritePC(context, data);
        return context->R15;
      }
      else
        UNPREDICTABLE(); // unaligned value to PC
    }
    else
      setGPRegister(context, Rt, data);
  }
  return context->R15 + ARM_INSTRUCTION_SIZE;
}


u32int armLdrRegInstruction(GCONTXT* context, u32int instruction)
{
  Instruction instr = {.raw = instruction};
  DEBUG_TRACE(INTERPRETER_ARM_LOAD, context, instruction);
  countLdrReg(context);
  if (ConditionPassed(instr.ldStReg.cc))
  {
    u8int Rt = instr.ldStReg.Rt, Rn = instr.ldStReg.Rn, Rm = instr.ldStReg.Rm;
    bool index = instr.ldStReg.P, add = instr.ldStReg.U;
    bool wback = (!index) || instr.ldStReg.W;
    u32int shiftAmount = 0;
    // this call also sets shiftAmount
    u32int shiftType = decodeShiftImmediate(instr.ldStReg.type, 
                             instr.ldStReg.imm5, &shiftAmount);
    if (Rm == GPR_PC)
      UNPREDICTABLE();
    if (wback && ((Rn == GPR_PC) || (Rn == Rt)))
      UNPREDICTABLE();


    u32int base = getGPRegister(context, Rn);
    u32int offset = shiftVal(getGPRegister(context, Rm), shiftType, shiftAmount);
    u32int offsetAddr = add ? (base + offset) : (base - offset);
    u32int address = index ? offsetAddr : base;
    u32int data = vmLoad(context, WORD, address);

    if (wback)
      setGPRegister(context, Rn, offsetAddr);

    if (Rt == GPR_PC)
    {
      if ((address & 3) == 0)
      {
        LoadWritePC(context, data);
        return context->R15;
      }
      else
        UNPREDICTABLE(); // unaligned value to PC
    }
    else
      setGPRegister(context, Rt, data);
  }
  return context->R15 + ARM_INSTRUCTION_SIZE;
}


u32int armLdrtImmInstruction(GCONTXT* context, u32int instruction)
{
  Instruction instr = {.raw = instruction};
  DEBUG_TRACE(INTERPRETER_ARM_LOAD, context, instruction);
  countLdrtImm(context);
  if (ConditionPassed(instr.ldStImm.cc))
  {
    u8int Rt = instr.ldStImm.Rt, Rn = instr.ldStImm.Rn;
    bool add = instr.ldStImm.U;
    if ((Rt == GPR_PC) || (Rn == GPR_PC) || (Rt == Rn))
      UNPREDICTABLE();

    u32int offset = instr.ldStImm.imm12 & 0xfff;
    u32int address = getGPRegister(context, Rn);
    u32int offsetAddr = add ? (address + offset) : (address - offset);

    if (shouldDataAbort(context, FALSE, FALSE, address))
      return context->R15; // abort!

    u32int data = vmLoad(context, WORD, address);
    setGPRegister(context, Rn, offsetAddr);
    setGPRegister(context, Rt, data);
  }
  return context->R15 + ARM_INSTRUCTION_SIZE;
}


u32int armLdrtRegInstruction(GCONTXT* context, u32int instruction)
{
  Instruction instr = {.raw = instruction};
  DEBUG_TRACE(INTERPRETER_ARM_LOAD, context, instruction);
  countLdrtReg(context);
  if (ConditionPassed(instr.ldStReg.cc))
  {
    u8int Rt = instr.ldStReg.Rt, Rn = instr.ldStReg.Rn, Rm = instr.ldStReg.Rm;
    bool add = instr.ldStReg.U;

    u32int shiftAmount = 0;
    // this call also sets shiftAmount
    u32int shiftType = decodeShiftImmediate(instr.ldStReg.type, 
                             instr.ldStReg.imm5, &shiftAmount);

    if ((Rt == GPR_PC) || (Rn == GPR_PC) || (Rt == Rn) || (Rm == GPR_PC))
      UNPREDICTABLE();

    u32int address = getGPRegister(context, Rn);
    if (shouldDataAbort(context, FALSE, FALSE, address))
      return context->R15; // abort!

    u32int data = vmLoad(context, WORD, address);
    u32int offset = shiftVal(getGPRegister(context, Rm), shiftType, shiftAmount);
    setGPRegister(context, Rn, add ? (address + offset) : (address - offset));
    setGPRegister(context, Rt, data);
  }
  return context->R15 + ARM_INSTRUCTION_SIZE;
}


/**************************************************************/
/************************** DUAL WORD loads********************/
u32int armLdrdImmInstruction(GCONTXT* context, u32int instruction)
{
  Instruction instr = {.raw = instruction};
  DEBUG_TRACE(INTERPRETER_ARM_LOAD, context, instruction);
  countLdrdImm(context);
  if (ConditionPassed(instr.ldStImm2.cc))
  {
    u8int Rt = instr.ldStImm2.Rt, Rt2 = Rt + 1, Rn = instr.ldStImm2.Rn;
    bool index = instr.ldStImm2.P, add = instr.ldStImm2.U;
    bool wback = (!index) || (instr.ldStImm2.W);
    u32int imm32 = ((instr.ldStImm2.imm4H << 4) | instr.ldStImm2.imm4L) & 0xff;

    if ((Rt & 1) == 1)
      UNPREDICTABLE();
    if ((!index) && (instr.ldStImm2.W))
      UNPREDICTABLE();
    if (wback && ((Rn == Rt) || (Rn == Rt2)))
      UNPREDICTABLE();
    if (Rt2 == GPR_PC)
      UNPREDICTABLE();

    u32int base = getGPRegister(context, Rn);
    u32int offsetAddress = add ? (base + imm32) : (base - imm32);
    u32int address = index ? offsetAddress : base;    

    u32int data = vmLoad(context, WORD, address);
    u32int data2 = vmLoad(context, WORD, address+4);
    setGPRegister(context, Rt, data);
    setGPRegister(context, Rt, data2);

    if (wback)
      setGPRegister(context, Rn, offsetAddress);
  }
  return context->R15 + ARM_INSTRUCTION_SIZE;
}


u32int armLdrdRegInstruction(GCONTXT* context, u32int instruction)
{
  Instruction instr = {.raw = instruction};
  DEBUG_TRACE(INTERPRETER_ARM_LOAD, context, instruction);
  countLdrdReg(context);
  if (ConditionPassed(instr.ldStReg2.cc))
  {
    u8int Rt = instr.ldStReg2.Rt, Rt2 = Rt + 1;
    u8int Rm = instr.ldStReg2.Rm, Rn = instr.ldStReg2.Rn;
    bool index = instr.ldStReg2.P, add = instr.ldStReg2.U;
    bool wback = (!index) || (instr.ldStReg2.W);

    if ((Rt & 1) == 1)
      UNPREDICTABLE();
    if ((!index) && (instr.ldStReg2.W))
      UNPREDICTABLE();

    if (wback && ((Rn == GPR_PC) || (Rn == Rt) || (Rn == Rt2)))
      UNPREDICTABLE();
    if ((Rt2 == GPR_PC) || (Rm == GPR_PC) || (Rm == Rt) || (Rm == Rt2))
      UNPREDICTABLE();

    u32int base = getGPRegister(context, Rn), offset = getGPRegister(context, Rm);
    u32int offsetAddress = add ? (base + offset) : (base - offset);
    u32int address = index ? offsetAddress : base;    

    u32int data = vmLoad(context, WORD, address);
    u32int data2 = vmLoad(context, WORD, address+4);
    setGPRegister(context, Rt, data);
    setGPRegister(context, Rt, data2);

    if (wback)
      setGPRegister(context, Rn, offsetAddress);
  }
  return context->R15 + ARM_INSTRUCTION_SIZE;
}


/**************************************************************/
/************************** multiword load ********************/
u32int armLdmInstruction(GCONTXT *context, u32int instruction)
{
  Instruction instr = {.raw = instruction};
  DEBUG_TRACE(INTERPRETER_ARM_LOAD, context, instruction);
  countLdm(context);
  if (ConditionPassed(instr.ldStMulti.cc))
  {
    u8int Rn = instr.ldStMulti.Rn;
    bool before = instr.ldStMulti.P, increment = instr.ldStMulti.U;
    bool wback = instr.ldStMulti.W;
    u16int registers = instr.ldStMulti.regList;

    if ((Rn == GPR_PC) || (registers == 0))
      UNPREDICTABLE();
    if (wback && (((registers >> Rn) & 1) == 1))
      UNPREDICTABLE();

    u32int bitCount = countBitsSet(registers);
    u32int base = getGPRegister(context, Rn);
    u32int address = 0;
    if (increment && before)        // LDMIB
      address = base + 4;
    else if (!increment && before)  // LDMDB
      address = base - (4 * bitCount);
    else if (!increment && !before) // LDMDA
      address = base - (4 * bitCount) + 4;
    else                            // LDMIA
      address = base;

    int i = 0;
    for (i = 0; i < 15; i++)
    {
      if (((registers >> i) & 1) == 1)
      {
        setGPRegister(context, i, vmLoad(context, WORD, address));
        address += 4;
      }
    }

    if (wback)
    {
      if (increment)
        setGPRegister(context, Rn, base + 4 * bitCount);
      else
        setGPRegister(context, Rn, base - 4 * bitCount);
    }

    if (((registers >> 15) & 1) == 1)
    {
      LoadWritePC(context, vmLoad(context, WORD, address));
      return context->R15;
    }

  }
  return context->R15 + ARM_INSTRUCTION_SIZE;
}


u32int armLdmUserInstruction(GCONTXT *context, u32int instruction)
{
  Instruction instr = {.raw = instruction};
  DEBUG_TRACE(INTERPRETER_ARM_LOAD, context, instruction);
  countLdmUser(context);
  if (ConditionPassed(instr.ldStMulti.cc))
  {
    u8int Rn = instr.ldStMulti.Rn;
    bool wordhigher = (instr.ldStMulti.P == instr.ldStMulti.U);
    bool increment = instr.ldStMulti.U;
    u16int registers = instr.ldStMulti.regList;

    if ((Rn == GPR_PC) || (registers == 0))
      UNPREDICTABLE();

    u32int base = getGPRegister(context, Rn);
    u32int address = increment ? base : base - 4*countBitsSet(registers);
    if (wordhigher)
      address += 4;

    // force user bit set and no PC in list: LDM user mode registers
    CPSRmode savedMode = context->CPSR.bits.mode;
    context->CPSR.bits.mode = USR_MODE;
    int i = 0;
    for (i = 0; i < 15; i++)
    {
      if (((registers >> i) & 1) == 1)
      {
        setGPRegister(context, i, vmLoad(context, WORD, address));
        address += 4;
      }
    }
    // done loading, restore mode
    context->CPSR.bits.mode = savedMode;
  }
  return context->R15 + ARM_INSTRUCTION_SIZE;
}

u32int armLdmExcRetInstruction(GCONTXT *context, u32int instruction)
{
  Instruction instr = {.raw = instruction};
  DEBUG_TRACE(INTERPRETER_ARM_LOAD, context, instruction);
  countLdmExceptionReturn(context);
  if (ConditionPassed(instr.ldStMulti.cc))
  {
    u8int Rn = instr.ldStMulti.Rn;
    u16int registers = instr.ldStMulti.regList;
    bool wordhigher = (instr.ldStMulti.P == instr.ldStMulti.U);
    bool wback = instr.ldStMulti.W, increment = instr.ldStMulti.U;

    if (Rn == GPR_PC)
      UNPREDICTABLE();
    if (wback && (((registers >> Rn) & 1) == 1))
      UNPREDICTABLE();

    u32int base = getGPRegister(context, Rn);
    u32int length = 4*countBitsSet(registers) + 4;
    u32int address = increment ? base : base - length;
    if (wordhigher)
      address += 4;

    int i = 0;
    for (i = 0; i < 15; i++)
    {
      if (((registers >> i) & 1) == 1)
      {
        setGPRegister(context, i, vmLoad(context, WORD, address));
        address += 4;
      }
    }
    u32int newPcValue = vmLoad(context, WORD, address);

    if (wback)
    {
      if (increment)
        setGPRegister(context, Rn, base + length);
      else
        setGPRegister(context, Rn, base - length);
    }

    CPSRWriteByInstr(context, SPSR(context).value, 0xF, TRUE);
    BranchWritePC(context, newPcValue);
    return context->R15;
  }
  return context->R15 + ARM_INSTRUCTION_SIZE;
}
