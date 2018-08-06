/*
 *	FM-7 EMULATOR "XM7"
 *
 *	Copyright (C) 1999-2017 �o�h�D(Twitter:@xm6_original)
 *	Copyright (C) 2001-2017 Ryu Takegami (Twitter:@RyuTakegami)
 *
 *	[ Win32API ���C���v���O���� ]
 *
 *	RHG����
 *	  2001.12.25		Draw�E�B���h�E��IME���֎~����悤�ύX(�o�h�D)
 *						�T�u�E�B���h�E�t�H���g�̃p�����[�^��ύX(�o�h�D)
 *	  2002.01.24		�I�����̏����������u�R���|�[�l���g�̃N���[���A�b�v��
 *						�ݒ�t�@�C���̕ۑ��v�ɕύX
 *	  2002.04.05		�N�����ɃT�C�Y�␳�O�̃E�B���h�E�������Ȃ��悤�ɏC��
 *	  2002.04.25		WinNT�n�œ�d�N�����̃R�}���h���C���p�����[�^����ɋN
 *						�����Ă���C���X�^���X�ɓn���Ȃ������C��
 *	  2002.05.01		�]���o�[�W�����Ƃ̌݊����m�ۂ̂���Win9x�n�ł̃R�}���h
 *						���C���n����WM_USER+1���g�������@�ɖ߂�
 *	  2002.08.15		�ꕔ�L�[�ɂ���WM_KEYDOWN�ł̃L�[�����󋵎擾�ɑΉ�
 *	  2002.08.29		�A�N�e�B�u�����ɃE�B���h�E�̍ĕ`����s���悤�ɕύX
 *						(�^�v���W�F�N�g�p�J�X�^���ł���t�A�� ^^;)
 *	  2002.09.11		�X�e�[�^�X���b�Z�[�W���������ɃV�X�e���^�C�}�[�𗘗p��
 *						��悤�ɕύX
 *	  2002.09.17		NT�n�ŃT�u�E�B���h�E�𕜌������ꍇ�Ɉꕔ�̃L�[��������
 *						���Ȃ�����C��
 *						���C���E�B���h�E�폜���ɃT�u�E�B���h�E���폜����悤��
 *						�ύX
 *	  2002.11.13		�X���b�h�D�揇�ʂ�ABOVE NORMAL�ɕύX
 *	  2002.12.11		DLL���J�����Y��Ă��������C��(�c�c�c)
 *	  2003.01.19		F10�ׂ���F10�ɃL�[�R�[�h�����蓖�Ă��Ă��鎞�̂ݍs��
 *						�悤�ɕύX
 *	  2003.01.23		ROMEO����t���O���R�P������C��
 *	  2003.02.10		V2�߂��ŋN������Ƀ`�����l���R�[���ݒ肪�ݒ�_�C�A��
 *						�O�ɔ��f����Ȃ������C��
 *	  2003.04.22		NT�΍��Break�����ɃV�X�e���^�C�}�𗘗p����悤�ɕύX
 *	  2003.05.11		DLL�ǂݍ��ݏ����̏����̖��Ńr�W���A���X�^�C���K�p�`
 *						�F�b�N�Ɏ��s��������C��(XP only)
 *	  2003.05.25		Win2000/XP�ŃV�X�e���J���[�ύX���ɃX�e�[�^�X�o�[�̐F
 *						���ς��Ȃ����Ƃ���������C��
 *						��ʃf�U�C���ύX���̃X�e�[�^�X�o�[�̃T�C�Y�ύX�ɑΉ�
 *	  2003.07.25		�E�B���h�E�ʒu�̕ۑ��E�����ɑΉ�
 *	  2003.09.23		NT�n��SHIFT+CAP�����͂ł��Ȃ������C��
 *	  2004.03.29		PC-9801�L�[�{�[�h�ŃJ�i�L�[���g�p�ł��Ȃ������C��
 *	  2004.05.03		IME�T�|�[�g�֐���DLL�ǂݍ��݃`�F�b�N�̈��S��������
 *	  2004.08.30		�N������̃G���[���b�Z�[�W�\�����ɓ�d�N���ł��Ă���
 *						�����ɑ΍�
 *						���z�}�V���EDirectX�̏��������s���ɃE�B���h�E��\����
 *						�Ȃ��悤�ɕύX
 *	  2004.09.13		�N�����̃E�C���h�E�T�C�Y�����AdjustWindowRect�𗘗p
 *						����悤�ɕύX
 *	  2004.10.20		NT�n��SHIFT+�J�^�J�i/�Ђ炪�Ȃ������Ȃ������C��
 *	  2005.10.15		�N������Ƀ}�E�X�L���v�`�����L���ɂȂ�Ȃ������C��
 *	  2010.01.14		Vista�ȍ~�ł̉E�V�t�g�L�[���ւ̑΍���s����
 *	  2010.01.20		WindowsVista/Windows7�ŋt�A�Z���u���E�C���h�E���J����
 *						�n���O������̉����̂��߃T�u�E�C���h�E�`���Ɨ���
 *	  2010.02.18		XM7���s���͌Œ�L�[�@�\�𖳌�������悤�ɕύX
 *	  2010.06.19		�ŏ������ɃL�[���͂��󂯕t��������C��
 *	  2010.08.02		�L�[�{�[�h�|�[�����O������w32_sch.c����ړ�
 *	  2010.10.03		2�{�g��\�����[�h�ɑΉ�
 *	  2011.01.26		�X�e�[�^�X�o�[��\�����ɃE�B���h�E���s�蒷�ɂ̂т���
 *						���C��
 *	  2012.03.06		���C�����[�v������ύX
 *	  2012.05.01		�t���X�N���[����ԕۑ������ւ̑Ή�
 *	  2012.06.03		�X�e�[�^�X�o�[�̕\����Ԃ̕ۑ��ɑΉ�
 *	  2012.07.14		������Ή��p�O��DLL�ɑΉ�
 *	  2012.10.10		Windows 8�ł�DrawMenuBar API�̋����ύX�ɑ΍�
 *						�N������OS�o�[�W�����`�F�b�N��������
 *	  2014.06.28		������Ή��p�O��DLL�ւ̑Ή���p�~
 *	  2017.06.17		�˔��q���Ȃ�������Ή��p�O��DLL�ւ̑Ή����ĊJ
 *	  2017.11.24		������Ή��p�O��DLL�ւ̑Ή������S�ɔp�~
 */

