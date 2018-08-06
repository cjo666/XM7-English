/*
 *	FM-7 EMULATOR "XM7"
 *
 *	Copyright (C) 1999-2017 ＰＩ．(Twitter:@xm6_original)
 *	Copyright (C) 2001-2017 Ryu Takegami (Twitter:@RyuTakegami)
 *
 *	[ Win32API スケジューラ ]
 *
 *	RHG履歴
 *	  2001.12.25		スレッド終了指示/確認処理の安全性向上(ＰＩ．)
 *						WinNT系でのスケジューラ精度を向上(ＰＩ．)
 *	  2002.05.07		論理演算/直線補間ウィンドウの更新処理を追加
 *	  2002.06.06		スレッドの優先順位設定にSetPriorityClassを使っていたの
 *						を修正
 *	  2002.07.30		ジョイスティック,マウスのポーリング間隔を描画同期に変更
 *	  2002.08.09		逆アセンブルウィンドウがPC非同期モードでもVM停止中は強
 *						制的にPC同期になるように変更
 *	  2002.08.11		低スペックマシン向けにCPU速度を自動で落とすモードを追加
 *	  2002.11.13		スケジューラスレッドの優先順位をABOVE NORMALに変更
 *	  2002.12.25		マウスエミュレーション使用時にブレークポイントで停止し
 *						た場合の処理を改善(?)
 *	  2003.01.02		全力駆動時の優先順位設定を改善(NT only)
 *	  2003.01.13		サスペンド対策処理を追加
 *	  2003.02.26		テープ高速モード時の優先順位設定を改善(NT only)
 *	  2004.01.24		サウンド生成の時間精度を向上
 *	  2012.03.06		Windows Vista/Windows 7においてクリティカルセクションの
 *						仕様が変わっていたのでSleepとUnlockVMの順序を修正
 */

#ifdef _WIN32

#define STRICT
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <mmsystem.h>
#include <assert.h>
#include <process.h>
#include "xm7.h"
#include "tapelp.h"
#include "display.h"
#include "subctrl.h"
#include "device.h"
#include "mouse.h"
#include "w32.h"
#include "w32_sch.h"
#include "w32_draw.h"
#include "w32_snd.h"
#include "w32_kbd.h"
#include "w32_bar.h"
#include "w32_sub.h"
#include "w32_res.h"

#ifdef ROMEO
#include "juliet.h"
#endif

/*
 *	グローバル ワーク
 */
DWORD dwExecTotal;						/* 実行トータル時間(us) */
DWORD dwDrawTotal;						/* 描画トータル回数 */
DWORD uTimerResolution;					/* タイマー精度 */
BOOL bTapeFullSpeed;					/* テープ高速モードフラグ */
#if !defined(DISABLE_FULLSPEED)
BOOL bFullSpeed;						/* 全力駆動フラグ */
BOOL bAutoSpeedAdjust;					/* 速度自動調整フラグ */
#endif
DWORD dwNowTime;						/* timeGetTimeの値 */
BOOL bTapeModeType;						/* テープ高速モードタイプ */

#if defined(DEBUG)
DWORD dwCpuExecTime;					/* CPU実行時間 */
DWORD dwDrawExecTime;					/* レンダラ・BLT実行時間 */
DWORD dwPollExecTime;					/* ポーリング実行時間 */
#endif

/*
 *	スタティック ワーク
 */
static HANDLE hThread;					/* スレッドハンドル */
static UINT uThResult;					/* スレッド戻り値 */
static int nPriority;					/* スレッド実行優先順位 */
static BOOL bDrawVsync;					/* VSYNCフラグ(描画用) */
static BOOL bPollVsync;					/* VSYNCフラグ(ポーリング用) */
static DWORD dwTempTime;				/* dwTempTime(ms) */
static DWORD dwExecTime;				/* 実行時間(ms) */
static int nFrameSkip;					/* フレームスキップ数(ms) */
static BOOL bRunningBak;				/* 実行フラグ backup */
static BOOL bFastMode;					/* 高速実行中フラグ backup */

static DWORD nSpeedCheck;				/* 速度調整用カウンタ(ms) */
static DWORD dwChkTime;					/* 速度調整基準時間(ms) */
static DWORD dwSleepCount;				/* スリープ回数 */

#if defined(DEBUG)
static LONGLONG lFreq;					/* QueryPerformanceFrequencyの値 */
static LONGLONG lStartTime;				/* 測定基準時間保持用 */
#endif


/*
 *	プロトタイプ宣言
 */
static UINT WINAPI ThreadSch(LPVOID);			/* スレッド関数 */

/*
 *	初期化
 */
