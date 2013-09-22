/*******************************************************************************
 *
 * WARNING: automatically generated; do not edit.
 * Edit the decoder specification instead, and generate a new decoder.
 *
 * Options used:
 * Specification:     decoder.xml
 * Number of stages:  1 (monolithic)
 * Type:              g (graph)
 * Exhaustive test:   no
 *
 ******************************************************************************/

  if ((instruction & (1u << 29)) == 0)
  {
    goto autodecoder_arm_monolithic_581;
  }

  if ((instruction & (1u << 30)) == 0)
  {
    goto autodecoder_arm_monolithic_581;
  }

  if ((instruction & (1u << 31)) == 0)
  {
    goto autodecoder_arm_monolithic_581;
  }

  if ((instruction & (1u << 27)) == 0)
  {
    goto autodecoder_arm_monolithic_538;
  }

  if ((instruction & (1u << 15)) == 0)
  {
    goto autodecoder_arm_monolithic_61;
  }

  if (instruction & (1u << 28))
  {
    goto autodecoder_arm_monolithic_3;
  }
  goto autodecoder_arm_monolithic_19;

autodecoder_arm_monolithic_61:
  if (instruction & (1u << 12))
  {
    goto autodecoder_arm_monolithic_26;
  }

  if (instruction & (1u << 13))
  {
    goto autodecoder_arm_monolithic_26;
  }

  if ((instruction & (1u << 14)) == 0)
  {
    goto autodecoder_arm_monolithic_58;
  }

autodecoder_arm_monolithic_26:
  if ((instruction & (1u << 28)) == 0)
  {
    goto autodecoder_arm_monolithic_25;
  }

autodecoder_arm_monolithic_3:
  if (instruction & (1u << 26))
  {
    goto autodecoder_arm_undefined;
  }

  if (instruction & (1u << 25))
  {
    handler->barePtr = (void *)(armBlxImmediateInstruction);
    return IRC_REPLACE;
  }
  goto autodecoder_arm_undefined;

autodecoder_arm_monolithic_58:
  if ((instruction & (1u << 28)) == 0)
  {
    goto autodecoder_arm_monolithic_25;
  }

  if (instruction & (1u << 26))
  {
    goto autodecoder_arm_undefined;
  }

  if ((instruction & (1u << 25)) == 0)
  {
    goto autodecoder_arm_monolithic_55;
  }
  handler->barePtr = (void *)(armBlxImmediateInstruction);
  return IRC_REPLACE;


autodecoder_arm_monolithic_55:
  if ((instruction & (1u << 4)) == 0)
  {
    goto autodecoder_arm_monolithic_54;
  }

  if (instruction & (1u << 7))
  {
    goto autodecoder_arm_undefined;
  }

  if (instruction & (1u << 8))
  {
    goto autodecoder_arm_monolithic_38;
  }
  goto autodecoder_arm_undefined;

autodecoder_arm_monolithic_54:
  if (instruction & (1u << 7))
  {
    goto autodecoder_arm_undefined;
  }

  if ((instruction & (1u << 8)) == 0)
  {
    goto autodecoder_arm_monolithic_52;
  }

autodecoder_arm_monolithic_38:
  if ((instruction & (1u << 10)) == 0)
  {
    goto autodecoder_arm_undefined;
  }

  if (instruction & (1u << 11))
  {
    goto autodecoder_arm_undefined;
  }

  if (instruction & (1u << 9))
  {
    goto autodecoder_arm_undefined;
  }

  if ((instruction & (1u << 22)) == 0)
  {
    goto autodecoder_arm_undefined;
  }

  if (instruction & (1u << 20))
  {
    goto autodecoder_arm_undefined;
  }

  if ((instruction & (1u << 16)) == 0)
  {
    goto autodecoder_arm_undefined;
  }

  if ((instruction & (1u << 19)) == 0)
  {
    goto autodecoder_arm_undefined;
  }

  if ((instruction & (1u << 18)) == 0)
  {
    goto autodecoder_arm_undefined;
  }

  if (instruction & (1u << 17))
  {
    goto autodecoder_arm_undefined;
  }

  if (instruction & (1u << 5))
  {
    goto autodecoder_arm_undefined;
  }

  if (instruction & (1u << 6))
  {
    goto autodecoder_arm_undefined;
  }
  handler->barePtr = (void *)(armSrsInstruction);
  return IRC_REPLACE;


autodecoder_arm_monolithic_52:
  if (instruction & (1u << 10))
  {
    goto autodecoder_arm_undefined;
  }

  if ((instruction & (1u << 11)) == 0)
  {
    goto autodecoder_arm_undefined;
  }

  if ((instruction & (1u << 9)) == 0)
  {
    goto autodecoder_arm_undefined;
  }

  if (instruction & (1u << 22))
  {
    goto autodecoder_arm_undefined;
  }

  if ((instruction & (1u << 20)) == 0)
  {
    goto autodecoder_arm_undefined;
  }

  if (instruction & (1u << 5))
  {
    goto autodecoder_arm_undefined;
  }

  if (instruction & (1u << 6))
  {
    goto autodecoder_arm_undefined;
  }

  if (instruction & (1u << 3))
  {
    goto autodecoder_arm_undefined;
  }

  if (instruction & (1u << 2))
  {
    goto autodecoder_arm_undefined;
  }

  if (instruction & (1u << 1))
  {
    goto autodecoder_arm_undefined;
  }

  if (instruction & (1u << 0))
  {
    goto autodecoder_arm_undefined;
  }
  handler->barePtr = (void *)(armRfeInstruction);
  return IRC_REPLACE;


autodecoder_arm_monolithic_538:
  if ((instruction & (1u << 15)) == 0)
  {
    goto autodecoder_arm_monolithic_537;
  }

  if ((instruction & (1u << 12)) == 0)
  {
    goto autodecoder_arm_monolithic_484;
  }

  if ((instruction & (1u << 13)) == 0)
  {
    goto autodecoder_arm_monolithic_484;
  }

  if ((instruction & (1u << 14)) == 0)
  {
    goto autodecoder_arm_monolithic_484;
  }

  if ((instruction & (1u << 28)) == 0)
  {
    goto autodecoder_arm_monolithic_431;
  }

  if ((instruction & (1u << 26)) == 0)
  {
    goto autodecoder_arm_undefined;
  }

  if ((instruction & (1u << 25)) == 0)
  {
    goto autodecoder_arm_monolithic_105;
  }

  if ((instruction & (1u << 24)) == 0)
  {
    goto autodecoder_arm_monolithic_69;
  }

  if (instruction & (1u << 4))
  {
    goto autodecoder_arm_undefined;
  }
  goto autodecoder_arm_monolithic_65;

autodecoder_arm_monolithic_69:
  if (instruction & (1u << 4))
  {
    goto autodecoder_arm_undefined;
  }
  goto autodecoder_arm_monolithic_68;

autodecoder_arm_monolithic_105:
  if ((instruction & (1u << 24)) == 0)
  {
    goto autodecoder_arm_monolithic_68;
  }

  if ((instruction & (1u << 4)) == 0)
  {
    goto autodecoder_arm_monolithic_103;
  }

  if (instruction & (1u << 7))
  {
    goto autodecoder_arm_monolithic_65;
  }

  if (instruction & (1u << 8))
  {
    goto autodecoder_arm_monolithic_65;
  }

  if (instruction & (1u << 10))
  {
    goto autodecoder_arm_monolithic_65;
  }

  if (instruction & (1u << 11))
  {
    goto autodecoder_arm_monolithic_65;
  }

  if (instruction & (1u << 9))
  {
    goto autodecoder_arm_monolithic_65;
  }

  if (instruction & (1u << 23))
  {
    goto autodecoder_arm_monolithic_65;
  }

  if ((instruction & (1u << 21)) == 0)
  {
    goto autodecoder_arm_monolithic_64;
  }

  if ((instruction & (1u << 22)) == 0)
  {
    goto autodecoder_arm_undefined;
  }

  if ((instruction & (1u << 20)) == 0)
  {
    goto autodecoder_arm_undefined;
  }

  if ((instruction & (1u << 16)) == 0)
  {
    goto autodecoder_arm_undefined;
  }

  if ((instruction & (1u << 19)) == 0)
  {
    goto autodecoder_arm_undefined;
  }

  if ((instruction & (1u << 18)) == 0)
  {
    goto autodecoder_arm_undefined;
  }

  if ((instruction & (1u << 17)) == 0)
  {
    goto autodecoder_arm_undefined;
  }

  if (instruction & (1u << 5))
  {
    goto autodecoder_arm_undefined;
  }

  if (instruction & (1u << 6))
  {
    return IRC_REMOVE;
  }
  goto autodecoder_arm_monolithic_74;

autodecoder_arm_monolithic_103:
  if (instruction & (1u << 7))
  {
    goto autodecoder_arm_monolithic_65;
  }

  if (instruction & (1u << 8))
  {
    goto autodecoder_arm_monolithic_65;
  }

  if (instruction & (1u << 10))
  {
    goto autodecoder_arm_monolithic_65;
  }

  if (instruction & (1u << 11))
  {
    goto autodecoder_arm_monolithic_65;
  }

  if (instruction & (1u << 9))
  {
    goto autodecoder_arm_monolithic_65;
  }

  if ((instruction & (1u << 23)) == 0)
  {
    goto autodecoder_arm_monolithic_97;
  }