#ifdef _WIN32

#define STRICT
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <commctrl.h>
#include <imm.h>
#include <shellapi.h>
#include <stdlib.h>
#include <assert.h>
#include <mmsystem.h>
#define DIRECTINPUT_VERSION		0x0300		/* DirectX3���w�� */
#include <dinput.h>
#include "xm7.h"
#include "mouse.h"
#include "tapelp.h"
#include "keyboard.h"
#include "w32.h"
#include "w32_bar.h"
#include "w32_draw.h"
#include "w32_dd.h"
#include "w32_gdi.h"
#include "w32_kbd.h"
#include "w32_sch.h"
#include "w32_snd.h"
#include "w32_res.h"
#include "w32_sub.h"
#include "w32_cfg.h"
#include "w32_comm.h"

#if defined(ROMEO)
#include "juliet.h"
#endif
#if defined(MIDI)
#include "w32_midi.h"
#endif


/*
 *	�O���[�o�� ���[�N
 */
HINSTANCE hAppInstance;					/* �A�v���P�[�V���� �C���X�^���X */
HWND hMainWnd;							/* ���C���E�C���h�E */
HWND hDrawWnd;							/* �h���[�E�C���h�E */
int nErrorCode;							/* �G���[�R�[�h */
BOOL bMenuLoop;							/* ���j���[���[�v�� */
BOOL bMenuExit;							/* ���j���[�����o���t���O */
BOOL bCloseReq;							/* �I���v���t���O */
LONG lCharWidth;						/* �L�����N�^���� */
LONG lCharHeight;						/* �L�����N�^�c�� */
BOOL bSync;								/* ���s�ɓ��� */
BOOL bSyncDisasm[4];					/* �t�A�Z���u����PC�ɓ��� */
BOOL bMinimize;							/* �ŏ����t���O */
BOOL bActivate;							/* �A�N�e�B�x�[�g�t���O */
BOOL bHideStatus;						/* �X�e�[�^�X�o�[���B�� */
HICON hAppIcon;							/* �A�C�R���n���h�� */
int nAppIcon;							/* �A�C�R���ԍ�(1,2,3) */
BOOL bNTflag;							/* ����OS�^�C�v1(NT) */
BOOL bXPflag;							/* ����OS�^�C�v2(XP) */
BOOL bVistaflag;						/* ����OS�^�C�v3(Vista/7) */
BOOL bWin7flag;							/* ����OS�^�C�v4(Windows 7) */
BOOL bWin8flag;							/* ����OS�^�C�v5(Windows 8) */
BOOL bWin10flag;						/* ����OS�^�C�v6(Windows 10) */
BOOL bMMXflag;							/* MMX�T�|�[�g�t���O */
BOOL bCMOVflag;							/* CMOV�T�|�[�g�t���O(���󖢎g�p) */
BOOL bHighPriority;						/* �n�C�v���C�I���e�B�t���O */
POINT WinPos;							/* �E�B���h�E�ʒu */
#if defined(ROMEO)
BOOL bRomeo;							/* ROMEO�F���t���O */
#endif
#if XM7_VER <= 2 && defined(FMTV151)
BOOL bFMTV151;							/* V2�`�����l���R�[���t���O */
#endif
HFONT hFont;							/* �T�u�E�B���h�E�p�t�H���g�n���h�� */

/*
 *	�X�^�e�B�b�N ���[�N
 */
static CRITICAL_SECTION CSection;		/* �N���e�B�J���Z�N�V���� */
static BOOL (WINAPI *WINNLSEnableIME)(HWND, BOOL);
										/* IME�L��/�����ݒ�pAPI�֐� */
static HINSTANCE hInstDLL;				/* user32.dll�̃n���h�� */

/*
 *	�A�Z���u���֐��̂��߂̃v���g�^�C�v�錾
 */
extern BOOL CheckMMX(void);				/* MMX�Ή��`�F�b�N */
extern BOOL CheckCMOV(void);			/* CMOV�Ή��`�F�b�N(���ݖ��g�p) */

/*-[ IME�T�|�[�g ]----------------------------------------------------------*/

/*
 *	IME�L��/�����ݒ�API
 *	DLL������
 */
void FASTCALL InitIMEDLL(void)
{
	/* user32.dll�����[�h */
	hInstDLL = LoadLibrary("user32.dll");

	/* �֐��̐擪�A�h���X��ݒ� */
	if (hInstDLL) {
		WINNLSEnableIME = (BOOL (WINAPI *)(HWND, BOOL))GetProcAddress(
			hInstDLL, "WINNLSEnableIME");

		if (!WINNLSEnableIME) {
			/* ���s */
			CleanIMEDLL();
		}
	}
}

/*
 *	IME�L��/�����ݒ�API
 *	DLL�N���[���A�b�v
 */
void FASTCALL CleanIMEDLL(void)
{
	/* DLL���ǂݍ��܂�Ă�����J�� */
	if (hInstDLL) {
		FreeLibrary(hInstDLL);
	}

	WINNLSEnableIME = NULL;
	hInstDLL = NULL;
}

/*
 *	IME�L��/�����؂芷�� 09/09
 */
BOOL FASTCALL EnableIME(HWND hWnd, BOOL flag)
{
	/* WINNLSEnableIME�֐����ǂݍ��߂Ă���ꍇ�͎��s */
	if (hInstDLL && WINNLSEnableIME) {
		return WINNLSEnableIME(hWnd, flag);
	}

	/* ���������̂܂ܕԂ�l�Ƃ��� */
	return flag;
}

