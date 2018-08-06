/*
 *	FM-7 EMULATOR "XM7"
 *
 *	Copyright (C) 1999-2017 �o�h�D(Twitter:@xm6_original)
 *	Copyright (C) 2001-2017 Ryu Takegami (Twitter:@RyuTakegami)
 *	line printer support 2010-2013 by Ben.JP
 *
 *	[ Win32API ���j���[�R�}���h ]
 *
 *	RHG����
 *	  2001.08.01		V2�ł̐V�K2DD�f�B�X�N�C���[�W�쐬�@�\��p�~
 *	  2001.08.08		�z�b�g���Z�b�g�@�\��ǉ�
 *	  2002.01.24		�����A�W���X�g�@�\�ł�VM���b�N������p�~
 *	  2002.03.03		Alt+F11�̘A�łŗ����邱�Ƃ���������C��
 *	  2002.05.04		F-BASIC���[�U�f�B�X�N�쐬�@�\��ǉ�
 *						2D/VFD�ϊ��ƐV�KD77�쐬�̃_�C�A���O��Ɨ���
 *	  2002.05.06		�N�����[�h�؂芷������FM-7���[�h�����̓��Z�b�g����Ȃ�
 *						�悤�ɕύX
 *	  2002.05.07		�_�����Z/������ԃE�B���h�E�p������ǉ�
 *	  2002.05.23		�_�����Z/������ԃE�B���h�E�Ƀ`�F�b�N������Ȃ�����
 *						�C��
 *	  2002.06.15		�X�e�[�g�t�@�C���̓���@���񂪔��f����Ȃ������C��
 *	  2002.07.10		�T�u�E�B���h�E��S�ĉB��/��������@�\��ǉ�
 *	  2002.09.13		�t�A�Z���u���E�B���h�E��PC�񓯊����[�h���Ɂu�ŐV�̏��
 *						�ɍX�V�v��I������ƕ\���A�h���X���ς������C��
 *	  2002.10.21		�g���[�X���͋���PC�����ɂȂ�悤�ɕύX
 *	  2003.03.09		�t�@�C���I���f�t�H���g�f�B���N�g���̎�ޕʕۑ��ɑΉ�
 *	  2004.05.03		�t���X�N���[�����Ɏ����ŃT�u�E�B���h�E���B���悤�ɕύX
 *						�t���X�N���[�����ɃT�u�E�B���h�E�֘A���j���[�𖳌�����
 *						��悤�ɕύX
 *	  2004.05.04		�t�@�C���I���_�C�A���O�̏����ʒu����ɃE�B���h�E������
 *						�Ȃ�悤�ɕύX
 *	  2004.05.28		�X�e�[�^�X�o�[�̃f�o�C�X�ʃt�@�C���h���b�v�ɑΉ�
 *	  2004.10.05		�f�B�X�N�C���[�W����"&"���܂܂�Ă���ƃ��j���[������
 *						�Ƀv���t�B�b�N�X�Ƃ��ď�������Ă��܂������C��
 *		(����)
 *	  2012.12.04		V2/V3�ɂ�����BASIC/DOS���[�h�̐؂�ւ��̍ۂ̃��Z�b�g
 *						������p�~
 */

#ifdef _WIN32

#define STRICT
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <commdlg.h>
#include <shellapi.h>
#include <stdlib.h>
#include <assert.h>
#include <tchar.h>
#define DIRECTINPUT_VERSION		0x0300		/* DirectX3���w�� */
#include <dinput.h>
#include <direct.h>
#include "xm7.h"
#include "fdc.h"
#include "tapelp.h"
#include "tools.h"
#include "mouse.h"
#include "rtc.h"
#include "display.h"
#include "subctrl.h"
#include "jsubsys.h"
#include "bubble.h"
#include "w32.h"
#include "w32_bar.h"
#include "w32_draw.h"
#include "w32_snd.h"
#include "w32_sch.h"
#include "w32_sub.h"
#include "w32_cfg.h"
#include "w32_res.h"
#include "w32_kbd.h"
#include "w32_comm.h"

/*
 *	�O���[�o�� ���[�N
 */
#if XM7_VER == 1
#if defined(BUBBLE)
char InitialDir[6][_MAX_DRIVE + _MAX_PATH];
#else
char InitialDir[5][_MAX_DRIVE + _MAX_PATH];
#endif
#else
char InitialDir[5][_MAX_DRIVE + _MAX_PATH];
#endif
BOOL bOFNCentering = TRUE;
#if defined(KBDPASTE)
HWND hKeyStrokeDialog;			/* �L�[���͎x���_�C�A���O�n���h�� */
BOOL bKeyStrokeModeless;		/* �L�[���͎x���_�C�A���O���[�h���X�t���O */
#endif

/*
 *	�X�^�e�B�b�N ���[�N
 */
static char StatePath[_MAX_PATH];
static char DiskTitle[16 + 1];
static BOOL DiskMedia;
static BOOL DiskFormat;
#if XM7_VER == 1 
#if defined(BUBBLE)
static char BubbleTitle[16 + 1];
static BOOL BubbleFormat;
#endif
#endif
#if defined(KBDPASTE)
static char KeyStrokeString[256];
static BOOL bImmStatus;
static DWORD dwConversion;
static DWORD dwSentence;
#endif

/*
 *	�v���g�^�C�v�錾
 */
extern int _getdrive(void);

/*
 *	ENTER/ESC/SPACE����҂��}�N��
 */
#define WAIT_KEY_POP()		while (	(GetAsyncKeyState(VK_RETURN) & 0x8000) || \
									(GetAsyncKeyState(VK_ESCAPE) & 0x8000) || \
									(GetAsyncKeyState(VK_SPACE) & 0x8000))

/*-[ �^�C�g�����̓_�C�A���O ]------------------------------------------------*/

/*
 *	�^�C�g�����̓_�C�A���O
 *	�_�C�A���O������
 */
static BOOL FASTCALL TitleDlgInit(HWND hDlg)
{
	HWND hWnd;
	RECT prect;
	RECT drect;

	ASSERT(hDlg);

	/* �e�E�C���h�E�̒����ɐݒ� */
	hWnd = GetParent(hDlg);
	GetWindowRect(hWnd, &prect);
	GetWindowRect(hDlg, &drect);
	drect.right -= drect.left;
	drect.bottom -= drect.top;
	drect.left = (prect.right - prect.left) / 2 + prect.left;
	drect.left -= (drect.right / 2);
	drect.top = (prect.bottom - prect.top) / 2 + prect.top;
	drect.top -= (drect.bottom / 2);
	MoveWindow(hDlg, drect.left, drect.top, drect.right, drect.bottom, FALSE);

	/* �G�f�B�b�g�e�L�X�g���� */
	hWnd = GetDlgItem(hDlg, IDC_TITLEEDIT);
	ASSERT(hWnd);
	strncpy(DiskTitle, "Default", sizeof(DiskTitle));
	SetWindowText(hWnd, DiskTitle);

#if XM7_VER >= 3
	/* ���f�B�A�^�C�v */
	CheckDlgButton(hDlg, IDC_TITLE2D, BST_CHECKED);
#endif

	return TRUE;
}

/*
 *	�^�C�g�����̓_�C�A���O
 *	�_�C�A���OOK
 */
static void FASTCALL TitleDlgOK(HWND hDlg)
{
	HWND hWnd;
	char string[128];

	ASSERT(hDlg);

	/* �G�f�B�b�g�e�L�X�g���� */
	hWnd = GetDlgItem(hDlg, IDC_TITLEEDIT);
	ASSERT(hWnd);

	/* ��������擾�A�R�s�[ */
	GetWindowText(hWnd, string, sizeof(string) - 1);
	memset(DiskTitle, 0, sizeof(DiskTitle));
	string[16] = '\0';
	strncpy(DiskTitle, string, sizeof(DiskTitle));

	/* ���f�B�A�^�C�v�擾 */
#if XM7_VER >= 3
	if (IsDlgButtonChecked(hDlg, IDC_TITLE2DD)) {
		DiskMedia = TRUE;
	}
	else {
		DiskMedia = FALSE;
	}
#else
	/* V2�ł�2D�̂� */
	DiskMedia = FALSE;
#endif

	if (IsDlgButtonChecked(hDlg, IDC_TITLEUSRDISK)) {
		DiskFormat = TRUE;
	}
	else {
		DiskFormat = FALSE;
	}
}

/*
 *	�^�C�g�����̓_�C�A���O
 *	�_�C�A���O�v���V�[�W��
 */
static BOOL CALLBACK TitleDlgProc(HWND hDlg, UINT iMsg,
									WPARAM wParam, LPARAM lParam)
{
	UNUSED(lParam);

	switch (iMsg) {
		/* �_�C�A���O������ */
		case WM_INITDIALOG:
			return TitleDlgInit(hDlg);

		/* �R�}���h���� */
		case WM_COMMAND:
			switch (LOWORD(wParam)) {
				/* OK�E�L�����Z�� */
				case IDOK:
				case IDCANCEL:
					if (LOWORD(wParam) == IDOK) {
						TitleDlgOK(hDlg);
					}
					EndDialog(hDlg, LOWORD(wParam));
					InvalidateRect(hDrawWnd, NULL, FALSE);
					SetMenuExitTimer();
					return TRUE;
			}
			break;
	}

	/* ����ȊO�́AFALSE */
	return FALSE;
}

/*-[ �^�C�g�����̓_�C�A���O(2D/VFD�ϊ��p) ]----------------------------------*/

/*
 *	�^�C�g�����̓_�C�A���O(2D/VFD�ϊ��p)
 *	�_�C�A���O������
 */
static BOOL FASTCALL TitleDlg2DInit(HWND hDlg)
{
	HWND hWnd;
	RECT prect;
	RECT drect;

	ASSERT(hDlg);

	/* �e�E�C���h�E�̒����ɐݒ� */
	hWnd = GetParent(hDlg);
	GetWindowRect(hWnd, &prect);
	GetWindowRect(hDlg, &drect);
	drect.right -= drect.left;
	drect.bottom -= drect.top;
	drect.left = (prect.right - prect.left) / 2 + prect.left;
	drect.left -= (drect.right / 2);
	drect.top = (prect.bottom - prect.top) / 2 + prect.top;
	drect.top -= (drect.bottom / 2);
	MoveWindow(hDlg, drect.left, drect.top, drect.right, drect.bottom, FALSE);

	/* �G�f�B�b�g�e�L�X�g���� */
	hWnd = GetDlgItem(hDlg, IDC_TITLEEDIT2D);
	ASSERT(hWnd);
	strncpy(DiskTitle, "Default", sizeof(DiskTitle));
	SetWindowText(hWnd, DiskTitle);

	return TRUE;
}

/*
 *	�^�C�g�����̓_�C�A���O(2D/VFD�ϊ��p)
 *	�_�C�A���OOK
 */
static void FASTCALL TitleDlg2DOK(HWND hDlg)
{
	HWND hWnd;
	char string[128];

	ASSERT(hDlg);

	/* �G�f�B�b�g�e�L�X�g���� */
	hWnd = GetDlgItem(hDlg, IDC_TITLEEDIT2D);
	ASSERT(hWnd);

	/* ��������擾�A�R�s�[ */
	GetWindowText(hWnd, string, sizeof(string) - 1);
	memset(DiskTitle, 0, sizeof(DiskTitle));
	string[16] = '\0';
	strncpy(DiskTitle, string, sizeof(DiskTitle));
}

/*
 *	�^�C�g�����̓_�C�A���O(2D/VFD�ϊ��p)
 *	�_�C�A���O�v���V�[�W��
 */
static BOOL CALLBACK TitleDlg2DProc(HWND hDlg, UINT iMsg,
									WPARAM wParam, LPARAM lParam)
{
	UNUSED(lParam);

	switch (iMsg) {
		/* �_�C�A���O������ */
		case WM_INITDIALOG:
			return TitleDlg2DInit(hDlg);

		/* �R�}���h���� */
		case WM_COMMAND:
			switch (LOWORD(wParam)) {
				/* OK�E�L�����Z�� */
				case IDOK:
				case IDCANCEL:
					if (LOWORD(wParam) == IDOK) {
						TitleDlg2DOK(hDlg);
					}
					EndDialog(hDlg, LOWORD(wParam));
					InvalidateRect(hDrawWnd, NULL, FALSE);
					SetMenuExitTimer();
					return TRUE;
			}
			break;
	}

	/* ����ȊO�́AFALSE */
	return FALSE;
}

/*-[ �o�u���ϊ��^�C�v�I���_�C�A���O ]----------------------------------------*/

#if XM7_VER == 1 
#if defined(BUBBLE)
/*
 *	�o�u���ϊ��^�C�v�I���_�C�A���O
 *	�_�C�A���O������
 */
static BOOL FASTCALL BubbleMediaTypeDlgInit(HWND hDlg)
{
	HWND hWnd;
	RECT prect;
	RECT drect;

	ASSERT(hDlg);

	/* �e�E�C���h�E�̒����ɐݒ� */
	hWnd = GetParent(hDlg);
	GetWindowRect(hWnd, &prect);
	GetWindowRect(hDlg, &drect);
	drect.right -= drect.left;
	drect.bottom -= drect.top;
	drect.left = (prect.right - prect.left) / 2 + prect.left;
	drect.left -= (drect.right / 2);
	drect.top = (prect.bottom - prect.top) / 2 + prect.top;
	drect.top -= (drect.bottom / 2);
	MoveWindow(hDlg, drect.left, drect.top, drect.right, drect.bottom, FALSE);

	/* �G�f�B�b�g�e�L�X�g���� */
	hWnd = GetDlgItem(hDlg, IDC_TITLEEDIT);
	ASSERT(hWnd);
	SetWindowText(hWnd, BubbleTitle);

	/* �t�H�[�}�b�g�^�C�v */
	CheckDlgButton(hDlg, IDC_MEDIAFORMATB77, BST_CHECKED);

	return TRUE;
}

/*
 *	�o�u���ϊ��^�C�v�I���_�C�A���O
 *	�_�C�A���OOK
 */
static void FASTCALL BubbleMediaTypeDlgOK(HWND hDlg)
{
	ASSERT(hDlg);

	if (IsDlgButtonChecked(hDlg, IDC_MEDIAFORMATBBL)) {
		BubbleFormat = FALSE;
	}
	else {
		BubbleFormat = TRUE;
	}
}

/*
 *	�o�u���ϊ��^�C�v�I���_�C�A���O
 *	�_�C�A���O�v���V�[�W��
 */
static BOOL CALLBACK BubbleMediaTypeDlgProc(HWND hDlg, UINT iMsg,
									WPARAM wParam, LPARAM lParam)
{
	UNUSED(lParam);

	switch (iMsg) {
		/* �_�C�A���O������ */
		case WM_INITDIALOG:
			return BubbleMediaTypeDlgInit(hDlg);

		/* �R�}���h���� */
		case WM_COMMAND:
			switch (LOWORD(wParam)) {
				/* OK�E�L�����Z�� */
				case IDOK:
				case IDCANCEL:
					if (LOWORD(wParam) == IDOK) {
						BubbleMediaTypeDlgOK(hDlg);
					}
					EndDialog(hDlg, LOWORD(wParam));
					InvalidateRect(hDrawWnd, NULL, FALSE);
					SetMenuExitTimer();
					return TRUE;
				case IDC_MEDIAFORMATB77:
					EnableWindow(GetDlgItem(hDlg, IDC_TITLEEDIT), TRUE);
					return TRUE;
				case IDC_MEDIAFORMATBBL:
					EnableWindow(GetDlgItem(hDlg, IDC_TITLEEDIT), FALSE);
					return TRUE;
			}
			break;
	}

	/* ����ȊO�́AFALSE */
	return FALSE;
}
#endif	/* defined(BUBBLE) */
#endif	/* XM7_VER == 1 */

/*-[ �t�@�C���I���R�����_�C�A���O ]-----------------------------------------*/

/*
 *	�t�@�C���I���R�����_�C�A���O
 *	������
 */
static void FASTCALL FileDialogInit(HWND hDlg)
{
	RECT drect;
	RECT prect;
	HWND hWnd;

	/* �_�C�A���O�����C���E�C���h�E�̒����Ɋ񂹂� */
	hWnd = GetParent(hDlg);
	GetWindowRect(hMainWnd, &prect);
	GetWindowRect(hWnd, &drect);
	drect.right -= drect.left;
	drect.bottom -= drect.top;
	drect.left = (prect.right - prect.left) / 2 + prect.left;
	drect.left -= (drect.right / 2);
	drect.top = (prect.bottom - prect.top) / 2 + prect.top;
	drect.top -= (drect.bottom / 2);
	MoveWindow(hWnd, drect.left, drect.top, drect.right, drect.bottom, FALSE);
}

/*
 *	�t�@�C���I���R�����_�C�A���O
 *	HOOK�֐�
 */
static UINT CALLBACK FileSelectHook(
						HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	UNUSED(wParam);
	UNUSED(lParam);

	switch (uMsg) {
		case WM_INITDIALOG:
			FileDialogInit(hDlg);
			return 0;
	}

	return 0;
}

