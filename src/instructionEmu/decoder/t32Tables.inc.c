/*
 * Thumb 32-bit decoding tables for the table search decoder
 */

static struct instruction32bit t32DataProcInstructions[] =
{
  // this is for T2 encoding, page 480
  { FALSE, &movInstruction,       0xF04F0000, 0xFBEF1000, "MOVW<c> <Rd>, #<imm12>" },
  // this is for T3 encoding
  { FALSE, &movInstruction,       0xF2400000, 0xFBF00000, "MOVW<c> <Rd>, #<imm16>" },
  { FALSE, &movInstruction,       0xF04F0000, 0xFBEF8000, "MOV{S}/W <Rd>, #<immt12>" },
  { FALSE, &movInstruction,       0xF2400000, 0xFBF08000, "MOVW <Rd>, #<imm16>" },
  { FALSE, &movtInstruction,      0xF2C00000, 0xFBF08000, "MOVT<c> <Rd>, #<imm16>" },
  { FALSE, &orrInstruction,       0xEA400000, 0xFFE08000, "ORR{S} <Rd>, <Rn>{,<shift>}" },
  { FALSE, &orrInstruction,       0xF0400000, 0xFBE08000, "ORR{S} <Rd>, <Rn>,#<imm12>" },
  { FALSE, &andInstruction,       0xEA000000, 0xFFF00000, "AND{S}.W <Rd>. <Rn>. <Rm>" },
  //trap for RD=15
  { TRUE,  &andInstruction,       0xF0000F00, 0xFBE08F00, "AND{S}<c> PC, <Rm>, #<imm12>" },
  { FALSE, &andInstruction,       0xF0000000, 0xFBE08000, "AND{S}<c> <Rd>, <Rm>, #<imm12>" },
  { FALSE, &addInstruction,       0xF1000000, 0xFBE08000, "ADD{S}.W <Rd>, <Rn>, #<imm8>" },
  { FALSE, &addInstruction,       0xEB000000, 0xFFE08000, "ADD{S}.W <Rd>, <Rn>, <Rm>{, <shift>}" },
  //ADD -> RD=PC -> CMN page 306
  { TRUE,  addInstruction,        0xF1000F00, 0xFBE08F00, "ADD{S}.W PC, <Rn>, #<imm8>" },
  /*
   * FIXME: impossible
   * RN=SP -> unimplemented. Should be OK
   * { FALSE, &addInstruction,       0xF10D8000, 0xFBEF8000, "ADD{S}.W <Rd>, SP, #<imm8>" },
   */
  // Encoding T4
  { FALSE, &addInstruction,       0xF2000000, 0xF2008000, "ADDW <Rd>, <Rn>, #<imm12>" },
  // RC=PC
  { TRUE,  &addInstruction,       0xF20F0000, 0xF20F8000, "ADDW PC, <Rn>, #<imm12>" },
  // RN=SP -> should be ok
  { FALSE, &addInstruction,       0xF20D0000, 0xF20F8000, "ADDW <Rd>, SP, #<imm8>" },
  { FALSE, &bicInstruction,       0xF0200000, 0xFBE08000, "BIC{S} <Rd>, <Rn>, #<imm12>" },
  { FALSE, &rsbInstruction,       0xF1C00000, 0xFBE08000, "RSB <Rd>, <Rn>, #<imm12>" },
  { FALSE, &rsbInstruction,       0xEBC00000, 0xFFE08000, "RSB <Rd>, <Rn>, <Rm>{,<shift>}" },
  { FALSE, &subInstruction,       0xF1A00000, 0xFBE08000, "SUB{S}.W <Rd>, <Rn>, #<imm12>" },
  { FALSE, &subInstruction,       0xF2A00000, 0xFBE08000, "SUB{S}W <Rd>, <Rn>, #<imm12>" },
  { FALSE, &subInstruction,       0xEBA00000, 0xFFE08000, "SUB{S}.W <Rd>, <Rn>, <Rm>{,<shitft>}" },
  // RN = SP -> should be ok
  { FALSE, &subInstruction,       0xF2AB0000, 0xFBEF8000, "SUBW <Rd>, SP, #<imm12>" },
  { FALSE, &subInstruction,       0xEBAB0000, 0xFFEF8000, "SUB{S} <Rd>, SP, <Rm>{,<shift>}" },
  { FALSE, &mvnInstruction,       0xEA6F0000, 0xFFEF8000, "MVN<c> <Rd>, <Rm>{,<shift>}" },
  /*
   * FIXME: impossible
   * { FALSE, &mvnInstruction,       0xE0AF0000, 0xFBEF8000, "MVN<c> <Rd>, #<imm12>" },
   */
  { TRUE,  &undefinedInstruction, 0x00000000, 0x00000000, "t32DataProcInstructions" }
};

