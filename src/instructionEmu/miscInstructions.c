#include "common/debug.h"

#include "vm/omap35xx/intc.h"

#include "instructionEmu/commonInstrFunctions.h"
#include "instructionEmu/miscInstructions.h"

#include "guestManager/scheduler.h"
#include "guestManager/guestExceptions.h"

#define T_BIT 	0x20
u32int nopInstruction(GCONTXT * context)
{
  printf("ERROR: NOP instr %08x @ %08x should not have trapped!\n",
         context->endOfBlockInstr, context->R15);
  DIE_NOW(context, "die.");
  return 0;
}

u32int bxInstruction(GCONTXT * context)
{
  u32int instr = context->endOfBlockInstr;
  
  u32int nextPC = 0;

  u32int instrCC = (instr >> 28) & 0xF;
  u32int cpsrCC = (context->CPSR >> 28) & 0xF;

  if (!evalCC(instrCC, cpsrCC))
  {
      nextPC = context->R15 + 4;
      return nextPC;
  }

  //check if switching to thumb mode
  u32int regDest = (instr & 0x0000000F);
  u32int addr = loadGuestGPR(regDest, context);

  if (addr & 0x1)
  {
    DIE_NOW(context, "BX Rm switching to Thumb. Unimplemented\n");
  }

  nextPC = addr & 0xFFFFFFFE;
  return nextPC;
}

u32int mulInstruction(GCONTXT * context)
{
  DIE_NOW(context, "MUL unfinished\n");
}

u32int mlaInstruction(GCONTXT * context)
{
  DIE_NOW(context, "MLA unfinished\n");
}

u32int swpInstruction(GCONTXT * context)
{
  DIE_NOW(context, "SWP unfinished\n");
  return 0;
}

u32int sumlalInstruction(GCONTXT * context)
{
  DIE_NOW(context, "SUMLAL unfinished\n");
  return 0;
}

u32int sumullInstruction(GCONTXT * context)
{
  DIE_NOW(context, "SUMULL unfinished\n");
  return 0;
}

u32int pliInstruction(GCONTXT * context)
{
#ifdef ARM_INSTR_TRACE
  printf("Warning: PLI!\n");
#endif
  return context->R15+4;
}

u32int dbgInstruction(GCONTXT * context)
{
  DIE_NOW(context, "DBG unfinished\n");
  return 0;
}

u32int dmbInstruction(GCONTXT * context)
{
  DIE_NOW(context, "DBM unfinished\n");
  return 0;
}

u32int dsbInstruction(GCONTXT * context)
{
#ifdef ARM_INSTR_TRACE
  printf("Warning: DSB (ignored)!\n");
#endif
  return context->R15+4;
}

u32int isbInstruction(GCONTXT * context)
{
#ifdef ARM_INSTR_TRACE
  printf("Warning: ISB (ignored)!\n");
#endif
  return context->R15+4;
}

u32int bfcInstruction(GCONTXT * context)
{
  DIE_NOW(context, "BFC unfinished\n");
  return 0;
}

u32int bfiInstruction(GCONTXT * context)
{
  DIE_NOW(context, "BFI unfinished\n");
  return 0;
}

u32int mlsInstruction(GCONTXT * context)
{
  DIE_NOW(context, "MLS unfinished\n");
  return 0;
}

u32int movwInstruction(GCONTXT * context)
{
  DIE_NOW(context, "MOVW unfinished\n");
  return 0;
}

u32int movtInstruction(GCONTXT * context)
{
  DIE_NOW(context, "MOVT unfinished\n");
  return 0;
}

u32int rbitInstruction(GCONTXT * context)
{
  DIE_NOW(context, "RBIT unfinished\n");
  return 0;
}

u32int usbfxInstruction(GCONTXT * context)
{
  DIE_NOW(context, "USBFX unfinished\n");
  return 0;
}

u32int smcInstruction(GCONTXT * context)
{
  DIE_NOW(context, "SMC unfinished\n");
  return 0;
}

u32int clrexInstruction(GCONTXT * context)
{
#ifdef ARM_INSTR_TRACE
  printf("Warning: CLREX!\n");
#endif
  return context->R15+4;
}

u32int yieldInstruction(GCONTXT * context)
{
  DIE_NOW(context, "YIELD unfinished\n");
  return 0;
}

u32int wfeInstruction(GCONTXT * context)
{
  DIE_NOW(context, "WFE unfinished\n");
  return 0;
}

u32int wfiInstruction(GCONTXT * context)
{
  // stop guest execution...
  guestIdle(context);
  return context->R15+4;
}

u32int sevInstruction(GCONTXT * context)
{
  DIE_NOW(context, "SEV unfinished\n");
  return 0;
}

