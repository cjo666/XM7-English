/*
 *	FM-7 EMULATOR "XM7"
 *
 *	Copyright (C) 1999-2017 ＰＩ．(Twitter:@xm6_original)
 *	Copyright (C) 2001-2017 Ryu Takegami (Twitter:@RyuTakegami)
 *
 *	[ Win32API DirectDraw ]
 *
 *	RHG履歴
 *	  2012.07.01		グリーンモニタモードに対応
 *	  2012.10.10		Windows 8でのDrawMenuBar APIの挙動変更に対策
 *						キャプションのバッファサイズを256バイトに増大
 *	  2013.02.12		ラスタレンダリングに対応
 *	  2014.03.16		V1/V2での32ビットカラーレンダラに対応
 */

#ifdef _WIN32

#define STRICT
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#ifdef DINPUT8
#define DIRECTDRAW_VERSION		0x700	/* DirectX7を指定 */
#else
#define DIRECTDRAW_VERSION		0x300	/* DirectX3を指定 */
#endif
#include <ddraw.h>
#include <objbase.h>
#include <assert.h>
#include "xm7.h"
#include "subctrl.h"
#include "display.h"
#include "multipag.h"
#include "ttlpalet.h"
#include "apalet.h"
#include "fdc.h"
#include "tapelp.h"
#include "bubble.h"
#include "keyboard.h"
#include "opn.h"
#include "w32.h"
#include "w32_bar.h"
#include "w32_sch.h"
#include "w32_res.h"
#include "w32_draw.h"
#include "w32_dd.h"
#include "w32_kbd.h"
#include "w32_sub.h"

/*
 *	グローバル ワーク
 */
#ifdef __cplusplus
extern "C" {
#endif
#if XM7_VER == 1
DWORD rgbTTLDD[16];						/* 640x200 パレット */
#else
DWORD rgbTTLDD[8];						/* 640x200 パレット */
DWORD rgbAnalogDD[4096];				/* 320x200 パレット */
#endif
BYTE nDDResolutionMode;					/* フルスクリーン時解像度 */
BOOL bDD480Status;						/* 640x480ステータスフラグ */
BOOL bDDtruecolor;						/* TrueColor優先フラグ */
BYTE DDDrawFlag[4000];					/* 8x8 再描画領域フラグ */
#if XM7_VER == 1 && defined(L4CARD)
DWORD rgbTTLL4DD[32];					/* 640x400 単色パレット */
#endif
#ifdef __cplusplus
}
#endif

/*
 *	スタティック ワーク
 */
#if XM7_VER >= 3
static BYTE nMode;						/* 画面モード */
#elif XM7_VER >= 2
static BOOL bAnalog;					/* アナログモードフラグ */
#elif XM7_VER == 1 && defined(L4CARD)
static BOOL b400Line;					/* 400ラインモードフラグ */
#endif
static RECT BkRect;						/* ウインドウ時の矩形 */
static BOOL bMouseCursor;				/* マウスカーソルフラグ */
static HMENU hMainMenu;					/* メインメニューハンドル */
static HBRUSH hDrawBrush;				/* ドローウィンドウのブラシ */
static HBRUSH hMainBrush;				/* メインウィンドウのブラシ */
static LPDIRECTDRAW2 lpdd2;				/* DirectDraw2 */
static LPDIRECTDRAWSURFACE lpdds[2];	/* DirectDrawSurface3 */
static LPDIRECTDRAWCLIPPER lpddc;		/* DirectDrawClipper */
static UINT nPixelFormat;				/* 320x200 ピクセルフォーマット */
static WORD nDrawTop;					/* 描画範囲上 */
static WORD nDrawBottom;				/* 描画範囲下 */
static WORD nDrawLeft;					/* 描画範囲左 */
static WORD nDrawRight;					/* 描画範囲右 */
static BOOL bPaletFlag;					/* パレット変更フラグ */
static BYTE nDDResolution;				/* フルスクリーン時 実際の解像度 */
static BOOL bClearFlag;					/* 上下クリアフラグ */
#if XM7_VER >= 3
static BOOL bWindowOpen;				/* ハードウェアウィンドウ状態 */
static WORD nWindowDx1;					/* ウィンドウ左上X座標 */
static WORD nWindowDy1;					/* ウィンドウ左上Y座標 */
static WORD nWindowDx2;					/* ウィンドウ右下X座標 */
static WORD nWindowDy2;					/* ウィンドウ右下Y座標 */
#endif

static char szRunMessage[128];			/* RUNメッセージ */
static char szStopMessage[128];			/* STOPメッセージ */
static char szCaption[256];				/* キャプション */
static char szCAPMessage[16];			/* CAPキーメッセージ */
static char szKANAMessage[16];			/* かなキーメッセージ */
static char szINSMessage[16];			/* INSキーメッセージ */
static int nCAP;						/* CAPキー */
static int nKANA;						/* かなキー */
static int nINS;						/* INSキー */
static int nDrive[2];					/* フロッピードライブ */
static char szDrive[2][16 + 1];			/* フロッピードライブ */
static int nTape;						/* テープ */
static BOOL bCaption;					/* キャプション表示 */
static int nScrX;						/* 横方向解像度 */
static int nScrY;						/* 縦方向解像度 */


/*
 *	マルチモニタ対応処理構造体
 *	Windows95でもコンパイルを通すため意図的に名前を変更している
 */
typedef struct tagMONITOR_INFO {  
    DWORD  cbSize; 
    RECT   rcMonitor; 
    RECT   rcWork; 
    DWORD  dwFlags; 
} MONITOR_INFO, *LPMONITOR_INFO; 

/*
 *	マルチモニタ対応処理関連
 */
static HMODULE hModUser32;				/* user32.dllモジュール */
static HMONITOR (WINAPI* monitorfromwindow)(HWND, DWORD);
static BOOL (WINAPI* getmonitorinfo)(HMONITOR, LPMONITOR_INFO);
static HRESULT (WINAPI* ddenumerateex)(LPDDENUMCALLBACKEX, LPVOID, DWORD);
static HMONITOR hmonitor;				/* 探索中の hmonitor */
static GUID gmonitor;					/* hmonitor に対応する GUID */
static POINT ptMouseBak;				/* ウィンドウ復帰時のマウス位置 */
static POINT ptFullScr;					/* フルスクリーン時初期マウス位置 */

/*
 *	アセンブラ関数のためのプロトタイプ宣言
 */
#ifdef __cplusplus
extern "C" {
#endif
#if XM7_VER == 1
/* 15/16ビットカラー */
void Render640DD(LPVOID lpSurface, LONG lPitch, int first, int last);
void RenderL4DD(LPVOID lpSurface, LONG lPitch, int first, int last);
void Render640mDD(LPVOID lpSurface, LONG lPitch, int first, int last);

/* 32ビットカラー */
void Render640Tc32DD(LPVOID lpSurface, LONG lPitch, int first, int last);
void RenderL4Tc32DD(LPVOID lpSurface, LONG lPitch, int first, int last);
void Render640mTc32DD(LPVOID lpSurface, LONG lPitch, int first, int last);

/* 呼び出し用 */
static void (*Render640)(LPVOID lpSurface, LONG lPitch, int first, int last);
static void (*Render640m)(LPVOID lpSurface, LONG lPitch, int first, int last);
#if defined(L4CARD)
static void (*RenderL4)(LPVOID lpSurface, LONG lPitch, int first, int last);
#endif
#elif XM7_VER == 2
/* 15/16ビットカラー */
void Render640DD(LPVOID lpSurface, LONG lPitch, int first, int last);
void Render320DD(LPVOID lpSurface, LONG lPitch, int first, int last);
void Render640cDD(LPVOID lpSurface, LONG lPitch, int first, int last);

/* 32ビットカラー */
void Render640Tc32DD(LPVOID lpSurface, LONG lPitch, int first, int last);
void Render320Tc32DD(LPVOID lpSurface, LONG lPitch, int first, int last);
void Render640cTc32DD(LPVOID lpSurface, LONG lPitch, int first, int last);

/* 呼び出し用 */
static void (*Render640)(LPVOID lpSurface, LONG lPitch, int first, int last);
static void (*Render320)(LPVOID lpSurface, LONG lPitch, int first, int last);
static void (*Render640c)(LPVOID lpSurface, LONG lPitch, int first, int last);
#elif XM7_VER >= 3
/* 15/16ビットカラー */
void Render640DD2(LPVOID lpSurface, LONG lPitch,
					int first, int last, int scale);
void Render640wDD2(LPVOID lpSurface, LONG lPitch,
					int first, int last, int firstx, int lastx, int scale);
void Render320DD(LPVOID lpSurface, LONG lPitch, int first, int last);
void Render320wDD(LPVOID lpSurface, LONG lPitch,
					int first, int last, int firstx, int lastx);
void Render256k555DD(LPVOID lpSurface, LONG lPitch,
					int first, int last, int multipage);
void Render256k565DD(LPVOID lpSurface, LONG lPitch,
					int first, int last, int multipage);
void Render640cDD(LPVOID lpSurface, LONG lPitch,
					int first, int last);
void Render640cwDD(LPVOID lpSurface, LONG lPitch,
					int first, int last, int firstx, int lastx);

/* 24ビットカラー */
void Render640Tc24DD2(LPVOID lpSurface, LONG lPitch,
					int first, int last, int scale);
void Render640wTc24DD2(LPVOID lpSurface, LONG lPitch,
					int first, int last, int firstx, int lastx, int scale);
void Render320Tc24DD(LPVOID lpSurface, LONG lPitch, int first, int last);
void Render320wTc24DD(LPVOID lpSurface, LONG lPitch,
					int first, int last, int firstx, int lastx);
void Render256kTc24DD(LPVOID lpSurface, LONG lPitch,
					int first, int last, int multipage);
void Render640cTc24DD(LPVOID lpSurface, LONG lPitch,
					int first, int last);
void Render640cwTc24DD(LPVOID lpSurface, LONG lPitch,
					int first, int last, int firstx, int lastx);

/* 32ビットカラー */
void Render640Tc32DD2(LPVOID lpSurface, LONG lPitch,
					int first, int last, int scale);
void Render640wTc32DD2(LPVOID lpSurface, LONG lPitch,
					int first, int last, int firstx, int lastx, int scale);
void Render320Tc32DD(LPVOID lpSurface, LONG lPitch, int first, int last);
void Render320wTc32DD(LPVOID lpSurface, LONG lPitch,
					int first, int last, int firstx, int lastx);
void Render256kTc32DD(LPVOID lpSurface, LONG lPitch,
					int first, int last, int multipage);
void Render640cTc32DD(LPVOID lpSurface, LONG lPitch,
					int first, int last);
void Render640cwTc32DD(LPVOID lpSurface, LONG lPitch,
					int first, int last, int firstx, int lastx);

/* 呼び出し用 */
static void (*Render640)(LPVOID lpSurface, LONG lPitch,
				int first, int last, int scale);
static void (*Render640w)(LPVOID lpSurface, LONG lPitch,
				int first, int last, int firstx, int lastx, int scale);
static void (*Render320)(LPVOID lpSurface, LONG lPitch, int first, int last);
static void (*Render320w)(LPVOID lpSurface, LONG lPitch,
				int first, int last, int firstx, int lastx);
static void (*Render256k)(LPVOID lpSurface, LONG lPitch,
				int first, int last, int multipage);
static void (*Render640c)(LPVOID lpSurface, LONG lPitch,
				int first, int last);
static void (*Render640cw)(LPVOID lpSurface, LONG lPitch,
				int first, int last, int firstx, int lastx);
#endif
#ifdef __cplusplus
}
#endif

#if XM7_VER == 1 && defined(L4CARD)
/*
 *	16色モード用パレットコード
 */
static const WORD Palet16_L4[] = {
	0x0000,	0x0017,	0xb800,	0xb817,	0x05e0,	0x05f7,	0xbde0,	0xbdf7,
	0x4208,	0x421f,	0xfa08,	0xfa1f,	0x47e8,	0x47ff,	0xffe8,	0xffff,
	0x0000,	0x0017,	0x5c00,	0x5c17,	0x02e0,	0x02f7,	0x5ee0,	0x5ef7,
	0x2108,	0x211f,	0x7d08,	0x7d1f,	0x23e8,	0x23ff,	0x7fe8,	0x7fff,
	0x0000,	0x0180,	0x0320,	0x0380,	0x0540,	0x0560,	0x05e0,	0x0600,
	0x0240,	0x0320,	0x04a0,	0x0500,	0x0700,	0x0740,	0x07c0,	0x07e0,
	0x0000,	0x00c0,	0x0180,	0x01c0,	0x02a0,	0x02c0,	0x02e0,	0x0300,
	0x0120,	0x0180,	0x0240,	0x0280,	0x0380,	0x03a0,	0x03c0,	0x03e0,
};
static const DWORD Palet16Tc_L4[] = {
	0x00000000,	0x000000bb,	0x00bb0000,	0x00bb00bb,
	0x0000bb00,	0x0000bbbb,	0x00bbbb00,	0x00bbbbbb,
	0x00444444,	0x004444ff,	0x00ff4444,	0x00ff44ff,
	0x0044ff44,	0x0044ffff,	0x00ffff44,	0x00ffffff,
	0x00000000,	0x00003400,	0x00005E00,	0x00006900,
	0x0000a000,	0x0000a600,	0x0000b500,	0x0000bb00,
	0x00004500,	0x00006000,	0x00008c00,	0x00009900,
	0x0000de00,	0x0000e600,	0x0000f800,	0x0000ff00
};
#endif

/*
 *	プロトタイプ宣言
 */
static void FASTCALL AllClear(BOOL clear_flag);


/*
 *	初期化
 */
void FASTCALL InitDD(void)
{
	/* ワークエリア初期化(設定ワークは変更しない) */
#if XM7_VER >= 3
	nMode = SCR_200LINE;
#elif XM7_VER >= 2
	bAnalog = FALSE;
#elif XM7_VER == 1 && defined(L4CARD)
	b400Line = FALSE;
#endif
	bMouseCursor = TRUE;
	lpdd2 = NULL;
	memset(lpdds, 0, sizeof(lpdds));
	lpddc = NULL;
	bClearFlag = TRUE;
	ptFullScr.x = 0;
	ptFullScr.y = 0;

	/* ステータスライン */
	szCaption[0] = '\0';
	nCAP = -1;
	nKANA = -1;
	nINS = -1;
	nDrive[0] = -1;
	nDrive[1] = -1;
	szDrive[0][0] = '\0';
	szDrive[1][0] = '\0';
	nTape = -1;
	bCaption = TRUE;

	/* マルチモニタ対応用にMonitorFromWindowをロード */
	hModUser32 = LoadLibrary("user32.dll");
	if (hModUser32) {
		monitorfromwindow = (HMONITOR (WINAPI*)(HWND, DWORD))GetProcAddress(hModUser32, "MonitorFromWindow");
		getmonitorinfo = (BOOL (WINAPI*)(HMONITOR, LPMONITOR_INFO))GetProcAddress(hModUser32, "GetMonitorInfoA");
	}
	else {
		monitorfromwindow = NULL;
		getmonitorinfo = NULL;
	}

	/* メッセージをロード */
	if (LoadString(hAppInstance, IDS_RUNCAPTION,
					szRunMessage, sizeof(szRunMessage)) == 0) {
		szRunMessage[0] = '\0';
	}
	if (LoadString(hAppInstance, IDS_STOPCAPTION,
					szStopMessage, sizeof(szStopMessage)) == 0) {
		szStopMessage[0] = '\0';
	}
	if (LoadString(hAppInstance, IDS_BAR_CAP,
					szCAPMessage, sizeof(szCAPMessage)) == 0) {
		strncpy(szCAPMessage, "CAP", sizeof(szCAPMessage));
	}
	if (LoadString(hAppInstance, IDS_BAR_KANA,
					szKANAMessage, sizeof(szKANAMessage)) == 0) {
#if XM7_VER == 1
		strncpy(szKANAMessage, "カナ", sizeof(szKANAMessage));
#else
		strncpy(szKANAMessage, "かな", sizeof(szKANAMessage));
#endif
	}
	if (LoadString(hAppInstance, IDS_BAR_INS,
					szINSMessage, sizeof(szINSMessage)) == 0) {
		strncpy(szINSMessage, "INS", sizeof(szINSMessage));
	}
}

