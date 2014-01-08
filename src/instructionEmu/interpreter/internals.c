#include "common/bit.h"
#include "common/stddef.h"

#include "cpuArch/constants.h"

#include "instructionEmu/decoder/arm/structs.h"
#include "instructionEmu/interpreter/common.h"
#include "instructionEmu/interpreter/internals.h"


enum guestBreakPointValues
{
  BKPT_TEST_PASS = 0,
  BKPT_DUMP_ACTIVE_SPT = 0xFFFF
};


static u32int *getHighGPRegisterPointer(GCONTXT *context, u32int registerIndex);

// rotate right function
static u32int rorVal(u32int value, u32int ramt)  __constant__;


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


// take the imm5 shift amount and shift type field from instr
// returns shift type, and adjusts shift amount
u32int decodeShiftImmediate(u32int instrShiftType, u32int imm5, u32int* shamt)
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
  bool result;
  // cond<3:1> switch
  switch ((conditionCode & 0xE) >> 1)
  {
    case 0:
      result = (context->CPSR.bits.Z == 1);
      break;
    case 1:
      result = (context->CPSR.bits.C == 1);
      break;
    case 2:
      result = (context->CPSR.bits.N == 1);
      break;
    case 3:
      result = (context->CPSR.bits.V == 1);
      break;
    case 4:
      result = (context->CPSR.bits.C == 1) && (context->CPSR.bits.Z == 0);
      break;
    case 5:
      result = (context->CPSR.bits.N == context->CPSR.bits.V);
      break;
    case 6:
      result = (context->CPSR.bits.N == context->CPSR.bits.V) && (context->CPSR.bits.Z == 0);
      break;
    // this case should not be hit: handled in previous ConditionPassed()
    case 7:
    default:
      result = TRUE;
  }
  // STARFIX: once this function isn't called directly apart form ConditionPassed()
  // remove the check for '1111'
  if (((conditionCode & 1) == 1) && (conditionCode != NV))
  {
    result = !result;
  }
  return result;
}

void invalidDataProcTrap(GCONTXT *context, u32int instruction, const char *message)
{
  printf("%#.8x @ %#.8x should not have trapped!" EOL, instruction, context->R15);
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
    return PC(context);
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


// generic any type shift function, changes input_parameter(carryFlag) value
u32int shiftVal(u32int value, u8int shiftType, u32int shamt)
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