/*-[ �T�u�E�B���h�E�T�|�[�g ]-----------------------------------------------*/

/*
 *	�e�L�X�g�t�H���g�쐬
 *	���Ăяo������DeleteObject���邱��
 */
HFONT FASTCALL CreateTextFont(void)
{
	HFONT hFont;

	/* ���ꔻ�肵�āA�V�t�gJIS�EANSI�ǂ��炩�̃t�H���g����� */
	if (PRIMARYLANGID(GetUserDefaultLangID()) == LANG_JAPANESE) {
		/* ���{�� */
		hFont = CreateFont(-12, 0, 0, 0, FW_DONTCARE, FALSE, FALSE, FALSE,
			SHIFTJIS_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
			DEFAULT_QUALITY, FIXED_PITCH, "MS Gothic");
	}
	else {
		/* �p�� */
		hFont = CreateFont(-11, 0, 0, 0, FW_DONTCARE, FALSE, FALSE, FALSE,
			ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
			DEFAULT_QUALITY, FIXED_PITCH, NULL);
	}

	ASSERT(hFont);
	return hFont;
}

/*
 *	�T�u�E�B���h�E�Z�b�g�A�b�v
 */
static void FASTCALL SetupSubWnd(HWND hWnd)
{
	HFONT hBackup;
	HDC hDC;
	TEXTMETRIC tm;

	ASSERT(hWnd);

	/* �E�C���h�E���[�N�������� */
	memset(hSubWnd, 0, sizeof(hSubWnd));
	memset(bShowSubWindow, 0, sizeof(bShowSubWindow));
	InitSubWndWork();

	/* DC�擾�A�t�H���g�Z���N�g */
	hDC = GetDC(hWnd);
	ASSERT(hDC);
	hBackup = SelectObject(hDC, hFont);
	ASSERT(hBackup);

	/* �e�L�X�g���g���b�N�擾 */
	GetTextMetrics(hDC, &tm);

	/* DC�N���[���A�b�v */
	SelectObject(hDC, hBackup);
	ReleaseDC(hWnd, hDC);

	/* ���ʂ��X�g�A */
	lCharWidth = tm.tmAveCharWidth;
	lCharHeight = tm.tmHeight + tm.tmExternalLeading;
}

/*-[ ���� ]-----------------------------------------------------------------*/

/*
 *	VM�����b�N
 */
void FASTCALL LockVM(void)
{
	EnterCriticalSection(&CSection);
}

/*
 *	VM���A�����b�N
 */
void FASTCALL UnlockVM(void)
{
	LeaveCriticalSection(&CSection);
}

/*-[ �h���[�E�C���h�E ]-----------------------------------------------------*/

/*
 * 	�E�C���h�E�v���V�[�W��
 */
static LRESULT CALLBACK DrawWndProc(HWND hWnd, UINT message,
								 WPARAM wParam,LPARAM lParam)
{
	/* ���b�Z�[�W�� */
	switch (message) {
		/* �E�C���h�E�쐬 */
		case WM_CREATE:
			/* IME���֎~���� */
			ImmAssociateContext(hWnd, (HIMC)NULL);
			break;

		/* �E�C���h�E�w�i�`�� */
		case WM_ERASEBKGND:
			/* �G���[�Ȃ��Ȃ�A�w�i�`�悹�� */
			if (nErrorCode == 0) {
				return 0;
			}
			break;

		/* �E�C���h�E�ĕ`�� */
		case WM_PAINT:
			/* ���b�N���K�v */
			LockVM();
			OnPaint(hWnd);
			UnlockVM();
			return 0;

		/* ���j���[�`�F�b�N (���C���E�B���h�E�ɑ΂��ă��b�Z�[�W���Ĕ��s����) */
		case WM_NCMOUSEMOVE:
		case WM_MOUSEMOVE:
			if (bWin8flag) {
				/* Windows 8���Ƃǂ����]���̏����ł��܂��s���Ȃ��̂� */
				PostMessage(hMainWnd, message, wParam, lParam);
			}
			else {
				PostMessage(hMainWnd, message, wParam, 
					MAKELPARAM(LOWORD(lParam), (HIWORD(lParam) +
					GetSystemMetrics(SM_CYMENU))));
			}
			return 0;
	}

	/* �f�t�H���g �E�C���h�E�v���V�[�W�� */
	return DefWindowProc(hWnd, message, wParam, lParam);
}

/*
 *	�h���[�E�C���h�E�쐬
 */
static HWND FASTCALL CreateDraw(HWND hParent)
{
	WNDCLASSEX wcex;
	char szWndName[] = "XM7_Draw";
	RECT rect;
	HWND hWnd;

	ASSERT(hParent);

	/* �e�E�C���h�E�̋�`���擾 */
	GetClientRect(hParent, &rect);

	/* �E�C���h�E�N���X�̓o�^ */
	memset(&wcex, 0, sizeof(wcex));
	wcex.cbSize = sizeof(wcex);
	wcex.style = CS_VREDRAW | CS_HREDRAW;
	wcex.lpfnWndProc = DrawWndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hAppInstance;
	wcex.hIcon = NULL;
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	wcex.lpszMenuName = NULL;
	wcex.lpszClassName = szWndName;
	wcex.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
	RegisterClassEx(&wcex);

	/* �E�C���h�E�쐬 */
	hWnd = CreateWindow(szWndName,
						szWndName,
						WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN,
						0,
						0,
						640,
						400,
						hParent,
						NULL,
						hAppInstance,
						NULL);

	/* ���ʂ������A�� */
	return hWnd;
}

/*-[ ���C���E�C���h�E ]-----------------------------------------------------*/

/*
 *	�E�C���h�E�쐬
 */
