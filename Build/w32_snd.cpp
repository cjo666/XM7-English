/*
 *	FM-7 EMULATOR "XM7"
 *
 *	Copyright (C) 1999-2017 ＰＩ．(Twitter:@xm6_original)
 *	Copyright (C) 2001-2017 Ryu Takegami (Twitter:@RyuTakegami)
 *							ROMEO support by usalin
 *
 *	[ Win32API サウンド ]
 *
 *	RHG履歴
 *	  2001.12.09		強制ステレオモードを追加
 *	  2001.12.25		音合成ルーチン群を一本化(ＰＩ．)
 *						OPN/WHG/THGレジスタへ書き込みを行う直前にそれまで経過
 *						した時間分だけ音を生成するようにした(ＰＩ．)
 *	  2001.12.26		サウンドバッファを短くすると不正処理で落ちる問題を修正
 *						音声合成モードが正常に動作しない問題を修正
 *	  2001.12.27		テープ音モニタの処理を変更
 *						BEEP音もFM音源と同様のタイミングで生成するように変更
 *	  2002.01.23		サウンドバッファ長変更時の処理を改善(ＰＩ．)
 *	  2002.06.15		FM-7モード時の独立PSGエミュレーション対応に伴ってTHG使
 *						用フラグの判定を変更
 *	  2002.08.31		他のDirectSound使用アプリケーションとの同時発音に対応
 *	  2002.09.21		S98ログ出力に対応
 *	  2002.11.11		FDDシーク音・CMTリレー音出力用のWAV合成処理を追加
 *	  2002.11.25		サウンドバッファサイズを大きくすると音が途切れる問題
 *						を修正
 *	  2003.01.28		FDDシーク音の最大同時合成数を20に変更
 *	  2003.02.26		Oh!FM掲載のプリンタポート接続タイプD/Aコンバータ(ジｏ
 *						○ト○○ンド!?)に対応
 *	  2003.05.15		新fmgen使用時のOPN/WHG/THGの独立プリスケーラ設定に対応
 *	  2004.03.17		サウンドバッファオーバーフロー時の処理を変更
 *	  2004.08.13		OPN/WHG/THGのopn.cへの統合に合わせた変更をおこなう
 *	  2007.12.16		パートミュート機能実装への準備を開始
 *	  2010.01.23		各音源のボリューム調整・チャンネルセパレーション設定
 *						機能を追加
 *	  2016.07.23		FM音源/PSGのレジスタ書き込みをいったんキューにため込み
 *						仮想マシンの実行時間に合わせて合成を行うように仕様変更
 */

#ifdef _WIN32

#define STRICT
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <mmsystem.h>
#define DIRECTSOUND_VERSION		0x300	/* DirectX3を指定 */
#include <dsound.h>
#include <assert.h>
#include <math.h>
#include <objbase.h>
#include <tchar.h>
#include "xm7.h"
#include "device.h"
#include "mainetc.h"
#include "opn.h"
#include "tapelp.h"
#include "cisc.h"
#include "opna.h"
#include "psg.h"
#include "keyboard.h"
#include "w32.h"
#include "w32_sch.h"
#include "w32_snd.h"
#include "juliet.h"
#if defined(MIDI)
#include "w32_midi.h"
#endif

/*
 *	グローバル ワーク
 */
UINT nSampleRate;						/* サンプリングレート */
UINT nSoundBuffer;						/* サウンドバッファサイズ */
UINT uStereoOut;						/* 出力モード */
BOOL bInterpolation;					/* サウンド補間モード */
BOOL bForceStereo;						/* 強制ステレオ出力 */
UINT nBeepFreq;							/* BEEP周波数 */
BOOL bTapeMon;							/* テープ音モニタ */
int hWavCapture;						/* WAVキャプチャハンドル */
BOOL bWavCapture;						/* WAVキャプチャ開始 */
#if defined(ROMEO)
BOOL bUseRomeo;							/* ろみお使用フラグ */
#endif
DWORD dwSoundTotal;						/* サウンドトータル時間 */
WORD nChannelMask[3];					/* チャンネルマスク */
UINT uChSeparation;						/* チャンネルセパレーション */
int nFMVolume;							/* FM音源ボリューム */
int nPSGVolume;							/* PSGボリューム */
int nBeepVolume;						/* BEEP音ボリューム */
int nCMTVolume;							/* CMT音モニタボリューム */
#if defined(FDDSND)
int nWaveVolume;						/* 各種効果音ボリューム */
#endif

/*
 *	スタティック ワーク
 */
static LPDIRECTSOUND lpds;				/* DirectSound */
static LPDIRECTSOUNDBUFFER lpdsp;		/* DirectSoundBuffer(プライマリ) */
static LPDIRECTSOUNDBUFFER lpdsb;		/* DirectSoundBuffer(セカンダリ) */
static DWORD *lpsbuf;					/* サウンド作成バッファ */
static BOOL bNowBank;					/* 現在再生中のバンク */
static UINT uBufSize;					/* サウンドバッファサイズ */
static UINT uRate;						/* 合成レート */
static UINT uTick;						/* 半バッファサイズの長さ */
static BOOL bMode;						/* サウンド補間モード */
static UINT uStereo;					/* 出力モード */
static UINT uSample;					/* サンプルカウンタ */
static UINT uBeep;						/* BEEP波形カウンタ */
#if XM7_VER >= 2
static UINT uKeyEncBeep;				/* キーエンコーダBEEP波形カウンタ */
#endif
static FM::OPN *pOPN[3];				/* OPNデバイス */
static int nScale[3];					/* OPNプリスケーラ */
static BYTE uCh3Mode[3];				/* OPN Ch.3モード */
static BOOL bInitFlag;					/* 初期化フラグ */
static WORD *pWavCapture;				/* キャプチャバッファ(64KB) */
static UINT nWavCapture;				/* キャプチャバッファ実データ */
static DWORD dwWavCapture;				/* キャプチャファイルサイズ */
static WORD uChannels;					/* 出力チャンネル数 */
static BOOL bBeepFlag;					/* BEEP出力 */
#if XM7_VER >= 2
static BOOL bKeyEncBeepFlag;			/* キーエンコーダBEEP出力 */
#endif
static BOOL bTHGUse;					/* THG使用フラグ(保存用) */
static BOOL bPartMute[3][6];			/* パートミュートフラグ */
static int nBeepLevel;					/* BEEP音出力レベル */
static int nCMTLevel;					/* CMT音モニタ出力レベル */
#if defined(FDDSND)
static int nWaveLevel;					/* 各種効果音出力レベル */
#endif
static BOOL bTapeFlag;					/* 現在のテープ出力状態 */
static BOOL bTapeFlag2;					/* 前回のテープ出力状態 */
static BYTE uTapeDelta;					/* テープ波形補間カウンタ */

#if defined(ROMEO)
static BOOL bRomeoMute;					/* ろみおミュートフラグ */
#endif

#if defined(LPRINT) && defined(JASTSOUND)
static BYTE *pDACbuf;					/* ジャストサウンド データバッファ */
static DWORD dwDACptr;					/* ジャストサウンド バッファポインタ */
#endif

#if !defined(FMGEN_DIRECT_OUTPUT)
/*
 *	スタティック ワーク (FM音源書き込みキュー)
 */
typedef struct _FMqueue {  
	WORD	reg;						/* レジスタ番号(拡張を考慮しWORD型) */
	BYTE	dat;						/* 書き込みデータ */
	BYTE	padding;					/* パディング */
} FMqueue;

static FMqueue OPNqueue[FMQUEUE_SIZE];	/* OPNレジスタ書き込みキュー */
static int uOPNqueuePtr;				/* OPNレジスタ書き込みキューポインタ */
static FMqueue WHGqueue[FMQUEUE_SIZE];	/* WHGレジスタ書き込みキュー */
static int uWHGqueuePtr;				/* WHGレジスタ書き込みキューポインタ */
static FMqueue THGqueue[FMQUEUE_SIZE];	/* THGレジスタ書き込みキュー */
static int uTHGqueuePtr;				/* THGレジスタ書き込みキューポインタ */
#endif

#if defined(FDDSND)
/*
 *	スタティック ワーク (WAV再生)
 */
static struct _WAVDATA {
	short *p;							/* 波形データポインタ */
	DWORD size;							/* データサイズ(サンプル数) */
	DWORD freq;							/* サンプリング周波数 */
} Wav[3];

