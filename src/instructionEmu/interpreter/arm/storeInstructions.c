#include "common/bit.h"

#include "instructionEmu/interpreter/common.h"
#include "instructionEmu/interpreter/internals.h"
#include "instructionEmu/interpreter/arm/storeInstructions.h"

#include "memoryManager/memoryProtection.h"

#include "perf/contextSwitchCounters.h"

/************************************************************/
/************************** BYTE stores *********************/
u32int armStrbImmInstruction(GCONTXT *context, Instruction instr)
{
  DEBUG_TRACE(INTERPRETER_ARM_STORE, context, instr.raw);
  countStrbImm(&(context->counters));
  if (ConditionPassed(instr.ldStImm.cc))
  {
    u8int Rt = instr.ldStImm.Rt, Rn = instr.ldStImm.Rn;
    u32int imm32 = instr.ldStImm.imm12 & 0xfff;
    bool index = instr.ldStImm.P, add = instr.ldStImm.U;
    bool wback = (!index) || instr.ldStImm.W;
    if (Rt == GPR_PC)
      UNPREDICTABLE();
    if (wback && ((Rn == GPR_PC) || (Rt == Rn)))
      UNPREDICTABLE();

    u32int base = getGPRegister(context, Rn);
    u32int offsetAddr = add ? (base + imm32) : (base - imm32);
    u32int address = index ? offsetAddr : base;

    vmStore(context, BYTE, address, getGPRegister(context, Rt) & 0xFF);

    if (wback)
      setGPRegister(context, Rn, offsetAddr);
  }
  return context->R15 + ARM_INSTRUCTION_SIZE;
}


u32int armStrbRegInstruction(GCONTXT *context, Instruction instr)
{
  DEBUG_TRACE(INTERPRETER_ARM_STORE, context, instr.raw);
  countStrbReg(&(context->counters));
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

    vmStore(context, BYTE, address, getGPRegister(context, Rt) & 0xFF);

    if (wback)
      setGPRegister(context, Rn, offsetAddr);

  }
  return context->R15 + ARM_INSTRUCTION_SIZE;
}


u32int armStrbtImmInstruction(GCONTXT *context, Instruction instr)
{
  DEBUG_TRACE(INTERPRETER_ARM_STORE, context, instr.raw);
  countStrbtImm(&(context->counters));
  if (ConditionPassed(instr.ldStImm.cc))
  {
    u8int Rt = instr.ldStImm.Rt, Rn = instr.ldStImm.Rn;
    bool add = instr.ldStImm.U;
    if ((Rt == GPR_PC) || (Rn == GPR_PC) || (Rt == Rn))
      UNPREDICTABLE();

    u32int address = getGPRegister(context, Rn);
    u32int offset = instr.ldStImm.imm12 & 0xfff;

    if (shouldDataAbort(context, FALSE, TRUE, address))
      return context->R15; // abort!

    vmStore(context, BYTE, address, getGPRegister(context, Rt) & 0xFF);
    // writeback
    setGPRegister(context, Rn, add ? (address + offset) : (address - offset));
  }
  return context->R15 + ARM_INSTRUCTION_SIZE;
}


u32int armStrbtRegInstruction(GCONTXT *context, Instruction instr)
{
  DEBUG_TRACE(INTERPRETER_ARM_STORE, context, instr.raw);
  countStrbtReg(&(context->counters));
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
    u32int offset = shiftVal(getGPRegister(context, Rm), shiftType, shiftAmount);
    if (shouldDataAbort(context, FALSE, TRUE, address))
      return context->R15; // abort!

    vmStore(context, BYTE, address, getGPRegister(context, Rt) & 0xFF);
    // writeback address
    setGPRegister(context, Rn, add ? (address + offset) : (address - offset));
  }
  return context->R15 + ARM_INSTRUCTION_SIZE;
}