static void FASTCALL OnCreate(HWND hWnd)
{
	BOOL flag;

	ASSERT(hWnd);

	/* �N���e�B�J���Z�N�V�����쐬 */
	InitializeCriticalSection(&CSection);

	/* IME���֎~���� */
	ImmAssociateContext(hWnd, (HIMC)NULL);

	/* �t�@�C���h���b�v���� */
	DragAcceptFiles(hWnd, TRUE);

	/* ���[�N�G���A������ */
	nErrorCode = 0;
	bMenuLoop = FALSE;
	bCloseReq = FALSE;
	bSync = TRUE;
	bSyncDisasm[0] = TRUE;
	bSyncDisasm[1] = TRUE;
#if XM7_VER == 1
#if defined(JSUB)
	bSyncDisasm[2] = TRUE;
#endif
#if defined(Z80CARD)
	bSyncDisasm[3] = TRUE;
#endif
#endif
	bMinimize = FALSE;
	bActivate = FALSE;
	bHideStatus = FALSE;
	WinPos.x = -99999;
	WinPos.y = -99999;

	/* �T�u�E�B���h�E���� */
	SetupSubWnd(hWnd);

	/* ROMEO�J�n���� by ���� */
#if defined(ROMEO)
	juliet_load();
	if (juliet_prepare() == 0) {
		bRomeo = TRUE;
		juliet_YMF288Reset();
	}
	else {
		bRomeo = FALSE;
	}
#endif

	/* �h���[�E�C���h�E�A�X�e�[�^�X�o�[���쐬 */
	hDrawWnd = CreateDraw(hWnd);
	hStatusBar = CreateStatus(hWnd);

	/* �R���|�[�l���g������ */
	LoadCfg();
	InitDraw();
	InitSnd();
	InitKbd();
	InitSch();
#if defined(FDDSND)
	InitFDDSnd();
#endif

	/* ���z�}�V�������� */
	if (!system_init()) {
		nErrorCode = 1;
		PostMessage(hWnd, WM_USER, 0, 0);
		return;
	}

	/* ����A���Z�b�g */
	ApplyCfg();
	system_reset();

	/* �R���|�[�l���g�Z���N�g */
	flag = TRUE;
	if (!SelectDraw(hDrawWnd)) {
		flag = FALSE;
	}
	if (!SelectSnd(hWnd)) {
		flag = FALSE;
	}
	if (!SelectKbd(hWnd)) {
		flag = FALSE;
	}
	if (!SelectSch()) {
		flag = FALSE;
	}

	/* �G���[�R�[�h���Z�b�g�����A�X�^�[�g */
	if (!flag) {
		nErrorCode = 2;
	}
	PostMessage(hWnd, WM_USER, 0, 0);
}

/*
 *	�E�C���h�E�N���[�Y
 */
static void FASTCALL OnClose(HWND hWnd)
{
	int i;

	ASSERT(hWnd);

	/* �T�E���h��~ */
	StopSnd();

	/* ���C���E�C���h�E����x����(�^�X�N�o�[�΍�) */
	ShowWindow(hWnd, SW_HIDE);

	/* �t���O�A�b�v */
	LockVM();
	bCloseReq = TRUE;
	UnlockVM();

	/* �T�u�E�C���h�E���ɔj�� */
	for (i=0; i<SWND_MAXNUM; i++) {
		if (hSubWnd[i]) {
			DestroyWindow(hSubWnd[i]);
		}
	}
}

/*
 *	�E�C���h�E�폜
 */
static void FASTCALL OnDestroy(void)
{
	/* �T�E���h��~ */
//	StopSnd();

	/* INI�t�@�C���������� */
	SaveCfg();

	/* �R���|�[�l���g �N���[���A�b�v */
#if defined(MIDI)
	CleanMIDI();
#endif
#if defined(FDDSND)
	CleanFDDSnd();
#endif
#if defined(RSC)
	CleanCommPort();
#endif
	CleanSch();
	CleanKbd();
	CleanSnd();
	CleanDraw();
	
	/* ROMEO�I������ by ���� */
#if defined(ROMEO)
	if (bRomeo) {
		juliet_YMF288Reset();
		Sleep(1);
	}
	juliet_unload();
	Sleep(1);
#endif

	/* �N���e�B�J���Z�N�V�����폜 */
	DeleteCriticalSection(&CSection);

	/* ���z�}�V�� �N���[���A�b�v */
	system_cleanup();
}

/*
 *	WM_QUIT����
 */
void FASTCALL OnQuit(void)
{
	/* �E�C���h�E�n���h�����N���A */
	hMainWnd = NULL;
	hDrawWnd = NULL;
	hStatusBar = NULL;
}

/*
 *	�T�C�Y�ύX
 */
void FASTCALL OnSize(HWND hWnd, WORD cx, WORD cy)
{
	RECT crect;
	RECT wrect;
	RECT trect;
	RECT srect;
	int width;
	int height;
	int wheight;

	ASSERT(hWnd);
	ASSERT(hDrawWnd);

	/* �ŏ����̏ꍇ�́A�������Ȃ� */
	if ((cx == 0) && (cy == 0)) {
		return;
	}

	/* �t���X�N���[���������l */
	if (bFullScreen) {
		return;
	}

	/* �c�[���o�[�A�X�e�[�^�X�o�[�̗L�����l���ɓ���A�v�Z */
	if (bDoubleSize) {
		width = 1280;
		height = 800;
	}
	else {
		width = 640;
		height = 400;
	}
	memset(&trect, 0, sizeof(trect));
	if (IsWindowVisible(hStatusBar)) {
		GetWindowRect(hStatusBar, &srect);
		wheight = height + (srect.bottom - srect.top);
	}
	else {
		memset(&srect, 0, sizeof(srect));
		wheight = height;
	}

	/* �N���C�A���g�̈�̃T�C�Y���擾 */
	GetClientRect(hWnd, &crect);

	/* �v���T�C�Y�Ɣ�r���A�␳ */
	if ((crect.right != width) || (crect.bottom != wheight)) {
		GetWindowRect(hWnd, &wrect);
		wrect.right -= wrect.left;
		wrect.bottom -= wrect.top;
		wrect.right -= crect.right;
		wrect.bottom -= crect.bottom;
		wrect.right += width;
		wrect.bottom += wheight;
		MoveWindow(hWnd, wrect.left, wrect.top, wrect.right, wrect.bottom, TRUE);
	}

	/* ���C���E�C���h�E�z�u */
	MoveWindow(hDrawWnd, 0, trect.bottom, width, height, TRUE);

	/* �X�e�[�^�X�o�[�z�u */
	if (IsWindowVisible(hStatusBar)) {
		MoveWindow(hStatusBar, 0, trect.bottom + height, width,
					(srect.bottom - srect.top), TRUE);
		SizeStatus(width);
	}
}

