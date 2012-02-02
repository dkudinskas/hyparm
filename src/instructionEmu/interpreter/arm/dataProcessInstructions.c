#include "instructionEmu/interpreter/internals.h"

#include "instructionEmu/interpreter/arm/dataProcessInstructions.h"


/*********************************/
/* ADD Rd, Rs, Rs2/imm, shiftAmt */
/*********************************/
u32int armAdcInstruction(GCONTXT *context, u32int instruction)
{
  DIE_NOW(context, "not implemented");
}

/*********************************/
/* ADD Rd, Rs, Rs2/imm, shiftAmt */
/*********************************/
u32int armAddInstruction(GCONTXT *context, u32int instruction)
{
  OPTYPE opType = ADD;
  return arithLogicOp(context, instruction, opType, __func__);
}

u32int armAdrInstruction(GCONTXT *context, u32int instruction)
{
  DIE_NOW(context, "not implemented");
}

/*********************************/
/* AND Rd, Rs, Rs2/imm, shiftAmt */
/*********************************/
u32int armAndInstruction(GCONTXT *context, u32int instruction)
{
  TRACE(context, instruction);
  DIE_NOW(context, "not implemented");
}

/*********************************/
/* ASR Rd, Rs                    */
/*********************************/
u32int armAsrInstruction(GCONTXT *context, u32int instruction)
{
  DIE_NOW(context, "not implemented");
}

/*********************************/
/* SBC Rd, Rs, Rs2/imm, shiftAmt */
/*********************************/
u32int armBicInstruction(GCONTXT *context, u32int instruction)
{
  DIE_NOW(context, "not implemented");
}

/*********************************/
/* CMN Rs, Rs2/imm               */
/*********************************/
u32int armCmnInstruction(GCONTXT *context, u32int instruction)
{
#ifdef CONFIG_BLOCK_COPY
  DIE_NOW(context, "cmnInstruction is executed but not yet checked for blockCopyCompatibility");
#else
  invalidDataProcTrap(context, instruction, __func__);
#endif
}

/*********************************/
/* CMP Rs, Rs2/imm               */
/*********************************/
u32int armCmpInstruction(GCONTXT *context, u32int instruction)
{
#ifdef CONFIG_BLOCK_COPY
  DIE_NOW(context, "cmpInstruction is executed but not yet checked for blockCopyCompatibility");
#else
  invalidDataProcTrap(context, instruction, __func__);
#endif
}

/*********************************/
/* ORR Rd, Rs, Rs2/imm, shiftAmt */
/*********************************/
u32int armEorInstruction(GCONTXT *context, u32int instruction)
{
  DIE_NOW(context, "not implemented");
}

/*********************************/
/* LSL Rd, Rs                    */
/*********************************/
u32int armLslInstruction(GCONTXT *context, u32int instruction)
{
  DIE_NOW(context, "not implemented");
}

/*********************************/
/* LSR Rd, Rs                    */
/*********************************/
u32int armLsrInstruction(GCONTXT *context, u32int instruction)
{
  DIE_NOW(context, "not implemented");
}

/*********************************/
/* MOV Rd, Rs                    */
/*********************************/
u32int armMovInstruction(GCONTXT *context, u32int instruction)
{
  OPTYPE opType = MOV;
  return arithLogicOp(context, instruction, opType, __func__);
}

u32int armMovtInstruction(GCONTXT *context, u32int instruction)
{
  DIE_NOW(context, "not implemented");
}

u32int armMovwInstruction(GCONTXT *context, u32int instruction)
{
  DIE_NOW(context, "not implemented");
}

/*********************************/
/* MVN Rd, Rs                    */
/*********************************/
u32int armMvnInstruction(GCONTXT *context, u32int instruction)
{
  DIE_NOW(context, "not implemented");
}

/*********************************/
/* ORR Rd, Rs, Rs2/imm, shiftAmt */
/*********************************/
u32int armOrrInstruction(GCONTXT *context, u32int instruction)
{
  DIE_NOW(context, "not implemented");
}

/*********************************/
/* ROR Rd, Rs                    */
/*********************************/
u32int armRorInstruction(GCONTXT *context, u32int instruction)
{
  DIE_NOW(context, "not implemented");
}

/*********************************/
/* RRX Rd, Rs                    */
/*********************************/
u32int armRrxInstruction(GCONTXT *context, u32int instruction)
{
  DIE_NOW(context, "not implemented");
}

/*********************************/
/* RSB Rd, Rs, Rs2/imm, shiftAmt */
/*********************************/
u32int armRsbInstruction(GCONTXT *context, u32int instruction)
{
  DIE_NOW(context, "not implemented");
}

/*********************************/
/* RSB Rd, Rs, Rs2/imm, shiftAmt */
/*********************************/
u32int armRscInstruction(GCONTXT *context, u32int instruction)
{
  DIE_NOW(context, "not implemented");
}

/*********************************/
/* SBC Rd, Rs, Rs2/imm, shiftAmt */
/*********************************/
u32int armSbcInstruction(GCONTXT *context, u32int instruction)
{
  DIE_NOW(context, "not implemented");
}

/*********************************/
/* SUB Rd, Rs, Rs2/imm, shiftAmt */
/*********************************/
u32int armSubInstruction(GCONTXT *context, u32int instruction)
{
#ifdef CONFIG_BLOCK_COPY
  DIE_NOW(context, "subInstruction is executed but not yet checked for blockCopyCompatibility");
#else
  OPTYPE opType = SUB;
  return arithLogicOp(context, instruction, opType, __func__);
#endif
}

/*********************************/
/* TEQ Rs, Rs2/imm               */
/*********************************/
u32int armTeqInstruction(GCONTXT *context, u32int instruction)
{
#ifdef CONFIG_BLOCK_COPY
  DIE_NOW(context, "teqInstruction is executed but not yet checked for blockCopyCompatibility");
#else
  invalidDataProcTrap(context, instruction, __func__);
#endif
}

/*********************************/
/* TST Rs, Rs2/imm               */
/*********************************/
u32int armTstInstruction(GCONTXT *context, u32int instruction)
{
  DIE_NOW(context, "not implemented");
}