/****************************************************************/
/************************** HALFWORD stores *********************/
u32int armStrhImmInstruction(GCONTXT *context, Instruction instr)
{
  DEBUG_TRACE(INTERPRETER_ARM_STORE, context, instr.raw);
  countStrhImm(&(context->counters));
  if (ConditionPassed(instr.ldStImm2.cc))
  {
    u8int Rt = instr.ldStImm2.Rt, Rn = instr.ldStImm2.Rn;
    u32int imm32 = (instr.ldStImm2.imm4H << 4) | instr.ldStImm2.imm4L;
    bool index = instr.ldStImm2.P, add = instr.ldStImm2.U;
    bool wback = (!index) || instr.ldStImm2.W;
    if (Rt == GPR_PC)
      UNPREDICTABLE();
    if (wback && ((Rn == GPR_PC) || (Rt == Rn)))
      UNPREDICTABLE();

    u32int base = getGPRegister(context, Rn);
    u32int offsetAddr = add ? (base + imm32) : (base - imm32);
    u32int address = index ? offsetAddr : base;

    vmStore(context, HALFWORD, address, getGPRegister(context, Rt) & 0xFFFF);

    if (wback)
      setGPRegister(context, Rn, offsetAddr);
  }
  return context->R15 + ARM_INSTRUCTION_SIZE;
}


u32int armStrhRegInstruction(GCONTXT *context, Instruction instr)
{
  DEBUG_TRACE(INTERPRETER_ARM_STORE, context, instr.raw);
  countStrhReg(&(context->counters));
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

    vmStore(context, HALFWORD, address, getGPRegister(context, Rt) & 0xFFFF);

    if (wback)
      setGPRegister(context, Rn, offsetAddr);

  }
  return context->R15 + ARM_INSTRUCTION_SIZE;
}


u32int armStrhtImmInstruction(GCONTXT *context, Instruction instr)
{
  DEBUG_TRACE(INTERPRETER_ARM_STORE, context, instr.raw);
  countStrhtImm(&(context->counters));
  if (ConditionPassed(instr.ldStImm2.cc))
  {
    u8int Rt = instr.ldStImm2.Rt, Rn = instr.ldStImm2.Rn;
    bool add = instr.ldStImm2.U;
    u32int imm32 = (instr.ldStImm2.imm4H << 4) | instr.ldStImm2.imm4L;

    if ((Rt == GPR_PC) || (Rn == GPR_PC) || (Rn == Rt))
      UNPREDICTABLE();

    u32int address = getGPRegister(context, Rn);

    if (shouldDataAbort(context, FALSE, TRUE, address))
      return context->R15; // abort!

    vmStore(context, HALFWORD, address, getGPRegister(context, Rt) & 0xFFFF);
    // writeback address
    setGPRegister(context, Rn, add ? (address + imm32) : (address - imm32));
  }
  return context->R15 + ARM_INSTRUCTION_SIZE;
}


u32int armStrhtRegInstruction(GCONTXT *context, Instruction instr)
{
  DEBUG_TRACE(INTERPRETER_ARM_STORE, context, instr.raw);
  countStrhtReg(&(context->counters));
  if (ConditionPassed(instr.ldStReg2.cc))
  {
    u8int Rt = instr.ldStReg2.Rt, Rn = instr.ldStReg2.Rn, Rm = instr.ldStReg2.Rm;
    bool add = instr.ldStReg2.U;

    if ((Rt == GPR_PC) || (Rn == GPR_PC) || (Rm == GPR_PC) || (Rn == Rt))
      UNPREDICTABLE();

    u32int address = getGPRegister(context, Rn);
    u32int offset = getGPRegister(context, Rm);

    if (shouldDataAbort(context, FALSE, TRUE, address))
      return context->R15; // abort!

    vmStore(context, HALFWORD, address, getGPRegister(context, Rt) & 0xFFFF);
    // writeback address
    setGPRegister(context, Rn, add ? (address + offset) : (address - offset));
  }
  return context->R15 + ARM_INSTRUCTION_SIZE;
}