static struct _WAVPLAY {
	BOOL	bPlay;						/* WAV再生フラグ */
	DWORD	dwWaveNo;					/* WAVでーたなんばー */
	DWORD	dwCount1;					/* WAVでーたかうんた(整数部) */
	DWORD	dwCount2;					/* WAVでーたかうんた(小数部) */
	DWORD	dwCount3;					/* WAV再生かうんた */
} WavP[SNDBUF];

static LPSTR WavName[] = {				/* WAVファイル名 */
	"RELAY_ON.WAV",
	"RELAYOFF.WAV",
	"FDDSEEK.WAV",
#if 0
	"HEADUP.WAV",
	"HEADDOWN.WAV",
#endif
};
#endif

/* ステレオ出力時の左右バランステーブル */
static int l_vol[3][5] = {
	{	16,	23,	 9,	16,	16	},
	{	16,	 9,	23,	 9,	23	},
	{	16,	16,	16,	23,	 9	},
};
static int r_vol[3][5] = {
	{	16,	 9,	23,	16,	16	},
	{	16,	23,	 9,	23,	 9	},
	{	16,	16,	16,	 9,	23	},
};

/*
 *	プロトタイプ宣言
 */
#ifdef __cplusplus
extern "C" {
#endif
void (*CopySoundBuffer)(DWORD *src, WORD *dst, int count);
extern void CopySndBufMMX(DWORD *src, WORD *dst, int count);
extern void CopySndBuf(DWORD *src, WORD *dst, int count);
#ifdef __cplusplus
}
#endif

/*
 *	ミュート用チャンネルマスクビット
 */
static const WORD nChannelMaskBit[6] = {
	0x000001, 0x000002, 0x000004, 0x000040, 0x000080, 0x000100
};

/*
 *	初期化
 */
void FASTCALL InitSnd(void)
{
	int i;

	/* ワークエリア初期化 */
	nSampleRate = 44100;
	nSoundBuffer = 100;
	bInterpolation = FALSE;
	uChSeparation = 9;
	nFMVolume = 0;
	nPSGVolume = -2;
	nBeepVolume = -24;
	nCMTVolume = -24;
	nWaveVolume = 0;
	nBeepFreq = 1200;
	uStereoOut = 0;
	bForceStereo = FALSE;
	bTapeMon = TRUE;

	hWavCapture = -1;
	bWavCapture = FALSE;
	pWavCapture = NULL;
	nWavCapture = 0;
	dwWavCapture = 0;

	lpds = NULL;
	lpdsp = NULL;
	lpdsb = NULL;
	lpsbuf = NULL;
	bNowBank = FALSE;
	uBufSize = 0;
	uRate = 0;
	uSample = 0;
	uBeep = 0;
#if XM7_VER >= 2
	uKeyEncBeep = 0;
#endif
	bMode = FALSE;
	uStereo = 0;
	uChannels = 1;

	bBeepFlag = FALSE;
#if XM7_VER >= 2
	bKeyEncBeepFlag = FALSE;
#endif
	bTapeFlag = FALSE;
	bTapeFlag2 = FALSE;
	uTapeDelta = 0;

	for (i=0; i<3; i++) {
		pOPN[i] = NULL;
		nScale[i] = 0;
	}
	bTHGUse = FALSE;
	memset(uCh3Mode, 0xff, sizeof(uCh3Mode));
	memset(bPartMute, 0x00, sizeof(bPartMute));
	bInitFlag = FALSE;
#if defined(ROMEO)
	bRomeoMute = FALSE;
#endif
	dwSoundTotal = 0;
	memset(nChannelMask, 0x00, sizeof(nChannelMask));
#if !defined(FMGEN_DIRECT_OUTPUT)
	memset(OPNqueue, 0x00, sizeof(OPNqueue));
	uOPNqueuePtr = 0;
	memset(WHGqueue, 0x00, sizeof(WHGqueue));
	uWHGqueuePtr = 0;
	memset(THGqueue, 0x00, sizeof(THGqueue));
	uTHGqueuePtr = 0;
#endif

#if defined(MIDI)
	/* MIDIワーク初期化 */
	nMidiDelay = 100;
	bMidiDelayMode = TRUE;
#endif

	/* 波形クリッピング関数を設定 */
	if (bMMXflag) {
		CopySoundBuffer = CopySndBufMMX;
	}
	else {
		CopySoundBuffer = CopySndBuf;
	}
}

/*
 *	クリーンアップ
 */
void FASTCALL CleanSnd(void)
{
	int i;

	/* サウンド停止 */
	StopSnd();

	/* OPNを解放 */
	for (i=0; i<3; i++) {
		if (pOPN[i]) {
			delete pOPN[i];
			pOPN[i] = NULL;
		}
	}

	/* サウンド作成バッファを解放 */
	if (lpsbuf) {
		free(lpsbuf);
		lpsbuf = NULL;
	}

	/* DirectSoundBufferを解放 */
	if (lpdsb) {
		lpdsb->Release();
		lpdsb = NULL;
	}
	if (lpdsp) {
		lpdsp->Release();
		lpdsp = NULL;
	}

	/* DirectSoundを解放 */
	if (lpds) {
		lpds->Release();
		lpds = NULL;
	}

	/* uRateをクリア */
	uRate = 0;

	/* キャプチャ関連 */
	if (hWavCapture >= 0) {
		CloseCaptureSnd();
	}
	if (pWavCapture) {
		free(pWavCapture);
		pWavCapture = NULL;
	}
	hWavCapture = -1;
	bWavCapture = FALSE;

#if defined(LPRINT) && defined(JASTSOUND)
	if (pDACbuf) {
		free(pDACbuf);
		pDACbuf = NULL;
	}
#endif
}

#if defined(FDDSND)
/*
 *	WAVEファイル読み込み (16ビットモノラルデータ専用)
 */
static BOOL FASTCALL LoadWav(LPSTR fname, struct _WAVDATA *wav)
{
	WAVEFORMATEX wfex;
	TCHAR path[_MAX_PATH];
	TCHAR drvname[_MAX_DRIVE];
	TCHAR dirname[_MAX_DIR];
	TCHAR filename[_MAX_FNAME];
	TCHAR extname[_MAX_EXT];
	BYTE buf[16];
	DWORD filSize;
	DWORD hdrSize;
	DWORD datSize;
	int fileh;

	ASSERT(fname);
	ASSERT(wav);

	/* ファイル名と拡張子を取り出す */
	_tsplitpath(fname, NULL, NULL, filename, extname);

	/* XM7のパスを得て */
	memset(path, 0, sizeof(path));
	GetModuleFileName(NULL, path, sizeof(path));
	_tsplitpath(path, drvname, dirname, NULL, NULL);

	/* パス合成 */
	_tmakepath(path, drvname, dirname, filename, extname);

	/* ファイルオープン */
	fileh = file_open(path, OPEN_R);
	if (fileh < 0) {
		return FALSE;
	}

	/* RIFFヘッダチェック */
	file_read(fileh, buf, 4);
	file_read(fileh, (BYTE *)&filSize, 4);
	buf[4] = '\0';
	if (strcmp((char *)buf, "RIFF")) {
		file_close(fileh);
		return FALSE;
	}
	filSize += 8;

	/* WAVEヘッダチェック */
	file_read(fileh, buf, 8);
	file_read(fileh, (BYTE *)&hdrSize, 4);
	buf[8] = '\0';
	if (strcmp((char *)buf, "WAVEfmt ")) {
		file_close(fileh);
		return FALSE;
	}
	hdrSize += (12 + 8);

	/* WAVEFORMATEXチェック */
	file_read(fileh, (BYTE *)&wfex, sizeof(wfex));
	if ((wfex.wFormatTag != WAVE_FORMAT_PCM) ||
		(wfex.nChannels != 1) || (wfex.wBitsPerSample != 16)) {
		/* 16ビットモノラル・リニアPCM以外は不可 */
		file_close(fileh);
		return FALSE;
	}

	/* dataチャンク検索 */
	while (hdrSize < filSize) {
		/* チャンクヘッダ読み込み */
		file_seek(fileh, hdrSize);
		file_read(fileh, buf, 4);
		file_read(fileh, (BYTE *)&datSize, 4);
		buf[4] = '\0';

		/* 次のチャンクヘッダオフセットを計算 */
		hdrSize += (datSize + 8);

		if (strcmp((char *)buf, "data") == 0) {
			/* dataチャンク読み込み */
			wav->size = datSize / 2;
			wav->freq = wfex.nSamplesPerSec;
			wav->p = (short *)malloc(datSize);
			if (wav->p == NULL) {
				file_close(fileh);
				return FALSE;
			}
			if (!file_read(fileh, (BYTE *)wav->p, wav->size)) {
				file_close(fileh);
				free(wav->p);
				wav->p = NULL;
				return FALSE;
			}
			file_close(fileh);
			return TRUE;
		}
	}

	/* dataチャンク発見できず */
	file_close(fileh);
	return FALSE;
}

