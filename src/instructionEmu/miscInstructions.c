#include "miscInstructions.h"
#include "commonInstrFunctions.h"

u32int nopInstruction(GCONTXT * context)
{
  serial_putstring("ERROR: NOP instr ");
  serial_putint(context->endOfBlockInstr);
  serial_putstring(" @ ");
  serial_putint(context->R15);
  serial_putstring(" should not have trapped!");
  serial_newline();
  dumpGuestContext(context);
  return 0;
}

u32int bxInstruction(GCONTXT * context)
{
  dumpGuestContext(context);
  serial_ERROR("BX unfinished\n");
  return 0;
}

u32int mulInstruction(GCONTXT * context)
{
  dumpGuestContext(context);
  serial_ERROR("MUL unfinished\n");
  return 0;
}

u32int mlaInstruction(GCONTXT * context)
{
  dumpGuestContext(context);
  serial_ERROR("MLA unfinished\n");
  return 0;
}

u32int swpInstruction(GCONTXT * context)
{
  dumpGuestContext(context);
  serial_ERROR("SWP unfinished\n");
  return 0;
}

u32int sumlalInstruction(GCONTXT * context)
{
  dumpGuestContext(context);
  serial_ERROR("SUMLAL unfinished\n");
  return 0;
}

u32int sumullInstruction(GCONTXT * context)
{
  dumpGuestContext(context);
  serial_ERROR("SUMULL unfinished\n");
  return 0;
}

u32int pliInstruction(GCONTXT * context)
{
#ifdef ARM_INSTR_TRACE
  serial_putstring("Warning: PLI!");
  serial_newline();
#endif
  return context->R15+4;
}

u32int dbgInstruction(GCONTXT * context)
{
  dumpGuestContext(context);
  serial_ERROR("DBG unfinished\n");
  return 0;
}

u32int dmbInstruction(GCONTXT * context)
{
  dumpGuestContext(context);
  serial_ERROR("DBM unfinished\n");
  return 0;
}

u32int dsbInstruction(GCONTXT * context)
{
  dumpGuestContext(context);
  serial_ERROR("DSB unfinished\n");
  return 0;
}

u32int isbInstruction(GCONTXT * context)
{
  dumpGuestContext(context);
  serial_ERROR("ISB unfinished\n");
  return 0;
}

u32int bfcInstruction(GCONTXT * context)
{
  dumpGuestContext(context);
  serial_ERROR("BFC unfinished\n");
  return 0;
}

u32int bfiInstruction(GCONTXT * context)
{
  dumpGuestContext(context);
  serial_ERROR("BFI unfinished\n");
  return 0;
}

u32int mlsInstruction(GCONTXT * context)
{
  dumpGuestContext(context);
  serial_ERROR("MLS unfinished\n");
  return 0;
}

u32int strhtInstruction(GCONTXT * context)
{
  dumpGuestContext(context);
  serial_ERROR("STRHT unfinished\n");
  return 0;
}

u32int ldrhtInstruction(GCONTXT * context)
{
  dumpGuestContext(context);
  serial_ERROR("LDRHT unfinished\n");
  return 0;
}

u32int movwInstruction(GCONTXT * context)
{
  dumpGuestContext(context);
  serial_ERROR("MOVW unfinished\n");
  return 0;
}

u32int movtInstruction(GCONTXT * context)
{
  dumpGuestContext(context);
  serial_ERROR("MOVT unfinished\n");
  return 0;
}

u32int rbitInstruction(GCONTXT * context)
{
  dumpGuestContext(context);
  serial_ERROR("RBIT unfinished\n");
  return 0;
}

u32int usbfxInstruction(GCONTXT * context)
{
  dumpGuestContext(context);
  serial_ERROR("USBFX unfinished\n");
  return 0;
}

u32int smcInstruction(GCONTXT * context)
{
  dumpGuestContext(context);
  serial_ERROR("SMC unfinished\n");
  return 0;
}

