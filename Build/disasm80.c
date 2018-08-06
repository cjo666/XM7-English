/*
 *	FM-7 EMULATOR "XM7"
 *
 *	Copyright (C) 1999-2017 ＰＩ．(Twitter:@xm6_original)
 *	Copyright (C) 2001-2017 Ryu Takegami (Twitter:@RyuTakegami)
 *	fMSX built-in disassembler  Copyright (C) 1995-2000 Marat Fayzullin
 *
 *	[ 逆アセンブラ(Z80) ]
 *
 *	RHG履歴
 *	  2017.06.18		某プロジェクトをベースにして新設
 */

#if XM7_VER == 1 && defined(Z80CARD)

#include <string.h>
#include "xm7.h"

/*
 *	スタティック ワーク
 */
static BYTE opc;					/* オペコード */
static WORD pc;						/* 実行前PC */
static WORD addpc;					/* PC加算値(命令長) */
static char linebuf[32];			/* 逆アセンブル出力バッファ */

static char *Mnemonics[256] =
{
	"NOP",			"LD    BC,#",	"LD    (BC),A",		"INC   BC",
	"INC   B",		"DEC   B",		"LD    B,*", 		"RLCA",
	"EX    AF,AF'",	"ADD   HL,BC",	"LD    A,(BC)",		"DEC   BC",
	"INC   C",		"DEC   C",		"LD    C,*",		"RRCA",
	"DJNZ  @",		"LD    DE,#",	"LD    (DE),A",		"INC   DE",
	"INC   D",		"DEC   D",		"LD    D,*",		"RLA",
	"JR    @",		"ADD   HL,DE",	"LD    A,(DE)",		"DEC   DE",
	"INC   E",		"DEC   E",		"LD    E,*",		"RRA",
	"JR    NZ,@",	"LD    HL,#",	"LD    (#),HL",		"INC   HL",
	"INC   H",		"DEC   H",		"LD    H,*",		"DAA",
	"JR    Z,@",	"ADD   HL,HL",	"LD    HL,(#)",		"DEC   HL",
	"INC   L",		"DEC   L",		"LD    L,*",		"CPL",
	"JR    NC,@",	"LD    SP,#",	"LD    (#),A",		"INC   SP",
	"INC   (HL)",	"DEC   (HL)",	"LD    (HL),*",		"SCF",
	"JR    C,@",	"ADD   HL,SP",	"LD    A,(#)",		"DEC   SP",
	"INC   A",		"DEC   A",		"LD    A,*",		"CCF",
	"LD    B,B",	"LD    B,C",	"LD    B,D",		"LD    B,E",
	"LD    B,H",	"LD    B,L",	"LD    B,(HL)",		"LD    B,A",
	"LD    C,B",	"LD    C,C",	"LD    C,D",		"LD    C,E",
	"LD    C,H",	"LD    C,L",	"LD    C,(HL)",		"LD    C,A",
	"LD    D,B",	"LD    D,C",	"LD    D,D",		"LD    D,E",
	"LD    D,H",	"LD    D,L",	"LD    D,(HL)",		"LD    D,A",
	"LD    E,B",	"LD    E,C",	"LD    E,D",		"LD    E,E",
	"LD    E,H",	"LD    E,L",	"LD    E,(HL)",		"LD    E,A",
	"LD    H,B",	"LD    H,C",	"LD    H,D",		"LD    H,E",
	"LD    H,H",	"LD    H,L",	"LD    H,(HL)",		"LD    H,A",
	"LD    L,B",	"LD    L,C",	"LD    L,D",		"LD    L,E",
	"LD    L,H",	"LD    L,L",	"LD    L,(HL)",		"LD    L,A",
	"LD    (HL),B",	"LD    (HL),C",	"LD    (HL),D",		"LD    (HL),E",
	"LD    (HL),H",	"LD    (HL),L",	"HALT",				"LD    (HL),A",
	"LD    A,B",	"LD    A,C",	"LD    A,D",		"LD    A,E",
	"LD    A,H",	"LD    A,L",	"LD    A,(HL)",		"LD    A,A",
	"ADD   B",		"ADD   C",		"ADD   D",			"ADD   E",
	"ADD   H",		"ADD   L",		"ADD   (HL)",		"ADD   A",
	"ADC   B",		"ADC   C",		"ADC   D",			"ADC   E",
	"ADC   H",		"ADC   L",		"ADC   (HL)",		"ADC   A",
	"SUB   B",		"SUB   C",		"SUB   D",			"SUB   E",
	"SUB   H",		"SUB   L",		"SUB   (HL)",		"SUB   A",
	"SBC   B",		"SBC   C",		"SBC   D",			"SBC   E",
	"SBC   H",		"SBC   L",		"SBC   (HL)",		"SBC   A",
	"AND   B",		"AND   C",		"AND   D",			"AND   E",
	"AND   H",		"AND   L",		"AND   (HL)",		"AND   A",
	"XOR   B",		"XOR   C",		"XOR   D",			"XOR   E",
	"XOR   H",		"XOR   L",		"XOR   (HL)",		"XOR   A",
	"OR    B",		"OR    C",		"OR    D",			"OR    E",
	"OR    H",		"OR    L",		"OR    (HL)",		"OR    A",
	"CP    B",		"CP    C",		"CP    D",			"CP    E",
	"CP    H",		"CP    L",		"CP    (HL)",		"CP    A",
	"RET   NZ",		"POP   BC",		"JP    NZ,#",		"JP    #",
	"CALL  NZ,#",	"PUSH  BC",		"ADD   *",			"RST   00h",
	"RET   Z",		"RET",			"JP    Z,#",		NULL,
	"CALL  Z,#",	"CALL  #",		"ADC   *",			"RST   08h",
	"RET   NC",		"POP   DE",		"JP    NC,#",		"OUT   (*), A",
	"CALL  NC,#",	"PUSH  DE",		"SUB   *",			"RST   10h",
	"RET   C",		"EXX",			"JP    C,#",		"IN    A, (*)",
	"CALL  C,#",	NULL,			"SBC   *",			"RST   18h",
	"RET   PO",		"POP   HL",		"JP    PO,#",		"EX    HL, (SP)",
	"CALL  PO,#",	"PUSH  HL",		"AND   *",			"RST   20h",
	"RET   PE",		"JP    (HL)",	"JP    PE,#",		"EX    DE, HL",
	"CALL  PE,#",	NULL,			"XOR   *",			"RST   28h",
	"RET   P",		"POP   AF",		"JP    P,#",		"DI",
	"CALL  P,#",	"PUSH  AF",		"OR    *",			"RST   30h",
	"RET   M",		"LD    SP, HL",	"JP    M,#",		"EI",
	"CALL  M,#",	NULL,			"CP    *",			"RST   38h"
};