/*
 *	FDDサウンド 初期化
 */
void FASTCALL InitFDDSnd(void)
{
	int i;

	/* ワーク初期化 */
	for (i=0; i<SNDBUF; i++) {
		memset(&WavP[i], 0, sizeof(_WAVPLAY));
	}

	/* WAVファイル読み込み */
	for (i=0; i<sizeof(Wav) / sizeof(_WAVDATA); i++) {
		if (!LoadWav(WavName[i], &Wav[i])) {
			Wav[i].size = 0;
			Wav[i].freq = 0;
		}
	}
}

/*
 *	FDDサウンド クリーンアップ
 */
void FASTCALL CleanFDDSnd(void)
{
	int i;

	for (i=0; i<sizeof(Wav) / sizeof(_WAVDATA); i++) {
		if (Wav[i].p) {
			free(Wav[i].p);
			Wav[i].p = NULL;
		}
	}
}
#endif

/*
 *	レジスタ設定
 */
static void FASTCALL SetReg(FM::OPN *pOPN, BYTE *reg)
{
	int i;

	/* PSG */
	for (i=0; i<16; i++) {
		pOPN->SetReg((BYTE)i, reg[i]);
	}

	/* FM音源キーオフ */
	for (i=0; i<3; i++) {
		pOPN->SetReg(0x28, (BYTE)i);
	}

	/* FM音源レジスタ */
	for (i=0x30; i<0xb4; i++) {
		pOPN->SetReg((BYTE)i, reg[i]);
	}

	/* FM音源動作モード */
	pOPN->SetReg(0x27, reg[0x27] & 0xc0);
}

/*
 *	セレクト
 */
BOOL FASTCALL SelectSnd(HWND hWnd)
{
	PCMWAVEFORMAT pcmwf;
	DSBUFFERDESC dsbd;
	WAVEFORMATEX wfex;
	int i;

	/* assert */
	ASSERT(hWnd);

	/* 起動フラグ立てる */
	bInitFlag = TRUE;

	/* パラメータを設定 */
	uRate = nSampleRate;
	uTick = nSoundBuffer;
	bMode = bInterpolation;
	uStereo = uStereoOut;
	uTapeDelta = 0;
	if ((uStereo > 0) || bForceStereo) {
		uChannels = 2;
	}
	else {
		uChannels = 1;
	}

	/* rate==0なら、何もしない */
	if (uRate == 0) {
		return TRUE;
	}

	/* DiectSoundオブジェクト作成 */
	if (FAILED(DirectSoundCreate(NULL, &lpds, NULL))) {
		/* デフォルトデバイスなしか、使用中 */
		return TRUE;
	}

	/* 協調レベルを設定(優先協調) */
	if (FAILED(lpds->SetCooperativeLevel(hWnd, DSSCL_PRIORITY))) {
		return FALSE;
	}

	/* プライマリバッファを作成 */
	memset(&dsbd, 0, sizeof(dsbd));
	dsbd.dwSize = sizeof(dsbd);
	dsbd.dwFlags = DSBCAPS_PRIMARYBUFFER;
	if (FAILED(lpds->CreateSoundBuffer(&dsbd, &lpdsp, NULL))) {
		return FALSE;
	}

	/* プライマリバッファのフォーマットを指定 */
	memset(&wfex, 0, sizeof(wfex));
	wfex.wFormatTag = WAVE_FORMAT_PCM;
	wfex.nChannels = uChannels;
	wfex.nSamplesPerSec = uRate;
	wfex.nBlockAlign = (WORD)(2 * uChannels);
	wfex.nAvgBytesPerSec = wfex.nSamplesPerSec * wfex.nBlockAlign;
	wfex.wBitsPerSample = 16;
	if (FAILED(lpdsp->SetFormat(&wfex))) {
		return FALSE;
	}

	/* セカンダリバッファを作成 */
	memset(&pcmwf, 0, sizeof(pcmwf));
	pcmwf.wf.wFormatTag = WAVE_FORMAT_PCM;
	pcmwf.wf.nChannels = uChannels;
	pcmwf.wf.nSamplesPerSec = uRate;
	pcmwf.wf.nBlockAlign = (WORD)(2 * uChannels);
	pcmwf.wf.nAvgBytesPerSec = pcmwf.wf.nSamplesPerSec * pcmwf.wf.nBlockAlign;
	pcmwf.wBitsPerSample = 16;
	memset(&dsbd, 0, sizeof(dsbd));
	dsbd.dwSize = sizeof(dsbd);
	dsbd.dwFlags = DSBCAPS_GLOBALFOCUS | DSBCAPS_GETCURRENTPOSITION2;
	dsbd.dwBufferBytes = (pcmwf.wf.nAvgBytesPerSec * uTick) / 1000;
	dsbd.dwBufferBytes += (DWORD)7;
	dsbd.dwBufferBytes &= (DWORD)0xfffffff8;	/* 8バイト境界 */
	uBufSize = dsbd.dwBufferBytes;
	dsbd.lpwfxFormat = (LPWAVEFORMATEX)&pcmwf;
	if (FAILED(lpds->CreateSoundBuffer(&dsbd, &lpdsb, NULL))) {
		return FALSE;
	}

	/* サウンドバッファを作成(セカンダリバッファの半分の時間で、DWORD) */
	lpsbuf = (DWORD *)malloc(uBufSize);
	if (lpsbuf == NULL) {
		return FALSE;
	}
	memset(lpsbuf, 0, uBufSize);

	/* サンプルカウンタ、サウンド時間をクリア */
	uSample = 0;
	dwSoundTotal = 0;

	/* OPNデバイスを作成 */
	for (i=0; i<3; i++) {
		pOPN[i] = new FM::OPN;
#if XM7_VER == 1
		if ((i == 1) && fmx_flag) {
			pOPN[i]->Init(PSG_CLOCK_MSX, uRate, bMode, NULL);
		}
		else {
			pOPN[i]->Init(OPN_CLOCK * 100, uRate, bMode, NULL);
		}
#else
		pOPN[i]->Init(OPN_CLOCK * 100, uRate, bMode, NULL);
#endif
		pOPN[i]->Reset();
		pOPN[i]->SetReg(0x27, 0);
	}

	/* THG SSGのエンベロープモードをPSG互換に設定 */
	pOPN[OPN_THG]->psg.SetSSGEnvMode(FALSE);
	bTHGUse = FALSE;

	/* 再セレクトに備え、レジスタ設定 */
	for (i=0; i<3; i++) {
		nScale[i] = 0;
	}
	opn_notify(0x27, 0);
	whg_notify(0x27, 0);
	thg_notify(0x27, 0);
	for (i=0; i<3; i++) {
		SetReg(pOPN[i], opn_reg[i]);
	}

	/* キャプチャ関連 */
	if (!pWavCapture) {
		pWavCapture = (WORD *)malloc(sizeof(WORD) * 0x8000);
	}
	ASSERT(hWavCapture == -1);
	ASSERT(!bWavCapture);

#if defined(LPRINT) && defined(JASTSOUND)
	/* ジャストサウンド 初期化 */
	pDACbuf = (BYTE *)malloc((uRate * uTick) / 100);
	pDACbuf[0] = 0x80;
	dwDACptr = 0;
#endif

	/* サウンドスタート */
	bNowBank = FALSE;
	PlaySnd();

	/* ボリューム設定 */
	SetSoundVolume();

	return TRUE;
}

/*
 *	適用
 */