u32int cpsieInstruction(GCONTXT * context)
{
  return cpsInstruction(context);
}

u32int cpsidInstruction(GCONTXT * context)
{
  return cpsInstruction(context);
}

u32int cpsInstruction(GCONTXT * context)
{
  u32int instr = context->endOfBlockInstr;
  u32int imod       = (instr & 0x000C0000) >> 18;
  u32int changeMode = (instr & 0x00020000) >> 17;
  u32int affectA    = (instr & 0x00000100) >>  8;
  u32int affectI    = (instr & 0x00000080) >>  7;
  u32int affectF    = (instr & 0x00000040) >>  6;
  u32int newMode    =  instr & 0x0000001F;
#ifdef ARM_INSTR_TRACE
  printf("CPS instr %08x @ %08x\n", instr, context->R15);
#endif

  if ( ((imod == 0) && (changeMode == 0)) || (imod == 1) )
  {
    DIE_NOW(context, "CPS unpredictable case\n");
  }

  if (guestInPrivMode(context))
  {
    u32int oldCpsr = context->CPSR;
    if (imod == 0x2) // enable
    {
#ifdef ARM_INSTR_TRACE
      printf("IMod: enable case\n");
#endif
      if (affectA != 0)
      {
        if ((oldCpsr & CPSR_AAB_BIT) != 0)
        {
          DIE_NOW(context, "Guest enabling async aborts globally!");
        }
        oldCpsr &= ~CPSR_AAB_BIT;
      }
      if (affectI)
      {
        if ( (oldCpsr & CPSR_IRQ_BIT) != 0)
        {
#ifdef ARM_INSTR_TRACE
          printf("Guest enabling irqs globally!\n");
#endif
          // chech interrupt controller if there is an interrupt pending
          if (isIrqPending())
          {
            context->guestIrqPending = TRUE;
          }
        } 
        oldCpsr &= ~CPSR_IRQ_BIT;
      }
      if (affectF)
      {
        if ( (oldCpsr & CPSR_FIQ_BIT) != 0)
        {
#ifdef ARM_INSTR_TRACE
          printf("Guest enabling FIQs globally!\n");
#endif
          // chech interrupt controller if there is an interrupt pending
          if (isFiqPending())
          {
            // context->guestFiqPending = TRUE; : IMPLEMENT!!
            DIE_NOW(context, "cps: FIQ pending!! unimplemented.");
          }
        } 
        oldCpsr &= ~CPSR_FIQ_BIT;
      }
    }
    else if (imod == 3) // disable
    {
      if (affectA)
      {
        if ((oldCpsr & CPSR_AAB_BIT) == 0)
        {
          DIE_NOW(context, "Guest disabling async aborts globally!");
        }
        oldCpsr |= CPSR_AAB_BIT;
      }
      if (affectI)
      {
        if ( (oldCpsr & CPSR_IRQ_BIT) == 0) // were enabled, now disabled
        {
          // chech interrupt controller if there is an interrupt pending
          if(context->guestIrqPending == TRUE)
          {
            context->guestIrqPending = FALSE;
          }
        } 
        oldCpsr |= CPSR_IRQ_BIT;
      }
      if (affectF)
      {
        if ( (oldCpsr & CPSR_FIQ_BIT) == 0)
        {
          DIE_NOW(context, "Guest disabling fiqs globally!");
        } 
        oldCpsr |= CPSR_FIQ_BIT;
      }
    }
    else
    {
      DIE_NOW(context, "CPS invalid IMOD\n");
    }
    // ARE we switching modes?
    if (changeMode)
    {
      oldCpsr &= ~CPSR_MODE_FIELD;
      oldCpsr |= newMode;
      DIE_NOW(context, "guest is changing execution modes. What?!");
    }
    context->CPSR = oldCpsr;
  }
  else
  {
    // guest is not in privileged mode! cps should behave as a nop, but lets see what went wrong.
    DIE_NOW(context, "CPS instruction: executed in guest user mode.");
  }

  return context->R15+4;
}

u32int pkhbtInstruction(GCONTXT * context)
{
  DIE_NOW(context, "PKHBT unfinished\n");
  return 0;
}

u32int pkhtbInstruction(GCONTXT * context)
{
  DIE_NOW(context, "PKHTB unfinished\n");
  return 0;
}

u32int qadd16Instruction(GCONTXT * context)
{
  DIE_NOW(context, "QADD16 unfinished\n");
  return 0;
}

u32int qadd8Instruction(GCONTXT * context)
{
  DIE_NOW(context, "QADD8 unfinished\n");
  return 0;
}

u32int qaddsubxInstruction(GCONTXT * context)
{
  DIE_NOW(context, "QADDSUBX unfinished\n");
  return 0;
}