u32int clrexInstruction(GCONTXT * context)
{
  dumpGuestContext(context);
  serial_ERROR("CLREX unfinished\n");
  return 0;
}

u32int yieldInstruction(GCONTXT * context)
{
  dumpGuestContext(context);
  serial_ERROR("YIELD unfinished\n");
  return 0;
}

u32int wfeInstruction(GCONTXT * context)
{
  dumpGuestContext(context);
  serial_ERROR("WFE unfinished\n");
  return 0;
}

u32int wfiInstruction(GCONTXT * context)
{
  dumpGuestContext(context);
  serial_ERROR("WFI unfinished\n");
  return 0;
}

u32int sevInstruction(GCONTXT * context)
{
  dumpGuestContext(context);
  serial_ERROR("SEV unfinished\n");
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
  u32int instrCC    = 0xF0000000 & instr;
  u32int imod       = (instr & 0x000C0000) >> 18;
  u32int changeMode = (instr & 0x00020000) >> 17;
  u32int affectA    = (instr & 0x00000080) >>  8;
  u32int affectI    = (instr & 0x00000040) >>  7;
  u32int affectF    = (instr & 0x00000020) >>  6;
  u32int newMode    =  instr & 0x0000001F;
#ifdef ARM_INSTR_TRACE
  serial_putstring("CPS instr ");
  serial_putint(instr);
  serial_putstring(" @ ");
  serial_putint(context->R15);
  serial_newline();
#endif

  /* eval condition flags */
  instrCC = instrCC >> 28;
  u32int cpsrCC = (context->CPSR >> 28) & 0xF;
  bool conditionMet = evalCC(instrCC, cpsrCC);
  if ( ((imod == 0) && (changeMode == 0)) || (imod == 1) )
  {
    serial_putint(instr);
    serial_ERROR(": CPS unpredictable case\n");
  }
  if (conditionMet)
  {
    if (guestInPrivMode(context))
    {
      u32int oldCpsr = context->CPSR;
      if (imod == 0x2) // enable
      {
        if (affectA != 0)
        {
          if ((oldCpsr & CPSR_AAB_BIT) != 0)
          {
            serial_ERROR("Guest enabling async aborts globally!");
          }
          oldCpsr &= ~CPSR_AAB_BIT;
        }
        if (affectI)
        {
          if ( (oldCpsr & CPSR_IRQ_BIT) != 0)
          {
            serial_ERROR("Guest enabling irqs globally!");
          } 
          oldCpsr &= ~CPSR_IRQ_BIT;
        }
        if (affectF)
        {
          if ( (oldCpsr & CPSR_FIQ_BIT) != 0)
          {
            serial_ERROR("Guest enabling fiqs globally!");
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
            serial_ERROR("Guest disabling async aborts globally!");
          }
          oldCpsr |= CPSR_AAB_BIT;
        }
        if (affectI)
        {
          if ( (oldCpsr & CPSR_IRQ_BIT) == 0)
          {
            serial_ERROR("Guest disabling irqs globally!");
          } 
          oldCpsr |= CPSR_IRQ_BIT;
        }
        if (affectF)
        {
          if ( (oldCpsr & CPSR_FIQ_BIT) == 0)
          {
            serial_ERROR("Guest disabling fiqs globally!");
          } 
          oldCpsr |= CPSR_FIQ_BIT;
        }
      }
      else
      {
        serial_ERROR("CPS invalid IMOD\n");
      }
      // ARE we switching modes?
      if (changeMode)
      {
        oldCpsr &= ~CPSR_MODE_FIELD;
        oldCpsr |= newMode; 
      }
      context->CPSR = oldCpsr;
    }
  }

  return context->R15+4;
}

u32int pkhbtInstruction(GCONTXT * context)
{
  dumpGuestContext(context);
  serial_ERROR("PKHBT unfinished\n");
  return 0;
}

u32int pkhtbInstruction(GCONTXT * context)
{
  dumpGuestContext(context);
  serial_ERROR("PKHTB unfinished\n");
  return 0;
}

