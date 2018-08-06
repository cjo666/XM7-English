/*
 *	FM-7 EMULATOR "XM7"
 *
 *	Copyright (C) 1999-2017 ＰＩ．(Twitter:@xm6_original)
 *	Copyright (C) 2001-2017 Ryu Takegami (Twitter:@RyuTakegami)
 *
 *	[ Win32API ]
 */

#ifdef _WIN32

#ifndef _w32_h_
#define _w32_h_

#ifndef __STDLIB_H
#include <stdlib.h>
#endif

/*
 *	定数、型定義
 */
#ifndef WM_THEMECHANGED
#define WM_THEMECHANGED		0x031A		/* WindowsXP テーマ適用メッセージ */
#endif
#ifndef	WM_MOUSEWHEEL
#define	WM_MOUSEWHEEL		0x020a		/* VS2005用(_WIN32_WINDOWSの代わり) */
#endif

#ifdef __cplusplus
extern "C" {
#endif
/*
 *	主要エントリ
 */
HFONT FASTCALL CreateTextFont(void);
										/* テキストフォント作成 */
void FASTCALL LockVM(void);
										/* VMロック */
void FASTCALL UnlockVM(void);
										/* VMアンロック */
void FASTCALL OnSize(HWND hWnd, WORD cx, WORD cy);
										/* サイズ変更 */
void FASTCALL OnCommand(HWND hWnd, WORD wID);
										/* WM_COMMAND */
void FASTCALL OnMenuPopup(HWND hWnd, HMENU hMenu, UINT uPos);
										/* WM_INITMENUPOPUP */
void FASTCALL OnDropFiles(HANDLE hDrop);
										/* WM_DROPFILES */
void FASTCALL OnCmdLine(LPSTR lpCmdLine);
										/* コマンドライン */
void FASTCALL OnAbout(HWND hWnd);
										/* バージョン情報 */
void FASTCALL OnRefresh(HWND hWnd);
										/* 最新の情報に更新(R) */
void FASTCALL EnableMenuSub(HMENU hMenu, UINT uID, BOOL bEnable);
										/* メニューEnable */
void FASTCALL CheckMenuSub(HMENU hMenu, UINT uID, BOOL bCheck);
										/* メニューCheck */
void FASTCALL InitIMEDLL(void);
										/* IME有効/無効API DLL初期化 */
void FASTCALL CleanIMEDLL(void);
										/* IME有効/無効API DLLクリーンアップ */
BOOL FASTCALL EnableIME(HWND hWnd, BOOL flag);
										/* IME有効/無効切り換え */
BOOL FASTCALL FileSelectSub(BOOL bOpen, UINT uFilterID, char *path, char *defext, BYTE IniDirNo);
										/* ファイル選択コモンダイアログ */
#if defined(MOUSE)
void FASTCALL MouseModeChange(BOOL flag);
										/* マウスキャプチャ切り替え */
#endif

/*
 *	主要ワーク
 */
#if XM7_VER == 1
#if defined(BUBBLE)
extern char InitialDir[6][_MAX_DRIVE + _MAX_PATH];
										/* 初期ディレクトリ */
#else
extern char InitialDir[5][_MAX_DRIVE + _MAX_PATH];
										/* 初期ディレクトリ */
#endif
#else
extern char InitialDir[5][_MAX_DRIVE + _MAX_PATH];
										/* 初期ディレクトリ */
#endif
extern HINSTANCE hAppInstance;
										/* アプリケーション インスタンス */
extern HWND hMainWnd;
										/* メインウインドウ */
extern HWND hDrawWnd;
										/* 描画ウインドウ */
extern int nErrorCode;
										/* エラーコード */
extern BOOL bMenuLoop;
										/* メニューループ中 */
extern BOOL bMenuExit;
										/* メニュー抜け出しフラグ */
extern BOOL bCloseReq;
										/* 終了要求フラグ */
extern LONG lCharWidth;
										/* キャラクタ横幅 */
extern LONG lCharHeight;
										/* キャラクタ縦幅 */
extern BOOL bSync;
										/* 実行に同期 */
extern BOOL bSyncDisasm[4];
										/* 逆アセンブルをPCに同期 */
extern BOOL bMinimize;
										/* 最小化フラグ */
extern BOOL bActivate;
										/* アクティベートフラグ */
extern BOOL bHideStatus;
										/* ステータスバーを隠す */
extern HICON hAppIcon;
										/* アイコンハンドル */
extern int nAppIcon;
										/* アイコン番号(1～3) */
extern BOOL bNTflag;
										/* 動作OSタイプ1(NT) */
extern BOOL bXPflag;
										/* 動作OSタイプ2(XP) */
extern BOOL bVistaflag;
										/* 動作OSタイプ3(Vista) */
extern BOOL bWin7flag;
										/* 動作OSタイプ4(Windows 7) */
extern BOOL bWin8flag;
										/* 動作OSタイプ4(Windows 8) */
extern BOOL bWin10flag;
										/* 動作OSタイプ3(Vista/7) */
extern BOOL bMMXflag;
										/* MMX対応フラグ */
extern BOOL bCMOVflag;
										/* CMOV対応フラグ */
extern BOOL bHighPriority;
										/* ハイプライオリティフラグ */
extern POINT WinPos;
										/* ウィンドウ位置 */
extern BOOL bOFNCentering;
										/* ファイルダイアログのセンタリング */
#if defined(ROMEO)
extern BOOL bRomeo;
										/* ROMEO認識フラグ */
#endif
#if ((XM7_VER <= 2) && defined(FMTV151))
extern BOOL bFMTV151;
										/* チャンネルコールエミュレーション */
#endif
extern HFONT hFont;
										/* サブウィンドウ用フォントハンドル */
#if XM7_VER >= 3
extern BOOL bGravestone;
										/* !? */
#endif
#if defined(KBDPASTE)
extern HWND hKeyStrokeDialog;
										/* キー入力支援ダイアログハンドル */
extern BOOL bKeyStrokeModeless;
										/* キー入力支援ダイアログモードレスフラグ */
#endif
#ifdef __cplusplus
}
#endif

#endif	/* _w32_h_ */
#endif	/* _WIN32 */
