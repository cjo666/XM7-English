/*
 *	FM-7 EMULATOR "XM7"
 *
 *	Copyright (C) 1999-2017 �o�h�D(Twitter:@xm6_original)
 *	Copyright (C) 2001-2017 Ryu Takegami (Twitter:@RyuTakegami)
 *
 *	[ Win32API �\�� ]
 */

#ifdef _WIN32

#ifndef _w32_draw_h_
#define _w32_draw_h_

/*
 *	�萔��`(V2�p)
 */
#define	V2XPoint	464
#define	V2YPoint	10
#define	V2XSize		27
#define	V2YSize		14
#define	V2XPxSz		5
#define	V2YPxSz		2

#ifdef _w32_h_

#ifdef __cplusplus
extern "C" {
#endif
/*
 *	��v�G���g��
 */
void FASTCALL InitDraw(void);
										/* ������ */
void FASTCALL CleanDraw(void);
										/* �N���[���A�b�v */
BOOL FASTCALL SelectDraw(HWND hWnd);
										/* �Z���N�g */
void FASTCALL ModeDraw(HWND hWnd, BOOL bFullScreen);
										/* �`�惂�[�h�ύX */
void FASTCALL OnPaint(HWND hWnd);
										/* �ĕ`�� */
void FASTCALL OnDraw(HWND hWnd, HDC hDC);
										/* �����`�� */
void FASTCALL EnterMenu(HWND hWnd);
										/* ���j���[�J�n */
void FASTCALL ExitMenu(void);
										/* ���j���[�I�� */
void FASTCALL SetDirtyFlag(int top, int bottom, BOOL flag);
										/* �ĕ`�惉�X�^�ꊇ�ݒ� */
#endif	/* _w32_h_ */


/*
 *	��v���[�N
 */
extern BOOL bFullScreen;
										/* �t���X�N���[�� */
extern BOOL bFullRequested;
										/* �t���X�N���[���v�� */
extern BOOL bDrawSelected;
										/* �Z���N�g�ς݃t���O */
extern BOOL bFullScan;
										/* �t���X�L����(Window) */
extern BOOL bFullScanFS;
										/* �t���X�L����(FullScreen) */
extern BOOL bDoubleSize;
										/* 2�{�\���t���O */
extern BOOL bPseudo400Line;
										/* �^��400���C���t���O */
#if XM7_VER == 1
extern BOOL bGreenMonitor;
										/* �O���[�����j�^�t���O */
#endif
#if XM7_VER == 2
extern BOOL bTTLMonitor;
										/* TTL���j�^�t���O */
#endif
extern BOOL bRasterRendering;
										/* ���X�^�����_�����O�t���O */
#if ((XM7_VER <= 2) && defined(FMTV151))
extern const BYTE nV2data[];
										/* V2�f�[�^ */
#endif
#ifdef __cplusplus
}
#endif

#endif	/* _w32_draw_h_ */
#endif	/* _WIN32 */
