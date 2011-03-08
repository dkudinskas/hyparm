#include "dataProcessInstr.h"
#include "commonInstrFunctions.h"
#include "debug.h"
void invalidDataProcTrap(char * msg, GCONTXT * gc)
{
  serial_putint(gc->endOfBlockInstr);
  serial_putstring(" @ ");
  serial_putint(gc->R15);
  serial_putstring(" should not have trapped!");
  serial_newline();
  DIE_NOW(gc, msg);
}
u32int arithLogicOp(GCONTXT * context, OPTYPE opType, char * instrString)
{
  u32int instr = context->endOfBlockInstr;
  u32int cpsrCC = (context->CPSR >> 28) & 0xF;
  u32int nextPC = context->R15;
  u32int regDest = (instr & 0x0000F000) >> 12;
  if (regDest != 0xF)
  {
    invalidDataProcTrap(instrString, context);
  }
#ifdef DATA_PROC_TRACE
  serial_putstring(instrString);
  serial_putint(instr);
  serial_putstring(" @ ");
  serial_putint(context->R15);
  serial_newline();
#endif
  
  int instrCC = (instr >> 28) & 0xF;
  bool conditionMet = evalCC(instrCC, cpsrCC);
  if (conditionMet)
  {
    // set-flags case is tricky! depends on guest mode.
    u32int setFlags = (instr & 0x00100000);
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
        if ( ((context->CPSR & CPSR_MODE_FIELD) == CPSR_MODE_USER) ||
             ((context->CPSR & CPSR_MODE_FIELD) == CPSR_MODE_SYSTEM) )
        {
          // there are no SPSR's in usr or sys modes!
          DIE_NOW(0, "arithLogicOp: exception return in guest usr/sys mode! bug.");
        }
        else
        {
          // copy SPSR to CPSR
          switch (context->CPSR & CPSR_MODE_FIELD)
          {
            case CPSR_MODE_FIQ:
              context->CPSR = context->SPSR_FIQ;
              break;
            case CPSR_MODE_IRQ:
              context->CPSR = context->SPSR_IRQ;
              break;
            case CPSR_MODE_SVC:
              context->CPSR = context->SPSR_SVC;
              break;
            case CPSR_MODE_ABORT:
              context->CPSR = context->SPSR_ABT;
              break;
            case CPSR_MODE_UNDEF:
              context->CPSR = context->SPSR_UND;
              break;
            case CPSR_MODE_USER:
            case CPSR_MODE_SYSTEM:
            default: 
              DIE_NOW(0, "arithLogicOp: invalid SPSR read for current guest mode.");
          } 
        }
      }
      else
      {
        dumpGuestContext(context);
        DIE_NOW(0, "unimplemented arithLogicOp set flags case");
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

u32int andPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr)
{
  DIE_NOW(0, "and PCFunct unfinished\n");
  return 0;
}

u32int andInstruction(GCONTXT * context)
{
  DIE_NOW(0, "andInstruction is executed but not yet checked for blockCopyCompatibility");
  DIE_NOW(context, "Unimplemented AND trap");
}
/*********************************/
/* RSB Rd, Rs, Rs2/imm, shiftAmt */
/*********************************/

u32int rsbPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr)
{
  DIE_NOW(0, "rsb PCFunct unfinished\n");
  return 0;
}

u32int rsbInstruction(GCONTXT * context)
{
  DIE_NOW(0, "rsbInstruction is executed but not yet checked for blockCopyCompatibility");
  DIE_NOW(context, "Unimplemented RSB trap");
}
/*********************************/
/* RSB Rd, Rs, Rs2/imm, shiftAmt */
/*********************************/

u32int rscPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr)
{
  DIE_NOW(0, "rsc PCFunct unfinished\n");
  return 0;
}

u32int rscInstruction(GCONTXT * context)
{
  DIE_NOW(0, "rscInstruction is executed but not yet checked for blockCopyCompatibility");
  DIE_NOW(context, "Unimplemented RSC trap");
}
/*********************************/
/* SUB Rd, Rs, Rs2/imm, shiftAmt */
/*********************************/

