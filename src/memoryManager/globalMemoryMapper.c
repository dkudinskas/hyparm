#include "globalMemoryMapper.h"
#include "guestContext.h"
#include "dataMoveInstr.h"

extern GCONTXT * getGuestContext(void); //from main.c


/* generic load store instruction emulation  *
 * called when we permission fault on memory *
 * access to a protected area - must emulate */
void emulateLoadStoreGeneric(GCONTXT * context, u32int address)
{
  // get the store instruction
  u32int instr = *((u32int*)context->R15);
  // save the end of block instruction, as we faulted /not/ on EOB
  u32int eobInstrBackup = context->endOfBlockInstr;
  // emulate methods will take instr from context, put it there
  context->endOfBlockInstr = instr;
  if ( ((instr & STR_IMM_MASK) == STR_IMM_MASKED) ||
       ((instr & STR_REG_MASK) == STR_REG_MASKED) )
  {
    // storing to a protected area.. adjust block cache if needed
    validateCachePreChange(context->blockCache, address);
    // STR Rd, [Rn, Rm/#imm12]
    strInstruction(context);
  }
  else if ( ((instr & STRB_IMM_MASK) == STRB_IMM_MASKED) ||
            ((instr & STRB_REG_MASK) == STRB_REG_MASKED) )
  {
    // storing to a protected area.. adjust block cache if needed
    validateCachePreChange(context->blockCache, address);
    // STRB Rd, [Rn, Rm/#imm12]
    strbInstruction(context);
  }
  else if ( ((instr & STRH_IMM_MASK) == STRH_IMM_MASKED) ||
            ((instr & STRH_REG_MASK) == STRH_REG_MASKED) )
  {
    // storing to a protected area.. adjust block cache if needed
    validateCachePreChange(context->blockCache, address);
    // STRH Rd, [Rn, Rm/#imm12]
    strhInstruction(context);
  }
  else if ((instr & STM_MASK) == STM_MASKED)
  {
    // more tricky with cache validation! since we do this in the stmInstruction
    // per address (word in memory) depending on the length of {reg list}
    // STM Rn, {reg list}
    stmInstruction(context);
  }
  else if ((instr & LDR_MASK) == LDR_MASKED)
  {
    // loads don't change memory. no need to validate cache
    // LDR Rd, Rn/#imm12
    ldrInstruction(context);
  }
  else if ((instr & LDRB_MASK) == LDRB_MASKED)
  {
    // LDRB Rd, [Rn, Rm/#imm12]
    ldrbInstruction(context);
  }
  else if ( ((instr & LDRH_IMM_MASK) == LDRH_IMM_MASKED) ||
            ((instr & LDRH_REG_MASK) == LDRH_REG_MASKED) )
  {
    // LDRH Rd, [Rn, Rm/#imm12]
    ldrhInstruction(context);
  }
  else if ((instr & LDM_MASK) == LDM_MASKED)
  {
    // LDM, Rn, {reg list}
    ldmInstruction(context);
  }
  else
  {
    serial_putstring("LoadStore @ ");
    serial_putint(context->R15);
    serial_putstring(" instruction ");
    serial_putint(instr);
    serial_newline(); 
    serial_ERROR("Load/Store generic unimplemented\n");
  } 
  // restore end of block instruction 
  context->endOfBlockInstr = eobInstrBackup;
}
