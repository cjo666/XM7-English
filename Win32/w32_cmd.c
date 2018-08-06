/*
 *	FM-7 EMULATOR "XM7"
 *
 *	Copyright (C) 1999-2017 ＰＩ．(Twitter:@xm6_original)
 *	Copyright (C) 2001-2017 Ryu Takegami (Twitter:@RyuTakegami)
 *	line printer support 2010-2013 by Ben.JP
 *
 *	[ Win32API メニューコマンド ]
 *
 *	RHG履歴
 *	  2001.08.01		V2での新規2DDディスクイメージ作成機能を廃止
 *	  2001.08.08		ホットリセット機能を追加
 *	  2002.01.24		時刻アジャスト機能でのVMロック処理を廃止
 *	  2002.03.03		Alt+F11の連打で落ちることがある問題を修正
 *	  2002.05.04		F-BASICユーザディスク作成機能を追加
 *						2D/VFD変換と新規D77作成のダイアログを独立化
 *	  2002.05.06		起動モード切り換え時にFM-7モードだけはリセットされない
 *						ように変更
 *	  2002.05.07		論理演算/直線補間ウィンドウ用処理を追加
 *	  2002.05.23		論理演算/直線補間ウィンドウにチェックが入らない問題を
 *						修正
 *	  2002.06.15		ステートファイルの動作機種情報が反映されない問題を修正
 *	  2002.07.10		サブウィンドウを全て隠す/復元する機能を追加
 *	  2002.09.13		逆アセンブルウィンドウのPC非同期モード時に「最新の情報
 *						に更新」を選択すると表示アドレスが変わる問題を修正
 *	  2002.10.21		トレース時は強制PC同期になるように変更
 *	  2003.03.09		ファイル選択デフォルトディレクトリの種類別保存に対応
 *	  2004.05.03		フルスクリーン時に自動でサブウィンドウを隠すように変更
 *						フルスクリーン時にサブウィンドウ関連メニューを無効化す
 *						るように変更
 *	  2004.05.04		ファイル選択ダイアログの初期位置が常にウィンドウ中央に
 *						なるように変更
 *	  2004.05.28		ステータスバーのデバイス別ファイルドロップに対応
 *	  2004.10.05		ディスクイメージ名に"&"が含まれているとメニュー生成時
 *						にプレフィックスとして処理されてしまう問題を修正
 *		(中略)
 *	  2012.12.04		V2/V3においてBASIC/DOSモードの切り替えの際のリセット
 *						処理を廃止
 */

#ifdef _WIN32

#define STRICT
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <commdlg.h>
#include <shellapi.h>
#include <stdlib.h>
#include <assert.h>
#include <tchar.h>
#define DIRECTINPUT_VERSION		0x0300		/* DirectX3を指定 */
#include <dinput.h>
#include <direct.h>
#include "xm7.h"
#include "fdc.h"
#include "tapelp.h"
#include "tools.h"
#include "mouse.h"
#include "rtc.h"
#include "display.h"
#include "subctrl.h"
#include "jsubsys.h"
#include "bubble.h"
#include "w32.h"
#include "w32_bar.h"
#include "w32_draw.h"
#include "w32_snd.h"
#include "w32_sch.h"
#include "w32_sub.h"
#include "w32_cfg.h"
#include "w32_res.h"
#include "w32_kbd.h"
#include "w32_comm.h"

/*
 *	グローバル ワーク
 */
#if XM7_VER == 1
#if defined(BUBBLE)
char InitialDir[6][_MAX_DRIVE + _MAX_PATH];
#else
char InitialDir[5][_MAX_DRIVE + _MAX_PATH];
#endif
#else
char InitialDir[5][_MAX_DRIVE + _MAX_PATH];
#endif
BOOL bOFNCentering = TRUE;
#if defined(KBDPASTE)
HWND hKeyStrokeDialog;			/* キー入力支援ダイアログハンドル */
BOOL bKeyStrokeModeless;		/* キー入力支援ダイアログモードレスフラグ */
#endif

/*
 *	スタティック ワーク
 */
static char StatePath[_MAX_PATH];
static char DiskTitle[16 + 1];
static BOOL DiskMedia;
static BOOL DiskFormat;
#if XM7_VER == 1 
#if defined(BUBBLE)
static char BubbleTitle[16 + 1];
static BOOL BubbleFormat;
#endif
#endif
#if defined(KBDPASTE)
static char KeyStrokeString[256];
static BOOL bImmStatus;
static DWORD dwConversion;
static DWORD dwSentence;
#endif

/*
 *	プロトタイプ宣言
 */
extern int _getdrive(void);

/*
 *	ENTER/ESC/SPACE解放待ちマクロ
 */
#define WAIT_KEY_POP()		while (	(GetAsyncKeyState(VK_RETURN) & 0x8000) || \
									(GetAsyncKeyState(VK_ESCAPE) & 0x8000) || \
									(GetAsyncKeyState(VK_SPACE) & 0x8000))

/*-[ タイトル入力ダイアログ ]------------------------------------------------*/

/*
 *	タイトル入力ダイアログ
 *	ダイアログ初期化
 */
static BOOL FASTCALL TitleDlgInit(HWND hDlg)
{
	HWND hWnd;
	RECT prect;
	RECT drect;

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

	/* エディットテキスト処理 */
	hWnd = GetDlgItem(hDlg, IDC_TITLEEDIT);
	ASSERT(hWnd);
	strncpy(DiskTitle, "Default", sizeof(DiskTitle));
	SetWindowText(hWnd, DiskTitle);

#if XM7_VER >= 3
	/* メディアタイプ */
	CheckDlgButton(hDlg, IDC_TITLE2D, BST_CHECKED);
#endif

	return TRUE;
}

/*
 *	タイトル入力ダイアログ
 *	ダイアログOK
 */
static void FASTCALL TitleDlgOK(HWND hDlg)
{
	HWND hWnd;
	char string[128];

	ASSERT(hDlg);

	/* エディットテキスト処理 */
	hWnd = GetDlgItem(hDlg, IDC_TITLEEDIT);
	ASSERT(hWnd);

	/* 文字列を取得、コピー */
	GetWindowText(hWnd, string, sizeof(string) - 1);
	memset(DiskTitle, 0, sizeof(DiskTitle));
	string[16] = '\0';
	strncpy(DiskTitle, string, sizeof(DiskTitle));

	/* メディアタイプ取得 */
#if XM7_VER >= 3
	if (IsDlgButtonChecked(hDlg, IDC_TITLE2DD)) {
		DiskMedia = TRUE;
	}
	else {
		DiskMedia = FALSE;
	}
#else
	/* V2では2Dのみ */
	DiskMedia = FALSE;
#endif

	if (IsDlgButtonChecked(hDlg, IDC_TITLEUSRDISK)) {
		DiskFormat = TRUE;
	}
	else {
		DiskFormat = FALSE;
	}
}

/*
 *	タイトル入力ダイアログ
 *	ダイアログプロシージャ
 */
static BOOL CALLBACK TitleDlgProc(HWND hDlg, UINT iMsg,
									WPARAM wParam, LPARAM lParam)
{
	UNUSED(lParam);

	switch (iMsg) {
		/* ダイアログ初期化 */
		case WM_INITDIALOG:
			return TitleDlgInit(hDlg);

		/* コマンド処理 */
		case WM_COMMAND:
			switch (LOWORD(wParam)) {
				/* OK・キャンセル */
				case IDOK:
				case IDCANCEL:
					if (LOWORD(wParam) == IDOK) {
						TitleDlgOK(hDlg);
					}
					EndDialog(hDlg, LOWORD(wParam));
					InvalidateRect(hDrawWnd, NULL, FALSE);
					SetMenuExitTimer();
					return TRUE;
			}
			break;
	}

	/* それ以外は、FALSE */
	return FALSE;
}

/*-[ タイトル入力ダイアログ(2D/VFD変換用) ]----------------------------------*/

/*
 *	タイトル入力ダイアログ(2D/VFD変換用)
 *	ダイアログ初期化
 */
static BOOL FASTCALL TitleDlg2DInit(HWND hDlg)
{
	HWND hWnd;
	RECT prect;
	RECT drect;

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

	/* エディットテキスト処理 */
	hWnd = GetDlgItem(hDlg, IDC_TITLEEDIT2D);
	ASSERT(hWnd);
	strncpy(DiskTitle, "Default", sizeof(DiskTitle));
	SetWindowText(hWnd, DiskTitle);

	return TRUE;
}

/*
 *	タイトル入力ダイアログ(2D/VFD変換用)
 *	ダイアログOK
 */
static void FASTCALL TitleDlg2DOK(HWND hDlg)
{
	HWND hWnd;
	char string[128];

	ASSERT(hDlg);

	/* エディットテキスト処理 */
	hWnd = GetDlgItem(hDlg, IDC_TITLEEDIT2D);
	ASSERT(hWnd);

	/* 文字列を取得、コピー */
	GetWindowText(hWnd, string, sizeof(string) - 1);
	memset(DiskTitle, 0, sizeof(DiskTitle));
	string[16] = '\0';
	strncpy(DiskTitle, string, sizeof(DiskTitle));
}

/*
 *	タイトル入力ダイアログ(2D/VFD変換用)
 *	ダイアログプロシージャ
 */
static BOOL CALLBACK TitleDlg2DProc(HWND hDlg, UINT iMsg,
									WPARAM wParam, LPARAM lParam)
{
	UNUSED(lParam);

	switch (iMsg) {
		/* ダイアログ初期化 */
		case WM_INITDIALOG:
			return TitleDlg2DInit(hDlg);

		/* コマンド処理 */
		case WM_COMMAND:
			switch (LOWORD(wParam)) {
				/* OK・キャンセル */
				case IDOK:
				case IDCANCEL:
					if (LOWORD(wParam) == IDOK) {
						TitleDlg2DOK(hDlg);
					}
					EndDialog(hDlg, LOWORD(wParam));
					InvalidateRect(hDrawWnd, NULL, FALSE);
					SetMenuExitTimer();
					return TRUE;
			}
			break;
	}

	/* それ以外は、FALSE */
	return FALSE;
}

/*-[ バブル変換タイプ選択ダイアログ ]----------------------------------------*/

#if XM7_VER == 1 
#if defined(BUBBLE)
/*
 *	バブル変換タイプ選択ダイアログ
 *	ダイアログ初期化
 */
static BOOL FASTCALL BubbleMediaTypeDlgInit(HWND hDlg)
{
	HWND hWnd;
	RECT prect;
	RECT drect;

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

	/* エディットテキスト処理 */
	hWnd = GetDlgItem(hDlg, IDC_TITLEEDIT);
	ASSERT(hWnd);
	SetWindowText(hWnd, BubbleTitle);

	/* フォーマットタイプ */
	CheckDlgButton(hDlg, IDC_MEDIAFORMATB77, BST_CHECKED);

	return TRUE;
}

/*
 *	バブル変換タイプ選択ダイアログ
 *	ダイアログOK
 */
static void FASTCALL BubbleMediaTypeDlgOK(HWND hDlg)
{
	ASSERT(hDlg);

	if (IsDlgButtonChecked(hDlg, IDC_MEDIAFORMATBBL)) {
		BubbleFormat = FALSE;
	}
	else {
		BubbleFormat = TRUE;
	}
}

/*
 *	バブル変換タイプ選択ダイアログ
 *	ダイアログプロシージャ
 */
static BOOL CALLBACK BubbleMediaTypeDlgProc(HWND hDlg, UINT iMsg,
									WPARAM wParam, LPARAM lParam)
{
	UNUSED(lParam);

	switch (iMsg) {
		/* ダイアログ初期化 */
		case WM_INITDIALOG:
			return BubbleMediaTypeDlgInit(hDlg);

		/* コマンド処理 */
		case WM_COMMAND:
			switch (LOWORD(wParam)) {
				/* OK・キャンセル */
				case IDOK:
				case IDCANCEL:
					if (LOWORD(wParam) == IDOK) {
						BubbleMediaTypeDlgOK(hDlg);
					}
					EndDialog(hDlg, LOWORD(wParam));
					InvalidateRect(hDrawWnd, NULL, FALSE);
					SetMenuExitTimer();
					return TRUE;
				case IDC_MEDIAFORMATB77:
					EnableWindow(GetDlgItem(hDlg, IDC_TITLEEDIT), TRUE);
					return TRUE;
				case IDC_MEDIAFORMATBBL:
					EnableWindow(GetDlgItem(hDlg, IDC_TITLEEDIT), FALSE);
					return TRUE;
			}
			break;
	}

	/* それ以外は、FALSE */
	return FALSE;
}
#endif	/* defined(BUBBLE) */
#endif	/* XM7_VER == 1 */

/*-[ ファイル選択コモンダイアログ ]-----------------------------------------*/

/*
 *	ファイル選択コモンダイアログ
 *	初期化
 */
static void FASTCALL FileDialogInit(HWND hDlg)
{
	RECT drect;
	RECT prect;
	HWND hWnd;

	/* ダイアログをメインウインドウの中央に寄せる */
	hWnd = GetParent(hDlg);
	GetWindowRect(hMainWnd, &prect);
	GetWindowRect(hWnd, &drect);
	drect.right -= drect.left;
	drect.bottom -= drect.top;
	drect.left = (prect.right - prect.left) / 2 + prect.left;
	drect.left -= (drect.right / 2);
	drect.top = (prect.bottom - prect.top) / 2 + prect.top;
	drect.top -= (drect.bottom / 2);
	MoveWindow(hWnd, drect.left, drect.top, drect.right, drect.bottom, FALSE);
}

/*
 *	ファイル選択コモンダイアログ
 *	HOOK関数
 */
static UINT CALLBACK FileSelectHook(
						HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	UNUSED(wParam);
	UNUSED(lParam);

	switch (uMsg) {
		case WM_INITDIALOG:
			FileDialogInit(hDlg);
			return 0;
	}

	return 0;
}

/*
 *	ファイル選択コモンダイアログ
 */
BOOL FASTCALL FileSelectSub(BOOL bOpen, UINT uFilterID, char *path, char *defext, BYTE IniDirNo)
{
	OPENFILENAME ofn;
	char dir[_MAX_DIR];
	char fname[_MAX_FNAME + _MAX_EXT];
	char filter[4096];
	int i, j;

	ASSERT((bOpen == TRUE) || (bOpen == FALSE));
	ASSERT(uFilterID > 0);
	ASSERT(path);

	/* データ作成 */
	memset(&ofn, 0, sizeof(ofn));
	memset(path, 0, _MAX_PATH);
	memset(fname, 0, sizeof(fname));
	ofn.lStructSize = 76;	/* sizeof(ofn)はV5拡張を含む */
	ofn.hwndOwner = hMainWnd;

	LoadString(hAppInstance, uFilterID, filter, sizeof(filter));
	j = strlen(filter);
	for (i=0; i<j; i++) {
		if (filter[i] == '|') {
			filter[i] = '\0';
		}
	}

	ofn.lpstrFilter = filter;
	ofn.lpstrFile = path;
	ofn.nMaxFile = _MAX_PATH;
	ofn.lpstrFileTitle = fname;
	ofn.nMaxFileTitle = sizeof(fname);
	ofn.lpstrDefExt = defext;
	ofn.lpstrInitialDir = InitialDir[IniDirNo];
	ofn.lpfnHook = FileSelectHook;

	/* コモンダイアログ実行 */
	if (bOpen) {
		ofn.Flags = OFN_EXPLORER | OFN_HIDEREADONLY |
					OFN_FILEMUSTEXIST;
		if (bOFNCentering) {
			ofn.Flags |= OFN_ENABLEHOOK;
		}
		if (!GetOpenFileName(&ofn)) {
			SetMenuExitTimer();
			return FALSE;
		}
	}
	else {
		ofn.Flags = OFN_EXPLORER | OFN_HIDEREADONLY |
					OFN_OVERWRITEPROMPT;
		if (bOFNCentering) {
			ofn.Flags |= OFN_ENABLEHOOK;
		}
		if (!GetSaveFileName(&ofn)) {
			SetMenuExitTimer();
			return FALSE;
		}
	}

	/* ディレクトリを保存 */
	_splitpath(path, InitialDir[IniDirNo], dir, NULL, NULL);
	if (dir[strlen(dir)-1] == '\\') {
		/* 最後のパス区切り記号は強制的に削る */
		dir[strlen(dir)-1] = '\0';
	}
	strncat(InitialDir[IniDirNo], dir,
			sizeof(InitialDir[IniDirNo]) - strlen(InitialDir[IniDirNo]) - 1);

	SetMenuExitTimer();
	return TRUE;
}