/*
 *	�t�@�C���I���R�����_�C�A���O
 */
BOOL FASTCALL FileSelectSub(BOOL bOpen, UINT uFilterID, char *path, char *defext, BYTE IniDirNo)
{
	OPENFILENAME ofn;
	char dir[_MAX_DIR];
	char fname[_MAX_FNAME + _MAX_EXT];
	char filter[4096];
	int i, j;

	ASSERT((bOpen == TRUE) || (bOpen == FALSE));
	ASSERT(uFilterID > 0);
	ASSERT(path);

	/* �f�[�^�쐬 */
	memset(&ofn, 0, sizeof(ofn));
	memset(path, 0, _MAX_PATH);
	memset(fname, 0, sizeof(fname));
	ofn.lStructSize = 76;	/* sizeof(ofn)��V5�g�����܂� */
	ofn.hwndOwner = hMainWnd;

	LoadString(hAppInstance, uFilterID, filter, sizeof(filter));
	j = strlen(filter);
	for (i=0; i<j; i++) {
		if (filter[i] == '|') {
			filter[i] = '\0';
		}
	}

	ofn.lpstrFilter = filter;
	ofn.lpstrFile = path;
	ofn.nMaxFile = _MAX_PATH;
	ofn.lpstrFileTitle = fname;
	ofn.nMaxFileTitle = sizeof(fname);
	ofn.lpstrDefExt = defext;
	ofn.lpstrInitialDir = InitialDir[IniDirNo];
	ofn.lpfnHook = FileSelectHook;

	/* �R�����_�C�A���O���s */
	if (bOpen) {
		ofn.Flags = OFN_EXPLORER | OFN_HIDEREADONLY |
					OFN_FILEMUSTEXIST;
		if (bOFNCentering) {
			ofn.Flags |= OFN_ENABLEHOOK;
		}
		if (!GetOpenFileName(&ofn)) {
			SetMenuExitTimer();
			return FALSE;
		}
	}
	else {
		ofn.Flags = OFN_EXPLORER | OFN_HIDEREADONLY |
					OFN_OVERWRITEPROMPT;
		if (bOFNCentering) {
			ofn.Flags |= OFN_ENABLEHOOK;
		}
		if (!GetSaveFileName(&ofn)) {
			SetMenuExitTimer();
			return FALSE;
		}
	}

	/* �f�B���N�g����ۑ� */
	_splitpath(path, InitialDir[IniDirNo], dir, NULL, NULL);
	if (dir[strlen(dir)-1] == '\\') {
		/* �Ō�̃p�X��؂�L���͋����I�ɍ�� */
		dir[strlen(dir)-1] = '\0';
	}
	strncat(InitialDir[IniDirNo], dir,
			sizeof(InitialDir[IniDirNo]) - strlen(InitialDir[IniDirNo]) - 1);

	SetMenuExitTimer();
	return TRUE;
}

/*-[ �L�[���͎x���_�C�A���O ]-----------------------------------------------*/

#if defined(KBDPASTE)
/*
 *	�L�[���͎x���_�C�A���O
 *	�_�C�A���O������
 */
static BOOL FASTCALL KeyStrokeDlgInit(HWND hDlg)
{
	HWND hWnd;
	RECT prect;
	RECT drect;

	ASSERT(hDlg);

	/* �e�E�C���h�E�̒����ɐݒ� */
	hWnd = GetParent(hDlg);
	GetWindowRect(hWnd, &prect);
	GetWindowRect(hDlg, &drect);
	drect.right -= drect.left;
	drect.bottom -= drect.top;
	drect.left = (prect.right - prect.left) / 2 + prect.left;
	drect.left -= (drect.right / 2);
	drect.top = (prect.bottom - prect.top) / 2 + prect.top;
	drect.top -= (drect.bottom / 2);
	MoveWindow(hDlg, drect.left, drect.top, drect.right, drect.bottom, FALSE);

	/* ������ */
	memset(KeyStrokeString, 0, sizeof(KeyStrokeString));

	return TRUE;
}

/*
 *	�L�[���͎x���_�C�A���O
 *	�_�C�A���OOK
 */
static void FASTCALL KeyStrokeDlgOK(HWND hDlg)
{
	HWND hWnd;

	ASSERT(hDlg);

	/* �G�f�B�b�g�e�L�X�g���� */
	hWnd = GetDlgItem(hDlg, IDC_KEYSTROKEEDIT);
	ASSERT(hWnd);

	/* ��������擾�A�R�s�[ */
	GetWindowText(hWnd, (LPTSTR)KeyStrokeString, sizeof(KeyStrokeString) - 1);
	SetWindowText(hWnd, (LPTSTR)"");
}

/*
 *	�L�[���͎x���_�C�A���O
 *	�_�C�A���O�v���V�[�W��
 */
static BOOL CALLBACK KeyStrokeDlgProc(HWND hDlg, UINT iMsg,
									WPARAM wParam, LPARAM lParam)
{
	HWND hWnd;
	HIMC hImm;

	UNUSED(lParam);

	switch (iMsg) {
		/* �_�C�A���O������ */
		case WM_INITDIALOG:
			/* �G�f�B�b�g�e�L�X�g���� */
			hWnd = GetDlgItem(hDlg, IDC_KEYSTROKEEDIT);
			ASSERT(hWnd);

		/* IMM�ݒ�𔼊p�J�i�ɐݒ� */
			hImm = ImmGetContext(hWnd);
			bImmStatus = ImmGetOpenStatus(hImm);
			ImmGetConversionStatus(hImm, &dwConversion, &dwSentence);
			ImmSetOpenStatus(hImm, TRUE);
			ImmSetConversionStatus(hImm,
					IME_CMODE_NATIVE | IME_CMODE_KATAKANA |
					IME_CMODE_ROMAN, IME_SMODE_NONE);
			ImmReleaseContext(hWnd, hImm);		/* �g�p��͉������ */

			return KeyStrokeDlgInit(hDlg);

		/* �R�}���h���� */
		case WM_COMMAND:
			switch (LOWORD(wParam)) {
				/* OK�E�L�����Z�� */
				case IDOK:
				case IDCANCEL:
					WAIT_KEY_POP();
					if (LOWORD(wParam) == IDOK) {
						KeyStrokeDlgOK(hDlg);
						if (strlen(KeyStrokeString) == 0) {
							break;
						}
						if (hKeyStrokeDialog) {
							/* �쐬 */
							LockVM();
							PasteKbd((char *)KeyStrokeString);
							UnlockVM();
							SetForegroundWindow(hMainWnd);
							break;
						}
					}

					/* �G�f�B�b�g�e�L�X�g���� */
					hWnd = GetDlgItem(hDlg, IDC_KEYSTROKEEDIT);
					ASSERT(hWnd);

					/* IMM�ݒ�̕��A */
					hImm = ImmGetContext(hWnd);
					ImmSetOpenStatus(hImm, bImmStatus);
					ImmSetConversionStatus(hImm, dwConversion, dwSentence);
					ImmReleaseContext(hWnd, hImm);

					if (hKeyStrokeDialog) {
						DestroyWindow(hKeyStrokeDialog);
						hKeyStrokeDialog = NULL;
					}
					else {
						EndDialog(hDlg, LOWORD(wParam));
					}

					InvalidateRect(hDrawWnd, NULL, FALSE);
					SetMenuExitTimer();
					return TRUE;
			}
			break;
	}

	/* ����ȊO�́AFALSE */
	return FALSE;
}
#endif

/*-[ �ėp�T�u ]-------------------------------------------------------------*/

/*
 *	���j���[Enable
 */
void FASTCALL EnableMenuSub(HMENU hMenu, UINT uID, BOOL bEnable)
{
	ASSERT(hMenu);
	ASSERT(uID > 0);

	if (bEnable) {
		EnableMenuItem(hMenu, uID, MF_BYCOMMAND | MF_ENABLED);
	}
	else {
		EnableMenuItem(hMenu, uID, MF_BYCOMMAND | MF_GRAYED);
	}
}

/*
 *	���j���[Enable (�ʒu�w��)
 */
#if XM7_VER == 1
#if defined(BUBBLE)
void FASTCALL EnableMenuPos(HMENU hMenu, UINT uPos, BOOL bEnable)
{
	ASSERT(hMenu);
	ASSERT(uPos > 0);

	if (bEnable) {
		EnableMenuItem(hMenu, uPos, MF_BYPOSITION | MF_ENABLED);
	}
	else {
		EnableMenuItem(hMenu, uPos, MF_BYPOSITION | MF_GRAYED);
	}
}
#endif
#endif

/*
 *	���j���[Check
 */
void FASTCALL CheckMenuSub(HMENU hMenu, UINT uID, BOOL bCheck)
{
	ASSERT(hMenu);
	ASSERT(uID > 0);

	if (bCheck) {
		CheckMenuItem(hMenu, uID, MF_BYCOMMAND | MF_CHECKED);
	}
	else {
		CheckMenuItem(hMenu, uID, MF_BYCOMMAND | MF_UNCHECKED);
	}
}

/*-[ �t�@�C�����j���[ ]-----------------------------------------------------*/

/*
 *	�X�e�[�g���[�h����
 */
static void FASTCALL StateLoad(char *path)
{
	char string[256];
	int state;

	state = system_load(path);
	if (state == STATELOAD_ERROR) {
		/* �{�̓ǂݍ��ݒ��̃G���[�������̂݃��Z�b�g */
		system_reset();
		OnRefresh(hMainWnd);
	}
	if (state != STATELOAD_SUCCESS) {
		LoadString(hAppInstance, IDS_STATEERROR, string, sizeof(string));
		MessageBox(hMainWnd, string, "XM7", MB_ICONSTOP | MB_OK);
		SetMenuExitTimer();
	}
	else {
		strncpy(StatePath, path, sizeof(StatePath));
		OnRefresh(hMainWnd);
	}

	/* �����E���s�ǂ���̃P�[�X�ł��^�[�Q�b�g�@���ύX���� */
	GetCfg();
	SetMachineVersion();
}

/*-[ �t�@�C�����j���[ ]-----------------------------------------------------*/

/*
 *	�J��(O)
 */
static void FASTCALL OnOpen(HWND hWnd)
{
	char path[_MAX_PATH];

	ASSERT(hWnd);

	/* �t�@�C���I���T�u */
	if (!FileSelectSub(TRUE, IDS_STATEFILTER, path, NULL, 2)) {
		return;
	}

	/* �X�e�[�g���[�h */
	LockVM();
	StopSnd();
	StateLoad(path);
	PlaySnd();
	ResetSch();
	UnlockVM();

	/* ��ʍĕ`�� */
	OnRefresh(hWnd);
}

/*
 *	���O��t���ĕۑ�(A)
 */
static void FASTCALL OnSaveAs(HWND hWnd)
{
	char path[_MAX_PATH];

	ASSERT(hWnd);

	/* �t�@�C���I���T�u */
	if (!FileSelectSub(FALSE, IDS_STATEFILTER, path, "XM7", 2)) {
		return;
	}

	/* �X�e�[�g�Z�[�u */
	LockVM();
	StopSnd();
	if (!system_save(path)) {
		LoadString(hAppInstance, IDS_STATEERROR, path, sizeof(path));
		MessageBox(hWnd, path, "XM7", MB_ICONSTOP | MB_OK);
		SetMenuExitTimer();
	}
	else {
		strncpy(StatePath, path, sizeof(StatePath));
	}
	PlaySnd();
	ResetSch();
	UnlockVM();
}

/*
 *	�㏑���ۑ�(S)
 */
static void FASTCALL OnSave(HWND hWnd)
{
	char string[128];

	/* �܂��ۑ�����Ă��Ȃ���΁A���O������ */
	if (StatePath[0] == '\0') {
		OnSaveAs(hWnd);
		return;
	}

	/* �X�e�[�g�Z�[�u */
	LockVM();
	StopSnd();
	if (!system_save(StatePath)) {
		LoadString(hAppInstance, IDS_STATEERROR, string, sizeof(string));
		MessageBox(hWnd, string, "XM7", MB_ICONSTOP | MB_OK);
		SetMenuExitTimer();
	}
	PlaySnd();
	ResetSch();
	UnlockVM();
}

#if defined(LPRINT)
/*
 *	���(P)
 */
static void FASTCALL OnPrint(void)
{
	lp_print();
}
#endif

/*
 *	���Z�b�g(R)
 */
static void FASTCALL OnReset(HWND hWnd)
{
	LockVM();
	system_reset();
	ResetSch();
	UnlockVM();

	/* �ĕ`�� */
	OnRefresh(hWnd);
}

/*
 *	�z�b�g���Z�b�g(H)
 */
static void FASTCALL OnHotReset(HWND hWnd)
{
	LockVM();
	system_hotreset();
	UnlockVM();

	/* �ĕ`�� */
	OnRefresh(hWnd);
}

/*
 *	TAB+���Z�b�g(F)
 */
#if XM7_VER >= 3
static void FASTCALL OnTabReset(HWND hWnd)
{
	LockVM();
	system_tabreset();
	UnlockVM();

	/* �ĕ`�� */
	OnRefresh(hWnd);
}
#endif

/*
 *	BASIC���[�h(B)
 */
static void FASTCALL OnBasic(void)
{
	LockVM();
	boot_mode = BOOT_BASIC;
	GetCfg();
#if XM7_VER >= 2
	if (fm7_ver < 2) {
		mainmem_transfer_boot();
	}
#else
	if (fm_subtype == FMSUB_FM8) {
		basicrom_en = TRUE;
	}
#endif
	UnlockVM();
}

/*
 *	DOS���[�h(D)
 */
static void FASTCALL OnDos(void)
{
	LockVM();
	boot_mode = BOOT_DOS;
	GetCfg();
#if XM7_VER >= 2
	if (fm7_ver < 2) {
		mainmem_transfer_boot();
	}
#else
	if (fm_subtype == FMSUB_FM8) {
		basicrom_en = FALSE;
	}
#endif
	UnlockVM();
}

/*
 *	�o�u�����[�h(U)
 */
#if XM7_VER == 1 
#if defined(BUBBLE)
static void FASTCALL OnBubble(void)
{
	LockVM();
	boot_mode = BOOT_BUBBLE;
	GetCfg();
	if (fm_subtype == FMSUB_FM8) {
		basicrom_en = FALSE;
	}
	UnlockVM();
}
#endif
#endif

/*
 *	�I��(X)
 */
static void FASTCALL OnExit(HWND hWnd)
{
	/* �E�C���h�E�N���[�Y */
	PostMessage(hWnd, WM_CLOSE, 0, 0);
}

/*
 *	�t�@�C��(F)���j���[
 */
static BOOL OnFile(HWND hWnd, WORD wID)
{
	ASSERT(hWnd);

	switch (wID) {
		/* �I�[�v�� */
		case IDM_OPEN:
			OnOpen(hWnd);
			return TRUE;

		/* �㏑���ۑ� */
		case IDM_SAVE:
			OnSave(hWnd);
			return TRUE;

		/* ���O�����ĕۑ� */
		case IDM_SAVEAS:
			OnSaveAs(hWnd);
			return TRUE;

#if defined(LPRINT)
		/* ��� */
		case IDM_PRINT:
			OnPrint();
			return TRUE;
#endif

		/* ���Z�b�g */
		case IDM_RESET:
			OnReset(hWnd);
			return TRUE;

		/* �z�b�g���Z�b�g */
		case IDM_HOTRESET:
			OnHotReset(hWnd);
			return TRUE;

#if XM7_VER >= 3
		/* TAB+���Z�b�g */
		case IDM_TABRESET:
			OnTabReset(hWnd);
			return TRUE;
#endif

		/* BASIC���[�h */
		case IDM_BASIC:
			OnBasic();
			return TRUE;

		/* DOS���[�h */
		case IDM_DOS:
			OnDos();
			return TRUE;

#if XM7_VER == 1 
#if defined(BUBBLE)
		/* �o�u�����[�h */
		case IDM_BUBBLE:
			OnBubble();
			return TRUE;
#endif
#endif

		/* �I�� */
		case IDM_EXIT:
			OnExit(hWnd);
			return TRUE;
	}

	return FALSE;
}

/*
 *	�t�@�C��(F)���j���[�X�V
 */
static void FASTCALL OnFilePopup(HMENU hMenu)
{
	UINT id;

	ASSERT(hMenu);

	switch (boot_mode) {
		case BOOT_BASIC:
			id = IDM_BASIC;
			break;
		case BOOT_DOS:
			id = IDM_DOS;
			break;
#if XM7_VER == 1 
#if defined(BUBBLE)
		case BOOT_BUBBLE:
			id = IDM_BUBBLE;
			break;
#endif
#endif
		default:
			ASSERT(FALSE);
			break;
	}

#if XM7_VER == 1
#if defined(BUBBLE)
	CheckMenuRadioItem(hMenu, IDM_BASIC, IDM_BUBBLE, id, MF_BYCOMMAND);
	EnableMenuSub(hMenu, IDM_BUBBLE, (fm_subtype == FMSUB_FM8) && bubble_available);
#else
	CheckMenuRadioItem(hMenu, IDM_BASIC, IDM_DOS, id, MF_BYCOMMAND);
#endif
#else
	CheckMenuRadioItem(hMenu, IDM_BASIC, IDM_DOS, id, MF_BYCOMMAND);
#endif

#if XM7_VER >= 3
	EnableMenuSub(hMenu, IDM_TABRESET, (fm7_ver >= 3) && init_is_exsx);
#endif

#if defined(LPRINT)
	EnableMenuSub(hMenu, IDM_PRINT, (lp_use == LP_EMULATION));
#endif
}

