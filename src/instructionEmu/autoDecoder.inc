autodecoder_node_00000000019f8580:
	if ((instr_word & (1u << 29)) == 0)
	{
		goto autodecoder_node_00000000019e2300;
	}

autodecoder_node_00000000019e9ce0:
	if ((instr_word & (1u << 30)) == 0)
	{
		goto autodecoder_node_00000000019e2300;
	}

autodecoder_node_00000000017f3a80:
	if (instr_word & (1u << 31))
	{
		goto autodecoder_node_00000000019ec200;
	}

autodecoder_node_00000000019e2300:
	if (instr_word & (1u << 26))
	{
		goto autodecoder_node_00000000019e0640;
	}

autodecoder_node_00000000019c39c0:
	if (instr_word & (1u << 27))
	{
		goto autodecoder_node_00000000019f9f60;
	}

autodecoder_node_00000000019d0a20:
	if (instr_word & (1u << 12))
	{
		goto autodecoder_node_00000000019c88e0;
	}

autodecoder_node_00000000019a8240:
	if (instr_word & (1u << 13))
	{
		goto autodecoder_node_00000000019c2a60;
	}

autodecoder_node_00000000019f8880:
	if (instr_word & (1u << 14))
	{
		goto autodecoder_node_00000000019c2a60;
	}

autodecoder_node_00000000019d0640:
	if (instr_word & (1u << 15))
	{
		goto autodecoder_node_00000000019c2a60;
	}

autodecoder_node_00000000019dd340:
	if (instr_word & (1u << 25))
	{
		goto autodecoder_node_00000000019e3680;
	}

autodecoder_node_00000000019ec680:
	if ((instr_word & (1u << 24)) == 0)
	{
		goto autodecoder_node_000000000180b2c0;
	}

autodecoder_node_00000000019def80:
	if (instr_word & (1u << 4))
	{
		goto autodecoder_node_00000000019cfdc0;
	}
	goto autodecoder_node_00000000019e68e0;

autodecoder_node_00000000019c88e0:
	if ((instr_word & (1u << 13)) == 0)
	{
		goto autodecoder_node_00000000019c2a60;
	}

autodecoder_node_00000000019e3980:
	if ((instr_word & (1u << 14)) == 0)
	{
		goto autodecoder_node_00000000019c2a60;
	}

autodecoder_node_00000000019dbf20:
	if (instr_word & (1u << 15))
	{
		goto autodecoder_node_00000000019d67c0;
	}

autodecoder_node_00000000019c2a60:
	if (instr_word & (1u << 25))
	{
		goto autodecoder_node_00000000019e3680;
	}

autodecoder_node_00000000019bfb80:
	if ((instr_word & (1u << 24)) == 0)
	{
		goto autodecoder_node_000000000180b2c0;
	}

autodecoder_node_00000000019e70c0:
	if ((instr_word & (1u << 4)) == 0)
	{
		goto autodecoder_node_00000000019c6320;
	}

autodecoder_node_00000000019cfdc0:
	if (instr_word & (1u << 7))
	{
		goto autodecoder_node_00000000019ef120;
	}

autodecoder_node_00000000019c76a0:
	if (instr_word & (1u << 23))
	{
		goto autodecoder_node_00000000019c3120;
	}

autodecoder_node_00000000019e5060:
	if (instr_word & (1u << 8))
	{
		goto autodecoder_node_00000000019cb320;
	}
	goto autodecoder_node_00000000019f9760;

autodecoder_node_00000000019d67c0:
	if (instr_word & (1u << 25))
	{
		goto autodecoder_node_00000000019d3820;
	}

autodecoder_node_00000000019d4700:
	if ((instr_word & (1u << 24)) == 0)
	{
		goto autodecoder_node_00000000019c1080;
	}

autodecoder_node_00000000019e1d20:
	if ((instr_word & (1u << 4)) == 0)
	{
		goto autodecoder_node_00000000019c8420;
	}

autodecoder_node_00000000019fa120:
	if (instr_word & (1u << 7))
	{
		goto autodecoder_node_00000000019ef120;
	}

autodecoder_node_00000000019d7a20:
	if (instr_word & (1u << 23))
	{
		goto autodecoder_node_00000000019e82e0;
	}

autodecoder_node_00000000019c4060:
	if (instr_word & (1u << 8))
	{
		goto autodecoder_node_0000000001915dc0;
	}

autodecoder_node_00000000019f9760:
	if (instr_word & (1u << 11))
	{
		goto autodecoder_node_00000000019cb320;
	}

autodecoder_node_00000000019da440:
	if (instr_word & (1u << 10))
	{
		goto autodecoder_node_00000000019cb320;
	}

autodecoder_node_00000000019fb240:
	if (instr_word & (1u << 9))
	{
		goto autodecoder_node_00000000019cb320;
	}

autodecoder_node_00000000019c26c0:
	if ((instr_word & (1u << 21)) == 0)
	{
		goto autodecoder_node_00000000019d5320;
	}

autodecoder_node_00000000019ceb20:
	if (instr_word & (1u << 20))
	{
		goto autodecoder_node_00000000017cc240;
	}

autodecoder_node_00000000019d9d20:
	if (instr_word & (1u << 5))
	{
		goto autodecoder_node_00000000019ec420;
	}
	goto autodecoder_node_00000000019cb960;

autodecoder_node_0000000001915dc0:
	if ((instr_word & (1u << 11)) == 0)
	{
		goto autodecoder_node_00000000019cb320;
	}

autodecoder_node_0000000001935dc0:
	if ((instr_word & (1u << 10)) == 0)
	{
		goto autodecoder_node_00000000019cb320;
	}

autodecoder_node_00000000019de180:
	if (instr_word & (1u << 9))
	{
		goto autodecoder_node_00000000019d9960;
	}

autodecoder_node_00000000019cb320:
	if ((instr_word & (1u << 21)) == 0)
	{
		goto autodecoder_node_00000000017cc240;
	}

autodecoder_node_00000000019d8900:
	if (instr_word & (1u << 20))
	{
		goto autodecoder_node_00000000017cc240;
	}
	goto autodecoder_node_00000000019c3760;

autodecoder_node_00000000019d9960:
	if ((instr_word & (1u << 21)) == 0)
	{
		goto autodecoder_node_00000000017cc240;
	}

autodecoder_node_00000000019e6120:
	if (instr_word & (1u << 20))
	{
		goto autodecoder_node_00000000017cc240;
	}

autodecoder_node_00000000019d1b40:
	if ((instr_word & (1u << 16)) == 0)
	{
		goto autodecoder_node_00000000019c3760;
	}

autodecoder_node_00000000019c1580:
	if ((instr_word & (1u << 19)) == 0)
	{
		goto autodecoder_node_00000000019c3760;
	}

autodecoder_node_00000000019c8260:
	if ((instr_word & (1u << 17)) == 0)
	{
		goto autodecoder_node_00000000019c3760;
	}

autodecoder_node_00000000019ef220:
	if (instr_word & (1u << 18))
	{
		goto autodecoder_node_00000000019e6f00;
	}

autodecoder_node_00000000019c3760:
	if ((instr_word & (1u << 5)) == 0)
	{
		goto autodecoder_node_00000000017cc240;
	}

autodecoder_node_00000000019ec420:
	if (instr_word & (1u << 6))
	{
		goto autodecoder_node_00000000019e32c0;
	}
	goto autodecoder_node_00000000017cc240;

autodecoder_node_00000000019e6f00:
	if ((instr_word & (1u << 5)) == 0)
	{
		goto autodecoder_node_00000000019e1760;
	}

autodecoder_node_00000000019ea160:
	if ((instr_word & (1u << 6)) == 0)
	{
		goto autodecoder_node_00000000019cdb40;
	}

autodecoder_node_00000000019e32c0:
	if (instr_word & (1u << 22))
	{
		goto autodecoder_node_0000000001806aa0;
	}
	goto autodecoder_node_00000000017cc240;

autodecoder_node_00000000019ec200:
	if (instr_word & (1u << 26))
	{
		goto autodecoder_node_00000000019faac0;
	}

autodecoder_node_00000000019e0a80:
	if (instr_word & (1u << 28))
	{
		goto autodecoder_node_00000000019d3ce0;
	}

autodecoder_node_00000000019f3720:
	if (instr_word & (1u << 27))
	{
		goto autodecoder_node_00000000019f9f60;
	}

autodecoder_node_00000000019cf140:
	if (instr_word & (1u << 12))
	{
		goto autodecoder_node_00000000019d1ac0;
	}

autodecoder_node_00000000019c7a20:
	if (instr_word & (1u << 13))
	{
		goto autodecoder_node_00000000019e5f00;
	}

autodecoder_node_00000000019d5800:
	if (instr_word & (1u << 14))
	{
		goto autodecoder_node_00000000019e5f00;
	}

autodecoder_node_00000000019e4c40:
	if (instr_word & (1u << 15))
	{
		goto autodecoder_node_00000000019e5f00;
	}

autodecoder_node_0000000001860600:
	if (instr_word & (1u << 25))
	{
		goto autodecoder_node_00000000019e3680;
	}

autodecoder_node_00000000019f43a0:
	if ((instr_word & (1u << 24)) == 0)
	{
		goto autodecoder_node_000000000180b2c0;
	}

autodecoder_node_00000000019ea5a0:
	if (instr_word & (1u << 4))
	{
		goto autodecoder_node_00000000019eab40;
	}

autodecoder_node_00000000019e68e0:
	if ((instr_word & (1u << 7)) == 0)
	{
		goto autodecoder_node_00000000019fa960;
	}

autodecoder_node_00000000019eebc0:
	if (instr_word & (1u << 23))
	{
		goto autodecoder_node_00000000019c3120;
	}

autodecoder_node_00000000019f5d80:
	if ((instr_word & (1u << 21)) == 0)
	{
		goto autodecoder_node_00000000019caba0;
	}

autodecoder_node_00000000019d47c0:
	if (instr_word & (1u << 20))
	{
		goto autodecoder_node_00000000017cc240;
	}

autodecoder_node_00000000019e4be0:
	if (instr_word & (1u << 5))
	{
		goto autodecoder_node_00000000019e1060;
	}

autodecoder_node_00000000018fabe0:
	if (instr_word & (1u << 6))
	{
		goto autodecoder_node_00000000019f8e80;
	}

autodecoder_node_00000000019d43c0:
	if ((instr_word & (1u << 22)) == 0)
	{
		goto autodecoder_node_0000000001817500;
	}

autodecoder_node_000000000181d580:
	return &smulbbInstruction;

autodecoder_node_00000000019f8e80:
	if ((instr_word & (1u << 22)) == 0)
	{
		goto autodecoder_node_00000000018185c0;
	}

autodecoder_node_000000000181f7e0:
	return &smulbtInstruction;

autodecoder_node_00000000019e1060:
	if (instr_word & (1u << 6))
	{
		goto autodecoder_node_000000000187c0a0;
	}

autodecoder_node_00000000019ee400:
	if (instr_word & (1u << 22))
	{
		goto autodecoder_node_000000000181e880;
	}

autodecoder_node_00000000018218c0:
	return &smulwbInstruction;

autodecoder_node_000000000181e880:
	return &smultbInstruction;

autodecoder_node_000000000187c0a0:
	if (instr_word & (1u << 22))
	{
		goto autodecoder_node_00000000018208a0;
	}

autodecoder_node_0000000001822720:
	return &smulwtInstruction;

autodecoder_node_00000000018208a0:
	return &smulttInstruction;

autodecoder_node_00000000019d1ac0:
	if ((instr_word & (1u << 13)) == 0)
	{
		goto autodecoder_node_00000000019e5f00;
	}

autodecoder_node_00000000019d1b60:
	if ((instr_word & (1u << 14)) == 0)
	{
		goto autodecoder_node_00000000019e5f00;
	}

autodecoder_node_00000000019e5dc0:
	if (instr_word & (1u << 15))
	{
		goto autodecoder_node_00000000019e0120;
	}

autodecoder_node_00000000019e5f00:
	if (instr_word & (1u << 25))
	{
		goto autodecoder_node_00000000019e3680;
	}

autodecoder_node_00000000019dc560:
	if (instr_word & (1u << 24))
	{
		goto autodecoder_node_00000000019c24c0;
	}

autodecoder_node_000000000180b2c0:
	if ((instr_word & (1u << 4)) == 0)
	{
		goto autodecoder_node_00000000017cc240;
	}

autodecoder_node_00000000019ca800:
	if (instr_word & (1u << 7))
	{
		goto autodecoder_node_00000000019de000;
	}
	goto autodecoder_node_00000000017cc240;

autodecoder_node_00000000019c24c0:
	if (instr_word & (1u << 4))
	{
		goto autodecoder_node_00000000019eab40;
	}

autodecoder_node_00000000019c6320:
	if (instr_word & (1u << 7))
	{
		goto autodecoder_node_00000000019e2520;
	}

autodecoder_node_00000000019fa960:
	if (instr_word & (1u << 23))
	{
		goto autodecoder_node_00000000019c3120;
	}

autodecoder_node_00000000019fb2a0:
	if (instr_word & (1u << 8))
	{
		goto autodecoder_node_00000000017cc240;
	}

autodecoder_node_00000000019ca6a0:
	if (instr_word & (1u << 11))
	{
		goto autodecoder_node_00000000017cc240;
	}

autodecoder_node_00000000019c8720:
	if (instr_word & (1u << 10))
	{
		goto autodecoder_node_00000000017cc240;
	}

autodecoder_node_00000000019f4020:
	if (instr_word & (1u << 9))
	{
		goto autodecoder_node_00000000017cc240;
	}

autodecoder_node_00000000019dcd80:
	if (instr_word & (1u << 21))
	{
		goto autodecoder_node_00000000017cc240;
	}
	goto autodecoder_node_00000000018c7280;

autodecoder_node_00000000019e2520:
	if (instr_word & (1u << 23))
	{
		goto autodecoder_node_00000000019c3120;
	}
	goto autodecoder_node_00000000019c2820;

autodecoder_node_00000000019eab40:
	if (instr_word & (1u << 7))
	{
		goto autodecoder_node_00000000019ef120;
	}

autodecoder_node_00000000019d3d60:
	if (instr_word & (1u << 23))
	{
		goto autodecoder_node_00000000019c3120;
	}

autodecoder_node_00000000018605c0:
	if (instr_word & (1u << 8))
	{
		goto autodecoder_node_00000000019de5c0;
	}
	goto autodecoder_node_00000000019ef2c0;

autodecoder_node_00000000019e3680:
	if ((instr_word & (1u << 24)) == 0)
	{
		goto autodecoder_node_00000000017cc240;
	}

autodecoder_node_000000000180f100:
	if (instr_word & (1u << 4))
	{
		goto autodecoder_node_00000000019c8d20;
	}

autodecoder_node_00000000019e42a0:
	if (instr_word & (1u << 7))
	{
		goto autodecoder_node_00000000019c8d20;
	}

autodecoder_node_00000000019c81c0:
	if (instr_word & (1u << 23))
	{
		goto autodecoder_node_00000000019c3120;
	}

