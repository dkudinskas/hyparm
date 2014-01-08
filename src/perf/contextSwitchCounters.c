#include "perf/contextSwitchCounters.h"
#include "instructionEmu/interpreter.h"

void dumpCounters(const GCONTXT* context)
{
  printf("====================================\n");
  printf("svc  count: %08x\n", context->svcCount);
  printf("====================================\n");

  // ALU exception return
  printf("addExceptionReturn count: %08x\n", context->addExceptionReturn);
  // ALU computed jumps
  printf("addJump count: %08x\n", context->addJump);
  printf("andJump count: %08x\n", context->andJump);
  printf("movJump count: %08x\n", context->movJump);
  // branches
  printf("branchLink count: %08x\n", context->branchLink);
  printf("branchNonlink count: %08x\n", context->branchNonlink);
  printf("branchConditional count: %08x\n", context->branchConditional);
  printf("branchNonconditional count: %08x\n", context->branchNonconditional);
  printf("branchImmediate count: %08x\n", context->branchImmediate);
  printf("branchRegister count: %08x\n", context->branchRegister);
  // coprocessor instructions
  printf("mrc count: %08x\n", context->mrc);
  printf("mcr count: %08x\n", context->mcr);
  // misc/system instructions
  printf("svc count: %08x\n", context->svc);
  printf("breakpoint count: %08x\n", context->breakpoint);
  printf("cps count: %08x\n", context->cps);
  printf("mrs count: %08x\n", context->mrs);
  printf("msrImm count: %08x\n", context->msrImm);
  printf("msrReg count: %08x\n", context->msrReg);
  printf("wfi count: %08x\n", context->wfi);
  // load instructions
  printf("ldrbImm count: %08x\n", context->ldrbImm);
  printf("ldrbReg count: %08x\n", context->ldrbReg);
  printf("ldrbtImm count: %08x\n", context->ldrbtImm);
  printf("ldrbtReg count: %08x\n", context->ldrbtReg);
  printf("ldrhImm count: %08x\n", context->ldrhImm);
  printf("ldrhReg count: %08x\n", context->ldrhReg);
  printf("ldrhtImm count: %08x\n", context->ldrhtImm);
  printf("ldrhtReg count: %08x\n", context->ldrhtReg);
  printf("ldrImm count: %08x\n", context->ldrImm);
  printf("ldrReg count: %08x\n", context->ldrReg);
  printf("ldrtImm count: %08x\n", context->ldrtImm);
  printf("ldrtReg count: %08x\n", context->ldrtReg);
  printf("ldrdImm count: %08x\n", context->ldrdImm);
  printf("ldrdReg count: %08x\n", context->ldrdReg);
  printf("ldm count: %08x\n", context->ldm);
  printf("ldmUser count: %08x\n", context->ldmUser);
  printf("ldmExceptionReturn count: %08x\n", context->ldmExceptionReturn);
  // store instructions
  printf("strbImm count: %08x\n", context->strbImm);
  printf("strbReg count: %08x\n", context->strbReg);
  printf("strbtImm count: %08x\n", context->strbtImm);
  printf("strbtReg count: %08x\n", context->strbtReg);
  printf("strhImm count: %08x\n", context->strhImm);
  printf("strhReg count: %08x\n", context->strhReg);
  printf("strhtImm count: %08x\n", context->strhtImm);
  printf("strhtReg count: %08x\n", context->strhtReg);
  printf("strImm count: %08x\n", context->strImm);
  printf("strReg count: %08x\n", context->strReg);
  printf("strtImm count: %08x\n", context->strtImm);
  printf("strtReg count: %08x\n", context->strtReg);
  printf("strdImm count: %08x\n", context->strdImm);
  printf("strdReg count: %08x\n", context->strdReg);
  printf("stm count: %08x\n", context->stm);
  printf("stmUser count: %08x\n", context->stmUser);
  // sync instructions
  printf("clrex count: %08x\n", context->clrex);
  printf("ldrex count: %08x\n", context->ldrex);
  printf("ldrexb count: %08x\n", context->ldrexb);
  printf("ldrexh count: %08x\n", context->ldrexh);
  printf("ldrexd count: %08x\n", context->ldrexd);
  printf("strex count: %08x\n", context->strex);
  printf("strexb count: %08x\n", context->strexb);
  printf("strexh count: %08x\n", context->strexh);
  printf("strexd count: %08x\n", context->strexd);
  // branches
  printf("armBInstruction: %08x\n", context->armBInstruction);
  printf("armBxInstruction: %08x\n", context->armBxInstruction);
  printf("armBxjInstruction: %08x\n", context->armBxjInstruction);
  printf("armBlxRegisterInstruction: %08x\n", context->armBlxRegisterInstruction);
  printf("armBlxImmediateInstruction: %08x\n", context->armBlxImmediateInstruction);
  printf("====== OF THESE, BRANCHES WERE: ==========\n");
  printf("branchLink: %08x\n", context->branchLink);
  printf("branchNonlink: %08x\n", context->branchNonlink);
  printf("branchConditional: %08x\n", context->branchConditional);
  printf("branchNonconditional: %08x\n", context->branchNonconditional);
  printf("branchImmediate: %08x\n", context->branchImmediate);
  printf("branchRegister: %08x\n", context->branchRegister);
  printf("===========================================\n");
  printf("dabt count: %08x\n", context->dabtCount);
  printf("dabtPriv: %08x\n", context->dabtPriv);
  printf("dabtUser: %08x\n", context->dabtUser);
  printf("====================================\n");
  printf("pabt count: %08x\n", context->pabtCount);
  printf("pabtPriv: %08x\n", context->pabtPriv);
  printf("pabtUser: %08x\n", context->pabtUser);
  printf("====================================\n");
  printf("irq  count: %08x\n", context->irqCount);
  printf("irqPriv: %08x\n", context->irqPriv);
  printf("irqUser: %08x\n", context->irqUser);
  printf("====================================\n");
}