/*-[ �f�B�X�N���j���[ ]-----------------------------------------------------*/

/*
 *	�h���C�u���J��
 */
static void FASTCALL OnDiskOpen(int Drive)
{
	char path[_MAX_PATH];

	ASSERT((Drive == 0) || (Drive == 1));

	/* �t�@�C���I�� */
	if (!FileSelectSub(TRUE, IDS_DISKFILTER, path, NULL, 0)) {
		return;
	}

	/* �Z�b�g */
	LockVM();
	fdc_setdisk(Drive, path);
	ResetSch();
	UnlockVM();
}

/*
 *	���h���C�u���J��
 */
static void FASTCALL OnDiskBoth(void)
{
	char path[_MAX_PATH];

	/* �t�@�C���I�� */
	if (!FileSelectSub(TRUE, IDS_DISKFILTER, path, NULL, 0)) {
		return;
	}

	/* �Z�b�g */
	LockVM();
	fdc_setdisk(0, path);
	fdc_setdisk(1, NULL);
	if ((fdc_ready[0] != FDC_TYPE_NOTREADY) && (fdc_medias[0] >= 2)) {
		fdc_setdisk(1, path);
		fdc_setmedia(1, 1);
	}
	ResetSch();
	UnlockVM();
}

/*
 *	�f�B�X�N�C�W�F�N�g
 */
static void FASTCALL OnDiskEject(int Drive)
{
	ASSERT((Drive == 0) || (Drive == 1));

	/* �C�W�F�N�g */
	LockVM();
	fdc_setdisk(Drive, NULL);
	UnlockVM();
}

/*
 *	�f�B�X�N�ꎞ���o��
 */
static void FASTCALL OnDiskTemp(int Drive)
{
	ASSERT((Drive == 0) || (Drive == 1));

	/* �������݋֎~�؂�ւ� */
	LockVM();
	if (fdc_teject[Drive]) {
		fdc_teject[Drive] = FALSE;
	}
	else {
		fdc_teject[Drive] = TRUE;
	}
	UnlockVM();
}

/*
 *	�f�B�X�N�������݋֎~
 */
static void FASTCALL OnDiskProtect(int Drive)
{
	ASSERT((Drive == 0) || (Drive == 1));

	/* �������݋֎~�؂�ւ� */
	LockVM();
	if (fdc_writep[Drive]) {
		fdc_setwritep(Drive, FALSE);
	}
	else {
		fdc_setwritep(Drive, TRUE);
	}
	ResetSch();
	UnlockVM();
}

/*
 *	�f�B�X�N(1)(0)���j���[
 */
static BOOL FASTCALL OnDisk(WORD wID)
{
	switch (wID) {
		/* �J�� */
		case IDM_D0OPEN:
			OnDiskOpen(0);
			break;
		case IDM_D1OPEN:
			OnDiskOpen(1);
			break;

		/* ���h���C�u�ŊJ�� */
		case IDM_DBOPEN:
			OnDiskBoth();
			break;

		/* ���O�� */
		case IDM_D0EJECT:
			OnDiskEject(0);
			break;
		case IDM_D1EJECT:
			OnDiskEject(1);
			break;

		/* �ꎞ�C�W�F�N�g */
		case IDM_D0TEMP:
			OnDiskTemp(0);
			break;
		case IDM_D1TEMP:
			OnDiskTemp(1);
			break;

		/* �������݋֎~ */
		case IDM_D0WRITE:
			OnDiskProtect(0);
			break;
		case IDM_D1WRITE:
			OnDiskProtect(1);
			break;
	}

	/* ���f�B�A���� */
	if ((wID >= IDM_D0MEDIA00) && (wID <= IDM_D0MEDIA15)) {
		LockVM();
		fdc_setmedia(0, wID - IDM_D0MEDIA00);
		ResetSch();
		UnlockVM();
	}
	if ((wID >= IDM_D1MEDIA00) && (wID <= IDM_D1MEDIA15)) {
		LockVM();
		fdc_setmedia(1, wID - IDM_D1MEDIA00);
		ResetSch();
		UnlockVM();
	}

	return FALSE;
}

/*
 *	�f�B�X�N(1)(0)���j���[�X�V
 */
static void FASTCALL OnDiskPopup(HMENU hMenu, int Drive)
{
	MENUITEMINFO mii;
	char string[128];
	char buffer[128];
	int offset;
	int i;
	int j;
	int k;

	ASSERT(hMenu);
	ASSERT((Drive == 0) || (Drive == 1));

	/* ���j���[���ׂč폜 */
	while (GetMenuItemCount(hMenu) > 0) {
		DeleteMenu(hMenu, 0, MF_BYPOSITION);
	}

	/* �I�t�Z�b�g�m�� */
	if (Drive == 0) {
		offset = 0;
	}
	else {
		offset = IDM_D1OPEN - IDM_D0OPEN;
	}

	/* ���j���[�\���̏����� */
	memset(&mii, 0, sizeof(mii));
	mii.cbSize = 44;	/* sizeof(mii)��WINVER>=0x0500���� */
	mii.fMask = MIIM_TYPE | MIIM_STATE | MIIM_ID;
	mii.fType = MFT_STRING;
	mii.fState = MFS_ENABLED;

	/* �I�[�v���ƁA���I�[�v�� */
	mii.wID = IDM_D0OPEN + offset;
	LoadString(hAppInstance, IDS_DISKOPEN, string, sizeof(string));
	mii.dwTypeData = string;
	mii.cch = strlen(string);
	InsertMenuItem(hMenu, 0, TRUE, &mii);
	mii.wID = IDM_DBOPEN;
	LoadString(hAppInstance, IDS_DISKBOTH, string, sizeof(string));
	mii.cch = strlen(string);
	InsertMenuItem(hMenu, 1, TRUE, &mii);

	/* �f�B�X�N���}������Ă��Ȃ���΁A�����܂� */
	if (fdc_ready[Drive] == FDC_TYPE_NOTREADY) {
		return;
	}

	/* �C�W�F�N�g */
	mii.wID = IDM_D0EJECT + offset;
	LoadString(hAppInstance, IDS_DISKEJECT, string, sizeof(string));
	mii.cch = strlen(string);
	InsertMenuItem(hMenu, 2, TRUE, &mii);

	/* �Z�p���[�^�}�� */
	mii.fType = MFT_SEPARATOR;
	InsertMenuItem(hMenu, 3, TRUE, &mii);

	/* �ꎞ���o�� */
	mii.wID = IDM_D0TEMP + offset;
	LoadString(hAppInstance, IDS_DISKTEMP, string, sizeof(string));
	if (fdc_teject[Drive]) {
		mii.fState = MFS_CHECKED | MFS_ENABLED;
	}
	else {
		mii.fState = MFS_ENABLED;
	}
	mii.fType = MFT_STRING;
	mii.cch = strlen(string);
	InsertMenuItem(hMenu, 4, TRUE, &mii);

	/* ���C�g�v���e�N�g */
	mii.wID = IDM_D0WRITE + offset;
	LoadString(hAppInstance, IDS_DISKPROTECT, string, sizeof(string));
	if (fdc_fwritep[Drive]) {
		mii.fState = MFS_GRAYED;
	}
	else {
		if (fdc_writep[Drive]) {
			mii.fState = MFS_CHECKED | MFS_ENABLED;
		}
		else {
			mii.fState = MFS_ENABLED;
		}
	}
	mii.fType = MFT_STRING;
	mii.cch = strlen(string);
	InsertMenuItem(hMenu, 5, TRUE, &mii);

	/* �Z�p���[�^�}�� */
	mii.fType = MFT_SEPARATOR;
	InsertMenuItem(hMenu, 6, TRUE, &mii);

	/* 2D/2DD/VFD�Ȃ���ꏈ�� */
	if ((fdc_ready[Drive] == FDC_TYPE_2D) ||
#if XM7_VER >= 3
		(fdc_ready[Drive] == FDC_TYPE_2DD) ||
#endif
		(fdc_ready[Drive] == FDC_TYPE_VFD)) {
		mii.wID = IDM_D0MEDIA00 + offset;
		mii.fState = MFS_CHECKED | MFS_ENABLED;
		mii.fType = MFT_STRING | MFT_RADIOCHECK;
		if (fdc_ready[Drive] == FDC_TYPE_2D) {
			LoadString(hAppInstance, IDS_DISK2D, string, sizeof(string));
		}
#if XM7_VER >= 3
		else if (fdc_ready[Drive] == FDC_TYPE_2DD) {
			LoadString(hAppInstance, IDS_DISK2DD, string, sizeof(string));
		}
#endif
		else {
			LoadString(hAppInstance, IDS_DISKVFD, string, sizeof(string));
		}
		mii.cch = strlen(string);
		InsertMenuItem(hMenu, 7, TRUE, &mii);
		return;
	}

	/* ���f�B�A���� */
	for (i=0; i<fdc_medias[Drive]; i++) {
		mii.wID = IDM_D0MEDIA00 + offset + i;
		if (fdc_media[Drive] == i) {
			mii.fState = MFS_CHECKED | MFS_ENABLED;
		}
		else {
			mii.fState = MFS_ENABLED;
		}
		mii.fType = MFT_STRING | MFT_RADIOCHECK;
		if (strlen(fdc_name[Drive][i]) == 0) {
			LoadString(hAppInstance, IDS_MEDIA_NAME, buffer, sizeof(buffer));
			/* 128�o�C�g�𒴂��Ȃ��Ƃ͎v���̂����c */
			_snprintf(string, sizeof(string), buffer, i + 1);
		}
		else {
			/* �v���t�B�b�N�X�����΍� */
			k = 0;
			for (j=0; j<(int)strlen(fdc_name[Drive][i]); j++) {
				if (fdc_name[Drive][i][j] == '&') {
					string[k++] = '&';
				}
				string[k++] = fdc_name[Drive][i][j];
			}
			string[k] = '\0';
		}
		mii.cch = strlen(string);
		InsertMenuItem(hMenu, 7 + i, TRUE, &mii);
	}
}

/*-[ �e�[�v���j���[ ]-------------------------------------------------------*/

/*
 *  �e�[�v�I�[�v��
 */
static void FASTCALL OnTapeOpen(void)
{
	char path[_MAX_PATH];

	/* �t�@�C���I�� */
	if (!FileSelectSub(TRUE, IDS_TAPEFILTER, path, NULL, 1)) {
		return;
	}

	/* �Z�b�g */
	LockVM();
	tape_setfile(path);
	ResetSch();
	UnlockVM();
}

/*
 *	�e�[�v�C�W�F�N�g
 */
static void FASTCALL OnTapeEject(void)
{
	/* �C�W�F�N�g */
	LockVM();
	tape_setfile(NULL);
	UnlockVM();
}

/*
 *	�����߂�
 */
static void FASTCALL OnRew(void)
{
	HCURSOR hCursor;

	/* �����߂� */
	hCursor = SetCursor(LoadCursor(NULL, IDC_WAIT));
	LockVM();
	StopSnd();

	tape_rew();

	PlaySnd();
	ResetSch();
	UnlockVM();
	SetCursor(hCursor);
}

/*
 *	�ŏ��܂Ŋ����߂�
 */
static void FASTCALL OnRewTop(void)
{
	HCURSOR hCursor;

	LockVM();
	StopSnd();

	tape_rewtop();

	PlaySnd();
	ResetSch();
	UnlockVM();
}

/*
 *	������
 */
static void FASTCALL OnFF(void)
{
	HCURSOR hCursor;

	/* �����߂� */
	hCursor = SetCursor(LoadCursor(NULL, IDC_WAIT));
	LockVM();
	StopSnd();

	tape_ff();

	PlaySnd();
	ResetSch();
	UnlockVM();
	SetCursor(hCursor);
}

/*
 *	�^��
 */
static void FASTCALL OnRec(void)
{
	/* �^�� */
	LockVM();
	if (tape_rec) {
		tape_setrec(FALSE);
	}
	else {
		tape_setrec(TRUE);
	}
	UnlockVM();
}

#if XM7_VER == 1
#if defined(BUBBLE)
/*
 *  �o�u���I�[�v��
 */
static void FASTCALL OnBubbleOpen(int unit)
{
	char path[_MAX_PATH];

	ASSERT ((unit == 0) || (unit == 1));

	/* �t�@�C���I�� */
	if (!FileSelectSub(TRUE, IDS_BUBBLEFILTER, path, NULL, 5)) {
		return;
	}

	/* �Z�b�g */
	LockVM();
	bmc_setfile(unit, path);
	bmc_setmedia(unit, 0);
	ResetSch();
	UnlockVM();
}


/*
 *  �����j�b�g�ŊJ��
 */
static void FASTCALL OnBubbleBoth(int unit)
{
	char path[_MAX_PATH];

	ASSERT((unit == 0) || (unit == 1));

	/* �t�@�C���I�� */
	if (!FileSelectSub(TRUE, IDS_BUBBLEFILTER, path, NULL, 5)) {
		return;
	}

	/* �Z�b�g */
	LockVM();
	bmc_setfile(unit + 0, path);
	bmc_setfile(unit + 1, NULL);
	bmc_setmedia(unit + 0, 0);
	if ((bmc_ready[unit] != BMC_TYPE_NOTREADY) && (bmc_medias[unit] >= 2)) {
		bmc_setfile(unit + 1, path);
		bmc_setmedia(unit + 1, 1);
	}
	ResetSch();
	UnlockVM();
}

/*
 *	�o�u���C�W�F�N�g
 */
static void FASTCALL OnBubbleEject(int unit)
{
	ASSERT((unit == 0) || (unit == 1));

	/* �C�W�F�N�g */
	LockVM();
	bmc_setfile(unit, NULL);
	UnlockVM();
}

/*
 *	�o�u���ꎞ���o��
 */
static void FASTCALL OnBubbleTemp(int unit)
{
	ASSERT((unit == 0) || (unit == 1));

	/* �������݋֎~�؂�ւ� */
	LockVM();
	if (bmc_teject[unit]) {
		bmc_teject[unit] = FALSE;
	}
	else {
		bmc_teject[unit] = TRUE;
	}
	UnlockVM();
}

/*
 *	�o�u���������݋֎~
 */
static void FASTCALL OnBubbleProtect(int unit)
{
	ASSERT((unit == 0) || (unit == 1));

	/* �������݋֎~�؂�ւ� */
	LockVM();
	if (bmc_writep[unit]) {
		bmc_setwritep(unit, FALSE);
	}
	else {
		bmc_setwritep(unit, TRUE);
	}
	ResetSch();
	UnlockVM();
}
#endif
#endif

/*
 *	�e�[�v(A)���j���[
 */
static BOOL FASTCALL OnTape(WORD wID)
{
	switch (wID) {
		/* �J�� */
		case IDM_TOPEN:
			OnTapeOpen();
			return TRUE;

		/* ���O�� */
		case IDM_TEJECT:
			OnTapeEject();
			return TRUE;

		/* �����߂� */
		case IDM_REW:
			OnRew();
			return TRUE;

		/* �ŏ��܂Ŋ����߂� */
		case IDM_REWTOP:
			OnRewTop();
			return TRUE;

		/* ������ */
		case IDM_FF:
			OnFF();
			return TRUE;

		/* �^�� */
		case IDM_REC:
			OnRec();
			return TRUE;

#if XM7_VER == 1
#if defined(BUBBLE)
		/* �J�� */
		case IDM_B0OPEN:
			OnBubbleOpen(0);
			return TRUE;
		case IDM_B1OPEN:
			OnBubbleOpen(1);
			return TRUE;

		/* �����j�b�g�ŊJ�� */
		case IDM_BBOPEN:
			OnBubbleBoth(0);
			break;

		/* ���O�� */
		case IDM_B0EJECT:
			OnBubbleEject(0);
			return TRUE;
		case IDM_B1EJECT:
			OnBubbleEject(1);
			return TRUE;

		/* �ꎞ�C�W�F�N�g */
		case IDM_B0TEMP:
			OnBubbleTemp(0);
			return TRUE;
		case IDM_B1TEMP:
			OnBubbleTemp(1);
			return TRUE;

		/* �������݋֎~ */
		case IDM_B0WRITE:
			OnBubbleProtect(0);
			return TRUE;
		case IDM_B1WRITE:
			OnBubbleProtect(1);
			return TRUE;
#endif
#endif
	}

#if XM7_VER == 1
#if defined(BUBBLE)
	/* ���f�B�A�����i�o�u���J�Z�b�g�j */
	if ((wID >= IDM_B0MEDIA00) && (wID <= IDM_B0MEDIA15)) {
		LockVM();
		bmc_setmedia(0, wID - IDM_B0MEDIA00);
		ResetSch();
		UnlockVM();
		return TRUE;
	}
	if ((wID >= IDM_B1MEDIA00) && (wID <= IDM_B1MEDIA15)) {
		LockVM();
		bmc_setmedia(1, wID - IDM_B1MEDIA00);
		ResetSch();
		UnlockVM();
		return TRUE;
	}
#endif
#endif

	return FALSE;
}

