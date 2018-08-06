/*
 *	FM-7 EMULATOR "XM7"
 *
 *	Copyright (C) 1999-2017 ＰＩ．(Twitter:@xm6_original)
 *	Copyright (C) 2001-2017 Ryu Takegami (Twitter:@RyuTakegami)
 *
 *	[ Win32API ウインドウ表示 ]
 *
 *	RHG履歴
 *	  2002.03.11		TrueColorレンダリングモードに対応
 *	  2002.05.31		4096色レンダラのメモリライトを4バイト単位で行うよう変更
 *	  2002.06.05		26万色モード時の32ビットTrueColor表示に対応
 *	  2002.06.20		8色モードのレンダリングを4bppで行うように変更
 *	  2002.06.21		更新が必要な部分だけレンダリング・BLTするように変更
 *						ハードウェアウィンドウ用通知関数を新設
 *	  2002.07.13		BITMAPINFOの確保サイズを修正
 *	  2002.07.20		V2.7βで4096色モードにすると落ちるバグを修正(ばく
 *	  2003.02.11		200ラインモードと400ラインモードのレンダラを統合
 *	  2004.03.17		疑似400ラインアダプタ対応
 *	  2008.01.20		↑なかったことにした
 *	  2010.10.03		2倍拡大表示モードを実装
 *	  2012.07.01		グリーンモニタモードに対応
 *	  2013.02.12		ラスタレンダリングに対応
 +	  2013.07.13		V3での4096色モードレンダリングを32bppに変更
 *	  2013.08.22		上記変更をラスタ単位レンダリング時のみに変更
 */

#ifdef _WIN32

#define STRICT
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <assert.h>
#include <stdlib.h>
#include "xm7.h"
#include "subctrl.h"
#include "display.h"
#include "multipag.h"
#include "ttlpalet.h"
#include "apalet.h"
#include "mouse.h"
#include "w32.h"
#include "w32_draw.h"
#include "w32_gdi.h"
#include "w32_kbd.h"
#include "w32_sub.h"

/*
 *	グローバル ワーク
 */
DWORD rgbTTLGDI[16];						/* デジタルパレット */
DWORD rgbAnalogGDI[4096];					/* アナログパレット */
BYTE *pBitsGDI;								/* ビットデータ */
BYTE GDIDrawFlag[4000];						/* 8x8 再描画領域フラグ */
#if XM7_VER == 1 && defined(L4CARD)
DWORD rgbTTLL4GDI[32];						/* デジタルパレット (L4) */
#endif

/*
 *	スタティック ワーク
 */
#if XM7_VER >= 3
static BYTE nMode;							/* 画面モード */
#elif XM7_VER >= 2
static BOOL bAnalog;						/* アナログモードフラグ */
#elif XM7_VER == 1 && defined(L4CARD)
static BOOL b400Line;						/* 400ラインモードフラグ */
#endif
static BYTE nNowBpp;						/* 現在のビット深度 */
static BOOL bMouseCursor;					/* マウスカーソルフラグ */
static HBITMAP hBitmap;						/* ビットマップ ハンドル */
static WORD nDrawTop;						/* 描画範囲上 */
static WORD nDrawBottom;					/* 描画範囲下 */
static WORD nDrawLeft;						/* 描画範囲左 */
static WORD nDrawRight;						/* 描画範囲右 */
static BOOL bPaletFlag;						/* パレット変更フラグ */
static BOOL bClearFlag;						/* クリアフラグ */
static BOOL bSelectCancelFlag;				/* セレクトキャンセルフラグ */
#if XM7_VER >= 3
static BOOL bWindowOpen;					/* ハードウェアウィンドウ状態 */
static WORD nWindowDx1;						/* ウィンドウ左上X座標 */
static WORD nWindowDy1;						/* ウィンドウ左上Y座標 */
static WORD nWindowDx2;						/* ウィンドウ右下X座標 */
static WORD nWindowDy2;						/* ウィンドウ右下Y座標 */
#endif

/*
 *	アセンブラ関数のためのプロトタイプ宣言
 */
#ifdef __cplusplus
extern "C" {
#endif
#if XM7_VER >= 3
extern void Render640GDI2(int first, int last, int pitch, int scale);
extern void Render640wGDI2(int first, int last, int firstx, int lastx, int pitch, int scale);
extern void Render320GDI(int first, int last);
extern void Render320wGDI(int first, int last, int firstx, int lastx);
extern void Render320GDI32bpp(int first, int last);
extern void Render320wGDI32bpp(int first, int last, int firstx, int lastx);
extern void Render256kGDI(int first, int last, int multipage);
extern void Render640cGDI(int first, int last);
extern void Render640cwGDI(int first, int last, int firstx, int lastx);
#elif XM7_VER == 2
extern void Render640GDI(int first, int last);
extern void Render320GDI(int first, int last);
extern void Render640cGDI(int first, int last);
#else
extern void Render640GDI(int first, int last);
extern void RenderL4GDI(int first, int last);
extern void Render640mGDI(int first, int last);
#endif
#ifdef __cplusplus
}
#endif

/*
 *	プロトタイプ宣言
 */
static void FASTCALL SetDrawFlag(BOOL flag);

/*
 *	初期化
 */
void FASTCALL InitGDI(void)
{
	/* ワークエリア初期化 */
#if XM7_VER >= 3
	nMode = SCR_200LINE;
#elif XM7_VER >= 2
	bAnalog = FALSE;
#elif XM7_VER == 1 && defined(L4CARD)
	b400Line = FALSE;
#endif
	nNowBpp = 4;
	bMouseCursor = TRUE;
	hBitmap = NULL;
	pBitsGDI = NULL;
	memset(rgbTTLGDI, 0, sizeof(rgbTTLGDI));
#if XM7_VER >= 2
	memset(rgbAnalogGDI, 0, sizeof(rgbAnalogGDI));
#elif XM7_VER == 1 && defined(L4CARD)
	memset(rgbTTLL4GDI, 0, sizeof(rgbTTLL4GDI));
#endif

	nDrawTop = 0;
	nDrawBottom = 400;
	nDrawLeft = 0;
	nDrawRight = 640;
	bPaletFlag = FALSE;
	bSelectCancelFlag = FALSE;
	SetDrawFlag(FALSE);

#if XM7_VER >= 3
	bWindowOpen = FALSE;
	nWindowDx1 = 640;
	nWindowDy1 = 400;
	nWindowDx2 = 0;
	nWindowDy2 = 0;
#endif
}

/*
 *	クリーンアップ
 */
void FASTCALL CleanGDI(void)
{
	if (hBitmap) {
		DeleteObject(hBitmap);
		hBitmap = NULL;
		pBitsGDI = NULL;
	}
}

/*
 *	GDIセレクト共通
 */
static BOOL FASTCALL SelectSub(HWND hWnd)
{
	BITMAPINFOHEADER *pbmi;
	HDC hDC;

	ASSERT(hWnd);

	/* DIBセクションが既に存在する場合、破棄する */
	if (hBitmap) {
		DeleteObject(hBitmap);
		hBitmap = NULL;
		pBitsGDI = NULL;
	}

	/* メモリ確保 */
	pbmi = (BITMAPINFOHEADER*)malloc(sizeof(BITMAPINFOHEADER) +
									 sizeof(RGBQUAD) * 256);
	if (!pbmi) {
		return FALSE;
	}
	memset(pbmi, 0, sizeof(BITMAPINFOHEADER) + sizeof(RGBQUAD) * 256);

	/* ビットマップ情報作成 */
	pbmi->biSize = sizeof(BITMAPINFOHEADER);
	pbmi->biWidth = 640;
	pbmi->biHeight = -400;
	pbmi->biPlanes = 1;
	pbmi->biBitCount = nNowBpp;
	pbmi->biCompression = BI_RGB;

	/* DC取得、DIBセクション作成 */
	hDC = GetDC(hWnd);
	hBitmap = CreateDIBSection(hDC, (BITMAPINFO*)pbmi, DIB_RGB_COLORS,
								(void**)&pBitsGDI, NULL, 0);
	ReleaseDC(hWnd, hDC);
	free(pbmi);
	if (!hBitmap) {
		return FALSE;
	}

	/* 全エリアを、一度クリア */
	if (!bRasterRendering) {
		memset(pBitsGDI, 0, (640 * 400 * nNowBpp) >> 3);
		bClearFlag = TRUE;
	}

	return TRUE;
}

/*
 *	全ての再描画フラグを設定
 */
static void FASTCALL SetDrawFlag(BOOL flag)
{
	memset(GDIDrawFlag, (BYTE)flag, sizeof(GDIDrawFlag));
}

/*
 *  640x200、デジタルモード
 *	セレクト
 */
static BOOL FASTCALL Select640(HWND hWnd)
{
	if ((!pBitsGDI) || (nNowBpp != 4)) {
		nNowBpp = 4;
		if (!SelectSub(hWnd)) {
			return FALSE;
		}
	}

	/* 全領域無効 */
	nDrawTop = 0;
	nDrawBottom = 400;
	nDrawLeft = 0;
	nDrawRight = 640;
	bPaletFlag = TRUE;
	SetDrawFlag(TRUE);

#if XM7_VER >= 3
	/* デジタル/200ラインモード */
	nMode = SCR_200LINE;
#elif XM7_VER >= 2
	/* デジタルモード */
	bAnalog = FALSE;
#elif XM7_VER == 1 && defined(L4CARD)
	/* 200ラインモード */
	b400Line = FALSE;
#endif

	return TRUE;
}

