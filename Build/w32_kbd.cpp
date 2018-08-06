/*
 *	FM-7 EMULATOR "XM7"
 *
 *	Copyright (C) 1999-2017 ＰＩ．(Twitter:@xm6_original)
 *	Copyright (C) 2001-2017 Ryu Takegami (Twitter:@RyuTakegami)
 *
 *	[ Win32API キーボード・ジョイスティック・マウス ]
 *
 *	RHG履歴
 *	  2001.12.19		マウス入力部(DirectInputバージョン)を統合
 *	  2002.01.05		WinNT系でのキー入力不具合への対策
 *	  2002.05.23		マウス中央ボタンでのマウスエミュレーション切換に対応
 *	  2002.06.19		ALTキーとの同時打鍵を無効化するようにした
 *	  2002.07.30		ジョイスティック/マウスのポーリング間隔を描画同期に変更
 *						0.5秒毎にジョイスティックの接続チェックを行うようにした
 *	  2002.08.15		XPでのDirectInputの理解不明な仕様変更(?)への対策を追加
 *	  2002.08.25		疑似リアルタイムキースキャンの処理を改善
 *						カーソルキーでのテンキーエミュレーション時に疑似リアル
 *						タイムキースキャンが効かない問題を修正
 *						諸事情でXP対策をNT系全てで使用するように変更
 *	  2002.12.25		マウスキャプチャ処理部を修正
 *	  2003.01.02		マウスキャプチャ終了時にカーソル位置の復元を追加
 *	  2003.01.12		起動直後にマウスボタンが押下状態になる問題を修正
 *	  2003.04.22		NT対策のBreak処理にシステムタイマを利用するように変更
 *	  2003.04.23		スキャンモード時のNT対策処理を改良
 *	  2003.05.27		BREAKキーを要対策キーに割り当てた場合の動作を変更
 *						キーマップ変更時のポーリング処理にNT対策処理を追加
 *	  2003.05.28		テンキー変換をカーソルキー同時押し状態から斜め方向の
 *						変換にも対応
 *	  2004.05.03		ホイールでのマウスキャプチャモード切り換えを導入
 *	  2010.01.14		Vista以降での右シフトキー問題への対策を行った
 *	  2010.01.19		キーアサイン時の右シフトキー問題への対策を行った
 *	  2012.05.14		DirectInput版のマウスモード切り換え処理を廃止
 *	  2015.02.03		DirectInput8に対応(VC++ only)
 *	  2017.03.19		キーボード種別自動判定が英語キーボードで正常に動作して
 *						いなかった問題を修正
 */

#ifdef _WIN32

#define STRICT
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#ifdef DINPUT8
#define DIRECTINPUT_VERSION		0x0800		/* DirectX8を指定 */
#else
#define DIRECTINPUT_VERSION		0x0300		/* DirectX3を指定 */
#endif
#include <dinput.h>
#include <mmsystem.h>
#include <assert.h>
#include "xm7.h"
#include "mainetc.h"
#include "keyboard.h"
#include "device.h"
#include "mouse.h"
#include "event.h"
#include "w32.h"
#include "w32_sch.h"
#include "w32_kbd.h"
#include "w32_bar.h"
#include "w32_res.h"
#include "w32_draw.h"

/*
 *	グローバル ワーク
 */
BYTE kbd_map[256];							/* キーボード マップ */
BYTE kbd_table[256];						/* 対応するFM-7物理コード */
int nJoyType[2];							/* ジョイスティックタイプ */
int nJoyRapid[2][2];						/* 連射タイプ */
int nJoyCode[2][7];							/* 生成コード */
BOOL bKbdReal;								/* 疑似リアルタイムキースキャン */
BOOL bTenCursor;							/* テンキー変換 */
BOOL bArrow8Dir;							/* テンキー変換 8方向モード */
BOOL bNTkeyPushFlag[4];						/* NT対策用キー押下フラグ */
BOOL bNTkeyMakeFlag[128];					/* NT対策用Make中フラグ */
BOOL bNTkbMode;								/* NT対策中フラグ */
#if defined(MOUSE)
BYTE uMidBtnMode;							/* 中央ボタン状態取得モード */
BOOL bDetectMouse;							/* マウス確認フラグ */
#endif
#if defined(KBDPASTE)
UINT uPasteWait;							/* ペースト待ち時間(ms) */
UINT uPasteWaitCntl;						/* ペースト待ち時間コントロールコード */
#endif

/*
 *	スタティック ワーク
 */
#ifdef DINPUT8
static LPDIRECTINPUT8 lpdi;					/* DirectInput */

static LPDIRECTINPUTDEVICE8 lpkbd;			/* キーボードデバイス */
#else
static LPDIRECTINPUT lpdi;					/* DirectInput */

static LPDIRECTINPUTDEVICE lpkbd;			/* キーボードデバイス */
#endif

	static DWORD keytime;						/* キーボードポーリング時間 */
static DWORD menuexittime;					/* メニュー抜け出し時間タイマ */
static DWORD dmyreadtime;					/* キーコードダミーリードタイマ */
static BOOL bDummyRead;						/* キーコードダミーリードフラグ */

static BYTE nKeyBuffer[KEYBUFFER_SIZE];		/* 内部キーバッファ */
static BYTE nKeyReadPtr;					/* 内部キーバッファ読出ポインタ */
static BYTE nKeyWritePtr;					/* 内部キーバッファ書込ポインタ */
static BYTE nLastKey;						/* 最後に押されたキー(FULL) */
static BYTE nLastKey2;						/* 最後に押されたキー(10KEY) */
static BYTE nTenDir;						/* テンキー変換 方向データ */
static BYTE nTenDir2;						/* テンキー変換 方向データ2 */

static BYTE joydat[3];						/* ジョイスティックデータ */
static BYTE joybk[2];						/* ジョイスティックバックアップ */
static int joyrapid[2][2];					/* ジョイスティック連射カウンタ */
static DWORD joytime;						/* ジョイスティックポーリング時間 */
static DWORD joytime2;						/* ジョイスティックポーリング時間 */
static BOOL joyplugged[2];					/* ジョイスティック接続フラグ */

#if defined(MOUSE)
#ifdef DINPUT8
static LPDIRECTINPUTDEVICE8 lpmos;			/* マウスデバイス */
#else
static LPDIRECTINPUTDEVICE lpmos;			/* マウスデバイス */
#endif

	static DWORD mostime;						/* マウスポーリング時間 */
static BOOL bCapture;						/* マウスキャプチャフラグ(Win32) */
static int nMouseX;							/* マウス X軸移動距離 */
static int nMouseY;							/* マウス Y軸移動距離 */
static BYTE nMouseButton;					/* マウス ボタン押下状態 */
static BYTE nMouseButtons;					/* マウス ボタン数 */
static BOOL bMouseCursor;					/* マウス カーソル表示状態 */
static int nMouseSX;						/* マウス X座標保存 */
static int nMouseSY;						/* マウス Y座標保存 */
#endif

#if defined(KBDPASTE)
static PTSTR strPaste;						/* ペーストデータ */
static PTSTR strPtr;						/* ペーストポインタ */
static char cPreKey;						/* 直前のコード */
static UINT uWaitCnt;						/* ペースト待ちカウンタ */
#endif


/*
 *	DirectInputコード→FM-7 物理コード
 *	コード対照表(106キーボード用)
 */