/*-[ キー入力支援ダイアログ ]-----------------------------------------------*/

#if defined(KBDPASTE)
/*
 *	キー入力支援ダイアログ
 *	ダイアログ初期化
 */
static BOOL FASTCALL KeyStrokeDlgInit(HWND hDlg)
{
	HWND hWnd;
	RECT prect;
	RECT drect;

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

	/* 初期化 */
	memset(KeyStrokeString, 0, sizeof(KeyStrokeString));

	return TRUE;
}

/*
 *	キー入力支援ダイアログ
 *	ダイアログOK
 */
static void FASTCALL KeyStrokeDlgOK(HWND hDlg)
{
	HWND hWnd;

	ASSERT(hDlg);

	/* エディットテキスト処理 */
	hWnd = GetDlgItem(hDlg, IDC_KEYSTROKEEDIT);
	ASSERT(hWnd);

	/* 文字列を取得、コピー */
	GetWindowText(hWnd, (LPTSTR)KeyStrokeString, sizeof(KeyStrokeString) - 1);
	SetWindowText(hWnd, (LPTSTR)"");
}

/*
 *	キー入力支援ダイアログ
 *	ダイアログプロシージャ
 */
static BOOL CALLBACK KeyStrokeDlgProc(HWND hDlg, UINT iMsg,
									WPARAM wParam, LPARAM lParam)
{
	HWND hWnd;
	HIMC hImm;

	UNUSED(lParam);

	switch (iMsg) {
		/* ダイアログ初期化 */
		case WM_INITDIALOG:
			/* エディットテキスト処理 */
			hWnd = GetDlgItem(hDlg, IDC_KEYSTROKEEDIT);
			ASSERT(hWnd);

		/* IMM設定を半角カナに設定 */
			hImm = ImmGetContext(hWnd);
			bImmStatus = ImmGetOpenStatus(hImm);
			ImmGetConversionStatus(hImm, &dwConversion, &dwSentence);
			ImmSetOpenStatus(hImm, TRUE);
			ImmSetConversionStatus(hImm,
					IME_CMODE_NATIVE | IME_CMODE_KATAKANA |
					IME_CMODE_ROMAN, IME_SMODE_NONE);
			ImmReleaseContext(hWnd, hImm);		/* 使用後は解放する */

			return KeyStrokeDlgInit(hDlg);

		/* コマンド処理 */
		case WM_COMMAND:
			switch (LOWORD(wParam)) {
				/* OK・キャンセル */
				case IDOK:
				case IDCANCEL:
					WAIT_KEY_POP();
					if (LOWORD(wParam) == IDOK) {
						KeyStrokeDlgOK(hDlg);
						if (strlen(KeyStrokeString) == 0) {
							break;
						}
						if (hKeyStrokeDialog) {
							/* 作成 */
							LockVM();
							PasteKbd((char *)KeyStrokeString);
							UnlockVM();
							SetForegroundWindow(hMainWnd);
							break;
						}
					}

					/* エディットテキスト処理 */
					hWnd = GetDlgItem(hDlg, IDC_KEYSTROKEEDIT);
					ASSERT(hWnd);

					/* IMM設定の復帰 */
					hImm = ImmGetContext(hWnd);
					ImmSetOpenStatus(hImm, bImmStatus);
					ImmSetConversionStatus(hImm, dwConversion, dwSentence);
					ImmReleaseContext(hWnd, hImm);

					if (hKeyStrokeDialog) {
						DestroyWindow(hKeyStrokeDialog);
						hKeyStrokeDialog = NULL;
					}
					else {
						EndDialog(hDlg, LOWORD(wParam));
					}

					InvalidateRect(hDrawWnd, NULL, FALSE);
					SetMenuExitTimer();
					return TRUE;
			}
			break;
	}

	/* それ以外は、FALSE */
	return FALSE;
}
#endif

/*-[ 汎用サブ ]-------------------------------------------------------------*/

/*
 *	メニューEnable
 */
void FASTCALL EnableMenuSub(HMENU hMenu, UINT uID, BOOL bEnable)
{
	ASSERT(hMenu);
	ASSERT(uID > 0);

	if (bEnable) {
		EnableMenuItem(hMenu, uID, MF_BYCOMMAND | MF_ENABLED);
	}
	else {
		EnableMenuItem(hMenu, uID, MF_BYCOMMAND | MF_GRAYED);
	}
}

/*
 *	メニューEnable (位置指定)
 */
#if XM7_VER == 1
#if defined(BUBBLE)
void FASTCALL EnableMenuPos(HMENU hMenu, UINT uPos, BOOL bEnable)
{
	ASSERT(hMenu);
	ASSERT(uPos > 0);

	if (bEnable) {
		EnableMenuItem(hMenu, uPos, MF_BYPOSITION | MF_ENABLED);
	}
	else {
		EnableMenuItem(hMenu, uPos, MF_BYPOSITION | MF_GRAYED);
	}
}
#endif
#endif

/*
 *	メニューCheck
 */
void FASTCALL CheckMenuSub(HMENU hMenu, UINT uID, BOOL bCheck)
{
	ASSERT(hMenu);
	ASSERT(uID > 0);

	if (bCheck) {
		CheckMenuItem(hMenu, uID, MF_BYCOMMAND | MF_CHECKED);
	}
	else {
		CheckMenuItem(hMenu, uID, MF_BYCOMMAND | MF_UNCHECKED);
	}
}

/*-[ ファイルメニュー ]-----------------------------------------------------*/

/*
 *	ステートロード処理
 */
static void FASTCALL StateLoad(char *path)
{
	char string[256];
	int state;

	state = system_load(path);
	if (state == STATELOAD_ERROR) {
		/* 本体読み込み中のエラー発生時のみリセット */
		system_reset();
		OnRefresh(hMainWnd);
	}
	if (state != STATELOAD_SUCCESS) {
		LoadString(hAppInstance, IDS_STATEERROR, string, sizeof(string));
		MessageBox(hMainWnd, string, "XM7", MB_ICONSTOP | MB_OK);
		SetMenuExitTimer();
	}
	else {
		strncpy(StatePath, path, sizeof(StatePath));
		OnRefresh(hMainWnd);
	}

	/* 成功・失敗どちらのケースでもターゲット機種を変更する */
	GetCfg();
	SetMachineVersion();
}

/*-[ ファイルメニュー ]-----------------------------------------------------*/

/*
 *	開く(O)
 */
static void FASTCALL OnOpen(HWND hWnd)
{
	char path[_MAX_PATH];

	ASSERT(hWnd);

	/* ファイル選択サブ */
	if (!FileSelectSub(TRUE, IDS_STATEFILTER, path, NULL, 2)) {
		return;
	}

	/* ステートロード */
	LockVM();
	StopSnd();
	StateLoad(path);
	PlaySnd();
	ResetSch();
	UnlockVM();

	/* 画面再描画 */
	OnRefresh(hWnd);
}

/*
 *	名前を付けて保存(A)
 */
static void FASTCALL OnSaveAs(HWND hWnd)
{
	char path[_MAX_PATH];

	ASSERT(hWnd);

	/* ファイル選択サブ */
	if (!FileSelectSub(FALSE, IDS_STATEFILTER, path, "XM7", 2)) {
		return;
	}

	/* ステートセーブ */
	LockVM();
	StopSnd();
	if (!system_save(path)) {
		LoadString(hAppInstance, IDS_STATEERROR, path, sizeof(path));
		MessageBox(hWnd, path, "XM7", MB_ICONSTOP | MB_OK);
		SetMenuExitTimer();
	}
	else {
		strncpy(StatePath, path, sizeof(StatePath));
	}
	PlaySnd();
	ResetSch();
	UnlockVM();
}

/*
 *	上書き保存(S)
 */
static void FASTCALL OnSave(HWND hWnd)
{
	char string[128];

	/* まだ保存されていなければ、名前をつける */
	if (StatePath[0] == '\0') {
		OnSaveAs(hWnd);
		return;
	}

	/* ステートセーブ */
	LockVM();
	StopSnd();
	if (!system_save(StatePath)) {
		LoadString(hAppInstance, IDS_STATEERROR, string, sizeof(string));
		MessageBox(hWnd, string, "XM7", MB_ICONSTOP | MB_OK);
		SetMenuExitTimer();
	}
	PlaySnd();
	ResetSch();
	UnlockVM();
}

#if defined(LPRINT)
/*
 *	印刷(P)
 */
static void FASTCALL OnPrint(void)
{
	lp_print();
}
#endif

/*
 *	リセット(R)
 */
static void FASTCALL OnReset(HWND hWnd)
{
	LockVM();
	system_reset();
	ResetSch();
	UnlockVM();

	/* 再描画 */
	OnRefresh(hWnd);
}

/*
 *	ホットリセット(H)
 */
static void FASTCALL OnHotReset(HWND hWnd)
{
	LockVM();
	system_hotreset();
	UnlockVM();

	/* 再描画 */
	OnRefresh(hWnd);
}

/*
 *	TAB+リセット(F)
 */
#if XM7_VER >= 3
static void FASTCALL OnTabReset(HWND hWnd)
{
	LockVM();
	system_tabreset();
	UnlockVM();

	/* 再描画 */
	OnRefresh(hWnd);
}
#endif

/*
 *	BASICモード(B)
 */
static void FASTCALL OnBasic(void)
{
	LockVM();
	boot_mode = BOOT_BASIC;
	GetCfg();
#if XM7_VER >= 2
	if (fm7_ver < 2) {
		mainmem_transfer_boot();
	}
#else
	if (fm_subtype == FMSUB_FM8) {
		basicrom_en = TRUE;
	}
#endif
	UnlockVM();
}

/*
 *	DOSモード(D)
 */
static void FASTCALL OnDos(void)
{
	LockVM();
	boot_mode = BOOT_DOS;
	GetCfg();
#if XM7_VER >= 2
	if (fm7_ver < 2) {
		mainmem_transfer_boot();
	}
#else
	if (fm_subtype == FMSUB_FM8) {
		basicrom_en = FALSE;
	}
#endif
	UnlockVM();
}

/*
 *	バブルモード(U)
 */
#if XM7_VER == 1 
#if defined(BUBBLE)
static void FASTCALL OnBubble(void)
{
	LockVM();
	boot_mode = BOOT_BUBBLE;
	GetCfg();
	if (fm_subtype == FMSUB_FM8) {
		basicrom_en = FALSE;
	}
	UnlockVM();
}
#endif
#endif

/*
 *	終了(X)
 */
static void FASTCALL OnExit(HWND hWnd)
{
	/* ウインドウクローズ */
	PostMessage(hWnd, WM_CLOSE, 0, 0);
}

/*
 *	ファイル(F)メニュー
 */
static BOOL OnFile(HWND hWnd, WORD wID)
{
	ASSERT(hWnd);

	switch (wID) {
		/* オープン */
		case IDM_OPEN:
			OnOpen(hWnd);
			return TRUE;

		/* 上書き保存 */
		case IDM_SAVE:
			OnSave(hWnd);
			return TRUE;

		/* 名前をつけて保存 */
		case IDM_SAVEAS:
			OnSaveAs(hWnd);
			return TRUE;

#if defined(LPRINT)
		/* 印刷 */
		case IDM_PRINT:
			OnPrint();
			return TRUE;
#endif

		/* リセット */
		case IDM_RESET:
			OnReset(hWnd);
			return TRUE;

		/* ホットリセット */
		case IDM_HOTRESET:
			OnHotReset(hWnd);
			return TRUE;

#if XM7_VER >= 3
		/* TAB+リセット */
		case IDM_TABRESET:
			OnTabReset(hWnd);
			return TRUE;
#endif

		/* BASICモード */
		case IDM_BASIC:
			OnBasic();
			return TRUE;

		/* DOSモード */
		case IDM_DOS:
			OnDos();
			return TRUE;

#if XM7_VER == 1 
#if defined(BUBBLE)
		/* バブルモード */
		case IDM_BUBBLE:
			OnBubble();
			return TRUE;
#endif
#endif

		/* 終了 */
		case IDM_EXIT:
			OnExit(hWnd);
			return TRUE;
	}

	return FALSE;
}

/*
 *	ファイル(F)メニュー更新
 */
static void FASTCALL OnFilePopup(HMENU hMenu)
{
	UINT id;

	ASSERT(hMenu);

	switch (boot_mode) {
		case BOOT_BASIC:
			id = IDM_BASIC;
			break;
		case BOOT_DOS:
			id = IDM_DOS;
			break;
#if XM7_VER == 1 
#if defined(BUBBLE)
		case BOOT_BUBBLE:
			id = IDM_BUBBLE;
			break;
#endif
#endif
		default:
			ASSERT(FALSE);
			break;
	}

#if XM7_VER == 1
#if defined(BUBBLE)
	CheckMenuRadioItem(hMenu, IDM_BASIC, IDM_BUBBLE, id, MF_BYCOMMAND);
	EnableMenuSub(hMenu, IDM_BUBBLE, (fm_subtype == FMSUB_FM8) && bubble_available);
#else
	CheckMenuRadioItem(hMenu, IDM_BASIC, IDM_DOS, id, MF_BYCOMMAND);
#endif
#else
	CheckMenuRadioItem(hMenu, IDM_BASIC, IDM_DOS, id, MF_BYCOMMAND);
#endif

#if XM7_VER >= 3
	EnableMenuSub(hMenu, IDM_TABRESET, (fm7_ver >= 3) && init_is_exsx);
#endif

#if defined(LPRINT)
	EnableMenuSub(hMenu, IDM_PRINT, (lp_use == LP_EMULATION));
#endif
}

/*-[ ディスクメニュー ]-----------------------------------------------------*/

/*
 *	ドライブを開く
 */
static void FASTCALL OnDiskOpen(int Drive)
{
	char path[_MAX_PATH];

	ASSERT((Drive == 0) || (Drive == 1));

	/* ファイル選択 */
	if (!FileSelectSub(TRUE, IDS_DISKFILTER, path, NULL, 0)) {
		return;
	}

	/* セット */
	LockVM();
	fdc_setdisk(Drive, path);
	ResetSch();
	UnlockVM();
}

/*
 *	両ドライブを開く
 */
static void FASTCALL OnDiskBoth(void)
{
	char path[_MAX_PATH];

	/* ファイル選択 */
	if (!FileSelectSub(TRUE, IDS_DISKFILTER, path, NULL, 0)) {
		return;
	}

	/* セット */
	LockVM();
	fdc_setdisk(0, path);
	fdc_setdisk(1, NULL);
	if ((fdc_ready[0] != FDC_TYPE_NOTREADY) && (fdc_medias[0] >= 2)) {
		fdc_setdisk(1, path);
		fdc_setmedia(1, 1);
	}
	ResetSch();
	UnlockVM();
}

/*
 *	ディスクイジェクト
 */
static void FASTCALL OnDiskEject(int Drive)
{
	ASSERT((Drive == 0) || (Drive == 1));

	/* イジェクト */
	LockVM();
	fdc_setdisk(Drive, NULL);
	UnlockVM();
}

/*
 *	ディスク一時取り出し
 */
static void FASTCALL OnDiskTemp(int Drive)
{
	ASSERT((Drive == 0) || (Drive == 1));

	/* 書き込み禁止切り替え */
	LockVM();
	if (fdc_teject[Drive]) {
		fdc_teject[Drive] = FALSE;
	}
	else {
		fdc_teject[Drive] = TRUE;
	}
	UnlockVM();
}