/*
 *	�L�b�N(���j���[�ɂ͖���`)
 */
static void FASTCALL OnKick(HWND hWnd)
{
	char buffer[128];

	buffer[0] = '\0';

	/* �G���[�R�[�h�� */
	switch (nErrorCode) {
		/* �G���[�Ȃ� */
		case 0:
			/* ���s�J�n */
			stopreq_flag = FALSE;
			run_flag = TRUE;

			/* �R�}���h���C������ */
			OnCmdLine(GetCommandLine());

#if defined(MOUSE)
			/* �}�E�X�L���v�`���J�n */
			SetMouseCapture(TRUE);
#endif
			break;

		/* VM�������G���[ */
		case 1:
			LoadString(hAppInstance, IDS_VMERROR, buffer, sizeof(buffer));
			MessageBox(hWnd, buffer, "XM7", MB_ICONSTOP | MB_OK);
			PostMessage(hWnd, WM_CLOSE, 0, 0);
			break;

		/* �R���|�[�l���g�������G���[ */
		case 2:
			LoadString(hAppInstance, IDS_DXERROR, buffer, sizeof(buffer));
			MessageBox(hWnd, buffer, "XM7", MB_ICONSTOP | MB_OK);
			PostMessage(hWnd, WM_CLOSE, 0, 0);
			break;
	}
}


/*
 * 	�E�C���h�E�v���V�[�W��
 */