/*
 *	�e�[�v(A)���j���[�X�V
 */
static void FASTCALL OnTapePopup(HMENU hMenu)
{
#if XM7_VER == 1
#if defined(BUBBLE)
#define	_BUBBLE
	HMENU hSubMenu;
	MENUITEMINFO mii;
	char string[128];
	char buffer[256];
	UINT offset;
	UINT uitem;
	int unit;
	int i;
	int j;
	int k;

	ASSERT(hMenu);

	/* �J�Z�b�g���j���[��1�Ԗڂ̃T�u���j���[�n���h�����擾 */
	hSubMenu = GetSubMenu(hMenu, 0);

	/* ���j���[���ׂč폜 */
	while (GetMenuItemCount(hSubMenu) > 0) {
		DeleteMenu(hSubMenu, 0, MF_BYPOSITION);
	}

	/* ���j���[�\���̏����� */
	memset(&mii, 0, sizeof(mii));
	mii.cbSize = 44;	/* sizeof(mii)��WINVER>=0x0500���� */
	mii.fMask = MIIM_TYPE | MIIM_STATE | MIIM_ID;
	mii.fType = MFT_STRING;
	mii.fState = MFS_ENABLED;

	/* �J�� */
	mii.wID = IDM_TOPEN;
	LoadString(hAppInstance, IDS_TAPEOPEN, string, sizeof(string));
	mii.dwTypeData = string;
	mii.cch = strlen(string);
	InsertMenuItem(hSubMenu, 0, TRUE, &mii);

	/* �e�[�v���Z�b�g����Ă��Ȃ���΁A�����܂� */
	if (tape_fileh != -1) {
		/* ���o�� */
		mii.wID = IDM_TEJECT;
		LoadString(hAppInstance, IDS_TAPEEJECT, string, sizeof(string));
		mii.cch = strlen(string);
		InsertMenuItem(hSubMenu, 1, TRUE, &mii);

		/* �Z�p���[�^�}�� */
		mii.fType = MFT_SEPARATOR;
		InsertMenuItem(hSubMenu, 2, TRUE, &mii);
		mii.fType = MFT_STRING;

		/* �����߂� */
		mii.wID = IDM_REW;
		LoadString(hAppInstance, IDS_TAPEREW, string, sizeof(string));
		mii.cch = strlen(string);
		InsertMenuItem(hSubMenu, 3, TRUE, &mii);

		/* �ŏ��܂Ŋ����߂� */
		mii.wID = IDM_REWTOP;
		LoadString(hAppInstance, IDS_TAPEREWTOP, string, sizeof(string));
		mii.cch = strlen(string);
		InsertMenuItem(hSubMenu, 4, TRUE, &mii);

		/* ������ */
		mii.wID = IDM_FF;
		LoadString(hAppInstance, IDS_TAPEFF, string, sizeof(string));
		mii.cch = strlen(string);
		InsertMenuItem(hSubMenu, 5, TRUE, &mii);

		/* �Z�p���[�^�}�� */
		mii.fType = MFT_SEPARATOR;
		InsertMenuItem(hSubMenu, 6, TRUE, &mii);
		mii.fType = MFT_STRING;

		/* �^�� */
		mii.wID = IDM_REC;
		LoadString(hAppInstance, IDS_TAPEREC, string, sizeof(string));
		mii.cch = strlen(string);
		if (tape_writep) {
			mii.fState = MFS_GRAYED;
		}
		else {
			if (tape_rec) {
				mii.fState = MFS_CHECKED | MFS_ENABLED;
			}
			else {
				mii.fState = MFS_ENABLED;
			}
		}
		InsertMenuItem(hSubMenu, 7, TRUE, &mii);
	}

	/* �o�u�������� �T�u���j���[ */
	for (unit = 0; unit < 2; unit ++) 
	{
		hSubMenu = GetSubMenu(hMenu, unit + 1);

		/* ���j���[���ׂč폜 */
		while (GetMenuItemCount(hSubMenu) > 0) {
			DeleteMenu(hSubMenu, 0, MF_BYPOSITION);
		}

		/* �I�t�Z�b�g�m�� */
		if (unit == 0) {
			offset = 0;
		}
		else {
			offset = IDM_B1OPEN - IDM_B0OPEN;
		}

		/* ���j���[�\���̏����� */
		memset(&mii, 0, sizeof(mii));
		mii.cbSize = 44;	/* sizeof(mii)��WINVER>=0x0500���� */
		mii.fMask = MIIM_TYPE | MIIM_STATE | MIIM_ID;
		mii.fType = MFT_STRING;

		/* �J�� */
		mii.wID = IDM_B0OPEN + offset;
		LoadString(hAppInstance, IDS_BUBBLEOPEN, string, sizeof(string));
		mii.dwTypeData = string;
		mii.fState = MFS_ENABLED;
		mii.cch = strlen(string);
		if (!bmc_enable || (fm_subtype != FMSUB_FM8)) {
			mii.fState = MFS_GRAYED;
		}
		else {
			mii.fState = MFS_ENABLED;
		}
		InsertMenuItem(hSubMenu, 0, TRUE, &mii);

		uitem = 1;

		if (bmc_enable) {
			/* ���I�[�v�� */
			mii.fMask = MIIM_TYPE | MIIM_STATE | MIIM_ID;
			mii.wID = IDM_BBOPEN;
			LoadString(hAppInstance, IDS_BBOPEN, string, sizeof(string));
			mii.cch = strlen(string);
			InsertMenuItem(hSubMenu, uitem++, TRUE, &mii);

			if (bmc_ready[unit] != BMC_TYPE_NOTREADY) {
				/* ���o�� */
				mii.wID = IDM_B0EJECT + offset;
				LoadString(hAppInstance, IDS_BUBBLEEJECT, string, sizeof(string));
				mii.cch = strlen(string);
				InsertMenuItem(hSubMenu, uitem++, TRUE, &mii);

				/* �Z�p���[�^�}�� */
				mii.fType = MFT_SEPARATOR;
				InsertMenuItem(hSubMenu, uitem++, TRUE, &mii);

				/* �ꎞ���o�� */
				mii.wID = IDM_B0TEMP + offset;
				LoadString(hAppInstance, IDS_BUBBLETEMP, string, sizeof(string));
				if ((bmc_ready[unit] == BMC_TYPE_NOTREADY) || (fm_subtype != FMSUB_FM8)) {
					mii.fState = MFS_GRAYED;
				}
				else if (bmc_teject[unit]) {
					mii.fState = MFS_CHECKED | MFS_ENABLED;
				}
				else {
					mii.fState = MFS_ENABLED;
				}
				mii.fType = MFT_STRING;
				mii.cch = strlen(string);
				InsertMenuItem(hSubMenu, uitem++, TRUE, &mii);

				/* �������݋֎~ */
				mii.wID = IDM_B0WRITE + offset;
				LoadString(hAppInstance, IDS_BUBBLEPROTECT, string, sizeof(string));
				if ((bmc_ready[unit] == BMC_TYPE_NOTREADY) || (fm_subtype != FMSUB_FM8)) {
					mii.fState = MFS_GRAYED;
				}
				else {
					if (bmc_fwritep[unit]) {
						mii.fState = MFS_GRAYED;
					}
					else {
						mii.fState = MFS_ENABLED;
					}
					if ((bmc_fwritep[unit]) || (bmc_writep[unit])) {
						mii.fState |= MFS_CHECKED;
					}
				}
				mii.fType = MFT_STRING;
				mii.cch = strlen(string);
				InsertMenuItem(hSubMenu, uitem++, TRUE, &mii);

				/* �Z�p���[�^�}�� */
				mii.fType = MFT_SEPARATOR;
				InsertMenuItem(hSubMenu, uitem++, TRUE, &mii);

				/* B77�Ȃ烁�f�B�A���� */
				if (bmc_ready[unit] == BMC_TYPE_B77) {
					for (i=0; i<bmc_medias[unit]; i++) {
						if (unit == 0) {
							mii.wID = IDM_B0MEDIA00 + i;
						}
						else {
							mii.wID = IDM_B1MEDIA00 + i;
						}
						mii.fState = MFS_ENABLED;
						if (bmc_media[unit] == i) {
							mii.fState |= MFS_CHECKED;
						}
						mii.fType = MFT_STRING | MFT_RADIOCHECK;
						if (strlen(bmc_name[unit][i]) == 0) {
							LoadString(hAppInstance, IDS_MEDIA_NAME, buffer, sizeof(buffer));
							_snprintf(string, sizeof(string), buffer, i + 1);
						}
						else {
							/* �v���t�B�b�N�X�����΍� */
							k = 0;
							for (j=0; j<(int)strlen(bmc_name[unit][i]); j++) {
								if (bmc_name[unit][i][j] == '&') {
									string[k++] = '&';
								}
								string[k++] = bmc_name[unit][i][j];
							}
							string[k] = '\0';
						}
						mii.cch = strlen(string);
						InsertMenuItem(hSubMenu, uitem++, TRUE, &mii);
					}
				}
				else {
					/* BBL�t�@�C���Ȃ�_�~�[���ڔ��� */
					if (unit == 0) {
						mii.wID = IDM_B0MEDIA00;
					}
					else {
						mii.wID = IDM_B1MEDIA00;
					}
					mii.fState = MFS_ENABLED | MFS_CHECKED;
					mii.fType = MFT_STRING | MFT_RADIOCHECK;
					LoadString(hAppInstance, IDS_BBLFILE, string, sizeof(string));
					mii.cch = strlen(string);
					InsertMenuItem(hSubMenu, uitem++, TRUE, &mii);
				}
			}
		}
	}

	EnableMenuPos(hMenu, 1, (fm_subtype == FMSUB_FM8));
	EnableMenuPos(hMenu, 2, (fm_subtype == FMSUB_FM8));
#endif
#endif

#if !defined(_BUBBLE)
	MENUITEMINFO mii;
	char string[128];

	ASSERT(hMenu);

	/* ���j���[���ׂč폜 */
	while (GetMenuItemCount(hMenu) > 0) {
		DeleteMenu(hMenu, 0, MF_BYPOSITION);
	}

	/* ���j���[�\���̏����� */
	memset(&mii, 0, sizeof(mii));
	mii.cbSize = 44;	/* sizeof(mii)��WINVER>=0x0500���� */
	mii.fMask = MIIM_TYPE | MIIM_STATE | MIIM_ID;
	mii.fType = MFT_STRING;
	mii.fState = MFS_ENABLED;

	/* �I�[�v�� */
	mii.wID = IDM_TOPEN;
	LoadString(hAppInstance, IDS_TAPEOPEN, string, sizeof(string));
	mii.dwTypeData = string;
	mii.cch = strlen(string);
	InsertMenuItem(hMenu, 0, TRUE, &mii);

	/* �e�[�v���Z�b�g����Ă��Ȃ���΁A�����܂� */
	if (tape_fileh == -1) {
		return;
	}

	/* �C�W�F�N�g */
	mii.wID = IDM_TEJECT;
	LoadString(hAppInstance, IDS_TAPEEJECT, string, sizeof(string));
	mii.cch = strlen(string);
	InsertMenuItem(hMenu, 1, TRUE, &mii);

	/* �Z�p���[�^�}�� */
	mii.fType = MFT_SEPARATOR;
	InsertMenuItem(hMenu, 2, TRUE, &mii);
	mii.fType = MFT_STRING;

	/* �����߂� */
	mii.wID = IDM_REW;
	LoadString(hAppInstance, IDS_TAPEREW, string, sizeof(string));
	mii.cch = strlen(string);
	InsertMenuItem(hMenu, 3, TRUE, &mii);

	/* �ŏ��܂Ŋ����߂� */
	mii.wID = IDM_REWTOP;
	LoadString(hAppInstance, IDS_TAPEREWTOP, string, sizeof(string));
	mii.cch = strlen(string);
	InsertMenuItem(hMenu, 4, TRUE, &mii);

	/* ������ */
	mii.wID = IDM_FF;
	LoadString(hAppInstance, IDS_TAPEFF, string, sizeof(string));
	mii.cch = strlen(string);
	InsertMenuItem(hMenu, 5, TRUE, &mii);

	/* �Z�p���[�^�}�� */
	mii.fType = MFT_SEPARATOR;
	InsertMenuItem(hMenu, 6, TRUE, &mii);
	mii.fType = MFT_STRING;

	/* �^�� */
	mii.wID = IDM_REC;
	LoadString(hAppInstance, IDS_TAPEREC, string, sizeof(string));
	mii.cch = strlen(string);
	if (tape_writep) {
		mii.fState = MFS_GRAYED;
	}
	else {
		if (tape_rec) {
			mii.fState = MFS_CHECKED | MFS_ENABLED;
		}
		else {
			mii.fState = MFS_ENABLED;
		}
	}
	InsertMenuItem(hMenu, 7, TRUE, &mii);
#endif

#undef _BUBBLE
}

/*-[ �\�����j���[ ]---------------------------------------------------------*/

/*
 *	�t���b�s�[�f�B�X�N�R���g���[��(F)
 */
static void FASTCALL OnFDC(void)
{
	ASSERT(hDrawWnd);

	/* �E�C���h�E�����݂���΁A�N���[�Y�w�����o�� */
	if (hSubWnd[SWND_FDC]) {
		PostMessage(hSubWnd[SWND_FDC], WM_CLOSE, 0, 0);
		return;
	}

	/* �E�C���h�E�쐬 */
	hSubWnd[SWND_FDC] = CreateFDC(hDrawWnd, SWND_FDC);
}

/*
 *	�o�u���������R���g���[��(B)
 */
#if XM7_VER == 1
#if defined(BUBBLE)
static void FASTCALL OnBMC(void)
{
	ASSERT(hDrawWnd);

	/* �E�C���h�E�����݂���΁A�N���[�Y�w�����o�� */
	if (hSubWnd[SWND_BMC]) {
		PostMessage(hSubWnd[SWND_BMC], WM_CLOSE, 0, 0);
		return;
	}

	/* �E�C���h�E�쐬 */
	hSubWnd[SWND_BMC] = CreateBMC(hDrawWnd, SWND_BMC);
}
#endif
#endif

/*
 *	FM�������W�X�^(O)
 */
static void FASTCALL OnOPNReg(void)
{
	ASSERT(hDrawWnd);

	/* �E�C���h�E�����݂���΁A�N���[�Y�w�����o�� */
	if (hSubWnd[SWND_OPNREG]) {
		PostMessage(hSubWnd[SWND_OPNREG], WM_CLOSE, 0, 0);
		return;
	}

	/* �E�C���h�E�쐬 */
	hSubWnd[SWND_OPNREG] = CreateOPNReg(hDrawWnd, SWND_OPNREG);
}

/*
 *	FM�����f�B�X�v���C(D)
 */
static void FASTCALL OnOPNDisp(void)
{
	ASSERT(hDrawWnd);

	/* �E�C���h�E�����݂���΁A�N���[�Y�w�����o�� */
	if (hSubWnd[SWND_OPNDISP]) {
		PostMessage(hSubWnd[SWND_OPNDISP], WM_CLOSE, 0, 0);
		return;
	}

	/* �E�C���h�E�쐬 */
	hSubWnd[SWND_OPNDISP] = CreateOPNDisp(hDrawWnd, SWND_OPNDISP);
}

/*
 *	�T�uCPU�R���g���[��(C)
 */
static void FASTCALL OnSubCtrl(void)
{
	ASSERT(hDrawWnd);

	/* �E�C���h�E�����݂���΁A�N���[�Y�w�����o�� */
	if (hSubWnd[SWND_SUBCTRL]) {
		PostMessage(hSubWnd[SWND_SUBCTRL], WM_CLOSE, 0, 0);
		return;
	}

	/* �E�C���h�E�쐬 */
	hSubWnd[SWND_SUBCTRL] = CreateSubCtrl(hDrawWnd, SWND_SUBCTRL);
}

/*
 *	�p���b�g���W�X�^(P)
 */
static void FASTCALL OnPaletteReg(void)
{
	ASSERT(hDrawWnd);

	/* �E�C���h�E�����݂���΁A�N���[�Y�w�����o�� */
	if (hSubWnd[SWND_PALETTE]) {
		PostMessage(hSubWnd[SWND_PALETTE], WM_CLOSE, 0, 0);
		return;
	}

	/* �E�C���h�E�쐬 */
	hSubWnd[SWND_PALETTE] = CreatePaletteReg(hDrawWnd, SWND_PALETTE);
}

	/*
 *	�L�[�{�[�h(K)
 */
static void FASTCALL OnKeyboard(void)
{
	ASSERT(hDrawWnd);

	/* �E�C���h�E�����݂���΁A�N���[�Y�w�����o�� */
	if (hSubWnd[SWND_KEYBOARD]) {
		PostMessage(hSubWnd[SWND_KEYBOARD], WM_CLOSE, 0, 0);
		return;
	}

	/* �E�C���h�E�쐬 */
	hSubWnd[SWND_KEYBOARD] = CreateKeyboard(hDrawWnd, SWND_KEYBOARD);
}