/*
 *	ディスク書き込み禁止
 */
static void FASTCALL OnDiskProtect(int Drive)
{
	ASSERT((Drive == 0) || (Drive == 1));

	/* 書き込み禁止切り替え */
	LockVM();
	if (fdc_writep[Drive]) {
		fdc_setwritep(Drive, FALSE);
	}
	else {
		fdc_setwritep(Drive, TRUE);
	}
	ResetSch();
	UnlockVM();
}

/*
 *	ディスク(1)(0)メニュー
 */
static BOOL FASTCALL OnDisk(WORD wID)
{
	switch (wID) {
		/* 開く */
		case IDM_D0OPEN:
			OnDiskOpen(0);
			break;
		case IDM_D1OPEN:
			OnDiskOpen(1);
			break;

		/* 両ドライブで開く */
		case IDM_DBOPEN:
			OnDiskBoth();
			break;

		/* 取り外し */
		case IDM_D0EJECT:
			OnDiskEject(0);
			break;
		case IDM_D1EJECT:
			OnDiskEject(1);
			break;

		/* 一時イジェクト */
		case IDM_D0TEMP:
			OnDiskTemp(0);
			break;
		case IDM_D1TEMP:
			OnDiskTemp(1);
			break;

		/* 書き込み禁止 */
		case IDM_D0WRITE:
			OnDiskProtect(0);
			break;
		case IDM_D1WRITE:
			OnDiskProtect(1);
			break;
	}

	/* メディア交換 */
	if ((wID >= IDM_D0MEDIA00) && (wID <= IDM_D0MEDIA15)) {
		LockVM();
		fdc_setmedia(0, wID - IDM_D0MEDIA00);
		ResetSch();
		UnlockVM();
	}
	if ((wID >= IDM_D1MEDIA00) && (wID <= IDM_D1MEDIA15)) {
		LockVM();
		fdc_setmedia(1, wID - IDM_D1MEDIA00);
		ResetSch();
		UnlockVM();
	}

	return FALSE;
}

/*
 *	ディスク(1)(0)メニュー更新
 */
static void FASTCALL OnDiskPopup(HMENU hMenu, int Drive)
{
	MENUITEMINFO mii;
	char string[128];
	char buffer[128];
	int offset;
	int i;
	int j;
	int k;

	ASSERT(hMenu);
	ASSERT((Drive == 0) || (Drive == 1));

	/* メニューすべて削除 */
	while (GetMenuItemCount(hMenu) > 0) {
		DeleteMenu(hMenu, 0, MF_BYPOSITION);
	}

	/* オフセット確定 */
	if (Drive == 0) {
		offset = 0;
	}
	else {
		offset = IDM_D1OPEN - IDM_D0OPEN;
	}

	/* メニュー構造体初期化 */
	memset(&mii, 0, sizeof(mii));
	mii.cbSize = 44;	/* sizeof(mii)はWINVER>=0x0500向け */
	mii.fMask = MIIM_TYPE | MIIM_STATE | MIIM_ID;
	mii.fType = MFT_STRING;
	mii.fState = MFS_ENABLED;

	/* オープンと、両オープン */
	mii.wID = IDM_D0OPEN + offset;
	LoadString(hAppInstance, IDS_DISKOPEN, string, sizeof(string));
	mii.dwTypeData = string;
	mii.cch = strlen(string);
	InsertMenuItem(hMenu, 0, TRUE, &mii);
	mii.wID = IDM_DBOPEN;
	LoadString(hAppInstance, IDS_DISKBOTH, string, sizeof(string));
	mii.cch = strlen(string);
	InsertMenuItem(hMenu, 1, TRUE, &mii);

	/* ディスクが挿入されていなければ、ここまで */
	if (fdc_ready[Drive] == FDC_TYPE_NOTREADY) {
		return;
	}

	/* イジェクト */
	mii.wID = IDM_D0EJECT + offset;
	LoadString(hAppInstance, IDS_DISKEJECT, string, sizeof(string));
	mii.cch = strlen(string);
	InsertMenuItem(hMenu, 2, TRUE, &mii);

	/* セパレータ挿入 */
	mii.fType = MFT_SEPARATOR;
	InsertMenuItem(hMenu, 3, TRUE, &mii);

	/* 一時取り出し */
	mii.wID = IDM_D0TEMP + offset;
	LoadString(hAppInstance, IDS_DISKTEMP, string, sizeof(string));
	if (fdc_teject[Drive]) {
		mii.fState = MFS_CHECKED | MFS_ENABLED;
	}
	else {
		mii.fState = MFS_ENABLED;
	}
	mii.fType = MFT_STRING;
	mii.cch = strlen(string);
	InsertMenuItem(hMenu, 4, TRUE, &mii);

	/* ライトプロテクト */
	mii.wID = IDM_D0WRITE + offset;
	LoadString(hAppInstance, IDS_DISKPROTECT, string, sizeof(string));
	if (fdc_fwritep[Drive]) {
		mii.fState = MFS_GRAYED;
	}
	else {
		if (fdc_writep[Drive]) {
			mii.fState = MFS_CHECKED | MFS_ENABLED;
		}
		else {
			mii.fState = MFS_ENABLED;
		}
	}
	mii.fType = MFT_STRING;
	mii.cch = strlen(string);
	InsertMenuItem(hMenu, 5, TRUE, &mii);

	/* セパレータ挿入 */
	mii.fType = MFT_SEPARATOR;
	InsertMenuItem(hMenu, 6, TRUE, &mii);

	/* 2D/2DD/VFDなら特殊処理 */
	if ((fdc_ready[Drive] == FDC_TYPE_2D) ||
#if XM7_VER >= 3
		(fdc_ready[Drive] == FDC_TYPE_2DD) ||
#endif
		(fdc_ready[Drive] == FDC_TYPE_VFD)) {
		mii.wID = IDM_D0MEDIA00 + offset;
		mii.fState = MFS_CHECKED | MFS_ENABLED;
		mii.fType = MFT_STRING | MFT_RADIOCHECK;
		if (fdc_ready[Drive] == FDC_TYPE_2D) {
			LoadString(hAppInstance, IDS_DISK2D, string, sizeof(string));
		}
#if XM7_VER >= 3
		else if (fdc_ready[Drive] == FDC_TYPE_2DD) {
			LoadString(hAppInstance, IDS_DISK2DD, string, sizeof(string));
		}
#endif
		else {
			LoadString(hAppInstance, IDS_DISKVFD, string, sizeof(string));
		}
		mii.cch = strlen(string);
		InsertMenuItem(hMenu, 7, TRUE, &mii);
		return;
	}

	/* メディアを回す */
	for (i=0; i<fdc_medias[Drive]; i++) {
		mii.wID = IDM_D0MEDIA00 + offset + i;
		if (fdc_media[Drive] == i) {
			mii.fState = MFS_CHECKED | MFS_ENABLED;
		}
		else {
			mii.fState = MFS_ENABLED;
		}
		mii.fType = MFT_STRING | MFT_RADIOCHECK;
		if (strlen(fdc_name[Drive][i]) == 0) {
			LoadString(hAppInstance, IDS_MEDIA_NAME, buffer, sizeof(buffer));
			/* 128バイトを超えないとは思うのだが… */
			_snprintf(string, sizeof(string), buffer, i + 1);
		}
		else {
			/* プレフィックス処理対策 */
			k = 0;
			for (j=0; j<(int)strlen(fdc_name[Drive][i]); j++) {
				if (fdc_name[Drive][i][j] == '&') {
					string[k++] = '&';
				}
				string[k++] = fdc_name[Drive][i][j];
			}
			string[k] = '\0';
		}
		mii.cch = strlen(string);
		InsertMenuItem(hMenu, 7 + i, TRUE, &mii);
	}
}

/*-[ テープメニュー ]-------------------------------------------------------*/

/*
 *  テープオープン
 */
static void FASTCALL OnTapeOpen(void)
{
	char path[_MAX_PATH];

	/* ファイル選択 */
	if (!FileSelectSub(TRUE, IDS_TAPEFILTER, path, NULL, 1)) {
		return;
	}

	/* セット */
	LockVM();
	tape_setfile(path);
	ResetSch();
	UnlockVM();
}

/*
 *	テープイジェクト
 */
static void FASTCALL OnTapeEject(void)
{
	/* イジェクト */
	LockVM();
	tape_setfile(NULL);
	UnlockVM();
}

/*
 *	巻き戻し
 */
static void FASTCALL OnRew(void)
{
	HCURSOR hCursor;

	/* 巻き戻し */
	hCursor = SetCursor(LoadCursor(NULL, IDC_WAIT));
	LockVM();
	StopSnd();

	tape_rew();

	PlaySnd();
	ResetSch();
	UnlockVM();
	SetCursor(hCursor);
}

/*
 *	最初まで巻き戻し
 */
static void FASTCALL OnRewTop(void)
{
	HCURSOR hCursor;

	LockVM();
	StopSnd();

	tape_rewtop();

	PlaySnd();
	ResetSch();
	UnlockVM();
}

/*
 *	早送り
 */
static void FASTCALL OnFF(void)
{
	HCURSOR hCursor;

	/* 巻き戻し */
	hCursor = SetCursor(LoadCursor(NULL, IDC_WAIT));
	LockVM();
	StopSnd();

	tape_ff();

	PlaySnd();
	ResetSch();
	UnlockVM();
	SetCursor(hCursor);
}

/*
 *	録音
 */
static void FASTCALL OnRec(void)
{
	/* 録音 */
	LockVM();
	if (tape_rec) {
		tape_setrec(FALSE);
	}
	else {
		tape_setrec(TRUE);
	}
	UnlockVM();
}

#if XM7_VER == 1
#if defined(BUBBLE)
/*
 *  バブルオープン
 */
static void FASTCALL OnBubbleOpen(int unit)
{
	char path[_MAX_PATH];

	ASSERT ((unit == 0) || (unit == 1));

	/* ファイル選択 */
	if (!FileSelectSub(TRUE, IDS_BUBBLEFILTER, path, NULL, 5)) {
		return;
	}

	/* セット */
	LockVM();
	bmc_setfile(unit, path);
	bmc_setmedia(unit, 0);
	ResetSch();
	UnlockVM();
}


/*
 *  両ユニットで開く
 */
static void FASTCALL OnBubbleBoth(int unit)
{
	char path[_MAX_PATH];

	ASSERT((unit == 0) || (unit == 1));

	/* ファイル選択 */
	if (!FileSelectSub(TRUE, IDS_BUBBLEFILTER, path, NULL, 5)) {
		return;
	}

	/* セット */
	LockVM();
	bmc_setfile(unit + 0, path);
	bmc_setfile(unit + 1, NULL);
	bmc_setmedia(unit + 0, 0);
	if ((bmc_ready[unit] != BMC_TYPE_NOTREADY) && (bmc_medias[unit] >= 2)) {
		bmc_setfile(unit + 1, path);
		bmc_setmedia(unit + 1, 1);
	}
	ResetSch();
	UnlockVM();
}

/*
 *	バブルイジェクト
 */
static void FASTCALL OnBubbleEject(int unit)
{
	ASSERT((unit == 0) || (unit == 1));

	/* イジェクト */
	LockVM();
	bmc_setfile(unit, NULL);
	UnlockVM();
}

/*
 *	バブル一時取り出し
 */
static void FASTCALL OnBubbleTemp(int unit)
{
	ASSERT((unit == 0) || (unit == 1));

	/* 書き込み禁止切り替え */
	LockVM();
	if (bmc_teject[unit]) {
		bmc_teject[unit] = FALSE;
	}
	else {
		bmc_teject[unit] = TRUE;
	}
	UnlockVM();
}

/*
 *	バブル書き込み禁止
 */
static void FASTCALL OnBubbleProtect(int unit)
{
	ASSERT((unit == 0) || (unit == 1));

	/* 書き込み禁止切り替え */
	LockVM();
	if (bmc_writep[unit]) {
		bmc_setwritep(unit, FALSE);
	}
	else {
		bmc_setwritep(unit, TRUE);
	}
	ResetSch();
	UnlockVM();
}
#endif
#endif

/*
 *	テープ(A)メニュー
 */
static BOOL FASTCALL OnTape(WORD wID)
{
	switch (wID) {
		/* 開く */
		case IDM_TOPEN:
			OnTapeOpen();
			return TRUE;

		/* 取り外す */
		case IDM_TEJECT:
			OnTapeEject();
			return TRUE;

		/* 巻き戻し */
		case IDM_REW:
			OnRew();
			return TRUE;

		/* 最初まで巻き戻し */
		case IDM_REWTOP:
			OnRewTop();
			return TRUE;

		/* 早送り */
		case IDM_FF:
			OnFF();
			return TRUE;

		/* 録音 */
		case IDM_REC:
			OnRec();
			return TRUE;

#if XM7_VER == 1
#if defined(BUBBLE)
		/* 開く */
		case IDM_B0OPEN:
			OnBubbleOpen(0);
			return TRUE;
		case IDM_B1OPEN:
			OnBubbleOpen(1);
			return TRUE;

		/* 両ユニットで開く */
		case IDM_BBOPEN:
			OnBubbleBoth(0);
			break;

		/* 取り外し */
		case IDM_B0EJECT:
			OnBubbleEject(0);
			return TRUE;
		case IDM_B1EJECT:
			OnBubbleEject(1);
			return TRUE;

		/* 一時イジェクト */
		case IDM_B0TEMP:
			OnBubbleTemp(0);
			return TRUE;
		case IDM_B1TEMP:
			OnBubbleTemp(1);
			return TRUE;

		/* 書き込み禁止 */
		case IDM_B0WRITE:
			OnBubbleProtect(0);
			return TRUE;
		case IDM_B1WRITE:
			OnBubbleProtect(1);
			return TRUE;
#endif
#endif
	}

#if XM7_VER == 1
#if defined(BUBBLE)
	/* メディア交換（バブルカセット） */
	if ((wID >= IDM_B0MEDIA00) && (wID <= IDM_B0MEDIA15)) {
		LockVM();
		bmc_setmedia(0, wID - IDM_B0MEDIA00);
		ResetSch();
		UnlockVM();
		return TRUE;
	}
	if ((wID >= IDM_B1MEDIA00) && (wID <= IDM_B1MEDIA15)) {
		LockVM();
		bmc_setmedia(1, wID - IDM_B1MEDIA00);
		ResetSch();
		UnlockVM();
		return TRUE;
	}
#endif
#endif

	return FALSE;
}

/*
 *	テープ(A)メニュー更新
 */
