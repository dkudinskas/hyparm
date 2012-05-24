#include "common/bit.h"

#include "instructionEmu/interpreter/internals.h"

#include "instructionEmu/interpreter/arm/storeInstructions.h"

#include "memoryManager/memoryProtection.h"


u32int armStrInstruction(GCONTXT *context, u32int instruction)
{
  if (!evaluateConditionCode(context, ARM_EXTRACT_CONDITION_CODE(instruction)))
  {
    return getNativeInstructionPointer(context) + ARM_INSTRUCTION_SIZE;
  }

  DEBUG_TRACE(INTERPRETER_ARM_STORE, context, instruction);

  u32int regOrImm = instruction & 0x02000000; // 1 = reg, 0 = imm
  u32int preOrPost = instruction & 0x01000000; // 1 = pre, 0 = post
  u32int incOrDec = instruction & 0x00800000; // 1 = inc, 0 = dec
  u32int writeBack = instruction & 0x00200000; // 1 = writeBack indexing, 0 = no writeback
  u32int regDst = (instruction & 0x000F0000) >> 16; // Base Destination address
  u32int regSrc = (instruction & 0x0000F000) >> 12; // Source value from this register...

  DEBUG(INTERPRETER_ARM_STORE, "armStrInstruction: regOrImm=%x preOrPost=%x incOrDec=%x writeBack="
      "%x regdest=%x regsrc=%x" EOL, regOrImm, preOrPost, incOrDec, writeBack, regDst, regSrc);

  u32int baseAddress = getGPRegister(context, regDst);
  u32int valueToStore = getGPRegister(context, regSrc);
  u32int offsetAddress;

  if (regOrImm == 0)
  {
    // immediate case
    u32int imm32 = instruction & 0x00000FFF;

    // offsetAddress = if increment then base + imm32 else base - imm32
    if (incOrDec != 0)
    {
      offsetAddress = baseAddress + imm32;
    }
    else
    {
      offsetAddress = baseAddress - imm32;
    }

    DEBUG(INTERPRETER_ARM_STORE, "armStrInstruction: imm32=%x baseAddress=%#.8x valueToStore=%x "
        "offsetAddress=%#.8x" EOL, imm32, baseAddress, valueToStore, offsetAddress);
  } // Immediate case ends
  else
  {
    // register case
    u32int regDst2 = instruction & 0x0000000F;
    ASSERT(regDst2 != GPR_PC, ERROR_UNPREDICTABLE_INSTRUCTION);
    u32int offsetRegisterValue = getGPRegister(context, regDst2);

    DEBUG(INTERPRETER_ARM_STORE, "armStrInstruction: regDst2=%x, baseAddress=%#.8x, "
        "offsetRegisterValue=%x, valueToStore=%x" EOL, regDst2, baseAddress, offsetRegisterValue,
        valueToStore);

    // (shift_t, shift_n) = DecodeImmShift(type, imm5)
    u32int shiftAmount = 0;
    u32int shiftType = decodeShiftImmediate(((instruction & 0x060) >> 5),
    ((instruction & 0xF80)>>7), &shiftAmount);
    u8int carryFlag = (context->CPSR & 0x20000000) >> 29;

    // offset = Shift(offsetRegisterValue, shiftType, shitAmount, cFlag);
    u32int offset = shiftVal(offsetRegisterValue, shiftType, shiftAmount, &carryFlag);

    // if increment then base + offset else base - offset
    if (incOrDec != 0)
    {
      // increment
      offsetAddress = baseAddress + offset;
    }
    else
    {
      // decrement
      offsetAddress = baseAddress - offset;
    }
  } // Register case ends

  u32int address = 0;
  // if preIndex then use offsetAddress else baseAddress
  if (preOrPost != 0)
  {
    address = offsetAddress;
  }
  else
  {
    address = baseAddress;
  }

  ASSERT((address & 0x3) == 0, "Rd [Rn, Rm/#imm] unaligned address!");

  // P = 0 and W == 1 then STR as if user mode
  if (preOrPost == 0 && writeBack != 0 && shouldDataAbort(context, FALSE, TRUE, address))
  {
    return getNativeInstructionPointer(context);
  }

  // *storeAddress = if sourceValue is PC then valueToStore+8 else valueToStore;
  vmStore(context, WORD, address, valueToStore);

  // wback = (P = 0) or (W = 1)
  bool wback = !preOrPost || writeBack;
  if (wback)
  {
    //if Rn == PC || n == t) then UNPREDICTABLE;
    ASSERT(regDst != GPR_PC, ERROR_UNPREDICTABLE_INSTRUCTION);
    ASSERT(regDst != regSrc, ERROR_UNPREDICTABLE_INSTRUCTION);
    // Rn = offsetAddr;
    setGPRegister(context, regDst, offsetAddress);
  }
  return getNativeInstructionPointer(context) + ARM_INSTRUCTION_SIZE;
}

