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


enum
{
  STR_IMMEDIATE_BASE_VALUE = 0x04000000,
  LDR_IMMEDIATE_BASE_VALUE = 0x04100000,
  SUB_IMMEDIATE_BASE_VALUE = 0x02400000,
  ADD_IMMEDIATE_BASE_VALUE = 0x02800000
};

#endif /* __INSTRUCTION_EMU__DECODER__ARM__STRUCTS_H__ */