/*
 *	�_�����Z/�������(L)
 */
#if XM7_VER >= 2
static void FASTCALL OnALULine(void)
{
	ASSERT(hDrawWnd);

	/* �E�C���h�E�����݂���΁A�N���[�Y�w�����o�� */
	if (hSubWnd[SWND_ALULINE]) {
		PostMessage(hSubWnd[SWND_ALULINE], WM_CLOSE, 0, 0);
		return;
	}

	/* �E�C���h�E�쐬 */
	hSubWnd[SWND_ALULINE] = CreateALULine(hDrawWnd, SWND_ALULINE);
}
#endif

/*
 *	�������Ǘ�(M)
 */
static void FASTCALL OnMMR(void)
{
	ASSERT(hDrawWnd);

	/* �E�C���h�E�����݂���΁A�N���[�Y�w�����o�� */
	if (hSubWnd[SWND_MMR]) {
		PostMessage(hSubWnd[SWND_MMR], WM_CLOSE, 0, 0);
		return;
	}

	/* �E�C���h�E�쐬 */
	hSubWnd[SWND_MMR] = CreateMMR(hDrawWnd, SWND_MMR);
}

#if XM7_VER >= 3
/*
 *	DMA�R���g���[��(A)
 */
static void FASTCALL OnDMAC(void)
{
	ASSERT(hDrawWnd);

	/* �E�C���h�E�����݂���΁A�N���[�Y�w�����o�� */
	if (hSubWnd[SWND_DMAC]) {
		PostMessage(hSubWnd[SWND_DMAC], WM_CLOSE, 0, 0);
		return;
	}

	/* �E�C���h�E�쐬 */
	hSubWnd[SWND_DMAC] = CreateDMAC(hDrawWnd, SWND_DMAC);
}
#endif

/*
 *	�X�e�[�^�X�o�[(S)
 */
static void FASTCALL OnStatus(void)
{
	/* �X�e�[�^�X�o�[���L���łȂ���΁A�������Ȃ� */
	if (!hStatusBar) {
		return;
	}

	if (IsWindowVisible(hStatusBar)) {
		/* ���� */
		ShowWindow(hStatusBar, SW_HIDE);
		bHideStatus = TRUE;
	}
	else {
		/* �\�� */
		ShowWindow(hStatusBar, SW_SHOW);
		bHideStatus = FALSE;
	}

	/* �ݒ胏�[�N�ɔ��f */
	GetCfg();

	/* �t���[���E�C���h�E�̃T�C�Y��␳ */
	OnSize(hMainWnd, 640, 400);
}

/*
 *	�ŐV�̏��ɍX�V(R)
 */
void FASTCALL OnRefresh(HWND hWnd)
{
	int i;

	ASSERT(hWnd);
	ASSERT(hDrawWnd);

	/* �t�A�Z���u���E�C���h�E�@�A�h���X�X�V */
	if ((hSubWnd[SWND_DISASM_MAIN]) && (bSyncDisasm[0])) {
		AddrDisAsm(MAINCPU, maincpu.pc);
	}
	if ((hSubWnd[SWND_DISASM_SUB]) && (bSyncDisasm[1])) {
		AddrDisAsm(SUBCPU, subcpu.pc);
	}
#if XM7_VER == 1
#if defined(JSUB)
	if ((hSubWnd[SWND_DISASM_JSUB]) && (bSyncDisasm[2])) {
		AddrDisAsm(JSUBCPU, jsubcpu.pc);
	}
#endif
#endif

	/* FM����/PSG���W�X�^/�f�B�X�v���C�E�C���h�E�X�V */
	ReSizeOPNReg();
	ReSizeOPNDisp();

	/* ���C���E�C���h�E */
	InvalidateRect(hWnd, NULL, FALSE);
	InvalidateRect(hDrawWnd, NULL, FALSE);

	/* �T�u�E�C���h�E */
	for (i=0; i<SWND_MAXNUM; i++) {
		if (hSubWnd[i]) {
			InvalidateRect(hSubWnd[i], NULL, FALSE);
		}
	}
}

/*
 *	���s�ɓ���(Y)
 */
static void FASTCALL OnSync(void)
{
	bSync = (!bSync);
}

/*
 *	�t���X�N���[��(U)
 */
static void FASTCALL OnFullScreen(HWND hWnd)
{
	BOOL bRun;
	int i;

	ASSERT(hWnd);

#if defined(KBDPASTE)
	if (hKeyStrokeDialog) {
		/* �E�C���h�E�N���[�Y */
		PostMessage(hKeyStrokeDialog, WM_CLOSE, 0, 0);
		Sleep(1);
	}
#endif

	/* VM�����b�N�A�X�g�b�v */
	LockVM();
	bRun = run_flag;
	run_flag = FALSE;
	StopSnd();

	/* ���[�h�؂�ւ� */
	if (bFullScreen) {
		ModeDraw(hWnd, FALSE);

		if (!bFullScreen) {
			/* ���ׂẴT�u�E�B���h�E���t���X�N���[�����O�̏�Ԃɖ߂� */
			for (i=0; i<SWND_MAXNUM; i++) {
				if (hSubWnd[i]) {
					if (bShowSubWindow[i]) {
						ShowWindow(hSubWnd[i], SW_RESTORE);
					}
				}
			}
			if (bPopupSwnd) {
				SetForegroundWindow(hWnd);
			}
		}
	}
	else {
		ModeDraw(hWnd, TRUE);

		if (bFullScreen) {
			/* ���ׂẴT�u�E�B���h�E���B�� */
			for (i=0; i<SWND_MAXNUM; i++) {
				bShowSubWindow[i] = FALSE;
				if (hSubWnd[i]) {
					if (ShowWindow(hSubWnd[i], SW_HIDE)) {
						bShowSubWindow[i] = TRUE;
					}
				}
			}
		}
	}

	/* VM���A�����b�N */
	GetCfg();
	run_flag = bRun;
	ResetSch();
	UnlockVM();
	PlaySnd();
}

/*
 *	�\��(V)���j���[
 */
static BOOL FASTCALL OnView(HWND hWnd, WORD wID)
{
	ASSERT(hWnd);

	switch (wID) {
		/* FDC */
		case IDM_FDC:
			OnFDC();
			return TRUE;

		/* �o�u���������R���g���[�� */
#if XM7_VER == 1
#if defined(BUBBLE)
		case IDM_BMC:
			OnBMC();
			return TRUE;
#endif
#endif

		/* OPN���W�X�^ */
		case IDM_OPNREG:
			OnOPNReg();
			return TRUE;

		/* OPN�f�B�X�v���C */
		case IDM_OPNDISP:
			OnOPNDisp();
			return TRUE;

		/* �T�uCPU�R���g���[�� */
		case IDM_SUBCTRL:
			OnSubCtrl();
			return TRUE;

		/* �p���b�g���W�X�^ */
		case IDM_PALETTE:
			OnPaletteReg();
			return TRUE;

		/* �L�[�{�[�h */
		case IDM_KEYBOARD:
			OnKeyboard();
			return TRUE;

#if XM7_VER >= 2
		/* �_�����Z/������� */
		case IDM_ALULINE:
			OnALULine();
			return TRUE;
#endif

		/* �������Ǘ� */
		case IDM_MMR:
			OnMMR();
			return TRUE;

#if XM7_VER >= 3
		/* DMA�R���g���[�� */
		case IDM_DMAC:
			OnDMAC();
			return TRUE;
#endif

		/* �X�e�[�^�X�o�[ */
		case IDM_STATUS:
			OnStatus();
			return TRUE;

		/* �ŐV�̏��ɍX�V */
		case IDM_REFRESH:
			OnRefresh(hWnd);
			return TRUE;

		/* ���s�ɓ��� */
		case IDM_SYNC:
			OnSync();
			return TRUE;

		/* �t���X�N���[�� */
		case IDM_FULLSCREEN:
			OnFullScreen(hWnd);
			return TRUE;
	}

	return FALSE;
}

/*
 *	�\��(V)���j���[�X�V
 */
static void FASTCALL OnViewPopup(HMENU hMenu)
{
	/* �T�u�E�C���h�E�Q */
	CheckMenuSub(hMenu, IDM_FDC, (BOOL)hSubWnd[SWND_FDC]);
#if XM7_VER == 1
#if defined(BUBBLE)
	CheckMenuSub(hMenu, IDM_BMC, (BOOL)hSubWnd[SWND_BMC]);
#endif
#endif
	CheckMenuSub(hMenu, IDM_OPNREG, (BOOL)hSubWnd[SWND_OPNREG]);
	CheckMenuSub(hMenu, IDM_OPNDISP, (BOOL)hSubWnd[SWND_OPNDISP]);
	CheckMenuSub(hMenu, IDM_PALETTE, (BOOL)hSubWnd[SWND_PALETTE]);
	CheckMenuSub(hMenu, IDM_SUBCTRL, (BOOL)hSubWnd[SWND_SUBCTRL]);
#if XM7_VER >= 2
	CheckMenuSub(hMenu, IDM_ALULINE, (BOOL)hSubWnd[SWND_ALULINE]);
#endif
	CheckMenuSub(hMenu, IDM_KEYBOARD, (BOOL)hSubWnd[SWND_KEYBOARD]);
	CheckMenuSub(hMenu, IDM_MMR, (BOOL)hSubWnd[SWND_MMR]);
#if XM7_VER >= 3
	CheckMenuSub(hMenu, IDM_DMAC, (BOOL)hSubWnd[SWND_DMAC]);
#endif

	/* ���̑� */
	if (hStatusBar) {
		CheckMenuSub(hMenu, IDM_STATUS, IsWindowVisible(hStatusBar));
	}
	else {
		CheckMenuSub(hMenu, IDM_STATUS, FALSE);
	}
	CheckMenuSub(hMenu, IDM_SYNC, bSync);
	CheckMenuSub(hMenu, IDM_FULLSCREEN, bFullScreen);

	/* �t���X�N���[�����̃T�u�E�B���h�E���j���[������ */
	EnableMenuSub(hMenu, IDM_FDC, !bFullScreen);
#if XM7_VER == 1
#if defined(BUBBLE)
	EnableMenuSub(hMenu, IDM_BMC, !bFullScreen);
#endif
#endif
	EnableMenuSub(hMenu, IDM_OPNREG, !bFullScreen);
	EnableMenuSub(hMenu, IDM_OPNDISP, !bFullScreen);
	EnableMenuSub(hMenu, IDM_PALETTE, !bFullScreen);
	EnableMenuSub(hMenu, IDM_SUBCTRL, !bFullScreen);
#if XM7_VER >= 2
	EnableMenuSub(hMenu, IDM_ALULINE, !bFullScreen);
#endif
	EnableMenuSub(hMenu, IDM_KEYBOARD, !bFullScreen);
	EnableMenuSub(hMenu, IDM_MMR, !bFullScreen);
#if XM7_VER >= 3
	EnableMenuSub(hMenu, IDM_DMAC, !bFullScreen);
#endif
}

/*-[ �f�o�b�O���j���[ ]-----------------------------------------------------*/

/*
 *	���s(X)
 */
static void FASTCALL OnExec(void)
{
	/* ���Ɏ��s���Ȃ�A�������Ȃ� */
	if (run_flag) {
		return;
	}

	/* �X�^�[�g */
	LockVM();
	stopreq_flag = FALSE;
	run_flag = TRUE;
	UnlockVM();
}

/*
 *	��~(B)
 */
static void FASTCALL OnBreak(void)
{
	/* ���ɒ�~��ԂȂ�A�������Ȃ� */
	if (!run_flag) {
		return;
	}

	/* ��~ */
	LockVM();
	stopreq_flag = TRUE;
	UnlockVM();
}

/*
 *	�g���[�X(T)
 */
static void FASTCALL OnTrace(HWND hWnd)
{
	ASSERT(hWnd);

	/* ��~��ԂłȂ���΁A���^�[�� */
	if (run_flag) {
		return;
	}

	/* ���s */
	schedule_trace();
	AddrDisAsm(MAINCPU, maincpu.pc);
	AddrDisAsm(SUBCPU, subcpu.pc);
#if XM7_VER == 1
#if defined(JSUB)
	AddrDisAsm(JSUBCPU, jsubcpu.pc);
#endif
#endif

	/* �\���X�V */
	OnRefresh(hWnd);
}

/*
 *	�u���[�N�|�C���g(B)
 */
static void FASTCALL OnBreakPoint(void)
{
	ASSERT(hDrawWnd);

	/* �E�C���h�E�����݂���΁A�N���[�Y�w�����o�� */
	if (hSubWnd[SWND_BREAKPOINT]) {
		PostMessage(hSubWnd[SWND_BREAKPOINT], WM_CLOSE, 0, 0);
		return;
	}

	/* �E�C���h�E�쐬 */
	hSubWnd[SWND_BREAKPOINT] = CreateBreakPoint(hDrawWnd, SWND_BREAKPOINT);
}

/*
 *	�X�P�W���[��(S)
 */
static void FASTCALL OnScheduler(void)
{
	ASSERT(hDrawWnd);

	/* �E�C���h�E�����݂���΁A�N���[�Y�w�����o�� */
	if (hSubWnd[SWND_SCHEDULER]) {
		PostMessage(hSubWnd[SWND_SCHEDULER], WM_CLOSE, 0, 0);
		return;
	}

	/* �E�C���h�E�쐬 */
	hSubWnd[SWND_SCHEDULER] = CreateScheduler(hDrawWnd, SWND_SCHEDULER);
}

/*
 *	CPU���W�X�^(C)
 */
static void FASTCALL OnCPURegister(int nCPU)
{
	int index;

#if XM7_VER >= 2
	ASSERT((nCPU >= MAINCPU) && (nCPU <= SUBCPU));
#elif defined(Z80CARD)
	ASSERT((nCPU >= MAINCPU) && (nCPU <= MAINZ80));
#else
	ASSERT((nCPU >= MAINCPU) && (nCPU <= JSUBCPU));
#endif
	ASSERT(hDrawWnd);

	/* �C���f�b�N�X���� */
	index = SWND_CPUREG_MAIN + nCPU;

	/* �E�C���h�E�����݂���΁A�N���[�Y�w�����o�� */
	if (hSubWnd[index]) {
		PostMessage(hSubWnd[index], WM_CLOSE, 0, 0);
		return;
	}

	/* �E�C���h�E�쐬 */
	hSubWnd[index] = CreateCPURegister(hDrawWnd, (BYTE)nCPU, index);
}

/*
 *	�t�A�Z���u��(D)
 */
static void FASTCALL OnDisAsm(int nCPU)
{
	int index;

#if XM7_VER >= 2
	ASSERT((nCPU >= MAINCPU) && (nCPU <= SUBCPU));
#elif defined(Z80CARD)
	ASSERT((nCPU >= MAINCPU) && (nCPU <= MAINZ80));
#else
	ASSERT((nCPU >= MAINCPU) && (nCPU <= JSUBCPU));
#endif
	ASSERT(hDrawWnd);

	/* �C���f�b�N�X���� */
	index = SWND_DISASM_MAIN + nCPU;

	/* �E�C���h�E�����݂���΁A�N���[�Y�w�����o�� */
	if (hSubWnd[index]) {
		PostMessage(hSubWnd[index], WM_CLOSE, 0, 0);
		return;
	}

	/* �E�C���h�E�쐬 */
	hSubWnd[index] = CreateDisAsm(hDrawWnd, (BYTE)nCPU, index);
}

/*
 *	�������_���v(M)
 */
static void FASTCALL OnMemory(int nCPU)
{
	int index;

#if XM7_VER >= 2
	ASSERT((nCPU >= MAINCPU) && (nCPU <= SUBCPU));
#else
	/* ���̃P�[�X�̂�Z80���[�h��p�E�C���h�E�͑��݂��Ȃ� */
	ASSERT((nCPU >= MAINCPU) && (nCPU <= JSUBCPU));
#endif
	ASSERT(hDrawWnd);

	/* �C���f�b�N�X���� */
	index = SWND_MEMORY_MAIN + nCPU;

	/* �E�C���h�E�����݂���΁A�N���[�Y�w�����o�� */
	if (hSubWnd[index]) {
		PostMessage(hSubWnd[index], WM_CLOSE, 0, 0);
		return;
	}

	/* �E�C���h�E�쐬 */
	hSubWnd[index] = CreateMemory(hDrawWnd, (BYTE)nCPU, index);
}

/*
 *	�f�o�b�O(D)���j���[
 */
