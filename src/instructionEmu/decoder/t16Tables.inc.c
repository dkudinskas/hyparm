/*
 * Thumb 16-bit decoding tables for the table search decoder
 */


static struct instruction32bit t16ArithmeticInstructions[] =
{
  // According to Thumb-2 spec, R15 cannot be used in arithmetic
  // instructions. So dont trap
  { FALSE, &addInstruction,       0x1800, 0xFF00, "ADD Rd, Rn, Rm" },
  /*
   * FIXME: impossible
   * page 310. Rd can be R15
   * { TRUE,  &addInstruction,       0x4487, 0x4487, "ADD<c> R15, Rm" },
   * Otherwise don't trap
   * { FALSE, &addInstruction,       0x4400, 0x4400, "ADD<c> Rd, Rm" },
   */
  // CMP is fine
  { FALSE, &cmpInstruction,       0x2800, 0xF800, "CMP<c> Rn, #<imm8>" },
  { FALSE, &subInstruction,       0x1E00, 0xFE00, "SUB{S} <Rd>, <Rn>,#<imm3>" },
  { FALSE, &subInstruction,       0x3800, 0xF800, "SUB{S} <Rdn>, #<imm8>" },
  { FALSE, &subInstruction,       0x1A00, 0xFE00, "SUB{S} <Rd>, <Rn> <Rm>" },
  { FALSE, &movInstruction,       0x2000, 0xF800, "MOV<c> <Rd>, #<imm8>" },
  { TRUE,  &undefinedInstruction, 0x0000, 0x0000, "t16ArithmeticInstructions" }
};

/* verified */
static struct instruction32bit t16ConditionalBranchSvcInstructions[] =
{
  { TRUE,  &svcInstruction,       0xDF00, 0xFF00, "SVC call" },
  { TRUE,  &t16BImmediate8Instruction, 0xD000, 0xF000, "B<c> [PC,#<imm8>]" },
  { TRUE,  &undefinedInstruction, 0x0000, 0x0000, "t16ConditionalBranchSvcInstructions" }
};

static struct instruction32bit t16DataProcInstructions[] =
{
  { FALSE, &mvnInstruction,       0x43C0, 0xFFC0, "MVN <Rd>, <Rm>" },
  { FALSE, &cmpInstruction,       0x4280, 0xFFC0, "CMP <Rn>, <Rm>" },
  /*
   * FIXME: impossible
   * { FALSE, &cmpInstruction,       0x4500, 0xFF00, "CMP <Rn>, <Rm>" },
   */
  { TRUE,  &undefinedInstruction, 0x0000, 0x0000, "t16DataProcInstructions" }
};

static struct instruction32bit t16ldrInstructions[] =
{
  /*
   * FIXME: impossible
   * { FALSE, &t16LdrInstruction,    0x6800, 0xF800, "LDR<c> <Rt>, [<Rn>{,#<imm5>}]" },
   * { FALSE, &t16LdrInstruction,    0x9800, 0xF800, "LDR<c> <Rt>, [SP{,#<imm8>}]" },
   */
  { FALSE, &t16LdrInstruction,    0x4800, 0xF800, "LDR<c> <Rt>, <label>" },
  /*
   * FIXME: impossible
   * { FALSE, &t16LdrInstruction,    0x5800, 0xFE00, "LDR<c> <Rt>, [<Rn>,<Rm>]" },
   */
  { TRUE,  &undefinedInstruction, 0x0000, 0x0000, "t16ldrInstructions" }
};

static struct instruction32bit t16LoadStoreInstructions[] =
{
  { FALSE, &t16StrInstruction,    0x6000, 0xF800, "STR Rd, [<Rn>, {#<imm5>}]" },
  { FALSE, &t16StrSpInstruction,  0x9000, 0xF800, "STR Rd, [SP,#<imm8]" },
  { FALSE, &t16LdrInstruction,    0x6800, 0xF800, "LDR Rd, [<Rn> {,#<imm5>}]" },
  { FALSE, &t16LdrInstruction,    0x9800, 0xF800, "LDR Rd, [SP {,#<imm8>}]" },
  { FALSE, &t16LdrInstruction,    0x7800, 0x7800, "LDRB Rd, #<imm5>" },
  { FALSE, &t16LdrhImmediateInstruction, 0x8800, 0xF800, "LDRH <Rt>, [<Rn>{, #<imm5>}]" },
  { FALSE, &t16StrbInstruction,   0x7000, 0xF800, "STRB <Rt>, [<Rn>, #<imm5>]" },
  /*
   * FIXME: impossible
   * { FALSE, &t16StrbInstruction,   0xA400, 0xFE00, "STRB <Rt>, [<Rn>, <Rm>]" },
   */
  { FALSE, &t16StrhInstruction,   0x8000, 0xF800, "STRH <Rt>,[<Rn>,{,#<imm>}]" },
  /*
   * FIXME: impossible
   * { FALSE, &t16StrhInstruction,   0x5200, 0xFE00, "STRH <Rt>, [<Rn>,{,<Rm>}" },
   */
  { TRUE,  &undefinedInstruction, 0x0000, 0x0000, "t16LoadStoreInstructions" }
};

