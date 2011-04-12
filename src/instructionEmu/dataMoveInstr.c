#include "common/debug.h"
#include "common/defines.h"

#include "guestManager/blockCache.h"

#include "vm/omap35xx/serial.h"

#include "instructionEmu/commonInstrFunctions.h"
#include "instructionEmu/dataMoveInstr.h"

#include "memoryManager/cp15coproc.h"
#include "memoryManager/mmu.h"


void invalidDataMoveTrap(char * msg, GCONTXT * gc)
{
  serial_putstring("ERROR: ");
  serial_putstring(msg);
  serial_putstring(" ");
  serial_putint(gc->endOfBlockInstr);
  serial_putstring(" @ ");
  serial_putint(gc->R15);
  serial_putstring(" should not have trapped!");
  serial_newline();
  dumpGuestContext(gc);
  while(TRUE)
  {
    // infinite loop
  }
}
/***********************************************************************************
 ***********************************************************************************
 ************************** STORE FUNCTIONS ****************************************
 ***********************************************************************************
 ***********************************************************************************/
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
#endif

u32int strInstruction(GCONTXT * context)
{
  u32int instr = context->endOfBlockInstr;
  u32int condcode = (instr & 0xF0000000) >> 28;
  u32int regOrImm = instr & 0x02000000; // 1 = reg, 0 = imm
  u32int preOrPost = instr & 0x01000000; // 1 = pre, 0 = post
  u32int incOrDec = instr & 0x00800000; // 1 = inc, 0 = dec
  u32int writeBack = instr & 0x00200000; // 1 = writeBack indexing, 0 = no writeback
  u32int regDst = (instr & 0x000F0000) >> 16; // Base Destination address
  u32int regSrc = (instr & 0x0000F000) >> 12; // Source value from this register...
#ifdef DATA_MOVE_TRACE
  serial_putstring("strInstr: ");
  serial_putstring("regOrImm=");
  serial_putint(regOrImm);
  serial_putstring(" preOrPost=");
  serial_putint(preOrPost);
  serial_putstring(" incOrDec=");
  serial_putint(incOrDec);
  serial_putstring(" writeBack=");
  serial_putint(writeBack);
  serial_putstring(" regDst=");
  serial_putint_nozeros(regDst);
  serial_putstring(" regSrc=");
  serial_putint_nozeros(regSrc);
  serial_newline();
#endif
  u32int offsetAddress = 0;
  u32int baseAddress = loadGuestGPR(regDst, context);
  u32int valueToStore = loadGuestGPR(regSrc, context);
  u32int cpsrCC = (context->CPSR & 0xF0000000) >> 28;
  if (!evalCC(condcode, cpsrCC))
  {
// condition not met! allright, we're done here. next instruction...
#ifdef DATA_MOVE_TRACE
    serial_putstring("strInstr: ");
    serial_putstring("condition not met");
    serial_newline();
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
    serial_putstring("strInstr: ");
    serial_putstring("imm case: ");
#endif
    // immediate case
    u32int imm32 = instr & 0x00000FFF;
#ifdef DATA_MOVE_TRACE
    serial_putstring("imm32=");
    serial_putint_nozeros(imm32);
    serial_putstring(" baseAddress=");
    serial_putint(baseAddress);
    serial_putstring(" valueToStore=");
    serial_putint(valueToStore);
    serial_putstring(" offsetAddress=");
    serial_putint(offsetAddress);
#endif
    // offsetAddress = if increment then base + imm32 else base - imm32
    if (incOrDec != 0)
    {
      offsetAddress = baseAddress + imm32;
#ifdef DATA_MOVE_TRACE
      serial_putstring(" inc");
      serial_newline();
#endif
    }
    else
    {
      offsetAddress = baseAddress - imm32;
#ifdef DATA_MOVE_TRACE
      serial_putstring(" dec");
      serial_newline();
#endif
    }
  } // Immediate case ends
  else
  {
#ifdef DATA_MOVE_TRACE
    serial_putstring("strInstr: ");
    serial_putstring("reg case: ");
#endif
    // register case
    u32int regDst2 = instr & 0x0000000F;
    u32int offsetRegisterValue = loadGuestGPR(regDst2, context);
#ifdef DATA_MOVE_TRACE
    serial_putstring("regDst2=");
    serial_putint_nozeros(regDst2);
    serial_putstring(" baseAddress=");
    serial_putint(baseAddress);
    serial_putstring(" offsetRegisterValue=");
    serial_putint(offsetRegisterValue);
    serial_putstring(" valueToStore=");
    serial_putint(valueToStore);
    serial_newline();
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
    dumpGuestContext(context);
    DIE_NOW(0, "STR Rd [Rn, Rm/#imm] unaligned address!\n");
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
  #ifdef CONFIG_BLOCK_COPY
  return (context->PCOfLastInstruction + 4);
  #else
  return (context->R15 + 4);
  #endif
}
  
#ifdef CONFIG_BLOCK_COPY
u32int* strbPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  DIE_NOW(0, "strb PCFunct unfinished\n");
  return 0;
}
#endif

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
    #ifdef CONFIG_BLOCK_COPY
    return context->PCOfLastInstruction + 4;
    #else
    return context->R15 +4;
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
		#ifdef CONFIG_BLOCK_COPY
		return context->PCOfLastInstruction;
		#else
		return context->R15;
		#endif
    }
    // if usr can write, continue
  }
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
  #ifdef CONFIG_BLOCK_COPY
  return (context->PCOfLastInstruction + 4);
  #else
  return (context->R15 + 4);
  #endif
}

