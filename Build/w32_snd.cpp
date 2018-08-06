/*
 *	FM-7 EMULATOR "XM7"
 *
 *	Copyright (C) 1999-2017 �o�h�D(Twitter:@xm6_original)
 *	Copyright (C) 2001-2017 Ryu Takegami (Twitter:@RyuTakegami)
 *							ROMEO support by usalin
 *
 *	[ Win32API �T�E���h ]
 *
 *	RHG����
 *	  2001.12.09		�����X�e���I���[�h��ǉ�
 *	  2001.12.25		���������[�`���Q����{��(�o�h�D)
 *						OPN/WHG/THG���W�X�^�֏������݂��s�����O�ɂ���܂Ōo��
 *						�������ԕ��������𐶐�����悤�ɂ���(�o�h�D)
 *	  2001.12.26		�T�E���h�o�b�t�@��Z������ƕs�������ŗ���������C��
 *						�����������[�h������ɓ��삵�Ȃ������C��
 *	  2001.12.27		�e�[�v�����j�^�̏�����ύX
 *						BEEP����FM�����Ɠ��l�̃^�C�~���O�Ő�������悤�ɕύX
 *	  2002.01.23		�T�E���h�o�b�t�@���ύX���̏��������P(�o�h�D)
 *	  2002.06.15		FM-7���[�h���̓Ɨ�PSG�G�~�����[�V�����Ή��ɔ�����THG�g
 *						�p�t���O�̔����ύX
 *	  2002.08.31		����DirectSound�g�p�A�v���P�[�V�����Ƃ̓��������ɑΉ�
 *	  2002.09.21		S98���O�o�͂ɑΉ�
 *	  2002.11.11		FDD�V�[�N���ECMT�����[���o�͗p��WAV����������ǉ�
 *	  2002.11.25		�T�E���h�o�b�t�@�T�C�Y��傫������Ɖ����r�؂����
 *						���C��
 *	  2003.01.28		FDD�V�[�N���̍ő哯����������20�ɕύX
 *	  2003.02.26		Oh!FM�f�ڂ̃v�����^�|�[�g�ڑ��^�C�vD/A�R���o�[�^(�W��
 *						���g�������h!?)�ɑΉ�
 *	  2003.05.15		�Vfmgen�g�p����OPN/WHG/THG�̓Ɨ��v���X�P�[���ݒ�ɑΉ�
 *	  2004.03.17		�T�E���h�o�b�t�@�I�[�o�[�t���[���̏�����ύX
 *	  2004.08.13		OPN/WHG/THG��opn.c�ւ̓����ɍ��킹���ύX�������Ȃ�
 *	  2007.12.16		�p�[�g�~���[�g�@�\�����ւ̏������J�n
 *	  2010.01.23		�e�����̃{�����[�������E�`�����l���Z�p���[�V�����ݒ�
 *						�@�\��ǉ�
 *	  2016.07.23		FM����/PSG�̃��W�X�^�������݂���������L���[�ɂ��ߍ���
 *						���z�}�V���̎��s���Ԃɍ��킹�č������s���悤�Ɏd�l�ύX
 */

#ifdef _WIN32

