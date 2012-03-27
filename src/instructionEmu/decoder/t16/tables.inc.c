/*
 * Thumb 16-bit decoding tables for the table search decoder
 */


static struct instruction32bit t16ArithmeticInstructions[] =
{
  // According to Thumb-2 spec, R15 cannot be used in arithmetic
  // instructions. So dont trap
  { IRC_SAFE,    &t16AddInstruction,       0x1800, 0xFF00, "ADD Rd, Rn, Rm" },
  /*
   * FIXME: impossible
   * page 310. Rd can be R15
   * { IRC_REPLACE, &addInstruction,       0x4487, 0x4487, "ADD<c> R15, Rm" },
   * Otherwise don't trap
   * { IRC_SAFE,    &addInstruction,       0x4400, 0x4400, "ADD<c> Rd, Rm" },
   */
  // CMP is fine
  { IRC_SAFE,    &t16CmpInstruction,       0x2800, 0xF800, "CMP<c> Rn, #<imm8>" },
  { IRC_SAFE,    &t16SubInstruction,       0x1E00, 0xFE00, "SUB{S} <Rd>, <Rn>,#<imm3>" },
  { IRC_SAFE,    &t16SubInstruction,       0x3800, 0xF800, "SUB{S} <Rdn>, #<imm8>" },
  { IRC_SAFE,    &t16SubInstruction,       0x1A00, 0xFE00, "SUB{S} <Rd>, <Rn> <Rm>" },
  { IRC_SAFE,    &t16MovInstruction,       0x2000, 0xF800, "MOV<c> <Rd>, #<imm8>" },
  { IRC_REPLACE, &undefinedInstruction, 0x0000, 0x0000, "t16ArithmeticInstructions" }
};

/* verified */
static struct instruction32bit t16ConditionalBranchSvcInstructions[] =
{
  { IRC_REPLACE, &svcInstruction,       0xDF00, 0xFF00, "SVC call" },
  { IRC_REPLACE, &t16BImmediate8Instruction, 0xD000, 0xF000, "B<c> [PC,#<imm8>]" },
  { IRC_REPLACE, &undefinedInstruction, 0x0000, 0x0000, "t16ConditionalBranchSvcInstructions" }
};

static struct instruction32bit t16DataProcInstructions[] =
{
  { IRC_SAFE,    &t16MvnInstruction,       0x43C0, 0xFFC0, "MVN <Rd>, <Rm>" },
  { IRC_SAFE,    &t16CmpInstruction,       0x4280, 0xFFC0, "CMP <Rn>, <Rm>" },
  /*
   * FIXME: impossible
   * { IRC_SAFE,    &cmpInstruction,       0x4500, 0xFF00, "CMP <Rn>, <Rm>" },
   */
  { IRC_REPLACE, &undefinedInstruction,    0x0000, 0x0000, "t16DataProcInstructions" }
};

static struct instruction32bit t16ldrInstructions[] =
{
  /*
   * FIXME: impossible
   * { IRC_SAFE,    &t16LdrInstruction,    0x6800, 0xF800, "LDR<c> <Rt>, [<Rn>{,#<imm5>}]" },
   * { IRC_SAFE,    &t16LdrInstruction,    0x9800, 0xF800, "LDR<c> <Rt>, [SP{,#<imm8>}]" },
   */
  { IRC_SAFE,    &t16LdrInstruction,    0x4800, 0xF800, "LDR<c> <Rt>, <label>" },
  /*
   * FIXME: impossible
   * { IRC_SAFE,    &t16LdrInstruction,    0x5800, 0xFE00, "LDR<c> <Rt>, [<Rn>,<Rm>]" },
   */
  { IRC_REPLACE, &undefinedInstruction, 0x0000, 0x0000, "t16ldrInstructions" }
};

static struct instruction32bit t16LoadStoreInstructions[] =
{
  { IRC_SAFE,    &t16StrInstruction,    0x6000, 0xF800, "STR Rd, [<Rn>, {#<imm5>}]" },
  { IRC_SAFE,    &t16StrSpInstruction,  0x9000, 0xF800, "STR Rd, [SP,#<imm8]" },
  { IRC_SAFE,    &t16LdrInstruction,    0x6800, 0xF800, "LDR Rd, [<Rn> {,#<imm5>}]" },
  { IRC_SAFE,    &t16LdrInstruction,    0x9800, 0xF800, "LDR Rd, [SP {,#<imm8>}]" },
  { IRC_SAFE,    &t16LdrInstruction,    0x7800, 0x7800, "LDRB Rd, #<imm5>" },
  { IRC_SAFE,    &t16LdrhImmediateInstruction, 0x8800, 0xF800, "LDRH <Rt>, [<Rn>{, #<imm5>}]" },
  { IRC_SAFE,    &t16StrbInstruction,   0x7000, 0xF800, "STRB <Rt>, [<Rn>, #<imm5>]" },
  /*
   * FIXME: impossible
   * { IRC_SAFE,    &t16StrbInstruction,   0xA400, 0xFE00, "STRB <Rt>, [<Rn>, <Rm>]" },
   */
  { IRC_SAFE,    &t16StrhInstruction,   0x8000, 0xF800, "STRH <Rt>,[<Rn>,{,#<imm>}]" },
  /*
   * FIXME: impossible
   * { IRC_SAFE,    &t16StrhInstruction,   0x5200, 0xFE00, "STRH <Rt>, [<Rn>,{,<Rm>}" },
   */
  { IRC_REPLACE, &undefinedInstruction, 0x0000, 0x0000, "t16LoadStoreInstructions" }
};

