/*
 *	FM-7 EMULATOR "XM7"
 *
 *	Copyright (C) 1999-2017 ＰＩ．(Twitter:@xm6_original)
 *	Copyright (C) 2001-2017 Ryu Takegami (Twitter:@RyuTakegami)
 *
 *	[ プログラマブルタイマ(MB8873H) 簡易版 ]
 *
 */

#if defined(MOUSE)

#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "xm7.h"
#include "device.h"
#include "mainetc.h"
#include "ptm.h"
#include "mouse.h"
#include "event.h"

/*
 *	グローバル ワーク
 */
int ptm_counter[6];						/* カウンタ */
int ptm_counter_preset[6];				/* カウンタプリセット値 */

BYTE ptm_mode_select[3];				/* 動作モード */
BOOL ptm_running_flag[3];				/* 動作中フラグ */

BOOL ptm_out_flag[3];					/* 出力許可フラグ */
BOOL ptm_irq_flag_int[3];				/* 割り込みフラグ */
BOOL ptm_irq_mask_int[3];					/* 割り込みマスクフラグ */
BOOL ptm_mode_16bit[3];					/* 16ビットカウントモード */
BOOL ptm_clock_type[3];					/* クロック源 */

BOOL ptm_preset_mode;					/* プリセット(InternalReset)モード */
BOOL ptm_select_reg1;					/* レジスタ選択状態 */
BOOL ptm_clock_divide;					/* クロック分周モード */

/*
 *	スタティック ワーク
 */
static BOOL ptm_counter_select[3];		/* デュアルモードレジスタ選択状態 */
static BYTE ptm_timer_adjust;			/* タイマ間隔近似用カウンタ */
static BYTE ptm_read_latch_buffer;		/* リード用ラッチバッファ */
static BYTE ptm_write_latch_buffer;		/* ライト用ラッチバッファ */

/*
 *	プロトタイプ宣言
 */
static BOOL FASTCALL ptm_event(void);


/*
 *	プログラマブルタイマ
 *	初期化
 */
BOOL FASTCALL ptm_init(void)
{
	return TRUE;
}

/*
 *	プログラマブルタイマ
 *	クリーンアップ
 */
void FASTCALL ptm_cleanup(void)
{
}

/*
 *	プログラマブルタイマ
 *	カウンタリセット
 */
static void FASTCALL ptm_counter_reset(void)
{
	int i;

	for (i=0; i<6; i++) {
		ptm_counter[i] = 0;
		ptm_counter_preset[i] = 0;
	}

	for (i=0; i<3; i++) {
		ptm_mode_select[i] = 0;
		ptm_running_flag[i] = FALSE;
		ptm_counter_select[i] = FALSE;
		ptm_clock_type[i] = FALSE;
	}

	ptm_preset_mode = TRUE;
}

/*
 *	プログラマブルタイマ
 *	リセット
 */
