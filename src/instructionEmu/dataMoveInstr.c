#include "common/debug.h"

#include "cpuArch/constants.h"

#include "guestManager/blockCache.h"

#include "instructionEmu/commonInstrFunctions.h"
#include "instructionEmu/dataMoveInstr.h"

#include "memoryManager/cp15coproc.h"
#include "memoryManager/mmu.h"

#ifdef CONFIG_THUMB2
#include "memoryManager/globalMemoryMapper.h"
#endif


void invalidDataMoveTrap(const char * msg, GCONTXT * gc)
{
  printf("ERROR: %08x @ %08x should not have trapped!\n", gc->endOfBlockInstr, gc->R15);
  DIE_NOW(gc, msg);
}

/***********************************************************************************
 ***********************************************************************************
 ************************** STORE FUNCTIONS ****************************************
 ***********************************************************************************
 ***********************************************************************************/

u32int strInstruction(GCONTXT * context)
{
  u32int instr;
  u32int condcode;
  u32int regOrImm;
  u32int preOrPost;
  u32int incOrDec;
  u32int writeBack;
  u32int regDst;
  u32int regSrc;
  u32int baseAddress;
  u32int valueToStore;
  u32int offsetAddress;

#ifdef CONFIG_THUMB2
  if (context->CPSR & PSR_T_BIT)
  {
    bool thumb32;
    u32int imm32;
    instr = context->endOfBlockInstr;
    thumb32 = isThumb32(instr);
    if (!thumb32) //16-bit
    {
      if ((instr & 0xF800) == 0x6000) //imm5
      {
        regSrc = instr & 0x7;
        regDst = (instr & 0x38) >> 3;
        imm32 = ((instr & 0x7C0) >> 6) << 2; //extend
      }
      else if ((instr & 0xF800) == 0x9000)
      {
        regDst = 0xD; // hardcode SP register
        regSrc = (instr & 0x700) >> 8;
        imm32 = (instr & 0xFF) << 2;//extend

      }
      else
      {
        DIE_NOW(0, "Unimplemented Thumb16 STR instruction");
      }
      baseAddress = loadGuestGPR(regDst, context);
      valueToStore = loadGuestGPR(regSrc, context);
      offsetAddress = baseAddress + imm32;
    }
    else //thats for 32bit thumb instr
    {
      DIE_NOW(0, "Unimplemented thumb32 STR");
    }
    //printf("strInstr@%08x: regsrc=%x, regdst=%x, address=%x, value=%x\n",context->R15,regSrc,regDst,offsetAddress,valueToStore);
    context->hardwareLibrary->storeFunction(context->hardwareLibrary, WORD, offsetAddress, valueToStore);
    return context->R15 + 2;

  }
  else //ARM
#endif
  {
    instr = context->endOfBlockInstr;
    condcode = (instr & 0xF0000000) >> 28;
    regOrImm = instr & 0x02000000; // 1 = reg, 0 = imm
    preOrPost = instr & 0x01000000; // 1 = pre, 0 = post
    incOrDec = instr & 0x00800000; // 1 = inc, 0 = dec
    writeBack = instr & 0x00200000; // 1 = writeBack indexing, 0 = no writeback
    regDst = (instr & 0x000F0000) >> 16; // Base Destination address
    regSrc = (instr & 0x0000F000) >> 12; // Source value from this register...

#ifdef DATA_MOVE_TRACE
    printf("strInstr: regOrImm=%x preOrPost=%x incOrDec=%x writeBack=%x regdest=%x regsrc=%x\n",
        regOrImm, preOrPost, incOrDec, writeBack, regDst, regSrc);
#endif

    baseAddress = loadGuestGPR(regDst, context);
    valueToStore = loadGuestGPR(regSrc, context);

    if (!evaluateConditionCode(context, condcode))
    {
      // condition not met! allright, we're done here. next instruction...
#ifdef DATA_MOVE_TRACE
      printf("strInstr: condition not met\n");
#endif

#ifdef CONFIG_BLOCK_COPY
      return context->PCOfLastInstruction + 4;
#else
      return context->R15 + 4;
#endif
    }

    if (regOrImm == 0)
    {
#ifdef DATA_MOVE_TRACE
      printf("strInstr: imm case: ");
#endif
      // immediate case
      u32int imm32 = instr & 0x00000FFF;

#ifdef DATA_MOVE_TRACE
      printf("imm32=%x baseAddress=%08x valueToStore=%x offsetAddress=%08x",
          imm32, baseAddress, valueToStore, offsetAddress);
#endif

      // offsetAddress = if increment then base + imm32 else base - imm32
      if (incOrDec != 0)
      {
        offsetAddress = baseAddress + imm32;
#ifdef DATA_MOVE_TRACE
        printf(" inc\n");
#endif
      }
      else
      {
        offsetAddress = baseAddress - imm32;
#ifdef DATA_MOVE_TRACE
        printf(" dec\n");
#endif
      }
    } // Immediate case ends
    else
    {
#ifdef DATA_MOVE_TRACE
      printf("strInstr: reg case: ");
#endif
      // register case
      u32int regDst2 = instr & 0x0000000F;

      u32int offsetRegisterValue = loadGuestGPR(regDst2, context);
#ifdef DATA_MOVE_TRACE
      printf("regDst2=%x, baseAddress=%08x, offsetRegisterValue=%x, valueToStore=%x\n",
          regDst2, baseAddress, offsetRegisterValue, valueToStore);
#endif

      // regDest2 == PC then UNPREDICTABLE
      if (regDst2 == 15)
      {
        DIE_NOW(0, "STR reg Rm == PC UNPREDICTABLE case!");
      }

      // (shift_t, shift_n) = DecodeImmShift(type, imm5)
      u32int shiftAmount = 0;
      u32int shiftType = decodeShiftImmediate(((instr & 0x060) >> 5),
      ((instr & 0xF80)>>7), &shiftAmount);
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
      DIE_NOW(context, "STR Rd [Rn, Rm/#imm] unaligned address!\n");
    }

    // P = 0 and W == 1 then STR as if user mode
    if ((preOrPost == 0) && (writeBack != 0))
    {
      bool abort = shouldDataAbort(FALSE, TRUE, address);
      if (abort)
      {
#ifdef CONFIG_BLOCK_COPY
        return context->PCOfLastInstruction;
#else
        return context->R15;
#endif
      }
    }

    // *storeAddress = if sourceValue is PC then valueToStore+8 else valueToStore;
    valueToStore = (regSrc == 15) ? (valueToStore + 8) : valueToStore;
    context->hardwareLibrary->storeFunction(context->hardwareLibrary, WORD, address, valueToStore);

    // wback = (P = 0) or (W = 1)
    bool wback = (preOrPost == 0) || (writeBack != 0);
    if (wback)
    {
      //if Rn == PC || n == t) then UNPREDICTABLE;
      if ((regDst == 15) || (regDst == regSrc))
      {
        DIE_NOW(0, "STR writeback UNPREDICTABLE case!");
      }
      // Rn = offsetAddr;
      storeGuestGPR(regDst, offsetAddress, context);
    }
#ifdef CONFIG_BLOCK_COPY
    return context->PCOfLastInstruction + 4;
#else
    return (context->R15 + 4);
#endif
  }
}

