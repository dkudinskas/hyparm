#include "common/bit.h"
#include "common/debug.h"

#include "cpuArch/constants.h"

#include "instructionEmu/commonInstrFunctions.h"


/* a function to serve as a dead-loop if we decode something invalid */
void invalidInstruction(u32int instr, const char *msg)
{
  printf("Invalid instruction detected! %.8x" EOL, instr);
  DIE_NOW(0, msg);
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
      return TEST_BITS_EQUAL(context->CPSR, PSR_CC_FLAG_N_BIT, PSR_CC_FLAG_V_BIT);
    case CC_LT:
      return TEST_BITS_NOT_EQUAL(context->CPSR, PSR_CC_FLAG_N_BIT, PSR_CC_FLAG_V_BIT);
    case CC_GT:
      return !(context->CPSR & PSR_CC_FLAG_Z_BIT)
          && TEST_BITS_EQUAL(context->CPSR, PSR_CC_FLAG_N_BIT, PSR_CC_FLAG_V_BIT);
    case CC_LE:
      return (context->CPSR & PSR_CC_FLAG_Z_BIT)
          || TEST_BITS_NOT_EQUAL(context->CPSR, PSR_CC_FLAG_N_BIT, PSR_CC_FLAG_V_BIT);
    case CC_AL:
      return TRUE;
    default:
      return FALSE;
  }
}

/* function to store a register value, evaluates modes. */
void storeGuestGPR(u32int regDest, u32int value, GCONTXT * context)
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
          invalidInstruction(context->endOfBlockInstr, "storeGuestGPR: invalid CPSR mode!");
      } // switch ends
      *strPtr = value;
    } // R13/R14 ends
  } // mode specific else ends
}

/* function to load a register value, evaluates modes. */
u32int loadGuestGPR(u32int regSrc, GCONTXT * context)
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
          invalidInstruction(context->endOfBlockInstr, "loadGuestGPR: invalid CPSR mode!");
      } // switch ends
      value = *ldPtr;
    } // R13/R14 ends
  } // mode specific else ends
  return value;
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

// generic any type shift function, changes input_parameter(carryFlag) value
u32int shiftVal(u32int value, u8int shiftType, u32int shamt, u8int * carryFlag)
{
  // RRX can only shift right by 1
  if ((shiftType == SHIFT_TYPE_RRX) && (shamt != 1))
  {
    DIE_NOW(0, "shiftVal() type rrx, but shamt not 1!");
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
        DIE_NOW(0,"shiftVal(): unimplemented shiftType.");
     } // switch
  } // else
  return retVal;
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

// take the imm5 shift amount and shift type field from instr
// returns shift type, and adjusts shift amount
u32int decodeShiftImmediate(u32int instrShiftType, u32int imm5, u32int * shamt)
{
  instrShiftType = instrShiftType & 0x3;
  *shamt = imm5 & 0x1F;
  switch (instrShiftType)
  {
    case 0:
      return SHIFT_TYPE_LSL;
    case 1:
      if (*shamt == 0)
      {
        *shamt = 32;
      }
      return SHIFT_TYPE_LSR;
    case 2:
      if (*shamt == 0)
      {
        *shamt = 32;
      }
      return SHIFT_TYPE_ASR;
    case 3:
      if(*shamt == 0)
      {
        *shamt = 1;
        return SHIFT_TYPE_RRX;
      }
      else
      {
        return SHIFT_TYPE_ROR;
      }

    default:
      DIE_NOW(0,"decodeShiftImmediate: voodoo dolls everywhere!");
  } // switch ends

  // compiler happy!
  return 0;
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
      DIE_NOW(0, "voodoo dolls everywhere!");
  } // switch ends
}

#ifdef CONFIG_THUMB2

u32int fetchThumbInstr(u16int *currhwAddress)
{
  u16int narrowInstr = *currhwAddress;
  switch (narrowInstr & THUMB32)
  {
    case THUMB32_1:
    case THUMB32_2:
    case THUMB32_3:
      /*
       * 32-bit Thumb instruction -- need to fetch next halfword.
       */
      return (narrowInstr << 16) | *++currhwAddress;
    default:
      /*
       * 16-bit Thumb instruction (?)
       * FIXME check coverage of masks
       */
      return narrowInstr;
  }
}

#endif
