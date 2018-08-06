/*
 *	FM-7 EMULATOR "XM7"
 *
 *	Copyright (C) 1999-2017 �o�h�D(Twitter:@xm6_original)
 *	Copyright (C) 2001-2017 Ryu Takegami (Twitter:@RyuTakegami)
 *
 *	[ Win32API �R���g���[���o�[ ]
 */

#ifdef _WIN32

#ifndef _w32_bar_h_
#define _w32_bar_h_

#ifdef __cplusplus
extern "C" {
#endif
/*
 *	��v�G���g��
 */
HWND FASTCALL CreateStatus(HWND hWnd);
										/* �X�e�[�^�X�o�[�쐬 */
void FASTCALL DrawStatus(void);
										/* �`�� */
void FASTCALL PaintStatus(void);
										/* ���ׂčĕ`�� */
void FASTCALL SizeStatus(LONG cx);
										/* �T�C�Y�ύX */
void FASTCALL OwnerDrawStatus(DRAWITEMSTRUCT *pDI);
										/* �I�[�i�[�h���[ */
void FASTCALL ResizeStatus(HWND hwnd, HWND hstatus);
										/* �X�e�[�^�X�o�[���T�C�Y */
void FASTCALL SetStatusMessage(UINT ID);
										/* �X�e�[�^�X���b�Z�[�W�ݒ� */
void FASTCALL EndStatusMessage(void);
										/* �X�e�[�^�X���b�Z�[�W���� */
void FASTCALL OnMenuSelect(WPARAM wParam);
										/* WM_MENUSELECT */
void FASTCALL OnExitMenuLoop(void);
										/* WM_EXITMENULOOP */
void FASTCALL InitThemeDLL(void);
										/* uxtheme.dll ������ */
void FASTCALL CleanThemeDLL(void);
										/* uxtheme.dll �N���[���A�b�v */
void FASTCALL ChangeStatusBorder(HWND hwnd);
										/* ���b�Z�[�W���{�[�_�[�`�� */

/*
 *	��v���[�N
 */
extern HWND hStatusBar;
										/* �X�e�[�^�X�o�[ */
extern BOOL bStatusMessage;
										/* �X�e�[�^�X���b�Z�[�W�\����� */
extern int uPaneX[3];
										/* Drap&Drop�p�y�C��X���W */
#ifdef __cplusplus
}
#endif

#endif	/* _w32_bar_h_ */
#endif	/* _WIN32 */
