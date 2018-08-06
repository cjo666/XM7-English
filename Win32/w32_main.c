/*
 *	FM-7 EMULATOR "XM7"
 *
 *	Copyright (C) 1999-2017 ＰＩ．(Twitter:@xm6_original)
 *	Copyright (C) 2001-2017 Ryu Takegami (Twitter:@RyuTakegami)
 *
 *	[ Win32API メインプログラム ]
 *
 *	RHG履歴
 *	  2001.12.25		DrawウィンドウもIMEを禁止するよう変更(ＰＩ．)
 *						サブウィンドウフォントのパラメータを変更(ＰＩ．)
 *	  2002.01.24		終了時の処理順序を「コンポーネントのクリーンアップ→
 *						設定ファイルの保存」に変更
 *	  2002.04.05		起動時にサイズ補正前のウィンドウが見えないように修正
 *	  2002.04.25		WinNT系で二重起動時のコマンドラインパラメータが先に起
 *						動しているインスタンスに渡せない問題を修正
 *	  2002.05.01		従来バージョンとの互換性確保のためWin9x系でのコマンド
 *						ライン渡しをWM_USER+1を使った方法に戻す
 *	  2002.08.15		一部キーについてWM_KEYDOWNでのキー押下状況取得に対応
 *	  2002.08.29		アクティブ化時にウィンドウの再描画を行うように変更
 *						(某プロジェクト用カスタム版から逆輸入 ^^;)
 *	  2002.09.11		ステータスメッセージ消去処理にシステムタイマーを利用す
 *						るように変更
 *	  2002.09.17		NT系でサブウィンドウを復元した場合に一部のキーが効かな
 *						くなる問題を修正
 *						メインウィンドウ削除時にサブウィンドウも削除するように
 *						変更
 *	  2002.11.13		スレッド優先順位をABOVE NORMALに変更
 *	  2002.12.11		DLLを開放し忘れていた問題を修正(………)
 *	  2003.01.19		F10潰しをF10にキーコードが割り当てられている時のみ行う
 *						ように変更
 *	  2003.01.23		ROMEO判定フラグがコケる問題を修正
 *	  2003.02.10		V2憑きで起動直後にチャンネルコール設定が設定ダイアロ
 *						グに反映されない問題を修正
 *	  2003.04.22		NT対策のBreak処理にシステムタイマを利用するように変更
 *	  2003.05.11		DLL読み込み処理の順序の問題でビジュアルスタイル適用チ
 *						ェックに失敗する問題を修正(XP only)
 *	  2003.05.25		Win2000/XPでシステムカラー変更時にステータスバーの色
 *						が変わらないことがある問題を修正
 *						画面デザイン変更時のステータスバーのサイズ変更に対応
 *	  2003.07.25		ウィンドウ位置の保存・復元に対応
 *	  2003.09.23		NT系でSHIFT+CAPが入力できない問題を修正
 *	  2004.03.29		PC-9801キーボードでカナキーが使用できない問題を修正
 *	  2004.05.03		IMEサポート関数のDLL読み込みチェックの安全性を強化
 *	  2004.08.30		起動直後のエラーメッセージ表示中に二重起動できてしま
 *						う問題に対策
 *						仮想マシン・DirectXの初期化失敗時にウィンドウを表示し
 *						ないように変更
 *	  2004.09.13		起動時のウインドウサイズ決定にAdjustWindowRectを利用
 *						するように変更
 *	  2004.10.20		NT系でSHIFT+カタカナ/ひらがながきかない問題を修正
 *	  2005.10.15		起動直後にマウスキャプチャが有効にならない問題を修正
 *	  2010.01.14		Vista以降での右シフトキー問題への対策を行った
 *	  2010.01.20		WindowsVista/Windows7で逆アセンブルウインドウを開くと
 *						ハングする問題の解消のためサブウインドウ描画を独立化
 *	  2010.02.18		XM7実行中は固定キー機能を無効化するように変更
 *	  2010.06.19		最小化時にキー入力を受け付ける問題を修正
 *	  2010.08.02		キーボードポーリング処理をw32_sch.cから移動
 *	  2010.10.03		2倍拡大表示モードに対応
 *	  2011.01.26		ステータスバー非表示時にウィンドウが不定長にのびる問題
 *						を修正
 *	  2012.03.06		メインループ処理を変更
 *	  2012.05.01		フルスクリーン状態保存処理への対応
 *	  2012.06.03		ステータスバーの表示状態の保存に対応
 *	  2012.07.14		多言語対応用外部DLLに対応
 *	  2012.10.10		Windows 8でのDrawMenuBar APIの挙動変更に対策
 *						起動時のOSバージョンチェックを厳密化
 *	  2014.06.28		多言語対応用外部DLLへの対応を廃止
 *	  2017.06.17		突拍子もなく多言語対応用外部DLLへの対応を再開
 *	  2017.11.24		多言語対応用外部DLLへの対応を完全に廃止
 */

#ifdef _WIN32

#define STRICT
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <commctrl.h>
#include <imm.h>
#include <shellapi.h>
#include <stdlib.h>
#include <assert.h>
#include <mmsystem.h>
#define DIRECTINPUT_VERSION		0x0300		/* DirectX3を指定 */
#include <dinput.h>
#include "xm7.h"
#include "mouse.h"
#include "tapelp.h"
#include "keyboard.h"
#include "w32.h"
#include "w32_bar.h"
#include "w32_draw.h"
#include "w32_dd.h"
#include "w32_gdi.h"
#include "w32_kbd.h"
#include "w32_sch.h"
#include "w32_snd.h"
#include "w32_res.h"
#include "w32_sub.h"
#include "w32_cfg.h"
#include "w32_comm.h"

