#include "common/debug.h"

#include "cpuArch/constants.h"

#include "guestManager/scheduler.h"

#ifdef CONFIG_THUMB2
#include "guestManager/guestExceptions.h"
#endif

#include "instructionEmu/commonInstrFunctions.h"
#include "instructionEmu/miscInstructions.h"

#ifdef CONFIG_THUMB2
#include "instructionEmu/decoder.h"
#endif

#include "vm/omap35xx/intc.h"


u32int nopInstruction(GCONTXT *context, u32int instruction)
{
  printf("ERROR: NOP instr %08x @ %08x should not have trapped!\n",
         instruction, context->R15);
  DIE_NOW(context, "die.");
  return 0;
}



u32int mulInstruction(GCONTXT *context, u32int instruction)
{
  DIE_NOW(context, "MUL unfinished\n");
}

u32int mlaInstruction(GCONTXT *context, u32int instruction)
{
  DIE_NOW(context, "MLA unfinished\n");
}

u32int swpInstruction(GCONTXT *context, u32int instruction)
{
  DIE_NOW(context, "SWP unfinished\n");
  return 0;
}

u32int sumlalInstruction(GCONTXT *context, u32int instruction)
{
  DIE_NOW(context, "SUMLAL unfinished\n");
  return 0;
}

u32int sumullInstruction(GCONTXT *context, u32int instruction)
{
  DIE_NOW(context, "SUMULL unfinished\n");
  return 0;
}

u32int pliInstruction(GCONTXT *context, u32int instruction)
{
#ifdef ARM_INSTR_TRACE
  printf("Warning: PLI!\n");
#endif
  return context->R15+4;
}

u32int dbgInstruction(GCONTXT *context, u32int instruction)
{
  DIE_NOW(context, "DBG unfinished\n");
  return 0;
}

u32int dmbInstruction(GCONTXT *context, u32int instruction)
{
  DIE_NOW(context, "DBM unfinished\n");
  return 0;
}

u32int dsbInstruction(GCONTXT *context, u32int instruction)
{
#ifdef ARM_INSTR_TRACE
  printf("Warning: DSB (ignored)!\n");
#endif
  return context->R15+4;
}

u32int isbInstruction(GCONTXT *context, u32int instruction)
{
#ifdef ARM_INSTR_TRACE
  printf("Warning: ISB (ignored)!\n");
#endif
  return context->R15+4;
}

u32int bfcInstruction(GCONTXT *context, u32int instruction)
{
  DIE_NOW(context, "BFC unfinished\n");
  return 0;
}

u32int bfiInstruction(GCONTXT *context, u32int instruction)
{
  DIE_NOW(context, "BFI unfinished\n");
  return 0;
}

u32int mlsInstruction(GCONTXT *context, u32int instruction)
{
  DIE_NOW(context, "MLS unfinished\n");
  return 0;
}

u32int movwInstruction(GCONTXT *context, u32int instruction)
{
  DIE_NOW(context, "MOVW unfinished\n");
  return 0;
}

u32int movtInstruction(GCONTXT *context, u32int instruction)
{
  DIE_NOW(context, "MOVT unfinished\n");
  return 0;
}

u32int rbitInstruction(GCONTXT *context, u32int instruction)
{
  DIE_NOW(context, "RBIT unfinished\n");
  return 0;
}

u32int usbfxInstruction(GCONTXT *context, u32int instruction)
{
  DIE_NOW(context, "USBFX unfinished\n");
  return 0;
}

u32int smcInstruction(GCONTXT *context, u32int instruction)
{
  DIE_NOW(context, "SMC unfinished\n");
  return 0;
}

u32int clrexInstruction(GCONTXT *context, u32int instruction)
{
#ifdef ARM_INSTR_TRACE
  printf("Warning: CLREX!\n");
#endif
  return context->R15+4;
}

u32int yieldInstruction(GCONTXT *context, u32int instruction)
{
  DIE_NOW(context, "YIELD unfinished\n");
  return 0;
}