void FASTCALL InitSch(void)
{
	/* ワークエリア初期化 */
	hThread = NULL;
	uThResult = 0;
	nPriority = THREAD_PRIORITY_NORMAL;
	bDrawVsync = TRUE;
	bPollVsync = TRUE;
	bRunningBak = FALSE;
	bFastMode = FALSE;

	/* グローバルワーク */
	dwExecTotal = 0;
	dwDrawTotal = 0;
	dwNowTime = 0;
	bTapeFullSpeed = FALSE;
#if !defined(DISABLE_FULLSPEED)
	bFullSpeed = FALSE;
	bAutoSpeedAdjust = FALSE;
#endif
	uTimerResolution = 1;
	bTapeModeType = FALSE;

#if defined(DEBUG)
	dwCpuExecTime = 0;
	dwDrawExecTime = 0;
	dwPollExecTime = 0;
#endif

	return;
}

/*
 *	クリーンアップ
 */
void FASTCALL CleanSch(void)
{
	DWORD dwExitCode;

	/* スレッドが万一終了していなければ、終わらせる */
	while (hThread && !uThResult) {
		bCloseReq = TRUE;
		while (TRUE) {
			GetExitCodeThread(hThread, &dwExitCode);
			if (dwExitCode == STILL_ACTIVE) {
				WaitForSingleObject(hThread, 10);
			}
			else {
				CloseHandle(hThread);
				break;
			}
		}
	}

	/* タイマー間隔を戻す */
	timeEndPeriod(uTimerResolution);
}

/*
 *	セレクト
 */
BOOL FASTCALL SelectSch(void)
{
	TIMECAPS	caps;

#if defined(DEBUG)
	if (!(QueryPerformanceFrequency((LARGE_INTEGER *)&lFreq))) {
		lFreq = 0;
	}
#endif

	/* タイマー間隔を1msに */
	timeGetDevCaps(&caps, sizeof(TIMECAPS));
	if (uTimerResolution < caps.wPeriodMin) {
		uTimerResolution = caps.wPeriodMin;
	}
	if (uTimerResolution > caps.wPeriodMax) {
		uTimerResolution = caps.wPeriodMax;
	}
	timeBeginPeriod(uTimerResolution);

	/* スレッド起動 */
	hThread = (HANDLE)_beginthreadex(NULL, 0, ThreadSch, 0, 0, &uThResult);
	if (hThread == NULL) {
		return FALSE;
	}

	/* スレッド優先順位を設定 */
	if (bNTflag && bHighPriority) {
		nPriority = THREAD_PRIORITY_ABOVE_NORMAL;
	}
	else {
		nPriority = THREAD_PRIORITY_NORMAL;
	}
	SetThreadPriority(hThread, nPriority);

	return TRUE;
}

/*
 *	VSYNC通知
 */
void FASTCALL vsync_notify(void)
{
	bDrawVsync = TRUE;
	bPollVsync = TRUE;
}

#if defined(DEBUG)
/*
 *	実行時間測定開始
 */
void FASTCALL StartCpuTime(void)
{
	if (lFreq) {
		QueryPerformanceCounter((LARGE_INTEGER *)&lStartTime);
	}
}

/*
 *	実行時間測定終了
 */
DWORD FASTCALL GetCpuTime(void)
{
	LONGLONG lTempTime;

	if (lFreq) {
		QueryPerformanceCounter((LARGE_INTEGER *)&lTempTime);
		return (DWORD)(((lTempTime - lStartTime) * 1000000) / lFreq);
	}
	else {
		return (DWORD)0;
	}
}
#endif

/*
 *	1ms実行
 */
void FASTCALL ExecSch(void)
{
	DWORD dwCount;
	DWORD dwExec;

#if defined(DEBUG)
	StartCpuTime();
#endif

	/* ポーリング */
	PollKbd();
	if (bPollVsync) {
		PollJoy();
#ifdef MOUSE
		PollMos();
#endif
		bPollVsync = FALSE;
	}

#if defined(DEBUG)
	dwPollExecTime += GetCpuTime();

	StartCpuTime();
#endif

	/* サウンド合成 */
	ProcessSnd(FALSE);

	dwCount = 1000;
	while (dwCount > 0) {
		/* 中止要求が上がっていれば、即座にリターン */
		if (stopreq_flag) {
			run_flag = FALSE;
			break;
		}

		/* ここで実行 */
		dwExec = schedule_exec(dwCount);
		dwCount -= dwExec;
		dwSoundTotal += dwExec;
	}

	/* トータルタイム増加 */
	dwExecTotal += (1000 - dwCount);

#if defined(DEBUG)
	dwCpuExecTime += GetCpuTime();
#endif
}

/*
 *	描画
 */