#if defined(ROMEO)
#include "juliet.h"
#endif
#if defined(MIDI)
#include "w32_midi.h"
#endif


/*
 *	グローバル ワーク
 */
HINSTANCE hAppInstance;					/* アプリケーション インスタンス */
HWND hMainWnd;							/* メインウインドウ */
HWND hDrawWnd;							/* ドローウインドウ */
int nErrorCode;							/* エラーコード */
BOOL bMenuLoop;							/* メニューループ中 */
BOOL bMenuExit;							/* メニュー抜け出しフラグ */
BOOL bCloseReq;							/* 終了要求フラグ */
LONG lCharWidth;						/* キャラクタ横幅 */
LONG lCharHeight;						/* キャラクタ縦幅 */
BOOL bSync;								/* 実行に同期 */
BOOL bSyncDisasm[4];					/* 逆アセンブルをPCに同期 */
BOOL bMinimize;							/* 最小化フラグ */
BOOL bActivate;							/* アクティベートフラグ */
BOOL bHideStatus;						/* ステータスバーを隠す */
HICON hAppIcon;							/* アイコンハンドル */
int nAppIcon;							/* アイコン番号(1,2,3) */
BOOL bNTflag;							/* 動作OSタイプ1(NT) */
BOOL bXPflag;							/* 動作OSタイプ2(XP) */
BOOL bVistaflag;						/* 動作OSタイプ3(Vista/7) */
BOOL bWin7flag;							/* 動作OSタイプ4(Windows 7) */
BOOL bWin8flag;							/* 動作OSタイプ5(Windows 8) */
BOOL bWin10flag;						/* 動作OSタイプ6(Windows 10) */
BOOL bMMXflag;							/* MMXサポートフラグ */
BOOL bCMOVflag;							/* CMOVサポートフラグ(現状未使用) */
BOOL bHighPriority;						/* ハイプライオリティフラグ */
POINT WinPos;							/* ウィンドウ位置 */
#if defined(ROMEO)
BOOL bRomeo;							/* ROMEO認識フラグ */
#endif
#if XM7_VER <= 2 && defined(FMTV151)
BOOL bFMTV151;							/* V2チャンネルコールフラグ */
#endif
HFONT hFont;							/* サブウィンドウ用フォントハンドル */

/*
 *	スタティック ワーク
 */
static CRITICAL_SECTION CSection;		/* クリティカルセクション */
static BOOL (WINAPI *WINNLSEnableIME)(HWND, BOOL);
										/* IME有効/無効設定用API関数 */
static HINSTANCE hInstDLL;				/* user32.dllのハンドル */

/*
 *	アセンブラ関数のためのプロトタイプ宣言
 */
extern BOOL CheckMMX(void);				/* MMX対応チェック */
extern BOOL CheckCMOV(void);			/* CMOV対応チェック(現在未使用) */

/*-[ IMEサポート ]----------------------------------------------------------*/

/*
 *	IME有効/無効設定API
 *	DLL初期化
 */
void FASTCALL InitIMEDLL(void)
{
	/* user32.dllをロード */
	hInstDLL = LoadLibrary("user32.dll");

	/* 関数の先頭アドレスを設定 */
	if (hInstDLL) {
		WINNLSEnableIME = (BOOL (WINAPI *)(HWND, BOOL))GetProcAddress(
			hInstDLL, "WINNLSEnableIME");

		if (!WINNLSEnableIME) {
			/* 失敗 */
			CleanIMEDLL();
		}
	}
}

/*
 *	IME有効/無効設定API
 *	DLLクリーンアップ
 */
void FASTCALL CleanIMEDLL(void)
{
	/* DLLが読み込まれていたら開放 */
	if (hInstDLL) {
		FreeLibrary(hInstDLL);
	}

	WINNLSEnableIME = NULL;
	hInstDLL = NULL;
}

/*
 *	IME有効/無効切り換え 09/09
 */
BOOL FASTCALL EnableIME(HWND hWnd, BOOL flag)
{
	/* WINNLSEnableIME関数が読み込めている場合は実行 */
	if (hInstDLL && WINNLSEnableIME) {
		return WINNLSEnableIME(hWnd, flag);
	}

	/* 引数をそのまま返り値とする */
	return flag;
}

/*-[ サブウィンドウサポート ]-----------------------------------------------*/

/*
 *	テキストフォント作成
 *	※呼び出し元でDeleteObjectすること
 */
HFONT FASTCALL CreateTextFont(void)
{
	HFONT hFont;

	/* 言語判定して、シフトJIS・ANSIどちらかのフォントを作る */
	if (PRIMARYLANGID(GetUserDefaultLangID()) == LANG_JAPANESE) {
		/* 日本語 */
		hFont = CreateFont(-12, 0, 0, 0, FW_DONTCARE, FALSE, FALSE, FALSE,
			SHIFTJIS_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
			DEFAULT_QUALITY, FIXED_PITCH, "MS Gothic");
	}
	else {
		/* 英語 */
		hFont = CreateFont(-11, 0, 0, 0, FW_DONTCARE, FALSE, FALSE, FALSE,
			ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
			DEFAULT_QUALITY, FIXED_PITCH, NULL);
	}

	ASSERT(hFont);
	return hFont;
}

