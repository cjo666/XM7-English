/*
 *	FM-7 EMULATOR "XM7"
 *
 *	Copyright (C) 1999-2017 ＰＩ．(Twitter:@xm6_original)
 *	Copyright (C) 2001-2017 Ryu Takegami (Twitter:@RyuTakegami)
 *
 *	[ Win32API コンフィギュレーション ]
 *
 *	RHG履歴
 *	  2001.07.27		ジョイスティックのデフォルト設定を「なし」に変更
 *	  2001.12.26		サウンドバッファ長の設定範囲を40ms～2000msに変更
 *	  2002.03.10		設定終了後の再描画結果がおかしい問題を修正
 *	  2002.04.05		FM音源の48kHz/88.2kHz/96kHz合成設定に対応
 *	  2002.05.29		CPU速度/サウンドバッファサイズ/BEEP周波数の直接指定に
 *						対応
 *	  2002.06.15		動作機種を変更した場合はVMをリセットするように変更
 *	  2002.09.08		キー入力ダイアログでIMEの動作を禁止するように修正
 *	  2002.09.09		INIファイルのサイクルスチール設定を内部の変数名表記に
 *						合わせて"CycleSteal"に変更
 *	  2002.09.12		ファイル選択デフォルトディレクトリの保存に対応
 *	  2002.09.16		設定ページごとのページ作成関数を廃止
 *	  2002.10.21		デバッグバージョンコンパイル時に全般ページ初期化処理で
 *						落ちる問題を修正
 *	  2002.12.09		V2憑きでチャンネルコールの説明が出ない問題を修正
 *	  2003.01.19		サウンドバッファのデフォルトサイズを100msに変更
 *						F10に割り当てたキーのF11への振り替えを廃止
 *	  2003.01.21		Keyboardセクション保存まわりをコッソリ変更(ｗ
 *	  2003.03.09		ファイル選択デフォルトディレクトリの種類別保存に対応
 *	  2003.05.02		サブCPUのデフォルト実行サイクル数を変更
 *	  2003.06.03		メインCPU・サブCPUのデフォルト実行サイクル数を微調整
 *	  2003.10.21		フルスキャンモードをウィンドウモードとフルスクリーン
 *						モードで別々に設定できるようにした
 *	  2004.03.17		疑似400ラインモード関連の設定項目を追加
 *	  2004.10.06		400ラインカード関連の設定項目を追加
 *	  2005.10.15		マウスモード切り替え操作の設定項目を追加
 *	  2005.11.12		キーマップ設定にFM-8でのキー名称を追加
 *	  2010.01.23		音量調整ページを実装
 *	  2010.04.24		起動モードの保存に対応
 *	  2012.05.01		フルスクリーン状態保存処理への対応
 *	  2012.05.28		カーソルキーによるテンキーエミュレーションのチェックが
 *						入らない問題を修正
 *	  2012.06.03		ステータスバーの表示状態の保存に対応
 *	  2012.07.01		バブルメモリ関連の項目を追加
 *	  2012.08.01		フルスクリーン時の画面拡大に対応
 */

#ifdef _WIN32

#define STRICT
#define WIN32_LEAN_AND_MEAN
#define NONAMELESSUNION
#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include <prsht.h>
#ifdef DINPUT8
#define DIRECTINPUT_VERSION		0x0800		/* DirectX8を指定 */
#else
#define DIRECTINPUT_VERSION		0x0300		/* DirectX3を指定 */
#endif
#include <dinput.h>
#include <mmsystem.h>
#include <stdlib.h>
#include <assert.h>
#include "xm7.h"
#include "device.h"
#include "mainetc.h"
#include "subctrl.h"
#include "fdc.h"
#include "tapelp.h"
#include "opn.h"
#include "keyboard.h"
#include "mmr.h"
#include "mouse.h"
#include "aluline.h"
#include "display.h"
#include "kanji.h"
#include "jcard.h"
#include "jsubsys.h"
#include "rs232c.h"
#include "bubble.h"
#include "w32.h"
#include "w32_bar.h"
#include "w32_cfg.h"
#include "w32_sch.h"
#include "w32_snd.h"
#include "w32_kbd.h"
#include "w32_dd.h"
#include "w32_draw.h"
#include "w32_gdi.h"
#include "w32_sub.h"
#include "w32_res.h"
#include "w32_comm.h"
#include "w32_midi.h"
#include "juliet.h"
#include "w32_lpr.h"


/*
 *	設定データ定義
 */
typedef struct {
	int fm7_ver;						/* ハードウェアバージョン */
	int boot_mode;						/* ブートモード */
	BOOL cycle_steal;					/* サイクルスチールフラグ */
	BOOL subclock_mode;					/* サブCPU 非ブランキング時タイミング */
	DWORD main_speed;					/* メインCPUスピード */
	DWORD mmr_speed;					/* メインCPU(MMR)スピード */
#if XM7_VER >= 3
	DWORD fmmr_speed;					/* メインCPU(高速MMR)スピード */
#endif
	DWORD sub_speed;					/* サブCPUスピード */
	DWORD uTimerResolution;				/* マルチメディアタイマー精度 */
	BOOL bTapeFull;						/* テープモータ時の速度フラグ */
#if !defined(DISABLE_FULLSPEED)
	BOOL bCPUFull;						/* 全力駆動フラグ */
	BOOL bSpeedAdjust;					/* 自動速度調整 */
#endif
	BOOL bTapeMode;						/* テープモータ速度制御タイプ */
#if XM7_VER == 1
	BYTE fm_subtype;					/* ハードウェアサブバージョン */
	BOOL lowspeed_mode;					/* CPU動作クロックモード */
	DWORD main_speed_low;				/* メインCPUスピード(低速) */
	DWORD sub_speed_low;				/* サブCPUスピード(低速) */
#if defined(JSUB)
	DWORD jsub_speed;					/* 日本語サブCPUスピード */
#endif
#endif

	int nSampleRate;					/* サンプリングレート */
	int nSoundBuffer;					/* サウンドバッファサイズ */
	int nBeepFreq;						/* BEEP周波数 */
	BOOL bInterpolation;				/* サウンド補間出力 */
	int uStereoOut;						/* 出力モード */
	BOOL bForceStereo;					/* 強制ステレオ出力 */
	BOOL bTapeMon;						/* テープ音モニタ */
#if defined(ROMEO)
	BOOL bUseRomeo;						/* ろみお使用フラグ */
#endif
	UINT uChSeparation;					/* ステレオチャンネルセパレーション */
	int nFMVolume;						/* FM音源ボリューム */
	int nPSGVolume;						/* PSGボリューム */
	int nBeepVolume;					/* BEEP音ボリューム */
	int nCMTVolume;						/* CMT音モニタボリューム */
	int nWaveVolume;					/* 各種効果音ボリューム */

	BYTE KeyMap[256];					/* キー変換テーブル */
	BOOL bKbdReal;						/* 擬似リアルタイムキースキャン */
	BOOL bTenCursor;					/* 方向キーをテンキーに対応 */
	BOOL bArrow8Dir;					/* テンキー変換 8方向モード */
#if defined(KBDPASTE)
	UINT uPasteWait;					/* 貼り付け時の文字単位待ち時間(ms) */
	UINT uPasteWaitCntl;				/* 貼り付け時のコントロールコード単位待ち時間(ms) */
#endif

	int nJoyType[2];					/* ジョイスティックタイプ */
	int nJoyRapid[2][2];				/* ジョイスティック連射 */
	int nJoyCode[2][7];					/* ジョイスティックコード */

	BYTE nDDResolutionMode;				/* フルスクリーン時の解像度 */
	BOOL bFullScan;						/* フルスキャン(ウィンドウモード) */
	BOOL bFullScanFS;					/* フルスキャン(フルスクリーン) */
	BOOL bFullScreen;					/* フルスクリーンモード */
	BOOL bDoubleSize;					/* 2倍表示フラグ */
	BOOL bDD480Status;					/* 640x480上下ステータス */
	BOOL bDDtruecolor;					/* TrueColor優先フラグ */
	BOOL bRasterRender;					/* ラスタレンダリング */
	BOOL bDrawAfterVSYNC;				/* 描画VSYNCタイミング */
	BOOL bHideStatus;					/* ステータスバー非表示 */
#if XM7_VER == 1
	BOOL bGreenMonitor;					/* グリーンモニタモード */
#endif
#if XM7_VER == 2
	BOOL bTTLMonitor;					/* TTLモニタモード */
#endif
	BOOL bPseudo400Line;				/* 疑似400ラインモード */

	BOOL bOPNEnable;					/* OPN有効フラグ(7 only) */
	BOOL bWHGEnable;					/* WHG有効フラグ */
	BOOL bTHGEnable;					/* THG有効フラグ */
#if XM7_VER == 1
	BOOL bFMXEnable;					/* FM-X PSG有効フラグ */
#endif
#if XM7_VER >= 2
	BOOL bDigitizeEnable;				/* ディジタイズ有効フラグ */
	BOOL bJCardEnable;					/* 日本語カード有効フラグ */
#endif
#if ((XM7_VER >= 3) || defined(FMTV151))
	BOOL bExtRAMEnable;					/* 拡張RAM有効フラグ */
	BYTE uExtRAMMode;					/* 拡張RAM動作モード */
#endif
#if XM7_VER == 1
#if defined(L4CARD)
	BOOL b400LineCardEnable;			/* 400ラインカード有効フラグ */
	BYTE uExtRAMMode;					/* 拡張RAM動作モード */
#endif
#if defined(JSUB)
	BOOL bJSubEnable;					/* 日本語サブシステム有効フラグ */
#endif
#if defined(BUBBLE)
	BOOL bBubbleEnable;					/* バブルメモリ有効フラグ */
#endif
	BYTE uBankSelectEnable;				/* バンク切り換えイネーブルフラグ */
#endif
#if defined(MOUSE)
	BOOL bMouseCapture;					/* マウスキャプチャフラグ */
	BYTE nMousePort;					/* マウス接続ポート */
	BYTE uMidBtnMode;					/* 中央ボタン状態取得モード */
#endif

#if defined(FDDSND)
	BOOL bFddWait;						/* FDDウェイト */
	BOOL bFddSound;						/* FDDシークサウンド */
#endif

#if defined(RSC)
	BOOL bCommPortEnable;				/* RS-232Cエミュレーション有効フラグ */
	BYTE uCommPortBps;					/* シリアルポート通信速度 */
	int nCommPortNo;					/* シリアルポート番号 */
#endif

#if defined(MIDI)
	char szMidiDevice[256];				/* MIDIデバイス名 */
	int nMidiDelay;						/* MIDI発音遅延時間 */
	BOOL bMidiDelayMode;				/* MIDI遅延をSoundBufferに合わせる */
#endif

#if defined(LPRINT)
	BYTE uPrinterEnable;				/* プリンタエミュレーションモード */
	BOOL bLprUseOsFont;					/* OSのフォントを利用する */
	BOOL bLprOutputKanji;				/* 漢字を出力する */
	char szLprLogPath[256+1];			/* プリンタログ出力パス */
#endif

#if XM7_VER == 1
	BOOL bPcgFlag;						/* PCGフラグ */
#endif
#if XM7_VER >= 2
	BOOL bLineBoost;					/* 直線補間全速力描画フラグ */
#endif
#if XM7_VER >= 3
	BOOL bGravestone;					/* !? */
	BOOL b400LineTiming;				/* ??? */
	BOOL bSubModeFix;					/* FM77AV用OS-9対策フラグ */
#endif

	BOOL bPopupSwnd;					/* サブウィンドウポップアップ状態 */
	BOOL bOFNCentering;					/* ファイルダイアログのセンタリング */
	BOOL bMagusPatch;					/* MAGUS対策処理 */
	BOOL bRomRamWrite;					/* FM-7モード時の裏RAM書込挙動変更 */
	BOOL bFdcEnable;					/* FDCイネーブル */
	BOOL bExtDetDisable;				/* EXTDETディセーブル */
#if XM7_VER == 1
	BOOL bMotorOnLowSpeed;				/* CMTモータON時強制低速モード */
#endif
#if defined(KBDPASTE)
	BOOL bKeyStrokeModeless;			/* キー入力支援ダイアログモード */
#endif
} configdat_t;

/*
 *	スタティック ワーク
 */
static UINT uPropertyState;				/* プロパティシート進行状況 */
static UINT uPropertyHelp;				/* ヘルプID */
static UINT KbdPageSelectID;			/* キーボードダイアログ */
static UINT KbdPageCurrentKey;			/* キーボードダイアログ */
static BYTE KbdPageMap[256];			/* キーボードダイアログ */
static UINT JoyPageIdx;					/* ジョイスティックページ */ 
static configdat_t configdat;			/* コンフィグ用データ */
static configdat_t propdat;				/* プロパティシート用データ */
static char szIniFile[_MAX_PATH];		/* INIファイル名 */
static char *pszSection;				/* セクション名 */


/*
 *	プロトタイプ宣言
 */
static void FASTCALL SheetInit(HWND hDlg);

/*
 *	コモンコントロールへのアクセスマクロ
 */
#define UpDown_GetPos(hwnd) \
	(DWORD)SendMessage((hwnd), UDM_GETPOS, 0, 0L)

#define UpDown_SetPos(hwnd, nPos) \
	SendMessage((hwnd), UDM_SETPOS, 0, MAKELPARAM(nPos, 0))

#define UpDown_SetRange(hwnd, nUpper, nLower) \
	SendMessage((hwnd), UDM_SETRANGE, 0, MAKELPARAM(nUpper, nLower))

/*
 *	パス保存用キー名
 */
static const char *InitDirStr[] = {
	"DiskImageDir",
	"TapeImageDir",
	"StateFileDir",
	"BMPFileDir",
	"WAVFileDir",
#if XM7_VER == 1
#if defined(BUBBLE)
	"BubbleImageDir",
#endif
#endif
	"LptLogFileDir",
};

/*-[ 設定データ ]-----------------------------------------------------------*/

/*
 *	設定データ
 *	ファイル名指定
 */
static void FASTCALL SetCfgFile(void)
{
	char path[_MAX_PATH];
	char drive[_MAX_DRIVE];
	char dir[_MAX_DIR];
	char fname[_MAX_FNAME];

	/* INIファイル名設定 */
	GetModuleFileName(NULL, path, sizeof(path));
	_splitpath(path, drive, dir, fname, NULL);
	strncpy(path, drive, sizeof(path));
	strncat(path, dir, sizeof(path) - strlen(path) - 1);
	strncat(path, fname, sizeof(path) - strlen(path) - 1);
	strncat(path, ".INI", sizeof(path) - strlen(path) - 1);

	strncpy(szIniFile, path, sizeof(szIniFile));
}

/*
 *	設定データ
 *	セクション名指定
 */
static void FASTCALL SetCfgSection(char *section)
{
	ASSERT(section);

	/* セクション名設定 */
	pszSection = section;
}

/*
 *	設定データ
 *	ロード(文字列)
 */
static BOOL LoadCfgString(char *key, char *buf, int length)
{
	ASSERT(key);

	GetPrivateProfileString(pszSection, key, ";", buf, length, szIniFile);

	if (buf[0] == ';') {
		return FALSE;
	}
	return TRUE;
}

/*
 *	設定データ
 *	ロード(int)
 */
static int LoadCfgInt(char *key, int def)
{
	ASSERT(key);

	return (int)GetPrivateProfileInt(pszSection, key, def, szIniFile);
}

/*
 *	設定データ
 *	ロード(BOOL)
 */
static BOOL FASTCALL LoadCfgBool(char *key, BOOL def)
{
	int dat;

	ASSERT(key);

	/* 読み込み */
	if (def) {
		dat = LoadCfgInt(key, 1);
	}
	else {
		dat = LoadCfgInt(key, 0);
	}

	/* 評価 */
	if (dat != 0) {
		return TRUE;
	}
	else {
		return FALSE;
	}
}

/*
 *	設定データ
 *	ロード
 */
