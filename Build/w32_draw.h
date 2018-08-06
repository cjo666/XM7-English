/*
 *	FM-7 EMULATOR "XM7"
 *
 *	Copyright (C) 1999-2017 ＰＩ．(Twitter:@xm6_original)
 *	Copyright (C) 2001-2017 Ryu Takegami (Twitter:@RyuTakegami)
 *
 *	[ Win32API 表示 ]
 */

#ifdef _WIN32

#ifndef _w32_draw_h_
#define _w32_draw_h_

/*
 *	定数定義(V2用)
 */
#define	V2XPoint	464
#define	V2YPoint	10
#define	V2XSize		27
#define	V2YSize		14
#define	V2XPxSz		5
#define	V2YPxSz		2

#ifdef _w32_h_

#ifdef __cplusplus
extern "C" {
#endif
/*
 *	主要エントリ
 */
void FASTCALL InitDraw(void);
										/* 初期化 */
void FASTCALL CleanDraw(void);
										/* クリーンアップ */
BOOL FASTCALL SelectDraw(HWND hWnd);
										/* セレクト */
void FASTCALL ModeDraw(HWND hWnd, BOOL bFullScreen);
										/* 描画モード変更 */
void FASTCALL OnPaint(HWND hWnd);
										/* 再描画 */
void FASTCALL OnDraw(HWND hWnd, HDC hDC);
										/* 部分描画 */
void FASTCALL EnterMenu(HWND hWnd);
										/* メニュー開始 */
void FASTCALL ExitMenu(void);
										/* メニュー終了 */
void FASTCALL SetDirtyFlag(int top, int bottom, BOOL flag);
										/* 再描画ラスタ一括設定 */
#endif	/* _w32_h_ */


/*
 *	主要ワーク
 */
extern BOOL bFullScreen;
										/* フルスクリーン */
extern BOOL bFullRequested;
										/* フルスクリーン要求 */
extern BOOL bDrawSelected;
										/* セレクト済みフラグ */
extern BOOL bFullScan;
										/* フルスキャン(Window) */
extern BOOL bFullScanFS;
										/* フルスキャン(FullScreen) */
extern BOOL bDoubleSize;
										/* 2倍表示フラグ */
extern BOOL bPseudo400Line;
										/* 疑似400ラインフラグ */
#if XM7_VER == 1
extern BOOL bGreenMonitor;
										/* グリーンモニタフラグ */
#endif
#if XM7_VER == 2
extern BOOL bTTLMonitor;
										/* TTLモニタフラグ */
#endif
extern BOOL bRasterRendering;
										/* ラスタレンダリングフラグ */
#if ((XM7_VER <= 2) && defined(FMTV151))
extern const BYTE nV2data[];
										/* V2データ */
#endif
#ifdef __cplusplus
}
#endif

#endif	/* _w32_draw_h_ */
#endif	/* _WIN32 */