u32int qadd16Instruction(GCONTXT * context)
{
  dumpGuestContext(context);
  serial_ERROR("QADD16 unfinished\n");
  return 0;
}

u32int qadd8Instruction(GCONTXT * context)
{
  dumpGuestContext(context);
  serial_ERROR("QADD8 unfinished\n");
  return 0;
}

u32int qaddsubxInstruction(GCONTXT * context)
{
  dumpGuestContext(context);
  serial_ERROR("QADDSUBX unfinished\n");
  return 0;
}

u32int qsub16Instruction(GCONTXT * context)
{
  dumpGuestContext(context);
  serial_ERROR("QSUB16 unfinished\n");
  return 0;
}

u32int qsub8Instruction(GCONTXT * context)
{
  dumpGuestContext(context);
  serial_ERROR("QSUB8 unfinished\n");
  return 0;
}

u32int qsubaddxInstruction(GCONTXT * context)
{
  dumpGuestContext(context);
  serial_ERROR("QSUBADDX unfinished\n");
  return 0;
}

u32int sadd16Instruction(GCONTXT * context)
{
  dumpGuestContext(context);
  serial_ERROR("SADD16 unfinished\n");
  return 0;
}

u32int sadd8Instruction(GCONTXT * context)
{
  dumpGuestContext(context);
  serial_ERROR("SADD8 unfinished\n");
  return 0;
}

u32int saddsubxInstruction(GCONTXT * context)
{
  dumpGuestContext(context);
  serial_ERROR("SADDADDX unfinished\n");
  return 0;
}

u32int shadd16Instruction(GCONTXT * context)
{
  dumpGuestContext(context);
  serial_ERROR("SHADD16 unfinished\n");
  return 0;
}

u32int shadd8Instruction(GCONTXT * context)
{
  dumpGuestContext(context);
  serial_ERROR("SHADD8 unfinished\n");
  return 0;
}

u32int shaddsubxInstruction(GCONTXT * context)
{
  dumpGuestContext(context);
  serial_ERROR("SHADDSUBX unfinished\n");
  return 0;
}

u32int shsub16Instruction(GCONTXT * context)
{
  dumpGuestContext(context);
  serial_ERROR("SHSUB16 unfinished\n");
  return 0;
}

u32int shsub8Instruction(GCONTXT * context)
{
  dumpGuestContext(context);
  serial_ERROR("SHSUB8 unfinished\n");
  return 0;
}

u32int shsubaddxInstruction(GCONTXT * context)
{
  dumpGuestContext(context);
  serial_ERROR("SHSUBADDX unfinished\n");
  return 0;
}

u32int ssub16Instruction(GCONTXT * context)
{
  dumpGuestContext(context);
  serial_ERROR("SSUB16 unfinished\n");
  return 0;
}

u32int ssub8Instruction(GCONTXT * context)
{
  dumpGuestContext(context);
  serial_ERROR("SSUB8 unfinished\n");
  return 0;
}

u32int ssubaddxInstruction(GCONTXT * context)
{
  dumpGuestContext(context);
  serial_ERROR("SSUBADDX unfinished\n");
  return 0;
}

u32int uadd16Instruction(GCONTXT * context)
{
  dumpGuestContext(context);
  serial_ERROR("UADD16 unfinished\n");
  return 0;
}

u32int uadd8Instruction(GCONTXT * context)
{
  dumpGuestContext(context);
  serial_ERROR("UADD8 unfinished\n");
  return 0;
}

u32int uaddsubxInstruction(GCONTXT * context)
{
  dumpGuestContext(context);
  serial_ERROR("UADDSUBX unfinished\n");
  return 0;
}

u32int uhadd16Instruction(GCONTXT * context)
{
  dumpGuestContext(context);
  serial_ERROR("UHADD16 unfinished\n");
  return 0;
}

u32int uhadd8Instruction(GCONTXT * context)
{
  dumpGuestContext(context);
  serial_ERROR("UHADD8 unfinished\n");
  return 0;
}