void FASTCALL ApplySnd(void)
{
	/* 起動処理時は、リターン */
	if (!bInitFlag) {
		return;
	}

	/* ROMEO未使用時はミュート */
#if defined(ROMEO)
	if (bRomeo) {
		ROMEO_Mute(!bUseRomeo);
	}
#endif

	/* パラメータ一致チェック */
	if ((uRate == nSampleRate) && (uTick == nSoundBuffer) &&
		(bMode == bInterpolation) && (uStereo == uStereoOut)) {
		return;
	}

	/* 既に準備ができているなら、解放 */
	if (uRate != 0) {
		CleanSnd();
	}

	/* 再セレクト */
	SelectSnd(hMainWnd);

	/* ボリューム設定 */
	SetSoundVolume();
}

/*
 *	ボリューム設定
 */
void FASTCALL SetSoundVolume(void)
{
	int i;

	/* FM音源/PSGボリューム設定 */
	for (i=0; i<3; i++) {
		ASSERT(pOPN[i]);

		if (pOPN[i]) {
			pOPN[i]->SetVolumeFM(nFMVolume * 2);
			pOPN[i]->psg.SetVolume(nPSGVolume * 2);
		}
	}

	/* BEEP音/CMT音/各種効果音ボリューム設定 */
	nBeepLevel = (int)(32767.0 * pow(10.0, nBeepVolume / 20.0));
	nCMTLevel = (int)(32767.0 * pow(10.0, nCMTVolume / 20.0));
#if defined(FDDSND)
	nWaveLevel = (int)(32767.0 * pow(10.0, nWaveVolume / 20.0));
#endif

	/* チャンネルセパレーション設定 */
	l_vol[0][1] = l_vol[1][2] = l_vol[2][3] = l_vol[1][4] =
	r_vol[1][1] = r_vol[0][2] = r_vol[1][3] = r_vol[2][4] = 16 + uChSeparation;
	r_vol[0][1] = r_vol[1][2] = r_vol[2][3] = r_vol[1][4] =
	l_vol[1][1] = l_vol[0][2] = l_vol[1][3] = l_vol[2][4] = 16 - uChSeparation;
}

/*
 *	ボリューム設定2(設定ダイアログ用)
 */
void FASTCALL SetSoundVolume2(UINT uSp, int nFM, int nPSG,
							  int nBeep, int nCMT, int nWav)
{
	int i;

	/* FM音源/PSGボリューム設定 */
	for (i=0; i<3; i++) {
		ASSERT(pOPN[i]);

		if (pOPN[i]) {
			pOPN[i]->SetVolumeFM(nFM * 2);
			pOPN[i]->psg.SetVolume(nPSG * 2);
		}
	}

	/* BEEP音/CMT音/各種効果音ボリューム設定 */
	nBeepLevel = (int)(32767.0 * pow(10.0, nBeep / 20.0));
	nCMTLevel = (int)(32767.0 * pow(10.0, nCMT / 20.0));
#if defined(FDDSND)
	nWaveLevel = (int)(32767.0 * pow(10.0, nWav / 20.0));
#endif

	/* チャンネルセパレーション設定 */
	l_vol[0][1] = l_vol[1][2] = l_vol[2][3] = l_vol[1][4] =
	r_vol[1][1] = r_vol[0][2] = r_vol[1][3] = r_vol[2][4] = 16 + uSp;
	r_vol[0][1] = r_vol[1][2] = r_vol[2][3] = r_vol[1][4] =
	l_vol[1][1] = l_vol[0][2] = l_vol[1][3] = l_vol[2][4] = 16 - uSp;
}

/*
 *	演奏開始
 */
void FASTCALL PlaySnd()
{
	HRESULT hr;
	WORD *ptr1, *ptr2;
	DWORD size1, size2;

	if (lpdsb) {
		/* バッファをすべてクリアする */
		if (lpsbuf) {
			memset(lpsbuf, 0, uBufSize);
		}

		/* ロック */
		hr = lpdsb->Lock(0, uBufSize, (void **)&ptr1, &size1,
						(void**)&ptr2, &size2, 0);
		/* バッファが失われていれば、リストア */
		if (hr == DSERR_BUFFERLOST) {
			lpdsb->Restore();
		}
		/* ロック成功した場合のみ、セット */
		if (SUCCEEDED(hr)) {
			if (ptr1) {
				memset(ptr1, 0, size1);
			}
			if (ptr2) {
				memset(ptr2, 0, size2);
			}

			/* アンロック */
			lpdsb->Unlock(ptr1, size1, ptr2, size2);
		}

		/* 演奏開始 */
		lpdsb->Play(0, 0, DSBPLAY_LOOPING);
	}

	/* サンプルカウンタ、サウンド時間をクリア */
	uSample = 0;
	dwSoundTotal = 0;
}

/*
 *	演奏停止
 */
void FASTCALL StopSnd()
{
	if (lpdsb) {
		lpdsb->Stop();
	}
}

/*
 *	BEEP合成
 */
static void FASTCALL BeepSnd(int32 *buf, int samples)
{
	int sf;
	int i;

	/* BEEP音出力チェック */
	if (!bBeepFlag) {
		return;
	}

	/* サンプル書き込み */
	for (i=0; i<samples; i++) {
		/* 矩形波を作成 */
		sf = (int)(uBeep * nBeepFreq * 2);
		sf /= (int)uRate;

		/* 偶・奇に応じてサンプル書き込み */
		if (uChannels == 1) {
			if (sf & 1) {
				*buf++ += nBeepLevel;
			}
			else {
				*buf++ -= nBeepLevel;
			}
		}
		else {
			if (sf & 1) {
				*buf++ += nBeepLevel;
				*buf++ += nBeepLevel;
			}
			else {
				*buf++ -= nBeepLevel;
				*buf++ -= nBeepLevel;
			}
		}

		/* カウンタアップ */
		uBeep++;
		if (uBeep >= uRate) {
			uBeep = 0;
		}
	}
}

#if XM7_VER >= 2
/*
 *	BEEP合成
 */
static void FASTCALL KeyEncBeepSnd(int32 *buf, int samples)
{
	int sf;
	int i;

	/* BEEP音出力チェック */
	if (!bKeyEncBeepFlag) {
		return;
	}

	/* サンプル書き込み */
	for (i=0; i<samples; i++) {
		/* 矩形波を作成 */
		sf = (int)(uKeyEncBeep * 2350 * 2);
		sf /= (int)uRate;

		/* 偶・奇に応じてサンプル書き込み */
		if (uChannels == 1) {
			if (sf & 1) {
				*buf++ += nBeepLevel;
			}
			else {
				*buf++ -= nBeepLevel;
			}
		}
		else {
			if (sf & 1) {
				*buf++ += nBeepLevel;
				*buf++ += nBeepLevel;
			}
			else {
				*buf++ -= nBeepLevel;
				*buf++ -= nBeepLevel;
			}
		}

		/* カウンタアップ */
		uKeyEncBeep++;
		if (uKeyEncBeep >= uRate) {
			uKeyEncBeep = 0;
		}
	}
}
#endif

/*
 *	テープ合成
 */
static void FASTCALL TapeSnd(int32 *buf, int samples)
{
	DWORD dat;
	int i;
	int tmp;

	/* テープ出力チェック */
	if (!tape_motor || !bTapeMon) {
		return;
	}

	/* 波形分割数を求める */
	if ((uRate % 11025) == 0) {
		tmp = (uRate * 5) / 44100;
	}
	else {
		tmp = (uRate * 5) / 48000;
	}

	/* 出力状態が変化した場合、波形補間を開始する */
	if (bTapeFlag != bTapeFlag2) {
		if (!uTapeDelta) {
			uTapeDelta = 1;
		}
		else {
			uTapeDelta = (BYTE)(tmp - uTapeDelta + 1);
		}
	}

	/* サンプル書き込み */
	for (i=0; i<samples; i++) {
		if (uTapeDelta) {
			/* 波形補間あり */
			dat = (0x1000 / tmp) * uTapeDelta;
			if (bTapeFlag) {
				dat = dat - nCMTLevel;
			}
			else {
				dat = nCMTLevel - dat;
			}

			uTapeDelta ++;
			if (uTapeDelta > tmp) {
				uTapeDelta = 0;
			}
		}
		else {
			/* 波形補間なし */
			if (bTapeFlag) {
				dat = nCMTLevel;
			}
			else {
				dat = -nCMTLevel;
			}
		}

		*buf++ += dat;
		if (uChannels == 2) {
			*buf++ += dat;
		}
	}

	/* 現在のテープ出力状態を保存 */
	bTapeFlag2 = bTapeFlag;
}

