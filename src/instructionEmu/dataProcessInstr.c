#include "dataProcessInstr.h"
#include "commonInstrFunctions.h"
#include "debug.h"
#include "blockCache.h" //necessary to check and clear blockCopyCache
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
{//It shouldn't matter that the destination register is PC if all PC-reads are intercepted
 //Than a store to the PC should store a valid value
  u32int instr = context->endOfBlockInstr;
  u32int cpsrCC = (context->CPSR >> 28) & 0xF;
  u32int nextPC = context->PCOfLastInstruction;
  u32int regDest = (instr & 0x0000F000) >> 12;
  if (regDest != 0xF)//Destination register is not PC -> instruction should have been handled by PCFunct
  {
    invalidDataProcTrap(instrString, context);
  }
#ifdef DATA_PROC_TRACE
  serial_putstring(instrString);
  serial_putint(instr);
  serial_putstring(" @ ");
  serial_putint(context->PCOfLastInstruction);
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
        dumpGuestContext(context);
        DIE_NOW(0, "unimplemented arithLogicOp set flags case");
      }
    }
    context->R15 = nextPC;
    return nextPC;
  }
  else
  {
    nextPC = context->PCOfLastInstruction + 4;
    return nextPC;
  }
}
/*********************************/
/* AND Rd, Rs, Rs2/imm, shiftAmt */
/*********************************/

u32int* andPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  u32int instruction = *instructionAddr;
  u32int destReg = (instruction>>12) & 0xF;
  u32int instr2Copy = instruction;
  //bit 25 set to 1 is immediate
  bool immediate = ((instruction>>25) & 0b1)==0b1;

  if(immediate)
  {
    if((instruction>>16 & 0xF) != 0xF)
    {//Instruction is safe!
      currBlockCopyCacheAddr=checkAndClearBlockCopyCacheAddress(currBlockCopyCacheAddr,context->blockCache,(u32int*)context->blockCopyCache,(u32int*)context->blockCopyCacheEnd);
      *(currBlockCopyCacheAddr++)=instruction;

      return currBlockCopyCacheAddr;
    }

    //Store PC in destReg
    currBlockCopyCacheAddr=savePCInReg(context, instructionAddr, currBlockCopyCacheAddr,  destReg);

    //Let instruction use destReg as inputRegister
    instr2Copy=zeroBits(instruction, 16);
    instr2Copy=instr2Copy | (destReg<<16);

    currBlockCopyCacheAddr=checkAndClearBlockCopyCacheAddress(currBlockCopyCacheAddr,context->blockCache,(u32int*)context->blockCopyCache,(u32int*)context->blockCopyCacheEnd);
    *(currBlockCopyCacheAddr++)=instr2Copy;

    return currBlockCopyCacheAddr;

  }
  else //something with registers (register or register-shifted register
  {
    //register-shifted register => bit 4 == 1 and bit 7 == 0
    bool shifted=((instruction>>4 & 0b1) == 0b1)&&((instruction>>7 & 0b1) == 0b0);
    if(shifted)
    {
      //ARM ARM p.350
      DIE_NOW(0,"andPCInstruction: register-shifted register cannot have PC as input!");
    }
    else
    {
      DIE_NOW(0,"andPCInstruction: register not implemented yet.");
      //THIS must make use of reserved word!
    }
  }
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

u32int* rsbPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
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

u32int* rscPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
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

u32int* subPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  //target register is not PC
  u32int instruction=*instructionAddr;
  u32int instr2=(instruction>>24) & 0x0F;
  u32int instrbit4=(instruction>>4) & 0x1;
  u32int instrbit7=(instruction>>7) & 0x1;
  u32int destReg=(instruction>>12) & 0xF;
  u32int instr2Copy=0;

  if(instr2 == 0x02){//Sub instruction is SUB immediate
    //Save PC in destReg
    currBlockCopyCacheAddr=savePCInReg(context, instructionAddr, currBlockCopyCacheAddr,  destReg);

    //Sub instruction will only be changed very little -> Rn is the only thing that has to be changed
    instr2Copy=instruction & 0xFFF0FFFF; //set Rn to Rd
    instr2Copy=instr2Copy | (destReg<<16);

    currBlockCopyCacheAddr=checkAndClearBlockCopyCacheAddress(currBlockCopyCacheAddr,context->blockCache,(u32int*)context->blockCopyCache,(u32int*)context->blockCopyCacheEnd);
    *(currBlockCopyCacheAddr++)=instr2Copy;

    return currBlockCopyCacheAddr;//3 instructions are added!
  }else{//Sub instruction is SUB with registers
    if(instrbit4 == 1 && instrbit7==0){//SUB register-shifted register
      DIE_NOW(0, "ADD (register-shifted register) is not implemented yet");
    }else{//Sub register
      DIE_NOW(0, "Add (register) is not implemented yet");
    }
  }

  DIE_NOW(0, "bottom subPCInstruction reached without return\n");
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