u32int uhaddsubxInstruction(GCONTXT * context)
{
  dumpGuestContext(context);
  serial_ERROR("UHADDSUBX unfinished\n");
  return 0;
}

u32int uhsub16Instruction(GCONTXT * context)
{
  dumpGuestContext(context);
  serial_ERROR("UHSUB16 unfinished\n");
  return 0;
}

u32int uhsub8Instruction(GCONTXT * context)
{
  dumpGuestContext(context);
  serial_ERROR("UHSUB8 unfinished\n");
  return 0;
}

u32int uhsubaddxInstruction(GCONTXT * context)
{
  dumpGuestContext(context);
  serial_ERROR("UHSUBADDX unfinished\n");
  return 0;
}

u32int uqadd16Instruction(GCONTXT * context)
{
  dumpGuestContext(context);
  serial_ERROR("UQADD16 unfinished\n");
  return 0;
}

u32int uqadd8Instruction(GCONTXT * context)
{
  dumpGuestContext(context);
  serial_ERROR("UQADD8 unfinished\n");
  return 0;
}

u32int uqaddsubxInstruction(GCONTXT * context)
{
  dumpGuestContext(context);
  serial_ERROR("UQADDSUBX unfinished\n");
  return 0;
}

u32int uqsub16Instruction(GCONTXT * context)
{
  dumpGuestContext(context);
  serial_ERROR("UQSUB16 unfinished\n");
  return 0;
}
u32int uqsub8Instruction(GCONTXT * context)
{
  dumpGuestContext(context);
  serial_ERROR("UQSUB8 unfinished\n");
  return 0;
}

u32int uqsubaddxInstruction(GCONTXT * context)
{
  dumpGuestContext(context);
  serial_ERROR("UQSUBADDX unfinished\n");
  return 0;
}

u32int usub16Instruction(GCONTXT * context)
{
  dumpGuestContext(context);
  serial_ERROR("USUB16 unfinished\n");
  return 0;
}

u32int usub8Instruction(GCONTXT * context)
{
  dumpGuestContext(context);
  serial_ERROR("USUB8 unfinished\n");
  return 0;
}

u32int usubaddxInstruction(GCONTXT * context)
{
  dumpGuestContext(context);
  serial_ERROR("USUBADDX unfinished\n");
  return 0;
}

u32int revInstruction(GCONTXT * context)
{
  dumpGuestContext(context);
  serial_ERROR("REV unfinished\n");
  return 0;
}

u32int rev16Instruction(GCONTXT * context)
{
  dumpGuestContext(context);
  serial_ERROR("REV16 unfinished\n");
  return 0;
}

u32int revshInstruction(GCONTXT * context)
{
  dumpGuestContext(context);
  serial_ERROR("REVSH unfinished\n");
  return 0;
}

u32int rfeInstruction(GCONTXT * context)
{
  dumpGuestContext(context);
  serial_ERROR("RFE unfinished\n");
  return 0;
}

u32int sxthInstruction(GCONTXT * context)
{
  dumpGuestContext(context);
  serial_ERROR("SXTH unfinished\n");
  return 0;
}

u32int sxtb16Instruction(GCONTXT * context)
{
  dumpGuestContext(context);
  serial_ERROR("SXTB16 unfinished\n");
  return 0;
}

u32int sxtbInstruction(GCONTXT * context)
{
  dumpGuestContext(context);
  serial_ERROR("SXTB unfinished\n");
  return 0;
}

u32int uxthInstruction(GCONTXT * context)
{
  dumpGuestContext(context);
  serial_ERROR("UXTHunfinished\n");
  return 0;
}

u32int uxtb16Instruction(GCONTXT * context)
{
  dumpGuestContext(context);
  serial_ERROR("UXTB16 unfinished\n");
  return 0;
}

u32int uxtbInstruction(GCONTXT * context)
{
  dumpGuestContext(context);
  serial_ERROR("UXTB unfinished\n");
  return 0;
}

u32int sxtahInstruction(GCONTXT * context)
{
  dumpGuestContext(context);
  serial_ERROR("SXTAH unfinished\n");
  return 0;
}