static void FASTCALL DrawSch(void)
{
	HDC hDC;

#if defined(DEBUG)
	StartCpuTime();
#endif

	/* ドローウインドウ */
	hDC = GetDC(hDrawWnd);
	OnDraw(hDrawWnd, hDC);
	ReleaseDC(hDrawWnd, hDC);

	/* サブウインドウの更新 */
	if (!bVistaflag) {
		DrawSubWindow();
	}

#if defined(DEBUG)
	dwDrawExecTime += GetCpuTime();
#endif

	/* カウンタアップ */
	dwDrawTotal++;

#if defined(DEBUG)
	dwCpuExecTime = 0;
	dwDrawExecTime = 0;
	dwPollExecTime = 0;
#endif
}

/*
 *	サブウインドウ描画
 */
void FASTCALL DrawSubWindow(void)
{
	/* サブウインドウ(Sync時のみ) */
	if (bSync) {
		RefreshBreakPoint();
		RefreshScheduler();
		RefreshCPURegister();
		if (bSyncDisasm[0] && run_flag) {
			AddrDisAsm(MAINCPU, maincpu.pc);
		}
		if (bSyncDisasm[1] && run_flag) {
			AddrDisAsm(SUBCPU, subcpu.pc);
		}
#if XM7_VER == 1
#if defined(JSUB)
		if (bSyncDisasm[2] && run_flag) {
			AddrDisAsm(JSUBCPU, jsubcpu.pc);
		}
#endif
#if defined(Z80CARD)
		if (bSyncDisasm[3] && run_flag) {
			AddrDisAsm(MAINZ80, (WORD)mainz80.pc);
		}
#endif
#endif
		RefreshDisAsm();
		RefreshMemory();

		RefreshFDC();
		RefreshOPNReg();
		RefreshOPNDisp();
		RefreshPaletteReg();
		RefreshSubCtrl();
		RefreshKeyboard();
		RefreshMMR();
#if XM7_VER == 1
#if defined(BUBBLE)
		RefreshBMC();
#endif
#endif
#if XM7_VER >= 2
		RefreshALULine();
#endif
#if XM7_VER >= 3
		RefreshDMAC();
#endif
	}
}

/*
 *	実行リセット
 *	※VMのロックは行っていないので注意
 */
void FASTCALL ResetSch(void)
{
	nFrameSkip = 0;
	dwExecTime = timeGetTime();
}

/*
 *	速度調整リセット
 */
void FASTCALL ResetSpeedAdjuster(void)
{
	nSpeedCheck = 0;
	dwSleepCount = 0;
	dwChkTime = timeGetTime();
}

/*
 *	スレッド関数
 */