#if XM7_VER == 1 && defined(L4CARD)
/*
 *  640x400、単色モード
 *	セレクト
 */
static BOOL FASTCALL SelectL4(HWND hWnd)
{
	if ((!pBitsGDI) || (nNowBpp != 8)) {
		nNowBpp = 8;
		if (!SelectSub(hWnd)) {
			return FALSE;
		}
	}

	/* 全領域無効 */
	nDrawTop = 0;
	nDrawBottom = 400;
	nDrawLeft = 0;
	nDrawRight = 640;
	bPaletFlag = TRUE;
	SetDrawFlag(TRUE);

	/* 400ラインモード */
	b400Line = TRUE;

	return TRUE;
}
#endif

#if XM7_VER >= 3
/*
 *  640x400、デジタルモード
 *	セレクト
 */
static BOOL FASTCALL Select400l(HWND hWnd)
{
	if ((!pBitsGDI) || (nNowBpp != 4)) {
		nNowBpp = 4;
		if (!SelectSub(hWnd)) {
			return FALSE;
		}
	}

	/* 全領域無効 */
	nDrawTop = 0;
	nDrawBottom = 400;
	nDrawLeft = 0;
	nDrawRight = 640;
	bPaletFlag = TRUE;
	SetDrawFlag(TRUE);

	/* デジタル/400ラインモード */
	nMode = SCR_400LINE;

	return TRUE;
}
#endif

#if XM7_VER >= 2
/*
 *  320x200、アナログモード
 *	セレクト
 */
static BOOL FASTCALL Select320(HWND hWnd)
{
#if XM7_VER == 2
	if ((!pBitsGDI) || (nNowBpp != 16)) {
		nNowBpp = 16;
		if (!SelectSub(hWnd)) {
			return FALSE;
		}
	}
#else
	if (bRasterRendering) {
		if ((!pBitsGDI) || (nNowBpp != 32)) {
			nNowBpp = 32;
			if (!SelectSub(hWnd)) {
				return FALSE;
			}
		}
	}
	else {
		if ((!pBitsGDI) || (nNowBpp != 16)) {
			nNowBpp = 16;
			if (!SelectSub(hWnd)) {
				return FALSE;
			}
		}
	}
#endif

	/* 全領域無効 */
	nDrawTop = 0;
	nDrawBottom = 400;
	nDrawLeft = 0;
	nDrawRight = 640;
	bPaletFlag = TRUE;
	SetDrawFlag(TRUE);

#if XM7_VER >= 3
	/* アナログ/200ラインモード */
	nMode = SCR_4096;
#else
	/* アナログモード */
	bAnalog = TRUE;
#endif

	return TRUE;
}
#endif

#if XM7_VER >= 3
/*
 *  320x200、26万色モード
 *	セレクト
 */
static BOOL FASTCALL Select256k(HWND hWnd)
{
	if ((!pBitsGDI) || (nNowBpp != 32)) {
		nNowBpp = 32;
		if (!SelectSub(hWnd)) {
			return FALSE;
		}
	}

	/* 全領域無効 */
	nDrawTop = 0;
	nDrawBottom = 400;
	nDrawLeft = 0;
	nDrawRight = 640;
	bPaletFlag = TRUE;
	SetDrawFlag(TRUE);

	/* アナログ(26万色)/200ラインモード */
	nMode = SCR_262144;

	return TRUE;
}
#endif

/*
 *	セレクトチェック
 */
static BOOL FASTCALL SelectCheck(void)
{
	/* 強制セレクトキャンセル処理 */
	if (bSelectCancelFlag) {
		bSelectCancelFlag = FALSE;
		return FALSE;
	}

#if XM7_VER >= 3
	/* 限りない手抜き(ォ */
	if (nMode == screen_mode) {
		return TRUE;
	}
	else {
		return FALSE;
	}
#elif XM7_VER >= 2
	/* 320x200 */
	if (mode320) {
		if (bAnalog) {
			return TRUE;
		}
		else {
			return FALSE;
		}
	}

	/* 640x200 */
	if (!bAnalog) {
		return TRUE;
	}
	else {
		return FALSE;
	}
#elif defined(L4CARD)
	/* 640x400 */
	if (enable_400line && enable_400linecard) {
		if (b400Line) {
			return TRUE;
		}
		else {
			return FALSE;
		}
	}

	/* 640x200 */
	if (!b400Line) {
		return TRUE;
	}
	else {
		return FALSE;
	}
#else
	return TRUE;
#endif
}

/*
 *	セレクト
 */
BOOL FASTCALL SelectGDI(HWND hWnd)
{
	ASSERT(hWnd);

	/* 未初期化なら */
	if (!pBitsGDI) {
#if XM7_VER >= 3
		switch (screen_mode) {
			case SCR_400LINE	:	return Select400l(hWnd);
			case SCR_262144		:	return Select256k(hWnd);
			case SCR_4096		:	return Select320(hWnd);
			case SCR_200LINE	:	return Select640(hWnd);
		}
#elif XM7_VER >= 2
		if (mode320) {
			return Select320(hWnd);
		}
		return Select640(hWnd);
#elif defined(L4CARD)
		if (enable_400line && enable_400linecard) {
			return SelectL4(hWnd);
		}
		return Select640(hWnd);
#else
		return Select640(hWnd);
#endif
	}

	/* 一致しているかチェック */
	if (SelectCheck()) {
		return TRUE;
	}

	/* セレクト */
#if XM7_VER >= 3
	switch (screen_mode) {
		case SCR_400LINE	:	return Select400l(hWnd);
		case SCR_262144		:	return Select256k(hWnd);
		case SCR_4096		:	return Select320(hWnd);
		default				:	return Select640(hWnd);
	}
#else
#if XM7_VER >= 2
	if (mode320) {
		return Select320(hWnd);
	}
#elif defined(L4CARD)
	if (enable_400line && enable_400linecard) {
		return SelectL4(hWnd);
	}
#endif
	return Select640(hWnd);
#endif
}

/*-[ 描画 ]-----------------------------------------------------------------*/

#if XM7_VER <= 2 && defined(FMTV151)
/*
 *	V2合成 (4bpp)
 */
static void FASTCALL DrawV2_4bpp(void)
{
	BYTE	x, y;
	int		xx, yy;
	WORD	col;
	BYTE	*pbits;

	if (!bFMTV151) {
		return;
	}

	if (!bRasterRendering) {
		if ((nDrawBottom < (V2YPoint * 2)) ||
			(nDrawTop > ((V2YPoint + V2YSize * V2YPxSz) * 2 - 1)) ||
			(nDrawRight < V2XPoint) ||
			(nDrawLeft > (V2XPoint + V2XSize * V2XPxSz - 1))) {
			return;
		}
	}

	pbits = (BYTE *)(pBitsGDI + V2YPoint * 640 + (V2XPoint >> 1));

	for (y = 0; y < V2YSize; y ++) {
		for (x = 0; x < V2XSize; x ++) {
			if (nV2data[y * V2XSize + x]) {
				col = (WORD)(10 - nV2data[y * V2XSize + x]);
				for (yy = y * V2YPxSz; yy < (y + 1) * V2YPxSz; yy ++) {
					for (xx = x * V2XPxSz; xx < (x + 1) * V2XPxSz; xx ++) {
						if (xx & 1) {
							pbits[yy * 640 + (xx >> 1)] &= (BYTE)0xf0;
							pbits[yy * 640 + (xx >> 1)] |= col;
							if (bPseudo400Line) {
								pbits[yy * 640+320+(xx >> 1)] &= (BYTE)0xf0;
								pbits[yy * 640+320+(xx >> 1)] |= col;
							}
						}
						else {
							pbits[yy * 640 + (xx >> 1)] &= (BYTE)0x0f;
							pbits[yy * 640 + (xx >> 1)] |= (BYTE)(col << 4);
							if (bPseudo400Line) {
								pbits[yy*640+320+(xx>>1)] &= (BYTE)0x0f;
								pbits[yy*640+320+(xx>>1)] |= (BYTE)(col << 4);
							}
						}
					}
				}
			}
		}
	}
}

/*
 *	V2合成 (16bpp)
 */
static void FASTCALL DrawV2_16bpp(void)
{
	BYTE	x, y;
	int		xx, yy;
	WORD	col;
	WORD	*pbits;

	/* パレットテーブル */
	static const WORD V2rgbTable[] = {
		0x0000 | 0x0000 | 0x0000,
		0x0000 | 0x03e0 | 0x0000,
	};

	if (!bFMTV151) {
		return;
	}

	if (!bRasterRendering) {
		if ((nDrawBottom < (V2YPoint * 2)) ||
			(nDrawTop > ((V2YPoint + V2YSize * V2YPxSz) * 2 - 1)) ||
			(nDrawRight < V2XPoint) ||
			(nDrawLeft > (V2XPoint + V2XSize * V2XPxSz - 1))) {
			return;
		}
	}

	pbits = (WORD *)(pBitsGDI + V2YPoint * 2560 + V2XPoint * 2);

	for (y = 0; y < V2YSize; y ++) {
		for (x = 0; x < V2XSize; x ++) {
			if (nV2data[y * V2XSize + x]) {
				col = V2rgbTable[nV2data[y * V2XSize + x]];
				for (yy = y * V2YPxSz; yy < (y + 1) * V2YPxSz; yy ++) {
					for (xx = x * V2XPxSz; xx < (x + 1) * V2XPxSz; xx ++) {
						pbits[yy * 1280 + xx] = col;
					}
				}
			}
		}
	}
}
#endif

