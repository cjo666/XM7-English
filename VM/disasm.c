/*
 *	FM-7 EMULATOR "XM7"
 *
 *	Copyright (C) 1999-2017 �o�h�D(Twitter:@xm6_original)
 *	Copyright (C) 2001-2017 Ryu Takegami (Twitter:@RyuTakegami)
 *
 *	[ �t�A�Z���u�� ]
 *
 *	RHG����
 *	  2003.02.11		�T�C�Y�k���ɖ��ʂȓw�͂����Ă݂�(��
 *	  2003.11.21		���{��T�uCPU�Ή�
 *	  2008.01.27		�C���w�����gM�n�e�[�u�����B�����ߑΉ��ɕύX
 *						�v���o�C�g�t��LBRA/LBSR�ɑΉ�
 */

#include <string.h>
#include "xm7.h"
#if XM7_VER == 1
#if defined(JSUB)
#include "jsubsys.h"
#endif
#endif

/*
 *	�X�^�e�B�b�N ���[�N
 */
static int cputype;					/* CPU��� */
static BYTE opc;					/* �I�y�R�[�h */
static WORD pc;						/* ���s�OPC */
static WORD addpc;					/* PC���Z�l(���ߒ�) */
static char linebuf[32];			/* �t�A�Z���u���o�̓o�b�t�@ */

/*
 *	��O�n1(0x00)�E�C���w�����gA(0x40)�E�C���w�����gB(0x50)�e�[�u��
 *	  �C���w�����g���[�h���̓��W�X�^�����ǉ������
 *	  0x0e �̂ݗ�O
 */
static const char *inh_tbl[] = {
	"NEG",
	"NEG",
	"NGC",
	"COM",
	"LSR",
	"LSR",
	"ROR",
	"ASR",
	"LSL",
	"ROL",
	"DEC",
	"DCC",
	"INC",
	"TST",
	"CLC",	/* 0x0e = JMP */
	"CLR"
};

/*
 *	��O�n2(0x10)�e�[�u��
 */
static const char *except2_tbl[] = {
	NULL,
	NULL,
	"NOP",
	"SYNC",
	"HCF",
	"HCF",
	"LBRA",
	"LBSR",
	"ASLCC",
	"DAA",
	"ORCC",
	"NOP",
	"ANDCC",
	"SEX",
	"EXG",
	"TFR"
};

/*
 *	�u�����`�n(0x20)�E�����O�u�����`�n(0x10 0x20)�e�[�u��
 *	  �����O�u�����`���͐擪�� L ���ǉ������
 */
static const char *branch_tbl[] = {
	"BRA",
	"BRN",
	"BHI",
	"BLS",
	"BCC",
	"BCS",
	"BNE",
	"BEQ",
	"BVC",
	"BVS",
	"BPL",
	"BMI",
	"BGE",
	"BLT",
	"BGT",
	"BLE"
};

/*
 *	LEA�A�X�^�b�N�n(0x30)�e�[�u��
 */
static const char *leastack_tbl[] = {
	"LEAX",
	"LEAY",
	"LEAS",
	"LEAU",
	"PSHS",
	"PULS",
	"PSHU",
	"PULU",
	"ANDCC",
	"RTS",
	"ABX",
	"RTI",
	"CWAI",
	"MUL",
	"RST",
	"SWI"
};

/*
 *	�C���w�����gM(0x60, 0x70)�e�[�u��
 */
static const char *inhm_tbl[] = {
	"NEG",
	"NEG",
	"NGC",
	"COM",
	"LSR",
	"LSR",
	"ROR",
	"ASR",
	"LSL",
	"ROL",
	"DEC",
	"DCC",
	"INC",
	"TST",
	"JMP",	/* 0x6e, 0x7e = JMP */
	"CLR"
};

/*
 *	A���W�X�^�AX���W�X�^�n(0x80)�e�[�u��
 */
static const char *regax_tbl[] = {
	"SUBA",
	"CMPA",
	"SBCA",
	"SUBD",
	"ANDA",
	"BITA",
	"LDA",
	"STA",
	"EORA",
	"ADCA",
	"ORA",
	"ADDA",
	"CMPX",
	"JSR",
	"LDX",
	"STX"
};

