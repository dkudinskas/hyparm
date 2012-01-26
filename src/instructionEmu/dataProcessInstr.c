#include "common/debug.h"

#include "instructionEmu/commonInstrFunctions.h"
#include "instructionEmu/dataProcessInstr.h"


void invalidDataProcTrap(const char * msg, GCONTXT * gc)
{
  printf("%08x @ %08x should not have trapped!\n", gc->endOfBlockInstr, gc->R15);
  DIE_NOW(gc, msg);
}

u32int arithLogicOp(GCONTXT * context, OPTYPE opType, const char * instrString)
{//It shouldn't matter that the destination register is PC if all PC-reads are intercepted
 //Than a store to the PC should store a valid value
  u32int instr = context->endOfBlockInstr;
  u32int cpsrCC = (context->CPSR >> 28) & 0xF;
#ifdef CONFIG_BLOCK_COPY
  u32int nextPC = context->PCOfLastInstruction;
#else
  u32int nextPC = context->R15;
#endif
  u32int regDest = (instr & 0x0000F000) >> 12;
  if (regDest != 0xF)//Destination register is not PC -> instruction should have been handled by PCFunct
  {
    invalidDataProcTrap(instrString, context);
  }
#ifdef DATA_PROC_TRACE
  printf(instrString);
#ifdef CONFIG_BLOCK_COPY
  printf(" %08x @ %08x\n", instr, context->PCOfLastInstruction);
#else
  printf(" %08x @ %08x\n", instr, context->R15);
#endif
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
        DIE_NOW(context, "unimplemented arithLogicOp set flags case");
      }
    }
    context->R15 = nextPC;
    return nextPC;
  }
  else
  {
	#ifdef CONFIG_BLOCK_COPY
    nextPC = context->PCOfLastInstruction + 4;
    #else
    nextPC = context->R15 + 4;
    #endif
    return nextPC;
  }
}
/*********************************/
/* AND Rd, Rs, Rs2/imm, shiftAmt */
/*********************************/
#ifdef CONFIG_BLOCK_COPY
u32int* andPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  return standardImmRegRSR(context, instructionAddr, currBlockCopyCacheAddr, blockCopyCacheStartAddress);
}
#endif

u32int andInstruction(GCONTXT * context)
{
  DIE_NOW(context, "Unimplemented AND trap");
}
/*********************************/
/* RSB Rd, Rs, Rs2/imm, shiftAmt */
/*********************************/

#ifdef CONFIG_BLOCK_COPY
u32int* rsbPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  return standardImmRegRSR(context, instructionAddr, currBlockCopyCacheAddr, blockCopyCacheStartAddress);
}
#endif

u32int rsbInstruction(GCONTXT * context)
{
  DIE_NOW(0, "rsbInstruction is executed but not yet checked for blockCopyCompatibility");
  DIE_NOW(context, "Unimplemented RSB trap");
}
/*********************************/
/* RSB Rd, Rs, Rs2/imm, shiftAmt */
/*********************************/

#ifdef CONFIG_BLOCK_COPY
u32int* rscPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  return standardImmRegRSR(context, instructionAddr, currBlockCopyCacheAddr, blockCopyCacheStartAddress);
}
#endif

u32int rscInstruction(GCONTXT * context)
{
  DIE_NOW(0, "rscInstruction is executed but not yet checked for blockCopyCompatibility");
  DIE_NOW(context, "Unimplemented RSC trap");
}
/*********************************/
/* SUB Rd, Rs, Rs2/imm, shiftAmt */
/*********************************/

#ifdef CONFIG_BLOCK_COPY
u32int* subPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  return standardImmRegRSR(context, instructionAddr, currBlockCopyCacheAddr, blockCopyCacheStartAddress);
}
#endif

u32int subInstruction(GCONTXT * context)
{
  DIE_NOW(0, "subInstruction is executed but not yet checked for blockCopyCompatibility");
  DIE_NOW(context, "Unimplemented SUB trap");
}
/*********************************/
/* SBC Rd, Rs, Rs2/imm, shiftAmt */
/*********************************/

#ifdef CONFIG_BLOCK_COPY
u32int* sbcPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  return standardImmRegRSR(context, instructionAddr, currBlockCopyCacheAddr, blockCopyCacheStartAddress);
}
#endif

u32int sbcInstruction(GCONTXT * context)
{
  DIE_NOW(0, "sbcInstruction is executed but not yet checked for blockCopyCompatibility");
  DIE_NOW(context, "Unimplemented SBC trap");
}
/*********************************/
/* ADD Rd, Rs, Rs2/imm, shiftAmt */
/*********************************/

#ifdef CONFIG_BLOCK_COPY
u32int* addPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  return standardImmRegRSR(context, instructionAddr, currBlockCopyCacheAddr, blockCopyCacheStartAddress);
}
#endif