static BYTE kbd_106_table[] = {
	DIK_ESCAPE,			0x5c,			/* BREAK(ESC) */
	DIK_F1,				0x5d,			/* PF1 */
	DIK_F2,				0x5e,			/* PF2 */
	DIK_F3,				0x5f,			/* PF3 */
	DIK_F4,				0x60,			/* PF4 */
	DIK_F5,				0x61,			/* PF5 */
	DIK_F6,				0x62,			/* PF6 */
	DIK_F7,				0x63,			/* PF7 */
	DIK_F8,				0x64,			/* PF8 */
	DIK_F9,				0x65,			/* PF9 */
	DIK_F10,			0x66,			/* PF10 */

	DIK_KANJI,			0x01,			/* ESC(半角/全角) */
	DIK_1,				0x02,			/* 1 */
	DIK_2,				0x03,			/* 2 */
	DIK_3,				0x04,			/* 3 */
	DIK_4,				0x05,			/* 4 */
	DIK_5,				0x06,			/* 5 */
	DIK_6,				0x07,			/* 6 */
	DIK_7,				0x08,			/* 7 */
	DIK_8,				0x09,			/* 8 */
	DIK_9,				0x0a,			/* 9 */
	DIK_0,				0x0b,			/* 0 */
	DIK_MINUS,			0x0c,			/* - */
	DIK_CIRCUMFLEX,		0x0d,			/* ^ */
	DIK_YEN,			0x0e,			/* \ */
	DIK_BACK,			0x0f,			/* BS */

	DIK_TAB,			0x10,			/* TAB */
	DIK_Q,				0x11,			/* Q */
	DIK_W,				0x12,			/* W */
	DIK_E,				0x13,			/* E */
	DIK_R,				0x14,			/* R */
	DIK_T,				0x15,			/* T */
	DIK_Y,				0x16,			/* Y */
	DIK_U,				0x17,			/* U */
	DIK_I,				0x18,			/* I */
	DIK_O,				0x19,			/* O */
	DIK_P,				0x1a,			/* P */
	DIK_AT,				0x1b,			/* @ */
	DIK_LBRACKET,		0x1c,			/* [ */
	DIK_RETURN,			0x1d,			/* CR */

	DIK_LCONTROL,		0x52,			/* CTRL(左Ctrl) */
	DIK_A,				0x1e,			/* A */
	DIK_S,				0x1f,			/* S */
	DIK_D,				0x20,			/* D */
	DIK_F,				0x21,			/* F */
	DIK_G,				0x22,			/* G */
	DIK_H,				0x23,			/* H */
	DIK_J,				0x24,			/* J */
	DIK_K,				0x25,			/* K */
	DIK_L,				0x26,			/* L */
	DIK_SEMICOLON,		0x27,			/* ; */
	DIK_COLON,			0x28,			/* : */
	DIK_RBRACKET,		0x29,			/* ] */

	DIK_LSHIFT,			0x53,			/* 左SHIFT */
	DIK_Z,				0x2a,			/* Z */
	DIK_X,				0x2b,			/* X */
	DIK_C,				0x2c,			/* C */
	DIK_V,				0x2d,			/* V */
	DIK_B,				0x2e,			/* B */
	DIK_N,				0x2f,			/* N */
	DIK_M,				0x30,			/* M */
	DIK_COMMA,			0x31,			/* , */
	DIK_PERIOD,			0x32,			/* . */
	DIK_SLASH,			0x33,			/* / */
	DIK_BACKSLASH,		0x34,			/* _ */
	DIK_RSHIFT,			0x54,			/* 右SHIFT */

	DIK_CAPITAL,		0x55,			/* CAP(Caps Lock) */
	DIK_NOCONVERT,		0x56,			/* GRAPH(無変換) */
	DIK_CONVERT,		0x57,			/* 左SPACE(変換) */
	DIK_KANA,			0x58,			/* 中SPACE(カタカナ) */
	DIK_SPACE,			0x35,			/* 右SPACE(SPACE) */
	DIK_RCONTROL,		0x5a,			/* かな(右Ctrl) */

	DIK_INSERT,			0x48,			/* INS(Insert) */
	DIK_DELETE,			0x4b,			/* DEL(Delete) */
	DIK_UP,				0x4d,			/* ↑ */
	DIK_LEFT,			0x4f,			/* ← */
	DIK_DOWN,			0x50,			/* ↓ */
	DIK_RIGHT,			0x51,			/* → */

	DIK_HOME,			0x49,			/* EL(Home) */
	DIK_PRIOR,			0x4a,			/* CLS(Page Up) */
	DIK_END,			0x4c,			/* DUP(End) */
	DIK_NEXT,			0x4e,			/* HOME(Page Down) */

	DIK_MULTIPLY,		0x36,			/* Tenkey * */
	DIK_DIVIDE,			0x37,			/* Tenkey / */
	DIK_ADD,			0x38,			/* Tenkey + */
	DIK_SUBTRACT,		0x39,			/* Tenkey - */
	DIK_NUMPAD7,		0x3a,			/* Tenkey 7 */
	DIK_NUMPAD8,		0x3b,			/* Tenkey 8 */
	DIK_NUMPAD9,		0x3c,			/* Tenkey 9 */
	DIK_NUMPAD4,		0x3e,			/* Tenkey 4 */
	DIK_NUMPAD5,		0x3f,			/* Tenkey 5 */
	DIK_NUMPAD6,		0x40,			/* Tenkey 6 */
	DIK_NUMPAD1,		0x42,			/* Tenkey 1 */
	DIK_NUMPAD2,		0x43,			/* Tenkey 2 */
	DIK_NUMPAD3,		0x44,			/* Tenkey 3 */
	DIK_NUMPAD0,		0x46,			/* Tenkey 0 */
	DIK_DECIMAL,		0x47,			/* Tenkey . */
	DIK_NUMPADENTER,	0x45			/* Tenkey CR */
};

/*
 *	DirectInputコード→FM-7 物理コード
 *	コード対照表(NEC PC-98キーボード用)
 */
static BYTE kbd_98_table[] = {
	DIK_SYSRQ,			0x5c,			/* BREAK(COPY) */
	DIK_F1,				0x5d,			/* PF1 */
	DIK_F2,				0x5e,			/* PF2 */
	DIK_F3,				0x5f,			/* PF3 */
	DIK_F4,				0x60,			/* PF4 */
	DIK_F5,				0x61,			/* PF5 */
	DIK_F6,				0x62,			/* PF6 */
	DIK_F7,				0x63,			/* PF7 */
	DIK_F8,				0x64,			/* PF8 */
	DIK_F9,				0x65,			/* PF9 */
	DIK_F10,			0x66,			/* PF10 */

	DIK_ESCAPE,			0x01,			/* ESC */
	DIK_1,				0x02,			/* 1 */
	DIK_2,				0x03,			/* 2 */
	DIK_3,				0x04,			/* 3 */
	DIK_4,				0x05,			/* 4 */
	DIK_5,				0x06,			/* 5 */
	DIK_6,				0x07,			/* 6 */
	DIK_7,				0x08,			/* 7 */
	DIK_8,				0x09,			/* 8 */
	DIK_9,				0x0a,			/* 9 */
	DIK_0,				0x0b,			/* 0 */
	DIK_MINUS,			0x0c,			/* - */
	DIK_CIRCUMFLEX,		0x0d,			/* ^ */
	DIK_YEN,			0x0e,			/* \ */
	DIK_BACK,			0x0f,			/* BS */

	DIK_TAB,			0x10,			/* TAB */
	DIK_Q,				0x11,			/* Q */
	DIK_W,				0x12,			/* W */
	DIK_E,				0x13,			/* E */
	DIK_R,				0x14,			/* R */
	DIK_T,				0x15,			/* T */
	DIK_Y,				0x16,			/* Y */
	DIK_U,				0x17,			/* U */
	DIK_I,				0x18,			/* I */
	DIK_O,				0x19,			/* O */
	DIK_P,				0x1a,			/* P */
	DIK_AT,				0x1b,			/* @ */
	DIK_LBRACKET,		0x1c,			/* [ */
	DIK_RETURN,			0x1d,			/* CR */

	DIK_LCONTROL,		0x52,			/* CTRL(左Ctrl) */
	DIK_A,				0x1e,			/* A */
	DIK_S,				0x1f,			/* S */
	DIK_D,				0x20,			/* D */
	DIK_F,				0x21,			/* F */
	DIK_G,				0x22,			/* G */
	DIK_H,				0x23,			/* H */
	DIK_J,				0x24,			/* J */
	DIK_K,				0x25,			/* K */
	DIK_L,				0x26,			/* L */
	DIK_SEMICOLON,		0x27,			/* ; */
	DIK_COLON,			0x28,			/* : */
	DIK_RBRACKET,		0x29,			/* ] */

	DIK_LSHIFT,			0x53,			/* 左SHIFT(SHIFT) */
	DIK_Z,				0x2a,			/* Z */
	DIK_X,				0x2b,			/* X */
	DIK_C,				0x2c,			/* C */
	DIK_V,				0x2d,			/* V */
	DIK_B,				0x2e,			/* B */
	DIK_N,				0x2f,			/* N */
	DIK_M,				0x30,			/* M */
	DIK_COMMA,			0x31,			/* , */
	DIK_PERIOD,			0x32,			/* . */
	DIK_SLASH,			0x33,			/* / */
	DIK_UNDERLINE,		0x34,			/* _ */
	DIK_RSHIFT,			0x54,			/* 右SHIFT(SHIFT) */

	DIK_CAPITAL,		0x55,			/* CAP(CAPS) */
	DIK_NOCONVERT,		0x56,			/* GRAPH(NFER) */
	/* 左SPACE 割り当てなし */
	DIK_SPACE,			0x35,			/* 右SPACE(SPACE) */
	DIK_KANJI,			0x58,			/* 中SPACE(XFER) */
	DIK_KANA,			0x5a,			/* かな(カナ) */

	DIK_INSERT,			0x48,			/* INS(Insert) */
	DIK_DELETE,			0x4b,			/* DEL(Delete) */
	DIK_UP,				0x4d,			/* ↑ */
	DIK_LEFT,			0x4f,			/* ← */
	DIK_DOWN,			0x50,			/* ↓ */
	DIK_RIGHT,			0x51,			/* → */

	DIK_HOME,			0x49,			/* EL(HOME CLR) */
	DIK_PRIOR,			0x4a,			/* CLS(ROLL DOWN) */
	DIK_END,			0x4c,			/* DUP(HELP) */
	DIK_NEXT,			0x4e,			/* HOME(ROLL UP) */

	DIK_MULTIPLY,		0x36,			/* Tenkey * */
	DIK_DIVIDE,			0x37,			/* Tenkey / */
	DIK_ADD,			0x38,			/* Tenkey + */
	DIK_SUBTRACT,		0x39,			/* Tenkey - */
	DIK_NUMPAD7,		0x3a,			/* Tenkey 7 */
	DIK_NUMPAD8,		0x3b,			/* Tenkey 8 */
	DIK_NUMPAD9,		0x3c,			/* Tenkey 9 */
	DIK_NUMPADEQUALS,	0x3d,			/* Tenkey = */
	DIK_NUMPAD4,		0x3e,			/* Tenkey 4 */
	DIK_NUMPAD5,		0x3f,			/* Tenkey 5 */
	DIK_NUMPAD6,		0x40,			/* Tenkey 6 */
	DIK_NUMPADCOMMA,	0x41,			/* Tenkey , */
	DIK_NUMPAD1,		0x42,			/* Tenkey 1 */
	DIK_NUMPAD2,		0x43,			/* Tenkey 2 */
	DIK_NUMPAD3,		0x44,			/* Tenkey 3 */
	DIK_NUMPAD0,		0x46,			/* Tenkey 0 */
	DIK_DECIMAL,		0x47,			/* Tenkey . */
	DIK_NUMPADENTER,	0x45			/* Tenkey CR */
};