/*
 *	クリーンアップ
 */
void FASTCALL CleanDD(void)
{
	DWORD dwStyle;
	RECT brect;
	int i;
	int width;
	int height;

	/* DirectDrawClipper */
	if (lpddc) {
		lpddc->Release();
		lpddc = NULL;
	}

	/* DirectDrawSurface3 */
	for (i=0; i<2; i++) {
		if (lpdds[i]) {
			lpdds[i]->Release();
			lpdds[i] = NULL;
		}
	}

	/* DirectDraw2 */
	if (lpdd2) {
		lpdd2->Release();
		lpdd2 = NULL;
	}

	/* ウインドウスタイルを戻す */
	dwStyle = GetWindowLong(hMainWnd, GWL_STYLE);
	dwStyle &= ~WS_POPUP;
	dwStyle |= (WS_CAPTION | WS_BORDER | WS_SYSMENU);
	SetWindowLong(hMainWnd, GWL_STYLE, dwStyle);
	dwStyle = GetWindowLong(hMainWnd, GWL_EXSTYLE);
	dwStyle |= WS_EX_WINDOWEDGE;
	SetWindowLong(hMainWnd, GWL_EXSTYLE, dwStyle);

	/* Windows Vista/Windows 7でのアイコン表示復帰 */
	SetClassLong(hMainWnd, GCL_HICONSM,
		(LONG)LoadIcon(hAppInstance, IDI_APPLICATION));

	/* Windows 8でのメニュー・ブラシ復元 */
	if (bWin8flag) {
		SetMenu(hMainWnd, hMainMenu);
		SetClassLong(hMainWnd, GCL_HBRBACKGROUND, (LONG)hMainBrush);
		SetClassLong(hDrawWnd, GCL_HBRBACKGROUND, (LONG)hDrawBrush);
	}

	/* ウインドウ位置を戻す */
	if (bDoubleSize) {
		width = 1280;
		height = 800;
	}
	else {
		width = 640;
		height = 400;
	}
	SetWindowPos(hMainWnd, HWND_NOTOPMOST, BkRect.left, BkRect.top,
		(BkRect.right - BkRect.left), (BkRect.bottom - BkRect.top),
		SWP_DRAWFRAME);

	MoveWindow(hDrawWnd, 0, 0, width, height, TRUE);
	if (hStatusBar) {
		GetWindowRect(hStatusBar, &brect);
		MoveWindow(hStatusBar, 0, height,
					(brect.right - brect.left),
					(brect.bottom - brect.top),
					TRUE);
	}

	/* user32.dllを解放 */
	if (hModUser32) {
		FreeLibrary(hModUser32);
		hModUser32 = NULL;
	}

	/* マウスポインタ位置を復帰 */
	SetCursorPos(ptMouseBak.x, ptMouseBak.y);
}


/*
 *	マルチモニタ用DirectDraw列挙コールバック関数
 */
static BOOL WINAPI DDEnumCallback(GUID FAR* guid, LPSTR desc, LPSTR name, LPVOID context, HMONITOR hm)
{
	UNUSED(desc);
	UNUSED(name);
	UNUSED(context);

	if (hm == hmonitor) {
		gmonitor = *guid;
		return 0;
	}
	return 1;
}

/*
 *	画面設定サブ
 */
static BOOL FASTCALL SetDisplayMode(int x, int y)
{
#if XM7_VER >= 3
	if (bDDtruecolor) {
		nPixelFormat = 3;
		if (FAILED(lpdd2->SetDisplayMode(x, y, 32, 0, 0))) {
			nPixelFormat = 4;
			if (FAILED(lpdd2->SetDisplayMode(x, y, 24, 0, 0))) {
				nPixelFormat = 0;
				if (FAILED(lpdd2->SetDisplayMode(x, y, 16, 0, 0))) {
					/* 失敗 */
					return FALSE;
				}
			}
		}
	}
	else {
		if (bWin8flag) {
			nPixelFormat = 3;
			if (FAILED(lpdd2->SetDisplayMode(x, y, 32, 0, 0))) {
				nPixelFormat = 0;
				if (FAILED(lpdd2->SetDisplayMode(x, y, 16, 0, 0))) {
					/* 失敗 */
					return FALSE;
				}
			}
		}
		else {
			nPixelFormat = 0;
			if (FAILED(lpdd2->SetDisplayMode(x, y, 16, 0, 0))) {
				nPixelFormat = 3;
				if (FAILED(lpdd2->SetDisplayMode(x, y, 32, 0, 0))) {
					/* 失敗 */
					return FALSE;
				}
			}
		}
	}
#else
	if (bWin8flag || bDDtruecolor) {
		nPixelFormat = 3;
		if (FAILED(lpdd2->SetDisplayMode(x, y, 32, 0, 0))) {
			nPixelFormat = 0;
			if (FAILED(lpdd2->SetDisplayMode(x, y, 16, 0, 0))) {
				/* 失敗 */
				return FALSE;
			}
		}
	}
	else {
		nPixelFormat = 0;
		if (FAILED(lpdd2->SetDisplayMode(x, y, 16, 0, 0))) {
			nPixelFormat = 3;
			if (FAILED(lpdd2->SetDisplayMode(x, y, 32, 0, 0))) {
				/* 失敗 */
				return FALSE;
			}
		}
	}
#endif

	/* 成功 */
	nScrX = x;
	nScrY = y;
	return TRUE;
}

/*
 *	セレクト
 */
BOOL FASTCALL SelectDD(void)
{
	DWORD dwStyle;
	LPDIRECTDRAW lpdd;
	DDSURFACEDESC ddsd;
	DDPIXELFORMAT ddpf;
	RECT brect;
	LPDIRECTDRAW lpdd1;
	HMODULE hModDDraw;
	RECT srect;
	MONITOR_INFO minfo;

	/* assert */
	ASSERT(hMainWnd);

	/* ウインドウ矩形・マウスポインタ位置を記憶する */
	GetWindowRect(hMainWnd, &BkRect);
	GetCursorPos(&ptMouseBak);

	/* メニューハンドルを記憶する */
	if (bWin8flag) {
		hMainMenu = GetMenu(hMainWnd);
	}

	/* ウインドウスタイルを変更 */
	dwStyle = GetWindowLong(hMainWnd, GWL_STYLE);
	dwStyle &= ~(WS_CAPTION | WS_BORDER | WS_SYSMENU);
	dwStyle |= WS_POPUP;
	SetWindowLong(hMainWnd, GWL_STYLE, dwStyle);
	dwStyle = GetWindowLong(hMainWnd, GWL_EXSTYLE);
	dwStyle &= ~WS_EX_WINDOWEDGE;
	SetWindowLong(hMainWnd, GWL_EXSTYLE, dwStyle);

	/* DirectDrawオブジェクトを作成(マルチモニタ対応) */
	lpdd = NULL;
	hmonitor = NULL;

	/* マルチモニタ対応用DirectDraw初期化処理 */
	memset(&gmonitor, 0, sizeof(gmonitor));
	if (monitorfromwindow) {
		hModDDraw = LoadLibrary("ddraw.dll");
		if (hModDDraw) {
			ddenumerateex = (HRESULT (WINAPI*)(LPDDENUMCALLBACKEX, LPVOID,
				DWORD))GetProcAddress(hModDDraw,"DirectDrawEnumerateExA");
			if (ddenumerateex) {
				hmonitor = monitorfromwindow(hMainWnd, 1);
				ddenumerateex(DDEnumCallback, NULL,
							  DDENUM_ATTACHEDSECONDARYDEVICES);
			}
			FreeLibrary(hModDDraw);
		}
	}

	if (!FAILED(CoCreateInstance(CLSID_DirectDraw, 0, CLSCTX_ALL, IID_IDirectDraw, (void**)&lpdd1))) {
		if (!FAILED(IDirectDraw_Initialize(lpdd1, &gmonitor))) {
			lpdd = lpdd1;
		}
	}

	if (!lpdd) {
		/* DirectDrawオブジェクトを作成 */
		if (FAILED(DirectDrawCreate(NULL, &lpdd, NULL))) {
			return FALSE;
		}
	}

	/* 協調モードを設定 */
	if (FAILED(lpdd->SetCooperativeLevel(hMainWnd, DDSCL_EXCLUSIVE | DDSCL_FULLSCREEN))) {
		lpdd->Release();
		return FALSE;
	}

	/* DirectDraw2インタフェースを取得 */
	if (FAILED(lpdd->QueryInterface(IID_IDirectDraw2, (LPVOID*)&lpdd2))) {
		lpdd->Release();
		return FALSE;
	}

	/* ここまで来れば、DirectDrawはもはや必要ない */
	lpdd->Release();

	/* 画面モードを設定 */
	switch (nDDResolutionMode) {
		case 0:	/* 640 x 400 */
			nDDResolution = DDRES_400LINE;
			if (!SetDisplayMode(640, 400)) {
				nDDResolution = DDRES_480LINE;
				if (!SetDisplayMode(640, 480)) {
					return FALSE;
				}
			}
			break;
		case 1:	/* 640 x 480 */
			nDDResolution = DDRES_480LINE;
			if (!SetDisplayMode(640, 480)) {
				nDDResolution = DDRES_400LINE;
				if (!SetDisplayMode(640, 400)) {
					return FALSE;
				}
			}
			break;
		case 2:	/* 1920 x 1200 */
			nDDResolution = DDRES_WUXGA;
			if (!SetDisplayMode(1920, 1200)) {
				nDDResolution = DDRES_400LINE;
				if (!SetDisplayMode(640, 400)) {
					nDDResolution = DDRES_480LINE;
					if (!SetDisplayMode(640, 480)) {
						return FALSE;
					}
				}
			}
			break;
		case 3:	/* 1280 x 1024 */
			nDDResolution = DDRES_SXGA;
			if (!SetDisplayMode(1280, 1024)) {
				nDDResolution = DDRES_480LINE;
				if (!SetDisplayMode(640, 480)) {
					nDDResolution = DDRES_400LINE;
					if (!SetDisplayMode(640, 400)) {
						return FALSE;
					}
				}
			}
			break;
		case 4:	/* 1280 x 800 */
			nDDResolution = DDRES_WXGA800;
			if (!SetDisplayMode(1280, 800)) {
				nDDResolution = DDRES_400LINE;
				if (!SetDisplayMode(640, 400)) {
					nDDResolution = DDRES_480LINE;
					if (!SetDisplayMode(640, 480)) {
						return FALSE;
					}
				}
			}
			break;
		default:ASSERT(FALSE);
				return FALSE;
	}

	/* プライマリサーフェイスを作成 */
	memset(&ddsd, 0, sizeof(ddsd));
	ddsd.dwSize = sizeof(ddsd);
	ddsd.dwFlags = DDSD_CAPS;
	if ((nDDResolution == DDRES_400LINE) || (nDDResolution == DDRES_480LINE)) {
		ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;
	}
	else {
		ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE | DDSCAPS_SYSTEMMEMORY;
	}
	if (FAILED(lpdd2->CreateSurface(&ddsd, &lpdds[0], NULL))) {
		return FALSE;
	}

	/* ワークサーフェイスを作成(DDSCAPS_SYSTEMMEMORYを指定する) */
	memset(&ddsd, 0, sizeof(ddsd));
	ddsd.dwSize = sizeof(ddsd);
	ddsd.dwFlags = DDSD_CAPS | DDSD_WIDTH | DDSD_HEIGHT;
	ddsd.dwWidth = 640;
	if (nDDResolution == DDRES_480LINE) {
		ddsd.dwHeight = 480;
	}
	else if (nDDResolution == DDRES_SXGA) {
		ddsd.dwHeight = 512;
	}
	else {
		ddsd.dwHeight = 400;
	}
	ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_SYSTEMMEMORY;
	if (FAILED(lpdd2->CreateSurface(&ddsd, &lpdds[1], NULL))) {
		return FALSE;
	}

	if (nPixelFormat == 0) {
		/* ピクセルフォーマットを得る */
		memset(&ddpf, 0, sizeof(ddpf));
		ddpf.dwSize = sizeof(ddpf);
		if (FAILED(lpdds[1]->GetPixelFormat(&ddpf))) {
			return FALSE;
		}

		/* ピクセルフォーマットをチェック HELで規定されている2タイプのみ対応 */
		if (!(ddpf.dwFlags & DDPF_RGB)) {
			return FALSE;
		}
		if ((ddpf.dwRBitMask == 0xf800) &&
			(ddpf.dwGBitMask == 0x07e0) &&
			(ddpf.dwBBitMask == 0x001f)) {
			nPixelFormat = 1;
		}
		if ((ddpf.dwRBitMask == 0x7c00) &&
			(ddpf.dwGBitMask == 0x03e0) &&
			(ddpf.dwBBitMask == 0x001f)) {
			nPixelFormat = 2;
		}

		if (nPixelFormat == 0) {
			return FALSE;
		}
	}

#if XM7_VER == 1
	/* レンダリング関数設定 */
	if (nPixelFormat == 3) {
		Render640 = Render640Tc32DD;
		Render640m = Render640mTc32DD;
#if defined(L4CARD)
		RenderL4 = RenderL4Tc32DD;
#endif
	}
	else {
		Render640 = Render640DD;
		Render640m = Render640mDD;
#if defined(L4CARD)
		RenderL4 = RenderL4DD;
#endif
	}
#elif XM7_VER == 2
	/* レンダリング関数設定 */
	if (nPixelFormat == 3) {
		Render640 = Render640Tc32DD;
		Render320 = Render320Tc32DD;
		Render640c = Render640cTc32DD;
	}
	else {
		Render640 = Render640DD;
		Render320 = Render320DD;
		Render640c = Render640cDD;
	}
#elif XM7_VER >= 3
	/* レンダリング関数設定 */
	if (nPixelFormat == 1) {
		Render640 = Render640DD2;
		Render320 = Render320DD;
		Render256k = Render256k565DD;
		Render640w = Render640wDD2;
		Render320w = Render320wDD;
		Render640c = Render640cDD;
		Render640cw = Render640cwDD;
	}
	else if (nPixelFormat == 2) {
		Render640 = Render640DD2;
		Render320 = Render320DD;
		Render256k = Render256k555DD;
		Render640w = Render640wDD2;
		Render320w = Render320wDD;
		Render640c = Render640cDD;
		Render640cw = Render640cwDD;
	}
	else if (nPixelFormat == 3) {
		Render640 = Render640Tc32DD2;
		Render320 = Render320Tc32DD;
		Render256k = Render256kTc32DD;
		Render640w = Render640wTc32DD2;
		Render320w = Render320wTc32DD;
		Render640c = Render640cTc32DD;
		Render640cw = Render640cwTc32DD;
	}
	else {
		Render640 = Render640Tc24DD2;
		Render320 = Render320Tc24DD;
		Render256k = Render256kTc24DD;
		Render640w = Render640wTc24DD2;
		Render320w = Render320wTc24DD;
		Render640c = Render640cTc24DD;
		Render640cw = Render640cwTc24DD;
	}
#endif

	/* クリッパーを作成、割り当て */
	if (FAILED(lpdd2->CreateClipper(NULL, &lpddc, NULL))) {
		return FALSE;
	}
	if (FAILED(lpddc->SetHWnd(NULL, hDrawWnd))) {
		return FALSE;
	}

	/* ウインドウサイズを変更 */
	if (nDDResolution != DDRES_400LINE) {
		if (hmonitor && getmonitorinfo) {
			memset(&minfo, 0, sizeof(minfo));
			minfo.cbSize = sizeof(minfo);
			getmonitorinfo(hmonitor, &minfo);
			srect = minfo.rcWork;
		}
		else {
			srect.left = 0;
			srect.top = 0;
			srect.right = nScrX;
			srect.bottom = nScrY;
		}
		if (hStatusBar) {
			GetWindowRect(hStatusBar, &brect);
		}
		else {
			brect.top = 0;
			brect.bottom = 0;
		}
		MoveWindow(hMainWnd, srect.left, srect.top, nScrX, nScrY + (brect.bottom - brect.top), TRUE);
		MoveWindow(hDrawWnd, 0, 0, nScrX, nScrY, TRUE);
		if (hStatusBar) {
			MoveWindow(hStatusBar, 0, nScrY, 0, (brect.bottom - brect.top),
						TRUE);
		}
	}

	/* メニューバー消去・ブラシ記憶(Windows 8のみ) */
	if (bWin8flag) {
		hDrawBrush = (HBRUSH)GetClassLong(hDrawWnd, GCL_HBRBACKGROUND);
		hMainBrush = (HBRUSH)GetClassLong(hMainWnd, GCL_HBRBACKGROUND);
		SetClassLong(hDrawWnd, GCL_HBRBACKGROUND, NULL);
		SetClassLong(hMainWnd, GCL_HBRBACKGROUND, NULL);

		SetMenu(hMainWnd, NULL);
		DrawMenuBar(hMainWnd);
	}

	/* キャプション表示フラグを設定 */
	if ((nDDResolution == DDRES_480LINE) || (nDDResolution == DDRES_SXGA)) {
		bCaption = bDD480Status;
	}
	else {
		bCaption = TRUE;
	}

	/* マウス位置を画面中央に移動する */
	GetWindowRect(hMainWnd, &srect);
	ptFullScr.x = (srect.right + srect.left) / 2;
	ptFullScr.y = (srect.bottom + srect.top) / 2;
	SetCursorPos(ptFullScr.x, ptFullScr.y);

	/* ワークセット */
#if XM7_VER >= 3
	nMode = SCR_200LINE;
#elif XM7_VER >= 2
	bAnalog = FALSE;
#elif XM7_VER == 1 && defined(L4CARD)
	b400Line = FALSE;
#endif
	ReDrawDD();
	AllClear(TRUE);
	InvalidateRect(hMainWnd, NULL, TRUE);
	InvalidateRect(hDrawWnd, NULL, TRUE);

	/* 完了 */
	return TRUE;
}

