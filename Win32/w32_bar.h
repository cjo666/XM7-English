/*
 *	FM-7 EMULATOR "XM7"
 *
 *	Copyright (C) 1999-2017 ＰＩ．(Twitter:@xm6_original)
 *	Copyright (C) 2001-2017 Ryu Takegami (Twitter:@RyuTakegami)
 *
 *	[ Win32API コントロールバー ]
 */

#ifdef _WIN32

#ifndef _w32_bar_h_
#define _w32_bar_h_

#ifdef __cplusplus
extern "C" {
#endif
/*
 *	主要エントリ
 */
HWND FASTCALL CreateStatus(HWND hWnd);
										/* ステータスバー作成 */
void FASTCALL DrawStatus(void);
										/* 描画 */
void FASTCALL PaintStatus(void);
										/* すべて再描画 */
void FASTCALL SizeStatus(LONG cx);
										/* サイズ変更 */
void FASTCALL OwnerDrawStatus(DRAWITEMSTRUCT *pDI);
										/* オーナードロー */
void FASTCALL ResizeStatus(HWND hwnd, HWND hstatus);
										/* ステータスバーリサイズ */
void FASTCALL SetStatusMessage(UINT ID);
										/* ステータスメッセージ設定 */
void FASTCALL EndStatusMessage(void);
										/* ステータスメッセージ解除 */
void FASTCALL OnMenuSelect(WPARAM wParam);
										/* WM_MENUSELECT */
void FASTCALL OnExitMenuLoop(void);
										/* WM_EXITMENULOOP */
void FASTCALL InitThemeDLL(void);
										/* uxtheme.dll 初期化 */
void FASTCALL CleanThemeDLL(void);
										/* uxtheme.dll クリーンアップ */
void FASTCALL ChangeStatusBorder(HWND hwnd);
										/* メッセージ部ボーダー描画 */

/*
 *	主要ワーク
 */
extern HWND hStatusBar;
										/* ステータスバー */
extern BOOL bStatusMessage;
										/* ステータスメッセージ表示状態 */
extern int uPaneX[3];
										/* Drap&Drop用ペインX座標 */
#ifdef __cplusplus
}
#endif

#endif	/* _w32_bar_h_ */
#endif	/* _WIN32 */
