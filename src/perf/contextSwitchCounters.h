#ifndef __INSTRUCTION_EMU__CONTEXT_SWITCH_COUNTERS_H__
#define __INSTRUCTION_EMU__CONTEXT_SWITCH_COUNTERS_H__

#include "guestManager/guestContext.h"

#include "instructionEmu/decoder/arm/structs.h"


#ifdef CONFIG_CONTEXT_SWITCH_COUNTERS
void dumpCounters(const GCONTXT* context);

void countAluInstruction(GCONTXT* context, Instruction instr);
void countBreakpoint(GCONTXT* context);
void countBranch(GCONTXT* context, Instruction instr);
void countBL(GCONTXT* context, Instruction instr);
void countBLXreg(GCONTXT* context, Instruction instr);
void countBX(GCONTXT* context, Instruction instr);
void countClrex(GCONTXT* context);
void countCps(GCONTXT* context, Instruction instr);
void countLdrbImm(GCONTXT* context);
void countLdrbReg(GCONTXT* context);
void countLdrbtImm(GCONTXT* context);
void countLdrbtReg(GCONTXT* context);
void countLdrhImm(GCONTXT* context);
void countLdrhReg(GCONTXT* context);
void countLdrhtImm(GCONTXT* context);
void countLdrhtReg(GCONTXT* context);
void countLdrImm(GCONTXT* context);
void countLdrReg(GCONTXT* context);
void countLdrtImm(GCONTXT* context);
void countLdrtReg(GCONTXT* context);
void countLdrdImm(GCONTXT* context);
void countLdrdReg(GCONTXT* context);
void countLdm(GCONTXT* context);
void countLdmUser(GCONTXT* context);
void countLdmExceptionReturn(GCONTXT* context);
void countLdrex(GCONTXT* context);
void countLdrexb(GCONTXT* context);
void countLdrexh(GCONTXT* context);
void countLdrexd(GCONTXT* context);
void countMcr(GCONTXT* context);
void countMrc(GCONTXT* context);
void countMrs(GCONTXT* context);
void countMsrImm(GCONTXT* context);
void countMsrReg(GCONTXT* context);
void countStrbImm(GCONTXT* context);
void countStrbReg(GCONTXT* context);
void countStrbtImm(GCONTXT* context);
void countStrbtReg(GCONTXT* context);
void countStrhImm(GCONTXT* context);
void countStrhReg(GCONTXT* context);
void countStrhtImm(GCONTXT* context);
void countStrhtReg(GCONTXT* context);
void countStrImm(GCONTXT* context);
void countStrReg(GCONTXT* context);
void countStrtImm(GCONTXT* context);
void countStrtReg(GCONTXT* context);
void countStrdImm(GCONTXT* context);
void countStrdReg(GCONTXT* context);
void countStm(GCONTXT* context);
void countStmUser(GCONTXT* context);
void countStrex(GCONTXT* context);
void countStrexb(GCONTXT* context);
void countStrexh(GCONTXT* context);
void countStrexd(GCONTXT* context);
void countSvc(GCONTXT* context);
void countWfi(GCONTXT* context);

void registerSvc(GCONTXT* context);
void registerDabt(GCONTXT* context, bool user);
void registerPabt(GCONTXT* context, bool user);
void registerIrq(GCONTXT* context, bool user);


__macro__ void countAluInstruction(GCONTXT* context, Instruction instr)
{
  if (instr.aluReg.S) // S bit in reg/imm the same place
  {
    switch (instr.aluReg.opc1)
    {
      case ADD: context->addExceptionReturn++; break;
    }
  }
  else
  {
    switch (instr.aluReg.opc1)
    {
      case AND: context->andJump++; break;
      case ADD: context->addJump++; break;
      case MOV: context->movJump++; break;
    }
  }
}


__macro__ void countBreakpoint(GCONTXT* context)
{
  context->breakpoint++;
}


__macro__ void countBranch(GCONTXT* context, Instruction instr)
{
  if (instr.branch.cc == AL)
  {
    context->branchNonconditional++;
  }
  else
  {
    context->branchConditional++;
  }
  context->branchImmediate++;
  context->branchNonlink++;
}