/*-[ 描画 ]-----------------------------------------------------------------*/

#if XM7_VER == 2 && defined(FMTV151)
/*
 *	V2合成
 */
static void FASTCALL DrawV2_DD(BYTE *pSurface, LONG lPitch)
{
	BYTE	x, y;
	int		xx, yy;
	WORD	col;
	WORD	*pbits;
	DWORD	*pbitsTC;

	/* パレットテーブル */
	static const WORD V2rgbTable[] = {
		0x0000 | 0x0000 | 0x0000,
		0x0000 | 0x07e0 | 0x0000,
		0x0000 | 0x0000 | 0x0000,
		0x0000 | 0x03e0 | 0x0000,
		0x0000 | 0x0000 | 0x0000
	};
	static const WORD V2rgbTableTc[] = {
		0x00000000,	0x0000ff00,	0x00000000
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


	if (nPixelFormat == 3) {
		pbitsTC = (DWORD *)(pSurface + V2YPoint * lPitch * 2 + V2XPoint * 4);

		for (y = 0; y < V2YSize; y ++) {
			for (x = 0; x < V2XSize; x ++) {
				if (nV2data[y * V2XSize + x]) {
					col = V2rgbTableTc[nV2data[y * V2XSize + x] + 0];
					for (yy = y * V2YPxSz; yy < (y + 1) * V2YPxSz; yy ++) {
						for (xx = x * V2XPxSz; xx < (x + 1) * V2XPxSz; xx ++) {
							pbitsTC[yy * (lPitch / 2) + xx] = col;
							if (!bAnalog && bPseudo400Line) {
								pbitsTC[yy * (lPitch/2) +(lPitch/4)+xx] = col;
							}
						}
					}
				}
			}
		}
	}
	else {
		pbits = (WORD *)(pSurface + V2YPoint * lPitch * 2 + V2XPoint * 2);

		for (y = 0; y < V2YSize; y ++) {
			for (x = 0; x < V2XSize; x ++) {
				if (nV2data[y * V2XSize + x]) {
					if (nPixelFormat == 1) {
						col = V2rgbTable[nV2data[y * V2XSize + x] + 0];
					}
					else {
						col = V2rgbTable[nV2data[y * V2XSize + x] + 2];
					}
					for (yy = y * V2YPxSz; yy < (y + 1) * V2YPxSz; yy ++) {
						for (xx = x * V2XPxSz; xx < (x + 1) * V2XPxSz; xx ++) {
							pbits[yy * lPitch + xx] = col;
							if (!bAnalog && bPseudo400Line) {
								pbits[yy * lPitch + (lPitch / 2) + xx] = col;
							}
						}
					}
				}
			}
		}
	}
}
#endif

/*
 *	全ての再描画フラグを設定
 */
static void FASTCALL SetDrawFlag(BOOL flag)
{
	memset(DDDrawFlag, (BYTE)flag, sizeof(DDDrawFlag));
}

/*
 *	全領域クリア
 */
static void FASTCALL AllClear(BOOL clear_flag)
{
	DDSURFACEDESC ddsd;
	HRESULT hResult;
	RECT rect;
	RECT drect;
	BYTE *p;
	int i;
	int lines;

	/* フラグチェック */
	if (!bClearFlag) {
		return;
	}

	/* サーフェイスをロック */
	memset(&ddsd, 0, sizeof(ddsd));
	ddsd.dwSize = sizeof(ddsd);
	hResult = lpdds[1]->Lock(NULL, &ddsd,
							 DDLOCK_WAIT | DDLOCK_SURFACEMEMORYPTR, NULL);
	if (hResult != DD_OK) {
		/* サーフェイスがロストしていれば、リストア */
		if (hResult == DDERR_SURFACELOST) {
			lpdds[1]->Restore();
		}
		/* 次回は全領域更新 */
		ReDrawDD();
		return;
	}

	/* オールクリア */
	if (clear_flag) {
		p = (BYTE *)ddsd.lpSurface;
		if (nDDResolution == DDRES_480LINE) {
			lines = 480;
		}
		else if (nDDResolution == DDRES_SXGA) {
			lines = 512;
		}
		else {
			lines = 400;
		}
		for (i=0; i<lines; i++) {
			if (nPixelFormat == 3) {
				memset(p, 0, 640 * 4);
			}
#if XM7_VER >= 3
			else if (nPixelFormat == 4) {
				memset(p, 0, 640 * 3);
			}
#endif
			else {
				memset(p, 0, 640 * 2);
			}
			p += ddsd.lPitch;
		}
	}

	/* サーフェイスをアンロック */
	lpdds[1]->Unlock(ddsd.lpSurface);

	/* ワークリセット */
	bClearFlag = FALSE;
	nDrawTop = 0;
	nDrawBottom = 400;
	nDrawLeft = 0;
	nDrawRight = 640;
	SetDrawFlag(TRUE);
#if XM7_VER >= 3
	bWindowOpen = FALSE;
	nWindowDx1 = 640;
	nWindowDy1 = 400;
	nWindowDx2 = 0;
	nWindowDy2 = 0;
#endif
	SetDirtyFlag(0, 400, TRUE);

	/* ステータスラインをクリア */
	szCaption[0] = '\0';
	nCAP = -1;
	nKANA = -1;
	nINS = -1;
	nDrive[0] = -1;
	nDrive[1] = -1;
	szDrive[0][0] = '\0';
	szDrive[1][0] = '\0';
	nTape = -1;

	/* 640x480時のみBlt */
	if ((nDDResolution == DDRES_400LINE) || (nDDResolution == DDRES_WUXGA) ||
		(nDDResolution == DDRES_WXGA800)) {
		return;
	}

	/* 条件が揃えば、Blt */
	if (nDDResolution == DDRES_480LINE) {
		rect.top = 0;
		rect.left = 0;
		rect.right = 640;
		rect.bottom = 40;
		drect = rect;
	}
	else if (nDDResolution == DDRES_SXGA) {
		rect.top = 0;
		rect.left = 0;
		rect.right = 640;
		rect.bottom = 56;
		drect.top = 0;
		drect.left = 0;
		drect.right = 1280;
		drect.bottom = 112;
	}
	hResult = lpdds[0]->Blt(&drect, lpdds[1], &rect, DDBLT_WAIT, NULL);
	if (hResult != DD_OK) {
		/* サーフェイスがロストしていれば、リストア */
		if (hResult == DDERR_SURFACELOST) {
			lpdds[0]->Restore();
			lpdds[1]->Restore();
		}
		/* 次回は全領域更新 */
		ReDrawDD();
		return;
	}

	/* 下部インジケータ描画 */
	if (nDDResolution == DDRES_480LINE) {
		rect.top = 440;
		rect.left = 0;
		rect.right = 640;
		rect.bottom = 480;
		drect = rect;
	}
	else if (nDDResolution == DDRES_SXGA) {
		rect.top = 456;
		rect.left = 0;
		rect.right = 640;
		rect.bottom = 480;
		drect.top = 912;
		drect.left = 0;
		drect.right = 1280;
		drect.bottom = 1024;
	}
	hResult = lpdds[0]->Blt(&drect, lpdds[1], &rect, DDBLT_WAIT, NULL);
	if (hResult != DD_OK) {
		/* サーフェイスがロストしていれば、リストア */
		if (hResult == DDERR_SURFACELOST) {
			lpdds[0]->Restore();
			lpdds[1]->Restore();
		}
		/* 次回は全領域更新 */
		ReDrawDD();
		return;
	}
}

/*
 *	スキャンライン描画
 */
static void FASTCALL RenderFullScan(BYTE *pSurface, LONG lPitch)
{
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

	/* 初期設定 */
	pSurface += top * lPitch;

	/* フラグチェック */
	if (bFullScanFS) {
		/* ループ */
		for (u=top; u<bottom; u += (WORD)2) {
			if (nPixelFormat == 3) {
				memcpy(&pSurface[lPitch], pSurface, 640 * 4);
			}
#if XM7_VER >= 3
			else if (nPixelFormat == 4) {
				memcpy(&pSurface[lPitch], pSurface, 640 * 3);
			}
#endif
			else {
				memcpy(&pSurface[lPitch], pSurface, 640 * 2);
			}
			pSurface += (lPitch * 2);
		}
	}
	else {
		/* ラスタ単位レンダリング時はここでクリアしないといけない */
		if (bRasterRendering) {
			/* ループ */
			for (u=top; u<bottom; u += (WORD)2) {
				if (nPixelFormat == 3) {
					memset(&pSurface[lPitch], 0x00, 640 * 4);
				}
#if XM7_VER >= 3
				else if (nPixelFormat == 4) {
					memset(&pSurface[lPitch], 0x00, 640 * 3);
				}
#endif
				else {
					memset(&pSurface[lPitch], 0x00, 640 * 2);
				}
				pSurface += (lPitch * 2);
			}
		}
	}
}

/*
 *	ステータスライン(キャプション)描画
 */
static void FASTCALL DrawCaption(void)
{
	HDC hDC;
	HRESULT hResult;
	RECT rect;
	RECT drect;

	ASSERT((nDDResolution == DDRES_480LINE) || (nDDResolution == DDRES_SXGA));
	ASSERT(bCaption);

	/* DC取得 */
	hResult = lpdds[1]->GetDC(&hDC);
	if (hResult != DD_OK) {
		/* サーフェイスがロストしていれば、リストア */
		if (hResult == DDERR_SURFACELOST) {
			lpdds[1]->Restore();
		}
		/* 次回は全領域更新 */
		ReDrawDD();
		return;
	}

	/* ExtTextOutを使い、一度で描画 */
	SetBkColor(hDC, RGB(0, 0, 0));
	SetTextColor(hDC, RGB(255, 255, 255));
	if (nDDResolution == DDRES_480LINE) {
		rect.top = 0;
		rect.left = 0;
		rect.right = 640;
		rect.bottom = 40;
	}
	else if (nDDResolution == DDRES_SXGA) {
		rect.top = 0;
		rect.left = 0;
		rect.right = 640;
		rect.bottom = 56;
	}
	ExtTextOut(hDC, 0, 0, ETO_OPAQUE, &rect,
					 szCaption, strlen(szCaption), NULL);

	/* DCを解放 */
	hResult = lpdds[1]->ReleaseDC(hDC);
	if (hResult != DD_OK) {
		/* サーフェイスがロストしていれば、リストア */
		if (hResult == DDERR_SURFACELOST) {
			lpdds[1]->Restore();
		}
		/* 次回は全領域更新 */
		ReDrawDD();
		return;
	}

	/* Blt */
	if (nDDResolution == DDRES_480LINE) {
		drect.top = 0;
		drect.left = 0;
		drect.right = 640;
		drect.bottom = 40;
	}
	else if (nDDResolution == DDRES_SXGA) {
		drect.top = 0;
		drect.left = 0;
		drect.right = 1280;
		drect.bottom = 112;
	}
	hResult = lpdds[0]->Blt(&drect, lpdds[1], &rect, DDBLT_WAIT, NULL);
	if (hResult != DD_OK) {
		/* サーフェイスがロストしていれば、リストア */
		if (hResult == DDERR_SURFACELOST) {
			lpdds[0]->Restore();
			lpdds[1]->Restore();
		}
		/* 次回は全領域更新 */
		ReDrawDD();
		return;
	}
}

/*
 *	キャラクタ描画
 */
static void FASTCALL DrawChar(HDC hDC, BYTE c, int x, int y, int color)
{
	int i;
	int j;
	BYTE *p;
	BYTE dat;

	/* サブROM(C)のフォントアドレスを得る */
	p = &subrom_c[c * 8];

	/* yループ */
	for (i=0; i<8; i++) {
		/* データ取得 */
		dat = *p;
		p++;

		/* xループ */
		for (j=0; j<8; j++) {
			if (dat & 0x80) {
				SetPixelV(hDC, x, y, color);
			}
			else {
				SetPixelV(hDC, x, y, RGB(0, 0, 0));
			}
			dat <<= 1;
			x++;
		}

		/* 次のyへ */
		x -= 8;
		y++;
	}
}

/*
 *	ステータスライン(キャプション)
 */
static BOOL FASTCALL StatusCaption(void)
{
	char string[256];
	char tmp[256];
	char drive[_MAX_DRIVE];
	char dir[_MAX_DIR];
	char fname[_MAX_FNAME];
	char ext[_MAX_EXT];

	/* 動作状況に応じて、コピー */
	if (run_flag) {
		strncpy(string, szRunMessage, sizeof(string));
	}
	else {
		strncpy(string, szStopMessage, sizeof(string));
	}
	strncat(string, " ", sizeof(string) - strlen(string) - 1);

#if !defined(DISABLE_FULLSPEED)
	/* CPU速度比率 */
	if (bAutoSpeedAdjust) {
		_snprintf(tmp, sizeof(tmp), "(%3d%%) ", speed_ratio / 100);
		strncat(string, tmp, sizeof(string) - strlen(string) - 1);
	}
#endif

	/* フロッピーディスクドライブ 0 */
	if (fdc_ready[0] != FDC_TYPE_NOTREADY) {
		/* ファイルネーム＋拡張子のみ取り出す */
		_splitpath(fdc_fname[0], drive, dir, fname, ext);
		_snprintf(tmp, sizeof(tmp), "- %s%s ", fname, ext);
		strncat(string, tmp, sizeof(string) - strlen(string) - 1);
	}

	/* フロッピーディスクドライブ 1 */
	if (fdc_ready[1] != FDC_TYPE_NOTREADY) {
		if ((strcmp(fdc_fname[0], fdc_fname[1]) != 0) ||
			(fdc_ready[0] == FDC_TYPE_NOTREADY)) {
			/* ファイルネーム＋拡張子のみ取り出す */
			_splitpath(fdc_fname[1], drive, dir, fname, ext);
			if (fdc_ready[0] == FDC_TYPE_NOTREADY) {
				_snprintf(tmp, sizeof(tmp), "- (%s%s) ", fname, ext);
			}
			else {
				_snprintf(tmp, sizeof(tmp), "(%s%s) ", fname, ext);
			}
			strncat(string, tmp, sizeof(string) - strlen(string) - 1);
		}
	}

	/* テープ */
	if (tape_fileh != -1) {
		/* ファイルネーム＋拡張子のみ取り出す */
		_splitpath(tape_fname, drive, dir, fname, ext);
		_snprintf(tmp, sizeof(tmp), "- %s%s ", fname, ext);
		strncat(string, tmp, sizeof(string) - strlen(string) - 1);
	}

#if XM7_VER == 1 && defined(BUBBLE)
	/* バブルメモリ */
	if (fm_subtype == FMSUB_FM8) {
		if (bmc_ready[0] != BMC_TYPE_NOTREADY) {
			/* ファイルネーム＋拡張子のみ取り出す */
			_splitpath(bmc_fname[0], drive, dir, fname, ext);
			_snprintf(tmp, sizeof(tmp), "- %s%s ", fname, ext);
			strncat(string, tmp, sizeof(string) - strlen(string) - 1);
		}

		if (bmc_ready[1] != BMC_TYPE_NOTREADY) {
			/* ファイルネーム＋拡張子のみ取り出す */
			_splitpath(bmc_fname[1], drive, dir, fname, ext);
			if (bmc_ready[0] == BMC_TYPE_NOTREADY) {
				_snprintf(tmp, sizeof(tmp), "- (%s%s) ", fname, ext);
			}
			else {
				_snprintf(tmp, sizeof(tmp), "(%s%s) ", fname, ext);
			}
			strncat(string, tmp, sizeof(string) - strlen(string) - 1);
		}
	}
#endif

	/* 比較描画 */
	string[255] = '\0';
	if (memcmp(szCaption, string, strlen(string) + 1) != 0) {
		strncpy(szCaption, string, sizeof(szCaption));
		return TRUE;
	}

	/* 前回と同じなので、描画しなくてよい */
	return FALSE;
}

/*
 *	ステータスライン(CAP)
 */
static BOOL FASTCALL StatusCAP(void)
{
	HDC hDC;
	HRESULT hResult;
	RECT rect;
	RECT drect;
	int num;

	/* 値取得、比較 */
	if (caps_flag) {
		num = 1;
	}
	else {
		num = 0;
	}
	if (num == nCAP) {
		return FALSE;
	}

	/* DC取得 */
	hResult = lpdds[1]->GetDC(&hDC);
	if (hResult != DD_OK) {
		/* サーフェイスがロストしていれば、リストア */
		if (hResult == DDERR_SURFACELOST) {
			lpdds[1]->Restore();
		}
		/* 次回は全領域更新 */
		ReDrawDD();
		return FALSE;
	}

	/* -1なら、全領域クリア */
	if (nDDResolution == DDRES_480LINE) {
		rect.left = 0;
		rect.right = 640;
		rect.top = 440;
		rect.bottom = 480;
	}
	else {
		rect.left = 0;
		rect.right = 640;
		rect.top = 472;
		rect.bottom = 512;
	}
	drect = rect;
	SetBkColor(hDC, RGB(0, 0, 0));
	SetTextColor(hDC, RGB(255, 255, 255));
	if (nCAP == -1) {
		/* クリア */
		ExtTextOut(hDC, 0, 0, ETO_OPAQUE, &rect, NULL, 0, NULL);

		/* "CAP"の文字描画 */
		TextOut(hDC, 500, drect.top + 4, szCAPMessage, strlen(szCAPMessage));

		/* ワクを描画 */
		rect.left = 500;
		rect.right = rect.left + 30;
		rect.top = drect.top + 25;
		rect.bottom = rect.top + 10;
		FrameRect(hDC, &rect, (HBRUSH)GetStockObject(WHITE_BRUSH));
	}

	/* ここでコピー */
	nCAP = num;

	/* メイン色描画 */
	rect.left = 501;
	rect.right = rect.left + 28;
	rect.top = drect.top + 26;
	rect.bottom = rect.top + 8;
	if (nCAP == 1) {
		SetBkColor(hDC, RGB(255, 0, 0));
	}
	else {
		SetBkColor(hDC, RGB(0, 0, 0));
	}
	ExtTextOut(hDC, 0, 0, ETO_OPAQUE, &rect, NULL, 0, NULL);

	/* DC解放 */
	hResult = lpdds[1]->ReleaseDC(hDC);
	if (hResult != DD_OK) {
		/* サーフェイスがロストしていれば、リストア */
		if (hResult == DDERR_SURFACELOST) {
			lpdds[1]->Restore();
		}
		/* 次回は全領域更新 */
		ReDrawDD();
		return FALSE;
	}

	return TRUE;
}

/*
 *	ステータスライン(かな)
 */
static BOOL FASTCALL StatusKANA(void)
{
	HDC hDC;
	HRESULT hResult;
	RECT rect;
	RECT drect;
	int num;

	/* 値取得、比較 */
	if (kana_flag) {
		num = 1;
	}
	else {
		num = 0;
	}
	if (num == nKANA) {
		return FALSE;
	}

	/* DC取得 */
	hResult = lpdds[1]->GetDC(&hDC);
	if (hResult != DD_OK) {
		/* サーフェイスがロストしていれば、リストア */
		if (hResult == DDERR_SURFACELOST) {
			lpdds[1]->Restore();
		}
		/* 次回は全領域更新 */
		ReDrawDD();
		return FALSE;
	}

	if (nDDResolution == DDRES_480LINE) {
		rect.left = 0;
		rect.right = 640;
		rect.top = 440;
		rect.bottom = 480;
	}
	else {
		rect.left = 0;
		rect.right = 640;
		rect.top = 472;
		rect.bottom = 512;
	}
	drect = rect;
	SetBkColor(hDC, RGB(0, 0, 0));
	SetTextColor(hDC, RGB(255, 255, 255));
	if (nKANA == -1) {
		/* "かな"の文字描画 */
		TextOut(hDC, 546, drect.top + 4, szKANAMessage, strlen(szKANAMessage));

		/* ワクを描画 */
		rect.left = 546;
		rect.right = rect.left + 30;
		rect.top = drect.top + 25;
		rect.bottom = rect.top + 10;
		FrameRect(hDC, &rect, (HBRUSH)GetStockObject(WHITE_BRUSH));
	}

	/* ここでコピー */
	nKANA = num;

	/* メイン色描画 */
	rect.left = 547;
	rect.right = rect.left + 28;
	rect.top = drect.top + 26;
	rect.bottom = rect.top + 8;
	if (nKANA == 1) {
		SetBkColor(hDC, RGB(255, 0, 0));
	}
	else {
		SetBkColor(hDC, RGB(0, 0, 0));
	}
	ExtTextOut(hDC, 0, 0, ETO_OPAQUE, &rect, NULL, 0, NULL);

	/* DC解放 */
	hResult = lpdds[1]->ReleaseDC(hDC);
	if (hResult != DD_OK) {
		/* サーフェイスがロストしていれば、リストア */
		if (hResult == DDERR_SURFACELOST) {
			lpdds[1]->Restore();
		}
		/* 次回は全領域更新 */
		ReDrawDD();
		return FALSE;
	}

	return TRUE;
}

/*
 *	ステータスライン(INS)
 */
static BOOL FASTCALL StatusINS(void)
{
	HDC hDC;
	HRESULT hResult;
	RECT rect;
	RECT drect;
	int num;

	/* 値取得、比較 */
	if (ins_flag) {
		num = 1;
	}
	else {
		num = 0;
	}
	if (num == nINS) {
		return FALSE;
	}

	/* DC取得 */
	hResult = lpdds[1]->GetDC(&hDC);
	if (hResult != DD_OK) {
		/* サーフェイスがロストしていれば、リストア */
		if (hResult == DDERR_SURFACELOST) {
			lpdds[1]->Restore();
		}
		/* 次回は全領域更新 */
		ReDrawDD();
		return FALSE;
	}

	if (nDDResolution == DDRES_480LINE) {
		rect.left = 0;
		rect.right = 640;
		rect.top = 440;
		rect.bottom = 480;
	}
	else {
		rect.left = 0;
		rect.right = 640;
		rect.top = 472;
		rect.bottom = 512;
	}
	drect = rect;
	SetBkColor(hDC, RGB(0, 0, 0));
	SetTextColor(hDC, RGB(255, 255, 255));
	if (nINS == -1) {
		/* "INS"の文字描画 */
		TextOut(hDC, 595, drect.top + 4, szINSMessage, strlen(szINSMessage));

		/* ワクを描画 */
		rect.left = 593;
		rect.right = rect.left + 30;
		rect.top = drect.top + 25;
		rect.bottom = rect.top + 10;
		FrameRect(hDC, &rect, (HBRUSH)GetStockObject(WHITE_BRUSH));
	}

	/* ここでコピー */
	nINS = num;

	/* メイン色描画 */
	rect.left = 594;
	rect.right = rect.left + 28;
	rect.top = drect.top + 26;
	rect.bottom = rect.top + 8;
	if (nINS == 1) {
		SetBkColor(hDC, RGB(255, 0, 0));
	}
	else {
		SetBkColor(hDC, RGB(0, 0, 0));
	}
	ExtTextOut(hDC, 0, 0, ETO_OPAQUE, &rect, NULL, 0, NULL);

	/* DC解放 */
	hResult = lpdds[1]->ReleaseDC(hDC);
	if (hResult != DD_OK) {
		/* サーフェイスがロストしていれば、リストア */
		if (hResult == DDERR_SURFACELOST) {
			lpdds[1]->Restore();
		}
		/* 次回は全領域更新 */
		ReDrawDD();
		return FALSE;
	}

	return TRUE;
}

/*
 *	ステータスライン(フロッピードライブ)
 */
static BOOL FASTCALL StatusDrive(int drive)
{
	char string[_MAX_FNAME + _MAX_EXT];
	char buffer[_MAX_FNAME + _MAX_EXT];
	char drive_[_MAX_DRIVE];
	char dir[_MAX_DIR];
	char fname[_MAX_FNAME];
	char ext[_MAX_EXT];
	HDC hDC;
	HRESULT hResult;
	RECT rect, srect;
	int num;
	char *name;

	ASSERT((drive == 0) || (drive == 1));

	/* 番号セット */
	if (fdc_ready[drive] == FDC_TYPE_NOTREADY) {
		num = FDC_ACCESS_NOTREADY;
	}
	else {
		num = fdc_access[drive];
		if (num == FDC_ACCESS_SEEK) {
			num = FDC_ACCESS_READY;
		}
	}

	/* 名前取得 */
	name = "";
	if ((fdc_ready[drive] == FDC_TYPE_D77) &&
		(strlen(fdc_name[drive][ fdc_media[drive] ]) > 0)) {
		name = fdc_name[drive][ fdc_media[drive] ];
	}
	else if (fdc_ready[drive] != FDC_TYPE_NOTREADY) {
		/* ファイルネーム＋拡張子のみ取り出す */
		_splitpath(fdc_fname[drive], drive_, dir, fname, ext);
		strncpy(string, fname, sizeof(string));
		strncat(string, ext, sizeof(string) - strlen(string) - 1);
		if ((fdc_ready[drive] == FDC_TYPE_D77) && (fdc_medias[drive] > 1)) {
			_snprintf(buffer, sizeof(buffer), "%s #%02d", string, 
					  fdc_media[drive] + 1);
			name = buffer;
		}
		else {
			name = string;
		}
	}

	/* 番号比較 */
	if (nDrive[drive] == num) {
		if (strcmp(szDrive[drive], name) == 0) {
			return FALSE;
		}
	}

	/* DC取得 */
	hResult = lpdds[1]->GetDC(&hDC);
	if (hResult != DD_OK) {
		/* サーフェイスがロストしていれば、リストア */
		if (hResult == DDERR_SURFACELOST) {
			lpdds[1]->Restore();
		}
		/* 次回は全領域更新 */
		ReDrawDD();
		return FALSE;
	}

	/* ここでコピー */
	nDrive[drive] = num;
	strncpy(szDrive[drive], name, sizeof(szDrive[drive]));

	/* 座標設定 */
	SetBkColor(hDC, RGB(0, 0, 0));
	rect.left = (drive ^ 1) * 160;
	rect.right = ((drive ^ 1) + 1) * 160 - 4;
	if (nDDResolution == DDRES_480LINE) {
		rect.top = 444;
		rect.bottom = 474;
	}
	else {
		rect.top = 476;
		rect.bottom = 506;
	}
	srect.left = rect.left + 1;
	srect.right = rect.right + 1;
	srect.top = rect.top + 1;
	srect.bottom = rect.bottom + 1;

	/* 色決定 */
	if ((nDrive[drive] != FDC_ACCESS_NOTREADY) && (!fdc_teject[drive])) {
		SetBkColor(hDC, RGB(63, 63, 63));
	}
	if (nDrive[drive] == FDC_ACCESS_READ) {
		SetBkColor(hDC, RGB(191, 0, 0));
	}
	if (nDrive[drive] == FDC_ACCESS_WRITE) {
		SetBkColor(hDC, RGB(0, 0, 191));
	}

	/* 背景を塗りつぶす */
	ExtTextOut(hDC, 0, 0, ETO_OPAQUE, &rect, NULL, 0, NULL);

	/* DrawText */
	SetTextColor(hDC, RGB(0, 0, 0));
	SetBkMode(hDC, TRANSPARENT);
	DrawText(hDC, szDrive[drive], strlen(szDrive[drive]), &srect,
		DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);
	SetTextColor(hDC, RGB(255, 255, 255));
	SetBkMode(hDC, TRANSPARENT);
	DrawText(hDC, szDrive[drive], strlen(szDrive[drive]), &rect,
		DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);

	/* ドライブ番号描画 */
	if (fdc_ready[drive] != FDC_TYPE_NOTREADY) {
		DrawChar(hDC, (BYTE)(0x30 + drive), rect.right - 8, rect.bottom - 8,
				 RGB(255, 255, 255));
	}

	/* DC解放 */
	hResult = lpdds[1]->ReleaseDC(hDC);
	if (hResult != DD_OK) {
		/* サーフェイスがロストしていれば、リストア */
		if (hResult == DDERR_SURFACELOST) {
			lpdds[1]->Restore();
		}
		/* 次回は全領域更新 */
		ReDrawDD();
		return FALSE;
	}

	return TRUE;
}

/*
 *	ステータスライン(テープ)
 */
static BOOL FASTCALL StatusTape(void)
{
	HDC hDC;
	HRESULT hResult;
	RECT rect, srect;
	int num;
	char string[64];

	/* 番号セット */
	num = 30000;
	if (tape_fileh != -1) {
		num = (int)((tape_offset >> 8) % 10000);
		if (tape_motor) {
			if (tape_rec) {
				num += 20000;
			}
			else {
				num += 10000;
			}
		}
	}

	/* 番号比較 */
	if (nTape == num) {
		return FALSE;
	}

	/* DC取得 */
	hResult = lpdds[1]->GetDC(&hDC);
	if (hResult != DD_OK) {
		/* サーフェイスがロストしていれば、リストア */
		if (hResult == DDERR_SURFACELOST) {
			lpdds[1]->Restore();
		}
		/* 次回は全領域更新 */
		ReDrawDD();
		return FALSE;
	}

	/* ここでコピー */
	nTape = num;

	/* 座標設定 */
	SetBkColor(hDC, RGB(0, 0, 0));
	rect.left = 360;
	rect.right = rect.left + 80;
	if (nDDResolution == DDRES_480LINE) {
		rect.top = 444;
		rect.bottom = 474;
	}
	else {
		rect.top = 476;
		rect.bottom = 506;
	}
	srect.left = rect.left + 1;
	srect.right = rect.right + 1;
	srect.top = rect.top + 1;
	srect.bottom = rect.bottom + 1;

	/* 色、文字列決定 */
	if (nTape >= 30000) {
		string[0] = '\0';
	}
	else {
		_snprintf(string, sizeof(string), "%04d", nTape % 10000);
		if (nTape >= 10000) {
			if (nTape >= 20000) {
				SetBkColor(hDC, RGB(0, 0, 191));
			}
			else {
				SetBkColor(hDC, RGB(191, 0, 0));
			}
		}
		else {
			SetBkColor(hDC, RGB(63, 63, 63));
		}
	}

	/* 背景を塗りつぶす */
	ExtTextOut(hDC, 0, 0, ETO_OPAQUE, &rect, NULL, 0, NULL);

	/* DrawText */
	SetTextColor(hDC, RGB(0, 0, 0));
	SetBkMode(hDC, TRANSPARENT);
	DrawText(hDC, string, strlen(string), &srect,
		DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);
	SetTextColor(hDC, RGB(255, 255, 255));
	SetBkMode(hDC, TRANSPARENT);
	DrawText(hDC, string, strlen(string), &rect,
		DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);

	/* DC解放 */
	hResult = lpdds[1]->ReleaseDC(hDC);
	if (hResult != DD_OK) {
		/* サーフェイスがロストしていれば、リストア */
		if (hResult == DDERR_SURFACELOST) {
			lpdds[1]->Restore();
		}
		/* 次回は全領域更新 */
		ReDrawDD();
		return FALSE;
	}

	return TRUE;
}

/*
 *	ステータスライン描画
 */
static void FASTCALL StatusLine(void)
{
	BOOL flag;
	HRESULT hResult;
	RECT rect;
	RECT drect;

	/* 640x480/1280x1024、ステータスありの場合のみ */
	if ((nDDResolution == DDRES_400LINE || nDDResolution == DDRES_WUXGA) ||
		(nDDResolution == DDRES_WXGA800) || !bCaption) {
		return;
	}

	/* キャプション */
	if (StatusCaption()) {
		DrawCaption();
	}

	flag = FALSE;

	/* キーボードステータス */
	if (StatusCAP()) {
		flag = TRUE;
	}
	if (StatusKANA()) {
		flag = TRUE;
	}
	if (StatusINS()) {
		flag = TRUE;
	}
	if (StatusDrive(0)) {
		flag = TRUE;
	}
	if (StatusDrive(1)) {
		flag = TRUE;
	}
	if (StatusTape()) {
		flag = TRUE;
	}

	/* フラグが降りていれば、描画する必要なし */
	if (!flag) {
		return;
	}

	/* Blt */
	if (nDDResolution == DDRES_480LINE) {
		rect.left = 0;
		rect.right = 640;
		rect.top = 440;
		rect.bottom = 480;
		drect = rect;
	}
	else {
		rect.left = 0;
		rect.right = 640;
		rect.top = 456;
		rect.bottom = 512;
		drect.left = rect.left * 2;
		drect.right = rect.right * 2;
		drect.top = rect.top * 2;
		drect.bottom = rect.bottom * 2;
	}
	hResult = lpdds[0]->Blt(&drect, lpdds[1], &rect, DDBLT_WAIT, NULL);
	if (hResult != DD_OK) {
		/* サーフェイスがロストしていれば、リストア */
		if (hResult == DDERR_SURFACELOST) {
			lpdds[0]->Restore();
			lpdds[1]->Restore();
		}
		/* 次回は全領域更新 */
		ReDrawDD();
		return;
	}
}

/*
 *	640x200、デジタルモード
 *	パレット設定
 */
static void FASTCALL Palet640(void)
{
	int i;
	int vpage;
#if XM7_VER == 1
	int base;
#endif
	BYTE col;

	/* HighColor用パレットテーブル */
	static const WORD rgbTable[] = {
		/* nPixelFormat = 1 */
		0x0000 | 0x0000 | 0x0000,
		0x0000 | 0x0000 | 0x001f,
		0xf800 | 0x0000 | 0x0000,
		0xf800 | 0x0000 | 0x001f,
		0x0000 | 0x07e0 | 0x0000,
		0x0000 | 0x07e0 | 0x001f,
		0xf800 | 0x07e0 | 0x0000,
		0xf800 | 0x07e0 | 0x001f,

		/* nPixelFormat = 2 */
		0x0000 | 0x0000 | 0x0000,
		0x0000 | 0x0000 | 0x001f,
		0x7c00 | 0x0000 | 0x0000,
		0x7c00 | 0x0000 | 0x001f,
		0x0000 | 0x03e0 | 0x0000,
		0x0000 | 0x03e0 | 0x001f,
		0x7c00 | 0x03e0 | 0x0000,
		0x7c00 | 0x03e0 | 0x001f,

		/* nPixelFormat = 1, Green Monitor */
		0x0000 | 0x0000 | 0x0000,
		0x0000 | 0x00c0 | 0x0000,
		0x0000 | 0x0240 | 0x0000,
		0x0000 | 0x0320 | 0x0000,
		0x0000 | 0x04a0 | 0x0000,
		0x0000 | 0x0580 | 0x0000,
		0x0000 | 0x0700 | 0x0000,
		0x0000 | 0x07e0 | 0x0000,

		/* nPixelFormat = 2, Green Monitor */
		0x0000 | 0x0000 | 0x0000,
		0x0000 | 0x0060 | 0x0000,
		0x0000 | 0x0120 | 0x0000,
		0x0000 | 0x0180 | 0x0000,
		0x0000 | 0x0240 | 0x0000,
		0x0000 | 0x02a0 | 0x0000,
		0x0000 | 0x0360 | 0x0000,
		0x0000 | 0x03e0 | 0x0000
	};

	/* TrueColor用パレットテーブル */
	static DWORD rgbTableTc[] = {
		/* nPixelFormat = 3 (TrueColor) */
		0x00000000,	0x000000ff,	0x00ff0000,	0x00ff00ff,
		0x0000ff00,	0x0000ffff,	0x00ffff00,	0x00ffffff,
		0x00000000, 0x00001c00, 0x00004c00, 0x00006800,
		0x00009600, 0x0000b200, 0x0000e200, 0x0000ff00
	};

	/* フラグがセットされていなければ、何もしない */
	if (!bPaletFlag) {
		return;
	}

	/* マルチページより、表示プレーン情報を得る */
	vpage = (~(multi_page >> 4)) & 0x07;

	/* 640x200、デジタルパレット */
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
			if (bGreenMonitor) {
				if (nPixelFormat == 1) {
					base = 16;
				}
				else if (nPixelFormat == 2) {
					base = 24;
				}
				else {
					base = 8;
				}
			}
			else {
				if (nPixelFormat == 1) {
					base = 0;
				}
				else if (nPixelFormat == 2) {
					base = 8;
				}
				else {
					base = 0;
				}
			}

			if (bPseudo400Line) {
				if (nPixelFormat == 3) {
					if (col & 2) {
						rgbTTLDD[i + 8] = rgbTableTc[7 + base];
					}
					else {
						rgbTTLDD[i + 8] = rgbTableTc[0 + base];
					}
					if (col & 4) {
						rgbTTLDD[i] = rgbTableTc[7 + base];
					}
					else {
						rgbTTLDD[i] = rgbTableTc[0 + base];
					}
				}
				else {
					if (col & 2) {
						rgbTTLDD[i + 8] = rgbTable[7 + base];
					}
					else {
						rgbTTLDD[i + 8] = rgbTable[0 + base];
					}
					if (col & 4) {
						rgbTTLDD[i] = rgbTable[7 + base];
					}
					else {
						rgbTTLDD[i] = rgbTable[0 + base];
					}
				}
			}
			else {
				if (bGreenMonitor) {
					if (nPixelFormat == 1) {
						rgbTTLDD[i] = rgbTable[col + 16];
					}
					else if (nPixelFormat == 2) {
						rgbTTLDD[i] = rgbTable[col + 24];
					}
					else {
						rgbTTLDD[i] = rgbTableTc[col + 8];
					}
				}
				else {
					if (nPixelFormat == 1) {
						rgbTTLDD[i] = rgbTable[col + 0];
					}
					else if (nPixelFormat == 2) {
						rgbTTLDD[i] = rgbTable[col + 8];
					}
					else {
						rgbTTLDD[i] = rgbTableTc[col];
					}
				}
			}
#else
			if (nPixelFormat == 1) {
				rgbTTLDD[i] = rgbTable[col + 0];
			}
			else if (nPixelFormat == 2) {
				rgbTTLDD[i] = rgbTable[col + 8];
			}
			else {
				rgbTTLDD[i] = rgbTableTc[col];
			}
#endif
		}
		else {
			/* CRT OFF */
			rgbTTLDD[i] = 0;
		}
	}

	/* フラグ降ろす */
	bPaletFlag = FALSE;
}

