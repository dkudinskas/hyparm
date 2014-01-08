#include "cpuArch/state.h"

#include "instructionEmu/decoder/arm/structs.h"
#include "instructionEmu/interpreter/internals.h"
#include "instructionEmu/translator/blockCopy.h"
#include "instructionEmu/translator/arm/pcHandlers.h"


/*
 * Translates common ALU instructions, imm case
 */
void armALUimm(TranslationStore *ts, BasicBlock *block, u32int pc, u32int instruction)
{
  Instruction instr = {.raw = instruction};
  if (instr.aluImm.Rn == GPR_PC)
  {
    DEBUG(TRANSLATION, "armALUimm: translating %08x @ %08x with cc=%x, Rd=%x, Rn=%x"
          EOL, instruction, pc, instr.aluImm.cc, instr.aluImm.Rd, instr.aluImm.Rn);
    armWritePCToRegister(ts, block, instr.aluImm.cc, instr.aluImm.Rd, pc);
    instr.aluImm.Rn = instr.aluImm.Rd;
  }
  addInstructionToBlock(ts, block, instr.raw);
}


/*
 * Translates common ALU instructions, reg case
 */
void armALUreg(TranslationStore *ts, BasicBlock *block, u32int pc, u32int instruction)
{
  Instruction instr = {.raw = instruction};
  switch (instr.aluReg.opc1)
  {
    case MVN:
    {
      armShiftPCImm(ts, block, pc, instruction);
      return;
    }
    case MOV:
    {
      if (instr.aluReg.imm5 == 0) // its a mov!
        armMovPCInstruction(ts, block, pc, instruction);
      else // its a shift!
        armShiftPCImm(ts, block, pc, instruction);
      return;
    }
    default:
      printf("armALUreg: instruction %08x\n", instr.raw);
      printf("armALUreg: unimplemented opc1 %x\n", instr.aluReg.opc1);
      DIE_NOW(context, "stop");
  }

  DIE_NOW(0, "armALUreg unimplemented\n");
}


/*
 * Translates ALU instructions without Rd, imm case
 */
void armALUimmNoDest(TranslationStore* ts, BasicBlock *block, u32int pc, u32int instruction)
{
  Instruction instr = {.raw = instruction};
  if (instr.aluImm.Rn == GPR_PC)
  {
    printf("armALUimmNoDest: translating %08x @ %08x cc=%x Rn=%x\n", 
           instr.raw, pc, instr.aluImm.cc, instr.aluImm.cc);
    // must spill a register to put correct PC value in
    // since there is no destination register to be 'dead'
    DIE_NOW(0, "armALUimmNoDest UNIMPLEMENTED\n");
  }
  addInstructionToBlock(ts, block, instruction);
}


/* this translates LDR(B) immediate case */
void armLdrPCImm(TranslationStore* ts, BasicBlock *block, u32int pc, u32int instruction)
{
  ARM_ldr_str_imm instr = {.value = instruction};
  u32int cond = instr.fields.cond;
  u32int Rt = instr.fields.Rt;
  u32int Rn = instr.fields.Rn;

  // Rt = PC -> hypercalls
  // Rn left to check:
  if (Rn == GPR_PC)
  {
    // Rn is PC, and Rn != Rt; so we can put correct value into Rt
    DEBUG(TRANSLATION, "armLdrPCImm: translating %#.8x @ %#.8x with cond=%x, Rt=%x, "
                       "Rn=%x" EOL, instruction, pc, cond, Rt, Rn);
    armWritePCToRegister(ts, block, cond, Rt, pc);
    instr.fields.Rn = Rt;
  }
  addInstructionToBlock(ts, block, instr.value);
}


/* this translates LDR(B) register case */
void armLdrPCReg(TranslationStore* ts, BasicBlock *block, u32int pc, u32int instruction)
{
  ARM_ldr_str_reg instr = {.value = instruction};
  u32int cond = instr.fields.cond;
  u32int Rt = instr.fields.Rt;
  u32int Rn = instr.fields.Rn;
  u32int Rm = instr.fields.Rm;

  u32int scratch = 0;
  u32int pcRegister = Rt;
  bool spill = FALSE;

  // Rm = PC -> unpredictable
  // Rt = PC -> hypercalls
  // Rn left to check:
  if (Rn == GPR_PC)
  {
    // Rn is PC, have to put correct value somewhere
    DEBUG(TRANSLATION, "armLdrPCReg: translating %#.8x @ %#.8x with cond=%x, Rt=%x, "
          "Rn=%x Rm=%x" EOL, instruction, pc, cond, Rt, Rn, Rm);

    // if Rm = Rt we can't use Rd to store PC value in, and need to spill a reg
    if (Rt == Rm)
    {
      scratch = getOtherRegisterOf2(Rt, Rm);
      armSpillRegister(ts, block, cond, scratch, 0);
      pcRegister = scratch;
      spill = TRUE;
    }

    armWritePCToRegister(ts, block, cond, pcRegister, pc);
    instr.fields.Rn = pcRegister;
  }

  addInstructionToBlock(ts, block, instr.value);

  if (spill)
  {
    armRestoreRegister(ts, block, cond, scratch);
  }
}