static LRESULT CALLBACK WndProc(HWND hWnd, UINT message,
								 WPARAM wParam,LPARAM lParam)
{
	PAINTSTRUCT ps;
	BYTE key_code;
	RECT rect;

	/* ���b�Z�[�W�� */
	switch (message) {
		/* �E�C���h�E�쐬 */
		case WM_CREATE:
			OnCreate(hWnd);
			break;

		/* �E�B���h�E�N���[�Y */
		case WM_CLOSE:
			OnClose(hWnd);
			break;

		/* �E�C���h�E�폜 */
		case WM_DESTROY:
			OnDestroy();
			PostQuitMessage(0);
			return 0;

		/* �E�B���h�E�ړ� */
		case WM_MOVE:
			if (!bFullScreen && (!(GetWindowLong(hWnd, GWL_STYLE) &
				(WS_MAXIMIZE | WS_MINIMIZE)))) {
				GetWindowRect(hWnd, &rect);
				WinPos.x = rect.left;
				WinPos.y = rect.top;
			}
			break;

		/* �E�C���h�E�T�C�Y�ύX */
		case WM_SIZE:
			/* �ŏ������̃L�[���͑΍� */
			if (wParam == SIZE_MINIMIZED) {
				bMinimize = TRUE;
			}
			else {
				bMinimize = FALSE;
			}
			OnSize(hWnd, LOWORD(lParam), HIWORD(lParam));
			break;

		/* �E�C���h�E�ĕ`�� */
		case WM_PAINT:
			/* ���b�N���K�v */
			LockVM();
			BeginPaint(hWnd, &ps);
			PaintStatus();
			EndPaint(hWnd, &ps);
			UnlockVM();
			return 0;

		/* ���[�U+0:�X�^�[�g */
		case WM_USER + 0:
			OnKick(hWnd);
			return 0;

		/* ���[�U+1:�R�}���h���C��(Win9x) */
		case WM_USER + 1:
			if (!bNTflag) {
				OnCmdLine((LPSTR)wParam);
			}
			break;

#if defined(MOUSE)
		/* ���[�U+2:�}�E�X�L���v�`��OFF */
		case WM_USER + 2:
			SetMouseCapture(FALSE);
			break;

		/* ���[�U+3:�}�E�X�L���v�`��ON */
		case WM_USER + 3:
			SetMouseCapture(TRUE);
			break;
#endif

		/* �R�}���h���C��(WinNT) */
		case WM_COPYDATA:
			if (bNTflag) {
				if (((PCOPYDATASTRUCT)lParam)->dwData == 0x01374d58) {
					OnCmdLine((LPSTR)((PCOPYDATASTRUCT)lParam)->lpData);
				}
			}
			break;

#ifndef DINPUT8
		case WM_KEYDOWN:
			if ((LOBYTE(HIWORD(lParam)) == 0x36) && bVistaflag) {
				/* Vista�ȍ~�̉E�V�t�g�΍� */
				bNTkeyPushFlag[KNT_RSHIFT] = TRUE;
				return 0;
			}
			break;
#endif

		case WM_SYSKEYUP:
		case WM_KEYUP:
			if ((LOBYTE(wParam) == VK_F10) && kbd_table[DIK_F10]) {
				/* F10�ׂ�(^^; */
				return 0;
			}
			if ((LOBYTE(wParam) == VK_MENU) && !(lParam & 0x01000000) &&
				 kbd_table[DIK_LMENU]) {
				/* ��Alt�ׂ�(^^; */
				return 0;
			}
			if ((LOBYTE(wParam) == VK_MENU) && (lParam & 0x01000000) &&
				 kbd_table[DIK_RMENU]) {
				/* �EAlt�ׂ�(^^; */
				return 0;
			}
#ifndef DINPUT8
			if ((LOBYTE(HIWORD(lParam)) == 0x36) && bVistaflag) {
				/* Vista�ȍ~�̉E�V�t�g�΍� */
				bNTkeyPushFlag[KNT_RSHIFT] = FALSE;
				return 0;
			}
#endif
			if (((LOBYTE(wParam) == VK_MENU) || (LOBYTE(wParam) == VK_F10)) && 
				 bWin8flag) {
				/* Windows 8 �΍􏈗� */
				if (bFullScreen) {
					if (!bMenuLoop) {
						EnterMenu(hWnd);
						bMenuLoop = TRUE;
					}
					else {
						ExitMenu();
						OnExitMenuLoop();
						bMenuLoop = FALSE;
					}
				}
			}
			break;

		/* ���j���[���[�v�J�n */
		case WM_ENTERMENULOOP:
			if (!bMenuLoop && !bWin8flag) {
				EnterMenu(hWnd);
			}
			bMenuLoop = TRUE;
			break;

		/* ���j���[���[�v�I�� */
		case WM_EXITMENULOOP:
			if (bMenuLoop && !bWin8flag) {
				ExitMenu();
				OnExitMenuLoop();
			}
			SetMenuExitTimer();
			bMenuLoop = FALSE;
			break;

		/* ���j���[�I�� */
		case WM_MENUSELECT:
			OnMenuSelect(wParam);
			break;

		/* ���j���[�`�F�b�N */
		case WM_NCMOUSEMOVE:
		case WM_MOUSEMOVE:
			if (bFullScreen) {
				if (HIWORD(lParam) < GetSystemMetrics(SM_CYMENUSIZE)) {
					if (!bMenuLoop) {
						EnterMenu(hMainWnd);
						bMenuLoop = TRUE;
					}
				}
				else {
					if (bMenuLoop) {
						ExitMenu();
						OnExitMenuLoop();
						bMenuLoop = FALSE;
					}
				}
			}
			break;

		/* ���j���[�|�b�v�A�b�v */
		case WM_INITMENUPOPUP:
			if (!HIWORD(lParam)) {
				OnMenuPopup(hWnd, (HMENU)wParam, (UINT)LOWORD(lParam));
				return 0;
			}
			break;

		/* ���j���[�R�}���h */
		case WM_COMMAND:
			EnterMenu(hWnd);
			OnCommand(hWnd, LOWORD(wParam));
			ExitMenu();
			return 0;

		/* �t�@�C���h���b�v */
		case WM_DROPFILES:
			OnDropFiles((HANDLE)wParam);
			return 0;

		/* �A�N�e�B�x�[�g */
		case WM_ACTIVATE:
			if (LOWORD(wParam) == WA_INACTIVE) {
				bActivate = FALSE;
			}
			else {
				if (!bActivate) {
					InvalidateRect(hDrawWnd, NULL, FALSE);
				}
				bActivate = TRUE;
			}

#if defined(MOUSE)
			SetMouseCapture(bActivate);
#endif
			break;

		/* �I�[�i�[�h���[ */
		case WM_DRAWITEM:
			if (wParam == ID_STATUS_BAR) {
				OwnerDrawStatus((LPDRAWITEMSTRUCT)lParam);
				return TRUE;
			}
			break;

		/* ��ʃf�U�C���ύX */
		case WM_SYSCOLORCHANGE:
			ResizeStatus(hWnd, hStatusBar);
			break;

		/* �e�[�}�ύX */
		case WM_THEMECHANGED:
			if (bXPflag) {
				ChangeStatusBorder(hStatusBar);
			}
			break;

		/* �^�C�}�[ */
		case WM_TIMER:
			if ((wParam >= 0x80) && (wParam < 0x100)) {
				/* �L�[�J�� */
				key_code = (BYTE)(wParam & 0x7f);
				if (bNTkeyMakeFlag[key_code]) {
					keyboard_break(key_code);
					bNTkeyMakeFlag[key_code] = FALSE;
				}
				KillTimer(hMainWnd, wParam);
			}
			else switch (wParam) {
				case ID_STATUS_BAR :
					/* �X�e�[�^�X���b�Z�[�W���� */
					if (bStatusMessage) {
						EndStatusMessage();
					}
					KillTimer(hMainWnd, ID_STATUS_BAR);
					break;
			}
			break;

		/* �d���Ǘ� */
		case WM_POWERBROADCAST:
			timeEndPeriod(uTimerResolution);
			timeBeginPeriod(uTimerResolution);
			break;

		/* �V�X�e���R�}���h */
		case WM_SYSCOMMAND:
			/* �X�N���[���Z�[�o�E���j�^�ߓd���[�h��}�� */
			if ((wParam == SC_SCREENSAVE) || (wParam == SC_MONITORPOWER)) {
				return 1;
			}
			break;
	}

	/* �f�t�H���g �E�C���h�E�v���V�[�W�� */
	return DefWindowProc(hWnd, message, wParam, lParam);
}

/*-[ �A�v���P�[�V���� ]-----------------------------------------------------*/

/*
 *	XM7�E�C���h�E�������A�R�}���h���C����n��
 */
static BOOL FASTCALL FindXM7Wnd(void)
{
	COPYDATASTRUCT	cds;
	HWND hWnd;
	char string[128];

	/* �E�C���h�E�N���X�Ō��� */
	hWnd = FindWindow("XM7", NULL);
	if (!hWnd) {
		return FALSE;
	}

	/* �e�L�X�g������擾 */
	GetWindowText(hWnd, string, sizeof(string));
	string[4] = '\0';
	if (strcmp("XM7 ", string) != 0) {
		return FALSE;
	}

	/* ���b�Z�[�W�𑗐M */
	if (bNTflag) {
		/* NT�ł�WM_COPYDATA���g�� */
		cds.dwData = 0x01374d58;
		cds.lpData = (void *)GetCommandLine();
		cds.cbData = lstrlen(GetCommandLine()) + 1;
		SendMessage(hWnd, WM_COPYDATA, (WPARAM)NULL, (LPARAM)&cds);
	}
	else {
		/* Win9x/Me�ł͏]���Ɠ������@�Ń��b�Z�[�W���M */
		SendMessage(hWnd, WM_USER + 1, (WPARAM)GetCommandLine(), (LPARAM)NULL);
	}

	return TRUE;
}

