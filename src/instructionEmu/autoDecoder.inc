autodecoder_node_0000000000c62b60:
	if ((instr_word & (1u << 29)) == 0)
	{
		goto autodecoder_node_0000000000c70100;
	}

autodecoder_node_0000000000c61360:
	if ((instr_word & (1u << 30)) == 0)
	{
		goto autodecoder_node_0000000000c70100;
	}

autodecoder_node_0000000000ce6580:
	if (instr_word & (1u << 31))
	{
		goto autodecoder_node_0000000000c846c0;
	}

autodecoder_node_0000000000c70100:
	if (instr_word & (1u << 27))
	{
		goto autodecoder_node_0000000000c7c840;
	}

autodecoder_node_0000000000c7b580:
	if (instr_word & (1u << 15))
	{
		goto autodecoder_node_0000000000b30780;
	}

autodecoder_node_0000000000cd32e0:
	if (instr_word & (1u << 12))
	{
		goto autodecoder_node_0000000000ce9480;
	}

autodecoder_node_0000000000cdbf80:
	if (instr_word & (1u << 13))
	{
		goto autodecoder_node_0000000000ce9480;
	}

autodecoder_node_0000000000c6d6e0:
	if (instr_word & (1u << 14))
	{
		goto autodecoder_node_0000000000ce9480;
	}

autodecoder_node_0000000000c6e9e0:
	if (instr_word & (1u << 26))
	{
		goto autodecoder_node_0000000000c69540;
	}

autodecoder_node_0000000000c60660:
	if (instr_word & (1u << 25))
	{
		goto autodecoder_node_0000000000c68ca0;
	}

autodecoder_node_0000000000c74980:
	if ((instr_word & (1u << 24)) == 0)
	{
		goto autodecoder_node_0000000000cc6900;
	}

autodecoder_node_0000000000ce5880:
	if (instr_word & (1u << 4))
	{
		goto autodecoder_node_0000000000c74c40;
	}
	goto autodecoder_node_0000000000c850a0;

autodecoder_node_0000000000b30780:
	if ((instr_word & (1u << 12)) == 0)
	{
		goto autodecoder_node_0000000000ce9480;
	}

autodecoder_node_0000000000ce63e0:
	if ((instr_word & (1u << 13)) == 0)
	{
		goto autodecoder_node_0000000000ce9480;
	}

autodecoder_node_0000000000c6fd00:
	if (instr_word & (1u << 14))
	{
		goto autodecoder_node_0000000000c6e4a0;
	}

autodecoder_node_0000000000ce9480:
	if (instr_word & (1u << 26))
	{
		goto autodecoder_node_0000000000c69540;
	}

autodecoder_node_0000000000c6a0e0:
	if (instr_word & (1u << 25))
	{
		goto autodecoder_node_0000000000c68ca0;
	}

autodecoder_node_0000000000c5e540:
	if ((instr_word & (1u << 24)) == 0)
	{
		goto autodecoder_node_0000000000cc6900;
	}

autodecoder_node_0000000000ce4420:
	if ((instr_word & (1u << 4)) == 0)
	{
		goto autodecoder_node_0000000000c60fe0;
	}

autodecoder_node_0000000000c74c40:
	if (instr_word & (1u << 7))
	{
		goto autodecoder_node_0000000000c87520;
	}

autodecoder_node_0000000000c723c0:
	if (instr_word & (1u << 23))
	{
		goto autodecoder_node_0000000000c71600;
	}

autodecoder_node_0000000000cbe900:
	if (instr_word & (1u << 8))
	{
		goto autodecoder_node_0000000000c6bd60;
	}
	goto autodecoder_node_0000000000ce60a0;

autodecoder_node_0000000000c6e4a0:
	if (instr_word & (1u << 26))
	{
		goto autodecoder_node_0000000000b595e0;
	}

autodecoder_node_0000000000cca2a0:
	if (instr_word & (1u << 25))
	{
		goto autodecoder_node_0000000000c794a0;
	}

autodecoder_node_0000000000c611e0:
	if ((instr_word & (1u << 24)) == 0)
	{
		goto autodecoder_node_0000000000ce6dc0;
	}

autodecoder_node_0000000000ce8ee0:
	if ((instr_word & (1u << 4)) == 0)
	{
		goto autodecoder_node_0000000000b73f00;
	}

autodecoder_node_0000000000c67f60:
	if (instr_word & (1u << 7))
	{
		goto autodecoder_node_0000000000c87520;
	}

autodecoder_node_0000000000ce27c0:
	if (instr_word & (1u << 23))
	{
		goto autodecoder_node_0000000000c7b6a0;
	}

autodecoder_node_0000000000c65620:
	if (instr_word & (1u << 8))
	{
		goto autodecoder_node_0000000000c7b300;
	}

autodecoder_node_0000000000ce60a0:
	if (instr_word & (1u << 11))
	{
		goto autodecoder_node_0000000000c6bd60;
	}

autodecoder_node_0000000000bc2320:
	if (instr_word & (1u << 10))
	{
		goto autodecoder_node_0000000000c6bd60;
	}

autodecoder_node_0000000000c68940:
	if (instr_word & (1u << 9))
	{
		goto autodecoder_node_0000000000c6bd60;
	}

autodecoder_node_0000000000cd53e0:
	if ((instr_word & (1u << 21)) == 0)
	{
		goto autodecoder_node_0000000000c70540;
	}

autodecoder_node_0000000000c6db60:
	if (instr_word & (1u << 20))
	{
		goto autodecoder_node_0000000000b0a240;
	}

autodecoder_node_0000000000c7bb00:
	if (instr_word & (1u << 5))
	{
		goto autodecoder_node_0000000000c66b80;
	}
	goto autodecoder_node_0000000000c70380;

autodecoder_node_0000000000c7b300:
	if ((instr_word & (1u << 11)) == 0)
	{
		goto autodecoder_node_0000000000c6bd60;
	}

autodecoder_node_0000000000cda360:
	if ((instr_word & (1u << 10)) == 0)
	{
		goto autodecoder_node_0000000000c6bd60;
	}

autodecoder_node_0000000000ce3f20:
	if (instr_word & (1u << 9))
	{
		goto autodecoder_node_0000000000c618e0;
	}

autodecoder_node_0000000000c6bd60:
	if ((instr_word & (1u << 21)) == 0)
	{
		goto autodecoder_node_0000000000b0a240;
	}

autodecoder_node_0000000000c7dc40:
	if (instr_word & (1u << 20))
	{
		goto autodecoder_node_0000000000b0a240;
	}
	goto autodecoder_node_0000000000c79c40;

autodecoder_node_0000000000c618e0:
	if ((instr_word & (1u << 21)) == 0)
	{
		goto autodecoder_node_0000000000b0a240;
	}

autodecoder_node_0000000000bb2020:
	if (instr_word & (1u << 20))
	{
		goto autodecoder_node_0000000000b0a240;
	}

autodecoder_node_0000000000ccc020:
	if ((instr_word & (1u << 16)) == 0)
	{
		goto autodecoder_node_0000000000c79c40;
	}

autodecoder_node_0000000000c7b5e0:
	if ((instr_word & (1u << 19)) == 0)
	{
		goto autodecoder_node_0000000000c79c40;
	}

autodecoder_node_0000000000c80020:
	if ((instr_word & (1u << 17)) == 0)
	{
		goto autodecoder_node_0000000000c79c40;
	}

autodecoder_node_0000000000b45c60:
	if (instr_word & (1u << 18))
	{
		goto autodecoder_node_0000000000ce3820;
	}

autodecoder_node_0000000000c79c40:
	if ((instr_word & (1u << 5)) == 0)
	{
		goto autodecoder_node_0000000000b0a240;
	}

autodecoder_node_0000000000c66b80:
	if (instr_word & (1u << 6))
	{
		goto autodecoder_node_0000000000cde960;
	}
	goto autodecoder_node_0000000000b0a240;

autodecoder_node_0000000000ce3820:
	if ((instr_word & (1u << 5)) == 0)
	{
		goto autodecoder_node_0000000000cd0e00;
	}

autodecoder_node_0000000000cc0140:
	if ((instr_word & (1u << 6)) == 0)
	{
		goto autodecoder_node_0000000000ccb340;
	}

autodecoder_node_0000000000cde960:
	if (instr_word & (1u << 22))
	{
		goto autodecoder_node_0000000000b44aa0;
	}
	goto autodecoder_node_0000000000b0a240;

autodecoder_node_0000000000c7c840:
	if (instr_word & (1u << 15))
	{
		goto autodecoder_node_0000000000c779a0;
	}
	goto autodecoder_node_0000000000cd4de0;

autodecoder_node_0000000000c846c0:
	if (instr_word & (1u << 27))
	{
		goto autodecoder_node_0000000000c79360;
	}

autodecoder_node_0000000000cd9b40:
	if (instr_word & (1u << 15))
	{
		goto autodecoder_node_0000000000c6d440;
	}

autodecoder_node_0000000000c6d880:
	if (instr_word & (1u << 12))
	{
		goto autodecoder_node_0000000000cdf740;
	}

autodecoder_node_0000000000ccd200:
	if (instr_word & (1u << 13))
	{
		goto autodecoder_node_0000000000cdf740;
	}

autodecoder_node_0000000000c7c060:
	if (instr_word & (1u << 14))
	{
		goto autodecoder_node_0000000000cdf740;
	}

autodecoder_node_0000000000c5dda0:
	if (instr_word & (1u << 28))
	{
		goto autodecoder_node_0000000000cdb600;
	}

autodecoder_node_0000000000c60700:
	if (instr_word & (1u << 26))
	{
		goto autodecoder_node_0000000000c69540;
	}

autodecoder_node_0000000000c6d2c0:
	if (instr_word & (1u << 25))
	{
		goto autodecoder_node_0000000000c68ca0;
	}

autodecoder_node_0000000000cd1740:
	if ((instr_word & (1u << 24)) == 0)
	{
		goto autodecoder_node_0000000000cc6900;
	}

autodecoder_node_0000000000c75020:
	if (instr_word & (1u << 4))
	{
		goto autodecoder_node_0000000000cbe500;
	}

autodecoder_node_0000000000c850a0:
	if ((instr_word & (1u << 7)) == 0)
	{
		goto autodecoder_node_0000000000c5d860;
	}

autodecoder_node_0000000000c7c260:
	if (instr_word & (1u << 23))
	{
		goto autodecoder_node_0000000000c71600;
	}

autodecoder_node_0000000000c5dde0:
	if ((instr_word & (1u << 21)) == 0)
	{
		goto autodecoder_node_0000000000c7ae80;
	}

autodecoder_node_0000000000c7be00:
	if (instr_word & (1u << 20))
	{
		goto autodecoder_node_0000000000b0a240;
	}

autodecoder_node_0000000000cd4be0:
	if (instr_word & (1u << 5))
	{
		goto autodecoder_node_0000000000c86ee0;
	}

autodecoder_node_0000000000ccbb80:
	if ((instr_word & (1u << 6)) == 0)
	{
		goto autodecoder_node_0000000000ce7f40;
	}

autodecoder_node_0000000000c7a1e0:
	if ((instr_word & (1u << 22)) == 0)
	{
		goto autodecoder_node_0000000000b565c0;
	}

autodecoder_node_0000000000b5d420:
	return &smulbtInstruction;

autodecoder_node_0000000000c86ee0:
	if (instr_word & (1u << 6))
	{
		goto autodecoder_node_0000000000c6d4e0;
	}

autodecoder_node_0000000000c68380:
	if (instr_word & (1u << 22))
	{
		goto autodecoder_node_0000000000b5c4a0;
	}

autodecoder_node_0000000000b5f500:
	return &smulwbInstruction;

autodecoder_node_0000000000b5c4a0:
	return &smultbInstruction;

autodecoder_node_0000000000c6d4e0:
	if (instr_word & (1u << 22))
	{
		goto autodecoder_node_0000000000b5e4e0;
	}

autodecoder_node_0000000000b60360:
	return &smulwtInstruction;

autodecoder_node_0000000000b5e4e0:
	return &smulttInstruction;

autodecoder_node_0000000000cdb600:
	if (instr_word & (1u << 26))
	{
		goto autodecoder_node_0000000000b0a6c0;
	}

autodecoder_node_0000000000c61720:
	if (instr_word & (1u << 25))
	{
		goto autodecoder_node_0000000000b0a6c0;
	}

autodecoder_node_0000000000cdf400:
	if ((instr_word & (1u << 24)) == 0)
	{
		goto autodecoder_node_0000000000b0a6c0;
	}

autodecoder_node_0000000000cbdea0:
	if (instr_word & (1u << 4))
	{
		goto autodecoder_node_0000000000cd4ec0;
	}

autodecoder_node_0000000000c61a60:
	if (instr_word & (1u << 23))
	{
		goto autodecoder_node_0000000000b0a6c0;
	}

autodecoder_node_0000000000ce5b20:
	if (instr_word & (1u << 11))
	{
		goto autodecoder_node_0000000000b0a6c0;
	}

autodecoder_node_0000000000cd2c80:
	if (instr_word & (1u << 10))
	{
		goto autodecoder_node_0000000000b0a6c0;
	}

autodecoder_node_0000000000ce5a60:
	if (instr_word & (1u << 9))
	{
		goto autodecoder_node_0000000000ce89a0;
	}

autodecoder_node_0000000000c86b00:
	if (instr_word & (1u << 21))
	{
		goto autodecoder_node_0000000000b0a6c0;
	}

autodecoder_node_0000000000c5e720:
	if (instr_word & (1u << 20))
	{
		goto autodecoder_node_0000000000b0a6c0;
	}

autodecoder_node_0000000000c6a2c0:
	if (instr_word & (1u << 16))
	{
		goto autodecoder_node_0000000000c70360;
	}

autodecoder_node_0000000000c5f9a0:
	if ((instr_word & (1u << 19)) == 0)
	{
		goto autodecoder_node_0000000000cd3900;
	}

autodecoder_node_0000000000c6bac0:
	if (instr_word & (1u << 17))
	{
		goto autodecoder_node_0000000000c69e80;
	}

autodecoder_node_0000000000c7ac60:
	if (instr_word & (1u << 18))
	{
		goto autodecoder_node_0000000000cd2100;
	}

autodecoder_node_0000000000cd4b20:
	if (instr_word & (1u << 5))
	{
		goto autodecoder_node_0000000000b0a6c0;
	}

autodecoder_node_0000000000c5f0e0:
	if (instr_word & (1u << 3))
	{
		goto autodecoder_node_0000000000cd1d40;
	}

autodecoder_node_0000000000ca3b80:
	if (instr_word & (1u << 2))
	{
		goto autodecoder_node_0000000000cd1d40;
	}

autodecoder_node_0000000000c7a880:
	if (instr_word & (1u << 1))
	{
		goto autodecoder_node_0000000000cd1d40;
	}

autodecoder_node_0000000000c848c0:
	if (instr_word & (1u << 0))
	{
		goto autodecoder_node_0000000000cd1d40;
	}
	goto autodecoder_node_0000000000b5c820;

autodecoder_node_0000000000cd2100:
	if (instr_word & (1u << 5))
	{
		goto autodecoder_node_0000000000b0a6c0;
	}

autodecoder_node_0000000000cc7460:
	if (instr_word & (1u << 3))
	{
		goto autodecoder_node_0000000000cd1d40;
	}

autodecoder_node_0000000000ce7e80:
	if (instr_word & (1u << 2))
	{
		goto autodecoder_node_0000000000cd1d40;
	}