u32int qsub16Instruction(GCONTXT * context)
{
  DIE_NOW(context, "QSUB16 unfinished\n");
  return 0;
}

u32int qsub8Instruction(GCONTXT * context)
{
  DIE_NOW(context, "QSUB8 unfinished\n");
  return 0;
}

u32int qsubaddxInstruction(GCONTXT * context)
{
  DIE_NOW(context, "QSUBADDX unfinished\n");
  return 0;
}

u32int sadd16Instruction(GCONTXT * context)
{
  DIE_NOW(context, "SADD16 unfinished\n");
  return 0;
}

u32int sadd8Instruction(GCONTXT * context)
{
  DIE_NOW(context, "SADD8 unfinished\n");
  return 0;
}

u32int saddsubxInstruction(GCONTXT * context)
{
  DIE_NOW(context, "SADDADDX unfinished\n");
  return 0;
}

u32int shadd16Instruction(GCONTXT * context)
{
  DIE_NOW(context, "SHADD16 unfinished\n");
  return 0;
}

u32int shadd8Instruction(GCONTXT * context)
{
  DIE_NOW(context, "SHADD8 unfinished\n");
  return 0;
}

u32int shaddsubxInstruction(GCONTXT * context)
{
  DIE_NOW(context, "SHADDSUBX unfinished\n");
  return 0;
}

u32int shsub16Instruction(GCONTXT * context)
{
  DIE_NOW(context, "SHSUB16 unfinished\n");
  return 0;
}

u32int shsub8Instruction(GCONTXT * context)
{
  DIE_NOW(context, "SHSUB8 unfinished\n");
  return 0;
}

u32int shsubaddxInstruction(GCONTXT * context)
{
  DIE_NOW(context, "SHSUBADDX unfinished\n");
  return 0;
}

u32int ssub16Instruction(GCONTXT * context)
{
  DIE_NOW(context, "SSUB16 unfinished\n");
  return 0;
}

u32int ssub8Instruction(GCONTXT * context)
{
  DIE_NOW(context, "SSUB8 unfinished\n");
  return 0;
}

u32int ssubaddxInstruction(GCONTXT * context)
{
  DIE_NOW(context, "SSUBADDX unfinished\n");
  return 0;
}

u32int uadd16Instruction(GCONTXT * context)
{
  DIE_NOW(context, "UADD16 unfinished\n");
  return 0;
}

u32int uadd8Instruction(GCONTXT * context)
{
  DIE_NOW(context, "UADD8 unfinished\n");
  return 0;
}

u32int uaddsubxInstruction(GCONTXT * context)
{
  DIE_NOW(context, "UADDSUBX unfinished\n");
  return 0;
}

u32int uhadd16Instruction(GCONTXT * context)
{
  DIE_NOW(context, "UHADD16 unfinished\n");
  return 0;
}

u32int uhadd8Instruction(GCONTXT * context)
{
  DIE_NOW(context, "UHADD8 unfinished\n");
  return 0;
}

u32int uhaddsubxInstruction(GCONTXT * context)
{
  DIE_NOW(context, "UHADDSUBX unfinished\n");
  return 0;
}

u32int uhsub16Instruction(GCONTXT * context)
{
  DIE_NOW(context, "UHSUB16 unfinished\n");
  return 0;
}

u32int uhsub8Instruction(GCONTXT * context)
{
  DIE_NOW(context, "UHSUB8 unfinished\n");
  return 0;
}

u32int uhsubaddxInstruction(GCONTXT * context)
{
  DIE_NOW(context, "UHSUBADDX unfinished\n");
  return 0;
}

u32int uqadd16Instruction(GCONTXT * context)
{
  DIE_NOW(context, "UQADD16 unfinished\n");
  return 0;
}

u32int uqadd8Instruction(GCONTXT * context)
{
  DIE_NOW(context, "UQADD8 unfinished\n");
  return 0;
}

u32int uqaddsubxInstruction(GCONTXT * context)
{
  DIE_NOW(context, "UQADDSUBX unfinished\n");
  return 0;
}

u32int uqsub16Instruction(GCONTXT * context)
{
  DIE_NOW(context, "UQSUB16 unfinished\n");
  return 0;
}
u32int uqsub8Instruction(GCONTXT * context)
{
  DIE_NOW(context, "UQSUB8 unfinished\n");
  return 0;
}

u32int uqsubaddxInstruction(GCONTXT * context)
{
  DIE_NOW(context, "UQSUBADDX unfinished\n");
  return 0;
}

u32int usub16Instruction(GCONTXT * context)
{
  DIE_NOW(context, "USUB16 unfinished\n");
  return 0;
}