u32int armStrbInstruction(GCONTXT * context, u32int instruction)
{
  if (!evaluateConditionCode(context, ARM_EXTRACT_CONDITION_CODE(instruction)))
  {
    return getNativeInstructionPointer(context) + ARM_INSTRUCTION_SIZE;
  }

  DEBUG_TRACE(INTERPRETER_ARM_STORE, context, instruction);

  u32int regOrImm = instruction & 0x02000000; // 1 = reg, 0 = imm
  u32int preOrPost = instruction & 0x01000000; // 1 = pre, 0 = post
  u32int incOrDec = instruction & 0x00800000; // 1 = inc, 0 = dec
  u32int writeBack = instruction & 0x00200000; // 1 = writeBack indexing, 0 = no writeback
  u32int regDst = (instruction & 0x000F0000) >> 16; // Base Destination address
  u32int regSrc = (instruction & 0x0000F000) >> 12; // Source value from this register...
  u32int offsetAddress;
  u32int baseAddress;
  u32int valueToStore;

  ASSERT(regSrc != GPR_PC, ERROR_UNPREDICTABLE_INSTRUCTION);

  baseAddress = getGPRegister(context, regDst);

  if (!regOrImm)
  {
    // immediate case
    u32int imm32 = instruction & 0x00000FFF;
    valueToStore = getGPRegister(context, regSrc) & 0xFF;
    // offsetAddress = if increment then base + imm32 else base - imm32
    if (incOrDec)
    {
      offsetAddress = baseAddress + imm32;
    }
    else
    {
      offsetAddress = baseAddress - imm32;
    }
  } // Immediate case ends
  else
  {
    // register case
    u32int regDst2 = instruction & 0x0000000F;
    ASSERT(regDst2 != GPR_PC, ERROR_UNPREDICTABLE_INSTRUCTION);
    u32int offsetRegisterValue = getGPRegister(context, regDst2);
    valueToStore = getGPRegister(context, regSrc) & 0xFF;

    // (shift_t, shift_n) = DecodeImmShift(type, imm5)
    u32int shiftAmount = 0;
    u32int shiftType = decodeShiftImmediate(((instruction & 0x060) >> 5),
    ((instruction & 0xF80)>>7), &shiftAmount);
    u8int carryFlag = (context->CPSR & 0x20000000) >> 29;

    // offset = Shift(offsetRegisterValue, shiftType, shitAmount, cFlag);
    u32int offset = shiftVal(offsetRegisterValue, shiftType, shiftAmount, &carryFlag);

    // if increment then base + offset else base - offset
    if (incOrDec)
    {
      // increment
      offsetAddress = baseAddress + offset;
    }
    else
    {
      // decrement
      offsetAddress = baseAddress - offset;
    }
  } // Register case ends

  u32int address = 0;
  // if preIndex then use offsetAddress else baseAddress
  if (preOrPost)
  {
    address = offsetAddress;
  }
  else
  {
    address = baseAddress;
  }

  // P = 0 and W == 1 then STR as if user mode -- only continue if usr can write
  if (!preOrPost && writeBack && shouldDataAbort(context, FALSE, TRUE, address))
  {
    return getNativeInstructionPointer(context);
  }

  // *storeAddress = if sourceValue is PC then valueToStore+8 else valueToStore;
  vmStore(context, BYTE, address, valueToStore & 0xFF);

  // wback = (P = 0) or (W = 1)
  bool wback = !preOrPost || writeBack;
  if (wback)
  {
    //if Rn == PC || n == t) then UNPREDICTABLE;
    ASSERT(regDst != GPR_PC, ERROR_UNPREDICTABLE_INSTRUCTION);
    ASSERT(regDst != regSrc, ERROR_UNPREDICTABLE_INSTRUCTION);
    // Rn = offsetAddr;
    setGPRegister(context, regDst, offsetAddress);
  }
  return getNativeInstructionPointer(context) + ARM_INSTRUCTION_SIZE;
}

