#include "instructionEmu/interpreter.h"

#include "perf/contextSwitchCounters.h"


void initCounters(PerfCounters* counters)
{
#ifdef CONFIG_COUNT_ALU
  // ALU exception returns
  // counters->addExceptionReturn = 0;
  counters->movExceptionReturn = 0;
  // ALU computed jumps
  counters->addJump = 0;
  // counters->andJump = 0;
  counters->movJump = 0;
#endif

#ifdef CONFIG_COUNT_BRANCH
  // branches
  counters->branchLink = 0;
  counters->branchNonlink = 0;
  counters->branchConditional = 0;
  counters->branchNonconditional = 0;
  counters->branchImmediate = 0;
  counters->branchRegister = 0;
  counters->bInstruction = 0;
  counters->blInstruction = 0;
  counters->bxInstruction = 0;
  counters->bxjInstruction = 0;
  counters->blxRegisterInstruction = 0;
  counters->blxImmediateInstruction = 0;
#endif

  // coprocessor instructions
#ifdef CONFIG_COUNT_MCR
  counters->mcr = 0;
#endif
#ifdef CONFIG_COUNT_MRC
  counters->mrc = 0;
#endif

  // misc/system instructions
#ifdef CONFIG_COUNT_SVC
  counters->svc = 0;
#endif
#ifdef CONFIG_COUNT_BKPT
  counters->breakpoint = 0;
#endif
#ifdef CONFIG_COUNT_CPS
  counters->cps = 0;
#endif
#ifdef CONFIG_COUNT_MRS
  counters->mrs = 0;
#endif
#ifdef CONFIG_COUNT_MSR
  counters->msrImm = 0;
  counters->msrReg = 0;
#endif
#ifdef CONFIG_COUNT_WFI
  counters->wfi = 0;
#endif

  // load instructions
#ifdef CONFIG_COUNT_LDRB
  counters->ldrbImm = 0;
  counters->ldrbReg = 0;
#endif
#ifdef CONFIG_COUNT_LDRBT
  counters->ldrbtImm = 0;
  counters->ldrbtReg = 0;
#endif
#ifdef CONFIG_COUNT_LDRH
  counters->ldrhImm = 0;
  counters->ldrhReg = 0;
#endif
#ifdef CONFIG_COUNT_LDRHT
  counters->ldrhtImm = 0;
  counters->ldrhtReg = 0;
#endif
#ifdef CONFIG_COUNT_LDR
  counters->ldrImm = 0;
  counters->ldrReg = 0;
#endif
#ifdef CONFIG_COUNT_LDRT
  counters->ldrtImm = 0;
  counters->ldrtReg = 0;
#endif
#ifdef CONFIG_COUNT_LDRD
  counters->ldrdImm = 0;
  counters->ldrdReg = 0;
#endif
#ifdef CONFIG_COUNT_LDM
  counters->ldm = 0;
  counters->ldmUser = 0;
  counters->ldmExceptionReturn = 0;
#endif

  // store instructions
#ifdef CONFIG_COUNT_STRB
  counters->strbImm = 0;
  counters->strbReg = 0;
#endif
#ifdef CONFIG_COUNT_STRBT
  counters->strbtImm = 0;
  counters->strbtReg = 0;
#endif
#ifdef CONFIG_COUNT_STRH
  counters->strhImm = 0;
  counters->strhReg = 0;
#endif
#ifdef CONFIG_COUNT_STRHT
  counters->strhtImm = 0;
  counters->strhtReg = 0;
#endif
#ifdef CONFIG_COUNT_STR
  counters->strImm = 0;
  counters->strReg = 0;
#endif
#ifdef CONFIG_COUNT_STRT
  counters->strtImm = 0;
  counters->strtReg = 0;
#endif
#ifdef CONFIG_COUNT_STRD
  counters->strdImm = 0;
  counters->strdReg = 0;
#endif
#ifdef CONFIG_COUNT_STM
  counters->stm = 0;
  counters->stmUser = 0;
#endif

  // sync instructions
#ifdef CONFIG_COUNT_CLREX
  counters->clrex = 0;
#endif
#ifdef CONFIG_COUNT_LDREX
  counters->ldrex = 0;
  counters->ldrexb = 0;
  counters->ldrexh = 0;
  counters->ldrexd = 0;
#endif
#ifdef CONFIG_COUNT_STREX
  counters->strex = 0;
  counters->strexb = 0;
  counters->strexh = 0;
  counters->strexd = 0;
#endif


#ifdef CONFIG_REGISTER_SVC
  counters->svcCount = 0;
#endif
#ifdef CONFIG_REGISTER_DABT
  counters->dabtCount = 0;
  counters->dabtPriv = 0;
  counters->dabtUser = 0;
#endif
#ifdef CONFIG_REGISTER_PABT
  counters->pabtCount = 0;
  counters->pabtPriv = 0;
  counters->pabtUser = 0;
#endif
#ifdef CONFIG_REGISTER_IRQ
  counters->irqCount = 0;
  counters->irqPriv = 0;
  counters->irqUser = 0;
#endif
}