u32int* sbcPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
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

u32int* addPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  //target register is not PC
  u32int instruction=*instructionAddr;
  u32int instr2=(instruction>>24) & 0x0F;
  u32int instrbit4=(instruction>>4) & 0x1;
  u32int instrbit7=(instruction>>7) & 0x1;
  u32int destReg=(instruction>>12) & 0xF;
  u32int instr2Copy=0;

  if(instr2 == 0x02){//Add instruction is ADD immediate
    //The real PC and the blockCopyCachePC are to far appart to solve it using a 12 bit immediate BUT
    //We can only get here when Rn==PC && Rd!=PC
    //=> We can first set Rd to value of PC and then perform Add{s}<c> <Rd>,<Rd>,<#const>
    //save PC
    currBlockCopyCacheAddr=savePCInReg(context, instructionAddr, currBlockCopyCacheAddr,  destReg);

    //Add instruction will only be changed very little -> Rn is the only thing that has to be changed
    instr2Copy=instruction & 0xFFF0FFFF; //set Rn to Rd
    instr2Copy=instr2Copy | (destReg<<16);

    currBlockCopyCacheAddr=checkAndClearBlockCopyCacheAddress(currBlockCopyCacheAddr,context->blockCache,(u32int*)context->blockCopyCache,(u32int*)context->blockCopyCacheEnd);
    *(currBlockCopyCacheAddr++)=instr2Copy;

    return currBlockCopyCacheAddr;

  }else{//Add instruction is ADD with registers
    if(instrbit4 == 1 && instrbit7==0){//ADD register-shifted register
      DIE_NOW(0, "ADD (register-shifted register) is not implemented yet");
    }else{//Add register
      DIE_NOW(0, "Add (register) is not implemented yet");
    }
  }


  DIE_NOW(0, "add PCFunct unfinished\n");
  return 0;
}

u32int addInstruction(GCONTXT * context)
{
  OPTYPE opType = ADD;
  return arithLogicOp(context, opType, "ADD instr ");
}
/*********************************/
/* ADD Rd, Rs, Rs2/imm, shiftAmt */
/*********************************/

u32int* adcPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
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

u32int* orrPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
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

u32int* eorPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
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

u32int* bicPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
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

u32int* movPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{ //Destination is surely not PC
  u32int instruction = *instructionAddr;
  u32int destReg=(instruction>>12) & 0xF;
  u32int instr2Copy=0;
  if( ( (instruction>>25) & 0b1 ) == 1)
  {//bit 25 == 1 -> immediate
    //PC cannot be read -> instruction is safe
    currBlockCopyCacheAddr=checkAndClearBlockCopyCacheAddress(currBlockCopyCacheAddr,context->blockCache,(u32int*)context->blockCopyCache,(u32int*)context->blockCopyCacheEnd);
    *(currBlockCopyCacheAddr++)=instruction;

    return currBlockCopyCacheAddr;
  }
  else
  {//bit 25 == 0 -> register
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

      currBlockCopyCacheAddr=checkAndClearBlockCopyCacheAddress(currBlockCopyCacheAddr,context->blockCache,(u32int*)context->blockCopyCache,(u32int*)context->blockCopyCacheEnd);
      *(currBlockCopyCacheAddr++)=instr2Copy;

      return currBlockCopyCacheAddr;

    }

  }

  DIE_NOW(0, "mov PCFunct unfinished\n");
  return 0;
}

u32int movInstruction(GCONTXT * context)
{
  //arithLogicOp should support movInstructions
  OPTYPE opType = MOV;
  return arithLogicOp(context, opType, "MOV instr ");
}
/*********************************/
/* MVN Rd, Rs                    */
/*********************************/

u32int* mvnPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
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

u32int* lslPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
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