static char *MnemonicsCB[256] =
{
	"RLC   B",		"RLC   C",		"RLC   D",			"RLC   E",
	"RLC   H",		"RLC   L",		"RLC   (HL)",		"RLC   A",
	"RRC   B",		"RRC   C",		"RRC   D",			"RRC   E",
	"RRC   H",		"RRC   L",		"RRC   (HL)",		"RRC   A",
	"RL    B",		"RL    C",		"RL    D",			"RL    E",
	"RL    H",		"RL    L",		"RL    (HL)",		"RL    A",
	"RR    B",		"RR    C",		"RR    D",			"RR    E",
	"RR    H",		"RR    L",		"RR    (HL)",		"RR    A",
	"SLA   B",		"SLA   C",		"SLA   D",			"SLA   E",
	"SLA   H",		"SLA   L",		"SLA   (HL)",		"SLA   A",
	"SRA   B",		"SRA   C",		"SRA   D",			"SRA   E",
	"SRA   H",		"SRA   L",		"SRA   (HL)",		"SRA   A",
	"SLL   B",		"SLL   C",		"SLL   D",			"SLL   E",
	"SLL   H",		"SLL   L",		"SLL   (HL)",		"SLL   A",
	"SRL   B",		"SRL   C",		"SRL   D",			"SRL   E",
	"SRL   H",		"SRL   L",		"SRL   (HL)",		"SRL   A",
	"BIT   0,B",	"BIT   0,C",	"BIT   0,D",		"BIT   0,E",
	"BIT   0,H",	"BIT   0,L",	"BIT   0,(HL)",		"BIT   0,A",
	"BIT   1,B",	"BIT   1,C",	"BIT   1,D",		"BIT   1,E",
	"BIT   1,H",	"BIT   1,L",	"BIT   1,(HL)",		"BIT   1,A",
	"BIT   2,B",	"BIT   2,C",	"BIT   2,D",		"BIT   2,E",
	"BIT   2,H",	"BIT   2,L",	"BIT   2,(HL)",		"BIT   2,A",
	"BIT   3,B",	"BIT   3,C",	"BIT   3,D",		"BIT   3,E",
	"BIT   3,H",	"BIT   3,L",	"BIT   3,(HL)",		"BIT   3,A",
	"BIT   4,B",	"BIT   4,C",	"BIT   4,D",		"BIT   4,E",
	"BIT   4,H",	"BIT   4,L",	"BIT   4,(HL)",		"BIT   4,A",
	"BIT   5,B",	"BIT   5,C",	"BIT   5,D",		"BIT   5,E",
	"BIT   5,H",	"BIT   5,L",	"BIT   5,(HL)",		"BIT   5,A",
	"BIT   6,B",	"BIT   6,C",	"BIT   6,D",		"BIT   6,E",
	"BIT   6,H",	"BIT   6,L",	"BIT   6,(HL)",		"BIT   6,A",
	"BIT   7,B",	"BIT   7,C",	"BIT   7,D",		"BIT   7,E",
	"BIT   7,H",	"BIT   7,L",	"BIT   7,(HL)",		"BIT   7,A",
	"RES   0,B",	"RES   0,C",	"RES   0,D",		"RES   0,E",
	"RES   0,H",	"RES   0,L",	"RES   0,(HL)",		"RES   0,A",
	"RES   1,B",	"RES   1,C",	"RES   1,D",		"RES   1,E",
	"RES   1,H",	"RES   1,L",	"RES   1,(HL)",		"RES   1,A",
	"RES   2,B",	"RES   2,C",	"RES   2,D",		"RES   2,E",
	"RES   2,H",	"RES   2,L",	"RES   2,(HL)",		"RES   2,A",
	"RES   3,B",	"RES   3,C",	"RES   3,D",		"RES   3,E",
	"RES   3,H",	"RES   3,L",	"RES   3,(HL)",		"RES   3,A",
	"RES   4,B",	"RES   4,C",	"RES   4,D",		"RES   4,E",
	"RES   4,H",	"RES   4,L",	"RES   4,(HL)",		"RES   4,A",
	"RES   5,B",	"RES   5,C",	"RES   5,D",		"RES   5,E",
	"RES   5,H",	"RES   5,L",	"RES   5,(HL)",		"RES   5,A",
	"RES   6,B",	"RES   6,C",	"RES   6,D",		"RES   6,E",
	"RES   6,H",	"RES   6,L",	"RES   6,(HL)",		"RES   6,A",
	"RES   7,B",	"RES   7,C",	"RES   7,D",		"RES   7,E",
	"RES   7,H",	"RES   7,L",	"RES   7,(HL)",		"RES   7,A",
	"SET   0,B",	"SET   0,C",	"SET   0,D",		"SET   0,E",
	"SET   0,H",	"SET   0,L",	"SET   0,(HL)",		"SET   0,A",
	"SET   1,B",	"SET   1,C",	"SET   1,D",		"SET   1,E",
	"SET   1,H",	"SET   1,L",	"SET   1,(HL)",		"SET   1,A",
	"SET   2,B",	"SET   2,C",	"SET   2,D",		"SET   2,E",
	"SET   2,H",	"SET   2,L",	"SET   2,(HL)",		"SET   2,A",
	"SET   3,B",	"SET   3,C",	"SET   3,D",		"SET   3,E",
	"SET   3,H",	"SET   3,L",	"SET   3,(HL)",		"SET   3,A",
	"SET   4,B",	"SET   4,C",	"SET   4,D",		"SET   4,E",
	"SET   4,H",	"SET   4,L",	"SET   4,(HL)",		"SET   4,A",
	"SET   5,B",	"SET   5,C",	"SET   5,D",		"SET   5,E",
	"SET   5,H",	"SET   5,L",	"SET   5,(HL)",		"SET   5,A",
	"SET   6,B",	"SET   6,C",	"SET   6,D",		"SET   6,E",
	"SET   6,H",	"SET   6,L",	"SET   6,(HL)",		"SET   6,A",
	"SET   7,B",	"SET   7,C",	"SET   7,D",		"SET   7,E",
	"SET   7,H",	"SET   7,L",	"SET   7,(HL)",		"SET   7,A"
};