/*
 *	オールクリア
 */
static void FASTCALL AllClear(void)
{
	/* すべてクリア */
	memset(pBitsGDI, 0, (640 * 400 * nNowBpp) >> 3);

	/* 全領域をレンダリング対象とする */
	nDrawTop = 0;
	nDrawBottom = 400;
	nDrawLeft = 0;
	nDrawRight = 640;
	SetDrawFlag(TRUE);

	bClearFlag = FALSE;
}

/*
 *	フルスキャン
 */
static void FASTCALL RenderFullScan(void)
{
	BYTE *p;
	BYTE *q;
	WORD u;
	WORD top, bottom;

	/* ラスタ単位レンダリング時に画面全体を処理する必要があるので */
	if (nDrawTop >= nDrawBottom) {
		top = 0;
		bottom = 400;
	}
	else {
		top = nDrawTop;
		bottom = nDrawBottom;
	}

	/* ポインタ初期化 */
	p = &pBitsGDI[(top * 640 * nNowBpp) >> 3];
	q = p + ((640 * nNowBpp) >> 3);

	/* ループ */
	for (u=top; u<bottom; u += (WORD)2) {
		memcpy(q, p, (640 * nNowBpp) >> 3);
		p += ((640 * 2 * nNowBpp) >> 3);
		q += ((640 * 2 * nNowBpp) >> 3);
	}
}

/*
 *	奇数ライン設定 (デジタルモード)
 */
static void FASTCALL RenderSetOddLineDigital(void)
{
	BYTE *p;
	WORD u;
	WORD top, bottom;

	/* ラスタ単位レンダリング時に画面全体を処理する必要があるので */
	if (nDrawTop >= nDrawBottom) {
		top = 0;
		bottom = 400;
	}
	else {
		top = nDrawTop;
		bottom = nDrawBottom;
	}

	/* ポインタ初期化 */
	p = &pBitsGDI[((top + 1) * 640 * nNowBpp) >> 3];

	/* ループ */
	for (u=top; u<bottom; u += (WORD)2) {
		memset(p, 0x88, (640 * nNowBpp) >> 3);
		p += ((640 * 2 * nNowBpp) >> 3);
	}
}

/*
 *	奇数ライン設定 (アナログモード)
 */
#if XM7_VER >= 2
static void FASTCALL RenderSetOddLineAnalog(void)
{
	BYTE *p;
	WORD u;
	WORD top, bottom;

	/* ラスタ単位レンダリング時に画面全体を処理する必要があるので */
	if (nDrawTop >= nDrawBottom) {
		top = 0;
		bottom = 400;
	}
	else {
		top = nDrawTop;
		bottom = nDrawBottom;
	}

	/* ポインタ初期化 */
	p = &pBitsGDI[((top + 1) * 640 * nNowBpp) >> 3];

	/* ループ */
	for (u=top; u<bottom; u += (WORD)2) {
		memset(p, 0x00, (640 * nNowBpp) >> 3);
		p += ((640 * 2 * nNowBpp) >> 3);
	}
}
#endif

/*
 *	640x200/400、デジタルモード
 *	パレット設定
 */
static void FASTCALL Palet640(void)
{
	int i;
	int vpage;
	BYTE col;

	/* パレットテーブル */
	static DWORD rgbTable[] = {
		0x00000000,
		0x000000ff,
		0x00ff0000,
		0x00ff00ff,
		0x0000ff00,
		0x0000ffff,
		0x00ffff00,
		0x00ffffff,
#if XM7_VER == 1
		0x00000000,
		0x00001c00,
		0x00004c00,
		0x00006800,
		0x00009600,
		0x0000b200,
		0x0000e200,
		0x0000ff00,
#endif
	};

	/* マルチページより、表示プレーン情報を得る */
	vpage = (~(multi_page >> 4)) & 0x07;

	/* 640x200/400、デジタルパレ	ット */
	for (i=0; i<8; i++) {
		if (crt_flag) {
			/* CRT ON */
#if XM7_VER == 1
			if (fm_subtype == FMSUB_FM8) {
				col = (BYTE)i;
			}
			else {
				col = (BYTE)(ttl_palet[i & vpage] & 0x07);
			}
#else
			col = (BYTE)(ttl_palet[i & vpage] & 0x07);
#endif

#if XM7_VER == 1
			if (bPseudo400Line) {
				if (bGreenMonitor) {
					if (col & 2) {
						rgbTTLGDI[i + 8] = rgbTable[15];
					}
					else {
						rgbTTLGDI[i + 8] = rgbTable[8];
					}
					if (col & 4) {
						rgbTTLGDI[i] = rgbTable[15];
					}
					else {
						rgbTTLGDI[i] = rgbTable[8];
					}
				}
				else {
					if (col & 2) {
						rgbTTLGDI[i + 8] = rgbTable[7];
					}
					else {
						rgbTTLGDI[i + 8] = rgbTable[0];
					}
					if (col & 4) {
						rgbTTLGDI[i] = rgbTable[7];
					}
					else {
						rgbTTLGDI[i] = rgbTable[0];
					}
				}
			}
			else {
				if (bGreenMonitor) {
					rgbTTLGDI[i] = rgbTable[col + 8];
				}
				else {
					rgbTTLGDI[i] = rgbTable[col];
				}
			}
#else
			rgbTTLGDI[i] = rgbTable[col];
#endif
		}
		else {
			/* CRT OFF */
			rgbTTLGDI[i] = rgbTable[0];
		}
	}

	/* FMTV-151 V2チャンネルコール用 */
#if XM7_VER == 2 && defined(FMTV151)
	rgbTTLGDI[8] = rgbTable[0];
	rgbTTLGDI[9] = rgbTable[4];
#endif
}

#if XM7_VER == 1 && defined(L4CARD)
/*
 *	640x400、単色モード
 *	パレット設定
 */
static void FASTCALL PaletL4(void)
{
	int i;

	/* パレットテーブル(16色) */
	static const DWORD rgbTable16[] = {
		0x00000000,
		0x000000bb,
		0x00bb0000,
		0x00bb00bb,
		0x0000bb00,
		0x0000bbbb,
		0x00bbbb00,
		0x00bbbbbb,
		0x00444444,
		0x004444ff,
		0x00ff4444,
		0x00ff44ff,
		0x0044ff44,
		0x0044ffff,
		0x00ffff44,
		0x00ffffff,
		0x00000000,
		0x00003300,
		0x00006500,
		0x00007100,
		0x0000a900,
		0x0000af00,
		0x0000bd00,
		0x0000c200,
		0x00004a00,
		0x00006700,
		0x00009600,
		0x0000a200,
		0x0000e200,
		0x0000e900,
		0x0000f900,
		0x0000ff00,
	};

	/* 640x400、テキストパレット */
	for (i=0; i<16; i++) {
		if (crt_flag) {
			/* CRT ON */
			if (bGreenMonitor) {
				rgbTTLL4GDI[i + 16] = rgbTable16[i + 16];
			}
			else {
				rgbTTLL4GDI[i + 16] = rgbTable16[i];
			}
		}
		else {
			/* CRT OFF */
			rgbTTLL4GDI[i + 16] = rgbTable16[0];
		}
	}

	/* 640x400、グラフィックパレット */
	for (i=0; i<3; i++) {
		if (crt_flag) {
			if (bGreenMonitor) {
				if ((i == 0) || (multi_page & (0x08 << i))) {
					rgbTTLL4GDI[i] = rgbTable16[(ttl_palet[0] & 0x0f) + 16];
				}
				else {
					rgbTTLL4GDI[i] = rgbTable16[(ttl_palet[1] & 0x0f) + 16];
				}
			}
			else {
				if ((i == 0) || (multi_page & (0x08 << i))) {
					rgbTTLL4GDI[i] = rgbTable16[ttl_palet[0] & 0x0f];
				}
				else {
					rgbTTLL4GDI[i] = rgbTable16[ttl_palet[1] & 0x0f];
				}
			}
		}
		else {
			rgbTTLL4GDI[i] = rgbTable16[0];
		}
	}
}
#endif

/*
 *	640x200、デジタルモード
 *	描画
 */
