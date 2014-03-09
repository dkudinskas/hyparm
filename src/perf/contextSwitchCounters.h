#ifndef __INSTRUCTION_EMU__CONTEXT_SWITCH_COUNTERS_H__
#define __INSTRUCTION_EMU__CONTEXT_SWITCH_COUNTERS_H__

#include "instructionEmu/decoder/arm/structs.h"

typedef struct ContextSwitchCounters {
#ifdef CONFIG_COUNT_ALU
  // ALU exception returns
  // u32int addExceptionReturn;
  u32int movExceptionReturn;
  // ALU computed jumps
  u32int addJump;
  // u32int andJump;
  u32int movJump;
#endif

  // branches
#ifdef CONFIG_COUNT_BRANCH
  u32int branchLink;
  u32int branchNonlink;
  u32int branchConditional;
  u32int branchNonconditional;
  u32int branchImmediate;
  u32int branchRegister;

  u32int bInstruction;
  u32int blInstruction;
  u32int bxInstruction;
  u32int bxjInstruction;
  u32int blxRegisterInstruction;
  u32int blxImmediateInstruction;
#endif

  // coprocessor instructions
#ifdef CONFIG_COUNT_MCR
  u32int mrc;
#endif
#ifdef CONFIG_COUNT_MRC
  u32int mcr;
#endif

  // misc/system instructions
#ifdef CONFIG_COUNT_SVC
  u32int svc;
#endif
#ifdef CONFIG_COUNT_BKPT
  u32int breakpoint;
#endif
#ifdef CONFIG_COUNT_CPS
  u32int cps;
#endif
#ifdef CONFIG_COUNT_MRS
  u32int mrs;
#endif
#ifdef CONFIG_COUNT_MSR
  u32int msrImm;
  u32int msrReg;
#endif
#ifdef CONFIG_COUNT_WFI
  u32int wfi;
#endif

  // load instructions
#ifdef CONFIG_COUNT_LDRB
  u32int ldrbImm;
  u32int ldrbReg;
#endif
#ifdef CONFIG_COUNT_LDRBT
  u32int ldrbtImm;
  u32int ldrbtReg;
#endif
#ifdef CONFIG_COUNT_LDRH
  u32int ldrhImm;
  u32int ldrhReg;
#endif
#ifdef CONFIG_COUNT_LDRHT
  u32int ldrhtImm;
  u32int ldrhtReg;
#endif
#ifdef CONFIG_COUNT_LDR
  u32int ldrImm;
  u32int ldrReg;
#endif
#ifdef CONFIG_COUNT_LDRT
  u32int ldrtImm;
  u32int ldrtReg;
#endif
#ifdef CONFIG_COUNT_LDRD
  u32int ldrdImm;
  u32int ldrdReg;
#endif
#ifdef CONFIG_COUNT_LDM
  u32int ldm;
  u32int ldmUser;
  u32int ldmExceptionReturn;
#endif

  // store instructions
#ifdef CONFIG_COUNT_STRB
  u32int strbImm;
  u32int strbReg;
#endif
#ifdef CONFIG_COUNT_STRBT
  u32int strbtImm;
  u32int strbtReg;
#endif
#ifdef CONFIG_COUNT_STRH
  u32int strhImm;
  u32int strhReg;
#endif
#ifdef CONFIG_COUNT_STRHT
  u32int strhtImm;
  u32int strhtReg;
#endif
#ifdef CONFIG_COUNT_STR
  u32int strImm;
  u32int strReg;
#endif
#ifdef CONFIG_COUNT_STRT
  u32int strtImm;
  u32int strtReg;
#endif
#ifdef CONFIG_COUNT_STRD
  u32int strdImm;
  u32int strdReg;
#endif
#ifdef CONFIG_COUNT_STM
  u32int stm;
  u32int stmUser;
#endif

  // sync instructions
#ifdef CONFIG_COUNT_CLREX
  u32int clrex;
#endif
#ifdef CONFIG_COUNT_LDREX
  u32int ldrex;
  u32int ldrexb;
  u32int ldrexh;
  u32int ldrexd;
#endif
#ifdef CONFIG_COUNT_STREX
  u32int strex;
  u32int strexb;
  u32int strexh;
  u32int strexd;
#endif

#ifdef CONFIG_REGISTER_SVC
  u32int svcCount;
#endif
#ifdef CONFIG_REGISTER_DABT
  u32int dabtCount;
  u32int dabtPriv;
  u32int dabtUser;
#endif
#ifdef CONFIG_REGISTER_PABT
  u32int pabtCount;
  u32int pabtPriv;
  u32int pabtUser;
#endif
#ifdef CONFIG_REGISTER_IRQ
  u32int irqCount;
  u32int irqPriv;
  u32int irqUser;
#endif
} PerfCounters;