/****************************************************************/
/************************** WORD stores *************************/
u32int armStrImmInstruction(GCONTXT* context, Instruction instr)
{
  DEBUG_TRACE(INTERPRETER_ARM_STORE, context, instr.raw);
  countStrImm(&(context->counters));
  if (ConditionPassed(instr.ldStImm.cc))
  {
    u8int Rt = instr.ldStImm.Rt, Rn = instr.ldStImm.Rn;
    u32int imm32 = instr.ldStImm.imm12;
    bool index = instr.ldStImm.P, add = instr.ldStImm.U;
    bool wback = (!index) || instr.ldStImm.W;
    if (wback && ((Rn == GPR_PC) || (Rt == Rn)))
      UNPREDICTABLE();

    u32int base = getGPRegister(context, Rn);
    u32int offsetAddr = add ? (base + imm32) : (base - imm32);
    u32int address = index ? offsetAddr : base;

    vmStore(context, WORD, address, getGPRegister(context, Rt));

    if (wback)
      setGPRegister(context, Rn, offsetAddr);
  }
  return context->R15 + ARM_INSTRUCTION_SIZE;
}


u32int armStrRegInstruction(GCONTXT *context, Instruction instr)
{
  DEBUG_TRACE(INTERPRETER_ARM_LOAD, context, instr.raw);
  countStrReg(&(context->counters));
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

    vmStore(context, WORD, address, getGPRegister(context, Rt));

    if (wback)
      setGPRegister(context, Rn, offsetAddr);
  }
  return context->R15 + ARM_INSTRUCTION_SIZE;
}


u32int armStrtImmInstruction(GCONTXT *context, Instruction instr)
{
  DEBUG_TRACE(INTERPRETER_ARM_STORE, context, instr.raw);
  countStrtImm(&(context->counters));
  if (ConditionPassed(instr.ldStImm.cc))
  {
    u8int Rt = instr.ldStImm.Rt, Rn = instr.ldStImm.Rn;
    bool add = instr.ldStImm.U;

    if ((Rt == GPR_PC) || (Rn == GPR_PC) || (Rt == Rn))
      UNPREDICTABLE();

    u32int offset = instr.ldStImm.imm12 & 0xfff;
    u32int address = getGPRegister(context, Rn);

    if (shouldDataAbort(context, FALSE, TRUE, address))
      return context->R15; // abort!

    vmStore(context, WORD, address, getGPRegister(context, Rt));
    // writeback
    setGPRegister(context, Rn, add ? (address + offset) : (address - offset));
  }
  return context->R15 + ARM_INSTRUCTION_SIZE;
}


u32int armStrtRegInstruction(GCONTXT *context, Instruction instr)
{
  DEBUG_TRACE(INTERPRETER_ARM_STORE, context, instr.raw);
  countStrtReg(&(context->counters));
  if (ConditionPassed(instr.ldStReg.cc))
  {
    u8int Rt = instr.ldStReg.Rt, Rn = instr.ldStReg.Rn, Rm = instr.ldStReg.Rm;
    bool add = instr.ldStReg.U;

    u32int shiftAmount = 0;
    // this call also sets shiftAmount
    u32int shiftType = decodeShiftImmediate(instr.ldStReg.type, 
                                            instr.ldStReg.imm5, &shiftAmount);

    if ((Rn == GPR_PC) || (Rt == Rn) || (Rm == GPR_PC))
      UNPREDICTABLE();

    u32int offset = shiftVal(getGPRegister(context, Rm), shiftType, shiftAmount);
    u32int address = getGPRegister(context, Rn);

    if (shouldDataAbort(context, FALSE, TRUE, address))
      return context->R15; // abort!

    vmStore(context, WORD, address, getGPRegister(context, Rt));
    // writeback
    setGPRegister(context, Rn, add ? (address + offset) : (address - offset));
  }
  return context->R15 + ARM_INSTRUCTION_SIZE;
}


/**************************************************************/
/************************** DUAL WORD stores ******************/
u32int armStrdImmInstruction(GCONTXT* context, Instruction instr)
{
  DEBUG_TRACE(INTERPRETER_ARM_STORE, context, instr.raw);
  countStrdImm(&(context->counters));
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
    if (wback && ((Rn == GPR_PC) || (Rn == Rt) || (Rn == Rt2)))
      UNPREDICTABLE();
    if (Rt2 == GPR_PC)
      UNPREDICTABLE();

    u32int base = getGPRegister(context, Rn);
    u32int offsetAddr = add ? (base + imm32) : (base - imm32);
    u32int address = index ? offsetAddr : base;    

    vmStore(context, WORD, address,   getGPRegister(context, Rt ));
    vmStore(context, WORD, address+4, getGPRegister(context, Rt2));

    if (wback)
      setGPRegister(context, Rn, offsetAddr);
  }
  return context->R15 + ARM_INSTRUCTION_SIZE;
}