/*
 * Translates LDR(B) in register & immediate forms for which Rd!=PC
 */
void armLdrPCInstruction(TranslationStore* ts, BasicBlock *block, u32int pc, u32int instruction)
{
  ARM_ldr_str_imm instr = {.value = instruction};
  if (instr.fields.I == 0)
  {
    armLdrPCImm(ts, block, pc, instruction);
  }
  else
  {
    // register case
    armLdrPCReg(ts, block, pc, instruction);
  }
}



void armLdrdhPCInstruction(TranslationStore* ts, BasicBlock *block, u32int pc, u32int instruction)
{
  printf("armLdrdhPCInstruction: pc %08x instr %08x\n", pc, instruction);
  DIE_NOW(0, "armLdrdhPCInstruction unimplemented\n");
}


/*
 * Translates MOV for which Rd!=PC.
 */
void armMovPCInstruction(TranslationStore* ts, BasicBlock *block, u32int pc, u32int instruction)
{
  ARM_ALU_reg instr = {.value = instruction};
  u32int cond = instr.fields.cond;
  u32int Rd = instr.fields.Rd;
  u32int Rm = instr.fields.Rm;
  bool S = instr.fields.S;

  // A MOV Rd,PC can be translated to only MOVT & MOVW.
  // MOVS requires an update of the condition flags and 
  // since MOVT does not support setting flags, an extra MOVS Rd,Rd is required.
  if (Rm == GPR_PC)
  {
    // This implementation expects Rd=PC to trap
    ASSERT(Rd != GPR_PC, ERROR_NOT_IMPLEMENTED);

    DEBUG(TRANSLATION, "armMovPCInstruction: translating %#.8x @ %#.8x with cond=%x, S=%x, Rd=%x, "
          "Rm=%x" EOL, instruction, pc, cond, S, Rd, Rm);

    armWritePCToRegister(ts, block, cond, Rd, pc);

    if (!S)
    {
      return;
    }
    instr.fields.Rm = Rd;
  }

  addInstructionToBlock(ts, block, instr.value);
}


/* Translates {ASR,LSL,LSR,MVN,ROR,RRX} in immediate forms for which Rd!=PC. */
void armShiftPCImm(TranslationStore* ts, BasicBlock *block, u32int pc, u32int instruction)
{
  ARM_ALU_reg instr = {.value = instruction};

  u32int cond = instr.fields.cond;
  u32int Rd = instr.fields.Rd;
  u32int Rm = instr.fields.Rm;

  if (Rm == GPR_PC)
  {
    ASSERT(Rd != GPR_PC, ERROR_NOT_IMPLEMENTED);

    DEBUG(TRANSLATION, "armShiftPCImm: translating %08x @ %08x with cond=%x, Rd=%x, "
          "Rm=%x" EOL, instr.value, pc, cond, Rd, Rm);

    armWritePCToRegister(ts, block, cond, Rd, pc);
    instr.fields.Rm = Rd;
  }
  addInstructionToBlock(ts, block, instr.value);
}