/*
 *	B���W�X�^�AD���W�X�^�AU���W�X�^�n(0xc0)�e�[�u��
 */
static const char *regbdu_tbl[] = {
	"SUBB",
	"CMPB",
	"SBCB",
	"ADDD",
	"ANDB",
	"BITB",
	"LDB",
	"STB",
	"EORB",
	"ADCB",
	"ORB",
	"ADDB",
	"LDD",
	"STD",
	"LDU",
	"STU"
};

/*
 *	TFR/EXG�e�[�u�� (6809)
 */
static char *tfrexg_tbl[] = {
	"D",
	"X",
	"Y",
	"U",
	"S",
	"PC",
	NULL,
	NULL,
	"A",
	"B",
	"CC",
	"DP",
	NULL,
	NULL
};

/*
 *	PSH/PUL�e�[�u��
 */
static char *pshpul_tbl[] = {
	"CC",
	"A",
	"B",
	"DP",
	"X",
	"Y",
	NULL,	/* S or U */
	"PC"
};

/*
 *	�C���f�b�N�X�e�[�u��
 */
static char *idx_tbl[] = {
	"X",
	"Y",
	"U",
	"S"
};

/*-[ �ėp�T�u ]-------------------------------------------------------------*/

/*
 *	�f�[�^�t�F�b�`
 */
static BYTE FASTCALL fetch(void)
{
	BYTE dat;

	switch (cputype) {
		case MAINCPU:
			dat = mainmem_readbnio((WORD)(pc + addpc));
			break;

		case SUBCPU:
			dat = submem_readbnio((WORD)(pc + addpc));
			break;

#if XM7_VER == 1
#if defined(JSUB)
		case JSUBCPU:
			dat = jsubmem_readbnio((WORD)(pc + addpc));
			break;
#endif
#endif

		default:
			ASSERT(FALSE);
			break;
	}

	addpc++;
	return dat;
}

/*
 *	�f�[�^�ǂݏo��
 */
static BYTE FASTCALL read_byte(WORD addr)
{
	BYTE dat;

	switch (cputype) {
		case MAINCPU:
			dat = mainmem_readbnio(addr);
			break;

		case SUBCPU:
			dat = submem_readbnio(addr);
			break;

#if XM7_VER == 1
#if defined(JSUB)
		case JSUBCPU:
			dat = jsubmem_readbnio(addr);
			break;
#endif
#endif

		default:
			ASSERT(FALSE);
			break;
	}

	return dat;
}

/*
 *	16�i1���Z�b�g �T�u
 */
static void FASTCALL sub1hex(BYTE dat, char *buffer)
{
	char buf[2];

	/* assert */
	ASSERT(buffer);

	buf[0] = (char)(dat + 0x30);
	if (dat > 9) {
		buf[0] = (char)(dat + 0x37);
	}

	buf[1] = '\0';
	strcat(buffer, buf);
}

/*
 *	16�i2���Z�b�g �T�u
 */
static void FASTCALL sub2hex(BYTE dat, char *buffer)
{
	sub1hex((BYTE)(dat >> 4), buffer);
	sub1hex((BYTE)(dat & 0x0f), buffer );
}

/*
 *	16�i2���Z�b�g
 */
static void FASTCALL set2hex(BYTE dat)
{
	strcat(linebuf, "$");

	sub2hex(dat, linebuf);
}

/*
 *	16�i4���Z�b�g �T�u
 */
static void FASTCALL sub4hex(WORD dat, char *buffer)
{
	sub2hex((BYTE)(dat >> 8), buffer);
	sub2hex((BYTE)(dat & 0xff), buffer);
}

/*
 *	16�i4���Z�b�g
 */
static void FASTCALL set4hex(WORD dat)
{
	strcat(linebuf, "$");
#if 1
	sub4hex(dat, linebuf);
#else
	sub2hex((BYTE)(dat >> 8), linebuf);
	sub2hex((BYTE)(dat & 0xff), linebuf);
#endif
}

/*
 *	10�i2���Z�b�g
 */
static void FASTCALL set2dec(BYTE dat)
{
	char buf[2];

	buf[1] = '\0';

	/* ��ʌ� */
	buf[0] = (char)(dat / 10);
	if (buf[0] > 0) {
		buf[0] += (char)0x30;
		strcat(linebuf, buf);
	}

	/* ���ʌ� */
	buf[0] = (char)(dat % 10);
	buf[0] += (char)0x30;
	strcat(linebuf, buf);
}