/*
 *	640x200、デジタルモード
 *	描画
 */
static void FASTCALL Draw640(void)
{
	DDSURFACEDESC ddsd;
	HRESULT hResult;
	RECT rect;
	RECT drect;
	BYTE *p;
#if XM7_VER >= 3
	WORD wdtop,wdbtm;
#endif

	/* オールクリア */
	AllClear(TRUE);

	/* パレット設定 */
	Palet640();

	/* ステータスライン */
	StatusLine();

	/* レンダリングチェック */
	if ((nDrawTop >= nDrawBottom) || (nDrawLeft >= nDrawRight)) {
		return;
	}

	/* サーフェイスをロック */
	memset(&ddsd, 0, sizeof(ddsd));
	ddsd.dwSize = sizeof(ddsd);
	hResult = lpdds[1]->Lock(NULL, &ddsd,
							 DDLOCK_WAIT | DDLOCK_SURFACEMEMORYPTR, NULL);
	if (hResult != DD_OK) {
		/* サーフェイスがロストしていれば、リストア */
		if (hResult == DDERR_SURFACELOST) {
			lpdds[1]->Restore();
		}
		/* 次回は全領域更新 */
		ReDrawDD();
		return;
	}

	/* レンダリング */
	p = (BYTE *)ddsd.lpSurface;
	if (nDDResolution == DDRES_480LINE) {
		p += (ddsd.lPitch * 40);
	}
	else if (nDDResolution == DDRES_SXGA) {
		p += (ddsd.lPitch * 56);
	}

#if XM7_VER >= 3
	/* ウィンドウオープン時 */
	if (window_open) {
		/* ウィンドウ外 上側の描画 */
		if ((nDrawTop >> 1) < window_dy1) {
			if (bPseudo400Line) {
				Render640c(p, ddsd.lPitch, nDrawTop >> 1, window_dy1);
			}
			else {
				Render640(p, ddsd.lPitch, nDrawTop >> 1, window_dy1, 2);
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
				Render640cw(p, ddsd.lPitch,
					wdtop, wdbtm, window_dx1, window_dx2);
			}
			else {
				Render640w(p, ddsd.lPitch,
					wdtop, wdbtm, window_dx1, window_dx2, 2);
			}
		}

		/* ウィンドウ外 下側の描画 */
		if ((nDrawBottom >> 1) > window_dy2) {
			if (bPseudo400Line) {
				Render640c(p, ddsd.lPitch, window_dy2, nDrawBottom >> 1);
			}
			else {
				Render640(p, ddsd.lPitch, window_dy2, nDrawBottom >> 1, 2);
			}
		}
	}
	else {
		if (bPseudo400Line) {
			Render640c(p, ddsd.lPitch, nDrawTop >> 1, nDrawBottom >> 1);
		}
		else {
			Render640(p, ddsd.lPitch, nDrawTop >> 1, nDrawBottom >> 1, 2);
		}
	}
	if (!bPseudo400Line) {
		RenderFullScan(p, ddsd.lPitch);
	}
#elif XM7_VER == 2
	if (bPseudo400Line) {
		Render640c(p, ddsd.lPitch, nDrawTop >> 1, nDrawBottom >> 1);
	}
	else {
		Render640(p, ddsd.lPitch, nDrawTop >> 1, nDrawBottom >> 1);
	}
#if defined(FMTV151)
	DrawV2_DD(p, ddsd.lPitch);
#endif
	if (!bPseudo400Line) {
		RenderFullScan(p, ddsd.lPitch);
	}
#else
	if (bPseudo400Line) {
		Render640m(p, ddsd.lPitch, nDrawTop >> 1, nDrawBottom >> 1);
	}
	else {
		Render640(p, ddsd.lPitch, nDrawTop >> 1, nDrawBottom >> 1);
		RenderFullScan(p, ddsd.lPitch);
	}
#endif

	/* サーフェイスをアンロック */
	lpdds[1]->Unlock(ddsd.lpSurface);

	/* Blt */
	rect.top = nDrawTop;
	rect.left = nDrawLeft;
	rect.right = nDrawRight;
	rect.bottom = nDrawBottom;
	drect = rect;
	if (nDDResolution == DDRES_480LINE) {
		rect.top += 40;
		rect.bottom += 40;
		drect.top += 40;
		drect.bottom += 40;
	}
	else if (nDDResolution == DDRES_WUXGA) {
		drect.top *= 3;
		drect.bottom *= 3;
		drect.left *= 3;
		drect.right *= 3;
	}
	else if (nDDResolution == DDRES_SXGA) {
		rect.top += 56;
		rect.bottom += 56;
		drect.left *= 2;
		drect.right *= 2;
		drect.top = rect.top * 2;
		drect.bottom = rect.bottom * 2;
	}
	else if (nDDResolution == DDRES_WXGA800) {
		drect.top *= 2;
		drect.bottom *= 2;
		drect.left *= 2;
		drect.right *= 2;
	}
	hResult = lpdds[0]->Blt(&drect, lpdds[1], &rect, DDBLT_WAIT, NULL);
	if (hResult != DD_OK) {
		/* サーフェイスがロストしていれば、リストア */
		if (hResult == DDERR_SURFACELOST) {
			lpdds[0]->Restore();
			lpdds[1]->Restore();
		}
		/* 次回は全領域更新 */
		ReDrawDD();
		return;
	}

	/* 次回に備え、ワークリセット */
	nDrawTop = 400;
	nDrawBottom = 0;
	nDrawLeft = 640;
	nDrawRight = 0;
	SetDrawFlag(FALSE);
}

