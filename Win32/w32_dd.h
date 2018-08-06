/*
 *	FM-7 EMULATOR "XM7"
 *
 *	Copyright (C) 1999-2017 ＰＩ．(Twitter:@xm6_original)
 *	Copyright (C) 2001-2017 Ryu Takegami (Twitter:@RyuTakegami)
 *
 *	[ Win32API DirectDraw ]
 */

#ifdef _WIN32

#ifndef _w32_dd_h_
#define _w32_dd_h_

#ifdef __cplusplus
extern "C" {
#endif
/*
 *	定数定義
 */
#define	DDRES_400LINE		0
#define	DDRES_480LINE		1
#define	DDRES_WUXGA			2
#define	DDRES_SXGA			3
#define	DDRES_WXGA800		4

/*
 *	主要エントリ
 */
void FASTCALL InitDD(void);
										/* 初期化 */
void FASTCALL CleanDD(void);
										/* クリーンアップ */
BOOL FASTCALL SelectDD(void);
										/* セレクト */
void FASTCALL DrawDD(void);
										/* 描画 */
void FASTCALL DrawRasterDD(int nRaster);
										/* ラスタ描画 */
void FASTCALL DrawPostRenderDD(void);
										/* ラスタレンダリング時描画 */
void FASTCALL EnterMenuDD(HWND hWnd);
										/* メニュー開始 */
void FASTCALL ExitMenuDD(void);
										/* メニュー終了 */
void FASTCALL VramDD(WORD addr);
										/* VRAM書き込み通知 */
void FASTCALL TvramDD(WORD addr);
										/* テキストVRAM書き込み通知 */
void FASTCALL DigitalDD(void);
										/* TTLパレット通知 */
void FASTCALL AnalogDD(void);
										/* アナログパレット通知 */
void FASTCALL ReDrawDD(void);
										/* 再描画通知 */
void FASTCALL ReDrawTVRamDD(void);
										/* 再描画通知 */
#if XM7_VER >= 3
void FASTCALL WindowDD(void);
										/* ハードウェアウィンドウ通知 */
#endif

/*
 *	主要ワーク
 */
#if XM7_VER == 1
extern DWORD rgbTTLDD[16];
										/* 640x200 パレット */
#else
extern DWORD rgbTTLDD[8];
										/* 640x200 パレット */
#endif
extern DWORD rgbAnalogDD[4096];
										/* 320x200 パレット */
extern BYTE nDDResolutionMode;
										/* フルスクリーン時解像度 */
extern BOOL bDD480Status;
										/* 640x480 ステータスフラグ */
extern BOOL bDDtruecolor;
										/* TrueColor優先フラグ */
#ifdef __cplusplus
}
#endif

#endif	/* _w32_dd_h_ */
#endif	/* _WIN32 */