/*
 *	�C���X�^���X������
 */
static HWND FASTCALL InitInstance(HINSTANCE hInst, int nCmdShow)
{
	HWND hWnd;
	HMENU hMenu;
	WNDCLASSEX wcex;
	char szAppName[] = "XM7";
	char szWindowName[128];
	int xx, yy;
	RECT rect;

	ASSERT(hInst);

	/* �E�C���h�E�n���h���N���A */
	hMainWnd = NULL;
	hDrawWnd = NULL;
	hStatusBar = NULL;

	/* �����E�B���h�E�T�C�Y���v�Z */
	rect.left = 0;
	rect.top = 0;
	if (LoadCfg_DoubleSize()) {
		rect.right = 1280;
		rect.bottom = 800;
	}
	else {
		rect.right = 640;
		rect.bottom = 400;
	}
	AdjustWindowRect(&rect, WS_OVERLAPPED | WS_SYSMENU | WS_CAPTION |
							WS_BORDER | WS_CLIPCHILDREN | WS_MINIMIZEBOX,
							TRUE);
	xx = rect.right - rect.left;
	yy = rect.bottom - rect.top;

	/* �����A�C�R�������[�h */
#if XM7_VER >= 3
	hAppIcon = LoadIcon(hAppInstance, MAKEINTRESOURCE(IDI_APPICON));
#elif XM7_VER >= 2
	hAppIcon = LoadIcon(hAppInstance, MAKEINTRESOURCE(IDI_AVICON));
#else
	hAppIcon = LoadIcon(hAppInstance, MAKEINTRESOURCE(IDI_APPICON));
#endif
	nAppIcon = 255;

	/* �E�C���h�E�N���X�̓o�^ */
	memset(&wcex, 0, sizeof(wcex));
	wcex.cbSize = sizeof(wcex);
	wcex.style = CS_VREDRAW | CS_HREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInst;
	wcex.hIcon = hAppIcon;
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	wcex.lpszMenuName = MAKEINTRESOURCE(IDR_MAINMENU);
	wcex.lpszClassName = szAppName;
	wcex.hIconSm = hAppIcon;
	RegisterClassEx(&wcex);

	/* �E�B���h�E�^�C�g�����쐬 */
	strncpy(szWindowName, szAppName, sizeof(szWindowName));
	strncat(szWindowName, " ", sizeof(szWindowName) - strlen(szWindowName) - 1);

	/* ���j���[���쐬 */
	hMenu = LoadMenu(hAppInstance, MAKEINTRESOURCE(IDR_MAINMENU));

	/* �E�C���h�E�쐬 */
	hWnd = CreateWindow(szAppName,
						szWindowName,
						WS_OVERLAPPED | WS_SYSMENU | WS_CAPTION | WS_BORDER |
						WS_CLIPCHILDREN | WS_MINIMIZEBOX,
						CW_USEDEFAULT,
						CW_USEDEFAULT,
						xx,
						yy,
						NULL,
						hMenu,
						hInst,
						NULL);
	if (!hWnd) {
		return NULL;
	}
	
	/* �E�B���h�E�ʒu�ݒ� */
	if ((WinPos.x == -99999) && (WinPos.y == -99999)) {
		GetWindowRect(hWnd, &rect);
		WinPos.x = (GetSystemMetrics(SM_CXSCREEN) - rect.right) / 2;
		WinPos.y = (GetSystemMetrics(SM_CYSCREEN) - rect.bottom) / 2;
	}
	SetWindowPos(hWnd, HWND_NOTOPMOST, WinPos.x, WinPos.y, 0, 0, SWP_NOSIZE);
	OnSize(hWnd, 640, 400);

	/* �E�C���h�E�\�� */
	if (nErrorCode == 0) {
		ShowWindow(hWnd, nCmdShow);
		UpdateWindow(hWnd);
	}

	return hWnd;
}