static char *MnemonicsED[256] =
{
	NULL,			NULL,			NULL,				NULL,
	NULL,			NULL,			NULL,				NULL,
	NULL,			NULL,			NULL,				NULL,
	NULL,			NULL,			NULL,				NULL,
	NULL,			NULL,			NULL,				NULL,
	NULL,			NULL,			NULL,				NULL,
	NULL,			NULL,			NULL,				NULL,
	NULL,			NULL,			NULL,				NULL,
	NULL,			NULL,			NULL,				NULL,
	NULL,			NULL,			NULL,				NULL,
	NULL,			NULL,			NULL,				NULL,
	NULL,			NULL,			NULL,				NULL,
	NULL,			NULL,			NULL,				NULL,
	NULL,			NULL,			NULL,				NULL,
	NULL,			NULL,			NULL,				NULL,
	NULL,			NULL,			NULL,				NULL,
	"IN    B,(C)",	"OUT   (C),B",	"SBC   HL,BC",		"LD    (#),BC",
	"NEG",			"RETN",			"IM    0",			"LD    I,A",
	"IN    C,(C)",	"OUT   (C),C",	"ADC   HL,BC",		"LD    BC,(#)",
	"+NEG",			"RETI",			"+IM   ?",			"LD    R,A",
	"IN    D,(C)",	"OUT   (C),D",	"SBC   HL,DE",		"LD    (#) DE",
	"+NEG",			"+RETN",		"IM    1",			"LD    A,I",
	"IN    E,(C)",	"OUT   (C),E",	"ADC   HL,DE",		"LD    DE,(#)",
	NULL,			"+RETN",		"IM    2",			"LD    A,R",
	"IN    H,(C)",	"OUT   (C),H",	"SBC   HL,HL",		"LD    (#),HL",
	"+NEG",			"+RETN",		"+IM   0",			"RRD",
	"IN    L,(C)",	"OUT   (C),L",	"ADC   HL,HL",		"LD    HL,(#)",
	NULL,			"+RETN",		"+IM   ?",			"RLD",
	"+IN   F,(C)",	"+OUT  (C),0",	"SBC   HL, SP",		"LD    (#),SP",
	"+NEG",			"+RETN",		"+IM   1",			NULL,
	"IN    A,(C)",	"OUT   (C),A",	"ADC   HL, SP",		"LD    SP,(#)",
	"+NEG",			"+RETN",		"+IM   2",			NULL,
	NULL,			NULL,			NULL,				NULL,
	NULL,			NULL,			NULL,				NULL,
	NULL,			NULL,			NULL,				NULL,
	NULL,			NULL,			NULL,				NULL,
	NULL,			NULL,			NULL,				NULL,
	NULL,			NULL,			NULL,				NULL,
	NULL,			NULL,			NULL,				NULL,
	NULL,			NULL,			NULL,				NULL,
	"LDI",			"CPI",			"INI",				"OUTI",
	NULL,			NULL,			NULL,				NULL,
	"LDD",			"CPD",			"IND",				"OUTD",
	NULL,			NULL,			NULL,				NULL,
	"LDIR",			"CPIR",			"INIR",				"OTIR",
	NULL,			NULL,			NULL,				NULL,
	"LDDR",			"CPDR",			"INDR",				"OTDR",
	NULL,			NULL,			NULL,				NULL,
	NULL,			NULL,			NULL,				NULL,
	NULL,			NULL,			NULL,				NULL,
	NULL,			NULL,			NULL,				NULL,
	NULL,			NULL,			NULL,				NULL,
	NULL,			NULL,			NULL,				NULL,
	NULL,			NULL,			NULL,				NULL,
	NULL,			NULL,			NULL,				NULL,
	NULL,			NULL,			NULL,				NULL,
};