static BOOL FASTCALL OnDebug(HWND hWnd, WORD wID)
{
	ASSERT(hWnd);

	switch (wID) {
		/* ���s */
		case IDM_EXEC:
			OnExec();
			return TRUE;

		/* �u���[�N */
		case IDM_BREAK:
			OnBreak();
			return TRUE;

		/* �g���[�X */
		case IDM_TRACE:
			OnTrace(hWnd);
			return TRUE;

		/* �u���[�N�|�C���g */
		case IDM_BREAKPOINT:
			OnBreakPoint();
			return TRUE;

		/* �X�P�W���[�� */
		case IDM_SCHEDULER:
			OnScheduler();
			return TRUE;

		/* CPU���W�X�^(���C��) */
		case IDM_CPU_MAIN:
			OnCPURegister(MAINCPU);
			return TRUE;

		/* CPU���W�X�^(�T�u) */
		case IDM_CPU_SUB:
			OnCPURegister(SUBCPU);
			return TRUE;

		/* �t�A�Z���u��(���C��) */
		case IDM_DISASM_MAIN:
			OnDisAsm(MAINCPU);
			return TRUE;

		/* �t�A�Z���u��(�T�u) */
		case IDM_DISASM_SUB:
			OnDisAsm(SUBCPU);
			return TRUE;

		/* �������_���v(���C��) */
		case IDM_MEMORY_MAIN:
			OnMemory(MAINCPU);
			return TRUE;

		/* �������_���v(�T�u) */
		case IDM_MEMORY_SUB:
			OnMemory(SUBCPU);
			return TRUE;

#if XM7_VER == 1
#if defined(JSUB)
		/* CPU���W�X�^(���{��T�u) */
		case IDM_CPU_JSUB:
			OnCPURegister(JSUBCPU);
			return TRUE;

		/* �t�A�Z���u��(���{��T�u) */
		case IDM_DISASM_JSUB:
			OnDisAsm(JSUBCPU);
			return TRUE;

		/* �������_���v(���{��T�u) */
		case IDM_MEMORY_JSUB:
			OnMemory(JSUBCPU);
			return TRUE;
#endif

#if defined(Z80CARD)
		/* CPU���W�X�^(���C��Z80) */
		case IDM_CPU_MAINZ80:
			OnCPURegister(MAINZ80);
			return TRUE;

		/* �t�A�Z���u��(���C��Z80) */
		case IDM_DISASM_MAINZ80:
			OnDisAsm(MAINZ80);
			return TRUE;
#endif
#endif
	}

	return FALSE;
}

/*
 *	�f�o�b�O(D)���j���[�X�V
 */
static void FASTCALL OnDebugPopup(HMENU hMenu)
{
	ASSERT(hMenu);

	/* ���s���� */
	EnableMenuSub(hMenu, IDM_EXEC, !run_flag);
	EnableMenuSub(hMenu, IDM_BREAK, run_flag);
	EnableMenuSub(hMenu, IDM_TRACE, !run_flag);

	/* �T�u�E�C���h�E�Q */
	CheckMenuSub(hMenu, IDM_BREAKPOINT, (BOOL)hSubWnd[SWND_BREAKPOINT]);
	CheckMenuSub(hMenu, IDM_SCHEDULER, (BOOL)hSubWnd[SWND_SCHEDULER]);
	CheckMenuSub(hMenu, IDM_CPU_MAIN, (BOOL)hSubWnd[SWND_CPUREG_MAIN]);
	CheckMenuSub(hMenu, IDM_CPU_SUB, (BOOL)hSubWnd[SWND_CPUREG_SUB]);
	CheckMenuSub(hMenu, IDM_DISASM_MAIN, (BOOL)hSubWnd[SWND_DISASM_MAIN]);
	CheckMenuSub(hMenu, IDM_DISASM_SUB, (BOOL)hSubWnd[SWND_DISASM_SUB]);
	CheckMenuSub(hMenu, IDM_MEMORY_MAIN, (BOOL)hSubWnd[SWND_MEMORY_MAIN]);
	CheckMenuSub(hMenu, IDM_MEMORY_SUB, (BOOL)hSubWnd[SWND_MEMORY_SUB]);
#if XM7_VER == 1 
#if defined(JSUB)
	CheckMenuSub(hMenu, IDM_CPU_JSUB, (BOOL)hSubWnd[SWND_CPUREG_JSUB]);
	CheckMenuSub(hMenu, IDM_DISASM_JSUB, (BOOL)hSubWnd[SWND_DISASM_JSUB]);
	CheckMenuSub(hMenu, IDM_MEMORY_JSUB, (BOOL)hSubWnd[SWND_MEMORY_JSUB]);
#endif
#if defined(Z80CARD)
	CheckMenuSub(hMenu, IDM_CPU_MAINZ80, (BOOL)hSubWnd[SWND_CPUREG_Z80]);
	CheckMenuSub(hMenu, IDM_DISASM_MAINZ80, (BOOL)hSubWnd[SWND_DISASM_Z80]);
#endif
#endif

	/* �t���X�N���[�����̃T�u�E�B���h�E���j���[������ */
	EnableMenuSub(hMenu, IDM_BREAKPOINT, !bFullScreen);
	EnableMenuSub(hMenu, IDM_SCHEDULER, !bFullScreen);
	EnableMenuSub(hMenu, IDM_CPU_MAIN, !bFullScreen);
	EnableMenuSub(hMenu, IDM_CPU_SUB, !bFullScreen);
	EnableMenuSub(hMenu, IDM_DISASM_MAIN, !bFullScreen);
	EnableMenuSub(hMenu, IDM_DISASM_SUB, !bFullScreen);
	EnableMenuSub(hMenu, IDM_MEMORY_MAIN, !bFullScreen);
	EnableMenuSub(hMenu, IDM_MEMORY_SUB, !bFullScreen);
#if XM7_VER == 1 
#if defined(JSUB)
	EnableMenuSub(hMenu, IDM_CPU_JSUB,
		!bFullScreen && jsub_available && jsub_enable);
	EnableMenuSub(hMenu, IDM_DISASM_JSUB,
		!bFullScreen && jsub_available && jsub_enable);
	EnableMenuSub(hMenu, IDM_MEMORY_JSUB,
		!bFullScreen && jsub_available && jsub_enable);
#endif
#if defined(Z80CARD)
	EnableMenuSub(hMenu, IDM_CPU_MAINZ80, !bFullScreen);
	EnableMenuSub(hMenu, IDM_DISASM_MAINZ80, !bFullScreen);
#endif
#endif
}

/*-[ �c�[�����j���[ ]-------------------------------------------------------*/

/*
 *	�}�E�X���[�h�؂芷��
 */
#if defined(MOUSE)
void FASTCALL MouseModeChange(BOOL flag)
{
	/* ���[�h�ω����Ȃ���΋A�� */
	if (mos_capture == flag) {
		return;
	}

	/* �}�E�X�L���v�`���t���O��ݒ� */
	mos_capture = flag;

	/* �X�e�[�^�X�o�[�ɏ�ԕ\�� */
	if (flag) {
		SetStatusMessage(IDS_MOUSE_ENABLE);
	}
	else {
		SetStatusMessage(IDS_MOUSE_DISABLE);
	}
}
#endif

/*
 *	�����A�W���X�g
 */
#if XM7_VER >= 2
static void FASTCALL OnTimeAdjust(void)
{
	/* �������Đݒ肷�� */
	rtc_time_adjust();

	/* �O�̂��߃X�P�W���[���������� */
	rtc_reset();
}
#endif

/*
 *	�}�E�X���[�h�؂芷��(M)
 */
#if defined(MOUSE)
static void FASTCALL OnMouseMode(void)
{
	/* �}�E�X�L���v�`���t���O�𔽓]�����ă��[�h�؂�ւ� */
	MouseModeChange(!mos_capture);
}
#endif

/*
 *	��ʃL���v�`��(C)
 */
static void FASTCALL OnGrpCapture(void)
{
	char path[_MAX_PATH];

	/* �t�@�C���I�� */
	if (!FileSelectSub(FALSE, IDS_GRPCAPFILTER, path, "BMP", 3)) {
		return;
	}

	/* �L���v�`�� */
	LockVM();
	StopSnd();
#if XM7_VER == 1
	capture_to_bmp(path, bFullScan, bGreenMonitor, bPseudo400Line);
#elif XM7_VER == 2
	capture_to_bmp(path, bFullScan, bTTLMonitor, bPseudo400Line);
#else
	capture_to_bmp(path, bFullScan, FALSE, bPseudo400Line);
#endif
	PlaySnd();
	ResetSch();
	UnlockVM();
}

/*
 *	��ʃL���v�`��2
 */
static void FASTCALL OnGrpCapture2(void)
{
	char path[_MAX_PATH];

	/* �t�@�C���I�� */
	if (!FileSelectSub(FALSE, IDS_GRPCAPFILTER, path, "BMP", 3)) {
		return;
	}

	/* �L���v�`�� */
	LockVM();
	StopSnd();
#if XM7_VER == 1
	capture_to_bmp2(path, bGreenMonitor, bPseudo400Line);
#elif XM7_VER == 2
	capture_to_bmp2(path, bTTLMonitor, bPseudo400Line);
#else
	capture_to_bmp2(path, FALSE, bPseudo400Line);
#endif
	PlaySnd();
	ResetSch();
	UnlockVM();
}

/*
 *	WAV�L���v�`��(W)
 */
static void FASTCALL OnWavCapture(HWND hWnd)
{
	char path[_MAX_PATH];

	ASSERT(hWnd);

	/* ���ɃL���v�`�����Ȃ�A�N���[�Y */
	if (hWavCapture >= 0) {
		LockVM();
		CloseCaptureSnd();
		UnlockVM();
		return;
	}

	/* �t�@�C���I�� */
	if (!FileSelectSub(FALSE, IDS_WAVCAPFILTER, path, "WAV", 4)) {
		return;
	}

	/* �L���v�`�� */
	LockVM();
	OpenCaptureSnd(path);
	UnlockVM();

	/* �������� */
	if (hWavCapture < 0) {
		LockVM();
		StopSnd();

		LoadString(hAppInstance, IDS_WAVCAPERROR, path, sizeof(path));
		MessageBox(hWnd, path, "XM7", MB_ICONSTOP | MB_OK);
		SetMenuExitTimer();

		PlaySnd();
		ResetSch();
		UnlockVM();
	}
}

/*
 *	�V�K�f�B�X�N�쐬(D)
 */
static void FASTCALL OnNewDisk(HWND hWnd)
{
	char path[_MAX_PATH];
	int ret;
	BOOL err;

	ASSERT(hWnd);

	/* �^�C�g������ */
	ret = DialogBox(hAppInstance, MAKEINTRESOURCE(IDD_TITLEDLG),
						hWnd, TitleDlgProc);
	if (ret != IDOK) {
		return;
	}

	/* �t�@�C���I�� */
	if (!FileSelectSub(FALSE, IDS_NEWDISKFILTER, path, "D77", 0)) {
		return;
	}

	/* �쐬 */
	LockVM();
	StopSnd();

	if (DiskFormat) {
		err = make_new_userdisk(path, DiskTitle, DiskMedia);
	}
	else {
		err = make_new_d77(path, DiskTitle, DiskMedia);
	}
	if (err) {
		LoadString(hAppInstance, IDS_NEWDISKOK, path, sizeof(path));
		MessageBox(hWnd, path, "XM7", MB_OK);
		SetMenuExitTimer();
	}

	PlaySnd();
	ResetSch();
	UnlockVM();
}

/*
 *	�V�K�e�[�v�쐬(T)
 */
static void FASTCALL OnNewTape(HWND hWnd)
{
	char path[_MAX_PATH];

	ASSERT(hWnd);

	/* �t�@�C���I�� */
	if (!FileSelectSub(FALSE, IDS_TAPEFILTER, path, "T77", 1)) {
		return;
	}

	/* �쐬 */
	LockVM();
	StopSnd();

	if (make_new_t77(path)) {
		LoadString(hAppInstance, IDS_NEWTAPEOK, path, sizeof(path));
		MessageBox(hWnd, path, "XM7", MB_OK);
		SetMenuExitTimer();
	}

	PlaySnd();
	ResetSch();
	UnlockVM();
}

#if XM7_VER == 1
#if defined(BUBBLE)
/*
 *	�V�K�o�u���J�Z�b�g�쐬(B)
 */
static void FASTCALL OnNewBubble(HWND hWnd)
{
	char path[_MAX_PATH];
	int ret;
	BOOL err;

	ASSERT(hWnd);

	/* �^�C�g������ */
	strncpy(BubbleTitle, "Default", sizeof(BubbleTitle));
	ret = DialogBox(hAppInstance, MAKEINTRESOURCE(IDD_MEDIATYPEDLG),
						hWnd, BubbleMediaTypeDlgProc);
	if (ret != IDOK) {
		return;
	}

	/* �t�@�C���I�� */
	if (BubbleFormat) {
		if (!FileSelectSub(FALSE, IDS_B77FILTER, path, "B77", 5)) {
			return;
		}
	}
	else {
		if (!FileSelectSub(FALSE, IDS_BBLFILTER, path, "BBL", 5)) {
			return;
		}
	}

	/* �쐬 */
	LockVM();
	StopSnd();

	if (BubbleFormat) {
		err = make_new_bubble(path, BubbleTitle);
	}
	else {
		err = make_new_bubble(path, NULL);
	}
	if (err) {
		LoadString(hAppInstance, IDS_NEWBUBBLEOK, path, sizeof(path));
		MessageBox(hWnd, path, "XM7", MB_OK);
		SetMenuExitTimer();
	}

	PlaySnd();
	ResetSch();
	UnlockVM();
}
#endif
#endif

/*
 *	VFD��D77�ϊ�(V)
 */
static void FASTCALL OnVFD2D77(HWND hWnd)
{
	char src[_MAX_PATH];
	char dst[_MAX_PATH];
	int ret;

	ASSERT(hWnd);

	/* �t�@�C���I�� */
	if (!FileSelectSub(TRUE, IDS_VFDFILTER, src, "VFD", 0)) {
		return;
	}

	/* �^�C�g������ */
	ret = DialogBox(hAppInstance, MAKEINTRESOURCE(IDD_TITLEDLG_2D),
						hWnd, TitleDlg2DProc);
	if (ret != IDOK) {
		SetMenuExitTimer();
		return;
	}

	/* �t�@�C���I�� */
	if (!FileSelectSub(FALSE, IDS_NEWDISKFILTER, dst, "D77", 0)) {
		return;
	}

	/* �쐬 */
	LockVM();
	StopSnd();

	if (conv_vfd_to_d77(src, dst, DiskTitle)) {
		LoadString(hAppInstance, IDS_CONVERTOK, src, sizeof(src));
	}
	else {
		LoadString(hAppInstance, IDS_CONVERTFAIL, src, sizeof(src));
	}
	MessageBox(hWnd, src, "XM7", MB_OK);
	SetMenuExitTimer();

	PlaySnd();
	ResetSch();
	UnlockVM();
}

/*
 *	2D��D77�ϊ�(2)
 */
static void FASTCALL On2D2D77(HWND hWnd)
{
	char src[_MAX_PATH];
	char dst[_MAX_PATH];
	int ret;

	ASSERT(hWnd);

	/* �t�@�C���I�� */
	if (!FileSelectSub(TRUE, IDS_2DFILTER, src, "2D", 0)) {
		return;
	}

	/* �^�C�g������ */
	ret = DialogBox(hAppInstance, MAKEINTRESOURCE(IDD_TITLEDLG_2D),
						hWnd, TitleDlg2DProc);
	if (ret != IDOK) {
		SetMenuExitTimer();
		return;
	}

	/* �t�@�C���I�� */
	if (!FileSelectSub(FALSE, IDS_NEWDISKFILTER, dst, "D77", 0)) {
		return;
	}

	/* �쐬 */
	LockVM();
	StopSnd();

	if (conv_2d_to_d77(src, dst, DiskTitle)) {
		LoadString(hAppInstance, IDS_CONVERTOK, src, sizeof(src));
	}
	else {
		LoadString(hAppInstance, IDS_CONVERTFAIL, src, sizeof(src));
	}
	MessageBox(hWnd, src, "XM7", MB_OK);
	SetMenuExitTimer();

	PlaySnd();
	ResetSch();
	UnlockVM();
}

/*
 *	VTP��T77�ϊ�(P)
 */
static void FASTCALL OnVTP2T77(HWND hWnd)
{
	char src[_MAX_PATH];
	char dst[_MAX_PATH];

	ASSERT(hWnd);

	/* �t�@�C���I�� */
	if (!FileSelectSub(TRUE, IDS_VTPFILTER, src, "VTP", 1)) {
		return;
	}

	/* �t�@�C���I�� */
	if (!FileSelectSub(FALSE, IDS_TAPEFILTER, dst, "T77", 1)) {
		return;
	}

	/* �쐬 */
	LockVM();
	StopSnd();

	if (conv_vtp_to_t77(src, dst)) {
		LoadString(hAppInstance, IDS_CONVERTOK, src, sizeof(src));
	}
	else {
		LoadString(hAppInstance, IDS_CONVERTFAIL, src, sizeof(src));
	}
	MessageBox(hWnd, src, "XM7", MB_OK);
	SetMenuExitTimer();

	PlaySnd();
	ResetSch();
	UnlockVM();
}

#if XM7_VER == 1
#if defined(BUBBLE)
/*
 *	BBL��B77�ϊ�(L)
 */