autodecoder_node_0000000000cdd600:
	if (instr_word & (1u << 1))
	{
		goto autodecoder_node_0000000000cd1d40;
	}

autodecoder_node_0000000000cc6200:
	if (instr_word & (1u << 0))
	{
		goto autodecoder_node_0000000000cd1d40;
	}
	goto autodecoder_node_0000000000c7d6c0;

autodecoder_node_0000000000cd4ec0:
	if (instr_word & (1u << 23))
	{
		goto autodecoder_node_0000000000b0a6c0;
	}

autodecoder_node_0000000000cc9880:
	if (instr_word & (1u << 11))
	{
		goto autodecoder_node_0000000000b0a6c0;
	}

autodecoder_node_0000000000cde620:
	if (instr_word & (1u << 10))
	{
		goto autodecoder_node_0000000000b0a6c0;
	}

autodecoder_node_0000000000ccf660:
	if (instr_word & (1u << 9))
	{
		goto autodecoder_node_0000000000ce89a0;
	}

autodecoder_node_0000000000cc92a0:
	if (instr_word & (1u << 21))
	{
		goto autodecoder_node_0000000000b0a6c0;
	}

autodecoder_node_0000000000c68e00:
	if (instr_word & (1u << 20))
	{
		goto autodecoder_node_0000000000b0a6c0;
	}

autodecoder_node_0000000000b492e0:
	if (instr_word & (1u << 16))
	{
		goto autodecoder_node_0000000000c70360;
	}

autodecoder_node_0000000000ce1fa0:
	if ((instr_word & (1u << 19)) == 0)
	{
		goto autodecoder_node_0000000000cd3900;
	}

autodecoder_node_0000000000cc9500:
	if (instr_word & (1u << 17))
	{
		goto autodecoder_node_0000000000c69e80;
	}

autodecoder_node_0000000000cd3900:
	if (instr_word & (1u << 5))
	{
		goto autodecoder_node_0000000000b0a6c0;
	}

autodecoder_node_0000000000cd1d40:
	if (instr_word & (1u << 22))
	{
		goto autodecoder_node_0000000000b0a6c0;
	}

autodecoder_node_0000000000b85660:
	return &cpsInstruction;

autodecoder_node_0000000000c69e80:
	if (instr_word & (1u << 18))
	{
		goto autodecoder_node_0000000000ce2aa0;
	}

autodecoder_node_0000000000c71660:
	if (instr_word & (1u << 5))
	{
		goto autodecoder_node_0000000000b0a6c0;
	}

autodecoder_node_0000000000b5c820:
	if (instr_word & (1u << 22))
	{
		goto autodecoder_node_0000000000b0a6c0;
	}

autodecoder_node_0000000000b726c0:
	return &cpsieInstruction;

autodecoder_node_0000000000ce2aa0:
	if (instr_word & (1u << 5))
	{
		goto autodecoder_node_0000000000b0a6c0;
	}

autodecoder_node_0000000000c7d6c0:
	if (instr_word & (1u << 22))
	{
		goto autodecoder_node_0000000000b0a6c0;
	}

autodecoder_node_0000000000b766e0:
	return &cpsidInstruction;

autodecoder_node_0000000000ce89a0:
	if (instr_word & (1u << 21))
	{
		goto autodecoder_node_0000000000b0a6c0;
	}

autodecoder_node_0000000000ccb220:
	if (instr_word & (1u << 20))
	{
		goto autodecoder_node_0000000000b0a6c0;
	}

autodecoder_node_0000000000cd25a0:
	if ((instr_word & (1u << 16)) == 0)
	{
		goto autodecoder_node_0000000000b0a6c0;
	}

autodecoder_node_0000000000c70360:
	if (instr_word & (1u << 19))
	{
		goto autodecoder_node_0000000000b0a6c0;
	}

autodecoder_node_0000000000c70a80:
	if (instr_word & (1u << 17))
	{
		goto autodecoder_node_0000000000b0a6c0;
	}

autodecoder_node_0000000000c75580:
	if (instr_word & (1u << 18))
	{
		goto autodecoder_node_0000000000b0a6c0;
	}

autodecoder_node_0000000000cd08c0:
	if (instr_word & (1u << 22))
	{
		goto autodecoder_node_0000000000b0a6c0;
	}

autodecoder_node_0000000000b89e40:
	return &setendInstruction;

autodecoder_node_0000000000c6d440:
	if ((instr_word & (1u << 12)) == 0)
	{
		goto autodecoder_node_0000000000cdf740;
	}

autodecoder_node_0000000000c671c0:
	if ((instr_word & (1u << 13)) == 0)
	{
		goto autodecoder_node_0000000000cdf740;
	}

autodecoder_node_0000000000ba8160:
	if (instr_word & (1u << 14))
	{
		goto autodecoder_node_0000000000be2b00;
	}

autodecoder_node_0000000000cdf740:
	if (instr_word & (1u << 28))
	{
		goto autodecoder_node_0000000000b0a6c0;
	}

autodecoder_node_0000000000c5dc60:
	if (instr_word & (1u << 26))
	{
		goto autodecoder_node_0000000000c69540;
	}

autodecoder_node_0000000000c5e360:
	if (instr_word & (1u << 25))
	{
		goto autodecoder_node_0000000000c68ca0;
	}

autodecoder_node_0000000000c8e3e0:
	if (instr_word & (1u << 24))
	{
		goto autodecoder_node_0000000000cdec20;
	}

autodecoder_node_0000000000cc6900:
	if ((instr_word & (1u << 4)) == 0)
	{
		goto autodecoder_node_0000000000b0a240;
	}

autodecoder_node_0000000000cc75c0:
	if (instr_word & (1u << 7))
	{
		goto autodecoder_node_0000000000cd17c0;
	}
	goto autodecoder_node_0000000000b0a240;

autodecoder_node_0000000000cdec20:
	if (instr_word & (1u << 4))
	{
		goto autodecoder_node_0000000000cbe500;
	}

autodecoder_node_0000000000c60fe0:
	if (instr_word & (1u << 7))
	{
		goto autodecoder_node_0000000000c66600;
	}

autodecoder_node_0000000000c5d860:
	if (instr_word & (1u << 23))
	{
		goto autodecoder_node_0000000000c71600;
	}

autodecoder_node_0000000000cceb80:
	if (instr_word & (1u << 8))
	{
		goto autodecoder_node_0000000000b0a240;
	}

autodecoder_node_0000000000c77c00:
	if (instr_word & (1u << 11))
	{
		goto autodecoder_node_0000000000b0a240;
	}

autodecoder_node_0000000000c66f40:
	if (instr_word & (1u << 10))
	{
		goto autodecoder_node_0000000000b0a240;
	}

autodecoder_node_0000000000cc70c0:
	if (instr_word & (1u << 9))
	{
		goto autodecoder_node_0000000000b0a240;
	}

autodecoder_node_0000000000c67d80:
	if (instr_word & (1u << 21))
	{
		goto autodecoder_node_0000000000b0a240;
	}
	goto autodecoder_node_0000000000c65a60;

autodecoder_node_0000000000c66600:
	if (instr_word & (1u << 23))
	{
		goto autodecoder_node_0000000000c71600;
	}
	goto autodecoder_node_0000000000cc7820;

autodecoder_node_0000000000cbe500:
	if (instr_word & (1u << 7))
	{
		goto autodecoder_node_0000000000c87520;
	}

autodecoder_node_0000000000c7c500:
	if (instr_word & (1u << 23))
	{
		goto autodecoder_node_0000000000c71600;
	}

autodecoder_node_0000000000cc9420:
	if (instr_word & (1u << 8))
	{
		goto autodecoder_node_0000000000cc8c00;
	}
	goto autodecoder_node_0000000000cd33e0;

autodecoder_node_0000000000c68ca0:
	if ((instr_word & (1u << 24)) == 0)
	{
		goto autodecoder_node_0000000000b0a240;
	}

autodecoder_node_0000000000cdd7e0:
	if (instr_word & (1u << 4))
	{
		goto autodecoder_node_0000000000c6a720;
	}

autodecoder_node_0000000000cdee20:
	if (instr_word & (1u << 7))
	{
		goto autodecoder_node_0000000000c6a720;
	}

autodecoder_node_0000000000c61b20:
	if (instr_word & (1u << 23))
	{
		goto autodecoder_node_0000000000c71600;
	}

autodecoder_node_0000000000ccdb20:
	if ((instr_word & (1u << 21)) == 0)
	{
		goto autodecoder_node_0000000000c70c20;
	}

autodecoder_node_0000000000ccd980:
	if (instr_word & (1u << 20))
	{
		goto autodecoder_node_0000000000b0a240;
	}

autodecoder_node_0000000000c8ad60:
	if (instr_word & (1u << 16))
	{
		goto autodecoder_node_0000000000b0a240;
	}

autodecoder_node_0000000000cdab40:
	if (instr_word & (1u << 19))
	{
		goto autodecoder_node_0000000000b0a240;
	}

autodecoder_node_0000000000c6d9a0:
	if (instr_word & (1u << 17))
	{
		goto autodecoder_node_0000000000b0a240;
	}

autodecoder_node_0000000000ce23c0:
	if (instr_word & (1u << 18))
	{
		goto autodecoder_node_0000000000b0a240;
	}

autodecoder_node_0000000000b3bb40:
	if (instr_word & (1u << 5))
	{
		goto autodecoder_node_0000000000b0a240;
	}

autodecoder_node_0000000000cc9400:
	if (instr_word & (1u << 6))
	{
		goto autodecoder_node_0000000000b0a240;
	}

autodecoder_node_0000000000c71aa0:
	if (instr_word & (1u << 3))
	{
		goto autodecoder_node_0000000000b0a240;
	}

autodecoder_node_0000000000ce5220:
	if (instr_word & (1u << 2))
	{
		goto autodecoder_node_0000000000b0a240;
	}

autodecoder_node_0000000000c60d80:
	if (instr_word & (1u << 1))
	{
		goto autodecoder_node_0000000000b0a240;
	}

autodecoder_node_0000000000cc5d60:
	if (instr_word & (1u << 0))
	{
		goto autodecoder_node_0000000000b0a240;
	}

autodecoder_node_0000000000c85e40:
	if (instr_word & (1u << 22))
	{
		goto autodecoder_node_0000000000b0a240;
	}
	goto autodecoder_node_0000000000bd6ec0;

autodecoder_node_0000000000c6a720:
	if (instr_word & (1u << 23))
	{
		goto autodecoder_node_0000000000c71600;
	}

autodecoder_node_0000000000ce87e0:
	if (instr_word & (1u << 21))
	{
		goto autodecoder_node_0000000000b0a240;
	}
	goto autodecoder_node_0000000000c70c20;

autodecoder_node_0000000000c71600:
	if ((instr_word & (1u << 21)) == 0)
	{
		goto autodecoder_node_0000000000b0a240;
	}

autodecoder_node_0000000000c72640:
	if (instr_word & (1u << 16))
	{
		goto autodecoder_node_0000000000cbe700;
	}

autodecoder_node_0000000000cc76c0:
	if (instr_word & (1u << 19))
	{
		goto autodecoder_node_0000000000cbe700;
	}

autodecoder_node_0000000000c9f420:
	if (instr_word & (1u << 17))
	{
		goto autodecoder_node_0000000000cbe700;
	}

autodecoder_node_0000000000ce1960:
	if (instr_word & (1u << 18))
	{
		goto autodecoder_node_0000000000cbe700;
	}
	goto autodecoder_node_0000000000b0a240;

autodecoder_node_0000000000c69540:
	if ((instr_word & (1u << 25)) == 0)
	{
		goto autodecoder_node_0000000000b0a240;
	}

autodecoder_node_0000000000ccc9e0:
	if (instr_word & (1u << 24))
	{
		goto autodecoder_node_0000000000ccaac0;
	}

autodecoder_node_0000000000c71940:
	if (instr_word & (1u << 4))
	{
		goto autodecoder_node_0000000000cd2720;
	}
	goto autodecoder_node_0000000000b0a240;

autodecoder_node_0000000000ccaac0:
	if ((instr_word & (1u << 4)) == 0)
	{
		goto autodecoder_node_0000000000b0a240;
	}

autodecoder_node_0000000000ce40a0:
	if (instr_word & (1u << 7))
	{
		goto autodecoder_node_0000000000cd1520;
	}

autodecoder_node_0000000000ccf2c0:
	if (instr_word & (1u << 23))
	{
		goto autodecoder_node_0000000000c96a40;
	}

autodecoder_node_0000000000cd9c80:
	if (instr_word & (1u << 21))
	{
		goto autodecoder_node_0000000000b0a6c0;
	}

autodecoder_node_0000000000ce46a0:
	if (instr_word & (1u << 20))
	{
		goto autodecoder_node_0000000000cc8dc0;
	}

autodecoder_node_0000000000c7b6e0:
	if (instr_word & (1u << 6))
	{
		goto autodecoder_node_0000000000cbdd80;
	}

autodecoder_node_0000000000c6c2c0:
	if (instr_word & (1u << 22))
	{
		goto autodecoder_node_0000000000b3fd20;
	}

autodecoder_node_0000000000b3dd40:
	return &smladInstruction;

autodecoder_node_0000000000cbdd80:
	if (instr_word & (1u << 22))
	{
		goto autodecoder_node_0000000000b34a20;
	}

autodecoder_node_0000000000b422c0:
	return &smlsdInstruction;

autodecoder_node_0000000000cc8dc0:
	if (instr_word & (1u << 6))
	{
		goto autodecoder_node_0000000000b0a6c0;
	}

autodecoder_node_0000000000ce3be0:
	if ((instr_word & (1u << 22)) == 0)
	{
		goto autodecoder_node_0000000000b0a6c0;
	}

autodecoder_node_0000000000b39b80:
	return &smmlaInstruction;

autodecoder_node_0000000000be2b00:
	if (instr_word & (1u << 28))
	{
		goto autodecoder_node_0000000000cd1b20;
	}

autodecoder_node_0000000000cc8220:
	if (instr_word & (1u << 26))
	{
		goto autodecoder_node_0000000000b595e0;
	}

autodecoder_node_0000000000c77a80:
	if (instr_word & (1u << 25))
	{
		goto autodecoder_node_0000000000c794a0;
	}

autodecoder_node_0000000000c5fd20:
	if (instr_word & (1u << 24))
	{
		goto autodecoder_node_0000000000c668e0;
	}

autodecoder_node_0000000000ce6dc0:
	if ((instr_word & (1u << 4)) == 0)
	{
		goto autodecoder_node_0000000000b52740;
	}

autodecoder_node_0000000000c6b4c0:
	if ((instr_word & (1u << 7)) == 0)
	{
		goto autodecoder_node_0000000000b52740;
	}

autodecoder_node_0000000000cd17c0:
	if (instr_word & (1u << 23))
	{
		goto autodecoder_node_0000000000cc6980;
	}

autodecoder_node_0000000000c66c00:
	if (instr_word & (1u << 8))
	{
		goto autodecoder_node_0000000000c607c0;
	}

autodecoder_node_0000000000cc7da0:
	if (instr_word & (1u << 11))
	{
		goto autodecoder_node_0000000000c607c0;
	}

autodecoder_node_0000000000cc7920:
	if (instr_word & (1u << 10))
	{
		goto autodecoder_node_0000000000c607c0;
	}

autodecoder_node_0000000000c7d660:
	if (instr_word & (1u << 9))
	{
		goto autodecoder_node_0000000000c607c0;
	}

autodecoder_node_0000000000c6c0e0:
	if (instr_word & (1u << 21))
	{
		goto autodecoder_node_0000000000c6ca20;
	}