/*
 *	WAVデータ合成 (FDD/CMT)
 */
#if defined(FDDSND)
static void FASTCALL WaveSnd(int32 *buf, int samples)
{
	int i;
	int j;
	int dat;

	/* サンプル書き込み */
	for (i=0; i<samples; i++) {
		for (j=0; j<SNDBUF; j++) {
			if (WavP[j].bPlay) {
				dat = Wav[WavP[j].dwWaveNo].p[WavP[j].dwCount1];
				dat = (short)(((int)dat * nWaveLevel) >> 16);
				*buf += (int)dat;
				if (uChannels == 2) {
					*(buf+1) += (int)dat;
				}

				/* カウントアップ */
				WavP[j].dwCount2 += (Wav[WavP[j].dwWaveNo].freq << 16) / uRate;
				if (WavP[j].dwCount2 > 0x10000) {
					WavP[j].dwCount1 += (WavP[j].dwCount2 >> 16);
					WavP[j].dwCount2 &= 0xFFFF;
					if (WavP[j].dwCount1 >= Wav[WavP[j].dwWaveNo].size) {
						WavP[j].bPlay = FALSE;
					}
				}
				WavP[j].dwCount3 ++;
			}
		}
		buf ++;
		if (uChannels == 2) {
			buf ++;
		}
	}
}
#endif

/*
 *	ジャストサウンド DACデータ合成
 */
#if defined(LPRINT) && defined(JASTSOUND)
static void FASTCALL DACSnd(int32 *buf, int samples)
{
	int i;
	DWORD ptr;
	int ratio;
	short dat;
	short dat1, dat2;

	if (!samples || (lp_use != LP_JASTSOUND)) {
		return;
	}

	/* サンプル書き込み */
	ratio = (dwDACptr << 16) / samples;
	ptr = 0;
	for (i=0; i<samples; i++) {
		dat1 = (short)pDACbuf[(ptr >> 16)];
		dat2 = (short)pDACbuf[(ptr >> 16) + 1];
		dat = (short)(((dat1 + (((dat2 - dat1) * (ptr & 0xffff)) >> 16)) - 0x80) << 8);
		ptr += ratio;

		*buf++ += (int)((dat * nWaveLevel) >> 16);
		if (uChannels == 2) {
			*buf++ += (int)((dat * nWaveLevel) >> 16);
		}
	}

	pDACbuf[0] = pDACbuf[dwDACptr];
	dwDACptr = 0;
}
#endif

/*
 *	波形合成
 */