/*
 *	����`
 */
static void FASTCALL notdef(void)
{
	strcat(linebuf, "?");
}

/*-[ �A�h���b�V���O���[�h���� ]---------------------------------------------*/

/*
 *	�����e�B�u���[�h(1�o�C�g)
 */
static void FASTCALL rel1(void)
{
	BYTE opr;

	/* �I�y�����h�擾 */
	opr = fetch();

	/* �Z�b�g */
	if (opr <= 0x7f) {
		set4hex((WORD)(pc + addpc + (BYTE)opr));
	}
	else {
		set4hex((WORD)(pc + addpc - (BYTE)(~opr + 1)));
	}
}

/*
 *	�����e�B�u���[�h(2�o�C�g)
 */
static void FASTCALL rel2(void)
{
	WORD dat;

	/* �I�y�����h�擾 */
	dat = (WORD)(fetch() << 8);
	dat |= (WORD)fetch();

	/* �Z�b�g */
	if (dat <= 0x7fff) {
		set4hex((WORD)(pc + addpc + dat));
	}
	else {
		set4hex((WORD)(pc + addpc - (~dat + 1)));
	}
}

/*
 *	�_�C���N�g���[�h
 */
static void FASTCALL direct(void)
{
	BYTE opr;

	/* �I�y�����h�擾 */
	opr = fetch();

	/* �Z�b�g */
	strcat(linebuf, "<");
	set2hex(opr);
}

/*
 *	�G�N�X�e���h���[�h
 */
static void FASTCALL extend(void)
{
	WORD dat;

	/* �I�y�����h�擾 */
	dat = (WORD)(fetch() << 8);
	dat |= (WORD)fetch();

	/* �Z�b�g */
	set4hex(dat);
}

/*
 *	�C�~�f�B�G�C�g���[�h(1�o�C�g)
 */
static void FASTCALL imm1(void)
{
	BYTE opr;

	/* �I�y�����h�擾 */
	opr = fetch();

	/* �Z�b�g */
	strcat(linebuf, "#");
	set2hex(opr);
}

/*
 *	�C�~�f�B�G�C�g���[�h(2�o�C�g)
 */
static void FASTCALL imm2(void)
{
	WORD dat;

	/* �I�y�����h�擾 */
	dat = (WORD)(fetch() << 8);
	dat |= (WORD)(fetch());

	/* �Z�b�g */
	strcat(linebuf, "#");
	set4hex(dat);
}

/*
 *	�C���f�b�N�X���[�h
 */