autodecoder_node_00000000019d6a80:
	if ((instr_word & (1u << 21)) == 0)
	{
		goto autodecoder_node_00000000019f9de0;
	}

autodecoder_node_00000000019ee280:
	if (instr_word & (1u << 20))
	{
		goto autodecoder_node_00000000017cc240;
	}

autodecoder_node_00000000019f8360:
	if (instr_word & (1u << 16))
	{
		goto autodecoder_node_00000000017cc240;
	}

autodecoder_node_00000000019dcb00:
	if (instr_word & (1u << 19))
	{
		goto autodecoder_node_00000000017cc240;
	}

autodecoder_node_00000000019e2d20:
	if (instr_word & (1u << 17))
	{
		goto autodecoder_node_00000000017cc240;
	}

autodecoder_node_00000000018f5720:
	if (instr_word & (1u << 18))
	{
		goto autodecoder_node_00000000017cc240;
	}

autodecoder_node_00000000019e4da0:
	if (instr_word & (1u << 5))
	{
		goto autodecoder_node_00000000017cc240;
	}

autodecoder_node_00000000019dc7c0:
	if (instr_word & (1u << 6))
	{
		goto autodecoder_node_00000000017cc240;
	}

autodecoder_node_00000000019d7460:
	if (instr_word & (1u << 3))
	{
		goto autodecoder_node_00000000017cc240;
	}

autodecoder_node_00000000019d7d00:
	if (instr_word & (1u << 0))
	{
		goto autodecoder_node_00000000017cc240;
	}

autodecoder_node_00000000019c9a00:
	if (instr_word & (1u << 2))
	{
		goto autodecoder_node_00000000017cc240;
	}

autodecoder_node_00000000019f7a00:
	if (instr_word & (1u << 1))
	{
		goto autodecoder_node_00000000017cc240;
	}

autodecoder_node_00000000019c4380:
	if (instr_word & (1u << 22))
	{
		goto autodecoder_node_00000000017cc240;
	}
	goto autodecoder_node_0000000001899400;

autodecoder_node_00000000019c8d20:
	if (instr_word & (1u << 23))
	{
		goto autodecoder_node_00000000019c3120;
	}

autodecoder_node_00000000019ddec0:
	if (instr_word & (1u << 21))
	{
		goto autodecoder_node_00000000017cc240;
	}
	goto autodecoder_node_00000000019f9de0;

autodecoder_node_00000000019c3120:
	if ((instr_word & (1u << 21)) == 0)
	{
		goto autodecoder_node_00000000017cc240;
	}

autodecoder_node_00000000019e6100:
	if (instr_word & (1u << 16))
	{
		goto autodecoder_node_00000000019f77e0;
	}

autodecoder_node_00000000019e80a0:
	if (instr_word & (1u << 19))
	{
		goto autodecoder_node_00000000019f77e0;
	}

autodecoder_node_00000000019edec0:
	if (instr_word & (1u << 17))
	{
		goto autodecoder_node_00000000019f77e0;
	}

autodecoder_node_000000000193ca60:
	if (instr_word & (1u << 18))
	{
		goto autodecoder_node_00000000019f77e0;
	}
	goto autodecoder_node_00000000017cc240;

autodecoder_node_00000000019e0120:
	if (instr_word & (1u << 25))
	{
		goto autodecoder_node_00000000019d3820;
	}

autodecoder_node_00000000019d0380:
	if (instr_word & (1u << 24))
	{
		goto autodecoder_node_00000000019cdd20;
	}

autodecoder_node_00000000019c1080:
	if ((instr_word & (1u << 4)) == 0)
	{
		goto autodecoder_node_00000000019c3d60;
	}

autodecoder_node_00000000019ca620:
	if ((instr_word & (1u << 7)) == 0)
	{
		goto autodecoder_node_00000000019c3d60;
	}

autodecoder_node_00000000019de000:
	if (instr_word & (1u << 23))
	{
		goto autodecoder_node_00000000019ee4c0;
	}

autodecoder_node_00000000019e4ac0:
	if (instr_word & (1u << 8))
	{
		goto autodecoder_node_0000000001892aa0;
	}

autodecoder_node_00000000017f9ac0:
	if (instr_word & (1u << 11))
	{
		goto autodecoder_node_0000000001892aa0;
	}

autodecoder_node_00000000019d3c00:
	if (instr_word & (1u << 10))
	{
		goto autodecoder_node_0000000001892aa0;
	}

autodecoder_node_00000000019f80a0:
	if (instr_word & (1u << 9))
	{
		goto autodecoder_node_0000000001892aa0;
	}

autodecoder_node_00000000019dbd00:
	if (instr_word & (1u << 21))
	{
		goto autodecoder_node_00000000019ea4c0;
	}

autodecoder_node_00000000019cca20:
	if (instr_word & (1u << 20))
	{
		goto autodecoder_node_00000000017cc240;
	}

autodecoder_node_00000000019e8da0:
	if (instr_word & (1u << 5))
	{
		goto autodecoder_node_00000000017cc240;
	}

autodecoder_node_00000000019e8fe0:
	if (instr_word & (1u << 6))
	{
		goto autodecoder_node_00000000017cc240;
	}
	goto autodecoder_node_00000000019bb6c0;

autodecoder_node_0000000001892aa0:
	if (instr_word & (1u << 21))
	{
		goto autodecoder_node_00000000019eeec0;
	}

autodecoder_node_00000000019f4e60:
	if (instr_word & (1u << 20))
	{
		goto autodecoder_node_00000000019cd940;
	}

autodecoder_node_00000000019e6380:
	if (instr_word & (1u << 5))
	{
		goto autodecoder_node_00000000019f77e0;
	}

autodecoder_node_00000000017de580:
	if (instr_word & (1u << 6))
	{
		goto autodecoder_node_00000000019f77e0;
	}

autodecoder_node_00000000019bb6c0:
	if (instr_word & (1u << 22))
	{
		goto autodecoder_node_00000000017cc6c0;
	}
	goto autodecoder_node_00000000017cc240;

autodecoder_node_00000000019ee4c0:
	if (instr_word & (1u << 8))
	{
		goto autodecoder_node_00000000019c18e0;
	}

autodecoder_node_00000000017eaf60:
	if (instr_word & (1u << 11))
	{
		goto autodecoder_node_00000000019c18e0;
	}

autodecoder_node_00000000019e8740:
	if (instr_word & (1u << 10))
	{
		goto autodecoder_node_00000000019c18e0;
	}

autodecoder_node_00000000019ed180:
	if (instr_word & (1u << 9))
	{
		goto autodecoder_node_00000000019c18e0;
	}

autodecoder_node_000000000194e900:
	if ((instr_word & (1u << 21)) == 0)
	{
		goto autodecoder_node_00000000017cc240;
	}

autodecoder_node_00000000019ea4c0:
	if (instr_word & (1u << 20))
	{
		goto autodecoder_node_00000000018fa300;
	}

autodecoder_node_00000000019f5180:
	if ((instr_word & (1u << 5)) == 0)
	{
		goto autodecoder_node_00000000017cc240;
	}

autodecoder_node_00000000019de500:
	if (instr_word & (1u << 6))
	{
		goto autodecoder_node_00000000017cc240;
	}

autodecoder_node_00000000019c3ec0:
	if (instr_word & (1u << 22))
	{
		goto autodecoder_node_00000000017ec440;
	}
	goto autodecoder_node_00000000017cc240;

autodecoder_node_00000000018fa300:
	if ((instr_word & (1u << 5)) == 0)
	{
		goto autodecoder_node_00000000017cc240;
	}

autodecoder_node_00000000019f47a0:
	if (instr_word & (1u << 6))
	{
		goto autodecoder_node_00000000017cc240;
	}
	goto autodecoder_node_00000000017ed320;

autodecoder_node_00000000019c18e0:
	if (instr_word & (1u << 21))
	{
		goto autodecoder_node_00000000019eeec0;
	}

autodecoder_node_00000000019cd940:
	if (instr_word & (1u << 5))
	{
		goto autodecoder_node_00000000019f77e0;
	}
	goto autodecoder_node_0000000001821180;

autodecoder_node_00000000019eeec0:
	if (instr_word & (1u << 20))
	{
		goto autodecoder_node_00000000019d6ce0;
	}

autodecoder_node_00000000019e3fc0:
	if ((instr_word & (1u << 5)) == 0)
	{
		goto autodecoder_node_0000000001821180;
	}

autodecoder_node_00000000019ec0c0:
	if (instr_word & (1u << 6))
	{
		goto autodecoder_node_00000000019f77e0;
	}

autodecoder_node_00000000019d2400:
	if ((instr_word & (1u << 22)) == 0)
	{
		goto autodecoder_node_00000000017cc6c0;
	}

autodecoder_node_00000000017ec440:
	return &strhtInstruction;

autodecoder_node_00000000019d6ce0:
	if (instr_word & (1u << 5))
	{
		goto autodecoder_node_00000000019f69e0;
	}

autodecoder_node_0000000001821180:
	if (instr_word & (1u << 6))
	{
		goto autodecoder_node_00000000019f77e0;
	}
	goto autodecoder_node_00000000017cc240;

autodecoder_node_00000000019f69e0:
	if (instr_word & (1u << 6))
	{
		goto autodecoder_node_00000000019f77e0;
	}

autodecoder_node_00000000017ed320:
	return &ldrhtInstruction;

autodecoder_node_00000000019cdd20:
	if (instr_word & (1u << 4))
	{
		goto autodecoder_node_00000000019f8640;
	}

autodecoder_node_00000000019c8420:
	if (instr_word & (1u << 7))
	{
		goto autodecoder_node_00000000018b6be0;
	}

autodecoder_node_00000000019d6de0:
	if (instr_word & (1u << 23))
	{
		goto autodecoder_node_000000000196d920;
	}

autodecoder_node_00000000019e7860:
	if (instr_word & (1u << 8))
	{
		goto autodecoder_node_000000000181eba0;
	}

autodecoder_node_00000000019d0a00:
	if (instr_word & (1u << 11))
	{
		goto autodecoder_node_00000000017cc240;
	}

autodecoder_node_00000000019d98a0:
	if (instr_word & (1u << 10))
	{
		goto autodecoder_node_00000000017cc240;
	}

autodecoder_node_0000000001903980:
	if (instr_word & (1u << 9))
	{
		goto autodecoder_node_00000000017cc240;
	}

autodecoder_node_00000000019c0640:
	if (instr_word & (1u << 21))
	{
		goto autodecoder_node_00000000018adb40;
	}

autodecoder_node_00000000018c7280:
	if (instr_word & (1u << 20))
	{
		goto autodecoder_node_00000000017cc240;
	}

autodecoder_node_00000000019cd7c0:
	if ((instr_word & (1u << 16)) == 0)
	{
		goto autodecoder_node_00000000017cc240;
	}

autodecoder_node_00000000019f4060:
	if ((instr_word & (1u << 19)) == 0)
	{
		goto autodecoder_node_00000000017cc240;
	}

autodecoder_node_00000000019a7ce0:
	if ((instr_word & (1u << 17)) == 0)
	{
		goto autodecoder_node_00000000017cc240;
	}

autodecoder_node_00000000019cd2a0:
	if ((instr_word & (1u << 18)) == 0)
	{
		goto autodecoder_node_00000000017cc240;
	}

autodecoder_node_00000000019d0400:
	if (instr_word & (1u << 5))
	{
		goto autodecoder_node_00000000017cc240;
	}

autodecoder_node_00000000019d2100:
	if (instr_word & (1u << 6))
	{
		goto autodecoder_node_00000000017cc240;
	}

autodecoder_node_00000000019fa180:
	if (instr_word & (1u << 3))
	{
		goto autodecoder_node_00000000017cc240;
	}

autodecoder_node_00000000019e0e60:
	if (instr_word & (1u << 0))
	{
		goto autodecoder_node_00000000017cc240;
	}

autodecoder_node_00000000019e8200:
	if (instr_word & (1u << 2))
	{
		goto autodecoder_node_00000000017cc240;
	}

autodecoder_node_00000000019ef180:
	if (instr_word & (1u << 1))
	{
		goto autodecoder_node_00000000017cc240;
	}

autodecoder_node_00000000018946a0:
	return &mrsInstruction;

autodecoder_node_00000000018adb40:
	if (instr_word & (1u << 20))
	{
		goto autodecoder_node_00000000017cc240;
	}

autodecoder_node_00000000019c0a40:
	if (instr_word & (1u << 5))
	{
		goto autodecoder_node_00000000017cc240;
	}

autodecoder_node_00000000019d3500:
	if (instr_word & (1u << 6))
	{
		goto autodecoder_node_00000000017cc240;
	}
	goto autodecoder_node_0000000001892400;

autodecoder_node_000000000181eba0:
	if ((instr_word & (1u << 11)) == 0)
	{
		goto autodecoder_node_00000000017cc240;
	}

autodecoder_node_00000000019bf940:
	if ((instr_word & (1u << 10)) == 0)
	{
		goto autodecoder_node_00000000017cc240;
	}

autodecoder_node_00000000019ce2e0:
	if ((instr_word & (1u << 9)) == 0)
	{
		goto autodecoder_node_00000000017cc240;
	}

autodecoder_node_00000000019fa7a0:
	if ((instr_word & (1u << 21)) == 0)
	{
		goto autodecoder_node_00000000017cc240;
	}

autodecoder_node_00000000019e2460:
	if (instr_word & (1u << 20))
	{
		goto autodecoder_node_00000000017cc240;
	}

autodecoder_node_00000000019e10e0:
	if ((instr_word & (1u << 16)) == 0)
	{
		goto autodecoder_node_00000000017cc240;
	}

autodecoder_node_0000000001862ee0:
	if ((instr_word & (1u << 19)) == 0)
	{
		goto autodecoder_node_00000000017cc240;
	}

autodecoder_node_000000000181a8e0:
	if ((instr_word & (1u << 17)) == 0)
	{
		goto autodecoder_node_00000000017cc240;
	}

autodecoder_node_00000000019ceb00:
	if ((instr_word & (1u << 18)) == 0)
	{
		goto autodecoder_node_00000000017cc240;
	}

autodecoder_node_00000000019e8ea0:
	if ((instr_word & (1u << 5)) == 0)
	{
		goto autodecoder_node_00000000017cc240;
	}

autodecoder_node_00000000019ea560:
	if (instr_word & (1u << 6))
	{
		goto autodecoder_node_00000000017cc240;
	}

autodecoder_node_00000000019c78e0:
	if (instr_word & (1u << 22))
	{
		goto autodecoder_node_00000000017cc240;
	}

autodecoder_node_0000000001804820:
	return &bxjInstruction;

autodecoder_node_000000000196d920:
	if (instr_word & (1u << 8))
	{
		goto autodecoder_node_00000000019e1b20;
	}

autodecoder_node_00000000019c0fe0:
	if (instr_word & (1u << 11))
	{
		goto autodecoder_node_00000000019e1b20;
	}

autodecoder_node_00000000019fb000:
	if (instr_word & (1u << 10))
	{
		goto autodecoder_node_00000000019e1b20;
	}

autodecoder_node_00000000019c5120:
	if (instr_word & (1u << 9))
	{
		goto autodecoder_node_00000000019e1b20;
	}

