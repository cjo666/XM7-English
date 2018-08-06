/*
 *	FM-7 EMULATOR "XM7"
 *
 *	Copyright (C) 1999-2017 �o�h�D(Twitter:@xm6_original)
 *	Copyright (C) 2001-2017 Ryu Takegami (Twitter:@RyuTakegami)
 *
 *	[ Win32API �T�E���h ]
 */

#ifdef _WIN32

#ifndef _w32_snd_h_
#define _w32_snd_h_

/*
 *	�萔��`
 */
#define SNDBUF					20		/* WAV�ő哯�������� */
#define FMQUEUE_SIZE			1024	/* ���W�X�^�������݃L���[�T�C�Y */
#define CHSEPARATION_DEFAULT	7		/* �`�����l���Z�p���[�V�����W���l */
#define FMVOLUME_DEFAULT		0		/* FM�����{�����[���f�t�H���g�l */
#define PSGVOLUME_DEFAULT		-2		/* PSG�{�����[���f�t�H���g�l */
#define BEEPVOLUME_DEFAULT		-24		/* BEEP���{�����[���f�t�H���g�l */
#define CMTVOLUME_DEFAULT		-24		/* CMT�����j�^�{�����[���f�t�H���g�l */
#define WAVEVOLUME_DEFAULT		-6		/* �e����ʉ��{�����[���f�t�H���g�l */


#ifdef __cplusplus
extern "C" {
#endif
/*
 *	��v�G���g��
 */
void FASTCALL InitSnd(void);
										/* ������ */
void FASTCALL CleanSnd(void);
										/* �N���[���A�b�v */
BOOL FASTCALL SelectSnd(HWND hWnd);
										/* �Z���N�g */
void FASTCALL ApplySnd(void);
										/* �K�p */
void FASTCALL PlaySnd(void);
										/* ���t�J�n */
void FASTCALL StopSnd(void);
										/* ���t��~ */
BOOL FASTCALL ProcessSnd(BOOL bZero);
										/* �o�b�t�@�[�U�莞���� */
void FASTCALL WaitSnd(void);
										/* �T�E���h�o�͊����ҋ@ */
int FASTCALL GetLevelSnd(int ch);
										/* �T�E���h���x���擾 */
void FASTCALL OpenCaptureSnd(char *fname);
										/* WAV�L���v�`���J�n */
void FASTCALL CloseCaptureSnd(void);
										/* WAV�L���v�`���I�� */
BOOL FASTCALL GetMute(int ch);
										/* �~���[�g��Ԏ擾 */
void FASTCALL SetMute(int ch, BOOL mute);
										/* �~���[�g��Ԑݒ� */
#if defined(ROMEO)
void FASTCALL ROMEO_Mute(BOOL flag);
										/* ROMEO�~���[�g */
#endif
#if defined(FDDSND)
void FASTCALL InitFDDSnd(void);
										/* ������ */
void FASTCALL CleanFDDSnd(void);
										/* �N���[���A�b�v */
#endif
void FASTCALL SetSoundVolume(void);
										/* �{�����[���ݒ� */
void FASTCALL SetSoundVolume2(UINT uSp, int nFM, int nPSG,
							  int nBeep, int nCMT, int nWav);
										/* �{�����[���ݒ�2 */

/*
 *	��v���[�N
 */
extern UINT nSampleRate;
										/* �T���v�����[�g(Hz�A0�Ŗ���) */
extern UINT nSoundBuffer;
										/* �T�E���h�o�b�t�@(�_�u���Ams) */
extern BOOL bInterpolation;
										/* �T�E���h��ԃ��[�h */
extern BOOL bPreciseMix;
										/* �����x�������[�h */
extern UINT nBeepFreq;
										/* BEEP���g��(Hz) */
extern int hWavCapture;
										/* WAV�L���v�`���t�@�C���n���h�� */
extern BOOL bWavCapture;
										/* WAV�L���v�`���J�n�� */
extern UINT uStereoOut;
										/* �o�̓��[�h */
extern BOOL bForceStereo;
										/* �����X�e���I�o�� */
extern BOOL bTapeMon;
										/* �e�[�v�����j�^ */
#if defined(ROMEO)
extern BOOL bUseRomeo;
										/* ��݂��g�p�t���O */
#endif
#if defined(FDDSND)
extern UINT uSeekVolume;
										/* �V�[�N���� */
#endif
extern UINT uChSeparation;
										/* �X�e���I�`�����l���Z�p���[�V���� */
extern int nFMVolume;
										/* FM�����{�����[�� */
extern int nPSGVolume;
										/* PSG�{�����[�� */
extern int nBeepVolume;
										/* BEEP���{�����[�� */
extern int nCMTVolume;
										/* CMT�����j�^�{�����[�� */
extern int nWaveVolume;
										/* ���ʉ��{�����[�� */
#if XM7_VER == 1
extern BOOL bFMXPSG;
										/* FM-X PSG�G�~�����[�V�������[�h(��) */
#endif
#ifdef __cplusplus
}
#endif

#endif	/* _w32_snd_h_ */
#endif	/* _WIN32 */
