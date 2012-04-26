#include "common/stddef.h"

#include "cpuArch/constants.h"

#include "instructionEmu/interpreter/blockCopy.h"
#include "instructionEmu/interpreter/internals.h"


/* This will save the PC corresponding to instructionAddress in reg. instructionAddress is the original address of the instruction */
u32int* savePCInReg(TranslationCache *tc, u32int * instructionAddress, u32int * currBlockCopyCacheAddr, u32int reg)
{
  //First we will calculate the PCValue (PC is 2 behind)
  u32int instructionAddr2 = (u32int) (instructionAddress + 2);
  //MOVW -> ARM ARM A8.6.96 p506
  //|    |    |    |    |    |11         0|
  //|COND|0011|0000|imm4| Rd |    imm12   |
  //|1110|0011|0000|imm4| Rd |    imm12  0|
  //   e    3    0    ?    ?   ?   ?   ?
  u32int instr2Copy = 0xe3000000;
  instr2Copy = instr2Copy | ((((u32int) instructionAddr2) >> 12 & 0xF) << 16); //set imm4 correct
  instr2Copy = instr2Copy | (reg << 12);
  instr2Copy = instr2Copy | ((u32int) instructionAddr2 & 0xFFF); //set imm12 correct

  currBlockCopyCacheAddr = updateCodeCachePointer(tc, currBlockCopyCacheAddr);
  *(currBlockCopyCacheAddr++) = instr2Copy;

  //MOVT -> ARM ARM A8.6.99 p512
  //|    |    |    |    |    |11         0|
  //|COND|0011|0100|imm4| Rd |    imm12   |
  //|1110|0011|0000|imm4| Rd |    imm12  0|
  //   e    3    4    ?    ?   ?   ?   ?
  instr2Copy = 0xe3400000;
  instr2Copy = instr2Copy | ((((u32int) instructionAddr2) >> 28 & 0xF) << 16); //set 4 top bits correct
  instr2Copy = instr2Copy | (reg << 12);
  instr2Copy = instr2Copy | (((u32int) instructionAddr2 >> 16) & 0xFFF); //set imm12 correct

  currBlockCopyCacheAddr = updateCodeCachePointer(tc, currBlockCopyCacheAddr);
  *(currBlockCopyCacheAddr++) = instr2Copy;
  //Return the current BlockCopyCacheAddr so that the PCfunct that called this function knows where to continue.
  return currBlockCopyCacheAddr;
}

/*
 * standardImmRegRSR is a function that is used for instructions that have 3 flavors:
 *     -(immediate)       <=> bit 25 == 1
 *     -(register-shifted register) <=> bit25 == 0 and bit7 == 0 and bit4 == 1
 *     -(register)        <=> bit 25 == 0 and not  register-shifted register (bit4 == 0)
 *
 * It will make the instruction PC safe if and only if
 *     For the register-shifted register flavor Rn,Rm,Rd and Rs cannot be PC -> UNPREDICTABLE
 *     The immediate flavor has only 1 source register (bits 16-19)
 *     Register flavor has 2 source registers (bits 16-19) and (bits 0-3)
 *     List of instructions: ADD, ADC, AND, BIC, EOR, MVN, ORR RSB, RSC, SBC, SUB, TEQ, TST
 *     MVN is a special case because one of the source register is set to 0000 -> only a small
 *     inconvenience when destReg = R0 but it can be taken care of.
 */
