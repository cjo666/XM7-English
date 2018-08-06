/*
 *	FM-7 EMULATOR "XM7"
 *
 *	Copyright (C) 1999-2017 ＰＩ．(Twitter:@xm6_original)
 *	Copyright (C) 2001-2017 Ryu Takegami (Twitter:@RyuTakegami)
 *
 *	[ MMR,TWR / I/O型RAMディスク ]
 */

#ifndef _mmr_h_
#define _mmr_h_

#ifdef __cplusplus
extern "C" {
#endif
/*
 *	主要エントリ
 */
BOOL FASTCALL mmr_init(void);
										/* 初期化 */
void FASTCALL mmr_cleanup(void);
										/* クリーンアップ */
void FASTCALL mmr_reset(void);
										/* リセット */
BOOL FASTCALL mmr_trans_twr(WORD addr, DWORD *taddr);
										/* TWRアドレス変換 */
DWORD FASTCALL mmr_trans_mmr(WORD addr);
										/* MMRアドレス変換 */
DWORD FASTCALL mmr_trans_phys_to_logi(DWORD target);
										/* 物理→論理アドレス変換 */
BOOL FASTCALL mmr_readb(WORD addr, BYTE *dat);
										/* メモリ読み出し */
BOOL FASTCALL mmr_writeb(WORD addr, BYTE dat);
										/* メモリ書き込み */
BOOL FASTCALL mmr_extrb(WORD *addr, BYTE *dat);
										/* MMR経由読み出し */
BOOL FASTCALL mmr_extbnio(WORD *addr, BYTE *dat);
										/* MMR経由読み出し(I/Oなし) */
BOOL FASTCALL mmr_extwb(WORD *addr, BYTE dat);
										/* MMR経由書き込み */
BOOL FASTCALL mmr_save(int fileh);
										/* セーブ */
BOOL FASTCALL mmr_load(int fileh, int ver);
										/* ロード */

/*
 *	主要ワーク
 */
extern BOOL mmr_flag;
										/* MMR有効フラグ */
extern BYTE mmr_seg;
										/* MMRセグメント */
extern BOOL mmr_modify;
										/* MMR状態変更フラグ */
#if XM7_VER >= 3
extern BYTE mmr_reg[0x80];
										/* MMRレジスタ */
extern BOOL mmr_ext;
										/* 拡張MMR有効フラグ */
extern BOOL mmr_fastmode;
										/* MMR高速フラグ */
extern BOOL mmr_extram;
										/* 拡張RAM有効フラグ */
extern BOOL mmr_fast_refresh;
										/* 高速リフレッシュフラグ */
extern BOOL twr_fastmode;
										/* TWR高速モード */
extern BOOL mmr_768kbmode;
										/* MMRモード(TRUE=768KB,FALSE=256KB) */
extern BOOL dsp_400linetiming;
										/* 400ラインタイミング出力フラグ */
#else
extern BYTE mmr_reg[0x40];
										/* MMRレジスタ */
#endif
#if XM7_VER == 1
extern BYTE bnk_reg;
										/* 拡張RAMバンクセレクトレジスタ */
extern BOOL mmr_64kbmode;
										/* MMRモード (TRUE=64KB,FALSE=192KB) */
#endif

extern BOOL twr_flag;
										/* TWR有効フラグ */
extern BYTE twr_reg;
										/* TWRレジスタ */
#ifdef __cplusplus
}
#endif

#endif	/* _mmr_h_ */