static char *MnemonicsXX[256] =
{
	"NOP",			"LD    BC,#",	"LD    (BC),A",		"INC   BC",
	"INC   B",		"DEC   B",		"LD    B,*",		"RLCA",
	"EX    AF,AF'",	"ADD   I%,BC",	"LD    A,(BC)",		"DEC   BC",
	"INC   C",		"DEC   C",		"LD    C,*",		"RRCA",
	"DJNZ  @",		"LD    DE,#",	"LD    (DE),A",		"INC   DE",
	"INC   D",		"DEC   D",		"LD    D,*",		"RLA",
	"JR    @",		"ADD   I%,DE",	"LD    A,(DE)",		"DEC   DE",
	"INC   E",		"DEC   E",		"LD    E,*",		"RRA",
	"JR    NZ,@",	"LD    I%,#",	"LD    (#),I%",		"INC   I%",
	"INC   I%H",	"DEC   I%H",	"LD    I%H,*",		"DAA",
	"JR    Z,@",	"ADD   I%,I%",	"LD    I%,(#)",		"DEC   I%",
	"INC   I%L",	"DEC   I%L",	"LD    I%L,*",		"CPL",
	"JR    NC,@",	"LD    SP,#",	"LD    (#),A",		"INC   SP",
	"INC   (I%^)",	"DEC   (I%^)",	"LD    (I%^),*",	"SCF",
	"JR    C,@",	"ADD   I%,SP",	"LD    A,(#)",		"DEC   SP",
	"INC   A",		"DEC   A",		"LD    A,*",		"CCF",
	"LD    B,B",	"LD    B,C",	"LD    B,D",		"LD    B,E",
	"LD    B,I%H",	"LD    B,I%L",	"LD    B,(I%^)",	"LD    B,A",
	"LD    C,B",	"LD    C,C",	"LD    C,D",		"LD    C,E",
	"LD    C,I%H",	"LD    C,I%L",	"LD    C,(I%^)",	"LD    C,A",
	"LD    D,B",	"LD    D,C",	"LD    D,D",		"LD    D,E",
	"LD    D,I%H",	"LD    D,I%L",	"LD    D,(I%^)",	"LD    D,A",
	"LD    E,B",	"LD    E,C",	"LD    E,D",		"LD    E,E",
	"LD    E,I%H",	"LD    E,I%L",	"LD    E,(I%^)",	"LD    E,A",
	"LD    I%H,B",	"LD    I%H,C",	"LD    I%H,D",		"LD    I%H,E",
	"LD    I%H,I%H","LD    I%H,I%L","LD    H,(I%^)",	"LD    I%H,A",
	"LD    I%L,B",	"LD    I%L,C",	"LD    I%L,D",		"LD    I%L,E",
	"LD    I%L,I%H","LD    I%L,I%L","LD    L,(I%^)",	"LD    I%L,A",
	"LD    (I%^),B","LD    (I%^),C","LD    (I%^),D",	"LD    (I%^),E",
	"LD    (I%^),H","LD    (I%^),L","HALT",				"LD    (I%^),A",
	"LD    A,B",	"LD    A,C",	"LD    A,D",		"LD    A,E",
	"LD    A,I%H",	"LD    A,I%L",	"LD    A,(I%^)",	"LD    A,A",
	"ADD   B",		"ADD   C",		"ADD   D",			"ADD   E",
	"ADD   I%H",	"ADD   I%L",	"ADD   (I%^)",		"ADD   A",
	"ADC   B",		"ADC   C",		"ADC   D",			"ADC   E",
	"ADC   I%H",	"ADC   I%L",	"ADC   (I%^)",		"ADC   A",
	"SUB   B",		"SUB   C",		"SUB   D",			"SUB   E",
	"SUB   I%H",	"SUB   I%L",	"SUB   (I%^)",		"SUB   A",
	"SBC   B",		"SBC   C",		"SBC   D",			"SBC   E",
	"SBC   I%H",	"SBC   I%L",	"SBC   (I%^)",		"SBC   A",
	"AND   B",		"AND   C",		"AND   D",			"AND   E",
	"AND   I%H",	"AND   I%L",	"AND   (I%^)",		"AND   A",
	"XOR   B",		"XOR   C",		"XOR   D",			"XOR   E",
	"XOR   I%H",	"XOR   I%L",	"XOR   (I%^)",		"XOR   A",
	"OR    B",		"OR    C",		"OR    D",			"OR    E",
	"OR    I%H",	"OR    I%L",	"OR    (I%^)",		"OR    A",
	"CP    B",		"CP    C",		"CP    D",			"CP    E",
	"CP    I%H",	"CP    I%L",	"CP    (I%^)",		"CP    A",
	"RET   NZ",		"POP   BC",		"JP    NZ,#",		"JP    #",
	"CALL  NZ,#",	"PUSH  BC",		"ADD   *",			"RST   00h",
	"RET   Z",		"RET",			"JP    Z,#",		NULL,
	"CALL  Z,#",	"CALL  #",		"ADC   *",			"RST   08h",
	"RET   NC",		"POP   DE",		"JP    NC,#",		"OUT   (*),A",
	"CALL  NC,#",	"PUSH  DE",		"SUB   *",			"RST   10h",
	"RET   C",		"EXX",			"JP    C,#",		"IN    A,(*)",
	"CALL  C,#",	NULL,			"SBC   *",			"RST   18h",
	"RET   PO",		"POP I%",		"JP    PO,#",		"EX    I%,(SP)",
	"CALL  PO,#",	"PUSH I%",		"AND   *",			"RST   20h",
	"RET   PE",		"JP (I%)",		"JP    PE,#",		"EX    DE,I%",
	"CALL  PE,#",	NULL,			"XOR   *",			"RST   28h",
	"RET   P",		"POP AF",		"JP    P,#",		"DI",
	"CALL  P,#",	"PUSH AF",		"OR    *",			"RST   30h",
	"RET   M",		"LD SP, I%",	"JP    M,#",		"EI",
	"CALL  M,#",	NULL,			"CP    *",			"RST   38h"
};