u32int* lsrPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  u32int instruction=*instructionAddr;
  u32int srcPCRegLoc = 0;//This is where the PC is in the instruction
  u32int destReg=(instruction>>12) & 0xF;
  u32int instr2Copy=0;
  //bits 16-19 are zero so Rm or Rn is PC

  if((instruction>>4 & 0b1) == 1)
  {//Bit 4 is 1 if extra register (LSR(register))
    DIE_NOW(0,"lsrPCInstruction LSR(register) with a srcReg==PC is UNPREDICTABLE?");
    //ToDO: Normally this cannot be executed->remove when confirmed
    if(((instruction>>8 & 0xF) == 0xF) && ((instruction & 0xF) == 0xF))
    {
      DIE_NOW(0, "LSR with both srcRegisters equal to PC is not implemented yet");
    }
    if((instruction>>8 & 0xF) == 0xF)
    {
      srcPCRegLoc=8;
    }
  }
  //Ready to do shift
  //save PC
  currBlockCopyCacheAddr=savePCInReg(context, instructionAddr, currBlockCopyCacheAddr,  destReg);

  //Step 2 modify ldrInstruction
  //Clear PC source Register
  instr2Copy=zeroBits(instruction, srcPCRegLoc);
  instr2Copy=instr2Copy | (destReg<<srcPCRegLoc);

  currBlockCopyCacheAddr=checkAndClearBlockCopyCacheAddress(currBlockCopyCacheAddr,context->blockCache,(u32int*)context->blockCopyCache,(u32int*)context->blockCopyCacheEnd);
  *(currBlockCopyCacheAddr++)=instr2Copy;

  return currBlockCopyCacheAddr;
}

u32int lsrInstruction(GCONTXT * context)
{
  DIE_NOW(0, "lsrInstruction is executed but not yet checked for blockCopyCompatibility");
  DIE_NOW(context, "Unimplemented LSR trap");
}
/*********************************/
/* ASR Rd, Rs                    */
/*********************************/

u32int* asrPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  u32int instruction = *instructionAddr;
  bool immediate = ( (instruction>>4 & 0x7) == 0x4 );
  u32int destReg = (instruction>>12) & 0xF;
  u32int instr2Copy = 0;
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
    else
    {//Safe instruction-> just copy it
      instr2Copy = instruction;
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

u32int asrInstruction(GCONTXT * context)
{
  DIE_NOW(0, "asrInstruction is executed but not yet checked for blockCopyCompatibility");
  DIE_NOW(context, "Unimplemented ASR trap");
}
/*********************************/
/* RRX Rd, Rs                    */
/*********************************/

u32int* rrxPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
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

u32int* rorPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
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

u32int* tstPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
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

u32int* teqPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
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

u32int* cmpPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  //ARM ARM p.392 and further:
  //CMP Rs <> PC otherwise unpredictable
  //If immediate only check Rn (bits 16-19 => srcReg1) else also check Rm (bits 0-3 => srcReg2)
  u32int instruction = *instructionAddr;
  u32int srcReg1 = (instruction >> 16) & 0xF;
  u32int srcReg2 = (instruction) & 0xF;
  bool immediate = ((instruction >> 24) & 0xF) == 0x3;
  //srcReg1 must always be checked
  bool replaceReg1 = (srcReg1 == 0xF);
  bool replaceReg2 = 0;//default false
  u32int scratchRegister = 0;//This only needs to set if register must be replaced
  u32int instr2Copy = 0;
  if(!immediate) //It is not an immediate instruction and thus srcReg2 must also be checked
  {
    replaceReg2=(srcReg2 == 0xF);
  }
  if(replaceReg1 || replaceReg2)
  { //there is a register that must be replaced -> backup scratchregister & move PC to scratchRegister
    //backup scratchregister = str
    DIE_NOW(0,"cmpPCInstruction: This isn't validated yet");
    scratchRegister = findUnusedRegister(srcReg1,srcReg2,-1);//Only 2 registers -> use negative value for 3th register
    currBlockCopyCacheAddr=backupRegister(scratchRegister, currBlockCopyCacheAddr, blockCopyCacheStartAddress);
    //MovePC to scratchRegister
    currBlockCopyCacheAddr=savePCInReg(context, instructionAddr, currBlockCopyCacheAddr, scratchRegister);
  }
  //Modify instruction & copy
  instr2Copy=instruction;
  if(replaceReg1)
  {// srcReg1 Must be replaced
    instr2Copy=instr2Copy & 0xFFF0FFFF;
    instr2Copy=instr2Copy | (scratchRegister<<16);
  }
  if(replaceReg2)
  {// srcReg2 Must be replaced
    instr2Copy=instr2Copy & 0xFFFFFFF0;
    instr2Copy=instr2Copy | (scratchRegister);
  }
  currBlockCopyCacheAddr=checkAndClearBlockCopyCacheAddress(currBlockCopyCacheAddr,context->blockCache,(u32int*)context->blockCopyCache,(u32int*)context->blockCopyCacheEnd);
  *(currBlockCopyCacheAddr++)=instr2Copy;

  //Restore scratchRegister if a register was replaced
  if(replaceReg1 || replaceReg2)
  {
    currBlockCopyCacheAddr=restoreRegister(scratchRegister, currBlockCopyCacheAddr, blockCopyCacheStartAddress);
  }

  return currBlockCopyCacheAddr;
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

u32int* cmnPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
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