u32int strbInstruction(GCONTXT * context)
{
  u32int instr = context->endOfBlockInstr;
  u32int condcode;
  u32int regOrImm;
  u32int preOrPost;
  u32int incOrDec;
  u32int writeBack;
  u32int regDst;
  u32int regSrc;
  u32int offsetAddress = 0;
  u32int baseAddress = 0;
  u32int valueToStore = 0;

#ifdef CONFIG_THUMB2
  if(context->CPSR & PSR_T_BIT)
  {
    u32int imm32 = 0;
    bool thumb32 = isThumb32(instr);
    u32int address = 0;
    if(!thumb32) // thumb16
    {
      if ( ((instr & THUMB16_STRB_IMM5_MASK) == THUMB16_STRB_IMM5))
      {
        regSrc = (instr & 0x0007);
        regDst = (instr & 0x0038)>>3;
        imm32 = (instr & 0x07C0)>>6;
      }
      else if( ((instr & THUMB16_STRB_REG_MASK) == THUMB16_STRB_REG))
      {
        regSrc = (instr & 0x0007);
        regDst = (instr & 0x0038)>>3;
        u32int regDst2 = (instr & 0x01C0)>>6;
        imm32 = loadGuestGPR(regDst2, context);
      }
      else
      {
        DIE_NOW(0,"Unimplemented Thumb16 STRB Instruction");
      }
      baseAddress = loadGuestGPR(regDst, context);
      offsetAddress = baseAddress + imm32;
      valueToStore = loadGuestGPR(regSrc, context);
      context->hardwareLibrary->storeFunction(context->hardwareLibrary, BYTE, offsetAddress, (valueToStore & 0xFF));
    }
    else // thumb32
    {
      if((instr & THUMB32_STRB_IMM12_MASK)==THUMB32_STRB_IMM12)
      {
        regOrImm = 0;
        regSrc = (instr & 0x0000F000)>>12;
        regDst = (instr & 0x000F0000)>>16;
        imm32 = (instr & 0x000000FF);
        writeBack = 0;
        incOrDec = 1;
        preOrPost = 1;
        baseAddress = loadGuestGPR(regDst, context);
        valueToStore = loadGuestGPR(regSrc, context);
        offsetAddress = baseAddress + imm32;
        //just be compatible
        address = offsetAddress;
      }
      else if ((instr & THUMB32_STRB_IMM8_MASK) == THUMB32_STRB_IMM8)
      {
        regOrImm = 0;
        regSrc = (instr & 0x0000F000)>>12;
        regDst = (instr & 0x000F0000)>>16;
        imm32 = (instr & 0x000000FF);
        writeBack = (instr & 0x00000100)>>8;
        incOrDec = (instr & 0x00000200)>>9;
        preOrPost = (instr & 0x00000400)>>10;
        baseAddress = loadGuestGPR(regDst, context);
        valueToStore = loadGuestGPR(regSrc, context);
        if(incOrDec != 0)
        {
          offsetAddress = baseAddress + imm32;
        }
        else
        {
          offsetAddress = baseAddress - imm32;
        }
        if(preOrPost != 0)
        {
          address = baseAddress;
        }
        else
        {
          address = offsetAddress;
        }
      }
      else
      {
        DIE_NOW(0,"Thumb32 STRB reg unimplemeted");
      }
      //printf("strbInstr: regsrc=%x, regdst=%x, address=%x, value=%x, P=%x, U=%x, W=%x\n",regSrc,regDst,address,valueToStore, preOrPost, incOrDec, writeBack);

      context->hardwareLibrary->storeFunction(context->hardwareLibrary, BYTE, address, (valueToStore & 0xFF));
      if(writeBack)
      {
        //printf("STRB: storing %x to %x\n", address, regDst);
        storeGuestGPR(regDst, valueToStore, context);
      }
    }
    if(thumb32)
    {
      return context->R15 +4;
    }
    else
    {
      return context->R15+2;
    }
  }
  else
#endif
  {
    condcode = (instr & 0xF0000000) >> 28;
    regOrImm = instr & 0x02000000; // 1 = reg, 0 = imm
    preOrPost = instr & 0x01000000; // 1 = pre, 0 = post
    incOrDec = instr & 0x00800000; // 1 = inc, 0 = dec
    writeBack = instr & 0x00200000; // 1 = writeBack indexing, 0 = no writeback
    regDst = (instr & 0x000F0000) >> 16; // Base Destination address
    regSrc = (instr & 0x0000F000) >> 12; // Source value from this register...
    offsetAddress = 0;
    baseAddress = 0;
    valueToStore = 0;
    if (!evaluateConditionCode(context, condcode))
    {
      // condition not met! allright, we're done here. next instruction...
#ifdef CONFIG_BLOCK_COPY
      return context->PCOfLastInstruction + 4;
#else
      return context->R15 + 4;
#endif
    }
    if (regSrc == 15)
    {
      DIE_NOW(0, "STRB source register PC UNPREDICTABLE case.");
    }
    if (regOrImm == 0)
    {
      // immediate case
      u32int imm32 = instr & 0x00000FFF;
      baseAddress = loadGuestGPR(regDst, context);
      valueToStore = loadGuestGPR(regSrc, context) & 0xFF;
      // offsetAddress = if increment then base + imm32 else base - imm32
      if (incOrDec != 0)
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
      u32int regDst2 = instr & 0x0000000F;
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
      u32int shiftType = decodeShiftImmediate(((instr & 0x060) >> 5),
      ((instr & 0xF80)>>7), &shiftAmount);
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

    // P = 0 and W == 1 then STR as if user mode
    if ((preOrPost == 0) && (writeBack != 0))
    {
      bool abort = shouldDataAbort(FALSE, TRUE, address);
      if (abort)
      {
#ifdef CONFIG_BLOCK_COPY
        return context->PCOfLastInstruction;
#else
        return context->R15;
#endif
      }
      // if usr can write, continue
    }

    // *storeAddress = if sourceValue is PC then valueToStore+8 else valueToStore;
    context->hardwareLibrary->storeFunction(context->hardwareLibrary, BYTE, address, (valueToStore & 0xFF));

    // wback = (P = 0) or (W = 1)
    bool wback = (preOrPost == 0) || (writeBack != 0);
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
#ifdef CONFIG_BLOCK_COPY
    return context->PCOfLastInstruction + 4;
#else
    return (context->R15 + 4);
#endif
  }
}

u32int strhtInstruction(GCONTXT * context)
{
  DIE_NOW(context, "STRHT unfinished\n");
  return 0;
}

u32int strhInstruction(GCONTXT * context)
{
  u32int instr = context->endOfBlockInstr;

  u32int condcode = 0;
  u32int preOrPost = 0;
  u32int incOrDec = 0;
  u32int regOrImm = 0;
  u32int writeBack = 0;
  u32int regDst = 0;
  u32int regSrc = 0;

#ifdef CONFIG_THUMB2
  if(context->CPSR & PSR_T_BIT)
  {
    u32int offset = 0;
    bool thumb32 = FALSE;
    thumb32 = isThumb32(instr);
    if(!thumb32)
    {
      if( ((instr & THUMB16_STRH_IMM5_MASK) == THUMB16_STRH_IMM5))
      {
        regSrc = (instr & 0x0007);
        regDst = (instr & 0x0038)>>3;
        offset = (instr & 0x07C0)>>6;
      }
      else if( ((instr & THUMB16_STRH_REG_MASK) == THUMB16_STRH_REG))
      {
        regSrc = (instr & 0x0007);
        regDst = (instr & 0x0038)>>3;
        u32int regDst2 = (instr & 0x01C0)>>6;
        offset = loadGuestGPR(regDst2, context);
      }
      else
      {
        DIE_NOW(0,"Unimplemented Thumb16 STRH");
      }
      u32int baseAddress = loadGuestGPR(regDst, context);
      u32int offsetAddress = baseAddress + offset;
      u32int valueToStore = loadGuestGPR(regSrc, context);
      context->hardwareLibrary->storeFunction(context->hardwareLibrary, HALFWORD, offsetAddress, valueToStore);
    }
    else
    {
      if( ((instr & THUMB32_STRH_REG_IMM5_MASK) == THUMB32_STRH_REG_IMM5))
      {
        regSrc = (instr & 0x0000F000)>>12;
        regDst = (instr & 0x000F0000)>>16;
        u32int imm12 = (instr & 0x00000FFF);
        u32int baseAddress = loadGuestGPR(regDst, context);
        u32int offsetAddress = baseAddress + imm12;
        u32int valueToStore = loadGuestGPR(regSrc, context);
        context->hardwareLibrary->storeFunction(context->hardwareLibrary, HALFWORD, offsetAddress, valueToStore);
      }
      else if( ((instr & THUMB32_STRH_REG_MASK) == THUMB32_STRH_REG))
      {
        regSrc = (instr & 0x0000F000)>>12;
        regDst = (instr & 0x000F0000)>>16;
        u32int regDst2 = (instr & 0x0000000F);
        u8int shift = (instr & 0x00000030)>>4;
        u32int baseAddress = loadGuestGPR(regDst, context);
        u32int offsetAddress = loadGuestGPR(regDst2, context);
        offsetAddress = baseAddress + (offsetAddress<<shift);
        u32int valueToStore = loadGuestGPR(regSrc, context);
        context->hardwareLibrary->storeFunction(context->hardwareLibrary, HALFWORD, offsetAddress, valueToStore);
      }
      else
      {
        DIE_NOW(0,"Unimplemented Thumb32 STRH");
      }
    }

    if(thumb32)
    {
      return context->R15+4;
    }
    else
    {
      return context->R15+2;
    }
  }
  else
#endif
  {
    condcode = (instr & 0xF0000000) >> 28;
    preOrPost = instr & 0x01000000; // 1 = pre, 0 = post
    incOrDec = instr & 0x00800000; // 1 = inc, 0 = dec
    regOrImm = instr & 0x00400000; // 1 = reg, 0 = imm
    writeBack = instr & 0x00200000; // 1 = writeBack indexing, 0 = no writeback
    regDst = (instr & 0x000F0000) >> 16; // Destination address
    regSrc = (instr & 0x0000F000) >> 12; // Source value from this register...

    if (!evaluateConditionCode(context, condcode))
    {
      // condition not met! allright, we're done here. next instruction...
#ifdef CONFIG_BLOCK_COPY
      return context->PCOfLastInstruction + 4;
#else
      return context->R15 + 4;
#endif
    }
    // P = 0 and W == 1 then STR as if user mode
    if ((preOrPost == 0) && (writeBack != 0))
    {
      DIE_NOW(0, "STRH as user mode unimplemented.");
    }
    if (regSrc == 15)
    {
      DIE_NOW(0, "STRH source register PC UNPREDICTABLE case.");
    }

    u32int offsetAddress = 0;
    u32int baseAddress = 0;
    u32int valueToStore = 0;

    if (regOrImm != 0)
    {
      // immediate case
      u32int imm4Top = instr & 0x00000F00;
      u32int imm4Bottom = instr & 0x0000000F;
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
      u32int regDst2 = instr & 0x0000000F;
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

    if ((address & 0x1) == 0x1)
    {
      DIE_NOW(context, "STRH Rd [Rn, Rm/#imm] unaligned address!\n");
    }
    context->hardwareLibrary->storeFunction(context->hardwareLibrary, HALFWORD, address, valueToStore);

    // wback = (P = 0) or (W = 1)
    bool wback = (preOrPost == 0) || (writeBack != 0);
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
#ifdef CONFIG_BLOCK_COPY
    return context->PCOfLastInstruction + 4;
#else
    return (context->R15 + 4);
#endif
  }
}

u32int stmInstruction(GCONTXT * context)
{
  u32int instr = 0;
  u32int condcode = 0;
  u32int prePost = 0;
  u32int upDown = 0;
  u32int forceUser = 0;
  u32int writeback = 0;
  u32int baseReg = 0;
  u32int regList = 0;
  u32int baseAddress = 0;

#ifdef CONFIG_THUMB2
  if(context->CPSR & PSR_T_BIT) // Thumb
  {
    u32int address = 0;
    bool thumb32 = FALSE;
    u32int valueLoaded = 0;
    // we trapped from Thumb mode.
    instr = context->endOfBlockInstr;
    thumb32 = isThumb32(instr);
    if(!thumb32)
    {
      if( ( instr & THUMB16_PUSH_MASK ) == THUMB16_PUSH )
      {
        regList = ( ( ((instr & 0x0100)>>8) << 15) ) | (instr & 0x00FF);
        baseReg = 0xD; // hardcode SP register
        address = loadGuestGPR(baseReg, context);
        address -= 4;// First item 4 bytes below the Stack pointer
        // Everything has to be stored in reverse order ( page 532 ).
        // Last item has to be just below the stack pointer

        // Is LR on the List?
        if( instr & 0x0100)//LR is on the list
        {
          valueLoaded = loadGuestGPR(0xE, context);
          validateCachePreChange(context->blockCache, address);
          context->hardwareLibrary->storeFunction(context->hardwareLibrary, WORD, address, valueLoaded);
          address -= 4;
        }

        // for i = 7 to 0. PUSH accepts only low registers
        int i = 7;
        for (i = 7; i >= 0; i--)
        {
          // if current register set
          if ( ((regList >> i) & 0x1) == 0x1)
          {
            valueLoaded = loadGuestGPR(i, context);
            // emulating store. Validate cache if needed
            validateCachePreChange(context->blockCache, address);
            context->hardwareLibrary->storeFunction(context->hardwareLibrary, WORD, address, valueLoaded);
            address -= 4;
          }
        } // for ends
        //thumb always update the SP to point to the start address
        address += 4;// FIX ME -> Not very smart, is it?
        storeGuestGPR(baseReg, address, context);
        //printf("Restore PC : %08x\n", context->R15+2);
        return context->R15+2;
      }
      else
      {
        DIE_NOW(0,"Unimplemented Thumb16 STM");
      }
    }
    else
    {
      DIE_NOW(0,"Unimplemented Thumb32 STM");
    }
  }
  else //ARM
#endif
  {
    instr = context->endOfBlockInstr;
#ifdef DATA_MOVE_TRACE
# ifdef CONFIG_BLOCK_COPY
    printf("STM instruction: %08x @ PC = %08x\n", instr, context->PCOfLastInstruction);
# else
    printf("STM instruction: %08x @ PC = %08x\n", instr, context->R15);
# endif
#endif

    condcode = (instr & 0xF0000000) >> 28;
    prePost = instr & 0x01000000;
    upDown = instr & 0x00800000;
    forceUser = instr & 0x00400000;
    writeback = instr & 0x00200000;

    baseReg = (instr & 0x000F0000) >> 16;
    regList = instr & 0x0000FFFF;
    baseAddress = loadGuestGPR(baseReg, context);

    if (!evaluateConditionCode(context, condcode))
    {
      // condition not met! allright, we're done here. next instruction...
#ifdef CONFIG_BLOCK_COPY
      return context->PCOfLastInstruction + 4;
#else
      return context->R15 + 4;
#endif
    }

    u32int savedCPSR = 0;
    if (forceUser != 0)
    {
      // force user bit set: STM user mode registers
      savedCPSR = context->CPSR;
      context->CPSR = (context->CPSR & ~0x1f) | PSR_USR_MODE;
    }

    int i = 0;
    u32int address = 0;
    if ((upDown == 0) && (prePost != 0)) // STM decrement before
    {
      // address = baseAddress - 4*(number of registers to store);
      address = baseAddress - 4 * countBitsSet(regList);
    }
    else if ((upDown == 0) && (prePost == 0)) // STM decrement after
    {
      // address = baseAddress - 4*(number of registers to store) + 4;
      address = baseAddress - 4 * countBitsSet(regList) + 4;
    }
    else if ((upDown != 0) && (prePost != 0)) // STM increment before
    {
      // address = baseAddress + 4 - will be incremented as we go
      address = baseAddress + 4;
    }
    else if ((upDown != 0) && (prePost == 0)) // STM increment after
    {
      // address = baseAddress - will be incremented as we go
      address = baseAddress;
    }

    // for i = 0 to 14
    for (i = 0; i < 15; i++)
    {
      // if current register set
      if (((regList >> i) & 0x1) == 0x1)
      {
#ifdef DATA_MOVE_TRACE
        printf("*(%08x) = R[%x] = %08x\n", address, i, loadGuestGPR(i, context));
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
    if (((regList >> 15) & 0x1) == 0x1)
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

#ifdef CONFIG_BLOCK_COPY
    return context->PCOfLastInstruction + 4;
#else
    return context->R15 + 4;
#endif
  }
}

/* store dual */
u32int strdInstruction(GCONTXT * context)
{
  u32int instr = context->endOfBlockInstr;
  u32int condcode = 0;
  u32int prePost = 0;
  u32int upDown = 0;
  u32int regOrImm = 0;
  u32int writeback = 0;
  u32int regDst = 0;
  u32int regSrc = 0;
  u32int regSrc2 = 0;

#ifdef CONFIG_THUMB2
  if(context->CPSR & PSR_T_BIT)
  {
    u32int address = 0;
    u32int imm8 = 0;
    if( ((instr & THUMB32_STRD_IMM8_MASK) == THUMB32_STRD_IMM8))
    {
      prePost = (instr & 0x01000000)>>24;
      upDown = (instr & 0x00800000)>>23;
      writeback = (instr & 0x00200000)>>21;
      regSrc = (instr & 0x0000F000)>>12;
      regSrc2 = (instr & 0x00000F00)>>8;
      regDst = (instr & 0x000F0000)>>16;
      imm8 = (instr & 0x000000FF)<<2;
    }
    else
    {
      DIE_NOW(0,"Unkown Thumb32 STRD");
    }

    u32int baseAddress = loadGuestGPR(regDst, context);
    u32int valueToStore = loadGuestGPR(regSrc, context);
    u32int valueToStore2 = loadGuestGPR(regSrc2, context);

    u32int offsetAddress = 0;
    // if 1 then increment, else decrement
    if(upDown)
    {
      offsetAddress = baseAddress + imm8;
    }
    else
    {
      offsetAddress = baseAddress - imm8;
    }

    // if 1 then pre-index, else post-index
    if(prePost)
    {
      address = offsetAddress;
    }
    else
    {
      address = baseAddress;
    }
#ifdef DATA_MOVE_TRACE
    printf("store val1 = %x@%08x store val2 = %x@%08x\n", regDst, imm8, valueToStore, address, valueToStore2, address+4);
#endif

    context->hardwareLibrary->storeFunction(context->hardwareLibrary, WORD, address, valueToStore);
    context->hardwareLibrary->storeFunction(context->hardwareLibrary, WORD, address+4, valueToStore2);

    if(writeback)
    {
      storeGuestGPR(regDst, offsetAddress, context);
    }
    return context->R15+4;
  }
  // ARM
  else
#endif
  {
    condcode = (instr & 0xF0000000) >> 28;
    prePost = instr & 0x01000000;
    upDown = instr & 0x00800000;
    regOrImm = instr & 0x00400000;
    writeback = instr & 0x00200000; // 0 = reg, !0 = imm
    regDst = (instr & 0x000F0000) >> 16;
    regSrc = (instr & 0x0000F000) >> 12;
    regSrc2 = regSrc + 1;

#ifdef DATA_MOVE_TRACE
    printf("STRD instruction: %08x @ PC = %08x\n", instr, context->R15);
#endif

    if (!evaluateConditionCode(context, condcode))
    {
      // condition not met! allright, we're done here. next instruction...
#ifdef CONFIG_BLOCK_COPY
      return context->PCOfLastInstruction + 4;
#else
      return context->R15 + 4;
#endif
    }
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
      u32int imm4h = (instr & 0x00000f00) >> 4;
      u32int imm4l = (instr & 0x0000000f);
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
      printf("imm32=%x baseAddress=%08x valueToStore=%x offsetAddress=%08x\n",
          imm32, baseAddress, valueToStore, offsetAddress);
#endif
    } // Immediate case ends
    else
    {
      // register case
      u32int regDst2 = instr & 0x0000000F;
      u32int offsetRegisterValue = loadGuestGPR(regDst2, context);
      // regDest2 == PC then UNPREDICTABLE
      if (regDst2 == 15)
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
      printf("Rm=%x baseAddress=%x valueToStore=%x offsetRegisterValue=%x\n",
          regDst2, baseAddress, valueToStore, offsetRegisterValue);
#endif
    } // Register case ends

    u32int address;
    // if preIndex then use offsetAddress else baseAddress
    if (prePost != 0)
    {
      address = offsetAddress;
    }
    else
    {
      address = baseAddress;
    }
#ifdef DATA_MOVE_TRACE
    printf("store address = %08x", address);
#endif
    // *storeAddress = if sourceValue is PC then valueToStore+8 else valueToStore;
    valueToStore = (regSrc == 15) ? (valueToStore + 8) : valueToStore;
#ifdef DATA_MOVE_TRACE
    printf("store val1 = %x store val2 = %x\n", valueToStore, valueToStore2);
#endif

    context->hardwareLibrary->storeFunction(context->hardwareLibrary, WORD, address, valueToStore);
    context->hardwareLibrary->storeFunction(context->hardwareLibrary, WORD, address + 4, valueToStore2);

    if (wback)
    {
      // Rn = offsetAddr;
      storeGuestGPR(regDst, offsetAddress, context);
    }
#ifdef CONFIG_BLOCK_COPY
    return context->PCOfLastInstruction + 4;
#else
    return (context->R15 + 4);
#endif
  }
}

u32int strexInstruction(GCONTXT * context)
{
  u32int instr = context->endOfBlockInstr;
#ifdef DATA_MOVE_TRACE
# ifdef CONFIG_BLOCK_COPY
  printf("STREX instruction: %08x @ PC = %08x\n", instr, context->PCOfLastInstruction);
# else
  printf("STREX instruction: %08x @ PC = %08x\n", instr, context->R15);
# endif
#endif

  u32int condcode = (instr & 0xF0000000) >> 28;
  u32int regN = (instr & 0x000F0000) >> 16;
  u32int regD = (instr & 0x0000F000) >> 12;
  u32int regT = (instr & 0x0000000F);

  if (!evaluateConditionCode(context, condcode))
  {
    // condition not met! allright, we're done here. next instruction...
#ifdef CONFIG_BLOCK_COPY
	return context->PCOfLastInstruction+4;
#else
    return context->R15 + 4;
#endif
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
#ifdef DATA_MOVE_TRACE
  printf("STREX instruction: address = %08x, valToStore = %x, valueFromReg %x\n",
      address, valToStore, regT);
#endif

  context->hardwareLibrary->storeFunction(context->hardwareLibrary, WORD, address, valToStore);
  // operation succeeded updating memory, flag regD (0 - updated, 1 - fail)
  storeGuestGPR(regD, 0, context);

#ifdef CONFIG_BLOCK_COPY
  return context->PCOfLastInstruction+4;
#else
  return context->R15 + 4;
#endif
}

u32int strexbInstruction(GCONTXT * context)
{
  u32int instr = context->endOfBlockInstr;
#ifdef DATA_MOVE_TRACE
# ifdef CONFIG_BLOCK_COPY
  printf("STREXB instruction: %08x @ PC = %08x\n", instr, context->PCOfLastInstruction);
# else
  printf("STREXB instruction: %08x @ PC = %08x\n", instr, context->R15);
# endif
#endif

  u32int condcode = (instr & 0xF0000000) >> 28;
  u32int regN = (instr & 0x000F0000) >> 16;
  u32int regD = (instr & 0x0000F000) >> 12;
  u32int regT = (instr & 0x0000000F);

  if (!evaluateConditionCode(context, condcode))
  {
    // condition not met! allright, we're done here. next instruction...
#ifdef CONFIG_BLOCK_COPY
    return context->PCOfLastInstruction+4;
#else
    return context->R15 + 4;
#endif
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
  context->hardwareLibrary->storeFunction(context->hardwareLibrary, BYTE, address, (valToStore & 0xFF));

  storeGuestGPR(regD, 0, context);

#ifdef CONFIG_BLOCK_COPY
  return context->PCOfLastInstruction+4;
#else
  return context->R15 + 4;
#endif
}

u32int strexdInstruction(GCONTXT * context)
{
#ifdef CONFIG_BLOCK_COPY
  DIE_NOW(context, "strexdInstruction is executed but not yet checked for blockCopyCompatibility");
#endif
  u32int instr = context->endOfBlockInstr;
#ifdef DATA_MOVE_TRACE
  printf("STREXD instruction: %08x @ PC = %08x\n", instr, context->R15);
#endif

  u32int condcode = (instr & 0xF0000000) >> 28;
  u32int regN = (instr & 0x000F0000) >> 16;
  u32int regD = (instr & 0x0000F000) >> 12;
  u32int regT = (instr & 0x0000000F);

  if (!evaluateConditionCode(context, condcode))
  {
    // condition not met! allright, we're done here. next instruction...
#ifdef CONFIG_BLOCK_COPY
    return context->PCOfLastInstruction+4;
#else
    return context->R15 + 4;
#endif
  }

  if ((regD == 15) || ((regT % 2) != 0) || (regT == 14) || (regN == 15))
  {
    DIE_NOW(0, "STREXD unpredictable case (PC used)");
  }
  if ((regD == regN) || (regD == regT) || (regD == (regT + 1)))
  {
    DIE_NOW(0, "STREXD unpredictable case (PC used)");
  }

  u32int address = loadGuestGPR(regN, context);

  // Create doubleword to store such that R[t] will be stored at addr and R[t2] at addr+4.
  u32int valToStore1 = loadGuestGPR(regT, context);
  u32int valToStore2 = loadGuestGPR(regT + 1, context);
  DIE_NOW(0, "STREXD: assuming littlendian!\n");
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

#ifdef CONFIG_BLOCK_COPY
  return context->PCOfLastInstruction+4;
#else
  return context->R15 + 4;
#endif
}

u32int strexhInstruction(GCONTXT * context)
{
#ifdef CONFIG_BLOCK_COPY
  DIE_NOW(context, "strexhInstruction is executed but not yet checked for blockCopyCompatibility");
#endif
  u32int instr = context->endOfBlockInstr;
#ifdef DATA_MOVE_TRACE
  printf("STREXH instruction: %08x @ PC = %08x\n", instr, context->R15);
#endif

  u32int condcode = (instr & 0xF0000000) >> 28;
  u32int regN = (instr & 0x000F0000) >> 16;
  u32int regD = (instr & 0x0000F000) >> 12;
  u32int regT = (instr & 0x0000000F);

  if (!evaluateConditionCode(context, condcode))
  {
    // condition not met! allright, we're done here. next instruction...
#ifdef CONFIG_BLOCK_COPY
    return context->PCOfLastInstruction+4;
#else
    return context->R15 + 4;
#endif
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

#ifdef CONFIG_BLOCK_COPY
  return context->PCOfLastInstruction+4;
#else
  return context->R15 + 4;
#endif
}

/***********************************************************************************
 ***********************************************************************************
 ************************** LOAD FUNCTIONS *****************************************
 ***********************************************************************************
 ***********************************************************************************/
u32int ldrhtInstruction(GCONTXT * context)
{
  DIE_NOW(context, "LDRHT unfinished\n");
  return 0;
}

u32int ldrhInstruction(GCONTXT * context)
{
#ifdef CONFIG_BLOCK_COPY
  DIE_NOW(context, "ldrhInstruction is executed but not yet checked for blockCopyCompatibility");
#endif
  u32int instr = context->endOfBlockInstr;
  u32int condcode = 0;
  u32int preOrPost = 0;
  u32int incOrDec = 0;
  u32int regOrImm = 0;
  u32int writeBack = 0;
  u32int regSrc = 0;
  u32int regDst = 0;

#ifdef CONFIG_THUMB2
  // Are we on Thumb mode?
  if(context->CPSR & PSR_T_BIT)
  {
    u32int imm12 = 0;
    bool thumb32 = FALSE;
    thumb32 = isThumb32(instr);
    if(!thumb32)
    {
      DIE_NOW(0, "Unimplemented ldrh/ldrsh instruction");
    }
    else
    {
      // Lets handle the LDRSH first

      if( instr & 0x01000000 )// Singed bit = 1
      {
        // Encoding T1 -> page 454 - A8-168
        if(instr & 0x00800000)
        {
          regSrc = (instr & 0x000F0000)>>16;
          regDst = (instr & 0x0000F000)>>12;
          imm12 = (instr & 0x00000FFF);
        }
        else
        {
          // unimplemented
          DIE_NOW(0, "Unimplemented Thumb32 LDRSH (T2)");
        }
      }
      else
      {
        // unimplemented
        DIE_NOW(0, "Unimplemented Thumb32 LDRH (T1+T2)");
      }
      u32int baseAddress = loadGuestGPR(regSrc, context);
      u32int offsetAddress = baseAddress + imm12;
      u32int valueLoaded = context->hardwareLibrary->loadFunction(context->hardwareLibrary, HALFWORD, offsetAddress);
      storeGuestGPR(regDst, valueLoaded, context);

      if(!thumb32)
      {
        return context->R15 + 2;
      }
      else
      {
        return context->R15 + 4;
      }
    }
  }
  else
#endif
  {
    condcode = (instr & 0xF0000000) >> 28;
    preOrPost = instr & 0x01000000; // 1 = pre, 0 = post
    incOrDec = instr & 0x00800000; // 1 = inc, 0 = dec
    regOrImm = instr & 0x00400000; // 1 = reg, 0 = imm
    writeBack = instr & 0x00200000; // 1 = writeBack indexing, 0 = no writeback
    regSrc = (instr & 0x000F0000) >> 16; // Source value from this register...
    regDst = (instr & 0x0000F000) >> 12; // Destination address

    // P = 0 and W == 1 then LDRHT (as if user mode)
    if ((preOrPost == 0) && (writeBack != 0))
    {
      DIE_NOW(0, "LDRH as user mode unimplemented.");
    }
    if (regDst == 15)
    {
      // cannot load halfword into PC!!
      DIE_NOW(0, "LDRH Rd=PC UNPREDICTABLE case.");
    }
    if (!evaluateConditionCode(context, condcode))
    {
      // condition not met! allright, we're done here. next instruction...
#ifdef CONFIG_BLOCK_COPY
      return context->PCOfLastInstruction+4;
#else
      return context->R15 + 4;
#endif
    }

    u32int baseAddress = loadGuestGPR(regSrc, context);
    ;
    u32int offsetAddress = 0;
    u32int address = 0;

    if (regOrImm != 0)
    {
      // immediate case
      u32int imm4Top = instr & 0x00000F00;
      u32int imm4Bottom = instr & 0x0000000F;
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
        DIE_NOW(0, "LDRH: load address unaligned.");
      }
    } // immediate case done
    else
    {
      // register case
      u32int regSrc2 = instr & 0x0000000F;
      if (regSrc2 == 15)
      {
        DIE_NOW(0, "LDRH reg Rm == PC UNPREDICTABLE case!");
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
        DIE_NOW(0, "LDRH: load address unaligned.");
      }
    } // reg case done

    u32int valueLoaded = context->hardwareLibrary->loadFunction(context->hardwareLibrary, HALFWORD, address);

    // put loaded val to reg
    storeGuestGPR(regDst, valueLoaded, context);

#ifdef DATA_MOVE_TRACE
    printf("ldrhInstr: %08x @ PC = %08x R[%x]=%x\n", instr, context->R15, regDst, valueLoaded);
#endif

    // wback = (P = 0) or (W = 1)
    bool wback = (preOrPost == 0) || (writeBack != 0);
    if (wback)
    {
      //if Rn == PC || Rn == Rt || Rn == Rm) then UNPREDICTABLE;
      if (regDst == regSrc)
      {
        DIE_NOW(0, "LDRH writeback UNPREDICTABLE case!");
      }
      // Rn = offsetAddr;
      storeGuestGPR(regSrc, offsetAddress, context);
    }

#ifdef CONFIG_BLOCK_COPY
    return context->PCOfLastInstruction+4;
#else
    return (context->R15 + 4);
#endif
  }
}

u32int ldrbInstruction(GCONTXT * context)
{
#ifdef CONFIG_BLOCK_COPY
  DIE_NOW(0, "ldrbInstruction is executed but not yet checked for blockCopyCompatibility");
#endif
  u32int instr = context->endOfBlockInstr;
  u32int condcode = 0;
  u32int regOrImm = 0;
  u32int preOrPost = 0;
  u32int incOrDec = 0;
  u32int writeBack = 0;
  u32int regSrc = 0;
  u32int regDst = 0;
  u32int offset = 0;
  u32int offsetAddress = 0;
  u32int baseAddress = 0;

#ifdef CONFIG_THUMB2
  if(context->CPSR & PSR_T_BIT)
  {
    bool thumb32 = isThumb32(instr);
    if(thumb32)
    {
      DIE_NOW(0,"Unimplemented Thumb-32 LDRB");
    }
    else
    {
      if( ((instr & THUMB16_LDRB_IMM5_MASK) == THUMB16_LDRB_IMM5))
      {
        regSrc = (instr & 0x0078)>>3;
        regDst = (instr & 0x0007);
        offset = (instr & 0x07C0)>>6;
      }
      else if ( ((instr & THUMB16_LDRB_REG_MASK) == THUMB16_LDRB_REG))
      {
        regSrc = (instr & 0x0078)>>3;
        regDst = (instr & 0x0007);
        u32int regSrc2 = (instr & 0x01C)>>6;
        offset = loadGuestGPR(regSrc2, context);
      }
      else
      {
        DIE_NOW(0,"Unimplemented Thumb-16 LDRB instruction");
      }
      baseAddress=loadGuestGPR(regSrc, context);
      offsetAddress = baseAddress + offset;
      u32int valueLoaded = context->hardwareLibrary->loadFunction(context->hardwareLibrary, BYTE, offsetAddress) & 0xFF;
      storeGuestGPR(regDst, valueLoaded, context);
    }
    if(thumb32)
    {
      return context->R15 + 4;
    }
    else
    {
      return context->R15 + 2;
    }
  }
  else
#endif
  {
    condcode = (instr & 0xF0000000) >> 28;
    regOrImm = instr & 0x02000000; // 1 = reg, 0 = imm
    preOrPost = instr & 0x01000000; // 1 = pre, 0 = post
    incOrDec = instr & 0x00800000; // 1 = inc, 0 = dec
    writeBack = instr & 0x00200000; // 1 = writeBack indexing, 0 = no writeback
    regSrc = (instr & 0x000F0000) >> 16; // Base Load address
    regDst = (instr & 0x0000F000) >> 12; // Destination - load to this

    offset = 0;
    offsetAddress = 0;
    baseAddress = loadGuestGPR(regSrc, context);

    if (regDst == 15)
    {
      DIE_NOW(0, "LDRB: cannot load a single byte into PC!");
    }

    if (!evaluateConditionCode(context, condcode))
    {
      // condition not met! allright, we're done here. next instruction...
#ifdef CONFIG_BLOCK_COPY
      return context->PCOfLastInstruction+4;
#else
      return context->R15 + 4;
#endif
    }

    if (regOrImm == 0)
    {
      if (regSrc == 15)
      {
        DIE_NOW(0, "check LDRB literal");
      }
      // immediate case
      offset = instr & 0x00000FFF;

    } // Immediate case ends
    else
    {
      // register case
      u32int regSrc2 = instr & 0x0000000F;
      if (regSrc2 == 15)
      {
        DIE_NOW(0, "LDRB reg Rm == PC UNPREDICTABLE case!");
      }
      u32int offsetRegisterValue = loadGuestGPR(regSrc2, context);

      // (shift_t, shift_n) = DecodeImmShift(type, imm5)
      u32int shiftAmount = 0;
      u32int shiftType = decodeShiftImmediate(((instr & 0x060) >> 5),
      ((instr & 0xF80)>>7), &shiftAmount);
      u8int carryFlag = (context->CPSR & 0x20000000) >> 29;

      // offset = Shift(offsetRegisterValue, shiftType, shitAmount, cFlag);
      offset = shiftVal(offsetRegisterValue, shiftType, shiftAmount, &carryFlag);

    } // Register case ends
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

    // P = 0 and W == 1 then LDRB as if user mode
    if ((preOrPost == 0) && (writeBack != 0))
    {
      bool abort = shouldDataAbort(FALSE, FALSE, address);
      if (abort)
      {
#ifdef CONFIG_BLOCK_COPY
        return context->PCOfLastInstruction;
#else
        return context->R15;
#endif
      }
    }

    // DO the actual load from memory
    u32int valueLoaded = context->hardwareLibrary->loadFunction(context->hardwareLibrary, BYTE, address) & 0xFF;

    // put loaded val to reg
    storeGuestGPR(regDst, valueLoaded, context);

    // wback = (P = 0) or (W = 1)
    bool wback = (preOrPost == 0) || (writeBack != 0);
    if (wback)
    {
      // if Rn == Rt then UNPREDICTABLE
      if (regDst == regSrc)
      {
        DIE_NOW(0, "LDRB writeback UNPREDICTABLE case!");
      }
      // Rn = offsetAddr;
      storeGuestGPR(regSrc, offsetAddress, context);
    }
#ifdef CONFIG_BLOCK_COPY
    return context->PCOfLastInstruction+4;
#else
    return context->R15 + 4;
#endif
  }
}

u32int ldrInstruction(GCONTXT * context)
{
  u32int condcode = 0;
  u32int regOrImm = 0;
  u32int preOrPost = 0;
  u32int writeBack = 0;
  u32int incOrDec = 0;
  u32int regSrc = 0;
  u32int regDst = 0;
  u32int offsetAddress = 0;
  u32int baseAddress = 0;

#ifdef CONFIG_THUMB2
  if(context->CPSR & PSR_T_BIT)
  {
    u32int imm32 = 0;
    u32int instr = 0;
    bool thumb32 = FALSE;
    instr = context->endOfBlockInstr;
    thumb32 = isThumb32(instr);
    if(!thumb32) //16-bit
    {
      if( ( instr & THUMB16_LDR_IMM5_MASK ) == THUMB16_LDR_IMM5 )
      {
        imm32 = ( instr & 0x07C0 )>>6;
        regSrc = ( instr & 0x0038 )>>3;
        regDst = ( instr & 0x0007 );
        baseAddress = loadGuestGPR(regSrc,context);
        offsetAddress = baseAddress + imm32;
      }
      else if ( ( instr & THUMB16_LDR_IMM8_MASK ) == THUMB16_LDR_IMM8 )
      {
        imm32 = ( instr & 0x00FF );
        // Source register is SP
        regSrc = 0xD;
        regDst = ( instr & 0x0700 )>>8;
        baseAddress = loadGuestGPR(regSrc,context);
        offsetAddress = baseAddress + imm32;
      }
      else if ( ( instr & THUMB16_LDR_IMM8_LIT_MASK ) == THUMB16_LDR_IMM8_LIT )
      {
        imm32 = ( instr & 0x00FF );
        regDst = ( instr & 0x0700 )>>8;
        offsetAddress = imm32;
      }
      else if ( ( instr & THUMB16_LDR_REG_MASK ) == THUMB16_LDR_REG )
      {
        u32int regSrc2 = ( instr & 0x01C0 )>>6;
        regSrc = ( instr & 0x0038 )>>3;
        regDst = ( instr & 0x0003 );
        baseAddress = loadGuestGPR(regSrc, context);
        imm32 = loadGuestGPR(regSrc2, context);
        offsetAddress = baseAddress + imm32;
      }
      else
      {
        DIE_NOW(0,"Unimplemented thumb16 LDR instr");
      }
      u32int valueLoaded =
      context->hardwareLibrary->loadFunction(context->hardwareLibrary, WORD, offsetAddress);
      storeGuestGPR(regDst, valueLoaded, context);
    }
    else
    {
      DIE_NOW(0,"Unimplemented Thumb32 LDR");
    }
    if (thumb32)
    {
      return context->R15+4;
    }
    else
    {
      return context->R15+2;
    }
  }
  else //ARM
#endif
  {
    u32int instr = context->endOfBlockInstr;
    condcode = (instr & 0xF0000000) >> 28;
    regOrImm = instr & 0x02000000; // 1 = reg, 0 = imm
    preOrPost = instr & 0x01000000; // 1 = pre, 0 = post
    incOrDec = instr & 0x00800000; // 1 = inc, 0 = dec
    writeBack = instr & 0x00200000; // 1 = writeBack indexing, 0 = no writeback
    regSrc = (instr & 0x000F0000) >> 16; // Base Load address
    regDst = (instr & 0x0000F000) >> 12; // Destination - load to this
    offsetAddress = 0;
    baseAddress = 0;
    if (!evaluateConditionCode(context, condcode))
    {
      // condition not met! allright, we're done here. next instruction...
#ifdef CONFIG_BLOCK_COPY
      return context->PCOfLastInstruction+4;
#else
      return context->R15 + 4;
#endif
    }
    if (regOrImm == 0)
    {
      // immediate case
      u32int imm32 = instr & 0x00000FFF;
      baseAddress = loadGuestGPR(regSrc, context);
      if (regSrc == 15)
      {
        baseAddress = baseAddress + 8;
      }

      // offsetAddress = if increment then base + imm32 else base - imm32
      if (incOrDec != 0)
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
      u32int regSrc2 = instr & 0x0000000F;
      baseAddress = loadGuestGPR(regSrc, context);
      if (regSrc == 15)
      {
        baseAddress = baseAddress + 8;
      }
      // regSrc2 == PC then UNPREDICTABLE
      if (regSrc2 == 15)
      {
        DIE_NOW(0, "LDR reg Rm == PC UNPREDICTABLE case!");
      }
      u32int offsetRegisterValue = loadGuestGPR(regSrc2, context);

      // (shift_t, shift_n) = DecodeImmShift(type, imm5)
      u32int shiftAmount = 0;
      u32int shiftType = decodeShiftImmediate(((instr & 0x060) >> 5),
      ((instr & 0xF80)>>7), &shiftAmount);
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
      DIE_NOW(context, "LDR Rd [Rn, Rm/#imm] unaligned address!\n");
    }

    // P = 0 and W == 1 then LDR as if user mode
    if ((preOrPost == 0) && (writeBack != 0))
    {
      bool abort = shouldDataAbort(FALSE, FALSE, address);
      if (abort)
      {
#ifdef CONFIG_BLOCK_COPY
        return context->PCOfLastInstruction;
#else
        return context->R15;
#endif
      }
    }

    // DO the actual load from memory
    u32int valueLoaded = context->hardwareLibrary->loadFunction(context->hardwareLibrary, WORD, address);

    // LDR loading to PC should load a word-aligned value
    if ((regDst == 15) && ((valueLoaded & 0x3) != 0))
    {
      printf("LDR: regDst = %x, load from addr %08x\n", regDst, valueLoaded);
      DIE_NOW(context, "LDR Rd [Rn, Rm/#imm] load unaligned value to PC!\n");
    }
    // put loaded val to reg
    storeGuestGPR(regDst, valueLoaded, context);

    // wback = (P = 0) or (W = 1)
    bool wback = (preOrPost == 0) || (writeBack != 0);
    if (wback)
    {
      // if Rn == Rt then UNPREDICTABLE
      if (regDst == regSrc)
      {
        DIE_NOW(0, "LDR writeback UNPREDICTABLE case!");
      }
      // Rn = offsetAddr;
      storeGuestGPR(regSrc, offsetAddress, context);
    }
    if (regDst == 15)
    {
#ifdef CONFIG_BLOCK_COPY
      return context->PCOfLastInstruction;
#else
      return context->R15;
#endif
    }
    else
    {
#ifdef CONFIG_BLOCK_COPY
      return context->PCOfLastInstruction+4;
#else
      return context->R15 + 4;
#endif
    }
  }
}

u32int popLdrInstruction(GCONTXT * context)
{
  DIE_NOW(0, "POP unfinished\n");
  return 0;
}

u32int popLdmInstruction(GCONTXT * context)
{
  // regList must include PC, otherwise wouldnt have trapped!
  // just treat as a regular load multiple.
  return ldmInstruction(context);
}

u32int ldmInstruction(GCONTXT * context)
{
  u32int instr = 0;
  u32int condcode = 0;
  u32int prePost = 0;
  u32int upDown = 0;
  u32int forceUser = 0;
  u32int writeback = 0;
  u32int baseReg = 0;
  u32int regList = 0;
  u32int baseAddress = 0;

#ifdef CONFIG_THUMB2
  if(context->CPSR & PSR_T_BIT) // Thumb
  {

    u32int valueLoaded = 0;

    bool thumb32 = FALSE;
    int i = 0;
    // we trapped from Thumb mode. I assume the PC reg is in the list
    instr = context->endOfBlockInstr;
    thumb32 = isThumb32(instr);
    if(!thumb32)
    {
      if((instr & 0x100) == 0)
      {
        DIE_NOW(0,"Thumb POP instruction trapped but PC is not on the list...");
      }
      regList = ( ((instr & 0x0100)>>8) << 15 ) | (instr & 0x00FF);
      baseReg = 0xD; // hardcode SP register
      baseAddress = loadGuestGPR(baseReg, context);
      // for i = 0 to 7. POP accepts only low registers
      for (i = 7; i >= 0; i--)
      {
        // if current register set
        if ( ((regList >> i) & 0x1) == 0x1)
        {
          valueLoaded = context->hardwareLibrary->loadFunction(context->hardwareLibrary, WORD, baseAddress);
          storeGuestGPR(i, valueLoaded, context);
          baseAddress = baseAddress + 4;
        }
      } // for ends
      // and now take care of the PC
      //thumb always update the SP
      if( ( instr & 0x0100))
      {
        valueLoaded = context->hardwareLibrary->loadFunction(context->hardwareLibrary, WORD, baseAddress);
        storeGuestGPR(0xF, valueLoaded, context);
        baseAddress += 4;
      }

      storeGuestGPR(baseReg, baseAddress, context);
      if ( (context->R15 & 0x1)==0) // In which mode are we returning to?
      {
        context->CPSR &= ~PSR_T_BIT;
      }
      context->R15 &= ~0x1;
      return context->R15;
    }
    else
    {
      DIE_NOW(0,"Thumb ldm unimplemented");
    }
  }
  else // ARM
#endif
  {
    instr = context->endOfBlockInstr;
#ifdef DATA_MOVE_TRACE
    printf("LDM instruction: %08x @ PC = %08x\n", instr, context->R15);
#endif
    condcode = (instr & 0xF0000000) >> 28;
    prePost = instr & 0x01000000;
    upDown = instr & 0x00800000;
    forceUser = instr & 0x00400000;
    writeback = instr & 0x00200000;
    baseReg = (instr & 0x000F0000) >> 16;
    regList = instr & 0x0000FFFF;
    baseAddress = loadGuestGPR(baseReg, context);

    if ((baseReg == 15) || (countBitsSet(regList) == 0))
    {
      DIE_NOW(0, "LDM UNPREDICTABLE: base=PC or no registers in list");
    }

    if (!evaluateConditionCode(context, condcode))
    {
      // condition not met! allright, we're done here. next instruction...
#ifdef CONFIG_BLOCK_COPY
      return context->PCOfLastInstruction + 4;
#else
      return context->R15 + 4;
#endif
    }

    u32int savedCPSR = 0;
    bool cpySpsr = FALSE;
    if (forceUser != 0)
    {
      // ok, is this exception return, or LDM user mode?
      if ((instr & 0x00008000) != 0)
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
        u32int valueLoaded = context->hardwareLibrary->loadFunction(context->hardwareLibrary, WORD, address);
        storeGuestGPR(i, valueLoaded, context);
#ifdef DATA_MOVE_TRACE
        printf("R[%x] = *(%08x) = %08x\n", i, address, valueLoaded);
#endif
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
            DIE_NOW(0, "LDM: exception return form sys/usr mode!");
        }
        if ((modeSpsr & PSR_MODE) == PSR_USR_MODE)
        {
          DIE_NOW(context, "LDM: exception return to user mode!");
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
#ifdef CONFIG_BLOCK_COPY
      return context->PCOfLastInstruction;
#else
      return context->R15;
#endif
    }
    else
    {
#ifdef CONFIG_BLOCK_COPY
      return context->PCOfLastInstruction + 4;
#else
      return context->R15 + 4;
#endif
    }
  }
}

/* load dual */
u32int ldrdInstruction(GCONTXT * context)
{
#ifdef CONFIG_BLOCK_COPY
  DIE_NOW(0, "ldrdInstruction is executed but not yet checked for blockCopyCompatibility");
#endif
  u32int instr = context->endOfBlockInstr;

  u32int condcode = (instr & 0xF0000000) >> 28;
  u32int prePost = instr & 0x01000000;
  u32int upDown = instr & 0x00800000;
  u32int regOrImm = instr & 0x00400000; // 0 = reg, 1 = imm
  u32int writeback = instr & 0x00200000;

  u32int regSrc = (instr & 0x000F0000) >> 16;
  u32int regDst = (instr & 0x0000F000) >> 12;
  u32int regDst2 = regDst + 1;

#ifdef DATA_MOVE_TRACE
  printf("LDRD instruction: %08x @ PC = %08x\n", instr, context->R15);
#endif

  if (!evaluateConditionCode(context, condcode))
  {
    // condition not met! allright, we're done here. next instruction...
#ifdef CONFIG_BLOCK_COPY
    return context->PCOfLastInstruction+4;
#else
    return context->R15 + 4;
#endif
  }

  if ((regDst % 2) == 1)
  {
    DIE_NOW(0, "LDRD undefined case: regDst must be even number!");
  }

  u32int offsetAddress = 0;
  u32int baseAddress = loadGuestGPR(regSrc, context);

  u32int wback = (prePost == 0) || (writeback != 0);

  // P = 0 and W == 1 then STR as if user mode
  if ((prePost == 0) && (writeback != 0))
  {
    DIE_NOW(context, "LDRD unpredictable case (P=0 AND W=1)!");
  }

  if (wback && ((regDst == 15) || (regSrc == regDst) || (regSrc == regDst2)))
  {
    DIE_NOW(context, "LDRD unpredictable register selection!");
  }
  if (regDst2 == 15)
  {
    DIE_NOW(context, "LDRD: unpredictable case, regDst2 = PC!");
  }

  if (regOrImm != 0)
  {
    // immediate case
    u32int imm4h = (instr & 0x00000f00) >> 4;
    u32int imm4l = (instr & 0x0000000f);
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
    printf("imm32=%x baseAddress=%08x offsetAddres=%08x\n", imm32, baseAddress, offsetAddress);
#endif
  } // Immediate case ends
  else
  {
    // register case
    u32int regSrc2 = instr & 0x0000000F;
    u32int offsetRegisterValue = loadGuestGPR(regSrc2, context);
    // regDest2 == PC then UNPREDICTABLE
    if (regSrc2 == 15)
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
    printf("Rm=%x baseAddress=%x offsetRegVal=%x\n", regSrc2, baseAddress, offsetRegisterValue);
#endif
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
#ifdef DATA_MOVE_TRACE
  printf("LDRD: load address = %x\n", address);
#endif

  u32int valueLoaded = context->hardwareLibrary->loadFunction(context->hardwareLibrary, WORD, address);
  u32int valueLoaded2 = context->hardwareLibrary->loadFunction(context->hardwareLibrary, WORD, address + 4);
  // put loaded values to their registers
  storeGuestGPR(regDst, valueLoaded, context);
  storeGuestGPR(regDst2, valueLoaded2, context);

#ifdef DATA_MOVE_TRACE
  printf("LDRD: valueLoaded1 = %x valueLoaded2 = %x\n", valueLoaded, valueLoaded2);
#endif

  if (wback)
  {
    // Rn = offsetAddr;
    storeGuestGPR(regSrc, offsetAddress, context);
  }
#ifdef CONFIG_BLOCK_COPY
  return context->PCOfLastInstruction+4;
#else
  return (context->R15 + 4);
#endif
}

u32int ldrexInstruction(GCONTXT * context)
{
  u32int instr = context->endOfBlockInstr;
#ifdef DATA_MOVE_TRACE
# ifdef CONFIG_BLOCK_COPY
  printf("LDREX instruction: %08x @ PC = %08x\n", instr, context->PCOfLastInstruction);
# else
  printf("LDREX instruction: %08x @ PC = %08x\n", instr, context->R15);
# endif
#endif

  u32int condcode = (instr & 0xF0000000) >> 28;
  u32int baseReg = (instr & 0x000F0000) >> 16;
  u32int regDest = (instr & 0x0000F000) >> 12;

  if (!evaluateConditionCode(context, condcode))
  {
    // condition not met! allright, we're done here. next instruction...
#ifdef CONFIG_BLOCK_COPY
    return context->PCOfLastInstruction+4;
#else
    return context->R15 + 4;
#endif
  }

  if ((baseReg == 15) || (regDest == 15))
  {
    DIE_NOW(0, "LDREX unpredictable case (PC used).");
  }

  u32int baseVal = loadGuestGPR(baseReg, context);
  u32int value = context->hardwareLibrary->loadFunction(context->hardwareLibrary, WORD, baseVal);
#ifdef DATA_MOVE_TRACE
  printf("LDREX instruction: baseVal = %08x loaded %x, store to %x\n", baseVal, value, regDest);
#endif
  storeGuestGPR(regDest, value, context);

#ifdef CONFIG_BLOCK_COPY
  return context->PCOfLastInstruction+4;
#else
  return context->R15 + 4;
#endif
}

u32int ldrexbInstruction(GCONTXT * context)
{
  u32int instr = context->endOfBlockInstr;
#ifdef DATA_MOVE_TRACE
# ifdef CONFIG_BLOCK_COPY
  printf("LDREXB instruction: %08 @ PC = %08x\n", instr, context->PCOfLastInstruction);
#else
  printf("LDREXB instruction: %08 @ PC = %08x\n", instr, context->R15);
# endif
#endif

  u32int condcode = (instr & 0xF0000000) >> 28;
  u32int baseReg = (instr & 0x000F0000) >> 16;
  u32int regDest = (instr & 0x0000F000) >> 12;

  if (!evaluateConditionCode(context, condcode))
  {
    // condition not met! allright, we're done here. next instruction...
#ifdef CONFIG_BLOCK_COPY
    return context->PCOfLastInstruction+4;
#else
    return context->R15 + 4;
#endif
  }

  if ((baseReg == 15) || (regDest == 15))
  {
    DIE_NOW(0, "LDREXB unpredictable case (PC used).");
  }
  u32int baseVal = loadGuestGPR(baseReg, context);
  // byte zero extended to word...
  u32int value = ((u32int) context->hardwareLibrary->loadFunction(context->hardwareLibrary, BYTE, baseVal) & 0xFF);
  storeGuestGPR(regDest, value, context);

#ifdef CONFIG_BLOCK_COPY
  return context->PCOfLastInstruction+4;
#else
  return context->R15 + 4;
#endif
}

/*****************************************************************
 * Load Register Exclusive Doubleword *
 * derives an address from a base register value, loads a 64-bit *
 * doubleword from memory, writes it to two registers and *
 * marks the physical address as exclusive access *
 *****************************************************************/
u32int ldrexdInstruction(GCONTXT * context)
{
#ifdef CONFIG_BLOCK_COPY
  DIE_NOW(0, "ldrexdInstruction is executed but not yet checked for blockCopyCompatibility");
#endif
  u32int instr = context->endOfBlockInstr;
#ifdef DATA_MOVE_TRACE
  printf("LDREXD instruction: %08x @ PC = %08x\n", instr, context->R15);
#endif

  u32int condcode = (instr & 0xF0000000) >> 28;
  u32int baseReg = (instr & 0x000F0000) >> 16;
  u32int regDest = (instr & 0x0000F000) >> 12;

  if (!evaluateConditionCode(context, condcode))
  {
    // condition not met! allright, we're done here. next instruction...
#ifdef CONFIG_BLOCK_COPY
    return context->PCOfLastInstruction+4;
#else
    return context->R15 + 4;
#endif
  }
  // must not be PC, destination must be even and not link register
  if ((baseReg == 15) || ((regDest % 2) != 0) || (regDest == 14))
  {
    DIE_NOW(0, "LDREXH unpredictable case (invalid registers).");
  }
  u32int baseVal = loadGuestGPR(baseReg, context);

  u32int value = context->hardwareLibrary->loadFunction(context->hardwareLibrary, WORD, baseVal);
  u32int value2 = context->hardwareLibrary->loadFunction(context->hardwareLibrary, WORD, baseVal + 4);
  storeGuestGPR(regDest, value, context);
  storeGuestGPR(regDest + 1, value2, context);

#ifdef CONFIG_BLOCK_COPY
  return context->PCOfLastInstruction+4;
#else
  return context->R15 + 4;
#endif
}

u32int ldrexhInstruction(GCONTXT * context)
{
#ifdef CONFIG_BLOCK_COPY
  DIE_NOW(0, "ldrexhInstruction is executed but not yet checked for blockCopyCompatibility");
#endif
  u32int instr = context->endOfBlockInstr;
#ifdef DATA_MOVE_TRACE
  printf("LDREXH instruction: %08x @ PC = %08x\n", instr, context->R15);
#endif

  u32int condcode = (instr & 0xF0000000) >> 28;
  u32int baseReg = (instr & 0x000F0000) >> 16;
  u32int regDest = (instr & 0x0000F000) >> 12;

  if (!evaluateConditionCode(context, condcode))
  {
    // condition not met! allright, we're done here. next instruction...
#ifdef CONFIG_BLOCK_COPY
    return context->PCOfLastInstruction+4;
#else
    return context->R15 + 4;
#endif
  }

  if ((baseReg == 15) || (regDest == 15))
  {
    DIE_NOW(0, "LDREXH unpredictable case (PC used).");
  }
  u32int baseVal = loadGuestGPR(baseReg, context);
  // halfword zero extended to word...
  u32int value = ((u32int) context->hardwareLibrary->loadFunction(context->hardwareLibrary, HALFWORD, baseVal) & 0xFFFF);
  storeGuestGPR(regDest, value, context);

#ifdef CONFIG_BLOCK_COPY
  return context->PCOfLastInstruction+4;
#else
  return context->R15 + 4;
#endif
}


#ifdef CONFIG_BLOCK_COPY

u32int* strPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  u32int instruction=*instructionAddr;
  u32int srcPCRegLoc = 16;//This is where the PC is in the instruction (if immediate always at bit 16 if not can be at bit 0)
  u32int destReg=(instruction>>12) & 0xF;
  u32int instr2Copy=instruction;
  bool conditionAlways = (instruction>>28 & 0xF) == 0xE;

  if(((instruction>>25 & 0b1) == 0b1) & ((instruction & 0xF) == 0xF)){//bit 25 is 1 when there are 2 source registers
    DIE_NOW(0, "str PCFunct: str (register) cannot have Rm as PC -> UNPREDICTABLE");
  }

  if(conditionAlways)
  {
    if((instruction>>srcPCRegLoc & 0xF) == 0xF)  //There only have to be taken measures if Rn is PC
      {
        //step 1 Copy PC (=instructionAddr2) to desReg
        currBlockCopyCacheAddr=savePCInReg(context, instructionAddr, currBlockCopyCacheAddr,  destReg);

        //Step 2 modify strInstruction
        //Clear PC source Register
        instr2Copy=zeroBits(instruction, srcPCRegLoc);
        instr2Copy=instr2Copy | (destReg<<srcPCRegLoc);
      }

      currBlockCopyCacheAddr=checkAndClearBlockCopyCacheAddress(currBlockCopyCacheAddr,context->blockCache,(u32int*)context->blockCopyCache,(u32int*)context->blockCopyCacheEnd);
      *(currBlockCopyCacheAddr++)=instr2Copy;

      return currBlockCopyCacheAddr;
  }
  else
  {
    /* condition might be false */
    DIE_NOW(0, "conditional strPCFunct not yet implemented");
  }
}

u32int* strbPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  DIE_NOW(0, "strb PCFunct unfinished\n");
  return 0;
}

u32int* strhtPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  DIE_NOW(0, "strht PCFunct unfinished\n");
  return 0;
}

u32int* strhPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  DIE_NOW(0, "strh PCFunct unfinished\n");
  return 0;
}

u32int* stmPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  u32int instruction = *instructionAddr;
  if((instruction & 0xF0000) == 0xF0000)
  {
    // According to ARM ARM: source register = PC ->  UNPREDICTABLE
    DIE_NOW(0, "stm PC had PC as Rn -> UNPREDICTABLE?!\n");
  }
  else
  {
    //Stores multiple registers to consecutive memory locations
    //PC is not used -> instruction is save to execute just copy it to blockCopyCache
    currBlockCopyCacheAddr=checkAndClearBlockCopyCacheAddress(currBlockCopyCacheAddr,context->blockCache,(u32int*)context->blockCopyCache,(u32int*)context->blockCopyCacheEnd);
    *(currBlockCopyCacheAddr++)=instruction;

    return currBlockCopyCacheAddr;
  }
}

u32int* strdPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  DIE_NOW(0, "strd PCFunct unfinished\n");
  return 0;
}