u32int sxtab16Instruction(GCONTXT * context)
{
  dumpGuestContext(context);
  serial_ERROR("SXTAB16 unfinished\n");
  return 0;
}

u32int sxtabInstruction(GCONTXT * context)
{
  dumpGuestContext(context);
  serial_ERROR("SXTAB unfinished\n");
  return 0;
}

u32int uxtahInstruction(GCONTXT * context)
{
  dumpGuestContext(context);
  serial_ERROR("UXTAH unfinished\n");
  return 0;
}

u32int uxtab16Instruction(GCONTXT * context)
{
  dumpGuestContext(context);
  serial_ERROR("UXTAB16 unfinished\n");
  return 0;
}

u32int uxtabInstruction(GCONTXT * context)
{
  dumpGuestContext(context);
  serial_ERROR("UXTAB unfinished\n");
  return 0;
}

u32int selInstruction(GCONTXT * context)
{
  dumpGuestContext(context);
  serial_ERROR("SEL unfinished\n");
  return 0;
}

u32int setendInstruction(GCONTXT * context)
{
  dumpGuestContext(context);
  serial_ERROR("SETEND unfinished\n");
  return 0;
}

u32int smuadInstruction(GCONTXT * context)
{
  dumpGuestContext(context);
  serial_ERROR("SMUAD unfinished\n");
  return 0;
}

u32int smusdInstruction(GCONTXT * context)
{
  dumpGuestContext(context);
  serial_ERROR("SMUSD unfinished\n");
  return 0;
}

u32int smladInstruction(GCONTXT * context)
{
  dumpGuestContext(context);
  serial_ERROR("SMLAD unfinished\n");
  return 0;
}

u32int smlaldInstruction(GCONTXT * context)
{
  dumpGuestContext(context);
  serial_ERROR("SMLALD unfinished\n");
  return 0;
}

u32int smlsdInstruction(GCONTXT * context)
{
  dumpGuestContext(context);
  serial_ERROR("SMLSD unfinished\n");
  return 0;
}

u32int smlsldInstruction(GCONTXT * context)
{
  dumpGuestContext(context);
  serial_ERROR("SMLSLD unfinished\n");
  return 0;
}

u32int smmulInstruction(GCONTXT * context)
{
  dumpGuestContext(context);
  serial_ERROR("SMMUL unfinished\n");
  return 0;
}

u32int smmlaInstruction(GCONTXT * context)
{
  dumpGuestContext(context);
  serial_ERROR("SMMLA unfinished\n");
  return 0;
}

u32int smmlsInstruction(GCONTXT * context)
{
  dumpGuestContext(context);
  serial_ERROR("SMMLS unfinished\n");
  return 0;
}

u32int srsInstruction(GCONTXT * context)
{
  dumpGuestContext(context);
  serial_ERROR("SRS unfinished\n");
  return 0;
}

u32int ssatInstruction(GCONTXT * context)
{
  dumpGuestContext(context);
  serial_ERROR("SSAT unfinished\n");
  return 0;
}

u32int ssat16Instruction(GCONTXT * context)
{
  dumpGuestContext(context);
  serial_ERROR("ssat16 unfinished\n");
  return 0;
}

u32int umaalInstruction(GCONTXT * context)
{
  dumpGuestContext(context);
  serial_ERROR("UMAAL unfinished\n");
  return 0;
}

u32int usad8Instruction(GCONTXT * context)
{
  dumpGuestContext(context);
  serial_ERROR("USAD8 unfinished\n");
  return 0;
}

u32int usada8Instruction(GCONTXT * context)
{
  dumpGuestContext(context);
  serial_ERROR("USADA unfinished\n");
  return 0;
}

u32int usatInstruction(GCONTXT * context)
{
  dumpGuestContext(context);
  serial_ERROR("USAT unfinished\n");
  return 0;
}

u32int usat16Instruction(GCONTXT * context)
{
  dumpGuestContext(context);
  serial_ERROR("USAT16 unfinished\n");
  return 0;
}

