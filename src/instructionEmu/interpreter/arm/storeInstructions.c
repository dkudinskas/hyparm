#include "common/bit.h"
#include "common/debug.h"

#include "cpuArch/constants.h"

#include "instructionEmu/commonInstrFunctions.h"

#include "instructionEmu/interpreter/storeInstructions.h"


u32int armStrInstruction(GCONTXT *context, u32int instruction)
{
  if (!evaluateConditionCode(context, ARM_EXTRACT_CONDITION_CODE(instruction)))
  {
    return getRealPC(context) + ARM_INSTRUCTION_SIZE;
  }

  u32int regOrImm = instruction & 0x02000000; // 1 = reg, 0 = imm
  u32int preOrPost = instruction & 0x01000000; // 1 = pre, 0 = post
  u32int incOrDec = instruction & 0x00800000; // 1 = inc, 0 = dec
  u32int writeBack = instruction & 0x00200000; // 1 = writeBack indexing, 0 = no writeback
  u32int regDst = (instruction & 0x000F0000) >> 16; // Base Destination address
  u32int regSrc = (instruction & 0x0000F000) >> 12; // Source value from this register...

#ifdef DATA_MOVE_TRACE
  printf("strInstr: regOrImm=%x preOrPost=%x incOrDec=%x writeBack=%x regdest=%x regsrc=%x" EOL,
      regOrImm, preOrPost, incOrDec, writeBack, regDst, regSrc);
#endif

  u32int baseAddress = loadGuestGPR(regDst, context);
  u32int valueToStore = loadGuestGPR(regSrc, context);
  u32int offsetAddress;

  if (regOrImm == 0)
  {
#ifdef DATA_MOVE_TRACE
    printf("strInstr: imm case: ");
#endif
    // immediate case
    u32int imm32 = instruction & 0x00000FFF;

#ifdef DATA_MOVE_TRACE
    printf("imm32=%x baseAddress=%#.8x valueToStore=%x offsetAddress=%#.8x",
        imm32, baseAddress, valueToStore, offsetAddress);
#endif

    // offsetAddress = if increment then base + imm32 else base - imm32
    if (incOrDec != 0)
    {
      offsetAddress = baseAddress + imm32;
#ifdef DATA_MOVE_TRACE
      printf(" inc" EOL);
#endif
    }
    else
    {
      offsetAddress = baseAddress - imm32;
#ifdef DATA_MOVE_TRACE
      printf(" dec" EOL);
#endif
    }
  } // Immediate case ends
  else
  {
#ifdef DATA_MOVE_TRACE
    printf("strInstr: reg case: ");
#endif
    // register case
    u32int regDst2 = instruction & 0x0000000F;

    u32int offsetRegisterValue = loadGuestGPR(regDst2, context);
#ifdef DATA_MOVE_TRACE
    printf("regDst2=%x, baseAddress=%#.8x, offsetRegisterValue=%x, valueToStore=%x" EOL,
        regDst2, baseAddress, offsetRegisterValue, valueToStore);
#endif

    if (regDst2 == GPR_PC)
    {
      DIE_NOW(0, "STR reg Rm == PC UNPREDICTABLE case!");
    }

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

  if ((address & 0x3) != 0x0)
  {
    DIE_NOW(context, "STR Rd [Rn, Rm/#imm] unaligned address!");
  }

  // P = 0 and W == 1 then STR as if user mode
  if (preOrPost == 0 && writeBack != 0 && shouldDataAbort(FALSE, TRUE, address))
  {
    return getRealPC(context);
  }

  // *storeAddress = if sourceValue is PC then valueToStore+8 else valueToStore;
  valueToStore = (regSrc == GPR_PC) ? (valueToStore + 8) : valueToStore;
  context->hardwareLibrary->storeFunction(context->hardwareLibrary, WORD, address, valueToStore);

  // wback = (P = 0) or (W = 1)
  bool wback = !preOrPost || writeBack;
  if (wback)
  {
    //if Rn == PC || n == t) then UNPREDICTABLE;
    if (regDst == GPR_PC || regDst == regSrc)
    {
      DIE_NOW(0, "STR writeback UNPREDICTABLE case!");
    }
    // Rn = offsetAddr;
    storeGuestGPR(regDst, offsetAddress, context);
  }
  return getRealPC(context) + ARM_INSTRUCTION_SIZE;
}

u32int armStrbInstruction(GCONTXT * context, u32int instruction)
{
  if (!evaluateConditionCode(context, ARM_EXTRACT_CONDITION_CODE(instruction)))
  {
    return getRealPC(context) + ARM_INSTRUCTION_SIZE;
  }

  u32int regOrImm = instruction & 0x02000000; // 1 = reg, 0 = imm
  u32int preOrPost = instruction & 0x01000000; // 1 = pre, 0 = post
  u32int incOrDec = instruction & 0x00800000; // 1 = inc, 0 = dec
  u32int writeBack = instruction & 0x00200000; // 1 = writeBack indexing, 0 = no writeback
  u32int regDst = (instruction & 0x000F0000) >> 16; // Base Destination address
  u32int regSrc = (instruction & 0x0000F000) >> 12; // Source value from this register...
  u32int offsetAddress;
  u32int baseAddress;
  u32int valueToStore;

  if (regSrc == GPR_PC)
  {
    DIE_NOW(0, "STRB source register PC UNPREDICTABLE case.");
  }
  if (!regOrImm)
  {
    // immediate case
    u32int imm32 = instruction & 0x00000FFF;
    baseAddress = loadGuestGPR(regDst, context);
    valueToStore = loadGuestGPR(regSrc, context) & 0xFF;
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
    baseAddress = loadGuestGPR(regDst, context);
    u32int offsetRegisterValue = loadGuestGPR(regDst2, context);
    valueToStore = loadGuestGPR(regSrc, context) & 0xFF;
    // regDest2 == PC then UNPREDICTABLE
    if (regDst2 == 15)
    {
      DIE_NOW(0, "STRB reg Rm == PC UNPREDICTABLE case!");
    }

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
  if (!preOrPost && writeBack && shouldDataAbort(FALSE, TRUE, address))
  {
    return getRealPC(context);
  }

  // *storeAddress = if sourceValue is PC then valueToStore+8 else valueToStore;
  context->hardwareLibrary->storeFunction(context->hardwareLibrary, BYTE, address, (valueToStore & 0xFF));

  // wback = (P = 0) or (W = 1)
  bool wback = !preOrPost || writeBack;
  if (wback)
  {
    //if Rn == PC || n == t) then UNPREDICTABLE;
    if ((regDst == 15) || (regDst == regSrc))
    {
      DIE_NOW(0, "STRB writeback UNPREDICTABLE case!");
    }
    // Rn = offsetAddr;
    storeGuestGPR(regDst, offsetAddress, context);
  }
  return getRealPC(context) + ARM_INSTRUCTION_SIZE;
}

u32int armStrhInstruction(GCONTXT *context, u32int instruction)
{
  if (!evaluateConditionCode(context, ARM_EXTRACT_CONDITION_CODE(instruction)))
  {
    return getRealPC(context) + ARM_INSTRUCTION_SIZE;
  }

  u32int preOrPost = instruction & 0x01000000; // 1 = pre, 0 = post
  u32int incOrDec = instruction & 0x00800000; // 1 = inc, 0 = dec
  u32int regOrImm = instruction & 0x00400000; // 1 = reg, 0 = imm
  u32int writeBack = instruction & 0x00200000; // 1 = writeBack indexing, 0 = no writeback
  u32int regDst = (instruction & 0x000F0000) >> 16; // Destination address
  u32int regSrc = (instruction & 0x0000F000) >> 12; // Source value from this register...

  // P = 0 and W == 1 then STR as if user mode
  if (!preOrPost && writeBack)
  {
    DIE_NOW(0, "STRH as user mode unimplemented.");
  }

  if (regSrc == GPR_PC)
  {
    DIE_NOW(0, "STRH source register PC UNPREDICTABLE case.");
  }

  u32int offsetAddress;
  u32int baseAddress;
  u32int valueToStore;

  if (regOrImm != 0)
  {
    // immediate case
    u32int imm4Top = instruction & 0x00000F00;
    u32int imm4Bottom = instruction & 0x0000000F;
    u32int imm32 = (imm4Top >> 4) | imm4Bottom; // imm field to +/- offset

    baseAddress = loadGuestGPR(regDst, context);
    valueToStore = loadGuestGPR(regSrc, context);

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
    u32int offsetRegisterValue = loadGuestGPR(regDst2, context);
    baseAddress = loadGuestGPR(regDst, context);
    valueToStore = loadGuestGPR(regSrc, context);
    // regDest2 == PC then UNPREDICTABLE
    if (regDst2 == 15)
    {
      DIE_NOW(0, "STRH reg Rm == PC UNPREDICTABLE case!");
    }

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

  if (address & 0x1)
  {
    DIE_NOW(context, "STRH Rd [Rn, Rm/#imm] unaligned address!");
  }

  context->hardwareLibrary->storeFunction(context->hardwareLibrary, HALFWORD, address, valueToStore);

  // wback = (P = 0) or (W = 1)
  bool wback = !preOrPost || writeBack;
  if (wback)
  {
    //if Rn == PC || n == t) then UNPREDICTABLE;
    if ((regDst == 15) || (regDst == regSrc))
    {
      DIE_NOW(0, "STRH writeback UNPREDICTABLE case!");
    }
    // Rn = offsetAddr;
    storeGuestGPR(regDst, offsetAddress, context);
  }
  return getRealPC(context) + ARM_INSTRUCTION_SIZE;
}

u32int armStrdInstruction(GCONTXT *context, u32int instruction)
{
  if (!evaluateConditionCode(context, ARM_EXTRACT_CONDITION_CODE(instruction)))
  {
    return getRealPC(context) + ARM_INSTRUCTION_SIZE;
  }

  u32int prePost = instruction & 0x01000000;
  u32int upDown = instruction & 0x00800000;
  u32int regOrImm = instruction & 0x00400000;
  u32int writeback = instruction & 0x00200000; // 0 = reg, !0 = imm
  u32int regDst = (instruction & 0x000F0000) >> 16;
  u32int regSrc = (instruction & 0x0000F000) >> 12;
  u32int regSrc2 = regSrc + 1;

#ifdef DATA_MOVE_TRACE
    printf("STRD instruction: %#.8x @ PC = %#.8x" EOL, instruction, getRealPC(context));
#endif

  if ((regSrc % 2) == 1)
  {
    DIE_NOW(0, "STRD undefined case: regSrc must be even number!");
  }

  u32int offsetAddress = 0;
  u32int baseAddress = loadGuestGPR(regDst, context);
  u32int valueToStore = loadGuestGPR(regSrc, context);
  u32int valueToStore2 = loadGuestGPR(regSrc2, context);

  u32int wback = (prePost == 0) || (writeback != 0);

  // P = 0 and W == 1 then STR as if user mode
  if ((prePost == 0) && (writeback != 0))
  {
    DIE_NOW(0, "STRD unpredictable case (P=0 AND W=1)!");
  }

  if (wback && ((regDst == 15) || (regDst == regSrc) || (regDst == regSrc2)))
  {
    DIE_NOW(0, "STRD unpredictable register selection!");
  }
  if (regSrc2 == 15)
  {
    DIE_NOW(0, "STRD: unpredictable case, regSrc2 = PC!");
  }

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
#ifdef DATA_MOVE_TRACE
    printf("imm32=%x baseAddress=%#.8x valueToStore=%x offsetAddress=%#.8x" EOL,
        imm32, baseAddress, valueToStore, offsetAddress);
#endif
  } // Immediate case ends
  else
  {
    // register case
    u32int regDst2 = instruction & 0x0000000F;
    u32int offsetRegisterValue = loadGuestGPR(regDst2, context);
    // regDest2 == PC then UNPREDICTABLE
    if (regDst2 == GPR_PC)
    {
      DIE_NOW(0, "STR reg Rm == PC UNPREDICTABLE case!");
    }

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
#ifdef DATA_MOVE_TRACE
    printf("Rm=%x baseAddress=%x valueToStore=%x offsetRegisterValue=%x" EOL,
        regDst2, baseAddress, valueToStore, offsetRegisterValue);
#endif
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
#ifdef DATA_MOVE_TRACE
  printf("store address = %#.8x", address);
#endif
  // *storeAddress = if sourceValue is PC then valueToStore+8 else valueToStore;
  if (regSrc == GPR_PC)
  {
    valueToStore += 8;
  }
#ifdef DATA_MOVE_TRACE
  printf("store val1 = %x store val2 = %x" EOL, valueToStore, valueToStore2);
#endif

  context->hardwareLibrary->storeFunction(context->hardwareLibrary, WORD, address, valueToStore);
  context->hardwareLibrary->storeFunction(context->hardwareLibrary, WORD, address + 4, valueToStore2);

  if (wback)
  {
    // Rn = offsetAddr;
    storeGuestGPR(regDst, offsetAddress, context);
  }
  return getRealPC(context) + ARM_INSTRUCTION_SIZE;
}

u32int armStrhtInstruction(GCONTXT *context, u32int instruction)
{
  if (!evaluateConditionCode(context, ARM_EXTRACT_CONDITION_CODE(instruction)))
  {
    return getRealPC(context) + ARM_INSTRUCTION_SIZE;
  }

  DIE_NOW(context, "armStrhtInstruction not implemented");
}

u32int armStrexInstruction(GCONTXT *context, u32int instruction)
{
  if (!evaluateConditionCode(context, ARM_EXTRACT_CONDITION_CODE(instruction)))
  {
    return getRealPC(context) + ARM_INSTRUCTION_SIZE;
  }

#ifdef DATA_MOVE_TRACE
  printf("STREX instruction: %#.8x @ PC = %#.8x" EOL, instruction, getRealPC(context));
#endif

  u32int regN = (instruction & 0x000F0000) >> 16;
  u32int regD = (instruction & 0x0000F000) >> 12;
  u32int regT = (instruction & 0x0000000F);

  if ((regN == GPR_PC) || (regD == GPR_PC) || (regT == GPR_PC))
  {
    DIE_NOW(0, "STREX unpredictable case (PC used)");
  }
  if ((regD == regN) || (regD == regT))
  {
    DIE_NOW(0, "STREX unpredictable case (invalid register use)");
  }

  u32int address = loadGuestGPR(regN, context);
  u32int valToStore = loadGuestGPR(regT, context);
#ifdef DATA_MOVE_TRACE
  printf("STREX instruction: address = %#.8x, valToStore = %x, valueFromReg %x" EOL,
      address, valToStore, regT);
#endif

  context->hardwareLibrary->storeFunction(context->hardwareLibrary, WORD, address, valToStore);
  // operation succeeded updating memory, flag regD (0 - updated, 1 - fail)
  storeGuestGPR(regD, 0, context);

  return getRealPC(context) + ARM_INSTRUCTION_SIZE;
}

u32int armStrexbInstruction(GCONTXT *context, u32int instruction)
{
  if (!evaluateConditionCode(context, ARM_EXTRACT_CONDITION_CODE(instruction)))
  {
    return getRealPC(context) + ARM_INSTRUCTION_SIZE;
  }

#ifdef DATA_MOVE_TRACE
  printf("STREXB instruction: %#.8x @ PC = %#.8x" EOL, instruction, getRealPC(context));
#endif

  u32int regN = (instruction & 0x000F0000) >> 16;
  u32int regD = (instruction & 0x0000F000) >> 12;
  u32int regT = (instruction & 0x0000000F);

  if ((regN == GPR_PC) || (regD == GPR_PC) || (regT == GPR_PC))
  {
    DIE_NOW(0, "STREX unpredictable case (PC used)");
  }
  if ((regD == regN) || (regD == regT))
  {
    DIE_NOW(0, "STREX unpredictable case (invalid register use)");
  }

  u32int address = loadGuestGPR(regN, context);

  u32int valToStore = loadGuestGPR(regT, context);
  context->hardwareLibrary->storeFunction(context->hardwareLibrary, BYTE, address, (valToStore & 0xFF));

  storeGuestGPR(regD, 0, context);

  return getRealPC(context) + ARM_INSTRUCTION_SIZE;
}

u32int armStrexhInstruction(GCONTXT *context, u32int instruction)
{
#ifdef CONFIG_BLOCK_COPY
  DIE_NOW(context, "strexhInstruction is executed but not yet checked for blockCopyCompatibility");
#endif

  if (!evaluateConditionCode(context, ARM_EXTRACT_CONDITION_CODE(instruction)))
  {
    return getRealPC(context) + ARM_INSTRUCTION_SIZE;
  }

#ifdef DATA_MOVE_TRACE
  printf("STREXH instruction: %#.8x @ PC = %#.8x" EOL, instruction, getRealPC(context));
#endif

  u32int condcode = (instruction & 0xF0000000) >> 28;
  u32int regN = (instruction & 0x000F0000) >> 16;
  u32int regD = (instruction & 0x0000F000) >> 12;
  u32int regT = (instruction & 0x0000000F);

  if (!evaluateConditionCode(context, condcode))
  {
    // condition not met! allright, we're done here. next instruction...
    return getRealPC(context) + 4;
  }

  if ((regN == 15) || (regD == 15) || (regT == 15))
  {
    DIE_NOW(0, "STREX unpredictable case (PC used)");
  }
  if ((regD == regN) || (regD == regT))
  {
    DIE_NOW(0, "STREX unpredictable case (invalid register use)");
  }

  u32int address = loadGuestGPR(regN, context);

  u32int valToStore = loadGuestGPR(regT, context);

  context->hardwareLibrary->storeFunction(context->hardwareLibrary, HALFWORD, address, (valToStore & 0xFFFF));
  storeGuestGPR(regD, 0, context);

  return getRealPC(context) + ARM_INSTRUCTION_SIZE;
}

u32int armStrexdInstruction(GCONTXT *context, u32int instruction)
{
#ifdef CONFIG_BLOCK_COPY
  DIE_NOW(context, "strexdInstruction is executed but not yet checked for blockCopyCompatibility");
#endif

  if (!evaluateConditionCode(context, ARM_EXTRACT_CONDITION_CODE(instruction)))
  {
    return getRealPC(context) + ARM_INSTRUCTION_SIZE;
  }

#ifdef DATA_MOVE_TRACE
  printf("STREXD instruction: %#.8x @ PC = %#.8x" EOL, instruction, getRealPC(context));
#endif

  u32int condcode = (instruction & 0xF0000000) >> 28;
  u32int regN = (instruction & 0x000F0000) >> 16;
  u32int regD = (instruction & 0x0000F000) >> 12;
  u32int regT = (instruction & 0x0000000F);

  if (!evaluateConditionCode(context, condcode))
  {
    // condition not met! allright, we're done here. next instruction...
    return getRealPC(context) + 4;
  }

  if (regD == GPR_PC || (regT % 2) || regT == GPR_LR || regN == GPR_PC || regD == regN
      || regD == regT || regD == (regT + 1))
  {
    DIE_NOW(0, "STREXD unpredictable case");
  }

  u32int address = loadGuestGPR(regN, context);

  // Create doubleword to store such that R[t] will be stored at addr and R[t2] at addr+4.
  u32int valToStore1 = loadGuestGPR(regT, context);
  u32int valToStore2 = loadGuestGPR(regT + 1, context);

  /*
   * FIXME STREXD: assuming littl endian
   */
  DIE_NOW(0, "STREXD: assuming littlendian!");

  bool littleEndian = TRUE;
  if (littleEndian)
  {
    context->hardwareLibrary->storeFunction(context->hardwareLibrary, WORD, address, valToStore2);
    context->hardwareLibrary->storeFunction(context->hardwareLibrary, WORD, address + 4, valToStore1);
  }
  else
  {
    context->hardwareLibrary->storeFunction(context->hardwareLibrary, WORD, address, valToStore1);
    context->hardwareLibrary->storeFunction(context->hardwareLibrary, WORD, address + 4, valToStore2);
  }
  storeGuestGPR(regD, 0, context);

  return getRealPC(context) + ARM_INSTRUCTION_SIZE;
}

u32int armStmInstruction(GCONTXT *context, u32int instruction)
{
  if (!evaluateConditionCode(context, ARM_EXTRACT_CONDITION_CODE(instruction)))
  {
    return getRealPC(context) + ARM_INSTRUCTION_SIZE;
  }

#ifdef DATA_MOVE_TRACE
    printf("STM instruction: %#.8x @ PC = %#.8x" EOL, instruction, getRealPC(context));
#endif


  u32int prePost = instruction & 0x01000000;
  u32int upDown = instruction & 0x00800000;
  u32int forceUser = instruction & 0x00400000;
  u32int writeback = instruction & 0x00200000;

  u32int baseReg = (instruction & 0x000F0000) >> 16;
  u32int regList = instruction & 0x0000FFFF;
  u32int baseAddress = loadGuestGPR(baseReg, context);

  u32int savedCPSR = 0;
  if (forceUser != 0)
  {
    // force user bit set: STM user mode registers
    savedCPSR = context->CPSR;
    context->CPSR = (context->CPSR & ~0x1f) | PSR_USR_MODE;
  }

  u32int address = 0;
  if (!upDown && prePost) // STM decrement before
  {
    // address = baseAddress - 4*(number of registers to store);
    address = baseAddress - 4 * countBitsSet(regList);
  }
  else if (!upDown && !prePost) // STM decrement after
  {
    // address = baseAddress - 4*(number of registers to store) + 4;
    address = baseAddress - 4 * countBitsSet(regList) + 4;
  }
  else if (upDown && prePost) // STM increment before
  {
    // address = baseAddress + 4 - will be incremented as we go
    address = baseAddress + 4;
  }
  else if (upDown && !prePost) // STM increment after
  {
    // address = baseAddress - will be incremented as we go
    address = baseAddress;
  }

  int i;
  for (i = 0; i < 15; i++)
  {
    // if current register set
    if (((regList >> i) & 0x1) == 0x1)
    {
#ifdef DATA_MOVE_TRACE
      printf("*(%#.8x) = R[%x] = %#.8x" EOL, address, i, loadGuestGPR(i, context));
#endif
      u32int valueLoaded = loadGuestGPR(i, context);
      // emulating store. Validate cache if needed
      validateCachePreChange(context->blockCache, address);
      // *(address)= R[i];
      context->hardwareLibrary->storeFunction(context->hardwareLibrary, WORD, address, valueLoaded);
      address = address + 4;
    }
  } // for ends
  // if store PC...
  if ((regList >> 15) & 0x1)
  {
    // emulating store. Validate cache if needed
    validateCachePreChange(context->blockCache, address);
    // *(address)= PC+8 - architectural feature due to pipeline..
    context->hardwareLibrary->storeFunction(context->hardwareLibrary, WORD, address, (loadGuestGPR(15, context) + 8));
  }

  // if writeback then baseReg = baseReg - 4 * number of registers to store;
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
    storeGuestGPR(baseReg, baseAddress, context);
  }

  // if we stored to user mode registers, lets restore the CPSR
  if (forceUser != 0)
  {
    context->CPSR = savedCPSR;
  }

  return getRealPC(context) + ARM_INSTRUCTION_SIZE;
}
