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
#ifdef CONFIG_BLOCK_COPY
  DIE_NOW(context, "nopInstruction is executed but not yet checked for blockCopyCompatibility");
#else
  printf("ERROR: NOP instr %08x @ %08x should not have trapped!\n",
         instruction, context->R15);
  DIE_NOW(context, "die.");
#endif
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
#ifdef CONFIG_BLOCK_COPY
  return context->PCOfLastInstruction + 4;
#else
  return context->R15+4;
#endif
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
#ifdef CONFIG_BLOCK_COPY
  return context->PCOfLastInstruction + 4;
#else
  return context->R15+4;
#endif
}

u32int isbInstruction(GCONTXT *context, u32int instruction)
{
#ifdef ARM_INSTR_TRACE
  printf("Warning: ISB (ignored)!\n");
#endif
#ifdef CONFIG_BLOCK_COPY
  return context->PCOfLastInstruction + 4;
#else
  return context->R15+4;
#endif
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
#ifdef CONFIG_BLOCK_COPY
  return context->PCOfLastInstruction + 4;
#else
  return context->R15+4;
#endif
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
#ifdef CONFIG_BLOCK_COPY
  return context->PCOfLastInstruction+4;
#else
  return context->R15+4;
#endif
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
#ifdef CONFIG_BLOCK_COPY
  printf("CPS instr %08x @ %08x\n", instr, context->PCOfLastInstruction);
#else
  printf("CPS instr %08x @ %08x\n", instr, context->R15);
#endif
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
#ifndef CONFIG_BLOCK_COPY_NO_IRQ
        /*If No IRQ's are wanted we shouldn't switch bits*/
        oldCpsr &= ~PSR_I_BIT;
#endif
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

#ifdef CONFIG_BLOCK_COPY
  return context->PCOfLastInstruction + 4;
#else
  return context->R15 + 4;
#endif
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
#ifdef CONFIG_BLOCK_COPY
  return context->PCOfLastInstruction + 4;
#else
  return context->R15+4;
#endif
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
#ifdef CONFIG_BLOCK_COPY
    nextPC = context->PCOfLastInstruction + 4;
#else
    nextPC = context->R15 + 4;
#endif
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

#ifdef CONFIG_BLOCK_COPY_NO_IRQ
  /* Set maskbit for interrupts */
  value = value | 0x80;
#endif

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
#ifdef CONFIG_BLOCK_COPY
  nextPC = context->PCOfLastInstruction + 4;
#else
  nextPC = context->R15 + 4;
#endif
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
#ifdef CONFIG_BLOCK_COPY
  printf("MRS instr %08x @ %08x\n", instr, context->PCOfLastInstruction);
#else
  printf("MRS instr %08x @ %08x\n", instr, context->R15);
#endif
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
#ifdef CONFIG_BLOCK_COPY_NO_IRQ
    /* Make sure that interrupts are not enabled*/
    value = value | 0x80; //bit 7 must be 1 so interupt bit is masked
#endif
#ifdef CONFIG_BLOCK_COPY_NO_IRQ
    /* When Interrupts are disabled the mask bit will be one but we don't want linux to now this */
    /* This will crash with undefined handler
     * storeGuestGPR(regDest, (value & ~0x00000080), context); */
    /* Use a hack to resolve this issue -> normally default behaviour but if certain instruction than modified behaviour */
    if(instr == 0xe10f2000)
    {
      storeGuestGPR(regDest, (value & ~0x00000080), context);
    }
    else
    {
      storeGuestGPR(regDest, value, context);
    }

#else
    storeGuestGPR(regDest, value, context);
#endif
  } // condition met ends

#ifdef CONFIG_BLOCK_COPY
  nextPC = context->PCOfLastInstruction + 4;
#else
  nextPC = context->R15 + 4;
#endif
  return nextPC;
}

u32int svcInstruction(GCONTXT *context, u32int instruction)
{
#ifdef CONFIG_BLOCK_COPY
  DIE_NOW(0, "svcInstruction is executed but not yet checked for blockCopyCompatibility");
#endif
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


#ifdef CONFIG_BLOCK_COPY

u32int* nopPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  DIE_NOW(0, "nop PCFunct unfinished\n");
  return 0;
}

u32int* mulPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  DIE_NOW(0, "mul PCFunct unfinished\n");
  return 0;
}

u32int* mlaPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  DIE_NOW(0, "mla PCFunct unfinished\n");
  return 0;
}

u32int* swpPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  DIE_NOW(0, "swp PCFunct unfinished\n");
  return 0;
}

