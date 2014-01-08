#ifndef __CPU_ARCH__STATE_H__
#define __CPU_ARCH__STATE_H__

#include "common/types.h"

typedef enum
{
  USR_MODE = 0x10,
  FIQ_MODE = 0x11,
  IRQ_MODE = 0x12,
  SVC_MODE = 0x13,
  MON_MODE = 0X16,
  ABT_MODE = 0x17,
  HYP_MODE = 0x1a,
  UND_MODE = 0x1b,
  SYS_MODE = 0x1f,
} CPSRmode;

typedef union
{
  struct
  {
    CPSRmode mode: 5;
    u32int T     : 1; // thumb bit
    u32int F     : 1; // fast interrupts disable
    u32int I     : 1; // interrupts disable
    u32int A     : 1; // async-aborts disable
    u32int E     : 1; // big-endian enable
    u32int IT_7_2: 6; // IT state bits [2-7]
    u32int GE    : 4; // greater-or-equals flag (SIMD)
    u32int pad0  : 4; // reserved
    u32int J     : 1; // jazelle bit
    u32int IT_0_1: 2; // IT state bits 0-1
    u32int Q     : 1; // cumulative saturation flag
    u32int V     : 1; // oVerflow flag
    u32int C     : 1; // carry flag
    u32int Z     : 1; // zero flag
    u32int N     : 1; // negative flag
  } bits;
  u32int value;
} CPSRreg;


typedef enum
{
  EQ = 0x0,  // equals
  NE = 0x1,  // not equals
  HS = 0x2,  // carry set / unsigned higher or same
  LO = 0x3,  // carry not set / unsigned lower
  MI = 0x4,  // minus / negative / N set
  PL = 0x5,  // plus / positive or zero / N clear
  VS = 0x6,  // overflow / V set
  VC = 0x7,  // no overflow / V clear
  HI = 0x8,  // unsigned higher
  LS = 0x9,  // unsigned lower or same
  GE = 0xA,  // signed greater than or equals
  LT = 0xB,  // signed less than
  GT = 0xC,  // signed greater than
  LE = 0xD,  // signed less than or equal
  AL = 0xE,  // always
  NV = 0xF,  // never - should not be used! only special uncond instr
} ConditionCode;


typedef enum
{
  InstrSet_ARM,
  InstrSet_Thumb,
  InstrSet_ThumbEE,
  InstrSet_Jazelle,
} InstructionSet;


typedef enum 
{
  AND = 0x0,
  EOR = 0x1,
  SUB = 0x2,
  RSB = 0x3,
  ADD = 0x4,
  ADC = 0x5,
  SBC = 0x6,
  RSC = 0x7,
  TST = 0x8,
  TEQ = 0x9,
  CMP = 0xA,
  CMN = 0xB,
  ORR = 0xC,
  MOV = 0xD,
  BIC = 0xE,
  MVN = 0xF
} AluOp;

#endif