autodecoder_node_0000000000c5dfe0:
	if (instr_word & (1u << 20))
	{
		goto autodecoder_node_0000000000b0a240;
	}

autodecoder_node_0000000000ce6ee0:
	if (instr_word & (1u << 5))
	{
		goto autodecoder_node_0000000000b0a240;
	}

autodecoder_node_0000000000c6ab80:
	if (instr_word & (1u << 6))
	{
		goto autodecoder_node_0000000000b0a240;
	}
	goto autodecoder_node_0000000000cd5280;

autodecoder_node_0000000000c607c0:
	if (instr_word & (1u << 21))
	{
		goto autodecoder_node_0000000000cd9940;
	}

autodecoder_node_0000000000c61fc0:
	if (instr_word & (1u << 20))
	{
		goto autodecoder_node_0000000000c6ac20;
	}

autodecoder_node_0000000000c791c0:
	if (instr_word & (1u << 5))
	{
		goto autodecoder_node_0000000000cbe700;
	}

autodecoder_node_0000000000ccd880:
	if (instr_word & (1u << 6))
	{
		goto autodecoder_node_0000000000cbe700;
	}

autodecoder_node_0000000000cd5280:
	if (instr_word & (1u << 22))
	{
		goto autodecoder_node_0000000000b0a6c0;
	}
	goto autodecoder_node_0000000000b0a240;

autodecoder_node_0000000000cc6980:
	if (instr_word & (1u << 8))
	{
		goto autodecoder_node_0000000000cd0ae0;
	}

autodecoder_node_0000000000c7ee00:
	if (instr_word & (1u << 11))
	{
		goto autodecoder_node_0000000000cd0ae0;
	}

autodecoder_node_0000000000cd1f60:
	if (instr_word & (1u << 10))
	{
		goto autodecoder_node_0000000000cd0ae0;
	}

autodecoder_node_0000000000b29440:
	if (instr_word & (1u << 9))
	{
		goto autodecoder_node_0000000000cd0ae0;
	}

autodecoder_node_0000000000ce0760:
	if ((instr_word & (1u << 21)) == 0)
	{
		goto autodecoder_node_0000000000b0a240;
	}

autodecoder_node_0000000000c6ca20:
	if (instr_word & (1u << 20))
	{
		goto autodecoder_node_0000000000c53560;
	}

autodecoder_node_0000000000cd2040:
	if ((instr_word & (1u << 5)) == 0)
	{
		goto autodecoder_node_0000000000b0a240;
	}

autodecoder_node_0000000000ce3dc0:
	if (instr_word & (1u << 6))
	{
		goto autodecoder_node_0000000000b0a240;
	}

autodecoder_node_0000000000cda420:
	if (instr_word & (1u << 22))
	{
		goto autodecoder_node_0000000000b2a440;
	}
	goto autodecoder_node_0000000000b0a240;

autodecoder_node_0000000000c53560:
	if ((instr_word & (1u << 5)) == 0)
	{
		goto autodecoder_node_0000000000b0a240;
	}

autodecoder_node_0000000000c6f2c0:
	if (instr_word & (1u << 6))
	{
		goto autodecoder_node_0000000000b0a240;
	}
	goto autodecoder_node_0000000000b2b320;

autodecoder_node_0000000000cd0ae0:
	if (instr_word & (1u << 21))
	{
		goto autodecoder_node_0000000000cd9940;
	}

autodecoder_node_0000000000c6ac20:
	if (instr_word & (1u << 5))
	{
		goto autodecoder_node_0000000000cbe700;
	}
	goto autodecoder_node_0000000000cdb700;

autodecoder_node_0000000000cd9940:
	if (instr_word & (1u << 20))
	{
		goto autodecoder_node_0000000000cc9940;
	}

autodecoder_node_0000000000c5eae0:
	if ((instr_word & (1u << 5)) == 0)
	{
		goto autodecoder_node_0000000000cdb700;
	}

autodecoder_node_0000000000b0d6c0:
	if (instr_word & (1u << 6))
	{
		goto autodecoder_node_0000000000cbe700;
	}

autodecoder_node_0000000000c865c0:
	if ((instr_word & (1u << 22)) == 0)
	{
		goto autodecoder_node_0000000000b0a6c0;
	}

autodecoder_node_0000000000b2a440:
	return &strhtInstruction;

autodecoder_node_0000000000cc9940:
	if (instr_word & (1u << 5))
	{
		goto autodecoder_node_0000000000c6b5e0;
	}

autodecoder_node_0000000000cdb700:
	if (instr_word & (1u << 6))
	{
		goto autodecoder_node_0000000000cbe700;
	}
	goto autodecoder_node_0000000000b0a240;

autodecoder_node_0000000000c6b5e0:
	if (instr_word & (1u << 6))
	{
		goto autodecoder_node_0000000000cbe700;
	}

autodecoder_node_0000000000b2b320:
	return &ldrhtInstruction;

autodecoder_node_0000000000c668e0:
	if (instr_word & (1u << 4))
	{
		goto autodecoder_node_0000000000ce8aa0;
	}

autodecoder_node_0000000000b73f00:
	if (instr_word & (1u << 7))
	{
		goto autodecoder_node_0000000000ba28e0;
	}

autodecoder_node_0000000000c681a0:
	if (instr_word & (1u << 23))
	{
		goto autodecoder_node_0000000000cd4040;
	}

autodecoder_node_0000000000c6f500:
	if (instr_word & (1u << 8))
	{
		goto autodecoder_node_0000000000c85a60;
	}

autodecoder_node_0000000000cce8a0:
	if (instr_word & (1u << 11))
	{
		goto autodecoder_node_0000000000b0a240;
	}

autodecoder_node_0000000000cdc8e0:
	if (instr_word & (1u << 10))
	{
		goto autodecoder_node_0000000000b0a240;
	}

autodecoder_node_0000000000cce320:
	if (instr_word & (1u << 9))
	{
		goto autodecoder_node_0000000000b0a240;
	}

autodecoder_node_0000000000ce8b40:
	if (instr_word & (1u << 21))
	{
		goto autodecoder_node_0000000000c7e3c0;
	}

autodecoder_node_0000000000c65a60:
	if (instr_word & (1u << 20))
	{
		goto autodecoder_node_0000000000b0a240;
	}

autodecoder_node_0000000000c5d780:
	if ((instr_word & (1u << 16)) == 0)
	{
		goto autodecoder_node_0000000000b0a240;
	}

autodecoder_node_0000000000c79e40:
	if ((instr_word & (1u << 19)) == 0)
	{
		goto autodecoder_node_0000000000b0a240;
	}

autodecoder_node_0000000000ce48c0:
	if ((instr_word & (1u << 17)) == 0)
	{
		goto autodecoder_node_0000000000b0a240;
	}

autodecoder_node_0000000000c5f4e0:
	if ((instr_word & (1u << 18)) == 0)
	{
		goto autodecoder_node_0000000000b0a240;
	}

autodecoder_node_0000000000c6df60:
	if (instr_word & (1u << 5))
	{
		goto autodecoder_node_0000000000b0a240;
	}

autodecoder_node_0000000000c85120:
	if (instr_word & (1u << 6))
	{
		goto autodecoder_node_0000000000b0a240;
	}

autodecoder_node_0000000000c5e4c0:
	if (instr_word & (1u << 3))
	{
		goto autodecoder_node_0000000000b0a240;
	}

autodecoder_node_0000000000c67ea0:
	if (instr_word & (1u << 2))
	{
		goto autodecoder_node_0000000000b0a240;
	}

autodecoder_node_0000000000bf14e0:
	if (instr_word & (1u << 1))
	{
		goto autodecoder_node_0000000000b0a240;
	}

autodecoder_node_0000000000cbfac0:
	if (instr_word & (1u << 0))
	{
		goto autodecoder_node_0000000000b0a240;
	}

autodecoder_node_0000000000bd2280:
	return &mrsInstruction;

autodecoder_node_0000000000c7e3c0:
	if (instr_word & (1u << 20))
	{
		goto autodecoder_node_0000000000b0a240;
	}

autodecoder_node_0000000000b4a6a0:
	if (instr_word & (1u << 5))
	{
		goto autodecoder_node_0000000000b0a240;
	}

autodecoder_node_0000000000ccebe0:
	if (instr_word & (1u << 6))
	{
		goto autodecoder_node_0000000000b0a240;
	}
	goto autodecoder_node_0000000000bcffe0;

autodecoder_node_0000000000c85a60:
	if ((instr_word & (1u << 11)) == 0)
	{
		goto autodecoder_node_0000000000b0a240;
	}

autodecoder_node_0000000000b8cb40:
	if ((instr_word & (1u << 10)) == 0)
	{
		goto autodecoder_node_0000000000b0a240;
	}

autodecoder_node_0000000000c3bf00:
	if ((instr_word & (1u << 9)) == 0)
	{
		goto autodecoder_node_0000000000b0a240;
	}

autodecoder_node_0000000000cd36a0:
	if ((instr_word & (1u << 21)) == 0)
	{
		goto autodecoder_node_0000000000b0a240;
	}

autodecoder_node_0000000000c60b80:
	if (instr_word & (1u << 20))
	{
		goto autodecoder_node_0000000000b0a240;
	}

autodecoder_node_0000000000c77940:
	if ((instr_word & (1u << 16)) == 0)
	{
		goto autodecoder_node_0000000000b0a240;
	}

autodecoder_node_0000000000cc9220:
	if ((instr_word & (1u << 19)) == 0)
	{
		goto autodecoder_node_0000000000b0a240;
	}

autodecoder_node_0000000000cd5060:
	if ((instr_word & (1u << 17)) == 0)
	{
		goto autodecoder_node_0000000000b0a240;
	}

autodecoder_node_0000000000c690c0:
	if ((instr_word & (1u << 18)) == 0)
	{
		goto autodecoder_node_0000000000b0a240;
	}

autodecoder_node_0000000000cd1720:
	if ((instr_word & (1u << 5)) == 0)
	{
		goto autodecoder_node_0000000000b0a240;
	}

autodecoder_node_0000000000b48f80:
	if (instr_word & (1u << 6))
	{
		goto autodecoder_node_0000000000b0a240;
	}

autodecoder_node_0000000000c7e880:
	if (instr_word & (1u << 22))
	{
		goto autodecoder_node_0000000000b0a240;
	}

autodecoder_node_0000000000b42820:
	return &bxjInstruction;

autodecoder_node_0000000000cd4040:
	if (instr_word & (1u << 8))
	{
		goto autodecoder_node_0000000000cc5de0;
	}

autodecoder_node_0000000000ccf240:
	if (instr_word & (1u << 11))
	{
		goto autodecoder_node_0000000000cc5de0;
	}

autodecoder_node_0000000000ce56a0:
	if (instr_word & (1u << 10))
	{
		goto autodecoder_node_0000000000cc5de0;
	}

autodecoder_node_0000000000c6ec20:
	if (instr_word & (1u << 9))
	{
		goto autodecoder_node_0000000000cc5de0;
	}

autodecoder_node_0000000000c7d4a0:
	if ((instr_word & (1u << 21)) == 0)
	{
		goto autodecoder_node_0000000000c79cc0;
	}

autodecoder_node_0000000000ccc340:
	if (instr_word & (1u << 16))
	{
		goto autodecoder_node_0000000000c7a380;
	}

autodecoder_node_0000000000c665a0:
	if (instr_word & (1u << 19))
	{
		goto autodecoder_node_0000000000c7a380;
	}

autodecoder_node_0000000000c7f320:
	if (instr_word & (1u << 17))
	{
		goto autodecoder_node_0000000000c7a380;
	}

autodecoder_node_0000000000bb3aa0:
	if (instr_word & (1u << 18))
	{
		goto autodecoder_node_0000000000c7a380;
	}

autodecoder_node_0000000000c7ad80:
	if (instr_word & (1u << 5))
	{
		goto autodecoder_node_0000000000bad960;
	}

autodecoder_node_0000000000cdd400:
	if (instr_word & (1u << 6))
	{
		goto autodecoder_node_0000000000ce3c40;
	}
	goto autodecoder_node_0000000000c4dca0;

autodecoder_node_0000000000bad960:
	if ((instr_word & (1u << 6)) == 0)
	{
		goto autodecoder_node_0000000000c656e0;
	}

autodecoder_node_0000000000c70660:
	if (instr_word & (1u << 22))
	{
		goto autodecoder_node_0000000000bf4300;
	}

autodecoder_node_0000000000c02320:
	return &rrxInstruction;

autodecoder_node_0000000000ba28e0:
	if (instr_word & (1u << 23))
	{
		goto autodecoder_node_0000000000cc5de0;
	}

autodecoder_node_0000000000cc7820:
	if (instr_word & (1u << 21))
	{
		goto autodecoder_node_0000000000b2f0c0;
	}

autodecoder_node_0000000000c7ae80:
	if (instr_word & (1u << 20))
	{
		goto autodecoder_node_0000000000b0a240;
	}

autodecoder_node_0000000000c698a0:
	if (instr_word & (1u << 5))
	{
		goto autodecoder_node_0000000000b4f7e0;
	}

autodecoder_node_0000000000c7e440:
	if (instr_word & (1u << 6))
	{
		goto autodecoder_node_0000000000c84a60;
	}

autodecoder_node_0000000000c7b920:
	if (instr_word & (1u << 22))
	{
		goto autodecoder_node_0000000000b57620;
	}

autodecoder_node_0000000000b50f80:
	return &smlabbInstruction;

autodecoder_node_0000000000b57620:
	return &smlalbbInstruction;

autodecoder_node_0000000000c84a60:
	if (instr_word & (1u << 22))
	{
		goto autodecoder_node_0000000000b59240;
	}

autodecoder_node_0000000000b53260:
	return &smlabtInstruction;

autodecoder_node_0000000000b59240:
	return &smlalbtInstruction;

autodecoder_node_0000000000b4f7e0:
	if (instr_word & (1u << 6))
	{
		goto autodecoder_node_0000000000c72ac0;
	}

autodecoder_node_0000000000c60ce0:
	if (instr_word & (1u << 22))
	{
		goto autodecoder_node_0000000000b58620;
	}

autodecoder_node_0000000000b52480:
	return &smlatbInstruction;

autodecoder_node_0000000000b58620:
	return &smlaltbInstruction;

autodecoder_node_0000000000c72ac0:
	if (instr_word & (1u << 22))
	{
		goto autodecoder_node_0000000000b5a1a0;
	}

autodecoder_node_0000000000b54700:
	return &smlattInstruction;

autodecoder_node_0000000000b5a1a0:
	return &smlalttInstruction;

autodecoder_node_0000000000b2f0c0:
	if (instr_word & (1u << 20))
	{
		goto autodecoder_node_0000000000b0a240;
	}

autodecoder_node_0000000000cdce60:
	if (instr_word & (1u << 5))
	{
		goto autodecoder_node_0000000000b0a240;
	}

autodecoder_node_0000000000c66ec0:
	if (instr_word & (1u << 6))
	{
		goto autodecoder_node_0000000000ce3200;
	}

autodecoder_node_0000000000ce7f40:
	if (instr_word & (1u << 22))
	{
		goto autodecoder_node_0000000000b0a240;
	}

autodecoder_node_0000000000b55500:
	return &smlawbInstruction;

autodecoder_node_0000000000ce3200:
	if (instr_word & (1u << 22))
	{
		goto autodecoder_node_0000000000b0a240;
	}

autodecoder_node_0000000000b565c0:
	return &smlawtInstruction;