#ifdef CONFIG_BLOCK_COPY
u32int* strhtPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  DIE_NOW(0, "strht PCFunct unfinished\n");
  return 0;
}
#endif

u32int strhtInstruction(GCONTXT * context)
{
  dumpGuestContext(context);
  DIE_NOW(0, "STRHT unfinished\n");
  return 0;
}

#ifdef CONFIG_BLOCK_COPY
u32int* strhPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  DIE_NOW(0, "strh PCFunct unfinished\n");
  return 0;
}
#endif

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
    dumpGuestContext(context);
    DIE_NOW(0, "STRH Rd [Rn, Rm/#imm] unaligned address!\n");
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
  #ifdef CONFIG_BLOCK_COPY
    return (context->PCOfLastInstruction + 4);
  #else
    return (context->R15 + 4);
  #endif
}

#ifdef CONFIG_BLOCK_COPY
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
#endif

/* This function can also get called when a data abort occurs and a store is emulated (emulateLoadStoreGeneric) */
u32int stmInstruction(GCONTXT * context)
{
  u32int instr = context->endOfBlockInstr;
#ifdef DATA_MOVE_TRACE
  serial_putstring("STM instruction: ");
  serial_putint(instr);
  serial_putstring(" @ PC=");
# ifdef CONFIG_BLOCK_COPY
  serial_putint(context->PCOfLastInstruction);
# else
  serial_putint(context->R15);
# endif
  serial_newline();
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
      serial_putstring("*(");
      serial_putint(address);
      serial_putstring(") = ");
      serial_putstring("R[");
      serial_putint_nozeros(i);
      serial_putstring("] =");
      serial_putint(loadGuestGPR(i, context));
      serial_newline();
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
    // *(address)= PC+8 - architectural feature due to pipeline.. -> loadGuestGPR will take care of this
    context->hardwareLibrary->storeFunction(context->hardwareLibrary, WORD,
                                            address, (loadGuestGPR(15, context)));
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
  return context->PCOfLastInstruction+4;
  #else
  return context->R15+4;
  #endif

}
/* store dual */

#ifdef CONFIG_BLOCK_COPY
u32int* strdPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  DIE_NOW(0, "strd PCFunct unfinished\n");
  return 0;
}
#endif

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
  serial_putstring("STRD instruction: ");
  serial_putint(instr);
  serial_putstring(" @ PC=");
# ifdef CONFIG_BLOCK_COPY
  serial_putint(context->PCOfLastInstruction);
# else
  serial_putint(context->R15);
# endif
  serial_newline();
