#ifndef __INSTRUCTION_EMU__DECODER__ARM__STRUCTS_H__
#define __INSTRUCTION_EMU__DECODER__ARM__STRUCTS_H__

#include "common/types.h"

 typedef union
 {
  struct armStrImmediateInstruction
  {
    unsigned immediate : 12;
    unsigned sourceRegister : 4;
    unsigned baseRegister : 4;
    unsigned : 1;
    unsigned writeBackIfNotIndex : 1;
    unsigned : 1;
    unsigned add : 1;
    unsigned index : 1;
    unsigned : 3;
    unsigned conditionCode : 4;
  } fields;
  u32int value;
} ARMStrImmediateInstruction;


typedef union
{
  struct armLdrImmediateInstruction
  {
    unsigned immediate : 12;
    unsigned sourceRegister : 4;
    unsigned baseRegister : 4;
    unsigned : 1;
    unsigned writeBack : 1;
    unsigned : 1;
    unsigned add : 1;
    unsigned index : 1;
    unsigned : 3;
    unsigned conditionCode : 4;
  } fields;
  u32int value;
} ARMLdrImmediateInstruction;


typedef union
{
  struct armAddImmediateInstruction
  {
    unsigned immediate : 12;
    unsigned destinationRegister : 4;
    unsigned operandRegister : 4;
    unsigned setFlags : 1;
    unsigned : 7;
    unsigned conditionCode : 4;
  } fields;
  u32int value;
} ARMAddImmediateInstruction;


typedef union
{
  struct armSubImmediateInstruction
  {
    unsigned immediate : 12;
    unsigned destinationRegister : 4;
    unsigned operandRegister : 4;
    unsigned setFlags : 1;
    unsigned : 7;
    unsigned conditionCode : 4;
  } fields;
  u32int value;
} ARMSubImmediateInstruction;


typedef union
{
  struct armAddImmediateInstructionALU
  {
    u32int immediate : 12;
    u32int destinationRegister : 4;
    u32int operandRegister : 4;
    u32int setFlags : 1;
    u32int pad0 : 7;
    u32int conditionCode : 4;
  } fields;
  u32int value;
} ARM_ALU_imm;


typedef union
{
  struct armLdrImmediateInstructionNew
  {
    u32int immediate : 12;
    u32int sourceRegister : 4;
    u32int baseRegister : 4;
    u32int : 1;
    u32int writeBack : 1;
    u32int : 1;
    u32int add : 1;
    u32int index : 1;
    u32int : 3;
    u32int conditionCode : 4;
  } fields;
  u32int value;
} ARM_ldr_imm;


typedef union
{
  struct armLdmStmInstruction
  {
    u32int register_list : 16;
    u32int Rn : 4;
    u32int L : 1;
    u32int W : 1;
    u32int S : 1;
    u32int U : 1;
    u32int P : 1;
    u32int pad : 3;
    u32int cond : 4;
  } fields;
  u32int value;
} ARM_ldm_stm;


typedef union
{
  struct armLdrStrImmediateInstruction
  {
    u32int imm12 : 12;
    u32int Rt : 4; // source
    u32int Rn : 4; // base
    u32int L : 1;
    u32int W : 1;
    u32int B : 1;
    u32int U : 1;
    u32int P : 1;
    u32int I : 1;
    u32int pad: 2;
    u32int cond : 4;
  } fields;
  u32int value;
} ARM_ldr_str_imm;


enum
{
  ADD_IMMEDIATE_BASE_VALUE = 0x02800000,
  SUB_IMMEDIATE_BASE_VALUE = 0x02400000,
  BRANCH_BASE_VALUE        = 0x0a000000,
  LDM_STM_BASE_VALUE       = 0x08000000,
  LDR_IMMEDIATE_BASE_VALUE = 0x04100000,
  STR_IMMEDIATE_BASE_VALUE = 0x04000000,
};

#endif /* __INSTRUCTION_EMU__DECODER__ARM__STRUCTS_H__ */