/*
 *	サブウィンドウセットアップ
 */
static void FASTCALL SetupSubWnd(HWND hWnd)
{
	HFONT hBackup;
	HDC hDC;
	TEXTMETRIC tm;

	ASSERT(hWnd);

	/* ウインドウワークを初期化 */
	memset(hSubWnd, 0, sizeof(hSubWnd));
	memset(bShowSubWindow, 0, sizeof(bShowSubWindow));
	InitSubWndWork();

	/* DC取得、フォントセレクト */
	hDC = GetDC(hWnd);
	ASSERT(hDC);
	hBackup = SelectObject(hDC, hFont);
	ASSERT(hBackup);

	/* テキストメトリック取得 */
	GetTextMetrics(hDC, &tm);

	/* DCクリーンアップ */
	SelectObject(hDC, hBackup);
	ReleaseDC(hWnd, hDC);

	/* 結果をストア */
	lCharWidth = tm.tmAveCharWidth;
	lCharHeight = tm.tmHeight + tm.tmExternalLeading;
}

/*-[ 同期 ]-----------------------------------------------------------------*/

/*
 *	VMをロック
 */
void FASTCALL LockVM(void)
{
	EnterCriticalSection(&CSection);
}

/*
 *	VMをアンロック
 */
void FASTCALL UnlockVM(void)
{
	LeaveCriticalSection(&CSection);
}

/*-[ ドローウインドウ ]-----------------------------------------------------*/

/*
 * 	ウインドウプロシージャ
 */
static LRESULT CALLBACK DrawWndProc(HWND hWnd, UINT message,
								 WPARAM wParam,LPARAM lParam)
{
	/* メッセージ別 */
	switch (message) {
		/* ウインドウ作成 */
		case WM_CREATE:
			/* IMEを禁止する */
			ImmAssociateContext(hWnd, (HIMC)NULL);
			break;

		/* ウインドウ背景描画 */
		case WM_ERASEBKGND:
			/* エラーなしなら、背景描画せず */
			if (nErrorCode == 0) {
				return 0;
			}
			break;

		/* ウインドウ再描画 */
		case WM_PAINT:
			/* ロックが必要 */
			LockVM();
			OnPaint(hWnd);
			UnlockVM();
			return 0;

		/* メニューチェック (メインウィンドウに対してメッセージを再発行する) */
		case WM_NCMOUSEMOVE:
		case WM_MOUSEMOVE:
			if (bWin8flag) {
				/* Windows 8だとどうも従来の処理でうまく行かないので */
				PostMessage(hMainWnd, message, wParam, lParam);
			}
			else {
				PostMessage(hMainWnd, message, wParam, 
					MAKELPARAM(LOWORD(lParam), (HIWORD(lParam) +
					GetSystemMetrics(SM_CYMENU))));
			}
			return 0;
	}

	/* デフォルト ウインドウプロシージャ */
	return DefWindowProc(hWnd, message, wParam, lParam);
}

/*
 *	ドローウインドウ作成
 */
static HWND FASTCALL CreateDraw(HWND hParent)
{
	WNDCLASSEX wcex;
	char szWndName[] = "XM7_Draw";
	RECT rect;
	HWND hWnd;

	ASSERT(hParent);

	/* 親ウインドウの矩形を取得 */
	GetClientRect(hParent, &rect);

	/* ウインドウクラスの登録 */
	memset(&wcex, 0, sizeof(wcex));
	wcex.cbSize = sizeof(wcex);
	wcex.style = CS_VREDRAW | CS_HREDRAW;
	wcex.lpfnWndProc = DrawWndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hAppInstance;
	wcex.hIcon = NULL;
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	wcex.lpszMenuName = NULL;
	wcex.lpszClassName = szWndName;
	wcex.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
	RegisterClassEx(&wcex);

	/* ウインドウ作成 */
	hWnd = CreateWindow(szWndName,
						szWndName,
						WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN,
						0,
						0,
						640,
						400,
						hParent,
						NULL,
						hAppInstance,
						NULL);

	/* 結果を持ち帰る */
	return hWnd;
}

/*-[ メインウインドウ ]-----------------------------------------------------*/

/*
 *	ウインドウ作成
 */
