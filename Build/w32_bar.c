/*
 *	FM-7 EMULATOR "XM7"
 *
 *	Copyright (C) 1999-2017 ＰＩ．(Twitter:@xm6_original)
 *	Copyright (C) 2001-2017 Ryu Takegami (Twitter:@RyuTakegami)
 *
 *	[ Win32API コントロールバー ]
 *
 *	RHG履歴
 *	  2003.05.25		ResizeStatus関数を追加
 *	  2004.05.03		WinXP対応処理のDLL読み込みチェックの安全性を強化
 *	  2004.08.31		2D/2DD/VFD/タイトルなしD77の表示処理を変更
 *	  2010.08.05		インジケータ周りをXM6に合わせた仕様に変更
 *	  2010.10.03		2倍拡大表示モードに対応
 *	  2012.06.03		ステータスバーの表示状態の初期値をSW_HIDEに変更
 *	  2012.08.01		CAP/かな/INSキー表示メッセージをリソースから読むように
 *						変更
 *	  2012.10.10		キャプションのバッファサイズを256バイトに増大
 */

#ifdef _WIN32

#define STRICT
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winbase.h>
#include <commctrl.h>
#include <stdlib.h>
#include <assert.h>
#include "xm7.h"
#include "keyboard.h"
#include "tapelp.h"
#include "display.h"
#include "ttlpalet.h"
#include "apalet.h"
#include "subctrl.h"
#include "fdc.h"
#include "bubble.h"
#include "w32.h"
#include "w32_bar.h"
#include "w32_sch.h"
#include "w32_draw.h"
#include "w32_res.h"

/*
 *	グローバル ワーク
 */
HWND hStatusBar;						/* ステータスバー */
BOOL bStatusMessage;					/* ステータスメッセージ表示状態 */
int uPaneX[3];							/* Drag&Drop用ペイン位置 */

/*
 *	スタティック ワーク
 */
static char szIdleMessage[128];			/* アイドルメッセージ */
static char szStatusMessage[128];		/* ステータスメッセージ */
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
static char szDrive[2][128];			/* フロッピードライブ */
static int nTape;						/* テープ */
static UINT uResID;						/* リソースID保存 */
static int nStatusBorder;				/* ステータスバーのボーダー描画状態 */
static BOOL (*IsAppThemed)(void);		/* テーマ適用確認関数(uxtheme.dll) */
static HINSTANCE hInstTheme;			/* uxtheme.dllのハンドル */
static BOOL bThemed;					/* テーマ適用状態 */

/*-[ ステータスバー ]-------------------------------------------------------*/

/*
 *	アクセスマクロ
 */
#define Status_SetParts(hwnd, nParts, aWidths) \
	SendMessage((hwnd), SB_SETPARTS, (WPARAM) nParts, (LPARAM) (LPINT) aWidths)

#define Status_SetText(hwnd, iPart, uType, szText) \
	SendMessage((hwnd), SB_SETTEXT, (WPARAM) (iPart | uType), (LPARAM) (LPSTR) szText)

/*
 *	ペイン定義
 */
#define PANE_DEFAULT	0
#define PANE_DRIVE1		1
#define PANE_DRIVE0		2
#define PANE_TAPE		3
#define PANE_CAP		4
#define PANE_KANA		5
#define PANE_INS		6


/*
 *	ステータスバー作成
 */
HWND FASTCALL CreateStatus(HWND hParent)
{
	HWND hWnd;

	ASSERT(hParent);

	/* テーマ適用フラグ初期化 */
	bThemed = FALSE;

	/* メッセージをロード */
	if (LoadString(hAppInstance, IDS_IDLEMESSAGE,
					szIdleMessage, sizeof(szIdleMessage)) == 0) {
		szIdleMessage[0] = '\0';
	}
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

	/* ステータスメッセージを初期化 */
	szStatusMessage[0] = '\0';
	bStatusMessage = FALSE;
	uResID = 0;

	/* ステータスバーを作成 */
	hWnd = CreateStatusWindow(WS_CHILD | CCS_BOTTOM | CCS_NORESIZE,
								"", hParent, ID_STATUS_BAR);
	ChangeStatusBorder(hWnd);
	ResizeStatus(hParent, hWnd);
	ShowWindow(hWnd, SW_HIDE);

	return hWnd;
}