autodecoder_arm_monolithic_65:
  if (instruction & (1u << 21))
  {
    goto autodecoder_arm_undefined;
  }
  goto autodecoder_arm_monolithic_64;

autodecoder_arm_monolithic_97:
  if ((instruction & (1u << 21)) == 0)
  {
    goto autodecoder_arm_monolithic_64;
  }

  if ((instruction & (1u << 22)) == 0)
  {
    goto autodecoder_arm_undefined;
  }

  if ((instruction & (1u << 20)) == 0)
  {
    goto autodecoder_arm_undefined;
  }

  if ((instruction & (1u << 16)) == 0)
  {
    goto autodecoder_arm_undefined;
  }

  if ((instruction & (1u << 19)) == 0)
  {
    goto autodecoder_arm_undefined;
  }

  if ((instruction & (1u << 18)) == 0)
  {
    goto autodecoder_arm_undefined;
  }

  if ((instruction & (1u << 17)) == 0)
  {
    goto autodecoder_arm_undefined;
  }

  if (instruction & (1u << 6))
  {
    return IRC_REMOVE;
  }
  goto autodecoder_arm_undefined;

autodecoder_arm_monolithic_68:
  if (instruction & (1u << 21))
  {
    goto autodecoder_arm_undefined;
  }

  if ((instruction & (1u << 22)) == 0)
  {
    goto autodecoder_arm_undefined;
  }

autodecoder_arm_monolithic_64:
  if ((instruction & (1u << 20)) == 0)
  {
    goto autodecoder_arm_undefined;
  }
  return IRC_REMOVE;


autodecoder_arm_monolithic_431:
  if (instruction & (1u << 26))
  {
    goto autodecoder_arm_monolithic_183;
  }

  if (instruction & (1u << 25))
  {
    goto autodecoder_arm_monolithic_259;
  }

  if ((instruction & (1u << 24)) == 0)
  {
    goto autodecoder_arm_monolithic_428;
  }

  if ((instruction & (1u << 4)) == 0)
  {
    goto autodecoder_arm_monolithic_393;
  }

  if (instruction & (1u << 7))
  {
    goto autodecoder_arm_monolithic_291;
  }

  if ((instruction & (1u << 8)) == 0)
  {
    goto autodecoder_arm_monolithic_327;
  }

  if ((instruction & (1u << 10)) == 0)
  {
    goto autodecoder_arm_monolithic_327;
  }

  if ((instruction & (1u << 11)) == 0)
  {
    goto autodecoder_arm_monolithic_327;
  }

  if ((instruction & (1u << 9)) == 0)
  {
    goto autodecoder_arm_monolithic_327;
  }

  if (instruction & (1u << 23))
  {
    goto autodecoder_arm_monolithic_303;
  }

  if ((instruction & (1u << 21)) == 0)
  {
    return IRC_SAFE;
  }

  if (instruction & (1u << 22))
  {
    goto autodecoder_arm_monolithic_307;
  }

  if (instruction & (1u << 20))
  {
    return IRC_SAFE;
  }

  if ((instruction & (1u << 16)) == 0)
  {
    goto autodecoder_arm_monolithic_315;
  }

  if ((instruction & (1u << 19)) == 0)
  {
    goto autodecoder_arm_monolithic_315;
  }

  if ((instruction & (1u << 18)) == 0)
  {
    goto autodecoder_arm_monolithic_315;
  }

  if ((instruction & (1u << 17)) == 0)
  {
    goto autodecoder_arm_monolithic_315;
  }

  if ((instruction & (1u << 5)) == 0)
  {
    goto autodecoder_arm_monolithic_312;
  }

  if (instruction & (1u << 6))
  {
    handler->barePtr = (void *)(armBkptInstruction);
    return IRC_REPLACE;
  }
  handler->barePtr = (void *)(armBlxRegisterInstruction);
  return IRC_REPLACE;

autodecoder_arm_monolithic_327:
  if (instruction & (1u << 23))
  {
    goto autodecoder_arm_monolithic_303;
  }
  goto autodecoder_arm_monolithic_326;

autodecoder_arm_monolithic_537:
  if (instruction & (1u << 12))
  {
    goto autodecoder_arm_monolithic_484;
  }

  if (instruction & (1u << 13))
  {
    goto autodecoder_arm_monolithic_484;
  }

  if ((instruction & (1u << 14)) == 0)
  {
    goto autodecoder_arm_monolithic_534;
  }

autodecoder_arm_monolithic_484:
  if (instruction & (1u << 28))
  {
    goto autodecoder_arm_undefined;
  }

  if (instruction & (1u << 26))
  {
    goto autodecoder_arm_monolithic_439;
  }

  if (instruction & (1u << 25))
  {
    goto autodecoder_arm_monolithic_449;
  }

  if ((instruction & (1u << 24)) == 0)
  {
    goto autodecoder_arm_monolithic_480;
  }

  if (instruction & (1u << 4))
  {
    goto autodecoder_arm_monolithic_452;
  }
  goto autodecoder_arm_monolithic_477;

autodecoder_arm_monolithic_534:
  if ((instruction & (1u << 28)) == 0)
  {
    goto autodecoder_arm_monolithic_533;
  }

  if (instruction & (1u << 26))
  {
    goto autodecoder_arm_undefined;
  }

  if (instruction & (1u << 25))
  {
    goto autodecoder_arm_undefined;
  }

  if ((instruction & (1u << 24)) == 0)
  {
    goto autodecoder_arm_undefined;
  }

  if (instruction & (1u << 10))
  {
    goto autodecoder_arm_undefined;
  }

  if (instruction & (1u << 11))
  {
    goto autodecoder_arm_undefined;
  }

  if ((instruction & (1u << 9)) == 0)
  {
    goto autodecoder_arm_monolithic_503;
  }

  if (instruction & (1u << 23))
  {
    goto autodecoder_arm_undefined;
  }

  if (instruction & (1u << 21))
  {
    goto autodecoder_arm_undefined;
  }

  if (instruction & (1u << 22))
  {
    goto autodecoder_arm_undefined;
  }

  if (instruction & (1u << 20))
  {
    goto autodecoder_arm_undefined;
  }

  if (instruction & (1u << 16))
  {
    goto autodecoder_arm_monolithic_491;
  }
  goto autodecoder_arm_undefined;

autodecoder_arm_monolithic_503:
  if (instruction & (1u << 23))
  {
    goto autodecoder_arm_undefined;
  }

  if (instruction & (1u << 21))
  {
    goto autodecoder_arm_undefined;
  }

  if (instruction & (1u << 22))
  {
    goto autodecoder_arm_undefined;
  }

  if (instruction & (1u << 20))
  {
    goto autodecoder_arm_undefined;
  }

  if ((instruction & (1u << 16)) == 0)
  {
    goto autodecoder_arm_monolithic_498;
  }

autodecoder_arm_monolithic_491:
  if (instruction & (1u << 19))
  {
    goto autodecoder_arm_undefined;
  }

  if (instruction & (1u << 18))
  {
    goto autodecoder_arm_undefined;
  }

  if (instruction & (1u << 17))
  {
    goto autodecoder_arm_undefined;
  }
  handler->barePtr = (void *)(armSetendInstruction);
  return IRC_REPLACE;


autodecoder_arm_monolithic_498:
  if (instruction & (1u << 5))
  {
    goto autodecoder_arm_undefined;
  }
  handler->barePtr = (void *)(armCpsInstruction);
  return IRC_REPLACE;


autodecoder_arm_monolithic_533:
  if (instruction & (1u << 26))
  {
    goto autodecoder_arm_monolithic_439;
  }

  if (instruction & (1u << 25))
  {
    goto autodecoder_arm_monolithic_449;
  }

  if ((instruction & (1u << 24)) == 0)
  {
    goto autodecoder_arm_monolithic_480;
  }

  if ((instruction & (1u << 4)) == 0)
  {
    goto autodecoder_arm_monolithic_529;
  }

autodecoder_arm_monolithic_452:
  if (instruction & (1u << 7))
  {
    goto autodecoder_arm_monolithic_291;
  }

  if (instruction & (1u << 23))
  {
    goto autodecoder_arm_monolithic_450;
  }

autodecoder_arm_monolithic_326:
  if ((instruction & (1u << 21)) == 0)
  {
    return IRC_SAFE;
  }

  if (instruction & (1u << 22))
  {
    goto autodecoder_arm_monolithic_307;
  }

  if (instruction & (1u << 20))
  {
    return IRC_SAFE;
  }

autodecoder_arm_monolithic_315:
  if ((instruction & (1u << 5)) == 0)
  {
    return IRC_SAFE;
  }

  if ((instruction & (1u << 6)) == 0)
  {
    return IRC_SAFE;
  }
  handler->barePtr = (void *)(armBkptInstruction);
  return IRC_REPLACE;


