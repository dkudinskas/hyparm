#include "common/bit.h"
#include "common/stddef.h"

#include "cpuArch/constants.h"

#include "instructionEmu/interpreter/internals.h"


u32int arithLogicOp(GCONTXT *context, u32int instr, OPTYPE opType, const char *instrString)
{
  u32int nextPC = context->R15;
  u32int regDest = (instr & 0x0000F000) >> 12;

  if (regDest != GPR_PC)
  {
    invalidDataProcTrap(context, instr, instrString);
  }

#ifdef DATA_PROC_TRACE
  printf("%s: %#.8x @ %#.8x" EOL, instrString, instr, context->R15);
#endif

  if (evaluateConditionCode(context, ARM_EXTRACT_CONDITION_CODE(instr)))
  {
    // set-flags case is tricky! depends on guest mode.
    u32int setFlags = (instr & 0x00100000); // S bit on intruction binary respresentation
    // source operand1
    u32int regSrc = (instr & 0x000F0000) >> 16;
    // source operand2 - register or immediate?
    u32int regOrImm = instr & 0x02000000; // 1 = imm, 0 = reg
    if (regOrImm != 0)
    {
      // source operand2 immediate: pc = regSrc +/- ror(immediate)
      u32int imm12 = instr & 0x00000FFF;
      switch (opType)
      {
        case ADD:
          nextPC = loadGuestGPR(regSrc, context) + armExpandImm12(imm12);
          if (regSrc == 0xF)
          {
            nextPC += 8;
          }
          break;
        case SUB:
          nextPC = loadGuestGPR(regSrc, context) - armExpandImm12(imm12);
          if (regSrc == 0xF)
          {
            nextPC += 8;
          }
          break;
        default:
          DIE_NOW(context, "invalid arithLogicOp opType");
      }
    }
    else
    {
      // register case: pc = regSrc + shift(regSrc2)
      u32int regSrc2   =  instr & 0x0000000F;
      u32int shiftType = (instr & 0x00000060) >> 5;
      u32int shamt = 0;
      if ((instr & 0x00000010) == 0)
      {
        // shift amount is an immediate field
        u32int imm5 = (instr & 0xF80) >> 7;
        u8int carryFlag = (context->CPSR & 0x20000000) >> 29;
        shiftType = decodeShiftImmediate(shiftType, imm5, &shamt);
        switch (opType)
        {
          case ADD:
            nextPC = loadGuestGPR(regSrc, context) + shiftVal(loadGuestGPR(regSrc2, context), shiftType, shamt, &carryFlag);
            if (regSrc == GPR_PC)
            {
              nextPC += 8;
            }
            break;
          case SUB:
            nextPC = loadGuestGPR(regSrc, context) - shiftVal(loadGuestGPR(regSrc2, context), shiftType, shamt, &carryFlag);
            if (regSrc == GPR_PC)
            {
              nextPC += 8;
            }
            break;
          case MOV:
            // cant be shifted - mov shifted reg is a pseudo instr
            if (shamt != 0)
            {
              DIE_NOW(context, "MOV PC, Rn cant be shifted - that is a pseudo instr");
            }
            nextPC = loadGuestGPR(regSrc2, context);
            break;
          default:
            DIE_NOW(context, "invalid arithLogicOp opType");
        }
      }
      else
      {
        // If shift amount is in register and any of the register
        // operands are PC then instruction unpredictable
        DIE_NOW(context, "unpredictable instruction <dataProc> PC, Rn, Rm, Rs");
      }
    }

    if (setFlags)
    {
      if (regDest == GPR_PC)
      {
        // copy SPSR to CPSR
        switch (context->CPSR & PSR_MODE)
        {
          case PSR_FIQ_MODE:
            context->CPSR = context->SPSR_FIQ;
            break;
          case PSR_IRQ_MODE:
            context->CPSR = context->SPSR_IRQ;
            break;
          case PSR_SVC_MODE:
            context->CPSR = context->SPSR_SVC;
            break;
          case PSR_ABT_MODE:
            context->CPSR = context->SPSR_ABT;
            break;
          case PSR_UND_MODE:
            context->CPSR = context->SPSR_UND;
            break;
          default:
            DIE_NOW(context, "arithLogicOp: no SPSR for current guest mode");
        }

        // Align PC
        if (context->CPSR & PSR_T_BIT)
        {
          nextPC &= ~1;
        }
        else
        {
          nextPC &= ~3;
        }
      }
      else
      {
        DIE_NOW(context, "arithLogicOp: unimplemented set flags case");
      }
    }

    if (nextPC & 1)
    {
      DIE_NOW(context, "Interworking branch not allowed");
    }
    return nextPC;
  }
  else
  {
    nextPC = context->R15 + ARM_INSTRUCTION_SIZE;
    return nextPC;
  }
}

/* expand immediate12 field of instruction */
u32int armExpandImm12(u32int imm12)
{
  // <7:0> immediate value, zero extended to 32
  u32int immVal = imm12 & 0xFF;
  // <11:8>*2 is the rotate amount
  u32int shamt = ((imm12 & 0xF00) >> 8) * 2;
  // rotate right!
  return rorVal(immVal, shamt);
}