u32int* standardImmRegRSR(TranslationCache *tc, u32int *instructionAddr, u32int *currBlockCopyCacheAddr, u32int *blockCopyCacheStartAddress)
{
  //target register is not PC
  u32int instruction = *instructionAddr;
  bool immediate = (instruction >> 25 & 0b1) == 0b1;
  u32int srcReg1 = instruction >> 16 & 0xF;
  u32int srcReg2 = instruction & 0xF;
  u32int destReg = instruction >> 12 & 0xF;
  bool replaceReg1 = FALSE;
  bool replaceReg2 = FALSE;
  u32int instr2Copy = instruction;
  if (ARM_EXTRACT_CONDITION_CODE(instruction) == CC_AL)
  {
    replaceReg1 = srcReg1 == GPR_PC;
    replaceReg2 = !immediate && srcReg2 == GPR_PC;
    if (replaceReg1 || replaceReg2)
    {
      /* Rd should not be used as input.  srcReg2 is part of the immediate value if it is a immediate flavor */
      if (srcReg1 == destReg || (!immediate && srcReg2 == destReg))
      {
        /* Some information before crashing that can be useful to determine if this is indeed the way to go.*/
        printf("instruction = %08x\n", instruction);
        printf("immediate = %08x\n", immediate);
        printf("srcReg1 = %08x\n", srcReg1);
        printf("srcReg2 = %08x\n", srcReg2);
        printf("destReg = %08x\n", destReg);
        DIE_NOW(NULL, "standardImmRegRSR special care should be taken it is not possible to use destReg");
        /*
         *  Not sure if this can occur.  If it occurs our usual trick won't work.
         *  It can still be solved using a scratch register.
         *  An example of the scratch register trick can be seen in tstPCInstruction
         *  So just place backup-scratch-register-instruction, put-PC-in-scratchregister-instruction, place modified instruction
         *  place restore-scratch-register-instruction
         *  THIS SHOULD BE DONE Inside this if-statement and this if-clause should return
         */
      }

      currBlockCopyCacheAddr = savePCInReg(tc, instructionAddr, currBlockCopyCacheAddr, destReg);

      if (replaceReg1)
      {
        /* Add instruction will only be changed very little -> Rn is the only thing that has to be changed */
        instr2Copy = instr2Copy & 0xFFF0FFFF; //set Rn to Rd
        instr2Copy = instr2Copy | (destReg << 16);
      }
      if (replaceReg2)
      {
        instr2Copy = instr2Copy & 0xFFFFFFF0; //set Rm to Rd
        instr2Copy = instr2Copy | (destReg);
      }
    }
    currBlockCopyCacheAddr = updateCodeCachePointer(tc, currBlockCopyCacheAddr);
    *(currBlockCopyCacheAddr++) = instr2Copy;

    return currBlockCopyCacheAddr;
  }
  else
  {
    /*If instruction is not always executed it is unsafe to use the destination register to safe
     * the program counter
     * We can use standardImmRegRSRNoDest for this.
     */
    return standardImmRegRSRNoDest(tc, instructionAddr, currBlockCopyCacheAddr, blockCopyCacheStartAddress);
  }
}

/*
 * Similar to standardImmRegRSR but there is no destination register which makes it
 * impossible to use the same trick.  A scratch register is needed.
 * List of instructions: CMN, CMP, TEQ, TST
 */
u32int* standardImmRegRSRNoDest(TranslationCache *tc, u32int * instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  u32int instruction = *instructionAddr;
  bool immediate = ((instruction >> 25) & 0b1) == 0b1;
  bool registerShifted = (((instruction >> 4) & 0b1) == 0b1) && (((instruction >> 7) & 0b1) == 0b0);
  u32int regSrc1 = (instruction >> 16) & 0xF;
  u32int regSrc2 = (instruction) & 0xF;
  bool rnIsPC = (regSrc1) == 0xF; /* see ARM ARM p.768 */
  bool rmIsPC = (regSrc2) == 0xF;
  u32int instr2Copy = instruction;
  u32int scratchReg = 0;

  if (immediate)
  {
    rmIsPC = 0;/* Immediate doesn't use second register -> always false */
  }
  else
  {
    if (registerShifted) /* register-shifted register */
    {
      DIE_NOW(NULL, "tstPCFunct: tst(register-shifted register) cannot have PC as source -> UNPREDICTABLE ");
    }
    /* if ordinary register flavor than rnIsPC & rmIsPC are already set correctly */
  }

  if (rnIsPC || rmIsPC)
  { /*Instruction has to be changed to a PC safe instructionstream. */
    printf("instruction = %08x\n", instruction);
    DIE_NOW(NULL, "standardImmRegRSRNoDest: this part is not tested.");
    /* No destination register so only source registers have to be checked*/
    scratchReg = getOtherRegisterOf2(regSrc1, regSrc2);
    /* place 'Backup scratchReg' instruction */
    currBlockCopyCacheAddr = backupRegister(tc, scratchReg, currBlockCopyCacheAddr, blockCopyCacheStartAddress);
    currBlockCopyCacheAddr = savePCInReg(tc, instructionAddr, currBlockCopyCacheAddr, scratchReg);

    if (rnIsPC)
    {
      instr2Copy = (instr2Copy & ~(0xF << 16)) | scratchReg << 16;
    }
    if (rmIsPC)
    {
      instr2Copy = (instr2Copy & ~0xF) | scratchReg;
    }
  }

  currBlockCopyCacheAddr = updateCodeCachePointer(tc, currBlockCopyCacheAddr);
  *(currBlockCopyCacheAddr++) = instr2Copy;

  if (rnIsPC || rmIsPC)
  {
    /* place 'restore scratchReg' instruction */
    currBlockCopyCacheAddr = restoreRegister(tc, scratchReg, currBlockCopyCacheAddr, blockCopyCacheStartAddress);
    /* Make sure scanner sees that we need a word to store the register*/
    currBlockCopyCacheAddr = (u32int*) (((u32int) currBlockCopyCacheAddr) | 0b1);
  }

  return currBlockCopyCacheAddr;
}

