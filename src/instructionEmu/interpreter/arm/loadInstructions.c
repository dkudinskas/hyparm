#include "common/bit.h"

#include "instructionEmu/interpreter/internals.h"

#include "instructionEmu/interpreter/arm/loadInstructions.h"

#include "guestManager/guestExceptions.h"

#include "memoryManager/memoryProtection.h"


u32int armLdrInstruction(GCONTXT *context, u32int instruction)
{
  if (!evaluateConditionCode(context, ARM_EXTRACT_CONDITION_CODE(instruction)))
  {
    return context->lastGuestPC + ARM_INSTRUCTION_SIZE;
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

  baseAddress = getGPRegister(context, regSrc);

  if (!regOrImm)
  {
    // immediate case
    u32int imm32 = instruction & 0x00000FFF;

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
    ASSERT(regSrc2 != GPR_PC, ERROR_UNPREDICTABLE_INSTRUCTION);

    u32int offsetRegisterValue = getGPRegister(context, regSrc2);

    // (shift_t, shift_n) = DecodeImmShift(type, imm5)
    u32int shiftAmount = 0;
    u32int shiftType = decodeShiftImmediate(((instruction & 0x060) >> 5),
                                           ((instruction & 0xF80)>>7), &shiftAmount);

    // offset = Shift(offsetRegisterValue, shiftType, shitAmount, cFlag);
    u32int offset = shiftVal(offsetRegisterValue, shiftType, shiftAmount,
                             context->CPSR.bits.C);

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

  ASSERT((address & 0x3) == 0, "Rd [Rn, Rm/#imm] unaligned address!");

  // P = 0 and W == 1 then LDR as if user mode
  if ((preOrPost == 0) && (writeBack != 0) && shouldDataAbort(context, FALSE, FALSE, address))
  {
    return context->lastGuestPC;
  }

  // DO the actual load from memory
  u32int valueLoaded = vmLoad(context, WORD, address);

  // LDR loading to PC should load a word-aligned value
  ASSERT(regDst != GPR_PC || (valueLoaded & 0x3) == 0, "loading unaligned value to PC!");

  // put loaded val to reg
  setGPRegister(context, regDst, valueLoaded);

  // wback = (P = 0) or (W = 1)
  bool wback = !preOrPost || writeBack;
  if (wback)
  {
    ASSERT(regDst != regSrc, ERROR_UNPREDICTABLE_INSTRUCTION);
    // Rn = offsetAddr;
    setGPRegister(context, regSrc, offsetAddress);
  }
  if (regDst == GPR_PC)
  {
    // can't use lastGuestPC, thats stale. use the value just loaded into PC
    return context->R15;
  }
  else
  {
    return context->lastGuestPC + ARM_INSTRUCTION_SIZE;
  }
}

u32int armLdrbInstruction(GCONTXT *context, u32int instruction)
{
  if (!evaluateConditionCode(context, ARM_EXTRACT_CONDITION_CODE(instruction)))
  {
    return context->lastGuestPC + ARM_INSTRUCTION_SIZE;
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

  u32int baseAddress = getGPRegister(context, regSrc);

  ASSERT(regDst != GPR_PC, ERROR_UNPREDICTABLE_INSTRUCTION);

  if (!regOrImm)
  {
    ASSERT(regSrc != GPR_PC, "check LDRB literal");

    // immediate case
    offset = instruction & 0x00000FFF;
  } // Immediate case ends
  else
  {
    // register case
    u32int regSrc2 = instruction & 0x0000000F;
    ASSERT(regSrc2 != GPR_PC, ERROR_UNPREDICTABLE_INSTRUCTION);

    u32int offsetRegisterValue = getGPRegister(context, regSrc2);

    // (shift_t, shift_n) = DecodeImmShift(type, imm5)
    u32int shiftAmount = 0;
    u32int shiftType = decodeShiftImmediate(((instruction & 0x060) >> 5),
                                            ((instruction & 0xF80)>>7), &shiftAmount);
    // offset = Shift(offsetRegisterValue, shiftType, shitAmount, cFlag);
    offset = shiftVal(offsetRegisterValue, shiftType, shiftAmount,
                      context->CPSR.bits.C);
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
  if (!preOrPost && writeBack && shouldDataAbort(context, FALSE, FALSE, address))
  {
    return context->lastGuestPC;
  }

  // DO the actual load from memory
  u32int valueLoaded = vmLoad(context, BYTE, address) & 0xFF;

  // put loaded val to reg
  setGPRegister(context, regDst, valueLoaded);

  // wback = (P = 0) or (W = 1)
  bool wback = !preOrPost || writeBack;
  if (wback)
  {
    ASSERT(regDst != regSrc, ERROR_UNPREDICTABLE_INSTRUCTION);
    // Rn = offsetAddr;
    setGPRegister(context, regSrc, offsetAddress);
  }
  return context->lastGuestPC + ARM_INSTRUCTION_SIZE;
}

u32int armLdrhInstruction(GCONTXT *context, u32int instruction)
{
  if (!evaluateConditionCode(context, ARM_EXTRACT_CONDITION_CODE(instruction)))
  {
    return context->lastGuestPC + ARM_INSTRUCTION_SIZE;
  }

  DEBUG_TRACE(INTERPRETER_ARM_LOAD, context, instruction);

  u32int preOrPost = instruction & 0x01000000; // 1 = pre, 0 = post
  u32int incOrDec = instruction & 0x00800000; // 1 = inc, 0 = dec
  u32int regOrImm = instruction & 0x00400000; // 1 = reg, 0 = imm
  u32int writeBack = instruction & 0x00200000; // 1 = writeBack indexing, 0 = no writeback
  u32int regSrc = (instruction & 0x000F0000) >> 16; // Source value from this register...
  u32int regDst = (instruction & 0x0000F000) >> 12; // Destination address

  // P = 0 and W == 1 then LDRHT (as if user mode)
  ASSERT(preOrPost || !writeBack, ERROR_NOT_IMPLEMENTED);

  ASSERT(regDst != GPR_PC, ERROR_UNPREDICTABLE_INSTRUCTION);

  u32int baseAddress = getGPRegister(context, regSrc);
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
    ASSERT((address & 0x1) == 0, "load address unaligned.");
  } // immediate case done
  else
  {
    // register case
    u32int regSrc2 = instruction & 0x0000000F;
    ASSERT(regSrc2 != GPR_PC, ERROR_UNPREDICTABLE_INSTRUCTION);

    u32int offsetRegisterValue = getGPRegister(context, regSrc2);

    // (shift_t, shift_n) = (SRType_LSL, 0);
    u32int shiftAmount = 0;
    u32int shiftType = decodeShiftImmediate(0, 0, &shiftAmount);

    // offset = Shift(offsetRegisterValue, shiftType, shitAmount, cFlag);
    u32int offset = shiftVal(offsetRegisterValue, shiftType, shiftAmount,
                             context->CPSR.bits.C);

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
    ASSERT((address & 0x1) == 0, "load address unaligned");
  } // reg case done

  u32int valueLoaded = vmLoad(context, HALFWORD, address) & 0xFFFF;

  // put loaded val to reg
  setGPRegister(context, regDst, valueLoaded);

  DEBUG(INTERPRETER_ARM_LOAD, "armLdrhInstruction: R[%x]=%#.8x" EOL, regDst, valueLoaded);

  // wback = (P = 0) or (W = 1)
  bool wback = (preOrPost == 0) || (writeBack != 0);
  if (wback)
  {
    //if Rn == PC || Rn == Rt || Rn == Rm) then UNPREDICTABLE;
    ASSERT(regDst != regSrc, ERROR_UNPREDICTABLE_INSTRUCTION);
    // Rn = offsetAddr;
    setGPRegister(context, regSrc, offsetAddress);
  }

  return context->lastGuestPC + ARM_INSTRUCTION_SIZE;
}

u32int armLdrdInstruction(GCONTXT *context, u32int instruction)
{
  if (!evaluateConditionCode(context, ARM_EXTRACT_CONDITION_CODE(instruction)))
  {
    return context->lastGuestPC + ARM_INSTRUCTION_SIZE;
  }

  DEBUG_TRACE(INTERPRETER_ARM_LOAD, context, instruction);

  u32int prePost = instruction & 0x01000000;
  u32int upDown = instruction & 0x00800000;
  u32int regOrImm = instruction & 0x00400000; // 0 = reg, 1 = imm
  u32int writeback = instruction & 0x00200000;

  u32int regSrc = (instruction & 0x000F0000) >> 16;
  u32int regDst = (instruction & 0x0000F000) >> 12;
  u32int regDst2 = regDst + 1;

  ASSERT((regDst & 1) == 0, ERROR_UNPREDICTABLE_INSTRUCTION);

  u32int offsetAddress = 0;
  u32int baseAddress = getGPRegister(context, regSrc);

  u32int wback = (prePost == 0) || (writeback != 0);

  // P = 0 and W == 1 then STR as if user mode
  ASSERT(prePost || !writeback, ERROR_UNPREDICTABLE_INSTRUCTION);
  ASSERT(!wback || (regDst != GPR_PC && regSrc != regDst && regSrc != regDst2), ERROR_UNPREDICTABLE_INSTRUCTION);
  ASSERT(regDst2 != GPR_PC, ERROR_UNPREDICTABLE_INSTRUCTION);

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
    ASSERT(regSrc2 != GPR_PC, ERROR_UNPREDICTABLE_INSTRUCTION);
    u32int offsetRegisterValue = getGPRegister(context, regSrc2);

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

  u32int valueLoaded = vmLoad(context, WORD, address);
  u32int valueLoaded2 = vmLoad(context, WORD, address + 4);
  // put loaded values to their registers
  setGPRegister(context, regDst, valueLoaded);
  setGPRegister(context, regDst2, valueLoaded2);

  DEBUG(INTERPRETER_ARM_LOAD, "armLdrdInstruction: loaded %#.8x %#.8x " EOL, valueLoaded,
      valueLoaded2);

  if (wback)
  {
    // Rn = offsetAddr;
    setGPRegister(context, regSrc, offsetAddress);
  }
  return context->lastGuestPC + ARM_INSTRUCTION_SIZE;
}


u32int armLdrtInstruction(GCONTXT *context, u32int instruction)
{
  return armLdrInstruction(context, instruction);
}


u32int armLdrhtInstruction(GCONTXT *context, u32int instruction)
{
  return armLdrhInstruction(context, instruction);
}


u32int armLdrbtInstruction(GCONTXT *context, u32int instruction)
{
  return armLdrbInstruction(context, instruction);
}


u32int armLdmInstruction(GCONTXT *context, u32int instruction)
{
  if (!evaluateConditionCode(context, ARM_EXTRACT_CONDITION_CODE(instruction)))
  {
    return context->lastGuestPC + ARM_INSTRUCTION_SIZE;
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
     * BLOCK COPY WARNING: we must use context->R15 instead of context->lastGuestPC
     * because we just updated the former!
     *
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

  return context->lastGuestPC + ARM_INSTRUCTION_SIZE;
}