/*
 *	キャプション描画
 */
static void FASTCALL DrawMainCaption(void)
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

#if XM7_VER == 1 
#if defined(BUBBLE)
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
#endif

	/* 比較描画 */
	string[255] = '\0';
	if (memcmp(szCaption, string, strlen(string) + 1) != 0) {
		strncpy(szCaption, string, sizeof(szCaption));
		SetWindowText(hMainWnd, szCaption);
	}
}

/*
 *	アイコンをチェック、変更
 */
static void FASTCALL CheckIcon(void)
{
	HICON hIcon;

#if XM7_VER == 1
	/* 一致チェック */
	if (fm_subtype == nAppIcon) {
		return;
	}

	/* 新アイコンをロード、セット */
	switch (fm_subtype) {
		case FMSUB_FM8:
			hIcon = LoadIcon(hAppInstance, MAKEINTRESOURCE(IDI_FM8ICON));
			break;
		case FMSUB_FM7:
			hIcon = LoadIcon(hAppInstance, MAKEINTRESOURCE(IDI_FM7ICON));
			break;
		case FMSUB_FM77:
			hIcon = LoadIcon(hAppInstance, MAKEINTRESOURCE(IDI_FM77ICON));
			break;
		default:
			hIcon = NULL;
			ASSERT(FALSE);
			break;
	}
	SendMessage(hMainWnd, WM_SETICON, ICON_BIG, (LPARAM)hIcon);
	SendMessage(hMainWnd, WM_SETICON, ICON_SMALL, (LPARAM)hIcon);

	/* 旧アイコンを削除 */
	DestroyIcon(hAppIcon);

	/* 記憶 */
	hAppIcon = hIcon;
	nAppIcon = fm_subtype;
#else
	/* 一致チェック */
	if (fm7_ver == nAppIcon) {
		return;
	}

	/* 新アイコンをロード、セット */
	switch (fm7_ver) {
		case 1:
			hIcon = LoadIcon(hAppInstance, MAKEINTRESOURCE(IDI_FM7ICON));
			break;
		case 2:
			hIcon = LoadIcon(hAppInstance, MAKEINTRESOURCE(IDI_77AVICON));
			break;
#if XM7_VER >= 3
		case 3:
			hIcon = LoadIcon(hAppInstance, MAKEINTRESOURCE(IDI_40EXICON2));
			break;
#endif
		default:
			hIcon = NULL;
			ASSERT(FALSE);
			break;
	}
	SendMessage(hMainWnd, WM_SETICON, ICON_BIG, (LPARAM)hIcon);
	SendMessage(hMainWnd, WM_SETICON, ICON_SMALL, (LPARAM)hIcon);

	/* 旧アイコンを削除 */
	DestroyIcon(hAppIcon);

	/* 記憶 */
	hAppIcon = hIcon;
	nAppIcon = fm7_ver;
#endif
}

/*
 *	CAPキー描画
 */
static void FASTCALL DrawCAP(void)
{
	int num;

	/* 番号決定 */
	if (caps_flag) {
		num = 1;
	}
	else {
		num = 0;
	}

	/* 同じなら何もしない */
	if (nCAP == num) {
		return;
	}

	/* 描画、ワーク更新 */
	nCAP = num;
	Status_SetText(hStatusBar, PANE_CAP, SBT_OWNERDRAW, PANE_CAP);
}

/*
 *	かなキー描画
 */