#if XM7_VER >= 3
/*
 *	640x400、デジタルモード
 *	描画
 */
static void FASTCALL Draw400l(void)
{
	DDSURFACEDESC ddsd;
	HRESULT hResult;
	RECT rect;
	RECT drect;
	BYTE *p;
	WORD wdtop,wdbtm;

	/* オールクリア */
	AllClear(TRUE);

	/* パレット設定 */
	Palet640();

	/* ステータスライン */
	StatusLine();

	/* レンダリングチェック */
	if ((nDrawTop >= nDrawBottom) || (nDrawLeft >= nDrawRight)) {
		return;
	}

	/* サーフェイスをロック */
	memset(&ddsd, 0, sizeof(ddsd));
	ddsd.dwSize = sizeof(ddsd);
	hResult = lpdds[1]->Lock(NULL, &ddsd,
							 DDLOCK_WAIT | DDLOCK_SURFACEMEMORYPTR, NULL);
	if (hResult != DD_OK) {
		/* サーフェイスがロストしていれば、リストア */
		if (hResult == DDERR_SURFACELOST) {
			lpdds[1]->Restore();
		}
		/* 次回は全領域更新 */
		ReDrawDD();
		return;
	}

	/* レンダリング */
	p = (BYTE *)ddsd.lpSurface;
	if (nDDResolution == DDRES_480LINE) {
		p += (ddsd.lPitch * 40);
	}
	else if (nDDResolution == DDRES_SXGA) {
		p += (ddsd.lPitch * 56);
	}

	/* ウィンドウオープン時 */
	if (window_open) {
		if (nDrawTop < window_dy1) {
			Render640(p, ddsd.lPitch, nDrawTop, window_dy1, 1);
		}

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
			Render640w(p, ddsd.lPitch, wdtop, wdbtm, window_dx1, window_dx2, 1);
		}
		if (nDrawBottom > window_dy2) {
			Render640(p, ddsd.lPitch, window_dy2, nDrawBottom, 1);
		}
	}
	else {
		Render640(p, ddsd.lPitch, nDrawTop, nDrawBottom, 1);
	}

	/* サーフェイスをアンロック */
	lpdds[1]->Unlock(ddsd.lpSurface);

	/* Blt */
	rect.top = nDrawTop;
	rect.left = nDrawLeft;
	rect.right = nDrawRight;
	rect.bottom = nDrawBottom;
	drect = rect;
	if (nDDResolution == DDRES_480LINE) {
		rect.top += 40;
		rect.bottom += 40;
		drect.top += 40;
		drect.bottom += 40;
	}
	else if (nDDResolution == DDRES_WUXGA) {
		drect.top *= 3;
		drect.bottom *= 3;
		drect.left *= 3;
		drect.right *= 3;
	}
	else if (nDDResolution == DDRES_SXGA) {
		rect.top += 56;
		rect.bottom += 56;
		drect.left *= 2;
		drect.right *= 2;
		drect.top = rect.top * 2;
		drect.bottom = rect.bottom * 2;
	}
	else if (nDDResolution == DDRES_WXGA800) {
		drect.top *= 2;
		drect.bottom *= 2;
		drect.left *= 2;
		drect.right *= 2;
	}
	hResult = lpdds[0]->Blt(&drect, lpdds[1], &rect, DDBLT_WAIT, NULL);
	if (hResult != DD_OK) {
		/* サーフェイスがロストしていれば、リストア */
		if (hResult == DDERR_SURFACELOST) {
			lpdds[0]->Restore();
			lpdds[1]->Restore();
		}
		/* 次回は全領域更新 */
		ReDrawDD();
		return;
	}

	/* 次回に備え、ワークリセット */
	nDrawTop = 400;
	nDrawBottom = 0;
	nDrawLeft = 640;
	nDrawRight = 0;
	SetDrawFlag(FALSE);
}
#endif

