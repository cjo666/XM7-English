/*
 *	FM-7 EMULATOR "XM7"
 *
 *	Copyright (C) 1999-2017 ＰＩ．(Twitter:@xm6_original)
 *	Copyright (C) 2001-2017 Ryu Takegami (Twitter:@RyuTakegami)
 *
 *	[ Win32API キーボード・ジョイスティック・マウス ]
 */

#ifdef _WIN32

#ifndef _w32_kbd_h_
#define _w32_kbd_h_

/*
 *	定数、型定義
 */
#define KNT_KANJI		0
#define KNT_KANA		1
#define KNT_CAPS		2
#define KNT_RSHIFT		3
#define KEYBUFFER_SIZE	64

#define MOSCAP_NONE		0
#define MOSCAP_WMESSAGE	1
#define MOSCAP_WHEELWM	2

#define VKNT_CAPITAL	0xf0
#define VKNT_KATAKANA	0xf1
#define VKNT_KANA		0xf2
#define VKNT_KANJI1		0xf3
#define VKNT_KANJI2		0xf4

#ifdef __cplusplus
extern "C" {
#endif
/*
 *	主要エントリ
 */
void FASTCALL InitKbd(void);
										/* 初期化 */
void FASTCALL CleanKbd(void);
										/* クリーンアップ */
BOOL FASTCALL SelectKbd(HWND hWnd);
										/* セレクト */
void FASTCALL SetMenuExitTimer(void);
										/* メニュー抜け出しタイマ設定 */
void FASTCALL PollKbd(void);
										/* キーボードポーリング */
void FASTCALL PollJoy(void);
										/* ジョイスティックポーリング */
void FASTCALL GetDefMapKbd(BYTE *pMap, int mode);
										/* デフォルトマップ取得 */
void FASTCALL SetMapKbd(BYTE *pMap);
										/* マップ設定 */
BOOL FASTCALL GetKbd(BYTE *pBuf);
										/* ポーリング＆キー情報取得 */
#if defined(KBDPASTE)
void FASTCALL PasteClipboardKbd(HWND hWnd);
										/*	キーボードペースト */
void FASTCALL PasteKbd(char *pstring);
										/*	キーボードペースト */
#endif
#if defined(MOUSE)
void FASTCALL PollMos(void);
										/* マウスポーリング */
void FASTCALL SetMouseCapture(BOOL en);
										/* マウスキャプチャ設定 */
#endif

/*
 *	主要ワーク
 */
extern BYTE kbd_map[256];
										/* キーボード マップ */
extern BYTE kbd_table[256];
										/* 対応するFM-7物理コード */
extern int nJoyType[2];
										/* ジョイスティックタイプ */
extern int nJoyRapid[2][2];
										/* 連射タイプ */
extern int nJoyCode[2][7];
										/* 生成コード */
extern BOOL bKbdReal;
										/* 擬似リアルタイムキースキャン */
extern BOOL bTenCursor;
										/* テンキー変換 */
extern BOOL bArrow8Dir;
										/* テンキー変換 8方向モード */
extern BOOL bNTkeyPushFlag[4];
										/* キー押下フラグ(NT) */
extern BOOL bNTkeyMakeFlag[128];
										/* キーMake中フラグ(NT) */
extern BOOL bNTkbMode;
										/* NT対策中フラグ */
#if defined(KBDPASTE)
extern UINT uPasteWait;
										/* ペースト待ち時間(ms) */
extern UINT uPasteWaitCntl;
										/* ペースト待ち時間(コントロールコード) */
#endif
#if defined(MOUSE)
extern BYTE uMidBtnMode;
										/* 中央ボタン状態取得モード */
extern BOOL bDetectMouse;
										/* マウス確認フラグ */
#endif
#ifdef __cplusplus
}
#endif

#endif	/* _w32_kbd_h_ */
#endif	/* _WIN32 */