u32int* sumlalPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  DIE_NOW(0, "sumlal PCFunct unfinished\n");
  return 0;
}

u32int* sumullPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  DIE_NOW(0, "sumull PCFunct unfinished\n");
  return 0;
}

u32int* pliPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  DIE_NOW(0, "pli PCFunct unfinished\n");
  return 0;
}

u32int* dbgPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  DIE_NOW(0, "dbg PCFunct unfinished\n");
  return 0;
}

u32int* dmbPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  DIE_NOW(0, "dmb PCFunct unfinished\n");
  return 0;
}

u32int* dsbPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  DIE_NOW(0, "dsb PCFunct unfinished\n");
  return 0;
}

u32int* isbPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  DIE_NOW(0, "isb PCFunct shouldn't be used since isbInstructions are always emulated!!\n");
  return 0;
}

u32int* bfcPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{

  /* Normally always safe*/
  u32int instruction = *instructionAddr;
  u32int destReg = (instruction>>12) & 0xF;
  if(destReg == 0xF)
  {
    DIE_NOW(0,"bfc PC: with Rd == PC -> UNPREDICTABLE");
  }

  //Other fields are safe
  currBlockCopyCacheAddr=checkAndClearBlockCopyCacheAddress(currBlockCopyCacheAddr,context->blockCache,(u32int*)context->blockCopyCache,(u32int*)context->blockCopyCacheEnd);
  *(currBlockCopyCacheAddr++)=instruction;

  return currBlockCopyCacheAddr;
}

u32int* bfiPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  DIE_NOW(0, "bfi PCFunct unfinished\n");
  return 0;
}

u32int* mlsPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  DIE_NOW(0, "mls PCFunct unfinished\n");
  return 0;
}

u32int* movwPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{ //Can be optimized -> this instruction is always safe!

  currBlockCopyCacheAddr=checkAndClearBlockCopyCacheAddress(currBlockCopyCacheAddr,context->blockCache,(u32int*)context->blockCopyCache,(u32int*)context->blockCopyCacheEnd);
  *(currBlockCopyCacheAddr++)=(*instructionAddr);

  return currBlockCopyCacheAddr;
}

u32int* movtPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  DIE_NOW(0, "movt PCFunct unfinished\n");
  return 0;
}

u32int* rbitPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  DIE_NOW(0, "rbit PCFunct unfinished\n");
  return 0;
}

u32int* usbfxPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  //This cannot be a dangerous function see ARM ARM p. 778
  u32int instruction = *instructionAddr;
  if((instruction & 0xF) == 0xF || ((instruction>>12)&0xF)==0xF){
    DIE_NOW(0, "ubfx PCFunct: with Rd or Rn == PC -> UNPREDICTABLE");
  }
  //Just copy the instruction
  currBlockCopyCacheAddr=checkAndClearBlockCopyCacheAddress(currBlockCopyCacheAddr,context->blockCache,(u32int*)context->blockCopyCache,(u32int*)context->blockCopyCacheEnd);
  *(currBlockCopyCacheAddr++)=instruction;

  return currBlockCopyCacheAddr;
}

u32int* smcPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  DIE_NOW(0, "smc PCFunct unfinished\n");
  return 0;
}

u32int* clrexPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  DIE_NOW(0, "clrex PCFunct unfinished\n");
  return 0;
}

u32int* yieldPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  DIE_NOW(0, "yield PCFunct unfinished\n");
  return 0;
}

u32int* wfePCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  DIE_NOW(0, "wfe PCFunct unfinished\n");
  return 0;
}

u32int* wfiPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  DIE_NOW(0, "wfi PCFunct unfinished\n");
  return 0;
}

u32int* sevPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  DIE_NOW(0, "sev PCFunct unfinished\n");
  return 0;
}

u32int* cpsiePCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  DIE_NOW(0, "cpsie PCFunct unfinished\n");
  return 0;
}

u32int* cpsidPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  DIE_NOW(0, "cpsid PCFunct unfinished\n");
  return 0;
}

u32int* cpsPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  DIE_NOW(0, "cps PCFunct unfinished\n");
  return 0;
}

u32int* pkhbtPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  DIE_NOW(0, "pkhbt PCFunct unfinished\n");
  return 0;
}

u32int* pkhtbPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  DIE_NOW(0, "pkhtb PCFunct unfinished\n");
  return 0;
}

u32int* qadd16PCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  DIE_NOW(0, "qadd16 PCFunct unfinished\n");
  return 0;
}

u32int* qadd8PCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  DIE_NOW(0, "qadd8 PCFunct unfinished\n");
  return 0;
}