static void FASTCALL OnTapePopup(HMENU hMenu)
{
#if XM7_VER == 1
#if defined(BUBBLE)
#define	_BUBBLE
	HMENU hSubMenu;
	MENUITEMINFO mii;
	char string[128];
	char buffer[256];
	UINT offset;
	UINT uitem;
	int unit;
	int i;
	int j;
	int k;

	ASSERT(hMenu);

	/* カセットメニューの1番目のサブメニューハンドルを取得 */
	hSubMenu = GetSubMenu(hMenu, 0);

	/* メニューすべて削除 */
	while (GetMenuItemCount(hSubMenu) > 0) {
		DeleteMenu(hSubMenu, 0, MF_BYPOSITION);
	}

	/* メニュー構造体初期化 */
	memset(&mii, 0, sizeof(mii));
	mii.cbSize = 44;	/* sizeof(mii)はWINVER>=0x0500向け */
	mii.fMask = MIIM_TYPE | MIIM_STATE | MIIM_ID;
	mii.fType = MFT_STRING;
	mii.fState = MFS_ENABLED;

	/* 開く */
	mii.wID = IDM_TOPEN;
	LoadString(hAppInstance, IDS_TAPEOPEN, string, sizeof(string));
	mii.dwTypeData = string;
	mii.cch = strlen(string);
	InsertMenuItem(hSubMenu, 0, TRUE, &mii);

	/* テープがセットされていなければ、ここまで */
	if (tape_fileh != -1) {
		/* 取り出し */
		mii.wID = IDM_TEJECT;
		LoadString(hAppInstance, IDS_TAPEEJECT, string, sizeof(string));
		mii.cch = strlen(string);
		InsertMenuItem(hSubMenu, 1, TRUE, &mii);

		/* セパレータ挿入 */
		mii.fType = MFT_SEPARATOR;
		InsertMenuItem(hSubMenu, 2, TRUE, &mii);
		mii.fType = MFT_STRING;

		/* 巻き戻し */
		mii.wID = IDM_REW;
		LoadString(hAppInstance, IDS_TAPEREW, string, sizeof(string));
		mii.cch = strlen(string);
		InsertMenuItem(hSubMenu, 3, TRUE, &mii);

		/* 最初まで巻き戻し */
		mii.wID = IDM_REWTOP;
		LoadString(hAppInstance, IDS_TAPEREWTOP, string, sizeof(string));
		mii.cch = strlen(string);
		InsertMenuItem(hSubMenu, 4, TRUE, &mii);

		/* 早送り */
		mii.wID = IDM_FF;
		LoadString(hAppInstance, IDS_TAPEFF, string, sizeof(string));
		mii.cch = strlen(string);
		InsertMenuItem(hSubMenu, 5, TRUE, &mii);

		/* セパレータ挿入 */
		mii.fType = MFT_SEPARATOR;
		InsertMenuItem(hSubMenu, 6, TRUE, &mii);
		mii.fType = MFT_STRING;

		/* 録音 */
		mii.wID = IDM_REC;
		LoadString(hAppInstance, IDS_TAPEREC, string, sizeof(string));
		mii.cch = strlen(string);
		if (tape_writep) {
			mii.fState = MFS_GRAYED;
		}
		else {
			if (tape_rec) {
				mii.fState = MFS_CHECKED | MFS_ENABLED;
			}
			else {
				mii.fState = MFS_ENABLED;
			}
		}
		InsertMenuItem(hSubMenu, 7, TRUE, &mii);
	}

	/* バブルメモリ サブメニュー */
	for (unit = 0; unit < 2; unit ++) 
	{
		hSubMenu = GetSubMenu(hMenu, unit + 1);

		/* メニューすべて削除 */
		while (GetMenuItemCount(hSubMenu) > 0) {
			DeleteMenu(hSubMenu, 0, MF_BYPOSITION);
		}

		/* オフセット確定 */
		if (unit == 0) {
			offset = 0;
		}
		else {
			offset = IDM_B1OPEN - IDM_B0OPEN;
		}

		/* メニュー構造体初期化 */
		memset(&mii, 0, sizeof(mii));
		mii.cbSize = 44;	/* sizeof(mii)はWINVER>=0x0500向け */
		mii.fMask = MIIM_TYPE | MIIM_STATE | MIIM_ID;
		mii.fType = MFT_STRING;

		/* 開く */
		mii.wID = IDM_B0OPEN + offset;
		LoadString(hAppInstance, IDS_BUBBLEOPEN, string, sizeof(string));
		mii.dwTypeData = string;
		mii.fState = MFS_ENABLED;
		mii.cch = strlen(string);
		if (!bmc_enable || (fm_subtype != FMSUB_FM8)) {
			mii.fState = MFS_GRAYED;
		}
		else {
			mii.fState = MFS_ENABLED;
		}
		InsertMenuItem(hSubMenu, 0, TRUE, &mii);

		uitem = 1;

		if (bmc_enable) {
			/* 両オープン */
			mii.fMask = MIIM_TYPE | MIIM_STATE | MIIM_ID;
			mii.wID = IDM_BBOPEN;
			LoadString(hAppInstance, IDS_BBOPEN, string, sizeof(string));
			mii.cch = strlen(string);
			InsertMenuItem(hSubMenu, uitem++, TRUE, &mii);

			if (bmc_ready[unit] != BMC_TYPE_NOTREADY) {
				/* 取り出し */
				mii.wID = IDM_B0EJECT + offset;
				LoadString(hAppInstance, IDS_BUBBLEEJECT, string, sizeof(string));
				mii.cch = strlen(string);
				InsertMenuItem(hSubMenu, uitem++, TRUE, &mii);

				/* セパレータ挿入 */
				mii.fType = MFT_SEPARATOR;
				InsertMenuItem(hSubMenu, uitem++, TRUE, &mii);

				/* 一時取り出し */
				mii.wID = IDM_B0TEMP + offset;
				LoadString(hAppInstance, IDS_BUBBLETEMP, string, sizeof(string));
				if ((bmc_ready[unit] == BMC_TYPE_NOTREADY) || (fm_subtype != FMSUB_FM8)) {
					mii.fState = MFS_GRAYED;
				}
				else if (bmc_teject[unit]) {
					mii.fState = MFS_CHECKED | MFS_ENABLED;
				}
				else {
					mii.fState = MFS_ENABLED;
				}
				mii.fType = MFT_STRING;
				mii.cch = strlen(string);
				InsertMenuItem(hSubMenu, uitem++, TRUE, &mii);

				/* 書き込み禁止 */
				mii.wID = IDM_B0WRITE + offset;
				LoadString(hAppInstance, IDS_BUBBLEPROTECT, string, sizeof(string));
				if ((bmc_ready[unit] == BMC_TYPE_NOTREADY) || (fm_subtype != FMSUB_FM8)) {
					mii.fState = MFS_GRAYED;
				}
				else {
					if (bmc_fwritep[unit]) {
						mii.fState = MFS_GRAYED;
					}
					else {
						mii.fState = MFS_ENABLED;
					}
					if ((bmc_fwritep[unit]) || (bmc_writep[unit])) {
						mii.fState |= MFS_CHECKED;
					}
				}
				mii.fType = MFT_STRING;
				mii.cch = strlen(string);
				InsertMenuItem(hSubMenu, uitem++, TRUE, &mii);

				/* セパレータ挿入 */
				mii.fType = MFT_SEPARATOR;
				InsertMenuItem(hSubMenu, uitem++, TRUE, &mii);

				/* B77ならメディアを回す */
				if (bmc_ready[unit] == BMC_TYPE_B77) {
					for (i=0; i<bmc_medias[unit]; i++) {
						if (unit == 0) {
							mii.wID = IDM_B0MEDIA00 + i;
						}
						else {
							mii.wID = IDM_B1MEDIA00 + i;
						}
						mii.fState = MFS_ENABLED;
						if (bmc_media[unit] == i) {
							mii.fState |= MFS_CHECKED;
						}
						mii.fType = MFT_STRING | MFT_RADIOCHECK;
						if (strlen(bmc_name[unit][i]) == 0) {
							LoadString(hAppInstance, IDS_MEDIA_NAME, buffer, sizeof(buffer));
							_snprintf(string, sizeof(string), buffer, i + 1);
						}
						else {
							/* プレフィックス処理対策 */
							k = 0;
							for (j=0; j<(int)strlen(bmc_name[unit][i]); j++) {
								if (bmc_name[unit][i][j] == '&') {
									string[k++] = '&';
								}
								string[k++] = bmc_name[unit][i][j];
							}
							string[k] = '\0';
						}
						mii.cch = strlen(string);
						InsertMenuItem(hSubMenu, uitem++, TRUE, &mii);
					}
				}
				else {
					/* BBLファイルならダミー項目発生 */
					if (unit == 0) {
						mii.wID = IDM_B0MEDIA00;
					}
					else {
						mii.wID = IDM_B1MEDIA00;
					}
					mii.fState = MFS_ENABLED | MFS_CHECKED;
					mii.fType = MFT_STRING | MFT_RADIOCHECK;
					LoadString(hAppInstance, IDS_BBLFILE, string, sizeof(string));
					mii.cch = strlen(string);
					InsertMenuItem(hSubMenu, uitem++, TRUE, &mii);
				}
			}
		}
	}

	EnableMenuPos(hMenu, 1, (fm_subtype == FMSUB_FM8));
	EnableMenuPos(hMenu, 2, (fm_subtype == FMSUB_FM8));
#endif
#endif

#if !defined(_BUBBLE)
	MENUITEMINFO mii;
	char string[128];

	ASSERT(hMenu);

	/* メニューすべて削除 */
	while (GetMenuItemCount(hMenu) > 0) {
		DeleteMenu(hMenu, 0, MF_BYPOSITION);
	}

	/* メニュー構造体初期化 */
	memset(&mii, 0, sizeof(mii));
	mii.cbSize = 44;	/* sizeof(mii)はWINVER>=0x0500向け */
	mii.fMask = MIIM_TYPE | MIIM_STATE | MIIM_ID;
	mii.fType = MFT_STRING;
	mii.fState = MFS_ENABLED;

	/* オープン */
	mii.wID = IDM_TOPEN;
	LoadString(hAppInstance, IDS_TAPEOPEN, string, sizeof(string));
	mii.dwTypeData = string;
	mii.cch = strlen(string);
	InsertMenuItem(hMenu, 0, TRUE, &mii);

	/* テープがセットされていなければ、ここまで */
	if (tape_fileh == -1) {
		return;
	}

	/* イジェクト */
	mii.wID = IDM_TEJECT;
	LoadString(hAppInstance, IDS_TAPEEJECT, string, sizeof(string));
	mii.cch = strlen(string);
	InsertMenuItem(hMenu, 1, TRUE, &mii);

	/* セパレータ挿入 */
	mii.fType = MFT_SEPARATOR;
	InsertMenuItem(hMenu, 2, TRUE, &mii);
	mii.fType = MFT_STRING;

	/* 巻き戻し */
	mii.wID = IDM_REW;
	LoadString(hAppInstance, IDS_TAPEREW, string, sizeof(string));
	mii.cch = strlen(string);
	InsertMenuItem(hMenu, 3, TRUE, &mii);

	/* 最初まで巻き戻し */
	mii.wID = IDM_REWTOP;
	LoadString(hAppInstance, IDS_TAPEREWTOP, string, sizeof(string));
	mii.cch = strlen(string);
	InsertMenuItem(hMenu, 4, TRUE, &mii);

	/* 早送り */
	mii.wID = IDM_FF;
	LoadString(hAppInstance, IDS_TAPEFF, string, sizeof(string));
	mii.cch = strlen(string);
	InsertMenuItem(hMenu, 5, TRUE, &mii);

	/* セパレータ挿入 */
	mii.fType = MFT_SEPARATOR;
	InsertMenuItem(hMenu, 6, TRUE, &mii);
	mii.fType = MFT_STRING;

	/* 録音 */
	mii.wID = IDM_REC;
	LoadString(hAppInstance, IDS_TAPEREC, string, sizeof(string));
	mii.cch = strlen(string);
	if (tape_writep) {
		mii.fState = MFS_GRAYED;
	}
	else {
		if (tape_rec) {
			mii.fState = MFS_CHECKED | MFS_ENABLED;
		}
		else {
			mii.fState = MFS_ENABLED;
		}
	}
	InsertMenuItem(hMenu, 7, TRUE, &mii);
#endif

#undef _BUBBLE
}

/*-[ 表示メニュー ]---------------------------------------------------------*/

/*
 *	フロッピーディスクコントローラ(F)
 */
static void FASTCALL OnFDC(void)
{
	ASSERT(hDrawWnd);

	/* ウインドウが存在すれば、クローズ指示を出す */
	if (hSubWnd[SWND_FDC]) {
		PostMessage(hSubWnd[SWND_FDC], WM_CLOSE, 0, 0);
		return;
	}

	/* ウインドウ作成 */
	hSubWnd[SWND_FDC] = CreateFDC(hDrawWnd, SWND_FDC);
}

/*
 *	バブルメモリコントローラ(B)
 */
#if XM7_VER == 1
#if defined(BUBBLE)
static void FASTCALL OnBMC(void)
{
	ASSERT(hDrawWnd);

	/* ウインドウが存在すれば、クローズ指示を出す */
	if (hSubWnd[SWND_BMC]) {
		PostMessage(hSubWnd[SWND_BMC], WM_CLOSE, 0, 0);
		return;
	}

	/* ウインドウ作成 */
	hSubWnd[SWND_BMC] = CreateBMC(hDrawWnd, SWND_BMC);
}
#endif
#endif

/*
 *	FM音源レジスタ(O)
 */
static void FASTCALL OnOPNReg(void)
{
	ASSERT(hDrawWnd);

	/* ウインドウが存在すれば、クローズ指示を出す */
	if (hSubWnd[SWND_OPNREG]) {
		PostMessage(hSubWnd[SWND_OPNREG], WM_CLOSE, 0, 0);
		return;
	}

	/* ウインドウ作成 */
	hSubWnd[SWND_OPNREG] = CreateOPNReg(hDrawWnd, SWND_OPNREG);
}

/*
 *	FM音源ディスプレイ(D)
 */
static void FASTCALL OnOPNDisp(void)
{
	ASSERT(hDrawWnd);

	/* ウインドウが存在すれば、クローズ指示を出す */
	if (hSubWnd[SWND_OPNDISP]) {
		PostMessage(hSubWnd[SWND_OPNDISP], WM_CLOSE, 0, 0);
		return;
	}

	/* ウインドウ作成 */
	hSubWnd[SWND_OPNDISP] = CreateOPNDisp(hDrawWnd, SWND_OPNDISP);
}

/*
 *	サブCPUコントロール(C)
 */
static void FASTCALL OnSubCtrl(void)
{
	ASSERT(hDrawWnd);

	/* ウインドウが存在すれば、クローズ指示を出す */
	if (hSubWnd[SWND_SUBCTRL]) {
		PostMessage(hSubWnd[SWND_SUBCTRL], WM_CLOSE, 0, 0);
		return;
	}

	/* ウインドウ作成 */
	hSubWnd[SWND_SUBCTRL] = CreateSubCtrl(hDrawWnd, SWND_SUBCTRL);
}

/*
 *	パレットレジスタ(P)
 */
static void FASTCALL OnPaletteReg(void)
{
	ASSERT(hDrawWnd);

	/* ウインドウが存在すれば、クローズ指示を出す */
	if (hSubWnd[SWND_PALETTE]) {
		PostMessage(hSubWnd[SWND_PALETTE], WM_CLOSE, 0, 0);
		return;
	}

	/* ウインドウ作成 */
	hSubWnd[SWND_PALETTE] = CreatePaletteReg(hDrawWnd, SWND_PALETTE);
}

	/*
 *	キーボード(K)
 */
static void FASTCALL OnKeyboard(void)
{
	ASSERT(hDrawWnd);

	/* ウインドウが存在すれば、クローズ指示を出す */
	if (hSubWnd[SWND_KEYBOARD]) {
		PostMessage(hSubWnd[SWND_KEYBOARD], WM_CLOSE, 0, 0);
		return;
	}

	/* ウインドウ作成 */
	hSubWnd[SWND_KEYBOARD] = CreateKeyboard(hDrawWnd, SWND_KEYBOARD);
}

/*
 *	論理演算/直線補間(L)
 */
#if XM7_VER >= 2
static void FASTCALL OnALULine(void)
{
	ASSERT(hDrawWnd);

	/* ウインドウが存在すれば、クローズ指示を出す */
	if (hSubWnd[SWND_ALULINE]) {
		PostMessage(hSubWnd[SWND_ALULINE], WM_CLOSE, 0, 0);
		return;
	}

	/* ウインドウ作成 */
	hSubWnd[SWND_ALULINE] = CreateALULine(hDrawWnd, SWND_ALULINE);
}
#endif

/*
 *	メモリ管理(M)
 */
static void FASTCALL OnMMR(void)
{
	ASSERT(hDrawWnd);

	/* ウインドウが存在すれば、クローズ指示を出す */
	if (hSubWnd[SWND_MMR]) {
		PostMessage(hSubWnd[SWND_MMR], WM_CLOSE, 0, 0);
		return;
	}

	/* ウインドウ作成 */
	hSubWnd[SWND_MMR] = CreateMMR(hDrawWnd, SWND_MMR);
}