void initCounters(PerfCounters* counters);
void resetExceptionCounters(PerfCounters* counters);
void dumpCounters(const PerfCounters* counters);


#ifdef CONFIG_COUNT_ALU
void countAluInstruction(PerfCounters* counters, Instruction instr);
__macro__ void countAluInstruction(PerfCounters* counters, Instruction instr)
{
  if (instr.aluReg.S) // S bit in reg/imm the same place
  {
    switch (instr.aluReg.opc1)
    {
      // case ADD: counters->addExceptionReturn++; break;
      case MOV: counters->movExceptionReturn++; break;
      default:
        printf("instr %08x opc1 %x\n", instr.raw, instr.aluReg.opc1);
        DIE_NOW(0, "unknown alu exc ret op\n");
    }
  }
  else
  {
    switch (instr.aluReg.opc1)
    {
      // case AND: counters->andJump++; break;
      case ADD: counters->addJump++; break;
      case MOV: counters->movJump++; break;
      default:
        printf("instr %08x opc1 %x\n", instr.raw, instr.aluReg.opc1);
        DIE_NOW(0, "unknown alu computed jump op\n");
    }
  }
}
#else
#define countAluInstruction(counters, instr);
#endif


#ifdef CONFIG_COUNT_BKPT
void countBreakpoint(PerfCounters* counters);
__macro__ void countBreakpoint(PerfCounters* counters)
{
  counters->breakpoint++;
}
#else
#define countBreakpoint(counters);
#endif


#ifdef CONFIG_COUNT_BRANCH
void countBranch(PerfCounters* counters, Instruction instr);
__macro__ void countBranch(PerfCounters* counters, Instruction instr)
{
  if (instr.branch.cc == AL)
  {
    counters->branchNonconditional++;
  }
  else
  {
    counters->branchConditional++;
  }
  counters->branchImmediate++;
  counters->branchNonlink++;

  counters->bInstruction++;
}

void countBL(PerfCounters* counters, Instruction instr);
__macro__ void countBL(PerfCounters* counters, Instruction instr)
{
  if (instr.branch.cc == AL)
  {
    counters->branchNonconditional++;
  }
  else
  {
    counters->branchConditional++;
  }
  counters->branchImmediate++;
  counters->branchLink++;

  counters->blInstruction++;
}

void countBLXreg(PerfCounters* counters, Instruction instr);
__macro__ void countBLXreg(PerfCounters* counters, Instruction instr)
{
  if (instr.BxReg.cc == AL)
  {
    counters->branchNonconditional++;
  }
  else
  {
    counters->branchConditional++;
  }
  counters->branchRegister++;
  counters->branchLink++;

  counters->blxRegisterInstruction++;
}

void countBX(PerfCounters* counters, Instruction instr);
__macro__ void countBX(PerfCounters* counters, Instruction instr)
{
  if (instr.BxReg.cc == AL)
  {
    counters->branchNonconditional++;
  }
  else
  {
    counters->branchConditional++;
  }
  counters->branchRegister++;
  counters->branchNonlink++;

  counters->bxInstruction++;
}
#else
#define countBranch(counters, instr);
#define countBL(counters, instr);
#define countBLXreg(counters, instr);
#define countBX(counters, instr);
#endif


