#include "common/bit.h"

#include "instructionEmu/interpreter/common.h"
#include "instructionEmu/interpreter/internals.h"
#include "instructionEmu/interpreter/arm/loadInstructions.h"

#include "guestManager/guestExceptions.h"

#include "memoryManager/memoryProtection.h"

/************************************************************/
/************************** BYTE loads***********************/
u32int armLdrbImmInstruction(GCONTXT* context, u32int instruction)
{
  Instruction instr = {.raw = instruction};
  DEBUG_TRACE(INTERPRETER_ARM_LOAD, context, instruction);

  if (ConditionPassed(instr.loadImm.cc))
  {
    u8int Rt = instr.loadImm.Rt, Rn = instr.loadImm.Rn;
    u32int imm32 = instr.loadImm.imm12 & 0xfff;
    bool index = instr.loadImm.P, add = instr.loadImm.U;
    bool wback = (!index) || instr.loadImm.W;
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

  if (ConditionPassed(instr.loadReg.cc))
  {
    u8int Rt = instr.loadReg.Rt, Rn = instr.loadReg.Rn, Rm = instr.loadReg.Rm;
    bool index = instr.loadReg.P, add = instr.loadReg.U;
    bool wback = (!index) || instr.loadReg.W;
    u32int shiftAmount = 0;
    // this call also sets shiftAmount
    u32int shiftType = decodeShiftImmediate(instr.loadReg.type, 
                             instr.loadReg.imm5, &shiftAmount);
    if ((Rt == GPR_PC) || (Rm == GPR_PC))
      UNPREDICTABLE();
    if (wback && ((Rn == GPR_PC) || (Rn == Rt)))
      UNPREDICTABLE();

    u32int base = getGPRegister(context, Rn);
    u32int offset = shiftVal(getGPRegister(context, Rm), shiftType, shiftAmount,
                                                          context->CPSR.bits.C);
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

  if (ConditionPassed(instr.loadImm.cc))
  {
    u8int Rt = instr.loadImm.Rt, Rn = instr.loadImm.Rn;
    u32int imm32 = instr.loadImm.imm12 & 0xfff;
    bool add = instr.loadImm.U;
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

  if (ConditionPassed(instr.loadReg.cc))
  {
    u8int Rt = instr.loadReg.Rt, Rn = instr.loadReg.Rn, Rm = instr.loadReg.Rm;
    bool add = instr.loadReg.U;
    u32int shiftAmount = 0;
    // this call also sets shiftAmount
    u32int shiftType = decodeShiftImmediate(instr.loadReg.type, 
                             instr.loadReg.imm5, &shiftAmount);
    if ((Rt == GPR_PC) || (Rn == GPR_PC) || (Rm == GPR_PC) || (Rt == Rn))
      UNPREDICTABLE();

    u32int address = getGPRegister(context, Rn);
    if (shouldDataAbort(context, FALSE, FALSE, address))
      return context->R15; // abort!

    u32int data = vmLoad(context, BYTE, address);
    u32int offset = shiftVal(getGPRegister(context, Rm), shiftType, shiftAmount,
                                                          context->CPSR.bits.C);
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

  if (ConditionPassed(instr.loadImm2.cc))
  {
    u8int Rt = instr.loadImm2.Rt, Rn = instr.loadImm2.Rn;
    u32int imm32 = instr.loadImm2.imm4H << 4 | instr.loadImm2.imm4L;
    bool index = instr.loadImm2.P, add = instr.loadImm2.U;
    bool wback = (!index) || instr.loadImm2.W;
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

  if (ConditionPassed(instr.loadReg2.cc))
  {
    u8int Rt = instr.loadReg2.Rt, Rn = instr.loadReg2.Rn, Rm = instr.loadReg2.Rm;
    bool index = instr.loadReg2.P, add = instr.loadReg2.U;
    bool wback = (!index) || instr.loadReg2.W;
    u32int shiftAmount = 0;
    // this call also sets shiftAmount
    u32int shiftType = decodeShiftImmediate(SHIFT_TYPE_LSL, 0, &shiftAmount);
    if ((Rt == GPR_PC) || (Rm == GPR_PC))
      UNPREDICTABLE();
    if (wback && ((Rn == GPR_PC) || (Rn == Rt)))
      UNPREDICTABLE();

    u32int base = getGPRegister(context, Rn);
    u32int offset = shiftVal(getGPRegister(context, Rm), shiftType, shiftAmount,
                                                          context->CPSR.bits.C);
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

  if (ConditionPassed(instr.loadReg2.cc))
  {
    u8int Rt = instr.loadReg2.Rt, Rn = instr.loadReg2.Rn;
    bool add = instr.loadReg2.U;
    u32int imm32 = instr.loadImm2.imm4H << 4 | instr.loadImm2.imm4L;

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

  if (ConditionPassed(instr.loadReg2.cc))
  {
    u8int Rt = instr.loadReg2.Rt, Rn = instr.loadReg2.Rn, Rm = instr.loadReg2.Rm;
    bool add = instr.loadReg2.U;

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

  if (ConditionPassed(instr.loadImm.cc))
  {
    u8int Rt = instr.loadImm.Rt, Rn = instr.loadImm.Rn;
    u32int imm32 = instr.loadImm.imm12 & 0xfff;
    bool index = instr.loadImm.P, add = instr.loadImm.U;
    bool wback = (!index) || instr.loadImm.W;
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

  if (ConditionPassed(instr.loadReg.cc))
  {
    u8int Rt = instr.loadReg.Rt, Rn = instr.loadReg.Rn, Rm = instr.loadReg.Rm;
    bool index = instr.loadReg.P, add = instr.loadReg.U;
    bool wback = (!index) || instr.loadReg.W;
    u32int shiftAmount = 0;
    // this call also sets shiftAmount
    u32int shiftType = decodeShiftImmediate(instr.loadReg.type, 
                             instr.loadReg.imm5, &shiftAmount);
    if (Rm == GPR_PC)
      UNPREDICTABLE();
    if (wback && ((Rn == GPR_PC) || (Rn == Rt)))
      UNPREDICTABLE();


    u32int base = getGPRegister(context, Rn);
    u32int offset = shiftVal(getGPRegister(context, Rm), shiftType, shiftAmount,
                                                          context->CPSR.bits.C);
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

  if (ConditionPassed(instr.loadImm.cc))
  {
    u8int Rt = instr.loadImm.Rt, Rn = instr.loadImm.Rn;
    u32int imm32 = instr.loadImm.imm12 & 0xfff;
    bool add = instr.loadImm.U;
    if ((Rt == GPR_PC) || (Rn == GPR_PC) || (Rt == Rn))
      UNPREDICTABLE();

    u32int address = getGPRegister(context, Rn);
    if (shouldDataAbort(context, FALSE, FALSE, address))
      return context->R15; // abort!

    u32int data = vmLoad(context, WORD, address);
    setGPRegister(context, Rn, add ? (address + imm32) : (address - imm32));

    setGPRegister(context, Rt, data);
  }
  return context->R15 + ARM_INSTRUCTION_SIZE;
}


u32int armLdrtRegInstruction(GCONTXT* context, u32int instruction)
{
  Instruction instr = {.raw = instruction};
  DEBUG_TRACE(INTERPRETER_ARM_LOAD, context, instruction);

  if (ConditionPassed(instr.loadReg.cc))
  {
    u8int Rt = instr.loadReg.Rt, Rn = instr.loadReg.Rn, Rm = instr.loadReg.Rm;
    bool add = instr.loadReg.U;

    u32int shiftAmount = 0;
    // this call also sets shiftAmount
    u32int shiftType = decodeShiftImmediate(instr.loadReg.type, 
                             instr.loadReg.imm5, &shiftAmount);

    if ((Rt == GPR_PC) || (Rn == GPR_PC) || (Rt == Rn) || (Rm == GPR_PC))
      UNPREDICTABLE();

    u32int address = getGPRegister(context, Rn);
    if (shouldDataAbort(context, FALSE, FALSE, address))
      return context->R15; // abort!

    u32int data = vmLoad(context, WORD, address);
    u32int offset = shiftVal(getGPRegister(context, Rm), shiftType, shiftAmount,
                                                          context->CPSR.bits.C);
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

  if (ConditionPassed(instr.loadReg.cc))
  {
    u8int Rt = instr.loadImm2.Rt, Rt2 = Rt + 1, Rn = instr.loadImm2.Rn;
    bool index = instr.loadImm2.P, add = instr.loadImm2.U;
    bool wback = (!index) || (instr.loadImm2.W);
    u32int imm32 = instr.loadImm.imm12 & 0xfff;

    if ((Rt & 1) == 1)
      UNPREDICTABLE();
    if ((!index) && (instr.loadImm2.W))
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

  if (ConditionPassed(instr.loadReg.cc))
  {
    u8int Rt = instr.loadReg2.Rt, Rt2 = Rt + 1;
    u8int Rm = instr.loadReg2.Rm, Rn = instr.loadReg2.Rn;
    bool index = instr.loadReg2.P, add = instr.loadReg2.U;
    bool wback = (!index) || (instr.loadReg2.W);

    if ((Rt & 1) == 1)
      UNPREDICTABLE();
    if ((!index) && (instr.loadImm2.W))
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

  if (ConditionPassed(instr.ldm.cc))
  {
    u8int Rn = instr.ldm.Rn;
    bool before = instr.ldm.P, increment = instr.ldm.U, wback = instr.ldm.W;
    u16int registers = instr.ldm.regList;

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

  if (ConditionPassed(instr.ldm.cc))
  {
    u8int Rn = instr.ldm.Rn;
    bool wordhigher = instr.ldm.P == instr.ldm.U, increment = instr.ldm.U;
    u16int registers = instr.ldm.regList;

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

  if (ConditionPassed(instr.ldm.cc))
  {
    u8int Rn = instr.ldm.Rn;
    u16int registers = instr.ldm.regList;
    bool wordhigher = (instr.ldm.P == instr.ldm.U);
    bool wback = instr.ldm.W, increment = instr.ldm.U;

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