// take shift type field from instr, return shift type
u32int decodeShift(u32int instrShiftType)
{
  instrShiftType = instrShiftType & 0x3;
  switch (instrShiftType)
  {
    case 0:
      return SHIFT_TYPE_LSL;
    case 1:
      return SHIFT_TYPE_LSR;
    case 2:
      return SHIFT_TYPE_ASR;
    case 3:
      return SHIFT_TYPE_ROR;
    default:
      DIE_NOW(NULL, "voodoo dolls everywhere!");
  } // switch ends
}

// take the imm5 shift amount and shift type field from instr
// returns shift type, and adjusts shift amount
u32int decodeShiftImmediate(u32int instrShiftType, u32int imm5, u32int * shamt)
{
  instrShiftType = instrShiftType & 0x3;
  *shamt = imm5 & 0x1F;
  switch (instrShiftType)
  {
    case SHIFT_TYPE_LSR:
    case SHIFT_TYPE_ASR:
      if (*shamt == 0)
      {
        *shamt = 32;
      }
    case SHIFT_TYPE_LSL:
      return instrShiftType;
    case SHIFT_TYPE_RRX:
      if (*shamt == 0)
      {
        *shamt = 1;
        return SHIFT_TYPE_RRX;
      }
      return SHIFT_TYPE_ROR;
    default:
      DIE_NOW(NULL, "decodeShiftImmediate: voodoo dolls everywhere!");
  }
}

bool evaluateConditionCode(GCONTXT *context, u32int conditionCode)
{
  switch (conditionCode)
  {
    case CC_EQ:
      return context->CPSR & PSR_CC_FLAG_Z_BIT;
    case CC_NE:
      return !(context->CPSR & PSR_CC_FLAG_Z_BIT);
    case CC_HS:
      return context->CPSR & PSR_CC_FLAG_C_BIT;
    case CC_LO:
      return !(context->CPSR & PSR_CC_FLAG_C_BIT);
    case CC_MI:
      return context->CPSR & PSR_CC_FLAG_N_BIT;
    case CC_PL:
      return !(context->CPSR & PSR_CC_FLAG_N_BIT);
    case CC_VS:
      return context->CPSR & PSR_CC_FLAG_V_BIT;
    case CC_VC:
      return !(context->CPSR & PSR_CC_FLAG_V_BIT);
    case CC_HI:
      return (context->CPSR & PSR_CC_FLAG_C_BIT) && !(context->CPSR & PSR_CC_FLAG_Z_BIT);
    case CC_LS:
      return !(context->CPSR & PSR_CC_FLAG_C_BIT) || (context->CPSR & PSR_CC_FLAG_Z_BIT);
    case CC_GE:
      return testBitsEqual(context->CPSR, PSR_CC_FLAG_N_BIT, PSR_CC_FLAG_V_BIT);
    case CC_LT:
      return testBitsNotEqual(context->CPSR, PSR_CC_FLAG_N_BIT, PSR_CC_FLAG_V_BIT);
    case CC_GT:
      return !(context->CPSR & PSR_CC_FLAG_Z_BIT)
          && testBitsEqual(context->CPSR, PSR_CC_FLAG_N_BIT, PSR_CC_FLAG_V_BIT);
    case CC_LE:
      return (context->CPSR & PSR_CC_FLAG_Z_BIT)
          || testBitsNotEqual(context->CPSR, PSR_CC_FLAG_N_BIT, PSR_CC_FLAG_V_BIT);
    case CC_AL:
      return TRUE;
    default:
      return FALSE;
  }
}

void invalidDataProcTrap(GCONTXT *context, u32int instruction, const char *message)
{
  printf("%#.8x @ %#.8x should not have trapped!" EOL, instruction, context->R15);
  DIE_NOW(context, message);
}

/* function to load a register value, evaluates modes. */
u32int loadGuestGPR(u32int regSrc, GCONTXT *context)
{
  /*
   * FIXME: use context as first argument like every other func
   */
  u32int guestMode = context->CPSR & PSR_MODE;
  u32int value = 0;

  if ((regSrc < 8) || (regSrc == 15))
  {
    // dont care about modes here. just get the value.
    u32int * ldPtr = &(context->R0);
    ldPtr = (u32int*)( (u32int)ldPtr + 4 * regSrc);
    value = *ldPtr;
  }
  else
  {
    u32int * ldPtr = 0;
    if ( (regSrc >=8) && (regSrc <= 12) )
    {
      if (guestMode == PSR_FIQ_MODE)
      {
        ldPtr = &(context->R8_FIQ);
      }
      else
      {
        ldPtr = &(context->R8);
      }
      value = ldPtr[regSrc-8];
    }
    else
    {
      // R13 / R14 left
      switch (guestMode)
      {
        case PSR_USR_MODE:
        case PSR_SYS_MODE:
          ldPtr = (regSrc == 13) ? (&(context->R13_USR)) : (&(context->R14_USR));
          break;
        case PSR_FIQ_MODE:
          ldPtr = (regSrc == 13) ? (&(context->R13_FIQ)) : (&(context->R14_FIQ));
          break;
        case PSR_IRQ_MODE:
          ldPtr = (regSrc == 13) ? (&(context->R13_IRQ)) : (&(context->R14_IRQ));
          break;
        case PSR_SVC_MODE:
          ldPtr = (regSrc == 13) ? (&(context->R13_SVC)) : (&(context->R14_SVC));
          break;
        case PSR_ABT_MODE:
          ldPtr = (regSrc == 13) ? (&(context->R13_ABT)) : (&(context->R14_ABT));
          break;
        case PSR_UND_MODE:
          ldPtr = (regSrc == 13) ? (&(context->R13_UND)) : (&(context->R14_UND));
          break;
        default:
          DIE_NOW(context, "loadGuestGPR: invalid CPSR mode!");
      } // switch ends
      value = *ldPtr;
    } // R13/R14 ends
  } // mode specific else ends
  return value;
}