/*
 *	DirectInputコード→FM-7 物理コード
 *	コード対照表(101キーボード用)
 */
static BYTE kbd_101_table[] = {
	DIK_ESCAPE,			0x5c,			/* BREAK(ESC) */
	DIK_F1,				0x5d,			/* PF1 */
	DIK_F2,				0x5e,			/* PF2 */
	DIK_F3,				0x5f,			/* PF3 */
	DIK_F4,				0x60,			/* PF4 */
	DIK_F5,				0x61,			/* PF5 */
	DIK_F6,				0x62,			/* PF6 */
	DIK_F7,				0x63,			/* PF7 */
	DIK_F8,				0x64,			/* PF8 */
	DIK_F9,				0x65,			/* PF9 */
	DIK_F10,			0x66,			/* PF10 */

	DIK_GRAVE,			0x01,			/* ESC(`) */
	DIK_1,				0x02,			/* 1 */
	DIK_2,				0x03,			/* 2 */
	DIK_3,				0x04,			/* 3 */
	DIK_4,				0x05,			/* 4 */
	DIK_5,				0x06,			/* 5 */
	DIK_6,				0x07,			/* 6 */
	DIK_7,				0x08,			/* 7 */
	DIK_8,				0x09,			/* 8 */
	DIK_9,				0x0a,			/* 9 */
	DIK_0,				0x0b,			/* 0 */
	DIK_MINUS,			0x0c,			/* - */
	DIK_EQUALS,			0x0d,			/* ^(=) */
	DIK_BACKSLASH,		0x0e,			/* \ */
	DIK_BACK,			0x0f,			/* BS */

	DIK_TAB,			0x10,			/* TAB */
	DIK_Q,				0x11,			/* Q */
	DIK_W,				0x12,			/* W */
	DIK_E,				0x13,			/* E */
	DIK_R,				0x14,			/* R */
	DIK_T,				0x15,			/* T */
	DIK_Y,				0x16,			/* Y */
	DIK_U,				0x17,			/* U */
	DIK_I,				0x18,			/* I */
	DIK_O,				0x19,			/* O */
	DIK_P,				0x1a,			/* P */
	DIK_LBRACKET,		0x1b,			/* @([) */
	DIK_RBRACKET,		0x1c,			/* [(]) */
	DIK_RETURN,			0x1d,			/* CR */

	DIK_LCONTROL,		0x52,			/* CTRL(左Ctrl) */
	DIK_A,				0x1e,			/* A */
	DIK_S,				0x1f,			/* S */
	DIK_D,				0x20,			/* D */
	DIK_F,				0x21,			/* F */
	DIK_G,				0x22,			/* G */
	DIK_H,				0x23,			/* H */
	DIK_J,				0x24,			/* J */
	DIK_K,				0x25,			/* K */
	DIK_L,				0x26,			/* L */
	DIK_SEMICOLON,		0x27,			/* ; */
	DIK_APOSTROPHE,		0x28,			/* :(') */
	/* ] 割り当てなし */

	DIK_LSHIFT,			0x53,			/* 左SHIFT */
	DIK_Z,				0x2a,			/* Z */
	DIK_X,				0x2b,			/* X */
	DIK_C,				0x2c,			/* C */
	DIK_V,				0x2d,			/* V */
	DIK_B,				0x2e,			/* B */
	DIK_N,				0x2f,			/* N */
	DIK_M,				0x30,			/* M */
	DIK_COMMA,			0x31,			/* , */
	DIK_PERIOD,			0x32,			/* . */
	DIK_SLASH,			0x33,			/* / */
	/* _ 割り当てなし */
	DIK_RSHIFT,			0x54,			/* 右SHIFT */

	DIK_CAPITAL,		0x55,			/* CAP(Caps Lock) */
	DIK_NUMLOCK,		0x56,			/* GRAPH(Num Lock) */
	/* 左スペース割り当てなし */
	/* 中スペース割り当てなし */
	DIK_SPACE,			0x35,			/* 右SPACE(SPACE) */
	DIK_RCONTROL,		0x5a,			/* かな(右Ctrl) */

	DIK_INSERT,			0x48,			/* INS(Insert) */
	DIK_DELETE,			0x4b,			/* DEL(Delete) */
	DIK_UP,				0x4d,			/* ↑ */
	DIK_LEFT,			0x4f,			/* ← */
	DIK_DOWN,			0x50,			/* ↓ */
	DIK_RIGHT,			0x51,			/* → */

	DIK_HOME,			0x49,			/* EL(Home) */
	DIK_PRIOR,			0x4a,			/* CLS(Page Up) */
	DIK_END,			0x4c,			/* DUP(End) */
	DIK_NEXT,			0x4e,			/* HOME(Page Down) */

	DIK_MULTIPLY,		0x36,			/* Tenkey * */
	DIK_DIVIDE,			0x37,			/* Tenkey / */
	DIK_ADD,			0x38,			/* Tenkey + */
	DIK_SUBTRACT,		0x39,			/* Tenkey - */
	DIK_NUMPAD7,		0x3a,			/* Tenkey 7 */
	DIK_NUMPAD8,		0x3b,			/* Tenkey 8 */
	DIK_NUMPAD9,		0x3c,			/* Tenkey 9 */
	DIK_NUMPAD4,		0x3e,			/* Tenkey 4 */
	DIK_NUMPAD5,		0x3f,			/* Tenkey 5 */
	DIK_NUMPAD6,		0x40,			/* Tenkey 6 */
	DIK_NUMPAD1,		0x42,			/* Tenkey 1 */
	DIK_NUMPAD2,		0x43,			/* Tenkey 2 */
	DIK_NUMPAD3,		0x44,			/* Tenkey 3 */
	DIK_NUMPAD0,		0x46,			/* Tenkey 0 */
	DIK_DECIMAL,		0x47,			/* Tenkey . */
	DIK_NUMPADENTER,	0x45			/* Tenkey CR */
};