static void FASTCALL DrawKANA(void)
{
	int num;

	/* 番号決定 */
	if (kana_flag) {
		num = 1;
	}
	else {
		num = 0;
	}

	/* 同じなら何もしない */
	if (nKANA == num) {
		return;
	}

	/* 描画、ワーク更新 */
	nKANA = num;
	Status_SetText(hStatusBar, PANE_KANA, SBT_OWNERDRAW, PANE_KANA);
}

/*
 *	INSキー描画
 */
static void FASTCALL DrawINS(void)
{
	int num;

	/* 番号決定 */
	if (ins_flag) {
		num = 1;
	}
	else {
		num = 0;
	}

	/* 同じなら何もしない */
	if (nINS == num) {
		return;
	}

	/* 描画、ワーク更新 */
	nINS = num;
	Status_SetText(hStatusBar, PANE_INS, SBT_OWNERDRAW, PANE_INS);
}

/*
 *	ドライブ描画
 */
static void FASTCALL DrawDrive(int drive, UINT nID)
{
	char string[_MAX_FNAME + _MAX_EXT];
	char buffer[_MAX_FNAME + _MAX_EXT + 4];
	char drive_[_MAX_DRIVE];
	char dir[_MAX_DIR];
	char fname[_MAX_FNAME];
	char ext[_MAX_EXT];
	int num;
	char *name;

	ASSERT((drive >= 0) && (drive <= 1));

	/* 番号セット */
	if ((fdc_ready[drive] == FDC_TYPE_NOTREADY) || (fdc_teject[drive])) {
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
			return;
		}
	}

	/* 描画 */
	nDrive[drive] = num;
	strncpy(szDrive[drive], name, sizeof(szDrive[drive]));
	Status_SetText(hStatusBar, nID, SBT_OWNERDRAW, nID);
}

/*
 *	テープ描画
 */
static void FASTCALL DrawTape(void)
{
	int num;

	/* ナンバー計算 */
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
		return;
	}

	/* 描画 */
	nTape = num;
	Status_SetText(hStatusBar, PANE_TAPE, SBT_OWNERDRAW, PANE_TAPE);
}

/*
 *	描画
 */
void FASTCALL DrawStatus(void)
{
	/* ウインドウチェック */
	if (!hMainWnd) {
		return;
	}

	DrawMainCaption();
	CheckIcon();

	/* 全画面、ステータスバーチェック */
	if (bFullScreen || !hStatusBar) {
		return;
	}

	DrawCAP();
	DrawKANA();
	DrawINS();
	DrawDrive(0, PANE_DRIVE0);
	DrawDrive(1, PANE_DRIVE1);
	DrawTape();
}

/*
 *	再描画
 */
void FASTCALL PaintStatus(void)
{
	/* 記憶ワークをすべてクリアする */
	szCaption[0] = '\0';
	nCAP = -1;
	nKANA = -1;
	nINS = -1;
	nDrive[0] = -1;
	nDrive[1] = -1;
	szDrive[0][0] = '\0';
	szDrive[1][0] = '\0';
	nTape = -1;

	/* 描画 */
	DrawStatus();
}

/*
 *	キャラクタ描画
 */
static void FASTCALL DrawChar(HDC hDC, BYTE c, int x, int y, UINT color)
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
 *	オーナードロー
 */