autodecoder_node_0000000000cc5de0:
	if ((instr_word & (1u << 21)) == 0)
	{
		goto autodecoder_node_0000000000c79cc0;
	}

autodecoder_node_0000000000c62860:
	if (instr_word & (1u << 16))
	{
		goto autodecoder_node_0000000000c7a380;
	}

autodecoder_node_0000000000c62b20:
	if (instr_word & (1u << 19))
	{
		goto autodecoder_node_0000000000c7a380;
	}

autodecoder_node_0000000000c7d220:
	if (instr_word & (1u << 17))
	{
		goto autodecoder_node_0000000000c7a380;
	}

autodecoder_node_0000000000c6d800:
	if (instr_word & (1u << 18))
	{
		goto autodecoder_node_0000000000c7a380;
	}

autodecoder_node_0000000000c6a080:
	if ((instr_word & (1u << 5)) == 0)
	{
		goto autodecoder_node_0000000000ce7ae0;
	}

autodecoder_node_0000000000ce2cc0:
	if ((instr_word & (1u << 6)) == 0)
	{
		goto autodecoder_node_0000000000c656e0;
	}

autodecoder_node_0000000000ccc240:
	if (instr_word & (1u << 22))
	{
		goto autodecoder_node_0000000000bf4300;
	}

autodecoder_node_0000000000c052e0:
	return &rorInstruction;

autodecoder_node_0000000000ce8aa0:
	if (instr_word & (1u << 7))
	{
		goto autodecoder_node_0000000000c87520;
	}

autodecoder_node_0000000000cc67c0:
	if (instr_word & (1u << 23))
	{
		goto autodecoder_node_0000000000c7b6a0;
	}

autodecoder_node_0000000000ccc8a0:
	if (instr_word & (1u << 8))
	{
		goto autodecoder_node_0000000000cceca0;
	}

autodecoder_node_0000000000cd33e0:
	if (instr_word & (1u << 11))
	{
		goto autodecoder_node_0000000000cc8c00;
	}

autodecoder_node_0000000000c7a1a0:
	if (instr_word & (1u << 10))
	{
		goto autodecoder_node_0000000000cc8c00;
	}

autodecoder_node_0000000000c7bc00:
	if (instr_word & (1u << 9))
	{
		goto autodecoder_node_0000000000cc8c00;
	}

autodecoder_node_0000000000c6c840:
	if (instr_word & (1u << 21))
	{
		goto autodecoder_node_0000000000cc5ae0;
	}

autodecoder_node_0000000000c70540:
	if (instr_word & (1u << 20))
	{
		goto autodecoder_node_0000000000b0a240;
	}

autodecoder_node_0000000000ccdd00:
	if (instr_word & (1u << 5))
	{
		goto autodecoder_node_0000000000b0a240;
	}

autodecoder_node_0000000000cdaf40:
	if ((instr_word & (1u << 6)) == 0)
	{
		goto autodecoder_node_0000000000b0a240;
	}

autodecoder_node_0000000000ce1a80:
	if (instr_word & (1u << 22))
	{
		goto autodecoder_node_0000000000b4a160;
	}

autodecoder_node_0000000000b485e0:
	return &qaddInstruction;

autodecoder_node_0000000000b4a160:
	return &qdaddInstruction;

autodecoder_node_0000000000cc5ae0:
	if (instr_word & (1u << 20))
	{
		goto autodecoder_node_0000000000b0a240;
	}

autodecoder_node_0000000000cc80c0:
	if (instr_word & (1u << 5))
	{
		goto autodecoder_node_0000000000cce420;
	}

autodecoder_node_0000000000c70380:
	if ((instr_word & (1u << 6)) == 0)
	{
		goto autodecoder_node_0000000000b0a240;
	}

autodecoder_node_0000000000c659a0:
	if (instr_word & (1u << 22))
	{
		goto autodecoder_node_0000000000b4cc40;
	}

autodecoder_node_0000000000b4b760:
	return &qsubInstruction;

autodecoder_node_0000000000b4cc40:
	return &qdsubInstruction;

autodecoder_node_0000000000cceca0:
	if ((instr_word & (1u << 11)) == 0)
	{
		goto autodecoder_node_0000000000cc8c00;
	}

autodecoder_node_0000000000c6e2c0:
	if ((instr_word & (1u << 10)) == 0)
	{
		goto autodecoder_node_0000000000cc8c00;
	}

autodecoder_node_0000000000c72b00:
	if (instr_word & (1u << 9))
	{
		goto autodecoder_node_0000000000c69a40;
	}

autodecoder_node_0000000000cc8c00:
	if ((instr_word & (1u << 21)) == 0)
	{
		goto autodecoder_node_0000000000b0a240;
	}

autodecoder_node_0000000000c70440:
	if (instr_word & (1u << 20))
	{
		goto autodecoder_node_0000000000b0a240;
	}
	goto autodecoder_node_0000000000c870e0;

autodecoder_node_0000000000c69a40:
	if ((instr_word & (1u << 21)) == 0)
	{
		goto autodecoder_node_0000000000b0a240;
	}

autodecoder_node_0000000000c795c0:
	if (instr_word & (1u << 20))
	{
		goto autodecoder_node_0000000000b0a240;
	}

autodecoder_node_0000000000c68aa0:
	if ((instr_word & (1u << 16)) == 0)
	{
		goto autodecoder_node_0000000000c870e0;
	}

autodecoder_node_0000000000ce1f40:
	if ((instr_word & (1u << 19)) == 0)
	{
		goto autodecoder_node_0000000000c870e0;
	}

autodecoder_node_0000000000ce2560:
	if ((instr_word & (1u << 17)) == 0)
	{
		goto autodecoder_node_0000000000c870e0;
	}

autodecoder_node_0000000000c7ec20:
	if (instr_word & (1u << 18))
	{
		goto autodecoder_node_0000000000cdb580;
	}

autodecoder_node_0000000000c870e0:
	if ((instr_word & (1u << 5)) == 0)
	{
		goto autodecoder_node_0000000000b0a240;
	}

autodecoder_node_0000000000cce420:
	if (instr_word & (1u << 6))
	{
		goto autodecoder_node_0000000000c61c40;
	}
	goto autodecoder_node_0000000000b0a240;

autodecoder_node_0000000000cdb580:
	if (instr_word & (1u << 5))
	{
		goto autodecoder_node_0000000000cbe340;
	}

autodecoder_node_0000000000cd0e00:
	if (instr_word & (1u << 6))
	{
		goto autodecoder_node_0000000000b0a240;
	}

autodecoder_node_0000000000b50ec0:
	if (instr_word & (1u << 22))
	{
		goto autodecoder_node_0000000000b0a240;
	}

autodecoder_node_0000000000b41160:
	return &bxInstruction;

autodecoder_node_0000000000cbe340:
	if (instr_word & (1u << 6))
	{
		goto autodecoder_node_0000000000c61c40;
	}

autodecoder_node_0000000000ccb340:
	if (instr_word & (1u << 22))
	{
		goto autodecoder_node_0000000000b0a240;
	}
	goto autodecoder_node_0000000000b13760;

autodecoder_node_0000000000c61c40:
	if (instr_word & (1u << 22))
	{
		goto autodecoder_node_0000000000b44aa0;
	}

autodecoder_node_0000000000b43b00:
	return &bkptInstruction;

autodecoder_node_0000000000b44aa0:
	return &smcInstruction;

autodecoder_node_0000000000c7b6a0:
	if ((instr_word & (1u << 21)) == 0)
	{
		goto autodecoder_node_0000000000c79cc0;
	}

autodecoder_node_0000000000b4a680:
	if (instr_word & (1u << 16))
	{
		goto autodecoder_node_0000000000c7a380;
	}

autodecoder_node_0000000000ccb3e0:
	if (instr_word & (1u << 19))
	{
		goto autodecoder_node_0000000000c7a380;
	}

autodecoder_node_0000000000cd0ee0:
	if (instr_word & (1u << 17))
	{
		goto autodecoder_node_0000000000c7a380;
	}

autodecoder_node_0000000000cd9920:
	if (instr_word & (1u << 18))
	{
		goto autodecoder_node_0000000000c7a380;
	}

autodecoder_node_0000000000c6ca60:
	if (instr_word & (1u << 5))
	{
		goto autodecoder_node_0000000000cdcbc0;
	}

autodecoder_node_0000000000ce7ae0:
	if (instr_word & (1u << 6))
	{
		goto autodecoder_node_0000000000ce3c40;
	}

autodecoder_node_0000000000c656e0:
	if (instr_word & (1u << 22))
	{
		goto autodecoder_node_0000000000bf4300;
	}

autodecoder_node_0000000000bf42c0:
	return &lslInstruction;

autodecoder_node_0000000000ce3c40:
	if (instr_word & (1u << 22))
	{
		goto autodecoder_node_0000000000bf4300;
	}

autodecoder_node_0000000000bfc0e0:
	return &asrInstruction;

autodecoder_node_0000000000cdcbc0:
	if (instr_word & (1u << 6))
	{
		goto autodecoder_node_0000000000cddb20;
	}

autodecoder_node_0000000000c84b80:
	if (instr_word & (1u << 22))
	{
		goto autodecoder_node_0000000000bf4300;
	}

autodecoder_node_0000000000bf9da0:
	return &lsrInstruction;

autodecoder_node_0000000000cddb20:
	if (instr_word & (1u << 22))
	{
		goto autodecoder_node_0000000000bf4300;
	}
	goto autodecoder_node_0000000000b0a240;

autodecoder_node_0000000000c87520:
	if (instr_word & (1u << 23))
	{
		goto autodecoder_node_0000000000c661e0;
	}

autodecoder_node_0000000000c67a40:
	if (instr_word & (1u << 8))
	{
		goto autodecoder_node_0000000000c6f980;
	}

autodecoder_node_0000000000c74aa0:
	if (instr_word & (1u << 11))
	{
		goto autodecoder_node_0000000000c6f980;
	}

autodecoder_node_0000000000c5e6c0:
	if (instr_word & (1u << 10))
	{
		goto autodecoder_node_0000000000c6f980;
	}

autodecoder_node_0000000000c7a540:
	if (instr_word & (1u << 9))
	{
		goto autodecoder_node_0000000000c6f980;
	}

autodecoder_node_0000000000c6ff20:
	if (instr_word & (1u << 21))
	{
		goto autodecoder_node_0000000000c6c960;
	}

autodecoder_node_0000000000c741e0:
	if (instr_word & (1u << 20))
	{
		goto autodecoder_node_0000000000b0a240;
	}

autodecoder_node_0000000000cce260:
	if (instr_word & (1u << 5))
	{
		goto autodecoder_node_0000000000b0a240;
	}

autodecoder_node_0000000000c664c0:
	if (instr_word & (1u << 6))
	{
		goto autodecoder_node_0000000000b0a240;
	}

autodecoder_node_0000000000b289e0:
	return &swpInstruction;

autodecoder_node_0000000000c661e0:
	if (instr_word & (1u << 8))
	{
		goto autodecoder_node_0000000000cd1900;
	}

autodecoder_node_0000000000c724a0:
	if (instr_word & (1u << 11))
	{
		goto autodecoder_node_0000000000c6f980;
	}

autodecoder_node_0000000000c69aa0:
	if (instr_word & (1u << 10))
	{
		goto autodecoder_node_0000000000c6f980;
	}

autodecoder_node_0000000000cc55e0:
	if (instr_word & (1u << 9))
	{
		goto autodecoder_node_0000000000c6f980;
	}

autodecoder_node_0000000000c6c960:
	if (instr_word & (1u << 20))
	{
		goto autodecoder_node_0000000000b0a240;
	}

autodecoder_node_0000000000c867c0:
	if (instr_word & (1u << 5))
	{
		goto autodecoder_node_0000000000b0a240;
	}
	goto autodecoder_node_0000000000ce7280;

autodecoder_node_0000000000cd1900:
	if ((instr_word & (1u << 11)) == 0)
	{
		goto autodecoder_node_0000000000c6f980;
	}

autodecoder_node_0000000000cd4e00:
	if ((instr_word & (1u << 10)) == 0)
	{
		goto autodecoder_node_0000000000c6f980;
	}

autodecoder_node_0000000000c695a0:
	if (instr_word & (1u << 9))
	{
		goto autodecoder_node_0000000000b1e060;
	}

autodecoder_node_0000000000c6f980:
	if (instr_word & (1u << 20))
	{
		goto autodecoder_node_0000000000cbe700;
	}

autodecoder_node_0000000000c68d40:
	if (instr_word & (1u << 5))
	{
		goto autodecoder_node_0000000000cbe700;
	}

autodecoder_node_0000000000cc87c0:
	if (instr_word & (1u << 6))
	{
		goto autodecoder_node_0000000000cbe700;
	}
	goto autodecoder_node_0000000000b0a6c0;

autodecoder_node_0000000000b1e060:
	if (instr_word & (1u << 21))
	{
		goto autodecoder_node_0000000000ccbd60;
	}

autodecoder_node_0000000000cdc300:
	if (instr_word & (1u << 20))
	{
		goto autodecoder_node_0000000000cdde60;
	}

autodecoder_node_0000000000c7c4e0:
	if (instr_word & (1u << 5))
	{
		goto autodecoder_node_0000000000cbe700;
	}

autodecoder_node_0000000000bf0aa0:
	if (instr_word & (1u << 6))
	{
		goto autodecoder_node_0000000000cbe700;
	}

autodecoder_node_0000000000cdd420:
	if (instr_word & (1u << 22))
	{
		goto autodecoder_node_0000000000b31560;
	}

autodecoder_node_0000000000b36b00:
	return &strexInstruction;

autodecoder_node_0000000000b31560:
	return &strexbInstruction;

autodecoder_node_0000000000cdde60:
	if (instr_word & (1u << 5))
	{
		goto autodecoder_node_0000000000cbe700;
	}

autodecoder_node_0000000000c68060:
	if (instr_word & (1u << 6))
	{
		goto autodecoder_node_0000000000cbe700;
	}

autodecoder_node_0000000000bab040:
	if ((instr_word & (1u << 3)) == 0)
	{
		goto autodecoder_node_0000000000cbe700;
	}

autodecoder_node_0000000000b549a0:
	if ((instr_word & (1u << 2)) == 0)
	{
		goto autodecoder_node_0000000000cbe700;
	}

autodecoder_node_0000000000c5f720:
	if ((instr_word & (1u << 1)) == 0)
	{
		goto autodecoder_node_0000000000cbe700;
	}

autodecoder_node_0000000000ccaaa0:
	if ((instr_word & (1u << 0)) == 0)
	{
		goto autodecoder_node_0000000000cbe700;
	}

autodecoder_node_0000000000c7d900:
	if (instr_word & (1u << 22))
	{
		goto autodecoder_node_0000000000b2d080;
	}

autodecoder_node_0000000000b357c0:
	return &ldrexInstruction;

autodecoder_node_0000000000b2d080:
	return &ldrexbInstruction;

autodecoder_node_0000000000ccbd60:
	if (instr_word & (1u << 20))
	{
		goto autodecoder_node_0000000000c86500;
	}

autodecoder_node_0000000000c6b980:
	if (instr_word & (1u << 5))
	{
		goto autodecoder_node_0000000000cbe700;
	}

autodecoder_node_0000000000cd3f60:
	if (instr_word & (1u << 6))
	{
		goto autodecoder_node_0000000000cbe700;
	}

autodecoder_node_0000000000c5e5e0:
	if (instr_word & (1u << 22))
	{
		goto autodecoder_node_0000000000b33f40;
	}

autodecoder_node_0000000000b32aa0:
	return &strexdInstruction;