u32int* qaddsubxPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  DIE_NOW(0, "qaddsubx PCFunct unfinished\n");
  return 0;
}

u32int* qsub16PCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  DIE_NOW(0, "qsub16 PCFunct unfinished\n");
  return 0;
}

u32int* qsub8PCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  DIE_NOW(0, "qsub8 PCFunct unfinished\n");
  return 0;
}

u32int* qsubaddxPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  DIE_NOW(0, "qsubaddx PCFunct unfinished\n");
  return 0;
}

u32int* sadd16PCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  DIE_NOW(0, "sadd16 PCFunct unfinished\n");
  return 0;
}

u32int* sadd8PCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  DIE_NOW(0, "sadd8 PCFunct unfinished\n");
  return 0;
}

u32int* saddsubxPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  DIE_NOW(0, "saddsubx PCFunct unfinished\n");
  return 0;
}

u32int* shadd16PCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  DIE_NOW(0, "shadd16 PCFunct unfinished\n");
  return 0;
}

u32int* shadd8PCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  DIE_NOW(0, "shadd8 PCFunct unfinished\n");
  return 0;
}

u32int* shaddsubxPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  DIE_NOW(0, "shaddsubx PCFunct unfinished\n");
  return 0;
}

u32int* shsub16PCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  DIE_NOW(0, "shsub16 PCFunct unfinished\n");
  return 0;
}

u32int* shsub8PCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  DIE_NOW(0, "shsub8 PCFunct unfinished\n");
  return 0;
}

u32int* shsubaddxPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  DIE_NOW(0, "shsubaddx PCFunct unfinished\n");
  return 0;
}

u32int* ssub16PCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  DIE_NOW(0, "ssub16 PCFunct unfinished\n");
  return 0;
}

u32int* ssub8PCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  DIE_NOW(0, "ssub8 PCFunct unfinished\n");
  return 0;
}

u32int* ssubaddxPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  DIE_NOW(0, "ssubaddx PCFunct unfinished\n");
  return 0;
}

u32int* uadd16PCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  DIE_NOW(0, "uadd16 PCFunct unfinished\n");
  return 0;
}

u32int* uadd8PCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  DIE_NOW(0, "uadd8 PCFunct unfinished\n");
  return 0;
}

u32int* uaddsubxPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  DIE_NOW(0, "uaddsubx PCFunct unfinished\n");
  return 0;
}

u32int* uhadd16PCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  DIE_NOW(0, "uhadd16 PCFunct unfinished\n");
  return 0;
}

u32int* uhadd8PCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  DIE_NOW(0, "uhadd8 PCFunct unfinished\n");
  return 0;
}

u32int* uhaddsubxPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  DIE_NOW(0, "uhaddsubx PCFunct unfinished\n");
  return 0;
}

u32int* uhsub16PCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  DIE_NOW(0, "uhsub16 PCFunct unfinished\n");
  return 0;
}

u32int* uhsub8PCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  DIE_NOW(0, "uhsub8 PCFunct unfinished\n");
  return 0;
}

u32int* uhsubaddxPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  DIE_NOW(0, "uhsubaddx PCFunct unfinished\n");
  return 0;
}

u32int* uqadd16PCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  DIE_NOW(0, "uqadd16 PCFunct unfinished\n");
  return 0;
}

u32int* uqadd8PCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  DIE_NOW(0, "uqadd8 PCFunct unfinished\n");
  return 0;
}

u32int* uqaddsubxPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  DIE_NOW(0, "uqaddsubx PCFunct unfinished\n");
  return 0;
}

u32int* uqsub16PCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  DIE_NOW(0, "uqsub16 PCFunct unfinished\n");
  return 0;
}

u32int* uqsub8PCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  DIE_NOW(0, "uqsub8 PCFunct unfinished\n");
  return 0;
}

u32int* uqsubaddxPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  DIE_NOW(0, "uqsubaddx PCFunct unfinished\n");
  return 0;
}

u32int* usub16PCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  DIE_NOW(0, "usub16 PCFunct unfinished\n");
  return 0;
}
u32int* usub8PCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  DIE_NOW(0, "usub8 PCFunct unfinished\n");
  return 0;
}

u32int* usubaddxPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  DIE_NOW(0, "usubaddx PCFunct unfinished\n");
  return 0;
}

u32int* revPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  DIE_NOW(0, "rev PCFunct unfinished\n");
  return 0;
}

u32int* rev16PCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  DIE_NOW(0, "rev16 PCFunct unfinished\n");
  return 0;
}

u32int* revshPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  DIE_NOW(0, "revsh PCFunct unfinished\n");
  return 0;
}