#ifdef CONFIG_COUNT_CLREX
void countClrex(PerfCounters* counters);
__macro__ void countClrex(PerfCounters* counters)
{
  counters->clrex++;
}
#else
#define countClrex(counters);
#endif


#ifdef CONFIG_COUNT_CPS
void countCps(PerfCounters* counters, Instruction instr);
__macro__ void countCps(PerfCounters* counters, Instruction instr)
{
  counters->cps++;
}
#else
#define countCps(counters, instr);
#endif


#ifdef CONFIG_COUNT_LDRB
void countLdrbImm(PerfCounters* counters);
__macro__ void countLdrbImm(PerfCounters* counters)
{
  counters->ldrbImm++;
}

void countLdrbReg(PerfCounters* counters);
__macro__ void countLdrbReg(PerfCounters* counters)
{
  counters->ldrbReg++;
}
#else
#define countLdrbImm(counters);
#define countLdrbReg(counters);
#endif


#ifdef CONFIG_COUNT_LDRBT
void countLdrbtImm(PerfCounters* counters);
__macro__ void countLdrbtImm(PerfCounters* counters)
{
  counters->ldrbtImm++;
}

void countLdrbtReg(PerfCounters* counters);
__macro__ void countLdrbtReg(PerfCounters* counters)
{
  counters->ldrbtReg++;
}
#else
#define countLdrbtImm(counters);
#define countLdrbtReg(counters);
#endif


#ifdef CONFIG_COUNT_LDRH
void countLdrhImm(PerfCounters* counters);
__macro__ void countLdrhImm(PerfCounters* counters)
{
  counters->ldrhImm++;
}

void countLdrhReg(PerfCounters* counters);
__macro__ void countLdrhReg(PerfCounters* counters)
{
  counters->ldrhReg++;
}
#else
#define countLdrhImm(counters);
#define countLdrhReg(counters);
#endif


#ifdef CONFIG_COUNT_LDRHT
void countLdrhtImm(PerfCounters* counters);
__macro__ void countLdrhtImm(PerfCounters* counters)
{
  counters->ldrhtImm++;
}

void countLdrhtReg(PerfCounters* counters);
__macro__ void countLdrhtReg(PerfCounters* counters)
{
  counters->ldrhtReg++;
}
#else
#define countLdrhtImm(counters);
#define countLdrhtReg(counters);
#endif


#ifdef CONFIG_COUNT_LDR
void countLdrImm(PerfCounters* counters);
__macro__ void countLdrImm(PerfCounters* counters)
{
  counters->ldrImm++;
}

void countLdrReg(PerfCounters* counters);
__macro__ void countLdrReg(PerfCounters* counters)
{
  counters->ldrReg++;
}
#else
#define countLdrImm(counters);
#define countLdrReg(counters);
#endif


#ifdef CONFIG_COUNT_LDRT
void countLdrtImm(PerfCounters* counters);
__macro__ void countLdrtImm(PerfCounters* counters)
{
  counters->ldrtImm++;
}

void countLdrtReg(PerfCounters* counters);
__macro__ void countLdrtReg(PerfCounters* counters)
{
  counters->ldrtReg++;
}
#else
#define countLdrtImm(counters);
#define countLdrtReg(counters);
#endif


#ifdef CONFIG_COUNT_LDRD
void countLdrdImm(PerfCounters* counters);
__macro__ void countLdrdImm(PerfCounters* counters)
{
  counters->ldrdImm++;
}

void countLdrdReg(PerfCounters* counters);
__macro__ void countLdrdReg(PerfCounters* counters)
{
  counters->ldrdReg++;
}
#else
#define countLdrdImm(counters);
#define countLdrdReg(counters);
#endif


#ifdef CONFIG_COUNT_LDM
void countLdm(PerfCounters* counters);
__macro__ void countLdm(PerfCounters* counters)
{
  counters->ldm++;
}

void countLdmUser(PerfCounters* counters);
__macro__ void countLdmUser(PerfCounters* counters)
{
  counters->ldmUser++;
}