static void FASTCALL MixingSound(DWORD *q, int samples, BOOL bZero)
{
	memset(q, 0, sizeof(DWORD) * samples * uChannels);
	if (!bZero) {
		if (uChannels == 1) {
			/* モノラル */
			if (opn_use) {
#if defined(ROMEO)
				if ((pOPN[OPN_STD]) && !bUseRomeo) {
#else
				if (pOPN[OPN_STD]) {
#endif
					pOPN[OPN_STD]->Mix((int32*)q, samples);
				}
			}
#if XM7_VER == 1
			if (fmx_flag || whg_use) {
#if defined(ROMEO)
				if (bUseRomeo || fmx_flag) {
					pOPN[OPN_WHG]->psg.Mix((int32*)q, samples);
				}
				else {
					pOPN[OPN_WHG]->Mix((int32*)q, samples);
				}
#else
				pOPN[OPN_WHG]->Mix((int32*)q, samples);
#endif
			}
#else
			if (whg_use) {
#if defined(ROMEO)
				if (bUseRomeo) {
					pOPN[OPN_WHG]->psg.Mix((int32*)q, samples);
				}
				else {
					pOPN[OPN_WHG]->Mix((int32*)q, samples);
				}
#else
				pOPN[OPN_WHG]->Mix((int32*)q, samples);
#endif
			}
#endif
			if (thg_use) {
				pOPN[OPN_THG]->Mix((int32*)q, samples);
			}
			else if (fm7_ver == 1) {
				pOPN[OPN_THG]->psg.Mix((int32*)q, samples);
			}
		}
		else {
			/* ステレオ */
			if (!whg_use && !thg_use) {
				/* WHG/THGを使用していない(強制モノラル) */
				if (opn_use) {
#if defined(ROMEO)
					if (!bUseRomeo) {
						pOPN[OPN_STD]->Mix2((int32*)q, samples, 16, 16);
					}
#else
					pOPN[OPN_STD]->Mix2((int32*)q, samples, 16, 16);
#endif
				}
#if XM7_VER == 1
				if (fmx_flag) {
					pOPN[OPN_WHG]->psg.Mix2((int32*)q, samples, 16, 16);
				}
#endif
				if (fm7_ver == 1) {
					pOPN[OPN_THG]->psg.Mix2((int32*)q, samples, 16, 16);
				}
			}
			else {
				/* WHGまたはTHGを使用中 */
				if (opn_use) {
#if defined(ROMEO)
					if (!bUseRomeo) {
						pOPN[OPN_STD]->Mix2((int32*)q, samples,
							l_vol[OPN_STD][uStereo], r_vol[OPN_STD][uStereo]);
					}
#else
					pOPN[OPN_STD]->Mix2((int32*)q, samples,
						l_vol[OPN_STD][uStereo], r_vol[OPN_STD][uStereo]);
#endif
				}
				if (whg_use) {
#if defined(ROMEO)
					if (bUseRomeo) {
						pOPN[OPN_WHG]->psg.Mix2((int32*)q, samples,
							l_vol[OPN_WHG][uStereo], r_vol[OPN_WHG][uStereo]);
					}
					else {
						pOPN[OPN_WHG]->Mix2((int32*)q, samples,
							l_vol[OPN_WHG][uStereo], r_vol[OPN_WHG][uStereo]);
					}
#else
					pOPN[OPN_WHG]->Mix2((int32*)q, samples,
						l_vol[OPN_WHG][uStereo], r_vol[OPN_WHG][uStereo]);
#endif
				}
				if (thg_use) {
					pOPN[OPN_THG]->Mix2((int32*)q, samples,
						l_vol[OPN_THG][uStereo], r_vol[OPN_THG][uStereo]);
				}
				else if (fm7_ver == 1) {
					pOPN[OPN_THG]->psg.Mix2((int32*)q, samples, 16, 16);
				}
			}
		}

		/* テープ */
		TapeSnd((int32*)q, samples);

		/* ビープ */
		BeepSnd((int32*)q, samples);
#if XM7_VER >= 2
		KeyEncBeepSnd((int32*)q, samples);
#endif

#if defined(FDDSND)
		/* WAVサウンド */
		WaveSnd((int32*)q, samples);
#endif

#if defined(LPRINT) && defined(JASTSOUND)
		/* ジャストサウンド */
		if (lp_use == LP_JASTSOUND) {
			DACSnd((int32*)q, samples);
		}
#endif
	}
}

#if !defined(FMGEN_DIRECT_OUTPUT)
/*
 *	デキュー
 */
static void FASTCALL FMReg_Dequeue(int queue)
{
	int ptr;

	if (queue & 1) {
		for (ptr = 0; ptr < uOPNqueuePtr; ptr++) {
			pOPN[OPN_STD]->SetReg((uint8)OPNqueue[ptr].reg,
				(uint8)OPNqueue[ptr].dat);
		}
		uOPNqueuePtr = 0;
	}

	if (queue & 2) {
		for (ptr = 0; ptr < uWHGqueuePtr; ptr++) {
			pOPN[OPN_WHG]->SetReg((uint8)WHGqueue[ptr].reg,
				(uint8)WHGqueue[ptr].dat);
		}
		uWHGqueuePtr = 0;
	}

	if (queue & 4) {
		for (ptr = 0; ptr < uTHGqueuePtr; ptr++) {
			pOPN[OPN_THG]->SetReg((uint8)THGqueue[ptr].reg,
				(uint8)THGqueue[ptr].dat);
		}
		uTHGqueuePtr = 0;
	}
}
#endif

/*
 *	サウンド作成バッファへ追加
 */
static void FASTCALL AddSnd(BOOL bFill, BOOL bZero)
{
	int i;
	int samples;

	/* OPNデバイスが作成されていなければ、何もしない */
	if (!pOPN[OPN_STD] || !pOPN[OPN_WHG] || !pOPN[OPN_THG]) {
		return;
	}

	/* bFillの場合のサンプル数 */
	/* (モノラル2byte/sample・ステレオ4byte/sample) */
	samples = (uBufSize / uChannels) >> 2;
	samples -= uSample;

	/* !bFillなら、時間から計測 */
	if (!bFill) {
		/* 時間経過から求めた理論サンプル数 */
		/* 計算結果がオーバーフローする問題に対策 2002/11/25 */
		i = (uRate / 25);
		i *= dwSoundTotal;
		i /= 40000;

		/* uSampleと比較、一致していれば何もしない */
		if (i <= (int)uSample) {
			return;
		}

		/* uSampleとの差が今回生成するサンプル数 */
		i -= (int)(uSample);

		/* samplesよりも小さければ合格 */
		if (i <= samples) {
			samples = i;
		}
	}

	/* キューからfmgenへのデータ送り込み */
#if !defined(FMGEN_DIRECT_OUTPUT)
	FMReg_Dequeue(0x07);
#endif

	/* ミキシング */
	MixingSound(&lpsbuf[uSample * uChannels], samples, bZero);

	/* 更新 */
	if (bFill) {
		dwSoundTotal = 0;
		uSample = 0;
	}
	else {
		uSample += samples;
	}
}

/*
 *	WAVキャプチャ処理
 */
static void FASTCALL WavCapture(void)
{
	UINT nSize;
	DWORD *p;
	WORD *q;
	int j;

	/* WAVキャプチャ中でなければ、リターン */
	if (hWavCapture < 0) {
		return;
	}
	ASSERT(pWavCapture);

	/* ポインタ、サイズを仮決め(nSizeはWORD変換後のBYTE値) */
	p = lpsbuf;
	nSize = uBufSize / 2;

	/* bWavCaptureがFALSEなら */
	if (!bWavCapture) {
		/* 頭出しチェック */
		while (nSize > 0) {
			if (uChannels == 1) {
				if (*p != 0) {
					break;
				}
				else {
					nSize -= 2;
					p++;
				}
			}
			else {
				if ((p[0] != 0) || (p[1] != 0)) {
					break;
				}
				else {
					nSize -= 4;
					p += 2;
				}
			}
		}
		/* 判定 */
		if (nSize == 0) {
			return;
		}
	}

	/* nWavCaptureを考慮 */
	if ((nWavCapture + nSize) >= 0x8000) {
		/* 32KBいっぱいまでコピー */
		j = (0x8000 - nWavCapture) >> 1;
		q = &pWavCapture[nWavCapture >> 1];
		CopySoundBuffer(p, q, j);
		p += j;

		/* 残りサイズを更新 */
		nSize -= (0x8000 - nWavCapture);

		/* 書き込み */
		file_write(hWavCapture, (BYTE*)pWavCapture, 0x8000);
		dwWavCapture += 0x8000;
		nWavCapture = 0;
	}

	/* 余りをコピー */
	j = nSize >> 1;
	q = &pWavCapture[nWavCapture >> 1];
	CopySoundBuffer(p, q, j);
	nWavCapture += nSize;

	/* 正式な録音状態 */
	bWavCapture = TRUE;
}

/*
 *	定期処理
 */
BOOL FASTCALL ProcessSnd(BOOL bZero)
{
	DWORD dwPlayC, dwWriteC;
	BOOL bWrite;
	DWORD dwOffset;
	WORD *ptr1, *ptr2;
	DWORD size1, size2;
	DWORD *p;
	int i;
	int j;
	HRESULT hr;

	/* ろみおコマンド発行 by うさ */
#if defined(ROMEO)
	if (bUseRomeo) {
		juliet_YMF288EXEC((DWORD)uTick);
	}
#endif

	/* MIDIコマンド発行 */
#if defined(MIDI)
	if (bMidiDelayMode) {
		MidiSendData((DWORD)uTick);
	}
	else {
		MidiSendData((DWORD)nMidiDelay);
	}
#endif

	/* 初期化されていなければ、何もしない */
	if (!lpdsb) {
		return TRUE;
	}

	/* 書き込み位置を得る */
	if (FAILED(lpdsb->GetCurrentPosition(&dwPlayC, &dwWriteC))) {
		return FALSE;
	}

	/* 書き込み位置とバンクから、必要性を判断 */
	bWrite = FALSE;
	if (bNowBank) {
		if (dwPlayC >= (uBufSize / 2)) {
			dwOffset = 0;
			bWrite = TRUE;
		}
	}
	else {
		if (dwPlayC < (uBufSize / 2)) {
			dwOffset = uBufSize / 2;
			bWrite = TRUE;
		}
	}

	/* 書き込む必要がなければ、リターン */
	if (!bWrite) {
		/* テープ */
		if (tape_motor && bTapeMon) {
			bWrite = TRUE;
		}
		/* BEEP */
		if (beep_flag && speaker_flag) {
			bWrite = TRUE;
		}

		/* どちらかがONなら、バッファ充填 */
		if (bWrite) {
			AddSnd(FALSE, bZero);
		}

		return FALSE;
	}

	/* 書き込み。まずサウンド作成バッファを全部埋める */
	AddSnd(TRUE, bZero);

	/* 次いでロック */
	hr = lpdsb->Lock(dwOffset, uBufSize / 2, (void **)&ptr1, &size1,
						(void**)&ptr2, &size2, 0);

	/* バッファが失われていれば、リストア */
	if (hr == DSERR_BUFFERLOST) {
		lpdsb->Restore();
		hr = lpdsb->Lock(dwOffset, uBufSize / 2, (void **)&ptr1, &size1,
							(void**)&ptr2, &size2, 0);
	}
	/* ロック成功しなければ、続けても意味がない */
	if (FAILED(hr)) {
		return FALSE;
	}

	/* サウンド作成バッファ→セカンダリバッファ */
	p = lpsbuf;
	i = (int)(size1 / 2);
	j = (int)(size2 / 2);
	CopySoundBuffer(p, ptr1, i);
	CopySoundBuffer(p + i, ptr2, j);

	/* アンロック */
	lpdsb->Unlock(ptr1, size1, ptr2, size2);

	/* バンク反転 */
	bNowBank = (!bNowBank);

	/* WAVキャプチャ処理 */
	WavCapture();

	return TRUE;
}

/*
 *	サウンド出力完了待機
 */
void FASTCALL WaitSnd(void)
{
	DWORD dwPlayC, dwWriteC;
	DWORD dwLength;

	while (TRUE) {
		/* 初期化されていなければ、何もしない */
		if (!lpdsb) {
			return;
		}

		/* 書き込み位置を得る */
		if (FAILED(lpdsb->GetCurrentPosition(&dwPlayC, &dwWriteC))) {
			break;
		}

		/* 書き込み位置とバンクから、必要性を判断 */
		if (bNowBank) {
			if (dwPlayC >= (uBufSize / 2)) {
				break;
			}
			dwLength = (uBufSize / 2);
		}
		else {
			if (dwPlayC < (uBufSize / 2)) {
				break;
			}
			dwLength = uBufSize;
		}

		if ((dwLength - dwPlayC) * 1000 > uRate) {
			Sleep(1);
		}
	}
}

/*
 *	レベル取得
 */
int FASTCALL GetLevelSnd(int ch)
{
	FM::OPN *p;
	int i;
	double s;
	double t;
	int *buf;

	ASSERT((ch >= 0) && (ch < 18));

	/* OPN */
	if (ch < 6) {
		p = pOPN[OPN_STD];
#if defined(ROMEO)
		/* ろみお使用時は取得できないので0 */
		if (bUseRomeo) {
			return 0;
		}
#endif

		/* 実際に使われていなければ0 */
		if ((!opn_enable || !opn_use) && (fm7_ver == 1)) {
			return 0;
		}
	}

	/* WHG */
	if ((ch >= 6) && (ch < 12)) {
		p = pOPN[OPN_WHG];
		ch -= 6;

		/* 実際に使われていなければ0 */
		if (!whg_enable || !whg_use) {
			return 0;
		}

#if defined(ROMEO)
		/* ろみお使用時はFM音源部は取得できないので0 */
		if (bUseRomeo && (ch < 3)) {
			return 0;
		}
#endif
	}

	/* THG */
	if ((ch >= 12) && (ch < 18)) {
		p = pOPN[OPN_THG];
		ch -= 12;

		/* 実際に使われていなければ0 */
		if ((!thg_enable || !thg_use) && (fm7_ver != 1)) {
			return 0;
		}
	}

	/* 存在チェック */
	if (!p) {
		return 0;
	}

	/* FM,PSGの区別 */
	if (ch < 3) {
		/* FM:512サンプルの2乗和を計算 */
		buf = p->rbuf[ch];

		s = 0;
		for (i=0; i<512; i++) {
			t = (double)*buf++;
			t *= t;
			s += t;
		}
		s /= 512;

		/* ゼロチェック */
		if (s == 0) {
			return 0;
		}

		/* log10を取る */
		s = log10(s);

		/* FM音源補正 */
		s *= 40.0;
	}
	else {
		/* PSG:512サンプルの2乗和を計算 */
		buf = p->psg.rbuf[ch - 3];

		s = 0;
		for (i=0; i<512; i++) {
			t = (double)*buf++;
			t *= t;
			s += t;
		}
		s /= 512;

		/* ゼロチェック */
		if (s == 0) {
			return 0;
		}

		/* log10を取る */
		s = log10(s);

		/* PSG音源補正 */
		s *= 60.0;
	}

	return (int)s;
}

/*
 *	WAVキャプチャ開始
 */
void FASTCALL OpenCaptureSnd(char *fname)
{
	WAVEFORMATEX wfex;
	DWORD dwSize;
	int fileh;

	ASSERT(fname);
	ASSERT(hWavCapture < 0);
	ASSERT(!bWavCapture);

	/* 合成中でなければ、リターン */
	if (!pOPN[OPN_STD] || !pOPN[OPN_WHG] || !pOPN[OPN_THG]) {
		return;
	}

	/* バッファが無ければ、リターン */
	if (!pWavCapture) {
		return;
	}

	/* uBufSize / 2が0x8000以下でないとエラー */
	if ((uBufSize / 2) > 0x8000) {
		return;
	}

	/* ファイルオープン(書き込みモード) */
	fileh = file_open(fname, OPEN_W);
	if (fileh < 0) {
		return;
	}

	/* RIFFヘッダ書き込み */
	if (!file_write(fileh, (BYTE*)"RIFF滅殺WAVEfmt ", 16)) {
		file_close(fileh);
		return;
	}

	/* WAVEFORMATEX書き込み */
	dwSize = sizeof(wfex);
	if (!file_write(fileh, (BYTE*)&dwSize, sizeof(dwSize))) {
		file_close(fileh);
		return;
	}
	memset(&wfex, 0, sizeof(wfex));
	wfex.cbSize = sizeof(wfex);
	wfex.wFormatTag = WAVE_FORMAT_PCM;
	wfex.nChannels = uChannels;
	wfex.nSamplesPerSec = uRate;
	wfex.nBlockAlign = (WORD)(2 * uChannels);
	wfex.nAvgBytesPerSec = wfex.nSamplesPerSec * wfex.nBlockAlign;
	wfex.wBitsPerSample = 16;
	if (!file_write(fileh, (BYTE *)&wfex, sizeof(wfex))) {
		file_close(fileh);
		return;
	}

	/* dataサブヘッダ書き込み */
	if (!file_write(fileh, (BYTE *)"data滅殺", 8)) {
		file_close(fileh);
		return;
	}

	/* ok */
	nWavCapture = 0;
	dwWavCapture = 0;
	bWavCapture = FALSE;
	hWavCapture = fileh;
}

/*
 *	WAVキャプチャ終了
 */
void FASTCALL CloseCaptureSnd(void)
{
	DWORD dwLength;

	ASSERT(hWavCapture >= 0);

	/* バッファに残った分を書き込み */
	file_write(hWavCapture, (BYTE*)pWavCapture, nWavCapture);
	dwWavCapture += nWavCapture;
	nWavCapture = 0;

	/* ファイルレングスを書き込む */
	file_seek(hWavCapture, 4);
	dwLength = dwWavCapture + sizeof(WAVEFORMATEX) + 20;
	file_write(hWavCapture, (BYTE *)&dwLength, sizeof(dwLength));

	/* data部レングスを書き込む */
	file_seek(hWavCapture, sizeof(WAVEFORMATEX) + 24);
	file_write(hWavCapture, (BYTE *)&dwWavCapture, sizeof(dwWavCapture));

	/* ファイルクローズ */
	file_close(hWavCapture);

	/* ワークエリアクリア */
	hWavCapture = -1;
	bWavCapture = FALSE;
}

/*
 *	ROMEO ミュート処理
 */
#if defined(ROMEO)
extern "C" {
void FASTCALL ROMEO_Mute(BOOL flag)
{
	int i;

	/* 状態が変化した場合のみ処理を行う */
	if (bRomeo && (bRomeoMute != flag)) {
		if (flag) {
			/* ミュート解除 */
			juliet_YMF288Mute(FALSE);

			/* PSG */
			for (i=0; i<16; i++) {
				juliet_YMF288A_B((BYTE)i, opn_reg[OPN_STD][i]);
			}

			/* FM音源キーオフ */
			for (i=0; i<3; i++) {
				juliet_YMF288A_B(0x28, (BYTE)i);
				juliet_YMF288A_B(0x28, (BYTE)(i | 0x04));
			}

			/* FM音源レジスタ */
			for (i=0x30; i<0xb4; i++) {
				juliet_YMF288A_B((BYTE)i, opn_reg[OPN_STD][i]);
				juliet_YMF288B_B((BYTE)i, opn_reg[OPN_WHG][i]);
			}

			/* FM音源動作モード */
			juliet_YMF288A_B((BYTE)0x27, (BYTE)(opn_reg[OPN_STD][0x27] & 0xc0));
		}
		else {
			/* ミュート設定 */
			juliet_YMF288Mute(TRUE);
		}

		/* フラグ保存 */
		bRomeoMute = flag;
	}
}
}
#endif

/*
 *	ROMEO プリスケーラ変更時用レジスタ再設定
 */
#if defined(ROMEO)
static void FASTCALL ROMEO_ChangePrescaler(void)
{
	int i;

	/* PSG */
	for (i=0; i<13; i++) {
		if ((i < 7) || (i > 10)) {
			juliet_YMF288A_B((BYTE)i, opn_reg[OPN_STD][i]);
		}
	}

	/* FM音源 */
	for (i=0xa0; i<0xb0; i++) {
		juliet_YMF288A_B((BYTE)i, opn_reg[OPN_STD][i]);
		juliet_YMF288B_B((BYTE)i, opn_reg[OPN_WHG][i]);
	}
}
#endif

/*
 *	ミュート状態取得
 */
extern "C" {
BOOL FASTCALL GetMute(int ch)
{
	return (BOOL)(nChannelMask[ch / 6] & nChannelMaskBit[ch % 6]);
}
}

/*
 *	ミュート状態設定
 */
extern "C" {
void FASTCALL SetMute(int ch, BOOL mute)
{
	/* フラグの設定 */
	if (mute) {
		nChannelMask[ch / 6] |= (WORD)nChannelMaskBit[ch % 6];
	}
	else {
		nChannelMask[ch / 6] &= (WORD)~nChannelMaskBit[ch % 6];
	}

	/* fmgenに対してミュート発行 */
	if (pOPN[ch / 6]) {
		pOPN[ch / 6]->SetChannelMask(nChannelMask[ch / 6]);
	}
}
}

/*
 *	OPN出力
 */
extern "C" {
void FASTCALL opn_notify(BYTE reg, BYTE dat)
{
	/* OPNがなければ、何もしない */
	if (!pOPN[OPN_STD]) {
		return;
	}

	/* プリスケーラを調整 */
	if (opn_scale[OPN_STD] != nScale[OPN_STD]) {
		nScale[OPN_STD] = opn_scale[OPN_STD];
#if defined(ROMEO)
		if (bUseRomeo) {
			ROMEO_ChangePrescaler();
		}
#endif

		switch (opn_scale[OPN_STD]) {
			case 2:
				pOPN[OPN_STD]->SetReg(0x2f, 0);
				break;
			case 3:
				pOPN[OPN_STD]->SetReg(0x2e, 0);
				break;
			case 6:
				pOPN[OPN_STD]->SetReg(0x2d, 0);
				break;
		}
	}

	/* Ch3動作モードチェック */
	if (reg == 0x27) {
		if (uCh3Mode[OPN_STD] == dat) {
			return;
		}
		uCh3Mode[OPN_STD] = dat;
	}

	/* 0xffレジスタはチェック */
	if (reg == 0xff) {
		if ((opn_reg[OPN_STD][0x27] & 0xc0) != 0x80) {
			return;
		}
	}

	/* サウンド合成 */
	AddSnd(FALSE, FALSE);

	/* 出力・キュー登録 */
#if defined(FMGEN_DIRECT_OUTPUT)
	pOPN[OPN_STD]->SetReg((uint8)reg, (uint8)dat);
#else
	if (uOPNqueuePtr >= FMQUEUE_SIZE) {
		FMReg_Dequeue(0x01);
	}
	OPNqueue[uOPNqueuePtr].reg = reg;
	OPNqueue[uOPNqueuePtr].dat = dat;
	uOPNqueuePtr ++;
#endif

	/* ろみお出力 */
#if defined(ROMEO)
	if (bUseRomeo) {
		juliet_YMF288A_B(reg, dat);
	}
#endif
}
}

/*
 *	WHG出力
 */
extern "C" {
void FASTCALL whg_notify(BYTE reg, BYTE dat)
{
	/* WHGがなければ、何もしない */
	if (!pOPN[OPN_WHG]) {
		return;
	}

	/* プリスケーラを調整 */
	if (opn_scale[OPN_WHG] != nScale[OPN_WHG]) {
		nScale[OPN_WHG] = opn_scale[OPN_WHG];
#if defined(ROMEO)
		if (bUseRomeo) {
			ROMEO_ChangePrescaler();
		}
#endif

		switch (opn_scale[OPN_WHG]) {
			case 2:
				pOPN[OPN_WHG]->SetReg(0x2f, 0);
				break;
			case 3:
				pOPN[OPN_WHG]->SetReg(0x2e, 0);
				break;
			case 6:
				pOPN[OPN_WHG]->SetReg(0x2d, 0);
				break;
		}
	}

	/* Ch3動作モードチェック */
	if (reg == 0x27) {
		if (uCh3Mode[OPN_WHG] == dat) {
			return;
		}
		uCh3Mode[OPN_WHG] = dat;
	}

	/* 0xffレジスタはチェック */
	if (reg == 0xff) {
		if ((opn_reg[OPN_WHG][0x27] & 0xc0) != 0x80) {
			return;
		}
	}

	/* サウンド合成 */
	AddSnd(FALSE, FALSE);

	/* 出力・キュー登録 */
#if defined(FMGEN_DIRECT_OUTPUT)
	pOPN[OPN_WHG]->SetReg((uint8)reg, (uint8)dat);
#else
	if (uWHGqueuePtr >= FMQUEUE_SIZE) {
		FMReg_Dequeue(0x02);
	}
	WHGqueue[uWHGqueuePtr].reg = reg;
	WHGqueue[uWHGqueuePtr].dat = dat;
	uWHGqueuePtr ++;
#endif

	/* ろみお出力 */
#if defined(ROMEO)
	if (bUseRomeo && ((reg >= 0x20) && (reg <= 0xfe))) {
		juliet_YMF288B_B(reg, dat);
	}
#endif
}
}

/*
 *	THG出力
 */
extern "C" {
void FASTCALL thg_notify(BYTE reg, BYTE dat)
{
	/* THGがなければ、何もしない */
	if (!pOPN[OPN_THG]) {
		return;
	}

	/* エンベロープモード設定 */
	if (thg_use != bTHGUse) {
		pOPN[OPN_THG]->psg.SetSSGEnvMode(thg_use);
		bTHGUse = thg_use;
	}

	/* プリスケーラを調整 */
	if (opn_scale[OPN_THG] != nScale[OPN_THG]) {
		nScale[OPN_THG] = opn_scale[OPN_THG];

		switch (opn_scale[OPN_THG]) {
			case 2:
				pOPN[OPN_THG]->SetReg(0x2f, 0);
				break;
			case 3:
				pOPN[OPN_THG]->SetReg(0x2e, 0);
				break;
			case 6:
				pOPN[OPN_THG]->SetReg(0x2d, 0);
				break;
		}
	}

	/* Ch3動作モードチェック */
	if (reg == 0x27) {
		if (uCh3Mode[OPN_THG] == dat) {
			return;
		}
		uCh3Mode[OPN_THG] = dat;
	}

	/* 0xffレジスタはチェック */
	if (reg == 0xff) {
		if ((opn_reg[OPN_THG][0x27] & 0xc0) != 0x80) {
			return;
		}
	}

	/* サウンド合成 */
	AddSnd(FALSE, FALSE);

	/* 出力・キュー登録 */
#if defined(FMGEN_DIRECT_OUTPUT)
	pOPN[OPN_THG]->SetReg((uint8)reg, (uint8)dat);
#else
	if (uTHGqueuePtr >= FMQUEUE_SIZE) {
		FMReg_Dequeue(0x04);
	}
	THGqueue[uTHGqueuePtr].reg = reg;
	THGqueue[uTHGqueuePtr].dat = dat;
	uTHGqueuePtr ++;
#endif
}
}

/*
 *	BEEP出力
 */
extern "C" {
void FASTCALL beep_notify(void)
{
	/* 出力状態が変化していなければリターン */
	if (!((beep_flag & speaker_flag) ^ bBeepFlag)) {
		return;
	}

	/* サウンド合成 */
	AddSnd(FALSE, FALSE);

	/* フラグ保持 */
	if (beep_flag && speaker_flag) {
		bBeepFlag = TRUE;
	}
	else {
		bBeepFlag = FALSE;
	}
}
}

#if XM7_VER >= 2
/*
 *	キーエンコーダBEEP出力
 */
extern "C" {
void FASTCALL keyencbeep_notify(void)
{
	/* 出力状態が変化していなければリターン */
	if (!(keyenc_beep_flag ^ bKeyEncBeepFlag)) {
		return;
	}

	/* サウンド合成 */
	AddSnd(FALSE, FALSE);

	/* フラグ保持 */
	if (keyenc_beep_flag) {
		bKeyEncBeepFlag = TRUE;
	}
	else {
		bKeyEncBeepFlag = FALSE;
	}
}
}
#endif

/*
 *	テープ出力
 */
extern "C" {
void FASTCALL tape_notify(BOOL flag)
{
	/* 出力状態が変化したかチェック */
	if (bTapeFlag == flag) {
		return;
	}

	if (bTapeMon) {
		/* サウンド合成 */
		AddSnd(FALSE, FALSE);
	}

	/* フラグ保持 */
	bTapeFlag = flag;
}
}

/*
 *	WAV出力
 */
#if defined(FDDSND)
extern "C" {
void FASTCALL wav_notify(BYTE no)
{
	int i;
	int j;
	DWORD k;

	/* サウンド合成 */
	AddSnd(FALSE, FALSE);

	if (no == SOUND_STOP) {
		/* 停止 */
		for (i=0; i<SNDBUF; i++) {
			WavP[i].bPlay = FALSE;
		}
	}
	else {
		if (Wav[no].freq) {
			j = 0;
			k = 0;
			for (i=0; i<SNDBUF; i++) {
				/* 再生停止中のチャンネルを検索 */
				if (!WavP[i].bPlay) {
					j = i;
					break;
				}
				else {
					/* 一番最初に再生が開始されたチャンネルを検索 */
					if (k < WavP[i].dwCount3) {
						k = WavP[i].dwCount3;
						j = i;
					}
				}
			}

			/* データセット */
			WavP[j].dwWaveNo	= no;
			WavP[j].dwCount1	= 0;
			WavP[j].dwCount2	= 0;
			WavP[j].dwCount3	= 0;
			WavP[j].bPlay		= TRUE;
		}
	}
}
}
#endif

/*
 *	ジャストサウンド DACデータ合成
 */
#if defined(LPRINT) && defined(JASTSOUND)
extern "C" {
void FASTCALL dac_notify(BYTE dat)
{
	if (dwDACptr < ((uRate * uTick) / 100)) {
		pDACbuf[++dwDACptr] = dat;
	}
}
}
#endif

#endif	/* _WIN32 */
