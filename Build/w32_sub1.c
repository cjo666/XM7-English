/*
 *	FM-7 EMULATOR "XM7"
 *
 *	Copyright (C) 1999-2017 ＰＩ．(Twitter:@xm6_original)
 *	Copyright (C) 2001-2017 Ryu Takegami (Twitter:@RyuTakegami)
 *
 *	[ Win32API サブウィンドウ１ ]
 *
 *	RHG履歴
 *	  2002.03.06	コンテキストメニューからのブレークポイント設定機能を追加
 *					メモリダンプウィンドウのスクロールバーをドラッグしたときに
 *					$FF80～$FFFFの内容が表示できない問題を修正
 *	  2002.03.10	逆アセンブルウィンドウのスクロールバー処理を修正($FFEFまで)
 *	  2002.05.04	逆アセンブルウィンドウに恐怖の強制ジャンプ機能を追加
 *					逆アセンブルとメモリダンプのコンテキストメニューを独立化
 *	  2002.05.23	逆アセンブルウィンドウのPC同期設定を独立
 *	  2002.09.12	逆アセンブル・メモリダンプウィンドウのサイズ変更に対応
 *					メモリダンプウィンドウにDP・I/O領域へのジャンプ機能を追加
 *					逆アセンブルウィンドウのスクロールバー処理を再修正
 *	  2002.10.04	DrawWindowText系関数でのリサイズをした場合の安全性を向上
 *	  2003.01.10	メモリダンプウィンドウにASCIIキャラクタ表示・チェックサム
 *					を追加・ウィンドウのリサイズ処理を改良
 *	  2003.01.26	メモリダンプウィンドウに漢字表示・メモリ内容変更機能を追加
 *					CPUレジスタウィンドウにコンテキストメニュー・レジスタ内容
 *					変更機能を追加
 *	  2003.01.27	メモリダンプウィンドウにメモリ検索機能を追加
 *					逆アセンブル・メモリダンプウィンドウにスタックジャンプ・
 *					ブレークポイントジャンプ機能を追加
 *	  2003.03.21	TileWindows/CascadeWindows時のサイズ変更処理を修正
 *	  2003.05.09	CPUレジスタ・逆アセンブル・メモリダンプウィンドウの対象
 *					CPU判定を強化した
 *					非漢字モードASCIIダンプでの0x80の表示を修正
 *	  2003.07.17	メモリ内容変更後にメモリダンプウィンドウの内容が更新され
 *					ない問題を修正
 *	  2003.11.21	3個以上のCPU(日本語通信カードとかステレオミュージックボッ
 *					クスとか)に対応できるように書き直しまくり(死
 *	  2004.09.18	メモリダンプウインドウのページアップ/ダウン時にウインドウ
 *					サイズに合わせてアップ/ダウン幅を変化させるように変更
 *	  2012.03.06	スケジューラウィンドウの文字列描画処理中にイベントが終了
 *					するとASSERTに引っかかる問題を修正
 *	  2015.03.12	メモリダンプウインドウの物理アドレス対応化
 *	  2015.03.13	サブウインドウのポップアップ対応
 *	  2015.11.13	アドレスヒストリを物理アドレス/論理アドレス別に持つよう変更
 *	  2016.09.04	Windows10で起動直後にCPUレジスタウインドウを開こうとすると
 *					落ちる問題を修正
 *					CPUレジスタウインドウのコンテキストメニューでDレジスタの内
 *					容変更ができない問題を修正
 */

#ifdef _WIN32

#define STRICT
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include <mmsystem.h>
#include <assert.h>
#include <stdlib.h>
#include "xm7.h"
#include "event.h"
#include "mmr.h"
#include "jsubsys.h"
#include "mouse.h"
#include "w32.h"
#include "w32_res.h"
#include "w32_sch.h"
#include "w32_sub.h"
#include "w32_kbd.h"

/*
 *	グローバル ワーク
 */
HWND hSubWnd[SWND_MAXNUM];				/* サブウインドウ */
BOOL bShowSubWindow[SWND_MAXNUM];		/* サブウィンドウ表示状態 */
BOOL bPopupSwnd = TRUE;					/* サブウィンドウポップアップ状態 */

/*
 *	スタティック ワーク
 */
static DWORD AddrHistoryPhys[16];		/* 物理アドレス アドレスヒストリ */
static DWORD AddrBufPhys;				/* 物理アドレス アドレスバッファ */
static int AddrNumPhys;					/* 物理アドレス アドレスヒストリ数 */
static BYTE nAddrDlgCPU;				/* アドレス設定対象CPU */
static BOOL bAddrDlgLinearAddr;			/* アドレス設定対象メモリ空間 */
static WORD AddrHistoryLogi[16];		/* 論理アドレス アドレスヒストリ */
static WORD AddrBufLogi;				/* 論理アドレス アドレスバッファ */
static int AddrNumLogi;					/* 論理アドレス アドレスヒストリ数*/

static BYTE nBreakCPU;					/* ブレークポイント 設定対象CPU */
static BYTE *pBreakPoint;				/* ブレークポイント Drawバッファ */
static HMENU hBreakPoint;				/* ブレークポイント メニューハンドル */
static POINT PosBreakPoint;				/* ブレークポイント マウス位置 */
static BYTE *pScheduler;				/* スケジューラ Drawバッファ */
static DWORD dwSchedulerTime[2];		/* スケジューラ 実時間 */
static DWORD dwSchedulerFrame[2];		/* スケジューラ フレーム数 */
static BYTE *pCPURegister[MAXCPU];		/* CPUレジスタ Drawバッファ */
static HMENU hCPURegister[MAXCPU];		/* CPUレジスタ メニューハンドル */
static BYTE *pDisAsm[MAXCPU];			/* 逆アセンブル Drawバッファ */
static DWORD dwDisAsm[MAXCPU];			/* 逆アセンブル アドレス */
static HMENU hDisAsm[MAXCPU];			/* 逆アセンブル メニューハンドル */
static POINT PosDisAsmPoint;			/* 逆アセンブル マウス位置 */
static BYTE nHeightDisAsm[MAXCPU];		/* 逆アセンブル ウィンドウ縦サイズ */
static BOOL bDisPhysicalAddr;			/* 逆アセンブル 物理アドレスモード */
static BYTE *pMemory[MAXCPU];			/* メモリダンプ Drawバッファ */
static DWORD dwMemory[MAXCPU];			/* メモリダンプ アドレス */
static HMENU hMemory[MAXCPU];			/* メモリダンプ メニューハンドル */
static BYTE nHeightMemory[MAXCPU];		/* メモリダンプ ウィンドウ縦サイズ */
static BOOL bAsciiDump[MAXCPU];			/* メモリダンプ ASCII表示 */
static BOOL bKanjiDump[MAXCPU];			/* メモリダンプ 漢字表示 */
static BOOL bDumpPhysicalAddr;			/* メモリダンプ 物理アドレスモード */
#if XM7_VER >= 3
static BOOL bExtendMMRMode;				/* メモリダンプ 拡張MMRモード */
#endif
#if XM7_VER >= 2
static int nFM7Ver;						/* メモリダンプ 機種種別保存領域 */
#else
static int nFMsubtype;					/* メモリダンプ 機種種別保存領域 */
#endif
static BYTE nMemDlgCPU;					/* メモリ変更 変更対象CPU */
static DWORD dwMemDlgAddr;				/* メモリ変更 アドレス */
static WORD nMemDlgByte;				/* メモリ変更 アドレス内容 */
static BYTE nRegDlgCPU;					/* レジスタ変更 変更対象CPU */
static BYTE nRegDlgNo;					/* レジスタ変更 変更対象レジスタ */
static BYTE nMemSrchCPU;				/* メモリ検索 検索対象CPU */
static DWORD dwMemSrchAddr;				/* メモリ検索 検索開始アドレス */
static DWORD dwMemSrchAddrSave[MAXCPU];	/* メモリ検索 検索開始アドレス main */
static char szMemSrchString[128];		/* メモリ検索 検索対象データ */
static WORD wStackAddr[16];				/* システムスタックジャンプアドレス */
static WORD wBreakAddr[16];				/* ブレークポイントジャンプアドレス */

/*
 *	プロトタイプ宣言
 */
static void FASTCALL PaintMemory(HWND hWnd);

/*
 *	シフトJIS判定マクロ+あるふぁ
 */
#define isKanji(p)	((BYTE)((p ^ 0x20) - 0xa1) <= 0x3b)
#define isKanji2(p)	(((BYTE)(p + 3) >= 0x43) && (p != 0x7f))
#define isJapanese	(PRIMARYLANGID(GetUserDefaultLangID()) == LANG_JAPANESE)

/*-[ 汎用ウィンドウ処理 ]----------------------------------------------------*/

/*
 *	サブウィンドウ
 *	サイズ変更
 */
static void FASTCALL WindowSizing(HWND hWnd, LPRECT rect, BYTE **buf)
{
	RECT wrect;
	POINT tpoint;
	POINT bpoint;
	int tframe;
	int bframe;
	int x;
	int y;
	BYTE *p;

	/* フレーム・タイトルバーのサイズを取得 */
	bframe = GetSystemMetrics(SM_CYSIZEFRAME);
	tframe = GetSystemMetrics(SM_CYCAPTION) + bframe;

	/* 新クライアント領域を計算 */
	tpoint.x = 0;
	tpoint.y = rect->top + tframe;
	bpoint.x = 0;
	bpoint.y = rect->bottom - bframe;
	ScreenToClient(hWnd, &tpoint);
	ScreenToClient(hWnd, &bpoint);

	/* クライアント領域を文字高の倍数に合わせる */
	tpoint.y = (tpoint.y / lCharHeight) * lCharHeight;
	bpoint.y = (bpoint.y / lCharHeight) * lCharHeight;

	/* スクリーン座標へ変換 */
	ClientToScreen(hWnd, &tpoint);
	ClientToScreen(hWnd, &bpoint);

	/* ウィンドウサイズを調整 */
	GetWindowRect(hWnd, &wrect);
	rect->left = wrect.left;
	rect->top = tpoint.y;
	rect->right = wrect.right;
	rect->bottom = bpoint.y;
	x = rect->right / lCharWidth;
	y = rect->bottom / lCharHeight;

	/* 縦方向のみ調整(横方向はAdjustWindowRect後に元の値を代入) */
	if (bPopupSwnd) {
		AdjustWindowRect(rect,	WS_POPUP | WS_OVERLAPPED | WS_SYSMENU |
								WS_CAPTION | WS_VISIBLE | 
								WS_MINIMIZEBOX | WS_SIZEBOX, FALSE);
	}
	else {
		AdjustWindowRect(rect,	WS_CHILD | WS_OVERLAPPED | WS_SYSMENU |
								WS_CAPTION | WS_VISIBLE | 
								WS_MINIMIZEBOX | WS_CLIPSIBLINGS | 
								WS_SIZEBOX, FALSE);
	}
	rect->left = wrect.left;
	rect->right = wrect.right;

	/* バッファを再取得し、0xFFで埋める */
	p = realloc(*buf, 2 * x * y);
	ASSERT(p);
	*buf = p;
	memset(p, 0xff, 2 * x * y);
}

/*
 *	サブウィンドウ
 *	テキスト描画
 */
void FASTCALL DrawWindowText(HDC hDC, BYTE *ptr, int x, int y)
{
	BYTE *p;
	BYTE *q;
	int xx, yy;
	HFONT hBackup;

	/* フォントセレクト */
	hBackup = SelectObject(hDC, hFont);

	/* 比較描画 */
	p = ptr;
	q = &p[x * y];
	for (yy=0; yy<y; yy++) {
		for (xx=0; xx<x; xx++) {
			if (ptr) {
				if (*p != *q) {
					TextOut(hDC, xx * lCharWidth, yy * lCharHeight,
						(LPCTSTR)p, 1);
					*q = *p;
				}
				p++;
				q++;
			}
		}
	}

	/* 終了 */
	SelectObject(hDC, hBackup);
}

/*
 *	サブウィンドウ
 *	テキスト描画 (漢字表示対応)
 */
void FASTCALL DrawWindowTextKanji(HDC hDC, BYTE *ptr, int x, int y)
{
	BYTE *p;
	BYTE *q;
	int xx, yy;
	HFONT hBackup;

	/* フォントセレクト */
	hBackup = SelectObject(hDC, hFont);

	/* 比較描画 */
	for (yy=0; yy<y; yy++) {
		for (xx=0; xx<x; xx++) {
			p = &ptr[x * yy + xx];
			q = &p[x * y];
			if (ptr) {
				if (isKanji(*p)) {
					if (*(WORD *)p != *(WORD *)q) {
						TextOut(hDC, xx * lCharWidth, yy * lCharHeight,
							(LPCTSTR)p, 2);
						*(WORD *)q = *(WORD *)p;
					}
					xx++;
				}
				else {
					if (*p != *q) {
						TextOut(hDC, xx * lCharWidth, yy * lCharHeight,
							(LPCTSTR)p, 1);
						*q = *p;
					}
				}
			}
		}
	}

	/* 終了 */
	SelectObject(hDC, hBackup);
}

/*
 *	サブウィンドウ
 *	テキスト描画(反転付き)
 */
void FASTCALL DrawWindowText2(HDC hDC, BYTE *ptr, int x, int y)
{
	BYTE *p;
	BYTE *q;
	int xx, yy;
	HFONT hBackup;
	COLORREF tcolor, bcolor;
	char dat;

	/* フォントセレクト */
	hBackup = SelectObject(hDC, hFont);

	/* 比較描画 */
	p = ptr;
	q = &p[x * y];
	for (yy=0; yy<y; yy++) {
		for (xx=0; xx<x; xx++) {
			if (ptr) {
				if (*p != *q) {
					if (*p & 0x80) {
						/* 反転表示 */
						dat = (char)(*p & 0x7f);
						bcolor = GetBkColor(hDC);
						tcolor = GetTextColor(hDC);
						SetTextColor(hDC, bcolor);
						SetBkColor(hDC, tcolor);
						TextOut(hDC, xx * lCharWidth, yy * lCharHeight, &dat, 1);
						SetTextColor(hDC, tcolor);
						SetBkColor(hDC, bcolor);
					}
					else {
						/* 通常表示 */
						TextOut(hDC, xx * lCharWidth, yy * lCharHeight,
							(LPCTSTR)p, 1);
					}
					*q = *p;
				}
				p++;
				q++;
			}
		}
	}

	/* 終了 */
	SelectObject(hDC, hBackup);
}

/*
 *	サブウインドウ
 *	WM_DESTROY処理
 */
void FASTCALL DestroySubWindow(HWND hWnd, BYTE **pBuf, HMENU hmenu)
{
	int i;

	/* メインウインドウへ自動通知 */
	for (i=0; i<SWND_MAXNUM; i++) {
		if (hSubWnd[i] == hWnd) {
			hSubWnd[i] = NULL;

			/* メニュー削除 */
			if (hmenu) {
				DestroyMenu(hmenu);
			}

			/* Drawバッファ削除 */
			if (pBuf && *pBuf) {
				free(*pBuf);
				*pBuf = NULL;
			}
		}
	}

	/* ドローウインドウを再描画 */
	InvalidateRect(hDrawWnd, NULL, FALSE);
}

/*
 *	サブウィンドウ
 *	位置を算出
 */
void FASTCALL PositioningSubWindow(HWND hParent, LPRECT rect, int index)
{
	RECT wrect;

	/* サブウィンドウ位置を算出 */
	rect->left = lCharWidth * index;
	rect->top = lCharHeight * index;
	rect->right = 0;
	rect->bottom = 0;
	if (rect->top >= 380) {
		rect->top -= 380;
	}
	if (bPopupSwnd) {
		GetWindowRect(hParent, (LPRECT)&wrect);
		rect->left += wrect.left;
		rect->top += wrect.top;
	}
}

/*-[ ウィンドウワーク初期化 ]------------------------------------------------*/

/*
 *	サブウィンドウ
 *	ワーク初期化
 */
void FASTCALL InitSubWndWork(void)
{
	int i;

	for (i=0; i<MAXCPU; i++) {
		nHeightDisAsm[i] = 8;			/* 逆アセンブル ウィンドウ縦サイズ */
		nHeightMemory[i] = 8;			/* メモリダンプ ウィンドウ縦サイズ */
		bAsciiDump[i] = FALSE;			/* メモリダンプ ASCII表示 */
		bKanjiDump[i] = FALSE;			/* メモリダンプ 漢字表示 */
		dwMemSrchAddrSave[i] = 0;		/* メモリ検索 検索開始アドレス */
	}
	bDisPhysicalAddr = FALSE;
	bDumpPhysicalAddr = FALSE;
}

/*-[ ただのてぬき ]----------------------------------------------------------*/

/*
 *	物理メモリモード状態取得
 */
static BOOL FASTCALL isLinearAddrMode(BOOL bPhysicalAddr)
{
	if (fm7_ver <= 1) {
#if XM7_VER == 1
		if (fm_subtype != FMSUB_FM77) {
			return FALSE;
		}
#else
		return FALSE;
#endif
	}

	return bPhysicalAddr;
}

/*
 *	メモリ内容変更ダイアログ・その他
 *	最大メモリ空間取得
 */
static DWORD FASTCALL GetMaxMemArea(BOOL bPhysicalAddr)
{
	if (fm7_ver <= 1) {
#if XM7_VER == 1
		if ((fm_subtype != FMSUB_FM77) || !bPhysicalAddr) {
			return 0x10000;
		}
		else {
			return 0x40000;
		}
#else
		return 0x10000;
#endif
	}
	else if ((fm7_ver <= 2) && bPhysicalAddr) {
		return 0x40000;
	}
#if XM7_VER >= 3
	else if (isLinearAddrMode(bPhysicalAddr)) {
		if (mmr_ext) {
			return 0x100000;
		}
		else {
			return 0x40000;
		}
	}
#endif

	return 0x10000;
}

/*-[ アドレスヒストリ ]------------------------------------------------------*/

/*
 *	アドレスヒストリ
 *	コンボボックスへの挿入
 */
static void FASTCALL InsertHistory(HWND hWnd, BOOL bIsLinearAddr)
{
	int i;
	char string[128];

	/* ヒストリを挿入 */
	(void)ComboBox_ResetContent(hWnd);
	if ((nAddrDlgCPU == MAINCPU) && isLinearAddrMode(bIsLinearAddr)) {
		for (i=AddrNumPhys; i>0; i--) {
			_snprintf(string, sizeof(string), "%05X",
			AddrHistoryPhys[i - 1] &
				(GetMaxMemArea(bIsLinearAddr) - 1));
			(void)ComboBox_AddString(hWnd, string);
		}
	}
	else {
		for (i=AddrNumLogi; i>0; i--) {
			_snprintf(string, sizeof(string), "%04X", AddrHistoryLogi[i - 1]);
			(void)ComboBox_AddString(hWnd, string);
		}
	}
}

/*
 *	アドレスヒストリ
 *	追加
 */
static void FASTCALL AddAddrHistory(DWORD addr, BOOL bIsLinearAddr)
{
	int i;

	/* 二重登録チェック */
	if ((nAddrDlgCPU == MAINCPU) && isLinearAddrMode(bIsLinearAddr)) {
		for (i=0; i<AddrNumPhys; i++) {
			if (AddrHistoryPhys[i] == addr) {
				return;
			}
		}

		/* ヒストリをシフト、挿入、カウントアップ */
		for (i=14; i>=0; i--) {
			AddrHistoryPhys[i + 1] = AddrHistoryPhys[i];
		}
		AddrHistoryPhys[0] = addr;
		if (AddrNumPhys < 16) {
			AddrNumPhys++;
		}
	}
	else {
		/* 二重登録チェック */
		for (i=0; i<AddrNumLogi; i++) {
			if (AddrHistoryLogi[i] == (WORD)(addr & 0xffff)) {
				return;
			}
		}

		/* ヒストリをシフト、挿入、カウントアップ */
		for (i=14; i>=0; i--) {
			AddrHistoryLogi[i + 1] = AddrHistoryLogi[i];
		}
		AddrHistoryLogi[0] = (WORD)(addr & 0xffff);
		if (AddrNumLogi < 16) {
			AddrNumLogi++;
		}
	}
}

/*-[ アドレス入力ダイアログ ]------------------------------------------------*/

/*
 *	アドレス入力ダイアログ
 *	ダイアログ初期化
 */
static BOOL FASTCALL AddrDlgInit(HWND hDlg)
{
	HWND hWnd;
	RECT prect;
	RECT drect;
	char string[128];

	ASSERT(hDlg);

	/* 親ウインドウの中央に設定 */
	hWnd = GetParent(hDlg);
	GetWindowRect(hWnd, &prect);
	GetWindowRect(hDlg, &drect);
	drect.right -= drect.left;
	drect.bottom -= drect.top;
	drect.left = (prect.right - prect.left) / 2 + prect.left;
	drect.left -= (drect.right / 2);
	drect.top = (prect.bottom - prect.top) / 2 + prect.top;
	drect.top -= (drect.bottom / 2);
	MoveWindow(hDlg, drect.left, drect.top, drect.right, drect.bottom, FALSE);

	/* コンボボックス処理 */
	hWnd = GetDlgItem(hDlg, IDC_ADDRCOMBO);
	ASSERT(hWnd);
	InsertHistory(hWnd, bAddrDlgLinearAddr);

	/* アドレスを設定 */
	if ((nAddrDlgCPU == MAINCPU) && isLinearAddrMode(bAddrDlgLinearAddr)) {
		_snprintf(string, sizeof(string), "%05X",
			AddrBufPhys & (GetMaxMemArea(bAddrDlgLinearAddr)-1));
	}
	else {
		_snprintf(string, sizeof(string), "%04X", AddrBufLogi);
	}
	ComboBox_SetText(hWnd, string);

	return TRUE;
}

/*
 *	アドレス入力ダイアログ
 *	ダイアログOK
 */
static void FASTCALL AddrDlgOK(HWND hDlg)
{
	DWORD addr;
	HWND hWnd;
	char string[128];

	ASSERT(hDlg);

	/* コンボボックス処理 */
	hWnd = GetDlgItem(hDlg, IDC_ADDRCOMBO);
	ASSERT(hWnd);

	/* 現在の値を取得 */
	ComboBox_GetText(hWnd, string, sizeof(string) - 1);
	addr = (DWORD)strtol(string, NULL, 16);
	if ((nAddrDlgCPU == MAINCPU) && bAddrDlgLinearAddr) {
		AddrBufPhys = (addr & (GetMaxMemArea(bAddrDlgLinearAddr) - 1));
		AddAddrHistory(AddrBufPhys, bAddrDlgLinearAddr);
	}
	else {
		AddrBufLogi = (WORD)(addr & 0xffff);
		AddAddrHistory(AddrBufLogi, FALSE);
	}
}

/*
 *	アドレス入力ダイアログ
 *	ダイアログプロシージャ
 */
static BOOL CALLBACK AddrDlgProc(HWND hDlg, UINT iMsg,
									WPARAM wParam, LPARAM lParam)
{
	UNUSED(lParam);

	switch (iMsg) {
		/* ダイアログ初期化 */
		case WM_INITDIALOG:
			return AddrDlgInit(hDlg);

		/* コマンド処理 */
		case WM_COMMAND:
			switch (LOWORD(wParam)) {
				/* OK・キャンセル */
				case IDOK:
				case IDCANCEL:
					if (LOWORD(wParam) == IDOK) {
						AddrDlgOK(hDlg);
					}
					EndDialog(hDlg, LOWORD(wParam));
					InvalidateRect(hDrawWnd, NULL, FALSE);
					return TRUE;
			}
			break;
	}

	/* それ以外は、FALSE */
	return FALSE;
}

