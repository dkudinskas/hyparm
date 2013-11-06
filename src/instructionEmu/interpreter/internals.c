#include "common/bit.h"
#include "common/stddef.h"

#include "cpuArch/constants.h"

#include "instructionEmu/interpreter/internals.h"


enum guestBreakPointValues
{
  BKPT_TEST_PASS = 0,
  BKPT_DUMP_ACTIVE_SPT = 0xFFFF
};


// take shift type field from instr, return shift type
static u32int decodeShift(u32int instrShiftType);

static u32int *getHighGPRegisterPointer(GCONTXT *context, u32int registerIndex);

// rotate right function
static u32int rorVal(u32int value, u32int ramt)  __constant__;


u32int arithLogicOp(GCONTXT *context, u32int instr, OPTYPE opType, const char *instrString)
{
  u32int nextPC = 0;
  u32int regDest = (instr & 0x0000F000) >> 12;

  if (regDest != GPR_PC)
  {
    invalidDataProcTrap(context, instr, instrString);
  }

#ifdef DATA_PROC_TRACE
  printf("%s: %#.8x @ %#.8x" EOL, instrString, instr, context->lastGuestPC);
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
      // if S bit is set, this is return from exception!
      if (setFlags != 0)
      {
        DIE_NOW(context, "arithmetic: imm case return from exception case unimplemented.\n");
      }

      // source operand2 immediate: pc = regSrc +/- ror(immediate)
      u32int imm12 = instr & 0x00000FFF;
      switch (opType)
      {
        case ADD:
        {
          nextPC = getGPRegister(context, regSrc) + armExpandImm12(imm12);
          break;
        }
        case SUB:
        {
          nextPC = getGPRegister(context, regSrc) - armExpandImm12(imm12);
          break;
        }
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
      ASSERT((instr & 0x00000010) == 0, ERROR_UNPREDICTABLE_INSTRUCTION)
      // shift amount is an immediate field
      u32int imm5 = (instr & 0xF80) >> 7;
      shiftType = decodeShiftImmediate(shiftType, imm5, &shamt);
      switch (opType)
      {
        case ADD:
        {
          // if S bit is set, this is return from exception!
          if (setFlags != 0)
          {
            DIE_NOW(context, "arithmetic: reg case return from exception case unimplemented.\n");
          }
          nextPC = getGPRegister(context, regSrc) + shiftVal(getGPRegister(context, regSrc2), shiftType, shamt, context->CPSR.bits.C);
          break;
        }
        case SUB:
        {
          // if S bit is set, this is return from exception!
          if (setFlags != 0)
          {
            DIE_NOW(context, "arithmetic: reg case return from exception case unimplemented.\n");
          }
          nextPC = getGPRegister(context, regSrc) - shiftVal(getGPRegister(context, regSrc2), shiftType, shamt, context->CPSR.bits.C);
          break;
        }
        case MOV:
        {
          // cant be shifted - mov shifted reg is a pseudo instr
          ASSERT(shamt == 0, ERROR_BAD_ARGUMENTS);
          nextPC = getGPRegister(context, regSrc2);
          break;
        }
        default:
          DIE_NOW(context, "invalid arithLogicOp opType");
      }
    }

    if (setFlags)
    {
      if (regDest == GPR_PC)
      {
        u32int spsrToCopy = 0;
        // copy SPSR to CPSR
        switch (context->CPSR.bits.mode)
        {
          case FIQ_MODE:
          {
            spsrToCopy = context->SPSR_FIQ.value;
            break;
          }
          case IRQ_MODE:
          {
            spsrToCopy = context->SPSR_IRQ.value;
            break;
          }
          case SVC_MODE:
          {
            spsrToCopy = context->SPSR_SVC.value;
            break;
          }
          case ABT_MODE:
          {
            spsrToCopy = context->SPSR_ABT.value;
            break;
          }
          case UND_MODE:
          {
            spsrToCopy = context->SPSR_UND.value;
            break;
          }
          default:
            DIE_NOW(context, "arithLogicOp: no SPSR for current guest mode");
        }
        if ((context->CPSR.bits.mode) != (spsrToCopy & PSR_MODE))
        {
          guestChangeMode(context, spsrToCopy & PSR_MODE);
        }
        context->CPSR.value = spsrToCopy;

        // Align PC
        if (context->CPSR.bits.T)
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
        // unimplemented setflags case
        DIE_NOW(context, ERROR_NOT_IMPLEMENTED);
      }
    }

    ASSERT((nextPC & 1) == 0, "Interworking branch not allowed");
    return nextPC;
  }
  else
  {
    return context->lastGuestPC + ARM_INSTRUCTION_SIZE;
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
static u32int decodeShift(u32int instrShiftType)
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
    default:
      return SHIFT_TYPE_ROR;
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

/*
 * This function is used in unit tests. It evaluates the value passed to the BKPT instruction.
 * Current values:
 * 0      pass
 * 0xFFFF print active shadow pagetable
 * other  fail
 */
void evaluateBreakpointValue(GCONTXT *context, u32int value)
{
  switch (value)
  {
    case BKPT_TEST_PASS:
    {
      DIE_NOW(context, "test passed");
    }
    case BKPT_DUMP_ACTIVE_SPT:
    {
      dumpTranslationTable(context->pageTables->shadowActive);
      break;
    }
    default:
    {
      printf("Breakpoint value %#.8x" EOL, value);
      DIE_NOW(context, "test failed");
    }
  }
  return;
}

bool evaluateConditionCode(GCONTXT *context, u32int conditionCode)
{
  switch (conditionCode)
  {
    case EQ:
      return context->CPSR.bits.Z;
    case NE:
      return !context->CPSR.bits.Z;
    case HS:
      return context->CPSR.bits.C;
    case LO:
      return !context->CPSR.bits.C;
    case MI:
      return context->CPSR.bits.N;
    case PL:
      return !context->CPSR.bits.N;
    case VS:
      return context->CPSR.bits.V;
    case VC:
      return !context->CPSR.bits.V;
    case HI:
      return context->CPSR.bits.C && !(context->CPSR.bits.Z);
    case LS:
      return !context->CPSR.bits.C || context->CPSR.bits.Z;
    case GE:
      return context->CPSR.bits.N == context->CPSR.bits.V;
    case LT:
      return context->CPSR.bits.N != context->CPSR.bits.V;
    case GT:
      return !context->CPSR.bits.Z && (context->CPSR.bits.N == context->CPSR.bits.V);
    case LE:
      return context->CPSR.bits.Z || context->CPSR.bits.N != context->CPSR.bits.V;
    case AL:
      return TRUE;
    default:
      return FALSE;
  }
}

void invalidDataProcTrap(GCONTXT *context, u32int instruction, const char *message)
{
  printf("%#.8x @ %#.8x should not have trapped!" EOL, instruction, context->lastGuestPC);
  DIE_NOW(context, message);
}

u32int getGPRegister(GCONTXT *context, u32int sourceRegister)
{
  if (sourceRegister < 8)
  {
    return getLowGPRegister(context, sourceRegister);
  }

  if (sourceRegister == GPR_PC)
  {
    return getNativeProgramCounter(context);
  }

  return *(getHighGPRegisterPointer(context, sourceRegister));
}

static u32int *getHighGPRegisterPointer(GCONTXT *context, u32int registerIndex)
{
  CPSRmode guestMode = context->CPSR.bits.mode;
  if (registerIndex <= 12)
  {
    return ((guestMode == FIQ_MODE ? &context->R8_FIQ : &context->R8) + registerIndex - 8);
  }

  switch (guestMode)
  {
    case USR_MODE:
    case SYS_MODE:
      return registerIndex == 13 ? &context->R13_USR : &context->R14_USR;
    case FIQ_MODE:
      return registerIndex == 13 ? &context->R13_FIQ : &context->R14_FIQ;
    case IRQ_MODE:
      return registerIndex == 13 ? &context->R13_IRQ : &context->R14_IRQ;
    case SVC_MODE:
      return registerIndex == 13 ? &context->R13_SVC : &context->R14_SVC;
    case ABT_MODE:
      return registerIndex == 13 ? &context->R13_ABT : &context->R14_ABT;
    case UND_MODE:
      return registerIndex == 13 ? &context->R13_UND : &context->R14_UND;
    default:
      DIE_NOW(context, ERROR_NOT_IMPLEMENTED);
  } // switch ends
}

void setGPRegister(GCONTXT *context, u32int destinationRegister, u32int value)
{
  if (destinationRegister < 8 || destinationRegister == GPR_PC)
  {
    setLowGPRegister(context, destinationRegister, value);
  }
  else
  {
    *(getHighGPRegisterPointer(context, destinationRegister)) = value;
  }
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
u32int shiftVal(u32int value, u8int shiftType, u32int shamt, u8int carryFlag)
{
  // RRX can only shift right by 1
  ASSERT((shiftType != SHIFT_TYPE_RRX) || (shamt == 1), "type rrx, but shamt not 1!");

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
        DIE_NOW(NULL, ERROR_NOT_IMPLEMENTED);
     } // switch
  } // else
  return retVal;
}