u32int addInstruction(GCONTXT * context)
{
  OPTYPE opType = ADD;
  return arithLogicOp(context, opType, "ADD instr ");
}
/*********************************/
/* ADD Rd, Rs, Rs2/imm, shiftAmt */
/*********************************/

#ifdef CONFIG_BLOCK_COPY
u32int* adcPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  return standardImmRegRSR(context, instructionAddr, currBlockCopyCacheAddr, blockCopyCacheStartAddress);
}
#endif

u32int adcInstruction(GCONTXT * context)
{
  DIE_NOW(0, "adcInstruction is executed but not yet checked for blockCopyCompatibility");
  DIE_NOW(context, "Unimplemented ADC trap");
}
/*********************************/
/* ORR Rd, Rs, Rs2/imm, shiftAmt */
/*********************************/

#ifdef CONFIG_BLOCK_COPY
u32int* orrPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  return standardImmRegRSR(context, instructionAddr, currBlockCopyCacheAddr, blockCopyCacheStartAddress);
}
#endif

u32int orrInstruction(GCONTXT * context)
{
  DIE_NOW(0, "orrInstruction is executed but not yet checked for blockCopyCompatibility");
  DIE_NOW(context, "Unimplemented ORR trap");
}
/*********************************/
/* ORR Rd, Rs, Rs2/imm, shiftAmt */
/*********************************/

#ifdef CONFIG_BLOCK_COPY
u32int* eorPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  return standardImmRegRSR(context, instructionAddr, currBlockCopyCacheAddr, blockCopyCacheStartAddress);
}
#endif

u32int eorInstruction(GCONTXT * context)
{
  DIE_NOW(0, "eorInstruction is executed but not yet checked for blockCopyCompatibility");
  DIE_NOW(context, "Unimplemented EOR trap");
}
/*********************************/
/* SBC Rd, Rs, Rs2/imm, shiftAmt */
/*********************************/

#ifdef CONFIG_BLOCK_COPY
u32int* bicPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  return standardImmRegRSR(context, instructionAddr, currBlockCopyCacheAddr, blockCopyCacheStartAddress);
}
#endif

u32int bicInstruction(GCONTXT * context)
{
  DIE_NOW(0, "bicInstruction is executed but not yet checked for blockCopyCompatibility");
  DIE_NOW(context, "Unimplemented BIC trap");
}
/*********************************/
/* MOV Rd, Rs                    */
/*********************************/