/*
 *	WinNT系キー入力対策
 *	コードテーブル
 */
static const BYTE DI_keyno[] = { DIK_KANJI, DIK_KANA, DIK_CAPITAL };

/*
 *	カーソルキー → テンキー変換
 *	物理コード対照表
 */
static BYTE TenDirTable[16] = {
	0x00, 0x3b, 0x43, 0xff, 0x3e, 0x3a, 0x42, 0x3e,
	0x40, 0x3c, 0x44, 0x40, 0xff, 0x3b, 0x43, 0xff
};


/*
 *	キーボード
 *	デフォルトマップ取得
 */
void FASTCALL GetDefMapKbd(BYTE *pMap, int mode)
{
	int i;
	int type;

	ASSERT(pMap);
	ASSERT((mode >= 0) && (mode <= 3));

	/* 一度、すべてクリア */
	memset(pMap, 0, 256);

	/* キーボードタイプ取得 */
	switch (mode) {
		/* 自動判別 */
		case 0:
			type = GetKeyboardType(1);
			if ((type & 0xf00) == 0xd00) {
				type = 0xd00;
			}
			else {
				if (type != 1) {
					if (GetKeyboardType(0) == 7) {
						type = 0;
					}
					else {
						type = 1;
					}
				}
			}
			break;
		/* 106 */
		case 1:
			type = 0;
			break;
		/* PC-98 */
		case 2:
			type = 0xd00;
			break;
		/* 101 */
		case 3:
			type = 1;
			break;
		/* その他 */
		default:
			ASSERT(FALSE);
			break;
	}

	/* 変換テーブルにデフォルト値をセット */
	if (type & 0xd00) {
		/* PC-98 */
		for (i=0; i<sizeof(kbd_98_table)/2; i++) {
			pMap[kbd_98_table[i * 2]] = kbd_98_table[i * 2 + 1];
		}
		return;
	}

	if (type == 0) {
		/* 106 */
		for (i=0; i<sizeof(kbd_106_table)/2; i++) {
			pMap[kbd_106_table[i * 2]] = kbd_106_table[i * 2 + 1];
		}
	}
	else {
		/* 101 */
		for (i=0; i<sizeof(kbd_101_table)/2; i++) {
			pMap[kbd_101_table[i * 2]] = kbd_101_table[i * 2 + 1];
		}
	}
}

/*
 *	キーボード
 *	マップ設定
 */
void FASTCALL SetMapKbd(BYTE *pMap)
{
	ASSERT(pMap);

	/* コピーするだけ */
	memcpy(kbd_table, pMap, sizeof(kbd_table));

	/* NT/PC-9801対策 : 対策フラグ(bit7)を有効にする */
	if (bNTkbMode) {
		kbd_table[DIK_KANJI] |= 0x80;
		kbd_table[DIK_CAPITAL] |= 0x80;
		kbd_table[DIK_KANA] |= 0x80;
	}
}

/*
 *	初期化
 */
void FASTCALL InitKbd(void)
{
	BYTE tmp_kbdmap[256];

	/* NT対策処理 */
	if (bNTflag || (GetKeyboardType(1) & 0xd00)) {
		bNTkbMode = TRUE;
	}
	else {
		bNTkbMode = FALSE;
	}

	/* ワークエリア初期化(キーボード) */
	lpdi = NULL;
	lpkbd = NULL;
	memset(kbd_map, 0, sizeof(kbd_map));
	memset(kbd_table, 0, sizeof(kbd_table));
	keytime = 0;

	/* ワークエリア初期化(キーボード NT/PC-9801対策) */
	memset(bNTkeyPushFlag, 0, sizeof(bNTkeyPushFlag));
	memset(bNTkeyMakeFlag, 0, sizeof(bNTkeyMakeFlag));
	bDummyRead = TRUE;
	dmyreadtime = 0;

	/* ワークエリア初期化(内部キーバッファ/疑似リアルタイムキースキャン) */
	memset(nKeyBuffer, 0, sizeof(nKeyBuffer));
	nKeyReadPtr = 0;
	nKeyWritePtr = 0;
	nLastKey = 0;
	nLastKey2 = 0;
	bKbdReal = FALSE;

	/* ワークエリア初期化(テンキー変換) */
	bArrow8Dir = TRUE;
	nTenDir = 0;
	nTenDir2 = 0;

	/* ワークエリア初期化(ジョイスティック) */
	joytime = 0;
	joytime2 = 0;
	memset(joydat, 0, sizeof(joydat));
	memset(joybk, 0, sizeof(joybk));
	memset(joyrapid, 0, sizeof(joyrapid));
	joyplugged[0] = TRUE;
	joyplugged[1] = TRUE;

#if defined(MOUSE)
	/* ワークエリア初期化(マウス) */
	mostime = 0;
	bCapture = FALSE;
	nMouseX = 0;
	nMouseY = 0;
	bMouseCursor = TRUE;
	nMouseButton = 0xf0;
	uMidBtnMode = MOSCAP_NONE;
#endif

	/* デフォルトマップを設定 */
	GetDefMapKbd(tmp_kbdmap, 0);
	SetMapKbd(tmp_kbdmap);

	/* テンキーエミュレーション */
	bTenCursor = FALSE;

#if defined(KBDPASTE)
	/* キーペーストデータ */
	strPaste = NULL;
	strPtr = NULL;
	cPreKey = 0;
	uWaitCnt = 0;
	uPasteWait = 0;
	uPasteWaitCntl = 0;
#endif
}

/*
 *	クリーンアップ
 */
void FASTCALL CleanKbd(void)
{
	/* DirectInputDevice(キーボード)を解放 */
	if (lpkbd) {
		lpkbd->Unacquire();
		lpkbd->Release();
		lpkbd = NULL;
	}

#if defined(MOUSE)
	/* DirectInputDevice(マウス)を解放 */
	if (lpmos) {
		lpmos->Unacquire();
		lpmos->Release();
		lpmos = NULL;
	}
#endif

	/* DirectInputを解放 */
	if (lpdi) {
		lpdi->Release();
		lpdi = NULL;
	}

#if defined(KBDPASTE)
	if (strPaste) {
		free(strPaste);
		strPaste = NULL;
		strPtr = NULL;
		uWaitCnt = 0;
	}
#endif
}

/*
 *	セレクト
 */
BOOL FASTCALL SelectKbd(HWND hWnd)
{
	DIDEVCAPS caps;

	ASSERT(hWnd);

#ifdef DINPUT8
	/* DirectInputオブジェクトを作成 */
	if (FAILED(DirectInput8Create(hAppInstance, DIRECTINPUT_VERSION,
							IID_IDirectInput8A, (void**)&lpdi, NULL))) {
		return FALSE;
	}
#else
	/* DirectInputオブジェクトを作成 */
	if (FAILED(DirectInputCreate(hAppInstance, DIRECTINPUT_VERSION,
							&lpdi, NULL))) {
		return FALSE;
	}
#endif

	/* キーボードデバイスを作成 */
	if (FAILED(lpdi->CreateDevice(GUID_SysKeyboard, &lpkbd, NULL))) {
		return FALSE;
	}

	/* キーボードデータ形式を設定 */
	if (FAILED(lpkbd->SetDataFormat(&c_dfDIKeyboard))) {
		return FALSE;
	}

	/* 協調レベルを設定 */
	if (FAILED(lpkbd->SetCooperativeLevel(hWnd,
						DISCL_BACKGROUND | DISCL_NONEXCLUSIVE))) {
		return FALSE;
	}

#if defined(MOUSE)
	/* マウスデバイスを作成 */
	bDetectMouse = FALSE;
	if (SUCCEEDED(lpdi->CreateDevice(GUID_SysMouse, &lpmos, NULL))) {
		/* マウスデータ形式を設定 */
		if (SUCCEEDED(lpmos->SetDataFormat(&c_dfDIMouse))) {
			/* 協調レベルを設定 */
			if (SUCCEEDED(lpmos->SetCooperativeLevel(hWnd,
						DISCL_BACKGROUND | DISCL_NONEXCLUSIVE))) {
				/* マウスボタン数を取得 */
				caps.dwSize = sizeof(DIDEVCAPS);
				if (SUCCEEDED(lpmos->GetCapabilities(&caps))) {
					nMouseButtons = (BYTE)caps.dwButtons;
					bDetectMouse = TRUE;
				}
			}
		}
	}
#endif

	return TRUE;
}