void FASTCALL OwnerDrawStatus(DRAWITEMSTRUCT *pDI)
{
	HBRUSH hBrush;
	COLORREF fColor;
	COLORREF bColor;
	RECT srect;
	char string[_MAX_FNAME + _MAX_EXT + 4];
	int drive;

	ASSERT(pDI);

	/* 文字列、色を決定 */
	switch (pDI->itemData) {
		/* フロッピーディスク */
		case PANE_DRIVE1:
		case PANE_DRIVE0:
			if (pDI->itemData == PANE_DRIVE0) {
				drive = 0;
			}
			else {
				drive = 1;
			}
			if ((nDrive[drive] == FDC_ACCESS_NOTREADY) &&
				(!fdc_teject[drive])) {
				strncpy(string, "", sizeof(string));
			}
			else {
				strncpy(string, szDrive[drive], sizeof(string));
			}
			fColor = RGB(255, 255, 255);
			bColor = RGB(63, 63, 63);
			if ((nDrive[drive] == FDC_ACCESS_NOTREADY) ||
				(fdc_teject[drive])) {
				bColor = RGB(0, 0, 0);
			}
			if (nDrive[drive] == FDC_ACCESS_READ) {
				bColor = RGB(191, 0, 0);
			}
			if (nDrive[drive] == FDC_ACCESS_WRITE) {
				bColor = RGB(0, 0, 191);
			}
			break;

		/* テープ */
		case PANE_TAPE:
			if (nTape >= 30000) {
				string[0] = '\0';
			}
			else {
				_snprintf(string, sizeof(string), "%04d", nTape % 10000);
			}
			fColor = RGB(255, 255, 255);
			bColor = RGB(63, 63, 63);
			if (nTape >= 10000) {
				if (nTape >= 30000) {
					bColor = RGB(0, 0, 0);
				}
				else if (nTape >= 20000) {
					bColor = RGB(0, 0, 191);
				}
				else {
					bColor = RGB(191, 0, 0);
				}
			}
			break;

		/* CAP */
		case PANE_CAP:
			fColor = RGB(255, 255, 255);
			strncpy(string, szCAPMessage, sizeof(string));
			if (nCAP) {
				bColor = RGB(255, 0, 0);
			}
			else {
				bColor = RGB(0, 0, 0);
			}
			break;

		/* かな */
		case PANE_KANA:
			fColor = RGB(255, 255, 255);
			strncpy(string, szKANAMessage, sizeof(string));
			if (nKANA) {
				bColor = RGB(255, 0, 0);
			}
			else {
				bColor = RGB(0, 0, 0);
			}
			break;

		/* INS */
		case PANE_INS:
			fColor = RGB(255, 255, 255);
			strncpy(string, szINSMessage, sizeof(string));
			if (nINS) {
				bColor = RGB(255, 0, 0);
			}
			else {
				bColor = RGB(0, 0, 0);
			}
			break;

		/* それ以外 */
		default:
			ASSERT(FALSE);
			return;
	}

	/* ブラシで塗る */
	hBrush = CreateSolidBrush(bColor);
	if (hBrush) {
		FillRect(pDI->hDC, &(pDI->rcItem), hBrush);
		DeleteObject(hBrush);
	}

	/* テキストの影を描画 */
	srect.left = pDI->rcItem.left + 1;
	srect.right = pDI->rcItem.right + 1;
	srect.top = pDI->rcItem.top + 1;
	srect.bottom = pDI->rcItem.bottom + 1;
	SetTextColor(pDI->hDC, RGB(0, 0, 0));
	SetBkMode(pDI->hDC, TRANSPARENT);
	DrawText(pDI->hDC, string, strlen(string), &srect,
				DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);

	/* テキストを描画 */
	SetTextColor(pDI->hDC, fColor);
	SetBkMode(pDI->hDC, TRANSPARENT);	/* for WinXP Luna */
	DrawText(pDI->hDC, string, strlen(string), &(pDI->rcItem),
				DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);

	/* ドライブ番号描画 */
	if ((pDI->itemData == PANE_DRIVE1) || (pDI->itemData == PANE_DRIVE0)) {
		DrawChar(pDI->hDC, (BYTE)(0x30 + drive),
				 pDI->rcItem.right - 8, pDI->rcItem.bottom - 8,
				 RGB(255, 255, 255));
	}
}

/*
 *	サイズ変更
 */