void resetExceptionCounters(PerfCounters* counters)
{
  initCounters(counters);
}


void dumpCounters(const PerfCounters* counters)
{
  printf("=================== Counters =====================\n");
#ifdef CONFIG_COUNT_ALU
  // ALU exception return
  // printf("addExceptionReturn count: %08x\n", counters->addExceptionReturn);
  printf("movExceptionReturn count: %08x\n", counters->movExceptionReturn);
  // ALU computed jumps
  printf("addJump count: %08x\n", counters->addJump);
  // printf("andJump count: %08x\n", counters->andJump);
  printf("movJump count: %08x\n", counters->movJump);
#endif

  // branches
#ifdef CONFIG_COUNT_BRANCH
  printf("bInstruction count: %08x\n", counters->bInstruction);
  printf("blInstruction count: %08x\n", counters->blInstruction);
  printf("bxInstruction count: %08x\n", counters->bxInstruction);
  printf("bxjInstruction count: %08x\n", counters->bxjInstruction);
  printf("blxRegisterInstruction count: %08x\n", counters->blxRegisterInstruction);
  printf("blxImmediateInstruction count: %08x\n", counters->blxImmediateInstruction);
  printf("====== OF THESE, BRANCHES WERE: ==========\n");
  printf("branchLink: %08x\n", counters->branchLink);
  printf("branchNonlink: %08x\n", counters->branchNonlink);
  printf("branchConditional: %08x\n", counters->branchConditional);
  printf("branchNonconditional: %08x\n", counters->branchNonconditional);
  printf("branchImmediate: %08x\n", counters->branchImmediate);
  printf("branchRegister: %08x\n", counters->branchRegister);
  printf("==========================================\n");
#endif

  // coprocessor instructions
#ifdef CONFIG_COUNT_MCR
  printf("mrc count: %08x\n", counters->mrc);
#endif
#ifdef CONFIG_COUNT_MRC
  printf("mcr count: %08x\n", counters->mcr);
#endif

  // misc/system instructions
#ifdef CONFIG_COUNT_SVC
  printf("svc count: %08x\n", counters->svc);
#endif
#ifdef CONFIG_COUNT_BKPT
  printf("breakpoint count: %08x\n", counters->breakpoint);
#endif
#ifdef CONFIG_COUNT_CPS
  printf("cps count: %08x\n", counters->cps);
#endif
#ifdef CONFIG_COUNT_MRS
  printf("mrs count: %08x\n", counters->mrs);
#endif
#ifdef CONFIG_COUNT_MSR
  printf("msrImm count: %08x\n", counters->msrImm);
  printf("msrReg count: %08x\n", counters->msrReg);
#endif
#ifdef CONFIG_COUNT_WFI
  printf("wfi count: %08x\n", counters->wfi);
#endif

  // load instructions
#ifdef CONFIG_COUNT_LDRB
  printf("ldrbImm count: %08x\n", counters->ldrbImm);
  printf("ldrbReg count: %08x\n", counters->ldrbReg);
#endif
#ifdef CONFIG_COUNT_LDRBT
  printf("ldrbtImm count: %08x\n", counters->ldrbtImm);
  printf("ldrbtReg count: %08x\n", counters->ldrbtReg);
#endif
#ifdef CONFIG_COUNT_LDRH
  printf("ldrhImm count: %08x\n", counters->ldrhImm);
  printf("ldrhReg count: %08x\n", counters->ldrhReg);
#endif
#ifdef CONFIG_COUNT_LDRHT
  printf("ldrhtImm count: %08x\n", counters->ldrhtImm);
  printf("ldrhtReg count: %08x\n", counters->ldrhtReg);
#endif
#ifdef CONFIG_COUNT_LDR
  printf("ldrImm count: %08x\n", counters->ldrImm);
  printf("ldrReg count: %08x\n", counters->ldrReg);
#endif
#ifdef CONFIG_COUNT_LDRT
  printf("ldrtImm count: %08x\n", counters->ldrtImm);
  printf("ldrtReg count: %08x\n", counters->ldrtReg);
#endif
#ifdef CONFIG_COUNT_LDRD
  printf("ldrdImm count: %08x\n", counters->ldrdImm);
  printf("ldrdReg count: %08x\n", counters->ldrdReg);
#endif
#ifdef CONFIG_COUNT_LDM
  printf("ldm count: %08x\n", counters->ldm);
  printf("ldmUser count: %08x\n", counters->ldmUser);
  printf("ldmExceptionReturn count: %08x\n", counters->ldmExceptionReturn);
#endif

  // store instructions
#ifdef CONFIG_COUNT_STRB
  printf("strbImm count: %08x\n", counters->strbImm);
  printf("strbReg count: %08x\n", counters->strbReg);
#endif
#ifdef CONFIG_COUNT_STRBT
  printf("strbtImm count: %08x\n", counters->strbtImm);
  printf("strbtReg count: %08x\n", counters->strbtReg);
#endif
#ifdef CONFIG_COUNT_STRH
  printf("strhImm count: %08x\n", counters->strhImm);
  printf("strhReg count: %08x\n", counters->strhReg);
#endif
#ifdef CONFIG_COUNT_STRHT
  printf("strhtImm count: %08x\n", counters->strhtImm);
  printf("strhtReg count: %08x\n", counters->strhtReg);
#endif
#ifdef CONFIG_COUNT_STR
  printf("strImm count: %08x\n", counters->strImm);
  printf("strReg count: %08x\n", counters->strReg);
#endif
#ifdef CONFIG_COUNT_STRT
  printf("strtImm count: %08x\n", counters->strtImm);
  printf("strtReg count: %08x\n", counters->strtReg);
#endif
#ifdef CONFIG_COUNT_STRD
  printf("strdImm count: %08x\n", counters->strdImm);
  printf("strdReg count: %08x\n", counters->strdReg);
#endif
#ifdef CONFIG_COUNT_STM
  printf("stm count: %08x\n", counters->stm);
  printf("stmUser count: %08x\n", counters->stmUser);
#endif

  // sync instructions
#ifdef CONFIG_COUNT_CLREX
  printf("clrex count: %08x\n", counters->clrex);
#endif
#ifdef CONFIG_COUNT_LDREX
  printf("ldrex count: %08x\n", counters->ldrex);
  printf("ldrexb count: %08x\n", counters->ldrexb);
  printf("ldrexh count: %08x\n", counters->ldrexh);
  printf("ldrexd count: %08x\n", counters->ldrexd);
#endif
#ifdef CONFIG_COUNT_STREX
  printf("strex count: %08x\n", counters->strex);
  printf("strexb count: %08x\n", counters->strexb);
  printf("strexh count: %08x\n", counters->strexh);
  printf("strexd count: %08x\n", counters->strexd);
#endif


#ifdef CONFIG_REGISTER_SVC
  printf("====================================\n");
  printf("svc  count: %08x\n", counters->svcCount);
#endif
#ifdef CONFIG_REGISTER_DABT
  printf("===========================================\n");
  printf("dabt count: %08x\n", counters->dabtCount);
  printf("dabtPriv: %08x\n", counters->dabtPriv);
  printf("dabtUser: %08x\n", counters->dabtUser);
#endif
#ifdef CONFIG_REGISTER_PABT
  printf("====================================\n");
  printf("pabt count: %08x\n", counters->pabtCount);
  printf("pabtPriv: %08x\n", counters->pabtPriv);
  printf("pabtUser: %08x\n", counters->pabtUser);
#endif
#ifdef CONFIG_REGISTER_IRQ
  printf("====================================\n");
  printf("irq  count: %08x\n", counters->irqCount);
  printf("irqPriv: %08x\n", counters->irqPriv);
  printf("irqUser: %08x\n", counters->irqUser);
#endif
  printf("====================================\n");
}