autodecoder_arm_monolithic_529:
  if (instruction & (1u << 7))
  {
    goto autodecoder_arm_monolithic_510;
  }

  if (instruction & (1u << 8))
  {
    goto autodecoder_arm_monolithic_461;
  }

  if (instruction & (1u << 10))
  {
    goto autodecoder_arm_monolithic_461;
  }

  if (instruction & (1u << 11))
  {
    goto autodecoder_arm_monolithic_461;
  }

  if (instruction & (1u << 9))
  {
    goto autodecoder_arm_monolithic_461;
  }

  if ((instruction & (1u << 23)) == 0)
  {
    goto autodecoder_arm_monolithic_471;
  }

  if ((instruction & (1u << 21)) == 0)
  {
    handler->barePtr = (void *)(armALUImmRegRSR);
    return IRC_PATCH_PC;
  }

  if (instruction & (1u << 22))
  {
    handler->barePtr = (void *)(armShiftPCImm);
    return IRC_PATCH_PC;
  }

  if (instruction & (1u << 20))
  {
    goto autodecoder_arm_monolithic_468;
  }

  if (instruction & (1u << 16))
  {
    goto autodecoder_arm_undefined;
  }

  if (instruction & (1u << 19))
  {
    goto autodecoder_arm_undefined;
  }

  if (instruction & (1u << 18))
  {
    goto autodecoder_arm_undefined;
  }

  if (instruction & (1u << 17))
  {
    goto autodecoder_arm_undefined;
  }

  if (instruction & (1u << 5))
  {
    handler->barePtr = (void *)(armShiftPCImm);
    return IRC_PATCH_PC;
  }

  if (instruction & (1u << 6))
  {
    handler->barePtr = (void *)(armShiftPCImm);
    return IRC_PATCH_PC;
  }

  if (instruction & (1u << 3))
  {
    handler->barePtr = (void *)(armMovPCInstruction);
    return IRC_PATCH_PC;
  }

  if (instruction & (1u << 2))
  {
    handler->barePtr = (void *)(armMovPCInstruction);
    return IRC_PATCH_PC;
  }

  if (instruction & (1u << 1))
  {
    handler->barePtr = (void *)(armMovPCInstruction);
    return IRC_PATCH_PC;
  }

  if (instruction & (1u << 0))
  {
    handler->barePtr = (void *)(armMovPCInstruction);
    return IRC_PATCH_PC;
  }
  return IRC_SAFE;

autodecoder_arm_monolithic_581:
  if ((instruction & (1u << 27)) == 0)
  {
    goto autodecoder_arm_monolithic_580;
  }

  if ((instruction & (1u << 15)) == 0)
  {
    goto autodecoder_arm_monolithic_25;
  }

autodecoder_arm_monolithic_19:
  if (instruction & (1u << 26))
  {
    goto autodecoder_arm_monolithic_10;
  }

  if (instruction & (1u << 25))
  {
    handler->barePtr = (void *)(armBInstruction);
    return IRC_REPLACE;
  }

  if (instruction & (1u << 22))
  {
    goto autodecoder_arm_monolithic_14;
  }

  if (instruction & (1u << 20))
  {
    handler->barePtr = (void *)(armLdmInstruction);
    return IRC_REPLACE;
  }
  handler->barePtr = (void *)(armStmPC);
  return IRC_PATCH_PC;

autodecoder_arm_monolithic_25:
  if ((instruction & (1u << 26)) == 0)
  {
    goto autodecoder_arm_monolithic_24;
  }

autodecoder_arm_monolithic_10:
  if ((instruction & (1u << 25)) == 0)
  {
    goto autodecoder_arm_undefined;
  }

  if ((instruction & (1u << 24)) == 0)
  {
    goto autodecoder_arm_monolithic_8;
  }
  handler->barePtr = (void *)(svcInstruction);
  return IRC_REPLACE;


autodecoder_arm_monolithic_8:
  if ((instruction & (1u << 4)) == 0)
  {
    goto autodecoder_arm_undefined;
  }

  if ((instruction & (1u << 20)) == 0)
  {
    handler->barePtr = (void *)(armMcrInstruction);
    return IRC_REPLACE;
  }
  handler->barePtr = (void *)(armMrcInstruction);
  return IRC_REPLACE;



autodecoder_arm_monolithic_24:
  if ((instruction & (1u << 25)) == 0)
  {
    goto autodecoder_arm_monolithic_23;
  }
  handler->barePtr = (void *)(armBInstruction);
  return IRC_REPLACE;


autodecoder_arm_monolithic_23:
  if ((instruction & (1u << 22)) == 0)
  {
    goto autodecoder_arm_monolithic_22;
  }

autodecoder_arm_monolithic_14:
  if ((instruction & (1u << 20)) == 0)
  {
    handler->barePtr = (void *)(armStmInstruction);
    return IRC_REPLACE;
  }
  handler->barePtr = (void *)(armLdmInstruction);
  return IRC_REPLACE;



autodecoder_arm_monolithic_22:
  if (instruction & (1u << 20))
  {
    return IRC_SAFE;
  }
  handler->barePtr = (void *)(armStmPC);
  return IRC_PATCH_PC;


autodecoder_arm_monolithic_580:
  if ((instruction & (1u << 15)) == 0)
  {
    goto autodecoder_arm_monolithic_579;
  }

  if ((instruction & (1u << 12)) == 0)
  {
    goto autodecoder_arm_monolithic_568;
  }

  if ((instruction & (1u << 13)) == 0)
  {
    goto autodecoder_arm_monolithic_568;
  }

  if ((instruction & (1u << 14)) == 0)
  {
    goto autodecoder_arm_monolithic_568;
  }

  if ((instruction & (1u << 26)) == 0)
  {
    goto autodecoder_arm_monolithic_561;
  }

autodecoder_arm_monolithic_183:
  if ((instruction & (1u << 25)) == 0)
  {
    goto autodecoder_arm_monolithic_182;
  }

  if ((instruction & (1u << 24)) == 0)
  {
    goto autodecoder_arm_monolithic_179;
  }

  if (instruction & (1u << 4))
  {
    goto autodecoder_arm_monolithic_123;
  }
  goto autodecoder_arm_monolithic_129;

autodecoder_arm_monolithic_179:
  if (instruction & (1u << 4))
  {
    goto autodecoder_arm_monolithic_170;
  }

  if (instruction & (1u << 21))
  {
    goto autodecoder_arm_monolithic_177;
  }
  goto autodecoder_arm_monolithic_129;

autodecoder_arm_monolithic_182:
  if (instruction & (1u << 24))
  {
    goto autodecoder_arm_monolithic_129;
  }

  if (instruction & (1u << 21))
  {
    return IRC_SAFE;
  }

autodecoder_arm_monolithic_129:
  if (instruction & (1u << 22))
  {
    goto autodecoder_arm_monolithic_126;
  }

  if ((instruction & (1u << 20)) == 0)
  {
    handler->barePtr = (void *)(armStrPCInstruction);
    return IRC_PATCH_PC;
  }
  handler->barePtr = (void *)(armLdrInstruction);
  return IRC_REPLACE;


autodecoder_arm_monolithic_561:
  if ((instruction & (1u << 25)) == 0)
  {
    goto autodecoder_arm_monolithic_560;
  }

autodecoder_arm_monolithic_259:
  if ((instruction & (1u << 24)) == 0)
  {
    goto autodecoder_arm_monolithic_258;
  }

  if ((instruction & (1u << 4)) == 0)
  {
    goto autodecoder_arm_monolithic_242;
  }

  if ((instruction & (1u << 7)) == 0)
  {
    goto autodecoder_arm_monolithic_200;
  }

  if (instruction & (1u << 8))
  {
    goto autodecoder_arm_monolithic_200;
  }

  if (instruction & (1u << 10))
  {
    goto autodecoder_arm_monolithic_200;
  }

  if (instruction & (1u << 11))
  {
    goto autodecoder_arm_monolithic_200;
  }

  if (instruction & (1u << 9))
  {
    goto autodecoder_arm_monolithic_200;
  }

  if (instruction & (1u << 23))
  {
    goto autodecoder_arm_monolithic_194;
  }

  if ((instruction & (1u << 21)) == 0)
  {
    goto autodecoder_arm_monolithic_198;
  }

  if (instruction & (1u << 22))
  {
    goto autodecoder_arm_monolithic_197;
  }

  if (instruction & (1u << 20))
  {
    handler->barePtr = (void *)(armALUImmRegRSRNoDest);
    return IRC_PATCH_PC;
  }

  if (instruction & (1u << 16))
  {
    handler->barePtr = (void *)(armMsrInstruction);
    return IRC_REPLACE;
  }

  if (instruction & (1u << 19))
  {
    handler->barePtr = (void *)(armMsrInstruction);
    return IRC_REPLACE;
  }

  if (instruction & (1u << 18))
  {
    handler->barePtr = (void *)(armMsrInstruction);
    return IRC_REPLACE;
  }

  if (instruction & (1u << 17))
  {
    handler->barePtr = (void *)(armMsrInstruction);
    return IRC_REPLACE;
  }

  if ((instruction & (1u << 5)) == 0)
  {
    handler->barePtr = (void *)(armMsrInstruction);
    return IRC_REPLACE;
  }

  if ((instruction & (1u << 6)) == 0)
  {
    handler->barePtr = (void *)(armMsrInstruction);
    return IRC_REPLACE;
  }
  handler->barePtr = (void *)(armDbgInstruction);
  return IRC_REPLACE;


