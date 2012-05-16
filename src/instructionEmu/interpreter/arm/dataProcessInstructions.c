#include "instructionEmu/interpreter/internals.h"

#include "instructionEmu/interpreter/arm/dataProcessInstructions.h"


/*********************************/
/* ADD Rd, Rs, Rs2/imm, shiftAmt */
/*********************************/
u32int armAdcInstruction(GCONTXT *context, u32int instruction)
{
  DIE_NOW(context, ERROR_NOT_IMPLEMENTED);
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
  DIE_NOW(context, ERROR_NOT_IMPLEMENTED);
}

/*********************************/
/* AND Rd, Rs, Rs2/imm, shiftAmt */
/*********************************/
u32int armAndInstruction(GCONTXT *context, u32int instruction)
{
  TRACE(context, instruction);
  DIE_NOW(context, ERROR_NOT_IMPLEMENTED);
}

/*********************************/
/* ASR Rd, Rs                    */
/*********************************/
u32int armAsrInstruction(GCONTXT *context, u32int instruction)
{
  DIE_NOW(context, ERROR_NOT_IMPLEMENTED);
}

/*********************************/
/* SBC Rd, Rs, Rs2/imm, shiftAmt */
/*********************************/
u32int armBicInstruction(GCONTXT *context, u32int instruction)
{
  DIE_NOW(context, ERROR_NOT_IMPLEMENTED);
}

/*********************************/
/* CMN Rs, Rs2/imm               */
/*********************************/
u32int armCmnInstruction(GCONTXT *context, u32int instruction)
{
  invalidDataProcTrap(context, instruction, __func__);
}

/*********************************/
/* CMP Rs, Rs2/imm               */
/*********************************/
u32int armCmpInstruction(GCONTXT *context, u32int instruction)
{
  invalidDataProcTrap(context, instruction, __func__);
}

/*********************************/
/* ORR Rd, Rs, Rs2/imm, shiftAmt */
/*********************************/
u32int armEorInstruction(GCONTXT *context, u32int instruction)
{
  DIE_NOW(context, ERROR_NOT_IMPLEMENTED);
}

/*********************************/
/* LSL Rd, Rs                    */
/*********************************/
u32int armLslInstruction(GCONTXT *context, u32int instruction)
{
  DIE_NOW(context, ERROR_NOT_IMPLEMENTED);
}

/*********************************/
/* LSR Rd, Rs                    */
/*********************************/
u32int armLsrInstruction(GCONTXT *context, u32int instruction)
{
  DIE_NOW(context, ERROR_NOT_IMPLEMENTED);
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
  DIE_NOW(context, ERROR_NOT_IMPLEMENTED);
}

u32int armMovwInstruction(GCONTXT *context, u32int instruction)
{
  DIE_NOW(context, ERROR_NOT_IMPLEMENTED);
}

/*********************************/
/* MVN Rd, Rs/imm                */
/*********************************/
u32int armMvnInstruction(GCONTXT *context, u32int instruction)
{
  DIE_NOW(context, ERROR_NOT_IMPLEMENTED);
}

/*********************************/
/* ORR Rd, Rs, Rs2/imm, shiftAmt */
/*********************************/
u32int armOrrInstruction(GCONTXT *context, u32int instruction)
{
  DIE_NOW(context, ERROR_NOT_IMPLEMENTED);
}

/*********************************/
/* ROR Rd, Rs                    */
/*********************************/
u32int armRorInstruction(GCONTXT *context, u32int instruction)
{
  DIE_NOW(context, ERROR_NOT_IMPLEMENTED);
}

/*********************************/
/* RRX Rd, Rs                    */
/*********************************/
u32int armRrxInstruction(GCONTXT *context, u32int instruction)
{
  DIE_NOW(context, ERROR_NOT_IMPLEMENTED);
}

/*********************************/
/* RSB Rd, Rs, Rs2/imm, shiftAmt */
/*********************************/
u32int armRsbInstruction(GCONTXT *context, u32int instruction)
{
  DIE_NOW(context, ERROR_NOT_IMPLEMENTED);
}

/*********************************/
/* RSB Rd, Rs, Rs2/imm, shiftAmt */
/*********************************/
u32int armRscInstruction(GCONTXT *context, u32int instruction)
{
  DIE_NOW(context, ERROR_NOT_IMPLEMENTED);
}

/*********************************/
/* SBC Rd, Rs, Rs2/imm, shiftAmt */
/*********************************/
u32int armSbcInstruction(GCONTXT *context, u32int instruction)
{
  DIE_NOW(context, ERROR_NOT_IMPLEMENTED);
}

/*********************************/
/* SUB Rd, Rs, Rs2/imm, shiftAmt */
/*********************************/
u32int armSubInstruction(GCONTXT *context, u32int instruction)
{
  OPTYPE opType = SUB;
  return arithLogicOp(context, instruction, opType, __func__);
}

/*********************************/
/* TEQ Rs, Rs2/imm               */
/*********************************/
u32int armTeqInstruction(GCONTXT *context, u32int instruction)
{
  invalidDataProcTrap(context, instruction, __func__);
}

/*********************************/
/* TST Rs, Rs2/imm               */
/*********************************/
u32int armTstInstruction(GCONTXT *context, u32int instruction)
{
  DIE_NOW(context, ERROR_NOT_IMPLEMENTED);
}
