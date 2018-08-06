/*
 *	FM-7 EMULATOR "XM7"
 *
 *	Copyright (C) 1999-2017 ＰＩ．(Twitter:@xm6_original)
 *	Copyright (C) 2001-2017 Ryu Takegami (Twitter:@RyuTakegami)
 *
 *	[ 6809CPU(C版,Original Source from M.A.M.E.) ]
 *
 *	RHG履歴
 *	  2002.05.06		XM7 V2/V3用に改造・Warning対策でキャストの嵐(ヴァク
 *	  2003.02.22		CPU_x86.asmと同一のI/Fに変更、その他大幅改悪(ｗ
 *	  2003.02.23		2バイトLBRA($10 $20),2バイトLBSR($10 $8D)に対応
 *						インデックスアドレッシングの隠しポストバイトに対応
 *	  2003.05.23		NEG/NEGA/NEGB命令のキャリーフラグの挙動を修正
 *						イミディエイト以外のLDS命令を実行してもNMIがマスクさ
 *						れたままになる問題を修正
 */

#include	"xm7.h"

/*-[ ワーク ]---------------------------------------------------------------*/

BYTE fetch_op = 0;						/* 直前にフェッチした命令コード */

/*-[ 内部ワーク ]-----------------------------------------------------------*/

static cpu6809_t *currentcpu;			/* 現在実行中のCPU構造体 */
static WORD eaddr;						/* 実効アドレス */

/*-[ マクロ定義 ]-----------------------------------------------------------*/

#define CCREG			currentcpu->cc
#define AREG			currentcpu->acc.h.a
#define BREG			currentcpu->acc.h.b
#define DREG			currentcpu->acc.d
#define DPREG			currentcpu->dp
#define XREG			currentcpu->x
#define YREG			currentcpu->y
#define UREG			currentcpu->u
#define SREG			currentcpu->s
#define PCREG			currentcpu->pc
#define INTR			currentcpu->intr
#define CYCLE			currentcpu->cycle
#define TOTAL			currentcpu->total

#define READB(addr)		currentcpu->readmem((WORD)(addr))
#define READW(addr)		(WORD)((READB(addr) << 8) | READB((addr) + 1))
#define WRITEB(addr,b)	currentcpu->writemem((WORD)(addr), (BYTE)(b))
#define WRITEW(addr,w)	{WRITEB(addr, ((w) >> 8)); WRITEB((addr) + 1, ((w) & 0xff));}

/*-[ プロトタイプ宣言 ]-----------------------------------------------------*/

static void FASTCALL cpu_reset(cpu6809_t *CPU);

/*-[ テーブル ]-------------------------------------------------------------*/

#define E   0x80				/* 0x80は実効アドレスの計算が必要なコード */

/* 通常ページのサイクル表 */
static BYTE cycles1[] =
{
	/*    0   1   2   3   4   5   6   7   8   9   A   B   C   D   E   F */
  /*0*/	  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  3,  6,
  /*1*/	255,255,  2,  2,  0,  0,  5,  9,  3,  2,  3,  2,  2,  2,  8,  6,
  /*2*/	  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,
  /*3*/	E+4,E+4,E+4,E+4,  5,  5,  5,  5,  4,  5,  3,  6, 20, 11,  1, 19,
  /*4*/	  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,
  /*5*/	  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,
  /*6*/	E+6,E+6,E+6,E+6,E+6,E+6,E+6,E+6,E+6,E+6,E+6,E+6,E+6,E+6,E+3,E+6,
  /*7*/	  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  4,  7,
  /*8*/	  2,  2,  2,  4,  2,  2,  2,  2,  2,  2,  2,  2,  4,  7,  3,  3,
  /*9*/	  4,  4,  4,  6,  4,  4,  4,  4,  4,  4,  4,  4,  6,  7,  5,  5,
  /*A*/	E+4,E+4,E+4,E+6,E+4,E+4,E+4,E+4,E+4,E+4,E+4,E+4,E+6,E+7,E+5,E+5,
  /*B*/	  5,  5,  5,  7,  5,  5,  5,  5,  5,  5,  5,  5,  7,  8,  6,  6,
  /*C*/	  2,  2,  2,  4,  2,  2,  2,  2,  2,  2,  2,  2,  3,  0,  3,  3,
  /*D*/	  4,  4,  4,  6,  4,  4,  4,  4,  4,  4,  4,  4,  5,  5,  5,  5,
  /*E*/	E+4,E+4,E+4,E+6,E+4,E+4,E+4,E+4,E+4,E+4,E+4,E+4,E+5,E+5,E+5,E+5,
  /*F*/	  5,  5,  5,  7,  5,  5,  5,  5,  5,  5,  5,  5,  6,  6,  6,  6
};

/* $10ページのサイクル表 */
static BYTE cycles2[] =
{
	/*    0   1   2   3   4   5   6   7   8   9   A   B   C   D   E   F */
  /*0*/	  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
  /*1*/	  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
  /*2*/	  5,  5,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,
  /*3*/	E+0,E+0,E+0,E+0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 20,
  /*4*/	  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
  /*5*/	  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
  /*6*/	E+0,E+0,E+0,E+0,E+0,E+0,E+0,E+0,E+0,E+0,E+0,E+0,E+0,E+0,E+0,E+0,
  /*7*/	  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
  /*8*/	  0,  0,  0,  5,  0,  0,  0,  0,  0,  0,  0,  0,  5,  9,  4,  0,
  /*9*/	  0,  0,  0,  7,  0,  0,  0,  0,  0,  0,  0,  0,  7,  0,  6,  6,
  /*A*/	E+0,E+0,E+0,E+7,E+0,E+0,E+0,E+0,E+0,E+0,E+0,E+0,E+7,E+0,E+6,E+6,
  /*B*/	  0,  0,  0,  8,  0,  0,  0,  0,  0,  0,  0,  0,  8,  0,  7,  7,
  /*C*/	  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  4,  0,
  /*D*/	  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  6,  6,
  /*E*/	E+0,E+0,E+0,E+0,E+0,E+0,E+0,E+0,E+0,E+0,E+0,E+0,E+0,E+0,E+6,E+6,
  /*F*/	  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  7,  7
};

static BYTE flags8i[256] = {
0x04,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x0a,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,
0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,
0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,
0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,
0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,
0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,
0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,
0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08
};

static BYTE flags8d[256] = {
0x04,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x02,
0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,
0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,
0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,
0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,
0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,
0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,
0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,
0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08
};

/*-[ マクロ定義 ]-----------------------------------------------------------*/

/* イミディエイトアドレッシング */
#define IMMBYTE(b)		{b = READB(PCREG++);}
#define IMMWORD(w)		{w = READW(PCREG); PCREG += (WORD)2;}

/* ダイレクトアドレッシング */
#define DIRECT			{IMMBYTE(eaddr); eaddr |= (WORD)(DPREG << 8);}
#define DIRBYTE(b)		{DIRECT; b=READB(eaddr);}
#define DIRWORD(w)		{DIRECT; w=READW(eaddr);}

/* エクステンドアドレッシング */
#define EXTENDED		IMMWORD(eaddr)
#define EXTBYTE(b)		{IMMWORD(eaddr); b = READB(eaddr);}
#define EXTWORD(w)		{IMMWORD(eaddr); w = READW(eaddr);}

/* リラティブアドレッシング */
#define BRANCH(f)		{IMMBYTE(t); if (f) PCREG += SIGNED(t);}
#define LBRANCH(f)		{IMMWORD(t); if (f) PCREG += (WORD)t;}
#define NXORV			( (CCREG & 0x08) ^ ((CCREG & 0x02) << 2) )

/* フラグ操作 */
#define SEC				(CCREG |= 0x01)
#define SEZ				(CCREG |= 0x04)

#define CLR_HNZVC		(CCREG &= 0xd0)
#define CLR_NZV			(CCREG &= 0xf1)
#define CLR_NZVC		(CCREG &= 0xf0)
#define CLR_Z			(CCREG &= 0xfb)
#define CLR_NZC			(CCREG &= 0xf2)
#define CLR_ZC			(CCREG &= 0xfa)

#define SET_Z(a)		if (!a) SEZ
#define SET_Z8(a)		SET_Z((BYTE)a)
#define SET_Z16(a)		SET_Z((WORD)a)
#define SET_N8(a)		CCREG |= (BYTE)((a & 0x80) >> 4)
#define SET_N16(a)		CCREG |= (BYTE)((a & 0x8000) >> 12)
#define SET_H(a,b,r)	CCREG |= (BYTE)(((a^b^r) & 0x10) << 1)
#define SET_C8(a)		CCREG |= (BYTE)((a & 0x100) >> 8)
#define SET_C16(a)		CCREG |= (BYTE)((a & 0x10000) >> 16)
#define SET_V8(a,b,r)	CCREG |= (BYTE)(((a^b^r^(r >> 1)) & 0x80) >> 6)
#define SET_V16(a,b,r)	CCREG |= (BYTE)(((a^b^r^(r >> 1)) & 0x8000) >> 14)
#define SET_NZ8(a)		{SET_N8(a); SET_Z(a);}
#define SET_NZ16(a)		{SET_N16(a); SET_Z(a);}

#define SET_FLAGS8I(a)	{CCREG |= flags8i[(a) & 0xff];}
#define SET_FLAGS8D(a)	{CCREG |= flags8d[(a) & 0xff];}
#define SET_FLAGS8(a,b,r)	{SET_N8(r); SET_Z8(r); SET_V8(a,b,r); SET_C8(r);}
#define SET_FLAGS16(a,b,r)	{SET_N16(r); SET_Z16(r); SET_V16(a,b,r); SET_C16(r);}

/* 符号拡張補助 */
#define SIGNED(b)		((WORD)((b & 0x80) ? (b | 0xff00) :(b)))

/* プッシュ、ポップ */
#define PUSHBYTE(b)		{--SREG; WRITEB(SREG, b);}
#define PUSHWORD(w)		{SREG -= (WORD)2; WRITEW(SREG, w);}
#define PULLBYTE(b)		{b = READB(SREG); SREG++;}
#define PULLWORD(w)		{w = READW(SREG); SREG += (WORD)2;}
#define PSHUBYTE(b)		{--UREG; WRITEB(UREG, b);}
#define PSHUWORD(w)		{UREG -= (WORD)2; WRITEW(UREG, w);}
#define PULUBYTE(b)		{b = READB(UREG); UREG++;}
#define PULUWORD(w)		{w = READW(UREG); UREG += (WORD)2;}

/* レジスタ取得、設定 */
#define GETREG(val,reg)	switch (reg) { \
							case 0x0: val = DREG; break; \
							case 0x1: val = XREG; break; \
							case 0x2: val = YREG; break; \
							case 0x3: val = UREG; break; \
							case 0x4: val = SREG; break; \
							case 0x5: val = PCREG; break; \
							case 0x8: val = (WORD)(0xff00 | AREG); break; \
							case 0x9: val = (WORD)(0xff00 | BREG); break; \
							case 0xa: val = (WORD)(0xff00 | CCREG); break; \
							case 0xb: val = (WORD)(0xff00 | DPREG); break; \
							default: val = 0xffff;  break; } 
#define SETREG(val,reg)	switch (reg) { \
							case 0x0: DREG = val; break; \
							case 0x1: XREG = val; break; \
							case 0x2: YREG = val; break; \
							case 0x3: UREG = val; break; \
							case 0x4: SREG = val; break; \
							case 0x5: PCREG = val; break; \
							case 0x8: AREG = (BYTE)val; break; \
							case 0x9: BREG = (BYTE)val; break; \
							case 0xa: CCREG = (BYTE)val; break; \
							case 0xb: DPREG = (BYTE)val; break; \
							default: break; }

/*-[ 命令セット ]-----------------------------------------------------------*/

/* $00 NEG direct ?**** */
static void FASTCALL neg_di( void )
{
	WORD t, r;

	DIRBYTE(t);
	r = (WORD)-t;
	CLR_NZVC;
	SET_FLAGS8(0, t, r);
	if (r) {
		SEC;
	}
	WRITEB(eaddr, r);
}

/* $02 NGC direct */
static void FASTCALL com_di( void );
static void FASTCALL ngc_di( void )
{
	if (CCREG & 0x01) {
		com_di();
	}
	else {
		neg_di();
	}
}

/* $03 COM direct -**01 */
static void FASTCALL com_di( void )
{
	BYTE t;

	DIRBYTE(t);
	t = (BYTE)~t;
	CLR_NZV;
	SET_NZ8(t);
	SEC;
	WRITEB(eaddr,t);
}

/* $04 LSR direct -0*-* */
static void FASTCALL lsr_di( void )
{
	BYTE t;

	DIRBYTE(t);
	CLR_NZC;
	CCREG |= (BYTE)(t & 0x01);
	t>>=1;
	SET_Z8(t);
	WRITEB(eaddr,t);
}

/* $05 ILLEGAL */

/* $06 ROR direct -**-* */
static void FASTCALL ror_di( void )
{
	BYTE t,r;

	DIRBYTE(t);
	r = (BYTE)((CCREG & 0x01) << 7);
	CLR_NZC;
	CCREG |= (BYTE)(t & 0x01);
	r |= (BYTE)(t>>1);
	SET_NZ8(r);
	WRITEB(eaddr,r);
}