autodecoder_node_00000000019c80c0:
	if ((instr_word & (1u << 21)) == 0)
	{
		goto autodecoder_node_00000000019e2ec0;
	}

autodecoder_node_00000000019c3280:
	if (instr_word & (1u << 16))
	{
		goto autodecoder_node_0000000001897260;
	}

autodecoder_node_00000000019c1940:
	if (instr_word & (1u << 19))
	{
		goto autodecoder_node_0000000001897260;
	}

autodecoder_node_00000000017eb220:
	if (instr_word & (1u << 17))
	{
		goto autodecoder_node_0000000001897260;
	}

autodecoder_node_00000000019f4780:
	if (instr_word & (1u << 18))
	{
		goto autodecoder_node_0000000001897260;
	}

autodecoder_node_000000000181ebc0:
	if (instr_word & (1u << 5))
	{
		goto autodecoder_node_00000000019c77a0;
	}

autodecoder_node_00000000019d6180:
	if (instr_word & (1u << 6))
	{
		goto autodecoder_node_00000000019c0a20;
	}
	goto autodecoder_node_00000000019cf0a0;

autodecoder_node_00000000019c77a0:
	if ((instr_word & (1u << 6)) == 0)
	{
		goto autodecoder_node_00000000019c29c0;
	}

autodecoder_node_00000000019e0aa0:
	if (instr_word & (1u << 22))
	{
		goto autodecoder_node_00000000018b6800;
	}

autodecoder_node_00000000018c43c0:
	return &rrxInstruction;

autodecoder_node_00000000018b6be0:
	if (instr_word & (1u << 23))
	{
		goto autodecoder_node_00000000019e1b20;
	}

autodecoder_node_00000000019c2820:
	if (instr_word & (1u << 21))
	{
		goto autodecoder_node_000000000187d560;
	}

autodecoder_node_00000000019caba0:
	if (instr_word & (1u << 20))
	{
		goto autodecoder_node_00000000017cc240;
	}

autodecoder_node_00000000019e1740:
	if (instr_word & (1u << 5))
	{
		goto autodecoder_node_00000000019e9f40;
	}

autodecoder_node_00000000019dda40:
	if (instr_word & (1u << 6))
	{
		goto autodecoder_node_00000000019d8160;
	}

autodecoder_node_00000000018a1660:
	if (instr_word & (1u << 22))
	{
		goto autodecoder_node_0000000001819620;
	}

autodecoder_node_0000000001812f80:
	return &smlabbInstruction;

autodecoder_node_0000000001819620:
	return &smlalbbInstruction;

autodecoder_node_00000000019d8160:
	if (instr_word & (1u << 22))
	{
		goto autodecoder_node_000000000181b240;
	}

autodecoder_node_0000000001815260:
	return &smlabtInstruction;

autodecoder_node_000000000181b240:
	return &smlalbtInstruction;

autodecoder_node_00000000019e9f40:
	if (instr_word & (1u << 6))
	{
		goto autodecoder_node_00000000019c2840;
	}

autodecoder_node_00000000019c14e0:
	if (instr_word & (1u << 22))
	{
		goto autodecoder_node_000000000181a620;
	}

autodecoder_node_0000000001814480:
	return &smlatbInstruction;

autodecoder_node_000000000181a620:
	return &smlaltbInstruction;

autodecoder_node_00000000019c2840:
	if (instr_word & (1u << 22))
	{
		goto autodecoder_node_000000000181c1a0;
	}

autodecoder_node_0000000001816700:
	return &smlattInstruction;

autodecoder_node_000000000181c1a0:
	return &smlalttInstruction;

autodecoder_node_000000000187d560:
	if (instr_word & (1u << 20))
	{
		goto autodecoder_node_00000000017cc240;
	}

autodecoder_node_00000000019f65c0:
	if (instr_word & (1u << 5))
	{
		goto autodecoder_node_00000000017cc240;
	}

autodecoder_node_00000000019d4100:
	if (instr_word & (1u << 6))
	{
		goto autodecoder_node_00000000019e8ec0;
	}

autodecoder_node_00000000019da4e0:
	if (instr_word & (1u << 22))
	{
		goto autodecoder_node_00000000017cc240;
	}

autodecoder_node_0000000001817500:
	return &smlawbInstruction;

autodecoder_node_00000000019e8ec0:
	if (instr_word & (1u << 22))
	{
		goto autodecoder_node_00000000017cc240;
	}

autodecoder_node_00000000018185c0:
	return &smlawtInstruction;

autodecoder_node_00000000019e1b20:
	if ((instr_word & (1u << 21)) == 0)
	{
		goto autodecoder_node_00000000019e2ec0;
	}

autodecoder_node_00000000019e34e0:
	if (instr_word & (1u << 16))
	{
		goto autodecoder_node_0000000001897260;
	}

autodecoder_node_00000000019f7160:
	if (instr_word & (1u << 19))
	{
		goto autodecoder_node_0000000001897260;
	}

autodecoder_node_00000000019d01c0:
	if (instr_word & (1u << 17))
	{
		goto autodecoder_node_0000000001897260;
	}

autodecoder_node_00000000019bf9e0:
	if (instr_word & (1u << 18))
	{
		goto autodecoder_node_0000000001897260;
	}

autodecoder_node_00000000019fb040:
	if ((instr_word & (1u << 5)) == 0)
	{
		goto autodecoder_node_00000000019f9480;
	}

autodecoder_node_00000000019bb5e0:
	if ((instr_word & (1u << 6)) == 0)
	{
		goto autodecoder_node_00000000019c29c0;
	}

autodecoder_node_00000000019cf3c0:
	if (instr_word & (1u << 22))
	{
		goto autodecoder_node_00000000018b6800;
	}

autodecoder_node_00000000018c7380:
	return &rorInstruction;

autodecoder_node_00000000019f8640:
	if (instr_word & (1u << 7))
	{
		goto autodecoder_node_00000000019ef120;
	}

autodecoder_node_00000000019bb4c0:
	if (instr_word & (1u << 23))
	{
		goto autodecoder_node_00000000019e82e0;
	}

autodecoder_node_00000000019ed0a0:
	if (instr_word & (1u << 8))
	{
		goto autodecoder_node_00000000019d7620;
	}

autodecoder_node_00000000019ef2c0:
	if (instr_word & (1u << 11))
	{
		goto autodecoder_node_00000000019de5c0;
	}

autodecoder_node_00000000019eade0:
	if (instr_word & (1u << 10))
	{
		goto autodecoder_node_00000000019de5c0;
	}

autodecoder_node_00000000019f9ce0:
	if (instr_word & (1u << 9))
	{
		goto autodecoder_node_00000000019de5c0;
	}

autodecoder_node_00000000019c38e0:
	if (instr_word & (1u << 21))
	{
		goto autodecoder_node_00000000019d9e20;
	}

autodecoder_node_00000000019d5320:
	if (instr_word & (1u << 20))
	{
		goto autodecoder_node_00000000017cc240;
	}

autodecoder_node_00000000019e2b00:
	if (instr_word & (1u << 5))
	{
		goto autodecoder_node_00000000017cc240;
	}

autodecoder_node_00000000019e75e0:
	if ((instr_word & (1u << 6)) == 0)
	{
		goto autodecoder_node_00000000017cc240;
	}

autodecoder_node_00000000019caaa0:
	if (instr_word & (1u << 22))
	{
		goto autodecoder_node_000000000180c160;
	}

autodecoder_node_000000000180a5e0:
	return &qaddInstruction;

autodecoder_node_000000000180c160:
	return &qdaddInstruction;

autodecoder_node_00000000019d9e20:
	if (instr_word & (1u << 20))
	{
		goto autodecoder_node_00000000017cc240;
	}

autodecoder_node_00000000019c7e80:
	if (instr_word & (1u << 5))
	{
		goto autodecoder_node_00000000019f3760;
	}

autodecoder_node_00000000019cb960:
	if ((instr_word & (1u << 6)) == 0)
	{
		goto autodecoder_node_00000000017cc240;
	}

autodecoder_node_00000000019e04a0:
	if (instr_word & (1u << 22))
	{
		goto autodecoder_node_000000000180ec40;
	}

autodecoder_node_000000000180d760:
	return &qsubInstruction;

autodecoder_node_000000000180ec40:
	return &qdsubInstruction;

autodecoder_node_00000000019d7620:
	if ((instr_word & (1u << 11)) == 0)
	{
		goto autodecoder_node_00000000019de5c0;
	}

autodecoder_node_0000000001917b80:
	if ((instr_word & (1u << 10)) == 0)
	{
		goto autodecoder_node_00000000019de5c0;
	}

autodecoder_node_00000000019d2820:
	if (instr_word & (1u << 9))
	{
		goto autodecoder_node_00000000019ef340;
	}

autodecoder_node_00000000019de5c0:
	if ((instr_word & (1u << 21)) == 0)
	{
		goto autodecoder_node_00000000017cc240;
	}

autodecoder_node_00000000019c1180:
	if (instr_word & (1u << 20))
	{
		goto autodecoder_node_00000000017cc240;
	}
	goto autodecoder_node_00000000019ed6a0;

autodecoder_node_00000000019ef340:
	if ((instr_word & (1u << 21)) == 0)
	{
		goto autodecoder_node_00000000017cc240;
	}

autodecoder_node_00000000019e00c0:
	if (instr_word & (1u << 20))
	{
		goto autodecoder_node_00000000017cc240;
	}

autodecoder_node_00000000019ec5c0:
	if ((instr_word & (1u << 16)) == 0)
	{
		goto autodecoder_node_00000000019ed6a0;
	}

autodecoder_node_00000000019dc020:
	if ((instr_word & (1u << 19)) == 0)
	{
		goto autodecoder_node_00000000019ed6a0;
	}

autodecoder_node_00000000019d80a0:
	if ((instr_word & (1u << 17)) == 0)
	{
		goto autodecoder_node_00000000019ed6a0;
	}

autodecoder_node_00000000019d84c0:
	if (instr_word & (1u << 18))
	{
		goto autodecoder_node_00000000019d2f80;
	}

autodecoder_node_00000000019ed6a0:
	if ((instr_word & (1u << 5)) == 0)
	{
		goto autodecoder_node_00000000017cc240;
	}

autodecoder_node_00000000019f3760:
	if (instr_word & (1u << 6))
	{
		goto autodecoder_node_00000000019d4e20;
	}
	goto autodecoder_node_00000000017cc240;

autodecoder_node_00000000019d2f80:
	if (instr_word & (1u << 5))
	{
		goto autodecoder_node_00000000019d2c60;
	}

autodecoder_node_00000000019e1760:
	if (instr_word & (1u << 6))
	{
		goto autodecoder_node_00000000017cc240;
	}

autodecoder_node_00000000019e2bc0:
	if (instr_word & (1u << 22))
	{
		goto autodecoder_node_00000000017cc240;
	}

autodecoder_node_0000000001803160:
	return &bxInstruction;

autodecoder_node_00000000019d2c60:
	if (instr_word & (1u << 6))
	{
		goto autodecoder_node_00000000019d4e20;
	}

autodecoder_node_00000000019cdb40:
	if (instr_word & (1u << 22))
	{
		goto autodecoder_node_00000000017cc240;
	}
	goto autodecoder_node_00000000017d5760;

autodecoder_node_00000000019d4e20:
	if (instr_word & (1u << 22))
	{
		goto autodecoder_node_0000000001806aa0;
	}

autodecoder_node_0000000001805b00:
	return &bkptInstruction;

autodecoder_node_0000000001806aa0:
	return &smcInstruction;

autodecoder_node_00000000019e82e0:
	if ((instr_word & (1u << 21)) == 0)
	{
		goto autodecoder_node_00000000019e2ec0;
	}

autodecoder_node_000000000196d820:
	if (instr_word & (1u << 16))
	{
		goto autodecoder_node_0000000001897260;
	}

autodecoder_node_00000000019e0880:
	if (instr_word & (1u << 19))
	{
		goto autodecoder_node_0000000001897260;
	}

autodecoder_node_00000000019e0820:
	if (instr_word & (1u << 17))
	{
		goto autodecoder_node_0000000001897260;
	}

autodecoder_node_00000000019f5fe0:
	if (instr_word & (1u << 18))
	{
		goto autodecoder_node_0000000001897260;
	}

autodecoder_node_00000000019d7700:
	if (instr_word & (1u << 5))
	{
		goto autodecoder_node_00000000019c35a0;
	}

autodecoder_node_00000000019f9480:
	if (instr_word & (1u << 6))
	{
		goto autodecoder_node_00000000019c0a20;
	}

autodecoder_node_00000000019c29c0:
	if (instr_word & (1u << 22))
	{
		goto autodecoder_node_00000000018b6800;
	}

autodecoder_node_00000000018b6840:
	return &lslInstruction;

autodecoder_node_00000000019c0a20:
	if (instr_word & (1u << 22))
	{
		goto autodecoder_node_00000000018b6800;
	}

autodecoder_node_00000000018be7a0:
	return &asrInstruction;

autodecoder_node_00000000019c35a0:
	if (instr_word & (1u << 6))
	{
		goto autodecoder_node_00000000019e5fe0;
	}

autodecoder_node_00000000019f97c0:
	if (instr_word & (1u << 22))
	{
		goto autodecoder_node_00000000018b6800;
	}

autodecoder_node_00000000018bc460:
	return &lsrInstruction;

autodecoder_node_00000000019e5fe0:
	if (instr_word & (1u << 22))
	{
		goto autodecoder_node_00000000018b6800;
	}
	goto autodecoder_node_00000000017cc240;

autodecoder_node_00000000019ef120:
	if (instr_word & (1u << 23))
	{
		goto autodecoder_node_00000000019d8080;
	}

autodecoder_node_00000000019e6880:
	if (instr_word & (1u << 8))
	{
		goto autodecoder_node_00000000019f8780;
	}

autodecoder_node_00000000019ea600:
	if (instr_word & (1u << 11))
	{
		goto autodecoder_node_00000000019f8780;
	}

autodecoder_node_00000000019e0180:
	if (instr_word & (1u << 10))
	{
		goto autodecoder_node_00000000019f8780;
	}

autodecoder_node_00000000019db9a0:
	if (instr_word & (1u << 9))
	{
		goto autodecoder_node_00000000019f8780;
	}

autodecoder_node_00000000019f5000:
	if (instr_word & (1u << 21))
	{
		goto autodecoder_node_00000000019c41e0;
	}

autodecoder_node_00000000019fab20:
	if (instr_word & (1u << 20))
	{
		goto autodecoder_node_00000000017cc240;
	}

autodecoder_node_00000000019ef3c0:
	if (instr_word & (1u << 5))
	{
		goto autodecoder_node_00000000017cc240;
	}

autodecoder_node_00000000019f7340:
	if (instr_word & (1u << 6))
	{
		goto autodecoder_node_00000000017cc240;
	}

autodecoder_node_00000000017ea9e0:
	return &swpInstruction;

autodecoder_node_00000000019d8080:
	if (instr_word & (1u << 8))
	{
		goto autodecoder_node_00000000019c8600;
	}