__macro__ void countBL(GCONTXT* context, Instruction instr)
{
  if (instr.branch.cc == AL)
  {
    context->branchNonconditional++;
  }
  else
  {
    context->branchConditional++;
  }
  context->branchImmediate++;
  context->branchLink++;
}


__macro__ void countBLXreg(GCONTXT* context, Instruction instr)
{
  if (instr.BxReg.cc == AL)
  {
    context->branchNonconditional++;
  }
  else
  {
    context->branchConditional++;
  }
  context->branchRegister++;
  context->branchLink++;
}


__macro__ void countBX(GCONTXT* context, Instruction instr)
{
  if (instr.BxReg.cc == AL)
  {
    context->branchNonconditional++;
  }
  else
  {
    context->branchConditional++;
  }
  context->branchRegister++;
  context->branchNonlink++;
}


__macro__ void countClrex(GCONTXT* context)
{
  context->clrex++;
}


__macro__ void countCps(GCONTXT* context, Instruction instr)
{
  context->cps++;
}


__macro__ void countLdrbImm(GCONTXT* context)
{
  context->ldrbImm++;
}


__macro__ void countLdrbReg(GCONTXT* context)
{
  context->ldrbReg++;
}


__macro__ void countLdrbtImm(GCONTXT* context)
{
  context->ldrbtImm++;
}


__macro__ void countLdrbtReg(GCONTXT* context)
{
  context->ldrbtReg++;
}


__macro__ void countLdrhImm(GCONTXT* context)
{
  context->ldrhImm++;
}


__macro__ void countLdrhReg(GCONTXT* context)
{
  context->ldrhReg++;
}


__macro__ void countLdrhtImm(GCONTXT* context)
{
  context->ldrhtImm++;
}


__macro__ void countLdrhtReg(GCONTXT* context)
{
  context->ldrhtReg++;
}


__macro__ void countLdrImm(GCONTXT* context)
{
  context->ldrImm++;
}


__macro__ void countLdrReg(GCONTXT* context)
{
  context->ldrReg++;
}


__macro__ void countLdrtImm(GCONTXT* context)
{
  context->ldrtImm++;
}


__macro__ void countLdrtReg(GCONTXT* context)
{
  context->ldrtReg++;
}


__macro__ void countLdrdImm(GCONTXT* context)
{
  context->ldrImm++;
}


__macro__ void countLdrdReg(GCONTXT* context)
{
  context->ldrReg++;
}


__macro__ void countLdm(GCONTXT* context)
{
  context->ldm++;
}


__macro__ void countLdmUser(GCONTXT* context)
{
  context->ldmUser++;
}


__macro__ void countLdmExceptionReturn(GCONTXT* context)
{
  context->ldmExceptionReturn++;
}


__macro__ void countLdrex(GCONTXT* context)
{
  context->ldrex++;
}


__macro__ void countLdrexb(GCONTXT* context)
{
  context->ldrexb++;
}


__macro__ void countLdrexh(GCONTXT* context)
{
  context->ldrexh++;
}


__macro__ void countLdrexd(GCONTXT* context)
{
  context->ldrexd++;
}



__macro__ void countMrs(GCONTXT* context)
{
  context->mrs++;
}


__macro__ void countMcr(GCONTXT* context)
{
  context->mcr++;
}


__macro__ void countMrc(GCONTXT* context)
{
  context->mrc++;
}


__macro__ void countMsrImm(GCONTXT* context)
{
  context->msrImm++;
}


__macro__ void countMsrReg(GCONTXT* context)
{
  context->msrReg++;
}


__macro__ void countStrbImm(GCONTXT* context)
{
  context->strbImm++;
}


__macro__ void countStrbReg(GCONTXT* context)
{
  context->strbReg++;
}


__macro__ void countStrbtImm(GCONTXT* context)
{
  context->strbtImm++;
}


__macro__ void countStrbtReg(GCONTXT* context)
{
  context->strbtReg++;
}


__macro__ void countStrhImm(GCONTXT* context)
{
  context->strhImm++;
}