u32int usub8Instruction(GCONTXT * context)
{
  DIE_NOW(context, "USUB8 unfinished\n");
  return 0;
}

u32int usubaddxInstruction(GCONTXT * context)
{
  DIE_NOW(context, "USUBADDX unfinished\n");
  return 0;
}

u32int revInstruction(GCONTXT * context)
{
  DIE_NOW(context, "REV unfinished\n");
  return 0;
}

u32int rev16Instruction(GCONTXT * context)
{
  DIE_NOW(context, "REV16 unfinished\n");
  return 0;
}

u32int revshInstruction(GCONTXT * context)
{
  DIE_NOW(context, "REVSH unfinished\n");
  return 0;
}

u32int rfeInstruction(GCONTXT * context)
{
  DIE_NOW(context, "RFE unfinished\n");
  return 0;
}

u32int sxthInstruction(GCONTXT * context)
{
  u32int instr = context->endOfBlockInstr;
  u32int instrCC = (instr >> 28) & 0xF;
  u32int cpsrCC = (context->CPSR >> 28) & 0xF;
  u32int regSrc = (instr & 0xF);
  u32int regDest = (instr & 0x0000F000) >> 12;
  u32int rotate = (instr & 0x00000C00) >> 10;
  u32int value = 0;
  bool conditionMet;
  if(regDest==15 || regSrc==15)
  {
  	DIE_NOW(0,"Rd/Rm is R15. Unpredictable behaviour\n");
  }
  conditionMet = evalCC(instrCC, cpsrCC);
  if (conditionMet)
  {
  	/* load the least 16bits from the source register */
	value=(loadGuestGPR(regSrc,context) & 0x0000FFFF);
    /* ARM7-A : page 729 */
	switch(rotate){
		case 0:
			value = rorVal(value,SXTH_R0);
			break;
		case 1:
			value = rorVal(value,SXTH_R8);
			break;
		case 2:
			value = rorVal(value,SXTH_R16);
			break;
		case 3:
			value = rorVal(value,SXTH_R24);
			break;
	}
	/* Extend it to 32bit */
	value = value<<16;
	/* Store it */
	storeGuestGPR(regDest,value,context);
  }
  return context->R15+4;
}

u32int sxtb16Instruction(GCONTXT * context)
{
  DIE_NOW(context, "SXTB16 unfinished\n");
  return 0;
}

u32int sxtbInstruction(GCONTXT * context)
{
  DIE_NOW(context, "SXTB unfinished\n");
  return 0;
}

u32int uxthInstruction(GCONTXT * context)
{
  DIE_NOW(context, "UXTHunfinished\n");
  return 0;
}

u32int uxtb16Instruction(GCONTXT * context)
{
  DIE_NOW(context, "UXTB16 unfinished\n");
  return 0;
}

u32int uxtbInstruction(GCONTXT * context)
{
  DIE_NOW(context, "UXTB unfinished\n");
  return 0;
}

u32int sxtahInstruction(GCONTXT * context)
{
  DIE_NOW(context, "SXTAH unfinished\n");
  return 0;
}

u32int sxtab16Instruction(GCONTXT * context)
{
  DIE_NOW(context, "SXTAB16 unfinished\n");
  return 0;
}

u32int sxtabInstruction(GCONTXT * context)
{
  DIE_NOW(context, "SXTAB unfinished\n");
  return 0;
}

u32int uxtahInstruction(GCONTXT * context)
{
  DIE_NOW(context, "UXTAH unfinished\n");
  return 0;
}

u32int uxtab16Instruction(GCONTXT * context)
{
  DIE_NOW(context, "UXTAB16 unfinished\n");
  return 0;
}

u32int uxtabInstruction(GCONTXT * context)
{
  DIE_NOW(context, "UXTAB unfinished\n");
  return 0;
}

u32int selInstruction(GCONTXT * context)
{
  DIE_NOW(context, "SEL unfinished\n");
  return 0;
}

u32int setendInstruction(GCONTXT * context)
{
  DIE_NOW(context, "SETEND unfinished\n");
  return 0;
}

u32int smuadInstruction(GCONTXT * context)
{
  DIE_NOW(context, "SMUAD unfinished\n");
  return 0;
}

u32int smusdInstruction(GCONTXT * context)
{
  DIE_NOW(context, "SMUSD unfinished\n");
  return 0;
}

u32int smladInstruction(GCONTXT * context)
{
  DIE_NOW(context, "SMLAD unfinished\n");
  return 0;
}

u32int smlaldInstruction(GCONTXT * context)
{
  DIE_NOW(context, "SMLALD unfinished\n");
  return 0;
}

