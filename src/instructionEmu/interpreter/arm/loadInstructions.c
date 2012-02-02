#include "common/bit.h"

#include "instructionEmu/interpreter/internals.h"

#include "instructionEmu/interpreter/arm/loadInstructions.h"


u32int armLdrInstruction(GCONTXT *context, u32int instruction)
{
  if (!evaluateConditionCode(context, ARM_EXTRACT_CONDITION_CODE(instruction)))
  {
    return context->R15 + ARM_INSTRUCTION_SIZE;
  }

  DEBUG_TRACE(INTERPRETER_ARM_LOAD, context, instruction);

  u32int regOrImm = instruction & 0x02000000; // 1 = reg, 0 = imm
  u32int preOrPost = instruction & 0x01000000; // 1 = pre, 0 = post
  u32int incOrDec = instruction & 0x00800000; // 1 = inc, 0 = dec
  u32int writeBack = instruction & 0x00200000; // 1 = writeBack indexing, 0 = no writeback
  u32int regSrc = (instruction & 0x000F0000) >> 16; // Base Load address
  u32int regDst = (instruction & 0x0000F000) >> 12; // Destination - load to this
  u32int offsetAddress = 0;
  u32int baseAddress = 0;

  if (!regOrImm)
  {
    // immediate case
    u32int imm32 = instruction & 0x00000FFF;
    baseAddress = loadGuestGPR(regSrc, context);
    if (regSrc == GPR_PC)
    {
      baseAddress += 8;
    }

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
    u32int regSrc2 = instruction & 0x0000000F;
    baseAddress = loadGuestGPR(regSrc, context);
    if (regSrc == GPR_PC)
    {
      baseAddress += 8;
    }
    // regSrc2 == PC then UNPREDICTABLE
    if (regSrc2 == GPR_PC)
    {
      DIE_NOW(context, "reg Rm == PC UNPREDICTABLE case!");
    }
    u32int offsetRegisterValue = loadGuestGPR(regSrc2, context);

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

  if ((address & 0x3) != 0x0)
  {
    DIE_NOW(context, "Rd [Rn, Rm/#imm] unaligned address!");
  }

  // P = 0 and W == 1 then LDR as if user mode
  if ((preOrPost == 0) && (writeBack != 0))
  {
    bool abort = shouldDataAbort(FALSE, FALSE, address);
    if (abort)
    {
      return context->R15;
    }
  }

  // DO the actual load from memory
  u32int valueLoaded = vmLoad(WORD, address);

  // LDR loading to PC should load a word-aligned value
  if ((regDst == 15) && ((valueLoaded & 0x3) != 0))
  {
    printf("LDR: regDst = %x, load from addr %#.8x" EOL, regDst, valueLoaded);
    DIE_NOW(context, "Rd [Rn, Rm/#imm] load unaligned value to PC!");
  }
  // put loaded val to reg
  storeGuestGPR(regDst, valueLoaded, context);

  // wback = (P = 0) or (W = 1)
  bool wback = !preOrPost || writeBack;
  if (wback)
  {
    // if Rn == Rt then UNPREDICTABLE
    if (regDst == regSrc)
    {
      DIE_NOW(context, "writeback UNPREDICTABLE case!");
    }
    // Rn = offsetAddr;
    storeGuestGPR(regSrc, offsetAddress, context);
  }
  if (regDst == GPR_PC)
  {
    return context->R15;
  }
  else
  {
    return context->R15 + ARM_INSTRUCTION_SIZE;
  }
}

u32int armLdrbInstruction(GCONTXT *context, u32int instruction)
{
  if (!evaluateConditionCode(context, ARM_EXTRACT_CONDITION_CODE(instruction)))
  {
    return context->R15 + ARM_INSTRUCTION_SIZE;
  }

  DEBUG_TRACE(INTERPRETER_ARM_LOAD, context, instruction);

  u32int offset = 0;
  u32int offsetAddress = 0;

  u32int regOrImm = instruction & 0x02000000; // 1 = reg, 0 = imm
  u32int preOrPost = instruction & 0x01000000; // 1 = pre, 0 = post
  u32int incOrDec = instruction & 0x00800000; // 1 = inc, 0 = dec
  u32int writeBack = instruction & 0x00200000; // 1 = writeBack indexing, 0 = no writeback
  u32int regSrc = (instruction & 0x000F0000) >> 16; // Base Load address
  u32int regDst = (instruction & 0x0000F000) >> 12; // Destination - load to this

  u32int baseAddress = loadGuestGPR(regSrc, context);

  if (regDst == GPR_PC)
  {
    DIE_NOW(context, "cannot load a single byte into PC!");
  }

  if (!regOrImm)
  {
    if (regSrc == GPR_PC)
    {
      DIE_NOW(context, "check LDRB literal");
    }
    // immediate case
    offset = instruction & 0x00000FFF;

  } // Immediate case ends
  else
  {
    // register case
    u32int regSrc2 = instruction & 0x0000000F;
    if (regSrc2 == 15)
    {
      DIE_NOW(context, "reg Rm == PC UNPREDICTABLE case!");
    }
    u32int offsetRegisterValue = loadGuestGPR(regSrc2, context);

    // (shift_t, shift_n) = DecodeImmShift(type, imm5)
    u32int shiftAmount = 0;
    u32int shiftType = decodeShiftImmediate(((instruction & 0x060) >> 5),
    ((instruction & 0xF80)>>7), &shiftAmount);
    u8int carryFlag = (context->CPSR & 0x20000000) >> 29;

    // offset = Shift(offsetRegisterValue, shiftType, shitAmount, cFlag);
    offset = shiftVal(offsetRegisterValue, shiftType, shiftAmount, &carryFlag);

  } // Register case ends

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

  // P = 0 and W == 1 then LDRB as if user mode
  if (!preOrPost && writeBack && shouldDataAbort(FALSE, FALSE, address))
  {
    return context->R15;
  }

  // DO the actual load from memory
  u32int valueLoaded = vmLoad(BYTE, address) & 0xFF;

  // put loaded val to reg
  storeGuestGPR(regDst, valueLoaded, context);

  // wback = (P = 0) or (W = 1)
  bool wback = !preOrPost || writeBack;
  if (wback)
  {
    // if Rn == Rt then UNPREDICTABLE
    if (regDst == regSrc)
    {
      DIE_NOW(context, "writeback UNPREDICTABLE case!");
    }
    // Rn = offsetAddr;
    storeGuestGPR(regSrc, offsetAddress, context);
  }
  return context->R15 + ARM_INSTRUCTION_SIZE;
}

u32int armLdrhInstruction(GCONTXT *context, u32int instruction)
{
  if (!evaluateConditionCode(context, ARM_EXTRACT_CONDITION_CODE(instruction)))
  {
    return context->R15 + ARM_INSTRUCTION_SIZE;
  }

  DEBUG_TRACE(INTERPRETER_ARM_LOAD, context, instruction);

  u32int preOrPost = instruction & 0x01000000; // 1 = pre, 0 = post
  u32int incOrDec = instruction & 0x00800000; // 1 = inc, 0 = dec
  u32int regOrImm = instruction & 0x00400000; // 1 = reg, 0 = imm
  u32int writeBack = instruction & 0x00200000; // 1 = writeBack indexing, 0 = no writeback
  u32int regSrc = (instruction & 0x000F0000) >> 16; // Source value from this register...
  u32int regDst = (instruction & 0x0000F000) >> 12; // Destination address

  // P = 0 and W == 1 then LDRHT (as if user mode)
  if (!preOrPost && writeBack)
  {
    DIE_NOW(context, "as user mode unimplemented.");
  }
  if (regDst == GPR_PC)
  {
    // cannot load halfword into PC!!
    DIE_NOW(context, "Rd=PC UNPREDICTABLE case.");
  }

  u32int baseAddress = loadGuestGPR(regSrc, context);
  ;
  u32int offsetAddress;
  u32int address;

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

    // if preIndex then use offsetAddress else baseAddress
    if (preOrPost != 0)
    {
      address = offsetAddress;
    }
    else
    {
      address = baseAddress;
    }
    if ((address & 0x1) == 0x1)
    {
      DIE_NOW(context, "load address unaligned.");
    }
  } // immediate case done
  else
  {
    // register case
    u32int regSrc2 = instruction & 0x0000000F;
    if (regSrc2 == 15)
    {
      DIE_NOW(context, "reg Rm == PC UNPREDICTABLE case!");
    }
    u32int offsetRegisterValue = loadGuestGPR(regSrc2, context);

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

    // if preIndex then use offsetAddress else baseAddress
    if (preOrPost != 0)
    {
      address = offsetAddress;
    }
    else
    {
      address = baseAddress;
    }
    if ((address & 0x1) == 0x1)
    {
      DIE_NOW(context, "load address unaligned.");
    }
  } // reg case done

  u32int valueLoaded = vmLoad(HALFWORD, address) & 0xFFFF;

  // put loaded val to reg
  storeGuestGPR(regDst, valueLoaded, context);

  DEBUG(INTERPRETER_ARM_LOAD, "armLdrhInstruction: R[%x]=%#.8x" EOL, regDst, valueLoaded);

  // wback = (P = 0) or (W = 1)
  bool wback = (preOrPost == 0) || (writeBack != 0);
  if (wback)
  {
    //if Rn == PC || Rn == Rt || Rn == Rm) then UNPREDICTABLE;
    if (regDst == regSrc)
    {
      DIE_NOW(context, "writeback UNPREDICTABLE case!");
    }
    // Rn = offsetAddr;
    storeGuestGPR(regSrc, offsetAddress, context);
  }

  return context->R15 + ARM_INSTRUCTION_SIZE;
}