static void FASTCALL idx(void)
{
	BYTE opr;
	BYTE high, low;
	BYTE offset;
	WORD woffset;

	/* �I�y�����h�擾 */
	opr = fetch();
	high = (BYTE)(opr & 0xf0);
	low = (BYTE)(opr & 0x0f);

	/* 0x00�`0x7f��5bit�I�t�Z�b�g */
	if (opr < 0x80) {
		if (opr & 0x10) {
			/* �}�C�i�X */
			offset = (BYTE)(~((opr & 0x0f) | 0xf0) + 1);
			strcat(linebuf, "-");
			set2dec(offset);
		}
		else {
			/* �v���X */
			offset = low;
			set2dec(offset);
		}

		strcat(linebuf, ",");

		/* X, Y, U, S */
		offset = (BYTE)((opr & 0x60) >> 5);
		ASSERT(offset <= 3);
		strcat(linebuf, idx_tbl[offset]);
		return;
	}

	/* 0x8e,0xae,0xce,0xee�͗�O $FFFF */
	/* 0x9e,0xbe,0xde,0xfe�͗�O[$FFFF] */
	if (low == 0x0e) {
		if (high & 0x10) {
			strcat(linebuf, "[");
		}
		woffset = 0xffff;
		set4hex(woffset);
		if (high & 0x10) {
			strcat(linebuf, "]");
		}
		return;
	}
	/* 0x8f,0xaf,0xcf,0xef�͗�O addr */
	/* 0x9f,0xbf,0xdf,0xff�͗�O[addr] */
	if (low == 0x0f) {
		if (high & 0x10) {
			strcat(linebuf, "[");
		}
		woffset = (WORD)fetch();
		woffset = (WORD)((woffset << 8) + (WORD)fetch());
		set4hex(woffset);
		if (high & 0x10) {
			strcat(linebuf, "]");
		}
		return;
	}

	/* 0x80�ȏ�ŁA���ʂ�0,1,2,3�̓I�[�g�C���N�������gor�f�N�������g */
	if (low < 4) {
		if (high & 0x10) {
			strcat(linebuf, "[");
		}
		strcat(linebuf, ",");

		/* �I�[�g�f�N�������g */
		if (low >= 2) {
			strcat(linebuf, "-");
		}
		if (low == 3) {
			strcat(linebuf, "-");
		}

		/* X, Y, U, S */
		offset = (BYTE)((opr & 0x60) >> 5);
		ASSERT(offset <= 3);
		strcat(linebuf, idx_tbl[offset]);

		/* �I�[�g�C���N�������g */
		if (low < 2) {
			strcat(linebuf, "+");
		}
		if (low == 1) {
			strcat(linebuf, "+");
		}

		if (high & 0x10) {
			strcat(linebuf, "]");
		}
		return;
	}

	/* ����4,5,6,B�̓��W�X�^�I�t�Z�b�g */
	if ((low == 4) || (low == 5) || (low == 6) || (low == 11)) {
		if (high & 0x10) {
			strcat(linebuf, "[");
		}

		switch (low) {
			case 4:
				strcat(linebuf, ",");
				break;
			case 5:
				strcat(linebuf, "B,");
				break;
			case 6:
				strcat(linebuf, "A,");
				break;
			case 11:
				strcat(linebuf, "D,");
				break;
			default:
				ASSERT(FALSE);
				break;
		}

		/* X, Y, U, S */
		offset = (BYTE)((opr & 0x60) >> 5);
		ASSERT(offset <= 3);
		strcat(linebuf, idx_tbl[offset]);

		if (high & 0x10) {
			strcat(linebuf, "]");
		}
		return;
	}

	/* ����8,C��8bit�I�t�Z�b�g */
	if ((low == 8) || (low == 12)) {
		if (high & 0x10) {
			strcat(linebuf, "[");
		}

		offset = fetch();

		/* X, Y, U, S, PCR */
		if (low == 12) {
			if (offset >= 0x80) {
				woffset = (WORD)(0xff00 + offset);
			}
			else {
				woffset = offset;
			}
			set4hex((WORD)(pc + addpc + woffset));
			strcat(linebuf, ",");
			strcat(linebuf, "PCR");
		}
		else {
			if (offset >= 0x80) {
				offset = (BYTE)(~offset + 1);
				strcat(linebuf, "-");
			}
			set2hex(offset);
			strcat(linebuf, ",");
			offset = (BYTE)((opr & 0x60) >> 5);
			ASSERT(offset <= 3);
			strcat(linebuf, idx_tbl[offset]);
		}

		if (high & 0x10) {
			strcat(linebuf, "]");
		}
		return;
	}

	/* ����9,D��16bit�I�t�Z�b�g */
	if ((low == 9) || (low == 13)) {
		if (high & 0x10) {
			strcat(linebuf, "[");
		}

		woffset = (WORD)fetch();
		woffset = (WORD)((woffset << 8) + (WORD)fetch());

		/* X, Y, U, S, PCR */
		if (low == 13) {
			set4hex((WORD)(woffset + pc + addpc));
			strcat(linebuf, ",");
			strcat(linebuf, "PCR");
		}
		else {
			if (woffset >= 0x8000) {
				woffset = (WORD)(~woffset + 1);
				strcat(linebuf, "-");
			}
			set4hex(woffset);
			strcat(linebuf, ",");
			offset = (BYTE)((opr & 0x60) >> 5);
			ASSERT(offset <= 3);
			strcat(linebuf, idx_tbl[offset]);
		}

		if (high & 0x10) {
			strcat(linebuf, "]");
		}
		return;
	}

	/* ����ȊO�͖���` */
	notdef();
}

/*
 *	TFR,EXG,���W�X�^�ԉ��Z
 */
