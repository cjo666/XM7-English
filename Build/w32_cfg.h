/*
 *	FM-7 EMULATOR "XM7"
 *
 *	Copyright (C) 1999-2017 ＰＩ．(Twitter:@xm6_original)
 *	Copyright (C) 2001-2017 Ryu Takegami (Twitter:@RyuTakegami)
 *
 *	[ Win32API コンフィギュレーション ]
 */

#ifdef _WIN32

#ifndef _w32_cfg_h_
#define _w32_cfg_h_

#ifdef __cplusplus
extern "C" {
#endif
/*
 *	主要エントリ
 */
void FASTCALL LoadCfg(void);
										/* 設定ロード */
BOOL FASTCALL LoadCfg_DoubleSize(void);
										/* 設定ロード(2倍拡大専用) */
BOOL FASTCALL LoadCfg_LanguageMode(void);
										/* 設定ロード(言語モード専用) */
void FASTCALL SaveCfg(void);
										/* 設定セーブ */
void FASTCALL ApplyCfg(void);
										/* 設定適用 */
void FASTCALL GetCfg(void);
										/* 設定取得 */
void FASTCALL SetMachineVersion(void);
										/* 動作機種再設定 */
void FASTCALL OnConfig(HWND hWnd);
										/* 設定ダイアログ */

/*
 *	主要ワーク
 */
#ifdef __cplusplus
}
#endif

#endif	/* _w32_cfg_h_ */
#endif	/* _WIN32 */
