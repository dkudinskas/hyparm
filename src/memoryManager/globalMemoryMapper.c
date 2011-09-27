#include "common/debug.h"

#include "cpuArch/constants.h"

#include "guestManager/guestContext.h"

#ifdef CONFIG_THUMB2
# include "instructionEmu/commonInstrFunctions.h"
#endif

#include "instructionEmu/interpreter/loadInstructions.h"
#include "instructionEmu/interpreter/storeInstructions.h"

#include "memoryManager/globalMemoryMapper.h"


/* generic load store instruction emulation  *
 * called when we permission fault on memory *
 * access to a protected area - must emulate */
void emulateLoadStoreGeneric(GCONTXT * context, u32int address)
{
  u32int instr;
  u32int eobInstrBackup;
  u32int eobHalfInstrBackup;

#ifdef CONFIG_THUMB2
  if (context->CPSR & PSR_T_BIT)
  {
    /*
     * Guest was executing in Thumb mode
     */
    instr = fetchThumbInstr((u16int *)(context->R15));
    eobInstrBackup = context->endOfBlockInstr;
    eobHalfInstrBackup = context->endOfBlockHalfInstr;

    if (isThumb32(instr))
    {
      /*
       * 32-bit Thumb instruction
       *
       * STRB
       */
      if
        (
          ((instr & THUMB32_STRB_IMM12_MASK) == THUMB32_STRB_IMM12) ||
          ((instr & THUMB32_STRB_IMM8_MASK) == THUMB32_STRB_IMM8)
        )
      {
        validateCachePreChange(context->blockCache, address);
        context->endOfBlockInstr = instr;
        t32StrbInstruction(context, instr);
      }
      else if ((instr & THUMB32_STRB_REG_MASK) == THUMB32_STRB_REG)
      {
        DIE_NOW(0,"Unimplemented Thumb32 register generic load/store");
      }
      /*
       * STRH
       */
      else if ((instr & THUMB32_STRH_REG_IMM5_MASK) == THUMB32_STRH_REG_IMM5)
       {
         validateCachePreChange(context->blockCache, address);
         context->endOfBlockInstr = instr;
         t32StrhImmediateInstruction(context, instr);
       }
      else if ((instr & THUMB32_STRH_REG_IMM8_MASK) == THUMB32_STRH_REG_IMM8)
      {
        validateCachePreChange(context->blockCache, address);
        context->endOfBlockInstr = instr;
        t32StrhtInstruction(context, instr);
      }
      else if ((instr & THUMB32_STRH_REG_MASK) == THUMB32_STRH_REG)
      {
        validateCachePreChange(context->blockCache, address);
        context->endOfBlockInstr = instr;
        t32StrhRegisterInstruction(context, instr);
      }
      /*
       * LDRSH
       */
      else if ((instr & THUMB32_LDRSH_REG_IMM8_MASK) == THUMB32_LDRSH_REG_IMM8)
      {
        context->endOfBlockInstr = instr;
        t32LdrshImmediate12Instruction(context, instr);
      }
      else if ((instr & THUMB32_LDRSH_REG_IMM12_MASK) == THUMB32_LDRSH_REG_IMM12)
      {
        context->endOfBlockInstr = instr;
        t32LdrshImmediate8Instruction(context, instr);
      }
      else if ((instr & THUMB32_LDRSH_REG_MASK) == THUMB32_LDRSH_REG)
      {
        context->endOfBlockInstr = instr;
        t32LdrshRegisterInstruction(context, instr);
      }
      else if ((instr & THUMB32_LDRSH_IMM12_MASK) == THUMB32_LDRSH_IMM12)
      {
        context->endOfBlockInstr = instr;
        t32LdrshLiteralInstruction(context, instr);
      }
      /*
       * STRD
       */
      else if (((instr & THUMB32_STRD_IMM8_MASK) == THUMB32_STRD_IMM8))
      {
        validateCachePreChange(context->blockCache, address);
        context->endOfBlockInstr = instr;
        t32StrdImmediateInstruction(context, instr);
      }
      /*
       * TODO: are there any other cases we must handle?
       */
      else
      {
        printf("Instruction: %08x@%08x\n",instr, context->R15);
        DIE_NOW(0, "Unknown THUMB32 load/store instr");
      }
    }
    else
    {
      /*
       * 16-bit Thumb instruction
       *
       * STR
       */
      if ((instr & THUMB16_STR_IMM5_MASK) == THUMB16_STR_IMM5)
      {
        context->endOfBlockInstr = instr;
        validateCachePreChange(context->blockCache, address);
        t16StrInstruction(context, instr);
      }
      else if ((instr & THUMB16_STR_IMM8_MASK) == THUMB16_STR_IMM8)
      {
        context->endOfBlockInstr = instr;
        validateCachePreChange(context->blockCache, address);
        t16StrSpInstruction(context, instr);
      }
      /*
       * LDR
       */
      else if
        (
          ((instr & THUMB16_LDR_IMM5_MASK) == THUMB16_LDR_IMM5) ||
          ((instr & THUMB16_LDR_IMM8_MASK) == THUMB16_LDR_IMM8) ||
          ((instr & THUMB16_LDR_IMM8_LIT_MASK) == THUMB16_LDR_IMM8_LIT) ||
          ((instr & THUMB16_LDR_REG_MASK) == THUMB16_LDR_REG)
        )
      {
        context->endOfBlockInstr = instr;
        t16LdrInstruction(context, instr);
      }
      /*
       * PUSH
       */
      else if ((instr & THUMB16_PUSH_MASK) == THUMB16_PUSH)
      {
        context->endOfBlockInstr = instr;
        validateCachePreChange(context->blockCache,address);
        t16PushInstruction(context, instr);
      }
      /*
       * LDRB
       */
      else if
        (
          ((instr & THUMB16_LDRB_IMM5_MASK) == THUMB16_LDRB_IMM5) ||
          ((instr & THUMB16_LDRB_REG_MASK) == THUMB16_LDRB_REG)
        )
      {
        context->endOfBlockInstr = instr;
        t16LdrbInstruction(context, instr);
      }
      /*
       * STRB
       */
      else if
        (
          ((instr & THUMB16_STRB_IMM5_MASK) == THUMB16_STRB_IMM5) ||
          ((instr & THUMB16_STRB_REG_MASK) == THUMB16_STRB_REG)
        )
      {
        context->endOfBlockInstr = instr;
        validateCachePreChange(context->blockCache,address);
        t16StrbInstruction(context, instr);
      }
      /*
       * STRH
       */
      else if
        (
          ((instr & THUMB16_STRH_IMM5_MASK) == THUMB16_STRH_IMM5) ||
          ((instr & THUMB16_STRH_REG_MASK) == THUMB16_STRH_REG)
        )
      {
        context->endOfBlockInstr = instr;
        validateCachePreChange(context->blockCache, address);
        t16StrhInstruction(context, instr);
      }
      /*
       * FIXME: are there any others we need to consider?
       */
      else
      {
        printf("Unimplemented Thumb16 %08x@%08x\n",instr,address);
        DIE_NOW(0,"Unimplemented Thumb16 Load/Store\n");
      }
    }
    context->endOfBlockHalfInstr = eobHalfInstrBackup;
  }
  else
#endif
  {
    /*
     * The guest was executing in ARM mode
     */
    // get the store instruction
    instr = *(volatile u32int *)(context->R15);
    // save the end of block instruction, as we faulted /not/ on EOB
    eobInstrBackup = context->endOfBlockInstr;
    // emulate methods will take instr from context, put it there
    context->endOfBlockInstr = instr;

    if
      (
        ((instr & STR_IMM_MASK) == STR_IMM_MASKED) ||
        ((instr & STR_REG_MASK) == STR_REG_MASKED)
      )
    {
      // storing to a protected area.. adjust block cache if needed
      validateCachePreChange(context->blockCache, address);
      // STR Rd, [Rn, Rm/#imm12]
      armStrInstruction(context, instr);
    }
    else if
      (
        ((instr & STRB_IMM_MASK) == STRB_IMM_MASKED) ||
        ((instr & STRB_REG_MASK) == STRB_REG_MASKED)
      )
    {
      // storing to a protected area.. adjust block cache if needed
      validateCachePreChange(context->blockCache, address);
      // STRB Rd, [Rn, Rm/#imm12]
      armStrbInstruction(context, instr);
    }
    else if
      (
        ((instr & STRH_IMM_MASK) == STRH_IMM_MASKED) ||
        ((instr & STRH_REG_MASK) == STRH_REG_MASKED)
      )
    {
      // storing to a protected area.. adjust block cache if needed
      validateCachePreChange(context->blockCache, address);
      // STRH Rd, [Rn, Rm/#imm12]
      armStrhInstruction(context, instr);
    }
    else if
      (
        ((instr & STRD_IMM_MASK) == STRD_IMM_MASKED) ||
        ((instr & STRD_REG_MASK) == STRD_REG_MASKED)
      )
    {
      // storing to a protected area.. adjust block cache if needed
      validateCachePreChange(context->blockCache, address);
      // STRD Rd, [Rn, Rm/#imm12]
      armStrdInstruction(context, instr);
    }
    else if ((instr & STM_MASK) == STM_MASKED)
    {
      // more tricky with cache validation! since we do this in the stmInstruction
      // per address (word in memory) depending on the length of {reg list}
      // STM Rn, {reg list}
      armStmInstruction(context, instr);
    }
    else if ((instr & LDR_MASK) == LDR_MASKED)
    {
      // loads don't change memory. no need to validate cache
      // LDR Rd, Rn/#imm12
      armLdrInstruction(context, instr);
    }
    else if ((instr & LDRB_MASK) == LDRB_MASKED)
    {
      // LDRB Rd, [Rn, Rm/#imm12]
      armLdrbInstruction(context, instr);
    }
    else if
      (
        ((instr & LDRH_IMM_MASK) == LDRH_IMM_MASKED) ||
        ((instr & LDRH_REG_MASK) == LDRH_REG_MASKED)
      )
    {
      // LDRH Rd, [Rn, Rm/#imm12]
      armLdrhInstruction(context, instr);
    }
    else if
      (
        ((instr & LDRD_IMM_MASK) == LDRD_IMM_MASKED) ||
        ((instr & LDRD_REG_MASK) == LDRD_REG_MASKED)
      )
    {
      // LDRD Rd, [Rn, Rm/#imm12]
      armLdrdInstruction(context, instr);
    }
    else if ((instr & LDM_MASK) == LDM_MASKED)
    {
      // LDM, Rn, {reg list}
      armLdmInstruction(context, instr);
    }
    else if ((instr & LDREX_MASK) == LDREX_MASKED)
    {
      // LDREX Rd, [Rn]
      armLdrexInstruction(context, instr);
    }
    else
    {
      printf("LoadStore @ %08x instruction %08x\n", context->R15, instr);
      DIE_NOW(context, "Load/Store generic unimplemented\n");
    }
  }
  // restore end of block instruction
  context->endOfBlockInstr = eobInstrBackup;
}