static void FASTCALL OnCreate(HWND hWnd)
{
	BOOL flag;

	ASSERT(hWnd);

	/* クリティカルセクション作成 */
	InitializeCriticalSection(&CSection);

	/* IMEを禁止する */
	ImmAssociateContext(hWnd, (HIMC)NULL);

	/* ファイルドロップ許可 */
	DragAcceptFiles(hWnd, TRUE);

	/* ワークエリア初期化 */
	nErrorCode = 0;
	bMenuLoop = FALSE;
	bCloseReq = FALSE;
	bSync = TRUE;
	bSyncDisasm[0] = TRUE;
	bSyncDisasm[1] = TRUE;
#if XM7_VER == 1
#if defined(JSUB)
	bSyncDisasm[2] = TRUE;
#endif
#if defined(Z80CARD)
	bSyncDisasm[3] = TRUE;
#endif
#endif
	bMinimize = FALSE;
	bActivate = FALSE;
	bHideStatus = FALSE;
	WinPos.x = -99999;
	WinPos.y = -99999;

	/* サブウィンドウ準備 */
	SetupSubWnd(hWnd);

	/* ROMEO開始処理 by うさ */
#if defined(ROMEO)
	juliet_load();
	if (juliet_prepare() == 0) {
		bRomeo = TRUE;
		juliet_YMF288Reset();
	}
	else {
		bRomeo = FALSE;
	}
#endif

	/* ドローウインドウ、ステータスバーを作成 */
	hDrawWnd = CreateDraw(hWnd);
	hStatusBar = CreateStatus(hWnd);

	/* コンポーネント初期化 */
	LoadCfg();
	InitDraw();
	InitSnd();
	InitKbd();
	InitSch();
#if defined(FDDSND)
	InitFDDSnd();
#endif

	/* 仮想マシン初期化 */
	if (!system_init()) {
		nErrorCode = 1;
		PostMessage(hWnd, WM_USER, 0, 0);
		return;
	}

	/* 直後、リセット */
	ApplyCfg();
	system_reset();

	/* コンポーネントセレクト */
	flag = TRUE;
	if (!SelectDraw(hDrawWnd)) {
		flag = FALSE;
	}
	if (!SelectSnd(hWnd)) {
		flag = FALSE;
	}
	if (!SelectKbd(hWnd)) {
		flag = FALSE;
	}
	if (!SelectSch()) {
		flag = FALSE;
	}

	/* エラーコードをセットさせ、スタート */
	if (!flag) {
		nErrorCode = 2;
	}
	PostMessage(hWnd, WM_USER, 0, 0);
}

/*
 *	ウインドウクローズ
 */
static void FASTCALL OnClose(HWND hWnd)
{
	int i;

	ASSERT(hWnd);

	/* サウンド停止 */
	StopSnd();

	/* メインウインドウを一度消す(タスクバー対策) */
	ShowWindow(hWnd, SW_HIDE);

	/* フラグアップ */
	LockVM();
	bCloseReq = TRUE;
	UnlockVM();

	/* サブウインドウを先に破棄 */
	for (i=0; i<SWND_MAXNUM; i++) {
		if (hSubWnd[i]) {
			DestroyWindow(hSubWnd[i]);
		}
	}
}

/*
 *	ウインドウ削除
 */
static void FASTCALL OnDestroy(void)
{
	/* サウンド停止 */
//	StopSnd();

	/* INIファイル書き込み */
	SaveCfg();

	/* コンポーネント クリーンアップ */
#if defined(MIDI)
	CleanMIDI();
#endif
#if defined(FDDSND)
	CleanFDDSnd();
#endif
#if defined(RSC)
	CleanCommPort();
#endif
	CleanSch();
	CleanKbd();
	CleanSnd();
	CleanDraw();
	
	/* ROMEO終了処理 by うさ */
#if defined(ROMEO)
	if (bRomeo) {
		juliet_YMF288Reset();
		Sleep(1);
	}
	juliet_unload();
	Sleep(1);
#endif

	/* クリティカルセクション削除 */
	DeleteCriticalSection(&CSection);

	/* 仮想マシン クリーンアップ */
	system_cleanup();
}

/*
 *	WM_QUIT処理
 */
void FASTCALL OnQuit(void)
{
	/* ウインドウハンドルをクリア */
	hMainWnd = NULL;
	hDrawWnd = NULL;
	hStatusBar = NULL;
}

/*
 *	サイズ変更
 */
void FASTCALL OnSize(HWND hWnd, WORD cx, WORD cy)
{
	RECT crect;
	RECT wrect;
	RECT trect;
	RECT srect;
	int width;
	int height;
	int wheight;

	ASSERT(hWnd);
	ASSERT(hDrawWnd);

	/* 最小化の場合は、何もしない */
	if ((cx == 0) && (cy == 0)) {
		return;
	}

	/* フルスクリーン時も同様 */
	if (bFullScreen) {
		return;
	}

	/* ツールバー、ステータスバーの有無を考慮に入れ、計算 */
	if (bDoubleSize) {
		width = 1280;
		height = 800;
	}
	else {
		width = 640;
		height = 400;
	}
	memset(&trect, 0, sizeof(trect));
	if (IsWindowVisible(hStatusBar)) {
		GetWindowRect(hStatusBar, &srect);
		wheight = height + (srect.bottom - srect.top);
	}
	else {
		memset(&srect, 0, sizeof(srect));
		wheight = height;
	}

	/* クライアント領域のサイズを取得 */
	GetClientRect(hWnd, &crect);

	/* 要求サイズと比較し、補正 */
	if ((crect.right != width) || (crect.bottom != wheight)) {
		GetWindowRect(hWnd, &wrect);
		wrect.right -= wrect.left;
		wrect.bottom -= wrect.top;
		wrect.right -= crect.right;
		wrect.bottom -= crect.bottom;
		wrect.right += width;
		wrect.bottom += wheight;
		MoveWindow(hWnd, wrect.left, wrect.top, wrect.right, wrect.bottom, TRUE);
	}

	/* メインウインドウ配置 */
	MoveWindow(hDrawWnd, 0, trect.bottom, width, height, TRUE);

	/* ステータスバー配置 */
	if (IsWindowVisible(hStatusBar)) {
		MoveWindow(hStatusBar, 0, trect.bottom + height, width,
					(srect.bottom - srect.top), TRUE);
		SizeStatus(width);
	}
}

/*
 *	キック(メニューには未定義)
 */