static void FASTCALL tfrexg(void)
{
	BYTE opr;
	const char **tfrexgtbl;

	/* �I�y�����h�擾 */
	opr = fetch();

	tfrexgtbl = (const char **)tfrexg_tbl;

	/* �쐬 */
	if (tfrexgtbl[(opr & 0xf0) >> 4] == NULL) {
		linebuf[0] = '\0';
		notdef();
		return;
	}
	strcat(linebuf, tfrexgtbl[(opr & 0xf0) >> 4]);
	strcat(linebuf, ",");
	if (tfrexgtbl[opr & 0x0f] == NULL) {
		linebuf[0] = '\0';
		notdef();
		return;
	}
 	strcat(linebuf, tfrexgtbl[opr & 0x0f]);
}

/*
 *	PSH,PUL
 */
static void FASTCALL pshpul(void)
{
	BYTE opr;
	char sreg[2];
	int i;
	int flag;

	/* �I�y�����h�擾 */
	opr = fetch();

	/* S,U�����肷�� */
	if (linebuf[3] == 'S') {
		sreg[0] = 'U';
	}
	else {
		sreg[0] = 'S';
	}
	sreg[1] = '\0';

	/* 8��]�� */
	flag = FALSE;
	for (i=0; i<8; i++) {
		if (opr & 0x01) {
			if (flag) {
				strcat(linebuf, ",");
			}
			if (i == 6) {
				/* S,U */
				strcat(linebuf, sreg);
			}
			else {
				/* ����ȊO */
				strcat(linebuf, pshpul_tbl[i]);
			}
			flag = TRUE;
		}
		opr >>= 1;
	}
}

/*
 *	�j�[���j�b�N�A��
 */
static void FASTCALL strcat_mnemonic(const char *s)
{
	char tmp[7];

	tmp[6] = '\0';
	memset(tmp, 0x20, sizeof(tmp) - 1);
	memcpy(tmp, s, strlen(s));
	strcat(linebuf, tmp);
}

/*-[ �I�y�R�[�h���� ]-------------------------------------------------------*/

/*
 *	�y�[�W2(0x10)
 */
static void FASTCALL page2(void)
{
	BYTE high, low;
	char tmp[5];

	/* �I�y�R�[�h���ēx�擾 */
	opc = fetch();
	high = (BYTE)(opc & 0xf0);
	low = (BYTE)(opc & 0x0f);

	/* 0x20��̓����O�u�����` */
	if (high == 0x20) {
		tmp[0] = 'L';
		strcpy(&tmp[1], branch_tbl[low]);
		strcat_mnemonic(tmp);
		rel2();
		return;
	}

	/* 0x3f��SWI2 */
	if (opc == 0x3f) {
		strcat_mnemonic("SWI2");
		return;
	}

	/* 0x8d��LBSR */
	if (opc == 0x8d) {
		strcat_mnemonic("LBSR");
		return;
	}

	/* 0xc0�ȏ��S���W�X�^ */
	if (opc >= 0xc0) {
		if (low <= 0x0d) {
			notdef();
			return;
		}
		if (opc == 0xcf) {
			notdef();
			return;
		}
		/* 0x?e��LDS�A0x?f��STS */
		if (low == 0x0e) {
			strcat_mnemonic("LDS");
		}
		if (low == 0x0f) {
			strcat_mnemonic("STS");
		}
		/* LDS,STS �A�h���b�V���O���[�h */
		switch (high) {
			case 0xc0:
				imm2();
				break;
			case 0xd0:
				direct();
				break;
			case 0xe0:
				idx();
				break;
			case 0xf0:
				extend();
				break;
			default:
				ASSERT(FALSE);
				break;
		}
		return;
	}

	/* 0x80�ȏ��CMPD, CMPY, LDY, STY */
	if (opc >= 0x80) {
		switch (low) {
			case 0x03:
				strcat_mnemonic("CMPD");
				break;
			case 0x0c:
				strcat_mnemonic("CMPY");
				break;
			case 0x0e:
				strcat_mnemonic("LDY");
				break;
			case 0x0f:
				if (high == 0x80) {
					notdef();
					return;
				}
				else {
					strcat_mnemonic("STY");
				}
				break;
			default:
				notdef();
				return;
		}

		/* �A�h���b�V���O���[�h */
		switch (high) {
			case 0x80:
				imm2();
				break;
			case 0x90:
				direct();
				break;
			case 0xa0:
				idx();
				break;
			case 0xb0:
				extend();
				break;
			default:
				ASSERT(FALSE);
				break;
		}

		return;
	}

	/* ����ȊO�͖���` */
	notdef();
}