#if XM7_VER >= 3
/*
 *	DMAコントローラ(A)
 */
static void FASTCALL OnDMAC(void)
{
	ASSERT(hDrawWnd);

	/* ウインドウが存在すれば、クローズ指示を出す */
	if (hSubWnd[SWND_DMAC]) {
		PostMessage(hSubWnd[SWND_DMAC], WM_CLOSE, 0, 0);
		return;
	}

	/* ウインドウ作成 */
	hSubWnd[SWND_DMAC] = CreateDMAC(hDrawWnd, SWND_DMAC);
}
#endif

/*
 *	ステータスバー(S)
 */
static void FASTCALL OnStatus(void)
{
	/* ステータスバーが有効でなければ、何もしない */
	if (!hStatusBar) {
		return;
	}

	if (IsWindowVisible(hStatusBar)) {
		/* 消去 */
		ShowWindow(hStatusBar, SW_HIDE);
		bHideStatus = TRUE;
	}
	else {
		/* 表示 */
		ShowWindow(hStatusBar, SW_SHOW);
		bHideStatus = FALSE;
	}

	/* 設定ワークに反映 */
	GetCfg();

	/* フレームウインドウのサイズを補正 */
	OnSize(hMainWnd, 640, 400);
}

/*
 *	最新の情報に更新(R)
 */
void FASTCALL OnRefresh(HWND hWnd)
{
	int i;

	ASSERT(hWnd);
	ASSERT(hDrawWnd);

	/* 逆アセンブルウインドウ　アドレス更新 */
	if ((hSubWnd[SWND_DISASM_MAIN]) && (bSyncDisasm[0])) {
		AddrDisAsm(MAINCPU, maincpu.pc);
	}
	if ((hSubWnd[SWND_DISASM_SUB]) && (bSyncDisasm[1])) {
		AddrDisAsm(SUBCPU, subcpu.pc);
	}
#if XM7_VER == 1
#if defined(JSUB)
	if ((hSubWnd[SWND_DISASM_JSUB]) && (bSyncDisasm[2])) {
		AddrDisAsm(JSUBCPU, jsubcpu.pc);
	}
#endif
#endif

	/* FM音源/PSGレジスタ/ディスプレイウインドウ更新 */
	ReSizeOPNReg();
	ReSizeOPNDisp();

	/* メインウインドウ */
	InvalidateRect(hWnd, NULL, FALSE);
	InvalidateRect(hDrawWnd, NULL, FALSE);

	/* サブウインドウ */
	for (i=0; i<SWND_MAXNUM; i++) {
		if (hSubWnd[i]) {
			InvalidateRect(hSubWnd[i], NULL, FALSE);
		}
	}
}

/*
 *	実行に同期(Y)
 */
static void FASTCALL OnSync(void)
{
	bSync = (!bSync);
}

/*
 *	フルスクリーン(U)
 */
static void FASTCALL OnFullScreen(HWND hWnd)
{
	BOOL bRun;
	int i;

	ASSERT(hWnd);

#if defined(KBDPASTE)
	if (hKeyStrokeDialog) {
		/* ウインドウクローズ */
		PostMessage(hKeyStrokeDialog, WM_CLOSE, 0, 0);
		Sleep(1);
	}
#endif

	/* VMをロック、ストップ */
	LockVM();
	bRun = run_flag;
	run_flag = FALSE;
	StopSnd();

	/* モード切り替え */
	if (bFullScreen) {
		ModeDraw(hWnd, FALSE);

		if (!bFullScreen) {
			/* すべてのサブウィンドウをフルスクリーン化前の状態に戻す */
			for (i=0; i<SWND_MAXNUM; i++) {
				if (hSubWnd[i]) {
					if (bShowSubWindow[i]) {
						ShowWindow(hSubWnd[i], SW_RESTORE);
					}
				}
			}
			if (bPopupSwnd) {
				SetForegroundWindow(hWnd);
			}
		}
	}
	else {
		ModeDraw(hWnd, TRUE);

		if (bFullScreen) {
			/* すべてのサブウィンドウを隠す */
			for (i=0; i<SWND_MAXNUM; i++) {
				bShowSubWindow[i] = FALSE;
				if (hSubWnd[i]) {
					if (ShowWindow(hSubWnd[i], SW_HIDE)) {
						bShowSubWindow[i] = TRUE;
					}
				}
			}
		}
	}

	/* VMをアンロック */
	GetCfg();
	run_flag = bRun;
	ResetSch();
	UnlockVM();
	PlaySnd();
}

/*
 *	表示(V)メニュー
 */
static BOOL FASTCALL OnView(HWND hWnd, WORD wID)
{
	ASSERT(hWnd);

	switch (wID) {
		/* FDC */
		case IDM_FDC:
			OnFDC();
			return TRUE;

		/* バブルメモリコントローラ */
#if XM7_VER == 1
#if defined(BUBBLE)
		case IDM_BMC:
			OnBMC();
			return TRUE;
#endif
#endif

		/* OPNレジスタ */
		case IDM_OPNREG:
			OnOPNReg();
			return TRUE;

		/* OPNディスプレイ */
		case IDM_OPNDISP:
			OnOPNDisp();
			return TRUE;

		/* サブCPUコントロール */
		case IDM_SUBCTRL:
			OnSubCtrl();
			return TRUE;

		/* パレットレジスタ */
		case IDM_PALETTE:
			OnPaletteReg();
			return TRUE;

		/* キーボード */
		case IDM_KEYBOARD:
			OnKeyboard();
			return TRUE;

#if XM7_VER >= 2
		/* 論理演算/直線補間 */
		case IDM_ALULINE:
			OnALULine();
			return TRUE;
#endif

		/* メモリ管理 */
		case IDM_MMR:
			OnMMR();
			return TRUE;

#if XM7_VER >= 3
		/* DMAコントローラ */
		case IDM_DMAC:
			OnDMAC();
			return TRUE;
#endif

		/* ステータスバー */
		case IDM_STATUS:
			OnStatus();
			return TRUE;

		/* 最新の情報に更新 */
		case IDM_REFRESH:
			OnRefresh(hWnd);
			return TRUE;

		/* 実行に同期 */
		case IDM_SYNC:
			OnSync();
			return TRUE;

		/* フルスクリーン */
		case IDM_FULLSCREEN:
			OnFullScreen(hWnd);
			return TRUE;
	}

	return FALSE;
}

/*
 *	表示(V)メニュー更新
 */
static void FASTCALL OnViewPopup(HMENU hMenu)
{
	/* サブウインドウ群 */
	CheckMenuSub(hMenu, IDM_FDC, (BOOL)hSubWnd[SWND_FDC]);
#if XM7_VER == 1
#if defined(BUBBLE)
	CheckMenuSub(hMenu, IDM_BMC, (BOOL)hSubWnd[SWND_BMC]);
#endif
#endif
	CheckMenuSub(hMenu, IDM_OPNREG, (BOOL)hSubWnd[SWND_OPNREG]);
	CheckMenuSub(hMenu, IDM_OPNDISP, (BOOL)hSubWnd[SWND_OPNDISP]);
	CheckMenuSub(hMenu, IDM_PALETTE, (BOOL)hSubWnd[SWND_PALETTE]);
	CheckMenuSub(hMenu, IDM_SUBCTRL, (BOOL)hSubWnd[SWND_SUBCTRL]);
#if XM7_VER >= 2
	CheckMenuSub(hMenu, IDM_ALULINE, (BOOL)hSubWnd[SWND_ALULINE]);
#endif
	CheckMenuSub(hMenu, IDM_KEYBOARD, (BOOL)hSubWnd[SWND_KEYBOARD]);
	CheckMenuSub(hMenu, IDM_MMR, (BOOL)hSubWnd[SWND_MMR]);
#if XM7_VER >= 3
	CheckMenuSub(hMenu, IDM_DMAC, (BOOL)hSubWnd[SWND_DMAC]);
#endif

	/* その他 */
	if (hStatusBar) {
		CheckMenuSub(hMenu, IDM_STATUS, IsWindowVisible(hStatusBar));
	}
	else {
		CheckMenuSub(hMenu, IDM_STATUS, FALSE);
	}
	CheckMenuSub(hMenu, IDM_SYNC, bSync);
	CheckMenuSub(hMenu, IDM_FULLSCREEN, bFullScreen);

	/* フルスクリーン時のサブウィンドウメニュー無効化 */
	EnableMenuSub(hMenu, IDM_FDC, !bFullScreen);
#if XM7_VER == 1
#if defined(BUBBLE)
	EnableMenuSub(hMenu, IDM_BMC, !bFullScreen);
#endif
#endif
	EnableMenuSub(hMenu, IDM_OPNREG, !bFullScreen);
	EnableMenuSub(hMenu, IDM_OPNDISP, !bFullScreen);
	EnableMenuSub(hMenu, IDM_PALETTE, !bFullScreen);
	EnableMenuSub(hMenu, IDM_SUBCTRL, !bFullScreen);
#if XM7_VER >= 2
	EnableMenuSub(hMenu, IDM_ALULINE, !bFullScreen);
#endif
	EnableMenuSub(hMenu, IDM_KEYBOARD, !bFullScreen);
	EnableMenuSub(hMenu, IDM_MMR, !bFullScreen);
#if XM7_VER >= 3
	EnableMenuSub(hMenu, IDM_DMAC, !bFullScreen);
#endif
}

/*-[ デバッグメニュー ]-----------------------------------------------------*/

/*
 *	実行(X)
 */
static void FASTCALL OnExec(void)
{
	/* 既に実行中なら、何もしない */
	if (run_flag) {
		return;
	}

	/* スタート */
	LockVM();
	stopreq_flag = FALSE;
	run_flag = TRUE;
	UnlockVM();
}

/*
 *	停止(B)
 */
static void FASTCALL OnBreak(void)
{
	/* 既に停止状態なら、何もしない */
	if (!run_flag) {
		return;
	}

	/* 停止 */
	LockVM();
	stopreq_flag = TRUE;
	UnlockVM();
}

/*
 *	トレース(T)
 */
static void FASTCALL OnTrace(HWND hWnd)
{
	ASSERT(hWnd);

	/* 停止状態でなければ、リターン */
	if (run_flag) {
		return;
	}

	/* 実行 */
	schedule_trace();
	AddrDisAsm(MAINCPU, maincpu.pc);
	AddrDisAsm(SUBCPU, subcpu.pc);
#if XM7_VER == 1
#if defined(JSUB)
	AddrDisAsm(JSUBCPU, jsubcpu.pc);
#endif
#endif

	/* 表示更新 */
	OnRefresh(hWnd);
}

/*
 *	ブレークポイント(B)
 */
static void FASTCALL OnBreakPoint(void)
{
	ASSERT(hDrawWnd);

	/* ウインドウが存在すれば、クローズ指示を出す */
	if (hSubWnd[SWND_BREAKPOINT]) {
		PostMessage(hSubWnd[SWND_BREAKPOINT], WM_CLOSE, 0, 0);
		return;
	}

	/* ウインドウ作成 */
	hSubWnd[SWND_BREAKPOINT] = CreateBreakPoint(hDrawWnd, SWND_BREAKPOINT);
}

/*
 *	スケジューラ(S)
 */
static void FASTCALL OnScheduler(void)
{
	ASSERT(hDrawWnd);

	/* ウインドウが存在すれば、クローズ指示を出す */
	if (hSubWnd[SWND_SCHEDULER]) {
		PostMessage(hSubWnd[SWND_SCHEDULER], WM_CLOSE, 0, 0);
		return;
	}

	/* ウインドウ作成 */
	hSubWnd[SWND_SCHEDULER] = CreateScheduler(hDrawWnd, SWND_SCHEDULER);
}

/*
 *	CPUレジスタ(C)
 */
static void FASTCALL OnCPURegister(int nCPU)
{
	int index;

#if XM7_VER >= 2
	ASSERT((nCPU >= MAINCPU) && (nCPU <= SUBCPU));
#elif defined(Z80CARD)
	ASSERT((nCPU >= MAINCPU) && (nCPU <= MAINZ80));
#else
	ASSERT((nCPU >= MAINCPU) && (nCPU <= JSUBCPU));
#endif
	ASSERT(hDrawWnd);

	/* インデックス決定 */
	index = SWND_CPUREG_MAIN + nCPU;

	/* ウインドウが存在すれば、クローズ指示を出す */
	if (hSubWnd[index]) {
		PostMessage(hSubWnd[index], WM_CLOSE, 0, 0);
		return;
	}

	/* ウインドウ作成 */
	hSubWnd[index] = CreateCPURegister(hDrawWnd, (BYTE)nCPU, index);
}

/*
 *	逆アセンブル(D)
 */
static void FASTCALL OnDisAsm(int nCPU)
{
	int index;

#if XM7_VER >= 2
	ASSERT((nCPU >= MAINCPU) && (nCPU <= SUBCPU));
#elif defined(Z80CARD)
	ASSERT((nCPU >= MAINCPU) && (nCPU <= MAINZ80));
#else
	ASSERT((nCPU >= MAINCPU) && (nCPU <= JSUBCPU));
#endif
	ASSERT(hDrawWnd);

	/* インデックス決定 */
	index = SWND_DISASM_MAIN + nCPU;

	/* ウインドウが存在すれば、クローズ指示を出す */
	if (hSubWnd[index]) {
		PostMessage(hSubWnd[index], WM_CLOSE, 0, 0);
		return;
	}

	/* ウインドウ作成 */
	hSubWnd[index] = CreateDisAsm(hDrawWnd, (BYTE)nCPU, index);
}

/*
 *	メモリダンプ(M)
 */
static void FASTCALL OnMemory(int nCPU)
{
	int index;

#if XM7_VER >= 2
	ASSERT((nCPU >= MAINCPU) && (nCPU <= SUBCPU));
#else
	/* このケースのみZ80モード専用ウインドウは存在しない */
	ASSERT((nCPU >= MAINCPU) && (nCPU <= JSUBCPU));
#endif
	ASSERT(hDrawWnd);

	/* インデックス決定 */
	index = SWND_MEMORY_MAIN + nCPU;

	/* ウインドウが存在すれば、クローズ指示を出す */
	if (hSubWnd[index]) {
		PostMessage(hSubWnd[index], WM_CLOSE, 0, 0);
		return;
	}

	/* ウインドウ作成 */
	hSubWnd[index] = CreateMemory(hDrawWnd, (BYTE)nCPU, index);
}

/*
 *	デバッグ(D)メニュー
 */
static BOOL FASTCALL OnDebug(HWND hWnd, WORD wID)
{
	ASSERT(hWnd);

	switch (wID) {
		/* 実行 */
		case IDM_EXEC:
			OnExec();
			return TRUE;

		/* ブレーク */
		case IDM_BREAK:
			OnBreak();
			return TRUE;

		/* トレース */
		case IDM_TRACE:
			OnTrace(hWnd);
			return TRUE;

		/* ブレークポイント */
		case IDM_BREAKPOINT:
			OnBreakPoint();
			return TRUE;

		/* スケジューラ */
		case IDM_SCHEDULER:
			OnScheduler();
			return TRUE;

		/* CPUレジスタ(メイン) */
		case IDM_CPU_MAIN:
			OnCPURegister(MAINCPU);
			return TRUE;

		/* CPUレジスタ(サブ) */
		case IDM_CPU_SUB:
			OnCPURegister(SUBCPU);
			return TRUE;

		/* 逆アセンブル(メイン) */
		case IDM_DISASM_MAIN:
			OnDisAsm(MAINCPU);
			return TRUE;

		/* 逆アセンブル(サブ) */
		case IDM_DISASM_SUB:
			OnDisAsm(SUBCPU);
			return TRUE;

		/* メモリダンプ(メイン) */
		case IDM_MEMORY_MAIN:
			OnMemory(MAINCPU);
			return TRUE;

		/* メモリダンプ(サブ) */
		case IDM_MEMORY_SUB:
			OnMemory(SUBCPU);
			return TRUE;

#if XM7_VER == 1
#if defined(JSUB)
		/* CPUレジスタ(日本語サブ) */
		case IDM_CPU_JSUB:
			OnCPURegister(JSUBCPU);
			return TRUE;

		/* 逆アセンブル(日本語サブ) */
		case IDM_DISASM_JSUB:
			OnDisAsm(JSUBCPU);
			return TRUE;

		/* メモリダンプ(日本語サブ) */
		case IDM_MEMORY_JSUB:
			OnMemory(JSUBCPU);
			return TRUE;
#endif

#if defined(Z80CARD)
		/* CPUレジスタ(メインZ80) */
		case IDM_CPU_MAINZ80:
			OnCPURegister(MAINZ80);
			return TRUE;

		/* 逆アセンブル(メインZ80) */
		case IDM_DISASM_MAINZ80:
			OnDisAsm(MAINZ80);
			return TRUE;
#endif
#endif
	}

	return FALSE;
}

