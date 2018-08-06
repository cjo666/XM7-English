/*
 *	FM-7 EMULATOR "XM7"
 *
 *	Copyright (C) 1999-2017 ＰＩ．(Twitter:@xm6_original)
 *	Copyright (C) 2001-2017 Ryu Takegami (Twitter:@RyuTakegami)
 *
 *	[ メインCPU ]
 *
 *	RHG履歴
 *	  2002.05.07		C言語版CPUコアに再対応
 *	  2003.11.21		XM7 V1.1対応に伴い、テープカウンタの精度を向上
 *	  2010.11.15		FM-8モード時にフロッピーからのIRQを禁止するよう変更
 */

#include "xm7.h"
#include "subctrl.h"
#include "keyboard.h"
#include "mainetc.h"
#include "tapelp.h"
#include "device.h"

/*
 *	グローバル ワーク
 */
cpu6809_t maincpu;						/* メインCPUレジスタ情報(6809) */
#if XM7_VER == 1 && defined(Z80CARD)
KMZ80_CONTEXT mainz80;					/* メインCPUコンテキスト(Z80) */
BOOL main_z80mode;						/* メインCPU Z80選択フラグ */
DWORD z80_cycles;						/* Z80サイクル数計算用カウンタ */
DWORD z80_cycles_sub;					/* Z80サイクル数補正用カウンタ */
BOOL z80_irq_through;					/* IRQ信号通過フラグ */
BOOL z80_firq_through;					/* FIRQ信号通過フラグ */
BOOL z80_nmi_through;					/* NMI信号通過フラグ */
#endif

/*
 *	プロトタイプ宣言
 */
void main_reset(void);
void main_line(void);
void main_exec(void);

#if XM7_VER == 1 && defined(Z80CARD)
/*
 *	メインCPU(Z80)
 *	メモリリード用ラッパ関数
 */
static Uint32 mainmem_readb_z80(void *u, Uint32 a)
{
	UNUSED(u);

	return (Uint32)mainmem_readb((WORD)a);
}

/*
 *	メインCPU(Z80)
 *	メモリライト用ラッパ関数
 */
static void mainmem_writeb_z80(void *u, Uint32 a, Uint32 d)
{
	UNUSED(u);

	mainmem_writeb((WORD)a, (BYTE)d);
}

/*
 *	メインCPU(Z80)
 *	I/Oリード用ラッパ関数
 */
static Uint32 mainio_readb_z80(void *u, Uint32 a)
{
	UNUSED(u);
	UNUSED(a);

	return 0xff;
}

/*
 *	メインCPU(Z80)
 *	I/Oライト用ラッパ関数
 */
static void mainio_writeb_z80(void *u, Uint32 a, Uint32 d)
{
	UNUSED(u);
	UNUSED(a);
	UNUSED(d);
}
#endif

/*
 *	メインCPU
 *	初期化
 */
BOOL FASTCALL maincpu_init(void)
{
	maincpu.readmem = mainmem_readb;
	maincpu.writemem = mainmem_writeb;
#if XM7_VER == 1 && defined(Z80CARD)
	main_z80mode = FALSE;
	z80_irq_through = FALSE;
	z80_firq_through = FALSE;
	z80_nmi_through = FALSE;
	mainz80.memread = mainmem_readb_z80;
	mainz80.memwrite = mainmem_writeb_z80;
	mainz80.ioread = mainio_readb_z80;
	mainz80.iowrite = mainio_writeb_z80;
#endif

	return TRUE;
}

/*
 *	メインCPU
 *	クリーンアップ
 */
void FASTCALL maincpu_cleanup(void)
{
	return;
}

/*
 *	メインCPU
 *	リセット
 */
void FASTCALL maincpu_reset(void)
{
	main_reset();
#if XM7_VER == 1 && defined(Z80CARD)
	kmz80_reset(&mainz80);
	main_z80mode = FALSE;
	z80_cycles = z80_cycles_sub = 0;
#endif
}

/*
 *	メインCPU
 *	１行実行
 */
void FASTCALL maincpu_execline(void)
{
	DWORD cnt;

#if XM7_VER >= 2
	cnt = 0x100;
#else
	if (lowspeed_mode ||
		((fm_subtype == FMSUB_FM8) && tape_motor && motoron_lowspeed)) {
		cnt = 0x09e;
	}
	else {
		cnt = 0x100;
	}
#endif

	/* 1行実行 */
#if XM7_VER == 1 && defined(Z80CARD)
	if (main_z80mode) {
		z80_cycles = kmz80_exec(&mainz80, 1);
		z80_cycles_sub = (z80_cycles_sub + (z80_cycles % 2));
		maincpu.cycle = (WORD)((z80_cycles / 2) + (z80_cycles_sub / 2));
		maincpu.total += maincpu.cycle;
		z80_cycles_sub %= 2;
	}
	else {
		main_line();
	}
#else
	main_line();
#endif

	/* テープカウンタ処理 */
	if (tape_motor) {
		tape_subcnt += (DWORD)(maincpu.cycle << 4);
		if (tape_subcnt >= cnt) {
			tape_subcnt -= cnt;
			tape_count++;
			if (tape_count == 0) {
				tape_count = 0xffff;
			}
		}
	}
}