static struct instruction32bit t16MiscInstructions[] =
{
  { IRC_SAFE,    &t16PushInstruction,   0xB400, 0xFE00, "PUSH {reglist}" },
  { IRC_SAFE,    &t16SubInstruction,       0xB080, 0xFF80, "SUB SP,SP,<imm7>" },
  { IRC_SAFE,    &t16UxtbInstruction,      0xB2C0, 0xFFC0, "UXTB<c> <Rd>, <Rm>" },
  { IRC_SAFE,    &t16UxthInstruction,      0xB280, 0xFFC0, "UXTH<C> <Rd>, <Rm>" },
  /*
   * FIXME: impossible
   * { IRC_SAFE,    &addInstruction,       0xA800, 0xF800, "ADD <Rd>, SP, #<imm8>" },
   */
  { IRC_SAFE,    &t16AddInstruction,       0xB000, 0xFF80, "ADD SP, SP, #<imm>" },
  // trap if PC is on POP reglist
  { IRC_REPLACE, &t16LdmInstruction,    0xBD00, 0xFF00, "POP <reglist+PC>" },
  { IRC_SAFE,    &t16LdmInstruction,    0xBC00, 0xFE00, "POP <reglist>" },
  { IRC_SAFE,    &nopInstruction,       0xBF00, 0xFFFF, "NOP" },
  { IRC_REPLACE, &t16BkptInstruction,   0xBE00, 0xFF00, "BKPT <imm8>" },
  { IRC_REPLACE, &t16ItInstruction,     0xBF00, 0xFF00, "IT" },
  { IRC_REPLACE, &undefinedInstruction, 0x0000, 0x0000, "t16MiscInstructions" }
};

static struct instruction32bit t16PCRelInstructions[] =
{
  // ADR instruction is add or sub but in any case it does not trap
  { IRC_SAFE,    &t16AddInstruction,       0xA000, 0xF800, "ADR <Rd>, <label>" },
  { IRC_REPLACE, &undefinedInstruction, 0x0000, 0x0000, "t16PCRelInstructions" }
};

static struct instruction32bit t16SpecialBranchInstructions[] =
{
  { IRC_REPLACE, &t16BxInstruction,     0x4700, 0xFF80, "BX<c> <Rm>" },
  { IRC_REPLACE, &t16BlxRegisterInstruction, 0x4780, 0xFF80, "BLX<c> <Rm>" },
  { IRC_REPLACE, &t16MovInstruction,       0x4607, 0xFF07, "MOV PC, Rm" },
  { IRC_SAFE,    &t16MovInstruction,       0x4600, 0xFF00,  "MOV Rd, Rm" },
  /*
   * FIXME: impossible
   * { IRC_SAFE,    &addInstruction,       0x1800, 0xFE00, "ADD <Rd>, <Rn>, <Rm>" },
   */
  { IRC_SAFE,    &t16AddInstruction,       0x4400, 0xFF00, "ADD <Rdn>, <Rm>" },
  { IRC_REPLACE, &undefinedInstruction, 0x0000, 0x0000, "t16SpecialBranchInstructions" }
};

static struct instruction32bit t16SPInstructions[] =
{
  { IRC_SAFE,    &t16AddInstruction,       0xA800, 0xF800, "ADD <Rd>, SP, #<imm>" },
  /*
   * FIXME: impossible
   * { IRC_SAFE,    &addInstruction,       0xB000, 0xFF80, "ADD SP, SP, #<imm>" },
   */
  { IRC_REPLACE, &undefinedInstruction, 0x0000, 0x0000, "t16SPInstructions" }
};

/* verified */
static struct instruction32bit t16UnconditionalInstructions[] =
{
  { IRC_REPLACE, &t16BImmediate11Instruction, 0xE000, 0xF800, "B<c> [PC,#<imm11>]" },
  { IRC_REPLACE, &undefinedInstruction, 0x0000, 0x0000, "t16UnconditionalInstructions" }
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