/* $07 ASR direct ?**-* */
static void FASTCALL asr_di( void )
{
	BYTE t;

	DIRBYTE(t);
	CLR_NZC;
	CCREG |= (BYTE)(t & 0x01);
	t>>=1;
	t |= (BYTE)((t&0x40)<<1);
	SET_NZ8(t);
	WRITEB(eaddr,t);
}

/* $08 ASL direct ?**** */
static void FASTCALL asl_di( void )
{
	WORD t, r;

	DIRBYTE(t);
	r = (WORD)(t << 1);
	CLR_NZVC;
	SET_FLAGS8(t, t, r);
	WRITEB(eaddr, r);
}

/* $09 ROL direct -**** */
static void FASTCALL rol_di( void )
{
	WORD t, r;

	DIRBYTE(t);
	r = (WORD)(CCREG & 0x01);
	r |= (WORD)(t << 1);
	CLR_NZVC;
	SET_FLAGS8(t, t, r);
	WRITEB(eaddr, r);
}

/* $0A DEC direct -***- */
static void FASTCALL dec_di( void )
{
	BYTE t;

	DIRBYTE(t);
	--t;
	CLR_NZV;
	SET_FLAGS8D(t);
	WRITEB(eaddr,t);
}

/* $0B DCC direct -**** */
static void FASTCALL dcc_di( void )
{
	BYTE t;

	DIRBYTE(t);
	--t;
	CLR_NZVC;
	SET_FLAGS8D(t);
	WRITEB(eaddr,t);
	if (t!=0) {
		SEC;
	}
}

/* $OC INC direct -***- */
static void FASTCALL inc_di( void )
{
	BYTE t;

	DIRBYTE(t);
	++t;
	CLR_NZV;
	SET_FLAGS8I(t);
	WRITEB(eaddr,t);
}

/* $OD TST direct -**0- */
static void FASTCALL tst_di( void )
{
	BYTE t;

	DIRBYTE(t);
	CLR_NZV;
	SET_NZ8(t);
}

/* $0E JMP direct ----- */
static void FASTCALL jmp_di( void )
{
	DIRECT;
	PCREG = eaddr;
}

/* $0F CLR direct -0100 */
static void FASTCALL clr_di( void )
{
	DIRECT;
	READB(eaddr);
	WRITEB(eaddr,0);
	CLR_NZVC;
	SEZ;
}

/* $12 NOP inherent ----- */
static void FASTCALL nop( void )
{
}

/* $13 SYNC inherent ----- */
static void FASTCALL sync( void )
{
	/* 初回チェック */
	if (!(INTR & INTR_SYNC_IN)) {
		INTR |= INTR_SYNC_IN;
		INTR &= ~INTR_SYNC_OUT;
		PCREG--;
		return;
	}

	/* 抜けチェック */
	if (!(INTR & INTR_SYNC_OUT)) {
		PCREG--;
		return;
	}

	/* SYNC終了 */
	INTR &= ~(INTR_SYNC_IN | INTR_SYNC_OUT);
}

/* $14 HALT */

/* $15 HALT */

/* $16 LBRA relative ----- */
static void FASTCALL lbra( void )
{
	IMMWORD(eaddr);
	PCREG += eaddr;
}

/* $17 LBSR relative ----- */
static void FASTCALL lbsr( void )
{
	IMMWORD(eaddr);
	PUSHWORD(PCREG);
	PCREG += eaddr;
}

/* $18 ASLCC inherent ****0 */
static void FASTCALL aslcc( void )
{
	if (CCREG & 0x04) {
		CCREG |= 0x01;
	}

	CCREG += CCREG;
	CCREG &= 0x3e;
}

/* $19 DAA inherent (AREG) -**0* */
static void FASTCALL daa( void )
{
	BYTE msn, lsn;
	WORD t, cf = 0;

	msn = (BYTE)(AREG & 0xf0);
	lsn = (BYTE)(AREG & 0x0f);
	if ( lsn>0x09 || CCREG &0x20 ) cf |= 0x06;
	if ( msn>0x80 && lsn>0x09 ) cf |= 0x60;
	if ( msn>0x90 || CCREG &0x01 ) cf |= 0x60;
	t = (WORD)(cf + AREG);
	CLR_NZV;
	SET_NZ8((BYTE)t);
	SET_C8(t);
	AREG = (BYTE)(t & 0xff);
}

/* $1A ORCC immediate ##### */
static void FASTCALL orcc( void )
{
	BYTE t;

	IMMBYTE(t);
	CCREG |=t;
}

/* $1B ILLEGAL */

/* $1C ANDCC immediate ##### */
static void FASTCALL andcc( void )
{
	BYTE t;

	IMMBYTE(t);
	CCREG &=t;
}

/* $1D SEX inherent -**0- */
static void FASTCALL sex( void )
{
	WORD t;

	t = SIGNED(BREG);
	DREG = t;
	CLR_NZV;
	SET_NZ16(t);
}

/* $1E EXG inherent ----- */
static void FASTCALL exg( void )
{
	WORD t1=0,t2=0;
	BYTE tb;

	IMMBYTE(tb);
	GETREG(t1,tb>>4);
	GETREG(t2,tb&15);
	SETREG(t2,tb>>4);
	SETREG(t1,tb&15);
}

/* $1F TFR inherent ----- */
static void FASTCALL tfr( void )
{
	BYTE tb;
	WORD t=0;

	IMMBYTE(tb);
	GETREG(t,tb>>4);
	SETREG(t,tb&15);
}

/* $20 BRA relative ----- */
static void FASTCALL bra( void )
{
	BYTE t;

	IMMBYTE(t);
	PCREG += SIGNED(t);
}

/* $21 BRN relative ----- */
static void FASTCALL brn( void )
{
	PCREG++;
}

/* $1021 LBRN relative ----- */
static void FASTCALL lbrn( void )
{
	PCREG += (WORD)2;
}

/* $22 BHI relative ----- */
static void FASTCALL bhi( void )
{
	BYTE t;

	BRANCH(!(CCREG & 0x05));
}

/* $1022 LBHI relative ----- */
static void FASTCALL lbhi( void )
{
	WORD t;

	LBRANCH(!(CCREG & 0x05));
}

/* $23 BLS relative ----- */
static void FASTCALL bls( void )
{
	BYTE t;

	BRANCH(CCREG & 0x05);
}

/* $1023 LBLS relative ----- */
static void FASTCALL lbls( void )
{
	WORD t;

	LBRANCH(CCREG & 0x05);
}

/* $24 BCC relative ----- */
static void FASTCALL bcc( void )
{
	BYTE t;

	BRANCH(!(CCREG & 0x01));
}

/* $1024 LBCC relative ----- */
static void FASTCALL lbcc( void )
{
	WORD t;

	LBRANCH(!(CCREG & 0x01));
}

/* $25 BCS relative ----- */
static void FASTCALL bcs( void )
{
	BYTE t;

	BRANCH(CCREG & 0x01);
}

/* $1025 LBCS relative ----- */
static void FASTCALL lbcs( void )
{
	WORD t;

	LBRANCH(CCREG & 0x01);
}

/* $26 BNE relative ----- */
static void FASTCALL bne( void )
{
	BYTE t;

	BRANCH(!(CCREG & 0x04));
}

/* $1026 LBNE relative ----- */
static void FASTCALL lbne( void )
{

	WORD t;

	LBRANCH(!(CCREG & 0x04));
}

/* $27 BEQ relative ----- */
static void FASTCALL beq( void )
{
	BYTE t;

	BRANCH(CCREG & 0x04);
}

/* $1027 LBEQ relative ----- */
static void FASTCALL lbeq( void )
{
	WORD t;

	LBRANCH(CCREG & 0x04);
}

/* $28 BVC relative ----- */
static void FASTCALL bvc( void )
{
	BYTE t;

	BRANCH(!(CCREG & 0x02));
}

/* $1028 LBVC relative ----- */
static void FASTCALL lbvc( void )
{
	WORD t;

	LBRANCH(!(CCREG & 0x02));
}

/* $29 BVS relative ----- */
static void FASTCALL bvs( void )
{
	BYTE t;

	BRANCH(CCREG & 0x02);
}

/* $1029 LBVS relative ----- */
static void FASTCALL lbvs( void )
{
	WORD t;

	LBRANCH(CCREG & 0x02);
}

/* $2A BPL relative ----- */
static void FASTCALL bpl( void )
{
	BYTE t;

	BRANCH(!(CCREG & 0x08));
}

/* $102A LBPL relative ----- */
static void FASTCALL lbpl( void )
{
	WORD t;

	LBRANCH(!(CCREG & 0x08));
}

/* $2B BMI relative ----- */
static void FASTCALL bmi( void )
{
	BYTE t;

	BRANCH(CCREG & 0x08);
}

/* $102B LBMI relative ----- */
static void FASTCALL lbmi( void )
{
	WORD t;

	LBRANCH(CCREG & 0x08);
}

/* $2C BGE relative ----- */
static void FASTCALL bge( void )
{
	BYTE t;

	BRANCH(!NXORV);
}

/* $102C LBGE relative ----- */
static void FASTCALL lbge( void )
{
	WORD t;

	LBRANCH(!NXORV);
}

/* $2D BLT relative ----- */
static void FASTCALL blt( void )
{
	BYTE t;

	BRANCH(NXORV);
}

/* $102D LBLT relative ----- */
static void FASTCALL lblt( void )
{
	WORD t;

	LBRANCH(NXORV);
}

/* $2E BGT relative ----- */
static void FASTCALL bgt( void )
{
	BYTE t;

	BRANCH(!(NXORV || (CCREG & 0x04)));
}

/* $102E LBGT relative ----- */
static void FASTCALL lbgt( void )
{
	WORD t;

	LBRANCH(!(NXORV || (CCREG & 0x04)));
}

/* $2F BLE relative ----- */
static void FASTCALL ble( void )
{
	BYTE t;

	BRANCH(NXORV || (CCREG & 0x04));
}

/* $102F LBLE relative ----- */
static void FASTCALL lble( void )
{
	WORD t;

	LBRANCH(NXORV || (CCREG & 0x04));
}

/* $30 LEAX indexed --*-- */
static void FASTCALL leax( void )
{
	XREG = eaddr;
	CLR_Z;
	SET_Z(XREG);
}

/* $31 LEAY indexed --*-- */
static void FASTCALL leay( void )
{
	YREG = eaddr;
	CLR_Z;
	SET_Z(YREG);
}

/* $32 LEAS indexed ----- */
static void FASTCALL leas( void )
{
	SREG = eaddr;
}

/* $33 LEAU indexed ----- */
static void FASTCALL leau( void )
{
	UREG = eaddr;
}

/* $34 PSHS inherent ----- */
static void FASTCALL pshs( void )
{
	BYTE t;

	IMMBYTE(t);
	if(t & 0x80) { PUSHWORD(PCREG); CYCLE+=(WORD)2; }
	if(t & 0x40) { PUSHWORD(UREG); CYCLE+=(WORD)2; }
	if(t & 0x20) { PUSHWORD(YREG); CYCLE+=(WORD)2; }
	if(t & 0x10) { PUSHWORD(XREG); CYCLE+=(WORD)2; }
	if(t & 0x08) { PUSHBYTE(DPREG); CYCLE++; }
	if(t & 0x04) { PUSHBYTE(BREG); CYCLE++; }
	if(t & 0x02) { PUSHBYTE(AREG); CYCLE++; }
	if(t & 0x01) { PUSHBYTE(CCREG); CYCLE++; }
}

/* $35 PULS inherent ----- */
static void FASTCALL puls( void )
{
	BYTE t;

	IMMBYTE(t);
	if(t & 0x01) { PULLBYTE(CCREG); CYCLE++; }
	if(t & 0x02) { PULLBYTE(AREG); CYCLE++; }
	if(t & 0x04) { PULLBYTE(BREG); CYCLE++; }
	if(t & 0x08) { PULLBYTE(DPREG); CYCLE++; }
	if(t & 0x10) { PULLWORD(XREG); CYCLE+=(WORD)2; }
	if(t & 0x20) { PULLWORD(YREG); CYCLE+=(WORD)2; }
	if(t & 0x40) { PULLWORD(UREG); CYCLE+=(WORD)2; }
	if(t & 0x80) { PULLWORD(PCREG); CYCLE+=(WORD)2; }
}

/* $36 PSHU inherent ----- */
static void FASTCALL pshu( void )
{
	BYTE t;

	IMMBYTE(t);
	if(t & 0x80) { PSHUWORD(PCREG); CYCLE+=(WORD)2; }
	if(t & 0x40) { PSHUWORD(SREG); CYCLE+=(WORD)2; }
	if(t & 0x20) { PSHUWORD(YREG); CYCLE+=(WORD)2; }
	if(t & 0x10) { PSHUWORD(XREG); CYCLE+=(WORD)2; }
	if(t & 0x08) { PSHUBYTE(DPREG); CYCLE++; }
	if(t & 0x04) { PSHUBYTE(BREG); CYCLE++; }
	if(t & 0x02) { PSHUBYTE(AREG); CYCLE++; }
	if(t & 0x01) { PSHUBYTE(CCREG); CYCLE++; }
}