/*
 *	メインCPU
 *	実行
 */
void FASTCALL maincpu_exec(void)
{
	DWORD cnt;

#if XM7_VER >= 2
	cnt = 0x100;
#else
	if (lowspeed_mode ||
		((fm_subtype == FMSUB_FM8) && tape_motor && motoron_lowspeed)) {
		cnt = 0x09e;
	}
	else {
		cnt = 0x100;
	}
#endif

	/* 実行 */
#if XM7_VER == 1 && defined(Z80CARD)
	if (main_z80mode) {
		z80_cycles = kmz80_exec(&mainz80, 1);
		z80_cycles_sub = (z80_cycles_sub + (z80_cycles % 2));
		maincpu.cycle = (WORD)((z80_cycles / 2) + (z80_cycles_sub / 2));
		maincpu.total += maincpu.cycle;
		z80_cycles_sub %= 2;
	}
	else {
		main_exec();
	}
#else
	main_exec();
#endif

	/* テープカウンタ処理 */
	if (tape_motor) {
		tape_subcnt += (DWORD)(maincpu.cycle << 4);
		if (tape_subcnt >= cnt) {
			tape_subcnt -= cnt;
			tape_count++;
			if (tape_count == 0) {
				tape_count = 0xffff;
			}
		}
	}
}

/*
 *	メインCPU
 *	NMI割り込み設定
 */
void FASTCALL maincpu_nmi(void)
{
	/* FM-77の1MB FDD対応(IRQ)で使われているが、未対応 */
	if (maincpu.intr & INTR_SLOAD){
		maincpu.intr |= INTR_NMI;
#if XM7_VER == 1 && defined(Z80CARD)
		if (z80_nmi_through) {
			mainz80.regs8[REGID_NMIREQ] |= 0x01;
		}
#endif
	}
}

/*
 *	メインCPU
 *	FIRQ割り込み設定
 */
void FASTCALL maincpu_firq(void)
{
	/* BREAKキー及び、サブCPUからのアテンション割り込み */
	if (break_flag || subattn_flag) {
		maincpu.intr |= INTR_FIRQ;
#if XM7_VER == 1 && defined(Z80CARD)
	if ((fm_subtype != FMSUB_FM8) && z80_firq_through) {
			mainz80.regs8[REGID_INTREQ] |= 0x01;
		}
#endif
	}
	else {
		maincpu.intr &= ~INTR_FIRQ;
#if XM7_VER == 1 && defined(Z80CARD)
		if ((fm_subtype != FMSUB_FM8) && z80_firq_through) {
			mainz80.regs8[REGID_INTREQ] &= (BYTE)~0x01;
		}
#endif
	}
}

/*
 *	メインCPU
 *	IRQ割り込み設定
 */
void FASTCALL maincpu_irq(void)
{
#if XM7_VER == 1
	/* FM-8モード時のIRQ割り込み設定 */
	if (fm_subtype == FMSUB_FM8) {
		if (txrdy_irq_flag ||
			rxrdy_irq_flag ||
			syndet_irq_flag) {
			maincpu.intr |= INTR_IRQ;
#if defined(Z80CARD)
			if (z80_irq_through) {
				mainz80.regs8[REGID_INTREQ] |= 0x01;
			}
#endif
		}
		else {
			maincpu.intr &= ~INTR_IRQ;
#if defined(Z80CARD)
			if (z80_irq_through) {
				mainz80.regs8[REGID_INTREQ] &= (BYTE)~0x01;
			}
#endif
		}
		return;
	}
#endif

	/* IRQ割り込み設定 */
	if ((key_irq_flag && !(key_irq_mask)) ||
		timer_irq_flag ||
		lp_irq_flag ||
		mfd_irq_flag ||
		txrdy_irq_flag ||
		rxrdy_irq_flag ||
		syndet_irq_flag ||
#if XM7_VER >= 3
		dma_irq_flag ||
#endif
		ptm_irq_flag ||
		opn_irq_flag ||
		whg_irq_flag ||
		thg_irq_flag) {
		maincpu.intr |= INTR_IRQ;
#if XM7_VER == 1 && defined(Z80CARD)
		if (z80_irq_through) {
			mainz80.regs8[REGID_INTREQ] |= 0x01;
		}
#endif
	}
	else {
		maincpu.intr &= ~INTR_IRQ;
#if XM7_VER == 1 && defined(Z80CARD)
		if (z80_irq_through) {
			mainz80.regs8[REGID_INTREQ] &= (BYTE)~0x01;
		}
#endif
	}
}

/*
 *	メインCPU
 *	セーブ
 */