#endif
  u32int cpsrCC = (context->CPSR & 0xF0000000) >> 28;
  if (!evalCC(condcode, cpsrCC))
  {
    // condition not met! allright, we're done here. next instruction...
    #ifdef CONFIG_BLOCK_COPY
      return context->PCOfLastInstruction+4;
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
    serial_putstring("imm32=");
    serial_putint_nozeros(imm32);
    serial_putstring(" baseAddress=");
    serial_putint(baseAddress);
    serial_putstring(" valueToStore=");
    serial_putint(valueToStore);
    serial_putstring(" offsetAddress=");
    serial_putint(offsetAddress);
    serial_newline();
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
    serial_putstring("Rm=");
    serial_putint_nozeros(regDst2);
    serial_putstring(" baseAddress=");
    serial_putint(baseAddress);
    serial_putstring(" valueToStore=");
    serial_putint(valueToStore);
    serial_putstring(" offsetRegisterValue=");
    serial_putint(offsetRegisterValue);
    serial_newline();
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
  serial_putstring("store address = ");
  serial_putint(address);
  serial_newline();
#endif
  // *storeAddress = if sourceValue is PC then valueToStore+8 else valueToStore;
  // valueToStore = (regSrc == 15) ? (valueToStore+8) : valueToStore; ->Not necessary loadGuestGPR already added 8
#ifdef DATA_MOVE_TRACE
  serial_putstring("store val1 = ");
  serial_putint(valueToStore);
  serial_putstring(" store val2 = ");
  serial_putint(valueToStore2);
  serial_newline();
#endif
  context->hardwareLibrary->storeFunction(context->hardwareLibrary, WORD, address, valueToStore);
  context->hardwareLibrary->storeFunction(context->hardwareLibrary, WORD, address+4, valueToStore2);
  if (wback)
  {
    // Rn = offsetAddr;
    storeGuestGPR(regDst, offsetAddress, context);
  }
  #ifdef CONFIG_BLOCK_COPY
    return context->PCOfLastInstruction+4;
  #else
    return context->R15 + 4;
  #endif
}

#ifdef CONFIG_BLOCK_COPY
u32int* strexPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  DIE_NOW(0, "strex PCFunct unfinished\n");
  return 0;
}
#endif

u32int strexInstruction(GCONTXT * context)
{
  u32int instr = context->endOfBlockInstr;
#ifdef DATA_MOVE_TRACE
  serial_putstring("STREX instruction: ");
  serial_putint(instr);
  serial_putstring(" @ PC=");
# ifdef CONFIG_BLOCK_COPY
  serial_putint(context->PCOfLastInstruction);
# else
  serial_putint(context->R15);
# endif
  serial_newline();
#endif
  u32int condcode = (instr & 0xF0000000) >> 28;
  u32int regN = (instr & 0x000F0000) >> 16;
  u32int regD = (instr & 0x0000F000) >> 12;
  u32int regT = (instr & 0x0000000F);
  u32int cpsrCC = (context->CPSR & 0xF0000000) >> 28;
  if (!evalCC(condcode, cpsrCC))
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
  serial_putstring("STREX instruction: address = ");
  serial_putint(address);
  serial_putstring(" valToStore ");
  serial_putint(valToStore);
  serial_putstring(", valueFrom ");
  serial_putint(regT);
  serial_newline();
  }
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

#ifdef CONFIG_BLOCK_COPY
u32int* strexbPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  DIE_NOW(0, "strexb PCFunct unfinished\n");
  return 0;
}
#endif

u32int strexbInstruction(GCONTXT * context)
{
  u32int instr = context->endOfBlockInstr;
#ifdef DATA_MOVE_TRACE
  serial_putstring("STREXB instruction: ");
  serial_putint(instr);
  serial_putstring(" @ PC=");
# ifdef CONFIG_BLOCK_COPY
  serial_putint(context->R15);
# else
  serial_putint(context->PCOfLastInstruction);
# endif
  serial_newline();
#endif
  u32int condcode = (instr & 0xF0000000) >> 28;
  u32int regN = (instr & 0x000F0000) >> 16;
  u32int regD = (instr & 0x0000F000) >> 12;
  u32int regT = (instr & 0x0000000F);
  u32int cpsrCC = (context->CPSR & 0xF0000000) >> 28;
  if (!evalCC(condcode, cpsrCC))
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

#ifdef CONFIG_BLOCK_COPY
u32int* strexdPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  DIE_NOW(0, "strexd PCFunct unfinished\n");
  return 0;
}
#endif