void FASTCALL SizeStatus(LONG cx)
{
	HDC hDC;
	TEXTMETRIC tm;
	LONG cw;
	UINT uPane[7];

	ASSERT(cx > 0);
	ASSERT(hStatusBar);

	/* テキストメトリックを取得 */
	hDC = GetDC(hStatusBar);
	GetTextMetrics(hDC, &tm);
	ReleaseDC(hStatusBar, hDC);
	cw = tm.tmAveCharWidth;

	/* ペインサイズを決定(ペイン右端の位置を設定) */
	uPane[PANE_INS] = cx;
	uPane[PANE_KANA] = uPane[PANE_INS] - cw * 4;
	uPane[PANE_CAP] = uPane[PANE_KANA] - cw * 4;
	uPane[PANE_TAPE] = uPane[PANE_CAP] - cw * 4;
	uPane[PANE_DRIVE0] = uPane[PANE_TAPE] - cw * 5;
	uPane[PANE_DRIVE1] = uPane[PANE_DRIVE0] - cw * 16;
	uPane[PANE_DEFAULT] = uPane[PANE_DRIVE1] - cw * 16;

	/* Drag&Drop用ペイン座標を設定 */
	uPaneX[0] = uPane[PANE_DEFAULT];
	uPaneX[1] = uPane[PANE_DRIVE1];
	uPaneX[2] = uPane[PANE_DRIVE0];

	/* ペインサイズ設定 */
	Status_SetParts(hStatusBar, sizeof(uPane)/sizeof(UINT), uPane);

	/* 再描画 */
	PaintStatus();
}

/*
 *	ステータスバーリサイズ
 */
void FASTCALL ResizeStatus(HWND hwnd, HWND hstatus)
{
	RECT rect;
	int height;

	/* 背景色変更 */
	SendMessage(hstatus, SB_SETBKCOLOR, 0, GetSysColor(COLOR_3DFACE));

	/* 高さを補正 */
	GetWindowRect(hMainWnd, &rect);
	if (bDoubleSize) {
		height = 800;
	}
	else {
		height = 400;
	}
	MoveWindow(hstatus, 0, height, (rect.right - rect.left),
			   GetSystemMetrics(SM_CYMENU), TRUE);

	/* フレームウインドウのサイズを補正 */
	GetClientRect(hwnd, &rect);
	PostMessage(hwnd, WM_SIZE, 0, MAKELPARAM(rect.right, rect.bottom));
}

/*
 *	ステータスメッセージ表示
 */
void FASTCALL SetStatusMessage(UINT ID)
{
	ASSERT(hStatusBar);

	/* ステータスバーが表示されていなければ、何もしない */
	if (!IsWindowVisible(hStatusBar)) {
		return;
	}

	/* 指定IDの文字列リソースロードを試みる */
	if (LoadString(hAppInstance, ID, szStatusMessage, sizeof(szStatusMessage)) != 0) {
		/* ステータスメッセージをセット */
		Status_SetText(hStatusBar, 0, nStatusBorder, szStatusMessage);

		/* ステータスメッセージ用タイマを設定する */
		if (bStatusMessage) {
			KillTimer(hMainWnd, ID_STATUS_BAR);
		}
		SetTimer(hMainWnd, ID_STATUS_BAR, 1500, NULL);

		/* ステータスメッセージ表示フラグを立てる */
		bStatusMessage = TRUE;
	}
	else {
		/* リソースロードに失敗した場合、表示フラグを降ろす */
		bStatusMessage = FALSE;
	}
}

/*
 *	ステータスメッセージ終了
 */
void FASTCALL EndStatusMessage(void)
{
	char buffer[128];

	ASSERT(hStatusBar);

	/* もともとステータスメッセージが表示されてなければ、何もしない */
	if (!bStatusMessage) {
		return;
	}

	/* ステータスメッセージ表示フラグを降ろす */
	bStatusMessage = FALSE;

	/* ステータスバーが表示されていなければ、何もしない */
	if (!IsWindowVisible(hStatusBar)) {
		return;
	}

	if (uResID != 0) {
		/* 文字列リソースロードを試みる */
		if (LoadString(hAppInstance, uResID, buffer, sizeof(buffer)) == 0) {
			buffer[0] = '\0';
		}
	}
	else {
		/* アイドルメッセージをコピー */
		strncpy(buffer, szIdleMessage, sizeof(buffer));
	}

	/* セット */
	Status_SetText(hStatusBar, 0, nStatusBorder, buffer);
}