#define STRICT
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <mmsystem.h>
#define DIRECTSOUND_VERSION		0x300	/* DirectX3���w�� */
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
 *	�O���[�o�� ���[�N
 */
UINT nSampleRate;						/* �T���v�����O���[�g */
UINT nSoundBuffer;						/* �T�E���h�o�b�t�@�T�C�Y */
UINT uStereoOut;						/* �o�̓��[�h */
BOOL bInterpolation;					/* �T�E���h��ԃ��[�h */
BOOL bForceStereo;						/* �����X�e���I�o�� */
UINT nBeepFreq;							/* BEEP���g�� */
BOOL bTapeMon;							/* �e�[�v�����j�^ */
int hWavCapture;						/* WAV�L���v�`���n���h�� */
BOOL bWavCapture;						/* WAV�L���v�`���J�n */
#if defined(ROMEO)
BOOL bUseRomeo;							/* ��݂��g�p�t���O */
#endif
DWORD dwSoundTotal;						/* �T�E���h�g�[�^������ */
WORD nChannelMask[3];					/* �`�����l���}�X�N */
UINT uChSeparation;						/* �`�����l���Z�p���[�V���� */
int nFMVolume;							/* FM�����{�����[�� */
int nPSGVolume;							/* PSG�{�����[�� */
int nBeepVolume;						/* BEEP���{�����[�� */
int nCMTVolume;							/* CMT�����j�^�{�����[�� */
#if defined(FDDSND)
int nWaveVolume;						/* �e����ʉ��{�����[�� */
#endif

/*
 *	�X�^�e�B�b�N ���[�N
 */
static LPDIRECTSOUND lpds;				/* DirectSound */
static LPDIRECTSOUNDBUFFER lpdsp;		/* DirectSoundBuffer(�v���C�}��) */
static LPDIRECTSOUNDBUFFER lpdsb;		/* DirectSoundBuffer(�Z�J���_��) */
static DWORD *lpsbuf;					/* �T�E���h�쐬�o�b�t�@ */
static BOOL bNowBank;					/* ���ݍĐ����̃o���N */
static UINT uBufSize;					/* �T�E���h�o�b�t�@�T�C�Y */
static UINT uRate;						/* �������[�g */
static UINT uTick;						/* ���o�b�t�@�T�C�Y�̒��� */
static BOOL bMode;						/* �T�E���h��ԃ��[�h */
static UINT uStereo;					/* �o�̓��[�h */
static UINT uSample;					/* �T���v���J�E���^ */
static UINT uBeep;						/* BEEP�g�`�J�E���^ */
#if XM7_VER >= 2
static UINT uKeyEncBeep;				/* �L�[�G���R�[�_BEEP�g�`�J�E���^ */
#endif
static FM::OPN *pOPN[3];				/* OPN�f�o�C�X */
static int nScale[3];					/* OPN�v���X�P�[�� */
static BYTE uCh3Mode[3];				/* OPN Ch.3���[�h */
static BOOL bInitFlag;					/* �������t���O */
static WORD *pWavCapture;				/* �L���v�`���o�b�t�@(64KB) */
static UINT nWavCapture;				/* �L���v�`���o�b�t�@���f�[�^ */
static DWORD dwWavCapture;				/* �L���v�`���t�@�C���T�C�Y */
static WORD uChannels;					/* �o�̓`�����l���� */
static BOOL bBeepFlag;					/* BEEP�o�� */
#if XM7_VER >= 2
static BOOL bKeyEncBeepFlag;			/* �L�[�G���R�[�_BEEP�o�� */
#endif
static BOOL bTHGUse;					/* THG�g�p�t���O(�ۑ��p) */
static BOOL bPartMute[3][6];			/* �p�[�g�~���[�g�t���O */
static int nBeepLevel;					/* BEEP���o�̓��x�� */
static int nCMTLevel;					/* CMT�����j�^�o�̓��x�� */
#if defined(FDDSND)
static int nWaveLevel;					/* �e����ʉ��o�̓��x�� */
#endif
static BOOL bTapeFlag;					/* ���݂̃e�[�v�o�͏�� */
static BOOL bTapeFlag2;					/* �O��̃e�[�v�o�͏�� */
static BYTE uTapeDelta;					/* �e�[�v�g�`��ԃJ�E���^ */

#if defined(ROMEO)
static BOOL bRomeoMute;					/* ��݂��~���[�g�t���O */
#endif

#if defined(LPRINT) && defined(JASTSOUND)
static BYTE *pDACbuf;					/* �W���X�g�T�E���h �f�[�^�o�b�t�@ */
static DWORD dwDACptr;					/* �W���X�g�T�E���h �o�b�t�@�|�C���^ */
#endif

#if !defined(FMGEN_DIRECT_OUTPUT)
/*
 *	�X�^�e�B�b�N ���[�N (FM�����������݃L���[)
 */
typedef struct _FMqueue {  
	WORD	reg;						/* ���W�X�^�ԍ�(�g�����l����WORD�^) */
	BYTE	dat;						/* �������݃f�[�^ */
	BYTE	padding;					/* �p�f�B���O */
} FMqueue;

static FMqueue OPNqueue[FMQUEUE_SIZE];	/* OPN���W�X�^�������݃L���[ */
static int uOPNqueuePtr;				/* OPN���W�X�^�������݃L���[�|�C���^ */
static FMqueue WHGqueue[FMQUEUE_SIZE];	/* WHG���W�X�^�������݃L���[ */
static int uWHGqueuePtr;				/* WHG���W�X�^�������݃L���[�|�C���^ */
static FMqueue THGqueue[FMQUEUE_SIZE];	/* THG���W�X�^�������݃L���[ */
static int uTHGqueuePtr;				/* THG���W�X�^�������݃L���[�|�C���^ */
#endif

#if defined(FDDSND)
/*
 *	�X�^�e�B�b�N ���[�N (WAV�Đ�)
 */
static struct _WAVDATA {
	short *p;							/* �g�`�f�[�^�|�C���^ */
	DWORD size;							/* �f�[�^�T�C�Y(�T���v����) */
	DWORD freq;							/* �T���v�����O���g�� */
} Wav[3];

static struct _WAVPLAY {
	BOOL	bPlay;						/* WAV�Đ��t���O */
	DWORD	dwWaveNo;					/* WAV�Ł[���Ȃ�΁[ */
	DWORD	dwCount1;					/* WAV�Ł[��������(������) */
	DWORD	dwCount2;					/* WAV�Ł[��������(������) */
	DWORD	dwCount3;					/* WAV�Đ������� */
} WavP[SNDBUF];

static LPSTR WavName[] = {				/* WAV�t�@�C���� */
	"RELAY_ON.WAV",
	"RELAYOFF.WAV",
	"FDDSEEK.WAV",
#if 0
	"HEADUP.WAV",
	"HEADDOWN.WAV",
#endif
};
#endif

/* �X�e���I�o�͎��̍��E�o�����X�e�[�u�� */
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
 *	�v���g�^�C�v�錾
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
 *	�~���[�g�p�`�����l���}�X�N�r�b�g
 */
static const WORD nChannelMaskBit[6] = {
	0x000001, 0x000002, 0x000004, 0x000040, 0x000080, 0x000100
};

/*
 *	������
 */
void FASTCALL InitSnd(void)
{
	int i;

	/* ���[�N�G���A������ */
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
	/* MIDI���[�N������ */
	nMidiDelay = 100;
	bMidiDelayMode = TRUE;
#endif

	/* �g�`�N���b�s���O�֐���ݒ� */
	if (bMMXflag) {
		CopySoundBuffer = CopySndBufMMX;
	}
	else {
		CopySoundBuffer = CopySndBuf;
	}
}

/*
 *	�N���[���A�b�v
 */
void FASTCALL CleanSnd(void)
{
	int i;

	/* �T�E���h��~ */
	StopSnd();

	/* OPN����� */
	for (i=0; i<3; i++) {
		if (pOPN[i]) {
			delete pOPN[i];
			pOPN[i] = NULL;
		}
	}

	/* �T�E���h�쐬�o�b�t�@����� */
	if (lpsbuf) {
		free(lpsbuf);
		lpsbuf = NULL;
	}

	/* DirectSoundBuffer����� */
	if (lpdsb) {
		lpdsb->Release();
		lpdsb = NULL;
	}
	if (lpdsp) {
		lpdsp->Release();
		lpdsp = NULL;
	}

	/* DirectSound����� */
	if (lpds) {
		lpds->Release();
		lpds = NULL;
	}

	/* uRate���N���A */
	uRate = 0;

	/* �L���v�`���֘A */
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
 *	WAVE�t�@�C���ǂݍ��� (16�r�b�g���m�����f�[�^��p)
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

	/* �t�@�C�����Ɗg���q�����o�� */
	_tsplitpath(fname, NULL, NULL, filename, extname);

	/* XM7�̃p�X�𓾂� */
	memset(path, 0, sizeof(path));
	GetModuleFileName(NULL, path, sizeof(path));
	_tsplitpath(path, drvname, dirname, NULL, NULL);

	/* �p�X���� */
	_tmakepath(path, drvname, dirname, filename, extname);

	/* �t�@�C���I�[�v�� */
	fileh = file_open(path, OPEN_R);
	if (fileh < 0) {
		return FALSE;
	}

	/* RIFF�w�b�_�`�F�b�N */
	file_read(fileh, buf, 4);
	file_read(fileh, (BYTE *)&filSize, 4);
	buf[4] = '\0';
	if (strcmp((char *)buf, "RIFF")) {
		file_close(fileh);
		return FALSE;
	}
	filSize += 8;

	/* WAVE�w�b�_�`�F�b�N */
	file_read(fileh, buf, 8);
	file_read(fileh, (BYTE *)&hdrSize, 4);
	buf[8] = '\0';
	if (strcmp((char *)buf, "WAVEfmt ")) {
		file_close(fileh);
		return FALSE;
	}
	hdrSize += (12 + 8);

	/* WAVEFORMATEX�`�F�b�N */
	file_read(fileh, (BYTE *)&wfex, sizeof(wfex));
	if ((wfex.wFormatTag != WAVE_FORMAT_PCM) ||
		(wfex.nChannels != 1) || (wfex.wBitsPerSample != 16)) {
		/* 16�r�b�g���m�����E���j�APCM�ȊO�͕s�� */
		file_close(fileh);
		return FALSE;
	}

	/* data�`�����N���� */
	while (hdrSize < filSize) {
		/* �`�����N�w�b�_�ǂݍ��� */
		file_seek(fileh, hdrSize);
		file_read(fileh, buf, 4);
		file_read(fileh, (BYTE *)&datSize, 4);
		buf[4] = '\0';

		/* ���̃`�����N�w�b�_�I�t�Z�b�g���v�Z */
		hdrSize += (datSize + 8);

		if (strcmp((char *)buf, "data") == 0) {
			/* data�`�����N�ǂݍ��� */
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

	/* data�`�����N�����ł��� */
	file_close(fileh);
	return FALSE;
}

/*
 *	FDD�T�E���h ������
 */
void FASTCALL InitFDDSnd(void)
{
	int i;

	/* ���[�N������ */
	for (i=0; i<SNDBUF; i++) {
		memset(&WavP[i], 0, sizeof(_WAVPLAY));
	}

	/* WAV�t�@�C���ǂݍ��� */
	for (i=0; i<sizeof(Wav) / sizeof(_WAVDATA); i++) {
		if (!LoadWav(WavName[i], &Wav[i])) {
			Wav[i].size = 0;
			Wav[i].freq = 0;
		}
	}
}

/*
 *	FDD�T�E���h �N���[���A�b�v
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
 *	���W�X�^�ݒ�
 */
static void FASTCALL SetReg(FM::OPN *pOPN, BYTE *reg)
{
	int i;

	/* PSG */
	for (i=0; i<16; i++) {
		pOPN->SetReg((BYTE)i, reg[i]);
	}

	/* FM�����L�[�I�t */
	for (i=0; i<3; i++) {
		pOPN->SetReg(0x28, (BYTE)i);
	}

	/* FM�������W�X�^ */
	for (i=0x30; i<0xb4; i++) {
		pOPN->SetReg((BYTE)i, reg[i]);
	}

	/* FM�������샂�[�h */
	pOPN->SetReg(0x27, reg[0x27] & 0xc0);
}

/*
 *	�Z���N�g
 */
BOOL FASTCALL SelectSnd(HWND hWnd)
{
	PCMWAVEFORMAT pcmwf;
	DSBUFFERDESC dsbd;
	WAVEFORMATEX wfex;
	int i;

	/* assert */
	ASSERT(hWnd);

	/* �N���t���O���Ă� */
	bInitFlag = TRUE;

	/* �p�����[�^��ݒ� */
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

	/* rate==0�Ȃ�A�������Ȃ� */
	if (uRate == 0) {
		return TRUE;
	}

	/* DiectSound�I�u�W�F�N�g�쐬 */
	if (FAILED(DirectSoundCreate(NULL, &lpds, NULL))) {
		/* �f�t�H���g�f�o�C�X�Ȃ����A�g�p�� */
		return TRUE;
	}

	/* �������x����ݒ�(�D�拦��) */
	if (FAILED(lpds->SetCooperativeLevel(hWnd, DSSCL_PRIORITY))) {
		return FALSE;
	}

	/* �v���C�}���o�b�t�@���쐬 */
	memset(&dsbd, 0, sizeof(dsbd));
	dsbd.dwSize = sizeof(dsbd);
	dsbd.dwFlags = DSBCAPS_PRIMARYBUFFER;
	if (FAILED(lpds->CreateSoundBuffer(&dsbd, &lpdsp, NULL))) {
		return FALSE;
	}

	/* �v���C�}���o�b�t�@�̃t�H�[�}�b�g���w�� */
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

	/* �Z�J���_���o�b�t�@���쐬 */
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
	dsbd.dwBufferBytes &= (DWORD)0xfffffff8;	/* 8�o�C�g���E */
	uBufSize = dsbd.dwBufferBytes;
	dsbd.lpwfxFormat = (LPWAVEFORMATEX)&pcmwf;
	if (FAILED(lpds->CreateSoundBuffer(&dsbd, &lpdsb, NULL))) {
		return FALSE;
	}

	/* �T�E���h�o�b�t�@���쐬(�Z�J���_���o�b�t�@�̔����̎��ԂŁADWORD) */
	lpsbuf = (DWORD *)malloc(uBufSize);
	if (lpsbuf == NULL) {
		return FALSE;
	}
	memset(lpsbuf, 0, uBufSize);

	/* �T���v���J�E���^�A�T�E���h���Ԃ��N���A */
	uSample = 0;
	dwSoundTotal = 0;

	/* OPN�f�o�C�X���쐬 */
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

	/* THG SSG�̃G���x���[�v���[�h��PSG�݊��ɐݒ� */
	pOPN[OPN_THG]->psg.SetSSGEnvMode(FALSE);
	bTHGUse = FALSE;

	/* �ăZ���N�g�ɔ����A���W�X�^�ݒ� */
	for (i=0; i<3; i++) {
		nScale[i] = 0;
	}
	opn_notify(0x27, 0);
	whg_notify(0x27, 0);
	thg_notify(0x27, 0);
	for (i=0; i<3; i++) {
		SetReg(pOPN[i], opn_reg[i]);
	}

	/* �L���v�`���֘A */
	if (!pWavCapture) {
		pWavCapture = (WORD *)malloc(sizeof(WORD) * 0x8000);
	}
	ASSERT(hWavCapture == -1);
	ASSERT(!bWavCapture);

#if defined(LPRINT) && defined(JASTSOUND)
	/* �W���X�g�T�E���h ������ */
	pDACbuf = (BYTE *)malloc((uRate * uTick) / 100);
	pDACbuf[0] = 0x80;
	dwDACptr = 0;
#endif

	/* �T�E���h�X�^�[�g */
	bNowBank = FALSE;
	PlaySnd();

	/* �{�����[���ݒ� */
	SetSoundVolume();

	return TRUE;
}

/*
 *	�K�p
 */
void FASTCALL ApplySnd(void)
{
	/* �N���������́A���^�[�� */
	if (!bInitFlag) {
		return;
	}

	/* ROMEO���g�p���̓~���[�g */
#if defined(ROMEO)
	if (bRomeo) {
		ROMEO_Mute(!bUseRomeo);
	}
#endif

	/* �p�����[�^��v�`�F�b�N */
	if ((uRate == nSampleRate) && (uTick == nSoundBuffer) &&
		(bMode == bInterpolation) && (uStereo == uStereoOut)) {
		return;
	}

	/* ���ɏ������ł��Ă���Ȃ�A��� */
	if (uRate != 0) {
		CleanSnd();
	}

	/* �ăZ���N�g */
	SelectSnd(hMainWnd);

	/* �{�����[���ݒ� */
	SetSoundVolume();
}

/*
 *	�{�����[���ݒ�
 */
void FASTCALL SetSoundVolume(void)
{
	int i;

	/* FM����/PSG�{�����[���ݒ� */
	for (i=0; i<3; i++) {
		ASSERT(pOPN[i]);

		if (pOPN[i]) {
			pOPN[i]->SetVolumeFM(nFMVolume * 2);
			pOPN[i]->psg.SetVolume(nPSGVolume * 2);
		}
	}

	/* BEEP��/CMT��/�e����ʉ��{�����[���ݒ� */
	nBeepLevel = (int)(32767.0 * pow(10.0, nBeepVolume / 20.0));
	nCMTLevel = (int)(32767.0 * pow(10.0, nCMTVolume / 20.0));
#if defined(FDDSND)
	nWaveLevel = (int)(32767.0 * pow(10.0, nWaveVolume / 20.0));
#endif

	/* �`�����l���Z�p���[�V�����ݒ� */
	l_vol[0][1] = l_vol[1][2] = l_vol[2][3] = l_vol[1][4] =
	r_vol[1][1] = r_vol[0][2] = r_vol[1][3] = r_vol[2][4] = 16 + uChSeparation;
	r_vol[0][1] = r_vol[1][2] = r_vol[2][3] = r_vol[1][4] =
	l_vol[1][1] = l_vol[0][2] = l_vol[1][3] = l_vol[2][4] = 16 - uChSeparation;
}

/*
 *	�{�����[���ݒ�2(�ݒ�_�C�A���O�p)
 */
void FASTCALL SetSoundVolume2(UINT uSp, int nFM, int nPSG,
							  int nBeep, int nCMT, int nWav)
{
	int i;

	/* FM����/PSG�{�����[���ݒ� */
	for (i=0; i<3; i++) {
		ASSERT(pOPN[i]);

		if (pOPN[i]) {
			pOPN[i]->SetVolumeFM(nFM * 2);
			pOPN[i]->psg.SetVolume(nPSG * 2);
		}
	}

	/* BEEP��/CMT��/�e����ʉ��{�����[���ݒ� */
	nBeepLevel = (int)(32767.0 * pow(10.0, nBeep / 20.0));
	nCMTLevel = (int)(32767.0 * pow(10.0, nCMT / 20.0));
#if defined(FDDSND)
	nWaveLevel = (int)(32767.0 * pow(10.0, nWav / 20.0));
#endif

	/* �`�����l���Z�p���[�V�����ݒ� */
	l_vol[0][1] = l_vol[1][2] = l_vol[2][3] = l_vol[1][4] =
	r_vol[1][1] = r_vol[0][2] = r_vol[1][3] = r_vol[2][4] = 16 + uSp;
	r_vol[0][1] = r_vol[1][2] = r_vol[2][3] = r_vol[1][4] =
	l_vol[1][1] = l_vol[0][2] = l_vol[1][3] = l_vol[2][4] = 16 - uSp;
}

/*
 *	���t�J�n
 */
void FASTCALL PlaySnd()
{
	HRESULT hr;
	WORD *ptr1, *ptr2;
	DWORD size1, size2;

	if (lpdsb) {
		/* �o�b�t�@�����ׂăN���A���� */
		if (lpsbuf) {
			memset(lpsbuf, 0, uBufSize);
		}

		/* ���b�N */
		hr = lpdsb->Lock(0, uBufSize, (void **)&ptr1, &size1,
						(void**)&ptr2, &size2, 0);
		/* �o�b�t�@�������Ă���΁A���X�g�A */
		if (hr == DSERR_BUFFERLOST) {
			lpdsb->Restore();
		}
		/* ���b�N���������ꍇ�̂݁A�Z�b�g */
		if (SUCCEEDED(hr)) {
			if (ptr1) {
				memset(ptr1, 0, size1);
			}
			if (ptr2) {
				memset(ptr2, 0, size2);
			}

			/* �A�����b�N */
			lpdsb->Unlock(ptr1, size1, ptr2, size2);
		}

		/* ���t�J�n */
		lpdsb->Play(0, 0, DSBPLAY_LOOPING);
	}

	/* �T���v���J�E���^�A�T�E���h���Ԃ��N���A */
	uSample = 0;
	dwSoundTotal = 0;
}

/*
 *	���t��~
 */
void FASTCALL StopSnd()
{
	if (lpdsb) {
		lpdsb->Stop();
	}
}

/*
 *	BEEP����
 */
static void FASTCALL BeepSnd(int32 *buf, int samples)
{
	int sf;
	int i;

	/* BEEP���o�̓`�F�b�N */
	if (!bBeepFlag) {
		return;
	}

	/* �T���v���������� */
	for (i=0; i<samples; i++) {
		/* ��`�g���쐬 */
		sf = (int)(uBeep * nBeepFreq * 2);
		sf /= (int)uRate;

		/* ��E��ɉ����ăT���v���������� */
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

		/* �J�E���^�A�b�v */
		uBeep++;
		if (uBeep >= uRate) {
			uBeep = 0;
		}
	}
}

#if XM7_VER >= 2
/*
 *	BEEP����
 */
static void FASTCALL KeyEncBeepSnd(int32 *buf, int samples)
{
	int sf;
	int i;

	/* BEEP���o�̓`�F�b�N */
	if (!bKeyEncBeepFlag) {
		return;
	}

	/* �T���v���������� */
	for (i=0; i<samples; i++) {
		/* ��`�g���쐬 */
		sf = (int)(uKeyEncBeep * 2350 * 2);
		sf /= (int)uRate;

		/* ��E��ɉ����ăT���v���������� */
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

		/* �J�E���^�A�b�v */
		uKeyEncBeep++;
		if (uKeyEncBeep >= uRate) {
			uKeyEncBeep = 0;
		}
	}
}
#endif

/*
 *	�e�[�v����
 */
static void FASTCALL TapeSnd(int32 *buf, int samples)
{
	DWORD dat;
	int i;
	int tmp;

	/* �e�[�v�o�̓`�F�b�N */
	if (!tape_motor || !bTapeMon) {
		return;
	}

	/* �g�`�����������߂� */
	if ((uRate % 11025) == 0) {
		tmp = (uRate * 5) / 44100;
	}
	else {
		tmp = (uRate * 5) / 48000;
	}

	/* �o�͏�Ԃ��ω������ꍇ�A�g�`��Ԃ��J�n���� */
	if (bTapeFlag != bTapeFlag2) {
		if (!uTapeDelta) {
			uTapeDelta = 1;
		}
		else {
			uTapeDelta = (BYTE)(tmp - uTapeDelta + 1);
		}
	}

	/* �T���v���������� */
	for (i=0; i<samples; i++) {
		if (uTapeDelta) {
			/* �g�`��Ԃ��� */
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
			/* �g�`��ԂȂ� */
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

	/* ���݂̃e�[�v�o�͏�Ԃ�ۑ� */
	bTapeFlag2 = bTapeFlag;
}

/*
 *	WAV�f�[�^���� (FDD/CMT)
 */
#if defined(FDDSND)
static void FASTCALL WaveSnd(int32 *buf, int samples)
{
	int i;
	int j;
	int dat;

	/* �T���v���������� */
	for (i=0; i<samples; i++) {
		for (j=0; j<SNDBUF; j++) {
			if (WavP[j].bPlay) {
				dat = Wav[WavP[j].dwWaveNo].p[WavP[j].dwCount1];
				dat = (short)(((int)dat * nWaveLevel) >> 16);
				*buf += (int)dat;
				if (uChannels == 2) {
					*(buf+1) += (int)dat;
				}

				/* �J�E���g�A�b�v */
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
 *	�W���X�g�T�E���h DAC�f�[�^����
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

	/* �T���v���������� */
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
 *	�g�`����
 */
static void FASTCALL MixingSound(DWORD *q, int samples, BOOL bZero)
{
	memset(q, 0, sizeof(DWORD) * samples * uChannels);
	if (!bZero) {
		if (uChannels == 1) {
			/* ���m���� */
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
			/* �X�e���I */
			if (!whg_use && !thg_use) {
				/* WHG/THG���g�p���Ă��Ȃ�(�������m����) */
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
				/* WHG�܂���THG���g�p�� */
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

		/* �e�[�v */
		TapeSnd((int32*)q, samples);

		/* �r�[�v */
		BeepSnd((int32*)q, samples);
#if XM7_VER >= 2
		KeyEncBeepSnd((int32*)q, samples);
#endif

#if defined(FDDSND)
		/* WAV�T�E���h */
		WaveSnd((int32*)q, samples);
#endif

#if defined(LPRINT) && defined(JASTSOUND)
		/* �W���X�g�T�E���h */
		if (lp_use == LP_JASTSOUND) {
			DACSnd((int32*)q, samples);
		}
#endif
	}
}

#if !defined(FMGEN_DIRECT_OUTPUT)
/*
 *	�f�L���[
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
 *	�T�E���h�쐬�o�b�t�@�֒ǉ�
 */
static void FASTCALL AddSnd(BOOL bFill, BOOL bZero)
{
	int i;
	int samples;

	/* OPN�f�o�C�X���쐬����Ă��Ȃ���΁A�������Ȃ� */
	if (!pOPN[OPN_STD] || !pOPN[OPN_WHG] || !pOPN[OPN_THG]) {
		return;
	}

	/* bFill�̏ꍇ�̃T���v���� */
	/* (���m����2byte/sample�E�X�e���I4byte/sample) */
	samples = (uBufSize / uChannels) >> 2;
	samples -= uSample;

	/* !bFill�Ȃ�A���Ԃ���v�� */
	if (!bFill) {
		/* ���Ԍo�߂��狁�߂����_�T���v���� */
		/* �v�Z���ʂ��I�[�o�[�t���[������ɑ΍� 2002/11/25 */
		i = (uRate / 25);
		i *= dwSoundTotal;
		i /= 40000;

		/* uSample�Ɣ�r�A��v���Ă���Ή������Ȃ� */
		if (i <= (int)uSample) {
			return;
		}

		/* uSample�Ƃ̍������񐶐�����T���v���� */
		i -= (int)(uSample);

		/* samples������������΍��i */
		if (i <= samples) {
			samples = i;
		}
	}

	/* �L���[����fmgen�ւ̃f�[�^���荞�� */
#if !defined(FMGEN_DIRECT_OUTPUT)
	FMReg_Dequeue(0x07);
#endif

	/* �~�L�V���O */
	MixingSound(&lpsbuf[uSample * uChannels], samples, bZero);

	/* �X�V */
	if (bFill) {
		dwSoundTotal = 0;
		uSample = 0;
	}
	else {
		uSample += samples;
	}
}

/*
 *	WAV�L���v�`������
 */
static void FASTCALL WavCapture(void)
{
	UINT nSize;
	DWORD *p;
	WORD *q;
	int j;

	/* WAV�L���v�`�����łȂ���΁A���^�[�� */
	if (hWavCapture < 0) {
		return;
	}
	ASSERT(pWavCapture);

	/* �|�C���^�A�T�C�Y��������(nSize��WORD�ϊ����BYTE�l) */
	p = lpsbuf;
	nSize = uBufSize / 2;

	/* bWavCapture��FALSE�Ȃ� */
	if (!bWavCapture) {
		/* ���o���`�F�b�N */
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
		/* ���� */
		if (nSize == 0) {
			return;
		}
	}

	/* nWavCapture���l�� */
	if ((nWavCapture + nSize) >= 0x8000) {
		/* 32KB�����ς��܂ŃR�s�[ */
		j = (0x8000 - nWavCapture) >> 1;
		q = &pWavCapture[nWavCapture >> 1];
		CopySoundBuffer(p, q, j);
		p += j;

		/* �c��T�C�Y���X�V */
		nSize -= (0x8000 - nWavCapture);

		/* �������� */
		file_write(hWavCapture, (BYTE*)pWavCapture, 0x8000);
		dwWavCapture += 0x8000;
		nWavCapture = 0;
	}

	/* �]����R�s�[ */
	j = nSize >> 1;
	q = &pWavCapture[nWavCapture >> 1];
	CopySoundBuffer(p, q, j);
	nWavCapture += nSize;

	/* �����Ș^����� */
	bWavCapture = TRUE;
}

/*
 *	�������
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

	/* ��݂��R�}���h���s by ���� */
#if defined(ROMEO)
	if (bUseRomeo) {
		juliet_YMF288EXEC((DWORD)uTick);
	}
#endif

	/* MIDI�R�}���h���s */
#if defined(MIDI)
	if (bMidiDelayMode) {
		MidiSendData((DWORD)uTick);
	}
	else {
		MidiSendData((DWORD)nMidiDelay);
	}
#endif

	/* ����������Ă��Ȃ���΁A�������Ȃ� */
	if (!lpdsb) {
		return TRUE;
	}

	/* �������݈ʒu�𓾂� */
	if (FAILED(lpdsb->GetCurrentPosition(&dwPlayC, &dwWriteC))) {
		return FALSE;
	}

	/* �������݈ʒu�ƃo���N����A�K�v���𔻒f */
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

	/* �������ޕK�v���Ȃ���΁A���^�[�� */
	if (!bWrite) {
		/* �e�[�v */
		if (tape_motor && bTapeMon) {
			bWrite = TRUE;
		}
		/* BEEP */
		if (beep_flag && speaker_flag) {
			bWrite = TRUE;
		}

		/* �ǂ��炩��ON�Ȃ�A�o�b�t�@�[�U */
		if (bWrite) {
			AddSnd(FALSE, bZero);
		}

		return FALSE;
	}

	/* �������݁B�܂��T�E���h�쐬�o�b�t�@��S�����߂� */
	AddSnd(TRUE, bZero);

	/* �����Ń��b�N */
	hr = lpdsb->Lock(dwOffset, uBufSize / 2, (void **)&ptr1, &size1,
						(void**)&ptr2, &size2, 0);

	/* �o�b�t�@�������Ă���΁A���X�g�A */
	if (hr == DSERR_BUFFERLOST) {
		lpdsb->Restore();
		hr = lpdsb->Lock(dwOffset, uBufSize / 2, (void **)&ptr1, &size1,
							(void**)&ptr2, &size2, 0);
	}
	/* ���b�N�������Ȃ���΁A�����Ă��Ӗ����Ȃ� */
	if (FAILED(hr)) {
		return FALSE;
	}

	/* �T�E���h�쐬�o�b�t�@���Z�J���_���o�b�t�@ */
	p = lpsbuf;
	i = (int)(size1 / 2);
	j = (int)(size2 / 2);
	CopySoundBuffer(p, ptr1, i);
	CopySoundBuffer(p + i, ptr2, j);

	/* �A�����b�N */
	lpdsb->Unlock(ptr1, size1, ptr2, size2);

	/* �o���N���] */
	bNowBank = (!bNowBank);

	/* WAV�L���v�`������ */
	WavCapture();

	return TRUE;
}

/*
 *	�T�E���h�o�͊����ҋ@
 */
void FASTCALL WaitSnd(void)
{
	DWORD dwPlayC, dwWriteC;
	DWORD dwLength;

	while (TRUE) {
		/* ����������Ă��Ȃ���΁A�������Ȃ� */
		if (!lpdsb) {
			return;
		}

		/* �������݈ʒu�𓾂� */
		if (FAILED(lpdsb->GetCurrentPosition(&dwPlayC, &dwWriteC))) {
			break;
		}

		/* �������݈ʒu�ƃo���N����A�K�v���𔻒f */
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
 *	���x���擾
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
		/* ��݂��g�p���͎擾�ł��Ȃ��̂�0 */
		if (bUseRomeo) {
			return 0;
		}
#endif

		/* ���ۂɎg���Ă��Ȃ����0 */
		if ((!opn_enable || !opn_use) && (fm7_ver == 1)) {
			return 0;
		}
	}

	/* WHG */
	if ((ch >= 6) && (ch < 12)) {
		p = pOPN[OPN_WHG];
		ch -= 6;

		/* ���ۂɎg���Ă��Ȃ����0 */
		if (!whg_enable || !whg_use) {
			return 0;
		}

#if defined(ROMEO)
		/* ��݂��g�p����FM�������͎擾�ł��Ȃ��̂�0 */
		if (bUseRomeo && (ch < 3)) {
			return 0;
		}
#endif
	}

	/* THG */
	if ((ch >= 12) && (ch < 18)) {
		p = pOPN[OPN_THG];
		ch -= 12;

		/* ���ۂɎg���Ă��Ȃ����0 */
		if ((!thg_enable || !thg_use) && (fm7_ver != 1)) {
			return 0;
		}
	}

	/* ���݃`�F�b�N */
	if (!p) {
		return 0;
	}

	/* FM,PSG�̋�� */
	if (ch < 3) {
		/* FM:512�T���v����2��a���v�Z */
		buf = p->rbuf[ch];

		s = 0;
		for (i=0; i<512; i++) {
			t = (double)*buf++;
			t *= t;
			s += t;
		}
		s /= 512;

		/* �[���`�F�b�N */
		if (s == 0) {
			return 0;
		}

		/* log10����� */
		s = log10(s);

		/* FM�����␳ */
		s *= 40.0;
	}
	else {
		/* PSG:512�T���v����2��a���v�Z */
		buf = p->psg.rbuf[ch - 3];

		s = 0;
		for (i=0; i<512; i++) {
			t = (double)*buf++;
			t *= t;
			s += t;
		}
		s /= 512;

		/* �[���`�F�b�N */
		if (s == 0) {
			return 0;
		}

		/* log10����� */
		s = log10(s);

		/* PSG�����␳ */
		s *= 60.0;
	}

	return (int)s;
}

/*
 *	WAV�L���v�`���J�n
 */
void FASTCALL OpenCaptureSnd(char *fname)
{
	WAVEFORMATEX wfex;
	DWORD dwSize;
	int fileh;

	ASSERT(fname);
	ASSERT(hWavCapture < 0);
	ASSERT(!bWavCapture);

	/* �������łȂ���΁A���^�[�� */
	if (!pOPN[OPN_STD] || !pOPN[OPN_WHG] || !pOPN[OPN_THG]) {
		return;
	}

	/* �o�b�t�@��������΁A���^�[�� */
	if (!pWavCapture) {
		return;
	}

	/* uBufSize / 2��0x8000�ȉ��łȂ��ƃG���[ */
	if ((uBufSize / 2) > 0x8000) {
		return;
	}

	/* �t�@�C���I�[�v��(�������݃��[�h) */
	fileh = file_open(fname, OPEN_W);
	if (fileh < 0) {
		return;
	}

	/* RIFF�w�b�_�������� */
	if (!file_write(fileh, (BYTE*)"RIFF�ŎEWAVEfmt ", 16)) {
		file_close(fileh);
		return;
	}

	/* WAVEFORMATEX�������� */
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

	/* data�T�u�w�b�_�������� */
	if (!file_write(fileh, (BYTE *)"data�ŎE", 8)) {
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
 *	WAV�L���v�`���I��
 */
void FASTCALL CloseCaptureSnd(void)
{
	DWORD dwLength;

	ASSERT(hWavCapture >= 0);

	/* �o�b�t�@�Ɏc���������������� */
	file_write(hWavCapture, (BYTE*)pWavCapture, nWavCapture);
	dwWavCapture += nWavCapture;
	nWavCapture = 0;

	/* �t�@�C�������O�X���������� */
	file_seek(hWavCapture, 4);
	dwLength = dwWavCapture + sizeof(WAVEFORMATEX) + 20;
	file_write(hWavCapture, (BYTE *)&dwLength, sizeof(dwLength));

	/* data�������O�X���������� */
	file_seek(hWavCapture, sizeof(WAVEFORMATEX) + 24);
	file_write(hWavCapture, (BYTE *)&dwWavCapture, sizeof(dwWavCapture));

	/* �t�@�C���N���[�Y */
	file_close(hWavCapture);

	/* ���[�N�G���A�N���A */
	hWavCapture = -1;
	bWavCapture = FALSE;
}

/*
 *	ROMEO �~���[�g����
 */
#if defined(ROMEO)
extern "C" {
void FASTCALL ROMEO_Mute(BOOL flag)
{
	int i;

	/* ��Ԃ��ω������ꍇ�̂ݏ������s�� */
	if (bRomeo && (bRomeoMute != flag)) {
		if (flag) {
			/* �~���[�g���� */
			juliet_YMF288Mute(FALSE);

			/* PSG */
			for (i=0; i<16; i++) {
				juliet_YMF288A_B((BYTE)i, opn_reg[OPN_STD][i]);
			}

			/* FM�����L�[�I�t */
			for (i=0; i<3; i++) {
				juliet_YMF288A_B(0x28, (BYTE)i);
				juliet_YMF288A_B(0x28, (BYTE)(i | 0x04));
			}

			/* FM�������W�X�^ */
			for (i=0x30; i<0xb4; i++) {
				juliet_YMF288A_B((BYTE)i, opn_reg[OPN_STD][i]);
				juliet_YMF288B_B((BYTE)i, opn_reg[OPN_WHG][i]);
			}

			/* FM�������샂�[�h */
			juliet_YMF288A_B((BYTE)0x27, (BYTE)(opn_reg[OPN_STD][0x27] & 0xc0));
		}
		else {
			/* �~���[�g�ݒ� */
			juliet_YMF288Mute(TRUE);
		}

		/* �t���O�ۑ� */
		bRomeoMute = flag;
	}
}
}
#endif

/*
 *	ROMEO �v���X�P�[���ύX���p���W�X�^�Đݒ�
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

	/* FM���� */
	for (i=0xa0; i<0xb0; i++) {
		juliet_YMF288A_B((BYTE)i, opn_reg[OPN_STD][i]);
		juliet_YMF288B_B((BYTE)i, opn_reg[OPN_WHG][i]);
	}
}
#endif

/*
 *	�~���[�g��Ԏ擾
 */
extern "C" {
BOOL FASTCALL GetMute(int ch)
{
	return (BOOL)(nChannelMask[ch / 6] & nChannelMaskBit[ch % 6]);
}
}

/*
 *	�~���[�g��Ԑݒ�
 */
extern "C" {
void FASTCALL SetMute(int ch, BOOL mute)
{
	/* �t���O�̐ݒ� */
	if (mute) {
		nChannelMask[ch / 6] |= (WORD)nChannelMaskBit[ch % 6];
	}
	else {
		nChannelMask[ch / 6] &= (WORD)~nChannelMaskBit[ch % 6];
	}

	/* fmgen�ɑ΂��ă~���[�g���s */
	if (pOPN[ch / 6]) {
		pOPN[ch / 6]->SetChannelMask(nChannelMask[ch / 6]);
	}
}
}

/*
 *	OPN�o��
 */
extern "C" {
void FASTCALL opn_notify(BYTE reg, BYTE dat)
{
	/* OPN���Ȃ���΁A�������Ȃ� */
	if (!pOPN[OPN_STD]) {
		return;
	}

	/* �v���X�P�[���𒲐� */
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

	/* Ch3���샂�[�h�`�F�b�N */
	if (reg == 0x27) {
		if (uCh3Mode[OPN_STD] == dat) {
			return;
		}
		uCh3Mode[OPN_STD] = dat;
	}

	/* 0xff���W�X�^�̓`�F�b�N */
	if (reg == 0xff) {
		if ((opn_reg[OPN_STD][0x27] & 0xc0) != 0x80) {
			return;
		}
	}

	/* �T�E���h���� */
	AddSnd(FALSE, FALSE);

	/* �o�́E�L���[�o�^ */
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

	/* ��݂��o�� */
#if defined(ROMEO)
	if (bUseRomeo) {
		juliet_YMF288A_B(reg, dat);
	}
#endif
}
}

/*
 *	WHG�o��
 */
extern "C" {
void FASTCALL whg_notify(BYTE reg, BYTE dat)
{
	/* WHG���Ȃ���΁A�������Ȃ� */
	if (!pOPN[OPN_WHG]) {
		return;
	}

	/* �v���X�P�[���𒲐� */
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

	/* Ch3���샂�[�h�`�F�b�N */
	if (reg == 0x27) {
		if (uCh3Mode[OPN_WHG] == dat) {
			return;
		}
		uCh3Mode[OPN_WHG] = dat;
	}

	/* 0xff���W�X�^�̓`�F�b�N */
	if (reg == 0xff) {
		if ((opn_reg[OPN_WHG][0x27] & 0xc0) != 0x80) {
			return;
		}
	}

	/* �T�E���h���� */
	AddSnd(FALSE, FALSE);

	/* �o�́E�L���[�o�^ */
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

	/* ��݂��o�� */
#if defined(ROMEO)
	if (bUseRomeo && ((reg >= 0x20) && (reg <= 0xfe))) {
		juliet_YMF288B_B(reg, dat);
	}
#endif
}
}

/*
 *	THG�o��
 */
extern "C" {
void FASTCALL thg_notify(BYTE reg, BYTE dat)
{
	/* THG���Ȃ���΁A�������Ȃ� */
	if (!pOPN[OPN_THG]) {
		return;
	}

	/* �G���x���[�v���[�h�ݒ� */
	if (thg_use != bTHGUse) {
		pOPN[OPN_THG]->psg.SetSSGEnvMode(thg_use);
		bTHGUse = thg_use;
	}

	/* �v���X�P�[���𒲐� */
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

	/* Ch3���샂�[�h�`�F�b�N */
	if (reg == 0x27) {
		if (uCh3Mode[OPN_THG] == dat) {
			return;
		}
		uCh3Mode[OPN_THG] = dat;
	}

	/* 0xff���W�X�^�̓`�F�b�N */
	if (reg == 0xff) {
		if ((opn_reg[OPN_THG][0x27] & 0xc0) != 0x80) {
			return;
		}
	}

	/* �T�E���h���� */
	AddSnd(FALSE, FALSE);

	/* �o�́E�L���[�o�^ */
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
 *	BEEP�o��
 */
extern "C" {
void FASTCALL beep_notify(void)
{
	/* �o�͏�Ԃ��ω����Ă��Ȃ���΃��^�[�� */
	if (!((beep_flag & speaker_flag) ^ bBeepFlag)) {
		return;
	}

	/* �T�E���h���� */
	AddSnd(FALSE, FALSE);

	/* �t���O�ێ� */
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
 *	�L�[�G���R�[�_BEEP�o��
 */
extern "C" {
void FASTCALL keyencbeep_notify(void)
{
	/* �o�͏�Ԃ��ω����Ă��Ȃ���΃��^�[�� */
	if (!(keyenc_beep_flag ^ bKeyEncBeepFlag)) {
		return;
	}

	/* �T�E���h���� */
	AddSnd(FALSE, FALSE);

	/* �t���O�ێ� */
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
 *	�e�[�v�o��
 */
extern "C" {
void FASTCALL tape_notify(BOOL flag)
{
	/* �o�͏�Ԃ��ω��������`�F�b�N */
	if (bTapeFlag == flag) {
		return;
	}

	if (bTapeMon) {
		/* �T�E���h���� */
		AddSnd(FALSE, FALSE);
	}

	/* �t���O�ێ� */
	bTapeFlag = flag;
}
}

/*
 *	WAV�o��
 */
#if defined(FDDSND)
extern "C" {
void FASTCALL wav_notify(BYTE no)
{
	int i;
	int j;
	DWORD k;

	/* �T�E���h���� */
	AddSnd(FALSE, FALSE);

	if (no == SOUND_STOP) {
		/* ��~ */
		for (i=0; i<SNDBUF; i++) {
			WavP[i].bPlay = FALSE;
		}
	}
	else {
		if (Wav[no].freq) {
			j = 0;
			k = 0;
			for (i=0; i<SNDBUF; i++) {
				/* �Đ���~���̃`�����l�������� */
				if (!WavP[i].bPlay) {
					j = i;
					break;
				}
				else {
					/* ��ԍŏ��ɍĐ����J�n���ꂽ�`�����l�������� */
					if (k < WavP[i].dwCount3) {
						k = WavP[i].dwCount3;
						j = i;
					}
				}
			}

			/* �f�[�^�Z�b�g */
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
 *	�W���X�g�T�E���h DAC�f�[�^����
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
