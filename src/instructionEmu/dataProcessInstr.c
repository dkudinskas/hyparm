#include "common/debug.h"

#include "cpuArch/constants.h"

#include "instructionEmu/commonInstrFunctions.h"
#include "instructionEmu/dataProcessInstr.h"


void invalidDataProcTrap(const char * msg, GCONTXT * gc)
{
  printf("%.8x @ %.8x should not have trapped!" EOL, gc->endOfBlockInstr, gc->R15);
  DIE_NOW(gc, msg);
}

u32int arithLogicOp(GCONTXT * context, OPTYPE opType, const char * instrString)
{
  u32int instr = context->endOfBlockInstr;
  u32int nextPC = context->R15;
  u32int regDest = (instr & 0x0000F000) >> 12;

  if (regDest != 0xF)
  {
    invalidDataProcTrap(instrString, context);
  }

#ifdef DATA_PROC_TRACE
  printf("%s %.8x @ %.8x" EOL, instrString, instr, context->R15);
#endif

  int instrCC = (instr >> 28) & 0xF;
  if (evaluateConditionCode(context, instrCC))
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
            if (regSrc == 0xF)
            {
              nextPC += 8;
            }
            break;
          case SUB:
            nextPC = loadGuestGPR(regSrc, context) - shiftVal(loadGuestGPR(regSrc2, context), shiftType, shamt, &carryFlag);
            if (regSrc == 0xF)
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
      if (regDest == 0xF)
      {
        if ( ((context->CPSR & PSR_MODE) == PSR_USR_MODE) ||
             ((context->CPSR & PSR_MODE) == PSR_SYS_MODE) )
        {
          // there are no SPSR's in usr or sys modes!
          DIE_NOW(0, "arithLogicOp: exception return in guest usr/sys mode! bug.");
        }
        else
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
              DIE_NOW(0, "arithLogicOp: invalid SPSR read for current guest mode.");
          } 
        }
      }
      else
      {
        DIE_NOW(context, "unimplemented arithLogicOp set flags case");
      }
    }

    context->R15 = nextPC;
#ifdef CONFIG_THUMB2
    /*
     * FIXME: Niels: WHY ??
     * Did you mean interworking bit ?
     */
    // clear thumb bit if needed
    nextPC &= ~0x1;
#endif
    return nextPC;
  }
  else
  {
#ifdef CONFIG_THUMB2
    if (context->CPSR & PSR_T_BIT)
    {
      nextPC = context->R15 +2;
    }
    else
#endif
    {
      nextPC = context->R15 + 4;
    }
    return nextPC;
  }
}


/*********************************/
/* AND Rd, Rs, Rs2/imm, shiftAmt */
/*********************************/
u32int andInstruction(GCONTXT * context)
{
  printf("%.8x" EOL, context->endOfBlockInstr);
  DIE_NOW(context, "Unimplemented AND trap");
}


/*********************************/
/* RSB Rd, Rs, Rs2/imm, shiftAmt */
/*********************************/
u32int rsbInstruction(GCONTXT * context)
{
  DIE_NOW(context, "Unimplemented RSB trap");
}


/*********************************/
/* RSB Rd, Rs, Rs2/imm, shiftAmt */
/*********************************/
u32int rscInstruction(GCONTXT * context)
{
  DIE_NOW(context, "Unimplemented RSC trap");
}


/*********************************/
/* SUB Rd, Rs, Rs2/imm, shiftAmt */
/*********************************/
u32int subInstruction(GCONTXT * context)
{
  OPTYPE opType = SUB;
  return arithLogicOp(context, opType, "SUB instr ");
}


/*********************************/
/* SBC Rd, Rs, Rs2/imm, shiftAmt */
/*********************************/
u32int sbcInstruction(GCONTXT * context)
{
  DIE_NOW(context, "Unimplemented SBC trap");
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
  DIE_NOW(context, "Unimplemented ADC trap");
}


/*********************************/
/* ORR Rd, Rs, Rs2/imm, shiftAmt */
/*********************************/
u32int orrInstruction(GCONTXT * context)
{
  DIE_NOW(context, "Unimplemented ORR trap");
}


/*********************************/
/* ORR Rd, Rs, Rs2/imm, shiftAmt */
/*********************************/
u32int eorInstruction(GCONTXT * context)
{
  DIE_NOW(context, "Unimplemented EOR trap");
}


/*********************************/
/* SBC Rd, Rs, Rs2/imm, shiftAmt */
/*********************************/
u32int bicInstruction(GCONTXT * context)
{
  DIE_NOW(context, "Unimplemented BIC trap");
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
  DIE_NOW(context, "Unimplemented MVN trap");
}


/*********************************/
/* LSL Rd, Rs                    */
/*********************************/
u32int lslInstruction(GCONTXT * context)
{
  DIE_NOW(context, "Unimplemented LSL trap");
}


/*********************************/
/* LSR Rd, Rs                    */
/*********************************/
u32int lsrInstruction(GCONTXT * context)
{
  DIE_NOW(context, "Unimplemented LSR trap");
}


/*********************************/
/* ASR Rd, Rs                    */
/*********************************/
u32int asrInstruction(GCONTXT * context)
{
  DIE_NOW(context, "Unimplemented ASR trap");
}


/*********************************/
/* RRX Rd, Rs                    */
/*********************************/
u32int rrxInstruction(GCONTXT * context)
{
  DIE_NOW(context, "Unimplemented RRX trap");
}


/*********************************/
/* ROR Rd, Rs                    */
/*********************************/
u32int rorInstruction(GCONTXT * context)
{
  DIE_NOW(context, "Unimplemented ROR trap");
}


/*********************************/
/* TST Rs, Rs2/imm               */
/*********************************/
u32int tstInstruction(GCONTXT * context)
{
  DIE_NOW(context, "Unimplemented tst interpreter");
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