u32int bxjInstruction(GCONTXT * context)
{
  dumpGuestContext(context);
  serial_ERROR("BXJ unfinished\n");
  return 0;
}

u32int bkptInstruction(GCONTXT * context)
{
  dumpGuestContext(context);
  serial_ERROR("BKPT unfinished\n");
  return 0;
}

u32int blxInstruction(GCONTXT * context)
{
  u32int instr = context->endOfBlockInstr;
  if ((instr & 0xfe000000) == 0xfa000000)
  {
    dumpGuestContext(context);
    serial_ERROR("BLX #imm24 switching to Thumb. Unimplemented.\n");
  }

  u32int nextPC = 0;
  
  u32int instrCC = (instr >> 28) & 0xF;
  u32int cpsrCC  = (context->CPSR >> 28) & 0xF;
  if (!evalCC(instrCC, cpsrCC))
  {
    nextPC = context->R15 + 4;
    return nextPC;
  }

  u32int regDest = (instr & 0x0000000F); // holds dest addr and mode bit
  u32int value = loadGuestGPR(regDest, context);

  u32int thumbMode = value & 0x1;
  if (thumbMode)
  {
    dumpGuestContext(context);
    serial_ERROR("BLX Rm switching to Thumb. Unimplemented.\n");
  }

  // link register
  storeGuestGPR(14, context->R15+4, context);

  nextPC = value & 0xFFFFFFFE;
  return nextPC;
}

u32int clzInstruction(GCONTXT * context)
{
  dumpGuestContext(context);
  serial_ERROR("CLZ unfinished\n");
  return 0;
}

u32int pldInstruction(GCONTXT * context)
{
#ifdef ARM_INSTR_TRACE
  serial_putstring("Warning: PLD!");
  serial_newline();
#endif
  return context->R15+4;
}

u32int smlabbInstruction(GCONTXT * context)
{
  dumpGuestContext(context);
  serial_ERROR("SMLABB unfinished\n");
  return 0;
}

u32int smlatbInstruction(GCONTXT * context)
{
  dumpGuestContext(context);
  serial_ERROR("SMLATB unfinished\n");
  return 0;
}

u32int smlabtInstruction(GCONTXT * context)
{
  dumpGuestContext(context);
  serial_ERROR("SMLABT unfinished\n");
  return 0;
}

u32int smlattInstruction(GCONTXT * context)
{
  dumpGuestContext(context);
  serial_ERROR("SMLATT unfinished\n");
  return 0;
}

u32int smlawbInstruction(GCONTXT * context)
{
  dumpGuestContext(context);
  serial_ERROR("SMLAWB unfinished\n");
  return 0;
}

u32int smlawtInstruction(GCONTXT * context)
{
  dumpGuestContext(context);
  serial_ERROR("SMLAWT unfinished\n");
  return 0;
}

u32int smlalbbInstruction(GCONTXT * context)
{
  dumpGuestContext(context);
  serial_ERROR("SMLALBB unfinished\n");
  return 0;
}

u32int smlaltbInstruction(GCONTXT * context)
{
  dumpGuestContext(context);
  serial_ERROR("SMLALTB unfinished\n");
  return 0;
}

u32int smlalbtInstruction(GCONTXT * context)
{
  dumpGuestContext(context);
  serial_ERROR("SMLALBT unfinished\n");
  return 0;
}

u32int smlalttInstruction(GCONTXT * context)
{
  dumpGuestContext(context);
  serial_ERROR("SMLALTT unfinished\n");
  return 0;
}

u32int smulbbInstruction(GCONTXT * context)
{
  dumpGuestContext(context);
  serial_ERROR("SMULBB unfinished\n");
  return 0;
}

u32int smultbInstruction(GCONTXT * context)
{
  dumpGuestContext(context);
  serial_ERROR("SMULTB unfinished\n");
  return 0;
}

u32int smulbtInstruction(GCONTXT * context)
{
  dumpGuestContext(context);
  serial_ERROR("SMULBT unfinished\n");
  return 0;
}