static void FASTCALL OnBBL2B77(HWND hWnd)
{
	char src[_MAX_PATH];
	char dst[_MAX_PATH];
	int ret;

	ASSERT(hWnd);

	/* �t�@�C���I�� */
	if (!FileSelectSub(TRUE, IDS_BBLFILTER, src, "BBL", 5)) {
		return;
	}

	/* �^�C�g������ */
	strncpy(DiskTitle, "Default", sizeof(DiskTitle));
	ret = DialogBox(hAppInstance, MAKEINTRESOURCE(IDD_TITLEDLG_2D),
						hWnd, TitleDlg2DProc);
	if (ret != IDOK) {
		SetMenuExitTimer();
		return;
	}

	/* �t�@�C���I�� */
	if (!FileSelectSub(FALSE, IDS_B77FILTER, dst, "B77", 5)) {
		return;
	}

	/* �쐬 */
	LockVM();
	StopSnd();

	if (conv_bbl_to_b77(src, dst, DiskTitle)) {
		LoadString(hAppInstance, IDS_CONVERTOK, src, sizeof(src));
	}
	else {
		LoadString(hAppInstance, IDS_CONVERTFAIL, src, sizeof(src));
	}
	MessageBox(hWnd, src, "XM7", MB_OK);
	SetMenuExitTimer();

	PlaySnd();
	ResetSch();
	UnlockVM();
}
#endif
#endif

#if defined(KBDPASTE)
/*
 *	�\��t��(E)
 */
static void FASTCALL OnPaste(HWND hWnd)
{
	ASSERT(hWnd);

	/* �y�[�X�g�҂����Ԑݒ肪�Ȃ��ꍇ */
	if ((uPasteWait == 0) && (uPasteWaitCntl == 0)) {
		return;
	}

	/* ���s */
	LockVM();
	StopSnd();

	PasteClipboardKbd(hWnd);

	PlaySnd();
	ResetSch();
	UnlockVM();
}

/*
 *	�L�[���͎x��(K)
 */
static void FASTCALL OnKeyStroke(HWND hWnd)
{
	int ret;

	ASSERT(hWnd);

	/* �y�[�X�g�҂����Ԑݒ肪�Ȃ��ꍇ */
	if ((uPasteWait == 0) && (uPasteWaitCntl == 0)) {
		return;
	}

	/* ���s */
	if (bFullScreen || !bKeyStrokeModeless) {
		/* ��d�ɊJ���Ȃ� */
		if (hKeyStrokeDialog) {
			/* �E�C���h�E�N���[�Y */
			PostMessage(hKeyStrokeDialog, WM_CLOSE, 0, 0);
			Sleep(1);
		}

		/* �L�[���͎x���_�C�A���O */
		ret = DialogBox(hAppInstance, MAKEINTRESOURCE(IDD_KEYSTROKEDLG),
						hWnd, KeyStrokeDlgProc);
		if (ret != IDOK) {
			return;
		}

		/* �쐬 */
		LockVM();
		PasteKbd((char *)KeyStrokeString);
		InvalidateRect(hDrawWnd, NULL, FALSE);
		UnlockVM();
	}
	else {
		/* ��d�ɊJ���Ȃ� */
		if (hKeyStrokeDialog) {
			SetForegroundWindow(hKeyStrokeDialog);
			return;
		}

		/* �L�[���͎x���_�C�A���O */
		hKeyStrokeDialog = CreateDialog(
			(HINSTANCE)GetWindowLong(hWnd, GWL_HINSTANCE),
			MAKEINTRESOURCE(IDD_KEYSTROKEDLG),
			hWnd,
			(DLGPROC)KeyStrokeDlgProc);
		if (hKeyStrokeDialog == NULL) {
			return;
		}
		ShowWindow(hKeyStrokeDialog, SW_SHOW);
		UpdateWindow(hKeyStrokeDialog);
	}
}
#endif

/*
 *	�T�E���h�o�̓��[�h�؂�ւ�
 */
static void FASTCALL OnChgSound(void)
{
	LockVM();

	/* �T�E���h���[�h�ύX */
	uStereoOut = (uStereoOut + 1) % 5;
	SetStatusMessage(IDS_SND_MONAURAL + uStereoOut);

	/* �K�p */
	ApplySnd();
	UnlockVM();
}

/*
 *	�T�C�N���X�`�[�����[�h�؂�ւ�
 */
static void FASTCALL OnChgCycleSteal(void)
{
	LockVM();

	/* �T�C�N���X�`�[�����[�h��ύX */
	cycle_steal = !cycle_steal;
	cycle_steal_default = cycle_steal;

	/* subclock_mode��cycle_steal�̔��] */
	subclock_mode = !cycle_steal;

	GetCfg();

	/* �����ł�subclock_mode������K�v�͂Ȃ� */
	if (cycle_steal) {
		SetStatusMessage(IDS_ENABLE_CYCLESTEAL);
	}
	else {
		SetStatusMessage(IDS_DISABLE_CYCLESTEAL);
	}

	UnlockVM();
}

/*
 *	�c�[��(T)���j���[
 */
static BOOL FASTCALL OnTool(HWND hWnd, WORD wID)
{
	ASSERT(hWnd);

	switch (wID) {
		/* �ݒ� */
		case IDM_CONFIG:
			OnConfig(hWnd);
			return TRUE;

#if defined(MOUSE)
		/* �}�E�X���[�h�؂芷�� */
		case IDM_MOUSEMODE:
			if (bDetectMouse) {
				OnMouseMode();
			}
			return TRUE;

		/* �}�E�X���[�h�؂芷��(F11,���j���[��\��) */
		case IDM_MOUSEMODE2:
			if (!kbd_table[DIK_F11] && bDetectMouse) {
				OnMouseMode();
			}
			return TRUE;

		/* �}�E�X���[�h�L�� */
		case IDM_MOUSEON:
			if (bDetectMouse) {
				MouseModeChange(TRUE);
			}
			return TRUE;

		/* �}�E�X���[�h���� */
		case IDM_MOUSEOFF:
			if (bDetectMouse) {
				MouseModeChange(FALSE);
			}
			return TRUE;
#endif

#if XM7_VER >= 2
		/* �����A�W���X�g */
		case IDM_TIMEADJUST:
			OnTimeAdjust();
			return TRUE;
#endif

		/* ��ʃL���v�`�� */
		case IDM_GRPCAP:
			OnGrpCapture();
			return TRUE;

		/* ��ʃL���v�`��2 */
		case IDM_GRPCAP2:
			OnGrpCapture2();
			return TRUE;

		/* WAV�L���v�`�� */
		case IDM_WAVCAP:
			OnWavCapture(hWnd);
			return TRUE;

		/* �V�K�f�B�X�N */
		case IDM_NEWDISK:
			OnNewDisk(hWnd);
			return TRUE;

		/* �V�K�e�[�v */
		case IDM_NEWTAPE:
			OnNewTape(hWnd);
			return TRUE;

#if XM7_VER == 1
#if defined(BUBBLE)
		/* �V�K�o�u���J�Z�b�g */
		case IDM_NEWBUBBLE:
			OnNewBubble(hWnd);
			return TRUE;
#endif
#endif

		/* VFD��D77 */
		case IDM_VFD2D77:
			OnVFD2D77(hWnd);
			return TRUE;

		/* 2D��D77 */
		case IDM_2D2D77:
			On2D2D77(hWnd);
			return TRUE;

		/* VTP��T77 */
		case IDM_VTP2T77:
			OnVTP2T77(hWnd);
			return TRUE;

#if XM7_VER == 1
#if defined(BUBBLE)
		/* BBL��B77 */
		case IDM_BBL2B77:
			OnBBL2B77(hWnd);
			return TRUE;
#endif
#endif

#if defined(KBDPASTE)
		/* �\��t�� */
		case IDM_PASTE:
			OnPaste(hWnd);
			return TRUE;

		/* �L�[���͎x�� */
		case IDM_KEYSTROKE:
			OnKeyStroke(hWnd);
			return TRUE;
#endif

		/* �T�E���h�o�͐؂�ւ�(���j���[��\��) */
		case IDM_CHG_SOUNDMODE:
			OnChgSound();
			return TRUE;

		/* �T�C�N���X�`�[�����[�h�؂�ւ�(���j���[��\��) */
		case IDM_CYCLESTEAL:
			OnChgCycleSteal();
			return TRUE;
	}

	return FALSE;
}

/*
 *	�c�[��(T)���j���[�X�V
 */
static void FASTCALL OnToolPopup(HMENU hMenu)
{
#if defined(MOUSE)
	MENUITEMINFO mii;
	char string[128];
#endif
#if defined(KBDPASTE)
	UINT uitem;
#endif

	ASSERT(hMenu);

#if defined(MOUSE)
	/* ���j���[���ڂ��炢������폜 */
	DeleteMenu(hMenu, IDM_MOUSEMODE, MF_BYCOMMAND);

	/* ���߂č��ڂ�˂����� */
	if (kbd_table[DIK_F11]) {
		LoadString(hAppInstance, IDS_MOUSEMODE_DISF11, string, sizeof(string));
	}
	else {
		LoadString(hAppInstance, IDS_MOUSEMODE_ENBF11, string, sizeof(string));
	}

	memset(&mii, 0, sizeof(mii));
	mii.cbSize = 44;	/* sizeof(mii)��WINVER>=0x0500���� */
	mii.fMask = MIIM_TYPE | MIIM_STATE | MIIM_ID;
	mii.fType = MFT_STRING;
	mii.fState = MFS_ENABLED;
	mii.wID = IDM_MOUSEMODE;
	mii.dwTypeData = string;
	mii.cch = strlen(string);
#if XM7_VER >= 2
	InsertMenuItem(hMenu, IDM_TIMEADJUST, MF_BYCOMMAND, &mii);
#else
	InsertMenuItem(hMenu, 2, MF_BYPOSITION, &mii);
#endif
	EnableMenuItem(hMenu, IDM_MOUSEMODE, !bDetectMouse);

	/* �}�E�X���[�h */
	CheckMenuSub(hMenu, IDM_MOUSEMODE, mos_capture);
#endif

	/* WAV�L���v�`�� �n���h�������ŃI�[�v���� */
	CheckMenuSub(hMenu, IDM_WAVCAP, (hWavCapture >= 0));

#if defined(KBDPASTE)
	uitem = 12;
#if defined(MOUSE)
	uitem += 1;
#endif
#if XM7_VER >= 2
	uitem += 1;
#endif
#if XM7_VER == 1
#if defined(BUBBLE)
	uitem += 2;
#endif
#endif

	/* INI�t�@�C���ɂ��ݒ肪�Ȃ��ꍇ�A�\��t���@�\�����j���[���ڂ���폜 */
	if ((uPasteWait == 0) && (uPasteWaitCntl == 0)) {
		while (GetMenuItemCount(hMenu) > (int)uitem) {
			DeleteMenu(hMenu, uitem, MF_BYPOSITION);
		}
	}
#endif
}

/*-[ �E�B���h�E���j���[ ]---------------------------------------------------*/

/*
 *	�S�E�C���h�E�̕\����Ԃ��ꊇ���ĕύX
 */
static void FASTCALL ShowAllWindowSub(int CmdShow)
{
	int i;

	for (i=0; i<SWND_MAXNUM; i++) {
		if (hSubWnd[i]) {
			ShowWindow(hSubWnd[i], CmdShow);
		}
	}
}

/*
 *	�d�˂ĕ\��(C)
 */
static void FASTCALL OnCascade(void)
{
	/* �d�˂ĕ\�� */
	ASSERT(hDrawWnd);
	CascadeWindows(hDrawWnd, 0, NULL, 0, NULL);
}

/*
 *	���ׂĕ\��(T)
 */
static void FASTCALL OnTile(void)
{
	/* ���ׂĕ\�� */
	ASSERT(hDrawWnd);
	TileWindows(hDrawWnd, MDITILE_VERTICAL, NULL, 0, NULL);
}

/*
 *	�S�ăA�C�R����(I)
 */
static void FASTCALL OnIconic(void)
{
	ShowAllWindowSub(SW_MINIMIZE);
}

/*
 *	�A�C�R���̐���(A)
 */
static void FASTCALL OnArrangeIcon(void)
{
	/* �A�C�R���̐��� */
	ASSERT(hDrawWnd);
	ArrangeIconicWindows(hDrawWnd);
}

/*
 *	�S�ĉB��(H)
 */
static void FASTCALL OnHide(void)
{
	ShowAllWindowSub(SW_HIDE);
}

/*
 *	�S�ĕ���(R)
 */
static void FASTCALL OnRestore(void)
{
	ShowAllWindowSub(SW_RESTORE);

	if (bPopupSwnd) {
		SetForegroundWindow(hMainWnd);
	}
}

/*
 *	�S�ĕ���(O)
 */
static void FASTCALL OnClose(void)
{
	int i;

	for (i=0; i<SWND_MAXNUM; i++) {
		if (hSubWnd[i]) {
			DestroyWindow(hSubWnd[i]);
			hSubWnd[i] = NULL;
		}
	}
}

/*
 *	�E�B���h�E(W)���j���[
 */
static BOOL FASTCALL OnWindow(WORD wID)
{
	int i;

	switch (wID) {
		/* �d�˂ĕ\�� */
		case IDM_CASCADE:
			OnCascade();
			return TRUE;

		/* ���ׂĕ\�� */
		case IDM_TILE:
			OnTile();
			return TRUE;

		/* �S�ăA�C�R���� */
		case IDM_ICONIC:
			OnIconic();
			return TRUE;

		/* �A�C�R���̐��� */
		case IDM_ARRANGEICON:
			OnArrangeIcon();
			return TRUE;

		/* �S�ĉB�� */
		case IDM_ALLHIDE:
			OnHide();
			return TRUE;

		/* �S�ĕ��� */
		case IDM_ALLRESTORE:
			OnRestore();
			return TRUE;

		/* �S�ĕ��� */
		case IDM_ALLCLOSE:
			OnClose();
			return TRUE;
	}

	/* �E�B���h�E�I���� */
	if ((wID >= IDM_SWND00) && (wID <= IDM_SWND15)) {
		for (i=0; i<SWND_MAXNUM; i++) {
			if (hSubWnd[i] == NULL) {
				continue;
			}

			/* �J�E���g�_�E�����AOK */
			if (wID == IDM_SWND00) {
				ShowWindow(hSubWnd[i], SW_RESTORE);
				SetWindowPos(hSubWnd[i], HWND_TOP, 0, 0, 0, 0,
					SWP_NOMOVE | SWP_NOSIZE);
				break;
			}
			else {
				wID--;
			}
		}
	}

	return FALSE;
}

/*
 *	�E�B���h�E(W)���j���[�X�V
 */
static void FASTCALL OnWindowPopup(HMENU hMenu)
{
	int i;
	BOOL flag;
	MENUITEMINFO mii;
	UINT nID;
	int count;
	char string[128];

	ASSERT(hMenu);

	/* �t���X�N���[�����̐擪�̂V�̖����� */
	EnableMenuSub(hMenu, IDM_CASCADE, !bFullScreen && !bPopupSwnd);
	EnableMenuSub(hMenu, IDM_TILE, !bFullScreen && !bPopupSwnd);
	EnableMenuSub(hMenu, IDM_ICONIC, !bFullScreen);
	EnableMenuSub(hMenu, IDM_ARRANGEICON, !bFullScreen && !bPopupSwnd);
	EnableMenuSub(hMenu, IDM_ALLHIDE, !bFullScreen);
	EnableMenuSub(hMenu, IDM_ALLRESTORE, !bFullScreen);
	EnableMenuSub(hMenu, IDM_ALLCLOSE, !bFullScreen);

	/* �擪�̂V���c���č폜 */
	while (GetMenuItemCount(hMenu) > 7) {
		DeleteMenu(hMenu, 7, MF_BYPOSITION);
	}

	/* �L���ȃT�u�E�C���h�E���Ȃ���΁A���̂܂܃��^�[�� */
	flag = FALSE;
	for (i=0; i<SWND_MAXNUM; i++) {
		if (hSubWnd[i] != NULL) {
			flag = TRUE;
		}
	}
	if (!flag) {
		return;
	}

	/* ���j���[�\���̏����� */
	memset(&mii, 0, sizeof(mii));
	mii.cbSize = 44;	/* sizeof(mii)��WINVER>=0x0500���� */
	mii.fMask = MIIM_TYPE | MIIM_STATE | MIIM_ID;
	mii.fType = MFT_STRING;
	mii.fState = MFS_ENABLED;

	/* �Z�p���[�^�}�� */
	mii.fType = MFT_SEPARATOR;
	InsertMenuItem(hMenu, 7, TRUE, &mii);
	mii.fType = MFT_STRING;

	/* �E�C���h�E�^�C�g�����Z�b�g */
	count = 0;
	nID = IDM_SWND00;
	for (i=0; i<SWND_MAXNUM; i++) {
		if (hSubWnd[i] != NULL) {
			/* ���j���[�}�� */
			mii.wID = nID;
			memset(string, 0, sizeof(string));
			GetWindowText(hSubWnd[i], string, sizeof(string) - 1);
			mii.dwTypeData = string;
			mii.cch = strlen(string);
			InsertMenuItem(hMenu, count + 8, TRUE, &mii);
			EnableMenuSub(hMenu, nID, !bFullScreen);

			/* ���� */
			nID++;
			count++;
		}
	}
}