autodecoder_arm_monolithic_242:
  if (instruction & (1u << 7))
  {
    goto autodecoder_arm_monolithic_200;
  }

  if (instruction & (1u << 8))
  {
    goto autodecoder_arm_monolithic_200;
  }

  if (instruction & (1u << 10))
  {
    goto autodecoder_arm_monolithic_200;
  }

  if (instruction & (1u << 11))
  {
    goto autodecoder_arm_monolithic_200;
  }

  if ((instruction & (1u << 9)) == 0)
  {
    goto autodecoder_arm_monolithic_237;
  }

autodecoder_arm_monolithic_200:
  if (instruction & (1u << 23))
  {
    goto autodecoder_arm_monolithic_194;
  }

  if (instruction & (1u << 21))
  {
    goto autodecoder_arm_monolithic_197;
  }
  goto autodecoder_arm_monolithic_198;

autodecoder_arm_monolithic_237:
  if ((instruction & (1u << 23)) == 0)
  {
    goto autodecoder_arm_monolithic_236;
  }

autodecoder_arm_monolithic_194:
  if ((instruction & (1u << 21)) == 0)
  {
    goto autodecoder_arm_monolithic_193;
  }

  if (instruction & (1u << 22))
  {
    handler->barePtr = (void *)(armMvnInstruction);
    return IRC_REPLACE;
  }

  if (instruction & (1u << 16))
  {
    goto autodecoder_arm_undefined;
  }

  if (instruction & (1u << 19))
  {
    goto autodecoder_arm_undefined;
  }

  if (instruction & (1u << 18))
  {
    goto autodecoder_arm_undefined;
  }

  if (instruction & (1u << 17))
  {
    goto autodecoder_arm_undefined;
  }
  handler->barePtr = (void *)(armMovInstruction);
  return IRC_REPLACE;

autodecoder_arm_monolithic_236:
  if ((instruction & (1u << 21)) == 0)
  {
    goto autodecoder_arm_monolithic_198;
  }

  if ((instruction & (1u << 22)) == 0)
  {
    goto autodecoder_arm_monolithic_234;
  }

autodecoder_arm_monolithic_197:
  if (instruction & (1u << 20))
  {
    handler->barePtr = (void *)(armALUImmRegRSRNoDest);
    return IRC_PATCH_PC;
  }
  handler->barePtr = (void *)(armMsrInstruction);
  return IRC_REPLACE;

autodecoder_arm_monolithic_234:
  if (instruction & (1u << 20))
  {
    handler->barePtr = (void *)(armALUImmRegRSRNoDest);
    return IRC_PATCH_PC;
  }

  if (instruction & (1u << 16))
  {
    handler->barePtr = (void *)(armMsrInstruction);
    return IRC_REPLACE;
  }

  if (instruction & (1u << 19))
  {
    handler->barePtr = (void *)(armMsrInstruction);
    return IRC_REPLACE;
  }

  if (instruction & (1u << 18))
  {
    handler->barePtr = (void *)(armMsrInstruction);
    return IRC_REPLACE;
  }

  if (instruction & (1u << 17))
  {
    handler->barePtr = (void *)(armMsrInstruction);
    return IRC_REPLACE;
  }

  if (instruction & (1u << 5))
  {
    handler->barePtr = (void *)(armMsrInstruction);
    return IRC_REPLACE;
  }

  if (instruction & (1u << 6))
  {
    handler->barePtr = (void *)(armMsrInstruction);
    return IRC_REPLACE;
  }

  if (instruction & (1u << 3))
  {
    handler->barePtr = (void *)(armMsrInstruction);
    return IRC_REPLACE;
  }

  if ((instruction & (1u << 2)) == 0)
  {
    goto autodecoder_arm_monolithic_225;
  }

  if (instruction & (1u << 1))
  {
    handler->barePtr = (void *)(armMsrInstruction);
    return IRC_REPLACE;
  }

  if (instruction & (1u << 0))
  {
    handler->barePtr = (void *)(armMsrInstruction);
    return IRC_REPLACE;
  }
  handler->barePtr = (void *)(armSevInstruction);
  return IRC_REPLACE;


autodecoder_arm_monolithic_225:
  if ((instruction & (1u << 1)) == 0)
  {
    goto autodecoder_arm_monolithic_224;
  }

  if ((instruction & (1u << 0)) == 0)
  {
    handler->barePtr = (void *)(armWfeInstruction);
    return IRC_REPLACE;
  }
  handler->barePtr = (void *)(armWfiInstruction);
  return IRC_REPLACE;



autodecoder_arm_monolithic_224:
  if ((instruction & (1u << 0)) == 0)
  {
    return IRC_SAFE;
  }
  handler->barePtr = (void *)(armYieldInstruction);
  return IRC_REPLACE;


autodecoder_arm_monolithic_560:
  if ((instruction & (1u << 24)) == 0)
  {
    goto autodecoder_arm_monolithic_428;
  }

  if ((instruction & (1u << 4)) == 0)
  {
    goto autodecoder_arm_monolithic_393;
  }

  if (instruction & (1u << 7))
  {
    goto autodecoder_arm_monolithic_291;
  }

  if ((instruction & (1u << 8)) == 0)
  {
    goto autodecoder_arm_monolithic_553;
  }

  if ((instruction & (1u << 10)) == 0)
  {
    goto autodecoder_arm_monolithic_553;
  }

  if ((instruction & (1u << 11)) == 0)
  {
    goto autodecoder_arm_monolithic_553;
  }

  if ((instruction & (1u << 9)) == 0)
  {
    goto autodecoder_arm_monolithic_553;
  }

  if (instruction & (1u << 23))
  {
    goto autodecoder_arm_monolithic_303;
  }

  if ((instruction & (1u << 21)) == 0)
  {
    return IRC_SAFE;
  }

  if (instruction & (1u << 22))
  {
    goto autodecoder_arm_monolithic_307;
  }

  if (instruction & (1u << 20))
  {
    return IRC_SAFE;
  }

  if ((instruction & (1u << 16)) == 0)
  {
    return IRC_SAFE;
  }

  if ((instruction & (1u << 19)) == 0)
  {
    return IRC_SAFE;
  }

  if ((instruction & (1u << 18)) == 0)
  {
    return IRC_SAFE;
  }

  if ((instruction & (1u << 17)) == 0)
  {
    return IRC_SAFE;
  }

  if ((instruction & (1u << 5)) == 0)
  {
    goto autodecoder_arm_monolithic_312;
  }

  if (instruction & (1u << 6))
  {
    return IRC_SAFE;
  }
  handler->barePtr = (void *)(armBlxRegisterInstruction);
  return IRC_REPLACE;


autodecoder_arm_monolithic_312:
  if (instruction & (1u << 6))
  {
    return IRC_SAFE;
  }
  handler->barePtr = (void *)(armBxInstruction);
  return IRC_REPLACE;


autodecoder_arm_monolithic_553:
  if ((instruction & (1u << 23)) == 0)
  {
    goto autodecoder_arm_monolithic_552;
  }

autodecoder_arm_monolithic_303:
  if ((instruction & (1u << 21)) == 0)
  {
    goto autodecoder_arm_monolithic_193;
  }

  if (instruction & (1u << 22))
  {
    handler->barePtr = (void *)(armMvnInstruction);
    return IRC_REPLACE;
  }

  if (instruction & (1u << 16))
  {
    goto autodecoder_arm_undefined;
  }

  if (instruction & (1u << 19))
  {
    goto autodecoder_arm_undefined;
  }

  if (instruction & (1u << 18))
  {
    goto autodecoder_arm_undefined;
  }

  if (instruction & (1u << 17))
  {
    goto autodecoder_arm_undefined;
  }

  if ((instruction & (1u << 5)) == 0)
  {
    goto autodecoder_arm_monolithic_296;
  }

  if (instruction & (1u << 6))
  {
    return IRC_SAFE;
  }
  handler->barePtr = (void *)(armLsrInstruction);
  return IRC_REPLACE;

autodecoder_arm_monolithic_393:
  if ((instruction & (1u << 7)) == 0)
  {
    goto autodecoder_arm_monolithic_392;
  }

  if (instruction & (1u << 23))
  {
    goto autodecoder_arm_monolithic_341;
  }
  goto autodecoder_arm_monolithic_345;

autodecoder_arm_monolithic_392:
  if ((instruction & (1u << 8)) == 0)
  {
    goto autodecoder_arm_monolithic_391;
  }

  if ((instruction & (1u << 10)) == 0)
  {
    goto autodecoder_arm_monolithic_358;
  }

  if ((instruction & (1u << 11)) == 0)
  {
    goto autodecoder_arm_monolithic_358;
  }

  if ((instruction & (1u << 9)) == 0)
  {
    goto autodecoder_arm_monolithic_358;
  }

  if (instruction & (1u << 23))
  {
    goto autodecoder_arm_monolithic_341;
  }

  if ((instruction & (1u << 21)) == 0)
  {
    handler->barePtr = (void *)(armALUImmRegRSRNoDest);
    return IRC_PATCH_PC;
  }

  if (instruction & (1u << 22))
  {
    handler->barePtr = (void *)(armALUImmRegRSRNoDest);
    return IRC_PATCH_PC;
  }

  if (instruction & (1u << 20))
  {
    handler->barePtr = (void *)(armALUImmRegRSRNoDest);
    return IRC_PATCH_PC;
  }

  if ((instruction & (1u << 16)) == 0)
  {
    handler->barePtr = (void *)(armALUImmRegRSRNoDest);
    return IRC_PATCH_PC;
  }

  if ((instruction & (1u << 19)) == 0)
  {
    handler->barePtr = (void *)(armALUImmRegRSRNoDest);
    return IRC_PATCH_PC;
  }

  if ((instruction & (1u << 18)) == 0)
  {
    handler->barePtr = (void *)(armALUImmRegRSRNoDest);
    return IRC_PATCH_PC;
  }

  if ((instruction & (1u << 17)) == 0)
  {
    handler->barePtr = (void *)(armALUImmRegRSRNoDest);
    return IRC_PATCH_PC;
  }

  if ((instruction & (1u << 5)) == 0)
  {
    handler->barePtr = (void *)(armALUImmRegRSRNoDest);
    return IRC_PATCH_PC;
  }

  if (instruction & (1u << 6))
  {
    handler->barePtr = (void *)(armALUImmRegRSRNoDest);
    return IRC_PATCH_PC;
  }
  handler->barePtr = (void *)(armBxjInstruction);
  return IRC_REPLACE;