u32int* rfePCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  DIE_NOW(0, "rfe PCFunct unfinished\n");
  return 0;
}

u32int* sxthPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  DIE_NOW(0, "sxth PCFunct unfinished\n");
  return 0;
}

u32int* sxtb16PCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  DIE_NOW(0, "sxtb16 PCFunct unfinished\n");
  return 0;
}

u32int* sxtbPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  u32int instruction = *instructionAddr;

  if((instruction & 0xF)==0xF)
  {
    //ARM ARM p.752
    DIE_NOW(0, "stxb PC: Rm = PC -> UNPREDICTABLE!");
  }
  //Always safe when not unpredictable
  currBlockCopyCacheAddr=checkAndClearBlockCopyCacheAddress(currBlockCopyCacheAddr,context->blockCache,(u32int*)context->blockCopyCache,(u32int*)context->blockCopyCacheEnd);
  *(currBlockCopyCacheAddr++)=instruction;

  return currBlockCopyCacheAddr;
}

u32int* uxthPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  u32int instruction = *instructionAddr;

  if((instruction & 0xF)==0xF)
  {
    //ARM ARM p.836
    DIE_NOW(0, "uxth PC: Rm = PC -> UNPREDICTABLE!");
  }
  //Always safe when not unpredictable
  currBlockCopyCacheAddr=checkAndClearBlockCopyCacheAddress(currBlockCopyCacheAddr,context->blockCache,(u32int*)context->blockCopyCache,(u32int*)context->blockCopyCacheEnd);
  *(currBlockCopyCacheAddr++)=instruction;

  return currBlockCopyCacheAddr;
}

u32int* uxtb16PCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  DIE_NOW(0, "uxtb16 PCFunct unfinished\n");
  return 0;
}


u32int* uxtbPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{//Can be optimized -> this instruction is always safe!
  u32int instruction = *instructionAddr;
  if( (instruction & 0xF) == 0xF)
  {
    DIE_NOW(0,"uxtbPCInstruction: Rm is PC -> unpredictable");
  }

  currBlockCopyCacheAddr=checkAndClearBlockCopyCacheAddress(currBlockCopyCacheAddr,context->blockCache,(u32int*)context->blockCopyCache,(u32int*)context->blockCopyCacheEnd);
  *(currBlockCopyCacheAddr++)=instruction;

  return currBlockCopyCacheAddr;
}

u32int* sxtahPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  DIE_NOW(0, "sxtah PCFunct unfinished\n");
  return 0;
}

u32int* sxtab16PCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  DIE_NOW(0, "sxtab16 PCFunct unfinished\n");
  return 0;
}

u32int* sxtabPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  DIE_NOW(0, "sxtab PCFunct unfinished\n");
  return 0;
}

u32int* uxtahPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  DIE_NOW(0, "uxtah PCFunct unfinished\n");
  return 0;
}

u32int* uxtab16PCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  DIE_NOW(0, "uxtab16 PCFunct unfinished\n");
  return 0;
}

u32int* uxtabPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  DIE_NOW(0, "uxtab PCFunct unfinished\n");
  return 0;
}

u32int* selPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  DIE_NOW(0, "sel PCFunct unfinished\n");
  return 0;
}

u32int* setendPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  DIE_NOW(0, "setend PCFunct unfinished\n");
  return 0;
}

u32int* smuadPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  DIE_NOW(0, "smuad PCFunct unfinished\n");
  return 0;
}

u32int* smusdPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  DIE_NOW(0, "smusd PCFunct unfinished\n");
  return 0;
}

u32int* smladPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  DIE_NOW(0, "smlad PCFunct unfinished\n");
  return 0;
}

u32int* smlaldPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  DIE_NOW(0, "smlald PCFunct unfinished\n");
  return 0;
}

u32int* smlsdPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  DIE_NOW(0, "smlsd PCFunct unfinished\n");
  return 0;
}

u32int* smlsldPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  DIE_NOW(0, "smlsld PCFunct unfinished\n");
  return 0;
}

u32int* smmulPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  DIE_NOW(0, "smmul PCFunct unfinished\n");
  return 0;
}

u32int* smmlaPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  DIE_NOW(0, "smmla PCFunct unfinished\n");
  return 0;
}

u32int* smmlsPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  DIE_NOW(0, "smmls PCFunct unfinished\n");
  return 0;
}

u32int* srsPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  DIE_NOW(0, "srs PCFunct unfinished\n");
  return 0;
}

u32int* ssatPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  DIE_NOW(0, "ssat PCFunct unfinished\n");
  return 0;
}