autodecoder_node_00000000019d4b40:
	if (instr_word & (1u << 11))
	{
		goto autodecoder_node_00000000019f8780;
	}

autodecoder_node_0000000001814940:
	if (instr_word & (1u << 10))
	{
		goto autodecoder_node_00000000019f8780;
	}

autodecoder_node_00000000019f4f00:
	if (instr_word & (1u << 9))
	{
		goto autodecoder_node_00000000019f8780;
	}

autodecoder_node_00000000019c41e0:
	if (instr_word & (1u << 20))
	{
		goto autodecoder_node_00000000017cc240;
	}

autodecoder_node_00000000019e9600:
	if (instr_word & (1u << 5))
	{
		goto autodecoder_node_00000000017cc240;
	}
	goto autodecoder_node_00000000019ed940;

autodecoder_node_00000000019c8600:
	if ((instr_word & (1u << 11)) == 0)
	{
		goto autodecoder_node_00000000019f8780;
	}

autodecoder_node_00000000019cd0a0:
	if ((instr_word & (1u << 10)) == 0)
	{
		goto autodecoder_node_00000000019f8780;
	}

autodecoder_node_00000000019d4080:
	if (instr_word & (1u << 9))
	{
		goto autodecoder_node_00000000019f45e0;
	}

autodecoder_node_00000000019f8780:
	if (instr_word & (1u << 20))
	{
		goto autodecoder_node_00000000019f77e0;
	}

autodecoder_node_00000000019f7200:
	if (instr_word & (1u << 5))
	{
		goto autodecoder_node_00000000019f77e0;
	}

autodecoder_node_00000000019d6b60:
	if (instr_word & (1u << 6))
	{
		goto autodecoder_node_00000000019f77e0;
	}
	goto autodecoder_node_00000000017cc6c0;

autodecoder_node_00000000019f45e0:
	if (instr_word & (1u << 21))
	{
		goto autodecoder_node_00000000019cbfc0;
	}

autodecoder_node_00000000019c2440:
	if (instr_word & (1u << 20))
	{
		goto autodecoder_node_00000000019d6a00;
	}

autodecoder_node_00000000019c1da0:
	if (instr_word & (1u << 5))
	{
		goto autodecoder_node_00000000019f77e0;
	}

autodecoder_node_00000000019e80e0:
	if (instr_word & (1u << 6))
	{
		goto autodecoder_node_00000000019f77e0;
	}

autodecoder_node_0000000001921dc0:
	if (instr_word & (1u << 22))
	{
		goto autodecoder_node_00000000017f3560;
	}

autodecoder_node_00000000017f8b00:
	return &strexInstruction;

autodecoder_node_00000000017f3560:
	return &strexbInstruction;

autodecoder_node_00000000019d6a00:
	if (instr_word & (1u << 5))
	{
		goto autodecoder_node_00000000019f77e0;
	}

autodecoder_node_00000000019b09e0:
	if (instr_word & (1u << 6))
	{
		goto autodecoder_node_00000000019f77e0;
	}

autodecoder_node_00000000019c92e0:
	if ((instr_word & (1u << 3)) == 0)
	{
		goto autodecoder_node_00000000019f77e0;
	}

autodecoder_node_00000000019f78e0:
	if ((instr_word & (1u << 0)) == 0)
	{
		goto autodecoder_node_00000000019f77e0;
	}

autodecoder_node_00000000019122c0:
	if ((instr_word & (1u << 2)) == 0)
	{
		goto autodecoder_node_00000000019f77e0;
	}

autodecoder_node_00000000019f3b60:
	if ((instr_word & (1u << 1)) == 0)
	{
		goto autodecoder_node_00000000019f77e0;
	}

autodecoder_node_00000000019e1dc0:
	if (instr_word & (1u << 22))
	{
		goto autodecoder_node_00000000017ef080;
	}

autodecoder_node_00000000017f77c0:
	return &ldrexInstruction;

autodecoder_node_00000000017ef080:
	return &ldrexbInstruction;

autodecoder_node_00000000019cbfc0:
	if (instr_word & (1u << 20))
	{
		goto autodecoder_node_00000000019d9b80;
	}

autodecoder_node_0000000001827260:
	if (instr_word & (1u << 5))
	{
		goto autodecoder_node_00000000019f77e0;
	}

autodecoder_node_00000000019c13c0:
	if (instr_word & (1u << 6))
	{
		goto autodecoder_node_00000000019f77e0;
	}

autodecoder_node_00000000019c8900:
	if (instr_word & (1u << 22))
	{
		goto autodecoder_node_00000000017f5f40;
	}

autodecoder_node_00000000017f4aa0:
	return &strexdInstruction;

autodecoder_node_00000000017f5f40:
	return &strexhInstruction;

autodecoder_node_00000000019d9b80:
	if (instr_word & (1u << 5))
	{
		goto autodecoder_node_00000000019f77e0;
	}

autodecoder_node_000000000180c6a0:
	if (instr_word & (1u << 6))
	{
		goto autodecoder_node_00000000019f77e0;
	}

autodecoder_node_00000000019c7f80:
	if ((instr_word & (1u << 3)) == 0)
	{
		goto autodecoder_node_00000000019f77e0;
	}

autodecoder_node_00000000019dc1c0:
	if ((instr_word & (1u << 0)) == 0)
	{
		goto autodecoder_node_00000000019f77e0;
	}

autodecoder_node_00000000019e9ba0:
	if ((instr_word & (1u << 2)) == 0)
	{
		goto autodecoder_node_00000000019f77e0;
	}

autodecoder_node_00000000019cd960:
	if ((instr_word & (1u << 1)) == 0)
	{
		goto autodecoder_node_00000000019f77e0;
	}

autodecoder_node_00000000019f4460:
	if (instr_word & (1u << 22))
	{
		goto autodecoder_node_00000000017f2080;
	}

autodecoder_node_00000000017f09c0:
	return &ldrexdInstruction;

autodecoder_node_00000000017f2080:
	return &ldrexhInstruction;

autodecoder_node_00000000019d3820:
	if (instr_word & (1u << 24))
	{
		goto autodecoder_node_000000000196d660;
	}

autodecoder_node_00000000019c3d60:
	if (instr_word & (1u << 23))
	{
		goto autodecoder_node_00000000019cb7c0;
	}

autodecoder_node_00000000019d2d20:
	if (instr_word & (1u << 21))
	{
		goto autodecoder_node_00000000019e59c0;
	}

autodecoder_node_00000000019d3880:
	if (instr_word & (1u << 22))
	{
		goto autodecoder_node_0000000001862b20;
	}

autodecoder_node_0000000001829480:
	return &andInstruction;

autodecoder_node_0000000001862b20:
	return &subInstruction;

autodecoder_node_00000000019e59c0:
	if (instr_word & (1u << 22))
	{
		goto autodecoder_node_000000000186aac0;
	}

autodecoder_node_000000000185ac80:
	return &eorInstruction;

autodecoder_node_000000000186aac0:
	return &rsbInstruction;

autodecoder_node_00000000019cb7c0:
	if (instr_word & (1u << 21))
	{
		goto autodecoder_node_00000000019d0100;
	}

autodecoder_node_00000000019c3000:
	if (instr_word & (1u << 22))
	{
		goto autodecoder_node_0000000001882320;
	}

autodecoder_node_0000000001872840:
	return &addInstruction;

autodecoder_node_0000000001882320:
	return &sbcInstruction;

autodecoder_node_00000000019d0100:
	if (instr_word & (1u << 22))
	{
		goto autodecoder_node_000000000188a060;
	}

autodecoder_node_000000000187a740:
	return &adcInstruction;

autodecoder_node_000000000188a060:
	return &rscInstruction;

autodecoder_node_000000000196d660:
	if (instr_word & (1u << 4))
	{
		goto autodecoder_node_00000000019ce4c0;
	}

autodecoder_node_00000000019e4b20:
	if (instr_word & (1u << 7))
	{
		goto autodecoder_node_00000000019e9320;
	}

autodecoder_node_00000000019c5420:
	if (instr_word & (1u << 23))
	{
		goto autodecoder_node_00000000019edce0;
	}

autodecoder_node_00000000019dfa40:
	if (instr_word & (1u << 8))
	{
		goto autodecoder_node_00000000019d19e0;
	}

autodecoder_node_00000000019f6220:
	if (instr_word & (1u << 11))
	{
		goto autodecoder_node_00000000019d19e0;
	}

autodecoder_node_00000000019ed5c0:
	if (instr_word & (1u << 10))
	{
		goto autodecoder_node_00000000019d19e0;
	}

autodecoder_node_00000000019d1460:
	if (instr_word & (1u << 9))
	{
		goto autodecoder_node_00000000019d19e0;
	}

autodecoder_node_00000000019f6820:
	if ((instr_word & (1u << 21)) == 0)
	{
		goto autodecoder_node_00000000019f9de0;
	}

autodecoder_node_00000000019d7860:
	if (instr_word & (1u << 20))
	{
		goto autodecoder_node_00000000017cc240;
	}

autodecoder_node_00000000019cb200:
	if (instr_word & (1u << 16))
	{
		goto autodecoder_node_0000000001892400;
	}

autodecoder_node_00000000019ec8c0:
	if (instr_word & (1u << 19))
	{
		goto autodecoder_node_0000000001892400;
	}

autodecoder_node_00000000018c7420:
	if (instr_word & (1u << 17))
	{
		goto autodecoder_node_0000000001892400;
	}

autodecoder_node_00000000019e4f80:
	if (instr_word & (1u << 18))
	{
		goto autodecoder_node_0000000001892400;
	}

autodecoder_node_00000000019d5480:
	if (instr_word & (1u << 5))
	{
		goto autodecoder_node_0000000001892400;
	}

autodecoder_node_00000000019da020:
	if (instr_word & (1u << 6))
	{
		goto autodecoder_node_0000000001892400;
	}

autodecoder_node_00000000018a2440:
	if (instr_word & (1u << 3))
	{
		goto autodecoder_node_0000000001892400;
	}

autodecoder_node_00000000019d1f20:
	if (instr_word & (1u << 0))
	{
		goto autodecoder_node_00000000019ebb00;
	}

autodecoder_node_00000000019de1c0:
	if (instr_word & (1u << 2))
	{
		goto autodecoder_node_00000000019dbee0;
	}

autodecoder_node_00000000019ce800:
	if ((instr_word & (1u << 1)) == 0)
	{
		goto autodecoder_node_00000000019d5e60;
	}

autodecoder_node_00000000019d1aa0:
	if (instr_word & (1u << 22))
	{
		goto autodecoder_node_0000000001892400;
	}

autodecoder_node_00000000018a5d60:
	return &wfeInstruction;

autodecoder_node_00000000019dbee0:
	if (instr_word & (1u << 1))
	{
		goto autodecoder_node_0000000001892400;
	}

autodecoder_node_00000000019c0d60:
	if (instr_word & (1u << 22))
	{
		goto autodecoder_node_0000000001892400;
	}

autodecoder_node_0000000001897e60:
	return &sevInstruction;

autodecoder_node_00000000019ebb00:
	if (instr_word & (1u << 2))
	{
		goto autodecoder_node_0000000001892400;
	}

autodecoder_node_00000000019e3600:
	if (instr_word & (1u << 1))
	{
		goto autodecoder_node_00000000019d3540;
	}

autodecoder_node_00000000019f68e0:
	if (instr_word & (1u << 22))
	{
		goto autodecoder_node_0000000001892400;
	}

autodecoder_node_00000000018a4520:
	return &yieldInstruction;

autodecoder_node_00000000019d3540:
	if (instr_word & (1u << 22))
	{
		goto autodecoder_node_0000000001892400;
	}

autodecoder_node_00000000018a6aa0:
	return &wfiInstruction;

autodecoder_node_00000000019d19e0:
	if ((instr_word & (1u << 21)) == 0)
	{
		goto autodecoder_node_00000000019f9de0;
	}

autodecoder_node_00000000019d0660:
	if (instr_word & (1u << 20))
	{
		goto autodecoder_node_00000000017cc240;
	}

autodecoder_node_00000000019d9700:
	if (instr_word & (1u << 16))
	{
		goto autodecoder_node_0000000001892400;
	}

autodecoder_node_00000000019f71a0:
	if (instr_word & (1u << 19))
	{
		goto autodecoder_node_0000000001892400;
	}

autodecoder_node_00000000019e8000:
	if (instr_word & (1u << 17))
	{
		goto autodecoder_node_0000000001892400;
	}

autodecoder_node_00000000018b3360:
	if (instr_word & (1u << 18))
	{
		goto autodecoder_node_0000000001892400;
	}

autodecoder_node_00000000019dcd40:
	if (instr_word & (1u << 5))
	{
		goto autodecoder_node_0000000001892400;
	}

autodecoder_node_00000000019dca80:
	if (instr_word & (1u << 6))
	{
		goto autodecoder_node_0000000001892400;
	}

autodecoder_node_00000000019d75a0:
	if (instr_word & (1u << 3))
	{
		goto autodecoder_node_0000000001892400;
	}

autodecoder_node_00000000019ec060:
	if (instr_word & (1u << 0))
	{
		goto autodecoder_node_0000000001892400;
	}

autodecoder_node_00000000019f8d00:
	if (instr_word & (1u << 2))
	{
		goto autodecoder_node_0000000001892400;
	}

autodecoder_node_00000000019d58c0:
	if (instr_word & (1u << 1))
	{
		goto autodecoder_node_0000000001892400;
	}

autodecoder_node_00000000019d5e60:
	if (instr_word & (1u << 22))
	{
		goto autodecoder_node_0000000001892400;
	}

autodecoder_node_0000000001899400:
	return &nopInstruction;

autodecoder_node_00000000019ce4c0:
	if (instr_word & (1u << 7))
	{
		goto autodecoder_node_00000000019e71c0;
	}

autodecoder_node_00000000019e9320:
	if (instr_word & (1u << 23))
	{
		goto autodecoder_node_00000000019edce0;
	}
	goto autodecoder_node_00000000019cb040;

autodecoder_node_00000000019e71c0:
	if (instr_word & (1u << 23))
	{
		goto autodecoder_node_00000000019edce0;
	}

autodecoder_node_00000000018c4a60:
	if (instr_word & (1u << 8))
	{
		goto autodecoder_node_00000000019cb040;
	}

autodecoder_node_00000000019f5420:
	if (instr_word & (1u << 11))
	{
		goto autodecoder_node_00000000019cb040;
	}

autodecoder_node_00000000019ed980:
	if (instr_word & (1u << 10))
	{
		goto autodecoder_node_00000000019cb040;
	}

autodecoder_node_00000000019f5840:
	if (instr_word & (1u << 9))
	{
		goto autodecoder_node_00000000019cb040;
	}

autodecoder_node_00000000019cf000:
	if ((instr_word & (1u << 21)) == 0)
	{
		goto autodecoder_node_00000000019f9de0;
	}

autodecoder_node_00000000019ee620:
	if (instr_word & (1u << 20))
	{
		goto autodecoder_node_00000000017cc240;
	}

autodecoder_node_00000000019d9200:
	if (instr_word & (1u << 16))
	{
		goto autodecoder_node_0000000001892400;
	}

