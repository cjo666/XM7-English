/*
 *	FM-7 EMULATOR "XM7"
 *
 *	Copyright (C) 1999-2017 ＰＩ．(Twitter:@xm6_original)
 *	Copyright (C) 2001-2017 Ryu Takegami (Twitter:@RyuTakegami)
 *
 *	[ Win32API バージョン情報ダイアログ ]
 *
 *	RHG履歴
 *	  2002.05.07		リンク先増強(ばく
 *	  2002.07.17		マウスポインタ形状が変わったままになる問題を修正
 *	  2002.09.09		バージョン文字列の設定方法を変更 (VC++対策)
 *	  2010.05.11		フルスクリーン時のリンク先クリックを無効にした
 */

#ifdef _WIN32

#define STRICT
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <shellapi.h>
#include <assert.h>
#include "xm7.h"
#include "w32.h"
#include "w32_res.h"
#include "w32_draw.h"
#include "w32_kbd.h"

/*
 *	グローバル ワーク
 */
BOOL bGravestone = FALSE;				/* !? */

/*
 *	スタティック ワーク
 */
static RECT AboutURLRect[4];			/* URLテキスト矩形 */
static BOOL bAboutURLHit[4];			/* URLフォーカスフラグ */
static char pszAboutURL[4][128];		/* URLテキスト */
static RECT IconRect;					/* アイコン矩形 */
static HICON hVerIcon;					/* アイコンハンドル */


/*
 *	ダイアログ初期化
 */
