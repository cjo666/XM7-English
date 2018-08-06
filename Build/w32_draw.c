/*
 *	FM-7 EMULATOR "XM7"
 *
 *	Copyright (C) 1999-2017 ＰＩ．(Twitter:@xm6_original)
 *	Copyright (C) 2001-2017 Ryu Takegami (Twitter:@RyuTakegami)
 *
 *	[ Win32API 表示 ]
 *
 *	RHG履歴
 *	  2002.04.01		非ディスプレイバンク書き換え時の通知動作抑制を400ライ
 *						ンモード時のみ抑制(意味不明…)
 *	  2002.04.23		↑を200ラインモードでも抑制(ヴァク)
 *	  2002.06.20		200ライン8色モードの裏バンクへの書き込み時は通知動作を
 *						抑制するようにした(とかいいながらまた大バグ出たりして)
 *						ハードウェアウィンドウ用通知関数を新設
 *	  2002.09.15		V2憑きバージョンとソースを統合
 *	  2004.03.17		疑似400ラインアダプタ対応
 *	  2008.01.20		↑なかったことにした
 *	  2012.05.01		フルスクリーン状態保存処理への対応
 *	  2012.07.01		グリーンモニタモードに対応
 *	  2013.02.12		ラスタレンダリングに対応
 */

#ifdef _WIN32

#define STRICT
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <assert.h>
#include "xm7.h"
#include "device.h"
#include "subctrl.h"
#include "display.h"
#include "multipag.h"
#include "w32.h"
#include "w32_gdi.h"
#include "w32_dd.h"
#include "w32_draw.h"
#include "w32_sub.h"

/*
 *	グローバル ワーク
 */
BOOL bFullScreen;							/* フルスクリーンフラグ */
BOOL bFullRequested;						/* フルスクリーン要求 */
BOOL bDrawSelected;							/* セレクト済みフラグ */
BOOL bFullScan;								/* フルスキャン(Window) */
BOOL bFullScanFS;							/* フルスキャン(FullScreen) */
BOOL bDoubleSize;							/* 2倍拡大フラグ */
BOOL bPseudo400Line;						/* 疑似400ラインフラグ */
#if XM7_VER == 1
BOOL bGreenMonitor;							/* グリーンモニタフラグ */
#endif
#if XM7_VER == 2
BOOL bTTLMonitor;							/* TTLモニタフラグ */
#endif
BOOL bRasterRendering;						/* ラスタレンダリングフラグ */

/*
 *	スタティック ワーク
 */
static BOOL bNextFrameRender;				/* 次フレーム描画フラグ */
static BOOL bDirtyLine[400];				/* 要書き換えフラグ */

/*
 *	初期化
 */
void FASTCALL InitDraw(void)
{
	/* ワークエリア初期化 */
	bFullScreen = FALSE;
	bFullRequested = FALSE;
	bDrawSelected = FALSE;
	bFullScan = FALSE;
	bFullScanFS = FALSE;
	bDoubleSize = FALSE;
	bPseudo400Line = FALSE;
#if XM7_VER == 1
	bGreenMonitor = FALSE;
#endif
#if XM7_VER == 2
	bTTLMonitor = FALSE;
#endif
	bRasterRendering = FALSE;
	bNextFrameRender = FALSE;
	memset(bDirtyLine, 0, sizeof(bDirtyLine));

	/* 起動後はGDIなので、GDIを初期化 */
	InitGDI();
}

/*
 *	クリーンアップ
 */
void FASTCALL CleanDraw(void)
{
	/* フルスクリーンフラグに応じて、クリーンアップ */
	if (bFullScreen) {
		CleanDD();
	}
	else {
		CleanGDI();
	}
}

/*
 *  セレクト
 */
BOOL FASTCALL SelectDraw(HWND hWnd)
{
	ASSERT(hWnd);

	/* セレクト済み */
	bDrawSelected = TRUE;

	/* 起動後はGDIを選択 */
	return SelectGDI(hWnd);
}