/*
 *	キーボード
 *	内部キーバッファ登録
 */
static void FASTCALL PushKeyCode(BYTE code)
{
	int i;

	if (nKeyWritePtr < KEYBUFFER_SIZE) {
		nKeyBuffer[nKeyWritePtr++] = code;
	}
	else {
		for (i=0; i<KEYBUFFER_SIZE; i++) {
			nKeyBuffer[i] = nKeyBuffer[i + 1];
		}
		nKeyBuffer[KEYBUFFER_SIZE - 1] = code;

		if (nKeyReadPtr > 0) {
			nKeyReadPtr --;
		}
	}
}

/*
 *	キーボード テンキー変換 
 *	方向コード変換
 */
static BYTE FASTCALL Cur2Ten_DirCode(BYTE code)
{
	switch (code) {
		case 0x4d:	/* ↑ */
			return 0x01;
		case 0x4f:	/* ← */
			return 0x04;
		case 0x50:	/* ↓ */
			return 0x02;
		case 0x51:	/* → */
			return 0x08;
	}

	return 0;
}

/*
 *	キーボード テンキー変換 
 *	マスクコード変換
 */
static BYTE FASTCALL Cur2Ten_MaskCode(BYTE code)
{
	switch (code) {
		case 0x4d:	/* ↑ */
		case 0x50:	/* ↓ */
			return 0xfc;
		case 0x4f:	/* ← */
		case 0x51:	/* → */
			return 0xf3;
	}

	return 0xff;
}

/*
 *	キーボード
 *	カーソルキー→テンキー変換 Make
 */
static BOOL FASTCALL Cur2Ten_Make(BYTE code)
{
	BYTE nTenDirOld;
	BYTE dircode;

	/* 以前の方向コードを保存 */
	nTenDirOld = nTenDir;

	/* キーコード変換 */
	dircode = Cur2Ten_DirCode(code);
	if (dircode) {
		if (bArrow8Dir) {
			/* 8方向 */
			nTenDir2 = (BYTE)(nTenDir & (BYTE)~Cur2Ten_MaskCode(code));
			nTenDir &= Cur2Ten_MaskCode(code);
			nTenDir |= dircode;
		}
		else {
			if (key_format == KEY_FORMAT_SCAN) {
				if (TenDirTable[dircode]) {
					keyboard_make(TenDirTable[dircode]);
				}
				return TRUE;
			}

			/* 4方向 */
			nTenDir2 = (BYTE)(nTenDir & (BYTE)Cur2Ten_MaskCode(code));
			nTenDir = dircode;
		}

		/* テーブル内容が0xffの場合、無効な組み合わせ */
		if (TenDirTable[nTenDir] == 0xff) {
			nTenDir = nTenDirOld;
			return TRUE;
		}

		/* キーコード発行 */
		if (nTenDir != nTenDirOld) {
			if (TenDirTable[nTenDirOld]) {
				keyboard_break(TenDirTable[nTenDirOld]);
				PushKeyCode(TenDirTable[nTenDir]);
			}
			else {
				keyboard_make(TenDirTable[nTenDir]);
			}
		}
		return TRUE;
	}

	return FALSE;
}

/*
 *	キーボード
 *	カーソルキー→テンキー変換 Break
 */
static BOOL FASTCALL Cur2Ten_Break(BYTE code)
{
	BYTE nTenDirOld;
	BYTE dircode;

	/* 以前の方向コードを保存 */
	nTenDirOld = nTenDir;

	/* キーコード変換 */
	dircode = Cur2Ten_DirCode(code);
	if (dircode) {
		if ((key_format == KEY_FORMAT_SCAN) && !bArrow8Dir) {
			if (TenDirTable[dircode]) {
				keyboard_break(TenDirTable[dircode]);
			}
			return TRUE;
		}

		nTenDir2 &= (BYTE)~dircode;
		if (nTenDir2) {
			nTenDir |= nTenDir2;
			nTenDir2 = 0;
		}
		nTenDir &= (BYTE)~dircode;

		/* テーブル内容が0xffの場合、無効な組み合わせ */
		if (TenDirTable[nTenDir] == 0xff) {
			nTenDir = nTenDirOld;
			return TRUE;
		}

		/* キーコード発行 */
		if (nTenDir != nTenDirOld) {
			keyboard_break(TenDirTable[nTenDirOld]);
			if (TenDirTable[nTenDir]) {
				PushKeyCode(TenDirTable[nTenDir]);
			}
			else if (key_format != KEY_FORMAT_SCAN) {
				/* 停止時は"5"を発行 */
				PushKeyCode(0x3f);
				PushKeyCode(0xbf);
			}
		}
		return TRUE;
	}

	return FALSE;
}

/*
 *	キーボード
 *	メニュー抜け出しタイマ設定
 */
void FASTCALL SetMenuExitTimer(void)
{
	bMenuExit = TRUE;
	menuexittime = dwExecTotal;
}

/*
 *	キーボード
 *	ポーリング
 */
