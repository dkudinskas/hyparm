#include "common/debug.h"
#include "common/stddef.h"

#include "instructionEmu/interpreter.h"
#include "instructionEmu/loadStoreDecode.h"
#include "instructionEmu/scanner.h"


/* generic load store instruction emulation  *
 * called when we permission fault on memory *
 * access to a protected area - must emulate */
void emulateLoadStoreGeneric(GCONTXT *context, u32int address)
{
#ifdef CONFIG_THUMB2
  u32int instr;
  if (context->CPSR.bits.T)
  {
    /*
     * Guest was executing in Thumb mode
     */
    instr = fetchThumbInstr((u16int *)(context->R15));

    if (txxIsThumb32(instr))
    {
      /*
       * 32-bit Thumb instruction
       *
       * STRB
       */
      if (((instr & THUMB32_STRB_IMM12_MASK) == THUMB32_STRB_IMM12) ||
          ((instr & THUMB32_STRB_IMM8_MASK) == THUMB32_STRB_IMM8))
      {
        clearTranslationsByAddress(context->translationStore, address);
        t32StrbInstruction(context, instr);
      }
      else if ((instr & THUMB32_STRB_REG_MASK) == THUMB32_STRB_REG)
      {
        DIE_NOW(context, ERROR_NOT_IMPLEMENTED);
      }
      /*
       * STRH
       */
      else if ((instr & THUMB32_STRH_REG_IMM5_MASK) == THUMB32_STRH_REG_IMM5)
      {
        clearTranslationsByAddress(context->translationStore, address);
        t32StrhImmediateInstruction(context, instr);
      }
      else if ((instr & THUMB32_STRH_REG_IMM8_MASK) == THUMB32_STRH_REG_IMM8)
      {
        clearTranslationsByAddress(context->translationStore, address);
        t32StrhtInstruction(context, instr);
      }
      else if ((instr & THUMB32_STRH_REG_MASK) == THUMB32_STRH_REG)
      {
        clearTranslationsByAddress(context->translationStore, address);
        t32StrhRegisterInstruction(context, instr);
      }
      /*
       * LDRSH
       */
      else if ((instr & THUMB32_LDRSH_REG_IMM8_MASK) == THUMB32_LDRSH_REG_IMM8)
      {
        t32LdrshImmediate12Instruction(context, instr);
      }
      else if ((instr & THUMB32_LDRSH_REG_IMM12_MASK) == THUMB32_LDRSH_REG_IMM12)
      {
        t32LdrshImmediate8Instruction(context, instr);
      }
      else if ((instr & THUMB32_LDRSH_REG_MASK) == THUMB32_LDRSH_REG)
      {
        t32LdrshRegisterInstruction(context, instr);
      }
      else if ((instr & THUMB32_LDRSH_IMM12_MASK) == THUMB32_LDRSH_IMM12)
      {
        t32LdrshLiteralInstruction(context, instr);
      }
      /*
       * STRD
       */
      else if (((instr & THUMB32_STRD_IMM8_MASK) == THUMB32_STRD_IMM8))
      {
        clearTranslationsByAddress(context->translationStore, address);
        t32StrdImmediateInstruction(context, instr);
      }
      /*
       * TODO: are there any other cases we must handle?
       */
      else
      {
        printf("Instruction: %08x@%08x" EOL,instr, context->R15);
        DIE_NOW(NULL, ERROR_NOT_IMPLEMENTED);
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
        clearTranslationsByAddress(context->translationStore, address);
        t16StrInstruction(context, instr);
      }
      else if ((instr & THUMB16_STR_IMM8_MASK) == THUMB16_STR_IMM8)
      {
        clearTranslationsByAddress(context->translationStore, address);
        t16StrSpInstruction(context, instr);
      }
      /*
       * LDR
       */
      else if (((instr & THUMB16_LDR_IMM5_MASK) == THUMB16_LDR_IMM5) ||
               ((instr & THUMB16_LDR_IMM8_MASK) == THUMB16_LDR_IMM8) ||
               ((instr & THUMB16_LDR_IMM8_LIT_MASK) == THUMB16_LDR_IMM8_LIT) ||
               ((instr & THUMB16_LDR_REG_MASK) == THUMB16_LDR_REG))
      {
        t16LdrInstruction(context, instr);
      }
      /*
       * PUSH
       */
      else if ((instr & THUMB16_PUSH_MASK) == THUMB16_PUSH)
      {
        clearTranslationsByAddress(context->translationStore, address);
        t16PushInstruction(context, instr);
      }
      /*
       * LDRB
       */
      else if (((instr & THUMB16_LDRB_IMM5_MASK) == THUMB16_LDRB_IMM5) ||
               ((instr & THUMB16_LDRB_REG_MASK) == THUMB16_LDRB_REG))
      {
        t16LdrbInstruction(context, instr);
      }
      /*
       * STRB
       */
      else if (((instr & THUMB16_STRB_IMM5_MASK) == THUMB16_STRB_IMM5) ||
               ((instr & THUMB16_STRB_REG_MASK) == THUMB16_STRB_REG))
      {
        clearTranslationsByAddress(context->translationStore, address);
        t16StrbInstruction(context, instr);
      }
      /*
       * STRH
       */
      else if (((instr & THUMB16_STRH_IMM5_MASK) == THUMB16_STRH_IMM5) ||
               ((instr & THUMB16_STRH_REG_MASK) == THUMB16_STRH_REG))
      {
        clearTranslationsByAddress(context->translationStore, address);
        t16StrhInstruction(context, instr);
      }
      /*
       * FIXME: are there any others we need to consider?
       */
      else
      {
        printf("Unimplemented Thumb16 %08x@%08x" EOL, instr, address);
        DIE_NOW(context, ERROR_NOT_IMPLEMENTED);
      }
    }
  }
  else
#endif
  Instruction instr = {.raw = *(volatile u32int *)(context->R15)};
  {
    /*
     * The guest was executing in ARM mode
     */
    // emulate methods will take instr from context, put it there
    if ((instr.raw & STR_IMM_MASK) == STR_IMM_MASKED)
    {
      // storing to a protected area.. adjust block cache if needed
      clearTranslationsByAddress(context->translationStore, address);
      // STR Rd, [Rn, Rm/#imm12]
      armStrImmInstruction(context, instr);
    }
    else if ((instr.raw & STR_REG_MASK) == STR_REG_MASKED)
    {
      // storing to a protected area.. adjust block cache if needed
      clearTranslationsByAddress(context->translationStore, address);
      // STR Rd, [Rn, Rm/#imm12]
      armStrRegInstruction(context, instr);
    }
    else if ((instr.raw & STRB_IMM_MASK) == STRB_IMM_MASKED)
    {
      // storing to a protected area.. adjust block cache if needed
      clearTranslationsByAddress(context->translationStore, address);
      // STRB Rd, [Rn, Rm/#imm12]
      armStrbImmInstruction(context, instr);
    }
    else if ((instr.raw & STRB_REG_MASK) == STRB_REG_MASKED)
    {
      // storing to a protected area.. adjust block cache if needed
      clearTranslationsByAddress(context->translationStore, address);
      // STRB Rd, [Rn, Rm/#imm12]
      armStrbRegInstruction(context, instr);
    }
    else if ((instr.raw & STRH_IMM_MASK) == STRH_IMM_MASKED)
    {
      // storing to a protected area.. adjust block cache if needed
      clearTranslationsByAddress(context->translationStore, address);
      // STRH Rd, [Rn, Rm/#imm12]
      armStrhImmInstruction(context, instr);
    }
    else if ((instr.raw & STRH_REG_MASK) == STRH_REG_MASKED)
    {
      // storing to a protected area.. adjust block cache if needed
      clearTranslationsByAddress(context->translationStore, address);
      // STRH Rd, [Rn, Rm/#imm12]
      armStrhRegInstruction(context, instr);
    }
    else if ((instr.raw & STRD_IMM_MASK) == STRD_IMM_MASKED)
    {
      // storing to a protected area.. adjust block cache if needed
      clearTranslationsByAddress(context->translationStore, address);
      // STRD Rd, [Rn, Rm/#imm12]
      armStrdImmInstruction(context, instr);
    }
    else if ((instr.raw & STRD_REG_MASK) == STRD_REG_MASKED)
    {
      // storing to a protected area.. adjust block cache if needed
      clearTranslationsByAddress(context->translationStore, address);
      // STRD Rd, [Rn, Rm/#imm12]
      armStrdRegInstruction(context, instr);
    }
    else if ((instr.raw & STREX_MASK) == STREX_MASKED)
    {
      // storing to a protected area.. adjust block cache if needed
      clearTranslationsByAddress(context->translationStore, address);
      // STREX Rd, [Rn, Rm]
      armStrexInstruction(context, instr);
    }
    else if ((instr.raw & STM_MASK) == STM_MASKED)
    {
      // more tricky with cache validation! since we do this in the stmInstruction
      // per address (word in memory) depending on the length of {reg list}
      // STM Rn, {reg list}
      armStmInstruction(context, instr);
    }
    else if ((instr.raw & LDR_REG_MASK) == LDR_REG_MASKED)
    {
      // LDR Rd, Rn, Rm
      armLdrRegInstruction(context, instr);
    }
    else if ((instr.raw & LDR_IMM_MASK) == LDR_IMM_MASKED)
    {
      // LDR Rd, Rn, imm12
      armLdrImmInstruction(context, instr);
    }
    else if ((instr.raw & LDRB_IMM_MASK) == LDRB_IMM_MASKED)
    {
      // LDRB Rd, [Rn, Rm/#imm12]
      armLdrbImmInstruction(context, instr);
    }
    else if ((instr.raw & LDRB_REG_MASK) == LDRB_REG_MASKED)
    {
      // LDRB Rd, [Rn, Rm/#imm12]
      armLdrbRegInstruction(context, instr);
    }
    else if ((instr.raw & LDRH_IMM_MASK) == LDRH_IMM_MASKED)
    {
      // LDRH Rd, [Rn, #imm12]
      armLdrhImmInstruction(context, instr);
    }
    else if ((instr.raw & LDRH_REG_MASK) == LDRH_REG_MASKED)
    {
      // LDRH Rd, [Rn, Rm]
      armLdrhRegInstruction(context, instr);
    }
    else if ((instr.raw & LDRD_IMM_MASK) == LDRD_IMM_MASKED)
    {
      // LDRD Rd, [Rn, #imm12]
      armLdrdImmInstruction(context, instr);
    }
    else if ((instr.raw & LDRD_REG_MASK) == LDRD_REG_MASKED)
    {
      // LDRD Rd, [Rn, Rm]
      armLdrdRegInstruction(context, instr);
    }
    else if ((instr.raw & LDM_MASK) == LDM_MASKED)
    {
      // LDM, Rn, {reg list}
      armLdmInstruction(context, instr);
    }
    else if ((instr.raw & LDREX_MASK) == LDREX_MASKED)
    {
      // LDREX Rd, [Rn]
      armLdrexInstruction(context, instr);
    }
    else
    {
      printf("LoadStore @ %08x instruction %08x" EOL, context->R15, instr.raw);
      DIE_NOW(context, ERROR_NOT_IMPLEMENTED);
    }
  }
}