autodecoder_node_00000000019ecea0:
	if (instr_word & (1u << 19))
	{
		goto autodecoder_node_0000000001892400;
	}

autodecoder_node_00000000019e5580:
	if (instr_word & (1u << 17))
	{
		goto autodecoder_node_0000000001892400;
	}

autodecoder_node_00000000019fa060:
	if (instr_word & (1u << 18))
	{
		goto autodecoder_node_0000000001892400;
	}

autodecoder_node_00000000019d35e0:
	if ((instr_word & (1u << 5)) == 0)
	{
		goto autodecoder_node_0000000001892400;
	}

autodecoder_node_000000000185a400:
	if ((instr_word & (1u << 6)) == 0)
	{
		goto autodecoder_node_0000000001892400;
	}

autodecoder_node_00000000019ece80:
	if (instr_word & (1u << 22))
	{
		goto autodecoder_node_0000000001892400;
	}

autodecoder_node_000000000189b3c0:
	return &dbgInstruction;

autodecoder_node_00000000019cb040:
	if (instr_word & (1u << 21))
	{
		goto autodecoder_node_00000000019fa500;
	}

autodecoder_node_00000000019f9de0:
	if (instr_word & (1u << 20))
	{
		goto autodecoder_node_00000000017cc240;
	}

autodecoder_node_00000000019e1600:
	if ((instr_word & (1u << 22)) == 0)
	{
		goto autodecoder_node_00000000017cc240;
	}

autodecoder_node_0000000001897000:
	return &movtInstruction;

autodecoder_node_00000000019fa500:
	if (instr_word & (1u << 20))
	{
		goto autodecoder_node_00000000017cc240;
	}

autodecoder_node_0000000001892400:
	return &msrInstruction;

autodecoder_node_00000000019edce0:
	if (instr_word & (1u << 21))
	{
		goto autodecoder_node_00000000019cb820;
	}

autodecoder_node_00000000019e2ec0:
	if (instr_word & (1u << 22))
	{
		goto autodecoder_node_00000000018ba600;
	}

autodecoder_node_00000000018aa620:
	return &orrInstruction;

autodecoder_node_00000000018ba600:
	return &bicInstruction;

autodecoder_node_00000000019cb820:
	if (instr_word & (1u << 16))
	{
		goto autodecoder_node_0000000001897260;
	}

autodecoder_node_00000000019d5700:
	if (instr_word & (1u << 19))
	{
		goto autodecoder_node_0000000001897260;
	}

autodecoder_node_00000000019e9940:
	if (instr_word & (1u << 17))
	{
		goto autodecoder_node_0000000001897260;
	}

autodecoder_node_00000000019d69c0:
	if (instr_word & (1u << 18))
	{
		goto autodecoder_node_0000000001897260;
	}

autodecoder_node_00000000019cf0a0:
	if (instr_word & (1u << 22))
	{
		goto autodecoder_node_00000000018b6800;
	}

autodecoder_node_00000000018b2ae0:
	return &movInstruction;

autodecoder_node_0000000001897260:
	if ((instr_word & (1u << 22)) == 0)
	{
		goto autodecoder_node_00000000017cc6c0;
	}

autodecoder_node_00000000018b6800:
	return &mvnInstruction;

autodecoder_node_00000000019f9f60:
	if (instr_word & (1u << 15))
	{
		goto autodecoder_node_00000000019f3a20;
	}

autodecoder_node_00000000019e7da0:
	if (instr_word & (1u << 25))
	{
		goto autodecoder_node_000000000196d4e0;
	}

autodecoder_node_00000000019f56a0:
	if ((instr_word & (1u << 20)) == 0)
	{
		goto autodecoder_node_00000000019d55c0;
	}

autodecoder_node_00000000019fa5c0:
	if (instr_word & (1u << 22))
	{
		goto autodecoder_node_0000000001934200;
	}
	goto autodecoder_node_00000000017cc240;

autodecoder_node_00000000019f3a20:
	if (instr_word & (1u << 25))
	{
		goto autodecoder_node_000000000196d4e0;
	}

autodecoder_node_00000000019c7f60:
	if (instr_word & (1u << 24))
	{
		goto autodecoder_node_00000000019e15a0;
	}

autodecoder_node_00000000019f95a0:
	if ((instr_word & (1u << 23)) == 0)
	{
		goto autodecoder_node_00000000019e15a0;
	}

autodecoder_node_00000000019dcb80:
	if ((instr_word & (1u << 21)) == 0)
	{
		goto autodecoder_node_00000000019e15a0;
	}

autodecoder_node_00000000019fa0c0:
	if ((instr_word & (1u << 20)) == 0)
	{
		goto autodecoder_node_00000000019d55c0;
	}

autodecoder_node_00000000019bff40:
	if ((instr_word & (1u << 16)) == 0)
	{
		goto autodecoder_node_0000000001934200;
	}

autodecoder_node_00000000019cdb20:
	if ((instr_word & (1u << 19)) == 0)
	{
		goto autodecoder_node_0000000001934200;
	}

autodecoder_node_00000000019e5820:
	if (instr_word & (1u << 17))
	{
		goto autodecoder_node_0000000001934200;
	}

autodecoder_node_00000000019f57e0:
	if ((instr_word & (1u << 18)) == 0)
	{
		goto autodecoder_node_0000000001934200;
	}

autodecoder_node_00000000019f6260:
	if (instr_word & (1u << 22))
	{
		goto autodecoder_node_0000000001934200;
	}

autodecoder_node_00000000019249e0:
	return &popLdmInstruction;

autodecoder_node_00000000019e15a0:
	if (instr_word & (1u << 20))
	{
		goto autodecoder_node_0000000001934200;
	}

autodecoder_node_00000000019d55c0:
	if ((instr_word & (1u << 22)) == 0)
	{
		goto autodecoder_node_00000000017cc240;
	}

autodecoder_node_00000000017d2ce0:
	return &stmInstruction;

autodecoder_node_0000000001934200:
	return &ldmInstruction;

autodecoder_node_000000000196d4e0:
	return &bInstruction;

autodecoder_node_00000000019d3ce0:
	if (instr_word & (1u << 27))
	{
		goto autodecoder_node_00000000019cffa0;
	}

autodecoder_node_00000000019df420:
	if (instr_word & (1u << 12))
	{
		goto autodecoder_node_00000000017cc6c0;
	}

autodecoder_node_00000000019f5960:
	if (instr_word & (1u << 13))
	{
		goto autodecoder_node_00000000017cc6c0;
	}

autodecoder_node_00000000019ce660:
	if (instr_word & (1u << 14))
	{
		goto autodecoder_node_00000000017cc6c0;
	}

autodecoder_node_00000000019c15c0:
	if (instr_word & (1u << 15))
	{
		goto autodecoder_node_00000000017cc6c0;
	}

autodecoder_node_0000000001815c00:
	if (instr_word & (1u << 25))
	{
		goto autodecoder_node_00000000017cc6c0;
	}

autodecoder_node_00000000019cabe0:
	if ((instr_word & (1u << 24)) == 0)
	{
		goto autodecoder_node_00000000017cc6c0;
	}

autodecoder_node_00000000019c0fa0:
	if (instr_word & (1u << 4))
	{
		goto autodecoder_node_00000000019dccc0;
	}

autodecoder_node_00000000019cc180:
	if (instr_word & (1u << 23))
	{
		goto autodecoder_node_00000000017cc6c0;
	}

autodecoder_node_00000000019d9540:
	if (instr_word & (1u << 11))
	{
		goto autodecoder_node_00000000017cc6c0;
	}

autodecoder_node_00000000019d53c0:
	if (instr_word & (1u << 10))
	{
		goto autodecoder_node_00000000017cc6c0;
	}

autodecoder_node_00000000019e8a80:
	if (instr_word & (1u << 9))
	{
		goto autodecoder_node_00000000019ea580;
	}

autodecoder_node_00000000019ce200:
	if (instr_word & (1u << 21))
	{
		goto autodecoder_node_00000000017cc6c0;
	}

autodecoder_node_00000000018112e0:
	if (instr_word & (1u << 20))
	{
		goto autodecoder_node_00000000017cc6c0;
	}

autodecoder_node_00000000019eaa20:
	if (instr_word & (1u << 16))
	{
		goto autodecoder_node_00000000017fb3a0;
	}

autodecoder_node_0000000001859700:
	if ((instr_word & (1u << 19)) == 0)
	{
		goto autodecoder_node_00000000019d6ac0;
	}

autodecoder_node_00000000019e5da0:
	if (instr_word & (1u << 17))
	{
		goto autodecoder_node_00000000019dc0e0;
	}

autodecoder_node_00000000019c3e40:
	if (instr_word & (1u << 18))
	{
		goto autodecoder_node_000000000186b360;
	}

autodecoder_node_00000000019c2f00:
	if (instr_word & (1u << 5))
	{
		goto autodecoder_node_00000000017cc6c0;
	}

autodecoder_node_000000000199d1e0:
	if (instr_word & (1u << 3))
	{
		goto autodecoder_node_00000000019dbd20;
	}

autodecoder_node_00000000019c85a0:
	if (instr_word & (1u << 0))
	{
		goto autodecoder_node_00000000019dbd20;
	}

autodecoder_node_00000000019e4f00:
	if (instr_word & (1u << 2))
	{
		goto autodecoder_node_00000000019dbd20;
	}

autodecoder_node_00000000019c6720:
	if (instr_word & (1u << 1))
	{
		goto autodecoder_node_00000000019dbd20;
	}
	goto autodecoder_node_00000000019e80c0;

autodecoder_node_000000000186b360:
	if (instr_word & (1u << 5))
	{
		goto autodecoder_node_00000000017cc6c0;
	}

autodecoder_node_00000000019ce960:
	if (instr_word & (1u << 3))
	{
		goto autodecoder_node_00000000019dbd20;
	}

autodecoder_node_00000000019faba0:
	if (instr_word & (1u << 0))
	{
		goto autodecoder_node_00000000019dbd20;
	}

autodecoder_node_00000000019d5740:
	if (instr_word & (1u << 2))
	{
		goto autodecoder_node_00000000019dbd20;
	}

autodecoder_node_00000000019c26e0:
	if (instr_word & (1u << 1))
	{
		goto autodecoder_node_00000000019dbd20;
	}
	goto autodecoder_node_00000000019f3d60;

autodecoder_node_00000000019dccc0:
	if (instr_word & (1u << 23))
	{
		goto autodecoder_node_00000000017cc6c0;
	}

autodecoder_node_00000000019ce0a0:
	if (instr_word & (1u << 11))
	{
		goto autodecoder_node_00000000017cc6c0;
	}

autodecoder_node_00000000018a1680:
	if (instr_word & (1u << 10))
	{
		goto autodecoder_node_00000000017cc6c0;
	}

autodecoder_node_00000000019cb540:
	if (instr_word & (1u << 9))
	{
		goto autodecoder_node_00000000019ea580;
	}

autodecoder_node_00000000019ef020:
	if (instr_word & (1u << 21))
	{
		goto autodecoder_node_00000000017cc6c0;
	}

autodecoder_node_00000000019e0a00:
	if (instr_word & (1u << 20))
	{
		goto autodecoder_node_00000000017cc6c0;
	}

autodecoder_node_00000000019f72e0:
	if (instr_word & (1u << 16))
	{
		goto autodecoder_node_00000000017fb3a0;
	}

autodecoder_node_00000000019d7e40:
	if ((instr_word & (1u << 19)) == 0)
	{
		goto autodecoder_node_00000000019d6ac0;
	}

autodecoder_node_00000000019facc0:
	if (instr_word & (1u << 17))
	{
		goto autodecoder_node_00000000019dc0e0;
	}

autodecoder_node_00000000019d6ac0:
	if (instr_word & (1u << 5))
	{
		goto autodecoder_node_00000000017cc6c0;
	}

autodecoder_node_00000000019dbd20:
	if (instr_word & (1u << 22))
	{
		goto autodecoder_node_00000000017cc6c0;
	}

autodecoder_node_0000000001847660:
	return &cpsInstruction;

autodecoder_node_00000000019dc0e0:
	if (instr_word & (1u << 18))
	{
		goto autodecoder_node_00000000019c5620;
	}

autodecoder_node_00000000019e9020:
	if (instr_word & (1u << 5))
	{
		goto autodecoder_node_00000000017cc6c0;
	}

autodecoder_node_00000000019e80c0:
	if (instr_word & (1u << 22))
	{
		goto autodecoder_node_00000000017cc6c0;
	}

autodecoder_node_00000000018346c0:
	return &cpsieInstruction;

autodecoder_node_00000000019c5620:
	if (instr_word & (1u << 5))
	{
		goto autodecoder_node_00000000017cc6c0;
	}

autodecoder_node_00000000019f3d60:
	if (instr_word & (1u << 22))
	{
		goto autodecoder_node_00000000017cc6c0;
	}

autodecoder_node_00000000018386e0:
	return &cpsidInstruction;

autodecoder_node_00000000019ea580:
	if (instr_word & (1u << 21))
	{
		goto autodecoder_node_00000000017cc6c0;
	}

autodecoder_node_00000000019f9a60:
	if (instr_word & (1u << 20))
	{
		goto autodecoder_node_00000000017cc6c0;
	}

autodecoder_node_00000000019e25a0:
	if ((instr_word & (1u << 16)) == 0)
	{
		goto autodecoder_node_00000000017cc6c0;
	}

autodecoder_node_00000000017fb3a0:
	if (instr_word & (1u << 19))
	{
		goto autodecoder_node_00000000017cc6c0;
	}

autodecoder_node_00000000019f4a40:
	if (instr_word & (1u << 17))
	{
		goto autodecoder_node_00000000017cc6c0;
	}

autodecoder_node_00000000019c95a0:
	if (instr_word & (1u << 18))
	{
		goto autodecoder_node_00000000017cc6c0;
	}

autodecoder_node_00000000019f69a0:
	if (instr_word & (1u << 22))
	{
		goto autodecoder_node_00000000017cc6c0;
	}

autodecoder_node_000000000184be40:
	return &setendInstruction;

autodecoder_node_00000000019cffa0:
	if (instr_word & (1u << 12))
	{
		goto autodecoder_node_00000000019f9fe0;
	}

autodecoder_node_00000000019d9300:
	if (instr_word & (1u << 13))
	{
		goto autodecoder_node_00000000019f9fe0;
	}

autodecoder_node_00000000019e9900:
	if (instr_word & (1u << 14))
	{
		goto autodecoder_node_00000000019f9fe0;
	}

autodecoder_node_00000000019ed0c0:
	if (instr_word & (1u << 15))
	{
		goto autodecoder_node_00000000019f9fe0;
	}

autodecoder_node_0000000001911600:
	if (instr_word & (1u << 25))
	{
		goto autodecoder_node_00000000017d5760;
	}

autodecoder_node_00000000019e7080:
	if (instr_word & (1u << 4))
	{
		goto autodecoder_node_00000000019b0b00;
	}

autodecoder_node_00000000019d63a0:
	if (instr_word & (1u << 7))
	{
		goto autodecoder_node_00000000017cc6c0;
	}