static void FASTCALL Draw640(HWND hWnd, HDC hDC)
{
	HDC hMemDC;
	HBITMAP hDefBitmap;
#if XM7_VER >= 3
	WORD wdtop, wdbtm;
#endif

	ASSERT(hWnd);
	ASSERT(hDC);

	/* パレット設定 */
	if (bPaletFlag) {
		Palet640();
	}

	/* クリア処理 */
	if (bClearFlag) {
		AllClear();
	}

	/* レンダリング */
	if ((nDrawTop < nDrawBottom) && (nDrawLeft < nDrawRight)) {
#if XM7_VER >= 3
		/* ウィンドウオープン時 */
		if (window_open) {
			/* ウィンドウ外 上側の描画 */
			if ((nDrawTop >> 1) < window_dy1) {
				if (bPseudo400Line) {
					Render640cGDI(nDrawTop >> 1, window_dy1);
				}
				else {
					Render640GDI2(nDrawTop >> 1, window_dy1, 320, 4);
				}
			}

			/* ウィンドウ内の描画 */
			if ((nDrawTop >> 1) > window_dy1) {
				wdtop = (WORD)(nDrawTop >> 1);
			}
			else {
				wdtop = window_dy1;
			}

			if ((nDrawBottom >> 1) < window_dy2) {
				wdbtm = (WORD)(nDrawBottom >> 1);
			}
			else {
				wdbtm = window_dy2;
			}

			if (wdbtm > wdtop) {
				if (bPseudo400Line) {
					Render640cwGDI(wdtop, wdbtm, window_dx1, window_dx2);
				}
				else {
					Render640wGDI2(wdtop, wdbtm, window_dx1, window_dx2,320,4);
				}
			}

			/* ウィンドウ外 下側の描画 */
			if ((nDrawBottom >> 1) > window_dy2) {
				if (bPseudo400Line) {
					Render640cGDI(window_dy2, nDrawBottom >> 1);
				}
				else {
					Render640GDI2(window_dy2, nDrawBottom >> 1, 320, 4);
				}
			}
		}
		else {
			if (bPseudo400Line) {
				Render640cGDI(nDrawTop >> 1, nDrawBottom >> 1);
			}
			else {
				Render640GDI2(nDrawTop >> 1, nDrawBottom >> 1, 320, 4);
			}
		}
#elif XM7_VER == 2
		if (bPseudo400Line) {
			Render640cGDI(nDrawTop >> 1, nDrawBottom >> 1);
		}
		else {
			Render640GDI(nDrawTop >> 1, nDrawBottom >> 1);
		}
#if defined(FMTV151)
		DrawV2_4bpp();
#endif	/* FMTV-151 */
#else
		if (bPseudo400Line) {
			Render640mGDI(nDrawTop >> 1, nDrawBottom >> 1);
		}
		else {
			Render640GDI(nDrawTop >> 1, nDrawBottom >> 1);
		}
#endif	/* XM7_VER >= 3 */

		if (!bPseudo400Line) {
			if (bFullScan) {
				RenderFullScan();
			}
			else {
				RenderSetOddLineDigital();
			}
		}
	}
	else {
		/* パレットが変更されていない場合は何もしない */
		if (!bPaletFlag) {
			return;
		}
	}

	/* パレットが変更されている場合は全領域を再描画 */
	if (bPaletFlag) {
		nDrawTop = 0;
		nDrawBottom = 400;
		nDrawLeft = 0;
		nDrawRight = 640;
		SetDrawFlag(TRUE);
	}

	/* メモリDC作成、オブジェクト選択 */
	hMemDC = CreateCompatibleDC(hDC);
	ASSERT(hMemDC);
	hDefBitmap = (HBITMAP)SelectObject(hMemDC, hBitmap);

	/* デスクトップの状況いかんでは、拒否もあり得る様 */
	if (hDefBitmap) {
		SetDIBColorTable(hMemDC, 0, 16, (RGBQUAD *)rgbTTLGDI);
		if (bDoubleSize) {
			StretchBlt(hDC, nDrawLeft * 2, nDrawTop * 2,
				(nDrawRight - nDrawLeft) * 2, (nDrawBottom - nDrawTop) * 2,
				hMemDC, nDrawLeft, nDrawTop,
				(nDrawRight - nDrawLeft), (nDrawBottom - nDrawTop), SRCCOPY);
		}
		else {
			BitBlt(hDC, nDrawLeft, nDrawTop,
				(nDrawRight - nDrawLeft), (nDrawBottom - nDrawTop),
				hMemDC, nDrawLeft, nDrawTop, SRCCOPY);
		}
		SelectObject(hMemDC, hDefBitmap);

		/* 次回に備え、ワークリセット */
		nDrawTop = 400;
		nDrawBottom = 0;
		nDrawLeft = 640;
		nDrawRight = 0;
		bPaletFlag = FALSE;
		SetDrawFlag(FALSE);
	}
	else {
		/* 再描画を起こす */
		InvalidateRect(hWnd, NULL, FALSE);
	}

	/* メモリDC削除 */
	DeleteDC(hMemDC);
}

#if XM7_VER >= 3
/*
 *	640x400、デジタルモード
 *	描画
 */
static void FASTCALL Draw400l(HWND hWnd, HDC hDC)
{
	HDC hMemDC;
	HBITMAP hDefBitmap;
	WORD wdtop, wdbtm;

	ASSERT(hWnd);
	ASSERT(hDC);

	/* パレット設定 */
	if (bPaletFlag) {
		Palet640();
	}

	/* クリア処理 */
	if (bClearFlag) {
		AllClear();
	}

	/* レンダリング */
	if ((nDrawTop < nDrawBottom) && (nDrawLeft < nDrawRight)) {
		/* ウィンドウオープン時 */
		if (window_open) {
			/* ウィンドウ外 上側の描画 */
			if (nDrawTop < window_dy1) {
				Render640GDI2(nDrawTop, window_dy1, 0, 2);
			}

			/* ウィンドウ内の描画 */
			if (nDrawTop > window_dy1) {
				wdtop = nDrawTop;
			}
			else {
				wdtop = window_dy1;
			}

			if (nDrawBottom < window_dy2) {
				wdbtm = nDrawBottom;
			}
			else {
				wdbtm = window_dy2;
			}

			if (wdbtm > wdtop) {
				Render640wGDI2(wdtop, wdbtm, window_dx1, window_dx2, 0, 2);
			}

			/* ウィンドウ外 下側の描画 */
			if (nDrawBottom > window_dy2) {
				Render640GDI2(window_dy2, nDrawBottom, 0, 2);
			}
		}
		else {
			Render640GDI2(nDrawTop, nDrawBottom, 0, 2);
		}
	}
	else {
		/* パレットが変更されていない場合は何もしない */
		if (!bPaletFlag) {
			return;
		}
	}

	/* パレットが変更されている場合は全領域を再描画 */
	if (bPaletFlag) {
		nDrawTop = 0;
		nDrawBottom = 400;
		nDrawLeft = 0;
		nDrawRight = 640;
		SetDrawFlag(TRUE);
	}

	/* メモリDC作成、オブジェクト選択 */
	hMemDC = CreateCompatibleDC(hDC);
	ASSERT(hMemDC);
	hDefBitmap = (HBITMAP)SelectObject(hMemDC, hBitmap);

	/* デスクトップの状況いかんでは、拒否もあり得る様 */
	if (hDefBitmap) {
		SetDIBColorTable(hMemDC, 0, 16, (RGBQUAD *)rgbTTLGDI);
		if (bDoubleSize) {
			StretchBlt(hDC, nDrawLeft * 2, nDrawTop * 2,
				(nDrawRight - nDrawLeft) * 2, (nDrawBottom - nDrawTop) * 2,
				hMemDC, nDrawLeft, nDrawTop,
				(nDrawRight - nDrawLeft), (nDrawBottom - nDrawTop), SRCCOPY);
		}
		else {
			BitBlt(hDC, nDrawLeft, nDrawTop,
				(nDrawRight - nDrawLeft), (nDrawBottom - nDrawTop),
				hMemDC, nDrawLeft, nDrawTop, SRCCOPY);
		}
		SelectObject(hMemDC, hDefBitmap);

		/* 次回に備え、ワークリセット */
		nDrawTop = 400;
		nDrawBottom = 0;
		nDrawLeft = 640;
		nDrawRight = 0;
		bPaletFlag = FALSE;
		SetDrawFlag(FALSE);
	}
	else {
		/* 再描画を起こす */
		InvalidateRect(hWnd, NULL, FALSE);
	}

	/* メモリDC削除 */
	DeleteDC(hMemDC);
}
#endif

#if XM7_VER == 1 && defined(L4CARD)
/*
 *	テキスト画面
 *	レンダリング
 */
static void FASTCALL RenderTextGDI(int first, int last, int left, int right)
{
	DWORD	tmp;
	int		x, y, x2;
	WORD	addr;
	BYTE	cursor_start, cursor_end, cursor_type;
	BYTE	raster, lines, maxx;
	BYTE	chr, atr, chr_dat;
	BYTE	line, line_old;
	BYTE	col;
	BYTE	*p;
	int		xsize;

	/* 表示OFFの場合、何もしない */
	if (!crt_flag) {
		return;
	}

	/* cursor */
	cursor_start	= (BYTE)(crtc_register[10] & 0x1f);
	cursor_end		= (BYTE)(crtc_register[11] & 0x1f);
	cursor_type		= (BYTE)((crtc_register[10] & 0x60) >> 5);

	/* 画面表示状態 */
	maxx			= (BYTE)(crtc_register[1] << 1);
	lines			= (BYTE)((crtc_register[9] & 0x1f) + 1);

	/* 1文字あたりのXサイズ */
	if (width40_flag) {
		xsize = 16;
	}
	else {
		xsize = 8;
	}

	for (x = (left / xsize); x < (right / xsize); x++) {
		line_old = 255;
		for (y = first; y < last; y++) {
			p = (BYTE *)pBitsGDI + y * 640;
			raster = (BYTE)(y % lines);
			line = (BYTE)(y / lines);
			if (line != line_old) {
				addr = (WORD)((text_start_addr + (line * maxx + x) * 2) & 0x0ffe);
				chr = tvram_c[addr + 0];
				atr = tvram_c[addr + 1];

				/* アトリビュート下位3ビットから描画色を設定 */
				col = (BYTE)((atr & 0x07) | 0x10);

				/* インテンシティアトリビュート処理 */
				if (atr & 0x20) {
					col |= (BYTE)8;
				}
			}

			/* フォントデータ取得(ブリンクアトリビュート処理を含む) */
			if ((!(atr & 0x10) || text_blink) && (raster < 16)) {
				chr_dat = subcg_l4[(WORD)(chr * 16) + raster];
			}
			else {
				chr_dat = 0x00;
			}

			/* リバースアトリビュート処理 */
			if (atr & 0x08) {
				chr_dat ^= (BYTE)0xFF;
			}

			/* カーソル処理 */
			if ((addr == cursor_addr) && (cursor_type != 1) &&
				(cursor_blink || (cursor_type == 0))) {
				if ((raster >= cursor_start) && (raster <= cursor_end)) {
					chr_dat ^= (BYTE)0xFF;
				}
			}

			/* 描画処理 */
			if (chr_dat) {
				for (x2 = 0; x2 < 8; x2++) {
					tmp = (DWORD)((x << 3) + x2);
					if (chr_dat & (1 << (7 ^ x2))) {
						if (width40_flag) {
							p[tmp * 2 + 0] = col;
							p[tmp * 2 + 1] = col;
						}
						else {
							p[tmp] = col;
						}
					}
				}
			}

			/* 今回描画した行位置を保存 */
			line_old = line;
		}
	}
}