autodecoder_node_0000000000b33f40:
	return &strexhInstruction;

autodecoder_node_0000000000c86500:
	if (instr_word & (1u << 5))
	{
		goto autodecoder_node_0000000000cbe700;
	}

autodecoder_node_0000000000ccdf60:
	if (instr_word & (1u << 6))
	{
		goto autodecoder_node_0000000000cbe700;
	}

autodecoder_node_0000000000c72420:
	if ((instr_word & (1u << 3)) == 0)
	{
		goto autodecoder_node_0000000000cbe700;
	}

autodecoder_node_0000000000cc7d80:
	if ((instr_word & (1u << 2)) == 0)
	{
		goto autodecoder_node_0000000000cbe700;
	}

autodecoder_node_0000000000cce680:
	if ((instr_word & (1u << 1)) == 0)
	{
		goto autodecoder_node_0000000000cbe700;
	}

autodecoder_node_0000000000cc73e0:
	if ((instr_word & (1u << 0)) == 0)
	{
		goto autodecoder_node_0000000000cbe700;
	}

autodecoder_node_0000000000c663c0:
	if (instr_word & (1u << 22))
	{
		goto autodecoder_node_0000000000b30080;
	}

autodecoder_node_0000000000b2e9c0:
	return &ldrexdInstruction;

autodecoder_node_0000000000b30080:
	return &ldrexhInstruction;

autodecoder_node_0000000000c794a0:
	if (instr_word & (1u << 24))
	{
		goto autodecoder_node_0000000000ce7920;
	}

autodecoder_node_0000000000b52740:
	if (instr_word & (1u << 23))
	{
		goto autodecoder_node_0000000000cdd2c0;
	}

autodecoder_node_0000000000c73c40:
	if (instr_word & (1u << 21))
	{
		goto autodecoder_node_0000000000b452c0;
	}

autodecoder_node_0000000000cddd00:
	if (instr_word & (1u << 22))
	{
		goto autodecoder_node_0000000000ba0760;
	}

autodecoder_node_0000000000b670c0:
	return &andInstruction;

autodecoder_node_0000000000ba0760:
	return &subInstruction;

autodecoder_node_0000000000b452c0:
	if (instr_word & (1u << 22))
	{
		goto autodecoder_node_0000000000ba8700;
	}

autodecoder_node_0000000000b988c0:
	return &eorInstruction;

autodecoder_node_0000000000ba8700:
	return &rsbInstruction;

autodecoder_node_0000000000cdd2c0:
	if (instr_word & (1u << 21))
	{
		goto autodecoder_node_0000000000c1ae80;
	}

autodecoder_node_0000000000c6e8a0:
	if (instr_word & (1u << 22))
	{
		goto autodecoder_node_0000000000bbff20;
	}

autodecoder_node_0000000000bb0440:
	return &addInstruction;

autodecoder_node_0000000000bbff20:
	return &sbcInstruction;

autodecoder_node_0000000000c1ae80:
	if (instr_word & (1u << 22))
	{
		goto autodecoder_node_0000000000bc7c60;
	}

autodecoder_node_0000000000bb8340:
	return &adcInstruction;

autodecoder_node_0000000000bc7c60:
	return &rscInstruction;

autodecoder_node_0000000000ce7920:
	if (instr_word & (1u << 4))
	{
		goto autodecoder_node_0000000000c67ae0;
	}

autodecoder_node_0000000000c7a0e0:
	if (instr_word & (1u << 7))
	{
		goto autodecoder_node_0000000000c85a40;
	}

autodecoder_node_0000000000cceb20:
	if (instr_word & (1u << 23))
	{
		goto autodecoder_node_0000000000cbec40;
	}

autodecoder_node_0000000000cdec00:
	if (instr_word & (1u << 8))
	{
		goto autodecoder_node_0000000000cd25e0;
	}

autodecoder_node_0000000000ce7de0:
	if (instr_word & (1u << 11))
	{
		goto autodecoder_node_0000000000cd25e0;
	}

autodecoder_node_0000000000c70860:
	if (instr_word & (1u << 10))
	{
		goto autodecoder_node_0000000000cd25e0;
	}

autodecoder_node_0000000000ce79c0:
	if (instr_word & (1u << 9))
	{
		goto autodecoder_node_0000000000cd25e0;
	}

autodecoder_node_0000000000cceaa0:
	if ((instr_word & (1u << 21)) == 0)
	{
		goto autodecoder_node_0000000000c70c20;
	}

autodecoder_node_0000000000cd3e20:
	if (instr_word & (1u << 20))
	{
		goto autodecoder_node_0000000000b0a240;
	}

autodecoder_node_0000000000c5f2e0:
	if (instr_word & (1u << 16))
	{
		goto autodecoder_node_0000000000bcffe0;
	}

autodecoder_node_0000000000cdd3a0:
	if (instr_word & (1u << 19))
	{
		goto autodecoder_node_0000000000bcffe0;
	}

autodecoder_node_0000000000c718e0:
	if (instr_word & (1u << 17))
	{
		goto autodecoder_node_0000000000bcffe0;
	}

autodecoder_node_0000000000c73d60:
	if (instr_word & (1u << 18))
	{
		goto autodecoder_node_0000000000bcffe0;
	}

autodecoder_node_0000000000cdee60:
	if (instr_word & (1u << 5))
	{
		goto autodecoder_node_0000000000bcffe0;
	}

autodecoder_node_0000000000cc9440:
	if (instr_word & (1u << 6))
	{
		goto autodecoder_node_0000000000bcffe0;
	}

autodecoder_node_0000000000ce15c0:
	if (instr_word & (1u << 3))
	{
		goto autodecoder_node_0000000000bcffe0;
	}

autodecoder_node_0000000000c708e0:
	if (instr_word & (1u << 2))
	{
		goto autodecoder_node_0000000000cdc020;
	}

autodecoder_node_0000000000c658e0:
	if (instr_word & (1u << 1))
	{
		goto autodecoder_node_0000000000c791e0;
	}

autodecoder_node_0000000000ce2de0:
	if ((instr_word & (1u << 0)) == 0)
	{
		goto autodecoder_node_0000000000c7da60;
	}

autodecoder_node_0000000000c7cd40:
	if (instr_word & (1u << 22))
	{
		goto autodecoder_node_0000000000bcffe0;
	}

autodecoder_node_0000000000be2000:
	return &yieldInstruction;

autodecoder_node_0000000000c791e0:
	if (instr_word & (1u << 0))
	{
		goto autodecoder_node_0000000000cc7f00;
	}

autodecoder_node_0000000000cdf600:
	if (instr_word & (1u << 22))
	{
		goto autodecoder_node_0000000000bcffe0;
	}

autodecoder_node_0000000000be3840:
	return &wfeInstruction;

autodecoder_node_0000000000cc7f00:
	if (instr_word & (1u << 22))
	{
		goto autodecoder_node_0000000000bcffe0;
	}

autodecoder_node_0000000000be4580:
	return &wfiInstruction;

autodecoder_node_0000000000cdc020:
	if (instr_word & (1u << 1))
	{
		goto autodecoder_node_0000000000bcffe0;
	}

autodecoder_node_0000000000c604a0:
	if (instr_word & (1u << 0))
	{
		goto autodecoder_node_0000000000bcffe0;
	}

autodecoder_node_0000000000ce6900:
	if (instr_word & (1u << 22))
	{
		goto autodecoder_node_0000000000bcffe0;
	}

autodecoder_node_0000000000bd5920:
	return &sevInstruction;

autodecoder_node_0000000000cd25e0:
	if ((instr_word & (1u << 21)) == 0)
	{
		goto autodecoder_node_0000000000c70c20;
	}

autodecoder_node_0000000000cdb1a0:
	if (instr_word & (1u << 20))
	{
		goto autodecoder_node_0000000000b0a240;
	}

autodecoder_node_0000000000cd2240:
	if (instr_word & (1u << 16))
	{
		goto autodecoder_node_0000000000bcffe0;
	}

autodecoder_node_0000000000cd9ba0:
	if (instr_word & (1u << 19))
	{
		goto autodecoder_node_0000000000bcffe0;
	}

autodecoder_node_0000000000c8aa80:
	if (instr_word & (1u << 17))
	{
		goto autodecoder_node_0000000000bcffe0;
	}

autodecoder_node_0000000000c66c20:
	if (instr_word & (1u << 18))
	{
		goto autodecoder_node_0000000000bcffe0;
	}

autodecoder_node_0000000000c7e8e0:
	if (instr_word & (1u << 5))
	{
		goto autodecoder_node_0000000000bcffe0;
	}

autodecoder_node_0000000000cccba0:
	if (instr_word & (1u << 6))
	{
		goto autodecoder_node_0000000000bcffe0;
	}

autodecoder_node_0000000000c68600:
	if (instr_word & (1u << 3))
	{
		goto autodecoder_node_0000000000bcffe0;
	}

autodecoder_node_0000000000ce6bc0:
	if (instr_word & (1u << 2))
	{
		goto autodecoder_node_0000000000bcffe0;
	}

autodecoder_node_0000000000c6e760:
	if (instr_word & (1u << 1))
	{
		goto autodecoder_node_0000000000bcffe0;
	}

autodecoder_node_0000000000cd0120:
	if (instr_word & (1u << 0))
	{
		goto autodecoder_node_0000000000bcffe0;
	}

autodecoder_node_0000000000c7da60:
	if (instr_word & (1u << 22))
	{
		goto autodecoder_node_0000000000bcffe0;
	}

autodecoder_node_0000000000bd6ec0:
	return &nopInstruction;

autodecoder_node_0000000000c67ae0:
	if (instr_word & (1u << 7))
	{
		goto autodecoder_node_0000000000ccb360;
	}

autodecoder_node_0000000000c85a40:
	if (instr_word & (1u << 23))
	{
		goto autodecoder_node_0000000000cbec40;
	}
	goto autodecoder_node_0000000000b3e180;

autodecoder_node_0000000000ccb360:
	if (instr_word & (1u << 23))
	{
		goto autodecoder_node_0000000000cbec40;
	}

autodecoder_node_0000000000cd3e40:
	if (instr_word & (1u << 8))
	{
		goto autodecoder_node_0000000000b3e180;
	}

autodecoder_node_0000000000ba7060:
	if (instr_word & (1u << 11))
	{
		goto autodecoder_node_0000000000b3e180;
	}

autodecoder_node_0000000000c7a0c0:
	if (instr_word & (1u << 10))
	{
		goto autodecoder_node_0000000000b3e180;
	}

autodecoder_node_0000000000ce76a0:
	if (instr_word & (1u << 9))
	{
		goto autodecoder_node_0000000000b3e180;
	}

autodecoder_node_0000000000cd9800:
	if ((instr_word & (1u << 21)) == 0)
	{
		goto autodecoder_node_0000000000c70c20;
	}

autodecoder_node_0000000000c60960:
	if (instr_word & (1u << 20))
	{
		goto autodecoder_node_0000000000b0a240;
	}

autodecoder_node_0000000000c79140:
	if (instr_word & (1u << 16))
	{
		goto autodecoder_node_0000000000bcffe0;
	}

autodecoder_node_0000000000ce7340:
	if (instr_word & (1u << 19))
	{
		goto autodecoder_node_0000000000bcffe0;
	}

autodecoder_node_0000000000ccb700:
	if (instr_word & (1u << 17))
	{
		goto autodecoder_node_0000000000bcffe0;
	}

autodecoder_node_0000000000c68020:
	if (instr_word & (1u << 18))
	{
		goto autodecoder_node_0000000000bcffe0;
	}

autodecoder_node_0000000000c6b760:
	if ((instr_word & (1u << 5)) == 0)
	{
		goto autodecoder_node_0000000000bcffe0;
	}

autodecoder_node_0000000000c7f440:
	if ((instr_word & (1u << 6)) == 0)
	{
		goto autodecoder_node_0000000000bcffe0;
	}

autodecoder_node_0000000000c86220:
	if (instr_word & (1u << 22))
	{
		goto autodecoder_node_0000000000bcffe0;
	}

autodecoder_node_0000000000bd8e80:
	return &dbgInstruction;

autodecoder_node_0000000000b3e180:
	if (instr_word & (1u << 21))
	{
		goto autodecoder_node_0000000000c78240;
	}

autodecoder_node_0000000000c70c20:
	if (instr_word & (1u << 20))
	{
		goto autodecoder_node_0000000000b0a240;
	}

autodecoder_node_0000000000c6a9e0:
	if ((instr_word & (1u << 22)) == 0)
	{
		goto autodecoder_node_0000000000b0a240;
	}

autodecoder_node_0000000000bd4940:
	return &movtInstruction;

autodecoder_node_0000000000c78240:
	if (instr_word & (1u << 20))
	{
		goto autodecoder_node_0000000000b0a240;
	}

autodecoder_node_0000000000bcffe0:
	return &msrInstruction;

autodecoder_node_0000000000cbec40:
	if (instr_word & (1u << 21))
	{
		goto autodecoder_node_0000000000c69f20;
	}

autodecoder_node_0000000000c79cc0:
	if (instr_word & (1u << 22))
	{
		goto autodecoder_node_0000000000bf83e0;
	}

autodecoder_node_0000000000be8140:
	return &orrInstruction;

autodecoder_node_0000000000bf83e0:
	return &bicInstruction;

autodecoder_node_0000000000c69f20:
	if (instr_word & (1u << 16))
	{
		goto autodecoder_node_0000000000c7a380;
	}

autodecoder_node_0000000000cdf6c0:
	if (instr_word & (1u << 19))
	{
		goto autodecoder_node_0000000000c7a380;
	}

autodecoder_node_0000000000ce50c0:
	if (instr_word & (1u << 17))
	{
		goto autodecoder_node_0000000000c7a380;
	}

autodecoder_node_0000000000cd0ea0:
	if (instr_word & (1u << 18))
	{
		goto autodecoder_node_0000000000c7a380;
	}

autodecoder_node_0000000000c4dca0:
	if (instr_word & (1u << 22))
	{
		goto autodecoder_node_0000000000bf4300;
	}

autodecoder_node_0000000000bf0560:
	return &movInstruction;

autodecoder_node_0000000000c7a380:
	if ((instr_word & (1u << 22)) == 0)
	{
		goto autodecoder_node_0000000000b0a6c0;
	}

autodecoder_node_0000000000bf4300:
	return &mvnInstruction;

autodecoder_node_0000000000b595e0:
	if ((instr_word & (1u << 25)) == 0)
	{
		goto autodecoder_node_0000000000b53780;
	}

autodecoder_node_0000000000c74ac0:
	if (instr_word & (1u << 24))
	{
		goto autodecoder_node_0000000000cd44c0;
	}

autodecoder_node_0000000000cd1820:
	if ((instr_word & (1u << 4)) == 0)
	{
		goto autodecoder_node_0000000000b53780;
	}

autodecoder_node_0000000000cd2720:
	if (instr_word & (1u << 7))
	{
		goto autodecoder_node_0000000000c79500;
	}

autodecoder_node_0000000000c5dd00:
	if (instr_word & (1u << 23))
	{
		goto autodecoder_node_0000000000ca3d80;
	}

autodecoder_node_0000000000c678c0:
	if ((instr_word & (1u << 8)) == 0)
	{
		goto autodecoder_node_0000000000b0a6c0;
	}

autodecoder_node_0000000000ccf7a0:
	if ((instr_word & (1u << 11)) == 0)
	{
		goto autodecoder_node_0000000000b0a6c0;
	}