/*
 *	�y�[�W3(0x11)
 */
static void FASTCALL page3(void)
{
	BYTE high, low;

	/* �I�y�R�[�h���ēx�擾 */
	opc = fetch();
	high = (BYTE)(opc & 0xf0);
	low = (BYTE)(opc & 0x0f);

	/* 0x3f��SWI3 */
	if (opc == 0x3f) {
		strcat_mnemonic("SWI3");
		return;
	}

	/* ��ʂ�8,9,A,B */
	if ((high >= 0x80) && (high <= 0xb0)) {
		/* ���ʃ`�F�b�N */
		switch (low) {
			case 3:
				strcat_mnemonic("CMPU");
				break;
			case 12:
				strcat_mnemonic("CMPS");
				break;
			default:
				notdef();
				return;
		}

		/* �A�h���b�V���O���[�h */
		switch (high) {
			case 0x80:
				imm2();
				break;
			case 0x90:
				direct();
				break;
			case 0xa0:
				idx();
				break;
			case 0xb0:
				extend();
				break;
			default:
				ASSERT(FALSE);
				break;
		}

		return;
	}

	/* ����ȊO�͖���` */
	notdef();
}

/*
 *	��O�n1(0x00)
 */
static void FASTCALL except1(void)
{
	if ((opc & 0x0f) == 0x0e) {
		/* 0x0e��JMP */
		strcat_mnemonic("JMP");
	}
	else {
		strcat_mnemonic(inh_tbl[opc & 0x0f]);
	}

	/* ���ׂă_�C���N�g */
	direct();
}

/*
 *	��O�n2(0x10)
 */
static void FASTCALL except2(void)
{
	/* 0x10, 0x11�Ŏn�܂�y�[�W */
	if (opc == 0x10) {
		page2();
		return;
	}
	if (opc == 0x11) {
		page3();
		return;
	}

	/* ����`�`�F�b�N */
	if (except2_tbl[opc & 0x0f] == NULL) {
		notdef();
		return;
	}
	strcat_mnemonic(except2_tbl[opc & 0x0f]);

	/* 0x16, 0x17�̓����O�u�����` */
	if ((opc == 0x16) || (opc == 0x17)) {
		rel2();
		return;
	}

	/* 0x1a, 0x1c�̓C�~�f�B�G�C�g */
	if ((opc == 0x1a) || (opc == 0x1c)) {
		imm1();
		return;
	}

	/* 0x1e, 0x1f��TFR/EXG */
	if ((opc == 0x1e) || (opc == 0x1f)) {
		tfrexg();
		return;
	}
}

/*
 *	�u�����`�n(0x20)
 */
static void FASTCALL branch(void)
{
	strcat_mnemonic(branch_tbl[opc - 0x20]);
	rel1();
}

/*
 *	LEA,�X�^�b�N�n(0x30)
 */
static void FASTCALL leastack(void)
{
	/* �I�y�R�[�h */
	strcat_mnemonic(leastack_tbl[opc & 0x0f]);

	/* LEA�̓C���f�b�N�X�̂� */
	if (opc < 0x34) {
		idx();
		return;
	}

	/* PSH,PUL�͐�p */
	if (opc < 0x38) {
		pshpul();
		return;
	}

	/* CWAI/�B������ANDCC�̓C�~�f�B�G�C�g */
	if ((opc == 0x38) || (opc == 0x3c)) {
		imm1();
	}
}

/*
 *	�C���w�����gA,B,M�n(0x40,0x50,0x60,0x70)
 */
static void FASTCALL inhabm(void)
{
	char tmp[5];

	switch (opc >> 4) {
		/* A/B���W�X�^ */
		case 0x4:
		case 0x5:
			strcpy(tmp, inh_tbl[opc & 0x0f]);
			tmp[3] = (char)('A' + ((opc & 0x10) >> 4));
			tmp[4] = '\0';
			strcat_mnemonic(tmp);
			break;
		/* �������A�C���f�b�N�X/�G�N�X�e���h */
		case 0x6:
		case 0x7:
			strcat_mnemonic(inhm_tbl[opc & 0x0f]);
			if ((opc & 0xf0) == 0x60) {
				idx();
			}
			else {
				extend();
			}
			break;
		default:
			ASSERT(FALSE);
			break;
	}
}