static void FASTCALL AboutDlgInit(HWND hDlg)
{
	/* バージョン文字列 */
	#if XM7_VER == 2 && defined(FMTV151)
		#if defined(LEVEL)
			/* レベルあり */
			#define	VER		VERSION" "LEVEL"-V2憑き"
			#define	VER_IN	VERSION" "LEVEL"-V2"
		#else
			/* レベルなし */
			#define	VER		VERSION"-V2憑き"
			#define	VER_IN	VERSION"-V2"
		#endif
	#else
		#if defined(LEVEL)
			/* レベルあり */
			#define	VER		VERSION" "LEVEL
			#define	VER_IN	VER
		#else
			/* レベルなし */
			#define	VER		VERSION
			#define	VER_IN	VER
		#endif
	#endif

	#if defined(BETAVER)
		#if defined(BETANO)
			/* β2以降 */
			#define	VERSTR		VER"β"BETANO" ("DATE")"
			#define	VERSTR_IN	VER_IN"-beta"BETANO" ("DATE")"
		#else
			/* β1(番号表記なし) */
			#define	VERSTR		VER"β ("DATE")"
			#define	VERSTR_IN	VER_IN"-beta ("DATE")"
		#endif
	#else
		/* 正式版 */
		#define	VERSTR			VER" ("DATE")"
		#define	VERSTR_IN		VER_IN" ("DATE")"
	#endif

	HWND hWnd;
	RECT prect;
	RECT drect;
	POINT point;
	int i;

	ASSERT(hDlg);

	/* バージョン文字列を設定 */
	hWnd = GetDlgItem(hDlg, IDC_VERSION);
	SetWindowText(hWnd, VERSTR);

	/* 文字列リソースをロード */
#if defined(ROMEO)
	for (i=0; i<4; i++) {
#else
	for (i=0; i<3; i++) {
#endif
		pszAboutURL[i][0] = '\0';
		LoadString(hAppInstance, IDS_ABOUTURL + i, pszAboutURL[i],
			sizeof(pszAboutURL[i]));
	}

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

	/* IDC_URLのクライアント座標を取得 */
#if defined(ROMEO)
	for (i=0; i<4; i++) {
#else
	for (i=0; i<3; i++) {
#endif
		hWnd = GetDlgItem(hDlg, IDC_URL + i);
		ASSERT(hWnd);
		GetWindowRect(hWnd, &prect);

		point.x = prect.left;
		point.y = prect.top;
		ScreenToClient(hDlg, &point);
		AboutURLRect[i].left = point.x;
		AboutURLRect[i].top = point.y;

		point.x = prect.right;
		point.y = prect.bottom;
		ScreenToClient(hDlg, &point);
		AboutURLRect[i].right = point.x;
		AboutURLRect[i].bottom = point.y;

		/* IDC_URLを削除 */
		DestroyWindow(hWnd);

		/* その他 */
		bAboutURLHit[i] = FALSE;
	}

	/* IDC_ABOUTICONのクライアント座標を取得 */
	hWnd = GetDlgItem(hDlg, IDC_ABOUTICON);
	ASSERT(hWnd);
	GetWindowRect(hWnd, &prect);

	point.x = prect.left;
	point.y = prect.top;
	ScreenToClient(hDlg, &point);
	IconRect.left = point.x;
	IconRect.top = point.y;

	point.x = prect.right;
	point.y = prect.bottom;
	ScreenToClient(hDlg, &point);
	IconRect.right = point.x;
	IconRect.bottom = point.y;

	/* IDC_ABOUTICONを削除 */
	DestroyWindow(hWnd);

	/* アイコンをロード */
#if XM7_VER >= 2
	switch (fm7_ver) {
		case 1:
			hVerIcon = LoadIcon(hAppInstance, MAKEINTRESOURCE(IDI_FM7ICON));
			break;
		case 2:
			hVerIcon = LoadIcon(hAppInstance, MAKEINTRESOURCE(IDI_77AVICON));
			break;
#if XM7_VER >= 3
		case 3:
#if defined(DEBUG)
			hVerIcon = LoadIcon(hAppInstance, MAKEINTRESOURCE(IDI_40SXICON));
#else
			if (bGravestone) {
				hVerIcon = LoadIcon(hAppInstance,
					 MAKEINTRESOURCE(IDI_40SXICON));
			}
			else {
				hVerIcon = LoadIcon(hAppInstance,
					 MAKEINTRESOURCE(IDI_40EXICON));
			}
#endif
			break;
#endif
		default:
			hVerIcon = NULL;
			ASSERT(FALSE);
			break;
	}
#else
	switch (fm_subtype) {
		case FMSUB_FM8:
			hVerIcon = LoadIcon(hAppInstance, MAKEINTRESOURCE(IDI_FM8ICON));
			break;
		case FMSUB_FM7:
			hVerIcon = LoadIcon(hAppInstance, MAKEINTRESOURCE(IDI_FM7ICON));
			break;
		case FMSUB_FM77:
		default:
			hVerIcon = LoadIcon(hAppInstance, MAKEINTRESOURCE(IDI_FM77ICON));
			break;
	}
#endif

	ASSERT(hVerIcon);

	#undef VER
}

/*
 *	ダイアログ描画
 */
static void AboutDlgPaint(HWND hDlg)
{
	HDC hDC;
	PAINTSTRUCT ps;
	HFONT hFont;
	HFONT hDefFont;
	TEXTMETRIC tm;
	LOGFONT lf;
	int i;

	ASSERT(hDlg);

	/* DCを取得 */
	hDC = BeginPaint(hDlg, &ps);

	/* GUIフォントのメトリックを得る */
	hFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
	hDefFont = (HFONT)SelectObject(hDC, hFont);
	GetTextMetrics(hDC, &tm);
	memset(&lf, 0, sizeof(lf));
	GetTextFace(hDC, LF_FACESIZE, lf.lfFaceName);
	SelectObject(hDC, hDefFont);

	/* アンダーラインを付加したフォントを作成 */
	lf.lfHeight = tm.tmHeight;
	lf.lfWidth = 0;
	lf.lfEscapement = 0;
	lf.lfOrientation = 0;
	lf.lfWeight = FW_DONTCARE;
	lf.lfItalic = tm.tmItalic;
	lf.lfUnderline = TRUE;
	lf.lfStrikeOut = tm.tmStruckOut;
	lf.lfCharSet = tm.tmCharSet;
	lf.lfOutPrecision = OUT_DEFAULT_PRECIS;
	lf.lfClipPrecision = CLIP_DEFAULT_PRECIS;
	lf.lfQuality = DEFAULT_QUALITY;
	lf.lfPitchAndFamily = tm.tmPitchAndFamily;
	hFont = CreateFontIndirect(&lf);
	hDefFont = (HFONT)SelectObject(hDC, hFont);

	/* 描画 */
#if defined(ROMEO)
	for (i=0; i<4; i++) {
#else
	for (i=0; i<3; i++) {
#endif
		if (bAboutURLHit[i]) {
			SetTextColor(hDC, RGB(255, 0, 0));
		}
		else {
			SetTextColor(hDC, RGB(0, 0, 255));
		}
		SetBkColor(hDC, GetSysColor(COLOR_3DFACE));
		SetBkMode(hDC, OPAQUE);
		DrawText(hDC, pszAboutURL[i], strlen(pszAboutURL[i]),
			&AboutURLRect[i], DT_CENTER | DT_SINGLELINE);
	}

	/* フォントを戻す */
	SelectObject(hDC, hDefFont);
	DeleteObject(hFont);

	/* アイコン描画 */
	DrawIcon(hDC, IconRect.left, IconRect.top, hVerIcon);

	/* DC解放 */
	EndPaint(hDlg, &ps);
}

/*
 *	ダイアログプロシージャ
 */
static BOOL CALLBACK AboutDlgProc(HWND hDlg, UINT iMsg,
									WPARAM wParam, LPARAM lParam)
{
	POINT point;
	BOOL bFlag;
	HCURSOR hCursor;
	int i;

	switch (iMsg) {
		/* ダイアログ初期化 */
		case WM_INITDIALOG:
			AboutDlgInit(hDlg);
			return TRUE;

		/* 再描画 */
		case WM_PAINT:
			AboutDlgPaint(hDlg);
			return 0;

		/* コマンド */
		case WM_COMMAND:
			switch (LOWORD(wParam)) {
				/* 終了 */
				case IDOK:
				case IDCANCEL:
					DestroyIcon(hVerIcon);
					EndDialog(hDlg, 0);
					InvalidateRect(hDrawWnd, NULL, FALSE);
					return TRUE;
			}
			break;

		/* 領域チェック */
		case WM_NCHITTEST:
			point.x = LOWORD(lParam);
			point.y = HIWORD(lParam);
			ScreenToClient(hDlg, &point);
			/* ＰＩ．氏サイト対策 */
#if defined(ROMEO)
			for (i=1; i<4; i++) {
#else
			for (i=1; i<3; i++) {
#endif
				bFlag = PtInRect(&AboutURLRect[i], point);
				/* フラグが異なったら、更新して再描画 */
				if (bFlag != bAboutURLHit[i]) {
					bAboutURLHit[i] = bFlag;
					InvalidateRect(hDlg, &AboutURLRect[i], FALSE);
				}
			}
			break;

		/* カーソル設定 */
		case WM_SETCURSOR:
			/* ＰＩ．氏サイト対策 */
#if defined(ROMEO)
			for (i=1; i<4; i++) {
#else
			for (i=1; i<3; i++) {
#endif
				if (bAboutURLHit[i]) {
					/* OSのバージョンによってはIDC_HANDは失敗する */
					hCursor = LoadCursor(NULL, MAKEINTRESOURCE(32649));
					if (!hCursor) {
						hCursor = LoadCursor(NULL, IDC_IBEAM);
					}
					SetCursor(hCursor);
					return TRUE;
				}
			}
			break;

		case WM_LBUTTONDOWN:
		case WM_LBUTTONDBLCLK:
			/* フルスクリーン時はブラウザを起動しない */
			if (bFullScreen) {
				break;
			}
			
#if defined(ROMEO)
			for (i=1; i<4; i++) {
#else
			for (i=1; i<3; i++) {
#endif
				if (bAboutURLHit[i]) {
					ShellExecute(hDlg, NULL, pszAboutURL[i], NULL, NULL,
						SW_SHOWNORMAL);
					return TRUE;
				}
			}
			break;
	}

	/* それ以外はFALSE */
	return FALSE;
}

/*
 *	バージョン情報
 */
void FASTCALL OnAbout(HWND hWnd)
{
	ASSERT(hWnd);

	/* モーダルダイアログ実行 */
	DialogBox(hAppInstance, MAKEINTRESOURCE(IDD_ABOUTDLG), hWnd, AboutDlgProc);
	SetMenuExitTimer();
}

#endif	/* _WIN32 */