#ifdef CONFIG_BLOCK_COPY
u32int* movPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{ //Destination is surely not PC
  u32int instruction = *instructionAddr;
  u32int destReg=(instruction>>12) & 0xF;
  u32int instr2Copy=instruction;
  bool conditionAlways = (instruction>>28 & 0xF) == 0xE;

  if(conditionAlways)
  {
    if( ( (instruction>>25) & 0b1 ) != 1)
      {//bit 25 != 1 -> there can be registers, PC can possibly be read
        if((instruction & 0xF) != 0xF)
        {
          DIE_NOW(0, "mov PCFunct: movPCFunct can only be called if last 4 bits are 1111\n");
        }else{
          //step 1 Copy PC (=instructionAddr2) to desReg
          currBlockCopyCacheAddr=savePCInReg(context, instructionAddr, currBlockCopyCacheAddr,  destReg);
          //Step 2 modify ldrInstruction
          //Clear PC source Register
          instr2Copy=zeroBits(instruction, 0);//set last 4 bits equal to zero
          instr2Copy=instr2Copy | (destReg);  //set last 4 bits so correct register is used
        }
      }

      currBlockCopyCacheAddr=checkAndClearBlockCopyCacheAddress(currBlockCopyCacheAddr,context->blockCache,(u32int*)context->blockCopyCache,(u32int*)context->blockCopyCacheEnd);
      *(currBlockCopyCacheAddr++)=instr2Copy;

      return currBlockCopyCacheAddr;
  }
  else
  {
    //condition != always

    if( ( (instruction>>25) & 0b1 ) == 0b0)
    {//bit 25 != 1 -> there can be registers, PC can possibly be read
      if((instruction & 0xF) != 0xF)
      {
        DIE_NOW(0, "mov PCFunct: movPCFunct can only be called if last 4 bits are 1111\n");
      }else{
        //Make instruction safe and return
        /* conditional instruction thus sometimes not executed */
        /*Instruction has to be changed to a PC safe instructionstream withouth using destReg. */
        u32int srcReg = instruction & 0xF;
        u32int srcPCRegLoc = 0;
        u32int scratchReg = findUnusedRegister(srcReg, destReg, -1);
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
    //if function hasn't returned at this point -> instruction is safe
    currBlockCopyCacheAddr=checkAndClearBlockCopyCacheAddress(currBlockCopyCacheAddr,context->blockCache,(u32int*)context->blockCopyCache,(u32int*)context->blockCopyCacheEnd);
    *(currBlockCopyCacheAddr++)=instruction;

    return currBlockCopyCacheAddr;

  }


}
#endif

u32int movInstruction(GCONTXT * context)
{
  //arithLogicOp should support movInstructions
  OPTYPE opType = MOV;
  return arithLogicOp(context, opType, "MOV instr ");
}
/*********************************/
/* MVN Rd, Rs                    */
/*********************************/

#ifdef CONFIG_BLOCK_COPY
u32int* mvnPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  u32int instruction = *instructionAddr;
  u32int destReg=(instruction>>12) & 0xF;
  u32int instr2Copy = instruction;
  bool immediate = (instruction >> 25 & 0b1) == 0b1;
  bool replaceReg1 = FALSE;
  bool conditionAlways = (instruction>>28 & 0xF) == 0xE;

  if(immediate)
  {
    //Always safe do nothing replaceReg1 is already false
  }
  else
  {
    if(conditionAlways)
    {
      bool registerShifted = ((instruction>>4 & 0b1)==0b1) && ((instruction>>4 & 0b1)==0b0);
      //Here we know it is register or register-shifted register
      if(registerShifted)
      {
        DIE_NOW(0,"MVNPC (register-shifted register) -> UNPREDICTABLE");
      }
      else
      {
        //eor (register)
        if((instruction & 0xF) == 0xF)
        {
          replaceReg1 = TRUE;
        }
      }
      if(replaceReg1){
        //step 1 Copy PC (=instructionAddr2) to desReg
        currBlockCopyCacheAddr=savePCInReg(context, instructionAddr, currBlockCopyCacheAddr,  destReg);
        if(replaceReg1)
        {
          //Step 2 modify eorInstruction
          //Clear PC source Register
          instr2Copy=zeroBits(instruction, 0);
          instr2Copy=instr2Copy | (destReg);
        }
      }
    }
    else
    {
      /* mvn with condition code != ALWAYS*/
      DIE_NOW(0,"conditional mvn PCFunct not yet implemented");
    }
  }

  currBlockCopyCacheAddr=checkAndClearBlockCopyCacheAddress(currBlockCopyCacheAddr,context->blockCache,(u32int*)context->blockCopyCache,(u32int*)context->blockCopyCacheEnd);
  *(currBlockCopyCacheAddr++)=instr2Copy;

  return currBlockCopyCacheAddr;
}
#endif

u32int mvnInstruction(GCONTXT * context)
{
  DIE_NOW(0, "mvnInstruction is executed but not yet checked for blockCopyCompatibility");
  DIE_NOW(context, "Unimplemented MVN trap");
}
/*********************************/
/* LSL Rd, Rs                    */
/*********************************/

#ifdef CONFIG_BLOCK_COPY
u32int* lslPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{  //This is the same as lsrPCInstruction only direction has changed -> only bit 5 differs
  return lsrPCInstruction(context, instructionAddr, currBlockCopyCacheAddr, blockCopyCacheStartAddress);
}
#endif

u32int lslInstruction(GCONTXT * context)
{
  DIE_NOW(0, "lslInstruction is executed but not yet checked for blockCopyCompatibility");
  DIE_NOW(context, "Unimplemented LSL trap");
}
/*********************************/
/* LSR Rd, Rs                    */
/*********************************/

#ifdef CONFIG_BLOCK_COPY
u32int* lsrPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{

  u32int instruction=*instructionAddr;
  u32int srcPCRegLoc = 0;//This is where the PC is in the instruction
  u32int destReg=(instruction>>12) & 0xF;
  u32int instr2Copy=instruction;
  bool conditionAlways = (instruction>>28 & 0xF) == 0xE;
  //bits 16-19 are zero so Rm or Rn is PC

  if(conditionAlways)
  {
    if((instruction>>4 & 0b1) == 1)
    {//Bit 4 is 1 if extra register (LSR(register))
      DIE_NOW(0,"lsrPCInstruction LSR(register) with a srcReg==PC is UNPREDICTABLE?");
    }
    //Ready to do shift
    //save PC
    if((instruction &  0xF) == 0xF){
      currBlockCopyCacheAddr=savePCInReg(context, instructionAddr, currBlockCopyCacheAddr,  destReg);

      //Step 2 modify ldrInstruction
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
    /*lsrPC Funct conditional*/
    DIE_NOW(0,"lsrPCFunct conditional is not yet implemented");

  }
}
#endif

u32int lsrInstruction(GCONTXT * context)
{
  DIE_NOW(0, "lsrInstruction is executed but not yet checked for blockCopyCompatibility");
  DIE_NOW(context, "Unimplemented LSR trap");
}
/*********************************/
/* ASR Rd, Rs                    */
/*********************************/

#ifdef CONFIG_BLOCK_COPY
u32int* asrPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  u32int instruction = *instructionAddr;
  bool immediate = ( (instruction>>4 & 0x7) == 0x4 );
  u32int destReg = (instruction>>12) & 0xF;
  u32int instr2Copy = instruction;
  bool conditionAlways = (instruction>>28 & 0xF) == 0xE;

  if(conditionAlways)
  {
    if(immediate)
    {
      if((instruction & 0xF)== 0xF)
      { //inputRegister = PC
        //step 1 Copy PC (=instructionAddr2) to desReg
        currBlockCopyCacheAddr=savePCInReg(context, instructionAddr, currBlockCopyCacheAddr,  destReg);

        //Step 2 modify ldrInstruction
        //Clear PC source Register
        instr2Copy=zeroBits(instruction, 0);
        instr2Copy=instr2Copy | (destReg);
      }
    }
    else
    {
      //ARM p 352
      DIE_NOW(0,"asrPCInstruction: ASR(register) cannot take PC as input!->UNPREDICTABLE");
    }

    currBlockCopyCacheAddr=checkAndClearBlockCopyCacheAddress(currBlockCopyCacheAddr,context->blockCache,(u32int*)context->blockCopyCache,(u32int*)context->blockCopyCacheEnd);
    *(currBlockCopyCacheAddr++)=instr2Copy;

    return currBlockCopyCacheAddr;
  }
  else
  {
    DIE_NOW(0,"asrPCInstruction conditional");
  }
}
#endif

u32int asrInstruction(GCONTXT * context)
{
  DIE_NOW(0, "asrInstruction is executed but not yet checked for blockCopyCompatibility");
  DIE_NOW(context, "Unimplemented ASR trap");
}
/*********************************/
/* RRX Rd, Rs                    */
/*********************************/

#ifdef CONFIG_BLOCK_COPY
u32int* rrxPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  DIE_NOW(0, "rrx PCFunct unfinished\n");
  return 0;
}
#endif