/* 37 PULU inherent ----- */
static void FASTCALL pulu( void )
{
	BYTE t;

	IMMBYTE(t);
	if(t & 0x01) { PULUBYTE(CCREG); CYCLE++; }
	if(t & 0x02) { PULUBYTE(AREG); CYCLE++; }
	if(t & 0x04) { PULUBYTE(BREG); CYCLE++; }
	if(t & 0x08) { PULUBYTE(DPREG); CYCLE++; }
	if(t & 0x10) { PULUWORD(XREG); CYCLE+=(WORD)2; }
	if(t & 0x20) { PULUWORD(YREG); CYCLE+=(WORD)2; }
	if(t & 0x40) { PULUWORD(SREG); CYCLE+=(WORD)2; }
	if(t & 0x80) { PULUWORD(PCREG); CYCLE+=(WORD)2; }
}

/* $38 ILLEGAL */

/* $39 RTS inherent ----- */
static void FASTCALL rts( void )
{
	PULLWORD(PCREG);
}

/* $3A ABX inherent ----- */
static void FASTCALL abx( void )
{
	XREG += BREG;
}

/* $3B RTI inherent ##### */
static void FASTCALL rti( void )
{
	PULLBYTE(CCREG);

	if (CCREG & 0x80) {
		PULLBYTE(AREG);
		PULLBYTE(BREG);
		PULLBYTE(DPREG);
		PULLWORD(XREG);
		PULLWORD(YREG);
		PULLWORD(UREG);
		CYCLE += (WORD)9;
	}
	PULLWORD(PCREG);
}

/* $3C CWAI immediate ----1 */
static void FASTCALL cwai( void )
{
	BYTE t;

	/* CWAIチェック */
	if (!(INTR & INTR_CWAI_IN)) {
		IMMBYTE(t);
		CCREG &= t;
		INTR |= INTR_CWAI_IN;
		INTR &= ~INTR_CWAI_OUT;
		PCREG -= (WORD)2;
		return;
	}

	/* CWAI終了チェック */
	if (!(INTR & INTR_CWAI_OUT)) {
		PCREG--;
		return;
	}

	/* CWAI終了 */
	INTR &= ~INTR_CWAI_IN;
	INTR &= ~INTR_CWAI_OUT;
	PCREG++;
}

/* $3D MUL inherent --*-@ */
static void FASTCALL mul( void )
{
	WORD t;

	t = (WORD)(AREG * BREG);
	CLR_ZC;
	SET_Z16(t);
	if (t & 0x80) SEC;
	DREG = (WORD)t;
}

/* $3E RST inherent ----- */
static void FASTCALL rst( void )
{
	cpu_reset(NULL);
}

/* $3F SWI (SWI2 SWI3) absolute indirect ----- */
static void FASTCALL swi( void )
{
	CCREG |= 0x80;

	PUSHWORD(PCREG);
	PUSHWORD(UREG);
	PUSHWORD(YREG);
	PUSHWORD(XREG);
	PUSHBYTE(DPREG);
	PUSHBYTE(BREG);
	PUSHBYTE(AREG);
	PUSHBYTE(CCREG);

	/* SWIのみ、ORCC #$50の効果がある */
	CCREG |= 0x50;

	PCREG = READW(0xfffa);
}

/* $103F SWI2 absolute indirect ----- */
static void FASTCALL swi2( void )
{
	CCREG |= 0x80;

	PUSHWORD(PCREG);
	PUSHWORD(UREG);
	PUSHWORD(YREG);
	PUSHWORD(XREG);
	PUSHBYTE(DPREG);
	PUSHBYTE(BREG);
	PUSHBYTE(AREG);
	PUSHBYTE(CCREG);

	PCREG = READW(0xfff4);
}

/* $113F SWI3 absolute indirect ----- */
static void FASTCALL swi3( void )
{
	CCREG |= 0x80;

	PUSHWORD(PCREG);
	PUSHWORD(UREG);
	PUSHWORD(YREG);
	PUSHWORD(XREG);
	PUSHBYTE(DPREG);
	PUSHBYTE(BREG);
	PUSHBYTE(AREG);
	PUSHBYTE(CCREG);

	PCREG = READW(0xfff2);
}

/* $40 NEGA inherent ?**** */
static void FASTCALL nega( void )
{
	WORD r;

	r = (WORD)-AREG;
	CLR_NZVC;
	SET_FLAGS8(0, AREG, r);
	if (r) {
		SEC;
	}
	AREG = (BYTE)r;
}

/* $42 NGCA inherent */
static void FASTCALL coma( void );
static void FASTCALL ngca( void )
{
	if (CCREG & 0x01) {
		coma();
	}
	else {
		nega();
	}
}

/* $43 COMA inherent -**01 */
static void FASTCALL coma( void )
{
	AREG = (BYTE)~AREG;
	CLR_NZV;
	SET_NZ8(AREG);
	SEC;
}

/* $44 LSRA inherent -0*-* */
static void FASTCALL lsra( void )
{
	CLR_NZC;
	CCREG |= (BYTE)(AREG&0x01);
	AREG>>=1;
	SET_Z8(AREG);
}

/* $45 ILLEGAL */

/* $46 RORA inherent -**-* */
static void FASTCALL rora( void )
{
	BYTE r;

	r = (BYTE)((CCREG &0x01) << 7);
	CLR_NZC;
	CCREG |= (BYTE)(AREG&0x01);
	r |= (BYTE)(AREG>>1);
	SET_NZ8(r);
	AREG=r;
}

/* $47 ASRA inherent ?**-* */
static void FASTCALL asra( void )
{
	CLR_NZC;
	CCREG |= (BYTE)(AREG&0x01);
	AREG >>= 1;
	AREG |= (BYTE)((AREG&0x40) << 1);
	SET_NZ8(AREG);
}

/* $48 ASLA inherent ?**** */
static void FASTCALL asla( void )
{
	WORD r;

	r = (WORD)(AREG << 1);
	CLR_NZVC;
	SET_FLAGS8(AREG, AREG, r);
	AREG = (BYTE)r;
}

/* $49 ROLA inherent -**** */
static void FASTCALL rola( void )
{
	WORD t, r;

	t = AREG;
	r = (WORD)(CCREG & 0x01);
	r |= (WORD)(t << 1);
	CLR_NZVC;
	SET_FLAGS8(t, t, r);
	AREG = (BYTE)r;
}

/* $4A DECA inherent -***- */
static void FASTCALL deca( void )
{
	--AREG;
	CLR_NZV;
	SET_FLAGS8D(AREG);
}

/* $4B DCCA inherent -**** */
static void FASTCALL dcca( void )
{
	--AREG;
	CLR_NZVC;
	SET_FLAGS8D(AREG);
	if (AREG!=0) {
		SEC;
	}
}

/* $4C INCA inherent -***- */
static void FASTCALL inca( void )
{
	++AREG;
	CLR_NZV;
	SET_FLAGS8I(AREG);
}

/* $4D TSTA inherent -**0- */
static void FASTCALL tsta( void )
{
	CLR_NZV;
	SET_NZ8(AREG);
}

/* $4E CLCA inherent -010- */
static void FASTCALL clca( void )
{
	AREG=0;
	CLR_NZV;
	SEZ;
}

/* $4F CLRA inherent -0100 */
static void FASTCALL clra( void )
{
	AREG=0;
	CLR_NZVC;
	SEZ;
}

/* $50 NEGB inherent ?**** */
static void FASTCALL negb( void )
{
	WORD r;

	r = (WORD)-BREG;
	CLR_NZVC;
	SET_FLAGS8(0, BREG, r);
	if (r) {
		SEC;
	}
	BREG = (BYTE)r;
}

/* $52 NGCB inherent */
static void FASTCALL comb( void );
static void FASTCALL ngcb( void )
{
	if (CCREG & 0x01) {
		comb();
	}
	else {
		negb();
	}
}

/* $53 COMB inherent -**01 */
static void FASTCALL comb( void )
{
	BREG = (BYTE)~BREG;
	CLR_NZV; SET_NZ8(BREG); SEC;
}

/* $54 LSRB inherent -0*-* */
static void FASTCALL lsrb( void )
{
	CLR_NZC; CCREG |= (BYTE)(BREG&0x01);
	BREG>>=1; SET_Z8(BREG);
}

/* $55 ILLEGAL */

/* $56 RORB inherent -**-* */
static void FASTCALL rorb( void )
{
	BYTE r;
	r= (BYTE)((CCREG &0x01)<<7);
	CLR_NZC; CCREG |= (BYTE)(BREG&0x01);
	r |= (BYTE)(BREG>>1); SET_NZ8(r);
	BREG=r;
}

/* $57 ASRB inherent ?**-* */
static void FASTCALL asrb( void )
{
	CLR_NZC; CCREG |= (BYTE)(BREG&0x01);
	BREG>>=1; BREG|=(BYTE)((BREG&0x40)<<1);
	SET_NZ8(BREG);
}

/* $58 ASLB inherent ?**** */
static void FASTCALL aslb( void )
{
	WORD r;

	r= (WORD)(BREG << 1);
	CLR_NZVC;
	SET_FLAGS8(BREG, BREG, r);
	BREG = (BYTE)r;
}

/* $59 ROLB inherent -**** */
static void FASTCALL rolb( void )
{
	WORD t, r;

	t = (WORD)BREG;
	r = (WORD)(CCREG & 0x01);
	r |= (WORD)(t << 1);
	CLR_NZVC;
	SET_FLAGS8(t, t, r);
	BREG = (BYTE)r;
}

/* $5A DECB inherent -***- */
static void FASTCALL decb( void )
{
	--BREG;
	CLR_NZV;
	SET_FLAGS8D(BREG);
}

/* $5B DCCB inherent -**** */
static void FASTCALL dccb( void )
{
	--BREG;
	CLR_NZVC;
	SET_FLAGS8D(BREG);
	if (BREG!=0) {
		SEC;
	}
}

/* $5C INCB inherent -***- */
static void FASTCALL incb( void )
{
	++BREG;
	CLR_NZV;
	SET_FLAGS8I(BREG);
}

/* $5D TSTB inherent -**0- */
static void FASTCALL tstb( void )
{
	CLR_NZV;
	SET_NZ8(BREG);
}

/* $5E CLCB inherent -010- */
static void FASTCALL clcb( void )
{
	BREG=0;
	CLR_NZV;
	SEZ;
}

/* $5F CLRB inherent -0100 */
static void FASTCALL clrb( void )
{
	BREG=0;
	CLR_NZVC;
	SEZ;
}

/* $60 NEG indexed ?**** */
static void FASTCALL neg_ix( void )
{
	WORD t, r;
	
	t = (WORD)READB(eaddr);
	r = (WORD)-t;
	CLR_NZVC;
	SET_FLAGS8(0, t, r);
	if (r) {
		SEC;
	}
	WRITEB(eaddr, r);
}

/* $62 NGC indexed */
static void FASTCALL com_ix( void );
static void FASTCALL ngc_ix( void )
{
	if (CCREG & 0x01) {
		com_ix();
	}
	else {
		neg_ix();
	}
}

/* $63 COM indexed -**01 */
static void FASTCALL com_ix( void )
{
	BYTE t;
	t = (BYTE)(~READB(eaddr));
	CLR_NZV; SET_NZ8(t); SEC;
	WRITEB(eaddr,t);
}

/* $64 LSR indexed -0*-* */
static void FASTCALL lsr_ix( void )
{
	BYTE t;
	t=READB(eaddr); CLR_NZC; CCREG |= (BYTE)(t & 0x01);
	t>>=1; SET_Z8(t);
	WRITEB(eaddr,t);
}

/* $65 ILLEGAL */

/* $66 ROR indexed -**-* */
static void FASTCALL ror_ix( void )
{
	BYTE t,r;
	t=READB(eaddr); r=(BYTE)((CCREG &0x01)<<7);
	CLR_NZC; CCREG |= (BYTE)(t & 0x01);
	r |= (BYTE)(t>>1); SET_NZ8(r);
	WRITEB(eaddr,r);
}

/* $67 ASR indexed ?**-* */
static void FASTCALL asr_ix( void )
{
	BYTE t;
	t=READB(eaddr); CLR_NZC; CCREG |= (BYTE)(t & 0x01);
	t>>=1; t|=(BYTE)((t&0x40)<<1);
	SET_NZ8(t);
	WRITEB(eaddr,t);
}

/* $68 ASL indexed ?**** */
static void FASTCALL asl_ix( void )
{
	WORD t, r;

	t = (WORD)READB(eaddr);
	r = (WORD)(t << 1);
	CLR_NZVC;
	SET_FLAGS8(t, t, r);
	WRITEB(eaddr, r);
}

/* $69 ROL indexed -**** */
static void FASTCALL rol_ix( void )
{
	WORD t, r;

	t = (WORD)READB(eaddr);
	r = (WORD)(CCREG &0x01);
	r |= (WORD)(t << 1);
	CLR_NZVC;
	SET_FLAGS8(t, t, r);
	WRITEB(eaddr, r);
}

/* $6A DEC indexed -***- */
static void FASTCALL dec_ix( void )
{
	BYTE t;
	t=(BYTE)(READB(eaddr)-1);
	CLR_NZV; SET_FLAGS8D(t);
	WRITEB(eaddr,t);
}

/* $6B DCC indexed -**** */
static void FASTCALL dcc_ix( void )
{
	BYTE t;
	t=(BYTE)(READB(eaddr)-1);
	CLR_NZVC; SET_FLAGS8D(t);
	WRITEB(eaddr,t);
	if (t!=0) {
		SEC;
	}
}

/* $6C INC indexed -***- */
static void FASTCALL inc_ix( void )
{
	BYTE t;
	t=(BYTE)(READB(eaddr)+1);
	CLR_NZV; SET_FLAGS8I(t);
	WRITEB(eaddr,t);
}

