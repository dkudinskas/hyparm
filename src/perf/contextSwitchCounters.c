#include "perf/contextSwitchCounters.h"

void countBranch(GCONTXT *context, Instruction instr)
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

void countBL(GCONTXT* context, Instruction instr)
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

void countBLXreg(GCONTXT* context, Instruction instr)
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

void countBX(GCONTXT* context, Instruction instr)
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

void registerSvc(GCONTXT *context, InstructionHandler handler)
{
  if (handler == armStmInstruction)
  {
    context->armStmInstruction++;
  }
  else if (handler == armLdmInstruction)
  {
    context->armLdmInstruction++;
  }
  else if (handler == armLdrInstruction)
  {
    context->armLdrInstruction++;
  }
  else if (handler == armBInstruction)
  {
    context->armBInstruction++;
  }
  else if (handler == armBlInstruction)
  {
    context->armBlInstruction++;
  }
  else if (handler == armSwpInstruction)
  {
    context->armSwpInstruction++;
  }
  else if (handler == armLdrexbInstruction)
  {
    context->armLdrexbInstruction++;
  }
  else if (handler == armLdrexdInstruction)
  {
    context->armLdrexdInstruction++;
  }
  else if (handler == armLdrexhInstruction)
  {
    context->armLdrexhInstruction++;
  }
  else if (handler == armStrexbInstruction)
  {
    context->armStrexbInstruction++;
  }
  else if (handler == armStrexdInstruction)
  {
    context->armStrexdInstruction++;
  }
  else if (handler == armStrexhInstruction)
  {
    context->armStrexhInstruction++;
  }
  else if (handler == armLdrexInstruction)
  {
    context->armLdrexInstruction++;
  }
  else if (handler == armStrexInstruction)
  {
    context->armStrexInstruction++;
  }
  else if (handler == armBxInstruction)
  {
    context->armBxInstruction++;
  }
  else if (handler == armBxjInstruction)
  {
    context->armBxjInstruction++;
  }
  else if (handler == armBkptInstruction)
  {
    context->armBkptInstruction++;
  }
  else if (handler == armSmcInstruction)
  {
    context->armSmcInstruction++;
  }
  else if (handler == armBlxRegisterInstruction)
  {
    context->armBlxRegisterInstruction++;
  }
  else if (handler == armAndInstruction)
  {
    context->armAndInstruction++;
  }
  else if (handler == armEorInstruction)
  {
    context->armEorInstruction++;
  }
  else if (handler == armSubInstruction)
  {
    context->armSubInstruction++;
  }
  else if (handler == armAddInstruction)
  {
    context->armAddInstruction++;
  }
  else if (handler == armAdcInstruction)
  {
    context->armAdcInstruction++;
  }
  else if (handler == armSbcInstruction)
  {
    context->armSbcInstruction++;
  }
  else if (handler == armRscInstruction)
  {
    context->armRscInstruction++;
  }
  else if (handler == armMsrInstruction)
  {
    context->armMsrInstruction++;
  }
  else if (handler == armMrsInstruction)
  {
    context->armMrsInstruction++;
  }
  else if (handler == armOrrInstruction)
  {
    context->armOrrInstruction++;
  }
  else if (handler == armMovInstruction)
  {
    context->armMovInstruction++;
  }
  else if (handler == armLslInstruction)
  {
    context->armLslInstruction++;
  }
  else if (handler == armLsrInstruction)
  {
    context->armLsrInstruction++;
  }
  else if (handler == armAsrInstruction)
  {
    context->armAsrInstruction++;
  }
  else if (handler == armRrxInstruction)
  {
    context->armRrxInstruction++;
  }
  else if (handler == armRorInstruction)
  {
    context->armRorInstruction++;
  }
  else if (handler == armBicInstruction)
  {
    context->armBicInstruction++;
  }
  else if (handler == armMvnInstruction)
  {
    context->armMvnInstruction++;
  }
  else if (handler == armYieldInstruction)
  {
    context->armYieldInstruction++;
  }
  else if (handler == armWfeInstruction)
  {
    context->armWfeInstruction++;
  }
  else if (handler == armWfiInstruction)
  {
    context->armWfiInstruction++;
  }
  else if (handler == armSevInstruction)
  {
    context->armSevInstruction++;
  }
  else if (handler == armDbgInstruction)
  {
    context->armDbgInstruction++;
  }
  else if (handler == armMrcInstruction)
  {
    context->armMrcInstruction++;
  }
  else if (handler == armMcrInstruction)
  {
    context->armMcrInstruction++;
  }
  else if (handler == armDmbInstruction)
  {
    context->armDmbInstruction++;
  }
  else if (handler == armDsbInstruction)
  {
    context->armDsbInstruction++;
  }
  else if (handler == armIsbInstruction)
  {
    context->armIsbInstruction++;
  }
  else if (handler == armClrexInstruction)
  {
    context->armClrexInstruction++;
  }
  else if (handler == armCpsInstruction)
  {
    context->armCpsInstruction++;
  }
  else if (handler == armRfeInstruction)
  {
    context->armRfeInstruction++;
  }
  else if (handler == armSetendInstruction)
  {
    context->armSetendInstruction++;
  }
  else if (handler == armSrsInstruction)
  {
    context->armSrsInstruction++;
  }
  else if (handler == armBlxImmediateInstruction)
  {
    context->armBlxImmediateInstruction++;
  }
  else if (handler == armPldInstruction)
  {
    context->armPldInstruction++;
  }
  else if (handler == armPliInstruction)
  {
    context->armPliInstruction++;
  }
  else if (handler == armStrbtInstruction)
  {
    context->armStrbtInstruction++;
  }
  else if (handler == armStrhtInstruction)
  {
    context->armStrhtInstruction++;
  }
  else if (handler == armStrtInstruction)
  {
    context->armStrtInstruction++;
  }
  else if (handler == armLdrbtInstruction)
  {
    context->armLdrbtInstruction++;
  }
  else if (handler == armLdrhtInstruction)
  {
    context->armLdrhtInstruction++;
  }
  else if (handler == armLdrtInstruction)
  {
    context->armLdrtInstruction++;
  }
  else
  {
    printf("handler = %p", handler);
    DIE_NOW(context, "handler...");
  }
}