autodecoder_node_00000000019d41c0:
	if (instr_word & (1u << 8))
	{
		goto autodecoder_node_00000000019e0740;
	}

autodecoder_node_00000000019d32a0:
	if ((instr_word & (1u << 11)) == 0)
	{
		goto autodecoder_node_00000000017cc6c0;
	}

autodecoder_node_00000000019d4f00:
	if (instr_word & (1u << 10))
	{
		goto autodecoder_node_00000000017cc6c0;
	}

autodecoder_node_00000000019d5240:
	if ((instr_word & (1u << 9)) == 0)
	{
		goto autodecoder_node_00000000017cc6c0;
	}

autodecoder_node_00000000019fa460:
	if ((instr_word & (1u << 20)) == 0)
	{
		goto autodecoder_node_00000000017cc6c0;
	}

autodecoder_node_00000000019de820:
	if (instr_word & (1u << 5))
	{
		goto autodecoder_node_00000000017cc6c0;
	}

autodecoder_node_00000000019f4300:
	if (instr_word & (1u << 6))
	{
		goto autodecoder_node_00000000017cc6c0;
	}

autodecoder_node_00000000019dace0:
	if (instr_word & (1u << 3))
	{
		goto autodecoder_node_00000000017cc6c0;
	}

autodecoder_node_00000000019d3420:
	if (instr_word & (1u << 0))
	{
		goto autodecoder_node_00000000017cc6c0;
	}

autodecoder_node_00000000019c6fc0:
	if (instr_word & (1u << 2))
	{
		goto autodecoder_node_00000000017cc6c0;
	}

autodecoder_node_00000000019e9520:
	if (instr_word & (1u << 1))
	{
		goto autodecoder_node_00000000017cc6c0;
	}

autodecoder_node_00000000017de5e0:
	if (instr_word & (1u << 22))
	{
		goto autodecoder_node_00000000017cc6c0;
	}

autodecoder_node_000000000184a480:
	return &rfeInstruction;

autodecoder_node_00000000019b0b00:
	if (instr_word & (1u << 7))
	{
		goto autodecoder_node_00000000017cc6c0;
	}

autodecoder_node_00000000019d27c0:
	if ((instr_word & (1u << 8)) == 0)
	{
		goto autodecoder_node_00000000017cc6c0;
	}

autodecoder_node_00000000019e0740:
	if (instr_word & (1u << 11))
	{
		goto autodecoder_node_00000000017cc6c0;
	}

autodecoder_node_00000000017ecfe0:
	if ((instr_word & (1u << 10)) == 0)
	{
		goto autodecoder_node_00000000017cc6c0;
	}

autodecoder_node_00000000019cb3e0:
	if (instr_word & (1u << 9))
	{
		goto autodecoder_node_00000000017cc6c0;
	}

autodecoder_node_00000000019e00e0:
	if (instr_word & (1u << 20))
	{
		goto autodecoder_node_00000000017cc6c0;
	}

autodecoder_node_00000000019c1be0:
	if ((instr_word & (1u << 16)) == 0)
	{
		goto autodecoder_node_00000000017cc6c0;
	}

autodecoder_node_00000000019d8ee0:
	if ((instr_word & (1u << 19)) == 0)
	{
		goto autodecoder_node_00000000017cc6c0;
	}

autodecoder_node_00000000019d7de0:
	if (instr_word & (1u << 17))
	{
		goto autodecoder_node_00000000017cc6c0;
	}

autodecoder_node_00000000019cb9c0:
	if ((instr_word & (1u << 18)) == 0)
	{
		goto autodecoder_node_00000000017cc6c0;
	}

autodecoder_node_00000000019d47a0:
	if (instr_word & (1u << 5))
	{
		goto autodecoder_node_00000000017cc6c0;
	}

autodecoder_node_00000000019f7240:
	if (instr_word & (1u << 6))
	{
		goto autodecoder_node_00000000017cc6c0;
	}

autodecoder_node_00000000019c66a0:
	if ((instr_word & (1u << 22)) == 0)
	{
		goto autodecoder_node_00000000017cc6c0;
	}

autodecoder_node_000000000184e720:
	return &srsInstruction;

autodecoder_node_00000000019f9fe0:
	if ((instr_word & (1u << 25)) == 0)
	{
		goto autodecoder_node_00000000017cc6c0;
	}

autodecoder_node_00000000017d5760:
	return &blxInstruction;

autodecoder_node_00000000019faac0:
	if (instr_word & (1u << 28))
	{
		goto autodecoder_node_00000000019cdba0;
	}

autodecoder_node_00000000019e0640:
	if (instr_word & (1u << 27))
	{
		goto autodecoder_node_00000000017d40a0;
	}

autodecoder_node_00000000019e1380:
	if ((instr_word & (1u << 12)) == 0)
	{
		goto autodecoder_node_00000000019f6180;
	}

autodecoder_node_000000000199bd80:
	if ((instr_word & (1u << 13)) == 0)
	{
		goto autodecoder_node_00000000019f6180;
	}

autodecoder_node_00000000019d4a60:
	if ((instr_word & (1u << 14)) == 0)
	{
		goto autodecoder_node_00000000019f6180;
	}

autodecoder_node_00000000019c7c40:
	if (instr_word & (1u << 15))
	{
		goto autodecoder_node_00000000019e7540;
	}

autodecoder_node_00000000019f6180:
	if ((instr_word & (1u << 25)) == 0)
	{
		goto autodecoder_node_00000000017cc240;
	}

autodecoder_node_00000000019e6800:
	if (instr_word & (1u << 24))
	{
		goto autodecoder_node_00000000019c12a0;
	}

autodecoder_node_00000000019e5100:
	if (instr_word & (1u << 4))
	{
		goto autodecoder_node_00000000019d5600;
	}
	goto autodecoder_node_00000000017cc240;

autodecoder_node_00000000019c12a0:
	if ((instr_word & (1u << 4)) == 0)
	{
		goto autodecoder_node_00000000017cc240;
	}

autodecoder_node_00000000018845c0:
	if (instr_word & (1u << 7))
	{
		goto autodecoder_node_00000000019e36a0;
	}

autodecoder_node_000000000185c960:
	if (instr_word & (1u << 23))
	{
		goto autodecoder_node_0000000001804c60;
	}

autodecoder_node_00000000019be740:
	if (instr_word & (1u << 21))
	{
		goto autodecoder_node_00000000017cc6c0;
	}

autodecoder_node_00000000019ca3e0:
	if (instr_word & (1u << 20))
	{
		goto autodecoder_node_00000000017d4040;
	}

autodecoder_node_00000000019da520:
	if (instr_word & (1u << 6))
	{
		goto autodecoder_node_00000000019d6c60;
	}

autodecoder_node_00000000019d9140:
	if (instr_word & (1u << 22))
	{
		goto autodecoder_node_00000000017f0c00;
	}

autodecoder_node_00000000017ee9e0:
	return &smladInstruction;

autodecoder_node_00000000019d6c60:
	if (instr_word & (1u << 22))
	{
		goto autodecoder_node_00000000017f5a40;
	}

autodecoder_node_00000000017f32e0:
	return &smlsdInstruction;

autodecoder_node_00000000017d4040:
	if (instr_word & (1u << 6))
	{
		goto autodecoder_node_00000000017cc6c0;
	}

autodecoder_node_00000000019cf0e0:
	if ((instr_word & (1u << 22)) == 0)
	{
		goto autodecoder_node_00000000017cc6c0;
	}

autodecoder_node_00000000017eab20:
	return &smmlaInstruction;

autodecoder_node_00000000019e7540:
	if ((instr_word & (1u << 25)) == 0)
	{
		goto autodecoder_node_00000000019d16a0;
	}

autodecoder_node_00000000019e8e40:
	if (instr_word & (1u << 24))
	{
		goto autodecoder_node_00000000019cdec0;
	}

autodecoder_node_000000000199d200:
	if ((instr_word & (1u << 4)) == 0)
	{
		goto autodecoder_node_00000000019d16a0;
	}

autodecoder_node_00000000019d5600:
	if (instr_word & (1u << 7))
	{
		goto autodecoder_node_00000000019e2a20;
	}

autodecoder_node_00000000019c1480:
	if (instr_word & (1u << 23))
	{
		goto autodecoder_node_00000000019ca5e0;
	}

autodecoder_node_00000000019e5de0:
	if ((instr_word & (1u << 8)) == 0)
	{
		goto autodecoder_node_00000000017cc6c0;
	}

autodecoder_node_00000000019f3fa0:
	if ((instr_word & (1u << 11)) == 0)
	{
		goto autodecoder_node_00000000017cc6c0;
	}

autodecoder_node_00000000019e0c00:
	if ((instr_word & (1u << 10)) == 0)
	{
		goto autodecoder_node_00000000017cc6c0;
	}

autodecoder_node_00000000019f4820:
	if ((instr_word & (1u << 9)) == 0)
	{
		goto autodecoder_node_00000000017cc6c0;
	}

autodecoder_node_00000000019dfaa0:
	if (instr_word & (1u << 21))
	{
		goto autodecoder_node_00000000019e8c00;
	}

autodecoder_node_00000000019f63c0:
	if ((instr_word & (1u << 20)) == 0)
	{
		goto autodecoder_node_00000000017cc6c0;
	}

autodecoder_node_00000000019d10a0:
	if (instr_word & (1u << 5))
	{
		goto autodecoder_node_00000000019ddee0;
	}

autodecoder_node_00000000019df3e0:
	if (instr_word & (1u << 6))
	{
		goto autodecoder_node_00000000019fa000;
	}

autodecoder_node_00000000019f7540:
	if (instr_word & (1u << 22))
	{
		goto autodecoder_node_000000000186efa0;
	}

autodecoder_node_0000000001872860:
	return &sadd16Instruction;

autodecoder_node_000000000186efa0:
	return &uadd16Instruction;

autodecoder_node_00000000019fa000:
	if (instr_word & (1u << 22))
	{
		goto autodecoder_node_0000000001863f40;
	}

autodecoder_node_0000000001867860:
	return &ssubaddxInstruction;

autodecoder_node_0000000001863f40:
	return &usubaddxInstruction;

autodecoder_node_00000000019ddee0:
	if (instr_word & (1u << 6))
	{
		goto autodecoder_node_00000000019cfa20;
	}

autodecoder_node_00000000019ea4a0:
	if (instr_word & (1u << 22))
	{
		goto autodecoder_node_0000000001861300;
	}

autodecoder_node_0000000001874c00:
	return &saddsubxInstruction;

autodecoder_node_0000000001861300:
	return &uaddsubxInstruction;

autodecoder_node_00000000019cfa20:
	if (instr_word & (1u << 22))
	{
		goto autodecoder_node_00000000018623a0;
	}

autodecoder_node_0000000001875cc0:
	return &ssub16Instruction;

autodecoder_node_00000000018623a0:
	return &usub16Instruction;

autodecoder_node_00000000019e8c00:
	if (instr_word & (1u << 20))
	{
		goto autodecoder_node_00000000019c3d80;
	}

autodecoder_node_00000000019f9ae0:
	if (instr_word & (1u << 5))
	{
		goto autodecoder_node_0000000001813920;
	}

autodecoder_node_00000000019c9f60:
	if (instr_word & (1u << 6))
	{
		goto autodecoder_node_00000000018fbc80;
	}

autodecoder_node_00000000019d02c0:
	if (instr_word & (1u << 22))
	{
		goto autodecoder_node_000000000185b3c0;
	}

autodecoder_node_000000000187bac0:
	return &qadd16Instruction;

autodecoder_node_000000000185b3c0:
	return &uqadd16Instruction;

autodecoder_node_00000000018fbc80:
	if (instr_word & (1u << 22))
	{
		goto autodecoder_node_0000000001826f00;
	}

autodecoder_node_0000000001871320:
	return &qsubaddxInstruction;

autodecoder_node_0000000001826f00:
	return &uqsubaddxInstruction;

autodecoder_node_0000000001813920:
	if (instr_word & (1u << 6))
	{
		goto autodecoder_node_0000000001867be0;
	}

autodecoder_node_00000000019dbf00:
	if (instr_word & (1u << 22))
	{
		goto autodecoder_node_000000000185d520;
	}

autodecoder_node_000000000187e240:
	return &qaddsubxInstruction;

autodecoder_node_000000000185d520:
	return &uqaddsubxInstruction;

autodecoder_node_0000000001867be0:
	if (instr_word & (1u << 22))
	{
		goto autodecoder_node_000000000185e520;
	}

autodecoder_node_000000000187f460:
	return &qsub16Instruction;

autodecoder_node_000000000185e520:
	return &uqsub16Instruction;

autodecoder_node_00000000019c3d80:
	if (instr_word & (1u << 5))
	{
		goto autodecoder_node_00000000019c5260;
	}

autodecoder_node_00000000019bf820:
	if (instr_word & (1u << 6))
	{
		goto autodecoder_node_00000000019db1a0;
	}

autodecoder_node_00000000019ea8a0:
	if (instr_word & (1u << 22))
	{
		goto autodecoder_node_0000000001865160;
	}

autodecoder_node_0000000001868b80:
	return &shadd16Instruction;

autodecoder_node_0000000001865160:
	return &uhadd16Instruction;

autodecoder_node_00000000019db1a0:
	if (instr_word & (1u << 22))
	{
		goto autodecoder_node_000000000185a0c0;
	}

autodecoder_node_000000000186dc60:
	return &shsubaddxInstruction;

autodecoder_node_000000000185a0c0:
	return &uhsubaddxInstruction;

autodecoder_node_00000000019c5260:
	if (instr_word & (1u << 6))
	{
		goto autodecoder_node_00000000018972c0;
	}

autodecoder_node_00000000019bfa00:
	if (instr_word & (1u << 22))
	{
		goto autodecoder_node_0000000001867420;
	}

autodecoder_node_000000000186b000:
	return &shaddsubxInstruction;

autodecoder_node_0000000001867420:
	return &uhaddsubxInstruction;

autodecoder_node_00000000018972c0:
	if (instr_word & (1u << 22))
	{
		goto autodecoder_node_0000000001858500;
	}

autodecoder_node_000000000186c0c0:
	return &shsub16Instruction;

autodecoder_node_0000000001858500:
	return &uhsub16Instruction;

autodecoder_node_00000000019ca5e0:
	if (instr_word & (1u << 8))
	{
		goto autodecoder_node_0000000001882700;
	}

autodecoder_node_00000000019df1c0:
	if (instr_word & (1u << 9))
	{
		goto autodecoder_node_00000000019f4280;
	}

autodecoder_node_00000000019c8fc0:
	if (instr_word & (1u << 21))
	{
		goto autodecoder_node_00000000019e58c0;
	}

autodecoder_node_00000000019f6f40:
	if (instr_word & (1u << 20))
	{
		goto autodecoder_node_00000000017cc6c0;
	}

autodecoder_node_00000000019f9fc0:
	if ((instr_word & (1u << 16)) == 0)
	{
		goto autodecoder_node_00000000019f9080;
	}

autodecoder_node_00000000019c73e0:
	if ((instr_word & (1u << 19)) == 0)
	{
		goto autodecoder_node_00000000019f9080;
	}