autodecoder_arm_monolithic_391:
  if (instruction & (1u << 10))
  {
    goto autodecoder_arm_monolithic_358;
  }

  if (instruction & (1u << 11))
  {
    goto autodecoder_arm_monolithic_358;
  }

  if ((instruction & (1u << 9)) == 0)
  {
    goto autodecoder_arm_monolithic_388;
  }

autodecoder_arm_monolithic_358:
  if ((instruction & (1u << 23)) == 0)
  {
    handler->barePtr = (void *)(armALUImmRegRSRNoDest);
    return IRC_PATCH_PC;
  }

autodecoder_arm_monolithic_341:
  if ((instruction & (1u << 21)) == 0)
  {
    goto autodecoder_arm_monolithic_193;
  }

  if (instruction & (1u << 22))
  {
    handler->barePtr = (void *)(armMvnInstruction);
    return IRC_REPLACE;
  }

  if (instruction & (1u << 16))
  {
    goto autodecoder_arm_undefined;
  }

  if (instruction & (1u << 19))
  {
    goto autodecoder_arm_undefined;
  }

  if (instruction & (1u << 18))
  {
    goto autodecoder_arm_undefined;
  }

  if (instruction & (1u << 17))
  {
    goto autodecoder_arm_undefined;
  }

  if ((instruction & (1u << 5)) == 0)
  {
    goto autodecoder_arm_monolithic_296;
  }

  if ((instruction & (1u << 6)) == 0)
  {
    handler->barePtr = (void *)(armLsrInstruction);
    return IRC_REPLACE;
  }
  handler->barePtr = (void *)(armRorInstruction);
  return IRC_REPLACE;


autodecoder_arm_monolithic_296:
  if (instruction & (1u << 6))
  {
    handler->barePtr = (void *)(armAsrInstruction);
    return IRC_REPLACE;
  }
  handler->barePtr = (void *)(armLslInstruction);
  return IRC_REPLACE;


autodecoder_arm_monolithic_388:
  if ((instruction & (1u << 23)) == 0)
  {
    goto autodecoder_arm_monolithic_387;
  }

  if ((instruction & (1u << 21)) == 0)
  {
    goto autodecoder_arm_monolithic_193;
  }

  if ((instruction & (1u << 22)) == 0)
  {
    goto autodecoder_arm_monolithic_369;
  }
  handler->barePtr = (void *)(armMvnInstruction);
  return IRC_REPLACE;


autodecoder_arm_monolithic_369:
  if (instruction & (1u << 16))
  {
    goto autodecoder_arm_undefined;
  }

  if (instruction & (1u << 19))
  {
    goto autodecoder_arm_undefined;
  }

  if (instruction & (1u << 18))
  {
    goto autodecoder_arm_undefined;
  }

  if (instruction & (1u << 17))
  {
    goto autodecoder_arm_undefined;
  }

  if ((instruction & (1u << 5)) == 0)
  {
    goto autodecoder_arm_monolithic_364;
  }

  if ((instruction & (1u << 6)) == 0)
  {
    handler->barePtr = (void *)(armLsrInstruction);
    return IRC_REPLACE;
  }
  handler->barePtr = (void *)(armRrxInstruction);
  return IRC_REPLACE;



autodecoder_arm_monolithic_364:
  if ((instruction & (1u << 6)) == 0)
  {
    handler->barePtr = (void *)(armMovInstruction);
    return IRC_REPLACE;
  }
  handler->barePtr = (void *)(armAsrInstruction);
  return IRC_REPLACE;



autodecoder_arm_monolithic_193:
  if ((instruction & (1u << 22)) == 0)
  {
    handler->barePtr = (void *)(armOrrInstruction);
    return IRC_REPLACE;
  }
  handler->barePtr = (void *)(armBicInstruction);
  return IRC_REPLACE;



autodecoder_arm_monolithic_387:
  if ((instruction & (1u << 21)) == 0)
  {
    goto autodecoder_arm_monolithic_386;
  }

  if (instruction & (1u << 20))
  {
    handler->barePtr = (void *)(armALUImmRegRSRNoDest);
    return IRC_PATCH_PC;
  }

  if (instruction & (1u << 5))
  {
    handler->barePtr = (void *)(armALUImmRegRSRNoDest);
    return IRC_PATCH_PC;
  }

  if (instruction & (1u << 6))
  {
    handler->barePtr = (void *)(armALUImmRegRSRNoDest);
    return IRC_PATCH_PC;
  }
  handler->barePtr = (void *)(armMsrInstruction);
  return IRC_REPLACE;


autodecoder_arm_monolithic_428:
  if ((instruction & (1u << 4)) == 0)
  {
    goto autodecoder_arm_monolithic_258;
  }

  if (instruction & (1u << 7))
  {
    goto autodecoder_arm_monolithic_426;
  }

autodecoder_arm_monolithic_258:
  if ((instruction & (1u << 23)) == 0)
  {
    goto autodecoder_arm_monolithic_257;
  }

  if ((instruction & (1u << 21)) == 0)
  {
    goto autodecoder_arm_monolithic_249;
  }

  if ((instruction & (1u << 22)) == 0)
  {
    handler->barePtr = (void *)(armAdcInstruction);
    return IRC_REPLACE;
  }
  handler->barePtr = (void *)(armRscInstruction);
  return IRC_REPLACE;



autodecoder_arm_monolithic_249:
  if ((instruction & (1u << 22)) == 0)
  {
    handler->barePtr = (void *)(armAddInstruction);
    return IRC_REPLACE;
  }
  handler->barePtr = (void *)(armSbcInstruction);
  return IRC_REPLACE;



autodecoder_arm_monolithic_257:
  if ((instruction & (1u << 21)) == 0)
  {
    goto autodecoder_arm_monolithic_256;
  }

  if ((instruction & (1u << 22)) == 0)
  {
    handler->barePtr = (void *)(armEorInstruction);
    return IRC_REPLACE;
  }
  handler->barePtr = (void *)(armRsbInstruction);
  return IRC_REPLACE;



autodecoder_arm_monolithic_256:
  if ((instruction & (1u << 22)) == 0)
  {
    handler->barePtr = (void *)(armAndInstruction);
    return IRC_REPLACE;
  }
  handler->barePtr = (void *)(armSubInstruction);
  return IRC_REPLACE;



autodecoder_arm_monolithic_579:
  if (instruction & (1u << 12))
  {
    goto autodecoder_arm_monolithic_568;
  }

  if (instruction & (1u << 13))
  {
    goto autodecoder_arm_monolithic_568;
  }

  if ((instruction & (1u << 14)) == 0)
  {
    goto autodecoder_arm_monolithic_576;
  }

autodecoder_arm_monolithic_568:
  if (instruction & (1u << 26))
  {
    goto autodecoder_arm_monolithic_439;
  }

  if (instruction & (1u << 25))
  {
    goto autodecoder_arm_monolithic_449;
  }

  if ((instruction & (1u << 24)) == 0)
  {
    goto autodecoder_arm_monolithic_480;
  }

  if (instruction & (1u << 4))
  {
    goto autodecoder_arm_monolithic_564;
  }

autodecoder_arm_monolithic_477:
  if ((instruction & (1u << 7)) == 0)
  {
    goto autodecoder_arm_monolithic_476;
  }

  if (instruction & (1u << 23))
  {
    goto autodecoder_arm_monolithic_459;
  }

autodecoder_arm_monolithic_345:
  if ((instruction & (1u << 21)) == 0)
  {
    goto autodecoder_arm_monolithic_198;
  }

  if (instruction & (1u << 22))
  {
    handler->barePtr = (void *)(armALUImmRegRSRNoDest);
    return IRC_PATCH_PC;
  }

  if (instruction & (1u << 20))
  {
    handler->barePtr = (void *)(armALUImmRegRSRNoDest);
    return IRC_PATCH_PC;
  }

  if (instruction & (1u << 5))
  {
    handler->barePtr = (void *)(armALUImmRegRSRNoDest);
    return IRC_PATCH_PC;
  }
  return IRC_SAFE;

autodecoder_arm_monolithic_576:
  if ((instruction & (1u << 26)) == 0)
  {
    goto autodecoder_arm_monolithic_575;
  }

autodecoder_arm_monolithic_439:
  if ((instruction & (1u << 25)) == 0)
  {
    goto autodecoder_arm_monolithic_438;
  }

  if ((instruction & (1u << 24)) == 0)
  {
    goto autodecoder_arm_monolithic_435;
  }

  if ((instruction & (1u << 4)) == 0)
  {
    goto autodecoder_arm_monolithic_126;
  }

