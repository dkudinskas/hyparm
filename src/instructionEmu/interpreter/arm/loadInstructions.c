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
  if (!evaluateConditionCode(context, ARM_EXTRACT_CONDITION_CODE(instruction)))
  {
    return context->R15 + ARM_INSTRUCTION_SIZE;
  }

  DEBUG_TRACE(INTERPRETER_ARM_LOAD, context, instruction);

  u32int prePost = instruction & 0x01000000;
  u32int upDown = instruction & 0x00800000;
  u32int forceUser = instruction & 0x00400000;
  u32int writeback = instruction & 0x00200000;
  u32int baseReg = (instruction & 0x000F0000) >> 16;
  u32int regList = instruction & 0x0000FFFF;

  ASSERT(baseReg != GPR_PC, ERROR_UNPREDICTABLE_INSTRUCTION);
  ASSERT(regList, ERROR_UNPREDICTABLE_INSTRUCTION);

  u32int baseAddress = getGPRegister(context, baseReg);
  CPSRmode savedMode = 0;
  bool cpySpsr = FALSE;
  if (forceUser != 0)
  {
    // ok, is this exception return, or LDM user mode?
    if ((instruction & 0x00008000) != 0)
    {
      // force user bit set and PC in list: exception return
      cpySpsr = TRUE;
    }
    else
    {
      // force user bit set and no PC in list: LDM user mode registers
      savedMode = context->CPSR.bits.mode;
      context->CPSR.bits.mode = USR_MODE;
    }
  }

  u32int address = 0;
  if ((upDown == 0) && (prePost != 0)) // LDM decrement before
  {
    // address = baseAddress - 4*(number of registers to load);
    address = baseAddress - 4 * countBitsSet(regList);
  }
  else if ((upDown == 0) && (prePost == 0)) // LDM decrement after
  {
    // address = baseAddress - 4*(number of registers to load) + 4;
    address = baseAddress - 4 * countBitsSet(regList) + 4;
  }
  else if ((upDown != 0) && (prePost != 0)) // LDM increment before
  {
    // address = baseAddress + 4 - will be incremented as we go
    address = baseAddress + 4;
  }
  else if ((upDown != 0) && (prePost == 0)) // LDM increment after
  {
    // address = baseAddress - will be incremented as we go
    address = baseAddress;
  }

  bool isPCinRegList = FALSE;
  int i;
  for (i = 0; i < 16; i++)
  {
    // if current register set
    if (((regList >> i) & 0x1) == 0x1)
    {
      if (i == 15)
      {
        isPCinRegList = TRUE;
      }
      // R[i] = *(address);
      u32int valueLoaded = vmLoad(context, WORD, address);
      setGPRegister(context, i, valueLoaded);
      DEBUG(INTERPRETER_ARM_LOAD, "armLdmInstruction: R[%x] = *(%#.8x) = %#.8x" EOL, i, address,
          valueLoaded);
      address = address + 4;
    }
  } // for ends

  // if writeback then baseReg = baseReg +/- 4 * number of registers to load;
  if (writeback != 0)
  {
    if (upDown == 0)
    {
      // decrement
      baseAddress = baseAddress - 4 * countBitsSet(regList);
    }
    else
    {
      // increment
      baseAddress = baseAddress + 4 * countBitsSet(regList);
    }
    setGPRegister(context, baseReg, baseAddress);
  }

  if (forceUser != 0)
  {
    // do we need to copy spsr? or return from userland?
    if (cpySpsr)
    {
      // ok, exception return option: restore SPSR to CPSR
      // SPSR! which?... depends what mode we are in...
      u32int modeSpsr = 0;
      switch (context->CPSR.bits.mode)
      {
        case FIQ_MODE:
        {
          modeSpsr = context->SPSR_FIQ.value;
          break;
        }
        case IRQ_MODE:
        {
          modeSpsr = context->SPSR_IRQ.value;
          break;
        }
        case SVC_MODE:
        {
          modeSpsr = context->SPSR_SVC.value;
          break;
        }
        case ABT_MODE:
        {
          modeSpsr = context->SPSR_ABT.value;
          break;
        }
        case UND_MODE:
        {
          modeSpsr = context->SPSR_UND.value;
          break;
        }
        default:
          DIE_NOW(context, "exception return from sys/usr mode!");
      }
      if (context->CPSR.bits.mode != (modeSpsr & PSR_MODE))
      {
        guestChangeMode(context, modeSpsr & PSR_MODE);
      }
      context->CPSR.value = modeSpsr;
    }
    else
    {
      // we made CPSR user mode to force 'ldm user'. restore CPSR
      context->CPSR.bits.mode = savedMode;
    }
  }

  if (isPCinRegList)
  {
    /*
     * If PC is in the list this is an interworking branch.
     */
    if (context->R15 & 0x1)
    {
#ifdef CONFIG_THUMB2
      context->CPSR.bits.T = 1;
      context->R15 &= ~1;
#else
      DIE_NOW(context, "Thumb is disabled (CONFIG_THUMB2 not set)");
#endif
    }
    else
    {
      context->R15 &= ~2;
    }
    return context->R15;
  }

  return context->R15 + ARM_INSTRUCTION_SIZE;
}
