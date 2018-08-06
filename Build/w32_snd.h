/*
 *	FM-7 EMULATOR "XM7"
 *
 *	Copyright (C) 1999-2017 ＰＩ．(Twitter:@xm6_original)
 *	Copyright (C) 2001-2017 Ryu Takegami (Twitter:@RyuTakegami)
 *
 *	[ Win32API サウンド ]
 */

#ifdef _WIN32

#ifndef _w32_snd_h_
#define _w32_snd_h_

/*
 *	定数定義
 */
#define SNDBUF					20		/* WAV最大同時発音数 */
#define FMQUEUE_SIZE			1024	/* レジスタ書き込みキューサイズ */
#define CHSEPARATION_DEFAULT	7		/* チャンネルセパレーション標準値 */
#define FMVOLUME_DEFAULT		0		/* FM音源ボリュームデフォルト値 */
#define PSGVOLUME_DEFAULT		-2		/* PSGボリュームデフォルト値 */
#define BEEPVOLUME_DEFAULT		-24		/* BEEP音ボリュームデフォルト値 */
#define CMTVOLUME_DEFAULT		-24		/* CMT音モニタボリュームデフォルト値 */
#define WAVEVOLUME_DEFAULT		-6		/* 各種効果音ボリュームデフォルト値 */


#ifdef __cplusplus
extern "C" {
#endif
/*
 *	主要エントリ
 */
void FASTCALL InitSnd(void);
										/* 初期化 */
void FASTCALL CleanSnd(void);
										/* クリーンアップ */
BOOL FASTCALL SelectSnd(HWND hWnd);
										/* セレクト */
void FASTCALL ApplySnd(void);
										/* 適用 */
void FASTCALL PlaySnd(void);
										/* 演奏開始 */
void FASTCALL StopSnd(void);
										/* 演奏停止 */
BOOL FASTCALL ProcessSnd(BOOL bZero);
										/* バッファ充填定時処理 */
void FASTCALL WaitSnd(void);
										/* サウンド出力完了待機 */
int FASTCALL GetLevelSnd(int ch);
										/* サウンドレベル取得 */
void FASTCALL OpenCaptureSnd(char *fname);
										/* WAVキャプチャ開始 */
void FASTCALL CloseCaptureSnd(void);
										/* WAVキャプチャ終了 */
BOOL FASTCALL GetMute(int ch);
										/* ミュート状態取得 */
void FASTCALL SetMute(int ch, BOOL mute);
										/* ミュート状態設定 */
#if defined(ROMEO)
void FASTCALL ROMEO_Mute(BOOL flag);
										/* ROMEOミュート */
#endif
#if defined(FDDSND)
void FASTCALL InitFDDSnd(void);
										/* 初期化 */
void FASTCALL CleanFDDSnd(void);
										/* クリーンアップ */
#endif
void FASTCALL SetSoundVolume(void);
										/* ボリューム設定 */
void FASTCALL SetSoundVolume2(UINT uSp, int nFM, int nPSG,
							  int nBeep, int nCMT, int nWav);
										/* ボリューム設定2 */

/*
 *	主要ワーク
 */
extern UINT nSampleRate;
										/* サンプルレート(Hz、0で無し) */
extern UINT nSoundBuffer;
										/* サウンドバッファ(ダブル、ms) */
extern BOOL bInterpolation;
										/* サウンド補間モード */
extern BOOL bPreciseMix;
										/* 高精度合成モード */
extern UINT nBeepFreq;
										/* BEEP周波数(Hz) */
extern int hWavCapture;
										/* WAVキャプチャファイルハンドル */
extern BOOL bWavCapture;
										/* WAVキャプチャ開始後 */
extern UINT uStereoOut;
										/* 出力モード */
extern BOOL bForceStereo;
										/* 強制ステレオ出力 */
extern BOOL bTapeMon;
										/* テープ音モニタ */
#if defined(ROMEO)
extern BOOL bUseRomeo;
										/* ろみお使用フラグ */
#endif
#if defined(FDDSND)
extern UINT uSeekVolume;
										/* シーク音量 */
#endif
extern UINT uChSeparation;
										/* ステレオチャンネルセパレーション */
extern int nFMVolume;
										/* FM音源ボリューム */
extern int nPSGVolume;
										/* PSGボリューム */
extern int nBeepVolume;
										/* BEEP音ボリューム */
extern int nCMTVolume;
										/* CMT音モニタボリューム */
extern int nWaveVolume;
										/* 効果音ボリューム */
#if XM7_VER == 1
extern BOOL bFMXPSG;
										/* FM-X PSGエミュレーションモード(仮) */
#endif
#ifdef __cplusplus
}
#endif

#endif	/* _w32_snd_h_ */
#endif	/* _WIN32 */