u32int armStrdRegInstruction(GCONTXT* context, Instruction instr)
{
  DEBUG_TRACE(INTERPRETER_ARM_STORE, context, instr.raw);
  countStrdReg(&(context->counters));
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
    if ((Rt2 == GPR_PC) || (Rm == GPR_PC))
      UNPREDICTABLE();
    if (wback && ((Rn == GPR_PC) || (Rn == Rt) || (Rn == Rt2)))
      UNPREDICTABLE();

    u32int base = getGPRegister(context, Rn);
    u32int offset = getGPRegister(context, Rm);
    u32int offsetAddr = add ? (base + offset) : (base - offset);
    u32int address = index ? offsetAddr : base;    

    vmStore(context, WORD, address,   getGPRegister(context, Rt ));
    vmStore(context, WORD, address+4, getGPRegister(context, Rt2));

    if (wback)
      setGPRegister(context, Rn, offsetAddr);
  }
  return context->R15 + ARM_INSTRUCTION_SIZE;
}


/**************************************************************/
/************************** multiword stores ******************/
u32int armStmInstruction(GCONTXT *context, Instruction instr)
{
  DEBUG_TRACE(INTERPRETER_ARM_STORE, context, instr.raw);
  countStm(&(context->counters));
  if (ConditionPassed(instr.ldStMulti.cc))
  {
    u8int Rn = instr.ldStMulti.Rn;
    u16int registers = instr.ldStMulti.regList;
    bool before = instr.ldStMulti.P, increment = instr.ldStMulti.U;
    bool wback = instr.ldStMulti.W;

    if ((Rn == GPR_PC) || (registers == 0))
      UNPREDICTABLE();

    u32int bitCount = countBitsSet(registers);
    u32int base = getGPRegister(context, Rn);
    u32int address = 0;

    if (increment && before)        // STMIB
      address = base + 4;
    else if (!increment && before)  // STMDB
      address = base - (4 * bitCount);
    else if (!increment && !before) // STMDA
      address = base - (4 * bitCount) + 4;
    else                            // STMIA
      address = base;

    int i = 0;
    for (i = 0; i < 16; i++)
    {
      if (((registers >> i) & 1) == 1)
      {
        u32int data = getGPRegister(context, i);
        // emulating store. Validate cache if needed
        clearTranslationsByAddress(context->translationStore, address);
        vmStore(context, WORD, address, data);
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
  }
  return context->R15 + ARM_INSTRUCTION_SIZE;
}


u32int armStmUserInstruction(GCONTXT *context, Instruction instr)
{
  DEBUG_TRACE(INTERPRETER_ARM_STORE, context, instr.raw);
  countStmUser(&(context->counters));
  if (ConditionPassed(instr.ldStMulti.cc))
  {
    u8int Rn = instr.ldStMulti.Rn;
    u16int registers = instr.ldStMulti.regList;
    bool increment = instr.ldStMulti.U;
    bool wordhigher = (instr.ldStMulti.P == instr.ldStMulti.U);

    if ((Rn == GPR_PC) || (registers == 0))
      UNPREDICTABLE();

    u32int base = getGPRegister(context, Rn);
    u32int address = increment ? base : base - 4*countBitsSet(registers);
    if (wordhigher)
      address += 4;

    // STM user mode registers
    CPSRmode savedMode = context->CPSR.bits.mode;
    context->CPSR.bits.mode = USR_MODE;
    int i = 0;
    for (i = 0; i < 16; i++)
    {
      if (((registers >> i) & 1) == 1)
      {
        u32int data = getGPRegister(context, i);
        // emulating store. Validate cache if needed
        clearTranslationsByAddress(context->translationStore, address);
        vmStore(context, WORD, address, data);
        address += 4;
      }
    }
    // done storing, restore mode
    context->CPSR.bits.mode = savedMode;
  }
  return context->R15 + ARM_INSTRUCTION_SIZE;
}