u32int subPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr)
{
  DIE_NOW(0, "sub PCFunct unfinished\n");
  return 0;
}

u32int subInstruction(GCONTXT * context)
{
  DIE_NOW(0, "subInstruction is executed but not yet checked for blockCopyCompatibility");
  DIE_NOW(context, "Unimplemented SUB trap");
}
/*********************************/
/* SBC Rd, Rs, Rs2/imm, shiftAmt */
/*********************************/

u32int sbcPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr)
{
  DIE_NOW(0, "sbc PCFunct unfinished\n");
  return 0;
}

u32int sbcInstruction(GCONTXT * context)
{
  DIE_NOW(0, "sbcInstruction is executed but not yet checked for blockCopyCompatibility");
  DIE_NOW(context, "Unimplemented SBC trap");
}
/*********************************/
/* ADD Rd, Rs, Rs2/imm, shiftAmt */
/*********************************/

u32int addPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr)
{
  //Check if target Register is PC if so HandleFunct should take care of possible PC problems

  DIE_NOW(0, "add PCFunct unfinished\n");
  return 0;
}

u32int addInstruction(GCONTXT * context)
{
  DIE_NOW(0, "addInstruction is executed but not yet checked for blockCopyCompatibility");
  OPTYPE opType = ADD;
  return arithLogicOp(context, opType, "ADD instr ");
}
/*********************************/
/* ADD Rd, Rs, Rs2/imm, shiftAmt */
/*********************************/

u32int adcPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr)
{
  DIE_NOW(0, "adc PCFunct unfinished\n");
  return 0;
}

u32int adcInstruction(GCONTXT * context)
{
  DIE_NOW(0, "adcInstruction is executed but not yet checked for blockCopyCompatibility");
  DIE_NOW(context, "Unimplemented ADC trap");
}
/*********************************/
/* ORR Rd, Rs, Rs2/imm, shiftAmt */
/*********************************/

u32int orrPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr)
{
  DIE_NOW(0, "orr PCFunct unfinished\n");
  return 0;
}

u32int orrInstruction(GCONTXT * context)
{
  DIE_NOW(0, "orrInstruction is executed but not yet checked for blockCopyCompatibility");
  DIE_NOW(context, "Unimplemented ORR trap");
}
/*********************************/
/* ORR Rd, Rs, Rs2/imm, shiftAmt */
/*********************************/

u32int eorPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr)
{
  DIE_NOW(0, "eor PCFunct unfinished\n");
  return 0;
}

u32int eorInstruction(GCONTXT * context)
{
  DIE_NOW(0, "eorInstruction is executed but not yet checked for blockCopyCompatibility");
  DIE_NOW(context, "Unimplemented EOR trap");
}
/*********************************/
/* SBC Rd, Rs, Rs2/imm, shiftAmt */
/*********************************/

u32int bicPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr)
{
  DIE_NOW(0, "bic PCFunct unfinished\n");
  return 0;
}

u32int bicInstruction(GCONTXT * context)
{
  DIE_NOW(0, "bicInstruction is executed but not yet checked for blockCopyCompatibility");
  DIE_NOW(context, "Unimplemented BIC trap");
}
/*********************************/
/* MOV Rd, Rs                    */
/*********************************/

u32int movPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr)
{
  DIE_NOW(0, "mov PCFunct unfinished\n");
  return 0;
}

u32int movInstruction(GCONTXT * context)
{
  DIE_NOW(0, "movInstruction is executed but not yet checked for blockCopyCompatibility");
  OPTYPE opType = MOV;
  return arithLogicOp(context, opType, "MOV instr ");
}
/*********************************/
/* MVN Rd, Rs                    */
/*********************************/

u32int mvnPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr)
{
  DIE_NOW(0, "mvn PCFunct unfinished\n");
  return 0;
}