static void FASTCALL OnKick(HWND hWnd)
{
	char buffer[128];

	buffer[0] = '\0';

	/* エラーコード別 */
	switch (nErrorCode) {
		/* エラーなし */
		case 0:
			/* 実行開始 */
			stopreq_flag = FALSE;
			run_flag = TRUE;

			/* コマンドライン処理 */
			OnCmdLine(GetCommandLine());

#if defined(MOUSE)
			/* マウスキャプチャ開始 */
			SetMouseCapture(TRUE);
#endif
			break;

		/* VM初期化エラー */
		case 1:
			LoadString(hAppInstance, IDS_VMERROR, buffer, sizeof(buffer));
			MessageBox(hWnd, buffer, "XM7", MB_ICONSTOP | MB_OK);
			PostMessage(hWnd, WM_CLOSE, 0, 0);
			break;

		/* コンポーネント初期化エラー */
		case 2:
			LoadString(hAppInstance, IDS_DXERROR, buffer, sizeof(buffer));
			MessageBox(hWnd, buffer, "XM7", MB_ICONSTOP | MB_OK);
			PostMessage(hWnd, WM_CLOSE, 0, 0);
			break;
	}
}


/*
 * 	ウインドウプロシージャ
 */
static LRESULT CALLBACK WndProc(HWND hWnd, UINT message,
								 WPARAM wParam,LPARAM lParam)
{
	PAINTSTRUCT ps;
	BYTE key_code;
	RECT rect;

	/* メッセージ別 */
	switch (message) {
		/* ウインドウ作成 */
		case WM_CREATE:
			OnCreate(hWnd);
			break;

		/* ウィンドウクローズ */
		case WM_CLOSE:
			OnClose(hWnd);
			break;

		/* ウインドウ削除 */
		case WM_DESTROY:
			OnDestroy();
			PostQuitMessage(0);
			return 0;

		/* ウィンドウ移動 */
		case WM_MOVE:
			if (!bFullScreen && (!(GetWindowLong(hWnd, GWL_STYLE) &
				(WS_MAXIMIZE | WS_MINIMIZE)))) {
				GetWindowRect(hWnd, &rect);
				WinPos.x = rect.left;
				WinPos.y = rect.top;
			}
			break;

		/* ウインドウサイズ変更 */
		case WM_SIZE:
			/* 最小化時のキー入力対策 */
			if (wParam == SIZE_MINIMIZED) {
				bMinimize = TRUE;
			}
			else {
				bMinimize = FALSE;
			}
			OnSize(hWnd, LOWORD(lParam), HIWORD(lParam));
			break;

		/* ウインドウ再描画 */
		case WM_PAINT:
			/* ロックが必要 */
			LockVM();
			BeginPaint(hWnd, &ps);
			PaintStatus();
			EndPaint(hWnd, &ps);
			UnlockVM();
			return 0;

		/* ユーザ+0:スタート */
		case WM_USER + 0:
			OnKick(hWnd);
			return 0;

		/* ユーザ+1:コマンドライン(Win9x) */
		case WM_USER + 1:
			if (!bNTflag) {
				OnCmdLine((LPSTR)wParam);
			}
			break;

#if defined(MOUSE)
		/* ユーザ+2:マウスキャプチャOFF */
		case WM_USER + 2:
			SetMouseCapture(FALSE);
			break;

		/* ユーザ+3:マウスキャプチャON */
		case WM_USER + 3:
			SetMouseCapture(TRUE);
			break;
#endif

		/* コマンドライン(WinNT) */
		case WM_COPYDATA:
			if (bNTflag) {
				if (((PCOPYDATASTRUCT)lParam)->dwData == 0x01374d58) {
					OnCmdLine((LPSTR)((PCOPYDATASTRUCT)lParam)->lpData);
				}
			}
			break;

#ifndef DINPUT8
		case WM_KEYDOWN:
			if ((LOBYTE(HIWORD(lParam)) == 0x36) && bVistaflag) {
				/* Vista以降の右シフト対策 */
				bNTkeyPushFlag[KNT_RSHIFT] = TRUE;
				return 0;
			}
			break;
#endif

		case WM_SYSKEYUP:
		case WM_KEYUP:
			if ((LOBYTE(wParam) == VK_F10) && kbd_table[DIK_F10]) {
				/* F10潰し(^^; */
				return 0;
			}
			if ((LOBYTE(wParam) == VK_MENU) && !(lParam & 0x01000000) &&
				 kbd_table[DIK_LMENU]) {
				/* 左Alt潰し(^^; */
				return 0;
			}
			if ((LOBYTE(wParam) == VK_MENU) && (lParam & 0x01000000) &&
				 kbd_table[DIK_RMENU]) {
				/* 右Alt潰し(^^; */
				return 0;
			}
#ifndef DINPUT8
			if ((LOBYTE(HIWORD(lParam)) == 0x36) && bVistaflag) {
				/* Vista以降の右シフト対策 */
				bNTkeyPushFlag[KNT_RSHIFT] = FALSE;
				return 0;
			}
#endif
			if (((LOBYTE(wParam) == VK_MENU) || (LOBYTE(wParam) == VK_F10)) && 
				 bWin8flag) {
				/* Windows 8 対策処理 */
				if (bFullScreen) {
					if (!bMenuLoop) {
						EnterMenu(hWnd);
						bMenuLoop = TRUE;
					}
					else {
						ExitMenu();
						OnExitMenuLoop();
						bMenuLoop = FALSE;
					}
				}
			}
			break;

		/* メニューループ開始 */
		case WM_ENTERMENULOOP:
			if (!bMenuLoop && !bWin8flag) {
				EnterMenu(hWnd);
			}
			bMenuLoop = TRUE;
			break;

		/* メニューループ終了 */
		case WM_EXITMENULOOP:
			if (bMenuLoop && !bWin8flag) {
				ExitMenu();
				OnExitMenuLoop();
			}
			SetMenuExitTimer();
			bMenuLoop = FALSE;
			break;

		/* メニュー選択 */
		case WM_MENUSELECT:
			OnMenuSelect(wParam);
			break;

		/* メニューチェック */
		case WM_NCMOUSEMOVE:
		case WM_MOUSEMOVE:
			if (bFullScreen) {
				if (HIWORD(lParam) < GetSystemMetrics(SM_CYMENUSIZE)) {
					if (!bMenuLoop) {
						EnterMenu(hMainWnd);
						bMenuLoop = TRUE;
					}
				}
				else {
					if (bMenuLoop) {
						ExitMenu();
						OnExitMenuLoop();
						bMenuLoop = FALSE;
					}
				}
			}
			break;

		/* メニューポップアップ */
		case WM_INITMENUPOPUP:
			if (!HIWORD(lParam)) {
				OnMenuPopup(hWnd, (HMENU)wParam, (UINT)LOWORD(lParam));
				return 0;
			}
			break;

		/* メニューコマンド */
		case WM_COMMAND:
			EnterMenu(hWnd);
			OnCommand(hWnd, LOWORD(wParam));
			ExitMenu();
			return 0;

		/* ファイルドロップ */
		case WM_DROPFILES:
			OnDropFiles((HANDLE)wParam);
			return 0;

		/* アクティベート */
		case WM_ACTIVATE:
			if (LOWORD(wParam) == WA_INACTIVE) {
				bActivate = FALSE;
			}
			else {
				if (!bActivate) {
					InvalidateRect(hDrawWnd, NULL, FALSE);
				}
				bActivate = TRUE;
			}

#if defined(MOUSE)
			SetMouseCapture(bActivate);
#endif
			break;

		/* オーナードロー */
		case WM_DRAWITEM:
			if (wParam == ID_STATUS_BAR) {
				OwnerDrawStatus((LPDRAWITEMSTRUCT)lParam);
				return TRUE;
			}
			break;

		/* 画面デザイン変更 */
		case WM_SYSCOLORCHANGE:
			ResizeStatus(hWnd, hStatusBar);
			break;

		/* テーマ変更 */
		case WM_THEMECHANGED:
			if (bXPflag) {
				ChangeStatusBorder(hStatusBar);
			}
			break;

		/* タイマー */
		case WM_TIMER:
			if ((wParam >= 0x80) && (wParam < 0x100)) {
				/* キー開放 */
				key_code = (BYTE)(wParam & 0x7f);
				if (bNTkeyMakeFlag[key_code]) {
					keyboard_break(key_code);
					bNTkeyMakeFlag[key_code] = FALSE;
				}
				KillTimer(hMainWnd, wParam);
			}
			else switch (wParam) {
				case ID_STATUS_BAR :
					/* ステータスメッセージ消去 */
					if (bStatusMessage) {
						EndStatusMessage();
					}
					KillTimer(hMainWnd, ID_STATUS_BAR);
					break;
			}
			break;

		/* 電源管理 */
		case WM_POWERBROADCAST:
			timeEndPeriod(uTimerResolution);
			timeBeginPeriod(uTimerResolution);
			break;

		/* システムコマンド */
		case WM_SYSCOMMAND:
			/* スクリーンセーバ・モニタ節電モードを抑制 */
			if ((wParam == SC_SCREENSAVE) || (wParam == SC_MONITORPOWER)) {
				return 1;
			}
			break;
	}

	/* デフォルト ウインドウプロシージャ */
	return DefWindowProc(hWnd, message, wParam, lParam);
}