u32int armStrhInstruction(GCONTXT *context, u32int instruction)
{
  if (!evaluateConditionCode(context, ARM_EXTRACT_CONDITION_CODE(instruction)))
  {
    return getNativeInstructionPointer(context) + ARM_INSTRUCTION_SIZE;
  }

  DEBUG_TRACE(INTERPRETER_ARM_STORE, context, instruction);

  u32int preOrPost = instruction & 0x01000000; // 1 = pre, 0 = post
  u32int incOrDec = instruction & 0x00800000; // 1 = inc, 0 = dec
  u32int regOrImm = instruction & 0x00400000; // 1 = reg, 0 = imm
  u32int writeBack = instruction & 0x00200000; // 1 = writeBack indexing, 0 = no writeback
  u32int regDst = (instruction & 0x000F0000) >> 16; // Destination address
  u32int regSrc = (instruction & 0x0000F000) >> 12; // Source value from this register...

  // P = 0 and W == 1 then STR as if user mode
  if (!preOrPost && writeBack)
  {
    DIE_NOW(context, ERROR_NOT_IMPLEMENTED);
  }

  ASSERT(regSrc != GPR_PC, ERROR_UNPREDICTABLE_INSTRUCTION);

  u32int offsetAddress;
  u32int baseAddress;
  u32int valueToStore;

  baseAddress = getGPRegister(context, regDst);
  valueToStore = getGPRegister(context, regSrc);

  if (regOrImm != 0)
  {
    // immediate case
    u32int imm4Top = instruction & 0x00000F00;
    u32int imm4Bottom = instruction & 0x0000000F;
    u32int imm32 = (imm4Top >> 4) | imm4Bottom; // imm field to +/- offset

    // offsetAddress = if increment then base + imm32 else base - imm32
    if (incOrDec != 0)
    {
      offsetAddress = baseAddress + imm32;
    }
    else
    {
      offsetAddress = baseAddress - imm32;
    }
  } // immediate case done
  else
  {
    // register case
    u32int regDst2 = instruction & 0x0000000F;
    ASSERT(regDst2 != GPR_PC, ERROR_UNPREDICTABLE_INSTRUCTION);
    u32int offsetRegisterValue = getGPRegister(context, regDst2);

    // (shift_t, shift_n) = (SRType_LSL, 0);
    u32int shiftAmount = 0;
    u32int shiftType = decodeShiftImmediate(0, 0, &shiftAmount);
    u8int carryFlag = (context->CPSR & 0x20000000) >> 29;

    // offset = Shift(offsetRegisterValue, shiftType, shitAmount, cFlag);
    u32int offset = shiftVal(offsetRegisterValue, shiftType, shiftAmount, &carryFlag);

    // if increment then base + offset else base - offset
    if (incOrDec != 0)
    {
      // increment
      offsetAddress = baseAddress + offset;
    }
    else
    {
      // decrement
      offsetAddress = baseAddress - offset;
    }
  } // reg case done

  u32int address = 0;
  // if preIndex then use offsetAddress else baseAddress
  if (preOrPost != 0)
  {
    address = offsetAddress;
  }
  else
  {
    address = baseAddress;
  }

  ASSERT((address & 0x1) == 0, "Rd [Rn, Rm/#imm] unaligned address!");

  vmStore(context, HALFWORD, address, valueToStore & 0xFFFF);

  // wback = (P = 0) or (W = 1)
  bool wback = !preOrPost || writeBack;
  if (wback)
  {
    //if Rn == PC || n == t) then UNPREDICTABLE;
    ASSERT(regDst != GPR_PC, ERROR_UNPREDICTABLE_INSTRUCTION);
    ASSERT(regDst != regSrc, ERROR_UNPREDICTABLE_INSTRUCTION);
    // Rn = offsetAddr;
    setGPRegister(context, regDst, offsetAddress);
  }
  return getNativeInstructionPointer(context) + ARM_INSTRUCTION_SIZE;
}