static struct instruction32bit t16MiscInstructions[] =
{
  { FALSE, &t16PushInstruction,   0xB400, 0xFE00, "PUSH {reglist}" },
  { FALSE, &subInstruction,       0xB080, 0xFF80, "SUB SP,SP,<imm7" },
  { FALSE, &uxtbInstruction,      0xB2C0, 0xFFC0, "UXTB<c> <Rd>, <Rm>" },
  { FALSE, &uxthInstruction,      0xB280, 0xFFC0, "UXTH<C> <Rd>, <Rm>" },
  /*
   * FIXME: impossible
   * { FALSE, &addInstruction,       0xA800, 0xF800, "ADD <Rd>, SP, #<imm8>" },
   */
  { FALSE, &addInstruction,       0xB000, 0xFF80, "ADD SP, SP, #<imm>" },
  // trap if PC is on POP reglist
  { TRUE,  &t16LdmInstruction,    0xBD00, 0xFF00, "POP <reglist+PC>" },
  { FALSE, &t16LdmInstruction,    0xBC00, 0xFE00, "POP <reglist>" },
  { FALSE, &nopInstruction,       0xBF00, 0xFFFF, "NOP" },
  /*
   * FIXME: implement IT support
   * Markos: I think that the If-Then-Else Instruction does not need to trap -.-
   * Niels: the corectness of the interpreter depends on ITSTATE, since no checks on ITSTATE are
   * implemented, we should trap here until we have implemented full support for ITSTATE. Of course,
   * since Markos has ignored this, the Thumb demo breaks if we trap on IT (it is broken anyway
   * after +/- 17 seconds).
   */
  { FALSE, &t16ItInstruction,     0xBF00, 0xFF00, "IT" },
  { TRUE,  &undefinedInstruction, 0x0000, 0x0000, "t16MiscInstructions" }
};

static struct instruction32bit t16PCRelInstructions[] =
{
  // ADR instruction is add or sub but in any case it does not trap
  { FALSE, &addInstruction,       0xA000, 0xF800, "ADR <Rd>, <label>" },
  { TRUE,  &undefinedInstruction, 0x0000, 0x0000, "t16PCRelInstructions" }
};

static struct instruction32bit t16SpecialBranchInstructions[] =
{
  { TRUE,  &t16BxInstruction,     0x4700, 0xFF80, "BX<c> <Rm>" },
  { TRUE,  &t16BlxRegisterInstruction, 0x4780, 0xFF80, "BLX<c> <Rm>" },
  { TRUE,  &movInstruction,       0x4607, 0xFF07, "MOV PC, Rm" },
  { FALSE, &movInstruction,       0x4600, 0xFF00,  "MOV Rd, Rm" },
  /*
   * FIXME: impossible
   * { FALSE, &addInstruction,       0x1800, 0xFE00, "ADD <Rd>, <Rn>, <Rm>" },
   */
  { FALSE, &addInstruction,       0x4400, 0xFF00, "ADD <Rdn>, <Rm>" },
  { TRUE,  &undefinedInstruction, 0x0000, 0x0000, "t16SpecialBranchInstructions" }
};

static struct instruction32bit t16SPInstructions[] =
{
  { FALSE, &addInstruction,       0xA800, 0xF800, "ADD <Rd>, SP, #<imm>" },
  /*
   * FIXME: impossible
   * { FALSE, &addInstruction,       0xB000, 0xFF80, "ADD SP, SP, #<imm>" },
   */
  { TRUE,  &undefinedInstruction, 0x0000, 0x0000, "t16SPInstructions" }
};

/* verified */
static struct instruction32bit t16UnconditionalInstructions[] =
{
  { TRUE,  &t16BImmediate11Instruction, 0xE000, 0xF800, "B<c> [PC,#<imm11>]" },
  { TRUE,  &undefinedInstruction, 0x0000, 0x0000, "t16UnconditionalInstructions" }
};


static struct TopLevelCategory t16Categories[] =
{
  { 0xF800, 0xE000, t16UnconditionalInstructions },
  { 0xF000, 0xD000, t16ConditionalBranchSvcInstructions },
  /*
   * TODO: not implemented
   * { 0xF800, 0xC800, t16LoadMultipleRegistersInstructions },
   * { 0xF800, 0xC000, t16StoreMultipleRegistersInstructions },
   */
  { 0xF000, 0xB000, t16MiscInstructions },
  { 0xF800, 0xA800, t16SPInstructions },
  { 0xF800, 0xA000, t16PCRelInstructions },
  { 0xE000, 0x8000, t16LoadStoreInstructions },
  { 0xE000, 0x6000, t16LoadStoreInstructions },
  /*
   * FIXME: impossible
   * { 0xE000, 0x5000, t16LoadStoreInstructions }
   */
  { 0xF800, 0x4800, t16ldrInstructions },
  { 0xFC00, 0x4400, t16SpecialBranchInstructions },
  { 0xFC00, 0x4000, t16DataProcInstructions },
  { 0xC000, 0x0000, t16ArithmeticInstructions }
};