autodecoder_arm_monolithic_123:
  if ((instruction & (1u << 7)) == 0)
  {
    goto autodecoder_arm_monolithic_122;
  }

  if (instruction & (1u << 23))
  {
    goto autodecoder_arm_monolithic_113;
  }

  if (instruction & (1u << 21))
  {
    goto autodecoder_arm_undefined;
  }

  if ((instruction & (1u << 22)) == 0)
  {
    goto autodecoder_arm_undefined;
  }

  if (instruction & (1u << 20))
  {
    goto autodecoder_arm_monolithic_108;
  }
  goto autodecoder_arm_undefined;

autodecoder_arm_monolithic_122:
  if ((instruction & (1u << 23)) == 0)
  {
    goto autodecoder_arm_monolithic_121;
  }

autodecoder_arm_monolithic_113:
  if ((instruction & (1u << 21)) == 0)
  {
    goto autodecoder_arm_monolithic_112;
  }

  if (instruction & (1u << 5))
  {
    goto autodecoder_arm_undefined;
  }
  goto autodecoder_arm_monolithic_108;

autodecoder_arm_monolithic_112:
  if (instruction & (1u << 22))
  {
    goto autodecoder_arm_monolithic_111;
  }
  goto autodecoder_arm_undefined;

autodecoder_arm_monolithic_121:
  if (instruction & (1u << 21))
  {
    goto autodecoder_arm_undefined;
  }

  if ((instruction & (1u << 22)) == 0)
  {
    goto autodecoder_arm_monolithic_119;
  }

  if (instruction & (1u << 20))
  {
    goto autodecoder_arm_monolithic_110;
  }
  return IRC_SAFE;

autodecoder_arm_monolithic_119:
  if (instruction & (1u << 20))
  {
    goto autodecoder_arm_undefined;
  }
  return IRC_SAFE;

autodecoder_arm_monolithic_435:
  if ((instruction & (1u << 4)) == 0)
  {
    goto autodecoder_arm_monolithic_434;
  }

autodecoder_arm_monolithic_170:
  if ((instruction & (1u << 7)) == 0)
  {
    goto autodecoder_arm_monolithic_169;
  }

  if ((instruction & (1u << 8)) == 0)
  {
    goto autodecoder_arm_monolithic_148;
  }

  if ((instruction & (1u << 10)) == 0)
  {
    goto autodecoder_arm_monolithic_148;
  }

  if ((instruction & (1u << 11)) == 0)
  {
    goto autodecoder_arm_monolithic_148;
  }

  if ((instruction & (1u << 9)) == 0)
  {
    goto autodecoder_arm_monolithic_148;
  }

  if ((instruction & (1u << 23)) == 0)
  {
    goto autodecoder_arm_monolithic_143;
  }

  if ((instruction & (1u << 21)) == 0)
  {
    goto autodecoder_arm_monolithic_139;
  }

  if (instruction & (1u << 20))
  {
    goto autodecoder_arm_monolithic_136;
  }
  goto autodecoder_arm_monolithic_132;

autodecoder_arm_monolithic_139:
  if (instruction & (1u << 22))
  {
    goto autodecoder_arm_undefined;
  }

  if (instruction & (1u << 20))
  {
    goto autodecoder_arm_undefined;
  }
  goto autodecoder_arm_monolithic_131;

autodecoder_arm_monolithic_143:
  if (instruction & (1u << 21))
  {
    goto autodecoder_arm_monolithic_141;
  }

  if ((instruction & (1u << 20)) == 0)
  {
    goto autodecoder_arm_undefined;
  }

autodecoder_arm_monolithic_141:
  if (instruction & (1u << 5))
  {
    goto autodecoder_arm_monolithic_108;
  }
  goto autodecoder_arm_monolithic_110;

autodecoder_arm_monolithic_169:
  if ((instruction & (1u << 8)) == 0)
  {
    goto autodecoder_arm_monolithic_168;
  }

  if ((instruction & (1u << 10)) == 0)
  {
    goto autodecoder_arm_monolithic_148;
  }

  if ((instruction & (1u << 11)) == 0)
  {
    goto autodecoder_arm_monolithic_148;
  }

  if ((instruction & (1u << 9)) == 0)
  {
    goto autodecoder_arm_monolithic_148;
  }

  if ((instruction & (1u << 23)) == 0)
  {
    goto autodecoder_arm_monolithic_156;
  }

  if ((instruction & (1u << 21)) == 0)
  {
    goto autodecoder_arm_monolithic_146;
  }

  if ((instruction & (1u << 20)) == 0)
  {
    goto autodecoder_arm_monolithic_131;
  }

autodecoder_arm_monolithic_136:
  if ((instruction & (1u << 16)) == 0)
  {
    goto autodecoder_arm_monolithic_132;
  }

  if ((instruction & (1u << 19)) == 0)
  {
    goto autodecoder_arm_monolithic_132;
  }

  if ((instruction & (1u << 18)) == 0)
  {
    goto autodecoder_arm_monolithic_132;
  }

  if ((instruction & (1u << 17)) == 0)
  {
    goto autodecoder_arm_monolithic_132;
  }

autodecoder_arm_monolithic_131:
  if (instruction & (1u << 5))
  {
    goto autodecoder_arm_monolithic_110;
  }
  return IRC_SAFE;

autodecoder_arm_monolithic_156:
  if (instruction & (1u << 21))
  {
    return IRC_SAFE;
  }

  if (instruction & (1u << 20))
  {
    return IRC_SAFE;
  }
  goto autodecoder_arm_undefined;

autodecoder_arm_monolithic_168:
  if ((instruction & (1u << 9)) == 0)
  {
    goto autodecoder_arm_monolithic_167;
  }

autodecoder_arm_monolithic_148:
  if ((instruction & (1u << 23)) == 0)
  {
    goto autodecoder_arm_undefined;
  }

  if (instruction & (1u << 21))
  {
    goto autodecoder_arm_monolithic_132;
  }

autodecoder_arm_monolithic_146:
  if (instruction & (1u << 22))
  {
    goto autodecoder_arm_undefined;
  }

  if (instruction & (1u << 20))
  {
    goto autodecoder_arm_undefined;
  }

autodecoder_arm_monolithic_132:
  if (instruction & (1u << 5))
  {
    goto autodecoder_arm_undefined;
  }
  return IRC_SAFE;

autodecoder_arm_monolithic_167:
  if ((instruction & (1u << 23)) == 0)
  {
    goto autodecoder_arm_undefined;
  }

  if (instruction & (1u << 21))
  {
    goto autodecoder_arm_monolithic_161;
  }

  if ((instruction & (1u << 22)) == 0)
  {
    goto autodecoder_arm_monolithic_164;
  }

  if (instruction & (1u << 20))
  {
    goto autodecoder_arm_undefined;
  }

  if (instruction & (1u << 5))
  {
    goto autodecoder_arm_monolithic_108;
  }
  goto autodecoder_arm_undefined;

autodecoder_arm_monolithic_164:
  if (instruction & (1u << 20))
  {
    goto autodecoder_arm_undefined;
  }

autodecoder_arm_monolithic_161:
  if ((instruction & (1u << 5)) == 0)
  {
    return IRC_SAFE;
  }

autodecoder_arm_monolithic_108:
  if (instruction & (1u << 6))
  {
    return IRC_SAFE;
  }
  goto autodecoder_arm_undefined;

autodecoder_arm_monolithic_434:
  if ((instruction & (1u << 21)) == 0)
  {
    goto autodecoder_arm_monolithic_126;
  }

autodecoder_arm_monolithic_177:
  if ((instruction & (1u << 22)) == 0)
  {
    goto autodecoder_arm_monolithic_176;
  }

  if ((instruction & (1u << 20)) == 0)
  {
    handler->barePtr = (void *)(armStrbtInstruction);
    return IRC_REPLACE;
  }
  handler->barePtr = (void *)(armLdrbtInstruction);
  return IRC_REPLACE;



autodecoder_arm_monolithic_176:
  if ((instruction & (1u << 20)) == 0)
  {
    handler->barePtr = (void *)(armStrtInstruction);
    return IRC_REPLACE;
  }
  handler->barePtr = (void *)(armLdrtInstruction);
  return IRC_REPLACE;



autodecoder_arm_monolithic_438:
  if (instruction & (1u << 24))
  {
    goto autodecoder_arm_monolithic_126;
  }

  if (instruction & (1u << 21))
  {
    return IRC_SAFE;
  }

autodecoder_arm_monolithic_126:
  if ((instruction & (1u << 20)) == 0)
  {
    handler->barePtr = (void *)(armStrPCInstruction);
    return IRC_PATCH_PC;
  }
  handler->barePtr = (void *)(armLdrPCInstruction);
  return IRC_PATCH_PC;


autodecoder_arm_monolithic_575:
  if ((instruction & (1u << 25)) == 0)
  {
    goto autodecoder_arm_monolithic_574;
  }