/*-[ アプリケーション ]-----------------------------------------------------*/

/*
 *	XM7ウインドウを検索、コマンドラインを渡す
 */
static BOOL FASTCALL FindXM7Wnd(void)
{
	COPYDATASTRUCT	cds;
	HWND hWnd;
	char string[128];

	/* ウインドウクラスで検索 */
	hWnd = FindWindow("XM7", NULL);
	if (!hWnd) {
		return FALSE;
	}

	/* テキスト文字列取得 */
	GetWindowText(hWnd, string, sizeof(string));
	string[4] = '\0';
	if (strcmp("XM7 ", string) != 0) {
		return FALSE;
	}

	/* メッセージを送信 */
	if (bNTflag) {
		/* NTではWM_COPYDATAを使う */
		cds.dwData = 0x01374d58;
		cds.lpData = (void *)GetCommandLine();
		cds.cbData = lstrlen(GetCommandLine()) + 1;
		SendMessage(hWnd, WM_COPYDATA, (WPARAM)NULL, (LPARAM)&cds);
	}
	else {
		/* Win9x/Meでは従来と同じ方法でメッセージ送信 */
		SendMessage(hWnd, WM_USER + 1, (WPARAM)GetCommandLine(), (LPARAM)NULL);
	}

	return TRUE;
}

/*
 *	インスタンス初期化
 */