/*
 *	メニューセレクト
 */
void FASTCALL OnMenuSelect(WPARAM wParam)
{
	char buffer[128];
	UINT uID;

	ASSERT(hStatusBar);

	/* ステータスバーが表示されていなければ、何もしない */
	if (!IsWindowVisible(hStatusBar)) {
		return;
	}

	/* ロードするリソースIDを決定 */
	uID = (UINT)LOWORD(wParam);
	if (((uID >= IDM_D0MEDIA00) && (uID <= IDM_D0MEDIA15)) ||
		((uID >= IDM_D1MEDIA00) && (uID <= IDM_D1MEDIA15))) {
		/* メディア交換 */
		uID = IDS_MEDIA_CHANGE;
	}

	/* 文字列リソースロードを試みる */
	if (LoadString(hAppInstance, uID, buffer, sizeof(buffer)) == 0) {
		buffer[0] = '\0';
	}

	/* セット */
	Status_SetText(hStatusBar, 0, nStatusBorder, buffer);

	/* リソースID(兼リロードフラグ)を保存 */
	uResID = uID;
}

/*
 *	メニュー終了
 */
void FASTCALL OnExitMenuLoop(void)
{
	ASSERT(hStatusBar);

	/* ステータスバーが表示されていなければ、何もしない */
	if (!IsWindowVisible(hStatusBar)) {
		return;
	}

	/* アイドルメッセージを表示 */
	if (bStatusMessage) {
		Status_SetText(hStatusBar, 0, nStatusBorder, szStatusMessage);
	}
	else {
		Status_SetText(hStatusBar, 0, nStatusBorder, szIdleMessage);
	}

	/* リソースID(兼リロードフラグ)を保存 */
	uResID = 0;
}

/*-[ WindowsXP対応処理 ]----------------------------------------------------*/

/*
 *	WindowsXP ビジュアルスタイル対応用処理 08/14
 *	UXTHEME.DLL初期化
 */
void FASTCALL InitThemeDLL(void)
{
	/* uxtheme.dllをロード */
	hInstTheme = LoadLibrary("uxtheme.dll");

	/* 関数の先頭アドレスを設定 */
	if (hInstTheme) {
		IsAppThemed = (BOOL(*)(void))GetProcAddress(hInstTheme, "IsAppThemed");

		if (!IsAppThemed) {
			/* 失敗 */
			CleanThemeDLL();
		}
	}
}

/*
 *	WindowsXP ビジュアルスタイル対応用処理
 *	UXTHEME.DLLクリーンアップ
 */
void FASTCALL CleanThemeDLL(void)
{
	/* DLLが読み込まれていたら開放 */
	if (hInstTheme) {
		FreeLibrary(hInstTheme);
		IsAppThemed = NULL;
		hInstTheme = NULL;
	}
}

/*
 *	WindowsXP ビジュアルスタイル対応用処理
 *	ステータスメッセージ部ボーダー描画状態変更
 */
void FASTCALL ChangeStatusBorder(HWND hwnd)
{
	/* テーマ適用状態では枠を描画する */
	if (hInstTheme && IsAppThemed) {
		if (!IsAppThemed()) {
			nStatusBorder = SBT_NOBORDERS;
			bThemed = FALSE;
		}
		else {
			nStatusBorder = 0;
			bThemed = TRUE;
		}
	}
	else {
		nStatusBorder = SBT_NOBORDERS;
		bThemed = FALSE;
	}

	/* アイドルメッセージを表示 */
	if (bStatusMessage) {
		Status_SetText(hwnd, 0, nStatusBorder, szStatusMessage);
	}
	else {
		Status_SetText(hwnd, 0, nStatusBorder, szIdleMessage);
	}
}

#endif	/* _WIN32 */