u32int smlsdInstruction(GCONTXT * context)
{
  DIE_NOW(context, "SMLSD unfinished\n");
  return 0;
}

u32int smlsldInstruction(GCONTXT * context)
{
  DIE_NOW(context, "SMLSLD unfinished\n");
  return 0;
}

u32int smmulInstruction(GCONTXT * context)
{
  DIE_NOW(context, "SMMUL unfinished\n");
  return 0;
}

u32int smmlaInstruction(GCONTXT * context)
{
  DIE_NOW(context, "SMMLA unfinished\n");
  return 0;
}

u32int smmlsInstruction(GCONTXT * context)
{
  DIE_NOW(context, "SMMLS unfinished\n");
  return 0;
}

u32int srsInstruction(GCONTXT * context)
{
  DIE_NOW(context, "SRS unfinished\n");
  return 0;
}

u32int ssatInstruction(GCONTXT * context)
{
  DIE_NOW(context, "SSAT unfinished\n");
  return 0;
}

u32int ssat16Instruction(GCONTXT * context)
{
  DIE_NOW(context, "ssat16 unfinished\n");
  return 0;
}

u32int umaalInstruction(GCONTXT * context)
{
  DIE_NOW(context, "UMAAL unfinished\n");
  return 0;
}

u32int usad8Instruction(GCONTXT * context)
{
  DIE_NOW(context, "USAD8 unfinished\n");
  return 0;
}

u32int usada8Instruction(GCONTXT * context)
{
  DIE_NOW(context, "USADA unfinished\n");
  return 0;
}

u32int usatInstruction(GCONTXT * context)
{
  DIE_NOW(context, "USAT unfinished\n");
  return 0;
}

u32int usat16Instruction(GCONTXT * context)
{
  DIE_NOW(context, "USAT16 unfinished\n");
  return 0;
}

u32int bxjInstruction(GCONTXT * context)
{
  DIE_NOW(context, "BXJ unfinished\n");
  return 0;
}

u32int bkptInstruction(GCONTXT * context)
{
  DIE_NOW(context, "BKPT unfinished\n");
  return 0;
}

u32int blxInstruction(GCONTXT * context)
{
  u32int instr = context->endOfBlockInstr;
  u32int nextPC = 0;
  u32int sign = 0x00800000 & instr;
  u32int instrCC = (instr >> 28) & 0xF;
  u32int cpsrCC  = (context->CPSR >> 28) & 0xF;
  u32int regDest = (instr & 0x0000000F); // holds dest addr and mode bit
  u32int value = loadGuestGPR(regDest, context);
  u32int thumbMode = value & 0x1;
  u32int target = instr & 0x00FFFFFF;

  if ((instr & 0xfe000000) == 0xfa000000)
  {
  	if (sign != 0)
  	{
    	// target negative!
    		target |= 0xFF000000;
  	}
  	target = target << 2;
	
	// blx <imm24>
  	context->CPSR |= T_BIT;
	storeGuestGPR(14, context->R15+4,context);

	nextPC = context->R15 + 8 + target;
	return nextPC;
  }

  if (!evalCC(instrCC, cpsrCC))
  {
    nextPC = context->R15 + 4;
    return nextPC;
  }

  if (thumbMode)
  {
      DIE_NOW(context, "BLX Rm switching to Thumb. Unimplemented.\n");
  }

  // link register
  storeGuestGPR(14, context->R15+4, context);

  nextPC = value & 0xFFFFFFFE;
  return nextPC;
}

u32int clzInstruction(GCONTXT * context)
{
  DIE_NOW(context, "CLZ unfinished\n");
  return 0;
}

u32int pldInstruction(GCONTXT * context)
{
#ifdef ARM_INSTR_TRACE
  printf("Warning: PLD!\n");
#endif
  return context->R15+4;
}

u32int smlabbInstruction(GCONTXT * context)
{
  DIE_NOW(context, "SMLABB unfinished\n");
  return 0;
}

u32int smlatbInstruction(GCONTXT * context)
{
  DIE_NOW(context, "SMLATB unfinished\n");
  return 0;
}

u32int smlabtInstruction(GCONTXT * context)
{
  DIE_NOW(context, "SMLABT unfinished\n");
  return 0;
}

u32int smlattInstruction(GCONTXT * context)
{
  DIE_NOW(context, "SMLATT unfinished\n");
  return 0;
}

u32int smlawbInstruction(GCONTXT * context)
{
  DIE_NOW(context, "SMLAWB unfinished\n");
  return 0;
}

u32int smlawtInstruction(GCONTXT * context)
{
  DIE_NOW(context, "SMLAWT unfinished\n");
  return 0;
}

