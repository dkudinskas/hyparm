#include "common/debug.h"

#include "vm/omap35xx/serial.h"

#include "instructionEmu/commonInstrFunctions.h"


extern GCONTXT * getGuestContext(void); //from main.c

/* a function to serve as a dead-loop if we decode something invalid */
void invalid_instruction(u32int instr, char * msg)
{
  serial_putstring("Invalid instruction detected! 0x");
  serial_putint(instr);
  serial_newline();
  serial_putstring(msg);
  serial_newline();
}

bool guestInPrivMode(GCONTXT * context)
{
  u32int modeField = context->CPSR & CPSR_MODE_FIELD;
  return (modeField == CPSR_MODE_USER) ? FALSE : TRUE;
}


bool evalCC(u32int instrCC, u32int cpsrCC)
{
  switch(instrCC)
  {
    case CC_EQ: // Z set
      if ((cpsrCC & CC_Z_FLAG) == CC_Z_FLAG)
      {
        return TRUE;
      }
      break;
    case CC_NE: // Z clear
      if ((cpsrCC & CC_Z_FLAG) == 0)
      {
        return TRUE;
      }
      break;
    case CC_HS: // C set
      if ((cpsrCC & CC_C_FLAG) == CC_C_FLAG)
      {
        return TRUE;
      }
      break;
    case CC_LO: // C clear
      if ((cpsrCC & CC_C_FLAG) == 0)
      {
        return TRUE;
      }
      break;
    case CC_MI: // N set
      if ((cpsrCC & CC_N_FLAG) == CC_N_FLAG)
      {
        return TRUE;
      }
      break;
    case CC_PL: // N clear
      if ((cpsrCC & CC_N_FLAG) == 0)
      {
        return TRUE;
      }
      break;
    case CC_VS: // V set
      if ((cpsrCC & CC_V_FLAG) == CC_V_FLAG)
      {
        return TRUE;
      }
      break;
    case CC_VC: // V clear
      if ((cpsrCC & CC_V_FLAG) == 0)
      {
        return TRUE;
      }
      break;
    case CC_HI: // C set and Z clear
      if ( ((cpsrCC & CC_C_FLAG) == CC_C_FLAG) && ((cpsrCC & CC_Z_FLAG) == 0) )
      {
        return TRUE;
      }
      break;
    case CC_LS: // C clear or Z set
      if ( ((cpsrCC & CC_C_FLAG) == 0) || ((cpsrCC & CC_Z_FLAG) == CC_Z_FLAG) )
      {
        return TRUE;
      }
      break;
    case CC_GE: //  N equals V
      if ( ((cpsrCC >> 3) & CC_V_FLAG) == (cpsrCC & CC_V_FLAG) )
      {
        return TRUE;
      }
      break;
    case CC_LT: // N is not equals to V
      if ( ((cpsrCC >> 3) & CC_V_FLAG) != (cpsrCC & CC_V_FLAG) )
      {
        return TRUE;
      }
      break;
    case CC_GT: // Z clear and N equals V
      if ( ((cpsrCC & CC_Z_FLAG) == 0) &&
         ( ((cpsrCC >> 3) & CC_V_FLAG) == (cpsrCC & CC_V_FLAG) ) )
      {
        return TRUE;
      }
      break;
    case CC_LE: // Z set or N is not equal to V
      if ( ((cpsrCC & CC_Z_FLAG) == CC_Z_FLAG) ||
         ( ((cpsrCC >> 3) & CC_V_FLAG) != (cpsrCC & CC_V_FLAG) ) )
      {
        return TRUE;
      }
      break;
    case CC_AL:
      return TRUE;
    case CC_NV:
      return FALSE;
  }
  // if we reached this, condition not met or cc was never!
  return FALSE;
}

/* function to store a register value, evaluates modes. */
void storeGuestGPR(u32int regDest, u32int value, GCONTXT * context)
{
  u32int guestMode = (context->CPSR) & CPSR_MODE_FIELD;

  if ( ((regDest >= 0) && (regDest < 8)) || (regDest == 15) )
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
      if (guestMode == CPSR_MODE_FIQ)
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
        case CPSR_MODE_USER:
        case CPSR_MODE_SYSTEM:
          strPtr = (regDest == 13) ? (&(context->R13_USR)) : (&(context->R14_USR));
          break;
        case CPSR_MODE_FIQ:
          strPtr = (regDest == 13) ? (&(context->R13_FIQ)) : (&(context->R14_FIQ));
          break;
        case CPSR_MODE_IRQ:
          strPtr = (regDest == 13) ? (&(context->R13_IRQ)) : (&(context->R14_IRQ));
          break;
        case CPSR_MODE_SVC:
          strPtr = (regDest == 13) ? (&(context->R13_SVC)) : (&(context->R14_SVC));
          break;
        case CPSR_MODE_ABORT:
          strPtr = (regDest == 13) ? (&(context->R13_ABT)) : (&(context->R14_ABT));
          break;
        case CPSR_MODE_UNDEF:
          strPtr = (regDest == 13) ? (&(context->R13_UND)) : (&(context->R14_UND));
          break;
        default:
          invalid_instruction(context->endOfBlockInstr, "storeGuestGPR: invalid CPSR mode!");
      } // switch ends
      *strPtr = value;
    } // R13/R14 ends
  } // mode specific else ends
}

/* function to load a register value, evaluates modes. */
u32int loadGuestGPR(u32int regSrc, GCONTXT * context)
{
  u32int guestMode = context->CPSR & CPSR_MODE_FIELD;
  u32int value = 0;

  if ( ((regSrc >= 0) && (regSrc < 8)) || (regSrc == 15) )
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
      if (guestMode == CPSR_MODE_FIQ)
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
        case CPSR_MODE_USER:
        case CPSR_MODE_SYSTEM:
          ldPtr = (regSrc == 13) ? (&(context->R13_USR)) : (&(context->R14_USR));
          break;
        case CPSR_MODE_FIQ:
          ldPtr = (regSrc == 13) ? (&(context->R13_FIQ)) : (&(context->R14_FIQ));
          break;
        case CPSR_MODE_IRQ:
          ldPtr = (regSrc == 13) ? (&(context->R13_IRQ)) : (&(context->R14_IRQ));
          break;
        case CPSR_MODE_SVC:
          ldPtr = (regSrc == 13) ? (&(context->R13_SVC)) : (&(context->R14_SVC));
          break;
        case CPSR_MODE_ABORT:
          ldPtr = (regSrc == 13) ? (&(context->R13_ABT)) : (&(context->R14_ABT));
          break;
        case CPSR_MODE_UNDEF:
          ldPtr = (regSrc == 13) ? (&(context->R13_UND)) : (&(context->R14_UND));
          break;
        default:
          invalid_instruction(context->endOfBlockInstr, "loadGuestGPR: invalid CPSR mode!");
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
      DIE_NOW(0,"voodoo dolls everywhere!");
  } // switch ends

  // compiler happy!
  return 0;
}

// count the number of ones in a 32 bit stream
u32int countBitsSet(u32int bitstream)
{
  u32int bitsSet = 0;
  int i = 0;
  for (i = 0; i < 32; i++)
  {
    if ((bitstream >> i) & 0x1)
    {
      bitsSet++;
    }
  }
  return bitsSet;
}
