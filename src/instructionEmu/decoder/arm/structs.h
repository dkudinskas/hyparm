#ifndef __INSTRUCTION_EMU__DECODER__ARM__STRUCTS_H__
#define __INSTRUCTION_EMU__DECODER__ARM__STRUCTS_H__

#include "common/types.h"

typedef union
{
  struct armALUImmediateInstruction
  {
    u32int imm12 : 12;
    u32int Rd : 4;
    u32int Rn : 4;
    u32int S : 1;
    u32int pad0 : 7;
    u32int cond : 4;
  } fields;
  u32int value;
} ARM_ALU_imm;


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
