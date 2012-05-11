#include "common/stddef.h"

#include "cpuArch/constants.h"

#include "instructionEmu/interpreter/blockCopy.h"
#include "instructionEmu/interpreter/internals.h"


u32int *armBackupRegisterToSpill(TranslationCache *tc, u32int *code, u32int conditionCode, u32int reg)
{
  ASSERT(conditionCode <= CC_AL, "invalid condition code");
  ASSERT(reg < 13, "invalid temporary register");

  code = updateCodeCachePointer(tc, code);
  u32int *pc = code + 2;
  bool add = tc->spillPage >= pc;
  u32int offset = (add ? (tc->spillPage - pc) : (pc - tc->spillPage)) * sizeof(u32int);

  ASSERT(offset <= 4095, "spill location unreachable from this location in C$");

  // assemble STR(offset, index=1,wback=0) to available spill location. currently always in spill page
  //|COND|010P|U0W0| Rn | Rt |    imm12   |
  *code = (conditionCode << 28) | (0b0101 << 24) | ((u32int)add << 23) | (GPR_PC << 16) | (reg << 12) | offset;
  return ++code;
}

u32int *armRestoreRegisterFromSpill(TranslationCache *tc, u32int *code, u32int conditionCode, u32int reg)
{
  ASSERT(conditionCode <= CC_AL, "invalid condition code");
  ASSERT(reg < 13, "invalid temporary register");

  code = updateCodeCachePointer(tc, code);
  u32int *pc = code + 2;
  bool add = tc->spillPage >= pc;
  u32int offset = (add ? (tc->spillPage - pc) : (pc - tc->spillPage)) * sizeof(u32int);

  ASSERT(offset <= 4095, "spill location unreachable from this location in C$");

  // assemble LDR(offset, index=1,wback=0) to available spill location. currently always in spill page
  //|COND|010P|U0W1| Rn | Rt |    imm12   |
  *code = (conditionCode << 28) | (0b0101 << 24) | ((u32int)add << 23) | (1 << 20) | (GPR_PC << 16) | (reg << 12) | offset;
  return ++code;
}

u32int *armWritePCToRegister(TranslationCache *tc, u32int *code, u32int conditionCode, u32int reg, u32int pc)
{
  ASSERT(conditionCode <= CC_AL, "invalid condition code");
  ASSERT(reg < 13, "invalid temporary register");

  pc += 8;

  // assemble MOVW
  //MOVW -> ARM ARM A8.6.96 p506
  //|COND|0011|0000|imm4| Rd |    imm12   |
  code = updateCodeCachePointer(tc, code);
  *code = (conditionCode << 28) | (0b00110000 << 20) | ((pc & 0xF000) << 4) | (reg << 12) | (pc & 0x0FFF);

  pc >>= 16;

  // assemble MOVT
  //MOVT -> ARM ARM A8.6.99 p512
  //|COND|0011|0100|imm4| Rd |    imm12   |
  code = updateCodeCachePointer(tc, ++code);
  *code = (conditionCode << 28) | (0b00110100 << 20) | ((pc & 0xF000) << 4) | (reg << 12) | (pc & 0x0FFF);

  return ++code;
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
 *     List of instructions: ADD, ADC, AND, BIC, EOR, MVN, ORR RSB, RSC, SBC, SUB
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

  u32int conditionCode = ARM_EXTRACT_CONDITION_CODE(instruction);

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

    currBlockCopyCacheAddr = armWritePCToRegister(tc, currBlockCopyCacheAddr, conditionCode, destReg, (u32int)instructionAddr);

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

/*
 * Similar to standardImmRegRSR but there is no destination register which makes it
 * impossible to use the same trick.  A scratch register is needed.
 * List of instructions: CMN, CMP, TEQ, TST
 */
u32int* standardImmRegRSRNoDest(TranslationCache *tc, u32int * instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  u32int instruction = *instructionAddr;
  bool immediate = ((instruction >> 25) & 0b1) == 0b1;
  u32int regSrc1 = (instruction >> 16) & 0xF;
  u32int regSrc2 = (instruction) & 0xF;
  bool rnIsPC = (regSrc1) == 0xF; /* see ARM ARM p.768 */
  bool rmIsPC = (regSrc2) == 0xF;
  u32int instr2Copy = instruction;
  u32int scratchReg = 0;

  /* if ordinary register flavor than rnIsPC & rmIsPC are already set correctly */
  if (immediate)
  {
    rmIsPC = 0;/* Immediate doesn't use second register -> always false */
  }

  if (rnIsPC || rmIsPC)
  { /*Instruction has to be changed to a PC safe instructionstream. */
    printf("instruction = %08x\n", instruction);
    DIE_NOW(NULL, "standardImmRegRSRNoDest: this part is not tested.");
    /* No destination register so only source registers have to be checked*/
    scratchReg = getOtherRegisterOf2(regSrc1, regSrc2);
    /* place 'Backup scratchReg' instruction */
    currBlockCopyCacheAddr = armBackupRegisterToSpill(tc, currBlockCopyCacheAddr, CC_AL, scratchReg);
    currBlockCopyCacheAddr = armWritePCToRegister(tc, currBlockCopyCacheAddr, CC_AL, scratchReg, (u32int)instructionAddr);

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
    currBlockCopyCacheAddr = armRestoreRegisterFromSpill(tc, currBlockCopyCacheAddr, CC_AL, scratchReg);
  }

  return currBlockCopyCacheAddr;
}