#if XM7_VER == 1 && defined(L4CARD)
/*
 *	640x400、単色モード
 *	パレット設定
 */
static void FASTCALL PaletL4(void)
{
	int i;
	int base;

	/* フラグがセットされていなければ、何もしない */
	if (!bPaletFlag) {
		return;
	}

	if (nPixelFormat == 3) {
		/* 640x400、グラフィックパレット */
		for (i=0; i<3; i++) {
			/* パレット開始位置を設定 */
			if (bGreenMonitor) {
				base = 16;
			}
			else {
				base = 0;
			}

			if (crt_flag) {
				if ((i == 0) || (multi_page & (0x08 << i))) {
					rgbTTLL4DD[i] = (DWORD)Palet16Tc_L4[(ttl_palet[0] & 0x0f) + base];
				}
				else {
					rgbTTLL4DD[i] = (DWORD)Palet16Tc_L4[(ttl_palet[1] & 0x0f) + base];
				}
			}
			else {
				rgbTTLL4DD[i] = (DWORD)Palet16Tc_L4[base];
			}
		}
	}
	else {
		/* パレット開始位置を設定 */
		if (bGreenMonitor) {
			if (nPixelFormat == 1) {
				base = 32;
			}
			else {
				base = 48;
			}
		}
		else {
			if (nPixelFormat == 1) {
				base = 0;
			}
			else {
				base = 16;
			}
		}

		/* 640x400、グラフィックパレット */
		for (i=0; i<3; i++) {
			if (crt_flag) {
				if ((i == 0) || (multi_page & (0x08 << i))) {
					rgbTTLL4DD[i] = (DWORD)Palet16_L4[(ttl_palet[0] & 0x0f) + base];
				}
				else {
					rgbTTLL4DD[i] = (DWORD)Palet16_L4[(ttl_palet[1] & 0x0f) + base];
				}
			}
			else {
				rgbTTLL4DD[i] = (DWORD)Palet16_L4[base];
			}
		}
	}

	/* フラグを降ろす */
	bPaletFlag = FALSE;
}

/*
 *	テキスト画面
 *	レンダリング
 */
static void FASTCALL RenderTextDD(BYTE *p, DWORD pitch,
						int first, int last, int left, int right)
{
	DWORD	tmp;
	int		x, y, x2;
	WORD	addr;
	DWORD	col;
	BYTE	cursor_start, cursor_end, cursor_type;
	BYTE	raster, lines, maxx;
	BYTE	chr, atr, chr_dat;
	BYTE	line, line_old;
	int		xsize;

	/* 表示OFFなら何もしない */
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

	if (width40_flag) {
		xsize = 16;
	}
	else {
		xsize = 8;
	}

	for (x=left / xsize; x<right / xsize; x++) {
		line_old = 255;
		for (y=first; y<last; y++) {
			raster = (BYTE)(y % lines);
			line = (BYTE)(y / lines);
			if (line != line_old) {
				addr = (WORD)((text_start_addr + (line * maxx + x) * 2) & 0x0ffe);
				chr = tvram_c[addr + 0];
				atr = tvram_c[addr + 1];

				if (atr & 0x20) {
					/* Intensity attribute ON */
					if (bGreenMonitor) {
						if (nPixelFormat == 3) {
							col = Palet16Tc_L4[(atr & 0x07) + 24];
						}
						else if (nPixelFormat == 1) {
							col = Palet16_L4[(atr & 0x07) + 40];
						}
						else {
							col = Palet16_L4[(atr & 0x07) + 56];
						}
					}
					else {
						if (nPixelFormat == 3) {
							col = Palet16Tc_L4[(atr & 0x07) + 8];
						}
						else if (nPixelFormat == 1) {
							col = Palet16_L4[(atr & 0x07) + 8];
						}
						else {
							col = Palet16_L4[(atr & 0x07) + 24];
						}
					}
				}
				else {
					/* Intensity attribute OFF */
					if (bGreenMonitor) {
						if (nPixelFormat == 3) {
							col = Palet16Tc_L4[(atr & 0x07) + 16];
						}
						else if (nPixelFormat == 1) {
							col = Palet16_L4[(atr & 0x07) + 32];
						}
						else {
							col = Palet16_L4[(atr & 0x07) + 48];
						}
					}
					else {
						if (nPixelFormat == 3) {
							col = Palet16Tc_L4[(atr & 0x07) + 0];
						}
						else if (nPixelFormat == 1) {
							col = Palet16_L4[(atr & 0x07) + 0];
						}
						else {
							col = Palet16_L4[(atr & 0x07) + 16];
						}
					}
				}
			}

			/* Blink attribute */
			if ((!(atr & 0x10) || text_blink) && (raster < 16)) {
				chr_dat = subcg_l4[(WORD)(chr * 16) + raster];
			}
			else {
				chr_dat = 0x00;
			}

			/* Reverse attribute */
			if (atr & 0x08) {
				chr_dat = (BYTE)~chr_dat;
			}

			/* ほんでもって、かーそる */
			if ((addr == cursor_addr) && (cursor_type != 1) &&
				(cursor_blink || (cursor_type == 0))) {
				if ((raster >= cursor_start) && (raster <= cursor_end)) {
					chr_dat = (BYTE)~chr_dat;
				}
			}

			if (chr_dat) {
				for (x2=0; x2<8; x2++) {
					if (chr_dat & (1 << (7-x2))) {
						if (nPixelFormat == 3) {
							if (width40_flag) {
								tmp = y * pitch + (((x << 3) + x2) << 3);
								*(DWORD *)(&p[tmp + 0]) = col;
								*(DWORD *)(&p[tmp + 4]) = col;
							}
							else {
								tmp = y * pitch + (((x << 3) + x2) << 2);
								*(DWORD *)(&p[tmp + 0]) = col;
							}
						}
						else {
							if (width40_flag) {
								tmp = y * pitch + (((x << 3) + x2) << 2);
								*(WORD *)(&p[tmp + 0]) = (WORD)col;
								*(WORD *)(&p[tmp + 2]) = (WORD)col;
							}
							else {
								tmp = y * pitch + (((x << 3) + x2) << 1);
								*(WORD *)(&p[tmp + 0]) = (WORD)col;
							}
						}
					}
				}
			}

			line_old = line;
		}
	}
}

/*
 *	640x400、単色モード
 *	描画
 */
static void FASTCALL DrawL4(void)
{
	DDSURFACEDESC ddsd;
	HRESULT hResult;
	RECT rect;
	RECT drect;
	BYTE *p;

	/* オールクリア */
	AllClear(TRUE);

	/* パレット設定 */
	PaletL4();

	/* ステータスライン */
	StatusLine();

	/* レンダリングチェック */
	if ((nDrawTop >= nDrawBottom) || (nDrawLeft >= nDrawRight)) {
		return;
	}

	/* サーフェイスをロック */
	memset(&ddsd, 0, sizeof(ddsd));
	ddsd.dwSize = sizeof(ddsd);
	hResult = lpdds[1]->Lock(NULL, &ddsd,
							 DDLOCK_WAIT | DDLOCK_SURFACEMEMORYPTR, NULL);
	if (hResult != DD_OK) {
		/* サーフェイスがロストしていれば、リストア */
		if (hResult == DDERR_SURFACELOST) {
			lpdds[1]->Restore();
		}
		/* 次回は全領域更新 */
		ReDrawDD();
		return;
	}

	/* レンダリング */
	p = (BYTE *)ddsd.lpSurface;
	if (nDDResolution == DDRES_480LINE) {
		p += (ddsd.lPitch * 40);
	}
	else if (nDDResolution == DDRES_SXGA) {
		p += (ddsd.lPitch * 56);
	}

	RenderL4(p, ddsd.lPitch, nDrawTop, nDrawBottom);
	RenderTextDD(p, ddsd.lPitch, nDrawTop, nDrawBottom, nDrawLeft, nDrawRight);

	/* サーフェイスをアンロック */
	lpdds[1]->Unlock(ddsd.lpSurface);

	/* Blt */
	rect.top = nDrawTop;
	rect.left = nDrawLeft;
	rect.right = nDrawRight;
	rect.bottom = nDrawBottom;
	drect = rect;
	if (nDDResolution == DDRES_480LINE) {
		rect.top += 40;
		rect.bottom += 40;
		drect.top += 40;
		drect.bottom += 40;
	}
	else if (nDDResolution == DDRES_WUXGA) {
		drect.top *= 3;
		drect.bottom *= 3;
		drect.left *= 3;
		drect.right *= 3;
	}
	else if (nDDResolution == DDRES_SXGA) {
		rect.top += 56;
		rect.bottom += 56;
		drect.left *= 2;
		drect.right *= 2;
		drect.top = rect.top * 2;
		drect.bottom = rect.bottom * 2;
	}
	else if (nDDResolution == DDRES_WXGA800) {
		drect.top *= 2;
		drect.bottom *= 2;
		drect.left *= 2;
		drect.right *= 2;
	}
	hResult = lpdds[0]->Blt(&drect, lpdds[1], &rect, DDBLT_WAIT, NULL);
	if (hResult != DD_OK) {
		/* サーフェイスがロストしていれば、リストア */
		if (hResult == DDERR_SURFACELOST) {
			lpdds[0]->Restore();
			lpdds[1]->Restore();
		}
		/* 次回は全領域更新 */
		ReDrawDD();
		return;
	}

	/* 次回に備え、ワークリセット */
	nDrawTop = 400;
	nDrawBottom = 0;
	nDrawLeft = 640;
	nDrawRight = 0;
	SetDrawFlag(FALSE);
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
	DWORD r, g, b;
	int amask;
	WORD color;

	/* フラグがセットされていなければ、何もしない */
	if (!bPaletFlag) {
		return;
	}

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
			r = (WORD)apalet_r[j];
			g = (WORD)apalet_g[j];
			b = (WORD)apalet_b[j];
		}
		else {
			r = 0;
			g = 0;
			b = 0;
		}

		/* ピクセルタイプに応じ、DWORDデータを作成 */
		if (nPixelFormat >= 3) {
			/* R8bit, G8bit, B8bit (TrueColor) */
			r = (DWORD)((r << 20) | (r << 16));
			g = (DWORD)((g << 12) | (g <<  8));
			b = (DWORD)((b <<  4) | (b      ));

			/* セット */
			rgbAnalogDD[i] = (DWORD)(r | g | b);
		}
		else {
			if (nPixelFormat == 1) {
				/* R5bit, G6bit, B5bitタイプ */
				r <<= 12;
				if (r > 0) {
					r |= 0x0800;
				}

				g <<= 7;
				if (g > 0) {
					g |= 0x0060;
				}

				b <<= 1;
				if (b > 0) {
					b |= 0x0001;
				}
			}
			else if (nPixelFormat == 2) {
				/* R5bit, G5bit, B5bitタイプ */
				r <<= 11;
				if (r > 0) {
					r |= 0x0400;
				}

				g <<= 6;
				if (g > 0) {
					g |= 0x0020;
				}

				b <<= 1;
				if (b > 0) {
					b |= 0x0001;
				}
			}

			/* セット */
			color = (WORD)(r | g | b);
			rgbAnalogDD[i] = (DWORD)((color << 16) | color);
		}
	}

	/* フラグ降ろす */
	bPaletFlag = FALSE;
}