/*
 *	デバッグ(D)メニュー更新
 */
static void FASTCALL OnDebugPopup(HMENU hMenu)
{
	ASSERT(hMenu);

	/* 実行制御 */
	EnableMenuSub(hMenu, IDM_EXEC, !run_flag);
	EnableMenuSub(hMenu, IDM_BREAK, run_flag);
	EnableMenuSub(hMenu, IDM_TRACE, !run_flag);

	/* サブウインドウ群 */
	CheckMenuSub(hMenu, IDM_BREAKPOINT, (BOOL)hSubWnd[SWND_BREAKPOINT]);
	CheckMenuSub(hMenu, IDM_SCHEDULER, (BOOL)hSubWnd[SWND_SCHEDULER]);
	CheckMenuSub(hMenu, IDM_CPU_MAIN, (BOOL)hSubWnd[SWND_CPUREG_MAIN]);
	CheckMenuSub(hMenu, IDM_CPU_SUB, (BOOL)hSubWnd[SWND_CPUREG_SUB]);
	CheckMenuSub(hMenu, IDM_DISASM_MAIN, (BOOL)hSubWnd[SWND_DISASM_MAIN]);
	CheckMenuSub(hMenu, IDM_DISASM_SUB, (BOOL)hSubWnd[SWND_DISASM_SUB]);
	CheckMenuSub(hMenu, IDM_MEMORY_MAIN, (BOOL)hSubWnd[SWND_MEMORY_MAIN]);
	CheckMenuSub(hMenu, IDM_MEMORY_SUB, (BOOL)hSubWnd[SWND_MEMORY_SUB]);
#if XM7_VER == 1 
#if defined(JSUB)
	CheckMenuSub(hMenu, IDM_CPU_JSUB, (BOOL)hSubWnd[SWND_CPUREG_JSUB]);
	CheckMenuSub(hMenu, IDM_DISASM_JSUB, (BOOL)hSubWnd[SWND_DISASM_JSUB]);
	CheckMenuSub(hMenu, IDM_MEMORY_JSUB, (BOOL)hSubWnd[SWND_MEMORY_JSUB]);
#endif
#if defined(Z80CARD)
	CheckMenuSub(hMenu, IDM_CPU_MAINZ80, (BOOL)hSubWnd[SWND_CPUREG_Z80]);
	CheckMenuSub(hMenu, IDM_DISASM_MAINZ80, (BOOL)hSubWnd[SWND_DISASM_Z80]);
#endif
#endif

	/* フルスクリーン時のサブウィンドウメニュー無効化 */
	EnableMenuSub(hMenu, IDM_BREAKPOINT, !bFullScreen);
	EnableMenuSub(hMenu, IDM_SCHEDULER, !bFullScreen);
	EnableMenuSub(hMenu, IDM_CPU_MAIN, !bFullScreen);
	EnableMenuSub(hMenu, IDM_CPU_SUB, !bFullScreen);
	EnableMenuSub(hMenu, IDM_DISASM_MAIN, !bFullScreen);
	EnableMenuSub(hMenu, IDM_DISASM_SUB, !bFullScreen);
	EnableMenuSub(hMenu, IDM_MEMORY_MAIN, !bFullScreen);
	EnableMenuSub(hMenu, IDM_MEMORY_SUB, !bFullScreen);
#if XM7_VER == 1 
#if defined(JSUB)
	EnableMenuSub(hMenu, IDM_CPU_JSUB,
		!bFullScreen && jsub_available && jsub_enable);
	EnableMenuSub(hMenu, IDM_DISASM_JSUB,
		!bFullScreen && jsub_available && jsub_enable);
	EnableMenuSub(hMenu, IDM_MEMORY_JSUB,
		!bFullScreen && jsub_available && jsub_enable);
#endif
#if defined(Z80CARD)
	EnableMenuSub(hMenu, IDM_CPU_MAINZ80, !bFullScreen);
	EnableMenuSub(hMenu, IDM_DISASM_MAINZ80, !bFullScreen);
#endif
#endif
}

/*-[ ツールメニュー ]-------------------------------------------------------*/

/*
 *	マウスモード切り換え
 */
#if defined(MOUSE)
void FASTCALL MouseModeChange(BOOL flag)
{
	/* モード変化がなければ帰る */
	if (mos_capture == flag) {
		return;
	}

	/* マウスキャプチャフラグを設定 */
	mos_capture = flag;

	/* ステータスバーに状態表示 */
	if (flag) {
		SetStatusMessage(IDS_MOUSE_ENABLE);
	}
	else {
		SetStatusMessage(IDS_MOUSE_DISABLE);
	}
}
#endif

/*
 *	時刻アジャスト
 */
#if XM7_VER >= 2
static void FASTCALL OnTimeAdjust(void)
{
	/* 時刻を再設定する */
	rtc_time_adjust();

	/* 念のためスケジュールを初期化 */
	rtc_reset();
}
#endif

/*
 *	マウスモード切り換え(M)
 */
#if defined(MOUSE)
static void FASTCALL OnMouseMode(void)
{
	/* マウスキャプチャフラグを反転させてモード切り替え */
	MouseModeChange(!mos_capture);
}
#endif

/*
 *	画面キャプチャ(C)
 */
static void FASTCALL OnGrpCapture(void)
{
	char path[_MAX_PATH];

	/* ファイル選択 */
	if (!FileSelectSub(FALSE, IDS_GRPCAPFILTER, path, "BMP", 3)) {
		return;
	}

	/* キャプチャ */
	LockVM();
	StopSnd();
#if XM7_VER == 1
	capture_to_bmp(path, bFullScan, bGreenMonitor, bPseudo400Line);
#elif XM7_VER == 2
	capture_to_bmp(path, bFullScan, bTTLMonitor, bPseudo400Line);
#else
	capture_to_bmp(path, bFullScan, FALSE, bPseudo400Line);
#endif
	PlaySnd();
	ResetSch();
	UnlockVM();
}

/*
 *	画面キャプチャ2
 */
static void FASTCALL OnGrpCapture2(void)
{
	char path[_MAX_PATH];

	/* ファイル選択 */
	if (!FileSelectSub(FALSE, IDS_GRPCAPFILTER, path, "BMP", 3)) {
		return;
	}

	/* キャプチャ */
	LockVM();
	StopSnd();
#if XM7_VER == 1
	capture_to_bmp2(path, bGreenMonitor, bPseudo400Line);
#elif XM7_VER == 2
	capture_to_bmp2(path, bTTLMonitor, bPseudo400Line);
#else
	capture_to_bmp2(path, FALSE, bPseudo400Line);
#endif
	PlaySnd();
	ResetSch();
	UnlockVM();
}

/*
 *	WAVキャプチャ(W)
 */
static void FASTCALL OnWavCapture(HWND hWnd)
{
	char path[_MAX_PATH];

	ASSERT(hWnd);

	/* 既にキャプチャ中なら、クローズ */
	if (hWavCapture >= 0) {
		LockVM();
		CloseCaptureSnd();
		UnlockVM();
		return;
	}

	/* ファイル選択 */
	if (!FileSelectSub(FALSE, IDS_WAVCAPFILTER, path, "WAV", 4)) {
		return;
	}

	/* キャプチャ */
	LockVM();
	OpenCaptureSnd(path);
	UnlockVM();

	/* 条件判定 */
	if (hWavCapture < 0) {
		LockVM();
		StopSnd();

		LoadString(hAppInstance, IDS_WAVCAPERROR, path, sizeof(path));
		MessageBox(hWnd, path, "XM7", MB_ICONSTOP | MB_OK);
		SetMenuExitTimer();

		PlaySnd();
		ResetSch();
		UnlockVM();
	}
}

/*
 *	新規ディスク作成(D)
 */
static void FASTCALL OnNewDisk(HWND hWnd)
{
	char path[_MAX_PATH];
	int ret;
	BOOL err;

	ASSERT(hWnd);

	/* タイトル入力 */
	ret = DialogBox(hAppInstance, MAKEINTRESOURCE(IDD_TITLEDLG),
						hWnd, TitleDlgProc);
	if (ret != IDOK) {
		return;
	}

	/* ファイル選択 */
	if (!FileSelectSub(FALSE, IDS_NEWDISKFILTER, path, "D77", 0)) {
		return;
	}

	/* 作成 */
	LockVM();
	StopSnd();

	if (DiskFormat) {
		err = make_new_userdisk(path, DiskTitle, DiskMedia);
	}
	else {
		err = make_new_d77(path, DiskTitle, DiskMedia);
	}
	if (err) {
		LoadString(hAppInstance, IDS_NEWDISKOK, path, sizeof(path));
		MessageBox(hWnd, path, "XM7", MB_OK);
		SetMenuExitTimer();
	}

	PlaySnd();
	ResetSch();
	UnlockVM();
}

/*
 *	新規テープ作成(T)
 */
static void FASTCALL OnNewTape(HWND hWnd)
{
	char path[_MAX_PATH];

	ASSERT(hWnd);

	/* ファイル選択 */
	if (!FileSelectSub(FALSE, IDS_TAPEFILTER, path, "T77", 1)) {
		return;
	}

	/* 作成 */
	LockVM();
	StopSnd();

	if (make_new_t77(path)) {
		LoadString(hAppInstance, IDS_NEWTAPEOK, path, sizeof(path));
		MessageBox(hWnd, path, "XM7", MB_OK);
		SetMenuExitTimer();
	}

	PlaySnd();
	ResetSch();
	UnlockVM();
}

#if XM7_VER == 1
#if defined(BUBBLE)
/*
 *	新規バブルカセット作成(B)
 */
static void FASTCALL OnNewBubble(HWND hWnd)
{
	char path[_MAX_PATH];
	int ret;
	BOOL err;

	ASSERT(hWnd);

	/* タイトル入力 */
	strncpy(BubbleTitle, "Default", sizeof(BubbleTitle));
	ret = DialogBox(hAppInstance, MAKEINTRESOURCE(IDD_MEDIATYPEDLG),
						hWnd, BubbleMediaTypeDlgProc);
	if (ret != IDOK) {
		return;
	}

	/* ファイル選択 */
	if (BubbleFormat) {
		if (!FileSelectSub(FALSE, IDS_B77FILTER, path, "B77", 5)) {
			return;
		}
	}
	else {
		if (!FileSelectSub(FALSE, IDS_BBLFILTER, path, "BBL", 5)) {
			return;
		}
	}

	/* 作成 */
	LockVM();
	StopSnd();

	if (BubbleFormat) {
		err = make_new_bubble(path, BubbleTitle);
	}
	else {
		err = make_new_bubble(path, NULL);
	}
	if (err) {
		LoadString(hAppInstance, IDS_NEWBUBBLEOK, path, sizeof(path));
		MessageBox(hWnd, path, "XM7", MB_OK);
		SetMenuExitTimer();
	}

	PlaySnd();
	ResetSch();
	UnlockVM();
}
#endif
#endif

/*
 *	VFD→D77変換(V)
 */
static void FASTCALL OnVFD2D77(HWND hWnd)
{
	char src[_MAX_PATH];
	char dst[_MAX_PATH];
	int ret;

	ASSERT(hWnd);

	/* ファイル選択 */
	if (!FileSelectSub(TRUE, IDS_VFDFILTER, src, "VFD", 0)) {
		return;
	}

	/* タイトル入力 */
	ret = DialogBox(hAppInstance, MAKEINTRESOURCE(IDD_TITLEDLG_2D),
						hWnd, TitleDlg2DProc);
	if (ret != IDOK) {
		SetMenuExitTimer();
		return;
	}

	/* ファイル選択 */
	if (!FileSelectSub(FALSE, IDS_NEWDISKFILTER, dst, "D77", 0)) {
		return;
	}

	/* 作成 */
	LockVM();
	StopSnd();

	if (conv_vfd_to_d77(src, dst, DiskTitle)) {
		LoadString(hAppInstance, IDS_CONVERTOK, src, sizeof(src));
	}
	else {
		LoadString(hAppInstance, IDS_CONVERTFAIL, src, sizeof(src));
	}
	MessageBox(hWnd, src, "XM7", MB_OK);
	SetMenuExitTimer();

	PlaySnd();
	ResetSch();
	UnlockVM();
}

/*
 *	2D→D77変換(2)
 */
static void FASTCALL On2D2D77(HWND hWnd)
{
	char src[_MAX_PATH];
	char dst[_MAX_PATH];
	int ret;

	ASSERT(hWnd);

	/* ファイル選択 */
	if (!FileSelectSub(TRUE, IDS_2DFILTER, src, "2D", 0)) {
		return;
	}

	/* タイトル入力 */
	ret = DialogBox(hAppInstance, MAKEINTRESOURCE(IDD_TITLEDLG_2D),
						hWnd, TitleDlg2DProc);
	if (ret != IDOK) {
		SetMenuExitTimer();
		return;
	}

	/* ファイル選択 */
	if (!FileSelectSub(FALSE, IDS_NEWDISKFILTER, dst, "D77", 0)) {
		return;
	}

	/* 作成 */
	LockVM();
	StopSnd();

	if (conv_2d_to_d77(src, dst, DiskTitle)) {
		LoadString(hAppInstance, IDS_CONVERTOK, src, sizeof(src));
	}
	else {
		LoadString(hAppInstance, IDS_CONVERTFAIL, src, sizeof(src));
	}
	MessageBox(hWnd, src, "XM7", MB_OK);
	SetMenuExitTimer();

	PlaySnd();
	ResetSch();
	UnlockVM();
}

/*
 *	VTP→T77変換(P)
 */
static void FASTCALL OnVTP2T77(HWND hWnd)
{
	char src[_MAX_PATH];
	char dst[_MAX_PATH];

	ASSERT(hWnd);

	/* ファイル選択 */
	if (!FileSelectSub(TRUE, IDS_VTPFILTER, src, "VTP", 1)) {
		return;
	}

	/* ファイル選択 */
	if (!FileSelectSub(FALSE, IDS_TAPEFILTER, dst, "T77", 1)) {
		return;
	}

	/* 作成 */
	LockVM();
	StopSnd();

	if (conv_vtp_to_t77(src, dst)) {
		LoadString(hAppInstance, IDS_CONVERTOK, src, sizeof(src));
	}
	else {
		LoadString(hAppInstance, IDS_CONVERTFAIL, src, sizeof(src));
	}
	MessageBox(hWnd, src, "XM7", MB_OK);
	SetMenuExitTimer();

	PlaySnd();
	ResetSch();
	UnlockVM();
}

#if XM7_VER == 1
#if defined(BUBBLE)
/*
 *	BBL→B77変換(L)
 */