u32int* strexPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  DIE_NOW(0, "strex PCFunct unfinished\n");
  return 0;
}

u32int* strexbPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  DIE_NOW(0, "strexb PCFunct unfinished\n");
  return 0;
}

u32int* strexdPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  DIE_NOW(0, "strexd PCFunct unfinished\n");
  return 0;
}

u32int* strexhPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  DIE_NOW(0, "strexh PCFunct unfinished\n");
  return 0;
}

u32int* ldrhtPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  DIE_NOW(0, "ldrht PCFunct unfinished\n");
  return 0;
}
u32int* ldrhPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  DIE_NOW(0, "ldrh PCFunct unfinished\n");
  return 0;
}

u32int* ldrbPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  DIE_NOW(0, "ldrb PCFunct unfinished\n");
  return 0;
}

/*
 * ldrPCInstruction is only called when destReg != PC
 */
u32int* ldrPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  u32int instruction =*instructionAddr;
  u32int srcPCRegLoc = 16;//This is where the PC is in the instruction (if immediate always at bit 16 if not can be at bit 0)
  u32int srcReg1 = (instruction>>srcPCRegLoc) & 0xF;
  bool srcReg1IsPC = (srcReg1)==0xF;
  u32int destReg = (instruction>>12) & 0xF;
  u32int instr2Copy = instruction;
  bool conditionAlways = (instruction>>28 & 0xF) == 0xE;
  u32int scratchReg;
  if(((instruction>>25 & 0b1) == 1) && ((instruction & 0xF) == 0xF)){//bit 25 is 1 when there are 2 source registers
    //see ARM ARM p 436 Rm cannot be PC
    DIE_NOW(0, "ldr PCFunct (register) with Rm = PC -> UNPREDICTABLE\n");
  }
  if(!srcReg1IsPC)
  {
    //It is safe to just copy the instruction
    currBlockCopyCacheAddr=checkAndClearBlockCopyCacheAddress(currBlockCopyCacheAddr,context->blockCache,(u32int*)context->blockCopyCache,(u32int*)context->blockCopyCacheEnd);
    *(currBlockCopyCacheAddr++)=instr2Copy;

    return currBlockCopyCacheAddr;
  }
  if(conditionAlways)
  {
      //Here starts the general procedure.  For this srcPCRegLoc must be set correctly
      //step 1 Copy PC (=instructionAddr2) to desReg
      currBlockCopyCacheAddr=savePCInReg(context, instructionAddr, currBlockCopyCacheAddr,  destReg);

      //Step 2 modify ldrInstruction
      //Clear PC source Register
      instr2Copy=zeroBits(instruction, srcPCRegLoc);
      instr2Copy=instr2Copy | (destReg<<srcPCRegLoc);


      currBlockCopyCacheAddr=checkAndClearBlockCopyCacheAddress(currBlockCopyCacheAddr,context->blockCache,(u32int*)context->blockCopyCache,(u32int*)context->blockCopyCacheEnd);
      *(currBlockCopyCacheAddr++)=instr2Copy;
      return currBlockCopyCacheAddr;
  }
  else
  {
    /* conditional instruction thus sometimes not executed */
    /*Instruction has to be changed to a PC safe instructionstream withouth using destReg. */
    scratchReg = findUnusedRegister(srcReg1, destReg, -1);
    /* place 'Backup scratchReg' instruction */
    currBlockCopyCacheAddr = backupRegister(scratchReg, currBlockCopyCacheAddr, blockCopyCacheStartAddress);
    currBlockCopyCacheAddr= savePCInReg(context, instructionAddr, currBlockCopyCacheAddr,  scratchReg);


    instr2Copy = zeroBits(instr2Copy, srcPCRegLoc);
    instr2Copy = instr2Copy | scratchReg<<srcPCRegLoc;

    currBlockCopyCacheAddr=checkAndClearBlockCopyCacheAddress(currBlockCopyCacheAddr,context->blockCache,(u32int*)context->blockCopyCache,(u32int*)context->blockCopyCacheEnd);
    *(currBlockCopyCacheAddr++)=instr2Copy;

    /* place 'restore scratchReg' instruction */
    currBlockCopyCacheAddr = restoreRegister(scratchReg, currBlockCopyCacheAddr, blockCopyCacheStartAddress);
    /* Make sure scanner sees that we need a word to store the register*/
    currBlockCopyCacheAddr = (u32int*)(((u32int)currBlockCopyCacheAddr)|0b1);

    return currBlockCopyCacheAddr;
  }
}