void FASTCALL PollKbd(void)
{
	HRESULT hr;
	BYTE buf[256];
	int i;
	BYTE fm7;
	BOOL bFlag;
	static BYTE key_last = 0;

#if defined(KBDPASTE)
	/* ここでカウント */
	if (uWaitCnt < (unsigned int)-1) {
		uWaitCnt ++;
	}
#endif

	/* DirectInputチェック */
	if (!lpkbd) {
		return;
	}

	/* メニュー抜け出しチェック */
	if (bMenuExit) {
		if ((dwExecTotal - menuexittime) < 300000) {
			return;
		}
		bMenuExit = FALSE;
	}

	/* 最小化チェック */
	if (bMinimize) {
		return;
	}

	/* アクティベートチェック */
	if (!bActivate) {
		/* 非アクティブ時はダミーリードタイマ更新 */
		bDummyRead = TRUE;
		dmyreadtime = dwExecTotal;
		return;
	}

	/* ダミーリード時間チェック */
	if (bDummyRead && ((dwExecTotal - dmyreadtime) > 100000)) {
		bDummyRead = FALSE;
	}

	/* 時間チェック */
	if ((dwExecTotal - keytime) < 20000) {
		return;
	}
	keytime = dwExecTotal;

#if defined(KBDPASTE)
	/* ペーストバッファ内にデータがたまっている場合、先に処理 */
	if (strPtr) {
		/* スキャンモード時は対象外とする */
		if (key_format == KEY_FORMAT_SCAN) {
			/* 捨てます！ */
			free(strPaste);
			strPaste = NULL;
			strPtr = NULL;
			uWaitCnt = 0;
		}
		else if (*strPtr) {
			if (((cPreKey & 0xe0) == 0) || (cPreKey == 0x7f)) {
				if (uWaitCnt - 1 >= uPasteWaitCntl) {
					if (keyboard_paste(*strPtr)) {
						/* ペーストしたのでポインタをすすめる */
						cPreKey = (*strPtr);
						strPtr++;
						uWaitCnt = 0;
					}
				}
			}
			else {
				if (uWaitCnt - 1 >= uPasteWait) {
					if (keyboard_paste(*strPtr)) {
						/* ペーストしたのでポインタをすすめる */
						cPreKey = (*strPtr);
						strPtr++;
						uWaitCnt = 0;
					}
				}
			}
			/* ペースト処理中は他のキー処理はしない */
			return;
		}
		else {
			/* ペースト終わったので解放 */
			free(strPaste);
			strPaste = NULL;
			strPtr = NULL;
			uWaitCnt = 0;
		}
	}
#endif

	/* キーバッファ内にコードがたまっている場合、先に処理 */
	if (nKeyReadPtr < nKeyWritePtr) {
		fm7 = nKeyBuffer[nKeyReadPtr++];
		if (fm7 == 0xff) {
			/* 0xff : キーリピートタイマ変更 */
			keyboard_repeat();
		}
		else if (fm7 & 0x80) {
			/* 0x80-0xfe : Breakコード */
			keyboard_break((BYTE)(fm7 & 0x7f));
		}
		else {
			/* 0x00-0x7f : Makeコード */
			keyboard_make(fm7);
		}

		return;
	}
	else {
		nKeyReadPtr = 0;
		nKeyWritePtr = 0;
	}

	/* デバイスのアクセス権を取得(何回取得してもよい) */
	hr = lpkbd->Acquire();
	if ((hr != DI_OK) && (hr != S_FALSE)) {
		return;
	}

	/* デバイス状態を取得 */
	if (lpkbd->GetDeviceState(sizeof(buf), buf) != DI_OK) {
		return;
	}

	/* NT/PC-9801対策処理 : キー押下時のフラグ反転 */
	if (bNTkbMode) {
		for (i = 0; i < 3; i ++) {
			buf[DI_keyno[i]] = kbd_map[DI_keyno[i]];
			if (bNTkeyPushFlag[i]) {
				buf[DI_keyno[i]] ^= (BYTE)0x80;
				bNTkeyPushFlag[i] = FALSE;
			}
		}
	}

	/* Vista/Windows7 右SHIFT対策処理 */
#ifndef DINPUT8
	if (bVistaflag) {
		if (bNTkeyPushFlag[KNT_RSHIFT]) {
			buf[DIK_RSHIFT] |= 0x80;
		}
		else {
			buf[DIK_RSHIFT] &= (BYTE)~0x80;
		}
	}
#endif

	/* フラグoff */
	bFlag = FALSE;

	/* 今までの状態と比較して、順に変換する */
	for (i=0; i<sizeof(buf); i++) {
		if (((buf[i] & 0x80) != (kbd_map[i] & 0x80)) && (!bFlag)) {
			if ((buf[i] & 0x80) || (kbd_table[i] & 0x80)) {
				/* キー押下 */
				fm7 = (BYTE)(kbd_table[i] & 0x7f);

				/* ALTキーとの同時押しは無効 */
				if ((!kbd_table[DIK_LMENU]) && (buf[DIK_LMENU] & 0x80)) {
					fm7 = 0;
				}
				if ((!kbd_table[DIK_RMENU]) && (buf[DIK_RMENU] & 0x80)) {
					fm7 = 0;
				}

				if (fm7 > 0) {
					if (!bMenuLoop) {
						if (!(bDummyRead && (kbd_table[i] & 0x80))) {

							if ((fm7 >= 0x4d) && (fm7 <= 0x51) && bTenCursor) {
								/* スキャンモード時は本来のコードも発行 */
								if ((key_format == KEY_FORMAT_SCAN) && (fm7 != 0x4e)) {
									PushKeyCode(fm7);
								}

								/* スキャンコード変換 */
								if (Cur2Ten_Make(fm7)) {
									fm7 = 0;
								}
							}

							/* NT/PC-9801対策 : キーBreak用タイマ登録 */
							if (kbd_table[i] & 0x80) {
								if (bNTkeyMakeFlag[kbd_table[i] & 0x7f]) {
									KillTimer(hMainWnd, kbd_table[i]);
									if ((key_format != KEY_FORMAT_SCAN) && (fm7 != 0x5c)) {
										keyboard_break(fm7);
										PushKeyCode(fm7);
									}
								}
								else {
									keyboard_make(fm7);
								}
								bNTkeyMakeFlag[kbd_table[i] & 0x7f] = TRUE;
								SetTimer(hMainWnd, kbd_table[i], 100, NULL);
							}
							else if (fm7) {
								keyboard_make(fm7);
							}
							bDummyRead = FALSE;

							/* 疑似リアルタイムキースキャン */
							if (bKbdReal && (key_format != KEY_FORMAT_SCAN)) {
								if ((fm7 >= 0x3a) && (fm7 <= 0x46)) {
									/* 以前押されていたキーがあれば再発行 */
									if (nLastKey && key_repeat_flag) {
										PushKeyCode(nLastKey);
									}
									PushKeyCode(0xff);

									/* 再発行用にキーコードを記憶 */
									if (fm7 == 0x3f) {
										nLastKey2 = 0;
									}
									else {
										nLastKey2 = fm7;
									}
								}
								else {
									/* 再発行用にキーコードを記憶 */
									if (fm7 != 0x5c) {
										nLastKey = fm7;
									}
									else {
										nLastKey = 0;
									}
								}
							}
						}

						if (fm7 != 0x5c) {
							key_last = fm7;
						}

						bFlag = TRUE;
					}
				}
			}
			else {
				/* キー離した */
				fm7 = (BYTE)(kbd_table[i] & 0x7f);
				if (fm7 > 0) {
					if ((fm7 >= 0x4d) && (fm7 <= 0x51) && bTenCursor) {
						/* スキャンモード時は本来のコードも発行 */
						if ((key_format == KEY_FORMAT_SCAN) && (fm7 != 0x4e)) {
							PushKeyCode((BYTE)(fm7 | 0x80));
						}

						/* スキャンコード変換 */
						if (Cur2Ten_Break(fm7)) {
							fm7 = 0;
						}
					}

					/* 疑似リアルタイムキースキャン */
					if (bKbdReal && (key_format != KEY_FORMAT_SCAN)) {
						if ((fm7 >= 0x3a) && (fm7 <= 0x46)) {
							/* テンキーの場合 */
							PushKeyCode((BYTE)(fm7 | 0x80));

							if (nLastKey2 == fm7) {
								PushKeyCode(0x3f);
								PushKeyCode(0xbf);
								nLastKey2 = 0;
							}

							/* 以前テンキー以外が押されていた場合、再発行 */
							if (nLastKey && key_repeat_flag) {
								PushKeyCode(nLastKey);
								PushKeyCode(0xff);
								key_last = nLastKey;
							}
						}
						else {
							/* テンキー以外の場合 */
							keyboard_break(fm7);
							if (nLastKey == fm7) {
								nLastKey = 0;
							}

							/* 以前テンキーが押されていた場合、再発行 */
							if (nLastKey2 && (nLastKey2 != key_last)) {
								PushKeyCode(nLastKey2);
								PushKeyCode(0xff);
								key_last = nLastKey2;
							}
						}
					}
					else if (fm7) {
						keyboard_break(fm7);
					}

					bFlag = TRUE;
				}
			}

			/* データをコピー */
			kbd_map[i] = buf[i];
		}
	}
}

/*
 *	キーボード
 *	ポーリング＆キー情報取得
 *	※VMのロックは行っていないので注意
 */
BOOL FASTCALL GetKbd(BYTE *pBuf)
{
	HRESULT hr;
	int i;

	ASSERT(pBuf);

	/* メモリクリア */
	memset(pBuf, 0, 256);

	/* DirectInputチェック */
	if (!lpkbd) {
		return FALSE;
	}

	/* デバイスのアクセス権を取得(何回取得してもよい) */
	hr = lpkbd->Acquire();
	if ((hr != DI_OK) && (hr != S_FALSE)) {
		return FALSE;
	}

	/* デバイス状態を取得 */
	if (lpkbd->GetDeviceState(256, pBuf) != DI_OK) {
		return FALSE;
	}

	/* NT対策 */
	if (bNTkbMode) {
		for (i=0; i<3; i++) {
			pBuf[DI_keyno[i]] = 0;
		}
		if ((GetAsyncKeyState(VKNT_KANJI1) |
			 GetAsyncKeyState(VKNT_KANJI2)) & 1) {
			pBuf[DIK_KANJI] = 0x80;
		}
		if ((GetAsyncKeyState(VKNT_CAPITAL) |
			 GetAsyncKeyState(VK_CAPITAL)) & 1) {
			pBuf[DIK_CAPITAL] = 0x80;
		}
		if ((GetAsyncKeyState(VKNT_KANA) |
			 GetAsyncKeyState(VK_KANA)) & 1) {
			pBuf[DIK_KANA] = 0x80;
		}
	}

#ifndef DINPUT8
	/* Vista/Windows7 右SHIFT対策処理 */
	if (bVistaflag) {
		/* Vista/Window7では0番は常に無効 */
		pBuf[0x00] = 0x00;

		/* 右シフトキーチェック */
		if (GetAsyncKeyState(VK_RSHIFT)) {
			pBuf[DIK_RSHIFT] = 0x80;
		}
	}
#endif

	return TRUE;
}