void countLdmExceptionReturn(PerfCounters* counters);
__macro__ void countLdmExceptionReturn(PerfCounters* counters)
{
  counters->ldmExceptionReturn++;
}
#else
#define countLdm(counters);
#define countLdmUser(counters);
#define countLdmExceptionReturn(counters);
#endif


#ifdef CONFIG_COUNT_LDREX
void countLdrex(PerfCounters* counters);
__macro__ void countLdrex(PerfCounters* counters)
{
  counters->ldrex++;
}

void countLdrexb(PerfCounters* counters);
__macro__ void countLdrexb(PerfCounters* counters)
{
  counters->ldrexb++;
}

void countLdrexh(PerfCounters* counters);
__macro__ void countLdrexh(PerfCounters* counters)
{
  counters->ldrexh++;
}

void countLdrexd(PerfCounters* counters);
__macro__ void countLdrexd(PerfCounters* counters)
{
  counters->ldrexd++;
}
#else
#define countLdrex(counters);
#define countLdrexb(counters);
#define countLdrexh(counters);
#define countLdrexd(counters);
#endif


#ifdef CONFIG_COUNT_MCR
void countMcr(PerfCounters* counters);
__macro__ void countMcr(PerfCounters* counters)
{
  counters->mcr++;
}
#else
#define countMcr(counters);
#endif


#ifdef CONFIG_COUNT_MRC
void countMrc(PerfCounters* counters);
__macro__ void countMrc(PerfCounters* counters)
{
  counters->mrc++;
}
#else
#define countMrc(counters);
#endif


#ifdef CONFIG_COUNT_MRS
void countMrs(PerfCounters* counters);
__macro__ void countMrs(PerfCounters* counters)
{
  counters->mrs++;
}
#else
#define countMrs(counters);
#endif


#ifdef CONFIG_COUNT_MSR
void countMsrImm(PerfCounters* counters);
__macro__ void countMsrImm(PerfCounters* counters)
{
  counters->msrImm++;
}

void countMsrReg(PerfCounters* counters);
__macro__ void countMsrReg(PerfCounters* counters)
{
  counters->msrReg++;
}
#else
#define countMsrImm(counters);
#define countMsrReg(counters);
#endif


#ifdef CONFIG_COUNT_STRB
void countStrbImm(PerfCounters* counters);
__macro__ void countStrbImm(PerfCounters* counters)
{
  counters->strbImm++;
}

void countStrbReg(PerfCounters* counters);
__macro__ void countStrbReg(PerfCounters* counters)
{
  counters->strbReg++;
}
#else
#define countStrbImm(counters);
#define countStrbReg(counters);
#endif


#ifdef CONFIG_COUNT_STRBT
void countStrbtImm(PerfCounters* counters);
__macro__ void countStrbtImm(PerfCounters* counters)
{
  counters->strbtImm++;
}

void countStrbtReg(PerfCounters* counters);
__macro__ void countStrbtReg(PerfCounters* counters)
{
  counters->strbtReg++;
}
#else
#define countStrbtImm(counters);
#define countStrbtReg(counters);
#endif


#ifdef CONFIG_COUNT_STRH
void countStrhImm(PerfCounters* counters);
__macro__ void countStrhImm(PerfCounters* counters)
{
  counters->strhImm++;
}

void countStrhReg(PerfCounters* counters);
__macro__ void countStrhReg(PerfCounters* counters)
{
  counters->strhReg++;
}
#else
#define countStrhImm(counters);
#define countStrhReg(counters);
#endif


#ifdef CONFIG_COUNT_STRHT
void countStrhtImm(PerfCounters* counters);
__macro__ void countStrhtImm(PerfCounters* counters)
{
  counters->strhtImm++;
}

void countStrhtReg(PerfCounters* counters);
__macro__ void countStrhtReg(PerfCounters* counters)
{
  counters->strhtReg++;
}
#else
#define countStrhtImm(counters);
#define countStrhtReg(counters);
#endif


#ifdef CONFIG_COUNT_STR
void countStrImm(PerfCounters* counters);
__macro__ void countStrImm(PerfCounters* counters)
{
  counters->strImm++;
}