u32int armLdrdInstruction(GCONTXT *context, u32int instruction)
{
  if (!evaluateConditionCode(context, ARM_EXTRACT_CONDITION_CODE(instruction)))
  {
    return context->R15 + ARM_INSTRUCTION_SIZE;
  }

  DEBUG_TRACE(INTERPRETER_ARM_LOAD, context, instruction);

  u32int prePost = instruction & 0x01000000;
  u32int upDown = instruction & 0x00800000;
  u32int regOrImm = instruction & 0x00400000; // 0 = reg, 1 = imm
  u32int writeback = instruction & 0x00200000;

  u32int regSrc = (instruction & 0x000F0000) >> 16;
  u32int regDst = (instruction & 0x0000F000) >> 12;
  u32int regDst2 = regDst + 1;

  if ((regDst % 2) == 1)
  {
    DIE_NOW(context, "undefined case: regDst must be even number!");
  }

  u32int offsetAddress = 0;
  u32int baseAddress = loadGuestGPR(regSrc, context);

  u32int wback = (prePost == 0) || (writeback != 0);

  // P = 0 and W == 1 then STR as if user mode
  if ((prePost == 0) && (writeback != 0))
  {
    DIE_NOW(context, "unpredictable case (P=0 AND W=1)!");
  }

  if (wback && ((regDst == 15) || (regSrc == regDst) || (regSrc == regDst2)))
  {
    DIE_NOW(context, "unpredictable register selection!");
  }
  if (regDst2 == 15)
  {
    DIE_NOW(context, "unpredictable case, regDst2 = PC!");
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
    DEBUG(INTERPRETER_ARM_LOAD, "armLdrdInstruction: imm32=%#.8x baseAddress=%#.8x offsetAddres="
        "%#.8x" EOL, imm32, baseAddress, offsetAddress);
  } // Immediate case ends
  else
  {
    // register case
    u32int regSrc2 = instruction & 0x0000000F;
    u32int offsetRegisterValue = loadGuestGPR(regSrc2, context);
    // regDest2 == PC then UNPREDICTABLE
    if (regSrc2 == 15)
    {
      DIE_NOW(context, "reg Rm == PC UNPREDICTABLE case!");
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
    DEBUG(INTERPRETER_ARM_LOAD, "armLdrdInstruction: Rm=%x baseAddress=%#.8x offsetRegVal=%#.8x"
        EOL, regSrc2, baseAddress, offsetAddress);
  } // Register case ends

  u32int address = 0;
  // if preIndex then use offsetAddress else baseAddress
  if (prePost != 0)
  {
    address = offsetAddress;
  }
  else
  {
    address = baseAddress;
  }

  DEBUG(INTERPRETER_ARM_LOAD, "armLdrdInstruction: address = %#.8x" EOL, address);

  u32int valueLoaded = vmLoad(WORD, address);
  u32int valueLoaded2 = vmLoad(WORD, address+4);
  // put loaded values to their registers
  storeGuestGPR(regDst, valueLoaded, context);
  storeGuestGPR(regDst2, valueLoaded2, context);

  DEBUG(INTERPRETER_ARM_LOAD, "armLdrdInstruction: loaded %#.8x %#.8x " EOL, valueLoaded,
      valueLoaded2);

  if (wback)
  {
    // Rn = offsetAddr;
    storeGuestGPR(regSrc, offsetAddress, context);
  }
  return context->R15 + ARM_INSTRUCTION_SIZE;
}

u32int armLdrhtInstruction(GCONTXT *context, u32int instruction)
{
  if (!evaluateConditionCode(context, ARM_EXTRACT_CONDITION_CODE(instruction)))
  {
    return context->R15 + ARM_INSTRUCTION_SIZE;
  }

  DEBUG_TRACE(INTERPRETER_ARM_LOAD, context, instruction);

  DIE_NOW(context, "not implemented");
}

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
  u32int baseAddress = loadGuestGPR(baseReg, context);

  if ((baseReg == GPR_PC) || !regList)
  {
    DIE_NOW(context, "UNPREDICTABLE: base=PC or no registers in list");
  }

  u32int savedCPSR = 0;
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
      savedCPSR = context->CPSR;
      context->CPSR = (context->CPSR & ~0x1f) | PSR_USR_MODE;
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
      u32int valueLoaded = vmLoad(WORD, address);
      storeGuestGPR(i, valueLoaded, context);
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
    storeGuestGPR(baseReg, baseAddress, context);
  }

  if (forceUser != 0)
  {
    // do we need to copy spsr? or return from userland?
    if (cpySpsr)
    {
      // ok, exception return option: restore SPSR to CPSR
      // SPSR! which?... depends what mode we are in...
      u32int modeSpsr = 0;
      switch (context->CPSR & PSR_MODE)
      {
        case PSR_FIQ_MODE:
          modeSpsr = context->SPSR_FIQ;
          break;
        case PSR_IRQ_MODE:
          modeSpsr = context->SPSR_IRQ;
          break;
        case PSR_SVC_MODE:
          modeSpsr = context->SPSR_SVC;
          break;
        case PSR_ABT_MODE:
          modeSpsr = context->SPSR_ABT;
          break;
        case PSR_UND_MODE:
          modeSpsr = context->SPSR_UND;
          break;
        default:
          DIE_NOW(context, "exception return from sys/usr mode!");
      }
      context->CPSR = modeSpsr;
    }
    else
    {
      context->CPSR = savedCPSR;
    }
  }

  if (isPCinRegList)
  {
    return context->R15;
  }
  else
  {
    return context->R15 + ARM_INSTRUCTION_SIZE;
  }
}