u32int strexdInstruction(GCONTXT * context)
{
  DIE_NOW(0, "strexdInstruction is executed but not yet checked for blockCopyCompatibility");
  u32int instr = context->endOfBlockInstr;
#ifdef DATA_MOVE_TRACE
  serial_putstring("STREXD instruction: ");
  serial_putint(instr);
  serial_putstring(" @ PC=");
  serial_putint(context->R15);
  serial_newline();
#endif
  u32int condcode = (instr & 0xF0000000) >> 28;
  u32int regN = (instr & 0x000F0000) >> 16;
  u32int regD = (instr & 0x0000F000) >> 12;
  u32int regT = (instr & 0x0000000F);
  u32int cpsrCC = (context->CPSR & 0xF0000000) >> 28;
  if (!evalCC(condcode, cpsrCC))
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
  
  #ifdef CONFIG_BLOCK_COPY
  return context->PCOfLastInstruction+4;
  #else
  return context->R15 + 4;
  #endif
}

#ifdef CONFIG_BLOCK_COPY
u32int* strexhPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  DIE_NOW(0, "strexh PCFunct unfinished\n");
  return 0;
}
#endif

u32int strexhInstruction(GCONTXT * context)
{
  DIE_NOW(0, "strexhInstruction is executed but not yet checked for blockCopyCompatibility");
  u32int instr = context->endOfBlockInstr;
#ifdef DATA_MOVE_TRACE
  serial_putstring("STREXH instruction: ");
  serial_putint(instr);
  serial_putstring(" @ PC=");
  serial_putint(context->R15);
  serial_newline();
#endif
  u32int condcode = (instr & 0xF0000000) >> 28;
  u32int regN = (instr & 0x000F0000) >> 16;
  u32int regD = (instr & 0x0000F000) >> 12;
  u32int regT = (instr & 0x0000000F);
  u32int cpsrCC = (context->CPSR & 0xF0000000) >> 28;
  if (!evalCC(condcode, cpsrCC))
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
#ifdef CONFIG_BLOCK_COPY
u32int* ldrhtPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  DIE_NOW(0, "ldrht PCFunct unfinished\n");
  return 0;
}
#endif

u32int ldrhtInstruction(GCONTXT * context)
{
  DIE_NOW(0, "ldrhtInstruction is executed but not yet checked for blockCopyCompatibility");
  dumpGuestContext(context);
  DIE_NOW(0, "LDRHT unfinished\n");
  return 0;
}

#ifdef CONFIG_BLOCK_COPY
u32int* ldrhPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  DIE_NOW(0, "ldrh PCFunct unfinished\n");
  return 0;
}
#endif

u32int ldrhInstruction(GCONTXT * context)
{
  DIE_NOW(0, "ldrhInstruction is executed but not yet checked for blockCopyCompatibility");
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
    #ifdef CONFIG_BLOCK_COPY
	return context->PCOfLastInstruction+4;
	#else
	return context->R15 + 4;
	#endif
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
  serial_putstring("ldrhInstr: ");
  serial_putint(instr);
  serial_putstring(" @ ");
  serial_putint(context->R15);
  serial_putstring(" R[");
  serial_putint_nozeros(regDst);
  serial_putstring("]=");
  serial_putint(valueLoaded);
  serial_newline();
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
  return context->R15 + 4;
  #endif
}

#ifdef CONFIG_BLOCK_COPY
u32int* ldrbPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  DIE_NOW(0, "ldrb PCFunct unfinished\n");
  return 0;
}
#endif

