/*
 *	FM-7 EMULATOR "XM7"
 *
 *	Copyright (C) 1999-2017 �o�h�D(Twitter:@xm6_original)
 *	Copyright (C) 2001-2017 Ryu Takegami (Twitter:@RyuTakegami)
 *
 *	[ Win32API DirectDraw ]
 */

#ifdef _WIN32

#ifndef _w32_dd_h_
#define _w32_dd_h_

#ifdef __cplusplus
extern "C" {
#endif
/*
 *	�萔��`
 */
#define	DDRES_400LINE		0
#define	DDRES_480LINE		1
#define	DDRES_WUXGA			2
#define	DDRES_SXGA			3
#define	DDRES_WXGA800		4

/*
 *	��v�G���g��
 */
void FASTCALL InitDD(void);
										/* ������ */
void FASTCALL CleanDD(void);
										/* �N���[���A�b�v */
BOOL FASTCALL SelectDD(void);
										/* �Z���N�g */
void FASTCALL DrawDD(void);
										/* �`�� */
void FASTCALL DrawRasterDD(int nRaster);
										/* ���X�^�`�� */
void FASTCALL DrawPostRenderDD(void);
										/* ���X�^�����_�����O���`�� */
void FASTCALL EnterMenuDD(HWND hWnd);
										/* ���j���[�J�n */
void FASTCALL ExitMenuDD(void);
										/* ���j���[�I�� */
void FASTCALL VramDD(WORD addr);
										/* VRAM�������ݒʒm */
void FASTCALL TvramDD(WORD addr);
										/* �e�L�X�gVRAM�������ݒʒm */
void FASTCALL DigitalDD(void);
										/* TTL�p���b�g�ʒm */
void FASTCALL AnalogDD(void);
										/* �A�i���O�p���b�g�ʒm */
void FASTCALL ReDrawDD(void);
										/* �ĕ`��ʒm */
void FASTCALL ReDrawTVRamDD(void);
										/* �ĕ`��ʒm */
#if XM7_VER >= 3
void FASTCALL WindowDD(void);
										/* �n�[�h�E�F�A�E�B���h�E�ʒm */
#endif

/*
 *	��v���[�N
 */
#if XM7_VER == 1
extern DWORD rgbTTLDD[16];
										/* 640x200 �p���b�g */
#else
extern DWORD rgbTTLDD[8];
										/* 640x200 �p���b�g */
#endif
extern DWORD rgbAnalogDD[4096];
										/* 320x200 �p���b�g */
extern BYTE nDDResolutionMode;
										/* �t���X�N���[�����𑜓x */
extern BOOL bDD480Status;
										/* 640x480 �X�e�[�^�X�t���O */
extern BOOL bDDtruecolor;
										/* TrueColor�D��t���O */
#ifdef __cplusplus
}
#endif

#endif	/* _w32_dd_h_ */
#endif	/* _WIN32 */