u32int mvnInstruction(GCONTXT * context)
{
  DIE_NOW(0, "mvnInstruction is executed but not yet checked for blockCopyCompatibility");
  DIE_NOW(context, "Unimplemented MVN trap");
}
/*********************************/
/* LSL Rd, Rs                    */
/*********************************/

u32int lslPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr)
{
  DIE_NOW(0, "lsl PCFunct unfinished\n");
  return 0;
}

u32int lslInstruction(GCONTXT * context)
{
  DIE_NOW(0, "lslInstruction is executed but not yet checked for blockCopyCompatibility");
  DIE_NOW(context, "Unimplemented LSL trap");
}
/*********************************/
/* LSR Rd, Rs                    */
/*********************************/

u32int lsrPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr)
{
  DIE_NOW(0, "lsr PCFunct unfinished\n");
  return 0;
}

u32int lsrInstruction(GCONTXT * context)
{
  DIE_NOW(0, "lsrInstruction is executed but not yet checked for blockCopyCompatibility");
  DIE_NOW(context, "Unimplemented LSR trap");
}
/*********************************/
/* ASR Rd, Rs                    */
/*********************************/

u32int asrPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr)
{
  DIE_NOW(0, "asr PCFunct unfinished\n");
  return 0;
}

u32int asrInstruction(GCONTXT * context)
{
  DIE_NOW(0, "asrInstruction is executed but not yet checked for blockCopyCompatibility");
  DIE_NOW(context, "Unimplemented ASR trap");
}
/*********************************/
/* RRX Rd, Rs                    */
/*********************************/

u32int rrxPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr)
{
  DIE_NOW(0, "rrx PCFunct unfinished\n");
  return 0;
}

u32int rrxInstruction(GCONTXT * context)
{
  DIE_NOW(0, "rrxInstruction is executed but not yet checked for blockCopyCompatibility");
  DIE_NOW(context, "Unimplemented RRX trap");
}
/*********************************/
/* ROR Rd, Rs                    */
/*********************************/

u32int rorPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr)
{
  DIE_NOW(0, "ror PCFunct unfinished\n");
  return 0;
}

u32int rorInstruction(GCONTXT * context)
{
  DIE_NOW(0, "rorInstruction is executed but not yet checked for blockCopyCompatibility");
  DIE_NOW(context, "Unimplemented ROR trap");
}
/*********************************/
/* TST Rs, Rs2/imm               */
/*********************************/

u32int tstPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr)
{
  DIE_NOW(0, "tst PCFunct unfinished\n");
  return 0;
}

u32int tstInstruction(GCONTXT * context)
{
  DIE_NOW(0, "tstInstruction is executed but not yet checked for blockCopyCompatibility");
  DIE_NOW(context, "Unimplemented tst interpreter");
}
/*********************************/
/* TEQ Rs, Rs2/imm               */
/*********************************/

u32int teqPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr)
{
  DIE_NOW(0, "teq PCFunct unfinished\n");
  return 0;
}

u32int teqInstruction(GCONTXT * context)
{
  DIE_NOW(0, "teqInstruction is executed but not yet checked for blockCopyCompatibility");
  invalidDataProcTrap("TEQ instr", context);
  return 0;
}
/*********************************/
/* CMP Rs, Rs2/imm               */
/*********************************/

u32int cmpPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr)
{
  DIE_NOW(0, "cmp PCFunct unfinished\n");
  return 0;
}

u32int cmpInstruction(GCONTXT * context)
{
  DIE_NOW(0, "cmpInstruction is executed but not yet checked for blockCopyCompatibility");
  invalidDataProcTrap("CMP instr", context);
  return 0;
}
/*********************************/
/* CMN Rs, Rs2/imm               */
/*********************************/

u32int cmnPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr)
{
  DIE_NOW(0, "cmn PCFunct unfinished\n");
  return 0;
}

u32int cmnInstruction(GCONTXT * context)
{
  DIE_NOW(0, "cmnInstruction is executed but not yet checked for blockCopyCompatibility");
  invalidDataProcTrap("CMN instr", context);
  return 0;
}