u32int armStrdInstruction(GCONTXT *context, u32int instruction)
{
  if (!evaluateConditionCode(context, ARM_EXTRACT_CONDITION_CODE(instruction)))
  {
    return getNativeInstructionPointer(context) + ARM_INSTRUCTION_SIZE;
  }

  DEBUG_TRACE(INTERPRETER_ARM_STORE, context, instruction);

  u32int prePost = instruction & 0x01000000;
  u32int upDown = instruction & 0x00800000;
  u32int regOrImm = instruction & 0x00400000;
  u32int writeback = instruction & 0x00200000; // 0 = reg, !0 = imm
  u32int regDst = ARM_EXTRACT_REGISTER(instruction, 16);
  u32int regSrc = ARM_EXTRACT_REGISTER(instruction, 12);
  u32int regSrc2 = regSrc + 1;
  u32int wback = (prePost == 0) || (writeback != 0);

  ASSERT(!(regSrc & 1), ERROR_UNPREDICTABLE_INSTRUCTION);
  ASSERT(regSrc != GPR_LR, ERROR_UNPREDICTABLE_INSTRUCTION);
  ASSERT(prePost || !writeback, ERROR_BAD_ARGUMENTS); // P = 0 and W == 1 then STR as if user mode
  ASSERT(!wback || (regDst != GPR_PC && regDst != regSrc && regDst != regSrc2), ERROR_UNPREDICTABLE_INSTRUCTION);

  u32int offsetAddress = 0;
  u32int baseAddress = getGPRegister(context, regDst);
  u32int valueToStore = getGPRegister(context, regSrc);
  u32int valueToStore2 = getGPRegister(context, regSrc2);

  if (regOrImm != 0)
  {
    // immediate case
    u32int imm4h = (instruction & 0x00000f00) >> 4;
    u32int imm4l = (instruction & 0x0000000f);
    u32int imm32 = imm4h | imm4l;

    // offsetAddress = if increment then base + imm32 else base - imm32
    if (upDown != 0)
    {
      offsetAddress = baseAddress + imm32;
    }
    else
    {
      offsetAddress = baseAddress - imm32;
    }
    DEBUG(INTERPRETER_ARM_STORE, "armStrdInstruction: imm32=%x baseAddress=%#.8x valueToStore=%x "
        "offsetAddress=%#.8x" EOL, imm32, baseAddress, valueToStore, offsetAddress);
  } // Immediate case ends
  else
  {
    // register case
    u32int regDst2 = instruction & 0x0000000F;
    ASSERT(regDst2 != GPR_PC, ERROR_UNPREDICTABLE_INSTRUCTION);
    u32int offsetRegisterValue = getGPRegister(context, regDst2);

    // if increment then base + offset else base - offset
    if (upDown != 0)
    {
      // increment
      offsetAddress = baseAddress + offsetRegisterValue;
    }
    else
    {
      // decrement
      offsetAddress = baseAddress - offsetRegisterValue;
    }
    DEBUG(INTERPRETER_ARM_STORE, "armStrdInstruction: Rm=%x baseAddress=%x valueToStore=%x "
        "offsetRegisterValue=%x" EOL, regDst2, baseAddress, valueToStore, offsetRegisterValue);
  } // Register case ends

  u32int address;
  // if preIndex then use offsetAddress else baseAddress
  if (prePost)
  {
    address = offsetAddress;
  }
  else
  {
    address = baseAddress;
  }

  DEBUG(INTERPRETER_ARM_STORE, "armStrdInstruction: store address = %#.8x, values %#.8x %#.8x" EOL,
      address, valueToStore, valueToStore2);

  vmStore(context, WORD, address, valueToStore);
  vmStore(context, WORD, address+4, valueToStore2);

  if (wback)
  {
    // Rn = offsetAddr;
    setGPRegister(context, regDst, offsetAddress);
  }
  return getNativeInstructionPointer(context) + ARM_INSTRUCTION_SIZE;
}