autodecoder_node_00000000019cce80:
	if ((instr_word & (1u << 17)) == 0)
	{
		goto autodecoder_node_00000000019f9080;
	}

autodecoder_node_00000000019ce080:
	if (instr_word & (1u << 18))
	{
		goto autodecoder_node_00000000019c03c0;
	}

autodecoder_node_00000000019f9080:
	if ((instr_word & (1u << 5)) == 0)
	{
		goto autodecoder_node_00000000019d4fc0;
	}

autodecoder_node_00000000019d7ba0:
	if ((instr_word & (1u << 6)) == 0)
	{
		goto autodecoder_node_00000000017cc6c0;
	}

autodecoder_node_00000000019b9840:
	if (instr_word & (1u << 22))
	{
		goto autodecoder_node_0000000001801f40;
	}

autodecoder_node_00000000018163c0:
	return &sxtab16Instruction;

autodecoder_node_0000000001801f40:
	return &uxtab16Instruction;

autodecoder_node_00000000019c03c0:
	if ((instr_word & (1u << 5)) == 0)
	{
		goto autodecoder_node_00000000019d4fc0;
	}

autodecoder_node_00000000019d1280:
	if ((instr_word & (1u << 6)) == 0)
	{
		goto autodecoder_node_00000000017cc6c0;
	}

autodecoder_node_00000000019dd480:
	if (instr_word & (1u << 22))
	{
		goto autodecoder_node_00000000017cc240;
	}

autodecoder_node_0000000001820a60:
	return &sxtb16Instruction;

autodecoder_node_00000000019e58c0:
	if (instr_word & (1u << 20))
	{
		goto autodecoder_node_00000000019eed80;
	}

autodecoder_node_00000000019de160:
	if ((instr_word & (1u << 16)) == 0)
	{
		goto autodecoder_node_00000000019e6d60;
	}

autodecoder_node_0000000001872c60:
	if ((instr_word & (1u << 19)) == 0)
	{
		goto autodecoder_node_00000000019e6d60;
	}

autodecoder_node_00000000019e3920:
	if ((instr_word & (1u << 17)) == 0)
	{
		goto autodecoder_node_00000000019e6d60;
	}

autodecoder_node_00000000019c25c0:
	if (instr_word & (1u << 18))
	{
		goto autodecoder_node_00000000019d3100;
	}

autodecoder_node_00000000019e6d60:
	if ((instr_word & (1u << 5)) == 0)
	{
		goto autodecoder_node_00000000019e05c0;
	}

autodecoder_node_00000000019e49e0:
	if ((instr_word & (1u << 6)) == 0)
	{
		goto autodecoder_node_00000000017cc6c0;
	}

autodecoder_node_00000000019c8960:
	if (instr_word & (1u << 22))
	{
		goto autodecoder_node_00000000018059a0;
	}

autodecoder_node_000000000180a560:
	return &sxtabInstruction;

autodecoder_node_00000000018059a0:
	return &uxtabInstruction;

autodecoder_node_00000000019d3100:
	if (instr_word & (1u << 5))
	{
		goto autodecoder_node_00000000019ed940;
	}
	goto autodecoder_node_00000000019e05c0;

autodecoder_node_00000000019eed80:
	if ((instr_word & (1u << 16)) == 0)
	{
		goto autodecoder_node_00000000019f3ac0;
	}

autodecoder_node_00000000019c40c0:
	if ((instr_word & (1u << 19)) == 0)
	{
		goto autodecoder_node_00000000019f3ac0;
	}

autodecoder_node_00000000019f9720:
	if ((instr_word & (1u << 17)) == 0)
	{
		goto autodecoder_node_00000000019f3ac0;
	}

autodecoder_node_000000000194bac0:
	if (instr_word & (1u << 18))
	{
		goto autodecoder_node_00000000019e2b60;
	}

autodecoder_node_00000000019f3ac0:
	if ((instr_word & (1u << 5)) == 0)
	{
		goto autodecoder_node_00000000019e05c0;
	}

autodecoder_node_00000000018c7440:
	if ((instr_word & (1u << 6)) == 0)
	{
		goto autodecoder_node_00000000017cc6c0;
	}

autodecoder_node_00000000019e9340:
	if (instr_word & (1u << 22))
	{
		goto autodecoder_node_00000000017fe6e0;
	}

autodecoder_node_0000000001812040:
	return &sxtahInstruction;

autodecoder_node_00000000017fe6e0:
	return &uxtahInstruction;

autodecoder_node_00000000019e2b60:
	if ((instr_word & (1u << 5)) == 0)
	{
		goto autodecoder_node_00000000019e05c0;
	}

autodecoder_node_00000000019ca6e0:
	if ((instr_word & (1u << 6)) == 0)
	{
		goto autodecoder_node_00000000017cc6c0;
	}

autodecoder_node_00000000019daac0:
	if (instr_word & (1u << 22))
	{
		goto autodecoder_node_00000000017cc240;
	}

autodecoder_node_000000000182c060:
	return &sxthInstruction;

autodecoder_node_0000000001882700:
	if ((instr_word & (1u << 11)) == 0)
	{
		goto autodecoder_node_00000000019f4280;
	}

autodecoder_node_00000000019c5aa0:
	if ((instr_word & (1u << 10)) == 0)
	{
		goto autodecoder_node_00000000019f4280;
	}

autodecoder_node_000000000181fb40:
	if ((instr_word & (1u << 9)) == 0)
	{
		goto autodecoder_node_00000000019f4280;
	}

autodecoder_node_00000000019d0480:
	if ((instr_word & (1u << 21)) == 0)
	{
		goto autodecoder_node_00000000019d5fe0;
	}

autodecoder_node_00000000019ef200:
	if (instr_word & (1u << 20))
	{
		goto autodecoder_node_00000000019d7f20;
	}

autodecoder_node_00000000019de460:
	if ((instr_word & (1u << 5)) == 0)
	{
		goto autodecoder_node_00000000019e05c0;
	}

autodecoder_node_00000000019d39c0:
	if (instr_word & (1u << 6))
	{
		goto autodecoder_node_00000000017cc6c0;
	}

autodecoder_node_00000000019e4cc0:
	if (instr_word & (1u << 22))
	{
		goto autodecoder_node_00000000018490a0;
	}

autodecoder_node_00000000017d63c0:
	return &ssat16Instruction;

autodecoder_node_00000000018490a0:
	return &usat16Instruction;

autodecoder_node_00000000019d7f20:
	if ((instr_word & (1u << 16)) == 0)
	{
		goto autodecoder_node_00000000019c4b40;
	}

autodecoder_node_00000000019d41e0:
	if ((instr_word & (1u << 19)) == 0)
	{
		goto autodecoder_node_00000000019c4b40;
	}

autodecoder_node_00000000019e4d80:
	if ((instr_word & (1u << 17)) == 0)
	{
		goto autodecoder_node_00000000019c4b40;
	}

autodecoder_node_00000000019e57c0:
	if ((instr_word & (1u << 18)) == 0)
	{
		goto autodecoder_node_00000000019c4b40;
	}

autodecoder_node_00000000019cb260:
	if ((instr_word & (1u << 5)) == 0)
	{
		goto autodecoder_node_00000000019e05c0;
	}

autodecoder_node_00000000018adb60:
	if (instr_word & (1u << 6))
	{
		goto autodecoder_node_00000000017cc6c0;
	}

autodecoder_node_00000000019ddb20:
	if (instr_word & (1u << 22))
	{
		goto autodecoder_node_000000000187fa20;
	}

autodecoder_node_00000000018283c0:
	return &revInstruction;

autodecoder_node_000000000187fa20:
	return &rbitInstruction;

autodecoder_node_00000000019e2a20:
	if (instr_word & (1u << 23))
	{
		goto autodecoder_node_00000000019cd460;
	}

autodecoder_node_00000000019e46c0:
	if ((instr_word & (1u << 8)) == 0)
	{
		goto autodecoder_node_00000000017cc6c0;
	}

autodecoder_node_00000000019c4b60:
	if ((instr_word & (1u << 11)) == 0)
	{
		goto autodecoder_node_00000000017cc6c0;
	}

autodecoder_node_00000000019f99a0:
	if ((instr_word & (1u << 10)) == 0)
	{
		goto autodecoder_node_00000000017cc6c0;
	}

autodecoder_node_00000000019c85c0:
	if ((instr_word & (1u << 9)) == 0)
	{
		goto autodecoder_node_00000000017cc6c0;
	}

autodecoder_node_00000000019c2e20:
	if (instr_word & (1u << 21))
	{
		goto autodecoder_node_0000000001899960;
	}

autodecoder_node_00000000019da6a0:
	if ((instr_word & (1u << 20)) == 0)
	{
		goto autodecoder_node_00000000017cc6c0;
	}

autodecoder_node_00000000019ce300:
	if (instr_word & (1u << 5))
	{
		goto autodecoder_node_0000000001873ee0;
	}

autodecoder_node_000000000186d2c0:
	if (instr_word & (1u << 6))
	{
		goto autodecoder_node_00000000017cc6c0;
	}

autodecoder_node_00000000019eafa0:
	if (instr_word & (1u << 22))
	{
		goto autodecoder_node_00000000018601e0;
	}

autodecoder_node_0000000001873b40:
	return &sadd8Instruction;

autodecoder_node_00000000018601e0:
	return &uadd8Instruction;

autodecoder_node_0000000001873ee0:
	if ((instr_word & (1u << 6)) == 0)
	{
		goto autodecoder_node_00000000017cc6c0;
	}

autodecoder_node_00000000019e6e20:
	if (instr_word & (1u << 22))
	{
		goto autodecoder_node_00000000018632a0;
	}

autodecoder_node_0000000001876b40:
	return &ssub8Instruction;

autodecoder_node_00000000018632a0:
	return &usub8Instruction;

autodecoder_node_0000000001899960:
	if (instr_word & (1u << 20))
	{
		goto autodecoder_node_00000000019eb1c0;
	}

autodecoder_node_00000000019ce900:
	if (instr_word & (1u << 5))
	{
		goto autodecoder_node_00000000019c0b80;
	}

autodecoder_node_00000000019f75c0:
	if (instr_word & (1u << 6))
	{
		goto autodecoder_node_00000000017cc6c0;
	}

autodecoder_node_00000000019defc0:
	if (instr_word & (1u << 22))
	{
		goto autodecoder_node_000000000185c560;
	}

autodecoder_node_000000000187d100:
	return &qadd8Instruction;

autodecoder_node_000000000185c560:
	return &uqadd8Instruction;

autodecoder_node_00000000019c0b80:
	if ((instr_word & (1u << 6)) == 0)
	{
		goto autodecoder_node_00000000017cc6c0;
	}

autodecoder_node_00000000019d8860:
	if (instr_word & (1u << 22))
	{
		goto autodecoder_node_000000000185f380;
	}

autodecoder_node_00000000018704e0:
	return &qsub8Instruction;

autodecoder_node_000000000185f380:
	return &uqsub8Instruction;

autodecoder_node_00000000019eb1c0:
	if (instr_word & (1u << 5))
	{
		goto autodecoder_node_00000000019d22e0;
	}

autodecoder_node_00000000019d2780:
	if (instr_word & (1u << 6))
	{
		goto autodecoder_node_00000000017cc6c0;
	}

autodecoder_node_00000000019f4c60:
	if (instr_word & (1u << 22))
	{
		goto autodecoder_node_0000000001866300;
	}

autodecoder_node_0000000001869d80:
	return &shadd8Instruction;

autodecoder_node_0000000001866300:
	return &uhadd8Instruction;

autodecoder_node_00000000019d22e0:
	if ((instr_word & (1u << 6)) == 0)
	{
		goto autodecoder_node_00000000017cc6c0;
	}

autodecoder_node_00000000019d6aa0:
	if (instr_word & (1u << 22))
	{
		goto autodecoder_node_0000000001859380;
	}

autodecoder_node_000000000186cf40:
	return &shsub8Instruction;

autodecoder_node_0000000001859380:
	return &uhsub8Instruction;

autodecoder_node_00000000019cd460:
	if ((instr_word & (1u << 8)) == 0)
	{
		goto autodecoder_node_00000000019f4280;
	}

autodecoder_node_00000000019dd380:
	if ((instr_word & (1u << 11)) == 0)
	{
		goto autodecoder_node_00000000019f4280;
	}

autodecoder_node_00000000019f58e0:
	if ((instr_word & (1u << 10)) == 0)
	{
		goto autodecoder_node_00000000019f4280;
	}

autodecoder_node_00000000019d0b60:
	if (instr_word & (1u << 9))
	{
		goto autodecoder_node_00000000019def20;
	}

autodecoder_node_00000000019f4280:
	if (instr_word & (1u << 21))
	{
		goto autodecoder_node_00000000019c4b40;
	}

autodecoder_node_00000000019d5fe0:
	if (instr_word & (1u << 20))
	{
		goto autodecoder_node_00000000017cc6c0;
	}

autodecoder_node_00000000019f9920:
	if (instr_word & (1u << 5))
	{
		goto autodecoder_node_00000000017cc6c0;
	}
	goto autodecoder_node_00000000019d4fc0;

autodecoder_node_00000000019def20:
	if (instr_word & (1u << 21))
	{
		goto autodecoder_node_00000000019cdbe0;
	}

autodecoder_node_00000000019e8320:
	if (instr_word & (1u << 20))
	{
		goto autodecoder_node_00000000017cc6c0;
	}

autodecoder_node_00000000019cac60:
	if (instr_word & (1u << 5))
	{
		goto autodecoder_node_00000000019d1000;
	}

autodecoder_node_00000000019d4fc0:
	if (instr_word & (1u << 6))
	{
		goto autodecoder_node_00000000019ce760;
	}

autodecoder_node_00000000019df3a0:
	if (instr_word & (1u << 22))
	{
		goto autodecoder_node_00000000017cc6c0;
	}

autodecoder_node_0000000001884040:
	return &pkhbtInstruction;

autodecoder_node_00000000019ce760:
	if (instr_word & (1u << 22))
	{
		goto autodecoder_node_00000000017cc6c0;
	}

autodecoder_node_0000000001877dc0:
	return &pkhtbInstruction;

autodecoder_node_00000000019d1000:
	if (instr_word & (1u << 6))
	{
		goto autodecoder_node_00000000017cc6c0;
	}

autodecoder_node_00000000019c50c0:
	if (instr_word & (1u << 22))
	{
		goto autodecoder_node_00000000017cc6c0;
	}

autodecoder_node_00000000017f95e0:
	return &selInstruction;

autodecoder_node_00000000019cdbe0:
	if ((instr_word & (1u << 20)) == 0)
	{
		goto autodecoder_node_00000000019c4b40;
	}

autodecoder_node_00000000019e22c0:
	if ((instr_word & (1u << 16)) == 0)
	{
		goto autodecoder_node_00000000019c4b40;
	}

autodecoder_node_00000000019eae40:
	if ((instr_word & (1u << 19)) == 0)
	{
		goto autodecoder_node_00000000019c4b40;
	}