static HWND FASTCALL InitInstance(HINSTANCE hInst, int nCmdShow)
{
	HWND hWnd;
	HMENU hMenu;
	WNDCLASSEX wcex;
	char szAppName[] = "XM7";
	char szWindowName[128];
	int xx, yy;
	RECT rect;

	ASSERT(hInst);

	/* ウインドウハンドルクリア */
	hMainWnd = NULL;
	hDrawWnd = NULL;
	hStatusBar = NULL;

	/* 初期ウィンドウサイズを計算 */
	rect.left = 0;
	rect.top = 0;
	if (LoadCfg_DoubleSize()) {
		rect.right = 1280;
		rect.bottom = 800;
	}
	else {
		rect.right = 640;
		rect.bottom = 400;
	}
	AdjustWindowRect(&rect, WS_OVERLAPPED | WS_SYSMENU | WS_CAPTION |
							WS_BORDER | WS_CLIPCHILDREN | WS_MINIMIZEBOX,
							TRUE);
	xx = rect.right - rect.left;
	yy = rect.bottom - rect.top;

	/* 初期アイコンをロード */
#if XM7_VER >= 3
	hAppIcon = LoadIcon(hAppInstance, MAKEINTRESOURCE(IDI_APPICON));
#elif XM7_VER >= 2
	hAppIcon = LoadIcon(hAppInstance, MAKEINTRESOURCE(IDI_AVICON));
#else
	hAppIcon = LoadIcon(hAppInstance, MAKEINTRESOURCE(IDI_APPICON));
#endif
	nAppIcon = 255;

	/* ウインドウクラスの登録 */
	memset(&wcex, 0, sizeof(wcex));
	wcex.cbSize = sizeof(wcex);
	wcex.style = CS_VREDRAW | CS_HREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInst;
	wcex.hIcon = hAppIcon;
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	wcex.lpszMenuName = MAKEINTRESOURCE(IDR_MAINMENU);
	wcex.lpszClassName = szAppName;
	wcex.hIconSm = hAppIcon;
	RegisterClassEx(&wcex);

	/* ウィンドウタイトルを作成 */
	strncpy(szWindowName, szAppName, sizeof(szWindowName));
	strncat(szWindowName, " ", sizeof(szWindowName) - strlen(szWindowName) - 1);

	/* メニューを作成 */
	hMenu = LoadMenu(hAppInstance, MAKEINTRESOURCE(IDR_MAINMENU));

	/* ウインドウ作成 */
	hWnd = CreateWindow(szAppName,
						szWindowName,
						WS_OVERLAPPED | WS_SYSMENU | WS_CAPTION | WS_BORDER |
						WS_CLIPCHILDREN | WS_MINIMIZEBOX,
						CW_USEDEFAULT,
						CW_USEDEFAULT,
						xx,
						yy,
						NULL,
						hMenu,
						hInst,
						NULL);
	if (!hWnd) {
		return NULL;
	}
	
	/* ウィンドウ位置設定 */
	if ((WinPos.x == -99999) && (WinPos.y == -99999)) {
		GetWindowRect(hWnd, &rect);
		WinPos.x = (GetSystemMetrics(SM_CXSCREEN) - rect.right) / 2;
		WinPos.y = (GetSystemMetrics(SM_CYSCREEN) - rect.bottom) / 2;
	}
	SetWindowPos(hWnd, HWND_NOTOPMOST, WinPos.x, WinPos.y, 0, 0, SWP_NOSIZE);
	OnSize(hWnd, 640, 400);

	/* ウインドウ表示 */
	if (nErrorCode == 0) {
		ShowWindow(hWnd, nCmdShow);
		UpdateWindow(hWnd);
	}

	return hWnd;
}