/*
 *	640x400、単色モード
 *	描画
 */
static void FASTCALL DrawL4(HWND hWnd, HDC hDC)
{
	HDC hMemDC;
	HBITMAP hDefBitmap;

	ASSERT(hWnd);
	ASSERT(hDC);

	/* パレット設定 */
	if (bPaletFlag) {
		PaletL4();
	}

	/* クリア処理 */
	if (bClearFlag) {
		AllClear();
	}

	/* レンダリング */
	if ((nDrawTop < nDrawBottom) && (nDrawLeft < nDrawRight)) {
		RenderL4GDI(nDrawTop, nDrawBottom);
		RenderTextGDI(nDrawTop, nDrawBottom, nDrawLeft, nDrawRight);
	}
	else {
		/* パレットが変更されていない場合は何もしない */
		if (!bPaletFlag) {
			return;
		}
	}

	/* パレットが変更されている場合は全領域を再描画 */
	if (bPaletFlag) {
		nDrawTop = 0;
		nDrawBottom = 400;
		nDrawLeft = 0;
		nDrawRight = 640;
		SetDrawFlag(TRUE);
	}

	/* メモリDC作成、オブジェクト選択 */
	hMemDC = CreateCompatibleDC(hDC);
	ASSERT(hMemDC);
	hDefBitmap = (HBITMAP)SelectObject(hMemDC, hBitmap);

	/* デスクトップの状況いかんでは、拒否もあり得る様 */
	if (hDefBitmap) {
		SetDIBColorTable(hMemDC, 0, 32, (RGBQUAD *)rgbTTLL4GDI);
		if (bDoubleSize) {
			StretchBlt(hDC, nDrawLeft * 2, nDrawTop * 2,
				(nDrawRight - nDrawLeft) * 2, (nDrawBottom - nDrawTop) * 2,
				hMemDC, nDrawLeft, nDrawTop,
				(nDrawRight - nDrawLeft), (nDrawBottom - nDrawTop), SRCCOPY);
		}
		else {
			BitBlt(hDC, nDrawLeft, nDrawTop,
				(nDrawRight - nDrawLeft), (nDrawBottom - nDrawTop),
				hMemDC, nDrawLeft, nDrawTop, SRCCOPY);
		}
		SelectObject(hMemDC, hDefBitmap);

		/* 次回に備え、ワークリセット */
		nDrawTop = 400;
		nDrawBottom = 0;
		nDrawLeft = 640;
		nDrawRight = 0;
		bPaletFlag = FALSE;
		SetDrawFlag(FALSE);
	}
	else {
		/* 再描画を起こす */
		InvalidateRect(hWnd, NULL, FALSE);
	}

	/* メモリDC削除 */
	DeleteDC(hMemDC);
}
#endif

#if XM7_VER >= 2
/*
 *	320x200、アナログモード
 *	パレット設定
 */
static void FASTCALL Palet320(void)
{
	int i, j;
	DWORD color;
	DWORD r, g, b;
	int amask;

	/* アナログマスクを作成 */
	amask = 0;
	if (!(multi_page & 0x10)) {
		amask |= 0x000f;
	}
	if (!(multi_page & 0x20)) {
		amask |= 0x00f0;
	}
	if (!(multi_page & 0x40)) {
		amask |= 0x0f00;
	}

	for (i=0; i<4096; i++) {
		/* 最下位から5bitづつB,G,R */
		color = 0;
#if XM7_VER == 2
		/* TTL RGB合成 */
		if (crt_flag && bTTLMonitor) {
			r = 0;
			g = 0;
			b = 0;
			j = 0;
			/* R */
			if ((i & amask) & 0x0080) {
				j |= 2;
			}

			/* G */
			if ((i & amask) & 0x0800) {
				j |= 4;
			}

			/* B */
			if ((i & amask) & 0x0008) {
				j |= 1;
			}

			/* RGB合成 */
			if (ttl_palet[j & 7] & 0x01) {
				b = 0x0f;
			}
			if (ttl_palet[j & 7] & 0x02) {
				r = 0x0f;
			}
			if (ttl_palet[j & 7] & 0x04) {
				g = 0x0f;
			}
		}
		else
#endif
		if (crt_flag) {
			j = i & amask;
			r = apalet_r[j];
			g = apalet_g[j];
			b = apalet_b[j];
		}
		else {
			r = 0;
			g = 0;
			b = 0;
		}

#if XM7_VER >= 3
		if (bRasterRendering) {
			/* R */
			color |= (WORD)r * 0x11;
			color <<= 8;

			/* G */
			color |= (WORD)g * 0x11;
			color <<= 8;

			/* B */
			color |= (WORD)b * 0x11;

			/* セット */
			rgbAnalogGDI[i] = color;

			continue;
		}
#endif

		/* R */
		r <<= 1;
		if (r > 0) {
			r |= 0x01;
		}
		color |= (WORD)r;
		color <<= 5;

		/* G */
		g <<= 1;
		if (g > 0) {
			g |= 0x01;
		}
		color |= (WORD)g;
		color <<= 5;

		/* B */
		b <<= 1;
		if (b > 0) {
			b |= 0x01;
		}
		color |= (WORD)b;

		/* セット */
		rgbAnalogGDI[i] = (DWORD)((color << 16) | color);
	}
}

/*
 *	320x200、アナログモード
 *	描画
 */
static void FASTCALL Draw320(HWND hWnd, HDC hDC)
{
	HDC hMemDC;
	HBITMAP hDefBitmap;
#if XM7_VER >= 3
	WORD wdtop, wdbtm;
#endif

	ASSERT(hWnd);
	ASSERT(hDC);

	/* パレット設定 */
	if (bPaletFlag) {
		Palet320();
	}

	/* クリア処理 */
	if (bClearFlag) {
		AllClear();
	}

	/* レンダリング */
	if (nDrawTop >= nDrawBottom) {
		return;
	}

#if XM7_VER >= 3
	/* ウィンドウオープン時 */
	if (window_open) {
		/* ウィンドウ外 上側の描画 */
		if ((nDrawTop >> 1) < window_dy1) {
			Render320GDI(nDrawTop >> 1, window_dy1);
		}

		/* ウィンドウ内の描画 */
		if ((nDrawTop >> 1) > window_dy1) {
			wdtop = (WORD)(nDrawTop >> 1);
		}
		else {
			wdtop = window_dy1;
		}

		if ((nDrawBottom >> 1) < window_dy2) {
			wdbtm = (WORD)(nDrawBottom >> 1);
		}
		else {
			wdbtm = window_dy2;
		}

		if (wdbtm > wdtop) {
			Render320wGDI(wdtop, wdbtm, window_dx1, window_dx2);
		}

		/* ウィンドウ外 下側の描画 */
		if ((nDrawBottom >> 1) > window_dy2) {
			Render320GDI(window_dy2, nDrawBottom >> 1);
		}
	}
	else {
		Render320GDI(nDrawTop >> 1, nDrawBottom >> 1);
	}
#else
	Render320GDI(nDrawTop >> 1, nDrawBottom >> 1);
#if defined(FMTV151)
	DrawV2_16bpp();
#endif
#endif

	if (bFullScan) {
		RenderFullScan();
	}

	/* メモリDC作成、オブジェクト選択 */
	hMemDC = CreateCompatibleDC(hDC);
	ASSERT(hMemDC);
	hDefBitmap = (HBITMAP)SelectObject(hMemDC, hBitmap);

	/* デスクトップの状況いかんでは、拒否もあり得る様 */
	if (hDefBitmap) {
		if (bDoubleSize) {
			StretchBlt(hDC, nDrawLeft * 2, nDrawTop * 2,
				(nDrawRight - nDrawLeft) * 2, (nDrawBottom - nDrawTop) * 2,
				hMemDC, nDrawLeft, nDrawTop,
				(nDrawRight - nDrawLeft), (nDrawBottom - nDrawTop), SRCCOPY);
		}
		else {
			BitBlt(hDC, nDrawLeft, nDrawTop,
				(nDrawRight - nDrawLeft), (nDrawBottom - nDrawTop),
				hMemDC, nDrawLeft, nDrawTop, SRCCOPY);
		}
		SelectObject(hMemDC, hDefBitmap);

		/* 次回に備え、ワークリセット */
		nDrawTop = 400;
		nDrawBottom = 0;
		nDrawLeft = 640;
		nDrawRight = 0;
		bPaletFlag = FALSE;
		SetDrawFlag(FALSE);
	}
	else {
		/* 再描画を起こす */
		InvalidateRect(hWnd, NULL, FALSE);
	}

	/* メモリDC解放 */
	DeleteDC(hMemDC);
}
#endif