/*
 *	WinMain
 */
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
						LPSTR lpszCmdParam, int nCmdShow)
{
	MSG msg;
	HACCEL hAccel;
	OSVERSIONINFO osvi;
	STICKYKEYS sk;
	STICKYKEYS skb;
	char buffer[128];
	int osver;

	UNUSED(hPrevInstance);
	UNUSED(lpszCmdParam);

	/* COM�������� */
	if (FAILED(CoInitialize(NULL))) {
		LoadString(hInstance, IDS_COINITERROR, buffer, sizeof(buffer));
		MessageBox(NULL, buffer, "XM7", MB_ICONSTOP | MB_OK);

		return -1;
	}

	/* �R�����R���g���[�������� */
	InitCommonControls();

	/* ���삵�Ă���OS��WinNT�n������ */
	osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	GetVersionEx(&osvi);
	osver = osvi.dwMajorVersion * 100 + osvi.dwMinorVersion;

	/* NT�`�F�b�N */
	if (osvi.dwPlatformId == VER_PLATFORM_WIN32_NT) {
		bNTflag = TRUE;
	}
	else {
		bNTflag = FALSE;
	}

	/* XP�`�F�b�N */
	if (osver >= 501) {
		bXPflag = TRUE;
	}
	else {
		bXPflag = FALSE;
	}

	/* Windows Vista�`�F�b�N */
	if (osver >= 600) {
		bVistaflag = TRUE;
	}
	else {
		bVistaflag = FALSE;
	}

	/* Windows 7�`�F�b�N */
	if (osver >= 601) {
		bWin7flag = TRUE;
	}
	else {
		bWin7flag = FALSE;
	}

	/* Windows 8�`�F�b�N */
	if (osver >= 602) {
		bWin8flag = TRUE;
	}
	else {
		bWin8flag = FALSE;
	}

	/* Windows 10�`�F�b�N */
	if (osver >= 604) {
		bWin10flag = TRUE;
	}
	else {
		bWin10flag = FALSE;
	}

	/* XM7�`�F�b�N�A�R�}���h���C���n�� */
	if (FindXM7Wnd()) {
		/* COM���I�� */
		CoUninitialize();

		return 0;
	}

	/* �e��t���O���擾�E�ݒ� */
	bMMXflag = CheckMMX();
	bCMOVflag = CheckCMOV();
#if defined(ROMEO)
	bRomeo = FALSE;
#endif
#if XM7_VER <= 2 && defined(FMTV151)
	bFMTV151 = TRUE;
#endif

	/* DLL������ */
	InitThemeDLL();
	InitIMEDLL();

	/* �t�H���g�n���h���擾 */
	hFont = CreateTextFont();

	/* �C���X�^���X��ۑ��A������ */
	hAppInstance = hInstance;
	hMainWnd = InitInstance(hInstance, nCmdShow);
	if (!hMainWnd) {
		CleanIMEDLL();
		CleanThemeDLL();
		DeleteObject(hFont);

		/* COM���I�� */
		CoUninitialize();

		return -1;
	}

	/* �Œ�L�[�@�\�̖����� */
	sk.cbSize = sizeof(STICKYKEYS);
	SystemParametersInfo(SPI_GETSTICKYKEYS, sizeof(STICKYKEYS), &sk, 0);
	skb = sk;
	sk.dwFlags &= ~(SKF_HOTKEYACTIVE | SKF_AVAILABLE | SKF_STICKYKEYSON);
	SystemParametersInfo(SPI_SETSTICKYKEYS, sizeof(STICKYKEYS), &sk, 0);

	/* �A�N�Z�����[�^�����[�h */
	hAccel = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDR_ACCELERATOR));
	ASSERT(hAccel);

	/* ���b�Z�[�W ���[�v */
	while (hMainWnd) {
		/* ���b�Z�[�W���`�F�b�N���A���� */
		if (PeekMessage(&msg, 0, 0, 0, PM_NOREMOVE)) {
			if (!GetMessage(&msg, NULL, 0, 0)) {
				OnQuit();
				break;
			}

#if defined(MOUSE)
			/* �����{�^���ɂ��}�E�X���[�h�؂�ւ� */
			if ((msg.message == WM_MBUTTONDOWN) &&
				(msg.wParam == MK_MBUTTON) &&
				(uMidBtnMode == MOSCAP_WMESSAGE)) {
				/* �����{�^���������ꂽ��}�E�X���[�h��؂芷���� */
				EnterMenu(hMainWnd);
				OnCommand(hMainWnd, IDM_MOUSEMODE);
				ExitMenu();
			}

			/* �z�C�[���ɂ��}�E�X���[�h�؂�ւ�(Unz����) */
#if defined(WM_MOUSEWHEEL)
			if ((msg.message == WM_MOUSEWHEEL) &&
				(uMidBtnMode == MOSCAP_WHEELWM)) {
				if ((short)HIWORD(msg.wParam) > 0) {
					EnterMenu(hMainWnd);
					OnCommand(hMainWnd, IDM_MOUSEON);
					ExitMenu();
				}
				if ((short)HIWORD(msg.wParam) < 0) {
					EnterMenu(hMainWnd);
					OnCommand(hMainWnd, IDM_MOUSEOFF);
					ExitMenu();
				}
			}
#endif
#endif

			/* NT�L�[�΍􏈗� */
			if (msg.message == WM_KEYDOWN) {
				switch (LOBYTE(msg.wParam)) {
					case VKNT_CAPITAL	:	/* CapsLock */
					case VK_CAPITAL		:	bNTkeyPushFlag[KNT_CAPS] = TRUE;
											break;
					case VKNT_KANA		:
					case VKNT_KATAKANA	:	/* �J�^�J�i/�Ђ炪��,�J�i(PC-98) */
					case VK_KANA		:	bNTkeyPushFlag[KNT_KANA] = TRUE;
											break;
					case VKNT_KANJI1	:	/* ���p/�S�p */
					case VKNT_KANJI2	:	bNTkeyPushFlag[KNT_KANJI] = TRUE;
											break;
				}
			}

#if defined(KBDPASTE)
			if (hKeyStrokeDialog) {
				if (IsDialogMessage(hKeyStrokeDialog, &msg)) {
					continue;
				}
			}
#endif

			if (!TranslateAccelerator(hMainWnd, hAccel, &msg)) {
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}

			continue;
		}

		/* �t���X�N���[���v�� */
		if (bFullRequested) {
			PostMessage(hMainWnd, WM_COMMAND, IDM_FULLSCREEN, 0);
			bFullRequested = FALSE;
		}

		/* �X�e�[�^�X�`�恕�X���[�v */
		if (nErrorCode == 0) {
			/* Windows Vista/7�̏ꍇ�A�����ŃT�u�E�B���h�E���X�V */
			if (bVistaflag) {
				DrawSubWindow();
			}

			DrawStatus();
			if (bSync) {
				ReSizeOPNReg();
				ReSizeOPNDisp();
			}

			/* ������ƈ�x�� */
			Sleep(16);
		}
	}

	/* �t�H���g�n���h����� */
	DeleteObject(hFont);

	/* DLL�J�� */
	CleanIMEDLL();
	CleanThemeDLL();

	/* �����I�Ƀ}�E�X�N���b�v������ */
	ClipCursor(0);

	/* �Œ�L�[�@�\�̕��� */
	SystemParametersInfo(SPI_SETSTICKYKEYS, sizeof(STICKYKEYS), &skb,  0);

	/* COM���I�� */
	CoUninitialize();

	/* �I�� */
	return msg.wParam;
}

#endif	/* _WIN32 */