static void FASTCALL OnBBL2B77(HWND hWnd)
{
	char src[_MAX_PATH];
	char dst[_MAX_PATH];
	int ret;

	ASSERT(hWnd);

	/* ファイル選択 */
	if (!FileSelectSub(TRUE, IDS_BBLFILTER, src, "BBL", 5)) {
		return;
	}

	/* タイトル入力 */
	strncpy(DiskTitle, "Default", sizeof(DiskTitle));
	ret = DialogBox(hAppInstance, MAKEINTRESOURCE(IDD_TITLEDLG_2D),
						hWnd, TitleDlg2DProc);
	if (ret != IDOK) {
		SetMenuExitTimer();
		return;
	}

	/* ファイル選択 */
	if (!FileSelectSub(FALSE, IDS_B77FILTER, dst, "B77", 5)) {
		return;
	}

	/* 作成 */
	LockVM();
	StopSnd();

	if (conv_bbl_to_b77(src, dst, DiskTitle)) {
		LoadString(hAppInstance, IDS_CONVERTOK, src, sizeof(src));
	}
	else {
		LoadString(hAppInstance, IDS_CONVERTFAIL, src, sizeof(src));
	}
	MessageBox(hWnd, src, "XM7", MB_OK);
	SetMenuExitTimer();

	PlaySnd();
	ResetSch();
	UnlockVM();
}
#endif
#endif

#if defined(KBDPASTE)
/*
 *	貼り付け(E)
 */
static void FASTCALL OnPaste(HWND hWnd)
{
	ASSERT(hWnd);

	/* ペースト待ち時間設定がない場合 */
	if ((uPasteWait == 0) && (uPasteWaitCntl == 0)) {
		return;
	}

	/* 実行 */
	LockVM();
	StopSnd();

	PasteClipboardKbd(hWnd);

	PlaySnd();
	ResetSch();
	UnlockVM();
}

/*
 *	キー入力支援(K)
 */
static void FASTCALL OnKeyStroke(HWND hWnd)
{
	int ret;

	ASSERT(hWnd);

	/* ペースト待ち時間設定がない場合 */
	if ((uPasteWait == 0) && (uPasteWaitCntl == 0)) {
		return;
	}

	/* 実行 */
	if (bFullScreen || !bKeyStrokeModeless) {
		/* 二重に開かない */
		if (hKeyStrokeDialog) {
			/* ウインドウクローズ */
			PostMessage(hKeyStrokeDialog, WM_CLOSE, 0, 0);
			Sleep(1);
		}

		/* キー入力支援ダイアログ */
		ret = DialogBox(hAppInstance, MAKEINTRESOURCE(IDD_KEYSTROKEDLG),
						hWnd, KeyStrokeDlgProc);
		if (ret != IDOK) {
			return;
		}

		/* 作成 */
		LockVM();
		PasteKbd((char *)KeyStrokeString);
		InvalidateRect(hDrawWnd, NULL, FALSE);
		UnlockVM();
	}
	else {
		/* 二重に開かない */
		if (hKeyStrokeDialog) {
			SetForegroundWindow(hKeyStrokeDialog);
			return;
		}

		/* キー入力支援ダイアログ */
		hKeyStrokeDialog = CreateDialog(
			(HINSTANCE)GetWindowLong(hWnd, GWL_HINSTANCE),
			MAKEINTRESOURCE(IDD_KEYSTROKEDLG),
			hWnd,
			(DLGPROC)KeyStrokeDlgProc);
		if (hKeyStrokeDialog == NULL) {
			return;
		}
		ShowWindow(hKeyStrokeDialog, SW_SHOW);
		UpdateWindow(hKeyStrokeDialog);
	}
}
#endif

/*
 *	サウンド出力モード切り替え
 */
static void FASTCALL OnChgSound(void)
{
	LockVM();

	/* サウンドモード変更 */
	uStereoOut = (uStereoOut + 1) % 5;
	SetStatusMessage(IDS_SND_MONAURAL + uStereoOut);

	/* 適用 */
	ApplySnd();
	UnlockVM();
}

/*
 *	サイクルスチールモード切り替え
 */
static void FASTCALL OnChgCycleSteal(void)
{
	LockVM();

	/* サイクルスチールモードを変更 */
	cycle_steal = !cycle_steal;
	cycle_steal_default = cycle_steal;

	/* subclock_modeはcycle_stealの反転 */
	subclock_mode = !cycle_steal;

	GetCfg();

	/* ここではsubclock_modeを見る必要はない */
	if (cycle_steal) {
		SetStatusMessage(IDS_ENABLE_CYCLESTEAL);
	}
	else {
		SetStatusMessage(IDS_DISABLE_CYCLESTEAL);
	}

	UnlockVM();
}

/*
 *	ツール(T)メニュー
 */
static BOOL FASTCALL OnTool(HWND hWnd, WORD wID)
{
	ASSERT(hWnd);

	switch (wID) {
		/* 設定 */
		case IDM_CONFIG:
			OnConfig(hWnd);
			return TRUE;

#if defined(MOUSE)
		/* マウスモード切り換え */
		case IDM_MOUSEMODE:
			if (bDetectMouse) {
				OnMouseMode();
			}
			return TRUE;

		/* マウスモード切り換え(F11,メニュー非表示) */
		case IDM_MOUSEMODE2:
			if (!kbd_table[DIK_F11] && bDetectMouse) {
				OnMouseMode();
			}
			return TRUE;

		/* マウスモード有効 */
		case IDM_MOUSEON:
			if (bDetectMouse) {
				MouseModeChange(TRUE);
			}
			return TRUE;

		/* マウスモード無効 */
		case IDM_MOUSEOFF:
			if (bDetectMouse) {
				MouseModeChange(FALSE);
			}
			return TRUE;
#endif

#if XM7_VER >= 2
		/* 時刻アジャスト */
		case IDM_TIMEADJUST:
			OnTimeAdjust();
			return TRUE;
#endif

		/* 画面キャプチャ */
		case IDM_GRPCAP:
			OnGrpCapture();
			return TRUE;

		/* 画面キャプチャ2 */
		case IDM_GRPCAP2:
			OnGrpCapture2();
			return TRUE;

		/* WAVキャプチャ */
		case IDM_WAVCAP:
			OnWavCapture(hWnd);
			return TRUE;

		/* 新規ディスク */
		case IDM_NEWDISK:
			OnNewDisk(hWnd);
			return TRUE;

		/* 新規テープ */
		case IDM_NEWTAPE:
			OnNewTape(hWnd);
			return TRUE;

#if XM7_VER == 1
#if defined(BUBBLE)
		/* 新規バブルカセット */
		case IDM_NEWBUBBLE:
			OnNewBubble(hWnd);
			return TRUE;
#endif
#endif

		/* VFD→D77 */
		case IDM_VFD2D77:
			OnVFD2D77(hWnd);
			return TRUE;

		/* 2D→D77 */
		case IDM_2D2D77:
			On2D2D77(hWnd);
			return TRUE;

		/* VTP→T77 */
		case IDM_VTP2T77:
			OnVTP2T77(hWnd);
			return TRUE;

#if XM7_VER == 1
#if defined(BUBBLE)
		/* BBL→B77 */
		case IDM_BBL2B77:
			OnBBL2B77(hWnd);
			return TRUE;
#endif
#endif

#if defined(KBDPASTE)
		/* 貼り付け */
		case IDM_PASTE:
			OnPaste(hWnd);
			return TRUE;

		/* キー入力支援 */
		case IDM_KEYSTROKE:
			OnKeyStroke(hWnd);
			return TRUE;
#endif

		/* サウンド出力切り替え(メニュー非表示) */
		case IDM_CHG_SOUNDMODE:
			OnChgSound();
			return TRUE;

		/* サイクルスチールモード切り替え(メニュー非表示) */
		case IDM_CYCLESTEAL:
			OnChgCycleSteal();
			return TRUE;
	}

	return FALSE;
}

/*
 *	ツール(T)メニュー更新
 */
static void FASTCALL OnToolPopup(HMENU hMenu)
{
#if defined(MOUSE)
	MENUITEMINFO mii;
	char string[128];
#endif
#if defined(KBDPASTE)
	UINT uitem;
#endif

	ASSERT(hMenu);

#if defined(MOUSE)
	/* メニュー項目からいったん削除 */
	DeleteMenu(hMenu, IDM_MOUSEMODE, MF_BYCOMMAND);

	/* 改めて項目を突っ込む */
	if (kbd_table[DIK_F11]) {
		LoadString(hAppInstance, IDS_MOUSEMODE_DISF11, string, sizeof(string));
	}
	else {
		LoadString(hAppInstance, IDS_MOUSEMODE_ENBF11, string, sizeof(string));
	}

	memset(&mii, 0, sizeof(mii));
	mii.cbSize = 44;	/* sizeof(mii)はWINVER>=0x0500向け */
	mii.fMask = MIIM_TYPE | MIIM_STATE | MIIM_ID;
	mii.fType = MFT_STRING;
	mii.fState = MFS_ENABLED;
	mii.wID = IDM_MOUSEMODE;
	mii.dwTypeData = string;
	mii.cch = strlen(string);
#if XM7_VER >= 2
	InsertMenuItem(hMenu, IDM_TIMEADJUST, MF_BYCOMMAND, &mii);
#else
	InsertMenuItem(hMenu, 2, MF_BYPOSITION, &mii);
#endif
	EnableMenuItem(hMenu, IDM_MOUSEMODE, !bDetectMouse);

	/* マウスモード */
	CheckMenuSub(hMenu, IDM_MOUSEMODE, mos_capture);
#endif

	/* WAVキャプチャ ハンドルが正でオープン中 */
	CheckMenuSub(hMenu, IDM_WAVCAP, (hWavCapture >= 0));

#if defined(KBDPASTE)
	uitem = 12;
#if defined(MOUSE)
	uitem += 1;
#endif
#if XM7_VER >= 2
	uitem += 1;
#endif
#if XM7_VER == 1
#if defined(BUBBLE)
	uitem += 2;
#endif
#endif

	/* INIファイルによる設定がない場合、貼り付け機能をメニュー項目から削除 */
	if ((uPasteWait == 0) && (uPasteWaitCntl == 0)) {
		while (GetMenuItemCount(hMenu) > (int)uitem) {
			DeleteMenu(hMenu, uitem, MF_BYPOSITION);
		}
	}
#endif
}

/*-[ ウィンドウメニュー ]---------------------------------------------------*/

/*
 *	全ウインドウの表示状態を一括して変更
 */
static void FASTCALL ShowAllWindowSub(int CmdShow)
{
	int i;

	for (i=0; i<SWND_MAXNUM; i++) {
		if (hSubWnd[i]) {
			ShowWindow(hSubWnd[i], CmdShow);
		}
	}
}

/*
 *	重ねて表示(C)
 */
static void FASTCALL OnCascade(void)
{
	/* 重ねて表示 */
	ASSERT(hDrawWnd);
	CascadeWindows(hDrawWnd, 0, NULL, 0, NULL);
}

/*
 *	並べて表示(T)
 */
static void FASTCALL OnTile(void)
{
	/* 並べて表示 */
	ASSERT(hDrawWnd);
	TileWindows(hDrawWnd, MDITILE_VERTICAL, NULL, 0, NULL);
}

/*
 *	全てアイコン化(I)
 */
static void FASTCALL OnIconic(void)
{
	ShowAllWindowSub(SW_MINIMIZE);
}

/*
 *	アイコンの整列(A)
 */
static void FASTCALL OnArrangeIcon(void)
{
	/* アイコンの整列 */
	ASSERT(hDrawWnd);
	ArrangeIconicWindows(hDrawWnd);
}

/*
 *	全て隠す(H)
 */
static void FASTCALL OnHide(void)
{
	ShowAllWindowSub(SW_HIDE);
}

/*
 *	全て復元(R)
 */
static void FASTCALL OnRestore(void)
{
	ShowAllWindowSub(SW_RESTORE);

	if (bPopupSwnd) {
		SetForegroundWindow(hMainWnd);
	}
}

/*
 *	全て閉じる(O)
 */
static void FASTCALL OnClose(void)
{
	int i;

	for (i=0; i<SWND_MAXNUM; i++) {
		if (hSubWnd[i]) {
			DestroyWindow(hSubWnd[i]);
			hSubWnd[i] = NULL;
		}
	}
}

/*
 *	ウィンドウ(W)メニュー
 */
static BOOL FASTCALL OnWindow(WORD wID)
{
	int i;

	switch (wID) {
		/* 重ねて表示 */
		case IDM_CASCADE:
			OnCascade();
			return TRUE;

		/* 並べて表示 */
		case IDM_TILE:
			OnTile();
			return TRUE;

		/* 全てアイコン化 */
		case IDM_ICONIC:
			OnIconic();
			return TRUE;

		/* アイコンの整列 */
		case IDM_ARRANGEICON:
			OnArrangeIcon();
			return TRUE;

		/* 全て隠す */
		case IDM_ALLHIDE:
			OnHide();
			return TRUE;

		/* 全て復元 */
		case IDM_ALLRESTORE:
			OnRestore();
			return TRUE;

		/* 全て閉じる */
		case IDM_ALLCLOSE:
			OnClose();
			return TRUE;
	}

	/* ウィンドウ選択か */
	if ((wID >= IDM_SWND00) && (wID <= IDM_SWND15)) {
		for (i=0; i<SWND_MAXNUM; i++) {
			if (hSubWnd[i] == NULL) {
				continue;
			}

			/* カウントダウンか、OK */
			if (wID == IDM_SWND00) {
				ShowWindow(hSubWnd[i], SW_RESTORE);
				SetWindowPos(hSubWnd[i], HWND_TOP, 0, 0, 0, 0,
					SWP_NOMOVE | SWP_NOSIZE);
				break;
			}
			else {
				wID--;
			}
		}
	}

	return FALSE;
}

/*
 *	ウィンドウ(W)メニュー更新
 */
static void FASTCALL OnWindowPopup(HMENU hMenu)
{
	int i;
	BOOL flag;
	MENUITEMINFO mii;
	UINT nID;
	int count;
	char string[128];

	ASSERT(hMenu);

	/* フルスクリーン時の先頭の７つの無効化 */
	EnableMenuSub(hMenu, IDM_CASCADE, !bFullScreen && !bPopupSwnd);
	EnableMenuSub(hMenu, IDM_TILE, !bFullScreen && !bPopupSwnd);
	EnableMenuSub(hMenu, IDM_ICONIC, !bFullScreen);
	EnableMenuSub(hMenu, IDM_ARRANGEICON, !bFullScreen && !bPopupSwnd);
	EnableMenuSub(hMenu, IDM_ALLHIDE, !bFullScreen);
	EnableMenuSub(hMenu, IDM_ALLRESTORE, !bFullScreen);
	EnableMenuSub(hMenu, IDM_ALLCLOSE, !bFullScreen);

	/* 先頭の７つを残して削除 */
	while (GetMenuItemCount(hMenu) > 7) {
		DeleteMenu(hMenu, 7, MF_BYPOSITION);
	}

	/* 有効なサブウインドウがなければ、そのままリターン */
	flag = FALSE;
	for (i=0; i<SWND_MAXNUM; i++) {
		if (hSubWnd[i] != NULL) {
			flag = TRUE;
		}
	}
	if (!flag) {
		return;
	}

	/* メニュー構造体初期化 */
	memset(&mii, 0, sizeof(mii));
	mii.cbSize = 44;	/* sizeof(mii)はWINVER>=0x0500向け */
	mii.fMask = MIIM_TYPE | MIIM_STATE | MIIM_ID;
	mii.fType = MFT_STRING;
	mii.fState = MFS_ENABLED;

	/* セパレータ挿入 */
	mii.fType = MFT_SEPARATOR;
	InsertMenuItem(hMenu, 7, TRUE, &mii);
	mii.fType = MFT_STRING;

	/* ウインドウタイトルをセット */
	count = 0;
	nID = IDM_SWND00;
	for (i=0; i<SWND_MAXNUM; i++) {
		if (hSubWnd[i] != NULL) {
			/* メニュー挿入 */
			mii.wID = nID;
			memset(string, 0, sizeof(string));
			GetWindowText(hSubWnd[i], string, sizeof(string) - 1);
			mii.dwTypeData = string;
			mii.cch = strlen(string);
			InsertMenuItem(hMenu, count + 8, TRUE, &mii);
			EnableMenuSub(hMenu, nID, !bFullScreen);

			/* 次へ */
			nID++;
			count++;
		}
	}
}