/* $6D TST indexed -**0- */
static void FASTCALL tst_ix( void )
{
	BYTE t;

	t = READB(eaddr);
	CLR_NZV;
	SET_NZ8(t);
}

/* $6E JMP indexed ----- */
static void FASTCALL jmp_ix( void )
{
	PCREG = eaddr;
}

/* $6F CLR indexed -0100 */
static void FASTCALL clr_ix( void )
{
	READB(eaddr);
	WRITEB(eaddr,0);
	CLR_NZVC;
	SEZ;
}

/* $70 NEG extended ?**** */
static void FASTCALL neg_ex( void )
{
	WORD t, r;

	EXTBYTE(t);
	r = (WORD)-t;
	CLR_NZVC;
	SET_FLAGS8(0, t, r);
	if (r) {
		SEC;
	}
	WRITEB(eaddr, r);
}

/* $72 NGC extended */
static void FASTCALL com_ex( void );
static void FASTCALL ngc_ex( void )
{
	if (CCREG & 0x01) {
		com_ex();
	}
	else {
		neg_ex();
	}
}

/* $73 COM extended -**01 */
static void FASTCALL com_ex( void )
{
	BYTE t;
	EXTBYTE(t); t = (BYTE)~t;
	CLR_NZV; SET_NZ8(t); SEC;
	WRITEB(eaddr,t);
}

/* $74 LSR extended -0*-* */
static void FASTCALL lsr_ex( void )
{
	BYTE t;
	EXTBYTE(t); CLR_NZC; CCREG |= (BYTE)(t & 0x01);
	t>>=1; SET_Z8(t);
	WRITEB(eaddr,t);
}

/* $75 ILLEGAL */

/* $76 ROR extended -**-* */
static void FASTCALL ror_ex( void )
{
	BYTE t,r;
	EXTBYTE(t); r= (BYTE)((CCREG &0x01)<<7);
	CLR_NZC; CCREG |= (BYTE)(t & 0x01);
	r |= (BYTE)(t>>1); SET_NZ8(r);
	WRITEB(eaddr,r);
}

/* $77 ASR extended ?**-* */
static void FASTCALL asr_ex( void )
{
	BYTE t;
	EXTBYTE(t); CLR_NZC; CCREG |= (BYTE)(t & 0x01);
	t>>=1; t|=(BYTE)((t&0x40)<<1);
	SET_NZ8(t);
	WRITEB(eaddr,t);
}

/* $78 ASL extended ?**** */
static void FASTCALL asl_ex( void )
{
	WORD t, r;

	EXTBYTE(t);
	r = (WORD)(t << 1);
	CLR_NZVC;
	SET_FLAGS8(t, t, r);
	WRITEB(eaddr, r);
}

/* $79 ROL extended -**** */
static void FASTCALL rol_ex( void )
{
	WORD t, r;

	EXTBYTE(t);
	r = (WORD)(CCREG & 0x01);
	r |= (WORD)(t << 1);
	CLR_NZVC;
	SET_FLAGS8(t, t, r);
	WRITEB(eaddr, r);
}

/* $7A DEC extended -***- */
static void FASTCALL dec_ex( void )
{
	BYTE t;
	EXTBYTE(t); --t;
	CLR_NZV; SET_FLAGS8D(t);
	WRITEB(eaddr,t);
}

/* $7B DCC extended -**** */
static void FASTCALL dcc_ex( void )
{
	BYTE t;
	EXTBYTE(t); --t;
	CLR_NZV; SET_FLAGS8D(t);
	WRITEB(eaddr,t);
	if (t!=0) {
		SEC;
	}
}

/* $7C INC extended -***- */
static void FASTCALL inc_ex( void )
{
	BYTE t;
	EXTBYTE(t); ++t;
	CLR_NZV; SET_FLAGS8I(t);
	WRITEB(eaddr,t);
}

/* $7D TST extended -**0- */
static void FASTCALL tst_ex( void )
{
	BYTE t;

	EXTBYTE(t);
	CLR_NZV;
	SET_NZ8(t);
}

/* $7E JMP extended ----- */
static void FASTCALL jmp_ex( void )
{
	EXTENDED;
	PCREG = eaddr;
}

/* $7F CLR extended -0100 */
static void FASTCALL clr_ex( void )
{
	EXTENDED;
	READB(eaddr);
	WRITEB(eaddr,0);
	CLR_NZVC;
	SEZ;
}

/* $80 SUBA immediate ?**** */
static void FASTCALL suba_im( void )
{
	WORD t, r;

	IMMBYTE(t);
	r = (WORD)(AREG - t);
	CLR_NZVC;
	SET_FLAGS8(AREG, t, r);
	AREG = (BYTE)r;
}

/* $81 CMPA immediate ?**** */
static void FASTCALL cmpa_im( void )
{
	WORD t, r;

	IMMBYTE(t);
	r = (WORD)(AREG - t);
	CLR_NZVC;
	SET_FLAGS8(AREG, t, r);
}

/* $82 SBCA immediate ?**** */
static void FASTCALL sbca_im( void )
{
	WORD t, r;

	IMMBYTE(t);
	r = (WORD)(AREG - t - (CCREG &0x01));
	CLR_NZVC;
	SET_FLAGS8(AREG, t, r);
	AREG = (BYTE)r;
}

/* $83 SUBD (CMPD CMPU) immediate -**** */
static void FASTCALL subd_im( void )
{
	DWORD t, r;

	IMMWORD(t); t &= 0xffff;
	r = DREG - t;
	CLR_NZVC;
	SET_FLAGS16(DREG, t, r);
	DREG = (WORD)r;
}

/* $1083 CMPD immediate -**** */
static void FASTCALL cmpd_im( void )
{
	DWORD t, r;

	IMMWORD(t); t &= 0xffff;
	r = DREG - t;
	CLR_NZVC;
	SET_FLAGS16(DREG, t, r);
}

/* $1183 CMPU immediate -**** */
static void FASTCALL cmpu_im( void )
{
	DWORD r, b;

	IMMWORD(b); b &= 0xffff;
	r = UREG-b;
	CLR_NZVC;
	SET_FLAGS16(UREG,b,r);
}

/* $84 ANDA immediate -**0- */
static void FASTCALL anda_im( void )
{
	BYTE t;

	IMMBYTE(t); AREG &= t;
	CLR_NZV; SET_NZ8(AREG);
}

/* $85 BITA immediate -**0- */
static void FASTCALL bita_im( void )
{
	BYTE t, r;

	IMMBYTE(t); r = AREG & t;
	CLR_NZV; SET_NZ8(r);
}

/* $86 LDA immediate -**0- */
static void FASTCALL lda_im( void )
{
	IMMBYTE(AREG);
	CLR_NZV;
	SET_NZ8(AREG);
}

/* $87 FLAG immediate -100- */
static void FASTCALL flag8_im( void )
{
	BYTE b;

	IMMBYTE(b);
	CLR_NZV;
	CCREG |= 0x08;
}

/* $88 EORA immediate -**0- */
static void FASTCALL eora_im( void )
{
	BYTE t;
	IMMBYTE(t); AREG ^= t;
	CLR_NZV; SET_NZ8(AREG);
}

/* $89 ADCA immediate ***** */
static void FASTCALL adca_im( void )
{
	WORD t, r;

	IMMBYTE(t);
	r = (WORD)(AREG + t + (CCREG &0x01));
	CLR_HNZVC;
	SET_FLAGS8(AREG, t, r);
	SET_H(AREG, t, r);
	AREG = (BYTE)r;
}

/* $8A ORA immediate -**0- */
static void FASTCALL ora_im( void )
{
	BYTE t;
	IMMBYTE(t); AREG |= t;
	CLR_NZV; SET_NZ8(AREG);
}

/* $8B ADDA immediate ***** */
static void FASTCALL adda_im( void )
{
	WORD t, r;

	IMMBYTE(t);
	r = (WORD)(AREG + t);
	CLR_HNZVC;
	SET_FLAGS8(AREG, t, r);
	SET_H(AREG, t, r);
	AREG = (BYTE)r;
}

/* $8C CMPX (CMPY CMPS) immediate -**** */
static void FASTCALL cmpx_im( void )
{
	DWORD r, d, b;

	IMMWORD(b); b &= 0xffff;
	d = XREG; r = d - b;
	CLR_NZVC; SET_FLAGS16(d, b, r);
}

/* $108C CMPY immediate -**** */
static void FASTCALL cmpy_im( void )
{
	DWORD r,d,b;
	IMMWORD(b); b &= 0xffff;
	d = YREG; r = d-b;
	CLR_NZVC; SET_FLAGS16(d,b,r);
}

/* $118C CMPS immediate -**** */
static void FASTCALL cmps_im( void )
{
	DWORD r,d,b;

	IMMWORD(b);
	b &= 0xffff;
	d = SREG;
	r = d-b;
	CLR_NZVC;
	SET_FLAGS16(d,b,r);
}

/* $8D BSR ----- */
static void FASTCALL bsr( void )
{
	BYTE t;

	IMMBYTE(t);
	PUSHWORD(PCREG);
	PCREG += SIGNED(t);
}

/* $8E LDX (LDY) immediate -**0- */
static void FASTCALL ldx_im( void )
{
	IMMWORD(XREG);
	CLR_NZV;
	SET_NZ16(XREG);
}

/* $108E LDY immediate -**0- */
static void FASTCALL ldy_im( void )
{
	IMMWORD(YREG);
	CLR_NZV;
	SET_NZ16(YREG);
}

/* $8F FLAG immediate -100- */
static void FASTCALL flag16_im( void )
{
	WORD w;

	IMMWORD(w);
	CLR_NZV;
	CCREG |= 0x08;
}

/* is this a legal instruction? */
/* $108F STY immediate -**0- */
static void FASTCALL sty_im( void )
{
}

/* $90 SUBA direct ?**** */
static void FASTCALL suba_di( void )
{
	WORD t, r;

	DIRBYTE(t);
	r = (WORD)(AREG - t);
	CLR_NZVC;
	SET_FLAGS8(AREG, t, r);
	AREG = (BYTE)r;
}

/* $91 CMPA direct ?**** */
static void FASTCALL cmpa_di( void )
{
	WORD t, r;

	DIRBYTE(t);
	r = (WORD)(AREG - t);
	CLR_NZVC;
	SET_FLAGS8(AREG, t, r);
}

/* $92 SBCA direct ?**** */
static void FASTCALL sbca_di( void )
{
	WORD t, r;

	DIRBYTE(t);
	r = (WORD)(AREG - t - (CCREG &0x01));
	CLR_NZVC;
	SET_FLAGS8(AREG, t, r);
	AREG = (BYTE)r;
}

/* $93 SUBD (CMPD CMPU) direct -**** */
static void FASTCALL subd_di( void )
{
	DWORD b, r;

	DIRWORD(b); b &= 0xffff;
	r = DREG - b;
	CLR_NZVC;
	SET_FLAGS16(DREG, b, r);
	DREG = (WORD)r;
}

/* $1093 CMPD direct -**** */
static void FASTCALL cmpd_di( void )
{
	DWORD b, r;

	DIRWORD(b); b &= 0xffff;
	r = DREG - b;
	CLR_NZVC;
	SET_FLAGS16(DREG, b, r);
}

/* $1193 CMPU direct -**** */
static void FASTCALL cmpu_di( void )
{
	DWORD r,b;
	DIRWORD(b); b &= 0xffff;
	r = UREG-b;
	CLR_NZVC; SET_FLAGS16(UREG,b,r);
}

/* $94 ANDA direct -**0- */
static void FASTCALL anda_di( void )
{
	BYTE t;
	DIRBYTE(t); AREG &= t;
	CLR_NZV; SET_NZ8(AREG);
}

/* $95 BITA direct -**0- */
static void FASTCALL bita_di( void )
{
	BYTE t,r;
	DIRBYTE(t); r = AREG&t;
	CLR_NZV; SET_NZ8(r);
}

/* $96 LDA direct -**0- */
static void FASTCALL lda_di( void )
{
	DIRBYTE(AREG);
	CLR_NZV; SET_NZ8(AREG);
}

/* $97 STA direct -**0- */
static void FASTCALL sta_di( void )
{
	CLR_NZV; SET_NZ8(AREG);
	DIRECT; WRITEB(eaddr,AREG);
}

/* $98 EORA direct -**0- */
static void FASTCALL eora_di( void )
{
	BYTE t;
	DIRBYTE(t); AREG ^= t;
	CLR_NZV; SET_NZ8(AREG);
}

/* $99 ADCA direct ***** */
static void FASTCALL adca_di( void )
{
	WORD t, r;

	DIRBYTE(t);
	r = (WORD)(AREG + t + (CCREG &0x01));
	CLR_HNZVC;
	SET_FLAGS8(AREG, t, r);
	SET_H(AREG, t, r);
	AREG = (BYTE)r;
}

/* $9A ORA direct -**0- */
static void FASTCALL ora_di( void )
{
	BYTE t;
	DIRBYTE(t); AREG |= t;
	CLR_NZV; SET_NZ8(AREG);
}

/* $9B ADDA direct ***** */
static void FASTCALL adda_di( void )
{
	WORD t, r;

	DIRBYTE(t);
	r = (WORD)(AREG + t);
	CLR_HNZVC;
	SET_FLAGS8(AREG, t, r);
	SET_H(AREG, t, r);
	AREG = (BYTE)r;
}