u32int ldrbInstruction(GCONTXT * context)
{
  DIE_NOW(0, "ldrbInstruction is executed but not yet checked for blockCopyCompatibility");
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
      #ifdef CONFIG_BLOCK_COPY
        return context->PCOfLastInstruction+4;
      #else
        return context->R15 + 4;
      #endif
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
  #ifdef CONFIG_BLOCK_COPY
    return context->PCOfLastInstruction+4;
  #else
    return context->R15 + 4;
  #endif
}

#ifdef CONFIG_BLOCK_COPY
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
    scratchReg = findUnusedRegister(srcReg1, -1, -1);
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
#endif

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
    dumpGuestContext(context);
    DIE_NOW(0, "LDR Rd [Rn, Rm/#imm] unaligned address!\n");
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
  u32int valueLoaded =
      context->hardwareLibrary->loadFunction(context->hardwareLibrary, WORD, address);
  // LDR loading to PC should load a word-aligned value
  if ((regDst == 15) && ((valueLoaded & 0x3) != 0))
  {
    dumpGuestContext(context);
    serial_putstring("LDR: regDst = ");
    serial_putint(regDst);
    serial_putstring(", from addr ");
    serial_putint(valueLoaded);
    serial_newline();
    DIE_NOW(0, "LDR Rd [Rn, Rm/#imm] load unaligned value to PC!\n");
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
    return context->R15; //PC is destination of ldr so we have to use this new value
  }
  else
  {
    #ifdef CONFIG_BLOCK_COPY
	return context->PCOfLastInstruction+4;
	#else
	return context->R15 + 4;
	#endif //just go to next instruction
  }
}

#ifdef CONFIG_BLOCK_COPY
u32int* popLdrPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  DIE_NOW(0, "popLdr PCFunct unfinished\n");
  return 0;
}
#endif

u32int popLdrInstruction(GCONTXT * context)
{
  DIE_NOW(0, "popLdrInstruction is executed but not yet checked for blockCopyCompatibility");
  DIE_NOW(0, "POP unfinished\n");
  return 0;
}

#ifdef CONFIG_BLOCK_COPY
u32int* popLdmPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  DIE_NOW(0, "popLdm PCFunct unfinished\n");
  return 0;
}
#endif

u32int popLdmInstruction(GCONTXT * context)
{
  // regList must include PC, otherwise wouldnt have trapped!
  // just treat as a regular load multiple.
  return ldmInstruction(context);
}

#ifdef CONFIG_BLOCK_COPY
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
#endif

u32int ldmInstruction(GCONTXT * context)
{//Checked for blockCopy functionality
  u32int instr = context->endOfBlockInstr;
#ifdef DATA_MOVE_TRACE
  serial_putstring("LDM instruction: ");
  serial_putint(instr);
  serial_putstring(" @ PC=");
# ifdef CONFIG_BLOCK_COPY
  serial_putint(context->PCOfLastInstruction);
# else
  serial_putint(context->R15);
# endif
  serial_newline();
#endif
  u32int condcode = (instr & 0xF0000000) >> 28;
  u32int prePost = instr & 0x01000000;
  u32int upDown = instr & 0x00800000;
  u32int forceUser = instr & 0x00400000;
  u32int writeback = instr & 0x00200000;
  u32int baseReg = (instr & 0x000F0000) >> 16;
  u32int regList = instr & 0x0000FFFF;
  u32int baseAddress = loadGuestGPR(baseReg, context);
  if ( (baseReg == 15) || (countBitsSet(regList) == 0) )
  {
    DIE_NOW(0, "LDM UNPREDICTABLE: base=PC or no registers in list");
  }
  u32int cpsrCC = (context->CPSR & 0xF0000000) >> 28;
  if (!evalCC(condcode, cpsrCC))
  {
    // condition not met! allright, we're done here. next instruction...
    #ifdef CONFIG_BLOCK_COPY
	return context->PCOfLastInstruction+4;
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
      context->CPSR = (context->CPSR & ~0x1f) | CPSR_MODE_USER;
    }
  }
  
  int i = 0;
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
      serial_putstring("R[");
      serial_putint_nozeros(i);
      serial_putstring("] = *(");
      serial_putint(address);
      serial_putstring(") = ");
      serial_putint(valueLoaded);
      serial_newline();
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
    return context->R15; //PC is in RegList so it is important that we load R15 instead of PCOfLastInstruction.  Because we have fetched the
                        //PC from memory and stored it in context->R15 (see above, the loop over the registers)
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
/* load dual */
#ifdef CONFIG_BLOCK_COPY
u32int* ldrdPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  DIE_NOW(0, "ldrd PCFunct unfinished\n");
  return 0;
}
#endif