/*
 *	320x200、アナログモード
 *	描画
 */
static void FASTCALL Draw320(void)
{
	DDSURFACEDESC ddsd;
	HRESULT hResult;
	RECT rect;
	RECT drect;
	BYTE *p;
#if XM7_VER >= 3
	WORD wdtop,wdbtm;
#endif

	/* オールクリア */
	AllClear(TRUE);

	/* パレット設定 */
	Palet320();

	/* ステータスライン */
	StatusLine();

	/* レンダリングチェック */
	if ((nDrawTop >= nDrawBottom) || (nDrawLeft >= nDrawRight)) {
		return;
	}

	/* サーフェイスをロック */
	memset(&ddsd, 0, sizeof(ddsd));
	ddsd.dwSize = sizeof(ddsd);
	hResult = lpdds[1]->Lock(NULL, &ddsd,
							 DDLOCK_WAIT | DDLOCK_SURFACEMEMORYPTR, NULL);
	if (hResult != DD_OK) {
		/* サーフェイスがロストしていれば、リストア */
		if (hResult == DDERR_SURFACELOST) {
			lpdds[1]->Restore();
		}
		/* 次回は全領域更新 */
		ReDrawDD();
		return;
	}

	/* レンダリング */
	p = (BYTE *)ddsd.lpSurface;
	if (nDDResolution == DDRES_480LINE) {
		p += (ddsd.lPitch * 40);
	}
	else if (nDDResolution == DDRES_SXGA) {
		p += (ddsd.lPitch * 56);
	}

#if XM7_VER >= 3
	/* ウィンドウオープン時 */
	if (window_open) {
		/* ウィンドウ外 上側の描画 */
		if ((nDrawTop >> 1) < window_dy1) {
			Render320(p, ddsd.lPitch, nDrawTop >> 1, window_dy1);
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
			Render320w(p, ddsd.lPitch, wdtop, wdbtm, window_dx1, window_dx2);
		}

		/* ウィンドウ外 下側の描画 */
		if ((nDrawBottom >> 1) > window_dy2) {
			Render320(p, ddsd.lPitch, window_dy2, nDrawBottom >> 1);
		}
	}
	else {
		Render320(p, ddsd.lPitch, nDrawTop>>1, nDrawBottom>>1);
	}
	RenderFullScan(p, ddsd.lPitch);
#else
	Render320(p, ddsd.lPitch, nDrawTop>>1, nDrawBottom>>1);
#if defined(FMTV151)
	DrawV2_DD(p, ddsd.lPitch);
#endif
	RenderFullScan(p, ddsd.lPitch);
#endif

	/* サーフェイスをアンロック */
	lpdds[1]->Unlock(ddsd.lpSurface);

	/* Blt */
	rect.top = nDrawTop;
	rect.left = nDrawLeft;
	rect.right = nDrawRight;
	rect.bottom = nDrawBottom;
	drect = rect;
	if (nDDResolution == DDRES_480LINE) {
		rect.top += 40;
		rect.bottom += 40;
		drect.top += 40;
		drect.bottom += 40;
	}
	else if (nDDResolution == DDRES_WUXGA) {
		drect.top *= 3;
		drect.bottom *= 3;
		drect.left *= 3;
		drect.right *= 3;
	}
	else if (nDDResolution == DDRES_SXGA) {
		rect.top += 56;
		rect.bottom += 56;
		drect.left *= 2;
		drect.right *= 2;
		drect.top = rect.top * 2;
		drect.bottom = rect.bottom * 2;
	}
	else if (nDDResolution == DDRES_WXGA800) {
		drect.top *= 2;
		drect.bottom *= 2;
		drect.left *= 2;
		drect.right *= 2;
	}
	hResult = lpdds[0]->Blt(&drect, lpdds[1], &rect, DDBLT_WAIT, NULL);
	if (hResult != DD_OK) {
		/* サーフェイスがロストしていれば、リストア */
		if (hResult == DDERR_SURFACELOST) {
			lpdds[0]->Restore();
			lpdds[1]->Restore();
		}
		/* 次回は全領域更新 */
		ReDrawDD();
		return;
	}

	/* 次回に備え、ワークリセット */
	nDrawTop = 400;
	nDrawBottom = 0;
	nDrawLeft = 640;
	nDrawRight = 0;
	SetDrawFlag(FALSE);
}
#endif

#if XM7_VER >= 3
/*
 *	320x200、26万色モード
 *	描画
 */
static void FASTCALL Draw256k(void)
{
	DDSURFACEDESC ddsd;
	HRESULT hResult;
	RECT rect;
	RECT drect;
	BYTE *p;

	/* オールクリア */
	AllClear(TRUE);

	/* ステータスライン */
	StatusLine();

	/* レンダリングチェック */
	if ((nDrawTop >= nDrawBottom) || (nDrawLeft >= nDrawRight)) {
		return;
	}

	/* サーフェイスをロック */
	memset(&ddsd, 0, sizeof(ddsd));
	ddsd.dwSize = sizeof(ddsd);
	hResult = lpdds[1]->Lock(NULL, &ddsd,
							 DDLOCK_WAIT | DDLOCK_SURFACEMEMORYPTR, NULL);
	if (hResult != DD_OK) {
		/* サーフェイスがロストしていれば、リストア */
		if (hResult == DDERR_SURFACELOST) {
			lpdds[1]->Restore();
		}
		/* 次回は全領域更新 */
		ReDrawDD();
		return;
	}

	/* レンダリング */
	p = (BYTE *)ddsd.lpSurface;
	if (nDDResolution == DDRES_480LINE) {
		p += (ddsd.lPitch * 40);
	}
	else if (nDDResolution == DDRES_SXGA) {
		p += (ddsd.lPitch * 56);
	}

	if (crt_flag) {
		Render256k(p, ddsd.lPitch, nDrawTop >> 1, nDrawBottom >> 1, multi_page);
	}
	else {
		Render256k(p, ddsd.lPitch, nDrawTop >> 1, nDrawBottom >> 1, 0xff);
	}
	RenderFullScan(p, ddsd.lPitch);

	/* サーフェイスをアンロック */
	lpdds[1]->Unlock(ddsd.lpSurface);

	/* Blt */
	rect.top = nDrawTop;
	rect.left = nDrawLeft;
	rect.right = nDrawRight;
	rect.bottom = nDrawBottom;
	drect = rect;
	if (nDDResolution == DDRES_480LINE) {
		rect.top += 40;
		rect.bottom += 40;
		drect.top += 40;
		drect.bottom += 40;
	}
	else if (nDDResolution == DDRES_WUXGA) {
		drect.top *= 3;
		drect.bottom *= 3;
		drect.left *= 3;
		drect.right *= 3;
	}
	else if (nDDResolution == DDRES_SXGA) {
		rect.top += 56;
		rect.bottom += 56;
		drect.left *= 2;
		drect.right *= 2;
		drect.top = rect.top * 2;
		drect.bottom = rect.bottom * 2;
	}
	else if (nDDResolution == DDRES_WXGA800) {
		drect.top *= 2;
		drect.bottom *= 2;
		drect.left *= 2;
		drect.right *= 2;
	}
	hResult = lpdds[0]->Blt(&drect, lpdds[1], &rect, DDBLT_WAIT, NULL);
	if (hResult != DD_OK) {
		/* サーフェイスがロストしていれば、リストア */
		if (hResult == DDERR_SURFACELOST) {
			lpdds[0]->Restore();
			lpdds[1]->Restore();
		}
		/* 次回は全領域更新 */
		ReDrawDD();
		return;
	}

	/* 次回に備え、ワークリセット */
	nDrawTop = 400;
	nDrawBottom = 0;
	nDrawLeft = 640;
	nDrawRight = 0;
	SetDrawFlag(FALSE);
}
#endif

/*
 *	描画チェック
 */
#if XM7_VER >= 2 || (XM7_VER == 1 && defined(L4CARD))
static void FASTCALL DrawDDCheck(void)
{
#if XM7_VER >= 3
	/* 限りない手抜き(ォ */
	if (nMode != screen_mode) {
		ReDrawDD();
		nMode = screen_mode;
	}
#elif XM7_VER >= 2
	/* 320x200 */
	if (mode320) {
		if (!bAnalog) {
			ReDrawDD();
			bAnalog = TRUE;
		}
		return;
	}

	/* 640x200 */
	if (bAnalog) {
		ReDrawDD();
		bAnalog = FALSE;
	}
#elif XM7_VER == 1 && defined(L4CARD)
	/* 640x400 */
	if (enable_400line) {
		if (!b400Line) {
			ReDrawDD();
			b400Line = TRUE;
		}
		return;
	}

	/* 640x200 */
	if (b400Line) {
		ReDrawDD();
		b400Line = FALSE;
	}
#endif
}
#endif

/*
 *	描画
 */
void FASTCALL DrawDD(void)
{
	/* 判定 */
#if XM7_VER >= 2 || (XM7_VER == 1 && defined(L4CARD))
	DrawDDCheck();
#endif

	/* 描画 */
#if XM7_VER >= 3
	switch (nMode) {
		case SCR_400LINE	:	Draw400l();
								return;
		case SCR_262144		:	Draw256k();
								return;
		case SCR_4096		:	Draw320();
								return;
		case SCR_200LINE	:	Draw640();
								return;
	}
#elif XM7_VER >= 2
	if (bAnalog) {
		Draw320();
		return;
	}
#elif XM7_VER == 1 && defined(L4CARD)
	if (b400Line) {
		DrawL4();
		return;
	}
#endif
	Draw640();
}

/*
 *	640x200、デジタルモード
 *	ラスタ描画
 */
static void FASTCALL DrawRaster640(int nRaster)
{
	DDSURFACEDESC ddsd;
	HRESULT hResult;
	BYTE *p;

	/* 表示範囲外の場合は何もしない */
	if (nRaster >= 200) {
		return;
	}

	/* パレット設定 */
	Palet640();

	/* サーフェイスをロック */
	memset(&ddsd, 0, sizeof(ddsd));
	ddsd.dwSize = sizeof(ddsd);
	hResult = lpdds[1]->Lock(NULL, &ddsd,
							 DDLOCK_WAIT | DDLOCK_SURFACEMEMORYPTR, NULL);
	if (hResult != DD_OK) {
		/* サーフェイスがロストしていれば、リストア */
		if (hResult == DDERR_SURFACELOST) {
			lpdds[1]->Restore();
		}
		/* 次回は全領域更新 */
		ReDrawDD();
		return;
	}

	/* レンダリング */
	p = (BYTE *)ddsd.lpSurface;
	if (nDDResolution == DDRES_480LINE) {
		p += (ddsd.lPitch * 40);
	}
	else if (nDDResolution == DDRES_SXGA) {
		p += (ddsd.lPitch * 56);
	}

#if XM7_VER >= 3
	/* ウィンドウオープン時 */
	if (window_open) {
		/* ウィンドウ外の描画 */
		if ((nRaster < window_dy1) || (nRaster > window_dy2)) {
			if (bPseudo400Line) {
				Render640c(p, ddsd.lPitch, nRaster, nRaster + 1);
			}
			else {
				Render640(p, ddsd.lPitch, nRaster, nRaster + 1, 2);
			}
		}
		else {
			if (bPseudo400Line) {
				Render640cw(p, ddsd.lPitch,
					nRaster, nRaster + 1, window_dx1, window_dx2);
			}
			else {
				Render640w(p, ddsd.lPitch,
					nRaster, nRaster + 1, window_dx1, window_dx2, 2);
			}
		}
	}
	else {
		if (bPseudo400Line) {
			Render640c(p, ddsd.lPitch, nRaster, nRaster + 1);
		}
		else {
			Render640(p, ddsd.lPitch, nRaster, nRaster + 1, 2);
		}
	}
#elif XM7_VER == 2
	if (bPseudo400Line) {
		Render640c(p, ddsd.lPitch, nRaster, nRaster + 1);
	}
	else {
		Render640(p, ddsd.lPitch, nRaster, nRaster + 1);
	}
#else
	if (bPseudo400Line) {
		Render640m(p, ddsd.lPitch, nRaster, nRaster + 1);
	}
	else {
		Render640(p, ddsd.lPitch, nRaster, nRaster + 1);
	}
#endif

	/* サーフェイスをアンロック */
	lpdds[1]->Unlock(ddsd.lpSurface);
}

#if XM7_VER >= 3
/*
 *	640x400、デジタルモード
 *	ラスタ描画
 */
static void FASTCALL DrawRaster400l(int nRaster)
{
	DDSURFACEDESC ddsd;
	HRESULT hResult;
	BYTE *p;

	/* 表示範囲外の場合は何もしない */
	if (nRaster >= 400) {
		return;
	}

	/* パレット設定 */
	Palet640();

	/* サーフェイスをロック */
	memset(&ddsd, 0, sizeof(ddsd));
	ddsd.dwSize = sizeof(ddsd);
	hResult = lpdds[1]->Lock(NULL, &ddsd,
							 DDLOCK_WAIT | DDLOCK_SURFACEMEMORYPTR, NULL);
	if (hResult != DD_OK) {
		/* サーフェイスがロストしていれば、リストア */
		if (hResult == DDERR_SURFACELOST) {
			lpdds[1]->Restore();
		}
		/* 次回は全領域更新 */
		ReDrawDD();
		return;
	}

	/* レンダリング */
	p = (BYTE *)ddsd.lpSurface;
	if (nDDResolution == DDRES_480LINE) {
		p += (ddsd.lPitch * 40);
	}
	else if (nDDResolution == DDRES_SXGA) {
		p += (ddsd.lPitch * 56);
	}

	/* ウィンドウオープン時 */
	if (window_open) {
		/* ウィンドウ外の描画 */
		if ((nRaster < window_dy1) || (nRaster > window_dy2)) {
			Render640(p, ddsd.lPitch, nRaster, nRaster + 1, 1);
		}
		else {
			Render640w(p, ddsd.lPitch, nRaster, nRaster + 1, window_dx1, window_dx2, 1);
		}
	}
	else {
		Render640(p, ddsd.lPitch, nRaster, nRaster + 1, 1);
	}

	/* サーフェイスをアンロック */
	lpdds[1]->Unlock(ddsd.lpSurface);
}
#endif

#if XM7_VER == 1 && defined(L4CARD)
/*
 *	テキスト画面
 *	ラスタ描画
 */