/*
 *	WinMain
 */
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
						LPSTR lpszCmdParam, int nCmdShow)
{
	MSG msg;
	HACCEL hAccel;
	OSVERSIONINFO osvi;
	STICKYKEYS sk;
	STICKYKEYS skb;
	char buffer[128];
	int osver;

	UNUSED(hPrevInstance);
	UNUSED(lpszCmdParam);

	/* COMを初期化 */
	if (FAILED(CoInitialize(NULL))) {
		LoadString(hInstance, IDS_COINITERROR, buffer, sizeof(buffer));
		MessageBox(NULL, buffer, "XM7", MB_ICONSTOP | MB_OK);

		return -1;
	}

	/* コモンコントロール初期化 */
	InitCommonControls();

	/* 動作しているOSがWinNT系か判定 */
	osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	GetVersionEx(&osvi);
	osver = osvi.dwMajorVersion * 100 + osvi.dwMinorVersion;

	/* NTチェック */
	if (osvi.dwPlatformId == VER_PLATFORM_WIN32_NT) {
		bNTflag = TRUE;
	}
	else {
		bNTflag = FALSE;
	}

	/* XPチェック */
	if (osver >= 501) {
		bXPflag = TRUE;
	}
	else {
		bXPflag = FALSE;
	}

	/* Windows Vistaチェック */
	if (osver >= 600) {
		bVistaflag = TRUE;
	}
	else {
		bVistaflag = FALSE;
	}

	/* Windows 7チェック */
	if (osver >= 601) {
		bWin7flag = TRUE;
	}
	else {
		bWin7flag = FALSE;
	}

	/* Windows 8チェック */
	if (osver >= 602) {
		bWin8flag = TRUE;
	}
	else {
		bWin8flag = FALSE;
	}

	/* Windows 10チェック */
	if (osver >= 604) {
		bWin10flag = TRUE;
	}
	else {
		bWin10flag = FALSE;
	}

	/* XM7チェック、コマンドライン渡し */
	if (FindXM7Wnd()) {
		/* COMを終了 */
		CoUninitialize();

		return 0;
	}

	/* 各種フラグを取得・設定 */
	bMMXflag = CheckMMX();
	bCMOVflag = CheckCMOV();
#if defined(ROMEO)
	bRomeo = FALSE;
#endif
#if XM7_VER <= 2 && defined(FMTV151)
	bFMTV151 = TRUE;
#endif

	/* DLL初期化 */
	InitThemeDLL();
	InitIMEDLL();

	/* フォントハンドル取得 */
	hFont = CreateTextFont();

	/* インスタンスを保存、初期化 */
	hAppInstance = hInstance;
	hMainWnd = InitInstance(hInstance, nCmdShow);
	if (!hMainWnd) {
		CleanIMEDLL();
		CleanThemeDLL();
		DeleteObject(hFont);

		/* COMを終了 */
		CoUninitialize();

		return -1;
	}

	/* 固定キー機能の無効化 */
	sk.cbSize = sizeof(STICKYKEYS);
	SystemParametersInfo(SPI_GETSTICKYKEYS, sizeof(STICKYKEYS), &sk, 0);
	skb = sk;
	sk.dwFlags &= ~(SKF_HOTKEYACTIVE | SKF_AVAILABLE | SKF_STICKYKEYSON);
	SystemParametersInfo(SPI_SETSTICKYKEYS, sizeof(STICKYKEYS), &sk, 0);

	/* アクセラレータをロード */
	hAccel = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDR_ACCELERATOR));
	ASSERT(hAccel);

	/* メッセージ ループ */
	while (hMainWnd) {
		/* メッセージをチェックし、処理 */
		if (PeekMessage(&msg, 0, 0, 0, PM_NOREMOVE)) {
			if (!GetMessage(&msg, NULL, 0, 0)) {
				OnQuit();
				break;
			}

#if defined(MOUSE)
			/* 中央ボタンによるマウスモード切り替え */
			if ((msg.message == WM_MBUTTONDOWN) &&
				(msg.wParam == MK_MBUTTON) &&
				(uMidBtnMode == MOSCAP_WMESSAGE)) {
				/* 中央ボタンが押されたらマウスモードを切り換える */
				EnterMenu(hMainWnd);
				OnCommand(hMainWnd, IDM_MOUSEMODE);
				ExitMenu();
			}

			/* ホイールによるマウスモード切り替え(Unz方式) */
#if defined(WM_MOUSEWHEEL)
			if ((msg.message == WM_MOUSEWHEEL) &&
				(uMidBtnMode == MOSCAP_WHEELWM)) {
				if ((short)HIWORD(msg.wParam) > 0) {
					EnterMenu(hMainWnd);
					OnCommand(hMainWnd, IDM_MOUSEON);
					ExitMenu();
				}
				if ((short)HIWORD(msg.wParam) < 0) {
					EnterMenu(hMainWnd);
					OnCommand(hMainWnd, IDM_MOUSEOFF);
					ExitMenu();
				}
			}
#endif
#endif

			/* NTキー対策処理 */
			if (msg.message == WM_KEYDOWN) {
				switch (LOBYTE(msg.wParam)) {
					case VKNT_CAPITAL	:	/* CapsLock */
					case VK_CAPITAL		:	bNTkeyPushFlag[KNT_CAPS] = TRUE;
											break;
					case VKNT_KANA		:
					case VKNT_KATAKANA	:	/* カタカナ/ひらがな,カナ(PC-98) */
					case VK_KANA		:	bNTkeyPushFlag[KNT_KANA] = TRUE;
											break;
					case VKNT_KANJI1	:	/* 半角/全角 */
					case VKNT_KANJI2	:	bNTkeyPushFlag[KNT_KANJI] = TRUE;
											break;
				}
			}

#if defined(KBDPASTE)
			if (hKeyStrokeDialog) {
				if (IsDialogMessage(hKeyStrokeDialog, &msg)) {
					continue;
				}
			}
#endif

			if (!TranslateAccelerator(hMainWnd, hAccel, &msg)) {
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}

			continue;
		}

		/* フルスクリーン要求 */
		if (bFullRequested) {
			PostMessage(hMainWnd, WM_COMMAND, IDM_FULLSCREEN, 0);
			bFullRequested = FALSE;
		}

		/* ステータス描画＆スリープ */
		if (nErrorCode == 0) {
			/* Windows Vista/7の場合、ここでサブウィンドウを更新 */
			if (bVistaflag) {
				DrawSubWindow();
			}

			DrawStatus();
			if (bSync) {
				ReSizeOPNReg();
				ReSizeOPNDisp();
			}

			/* ちょっと一休み */
			Sleep(16);
		}
	}

	/* フォントハンドル解放 */
	DeleteObject(hFont);

	/* DLL開放 */
	CleanIMEDLL();
	CleanThemeDLL();

	/* 強制的にマウスクリップを解除 */
	ClipCursor(0);

	/* 固定キー機能の復元 */
	SystemParametersInfo(SPI_SETSTICKYKEYS, sizeof(STICKYKEYS), &skb,  0);

	/* COMを終了 */
	CoUninitialize();

	/* 終了 */
	return msg.wParam;
}

#endif	/* _WIN32 */