u32int* ssat16PCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  DIE_NOW(0, "ssat16 PCFunct unfinished\n");
  return 0;
}

u32int* umaalPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  DIE_NOW(0, "umaal PCFunct unfinished\n");
  return 0;
}

u32int* usad8PCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  DIE_NOW(0, "usad8 PCFunct unfinished\n");
  return 0;
}

u32int* usada8PCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  DIE_NOW(0, "usada8 PCFunct unfinished\n");
  return 0;
}

u32int* usatPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  DIE_NOW(0, "usat PCFunct unfinished\n");
  return 0;
}

u32int* usat16PCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  DIE_NOW(0, "usat16 PCFunct unfinished\n");
  return 0;
}

u32int* bkptPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  DIE_NOW(0, "bkpt PCFunct unfinished\n");
  return 0;
}

u32int* clzPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  u32int instruction = *instructionAddr;

  if((instruction & 0xF) == 0xF || ((instruction>>12 & 0xF)==0xF))
  {  //see ARM ARM clz p.384
    DIE_NOW(0, "clz PCFunct: bits 0-3 = 0xF -> UNPREDICTABLE BEHAVIOR\n");
  }
  //Since no register can be PC the instruction is save to execute -> copy it to blockCopyCache
  currBlockCopyCacheAddr=checkAndClearBlockCopyCacheAddress(currBlockCopyCacheAddr,context->blockCache,(u32int*)context->blockCopyCache,(u32int*)context->blockCopyCacheEnd);
  *(currBlockCopyCacheAddr++)=instruction;

  return currBlockCopyCacheAddr;
}

u32int* pldPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  DIE_NOW(0, "pld PCFunct unfinished\n");
  return 0;
}

u32int* smlabbPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  DIE_NOW(0, "smlabb PCFunct unfinished\n");
  return 0;
}

u32int* smlatbPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  DIE_NOW(0, "smlatb PCFunct unfinished\n");
  return 0;
}

u32int* smlabtPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  DIE_NOW(0, "smlabt PCFunct unfinished\n");
  return 0;
}

u32int* smlattPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  DIE_NOW(0, "smlatt PCFunct unfinished\n");
  return 0;
}

u32int* smlawbPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  DIE_NOW(0, "smlawb PCFunct unfinished\n");
  return 0;
}

u32int* smlawtPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  DIE_NOW(0, "smlawt PCFunct unfinished\n");
  return 0;
}

u32int* smlalbbPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  DIE_NOW(0, "smlalbb PCFunct unfinished\n");
  return 0;
}

u32int* smlaltbPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  DIE_NOW(0, "smlaltb PCFunct unfinished\n");
  return 0;
}

u32int* smlalbtPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  DIE_NOW(0, "smlalbt PCFunct unfinished\n");
  return 0;
}

u32int* smlalttPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  DIE_NOW(0, "smlaltt PCFunct unfinished\n");
  return 0;
}

u32int* smulbbPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  DIE_NOW(0, "smulbb PCFunct unfinished\n");
  return 0;
}

u32int* smultbPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  DIE_NOW(0, "smultb PCFunct unfinished\n");
  return 0;
}

u32int* smulbtPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  DIE_NOW(0, "smulbt PCFunct unfinished\n");
  return 0;
}

u32int* smulttPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  DIE_NOW(0, "smultt PCFunct unfinished\n");
  return 0;
}

u32int* smulwbPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  DIE_NOW(0, "smulwb PCFunct unfinished\n");
  return 0;
}

u32int* smulwtPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  DIE_NOW(0, "smulwt PCFunct unfinished\n");
  return 0;
}

u32int* qaddPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  DIE_NOW(0, "qadd PCFunct unfinished\n");
  return 0;
}

u32int* qdaddPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  DIE_NOW(0, "qdadd PCFunct unfinished\n");
  return 0;
}

u32int* qsubPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  DIE_NOW(0, "qsub PCFunct unfinished\n");
  return 0;
}

u32int* qdsubPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  DIE_NOW(0, "qdsub PCFunct unfinished\n");
  return 0;
}

u32int* msrPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  DIE_NOW(0, "msr PCFunct unfinished\n");
  return 0;
}

u32int* mrsPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  DIE_NOW(0, "mrs PCFunct unfinished\n");
  return 0;
}

u32int* svcPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  DIE_NOW(0, "svc PCFunct unfinished\n");
  return 0;
}

u32int* undefinedPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  DIE_NOW(0, "undefined PCFunct unfinished\n");
  return 0;
}

#endif /* CONFIG_BLOCK_COPY */