// rotate right function
u32int rorVal(u32int value, u32int ramt)
{
  u32int retVal = value;
  u32int leftToRotate = ramt;
  while (leftToRotate > 0)
  {
    u32int rightMostBit = retVal & 0x1;
    retVal >>= 1;
    if (rightMostBit == 1)
    {
      retVal |= 0x80000000;
    }
    else
    {
      retVal &= 0x7FFFFFFF;
    }
    leftToRotate--;
  }
  return retVal;
}

// generic any type shift function, changes input_parameter(carryFlag) value
u32int shiftVal(u32int value, u8int shiftType, u32int shamt, u8int * carryFlag)
{
  // RRX can only shift right by 1
  if ((shiftType == SHIFT_TYPE_RRX) && (shamt != 1))
  {
    DIE_NOW(NULL, "shiftVal: type rrx, but shamt not 1!");
  }

  u32int retVal = 0;
  if (shamt == 0)
  {
    return value;
  }
  else
  {
     switch(shiftType)
     {
       case SHIFT_TYPE_ROR:
         // ror doesnt adjust carry flag!
         retVal = rorVal(value, shamt);
         break;
       case SHIFT_TYPE_LSL:
         retVal = value << shamt;
         break;
       case SHIFT_TYPE_LSR:
         retVal = value >> shamt;
         break;
       case SHIFT_TYPE_ASR:
       case SHIFT_TYPE_RRX:
       default:
        DIE_NOW(NULL, "shiftVal: unimplemented shiftType");
     } // switch
  } // else
  return retVal;
}

/* function to store a register value, evaluates modes. */
void storeGuestGPR(u32int regDest, u32int value, GCONTXT *context)
{
  /*
   * FIXME: use context as first argument like every other func
   */
  u32int guestMode = (context->CPSR) & PSR_MODE;

  if ((regDest < 8) || (regDest == 15))
  {
    // dont care about modes here. just store.
    u32int * strPtr = &(context->R0);
    strPtr = (u32int*)( (u32int)strPtr + 4 * regDest);
    *strPtr = value;
    return;
  }
  else
  {
    u32int * strPtr = 0;
    if ( (regDest >=8) && (regDest <= 12) )
    {
      if (guestMode == PSR_FIQ_MODE)
      {
        strPtr = &(context->R8_FIQ);
      }
      else
      {
        strPtr = &(context->R8);
      }
      strPtr[regDest-8] = value;
      return;
    }
    else
    {
      // R13 / R14 left
      switch (guestMode)
      {
        case PSR_USR_MODE:
        case PSR_SYS_MODE:
          strPtr = (regDest == 13) ? (&(context->R13_USR)) : (&(context->R14_USR));
          break;
        case PSR_FIQ_MODE:
          strPtr = (regDest == 13) ? (&(context->R13_FIQ)) : (&(context->R14_FIQ));
          break;
        case PSR_IRQ_MODE:
          strPtr = (regDest == 13) ? (&(context->R13_IRQ)) : (&(context->R14_IRQ));
          break;
        case PSR_SVC_MODE:
          strPtr = (regDest == 13) ? (&(context->R13_SVC)) : (&(context->R14_SVC));
          break;
        case PSR_ABT_MODE:
          strPtr = (regDest == 13) ? (&(context->R13_ABT)) : (&(context->R14_ABT));
          break;
        case PSR_UND_MODE:
          strPtr = (regDest == 13) ? (&(context->R13_UND)) : (&(context->R14_UND));
          break;
        default:
          DIE_NOW(context, "storeGuestGPR: invalid CPSR mode!");
      } // switch ends
      *strPtr = value;
    } // R13/R14 ends
  } // mode specific else ends
}

#ifdef CONFIG_GUEST_TEST
/*
 * This function is used in unit tests. It evaluates the value passed to the BKPT instruction.
 * Current values:
 * 0      pass
 * other  fail
 */
void evalBkptVal(GCONTXT *context, u32int value) {
  switch(value) {
    case 0:
      DIE_NOW(context, "test passed");
    default:
      DIE_NOW(context, "test failed");
  }
}
#endif