#if defined(KBDPASTE)
/*
 *	キーボード
 *	ペースト（クリップボード）
 */
void FASTCALL PasteClipboardKbd(HWND hWnd)
{
	BOOL b;
	HGLOBAL hg;
	PTSTR strClip;

	ASSERT(hWnd);

	b = OpenClipboard(hWnd);
	hg = GetClipboardData(CF_TEXT);
	if (b && hg) {
		strClip = (PTSTR)GlobalLock(hg);
		if (strPaste) {
			free(strPaste);
			strPaste = NULL;
			strPtr = NULL;
			uWaitCnt = 0;
		}
		strPtr = strPaste = (PTSTR)malloc(GlobalSize(hg));
		if (strPaste) {
			lstrcpy(strPaste, strClip);
		}
		GlobalUnlock(hg);
		CloseClipboard();
	}
}

/*
 *	キーボード
 *	ペースト（引数）
 */
void FASTCALL PasteKbd(char *pstring)
{
	PTSTR strKbd;

	strKbd = (PTSTR)pstring;
	if (strPaste) {
		free(strPaste);
		strPaste = NULL;
		strPtr = NULL;
		uWaitCnt = 0;
	}
	strPtr = strPaste = (PTSTR)malloc(strlen(pstring));
	if (strPaste) {
		lstrcpy(strPaste, strKbd);
	}
}
#endif

/*
 *	ジョイスティック
 *	デバイスより読み込み
 */
static BYTE FASTCALL GetJoy(int index, BOOL flag)
{
	int num;
	JOYINFOEX jiex;
	MMRESULT result;
	BYTE ret;

	/* assert */
	ASSERT((index == 0) || (index == 1));

	/* サポートジョイスティック数を取得 */
	num = joyGetNumDevs();
	if (index >= num) {
		joyplugged[index] = FALSE;
		return 0;
	}

	/* データクリア */
	ret = 0;

	/* データ取得 */
	memset(&jiex, 0, sizeof(jiex));
	jiex.dwSize = sizeof(jiex);
	jiex.dwFlags = JOY_RETURNX | JOY_RETURNY | JOY_RETURNBUTTONS |
					JOY_RETURNPOV | JOY_RETURNCENTERED;
	result = joyGetPosEx(index, &jiex);
	if (result == JOYERR_NOERROR) {
		joyplugged[index] = TRUE;

		/* データ評価(方向) */
		if (jiex.dwXpos < 0x4000) {
			ret |= 0x04;
		}
		if (jiex.dwXpos > 0xc000) {
			ret |= 0x08;
		}
		if (jiex.dwYpos < 0x4000) {
			ret |= 0x01;
		}
		if (jiex.dwYpos > 0xc000) {
			ret |= 0x02;
		}
		switch (jiex.dwPOV) {  
			case 0:  
				ret |= 0x01;
				break;
			case 4500:  
				ret |= 0x09;
				break;
			case 9000:  
				ret |= 0x08;
				break;
			case 13500:  
				ret |= 0x0a;
				break;
			case 18000:  
				ret |= 0x02;
				break;
			case 22500:  
				ret |= 0x06;
				break;
			case 27000:  
				ret |= 0x04;
				break;
			case 31500:  
				ret |= 0x05;
				break;
		}  

		/* データ評価(ボタン) */
		if (jiex.dwButtons & JOY_BUTTON1) {
			ret |= 0x20;
		}
		if (jiex.dwButtons & JOY_BUTTON2) {
			ret |= 0x10;
		}
	}
	else if (flag) {
		joyplugged[index] = FALSE;
		ret = 0;
	}

	return ret;
}

/*
 *	ジョイスティック
 *	連射カウンタテーブル(片側、20ms単位)
 */
static const BYTE JoyRapidCounter[] = {
	0,			/* なし */
	25,			/* 1ショット */
	12,			/* 2ショット */
	8,			/* 3ショット */
	6,			/* 4ショット */
	5,			/* 5ショット */
	4,			/* 6ショット */
	3,			/* 8ショット */
	2,			/* 12ショット */
	1			/* 25ショット */
};

/*
 *	ジョイスティック
 *	デバイスより読み込み(連射つき)
 */
static BYTE FASTCALL GetRapidJoy(int index, BOOL flag)
{
	int i;
	BYTE bit;
	BYTE dat;

	/* assert */
	ASSERT((index == 0) || (index == 1));

	/* 非接続チェック1 (接続チェック時は通す) */
	if ((!flag) && (!joyplugged[index])) {
		return 0x00;
	}

	/* データ取得 */
	dat = GetJoy(index, flag);

	/* 非接続チェック2 */
	if (!joyplugged[index]) {
		return 0x00;
	}

	/* ボタンチェック */
	bit = 0x10;
	for (i=0; i<2; i++) {
		if ((dat & bit) && (nJoyRapid[index][i] > 0)) {
			/* 連射ありで押されている。カウンタチェック */
			if (joyrapid[index][i] == 0) {
				/* 初期カウンタを代入 */
				joyrapid[index][i] = JoyRapidCounter[nJoyRapid[index][i]];
			}
			else {
				/* カウンタデクリメント */
				joyrapid[index][i]--;
				if ((joyrapid[index][i] & 0xff) == 0) {
					/* 反転タイミングなので、時間を加算して反転 */
					joyrapid[index][i] += JoyRapidCounter[nJoyRapid[index][i]];
					joyrapid[index][i] ^= 0x100;
				}
			}
			/* ボタンが押されていないように振る舞う */
			if (joyrapid[index][i] >= 0x100) {
				dat &= (BYTE)(~bit);
			}
		}
		else {
			/* ボタンが押されてないので、連射カウンタクリア */
			joyrapid[index][i] = 0;
		}
		/* 次のビットへ */
		bit <<= 1;
	}

	return dat;
}

/*
 *	ジョイスティック
 *	コード変換
 */
static BYTE FASTCALL PollJoyCode(int code)
{
	/* 0x70未満は無視 */
	if (code < 0x70) {
		return 0;
	}

	/* 0x70から上下左右 */
	switch (code) {
		/* 上 */
		case 0x70:
			return 0x01;
		/* 下 */
		case 0x71:
			return 0x02;
		/* 左 */
		case 0x72:
			return 0x04;
		/* 右 */
		case 0x73:
			return 0x08;
		/* Aボタン */
		case 0x74:
			return 0x10;
		/* Bボタン */
		case 0x75:
			return 0x20;
		/* それ以外 */
		default:
			ASSERT(FALSE);
			break;
	}

	return 0;
}

/*
 *	ジョイスティック
 *	ポーリング(ジョイスティック)
 */
static BYTE FASTCALL PollJoySub(int index, BYTE dat)
{
	int i;
	BYTE ret;
	BYTE bit;

	/* assert */
	ASSERT((index == 0) || (index == 1));

	/* 終了データクリア */
	ret = 0;

	/* 方向 */
	bit = 0x01;
	for (i=0; i<4; i++) {
		/* ボタンが押されているか */
		if (dat & bit) {
			/* コード変換 */
			ret |= PollJoyCode(nJoyCode[index][i]);
		}
		bit <<= 1;
	}

	/* センターチェック */
	if ((dat & 0x0f) == 0) {
		if ((joybk[index] & 0x0f) != 0) {
			ret |= PollJoyCode(nJoyCode[index][4]);
		}
	}

	/* ボタン */
	if (dat & 0x10) {
		ret |= PollJoyCode(nJoyCode[index][5]);
	}
	if (dat & 0x20) {
		ret |= PollJoyCode(nJoyCode[index][6]);
	}

	return ret;
}

/*
 *	ジョイスティック
 *	ポーリング(キーボード)
 */