u32int rrxInstruction(GCONTXT * context)
{
  DIE_NOW(0, "rrxInstruction is executed but not yet checked for blockCopyCompatibility");
  DIE_NOW(context, "Unimplemented RRX trap");
}
/*********************************/
/* ROR Rd, Rs                    */
/*********************************/

#ifdef CONFIG_BLOCK_COPY
u32int* rorPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  DIE_NOW(0, "ror PCFunct unfinished\n");
  return 0;
}
#endif

u32int rorInstruction(GCONTXT * context)
{
  DIE_NOW(0, "rorInstruction is executed but not yet checked for blockCopyCompatibility");
  DIE_NOW(context, "Unimplemented ROR trap");
}
/*********************************/
/* TST Rs, Rs2/imm               */
/*********************************/

#ifdef CONFIG_BLOCK_COPY
u32int* tstPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  return standardImmRegRSRNoDest(context, instructionAddr, currBlockCopyCacheAddr, blockCopyCacheStartAddress);
}
#endif

u32int tstInstruction(GCONTXT * context)
{
  DIE_NOW(0, "tstInstruction is executed but not yet checked for blockCopyCompatibility");
  DIE_NOW(context, "Unimplemented tst interpreter");
}
/*********************************/
/* TEQ Rs, Rs2/imm               */
/*********************************/

#ifdef CONFIG_BLOCK_COPY
u32int* teqPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  return standardImmRegRSRNoDest(context, instructionAddr, currBlockCopyCacheAddr, blockCopyCacheStartAddress);
}
#endif

u32int teqInstruction(GCONTXT * context)
{
  DIE_NOW(0, "teqInstruction is executed but not yet checked for blockCopyCompatibility");
  invalidDataProcTrap("TEQ instr", context);
  return 0;
}
/*********************************/
/* CMP Rs, Rs2/imm               */
/*********************************/

#ifdef CONFIG_BLOCK_COPY
u32int* cmpPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  return standardImmRegRSRNoDest(context, instructionAddr, currBlockCopyCacheAddr, blockCopyCacheStartAddress);
}
#endif

u32int cmpInstruction(GCONTXT * context)
{
  DIE_NOW(0, "cmpInstruction is executed but not yet checked for blockCopyCompatibility");
  invalidDataProcTrap("CMP instr", context);
  return 0;
}
/*********************************/
/* CMN Rs, Rs2/imm               */
/*********************************/
#ifdef CONFIG_BLOCK_COPY
u32int* cmnPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  return standardImmRegRSRNoDest(context, instructionAddr, currBlockCopyCacheAddr, blockCopyCacheStartAddress);
}
#endif

u32int cmnInstruction(GCONTXT * context)
{
  DIE_NOW(0, "cmnInstruction is executed but not yet checked for blockCopyCompatibility");
  invalidDataProcTrap("CMN instr", context);
  return 0;
}