static UINT WINAPI ThreadSch(LPVOID param)
{
	BOOL fast_mode;
	int tmp;

	UNUSED(param);

	/* 初期化 */
	ResetSch();
	ResetSpeedAdjuster();

	/* 無限ループ(クローズ指示があれば終了) */
	while (!bCloseReq) {
		/* いきなりロック */
		LockVM();

		/* 実行指示が変化したかチェック */
		if (bRunningBak != run_flag) {
			bRunningBak = run_flag;

#ifdef ROMEO
			/* YMF288をミュート */
			ROMEO_Mute(!run_flag);
#endif

#ifdef MOUSE
			/* ブレークポイント停止時のマウスカーソル対策(--; */
			if (hMainWnd && !run_flag && mos_capture && !bFullScreen) {
				PostMessage(hMainWnd, WM_USER + 2, 0, 0);
			}
#endif
		}

		/* 実行指示がなければ、スリープ */
		if (!run_flag) {
			/* 無音を作ってスリープ */
			ProcessSnd(TRUE);
			UnlockVM();
			Sleep(10);
			ResetSch();
			ResetSpeedAdjuster();
			continue;
		}

		/* リセット時はカウンタ類を初期化 */
		if (reset_flag) {
			ResetSpeedAdjuster();
#ifdef ROMEO
			if (bRomeo) {
				/* ついでにろみおリセット */
				juliet_YMF288Reset();
			}
#endif
			reset_flag = FALSE;
		}

		/* 時間を取得(49日でのループを考慮) */
		dwNowTime = timeGetTime();
		dwTempTime = dwNowTime;
		if (dwTempTime < dwExecTime) {
			dwExecTime = 0;
		}

		/* 時間を比較 */
		if (dwTempTime <= dwExecTime) {
			/* 時間が余っているが、描画できるか */
			if (bDrawVsync) {
				DrawSch();
				nFrameSkip = 0;
				bDrawVsync = FALSE;
			}

			/* 再度、時間を取得(49日でのループを考慮) */
			dwNowTime = timeGetTime();
			dwTempTime = dwNowTime;
			if (dwTempTime < dwExecTime) {
				dwExecTime = 0;
			}
			if (dwTempTime > dwExecTime) {
				UnlockVM();
				continue;
			}

			/* 高速モード状態が変化したら、スレッド優先順位を変更 */
			if (bNTflag) {
#if defined(DISABLE_FULLSPEED)
				fast_mode = (tape_motor && bTapeFullSpeed);
#else
				fast_mode = ((tape_motor && bTapeFullSpeed) || bFullSpeed);
#endif
				if (fast_mode != bFastMode) {
					bFastMode = fast_mode;
					if (fast_mode) {
						SetThreadPriority(
							hThread, THREAD_PRIORITY_BELOW_NORMAL);
					}
					else {
						SetThreadPriority(hThread, nPriority);
					}
				}
			}

			/* 時間に余裕があるので、テープ高速モード判定 */
			if ((!tape_motor || !bTapeFullSpeed) || !bTapeModeType) {
				dwSleepCount ++;
#if defined(DISABLE_FULLSPEED)
				if (tape_motor && bTapeFullSpeed && !bTapeModeType) {
#else
				if (bFullSpeed || (tape_motor && bTapeFullSpeed &&
					!bTapeModeType)) {
#endif
					/* 全力駆動モード あまった時間もCPUを動かす */
					UnlockVM();

					/* とりあえず他のプロセスに制御を移す */
					if (bNTflag) {
						Sleep(0);
					}

					while (!stopreq_flag) {
						if (dwTempTime != timeGetTime()) {
							break;
						}

						/* テープ高速モード時はメインのみ全力駆動 */
#if defined(DISABLE_FULLSPEED)
						if (tape_motor && bTapeFullSpeed) {
#else
						if (tape_motor && bTapeFullSpeed && !bFullSpeed) {
#endif
							schedule_main_fullspeed();
						}
						else {
							schedule_fullspeed();
						}
					}
					continue;
				}
				else {
					UnlockVM();
					Sleep(1);
					continue;
				}
			}

			/* テープ高速モード */
			dwExecTime = dwTempTime - 1;
			if (dwExecTime > dwTempTime) {
				dwExecTime++;
			}
		}

		/* 実行 */
		ExecSch();
		nFrameSkip++;
		nSpeedCheck++;
		dwExecTime++;

		/* 自動速度調整 */
		if (nSpeedCheck >= 200) {
#if defined(DISABLE_FULLSPEED)
			speed_ratio = 10000;
#else
			if (bAutoSpeedAdjust) {
				/* 時間を取得(49日でのループを考慮) */
				dwTempTime = timeGetTime();
				if (dwTempTime < dwChkTime) {
					dwChkTime = 0;
				}

				/* 速度調整間隔+スリープ時間と実際の実行時間の比率を求める */
				speed_ratio = (nSpeedCheck + dwSleepCount) * speed_ratio;
				speed_ratio /= (dwTempTime - dwChkTime);

				/* CPU速度比率を100%から5%の間に制限する */
				if (speed_ratio > 10000) {
					speed_ratio = 10000;
				}
				else if (speed_ratio < 500) {
					speed_ratio = 500;
				}
			}
			else {
				/* 速度調整無効時は100%固定 */
				speed_ratio = 10000;
			}
#endif

			/* カウンタ類を初期化 */
			ResetSpeedAdjuster();
		}

		/* 終了対策で、ここで抜ける */
		if (bCloseReq) {
			UnlockVM();
			break;
		}

		/* Break対策 */
		if (!run_flag) {
			DrawSch();
			bDrawVsync = FALSE;
			nFrameSkip = 0;
			UnlockVM();
			continue;
		}

#if defined(DISABLE_FULLSPEED)
		tmp = 500;
#else
		/* スキップカウンタが規定値以下なら、続けて実行 */
		if (bAutoSpeedAdjust) {
			tmp = (10000 - speed_ratio) / 10;

			/* 2fps～15fps/30fpsの間に制限 */
#if XM7_VER >= 2
#if XM7_VER >= 3
			if (screen_mode & SCR_ANALOG) {
#else
			if (mode320) {
#endif
				/* 4096色/26万色モードでは最高15fps */
				if (tmp < 66) {
					tmp = 66;
				}
			}
			else {
				/* 8色モードでは最高30fps */
				if (tmp < 33) {
					tmp = 33;
				}
			}
#else
			if (tmp < 33) {
				tmp = 33;
			}
#endif
			if (tmp > 500) {
				tmp = 500;
			}
		}
		else {
			tmp = 500;
		}
#endif

		if (nFrameSkip >= tmp) {
			/* 無描画が続いているので、ここで一回描画 */
			DrawSch();
			ResetSch();
			bDrawVsync = FALSE;
		}
		UnlockVM();
	}

	/* 終了を明示するため、要求フラグを降ろす */
	bCloseReq = FALSE;

	/* スレッド終了 */
	_endthreadex(TRUE);

	return TRUE;
}

#endif	/* _WIN32 */