#if XM7_VER >= 3
/*
 *	320x200、26万色モード
 *	描画
 */
static void FASTCALL Draw256k(HWND hWnd, HDC hDC)
{
	HDC hMemDC;
	HBITMAP hDefBitmap;

	ASSERT(hWnd);
	ASSERT(hDC);

	/* クリア処理 */
	if (bClearFlag) {
		AllClear();
	}

	/* レンダリング */
	if (nDrawTop >= nDrawBottom) {
		return;
	}
	if (crt_flag) {
		Render256kGDI(nDrawTop >> 1, nDrawBottom >> 1, multi_page);
	}
	else {
		Render256kGDI(nDrawTop >> 1, nDrawBottom >> 1, 0xff);
	}
	if (bFullScan) {
		RenderFullScan();
	}

	/* メモリDC作成、オブジェクト選択 */
	hMemDC = CreateCompatibleDC(hDC);
	ASSERT(hMemDC);
	hDefBitmap = (HBITMAP)SelectObject(hMemDC, hBitmap);

	/* デスクトップの状況いかんでは、拒否もあり得る様 */
	if (hDefBitmap) {
		if (bDoubleSize) {
			StretchBlt(hDC, nDrawLeft * 2, nDrawTop * 2,
				(nDrawRight - nDrawLeft) * 2, (nDrawBottom - nDrawTop) * 2,
				hMemDC, nDrawLeft, nDrawTop,
				(nDrawRight - nDrawLeft), (nDrawBottom - nDrawTop), SRCCOPY);
		}
		else {
			BitBlt(hDC, nDrawLeft, nDrawTop,
				(nDrawRight - nDrawLeft), (nDrawBottom - nDrawTop),
				hMemDC, nDrawLeft, nDrawTop, SRCCOPY);
		}
		SelectObject(hMemDC, hDefBitmap);

		/* 次回に備え、ワークリセット */
		nDrawTop = 400;
		nDrawBottom = 0;
		nDrawLeft = 640;
		nDrawRight = 0;
		bPaletFlag = FALSE;
		SetDrawFlag(FALSE);
	}
	else {
		/* 再描画を起こす */
		InvalidateRect(hWnd, NULL, FALSE);
	}

	/* メモリDC解放 */
	DeleteDC(hMemDC);
}
#endif

/*
 *	描画
 */
void FASTCALL DrawGDI(HWND hWnd, HDC hDC)
{
	ASSERT(hWnd);
	ASSERT(hDC);

	/* 640-320 自動切り替え */
	SelectGDI(hWnd);

#if XM7_VER >= 3
	/* いずれかを使って描画 */
	switch (nMode) {
		case SCR_400LINE	:	Draw400l(hWnd, hDC);
								return;
		case SCR_262144		:	Draw256k(hWnd, hDC);
								return;
		case SCR_4096		:	Draw320(hWnd, hDC);
								return;
		case SCR_200LINE	:	Draw640(hWnd, hDC);
								return;
	}
#elif XM7_VER >= 2
	/* どちらかを使って描画 */
	if (bAnalog) {
		Draw320(hWnd, hDC);
	}
	else {
		Draw640(hWnd, hDC);
	}
#elif defined(L4CARD)
	/* どちらかを使って描画 */
	if (b400Line) {
		DrawL4(hWnd, hDC);
	}
	else {
		Draw640(hWnd, hDC);
	}
#else
	Draw640(hWnd, hDC);
#endif
}

/*
 *	640x200、デジタルモード
 *	ラスタ描画
 */
static void FASTCALL DrawRaster640(int nRaster)
{
	/* 表示範囲外の場合は何もしない */
	if (nRaster >= 200) {
		return;
	}

	/* レンダリング */
#if XM7_VER >= 3
	/* ウィンドウオープン時 */
	if (window_open) {
		/* ウィンドウ外の描画 */
		if ((nRaster < window_dy1) || (nRaster > window_dy2)) {
			if (bPseudo400Line) {
				Render640cGDI(nRaster, nRaster + 1);
			}
			else {
				Render640GDI2(nRaster, nRaster + 1, 320, 4);
			}
		}
		else {
			if (bPseudo400Line) {
				Render640cwGDI(nRaster, nRaster + 1, window_dx1, window_dx2);
			}
			else {
				Render640wGDI2(nRaster,nRaster+1,window_dx1,window_dx2,320,4);
			}
		}
	}
	else {
		if (bPseudo400Line) {
			Render640cGDI(nRaster, nRaster + 1);
		}
		else {
			Render640GDI2(nRaster, nRaster + 1, 320, 4);
		}
	}
#elif XM7_VER == 2
	if (bPseudo400Line) {
		Render640cGDI(nRaster, nRaster + 1);
	}
	else {
		Render640GDI(nRaster, nRaster + 1);
	}
#else
	if (bPseudo400Line) {
		Render640mGDI(nRaster, nRaster + 1);
	}
	else {
		Render640GDI(nRaster, nRaster + 1);
	}
#endif	/* XM7_VER >= 3 */
}

#if XM7_VER >= 3
/*
 *	640x400、デジタルモード
 *	ラスタ描画
 */
static void FASTCALL DrawRaster400l(int nRaster)
{
	/* 表示範囲外の場合は何もしない */
	if (nRaster >= 400) {
		return;
	}

	/* レンダリング */
	/* ウィンドウオープン時 */
	if (window_open) {
		/* ウィンドウ外の描画 */
		if ((nRaster < window_dy1) || (nRaster > window_dy2)) {
			Render640GDI2(nRaster, nRaster + 1, 0, 2);
		}
		else {
			Render640wGDI2(nRaster, nRaster + 1, window_dx1, window_dx2, 0, 2);
		}
	}
	else {
		Render640GDI2(nRaster, nRaster + 1, 0, 2);
	}
}
#endif

#if XM7_VER == 1 && defined(L4CARD)
/*
 *	テキスト画面
 *	ラスタ描画
 */
static void FASTCALL RenderTextRasterGDI(int nRaster, int left, int right)
{
	DWORD	tmp;
	int		x, x2;
	WORD	addr;
	BYTE	cursor_start, cursor_end, cursor_type;
	BYTE	raster, lines, maxx;
	BYTE	chr, atr, chr_dat;
	BYTE	line;
	BYTE	col;
	BYTE	*p;
	int		xsize;

	/* 表示OFFの場合、何もしない */
	if (!crt_flag) {
		return;
	}

	/* cursor */
	cursor_start	= (BYTE)(crtc_register[10] & 0x1f);
	cursor_end		= (BYTE)(crtc_register[11] & 0x1f);
	cursor_type		= (BYTE)((crtc_register[10] & 0x60) >> 5);

	/* 画面表示状態 */
	maxx			= (BYTE)(crtc_register[1] << 1);
	lines			= (BYTE)((crtc_register[9] & 0x1f) + 1);

	/* 1文字あたりのXサイズ */
	if (width40_flag) {
		xsize = 16;
	}
	else {
		xsize = 8;
	}

	for (x = (left / xsize); x < (right / xsize); x++) {
		p = (BYTE *)pBitsGDI + nRaster * 640;
		raster = (BYTE)(nRaster % lines);
		line = (BYTE)(nRaster / lines);
		addr = (WORD)((text_start_addr + (line * maxx + x) * 2) & 0x0ffe);
		chr = tvram_c[addr + 0];
		atr = tvram_c[addr + 1];

		/* アトリビュート下位3ビットから描画色を設定 */
		col = (BYTE)((atr & 0x07) | 0x10);

		/* インテンシティアトリビュート処理 */
		if (atr & 0x20) {
			col |= (BYTE)8;
		}

		/* フォントデータ取得(ブリンクアトリビュート処理を含む) */
		if ((!(atr & 0x10) || text_blink) && (raster < 16)) {
			chr_dat = subcg_l4[(WORD)(chr * 16) + (nRaster % lines)];
		}
		else {
			chr_dat = 0x00;
		}

		/* リバースアトリビュート処理 */
		if (atr & 0x08) {
			chr_dat ^= (BYTE)0xFF;
		}

		/* カーソル処理 */
		if ((addr == cursor_addr) && (cursor_type != 1) &&
			(cursor_blink || (cursor_type == 0))) {
			if ((raster >= cursor_start) && (raster <= cursor_end)) {
				chr_dat ^= (BYTE)0xFF;
			}
		}

		/* 描画処理 */
		if (chr_dat) {
			for (x2 = 0; x2 < 8; x2++) {
				tmp = (DWORD)((x << 3) + x2);
				if (chr_dat & (1 << (7 ^ x2))) {
					if (width40_flag) {
						p[tmp * 2 + 0] = col;
						p[tmp * 2 + 1] = col;
					}
					else {
						p[tmp] = col;
					}
				}
			}
		}
	}
}

/*
 *	640x400、単色モード
 *	ラスタ描画
 */