autodecoder_arm_monolithic_449:
  if ((instruction & (1u << 24)) == 0)
  {
    handler->barePtr = (void *)(armALUImmRegRSR);
    return IRC_PATCH_PC;
  }

  if ((instruction & (1u << 23)) == 0)
  {
    goto autodecoder_arm_monolithic_447;
  }

  if (instruction & (1u << 21))
  {
    goto autodecoder_arm_monolithic_444;
  }
  handler->barePtr = (void *)(armALUImmRegRSR);
  return IRC_PATCH_PC;

autodecoder_arm_monolithic_447:
  if (instruction & (1u << 21))
  {
    handler->barePtr = (void *)(armALUImmRegRSRNoDest);
    return IRC_PATCH_PC;
  }
  goto autodecoder_arm_monolithic_198;

autodecoder_arm_monolithic_574:
  if ((instruction & (1u << 24)) == 0)
  {
    goto autodecoder_arm_monolithic_480;
  }

  if ((instruction & (1u << 4)) == 0)
  {
    goto autodecoder_arm_monolithic_572;
  }

autodecoder_arm_monolithic_564:
  if ((instruction & (1u << 7)) == 0)
  {
    goto autodecoder_arm_monolithic_563;
  }

autodecoder_arm_monolithic_291:
  if ((instruction & (1u << 8)) == 0)
  {
    goto autodecoder_arm_monolithic_290;
  }

  if ((instruction & (1u << 10)) == 0)
  {
    goto autodecoder_arm_monolithic_277;
  }

  if ((instruction & (1u << 11)) == 0)
  {
    goto autodecoder_arm_monolithic_277;
  }

  if ((instruction & (1u << 9)) == 0)
  {
    goto autodecoder_arm_monolithic_277;
  }

  if ((instruction & (1u << 23)) == 0)
  {
    goto autodecoder_arm_monolithic_277;
  }

  if ((instruction & (1u << 22)) == 0)
  {
    goto autodecoder_arm_monolithic_272;
  }

  if ((instruction & (1u << 20)) == 0)
  {
    goto autodecoder_arm_monolithic_268;
  }

  if (instruction & (1u << 5))
  {
    handler->barePtr = (void *)(armLdrdhPCInstruction);
    return IRC_PATCH_PC;
  }

  if (instruction & (1u << 6))
  {
    handler->barePtr = (void *)(armLdrdhPCInstruction);
    return IRC_PATCH_PC;
  }

  if ((instruction & (1u << 3)) == 0)
  {
    handler->barePtr = (void *)(armLdrdhPCInstruction);
    return IRC_PATCH_PC;
  }

  if ((instruction & (1u << 2)) == 0)
  {
    handler->barePtr = (void *)(armLdrdhPCInstruction);
    return IRC_PATCH_PC;
  }

  if ((instruction & (1u << 1)) == 0)
  {
    handler->barePtr = (void *)(armLdrdhPCInstruction);
    return IRC_PATCH_PC;
  }

  if (instruction & (1u << 0))
  {
    return IRC_SAFE;
  }
  handler->barePtr = (void *)(armLdrdhPCInstruction);
  return IRC_PATCH_PC;

autodecoder_arm_monolithic_272:
  if ((instruction & (1u << 20)) == 0)
  {
    goto autodecoder_arm_monolithic_111;
  }

  if (instruction & (1u << 5))
  {
    goto autodecoder_arm_undefined;
  }

  if (instruction & (1u << 6))
  {
    goto autodecoder_arm_undefined;
  }

autodecoder_arm_monolithic_74:
  if ((instruction & (1u << 3)) == 0)
  {
    goto autodecoder_arm_undefined;
  }

  if ((instruction & (1u << 2)) == 0)
  {
    goto autodecoder_arm_undefined;
  }

  if ((instruction & (1u << 1)) == 0)
  {
    goto autodecoder_arm_undefined;
  }

  if (instruction & (1u << 0))
  {
    return IRC_SAFE;
  }
  goto autodecoder_arm_undefined;

autodecoder_arm_monolithic_290:
  if (instruction & (1u << 10))
  {
    goto autodecoder_arm_monolithic_277;
  }

  if (instruction & (1u << 11))
  {
    goto autodecoder_arm_monolithic_277;
  }

  if ((instruction & (1u << 9)) == 0)
  {
    goto autodecoder_arm_monolithic_287;
  }

autodecoder_arm_monolithic_277:
  if (instruction & (1u << 22))
  {
    goto autodecoder_arm_monolithic_276;
  }
  goto autodecoder_arm_undefined;

autodecoder_arm_monolithic_287:
  if (instruction & (1u << 23))
  {
    goto autodecoder_arm_monolithic_276;
  }

  if (instruction & (1u << 21))
  {
    goto autodecoder_arm_monolithic_276;
  }

  if (instruction & (1u << 20))
  {
    handler->barePtr = (void *)(armLdrdhPCInstruction);
    return IRC_PATCH_PC;
  }

  if (instruction & (1u << 5))
  {
    handler->barePtr = (void *)(armStrPCInstruction);
    return IRC_PATCH_PC;
  }

  if (instruction & (1u << 6))
  {
    handler->barePtr = (void *)(armLdrdhPCInstruction);
    return IRC_PATCH_PC;
  }
  handler->barePtr = (void *)(armSwpInstruction);
  return IRC_REPLACE;


autodecoder_arm_monolithic_563:
  if ((instruction & (1u << 23)) == 0)
  {
    goto autodecoder_arm_monolithic_552;
  }

autodecoder_arm_monolithic_450:
  if ((instruction & (1u << 21)) == 0)
  {
    return IRC_SAFE;
  }

autodecoder_arm_monolithic_444:
  if (instruction & (1u << 22))
  {
    return IRC_SAFE;
  }

  if (instruction & (1u << 16))
  {
    goto autodecoder_arm_undefined;
  }

  if (instruction & (1u << 19))
  {
    goto autodecoder_arm_undefined;
  }

  if (instruction & (1u << 18))
  {
    goto autodecoder_arm_undefined;
  }

  if (instruction & (1u << 17))
  {
    goto autodecoder_arm_undefined;
  }
  return IRC_SAFE;

autodecoder_arm_monolithic_552:
  if ((instruction & (1u << 21)) == 0)
  {
    return IRC_SAFE;
  }

  if ((instruction & (1u << 22)) == 0)
  {
    return IRC_SAFE;
  }

autodecoder_arm_monolithic_307:
  if (instruction & (1u << 20))
  {
    return IRC_SAFE;
  }

  if ((instruction & (1u << 5)) == 0)
  {
    return IRC_SAFE;
  }

  if ((instruction & (1u << 6)) == 0)
  {
    return IRC_SAFE;
  }
  handler->barePtr = (void *)(armSmcInstruction);
  return IRC_REPLACE;


autodecoder_arm_monolithic_572:
  if ((instruction & (1u << 7)) == 0)
  {
    goto autodecoder_arm_monolithic_476;
  }

autodecoder_arm_monolithic_510:
  if (instruction & (1u << 23))
  {
    goto autodecoder_arm_monolithic_459;
  }

autodecoder_arm_monolithic_198:
  if (instruction & (1u << 20))
  {
    handler->barePtr = (void *)(armALUImmRegRSRNoDest);
    return IRC_PATCH_PC;
  }
  return IRC_SAFE;

autodecoder_arm_monolithic_476:
  if (instruction & (1u << 8))
  {
    goto autodecoder_arm_monolithic_461;
  }

  if (instruction & (1u << 10))
  {
    goto autodecoder_arm_monolithic_461;
  }

  if (instruction & (1u << 11))
  {
    goto autodecoder_arm_monolithic_461;
  }

  if ((instruction & (1u << 9)) == 0)
  {
    goto autodecoder_arm_monolithic_472;
  }

autodecoder_arm_monolithic_461:
  if ((instruction & (1u << 23)) == 0)
  {
    handler->barePtr = (void *)(armALUImmRegRSRNoDest);
    return IRC_PATCH_PC;
  }

autodecoder_arm_monolithic_459:
  if ((instruction & (1u << 21)) == 0)
  {
    handler->barePtr = (void *)(armALUImmRegRSR);
    return IRC_PATCH_PC;
  }

  if (instruction & (1u << 22))
  {
    handler->barePtr = (void *)(armShiftPCImm);
    return IRC_PATCH_PC;
  }

  if (instruction & (1u << 16))
  {
    goto autodecoder_arm_undefined;
  }

  if (instruction & (1u << 19))
  {
    goto autodecoder_arm_undefined;
  }

  if (instruction & (1u << 18))
  {
    goto autodecoder_arm_undefined;
  }

  if (instruction & (1u << 17))
  {
    goto autodecoder_arm_undefined;
  }
  handler->barePtr = (void *)(armShiftPCImm);
  return IRC_PATCH_PC;

autodecoder_arm_monolithic_472:
  if ((instruction & (1u << 23)) == 0)
  {
    goto autodecoder_arm_monolithic_471;
  }

  if ((instruction & (1u << 21)) == 0)
  {
    handler->barePtr = (void *)(armALUImmRegRSR);
    return IRC_PATCH_PC;
  }

  if (instruction & (1u << 22))
  {
    handler->barePtr = (void *)(armShiftPCImm);
    return IRC_PATCH_PC;
  }