u32int * backupRegister(TranslationCache *tc, u32int reg2Backup, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  u32int targetAddr = (((u32int) blockCopyCacheStartAddress) & 0xFFFFFFFE) + 4;
  u32int offset = 0;
  //STR(immediate) -> ARM ARM A8.6.194 p696
  //|    |    |    |    |    |11         0|
  //|COND|010P|U0W0| Rn | Rt |    imm12   |
  //|1110|0101|0000|1111|????|????????????|
  //Rn=baseregister=PC,Rt=sourceRegister,P=1 (otherwise imm12 is ignored),U==1 add imm12 <-> U==0 subtract imm12,W=writeback=0
  u32int instr2Copy = 0xe50F0000;
  //set scratchRegister
  instr2Copy = instr2Copy | reg2Backup << 12;
  /*Now Check if there is already a free word in blockCopyCache if no then instructions should be placed after a free word (this way
   * less instructions need to be copied afterwards.
   */
  if ((((u32int) blockCopyCacheStartAddress) & 0b1) == 0b0)
  {
    /* No free word make one */
    /* First check if the current word is free (and free it up if not)*/
    currBlockCopyCacheAddr = updateCodeCachePointer(tc, currBlockCopyCacheAddr);
    /*Then go to next word*/
    currBlockCopyCacheAddr++;
    /* As there needs to be taken maximum 1 backup per instruction no further actions need to be taken.  Scanner will change blockCopyCacheStartAddress if necessary
     * Be sure to ignore last bit when using blockCopyCacheStartAddress.*/
  }

  //set imm12 -> No way that the offset will be bigger than a 12 bit value, PC is 2 behind -> +8
  offset = (u32int) currBlockCopyCacheAddr + 8 - targetAddr;
  if (offset > 0xFFF)
  {
    /* It is possible that the offset will be something like 0xFFFF????.  This is when the block is split.  The offset will be rewritten
     * when block is merged but if this offset is added we also overwrite the first bits of the instruction leading to the creation of
     * another instruction.  We can just reset the offset to 0 */
    offset = 0;
  }
  instr2Copy = instr2Copy | (offset);
  currBlockCopyCacheAddr = updateCodeCachePointer(tc, currBlockCopyCacheAddr);
  *(currBlockCopyCacheAddr++) = instr2Copy;
  return currBlockCopyCacheAddr;
}

u32int *restoreRegister(TranslationCache *tc, u32int reg2Restore, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  u32int targetAddr = (((u32int) blockCopyCacheStartAddress) & 0xFFFFFFFE) + 4;
  u32int offset = 0;

  //ldr(immediate) -> ARM ARM A8.6.58 p432
  //|    |    |    |    |    |11         0|
  //|COND|010P|U0W1| Rn | Rt |    imm12   |
  //|1110|0101|0001|1111|????|????????????|
  //Rn=baseregister=PC,Rt=sourceRegister,P=1 (otherwise imm12 is ignored),U==1 add imm12 <-> U==0 subtract imm12,W=writeback=0
  u32int instr2Copy = 0xe51F0000;
  //set scratchRegister
  instr2Copy = instr2Copy | reg2Restore << 12;
  /*set imm12 -> No way that the offset will be bigger than a 12 bit value, PC is 2 behind -> +8
   * Be sure to ignore last bit when using blockCopyCacheStartAddress.*/
  offset = (u32int) currBlockCopyCacheAddr + 8 - targetAddr;
  if (offset > 0xFFF)
  {
    /* It is possible that the offset will be something like 0xFFFF????.  This is when the block is split.  The offset will be rewritten
     * when block is merged but if this offset is added we also overwrite the first bits of the instruction leading to the creation of
     * another instruction.  We can just reset the offset to 0 */
    /*serial_putstring("offset = ");
     serial_putint(offset);
     serial_newline();*/
    offset = 0; /*  */
  }
  instr2Copy = instr2Copy | (offset);
  currBlockCopyCacheAddr = updateCodeCachePointer(tc, currBlockCopyCacheAddr);
  *(currBlockCopyCacheAddr++) = instr2Copy;
  return currBlockCopyCacheAddr;
}
