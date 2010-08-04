#include "dataProcessInstr.h"
#include "commonInstrFunctions.h"

void invalidDataProcTrap(char * msg, GCONTXT * gc)
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

u32int arithLogicOp(GCONTXT * context, OPTYPE opType, char * instrString)
{
  u32int instr = context->endOfBlockInstr;
  int instrCC = (instr >> 28) & 0xF;
  u32int nextPC = context->R15;
  u32int regDest = (instr & 0x0000F000) >> 12;

  if (regDest != 0xF)
  {
    invalidDataProcTrap(instrString, context);
  }
  // set-flags case is tricky! depends on guest mode.
  u32int setFlags = (instr & 0x00100000);
  if (setFlags)
  {
    error_function("unimplemented arithLogicOp set flags case", context);
  }

#ifdef DATA_PROC_TRACE
  serial_putstring(instrString);
  serial_putint(instr);
  serial_putstring(" @ ");
  serial_putint(context->R15);
  serial_newline();
#endif
  
  u32int cpsrCC = (context->CPSR >> 28) & 0xF;
  bool conditionMet = evalCC(instrCC, cpsrCC);
  if (conditionMet)
  {
    // source operand1
    u32int regSrc = (instr & 0x000F0000) >> 16;
    // source operand2 - register or immediate?    
    u32int regOrImm = instr & 0x02000000;
    if (regOrImm != 0)
    {
      // source operand2 immediate: pc = regSrc + ror(immediate)
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
        default:
          error_function("invalid arithLogicOp opType", context);
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
            nextPC = loadGuestGPR(regSrc, context) +
               shiftVal(loadGuestGPR(regSrc2, context), shiftType, shamt, &carryFlag);
            if (regSrc == 0xF)
            {
              nextPC += 8;
            }
            break;
          case MOV:
            // cant be shifted - mov shifted reg is a pseudo instr
            if (shamt != 0)
            {
              error_function("MOV PC, Rn cant be shifted - that is a pseudo instr", context);
            }
            nextPC = loadGuestGPR(regSrc2, context);
            break;
          default:
            error_function("invalid arithLogicOp opType", context);
        }
      }
      else
      {
        // If shift amount is in register and any of the register
        // operands are PC then instruction unpredictable
        error_function("unpredictable instruction <dataProc> PC, Rn, Rm, Rs", context);
      }
    }
    context->R15 = nextPC;
    return nextPC;
  }
  else
  {
    nextPC = context->R15 + 4;
    return nextPC;
  }
}



/*********************************/
/* AND Rd, Rs, Rs2/imm, shiftAmt */
/*********************************/
u32int andInstruction(GCONTXT * context)
{
  error_function("Unimplemented AND trap", context);
  return 0;
}


/*********************************/
/* RSB Rd, Rs, Rs2/imm, shiftAmt */
/*********************************/
u32int rsbInstruction(GCONTXT * context)
{
  error_function("Unimplemented RSB trap", context);
  return 0;
}


/*********************************/
/* RSB Rd, Rs, Rs2/imm, shiftAmt */
/*********************************/
u32int rscInstruction(GCONTXT * context)
{
  error_function("Unimplemented RSC trap", context);
  return 0;
}


/*********************************/
/* SUB Rd, Rs, Rs2/imm, shiftAmt */
/*********************************/
u32int subInstruction(GCONTXT * context)
{
  error_function("Unimplemented SUB trap", context);
  return 0;
}


/*********************************/
/* SBC Rd, Rs, Rs2/imm, shiftAmt */
/*********************************/
u32int sbcInstruction(GCONTXT * context)
{
  error_function("Unimplemented SBC trap", context);
  return 0;
}


/*********************************/
/* ADD Rd, Rs, Rs2/imm, shiftAmt */
/*********************************/
u32int addInstruction(GCONTXT * context)
{
  OPTYPE opType = ADD;
  return arithLogicOp(context, opType, "ADD instr ");
}


/*********************************/
/* ADD Rd, Rs, Rs2/imm, shiftAmt */
/*********************************/
u32int adcInstruction(GCONTXT * context)
{
  error_function("Unimplemented ADC trap", context);
  return 0;
}


/*********************************/
/* ORR Rd, Rs, Rs2/imm, shiftAmt */
/*********************************/
u32int orrInstruction(GCONTXT * context)
{
  error_function("Unimplemented ORR trap", context);
  return 0;
}


/*********************************/
/* ORR Rd, Rs, Rs2/imm, shiftAmt */
/*********************************/
u32int eorInstruction(GCONTXT * context)
{
  error_function("Unimplemented EOR trap", context);
  return 0;
}


/*********************************/
/* SBC Rd, Rs, Rs2/imm, shiftAmt */
/*********************************/
u32int bicInstruction(GCONTXT * context)
{
  error_function("Unimplemented BIC trap", context);
  return 0;
}


/*********************************/
/* MOV Rd, Rs                    */
/*********************************/
u32int movInstruction(GCONTXT * context)
{
  OPTYPE opType = MOV;
  return arithLogicOp(context, opType, "MOV instr ");
}


/*********************************/
/* MVN Rd, Rs                    */
/*********************************/
u32int mvnInstruction(GCONTXT * context)
{
  error_function("Unimplemented MVN trap", context);
  return 0;
}


/*********************************/
/* LSL Rd, Rs                    */
/*********************************/
u32int lslInstruction(GCONTXT * context)
{
  error_function("Unimplemented LSL trap", context);
  return 0;
}


/*********************************/
/* LSR Rd, Rs                    */
/*********************************/
u32int lsrInstruction(GCONTXT * context)
{
  error_function("Unimplemented LSR trap", context);
  return 0;
}


/*********************************/
/* ASR Rd, Rs                    */
/*********************************/
u32int asrInstruction(GCONTXT * context)
{
  error_function("Unimplemented ASR trap", context);
  return 0;
}


/*********************************/
/* RRX Rd, Rs                    */
/*********************************/
u32int rrxInstruction(GCONTXT * context)
{
  error_function("Unimplemented RRX trap", context);
  return 0;
}


/*********************************/
/* ROR Rd, Rs                    */
/*********************************/
u32int rorInstruction(GCONTXT * context)
{
  error_function("Unimplemented ROR trap", context);
  return 0;
}


/*********************************/
/* TST Rs, Rs2/imm               */
/*********************************/
u32int tstInstruction(GCONTXT * context)
{
  error_function("Unimplemented tst interpreter", context);
  return 0;
}


/*********************************/
/* TEQ Rs, Rs2/imm               */
/*********************************/
u32int teqInstruction(GCONTXT * context)
{
  invalidDataProcTrap("TEQ instr", context);
  return 0;
}


/*********************************/
/* CMP Rs, Rs2/imm               */
/*********************************/
u32int cmpInstruction(GCONTXT * context)
{
  invalidDataProcTrap("CMP instr", context);
  return 0;
}


/*********************************/
/* CMN Rs, Rs2/imm               */
/*********************************/
u32int cmnInstruction(GCONTXT * context)
{
  invalidDataProcTrap("CMN instr", context);
  return 0;
}