/* $9C CMPX (CMPY CMPS) direct -**** */
static void FASTCALL cmpx_di( void )
{
	DWORD r,d,b;

	DIRWORD(b);
	b &= 0xffff;
	d = XREG;
	r = d - b;
	CLR_NZVC;
	SET_FLAGS16(d,b,r);
}

/* $109C CMPY direct -**** */
static void FASTCALL cmpy_di( void )
{
	DWORD r,d,b;

	DIRWORD(b);
	b &= 0xffff;
	d = YREG;
	r = d - b;
	CLR_NZVC;
	SET_FLAGS16(d,b,r);
}

/* $119C CMPS direct -**** */
static void FASTCALL cmps_di( void )
{
	DWORD r,d,b;

	DIRWORD(b);
	b &= 0xffff;
	d = SREG;
	r = d - b;
	CLR_NZVC;
	SET_FLAGS16(d,b,r);
}

/* $9D JSR direct ----- */
static void FASTCALL jsr_di( void )
{
	DIRECT;
	PUSHWORD(PCREG);
	PCREG = eaddr;
}

/* $9E LDX (LDY) direct -**0- */
static void FASTCALL ldx_di( void )
{
	DIRWORD(XREG);
	CLR_NZV;
	SET_NZ16(XREG);
}

/* $109E LDY direct -**0- */
static void FASTCALL ldy_di( void )
{
	DIRWORD(YREG);
	CLR_NZV;
	SET_NZ16(YREG);
}

/* $9F STX (STY) direct -**0- */
static void FASTCALL stx_di( void )
{
	CLR_NZV;
	SET_NZ16(XREG);
	DIRECT;
	WRITEW(eaddr, XREG);
}

/* $109F STY direct -**0- */
static void FASTCALL sty_di( void )
{
	CLR_NZV;
	SET_NZ16(YREG);
	DIRECT;
	WRITEW(eaddr, YREG);
}

/* $a0 SUBA indexed ?**** */
static void FASTCALL suba_ix( void )
{
	WORD t, r;

	t = READB(eaddr);
	r = (WORD)(AREG - t);
	CLR_NZVC;
	SET_FLAGS8(AREG, t, r);
	AREG = (BYTE)r;
}

/* $a1 CMPA indexed ?**** */
static void FASTCALL cmpa_ix( void )
{
	WORD t, r;

	t = READB(eaddr);
	r = (WORD)(AREG - t);
	CLR_NZVC;
	SET_FLAGS8(AREG, t, r);
}

/* $a2 SBCA indexed ?**** */
static void FASTCALL sbca_ix( void )
{
	WORD t, r;

	t = READB(eaddr);
	r = (WORD)(AREG - t - (CCREG &0x01));
	CLR_NZVC;
	SET_FLAGS8(AREG, t, r);
	AREG = (BYTE)r;
}

/* $a3 SUBD (CMPD CMPU) indexed -**** */
static void FASTCALL subd_ix( void )
{
	DWORD t, r;

	t = READW(eaddr); t &= 0xffff;
	r = DREG - t;
	CLR_NZVC;
	SET_FLAGS16(DREG, t, r);
	DREG = (WORD)r;
}

/* $10a3 CMPD indexed -**** */
static void FASTCALL cmpd_ix( void )
{
	DWORD t, r;

	t = READW(eaddr); t &= 0xffff;
	r = DREG - t;
	CLR_NZVC;
	SET_FLAGS16(DREG, t, r);
}

/* $11a3 CMPU indexed -**** */
static void FASTCALL cmpu_ix( void )
{
	DWORD t, r;

	t = READW(eaddr); t &= 0xffff;
	r = UREG - t;
	CLR_NZVC;
	SET_FLAGS16(UREG, t, r);
}

/* $a4 ANDA indexed -**0- */
static void FASTCALL anda_ix( void )
{
	AREG &= READB(eaddr);
	CLR_NZV; SET_NZ8(AREG);
}

/* $a5 BITA indexed -**0- */
static void FASTCALL bita_ix( void )
{
	BYTE r;
	r = AREG&READB(eaddr);
	CLR_NZV; SET_NZ8(r);
}

/* $a6 LDA indexed -**0- */
static void FASTCALL lda_ix( void )
{
	AREG = READB(eaddr);
	CLR_NZV; SET_NZ8(AREG);
}

/* $a7 STA indexed -**0- */
static void FASTCALL sta_ix( void )
{
	CLR_NZV; SET_NZ8(AREG);
	WRITEB(eaddr,AREG);
}

/* $a8 EORA indexed -**0- */
static void FASTCALL eora_ix( void )
{
	AREG ^= READB(eaddr);
	CLR_NZV; SET_NZ8(AREG);
}

/* $a9 ADCA indexed ***** */
static void FASTCALL adca_ix( void )
{
	WORD t, r;

	t = READB(eaddr);
	r = (WORD)(AREG + t + (CCREG &0x01));
	CLR_HNZVC;
	SET_FLAGS8(AREG, t, r);
	SET_H(AREG, t, r);
	AREG = (BYTE)r;
}

/* $aA ORA indexed -**0- */
static void FASTCALL ora_ix( void )
{
	AREG |= READB(eaddr);
	CLR_NZV; SET_NZ8(AREG);
}

/* $aB ADDA indexed ***** */
static void FASTCALL adda_ix( void )
{
	WORD t, r;

	t = READB(eaddr);
	r = (WORD)(AREG + t);
	CLR_HNZVC;
	SET_FLAGS8(AREG, t, r);
	SET_H(AREG, t, r);
	AREG = (BYTE)r;
}

/* $aC CMPX (CMPY CMPS) indexed -**** */
static void FASTCALL cmpx_ix( void )
{
	DWORD t, r;

	t = READW(eaddr); t &= 0xffff;
	r = XREG - t;
	CLR_NZVC;
	SET_FLAGS16(XREG, t, r);
}

/* $10aC CMPY indexed -**** */
static void FASTCALL cmpy_ix( void )
{
	DWORD t, r;

	t = READW(eaddr); t &= 0xffff;
	r = YREG - t;
	CLR_NZVC;
	SET_FLAGS16(YREG, t, r);
}

/* $11aC CMPS indexed -**** */
static void FASTCALL cmps_ix( void )
{
	DWORD t, r;

	t = READW(eaddr); t &= 0xffff;
	r = SREG - t;
	CLR_NZVC;
	SET_FLAGS16(SREG, t, r);
}

/* $aD JSR indexed ----- */
static void FASTCALL jsr_ix( void )
{
	PUSHWORD(PCREG);
	PCREG = eaddr;
}

/* $aE LDX (LDY) indexed -**0- */
static void FASTCALL ldx_ix( void )
{
	XREG = READW(eaddr);
	CLR_NZV;
	SET_NZ16(XREG);
}

/* $10aE LDY indexed -**0- */
static void FASTCALL ldy_ix( void )
{
	YREG = READW(eaddr);
	CLR_NZV;
	SET_NZ16(YREG);
}

/* $aF STX (STY) indexed -**0- */
static void FASTCALL stx_ix( void )
{
	CLR_NZV;
	SET_NZ16(XREG);
	WRITEW(eaddr, XREG);
}

/* $10aF STY indexed -**0- */
static void FASTCALL sty_ix( void )
{
	CLR_NZV;
	SET_NZ16(YREG);
	WRITEW(eaddr, YREG);
}

/* $b0 SUBA extended ?**** */
static void FASTCALL suba_ex( void )
{
	WORD t, r;

	EXTBYTE(t);
	r = (WORD)(AREG - t);
	CLR_NZVC;
	SET_FLAGS8(AREG, t, r);
	AREG = (BYTE)r;
}

/* $b1 CMPA extended ?**** */
static void FASTCALL cmpa_ex( void )
{
	WORD t, r;

	EXTBYTE(t);
	r = (WORD)(AREG - t);
	CLR_NZVC;
	SET_FLAGS8(AREG, t, r);
}

/* $b2 SBCA extended ?**** */
static void FASTCALL sbca_ex( void )
{
	WORD t, r;

	EXTBYTE(t);
	r = (WORD)(AREG - t -(CCREG &0x01));
	CLR_NZVC;
	SET_FLAGS8(AREG, t, r);
	AREG = (BYTE)r;
}

/* $b3 SUBD (CMPD CMPU) extended -**** */
static void FASTCALL subd_ex( void )
{
	DWORD t, r;

	EXTWORD(t); t &= 0xffff;
	r = DREG - t;
	CLR_NZVC;
	SET_FLAGS16(DREG, t, r);
	DREG = (WORD)r;
}

/* $10b3 CMPD extended -**** */
static void FASTCALL cmpd_ex( void )
{
	DWORD t, r;

	EXTWORD(t); t &= 0xffff;
	r = DREG - t;
	CLR_NZVC;
	SET_FLAGS16(DREG, t, r);
}

/* $11b3 CMPU extended -**** */
static void FASTCALL cmpu_ex( void )
{
	DWORD r,b;
	EXTWORD(b); b &= 0xffff;
	r = UREG-b;
	CLR_NZVC; SET_FLAGS16(UREG,b,r);
}

/* $b4 ANDA extended -**0- */
static void FASTCALL anda_ex( void )
{
	BYTE t;
	EXTBYTE(t); AREG &= t;
	CLR_NZV; SET_NZ8(AREG);
}

/* $b5 BITA extended -**0- */
static void FASTCALL bita_ex( void )
{
	BYTE t,r;

	EXTBYTE(t);
	r = AREG & t;
	CLR_NZV;
	SET_NZ8(r);
}

/* $b6 LDA extended -**0- */
static void FASTCALL lda_ex( void )
{
	EXTBYTE(AREG);
	CLR_NZV;
	SET_NZ8(AREG);
}

/* $b7 STA extended -**0- */
static void FASTCALL sta_ex( void )
{
	CLR_NZV;
	SET_NZ8(AREG);
	EXTENDED;
	WRITEB(eaddr,AREG);
}

/* $b8 EORA extended -**0- */
static void FASTCALL eora_ex( void )
{
	BYTE t;

	EXTBYTE(t);
	AREG ^= t;
	CLR_NZV;
	SET_NZ8(AREG);
}

/* $b9 ADCA extended ***** */
static void FASTCALL adca_ex( void )
{
	WORD t, r;

	EXTBYTE(t);
	r = (WORD)(AREG + t + (CCREG &0x01));
	CLR_HNZVC;
	SET_FLAGS8(AREG, t ,r);
	SET_H(AREG, t, r);
	AREG = (BYTE)r;
}

/* $bA ORA extended -**0- */
static void FASTCALL ora_ex( void )
{
	BYTE t;
	EXTBYTE(t); AREG |= t;
	CLR_NZV; SET_NZ8(AREG);
}

/* $bB ADDA extended ***** */
static void FASTCALL adda_ex( void )
{
	WORD t, r;

	EXTBYTE(t);
	r = (WORD)(AREG + t);
	CLR_HNZVC;
	SET_FLAGS8(AREG, t, r);
	SET_H(AREG, t, r);
	AREG = (BYTE)r;
}

/* $bC CMPX (CMPY CMPS) extended -**** */
static void FASTCALL cmpx_ex( void )
{
	DWORD r,d,b;

	EXTWORD(b);
	b &= 0xffff;
	d = XREG;
	r = d - b;
	CLR_NZVC;
	SET_FLAGS16(d,b,r);
}

/* $10bC CMPY extended -**** */
static void FASTCALL cmpy_ex( void )
{
	DWORD r,d,b;

	EXTWORD(b);
	b &= 0xffff;
	d = YREG;
	r = d - b;
	CLR_NZVC;
	SET_FLAGS16(d,b,r);
}

/* $11bC CMPS extended -**** */
static void FASTCALL cmps_ex( void )
{
	DWORD r,d,b;

	EXTWORD(b);
	b &= 0xffff;
	d = SREG;
	r = d - b;
	CLR_NZVC;
	SET_FLAGS16(d,b,r);
}

/* $bD JSR extended ----- */
static void FASTCALL jsr_ex( void )
{
	EXTENDED;
	PUSHWORD(PCREG);
	PCREG = eaddr;
}

/* $bE LDX (LDY) extended -**0- */
static void FASTCALL ldx_ex( void )
{
	EXTWORD(XREG);
	CLR_NZV;
	SET_NZ16(XREG);
}

/* $10bE LDY extended -**0- */
static void FASTCALL ldy_ex( void )
{
	EXTWORD(YREG);
	CLR_NZV;
	SET_NZ16(YREG);
}

/* $bF STX (STY) extended -**0- */
static void FASTCALL stx_ex( void )
{
	CLR_NZV;
	SET_NZ16(XREG);
	EXTENDED;
	WRITEW(eaddr, XREG);
}

/* $10bF STY extended -**0- */
static void FASTCALL sty_ex( void )
{
	CLR_NZV;
	SET_NZ16(YREG);
	EXTENDED;
	WRITEW(eaddr,YREG);
}

/* $c0 SUBB immediate ?**** */
static void FASTCALL subb_im( void )
{
	WORD t,r;

	IMMBYTE(t);
	r = (WORD)(BREG - t);
	CLR_NZVC;
	SET_FLAGS8(BREG, t, r);
	BREG = (BYTE)r;
}