autodecoder_node_00000000017fed20:
	if ((instr_word & (1u << 17)) == 0)
	{
		goto autodecoder_node_00000000019c4b40;
	}

autodecoder_node_000000000189fb20:
	if (instr_word & (1u << 18))
	{
		goto autodecoder_node_00000000019f7d80;
	}

autodecoder_node_00000000019c4b40:
	if (instr_word & (1u << 5))
	{
		goto autodecoder_node_00000000017cc6c0;
	}
	goto autodecoder_node_00000000019e05c0;

autodecoder_node_00000000019f7d80:
	if (instr_word & (1u << 5))
	{
		goto autodecoder_node_00000000019eb5e0;
	}

autodecoder_node_00000000019e05c0:
	if (instr_word & (1u << 22))
	{
		goto autodecoder_node_00000000017d7ae0;
	}

autodecoder_node_00000000017de100:
	return &ssatInstruction;

autodecoder_node_00000000017d7ae0:
	return &usatInstruction;

autodecoder_node_00000000019eb5e0:
	if (instr_word & (1u << 6))
	{
		goto autodecoder_node_00000000017cc6c0;
	}

autodecoder_node_00000000019d51c0:
	if (instr_word & (1u << 22))
	{
		goto autodecoder_node_000000000182a8c0;
	}

autodecoder_node_0000000001829960:
	return &rev16Instruction;

autodecoder_node_000000000182a8c0:
	return &revshInstruction;

autodecoder_node_00000000019cdec0:
	if (instr_word & (1u << 4))
	{
		goto autodecoder_node_00000000019e89a0;
	}

autodecoder_node_00000000019d16a0:
	if ((instr_word & (1u << 20)) == 0)
	{
		goto autodecoder_node_00000000017cc240;
	}

autodecoder_node_00000000018ad480:
	return &ldrInstruction;

autodecoder_node_00000000019e89a0:
	if (instr_word & (1u << 7))
	{
		goto autodecoder_node_00000000019e36a0;
	}

autodecoder_node_00000000019d5b60:
	if (instr_word & (1u << 23))
	{
		goto autodecoder_node_0000000001804c60;
	}

autodecoder_node_00000000019e9dc0:
	if (instr_word & (1u << 21))
	{
		goto autodecoder_node_00000000017cc6c0;
	}

autodecoder_node_000000000186f820:
	if (instr_word & (1u << 20))
	{
		goto autodecoder_node_00000000019db420;
	}

autodecoder_node_00000000019c6520:
	if (instr_word & (1u << 6))
	{
		goto autodecoder_node_00000000019eb9c0;
	}

autodecoder_node_00000000019f3c00:
	if (instr_word & (1u << 22))
	{
		goto autodecoder_node_00000000017f0c00;
	}

autodecoder_node_00000000017fae40:
	return &smuadInstruction;

autodecoder_node_00000000017f0c00:
	return &smlaldInstruction;

autodecoder_node_00000000019eb9c0:
	if (instr_word & (1u << 22))
	{
		goto autodecoder_node_00000000017f5a40;
	}

autodecoder_node_00000000017fcc20:
	return &smusdInstruction;

autodecoder_node_00000000017f5a40:
	return &smlsldInstruction;

autodecoder_node_00000000019db420:
	if (instr_word & (1u << 6))
	{
		goto autodecoder_node_00000000017cc6c0;
	}

autodecoder_node_00000000019e43e0:
	if ((instr_word & (1u << 22)) == 0)
	{
		goto autodecoder_node_00000000017cc6c0;
	}

autodecoder_node_00000000017e8c80:
	return &smmulInstruction;

autodecoder_node_00000000019e36a0:
	if (instr_word & (1u << 23))
	{
		goto autodecoder_node_0000000001804c60;
	}

autodecoder_node_00000000019f5f40:
	if (instr_word & (1u << 21))
	{
		goto autodecoder_node_00000000017cc6c0;
	}

autodecoder_node_00000000019d0b40:
	if ((instr_word & (1u << 20)) == 0)
	{
		goto autodecoder_node_00000000017cc6c0;
	}

autodecoder_node_00000000019d17e0:
	if ((instr_word & (1u << 6)) == 0)
	{
		goto autodecoder_node_00000000017cc6c0;
	}

autodecoder_node_00000000019d34c0:
	if ((instr_word & (1u << 22)) == 0)
	{
		goto autodecoder_node_00000000017cc6c0;
	}

autodecoder_node_00000000017ecce0:
	return &smmlsInstruction;

autodecoder_node_0000000001804c60:
	if (instr_word & (1u << 21))
	{
		goto autodecoder_node_00000000019da140;
	}

autodecoder_node_00000000019fa0e0:
	if (instr_word & (1u << 5))
	{
		goto autodecoder_node_00000000017cc6c0;
	}

autodecoder_node_00000000019e4ba0:
	if (instr_word & (1u << 6))
	{
		goto autodecoder_node_00000000017cc6c0;
	}

autodecoder_node_00000000019f77e0:
	if (instr_word & (1u << 22))
	{
		goto autodecoder_node_00000000017cc240;
	}
	goto autodecoder_node_00000000017cc6c0;

autodecoder_node_00000000019da140:
	if (instr_word & (1u << 5))
	{
		goto autodecoder_node_00000000017cc6c0;
	}

autodecoder_node_00000000019ed940:
	if ((instr_word & (1u << 6)) == 0)
	{
		goto autodecoder_node_00000000017cc6c0;
	}

autodecoder_node_00000000017cc240:
	return 0;

autodecoder_node_00000000017d40a0:
	if ((instr_word & (1u << 25)) == 0)
	{
		goto autodecoder_node_00000000017cc6c0;
	}

autodecoder_node_00000000019d20a0:
	if (instr_word & (1u << 24))
	{
		goto autodecoder_node_000000000198f3e0;
	}

autodecoder_node_00000000019eef80:
	if ((instr_word & (1u << 4)) == 0)
	{
		goto autodecoder_node_00000000017cc6c0;
	}

autodecoder_node_00000000019e2d40:
	if (instr_word & (1u << 20))
	{
		goto autodecoder_node_00000000019b07c0;
	}

autodecoder_node_00000000019c55c0:
	return &mcrInstruction;

autodecoder_node_00000000019b07c0:
	return &mrcInstruction;

autodecoder_node_000000000198f3e0:
	return &svcInstruction;

autodecoder_node_00000000019cdba0:
	if (instr_word & (1u << 27))
	{
		goto autodecoder_node_00000000017cc6c0;
	}

autodecoder_node_00000000019f5f20:
	if ((instr_word & (1u << 12)) == 0)
	{
		goto autodecoder_node_00000000017cc6c0;
	}

autodecoder_node_00000000019ecf60:
	if ((instr_word & (1u << 13)) == 0)
	{
		goto autodecoder_node_00000000017cc6c0;
	}

autodecoder_node_00000000019e3520:
	if ((instr_word & (1u << 14)) == 0)
	{
		goto autodecoder_node_00000000017cc6c0;
	}

autodecoder_node_00000000019cf060:
	if ((instr_word & (1u << 15)) == 0)
	{
		goto autodecoder_node_00000000017cc6c0;
	}

autodecoder_node_00000000019db740:
	if (instr_word & (1u << 25))
	{
		goto autodecoder_node_00000000019e0840;
	}

autodecoder_node_00000000019e3d20:
	if ((instr_word & (1u << 24)) == 0)
	{
		goto autodecoder_node_00000000019d8ae0;
	}

autodecoder_node_00000000019ce480:
	if (instr_word & (1u << 4))
	{
		goto autodecoder_node_00000000019d2aa0;
	}

autodecoder_node_00000000019e6160:
	if (instr_word & (1u << 7))
	{
		goto autodecoder_node_00000000019d7380;
	}

autodecoder_node_00000000019c66c0:
	if (instr_word & (1u << 23))
	{
		goto autodecoder_node_00000000019d7380;
	}

autodecoder_node_00000000019ef0e0:
	if (instr_word & (1u << 8))
	{
		goto autodecoder_node_00000000019d7380;
	}

autodecoder_node_00000000019dac60:
	if (instr_word & (1u << 11))
	{
		goto autodecoder_node_00000000019d7380;
	}

autodecoder_node_00000000019eb620:
	if (instr_word & (1u << 10))
	{
		goto autodecoder_node_00000000019d7380;
	}

autodecoder_node_00000000019ebca0:
	if (instr_word & (1u << 9))
	{
		goto autodecoder_node_00000000019d7380;
	}

autodecoder_node_00000000019d83e0:
	if ((instr_word & (1u << 21)) == 0)
	{
		goto autodecoder_node_00000000019c83a0;
	}

autodecoder_node_00000000019ebec0:
	if ((instr_word & (1u << 20)) == 0)
	{
		goto autodecoder_node_00000000017cc6c0;
	}

autodecoder_node_00000000019e71a0:
	if ((instr_word & (1u << 16)) == 0)
	{
		goto autodecoder_node_00000000017cc6c0;
	}

autodecoder_node_00000000019d6460:
	if ((instr_word & (1u << 19)) == 0)
	{
		goto autodecoder_node_00000000017cc6c0;
	}

autodecoder_node_00000000019f7880:
	if ((instr_word & (1u << 17)) == 0)
	{
		goto autodecoder_node_00000000017cc6c0;
	}

autodecoder_node_00000000019d9de0:
	if ((instr_word & (1u << 18)) == 0)
	{
		goto autodecoder_node_00000000017cc6c0;
	}

autodecoder_node_00000000019dbfe0:
	if (instr_word & (1u << 5))
	{
		goto autodecoder_node_00000000019bf860;
	}

autodecoder_node_00000000019da980:
	if ((instr_word & (1u << 6)) == 0)
	{
		goto autodecoder_node_00000000017cc6c0;
	}

autodecoder_node_00000000019c0d20:
	if ((instr_word & (1u << 22)) == 0)
	{
		goto autodecoder_node_00000000017cc6c0;
	}

autodecoder_node_00000000017cfac0:
	return &dsbInstruction;

autodecoder_node_00000000019bf860:
	if ((instr_word & (1u << 6)) == 0)
	{
		goto autodecoder_node_00000000017cc6c0;
	}

autodecoder_node_00000000019e3500:
	if ((instr_word & (1u << 22)) == 0)
	{
		goto autodecoder_node_00000000017cc6c0;
	}

autodecoder_node_00000000017d04a0:
	return &isbInstruction;

autodecoder_node_00000000019d2aa0:
	if (instr_word & (1u << 7))
	{
		goto autodecoder_node_00000000019d7380;
	}

autodecoder_node_00000000019dc140:
	if (instr_word & (1u << 23))
	{
		goto autodecoder_node_00000000019d7380;
	}

autodecoder_node_00000000019eca80:
	if (instr_word & (1u << 8))
	{
		goto autodecoder_node_00000000019d7380;
	}

autodecoder_node_00000000019e1200:
	if (instr_word & (1u << 11))
	{
		goto autodecoder_node_00000000019d7380;
	}

autodecoder_node_00000000019c2b20:
	if (instr_word & (1u << 10))
	{
		goto autodecoder_node_00000000019d7380;
	}

autodecoder_node_00000000019d3a40:
	if (instr_word & (1u << 9))
	{
		goto autodecoder_node_00000000019d7380;
	}

autodecoder_node_00000000019c8cc0:
	if ((instr_word & (1u << 21)) == 0)
	{
		goto autodecoder_node_00000000019c83a0;
	}

autodecoder_node_00000000019def60:
	if ((instr_word & (1u << 20)) == 0)
	{
		goto autodecoder_node_00000000017cc6c0;
	}

autodecoder_node_00000000019d6bc0:
	if ((instr_word & (1u << 16)) == 0)
	{
		goto autodecoder_node_00000000017cc6c0;
	}

autodecoder_node_00000000019f8200:
	if ((instr_word & (1u << 19)) == 0)
	{
		goto autodecoder_node_00000000017cc6c0;
	}

autodecoder_node_00000000018999a0:
	if ((instr_word & (1u << 17)) == 0)
	{
		goto autodecoder_node_00000000017cc6c0;
	}

autodecoder_node_00000000019d3140:
	if ((instr_word & (1u << 18)) == 0)
	{
		goto autodecoder_node_00000000017cc6c0;
	}

autodecoder_node_00000000019d7fa0:
	if (instr_word & (1u << 5))
	{
		goto autodecoder_node_00000000017cc6c0;
	}

autodecoder_node_00000000019cb440:
	if (instr_word & (1u << 6))
	{
		goto autodecoder_node_0000000001876040;
	}

autodecoder_node_00000000019ecf20:
	if ((instr_word & (1u << 3)) == 0)
	{
		goto autodecoder_node_00000000017cc6c0;
	}

autodecoder_node_00000000019c1240:
	if ((instr_word & (1u << 0)) == 0)
	{
		goto autodecoder_node_00000000017cc6c0;
	}

autodecoder_node_00000000019dc980:
	if ((instr_word & (1u << 2)) == 0)
	{
		goto autodecoder_node_00000000017cc6c0;
	}

autodecoder_node_00000000019e9660:
	if ((instr_word & (1u << 1)) == 0)
	{
		goto autodecoder_node_00000000017cc6c0;
	}

autodecoder_node_00000000019d9640:
	if ((instr_word & (1u << 22)) == 0)
	{
		goto autodecoder_node_00000000017cc6c0;
	}

autodecoder_node_00000000017d1ee0:
	return &clrexInstruction;

autodecoder_node_0000000001876040:
	if ((instr_word & (1u << 22)) == 0)
	{
		goto autodecoder_node_00000000017cc6c0;
	}

autodecoder_node_00000000017ced20:
	return &dmbInstruction;

autodecoder_node_00000000019e0840:
	if (instr_word & (1u << 24))
	{
		goto autodecoder_node_00000000019c00e0;
	}

autodecoder_node_00000000019d77e0:
	if (instr_word & (1u << 4))
	{
		goto autodecoder_node_00000000017cc6c0;
	}

autodecoder_node_00000000019d8ae0:
	if (instr_word & (1u << 21))
	{
		goto autodecoder_node_00000000017cc6c0;
	}

autodecoder_node_00000000019eba60:
	if ((instr_word & (1u << 20)) == 0)
	{
		goto autodecoder_node_00000000017cc6c0;
	}

autodecoder_node_00000000019caa40:
	if ((instr_word & (1u << 22)) == 0)
	{
		goto autodecoder_node_00000000017cc6c0;
	}

autodecoder_node_00000000017dbbc0:
	return &pliInstruction;

autodecoder_node_00000000019c00e0:
	if (instr_word & (1u << 4))
	{
		goto autodecoder_node_00000000017cc6c0;
	}

autodecoder_node_00000000019d7380:
	if (instr_word & (1u << 21))
	{
		goto autodecoder_node_00000000017cc6c0;
	}

autodecoder_node_00000000019c83a0:
	if ((instr_word & (1u << 20)) == 0)
	{
		goto autodecoder_node_00000000017cc6c0;
	}

autodecoder_node_00000000017d8080:
	return &pldInstruction;

autodecoder_node_00000000017cc6c0:
	DIE_NOW(0, "autoDecoder: undefined instruction");