u32int armStrtInstruction(GCONTXT *context, u32int instruction)
{
  return armStrInstruction(context, instruction);
}


u32int armStrhtInstruction(GCONTXT *context, u32int instruction)
{
  return armStrhInstruction(context, instruction);
}


u32int armStrbtInstruction(GCONTXT *context, u32int instruction)
{
  return armStrbInstruction(context, instruction);
}

u32int armStmInstruction(GCONTXT *context, u32int instruction)
{
  if (!evaluateConditionCode(context, ARM_EXTRACT_CONDITION_CODE(instruction)))
  {
    return getNativeInstructionPointer(context) + ARM_INSTRUCTION_SIZE;
  }

  DEBUG_TRACE(INTERPRETER_ARM_STORE, context, instruction);

  u32int prePost = instruction & 0x01000000;
  u32int upDown = instruction & 0x00800000;
  u32int forceUser = instruction & 0x00400000;
  u32int writeback = instruction & 0x00200000;

  u32int baseReg = ARM_EXTRACT_REGISTER(instruction, 16);
  ASSERT(baseReg != GPR_PC, ERROR_UNPREDICTABLE_INSTRUCTION);

  u32int regList = instruction & 0x0000FFFF;
  ASSERT(regList, ERROR_UNPREDICTABLE_INSTRUCTION);

  u32int baseAddress = getGPRegister(context, baseReg);

  u32int savedCPSR = 0;
  if (forceUser != 0)
  {
    // force user bit set: STM user mode registers
    savedCPSR = context->CPSR;
    context->CPSR = (context->CPSR & ~0x1f) | PSR_USR_MODE;
  }

  u32int address = baseAddress;
  if (!upDown && prePost) // STM decrement before
  {
    // address = baseAddress - 4*(number of registers to store);
    address -= 4 * countBitsSet(regList);
  }
  else if (!upDown && !prePost) // STM decrement after
  {
    // address = baseAddress - 4*(number of registers to store) + 4;
    address -= 4 * countBitsSet(regList) - 4;
  }
  else if (upDown && prePost) // STM increment before
  {
    // address = baseAddress + 4 - will be incremented as we go
    address += 4;
  }

  for (int i = 0; i <= GPR_PC; i++)
  {
    // if current register set
    if (((regList >> i) & 0x1) == 0x1)
    {
      u32int valueLoaded = getGPRegister(context, i);
      DEBUG(INTERPRETER_ARM_STORE, "armStmInstruction: *(%#.8x) = R[%x] = %#.8x" EOL, address, i,
          valueLoaded);
      // emulating store. Validate cache if needed
      clearTranslationCacheByAddress(&context->translationCache, address);
      // *(address)= R[i];
      vmStore(context, WORD, address, valueLoaded);
      address += 4;
    }
  } // for ends

  // if writeback then baseReg = baseReg - 4 * number of registers to store;
  if (writeback != 0)
  {
    if (upDown == 0)
    {
      // decrement
      baseAddress -= 4 * countBitsSet(regList);
    }
    else
    {
      // increment
      baseAddress += 4 * countBitsSet(regList);
    }
    setGPRegister(context, baseReg, baseAddress);
  }

  // if we stored to user mode registers, lets restore the CPSR
  if (forceUser)
  {
    context->CPSR = savedCPSR;
  }

  return getNativeInstructionPointer(context) + ARM_INSTRUCTION_SIZE;
}