/* $c1 CMPB immediate ?**** */
static void FASTCALL cmpb_im( void )
{
	WORD t,r;

	IMMBYTE(t); 
	r = (WORD)(BREG - t);
	CLR_NZVC;
	SET_FLAGS8(BREG, t, r);
}

/* $c2 SBCB immediate ?**** */
static void FASTCALL sbcb_im( void )
{
	WORD t, r;

	IMMBYTE(t);
	r = (WORD)(BREG - t - (CCREG &0x01));
	CLR_NZVC;
	SET_FLAGS8(BREG, t, r);
	BREG = (BYTE)r;
}

/* $c3 ADDD immediate -**** */
static void FASTCALL addd_im( void )
{
	DWORD t, r;

	IMMWORD(t); t &= 0xffff;
	r = DREG + t;
	CLR_NZVC;
	SET_FLAGS16(DREG, t, r);
	DREG = (WORD)r;
}

/* $c4 ANDB immediate -**0- */
static void FASTCALL andb_im( void )
{
	BYTE t;

	IMMBYTE(t);
	BREG &= t;
	CLR_NZV;
	SET_NZ8(BREG);
}

/* $c5 BITB immediate -**0- */
static void FASTCALL bitb_im( void )
{
	BYTE t, r;

	IMMBYTE(t);
	r = BREG & t;
	CLR_NZV;
	SET_NZ8(r);
}

/* $c6 LDB immediate -**0- */
static void FASTCALL ldb_im( void )
{
	IMMBYTE(BREG);
	CLR_NZV;
	SET_NZ8(BREG);
}

/* $c8 EORB immediate -**0- */
static void FASTCALL eorb_im( void )
{
	BYTE t;

	IMMBYTE(t);
	BREG ^= t;
	CLR_NZV;
	SET_NZ8(BREG);
}

/* $c9 ADCB immediate ***** */
static void FASTCALL adcb_im( void )
{
	WORD t, r;

	IMMBYTE(t);
	r = (WORD)(BREG + t + (CCREG &0x01));
	CLR_HNZVC;
	SET_FLAGS8(BREG, t, r);
	SET_H(BREG, t, r);
	BREG = (BYTE)r;
}

/* $cA ORB immediate -**0- */
static void FASTCALL orb_im( void )
{
	BYTE t;

	IMMBYTE(t);
	BREG |= t;
	CLR_NZV;
	SET_NZ8(BREG);
}

/* $cB ADDB immediate ***** */
static void FASTCALL addb_im( void )
{
	WORD t, r;

	IMMBYTE(t);
	r = (WORD)(BREG + t);
	CLR_HNZVC;
	SET_FLAGS8(BREG, t, r);
	SET_H(BREG, t, r);
	BREG = (BYTE)r;
}

/* $cC LDD immediate -**0- */
static void FASTCALL ldd_im( void )
{
	WORD t;

	IMMWORD(t);
	DREG = t;
	CLR_NZV;
	SET_NZ16(t);
}

/* $cE LDU (LDS) immediate -**0- */
static void FASTCALL ldu_im( void )
{
	IMMWORD(UREG);
	CLR_NZV;
	SET_NZ16(UREG);
}

/* $10cE LDS immediate -**0- */
static void FASTCALL lds_im( void )
{
	IMMWORD(SREG);
	CLR_NZV;
	SET_NZ16(SREG);

	/* NMI許可 */
	INTR |= INTR_SLOAD;
}

/* is this a legal instruction? */
/* $10cF STS immediate -**0- */
static void FASTCALL sts_im( void )
{
}

/* $d0 SUBB direct ?**** */
static void FASTCALL subb_di( void )
{
	WORD t, r;

	DIRBYTE(t);
	r = (WORD)(BREG - t);
	CLR_NZVC;
	SET_FLAGS8(BREG, t, r);
	BREG = (BYTE)r;
}

/* $d1 CMPB direct ?**** */
static void FASTCALL cmpb_di( void )
{
	WORD t,r;
	DIRBYTE(t); r = (WORD)(BREG-t);
	CLR_NZVC; SET_FLAGS8(BREG,t,r);
}

/* $d2 SBCB direct ?**** */
static void FASTCALL sbcb_di( void )
{
	WORD t, r;

	DIRBYTE(t);
	r = (WORD)(BREG - t - (CCREG &0x01));
	CLR_NZVC;
	SET_FLAGS8(BREG, t, r);
	BREG = (BYTE)r;
}

/* $d3 ADDD direct -**** */
static void FASTCALL addd_di( void )
{
	DWORD t, r;

	DIRWORD(t); t &= 0xffff;
	r = DREG + t;
	CLR_NZVC;
	SET_FLAGS16(DREG, t, r);
	DREG = (WORD)r;
}

/* $d4 ANDB direct -**0- */
static void FASTCALL andb_di( void )
{
	BYTE t;
	DIRBYTE(t); BREG &= t;
	CLR_NZV; SET_NZ8(BREG);
}

/* $d5 BITB direct -**0- */
static void FASTCALL bitb_di( void )
{
	BYTE t,r;
	DIRBYTE(t); r = BREG&t;
	CLR_NZV; SET_NZ8(r);
}

/* $d6 LDB direct -**0- */
static void FASTCALL ldb_di( void )
{
	DIRBYTE(BREG);
	CLR_NZV; SET_NZ8(BREG);
}

/* $d7 STB direct -**0- */
static void FASTCALL stb_di( void )
{
	CLR_NZV; SET_NZ8(BREG);
	DIRECT; WRITEB(eaddr,BREG);
}

/* $d8 EORB direct -**0- */
static void FASTCALL eorb_di( void )
{
	BYTE t;
	DIRBYTE(t); BREG ^= t;
	CLR_NZV; SET_NZ8(BREG);
}

/* $d9 ADCB direct ***** */
static void FASTCALL adcb_di( void )
{
	WORD t, r;

	DIRBYTE(t);
	r = (WORD)(BREG + t + (CCREG &0x01));
	CLR_HNZVC;
	SET_FLAGS8(BREG, t, r);
	SET_H(BREG, t, r);
	BREG = (BYTE)r;
}

/* $dA ORB direct -**0- */
static void FASTCALL orb_di( void )
{
	BYTE t;
	DIRBYTE(t); BREG |= t;
	CLR_NZV; SET_NZ8(BREG);
}

/* $dB ADDB direct ***** */
static void FASTCALL addb_di( void )
{
	WORD t, r;

	DIRBYTE(t);
	r = (WORD)(BREG + t);
	CLR_HNZVC;
	SET_FLAGS8(BREG, t, r);
	SET_H(BREG, t, r);
	BREG = (BYTE)r;
}

/* $dC LDD direct -**0- */
static void FASTCALL ldd_di( void )
{
	WORD t;
	DIRWORD(t); DREG = t;
	CLR_NZV; SET_NZ16(t);
}

/* $dD STD direct -**0- */
static void FASTCALL std_di( void )
{
	DIRECT;
	CLR_NZV;
	SET_NZ16(DREG);
	WRITEW(eaddr, DREG);
}

/* $dE LDU (LDS) direct -**0- */
static void FASTCALL ldu_di( void )
{
	DIRWORD(UREG);
	CLR_NZV; SET_NZ16(UREG);
}

/* $10dE LDS direct -**0- */
static void FASTCALL lds_di( void )
{
	DIRWORD(SREG);
	CLR_NZV; SET_NZ16(SREG);

	/* NMI許可 */
	INTR |= INTR_SLOAD;
}

/* $dF STU (STS) direct -**0- */
static void FASTCALL stu_di( void )
{
	CLR_NZV;
	SET_NZ16(UREG);
	DIRECT;
	WRITEW(eaddr, UREG);
}

/* $10dF STS direct -**0- */
static void FASTCALL sts_di( void )
{
	CLR_NZV;
	SET_NZ16(SREG);
	DIRECT;
	WRITEW(eaddr, SREG);
}

/* $e0 SUBB indexed ?**** */
static void FASTCALL subb_ix( void )
{
	WORD t, r;

	t = READB(eaddr);
	r = (WORD)(BREG - t);
	CLR_NZVC;
	SET_FLAGS8(BREG, t, r);
	BREG = (BYTE)r;
}

/* $e1 CMPB indexed ?**** */
static void FASTCALL cmpb_ix( void )
{
	WORD t, r;

	t = READB(eaddr);
	r = (WORD)(BREG - t);
	CLR_NZVC;
	SET_FLAGS8(BREG, t, r);
}

/* $e2 SBCB indexed ?**** */
static void FASTCALL sbcb_ix( void )
{
	WORD t, r;

	t = READB(eaddr);
	r = (WORD)(BREG - t - (CCREG &0x01));
	CLR_NZVC;
	SET_FLAGS8(BREG, t, r);
	BREG = (BYTE)r;
}

/* $e3 ADDD indexed -**** */
static void FASTCALL addd_ix( void )
{
	DWORD t, r;

	t = READW(eaddr); t &= 0xffff;
	r = DREG + t;
	CLR_NZVC;
	SET_FLAGS16(DREG, t, r);
	DREG = (WORD)r;
}

/* $e4 ANDB indexed -**0- */
static void FASTCALL andb_ix( void )
{
	BREG &= READB(eaddr);
	CLR_NZV; SET_NZ8(BREG);
}

/* $e5 BITB indexed -**0- */
static void FASTCALL bitb_ix( void )
{
	BYTE r;
	r = BREG&READB(eaddr);
	CLR_NZV; SET_NZ8(r);
}

/* $e6 LDB indexed -**0- */
static void FASTCALL ldb_ix( void )
{
	BREG = READB(eaddr);
	CLR_NZV; SET_NZ8(BREG);
}

/* $e7 STB indexed -**0- */
static void FASTCALL stb_ix( void )
{
	CLR_NZV; SET_NZ8(BREG);
	WRITEB(eaddr,BREG);
}

/* $e8 EORB indexed -**0- */
static void FASTCALL eorb_ix( void )
{
	BREG ^= READB(eaddr);
	CLR_NZV; SET_NZ8(BREG);
}

/* $e9 ADCB indexed ***** */
static void FASTCALL adcb_ix( void )
{
	WORD t, r;

	t = READB(eaddr);
	r = (WORD)(BREG + t + (CCREG &0x01));
	CLR_HNZVC;
	SET_FLAGS8(BREG, t, r);
	SET_H(BREG, t, r);
	BREG = (BYTE)r;
}

/* $eA ORB indexed -**0- */
static void FASTCALL orb_ix( void )
{
	BREG |= READB(eaddr);
	CLR_NZV; SET_NZ8(BREG);
}

/* $eB ADDB indexed ***** */
static void FASTCALL addb_ix( void )
{
	WORD t, r;

	t = READB(eaddr);
	r = (WORD)(BREG + t);
	CLR_HNZVC;
	SET_FLAGS8(BREG, t, r);
	SET_H(BREG, t, r);
	BREG = (BYTE)r;
}

/* $eC LDD indexed -**0- */
static void FASTCALL ldd_ix( void )
{
	WORD t;
	t = READW(eaddr);
	CLR_NZV;
	SET_NZ16(t);
	DREG = t;
}

/* $eD STD indexed -**0- */
static void FASTCALL std_ix( void )
{
	CLR_NZV;
	SET_NZ16(DREG);
	WRITEW(eaddr, DREG);
}

/* $eE LDU (LDS) indexed -**0- */
static void FASTCALL ldu_ix( void )
{
	UREG = READW(eaddr);
	CLR_NZV;
	SET_NZ16(UREG);
}

/* $10eE LDS indexed -**0- */
static void FASTCALL lds_ix( void )
{
	SREG = READW(eaddr);
	CLR_NZV;
	SET_NZ16(SREG);

	/* NMI許可 */
	INTR |= INTR_SLOAD;
}

/* $eF STU (STS) indexed -**0- */
static void FASTCALL stu_ix( void )
{
	CLR_NZV;
	SET_NZ16(UREG);
	WRITEW(eaddr, UREG);
}

/* $10eF STS indexed -**0- */
static void FASTCALL sts_ix( void )
{
	CLR_NZV;
	SET_NZ16(SREG);
	WRITEW(eaddr, SREG);
}

/* $f0 SUBB extended ?**** */
static void FASTCALL subb_ex( void )
{
	WORD t, r;

	EXTBYTE(t);
	r = (WORD)(BREG - t);
	CLR_NZVC;
	SET_FLAGS8(BREG, t, r);
	BREG = (BYTE)r;
}

/* $f1 CMPB extended ?**** */
static void FASTCALL cmpb_ex( void )
{
	WORD t, r;

	EXTBYTE(t);
	r = (WORD)(BREG - t);
	CLR_NZVC;
	SET_FLAGS8(BREG, t, r);
}

/* $f2 SBCB extended ?**** */
static void FASTCALL sbcb_ex( void )
{
	WORD t, r;

	EXTBYTE(t);
	r = (WORD)(BREG - t - (CCREG &0x01));
	CLR_NZVC;
	SET_FLAGS8(BREG, t, r);
	BREG = (BYTE)r;
}

/* $f3 ADDD extended -**** */
static void FASTCALL addd_ex( void )
{
	DWORD t, r;

	EXTWORD(t); t &= 0xffff;
	r = DREG + t;
	CLR_NZVC;
	SET_FLAGS16(DREG ,t ,r);
	DREG = (WORD)r;
}

/* $f4 ANDB extended -**0- */
static void FASTCALL andb_ex( void )
{
	BYTE t;
	EXTBYTE(t); BREG &= t;
	CLR_NZV; SET_NZ8(BREG);
}