u32int wfeInstruction(GCONTXT *context, u32int instruction)
{
  DIE_NOW(context, "WFE unfinished\n");
  return 0;
}

u32int wfiInstruction(GCONTXT *context, u32int instruction)
{
  // stop guest execution...
  guestIdle(context);
  return context->R15+4;
}

u32int sevInstruction(GCONTXT *context, u32int instruction)
{
  DIE_NOW(context, "SEV unfinished\n");
  return 0;
}

u32int cpsInstruction(GCONTXT *context, u32int instr)
{
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

  if ((context->CPSR & PSR_MODE) != PSR_USR_MODE)
  {
    u32int oldCpsr = context->CPSR;
    if (imod == 0x2) // enable
    {
#ifdef ARM_INSTR_TRACE
      printf("IMod: enable case\n");
#endif
      if (affectA != 0)
      {
        if ((oldCpsr & PSR_A_BIT) != 0)
        {
          DIE_NOW(context, "Guest enabling async aborts globally!");
        }
        oldCpsr &= ~PSR_A_BIT;
      }
      if (affectI)
      {
        if ((oldCpsr & PSR_I_BIT) != 0)
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
        oldCpsr &= ~PSR_I_BIT;
      }
      if (affectF)
      {
        if ((oldCpsr & PSR_F_BIT) != 0)
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
        oldCpsr &= ~PSR_F_BIT;
      }
    }
    else if (imod == 3) // disable
    {
      if (affectA)
      {
        if (!(oldCpsr & PSR_A_BIT))
        {
          DIE_NOW(context, "Guest disabling async aborts globally!");
        }
        oldCpsr |= PSR_A_BIT;
      }
      if (affectI)
      {
        if (!(oldCpsr & PSR_I_BIT)) // were enabled, now disabled
        {
          // chech interrupt controller if there is an interrupt pending
          if (context->guestIrqPending)
          {
            /*
             * FIXME: Niels: wtf? why do we need the if?
             */
            context->guestIrqPending = FALSE;
          }
        }
        oldCpsr |= PSR_I_BIT;
      }
      if (affectF)
      {
        if (!(oldCpsr & PSR_F_BIT))
        {
          DIE_NOW(context, "Guest disabling fiqs globally!");
        }
        oldCpsr |= PSR_F_BIT;
      }
    }
    else
    {
      DIE_NOW(context, "CPS invalid IMOD\n");
    }
    // ARE we switching modes?
    if (changeMode)
    {
      oldCpsr &= ~PSR_MODE;
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

  return context->R15 + 4;
}

u32int pkhbtInstruction(GCONTXT *context, u32int instruction)
{
  DIE_NOW(context, "PKHBT unfinished\n");
  return 0;
}

u32int pkhtbInstruction(GCONTXT *context, u32int instruction)
{
  DIE_NOW(context, "PKHTB unfinished\n");
  return 0;
}

u32int qadd16Instruction(GCONTXT *context, u32int instruction)
{
  DIE_NOW(context, "QADD16 unfinished\n");
  return 0;
}

u32int qadd8Instruction(GCONTXT *context, u32int instruction)
{
  DIE_NOW(context, "QADD8 unfinished\n");
  return 0;
}

u32int qaddsubxInstruction(GCONTXT *context, u32int instruction)
{
  DIE_NOW(context, "QADDSUBX unfinished\n");
  return 0;
}

u32int ubxInstruction(GCONTXT *context, u32int instruction)
{
  DIE_NOW(context, "UBXT unfinished\n");
  return 0;
}

u32int qsub16Instruction(GCONTXT *context, u32int instruction)
{
  DIE_NOW(context, "QSUB16 unfinished\n");
  return 0;
}

u32int qsub8Instruction(GCONTXT *context, u32int instruction)
{
  DIE_NOW(context, "QSUB8 unfinished\n");
  return 0;
}

u32int qsubaddxInstruction(GCONTXT *context, u32int instruction)
{
  DIE_NOW(context, "QSUBADDX unfinished\n");
  return 0;
}

u32int sadd16Instruction(GCONTXT *context, u32int instruction)
{
  DIE_NOW(context, "SADD16 unfinished\n");
  return 0;
}

u32int sadd8Instruction(GCONTXT *context, u32int instruction)
{
  DIE_NOW(context, "SADD8 unfinished\n");
  return 0;
}

u32int saddsubxInstruction(GCONTXT *context, u32int instruction)
{
  DIE_NOW(context, "SADDADDX unfinished\n");
  return 0;
}

u32int shadd16Instruction(GCONTXT *context, u32int instruction)
{
  DIE_NOW(context, "SHADD16 unfinished\n");
  return 0;
}

u32int shadd8Instruction(GCONTXT *context, u32int instruction)
{
  DIE_NOW(context, "SHADD8 unfinished\n");
  return 0;
}

u32int shaddsubxInstruction(GCONTXT *context, u32int instruction)
{
  DIE_NOW(context, "SHADDSUBX unfinished\n");
  return 0;
}

u32int shsub16Instruction(GCONTXT *context, u32int instruction)
{
  DIE_NOW(context, "SHSUB16 unfinished\n");
  return 0;
}

u32int shsub8Instruction(GCONTXT *context, u32int instruction)
{
  DIE_NOW(context, "SHSUB8 unfinished\n");
  return 0;
}

u32int shsubaddxInstruction(GCONTXT *context, u32int instruction)
{
  DIE_NOW(context, "SHSUBADDX unfinished\n");
  return 0;
}

u32int ssub16Instruction(GCONTXT *context, u32int instruction)
{
  DIE_NOW(context, "SSUB16 unfinished\n");
  return 0;
}

u32int ssub8Instruction(GCONTXT *context, u32int instruction)
{
  DIE_NOW(context, "SSUB8 unfinished\n");
  return 0;
}

u32int ssubaddxInstruction(GCONTXT *context, u32int instruction)
{
  DIE_NOW(context, "SSUBADDX unfinished\n");
  return 0;
}

u32int uadd16Instruction(GCONTXT *context, u32int instruction)
{
  DIE_NOW(context, "UADD16 unfinished\n");
  return 0;
}

u32int uadd8Instruction(GCONTXT *context, u32int instruction)
{
  DIE_NOW(context, "UADD8 unfinished\n");
  return 0;
}

u32int uaddsubxInstruction(GCONTXT *context, u32int instruction)
{
  DIE_NOW(context, "UADDSUBX unfinished\n");
  return 0;
}

u32int uhadd16Instruction(GCONTXT *context, u32int instruction)
{
  DIE_NOW(context, "UHADD16 unfinished\n");
  return 0;
}

u32int uhadd8Instruction(GCONTXT *context, u32int instruction)
{
  DIE_NOW(context, "UHADD8 unfinished\n");
  return 0;
}

u32int uhaddsubxInstruction(GCONTXT *context, u32int instruction)
{
  DIE_NOW(context, "UHADDSUBX unfinished\n");
  return 0;
}

u32int uhsub16Instruction(GCONTXT *context, u32int instruction)
{
  DIE_NOW(context, "UHSUB16 unfinished\n");
  return 0;
}

u32int uhsub8Instruction(GCONTXT *context, u32int instruction)
{
  DIE_NOW(context, "UHSUB8 unfinished\n");
  return 0;
}

u32int uhsubaddxInstruction(GCONTXT *context, u32int instruction)
{
  DIE_NOW(context, "UHSUBADDX unfinished\n");
  return 0;
}

u32int uqadd16Instruction(GCONTXT *context, u32int instruction)
{
  DIE_NOW(context, "UQADD16 unfinished\n");
  return 0;
}

u32int uqadd8Instruction(GCONTXT *context, u32int instruction)
{
  DIE_NOW(context, "UQADD8 unfinished\n");
  return 0;
}

u32int uqaddsubxInstruction(GCONTXT *context, u32int instruction)
{
  DIE_NOW(context, "UQADDSUBX unfinished\n");
  return 0;
}

u32int uqsub16Instruction(GCONTXT *context, u32int instruction)
{
  DIE_NOW(context, "UQSUB16 unfinished\n");
  return 0;
}
u32int uqsub8Instruction(GCONTXT *context, u32int instruction)
{
  DIE_NOW(context, "UQSUB8 unfinished\n");
  return 0;
}

u32int uqsubaddxInstruction(GCONTXT *context, u32int instruction)
{
  DIE_NOW(context, "UQSUBADDX unfinished\n");
  return 0;
}

u32int usub16Instruction(GCONTXT *context, u32int instruction)
{
  DIE_NOW(context, "USUB16 unfinished\n");
  return 0;
}

u32int usub8Instruction(GCONTXT *context, u32int instruction)
{
  DIE_NOW(context, "USUB8 unfinished\n");
  return 0;
}

u32int usubaddxInstruction(GCONTXT *context, u32int instruction)
{
  DIE_NOW(context, "USUBADDX unfinished\n");
  return 0;
}

u32int revInstruction(GCONTXT *context, u32int instruction)
{
  DIE_NOW(context, "REV unfinished\n");
  return 0;
}

u32int rev16Instruction(GCONTXT *context, u32int instruction)
{
  DIE_NOW(context, "REV16 unfinished\n");
  return 0;
}

u32int revshInstruction(GCONTXT *context, u32int instruction)
{
  DIE_NOW(context, "REVSH unfinished\n");
  return 0;
}

u32int rfeInstruction(GCONTXT *context, u32int instruction)
{
  DIE_NOW(context, "RFE unfinished\n");
  return 0;
}

u32int sxthInstruction(GCONTXT *context, u32int instr)
{
  u32int instrCC = (instr >> 28) & 0xF;
  u32int regSrc = (instr & 0xF);
  u32int regDest = (instr & 0x0000F000) >> 12;
  u32int rotate = (instr & 0x00000C00) >> 10;
  u32int value = 0;
  if (regDest == 15 || regSrc == 15)
  {
    DIE_NOW(0,"Rd/Rm is R15. Unpredictable behaviour\n");
  }
  if (evaluateConditionCode(context, instrCC))
  {
    /* load the least 16bits from the source register */
    value=(loadGuestGPR(regSrc,context) & 0x0000FFFF);
    /* ARM7-A : page 729 */
    switch (rotate)
    {
      case 0:
        value = rorVal(value, SXTH_R0);
        break;
      case 1:
        value = rorVal(value, SXTH_R8);
        break;
      case 2:
        value = rorVal(value, SXTH_R16);
        break;
      case 3:
        value = rorVal(value, SXTH_R24);
        break;
    }
    /* Extend it to 32bit */
    value = value<<16;
    /* Store it */
    storeGuestGPR(regDest,value,context);
  }
  return context->R15 + 4;
}

u32int sxtb16Instruction(GCONTXT *context, u32int instruction)
{
  DIE_NOW(context, "SXTB16 unfinished\n");
  return 0;
}

u32int sxtbInstruction(GCONTXT *context, u32int instruction)
{
  DIE_NOW(context, "SXTB unfinished\n");
  return 0;
}

u32int uxthInstruction(GCONTXT *context, u32int instruction)
{
  DIE_NOW(context, "UXTHunfinished\n");
  return 0;
}

u32int uxtb16Instruction(GCONTXT *context, u32int instruction)
{
  DIE_NOW(context, "UXTB16 unfinished\n");
  return 0;
}

u32int uxtbInstruction(GCONTXT *context, u32int instruction)
{
  DIE_NOW(context, "UXTB unfinished\n");
  return 0;
}

u32int sxtahInstruction(GCONTXT *context, u32int instruction)
{
  DIE_NOW(context, "SXTAH unfinished\n");
  return 0;
}

u32int sxtab16Instruction(GCONTXT *context, u32int instruction)
{
  DIE_NOW(context, "SXTAB16 unfinished\n");
  return 0;
}

u32int sxtabInstruction(GCONTXT *context, u32int instruction)
{
  DIE_NOW(context, "SXTAB unfinished\n");
  return 0;
}

u32int uxtahInstruction(GCONTXT *context, u32int instruction)
{
  DIE_NOW(context, "UXTAH unfinished\n");
  return 0;
}

u32int uxtab16Instruction(GCONTXT *context, u32int instruction)
{
  DIE_NOW(context, "UXTAB16 unfinished\n");
  return 0;
}

u32int uxtabInstruction(GCONTXT *context, u32int instruction)
{
  DIE_NOW(context, "UXTAB unfinished\n");
  return 0;
}

u32int selInstruction(GCONTXT *context, u32int instruction)
{
  DIE_NOW(context, "SEL unfinished\n");
  return 0;
}

u32int setendInstruction(GCONTXT *context, u32int instruction)
{
  DIE_NOW(context, "SETEND unfinished\n");
  return 0;
}

u32int smuadInstruction(GCONTXT *context, u32int instruction)
{
  DIE_NOW(context, "SMUAD unfinished\n");
  return 0;
}

u32int smusdInstruction(GCONTXT *context, u32int instruction)
{
  DIE_NOW(context, "SMUSD unfinished\n");
  return 0;
}

u32int smladInstruction(GCONTXT *context, u32int instruction)
{
  DIE_NOW(context, "SMLAD unfinished\n");
  return 0;
}

u32int smlaldInstruction(GCONTXT *context, u32int instruction)
{
  DIE_NOW(context, "SMLALD unfinished\n");
  return 0;
}

u32int smlsdInstruction(GCONTXT *context, u32int instruction)
{
  DIE_NOW(context, "SMLSD unfinished\n");
  return 0;
}

u32int smlsldInstruction(GCONTXT *context, u32int instruction)
{
  DIE_NOW(context, "SMLSLD unfinished\n");
  return 0;
}

u32int smmulInstruction(GCONTXT *context, u32int instruction)
{
  DIE_NOW(context, "SMMUL unfinished\n");
  return 0;
}

u32int smmlaInstruction(GCONTXT *context, u32int instruction)
{
  DIE_NOW(context, "SMMLA unfinished\n");
  return 0;
}

u32int smmlsInstruction(GCONTXT *context, u32int instruction)
{
  DIE_NOW(context, "SMMLS unfinished\n");
  return 0;
}

u32int srsInstruction(GCONTXT *context, u32int instruction)
{
  DIE_NOW(context, "SRS unfinished\n");
  return 0;
}

u32int ssatInstruction(GCONTXT *context, u32int instruction)
{
  DIE_NOW(context, "SSAT unfinished\n");
  return 0;
}

u32int ssat16Instruction(GCONTXT *context, u32int instruction)
{
  DIE_NOW(context, "ssat16 unfinished\n");
  return 0;
}

u32int umaalInstruction(GCONTXT *context, u32int instruction)
{
  DIE_NOW(context, "UMAAL unfinished\n");
  return 0;
}

u32int usad8Instruction(GCONTXT *context, u32int instruction)
{
  DIE_NOW(context, "USAD8 unfinished\n");
  return 0;
}

u32int usada8Instruction(GCONTXT *context, u32int instruction)
{
  DIE_NOW(context, "USADA unfinished\n");
  return 0;
}

u32int usatInstruction(GCONTXT *context, u32int instruction)
{
  DIE_NOW(context, "USAT unfinished\n");
  return 0;
}

u32int usat16Instruction(GCONTXT *context, u32int instruction)
{
  DIE_NOW(context, "USAT16 unfinished\n");
  return 0;
}


u32int bkptInstruction(GCONTXT *context, u32int instruction)
{
  DIE_NOW(context, "BKPT unfinished\n");
  return 0;
}



u32int clzInstruction(GCONTXT *context, u32int instruction)
{
  DIE_NOW(context, "CLZ unfinished\n");
  return 0;
}

u32int pldInstruction(GCONTXT *context, u32int instruction)
{
#ifdef ARM_INSTR_TRACE
  printf("Warning: PLD!\n");
#endif
  return context->R15+4;
}

u32int smlabbInstruction(GCONTXT *context, u32int instruction)
{
  DIE_NOW(context, "SMLABB unfinished\n");
  return 0;
}

u32int smlatbInstruction(GCONTXT *context, u32int instruction)
{
  DIE_NOW(context, "SMLATB unfinished\n");
  return 0;
}

u32int smlabtInstruction(GCONTXT *context, u32int instruction)
{
  DIE_NOW(context, "SMLABT unfinished\n");
  return 0;
}

u32int smlattInstruction(GCONTXT *context, u32int instruction)
{
  DIE_NOW(context, "SMLATT unfinished\n");
  return 0;
}

u32int smlawbInstruction(GCONTXT *context, u32int instruction)
{
  DIE_NOW(context, "SMLAWB unfinished\n");
  return 0;
}

u32int smlawtInstruction(GCONTXT *context, u32int instruction)
{
  DIE_NOW(context, "SMLAWT unfinished\n");
  return 0;
}

u32int smlalbbInstruction(GCONTXT *context, u32int instruction)
{
  DIE_NOW(context, "SMLALBB unfinished\n");
  return 0;
}

u32int smlaltbInstruction(GCONTXT *context, u32int instruction)
{
  DIE_NOW(context, "SMLALTB unfinished\n");
  return 0;
}

u32int smlalbtInstruction(GCONTXT *context, u32int instruction)
{
  DIE_NOW(context, "SMLALBT unfinished\n");
  return 0;
}

u32int smlalttInstruction(GCONTXT *context, u32int instruction)
{
  DIE_NOW(context, "SMLALTT unfinished\n");
  return 0;
}

u32int smullInstruction(GCONTXT *context, u32int instruction)
{
  DIE_NOW(context, "SMULL unfinished\n");
  return 0;
}

u32int smulbbInstruction(GCONTXT *context, u32int instruction)
{
  DIE_NOW(context, "SMULBB unfinished\n");
  return 0;
}

u32int smultbInstruction(GCONTXT *context, u32int instruction)
{
  DIE_NOW(context, "SMULTB unfinished\n");
  return 0;
}

u32int smulbtInstruction(GCONTXT *context, u32int instruction)
{
  DIE_NOW(context, "SMULBT unfinished\n");
  return 0;
}

u32int smulttInstruction(GCONTXT *context, u32int instruction)
{
  DIE_NOW(context, "SMULTT unfinished\n");
  return 0;
}

u32int smulwbInstruction(GCONTXT *context, u32int instruction)
{
  DIE_NOW(context, "SMULWD unfinished\n");
  return 0;
}

u32int smulwtInstruction(GCONTXT *context, u32int instruction)
{
  DIE_NOW(context, "SMULWT unfinished\n");
  return 0;
}

u32int qaddInstruction(GCONTXT *context, u32int instruction)
{
  DIE_NOW(context, "QADD unfinished\n");
  return 0;
}

u32int qdaddInstruction(GCONTXT *context, u32int instruction)
{
  DIE_NOW(context, "QDADD unfinished\n");
  return 0;
}

u32int qsubInstruction(GCONTXT *context, u32int instruction)
{
  DIE_NOW(context, "QSUB unfinished\n");
  return 0;
}

u32int qdsubInstruction(GCONTXT *context, u32int instruction)
{
  DIE_NOW(context, "QDSUB unfinished\n");
  return 0;
}

u32int msrInstruction(GCONTXT *context, u32int instr)
{
  u32int instrCC =    (instr & 0xF0000000) >> 28;
  u32int regOrImm =   (instr & 0x02000000); // if 1 then imm12, 0 then Reg
  u32int cpsrOrSpsr = (instr & 0x00400000); // if 0 then cpsr, !0 then spsr
  u32int fieldMsk =   (instr & 0x000F0000) >> 16;

  u32int value = 0;
  u32int nextPC = 0;


  if (!evaluateConditionCode(context, instrCC))
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
    switch (context->CPSR & PSR_MODE)
    {
      case PSR_FIQ_MODE:
        oldValue = context->SPSR_FIQ;
        break;
      case PSR_IRQ_MODE:
        oldValue = context->SPSR_IRQ;
        break;
      case PSR_SVC_MODE:
        oldValue = context->SPSR_SVC;
        break;
      case PSR_ABT_MODE:
        oldValue = context->SPSR_ABT;
        break;
      case PSR_UND_MODE:
        oldValue = context->SPSR_UND;
        break;
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
  if (((fieldMsk & 0x1) == 0x1) && (context->CPSR & PSR_MODE) != PSR_USR_MODE)
  {
#ifndef CONFIG_THUMB2
    // check for thumb toggle!
    if ((oldValue & PSR_T_BIT) != (value & PSR_T_BIT))
    {
          DIE_NOW(context, "MSR toggle THUMB bit.");
    }
#endif
    // separate the field we're gonna update from new value
    u32int appliedValue = (value & 0x000000FF);
    // clear old fields!
    oldValue &= 0xFFFFFF00;
    // update old value...
    oldValue |= appliedValue;
  }
  if ( ((fieldMsk & 0x2) == 0x2) && (context->CPSR & PSR_MODE) != PSR_USR_MODE)
  {
    // extension field: async abt, endianness, IT[7:2]
    // check for endiannes toggle!
    if ((oldValue & PSR_E_BIT) != (value & PSR_E_BIT))
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
  if ( ((fieldMsk & 0x4) == 0x4) && (context->CPSR & PSR_MODE) != PSR_USR_MODE)
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
    switch (context->CPSR & PSR_MODE)
    {
      case PSR_FIQ_MODE:
        context->SPSR_FIQ = oldValue;
        break;
      case PSR_IRQ_MODE:
        context->SPSR_IRQ = oldValue;
        break;
      case PSR_SVC_MODE:
        context->SPSR_SVC = oldValue;
        break;
      case PSR_ABT_MODE:
        context->SPSR_ABT = oldValue;
        break;
      case PSR_UND_MODE:
        context->SPSR_UND = oldValue;
        break;
      default:
        DIE_NOW(context, "MSR: invalid SPSR write for current guest mode.");
    }
  }
  nextPC = context->R15 + 4;
  return nextPC;
}

u32int mrsInstruction(GCONTXT *context, u32int instr)
{
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
  if (evaluateConditionCode(context, instrCC))
  {
    if (regSrc == 0)
    {
      // CPSR case
      value = context->CPSR;
    }
    else
    {
      // SPSR case
      int guestMode = (context->CPSR) & PSR_MODE;
      switch(guestMode)
      {
        case PSR_FIQ_MODE:
          value = context->SPSR_FIQ;
          break;
        case PSR_IRQ_MODE:
          value = context->SPSR_IRQ;
          break;
        case PSR_SVC_MODE:
          value = context->SPSR_SVC;
          break;
        case PSR_ABT_MODE:
          value = context->SPSR_ABT;
          break;
        case PSR_UND_MODE:
          value = context->SPSR_UND;
          break;
        case PSR_USR_MODE:
        case PSR_SYS_MODE:
        default:
          invalidInstruction(instr, "MRS cannot request spsr in user/system mode");
      } // switch ends
    } // spsr case ends
    storeGuestGPR(regDest, value, context);
  } // condition met ends

  nextPC = context->R15 + 4;
  return nextPC;
}



u32int svcInstruction(GCONTXT *context, u32int instruction)
{
  DIE_NOW(0,"I shouldn't be here");
#ifdef ARM_INSTR_TRACE
  printf("SVC instr %08x @ %08x\n", instruction, context->R15);
#endif
  return 0;
}

u32int undefinedInstruction(GCONTXT *context, u32int instruction)
{
  invalidInstruction(instruction, "undefined instruction");
}

u32int t16ItInstruction(GCONTXT *context, u32int instruction)
{
  DIE_NOW(context, "t16ItInstruction: none of the emulation functions support IT/ITSTATE");
}