autodecoder_node_0000000000cd4fa0:
	if ((instr_word & (1u << 10)) == 0)
	{
		goto autodecoder_node_0000000000b0a6c0;
	}

autodecoder_node_0000000000ce6b40:
	if ((instr_word & (1u << 9)) == 0)
	{
		goto autodecoder_node_0000000000b0a6c0;
	}

autodecoder_node_0000000000c7d040:
	if (instr_word & (1u << 21))
	{
		goto autodecoder_node_0000000000bf0ac0;
	}

autodecoder_node_0000000000cd29a0:
	if ((instr_word & (1u << 20)) == 0)
	{
		goto autodecoder_node_0000000000b0a6c0;
	}

autodecoder_node_0000000000ce0b60:
	if (instr_word & (1u << 5))
	{
		goto autodecoder_node_0000000000c6dd20;
	}

autodecoder_node_0000000000c70b20:
	if (instr_word & (1u << 6))
	{
		goto autodecoder_node_0000000000c66fe0;
	}

autodecoder_node_0000000000c6e600:
	if (instr_word & (1u << 22))
	{
		goto autodecoder_node_0000000000bae980;
	}

autodecoder_node_0000000000bc1dc0:
	return &sadd16Instruction;

autodecoder_node_0000000000bae980:
	return &uadd16Instruction;

autodecoder_node_0000000000c66fe0:
	if (instr_word & (1u << 22))
	{
		goto autodecoder_node_0000000000bb3760;
	}

autodecoder_node_0000000000bb6da0:
	return &ssubaddxInstruction;

autodecoder_node_0000000000bb3760:
	return &usubaddxInstruction;

autodecoder_node_0000000000c6dd20:
	if (instr_word & (1u << 6))
	{
		goto autodecoder_node_0000000000cdb540;
	}

autodecoder_node_0000000000ccaf80:
	if (instr_word & (1u << 22))
	{
		goto autodecoder_node_0000000000bb0ce0;
	}

autodecoder_node_0000000000bc4100:
	return &saddsubxInstruction;

autodecoder_node_0000000000bb0ce0:
	return &uaddsubxInstruction;

autodecoder_node_0000000000cdb540:
	if (instr_word & (1u << 22))
	{
		goto autodecoder_node_0000000000bb1ca0;
	}

autodecoder_node_0000000000bc51c0:
	return &ssub16Instruction;

autodecoder_node_0000000000bb1ca0:
	return &usub16Instruction;

autodecoder_node_0000000000bf0ac0:
	if (instr_word & (1u << 20))
	{
		goto autodecoder_node_0000000000c5e3a0;
	}

autodecoder_node_0000000000b4f600:
	if (instr_word & (1u << 5))
	{
		goto autodecoder_node_0000000000c5d940;
	}

autodecoder_node_0000000000c78ea0:
	if (instr_word & (1u << 6))
	{
		goto autodecoder_node_0000000000c7ae40;
	}

autodecoder_node_0000000000ce3c80:
	if (instr_word & (1u << 22))
	{
		goto autodecoder_node_0000000000baab40;
	}

autodecoder_node_0000000000bcb060:
	return &qadd16Instruction;

autodecoder_node_0000000000baab40:
	return &uqadd16Instruction;

autodecoder_node_0000000000c7ae40:
	if (instr_word & (1u << 22))
	{
		goto autodecoder_node_0000000000b9f960;
	}

autodecoder_node_0000000000bc08a0:
	return &qsubaddxInstruction;

autodecoder_node_0000000000b9f960:
	return &uqsubaddxInstruction;

autodecoder_node_0000000000c5d940:
	if (instr_word & (1u << 6))
	{
		goto autodecoder_node_0000000000cde780;
	}

autodecoder_node_0000000000ccaa00:
	if (instr_word & (1u << 22))
	{
		goto autodecoder_node_0000000000bacca0;
	}

autodecoder_node_0000000000bcd640:
	return &qaddsubxInstruction;

autodecoder_node_0000000000bacca0:
	return &uqaddsubxInstruction;

autodecoder_node_0000000000cde780:
	if (instr_word & (1u << 22))
	{
		goto autodecoder_node_0000000000b9ddc0;
	}

autodecoder_node_0000000000bbe8a0:
	return &qsub16Instruction;

autodecoder_node_0000000000b9ddc0:
	return &uqsub16Instruction;

autodecoder_node_0000000000c5e3a0:
	if (instr_word & (1u << 5))
	{
		goto autodecoder_node_0000000000c70f60;
	}

autodecoder_node_0000000000ccf6e0:
	if (instr_word & (1u << 6))
	{
		goto autodecoder_node_0000000000c5f0c0;
	}

autodecoder_node_0000000000cbfbe0:
	if (instr_word & (1u << 22))
	{
		goto autodecoder_node_0000000000bb49a0;
	}

autodecoder_node_0000000000bb8180:
	return &shadd16Instruction;

autodecoder_node_0000000000bb49a0:
	return &uhadd16Instruction;

autodecoder_node_0000000000c5f0c0:
	if (instr_word & (1u << 22))
	{
		goto autodecoder_node_0000000000ba9960;
	}

autodecoder_node_0000000000bbd5a0:
	return &shsubaddxInstruction;

autodecoder_node_0000000000ba9960:
	return &uhsubaddxInstruction;

autodecoder_node_0000000000c70f60:
	if (instr_word & (1u << 6))
	{
		goto autodecoder_node_0000000000c9b3a0;
	}

autodecoder_node_0000000000cd19e0:
	if (instr_word & (1u << 22))
	{
		goto autodecoder_node_0000000000ba6d40;
	}

autodecoder_node_0000000000bba7e0:
	return &shaddsubxInstruction;

autodecoder_node_0000000000ba6d40:
	return &uhaddsubxInstruction;

autodecoder_node_0000000000c9b3a0:
	if (instr_word & (1u << 22))
	{
		goto autodecoder_node_0000000000ba7dc0;
	}

autodecoder_node_0000000000bbb9c0:
	return &shsub16Instruction;

autodecoder_node_0000000000ba7dc0:
	return &uhsub16Instruction;

autodecoder_node_0000000000ca3d80:
	if (instr_word & (1u << 8))
	{
		goto autodecoder_node_0000000000ccfe80;
	}

autodecoder_node_0000000000cd49c0:
	if (instr_word & (1u << 9))
	{
		goto autodecoder_node_0000000000cc9680;
	}

autodecoder_node_0000000000ce24a0:
	if (instr_word & (1u << 21))
	{
		goto autodecoder_node_0000000000c66a60;
	}

autodecoder_node_0000000000c65b80:
	if (instr_word & (1u << 20))
	{
		goto autodecoder_node_0000000000b0a6c0;
	}

autodecoder_node_0000000000cc67a0:
	if ((instr_word & (1u << 16)) == 0)
	{
		goto autodecoder_node_0000000000c68480;
	}

autodecoder_node_0000000000cc6d80:
	if ((instr_word & (1u << 19)) == 0)
	{
		goto autodecoder_node_0000000000c68480;
	}

autodecoder_node_0000000000c706a0:
	if ((instr_word & (1u << 17)) == 0)
	{
		goto autodecoder_node_0000000000c68480;
	}

autodecoder_node_0000000000ce2000:
	if (instr_word & (1u << 18))
	{
		goto autodecoder_node_0000000000cc8a00;
	}

autodecoder_node_0000000000c68480:
	if ((instr_word & (1u << 5)) == 0)
	{
		goto autodecoder_node_0000000000cc8c60;
	}

autodecoder_node_0000000000ce14a0:
	if ((instr_word & (1u << 6)) == 0)
	{
		goto autodecoder_node_0000000000b0a6c0;
	}

autodecoder_node_0000000000c71440:
	if (instr_word & (1u << 22))
	{
		goto autodecoder_node_0000000000b50840;
	}

autodecoder_node_0000000000b54ce0:
	return &sxtab16Instruction;

autodecoder_node_0000000000b50840:
	return &uxtab16Instruction;

autodecoder_node_0000000000cc8a00:
	if ((instr_word & (1u << 5)) == 0)
	{
		goto autodecoder_node_0000000000cc8c60;
	}

autodecoder_node_0000000000c70920:
	if ((instr_word & (1u << 6)) == 0)
	{
		goto autodecoder_node_0000000000b0a6c0;
	}

autodecoder_node_0000000000cd00a0:
	if (instr_word & (1u << 22))
	{
		goto autodecoder_node_0000000000b0a240;
	}

autodecoder_node_0000000000b995c0:
	return &sxtb16Instruction;

autodecoder_node_0000000000c66a60:
	if (instr_word & (1u << 20))
	{
		goto autodecoder_node_0000000000c6f4c0;
	}

autodecoder_node_0000000000c67de0:
	if ((instr_word & (1u << 16)) == 0)
	{
		goto autodecoder_node_0000000000cbf980;
	}

autodecoder_node_0000000000c6ba00:
	if ((instr_word & (1u << 19)) == 0)
	{
		goto autodecoder_node_0000000000cbf980;
	}

autodecoder_node_0000000000c66e80:
	if ((instr_word & (1u << 17)) == 0)
	{
		goto autodecoder_node_0000000000cbf980;
	}

autodecoder_node_0000000000cdff40:
	if (instr_word & (1u << 18))
	{
		goto autodecoder_node_0000000000ce52e0;
	}

autodecoder_node_0000000000cbf980:
	if ((instr_word & (1u << 5)) == 0)
	{
		goto autodecoder_node_0000000000c67920;
	}

autodecoder_node_0000000000b781c0:
	if ((instr_word & (1u << 6)) == 0)
	{
		goto autodecoder_node_0000000000b0a6c0;
	}

autodecoder_node_0000000000c7ee60:
	if (instr_word & (1u << 22))
	{
		goto autodecoder_node_0000000000b44be0;
	}

autodecoder_node_0000000000b58e40:
	return &sxtabInstruction;

autodecoder_node_0000000000b44be0:
	return &uxtabInstruction;

autodecoder_node_0000000000ce52e0:
	if (instr_word & (1u << 5))
	{
		goto autodecoder_node_0000000000ce7280;
	}
	goto autodecoder_node_0000000000c67920;

autodecoder_node_0000000000c6f4c0:
	if ((instr_word & (1u << 16)) == 0)
	{
		goto autodecoder_node_0000000000c84a40;
	}

autodecoder_node_0000000000c6c5a0:
	if ((instr_word & (1u << 19)) == 0)
	{
		goto autodecoder_node_0000000000c84a40;
	}

autodecoder_node_0000000000ce66a0:
	if ((instr_word & (1u << 17)) == 0)
	{
		goto autodecoder_node_0000000000c84a40;
	}

autodecoder_node_0000000000ccd9a0:
	if (instr_word & (1u << 18))
	{
		goto autodecoder_node_0000000000c7c0e0;
	}

autodecoder_node_0000000000c84a40:
	if ((instr_word & (1u << 5)) == 0)
	{
		goto autodecoder_node_0000000000c67920;
	}

autodecoder_node_0000000000c7ff00:
	if ((instr_word & (1u << 6)) == 0)
	{
		goto autodecoder_node_0000000000b0a6c0;
	}

autodecoder_node_0000000000ccd040:
	if (instr_word & (1u << 22))
	{
		goto autodecoder_node_0000000000b4cc20;
	}

autodecoder_node_0000000000b61000:
	return &sxtahInstruction;

autodecoder_node_0000000000b4cc20:
	return &uxtahInstruction;

autodecoder_node_0000000000c7c0e0:
	if ((instr_word & (1u << 5)) == 0)
	{
		goto autodecoder_node_0000000000c67920;
	}

autodecoder_node_0000000000bc33e0:
	if ((instr_word & (1u << 6)) == 0)
	{
		goto autodecoder_node_0000000000b0a6c0;
	}

autodecoder_node_0000000000cbfee0:
	if (instr_word & (1u << 22))
	{
		goto autodecoder_node_0000000000b0a240;
	}

autodecoder_node_0000000000ba4ba0:
	return &sxthInstruction;

autodecoder_node_0000000000ccfe80:
	if ((instr_word & (1u << 11)) == 0)
	{
		goto autodecoder_node_0000000000cc9680;
	}

autodecoder_node_0000000000c69640:
	if ((instr_word & (1u << 10)) == 0)
	{
		goto autodecoder_node_0000000000cc9680;
	}

autodecoder_node_0000000000ccc280:
	if ((instr_word & (1u << 9)) == 0)
	{
		goto autodecoder_node_0000000000cc9680;
	}

autodecoder_node_0000000000b39fe0:
	if ((instr_word & (1u << 21)) == 0)
	{
		goto autodecoder_node_0000000000c5f840;
	}

autodecoder_node_0000000000c79d00:
	if (instr_word & (1u << 20))
	{
		goto autodecoder_node_0000000000c6fe20;
	}

autodecoder_node_0000000000c8a580:
	if ((instr_word & (1u << 5)) == 0)
	{
		goto autodecoder_node_0000000000c67920;
	}

autodecoder_node_0000000000c851e0:
	if (instr_word & (1u << 6))
	{
		goto autodecoder_node_0000000000b0a6c0;
	}

autodecoder_node_0000000000ce3e40:
	if (instr_word & (1u << 22))
	{
		goto autodecoder_node_0000000000b1db80;
	}

autodecoder_node_0000000000b26220:
	return &ssat16Instruction;

autodecoder_node_0000000000b1db80:
	return &usat16Instruction;

autodecoder_node_0000000000c6fe20:
	if ((instr_word & (1u << 16)) == 0)
	{
		goto autodecoder_node_0000000000c7cc00;
	}

autodecoder_node_0000000000cd97a0:
	if ((instr_word & (1u << 19)) == 0)
	{
		goto autodecoder_node_0000000000c7cc00;
	}

autodecoder_node_0000000000c6a100:
	if ((instr_word & (1u << 17)) == 0)
	{
		goto autodecoder_node_0000000000c7cc00;
	}

autodecoder_node_0000000000c6a7c0:
	if ((instr_word & (1u << 18)) == 0)
	{
		goto autodecoder_node_0000000000c7cc00;
	}

autodecoder_node_0000000000ce1ac0:
	if ((instr_word & (1u << 5)) == 0)
	{
		goto autodecoder_node_0000000000c67920;
	}

autodecoder_node_0000000000bd3da0:
	if (instr_word & (1u << 6))
	{
		goto autodecoder_node_0000000000b0a6c0;
	}

autodecoder_node_0000000000cc6940:
	if (instr_word & (1u << 22))
	{
		goto autodecoder_node_0000000000bcf120;
	}

autodecoder_node_0000000000ba0f80:
	return &revInstruction;

autodecoder_node_0000000000bcf120:
	return &rbitInstruction;

autodecoder_node_0000000000c79500:
	if (instr_word & (1u << 23))
	{
		goto autodecoder_node_0000000000c694a0;
	}

autodecoder_node_0000000000ccefe0:
	if ((instr_word & (1u << 8)) == 0)
	{
		goto autodecoder_node_0000000000b0a6c0;
	}

autodecoder_node_0000000000cdb5c0:
	if ((instr_word & (1u << 11)) == 0)
	{
		goto autodecoder_node_0000000000b0a6c0;
	}

autodecoder_node_0000000000ccd9c0:
	if ((instr_word & (1u << 10)) == 0)
	{
		goto autodecoder_node_0000000000b0a6c0;
	}

autodecoder_node_0000000000c6b060:
	if ((instr_word & (1u << 9)) == 0)
	{
		goto autodecoder_node_0000000000b0a6c0;
	}

autodecoder_node_0000000000c62280:
	if (instr_word & (1u << 21))
	{
		goto autodecoder_node_0000000000c857c0;
	}