u32int smlalbbInstruction(GCONTXT * context)
{
  DIE_NOW(context, "SMLALBB unfinished\n");
  return 0;
}

u32int smlaltbInstruction(GCONTXT * context)
{
  DIE_NOW(context, "SMLALTB unfinished\n");
  return 0;
}

u32int smlalbtInstruction(GCONTXT * context)
{
  DIE_NOW(context, "SMLALBT unfinished\n");
  return 0;
}

u32int smlalttInstruction(GCONTXT * context)
{
  DIE_NOW(context, "SMLALTT unfinished\n");
  return 0;
}

u32int smulbbInstruction(GCONTXT * context)
{
  DIE_NOW(context, "SMULBB unfinished\n");
  return 0;
}

u32int smultbInstruction(GCONTXT * context)
{
  DIE_NOW(context, "SMULTB unfinished\n");
  return 0;
}

u32int smulbtInstruction(GCONTXT * context)
{
  DIE_NOW(context, "SMULBT unfinished\n");
  return 0;
}

u32int smulttInstruction(GCONTXT * context)
{
  DIE_NOW(context, "SMULTT unfinished\n");
  return 0;
}

u32int smulwbInstruction(GCONTXT * context)
{
  DIE_NOW(context, "SMULWD unfinished\n");
  return 0;
}

u32int smulwtInstruction(GCONTXT * context)
{
  DIE_NOW(context, "SMULWT unfinished\n");
  return 0;
}

u32int qaddInstruction(GCONTXT * context)
{
  DIE_NOW(context, "QADD unfinished\n");
  return 0;
}

u32int qdaddInstruction(GCONTXT * context)
{
  DIE_NOW(context, "QDADD unfinished\n");
  return 0;
}

u32int qsubInstruction(GCONTXT * context)
{
  DIE_NOW(context, "QSUB unfinished\n");
  return 0;
}

u32int qdsubInstruction(GCONTXT * context)
{
  DIE_NOW(context, "QDSUB unfinished\n");
  return 0;
}