static void FASTCALL DrawRasterL4(int nRaster)
{
	/* 表示範囲外の場合は何もしない */
	if (nRaster >= 400) {
		return;
	}

	/* レンダリング */
	RenderL4GDI(nRaster, nRaster + 1);
	RenderTextRasterGDI(nRaster, 0, 640);
}
#endif

#if XM7_VER >= 2
/*
 *	320x200、アナログモード
 *	ラスタ描画
 */
static void FASTCALL DrawRaster320(int nRaster)
{
	/* 表示範囲外の場合は何もしない */
	if (nRaster >= 200) {
		return;
	}

	/* パレット設定 */
	Palet320();

	/* レンダリング */
	/* ウィンドウオープン時 */
#if XM7_VER >= 3
	if (window_open) {
		/* ウィンドウ外の描画 */
		if ((nRaster < window_dy1) || (nRaster > window_dy2)) {
			Render320GDI32bpp(nRaster, nRaster + 1);
		}
		else {
			Render320wGDI32bpp(nRaster, nRaster + 1, window_dx1, window_dx2);
		}
	}
	else {
		Render320GDI32bpp(nRaster, nRaster + 1);
	}
#else
	Render320GDI(nRaster, nRaster + 1);
#endif
}
#endif

#if XM7_VER >= 3
/*
 *	320x200、26万色モード
 *	ラスタ描画
 */
static void FASTCALL DrawRaster256k(int nRaster)
{
	/* 表示範囲外の場合は何もしない */
	if (nRaster >= 200) {
		return;
	}

	/* レンダリング */
	if (crt_flag) {
		Render256kGDI(nRaster, nRaster + 1, multi_page);
	}
	else {
		Render256kGDI(nRaster, nRaster + 1, 0xff);
	}
}
#endif

/*
 *	ラスタ単位レンダリング
 *	描画
 */
void FASTCALL DrawPostRenderGDI(HWND hWnd, HDC hDC)
{
	HDC hMemDC;
	HBITMAP hDefBitmap;
	WORD top, bottom;

	ASSERT(hWnd);
	ASSERT(hDC);

	/* パレット設定 */
#if XM7_VER >= 3
	if (nMode == SCR_400LINE) {
		if (bPaletFlag) {
			Palet640();
		}
	}
	else if (nMode == SCR_200LINE) {
		if (bPaletFlag) {
			Palet640();
		}
		if (!bPseudo400Line) {
			if (bFullScan) {
				RenderFullScan();
			}
			else {
				RenderSetOddLineDigital();
			}
		}
	}
	else {
		if (bFullScan) {
			RenderFullScan();
		}
		else {
			RenderSetOddLineAnalog();
		}
	}
#elif XM7_VER >= 2
	if (bAnalog) {
#if defined(FMTV151)
		DrawV2_16bpp();
#endif
		if (bFullScan) {
			RenderFullScan();
		}
		else {
			RenderSetOddLineAnalog();
		}
	}
	else {
#if defined(FMTV151)
		DrawV2_4bpp();
#endif
		if (bPaletFlag) {
			Palet640();
		}
		if (!bPseudo400Line) {
			if (bFullScan) {
				RenderFullScan();
			}
			else {
				RenderSetOddLineDigital();
			}
		}
	}
#elif defined(L4CARD)
	if (b400Line) {
		if (bPaletFlag) {
			PaletL4();
		}
	}
	else {
		if (bPaletFlag) {
			Palet640();
		}
		if (!bPseudo400Line) {
			if (bFullScan) {
				RenderFullScan();
			}
			else {
				RenderSetOddLineDigital();
			}
		}
	}
#else
	if (bPaletFlag) {
		Palet640();
	}
	if (!bPseudo400Line) {
		if (bFullScan) {
			RenderFullScan();
		}
		else {
			RenderSetOddLineDigital();
		}
	}
#endif

	/* メモリDC作成、オブジェクト選択 */
	hMemDC = CreateCompatibleDC(hDC);
	ASSERT(hMemDC);
	hDefBitmap = (HBITMAP)SelectObject(hMemDC, hBitmap);

	/* ラスタ単位レンダリング時にひっくり返ることがあるので */
	if (nDrawTop >= nDrawBottom) {
		top = 0;
		bottom = 400;
	}
	else {
		top = nDrawTop;
		bottom = nDrawBottom;
	}

	/* デスクトップの状況いかんでは、拒否もあり得る様 */
	if (hDefBitmap) {
	/* パレット設定 */
#if XM7_VER >= 3
		switch (nMode) {
			case SCR_400LINE	:
			case SCR_200LINE	:
				SetDIBColorTable(hMemDC, 0, 16, (RGBQUAD *)rgbTTLGDI);
				break;
		}
#elif XM7_VER >= 2
		if (!bAnalog) {
			SetDIBColorTable(hMemDC, 0, 16, (RGBQUAD *)rgbTTLGDI);
		}
#elif defined(L4CARD)
		if (b400Line) {
			SetDIBColorTable(hMemDC, 0, 32, (RGBQUAD *)rgbTTLL4GDI);
		}
		else {
			SetDIBColorTable(hMemDC, 0, 16, (RGBQUAD *)rgbTTLGDI);
		}
#else
		SetDIBColorTable(hMemDC, 0, 16, (RGBQUAD *)rgbTTLGDI);
#endif

		if (bDoubleSize) {
			StretchBlt(hDC, 0, top * 2, 1280, (bottom - top) * 2,
				hMemDC, 0, top, 640, (bottom - top), SRCCOPY);
		}
		else {
			BitBlt(hDC, 0, top, 640, (bottom - top),
				hMemDC, 0, top, SRCCOPY);
		}
		SelectObject(hMemDC, hDefBitmap);

		/* 次回に備え、ワークリセット */
		nDrawTop = 400;
		nDrawBottom = 0;
		nDrawLeft = 640;
		nDrawRight = 0;
		SetDrawFlag(TRUE);
	}
	else {
		/* 再描画を起こす */
		InvalidateRect(hWnd, NULL, FALSE);
	}

	/* メモリDC削除 */
	DeleteDC(hMemDC);
}

/*
 *	ラスタ単位レンダリング
 *	各ラスタレンダリング
 */
void FASTCALL DrawRasterGDI(int nRaster)
{
	/* 640-320 自動切り替え */
	SelectGDI(hDrawWnd);

#if XM7_VER >= 3
	/* いずれかを使って描画 */
	switch (nMode) {
		case SCR_400LINE	:	DrawRaster400l(nRaster);
								return;
		case SCR_262144		:	DrawRaster256k(nRaster);
								return;
		case SCR_4096		:	DrawRaster320(nRaster);
								return;
		case SCR_200LINE	:	DrawRaster640(nRaster);
								return;
	}
#elif XM7_VER >= 2
	/* どちらかを使って描画 */
	if (bAnalog) {
		DrawRaster320(nRaster);
	}
	else {
		DrawRaster640(nRaster);
	}
#elif defined(L4CARD)
	/* どちらかを使って描画 */
	if (b400Line) {
		DrawRasterL4(nRaster);
	}
	else {
		DrawRaster640(nRaster);
	}
#else
	DrawRaster640(nRaster);
#endif
}

/*
 *	メニュー開始
 */
void FASTCALL EnterMenuGDI(HWND hWnd)
{
	ASSERT(hWnd);
	UNUSED(hWnd);

	/* ビットマップハンドルの有無で判定できる */
	if (!hBitmap) {
		return;
	}

	/* マウスカーソルon */
	if (!bMouseCursor) {
		ShowCursor(TRUE);
		bMouseCursor = TRUE;
	}

#if defined(MOUSE)
	/* マウスキャプチャ停止 */
	SetMouseCapture(FALSE);
#endif
}

/*
 *	メニュー終了
 */
void FASTCALL ExitMenuGDI(void)
{
	/* ビットマップハンドルの有無で判定できる */
	if (!hBitmap) {
		return;
	}

	/* マウスカーソルon */
	if (!bMouseCursor) {
		ShowCursor(TRUE);
		bMouseCursor = TRUE;
	}

#if defined(MOUSE)
	/* マウスキャプチャ開始 */
	SetMouseCapture(TRUE);
#endif
}

/*-[ VMとの接続 ]-----------------------------------------------------------*/

/*
 *	VRAMセット
 */