u32int smulttInstruction(GCONTXT * context)
{
  dumpGuestContext(context);
  serial_ERROR("SMULTT unfinished\n");
  return 0;
}

u32int smulwbInstruction(GCONTXT * context)
{
  dumpGuestContext(context);
  serial_ERROR("SMULWD unfinished\n");
  return 0;
}

u32int smulwtInstruction(GCONTXT * context)
{
  dumpGuestContext(context);
  serial_ERROR("SMULWT unfinished\n");
  return 0;
}

u32int qaddInstruction(GCONTXT * context)
{
  dumpGuestContext(context);
  serial_ERROR("QADD unfinished\n");
  return 0;
}

u32int qdaddInstruction(GCONTXT * context)
{
  dumpGuestContext(context);
  serial_ERROR("QDADD unfinished\n");
  return 0;
}

u32int qsubInstruction(GCONTXT * context)
{
  dumpGuestContext(context);
  serial_ERROR("QSUB unfinished\n");
  return 0;
}

u32int qdsubInstruction(GCONTXT * context)
{
  dumpGuestContext(context);
  serial_ERROR("QDSUB unfinished\n");
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
      invalid_instruction(instr, "MSR cannot use PC as source register");
    }
    value = loadGuestGPR(regSrc, context);
  }
  else
  {
    // immediate case
    u32int immediate = instr & 0x00000FFF;
    value = armExpandImm12(immediate);
  }
    
  // [3:0] field mask:
  // - bit 0: set control field (mode bits/interrupt bits)
  // - bit 1: set extension field (??? [15:8] of cpsr)
  // - bit 2: set status field (??? [23:16] of cpsr)
  // - bit 3: set condition flags of cpsr
  u32int oldValue = 0;
  if (cpsrOrSpsr == 0)
  {
    // CPSR!
    oldValue = context->CPSR;
  }
  else
  {
    // SPSR! which?... depends what mode we are in...
    error_function("MSR writing SPSR unimplemented!", context);
  }
  if ( ((fieldMsk & 0x1) == 0x1) && (guestInPrivMode(context)) )
  {
    // check for fiq toggle!
    if ((oldValue & CPSR_FIQ_BIT) != (value & CPSR_FIQ_BIT))
    {
      error_function("MSR toggle FIQ bit.", context);
    } 
    // check for fiq toggle!
    if ((oldValue & CPSR_IRQ_BIT) != (value & CPSR_IRQ_BIT))
    {
      error_function("MSR toggle IRQ bit.", context);
    } 
    // check for thumb toggle!
    if ((oldValue & CPSR_THUMB_BIT) != (value & CPSR_THUMB_BIT))
    {
      error_function("MSR toggle THUMB bit.", context);
    } 
    // separate the field we're gonna update from new value 
    u32int appliedValue = (value & (CPSR_MODE_FIELD | CPSR_THUMB_BIT | CPSR_FIQ_BIT | CPSR_IRQ_BIT));
    // clear old fields!
    oldValue &= ~(CPSR_MODE_FIELD | CPSR_THUMB_BIT | CPSR_FIQ_BIT | CPSR_IRQ_BIT);
    // update old value...
    oldValue |= appliedValue;
  }
  if ( ((fieldMsk & 0x2) == 0x2) && (guestInPrivMode(context)) )
  {
    error_function("MSR setting reserved fields in CPSR (extention?)", context);
  }
  if ( ((fieldMsk & 0x4) == 0x4) && (guestInPrivMode(context)) )
  {
    error_function("MSR setting reserved fields in CPSR (status field)", context);
  }
  if ((fieldMsk & 0x4) == 0x8)
  {
    // separate the field we're gonna update from new value 
    u32int appliedValue = (value & CPSR_CC_FIELD);
    // clear old fields!
    oldValue &= ~(CPSR_CC_FIELD);
    // update old value...
    oldValue |= appliedValue;
  }