u32int ldrdInstruction(GCONTXT * context)
{
  DIE_NOW(0, "ldrdInstruction is executed but not yet checked for blockCopyCompatibility");
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
  serial_putstring("LDRD instruction: ");
  serial_putint(instr);
  serial_putstring(" @ PC=");
  serial_putint(context->R15);
  serial_newline();
#endif
  u32int cpsrCC = (context->CPSR & 0xF0000000) >> 28;
  if (!evalCC(condcode, cpsrCC))
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
    serial_putstring("imm32=");
    serial_putint_nozeros(imm32);
    serial_putstring(" baseAddress=");
    serial_putint(baseAddress);
    serial_putstring(" offsetAddress=");
    serial_putint(offsetAddress);
    serial_newline();
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
    serial_putstring("Rm=");
    serial_putint_nozeros(regSrc2);
    serial_putstring(" baseAddress=");
    serial_putint(baseAddress);
    serial_putstring(" offsetRegisterValue=");
    serial_putint(offsetRegisterValue);
    serial_newline();
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
  serial_putstring("LDRD: load address = ");
  serial_putint(address);
  serial_newline();
#endif
  u32int valueLoaded = 
    context->hardwareLibrary->loadFunction(context->hardwareLibrary, WORD, address);
  u32int valueLoaded2 = 
    context->hardwareLibrary->loadFunction(context->hardwareLibrary, WORD, address+4);
  // put loaded values to their registers
  storeGuestGPR(regDst,  valueLoaded,  context);
  storeGuestGPR(regDst2, valueLoaded2, context);
#ifdef DATA_MOVE_TRACE
  serial_putstring("LDRD: valueLoaded1 = ");
  serial_putint(valueLoaded);
  serial_putstring(" valueLoaded2 = ");
  serial_putint(valueLoaded2);
  serial_newline();
#endif
  if (wback)
  {
    // Rn = offsetAddr;
    storeGuestGPR(regSrc, offsetAddress, context);
  }
  #ifdef CONFIG_BLOCK_COPY
  return context->PCOfLastInstruction+4;
  #else
  return context->R15 + 4;
  #endif
}

#ifdef CONFIG_BLOCK_COPY
u32int* ldrexPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  DIE_NOW(0, "ldrex PCFunct unfinished\n");
  return 0;
}
#endif

u32int ldrexInstruction(GCONTXT * context)
{
  u32int instr = context->endOfBlockInstr;
#ifdef DATA_MOVE_TRACE
  serial_putstring("LDREX instruction: ");
  serial_putint(instr);
  serial_putstring(" @ PC=");
# ifdef
  serial_putint(context->PCOfLastInstruction);
# else
  serial_putint(context->R15);
# endif
  serial_newline();
#endif
  u32int condcode = (instr & 0xF0000000) >> 28;
  u32int baseReg = (instr & 0x000F0000) >> 16;
  u32int regDest = (instr & 0x0000F000) >> 12;
  u32int cpsrCC = (context->CPSR & 0xF0000000) >> 28;
  if (!evalCC(condcode, cpsrCC))
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
  u32int value =
      context->hardwareLibrary->loadFunction(context->hardwareLibrary, WORD, baseVal);
#ifdef DATA_MOVE_TRACE
  serial_putstring("LDREX instruction: baseVal = ");
  serial_putint(baseVal);
  serial_putstring(" loaded ");
  serial_putint(value);
  serial_putstring(", store to ");
  serial_putint(regDest);
  serial_newline();
#endif
  storeGuestGPR(regDest, value, context);
  
  #ifdef CONFIG_BLOCK_COPY
    return context->PCOfLastInstruction+4;
  #else
    return context->R15 + 4;
  #endif
}

