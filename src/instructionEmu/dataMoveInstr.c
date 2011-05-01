#include "common/debug.h"
#include "common/defines.h"

#include "guestManager/blockCache.h"

#include "instructionEmu/commonInstrFunctions.h"
#include "instructionEmu/dataMoveInstr.h"

#include "memoryManager/cp15coproc.h"
#include "memoryManager/mmu.h"


void invalidDataMoveTrap(char * msg, GCONTXT * gc)
{
  printf("ERROR: %08x @ %08x should not have trapped!\n", 
         gc->endOfBlockInstr, gc->R15);
  DIE_NOW(gc, msg);
}

/***********************************************************************************
***********************************************************************************
************************** STORE FUNCTIONS ****************************************
***********************************************************************************
***********************************************************************************/

u32int strInstruction(GCONTXT * context)
{
  u32int instr = 0;
  u32int condcode = 0;
  u32int regOrImm = 0;
  u32int preOrPost = 0;
  u32int incOrDec = 0;
  u32int writeBack = 0;
  u32int regDst = 0;
  u32int regSrc = 0;
  bool thumb32 = FALSE;
  u32int offsetAddress = 0;
  u32int baseAddress = 0;
  u32int valueToStore = 0;
  u32int imm32 = 0;
 
  if(context->CPSR & T_BIT)
  {	
	bool regSP = FALSE; // page 666
	instr = decodeThumbInstr(context,0);
	thumb32 = isThumb32(instr);
	printf("generic %08x\n",instr);
	if(!thumb32)//16-bit
	{
		if((instr & 0x6000) == 0x6000) //imm5
		{
			regSP = FALSE;
			regSrc = instr & 0x00000007;
			regDst = (instr & 0x000007C)>>3;
			imm32 = (instr & 0x000007C0)>>6;
		}
		else
		{
			regSP = TRUE; //source register is SP for imm8
			regDst = 0x0000000D; // hardcode SP register
			regSrc = (instr & 0x00000700)>>8;
			imm32 = instr & 0x000000FF;
			
		}
	baseAddress = loadGuestGPR(regDst, context);
	valueToStore = loadGuestGPR(regSrc, context);
	offsetAddress = baseAddress + imm32; 
	}
	printf("strInstr: regsrc=%x, regdst=%x, address=%x, value=%x\n",regSrc,regDst,offsetAddress,valueToStore);
	context->hardwareLibrary->storeFunction(context->hardwareLibrary, WORD, offsetAddress, valueToStore);
	return context->R15+2;

  }
  else //ARM
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

	  u32int cpsrCC = (context->CPSR & 0xF0000000) >> 28;
	  if (!evalCC(condcode, cpsrCC))
	  {
	    // condition not met! allright, we're done here. next instruction...
#ifdef DATA_MOVE_TRACE
	    printf("strInstr: condition not met\n");
#endif
	    return context->R15 + 4;
	  }

	  if (regOrImm == 0)
	  {
#ifdef DATA_MOVE_TRACE
	    printf("strInstr: imm case: ");
#endif
	    // immediate case
	    imm32 = instr & 0x00000FFF;
	
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
    	u32int shiftType = decodeShiftImmediate(((instr & 0x060)>>5),
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
	      return context->R15;
    	}
  	}

	  // *storeAddress = if sourceValue is PC then valueToStore+8 else valueToStore;
	  valueToStore = (regSrc == 15) ? (valueToStore+8) : valueToStore;
	  context->hardwareLibrary->storeFunction(context->hardwareLibrary, WORD, address, valueToStore);

	  // wback = (P = 0) or (W = 1)
	  bool wback = (preOrPost == 0) || (writeBack != 0);
	  if (wback)
	  {
    	//if Rn == PC || n == t) then UNPREDICTABLE;
	    if ( (regDst == 15) || (regDst == regSrc) )
    	{
	      DIE_NOW(0, "STR writeback UNPREDICTABLE case!");
    	}
	    // Rn = offsetAddr;
    	storeGuestGPR(regDst, offsetAddress, context);
	  }
	  return (context->R15 + 4);
	}
}
u32int strbInstruction(GCONTXT * context)
{
  u32int instr = context->endOfBlockInstr;
  
  u32int condcode = (instr & 0xF0000000) >> 28;
  u32int regOrImm = instr & 0x02000000; // 1 = reg, 0 = imm
  u32int preOrPost = instr & 0x01000000; // 1 = pre, 0 = post
  u32int incOrDec = instr & 0x00800000; // 1 = inc, 0 = dec
  u32int writeBack = instr & 0x00200000; // 1 = writeBack indexing, 0 = no writeback
  u32int regDst = (instr & 0x000F0000) >> 16; // Base Destination address
  u32int regSrc = (instr & 0x0000F000) >> 12; // Source value from this register...

  u32int offsetAddress = 0;
  u32int baseAddress = 0;
  u32int valueToStore = 0;

  u32int cpsrCC = (context->CPSR & 0xF0000000) >> 28;
  if (!evalCC(condcode, cpsrCC))
  {
    // condition not met! allright, we're done here. next instruction...
    return context->R15 + 4;
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
    u32int shiftType = decodeShiftImmediate(((instr & 0x060)>>5),
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
      return context->R15;
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
    if ( (regDst == 15) || (regDst == regSrc) )
    {
      DIE_NOW(0, "STRB writeback UNPREDICTABLE case!");
    }
    // Rn = offsetAddr;
    storeGuestGPR(regDst, offsetAddress, context);
  }
  return (context->R15 + 4);
}


u32int strhtInstruction(GCONTXT * context)
{
  DIE_NOW(context, "STRHT unfinished\n");
  return 0;
}


u32int strhInstruction(GCONTXT * context)
{
  u32int instr = context->endOfBlockInstr;
  
  u32int condcode = (instr & 0xF0000000) >> 28;
  u32int preOrPost = instr & 0x01000000; // 1 = pre, 0 = post
  u32int incOrDec = instr & 0x00800000; // 1 = inc, 0 = dec
  u32int regOrImm = instr & 0x00400000; // 1 = reg, 0 = imm
  u32int writeBack = instr & 0x00200000; // 1 = writeBack indexing, 0 = no writeback
  u32int regDst = (instr & 0x000F0000) >> 16; // Destination address
  u32int regSrc = (instr & 0x0000F000) >> 12; // Source value from this register...

  u32int cpsrCC = (context->CPSR & 0xF0000000) >> 28;
  if (!evalCC(condcode, cpsrCC))
  {
    // condition not met! allright, we're done here. next instruction...
    return context->R15 + 4;
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
    if ( (regDst == 15) || (regDst == regSrc) )
    {
      DIE_NOW(0, "STRH writeback UNPREDICTABLE case!");
    }
    // Rn = offsetAddr;
    storeGuestGPR(regDst, offsetAddress, context);
  }
  return (context->R15 + 4);
}



u32int stmInstruction(GCONTXT * context)
{
  u32int instr = context->endOfBlockInstr;
#ifdef DATA_MOVE_TRACE
  printf("STM instruction: %08x @ PC = %08x\n", instr, context->R15);
#endif

  u32int condcode = (instr & 0xF0000000) >> 28;
  u32int prePost = instr & 0x01000000;
  u32int upDown = instr & 0x00800000;
  u32int forceUser = instr & 0x00400000;
  u32int writeback = instr & 0x00200000;

  u32int baseReg = (instr & 0x000F0000) >> 16;
  u32int regList = instr & 0x0000FFFF;
  u32int baseAddress = loadGuestGPR(baseReg, context);

  u32int cpsrCC = (context->CPSR & 0xF0000000) >> 28;
  if (!evalCC(condcode, cpsrCC))
  {
    // condition not met! allright, we're done here. next instruction...
    return context->R15 + 4;
  }

  u32int savedCPSR = 0;
  if (forceUser != 0)
  {
    // force user bit set: STM user mode registers
    savedCPSR = context->CPSR;
    context->CPSR = (context->CPSR & ~0x1f) | CPSR_MODE_USER;
  }

  int i = 0;
  u32int address = 0;
  if ( (upDown == 0) && (prePost != 0) ) // STM decrement before
  {
    // address = baseAddress - 4*(number of registers to store);
    address = baseAddress - 4 * countBitsSet(regList);
  }
  else if ( (upDown == 0) && (prePost == 0) ) // STM decrement after
  {
    // address = baseAddress - 4*(number of registers to store) + 4;
    address = baseAddress - 4 * countBitsSet(regList) + 4;
  }
  else if ( (upDown != 0) && (prePost != 0) ) // STM increment before
  {
    // address = baseAddress + 4 - will be incremented as we go
    address = baseAddress + 4;
  }
  else if ( (upDown != 0) && (prePost == 0) ) // STM increment after
  {
    // address = baseAddress - will be incremented as we go
    address = baseAddress;
  }
  
  // for i = 0 to 14
  for (i = 0; i < 15; i++)
  {
    // if current register set
    if ( ((regList >> i) & 0x1) == 0x1)
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
  if ( ((regList >> 15) & 0x1) == 0x1)
  {
    // emulating store. Validate cache if needed
    validateCachePreChange(context->blockCache, address);
    // *(address)= PC+8 - architectural feature due to pipeline..
    context->hardwareLibrary->storeFunction(context->hardwareLibrary, WORD,
                                            address, (loadGuestGPR(15, context)+8));
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

  return context->R15+4;
}

/* store dual */
u32int strdInstruction(GCONTXT * context)
{
  u32int instr = context->endOfBlockInstr;

  u32int condcode = (instr & 0xF0000000) >> 28;
  u32int prePost = instr & 0x01000000;
  u32int upDown = instr & 0x00800000;
  u32int regOrImm = instr & 0x00400000;
  u32int writeback = instr & 0x00200000; // 0 = reg, !0 = imm
  u32int regDst = (instr & 0x000F0000) >> 16;
  u32int regSrc = (instr & 0x0000F000) >> 12;
  u32int regSrc2 = regSrc+1;

#ifdef DATA_MOVE_TRACE
  printf("STRD instruction: %08x @ PC = %08x\n", instr, context->R15);
#endif

  u32int cpsrCC = (context->CPSR & 0xF0000000) >> 28;
  if (!evalCC(condcode, cpsrCC))
  {
    // condition not met! allright, we're done here. next instruction...
    return context->R15 + 4;
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

  if (wback && ((regDst == 15) || (regDst == regSrc) || (regDst == regSrc2)) )
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
  printf("store address = %08x", address);
#endif
  // *storeAddress = if sourceValue is PC then valueToStore+8 else valueToStore;
  valueToStore = (regSrc == 15) ? (valueToStore+8) : valueToStore;
#ifdef DATA_MOVE_TRACE
  printf("store val1 = %x store val2 = %x\n", valueToStore, valueToStore2);
#endif

  context->hardwareLibrary->storeFunction(context->hardwareLibrary, WORD, address, valueToStore);
  context->hardwareLibrary->storeFunction(context->hardwareLibrary, WORD, address+4, valueToStore2);

  if (wback)
  {
    // Rn = offsetAddr;
    storeGuestGPR(regDst, offsetAddress, context);
  }
  return (context->R15 + 4);
}


u32int strexInstruction(GCONTXT * context)
{
  u32int instr = context->endOfBlockInstr;
#ifdef DATA_MOVE_TRACE
  printf("STREX instruction: %08x @ PC = %08x\n", instr, context->R15);
#endif

  u32int condcode = (instr & 0xF0000000) >> 28;
  u32int regN = (instr & 0x000F0000) >> 16;
  u32int regD = (instr & 0x0000F000) >> 12;
  u32int regT = (instr & 0x0000000F);

  u32int cpsrCC = (context->CPSR & 0xF0000000) >> 28;
  if (!evalCC(condcode, cpsrCC))
  {
    // condition not met! allright, we're done here. next instruction...
    return context->R15 + 4;
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
  
  return context->R15 + 4;
}

u32int strexbInstruction(GCONTXT * context)
{
  u32int instr = context->endOfBlockInstr;
#ifdef DATA_MOVE_TRACE
  printf("STREXB instruction: %08x @ PC = %08x\n", instr, context->R15);
#endif

  u32int condcode = (instr & 0xF0000000) >> 28;
  u32int regN = (instr & 0x000F0000) >> 16;
  u32int regD = (instr & 0x0000F000) >> 12;
  u32int regT = (instr & 0x0000000F);

  u32int cpsrCC = (context->CPSR & 0xF0000000) >> 28;
  if (!evalCC(condcode, cpsrCC))
  {
    // condition not met! allright, we're done here. next instruction...
    return context->R15 + 4;
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
  
  return context->R15 + 4;
}

u32int strexdInstruction(GCONTXT * context)
{
  u32int instr = context->endOfBlockInstr;
#ifdef DATA_MOVE_TRACE
  printf("STREXD instruction: %08x @ PC = %08x\n", instr, context->R15);
#endif

  u32int condcode = (instr & 0xF0000000) >> 28;
  u32int regN = (instr & 0x000F0000) >> 16;
  u32int regD = (instr & 0x0000F000) >> 12;
  u32int regT = (instr & 0x0000000F);

  u32int cpsrCC = (context->CPSR & 0xF0000000) >> 28;
  if (!evalCC(condcode, cpsrCC))
  {
    // condition not met! allright, we're done here. next instruction...
    return context->R15 + 4;
  }
  
  if ((regD == 15) || ((regT % 2) != 0) || (regT == 14) || (regN == 15))
  {
    DIE_NOW(0, "STREXD unpredictable case (PC used)");
  }
  if ((regD == regN) || (regD == regT) || (regD == (regT+1)))
  {
    DIE_NOW(0, "STREXD unpredictable case (PC used)");
  }

  u32int address = loadGuestGPR(regN, context);

  // Create doubleword to store such that R[t] will be stored at addr and R[t2] at addr+4.
  u32int valToStore1 = loadGuestGPR(regT, context);
  u32int valToStore2 = loadGuestGPR(regT+1, context);
  DIE_NOW(0, "STREXD: assuming littlendian!\n");
  bool littleEndian = TRUE;
  if (littleEndian)
  {
    context->hardwareLibrary->storeFunction(context->hardwareLibrary, WORD, address, valToStore2);
    context->hardwareLibrary->storeFunction(context->hardwareLibrary, WORD, address+4, valToStore1);
  }
  else
  {
    context->hardwareLibrary->storeFunction(context->hardwareLibrary, WORD, address, valToStore1);
    context->hardwareLibrary->storeFunction(context->hardwareLibrary, WORD, address+4, valToStore2);
  }
  storeGuestGPR(regD, 0, context);
  
  return context->R15 + 4;
}

u32int strexhInstruction(GCONTXT * context)
{
  u32int instr = context->endOfBlockInstr;
#ifdef DATA_MOVE_TRACE
  printf("STREXH instruction: %08x @ PC = %08x\n", instr, context->R15);
#endif

  u32int condcode = (instr & 0xF0000000) >> 28;
  u32int regN = (instr & 0x000F0000) >> 16;
  u32int regD = (instr & 0x0000F000) >> 12;
  u32int regT = (instr & 0x0000000F);

  u32int cpsrCC = (context->CPSR & 0xF0000000) >> 28;
  if (!evalCC(condcode, cpsrCC))
  {
    // condition not met! allright, we're done here. next instruction...
    return context->R15 + 4;
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
  
  return context->R15 + 4;
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
  u32int instr = context->endOfBlockInstr;
  
  u32int condcode = (instr & 0xF0000000) >> 28;
  u32int preOrPost = instr & 0x01000000; // 1 = pre, 0 = post
  u32int incOrDec = instr & 0x00800000; // 1 = inc, 0 = dec
  u32int regOrImm = instr & 0x00400000; // 1 = reg, 0 = imm
  u32int writeBack = instr & 0x00200000; // 1 = writeBack indexing, 0 = no writeback
  u32int regSrc = (instr & 0x000F0000) >> 16; // Source value from this register...
  u32int regDst = (instr & 0x0000F000) >> 12; // Destination address

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

  u32int cpsrCC = (context->CPSR & 0xF0000000) >> 28;
  if (!evalCC(condcode, cpsrCC))
  {
    // condition not met! allright, we're done here. next instruction...
    return context->R15 + 4;
  }

  u32int baseAddress = loadGuestGPR(regSrc, context);;
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


  u32int valueLoaded =
    context->hardwareLibrary->loadFunction(context->hardwareLibrary, HALFWORD, address);

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

  return (context->R15 + 4);
}


u32int ldrbInstruction(GCONTXT * context)
{
  u32int instr = context->endOfBlockInstr;
  
  u32int condcode = (instr & 0xF0000000) >> 28;
  u32int regOrImm = instr & 0x02000000; // 1 = reg, 0 = imm
  u32int preOrPost = instr & 0x01000000; // 1 = pre, 0 = post
  u32int incOrDec = instr & 0x00800000; // 1 = inc, 0 = dec
  u32int writeBack = instr & 0x00200000; // 1 = writeBack indexing, 0 = no writeback
  u32int regSrc = (instr & 0x000F0000) >> 16; // Base Load address
  u32int regDst = (instr & 0x0000F000) >> 12; // Destination - load to this

  u32int offset = 0;
  u32int offsetAddress = 0;
  u32int baseAddress = loadGuestGPR(regSrc, context);

  if (regDst == 15)
  {
    DIE_NOW(0, "LDRB: cannot load a single byte into PC!");
  }
  
  u32int cpsrCC = (context->CPSR & 0xF0000000) >> 28;
  if (!evalCC(condcode, cpsrCC))
  {
    // condition not met! allright, we're done here. next instruction...
    return context->R15 + 4;
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
    u32int shiftType = decodeShiftImmediate(((instr & 0x060)>>5),
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
      return context->R15;
    }
  }

  // DO the actual load from memory
  u32int valueLoaded =
      context->hardwareLibrary->loadFunction(context->hardwareLibrary, BYTE, address) & 0xFF;

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
  return context->R15+4;
}

u32int ldrInstruction(GCONTXT * context)
{
  u32int instr = context->endOfBlockInstr;
   
  u32int condcode = (instr & 0xF0000000) >> 28;
  u32int regOrImm = instr & 0x02000000; // 1 = reg, 0 = imm
  u32int preOrPost = instr & 0x01000000; // 1 = pre, 0 = post
  u32int incOrDec = instr & 0x00800000; // 1 = inc, 0 = dec
  u32int writeBack = instr & 0x00200000; // 1 = writeBack indexing, 0 = no writeback
  u32int regSrc = (instr & 0x000F0000) >> 16; // Base Load address
  u32int regDst = (instr & 0x0000F000) >> 12; // Destination - load to this
  u32int offsetAddress = 0;
  u32int baseAddress = 0;

  u32int cpsrCC = (context->CPSR & 0xF0000000) >> 28;
  if (!evalCC(condcode, cpsrCC))
  {
    // condition not met! allright, we're done here. next instruction...
    return context->R15 + 4;
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
    u32int shiftType = decodeShiftImmediate(((instr & 0x060)>>5),
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
      return context->R15;
    }
  }

  // DO the actual load from memory
  u32int valueLoaded =
      context->hardwareLibrary->loadFunction(context->hardwareLibrary, WORD, address);

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
    return context->R15;
  }
  else
  {
    return context->R15+4;
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
  u32int prePost = 0;
  u32int upDown = 0;
  u32int forceUser = 0;
  u32int writeback = 0;
  u32int baseReg = 0;
  u32int regList = 0;
  u32int baseAddress = 0;
  u32int valueLoaded = 0;
  u32int condcode = 0;
  int i = 0;
  bool thumb32 = FALSE;
  if(context->CPSR & T_BIT) // Thumb
  {
  	// we trapped from Thumb mode. I assume the PC reg is in the list
	instr = decodeThumbInstr(context,0);
	printf("Thumb POP: %08x\n",instr);
	thumb32 = isThumb32(instr);
	if(!thumb32)
	{
		if((instr & 0x100) == 0)
		{
			DIE_NOW(0,"Thumb POP instruction trapped but PC is not on the list...");
		}
		regList = ( (instr & 0x0100) << 15 ) | (instr & 0x00FF);
		baseReg = 0x0000000D; // hardcode SP register
		baseAddress = loadGuestGPR(baseReg, context);
		// for i = 0 to 7. POP accepts only low registers
	    for (i = 0; i < 7; i++)
		{
    		// if current register set
	    	if ( ((regList >> i) & 0x1) == 0x1)
	   		{
    	  		valueLoaded = context->hardwareLibrary->loadFunction(context->hardwareLibrary, WORD, baseAddress);
		     	printf("Storing %08x to %08x\n", valueLoaded, i);
				storeGuestGPR(i, valueLoaded, context);
	      		baseAddress = baseAddress + 4;
		    }
		} // for ends
		// and now take care of the PC
		valueLoaded = context->hardwareLibrary->loadFunction(context->hardwareLibrary, WORD, baseAddress);
		printf("Storing %08x to PC", valueLoaded);
		storeGuestGPR(0xF, valueLoaded, context);
		baseAddress += 4;
		//thumb always update the SP
		storeGuestGPR(baseReg, baseAddress, context);
		return context->R15;
  	}
	else
	{
		DIE_NOW(0,"Thumb ldm unimplemented");
	}
  }
  else // ARM
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

	if ( (baseReg == 15) || (countBitsSet(regList) == 0) )
	  {
    	DIE_NOW(0, "LDM UNPREDICTABLE: base=PC or no registers in list");
	  }

	  u32int cpsrCC = (context->CPSR & 0xF0000000) >> 28;
	  if (!evalCC(condcode, cpsrCC))
	  {
	    // condition not met! allright, we're done here. next instruction...
	    return context->R15 + 4;
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
	      context->CPSR = (context->CPSR & ~0x1f) | CPSR_MODE_USER;
    	}
  	  }
  
	  u32int address = 0;
	  if ( (upDown == 0) && (prePost != 0) ) // LDM decrement before
	  {
    	// address = baseAddress - 4*(number of registers to load);
	    address = baseAddress - 4 * countBitsSet(regList);
	  }
	  else if ( (upDown == 0) && (prePost == 0) ) // LDM decrement after
	  {
	    // address = baseAddress - 4*(number of registers to load) + 4;
	    address = baseAddress - 4 * countBitsSet(regList) + 4;
  	  }
	  else if ( (upDown != 0) && (prePost != 0) ) // LDM increment before
	  {
    	// address = baseAddress + 4 - will be incremented as we go
	    address = baseAddress + 4;
  	  }	
	  else if ( (upDown != 0) && (prePost == 0) ) // LDM increment after
	  {
    	// address = baseAddress - will be incremented as we go
	    address = baseAddress;
	  }

	  bool isPCinRegList = FALSE;
	  // for i = 0 to 15
	  for (i = 0; i < 16; i++)
	  {
    	// if current register set
	    if ( ((regList >> i) & 0x1) == 0x1)
	    {
	      if (i == 15)
    	  {
	        isPCinRegList = TRUE;
    	  }
	      // R[i] = *(address);
    	  u32int valueLoaded =
	      context->hardwareLibrary->loadFunction(context->hardwareLibrary, WORD, address);
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
	      switch (context->CPSR & CPSR_MODE_FIELD)
	      {
	        case CPSR_MODE_FIQ:
	          modeSpsr = context->SPSR_FIQ;
	          break;
	        case CPSR_MODE_IRQ:
	          modeSpsr = context->SPSR_IRQ;
	          break;
    	    case CPSR_MODE_SVC:
	          modeSpsr = context->SPSR_SVC;
	          break;
	        case CPSR_MODE_ABORT:
	          modeSpsr = context->SPSR_ABT;
	          break;
	        case CPSR_MODE_UNDEF:
	          modeSpsr = context->SPSR_UND;
	          break;
	        case CPSR_MODE_USER:
	        case CPSR_MODE_SYSTEM:
	        default:
	          DIE_NOW(0, "LDM: exception return form sys/usr mode!");
	      }
    	  if ((modeSpsr & CPSR_MODE_FIELD) == CPSR_MODE_USER)
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
	    return context->R15;
	  }
	  else
	  {
    	return context->R15+4;
  	  }
	}
}

/* load dual */
u32int ldrdInstruction(GCONTXT * context)
{
  u32int instr = context->endOfBlockInstr;

  u32int condcode  = (instr & 0xF0000000) >> 28;
  u32int prePost   = instr & 0x01000000;
  u32int upDown    = instr & 0x00800000;
  u32int regOrImm  = instr & 0x00400000; // 0 = reg, 1 = imm
  u32int writeback = instr & 0x00200000;

  u32int regSrc = (instr & 0x000F0000) >> 16;
  u32int regDst = (instr & 0x0000F000) >> 12;
  u32int regDst2 = regDst+1;

#ifdef DATA_MOVE_TRACE
  printf("LDRD instruction: %08x @ PC = %08x\n", instr, context->R15);
#endif

  u32int cpsrCC = (context->CPSR & 0xF0000000) >> 28;
  if (!evalCC(condcode, cpsrCC))
  {
    // condition not met! allright, we're done here. next instruction...
    return context->R15 + 4;
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

  if (wback && ((regDst == 15) || (regSrc == regDst) || (regSrc == regDst2)) )
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

  u32int valueLoaded = 
    context->hardwareLibrary->loadFunction(context->hardwareLibrary, WORD, address);
  u32int valueLoaded2 = 
    context->hardwareLibrary->loadFunction(context->hardwareLibrary, WORD, address+4);
  // put loaded values to their registers
  storeGuestGPR(regDst,  valueLoaded,  context);
  storeGuestGPR(regDst2, valueLoaded2, context);

#ifdef DATA_MOVE_TRACE
  printf("LDRD: valueLoaded1 = %x valueLoaded2 = %x\n", valueLoaded, valueLoaded2);
#endif

  if (wback)
  {
    // Rn = offsetAddr;
    storeGuestGPR(regSrc, offsetAddress, context);
  }
  return (context->R15 + 4);
}

u32int ldrexInstruction(GCONTXT * context)
{
  u32int instr = context->endOfBlockInstr;
#ifdef DATA_MOVE_TRACE
  printf("LDREX instruction: %08x @ PC = %08x\n", instr, context->R15);
#endif

  u32int condcode = (instr & 0xF0000000) >> 28;
  u32int baseReg = (instr & 0x000F0000) >> 16;
  u32int regDest = (instr & 0x0000F000) >> 12;

  u32int cpsrCC = (context->CPSR & 0xF0000000) >> 28;
  if (!evalCC(condcode, cpsrCC))
  {
    // condition not met! allright, we're done here. next instruction...
    return context->R15 + 4;
  }
  
  if ((baseReg == 15) || (regDest == 15))
  {
    DIE_NOW(0, "LDREX unpredictable case (PC used).");
  }

  u32int baseVal = loadGuestGPR(baseReg, context);
  u32int value =
      context->hardwareLibrary->loadFunction(context->hardwareLibrary, WORD, baseVal);
#ifdef DATA_MOVE_TRACE
  printf("LDREX instruction: baseVal = %08x loaded %x, store to %x\n", baseVal, value, regDest);
#endif
  storeGuestGPR(regDest, value, context);
  
  return context->R15 + 4;
}


u32int ldrexbInstruction(GCONTXT * context)
{
  u32int instr = context->endOfBlockInstr;
#ifdef DATA_MOVE_TRACE
  printf("LDREXB instruction: %08 @ PC = %08x\n", instr, context->R15);
#endif

  u32int condcode = (instr & 0xF0000000) >> 28;
  u32int baseReg = (instr & 0x000F0000) >> 16;
  u32int regDest = (instr & 0x0000F000) >> 12;

  u32int cpsrCC = (context->CPSR & 0xF0000000) >> 28;
  if (!evalCC(condcode, cpsrCC))
  {
    // condition not met! allright, we're done here. next instruction...
    return context->R15 + 4;
  }
  
  if ((baseReg == 15) || (regDest == 15))
  {
    DIE_NOW(0, "LDREXB unpredictable case (PC used).");
  }
  u32int baseVal = loadGuestGPR(baseReg, context);
  // byte zero extended to word...
  u32int value = ((u32int)context->hardwareLibrary->loadFunction(context->hardwareLibrary, BYTE, baseVal) & 0xFF);
  storeGuestGPR(regDest, value, context);
  
  return context->R15 + 4;
}


/*****************************************************************
* Load Register Exclusive Doubleword *
* derives an address from a base register value, loads a 64-bit *
* doubleword from memory, writes it to two registers and *
* marks the physical address as exclusive access *
*****************************************************************/
u32int ldrexdInstruction(GCONTXT * context)
{
  u32int instr = context->endOfBlockInstr;
#ifdef DATA_MOVE_TRACE
  printf("LDREXD instruction: %08x @ PC = %08x\n", instr, context->R15);
#endif

  u32int condcode = (instr & 0xF0000000) >> 28;
  u32int baseReg = (instr & 0x000F0000) >> 16;
  u32int regDest = (instr & 0x0000F000) >> 12;
  
  u32int cpsrCC = (context->CPSR & 0xF0000000) >> 28;
  if (!evalCC(condcode, cpsrCC))
  {
    // condition not met! allright, we're done here. next instruction...
    return context->R15 + 4;
  }
  // must not be PC, destination must be even and not link register
  if ((baseReg == 15) || ((regDest % 2) != 0) || (regDest == 14))
  {
    DIE_NOW(0, "LDREXH unpredictable case (invalid registers).");
  }
  u32int baseVal = loadGuestGPR(baseReg, context);

  u32int value =
      context->hardwareLibrary->loadFunction(context->hardwareLibrary, WORD, baseVal);
  u32int value2 =
      context->hardwareLibrary->loadFunction(context->hardwareLibrary, WORD, baseVal+4);
  storeGuestGPR(regDest, value, context);
  storeGuestGPR(regDest+1, value2, context);
  
  return context->R15 + 4;
}

u32int ldrexhInstruction(GCONTXT * context)
{
  u32int instr = context->endOfBlockInstr;
#ifdef DATA_MOVE_TRACE
  printf("LDREXH instruction: %08x @ PC = %08x\n", instr, context->R15);
#endif

  u32int condcode = (instr & 0xF0000000) >> 28;
  u32int baseReg = (instr & 0x000F0000) >> 16;
  u32int regDest = (instr & 0x0000F000) >> 12;

  u32int cpsrCC = (context->CPSR & 0xF0000000) >> 28;
  if (!evalCC(condcode, cpsrCC))
  {
    // condition not met! allright, we're done here. next instruction...
    return context->R15 + 4;
  }
  
  if ((baseReg == 15) || (regDest == 15))
  {
    DIE_NOW(0, "LDREXH unpredictable case (PC used).");
  }
  u32int baseVal = loadGuestGPR(baseReg, context);
  // halfword zero extended to word...
  u32int value =
    ((u32int)context->hardwareLibrary->loadFunction(context->hardwareLibrary, HALFWORD, baseVal) & 0xFFFF);
  storeGuestGPR(regDest, value, context);
  
  return context->R15 + 4;
}