autodecoder_node_0000000000c5dca0:
	if ((instr_word & (1u << 20)) == 0)
	{
		goto autodecoder_node_0000000000b0a6c0;
	}

autodecoder_node_0000000000ce57e0:
	if (instr_word & (1u << 5))
	{
		goto autodecoder_node_0000000000c6d5c0;
	}

autodecoder_node_0000000000cdb060:
	if (instr_word & (1u << 6))
	{
		goto autodecoder_node_0000000000b0a6c0;
	}

autodecoder_node_0000000000ce49e0:
	if (instr_word & (1u << 22))
	{
		goto autodecoder_node_0000000000bafb80;
	}

autodecoder_node_0000000000bc3020:
	return &sadd8Instruction;

autodecoder_node_0000000000bafb80:
	return &uadd8Instruction;

autodecoder_node_0000000000c6d5c0:
	if ((instr_word & (1u << 6)) == 0)
	{
		goto autodecoder_node_0000000000b0a6c0;
	}

autodecoder_node_0000000000cc7400:
	if (instr_word & (1u << 22))
	{
		goto autodecoder_node_0000000000bb2a80;
	}

autodecoder_node_0000000000bb60a0:
	return &ssub8Instruction;

autodecoder_node_0000000000bb2a80:
	return &usub8Instruction;

autodecoder_node_0000000000c857c0:
	if (instr_word & (1u << 20))
	{
		goto autodecoder_node_0000000000c5f4c0;
	}

autodecoder_node_0000000000c6db80:
	if (instr_word & (1u << 5))
	{
		goto autodecoder_node_0000000000ccdc20;
	}

autodecoder_node_0000000000ce90e0:
	if (instr_word & (1u << 6))
	{
		goto autodecoder_node_0000000000b0a6c0;
	}

autodecoder_node_0000000000cccc80:
	if (instr_word & (1u << 22))
	{
		goto autodecoder_node_0000000000babce0;
	}

autodecoder_node_0000000000bcc5e0:
	return &qadd8Instruction;

autodecoder_node_0000000000babce0:
	return &uqadd8Instruction;

autodecoder_node_0000000000ccdc20:
	if ((instr_word & (1u << 6)) == 0)
	{
		goto autodecoder_node_0000000000b0a6c0;
	}

autodecoder_node_0000000000c660c0:
	if (instr_word & (1u << 22))
	{
		goto autodecoder_node_0000000000b9ec20;
	}

autodecoder_node_0000000000bbf9a0:
	return &qsub8Instruction;

autodecoder_node_0000000000b9ec20:
	return &uqsub8Instruction;

autodecoder_node_0000000000c5f4c0:
	if (instr_word & (1u << 5))
	{
		goto autodecoder_node_0000000000cc56c0;
	}

autodecoder_node_0000000000c6b700:
	if (instr_word & (1u << 6))
	{
		goto autodecoder_node_0000000000b0a6c0;
	}

autodecoder_node_0000000000c6e3e0:
	if (instr_word & (1u << 22))
	{
		goto autodecoder_node_0000000000ba5b80;
	}

autodecoder_node_0000000000bb95a0:
	return &shadd8Instruction;

autodecoder_node_0000000000ba5b80:
	return &uhadd8Instruction;

autodecoder_node_0000000000cc56c0:
	if ((instr_word & (1u << 6)) == 0)
	{
		goto autodecoder_node_0000000000b0a6c0;
	}

autodecoder_node_0000000000cdc600:
	if (instr_word & (1u << 22))
	{
		goto autodecoder_node_0000000000ba8ca0;
	}

autodecoder_node_0000000000bbc840:
	return &shsub8Instruction;

autodecoder_node_0000000000ba8ca0:
	return &uhsub8Instruction;

autodecoder_node_0000000000c694a0:
	if ((instr_word & (1u << 8)) == 0)
	{
		goto autodecoder_node_0000000000cc9680;
	}

autodecoder_node_0000000000cdc7e0:
	if ((instr_word & (1u << 11)) == 0)
	{
		goto autodecoder_node_0000000000cc9680;
	}

autodecoder_node_0000000000c7c9e0:
	if ((instr_word & (1u << 10)) == 0)
	{
		goto autodecoder_node_0000000000cc9680;
	}

autodecoder_node_0000000000ce0fc0:
	if (instr_word & (1u << 9))
	{
		goto autodecoder_node_0000000000c5ea60;
	}

autodecoder_node_0000000000cc9680:
	if (instr_word & (1u << 21))
	{
		goto autodecoder_node_0000000000c7cc00;
	}

autodecoder_node_0000000000c5f840:
	if (instr_word & (1u << 20))
	{
		goto autodecoder_node_0000000000b0a6c0;
	}

autodecoder_node_0000000000cdafc0:
	if (instr_word & (1u << 5))
	{
		goto autodecoder_node_0000000000b0a6c0;
	}
	goto autodecoder_node_0000000000cc8c60;

autodecoder_node_0000000000c5ea60:
	if (instr_word & (1u << 21))
	{
		goto autodecoder_node_0000000000cd16e0;
	}

autodecoder_node_0000000000cd3aa0:
	if (instr_word & (1u << 20))
	{
		goto autodecoder_node_0000000000b0a6c0;
	}

autodecoder_node_0000000000c7b2a0:
	if (instr_word & (1u << 5))
	{
		goto autodecoder_node_0000000000c6ad40;
	}

autodecoder_node_0000000000cc8c60:
	if (instr_word & (1u << 6))
	{
		goto autodecoder_node_0000000000cd1b00;
	}

autodecoder_node_0000000000ce29a0:
	if (instr_word & (1u << 22))
	{
		goto autodecoder_node_0000000000b0a6c0;
	}

autodecoder_node_0000000000bd3820:
	return &pkhbtInstruction;

autodecoder_node_0000000000cd1b00:
	if (instr_word & (1u << 22))
	{
		goto autodecoder_node_0000000000b0a6c0;
	}

autodecoder_node_0000000000bc75e0:
	return &pkhtbInstruction;

autodecoder_node_0000000000c6ad40:
	if (instr_word & (1u << 6))
	{
		goto autodecoder_node_0000000000b0a6c0;
	}

autodecoder_node_0000000000cc63a0:
	if (instr_word & (1u << 22))
	{
		goto autodecoder_node_0000000000b0a6c0;
	}

autodecoder_node_0000000000b484e0:
	return &selInstruction;

autodecoder_node_0000000000cd16e0:
	if ((instr_word & (1u << 20)) == 0)
	{
		goto autodecoder_node_0000000000c7cc00;
	}

autodecoder_node_0000000000cda8e0:
	if ((instr_word & (1u << 16)) == 0)
	{
		goto autodecoder_node_0000000000c7cc00;
	}

autodecoder_node_0000000000c77be0:
	if ((instr_word & (1u << 19)) == 0)
	{
		goto autodecoder_node_0000000000c7cc00;
	}

autodecoder_node_0000000000c6df80:
	if ((instr_word & (1u << 17)) == 0)
	{
		goto autodecoder_node_0000000000c7cc00;
	}

autodecoder_node_0000000000ccb7c0:
	if (instr_word & (1u << 18))
	{
		goto autodecoder_node_0000000000cbe5c0;
	}

autodecoder_node_0000000000c7cc00:
	if (instr_word & (1u << 5))
	{
		goto autodecoder_node_0000000000b0a6c0;
	}
	goto autodecoder_node_0000000000c67920;

autodecoder_node_0000000000cbe5c0:
	if (instr_word & (1u << 5))
	{
		goto autodecoder_node_0000000000cd40a0;
	}

autodecoder_node_0000000000c67920:
	if (instr_word & (1u << 22))
	{
		goto autodecoder_node_0000000000b27740;
	}

autodecoder_node_0000000000b2e120:
	return &ssatInstruction;

autodecoder_node_0000000000b27740:
	return &usatInstruction;

autodecoder_node_0000000000cd40a0:
	if (instr_word & (1u << 6))
	{
		goto autodecoder_node_0000000000b0a6c0;
	}

autodecoder_node_0000000000ce2fc0:
	if (instr_word & (1u << 22))
	{
		goto autodecoder_node_0000000000ba33e0;
	}

autodecoder_node_0000000000ba2440:
	return &rev16Instruction;

autodecoder_node_0000000000ba33e0:
	return &revshInstruction;

autodecoder_node_0000000000cd44c0:
	if (instr_word & (1u << 4))
	{
		goto autodecoder_node_0000000000ccbe00;
	}

autodecoder_node_0000000000b53780:
	if ((instr_word & (1u << 20)) == 0)
	{
		goto autodecoder_node_0000000000b0a240;
	}

autodecoder_node_0000000000bfd1a0:
	return &ldrInstruction;

autodecoder_node_0000000000ccbe00:
	if (instr_word & (1u << 7))
	{
		goto autodecoder_node_0000000000cd1520;
	}

autodecoder_node_0000000000c7b360:
	if (instr_word & (1u << 23))
	{
		goto autodecoder_node_0000000000c96a40;
	}

autodecoder_node_0000000000cd32a0:
	if (instr_word & (1u << 21))
	{
		goto autodecoder_node_0000000000b0a6c0;
	}

autodecoder_node_0000000000c72e20:
	if (instr_word & (1u << 20))
	{
		goto autodecoder_node_0000000000b9f000;
	}

autodecoder_node_0000000000cdf980:
	if (instr_word & (1u << 6))
	{
		goto autodecoder_node_0000000000b2e5c0;
	}

autodecoder_node_0000000000c67200:
	if (instr_word & (1u << 22))
	{
		goto autodecoder_node_0000000000b3fd20;
	}

autodecoder_node_0000000000b4a060:
	return &smuadInstruction;

autodecoder_node_0000000000b3fd20:
	return &smlaldInstruction;

autodecoder_node_0000000000b2e5c0:
	if (instr_word & (1u << 22))
	{
		goto autodecoder_node_0000000000b34a20;
	}

autodecoder_node_0000000000b4bf60:
	return &smusdInstruction;

autodecoder_node_0000000000b34a20:
	return &smlsldInstruction;

autodecoder_node_0000000000b9f000:
	if (instr_word & (1u << 6))
	{
		goto autodecoder_node_0000000000b0a6c0;
	}

autodecoder_node_0000000000ce3400:
	if ((instr_word & (1u << 22)) == 0)
	{
		goto autodecoder_node_0000000000b0a6c0;
	}

autodecoder_node_0000000000b37b80:
	return &smmulInstruction;

autodecoder_node_0000000000cd1520:
	if (instr_word & (1u << 23))
	{
		goto autodecoder_node_0000000000c96a40;
	}

autodecoder_node_0000000000c6c3e0:
	if (instr_word & (1u << 21))
	{
		goto autodecoder_node_0000000000b0a6c0;
	}

autodecoder_node_0000000000cdd240:
	if ((instr_word & (1u << 20)) == 0)
	{
		goto autodecoder_node_0000000000b0a6c0;
	}

autodecoder_node_0000000000c703e0:
	if ((instr_word & (1u << 6)) == 0)
	{
		goto autodecoder_node_0000000000b0a6c0;
	}

autodecoder_node_0000000000ce0aa0:
	if ((instr_word & (1u << 22)) == 0)
	{
		goto autodecoder_node_0000000000b0a6c0;
	}

autodecoder_node_0000000000b3b860:
	return &smmlsInstruction;

autodecoder_node_0000000000c96a40:
	if (instr_word & (1u << 21))
	{
		goto autodecoder_node_0000000000baff80;
	}

autodecoder_node_0000000000c62740:
	if (instr_word & (1u << 5))
	{
		goto autodecoder_node_0000000000b0a6c0;
	}

autodecoder_node_0000000000ccca80:
	if (instr_word & (1u << 6))
	{
		goto autodecoder_node_0000000000b0a6c0;
	}

autodecoder_node_0000000000cbe700:
	if (instr_word & (1u << 22))
	{
		goto autodecoder_node_0000000000b0a240;
	}
	goto autodecoder_node_0000000000b0a6c0;

autodecoder_node_0000000000baff80:
	if (instr_word & (1u << 5))
	{
		goto autodecoder_node_0000000000b0a6c0;
	}

autodecoder_node_0000000000ce7280:
	if (instr_word & (1u << 6))
	{
		goto autodecoder_node_0000000000b0a240;
	}
	goto autodecoder_node_0000000000b0a6c0;

autodecoder_node_0000000000cd1b20:
	if ((instr_word & (1u << 26)) == 0)
	{
		goto autodecoder_node_0000000000b0a6c0;
	}

autodecoder_node_0000000000c752c0:
	if (instr_word & (1u << 25))
	{
		goto autodecoder_node_0000000000c6a000;
	}

autodecoder_node_0000000000cd24c0:
	if ((instr_word & (1u << 24)) == 0)
	{
		goto autodecoder_node_0000000000c5e1e0;
	}

autodecoder_node_0000000000cd0720:
	if (instr_word & (1u << 4))
	{
		goto autodecoder_node_0000000000cdbc40;
	}

autodecoder_node_0000000000c703a0:
	if (instr_word & (1u << 7))
	{
		goto autodecoder_node_0000000000cdb2e0;
	}

autodecoder_node_0000000000ce74a0:
	if (instr_word & (1u << 23))
	{
		goto autodecoder_node_0000000000cdb2e0;
	}

autodecoder_node_0000000000c69d80:
	if (instr_word & (1u << 8))
	{
		goto autodecoder_node_0000000000cdb2e0;
	}

autodecoder_node_0000000000bcb600:
	if (instr_word & (1u << 11))
	{
		goto autodecoder_node_0000000000cdb2e0;
	}

autodecoder_node_0000000000ce3e20:
	if (instr_word & (1u << 10))
	{
		goto autodecoder_node_0000000000cdb2e0;
	}

autodecoder_node_0000000000ce9540:
	if (instr_word & (1u << 9))
	{
		goto autodecoder_node_0000000000cdb2e0;
	}

autodecoder_node_0000000000c61e00:
	if ((instr_word & (1u << 21)) == 0)
	{
		goto autodecoder_node_0000000000c7dee0;
	}

autodecoder_node_0000000000c6cc80:
	if ((instr_word & (1u << 20)) == 0)
	{
		goto autodecoder_node_0000000000b0a6c0;
	}

autodecoder_node_0000000000c74880:
	if ((instr_word & (1u << 16)) == 0)
	{
		goto autodecoder_node_0000000000b0a6c0;
	}

autodecoder_node_0000000000cd3ce0:
	if ((instr_word & (1u << 19)) == 0)
	{
		goto autodecoder_node_0000000000b0a6c0;
	}

autodecoder_node_0000000000ce7d80:
	if ((instr_word & (1u << 17)) == 0)
	{
		goto autodecoder_node_0000000000b0a6c0;
	}

autodecoder_node_0000000000c69e00:
	if ((instr_word & (1u << 18)) == 0)
	{
		goto autodecoder_node_0000000000b0a6c0;
	}

autodecoder_node_0000000000c66aa0:
	if (instr_word & (1u << 5))
	{
		goto autodecoder_node_0000000000b5e860;
	}

autodecoder_node_0000000000c79880:
	if ((instr_word & (1u << 6)) == 0)
	{
		goto autodecoder_node_0000000000b0a6c0;
	}

autodecoder_node_0000000000cc5c60:
	if ((instr_word & (1u << 22)) == 0)
	{
		goto autodecoder_node_0000000000b0a6c0;
	}

autodecoder_node_0000000000b0dac0:
	return &dsbInstruction;