static struct instruction32bit t32SingleStoreInstructions[] =
{
  { FALSE, &t32StrbInstruction,   0xF8800000, 0xFFF00000, "STRB Rt, [Rn, #<imm12>]" },
  { FALSE, &t32StrbInstruction,   0xF8000800, 0xFFF00800, "STRB Rt, [Rn, +-#<imm8>]" },
  { FALSE, &t32StrhImmediateInstruction, 0xF8A00000, 0xFFF00000, "STRH.W <Rt>, [<Rn>, {#<imm12>}]" },
  { FALSE, &t32StrhRegisterInstruction, 0xF8200000, 0xFFF00FC0, "STRH.W <Rt>, [<Rn>, <Rm> {, LSL #<imm2>}]" },
  { TRUE,  &undefinedInstruction, 0x00000000, 0x00000000, "t32SingleStoreInstructions" }
};

static struct instruction32bit t32LoadByteInstructions[] =
{
  { TRUE,  &t32LdrbInstruction,   0xF81FF000, 0xFEFFFFC0, "LDRB<c> PC ,#<label>" },
  { FALSE, &t32LdrbInstruction,   0xF8900000, 0xFFF00000, "LDRB<c> Rt, [Rn{,#<imm12>}]" },
  { FALSE, &t32LdrbInstruction,   0xF8100900, 0xFFF00900, "LDRB<c> Rt, [Rn,{#<imm12>}]" },
  { FALSE, &t32LdrbInstruction,   0xF8100C00, 0xFFF00E00, "LDRB<c> Rt, [Rn,{#<imm12>}]" },
  { TRUE,  &undefinedInstruction, 0x00000000, 0x00000000, "t32LoadByteInstructions" }
};

/*
 * Verified and correct. LDR{S}HT instructions missing.
 * LDRSHT 0x FFF0 0F00 / 0x F930 0E00
 * LDRHT  0x FFF0 0F00 / 0x F830 0E00
 */
static struct instruction32bit t32LoadHalfWordInstructions[] =
{
  { FALSE, &t32LdrhImmediateInstruction,    0xF8B00000, 0xFFF00000, "LDRH.W <Rt>, [<Rn>{. #<imm32>}]" },
  { FALSE, &t32LdrhLiteralInstruction,      0xF83F0000, 0xFF7F0000, "LDRH <Rt>, <label>" },
  { FALSE, &t32LdrhRegisterInstruction,     0xF8300000, 0xFFF00FC0, "LDRH, <Rt>, [<Rn>{,LSL #<imm2>}]" },
  { FALSE, &t32LdrshImmediate12Instruction, 0xF9B00000, 0xFFF00000, "LDRSH<c> <Rt>, [<Rn>, #<imm12>]" },
  { FALSE, &t32LdrshLiteralInstruction,     0xF93F0000, 0xFF7F0000, "LDRSH<c> <Rt>, <label>" },
  { FALSE, &t32LdrshImmediate8Instruction,  0xF9300800, 0xFFF00800, "LDRSH<c> <Rt>, [<Rn>, #<Rn>, +/-#imm8]" }, // Page 454, A8-168
  { FALSE, &t32LdrshRegisterInstruction,    0xF9300000, 0xFFF00FC0, "LDRSH<c> <Rt>, [<Rn>, <Rm>]" },
  { TRUE,  &undefinedInstruction, 0x00000000, 0x00000000, "t32LoadHalfWordInstructions" }
};