/* $f5 BITB extended -**0- */
static void FASTCALL bitb_ex( void )
{
	BYTE t,r;

	EXTBYTE(t);
	r = BREG & t;
	CLR_NZV;
	SET_NZ8(r);
}

/* $f6 LDB extended -**0- */
static void FASTCALL ldb_ex( void )
{
	EXTBYTE(BREG);
	CLR_NZV;
	SET_NZ8(BREG);
}

/* $f7 STB extended -**0- */
static void FASTCALL stb_ex( void )
{
	CLR_NZV;
	SET_NZ8(BREG);
	EXTENDED;
	WRITEB(eaddr,BREG);
}

/* $f8 EORB extended -**0- */
static void FASTCALL eorb_ex( void )
{
	BYTE t;
	EXTBYTE(t); BREG ^= t;
	CLR_NZV; SET_NZ8(BREG);
}

/* $f9 ADCB extended ***** */
static void FASTCALL adcb_ex( void )
{
	WORD t, r;

	EXTBYTE(t);
	r = (WORD)(BREG + t + (CCREG &0x01));
	CLR_HNZVC;
	SET_FLAGS8(BREG, t, r);
	SET_H(BREG, t, r);
	BREG = (BYTE)r;
}

/* $fA ORB extended -**0- */
static void FASTCALL orb_ex( void )
{
	BYTE t;
	EXTBYTE(t); BREG |= t;
	CLR_NZV; SET_NZ8(BREG);
}

/* $fB ADDB extended ***** */
static void FASTCALL addb_ex( void )
{
	WORD t, r;

	EXTBYTE(t);
	r = (WORD)(BREG + t);
	CLR_HNZVC;
	SET_FLAGS8(BREG, t, r);
	SET_H(BREG, t, r);
	BREG = (BYTE)r;
}

/* $fC LDD extended -**0- */
static void FASTCALL ldd_ex( void )
{
	WORD t;

	EXTWORD(t);
	DREG = t;
	CLR_NZV;
	SET_NZ16(t);
}

/* $fD STD extended -**0- */
static void FASTCALL std_ex( void )
{
	EXTENDED;
	CLR_NZV;
	SET_NZ16(DREG);
	WRITEW(eaddr, DREG);
}

/* $fE LDU (LDS) extended -**0- */
static void FASTCALL ldu_ex( void )
{
	EXTWORD(UREG);
	CLR_NZV;
	SET_NZ16(UREG);
}

/* $10fE LDS extended -**0- */
static void FASTCALL lds_ex( void )
{
	EXTWORD(SREG);
	CLR_NZV;
	SET_NZ16(SREG);

	/* NMI許可 */
	INTR |= INTR_SLOAD;
}

/* $fF STU (STS) extended -**0- */
static void FASTCALL stu_ex( void )
{
	CLR_NZV;
	SET_NZ16(UREG);
	EXTENDED;
	WRITEW(eaddr, UREG);
}

/* $10fF STS extended -**0- */
static void FASTCALL sts_ex( void )
{
	CLR_NZV;
	SET_NZ16(SREG);
	EXTENDED;
	WRITEW(eaddr, SREG);
}

/*-[ ディスパッチ部 ]-------------------------------------------------------*/

/*
 *	実効アドレス取得(5bit)
 */
static void FASTCALL cpu_ea_5bit(BYTE postb)
{
	WORD srcreg;

	if (postb < 0x20) {
		srcreg = XREG;
	}
	else if (postb < 0x40) {
		srcreg = YREG;
	}
	else if (postb < 0x60) {
		srcreg = UREG;
	}
	else {
		srcreg = SREG;
	}

	postb &= 0x1f;
	if (postb <= 0x0f) {
		eaddr = (WORD)(srcreg + (postb & 0x0f));
	}
	else {
		eaddr = (WORD)(srcreg - (16 - (postb & 0x0f)));
	}
}

/*
 *	実効アドレス取得（インデックスアドレッシング）
 */
static void FASTCALL cpu_calc_ea(void)
{
	BYTE postb;
	WORD *srcreg;

	/* ポストバイト取得、サイクルカウンタ初期化 */
	postb = READB(PCREG++);

	/* 分岐 */
	if (postb < 0x80) {
		cpu_ea_5bit(postb);
		CYCLE++;
		return;
	}
	else if (postb < 0xa0) {
		srcreg = &XREG;
	}
	else if (postb < 0xc0) {
		srcreg = &YREG;
	}
	else if (postb < 0xe0) {
		srcreg = &UREG;
	}
	else {
		srcreg = &SREG;
	}

	switch (postb & 0x0f) {
		case 0x00:	// ,R+
					eaddr = *srcreg;
					*srcreg += (WORD)1;
					CYCLE += (WORD)2;
					break;
		case 0x01:	// ,R++
					eaddr = *srcreg;
					*srcreg += (WORD)2;
					CYCLE+=(WORD)3;
					break;
		case 0x02:	// ,-R
					*srcreg -= (WORD)1;
					eaddr = *srcreg;
					CYCLE += (WORD)2;
					break;
		case 0x03:	// ,R--
					*srcreg -= (WORD)2;
					eaddr = *srcreg;
					CYCLE += (WORD)3;
					break;
		case 0x04:	// ,R
					eaddr = *srcreg;
					break;
		case 0x05:	// B,R
					eaddr = (WORD)(*srcreg + SIGNED(BREG));
					CYCLE++;
					break;
		case 0x06:	// A,R
					eaddr = (WORD)(*srcreg + SIGNED(AREG));
					CYCLE++;
					break;
		case 0x07:	// A,R (ILLEGAL)
					eaddr = (WORD)(*srcreg + SIGNED(AREG));
					CYCLE++;
					break;
		case 0x08:	// 8bit, R
					IMMBYTE(eaddr);
					eaddr = (WORD)(*srcreg + SIGNED(eaddr));
					CYCLE++;
					break;
		case 0x09:	// 16bit, R
					IMMWORD(eaddr);
					eaddr += (WORD)*srcreg;
					CYCLE += (WORD)4;
					break;
		case 0x0A:	// ILLEGAL
					eaddr = (PCREG + 1) | 0xff;
					CYCLE++;
					break;
		case 0x0B:	// D,R
					eaddr = (WORD)(*srcreg + DREG);
					CYCLE += (WORD)4;
					break;
		case 0x0C:	// 8bit, PCR
					IMMBYTE(eaddr);
					eaddr = (WORD)(PCREG + SIGNED(eaddr));
					CYCLE++;
					break;
		case 0x0D:	// 16bit, PCR
					IMMWORD(eaddr);
					eaddr += PCREG;
					CYCLE += (WORD)5;
					break;
		case 0x0E:	// ILLEGAL
					eaddr = 0xffff;
					break;
		case 0x0F:	// EXTEND
					IMMWORD(eaddr);
					CYCLE += (WORD)5;
					break;
	}

	/* インダイレクトモード */
	if (postb & 0x10) {
		eaddr = READW(eaddr);
		CYCLE += (WORD)3;
	}
}

/*
 *	未定義命令
 */
static void FASTCALL illegal(void)
{
}

/*
 *	CPU１命令実行
 */