u32int* popLdrPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  DIE_NOW(0, "popLdr PCFunct unfinished\n");
  return 0;
}

u32int* popLdmPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  DIE_NOW(0, "popLdm PCFunct unfinished\n");
  return 0;
}

u32int* ldmPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  u32int instruction = *instructionAddr;
  if((instruction>>16 & 0xF) == 0xF){
    DIE_NOW(0, "ldm that is using PC is UNPREDICTABLE");//see ARM ARM P.424-428
  }
  //This means that instruction is always save
  currBlockCopyCacheAddr=checkAndClearBlockCopyCacheAddress(currBlockCopyCacheAddr,context->blockCache,(u32int*)context->blockCopyCache,(u32int*)context->blockCopyCacheEnd);
  *(currBlockCopyCacheAddr++)=instruction;

  return currBlockCopyCacheAddr;
}

u32int* ldrdPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  DIE_NOW(0, "ldrd PCFunct unfinished\n");
  return 0;
}

u32int* ldrexPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  DIE_NOW(0, "ldrex PCFunct unfinished\n");
  return 0;
}

u32int* ldrexbPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  DIE_NOW(0, "ldrexb PCFunct unfinished\n");
  return 0;
}

u32int* ldrexdPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  DIE_NOW(0, "ldrexd PCFunct unfinished\n");
  return 0;
}

u32int* ldrexhPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  DIE_NOW(0, "ldrexh PCFunct unfinished\n");
  return 0;
}

#endif