/*
 *	A���W�X�^�AX���W�X�^�n(0x80, 0x90, 0xa0, 0xb0)
 *	B���W�X�^�AD���W�X�^�AU���W�X�^�n(0xc0, 0xd0, 0xe0, 0xf0)
 */
static void FASTCALL regaxbdu(void)
{
	BYTE opc2;

	/* ���������̓s����Abit6��0�ɌŒ� */
	opc2 = (BYTE)(opc & 0xbf);

	/* 0x87, 0x8f, 0xc7, 0xcf�͉B������ */
	if ((opc2 == 0x87) || (opc2 == 0x8f)) {
		strcat_mnemonic("FLAG");
		if (opc2 == 0x87) {
			imm1();
		}
		else {
			imm2();
		}
		return;
	}

	/* 0x8d��BSR */
	if (opc == 0x8d) {
		strcat_mnemonic("BSR");
		rel1();
		return;
	}

	/* 0xcd��HCF */
	if (opc == 0xcd) {
		strcat_mnemonic("HCF");
		return;
	}

	/* ����ȊO */
	if (opc & 0x40) {
		strcat_mnemonic(regbdu_tbl[opc & 0x0f]);
	}
	else {
		strcat_mnemonic(regax_tbl[opc & 0x0f]);
	}

	/* �A�h���b�V���O���[�h�� */
	switch (opc2 >> 4) {
		case 0x8:
			if ((opc2 == 0x83) || (opc2 == 0x8c) || (opc2 == 0x8e)) {
				imm2();
			}
			else {
				imm1();
			}
			break;
		case 0x9:
			direct();
			break;
		case 0xa:
			idx();
			break;
		case 0xb:
			extend();
			break;
		default:
			ASSERT(FALSE);
			break;
	}
}

/*-[ ���C�� ]---------------------------------------------------------------*/

/*
 *	�t�A�Z���u���{��
 */
int FASTCALL disline(int cpu, WORD pcreg, char *buffer)
{
	int i;
	int j;

	/* assert */
#if XM7_VER == 1
#if defined(JSUB)
	ASSERT((cpu == MAINCPU) || (cpu == SUBCPU) || (cpu == JSUBCPU));
#else
	ASSERT((cpu == MAINCPU) || (cpu == SUBCPU));
#endif
#else
	ASSERT((cpu == MAINCPU) || (cpu == SUBCPU));
#endif
	ASSERT(buffer);

	/* �����ݒ� */
	cputype = cpu;
	pc = pcreg;
	addpc = 0;
	linebuf[0] = '\0';

	/* �擪�̃o�C�g��ǂݏo�� */
	opc = fetch();

	/* �O���[�v�� */
	switch ((int)(opc >> 4)) {
		case 0x0:
			except1();	
			break;
		case 0x1:
			except2();
			break;
		case 0x2:
			branch();
			break;
		case 0x3:
			leastack();
			break;
		case 0x4:
		case 0x5:
		case 0x6:
		case 0x7:
			inhabm();
			break;
		case 0x8:
		case 0x9:
		case 0xa:
		case 0xb:
		case 0xc:
		case 0xd:
		case 0xe:
		case 0xf:
			regaxbdu();
			break;
		default:
			ASSERT(FALSE);
	}

	/* ���߃f�[�^���Z�b�g */
	buffer[0] = '\0';

	sub4hex(pcreg, buffer);
	strcat(buffer, " ");
	for (i=0; i<addpc; i++) {
		sub2hex(read_byte((WORD)(pcreg + i)), buffer);
		strcat(buffer, " ");
	}

	/* buffer��20�o�C�g+'\0'�ɂȂ�悤�������� */
	j = strlen(buffer);
	if (j < 20) {
		for (i=0; i<20 - j; i++) {
			buffer[i + j] = ' ';
		}
		buffer[i + j] = '\0';
	}

	strcat(buffer, linebuf);
 	return (int)addpc;
}