/*
 *	モード切り替え
 */
void FASTCALL ModeDraw(HWND hWnd, BOOL bFullFlag)
{
	ASSERT(hWnd);

	/* 現状と一致していれば、変える必要なし */
	if (bFullFlag == bFullScreen) {
		return;
	}

	if (bFullFlag) {
		/* フルスクリーンへ */
		CleanGDI();
		InitDD();
		bFullScreen = TRUE;
		if (!SelectDD()) {
			/* フルスクリーン失敗 */
			bFullScreen = FALSE;
			CleanDD();
			InitGDI();
			SelectGDI(hWnd);
		}
	}
	else {
		/* ウインドウへ */
		bFullScreen = FALSE;
		CleanDD();
		InitGDI();
		if (!SelectGDI(hWnd)) {
			/* ウインドウ失敗 */
			CleanGDI();
			bFullScreen = TRUE;
			InitDD();
			SelectDD();
		}
	}

	/* 再描画のため少し細工 */
	display_notify();
}

/*
 *	描画(通常)
 */
void FASTCALL OnDraw(HWND hWnd, HDC hDC)
{
	ASSERT(hWnd);
	ASSERT(hDC);

	/* エラーなら何もしない */
	if (nErrorCode == 0) {
		if (bFullScreen) {
			if (bRasterRendering) {
				DrawPostRenderDD();
			}
			else {
				DrawDD();
			}
		}
		else {
			if (bRasterRendering) {
				DrawPostRenderGDI(hWnd, hDC);
			}
			else {
				DrawGDI(hWnd, hDC);
			}
		}
	}
}

/*
 *	描画(WM_PAINT)
 */
void FASTCALL OnPaint(HWND hWnd)
{
	PAINTSTRUCT ps;
	HDC hDC;

	ASSERT(hWnd);

	hDC = BeginPaint(hWnd, &ps);

	/* エラーなら何もしない */
	if (nErrorCode == 0) {
		/* 再描画指示 */
		if (bFullScreen) {
			ReDrawDD();
		}
		else {
			ReDrawGDI();
		}

		/* 描画 */
		OnDraw(hWnd, hDC);
	}

	EndPaint(hWnd, &ps);
}

/*
 *	メニュー開始
 */
void FASTCALL EnterMenu(HWND hWnd)
{
	if (bFullScreen) {
		EnterMenuDD(hWnd);
	}
	else {
		EnterMenuGDI(hWnd);
	}
}

/*
 *	メニュー終了
 */
void FASTCALL ExitMenu(void)
{
	if (bFullScreen) {
		ExitMenuDD();
	}
	else {
		ExitMenuGDI();
	}
}

/*-[ VMとの接続 ]-----------------------------------------------------------*/

/*
 *	再描画ラスタ一括設定
 */
void FASTCALL SetDirtyFlag(int top, int bottom, BOOL flag)
{
	int y;

	if (bRasterRendering) {
		for (y = top; y < bottom; y++) {
			bDirtyLine[y] = flag;
		}
	}
}

/*
 *	VRAM書き込み通知
 */