static char *MnemonicsXCB[256] =
{
	"RLC   (I%^),B","RLC   (I%^),C","RLC   (I%^),D",	"RLC   (I%^),E",
	"RLC   (I%^),H","RLC   (I%^),L","RLC   (I%^)",		"RLC   (I%^),A",
	"RRC   (I%^),B","RRC   (I%^),C","RRC   (I%^),D",	"RRC   (I%^),E",
	"RRC   (I%^),H","RRC   (I%^),L","RRC   (I%^)",		"RRC   (I%^),A",
	"RL    (I%^),B","RL    (I%^),C","RL    (I%^),D",	"RL    (I%^),E",
	"RL    (I%^),H","RL    (I%^),L","RL    (I%^)",		"RL    (I%^),A",
	"RR    (I%^),B","RR    (I%^),C","RR    (I%^),D",	"RR    (I%^),E",
	"RR    (I%^),H","RR    (I%^),L","RR    (I%^)",		"RR    (I%^),A",
	"SLA   B",		"SLA   C",		"SLA   D",			"SLA   E",
	"SLA   H",		"SLA   L",		"SLA   (I%^)",		"SLA   A",
	"SRA   B",		"SRA   C",		"SRA   D",			"SRA   E",
	"SRA   H",		"SRA   L",		"SRA   (I%^)",		"SRA   A",
	"SLL   B",		"SLL   C",		"SLL   D",			"SLL   E",
	"SLL   H",		"SLL   L",		"SLL   (I%^)",		"SLL   A",
	"SRL   B",		"SRL   C",		"SRL   D",			"SRL   E",
	"SRL   H",		"SRL   L",		"SRL   (I%^)",		"SRL   A",
	"BIT   0,B",	"BIT   0,C",	"BIT   0,D",		"BIT   0,E",
	"BIT   0,H",	"BIT   0,L",	"BIT   0,(I%^)",	"BIT   0,A",
	"BIT   1,B",	"BIT   1,C",	"BIT   1,D",		"BIT   1,E",
	"BIT   1,H",	"BIT   1,L",	"BIT   1,(I%^)",	"BIT   1,A",
	"BIT   2,B",	"BIT   2,C",	"BIT   2,D",		"BIT   2,E",
	"BIT   2,H",	"BIT   2,L",	"BIT   2,(I%^)",	"BIT   2,A",
	"BIT   3,B",	"BIT   3,C",	"BIT   3,D",		"BIT   3,E",
	"BIT   3,H",	"BIT   3,L",	"BIT   3,(I%^)",	"BIT   3,A",
	"BIT   4,B",	"BIT   4,C",	"BIT   4,D",		"BIT   4,E",
	"BIT   4,H",	"BIT   4,L",	"BIT   4,(I%^)",	"BIT   4,A",
	"BIT   5,B",	"BIT   5,C",	"BIT   5,D",		"BIT   5,E",
	"BIT   5,H",	"BIT   5,L",	"BIT   5,(I%^)",	"BIT   5,A",
	"BIT   6,B",	"BIT   6,C",	"BIT   6,D",		"BIT   6,E",
	"BIT   6,H",	"BIT   6,L",	"BIT   6,(I%^)",	"BIT   6,A",
	"BIT   7,B",	"BIT   7,C",	"BIT   7,D",		"BIT   7,E",
	"BIT   7,H",	"BIT   7,L",	"BIT   7,(I%^)",	"BIT   7,A",
	"RES   0,B",	"RES   0,C",	"RES   0,D",		"RES   0,E",
	"RES   0,H",	"RES   0,L",	"RES   0,(I%^)",	"RES   0,A",
	"RES   1,B",	"RES   1,C",	"RES   1,D",		"RES   1,E",
	"RES   1,H",	"RES   1,L",	"RES   1,(I%^)",	"RES   1,A",
	"RES   2,B",	"RES   2,C",	"RES   2,D",		"RES   2,E",
	"RES   2,H",	"RES   2,L",	"RES   2,(I%^)",	"RES   2,A",
	"RES   3,B",	"RES   3,C",	"RES   3,D",		"RES   3,E",
	"RES   3,H",	"RES   3,L",	"RES   3,(I%^)",	"RES   3,A",
	"RES   4,B",	"RES   4,C",	"RES   4,D",		"RES   4,E",
	"RES   4,H",	"RES   4,L",	"RES   4,(I%^)",	"RES   4,A",
	"RES   5,B",	"RES   5,C",	"RES   5,D",		"RES   5,E",
	"RES   5,H",	"RES   5,L",	"RES   5,(I%^)",	"RES   5,A",
	"RES   6,B",	"RES   6,C",	"RES   6,D",		"RES   6,E",
	"RES   6,H",	"RES   6,L",	"RES   6,(I%^)",	"RES   6,A",
	"RES   7,B",	"RES   7,C",	"RES   7,D",		"RES   7,E",
	"RES   7,H",	"RES   7,L",	"RES   7,(I%^)",	"RES   7,A",
	"SET   0,B",	"SET   0,C",	"SET   0,D",		"SET   0,E",
	"SET   0,H",	"SET   0,L",	"SET   0,(I%^)",	"SET   0,A",
	"SET   1,B",	"SET   1,C",	"SET   1,D",		"SET   1,E",
	"SET   1,H",	"SET   1,L",	"SET   1,(I%^)",	"SET   1,A",
	"SET   2,B",	"SET   2,C",	"SET   2,D",		"SET   2,E",
	"SET   2,H",	"SET   2,L",	"SET   2,(I%^)",	"SET   2,A",
	"SET   3,B",	"SET   3,C",	"SET   3,D",		"SET   3,E",
	"SET   3,H",	"SET   3,L",	"SET   3,(I%^)",	"SET   3,A",
	"SET   4,B",	"SET   4,C",	"SET   4,D",		"SET   4,E",
	"SET   4,H",	"SET   4,L",	"SET   4,(I%^)",	"SET   4,A",
	"SET   5,B",	"SET   5,C",	"SET   5,D",		"SET   5,E",
	"SET   5,H",	"SET   5,L",	"SET   5,(I%^)",	"SET   5,A",
	"SET   6,B",	"SET   6,C",	"SET   6,D",		"SET   6,E",
	"SET   6,H",	"SET   6,L",	"SET   6,(I%^)",	"SET   6,A",
	"SET   7,B",	"SET   7,C",	"SET   7,D",		"SET   7,E",
	"SET   7,H",	"SET   7,L",	"SET   7,(I%^)",	"SET   7,A"
};

