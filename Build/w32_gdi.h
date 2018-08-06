/*
 *	FM-7 EMULATOR "XM7"
 *
 *	Copyright (C) 1999-2017 �o�h�D(Twitter:@xm6_original)
 *	Copyright (C) 2001-2017 Ryu Takegami (Twitter:@RyuTakegami)
 *
 *	[ Win32API �E�C���h�E�\�� ]
 */

#ifdef _WIN32

#ifndef _w32_gdi_h_
#define _w32_gdi_h_

#ifdef __cplusplus
extern "C" {
#endif
/*
 *	��v�G���g��
 */
void FASTCALL InitGDI(void);
										/* ������ */
void FASTCALL CleanGDI(void);
										/* �N���[���A�b�v */
BOOL FASTCALL SelectGDI(HWND hWnd);
										/* �Z���N�g */
void FASTCALL DrawGDI(HWND hWnd, HDC hDC);
										/* �`�� */
void FASTCALL DrawRasterGDI(int nRaster);
										/* ���X�^�`�� */
void FASTCALL DrawPostRenderGDI(HWND hWnd, HDC hDC);
										/* ���X�^�����_�����O���`�� */
void FASTCALL EnterMenuGDI(HWND hWnd);
										/* ���j���[�J�n */
void FASTCALL ExitMenuGDI(void);
										/* ���j���[�I�� */
void FASTCALL VramGDI(WORD addr);
										/* VRAM�������ݒʒm */
void FASTCALL TvramGDI(WORD addr);
										/* �e�L�X�gVRAM�������ݒʒm */
void FASTCALL DigitalGDI(void);
										/* TTL�p���b�g�ʒm */
void FASTCALL AnalogGDI(void);
										/* �A�i���O�p���b�g�ʒm */
void FASTCALL ReDrawGDI(void);
										/* �ĕ`��ʒm */
void FASTCALL ReDrawTVRamGDI(void);
										/* �e�L�X�g�ĕ`��ʒm */
#if XM7_VER >= 3
void FASTCALL WindowGDI(void);
										/* �n�[�h�E�F�A�E�B���h�E�ʒm */
#endif
void SelectCancelGDI(void);
										/*	�Z���N�g�L�����Z������ */

/*
 *	��v���[�N
 */
extern DWORD rgbTTLGDI[16];
										/* �f�W�^���p���b�g */
extern DWORD rgbAnalogGDI[4096];
										/* �A�i���O�p���b�g */
extern BYTE *pBitsGDI;
										/* �r�b�g�f�[�^ */
#ifdef __cplusplus
}
#endif

#endif	/* _w32_gdi_h_ */
#endif	/* _WIN32 */