void FASTCALL vram_notify(WORD addr, BYTE dat)
{
	WORD y;

	UNUSED(dat);

#if XM7_VER >= 2
#if XM7_VER >= 3
	if (screen_mode == SCR_200LINE) {
#else
	if (!mode320) {
#endif
		if (vram_active != vram_display) {
			return;
		}
	}
#endif

	/* Y座標算出 */
	if (bRasterRendering) {
#if XM7_VER >= 3
		switch (screen_mode) {
			case SCR_400LINE	:	addr &= 0x7fff;
									y = (WORD)(addr / 80);
									break;
			case SCR_262144		:
			case SCR_4096		:	addr &= 0x1fff;
									y = (WORD)(addr / 40);
									break;
			case SCR_200LINE	:	addr &= 0x3fff;
									y = (WORD)(addr / 80);
									break;
		}
#elif XM7_VER >= 2
		if (mode320) {
			addr &= 0x1fff;
			y = (WORD)(addr / 40);
		}
		else {
			addr &= 0x3fff;
			y = (WORD)(addr / 80);
		}
#elif defined(L4CARD)
		if (enable_400line) {
			addr -= vram_offset[0];
			addr &= 0x7fff;
			y = (WORD)(addr / 80);
		}
		else {
			addr &= 0x3fff;
			y = (WORD)(addr / 80);
		}
#else
		addr &= 0x3fff;
		y = (WORD)(addr / 80);
#endif
		if (y < 400) {
			bDirtyLine[y] = TRUE;
		}
	}

	if (bFullScreen) {
		VramDD(addr);
	}
	else {
		VramGDI(addr);
	}
}


/*
 *	TTLパレット通知
 */
void FASTCALL ttlpalet_notify(void)
{
	if (bRasterRendering) {
		bNextFrameRender = TRUE;
		SetDirtyFlag(now_raster, 400, TRUE);
	}

	if (bFullScreen) {
		DigitalDD();
	}
	else {
		DigitalGDI();
	}
}

/*
 *	アナログパレット通知
 */
void FASTCALL apalet_notify(void)
{
	if (bRasterRendering) {
		bNextFrameRender = TRUE;
		SetDirtyFlag(now_raster, 400, TRUE);
	}

	if (bFullScreen) {
		AnalogDD();
	}
	else {
		AnalogGDI();
	}
}

/*
 *  パレット変更通知
 */
void FASTCALL refpalet_notify(void)
{
	bPaletteRefresh = TRUE;
}

/*
 *  再描画要求通知
 */
void FASTCALL display_notify(void)
{
	int i;
	int raster;

	if (bRasterRendering) {
		bNextFrameRender = TRUE;
		SetDirtyFlag(0, 400, TRUE);
		if (!run_flag) {
			raster = now_raster;
			for (i = 0; i < 400; i++) {
				now_raster = i;
				hblank_notify();
			}
			now_raster = raster;
		}
	}

	if (bFullScreen) {
		/* ReDrawは無駄なクリアを含むので、多少細工 */
		AnalogDD();
	}
	else {
		AnalogGDI();
	}
}

/*
 *	VBLANK終了要求通知
 */
void FASTCALL vblankperiod_notify(void)
{
	BOOL flag;
	int y;
	int ymax;

	if (bRasterRendering) {
		/* 次のフレームを強制的に書き換えるか */
		if (bNextFrameRender) {
			bNextFrameRender = FALSE;
			SetDirtyFlag(0, 400, TRUE);
		}
		else {
			/* 書き換えが必要かチェック */
#if XM7_VER >= 3
			if (screen_mode == SCR_400LINE) {
				ymax = 400;
			}
			else {
				ymax = 200;
			}
#elif XM7_VER == 1 && defined(L4CARD)
			if (enable_400line) {
				ymax = 400;
			}
			else {
				ymax = 200;
			}
#else
			ymax = 200;
#endif
			flag = FALSE;
			for (y = 0; y < ymax; y++) {
				flag |= bDirtyLine[y];
			}
			if (!flag) {
				return;
			}
		}

		/* 全領域をレンダリング */
		if (bFullScreen) {
			/* ReDrawは無駄なクリアを含むので、多少細工 */
			AnalogDD();
		}
		else {
			AnalogGDI();
		}
	}
}

/*
 *	HBLANK要求通知
 */
void FASTCALL hblank_notify(void)
{
	/* VMが動いていない場合は何もしない */
	if (!run_flag) {
		return;
	}

	if (bRasterRendering) {
		if (bDirtyLine[now_raster]) {
			bDirtyLine[now_raster] = FALSE;
			if (bFullScreen) {
				DrawRasterDD(now_raster);
			}
			else {
				DrawRasterGDI(now_raster);
			}
		}
	}
}

/*
 *	ディジタイズ要求通知
 */
void FASTCALL digitize_notify(void)
{
}

#if XM7_VER >= 3
/*
 *	ハードウェアウィンドウ通知
 */
void FASTCALL window_notify(void)
{
	if (bFullScreen) {
		WindowDD();
	}
	else {
		WindowGDI();
	}
}
#endif

#if XM7_VER == 1 && defined(L4CARD)
/*
 *	テキストVRAM書き込み通知
 */
void FASTCALL tvram_notify(WORD addr, BYTE dat)
{
	WORD ysize;
	WORD y;

	UNUSED(dat);

	if (bRasterRendering) {
		/* Y座標算出 */
		addr = (WORD)((addr - text_start_addr) & 0x0ffe);
		ysize = (WORD)((crtc_register[9] & 0x1f) + 1);
		if (width40_flag) {
			y = (WORD)((addr / 80) * ysize);
		}
		else {
			y = (WORD)((addr / 160) * ysize);
		}

		/* オーバーチェック */
		if (y >= 400) {
			return;
		}

		/* 再描画フラグを設定 */
		SetDirtyFlag(y, y + ysize, TRUE);
	}

	if (bFullScreen) {
		TvramDD(addr);
	}
	else {
		TvramGDI(addr);
	}
}

/*
 *  テキスト再描画要求通知
 */
void FASTCALL tvram_redraw_notify(void)
{
	bNextFrameRender = TRUE;
	SetDirtyFlag(now_raster, 400, TRUE);

	if (bFullScreen) {
		ReDrawTVRamDD();
	}
	else {
		ReDrawTVRamGDI();
	}
}
#endif

/*-[ V2 ]-------------------------------------------------------------------*/

#if XM7_VER <= 2 && defined(FMTV151)
/*
 *	V2
 *	0=透過、1=緑、2=黒
 */
const BYTE nV2data[] = {
/*						  1 1 1 1 1 1 1 1 1 1 2 2 2 2 2 2 2 2 */
/*		1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 */
		1,1,1,2,0,0,0,0,1,1,1,2,0,0,0,0,0,1,1,1,1,1,1,1,2,0,0,
		1,1,1,2,0,0,0,0,1,1,1,2,0,0,0,0,1,1,1,1,1,1,1,1,1,2,0,
		1,1,1,2,0,0,0,0,1,1,1,2,0,0,0,1,1,1,1,2,0,0,1,1,1,1,2,
		1,1,1,2,0,0,0,0,1,1,1,2,0,0,0,1,1,1,2,0,0,0,0,1,1,1,2,
		1,1,1,2,0,0,0,0,1,1,1,2,0,0,0,0,0,0,0,0,0,0,0,1,1,1,2,
		1,1,1,2,0,0,0,0,1,1,1,2,0,0,0,0,0,0,0,0,0,0,1,1,1,1,2,
		1,1,1,2,0,0,0,0,1,1,1,2,0,0,0,0,0,0,0,0,0,1,1,1,1,2,0,
		1,1,1,2,0,0,0,0,1,1,1,2,0,0,0,0,0,0,0,0,1,1,1,1,2,0,0,
		1,1,1,2,0,0,0,0,1,1,1,2,0,0,0,0,0,0,0,1,1,1,1,2,0,0,0,
		1,1,1,1,2,0,0,1,1,1,1,2,0,0,0,0,0,0,1,1,1,1,2,0,0,0,0,
		0,1,1,1,1,2,1,1,1,1,2,0,0,0,0,0,0,1,1,1,1,2,0,0,0,0,0,
		0,0,1,1,1,1,1,1,1,2,0,0,0,0,0,0,1,1,1,1,2,0,0,0,0,0,0,
		0,0,0,1,1,1,1,1,2,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,2,
		0,0,0,0,1,1,1,2,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,2,
};

#endif

#endif	/* _WIN32 */