/*
 *	アドレス入力
 */
static BOOL FASTCALL AddrDlg(HWND hWnd, DWORD *pAddr, BYTE nCPU,
							 BOOL bLinearAddr)
{
	int ret;

	ASSERT(hWnd);
	ASSERT(pAddr);

	/* 物理アドレスモードをセット */
	bAddrDlgLinearAddr = bLinearAddr;

	/* アドレスをセット */
#if XM7_VER == 1 && defined(Z80CARD)
	if (((nCPU == MAINCPU) || (nCPU == MAINZ80)) && bAddrDlgLinearAddr) {
#else
	if ((nCPU == MAINCPU) && bAddrDlgLinearAddr) {
#endif
		AddrBufPhys = *pAddr;
	}
	else {
		AddrBufLogi = *(WORD *)pAddr;
	}

	/* モーダルダイアログ実行 */
	ret = DialogBox(hAppInstance, MAKEINTRESOURCE(IDD_ADDRDLG), hWnd, AddrDlgProc);
	if (ret != IDOK) {
		return FALSE;
	}

	/* アドレスをセットし、帰る */
#if XM7_VER == 1 && defined(Z80CARD)
	if (((nCPU == MAINCPU) || (nCPU == MAINZ80)) && bAddrDlgLinearAddr) {
#else
	if ((nCPU == MAINCPU) && bAddrDlgLinearAddr) {
#endif
		*pAddr = AddrBufPhys;
	}
	else {
		*(WORD *)pAddr = AddrBufLogi;
	}

	return TRUE;
}

/*-[ ブレークポイント設定ダイアログ ]----------------------------------------*/

/*
 *	ブレークポイント設定ダイアログ
 *	ダイアログ初期化
 */
static BOOL FASTCALL BreakPointDlgInit(HWND hDlg)
{
	HWND hWnd;
	RECT prect;
	RECT drect;
	char string[128];

	ASSERT(hDlg);

	/* 親ウインドウの中央に設定 */
	hWnd = GetParent(hDlg);
	GetWindowRect(hWnd, &prect);
	GetWindowRect(hDlg, &drect);
	drect.right -= drect.left;
	drect.bottom -= drect.top;
	drect.left = (prect.right - prect.left) / 2 + prect.left;
	drect.left -= (drect.right / 2);
	drect.top = (prect.bottom - prect.top) / 2 + prect.top;
	drect.top -= (drect.bottom / 2);
	MoveWindow(hDlg, drect.left, drect.top, drect.right, drect.bottom, FALSE);

	/* コンボボックス処理 */
	hWnd = GetDlgItem(hDlg, IDC_BREAKP_ADDRCOMBO);
	ASSERT(hWnd);
	InsertHistory(hWnd, FALSE);

	/* アドレスを設定 */
	_snprintf(string, sizeof(string), "%04X", AddrBufLogi);
	ComboBox_SetText(hWnd, string);

	/* 対象CPUを設定 */
	switch (nBreakCPU) {
		case MAINCPU :	CheckDlgButton(hDlg, IDC_BREAKP_CPU_MAIN, BST_CHECKED);
						break;
		case SUBCPU :	CheckDlgButton(hDlg, IDC_BREAKP_CPU_SUB, BST_CHECKED);
						break;
#if XM7_VER == 1
#if defined(JSUB)
		case JSUBCPU :	CheckDlgButton(hDlg, IDC_BREAKP_CPU_JSUB, BST_CHECKED);
						break;
#endif
#if defined(Z80CARD)
		case MAINZ80 :	CheckDlgButton(hDlg, IDC_BREAKP_CPU_MAINZ80, BST_CHECKED);
						break;
#endif
#endif
	}

	return TRUE;
}

/*
 *	ブレークポイント設定ダイアログ
 *	ダイアログOK
 */
static void FASTCALL BreakPointDlgOK(HWND hDlg)
{
	HWND hWnd;
	char string[128];

	ASSERT(hDlg);

	/* コンボボックス処理 */
	hWnd = GetDlgItem(hDlg, IDC_BREAKP_ADDRCOMBO);
	ASSERT(hWnd);

	/* 対象CPUを取得 */
	if (IsDlgButtonChecked(hDlg, IDC_BREAKP_CPU_MAIN) == BST_CHECKED) {
		nBreakCPU = MAINCPU;
	}
#if XM7_VER == 1
#if defined(JSUB)
	else if (IsDlgButtonChecked(hDlg, IDC_BREAKP_CPU_JSUB) == BST_CHECKED) {
		nBreakCPU = JSUBCPU;
	}
#endif
#if defined(Z80CARD)
	else if (IsDlgButtonChecked(hDlg, IDC_BREAKP_CPU_MAINZ80) == BST_CHECKED) {
		nBreakCPU = MAINZ80;
	}
#endif
#endif
	else {
		nBreakCPU = SUBCPU;
	}

	/* 現在の値を取得 */
	ComboBox_GetText(hWnd, string, sizeof(string) - 1);
	AddrBufLogi = (WORD)strtol(string, NULL, 16);

	/* ヒストリ追加 */
	AddAddrHistory(AddrBufLogi, FALSE);
}

/*
 *	ブレークポイント設定ダイアログ
 *	ダイアログプロシージャ
 */
static BOOL CALLBACK BreakPointDlgProc(HWND hDlg, UINT iMsg,
									WPARAM wParam, LPARAM lParam)
{
	UNUSED(lParam);

	switch (iMsg) {
		/* ダイアログ初期化 */
		case WM_INITDIALOG:
			return BreakPointDlgInit(hDlg);

		/* コマンド処理 */
		case WM_COMMAND:
			switch (LOWORD(wParam)) {
				/* OK・キャンセル */
				case IDOK:
				case IDCANCEL:
					if (LOWORD(wParam) == IDOK) {
						BreakPointDlgOK(hDlg);
					}
					EndDialog(hDlg, LOWORD(wParam));
					InvalidateRect(hDrawWnd, NULL, FALSE);
					return TRUE;
			}
			break;
	}

	/* それ以外は、FALSE */
	return FALSE;
}

/*
 *	ブレークポイント設定
 */
static BOOL FASTCALL BreakPointDlg(HWND hWnd, int num)
{
	int ret;

	ASSERT(hWnd);

	/* アドレスをセット */
	if (breakp[num].flag != BREAKP_NOTUSE) {
		AddrBufLogi = breakp[num].addr;
		nBreakCPU = (BYTE)breakp[num].cpu;
	}
	else {
		AddrBufLogi = 0;
		nBreakCPU = MAINCPU;
	}

	/* モーダルダイアログ実行 */
	ret = DialogBox(hAppInstance, MAKEINTRESOURCE(IDD_BREAKPDLG), hWnd, BreakPointDlgProc);
	if (ret != IDOK) {
		return FALSE;
	}

	/* アドレスをセットし、帰る */
	schedule_setbreak2(num, nBreakCPU, (WORD)AddrBufLogi);
	return TRUE;
}

/*-[ ブレークポイント ]------------------------------------------------------*/

/*
 *	ブレークポイントウインドウ
 *	セットアップ
 */
static void FASTCALL SetupBreakPoint(BYTE *p, int x, int y)
{
	int i;
	char string[128];
	char tmp[128];

	ASSERT(p);
	ASSERT(x > 0);
	ASSERT(y > 0);

	/* VMをロック */
	LockVM();

	/* 一旦スペースで埋める */
	memset(p, 0x20, x * y);

	/* ブレークポイントループ */
	for (i=0; i<BREAKP_MAXNUM; i++) {
		/* 文字列作成 */
		string[0] = '\0';
		if (breakp[i].flag == BREAKP_NOTUSE) {
			_snprintf(string, sizeof(string), "%2d ------------------", i + 1);
		}
		else {
			switch (breakp[i].cpu) {
				case MAINCPU :	strncpy(tmp, "Main", sizeof(tmp));
								break;
				case SUBCPU :	strncpy(tmp, "Sub ", sizeof(tmp));
								break;
#if XM7_VER == 1
#if defined(JSUB)
				case JSUBCPU :	strncpy(tmp, "Jsub", sizeof(tmp));
								break;
#endif
#if defined(Z80CARD)
				case MAINZ80 : strncpy(tmp, "Z80 ", sizeof(tmp));
								break;
#endif
#endif
			}
			_snprintf(string, sizeof(string), "%2d %s %04X ",
				i + 1, tmp, breakp[i].addr);
			switch (breakp[i].flag) {
				case BREAKP_ENABLED:
					strncat(string, " Enabled", sizeof(string) - strlen(string) - 1);
					break;
				case BREAKP_DISABLED:
					strncat(string, "Disabled", sizeof(string) - strlen(string) - 1);
					break;
			}
		}

		/* コピー */
		memcpy(&p[x * (i % 8) + 23 * (i / 8)], string, strlen(string));
	}

	/* VMをアンロック */
	UnlockVM();
}

/*
 *	ブレークポイントウインドウ
 *	描画
 */
static void FASTCALL DrawBreakPoint(HWND hWnd, HDC hDC)
{
	RECT rect;
	int x, y;

	ASSERT(hWnd);
	ASSERT(hDC);

	/* ウインドウジオメトリを得る */
	GetClientRect(hWnd, &rect);
	x = rect.right / lCharWidth;
	y = rect.bottom / lCharHeight;
	if ((x == 0) || (y == 0)) {
		return;
	}

	/* セットアップ */
	if (!pBreakPoint) {
		return;
	}
	SetupBreakPoint(pBreakPoint, x, y);

	/* 描画 */
	DrawWindowText(hDC, pBreakPoint, x, y);
}

/*
 *	ブレークポイントウインドウ
 *	リフレッシュ
 */
void FASTCALL RefreshBreakPoint(void)
{
	HWND hWnd;
	HDC hDC;

	/* 常に呼ばれるので、存在チェックすること */
	if (hSubWnd[SWND_BREAKPOINT] == NULL) {
		return;
	}

	/* 描画 */
	hWnd = hSubWnd[SWND_BREAKPOINT];
	hDC = GetDC(hWnd);
	DrawBreakPoint(hWnd, hDC);
	ReleaseDC(hWnd, hDC);
}

/*
 *	ブレークポイントウインドウ
 *	再描画
 */
static void FASTCALL PaintBreakPoint(HWND hWnd)
{
	HDC hDC;
	PAINTSTRUCT ps;
	RECT rect;
	BYTE *p;
	int x, y;

	ASSERT(hWnd);

	/* ポインタを設定(存在しなければ何もしない) */
	p = pBreakPoint;
	if (!p) {
		return;
	}

	/* ウインドウジオメトリを得る */
	GetClientRect(hWnd, &rect);
	x = rect.right / lCharWidth;
	y = rect.bottom / lCharHeight;

	/* 後半エリアをFFで埋める */
	if ((x > 0) && (y > 0)) {
		memset(&p[x * y], 0xff, x * y);
	}

	/* 描画 */
	hDC = BeginPaint(hWnd, &ps);
	ASSERT(hDC);
	DrawBreakPoint(hWnd, hDC);
	EndPaint(hWnd, &ps);
}

/*
 *	ブレークポイント
 *	コマンド
 */
static void FASTCALL CmdBreakPoint(HWND hWnd, WORD wID)
{
	int x, y;
	int num;
	POINT point;

	ASSERT(hWnd);

	/* インデックス番号取得 */
	point = PosBreakPoint;
	x = point.x / lCharWidth;
	y = point.y / lCharHeight;
	num = (x / 23) * 8 + y;
	if ((num < 0) || ((num < 8) && (x > 20)) || (num >= BREAKP_MAXNUM)) {
		return;
	}

	/* コマンド別 */
	switch (wID) {
		/* ジャンプ */
		case IDM_BREAKP_JUMP:
			if ((num < 8) && (x > 20)) {
				break;
			}
			if (breakp[num].flag != BREAKP_NOTUSE) {
				AddrDisAsm((BYTE)breakp[num].cpu, breakp[num].addr);
			}
			break;

		/* 設定 */
		case IDM_BREAKP_SET:
			if ((num < 8) && (x > 20)) {
				break;
			}
			BreakPointDlg(hWnd, num);
			break;

		/* イネーブル */
		case IDM_BREAKP_ENABLE:
			if ((num < 8) && (x > 20)) {
				break;
			}
			if (breakp[num].flag == BREAKP_DISABLED) {
				breakp[num].flag = BREAKP_ENABLED;
				InvalidateRect(hWnd, NULL, FALSE);
				RefreshDisAsm();
			}
			break;

		/* ディセーブル */
		case IDM_BREAKP_DISABLE:
			if ((num < 8) && (x > 20)) {
				break;
			}
			if (breakp[num].flag == BREAKP_ENABLED) {
				breakp[num].flag = BREAKP_DISABLED;
				InvalidateRect(hWnd, NULL, FALSE);
				RefreshDisAsm();
			}
			break;

		/* クリア */
		case IDM_BREAKP_CLEAR:
			if ((num < 8) && (x > 20)) {
				break;
			}
			if (breakp[num].flag != BREAKP_NOTUSE) {
				schedule_setbreak(breakp[num].cpu, breakp[num].addr);
			}
			InvalidateRect(hWnd, NULL, FALSE);
			RefreshDisAsm();
			break;

		/* 全てクリア */
		case IDM_BREAKP_ALL:
			for (num=0; num<BREAKP_MAXNUM; num++) {
				if (breakp[num].flag != BREAKP_NOTUSE) {
					schedule_setbreak(breakp[num].cpu, breakp[num].addr);
				}
			}
			InvalidateRect(hWnd, NULL, FALSE);
			RefreshDisAsm();
			break;
	}
}

/*
 *	ブレークポイントウインドウ
 *	ウインドウプロシージャ
 */
static LRESULT CALLBACK BreakPointProc(HWND hWnd, UINT message,
								 WPARAM wParam, LPARAM lParam)
{
	POINT point;
	int i, x, y;
	HMENU hMenu;

	/* メッセージ別 */
	switch (message) {
		/* ウインドウ再描画 */
		case WM_PAINT:
			/* ロックが必要 */
			LockVM();
			PaintBreakPoint(hWnd);
			UnlockVM();
			return 0;

		/* 左クリック */
		case WM_LBUTTONDOWN:
		case WM_LBUTTONDBLCLK:
#if defined(MOUSE)
			/* マウスエミュレーション使用時は無効 */
			if (mos_capture && !stopreq_flag && run_flag) {
				return FALSE;
			}
#endif

			/* カーソル位置から、決定 */
			x = LOWORD(lParam) / lCharWidth;
			y = HIWORD(lParam) / lCharHeight;
			i = (x / 23) * 8 + y;
			if ((i < 8) && (x > 20)) {
				return 0;
			}
			if ((i >= 0) && (i < BREAKP_MAXNUM)) {
				if (breakp[i].flag != BREAKP_NOTUSE) {
					AddrDisAsm((BYTE)breakp[i].cpu, breakp[i].addr);
				}
			}
			return 0;

		/* コンテキストメニュー */
		case WM_RBUTTONDOWN:
#if defined(MOUSE)
			/* マウスエミュレーション使用時は無効 */
			if (mos_capture && !stopreq_flag && run_flag) {
				return FALSE;
			}
#endif

			point.x = LOWORD(lParam);
			point.y = HIWORD(lParam);
			PosBreakPoint = point;
			hMenu = GetSubMenu(hBreakPoint, 0);
			ClientToScreen(hWnd, &point);
			TrackPopupMenu(hMenu, 0, point.x, point.y, 0, hWnd, NULL);
			return 0;

		/* ウインドウ削除 */
		case WM_DESTROY:
			LockVM();

			/* メニュー削除 */
			DestroyMenu(hBreakPoint);

			/* メインウインドウへ自動通知 */
			DestroySubWindow(hWnd, &pBreakPoint, NULL);

			UnlockVM();
			break;

		/* コマンド */
		case WM_COMMAND:
			CmdBreakPoint(hWnd, LOWORD(wParam));
			break;
	}

	/* デフォルト ウインドウプロシージャ */
	return DefWindowProc(hWnd, message, wParam, lParam);
}

/*
 *	ブレークポイントウインドウ
 *	ウインドウ作成
 */
HWND FASTCALL CreateBreakPoint(HWND hParent, int index)
{
	WNDCLASSEX wcex;
	char szClassName[] = "XM7_BreakPoint";
	char szWndName[128];
	RECT rect;
	RECT crect, wrect;
	HWND hWnd;
	DWORD dwStyle;

	ASSERT(hParent);

	/* ウインドウ矩形を計算 */
	PositioningSubWindow(hParent, &rect, index);
	rect.right = lCharWidth * 44;
	rect.bottom = lCharHeight * (BREAKP_MAXNUM / 2);

	/* ウインドウタイトルを決定、バッファ確保 */
	LoadString(hAppInstance, IDS_SWND_BREAKPOINT,
				szWndName, sizeof(szWndName));
	pBreakPoint = malloc(2 * (rect.right / lCharWidth) *
								(rect.bottom / lCharHeight));

	/* メニューをロード */
	hBreakPoint = LoadMenu(hAppInstance, MAKEINTRESOURCE(IDR_BREAKPOINTMENU));

	/* ウインドウクラスの登録 */
	memset(&wcex, 0, sizeof(wcex));
	wcex.cbSize = sizeof(wcex);
	wcex.style = CS_VREDRAW | CS_HREDRAW;
	wcex.lpfnWndProc = BreakPointProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hAppInstance;
	wcex.hIcon = LoadIcon(hAppInstance, MAKEINTRESOURCE(IDI_WNDICON));
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	wcex.lpszMenuName = NULL;
	wcex.lpszClassName = szClassName;
	wcex.hIconSm = LoadIcon(hAppInstance, MAKEINTRESOURCE(IDI_WNDICON));
	RegisterClassEx(&wcex);

	/* ウインドウ作成 */
	if (bPopupSwnd) {
		dwStyle =	WS_POPUP | WS_OVERLAPPED | WS_SYSMENU | WS_CAPTION |
					WS_VISIBLE | WS_MINIMIZEBOX | WS_BORDER;
	}
	else {
		dwStyle = WS_CHILD | WS_OVERLAPPED | WS_SYSMENU | WS_CAPTION |
					WS_VISIBLE | WS_MINIMIZEBOX | WS_CLIPSIBLINGS;
	}
	hWnd = CreateWindow(szClassName,
						szWndName,
						dwStyle,
						rect.left,
						rect.top,
						rect.right,
						rect.bottom,
						hParent,
						NULL,
						hAppInstance,
						NULL);

	/* 有効なら、サイズ補正して手前に置く */
	if (hWnd) {
		GetWindowRect(hWnd, &wrect);
		GetClientRect(hWnd, &crect);
		wrect.right += (rect.right - crect.right);
		wrect.bottom += (rect.bottom - crect.bottom);
		SetWindowPos(hWnd, HWND_TOP, wrect.left, wrect.top,
			wrect.right - wrect.left, wrect.bottom - wrect.top, SWP_NOMOVE);
	}

	/* ポップアップウインドウ時はアクティブウインドウを前面に変更 */
	if (bPopupSwnd) {
		SetForegroundWindow(hMainWnd);
	}

	/* 結果を持ち帰る */
	return hWnd;
}

/*-[ スケジューラウインドウ ]------------------------------------------------*/

static char *pszSchedulerTitle[] = {
	"Main Timer",
	"Sub  Timer",
	"OPN Timer A",
	"OPN Timer B",
	"Key Repeat",
	"BEEP",
	"V-SYNC",
	"V/H-BLANK",
#if XM7_VER >= 2
	"Line LSI",
	"RTC Clock",	/* V3.2L02～ */
#else
	"Text Blink",
	"(Reserved)",
#endif
	"WHG Timer A",
	"WHG Timer B",
	"THG Timer A",
	"THG Timer B",
	"FDC (M.Sec)",
	"FDC (Lost)",
	"FDD (Seek)",
	"Tape Sound",
#if defined(MOUSE)
	"Mouse Lost",
#else
	"(Reserved)",
#endif
#if XM7_VER >= 3
	"DMA Start",
#else
	"(Reserved)",
#endif
#if XM7_VER >= 2
	"KeyEnc ACK",
#else
	"(Reserved)",
#endif
#if defined(RSC)
	"RS TxTiming",
	"RS RxTiming",
#else
	"(Reserved)",
	"(Reserved)",
#endif
#if XM7_VER >= 2
	"KeyEnc BEEP",
	"KeyEnc MSG",
#else
	"(Reserved)",
	"(Reserved)",
#endif
#if defined(MOUSE)
	"PTM Timer",
#else
	"(Reserved)",
#endif
#if XM7_VER == 1 && defined(BUBBLE)
	"Bubble Lost",
	"BubbleMPage",
#else
	"(Reserved)",
	"(Reserved)",
#endif
	"(Reserved)",
	"(Reserved)",
	"(Reserved)",
	"(Reserved)",
};

/*
 *	スケジューラウインドウ
 *	セットアップ
 */
static void FASTCALL SetupScheduler(BYTE *p, int x, int y)
{
	int i;
	int j;
	char string[128];
	DWORD dwFrame;
	DWORD dwTime;

	ASSERT(p);
	ASSERT(x > 0);
	ASSERT(y > 0);

	/* VMをロック */
	LockVM();

	/* 一旦スペースで埋める */
	memset(p, 0x20, x * y);

	/* 実行時間 */
	_snprintf(string, sizeof(string), "Execute Time %12d ms",
		dwExecTotal / 1000);
	memcpy(p, string, strlen(string));

	/* フレーム数トータル */
	dwFrame = dwDrawTotal - dwSchedulerFrame[0];
	dwTime = timeGetTime() - dwSchedulerTime[0];
	if (dwTime > 0) {
		dwFrame = dwFrame * 100000 / dwTime;
	}
	else {
		dwSchedulerTime[0] = timeGetTime();
		dwFrame = 0;
	}
	_snprintf(string, sizeof(string), "Frame Rate (Total) %3d.%02d fps",
		dwFrame / 100, dwFrame % 100);
	memcpy(&p[x * 1], string, strlen(string));

	/* フレーム数2sec */
	dwTime = timeGetTime() - dwSchedulerTime[1];
	if (dwTime > 0) {
		dwFrame = dwDrawTotal - dwSchedulerFrame[1];
		dwFrame = dwFrame * 100000 / dwTime;
	}
	else {
		dwFrame = 0;
	}
	if ((dwTime < 0) || (dwTime > 2000)) {
		dwSchedulerTime[1] = timeGetTime();
		dwSchedulerFrame[1] = dwDrawTotal;
	}
	_snprintf(string, sizeof(string), "Frame Rate (Latest)%3d.%02d fps",
		dwFrame / 100, dwFrame % 100);
	memcpy(&p[x * 2], string, strlen(string));

	/* ループ */
	j = 4;
	for (i=0; i<EVENT_MAXNUM; i++) {
		/* タイトル */
		memcpy(&p[x * j], pszSchedulerTitle[i], strlen(pszSchedulerTitle[i]));

		/* カレント、リロード */
		if (event[i].flag != EVENT_NOTUSE) {
			_snprintf(string, sizeof(string), "%4d.%03dms",
				event[i].current / 1000, event[i].current % 1000);
			memcpy(&p[x * j + 12], string, strlen(string));

			_snprintf(string, sizeof(string), "(%4d.%03dms)",
				event[i].reload / 1000, event[i].reload % 1000);
			memcpy(&p[x * j + 23], string, strlen(string));

			/* ステータス */
			switch (event[i].flag) {
				case EVENT_ENABLED:
					strncpy(string, " Enabled", sizeof(string));
					break;

				case EVENT_DISABLED:
					strncpy(string, "Disabled", sizeof(string));
					break;

				default:
					ASSERT(FALSE);
					break;
			}
		}
		else {
			strncpy(string, "", sizeof(string));
		}

		memcpy(&p[x * j + 36], string, strlen(string));
		j++;
	}

	/* オーバーサイクル数 */
	strncpy(string,  "|  Over Cycle", sizeof(string));
	memcpy(&p[x * 0 + 30], string, strlen(string));
	_snprintf(string, sizeof(string), "| Main %7d", main_overcycles);
	memcpy(&p[x * 1 + 30], string, strlen(string));
	_snprintf(string, sizeof(string), "| Sub  %7d", sub_overcycles);
	memcpy(&p[x * 2 + 30], string, strlen(string));

#if defined(DEBUG)
	if (dwFrame == 0) {
		dwTime = 100000;
	}
	else {
		dwTime = 100000 / dwFrame;
	}

	/* 実働時間(でばっぐば～じょん用) */
	_snprintf(string, sizeof(string), "Exec Time (CPU)  %4d.%03d ms (%3d\%)",
		dwCpuExecTime / 1000, dwCpuExecTime % 1000, dwCpuExecTime / dwTime);
	memcpy(&p[x * (j + 1)], string, strlen(string));

	_snprintf(string, sizeof(string), "Exec Time (DRAW) %4d.%03d ms (%3d\%)",
		dwDrawExecTime / 1000, dwDrawExecTime % 1000, dwDrawExecTime / dwTime);
	memcpy(&p[x * (j + 2)], string, strlen(string));

	_snprintf(string, sizeof(string), "Exec Time (POLL) %4d.%03d ms (%3d\%)",
		dwPollExecTime / 1000, dwPollExecTime % 1000, dwPollExecTime / dwTime);
	memcpy(&p[x * (j + 3)], string, strlen(string));

	i = (dwTime * 100) - (dwCpuExecTime + dwDrawExecTime + dwPollExecTime);
	if (i < 0) {
		i = 0;
	}
	_snprintf(string, sizeof(string), "Idle Time        %4d.%03d ms (%3d\%)",
		i / 1000, i % 1000, i / dwTime);
	memcpy(&p[x * (j + 4)], string, strlen(string));
#endif

	/* VMをアンロック */
	UnlockVM();
}

/*
 *	スケジューラウインドウ
 *	描画
 */
static void FASTCALL DrawScheduler(HWND hWnd, HDC hDC)
{
	RECT rect;
	int x, y;

	ASSERT(hWnd);
	ASSERT(hDC);

	/* ウインドウジオメトリを得る */
	GetClientRect(hWnd, &rect);
	x = rect.right / lCharWidth;
	y = rect.bottom / lCharHeight;
	if ((x == 0) || (y == 0)) {
		return;
	}

	/* セットアップ */
	if (!pScheduler) {
		return;
	}
	SetupScheduler(pScheduler, x, y);

	/* 描画 */
	DrawWindowText(hDC, pScheduler, x, y);
}

/*
 *	スケジューラウインドウ
 *	リフレッシュ
 */
void FASTCALL RefreshScheduler(void)
{
	HWND hWnd;
	HDC hDC;

	/* 常に呼ばれるので、存在チェックすること */
	if (hSubWnd[SWND_SCHEDULER] == NULL) {
		return;
	}

	/* 描画 */
	hWnd = hSubWnd[SWND_SCHEDULER];
	hDC = GetDC(hWnd);
	DrawScheduler(hWnd, hDC);
	ReleaseDC(hWnd, hDC);
}

/*
 *	スケジューラウインドウ
 *	再描画
 */
static void FASTCALL PaintScheduler(HWND hWnd)
{
	HDC hDC;
	PAINTSTRUCT ps;
	RECT rect;
	BYTE *p;
	int x, y;

	ASSERT(hWnd);

	/* ポインタを設定(存在しなければ何もしない) */
	p = pScheduler;
	if (!p) {
		return;
	}

	/* ウインドウジオメトリを得る */
	GetClientRect(hWnd, &rect);
	x = rect.right / lCharWidth;
	y = rect.bottom / lCharHeight;

	/* 後半エリアをFFで埋める */
	if ((x > 0) && (y > 0)) {
		memset(&p[x * y], 0xff, x * y);
	}

	/* 描画 */
	hDC = BeginPaint(hWnd, &ps);
	ASSERT(hDC);
	DrawScheduler(hWnd, hDC);
	EndPaint(hWnd, &ps);
}

/*
 *	スケジューラウインドウ
 *	ウインドウプロシージャ
 */
static LRESULT CALLBACK SchedulerProc(HWND hWnd, UINT message,
								 WPARAM wParam, LPARAM lParam)
{
	/* メッセージ別 */
	switch (message) {
		/* ウインドウ再描画 */
		case WM_PAINT:
			/* ロックが必要 */
			LockVM();
			PaintScheduler(hWnd);
			UnlockVM();
			return 0;

		/* ウインドウ削除 */
		case WM_DESTROY:
			LockVM();

			/* メインウインドウへ自動通知 */
			DestroySubWindow(hWnd, &pScheduler, NULL);

			UnlockVM();
			break;
	}

	/* デフォルト ウインドウプロシージャ */
	return DefWindowProc(hWnd, message, wParam, lParam);
}

/*
 *	スケジューラウインドウ
 *	ウインドウ作成
 */
HWND FASTCALL CreateScheduler(HWND hParent, int index)
{
	WNDCLASSEX wcex;
	char szClassName[] = "XM7_Scheduler";
	char szWndName[128];
	RECT rect;
	RECT crect, wrect;
	HWND hWnd;
	DWORD dwStyle;

	ASSERT(hParent);

	/* ウインドウ矩形を計算 */
	PositioningSubWindow(hParent, &rect, index);
	rect.right = lCharWidth * 44;
#if defined(DEBUG)
	rect.bottom = lCharHeight * (EVENT_MAXNUM + 9);
#else
	rect.bottom = lCharHeight * (EVENT_MAXNUM + 4);
#endif

	/* ウインドウタイトルを決定、バッファ確保 */
	LoadString(hAppInstance, IDS_SWND_SCHEDULER,
				szWndName, sizeof(szWndName));
	pScheduler = malloc(2 * (rect.right / lCharWidth) *
								(rect.bottom / lCharHeight));

	/* 時間、フレーム数初期化 */
	dwSchedulerTime[0] = timeGetTime();
	dwSchedulerTime[1] = dwSchedulerTime[0];
	dwSchedulerFrame[0] = dwDrawTotal;
	dwSchedulerFrame[1] = dwSchedulerFrame[0];

	/* ウインドウクラスの登録 */
	memset(&wcex, 0, sizeof(wcex));
	wcex.cbSize = sizeof(wcex);
	wcex.style = CS_VREDRAW | CS_HREDRAW;
	wcex.lpfnWndProc = SchedulerProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hAppInstance;
	wcex.hIcon = LoadIcon(hAppInstance, MAKEINTRESOURCE(IDI_WNDICON));
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	wcex.lpszMenuName = NULL;
	wcex.lpszClassName = szClassName;
	wcex.hIconSm = LoadIcon(hAppInstance, MAKEINTRESOURCE(IDI_WNDICON));
	RegisterClassEx(&wcex);

	/* ウインドウ作成 */
	if (bPopupSwnd) {
		dwStyle =	WS_POPUP | WS_OVERLAPPED | WS_SYSMENU | WS_CAPTION |
					WS_VISIBLE | WS_MINIMIZEBOX | WS_BORDER;
	}
	else {
		dwStyle = WS_CHILD | WS_OVERLAPPED | WS_SYSMENU | WS_CAPTION |
					WS_VISIBLE | WS_MINIMIZEBOX | WS_CLIPSIBLINGS;
	}
	hWnd = CreateWindow(szClassName,
						szWndName,
						dwStyle,
						rect.left,
						rect.top,
						rect.right,
						rect.bottom,
						hParent,
						NULL,
						hAppInstance,
						NULL);

	/* 有効なら、サイズ補正して手前に置く */
	if (hWnd) {
		GetWindowRect(hWnd, &wrect);
		GetClientRect(hWnd, &crect);
		wrect.right += (rect.right - crect.right);
		wrect.bottom += (rect.bottom - crect.bottom);
		SetWindowPos(hWnd, HWND_TOP, wrect.left, wrect.top,
			wrect.right - wrect.left, wrect.bottom - wrect.top, SWP_NOMOVE);
	}

	/* ポップアップウインドウ時はアクティブウインドウを前面に変更 */
	if (bPopupSwnd) {
		SetForegroundWindow(hMainWnd);
	}

	/* 結果を持ち帰る */
	return hWnd;
}

/*-[ レジスタ内容変更ダイアログ ]--------------------------------------------*/

/*
 *	レジスタ内容変更ダイアログ
 *	ダイアログ初期化
 */
static BOOL FASTCALL CPURegDlgInit(HWND hDlg)
{
	static const char *RegName[] = {
		"CC", "A", "B", "DP", "X", "Y", "U", "S", "D", "", "IR", "PC"
	};
#if XM7_VER == 1 && defined(Z80CARD)
	static const char *RegNameZ80[] = {
		"AF",	"BC",	"DE",	"HL",	"IX",	"IY",	"SP",	"PC",
	};
#endif

	HWND hWnd;
	RECT prect;
	RECT drect;
	cpu6809_t *pReg;
	char tmp[16];
	char tmp2[128];
	char string[128];
	WORD dat;
	UINT id;

	ASSERT(hDlg);

	/* 親ウインドウの中央に設定 */
	hWnd = GetParent(hDlg);
	GetWindowRect(hWnd, &prect);
	GetWindowRect(hDlg, &drect);
	drect.right -= drect.left;
	drect.bottom -= drect.top;
	drect.left = (prect.right - prect.left) / 2 + prect.left;
	drect.left -= (drect.right / 2);
	drect.top = (prect.bottom - prect.top) / 2 + prect.top;
	drect.top -= (drect.bottom / 2);
	MoveWindow(hDlg, drect.left, drect.top, drect.right, drect.bottom, FALSE);

	/* 初期設定 */
	switch (nRegDlgCPU) {
		case MAINCPU :	id = IDS_SWND_CPU_MAIN;
						pReg = &maincpu;
						break;
		case SUBCPU :	id = IDS_SWND_CPU_SUB;
						pReg = &subcpu;
						break;
#if XM7_VER == 1
#if defined(JSUB)
		case JSUBCPU :	id = IDS_SWND_CPU_JSUB;
						pReg = &jsubcpu;
						break;
#endif
#if defined(Z80CARD)
		case MAINZ80 :	id = IDS_SWND_CPU_MAINZ80;
						pReg = NULL;
						break;
#endif
#endif
	}
	LoadString(hAppInstance, id, tmp, sizeof(tmp));

	/* アドレスを設定 */
	hWnd = GetDlgItem(hDlg, IDC_REGLABEL);
	ASSERT(hWnd);
#if XM7_VER == 1 && defined(Z80CARD)
	if (nRegDlgCPU == MAINZ80) {
		if (LoadString(hAppInstance, IDS_SWND_REG_MESSAGE,
						tmp2, sizeof(tmp2)) == 0) {
			_snprintf(string, sizeof(string), "%sレジスタ (%s)",
				RegNameZ80[nRegDlgNo], "Z80");
		}
		else {
			_snprintf(string, sizeof(string), tmp2,
				RegNameZ80[nRegDlgNo], "Z80");
		}
	}
	else {
		if (LoadString(hAppInstance, IDS_SWND_REG_MESSAGE,
						tmp2, sizeof(tmp2)) == 0) {
			_snprintf(string, sizeof(string), "%sレジスタ (%s)",
				RegName[nRegDlgNo], tmp);
		}
		else {
			_snprintf(string, sizeof(string), tmp2, RegName[nRegDlgNo], tmp);
		}
	}
#else
	if (LoadString(hAppInstance, IDS_SWND_REG_MESSAGE,
					tmp2, sizeof(tmp2)) == 0) {
		_snprintf(string, sizeof(string), "%sレジスタ (%s)",
			RegName[nRegDlgNo], tmp);
	}
	else {
		_snprintf(string, sizeof(string), tmp2, RegName[nRegDlgNo], tmp);
	}
#endif
	SetWindowText(hWnd, string);

	/* 元データを設定 */
	hWnd = GetDlgItem(hDlg, IDC_REGEDIT);
	ASSERT(hWnd);

#if XM7_VER == 1 && defined(Z80CARD)
	if (nRegDlgCPU == MAINZ80) {
		switch (nRegDlgNo) {
			case 0:	/* AF */
					dat = (WORD)((mainz80.regs8[REGID_A] << 8) |
						mainz80.regs8[REGID_F]);
					break;
			case 1:	/* BC */
					dat = (WORD)((mainz80.regs8[REGID_B] << 8) |
						mainz80.regs8[REGID_C]);
					break;
			case 2:	/* DE */
					dat = (WORD)((mainz80.regs8[REGID_D] << 8) |
						mainz80.regs8[REGID_E]);
					break;
			case 3:	/* HL */
					dat = (WORD)((mainz80.regs8[REGID_H] << 8) |
						mainz80.regs8[REGID_L]);
					break;
			case 4:	/* IX */
					dat = (WORD)((mainz80.regs8[REGID_IXH] << 8) |
						mainz80.regs8[REGID_IXL]);
					break;
			case 5:	/* IY */
					dat = (WORD)((mainz80.regs8[REGID_IYH] << 8) |
						mainz80.regs8[REGID_IYL]);
					break;
			case 6:	/* SP */
					dat = mainz80.sp;
					break;
			case 7:	/* PC */
					dat = mainz80.pc;
					break;
			default:
					ASSERT(FALSE);
		}
		_snprintf(string, sizeof(string), "%04X", dat);
		SetWindowText(hWnd, string);

		return TRUE;
	}
#endif

	switch (nRegDlgNo) {
		case 0:	/* CC */
				dat = pReg -> cc;
				break;
		case 1:	/* A */
				dat = pReg -> acc.h.a;
				break;
		case 2:	/* B */
				dat = pReg -> acc.h.b;
				break;
		case 3:	/* DP */
				dat = pReg -> dp;
				break;
		case 4:	/* X */
				dat = pReg -> x;
				break;
		case 5:	/* Y */
				dat = pReg -> y;
				break;
		case 6:	/* U */
				dat = pReg -> u;
				break;
		case 7:	/* S */
				dat = pReg -> s;
				break;
		case 8:	/* D */
				dat = pReg -> acc.d;
				break;
		case 11:	/* PC */
				dat = pReg -> pc;
				break;
		default:
				ASSERT(FALSE);
	}
	if (nRegDlgNo < 4) {
		/* 8ビットレジスタ */
		_snprintf(string, sizeof(string), "%02X", dat);
	}
	else {
		/* 16ビットレジスタ */
		_snprintf(string, sizeof(string), "%04X", dat);
	}
	SetWindowText(hWnd, string);

	return TRUE;
}

/*
 *	レジスタ内容変更ダイアログ
 *	ダイアログOK
 */
static void FASTCALL CPURegDlgOK(HWND hDlg)
{
	HWND hWnd;
	cpu6809_t *pReg;
	char string[128];
	WORD dat;

	ASSERT(hDlg);
	LockVM();

	/* 初期設定 */
	switch (nRegDlgCPU) {
		case MAINCPU :	pReg = &maincpu;
						break;
		case SUBCPU :	pReg = &subcpu;
						break;
#if XM7_VER == 1
#if defined(JSUB)
		case JSUBCPU :	pReg = &jsubcpu;
						break;
#endif
#endif
	}

	/* 現在の値を取得 */
	hWnd = GetDlgItem(hDlg, IDC_REGEDIT);
	ASSERT(hWnd);
	GetWindowText(hWnd, string, sizeof(string) - 1);
	dat = (WORD)strtol(string, NULL, 16);

#if XM7_VER == 1 && defined(Z80CARD)
	if (nRegDlgCPU == MAINZ80) {
		/* データを設定 */
		switch (nRegDlgNo) {
			case 0:	/* AF */
					mainz80.regs8[REGID_A] = (BYTE)((dat >> 8) & 0xff);
					mainz80.regs8[REGID_F] = (BYTE)(dat & 0xff);
					break;
			case 1:	/* BC */
					mainz80.regs8[REGID_B] = (BYTE)((dat >> 8) & 0xff);
					mainz80.regs8[REGID_C] = (BYTE)(dat & 0xff);
					break;
			case 2:	/* DE */
					mainz80.regs8[REGID_D] = (BYTE)((dat >> 8) & 0xff);
					mainz80.regs8[REGID_E] = (BYTE)(dat & 0xff);
					break;
			case 3:	/* HL */
					mainz80.regs8[REGID_H] = (BYTE)((dat >> 8) & 0xff);
					mainz80.regs8[REGID_L] = (BYTE)(dat & 0xff);
					break;
			case 4:	/* IX */
					mainz80.regs8[REGID_IXH] = (BYTE)((dat >> 8) & 0xff);
					mainz80.regs8[REGID_IXL] = (BYTE)(dat & 0xff);
					break;
			case 5:	/* IY */
					mainz80.regs8[REGID_IYH] = (BYTE)((dat >> 8) & 0xff);
					mainz80.regs8[REGID_IYL] = (BYTE)(dat & 0xff);
					break;
			case 6:	/* SP */
					mainz80.sp = dat;
					break;
			case 7:	/* PC */
					mainz80.pc = dat;
					break;
			default:
					ASSERT(FALSE);
		}
	}
	else {
		/* データを設定 */
		switch (nRegDlgNo) {
			case 0:	/* CC */
					pReg -> cc = (BYTE)dat;
					break;
			case 1:	/* A */
					pReg -> acc.h.a = (BYTE)dat;
					break;
			case 2:	/* B */
					pReg -> acc.h.b = (BYTE)dat;
					break;
			case 3:	/* DP */
					pReg -> dp = (BYTE)dat;
					break;
			case 4:	/* X */
					pReg -> x = dat;
					break;
			case 5:	/* Y */
					pReg -> y = dat;
					break;
			case 6:	/* U */
					pReg -> u = dat;
					break;
			case 7:	/* S */
					pReg -> s = dat;
					break;
			case 8:	/* D */
					pReg -> acc.d = dat;
					break;
			case 11:	/* PC */
					pReg -> pc = dat;
					break;
			default:
					ASSERT(FALSE);
		}
	}
#else
	/* データを設定 */
	switch (nRegDlgNo) {
		case 0:	/* CC */
				pReg -> cc = (BYTE)dat;
				break;
		case 1:	/* A */
				pReg -> acc.h.a = (BYTE)dat;
				break;
		case 2:	/* B */
				pReg -> acc.h.b = (BYTE)dat;
				break;
		case 3:	/* DP */
				pReg -> dp = (BYTE)dat;
				break;
		case 4:	/* X */
				pReg -> x = dat;
				break;
		case 5:	/* Y */
				pReg -> y = dat;
				break;
		case 6:	/* U */
				pReg -> u = dat;
				break;
		case 7:	/* S */
				pReg -> s = dat;
				break;
		case 8:	/* D */
				pReg -> acc.d = dat;
				break;
		case 11:	/* PC */
				pReg -> pc = dat;
				break;
		default:
				ASSERT(FALSE);
	}
#endif

	UnlockVM();
}

/*
 *	レジスタ内容変更ダイアログ
 *	ダイアログプロシージャ
 */
static BOOL CALLBACK CPURegDlgProc(HWND hDlg, UINT iMsg,
									WPARAM wParam, LPARAM lParam)
{
	UNUSED(lParam);

	switch (iMsg) {
		/* ダイアログ初期化 */
		case WM_INITDIALOG:
			return CPURegDlgInit(hDlg);

		/* コマンド処理 */
		case WM_COMMAND:
			switch (LOWORD(wParam)) {
				/* OK・キャンセル */
				case IDOK:
				case IDCANCEL:
					if (LOWORD(wParam) == IDOK) {
						CPURegDlgOK(hDlg);
					}
					EndDialog(hDlg, LOWORD(wParam));
					InvalidateRect(hDrawWnd, NULL, FALSE);
					return TRUE;
			}
			break;
	}

	/* それ以外は、FALSE */
	return FALSE;
}

/*
 *	レジスタ内容変更
 */
static BOOL FASTCALL CPURegDlg(HWND hWnd, BYTE nCPU, int x, int y)
{
	int ret;

	ASSERT(hWnd);

	/* パラメータを保存・設定 */
	nRegDlgCPU = nCPU;

#if XM7_VER == 1 && defined(Z80CARD)
	if (nCPU == MAINZ80) {
		/* レジスタ番号をセット */
		nRegDlgNo = (BYTE)(((x / 20) * 4) + y);

		/* 座標チェック */
		if ((x % 20) >= 8) {
			return FALSE;
		}
	}
	else {
		/* レジスタ番号をセット */
		nRegDlgNo = (BYTE)(((x / 10) * 4) + y);

		/* IRレジスタ・座標チェック */
		if (((nRegDlgNo == 8) && (x != -1)) ||
			(nRegDlgNo == 9) || (nRegDlgNo == 10) || ((x % 10) >= 7)) {
			return FALSE;
		}
	}
#else
	/* レジスタ番号をセット */
	nRegDlgNo = (BYTE)(((x / 10) * 4) + y);

	/* IRレジスタ・座標チェック */
	if (((nRegDlgNo == 8) && (x != -1)) ||
		 (nRegDlgNo == 9) || (nRegDlgNo == 10) || ((x % 10) >= 7)) {
		return FALSE;
	}
#endif

	/* モーダルダイアログ実行 */
	ret = DialogBox(hAppInstance, MAKEINTRESOURCE(IDD_REGDLG), hWnd, CPURegDlgProc);
	if (ret != IDOK) {
		return FALSE;
	}
	return TRUE;
}

/*-[ CPUレジスタウインドウ ]-------------------------------------------------*/

/*
 *	CPUレジスタウインドウ
 *	セットアップ
 */
static void FASTCALL SetupCPURegister(BYTE nCPU, BYTE *p, int x, int y)
{
	char buf[128];
	cpu6809_t *pReg;

	ASSERT(nCPU < MAXCPU);
	ASSERT(p);
	ASSERT(x > 0);
	ASSERT(y > 0);

	/* 一旦スペースで埋める */
	memset(p, 0x20, x * y);

	/* レジスタバッファを得る */
	switch (nCPU) {
		case MAINCPU :	pReg = &maincpu;
						break;
		case SUBCPU :	pReg = &subcpu;
						break;
#if XM7_VER == 1
#if defined(JSUB)
		case JSUBCPU :	pReg = &jsubcpu;
						break;
#endif
#endif
	}

	/* セット */
	_snprintf(buf, sizeof(buf), 
		"CC   %02X   X  %04X", pReg->cc, pReg->x);
	memcpy(&p[0 * x + 0], buf, strlen(buf));
	_snprintf(buf, sizeof(buf), 
		"A    %02X   Y  %04X", pReg->acc.h.a, pReg->y);
	memcpy(&p[1 * x + 0], buf, strlen(buf));
	_snprintf(buf, sizeof(buf), 
		"B    %02X   U  %04X   IR %04X", pReg->acc.h.b, pReg->u, pReg->intr);
	memcpy(&p[2 * x + 0], buf, strlen(buf));
	_snprintf(buf, sizeof(buf), 
		"DP   %02X   S  %04X   PC %04X", pReg->dp, pReg->s, pReg->pc);
	memcpy(&p[3 * x + 0], buf, strlen(buf));
}

/*
 *	CPUレジスタウインドウ(Z80)
 *	セットアップ
 */
#if XM7_VER == 1 && defined(Z80CARD)
static void FASTCALL SetupCPURegisterZ80(BYTE nCPU, BYTE *p, int x, int y)
{
	char buf[128];

	ASSERT(nCPU != MAINZ80);
	ASSERT(p);
	ASSERT(x > 0);
	ASSERT(y > 0);
	UNUSED(nCPU);

	/* 一旦スペースで埋める */
	memset(p, 0x20, x * y);

	/* セット */
	_snprintf(buf, sizeof(buf), 
		"AF  %02X%02X  AF' %04X  IX  %02X%02X",
		mainz80.regs8[REGID_A], mainz80.regs8[REGID_F], mainz80.saf,
		mainz80.regs8[REGID_IXH], mainz80.regs8[REGID_IXL]);
	memcpy(&p[0 * x + 0], buf, strlen(buf));
	_snprintf(buf, sizeof(buf), 
		"BC  %02X%02X  BC' %04X  IY  %02X%02X",
		mainz80.regs8[REGID_B], mainz80.regs8[REGID_C], mainz80.sbc,
		mainz80.regs8[REGID_IYH], mainz80.regs8[REGID_IYL]);
	memcpy(&p[1 * x + 0], buf, strlen(buf));
	_snprintf(buf, sizeof(buf), 
		"DE  %02X%02X  DE' %04X  SP  %04X",
		mainz80.regs8[REGID_D], mainz80.regs8[REGID_E], mainz80.sde,
		mainz80.sp);
	memcpy(&p[2 * x + 0], buf, strlen(buf));
	_snprintf(buf, sizeof(buf), 
		"HL  %02X%02X  HL' %04X  PC  %04X",
		mainz80.regs8[REGID_H], mainz80.regs8[REGID_L], mainz80.shl,
		mainz80.pc);
	memcpy(&p[3 * x + 0], buf, strlen(buf));
}
#endif

/*
 *	CPUレジスタウインドウ
 *	描画
 */
static void FASTCALL DrawCPURegister(HWND hWnd, HDC hDC, BYTE nCPU)
{
	RECT rect;
	BYTE *p;
	int x, y;

	ASSERT(hWnd);
	ASSERT(hDC);

	/* Drawバッファを得る(存在しなければ何もしない) */
	p = pCPURegister[nCPU];
	if (!p) {
		return;
	}

	/* ウインドウジオメトリを得る */
	GetClientRect(hWnd, &rect);
	x = rect.right / lCharWidth;
	y = rect.bottom / lCharHeight;
	if ((x == 0) || (y == 0)) {
		return;
	}

	/* セットアップ */
#if XM7_VER == 1 && defined(Z80CARD)
	if (nCPU == MAINZ80) {
		SetupCPURegisterZ80(nCPU, p, x, y);
	}
	else {
		SetupCPURegister(nCPU, p, x, y);
	}
#else
	SetupCPURegister(nCPU, p, x, y);
#endif

	/* 描画 */
	DrawWindowText(hDC, p, x, y);
}

/*
 *	CPUレジスタウインドウ
 *	リフレッシュ
 */
void FASTCALL RefreshCPURegister(void)
{
	HWND hWnd;
	HDC hDC;

	/* メインCPU */
	if (hSubWnd[SWND_CPUREG_MAIN]) {
		hWnd = hSubWnd[SWND_CPUREG_MAIN];
		hDC = GetDC(hWnd);
		DrawCPURegister(hWnd, hDC, MAINCPU);
		ReleaseDC(hWnd, hDC);
	}

	/* サブCPU */
	if (hSubWnd[SWND_CPUREG_SUB]) {
		hWnd = hSubWnd[SWND_CPUREG_SUB];
		hDC = GetDC(hWnd);
		DrawCPURegister(hWnd, hDC, SUBCPU);
		ReleaseDC(hWnd, hDC);
	}

#if XM7_VER == 1
#if defined(JSUB)
	/* 日本語サブCPU */
	if (hSubWnd[SWND_CPUREG_JSUB]) {
		hWnd = hSubWnd[SWND_CPUREG_JSUB];
		hDC = GetDC(hWnd);
		DrawCPURegister(hWnd, hDC, JSUBCPU);
		ReleaseDC(hWnd, hDC);
	}
#endif

#if defined(JSUB)
	/* メインCPU(Z80) */
	if (hSubWnd[SWND_CPUREG_Z80]) {
		hWnd = hSubWnd[SWND_CPUREG_Z80];
		hDC = GetDC(hWnd);
		DrawCPURegister(hWnd, hDC, MAINZ80);
		ReleaseDC(hWnd, hDC);
	}
#endif
#endif
}

/*
 *	CPUレジスタウインドウ
 *	再描画
 */
static void FASTCALL PaintCPURegister(HWND hWnd)
{
	HDC hDC;
	PAINTSTRUCT ps;
	int i;
	RECT rect;
	BYTE *p;
	int x, y;
	BYTE nCPU;
	
	ASSERT(hWnd);

	/* Drawバッファを得る(存在しなければ何もしない) */
	for (i=0; i<SWND_MAXNUM; i++) {
		if (hSubWnd[i] == hWnd) {
			nCPU = (BYTE)((i - SWND_CPUREG_MAIN) % MAXCPU);
			p = pCPURegister[nCPU];
			break;
		}
	}

	if (!p) {
		return;
	}

	/* ウインドウジオメトリを得る */
	GetClientRect(hWnd, &rect);
	x = rect.right / lCharWidth;
	y = rect.bottom / lCharHeight;

	/* 後半エリアをFFで埋める */
	if ((x > 0) && (y > 0)) {
		memset(&p[x * y], 0xff, x * y);
	}

	/* 描画 */
	hDC = BeginPaint(hWnd, &ps);
	ASSERT(hDC);
	DrawCPURegister(hWnd, hDC, nCPU);
	EndPaint(hWnd, &ps);
}

/*
 *	CPUレジスタウインドウ
 *	ウインドウプロシージャ
 */
static LRESULT CALLBACK CPURegisterProc(HWND hWnd, UINT message,
								 WPARAM wParam, LPARAM lParam, BYTE nCPU)
{
	HMENU hMenu;
	POINT point;

	/* メッセージ別 */
	switch (message) {
		/* ウインドウ再描画 */
		case WM_PAINT:
			LockVM();
			PaintCPURegister(hWnd);
			UnlockVM();
			return 0;

		/* ウインドウ削除 */
		case WM_DESTROY:
			LockVM();

			/* メインウインドウへ自動通知 */
			DestroySubWindow(hWnd, &pCPURegister[nCPU], hCPURegister[nCPU]);

			UnlockVM();
			break;

		/* レジスタ内容変更 */
		case WM_LBUTTONDBLCLK:
#if defined(MOUSE)
			/* マウスエミュレーション使用時は無効 */
			if (mos_capture && !stopreq_flag && run_flag) {
				return FALSE;
			}
#endif

			/* カーソル位置から、決定 */
			point.x = LOWORD(lParam) / lCharWidth;
			point.y = HIWORD(lParam) / lCharHeight;
			CPURegDlg(hWnd, nCPU, point.x, point.y);
			return 0;

		/* コンテキストメニュー */
		case WM_CONTEXTMENU:
#if defined(MOUSE)
			/* マウスエミュレーション使用時は無効 */
			if (mos_capture && !stopreq_flag && run_flag) {
				return FALSE;
			}
#endif

			/* コンテキストメニューをロード */
			hMenu = GetSubMenu(hCPURegister[nCPU], 0);

			/* コンテキストメニューを実行 */
			point.x = LOWORD(lParam);
			point.y = HIWORD(lParam);
			TrackPopupMenu(hMenu, 0, point.x, point.y, 0, hWnd, NULL);
			return 0;

		/* コマンド */
		case WM_COMMAND:
#if XM7_VER == 1 && defined(Z80CARD)
			if (nCPU == MAINZ80) {
				CPURegDlg(hWnd, nCPU, -1, LOWORD(wParam) - IDM_REG_AF);
			}
			else {
				CPURegDlg(hWnd, nCPU, -1, LOWORD(wParam) - IDM_REG_CC);
			}
#else
			CPURegDlg(hWnd, nCPU, -1, LOWORD(wParam) - IDM_REG_CC);
#endif
			break;
	}

	/* デフォルト ウインドウプロシージャ */
	return DefWindowProc(hWnd, message, wParam, lParam);
}

/*
 *	CPUレジスタウインドウ(メイン)
 *	ウインドウプロシージャ
 */
static LRESULT CALLBACK CPURegisterProcMain(HWND hWnd, UINT message,
								 WPARAM wParam, LPARAM lParam)
{
	return CPURegisterProc(hWnd, message, wParam, lParam, MAINCPU);
}

/*
 *	CPUレジスタウインドウ(サブ)
 *	ウインドウプロシージャ
 */
static LRESULT CALLBACK CPURegisterProcSub(HWND hWnd, UINT message,
								 WPARAM wParam, LPARAM lParam)
{
	return CPURegisterProc(hWnd, message, wParam, lParam, SUBCPU);
}

#if XM7_VER == 1
/*
 *	CPUレジスタウインドウ(日本語サブ)
 *	ウインドウプロシージャ
 */
#if defined(JSUB)
static LRESULT CALLBACK CPURegisterProcJsub(HWND hWnd, UINT message,
								 WPARAM wParam, LPARAM lParam)
{
	return CPURegisterProc(hWnd, message, wParam, lParam, JSUBCPU);
}
#endif

/*
 *	CPUレジスタウインドウ(メインZ80)
 *	ウインドウプロシージャ
 */
#if defined(Z80CARD)
static LRESULT CALLBACK CPURegisterProcZ80(HWND hWnd, UINT message,
								 WPARAM wParam, LPARAM lParam)
{
	return CPURegisterProc(hWnd, message, wParam, lParam, MAINZ80);
}
#endif
#endif

/*
 *	CPUレジスタウインドウ
 *	ウインドウ作成
 */
HWND FASTCALL CreateCPURegister(HWND hParent, BYTE nCPU, int index)
{
	WNDCLASSEX wcex;
	char szClassNameMain[] = "XM7_CPURegisterMain";
	char szClassNameSub[] = "XM7_CPURegisterSub";
#if XM7_VER == 1
#if defined(JSUB)
	char szClassNameJsub[] = "XM7_CPURegisterJsub";
#endif
#if defined(Z80CARD)
	char szClassNameZ80[] = "XM7_CPURegisterZ80";
#endif
#endif
	char *szClassName;
	char szWndName[128];
	RECT rect;
	RECT crect, wrect;
	HWND hWnd;
	UINT id;
	DWORD dwStyle;

	ASSERT(hParent);

	/* ウインドウ矩形を計算 */
	PositioningSubWindow(hParent, &rect, index);
	rect.right = lCharWidth * 27;
	rect.bottom = lCharHeight * 4;

	/* ウインドウタイトルを決定、バッファ確保 */
	switch (nCPU) {
		case MAINCPU :	id = IDS_SWND_CPUREG_MAIN;
						szClassName = szClassNameMain;
						break;
		case SUBCPU :	id = IDS_SWND_CPUREG_SUB;
						szClassName = szClassNameSub;
						break;
#if XM7_VER == 1
#if defined(JSUB)
		case JSUBCPU :	id = IDS_SWND_CPUREG_JSUB;
						szClassName = szClassNameJsub;
						break;
#endif
#if defined(Z80CARD)
		case MAINZ80 :	id = IDS_SWND_CPUREG_Z80;
						szClassName = szClassNameZ80;
						rect.right = lCharWidth * 28;
						rect.bottom = lCharHeight * 4;
						break;
#endif
#endif
	}
	LoadString(hAppInstance, id, szWndName, sizeof(szWndName));
	pCPURegister[nCPU] = malloc(2 * (rect.right / lCharWidth) *
							(rect.bottom / lCharHeight));
#if XM7_VER == 1 && defined(Z80CARD)
	if (nCPU == MAINZ80) {
		hCPURegister[nCPU] = LoadMenu(hAppInstance,
								MAKEINTRESOURCE(IDR_CPUREGZ80MENU));
	}
	else {
		hCPURegister[nCPU] = LoadMenu(hAppInstance,
								MAKEINTRESOURCE(IDR_CPUREGMENU));
	}
#else
	hCPURegister[nCPU] = LoadMenu(hAppInstance,
							MAKEINTRESOURCE(IDR_CPUREGMENU));
#endif

	/* ウインドウクラスの登録 */
	memset(&wcex, 0, sizeof(wcex));
	wcex.cbSize = sizeof(wcex);
	wcex.style = CS_VREDRAW | CS_HREDRAW | CS_DBLCLKS;
	switch (nCPU) {
		case MAINCPU :	wcex.lpfnWndProc = CPURegisterProcMain;
						break;
		case SUBCPU :	wcex.lpfnWndProc = CPURegisterProcSub;
						break;
#if XM7_VER == 1
#if defined(JSUB)
		case JSUBCPU :	wcex.lpfnWndProc = CPURegisterProcJsub;
						break;
#endif
#if defined(Z80CARD)
		case MAINZ80 :	wcex.lpfnWndProc = CPURegisterProcZ80;
						break;
#endif
#endif
	}
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hAppInstance;
	wcex.hIcon = LoadIcon(hAppInstance, MAKEINTRESOURCE(IDI_WNDICON));
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	wcex.lpszMenuName = NULL;
	wcex.lpszClassName = szClassName;
	wcex.hIconSm = LoadIcon(hAppInstance, MAKEINTRESOURCE(IDI_WNDICON));
	RegisterClassEx(&wcex);

	/* ウインドウ作成 */
	if (bPopupSwnd) {
		dwStyle =	WS_POPUP | WS_OVERLAPPED | WS_SYSMENU | WS_CAPTION |
					WS_VISIBLE | WS_MINIMIZEBOX | WS_BORDER;
	}
	else {
		dwStyle = WS_CHILD | WS_OVERLAPPED | WS_SYSMENU | WS_CAPTION |
					WS_VISIBLE | WS_MINIMIZEBOX | WS_CLIPSIBLINGS;
	}
	hWnd = CreateWindow(szClassName,
						szWndName,
						dwStyle,
						rect.left,
						rect.top,
						rect.right,
						rect.bottom,
						hParent,
						NULL,
						hAppInstance,
						NULL);

	/* 有効なら、サイズ補正して手前に置く */
	if (hWnd) {
		GetWindowRect(hWnd, &wrect);
		GetClientRect(hWnd, &crect);
		wrect.right += (rect.right - crect.right);
		wrect.bottom += (rect.bottom - crect.bottom);
		SetWindowPos(hWnd, HWND_TOP, wrect.left, wrect.top,
			wrect.right - wrect.left, wrect.bottom - wrect.top, SWP_NOMOVE);
	}

	/* ポップアップウインドウ時はアクティブウインドウを前面に変更 */
	if (bPopupSwnd) {
		SetForegroundWindow(hMainWnd);
	}

	/* 結果を持ち帰る */
	return hWnd;
}

/*-[ スタック・ブレークポイントジャンプ ]------------------------------------*/

/*
 *	スタックジャンプメニュー挿入
 */
static void FASTCALL InsertStackJumpMenu(HMENU hMenu, BYTE nCPU, UINT id)
{
	BYTE (FASTCALL *readbnio)(WORD addr);
	MENUITEMINFO mii;
	char string[128];
	WORD addr;
	int i;

	/* メニュー構造体初期化 */
	memset(&mii, 0, sizeof(mii));
	mii.cbSize = 44;
	mii.fMask = MIIM_TYPE | MIIM_STATE | MIIM_ID;
	mii.fType = MFT_STRING;
	mii.fState = MFS_ENABLED;

	/* メニューすべて削除 */
	while (GetMenuItemCount(hMenu) > 0) {
		DeleteMenu(hMenu, 0, MF_BYPOSITION);
	}

	/* 項目を追加する */
	for (i=15; i>=0; i--) {
		mii.wID = id + i;
		switch (nCPU) {
#if XM7_VER == 1 && defined(Z80CARD)
			case MAINZ80 :
#endif
			case MAINCPU :	readbnio = mainmem_readbnio;
							break;
			case SUBCPU :	readbnio = submem_readbnio;
							break;
#if XM7_VER == 1
#if defined(JSUB)
			case JSUBCPU :	readbnio = jsubmem_readbnio;
							break;
#endif
#endif
		}
#if XM7_VER == 1 && defined(Z80CARD)
		if (nCPU  == MAINZ80) {
			addr = (WORD)(readbnio((WORD)(mainz80.sp + i + 1)) << 8);
			addr |= (WORD)readbnio((WORD)(mainz80.sp + i));
		}
		else {
			addr = (WORD)(readbnio((WORD)(maincpu.s + i)) << 8);
			addr |= (WORD)readbnio((WORD)(maincpu.s + i + 1));
		}
#else
		addr = (WORD)(readbnio((WORD)(maincpu.s + i)) << 8);
		addr |= (WORD)readbnio((WORD)(maincpu.s + i + 1));
#endif

		wStackAddr[i] = addr;
#if XM7_VER == 1 && defined(Z80CARD)
		if (nCPU  == MAINZ80) {
			_snprintf(string, sizeof(string), "SP+%d ($%04X)", i, addr);
		}
		else {
			_snprintf(string, sizeof(string), "%d,S ($%04X)", i, addr);
		}
#else
		_snprintf(string, sizeof(string), "%d,S ($%04X)", i, addr);
#endif
		mii.dwTypeData = string;
		mii.cch = strlen(string);
		InsertMenuItem(hMenu, 0, TRUE, &mii);
	}
}

/*
 *	ブレークポイントジャンプメニュー挿入
 */
static void FASTCALL InsertBreakPointMenu(HMENU hMenu, BYTE nCPU, UINT id)
{
	MENUITEMINFO mii;
	char string[128];
	int no;
	int i;

	/* メニュー構造体初期化 */
	memset(&mii, 0, sizeof(mii));
	mii.cbSize = 44;
	mii.fMask = MIIM_TYPE | MIIM_STATE | MIIM_ID;
	mii.fType = MFT_STRING;
	mii.fState = MFS_ENABLED;

	/* メニューすべて削除 */
	while (GetMenuItemCount(hMenu) > 0) {
		DeleteMenu(hMenu, 0, MF_BYPOSITION);
	}

	/* 項目を追加する */
	no = 0;
	for (i=15; i>=0; i--) {
		if ((breakp[i].flag != BREAKP_NOTUSE) && (breakp[i].cpu == nCPU)) {
			mii.wID = id + no;
			wBreakAddr[no++] = breakp[i].addr;
			_snprintf(string, sizeof(string), "%d : $%04X",
				i+1, breakp[i].addr);
			mii.dwTypeData = string;
			mii.cch = strlen(string);
			InsertMenuItem(hMenu, 0, TRUE, &mii);
		}
	}
	if (no == 0) {
		/* 1つもブレークポイントがない場合 */
		mii.wID = id;
		mii.fState = MFS_GRAYED;
		LoadString(hAppInstance, IDS_SWND_BREAKPOINT_NONE,
				string, sizeof(string));
		mii.dwTypeData = string;
		mii.cch = strlen(string);
		InsertMenuItem(hMenu, 0, TRUE, &mii);
	}
}

/*
 *	スタックジャンプメニュー挿入
 */
static void FASTCALL InsertPhysicalMemMenu(HMENU hMenu, BYTE nCPU, UINT id)
{
	MENUITEMINFO mii;
	char string[128];
	int i;

	UNUSED(nCPU);
	
	/* メニュー構造体初期化 */
	memset(&mii, 0, sizeof(mii));
	mii.cbSize = 44;
	mii.fMask = MIIM_TYPE | MIIM_STATE | MIIM_ID;
	mii.fType = MFT_STRING;
	mii.fState = MFS_ENABLED;

	/* メニューすべて削除 */
	while (GetMenuItemCount(hMenu) > 0) {
		DeleteMenu(hMenu, 0, MF_BYPOSITION);
	}

	/* 項目を追加する */
#if XM7_VER >= 3
	if (mmr_ext) {
		i = 15;
	}
	else {
		i = 3;
	}
#else
	i = 3;
#endif
	for (; i>=0; i--) {
		mii.wID = id + i;
		_snprintf(string, sizeof(string), "$%1X0000-$%1XFFFF", i, i);
		mii.dwTypeData = string;
		mii.cch = strlen(string);
		InsertMenuItem(hMenu, 0, TRUE, &mii);
	}
}

/*-[ 逆アセンブルウインドウ ]------------------------------------------------*/

/*
 *	逆アセンブルウインドウ
 *	アドレス設定
 */
void FASTCALL AddrDisAsm(BYTE nCPU, DWORD dwAddr)
{
	SCROLLINFO sif;
	HWND hWnd;

	/* ウインドウジオメトリを得る */
	hWnd = NULL;
	switch (nCPU) {
		case MAINCPU :	hWnd = hSubWnd[SWND_DISASM_MAIN];
						break;
		case SUBCPU :	hWnd = hSubWnd[SWND_DISASM_SUB];
						break;
#if XM7_VER == 1
#if defined(JSUB)
		case JSUBCPU :	hWnd = hSubWnd[SWND_DISASM_JSUB];
						break;
#endif
#if defined(Z80CARD)
		case MAINZ80 :	hWnd = hSubWnd[SWND_DISASM_Z80];
						break;
#endif
#endif
	}
	if (!hWnd) {
		/* ウインドウがなければそのままリターン */
		return;
	}

	/* SCROLLINFO構造体の初期化 */
	memset(&sif, 0, sizeof(sif));
	sif.cbSize = sizeof(sif);
	sif.fMask = SIF_POS;
	sif.nPos = (int)dwAddr;

	/* 存在チェック＆設定 */
	dwDisAsm[nCPU] = dwAddr;
	SetScrollInfo(hWnd, SB_VERT, &sif, TRUE);

	/* リフレッシュ */
	RefreshDisAsm();
}

/*
 *	逆アセンブルウィンドウ
 *	リサイズ
 */
static void FASTCALL ResizeDisAsm(HWND hWnd, WORD cx, WORD cy, BYTE nCPU)
{
	RECT crect;
	RECT wrect;
	int width;

	ASSERT(hWnd);

	/* 最小化の場合は、何もしない */
	if ((cx == 0) && (cy == 0)) {
		return;
	}

	/* サイズ変更 */
	width = 54;
	nHeightDisAsm[nCPU] = (WORD)(cy / lCharHeight);
	if (nHeightDisAsm[nCPU] < 1) {
		nHeightDisAsm[nCPU] = 1;
	}

	/* ウィンドウ矩形変更 */
	GetWindowRect(hWnd, &wrect);
	GetClientRect(hWnd, &crect);
	wrect.right -= wrect.left;
	wrect.right -= crect.right;
	wrect.right += (width * lCharWidth);
	wrect.bottom -= wrect.top;
	wrect.bottom -= crect.bottom;
	wrect.bottom += (nHeightDisAsm[nCPU] * lCharHeight);
	SetWindowPos(hWnd, HWND_TOP, 0, 0, wrect.right, wrect.bottom,
							SWP_NOZORDER | SWP_NOMOVE);
}

/*
 *	逆アセンブルウインドウ
 *	セットアップ
 */
static void FASTCALL SetupDisAsm(BYTE nCPU, BYTE *p, int x, int y)
{
	char string[128];
	char tmp[16];
	int addr;
	int i;
	int j;
	int k;
	int ret;
	WORD pc;

	ASSERT(nCPU < MAXCPU);
	ASSERT(p);
	ASSERT(x > 0);
	ASSERT(y > 0);

	/* 一旦スペースで埋める */
	memset(p, 0x20, x * y);

	/* 初期設定 */
	switch (nCPU) {
		case MAINCPU :	pc = maincpu.pc;
						break;
		case SUBCPU :	pc = subcpu.pc;
						break;
#if XM7_VER == 1
#if defined(JSUB)
		case JSUBCPU :	pc = jsubcpu.pc;
						break;
#endif
#if defined(Z80CARD)
		case MAINZ80 :	pc = (WORD)mainz80.pc;
						break;
#endif
#endif
	}
	addr = (int)dwDisAsm[nCPU];

	for (i=0; i<y; i++) {
		 /* 逆アセンブル */
#if XM7_VER == 1 && defined(Z80CARD)
		if (nCPU == MAINZ80) {
			ret = disline80(nCPU, (WORD)addr, string);
		}
		else {
			ret = disline(nCPU, (WORD)addr, string);
		}
#else
		ret = disline(nCPU, (WORD)addr, string);
#endif

		/* セット */
#if XM7_VER == 1 && defined(Z80CARD)
		if (((nCPU == MAINCPU) || (nCPU == MAINZ80)) &&
			 isLinearAddrMode(bDisPhysicalAddr)) {
#else
		if ((nCPU == MAINCPU) && isLinearAddrMode(bDisPhysicalAddr)) {
#endif
			memcpy(&p[x * i + 9], string, strlen(string));
			_snprintf(tmp, sizeof(tmp), "%05X", mmr_trans_mmr((WORD)addr));
			memcpy(&p[x * i + 3], tmp, 5);
		}
		else {
			memcpy(&p[x * i + 3], string, strlen(string));
		}

		/* マーク */
		if (pc == addr) {
			p[x * i + 2] = '>';
		}
		for (j=0; j<BREAKP_MAXNUM; j++) {
			if (breakp[j].addr != addr) {
				continue;
			}
			if (breakp[j].cpu != nCPU) {
				continue;
			}
			if (breakp[j].flag == BREAKP_NOTUSE) {
				continue;
			}
			if (breakp[j].flag == BREAKP_DISABLED) {
				continue;
			}

			/* ブレークポイント */
			_snprintf(tmp, sizeof(tmp), "%2d", j + 1);
			memcpy(&p[x * i + 0], tmp, 2);

			/* 反転 */
			for (k=0; k<x; k++) {
				 p[x * i + k] = (BYTE)(p[x * i + k] | 0x80);
			}
		}

		/* 加算、オーバーチェック */
		addr += ret;
		if (addr >= 0x10000) {
			break;
		}
	}
}

/*
 *	逆アセンブルウインドウ
 *	描画
 */
static void FASTCALL DrawDisAsm(HWND hWnd, HDC hDC)
{
	int i;
	BYTE nCPU;
	RECT rect;
	BYTE *p;
	int x, y;

	ASSERT(hWnd);
	ASSERT(hDC);

	/* Drawバッファを得る(存在しなければ何もしない) */
	for (i=0; i<SWND_MAXNUM; i++) {
		if (hSubWnd[i] == hWnd) {
			nCPU = (BYTE)((i - SWND_DISASM_MAIN) % MAXCPU);
			p = pDisAsm[nCPU];
			break;
		}
	}
	if (!p) {
		return;
	}

	/* ウインドウジオメトリを得る */
	GetClientRect(hWnd, &rect);
	x = rect.right / lCharWidth;
	y = rect.bottom / lCharHeight;
	if ((x == 0) || (y == 0)) {
		return;
	}

	/* セットアップ */
	SetupDisAsm(nCPU, p, x, y);

	/* 描画 */
	DrawWindowText2(hDC, p, x, y);
}

/*
 *	逆アセンブルウインドウ
 *	リフレッシュ
 */
void FASTCALL RefreshDisAsm(void)
{
	HWND hWnd;
	HDC hDC;

	/* メインCPU */
	if (hSubWnd[SWND_DISASM_MAIN]) {
		hWnd = hSubWnd[SWND_DISASM_MAIN];
		hDC = GetDC(hWnd);
		DrawDisAsm(hWnd, hDC);
		ReleaseDC(hWnd, hDC);
	}

	/* サブCPU */
	if (hSubWnd[SWND_DISASM_SUB]) {
		hWnd = hSubWnd[SWND_DISASM_SUB];
		hDC = GetDC(hWnd);
		DrawDisAsm(hWnd, hDC);
		ReleaseDC(hWnd, hDC);
	}

#if XM7_VER == 1
#if defined(JSUB)
	/* 日本語サブCPU */
	if (hSubWnd[SWND_DISASM_JSUB]) {
		hWnd = hSubWnd[SWND_DISASM_JSUB];
		hDC = GetDC(hWnd);
		DrawDisAsm(hWnd, hDC);
		ReleaseDC(hWnd, hDC);
	}
#endif

#if defined(JSUB)
	/* メインCPU(Z80) */
	if (hSubWnd[SWND_DISASM_Z80]) {
		hWnd = hSubWnd[SWND_DISASM_Z80];
		hDC = GetDC(hWnd);
		DrawDisAsm(hWnd, hDC);
		ReleaseDC(hWnd, hDC);
	}
#endif
#endif
}

/*
 *	逆アセンブルウインドウ
 *	再描画
 */
static void FASTCALL PaintDisAsm(HWND hWnd)
{
	HDC hDC;
	PAINTSTRUCT ps;
	int i;
	RECT rect;
	BYTE *p;
	int x, y;

	ASSERT(hWnd);

	/* Drawバッファを得る(存在しなければ何もしない) */
	for (i=0; i<SWND_MAXNUM; i++) {
		if (hSubWnd[i] == hWnd) {
			p = pDisAsm[((i - SWND_DISASM_MAIN) % MAXCPU)];
			break;
		}
	}

	if (!p) {
		return;
	}

	/* ウインドウジオメトリを得る */
	GetClientRect(hWnd, &rect);
	x = rect.right / lCharWidth;
	y = rect.bottom / lCharHeight;

	/* 後半エリアをFFで埋める */
	if ((x > 0) && (y > 0)) {
		memset(&p[x * y], 0xff, x * y);
	}

	/* 描画 */
	hDC = BeginPaint(hWnd, &ps);
	ASSERT(hDC);
	DrawDisAsm(hWnd, hDC);
	EndPaint(hWnd, &ps);
}

/*
 *	逆アセンブルウインドウ
 *	左クリック
 */
static void FASTCALL LButtonDisAsm(HWND hWnd, POINT point, BOOL mode)
{
	char string[128];
	int addr;
	BOOL flag;
	BYTE nCPU;
	int i;
	int y;
	int ret;

	ASSERT(hWnd);

	/* 行カウントを得る */
	y = point.y / lCharHeight;

	/* 実際に逆アセンブルしてみる */
	for (i=0; i<SWND_MAXNUM; i++) {
		if (hSubWnd[i] == hWnd) {
			nCPU = (BYTE)((i - SWND_CPUREG_MAIN) % MAXCPU);
			addr = (int)dwDisAsm[nCPU];
			break;
		}
	}

	/* 逆アセンブル ループ */
	for (i=0; i<y; i++) {
#if XM7_VER == 1 && defined(Z80CARD)
		if (nCPU == MAINZ80) {
			ret = disline80(nCPU, (WORD)addr, string);
		}
		else {
			ret = disline(nCPU, (WORD)addr, string);
		}
#else
		ret = disline(nCPU, (WORD)addr, string);
#endif
		addr += ret;
		if (addr >= 0x10000) {
			return;
		}
	}

	if (mode) {
		/* 強制ジャンプ */
		switch (nCPU) {
			case MAINCPU :	maincpu.pc = (WORD)addr;
							break;
			case SUBCPU :	subcpu.pc = (WORD)addr;
							break;
#if XM7_VER == 1
#if defined(JSUB)
			case JSUBCPU :	jsubcpu.pc = (WORD)addr;
							break;
#endif
#if defined(Z80CARD)
			case MAINZ80 :	mainz80.pc = (Uint32)addr;
							break;
#endif
#endif
		}
	}
	else {
		/* ブレークポイント on/off */
		flag = FALSE;
		for (i=0; i<BREAKP_MAXNUM; i++) {
			if ((breakp[i].cpu == nCPU) && (breakp[i].addr == addr)) {
				if (breakp[i].flag != BREAKP_NOTUSE) {
					breakp[i].flag = BREAKP_NOTUSE;
					flag = TRUE;
					break;
				}
			}
		}
		if (!flag) {
			schedule_setbreak(nCPU, (WORD)addr);
		}

		/* ブレークポイントウインドウも、再描画する */
		if (hSubWnd[SWND_BREAKPOINT]) {
			InvalidateRect(hSubWnd[SWND_BREAKPOINT], NULL, FALSE);
		}
	}

	/* 再描画 */
	InvalidateRect(hWnd, NULL, FALSE);
}

/*
 *	逆アセンブルウインドウ
 *	コマンド処理
 */
static void FASTCALL CmdDisAsm(HWND hWnd, WORD wID, BYTE nCPU)
{
	DWORD target;
	cpu6809_t *cpu;

	/* CPU構造体決定 */
	switch (nCPU) {
#if XM7_VER == 1 && defined(Z80CARD)
		case MAINZ80 :
#endif
		case MAINCPU :	cpu = &maincpu;
						break;
		case SUBCPU :	cpu = &subcpu;
						break;
#if XM7_VER == 1
#if defined(JSUB)
		case JSUBCPU :	cpu = &jsubcpu;
						break;
#endif
#endif
	}

	/* ターゲットアドレス決定 */
	switch (wID) {
		case IDM_DIS_ADDR:
#if XM7_VER == 1 && defined(Z80CARD)
			if (((nCPU == MAINCPU) || (nCPU == MAINZ80)) &&
				 isLinearAddrMode(bDisPhysicalAddr)) {
#else
			if ((nCPU == MAINCPU) && isLinearAddrMode(bDisPhysicalAddr)) {
#endif
				target = mmr_trans_mmr((WORD)dwDisAsm[nCPU]);
				if (!AddrDlg(hWnd, &target, nCPU, TRUE)) {
					return;
				}
				target = mmr_trans_phys_to_logi(target);
			}
			else {
				target = dwDisAsm[nCPU];
				if (!AddrDlg(hWnd, &target, nCPU, FALSE)) {
					return;
				}
				target &= (WORD)0xffff;
			}
			break;
		case IDM_DIS_PHYSADDR:
#if XM7_VER == 1 && defined(Z80CARD)
			if ((nCPU == MAINCPU) || (nCPU == MAINZ80)) {
#else
			if (nCPU == MAINCPU) {
#endif
				bDisPhysicalAddr = !bDisPhysicalAddr;
			}
			return;

		case IDM_DIS_PC:
			target = cpu->pc;
			break;
		case IDM_DIS_X:
			target = cpu->x;
			break;
		case IDM_DIS_Y:
			target = cpu->y;
			break;
		case IDM_DIS_U:
			target = cpu->u;
			break;
		case IDM_DIS_S:
			target = cpu->s;
			break;
#if XM7_VER == 1 && defined(Z80CARD)
		case IDM_DIS_PC_Z80:
			target = mainz80.pc;
			break;
		case IDM_DIS_BC:
			target = (WORD)((mainz80.regs8[REGID_B]) << 8 |
				mainz80.regs8[REGID_C]);
			break;
		case IDM_DIS_DE:
			target = (WORD)((mainz80.regs8[REGID_D]) << 8 |
				mainz80.regs8[REGID_E]);
			break;
		case IDM_DIS_HL:
			target = (WORD)((mainz80.regs8[REGID_H]) << 8 |
				mainz80.regs8[REGID_L]);
			break;
		case IDM_DIS_IX:
			target = (WORD)((mainz80.regs8[REGID_IXH]) << 8 |
				mainz80.regs8[REGID_IXL]);
			break;
		case IDM_DIS_IY:
			target = (WORD)((mainz80.regs8[REGID_IYH]) << 8 |
				mainz80.regs8[REGID_IYL]);
			break;
		case IDM_DIS_SP:
			target = mainz80.sp;
			break;
#endif

		case IDM_DIS_RESET:
			target = (WORD)((cpu->readmem(0xfffe) << 8) | cpu->readmem(0xffff));
			break;
		case IDM_DIS_NMI:
			target = (WORD)((cpu->readmem(0xfffc) << 8) | cpu->readmem(0xfffd));
			break;
		case IDM_DIS_SWI:
			target = (WORD)((cpu->readmem(0xfffa) << 8) | cpu->readmem(0xfffb));
			break;
		case IDM_DIS_IRQ:
			target = (WORD)((cpu->readmem(0xfff8) << 8) | cpu->readmem(0xfff9));
			break;
		case IDM_DIS_FIRQ:
			target = (WORD)((cpu->readmem(0xfff6) << 8) | cpu->readmem(0xfff7));
			break;
		case IDM_DIS_SWI2:
			target = (WORD)((cpu->readmem(0xfff4) << 8) | cpu->readmem(0xfff5));
			break;
		case IDM_DIS_SWI3:
			target = (WORD)((cpu->readmem(0xfff2) << 8) | cpu->readmem(0xfff3));
			break;
		case IDM_DIS_JUMP:
			LButtonDisAsm(hWnd, PosDisAsmPoint, TRUE);
#if XM7_VER == 1 && defined(Z80CARD)
			if (nCPU == MAINZ80) {
				target = (WORD)mainz80.pc;
			}
			else {
				target = cpu->pc;
			}
#else
			target = cpu->pc;
#endif
			break;
		case IDM_DIS_SYNCPC:
			bSyncDisasm[nCPU] = (!bSyncDisasm[nCPU]);
			return;
		default:
			if ((wID >= IDM_DIS_STACK) && (wID <= IDM_DIS_STACK + 15)) {
				target = (WORD)wStackAddr[wID - IDM_DIS_STACK];
			}
			else if ((wID >= IDM_DIS_BREAK) && (wID <= IDM_DIS_BREAK + 15)) {
				target = (WORD)wBreakAddr[wID - IDM_DIS_BREAK];
			}
			break;
	}

	/* 設定＆更新 */
	AddrDisAsm(nCPU, (WORD)target);
}

/*
 *	逆アセンブルウインドウ
 *	ウインドウプロシージャ
 */
static LRESULT CALLBACK DisAsmProc(HWND hWnd, UINT message,
								 WPARAM wParam, LPARAM lParam, BYTE nCPU)
{
	static BOOL bResize = FALSE;	/* 手動リサイズフラグ */
	DWORD dwAddr;
	char string[128];
	int ret;
	int i;
	POINT point;
	HMENU hMenu;
	RECT rect;

	/* メッセージ別 */
	switch (message) {
		/* ウインドウ再描画 */
		case WM_PAINT:
			/* ロックが必要 */
			LockVM();
			PaintDisAsm(hWnd);
			UnlockVM();
			return 0;

		/* ウインドウ削除 */
		case WM_DESTROY:
			LockVM();

			/* メインウインドウへ自動通知 */
			DestroySubWindow(hWnd, &pDisAsm[nCPU], hDisAsm[nCPU]);

			UnlockVM();
			break;

		/* ウインドウサイズ変更 */
		case WM_SIZE:
			if (bResize) {
				bResize = FALSE;
				ResizeDisAsm(hWnd, LOWORD(lParam), HIWORD(lParam), nCPU);
			}
			break;

		/* ウインドウサイズ変更中 */
		case WM_SIZING:
			WindowSizing(hWnd, (LPRECT)lParam, &pDisAsm[nCPU]);
			bResize = TRUE;
			break;

		/* ウィンドウサイズ変更メッセージ */
		case WM_WINDOWPOSCHANGING:
			bResize = TRUE;
			break;

		/* 最小サイズ制限 */
		case WM_GETMINMAXINFO:
			rect.left = 0;
			rect.right = 54 * lCharWidth + GetSystemMetrics(SM_CXVSCROLL);
			rect.top = 0;
			rect.bottom = lCharHeight;
			if (bPopupSwnd) {
				AdjustWindowRect(&rect, WS_POPUP | WS_OVERLAPPED | WS_SYSMENU |
										WS_CAPTION | WS_VISIBLE | 
										WS_MINIMIZEBOX | WS_SIZEBOX, FALSE);
			}
			else {
				AdjustWindowRect(&rect, WS_CHILD | WS_OVERLAPPED | WS_SYSMENU |
										WS_CAPTION | WS_VISIBLE | 
										WS_MINIMIZEBOX | WS_CLIPSIBLINGS | 
										WS_SIZEBOX, FALSE);
			}
			((LPMINMAXINFO)lParam)->ptMinTrackSize.x = rect.right - rect.left;
			((LPMINMAXINFO)lParam)->ptMaxTrackSize.x = rect.right - rect.left;
			((LPMINMAXINFO)lParam)->ptMinTrackSize.y = rect.bottom - rect.top;
			return 0;

		/* コンテキストメニュー */
		case WM_RBUTTONDOWN:
#if defined(MOUSE)
			/* マウスエミュレーション使用時は無効 */
			if (mos_capture && !stopreq_flag && run_flag) {
				return FALSE;
			}
#endif

			/* コンテキストメニューをロード */
			hMenu = GetSubMenu(hDisAsm[nCPU], 0);
			CheckMenuSub(hMenu, IDM_DIS_SYNCPC, bSyncDisasm[nCPU]);
			EnableMenuSub(hMenu, IDM_DIS_SYNCPC, bSync);

			/* 物理アドレスモード設定 */
#if XM7_VER >= 2
			if ((nCPU == MAINCPU) && (fm7_ver >= 2)) {
#else
#if XM7_VER == 1 && defined(Z80CARD)
			if (((nCPU == MAINCPU) || (nCPU == MAINZ80)) &&
				 (fm_subtype == FMSUB_FM77)) {
#else
			if ((nCPU == MAINCPU) && (fm_subtype == FMSUB_FM77)) {
#endif
#endif
				CheckMenuSub(hMenu, IDM_DIS_PHYSADDR, bDisPhysicalAddr);
				EnableMenuSub(hMenu, IDM_DIS_PHYSADDR, TRUE);
			}
			else {
				CheckMenuSub(hMenu, IDM_DIS_PHYSADDR, FALSE);
				EnableMenuSub(hMenu, IDM_DIS_PHYSADDR, FALSE);
			}

#if XM7_VER == 1 && defined(Z80CARD)
			/* 割り込みベクタメニューOFF */
			if (nCPU == MAINZ80) {
				EnableMenuItem(hMenu, 6, MF_BYPOSITION | MF_GRAYED);
			}
#endif

			/* システムスタックジャンプメニュー設定 */
			InsertStackJumpMenu(GetSubMenu(hMenu, 7), nCPU, IDM_DIS_STACK);

			/* ブレークポイントメニュー設定 */
			InsertBreakPointMenu(GetSubMenu(hMenu, 8), nCPU, IDM_DIS_BREAK);

			/* コンテキストメニューを実行 */
			point.x = LOWORD(lParam);
			point.y = HIWORD(lParam);
			PosDisAsmPoint = point;
			ClientToScreen(hWnd, &point);
			TrackPopupMenu(hMenu, 0, point.x, point.y, 0, hWnd, NULL);
			return 0;

		/* 左クリック・ダブルクリック */
		case WM_LBUTTONDOWN:
		case WM_LBUTTONDBLCLK:
#if defined(MOUSE)
			/* マウスエミュレーション使用時は無効 */
			if (mos_capture && !stopreq_flag && run_flag) {
				return FALSE;
			}
#endif

			point.x = LOWORD(lParam);
			point.y = HIWORD(lParam);
			LButtonDisAsm(hWnd, point, FALSE);
			break;

		/* コマンド */
		case WM_COMMAND:
			CmdDisAsm(hWnd, LOWORD(wParam), nCPU);
			break;

		/* 垂直スクロールバー */
		case WM_VSCROLL:
			/* タイプ判別 */
			dwAddr = dwDisAsm[nCPU];

			/* アクション別 */
			switch (LOWORD(wParam)) {
				/* トップ */
				case SB_TOP:
					dwAddr = 0;
					break;
				/* 終端 */
				case SB_BOTTOM:
					dwAddr = 0xffff;
					break;
				/* １行上 */
				case SB_LINEUP:
					if (dwAddr > 0) {
						dwAddr--;
					}
					break;
				/* １行下(ここは工夫) */
				case SB_LINEDOWN:
#if XM7_VER == 1 && defined(Z80CARD)
					if (nCPU == MAINZ80) {
						ret = disline80(nCPU, (WORD)dwAddr, string);
					}
					else {
						ret = disline(nCPU, (WORD)dwAddr, string);
					}
#else
					ret = disline(nCPU, (WORD)dwAddr, string);
#endif
					i = (int)dwAddr;
					i += ret;
					if (i < 0x10000) {
						dwAddr += (WORD)ret;
					}
					break;
				/* ページアップ */
				case SB_PAGEUP:
					if (dwAddr < 0x80) {
						dwAddr = 0;
					}
					else {
						dwAddr -= (WORD)0x80;
					}
					break;
				/* ページダウン */
				case SB_PAGEDOWN:
					if (dwAddr >= 0xff80) {
						dwAddr = 0xffff;
					}
					else {
						dwAddr += (WORD)0x80;
					}
					break;
				/* 直接指定 */
				case SB_THUMBTRACK:
					dwAddr = HIWORD(wParam);
					if (dwAddr >= 0xffff) {
						dwAddr = 0xffff;
					}
					break;
			}
			AddrDisAsm(nCPU, dwAddr);
			RefreshDisAsm();
			break;
	}

	/* デフォルト ウインドウプロシージャ */
	return DefWindowProc(hWnd, message, wParam, lParam);
}

/*
 *	逆アセンブルウインドウ(メイン)
 *	ウインドウプロシージャ
 */
static LRESULT CALLBACK DisAsmProcMain(HWND hWnd, UINT message,
								 WPARAM wParam, LPARAM lParam)
{
	return DisAsmProc(hWnd, message, wParam, lParam, MAINCPU);
}

/*
 *	逆アセンブルウインドウ(サブ)
 *	ウインドウプロシージャ
 */
static LRESULT CALLBACK DisAsmProcSub(HWND hWnd, UINT message,
								 WPARAM wParam, LPARAM lParam)
{
	return DisAsmProc(hWnd, message, wParam, lParam, SUBCPU);
}

#if XM7_VER == 1
/*
 *	逆アセンブルウインドウ(日本語サブ)
 *	ウインドウプロシージャ
 */
#if defined(JSUB)
static LRESULT CALLBACK DisAsmProcJsub(HWND hWnd, UINT message,
								 WPARAM wParam, LPARAM lParam)
{
	return DisAsmProc(hWnd, message, wParam, lParam, JSUBCPU);
}
#endif

/*
 *	逆アセンブルウインドウ(メインZ80)
 *	ウインドウプロシージャ
 */
#if defined(Z80CARD)
static LRESULT CALLBACK DisAsmProcZ80(HWND hWnd, UINT message,
								 WPARAM wParam, LPARAM lParam)
{
	return DisAsmProc(hWnd, message, wParam, lParam, MAINZ80);
}
#endif
#endif

/*
 *	逆アセンブルウインドウ
 *	ウインドウ作成
 */
HWND FASTCALL CreateDisAsm(HWND hParent, BYTE nCPU, int index)
{
	WNDCLASSEX wcex;
	char szClassNameMain[] = "XM7_DisAsmMain";
	char szClassNameSub[] = "XM7_DisAsmSub";
#if XM7_VER == 1
#if defined(JSUB)
	char szClassNameJsub[] = "XM7_DisAsmJsub";
#endif
#if defined(Z80CARD)
	char szClassNameZ80[] = "XM7_DisAsmZ80";
#endif
#endif
	char *szClassName;
	char szWndName[128];
	RECT rect;
	RECT crect, wrect;
	HWND hWnd;
	SCROLLINFO si;
	UINT id;
	WORD pc;
	DWORD dwStyle;

	ASSERT(hParent);

	/* ウインドウ矩形を計算 */
	PositioningSubWindow(hParent, &rect, index);
	rect.right = lCharWidth * 54;

	/* ウインドウタイトルを決定、バッファ確保、メニュロード */
	switch (nCPU) {
		case MAINCPU :	id = IDS_SWND_DISASM_MAIN;
						pc = maincpu.pc;
						szClassName = szClassNameMain;
						break;
		case SUBCPU :	id = IDS_SWND_DISASM_SUB;
						pc = subcpu.pc;
						szClassName = szClassNameSub;
						break;
#if XM7_VER == 1
#if defined(JSUB)
		case JSUBCPU :	id = IDS_SWND_DISASM_JSUB;
						pc = jsubcpu.pc;
						szClassName = szClassNameJsub;
						break;
#endif
#if defined(Z80CARD)
		case MAINZ80 :	id = IDS_SWND_DISASM_Z80;
						pc = (WORD)mainz80.pc;
						szClassName = szClassNameZ80;
						break;
#endif
#endif
	}
	rect.bottom = lCharHeight * nHeightDisAsm[nCPU];
	LoadString(hAppInstance, id, szWndName, sizeof(szWndName));
	pDisAsm[nCPU] = malloc(2 * (rect.right / lCharWidth) * nHeightDisAsm[nCPU]);
	dwDisAsm[nCPU] = pc;
#if XM7_VER == 1 && defined(Z80CARD)
	if (nCPU == MAINZ80) {
		hDisAsm[nCPU] = LoadMenu(hAppInstance, MAKEINTRESOURCE(IDR_DISASMZ80MENU));
	}
	else {
		hDisAsm[nCPU] = LoadMenu(hAppInstance, MAKEINTRESOURCE(IDR_DISASMMENU));
	}
#else
	hDisAsm[nCPU] = LoadMenu(hAppInstance, MAKEINTRESOURCE(IDR_DISASMMENU));
#endif

	/* ウインドウクラスの登録 */
	memset(&wcex, 0, sizeof(wcex));
	wcex.cbSize = sizeof(wcex);
	wcex.style = CS_VREDRAW | CS_HREDRAW;
	switch (nCPU) {
		case MAINCPU :	wcex.lpfnWndProc = DisAsmProcMain;
						break;
		case SUBCPU :	wcex.lpfnWndProc = DisAsmProcSub;
						break;
#if XM7_VER == 1
#if defined(JSUB)
		case JSUBCPU :	wcex.lpfnWndProc = DisAsmProcJsub;
						break;
#endif
#if defined(Z80CARD)
		case MAINZ80 :	wcex.lpfnWndProc = DisAsmProcZ80;
						break;
#endif
#endif
	}
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hAppInstance;
	wcex.hIcon = LoadIcon(hAppInstance, MAKEINTRESOURCE(IDI_WNDICON));
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	wcex.lpszMenuName = NULL;
	wcex.lpszClassName = szClassName;
	wcex.hIconSm = LoadIcon(hAppInstance, MAKEINTRESOURCE(IDI_WNDICON));
	RegisterClassEx(&wcex);

	/* ウインドウ作成 */
	if (bPopupSwnd) {
		dwStyle =	WS_POPUP | WS_OVERLAPPED | WS_SYSMENU | WS_CAPTION |
					WS_VISIBLE | WS_MINIMIZEBOX | WS_VSCROLL | WS_SIZEBOX;
	}
	else {
		dwStyle =	WS_CHILD | WS_OVERLAPPED | WS_SYSMENU | WS_CAPTION |
					WS_VISIBLE | WS_MINIMIZEBOX | WS_CLIPSIBLINGS |
					WS_VSCROLL | WS_SIZEBOX;
	}
	hWnd = CreateWindow(szClassName,
						szWndName,
						dwStyle,
						rect.left,
						rect.top,
						rect.right,
						rect.bottom,
						hParent,
						NULL,
						hAppInstance,
						NULL);

	/* 有効なら、サイズ補正して手前に置く */
	if (hWnd) {
		GetWindowRect(hWnd, &wrect);
		GetClientRect(hWnd, &crect);
		wrect.right += (rect.right - crect.right);
		wrect.bottom += (rect.bottom - crect.bottom);
		SetWindowPos(hWnd, HWND_TOP, wrect.left, wrect.top,
			wrect.right - wrect.left, wrect.bottom - wrect.top, SWP_NOMOVE);

		/* スクロールバーの設定が必要 */
		memset(&si, 0, sizeof(si));
		si.cbSize = sizeof(si);
		si.fMask = SIF_ALL;
		si.nMin = 0;
		si.nMax = 0x100fe;
		si.nPage = 0x100;
		si.nPos = pc;
		SetScrollInfo(hWnd, SB_VERT, &si, TRUE);
	}

	/* ポップアップウインドウ時はアクティブウインドウを前面に変更 */
	if (bPopupSwnd) {
		SetForegroundWindow(hMainWnd);
	}

	/* 結果を持ち帰る */
	return hWnd;
}

/*-[ メモリ内容変更ダイアログ ]----------------------------------------------*/

/*
 *	メモリ内容変更ダイアログ
 *	ダイアログ初期化
 */
static BOOL FASTCALL MemoryChangeDlgInit(HWND hDlg)
{
	HWND hWnd;
	RECT prect;
	RECT drect;
	char tmp[16];
	char string[128];
	UINT id;

	ASSERT(hDlg);

	/* 親ウインドウの中央に設定 */
	hWnd = GetParent(hDlg);
	GetWindowRect(hWnd, &prect);
	GetWindowRect(hDlg, &drect);
	drect.right -= drect.left;
	drect.bottom -= drect.top;
	drect.left = (prect.right - prect.left) / 2 + prect.left;
	drect.left -= (drect.right / 2);
	drect.top = (prect.bottom - prect.top) / 2 + prect.top;
	drect.top -= (drect.bottom / 2);
	MoveWindow(hDlg, drect.left, drect.top, drect.right, drect.bottom, FALSE);

	/* アドレスを設定 */
	hWnd = GetDlgItem(hDlg, IDC_MEMLABEL);
	ASSERT(hWnd);
	switch (nMemDlgCPU) {
		case MAINCPU :	if (isLinearAddrMode(bDumpPhysicalAddr)) {
							id = IDS_SWND_CPU_MAIN_P;
						}
						else {
							id = IDS_SWND_CPU_MAIN;
						}
						break;
		case SUBCPU :	id = IDS_SWND_CPU_SUB;
						break;
#if XM7_VER == 1
#if defined(JSUB)
		case JSUBCPU :	id = IDS_SWND_CPU_JSUB;
						break;
#endif
#endif
	}
	LoadString(hAppInstance, id, tmp, sizeof(tmp));
	if ((nMemDlgCPU == MAINCPU) && isLinearAddrMode(bDumpPhysicalAddr)) {
		_snprintf(string, sizeof(string), "$%05X (%s)",
			dwMemDlgAddr & (GetMaxMemArea(bDumpPhysicalAddr) - 1), tmp);
	}
	else {
		_snprintf(string, sizeof(string), "$%04X (%s)",
			dwMemDlgAddr & 0xffff, tmp);
	}
	SetWindowText(hWnd, string);

	/* 元データを設定 */
	hWnd = GetDlgItem(hDlg, IDC_MEMEDIT);
	ASSERT(hWnd);
	_snprintf(string, sizeof(string), "%02X", nMemDlgByte);
	SetWindowText(hWnd, string);

	return TRUE;
}

/*
 *	メモリ内容変更ダイアログ
 *	ダイアログプロシージャ
 */
static BOOL CALLBACK MemoryChangeDlgProc(HWND hDlg, UINT iMsg,
									WPARAM wParam, LPARAM lParam)
{
	HWND hWnd;
	char string[128];

	UNUSED(lParam);

	switch (iMsg) {
		/* ダイアログ初期化 */
		case WM_INITDIALOG:
			return MemoryChangeDlgInit(hDlg);

		/* コマンド処理 */
		case WM_COMMAND:
			switch (LOWORD(wParam)) {
				/* OK・キャンセル */
				case IDOK:
				case IDCANCEL:
					if (LOWORD(wParam) == IDOK) {
						/* 現在の値を取得 */
						hWnd = GetDlgItem(hDlg, IDC_MEMEDIT);
						ASSERT(hWnd);
						GetWindowText(hWnd, string, sizeof(string) - 1);
						if (string[0] == 0x27) {
							nMemDlgByte = (BYTE)string[1];
						}
						else {
							nMemDlgByte = (BYTE)strtol(string, NULL, 16);
						}
					}
					EndDialog(hDlg, LOWORD(wParam));
					InvalidateRect(hDrawWnd, NULL, FALSE);
					return TRUE;
			}
			break;
	}

	/* それ以外は、FALSE */
	return FALSE;
}

/*
 *	メモリ内容変更
 */
static BOOL FASTCALL MemoryChange(HWND hWnd, BYTE nCPU, int x, int y)
{
	BYTE (FASTCALL *readbnio)(WORD addr);
	void (FASTCALL *writeb)(WORD addr, BYTE dat);
	int height;
	int ret;

	ASSERT(hWnd);

	/* パラメータを保存・設定 */
	switch (nCPU) {
		case MAINCPU :	readbnio = mainmem_readbnio;
						writeb = mainmem_writeb;
						break;
		case SUBCPU :	readbnio = submem_readbnio;
						writeb = submem_writeb;
						break;
#if XM7_VER == 1
#if defined(JSUB)
		case JSUBCPU :	readbnio = jsubmem_readbnio;
						writeb = jsubmem_writeb;
						break;
#endif
#endif
	}
	ASSERT(readbnio);
	ASSERT(writeb);

	nMemDlgCPU = nCPU;
	height = nHeightMemory[nCPU];
	if (bAsciiDump[nCPU]) {
		height -= 3;
	}

	/* アドレスをセット */
	if ((x < 7) || (x > 54) || (((x - 7) % 3) == 2) ||
		(y < 0) || (y >= height)) {
		/* 座標が範囲外 */
		return FALSE;
	}

	/* アドレスと元データを取得 */
	if ((nMemDlgCPU == MAINCPU) && isLinearAddrMode(bDumpPhysicalAddr)) {
		dwMemDlgAddr  = (DWORD)(dwMemory[nCPU] & 0xffff0);
		dwMemDlgAddr += (DWORD)((x - 7) / 3);
		dwMemDlgAddr += (DWORD)(y * 16);

		/* 元データをセット */
		nMemDlgByte = mainmem_readbnio_p(dwMemDlgAddr);
	}
	else {
		dwMemDlgAddr  = (WORD)(dwMemory[nCPU] & 0xfff0);
		dwMemDlgAddr += (WORD)((x - 7) / 3);
		dwMemDlgAddr += (WORD)(y * 16);

		/* 元データをセット */
		nMemDlgByte = readbnio((WORD)dwMemDlgAddr);
	}


	/* モーダルダイアログ実行 */
	ret = DialogBox(hAppInstance, MAKEINTRESOURCE(IDD_MEMDLG), hWnd, MemoryChangeDlgProc);
	if (ret != IDOK) {
		return FALSE;
	}

	/* データをセット */
	LockVM();
	if ((nMemDlgCPU == MAINCPU) && isLinearAddrMode(bDumpPhysicalAddr)) {
		mainmem_writeb_p(dwMemDlgAddr, (BYTE)nMemDlgByte);
	}
	else {
		writeb((WORD)dwMemDlgAddr, (BYTE)nMemDlgByte);
	}
	InvalidateRect(hWnd, NULL, FALSE);
	UnlockVM();

	return TRUE;
}

/*-[ メモリ検索ダイアログ ]--------------------------------------------------*/

/*
 *	メモリ検索ダイアログ
 *	ダイアログ初期化
 */
static BOOL FASTCALL MemorySearchDlgInit(HWND hDlg)
{
	HWND hWnd;
	RECT prect;
	RECT drect;
	char string[128];
	DWORD addr;

	ASSERT(hDlg);

	/* 親ウインドウの中央に設定 */
	hWnd = GetParent(hDlg);
	GetWindowRect(hWnd, &prect);
	GetWindowRect(hDlg, &drect);
	drect.right -= drect.left;
	drect.bottom -= drect.top;
	drect.left = (prect.right - prect.left) / 2 + prect.left;
	drect.left -= (drect.right / 2);
	drect.top = (prect.bottom - prect.top) / 2 + prect.top;
	drect.top -= (drect.bottom / 2);
	MoveWindow(hDlg, drect.left, drect.top, drect.right, drect.bottom, FALSE);

	/* コンボボックス処理 */
	hWnd = GetDlgItem(hDlg, IDC_SEARCHADDR);
	ASSERT(hWnd);
	InsertHistory(hWnd, bDumpPhysicalAddr);

	/* アドレスを設定 */
	addr = dwMemSrchAddrSave[nMemSrchCPU];
	if ((nMemSrchCPU == MAINCPU) && isLinearAddrMode(bDumpPhysicalAddr)) {
		_snprintf(string, sizeof(string), "%05X",
			addr & (GetMaxMemArea(bDumpPhysicalAddr) - 1));
	}
	else {
		_snprintf(string, sizeof(string), "%04X", addr & 0xffff);
	}
	ComboBox_SetText(hWnd, string);

	/* 検索文字列を設定 */
	hWnd = GetDlgItem(hDlg, IDC_SEARCHDATA);
	ASSERT(hWnd);
	SetWindowText(hWnd, (LPSTR)szMemSrchString);

	return TRUE;
}

/*
 *	メモリ検索ダイアログ
 *	ダイアログOK
 */
static void FASTCALL MemorySearchDlgOK(HWND hDlg)
{
	HWND hWnd;
	char string[128];

	ASSERT(hDlg);

	/* 現在の値を取得 */
	hWnd = GetDlgItem(hDlg, IDC_SEARCHDATA);
	ASSERT(hWnd);
	GetWindowText(hWnd, szMemSrchString, sizeof(szMemSrchString) - 1);

	/* コンボボックス処理 */
	hWnd = GetDlgItem(hDlg, IDC_SEARCHADDR);
	ASSERT(hWnd);

	/* 現在の値を取得 */
	ComboBox_GetText(hWnd, string, sizeof(string) - 1);
	dwMemSrchAddr = (DWORD)strtol(string, NULL, 16);
	if ((nMemSrchCPU == MAINCPU) && isLinearAddrMode(bDumpPhysicalAddr)) {
		dwMemSrchAddr &= (GetMaxMemArea(bDumpPhysicalAddr) - 1);
	}
	else {
		dwMemSrchAddr &= 0xffff;
	}

	/* ヒストリ追加 */
	AddAddrHistory(dwMemSrchAddr, bDumpPhysicalAddr);
}

/*
 *	メモリ検索ダイアログ
 *	ダイアログプロシージャ
 */
static BOOL CALLBACK MemorySearchDlgProc(HWND hDlg, UINT iMsg,
									WPARAM wParam, LPARAM lParam)
{
	UNUSED(lParam);

	switch (iMsg) {
		/* ダイアログ初期化 */
		case WM_INITDIALOG:
			return MemorySearchDlgInit(hDlg);

		/* コマンド処理 */
		case WM_COMMAND:
			switch (LOWORD(wParam)) {
				/* OK・キャンセル */
				case IDOK:
				case IDCANCEL:
					if (LOWORD(wParam) == IDOK) {
						MemorySearchDlgOK(hDlg);
					}
					EndDialog(hDlg, LOWORD(wParam));
					InvalidateRect(hDrawWnd, NULL, FALSE);
					return TRUE;
			}
			break;
	}

	/* それ以外は、FALSE */
	return FALSE;
}

/*
 *	メモリ検索
 */
static BOOL FASTCALL MemorySearch(HWND hWnd, BYTE nCPU)
{
	BYTE (FASTCALL *readbnio)(WORD addr);
	BYTE searchdata[128];
	char string[128];
	char tmp[128];
	char *p;
	char *q;
	BOOL flag;
	BYTE dat;
	DWORD datasize;
	DWORD i;
	DWORD j;
	int ret;

	ASSERT(hWnd);

	/* モーダルダイアログ実行 */
	nMemSrchCPU = nCPU;
	ret = DialogBox(hAppInstance, MAKEINTRESOURCE(IDD_SEARCHDLG), hWnd, MemorySearchDlgProc);
	if (ret != IDOK) {
		return FALSE;
	}

	/* ワーク設定 */
	switch (nCPU) {
		case MAINCPU :	readbnio = mainmem_readbnio;
						break;
		case SUBCPU :	readbnio = submem_readbnio;
						break;
#if XM7_VER == 1
#if defined(JSUB)
		case JSUBCPU :	readbnio = jsubmem_readbnio;
						break;
#endif
#endif
	}
	dwMemSrchAddrSave[nCPU] = dwMemSrchAddr;
	memset(searchdata, 0, sizeof(searchdata));
	memset(string, 0, sizeof(string));
	memset(tmp, 0, sizeof(tmp));

	/* 検索文字列を変換 */
	if (szMemSrchString[0] == '#') {
		/* 16進数として許される文字のみ抜き出す */
		p = &szMemSrchString[1];
		q = string;
		while (*p != '\0') {
			if ((	((*p >= '0') && (*p <= '9')) ||
					((*p >= 'A') && (*p <= 'F')) ||
					((*p >= 'a') && (*p <= 'f')))) {
				*q++ = *p;
			}
			p++;
		}

		/* データ列へ変換 */
		datasize = 0;
		p = string;

		while (*p != '\0') {
			memcpy(tmp, p, 2);
			searchdata[datasize ++] = (BYTE)strtol(tmp, NULL, 16);
			p += 2;
		}
	}
	else {
		/* 文字列指定 */
		datasize = strlen(szMemSrchString);
		memcpy(searchdata, szMemSrchString, datasize);
	}

	/* 検索データがない場合何もしない */
	if (datasize == 0) {
		return FALSE;
	}

	/* メモリからデータを検索 */
	flag = FALSE;
#if XM7_VER == 1 && defined(Z80CARD)
	if (((nCPU == MAINCPU) || (nCPU == MAINZ80)) &&
		 isLinearAddrMode(bDumpPhysicalAddr)) {
#else
	if ((nCPU == MAINCPU) && isLinearAddrMode(bDumpPhysicalAddr)) {
#endif
		for (i=dwMemSrchAddr; i<GetMaxMemArea(bDumpPhysicalAddr) - datasize; i++) {
			dat = mainmem_readbnio_p(i);

			/* 先頭データが一致した場合に残りのデータもチェック*/
			if (searchdata[0] == dat) {
				for (j=1; j<datasize; j++) {
					if (searchdata[j] != mainmem_readbnio_p(i + j)) {
						break;
					}
				}
				if (j == datasize) {
					/* 発見 */
					LoadString(hAppInstance, IDS_SWND_SRCH_FOUND_P, tmp, sizeof(tmp));
					_snprintf(string, sizeof(string), tmp, i);
					flag = TRUE;
					break;
				}
			}
		}
	}
	else {
		for (i=(WORD)dwMemSrchAddr; i<65536 - datasize; i++) {
			dat = readbnio((WORD)i);

			/* 先頭データが一致した場合に残りのデータもチェック*/
			if (searchdata[0] == dat) {
				for (j=1; j<datasize; j++) {
					if (searchdata[j] != readbnio((WORD)(i + j))) {
						break;
					}
				}
				if (j == datasize) {
					/* 発見 */
					LoadString(hAppInstance, IDS_SWND_SRCH_FOUND, tmp, sizeof(tmp));
					_snprintf(string, sizeof(string), tmp, i);
					flag = TRUE;
					break;
				}
			}
		}
	}

	if (!flag) {
		/* 発見できず */
		LoadString(hAppInstance, IDS_SWND_SRCH_NOTFOUND, string, sizeof(string));
	}

	/* 検索結果を表示 */
	MessageBox(hMainWnd, string, "XM7", MB_OK | MB_ICONINFORMATION);
	SetMenuExitTimer();

	if (flag) {
		/* 発見できた場合はそのアドレスに飛ばす */
		AddrMemory(nCPU, i);

		/* 検索開始アドレスを更新 */
#if XM7_VER == 1 && defined(Z80CARD)
		if (((nCPU == MAINCPU) || (nCPU == MAINZ80)) &&
			 isLinearAddrMode(bDumpPhysicalAddr)) {
#else
		if ((nCPU == MAINCPU) && isLinearAddrMode(bDumpPhysicalAddr)) {
#endif
			dwMemSrchAddrSave[nCPU] = (DWORD)(i + datasize);
		}
		else {
			dwMemSrchAddrSave[nCPU] = (WORD)(i + datasize);
		}
	}

	return TRUE;
}

/*-[ メモリダンプウインドウ ]------------------------------------------------*/

/*
 *	メモリダンプウインドウ
 *	アドレス設定
 */
void FASTCALL AddrMemory(BYTE nCPU, DWORD dwAddr)
{
	SCROLLINFO sif;
	int height;

	/* アドレス移動範囲を制限 */
#if XM7_VER == 1 && defined(Z80CARD)
	if (((nCPU == MAINCPU) || (nCPU == MAINZ80)) &&
		 isLinearAddrMode(bDumpPhysicalAddr)) {
#else
	if ((nCPU == MAINCPU) && isLinearAddrMode(bDumpPhysicalAddr)) {
#endif
		dwAddr &= (DWORD)(GetMaxMemArea(bDumpPhysicalAddr) - 1);
	}
	else {
		dwAddr &= (DWORD)(0xfff0);
	}
	height = nHeightMemory[nCPU];
	if (bAsciiDump[nCPU]) {
		height -= 3;
	}
	if (dwAddr >= ((GetMaxMemArea(bDumpPhysicalAddr) + 0x0f) - height * 16)) {
		dwAddr = GetMaxMemArea(bDumpPhysicalAddr) - (height * 16);
	}

	memset(&sif, 0, sizeof(sif));
	sif.cbSize = sizeof(sif);
	sif.fMask = SIF_POS;
	sif.nPos = (int)(dwAddr >> 4);

	/* 存在チェック＆設定 */
	dwMemory[nCPU] = dwAddr;
	switch (nCPU) {
		case MAINCPU :	if (hSubWnd[SWND_MEMORY_MAIN]) {
							SetScrollInfo(hSubWnd[SWND_MEMORY_MAIN], SB_VERT,
								&sif, TRUE);
						}
						break;
		case SUBCPU :	if (hSubWnd[SWND_MEMORY_SUB]) {
							SetScrollInfo(hSubWnd[SWND_MEMORY_SUB], SB_VERT,
								&sif, TRUE);
						}
						break;
#if XM7_VER == 1
#if defined(JSUB)
		case JSUBCPU :	if (hSubWnd[SWND_MEMORY_JSUB]) {
							SetScrollInfo(hSubWnd[SWND_MEMORY_JSUB], SB_VERT,
								&sif, TRUE);
						}
						break;
#endif
#endif
	}

	/* リフレッシュ */
	RefreshMemory();
}

/*
 *	メモリダンプウィンドウ
 *	リサイズ
 */
static void FASTCALL ResizeMemory(HWND hWnd, WORD cx, WORD cy, BYTE nCPU)
{
	RECT crect;
	RECT wrect;
	SCROLLINFO si;
	BYTE *p;
	int width;
	int height;

	ASSERT(hWnd);

	/* 最小化の場合は、何もしない */
	if ((cx == 0) && (cy == 0)) {
		return;
	}

	/* 縦サイズを変更 */
	nHeightMemory[nCPU] = (WORD)(cy / lCharHeight);
	if (nHeightMemory[nCPU] < 1) {
		nHeightMemory[nCPU] = 1;
	}

	/* 横サイズを変更 */
	if (bAsciiDump[nCPU]) {
		width = 76;
	}
	else {
		width = 54;
	}

	/* ウィンドウ矩形変更 */
	GetWindowRect(hWnd, &wrect);
	GetClientRect(hWnd, &crect);
	wrect.right -= wrect.left;
	wrect.right -= crect.right;
	wrect.right += (width * lCharWidth);
	wrect.bottom -= wrect.top;
	wrect.bottom -= crect.bottom;
	wrect.bottom += (nHeightMemory[nCPU] * lCharHeight);
	SetWindowPos(hWnd, HWND_TOP, 0, 0, wrect.right, wrect.bottom,
							SWP_NOZORDER | SWP_NOMOVE);

	/* バッファを再取得し、0xFFで埋める */
	p = realloc(pMemory[nCPU], 2 * width * nHeightMemory[nCPU]);
	ASSERT(p);
	pMemory[nCPU] = p;
	memset(p, 0xff, 2 * width * nHeightMemory[nCPU]);

	/* スクロールバーの設定 */
	memset(&si, 0, sizeof(si));
	height = nHeightMemory[nCPU];
	if (bAsciiDump[nCPU]) {
		height -= 3;
	}
	si.cbSize = sizeof(si);
	si.fMask = SIF_ALL;
	si.nMin = 0;
	si.nMax = ((GetMaxMemArea(bDumpPhysicalAddr) >> 4) | 0x0f) - height;
	si.nPage = 0x10;
	si.nPos = (dwMemory[nCPU] >> 4);
	if (si.nPos > si.nMax) {
		si.nPos = si.nMax;
	}
	SetScrollInfo(hWnd, SB_VERT, &si, TRUE);

	if (bPopupSwnd) {
		SetForegroundWindow(hMainWnd);
	}
}

/*
 *	メモリダンプウィンドウ
 *	リサイズ(行数指定)
 */
static void FASTCALL ResizeMemoryLines(HWND hWnd, int lines, BYTE nCPU)
{
	RECT rect;

	GetWindowRect(hWnd, &rect);
	ResizeMemory(hWnd, 	(WORD)(rect.right - rect.left + 1), 
						(WORD)(lCharHeight * lines), nCPU);
}

/*
 *	メモリダンプウインドウ
 *	セットアップ
 */
static void FASTCALL SetupMemory(BYTE nCPU, BYTE *p, int x, int y)
{
	BYTE (FASTCALL *readbnio)(WORD addr);
	char string[128];
	char temp[32];
	DWORD addr;
	int i;
	int j;
	int height;
	BYTE mem[16];
	BYTE tsum[16];
	BYTE sum;
	BYTE knjsave;
	int offset;
	BOOL isj;

	ASSERT(nCPU < MAXCPU);
	ASSERT(p);
	ASSERT(x > 0);
	ASSERT(y > 0);

	/* 一旦スペースで埋める */
	memset(p, 0x20, x * y);
	memset(tsum, 0, 16);

	/* ワーク初期化 */
	knjsave = 0;
	isj = isJapanese;

	/* アドレス取得 */
#if XM7_VER == 1 && defined(Z80CARD)
	if (((nCPU == MAINCPU) || (nCPU == MAINZ80)) &&
		 isLinearAddrMode(bDumpPhysicalAddr)) {
#else
	if ((nCPU == MAINCPU) && isLinearAddrMode(bDumpPhysicalAddr)) {
#endif
		addr = (DWORD)(dwMemory[nCPU] & 0xffff0);
	}
	else {
		addr = (WORD)(dwMemory[nCPU] & 0xfff0);
	}
	height = nHeightMemory[nCPU];
	switch (nCPU) {
		case MAINCPU :	readbnio = mainmem_readbnio;
						break;
		case SUBCPU :	readbnio = submem_readbnio;
						break;
#if XM7_VER == 1
#if defined(JSUB)
		case JSUBCPU :	readbnio = jsubmem_readbnio;
						break;
#endif
#endif
	}

	/* ASCIIダンプモード 上部ガイド */
	if (bAsciiDump[nCPU]) {
		strncpy(string, "Addr  ", sizeof(string));
		for (j=0; j<16; j++) {
			_snprintf(temp, sizeof(temp), " +%01X", j);
			strncat(string, temp, sizeof(string) - strlen(string) - 1);
		}
		strncat(string, " Sum", sizeof(string) - strlen(string) - 1);

		/* コピー */
		memcpy(p, string, strlen(string));

		offset = 1;
		height -= 3;
	}
	else {
		offset = 0;
	}

	/* ループ */
	for (i=0; i<height; i++) {
		/* 作成 */
#if XM7_VER == 1 && defined(Z80CARD)
	if (((nCPU == MAINCPU) || (nCPU == MAINZ80)) &&
		 isLinearAddrMode(bDumpPhysicalAddr)) {
#else
		if ((nCPU == MAINCPU) && isLinearAddrMode(bDumpPhysicalAddr)) {
#endif
			_snprintf(string, sizeof(string), "%05X:", addr);
		}
		else {
			_snprintf(string, sizeof(string), "%04X :", (WORD)(addr & 0xffff));
		}
		sum = 0;
		for (j=0; j<16; j++) {
#if XM7_VER == 1 && defined(Z80CARD)
			if (((nCPU == MAINCPU) || (nCPU == MAINZ80)) &&
				 isLinearAddrMode(bDumpPhysicalAddr)) {
#else
			if ((nCPU == MAINCPU) && isLinearAddrMode(bDumpPhysicalAddr)) {
#endif
				mem[j] = mainmem_readbnio_p((DWORD)(addr + j));
			}
			else {
				mem[j] = readbnio((WORD)(addr + j));
			}
			_snprintf(temp, sizeof(temp), " %02X", mem[j]);
			tsum[j] += mem[j];
			sum += mem[j];
			strncat(string, temp, sizeof(string) - strlen(string) - 1);
		}

		if (bAsciiDump[nCPU]) {
			/* チェックサム・文字列ガイド */
			_snprintf(temp, sizeof(temp), " :%02X  ................", sum);
			strncat(string, temp, sizeof(string) - strlen(string) - 1);

			/* 漢字コードが2行にまたがる場合の処理 */
			if (isj && isKanji(knjsave)) {
				string[59] = knjsave;
				knjsave = 1;
			}

			/* ASCIIダンプ */
			for (j=0; j<16; j++) {
				if ((mem[j] < 0x20) || (mem[j] == 0x7f) || (mem[j] >= 0xfd)) {
					/* コントロールコード(非表示) */
					knjsave = 0;
				}
				else if (!isj && (mem[j] & 0x80)) {
					/* 非日本語モード 0x80～0xFF(非表示) */
					knjsave = 0;
				}
				else if (bKanjiDump[nCPU] && isKanji(mem[j]) && (knjsave == 0)) {
					/* 漢字処理 */

					if (j == 15) {
						/* 文字が次行にまたがる場合は非表示 */
						string[j + 60] = ' ';
						knjsave = mem[j];
					}
					else {
						if (isKanji2(mem[j + 1])) {
							/* 続く文字が第2バイトとなる場合 */
							string[j + 60] = mem[j];
							knjsave = 1;
						}
						else {
							/* コード異常(非表示) */
							knjsave = 0;
						}
					}
				}
				else {
					if ((mem[j] == 0xa0) && (knjsave == 0)) {
						/* データが$A0でシフトJIS第2バイトでない場合 */
						string[j + 60] = ' ';
					}
					else if ((!isKanji(mem[j]) && (mem[j] != 0x80)) || (knjsave == 1)) {
						/* ANK文字 または シフトJIS第2バイト */
						string[j + 60] = mem[j];
					}
					knjsave = 0;
				}
			}
		}

		/* コピー */
		memcpy(&p[x * (i + offset)], string, strlen(string));

		/* 次へ */
		addr = (DWORD)(addr + 0x0010) & (GetMaxMemArea(bDumpPhysicalAddr) - 1);
		if (addr == 0) {
			break;
		}
	}

	/* ASCIIダンプモード 下部罫線・縦サム・ブロックサム */
	if (bAsciiDump[nCPU]) {
		/* 罫線作成 */
		memset(string, '-', 76);
		memset(&string[58], ' ', 2);
		string[77] = '\0';

		/* コピー */
		memcpy(&p[x * (height + 1)], string, strlen(string));

		/* 縦サム作成 */
		_snprintf(string, sizeof(string), "Sum  :", addr);
		sum = 0;
		for (j=0; j<16; j++) {
			_snprintf(temp, sizeof(temp), " %02X", tsum[j]);
			sum += tsum[j];
			strncat(string, temp, sizeof(string) - strlen(string) - 1);
		}

		/* チェックサム */
		_snprintf(temp, sizeof(temp), " :%02X", sum);
		strncat(string, temp, sizeof(string) - strlen(string) - 1);

		/* コピー */
		memcpy(&p[x * (height + 2)], string, strlen(string));
	}
}

/*
 *	メモリダンプウインドウ
 *	描画
 */
static void FASTCALL DrawMemory(HWND hWnd, HDC hDC)
{
	int i;
	BYTE nCPU;
	RECT rect;
	BYTE *p;
	int x, y;

	ASSERT(hWnd);
	ASSERT(hDC);

	/* Drawバッファを得る(存在しなければ何もしない) */
	for (i=0; i<SWND_MAXNUM; i++) {
		if (hSubWnd[i] == hWnd) {
			nCPU = (BYTE)((i - SWND_MEMORY_MAIN) % MAXCPU);
			p = pMemory[nCPU];
			break;
		}
	}
	if (!p) {
		return;
	}

	/* ウインドウジオメトリを得る */
	GetClientRect(hWnd, &rect);
	x = rect.right / lCharWidth;
	y = rect.bottom / lCharHeight;
	if ((x == 0) || (y == 0)) {
		return;
	}

	/* セットアップ */
	SetupMemory(nCPU, p, x, y);

	/* 描画 */
	if (bKanjiDump[nCPU] && isJapanese) {
		DrawWindowTextKanji(hDC, p, x, y);
	}
	else {
		DrawWindowText(hDC, p, x, y);
	}
}

/*
 *	メモリダンプウインドウ
 *	リフレッシュ
 */
void FASTCALL RefreshMemory(void)
{
	HWND hWnd;
	HDC hDC;
	DWORD dwAddr;
	int height;

	/* メインCPU */
	if (hSubWnd[SWND_MEMORY_MAIN]) {
		hWnd = hSubWnd[SWND_MEMORY_MAIN];
#if XM7_VER >= 3
		if ((bExtendMMRMode != mmr_ext) || (nFM7Ver != fm7_ver)) {
#elif XM7_VER == 2
		if (nFM7Ver != fm7_ver) {
#else
		if (nFMsubtype != fm_subtype) {
#endif
#if XM7_VER >= 2
			nFM7Ver = fm7_ver;
#if XM7_VER >= 3
			bExtendMMRMode = mmr_ext;
#endif
#else
			nFMsubtype = fm_subtype;
#endif
			if (bDumpPhysicalAddr) {
#if XM7_VER >= 2
				if (fm7_ver == 1) {
#else
				if (fm_subtype != FMSUB_FM77) {
#endif
					dwMemory[MAINCPU] = mmr_trans_phys_to_logi(
						dwMemory[MAINCPU]);
				}
				else {
					dwMemory[MAINCPU] = mmr_trans_mmr(
						(WORD)(dwMemory[MAINCPU] & 0xffff));
				}
			}
			height = nHeightMemory[MAINCPU];
			if (bAsciiDump[MAINCPU]) {
				height -= 3;
			}
			ResizeMemoryLines(hWnd, nHeightMemory[MAINCPU], MAINCPU);
			dwAddr = GetMaxMemArea(bDumpPhysicalAddr) - (height * 0x10);
			if (dwMemory[MAINCPU] >= dwAddr) {
				dwMemory[MAINCPU] = dwAddr;
			}
			AddrMemory(MAINCPU, dwMemory[MAINCPU]);
			PaintMemory(hWnd);
		}
		else {
			hDC = GetDC(hWnd);
			DrawMemory(hWnd, hDC);
			ReleaseDC(hWnd, hDC);
		}
	}

	/* サブCPU */
	if (hSubWnd[SWND_MEMORY_SUB]) {
		hWnd = hSubWnd[SWND_MEMORY_SUB];
		hDC = GetDC(hWnd);
		DrawMemory(hWnd, hDC);
		ReleaseDC(hWnd, hDC);
	}

#if XM7_VER == 1
#if defined(JSUB)
	/* 日本語サブCPU */
	if (hSubWnd[SWND_MEMORY_JSUB]) {
		hWnd = hSubWnd[SWND_MEMORY_JSUB];
		hDC = GetDC(hWnd);
		DrawMemory(hWnd, hDC);
		ReleaseDC(hWnd, hDC);
	}
#endif
#endif
}

/*
 *	メモリダンプウインドウ
 *	再描画
 */
static void FASTCALL PaintMemory(HWND hWnd)
{
	HDC hDC;
	PAINTSTRUCT ps;
	int i;
	RECT rect;
	BYTE *p;
	int x, y;

	ASSERT(hWnd);

	/* Drawバッファを得る(存在しなければ何もしない) */
	for (i=0; i<SWND_MAXNUM; i++) {
		if (hSubWnd[i] == hWnd) {
			p = pMemory[((i - SWND_MEMORY_MAIN) % MAXCPU)];
			break;
		}
	}

	if (!p) {
		return;
	}

	/* ウインドウジオメトリを得る */
	GetClientRect(hWnd, &rect);
	x = rect.right / lCharWidth;
	y = rect.bottom / lCharHeight;

	/* 後半エリアをFFで埋める */
	if ((x > 0) && (y > 0)) {
		memset(&p[x * y], 0xff, x * y);
	}

	/* 描画 */
	hDC = BeginPaint(hWnd, &ps);
	ASSERT(hDC);
	DrawMemory(hWnd, hDC);
	EndPaint(hWnd, &ps);
}

/*
 *	メモリダンプウインドウ
 *	コマンド処理
 */
static void FASTCALL CmdMemory(HWND hWnd, WORD wID, BYTE nCPU)
{
	DWORD target;
	cpu6809_t *cpu;
	BYTE height;
#if XM7_VER == 1 && defined(Z80CARD)
	WORD tmp;
#endif

	/* CPU構造体決定 */
	switch (nCPU) {
		case MAINCPU :	cpu = &maincpu;
						break;
		case SUBCPU :	cpu = &subcpu;
						break;
#if XM7_VER == 1
#if defined(JSUB)
		case JSUBCPU :	cpu = &jsubcpu;
						break;
#endif
#endif
	}

	/* ターゲットアドレス決定 */
	switch (wID) {
		case IDM_DMP_ADDR:
			target = dwMemory[nCPU];
#if XM7_VER == 1 && defined(Z80CARD)
			if (((nCPU == MAINCPU) || (nCPU == MAINZ80)) &&
				 isLinearAddrMode(bDumpPhysicalAddr)) {
#else
			if ((nCPU == MAINCPU) && isLinearAddrMode(bDumpPhysicalAddr)) {
#endif
				if (!AddrDlg(hWnd, &target, nCPU, TRUE)) {
					return;
				}
			}
			else {
				if (!AddrDlg(hWnd, &target, nCPU, FALSE)) {
					return;
				}
			}
			target &= GetMaxMemArea(bDumpPhysicalAddr) - 1;
			break;

		case IDM_DMP_BLK128:
		case IDM_DMP_BLK256:
			if (wID == IDM_DMP_BLK128) {
				height = 8;
			}
			else if (wID == IDM_DMP_BLK256) {
				height = 16;
			}
			if (bAsciiDump[nCPU]) {
				height += (BYTE)3;
			}
			ResizeMemoryLines(hWnd, height, nCPU);
			return;

		case IDM_DMP_ASCII:
			bAsciiDump[nCPU] = !bAsciiDump[nCPU];

			/* 縦サイズ変更 */
			height = nHeightMemory[nCPU];
			if (bAsciiDump[nCPU]) {
				height += (BYTE)3;
			}
			else {
				height -= (BYTE)3;
			}
			ResizeMemoryLines(hWnd, height, nCPU);
			PaintMemory(hWnd);
			return;

		case IDM_DMP_KANJI:
			bKanjiDump[nCPU] = !bKanjiDump[nCPU];
			InvalidateRect(hWnd, NULL, FALSE);
			return;

		case IDM_DMP_SEARCH:
			MemorySearch(hWnd, nCPU);
			return;

		case IDM_DMP_PHYSADDR:
#if XM7_VER == 1
			if ((fm_subtype != FMSUB_FM77) || nCPU != MAINCPU) {
#else
			if ((fm7_ver <= 1) || nCPU != MAINCPU) {
#endif
				break;
			}

			if (bDumpPhysicalAddr) {
				target = mmr_trans_phys_to_logi(dwMemory[nCPU]);
			}
			else {
				target = mmr_trans_mmr((WORD)(dwMemory[nCPU] & 0xffff));
			}
			bDumpPhysicalAddr = !bDumpPhysicalAddr;
			ResizeMemoryLines(hWnd, nHeightMemory[nCPU], nCPU);
			break;

		case IDM_DMP_PC:
#if XM7_VER == 1 && defined(Z80CARD)
			if (((nCPU == MAINCPU) || (nCPU == MAINZ80)) &&
				 isLinearAddrMode(bDumpPhysicalAddr)) {
#else
			if ((nCPU == MAINCPU) && isLinearAddrMode(bDumpPhysicalAddr)) {
#endif
				target = mmr_trans_mmr(cpu->pc);
			}
			else {
				target = (DWORD)cpu->pc;
			}
			break;
		case IDM_DMP_X:
#if XM7_VER == 1 && defined(Z80CARD)
			if (((nCPU == MAINCPU) || (nCPU == MAINZ80)) &&
				 isLinearAddrMode(bDumpPhysicalAddr)) {
#else
			if ((nCPU == MAINCPU) && isLinearAddrMode(bDumpPhysicalAddr)) {
#endif
				target = mmr_trans_mmr(cpu->x);
			}
			else {
				target = (DWORD)cpu->x;
			}
			break;
		case IDM_DMP_Y:
#if XM7_VER == 1 && defined(Z80CARD)
			if (((nCPU == MAINCPU) || (nCPU == MAINZ80)) &&
				 isLinearAddrMode(bDumpPhysicalAddr)) {
#else
			if ((nCPU == MAINCPU) && isLinearAddrMode(bDumpPhysicalAddr)) {
#endif
				target = mmr_trans_mmr(cpu->y);
			}
			else {
				target = (DWORD)cpu->y;
			}
			break;
		case IDM_DMP_U:
#if XM7_VER == 1 && defined(Z80CARD)
			if (((nCPU == MAINCPU) || (nCPU == MAINZ80)) &&
				 isLinearAddrMode(bDumpPhysicalAddr)) {
#else
			if ((nCPU == MAINCPU) && isLinearAddrMode(bDumpPhysicalAddr)) {
#endif
				target = mmr_trans_mmr(cpu->u);
			}
			else {
				target = (DWORD)cpu->u;
			}
			break;
		case IDM_DMP_S:
#if XM7_VER == 1 && defined(Z80CARD)
			if (((nCPU == MAINCPU) || (nCPU == MAINZ80)) &&
				 isLinearAddrMode(bDumpPhysicalAddr)) {
#else
			if ((nCPU == MAINCPU) && isLinearAddrMode(bDumpPhysicalAddr)) {
#endif
				target = mmr_trans_mmr(cpu->s);
			}
			else {
				target = (DWORD)cpu->s;
			}
			break;
		case IDM_DMP_DP:
#if XM7_VER == 1 && defined(Z80CARD)
			if (((nCPU == MAINCPU) || (nCPU == MAINZ80)) &&
				 isLinearAddrMode(bDumpPhysicalAddr)) {
#else
			if ((nCPU == MAINCPU) && isLinearAddrMode(bDumpPhysicalAddr)) {
#endif
				target = mmr_trans_mmr((WORD)(cpu->dp << 8));
			}
			else {
				target = (DWORD)(cpu->dp << 8);
			}
			break;

#if XM7_VER == 1 && defined(Z80CARD)
			case IDM_DMP_PC_Z80:
				tmp = mainz80.pc;
				if (isLinearAddrMode(bDumpPhysicalAddr)) {
					target = mmr_trans_mmr(tmp);
				}
				else {
					target = (DWORD)tmp;
				}
				break;
			case IDM_DMP_BC:
				tmp = (WORD)((mainz80.regs8[REGID_B]) << 8 |
					mainz80.regs8[REGID_C]);
				if (isLinearAddrMode(bDumpPhysicalAddr)) {
					target = mmr_trans_mmr(tmp);
				}
				else {
					target = (DWORD)tmp;
				}
				break;
			case IDM_DMP_DE:
				tmp = (WORD)((mainz80.regs8[REGID_D]) << 8 |
					mainz80.regs8[REGID_E]);
				if (isLinearAddrMode(bDumpPhysicalAddr)) {
					target = mmr_trans_mmr(tmp);
				}
				else {
					target = (DWORD)tmp;
				}
				break;
			case IDM_DMP_HL:
				tmp = (WORD)((mainz80.regs8[REGID_H]) << 8 |
					mainz80.regs8[REGID_L]);
				if (isLinearAddrMode(bDumpPhysicalAddr)) {
					target = mmr_trans_mmr(tmp);
				}
				else {
					target = (DWORD)tmp;
				}
				break;
			case IDM_DMP_IX:
				tmp = (WORD)((mainz80.regs8[REGID_IXH]) << 8 |
					mainz80.regs8[REGID_IXL]);
				if (isLinearAddrMode(bDumpPhysicalAddr)) {
					target = mmr_trans_mmr(tmp);
				}
				else {
					target = (DWORD)tmp;
				}
				break;
			case IDM_DMP_IY:
				tmp = (WORD)((mainz80.regs8[REGID_IYH]) << 8 |
					mainz80.regs8[REGID_IYL]);
				if (isLinearAddrMode(bDumpPhysicalAddr)) {
					target = mmr_trans_mmr(tmp);
				}
				else {
					target = (DWORD)tmp;
				}
				break;
			case IDM_DMP_SP:
				tmp = mainz80.sp;
				if (isLinearAddrMode(bDumpPhysicalAddr)) {
					target = mmr_trans_mmr(tmp);
				}
				else {
					target = (DWORD)tmp;
				}
				break;
#endif

		case IDM_DMP_RESET:
			target = (WORD)((cpu->readmem(0xfffe) << 8) | cpu->readmem(0xffff));
#if XM7_VER == 1 && defined(Z80CARD)
			if (((nCPU == MAINCPU) || (nCPU == MAINZ80)) &&
				 isLinearAddrMode(bDumpPhysicalAddr)) {
#else
			if ((nCPU == MAINCPU) && isLinearAddrMode(bDumpPhysicalAddr)) {
#endif
				target = (DWORD)mmr_trans_mmr((WORD)target);
			}
			break;
		case IDM_DMP_NMI:
			target = (WORD)((cpu->readmem(0xfffc) << 8) | cpu->readmem(0xfffd));
#if XM7_VER == 1 && defined(Z80CARD)
			if (((nCPU == MAINCPU) || (nCPU == MAINZ80)) &&
				 isLinearAddrMode(bDumpPhysicalAddr)) {
#else
			if ((nCPU == MAINCPU) && isLinearAddrMode(bDumpPhysicalAddr)) {
#endif
				target = (DWORD)mmr_trans_mmr((WORD)target);
			}
			break;
		case IDM_DMP_SWI:
			target = (WORD)((cpu->readmem(0xfffa) << 8) | cpu->readmem(0xfffb));
#if XM7_VER == 1 && defined(Z80CARD)
			if (((nCPU == MAINCPU) || (nCPU == MAINZ80)) &&
				 isLinearAddrMode(bDumpPhysicalAddr)) {
#else
			if ((nCPU == MAINCPU) && isLinearAddrMode(bDumpPhysicalAddr)) {
#endif
				target = (DWORD)mmr_trans_mmr((WORD)target);
			}
			break;
		case IDM_DMP_IRQ:
			target = (WORD)((cpu->readmem(0xfff8) << 8) | cpu->readmem(0xfff9));
#if XM7_VER == 1 && defined(Z80CARD)
			if (((nCPU == MAINCPU) || (nCPU == MAINZ80)) &&
				 isLinearAddrMode(bDumpPhysicalAddr)) {
#else
			if ((nCPU == MAINCPU) && isLinearAddrMode(bDumpPhysicalAddr)) {
#endif
				target = (DWORD)mmr_trans_mmr((WORD)target);
			}
			break;
		case IDM_DMP_FIRQ:
			target = (WORD)((cpu->readmem(0xfff6) << 8) | cpu->readmem(0xfff7));
#if XM7_VER == 1 && defined(Z80CARD)
			if (((nCPU == MAINCPU) || (nCPU == MAINZ80)) &&
				 isLinearAddrMode(bDumpPhysicalAddr)) {
#else
			if ((nCPU == MAINCPU) && isLinearAddrMode(bDumpPhysicalAddr)) {
#endif
				target = (DWORD)mmr_trans_mmr((WORD)target);
			}
			break;
		case IDM_DMP_SWI2:
			target = (WORD)((cpu->readmem(0xfff4) << 8) | cpu->readmem(0xfff5));
#if XM7_VER == 1 && defined(Z80CARD)
			if (((nCPU == MAINCPU) || (nCPU == MAINZ80)) &&
				 isLinearAddrMode(bDumpPhysicalAddr)) {
#else
			if ((nCPU == MAINCPU) && isLinearAddrMode(bDumpPhysicalAddr)) {
#endif
				target = (DWORD)mmr_trans_mmr((WORD)target);
			}
			break;
		case IDM_DMP_SWI3:
			target = (WORD)((cpu->readmem(0xfff2) << 8) | cpu->readmem(0xfff3));
#if XM7_VER == 1 && defined(Z80CARD)
			if (((nCPU == MAINCPU) || (nCPU == MAINZ80)) &&
				 isLinearAddrMode(bDumpPhysicalAddr)) {
#else
			if ((nCPU == MAINCPU) && isLinearAddrMode(bDumpPhysicalAddr)) {
#endif
				target = (DWORD)mmr_trans_mmr((WORD)target);
			}
			break;
		case IDM_DMP_IO:
			switch (nCPU) {
				case MAINCPU :	if (bDumpPhysicalAddr) {
									bDumpPhysicalAddr = FALSE;
									ResizeMemoryLines(hWnd,
										nHeightMemory[nCPU], nCPU);
								}
								target = 0xfd00;
								break;
				case SUBCPU :	target = 0xd400;
								break;
				default :		target = 0x0000;
			}
			break;
		default:
			if ((wID >= IDM_DMP_STACK) && (wID <= IDM_DMP_STACK + 15)) {
				target = mmr_trans_mmr((WORD)wStackAddr[wID - IDM_DMP_STACK]);
			}
			if ((wID >= IDM_DMP_00000) && (wID <= IDM_DMP_F0000)) {
				target = (wID - IDM_DMP_00000) * 0x10000;
			}
			break;
	}

	/* 設定＆更新 */
	AddrMemory(nCPU, target);
}

/*
 *	メモリダンプウインドウ
 *	ウインドウプロシージャ
 */
static LRESULT CALLBACK MemoryProc(HWND hWnd, UINT message,
								 WPARAM wParam, LPARAM lParam, BYTE nCPU)
{
	static BOOL bResize = FALSE;		/* 手動リサイズフラグ */
	HMENU hMenu;
	POINT point;
	DWORD dwAddr;
	int height;
	UINT idcheck;
	RECT rect;
#if XM7_VER == 1 && defined(Z80CARD)
	int i;
#endif

	/* メッセージ別 */
	switch (message) {
		/* ウインドウ再描画 */
		case WM_PAINT:
			/* ロックが必要 */
			LockVM();
			PaintMemory(hWnd);
			UnlockVM();
			return 0;

		/* ウインドウ削除 */
		case WM_DESTROY:
			LockVM();

			/* メインウインドウへ自動通知 */
			DestroySubWindow(hWnd, &pMemory[nCPU], hMemory[nCPU]);

			UnlockVM();
			break;

		/* ウインドウサイズ変更 */
		case WM_SIZE:
			if (bResize) {
				bResize = FALSE;
				ResizeMemory(hWnd, LOWORD(lParam), HIWORD(lParam), nCPU);
				height = nHeightMemory[nCPU];
				if (bAsciiDump[nCPU]) {
					height -= 3;
				}
#if XM7_VER == 1
				if ((nCPU == MAINCPU) || (nCPU == MAINZ80)) {
#else
				if (nCPU == MAINCPU) {
#endif
					dwAddr = GetMaxMemArea(bDumpPhysicalAddr) - (height * 0x10);
				}
				else {
					dwAddr = 0x10000 - (height * 0x10);
				}
				if (dwMemory[nCPU] >= dwAddr) {
					AddrMemory(nCPU, dwAddr);
					PaintMemory(hWnd);
				}
			}
			break;

		/* ウインドウサイズ変更中 */
		case WM_SIZING:
			WindowSizing(hWnd, (LPRECT)lParam, &pMemory[nCPU]);
			bResize = TRUE;
			break;

		/* ウィンドウサイズ変更メッセージ */
		case WM_WINDOWPOSCHANGING:
			bResize = TRUE;
			break;

		/* 最小サイズ制限 */
		case WM_GETMINMAXINFO:
			rect.left = 0;
			rect.right = 54 * lCharWidth + GetSystemMetrics(SM_CXVSCROLL);
			rect.top = 0;
			rect.bottom = lCharHeight;

			/* ASCIIダンプモード時の縦サムスペースを考慮 */
			if (bAsciiDump[nCPU]) {
				rect.right += (22 * lCharWidth);
				rect.bottom += (lCharHeight * 3);
			}

			if (bPopupSwnd) {
				AdjustWindowRect(&rect, WS_POPUP | WS_OVERLAPPED | WS_SYSMENU |
										WS_CAPTION | WS_VISIBLE | 
										WS_MINIMIZEBOX | WS_SIZEBOX, FALSE);
			}
			else {
				AdjustWindowRect(&rect, WS_CHILD | WS_OVERLAPPED | WS_SYSMENU |
										WS_CAPTION | WS_VISIBLE | 
										WS_MINIMIZEBOX | WS_CLIPSIBLINGS | 
										WS_SIZEBOX, FALSE);
			}
			((LPMINMAXINFO)lParam)->ptMinTrackSize.x = rect.right - rect.left;
			((LPMINMAXINFO)lParam)->ptMaxTrackSize.x = rect.right - rect.left;
			((LPMINMAXINFO)lParam)->ptMinTrackSize.y = rect.bottom - rect.top;
			return 0;

		/* メモリ内容変更 */
		case WM_LBUTTONDBLCLK:
#if defined(MOUSE)
			/* マウスエミュレーション使用時は無効 */
			if (mos_capture && !stopreq_flag && run_flag) {
				return FALSE;
			}
#endif

			/* カーソル位置から、決定 */
			point.x = LOWORD(lParam) / lCharWidth;
			point.y = HIWORD(lParam) / lCharHeight;
			if (bAsciiDump[nCPU]) {
				point.y --;
			}

			MemoryChange(hWnd, nCPU, point.x, point.y);
			return 0;

		/* コンテキストメニュー */
		case WM_CONTEXTMENU:
#if defined(MOUSE)
			/* マウスエミュレーション使用時は無効 */
			if (mos_capture && !stopreq_flag && run_flag) {
				return FALSE;
			}
#endif

			/* サブメニューを取り出す */
			hMenu = GetSubMenu(hMemory[nCPU], 0);
			height = nHeightMemory[nCPU];
			if (bAsciiDump[nCPU]) {
				height -= 3;
			}

			/* 表示サイズチェック */
			CheckMenuSub(hMenu, IDM_DMP_BLK128, FALSE);
			CheckMenuSub(hMenu, IDM_DMP_BLK256, FALSE);
			if (height == 8) {
				idcheck = IDM_DMP_BLK128;
			}
			else if (height == 16) {
				idcheck = IDM_DMP_BLK256;
			}
			else {
				idcheck = 0;
			}
			if (idcheck > 0) {
				CheckMenuRadioItem(hMenu, IDM_DMP_BLK128, IDM_DMP_BLK256, idcheck, MF_BYCOMMAND);
			}

			/* ASCIIダンプチェック */
			CheckMenuSub(hMenu, IDM_DMP_ASCII, bAsciiDump[nCPU]);

			/* 漢字表示チェック */
			if (isJapanese) {
				EnableMenuSub(hMenu, IDM_DMP_KANJI, bAsciiDump[nCPU]);
				CheckMenuSub(hMenu, IDM_DMP_KANJI, bKanjiDump[nCPU]);
			}
			else {
				EnableMenuSub(hMenu, IDM_DMP_KANJI, FALSE);
				CheckMenuSub(hMenu, IDM_DMP_KANJI, FALSE);
			}

#if XM7_VER == 1
#if defined(JSUB)
			/* I/O領域チェック */
			if (nCPU == JSUBCPU) {
				EnableMenuSub(hMenu, IDM_DMP_IO, FALSE);
			}
			else {
				EnableMenuSub(hMenu, IDM_DMP_IO, TRUE);
			}
#endif
#endif

			/* 物理アドレス */
#if XM7_VER == 1
#if defined(Z80CARD)
			if ((fm_subtype == FMSUB_FM77) &&
				((nCPU == MAINCPU) || (nCPU == MAINZ80))) {
#else
			if ((fm_subtype == FMSUB_FM77) && (nCPU == MAINCPU)) {
#endif
#else
			if ((fm7_ver >= 2) && (nCPU == MAINCPU)) {
#endif
				EnableMenuSub(hMenu, IDM_DMP_PHYSADDR, TRUE);
				CheckMenuSub(hMenu, IDM_DMP_PHYSADDR, bDumpPhysicalAddr);
				if (bDumpPhysicalAddr) {
					EnableMenuItem(hMenu, 9, MF_BYPOSITION | MF_ENABLED);
				}
				else {
					EnableMenuItem(hMenu, 9, MF_BYPOSITION | MF_GRAYED);
				}
			}
			else {
				EnableMenuSub(hMenu, IDM_DMP_PHYSADDR, FALSE);
				CheckMenuSub(hMenu, IDM_DMP_PHYSADDR, FALSE);
				EnableMenuItem(hMenu, 9, MF_BYPOSITION | MF_GRAYED);
			}
			InsertPhysicalMemMenu(GetSubMenu(hMenu, 9), nCPU, IDM_DMP_00000);

#if XM7_VER == 1 && defined(Z80CARD)
			/* サブCPU/日本語サブCPU時のZ80レジスタ削除 */
			if ((nCPU != MAINCPU) && (nCPU != MAINZ80)) {
				for (i = 6; i < 13; i++) {
					DeleteMenu(GetSubMenu(hMenu, 11), 6, MF_BYPOSITION);
				}
			}
#endif

			/* システムスタックジャンプメニュー設定 */
			InsertStackJumpMenu(GetSubMenu(hMenu, 13), nCPU, IDM_DMP_STACK);

			/* コンテキストメニューを実行 */
			point.x = LOWORD(lParam);
			point.y = HIWORD(lParam);
			TrackPopupMenu(hMenu, 0, point.x, point.y, 0, hWnd, NULL);
			return 0;

		/* コマンド */
		case WM_COMMAND:
			CmdMemory(hWnd, LOWORD(wParam), nCPU);
			break;

		/* 垂直スクロールバー */
		case WM_VSCROLL:
			/* タイプ判別 */
			dwAddr = dwMemory[nCPU];
			height = nHeightMemory[nCPU];
			if (bAsciiDump[nCPU]) {
				height -= 3;
			}

			/* アクション別 */
			switch (LOWORD(wParam)) {
				/* トップ */
				case SB_TOP:
					dwAddr = 0;
					break;
				/* 終端 */
				case SB_BOTTOM:
#if XM7_VER == 1 && defined(Z80CARD)
					if ((nCPU == MAINCPU) || (nCPU == MAINZ80)) {
#else
					if (nCPU == MAINCPU) {
#endif
						dwAddr = (DWORD)(GetMaxMemArea(bDumpPhysicalAddr) - height * 16);
					}
					else {
						dwAddr = (DWORD)(0x10000 - height * 16);
					}
					break;
				/* １行上 */
				case SB_LINEUP:
					if (dwAddr >= 0x0010) {
						dwAddr -= (WORD)0x0010;
					}
					break;
				/* １行下 */
				case SB_LINEDOWN:
#if XM7_VER == 1 && defined(Z80CARD)
					if ((nCPU == MAINCPU) || (nCPU == MAINZ80)) {
#else
					if (nCPU == MAINCPU) {
#endif
						if (dwAddr < GetMaxMemArea(bDumpPhysicalAddr) - (DWORD)(height * 16)) {
							dwAddr += (DWORD)0x0010;
						}
					}
					else {
						if (dwAddr < 0x10000 - (DWORD)(height * 16)) {
							dwAddr += (DWORD)0x0010;
						}
					}
					break;
				/* ページアップ */
				case SB_PAGEUP:
					if (dwAddr <= (DWORD)(height * 16)) {
						dwAddr = 0;
					}
					else {
						dwAddr -= (DWORD)(height * 0x10);
#if XM7_VER == 1 && defined(Z80CARD)
						if ((nCPU == MAINCPU) || (nCPU == MAINZ80)) {
#else
						if (nCPU == MAINCPU) {
#endif
							dwAddr &= (GetMaxMemArea(bDumpPhysicalAddr) - 1);
						}
						else {
							dwAddr &= 0xffff;
						}
					}
					break;
				/* ページダウン */
				case SB_PAGEDOWN:
#if XM7_VER == 1 && defined(Z80CARD)
					if ((nCPU == MAINCPU) || (nCPU == MAINZ80)) {
#else
					if (nCPU == MAINCPU) {
#endif
						if (dwAddr >= (GetMaxMemArea(bDumpPhysicalAddr) - (height * 16))) {
							dwAddr = GetMaxMemArea(bDumpPhysicalAddr) - (height * 16);
						}
						else {
							dwAddr += (DWORD)(height * 16);
							dwAddr &= (GetMaxMemArea(bDumpPhysicalAddr) - 1);
						}
					}
					else {
						if (dwAddr >= (0x10000 - (DWORD)(height * 16))) {
							dwAddr = (0x10000 - (DWORD)(height * 16));
						}
						else {
							dwAddr += (DWORD)(height * 16);
							dwAddr &= 0xffff;
						}
					}
					break;
				/* 直接指定 */
				case SB_THUMBTRACK:
#if XM7_VER == 1 && defined(Z80CARD)
					if ((nCPU == MAINCPU) || (nCPU == MAINZ80)) {
#else
					if (nCPU == MAINCPU) {
#endif
						dwAddr = (DWORD)(HIWORD(wParam) * 16) &
							(GetMaxMemArea(bDumpPhysicalAddr) - 1);
						if (dwAddr >= ((GetMaxMemArea(bDumpPhysicalAddr) + 0x0f) -
							(DWORD)(height * 16))) {
							dwAddr = GetMaxMemArea(bDumpPhysicalAddr) - (DWORD)(height * 16);
						}
					}
					else {
						dwAddr = (DWORD)(HIWORD(wParam) * 16) & 0xffff;
						if (dwAddr >= (0x1000f - (DWORD)(height * 16))) {
							dwAddr = 0x10000 - (DWORD)(height * 16);
						}
					}
					break;
			}
			dwAddr &= (GetMaxMemArea(bDumpPhysicalAddr) - 0x10);
			AddrMemory(nCPU, dwAddr);
			RefreshMemory();
			break;
	}

	/* デフォルト ウインドウプロシージャ */
	return DefWindowProc(hWnd, message, wParam, lParam);
}

/*
 *	メモリダンプウインドウ(メイン)
 *	ウインドウプロシージャ
 */
static LRESULT CALLBACK MemoryProcMain(HWND hWnd, UINT message,
								 WPARAM wParam, LPARAM lParam)
{
	return MemoryProc(hWnd, message, wParam, lParam, MAINCPU);
}

/*
 *	メモリダンプウインドウ(サブ)
 *	ウインドウプロシージャ
 */
static LRESULT CALLBACK MemoryProcSub(HWND hWnd, UINT message,
								 WPARAM wParam, LPARAM lParam)
{
	return MemoryProc(hWnd, message, wParam, lParam, SUBCPU);
}

#if XM7_VER == 1
/*
 *	メモリダンプウインドウ(日本語サブ)
 *	ウインドウプロシージャ
 */
#if defined(JSUB)
static LRESULT CALLBACK MemoryProcJsub(HWND hWnd, UINT message,
								 WPARAM wParam, LPARAM lParam)
{
	return MemoryProc(hWnd, message, wParam, lParam, JSUBCPU);
}
#endif
#endif

/*
 *	メモリダンプウインドウ
 *	ウインドウ作成
 */
HWND FASTCALL CreateMemory(HWND hParent, BYTE nCPU, int index)
{
	WNDCLASSEX wcex;
	char szClassNameMain[] = "XM7_MemoryMain";
	char szClassNameSub[] = "XM7_MemorySub";
#if XM7_VER == 1
#if defined(JSUB)
	char szClassNameJsub[] = "XM7_MemoryJsub";
#endif
#endif
	char *szClassName;
	char szWndName[128];
	RECT rect;
	RECT crect, wrect;
	HWND hWnd;
	int height;
	SCROLLINFO si;
	UINT id;
	DWORD pc;
	DWORD dwStyle;

	ASSERT(hParent);

	/* MMR拡張モード、機種バージョン/サブバージョンの初期値を設定 */
#if XM7_VER >= 2
	nFM7Ver = fm7_ver;
#if XM7_VER >= 3
	bExtendMMRMode = mmr_ext;
#endif
#else
	nFMsubtype = fm_subtype;
#endif

	/* ウインドウ矩形を計算 */
	PositioningSubWindow(hParent, &rect, index);

	/* ウインドウタイトルを決定、バッファ確保、メニューロード */
	switch (nCPU) {
		case MAINCPU :	id = IDS_SWND_MEMORY_MAIN;
						if (bDumpPhysicalAddr) {
							pc = maincpu.pc;
							if (!mmr_trans_twr(maincpu.pc, &pc)) {
								pc = mmr_trans_mmr(maincpu.pc);
							}
						}
						else {
							pc = maincpu.pc;
						}
						szClassName = szClassNameMain;
						break;
		case SUBCPU :	id = IDS_SWND_MEMORY_SUB;
						pc = subcpu.pc;
						szClassName = szClassNameSub;
						break;
#if XM7_VER == 1
#if defined(JSUB)
		case JSUBCPU :	id = IDS_SWND_MEMORY_JSUB;
						pc = jsubcpu.pc;
						szClassName = szClassNameJsub;
						break;
#endif
#endif
	}
	rect.bottom = lCharHeight * nHeightMemory[nCPU];
	LoadString(hAppInstance, id, szWndName, sizeof(szWndName));
	pMemory[nCPU] = malloc(2 * 77 * nHeightMemory[nCPU]);
	dwMemory[nCPU] = pc;
#if XM7_VER == 1 && defined(Z80CARD)
	if (nCPU == MAINCPU) {
		hMemory[nCPU] = LoadMenu(hAppInstance, MAKEINTRESOURCE(IDR_MEMDMPMAINMENU));
	}
	else {
		hMemory[nCPU] = LoadMenu(hAppInstance, MAKEINTRESOURCE(IDR_MEMDMPMENU));
	}
#else
	hMemory[nCPU] = LoadMenu(hAppInstance, MAKEINTRESOURCE(IDR_MEMDMPMENU));
#endif

	/* ウィンドウ幅決定 */
	if (bAsciiDump[nCPU]) {
		rect.right = lCharWidth * 76;
	}
	else {
		rect.right = lCharWidth * 54;
	}

	/* ウインドウクラスの登録 */
	memset(&wcex, 0, sizeof(wcex));
	wcex.cbSize = sizeof(wcex);
	wcex.style = CS_VREDRAW | CS_HREDRAW | CS_DBLCLKS;
	switch (nCPU) {
		case MAINCPU :	wcex.lpfnWndProc = MemoryProcMain;
						break;
		case SUBCPU :	wcex.lpfnWndProc = MemoryProcSub;
						break;
#if XM7_VER == 1
#if defined(JSUB)
		case JSUBCPU :	wcex.lpfnWndProc = MemoryProcJsub;
						break;
#endif
#endif
	}
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hAppInstance;
	wcex.hIcon = LoadIcon(hAppInstance, MAKEINTRESOURCE(IDI_WNDICON));
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	wcex.lpszMenuName = NULL;
	wcex.lpszClassName = szClassName;
	wcex.hIconSm = LoadIcon(hAppInstance, MAKEINTRESOURCE(IDI_WNDICON));
	RegisterClassEx(&wcex);

	/* ウインドウ作成 */
	if (bPopupSwnd) {
		dwStyle =	WS_POPUP | WS_OVERLAPPED | WS_SYSMENU | WS_CAPTION |
					WS_VISIBLE | WS_MINIMIZEBOX | WS_VSCROLL | WS_SIZEBOX;
	}
	else {
		dwStyle =	WS_CHILD | WS_OVERLAPPED | WS_SYSMENU | WS_CAPTION |
					WS_VISIBLE | WS_MINIMIZEBOX | WS_CLIPSIBLINGS |
					WS_VSCROLL | WS_SIZEBOX;
	}
	hWnd = CreateWindow(szClassName,
						szWndName,
						dwStyle,
						rect.left,
						rect.top,
						rect.right,
						rect.bottom,
						hParent,
						NULL,
						hAppInstance,
						NULL);

	/* 有効なら、サイズ補正して手前に置く */
	if (hWnd) {
		GetWindowRect(hWnd, &wrect);
		GetClientRect(hWnd, &crect);
		wrect.right += (rect.right - crect.right);
		wrect.bottom += (rect.bottom - crect.bottom);
		SetWindowPos(hWnd, HWND_TOP, wrect.left, wrect.top,
			wrect.right - wrect.left, wrect.bottom - wrect.top, SWP_NOMOVE);

		/* スクロールバーの設定が必要 */
		memset(&si, 0, sizeof(si));
		si.cbSize = sizeof(si);
		si.fMask = SIF_ALL;
		si.nMin = 0;
		si.nPage = 0x10;
		height = nHeightMemory[nCPU];
		si.nPos = (pc >> 4);
		if (bAsciiDump) {
			height -= 3;
		}
		si.nMax = ((GetMaxMemArea(bDumpPhysicalAddr) >> 4) + 0x0f) - height;
		SetScrollInfo(hWnd, SB_VERT, &si, TRUE);
	}

	/* ポップアップウインドウ時はアクティブウインドウを前面に変更 */
	if (bPopupSwnd) {
		SetForegroundWindow(hMainWnd);
	}

	/* 結果を持ち帰る */
	return hWnd;
}

#endif	/* _WIN32 */
