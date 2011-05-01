#include "common/debug.h"

#include "guestManager/guestContext.h"

#include "instructionEmu/dataMoveInstr.h"
#include "instructionEmu/commonInstrFunctions.h"

#include "memoryManager/globalMemoryMapper.h"


extern GCONTXT * getGuestContext(void); //from main.c


/* generic load store instruction emulation  *
 * called when we permission fault on memory *
 * access to a protected area - must emulate */
void emulateLoadStoreGeneric(GCONTXT * context, u32int address)
{
  u32int instr = 0;
  u32int eobInstrBackup = 0;
  u32int eobHalfInstrBackup =0;
  bool thumb32 = 0;
  if(context->CPSR & T_BIT) //Thumb
  {
	eobInstrBackup = context->endOfBlockInstr;
	eobHalfInstrBackup = context->endOfBlockHalfInstr;
	//take care on what you fetch. Segmentation fault is just around the corner
	if((((u32int)context->R15) & 0x3 ) < 0x2)
	{
	  	instr = *((u32int*)context->R15);
	}
	else
	{	
		instr = *((u16int*)context->R15);
	}
	printf("Storing on %08x, full instr %08x\n",address,instr);
	instr = decodeThumbInstr(context,instr);
    printf("Found %08x\n",instr);
	thumb32 = isThumb32(instr);
	if(thumb32)
	{
		DIE_NOW(0,"Unimplemented Thumb32 generic load/store");
	}
	if ( ( instr & THUMB16_STR_IMM5 ) || instr & THUMB16_STR_IMM8 )
	{
		// validate cache if needed
		validateCachePreChange(context->blockCache, address);
		// cheat strInstruction. We know that this is a thumb16 instr
		// so make sure that decoder will recognize it :-/
		context->endOfBlockInstr = instr;
		context->endOfBlockHalfInstr = LHALF; 
		strInstruction(context);
	}
	context->endOfBlockInstr = eobInstrBackup;
	context->endOfBlockHalfInstr = eobHalfInstrBackup; 
  }
  else //ARM
  {
	  // get the store instruction
	  instr = *((u32int*)context->R15);
	  // save the end of block instruction, as we faulted /not/ on EOB
	  eobInstrBackup = context->endOfBlockInstr;
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
	  else if ( ((instr & STRD_IMM_MASK) == STRD_IMM_MASKED) ||
            ((instr & STRD_REG_MASK) == STRD_REG_MASKED) )
  	  {
	    // storing to a protected area.. adjust block cache if needed
    	validateCachePreChange(context->blockCache, address);
	    // STRD Rd, [Rn, Rm/#imm12]
    	strdInstruction(context);
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
	  else if ( ((instr & LDRD_IMM_MASK) == LDRD_IMM_MASKED) ||
            ((instr & LDRD_REG_MASK) == LDRD_REG_MASKED) )
      {
	    // LDRD Rd, [Rn, Rm/#imm12]
    	ldrdInstruction(context);
	  }
	  else if ((instr & LDM_MASK) == LDM_MASKED)
	  {
    	// LDM, Rn, {reg list}
	    ldmInstruction(context);
  	  }
	  else if ((instr & LDREX_MASK) == LDREX_MASKED)
  	  {
	    // LDREX Rd, [Rn]
    	ldrexInstruction(context);
	  }
	  else
	  {
    	printf("LoadStore @ %08x instruction %08x\n", context->R15, instr);
	    DIE_NOW(context, "Load/Store generic unimplemented\n");
  	  } 
	  // restore end of block instruction 
	  context->endOfBlockInstr = eobInstrBackup;
	}
}