/*-[ 汎用サブ ]-------------------------------------------------------------*/

/*
 *	データフェッチ
 */
static BYTE FASTCALL fetch(void)
{
	return mainmem_readbnio((WORD)(pc + (addpc++)));
}

/*
 *	データフェッチ(2バイト)
 */
static WORD FASTCALL fetch16(void)
{
	WORD dat;

	dat = mainmem_readbnio((WORD)(pc + (addpc++)));
	dat |= (WORD)(mainmem_readbnio((WORD)(pc + (addpc++))) << 8);
	return dat;
}

/*
 *	データ読み出し
 */
static BYTE FASTCALL read_byte(WORD addr)
{
	return mainmem_readbnio(addr);
}

/*
 *	16進1桁セット サブ
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
 *	16進2桁セット サブ
 */
static void FASTCALL sub2hex(BYTE dat, char *buffer)
{
	sub1hex((BYTE)(dat >> 4), buffer);
	sub1hex((BYTE)(dat & 0x0f), buffer );
}

/*
 *	16進2桁セット
 */
static void FASTCALL set2hex(BYTE dat)
{
	if (dat >= 0xa0) {
		strcat(linebuf, "0");
	}

	sub2hex(dat, linebuf);
	strcat(linebuf, "h");
}

/*
 *	16進4桁セット サブ
 */