__macro__ void countStrhReg(GCONTXT* context)
{
  context->strhReg++;
}


__macro__ void countStrhtImm(GCONTXT* context)
{
  context->strhtImm++;
}


__macro__ void countStrhtReg(GCONTXT* context)
{
  context->strhtReg++;
}


__macro__ void countStrImm(GCONTXT* context)
{
  context->strImm++;
}


__macro__ void countStrReg(GCONTXT* context)
{
  context->strReg++;
}


__macro__ void countStrtImm(GCONTXT* context)
{
  context->strtImm++;
}


__macro__ void countStrtReg(GCONTXT* context)
{
  context->strtReg++;
}


__macro__ void countStrdImm(GCONTXT* context)
{
  context->strdImm++;
}


__macro__ void countStrdReg(GCONTXT* context)
{
  context->strdReg++;
}


__macro__ void countStm(GCONTXT* context)
{
  context->stm++;
}


__macro__ void countStmUser(GCONTXT* context)
{
  context->stmUser++;
}


__macro__ void countStrex(GCONTXT* context)
{
  context->strex++;
}


__macro__ void countStrexb(GCONTXT* context)
{
  context->strexb++;
}


__macro__ void countStrexh(GCONTXT* context)
{
  context->strexh++;
}


__macro__ void countStrexd(GCONTXT* context)
{
  context->strexd++;
}


__macro__ void countSvc(GCONTXT* context)
{
  context->svc++;
}


__macro__ void countWfi(GCONTXT* context)
{
  context->wfi++;
}


__macro__ void registerSvc(GCONTXT* context)
{
  context->svcCount++;
}


__macro__ void registerDabt(GCONTXT* context, bool user)
{
  context->dabtCount++;
  if (user)
    context->dabtUser++;
  else
    context->dabtPriv++;
}


__macro__ void registerPabt(GCONTXT* context, bool user)
{
  context->pabtCount++;
  if (user)
    context->pabtUser++;
  else
    context->pabtPriv++;
}


__macro__ void registerIrq(GCONTXT* context, bool user)
{
  context->irqCount++;
  if (user)
    context->irqUser++;
  else
    context->irqPriv++;
}
#else
#define dumpCounters(context);

#define countAluInstruction(context, instr);
#define countBreakpoint(context);
#define countBranch(context, instr);
#define countBL(context, instr);
#define countBLXreg(context, instr);
#define countBX(context, instr);
#define countClrex(context);
#define countCps(context, instr);
#define countLdrbImm(context);
#define countLdrbReg(context);
#define countLdrbtImm(context);
#define countLdrbtReg(context);
#define countLdrhImm(context);
#define countLdrhReg(context);
#define countLdrhtImm(context);
#define countLdrhtReg(context);
#define countLdrImm(context);
#define countLdrReg(context);
#define countLdrtImm(context);
#define countLdrtReg(context);
#define countLdrdImm(context);
#define countLdrdReg(context);
#define countLdm(context);
#define countLdmUser(context);
#define countLdmExceptionReturn(context);
#define countLdrex(context);
#define countLdrexb(context);
#define countLdrexh(context);
#define countLdrexd(context);
#define countMcr(context);
#define countMrc(context);
#define countMrs(context);
#define countMsrImm(context);
#define countMsrReg(context);
#define countStrbImm(context);
#define countStrbReg(context);
#define countStrbtImm(context);
#define countStrbtReg(context);
#define countStrhImm(context);
#define countStrhReg(context);
#define countStrhtImm(context);
#define countStrhtReg(context);
#define countStrImm(context);
#define countStrReg(context);
#define countStrtImm(context);
#define countStrtReg(context);
#define countStrdImm(context);
#define countStrdReg(context);
#define countStm(context);
#define countStmUser(context);
#define countStrex(context);
#define countStrexb(context);
#define countStrexh(context);
#define countStrexd(context);
#define countSvc(context);
#define countWfi(context);

#define registerSvc(context);
#define registerDabt(context, user);
#define registerPabt(context, user);
#define registerIrq(context, user);
#endif


#endif
