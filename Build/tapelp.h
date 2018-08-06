/*
 *	FM-7 EMULATOR "XM7"
 *
 *	Copyright (C) 1999-2017 ＰＩ．(Twitter:@xm6_original)
 *	Copyright (C) 2001-2017 Ryu Takegami (Twitter:@RyuTakegami)
 *	line printer support 2010-2013 by Ben.JP
 *
 *	[ カセットテープ＆プリンタ ]
 */

#ifndef _tapelp_h_
#define _tapelp_h_

/*
 *	定数、型定義
 */
#define	TAPE_SAVEBUFSIZE	4096			/* テープ書き込みバッファサイズ */
#define	TAPE_LOADBUFSIZE	4096			/* テープ読み込みバッファサイズ */
#define	LP_TEMPFILENAME		"xm7_lpr.$$$"	/* プリンタ テンポラリファイル名 */
#define	LP_DISABLE			0				/* プリンタ エミュレーション無効 */
#define	LP_EMULATION		1				/* プリンタ エミュレーション有効 */
#define	LP_LOG				2				/* プリンタ ログファイル出力 */
#define	LP_JASTSOUND		3				/* プリンタ ジャストサウンド出力 */

#ifdef __cplusplus
extern "C" {
#endif
/*
 *	主要エントリ
 */
BOOL FASTCALL tapelp_init(void);
										/* 初期化 */
void FASTCALL tapelp_cleanup(void);
										/* クリーンアップ */
void FASTCALL tapelp_reset(void);
										/* リセット */
BOOL FASTCALL tapelp_readb(WORD addr, BYTE *dat);
										/* メモリ読み出し */
BOOL FASTCALL tapelp_writeb(WORD addr, BYTE dat);
										/* メモリ書き込み */
BOOL FASTCALL tapelp_save(int fileh);
										/* セーブ */
BOOL FASTCALL tapelp_load(int fileh, int ver);
										/* ロード */
void FASTCALL lp_setfile(char *fname);
										/* プリンタファイル設定 */
#if defined(LPRINT)
void FASTCALL lp_print(void);
										/* プリンタへ印刷 */
#endif
void FASTCALL tape_setfile(char *fname);
										/* テープファイル設定 */
void FASTCALL tape_setrec(BOOL flag);
										/* テープ録音フラグ設定 */
void FASTCALL tape_rew(void);
										/* テープ巻き戻し */
void FASTCALL tape_rewtop(void);
										/* テープを最初まで巻き戻し */
void FASTCALL tape_ff(void);
										/* テープ早送り */

/*
 *	主要ワーク
 */
extern BOOL tape_in;
										/* テープ 入力データ */
extern BOOL tape_out;
										/* テープ 出力データ */
extern BOOL tape_motor;
										/* テープ モータ */
extern BOOL tape_rec;
										/* テープ 録音中 */
extern BOOL tape_writep;
										/* テープ 録音不可 */
extern WORD tape_count;
										/* テープ カウンタ */
extern DWORD tape_subcnt;
										/* テープ サブカウント */
extern int tape_fileh;
										/* テープ ファイルハンドル */
extern DWORD tape_offset;
										/* テープ ファイルオフセット */
extern char tape_fname[256+1];
										/* テープ ファイルネーム */
extern BOOL tape_monitor;
										/* テープ テープ音モニタフラグ */
extern BOOL tape_sound;
										/* テープ リレー音出力フラグ */

#if defined(LPRINT)
extern BYTE	lp_use;
										/* プリンタ 使用モード */
#endif
extern BYTE lp_data;
										/* プリンタ 出力データ */
extern BOOL lp_busy;
										/* プリンタ BUSYフラグ */
extern BOOL lp_error;
										/* プリンタ エラーフラグ */
extern BOOL lp_pe;
										/* プリンタ PEフラグ */
extern BOOL lp_ackng;
										/* プリンタ ACKフラグ */
extern BOOL lp_online;
										/* プリンタ オンライン */
extern BOOL lp_strobe;
										/* プリンタ ストローブ */
extern char lp_fname[256+1];
										/* プリンタ ファイルネーム */
extern int lp_fileh;
										/* プリンタ ファイルハンドル */

#ifdef __cplusplus
}
#endif

#endif	/* _tapelp_h_ */
