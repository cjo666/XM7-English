/*
 *	FM-7 EMULATOR "XM7"
 *
 *	Copyright (C) 1999-2017 ＰＩ．(Twitter:@xm6_original)
 *	Copyright (C) 2001-2017 Ryu Takegami (Twitter:@RyuTakegami)
 *
 *	[ インテリジェントマウス/FMマウス ]
 */

#if defined(MOUSE)

#ifndef _mouse_h_
#define _mouse_h_

#ifdef __cplusplus
extern "C" {
#endif
/*
 *	主要エントリ
 */
BOOL FASTCALL mos_init(void);
										/* 初期化 */
void FASTCALL mos_cleanup(void);
										/* クリーンアップ */
void FASTCALL mos_reset(void);
										/* リセット */
void FASTCALL mos_strobe_signal(BOOL strb);
										/* ストローブ信号処理 */
BYTE FASTCALL mos_readdata(BYTE trigger);
										/* データ読み込み */
BOOL FASTCALL mos_save(int fileh);
										/* セーブ */
BOOL FASTCALL mos_load(int fileh, int ver);
										/* ロード */
BOOL FASTCALL mouse_readb(WORD addr, BYTE *dat);
										/* マウスセット I/O読み出し */
BOOL FASTCALL mouse_writeb(WORD addr, BYTE dat);
										/* マウスセット I/O書き込み */

/*
 *	主要ワーク
 */
extern BYTE mos_port;
										/* マウス接続ポート */
extern BOOL mos_capture;
										/* マウスキャプチャフラグ */
#ifdef __cplusplus
}
#endif

#endif	/* _mouse_h_ */

#endif	/* MOUSE */