void countStrReg(PerfCounters* counters);
__macro__ void countStrReg(PerfCounters* counters)
{
  counters->strReg++;
}
#else
#define countStrImm(counters);
#define countStrReg(counters);
#endif


#ifdef CONFIG_COUNT_STRT
void countStrtImm(PerfCounters* counters);
__macro__ void countStrtImm(PerfCounters* counters)
{
  counters->strtImm++;
}

void countStrtReg(PerfCounters* counters);
__macro__ void countStrtReg(PerfCounters* counters)
{
  counters->strtReg++;
}
#else
#define countStrtImm(counters);
#define countStrtReg(counters);
#endif


#ifdef CONFIG_COUNT_STRD
void countStrdImm(PerfCounters* counters);
__macro__ void countStrdImm(PerfCounters* counters)
{
  counters->strdImm++;
}

void countStrdReg(PerfCounters* counters);
__macro__ void countStrdReg(PerfCounters* counters)
{
  counters->strdReg++;
}
#else
#define countStrdImm(counters);
#define countStrdReg(counters);
#endif


#ifdef CONFIG_COUNT_STM
void countStm(PerfCounters* counters);
__macro__ void countStm(PerfCounters* counters)
{
  counters->stm++;
}

void countStmUser(PerfCounters* counters);
__macro__ void countStmUser(PerfCounters* counters)
{
  counters->stmUser++;
}
#else
#define countStm(counters);
#define countStmUser(counters);
#endif


#ifdef CONFIG_COUNT_STREX
void countStrex(PerfCounters* counters);
__macro__ void countStrex(PerfCounters* counters)
{
  counters->strex++;
}

void countStrexb(PerfCounters* counters);
__macro__ void countStrexb(PerfCounters* counters)
{
  counters->strexb++;
}

void countStrexh(PerfCounters* counters);
__macro__ void countStrexh(PerfCounters* counters)
{
  counters->strexh++;
}

void countStrexd(PerfCounters* counters);
__macro__ void countStrexd(PerfCounters* counters)
{
  counters->strexd++;
}
#else
#define countStrex(counters);
#define countStrexb(counters);
#define countStrexh(counters);
#define countStrexd(counters);
#endif


#ifdef CONFIG_COUNT_SVC
void countSvc(PerfCounters* counters);
__macro__ void countSvc(PerfCounters* counters)
{
  counters->svc++;
}
#else
#define countSvc(counters);
#endif


#ifdef CONFIG_COUNT_WFI
void countWfi(PerfCounters* counters);
__macro__ void countWfi(PerfCounters* counters)
{
  counters->wfi++;
}
#else
#define countWfi(counters);
#endif


/************************ register amount totals ********************/

#ifdef CONFIG_REGISTER_SVC
void registerSvc(PerfCounters* counters);
__macro__ void registerSvc(PerfCounters* counters)
{
  counters->svcCount++;
}
#else
#define registerSvc(counters);
#endif


#ifdef CONFIG_REGISTER_DABT
void registerDabt(PerfCounters* counters, bool user);
__macro__ void registerDabt(PerfCounters* counters, bool user)
{
  counters->dabtCount++;
  if (user)
    counters->dabtUser++;
  else
    counters->dabtPriv++;
}
#else
#define registerDabt(counters, user);
#endif


#ifdef CONFIG_REGISTER_PABT
void registerPabt(PerfCounters* counters, bool user);
__macro__ void registerPabt(PerfCounters* counters, bool user)
{
  counters->pabtCount++;
  if (user)
    counters->pabtUser++;
  else
    counters->pabtPriv++;
}
#else
#define registerPabt(counters, user);
#endif


#ifdef CONFIG_REGISTER_IRQ
void registerIrq(PerfCounters* counters, bool user);
__macro__ void registerIrq(PerfCounters* counters, bool user)
{
  counters->irqCount++;
  if (user)
    counters->irqUser++;
  else
    counters->irqPriv++;
}
#else
#define registerIrq(counters, user);
#endif


#endif
