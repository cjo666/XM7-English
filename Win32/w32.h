/*
 *	FM-7 EMULATOR "XM7"
 *
 *	Copyright (C) 1999-2017 �o�h�D(Twitter:@xm6_original)
 *	Copyright (C) 2001-2017 Ryu Takegami (Twitter:@RyuTakegami)
 *
 *	[ Win32API ]
 */

#ifdef _WIN32

#ifndef _w32_h_
#define _w32_h_

#ifndef __STDLIB_H
#include <stdlib.h>
#endif

/*
 *	�萔�A�^��`
 */
#ifndef WM_THEMECHANGED
#define WM_THEMECHANGED		0x031A		/* WindowsXP �e�[�}�K�p���b�Z�[�W */
#endif
#ifndef	WM_MOUSEWHEEL
#define	WM_MOUSEWHEEL		0x020a		/* VS2005�p(_WIN32_WINDOWS�̑���) */
#endif

#ifdef __cplusplus
extern "C" {
#endif
/*
 *	��v�G���g��
 */
HFONT FASTCALL CreateTextFont(void);
										/* �e�L�X�g�t�H���g�쐬 */
void FASTCALL LockVM(void);
										/* VM���b�N */
void FASTCALL UnlockVM(void);
										/* VM�A�����b�N */
void FASTCALL OnSize(HWND hWnd, WORD cx, WORD cy);
										/* �T�C�Y�ύX */
void FASTCALL OnCommand(HWND hWnd, WORD wID);
										/* WM_COMMAND */
void FASTCALL OnMenuPopup(HWND hWnd, HMENU hMenu, UINT uPos);
										/* WM_INITMENUPOPUP */
void FASTCALL OnDropFiles(HANDLE hDrop);
										/* WM_DROPFILES */
void FASTCALL OnCmdLine(LPSTR lpCmdLine);
										/* �R�}���h���C�� */
void FASTCALL OnAbout(HWND hWnd);
										/* �o�[�W������� */
void FASTCALL OnRefresh(HWND hWnd);
										/* �ŐV�̏��ɍX�V(R) */
void FASTCALL EnableMenuSub(HMENU hMenu, UINT uID, BOOL bEnable);
										/* ���j���[Enable */
void FASTCALL CheckMenuSub(HMENU hMenu, UINT uID, BOOL bCheck);
										/* ���j���[Check */
void FASTCALL InitIMEDLL(void);
										/* IME�L��/����API DLL������ */
void FASTCALL CleanIMEDLL(void);
										/* IME�L��/����API DLL�N���[���A�b�v */
BOOL FASTCALL EnableIME(HWND hWnd, BOOL flag);
										/* IME�L��/�����؂芷�� */
BOOL FASTCALL FileSelectSub(BOOL bOpen, UINT uFilterID, char *path, char *defext, BYTE IniDirNo);
										/* �t�@�C���I���R�����_�C�A���O */
#if defined(MOUSE)
void FASTCALL MouseModeChange(BOOL flag);
										/* �}�E�X�L���v�`���؂�ւ� */
#endif

/*
 *	��v���[�N
 */
#if XM7_VER == 1
#if defined(BUBBLE)
extern char InitialDir[6][_MAX_DRIVE + _MAX_PATH];
										/* �����f�B���N�g�� */
#else
extern char InitialDir[5][_MAX_DRIVE + _MAX_PATH];
										/* �����f�B���N�g�� */
#endif
#else
extern char InitialDir[5][_MAX_DRIVE + _MAX_PATH];
										/* �����f�B���N�g�� */
#endif
extern HINSTANCE hAppInstance;
										/* �A�v���P�[�V���� �C���X�^���X */
extern HWND hMainWnd;
										/* ���C���E�C���h�E */
extern HWND hDrawWnd;
										/* �`��E�C���h�E */
extern int nErrorCode;
										/* �G���[�R�[�h */
extern BOOL bMenuLoop;
										/* ���j���[���[�v�� */
extern BOOL bMenuExit;
										/* ���j���[�����o���t���O */
extern BOOL bCloseReq;
										/* �I���v���t���O */
extern LONG lCharWidth;
										/* �L�����N�^���� */
extern LONG lCharHeight;
										/* �L�����N�^�c�� */
extern BOOL bSync;
										/* ���s�ɓ��� */
extern BOOL bSyncDisasm[4];
										/* �t�A�Z���u����PC�ɓ��� */
extern BOOL bMinimize;
										/* �ŏ����t���O */
extern BOOL bActivate;
										/* �A�N�e�B�x�[�g�t���O */
extern BOOL bHideStatus;
										/* �X�e�[�^�X�o�[���B�� */
extern HICON hAppIcon;
										/* �A�C�R���n���h�� */
extern int nAppIcon;
										/* �A�C�R���ԍ�(1�`3) */
extern BOOL bNTflag;
										/* ����OS�^�C�v1(NT) */
extern BOOL bXPflag;
										/* ����OS�^�C�v2(XP) */
extern BOOL bVistaflag;
										/* ����OS�^�C�v3(Vista) */
extern BOOL bWin7flag;
										/* ����OS�^�C�v4(Windows 7) */
extern BOOL bWin8flag;
										/* ����OS�^�C�v4(Windows 8) */
extern BOOL bWin10flag;
										/* ����OS�^�C�v3(Vista/7) */
extern BOOL bMMXflag;
										/* MMX�Ή��t���O */
extern BOOL bCMOVflag;
										/* CMOV�Ή��t���O */
extern BOOL bHighPriority;
										/* �n�C�v���C�I���e�B�t���O */
extern POINT WinPos;
										/* �E�B���h�E�ʒu */
extern BOOL bOFNCentering;
										/* �t�@�C���_�C�A���O�̃Z���^�����O */
#if defined(ROMEO)
extern BOOL bRomeo;
										/* ROMEO�F���t���O */
#endif
#if ((XM7_VER <= 2) && defined(FMTV151))
extern BOOL bFMTV151;
										/* �`�����l���R�[���G�~�����[�V���� */
#endif
extern HFONT hFont;
										/* �T�u�E�B���h�E�p�t�H���g�n���h�� */
#if XM7_VER >= 3
extern BOOL bGravestone;
										/* !? */
#endif
#if defined(KBDPASTE)
extern HWND hKeyStrokeDialog;
										/* �L�[���͎x���_�C�A���O�n���h�� */
extern BOOL bKeyStrokeModeless;
										/* �L�[���͎x���_�C�A���O���[�h���X�t���O */
#endif
#ifdef __cplusplus
}
#endif

#endif	/* _w32_h_ */
#endif	/* _WIN32 */