static void FASTCALL cpu_execline(cpu6809_t *CPU)
{
	BYTE opc;

	if (CPU) {
		currentcpu = CPU;
	}

	/* オペコード取得 */
	opc = fetch_op = READB(PCREG++);

	/* １バイト命令かチェック、EA計算 */
	CYCLE = cycles1[opc];
	if (CYCLE != 0x00ff) {
		if (CYCLE & 0x0080) {
			CYCLE &= 0x007f;
			cpu_calc_ea();
		}
		switch (opc) {
			case 0x00: neg_di(); break;
			case 0x01: neg_di(); break;
			case 0x02: ngc_di(); break;
			case 0x03: com_di(); break;
			case 0x04: lsr_di(); break;
			case 0x05: lsr_di(); break;
			case 0x06: ror_di(); break;
			case 0x07: asr_di(); break;
			case 0x08: asl_di(); break;
			case 0x09: rol_di(); break;
			case 0x0a: dec_di(); break;
			case 0x0b: dcc_di(); break;
			case 0x0c: inc_di(); break;
			case 0x0d: tst_di(); break;
			case 0x0e: jmp_di(); break;
			case 0x0f: clr_di(); break;
			case 0x10: illegal(); break;
			case 0x11: illegal(); break;
			case 0x12: nop(); break;
			case 0x13: sync(); break;
			case 0x14: illegal(); break;
			case 0x15: illegal(); break;
			case 0x16: lbra(); break;
			case 0x17: lbsr(); break;
			case 0x18: aslcc(); break;
			case 0x19: daa(); break;
			case 0x1a: orcc(); break;
			case 0x1b: nop(); break;
			case 0x1c: andcc(); break;
			case 0x1d: sex(); break;
			case 0x1e: exg(); break;
			case 0x1f: tfr(); break;
			case 0x20: bra(); break;
			case 0x21: brn(); break;
			case 0x22: bhi(); break;
			case 0x23: bls(); break;
			case 0x24: bcc(); break;
			case 0x25: bcs(); break;
			case 0x26: bne(); break;
			case 0x27: beq(); break;
			case 0x28: bvc(); break;
			case 0x29: bvs(); break;
			case 0x2a: bpl(); break;
			case 0x2b: bmi(); break;
			case 0x2c: bge(); break;
			case 0x2d: blt(); break;
			case 0x2e: bgt(); break;
			case 0x2f: ble(); break;
			case 0x30: leax(); break;
			case 0x31: leay(); break;
			case 0x32: leas(); break;
			case 0x33: leau(); break;
			case 0x34: pshs(); break;
			case 0x35: puls(); break;
			case 0x36: pshu(); break;
			case 0x37: pulu(); break;
			case 0x38: andcc(); break;
			case 0x39: rts(); break;
			case 0x3a: abx(); break;
			case 0x3b: rti(); break;
			case 0x3c: cwai(); break;
			case 0x3d: mul(); break;
			case 0x3e: rst(); break;
			case 0x3f: swi(); break;
			case 0x40: nega(); break;
			case 0x41: nega(); break;
			case 0x42: ngca(); break;
			case 0x43: coma(); break;
			case 0x44: lsra(); break;
			case 0x45: lsra(); break;
			case 0x46: rora(); break;
			case 0x47: asra(); break;
			case 0x48: asla(); break;
			case 0x49: rola(); break;
			case 0x4a: deca(); break;
			case 0x4b: dcca(); break;
			case 0x4c: inca(); break;
			case 0x4d: tsta(); break;
			case 0x4e: clca(); break;
			case 0x4f: clra(); break;
			case 0x50: negb(); break;
			case 0x51: negb(); break;
			case 0x52: ngcb(); break;
			case 0x53: comb(); break;
			case 0x54: lsrb(); break;
			case 0x55: lsrb(); break;
			case 0x56: rorb(); break;
			case 0x57: asrb(); break;
			case 0x58: aslb(); break;
			case 0x59: rolb(); break;
			case 0x5a: decb(); break;
			case 0x5b: dccb(); break;
			case 0x5c: incb(); break;
			case 0x5d: tstb(); break;
			case 0x5e: clcb(); break;
			case 0x5f: clrb(); break;
			case 0x60: neg_ix(); break;
			case 0x61: neg_ix(); break;
			case 0x62: ngc_ix(); break;
			case 0x63: com_ix(); break;
			case 0x64: lsr_ix(); break;
			case 0x65: lsr_ix(); break;
			case 0x66: ror_ix(); break;
			case 0x67: asr_ix(); break;
			case 0x68: asl_ix(); break;
			case 0x69: rol_ix(); break;
			case 0x6a: dec_ix(); break;
			case 0x6b: dcc_ix(); break;
			case 0x6c: inc_ix(); break;
			case 0x6d: tst_ix(); break;
			case 0x6e: jmp_ix(); break;
			case 0x6f: clr_ix(); break;
			case 0x70: neg_ex(); break;
			case 0x71: neg_ex(); break;
			case 0x72: ngc_ex(); break;
			case 0x73: com_ex(); break;
			case 0x74: lsr_ex(); break;
			case 0x75: lsr_ex(); break;
			case 0x76: ror_ex(); break;
			case 0x77: asr_ex(); break;
			case 0x78: asl_ex(); break;
			case 0x79: rol_ex(); break;
			case 0x7a: dec_ex(); break;
			case 0x7b: dcc_ex(); break;
			case 0x7c: inc_ex(); break;
			case 0x7d: tst_ex(); break;
			case 0x7e: jmp_ex(); break;
			case 0x7f: clr_ex(); break;
			case 0x80: suba_im(); break;
			case 0x81: cmpa_im(); break;
			case 0x82: sbca_im(); break;
			case 0x83: subd_im(); break;
			case 0x84: anda_im(); break;
			case 0x85: bita_im(); break;
			case 0x86: lda_im(); break;
			case 0x87: flag8_im(); break;
			case 0x88: eora_im(); break;
			case 0x89: adca_im(); break;
			case 0x8a: ora_im(); break;
			case 0x8b: adda_im(); break;
			case 0x8c: cmpx_im(); break;
			case 0x8d: bsr(); break;
			case 0x8e: ldx_im(); break;
			case 0x8f: flag16_im(); break;
			case 0x90: suba_di(); break;
			case 0x91: cmpa_di(); break;
			case 0x92: sbca_di(); break;
			case 0x93: subd_di(); break;
			case 0x94: anda_di(); break;
			case 0x95: bita_di(); break;
			case 0x96: lda_di(); break;
			case 0x97: sta_di(); break;
			case 0x98: eora_di(); break;
			case 0x99: adca_di(); break;
			case 0x9a: ora_di(); break;
			case 0x9b: adda_di(); break;
			case 0x9c: cmpx_di(); break;
			case 0x9d: jsr_di(); break;
			case 0x9e: ldx_di(); break;
			case 0x9f: stx_di(); break;
			case 0xa0: suba_ix(); break;
			case 0xa1: cmpa_ix(); break;
			case 0xa2: sbca_ix(); break;
			case 0xa3: subd_ix(); break;
			case 0xa4: anda_ix(); break;
			case 0xa5: bita_ix(); break;
			case 0xa6: lda_ix(); break;
			case 0xa7: sta_ix(); break;
			case 0xa8: eora_ix(); break;
			case 0xa9: adca_ix(); break;
			case 0xaa: ora_ix(); break;
			case 0xab: adda_ix(); break;
			case 0xac: cmpx_ix(); break;
			case 0xad: jsr_ix(); break;
			case 0xae: ldx_ix(); break;
			case 0xaf: stx_ix(); break;
			case 0xb0: suba_ex(); break;
			case 0xb1: cmpa_ex(); break;
			case 0xb2: sbca_ex(); break;
			case 0xb3: subd_ex(); break;
			case 0xb4: anda_ex(); break;
			case 0xb5: bita_ex(); break;
			case 0xb6: lda_ex(); break;
			case 0xb7: sta_ex(); break;
			case 0xb8: eora_ex(); break;
			case 0xb9: adca_ex(); break;
			case 0xba: ora_ex(); break;
			case 0xbb: adda_ex(); break;
			case 0xbc: cmpx_ex(); break;
			case 0xbd: jsr_ex(); break;
			case 0xbe: ldx_ex(); break;
			case 0xbf: stx_ex(); break;
			case 0xc0: subb_im(); break;
			case 0xc1: cmpb_im(); break;
			case 0xc2: sbcb_im(); break;
			case 0xc3: addd_im(); break;
			case 0xc4: andb_im(); break;
			case 0xc5: bitb_im(); break;
			case 0xc6: ldb_im(); break;
			case 0xc7: flag8_im(); break;
			case 0xc8: eorb_im(); break;
			case 0xc9: adcb_im(); break;
			case 0xca: orb_im(); break;
			case 0xcb: addb_im(); break;
			case 0xcc: ldd_im(); break;
			case 0xcd: illegal(); break;
			case 0xce: ldu_im(); break;
			case 0xcf: flag16_im(); break;
			case 0xd0: subb_di(); break;
			case 0xd1: cmpb_di(); break;
			case 0xd2: sbcb_di(); break;
			case 0xd3: addd_di(); break;
			case 0xd4: andb_di(); break;
			case 0xd5: bitb_di(); break;
			case 0xd6: ldb_di(); break;
			case 0xd7: stb_di(); break;
			case 0xd8: eorb_di(); break;
			case 0xd9: adcb_di(); break;
			case 0xda: orb_di(); break;
			case 0xdb: addb_di(); break;
			case 0xdc: ldd_di(); break;
			case 0xdd: std_di(); break;
			case 0xde: ldu_di(); break;
			case 0xdf: stu_di(); break;
			case 0xe0: subb_ix(); break;
			case 0xe1: cmpb_ix(); break;
			case 0xe2: sbcb_ix(); break;
			case 0xe3: addd_ix(); break;
			case 0xe4: andb_ix(); break;
			case 0xe5: bitb_ix(); break;
			case 0xe6: ldb_ix(); break;
			case 0xe7: stb_ix(); break;
			case 0xe8: eorb_ix(); break;
			case 0xe9: adcb_ix(); break;
			case 0xea: orb_ix(); break;
			case 0xeb: addb_ix(); break;
			case 0xec: ldd_ix(); break;
			case 0xed: std_ix(); break;
			case 0xee: ldu_ix(); break;
			case 0xef: stu_ix(); break;
			case 0xf0: subb_ex(); break;
			case 0xf1: cmpb_ex(); break;
			case 0xf2: sbcb_ex(); break;
			case 0xf3: addd_ex(); break;
			case 0xf4: andb_ex(); break;
			case 0xf5: bitb_ex(); break;
			case 0xf6: ldb_ex(); break;
			case 0xf7: stb_ex(); break;
			case 0xf8: eorb_ex(); break;
			case 0xf9: adcb_ex(); break;
			case 0xfa: orb_ex(); break;
			case 0xfb: addb_ex(); break;
			case 0xfc: ldd_ex(); break;
			case 0xfd: std_ex(); break;
			case 0xfe: ldu_ex(); break;
			case 0xff: stu_ex(); break;
		}
	}
	else {
		/* 0x10台かチェック、EA計算 */
		if (opc == 0x10) {
			opc = READB(PCREG++);
			CYCLE = cycles2[opc];
			if (CYCLE & 0x0080) {
				CYCLE &= 0x007f;
				cpu_calc_ea();
			}
			switch(opc) {
				case 0x20: lbra(); break;
				case 0x21: lbrn(); break;
				case 0x22: lbhi(); break;
				case 0x23: lbls(); break;
				case 0x24: lbcc(); break;
				case 0x25: lbcs(); break;
				case 0x26: lbne(); break;
				case 0x27: lbeq(); break;
				case 0x28: lbvc(); break;
				case 0x29: lbvs(); break;
				case 0x2a: lbpl(); break;
				case 0x2b: lbmi(); break;
				case 0x2c: lbge(); break;
				case 0x2d: lblt(); break;
				case 0x2e: lbgt(); break;
				case 0x2f: lble(); break;
				case 0x3f: swi2(); break;
				case 0x83: cmpd_im(); break;
				case 0x8c: cmpy_im(); break;
				case 0x8d: lbsr(); break;
				case 0x8e: ldy_im(); break;
				case 0x8f: sty_im(); break; /* ILLEGAL? */
				case 0x93: cmpd_di(); break;
				case 0x9c: cmpy_di(); break;
				case 0x9e: ldy_di(); break;
				case 0x9f: sty_di(); break;
				case 0xa3: cmpd_ix(); break;
				case 0xac: cmpy_ix(); break;
				case 0xae: ldy_ix(); break;
				case 0xaf: sty_ix(); break;
				case 0xb3: cmpd_ex(); break;
				case 0xbc: cmpy_ex(); break;
				case 0xbe: ldy_ex(); break;
				case 0xbf: sty_ex(); break;
				case 0xce: lds_im(); break;
				case 0xcf: sts_im(); break; /* ILLEGAL? */
				case 0xde: lds_di(); break;
				case 0xdf: sts_di(); break;
				case 0xee: lds_ix(); break;
				case 0xef: sts_ix(); break;
				case 0xfe: lds_ex(); break;
				case 0xff: sts_ex(); break;
				default: PCREG--; illegal(); break;
				}
			}
		else {
			opc = READB(PCREG++);
			CYCLE = cycles2[opc];
			if (CYCLE & 0x0080) {
				CYCLE &= 0x007f;
				cpu_calc_ea();
			}
			switch (opc) {
				case 0x3f: swi3(); break;
				case 0x83: cmpu_im(); break;
				case 0x8c: cmps_im(); break;
				case 0x93: cmpu_di(); break;
				case 0x9c: cmps_di(); break;
				case 0xa3: cmpu_ix(); break;
				case 0xac: cmps_ix(); break;
				case 0xb3: cmpu_ex(); break;
				case 0xbc: cmps_ex(); break;
				default: PCREG--; illegal(); break;
			}
		}
	}
}

/*-[ コントロール部 ]-------------------------------------------------------*/

/*
 *	NMI割り込みを処理
 */
static void FASTCALL cpu_nmi(void)
{
	/* CWAI対応 */
	INTR |= INTR_CWAI_OUT;

	/* Eフラグをセット、レジスタ退避 */
	CCREG |= 0x80;

	PUSHWORD(PCREG);
	PUSHWORD(UREG);
	PUSHWORD(YREG);
	PUSHWORD(XREG);
	PUSHBYTE(DPREG);
	PUSHBYTE(BREG);
	PUSHBYTE(AREG);
	PUSHBYTE(CCREG);

	/* NMI実行 */
	CCREG |= 0x50;
	PCREG = READW(0xfffc);

	/* フラグを降ろす */
	INTR &= ~INTR_NMI;
}

/*
 *	FIRQ割り込みを処理
 */
static void FASTCALL cpu_firq(void)
{
	if (INTR & INTR_CWAI_IN) {
		/* CWAI実行中。E=1で行く */
		CCREG |= 0x80;

		/* レジスタ退避 */
		PUSHWORD(PCREG);
		PUSHWORD(UREG);
		PUSHWORD(YREG);
		PUSHWORD(XREG);
		PUSHBYTE(DPREG);
		PUSHBYTE(BREG);
		PUSHBYTE(AREG);
		PUSHBYTE(CCREG);
	}
	else {
		/* Eフラグ降ろす */
		CCREG &= 0x7f;

		/* レジスタ退避 */
		PUSHWORD(PCREG);
		PUSHBYTE(CCREG);
	}

	/* FIRQ実行 */
	CCREG |= 0x50;
	PCREG = READW(0xfff6);

	/* フラグを降ろす */
	/*INTR &= ~INTR_FIRQ;*/
}

/*
 *	IRQ割り込みを処理
 */
static void FASTCALL cpu_irq(void)
{
	/* CWAI対応 */
	INTR |= INTR_CWAI_OUT;

	/* Eフラグセット */
	CCREG |= 0x80;

	/* レジスタ退避 */
	PUSHWORD(PCREG);
	PUSHWORD(UREG);
	PUSHWORD(YREG);
	PUSHWORD(XREG);
	PUSHBYTE(DPREG);
	PUSHBYTE(BREG);
	PUSHBYTE(AREG);
	PUSHBYTE(CCREG);

	/* IRQ実行 */
	CCREG |= 0x10;
	PCREG = READW(0xfff8);

	/* フラグを降ろす */
	/*INTR &= ~INTR_IRQ;*/
}

/*
 *	CPUリセット
 */
static void FASTCALL cpu_reset(cpu6809_t *CPU)
{
	if (CPU) {
		currentcpu = CPU;
	}

	CCREG = 0x50;
	AREG = 0;
	BREG = 0;
	DPREG = 0;
	XREG = 0;
	YREG = 0;
	UREG = 0;
	SREG = 0;
	INTR = 0;
	CYCLE = 0;
	TOTAL = 0;

	PCREG = (WORD)((READB(0xfffe) << 8) + READB(0xffff));
}

/*
 *	CPU実行
 */
static void FASTCALL cpu_exec(cpu6809_t *CPU)
{
	if (CPU) {
		currentcpu = CPU;
	}

	/* NMI割り込みチェック */
	if (INTR & INTR_NMI) {
		INTR |= INTR_SYNC_OUT;
		if (INTR & INTR_SLOAD) {
			cpu_nmi();
			cpu_execline(CPU);
			TOTAL += CYCLE;
			return;
		}
	}

	/* FIRQ割り込みチェック */
	if (INTR & INTR_FIRQ) {
		INTR |= INTR_SYNC_OUT;
		if (!(CCREG & 0x40)) {
			cpu_firq();
			cpu_execline(CPU);
			TOTAL += CYCLE;
			return;
		}
	}

	/* IRQ割り込みチェック */
	if (INTR & INTR_IRQ) {
		INTR |= INTR_SYNC_OUT;
		if (!(CCREG & 0x10)) {
			cpu_irq();
			cpu_execline(CPU);
			TOTAL += CYCLE;
			return;
		}
	}

	/* 割り込みなし */
	cpu_execline(CPU);
	TOTAL += CYCLE;
	return;
}

/*
 *	cpu_x86.asm互換I/F
 *	メインCPUリセット
 */
void main_reset(void)
{
	cpu_reset(&maincpu);
}

/*
 *	cpu_x86.asm互換I/F
 *	メインCPU実行
 */
void main_exec(void)
{
	cpu_exec(&maincpu);
}

/*
 *	cpu_x86.asm互換I/F
 *	メインCPU 1命令実行
 */
void main_line(void)
{
	cpu_execline(&maincpu);
}

/*
 *	cpu_x86.asm互換I/F
 *	サブCPUリセット
 */
void sub_reset(void)
{
	cpu_reset(&subcpu);
}

/*
 *	cpu_x86.asm互換I/F
 *	サブCPU実行
 */
void sub_exec(void)
{
	cpu_exec(&subcpu);
}

/*
 *	cpu_x86.asm互換I/F
 *	サブCPU 1命令実行
 */
void sub_line(void)
{
	cpu_execline(&subcpu);
}

#if XM7_VER == 1
#if defined(JSUB)
/*
 *	cpu_x86.asm互換I/F
 *	サブCPUリセット
 */
void jsub_reset(void)
{
	cpu_reset(&jsubcpu);
}

/*
 *	cpu_x86.asm互換I/F
 *	サブCPU実行
 */
void jsub_exec(void)
{
	cpu_exec(&jsubcpu);
}

/*
 *	cpu_x86.asm互換I/F
 *	サブCPU 1命令実行
 */
void jsub_line(void)
{
	cpu_execline(&jsubcpu);
}
#endif
#endif