/*-[ ヘルプメニュー ]-------------------------------------------------------*/

/*
 *	ヘルプ(H)メニュー
 */
static BOOL FASTCALL OnHelp(HWND hWnd, WORD wID)
{
	switch (wID) {
		/* バージョン情報 */
		case IDM_ABOUT:
			OnAbout(hWnd);
			return TRUE;
	}

	return FALSE;
}

/*-[ メニューコマンド処理 ]-------------------------------------------------*/

/*
 *	メニューコマンド処理
 */
void FASTCALL OnCommand(HWND hWnd, WORD wID)
{
	ASSERT(hWnd);

	if (OnFile(hWnd, wID)) {
		return;
	}
	if (OnDisk(wID)) {
		return;
	}
	if (OnTape(wID)) {
		return;
	}
	if (OnView(hWnd, wID)) {
		return;
	}
	if (OnDebug(hWnd, wID)) {
		return;
	}
	if (OnTool(hWnd, wID)) {
		return;
	}
	if (OnWindow(wID)) {
		return;
	}
	if (OnHelp(hWnd, wID)) {
		return;
	}
}

/*
 *	メニューコマンド更新処理
 */
void FASTCALL OnMenuPopup(HWND hWnd, HMENU hSubMenu, UINT uPos)
{
	HMENU hMenu;

	ASSERT(hWnd);
	ASSERT(hSubMenu);

	/* メインメニューの更新かチェック */
	hMenu = GetMenu(hWnd);
	if (GetSubMenu(hMenu, uPos) != hSubMenu) {
		return;
	}

	/* ロックが必要 */
	LockVM();

	switch (uPos) {
		/* ファイル */
		case 0:
			OnFilePopup(hSubMenu);
			break;

		/* ドライブ1 */
		case 1:
			OnDiskPopup(hSubMenu, 1);
			break;

		/* ドライブ0 */
		case 2:
			OnDiskPopup(hSubMenu, 0);
			break;

		/* テープ */
		case 3:
			OnTapePopup(hSubMenu);
			break;

		/* 表示 */
		case 4:
			OnViewPopup(hSubMenu);
			break;

		/* デバッグ */
		case 5:
			OnDebugPopup(hSubMenu);
			break;

		/* ツール */
		case 6:
			OnToolPopup(hSubMenu);
			break;

		/* ウィンドウ */
		case 7:
			OnWindowPopup(hSubMenu);
			break;
	}

	/* アンロック */
	UnlockVM();
}

/*-[ ドラッグ＆ドロップ・コマンドライン処理 ]-------------------------------*/

/*
 *	ファイルドロップサブ
 */
void FASTCALL OnDropSub(char *path)
{
	char dir[_MAX_DIR];
	char ext[_MAX_EXT];
	char InitDir[_MAX_DRIVE + _MAX_PATH];

	ASSERT(path);

	/* 拡張子だけ分離 */
	_splitpath(path, InitDir, dir, NULL, ext);
	strncat(InitDir, dir, sizeof(InitDir) - strlen(InitDir) - 1);

	/* D77 */
	if (stricmp(ext, ".D77") == 0) {
		strncpy(InitialDir[0], InitDir, sizeof(InitialDir[0]));
		LockVM();
		StopSnd();
		fdc_setdisk(0, path);
		fdc_setdisk(1, NULL);
		if ((fdc_ready[0] != FDC_TYPE_NOTREADY) && (fdc_medias[0] >= 2)) {
			fdc_setdisk(1, path);
			fdc_setmedia(1, 1);
		}
		system_reset();
		OnRefresh(hMainWnd);
		PlaySnd();
		ResetSch();
		UnlockVM();
	}

	/* 2D/2DD/VFD */
#if XM7_VER >= 3
	if ((stricmp(ext, ".2D") == 0) || (stricmp(ext, ".2DD") == 0) ||
		(stricmp(ext, ".VFD") == 0)) {
#else
	if ((stricmp(ext, ".2D") == 0) || (stricmp(ext, ".VFD") == 0)) {
#endif
		strncpy(InitialDir[0], InitDir, sizeof(InitialDir[0]));
		LockVM();
		StopSnd();
		fdc_setdisk(0, path);
		fdc_setdisk(1, NULL);
		system_reset();
		OnRefresh(hMainWnd);
		PlaySnd();
		ResetSch();
		UnlockVM();
	}

	/* T77 */
	if (stricmp(ext, ".T77") == 0) {
		strncpy(InitialDir[1], InitDir, sizeof(InitialDir[0]));
		LockVM();
		tape_setfile(path);
		UnlockVM();
	}

#if XM7_VER == 1
#if defined(BUBBLE)
	if (bmc_enable) {
		/* B77 */
		if (stricmp(ext, ".B77") == 0) {
			strncpy(InitialDir[5], InitDir, sizeof(InitialDir[5]));
			LockVM();
			StopSnd();
			if (bmc_enable) {
				bmc_setfile(0, path);
				bmc_setfile(1, NULL);
				if (bmc_ready[0] != BMC_TYPE_NOTREADY) {
					if (bmc_medias[0] >= 2) {
						bmc_setfile(1, path);
						bmc_setmedia(1, 1);
					}
				}
			}
			PlaySnd();
			ResetSch();
			UnlockVM();
		}
	}

	/* BBL */
	if (stricmp(ext, ".BBL") == 0) {
		strncpy(InitialDir[5], InitDir, sizeof(InitialDir[5]));
		LockVM();
		bmc_setfile(0, path);
		bmc_setfile(1, NULL);
		UnlockVM();
	}
#endif
#endif

	/* XM7 */
	if (stricmp(ext, ".XM7") == 0) {
		strncpy(InitialDir[2], InitDir, sizeof(InitialDir[2]));
		LockVM();
		StopSnd();
		StateLoad(path);
		GetCfg();
		PlaySnd();
		ResetSch();
		UnlockVM();
	}

	/* 表示内容を更新 */
	if (hDrawWnd) {
		InvalidateRect(hDrawWnd, NULL, FALSE);
	}
}

/*
 *	ステータスバー
 *	ファイルドロップサブ
 */
void FASTCALL OnBarDropSub(char *path, POINT point)
{
	char dir[_MAX_DIR];
	char ext[_MAX_EXT];
	char InitDir[_MAX_DRIVE + _MAX_PATH];
	int drive;

	ASSERT(path);

	/* 拡張子だけ分離 */
	_splitpath(path, InitDir, dir, NULL, ext);
	strncat(InitDir, dir, sizeof(InitDir) - strlen(InitDir) - 1);

	/* ディスクイメージ */
	if ((point.x >= uPaneX[0]) && (point.x < uPaneX[2])) {
		/* ドライブ決定 */
		if (point.x >= uPaneX[1]) {
			drive = 0;
		}
		else {
			drive = 1;
		}

		/* D77 */
		if (stricmp(ext, ".D77") == 0) {
			strncpy(InitialDir[0], InitDir, sizeof(InitialDir[0]));
			LockVM();
			StopSnd();
			fdc_setdisk(drive, path);
			PlaySnd();
			ResetSch();
			UnlockVM();
			return;
		}

		/* 2D/2DD/VFD */
#if XM7_VER >= 3
		if ((stricmp(ext, ".2D") == 0) || (stricmp(ext, ".2DD") == 0) ||
			(stricmp(ext, ".VFD") == 0)) {
#else
		if ((stricmp(ext, ".2D") == 0) || (stricmp(ext, ".VFD") == 0)) {
#endif
			strncpy(InitialDir[0], InitDir, sizeof(InitialDir[0]));
			LockVM();
			StopSnd();
			fdc_setdisk(drive, path);
			PlaySnd();
			ResetSch();
			UnlockVM();
			return;
		}
	}

	/* ディスクイメージ以外・範囲外 */
	OnDropSub(path);
}

/*
 *	ファイルドロップ
 */
void FASTCALL OnDropFiles(HANDLE hDrop)
{
	char path[_MAX_PATH];
	POINT point;
	POINT spoint;
	HWND hwnd;

	ASSERT(hDrop);

	/* ファイル名受け取り */
	DragQueryPoint(hDrop, &point);
	DragQueryFile(hDrop, 0, path, sizeof(path));
	DragFinish(hDrop);

	/* 処理 */
	spoint = point;
	ClientToScreen(hMainWnd, &spoint);
	hwnd = WindowFromPoint(spoint);
	if (hwnd == hStatusBar) {
		/* ステータスバー上へのドロップ */
		OnBarDropSub(path, point);
	}
	else {
		/* ドローウィンドウ内へのドロップは従来処理 */
		OnDropSub(path);
	}
}

/*
 *	コマンドライン処理
 */
void FASTCALL OnCmdLine(LPSTR lpCmdLine)
{
	char dir[_MAX_DIR];
	char ext[_MAX_EXT];
	char path[_MAX_PATH];
	char fullpath[_MAX_PATH];
	char d77_path[_MAX_PATH];
	char InitDir[_MAX_PATH];
	LPSTR p;
	LPSTR q;
	BOOL flag;
	int drive;
#if XM7_VER == 1
#if defined(BUBBLE)
	int unit;
	char bbl_path[_MAX_PATH];
#endif
#endif
	BOOL tape_set;

	ASSERT(lpCmdLine);

	/* ワーク初期化 */
	drive = 0;
#if XM7_VER == 1
#if defined(BUBBLE)
	unit = 0;
#endif
#endif
	tape_set = FALSE;
	d77_path[0] = '\0';

	/* ファイル名をスキップ */
	p = lpCmdLine;
	flag = FALSE;
	for (;;) {
		/* 終了チェック */
		if (*p == '\0') {
			return;
		}

		/* クォートチェック */
		if (*p == '"') {
			flag = !flag;
		}

		/* スペースチェック */
		if ((*p == ' ') && !flag) {
			break;
		}

		/* 次へ */
		p++;
	}

	/* VMをロック */
	LockVM();
	StopSnd();

	/* コマンドラインオプションが終わるまでループ */
	while (*p) {
		path[0] = '\0';

		/* スペースを読み飛ばす */
		for (;;) {
			if (*p != ' ') {
				break;
			}

			/* 次へ */
			p++;
		}

		if (*p == '"') {
			/* クォートチェック */
			p++;
			q = path;

			/* クォートが出るまで続ける */
			for (;;) {
				*q = *p++;

				/* クォート内終了チェック */
				if (*q == '\0') {
					path[0] = '\0';
					break;
				}

				/* クォートチェック */
				if (*q == '"') {
					*q = '\0';
					break;
				}

				/* 次へ */
				q++;
			}
		}
		else if (*p) {
			/* クォートなし */
			q = path;

			/* スペースが出るか文字列が終了するまで続ける */
			for (;;) {
				*q = *p++;

				/* 区切り文字チェック */
				if ((*q == '\0') || (*q == ' ')) {
					*q = '\0';
					break;
				}

				/* 次へ */
				q++;
			}
		}

		/* 終了チェック */
		if (!path[0]) {
			break;
		}

		/* フルパスを生成 */
		_fullpath(fullpath, path, _MAX_PATH);

		/* 拡張子だけ分離 */
		_splitpath(fullpath, InitDir, dir, NULL, ext);
		strncat(InitDir, dir, sizeof(InitDir) - strlen(InitDir) - 1);

		/* D77/2D/2DD/VFD */
#if XM7_VER >= 3
		if ((stricmp(ext, ".D77") == 0) || (stricmp(ext, ".VFD") == 0) ||
			(stricmp(ext, ".2D") == 0) || (stricmp(ext, ".2DD") == 0)) {
#else
		if ((stricmp(ext, ".D77") == 0) || (stricmp(ext, ".VFD") == 0) ||
			(stricmp(ext, ".2D") == 0)) {
#endif

			/* 2枚までマウント可能 */
			if (drive < 2) {
				if (drive == 0) {
					/* 1枚目の場合、初期パスを保存 */
					strncpy(InitialDir[0], InitDir, sizeof(InitialDir[0]));

					/* D77を1つだけ指定した場合のためにファイル名を保存 */
					if (stricmp(ext, ".D77") == 0) {
						strncpy(d77_path, fullpath, sizeof(d77_path));
					}
				}

				/* ディスクをマウントする */
				fdc_setdisk(drive, fullpath);

				/* マウント成功ならドライブ番号(=マウント枚数)を+1 */
				if (fdc_ready[drive] != FDC_TYPE_NOTREADY) {
					drive ++;
				}
			}
		}

		/* T77 */
		if (stricmp(ext, ".T77") == 0) {
			/* 有効なのは1つのみ */
			if (!tape_set) {
				/* 初期パスを保存 */
				strncpy(InitialDir[1], InitDir, sizeof(InitialDir[1]));

				/* テープをマウント */
				tape_setfile(fullpath);

				/* マウント成功ならテープマウント禁止フラグを立てる */
				if (tape_fileh != -1) {
					tape_set = TRUE;
				}
			}
		}

#if XM7_VER == 1
#if defined(BUBBLE)
		/* B77/BBL */
		if (((stricmp(ext, ".B77") == 0) || (stricmp(ext, ".BBL") == 0)) &&
			bmc_enable) {

			if (unit == 0) {
				/* 1個目の場合、初期パスを保存 */
				strncpy(InitialDir[5], InitDir, sizeof(InitialDir[5]));

				/* BBLを1つだけ指定した場合のためにファイル名を保存 */
				if (stricmp(ext, ".BBL") == 0) {
					strncpy(bbl_path, fullpath, sizeof(bbl_path));
				}

				/* ユニット1のマウントを解除 */
				bmc_setfile(0, NULL);
				bmc_setfile(1, NULL);
			}

			/* 2個までマウント可能 */
			if (unit < 2 && bmc_enable) {
				/* バブルカセットをマウントする */
				bmc_setfile(unit, path);

				/* マウント成功ならユニット番号(=マウント個数)を+1 */
				if (bmc_ready[unit] != BMC_TYPE_NOTREADY) {
					unit ++;
				}
			}
		}
#endif
#endif

		/* XM7 */
		if (stricmp(ext, ".XM7") == 0) {
			strncpy(InitialDir[2], InitDir, sizeof(InitialDir[2]));
			StateLoad(fullpath);
			GetCfg();

			/* ステートロード時にマウントが行われるので処理を打ち切る */
			drive = 0;
#if XM7_VER == 1
#if defined(BUBBLE)
			unit = 0;
#endif
#endif
			break;
		}
	}

	/* ディスクイメージファイル指定数が1つの場合の特殊処理 */
	if (drive == 1) {
		/* ドライブ1のマウントを解除 */
		fdc_setdisk(1, NULL);

		/* D77の場合、2枚目があればドライブ1にマウント */
		if (d77_path[0]) {
			if ((fdc_ready[0] != FDC_TYPE_NOTREADY) && (fdc_medias[0] >= 2)) {
				fdc_setdisk(1, d77_path);
				fdc_setmedia(1, 1);
			}
		}
	}

#if XM7_VER == 1
#if defined(BUBBLE)
	/* バブルイメージファイル指定数が1つの場合の特殊処理 */
	if (unit == 1) {
		/* B77の場合、2個目があればドライブ1にマウント */
		if (bbl_path[0]) {
			if ((bmc_ready[0] != BMC_TYPE_NOTREADY) && (bmc_medias[0] >= 2)) {
				bmc_setfile(1, bbl_path);
				bmc_setmedia(1, 1);
			}
		}
	}
#endif
#endif

	/* ディスクイメージがマウントされた場合、VMリセット */
	if (drive >= 1) {
		system_reset();
		OnRefresh(hMainWnd);
	}

	/* VMをアンロック */
	PlaySnd();
	ResetSch();
	UnlockVM();

	/* 表示内容を更新 */
	if (hDrawWnd) {
		InvalidateRect(hDrawWnd, NULL, FALSE);
	}
}

#endif	/* _WIN32 */