static struct instruction32bit t32MultiplyInstructions[] =
{
  { FALSE, &mulInstruction,       0xFB00F000, 0xFFF0F0F0, "MULW <Rd>, <Rn>, <Rm>" },
  { FALSE, &smullInstruction,     0xFB800000, 0xFFE000F0, "SMULL <RdLo>, <RdHi>, <Rn>, <Rm>" },
  { TRUE,  &undefinedInstruction, 0x00000000, 0x00000000, "t32MultiplyInstructions" }
};

static struct instruction32bit t32BranchMiscInstructions[] =
{
  { TRUE,  t32BImmediate17Instruction, 0xF0008000, 0xF800D000, "B <imm17>" },
  { TRUE,  t32BImmediate21Instruction, 0xF0009000, 0xF800D000, "B <imm21>" },
  { TRUE,  t32BlInstruction,      0xF000D000, 0xF800D000, "BL, #<imm21>" },
  { TRUE,  &t32BlxImmediateInstruction, 0xF000C000, 0xF800D000, "BLX, #<imm21>" },
  { TRUE,  &undefinedInstruction, 0x00000000, 0x00000000, "t32BranchMiscInstructions" }
};

static struct instruction32bit t32LoadStoreDoubleExclusiveInstructions[] =
{
  { FALSE, &t32LdrdInstruction,   0xE8500000, 0xFE500000, "LDRD <Rt>, <Rt2>, [<Rn>,{,#+/-<imm>}]" },
  { FALSE, &t32LdrdInstruction,   0xE85F0000, 0xFE7F0000, "LDRD <Rt>, <Rt2>, <label>" },
  { FALSE, &t32StrdImmediateInstruction, 0xE8400000, 0xFE500000, "STRD <Rt>, <Rt2>, [<Rn>,{,#+/-<imm>}]" },
  { TRUE,  &undefinedInstruction, 0x00000000, 0x00000000, "t32LoadStoreDoubleExclusiveInstructions" }
};


static struct TopLevelCategory t32Categories[] =
{
  /*
   * TODO: not implemented
   * { 0xFE400000, 0xE8000000, t32LoadStoreMultipleInstructions },
   */
  { 0xFE400000, 0xE8400000, t32LoadStoreDoubleExclusiveInstructions },
  { 0xF8008000, 0xF0008000, t32BranchMiscInstructions },
  { 0xFE000000, 0xEA000000, t32DataProcInstructions },
  { 0xF8008000, 0xF0000000, t32DataProcInstructions },
  { 0xFF100000, 0xF8000000, t32SingleStoreInstructions },
  /*
   * TODO: not implemented
   * { 0xFF100000, 0xF9000000, t32SimdStructLoadStoreInstructions },
   */
  { 0xFE700000, 0xF8100000, t32LoadByteInstructions },
  { 0xFE700000, 0xF8300000, t32LoadHalfWordInstructions },
  /*
   * TODO: not implemented
   * { 0xF8500000, 0xF8500000, t32LoadWordInstructions },
   */
  { 0xF8700000, 0xF8700000, NULL },
  { 0xFF000000, 0xFA000000, t32DataProcInstructions },
  { 0xFF000000, 0xFB000000, t32MultiplyInstructions },
  /*
   * TODO: not implemented
   * { 0xFF800000, 0xFB800000, t32LongMultiplyInstructions },
   * { 0xEC000000, 0xEC000000, t32CoprocInstructions },
   */
  { 0x00000000, 0x00000000, NULL }
};
