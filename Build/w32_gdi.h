/*
 *	FM-7 EMULATOR "XM7"
 *
 *	Copyright (C) 1999-2017 ＰＩ．(Twitter:@xm6_original)
 *	Copyright (C) 2001-2017 Ryu Takegami (Twitter:@RyuTakegami)
 *
 *	[ Win32API ウインドウ表示 ]
 */

#ifdef _WIN32

#ifndef _w32_gdi_h_
#define _w32_gdi_h_

#ifdef __cplusplus
extern "C" {
#endif
/*
 *	主要エントリ
 */
void FASTCALL InitGDI(void);
										/* 初期化 */
void FASTCALL CleanGDI(void);
										/* クリーンアップ */
BOOL FASTCALL SelectGDI(HWND hWnd);
										/* セレクト */
void FASTCALL DrawGDI(HWND hWnd, HDC hDC);
										/* 描画 */
void FASTCALL DrawRasterGDI(int nRaster);
										/* ラスタ描画 */
void FASTCALL DrawPostRenderGDI(HWND hWnd, HDC hDC);
										/* ラスタレンダリング時描画 */
void FASTCALL EnterMenuGDI(HWND hWnd);
										/* メニュー開始 */
void FASTCALL ExitMenuGDI(void);
										/* メニュー終了 */
void FASTCALL VramGDI(WORD addr);
										/* VRAM書き込み通知 */
void FASTCALL TvramGDI(WORD addr);
										/* テキストVRAM書き込み通知 */
void FASTCALL DigitalGDI(void);
										/* TTLパレット通知 */
void FASTCALL AnalogGDI(void);
										/* アナログパレット通知 */
void FASTCALL ReDrawGDI(void);
										/* 再描画通知 */
void FASTCALL ReDrawTVRamGDI(void);
										/* テキスト再描画通知 */
#if XM7_VER >= 3
void FASTCALL WindowGDI(void);
										/* ハードウェアウィンドウ通知 */
#endif
void SelectCancelGDI(void);
										/*	セレクトキャンセル処理 */

/*
 *	主要ワーク
 */
extern DWORD rgbTTLGDI[16];
										/* デジタルパレット */
extern DWORD rgbAnalogGDI[4096];
										/* アナログパレット */
extern BYTE *pBitsGDI;
										/* ビットデータ */
#ifdef __cplusplus
}
#endif

#endif	/* _w32_gdi_h_ */
#endif	/* _WIN32 */