BOOL FASTCALL maincpu_save(int fileh)
{
	/* プラットフォームごとのパッキング差を回避するため、分割 */
	if (!file_byte_write(fileh, maincpu.cc)) {
		return FALSE;
	}

	if (!file_byte_write(fileh, maincpu.dp)) {
		return FALSE;
	}

	if (!file_word_write(fileh, maincpu.acc.d)) {
		return FALSE;
	}

	if (!file_word_write(fileh, maincpu.x)) {
		return FALSE;
	}

	if (!file_word_write(fileh, maincpu.y)) {
		return FALSE;
	}

	if (!file_word_write(fileh, maincpu.u)) {
		return FALSE;
	}

	if (!file_word_write(fileh, maincpu.s)) {
		return FALSE;
	}

	if (!file_word_write(fileh, maincpu.pc)) {
		return FALSE;
	}

	if (!file_word_write(fileh, maincpu.intr)) {
		return FALSE;
	}

	if (!file_word_write(fileh, maincpu.cycle)) {
		return FALSE;
	}

	if (!file_word_write(fileh, maincpu.total)) {
		return FALSE;
	}

#if XM7_VER == 1 && defined(Z80CARD)
	if (!file_bool_write(fileh, main_z80mode)) {
		return FALSE;
	}
	if (!file_dword_write(fileh, z80_cycles)) {
		return FALSE;
	}
	if (!file_dword_write(fileh, z80_cycles_sub)) {
		return FALSE;
	}
	if (!file_write(fileh, mainz80.regs8, sizeof(mainz80.regs8))) {
		return FALSE;
	}
	if (!file_dword_write(fileh, mainz80.sp)) {
		return FALSE;
	}
	if (!file_dword_write(fileh, mainz80.pc)) {
		return FALSE;
	}
	if (!file_dword_write(fileh, mainz80.saf)) {
		return FALSE;
	}
	if (!file_dword_write(fileh, mainz80.sbc)) {
		return FALSE;
	}
	if (!file_dword_write(fileh, mainz80.sde)) {
		return FALSE;
	}
	if (!file_dword_write(fileh, mainz80.shl)) {
		return FALSE;
	}
	if (!file_dword_write(fileh, mainz80.t_fl)) {
		return FALSE;
	}
	if (!file_dword_write(fileh, mainz80.t_dx)) {
		return FALSE;
	}
#endif

	return TRUE;
}

/*
 *	メインCPU
 *	ロード
 */
BOOL FASTCALL maincpu_load(int fileh, int ver)
{
	/* バージョンチェック */
	if (ver < 200) {
		return FALSE;
	}

	/* プラットフォームごとのパッキング差を回避するため、分割 */
	if (!file_byte_read(fileh, &maincpu.cc)) {
		return FALSE;
	}

	if (!file_byte_read(fileh, &maincpu.dp)) {
		return FALSE;
	}

	if (!file_word_read(fileh, &maincpu.acc.d)) {
		return FALSE;
	}

	if (!file_word_read(fileh, &maincpu.x)) {
		return FALSE;
	}

	if (!file_word_read(fileh, &maincpu.y)) {
		return FALSE;
	}

	if (!file_word_read(fileh, &maincpu.u)) {
		return FALSE;
	}

	if (!file_word_read(fileh, &maincpu.s)) {
		return FALSE;
	}

	if (!file_word_read(fileh, &maincpu.pc)) {
		return FALSE;
	}

	if (!file_word_read(fileh, &maincpu.intr)) {
		return FALSE;
	}

	if (!file_word_read(fileh, &maincpu.cycle)) {
		return FALSE;
	}

	if (!file_word_read(fileh, &maincpu.total)) {
		return FALSE;
	}

#if XM7_VER == 1 && defined(Z80CARD)
	if ((ver >= 310) && (ver <= 499)) {
		if (!file_bool_read(fileh, &main_z80mode)) {
			return FALSE;
		}
		if (!file_dword_read(fileh, &z80_cycles)) {
			return FALSE;
		}
		if (!file_dword_read(fileh, &z80_cycles_sub)) {
			return FALSE;
		}
		if (!file_read(fileh, mainz80.regs8, REGID_REGS8SIZE)) {
			return FALSE;
		}
		if (!file_dword_read(fileh, (DWORD *)&mainz80.sp)) {
			return FALSE;
		}
		if (!file_dword_read(fileh, (DWORD *)&mainz80.pc)) {
			return FALSE;
		}
		if (!file_dword_read(fileh, (DWORD *)&mainz80.saf)) {
			return FALSE;
		}
		if (!file_dword_read(fileh, (DWORD *)&mainz80.sbc)) {
			return FALSE;
		}
		if (!file_dword_read(fileh, (DWORD *)&mainz80.sde)) {
			return FALSE;
		}
		if (!file_dword_read(fileh, (DWORD *)&mainz80.shl)) {
			return FALSE;
		}
		if (!file_dword_read(fileh, (DWORD *)&mainz80.t_fl)) {
			return FALSE;
		}
		if (!file_dword_read(fileh, (DWORD *)&mainz80.t_dx)) {
			return FALSE;
		}
	}
	else {
		kmz80_reset(&mainz80);
		main_z80mode = FALSE;
		z80_cycles = z80_cycles_sub = 0;
	}
#endif

	return TRUE;
}