static void FASTCALL sub4hex(WORD dat, char *buffer)
{
	sub2hex((BYTE)(dat >> 8), buffer);
	sub2hex((BYTE)(dat & 0xff), buffer);
}

/*
 *	16進4桁セット
 */
static void FASTCALL set4hex(WORD dat)
{
	if (dat >= 0xa000) {
		strcat(linebuf, "0");
	}
	sub4hex(dat, linebuf);
	strcat(linebuf, "h");
}

/*-[ メイン ]---------------------------------------------------------------*/

/*
 *	逆アセンブラ本体
 */
int FASTCALL disline80(int cpu, WORD pcreg, char *buffer)
{
	char tmp[64], ofsstr[8], index_char, offset, *str, *ptr;
	WORD dat;
	int i;
	int j;

	/* assert */
	ASSERT(buffer);
	ASSERT(cpu != MAINZ80);
	UNUSED(cpu);

	/* 初期設定 */
	pc         = pcreg;
	addpc      = 0;
	linebuf[0] = '\0';
	offset     = 0;
	index_char = '\0';

	/* 先頭のバイトを読み出し */
	opc = fetch();

	switch(opc) {
		case 0xCB:
			str = MnemonicsCB[fetch()];
			break;
		case 0xED:
			str = MnemonicsED[fetch()];
			break;
		case 0xDD:
			index_char = 'X';
			if ((opc = fetch()) != 0xCB) {
				str = MnemonicsXX[opc];
				offset = fetch();
			}
			else {
				offset = fetch();
				str = MnemonicsXCB[fetch()];
			}
			break;
		case 0xFD:
			index_char = 'Y';
			if ((opc = fetch()) != 0xCB) {
				str = MnemonicsXX[opc];
				offset = fetch();
			}
			else {
				offset = fetch();
				str = MnemonicsXCB[fetch()];
			}
			break;
		default:
			str = Mnemonics[opc];
	}

	if (!str) {
		strcpy(linebuf, "?");
	}
	else {
		if ((ptr = strchr(str, '^')) != 0) {
			strncpy(tmp, str, ptr - str);
			tmp[ptr - str] = '\0';

			sprintf(ofsstr, "%+ d", offset);

			strcat(tmp, ofsstr);
			strcat(tmp, ptr + 1);
		}
		else {
			strcpy(tmp, str);
		}
		if ((ptr = strchr(tmp, '%')) != 0) {
			*ptr = index_char;
		}

		if ((ptr = strchr(tmp, '*')) != 0) {
			strncpy(linebuf, tmp, ptr - tmp);
			linebuf[ptr - tmp] = '\0';

			set2hex(fetch());
			strcat(linebuf, ptr + 1);
		}
		else if ((ptr = strchr(tmp, '#')) != 0) {
			strncpy(linebuf, tmp, ptr - tmp);
			linebuf[ptr - tmp] = '\0';

			set4hex(fetch16());
			strcat(linebuf, ptr + 1);
		}
		else if ((ptr = strchr(tmp, '@')) != 0) {
			strncpy(linebuf, tmp, ptr - tmp);
			linebuf[ptr - tmp] = '\0';

			dat = fetch();
			set4hex((WORD)(((char)dat + (pc + addpc)) & 0xFFFF));
			strcat(linebuf, ptr + 1);
		}
		else {
			strcpy(linebuf, tmp);
		}
	}

	/* 命令データをセット */
	buffer[0] = '\0';

	sub4hex(pcreg, buffer);
	strcat(buffer, " ");
	for (i=0; i<addpc; i++) {
		sub2hex(read_byte((WORD)(pcreg + i)), buffer);
		strcat(buffer, " ");
	}

	/* bufferが20バイト+'\0'になるよう調整する */
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

#endif