#ifdef CONFIG_BLOCK_COPY
u32int* ldrexbPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  DIE_NOW(0, "ldrexb PCFunct unfinished\n");
  return 0;
}
#endif

u32int ldrexbInstruction(GCONTXT * context)
{
  u32int instr = context->endOfBlockInstr;
#ifdef DATA_MOVE_TRACE
  serial_putstring("LDREXB instruction: ");
  serial_putint(instr);
  serial_putstring(" @ PC=");
# ifdef CONFIG_BLOCK_COPY
  serial_putint(context->PCOfLastInstruction);
#else
  serial_putint(context->R15);
# endif
  serial_newline();
#endif
  u32int condcode = (instr & 0xF0000000) >> 28;
  u32int baseReg = (instr & 0x000F0000) >> 16;
  u32int regDest = (instr & 0x0000F000) >> 12;
  u32int cpsrCC = (context->CPSR & 0xF0000000) >> 28;
  if (!evalCC(condcode, cpsrCC))
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
  u32int value = ((u32int)context->hardwareLibrary->loadFunction(context->hardwareLibrary, BYTE, baseVal) & 0xFF);
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
#ifdef CONFIG_BLOCK_COPY
u32int* ldrexdPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  DIE_NOW(0, "ldrexd PCFunct unfinished\n");
  return 0;
}
#endif

u32int ldrexdInstruction(GCONTXT * context)
{
  DIE_NOW(0, "ldrexdInstruction is executed but not yet checked for blockCopyCompatibility");
  u32int instr = context->endOfBlockInstr;
#ifdef DATA_MOVE_TRACE
  serial_putstring("LDREXD instruction: ");
  serial_putint(instr);
  serial_putstring(" @ PC=");
  serial_putint(context->R15);
  serial_newline();
#endif
  u32int condcode = (instr & 0xF0000000) >> 28;
  u32int baseReg = (instr & 0x000F0000) >> 16;
  u32int regDest = (instr & 0x0000F000) >> 12;
  
  u32int cpsrCC = (context->CPSR & 0xF0000000) >> 28;
  if (!evalCC(condcode, cpsrCC))
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
  u32int value =
      context->hardwareLibrary->loadFunction(context->hardwareLibrary, WORD, baseVal);
  u32int value2 =
      context->hardwareLibrary->loadFunction(context->hardwareLibrary, WORD, baseVal+4);
  storeGuestGPR(regDest, value, context);
  storeGuestGPR(regDest+1, value2, context);
  
  #ifdef CONFIG_BLOCK_COPY
  return context->PCOfLastInstruction+4;
  #else
  return context->R15 + 4;
  #endif
}

#ifdef CONFIG_BLOCK_COPY
u32int* ldrexhPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  DIE_NOW(0, "ldrexh PCFunct unfinished\n");
  return 0;
}
#endif

u32int ldrexhInstruction(GCONTXT * context)
{
  DIE_NOW(0, "ldrexhInstruction is executed but not yet checked for blockCopyCompatibility");
  u32int instr = context->endOfBlockInstr;
#ifdef DATA_MOVE_TRACE
  serial_putstring("LDREXH instruction: ");
  serial_putint(instr);
  serial_putstring(" @ PC=");
  serial_putint(context->R15);
  serial_newline();
#endif
  u32int condcode = (instr & 0xF0000000) >> 28;
  u32int baseReg = (instr & 0x000F0000) >> 16;
  u32int regDest = (instr & 0x0000F000) >> 12;
  u32int cpsrCC = (context->CPSR & 0xF0000000) >> 28;
  if (!evalCC(condcode, cpsrCC))
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
  u32int value =
    ((u32int)context->hardwareLibrary->loadFunction(context->hardwareLibrary, HALFWORD, baseVal) & 0xFFFF);
  storeGuestGPR(regDest, value, context);
  
  #ifdef CONFIG_BLOCK_COPY
  return context->PCOfLastInstruction+4;
  #else
  return context->R15 + 4;
  #endif
}