/*-[ �w���v���j���[ ]-------------------------------------------------------*/

/*
 *	�w���v(H)���j���[
 */
static BOOL FASTCALL OnHelp(HWND hWnd, WORD wID)
{
	switch (wID) {
		/* �o�[�W������� */
		case IDM_ABOUT:
			OnAbout(hWnd);
			return TRUE;
	}

	return FALSE;
}

/*-[ ���j���[�R�}���h���� ]-------------------------------------------------*/

/*
 *	���j���[�R�}���h����
 */
void FASTCALL OnCommand(HWND hWnd, WORD wID)
{
	ASSERT(hWnd);

	if (OnFile(hWnd, wID)) {
		return;
	}
	if (OnDisk(wID)) {
		return;
	}
	if (OnTape(wID)) {
		return;
	}
	if (OnView(hWnd, wID)) {
		return;
	}
	if (OnDebug(hWnd, wID)) {
		return;
	}
	if (OnTool(hWnd, wID)) {
		return;
	}
	if (OnWindow(wID)) {
		return;
	}
	if (OnHelp(hWnd, wID)) {
		return;
	}
}

/*
 *	���j���[�R�}���h�X�V����
 */
void FASTCALL OnMenuPopup(HWND hWnd, HMENU hSubMenu, UINT uPos)
{
	HMENU hMenu;

	ASSERT(hWnd);
	ASSERT(hSubMenu);

	/* ���C�����j���[�̍X�V���`�F�b�N */
	hMenu = GetMenu(hWnd);
	if (GetSubMenu(hMenu, uPos) != hSubMenu) {
		return;
	}

	/* ���b�N���K�v */
	LockVM();

	switch (uPos) {
		/* �t�@�C�� */
		case 0:
			OnFilePopup(hSubMenu);
			break;

		/* �h���C�u1 */
		case 1:
			OnDiskPopup(hSubMenu, 1);
			break;

		/* �h���C�u0 */
		case 2:
			OnDiskPopup(hSubMenu, 0);
			break;

		/* �e�[�v */
		case 3:
			OnTapePopup(hSubMenu);
			break;

		/* �\�� */
		case 4:
			OnViewPopup(hSubMenu);
			break;

		/* �f�o�b�O */
		case 5:
			OnDebugPopup(hSubMenu);
			break;

		/* �c�[�� */
		case 6:
			OnToolPopup(hSubMenu);
			break;

		/* �E�B���h�E */
		case 7:
			OnWindowPopup(hSubMenu);
			break;
	}

	/* �A�����b�N */
	UnlockVM();
}

/*-[ �h���b�O���h���b�v�E�R�}���h���C������ ]-------------------------------*/

/*
 *	�t�@�C���h���b�v�T�u
 */
void FASTCALL OnDropSub(char *path)
{
	char dir[_MAX_DIR];
	char ext[_MAX_EXT];
	char InitDir[_MAX_DRIVE + _MAX_PATH];

	ASSERT(path);

	/* �g���q�������� */
	_splitpath(path, InitDir, dir, NULL, ext);
	strncat(InitDir, dir, sizeof(InitDir) - strlen(InitDir) - 1);

	/* D77 */
	if (stricmp(ext, ".D77") == 0) {
		strncpy(InitialDir[0], InitDir, sizeof(InitialDir[0]));
		LockVM();
		StopSnd();
		fdc_setdisk(0, path);
		fdc_setdisk(1, NULL);
		if ((fdc_ready[0] != FDC_TYPE_NOTREADY) && (fdc_medias[0] >= 2)) {
			fdc_setdisk(1, path);
			fdc_setmedia(1, 1);
		}
		system_reset();
		OnRefresh(hMainWnd);
		PlaySnd();
		ResetSch();
		UnlockVM();
	}

	/* 2D/2DD/VFD */
#if XM7_VER >= 3
	if ((stricmp(ext, ".2D") == 0) || (stricmp(ext, ".2DD") == 0) ||
		(stricmp(ext, ".VFD") == 0)) {
#else
	if ((stricmp(ext, ".2D") == 0) || (stricmp(ext, ".VFD") == 0)) {
#endif
		strncpy(InitialDir[0], InitDir, sizeof(InitialDir[0]));
		LockVM();
		StopSnd();
		fdc_setdisk(0, path);
		fdc_setdisk(1, NULL);
		system_reset();
		OnRefresh(hMainWnd);
		PlaySnd();
		ResetSch();
		UnlockVM();
	}

	/* T77 */
	if (stricmp(ext, ".T77") == 0) {
		strncpy(InitialDir[1], InitDir, sizeof(InitialDir[0]));
		LockVM();
		tape_setfile(path);
		UnlockVM();
	}

#if XM7_VER == 1
#if defined(BUBBLE)
	if (bmc_enable) {
		/* B77 */
		if (stricmp(ext, ".B77") == 0) {
			strncpy(InitialDir[5], InitDir, sizeof(InitialDir[5]));
			LockVM();
			StopSnd();
			if (bmc_enable) {
				bmc_setfile(0, path);
				bmc_setfile(1, NULL);
				if (bmc_ready[0] != BMC_TYPE_NOTREADY) {
					if (bmc_medias[0] >= 2) {
						bmc_setfile(1, path);
						bmc_setmedia(1, 1);
					}
				}
			}
			PlaySnd();
			ResetSch();
			UnlockVM();
		}
	}

	/* BBL */
	if (stricmp(ext, ".BBL") == 0) {
		strncpy(InitialDir[5], InitDir, sizeof(InitialDir[5]));
		LockVM();
		bmc_setfile(0, path);
		bmc_setfile(1, NULL);
		UnlockVM();
	}
#endif
#endif

	/* XM7 */
	if (stricmp(ext, ".XM7") == 0) {
		strncpy(InitialDir[2], InitDir, sizeof(InitialDir[2]));
		LockVM();
		StopSnd();
		StateLoad(path);
		GetCfg();
		PlaySnd();
		ResetSch();
		UnlockVM();
	}

	/* �\�����e���X�V */
	if (hDrawWnd) {
		InvalidateRect(hDrawWnd, NULL, FALSE);
	}
}

/*
 *	�X�e�[�^�X�o�[
 *	�t�@�C���h���b�v�T�u
 */
void FASTCALL OnBarDropSub(char *path, POINT point)
{
	char dir[_MAX_DIR];
	char ext[_MAX_EXT];
	char InitDir[_MAX_DRIVE + _MAX_PATH];
	int drive;

	ASSERT(path);

	/* �g���q�������� */
	_splitpath(path, InitDir, dir, NULL, ext);
	strncat(InitDir, dir, sizeof(InitDir) - strlen(InitDir) - 1);

	/* �f�B�X�N�C���[�W */
	if ((point.x >= uPaneX[0]) && (point.x < uPaneX[2])) {
		/* �h���C�u���� */
		if (point.x >= uPaneX[1]) {
			drive = 0;
		}
		else {
			drive = 1;
		}

		/* D77 */
		if (stricmp(ext, ".D77") == 0) {
			strncpy(InitialDir[0], InitDir, sizeof(InitialDir[0]));
			LockVM();
			StopSnd();
			fdc_setdisk(drive, path);
			PlaySnd();
			ResetSch();
			UnlockVM();
			return;
		}

		/* 2D/2DD/VFD */
#if XM7_VER >= 3
		if ((stricmp(ext, ".2D") == 0) || (stricmp(ext, ".2DD") == 0) ||
			(stricmp(ext, ".VFD") == 0)) {
#else
		if ((stricmp(ext, ".2D") == 0) || (stricmp(ext, ".VFD") == 0)) {
#endif
			strncpy(InitialDir[0], InitDir, sizeof(InitialDir[0]));
			LockVM();
			StopSnd();
			fdc_setdisk(drive, path);
			PlaySnd();
			ResetSch();
			UnlockVM();
			return;
		}
	}

	/* �f�B�X�N�C���[�W�ȊO�E�͈͊O */
	OnDropSub(path);
}

/*
 *	�t�@�C���h���b�v
 */
void FASTCALL OnDropFiles(HANDLE hDrop)
{
	char path[_MAX_PATH];
	POINT point;
	POINT spoint;
	HWND hwnd;

	ASSERT(hDrop);

	/* �t�@�C�����󂯎�� */
	DragQueryPoint(hDrop, &point);
	DragQueryFile(hDrop, 0, path, sizeof(path));
	DragFinish(hDrop);

	/* ���� */
	spoint = point;
	ClientToScreen(hMainWnd, &spoint);
	hwnd = WindowFromPoint(spoint);
	if (hwnd == hStatusBar) {
		/* �X�e�[�^�X�o�[��ւ̃h���b�v */
		OnBarDropSub(path, point);
	}
	else {
		/* �h���[�E�B���h�E���ւ̃h���b�v�͏]������ */
		OnDropSub(path);
	}
}

/*
 *	�R�}���h���C������
 */
void FASTCALL OnCmdLine(LPSTR lpCmdLine)
{
	char dir[_MAX_DIR];
	char ext[_MAX_EXT];
	char path[_MAX_PATH];
	char fullpath[_MAX_PATH];
	char d77_path[_MAX_PATH];
	char InitDir[_MAX_PATH];
	LPSTR p;
	LPSTR q;
	BOOL flag;
	int drive;
#if XM7_VER == 1
#if defined(BUBBLE)
	int unit;
	char bbl_path[_MAX_PATH];
#endif
#endif
	BOOL tape_set;

	ASSERT(lpCmdLine);

	/* ���[�N������ */
	drive = 0;
#if XM7_VER == 1
#if defined(BUBBLE)
	unit = 0;
#endif
#endif
	tape_set = FALSE;
	d77_path[0] = '\0';

	/* �t�@�C�������X�L�b�v */
	p = lpCmdLine;
	flag = FALSE;
	for (;;) {
		/* �I���`�F�b�N */
		if (*p == '\0') {
			return;
		}

		/* �N�H�[�g�`�F�b�N */
		if (*p == '"') {
			flag = !flag;
		}

		/* �X�y�[�X�`�F�b�N */
		if ((*p == ' ') && !flag) {
			break;
		}

		/* ���� */
		p++;
	}

	/* VM�����b�N */
	LockVM();
	StopSnd();

	/* �R�}���h���C���I�v�V�������I���܂Ń��[�v */
	while (*p) {
		path[0] = '\0';

		/* �X�y�[�X��ǂݔ�΂� */
		for (;;) {
			if (*p != ' ') {
				break;
			}

			/* ���� */
			p++;
		}

		if (*p == '"') {
			/* �N�H�[�g�`�F�b�N */
			p++;
			q = path;

			/* �N�H�[�g���o��܂ő����� */
			for (;;) {
				*q = *p++;

				/* �N�H�[�g���I���`�F�b�N */
				if (*q == '\0') {
					path[0] = '\0';
					break;
				}

				/* �N�H�[�g�`�F�b�N */
				if (*q == '"') {
					*q = '\0';
					break;
				}

				/* ���� */
				q++;
			}
		}
		else if (*p) {
			/* �N�H�[�g�Ȃ� */
			q = path;

			/* �X�y�[�X���o�邩�����񂪏I������܂ő����� */
			for (;;) {
				*q = *p++;

				/* ��؂蕶���`�F�b�N */
				if ((*q == '\0') || (*q == ' ')) {
					*q = '\0';
					break;
				}

				/* ���� */
				q++;
			}
		}

		/* �I���`�F�b�N */
		if (!path[0]) {
			break;
		}

		/* �t���p�X�𐶐� */
		_fullpath(fullpath, path, _MAX_PATH);

		/* �g���q�������� */
		_splitpath(fullpath, InitDir, dir, NULL, ext);
		strncat(InitDir, dir, sizeof(InitDir) - strlen(InitDir) - 1);

		/* D77/2D/2DD/VFD */
#if XM7_VER >= 3
		if ((stricmp(ext, ".D77") == 0) || (stricmp(ext, ".VFD") == 0) ||
			(stricmp(ext, ".2D") == 0) || (stricmp(ext, ".2DD") == 0)) {
#else
		if ((stricmp(ext, ".D77") == 0) || (stricmp(ext, ".VFD") == 0) ||
			(stricmp(ext, ".2D") == 0)) {
#endif

			/* 2���܂Ń}�E���g�\ */
			if (drive < 2) {
				if (drive == 0) {
					/* 1���ڂ̏ꍇ�A�����p�X��ۑ� */
					strncpy(InitialDir[0], InitDir, sizeof(InitialDir[0]));

					/* D77��1�����w�肵���ꍇ�̂��߂Ƀt�@�C������ۑ� */
					if (stricmp(ext, ".D77") == 0) {
						strncpy(d77_path, fullpath, sizeof(d77_path));
					}
				}

				/* �f�B�X�N���}�E���g���� */
				fdc_setdisk(drive, fullpath);

				/* �}�E���g�����Ȃ�h���C�u�ԍ�(=�}�E���g����)��+1 */
				if (fdc_ready[drive] != FDC_TYPE_NOTREADY) {
					drive ++;
				}
			}
		}

		/* T77 */
		if (stricmp(ext, ".T77") == 0) {
			/* �L���Ȃ̂�1�̂� */
			if (!tape_set) {
				/* �����p�X��ۑ� */
				strncpy(InitialDir[1], InitDir, sizeof(InitialDir[1]));

				/* �e�[�v���}�E���g */
				tape_setfile(fullpath);

				/* �}�E���g�����Ȃ�e�[�v�}�E���g�֎~�t���O�𗧂Ă� */
				if (tape_fileh != -1) {
					tape_set = TRUE;
				}
			}
		}

#if XM7_VER == 1
#if defined(BUBBLE)
		/* B77/BBL */
		if (((stricmp(ext, ".B77") == 0) || (stricmp(ext, ".BBL") == 0)) &&
			bmc_enable) {

			if (unit == 0) {
				/* 1�ڂ̏ꍇ�A�����p�X��ۑ� */
				strncpy(InitialDir[5], InitDir, sizeof(InitialDir[5]));

				/* BBL��1�����w�肵���ꍇ�̂��߂Ƀt�@�C������ۑ� */
				if (stricmp(ext, ".BBL") == 0) {
					strncpy(bbl_path, fullpath, sizeof(bbl_path));
				}

				/* ���j�b�g1�̃}�E���g������ */
				bmc_setfile(0, NULL);
				bmc_setfile(1, NULL);
			}

			/* 2�܂Ń}�E���g�\ */
			if (unit < 2 && bmc_enable) {
				/* �o�u���J�Z�b�g���}�E���g���� */
				bmc_setfile(unit, path);

				/* �}�E���g�����Ȃ烆�j�b�g�ԍ�(=�}�E���g��)��+1 */
				if (bmc_ready[unit] != BMC_TYPE_NOTREADY) {
					unit ++;
				}
			}
		}
#endif
#endif

		/* XM7 */
		if (stricmp(ext, ".XM7") == 0) {
			strncpy(InitialDir[2], InitDir, sizeof(InitialDir[2]));
			StateLoad(fullpath);
			GetCfg();

			/* �X�e�[�g���[�h���Ƀ}�E���g���s����̂ŏ�����ł��؂� */
			drive = 0;
#if XM7_VER == 1
#if defined(BUBBLE)
			unit = 0;
#endif
#endif
			break;
		}
	}

	/* �f�B�X�N�C���[�W�t�@�C���w�萔��1�̏ꍇ�̓��ꏈ�� */
	if (drive == 1) {
		/* �h���C�u1�̃}�E���g������ */
		fdc_setdisk(1, NULL);

		/* D77�̏ꍇ�A2���ڂ�����΃h���C�u1�Ƀ}�E���g */
		if (d77_path[0]) {
			if ((fdc_ready[0] != FDC_TYPE_NOTREADY) && (fdc_medias[0] >= 2)) {
				fdc_setdisk(1, d77_path);
				fdc_setmedia(1, 1);
			}
		}
	}

#if XM7_VER == 1
#if defined(BUBBLE)
	/* �o�u���C���[�W�t�@�C���w�萔��1�̏ꍇ�̓��ꏈ�� */
	if (unit == 1) {
		/* B77�̏ꍇ�A2�ڂ�����΃h���C�u1�Ƀ}�E���g */
		if (bbl_path[0]) {
			if ((bmc_ready[0] != BMC_TYPE_NOTREADY) && (bmc_medias[0] >= 2)) {
				bmc_setfile(1, bbl_path);
				bmc_setmedia(1, 1);
			}
		}
	}
#endif
#endif

	/* �f�B�X�N�C���[�W���}�E���g���ꂽ�ꍇ�AVM���Z�b�g */
	if (drive >= 1) {
		system_reset();
		OnRefresh(hMainWnd);
	}

	/* VM���A�����b�N */
	PlaySnd();
	ResetSch();
	UnlockVM();

	/* �\�����e���X�V */
	if (hDrawWnd) {
		InvalidateRect(hDrawWnd, NULL, FALSE);
	}
}

#endif	/* _WIN32 */