void FASTCALL LoadCfg(void)
{
	int i;
	int j;
	char string[128];
	char path[_MAX_PATH];
	char dir[_MAX_DIR];
	char InitDir[_MAX_DIR + _MAX_PATH];
	BOOL flag;
	static const int JoyTable[] = {
		0x70, 0x71, 0x72, 0x73, 0, 0x74, 0x75
	};

	SetCfgFile();

	/* Generalセクション */
	SetCfgSection("General");
	if (!LoadCfgString("Directory", InitDir, MAX_PATH)) {
		GetModuleFileName(NULL, path, sizeof(path));
		_splitpath(path, InitDir, dir, NULL, NULL);
		if (dir[strlen(dir)-1] == '\\') {
			/* 最後のパス区切り記号は強制的に削る */
			dir[strlen(dir)-1] = '\0';
		}
		strncat(InitDir, dir, sizeof(InitDir) - strlen(InitDir) - 1);
	}
#if XM7_VER == 1
#if defined(BUBBLE)
#if defined(LPRINT)
	for (i=0; i<7; i++) {
#else
	for (i=0; i<6; i++) {
#endif
#else
	for (i=0; i<5; i++) {
#endif
#else
#if defined(LPRINT)
	for (i=0; i<6; i++) {
#else
	for (i=0; i<5; i++) {
#endif
#endif
		if (!LoadCfgString((char *)InitDirStr[i], InitialDir[i], MAX_PATH)) {
			strncpy(InitialDir[i], InitDir, sizeof(InitDir));
		}
	}

#if XM7_VER >= 2
#if XM7_VER >= 3
	configdat.fm7_ver = LoadCfgInt("Version", 3);
	if ((configdat.fm7_ver < 1) || (configdat.fm7_ver > 3)) {
		configdat.fm7_ver = 3;
	}
#else
	configdat.fm7_ver = LoadCfgInt("Version", 2);
	if ((configdat.fm7_ver < 1) || (configdat.fm7_ver > 2)) {
		configdat.fm7_ver = 2;
	}
#endif
#else
	configdat.fm7_ver = 1;
	configdat.fm_subtype = (BYTE)LoadCfgInt("SubVersion", FMSUB_FM77);
	configdat.lowspeed_mode = LoadCfgBool("LowSpeedMode", FALSE);
#endif
	configdat.boot_mode = LoadCfgInt("BootMode", BOOT_BASIC);
#if XM7_VER == 1 
#if defined(BUBBLE)
	if ((configdat.boot_mode < BOOT_BASIC) || (configdat.boot_mode > BOOT_BUBBLE)) {
		configdat.boot_mode = BOOT_BASIC;
	}
	if (configdat.boot_mode == BOOT_BUBBLE && !bubble_available) {
		configdat.boot_mode = BOOT_DOS;
	}
#else
	if ((configdat.boot_mode < BOOT_BASIC) || (configdat.boot_mode > BOOT_DOS)) {
		configdat.boot_mode = BOOT_BASIC;
	}
#endif
#else
	if ((configdat.boot_mode < BOOT_BASIC) || (configdat.boot_mode > BOOT_DOS)) {
		configdat.boot_mode = BOOT_BASIC;
	}
#endif

	i = LoadCfgInt("CycleSteel", 9999);
	if ((i < 0) || (i > 1)) {
		configdat.cycle_steal = LoadCfgBool("CycleSteal", TRUE);
	}
	else {
		configdat.cycle_steal = (BOOL)i;
	}
	configdat.subclock_mode = LoadCfgBool("SubClockMode", FALSE);
	configdat.main_speed = LoadCfgInt("MainSpeed", MAINCYCLES);
	if ((configdat.main_speed < 1) || (configdat.main_speed > 9999)) {
		configdat.main_speed = MAINCYCLES;
	}
	configdat.mmr_speed = LoadCfgInt("MMRSpeed", MAINCYCLES_MMR);
	if ((configdat.mmr_speed < 1) || (configdat.mmr_speed > 9999)) {
		configdat.mmr_speed = MAINCYCLES_MMR;
	}
#if XM7_VER >= 3
	configdat.fmmr_speed = LoadCfgInt("FastMMRSpeed", MAINCYCLES_FMMR);
	if ((configdat.fmmr_speed < 1) || (configdat.fmmr_speed > 9999)) {
		configdat.fmmr_speed = MAINCYCLES_FMMR;
	}
#endif
	configdat.sub_speed = LoadCfgInt("SubSpeed", SUBCYCLES);
	if ((configdat.sub_speed < 1) || (configdat.sub_speed > 9999)) {
		configdat.sub_speed = SUBCYCLES;
	}
#if XM7_VER == 1
	configdat.main_speed_low = LoadCfgInt("MainSpeedLow", MAINCYCLES_LOW);
	if ((configdat.main_speed_low < 1) || (configdat.main_speed_low > 9999)) {
		configdat.main_speed_low = MAINCYCLES_LOW;
	}
	configdat.sub_speed_low = LoadCfgInt("SubSpeedLow", SUBCYCLES_LOW);
	if ((configdat.sub_speed_low < 1) || (configdat.sub_speed_low > 9999)) {
		configdat.sub_speed_low = SUBCYCLES_LOW;
	}
#if defined(JSUB)
	configdat.jsub_speed = LoadCfgInt("JsubSpeed", JSUBCYCLES);
	if ((configdat.jsub_speed < 1) || (configdat.jsub_speed > 9999)) {
		configdat.jsub_speed = SUBCYCLES;
	}
#endif
#endif
	configdat.bTapeFull = LoadCfgBool("TapeFullSpeed", TRUE);
#if !defined(DISABLE_FULLSPEED)
	configdat.bCPUFull = LoadCfgBool("FullSpeed", FALSE);
	configdat.bSpeedAdjust = LoadCfgBool("AutoSpeedAdjust", FALSE);
#endif
	configdat.bTapeMode = LoadCfgBool("TapeFullSpeedMode", FALSE);
#if defined(FDDSND)
	configdat.bFddWait = LoadCfgInt("FDDWait", FALSE);
#endif

	/* Generalセクション(ウィンドウ位置) */
	WinPos.x = LoadCfgInt("WindowPosX", -99999);
	WinPos.y = LoadCfgInt("WindowPosY", -99999);

	/* 全般ページ(隠し) */
	bHighPriority = LoadCfgBool("HighPriority", FALSE);
	configdat.uTimerResolution = LoadCfgInt("TimerResolution", 1);
	if ((configdat.uTimerResolution < 1) || (configdat.uTimerResolution > 10)) {
		configdat.uTimerResolution = 1;
	}

	/* Soundセクション */
	SetCfgSection("Sound");
	configdat.nSampleRate = LoadCfgInt("SampleRate", 44100);
	if ((configdat.nSampleRate != 0) &&
		(configdat.nSampleRate != 22050) &&
		(configdat.nSampleRate != 25600) &&
		(configdat.nSampleRate != 44100) &&
		(configdat.nSampleRate != 48000) &&
		(configdat.nSampleRate != 51200) &&
		(configdat.nSampleRate != 88200) &&
		(configdat.nSampleRate != 96000)) {
		configdat.nSampleRate = 44100;
	}
	configdat.nSoundBuffer = LoadCfgInt("SoundBuffer", 100);
	if ((configdat.nSoundBuffer < 40) || (configdat.nSoundBuffer > 1000)) {
		configdat.nSoundBuffer = 100;
	}
	configdat.nBeepFreq = LoadCfgInt("BeepFreq", 1200);
	if ((configdat.nBeepFreq < 100) || (configdat.nBeepFreq > 9999)) {
		configdat.nBeepFreq = 1200;
	}
	configdat.bInterpolation = LoadCfgBool("FMHQmode", TRUE);
	configdat.uStereoOut = LoadCfgInt("StereoOut", 0);
	if ((configdat.uStereoOut < 0) || (configdat.uStereoOut > 4)) {
		configdat.uStereoOut = 0;
	}
	configdat.bTapeMon = LoadCfgBool("TapeMon", FALSE);
	configdat.bForceStereo = LoadCfgInt("ForceStereoOutput", FALSE);
#if defined(ROMEO)
	configdat.bUseRomeo = LoadCfgBool("UseROMEO", TRUE);
#endif
#if defined(FDDSND)
	configdat.bFddSound = LoadCfgBool("FDDSound", FALSE);
#endif
	configdat.uChSeparation = LoadCfgInt("ChannelSeparation",
		CHSEPARATION_DEFAULT);
	if (configdat.uChSeparation > 16) {
		configdat.uChSeparation = CHSEPARATION_DEFAULT;
	}
	configdat.nFMVolume = LoadCfgInt("FMVolume", FMVOLUME_DEFAULT);
	if ((configdat.nFMVolume < -96) || (configdat.nFMVolume > 10)) {
		configdat.nFMVolume = FMVOLUME_DEFAULT;
	}
	configdat.nPSGVolume = LoadCfgInt("PSGVolume", PSGVOLUME_DEFAULT);
	if ((configdat.nPSGVolume < -96) || (configdat.nPSGVolume > 10)) {
		configdat.nPSGVolume = PSGVOLUME_DEFAULT;
	}
	configdat.nBeepVolume = LoadCfgInt("BeepVolume", BEEPVOLUME_DEFAULT);
	if ((configdat.nBeepVolume < -96) || (configdat.nBeepVolume > 0)) {
		configdat.nBeepVolume = BEEPVOLUME_DEFAULT;
	}
	configdat.nCMTVolume = LoadCfgInt("CMTVolume", CMTVOLUME_DEFAULT);
	if ((configdat.nCMTVolume < -96) || (configdat.nCMTVolume > 0)) {
		configdat.nCMTVolume = CMTVOLUME_DEFAULT;
	}
#if defined(FDDSND)
	configdat.nWaveVolume = LoadCfgInt("WaveVolume", WAVEVOLUME_DEFAULT);
	if ((configdat.nWaveVolume < -96) || (configdat.nWaveVolume > 0)) {
		configdat.nWaveVolume = WAVEVOLUME_DEFAULT;
	}
#endif

	/* Keyboardセクション */
	SetCfgSection("Keyboard");
	configdat.bKbdReal = LoadCfgBool("RealTimeKeyScan", FALSE);
	configdat.bTenCursor = LoadCfgBool("UseArrowFor10Key", FALSE);
	configdat.bArrow8Dir = LoadCfgBool("Arrow8Dir", TRUE);
	flag = FALSE;
	for (i=0; i<256; i++) {
		_snprintf(string, sizeof(string), "Key%d", i);
		j = i;
		configdat.KeyMap[j] = (BYTE)LoadCfgInt(string, 0);
		/* どれか一つでもロードできたら、ok */
		if (configdat.KeyMap[j] != 0) {
			flag = TRUE;
		}
	}
	/* フラグが降りていれば、デフォルトのマップをもらう */
	if (!flag) {
		GetDefMapKbd(configdat.KeyMap, 0);
	}
#if defined(KBDPASTE)
	configdat.uPasteWait = LoadCfgInt("PasteWait", 0);
	configdat.uPasteWaitCntl = LoadCfgInt("PasteWaitCntl", 0);
	configdat.bKeyStrokeModeless = LoadCfgBool("KeyStrokeModeless", FALSE);
#endif

	/* JoyStickセクション */
	SetCfgSection("JoyStick");
	for (i=0; i<2; i++) {
		_snprintf(string, sizeof(string), "Type%d", i);
		configdat.nJoyType[i] = LoadCfgInt(string, 0);
		if ((configdat.nJoyType[i] < 0) || (configdat.nJoyType[i] > 4)) {
			configdat.nJoyType[i] = 0;
		}

		for (j=0; j<2; j++) {
			_snprintf(string, sizeof(string), "Rapid%d", i * 10 + j);
			configdat.nJoyRapid[i][j] = LoadCfgInt(string, 0);
			if ((configdat.nJoyRapid[i][j] < 0) || (configdat.nJoyRapid[i][j] > 9)) {
				configdat.nJoyRapid[i][j] = 0;
			}

		}

		flag = TRUE;
		for (j=0; j<7; j++) {
			_snprintf(string, sizeof(string), "Code%d", i * 10 + j);
			configdat.nJoyCode[i][j] = LoadCfgInt(string, -1);
			if ((configdat.nJoyCode[i][j] < 0) || (configdat.nJoyCode[i][j] > 0x75)) {
				flag = FALSE;
			}
		}
		/* レンジエラーなら初期値設定 */
		if (!flag) {
			for (j=0; j<7; j++) {
				configdat.nJoyCode[i][j] = JoyTable[j];
			}
		}
	}

	/* Screenセクション */
	SetCfgSection("Screen");
	configdat.nDDResolutionMode = (BYTE)LoadCfgInt("DD480Line", 255);
	if (configdat.nDDResolutionMode == 255) {
		configdat.nDDResolutionMode = (BYTE)LoadCfgInt("DDResolution", 1);
	}
	if (configdat.nDDResolutionMode >= 5) {
		configdat.nDDResolutionMode = DDRES_480LINE;
	}
	configdat.bFullScan = LoadCfgBool("FullScan", FALSE);
	configdat.bFullScanFS = LoadCfgBool("FullScanFS", FALSE);
	configdat.bFullScreen = LoadCfgBool("FullScreen", FALSE);
	configdat.bDoubleSize = LoadCfgBool("DoubleSize", FALSE);
	configdat.bDD480Status = LoadCfgBool("DD480Status", TRUE);
	configdat.bRasterRender = LoadCfgBool("RasterRender", FALSE);
	configdat.bDrawAfterVSYNC = LoadCfgBool("DrawAfterVSYNC", TRUE);
#if XM7_VER == 3
	configdat.bDDtruecolor = LoadCfgBool("DDTrueColor", TRUE);
#else
	configdat.bDDtruecolor = LoadCfgBool("DDTrueColor", FALSE);
#endif
	configdat.bHideStatus = LoadCfgBool("HideStatusBar", FALSE);
#if XM7_VER == 1
	configdat.bGreenMonitor = LoadCfgBool("GreenMonitor", FALSE);
#endif
#if XM7_VER == 2
	configdat.bTTLMonitor = LoadCfgBool("TTLMonitor", FALSE);
#endif
	configdat.bPseudo400Line = LoadCfgBool("Pseudo400Line", FALSE);

	/* Optionセクション */
	SetCfgSection("Option");
	configdat.bOPNEnable = LoadCfgBool("OPNEnable", TRUE);
	configdat.bWHGEnable = LoadCfgBool("WHGEnable", TRUE);
	configdat.bTHGEnable = LoadCfgBool("THGEnable", TRUE);
#if XM7_VER == 1
	configdat.bFMXEnable = LoadCfgBool("FMXEnable", FALSE);
#endif
#if XM7_VER >= 2
	configdat.bDigitizeEnable = LoadCfgBool("DigitizeEnable", TRUE);
	configdat.bJCardEnable = LoadCfgBool("JCardEnable", FALSE);
#endif
#if XM7_VER >= 3
	configdat.bExtRAMEnable = LoadCfgBool("ExtRAMEnable", FALSE);
	configdat.uExtRAMMode = (BYTE)LoadCfgInt("ExtRAMMode", 5);
	if ((configdat.uExtRAMMode < 4) || (configdat.uExtRAMMode > 5)) {
		configdat.uExtRAMMode = 5;
	}
#elif defined(FMTV151)
	configdat.bExtRAMEnable = bFMTV151;
#endif
#if XM7_VER == 1
#if defined(L4CARD)
	configdat.b400LineCardEnable = LoadCfgBool("400LineCardEnable", TRUE);
	configdat.uExtRAMMode = (BYTE)LoadCfgInt("ExtRAMMode", 2);
	if ((configdat.uExtRAMMode < 1) || (configdat.uExtRAMMode > 2)) {
		configdat.uExtRAMMode = 2;
	}
#endif
#if defined(JSUB)
	configdat.bJSubEnable = LoadCfgBool("JSubEnable", TRUE);
#endif
#if defined(BUBBLE)
	configdat.bBubbleEnable = LoadCfgBool("BubbleEnable", FALSE);
#endif
	configdat.uBankSelectEnable = LoadCfgInt("BankSelectEnable", 0);
#endif
#if defined(MOUSE)
	configdat.bMouseCapture = LoadCfgBool("MouseEmulation", FALSE);
	configdat.nMousePort = (BYTE)LoadCfgInt("MousePort", 1);
	if ((configdat.nMousePort < 1) || (configdat.nMousePort > 3)) {
		configdat.nMousePort = 1;
	}
	configdat.uMidBtnMode = (BYTE)LoadCfgInt("MidBtnMode", MOSCAP_NONE);
	if (configdat.uMidBtnMode > 2) {
		configdat.uMidBtnMode = 0;
	}
#endif

	/* Portsセクション */
#if defined(RSC) || defined(MIDI)
	SetCfgSection("Ports");
#if defined(RSC)
	configdat.bCommPortEnable = LoadCfgBool("CommPortEnable", FALSE);
	configdat.uCommPortBps = (BYTE)LoadCfgInt("CommPortBps", 0);
	if (configdat.uCommPortBps > 4) {
		configdat.uCommPortBps = 0;
	}
	configdat.nCommPortNo = LoadCfgInt("CommPort", 1);
	if ((configdat.nCommPortNo < 1) || (configdat.nCommPortNo > 16)) {
		configdat.nCommPortNo = 1;
	}
#endif

#if defined(MIDI)
	if (!LoadCfgString("MidiPort", configdat.szMidiDevice, 256)) {
		strncpy(configdat.szMidiDevice, "", sizeof(configdat.szMidiDevice));
	}
	configdat.nMidiDelay = LoadCfgInt("MidiDelay", 100);
	if ((configdat.nMidiDelay < 0) || (configdat.nMidiDelay > 1000)) {
		configdat.nMidiDelay = 100;
	}
	configdat.bMidiDelayMode = LoadCfgBool("MidiDelayMode", TRUE);
#endif
#endif	/* RSC/MIDI */

	/* Printerセクション */
#if defined(LPRINT)
	SetCfgSection("Printer");
	configdat.uPrinterEnable = (BYTE)LoadCfgInt("PrinterEnable", 0);
#if defined(JASTSOUND)
	if (configdat.uPrinterEnable > LP_JASTSOUND) {
#else
	if (configdat.uPrinterEnable > LP_LOG) {
#endif
		configdat.uPrinterEnable = 0;
	}
	configdat.bLprUseOsFont = LoadCfgBool("LprUseOsFont", FALSE);
	configdat.bLprOutputKanji = LoadCfgBool("LprOutputKanji", FALSE);
	if (!LoadCfgString("LprOutPath", configdat.szLprLogPath, 256)) {
		strncpy(configdat.szLprLogPath, "\0", sizeof(configdat.szLprLogPath));
	}
#endif /* LPRINT */

	/* Miscセクション */
	SetCfgSection("Misc");
	configdat.bPopupSwnd = LoadCfgBool("PopupSwnd", TRUE);
#if XM7_VER == 3
	configdat.bSubModeFix = LoadCfgBool("SubModeFix", FALSE);
#endif
	configdat.bOFNCentering = LoadCfgBool("OFNCentering", FALSE);
	configdat.bMagusPatch = LoadCfgBool("MagusPatch", FALSE);
	configdat.bRomRamWrite = LoadCfgBool("RomRamWrite", FALSE);
	configdat.bFdcEnable = LoadCfgBool("FdcEnable", TRUE);
#if XM7_VER == 1
	configdat.bPcgFlag = LoadCfgBool("PCGFlag", FALSE);
	configdat.bMotorOnLowSpeed = LoadCfgBool("MotorOnLowSpeed", TRUE);
#endif

	/* Unofficialセクション */
	SetCfgSection("Unofficial");
	kanji_asis_flag = LoadCfgBool("KanjiAsIs", FALSE);
#if XM7_VER >= 2
	configdat.bLineBoost = LoadCfgBool("LineBoost", FALSE);
#endif
#if XM7_VER >= 3
	configdat.bGravestone = LoadCfgBool("Gravestone", FALSE);
	configdat.b400LineTiming = LoadCfgBool("400LineTiming", FALSE);
#endif
#if XM7_VER == 1 && defined(L4CARD)
	ankcg_force_internal = LoadCfgBool("ForceInternalFont", FALSE);
#endif
	configdat.bExtDetDisable = LoadCfgBool("ExtDetDisable", FALSE);
}

/*
 *	設定データ
 *	ロード(2倍拡大モード専用)
 */
BOOL FASTCALL LoadCfg_DoubleSize(void)
{
	/* 2倍拡大モードの状態を読み込んで返す */
	SetCfgFile();
	SetCfgSection("Screen");
	return LoadCfgBool("DoubleSize", FALSE);
}

/*
 *	設定データ
 *	ロード(言語設定モード専用)
 */
BOOL FASTCALL LoadCfg_LanguageMode(void)
{
	/* 2倍拡大モードの状態を読み込んで返す */
	SetCfgFile();
	SetCfgSection("Unofficial");
	return LoadCfgBool("LanguageMode", FALSE);
}

/*
 *	設定データ
 *	削除
 */
static void FASTCALL DeleteCfg(char *key)
{
	ASSERT(key);

	WritePrivateProfileString(pszSection, key, NULL, szIniFile);
}

/*
 *	設定データ
 *	セーブ(文字列)
 */
static void FASTCALL SaveCfgString(char *key, char *string)
{
	ASSERT(key);
	ASSERT(string);

	WritePrivateProfileString(pszSection, key, string, szIniFile);
}

/*
 *	設定データ
 *	セーブ(４バイトint)
 */
static void FASTCALL SaveCfgInt(char *key, int dat)
{
	char string[128];

	ASSERT(key);

	_snprintf(string, sizeof(string), "%d", dat);
	SaveCfgString(key, string);
}

/*
 *	設定データ
 *	セーブ(BOOL)
 */
static void FASTCALL SaveCfgBool(char *key, BOOL dat)
{
	ASSERT(key);

	if (dat) {
		SaveCfgInt(key, 1);
	}
	else {
		SaveCfgInt(key, 0);
	}
}

/*
 *	設定データ
 *	セーブ
 */
void FASTCALL SaveCfg(void)
{
	int i;
	int j;
	char string[128];

	SetCfgFile();

	/* Generalセクション */
	SetCfgSection("General");
	DeleteCfg("Directory");		/* V3.3L20で種別毎の保存に対応 */
	DeleteCfg("CycleSteel");	/* V3.2L01でCycleStealにキー名称を変更 */
#if XM7_VER == 1
#if defined(BUBBLE)
#if defined(LPRINT)
	for (i=0; i<7; i++) {
#else
	for (i=0; i<6; i++) {
#endif
#else
	for (i=0; i<5; i++) {
#endif
#else
#if defined(LPRINT)
	for (i=0; i<6; i++) {
#else
	for (i=0; i<5; i++) {
#endif
#endif
		SaveCfgString((char *)InitDirStr[i], InitialDir[i]);
	}
#if XM7_VER >= 2
	SaveCfgInt("Version", configdat.fm7_ver);
#else
	SaveCfgInt("SubVersion", configdat.fm_subtype);
	SaveCfgBool("LowSpeedMode", configdat.lowspeed_mode);
#endif
	SaveCfgInt("BootMode", configdat.boot_mode);
	SaveCfgBool("CycleSteal", configdat.cycle_steal);
	SaveCfgBool("SubClockMode", configdat.subclock_mode);
	SaveCfgInt("MainSpeed", configdat.main_speed);
	SaveCfgInt("MMRSpeed", configdat.mmr_speed);
#if XM7_VER >= 3
	SaveCfgInt("FastMMRSpeed", configdat.fmmr_speed);
#endif
	SaveCfgInt("SubSpeed", configdat.sub_speed);
#if XM7_VER == 1
	SaveCfgInt("MainSpeedLow", configdat.main_speed_low);
	SaveCfgInt("SubSpeedLow", configdat.sub_speed_low);
#if defined(JSUB)
	SaveCfgInt("JsubSpeed", configdat.jsub_speed);
#endif
#endif
	SaveCfgBool("TapeFullSpeed", configdat.bTapeFull);
	SaveCfgBool("TapeFullSpeedMode", configdat.bTapeMode);
#if !defined(DISABLE_FULLSPEED)
	SaveCfgBool("FullSpeed", configdat.bCPUFull);
	SaveCfgBool("AutoSpeedAdjust", configdat.bSpeedAdjust);
#endif
#if defined(FDDSND)
	SaveCfgBool("FDDWait", configdat.bFddWait);
#endif

	/* Generalセクション(ウィンドウ位置) */
	SaveCfgInt("WindowPosX", WinPos.x);
	SaveCfgInt("WindowPosY", WinPos.y);

	/* Soundセクション */
	SetCfgSection("Sound");
	SaveCfgInt("SampleRate", configdat.nSampleRate);
	SaveCfgInt("SoundBuffer", configdat.nSoundBuffer);
	SaveCfgInt("BeepFreq", configdat.nBeepFreq);
	SaveCfgBool("FMHQmode", configdat.bInterpolation);
	SaveCfgInt("StereoOut", configdat.uStereoOut);
	SaveCfgBool("TapeMon", configdat.bTapeMon);
#if defined(ROMEO)
	SaveCfgBool("UseROMEO", configdat.bUseRomeo);
#endif
#if defined(FDDSND)
	SaveCfgBool("FDDSound", configdat.bFddSound);
#endif
	SaveCfgInt("ChannelSeparation", configdat.uChSeparation);
	SaveCfgInt("FMVolume", configdat.nFMVolume);
	SaveCfgInt("PSGVolume", configdat.nPSGVolume);
	SaveCfgInt("BeepVolume", configdat.nBeepVolume);
	SaveCfgInt("CMTVolume", configdat.nCMTVolume);
#if defined(FDDSND)
	SaveCfgInt("WaveVolume", configdat.nWaveVolume);
#endif

	/* Keyboardセクション */
	SetCfgSection("Keyboard");
	SaveCfgBool("RealTimeKeyScan", configdat.bKbdReal);
	SaveCfgBool("UseArrowFor10Key", configdat.bTenCursor);
	SaveCfgBool("Arrow8Dir", configdat.bArrow8Dir);
	for (i=0; i<256; i++) {
		_snprintf(string, sizeof(string), "Key%d", i);
		DeleteCfg(string);
		if (configdat.KeyMap[i] != 0) {
			SaveCfgInt(string, (BYTE)(configdat.KeyMap[i] & 0x7f));
		}
	}
#if defined(KBDPASTE)
	SaveCfgInt("PasteWait", configdat.uPasteWait);
	SaveCfgInt("PasteWaitCntl", configdat.uPasteWaitCntl);
#endif

	/* JoyStickセクション */
	SetCfgSection("JoyStick");
	for (i=0; i<2; i++) {
		_snprintf(string, sizeof(string), "Type%d", i);
		SaveCfgInt(string, configdat.nJoyType[i]);

		for (j=0; j<2; j++) {
			_snprintf(string, sizeof(string), "Rapid%d", i * 10 + j);
			SaveCfgInt(string, configdat.nJoyRapid[i][j]);
		}

		for (j=0; j<7; j++) {
			_snprintf(string, sizeof(string), "Code%d", i * 10 + j);
			SaveCfgInt(string, configdat.nJoyCode[i][j]);
		}
	}

	/* Screenセクション */
	SetCfgSection("Screen");
	DeleteCfg("DD480Line");		/* V3.4L51でDDResolutionにキー名称を変更 */
	DeleteCfg("DrawTiming");	/* V3.4L60でラスタレンダリング搭載により廃止 */
	SaveCfgInt("DDResolution", configdat.nDDResolutionMode);
	SaveCfgBool("FullScan", configdat.bFullScan);
	SaveCfgBool("FullScanFS", configdat.bFullScanFS);
	SaveCfgBool("FullScreen", configdat.bFullScreen);
	SaveCfgBool("DoubleSize", configdat.bDoubleSize);
	SaveCfgBool("DD480Status", configdat.bDD480Status);
	SaveCfgBool("RasterRender", configdat.bRasterRender);
	SaveCfgBool("DrawAfterVSYNC", configdat.bDrawAfterVSYNC);
	SaveCfgBool("DDTrueColor", configdat.bDDtruecolor);
	SaveCfgBool("HideStatusBar", configdat.bHideStatus);
#if XM7_VER == 1
	SaveCfgBool("GreenMonitor", configdat.bGreenMonitor);
#endif
#if XM7_VER == 2
	SaveCfgBool("TTLMonitor", configdat.bTTLMonitor);
#endif
	SaveCfgBool("Pseudo400Line", configdat.bPseudo400Line);

	/* Optionセクション */
	SetCfgSection("Option");
	DeleteCfg("SubBusyDelay");	/* V3.1で廃止 */
	SaveCfgBool("OPNEnable", configdat.bOPNEnable);
	SaveCfgBool("WHGEnable", configdat.bWHGEnable);
	SaveCfgBool("THGEnable", configdat.bTHGEnable);
#if XM7_VER >= 2
	SaveCfgBool("DigitizeEnable", configdat.bDigitizeEnable);
	SaveCfgBool("JCardEnable", configdat.bJCardEnable);
#endif
#if XM7_VER >= 3
	SaveCfgBool("ExtRAMEnable", configdat.bExtRAMEnable);
	SaveCfgInt("ExtRAMMode", configdat.uExtRAMMode);
#endif
#if XM7_VER == 1
#if defined(L4CARD)
	SaveCfgBool("400LineCardEnable", configdat.b400LineCardEnable);
	SaveCfgInt("ExtRAMMode", configdat.uExtRAMMode);
#endif
#if defined(JSUB)
	SaveCfgBool("JSubEnable", configdat.bJSubEnable);
#endif
#if defined(BUBBLE)
	SaveCfgBool("BubbleEnable", configdat.bBubbleEnable);
#endif
	SaveCfgInt("BankSelectEnable", configdat.uBankSelectEnable);
#endif
#if defined(MOUSE)
	SaveCfgBool("MouseEmulation", configdat.bMouseCapture);
	SaveCfgInt("MousePort", configdat.nMousePort);
	SaveCfgInt("MidBtnMode", configdat.uMidBtnMode);
#endif

	/* Portsセクション */
#if defined(RSC) || defined(MIDI)
	SetCfgSection("Ports");
#if defined(RSC)
	SaveCfgBool("CommPortEnable", configdat.bCommPortEnable);
	SaveCfgInt("CommPortBps", configdat.uCommPortBps);
	SaveCfgInt("CommPort", configdat.nCommPortNo);
#endif
#if defined(MIDI)
	SaveCfgString("MidiPort", configdat.szMidiDevice);
	SaveCfgInt("MidiDelay", configdat.nMidiDelay);
	SaveCfgBool("MidiDelayMode", configdat.bMidiDelayMode);
#endif
#endif	/* RSC/MIDI */

	/* Printerセクション */
#if defined(LPRINT)
	SetCfgSection("Printer");
	SaveCfgInt("PrinterEnable", configdat.uPrinterEnable);
	SaveCfgBool("LprUseOsFont", configdat.bLprUseOsFont);
	SaveCfgBool("LprOutputKanji", configdat.bLprOutputKanji);
	SaveCfgString("LprOutPath", configdat.szLprLogPath);
#endif /* LPRINT */

	/* Miscセクション */
	SetCfgSection("Misc");
	SaveCfgBool("MagusPatch", configdat.bMagusPatch);
	SaveCfgBool("RomRamWrite", configdat.bRomRamWrite);
	SaveCfgBool("OFNCentering", configdat.bOFNCentering);
	SaveCfgBool("PopupSwnd", configdat.bPopupSwnd);
	SaveCfgBool("FdcEnable", configdat.bFdcEnable);
#if XM7_VER == 1
	SaveCfgBool("MotorOnLowSpeed", configdat.bMotorOnLowSpeed);
#endif
}

/*
 *	設定データ適用
 *	※VMのロックは行っていないので注意
 */
void FASTCALL ApplyCfg(void)
{
	RECT rect;
	int i;
	char tmp[128];
	char buffer[256+128+1];

	tmp[0] = '\0';
	buffer[0] = '\0';

	/* Generalセクション */
	fm7_ver = configdat.fm7_ver;
	boot_mode = configdat.boot_mode;
	cycle_steal_default = configdat.cycle_steal;
	subclock_mode = configdat.subclock_mode;
	main_speed = configdat.main_speed * 10;
	mmr_speed = configdat.mmr_speed * 10;
#if XM7_VER >= 3
	fmmr_speed = configdat.fmmr_speed * 10;
#endif
	sub_speed = configdat.sub_speed * 10;
	bTapeFullSpeed = configdat.bTapeFull;
#if !defined(DISABLE_FULLSPEED)
	bFullSpeed = configdat.bCPUFull;
	bAutoSpeedAdjust = configdat.bSpeedAdjust;
#endif
	uTimerResolution = configdat.uTimerResolution;
#if defined(FDDSND)
	fdc_waitmode = configdat.bFddWait;
#endif
	bTapeModeType = configdat.bTapeMode;
#if XM7_VER == 1
	/* 使用不可なモードが選択されている場合、強制的にモード変更 */
	if ((configdat.fm_subtype == FMSUB_FM8) && !available_fm8roms) {
		configdat.fm_subtype = FMSUB_FM77;
	}
	else if ((configdat.fm_subtype != FMSUB_FM8) && !available_fm7roms) {
		configdat.fm_subtype = FMSUB_FM8;
	}

	fm_subtype = configdat.fm_subtype;
	lowspeed_mode = configdat.lowspeed_mode;
	main_speed_low = configdat.main_speed_low * 10;
	sub_speed_low = configdat.sub_speed_low * 10;
#if defined(JSUB)
	jsub_speed = configdat.jsub_speed * 10;
#endif
#endif

	/* Soundセクション */
	nSampleRate = configdat.nSampleRate;
	nSoundBuffer = configdat.nSoundBuffer;
	nBeepFreq = configdat.nBeepFreq;
	bInterpolation = configdat.bInterpolation;
	uStereoOut = configdat.uStereoOut;
	bForceStereo = configdat.bForceStereo;
	bTapeMon = configdat.bTapeMon;
	tape_monitor = configdat.bTapeMon;
#if defined(ROMEO)
	if (bRomeo) {
		bUseRomeo = configdat.bUseRomeo;
		juliet_YMF288Mute(!bUseRomeo);
	}
	else {
		bUseRomeo = FALSE;
	}
#endif
	uChSeparation = configdat.uChSeparation;
	nFMVolume = configdat.nFMVolume;
	nPSGVolume = configdat.nPSGVolume;
	nBeepVolume = configdat.nBeepVolume;
	nCMTVolume = configdat.nCMTVolume;
#if defined(FDDSND)
	fdc_sound = configdat.bFddSound;
	tape_sound = configdat.bFddSound;
	nWaveVolume = configdat.nWaveVolume;
#endif
	ApplySnd();

	/* Keyboardセクション */
	SetMapKbd(configdat.KeyMap);
	bKbdReal = configdat.bKbdReal;
	bTenCursor = configdat.bTenCursor;
	bArrow8Dir = configdat.bArrow8Dir;
#if defined(KBDPASTE)
	uPasteWait = configdat.uPasteWait;
	uPasteWaitCntl = configdat.uPasteWaitCntl;
	bKeyStrokeModeless = configdat.bKeyStrokeModeless;
#endif

	/* JoyStickセクション */
	memcpy(nJoyType, configdat.nJoyType, sizeof(nJoyType));
	memcpy(nJoyRapid, configdat.nJoyRapid, sizeof(nJoyRapid));
	memcpy(nJoyCode, configdat.nJoyCode, sizeof(nJoyCode));

	/* Screenセクション */
	InvalidateRect(hDrawWnd, NULL, FALSE);
	nDDResolutionMode = configdat.nDDResolutionMode;
	bFullScan = configdat.bFullScan;
	bFullScanFS = configdat.bFullScanFS;
	if (bDrawSelected) {
		if (bFullScreen != configdat.bFullScreen) {
			PostMessage(hMainWnd, WM_COMMAND, IDM_FULLSCREEN, 0);
		}
	}
	else {
		bFullRequested = configdat.bFullScreen;
		bFullScreen = FALSE;
	}
	bDoubleSize = configdat.bDoubleSize;
	bDD480Status = configdat.bDD480Status;
	if (bRasterRendering != configdat.bRasterRender) {
		SelectCancelGDI();
	}
	bRasterRendering = configdat.bRasterRender;
	draw_aftervsync = configdat.bDrawAfterVSYNC;
	bDDtruecolor = configdat.bDDtruecolor;
	bHideStatus = configdat.bHideStatus;
#if XM7_VER == 1
	bGreenMonitor = configdat.bGreenMonitor;
#endif
#if XM7_VER == 2
	bTTLMonitor = configdat.bTTLMonitor;
#endif
	bPseudo400Line = configdat.bPseudo400Line;
#if defined(DISABLE_FULLSPEED)
	line_boost = configdat.bLineBoost;
#else
#if XM7_VER >= 2
	if (bFullSpeed) {
		line_boost = TRUE;
	}
	else {
		line_boost = configdat.bLineBoost;
	}
#endif
#endif

	/* Optionセクション */
	opn_enable = configdat.bOPNEnable;
	whg_enable = configdat.bWHGEnable;
	thg_enable = configdat.bTHGEnable;
#if XM7_VER == 1
	fmx_flag = configdat.bFMXEnable;
#endif
#if XM7_VER >= 2
	digitize_enable = configdat.bDigitizeEnable;
	if ((jcard_enable != configdat.bJCardEnable) && (fm7_ver == 2)) {
		jcard_enable = configdat.bJCardEnable;
		system_reset();
	}
	else {
		jcard_enable = configdat.bJCardEnable;
	}
#endif
#if XM7_VER >= 3
	mmr_extram = configdat.bExtRAMEnable;

	/* 拡張RAM動作モード(なぜかXM7dash互換) */
	if (configdat.uExtRAMMode == 5) {
		mmr_768kbmode = TRUE;
	}
	else {
		mmr_768kbmode = FALSE;
	}
#elif defined(FMTV151)
	bFMTV151 = configdat.bExtRAMEnable;
#endif
#if XM7_VER == 1
#if defined(L4CARD)
	if (detect_400linecard) {
		/* Enable→Disable時に400ラインモードになっている時は強制リセット */
		if (enable_400linecard && !configdat.b400LineCardEnable &&
			enable_400line) {
			system_reset();
			OnRefresh(hMainWnd);
		}
		enable_400linecard = configdat.b400LineCardEnable;
	}
	else {
		enable_400linecard = FALSE;
	}

	/* 拡張RAM動作モード(なぜかXM7dash互換) */
	if (configdat.uExtRAMMode == 1) {
		mmr_64kbmode = TRUE;
	}
	else {
		mmr_64kbmode = FALSE;
	}
#endif
#if defined(JSUB)
	if (jsub_available) {
		jsub_enable = configdat.bJSubEnable;
	}
	else {
		jsub_enable = FALSE;
	}
#endif
#if defined(BUBBLE)
	bmc_enable = configdat.bBubbleEnable;
#endif
	banksel_en = configdat.uBankSelectEnable;
#endif
#if defined(MOUSE)
	mos_capture = configdat.bMouseCapture;
	if (((mos_port >= 1) && (mos_port <= 2) && (configdat.nMousePort == 3)) ||
		 ((configdat.nMousePort >= 1) && (configdat.nMousePort <= 2) &&
		 (mos_port == 3))) {
		mos_port = configdat.nMousePort;
		system_reset();
	}
	else {
		mos_port = configdat.nMousePort;
	}
	uMidBtnMode = configdat.uMidBtnMode;
#endif

	/* Portsセクション */
#if defined(RSC)
	rs_use = configdat.bCommPortEnable;
	rs_baudrate_v2 = configdat.uCommPortBps;
	nCommPortNo = configdat.nCommPortNo;
#endif
#if defined(MIDI)
	strncpy(szMidiDevice, configdat.szMidiDevice, sizeof(szMidiDevice));
	nMidiDelay = configdat.nMidiDelay;
	bMidiDelayMode = configdat.bMidiDelayMode;
#endif

	/* Printerセクション */
#if defined(LPRINT)
	if ((lp_use != configdat.uPrinterEnable) ||
		((configdat.uPrinterEnable == LP_LOG) &&
		(strcmp(lp_fname, configdat.szLprLogPath) != 0))) {
		/* モード切り替え時のファイルクローズ/オープン処理をここで行う */
		lp_use = configdat.uPrinterEnable;
		switch (lp_use) {
			case LP_EMULATION:
				lp_setfile(LP_TEMPFILENAME);
				break;
			case LP_LOG:
				if (configdat.szLprLogPath[0] == '\0') {
					lp_setfile(NULL);
				}
				else {
					lp_setfile(configdat.szLprLogPath);
					if (lp_fileh == -1) {
						LoadString(hAppInstance, IDS_LPRLOGERROR,
							tmp, sizeof(tmp));
						_snprintf(buffer, sizeof(buffer), tmp,
							configdat.szLprLogPath);
						MessageBox(hMainWnd, buffer, "XM7",
							MB_ICONEXCLAMATION | MB_OK);
					}
				}
				break;
			case LP_JASTSOUND:
			case LP_DISABLE:
				lp_setfile(NULL);
				break;
			default:
				ASSERT(FALSE);
		}
	}

	lpr_use_os_font = configdat.bLprUseOsFont;
	lpr_output_kanji = configdat.bLprOutputKanji;
#endif

	/* Miscセクション */
	if (bPopupSwnd != configdat.bPopupSwnd) {
		for (i=0; i<SWND_MAXNUM; i++) {
			if (hSubWnd[i]) {
				DestroyWindow(hSubWnd[i]);
				hSubWnd[i] = NULL;
			}
		}
	}
	bPopupSwnd = configdat.bPopupSwnd;
#if XM7_VER == 1
	pcg_flag = configdat.bPcgFlag;
	motoron_lowspeed = configdat.bMotorOnLowSpeed;
#endif
#if XM7_VER == 3
	submode_fix = configdat.bSubModeFix;
#endif
	bOFNCentering = configdat.bOFNCentering;
	magus_patch = configdat.bMagusPatch;
	rom_ram_write = configdat.bRomRamWrite;
	fdc_enable = configdat.bFdcEnable;

	/* Unofficialセクション */
#if XM7_VER >= 3
	bGravestone = configdat.bGravestone;
	dsp_400linetiming = configdat.b400LineTiming;
#endif
	extdet_disable = configdat.bExtDetDisable;

	/* ステータスバーのサイズ調整 */
	if (hStatusBar) {
		if (bHideStatus) {
			/* 消去 */
			ShowWindow(hStatusBar, SW_HIDE);
		}
		else {
			/* 表示 */
			ShowWindow(hStatusBar, SW_SHOW);
		}

		/* フレームウインドウのサイズを補正 */
		GetClientRect(hMainWnd, &rect);
		PostMessage(hMainWnd, WM_SIZE, 0, MAKELPARAM(rect.right, rect.bottom));
	}

	/* 表示内容を更新 */
	if (hMainWnd) {
		OnSize(hMainWnd, 640, 400);
	}
	InvalidateRect(hDrawWnd, NULL, FALSE);
	display_notify();
}

/*
 *	設定データ取得
 */
void FASTCALL GetCfg(void)
{
	/* Generalセクション */
	configdat.fm7_ver = fm7_ver;
	configdat.boot_mode = boot_mode;
	configdat.cycle_steal = cycle_steal_default;
	configdat.subclock_mode = subclock_mode;
	configdat.main_speed = main_speed / 10;
	configdat.mmr_speed = mmr_speed / 10;
#if XM7_VER >= 3
	configdat.fmmr_speed = fmmr_speed / 10;
#endif
	configdat.sub_speed = sub_speed / 10;
	configdat.bTapeFull = bTapeFullSpeed;
#if !defined(DISABLE_FULLSPEED)
	configdat.bCPUFull = bFullSpeed;
	configdat.bSpeedAdjust = bAutoSpeedAdjust;
#endif
	configdat.bTapeMode = bTapeModeType;
#if XM7_VER == 1
	configdat.fm_subtype = fm_subtype;
	configdat.lowspeed_mode = lowspeed_mode;
	configdat.main_speed_low = main_speed_low / 10;
	configdat.sub_speed_low = sub_speed_low / 10;
#if defined(JSUB)
	configdat.jsub_speed = jsub_speed / 10;
#endif
#endif

	/* Screenセクション */
	configdat.bFullScreen = bFullScreen;
	configdat.bHideStatus = bHideStatus;

	/* Optionセクション */
	configdat.bOPNEnable = opn_enable;
	configdat.bWHGEnable = whg_enable;
	configdat.bTHGEnable = thg_enable;
#if XM7_VER >= 2
	configdat.bDigitizeEnable = digitize_enable;
#if XM7_VER == 2
	if (jcard_available) {
		configdat.bJCardEnable = jcard_enable;
	}
	else {
		configdat.bJCardEnable = FALSE;
	}
#else
	configdat.bJCardEnable = jcard_enable;
#endif
#endif
#if XM7_VER >= 3
	configdat.bExtRAMEnable = mmr_extram;
#endif
#if XM7_VER == 1
#if defined(L4CARD)
	if (detect_400linecard) {
		configdat.b400LineCardEnable = enable_400linecard;
	}
	else {
		configdat.b400LineCardEnable = FALSE;
	}
#endif
#if defined(JSUB)
	if (jsub_available) {
		configdat.bJSubEnable = jsub_enable;
	}
	else {
		configdat.bJSubEnable = FALSE;
	}
#endif
#if defined(BUBBLE)
	configdat.bBubbleEnable = bmc_enable;
#endif
	configdat.uBankSelectEnable = banksel_en;
#endif
}

/*
 *	動作機種再設定
 */
void FASTCALL SetMachineVersion(void)
{
	configdat.fm7_ver = fm7_ver;
#if XM7_VER == 1
	configdat.fm_subtype = fm_subtype;
#endif
}

/*-[ ヘルプサポート ]-------------------------------------------------------*/

/*
 *	サポートIDリスト
 *	(先に来るものほど優先)
 */
static const UINT PageHelpList[] = {
#if XM7_VER >= 3
	IDC_GP_FM7,
	IDC_GP_FM77AV,
	IDC_GP_AV40EX,
#elif XM7_VER == 1
	IDC_GP_FM8,
	IDC_GP_FM7,
	IDC_GP_FM77,
	IDC_GP_HIGHSPEED,
#else
	IDC_GP_MACHINEG,
#endif
	IDC_GP_HIGHSPEED,
	IDC_GP_MIDSPEED,
	IDC_GP_LOWSPEED,
	IDC_GP_TAPESPEED,
	IDC_GP_TAPESPEEDMODE,
#if !defined(DISABLE_FULLSPEED)
	IDC_GP_FULLSPEED,
	IDC_GP_AUTOSPEEDADJUST,
#endif
#if defined(FDDSND)
	IDC_GP_FDDWAIT,
#endif
	IDC_GP_CPUDEFAULT,
	IDC_GP_SPEEDG,
	IDC_SP_HQMODE,
	IDC_SP_RATEG,
	IDC_SP_BUFFERG,
	IDC_SP_STEREO,
	IDC_SP_TAPEMON,
#if defined(FDDSND)
	IDC_SP_FDDSOUND,
#endif
#if defined(ROMEO)
	IDC_SP_ROMEO,
#endif
	IDC_SP_BEEPG,
	IDC_VP_DEFAULT,
	IDC_VP_SEPARATIONG,
	IDC_VP_VOLUMEG,
	IDC_KP_LIST,
	IDC_KP_101B,
	IDC_KP_106B,
	IDC_KP_98B,
	IDC_KP_USEARROWFOR10,
	IDC_KP_ARROW8DIR,
	IDC_KP_KBDREAL,
	IDC_JP_PORTG,
	IDC_JP_TYPEG,
	IDC_JP_RAPIDG,
	IDC_JP_CODEG,
#if defined(MIDI)
	IDC_POP_MIDIDLY,
	IDC_POP_MIDIDLYSB,
	IDC_POP_MIDIG,
#endif
#if defined(RSC)
	IDC_POP_COMENABLE,
	IDC_POP_COMBPSC,
	IDC_POP_COMG,
#endif
#if defined(LPRINT)
	IDC_LPP_LPREMUENABLE,
	IDC_LPP_LPROSFNT,
	IDC_LPP_LPRKANJI,
	IDC_LPP_LPRLOGENABLE,
	IDC_LPP_LPRLOGPATH,
	IDC_LPP_LPRLOGPATHNAME,
	IDC_LPP_LPRLOGDIALOG,
#if defined(JASTSOUND)
	IDC_LPP_LPRJASTSOUNDENABLE,
#endif
	IDC_LPP_LPRDISABLE,
	IDC_LPP_LPRG,
#endif
	IDC_SCP_MODEC,
	IDC_SCP_24K,
	IDC_SCP_24KFS,
	IDC_SCP_DOUBLESIZE,
	IDC_SCP_CAPTIONB,
	IDC_SCP_TRUECOLOR,
	IDC_SCP_RASTERRENDER,
	IDC_SCP_PSEUDO400LINE,
	IDC_SCP_MODEG,
	IDC_OP_OPNB,
	IDC_OP_WHGB,
	IDC_OP_THGB,
	IDC_OP_DIGITIZEG,
#if XM7_VER == 1
#if defined(L4CARD)
	IDC_OP_400LINEG,
#endif
#if defined(JSUB)
	IDC_OP_JSUBG,
#endif
#if defined(BUBBLE)
	IDC_OP_BMCG,
#endif
	IDC_SCP_GREENMONITOR,
	IDC_AP_BANKSELB,
	IDC_AP_MOTORON_LOWSPEED,
#endif
#if XM7_VER == 2
	IDC_SCP_TTLMONITOR,
#endif
#if XM7_VER >= 2
	IDC_OP_JCARDG,
#endif
#if ((XM7_VER >= 3) || defined(FMTV151))
	IDC_OP_RAMG,
#endif
#if defined(MOUSE)
	IDC_OP_MOUSEEM,
	IDC_OP_MOUSESW,
	IDC_OP_MOUSEG,
#endif
	IDC_AP_MAGUSPATCH,
	IDC_AP_ROMRAMWRITE,
	IDC_AP_OFNCENTERING,
	IDC_AP_POPUPSWND,
	IDC_AP_FDCDISABLE,
	0
};

/*
 *	ページヘルプ
 */
static void FASTCALL PageHelp(HWND hDlg, UINT uID)
{
	POINT point;
	RECT rect;
	HWND hWnd;
	int i;
	char string[256];
	UINT uHelpID;

	ASSERT(hDlg);

	/* ポイント作成 */
	GetCursorPos(&point);

	/* ヘルプリストに載っているIDを回る */
	for (i=0; ;i++) {
		/* 終了チェック */
		if (PageHelpList[i] == 0) {
			break;
		}

		/* ウインドウハンドル取得 */
		hWnd = GetDlgItem(hDlg, PageHelpList[i]);
		if (!hWnd) {
			continue;
		}
		if (!IsWindowVisible(hWnd)) {
			continue;
		}

		/* 矩形を得て、PtInRectでチェックする */
		GetWindowRect(hWnd, &rect);
		if (!PtInRect(&rect, point)) {
			continue;
		}

		/* キャッシュチェック */
		if (PageHelpList[i] == uPropertyHelp) {
			return;
		}
		uPropertyHelp = PageHelpList[i];

		/* 一部リソースIDの入れ替え */
		uHelpID = uPropertyHelp;
#if defined(MOUSE)
		if (uPropertyHelp == IDC_OP_MOUSESWC) {
			uHelpID = IDC_OP_MOUSESW;
		}
#endif

		/* 文字列リソースをロード、設定 */
		string[0] = '\0';
		LoadString(hAppInstance, uHelpID, string, sizeof(string));
		hWnd = GetDlgItem(hDlg, uID);
		if (hWnd) {
			SetWindowText(hWnd, string);
		}

		/* 設定終了 */
		return;
	}

	/* ヘルプリスト範囲外の矩形。文字列なし */
	if (uPropertyHelp == 0) {
		return;
	}
	uPropertyHelp = 0;

	string[0] = '\0';
	hWnd = GetDlgItem(hDlg, uID);
	if (hWnd) {
		SetWindowText(hWnd, string);
	}
}

/*-[ 全般ページ ]-----------------------------------------------------------*/

/*
 *	全般ページ
 *	ダイアログ初期化
 */
static void FASTCALL GeneralPageInit(HWND hDlg)
{
	HWND hWnd;
	char string[128];

	ASSERT(hDlg);

	/* シート初期化 */
	SheetInit(hDlg);

	/* 動作機種 */
#if XM7_VER >= 2
	switch (propdat.fm7_ver) {
		case 1:
			CheckDlgButton(hDlg, IDC_GP_FM7, BST_CHECKED);
			break;
		case 2:
			CheckDlgButton(hDlg, IDC_GP_FM77AV, BST_CHECKED);
			break;
#if XM7_VER >= 3
		case 3:
			CheckDlgButton(hDlg, IDC_GP_AV40EX, BST_CHECKED);
			break;
#endif
	}
#else
	if (propdat.fm_subtype == FMSUB_FM8) {
		CheckDlgButton(hDlg, IDC_GP_FM8, BST_CHECKED);
	}
	else if (propdat.fm_subtype == FMSUB_FM7) {
		CheckDlgButton(hDlg, IDC_GP_FM7, BST_CHECKED);
	}
	else {
		CheckDlgButton(hDlg, IDC_GP_FM77, BST_CHECKED);
	}

	hWnd = GetDlgItem(hDlg, IDC_GP_FM8);
	EnableWindow(hWnd, available_fm8roms);
	hWnd = GetDlgItem(hDlg, IDC_GP_FM7);
	EnableWindow(hWnd, available_fm7roms);
	hWnd = GetDlgItem(hDlg, IDC_GP_FM77);
	EnableWindow(hWnd, available_fm7roms);

	/* メインCPU動作クロック */
	if (propdat.lowspeed_mode) {
		CheckDlgButton(hDlg, IDC_GP_MAIN1MHZ, BST_CHECKED);
	}
	else {
		CheckDlgButton(hDlg, IDC_GP_MAIN2MHZ, BST_CHECKED);
	}
#endif

	/* 動作モード */
	if (propdat.cycle_steal) {
		CheckDlgButton(hDlg, IDC_GP_HIGHSPEED, BST_CHECKED);
	}
	else if (!propdat.subclock_mode) {
		CheckDlgButton(hDlg, IDC_GP_MIDSPEED, BST_CHECKED);
	}
	else {
		CheckDlgButton(hDlg, IDC_GP_LOWSPEED, BST_CHECKED);
	}

	/* CPU選択 */
	hWnd = GetDlgItem(hDlg, IDC_GP_CPUCOMBO);
	ASSERT(hWnd);
	string[0] = '\0';
	(void)ComboBox_ResetContent(hWnd);
	LoadString(hAppInstance, IDS_GP_MAINCPU, string, sizeof(string));
	(void)ComboBox_AddString(hWnd, string);
#if XM7_VER >= 2
	LoadString(hAppInstance, IDS_GP_MAINMMR, string, sizeof(string));
	(void)ComboBox_AddString(hWnd, string);
#if XM7_VER >= 3
	LoadString(hAppInstance, IDS_GP_FASTMMR, string, sizeof(string));
	(void)ComboBox_AddString(hWnd, string);
#endif
	LoadString(hAppInstance, IDS_GP_SUBCPU, string, sizeof(string));
	(void)ComboBox_AddString(hWnd, string);
#else
	LoadString(hAppInstance, IDS_GP_MAINCPU_LOW, string, sizeof(string));
	(void)ComboBox_AddString(hWnd, string);
	LoadString(hAppInstance, IDS_GP_MAINMMR, string, sizeof(string));
	(void)ComboBox_AddString(hWnd, string);
	LoadString(hAppInstance, IDS_GP_SUBCPU, string, sizeof(string));
	(void)ComboBox_AddString(hWnd, string);
	LoadString(hAppInstance, IDS_GP_SUBCPU_LOW, string, sizeof(string));
	(void)ComboBox_AddString(hWnd, string);
#if defined(JSUB)
	LoadString(hAppInstance, IDS_GP_JSUBCPU, string, sizeof(string));
	(void)ComboBox_AddString(hWnd, string);
#endif
#endif
	(void)ComboBox_SetCurSel(hWnd, 0);

	/* CPU速度 */
	hWnd = GetDlgItem(hDlg, IDC_GP_CPUSPIN);
	ASSERT(hWnd);
	UpDown_SetRange(hWnd, 9999, 1);
	UpDown_SetPos(hWnd, propdat.main_speed);

	/* テープモータフラグ */
	if (propdat.bTapeFull) {
		CheckDlgButton(hDlg, IDC_GP_TAPESPEED, BST_CHECKED);
	}
	else {
		CheckDlgButton(hDlg, IDC_GP_TAPESPEED, BST_UNCHECKED);
	}

	/* テープモータモードフラグ */
	if (propdat.bTapeMode) {
		CheckDlgButton(hDlg, IDC_GP_TAPESPEEDMODE, BST_CHECKED);
	}
	else {
		CheckDlgButton(hDlg, IDC_GP_TAPESPEEDMODE, BST_UNCHECKED);
	}

#if defined(DISABLE_FULLSPEED)
	hWnd = GetDlgItem(hDlg, IDC_GP_FULLSPEED);
	EnableWindow(hWnd, FALSE);
	hWnd = GetDlgItem(hDlg, IDC_GP_AUTOSPEEDADJUST);
	EnableWindow(hWnd, FALSE);
#else
	/* 全力駆動フラグ */
	if (propdat.bCPUFull) {
		CheckDlgButton(hDlg, IDC_GP_FULLSPEED, BST_CHECKED);
	}
	else {
		CheckDlgButton(hDlg, IDC_GP_FULLSPEED, BST_UNCHECKED);
	}

	/* 自動速度調整フラグ */
	if (propdat.bSpeedAdjust) {
		CheckDlgButton(hDlg, IDC_GP_AUTOSPEEDADJUST, BST_CHECKED);
	}
	else {
		CheckDlgButton(hDlg, IDC_GP_AUTOSPEEDADJUST, BST_UNCHECKED);
	}
#endif

	/* FDDウェイトフラグ */
#if defined(FDDSND)
	if (propdat.bFddWait) {
		CheckDlgButton(hDlg, IDC_GP_FDDWAIT, BST_CHECKED);
	}
	else {
		CheckDlgButton(hDlg, IDC_GP_FDDWAIT, BST_UNCHECKED);
	}
#endif
}

/*
 *	全般ページ
 *	コマンド
 */
static void FASTCALL GeneralPageCmd(HWND hDlg, WORD wID, WORD wNotifyCode, HWND hWnd)
{
	int index;
	int cycles;
	HWND hSpin;
	HWND hCombo;

	ASSERT(hDlg);

	/* ID別 */
	switch (wID) {
		/* CPU速度 */
		case IDC_GP_CPUTEXT:
			if (wNotifyCode == EN_CHANGE) {
				cycles = GetDlgItemInt(hDlg, IDC_GP_CPUTEXT, 0, FALSE);
				if (cycles < 1) {
					cycles = 1;
				}
				else if (cycles > 9999) {
					cycles = 9999;
				}

				/* 値を格納 */
				hCombo = GetDlgItem(hDlg, IDC_GP_CPUCOMBO);
				index = ComboBox_GetCurSel(hCombo);
				switch (index) {
					case 0:
						propdat.main_speed = (DWORD)cycles;
						break;
#if XM7_VER >= 2
					case 1:
						propdat.mmr_speed = (DWORD)cycles;
						break;
					case 2:
#if XM7_VER >= 3
						propdat.fmmr_speed = (DWORD)cycles;
						break;
					case 3:
#endif
						propdat.sub_speed = (DWORD)cycles;
						break;
#else
					case 1:
						propdat.main_speed_low = (DWORD)cycles;
						break;
					case 2:
						propdat.mmr_speed = (DWORD)cycles;
						break;
					case 3:
						propdat.sub_speed = (DWORD)cycles;
						break;
					case 4:
						propdat.sub_speed_low = (DWORD)cycles;
						break;
#if defined(JSUB)
					case 5:
						propdat.jsub_speed = (DWORD)cycles;
						break;
#endif
#endif
					case CB_ERR:
						/* 項目未選択 */
						break;
					default:
						ASSERT(FALSE);
						break;
				}
			}
			break;

		/* CPU選択コンボボックス */
		case IDC_GP_CPUCOMBO:
			/* 選択対象が変わったら、新しい値をロード */
			if (wNotifyCode == CBN_SELCHANGE) {
				index = ComboBox_GetCurSel(hWnd);
				hSpin = GetDlgItem(hDlg, IDC_GP_CPUSPIN);
				ASSERT(hSpin);
				switch (index) {
					case 0:
						UpDown_SetPos(hSpin, propdat.main_speed);
						break;
#if XM7_VER >= 2
					case 1:
						UpDown_SetPos(hSpin, propdat.mmr_speed);
						break;
					case 2:
#if XM7_VER >= 3
						UpDown_SetPos(hSpin, propdat.fmmr_speed);
						break;
					case 3:
#endif
						UpDown_SetPos(hSpin, propdat.sub_speed);
						break;
#else
					case 1:
						UpDown_SetPos(hSpin, propdat.main_speed_low);
						break;
					case 2:
						UpDown_SetPos(hSpin, propdat.mmr_speed);
						break;
					case 3:
						UpDown_SetPos(hSpin, propdat.sub_speed);
						break;
					case 4:
						UpDown_SetPos(hSpin, propdat.sub_speed_low);
						break;
#if defined(JSUB)
					case 5:
						UpDown_SetPos(hSpin, propdat.jsub_speed);
						break;
#endif
#endif
					default:
						ASSERT(FALSE);
				}
			}
			break;

		case IDC_GP_CPUDEFAULT:
			/* デフォルト値を設定 */
			propdat.main_speed = MAINCYCLES;
			propdat.mmr_speed = MAINCYCLES_MMR;
#if XM7_VER >= 3
			propdat.fmmr_speed = MAINCYCLES_FMMR;
#endif
			propdat.sub_speed = SUBCYCLES;
#if XM7_VER == 1
			propdat.main_speed_low = MAINCYCLES_LOW;
			propdat.sub_speed_low = SUBCYCLES_LOW;
#if defined(JSUB)
			propdat.jsub_speed = JSUBCYCLES;
#endif
#endif

			/* 新しい値をロード */
			hCombo = GetDlgItem(hDlg, IDC_GP_CPUCOMBO);
			index = ComboBox_GetCurSel(hCombo);
			hSpin = GetDlgItem(hDlg, IDC_GP_CPUSPIN);
			ASSERT(hSpin);
			switch (index) {
				case 0:
					UpDown_SetPos(hSpin, propdat.main_speed);
					break;
#if XM7_VER >= 2
				case 1:
					UpDown_SetPos(hSpin, propdat.mmr_speed);
					break;
				case 2:
#if XM7_VER >= 3
					UpDown_SetPos(hSpin, propdat.fmmr_speed);
					break;
				case 3:
#endif
					UpDown_SetPos(hSpin, propdat.sub_speed);
					break;
#else
				case 1:
					UpDown_SetPos(hSpin, propdat.main_speed_low);
					break;
				case 2:
					UpDown_SetPos(hSpin, propdat.mmr_speed);
					break;
				case 3:
					UpDown_SetPos(hSpin, propdat.sub_speed);
					break;
				case 4:
					UpDown_SetPos(hSpin, propdat.sub_speed_low);
					break;
#if defined(JSUB)
				case 5:
					UpDown_SetPos(hSpin, propdat.jsub_speed);
					break;
#endif
#endif
				default:
					ASSERT(FALSE);
			}
			break;
	}
}

/*
 *	全般ページ
 *	垂直スクロール
 */
static void FASTCALL GeneralPageVScroll(HWND hDlg, WORD wPos, HWND hWnd)
{
	int index;
	HWND hCombo;

	/* チェック */
	if (hWnd != GetDlgItem(hDlg, IDC_GP_CPUSPIN)) {
		return;
	}

	/* 値を格納 */
	hCombo = GetDlgItem(hDlg, IDC_GP_CPUCOMBO);
	index = ComboBox_GetCurSel(hCombo);
	switch (index) {
		case 0:
			propdat.main_speed = (DWORD)wPos;
			break;
#if XM7_VER >= 2
		case 1:
			propdat.mmr_speed = (DWORD)wPos;
			break;
		case 2:
#if XM7_VER >= 3
			propdat.fmmr_speed = (DWORD)wPos;
			break;
		case 3:
#endif
			propdat.sub_speed = (DWORD)wPos;
			break;
#else
		case 1:
			propdat.main_speed_low = (DWORD)wPos;
			break;
		case 2:
			propdat.mmr_speed = (DWORD)wPos;
			break;
		case 3:
			propdat.sub_speed = (DWORD)wPos;
			break;
		case 4:
			propdat.sub_speed_low = (DWORD)wPos;
			break;
#if defined(JSUB)
		case 5:
			propdat.jsub_speed = (DWORD)wPos;
			break;
#endif
#endif
		default:
			ASSERT(FALSE);
			break;
	}
}

/*
 *	全般ページ
 *	適用
 */
static void FASTCALL GeneralPageApply(HWND hDlg)
{
	/* ステート変更 */
	uPropertyState = 2;

	/* FM-7バージョン */
#if XM7_VER >= 2
	if (IsDlgButtonChecked(hDlg, IDC_GP_FM7) == BST_CHECKED) {
		propdat.fm7_ver = 1;
	}
#if XM7_VER >= 3
	else if (IsDlgButtonChecked(hDlg, IDC_GP_FM77AV) == BST_CHECKED) {
		propdat.fm7_ver = 2;
	}
	else
	{
		propdat.fm7_ver = 3;
	}
#else
	else {
		propdat.fm7_ver = 2;
	}
#endif
#else
	if (IsDlgButtonChecked(hDlg, IDC_GP_FM8) == BST_CHECKED) {
		propdat.fm_subtype = FMSUB_FM8;
	}
	else if (IsDlgButtonChecked(hDlg, IDC_GP_FM7) == BST_CHECKED) {
		propdat.fm_subtype = FMSUB_FM7;
	}
	else {
		propdat.fm_subtype = FMSUB_FM77;
	}

	/* メインCPU動作クロック */
	if (IsDlgButtonChecked(hDlg, IDC_GP_MAIN1MHZ) == BST_CHECKED) {
		propdat.lowspeed_mode = TRUE;
	}
	else {
		propdat.lowspeed_mode = FALSE;
	}
#endif

	/* サイクルスチール */
	if (IsDlgButtonChecked(hDlg, IDC_GP_HIGHSPEED) == BST_CHECKED) {
		propdat.cycle_steal = TRUE;
		propdat.subclock_mode = FALSE;
	}
	else if (IsDlgButtonChecked(hDlg, IDC_GP_MIDSPEED) == BST_CHECKED) {
		propdat.cycle_steal = FALSE;
		propdat.subclock_mode = FALSE;
	}
	else {
		propdat.cycle_steal = FALSE;
		propdat.subclock_mode = TRUE;
	}

	/* テープ高速モード */
	if (IsDlgButtonChecked(hDlg, IDC_GP_TAPESPEED) == BST_CHECKED) {
		propdat.bTapeFull = TRUE;
	}
	else {
		propdat.bTapeFull = FALSE;
	}

	/* テープ高速モード動作タイプ */
	if (IsDlgButtonChecked(hDlg, IDC_GP_TAPESPEEDMODE) == BST_CHECKED) {
		propdat.bTapeMode = TRUE;
	}
	else {
		propdat.bTapeMode = FALSE;
	}

#if !defined(DISABLE_FULLSPEED)
	/* 全力駆動 */
	if (IsDlgButtonChecked(hDlg, IDC_GP_FULLSPEED) == BST_CHECKED) {
		propdat.bCPUFull = TRUE;
	}
	else {
		propdat.bCPUFull = FALSE;
	}

	/* 自動速度調整 */
	if (IsDlgButtonChecked(hDlg, IDC_GP_AUTOSPEEDADJUST) == BST_CHECKED) {
		propdat.bSpeedAdjust = TRUE;
	}
	else {
		propdat.bSpeedAdjust = FALSE;
	}
#endif

	/* FDDウェイト */
#if defined(FDDSND)
	if (IsDlgButtonChecked(hDlg, IDC_GP_FDDWAIT) == BST_CHECKED) {
		propdat.bFddWait = TRUE;
	}
	else {
		propdat.bFddWait = FALSE;
	}
#endif
}

/*
 *	全般ページ
 *	ダイアログプロシージャ
 */
static BOOL CALLBACK GeneralPageProc(HWND hDlg, UINT msg,
									 WPARAM wParam, LPARAM lParam)
{
	LPNMHDR pnmh;

	switch (msg) {
		/* 初期化 */
		case WM_INITDIALOG:
			GeneralPageInit(hDlg);
			return TRUE;

		/* コマンド */
		case WM_COMMAND:
			GeneralPageCmd(hDlg, LOWORD(wParam), HIWORD(wParam), (HWND)lParam);
			return TRUE;

		/* 通知 */
		case WM_NOTIFY:
			pnmh = (LPNMHDR)lParam;
			if (pnmh->code == PSN_APPLY) {
				GeneralPageApply(hDlg);
				return TRUE;
			}
			break;

		/* 垂直スクロール */
		case WM_VSCROLL:
			GeneralPageVScroll(hDlg, HIWORD(wParam), (HWND)lParam);
			break;

		/* カーソル設定 */
		case WM_SETCURSOR:
			if (HIWORD(lParam) == WM_MOUSEMOVE) {
				PageHelp(hDlg, IDC_GP_HELP);
			}
			break;
	}

	return FALSE;
}

/*-[ サウンドページ ]-------------------------------------------------------*/

/*
 *	サウンドページ
 *	コマンド
 */
static void FASTCALL SoundPageCmd(HWND hDlg, WORD wID, WORD wNotifyCode, HWND hWnd)
{
	int tmp;
	HWND hSpin;

	ASSERT(hDlg);
	UNUSED(hWnd);

	/* ID別 */
	switch (wID) {
		/* サウンドバッファ */
		case IDC_SP_BUFEDIT:
			if (wNotifyCode == EN_CHANGE) {
				tmp = GetDlgItemInt(hDlg, IDC_SP_BUFEDIT, 0, FALSE) / 10;
				if (tmp < 4) {
					tmp = 4;
				}
				else if (tmp > 100) {
					tmp = 100;
				}

				hSpin = GetDlgItem(hDlg, IDC_SP_BUFSPIN);
				ASSERT(hSpin);
				UpDown_SetPos(hSpin, tmp);
			}
			break;

		/* BEEP周波数 */
		case IDC_SP_BEEPEDIT:
			if (wNotifyCode == EN_CHANGE) {
				tmp = GetDlgItemInt(hDlg, IDC_SP_BEEPEDIT, 0, FALSE);
				if (tmp < 100) {
					tmp = 100;
				}
				else if (tmp > 9999) {
					tmp = 9999;
				}

				hSpin = GetDlgItem(hDlg, IDC_SP_BEEPSPIN);
				ASSERT(hSpin);
				UpDown_SetPos(hSpin, tmp);
			}
			break;
		}
}

/*
 *	サウンドページ
 *	垂直スクロール
 */
static void FASTCALL SoundPageVScroll(HWND hDlg, WORD wPos, HWND hWnd)
{
	HWND hBuddyWnd;
	char string[128];

	ASSERT(hDlg);
	ASSERT(hWnd);

	/* ウインドウハンドルをチェック */
	if (hWnd == GetDlgItem(hDlg, IDC_SP_BUFSPIN)) {
		/* サウンドバッファ */
		/* ポジションから、バディウインドウに値を設定 */
		hBuddyWnd = GetDlgItem(hDlg, IDC_SP_BUFEDIT);
		ASSERT(hBuddyWnd);
		_snprintf(string, sizeof(string), "%d", wPos * 10);
		SetWindowText(hBuddyWnd, string);
	}
	else if (hWnd == GetDlgItem(hDlg, IDC_SP_BEEPSPIN)) {
		/* BEEP周波数 */
		/* ポジションから、バディウインドウに値を設定 */
		hBuddyWnd = GetDlgItem(hDlg, IDC_SP_BEEPEDIT);
		ASSERT(hBuddyWnd);
		_snprintf(string, sizeof(string), "%d", wPos);
		SetWindowText(hBuddyWnd, string);
	}
}

/*
 *	サウンドページ
 *	ダイアログ初期化
 */
static void FASTCALL SoundPageInit(HWND hDlg)
{
	HWND hWnd;
	char string[128];

	/* シート初期化 */
	SheetInit(hDlg);

	/* サンプリングレート */
	switch (propdat.nSampleRate) {
		case 96000:
			CheckDlgButton(hDlg, IDC_SP_96K, BST_CHECKED);
			break;
		case 88200:
			CheckDlgButton(hDlg, IDC_SP_88K, BST_CHECKED);
			break;
		case 51200:
			CheckDlgButton(hDlg, IDC_SP_51K, BST_CHECKED);
			break;
		case 48000:
			CheckDlgButton(hDlg, IDC_SP_48K, BST_CHECKED);
			break;
		case 44100:
			CheckDlgButton(hDlg, IDC_SP_44K, BST_CHECKED);
			break;
		case 25600:
			CheckDlgButton(hDlg, IDC_SP_25K, BST_CHECKED);
			break;
		case 22050:
			CheckDlgButton(hDlg, IDC_SP_22K, BST_CHECKED);
			break;
		case 0:
			CheckDlgButton(hDlg, IDC_SP_NONE, BST_CHECKED);
			break;
		default:
			ASSERT(FALSE);
			break;
	}

	/* サウンドバッファ */
	hWnd = GetDlgItem(hDlg, IDC_SP_BUFSPIN);
	ASSERT(hWnd);
	UpDown_SetRange(hWnd, 100, 4);
	UpDown_SetPos(hWnd, propdat.nSoundBuffer / 10);
	SoundPageVScroll(hDlg, LOWORD(UpDown_GetPos(hWnd)), hWnd);

	/* BEEP周波数 */
	hWnd = GetDlgItem(hDlg, IDC_SP_BEEPSPIN);
	ASSERT(hWnd);
	UpDown_SetRange(hWnd, 9999, 100);
	UpDown_SetPos(hWnd, propdat.nBeepFreq);
	SoundPageVScroll(hDlg, LOWORD(UpDown_GetPos(hWnd)), hWnd);

	/* FM高品質合成モード */
	if (propdat.bInterpolation) {
		CheckDlgButton(hDlg, IDC_SP_HQMODE, BST_CHECKED);
	}
	else {
		CheckDlgButton(hDlg, IDC_SP_HQMODE, BST_UNCHECKED);
	}

	/* 出力モード */
	hWnd = GetDlgItem(hDlg, IDC_SP_STEREO);
	ASSERT(hWnd);
	string[0] = '\0';
	(void)ComboBox_ResetContent(hWnd);
	LoadString(hAppInstance, IDS_SP_MONO, string, sizeof(string));
	(void)ComboBox_AddString(hWnd, string);
	LoadString(hAppInstance, IDS_SP_STEREO, string, sizeof(string));
	(void)ComboBox_AddString(hWnd, string);
	LoadString(hAppInstance, IDS_SP_STEREO_WHGREV, string, sizeof(string));
	(void)ComboBox_AddString(hWnd, string);
	LoadString(hAppInstance, IDS_SP_STEREO_THG, string, sizeof(string));
	(void)ComboBox_AddString(hWnd, string);
	LoadString(hAppInstance, IDS_SP_STEREO_THGREV, string, sizeof(string));
	(void)ComboBox_AddString(hWnd, string);
	(void)ComboBox_SetCurSel(hWnd, propdat.uStereoOut);

	/* テープ音モニタ */
	if (propdat.bTapeMon) {
		CheckDlgButton(hDlg, IDC_SP_TAPEMON, BST_CHECKED);
	}
	else {
		CheckDlgButton(hDlg, IDC_SP_TAPEMON, BST_UNCHECKED);
	}

	/* ろみお */
#if defined(ROMEO)
	if (propdat.bUseRomeo) {
		CheckDlgButton(hDlg, IDC_SP_ROMEO, BST_CHECKED);
	}
	else {
		CheckDlgButton(hDlg, IDC_SP_ROMEO, BST_UNCHECKED);
	}
	if (!bRomeo) {
		hWnd = GetDlgItem(hDlg, IDC_SP_ROMEO);
		EnableWindow(hWnd, FALSE);
	}
#endif

	/* FDDシーク音 */
#if defined(FDDSND)
	if (propdat.bFddSound) {
		CheckDlgButton(hDlg, IDC_SP_FDDSOUND, BST_CHECKED);
	}
	else {
		CheckDlgButton(hDlg, IDC_SP_FDDSOUND, BST_UNCHECKED);
	}
#endif
}

/*
 *	サウンドページ
 *	適用
 */
static void FASTCALL SoundPageApply(HWND hDlg)
{
	HWND hWnd;
	UINT uPos;

	/* ステート変更 */
	uPropertyState = 2;

	/* サンプリングレート */
	if (IsDlgButtonChecked(hDlg, IDC_SP_96K) == BST_CHECKED) {
		propdat.nSampleRate = 96000;
	}
	if (IsDlgButtonChecked(hDlg, IDC_SP_88K) == BST_CHECKED) {
		propdat.nSampleRate = 88200;
	}
	if (IsDlgButtonChecked(hDlg, IDC_SP_51K) == BST_CHECKED) {
		propdat.nSampleRate = 51200;
	}
	if (IsDlgButtonChecked(hDlg, IDC_SP_48K) == BST_CHECKED) {
		propdat.nSampleRate = 48000;
	}
	if (IsDlgButtonChecked(hDlg, IDC_SP_44K) == BST_CHECKED) {
		propdat.nSampleRate = 44100;
	}
	if (IsDlgButtonChecked(hDlg, IDC_SP_25K) == BST_CHECKED) {
		propdat.nSampleRate = 25600;
	}
	if (IsDlgButtonChecked(hDlg, IDC_SP_22K) == BST_CHECKED) {
		propdat.nSampleRate = 22050;
	}
	if (IsDlgButtonChecked(hDlg, IDC_SP_NONE) == BST_CHECKED) {
		propdat.nSampleRate = 0;
	}

	/* サウンドバッファ */
	hWnd = GetDlgItem(hDlg, IDC_SP_BUFSPIN);
	ASSERT(hWnd);
	uPos = LOWORD(UpDown_GetPos(hWnd));
	propdat.nSoundBuffer = uPos * 10;

	/* BEEP周波数 */
	hWnd = GetDlgItem(hDlg, IDC_SP_BEEPSPIN);
	ASSERT(hWnd);
	propdat.nBeepFreq = LOWORD(UpDown_GetPos(hWnd));

	/* FM高品質合成モード */
	if (IsDlgButtonChecked(hDlg, IDC_SP_HQMODE) == BST_CHECKED) {
		propdat.bInterpolation = TRUE;
	}
	else {
		propdat.bInterpolation = FALSE;
	}

	/* 出力モード */
	hWnd = GetDlgItem(hDlg, IDC_SP_STEREO);
	propdat.uStereoOut = ComboBox_GetCurSel(hWnd);

	/* テープ音モニタ */
	if (IsDlgButtonChecked(hDlg, IDC_SP_TAPEMON) == BST_CHECKED) {
		propdat.bTapeMon = TRUE;
	}
	else {
		propdat.bTapeMon = FALSE;
	}

	/* ろみお */
#if defined(ROMEO)
	if ((IsDlgButtonChecked(hDlg, IDC_SP_ROMEO) == BST_CHECKED) && bRomeo) {
		propdat.bUseRomeo = TRUE;
	}
	else {
		propdat.bUseRomeo = FALSE;
	}
#endif

	/* FDDシーク音 */
#if defined(FDDSND)
	if (IsDlgButtonChecked(hDlg, IDC_SP_FDDSOUND) == BST_CHECKED) {
		propdat.bFddSound = TRUE;
	}
	else {
		propdat.bFddSound = FALSE;
	}
#endif
}

/*
 *	サウンドページ
 *	ダイアログプロシージャ
 */
static BOOL CALLBACK SoundPageProc(HWND hDlg, UINT msg,
									 WPARAM wParam, LPARAM lParam)
{
	LPNMHDR pnmh;

	switch (msg) {
		/* 初期化 */
		case WM_INITDIALOG:
			SoundPageInit(hDlg);
			return TRUE;

		/* コマンド */
		case WM_COMMAND:
			SoundPageCmd(hDlg, LOWORD(wParam), HIWORD(wParam), (HWND)lParam);
			return TRUE;

		/* 通知 */
		case WM_NOTIFY:
			pnmh = (LPNMHDR)lParam;
			if (pnmh->code == PSN_APPLY) {
				SoundPageApply(hDlg);
				return TRUE;
			}
			break;

		/* 垂直スクロール */
		case WM_VSCROLL:
			SoundPageVScroll(hDlg, HIWORD(wParam), (HWND)lParam);
			break;

		/* カーソル設定 */
		case WM_SETCURSOR:
			if (HIWORD(lParam) == WM_MOUSEMOVE) {
				PageHelp(hDlg, IDC_SP_HELP);
			}
			break;
	}

	return FALSE;
}

/*-[ 音量調整ページ ]-------------------------------------------------------*/

/*
 *	音量調整ページ
 *	スライダーアップデート
 */
static void FASTCALL UpdateVolumeSlider(HWND hDlg)
{
	UINT uSp;
	int nFM;
	int nPSG;
	int nBeep;
	int nCMT;
	int nWav;
	char string[128];

	/* 各エディットボックスにスライダーの値を設定 */
	uSp = SendDlgItemMessage(hDlg, IDC_VP_SEPARATION, TBM_GETPOS, 0, 0);
	_snprintf(string, sizeof(string), "%u", uSp);
	Edit_SetText(GetDlgItem(hDlg, IDC_VP_SEPARATIONTEXT), string);
	nFM = SendDlgItemMessage(hDlg, IDC_VP_FMVOLUME, TBM_GETPOS, 0, 0);
	_snprintf(string, sizeof(string), "%d", nFM);
	Edit_SetText(GetDlgItem(hDlg, IDC_VP_FMVOLUMETEXT), string);
	nPSG = SendDlgItemMessage(hDlg, IDC_VP_PSGVOLUME, TBM_GETPOS, 0, 0);
	_snprintf(string, sizeof(string), "%d", nPSG);
	Edit_SetText(GetDlgItem(hDlg, IDC_VP_PSGVOLUMETEXT), string);
	nBeep = SendDlgItemMessage(hDlg, IDC_VP_BEEPVOLUME, TBM_GETPOS, 0, 0);
	_snprintf(string, sizeof(string), "%d", nBeep);
	Edit_SetText(GetDlgItem(hDlg, IDC_VP_BEEPVOLUMETEXT), string);
	nCMT = SendDlgItemMessage(hDlg, IDC_VP_CMTVOLUME, TBM_GETPOS, 0, 0);
	_snprintf(string, sizeof(string), "%d", nCMT);
	Edit_SetText(GetDlgItem(hDlg, IDC_VP_CMTVOLUMETEXT), string);
#if defined(FDDSND)
	nWav = SendDlgItemMessage(hDlg, IDC_VP_WAVEVOLUME, TBM_GETPOS, 0, 0);
	_snprintf(string, sizeof(string), "%d", nWav);
	Edit_SetText(GetDlgItem(hDlg, IDC_VP_WAVEVOLUMETEXT), string);
#endif

	/* ボリューム設定 */
	SetSoundVolume2(uSp, nFM, nPSG, nBeep, nCMT, nWav);
}

/*
 *	音量調整ページ
 *	ダイアログ初期化
 */
static void FASTCALL VolumePageInit(HWND hDlg)
{
	ASSERT(hDlg);

	/* シート初期化 */
	SheetInit(hDlg);

	/* チャンネルセパレーション */
	SendDlgItemMessage(hDlg, IDC_VP_SEPARATION, TBM_SETLINESIZE, 0, 1);
	SendDlgItemMessage(hDlg, IDC_VP_SEPARATION, TBM_SETPAGESIZE, 0, 4);
	SendDlgItemMessage(hDlg, IDC_VP_SEPARATION, TBM_SETRANGE, TRUE,
		MAKELONG(0, 16));
	SendDlgItemMessage(hDlg, IDC_VP_SEPARATION, TBM_SETPOS, TRUE,
		propdat.uChSeparation);

	/* FM音源 */
	SendDlgItemMessage(hDlg, IDC_VP_FMVOLUME, TBM_SETLINESIZE, 0, 1);
	SendDlgItemMessage(hDlg, IDC_VP_FMVOLUME, TBM_SETPAGESIZE, 0, 6);
	SendDlgItemMessage(hDlg, IDC_VP_FMVOLUME, TBM_SETRANGE, TRUE,
		MAKELONG(-96, 20));
	SendDlgItemMessage(hDlg, IDC_VP_FMVOLUME, TBM_SETPOS, TRUE,
		propdat.nFMVolume);

	/* PSG */
	SendDlgItemMessage(hDlg, IDC_VP_PSGVOLUME, TBM_SETLINESIZE, 0, 1);
	SendDlgItemMessage(hDlg, IDC_VP_PSGVOLUME, TBM_SETPAGESIZE, 0, 6);
	SendDlgItemMessage(hDlg, IDC_VP_PSGVOLUME, TBM_SETRANGE, TRUE,
		MAKELONG(-96, 20));
	SendDlgItemMessage(hDlg, IDC_VP_PSGVOLUME, TBM_SETPOS, TRUE,
		propdat.nPSGVolume);

	/* BEEP */
	SendDlgItemMessage(hDlg, IDC_VP_BEEPVOLUME, TBM_SETLINESIZE, 0, 1);
	SendDlgItemMessage(hDlg, IDC_VP_BEEPVOLUME, TBM_SETPAGESIZE, 0, 6);
	SendDlgItemMessage(hDlg, IDC_VP_BEEPVOLUME, TBM_SETRANGE, TRUE,
		MAKELONG(-96, 0));
	SendDlgItemMessage(hDlg, IDC_VP_BEEPVOLUME, TBM_SETPOS, TRUE,
		propdat.nBeepVolume);

	/* CMT音モニタ */
	SendDlgItemMessage(hDlg, IDC_VP_CMTVOLUME, TBM_SETLINESIZE, 0, 1);
	SendDlgItemMessage(hDlg, IDC_VP_CMTVOLUME, TBM_SETPAGESIZE, 0, 6);
	SendDlgItemMessage(hDlg, IDC_VP_CMTVOLUME, TBM_SETRANGE, TRUE,
		MAKELONG(-96, 0));
	SendDlgItemMessage(hDlg, IDC_VP_CMTVOLUME, TBM_SETPOS, TRUE,
		propdat.nCMTVolume);

	/* 各種効果音 */
#if defined(FDDSND)
	SendDlgItemMessage(hDlg, IDC_VP_WAVEVOLUME, TBM_SETLINESIZE, 0, 1);
	SendDlgItemMessage(hDlg, IDC_VP_WAVEVOLUME, TBM_SETPAGESIZE, 0, 6);
	SendDlgItemMessage(hDlg, IDC_VP_WAVEVOLUME, TBM_SETRANGE, TRUE,
		MAKELONG(-96, 0));
	SendDlgItemMessage(hDlg, IDC_VP_WAVEVOLUME, TBM_SETPOS, TRUE,
		propdat.nWaveVolume);
#endif

	/* スライダーアップデート */
	UpdateVolumeSlider(hDlg);
}

/*
 *	音量調整ページ
 *	適用
 */
static void FASTCALL VolumePageApply(HWND hDlg)
{
	/* ステート変更 */
	uPropertyState = 2;

	/* チャンネルセパレーション */
	propdat.uChSeparation = SendDlgItemMessage(hDlg, IDC_VP_SEPARATION,
		TBM_GETPOS, 0, 0);

	/* FM音源 */
	propdat.nFMVolume = SendDlgItemMessage(hDlg, IDC_VP_FMVOLUME,
		TBM_GETPOS, 0, 0);

	/* PSG */
	propdat.nPSGVolume = SendDlgItemMessage(hDlg, IDC_VP_PSGVOLUME,
		TBM_GETPOS, 0, 0);

	/* BEEP */
	propdat.nBeepVolume = SendDlgItemMessage(hDlg, IDC_VP_BEEPVOLUME,
		TBM_GETPOS, 0, 0);

	/* CMT音モニタ */
	propdat.nCMTVolume = SendDlgItemMessage(hDlg, IDC_VP_CMTVOLUME,
		TBM_GETPOS, 0, 0);

	/* 各種効果音 */
#if defined(FDDSND)
	propdat.nWaveVolume = SendDlgItemMessage(hDlg, IDC_VP_WAVEVOLUME,
		TBM_GETPOS, 0, 0);
#endif
}

/*
 *	音量調整ページ
 *	ダイアログプロシージャ
 */
static BOOL CALLBACK VolumePageProc(HWND hDlg, UINT msg,
									WPARAM wParam, LPARAM lParam)
{
	LPNMHDR pnmh;

	switch (msg) {
		/* ダイアログ初期化 */
		case WM_INITDIALOG:
			VolumePageInit(hDlg);
			return TRUE;

		/* コマンド */
		case WM_COMMAND:
			if (LOWORD(wParam) == IDC_VP_DEFAULT) {
				SendDlgItemMessage(hDlg, IDC_VP_SEPARATION, TBM_SETPOS,
					TRUE, CHSEPARATION_DEFAULT);
				SendDlgItemMessage(hDlg, IDC_VP_FMVOLUME, TBM_SETPOS,
					TRUE, FMVOLUME_DEFAULT);
				SendDlgItemMessage(hDlg, IDC_VP_PSGVOLUME, TBM_SETPOS,
					TRUE, PSGVOLUME_DEFAULT);
				SendDlgItemMessage(hDlg, IDC_VP_BEEPVOLUME, TBM_SETPOS,
					TRUE, BEEPVOLUME_DEFAULT);
				SendDlgItemMessage(hDlg, IDC_VP_CMTVOLUME, TBM_SETPOS,
					TRUE, CMTVOLUME_DEFAULT);
#if defined(FDDSND)
				SendDlgItemMessage(hDlg, IDC_VP_WAVEVOLUME, TBM_SETPOS,
					TRUE, WAVEVOLUME_DEFAULT);
#endif
				UpdateVolumeSlider(hDlg);
				return TRUE;
			}
			break;

		/* 通知 */
		case WM_NOTIFY:
			pnmh = (LPNMHDR)lParam;
			if (pnmh->code == PSN_APPLY) {
				VolumePageApply(hDlg);
				return TRUE;
			}
			if (pnmh->code == PSN_QUERYCANCEL) {
				SetSoundVolume();
				return TRUE;
			}
			break;

		/* 縦スクロール */
		case WM_HSCROLL:
			UpdateVolumeSlider(hDlg);
			return FALSE;

		/* カーソル設定 */
		case WM_SETCURSOR:
			if (HIWORD(lParam) == WM_MOUSEMOVE) {
				PageHelp(hDlg, IDC_VP_HELP);
			}
			break;
		}

	/* それ以外はFALSE */
	return FALSE;
}

/*-[ キー入力ダイアログ ]---------------------------------------------------*/

/*
 *	キーボードページ
 *	DirectInput キーコードテーブル
 */
static char *KbdPageDirectInput[] = {
	NULL,					/* 0x00 */
	"DIK_ESCAPE",			/* 0x01 */
	"DIK_1",				/* 0x02 */
	"DIK_2",				/* 0x03 */
	"DIK_3",				/* 0x04 */
	"DIK_4",				/* 0x05 */
	"DIK_5",				/* 0x06 */
	"DIK_6",				/* 0x07 */
	"DIK_7",				/* 0x08 */
	"DIK_8",				/* 0x09 */
	"DIK_9",				/* 0x0A */
	"DIK_0",				/* 0x0B */
	"DIK_MINUS",			/* 0x0C */
	"DIK_EQUALS",			/* 0x0D */
	"DIK_BACK",				/* 0x0E */
	"DIK_TAB",				/* 0x0F */
	"DIK_Q",				/* 0x10 */
	"DIK_W",				/* 0x11 */
	"DIK_E",				/* 0x12 */
	"DIK_R",				/* 0x13 */
	"DIK_T",				/* 0x14 */
	"DIK_Y",				/* 0x15 */
	"DIK_U",				/* 0x16 */
	"DIK_I",				/* 0x17 */
	"DIK_O",				/* 0x18 */
	"DIK_P",				/* 0x19 */
	"DIK_LBRACKET",			/* 0x1A */
	"DIK_RBRACKET",			/* 0x1B */
	"DIK_RETURN",			/* 0x1C */
	"DIK_LCONTROL",			/* 0x1D */
	"DIK_A",				/* 0x1E */
	"DIK_S",				/* 0x1F */
	"DIK_D",				/* 0x20 */
	"DIK_F",				/* 0x21 */
	"DIK_G",				/* 0x22 */
	"DIK_H",				/* 0x23 */
	"DIK_J",				/* 0x24 */
	"DIK_K",				/* 0x25 */
	"DIK_L",				/* 0x26 */
	"DIK_SEMICOLON",		/* 0x27 */
	"DIK_APOSTROPHE",		/* 0x28 */
	"DIK_GRAVE",			/* 0x29 */
	"DIK_LSHIFT",			/* 0x2A */
	"DIK_BACKSLASH",		/* 0x2B */
	"DIK_Z",				/* 0x2C */
	"DIK_X",				/* 0x2D */
	"DIK_C",				/* 0x2E */
	"DIK_V",				/* 0x2F */
	"DIK_B",				/* 0x30 */
	"DIK_N",				/* 0x31 */
	"DIK_M",				/* 0x32 */
	"DIK_COMMA",			/* 0x33 */
	"DIK_PERIOD",			/* 0x34 */
	"DIK_SLASH",			/* 0x35 */
	"DIK_RSHIFT",			/* 0x36 */
	"DIK_MULTIPLY",			/* 0x37 */
	"DIK_LMENU",			/* 0x38 */
	"DIK_SPACE",			/* 0x39 */
	"DIK_CAPITAL",			/* 0x3A */
	"DIK_F1",				/* 0x3B */
	"DIK_F2",				/* 0x3C */
	"DIK_F3",				/* 0x3D */
	"DIK_F4",				/* 0x3E */
	"DIK_F5",				/* 0x3F */
	"DIK_F6",				/* 0x40 */
	"DIK_F7",				/* 0x41 */
	"DIK_F8",				/* 0x42 */
	"DIK_F9",				/* 0x43 */
	"DIK_F10",				/* 0x44 */
	"DIK_NUMLOCK",			/* 0x45 */
	"DIK_SCROLL",			/* 0x46 */
	"DIK_NUMPAD7",			/* 0x47 */
	"DIK_NUMPAD8",			/* 0x48 */
	"DIK_NUMPAD9",			/* 0x49 */
	"DIK_SUBTRACT",			/* 0x4A */
	"DIK_NUMPAD4",			/* 0x4B */
	"DIK_NUMPAD5",			/* 0x4C */
	"DIK_NUMPAD6",			/* 0x4D */
	"DIK_ADD",				/* 0x4E */
	"DIK_NUMPAD1",			/* 0x4F */
	"DIK_NUMPAD2",			/* 0x50 */
	"DIK_NUMPAD3",			/* 0x51 */
	"DIK_NUMPAD0",			/* 0x52 */
	"DIK_DECIMAL",			/* 0x53 */
	NULL,					/* 0x54 */
	NULL,					/* 0x55 */
	"DIK_OEM_102",			/* 0x56 */
	"DIK_F11",				/* 0x57 */
	"DIK_F12",				/* 0x58 */
	NULL,					/* 0x59 */
	NULL,					/* 0x5A */
	NULL,					/* 0x5B */
	NULL,					/* 0x5C */
	NULL,					/* 0x5D */
	NULL,					/* 0x5E */
	NULL,					/* 0x5F */
	NULL,					/* 0x60 */
	NULL,					/* 0x61 */
	NULL,					/* 0x62 */
	NULL,					/* 0x63 */
	"DIK_F13",				/* 0x64 */
	"DIK_F14",				/* 0x65 */
	"DIK_F15",				/* 0x66 */
	NULL,					/* 0x67 */
	NULL,					/* 0x68 */
	NULL,					/* 0x69 */
	NULL,					/* 0x6A */
	NULL,					/* 0x6B */
	NULL,					/* 0x6C */
	NULL,					/* 0x6D */
	NULL,					/* 0x6E */
	NULL,					/* 0x6F */
	"DIK_KANA",				/* 0x70 */
	NULL,					/* 0x71 */
	NULL,					/* 0x72 */
	"DIK_ABNT_C1",			/* 0x73 */
	NULL,					/* 0x74 */
	NULL,					/* 0x75 */
	NULL,					/* 0x76 */
	NULL,					/* 0x77 */
	NULL,					/* 0x78 */
	"DIK_CONVERT",			/* 0x79 */
	NULL,					/* 0x7A */
	"DIK_NOCONVERT",		/* 0x7B */
	NULL,					/* 0x7C */
	"DIK_YEN",				/* 0x7D */
	"DIK_ABNT_C2",			/* 0x7E */
	NULL,					/* 0x7F */
	NULL,					/* 0x80 */
	NULL,					/* 0x81 */
	NULL,					/* 0x82 */
	NULL,					/* 0x83 */
	NULL,					/* 0x84 */
	NULL,					/* 0x85 */
	NULL,					/* 0x86 */
	NULL,					/* 0x87 */
	NULL,					/* 0x88 */
	NULL,					/* 0x89 */
	NULL,					/* 0x8A */
	NULL,					/* 0x8B */
	NULL,					/* 0x8C */
	"DIK_NUMPADEQUALS",		/* 0x8D */
	NULL,					/* 0x8E */
	NULL,					/* 0x8F */
	"DIK_PREVTRACK",		/* 0x90 */
	"DIK_AT",				/* 0x91 */
	"DIK_COLON",			/* 0x92 */
	"DIK_UNDERLINE",		/* 0x93 */
	"DIK_KANJI",			/* 0x94 */
	"DIK_STOP",				/* 0x95 */
	"DIK_AX",				/* 0x96 */
	"DIK_UNLABELED",		/* 0x97 */
	NULL,					/* 0x98 */
	"DIK_NEXTTRACK",		/* 0x99 */
	NULL,					/* 0x9A */
	NULL,					/* 0x9B */
	"DIK_NUMPADENTER",		/* 0x9C */
	"DIK_RCONTROL",			/* 0x9D */
	NULL,					/* 0x9E */
	NULL,					/* 0x9F */
	"DIK_MUTE",				/* 0xA0 */
	"DIK_CALCULATOR",		/* 0xA1 */
	"DIK_PLAYPAUSE",		/* 0xA2 */
	NULL,					/* 0xA3 */
	"DIK_MEDIASTOP",		/* 0xA4 */
	NULL,					/* 0xA5 */
	NULL,					/* 0xA6 */
	NULL,					/* 0xA7 */
	NULL,					/* 0xA8 */
	NULL,					/* 0xA9 */
	NULL,					/* 0xAA */
	NULL,					/* 0xAB */
	NULL,					/* 0xAC */
	NULL,					/* 0xAD */
	"DIK_VOLUMEDOWN",		/* 0xAE */
	NULL,					/* 0xAF */
	"DIK_VOLUMEUP",			/* 0xB0 */
	NULL,					/* 0xB1 */
	"DIK_WEBHOME",			/* 0xB2 */
	"DIK_NUMPADCOMMA",		/* 0xB3 */
	NULL,					/* 0xB4 */
	"DIK_DIVIDE",			/* 0xB5 */
	NULL,					/* 0xB6 */
	"DIK_SYSRQ",			/* 0xB7 */
	"DIK_RMENU",			/* 0xB8 */
	NULL,					/* 0xB9 */
	NULL,					/* 0xBA */
	NULL,					/* 0xBB */
	NULL,					/* 0xBC */
	NULL,					/* 0xBD */
	NULL,					/* 0xBE */
	NULL,					/* 0xBF */
	NULL,					/* 0xC0 */
	NULL,					/* 0xC1 */
	NULL,					/* 0xC2 */
	NULL,					/* 0xC3 */
	NULL,					/* 0xC4 */
	"DIK_PAUSE",			/* 0xC5 */
	NULL,					/* 0xC6 */
	"DIK_HOME",				/* 0xC7 */
	"DIK_UP",				/* 0xC8 */
	"DIK_PRIOR",			/* 0xC9 */
	NULL,					/* 0xCA */
	"DIK_LEFT",				/* 0xCB */
	NULL,					/* 0xCC */
	"DIK_RIGHT",			/* 0xCD */
	NULL,					/* 0xCE */
	"DIK_END",				/* 0xCF */
	"DIK_DOWN",				/* 0xD0 */
	"DIK_NEXT",				/* 0xD1 */
	"DIK_INSERT",			/* 0xD2 */
	"DIK_DELETE",			/* 0xD3 */
	NULL,					/* 0xD4 */
	NULL,					/* 0xD5 */
	NULL,					/* 0xD6 */
	NULL,					/* 0xD7 */
	NULL,					/* 0xD8 */
	NULL,					/* 0xD9 */
	NULL,					/* 0xDA */
	"DIK_LWIN",				/* 0xDB */
	"DIK_RWIN",				/* 0xDC */
	"DIK_APPS",				/* 0xDD */
	"DIK_POWER",			/* 0xDE */
	"DIK_SLEEP",			/* 0xDF */
	NULL,					/* 0xE0 */
	NULL,					/* 0xE1 */
	NULL,					/* 0xE2 */
	"DIK_WAKE",				/* 0xE3 */
	NULL,					/* 0xE4 */
	"DIK_WEBSEARCH",		/* 0xE5 */
	"DIK_WEBFAVORITES",		/* 0xE6 */
	"DIK_WEBREFRESH",		/* 0xE7 */
	"DIK_WEBSTOP",			/* 0xE8 */
	"DIK_WEBFORWARD",		/* 0xE9 */
	"DIK_WEBBACK",			/* 0xEA */
	"DIK_MYCOMPUTER",		/* 0xEB */
	"DIK_MAIL",				/* 0xEC */
	"DIK_MEDIASELECT",		/* 0xED */
	NULL,					/* 0xEE */
	NULL,					/* 0xEF */
	NULL,					/* 0xF0 */
	NULL,					/* 0xF1 */
	NULL,					/* 0xF2 */
	NULL,					/* 0xF3 */
	NULL,					/* 0xF4 */
	NULL,					/* 0xF5 */
	NULL,					/* 0xF6 */
	NULL,					/* 0xF7 */
	NULL,					/* 0xF8 */
	NULL,					/* 0xF9 */
	NULL,					/* 0xFA */
	NULL,					/* 0xFB */
	NULL,					/* 0xFC */
	NULL,					/* 0xFD */
	NULL,					/* 0xFE */
	NULL					/* 0xFF */
};

/*
 *	キーボードページ
 *	FM77AV キーコードテーブル
 */
#if XM7_VER >= 2
static char *KbdPageFM77AV[] = {
	NULL, NULL,				/* 0x00 */
	"ESC", NULL,			/* 0x01 */
	"1", "ぬ",				/* 0x02 */
	"2", "ふ",				/* 0x03 */
	"3", "あ",				/* 0x04 */
	"4", "う",				/* 0x05 */
	"5", "え",				/* 0x06 */
	"6", "お",				/* 0x07 */
	"7", "や",				/* 0x08 */
	"8", "ゆ",				/* 0x09 */
	"9", "よ",				/* 0x0A */
	"0", "わ",				/* 0x0B */
	"-", "ほ",				/* 0x0C */
	"^", "へ",				/* 0x0D */
	"\\", "ー",				/* 0x0E */
	"BS", NULL,				/* 0x0F */
	"TAB", NULL,			/* 0x10 */
	"Q", "た",				/* 0x11 */
	"W", "て",				/* 0x12 */
	"E", "い",				/* 0x13 */
	"R", "す",				/* 0x14 */
	"T", "か",				/* 0x15 */
	"Y", "ん",				/* 0x16 */
	"U", "な",				/* 0x17 */
	"I", "に",				/* 0x18 */
	"O", "ら",				/* 0x19 */
	"P", "せ",				/* 0x1A */
	"@", "゛",				/* 0x1B */
	"[", "゜",				/* 0x1C */
	"RETURN", NULL,			/* 0x1D */
	"A", "ち",				/* 0x1E */
	"S", "と",				/* 0x1F */
	"D", "し",				/* 0x20 */
	"F", "は",				/* 0x21 */
	"G", "き",				/* 0x22 */
	"H", "く",				/* 0x23 */
	"J", "ま",				/* 0x24 */
	"K", "の",				/* 0x25 */
	"L", "り",				/* 0x26 */
	";", "れ",				/* 0x27 */
	":", "け",				/* 0x28 */
	"]", "む",				/* 0x29 */
	"Z", "つ",				/* 0x2A */
	"X", "さ",				/* 0x2B */
	"C", "そ",				/* 0x2C */
	"V", "ひ",				/* 0x2D */
	"B", "こ",				/* 0x2E */
	"N", "み",				/* 0x2F */
	"M", "も",				/* 0x30 */
	",", "ね",				/* 0x31 */
	".", "る",				/* 0x32 */
	"/", "め",				/* 0x33 */
	"_", "ろ",				/* 0x34 */
	"SPACE(右)", NULL,		/* 0x35 */
	"*", "テンキー",		/* 0x36 */
	"/", "テンキー",		/* 0x37 */
	"+", "テンキー",		/* 0x38 */
	"-", "テンキー",		/* 0x39 */
	"7", "テンキー",		/* 0x3A */
	"8", "テンキー",		/* 0x3B */
	"9", "テンキー",		/* 0x3C */
	"=", "テンキー",		/* 0x3D */
	"4", "テンキー",		/* 0x3E */
	"5", "テンキー",		/* 0x3F */
	"6", "テンキー",		/* 0x40 */
	",", "テンキー",		/* 0x41 */
	"1", "テンキー",		/* 0x42 */
	"2", "テンキー",		/* 0x43 */
	"3", "テンキー",		/* 0x44 */
	"RETURN", "テンキー",	/* 0x45 */
	"0", "テンキー",		/* 0x46 */
	".", "テンキー",		/* 0x47 */
	"INS", NULL,			/* 0x48 */
	"EL", NULL,				/* 0x49 */
	"CLS", NULL,			/* 0x4A */
	"DEL", NULL,			/* 0x4B */
	"DUP", NULL,			/* 0x4C */
	"↑", NULL,				/* 0x4D */
	"HOME", NULL,			/* 0x4E */
	"←", NULL,				/* 0x4F */
	"↓", NULL,				/* 0x50 */
	"→", NULL,				/* 0x51 */
	"CTRL", NULL,			/* 0x52 */
	"SHIFT(左)", NULL,		/* 0x53 */
	"SHIFT(右)", NULL,		/* 0x54 */
	"CAP", NULL,			/* 0x55 */
	"GRAPH", NULL,			/* 0x56 */
	"SPACE(左)", NULL,		/* 0x57 */
	"SPACE(中)", NULL,		/* 0x58 */
	NULL, NULL,				/* 0x59 */
	"かな", NULL,			/* 0x5A */
	NULL, NULL,				/* 0x5B */
	"BREAK", NULL,			/* 0x5C */
	"PF1",	NULL,			/* 0x5D */
	"PF2",	NULL,			/* 0x5E */
	"PF3",	NULL,			/* 0x5F */
	"PF4",	NULL,			/* 0x60 */
	"PF5",	NULL,			/* 0x61 */
	"PF6",	NULL,			/* 0x62 */
	"PF7",	NULL,			/* 0x63 */
	"PF8",	NULL,			/* 0x64 */
	"PF9",	NULL,			/* 0x65 */
	"PF10",	NULL			/* 0x66 */
};
#else
static char *KbdPageFM77AV[] = {
	NULL, NULL,				/* 0x00 */
	"ESC", NULL,			/* 0x01 */
	"1", "ヌ",				/* 0x02 */
	"2", "フ",				/* 0x03 */
	"3", "ア",				/* 0x04 */
	"4", "ウ",				/* 0x05 */
	"5", "エ",				/* 0x06 */
	"6", "オ",				/* 0x07 */
	"7", "ヤ",				/* 0x08 */
	"8", "ユ",				/* 0x09 */
	"9", "ヨ",				/* 0x0A */
	"0", "ワ",				/* 0x0B */
	"-", "ホ",				/* 0x0C */
	"^", "ヘ",				/* 0x0D */
	"\\", "ー",				/* 0x0E */
	"BS", NULL,				/* 0x0F */
	"TAB", NULL,			/* 0x10 */
	"Q", "タ",				/* 0x11 */
	"W", "テ",				/* 0x12 */
	"E", "イ",				/* 0x13 */
	"R", "ス",				/* 0x14 */
	"T", "カ",				/* 0x15 */
	"Y", "ン",				/* 0x16 */
	"U", "ナ",				/* 0x17 */
	"I", "ニ",				/* 0x18 */
	"O", "ラ",				/* 0x19 */
	"P", "セ",				/* 0x1A */
	"@", "゛",				/* 0x1B */
	"[", "゜",				/* 0x1C */
	"RETURN", NULL,			/* 0x1D */
	"A", "チ",				/* 0x1E */
	"S", "ト",				/* 0x1F */
	"D", "シ",				/* 0x20 */
	"F", "ハ",				/* 0x21 */
	"G", "キ",				/* 0x22 */
	"H", "ク",				/* 0x23 */
	"J", "マ",				/* 0x24 */
	"K", "ノ",				/* 0x25 */
	"L", "リ",				/* 0x26 */
	";", "レ",				/* 0x27 */
	":", "ケ",				/* 0x28 */
	"]", "ム",				/* 0x29 */
	"Z", "ツ",				/* 0x2A */
	"X", "サ",				/* 0x2B */
	"C", "ソ",				/* 0x2C */
	"V", "ヒ",				/* 0x2D */
	"B", "コ",				/* 0x2E */
	"N", "ミ",				/* 0x2F */
	"M", "モ",				/* 0x30 */
	",", "ネ",				/* 0x31 */
	".", "ル",				/* 0x32 */
	"/", "メ",				/* 0x33 */
	"_", "ロ",				/* 0x34 */
	"SPACE", NULL,			/* 0x35 */
	"*", "テンキー",		/* 0x36 */
	"/", "テンキー",		/* 0x37 */
	"+", "テンキー",		/* 0x38 */
	"-", "テンキー",		/* 0x39 */
	"7", "テンキー",		/* 0x3A */
	"8", "テンキー",		/* 0x3B */
	"9", "テンキー",		/* 0x3C */
	"=", "テンキー",		/* 0x3D */
	"4", "テンキー",		/* 0x3E */
	"5", "テンキー",		/* 0x3F */
	"6", "テンキー",		/* 0x40 */
	",", "テンキー",		/* 0x41 */
	"1", "テンキー",		/* 0x42 */
	"2", "テンキー",		/* 0x43 */
	"3", "テンキー",		/* 0x44 */
	"RETURN", "テンキー",	/* 0x45 */
	"0", "テンキー",		/* 0x46 */
	".", "テンキー",		/* 0x47 */
	"INS", NULL,			/* 0x48 */
	"EL", NULL,				/* 0x49 */
	"CLS", "(CLEAR)",		/* 0x4A */
	"DEL", NULL,			/* 0x4B */
	"DUP", NULL,			/* 0x4C */
	"↑", NULL,				/* 0x4D */
	"HOME", NULL,			/* 0x4E */
	"←", NULL,				/* 0x4F */
	"↓", NULL,				/* 0x50 */
	"→", NULL,				/* 0x51 */
	"CTRL", NULL,			/* 0x52 */
	"SHIFT(左)", NULL,		/* 0x53 */
	"SHIFT(右)", NULL,		/* 0x54 */
	"CAP", NULL,			/* 0x55 */
	"GRAPH", NULL,			/* 0x56 */
	NULL, NULL,				/* 0x57 */
	NULL, NULL,				/* 0x58 */
	NULL, NULL,				/* 0x59 */
	"カナ", NULL,			/* 0x5A */
	NULL, NULL,				/* 0x5B */
	"BREAK", "(STOP)",		/* 0x5C */
	"PF1",	NULL,			/* 0x5D */
	"PF2",	NULL,			/* 0x5E */
	"PF3",	NULL,			/* 0x5F */
	"PF4",	NULL,			/* 0x60 */
	"PF5",	NULL,			/* 0x61 */
	"PF6",	NULL,			/* 0x62 */
	"PF7",	NULL,			/* 0x63 */
	"PF8",	NULL,			/* 0x64 */
	"PF9",	NULL,			/* 0x65 */
	"PF10",	NULL			/* 0x66 */
};
#endif

/*
 *	インデックス→FM77AV キーコード
 */
static int FASTCALL KbdPageIndex2FM77AV(int index)
{
	int i;

	for (i=0; i<sizeof(KbdPageFM77AV)/sizeof(char *)/2; i++) {
		/* NULLのキーはスキップ */
		if (KbdPageFM77AV[i * 2] == NULL) {
			continue;
		}

		/* NULLでなければ、チェック＆デクリメント */
		if (index == 0) {
			return i;
		}
		index--;
	}

	/* エラー */
	return 0;
}

/*
 *	FM77AV キーコード→インデックス
 */
static int FASTCALL KbdPageFM77AV2Index(int keycode)
{
	int i;
	int index;

	index = 0;
	for (i=0; i<sizeof(KbdPageFM77AV)/sizeof(char *)/2; i++) {
		/* キーコードに到達したら終了 */
		if (i == keycode) {
			break;
		}

		/* NULLのキーはスキップ */
		if (KbdPageFM77AV[i * 2] == NULL) {
			continue;
		}

		/* NULLでなければ、インクリメント */
		index++;
	}

	return index;
}

/*
 *	FM77AV キーコード→DirectInput キーコード
 */
static int FASTCALL KbdPageFM77AV2DirectInput(int fm)
{
	int i;

	/* 検索 */
	for (i=0; i<256; i++) {
		if (propdat.KeyMap[i] == fm) {
			return i;
		}
	}

	/* エラー */
	return 0;
}

/*
 *	キー入力ダイアログ
 *	ダイアログ初期化
 */
static void FASTCALL KeyInDlgInit(HWND hDlg)
{
	HWND hWnd;
	RECT prect;
	RECT drect;
	int fm;
	int di;
	char formstr[128];
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

	/* キー番号テキスト初期化 */
	fm = KbdPageIndex2FM77AV(KbdPageSelectID);
	formstr[0] = '\0';
	LoadString(hAppInstance, IDC_KEYIN_LABEL, formstr, sizeof(formstr));
	_snprintf(string, sizeof(string), formstr, fm);
	hWnd = GetDlgItem(hDlg, IDC_KEYIN_LABEL);
	SetWindowText(hWnd, string);

	/* DirectInputキーテキスト初期化 */
	di = KbdPageFM77AV2DirectInput(fm);
	ASSERT((di >= 0) && (di < 256));
	hWnd = GetDlgItem(hDlg, IDC_KEYIN_KEY);
	if (KbdPageDirectInput[di]) {
		SetWindowText(hWnd, KbdPageDirectInput[di]);
	}
	else {
		LoadString(hAppInstance, IDC_KEYIN_KEY, string, sizeof(string));
		SetWindowText(hWnd, string);
	}

	/* グローバルワーク初期化 */
	KbdPageCurrentKey = di;
	GetKbd(KbdPageMap);

	/* タイマーをスタート */
	SetTimer(hDlg, IDD_KEYINDLG, 50, NULL);
}

/*
 *	キー入力ダイアログ
 *	タイマー
 */
static void FASTCALL KeyInTimer(HWND hDlg)
{
	BYTE buf[256];
	int i;
	HWND hWnd;
	char string[128];

	ASSERT(hDlg);

	/* キー入力 */
	LockVM();
	GetKbd(buf);
	UnlockVM();

	/* 今回押されたキーを特定 */
	for (i=0; i<256; i++) {
		if ((KbdPageMap[i] ^ buf[i]) & 0x80) {
			break;
		}
	}

	/* キーマップを更新し、チェック */
	memcpy(KbdPageMap, buf, 256);
	if (i >= 256) {
		return;
	}

	/* キーの番号、テキストをセット */
	KbdPageCurrentKey = i;

	hWnd = GetDlgItem(hDlg, IDC_KEYIN_KEY);
	if (KbdPageDirectInput[i]) {
		SetWindowText(hWnd, KbdPageDirectInput[i]);
	}
	else {
		LoadString(hAppInstance, IDC_KEYIN_KEY, string, sizeof(string));
		SetWindowText(hWnd, string);
	}
}

/*
 *	キー入力ダイアログ
 *	ダイアログプロシージャ
 */
static BOOL CALLBACK KeyInDlgProc(HWND hDlg, UINT iMsg,
									WPARAM wParam, LPARAM lParam)
{
	UNUSED(lParam);

	switch (iMsg) {
		/* ダイアログ初期化 */
		case WM_INITDIALOG:
			KeyInDlgInit(hDlg);

			/* IMEオフ */
			EnableIME(hDlg, FALSE);

			return TRUE;

		/* コマンド */
		case WM_COMMAND:
			if (LOWORD(wParam) != IDCANCEL) {
				return TRUE;
			}
			/* ESCキーを検知するための工夫 */
			if (KbdPageCurrentKey == 0x01) {
				EndDialog(hDlg, IDCANCEL);
				return TRUE;
			}
			KeyInTimer(hDlg);
			if (KbdPageCurrentKey == 0x01) {
				return TRUE;
			}
			EndDialog(hDlg, IDCANCEL);
			return TRUE;

		/* タイマー */
		case WM_TIMER:
			KeyInTimer(hDlg);
			return TRUE;

		/* 右クリック */
		case WM_RBUTTONDOWN:
			EndDialog(hDlg, IDOK);
			return TRUE;

		/* ウインドウ破棄 */
		case WM_DESTROY:
			KillTimer(hDlg, IDD_KEYINDLG);

			/* IMEオン */
			EnableIME(hDlg, TRUE);

			break;
	}

	/* それ以外はFALSE */
	return FALSE;
}

/*-[ キーボードページ ]-----------------------------------------------------*/

/*
 *	キーボードページ
 *	ダイアログ初期化(ヘッダーアイテム)
 */
static void FASTCALL KbdPageInitColumn(HWND hWnd)
{
	int i;
	char string[128];
	TEXTMETRIC tm;
	HDC hDC;
	LV_COLUMN lvc;
	static const UINT uHeaderTable[] = {
		IDS_KP_KEYNO,
		IDS_KP_KEYFM,
		IDS_KP_KEYKANA,
		IDS_KP_KEYDI
	};

	ASSERT(hWnd);

	/* テキストメトリックを取得 */
	hDC = GetDC(hWnd);
	GetTextMetrics(hDC, &tm);
	ReleaseDC(hWnd, hDC);

	/* 挿入ループ */
	for (i=0; i<(sizeof(uHeaderTable)/sizeof(UINT)); i++) {
		/* テキストをロード */
		LoadString(hAppInstance, uHeaderTable[i], string, sizeof(string));

		/* カラム構造体を作成 */
		lvc.mask = LVCF_FMT | LVCF_SUBITEM | LVCF_WIDTH | LVCF_TEXT;
		lvc.iSubItem = i;
		lvc.fmt = LVCFMT_LEFT;
		lvc.cx = (strlen(string) + 1) * tm.tmAveCharWidth;
		lvc.pszText = string;
		lvc.cchTextMax = strlen(string);

		/* カラム挿入 */
		ListView_InsertColumn(hWnd, i, &lvc);
	}
}

/*
 *	キーボードページ
 *	サブアイテム一括設定
 */
static void FASTCALL KbdPageSubItem(HWND hDlg)
{
	HWND hWnd;
	int i;
	int j;
	int index;

	/* リストコントロール取得 */
	hWnd = GetDlgItem(hDlg, IDC_KP_LIST);
	ASSERT(hWnd);

	/* アイテム挿入 */
	for (index=0; ; index++) {
		/* FM77AVキー番号を得る */
		i = KbdPageIndex2FM77AV(index);
		if (i == 0) {
			break;
		}

		/* 該当するDirectXキー番号を得る */
		j = KbdPageFM77AV2DirectInput(i);

		/* 文字列セット */
		if (KbdPageDirectInput[j]) {
			ListView_SetItemText(hWnd, index, 3, KbdPageDirectInput[j]);
		}
		else {
			ListView_SetItemText(hWnd, index, 3, "");
		}
	}
}

/*
 *	キーボードページ
 *	ダイアログ初期化
 */
static void FASTCALL KbdPageInit(HWND hDlg)
{
	HWND hWnd;
	LV_ITEM lvi;
	int index;
	int i;
	char string[128];

	/* シート初期化 */
	SheetInit(hDlg);

	/* リストコントロール取得 */
	hWnd = GetDlgItem(hDlg, IDC_KP_LIST);
	ASSERT(hWnd);

	/* カラム挿入 */
	KbdPageInitColumn(hWnd);

	/* アイテム挿入 */
	for (index=0; ; index++) {
		/* FM77AVキー番号を得る */
		i = KbdPageIndex2FM77AV(index);
		if (i == 0) {
			break;
		}

		/* アイテム挿入 */
		_snprintf(string, sizeof(string), "%02X", i);
		lvi.mask = LVIF_TEXT;
		lvi.pszText = string;
		lvi.cchTextMax = strlen(string);
		lvi.iItem = index;
		lvi.iSubItem = 0;
		ListView_InsertItem(hWnd, &lvi);

		/* サブアイテム×２をセット */
		if (KbdPageFM77AV[i * 2 + 0]) {
			ListView_SetItemText(hWnd, index, 1, KbdPageFM77AV[i * 2 + 0]);
		}
		if (KbdPageFM77AV[i * 2 + 1]) {
			ListView_SetItemText(hWnd, index, 2, KbdPageFM77AV[i * 2 + 1]);
		}
	}

	/* サブアイテム */
	KbdPageSubItem(hDlg);

	/* テンキーエミュレーション */
	if (propdat.bTenCursor) {
		CheckDlgButton(hDlg, IDC_KP_USEARROWFOR10, BST_CHECKED);
	}
	else {
		CheckDlgButton(hDlg, IDC_KP_USEARROWFOR10, BST_UNCHECKED);
	}

	/* テンキーエミュレーション8方向モード */
	if (propdat.bArrow8Dir) {
		CheckDlgButton(hDlg, IDC_KP_ARROW8DIR, BST_CHECKED);
	}
	else {
		CheckDlgButton(hDlg, IDC_KP_ARROW8DIR, BST_UNCHECKED);
	}

	/* 疑似リアルタイムキースキャン */
	if (propdat.bKbdReal) {
		CheckDlgButton(hDlg, IDC_KP_KBDREAL, BST_CHECKED);
	}
	else {
		CheckDlgButton(hDlg, IDC_KP_KBDREAL, BST_UNCHECKED);
	}
}

/*
 *	キーボードページ
 *	コマンド
 */
static void FASTCALL KbdPageCmd(HWND hDlg, WORD wID, WORD wNotifyCode, HWND hWnd)
{
	ASSERT(hDlg);
	UNUSED(wNotifyCode);
	UNUSED(hWnd);

	switch (wID) {
		/* 106キーボード */
		case IDC_KP_106B:
			GetDefMapKbd(propdat.KeyMap, 1);
			KbdPageSubItem(hDlg);
			break;

		/* PC-98キーボード */
		case IDC_KP_98B:
			GetDefMapKbd(propdat.KeyMap, 2);
			KbdPageSubItem(hDlg);
			break;

		/* 101キーボード */
		case IDC_KP_101B:
			GetDefMapKbd(propdat.KeyMap, 3);
			KbdPageSubItem(hDlg);
			break;
	}
}

/*
 *	キーボードページ
 *	ダブルクリック
 */
static void FASTCALL KbdPageDblClk(HWND hDlg)
{
	HWND hWnd;
	int count;
	int i;
	int j;

	ASSERT(hDlg);

	/* リストコントロール取得 */
	hWnd = GetDlgItem(hDlg, IDC_KP_LIST);
	ASSERT(hWnd);

	/* 選択されているアイテムが無ければ、リターン */
	if (ListView_GetSelectedCount(hWnd) == 0) {
		return;
	}

	/* 登録されているアイテムの個数を取得 */
	count = ListView_GetItemCount(hWnd);

	/* セレクトされているインデックスを得る */
	for (i=0; i<count; i++) {
		if (ListView_GetItemState(hWnd, i, LVIS_SELECTED)) {
			break;
		}
	}

	/* グローバルへ記憶 */
	ASSERT(i < count);
	KbdPageSelectID = i;

	/* モーダルダイアログ実行 */
	if (DialogBox(hAppInstance, MAKEINTRESOURCE(IDD_KEYINDLG),
						hDlg, (DLGPROC)KeyInDlgProc) == IDOK) {
		/* マップを修正 */
		j = KbdPageIndex2FM77AV(i);
		for (i=0; i<256; i++) {
			if (propdat.KeyMap[i] == j) {
				propdat.KeyMap[i] = 0;
			}
		}
		propdat.KeyMap[KbdPageCurrentKey] = (BYTE)j;

		/* 再描画 */
		KbdPageSubItem(hDlg);
	}
}

/*
 *	キーボードページ
 *	適用
 */
static void FASTCALL KbdPageApply(HWND hDlg)
{
	/* ステート変更 */
	uPropertyState = 2;

	/* テンキーエミュレーション */
	if (IsDlgButtonChecked(hDlg, IDC_KP_USEARROWFOR10) == BST_CHECKED) {
		propdat.bTenCursor = TRUE;
	}
	else {
		propdat.bTenCursor = FALSE;
	}

	/* テンキーエミュレーション 8方向モード */
	if (IsDlgButtonChecked(hDlg, IDC_KP_ARROW8DIR) == BST_CHECKED) {
		propdat.bArrow8Dir = TRUE;
	}
	else {
		propdat.bArrow8Dir = FALSE;
	}

	/* 疑似リアルタイムキースキャン */
	if (IsDlgButtonChecked(hDlg, IDC_KP_KBDREAL) == BST_CHECKED) {
		propdat.bKbdReal = TRUE;
	}
	else {
		propdat.bKbdReal = FALSE;
	}
}

/*
 *	キーボードページ
 *	ダイアログプロシージャ
 */
static BOOL CALLBACK KbdPageProc(HWND hDlg, UINT msg,
									 WPARAM wParam, LPARAM lParam)
{
	LPNMHDR pnmh;
	LV_KEYDOWN *plkd;

	switch (msg) {
		/* 初期化 */
		case WM_INITDIALOG:
			KbdPageInit(hDlg);
			return TRUE;

		/* コマンド */
		case WM_COMMAND:
			KbdPageCmd(hDlg, LOWORD(wParam), HIWORD(wParam), (HWND)lParam);
			return TRUE;

		/* 通知 */
		case WM_NOTIFY:
			pnmh = (LPNMHDR)lParam;
			/* ページ終了 */
			if (pnmh->code == PSN_APPLY) {
				KbdPageApply(hDlg);
				return TRUE;
			}
			/* リストビュー ダブルクリック */
			if ((pnmh->idFrom == IDC_KP_LIST) && (pnmh->code == NM_DBLCLK)) {
				KbdPageDblClk(hDlg);
				return TRUE;
			}
			/* リストビュー 特殊キー入力 */
			if ((pnmh->idFrom == IDC_KP_LIST) && (pnmh->code == LVN_KEYDOWN)) {
				plkd = (LV_KEYDOWN*)pnmh;
				/* SPACEが入力されたらキー選択 */
				if (plkd->wVKey == VK_SPACE) {
					KbdPageDblClk(hDlg);
					return TRUE;
				}
			}
			break;

		/* カーソル設定 */
		case WM_SETCURSOR:
			if (HIWORD(lParam) == WM_MOUSEMOVE) {
				PageHelp(hDlg, IDC_KP_HELP);
			}
			break;
	}

	return FALSE;
}

/*-[ ジョイスティックページ ]-----------------------------------------------*/

/*
 *	ジョイスティックページ
 *	コンボボックステーブル
 */
static const UINT JoyPageComboTable[] = {
	IDC_JP_UPC,
	IDC_JP_DOWNC,
	IDC_JP_LEFTC,
	IDC_JP_RIGHTC,
	IDC_JP_CENTERC,
	IDC_JP_AC,
	IDC_JP_BC
};

/*
 *	ジョイスティックページ
 *	コードテーブル
 */
static const UINT JoyPageCodeTable[] = {
	IDS_JP_TYPE0,
	IDC_JP_UP,
	IDC_JP_DOWN,
	IDC_JP_LEFT,
	IDC_JP_RIGHT,
	IDC_JP_A,
	IDC_JP_B
};

/*
 *	ジョイスティックページ
 *	コンボボックスセレクト
 */
static void FASTCALL JoyPageCombo(HWND hDlg, WORD wID)
{
	int i;
	int j;
	HWND hWnd;
	char string[128];

	ASSERT(hDlg);

	/* IDチェック */
	if (wID != IDC_JP_TYPEC) {
		return;
	}

	/* "使用しない"メッセージをロード */
	string[0] = '\0';
	LoadString(hAppInstance, IDS_JP_TYPE0, string, sizeof(string));

	/* コンボックスをクリア */
	for (i=0; i<sizeof(JoyPageComboTable)/sizeof(UINT); i++) {
		hWnd = GetDlgItem(hDlg, JoyPageComboTable[i]);
		ASSERT(hWnd);
		(void)ComboBox_ResetContent(hWnd);
	}

	/* タイプ取得 */
	hWnd = GetDlgItem(hDlg, IDC_JP_TYPEC);
	ASSERT(hWnd);
	j = ComboBox_GetCurSel(hWnd);

	/* キーボードの場合 */
	if (j == 3) {
		for (i=0; i<sizeof(JoyPageComboTable)/sizeof(UINT); i++) {
			/* ハンドル取得 */
			hWnd = GetDlgItem(hDlg, JoyPageComboTable[i]);
			ASSERT(hWnd);

			/* キーコード挿入 */
			(void)ComboBox_AddString(hWnd, string);
			for (j=0; j<sizeof(KbdPageFM77AV)/sizeof(char*); j+=2) {
				if (KbdPageFM77AV[j]) {
					(void)ComboBox_AddString(hWnd, KbdPageFM77AV[j]);
				}
			}
			/* キーコードにカーソル合わせ */
			if (propdat.nJoyCode[JoyPageIdx][i] == 0) {
				(void)ComboBox_SetCurSel(hWnd, 0);
				continue;
			}
			if (propdat.nJoyCode[JoyPageIdx][i] > 0x66) {
				(void)ComboBox_SetCurSel(hWnd, 0);
				continue;
			}
			j = KbdPageFM77AV2Index(propdat.nJoyCode[JoyPageIdx][i]);
			(void)ComboBox_SetCurSel(hWnd, j + 1);
		}
		return;
	}

	/* ジョイスティックの場合 */
	for (i=0; i<sizeof(JoyPageComboTable)/sizeof(UINT); i++) {
		/* ハンドル取得 */
		hWnd = GetDlgItem(hDlg, JoyPageComboTable[i]);
		ASSERT(hWnd);

		/* 文字列挿入 */
		for (j=0; j<sizeof(JoyPageCodeTable)/sizeof(UINT); j++) {
			string[0] = '\0';
			LoadString(hAppInstance, JoyPageCodeTable[j], string, sizeof(string));
			(void)ComboBox_AddString(hWnd, string);
		}

		/* カーソル設定 */
		if (propdat.nJoyCode[JoyPageIdx][i] < 0x70) {
			(void)ComboBox_SetCurSel(hWnd, 0);
			continue;
		}
		if (propdat.nJoyCode[JoyPageIdx][i] < 0x74) {
			(void)ComboBox_SetCurSel(hWnd, propdat.nJoyCode[JoyPageIdx][i] - 0x70 + 1);
			continue;
		}
		(void)ComboBox_SetCurSel(hWnd, propdat.nJoyCode[JoyPageIdx][i] - 0x74 + 5);
	}
}

/*
 *	ジョイスティックページ
 *	データセット
 */
static void FASTCALL JoyPageSet(HWND hDlg)
{
	HWND hWnd;

	ASSERT(hDlg);
	ASSERT((JoyPageIdx == 0) || (JoyPageIdx == 1));

	/* 連射コンボボックス */
	hWnd = GetDlgItem(hDlg, IDC_JP_RAPIDAC);
	ASSERT(hWnd);
	(void)ComboBox_SetCurSel(hWnd, propdat.nJoyRapid[JoyPageIdx][0]);
	hWnd = GetDlgItem(hDlg, IDC_JP_RAPIDBC);
	ASSERT(hWnd);
	(void)ComboBox_SetCurSel(hWnd, propdat.nJoyRapid[JoyPageIdx][1]);

	/* タイプコンボボックス */
	hWnd = GetDlgItem(hDlg, IDC_JP_TYPEC);
	ASSERT(hWnd);
	(void)ComboBox_SetCurSel(hWnd, propdat.nJoyType[JoyPageIdx]);

	/* コード処理 */
	JoyPageCombo(hDlg, IDC_JP_TYPEC);
}

/*
 *	ジョイスティックページ
 *	データ取得
 */
static void FASTCALL JoyPageGet(HWND hDlg)
{
	HWND hWnd;
	int i;
	int j;

	ASSERT(hDlg);
	ASSERT((JoyPageIdx == 0) || (JoyPageIdx == 1));

	/* 連射コンボボックス */
	hWnd = GetDlgItem(hDlg, IDC_JP_RAPIDAC);
	ASSERT(hWnd);
	propdat.nJoyRapid[JoyPageIdx][0] = ComboBox_GetCurSel(hWnd);
	hWnd = GetDlgItem(hDlg, IDC_JP_RAPIDBC);
	ASSERT(hWnd);
	propdat.nJoyRapid[JoyPageIdx][1] = ComboBox_GetCurSel(hWnd);

	/* タイプコンボボックス */
	hWnd = GetDlgItem(hDlg, IDC_JP_TYPEC);
	ASSERT(hWnd);
	propdat.nJoyType[JoyPageIdx] = ComboBox_GetCurSel(hWnd);

	/* コード */
	if (propdat.nJoyType[JoyPageIdx] == 3) {
		/* キーボード */
		for (i=0; i<sizeof(JoyPageComboTable)/sizeof(UINT); i++) {
			/* ハンドル取得 */
			hWnd = GetDlgItem(hDlg, JoyPageComboTable[i]);
			ASSERT(hWnd);

			/* コード変換、セット */
			j = ComboBox_GetCurSel(hWnd);
			if (j != 0) {
				j = KbdPageIndex2FM77AV(j - 1);
			}
			propdat.nJoyCode[JoyPageIdx][i] = j;
		}
		return;
	}

	/* ジョイスティック */
	for (i=0; i<sizeof(JoyPageComboTable)/sizeof(UINT); i++) {
		/* ハンドル取得 */
		hWnd = GetDlgItem(hDlg, JoyPageComboTable[i]);
		ASSERT(hWnd);

		/* コード変換、セット */
		j = ComboBox_GetCurSel(hWnd);
		if (j == 0) {
			propdat.nJoyCode[JoyPageIdx][i] = 0;
			continue;
		}
		if (j < 5) {
			propdat.nJoyCode[JoyPageIdx][i] = (j - 1) + 0x70;
			continue;
		}
		propdat.nJoyCode[JoyPageIdx][i] = (j - 5) + 0x74;
	}
}

/*
 *	ジョイスティックページ
 *	ボタン押された
 */
static void FASTCALL JoyPageButton(HWND hDlg, WORD wID)
{
	ASSERT(hDlg);

	switch (wID) {
		/* ポート1 を選択 */
		case IDC_JP_PORT1:
			JoyPageGet(hDlg);
			JoyPageIdx = 0;
			JoyPageSet(hDlg);
			break;

		/* ポート2 を選択 */
		case IDC_JP_PORT2:
			JoyPageGet(hDlg);
			JoyPageIdx = 1;
			JoyPageSet(hDlg);
			break;

		/* それ以外 */
		default:
			ASSERT(FALSE);
			break;
	}
}

/*
 *	ジョイスティックページ
 *	ダイアログ初期化
 */
static void FASTCALL JoyPageInit(HWND hDlg)
{
	HWND hWnd[2];
	int i;
	char string[128];
#if defined(LPRINT)
	int nJoyStick;
#endif

	/* シート初期化 */
	ASSERT(hDlg);
	SheetInit(hDlg);

	/* タイプコンボボックス */
	hWnd[0] = GetDlgItem(hDlg, IDC_JP_TYPEC);
	ASSERT(hWnd[0]);
	(void)ComboBox_ResetContent(hWnd[0]);
#if defined(LPRINT)
	/* プリンタ使用 */
	if (lp_use) {
		nJoyStick = 4;	/* "電波新聞社JOY I/F" を除く */
	}
	else {
		nJoyStick = 5;	/* "電波新聞社JOY I/F" を含む */
	}
	for (i=0; i<nJoyStick; i++) {
#else
	for (i=0; i<5; i++) {
#endif
		string[0] = '\0';
		LoadString(hAppInstance, IDS_JP_TYPE0 + i, string, sizeof(string));
		(void)ComboBox_AddString(hWnd[0], string);
	}
	(void)ComboBox_SetCurSel(hWnd[0], 0);

	/* 連射コンボボックス */
	hWnd[0] = GetDlgItem(hDlg, IDC_JP_RAPIDAC);
	ASSERT(hWnd[0]);
	(void)ComboBox_ResetContent(hWnd[0]);
	hWnd[1] = GetDlgItem(hDlg, IDC_JP_RAPIDBC);
	ASSERT(hWnd[1]);
	(void)ComboBox_ResetContent(hWnd[1]);
	for (i=0; i<10; i++) {
		string[0] = '\0';
		LoadString(hAppInstance, IDS_JP_RAPID0 + i, string, sizeof(string));
		(void)ComboBox_AddString(hWnd[0], string);
		(void)ComboBox_AddString(hWnd[1], string);
	}
	(void)ComboBox_SetCurSel(hWnd[0], 0);
	(void)ComboBox_SetCurSel(hWnd[1], 0);

	/* ポート選択グループボタン */
	CheckDlgButton(hDlg, IDC_JP_PORT1, BST_CHECKED);
	JoyPageIdx = 0;
	JoyPageSet(hDlg);
}

/*
 *	ジョイスティックページ
 *	適用
 */
static void FASTCALL JoyPageApply(HWND hDlg)
{
	ASSERT(hDlg);

	/* ステート変更 */
	uPropertyState = 2;

	/* データ取得 */
	JoyPageGet(hDlg);
}

/*
 *	ジョイスティックページ
 *	ダイアログプロシージャ
 */
static BOOL CALLBACK JoyPageProc(HWND hDlg, UINT msg,
									 WPARAM wParam, LPARAM lParam)
{
	LPNMHDR pnmh;

	switch (msg) {
		/* 初期化 */
		case WM_INITDIALOG:
			JoyPageInit(hDlg);
			return TRUE;

		/* コマンド */
		case WM_COMMAND:
			switch (HIWORD(wParam)) {
				/* ボタンクリック */
				case BN_CLICKED:
					JoyPageButton(hDlg, LOWORD(wParam));
					break;
				/* コンボ選択 */
				case CBN_SELCHANGE:
					JoyPageCombo(hDlg, LOWORD(wParam));
					break;
			}
			return TRUE;

		/* 通知 */
		case WM_NOTIFY:
			pnmh = (LPNMHDR)lParam;
			if (pnmh->code == PSN_APPLY) {
				JoyPageApply(hDlg);
				return TRUE;
			}
			break;

		/* カーソル設定 */
		case WM_SETCURSOR:
			if (HIWORD(lParam) == WM_MOUSEMOVE) {
				PageHelp(hDlg, IDC_JP_HELP);
			}
			break;
	}

	return FALSE;
}

/*-[ スクリーンページ ]-----------------------------------------------------*/

/*
 *	スクリーンページ
 *	ダイアログ初期化
 */
static void FASTCALL ScrPageInit(HWND hDlg)
{
	HWND hWnd;
	char string[128];
	int i;

	/* シート初期化 */
	ASSERT(hDlg);
	SheetInit(hDlg);

	/* 全画面優先モード */
	hWnd = GetDlgItem(hDlg, IDC_SCP_MODEC);
	ASSERT(hWnd);
	string[0] = '\0';
	(void)ComboBox_ResetContent(hWnd);
	for (i=0; i<5; i++) {
		LoadString(hAppInstance, IDS_SCP_400LINE + i, string, sizeof(string));
		(void)ComboBox_AddString(hWnd, string);
	}
	(void)ComboBox_SetCurSel(hWnd, propdat.nDDResolutionMode);

	/* ウィンドウモード時フルスキャン(24k) */
	if (propdat.bFullScan) {
		CheckDlgButton(hDlg, IDC_SCP_24K, BST_CHECKED);
	}
	else {
		CheckDlgButton(hDlg, IDC_SCP_24K, BST_UNCHECKED);
	}

	/* フルスクリーン時フルスキャン(24k) */
	if (propdat.bFullScanFS) {
		CheckDlgButton(hDlg, IDC_SCP_24KFS, BST_CHECKED);
	}
	else {
		CheckDlgButton(hDlg, IDC_SCP_24KFS, BST_UNCHECKED);
	}

	/* ウィンドウモード時2倍表示 */
	if (propdat.bDoubleSize) {
		CheckDlgButton(hDlg, IDC_SCP_DOUBLESIZE, BST_CHECKED);
	}
	else {
		CheckDlgButton(hDlg, IDC_SCP_DOUBLESIZE, BST_UNCHECKED);
	}

	/* 上下ステータス */
	if (propdat.bDD480Status) {
		CheckDlgButton(hDlg, IDC_SCP_CAPTIONB, BST_CHECKED);
	}
	else {
		CheckDlgButton(hDlg, IDC_SCP_CAPTIONB, BST_UNCHECKED);
	}

	/* 画面描画通知タイミング */
	if (propdat.bRasterRender) {
		CheckDlgButton(hDlg, IDC_SCP_RASTERRENDER, BST_CHECKED);
	}
	else {
		CheckDlgButton(hDlg, IDC_SCP_RASTERRENDER, BST_UNCHECKED);
	}

#if XM7_VER == 1
	/* グリーンモニタモード */
	if (propdat.bGreenMonitor) {
		CheckDlgButton(hDlg, IDC_SCP_GREENMONITOR, BST_CHECKED);
	}
	else {
		CheckDlgButton(hDlg, IDC_SCP_GREENMONITOR, BST_UNCHECKED);
	}
#endif

#if XM7_VER == 2
	/* TTLモニタモード */
	if (propdat.bTTLMonitor) {
		CheckDlgButton(hDlg, IDC_SCP_TTLMONITOR, BST_CHECKED);
	}
	else {
		CheckDlgButton(hDlg, IDC_SCP_TTLMONITOR, BST_UNCHECKED);
	}
#endif

	/* TrueColor優先 */
	if (propdat.bDDtruecolor || bWin8flag) {
		CheckDlgButton(hDlg, IDC_SCP_TRUECOLOR, BST_CHECKED);
	}
	else {
		CheckDlgButton(hDlg, IDC_SCP_TRUECOLOR, BST_UNCHECKED);
	}
	hWnd = GetDlgItem(hDlg, IDC_SCP_TRUECOLOR);
	EnableWindow(hWnd, !bWin8flag);

	/* 疑似400ライン */
	if (propdat.bPseudo400Line) {
		CheckDlgButton(hDlg, IDC_SCP_PSEUDO400LINE, BST_CHECKED);
	}
	else {
		CheckDlgButton(hDlg, IDC_SCP_PSEUDO400LINE, BST_UNCHECKED);
	}
}

/*
 *	スクリーンページ
 *	適用
 */
static void FASTCALL ScrPageApply(HWND hDlg)
{
	HWND hWnd;

	ASSERT(hDlg);

	/* ステート変更 */
	uPropertyState = 2;

	/* 全画面優先モード */
	hWnd = GetDlgItem(hDlg, IDC_SCP_MODEC);
	ASSERT(hWnd);
	propdat.nDDResolutionMode = (BYTE)ComboBox_GetCurSel(hWnd);
	if (propdat.nDDResolutionMode > 4) {
		propdat.nDDResolutionMode = DDRES_480LINE;
	}

	/* ウィンドウモード時フルスキャン(24k) */
	if (IsDlgButtonChecked(hDlg, IDC_SCP_24K) == BST_CHECKED) {
		propdat.bFullScan = TRUE;
	}
	else {
		propdat.bFullScan = FALSE;
	}

	/* フルスクリーン時フルスキャン(24k) */
	if (IsDlgButtonChecked(hDlg, IDC_SCP_24KFS) == BST_CHECKED) {
		propdat.bFullScanFS = TRUE;
	}
	else {
		propdat.bFullScanFS = FALSE;
	}

	/* ウィンドウモード時2倍表示 */
	if (IsDlgButtonChecked(hDlg, IDC_SCP_DOUBLESIZE) == BST_CHECKED) {
		propdat.bDoubleSize = TRUE;
	}
	else {
		propdat.bDoubleSize = FALSE;
	}

	/* 上下ステータス */
	if (IsDlgButtonChecked(hDlg, IDC_SCP_CAPTIONB) == BST_CHECKED) {
		propdat.bDD480Status = TRUE;
	}
	else {
		propdat.bDD480Status = FALSE;
	}

	/* ラスタレンダリング */
	if (IsDlgButtonChecked(hDlg, IDC_SCP_RASTERRENDER) == BST_CHECKED) {
		propdat.bRasterRender = TRUE;
	}
	else {
		propdat.bRasterRender = FALSE;
	}

#if XM7_VER == 1
	/* グリーンモニタモード */
	if (IsDlgButtonChecked(hDlg, IDC_SCP_GREENMONITOR) == BST_CHECKED) {
		propdat.bGreenMonitor = TRUE;
	}
	else {
		propdat.bGreenMonitor = FALSE;
	}
#endif

#if XM7_VER == 2
	/* TTLモニタモード */
	if (IsDlgButtonChecked(hDlg, IDC_SCP_TTLMONITOR) == BST_CHECKED) {
		propdat.bTTLMonitor = TRUE;
	}
	else {
		propdat.bTTLMonitor = FALSE;
	}
#endif

	/* TrueColor優先 */
	if (IsDlgButtonChecked(hDlg, IDC_SCP_TRUECOLOR) == BST_CHECKED) {
		propdat.bDDtruecolor = TRUE;
	}
	else {
		propdat.bDDtruecolor = FALSE;
	}

	/* 疑似４００ラインモード */
	if (IsDlgButtonChecked(hDlg, IDC_SCP_PSEUDO400LINE) == BST_CHECKED) {
		propdat.bPseudo400Line = TRUE;
	}
	else {
		propdat.bPseudo400Line = FALSE;
	}
}

/*
 *	スクリーンページ
 *	ダイアログプロシージャ
 */
static BOOL CALLBACK ScrPageProc(HWND hDlg, UINT msg,
									 WPARAM wParam, LPARAM lParam)
{
	LPNMHDR pnmh;

	UNUSED(wParam);

	switch (msg) {
		/* 初期化 */
		case WM_INITDIALOG:
			ScrPageInit(hDlg);
			return TRUE;

		/* 通知 */
		case WM_NOTIFY:
			pnmh = (LPNMHDR)lParam;
			if (pnmh->code == PSN_APPLY) {
				ScrPageApply(hDlg);
				return TRUE;
			}
			break;

		/* カーソル設定 */
		case WM_SETCURSOR:
			if (HIWORD(lParam) == WM_MOUSEMOVE) {
				PageHelp(hDlg, IDC_SCP_HELP);
			}
			break;
	}

	return FALSE;
}

/*-[ ポートページ ]---------------------------------------------------------*/

#if defined(MIDI) || defined(RSC)

#if defined(MIDI)
/*
 *	ポートページ
 *	コマンド
 */
static void FASTCALL PortPageCmd(HWND hDlg, WORD wID, WORD wNotifyCode, HWND hWnd)
{
	HWND hSpin;
	BOOL flag;
	int tmp;

	ASSERT(hDlg);

	/* ID別 */
	switch (wID) {
		/* MIDI発音遅延時間 */
		case IDC_POP_MIDIDLYEDIT:
			if (wNotifyCode == EN_CHANGE) {
				tmp = GetDlgItemInt(hDlg, IDC_POP_MIDIDLYEDIT, 0, FALSE) / 10;
				if (tmp < 0) {
					tmp = 0;
				}
				else if (tmp > 100) {
					tmp = 100;
				}

				hSpin = GetDlgItem(hDlg, IDC_POP_MIDIDLYSPIN);
				ASSERT(hSpin);
				UpDown_SetPos(hSpin, tmp);
			}
			break;

		/* MIDI発音遅延をサウンドバッファ長と一致させる */
		case IDC_POP_MIDIDLYSB:
			if (IsDlgButtonChecked(hDlg, IDC_POP_MIDIDLYSB) == BST_CHECKED) {
				flag = FALSE;
			}
			else {
				flag = TRUE;
			}
			hWnd = GetDlgItem(hDlg, IDC_POP_MIDIDLYSPIN);
			EnableWindow(hWnd, flag);
			hWnd = GetDlgItem(hDlg, IDC_POP_MIDIDLYEDIT);
			EnableWindow(hWnd, flag);
			break;
	}
}

/*
 *	ポートページ
 *	垂直スクロール
 */
static void FASTCALL PortPageVScroll(HWND hDlg, WORD wPos, HWND hWnd)
{
	HWND hBuddyWnd;
	char string[128];

	ASSERT(hDlg);
	ASSERT(hWnd);

	/* ウインドウハンドルをチェック */
	if (hWnd == GetDlgItem(hDlg, IDC_POP_MIDIDLYSPIN)) {
		/* MIDI発音遅延時間 */
		/* ポジションから、バディウインドウに値を設定 */
		hBuddyWnd = GetDlgItem(hDlg, IDC_POP_MIDIDLYEDIT);
		ASSERT(hBuddyWnd);
		_snprintf(string, sizeof(string), "%d", wPos * 10);
		SetWindowText(hBuddyWnd, string);
	}
}
#endif

/*
 *	ポートページ
 *	ダイアログ初期化
 */
static void FASTCALL PortPageInit(HWND hDlg)
{
#if defined(MIDI)
	MIDIOUTCAPS moc;
#endif
	HWND hWnd;
	char string[128];
	int i, numsel;

	/* シート初期化 */
	ASSERT(hDlg);
	SheetInit(hDlg);

#if defined(MIDI)
	/* MIDIポート選択 */
	numsel = 0;
	hWnd = GetDlgItem(hDlg, IDC_POP_MIDIDEVC);
	ASSERT(hWnd);
	string[0] = '\0';
	(void)ComboBox_ResetContent(hWnd);
	LoadString(hAppInstance, IDS_POP_NONE, string, sizeof(string));
	(void)ComboBox_AddString(hWnd, string);
	for (i=0; i<(int)midiOutGetNumDevs(); i++) {
		midiOutGetDevCaps(i, &moc, sizeof(moc));
		(void)ComboBox_AddString(hWnd, moc.szPname);
		if (!strcmp(moc.szPname, propdat.szMidiDevice)) {
			numsel = i + 1;
		}
	}
	(void)ComboBox_SetCurSel(hWnd, numsel);

	/* MIDI発音遅延時間 */
	hWnd = GetDlgItem(hDlg, IDC_POP_MIDIDLYSPIN);
	ASSERT(hWnd);
	UpDown_SetRange(hWnd, 100, 0);
	UpDown_SetPos(hWnd, propdat.nMidiDelay / 10);
	PortPageVScroll(hDlg, LOWORD(UpDown_GetPos(hWnd)), hWnd);
	EnableWindow(hWnd, !propdat.bMidiDelayMode);
	hWnd = GetDlgItem(hDlg, IDC_POP_MIDIDLYEDIT);
	EnableWindow(hWnd, !propdat.bMidiDelayMode);

	/* MIDI発音遅延をサウンドバッファ長と一致させる */
	if (propdat.bMidiDelayMode) {
		CheckDlgButton(hDlg, IDC_POP_MIDIDLYSB, BST_CHECKED);
	}
	else {
		CheckDlgButton(hDlg, IDC_POP_MIDIDLYSB, BST_UNCHECKED);
	}
#endif

#if defined(RSC)
	/* RS-232C使用 */
	if (propdat.bCommPortEnable) {
		CheckDlgButton(hDlg, IDC_POP_COMENABLE, BST_CHECKED);
	}
	else {
		CheckDlgButton(hDlg, IDC_POP_COMENABLE, BST_UNCHECKED);
	}

	/* RS-232Cボーレート選択 */
	hWnd = GetDlgItem(hDlg, IDC_POP_COMBPSC);
	ASSERT(hWnd);
	string[0] = '\0';
	(void)ComboBox_ResetContent(hWnd);
	for (i=0; i<5; i++) {
		LoadString(hAppInstance, IDS_POP_COM300 + i, string, sizeof(string));
		(void)ComboBox_AddString(hWnd, string);
	}
	(void)ComboBox_SetCurSel(hWnd, propdat.uCommPortBps);

	/* RS-232Cポート選択 */
	hWnd = GetDlgItem(hDlg, IDC_POP_COMPORTC);
	ASSERT(hWnd);
	string[0] = '\0';
	(void)ComboBox_ResetContent(hWnd);
	for (i=1; i<=16; i++) {
		_snprintf(string, sizeof(string), "COM%d", i);
		(void)ComboBox_AddString(hWnd, string);
	}
	(void)ComboBox_SetCurSel(hWnd, propdat.nCommPortNo - 1);
#endif
}

/*
 *	ポートページ
 *	適用
 */
static void FASTCALL PortPageApply(HWND hDlg)
{
	MIDIOUTCAPS moc;
	HWND hWnd;
	UINT uPos;
	int i;

	ASSERT(hDlg);

	/* ステート変更 */
	uPropertyState = 2;

#if defined(MIDI)
	/* MIDIポート */
	hWnd = GetDlgItem(hDlg, IDC_POP_MIDIDEVC);
	i = ComboBox_GetCurSel(hWnd);
	if (i == 0) {
		strncpy(propdat.szMidiDevice, "", sizeof(propdat.szMidiDevice));
	}
	else {
		midiOutGetDevCaps(i - 1, &moc, sizeof(moc));
		strncpy(propdat.szMidiDevice, moc.szPname, 
				sizeof(propdat.szMidiDevice));
	}

	/* MIDI発音遅延時間 */
	hWnd = GetDlgItem(hDlg, IDC_POP_MIDIDLYSPIN);
	ASSERT(hWnd);
	uPos = LOWORD(UpDown_GetPos(hWnd));
	propdat.nMidiDelay = uPos * 10;

	/* MIDI発音遅延をサウンドバッファ長と一致させる */
	if (IsDlgButtonChecked(hDlg, IDC_POP_MIDIDLYSB) == BST_CHECKED) {
		propdat.bMidiDelayMode = TRUE;
	}
	else {
		propdat.bMidiDelayMode = FALSE;
	}
#endif

#if defined(RSC)
	/* RS-232C使用 */
	if (IsDlgButtonChecked(hDlg, IDC_POP_COMENABLE) == BST_CHECKED) {
		propdat.bCommPortEnable = TRUE;
	}
	else {
		propdat.bCommPortEnable = FALSE;
	}

	/* RS-232Cボーレート */
	hWnd = GetDlgItem(hDlg, IDC_POP_COMBPSC);
	propdat.uCommPortBps = (BYTE)ComboBox_GetCurSel(hWnd);

	/* RS-232Cポート */
	hWnd = GetDlgItem(hDlg, IDC_POP_COMPORTC);
	propdat.nCommPortNo = ComboBox_GetCurSel(hWnd) + 1;
#endif
}

/*
 *	ポートページ
 *	ダイアログプロシージャ
 */
static BOOL CALLBACK PortPageProc(HWND hDlg, UINT msg,
									 WPARAM wParam, LPARAM lParam)
{
	LPNMHDR pnmh;

	switch (msg) {
		/* 初期化 */
		case WM_INITDIALOG:
			PortPageInit(hDlg);
			return TRUE;

#if defined(MIDI)
		/* コマンド */
		case WM_COMMAND:
			PortPageCmd(hDlg, LOWORD(wParam), HIWORD(wParam), (HWND)lParam);
			return TRUE;
#endif

		/* 通知 */
		case WM_NOTIFY:
			pnmh = (LPNMHDR)lParam;
			if (pnmh->code == PSN_APPLY) {
				PortPageApply(hDlg);
				return TRUE;
			}
			break;

#if defined(MIDI)
		/* 垂直スクロール */
		case WM_VSCROLL:
			PortPageVScroll(hDlg, HIWORD(wParam), (HWND)lParam);
			break;
#endif

		/* カーソル設定 */
		case WM_SETCURSOR:
			if (HIWORD(lParam) == WM_MOUSEMOVE) {
				PageHelp(hDlg, IDC_POP_HELP);
			}
			break;
	}

	return FALSE;
}

#endif	/* RSC/MIDI */

/*-[ プリンタページ ]--------------------------------------------------------*/

#if defined(LPRINT)

/*
 *	プリンタページ
 *	ダイアログ初期化
 */
static void FASTCALL LprPageInit(HWND hDlg)
{
#if !defined(JASTSOUND)
HWND hWnd;
#endif

	/* シート初期化 */
	ASSERT(hDlg);
	SheetInit(hDlg);

	/* プリンタエミュレーションモード */
	switch (propdat.uPrinterEnable) {
		case LP_EMULATION:
			CheckDlgButton(hDlg, IDC_LPP_LPREMUENABLE, BST_CHECKED);
			break;
		case LP_LOG:
			CheckDlgButton(hDlg, IDC_LPP_LPRLOGENABLE, BST_CHECKED);
			break;
#if defined(JASTSOUND)
		case LP_JASTSOUND:
			CheckDlgButton(hDlg, IDC_LPP_LPRJASTSOUNDENABLE, BST_CHECKED);
			break;
#endif
		case LP_DISABLE:
			CheckDlgButton(hDlg, IDC_LPP_LPRDISABLE, BST_CHECKED);
			break;
		default:
			ASSERT(FALSE);
	}

#if !defined(JASTSOUND)
	hWnd = GetDlgItem(hDlg, IDC_LPP_LPRJASTSOUNDENABLE);
	EnableWindow(hWnd, FALSE);
#endif

	/* OSのフォントを利用する */
	if (propdat.bLprUseOsFont) {
		CheckDlgButton(hDlg, IDC_LPP_LPROSFNT, BST_CHECKED);
	}
	else {
		CheckDlgButton(hDlg, IDC_LPP_LPROSFNT, BST_UNCHECKED);
	}

	/* 漢字を出力する */
	if (propdat.bLprOutputKanji) {
		CheckDlgButton(hDlg, IDC_LPP_LPRKANJI, BST_CHECKED);
	}
	else {
		CheckDlgButton(hDlg, IDC_LPP_LPRKANJI, BST_UNCHECKED);
	}

	/* プリンタログ出力パス */
	SetWindowText(GetDlgItem(hDlg, IDC_LPP_LPRLOGPATHNAME),
		propdat.szLprLogPath);
}

/*
 *	プリンタページ
 *	コマンド
 */
static void FASTCALL LprPageCmd(HWND hDlg, WORD wID, WORD wNotifyCode, HWND hWnd)
{
	char tmp[MAX_PATH + 1];
	char path[256 + 1];

	ASSERT(hDlg);
	UNUSED(wNotifyCode);
	UNUSED(hWnd);

	/* ID別 */
	switch (wID) {
		/* プリンタログ出力パス */
		case IDC_LPP_LPRLOGDIALOG:
#if XM7_VER == 1
#if defined(BUBBLE)
			if (!FileSelectSub(FALSE, IDS_LPRFILTER, tmp, NULL, 6)) {
#else
			if (!FileSelectSub(FALSE, IDS_LPRFILTER, tmp, NULL, 5)) {
#endif
#else
			if (!FileSelectSub(FALSE, IDS_LPRFILTER, tmp, NULL, 5)) {
#endif
				return;
			}
			_snprintf(path, sizeof(path), tmp);
			SetWindowText(GetDlgItem(hDlg, IDC_LPP_LPRLOGPATHNAME), path);
			break;
	}
}

/*
 *	プリンタページ
 *	適用
 */
static void FASTCALL LprPageApply(HWND hDlg)
{
	ASSERT(hDlg);

	/* ステート変更 */
	uPropertyState = 2;

	/* プリンタ使用 */
	if (IsDlgButtonChecked(hDlg, IDC_LPP_LPREMUENABLE) == BST_CHECKED) {
		propdat.uPrinterEnable = LP_EMULATION;
	}
	else if (IsDlgButtonChecked(hDlg, IDC_LPP_LPRLOGENABLE) == BST_CHECKED) {
		propdat.uPrinterEnable = LP_LOG;
	}
#if defined(JASTSOUND)
	else if (IsDlgButtonChecked(hDlg, IDC_LPP_LPRJASTSOUNDENABLE) == BST_CHECKED) {
		propdat.uPrinterEnable = LP_JASTSOUND;
	}
#endif
	else {
		propdat.uPrinterEnable = LP_DISABLE;
	}

	/* OSのフォントを利用する */
	if (IsDlgButtonChecked(hDlg, IDC_LPP_LPROSFNT) == BST_CHECKED) {
		propdat.bLprUseOsFont = TRUE;
	}
	else {
		propdat.bLprUseOsFont = FALSE;
	}

	/* 漢字を出力する */
	if (IsDlgButtonChecked(hDlg, IDC_LPP_LPRKANJI) == BST_CHECKED) {
		propdat.bLprOutputKanji = TRUE;
	}
	else {
		propdat.bLprOutputKanji = FALSE;
	}

	/* プリンタログ出力パス */
	GetWindowText(GetDlgItem(hDlg, IDC_LPP_LPRLOGPATHNAME),
		propdat.szLprLogPath, 256);
}

/*
 *	プリンタページ
 *	ダイアログプロシージャ
 */
static BOOL CALLBACK LprPageProc(HWND hDlg, UINT msg,
									 WPARAM wParam, LPARAM lParam)
{
	LPNMHDR pnmh;

	UNUSED(wParam);

	switch (msg) {
		/* 初期化 */
		case WM_INITDIALOG:
			LprPageInit(hDlg);
			return TRUE;

		/* コマンド */
		case WM_COMMAND:
			LprPageCmd(hDlg, LOWORD(wParam), HIWORD(wParam), (HWND)lParam);
			return TRUE;

		/* 通知 */
		case WM_NOTIFY:
			pnmh = (LPNMHDR)lParam;
			if (pnmh->code == PSN_APPLY) {
				LprPageApply(hDlg);
				return TRUE;
			}
			break;

		/* カーソル設定 */
		case WM_SETCURSOR:
			if (HIWORD(lParam) == WM_MOUSEMOVE) {
				PageHelp(hDlg, IDC_LPP_HELP);
			}
			break;
	}

	return FALSE;
}

#endif /* LPRINT */

/*-[ オプションページ ]-----------------------------------------------------*/

/*
 *	オプションページ
 *	ダイアログ初期化
 */
static void FASTCALL OptPageInit(HWND hDlg)
{
#if (XM7_VER == 1 && defined(L4CARD)) || defined(MOUSE)
	HWND hWnd;
#if defined(MOUSE)
	char string[128];
#endif
#endif

	/* シート初期化 */
	ASSERT(hDlg);
	SheetInit(hDlg);

	/* OPN */
	if (propdat.bOPNEnable) {
		CheckDlgButton(hDlg, IDC_OP_OPNB, BST_CHECKED);
	}
	else {
		CheckDlgButton(hDlg, IDC_OP_OPNB, BST_UNCHECKED);
	}

	/* WHG */
	if (propdat.bWHGEnable) {
		CheckDlgButton(hDlg, IDC_OP_WHGB, BST_CHECKED);
	}
	else {
		CheckDlgButton(hDlg, IDC_OP_WHGB, BST_UNCHECKED);
	}

	/* THG */
	if (propdat.bTHGEnable) {
		CheckDlgButton(hDlg, IDC_OP_THGB, BST_CHECKED);
	}
	else {
		CheckDlgButton(hDlg, IDC_OP_THGB, BST_UNCHECKED);
	}

#if XM7_VER >= 2
	/* ビデオディジタイズ */
	if (propdat.bDigitizeEnable) {
		CheckDlgButton(hDlg, IDC_OP_DIGITIZEB, BST_CHECKED);
	}
	else {
		CheckDlgButton(hDlg, IDC_OP_DIGITIZEB, BST_UNCHECKED);
	}
#endif

#if XM7_VER >= 2
	/* 日本語カード */
	if (propdat.bJCardEnable) {
		CheckDlgButton(hDlg, IDC_OP_JCARDB, BST_CHECKED);
	}
	else {
		CheckDlgButton(hDlg, IDC_OP_JCARDB, BST_UNCHECKED);
	}
#if XM7_VER == 2
	if (!jcard_available) {
		hWnd = GetDlgItem(hDlg, IDC_OP_JCARDB);
		EnableWindow(hWnd, FALSE);
	}
#endif
#endif

#if ((XM7_VER >= 3) || defined(FMTV151))
	/* 拡張RAM/FMTV-151 */
	if (propdat.bExtRAMEnable) {
		CheckDlgButton(hDlg, IDC_OP_RAMB, BST_CHECKED);
	}
	else {
		CheckDlgButton(hDlg, IDC_OP_RAMB, BST_UNCHECKED);
	}
#endif

#if XM7_VER == 1
#if defined(L4CARD)
	/* 400ラインカード */
	if (detect_400linecard && propdat.b400LineCardEnable) {
		CheckDlgButton(hDlg, IDC_OP_400LINEB, BST_CHECKED);
	}
	else {
		CheckDlgButton(hDlg, IDC_OP_400LINEB, BST_UNCHECKED);
	}
	if (!detect_400linecard) {
		hWnd = GetDlgItem(hDlg, IDC_OP_400LINEB);
		EnableWindow(hWnd, FALSE);
	}
#endif

#if defined(JSUB)
	/* 日本語サブシステム */
	if (propdat.bJSubEnable) {
		CheckDlgButton(hDlg, IDC_OP_JSUBB, BST_CHECKED);
	}
	else {
		CheckDlgButton(hDlg, IDC_OP_JSUBB, BST_UNCHECKED);
	}
	if (!jsub_available) {
		hWnd = GetDlgItem(hDlg, IDC_OP_JSUBB);
		EnableWindow(hWnd, FALSE);
	}
#endif

#if defined(BUBBLE)
	/* バブルメモリ */
	if (propdat.bBubbleEnable) {
		CheckDlgButton(hDlg, IDC_OP_BMCB, BST_CHECKED);
	}
	else {
		CheckDlgButton(hDlg, IDC_OP_BMCB, BST_UNCHECKED);
	}
#endif
#endif

#if defined(MOUSE)
	/* マウスエミュレーション */
	if (propdat.bMouseCapture) {
		CheckDlgButton(hDlg, IDC_OP_MOUSEEM, BST_CHECKED);
	}
	else {
		CheckDlgButton(hDlg, IDC_OP_MOUSEEM, BST_UNCHECKED);
	}

	/* マウス接続ポート */
	if (propdat.nMousePort == 1) {
		CheckDlgButton(hDlg, IDC_OP_MOUSE_PORT1, BST_CHECKED);
	}
	else if (propdat.nMousePort == 2) {
		CheckDlgButton(hDlg, IDC_OP_MOUSE_PORT2, BST_CHECKED);
	}
	else {
		CheckDlgButton(hDlg, IDC_OP_MOUSE_FMMOUSE, BST_CHECKED);
	}

	/* マウスモード切り替え操作 */
	hWnd = GetDlgItem(hDlg, IDC_OP_MOUSESWC);
	ASSERT(hWnd);
	string[0] = '\0';
	(void)ComboBox_ResetContent(hWnd);
	LoadString(hAppInstance, IDS_OP_MOUSESW_1, string, sizeof(string));
	(void)ComboBox_AddString(hWnd, string);
	LoadString(hAppInstance, IDS_OP_MOUSESW_2, string, sizeof(string));
	(void)ComboBox_AddString(hWnd, string);
	LoadString(hAppInstance, IDS_OP_MOUSESW_3, string, sizeof(string));
	(void)ComboBox_AddString(hWnd, string);
	(void)ComboBox_SetCurSel(hWnd, propdat.uMidBtnMode);
#endif
}

/*
 *	オプションページ
 *	適用
 */
static void FASTCALL OptPageApply(HWND hDlg)
{
#if defined(MOUSE)
	HWND hWnd;
#endif

	ASSERT(hDlg);

	/* ステート変更 */
	uPropertyState = 2;

	/* OPN */
	if (IsDlgButtonChecked(hDlg, IDC_OP_OPNB) == BST_CHECKED) {
		propdat.bOPNEnable = TRUE;
	}
	else {
		propdat.bOPNEnable = FALSE;
	}

	/* WHG */
	if (IsDlgButtonChecked(hDlg, IDC_OP_WHGB) == BST_CHECKED) {
		propdat.bWHGEnable = TRUE;
	}
	else {
		propdat.bWHGEnable = FALSE;
	}

	/* THG */
	if (IsDlgButtonChecked(hDlg, IDC_OP_THGB) == BST_CHECKED) {
		propdat.bTHGEnable = TRUE;
	}
	else {
		propdat.bTHGEnable = FALSE;
	}

#if XM7_VER >= 2
	/* ビデオディジタイズ */
	if (IsDlgButtonChecked(hDlg, IDC_OP_DIGITIZEB) == BST_CHECKED) {
		propdat.bDigitizeEnable = TRUE;
	}
	else {
		propdat.bDigitizeEnable = FALSE;
	}
#endif

#if XM7_VER >= 2
	/* 日本語カード */
	if (IsDlgButtonChecked(hDlg, IDC_OP_JCARDB) == BST_CHECKED) {
		propdat.bJCardEnable = TRUE;
	}
	else {
		propdat.bJCardEnable = FALSE;
	}
#endif

#if ((XM7_VER >= 3) || defined(FMTV151))
	/* 拡張RAM/FMTV-151 */
	if (IsDlgButtonChecked(hDlg, IDC_OP_RAMB) == BST_CHECKED) {
		propdat.bExtRAMEnable = TRUE;
	}
	else {
		propdat.bExtRAMEnable = FALSE;
	}
#endif

#if XM7_VER == 1
#if defined(L4CARD)
	/* 400ラインカード */
	if (IsDlgButtonChecked(hDlg, IDC_OP_400LINEB) == BST_CHECKED) {
		propdat.b400LineCardEnable = TRUE;
	}
	else {
		propdat.b400LineCardEnable = FALSE;
	}
#endif

#if defined(JSUB)
	/* 日本語サブシステム */
	if (IsDlgButtonChecked(hDlg, IDC_OP_JSUBB) == BST_CHECKED) {
		propdat.bJSubEnable = TRUE;
	}
	else {
		propdat.bJSubEnable = FALSE;
	}
#endif

#if defined(BUBBLE)
	/* バブルメモリ */
	if (IsDlgButtonChecked(hDlg, IDC_OP_BMCB) == BST_CHECKED) {
		propdat.bBubbleEnable = TRUE;
	}
	else {
		propdat.bBubbleEnable = FALSE;
	}
#endif
#endif

#if defined(MOUSE)
	/* マウスエミュレーション */
	if (IsDlgButtonChecked(hDlg, IDC_OP_MOUSEEM) == BST_CHECKED) {
		propdat.bMouseCapture = TRUE;
	}
	else {
		propdat.bMouseCapture = FALSE;
	}

	/* マウス接続ポート */
	if (IsDlgButtonChecked(hDlg, IDC_OP_MOUSE_PORT1) == BST_CHECKED) {
		propdat.nMousePort = 1;
	}
	else if (IsDlgButtonChecked(hDlg, IDC_OP_MOUSE_PORT2) == BST_CHECKED) {
		propdat.nMousePort = 2;
	}
	else {
		propdat.nMousePort = 3;
	}

	/* マウスモード切り替え操作 */
	hWnd = GetDlgItem(hDlg, IDC_OP_MOUSESWC);
	propdat.uMidBtnMode = (BYTE)ComboBox_GetCurSel(hWnd);
#endif
}

/*
 *	オプションページ
 *	ダイアログプロシージャ
 */
static BOOL CALLBACK OptPageProc(HWND hDlg, UINT msg,
									 WPARAM wParam, LPARAM lParam)
{
	LPNMHDR pnmh;

	UNUSED(wParam);

	switch (msg) {
		/* 初期化 */
		case WM_INITDIALOG:
			OptPageInit(hDlg);
			return TRUE;

		/* 通知 */
		case WM_NOTIFY:
			pnmh = (LPNMHDR)lParam;
			if (pnmh->code == PSN_APPLY) {
				OptPageApply(hDlg);
				return TRUE;
			}
			break;

		/* カーソル設定 */
		case WM_SETCURSOR:
			if (HIWORD(lParam) == WM_MOUSEMOVE) {
				PageHelp(hDlg, IDC_OP_HELP);
			}
			break;
	}

	return FALSE;
}

/*-[ 改造・その他ページ ]---------------------------------------------------*/

/*
 *	改造・その他ページ
 *	ダイアログ初期化
 */
static void FASTCALL AltPageInit(HWND hDlg)
{
#if XM7_VER == 1
	char string[256];
	HWND hWnd;
#endif

	/* シート初期化 */
	ASSERT(hDlg);
	SheetInit(hDlg);

	/* MAGUS対策処理 */
	if (propdat.bMagusPatch) {
		CheckDlgButton(hDlg, IDC_AP_MAGUSPATCH, BST_CHECKED);
	}
	else {
		CheckDlgButton(hDlg, IDC_AP_MAGUSPATCH, BST_UNCHECKED);
	}

#if XM7_VER == 1
	/* バンク切り換えイネーブル */
	hWnd = GetDlgItem(hDlg, IDC_AP_BANKSELB);
	ASSERT(hWnd);
	string[0] = '\0';
	(void)ComboBox_ResetContent(hWnd);
	LoadString(hAppInstance, IDS_AP_BANKSEL_OFF, string, sizeof(string));
	(void)ComboBox_AddString(hWnd, string);
	LoadString(hAppInstance, IDS_AP_BANKSEL_ON, string, sizeof(string));
	(void)ComboBox_AddString(hWnd, string);
	LoadString(hAppInstance, IDS_AP_BANKSEL_ON_DIPSW, string, sizeof(string));
	(void)ComboBox_AddString(hWnd, string);
	(void)ComboBox_SetCurSel(hWnd, configdat.uBankSelectEnable);

	/* FM-8モード時テープモータON時強制低速モード */
	if (propdat.bMotorOnLowSpeed) {
		CheckDlgButton(hDlg, IDC_AP_MOTORON_LOWSPEED, BST_CHECKED);
	}
	else {
		CheckDlgButton(hDlg, IDC_AP_MOTORON_LOWSPEED, BST_UNCHECKED);
	}
#endif

	/* FM-7モード時裏RAM書込挙動変更 */
	if (propdat.bRomRamWrite) {
		CheckDlgButton(hDlg, IDC_AP_ROMRAMWRITE, BST_CHECKED);
	}
	else {
		CheckDlgButton(hDlg, IDC_AP_ROMRAMWRITE, BST_UNCHECKED);
	}

	/* FDC切り離し(注:チェック状態はフラグと逆です) */
	if (propdat.bFdcEnable) {
		CheckDlgButton(hDlg, IDC_AP_FDCDISABLE, BST_UNCHECKED);
	}
	else {
		CheckDlgButton(hDlg, IDC_AP_FDCDISABLE, BST_CHECKED);
	}

	/* サブウインドウのポップアップ化 */
	if (propdat.bPopupSwnd) {
		CheckDlgButton(hDlg, IDC_AP_POPUPSWND, BST_CHECKED);
	}
	else {
		CheckDlgButton(hDlg, IDC_AP_POPUPSWND, BST_UNCHECKED);
	}

	/* ファイル選択ダイアログのセンタリング */
	if (propdat.bOFNCentering) {
		CheckDlgButton(hDlg, IDC_AP_OFNCENTERING, BST_UNCHECKED);
	}
	else {
		CheckDlgButton(hDlg, IDC_AP_OFNCENTERING, BST_CHECKED);
	}

}

/*
 *	改造・その他ページ
 *	適用
 */
static void FASTCALL AltPageApply(HWND hDlg)
{
	ASSERT(hDlg);

	/* ステート変更 */
	uPropertyState = 2;

	/* MAGUS対策処理 */
	if (IsDlgButtonChecked(hDlg, IDC_AP_MAGUSPATCH) == BST_CHECKED) {
		propdat.bMagusPatch = TRUE;
	}
	else {
		propdat.bMagusPatch = FALSE;
	}

#if XM7_VER == 1
	/* バンク切り換えイネーブル */
	propdat.uBankSelectEnable =
		ComboBox_GetCurSel(GetDlgItem(hDlg, IDC_AP_BANKSELB));

	/* FM-8モード時カセットモータON時強制低速モード */
	if (IsDlgButtonChecked(hDlg, IDC_AP_MOTORON_LOWSPEED) == BST_CHECKED) {
		propdat.bMotorOnLowSpeed = TRUE;
	}
	else {
		propdat.bMotorOnLowSpeed = FALSE;
	}
#endif

	/* FM-7モード時裏RAM書込挙動変更 */
	if (IsDlgButtonChecked(hDlg, IDC_AP_ROMRAMWRITE) == BST_CHECKED) {
		propdat.bRomRamWrite = TRUE;
	}
	else {
		propdat.bRomRamWrite = FALSE;
	}

	/* FDC切り離し(注:チェック状態はフラグと逆です) */
	if (IsDlgButtonChecked(hDlg, IDC_AP_FDCDISABLE) == BST_CHECKED) {
		propdat.bFdcEnable = FALSE;
	}
	else {
		propdat.bFdcEnable = TRUE;
	}

	/* サブウインドウのポップアップ化 */
	if (IsDlgButtonChecked(hDlg, IDC_AP_POPUPSWND) == BST_CHECKED) {
		propdat.bPopupSwnd = TRUE;
	}
	else {
		propdat.bPopupSwnd = FALSE;
	}

	/* ファイル選択ダイアログのセンタリング */
	if (IsDlgButtonChecked(hDlg, IDC_AP_OFNCENTERING) == BST_CHECKED) {
		propdat.bOFNCentering = FALSE;
	}
	else {
		propdat.bOFNCentering = TRUE;
	}
}

/*
 *	改造・その他ページ
 *	ダイアログプロシージャ
 */
static BOOL CALLBACK AltPageProc(HWND hDlg, UINT msg,
									 WPARAM wParam, LPARAM lParam)
{
	LPNMHDR pnmh;

	UNUSED(wParam);

	switch (msg) {
		/* 初期化 */
		case WM_INITDIALOG:
			AltPageInit(hDlg);
			return TRUE;

		/* 通知 */
		case WM_NOTIFY:
			pnmh = (LPNMHDR)lParam;
			if (pnmh->code == PSN_APPLY) {
				AltPageApply(hDlg);
				return TRUE;
			}
			break;

		/* カーソル設定 */
		case WM_SETCURSOR:
			if (HIWORD(lParam) == WM_MOUSEMOVE) {
				PageHelp(hDlg, IDC_OP_HELP);
			}
			break;
	}

	return FALSE;
}

/*-[ プロパティシート ]-----------------------------------------------------*/

/*
 *	プロパティシート
 *	ページ作成
 */
static HPROPSHEETPAGE FASTCALL PageCreate(UINT TemplateID,
			BOOL (CALLBACK *DlgProc)(HWND, UINT, WPARAM, LPARAM))
{
	PROPSHEETPAGE psp;

	/* 構造体を作成 */
	memset(&psp, 0, sizeof(PROPSHEETPAGE));
	psp.dwSize = PROPSHEETPAGE_V1_SIZE;
	psp.dwFlags = 0;
	psp.hInstance = hAppInstance;
	psp.u.pszTemplate = MAKEINTRESOURCE(TemplateID);
	psp.pfnDlgProc = (DLGPROC)DlgProc;

	return CreatePropertySheetPage(&psp);
}

/*
 *	プロパティシート
 *	初期化
 */
static void FASTCALL SheetInit(HWND hDlg)
{
	RECT drect;
	RECT prect;
	LONG lStyleEx;
	HWND hWnd;

	/* 初期化フラグをチェック、シート初期化済みに設定 */
	if (uPropertyState > 0) {
		return;
	}
	uPropertyState = 1;

	/* プロパティシートからヘルプボタンを除去 */
	lStyleEx = GetWindowLong(GetParent(hDlg), GWL_EXSTYLE);
	lStyleEx &= ~WS_EX_CONTEXTHELP;
	SetWindowLong(GetParent(hDlg), GWL_EXSTYLE, lStyleEx);

	/* タブの複数行表示化 */
	hWnd = PropSheet_GetTabControl(GetParent(hDlg));
	lStyleEx = GetWindowLong(hWnd, GWL_STYLE);
	lStyleEx &= ~TCS_SINGLELINE;
	lStyleEx |= TCS_MULTILINE;
	SetWindowLong(hWnd, GWL_STYLE, lStyleEx);

	/* プロパティシートを、メインウインドウの中央に寄せる */
	GetWindowRect(hMainWnd, &prect);
	GetWindowRect(GetParent(hDlg), &drect);
	drect.right -= drect.left;
	drect.bottom -= drect.top;
	drect.left = (prect.right - prect.left) / 2 + prect.left;
	drect.left -= (drect.right / 2);
	drect.top = (prect.bottom - prect.top) / 2 + prect.top;
	drect.top -= (drect.bottom / 2);
	MoveWindow(GetParent(hDlg), drect.left, drect.top, drect.right, drect.bottom, FALSE);

}

/*
 *	設定(C)
 */
void FASTCALL OnConfig(HWND hWnd)
{
	PROPSHEETHEADER pshead;
	HPROPSHEETPAGE hpspage[16];
	int i;
	int ver;
	int page;

	ASSERT(hWnd);

	/* データ転送 */
	propdat = configdat;

	/* プロパティページ作成 */
	page = 0;
	hpspage[page++] = PageCreate(IDD_GENERALPAGE, GeneralPageProc);
	hpspage[page++] = PageCreate(IDD_SOUNDPAGE, SoundPageProc);
	hpspage[page++] = PageCreate(IDD_VOLUMEPAGE, VolumePageProc);
	hpspage[page++] = PageCreate(IDD_KBDPAGE, KbdPageProc);
	hpspage[page++] = PageCreate(IDD_JOYPAGE, JoyPageProc);
	hpspage[page++] = PageCreate(IDD_SCRPAGE, ScrPageProc);
#if defined(RSC) || defined(MIDI)
	hpspage[page++] = PageCreate(IDD_PORTPAGE, PortPageProc);
#endif
#if defined(LPRINT)
	hpspage[page++] = PageCreate(IDD_LPRPAGE, LprPageProc);
#endif
	hpspage[page++] = PageCreate(IDD_OPTPAGE, OptPageProc);
	hpspage[page++] = PageCreate(IDD_ALTERPAGE, AltPageProc);

	/* プロパティページチェック */
	for (i=0; i<page; i++) {
		if (!hpspage[i]) {
			return;
		}
	}

	/* プロパティシート作成 */
	memset(&pshead, 0, sizeof(pshead));
	pshead.dwSize = PROPSHEETHEADER_V1_SIZE;
	pshead.dwFlags = PSH_NOAPPLYNOW;
	pshead.hwndParent = hWnd;
	pshead.hInstance = hAppInstance;
	pshead.pszCaption = MAKEINTRESOURCE(IDS_CONFIGCAPTION);
	pshead.nPages = page;
	pshead.u2.nStartPage = 0;
	pshead.u3.phpage = hpspage;

	/* プロパティシート実行 */
	uPropertyState = 0;
	uPropertyHelp = 0;
	PropertySheet(&pshead);

	/* 結果がok以外なら終了 */
	if (uPropertyState != 2) {
		return;
	}

	/* okなので、データ転送 */
	configdat = propdat;

	/* 適用 */
	LockVM();
#if XM7_VER >= 2
	ver = fm7_ver;
#else
	ver = fm_subtype;
#endif
	ApplyCfg();

	/* 動作機種変更を伴う場合、リセットする */
#if XM7_VER == 1 && defined(BUBBLE)
	if ((fm_subtype != FMSUB_FM8) && (boot_mode == BOOT_BUBBLE)) {
		boot_mode = BOOT_DOS;
		GetCfg();
	}
#endif

#if XM7_VER >= 2
	if (ver != fm7_ver) {
#else
	if (ver != fm_subtype) {
#endif
		system_reset();
		OnRefresh(hMainWnd);
	}

	UnlockVM();
}
#endif	/* _WIN32 */