autodecoder_node_0000000000b5e860:
	if ((instr_word & (1u << 6)) == 0)
	{
		goto autodecoder_node_0000000000b0a6c0;
	}

autodecoder_node_0000000000cde280:
	if ((instr_word & (1u << 22)) == 0)
	{
		goto autodecoder_node_0000000000b0a6c0;
	}

autodecoder_node_0000000000b0e4a0:
	return &isbInstruction;

autodecoder_node_0000000000cdbc40:
	if (instr_word & (1u << 7))
	{
		goto autodecoder_node_0000000000cdb2e0;
	}

autodecoder_node_0000000000ccfec0:
	if (instr_word & (1u << 23))
	{
		goto autodecoder_node_0000000000cdb2e0;
	}

autodecoder_node_0000000000ccf8c0:
	if (instr_word & (1u << 8))
	{
		goto autodecoder_node_0000000000cdb2e0;
	}

autodecoder_node_0000000000cdfc40:
	if (instr_word & (1u << 11))
	{
		goto autodecoder_node_0000000000cdb2e0;
	}

autodecoder_node_0000000000cd51c0:
	if (instr_word & (1u << 10))
	{
		goto autodecoder_node_0000000000cdb2e0;
	}

autodecoder_node_0000000000cc84c0:
	if (instr_word & (1u << 9))
	{
		goto autodecoder_node_0000000000cdb2e0;
	}

autodecoder_node_0000000000c7b200:
	if ((instr_word & (1u << 21)) == 0)
	{
		goto autodecoder_node_0000000000c7dee0;
	}

autodecoder_node_0000000000cdf160:
	if ((instr_word & (1u << 20)) == 0)
	{
		goto autodecoder_node_0000000000b0a6c0;
	}

autodecoder_node_0000000000c61f80:
	if ((instr_word & (1u << 16)) == 0)
	{
		goto autodecoder_node_0000000000b0a6c0;
	}

autodecoder_node_0000000000ce1100:
	if ((instr_word & (1u << 19)) == 0)
	{
		goto autodecoder_node_0000000000b0a6c0;
	}

autodecoder_node_0000000000ce0800:
	if ((instr_word & (1u << 17)) == 0)
	{
		goto autodecoder_node_0000000000b0a6c0;
	}

autodecoder_node_0000000000ccb1a0:
	if ((instr_word & (1u << 18)) == 0)
	{
		goto autodecoder_node_0000000000b0a6c0;
	}

autodecoder_node_0000000000c6c3a0:
	if (instr_word & (1u << 5))
	{
		goto autodecoder_node_0000000000b0a6c0;
	}

autodecoder_node_0000000000c86e80:
	if (instr_word & (1u << 6))
	{
		goto autodecoder_node_0000000000c7cfe0;
	}

autodecoder_node_0000000000c7e180:
	if ((instr_word & (1u << 3)) == 0)
	{
		goto autodecoder_node_0000000000b0a6c0;
	}

autodecoder_node_0000000000cccc60:
	if ((instr_word & (1u << 2)) == 0)
	{
		goto autodecoder_node_0000000000b0a6c0;
	}

autodecoder_node_0000000000cd4740:
	if ((instr_word & (1u << 1)) == 0)
	{
		goto autodecoder_node_0000000000b0a6c0;
	}

autodecoder_node_0000000000cc86e0:
	if ((instr_word & (1u << 0)) == 0)
	{
		goto autodecoder_node_0000000000b0a6c0;
	}

autodecoder_node_0000000000b4f5e0:
	if ((instr_word & (1u << 22)) == 0)
	{
		goto autodecoder_node_0000000000b0a6c0;
	}

autodecoder_node_0000000000b0fee0:
	return &clrexInstruction;

autodecoder_node_0000000000c7cfe0:
	if ((instr_word & (1u << 22)) == 0)
	{
		goto autodecoder_node_0000000000b0a6c0;
	}

autodecoder_node_0000000000b0cd20:
	return &dmbInstruction;

autodecoder_node_0000000000c6a000:
	if (instr_word & (1u << 24))
	{
		goto autodecoder_node_0000000000cd04c0;
	}

autodecoder_node_0000000000ce8ba0:
	if (instr_word & (1u << 4))
	{
		goto autodecoder_node_0000000000b0a6c0;
	}

autodecoder_node_0000000000c5e1e0:
	if (instr_word & (1u << 21))
	{
		goto autodecoder_node_0000000000b0a6c0;
	}

autodecoder_node_0000000000cc8ba0:
	if ((instr_word & (1u << 20)) == 0)
	{
		goto autodecoder_node_0000000000b0a6c0;
	}

autodecoder_node_0000000000ce8f20:
	if ((instr_word & (1u << 22)) == 0)
	{
		goto autodecoder_node_0000000000b0a6c0;
	}

autodecoder_node_0000000000b19bc0:
	return &pliInstruction;

autodecoder_node_0000000000cd04c0:
	if (instr_word & (1u << 4))
	{
		goto autodecoder_node_0000000000b0a6c0;
	}

autodecoder_node_0000000000cdb2e0:
	if (instr_word & (1u << 21))
	{
		goto autodecoder_node_0000000000b0a6c0;
	}

autodecoder_node_0000000000c7dee0:
	if ((instr_word & (1u << 20)) == 0)
	{
		goto autodecoder_node_0000000000b0a6c0;
	}

autodecoder_node_0000000000b16080:
	return &pldInstruction;

autodecoder_node_0000000000c79360:
	if (instr_word & (1u << 15))
	{
		goto autodecoder_node_0000000000c6b480;
	}

autodecoder_node_0000000000ccbbe0:
	if (instr_word & (1u << 12))
	{
		goto autodecoder_node_0000000000c6ec40;
	}

autodecoder_node_0000000000c7c540:
	if (instr_word & (1u << 13))
	{
		goto autodecoder_node_0000000000c6ec40;
	}

autodecoder_node_0000000000c6dda0:
	if (instr_word & (1u << 14))
	{
		goto autodecoder_node_0000000000c6ec40;
	}

autodecoder_node_0000000000c7b860:
	if ((instr_word & (1u << 28)) == 0)
	{
		goto autodecoder_node_0000000000cd4de0;
	}

autodecoder_node_0000000000ce2e80:
	if (instr_word & (1u << 26))
	{
		goto autodecoder_node_0000000000b0a6c0;
	}

autodecoder_node_0000000000c5dc40:
	if (instr_word & (1u << 25))
	{
		goto autodecoder_node_0000000000b13760;
	}

autodecoder_node_0000000000ce07a0:
	if (instr_word & (1u << 4))
	{
		goto autodecoder_node_0000000000c85740;
	}

autodecoder_node_0000000000ce0b40:
	if (instr_word & (1u << 7))
	{
		goto autodecoder_node_0000000000b0a6c0;
	}

autodecoder_node_0000000000cc56e0:
	if (instr_word & (1u << 8))
	{
		goto autodecoder_node_0000000000cced80;
	}

autodecoder_node_0000000000c7bb40:
	if ((instr_word & (1u << 11)) == 0)
	{
		goto autodecoder_node_0000000000b0a6c0;
	}

autodecoder_node_0000000000c6ddc0:
	if (instr_word & (1u << 10))
	{
		goto autodecoder_node_0000000000b0a6c0;
	}

autodecoder_node_0000000000cdb120:
	if ((instr_word & (1u << 9)) == 0)
	{
		goto autodecoder_node_0000000000b0a6c0;
	}

autodecoder_node_0000000000ce66e0:
	if ((instr_word & (1u << 20)) == 0)
	{
		goto autodecoder_node_0000000000b0a6c0;
	}

autodecoder_node_0000000000cc68a0:
	if (instr_word & (1u << 5))
	{
		goto autodecoder_node_0000000000b0a6c0;
	}

autodecoder_node_0000000000b99c20:
	if (instr_word & (1u << 6))
	{
		goto autodecoder_node_0000000000b0a6c0;
	}

autodecoder_node_0000000000ce8340:
	if (instr_word & (1u << 3))
	{
		goto autodecoder_node_0000000000b0a6c0;
	}

autodecoder_node_0000000000bb87a0:
	if (instr_word & (1u << 2))
	{
		goto autodecoder_node_0000000000b0a6c0;
	}

autodecoder_node_0000000000bb1040:
	if (instr_word & (1u << 1))
	{
		goto autodecoder_node_0000000000b0a6c0;
	}

autodecoder_node_0000000000c6fda0:
	if (instr_word & (1u << 0))
	{
		goto autodecoder_node_0000000000b0a6c0;
	}

autodecoder_node_0000000000b568a0:
	if (instr_word & (1u << 22))
	{
		goto autodecoder_node_0000000000b0a6c0;
	}

autodecoder_node_0000000000b88480:
	return &rfeInstruction;

autodecoder_node_0000000000c85740:
	if (instr_word & (1u << 7))
	{
		goto autodecoder_node_0000000000b0a6c0;
	}

autodecoder_node_0000000000cd09e0:
	if ((instr_word & (1u << 8)) == 0)
	{
		goto autodecoder_node_0000000000b0a6c0;
	}

autodecoder_node_0000000000cced80:
	if (instr_word & (1u << 11))
	{
		goto autodecoder_node_0000000000b0a6c0;
	}

autodecoder_node_0000000000ccdc00:
	if ((instr_word & (1u << 10)) == 0)
	{
		goto autodecoder_node_0000000000b0a6c0;
	}

autodecoder_node_0000000000cde0a0:
	if (instr_word & (1u << 9))
	{
		goto autodecoder_node_0000000000b0a6c0;
	}

autodecoder_node_0000000000c6c7a0:
	if (instr_word & (1u << 20))
	{
		goto autodecoder_node_0000000000b0a6c0;
	}

autodecoder_node_0000000000ce7c60:
	if ((instr_word & (1u << 16)) == 0)
	{
		goto autodecoder_node_0000000000b0a6c0;
	}

autodecoder_node_0000000000cdb400:
	if ((instr_word & (1u << 19)) == 0)
	{
		goto autodecoder_node_0000000000b0a6c0;
	}

autodecoder_node_0000000000c96c60:
	if (instr_word & (1u << 17))
	{
		goto autodecoder_node_0000000000b0a6c0;
	}

autodecoder_node_0000000000bac0c0:
	if ((instr_word & (1u << 18)) == 0)
	{
		goto autodecoder_node_0000000000b0a6c0;
	}

autodecoder_node_0000000000c620c0:
	if (instr_word & (1u << 5))
	{
		goto autodecoder_node_0000000000b0a6c0;
	}

autodecoder_node_0000000000c78e40:
	if (instr_word & (1u << 6))
	{
		goto autodecoder_node_0000000000b0a6c0;
	}

autodecoder_node_0000000000c6e840:
	if ((instr_word & (1u << 22)) == 0)
	{
		goto autodecoder_node_0000000000b0a6c0;
	}

autodecoder_node_0000000000b8c720:
	return &srsInstruction;

autodecoder_node_0000000000c6ec40:
	if (instr_word & (1u << 28))
	{
		goto autodecoder_node_0000000000ccc400;
	}

autodecoder_node_0000000000cd4de0:
	if (instr_word & (1u << 26))
	{
		goto autodecoder_node_0000000000ce0820;
	}

autodecoder_node_0000000000c65c20:
	if (instr_word & (1u << 25))
	{
		goto autodecoder_node_0000000000ca43e0;
	}

autodecoder_node_0000000000c8a780:
	if ((instr_word & (1u << 20)) == 0)
	{
		goto autodecoder_node_0000000000cdb560;
	}

autodecoder_node_0000000000c716e0:
	if (instr_word & (1u << 22))
	{
		goto autodecoder_node_0000000000c6b500;
	}
	goto autodecoder_node_0000000000b0a240;

autodecoder_node_0000000000c6b480:
	if (instr_word & (1u << 28))
	{
		goto autodecoder_node_0000000000ccc400;
	}

autodecoder_node_0000000000c779a0:
	if (instr_word & (1u << 26))
	{
		goto autodecoder_node_0000000000ce0820;
	}

autodecoder_node_0000000000ce6280:
	if (instr_word & (1u << 25))
	{
		goto autodecoder_node_0000000000ca43e0;
	}

autodecoder_node_0000000000c71a20:
	if (instr_word & (1u << 24))
	{
		goto autodecoder_node_0000000000c67520;
	}

autodecoder_node_0000000000c6ffa0:
	if ((instr_word & (1u << 23)) == 0)
	{
		goto autodecoder_node_0000000000c67520;
	}

autodecoder_node_0000000000c77d60:
	if ((instr_word & (1u << 21)) == 0)
	{
		goto autodecoder_node_0000000000c67520;
	}

autodecoder_node_0000000000c94a40:
	if ((instr_word & (1u << 20)) == 0)
	{
		goto autodecoder_node_0000000000cdb560;
	}

autodecoder_node_0000000000c744e0:
	if ((instr_word & (1u << 16)) == 0)
	{
		goto autodecoder_node_0000000000c6b500;
	}

autodecoder_node_0000000000c66e40:
	if ((instr_word & (1u << 19)) == 0)
	{
		goto autodecoder_node_0000000000c6b500;
	}

autodecoder_node_0000000000c9f9c0:
	if (instr_word & (1u << 17))
	{
		goto autodecoder_node_0000000000c6b500;
	}

autodecoder_node_0000000000cd52c0:
	if ((instr_word & (1u << 18)) == 0)
	{
		goto autodecoder_node_0000000000c6b500;
	}

autodecoder_node_0000000000ccee80:
	if (instr_word & (1u << 22))
	{
		goto autodecoder_node_0000000000c6b500;
	}

autodecoder_node_0000000000c5be40:
	return &popLdmInstruction;

autodecoder_node_0000000000c67520:
	if (instr_word & (1u << 20))
	{
		goto autodecoder_node_0000000000c6b500;
	}

autodecoder_node_0000000000cdb560:
	if (instr_word & (1u << 22))
	{
		goto autodecoder_node_0000000000b8c840;
	}

autodecoder_node_0000000000b0a240:
	return 0;

autodecoder_node_0000000000b8c840:
	return &stmInstruction;

autodecoder_node_0000000000c6b500:
	return &ldmInstruction;

autodecoder_node_0000000000ca43e0:
	return &bInstruction;

autodecoder_node_0000000000ce0820:
	if ((instr_word & (1u << 25)) == 0)
	{
		goto autodecoder_node_0000000000b0a6c0;
	}

autodecoder_node_0000000000c73700:
	if (instr_word & (1u << 24))
	{
		goto autodecoder_node_0000000000cc60e0;
	}

autodecoder_node_0000000000cc6300:
	if ((instr_word & (1u << 4)) == 0)
	{
		goto autodecoder_node_0000000000b0a6c0;
	}

autodecoder_node_0000000000ccad00:
	if (instr_word & (1u << 20))
	{
		goto autodecoder_node_0000000000ce72a0;
	}

autodecoder_node_0000000000c99320:
	return &mcrInstruction;

autodecoder_node_0000000000ce72a0:
	return &mrcInstruction;

autodecoder_node_0000000000cc60e0:
	return &svcInstruction;

autodecoder_node_0000000000ccc400:
	if (instr_word & (1u << 26))
	{
		goto autodecoder_node_0000000000b0a6c0;
	}

autodecoder_node_0000000000ce5ce0:
	if ((instr_word & (1u << 25)) == 0)
	{
		goto autodecoder_node_0000000000b0a6c0;
	}

autodecoder_node_0000000000b13760:
	return &blxInstruction;

autodecoder_node_0000000000b0a6c0:
	DIE_NOW(0, "autoDecoder: undefined instruction");