void FASTCALL ptm_reset(void)
{
	int i;

	ptm_counter_reset();

	for (i=0; i<3; i++) {
		ptm_out_flag[i] = FALSE;
		ptm_irq_flag_int[i] = FALSE;
		ptm_irq_mask_int[i] = TRUE;
		ptm_mode_16bit[i] = TRUE;
	}

	ptm_read_latch_buffer = 0x00;
	ptm_write_latch_buffer = 0x00;

	ptm_clock_divide = FALSE;
	ptm_select_reg1 = FALSE;

	/* イベント */
#if XM7_VER == 1
	if ((fm_subtype == FMSUB_FM8) || (mos_port != 3)) {
#else
	if (mos_port != 3) {
#endif
		schedule_delevent(EVENT_PTM_TIMER);
	}
	else {
		schedule_setevent(EVENT_PTM_TIMER, 52, ptm_event);
	}
	ptm_timer_adjust = 0;
}

/*
 *	プログラマブルタイマ
 *	イベント
 */
static BOOL FASTCALL ptm_event(void)
{
	BYTE i;

	/* プリセットモードなら何もしない */
	if (ptm_preset_mode) {
		return TRUE;
	}

	/* 19.2kHzタイマを近似する */
	ptm_timer_adjust = (BYTE)((ptm_timer_adjust + 1) % 12);
	if (ptm_timer_adjust == 0) {
		schedule_setevent(EVENT_PTM_TIMER, 53, ptm_event);
	}
	else {
		schedule_setevent(EVENT_PTM_TIMER, 52, ptm_event);
	}

	/* 各タイマを動かす */
	for (i=0; i<3; i++) {
		if (ptm_running_flag[i]) {

			if (ptm_clock_type[i]) {
				ptm_counter[i] -= 104;
			}
			else {
				ptm_counter[i] --;
			}

			/* 以下、カウンタが０以下になったら発動 */
			if (ptm_counter[i] <= 0) {
				/* フラグを立てる */
				ptm_irq_flag_int[i] = TRUE;

				/* マスクされていなければ割り込みをかける(CH0以外) */
				if (!(ptm_irq_mask_int[i] || ptm_irq_mask)) {
					ptm_irq_flag = TRUE;
					maincpu_irq();
				}

				if (ptm_mode_select[i] & 0x20) {
					/* シングルモードの場合はタイマ動作を解除する */
					ptm_running_flag[i] = FALSE;
				}
				else {
					/* カウンタリセット */
					if ((ptm_mode_16bit[i]) || (!ptm_counter_select[i])) {
						ptm_counter[i] += ptm_counter_preset[i];
					}
					else {
						ptm_counter[i] += ptm_counter_preset[i + 3];
					}
					ptm_counter_select[i] = !ptm_counter_select[i];
				}
			}
		}
	}

	return TRUE;
}

/*
 *	カウンタプリセット
 */
static void FASTCALL ptm_preset_counter(BYTE reg)
{
	/* 表カウンタ設定 */
	ptm_counter_preset[reg] = ptm_counter[reg];
	if (ptm_clock_divide) {
		ptm_counter_preset[reg] *= 8;
	}

	/* ８ビットモード時の裏カウンタ設定 */
	if (!ptm_mode_16bit[reg]) {
		ptm_counter_preset[reg + 3] = ptm_counter[reg + 3];
		if (ptm_clock_divide) {
			ptm_counter_preset[reg + 3] *= 8;
		}
	}

	/* フラグ類リセット */
	ptm_running_flag[reg] = TRUE;
	ptm_counter_select[reg] = FALSE;
	ptm_irq_flag_int[reg] = FALSE;
}

/*
 *	コントロールレジスタ設定
 */
static void FASTCALL ptm_control_set(BYTE reg, BYTE dat)
{
	/* bit7:出力マスク */
	if (dat & 0x80) {
		ptm_out_flag[reg] = TRUE;
	}
	else {
		ptm_out_flag[reg] = FALSE;
	}

	/* bit6:割り込みマスク */
	if (dat & 0x40) {
		ptm_irq_mask_int[reg] = FALSE;
	}
	else {
		ptm_irq_mask_int[reg] = TRUE;
	}

	/* bit1:クロック源選択 */
	if (dat & 0x02) {
		/* bit1=1 : Eクロック(2MHz)基準(まともに対応していません) */
		ptm_clock_type[reg] = TRUE;
	}
	else {
		/* bit1=0 : Cクロック(19.2kHz)基準 */
		ptm_clock_type[reg] = FALSE;
	}

	/* bit2:モード選択 */
	if (dat & 0x04) {
		/* bit2=1 : 8ビットモード */
		ptm_mode_16bit[reg] = FALSE;
	}
	else {
		/* bit2=0 : 16ビットモード */
		ptm_mode_16bit[reg] = TRUE;
	}

	/* bit5-3:モードセレクト */
	ptm_mode_select[reg] = (BYTE)(dat & 0x38);
	switch(dat & 0x38) {
		case 0x10 :	/* コンティニューモード(START) */
		case 0x30 :	/* シングルモード(START) */
					ptm_preset_counter(reg);
					break;

		case 0x00 :	/* コンティニューモード(LATCH) */
		case 0x20 :	/* シングルモード(LATCH) */
					break;

		case 0x08 :	/* 周波数比較モード(SHORT) */
		case 0x28 :	/* 周波数比較モード(LONG) */
		case 0x18 :	/* パルス比較モード(SHORT) */
		case 0x38 :	/* パルス比較モード(LONG) */
					ptm_running_flag[reg] = FALSE;
					ptm_irq_flag_int[reg] = FALSE;
					break;
	}
}

/*
 *	プログラマブルタイマ
 *	１バイト取得
 */
BOOL FASTCALL ptm_readb(WORD addr,BYTE *dat)
{
	BYTE tmp;
	BYTE reg;

#if XM7_VER == 1
	if ((fm_subtype == FMSUB_FM8) || (mos_port != 3)) {
#else
	if (mos_port != 3) {
#endif
		return FALSE;
	}

	switch (addr) {
		/* ステータスレジスタ */
		case 0xfde1:
			tmp = 0x00;
			if (ptm_irq_flag_int[0]) {
				tmp |= 0x01;
				ptm_irq_flag_int[0] = FALSE;
			}
			if (ptm_irq_flag_int[1]) {
				tmp |= 0x02;
				ptm_irq_flag_int[1] = FALSE;
			}
			if (ptm_irq_flag_int[2]) {
				tmp |= 0x04;
				ptm_irq_flag_int[2] = FALSE;
			}
			if (ptm_irq_flag) {
				tmp |= 0x80;
			}
			*dat = tmp;

			/* IRQを解除 */
			ptm_irq_flag = FALSE;
			maincpu_irq();
			return TRUE;

		/* カウンタ(上位バイト) */
		case 0xfde2:
		case 0xfde4:
		case 0xfde6:
			reg = (BYTE)((addr - 0xfde2) >> 1);
			if (ptm_mode_16bit[reg]) {
				/* １６ビットモード */
				*dat = (BYTE)(ptm_counter[reg] >> 8);
				ptm_read_latch_buffer = (BYTE)(ptm_counter[reg] & 0xFF);
			}
			else {
				/* ８ビットモード */
				*dat = (BYTE)(ptm_counter[reg] & 0xFF);
				ptm_read_latch_buffer = (BYTE)(ptm_counter[reg + 3] & 0xFF);
			}
			return TRUE;

		/* カウンタ(下位バイト,ラッチバッファ) */
		case 0xfde3:
		case 0xfde5:
		case 0xfde7:
			*dat = ptm_read_latch_buffer;
			return TRUE;
	}

	return FALSE;
}

/*
 *	プログラマブルタイマ
 *	１バイト書き込み
 */
BOOL FASTCALL ptm_writeb(WORD addr, BYTE dat)
{
	BYTE reg;

#if XM7_VER == 1
	if ((fm_subtype == FMSUB_FM8) || (mos_port != 3)) {
#else
	if (mos_port != 3) {
#endif
		return FALSE;
	}

	switch (addr) {
		/* コントロールレジスタ 1/3 */
		case 0xfde0:
			if (ptm_select_reg1) {
				/* bit0:タイマプリセット */
				if (dat & 0x01) {
					ptm_counter_reset();
				}
				else {
					ptm_control_set(0, dat);
					ptm_preset_mode = FALSE;
				}
			}
			else {
				/* bit0:クロック分周ビット */
				if (dat & 0x01) {
					ptm_clock_divide = TRUE;
				}
				else {
					ptm_clock_divide = FALSE;
				}
				ptm_control_set(2, dat);
			}
			return TRUE;

		/* コントロールレジスタ 2 */
		case 0xfde1:
			ptm_control_set(1, dat);
			/* bit0 : レジスタ選択ビット */
			if (dat & 0x01) {
				ptm_select_reg1 = TRUE;
			}
			else {
				ptm_select_reg1 = FALSE;
			}
			return TRUE;

		/* カウンタ(上位バイト,ラッチバッファ) */
		case 0xfde2:
		case 0xfde4:
		case 0xfde6:
			ptm_write_latch_buffer = (BYTE)dat;
			return TRUE;

		/* カウンタ(下位バイト) */
		case 0xfde3:
		case 0xfde5:
		case 0xfde7:
			reg = (BYTE)((addr - 0xfde2) >> 1);
			if (ptm_mode_16bit[reg]) {
				/* １６ビットモード */
				ptm_counter[reg] = (WORD)((ptm_write_latch_buffer << 8) | dat);
			}
			else {
				/* ８ビットモード */
				ptm_counter[reg + 0] = (WORD)ptm_write_latch_buffer;
				ptm_counter[reg + 3] = (WORD)dat;
			}

			/* LATCHモードのカウンタリセット処理 */
			if (!(ptm_mode_select[reg] & 0x10)) {
				ptm_preset_counter(reg);
			}
			return TRUE;
	}

	return FALSE;
}

/*
 *	プログラマブルタイマ
 *	セーブ
 */
BOOL FASTCALL ptm_save(int fileh)
{
	int i;

	for (i=0; i<6; i++) {
		if (!file_dword_write(fileh, ptm_counter[i])) {
			return FALSE;
		}
		if (!file_dword_write(fileh, ptm_counter_preset[i])) {
			return FALSE;
		}
	}

	for (i=0; i<3; i++) {
		if (!file_byte_write(fileh, ptm_mode_select[i])) {
			return FALSE;
		}
		if (!file_bool_write(fileh, ptm_running_flag[i])) {
			return FALSE;
		}

		if (!file_bool_write(fileh, ptm_out_flag[i])) {
			return FALSE;
		}
		if (!file_bool_write(fileh, ptm_irq_flag_int[i])) {
			return FALSE;
		}
		if (!file_bool_write(fileh, ptm_irq_mask_int[i])) {
			return FALSE;
		}
		if (!file_bool_write(fileh, ptm_mode_16bit[i])) {
			return FALSE;
		}
		if (!file_bool_write(fileh, ptm_clock_type[i])) {
			return FALSE;
		}
	}

	if (!file_bool_write(fileh, ptm_preset_mode)) {
		return FALSE;
	}
	if (!file_bool_write(fileh, ptm_select_reg1)) {
		return FALSE;
	}
	if (!file_bool_write(fileh, ptm_clock_divide)) {
		return FALSE;
	}

	return TRUE;
}

/*
 *	プログラマブルタイマ
 *	ロード
 */
BOOL FASTCALL ptm_load(int fileh, int ver)
{
	int	i;

	/* バージョンチェック */
#if XM7_VER == 1
	if (ver < 309) {
#elif XM7_VER == 2
	if ((ver >= 500) && (ver < 719)) {
#else
	if (((ver >= 500) && (ver < 719)) || ((ver >= 800) && (ver < 919))) {
#endif
		return TRUE;
	}

	for (i=0; i<6; i++) {
		if (!file_dword_read(fileh, (DWORD *)&ptm_counter[i])) {
			return FALSE;
		}
		if (!file_dword_read(fileh, (DWORD *)&ptm_counter_preset[i])) {
			return FALSE;
		}
	}

	for (i=0; i<3; i++) {
		if (!file_byte_read(fileh, &ptm_mode_select[i])) {
			return FALSE;
		}
		if (!file_bool_read(fileh, &ptm_running_flag[i])) {
			return FALSE;
		}

		if (!file_bool_read(fileh, &ptm_out_flag[i])) {
			return FALSE;
		}
		if (!file_bool_read(fileh, &ptm_irq_flag_int[i])) {
			return FALSE;
		}
		if (!file_bool_read(fileh, &ptm_irq_mask_int[i])) {
			return FALSE;
		}
		if (!file_bool_read(fileh, &ptm_mode_16bit[i])) {
			return FALSE;
		}
		if (!file_bool_read(fileh, &ptm_clock_type[i])) {
			return FALSE;
		}
	}

	if (!file_bool_read(fileh, &ptm_preset_mode)) {
		return FALSE;
	}
	if (!file_bool_read(fileh, &ptm_select_reg1)) {
		return FALSE;
	}
	if (!file_bool_read(fileh, &ptm_clock_divide)) {
		return FALSE;
	}

	/* イベント */
	schedule_handle(EVENT_PTM_TIMER, ptm_event);

	return TRUE;
}

#endif