static void FASTCALL RenderTextRasterDD(BYTE *p, DWORD pitch,
						int nRaster, int left, int right)
{
	DWORD	tmp;
	int		x, x2;
	WORD	addr;
	DWORD	col;
	BYTE	cursor_start, cursor_end, cursor_type;
	BYTE	raster, lines, maxx;
	BYTE	chr, atr, chr_dat;
	BYTE	line;
	int		xsize;

	/* 表示OFFなら何もしない */
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

	if (width40_flag) {
		xsize = 16;
	}
	else {
		xsize = 8;
	}

	for (x=left / xsize; x<right / xsize; x++) {
		raster = (BYTE)(nRaster % lines);
		line = (BYTE)(nRaster / lines);
		addr = (WORD)((text_start_addr + (line * maxx + x) * 2) & 0x0ffe);
		chr = tvram_c[addr + 0];
		atr = tvram_c[addr + 1];

		if (atr & 0x20) {
			/* Intensity attribute ON */
			if (bGreenMonitor) {
				if (nPixelFormat == 3) {
					col = Palet16Tc_L4[(atr & 0x07) + 24];
				}
				else if (nPixelFormat == 1) {
					col = Palet16_L4[(atr & 0x07) + 40];
				}
				else {
					col = Palet16_L4[(atr & 0x07) + 56];
				}
			}
			else {
				if (nPixelFormat == 3) {
					col = Palet16Tc_L4[(atr & 0x07) + 8];
				}
				else if (nPixelFormat == 1) {
					col = Palet16_L4[(atr & 0x07) + 8];
				}
				else {
					col = Palet16_L4[(atr & 0x07) + 24];
				}
			}
		}
		else {
			/* Intensity attribute OFF */
			if (bGreenMonitor) {
				if (nPixelFormat == 3) {
					col = Palet16Tc_L4[(atr & 0x07) + 16];
				}
				else if (nPixelFormat == 1) {
					col = Palet16_L4[(atr & 0x07) + 32];
				}
				else {
					col = Palet16_L4[(atr & 0x07) + 48];
				}
			}
			else {
				if (nPixelFormat == 3) {
					col = Palet16Tc_L4[(atr & 0x07) + 0];
				}
				else if (nPixelFormat == 1) {
					col = Palet16_L4[(atr & 0x07) + 0];
				}
				else {
					col = Palet16_L4[(atr & 0x07) + 16];
				}
			}
		}

		/* Blink attribute */
		if ((!(atr & 0x10) || text_blink) && (raster < 16)) {
			chr_dat = subcg_l4[(WORD)(chr * 16) + raster];
		}
		else {
			chr_dat = 0x00;
		}

		/* Reverse attribute */
		if (atr & 0x08) {
			chr_dat = (BYTE)~chr_dat;
		}

		/* ほんでもって、かーそる */
		if ((addr == cursor_addr) && (cursor_type != 1) &&
			(cursor_blink || (cursor_type == 0))) {
			if ((raster >= cursor_start) && (raster <= cursor_end)) {
				chr_dat = (BYTE)~chr_dat;
			}
		}

		if (chr_dat) {
			for (x2=0; x2<8; x2++) {
				if (chr_dat & (1 << (7-x2))) {
					if (nPixelFormat == 3) {
						if (width40_flag) {
							tmp = nRaster * pitch + (((x << 3) + x2) << 3);
							*(DWORD *)(&p[tmp + 0]) = col;
							*(DWORD *)(&p[tmp + 4]) = col;
						}
						else {
							tmp = nRaster * pitch + (((x << 3) + x2) << 2);
							*(DWORD *)(&p[tmp + 0]) = col;
						}
					}
					else {
						if (width40_flag) {
							tmp = nRaster * pitch + (((x << 3) + x2) << 2);
							*(WORD *)(&p[tmp + 0]) = (WORD)col;
							*(WORD *)(&p[tmp + 2]) = (WORD)col;
						}
						else {
							tmp = nRaster * pitch + (((x << 3) + x2) << 1);
							*(WORD *)(&p[tmp + 0]) = (WORD)col;
						}
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
	DDSURFACEDESC ddsd;
	HRESULT hResult;
	BYTE *p;

	/* 表示範囲外の場合は何もしない */
	if (nRaster >= 400) {
		return;
	}

	/* パレット設定 */
	PaletL4();

	/* サーフェイスをロック */
	memset(&ddsd, 0, sizeof(ddsd));
	ddsd.dwSize = sizeof(ddsd);
	hResult = lpdds[1]->Lock(NULL, &ddsd,
							 DDLOCK_WAIT | DDLOCK_SURFACEMEMORYPTR, NULL);
	if (hResult != DD_OK) {
		/* サーフェイスがロストしていれば、リストア */
		if (hResult == DDERR_SURFACELOST) {
			lpdds[1]->Restore();
		}
		/* 次回は全領域更新 */
		ReDrawDD();
		return;
	}

	/* レンダリング */
	p = (BYTE *)ddsd.lpSurface;
	if (nDDResolution == DDRES_480LINE) {
		p += (ddsd.lPitch * 40);
	}
	else if (nDDResolution == DDRES_SXGA) {
		p += (ddsd.lPitch * 56);
	}

	RenderL4(p, ddsd.lPitch, nRaster, nRaster + 1);
	RenderTextRasterDD(p, ddsd.lPitch, nRaster, 0, 640);

	/* サーフェイスをアンロック */
	lpdds[1]->Unlock(ddsd.lpSurface);
}
#endif

#if XM7_VER >= 2
/*
 *	320x200、アナログモード
 *	ラスタ描画
 */
static void FASTCALL DrawRaster320(int nRaster)
{
	DDSURFACEDESC ddsd;
	HRESULT hResult;
	BYTE *p;

	/* 表示範囲外の場合は何もしない */
	if (nRaster >= 200) {
		return;
	}

	/* パレット設定 */
	Palet320();

	/* サーフェイスをロック */
	memset(&ddsd, 0, sizeof(ddsd));
	ddsd.dwSize = sizeof(ddsd);
	hResult = lpdds[1]->Lock(NULL, &ddsd,
							 DDLOCK_WAIT | DDLOCK_SURFACEMEMORYPTR, NULL);
	if (hResult != DD_OK) {
		/* サーフェイスがロストしていれば、リストア */
		if (hResult == DDERR_SURFACELOST) {
			lpdds[1]->Restore();
		}
		/* 次回は全領域更新 */
		ReDrawDD();
		return;
	}

	/* レンダリング */
	p = (BYTE *)ddsd.lpSurface;
	if (nDDResolution == DDRES_480LINE) {
		p += (ddsd.lPitch * 40);
	}
	else if (nDDResolution == DDRES_SXGA) {
		p += (ddsd.lPitch * 56);
	}

#if XM7_VER >= 3
	/* ウィンドウオープン時 */
	if (window_open) {
		/* ウィンドウ外の描画 */
		if ((nRaster < window_dy1) || (nRaster > window_dy2)) {
			Render320(p, ddsd.lPitch, nRaster, nRaster + 1);
		}
		else {
			Render320w(p, ddsd.lPitch, nRaster, nRaster + 1, window_dx1, window_dx2);
		}
	}
	else {
		Render320(p, ddsd.lPitch, nRaster, nRaster + 1);
	}
#else
	Render320(p, ddsd.lPitch, nRaster, nRaster + 1);
#endif

	/* サーフェイスをアンロック */
	lpdds[1]->Unlock(ddsd.lpSurface);
}
#endif

#if XM7_VER >= 3
/*
 *	320x200、26万色モード
 *	ラスタ描画
 */
static void FASTCALL DrawRaster256k(int nRaster)
{
	DDSURFACEDESC ddsd;
	HRESULT hResult;
	BYTE *p;

	/* 表示範囲外の場合は何もしない */
	if (nRaster >= 200) {
		return;
	}

	/* サーフェイスをロック */
	memset(&ddsd, 0, sizeof(ddsd));
	ddsd.dwSize = sizeof(ddsd);
	hResult = lpdds[1]->Lock(NULL, &ddsd,
							 DDLOCK_WAIT | DDLOCK_SURFACEMEMORYPTR, NULL);
	if (hResult != DD_OK) {
		/* サーフェイスがロストしていれば、リストア */
		if (hResult == DDERR_SURFACELOST) {
			lpdds[1]->Restore();
		}
		/* 次回は全領域更新 */
		ReDrawDD();
		return;
	}

	/* レンダリング */
	p = (BYTE *)ddsd.lpSurface;
	if (nDDResolution == DDRES_480LINE) {
		p += (ddsd.lPitch * 40);
	}
	else if (nDDResolution == DDRES_SXGA) {
		p += (ddsd.lPitch * 56);
	}

	if (crt_flag) {
		Render256k(p, ddsd.lPitch, nRaster, nRaster + 1, multi_page);
	}
	else {
		Render256k(p, ddsd.lPitch, nRaster, nRaster + 1, 0xff);
	}

	/* サーフェイスをアンロック */
	lpdds[1]->Unlock(ddsd.lpSurface);
}
#endif

/*
 *	ラスタ単位レンダリング
 *	描画
 */
void FASTCALL DrawPostRenderDD(void)
{
	DDSURFACEDESC ddsd;
	HRESULT hResult;
	RECT rect;
	RECT drect;
	BYTE *p;

	/* オールクリア */
	AllClear(FALSE);

	/* ステータスライン */
	StatusLine();

	/* サーフェイスをロック */
	memset(&ddsd, 0, sizeof(ddsd));
	ddsd.dwSize = sizeof(ddsd);
	hResult = lpdds[1]->Lock(NULL, &ddsd,
							 DDLOCK_WAIT | DDLOCK_SURFACEMEMORYPTR, NULL);
	if (hResult != DD_OK) {
		/* サーフェイスがロストしていれば、リストア */
		if (hResult == DDERR_SURFACELOST) {
			lpdds[1]->Restore();
		}
		/* 次回は全領域更新 */
		ReDrawDD();
		return;
	}

	/* レンダリング後処理 */
	p = (BYTE *)ddsd.lpSurface;
	if (nDDResolution == DDRES_480LINE) {
		p += (ddsd.lPitch * 40);
	}
	else if (nDDResolution == DDRES_SXGA) {
		p += (ddsd.lPitch * 56);
	}

#if XM7_VER >= 3
	if ((nMode != SCR_400LINE) && !bPseudo400Line) {
		RenderFullScan(p, ddsd.lPitch);
	}
#elif XM7_VER == 2 && defined(FMTV151)
	DrawV2_DD(p, ddsd.lPitch);
	if (!bPseudo400Line) {
		RenderFullScan(p, ddsd.lPitch);
	}
#elif XM7_VER == 1
#if defined(L4CARD)
	if (!b400Line && !bPseudo400Line) {
#else
	if (!bPseudo400Line) {
#endif
		RenderFullScan(p, ddsd.lPitch);
	}
#else
	if (!bPseudo400Line) {
		RenderFullScan(p, ddsd.lPitch);
	}
#endif

	/* サーフェイスをアンロック */
	lpdds[1]->Unlock(ddsd.lpSurface);

	/* Blt */
	rect.left = 0;
	rect.right = 640;

	/* ラスタ単位レンダリング時にひっくり返ることがあるので */
	if (nDrawTop >= nDrawBottom) {
		rect.top = 0;
		rect.bottom = 400;
	}
	else {
		rect.top = nDrawTop;
		rect.bottom = nDrawBottom;
	}

	drect = rect;

	if (nDDResolution == DDRES_480LINE) {
		rect.top += 40;
		rect.bottom += 40;
		drect.top += 40;
		drect.bottom += 40;
	}
	else if (nDDResolution == DDRES_WUXGA) {
		drect.top *= 3;
		drect.bottom *= 3;
		drect.left *= 3;
		drect.right *= 3;
	}
	else if (nDDResolution == DDRES_SXGA) {
		rect.top += 56;
		rect.bottom += 56;
		drect.left *= 2;
		drect.right *= 2;
		drect.top = rect.top * 2;
		drect.bottom = rect.bottom * 2;
	}
	else if (nDDResolution == DDRES_WXGA800) {
		drect.top *= 2;
		drect.bottom *= 2;
		drect.left *= 2;
		drect.right *= 2;
	}
	hResult = lpdds[0]->Blt(&drect, lpdds[1], &rect, DDBLT_WAIT, NULL);
	if (hResult != DD_OK) {
		/* サーフェイスがロストしていれば、リストア */
		if (hResult == DDERR_SURFACELOST) {
			lpdds[0]->Restore();
			lpdds[1]->Restore();
		}
		/* 次回は全領域更新 */
		ReDrawDD();
		return;
	}

	/* 次回に備え、ワークリセット */
	nDrawTop = 400;
	nDrawBottom = 0;
	nDrawLeft = 640;
	nDrawRight = 0;
	SetDrawFlag(TRUE);
}

/*
 *	ラスタ単位レンダリング
 *	各ラスタレンダリング
 */
void FASTCALL DrawRasterDD(int nRaster)
{
	/* 判定 */
#if XM7_VER >= 2 || (XM7_VER == 1 && defined(L4CARD))
	DrawDDCheck();
#endif

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
void FASTCALL EnterMenuDD(HWND hWnd)
{
	ASSERT(hWnd);

	/* クリッパーの有無で判定できる */
	if (!lpddc) {
		return;
	}

	/* クリッパーセット */
	LockVM();
	lpdds[0]->SetClipper(lpddc);
	UnlockVM();

	/* マウスカーソルon */
	if (!bMouseCursor) {
		ShowCursor(TRUE);
		bMouseCursor = TRUE;
	}

#if defined(MOUSE)
	/* マウスキャプチャ停止 */
	SetMouseCapture(FALSE);
#endif

	/* メニューバーを描画 */
	if (bWin8flag) {
		/* Windows 8では元のメインメニューを復元する */
		if (hWnd == hMainWnd) {
			SetMenu(hMainWnd, hMainMenu);
		}
	}
	DrawMenuBar(hMainWnd);
}

/*
 *	メニュー終了
 */
void FASTCALL ExitMenuDD(void)
{
	/* クリッパーの有無で判定できる */
	if (!lpddc) {
		return;
	}

	/* クリッパー解除 */
	LockVM();
	lpdds[0]->SetClipper(NULL);
	UnlockVM();

#if defined(MOUSE)
	/* マウスキャプチャ設定 */
	SetMouseCapture(TRUE);
#endif

	/* マウスカーソルOFF */
	if (bMouseCursor) {
		ShowCursor(FALSE);
		bMouseCursor = FALSE;
	}

	/* メニューバーを消去 */
	if (bWin8flag) {
		/* Windows 8ではメインメニューを一時的に解放しメニューバーを描画 */
		SetMenu(hMainWnd, NULL);
		DrawMenuBar(hMainWnd);
	}

	/* 再表示 */
	ReDrawDD();
}

/*-[ VMとの接続 ]-----------------------------------------------------------*/

/*
 *	VRAMセット
 */
void FASTCALL VramDD(WORD addr)
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
	DDDrawFlag[(y >> 3) * 80 + (x >> 3)] = 1;

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
void FASTCALL TvramDD(WORD addr)
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
		DDDrawFlag[(yy >> 3) * 80 + (x >> 3)] = 1;
		if (width40_flag) {
			DDDrawFlag[(yy >> 3) * 80 + (x >> 3) + 1] = 1;
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
void FASTCALL ReDrawTVRamDD(void)
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
void FASTCALL DigitalDD(void)
{
	nDrawTop = 0;
	nDrawBottom = 400;
	nDrawLeft = 0;
	nDrawRight = 640;
	bPaletFlag = TRUE;
	SetDrawFlag(TRUE);
}

/*
 *	アナログパレットセット
 */
void FASTCALL AnalogDD(void)
{
	nDrawTop = 0;
	nDrawBottom = 400;
	nDrawLeft = 0;
	nDrawRight = 640;
	bPaletFlag = TRUE;
	SetDrawFlag(TRUE);
}

/*
 *	再描画要求
 */
void FASTCALL ReDrawDD(void)
{
	/* 全領域レンダリング */
	nDrawTop = 0;
	nDrawBottom = 400;
	nDrawLeft = 0;
	nDrawRight = 640;
	bPaletFlag = TRUE;
	bClearFlag = TRUE;
	SetDrawFlag(TRUE);
	SetDirtyFlag(0, 400, TRUE);
}

#if XM7_VER >= 3
/*
 *	ハードウェアウィンドウ通知
 */
void FASTCALL WindowDD(void)
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
		p = &DDDrawFlag[(nDrawTop >> 3) * 80 + (nDrawLeft >> 3)];
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

#endif	/* _WIN32 */