static void FASTCALL PollJoyKbd(int index, BYTE dat)
{
	BYTE bit;
	int i;

	/* 上下左右 */
	bit = 0x01;
	for (i=0; i<4; i++) {
		if (dat & bit) {
			/* 初めて押されたら、make発行 */
			if ((joybk[index] & bit) == 0) {
				if ((nJoyCode[index][i] > 0) && (nJoyCode[index][i] <= 0x66)) {
					keyboard_make((BYTE)nJoyCode[index][i]);
				}
			}
		}
		else {
			/* 初めて離されたら、break発行 */
			if ((joybk[index] & bit) != 0) {
				if ((nJoyCode[index][i] > 0) && (nJoyCode[index][i] <= 0x66)) {
					keyboard_break((BYTE)nJoyCode[index][i]);
				}
			}
		}
		bit <<= 1;
	}

	/* センターチェック */
	if ((dat & 0x0f) == 0) {
		if ((joybk[index] & 0x0f) != 0) {
			/* make/breakを続けて出す */
			if ((nJoyCode[index][4] > 0) && (nJoyCode[index][4] <= 0x66)) {
				keyboard_make((BYTE)nJoyCode[index][4]);
				keyboard_break((BYTE)nJoyCode[index][4]);
			}
		}
	}

	/* ボタン */
	bit = 0x10;
	for (i=0; i<2; i++) {
		if (dat & bit) {
			/* 初めて押さたら、make発行 */
			if ((joybk[index] & bit) == 0) {
				if ((nJoyCode[index][i + 5] > 0) && (nJoyCode[index][i + 5] <= 0x66)) {
					keyboard_make((BYTE)nJoyCode[index][i + 5]);
				}
			}
		}
		else {
			/* 初めて離されたら、break発行 */
			if ((joybk[index] & bit) != 0) {
				if ((nJoyCode[index][i + 5] > 0) && (nJoyCode[index][i + 5] <= 0x66)) {
					keyboard_break((BYTE)nJoyCode[index][i + 5]);
				}
			}
		}
		bit <<= 1;
	}
}

/*
 *	ジョイスティック
 *	ポーリング
 */
void FASTCALL PollJoy(void)
{
	BYTE dat;
	BOOL check;
	int i;

	/* 間隔チェック(ポーリング用) */
	if ((dwExecTotal - joytime) < 10000) {
		return;
	}
	joytime = dwExecTotal;

	/* データをクリア */
	memset(joydat, 0, sizeof(joydat));

	/* 無効チェック */
	if ((nJoyType[0] == 0) && (nJoyType[1] == 0)) {
		return;
	}

	/* 間隔チェック(接続チェック用) */
	if ((dwExecTotal - joytime2) >= 500000) {
		joytime2 = dwExecTotal;
		check = TRUE;
	}
	else {
		check = FALSE;
	}

	/* デバイスループ */
	for (i=0; i<2; i++) {
		/* データ取得(連射つき) */
		dat = GetRapidJoy(i, check);
		if (!joyplugged[i]) {
			continue;
		}

		/* タイプ別 */
		switch (nJoyType[i]) {
			/* ジョイスティックポート1 */
			case 1:
				joydat[0] = PollJoySub(i, dat);
				break;
			/* ジョイスティックポート2 */
			case 2:
				joydat[1] = PollJoySub(i, dat);
				break;
			/* キーボード */
			case 3:
				PollJoyKbd(i, dat);
				break;
			/* 電波新聞社ジョイスティック */
			case 4:
				joydat[2] = PollJoySub(i, dat);
				break;
		}

		/* データ更新 */
		joybk[i] = dat;
	}
}

/*
 *	ジョイスティック
 *	データリクエスト
 */
BYTE FASTCALL joy_request(BYTE no)
{
/*	ASSERT((no >= 0) && (no < 3)); */
	ASSERT(no < 3);

	return joydat[no];
}


#if defined(MOUSE)
/*
 *	マウス
 *	ポーリング
 */
void FASTCALL PollMos(void)
{
	DIMOUSESTATE buf;
	HRESULT hr;

	/* DirectInputチェック */
	if (!lpmos) {
		return;
	}

	/* マウス存在チェック */
	if (!bDetectMouse) {
		return;
	}

	/* アクティベートチェック */
	if (!bActivate) {
		return;
	}

	/* 時間チェック */
	if ((dwExecTotal - mostime) < 10000) {
		return;
	}
	mostime = dwExecTotal;

	/* デバイスのアクセス権を取得(何回取得してもよい) */
	hr = lpmos->Acquire();
	if ((hr != DI_OK) && (hr != S_FALSE)) {
		return;
	}

	/* デバイス状態を取得 */
	if (lpmos->GetDeviceState(sizeof(DIMOUSESTATE), &buf) != DI_OK) {
		return;
	}

	/* 移動距離・ボタン状態の設定 */
	if (bCapture && mos_capture) {
		/* マウス移動距離を蓄積 */
		nMouseX -= buf.lX;
		nMouseY -= buf.lY;

		/* ボタン状態を設定 */
		nMouseButton = 0xf0;
		if (buf.rgbButtons[0]) {
			/* 左ボタン押下 */
			nMouseButton &= ~0x10;
		}
		if (buf.rgbButtons[1]) {
			/* 右ボタン押下 */
			nMouseButton &= ~0x20;
		}
	}
}


/*
 *	マウス
 *	キャプチャ状態設定
 */
void FASTCALL SetMouseCapture(BOOL en)
{
	DIMOUSESTATE buf;
	HRESULT hr;
	RECT rect;
	POINT center;

	ASSERT(hDrawWnd);

	/* キャプチャ停止中/VM停止中/非アクティブ時は強制的に無効 */
	if (!mos_capture || stopreq_flag || !run_flag || !bActivate || !lpmos) {
		en = FALSE;
	}

	/* カーソル表示/消去 */
	if (bMouseCursor == en) {
		ShowCursor(!en);
		bMouseCursor = !en;
	}

	/* キャプチャ状態に変化がなければ帰る */
	if (bCapture == en) {
		return;
	}

	if (en) {
		if (hDrawWnd) {
			/* カーソル位置を保存 */
			GetCursorPos(&center);
			nMouseSX = center.x;
			nMouseSY = center.y;

			/* 描画ウィンドウの四隅の座標を求める */
			GetWindowRect(hDrawWnd, &rect);

			/* 四隅の座標から中心座標を求める */
			center.x = (rect.right + rect.left) / 2;
			center.y = (rect.bottom + rect.top) / 2;

			/* カーソルをウィンドウ中央に固定 */
			rect.left   = center.x;
			rect.right  = center.x;
			rect.top    = center.y;
			rect.bottom = center.y;
			SetCursorPos(center.x, center.y);
			ClipCursor(&rect);
		}
	}
	else {
		/* クリップ解除 */
		ClipCursor(0);

		/* カーソル位置を復元 */
		SetCursorPos(nMouseSX, nMouseSY);
	}

	/* キャプチャ状態を保存 */
	bCapture = en;

	/* マウス移動距離をクリア */
	nMouseX = 0;
	nMouseY = 0;

	/* デバイス状態を空読み */
	if (lpmos) {
		hr = lpmos->Acquire();
		if ((hr == DI_OK) || (hr == S_FALSE)) {
			lpmos->GetDeviceState(sizeof(DIMOUSESTATE), &buf);
		}
	}
}

/*
 *	マウス
 *	移動データリクエスト
 */
void FASTCALL mospos_request(BYTE *move_x, BYTE *move_y)
{
	if (bCapture) {
		/* 移動距離を符号付き８ビットの範囲に収める */
		if (nMouseX > 127) {
			nMouseX = 127;
		}
		else if (nMouseX < -127) {
			nMouseX = -127;
		}

		if (nMouseY > 127) {
			nMouseY = 127;
		}
		else if (nMouseY < -127) {
			nMouseY = -127;
		}

		*move_x = (BYTE)nMouseX;
		*move_y = (BYTE)nMouseY;
	}
	else {
		/* キャプチャ一時停止中は移動していないことにする */
		*move_x = 0;
		*move_y = 0;
	}

	/* マウス移動距離をクリア */
	nMouseX = 0;
	nMouseY = 0;
}

/*
 *	マウス
 *	ボタンデータリクエスト
 */
BYTE FASTCALL mosbtn_request(void)
{
	/* ボタン情報を返す */
	return nMouseButton;
}

#endif	/* MOUSE */

#endif	/* _WIN32 */
