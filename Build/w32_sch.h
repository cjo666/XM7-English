/*
 *	FM-7 EMULATOR "XM7"
 *
 *	Copyright (C) 1999-2017 ＰＩ．(Twitter:@xm6_original)
 *	Copyright (C) 2001-2017 Ryu Takegami (Twitter:@RyuTakegami)
 *
 *	[ Win32API スケジューラ ]
 */

#ifdef _WIN32

#ifndef _w32_sch_h_
#define _w32_sch_h_

#ifdef __cplusplus
extern "C" {
#endif
/*
 *	主要エントリ
 */
void FASTCALL InitSch(void);
										/* 初期化 */
void FASTCALL CleanSch(void);
										/* クリーンアップ */
BOOL FASTCALL SelectSch(void);
										/* セレクト */
void FASTCALL ResetSch(void);
										/* 実行リセット */
void FASTCALL DrawSubWindow(void);
										/* サブウインドウ描画 */

/*
 *	主要ワーク
 */
extern DWORD dwExecTotal;
										/* 実行時間トータル */
extern DWORD dwDrawTotal;
										/* 描画回数トータル */
extern DWORD dwSoundTotal;
										/* サウンド時間トータル */
extern DWORD uTimerResolution;
										/* タイマー精度 */
extern BOOL bTapeFullSpeed;
										/* テープ高速モード */
extern BOOL bFullSpeed;
										/* 全力駆動 */
extern BOOL bAutoSpeedAdjust;	
										/* 速度自動調整フラグ */
extern DWORD dwNowTime;
										/* timeGetTimeの値 */
extern BOOL bTapeModeType;
										/* テープ高速モードタイプ */
#if defined(DEBUG)
extern DWORD dwCpuExecTime;
										/* CPU実行時間 */
extern DWORD dwDrawExecTime;
										/* レンダラ・BLT実行時間 */
extern DWORD dwPollExecTime;
										/* ポーリング実行時間 */
extern DWORD dwMainCycle;
										/* 1msあたりのメインCPU実行サイクル数 */
extern DWORD dwSubCycle;
										/* 1msあたりのサブCPU実行サイクル数 */
extern DWORD dwMainCycleAvg;
										/* 100ms間のメイン実行サイクル平均 */
extern DWORD dwSubCycleAvg;
										/* 100ms間のサブ実行サイクル平均 */
#endif
extern BYTE nTimePeriod;
										/* タイマ精度設定指示 */
#ifdef __cplusplus
}
#endif

#endif	/* _w32_sch_h_ */
#endif	/* _WIN32 */