u32int msrInstruction(GCONTXT * context)
{
  u32int instr = context->endOfBlockInstr;
  u32int instrCC =    (instr & 0xF0000000) >> 28;
  u32int regOrImm =   (instr & 0x02000000); // if 1 then imm12, 0 then Reg
  u32int cpsrOrSpsr = (instr & 0x00400000); // if 0 then cpsr, !0 then spsr
  u32int fieldMsk =   (instr & 0x000F0000) >> 16;
  
  u32int value = 0;
  u32int nextPC = 0;


  u32int cpsrCC = (context->CPSR >> 28) & 0xF;
  if (!evalCC(instrCC, cpsrCC))
  {
    nextPC = context->R15 + 4;
    return nextPC;
  }
  
  if (regOrImm == 0)
  {
    // register case
    u32int regSrc = instr & 0x0000000F;
    if (regSrc == 0xF)
    {
      invalidInstruction(instr, "MSR cannot use PC as source register");
    }
    value = loadGuestGPR(regSrc, context);
  }
  else
  {
    // immediate case
    u32int immediate = instr & 0x00000FFF;
    value = armExpandImm12(immediate);
  }
    
  u32int oldValue = 0;
  if (cpsrOrSpsr == 0)
  {
    // CPSR!
    oldValue = context->CPSR;
  }
  else
  {
    // SPSR! which?... depends what mode we are in...
    switch (context->CPSR & CPSR_MODE_FIELD)
    {
      case CPSR_MODE_FIQ:
        oldValue = context->SPSR_FIQ;
        break;
      case CPSR_MODE_IRQ:
        oldValue = context->SPSR_IRQ;
        break;
      case CPSR_MODE_SVC:
        oldValue = context->SPSR_SVC;
        break;
      case CPSR_MODE_ABORT:
        oldValue = context->SPSR_ABT;
        break;
      case CPSR_MODE_UNDEF:
        oldValue = context->SPSR_UND;
        break;
      case CPSR_MODE_USER:
      case CPSR_MODE_SYSTEM:
      default: 
        DIE_NOW(context, "MSR: invalid SPSR write for current guest mode.");
    } 
  }

  // [3:0] field mask:
  // - bit 0: set control field (mode bits/interrupt bits)
  // - bit 1: set extension field (??? [15:8] of cpsr)
  // - bit 2: set status field (??? [23:16] of cpsr)
  // - bit 3: set condition flags of cpsr

  // control field [7-0] set.
  if ( ((fieldMsk & 0x1) == 0x1) && (guestInPrivMode(context)) )
  {
    // check for thumb toggle!
    if ((oldValue & CPSR_THUMB_BIT) != (value & CPSR_THUMB_BIT))
    {
          DIE_NOW(context, "MSR toggle THUMB bit.");
    }
    // separate the field we're gonna update from new value
    u32int appliedValue = (value & 0x000000FF);
    // clear old fields!
    oldValue &= 0xFFFFFF00;
    // update old value...
    oldValue |= appliedValue;
  }
  if ( ((fieldMsk & 0x2) == 0x2) && (guestInPrivMode(context)) )
  {
    // extension field: async abt, endianness, IT[7:2]
    // check for endiannes toggle!
    if ((oldValue & CPSR_ENDIANNESS) != (value & CPSR_ENDIANNESS))
    {
      DIE_NOW(context, "MSR toggle endianess bit.");
    } 
    // separate the field we're gonna update from new value 
    u32int appliedValue = (value & 0x0000FF00);
    // clear old fields!
    oldValue &= 0xFFFF00FF;
    // update old value...
    oldValue |= appliedValue;
  }
  if ( ((fieldMsk & 0x4) == 0x4) && (guestInPrivMode(context)) )
  {
    // status field: reserved and GE[3:0]
    // separate the field we're gonna update from new value 
    u32int appliedValue = (value & 0x00FF0000);
    // clear old fields!
    oldValue &= 0xFF00FFFF;
    // update old value...
    oldValue |= appliedValue;
  }
  if ((fieldMsk & 0x8) == 0x8)
  {
    // condition flags, q, it, J. Dont need to be priv to change those thus no check
    // separate the field we're gonna update from new value 
    u32int appliedValue = (value & 0xFF000000);
    // clear old fields!
    oldValue &= 0x00FFFFFF;
    // update old value...
    oldValue |= appliedValue;
  }

#ifdef ARM_INSTR_TRACE
  printf("MSR instr %08x @ %08x\n", instr, context->R15);
#endif
  // got the final value to write in u32int oldValue. where do we write it thou..?
  if (cpsrOrSpsr == 0)
  {
    // CPSR!
    context->CPSR = oldValue;
  }
  else
  {
    // SPSR! which?... depends what mode we are in...
    switch (context->CPSR & CPSR_MODE_FIELD)
    {
      case CPSR_MODE_FIQ:
        context->SPSR_FIQ = oldValue;
        break;
      case CPSR_MODE_IRQ:
        context->SPSR_IRQ = oldValue;
        break;
      case CPSR_MODE_SVC:
        context->SPSR_SVC = oldValue;
        break;
      case CPSR_MODE_ABORT:
        context->SPSR_ABT = oldValue;
        break;
      case CPSR_MODE_UNDEF:
        context->SPSR_UND = oldValue;
        break;
      case CPSR_MODE_USER:
      case CPSR_MODE_SYSTEM:
      default: 
        DIE_NOW(context, "MSR: invalid SPSR write for current guest mode.");
    } 
  }
  nextPC = context->R15 + 4;
  return nextPC;
}

u32int mrsInstruction(GCONTXT * context)
{
  u32int instr = context->endOfBlockInstr;
  int regSrc   =  instr & 0x00400000;
  int regDest  = (instr & 0x0000F000) >> 12;

  int instrCC = 0;
  u32int value = 0;
  u32int nextPC = 0;

#ifdef ARM_INSTR_TRACE
  printf("MRS instr %08x @ %08x\n", instr, context->R15);
#endif

  if (regDest == 0xF)
  {
    invalidInstruction(instr, "MRS cannot use PC as destination");
  }

  instrCC = (instr >> 28) & 0xF;
  u32int cpsrCC = (context->CPSR >> 28) & 0xF;
  bool conditionMet = evalCC(instrCC, cpsrCC);
  if (conditionMet)
  {
    if (regSrc == 0)
    {
      // CPSR case
      value = context->CPSR;
    }
    else
    {
      // SPSR case
      int guestMode = (context->CPSR) & CPSR_MODE_FIELD;
      switch(guestMode)
      {
        case CPSR_MODE_FIQ:
          value = context->SPSR_FIQ;
          break;
        case CPSR_MODE_IRQ:
          value = context->SPSR_IRQ;
          break;
        case CPSR_MODE_SVC:
          value = context->SPSR_SVC;
          break;
        case CPSR_MODE_ABORT:
          value = context->SPSR_ABT;
          break;
        case CPSR_MODE_UNDEF:
          value = context->SPSR_UND;
          break;
        case CPSR_MODE_USER:
        case CPSR_MODE_SYSTEM:
        default:
          invalidInstruction(instr, "MRS cannot request spsr in user/system mode");
      } // switch ends
    } // spsr case ends
    storeGuestGPR(regDest, value, context);
  } // condition met ends

  nextPC = context->R15 + 4;
  return nextPC;
}