void armStmPC(TranslationStore* ts, BasicBlock *block, u32int pc, u32int instruction)
{
  DEBUG(TRANSLATION, "armStmPC: PC=%x, instruction %08x" EOL, pc, instruction);
  DEBUG(TRANSLATION, "armStmPC: block %p csStart %p" EOL, block, block->codeStoreStart);

  ARM_ldm_stm stm;
  stm.value = instruction;

  // first, simply add the instruction, which might store an incorrect PC value
  addInstructionToBlock(ts, block, instruction);

  if ((stm.fields.register_list & 0x8000) != 0)
  {
    // PC was in list. we have to fix up the incorrectly stored value.
    u16int regList = stm.fields.register_list;
    u32int Rn      = stm.fields.Rn;
    bool wback     = stm.fields.W;
    bool add       = stm.fields.U;
    bool index     = stm.fields.P;
    u32int cond    = stm.fields.cond;
    u32int scratch, Rn_offs;

    // set scratch register to be different from base register of STM
    scratch = (Rn == 0) ? 1 : 0;

    // push scratch register.
    ARM_ldm_stm push;
    push.value = LDM_STM_BASE_VALUE;
    push.fields.register_list = 1 << scratch;
    push.fields.Rn = 13;
    push.fields.L  = 0;
    push.fields.W  = 1;
    push.fields.S  = 0;
    push.fields.U  = 0;
    push.fields.P  = 1;
    push.fields.cond = cond;
    addInstructionToBlock(ts, block, push.value);

    // put correct guest PC into scratch
    armWritePCToRegister(ts, block, cond, scratch, pc);

    bool addOffset = FALSE;
    // now we must calculate the offset where to store correct PC
    if (wback)
    {
      if (add)
      {
        DIE_NOW(0, "armStmPC: add unimplemented\n");
      }
      else
      {
        if (index)
        {
          // STMDB (pre) address offset before transfer
          // the highest register in the list (PC) is stored in old Rn-4
          // since this is writeback case, current Rn = R[n] - 4*BitCount(registers);
          Rn_offs = countBitsSet(regList)*4 - 4;
          addOffset = TRUE;

          if (Rn == 13)
          {
            // guest push'ing, and we also pushed 1 more word onto guest stack
            // adjust offset to account for it. +4
            Rn_offs += 4;
          }
        }
        else
        {
          DIE_NOW(0, "armStmPC: post unimplemented\n");
          // STMDA (post) address offset after transfer
          // the highest register in the list (PC) is stored in Rn
        }
      } // U = 0
    } // back
    else
    {
      DIE_NOW(0, "armStmPC: wback 0 unimplemented\n");
    }

    // we know where to put the correct PC value: correct it!
    ARM_ldr_str_imm str;
    str.value = STR_IMMEDIATE_BASE_VALUE;
    str.fields.imm12 = Rn_offs;
    str.fields.Rt = scratch;
    str.fields.Rn = Rn;
    str.fields.L  = 0;
    str.fields.W  = 0;
    str.fields.B  = 0;
    str.fields.U  = addOffset;
    str.fields.P  = 1;
    str.fields.I  = 0;
    str.fields.cond = cond;
    addInstructionToBlock(ts, block, str.value);

    // pop scratch register.
    ARM_ldm_stm pop;
    pop.value = LDM_STM_BASE_VALUE;
    pop.fields.register_list = 1 << scratch;
    pop.fields.Rn = 13;
    pop.fields.L  = 1;
    pop.fields.W  = 1;
    pop.fields.S  = 0;
    pop.fields.U  = 1;
    pop.fields.P  = 0;
    pop.fields.cond = cond;
    addInstructionToBlock(ts, block, pop.value);
  }
}


/*
 * Translates STR{,B,H,D} in register & immediate forms
 */
void armStrPCImm(TranslationStore* ts, BasicBlock *block, u32int pc, u32int instruction)
{
  ARM_ldr_str_imm instr = {.value = instruction};
  u32int cond = instr.fields.cond;
  u32int Rt = instr.fields.Rt;
  u32int Rn = instr.fields.Rn;

  u32int scratch = 0;
  u32int pcRegister = Rt;
  bool spill = FALSE;

  const bool replaceRt = (Rt == GPR_PC);
  const bool replaceRn = (Rn == GPR_PC);

  if (replaceRt || replaceRn)
  {
    if (!(instruction & 0x6000000) && (instruction & 0x40))
    {
      DIE_NOW(0, "strd");
    }
    else
    {
      //Instruction is STR{,B,H} immediate: only 2 registers used!
      scratch = getOtherRegisterOf2(Rt, Rn);
      armSpillRegister(ts, block, cond, scratch, 0);
      pcRegister = scratch;
      spill = TRUE;
    }

    armWritePCToRegister(ts, block, cond, pcRegister, pc);

    if (replaceRt)
    {
      instr.fields.Rt = pcRegister;
    }
    if (replaceRn)
    {
      instr.fields.Rn = pcRegister;
    }
  }

  addInstructionToBlock(ts, block, instr.value);

  if (spill)
  {
    armRestoreRegister(ts, block, cond, scratch);
  }
}


void armStrPCReg(TranslationStore* ts, BasicBlock *block, u32int pc, u32int instruction)
{
  ARM_ldr_str_reg instr = {.value = instruction};
  u32int Rt = instr.fields.Rt;
  u32int Rn = instr.fields.Rn;

  const bool replaceRt = (Rt == GPR_PC);
  const bool replaceRn = (Rn == GPR_PC);

  if (replaceRt || replaceRn)
  {
    DIE_NOW(0, "armStrPCInstruction: spill register stub.\n");
  }

  addInstructionToBlock(ts, block, instruction);

  if (replaceRt || replaceRn)
  {
    DIE_NOW(0, "armStrPCInstruction: restore register stub.\n");
  }
}


/*
 * Translates STR(B) in register & immediate forms for which Rd!=PC
 */
void armStrPCInstruction(TranslationStore* ts, BasicBlock *block, u32int pc, u32int instruction)
{
  ARM_ldr_str_imm instr = {.value = instruction};
  if (instr.fields.I == 0)
  {
    armStrPCImm(ts, block, pc, instruction);
  }
  else
  {
    // register case
    armStrPCReg(ts, block, pc, instruction);
  }
}



void armStrtPCInstruction(TranslationStore* ts, BasicBlock *block, u32int pc, u32int instruction)
{
  DIE_NOW(0, "armStrtPCInstruction unimplemented\n");
}