autodecoder_arm_monolithic_468:
  if (instruction & (1u << 16))
  {
    goto autodecoder_arm_undefined;
  }

  if (instruction & (1u << 19))
  {
    goto autodecoder_arm_undefined;
  }

  if (instruction & (1u << 18))
  {
    goto autodecoder_arm_undefined;
  }

  if (instruction & (1u << 17))
  {
    goto autodecoder_arm_undefined;
  }

  if (instruction & (1u << 5))
  {
    handler->barePtr = (void *)(armShiftPCImm);
    return IRC_PATCH_PC;
  }

  if ((instruction & (1u << 6)) == 0)
  {
    handler->barePtr = (void *)(armMovPCInstruction);
    return IRC_PATCH_PC;
  }
  handler->barePtr = (void *)(armShiftPCImm);
  return IRC_PATCH_PC;



autodecoder_arm_monolithic_471:
  if (instruction & (1u << 21))
  {
    handler->barePtr = (void *)(armALUImmRegRSRNoDest);
    return IRC_PATCH_PC;
  }

autodecoder_arm_monolithic_386:
  if (instruction & (1u << 20))
  {
    handler->barePtr = (void *)(armALUImmRegRSRNoDest);
    return IRC_PATCH_PC;
  }

  if ((instruction & (1u << 16)) == 0)
  {
    handler->barePtr = (void *)(armALUImmRegRSRNoDest);
    return IRC_PATCH_PC;
  }

  if ((instruction & (1u << 19)) == 0)
  {
    handler->barePtr = (void *)(armALUImmRegRSRNoDest);
    return IRC_PATCH_PC;
  }

  if ((instruction & (1u << 18)) == 0)
  {
    handler->barePtr = (void *)(armALUImmRegRSRNoDest);
    return IRC_PATCH_PC;
  }

  if ((instruction & (1u << 17)) == 0)
  {
    handler->barePtr = (void *)(armALUImmRegRSRNoDest);
    return IRC_PATCH_PC;
  }

  if (instruction & (1u << 5))
  {
    handler->barePtr = (void *)(armALUImmRegRSRNoDest);
    return IRC_PATCH_PC;
  }

  if (instruction & (1u << 6))
  {
    handler->barePtr = (void *)(armALUImmRegRSRNoDest);
    return IRC_PATCH_PC;
  }

  if (instruction & (1u << 3))
  {
    handler->barePtr = (void *)(armALUImmRegRSRNoDest);
    return IRC_PATCH_PC;
  }

  if (instruction & (1u << 2))
  {
    handler->barePtr = (void *)(armALUImmRegRSRNoDest);
    return IRC_PATCH_PC;
  }

  if (instruction & (1u << 1))
  {
    handler->barePtr = (void *)(armALUImmRegRSRNoDest);
    return IRC_PATCH_PC;
  }

  if (instruction & (1u << 0))
  {
    handler->barePtr = (void *)(armALUImmRegRSRNoDest);
    return IRC_PATCH_PC;
  }
  handler->barePtr = (void *)(armMrsInstruction);
  return IRC_REPLACE;



autodecoder_arm_monolithic_480:
  if ((instruction & (1u << 4)) == 0)
  {
    handler->barePtr = (void *)(armALUImmRegRSR);
    return IRC_PATCH_PC;
  }

  if ((instruction & (1u << 7)) == 0)
  {
    return IRC_SAFE;
  }

autodecoder_arm_monolithic_426:
  if (instruction & (1u << 8))
  {
    goto autodecoder_arm_monolithic_415;
  }

  if (instruction & (1u << 10))
  {
    goto autodecoder_arm_monolithic_415;
  }

  if (instruction & (1u << 11))
  {
    goto autodecoder_arm_monolithic_415;
  }

  if ((instruction & (1u << 9)) == 0)
  {
    goto autodecoder_arm_monolithic_422;
  }

autodecoder_arm_monolithic_415:
  if ((instruction & (1u << 23)) == 0)
  {
    goto autodecoder_arm_monolithic_414;
  }

  if ((instruction & (1u << 21)) == 0)
  {
    goto autodecoder_arm_monolithic_408;
  }

  if (instruction & (1u << 22))
  {
    goto autodecoder_arm_monolithic_401;
  }
  goto autodecoder_arm_monolithic_404;

autodecoder_arm_monolithic_408:
  if (instruction & (1u << 22))
  {
    goto autodecoder_arm_monolithic_407;
  }
  goto autodecoder_arm_monolithic_111;

autodecoder_arm_monolithic_414:
  if ((instruction & (1u << 21)) == 0)
  {
    goto autodecoder_arm_monolithic_413;
  }

  if (instruction & (1u << 22))
  {
    goto autodecoder_arm_monolithic_411;
  }

autodecoder_arm_monolithic_404:
  if ((instruction & (1u << 20)) == 0)
  {
    goto autodecoder_arm_monolithic_111;
  }

  if ((instruction & (1u << 5)) == 0)
  {
    goto autodecoder_arm_monolithic_110;
  }

  if (instruction & (1u << 6))
  {
    goto autodecoder_arm_undefined;
  }
  handler->barePtr = (void *)(armLdrhtInstruction);
  return IRC_REPLACE;

autodecoder_arm_monolithic_413:
  if (instruction & (1u << 22))
  {
    goto autodecoder_arm_monolithic_276;
  }

autodecoder_arm_monolithic_111:
  if (instruction & (1u << 5))
  {
    goto autodecoder_arm_undefined;
  }

autodecoder_arm_monolithic_110:
  if (instruction & (1u << 6))
  {
    goto autodecoder_arm_undefined;
  }
  return IRC_SAFE;

autodecoder_arm_monolithic_422:
  if ((instruction & (1u << 23)) == 0)
  {
    goto autodecoder_arm_monolithic_421;
  }

  if ((instruction & (1u << 21)) == 0)
  {
    goto autodecoder_arm_monolithic_407;
  }

  if ((instruction & (1u << 22)) == 0)
  {
    goto autodecoder_arm_monolithic_416;
  }

autodecoder_arm_monolithic_401:
  if (instruction & (1u << 20))
  {
    goto autodecoder_arm_monolithic_397;
  }
  goto autodecoder_arm_monolithic_400;

autodecoder_arm_monolithic_421:
  if ((instruction & (1u << 21)) == 0)
  {
    goto autodecoder_arm_monolithic_420;
  }

  if ((instruction & (1u << 22)) == 0)
  {
    goto autodecoder_arm_monolithic_416;
  }

autodecoder_arm_monolithic_411:
  if ((instruction & (1u << 20)) == 0)
  {
    goto autodecoder_arm_monolithic_400;
  }

  if (instruction & (1u << 5))
  {
    goto autodecoder_arm_monolithic_396;
  }
  handler->barePtr = (void *)(armLdrdhPCInstruction);
  return IRC_PATCH_PC;

autodecoder_arm_monolithic_400:
  if ((instruction & (1u << 5)) == 0)
  {
    goto autodecoder_arm_monolithic_267;
  }

  if (instruction & (1u << 6))
  {
    handler->barePtr = (void *)(armStrPCInstruction);
    return IRC_PATCH_PC;
  }
  handler->barePtr = (void *)(armStrhtInstruction);
  return IRC_REPLACE;


autodecoder_arm_monolithic_416:
  if ((instruction & (1u << 20)) == 0)
  {
    goto autodecoder_arm_monolithic_268;
  }

autodecoder_arm_monolithic_397:
  if ((instruction & (1u << 5)) == 0)
  {
    goto autodecoder_arm_monolithic_267;
  }

autodecoder_arm_monolithic_396:
  if (instruction & (1u << 6))
  {
    handler->barePtr = (void *)(armLdrdhPCInstruction);
    return IRC_PATCH_PC;
  }
  handler->barePtr = (void *)(armLdrhtInstruction);
  return IRC_REPLACE;


autodecoder_arm_monolithic_420:
  if ((instruction & (1u << 22)) == 0)
  {
    goto autodecoder_arm_monolithic_407;
  }

autodecoder_arm_monolithic_276:
  if (instruction & (1u << 20))
  {
    handler->barePtr = (void *)(armLdrdhPCInstruction);
    return IRC_PATCH_PC;
  }

  if (instruction & (1u << 5))
  {
    handler->barePtr = (void *)(armStrPCInstruction);
    return IRC_PATCH_PC;
  }

  if (instruction & (1u << 6))
  {
    handler->barePtr = (void *)(armLdrdhPCInstruction);
    return IRC_PATCH_PC;
  }
  goto autodecoder_arm_undefined;


autodecoder_arm_monolithic_407:
  if ((instruction & (1u << 20)) == 0)
  {
    goto autodecoder_arm_monolithic_268;
  }

  if (instruction & (1u << 5))
  {
    handler->barePtr = (void *)(armLdrdhPCInstruction);
    return IRC_PATCH_PC;
  }
  goto autodecoder_arm_monolithic_267;

autodecoder_arm_monolithic_268:
  if ((instruction & (1u << 5)) == 0)
  {
    goto autodecoder_arm_monolithic_267;
  }
  handler->barePtr = (void *)(armStrPCInstruction);
  return IRC_PATCH_PC;


autodecoder_arm_monolithic_267:
  if ((instruction & (1u << 6)) == 0)
  {
    return IRC_SAFE;
  }
  handler->barePtr = (void *)(armLdrdhPCInstruction);
  return IRC_PATCH_PC;




autodecoder_arm_undefined:
  printf("instruction = %#.8x" EOL, instruction);
  DIE_NOW(NULL, "undefined instruction");