u32int bInstruction(GCONTXT * context)
{
  u16int halfinstr = context->endOfBlockHalfInstr;
  u32int instr = context->endOfBlockInstr;
  u32int instrCC = 0;
  u32int sign = 0;
  u32int link;
  int target = 0;
  u32int nextPC = 0;
  
#ifdef ARM_INSTR_TRACE
  printf("Branch instr %08x @ %08x\n", instr, context->R15);
#endif
  // Are we on Thumb?
  if (context->CPSR & T_BIT)
  {
    /* Reconstruct instruction */
	if(!halfinstr) // !0 -> half instruction from previous word
	{
		instr=(halfinstr<<16)|(instr & 0x0000FFFF);
	}
	// B has 2 different encodings in Thumb-2. Find out which one is
	// WHAT A MESS! -> ARM-A manual : page 344
	sign = ( (instr & 0x04000000 ) >> 26 );
	u8int i1 = ( ~ ( ( (instr & 0x00002000) >> 13 ) ^ sign ) ) & 0x1;  // NOT ( I1 EOR sign )
	u8int i2 = ( ~ ( ( (instr & 0x00000800) >> 11 ) ^ sign ) ) & 0x1;  // NOT ( I2 EOR sign )
	target = (sign<<24)|(i1<<23)|(i2<<22)|(((instr & 0x03FF0000)>>16)<<11)|(instr & 0x000007FF);

	if((instr & 0x00008000) == 0)
	{
	
		instrCC = (0x03C00000 & instr) >> 22;
	}
	link = 0x00005000 & instr;
	if (link == 0x00005000 || link == 0x00004000)
	{
		link = 1 ;
	}
	else
	{
		link = 0;
	}
  }
  // ARM Mode
  else
  {
  	instr = context->endOfBlockInstr;
  	instrCC = (0xF0000000 & instr) >> 28;
  	sign = 0x00800000 & instr;
  	link = 0x0F000000 & instr;
  	target  = 0x00FFFFFF & instr;
  	if (link == 0x0B000000)
	{
    	link = 1;
  	}
  	else
  	{
    	link = 0;
    }
  }

  // sign extend 24 bit imm to 32 bit offset
  if (sign != 0)
  {
    // target negative!
    target |= 0xFF000000;
  }
  if(context->CPSR & T_BIT)
  {
  	target = target << 1;
  }
  else
  {
  	target = target << 2;
  }

   /* eval condition flags only for Thumb-2 first encoding or ARM encoding*/
  if( (context->CPSR & T_BIT && ( (instr & 0x00008000) == 0 ) ) || !(context->CPSR & T_BIT) )
  {
  	u32int cpsrCC = (context->CPSR >> 28) & 0xF;
  	bool conditionMet = evalCC(instrCC, cpsrCC);
  	if (conditionMet)
  	{
   	 // condition met
   	 u32int currPC = context->R15;
   	 if(context->CPSR & T_BIT)
	 {
	 	currPC += 4;
	 }
	 else
	 {
	 	currPC += 8;
	 }
   	 nextPC = currPC + target;
   	 if (link)
  	 {
		if(context->CPSR & T_BIT )
		{
			storeGuestGPR(14, context->R15+2, context);
		}
		else
		{
			storeGuestGPR(14, context->R15+4, context);
		}
  	 }
	}
    else
    {
    	// condition not met!
	    if(context->CPSR & T_BIT)
		{
			nextPC = context->R15 + 2;
		}
		else
		{
			nextPC = context->R15 + 4;
		}
    }
  }
  // Thumb-2 second encoding were no CC flags exist. Store to R14 anyway
  else
  {
   	 storeGuestGPR(14, context->R15+2, context);
     u32int currPC = context->R15;
   	 currPC += 4;
   	 nextPC = currPC + target;
  }
  return nextPC;
}

u32int svcInstruction(GCONTXT * context)
{
  u32int nextPC = 0;
#ifdef ARM_INSTR_TRACE
  u32int instr = context->endOfBlockInstr;
  printf("SVC instr %08x @ %08x\n", instr, context->R15);
#endif
	// save cpsr
	context->SPSR_SVC =context->CPSR;
	// set guest to svc
	context->CPSR = (context->CPSR & 0xffffffe0) | CPSR_MODE_SVC;
	// now disable the interrupts
	context->CPSR |= CPSR_IRQ_DIS; 
	// preserve program counter
  	storeGuestGPR(14, context->R15+4, context);

	// set pc to swi handler
  	nextPC = context->guestSwiHandler;

	return nextPC;
}

u32int undefinedInstruction(GCONTXT * context)
{
  invalidInstruction(context->endOfBlockInstr, "undefined instruction");
  return 0;
}
