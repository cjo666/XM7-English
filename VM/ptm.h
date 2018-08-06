/*
 *	FM-7 EMULATOR "XM7"
 *
 *	Copyright (C) 1999-2017 ＰＩ．(Twitter:@xm6_original)
 *	Copyright (C) 2001-2017 Ryu Takegami (Twitter:@RyuTakegami)
 *
 *	[ プログラマブルタイマ(MB8873H) 簡易版 ]
 */

#if defined(MOUSE)

#ifndef _ptm_h_
#define _ptm_h_

#ifdef __cplusplus
extern "C" {
#endif
/*
 *	主要エントリ
 */
BOOL FASTCALL ptm_init(void);
										/* 初期化 */
void FASTCALL ptm_cleanup(void);
										/* クリーンアップ */
void FASTCALL ptm_reset(void);
										/* リセット */
BOOL FASTCALL ptm_readb(WORD addr, BYTE *dat);
										/* メモリ読み出し */
BOOL FASTCALL ptm_writeb(WORD addr, BYTE dat);
										/* メモリ書き込み */
BOOL FASTCALL ptm_save(int fileh);
										/* セーブ */
BOOL FASTCALL ptm_load(int fileh, int ver);
										/* ロード */

/*
 *	グローバル ワーク
 */
extern int ptm_counter[6];	
										/* カウンタ */
extern int ptm_counter_preset[6];
										/* カウンタプリセット値 */

extern BYTE ptm_mode_select[3];
										/* 動作モード */
extern BOOL ptm_running_flag[3];
										/* 動作中フラグ */

extern BOOL ptm_out_flag[3];
										/* 出力許可フラグ */
extern BOOL ptm_irq_flag_int[3];
										/* 割り込みフラグ */
extern BOOL ptm_irq_mask_int[3];
										/* 割り込みマスクフラグ */
extern BOOL ptm_mode_16bit[3];
										/* 16ビットカウントモード */
extern BOOL ptm_clock_type[3];
										/* クロック源 */

extern BOOL ptm_preset_mode;
										/* プリセット(InternalReset)モード */
extern BOOL ptm_select_reg1;
										/* レジスタ選択状態 */
extern BOOL ptm_clock_divide;
										/* クロック分周モード */
#ifdef __cplusplus
}
#endif

#endif	/* _ptm_h_ */
#endif