#ifdef ARM_INSTR_TRACE
  serial_putstring("MSR instr ");
  serial_putint(instr);
  serial_putstring(" @ ");
  serial_putint(context->R15);
#endif
  // got the final value to write in u32int oldValue. where do we write it thou..?
  if (cpsrOrSpsr == 0)
  {
#ifdef ARM_INSTR_TRACE
    serial_putstring(" value ");
    serial_putint(context->CPSR);
#endif
    // CPSR!
    context->CPSR = oldValue;
  }
  else
  {
    // SPSR! which?... depends what mode we are in...
    error_function("MSR writing SPSR unimplemented!", context);
  }
#ifdef ARM_INSTR_TRACE
  serial_newline();
#endif
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
  serial_putstring("MRS instr ");
  serial_putint(instr);
  serial_putstring(" @ ");
  serial_putint(context->R15);
  serial_newline();
#endif

  if (regDest == 0xF)
  {
    invalid_instruction(instr, "MRS cannot use PC as destination");
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
          invalid_instruction(instr, "MRS cannot request spsr in user/system mode");
      } // switch ends
    } // spsr case ends
    storeGuestGPR(regDest, value, context);
  } // condition met ends
  nextPC = context->R15 + 4;
  return nextPC;
}

u32int bInstruction(GCONTXT * context)
{
  u32int instr = context->endOfBlockInstr;
  u32int instrCC = 0xF0000000 & instr;
  u32int sign = 0x00800000 & instr;
  u32int link = 0x0F000000 & instr;
  int target  = 0x00FFFFFF & instr;
  u32int nextPC = 0;

#ifdef ARM_INSTR_TRACE
  serial_putstring("Branch instr ");
  serial_putint(instr);
  serial_putstring(" @ ");
  serial_putint(context->R15);
  serial_newline();
#endif

  if (link == 0x0B000000)
  {
    link = 1;
  }
  else
  {
    link = 0;
  }

  // sign extend 24 bit imm to 32 bit offset
  if (sign != 0)
  {
    // target negative!
    target |= 0xFF000000;
  }
  target = target << 2;

  /* eval condition flags */
  instrCC = instrCC >> 28;
  u32int cpsrCC = (context->CPSR >> 28) & 0xF;
  bool conditionMet = evalCC(instrCC, cpsrCC);
  if (conditionMet)
  {
    // condition met
    u32int currPC = context->R15;
    currPC += 8;
    nextPC = currPC + target;
    if (link)
    {
      storeGuestGPR(14, context->R15+4, context);
/*
      // it was branch and link... what context is the guest in?
      u32int guestMode = context->CPSR & CPSR_MODE_FIELD;
      switch(guestMode)
      {
        case CPSR_MODE_USER:
        case CPSR_MODE_SYSTEM:
          context->R14_USR = context->R15+4;
          break;
        case CPSR_MODE_FIQ:
          context->R14_FIQ = context->R15+4;
          break;
        case CPSR_MODE_IRQ:
          context->R14_IRQ = context->R15+4;
          break;
        case CPSR_MODE_SVC:
          context->R14_SVC = context->R15+4;
          break;
        case CPSR_MODE_ABORT:
          context->R14_ABT = context->R15+4;
          break;
        case CPSR_MODE_UNDEF:
          context->R14_UND = context->R15+4;
          break;
        default:
          invalid_instruction(instr, "bInstruction evaluate: invalid CPSR mode!");
      } // switch ends
*/
    }
  }
  else
  {
    // condition not met!
    nextPC = context->R15 + 4;
  }
  return nextPC;
}

u32int svcInstruction(GCONTXT * context)
{
#ifdef ARM_INSTR_TRACE
  u32int instr = context->endOfBlockInstr;
  serial_putstring("SVC instr ");
  serial_putint(instr);
  serial_putstring(" @ ");
  serial_putint(context->R15);
  serial_newline();
#endif
  return 0;
}

u32int undefinedInstruction(GCONTXT * context)
{
  invalid_instruction(context->endOfBlockInstr, "undefined instruction");
  return 0;
}