void FASTCALL VramGDI(WORD addr)
{
	WORD x;
	WORD y;

	/* y座標算出 */
#if XM7_VER >= 3
	switch (nMode) {
		case SCR_400LINE	:	addr &= 0x7fff;
								x = (WORD)((addr % 80) << 3);
								y = (WORD)(addr / 80);
								break;
		case SCR_262144		:
		case SCR_4096		:	addr &= 0x1fff;
								x = (WORD)((addr % 40) << 4);
								y = (WORD)((addr / 40) << 1);
								break;
		case SCR_200LINE	:	addr &= 0x3fff;
								x = (WORD)((addr % 80) << 3);
								y = (WORD)((addr / 80) << 1);
								break;
	}
#elif XM7_VER >= 2
	if (bAnalog) {
		addr &= 0x1fff;
		x = (WORD)((addr % 40) << 4);
		y = (WORD)((addr / 40) << 1);
	}
	else {
		addr &= 0x3fff;
		x = (WORD)((addr % 80) << 3);
		y = (WORD)((addr / 80) << 1);
	}
#elif defined(L4CARD)
	if (b400Line) {
		addr -= vram_offset[0];
		addr &= 0x7fff;
		x = (WORD)((addr % 80) << 3);
		y = (WORD)(addr / 80);
	}
	else {
		addr &= 0x3fff;
		x = (WORD)((addr % 80) << 3);
		y = (WORD)((addr / 80) << 1);
	}
#else
	addr &= 0x3fff;
	x = (WORD)((addr % 80) << 3);
	y = (WORD)((addr / 80) << 1);
#endif

	/* オーバーチェック */
	if ((x >= 640) || (y >= 400)) {
		return;
	}

	/* 再描画フラグを設定 */
	GDIDrawFlag[(y >> 3) * 80 + (x >> 3)] = 1;

	/* 垂直方向更新 */
	if (nDrawTop > y) {
		nDrawTop = y;
	}
	if (nDrawBottom <= y) {
#if XM7_VER >= 3
		if (nMode == SCR_400LINE) {
			nDrawBottom = (WORD)(y + 1);
		}
		else {
			nDrawBottom = (WORD)(y + 2);
		}
#else
		nDrawBottom = (WORD)(y + 2);
#endif
	}

	/* 水平方向更新 */
	if (nDrawLeft > x) {
		nDrawLeft = x;
	}
	if (nDrawRight <= x) {
#if XM7_VER >= 2
#if XM7_VER >= 3
		if (nMode & SCR_ANALOG) {
#else
		if (bAnalog) {
#endif
			nDrawRight = (WORD)(x + 16);
		}
		else {
			nDrawRight = (WORD)(x + 8);
		}
#else
		nDrawRight = (WORD)(x + 8);
#endif
	}
}

#if XM7_VER == 1 && defined(L4CARD)
/*
 *	テキストVRAMセット
 */
void FASTCALL TvramGDI(WORD addr)
{
	WORD ysize;
	WORD x;
	WORD y;
	BYTE maxy;
	WORD yy;

	maxy = (BYTE)(crtc_register[6] & 0x7f);

	/* 座標算出 */
	addr = (WORD)((addr - text_start_addr) & 0x0ffe);
	ysize = (WORD)((crtc_register[9] & 0x1f) + 1);
	if (width40_flag) {
		x = (WORD)((addr % 80) * 8);
		y = (WORD)(addr / 80);
	}
	else {
		x = (WORD)((addr % 160) * 4);
		y = (WORD)(addr / 160);
	}

	/* オーバーチェック */
	if (y >= maxy) {
		return;
	}
	y = (WORD)(y * ysize);
	if (y > 400 - ysize) {
		y = (WORD)(400 - ysize);
	}

	/* 再描画フラグを設定 */
	for (yy = y; yy < (WORD)(y + ysize); yy += (WORD)8) {
		GDIDrawFlag[(yy >> 3) * 80 + (x >> 3)] = 1;
		if (width40_flag) {
			GDIDrawFlag[(yy >> 3) * 80 + (x >> 3) + 1] = 1;
		}
	}

	/* 垂直方向更新 */
	if (nDrawTop > y) {
		nDrawTop = y;
	}
	if (nDrawBottom <= (y + ysize - 1)) {
		nDrawBottom = (WORD)(y + ysize);
	}

	/* 水平方向更新 */
	if (nDrawLeft > x) {
		nDrawLeft = x;
	}
	if (nDrawRight <= x) {
		if (width40_flag) {
			nDrawRight = (WORD)(x + 16);
		}
		else {
			nDrawRight = (WORD)(x + 8);
		}
	}
}

/*
 *	テキストVRAM再描画要求
 */
void FASTCALL ReDrawTVRamGDI(void)
{
	WORD maxy;

	maxy = (WORD)((crtc_register[6] & 0x7f) * ((crtc_register[9] & 0x1f) + 1));

	nDrawTop = 0;
	if (nDrawBottom < maxy) {
		nDrawBottom = maxy;
	}
	nDrawLeft = 0;
	nDrawRight = 640;
	SetDrawFlag(TRUE);
}
#endif

/*
 *	TTLパレットセット
 */
void FASTCALL DigitalGDI(void)
{
	/* 不要なレンダリングを抑制するため、領域設定は描画時に行う */
	bPaletFlag = TRUE;
}

/*
 *	アナログパレットセット
 */
void FASTCALL AnalogGDI(void)
{
	bPaletFlag = TRUE;
	nDrawTop = 0;
	nDrawBottom = 400;
	nDrawLeft = 0;
	nDrawRight = 640;
	SetDrawFlag(TRUE);
}

/*
 *	再描画要求
 */
void FASTCALL ReDrawGDI(void)
{
	/* 再描画 */
	nDrawTop = 0;
	nDrawBottom = 400;
	nDrawLeft = 0;
	nDrawRight = 640;
	bPaletFlag = TRUE;
	bClearFlag = TRUE;
	SetDrawFlag(TRUE);
}

#if XM7_VER >= 3
/*
 *	ハードウェアウィンドウ通知
 */
void FASTCALL WindowGDI(void)
{
	WORD tmpLeft, tmpRight;
	WORD tmpTop, tmpBottom;
	WORD tmpDx1, tmpDx2;
	WORD tmpDy1, tmpDy2;
	BYTE *p;
	int i;

	/* 26万色モード時は何もしない */
	if (nMode == SCR_262144) {
		return;
	}

	/* 前もってクリッピングする */
	window_clip(nMode);

	/* ウィンドウサイズを補正 */
	tmpDx1 = window_dx1;
	tmpDy1 = window_dy1;
	tmpDx2 = window_dx2;
	tmpDy2 = window_dy2;
	if (nMode != SCR_400LINE) {
		tmpDy1 <<= 1;
		tmpDy2 <<= 1;
	}
	if (nMode == SCR_4096) {
		tmpDx1 <<= 1;
		tmpDx2 <<= 1;
	}

	if (bWindowOpen != window_open) {
		if (window_open) {
			/* ウィンドウを開いた場合 */
			tmpLeft = tmpDx1;
			tmpRight = tmpDx2;
			tmpTop = tmpDy1;
			tmpBottom = tmpDy2;
		}
		else {
			/* ウィンドウを閉じた場合 */
			tmpLeft = nWindowDx1;
			tmpRight = nWindowDx2;
			tmpTop = nWindowDy1;
			tmpBottom = nWindowDy2;
		}
	}
	else {
		if (window_open) {
			/* 更新領域サイズを現在のものに設定 */
			tmpTop = nDrawTop;
			tmpBottom = nDrawBottom;
			tmpLeft = nDrawLeft;
			tmpRight = nDrawRight;

			/* 座標変更チェック */
			if (!((nWindowDx1 == tmpDx1) &&
				  (nWindowDy1 == tmpDy1) &&
				  (nWindowDx2 == tmpDx2) &&
				  (nWindowDy2 == tmpDy2))) {
				/* 左上X */
				if (nWindowDx1 < tmpDx1) {
					tmpLeft = nWindowDx1;
				}
				else {
					tmpLeft = tmpDx1;
				}

				/* 右下X */
				if (nWindowDx2 > tmpDx2) {
					tmpRight = nWindowDx2;
				}
				else {
					tmpRight = tmpDx2;
				}

				/* 左上Y */
				if (nWindowDy1 < tmpDy1) {
					tmpTop = nWindowDy1;
				}
				else {
					tmpTop = tmpDy1;
				}

				/* 右下Y */
				if (nWindowDy2 > tmpDy2) {
					tmpBottom = nWindowDy2;
				}
				else {
					tmpBottom = tmpDy2;
				}
			}
		}
		else {
			/* ウィンドウが開いていないので何もしない */
			return;
		}
	}

	/* 処理前の再描画領域と比較して広ければ領域を更新 */
	if (tmpLeft < nDrawLeft) {
		nDrawLeft = tmpLeft;
	}
	if (tmpRight > nDrawRight) {
		nDrawRight = tmpRight;
	}
	if (tmpTop < nDrawTop) {
		nDrawTop = tmpTop;
	}
	if (tmpBottom > nDrawBottom) {
		nDrawBottom = tmpBottom;
	}

	/* 再描画フラグを更新 */
	if ((nDrawLeft < nDrawRight) && (nDrawTop < nDrawBottom)) {
		if (nMode == SCR_400LINE) {
			SetDirtyFlag(nDrawTop, nDrawBottom + 1, TRUE);
		}
		else {
			SetDirtyFlag(nDrawTop >> 1, (nDrawBottom >> 1) + 1, TRUE);
		}
		p = &GDIDrawFlag[(nDrawTop >> 3) * 80 + (nDrawLeft >> 3)];
		for (i = (nDrawTop >> 3); i < ((nDrawBottom + 7) >> 3) ; i++) {
			memset(p, 1, (nDrawRight - nDrawLeft) >> 3);
			p += 80;
		}
	}

	/* ウィンドウオープン状態を保存 */
	bWindowOpen = window_open;
	nWindowDx1 = tmpDx1;
	nWindowDy1 = tmpDy1;
	nWindowDx2 = tmpDx2;
	nWindowDy2 = tmpDy2;
}
#endif

/*
 *	セレクトキャンセル処理
 */
void SelectCancelGDI(void)
{
	if (!bFullScreen) {
		bSelectCancelFlag = TRUE;
		SetDirtyFlag(0, 400, TRUE);
		SetDrawFlag(TRUE);
	}
}

#endif	/* _WIN32 */
