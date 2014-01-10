#ifndef __INSTRUCTION_EMU__DECODER__ARM__STRUCTS_H__
#define __INSTRUCTION_EMU__DECODER__ARM__STRUCTS_H__

#include "common/types.h"

#define ADD_IMMEDIATE_BASE_VALUE    0x02800000
#define SUB_IMMEDIATE_BASE_VALUE    0x02400000
#define BRANCH_BASE_VALUE           0x0a000000
#define LDM_STM_BASE_VALUE          0x08000000
#define LDR_IMMEDIATE_BASE_VALUE    0x04100000
#define STR_IMMEDIATE_BASE_VALUE    0x04000000
#define MRC_BASE_VALUE              0x0e100010
#define MCR_BASE_VALUE              0x0e000010


// this union will hold possible encodings for instructions
// each struct must be 32 bits exactly
// instruction as a whole can be accesssed via 'raw' field
typedef union ARMInstruction
{
  struct ALUimm
  {
    unsigned imm12: 12;
    unsigned Rd : 4;
    unsigned Rn : 4;
    unsigned S : 1;
    unsigned opc0 : 4;
    unsigned I : 1;
    unsigned opc1 : 2;
    unsigned cc : 4;
  } aluImm;

  struct ALUreg
  {
    unsigned Rm : 4;
    unsigned opc0 : 1;
    unsigned type : 2;
    unsigned imm5 : 5;
    unsigned Rd : 4;
    unsigned Rn : 4;
    unsigned S : 1;
    unsigned opc1 : 4;
    unsigned I : 1;
    unsigned opc2 : 2;
    unsigned cc : 4;
  } aluReg;

  struct Breakpoint
  {
    unsigned imm4:   4;
    unsigned opc0:   4;
    unsigned imm12: 12;
    unsigned opc1:   8;
    unsigned cc:     4;
  } bkpt;

  struct Branch
  {
    unsigned imm24: 24;
    unsigned opc0:   4;
    unsigned cc:     4;
  } branch;

  struct BxReg
  {
    unsigned Rm:     4;
    unsigned opc0:  24;
    unsigned cc:     4;
  } BxReg;

  struct Cps
  {
    unsigned mode:   5;
    unsigned opc0:   1;
    unsigned F:      1;
    unsigned I:      1;
    unsigned A:      1;
    unsigned opc1:   8;
    unsigned M:      1;
    unsigned imod:   2;
    unsigned opc2:  12;
  } cps;

  struct Ldrex
  {
    unsigned opc0:  12;
    unsigned Rt:     4;
    unsigned Rn:     4;
    unsigned opc1:   8;
    unsigned cc:     4;
  } ldrex;

  struct LoadStoreMultiple
  {
    unsigned regList:16;
    unsigned Rn:     4;
    unsigned load:   1;
    unsigned W:      1;
    unsigned user:   1;
    unsigned U:      1;
    unsigned P:      1;
    unsigned opc1:   3;
    unsigned cc:     4;
  } ldStMulti;

  struct LoadStoreImm
  {
    unsigned imm12: 12;
    unsigned Rt:     4;
    unsigned Rn:     4;
    unsigned load:   1;
    unsigned W:      1;
    unsigned byte:   1;
    unsigned U:      1;
    unsigned P:      1;
    unsigned I:      1;
    unsigned opc2:   2;
    unsigned cc:     4;
  } ldStImm;

  struct LoadStoreImm2
  {
    unsigned imm4L:  4;
    unsigned opc0:   4;
    unsigned imm4H:  4;
    unsigned Rt:     4;
    unsigned Rn:     4;
    unsigned opc1:   1;
    unsigned W:      1;
    unsigned opc2:   1;
    unsigned U:      1;
    unsigned P:      1;
    unsigned opc3:   3;
    unsigned cc:     4;
  } ldStImm2;

  struct LoadStoreReg
  {
    unsigned Rm:     4;
    unsigned opc0:   1;
    unsigned type:   2;
    unsigned imm5:   5;
    unsigned Rt:     4;
    unsigned Rn:     4;
    unsigned load:   1;
    unsigned W:      1;
    unsigned byte:   1;
    unsigned U:      1;
    unsigned P:      1;
    unsigned I:      1;
    unsigned opc3:   2;
    unsigned cc:     4;
  } ldStReg;

  struct LoadStoreReg2
  {
    unsigned Rm:     4;
    unsigned opc0:   8;
    unsigned Rt:     4;
    unsigned Rn:     4;
    unsigned opc1:   1;
    unsigned W:      1;
    unsigned opc2:   1;
    unsigned U:      1;
    unsigned P:      1;
    unsigned opc3:   3;
    unsigned cc:     4;
  } ldStReg2;

  struct Mcr
  {
    unsigned CRm:    4;
    unsigned opcode0:1;
    unsigned opc2:   3;
    unsigned coproc: 4;
    unsigned Rt:     4;
    unsigned CRn:    4;
    unsigned opcode1:1;
    unsigned opc1:   3;
    unsigned opcode2:4;
    unsigned cc:     4;
  } mcr;

  struct Mrs
  {
    unsigned opc0:  12;
    unsigned Rd:     4;
    unsigned opc1:   6;
    unsigned R:      1;
    unsigned opc2:   5;
    unsigned cc:     4;
  } mrs;

  struct MsrImm
  {
    unsigned imm12: 12;
    unsigned opc0:   4;
    unsigned mask:   4;
    unsigned opc1:   2;
    unsigned R:      1;
    unsigned opc2:   5;
    unsigned cc:     4;
  } msrImm;

  struct MsrReg
  {
    unsigned Rn:     4;
    unsigned opc0:  12;
    unsigned mask:   4;
    unsigned opc1:   2;
    unsigned R:      1;
    unsigned opc2:   5;
    unsigned cc:     4;
  } msrReg;

  struct Strex
  {
    unsigned Rt:     4;
    unsigned opc0:   8;
    unsigned Rd:     4;
    unsigned Rn:     4;
    unsigned opc1:   8;
    unsigned cc:     4;
  } strex;

  u32int raw;
} Instruction;

#endif /* __INSTRUCTION_EMU__DECODER__ARM__STRUCTS_H__ */
