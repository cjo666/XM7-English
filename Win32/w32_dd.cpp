/*
 *	FM-7 EMULATOR "XM7"
 *
 *	Copyright (C) 1999-2017 �o�h�D(Twitter:@xm6_original)
 *	Copyright (C) 2001-2017 Ryu Takegami (Twitter:@RyuTakegami)
 *
 *	[ Win32API DirectDraw ]
 *
 *	RHG����
 *	  2012.07.01		�O���[�����j�^���[�h�ɑΉ�
 *	  2012.10.10		Windows 8�ł�DrawMenuBar API�̋����ύX�ɑ΍�
 *						�L���v�V�����̃o�b�t�@�T�C�Y��256�o�C�g�ɑ���
 *	  2013.02.12		���X�^�����_�����O�ɑΉ�
 *	  2014.03.16		V1/V2�ł�32�r�b�g�J���[�����_���ɑΉ�
 */

#ifdef _WIN32

#define STRICT
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#ifdef DINPUT8
#define DIRECTDRAW_VERSION		0x700	/* DirectX7���w�� */
#else
#define DIRECTDRAW_VERSION		0x300	/* DirectX3���w�� */
#endif
#include <ddraw.h>
#include <objbase.h>
#include <assert.h>
#include "xm7.h"
#include "subctrl.h"
#include "display.h"
#include "multipag.h"
#include "ttlpalet.h"
#include "apalet.h"
#include "fdc.h"
#include "tapelp.h"
#include "bubble.h"
#include "keyboard.h"
#include "opn.h"
#include "w32.h"
#include "w32_bar.h"
#include "w32_sch.h"
#include "w32_res.h"
#include "w32_draw.h"
#include "w32_dd.h"
#include "w32_kbd.h"
#include "w32_sub.h"

/*
 *	�O���[�o�� ���[�N
 */
#ifdef __cplusplus
extern "C" {
#endif
#if XM7_VER == 1
DWORD rgbTTLDD[16];						/* 640x200 �p���b�g */
#else
DWORD rgbTTLDD[8];						/* 640x200 �p���b�g */
DWORD rgbAnalogDD[4096];				/* 320x200 �p���b�g */
#endif
BYTE nDDResolutionMode;					/* �t���X�N���[�����𑜓x */
BOOL bDD480Status;						/* 640x480�X�e�[�^�X�t���O */
BOOL bDDtruecolor;						/* TrueColor�D��t���O */
BYTE DDDrawFlag[4000];					/* 8x8 �ĕ`��̈�t���O */
#if XM7_VER == 1 && defined(L4CARD)
DWORD rgbTTLL4DD[32];					/* 640x400 �P�F�p���b�g */
#endif
#ifdef __cplusplus
}
#endif

/*
 *	�X�^�e�B�b�N ���[�N
 */
#if XM7_VER >= 3
static BYTE nMode;						/* ��ʃ��[�h */
#elif XM7_VER >= 2
static BOOL bAnalog;					/* �A�i���O���[�h�t���O */
#elif XM7_VER == 1 && defined(L4CARD)
static BOOL b400Line;					/* 400���C�����[�h�t���O */
#endif
static RECT BkRect;						/* �E�C���h�E���̋�` */
static BOOL bMouseCursor;				/* �}�E�X�J�[�\���t���O */
static HMENU hMainMenu;					/* ���C�����j���[�n���h�� */
static HBRUSH hDrawBrush;				/* �h���[�E�B���h�E�̃u���V */
static HBRUSH hMainBrush;				/* ���C���E�B���h�E�̃u���V */
static LPDIRECTDRAW2 lpdd2;				/* DirectDraw2 */
static LPDIRECTDRAWSURFACE lpdds[2];	/* DirectDrawSurface3 */
static LPDIRECTDRAWCLIPPER lpddc;		/* DirectDrawClipper */
static UINT nPixelFormat;				/* 320x200 �s�N�Z���t�H�[�}�b�g */
static WORD nDrawTop;					/* �`��͈͏� */
static WORD nDrawBottom;				/* �`��͈͉� */
static WORD nDrawLeft;					/* �`��͈͍� */
static WORD nDrawRight;					/* �`��͈͉E */
static BOOL bPaletFlag;					/* �p���b�g�ύX�t���O */
static BYTE nDDResolution;				/* �t���X�N���[���� ���ۂ̉𑜓x */
static BOOL bClearFlag;					/* �㉺�N���A�t���O */
#if XM7_VER >= 3
static BOOL bWindowOpen;				/* �n�[�h�E�F�A�E�B���h�E��� */
static WORD nWindowDx1;					/* �E�B���h�E����X���W */
static WORD nWindowDy1;					/* �E�B���h�E����Y���W */
static WORD nWindowDx2;					/* �E�B���h�E�E��X���W */
static WORD nWindowDy2;					/* �E�B���h�E�E��Y���W */
#endif

static char szRunMessage[128];			/* RUN���b�Z�[�W */
static char szStopMessage[128];			/* STOP���b�Z�[�W */
static char szCaption[256];				/* �L���v�V���� */
static char szCAPMessage[16];			/* CAP�L�[���b�Z�[�W */
static char szKANAMessage[16];			/* ���ȃL�[���b�Z�[�W */
static char szINSMessage[16];			/* INS�L�[���b�Z�[�W */
static int nCAP;						/* CAP�L�[ */
static int nKANA;						/* ���ȃL�[ */
static int nINS;						/* INS�L�[ */
static int nDrive[2];					/* �t���b�s�[�h���C�u */
static char szDrive[2][16 + 1];			/* �t���b�s�[�h���C�u */
static int nTape;						/* �e�[�v */
static BOOL bCaption;					/* �L���v�V�����\�� */
static int nScrX;						/* �������𑜓x */
static int nScrY;						/* �c�����𑜓x */


/*
 *	�}���`���j�^�Ή������\����
 *	Windows95�ł��R���p�C����ʂ����߈Ӑ}�I�ɖ��O��ύX���Ă���
 */
typedef struct tagMONITOR_INFO {  
    DWORD  cbSize; 
    RECT   rcMonitor; 
    RECT   rcWork; 
    DWORD  dwFlags; 
} MONITOR_INFO, *LPMONITOR_INFO; 

/*
 *	�}���`���j�^�Ή������֘A
 */
static HMODULE hModUser32;				/* user32.dll���W���[�� */
static HMONITOR (WINAPI* monitorfromwindow)(HWND, DWORD);
static BOOL (WINAPI* getmonitorinfo)(HMONITOR, LPMONITOR_INFO);
static HRESULT (WINAPI* ddenumerateex)(LPDDENUMCALLBACKEX, LPVOID, DWORD);
static HMONITOR hmonitor;				/* �T������ hmonitor */
static GUID gmonitor;					/* hmonitor �ɑΉ����� GUID */
static POINT ptMouseBak;				/* �E�B���h�E���A���̃}�E�X�ʒu */
static POINT ptFullScr;					/* �t���X�N���[���������}�E�X�ʒu */

/*
 *	�A�Z���u���֐��̂��߂̃v���g�^�C�v�錾
 */
#ifdef __cplusplus
extern "C" {
#endif
#if XM7_VER == 1
/* 15/16�r�b�g�J���[ */
void Render640DD(LPVOID lpSurface, LONG lPitch, int first, int last);
void RenderL4DD(LPVOID lpSurface, LONG lPitch, int first, int last);
void Render640mDD(LPVOID lpSurface, LONG lPitch, int first, int last);

/* 32�r�b�g�J���[ */
void Render640Tc32DD(LPVOID lpSurface, LONG lPitch, int first, int last);
void RenderL4Tc32DD(LPVOID lpSurface, LONG lPitch, int first, int last);
void Render640mTc32DD(LPVOID lpSurface, LONG lPitch, int first, int last);

/* �Ăяo���p */
static void (*Render640)(LPVOID lpSurface, LONG lPitch, int first, int last);
static void (*Render640m)(LPVOID lpSurface, LONG lPitch, int first, int last);
#if defined(L4CARD)
static void (*RenderL4)(LPVOID lpSurface, LONG lPitch, int first, int last);
#endif
#elif XM7_VER == 2
/* 15/16�r�b�g�J���[ */
void Render640DD(LPVOID lpSurface, LONG lPitch, int first, int last);
void Render320DD(LPVOID lpSurface, LONG lPitch, int first, int last);
void Render640cDD(LPVOID lpSurface, LONG lPitch, int first, int last);

/* 32�r�b�g�J���[ */
void Render640Tc32DD(LPVOID lpSurface, LONG lPitch, int first, int last);
void Render320Tc32DD(LPVOID lpSurface, LONG lPitch, int first, int last);
void Render640cTc32DD(LPVOID lpSurface, LONG lPitch, int first, int last);

/* �Ăяo���p */
static void (*Render640)(LPVOID lpSurface, LONG lPitch, int first, int last);
static void (*Render320)(LPVOID lpSurface, LONG lPitch, int first, int last);
static void (*Render640c)(LPVOID lpSurface, LONG lPitch, int first, int last);
#elif XM7_VER >= 3
/* 15/16�r�b�g�J���[ */
void Render640DD2(LPVOID lpSurface, LONG lPitch,
					int first, int last, int scale);
void Render640wDD2(LPVOID lpSurface, LONG lPitch,
					int first, int last, int firstx, int lastx, int scale);
void Render320DD(LPVOID lpSurface, LONG lPitch, int first, int last);
void Render320wDD(LPVOID lpSurface, LONG lPitch,
					int first, int last, int firstx, int lastx);
void Render256k555DD(LPVOID lpSurface, LONG lPitch,
					int first, int last, int multipage);
void Render256k565DD(LPVOID lpSurface, LONG lPitch,
					int first, int last, int multipage);
void Render640cDD(LPVOID lpSurface, LONG lPitch,
					int first, int last);
void Render640cwDD(LPVOID lpSurface, LONG lPitch,
					int first, int last, int firstx, int lastx);

/* 24�r�b�g�J���[ */
void Render640Tc24DD2(LPVOID lpSurface, LONG lPitch,
					int first, int last, int scale);
void Render640wTc24DD2(LPVOID lpSurface, LONG lPitch,
					int first, int last, int firstx, int lastx, int scale);
void Render320Tc24DD(LPVOID lpSurface, LONG lPitch, int first, int last);
void Render320wTc24DD(LPVOID lpSurface, LONG lPitch,
					int first, int last, int firstx, int lastx);
void Render256kTc24DD(LPVOID lpSurface, LONG lPitch,
					int first, int last, int multipage);
void Render640cTc24DD(LPVOID lpSurface, LONG lPitch,
					int first, int last);
void Render640cwTc24DD(LPVOID lpSurface, LONG lPitch,
					int first, int last, int firstx, int lastx);

/* 32�r�b�g�J���[ */
void Render640Tc32DD2(LPVOID lpSurface, LONG lPitch,
					int first, int last, int scale);
void Render640wTc32DD2(LPVOID lpSurface, LONG lPitch,
					int first, int last, int firstx, int lastx, int scale);
void Render320Tc32DD(LPVOID lpSurface, LONG lPitch, int first, int last);
void Render320wTc32DD(LPVOID lpSurface, LONG lPitch,
					int first, int last, int firstx, int lastx);
void Render256kTc32DD(LPVOID lpSurface, LONG lPitch,
					int first, int last, int multipage);
void Render640cTc32DD(LPVOID lpSurface, LONG lPitch,
					int first, int last);
void Render640cwTc32DD(LPVOID lpSurface, LONG lPitch,
					int first, int last, int firstx, int lastx);

/* �Ăяo���p */
static void (*Render640)(LPVOID lpSurface, LONG lPitch,
				int first, int last, int scale);
static void (*Render640w)(LPVOID lpSurface, LONG lPitch,
				int first, int last, int firstx, int lastx, int scale);
static void (*Render320)(LPVOID lpSurface, LONG lPitch, int first, int last);
static void (*Render320w)(LPVOID lpSurface, LONG lPitch,
				int first, int last, int firstx, int lastx);
static void (*Render256k)(LPVOID lpSurface, LONG lPitch,
				int first, int last, int multipage);
static void (*Render640c)(LPVOID lpSurface, LONG lPitch,
				int first, int last);
static void (*Render640cw)(LPVOID lpSurface, LONG lPitch,
				int first, int last, int firstx, int lastx);
#endif
#ifdef __cplusplus
}
#endif

#if XM7_VER == 1 && defined(L4CARD)
/*
 *	16�F���[�h�p�p���b�g�R�[�h
 */
static const WORD Palet16_L4[] = {
	0x0000,	0x0017,	0xb800,	0xb817,	0x05e0,	0x05f7,	0xbde0,	0xbdf7,
	0x4208,	0x421f,	0xfa08,	0xfa1f,	0x47e8,	0x47ff,	0xffe8,	0xffff,
	0x0000,	0x0017,	0x5c00,	0x5c17,	0x02e0,	0x02f7,	0x5ee0,	0x5ef7,
	0x2108,	0x211f,	0x7d08,	0x7d1f,	0x23e8,	0x23ff,	0x7fe8,	0x7fff,
	0x0000,	0x0180,	0x0320,	0x0380,	0x0540,	0x0560,	0x05e0,	0x0600,
	0x0240,	0x0320,	0x04a0,	0x0500,	0x0700,	0x0740,	0x07c0,	0x07e0,
	0x0000,	0x00c0,	0x0180,	0x01c0,	0x02a0,	0x02c0,	0x02e0,	0x0300,
	0x0120,	0x0180,	0x0240,	0x0280,	0x0380,	0x03a0,	0x03c0,	0x03e0,
};
static const DWORD Palet16Tc_L4[] = {
	0x00000000,	0x000000bb,	0x00bb0000,	0x00bb00bb,
	0x0000bb00,	0x0000bbbb,	0x00bbbb00,	0x00bbbbbb,
	0x00444444,	0x004444ff,	0x00ff4444,	0x00ff44ff,
	0x0044ff44,	0x0044ffff,	0x00ffff44,	0x00ffffff,
	0x00000000,	0x00003400,	0x00005E00,	0x00006900,
	0x0000a000,	0x0000a600,	0x0000b500,	0x0000bb00,
	0x00004500,	0x00006000,	0x00008c00,	0x00009900,
	0x0000de00,	0x0000e600,	0x0000f800,	0x0000ff00
};
#endif

/*
 *	�v���g�^�C�v�錾
 */
static void FASTCALL AllClear(BOOL clear_flag);


/*
 *	������
 */
void FASTCALL InitDD(void)
{
	/* ���[�N�G���A������(�ݒ胏�[�N�͕ύX���Ȃ�) */
#if XM7_VER >= 3
	nMode = SCR_200LINE;
#elif XM7_VER >= 2
	bAnalog = FALSE;
#elif XM7_VER == 1 && defined(L4CARD)
	b400Line = FALSE;
#endif
	bMouseCursor = TRUE;
	lpdd2 = NULL;
	memset(lpdds, 0, sizeof(lpdds));
	lpddc = NULL;
	bClearFlag = TRUE;
	ptFullScr.x = 0;
	ptFullScr.y = 0;

	/* �X�e�[�^�X���C�� */
	szCaption[0] = '\0';
	nCAP = -1;
	nKANA = -1;
	nINS = -1;
	nDrive[0] = -1;
	nDrive[1] = -1;
	szDrive[0][0] = '\0';
	szDrive[1][0] = '\0';
	nTape = -1;
	bCaption = TRUE;

	/* �}���`���j�^�Ή��p��MonitorFromWindow�����[�h */
	hModUser32 = LoadLibrary("user32.dll");
	if (hModUser32) {
		monitorfromwindow = (HMONITOR (WINAPI*)(HWND, DWORD))GetProcAddress(hModUser32, "MonitorFromWindow");
		getmonitorinfo = (BOOL (WINAPI*)(HMONITOR, LPMONITOR_INFO))GetProcAddress(hModUser32, "GetMonitorInfoA");
	}
	else {
		monitorfromwindow = NULL;
		getmonitorinfo = NULL;
	}

	/* ���b�Z�[�W�����[�h */
	if (LoadString(hAppInstance, IDS_RUNCAPTION,
					szRunMessage, sizeof(szRunMessage)) == 0) {
		szRunMessage[0] = '\0';
	}
	if (LoadString(hAppInstance, IDS_STOPCAPTION,
					szStopMessage, sizeof(szStopMessage)) == 0) {
		szStopMessage[0] = '\0';
	}
	if (LoadString(hAppInstance, IDS_BAR_CAP,
					szCAPMessage, sizeof(szCAPMessage)) == 0) {
		strncpy(szCAPMessage, "CAP", sizeof(szCAPMessage));
	}
	if (LoadString(hAppInstance, IDS_BAR_KANA,
					szKANAMessage, sizeof(szKANAMessage)) == 0) {
#if XM7_VER == 1
		strncpy(szKANAMessage, "�J�i", sizeof(szKANAMessage));
#else
		strncpy(szKANAMessage, "����", sizeof(szKANAMessage));
#endif
	}
	if (LoadString(hAppInstance, IDS_BAR_INS,
					szINSMessage, sizeof(szINSMessage)) == 0) {
		strncpy(szINSMessage, "INS", sizeof(szINSMessage));
	}
}

/*
 *	�N���[���A�b�v
 */
void FASTCALL CleanDD(void)
{
	DWORD dwStyle;
	RECT brect;
	int i;
	int width;
	int height;

	/* DirectDrawClipper */
	if (lpddc) {
		lpddc->Release();
		lpddc = NULL;
	}

	/* DirectDrawSurface3 */
	for (i=0; i<2; i++) {
		if (lpdds[i]) {
			lpdds[i]->Release();
			lpdds[i] = NULL;
		}
	}

	/* DirectDraw2 */
	if (lpdd2) {
		lpdd2->Release();
		lpdd2 = NULL;
	}

	/* �E�C���h�E�X�^�C����߂� */
	dwStyle = GetWindowLong(hMainWnd, GWL_STYLE);
	dwStyle &= ~WS_POPUP;
	dwStyle |= (WS_CAPTION | WS_BORDER | WS_SYSMENU);
	SetWindowLong(hMainWnd, GWL_STYLE, dwStyle);
	dwStyle = GetWindowLong(hMainWnd, GWL_EXSTYLE);
	dwStyle |= WS_EX_WINDOWEDGE;
	SetWindowLong(hMainWnd, GWL_EXSTYLE, dwStyle);

	/* Windows Vista/Windows 7�ł̃A�C�R���\�����A */
	SetClassLong(hMainWnd, GCL_HICONSM,
		(LONG)LoadIcon(hAppInstance, IDI_APPLICATION));

	/* Windows 8�ł̃��j���[�E�u���V���� */
	if (bWin8flag) {
		SetMenu(hMainWnd, hMainMenu);
		SetClassLong(hMainWnd, GCL_HBRBACKGROUND, (LONG)hMainBrush);
		SetClassLong(hDrawWnd, GCL_HBRBACKGROUND, (LONG)hDrawBrush);
	}

	/* �E�C���h�E�ʒu��߂� */
	if (bDoubleSize) {
		width = 1280;
		height = 800;
	}
	else {
		width = 640;
		height = 400;
	}
	SetWindowPos(hMainWnd, HWND_NOTOPMOST, BkRect.left, BkRect.top,
		(BkRect.right - BkRect.left), (BkRect.bottom - BkRect.top),
		SWP_DRAWFRAME);

	MoveWindow(hDrawWnd, 0, 0, width, height, TRUE);
	if (hStatusBar) {
		GetWindowRect(hStatusBar, &brect);
		MoveWindow(hStatusBar, 0, height,
					(brect.right - brect.left),
					(brect.bottom - brect.top),
					TRUE);
	}

	/* user32.dll����� */
	if (hModUser32) {
		FreeLibrary(hModUser32);
		hModUser32 = NULL;
	}

	/* �}�E�X�|�C���^�ʒu�𕜋A */
	SetCursorPos(ptMouseBak.x, ptMouseBak.y);
}


/*
 *	�}���`���j�^�pDirectDraw�񋓃R�[���o�b�N�֐�
 */
static BOOL WINAPI DDEnumCallback(GUID FAR* guid, LPSTR desc, LPSTR name, LPVOID context, HMONITOR hm)
{
	UNUSED(desc);
	UNUSED(name);
	UNUSED(context);

	if (hm == hmonitor) {
		gmonitor = *guid;
		return 0;
	}
	return 1;
}

/*
 *	��ʐݒ�T�u
 */
static BOOL FASTCALL SetDisplayMode(int x, int y)
{
#if XM7_VER >= 3
	if (bDDtruecolor) {
		nPixelFormat = 3;
		if (FAILED(lpdd2->SetDisplayMode(x, y, 32, 0, 0))) {
			nPixelFormat = 4;
			if (FAILED(lpdd2->SetDisplayMode(x, y, 24, 0, 0))) {
				nPixelFormat = 0;
				if (FAILED(lpdd2->SetDisplayMode(x, y, 16, 0, 0))) {
					/* ���s */
					return FALSE;
				}
			}
		}
	}
	else {
		if (bWin8flag) {
			nPixelFormat = 3;
			if (FAILED(lpdd2->SetDisplayMode(x, y, 32, 0, 0))) {
				nPixelFormat = 0;
				if (FAILED(lpdd2->SetDisplayMode(x, y, 16, 0, 0))) {
					/* ���s */
					return FALSE;
				}
			}
		}
		else {
			nPixelFormat = 0;
			if (FAILED(lpdd2->SetDisplayMode(x, y, 16, 0, 0))) {
				nPixelFormat = 3;
				if (FAILED(lpdd2->SetDisplayMode(x, y, 32, 0, 0))) {
					/* ���s */
					return FALSE;
				}
			}
		}
	}
#else
	if (bWin8flag || bDDtruecolor) {
		nPixelFormat = 3;
		if (FAILED(lpdd2->SetDisplayMode(x, y, 32, 0, 0))) {
			nPixelFormat = 0;
			if (FAILED(lpdd2->SetDisplayMode(x, y, 16, 0, 0))) {
				/* ���s */
				return FALSE;
			}
		}
	}
	else {
		nPixelFormat = 0;
		if (FAILED(lpdd2->SetDisplayMode(x, y, 16, 0, 0))) {
			nPixelFormat = 3;
			if (FAILED(lpdd2->SetDisplayMode(x, y, 32, 0, 0))) {
				/* ���s */
				return FALSE;
			}
		}
	}
#endif

	/* ���� */
	nScrX = x;
	nScrY = y;
	return TRUE;
}

/*
 *	�Z���N�g
 */
BOOL FASTCALL SelectDD(void)
{
	DWORD dwStyle;
	LPDIRECTDRAW lpdd;
	DDSURFACEDESC ddsd;
	DDPIXELFORMAT ddpf;
	RECT brect;
	LPDIRECTDRAW lpdd1;
	HMODULE hModDDraw;
	RECT srect;
	MONITOR_INFO minfo;

	/* assert */
	ASSERT(hMainWnd);

	/* �E�C���h�E��`�E�}�E�X�|�C���^�ʒu���L������ */
	GetWindowRect(hMainWnd, &BkRect);
	GetCursorPos(&ptMouseBak);

	/* ���j���[�n���h�����L������ */
	if (bWin8flag) {
		hMainMenu = GetMenu(hMainWnd);
	}

	/* �E�C���h�E�X�^�C����ύX */
	dwStyle = GetWindowLong(hMainWnd, GWL_STYLE);
	dwStyle &= ~(WS_CAPTION | WS_BORDER | WS_SYSMENU);
	dwStyle |= WS_POPUP;
	SetWindowLong(hMainWnd, GWL_STYLE, dwStyle);
	dwStyle = GetWindowLong(hMainWnd, GWL_EXSTYLE);
	dwStyle &= ~WS_EX_WINDOWEDGE;
	SetWindowLong(hMainWnd, GWL_EXSTYLE, dwStyle);

	/* DirectDraw�I�u�W�F�N�g���쐬(�}���`���j�^�Ή�) */
	lpdd = NULL;
	hmonitor = NULL;

	/* �}���`���j�^�Ή��pDirectDraw���������� */
	memset(&gmonitor, 0, sizeof(gmonitor));
	if (monitorfromwindow) {
		hModDDraw = LoadLibrary("ddraw.dll");
		if (hModDDraw) {
			ddenumerateex = (HRESULT (WINAPI*)(LPDDENUMCALLBACKEX, LPVOID,
				DWORD))GetProcAddress(hModDDraw,"DirectDrawEnumerateExA");
			if (ddenumerateex) {
				hmonitor = monitorfromwindow(hMainWnd, 1);
				ddenumerateex(DDEnumCallback, NULL,
							  DDENUM_ATTACHEDSECONDARYDEVICES);
			}
			FreeLibrary(hModDDraw);
		}
	}

	if (!FAILED(CoCreateInstance(CLSID_DirectDraw, 0, CLSCTX_ALL, IID_IDirectDraw, (void**)&lpdd1))) {
		if (!FAILED(IDirectDraw_Initialize(lpdd1, &gmonitor))) {
			lpdd = lpdd1;
		}
	}

	if (!lpdd) {
		/* DirectDraw�I�u�W�F�N�g���쐬 */
		if (FAILED(DirectDrawCreate(NULL, &lpdd, NULL))) {
			return FALSE;
		}
	}

	/* �������[�h��ݒ� */
	if (FAILED(lpdd->SetCooperativeLevel(hMainWnd, DDSCL_EXCLUSIVE | DDSCL_FULLSCREEN))) {
		lpdd->Release();
		return FALSE;
	}

	/* DirectDraw2�C���^�t�F�[�X���擾 */
	if (FAILED(lpdd->QueryInterface(IID_IDirectDraw2, (LPVOID*)&lpdd2))) {
		lpdd->Release();
		return FALSE;
	}

	/* �����܂ŗ���΁ADirectDraw�͂��͂�K�v�Ȃ� */
	lpdd->Release();

	/* ��ʃ��[�h��ݒ� */
	switch (nDDResolutionMode) {
		case 0:	/* 640 x 400 */
			nDDResolution = DDRES_400LINE;
			if (!SetDisplayMode(640, 400)) {
				nDDResolution = DDRES_480LINE;
				if (!SetDisplayMode(640, 480)) {
					return FALSE;
				}
			}
			break;
		case 1:	/* 640 x 480 */
			nDDResolution = DDRES_480LINE;
			if (!SetDisplayMode(640, 480)) {
				nDDResolution = DDRES_400LINE;
				if (!SetDisplayMode(640, 400)) {
					return FALSE;
				}
			}
			break;
		case 2:	/* 1920 x 1200 */
			nDDResolution = DDRES_WUXGA;
			if (!SetDisplayMode(1920, 1200)) {
				nDDResolution = DDRES_400LINE;
				if (!SetDisplayMode(640, 400)) {
					nDDResolution = DDRES_480LINE;
					if (!SetDisplayMode(640, 480)) {
						return FALSE;
					}
				}
			}
			break;
		case 3:	/* 1280 x 1024 */
			nDDResolution = DDRES_SXGA;
			if (!SetDisplayMode(1280, 1024)) {
				nDDResolution = DDRES_480LINE;
				if (!SetDisplayMode(640, 480)) {
					nDDResolution = DDRES_400LINE;
					if (!SetDisplayMode(640, 400)) {
						return FALSE;
					}
				}
			}
			break;
		case 4:	/* 1280 x 800 */
			nDDResolution = DDRES_WXGA800;
			if (!SetDisplayMode(1280, 800)) {
				nDDResolution = DDRES_400LINE;
				if (!SetDisplayMode(640, 400)) {
					nDDResolution = DDRES_480LINE;
					if (!SetDisplayMode(640, 480)) {
						return FALSE;
					}
				}
			}
			break;
		default:ASSERT(FALSE);
				return FALSE;
	}

	/* �v���C�}���T�[�t�F�C�X���쐬 */
	memset(&ddsd, 0, sizeof(ddsd));
	ddsd.dwSize = sizeof(ddsd);
	ddsd.dwFlags = DDSD_CAPS;
	if ((nDDResolution == DDRES_400LINE) || (nDDResolution == DDRES_480LINE)) {
		ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;
	}
	else {
		ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE | DDSCAPS_SYSTEMMEMORY;
	}
	if (FAILED(lpdd2->CreateSurface(&ddsd, &lpdds[0], NULL))) {
		return FALSE;
	}

	/* ���[�N�T�[�t�F�C�X���쐬(DDSCAPS_SYSTEMMEMORY���w�肷��) */
	memset(&ddsd, 0, sizeof(ddsd));
	ddsd.dwSize = sizeof(ddsd);
	ddsd.dwFlags = DDSD_CAPS | DDSD_WIDTH | DDSD_HEIGHT;
	ddsd.dwWidth = 640;
	if (nDDResolution == DDRES_480LINE) {
		ddsd.dwHeight = 480;
	}
	else if (nDDResolution == DDRES_SXGA) {
		ddsd.dwHeight = 512;
	}
	else {
		ddsd.dwHeight = 400;
	}
	ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_SYSTEMMEMORY;
	if (FAILED(lpdd2->CreateSurface(&ddsd, &lpdds[1], NULL))) {
		return FALSE;
	}

	if (nPixelFormat == 0) {
		/* �s�N�Z���t�H�[�}�b�g�𓾂� */
		memset(&ddpf, 0, sizeof(ddpf));
		ddpf.dwSize = sizeof(ddpf);
		if (FAILED(lpdds[1]->GetPixelFormat(&ddpf))) {
			return FALSE;
		}

		/* �s�N�Z���t�H�[�}�b�g���`�F�b�N HEL�ŋK�肳��Ă���2�^�C�v�̂ݑΉ� */
		if (!(ddpf.dwFlags & DDPF_RGB)) {
			return FALSE;
		}
		if ((ddpf.dwRBitMask == 0xf800) &&
			(ddpf.dwGBitMask == 0x07e0) &&
			(ddpf.dwBBitMask == 0x001f)) {
			nPixelFormat = 1;
		}
		if ((ddpf.dwRBitMask == 0x7c00) &&
			(ddpf.dwGBitMask == 0x03e0) &&
			(ddpf.dwBBitMask == 0x001f)) {
			nPixelFormat = 2;
		}

		if (nPixelFormat == 0) {
			return FALSE;
		}
	}

#if XM7_VER == 1
	/* �����_�����O�֐��ݒ� */
	if (nPixelFormat == 3) {
		Render640 = Render640Tc32DD;
		Render640m = Render640mTc32DD;
#if defined(L4CARD)
		RenderL4 = RenderL4Tc32DD;
#endif
	}
	else {
		Render640 = Render640DD;
		Render640m = Render640mDD;
#if defined(L4CARD)
		RenderL4 = RenderL4DD;
#endif
	}
#elif XM7_VER == 2
	/* �����_�����O�֐��ݒ� */
	if (nPixelFormat == 3) {
		Render640 = Render640Tc32DD;
		Render320 = Render320Tc32DD;
		Render640c = Render640cTc32DD;
	}
	else {
		Render640 = Render640DD;
		Render320 = Render320DD;
		Render640c = Render640cDD;
	}
#elif XM7_VER >= 3
	/* �����_�����O�֐��ݒ� */
	if (nPixelFormat == 1) {
		Render640 = Render640DD2;
		Render320 = Render320DD;
		Render256k = Render256k565DD;
		Render640w = Render640wDD2;
		Render320w = Render320wDD;
		Render640c = Render640cDD;
		Render640cw = Render640cwDD;
	}
	else if (nPixelFormat == 2) {
		Render640 = Render640DD2;
		Render320 = Render320DD;
		Render256k = Render256k555DD;
		Render640w = Render640wDD2;
		Render320w = Render320wDD;
		Render640c = Render640cDD;
		Render640cw = Render640cwDD;
	}
	else if (nPixelFormat == 3) {
		Render640 = Render640Tc32DD2;
		Render320 = Render320Tc32DD;
		Render256k = Render256kTc32DD;
		Render640w = Render640wTc32DD2;
		Render320w = Render320wTc32DD;
		Render640c = Render640cTc32DD;
		Render640cw = Render640cwTc32DD;
	}
	else {
		Render640 = Render640Tc24DD2;
		Render320 = Render320Tc24DD;
		Render256k = Render256kTc24DD;
		Render640w = Render640wTc24DD2;
		Render320w = Render320wTc24DD;
		Render640c = Render640cTc24DD;
		Render640cw = Render640cwTc24DD;
	}
#endif

	/* �N���b�p�[���쐬�A���蓖�� */
	if (FAILED(lpdd2->CreateClipper(NULL, &lpddc, NULL))) {
		return FALSE;
	}
	if (FAILED(lpddc->SetHWnd(NULL, hDrawWnd))) {
		return FALSE;
	}

	/* �E�C���h�E�T�C�Y��ύX */
	if (nDDResolution != DDRES_400LINE) {
		if (hmonitor && getmonitorinfo) {
			memset(&minfo, 0, sizeof(minfo));
			minfo.cbSize = sizeof(minfo);
			getmonitorinfo(hmonitor, &minfo);
			srect = minfo.rcWork;
		}
		else {
			srect.left = 0;
			srect.top = 0;
			srect.right = nScrX;
			srect.bottom = nScrY;
		}
		if (hStatusBar) {
			GetWindowRect(hStatusBar, &brect);
		}
		else {
			brect.top = 0;
			brect.bottom = 0;
		}
		MoveWindow(hMainWnd, srect.left, srect.top, nScrX, nScrY + (brect.bottom - brect.top), TRUE);
		MoveWindow(hDrawWnd, 0, 0, nScrX, nScrY, TRUE);
		if (hStatusBar) {
			MoveWindow(hStatusBar, 0, nScrY, 0, (brect.bottom - brect.top),
						TRUE);
		}
	}

	/* ���j���[�o�[�����E�u���V�L��(Windows 8�̂�) */
	if (bWin8flag) {
		hDrawBrush = (HBRUSH)GetClassLong(hDrawWnd, GCL_HBRBACKGROUND);
		hMainBrush = (HBRUSH)GetClassLong(hMainWnd, GCL_HBRBACKGROUND);
		SetClassLong(hDrawWnd, GCL_HBRBACKGROUND, NULL);
		SetClassLong(hMainWnd, GCL_HBRBACKGROUND, NULL);

		SetMenu(hMainWnd, NULL);
		DrawMenuBar(hMainWnd);
	}

	/* �L���v�V�����\���t���O��ݒ� */
	if ((nDDResolution == DDRES_480LINE) || (nDDResolution == DDRES_SXGA)) {
		bCaption = bDD480Status;
	}
	else {
		bCaption = TRUE;
	}

	/* �}�E�X�ʒu����ʒ����Ɉړ����� */
	GetWindowRect(hMainWnd, &srect);
	ptFullScr.x = (srect.right + srect.left) / 2;
	ptFullScr.y = (srect.bottom + srect.top) / 2;
	SetCursorPos(ptFullScr.x, ptFullScr.y);

	/* ���[�N�Z�b�g */
#if XM7_VER >= 3
	nMode = SCR_200LINE;
#elif XM7_VER >= 2
	bAnalog = FALSE;
#elif XM7_VER == 1 && defined(L4CARD)
	b400Line = FALSE;
#endif
	ReDrawDD();
	AllClear(TRUE);
	InvalidateRect(hMainWnd, NULL, TRUE);
	InvalidateRect(hDrawWnd, NULL, TRUE);

	/* ���� */
	return TRUE;
}

/*-[ �`�� ]-----------------------------------------------------------------*/

#if XM7_VER == 2 && defined(FMTV151)
/*
 *	V2����
 */
static void FASTCALL DrawV2_DD(BYTE *pSurface, LONG lPitch)
{
	BYTE	x, y;
	int		xx, yy;
	WORD	col;
	WORD	*pbits;
	DWORD	*pbitsTC;

	/* �p���b�g�e�[�u�� */
	static const WORD V2rgbTable[] = {
		0x0000 | 0x0000 | 0x0000,
		0x0000 | 0x07e0 | 0x0000,
		0x0000 | 0x0000 | 0x0000,
		0x0000 | 0x03e0 | 0x0000,
		0x0000 | 0x0000 | 0x0000
	};
	static const WORD V2rgbTableTc[] = {
		0x00000000,	0x0000ff00,	0x00000000
	};

	if (!bFMTV151) {
		return;
	}

	if (!bRasterRendering) {
		if ((nDrawBottom < (V2YPoint * 2)) ||
			(nDrawTop > ((V2YPoint + V2YSize * V2YPxSz) * 2 - 1)) ||
			(nDrawRight < V2XPoint) ||
			(nDrawLeft > (V2XPoint + V2XSize * V2XPxSz - 1))) {
			return;
		}
	}


	if (nPixelFormat == 3) {
		pbitsTC = (DWORD *)(pSurface + V2YPoint * lPitch * 2 + V2XPoint * 4);

		for (y = 0; y < V2YSize; y ++) {
			for (x = 0; x < V2XSize; x ++) {
				if (nV2data[y * V2XSize + x]) {
					col = V2rgbTableTc[nV2data[y * V2XSize + x] + 0];
					for (yy = y * V2YPxSz; yy < (y + 1) * V2YPxSz; yy ++) {
						for (xx = x * V2XPxSz; xx < (x + 1) * V2XPxSz; xx ++) {
							pbitsTC[yy * (lPitch / 2) + xx] = col;
							if (!bAnalog && bPseudo400Line) {
								pbitsTC[yy * (lPitch/2) +(lPitch/4)+xx] = col;
							}
						}
					}
				}
			}
		}
	}
	else {
		pbits = (WORD *)(pSurface + V2YPoint * lPitch * 2 + V2XPoint * 2);

		for (y = 0; y < V2YSize; y ++) {
			for (x = 0; x < V2XSize; x ++) {
				if (nV2data[y * V2XSize + x]) {
					if (nPixelFormat == 1) {
						col = V2rgbTable[nV2data[y * V2XSize + x] + 0];
					}
					else {
						col = V2rgbTable[nV2data[y * V2XSize + x] + 2];
					}
					for (yy = y * V2YPxSz; yy < (y + 1) * V2YPxSz; yy ++) {
						for (xx = x * V2XPxSz; xx < (x + 1) * V2XPxSz; xx ++) {
							pbits[yy * lPitch + xx] = col;
							if (!bAnalog && bPseudo400Line) {
								pbits[yy * lPitch + (lPitch / 2) + xx] = col;
							}
						}
					}
				}
			}
		}
	}
}
#endif

/*
 *	�S�Ă̍ĕ`��t���O��ݒ�
 */
static void FASTCALL SetDrawFlag(BOOL flag)
{
	memset(DDDrawFlag, (BYTE)flag, sizeof(DDDrawFlag));
}

/*
 *	�S�̈�N���A
 */
static void FASTCALL AllClear(BOOL clear_flag)
{
	DDSURFACEDESC ddsd;
	HRESULT hResult;
	RECT rect;
	RECT drect;
	BYTE *p;
	int i;
	int lines;

	/* �t���O�`�F�b�N */
	if (!bClearFlag) {
		return;
	}

	/* �T�[�t�F�C�X�����b�N */
	memset(&ddsd, 0, sizeof(ddsd));
	ddsd.dwSize = sizeof(ddsd);
	hResult = lpdds[1]->Lock(NULL, &ddsd,
							 DDLOCK_WAIT | DDLOCK_SURFACEMEMORYPTR, NULL);
	if (hResult != DD_OK) {
		/* �T�[�t�F�C�X�����X�g���Ă���΁A���X�g�A */
		if (hResult == DDERR_SURFACELOST) {
			lpdds[1]->Restore();
		}
		/* ����͑S�̈�X�V */
		ReDrawDD();
		return;
	}

	/* �I�[���N���A */
	if (clear_flag) {
		p = (BYTE *)ddsd.lpSurface;
		if (nDDResolution == DDRES_480LINE) {
			lines = 480;
		}
		else if (nDDResolution == DDRES_SXGA) {
			lines = 512;
		}
		else {
			lines = 400;
		}
		for (i=0; i<lines; i++) {
			if (nPixelFormat == 3) {
				memset(p, 0, 640 * 4);
			}
#if XM7_VER >= 3
			else if (nPixelFormat == 4) {
				memset(p, 0, 640 * 3);
			}
#endif
			else {
				memset(p, 0, 640 * 2);
			}
			p += ddsd.lPitch;
		}
	}

	/* �T�[�t�F�C�X���A�����b�N */
	lpdds[1]->Unlock(ddsd.lpSurface);

	/* ���[�N���Z�b�g */
	bClearFlag = FALSE;
	nDrawTop = 0;
	nDrawBottom = 400;
	nDrawLeft = 0;
	nDrawRight = 640;
	SetDrawFlag(TRUE);
#if XM7_VER >= 3
	bWindowOpen = FALSE;
	nWindowDx1 = 640;
	nWindowDy1 = 400;
	nWindowDx2 = 0;
	nWindowDy2 = 0;
#endif
	SetDirtyFlag(0, 400, TRUE);

	/* �X�e�[�^�X���C�����N���A */
	szCaption[0] = '\0';
	nCAP = -1;
	nKANA = -1;
	nINS = -1;
	nDrive[0] = -1;
	nDrive[1] = -1;
	szDrive[0][0] = '\0';
	szDrive[1][0] = '\0';
	nTape = -1;

	/* 640x480���̂�Blt */
	if ((nDDResolution == DDRES_400LINE) || (nDDResolution == DDRES_WUXGA) ||
		(nDDResolution == DDRES_WXGA800)) {
		return;
	}

	/* �����������΁ABlt */
	if (nDDResolution == DDRES_480LINE) {
		rect.top = 0;
		rect.left = 0;
		rect.right = 640;
		rect.bottom = 40;
		drect = rect;
	}
	else if (nDDResolution == DDRES_SXGA) {
		rect.top = 0;
		rect.left = 0;
		rect.right = 640;
		rect.bottom = 56;
		drect.top = 0;
		drect.left = 0;
		drect.right = 1280;
		drect.bottom = 112;
	}
	hResult = lpdds[0]->Blt(&drect, lpdds[1], &rect, DDBLT_WAIT, NULL);
	if (hResult != DD_OK) {
		/* �T�[�t�F�C�X�����X�g���Ă���΁A���X�g�A */
		if (hResult == DDERR_SURFACELOST) {
			lpdds[0]->Restore();
			lpdds[1]->Restore();
		}
		/* ����͑S�̈�X�V */
		ReDrawDD();
		return;
	}

	/* �����C���W�P�[�^�`�� */
	if (nDDResolution == DDRES_480LINE) {
		rect.top = 440;
		rect.left = 0;
		rect.right = 640;
		rect.bottom = 480;
		drect = rect;
	}
	else if (nDDResolution == DDRES_SXGA) {
		rect.top = 456;
		rect.left = 0;
		rect.right = 640;
		rect.bottom = 480;
		drect.top = 912;
		drect.left = 0;
		drect.right = 1280;
		drect.bottom = 1024;
	}
	hResult = lpdds[0]->Blt(&drect, lpdds[1], &rect, DDBLT_WAIT, NULL);
	if (hResult != DD_OK) {
		/* �T�[�t�F�C�X�����X�g���Ă���΁A���X�g�A */
		if (hResult == DDERR_SURFACELOST) {
			lpdds[0]->Restore();
			lpdds[1]->Restore();
		}
		/* ����͑S�̈�X�V */
		ReDrawDD();
		return;
	}
}

/*
 *	�X�L�������C���`��
 */
static void FASTCALL RenderFullScan(BYTE *pSurface, LONG lPitch)
{
	WORD u;
	WORD top, bottom;

	/* ���X�^�P�ʃ����_�����O���ɉ�ʑS�̂���������K�v������̂� */
	if (nDrawTop >= nDrawBottom) {
		top = 0;
		bottom = 400;
	}
	else {
		top = nDrawTop;
		bottom = nDrawBottom;
	}

	/* �����ݒ� */
	pSurface += top * lPitch;

	/* �t���O�`�F�b�N */
	if (bFullScanFS) {
		/* ���[�v */
		for (u=top; u<bottom; u += (WORD)2) {
			if (nPixelFormat == 3) {
				memcpy(&pSurface[lPitch], pSurface, 640 * 4);
			}
#if XM7_VER >= 3
			else if (nPixelFormat == 4) {
				memcpy(&pSurface[lPitch], pSurface, 640 * 3);
			}
#endif
			else {
				memcpy(&pSurface[lPitch], pSurface, 640 * 2);
			}
			pSurface += (lPitch * 2);
		}
	}
	else {
		/* ���X�^�P�ʃ����_�����O���͂����ŃN���A���Ȃ��Ƃ����Ȃ� */
		if (bRasterRendering) {
			/* ���[�v */
			for (u=top; u<bottom; u += (WORD)2) {
				if (nPixelFormat == 3) {
					memset(&pSurface[lPitch], 0x00, 640 * 4);
				}
#if XM7_VER >= 3
				else if (nPixelFormat == 4) {
					memset(&pSurface[lPitch], 0x00, 640 * 3);
				}
#endif
				else {
					memset(&pSurface[lPitch], 0x00, 640 * 2);
				}
				pSurface += (lPitch * 2);
			}
		}
	}
}

/*
 *	�X�e�[�^�X���C��(�L���v�V����)�`��
 */
static void FASTCALL DrawCaption(void)
{
	HDC hDC;
	HRESULT hResult;
	RECT rect;
	RECT drect;

	ASSERT((nDDResolution == DDRES_480LINE) || (nDDResolution == DDRES_SXGA));
	ASSERT(bCaption);

	/* DC�擾 */
	hResult = lpdds[1]->GetDC(&hDC);
	if (hResult != DD_OK) {
		/* �T�[�t�F�C�X�����X�g���Ă���΁A���X�g�A */
		if (hResult == DDERR_SURFACELOST) {
			lpdds[1]->Restore();
		}
		/* ����͑S�̈�X�V */
		ReDrawDD();
		return;
	}

	/* ExtTextOut���g���A��x�ŕ`�� */
	SetBkColor(hDC, RGB(0, 0, 0));
	SetTextColor(hDC, RGB(255, 255, 255));
	if (nDDResolution == DDRES_480LINE) {
		rect.top = 0;
		rect.left = 0;
		rect.right = 640;
		rect.bottom = 40;
	}
	else if (nDDResolution == DDRES_SXGA) {
		rect.top = 0;
		rect.left = 0;
		rect.right = 640;
		rect.bottom = 56;
	}
	ExtTextOut(hDC, 0, 0, ETO_OPAQUE, &rect,
					 szCaption, strlen(szCaption), NULL);

	/* DC����� */
	hResult = lpdds[1]->ReleaseDC(hDC);
	if (hResult != DD_OK) {
		/* �T�[�t�F�C�X�����X�g���Ă���΁A���X�g�A */
		if (hResult == DDERR_SURFACELOST) {
			lpdds[1]->Restore();
		}
		/* ����͑S�̈�X�V */
		ReDrawDD();
		return;
	}

	/* Blt */
	if (nDDResolution == DDRES_480LINE) {
		drect.top = 0;
		drect.left = 0;
		drect.right = 640;
		drect.bottom = 40;
	}
	else if (nDDResolution == DDRES_SXGA) {
		drect.top = 0;
		drect.left = 0;
		drect.right = 1280;
		drect.bottom = 112;
	}
	hResult = lpdds[0]->Blt(&drect, lpdds[1], &rect, DDBLT_WAIT, NULL);
	if (hResult != DD_OK) {
		/* �T�[�t�F�C�X�����X�g���Ă���΁A���X�g�A */
		if (hResult == DDERR_SURFACELOST) {
			lpdds[0]->Restore();
			lpdds[1]->Restore();
		}
		/* ����͑S�̈�X�V */
		ReDrawDD();
		return;
	}
}

/*
 *	�L�����N�^�`��
 */
static void FASTCALL DrawChar(HDC hDC, BYTE c, int x, int y, int color)
{
	int i;
	int j;
	BYTE *p;
	BYTE dat;

	/* �T�uROM(C)�̃t�H���g�A�h���X�𓾂� */
	p = &subrom_c[c * 8];

	/* y���[�v */
	for (i=0; i<8; i++) {
		/* �f�[�^�擾 */
		dat = *p;
		p++;

		/* x���[�v */
		for (j=0; j<8; j++) {
			if (dat & 0x80) {
				SetPixelV(hDC, x, y, color);
			}
			else {
				SetPixelV(hDC, x, y, RGB(0, 0, 0));
			}
			dat <<= 1;
			x++;
		}

		/* ����y�� */
		x -= 8;
		y++;
	}
}

/*
 *	�X�e�[�^�X���C��(�L���v�V����)
 */
static BOOL FASTCALL StatusCaption(void)
{
	char string[256];
	char tmp[256];
	char drive[_MAX_DRIVE];
	char dir[_MAX_DIR];
	char fname[_MAX_FNAME];
	char ext[_MAX_EXT];

	/* ����󋵂ɉ����āA�R�s�[ */
	if (run_flag) {
		strncpy(string, szRunMessage, sizeof(string));
	}
	else {
		strncpy(string, szStopMessage, sizeof(string));
	}
	strncat(string, " ", sizeof(string) - strlen(string) - 1);

#if !defined(DISABLE_FULLSPEED)
	/* CPU���x�䗦 */
	if (bAutoSpeedAdjust) {
		_snprintf(tmp, sizeof(tmp), "(%3d%%) ", speed_ratio / 100);
		strncat(string, tmp, sizeof(string) - strlen(string) - 1);
	}
#endif

	/* �t���b�s�[�f�B�X�N�h���C�u 0 */
	if (fdc_ready[0] != FDC_TYPE_NOTREADY) {
		/* �t�@�C���l�[���{�g���q�̂ݎ��o�� */
		_splitpath(fdc_fname[0], drive, dir, fname, ext);
		_snprintf(tmp, sizeof(tmp), "- %s%s ", fname, ext);
		strncat(string, tmp, sizeof(string) - strlen(string) - 1);
	}

	/* �t���b�s�[�f�B�X�N�h���C�u 1 */
	if (fdc_ready[1] != FDC_TYPE_NOTREADY) {
		if ((strcmp(fdc_fname[0], fdc_fname[1]) != 0) ||
			(fdc_ready[0] == FDC_TYPE_NOTREADY)) {
			/* �t�@�C���l�[���{�g���q�̂ݎ��o�� */
			_splitpath(fdc_fname[1], drive, dir, fname, ext);
			if (fdc_ready[0] == FDC_TYPE_NOTREADY) {
				_snprintf(tmp, sizeof(tmp), "- (%s%s) ", fname, ext);
			}
			else {
				_snprintf(tmp, sizeof(tmp), "(%s%s) ", fname, ext);
			}
			strncat(string, tmp, sizeof(string) - strlen(string) - 1);
		}
	}

	/* �e�[�v */
	if (tape_fileh != -1) {
		/* �t�@�C���l�[���{�g���q�̂ݎ��o�� */
		_splitpath(tape_fname, drive, dir, fname, ext);
		_snprintf(tmp, sizeof(tmp), "- %s%s ", fname, ext);
		strncat(string, tmp, sizeof(string) - strlen(string) - 1);
	}

#if XM7_VER == 1 && defined(BUBBLE)
	/* �o�u�������� */
	if (fm_subtype == FMSUB_FM8) {
		if (bmc_ready[0] != BMC_TYPE_NOTREADY) {
			/* �t�@�C���l�[���{�g���q�̂ݎ��o�� */
			_splitpath(bmc_fname[0], drive, dir, fname, ext);
			_snprintf(tmp, sizeof(tmp), "- %s%s ", fname, ext);
			strncat(string, tmp, sizeof(string) - strlen(string) - 1);
		}

		if (bmc_ready[1] != BMC_TYPE_NOTREADY) {
			/* �t�@�C���l�[���{�g���q�̂ݎ��o�� */
			_splitpath(bmc_fname[1], drive, dir, fname, ext);
			if (bmc_ready[0] == BMC_TYPE_NOTREADY) {
				_snprintf(tmp, sizeof(tmp), "- (%s%s) ", fname, ext);
			}
			else {
				_snprintf(tmp, sizeof(tmp), "(%s%s) ", fname, ext);
			}
			strncat(string, tmp, sizeof(string) - strlen(string) - 1);
		}
	}
#endif

	/* ��r�`�� */
	string[255] = '\0';
	if (memcmp(szCaption, string, strlen(string) + 1) != 0) {
		strncpy(szCaption, string, sizeof(szCaption));
		return TRUE;
	}

	/* �O��Ɠ����Ȃ̂ŁA�`�悵�Ȃ��Ă悢 */
	return FALSE;
}

/*
 *	�X�e�[�^�X���C��(CAP)
 */
static BOOL FASTCALL StatusCAP(void)
{
	HDC hDC;
	HRESULT hResult;
	RECT rect;
	RECT drect;
	int num;

	/* �l�擾�A��r */
	if (caps_flag) {
		num = 1;
	}
	else {
		num = 0;
	}
	if (num == nCAP) {
		return FALSE;
	}

	/* DC�擾 */
	hResult = lpdds[1]->GetDC(&hDC);
	if (hResult != DD_OK) {
		/* �T�[�t�F�C�X�����X�g���Ă���΁A���X�g�A */
		if (hResult == DDERR_SURFACELOST) {
			lpdds[1]->Restore();
		}
		/* ����͑S�̈�X�V */
		ReDrawDD();
		return FALSE;
	}

	/* -1�Ȃ�A�S�̈�N���A */
	if (nDDResolution == DDRES_480LINE) {
		rect.left = 0;
		rect.right = 640;
		rect.top = 440;
		rect.bottom = 480;
	}
	else {
		rect.left = 0;
		rect.right = 640;
		rect.top = 472;
		rect.bottom = 512;
	}
	drect = rect;
	SetBkColor(hDC, RGB(0, 0, 0));
	SetTextColor(hDC, RGB(255, 255, 255));
	if (nCAP == -1) {
		/* �N���A */
		ExtTextOut(hDC, 0, 0, ETO_OPAQUE, &rect, NULL, 0, NULL);

		/* "CAP"�̕����`�� */
		TextOut(hDC, 500, drect.top + 4, szCAPMessage, strlen(szCAPMessage));

		/* ���N��`�� */
		rect.left = 500;
		rect.right = rect.left + 30;
		rect.top = drect.top + 25;
		rect.bottom = rect.top + 10;
		FrameRect(hDC, &rect, (HBRUSH)GetStockObject(WHITE_BRUSH));
	}

	/* �����ŃR�s�[ */
	nCAP = num;

	/* ���C���F�`�� */
	rect.left = 501;
	rect.right = rect.left + 28;
	rect.top = drect.top + 26;
	rect.bottom = rect.top + 8;
	if (nCAP == 1) {
		SetBkColor(hDC, RGB(255, 0, 0));
	}
	else {
		SetBkColor(hDC, RGB(0, 0, 0));
	}
	ExtTextOut(hDC, 0, 0, ETO_OPAQUE, &rect, NULL, 0, NULL);

	/* DC��� */
	hResult = lpdds[1]->ReleaseDC(hDC);
	if (hResult != DD_OK) {
		/* �T�[�t�F�C�X�����X�g���Ă���΁A���X�g�A */
		if (hResult == DDERR_SURFACELOST) {
			lpdds[1]->Restore();
		}
		/* ����͑S�̈�X�V */
		ReDrawDD();
		return FALSE;
	}

	return TRUE;
}

/*
 *	�X�e�[�^�X���C��(����)
 */
static BOOL FASTCALL StatusKANA(void)
{
	HDC hDC;
	HRESULT hResult;
	RECT rect;
	RECT drect;
	int num;

	/* �l�擾�A��r */
	if (kana_flag) {
		num = 1;
	}
	else {
		num = 0;
	}
	if (num == nKANA) {
		return FALSE;
	}

	/* DC�擾 */
	hResult = lpdds[1]->GetDC(&hDC);
	if (hResult != DD_OK) {
		/* �T�[�t�F�C�X�����X�g���Ă���΁A���X�g�A */
		if (hResult == DDERR_SURFACELOST) {
			lpdds[1]->Restore();
		}
		/* ����͑S�̈�X�V */
		ReDrawDD();
		return FALSE;
	}

	if (nDDResolution == DDRES_480LINE) {
		rect.left = 0;
		rect.right = 640;
		rect.top = 440;
		rect.bottom = 480;
	}
	else {
		rect.left = 0;
		rect.right = 640;
		rect.top = 472;
		rect.bottom = 512;
	}
	drect = rect;
	SetBkColor(hDC, RGB(0, 0, 0));
	SetTextColor(hDC, RGB(255, 255, 255));
	if (nKANA == -1) {
		/* "����"�̕����`�� */
		TextOut(hDC, 546, drect.top + 4, szKANAMessage, strlen(szKANAMessage));

		/* ���N��`�� */
		rect.left = 546;
		rect.right = rect.left + 30;
		rect.top = drect.top + 25;
		rect.bottom = rect.top + 10;
		FrameRect(hDC, &rect, (HBRUSH)GetStockObject(WHITE_BRUSH));
	}

	/* �����ŃR�s�[ */
	nKANA = num;

	/* ���C���F�`�� */
	rect.left = 547;
	rect.right = rect.left + 28;
	rect.top = drect.top + 26;
	rect.bottom = rect.top + 8;
	if (nKANA == 1) {
		SetBkColor(hDC, RGB(255, 0, 0));
	}
	else {
		SetBkColor(hDC, RGB(0, 0, 0));
	}
	ExtTextOut(hDC, 0, 0, ETO_OPAQUE, &rect, NULL, 0, NULL);

	/* DC��� */
	hResult = lpdds[1]->ReleaseDC(hDC);
	if (hResult != DD_OK) {
		/* �T�[�t�F�C�X�����X�g���Ă���΁A���X�g�A */
		if (hResult == DDERR_SURFACELOST) {
			lpdds[1]->Restore();
		}
		/* ����͑S�̈�X�V */
		ReDrawDD();
		return FALSE;
	}

	return TRUE;
}

/*
 *	�X�e�[�^�X���C��(INS)
 */
static BOOL FASTCALL StatusINS(void)
{
	HDC hDC;
	HRESULT hResult;
	RECT rect;
	RECT drect;
	int num;

	/* �l�擾�A��r */
	if (ins_flag) {
		num = 1;
	}
	else {
		num = 0;
	}
	if (num == nINS) {
		return FALSE;
	}

	/* DC�擾 */
	hResult = lpdds[1]->GetDC(&hDC);
	if (hResult != DD_OK) {
		/* �T�[�t�F�C�X�����X�g���Ă���΁A���X�g�A */
		if (hResult == DDERR_SURFACELOST) {
			lpdds[1]->Restore();
		}
		/* ����͑S�̈�X�V */
		ReDrawDD();
		return FALSE;
	}

	if (nDDResolution == DDRES_480LINE) {
		rect.left = 0;
		rect.right = 640;
		rect.top = 440;
		rect.bottom = 480;
	}
	else {
		rect.left = 0;
		rect.right = 640;
		rect.top = 472;
		rect.bottom = 512;
	}
	drect = rect;
	SetBkColor(hDC, RGB(0, 0, 0));
	SetTextColor(hDC, RGB(255, 255, 255));
	if (nINS == -1) {
		/* "INS"�̕����`�� */
		TextOut(hDC, 595, drect.top + 4, szINSMessage, strlen(szINSMessage));

		/* ���N��`�� */
		rect.left = 593;
		rect.right = rect.left + 30;
		rect.top = drect.top + 25;
		rect.bottom = rect.top + 10;
		FrameRect(hDC, &rect, (HBRUSH)GetStockObject(WHITE_BRUSH));
	}

	/* �����ŃR�s�[ */
	nINS = num;

	/* ���C���F�`�� */
	rect.left = 594;
	rect.right = rect.left + 28;
	rect.top = drect.top + 26;
	rect.bottom = rect.top + 8;
	if (nINS == 1) {
		SetBkColor(hDC, RGB(255, 0, 0));
	}
	else {
		SetBkColor(hDC, RGB(0, 0, 0));
	}
	ExtTextOut(hDC, 0, 0, ETO_OPAQUE, &rect, NULL, 0, NULL);

	/* DC��� */
	hResult = lpdds[1]->ReleaseDC(hDC);
	if (hResult != DD_OK) {
		/* �T�[�t�F�C�X�����X�g���Ă���΁A���X�g�A */
		if (hResult == DDERR_SURFACELOST) {
			lpdds[1]->Restore();
		}
		/* ����͑S�̈�X�V */
		ReDrawDD();
		return FALSE;
	}

	return TRUE;
}

/*
 *	�X�e�[�^�X���C��(�t���b�s�[�h���C�u)
 */
static BOOL FASTCALL StatusDrive(int drive)
{
	char string[_MAX_FNAME + _MAX_EXT];
	char buffer[_MAX_FNAME + _MAX_EXT];
	char drive_[_MAX_DRIVE];
	char dir[_MAX_DIR];
	char fname[_MAX_FNAME];
	char ext[_MAX_EXT];
	HDC hDC;
	HRESULT hResult;
	RECT rect, srect;
	int num;
	char *name;

	ASSERT((drive == 0) || (drive == 1));

	/* �ԍ��Z�b�g */
	if (fdc_ready[drive] == FDC_TYPE_NOTREADY) {
		num = FDC_ACCESS_NOTREADY;
	}
	else {
		num = fdc_access[drive];
		if (num == FDC_ACCESS_SEEK) {
			num = FDC_ACCESS_READY;
		}
	}

	/* ���O�擾 */
	name = "";
	if ((fdc_ready[drive] == FDC_TYPE_D77) &&
		(strlen(fdc_name[drive][ fdc_media[drive] ]) > 0)) {
		name = fdc_name[drive][ fdc_media[drive] ];
	}
	else if (fdc_ready[drive] != FDC_TYPE_NOTREADY) {
		/* �t�@�C���l�[���{�g���q�̂ݎ��o�� */
		_splitpath(fdc_fname[drive], drive_, dir, fname, ext);
		strncpy(string, fname, sizeof(string));
		strncat(string, ext, sizeof(string) - strlen(string) - 1);
		if ((fdc_ready[drive] == FDC_TYPE_D77) && (fdc_medias[drive] > 1)) {
			_snprintf(buffer, sizeof(buffer), "%s #%02d", string, 
					  fdc_media[drive] + 1);
			name = buffer;
		}
		else {
			name = string;
		}
	}

	/* �ԍ���r */
	if (nDrive[drive] == num) {
		if (strcmp(szDrive[drive], name) == 0) {
			return FALSE;
		}
	}

	/* DC�擾 */
	hResult = lpdds[1]->GetDC(&hDC);
	if (hResult != DD_OK) {
		/* �T�[�t�F�C�X�����X�g���Ă���΁A���X�g�A */
		if (hResult == DDERR_SURFACELOST) {
			lpdds[1]->Restore();
		}
		/* ����͑S�̈�X�V */
		ReDrawDD();
		return FALSE;
	}

	/* �����ŃR�s�[ */
	nDrive[drive] = num;
	strncpy(szDrive[drive], name, sizeof(szDrive[drive]));

	/* ���W�ݒ� */
	SetBkColor(hDC, RGB(0, 0, 0));
	rect.left = (drive ^ 1) * 160;
	rect.right = ((drive ^ 1) + 1) * 160 - 4;
	if (nDDResolution == DDRES_480LINE) {
		rect.top = 444;
		rect.bottom = 474;
	}
	else {
		rect.top = 476;
		rect.bottom = 506;
	}
	srect.left = rect.left + 1;
	srect.right = rect.right + 1;
	srect.top = rect.top + 1;
	srect.bottom = rect.bottom + 1;

	/* �F���� */
	if ((nDrive[drive] != FDC_ACCESS_NOTREADY) && (!fdc_teject[drive])) {
		SetBkColor(hDC, RGB(63, 63, 63));
	}
	if (nDrive[drive] == FDC_ACCESS_READ) {
		SetBkColor(hDC, RGB(191, 0, 0));
	}
	if (nDrive[drive] == FDC_ACCESS_WRITE) {
		SetBkColor(hDC, RGB(0, 0, 191));
	}

	/* �w�i��h��Ԃ� */
	ExtTextOut(hDC, 0, 0, ETO_OPAQUE, &rect, NULL, 0, NULL);

	/* DrawText */
	SetTextColor(hDC, RGB(0, 0, 0));
	SetBkMode(hDC, TRANSPARENT);
	DrawText(hDC, szDrive[drive], strlen(szDrive[drive]), &srect,
		DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);
	SetTextColor(hDC, RGB(255, 255, 255));
	SetBkMode(hDC, TRANSPARENT);
	DrawText(hDC, szDrive[drive], strlen(szDrive[drive]), &rect,
		DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);

	/* �h���C�u�ԍ��`�� */
	if (fdc_ready[drive] != FDC_TYPE_NOTREADY) {
		DrawChar(hDC, (BYTE)(0x30 + drive), rect.right - 8, rect.bottom - 8,
				 RGB(255, 255, 255));
	}

	/* DC��� */
	hResult = lpdds[1]->ReleaseDC(hDC);
	if (hResult != DD_OK) {
		/* �T�[�t�F�C�X�����X�g���Ă���΁A���X�g�A */
		if (hResult == DDERR_SURFACELOST) {
			lpdds[1]->Restore();
		}
		/* ����͑S�̈�X�V */
		ReDrawDD();
		return FALSE;
	}

	return TRUE;
}

/*
 *	�X�e�[�^�X���C��(�e�[�v)
 */
static BOOL FASTCALL StatusTape(void)
{
	HDC hDC;
	HRESULT hResult;
	RECT rect, srect;
	int num;
	char string[64];

	/* �ԍ��Z�b�g */
	num = 30000;
	if (tape_fileh != -1) {
		num = (int)((tape_offset >> 8) % 10000);
		if (tape_motor) {
			if (tape_rec) {
				num += 20000;
			}
			else {
				num += 10000;
			}
		}
	}

	/* �ԍ���r */
	if (nTape == num) {
		return FALSE;
	}

	/* DC�擾 */
	hResult = lpdds[1]->GetDC(&hDC);
	if (hResult != DD_OK) {
		/* �T�[�t�F�C�X�����X�g���Ă���΁A���X�g�A */
		if (hResult == DDERR_SURFACELOST) {
			lpdds[1]->Restore();
		}
		/* ����͑S�̈�X�V */
		ReDrawDD();
		return FALSE;
	}

	/* �����ŃR�s�[ */
	nTape = num;

	/* ���W�ݒ� */
	SetBkColor(hDC, RGB(0, 0, 0));
	rect.left = 360;
	rect.right = rect.left + 80;
	if (nDDResolution == DDRES_480LINE) {
		rect.top = 444;
		rect.bottom = 474;
	}
	else {
		rect.top = 476;
		rect.bottom = 506;
	}
	srect.left = rect.left + 1;
	srect.right = rect.right + 1;
	srect.top = rect.top + 1;
	srect.bottom = rect.bottom + 1;

	/* �F�A�����񌈒� */
	if (nTape >= 30000) {
		string[0] = '\0';
	}
	else {
		_snprintf(string, sizeof(string), "%04d", nTape % 10000);
		if (nTape >= 10000) {
			if (nTape >= 20000) {
				SetBkColor(hDC, RGB(0, 0, 191));
			}
			else {
				SetBkColor(hDC, RGB(191, 0, 0));
			}
		}
		else {
			SetBkColor(hDC, RGB(63, 63, 63));
		}
	}

	/* �w�i��h��Ԃ� */
	ExtTextOut(hDC, 0, 0, ETO_OPAQUE, &rect, NULL, 0, NULL);

	/* DrawText */
	SetTextColor(hDC, RGB(0, 0, 0));
	SetBkMode(hDC, TRANSPARENT);
	DrawText(hDC, string, strlen(string), &srect,
		DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);
	SetTextColor(hDC, RGB(255, 255, 255));
	SetBkMode(hDC, TRANSPARENT);
	DrawText(hDC, string, strlen(string), &rect,
		DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);

	/* DC��� */
	hResult = lpdds[1]->ReleaseDC(hDC);
	if (hResult != DD_OK) {
		/* �T�[�t�F�C�X�����X�g���Ă���΁A���X�g�A */
		if (hResult == DDERR_SURFACELOST) {
			lpdds[1]->Restore();
		}
		/* ����͑S�̈�X�V */
		ReDrawDD();
		return FALSE;
	}

	return TRUE;
}

/*
 *	�X�e�[�^�X���C���`��
 */
static void FASTCALL StatusLine(void)
{
	BOOL flag;
	HRESULT hResult;
	RECT rect;
	RECT drect;

	/* 640x480/1280x1024�A�X�e�[�^�X����̏ꍇ�̂� */
	if ((nDDResolution == DDRES_400LINE || nDDResolution == DDRES_WUXGA) ||
		(nDDResolution == DDRES_WXGA800) || !bCaption) {
		return;
	}

	/* �L���v�V���� */
	if (StatusCaption()) {
		DrawCaption();
	}

	flag = FALSE;

	/* �L�[�{�[�h�X�e�[�^�X */
	if (StatusCAP()) {
		flag = TRUE;
	}
	if (StatusKANA()) {
		flag = TRUE;
	}
	if (StatusINS()) {
		flag = TRUE;
	}
	if (StatusDrive(0)) {
		flag = TRUE;
	}
	if (StatusDrive(1)) {
		flag = TRUE;
	}
	if (StatusTape()) {
		flag = TRUE;
	}

	/* �t���O���~��Ă���΁A�`�悷��K�v�Ȃ� */
	if (!flag) {
		return;
	}

	/* Blt */
	if (nDDResolution == DDRES_480LINE) {
		rect.left = 0;
		rect.right = 640;
		rect.top = 440;
		rect.bottom = 480;
		drect = rect;
	}
	else {
		rect.left = 0;
		rect.right = 640;
		rect.top = 456;
		rect.bottom = 512;
		drect.left = rect.left * 2;
		drect.right = rect.right * 2;
		drect.top = rect.top * 2;
		drect.bottom = rect.bottom * 2;
	}
	hResult = lpdds[0]->Blt(&drect, lpdds[1], &rect, DDBLT_WAIT, NULL);
	if (hResult != DD_OK) {
		/* �T�[�t�F�C�X�����X�g���Ă���΁A���X�g�A */
		if (hResult == DDERR_SURFACELOST) {
			lpdds[0]->Restore();
			lpdds[1]->Restore();
		}
		/* ����͑S�̈�X�V */
		ReDrawDD();
		return;
	}
}

/*
 *	640x200�A�f�W�^�����[�h
 *	�p���b�g�ݒ�
 */
static void FASTCALL Palet640(void)
{
	int i;
	int vpage;
#if XM7_VER == 1
	int base;
#endif
	BYTE col;

	/* HighColor�p�p���b�g�e�[�u�� */
	static const WORD rgbTable[] = {
		/* nPixelFormat = 1 */
		0x0000 | 0x0000 | 0x0000,
		0x0000 | 0x0000 | 0x001f,
		0xf800 | 0x0000 | 0x0000,
		0xf800 | 0x0000 | 0x001f,
		0x0000 | 0x07e0 | 0x0000,
		0x0000 | 0x07e0 | 0x001f,
		0xf800 | 0x07e0 | 0x0000,
		0xf800 | 0x07e0 | 0x001f,

		/* nPixelFormat = 2 */
		0x0000 | 0x0000 | 0x0000,
		0x0000 | 0x0000 | 0x001f,
		0x7c00 | 0x0000 | 0x0000,
		0x7c00 | 0x0000 | 0x001f,
		0x0000 | 0x03e0 | 0x0000,
		0x0000 | 0x03e0 | 0x001f,
		0x7c00 | 0x03e0 | 0x0000,
		0x7c00 | 0x03e0 | 0x001f,

		/* nPixelFormat = 1, Green Monitor */
		0x0000 | 0x0000 | 0x0000,
		0x0000 | 0x00c0 | 0x0000,
		0x0000 | 0x0240 | 0x0000,
		0x0000 | 0x0320 | 0x0000,
		0x0000 | 0x04a0 | 0x0000,
		0x0000 | 0x0580 | 0x0000,
		0x0000 | 0x0700 | 0x0000,
		0x0000 | 0x07e0 | 0x0000,

		/* nPixelFormat = 2, Green Monitor */
		0x0000 | 0x0000 | 0x0000,
		0x0000 | 0x0060 | 0x0000,
		0x0000 | 0x0120 | 0x0000,
		0x0000 | 0x0180 | 0x0000,
		0x0000 | 0x0240 | 0x0000,
		0x0000 | 0x02a0 | 0x0000,
		0x0000 | 0x0360 | 0x0000,
		0x0000 | 0x03e0 | 0x0000
	};

	/* TrueColor�p�p���b�g�e�[�u�� */
	static DWORD rgbTableTc[] = {
		/* nPixelFormat = 3 (TrueColor) */
		0x00000000,	0x000000ff,	0x00ff0000,	0x00ff00ff,
		0x0000ff00,	0x0000ffff,	0x00ffff00,	0x00ffffff,
		0x00000000, 0x00001c00, 0x00004c00, 0x00006800,
		0x00009600, 0x0000b200, 0x0000e200, 0x0000ff00
	};

	/* �t���O���Z�b�g����Ă��Ȃ���΁A�������Ȃ� */
	if (!bPaletFlag) {
		return;
	}

	/* �}���`�y�[�W���A�\���v���[�����𓾂� */
	vpage = (~(multi_page >> 4)) & 0x07;

	/* 640x200�A�f�W�^���p���b�g */
	for (i=0; i<8; i++) {
		if (crt_flag) {
			/* CRT ON */
#if XM7_VER == 1
			if (fm_subtype == FMSUB_FM8) {
				col = (BYTE)i;
			}
			else {
				col = (BYTE)(ttl_palet[i & vpage] & 0x07);
			}
#else
			col = (BYTE)(ttl_palet[i & vpage] & 0x07);
#endif

#if XM7_VER == 1
			if (bGreenMonitor) {
				if (nPixelFormat == 1) {
					base = 16;
				}
				else if (nPixelFormat == 2) {
					base = 24;
				}
				else {
					base = 8;
				}
			}
			else {
				if (nPixelFormat == 1) {
					base = 0;
				}
				else if (nPixelFormat == 2) {
					base = 8;
				}
				else {
					base = 0;
				}
			}

			if (bPseudo400Line) {
				if (nPixelFormat == 3) {
					if (col & 2) {
						rgbTTLDD[i + 8] = rgbTableTc[7 + base];
					}
					else {
						rgbTTLDD[i + 8] = rgbTableTc[0 + base];
					}
					if (col & 4) {
						rgbTTLDD[i] = rgbTableTc[7 + base];
					}
					else {
						rgbTTLDD[i] = rgbTableTc[0 + base];
					}
				}
				else {
					if (col & 2) {
						rgbTTLDD[i + 8] = rgbTable[7 + base];
					}
					else {
						rgbTTLDD[i + 8] = rgbTable[0 + base];
					}
					if (col & 4) {
						rgbTTLDD[i] = rgbTable[7 + base];
					}
					else {
						rgbTTLDD[i] = rgbTable[0 + base];
					}
				}
			}
			else {
				if (bGreenMonitor) {
					if (nPixelFormat == 1) {
						rgbTTLDD[i] = rgbTable[col + 16];
					}
					else if (nPixelFormat == 2) {
						rgbTTLDD[i] = rgbTable[col + 24];
					}
					else {
						rgbTTLDD[i] = rgbTableTc[col + 8];
					}
				}
				else {
					if (nPixelFormat == 1) {
						rgbTTLDD[i] = rgbTable[col + 0];
					}
					else if (nPixelFormat == 2) {
						rgbTTLDD[i] = rgbTable[col + 8];
					}
					else {
						rgbTTLDD[i] = rgbTableTc[col];
					}
				}
			}
#else
			if (nPixelFormat == 1) {
				rgbTTLDD[i] = rgbTable[col + 0];
			}
			else if (nPixelFormat == 2) {
				rgbTTLDD[i] = rgbTable[col + 8];
			}
			else {
				rgbTTLDD[i] = rgbTableTc[col];
			}
#endif
		}
		else {
			/* CRT OFF */
			rgbTTLDD[i] = 0;
		}
	}

	/* �t���O�~�낷 */
	bPaletFlag = FALSE;
}

/*
 *	640x200�A�f�W�^�����[�h
 *	�`��
 */
static void FASTCALL Draw640(void)
{
	DDSURFACEDESC ddsd;
	HRESULT hResult;
	RECT rect;
	RECT drect;
	BYTE *p;
#if XM7_VER >= 3
	WORD wdtop,wdbtm;
#endif

	/* �I�[���N���A */
	AllClear(TRUE);

	/* �p���b�g�ݒ� */
	Palet640();

	/* �X�e�[�^�X���C�� */
	StatusLine();

	/* �����_�����O�`�F�b�N */
	if ((nDrawTop >= nDrawBottom) || (nDrawLeft >= nDrawRight)) {
		return;
	}

	/* �T�[�t�F�C�X�����b�N */
	memset(&ddsd, 0, sizeof(ddsd));
	ddsd.dwSize = sizeof(ddsd);
	hResult = lpdds[1]->Lock(NULL, &ddsd,
							 DDLOCK_WAIT | DDLOCK_SURFACEMEMORYPTR, NULL);
	if (hResult != DD_OK) {
		/* �T�[�t�F�C�X�����X�g���Ă���΁A���X�g�A */
		if (hResult == DDERR_SURFACELOST) {
			lpdds[1]->Restore();
		}
		/* ����͑S�̈�X�V */
		ReDrawDD();
		return;
	}

	/* �����_�����O */
	p = (BYTE *)ddsd.lpSurface;
	if (nDDResolution == DDRES_480LINE) {
		p += (ddsd.lPitch * 40);
	}
	else if (nDDResolution == DDRES_SXGA) {
		p += (ddsd.lPitch * 56);
	}

#if XM7_VER >= 3
	/* �E�B���h�E�I�[�v���� */
	if (window_open) {
		/* �E�B���h�E�O �㑤�̕`�� */
		if ((nDrawTop >> 1) < window_dy1) {
			if (bPseudo400Line) {
				Render640c(p, ddsd.lPitch, nDrawTop >> 1, window_dy1);
			}
			else {
				Render640(p, ddsd.lPitch, nDrawTop >> 1, window_dy1, 2);
			}
		}

		/* �E�B���h�E���̕`�� */
		if ((nDrawTop >> 1) > window_dy1) {
			wdtop = (WORD)(nDrawTop >> 1);
		}
		else {
			wdtop = window_dy1;
		}

		if ((nDrawBottom >> 1) < window_dy2) {
			wdbtm = (WORD)(nDrawBottom >> 1);
		}
		else {
			wdbtm = window_dy2;
		}

		if (wdbtm > wdtop) {
			if (bPseudo400Line) {
				Render640cw(p, ddsd.lPitch,
					wdtop, wdbtm, window_dx1, window_dx2);
			}
			else {
				Render640w(p, ddsd.lPitch,
					wdtop, wdbtm, window_dx1, window_dx2, 2);
			}
		}

		/* �E�B���h�E�O �����̕`�� */
		if ((nDrawBottom >> 1) > window_dy2) {
			if (bPseudo400Line) {
				Render640c(p, ddsd.lPitch, window_dy2, nDrawBottom >> 1);
			}
			else {
				Render640(p, ddsd.lPitch, window_dy2, nDrawBottom >> 1, 2);
			}
		}
	}
	else {
		if (bPseudo400Line) {
			Render640c(p, ddsd.lPitch, nDrawTop >> 1, nDrawBottom >> 1);
		}
		else {
			Render640(p, ddsd.lPitch, nDrawTop >> 1, nDrawBottom >> 1, 2);
		}
	}
	if (!bPseudo400Line) {
		RenderFullScan(p, ddsd.lPitch);
	}
#elif XM7_VER == 2
	if (bPseudo400Line) {
		Render640c(p, ddsd.lPitch, nDrawTop >> 1, nDrawBottom >> 1);
	}
	else {
		Render640(p, ddsd.lPitch, nDrawTop >> 1, nDrawBottom >> 1);
	}
#if defined(FMTV151)
	DrawV2_DD(p, ddsd.lPitch);
#endif
	if (!bPseudo400Line) {
		RenderFullScan(p, ddsd.lPitch);
	}
#else
	if (bPseudo400Line) {
		Render640m(p, ddsd.lPitch, nDrawTop >> 1, nDrawBottom >> 1);
	}
	else {
		Render640(p, ddsd.lPitch, nDrawTop >> 1, nDrawBottom >> 1);
		RenderFullScan(p, ddsd.lPitch);
	}
#endif

	/* �T�[�t�F�C�X���A�����b�N */
	lpdds[1]->Unlock(ddsd.lpSurface);

	/* Blt */
	rect.top = nDrawTop;
	rect.left = nDrawLeft;
	rect.right = nDrawRight;
	rect.bottom = nDrawBottom;
	drect = rect;
	if (nDDResolution == DDRES_480LINE) {
		rect.top += 40;
		rect.bottom += 40;
		drect.top += 40;
		drect.bottom += 40;
	}
	else if (nDDResolution == DDRES_WUXGA) {
		drect.top *= 3;
		drect.bottom *= 3;
		drect.left *= 3;
		drect.right *= 3;
	}
	else if (nDDResolution == DDRES_SXGA) {
		rect.top += 56;
		rect.bottom += 56;
		drect.left *= 2;
		drect.right *= 2;
		drect.top = rect.top * 2;
		drect.bottom = rect.bottom * 2;
	}
	else if (nDDResolution == DDRES_WXGA800) {
		drect.top *= 2;
		drect.bottom *= 2;
		drect.left *= 2;
		drect.right *= 2;
	}
	hResult = lpdds[0]->Blt(&drect, lpdds[1], &rect, DDBLT_WAIT, NULL);
	if (hResult != DD_OK) {
		/* �T�[�t�F�C�X�����X�g���Ă���΁A���X�g�A */
		if (hResult == DDERR_SURFACELOST) {
			lpdds[0]->Restore();
			lpdds[1]->Restore();
		}
		/* ����͑S�̈�X�V */
		ReDrawDD();
		return;
	}

	/* ����ɔ����A���[�N���Z�b�g */
	nDrawTop = 400;
	nDrawBottom = 0;
	nDrawLeft = 640;
	nDrawRight = 0;
	SetDrawFlag(FALSE);
}

#if XM7_VER >= 3
/*
 *	640x400�A�f�W�^�����[�h
 *	�`��
 */
static void FASTCALL Draw400l(void)
{
	DDSURFACEDESC ddsd;
	HRESULT hResult;
	RECT rect;
	RECT drect;
	BYTE *p;
	WORD wdtop,wdbtm;

	/* �I�[���N���A */
	AllClear(TRUE);

	/* �p���b�g�ݒ� */
	Palet640();

	/* �X�e�[�^�X���C�� */
	StatusLine();

	/* �����_�����O�`�F�b�N */
	if ((nDrawTop >= nDrawBottom) || (nDrawLeft >= nDrawRight)) {
		return;
	}

	/* �T�[�t�F�C�X�����b�N */
	memset(&ddsd, 0, sizeof(ddsd));
	ddsd.dwSize = sizeof(ddsd);
	hResult = lpdds[1]->Lock(NULL, &ddsd,
							 DDLOCK_WAIT | DDLOCK_SURFACEMEMORYPTR, NULL);
	if (hResult != DD_OK) {
		/* �T�[�t�F�C�X�����X�g���Ă���΁A���X�g�A */
		if (hResult == DDERR_SURFACELOST) {
			lpdds[1]->Restore();
		}
		/* ����͑S�̈�X�V */
		ReDrawDD();
		return;
	}

	/* �����_�����O */
	p = (BYTE *)ddsd.lpSurface;
	if (nDDResolution == DDRES_480LINE) {
		p += (ddsd.lPitch * 40);
	}
	else if (nDDResolution == DDRES_SXGA) {
		p += (ddsd.lPitch * 56);
	}

	/* �E�B���h�E�I�[�v���� */
	if (window_open) {
		if (nDrawTop < window_dy1) {
			Render640(p, ddsd.lPitch, nDrawTop, window_dy1, 1);
		}

		if (nDrawTop > window_dy1) {
			wdtop = nDrawTop;
		}
		else {
			wdtop = window_dy1;
		}
		if (nDrawBottom < window_dy2) {
			wdbtm = nDrawBottom;
		}
		else {
			wdbtm = window_dy2;
		}
		if (wdbtm > wdtop) {
			Render640w(p, ddsd.lPitch, wdtop, wdbtm, window_dx1, window_dx2, 1);
		}
		if (nDrawBottom > window_dy2) {
			Render640(p, ddsd.lPitch, window_dy2, nDrawBottom, 1);
		}
	}
	else {
		Render640(p, ddsd.lPitch, nDrawTop, nDrawBottom, 1);
	}

	/* �T�[�t�F�C�X���A�����b�N */
	lpdds[1]->Unlock(ddsd.lpSurface);

	/* Blt */
	rect.top = nDrawTop;
	rect.left = nDrawLeft;
	rect.right = nDrawRight;
	rect.bottom = nDrawBottom;
	drect = rect;
	if (nDDResolution == DDRES_480LINE) {
		rect.top += 40;
		rect.bottom += 40;
		drect.top += 40;
		drect.bottom += 40;
	}
	else if (nDDResolution == DDRES_WUXGA) {
		drect.top *= 3;
		drect.bottom *= 3;
		drect.left *= 3;
		drect.right *= 3;
	}
	else if (nDDResolution == DDRES_SXGA) {
		rect.top += 56;
		rect.bottom += 56;
		drect.left *= 2;
		drect.right *= 2;
		drect.top = rect.top * 2;
		drect.bottom = rect.bottom * 2;
	}
	else if (nDDResolution == DDRES_WXGA800) {
		drect.top *= 2;
		drect.bottom *= 2;
		drect.left *= 2;
		drect.right *= 2;
	}
	hResult = lpdds[0]->Blt(&drect, lpdds[1], &rect, DDBLT_WAIT, NULL);
	if (hResult != DD_OK) {
		/* �T�[�t�F�C�X�����X�g���Ă���΁A���X�g�A */
		if (hResult == DDERR_SURFACELOST) {
			lpdds[0]->Restore();
			lpdds[1]->Restore();
		}
		/* ����͑S�̈�X�V */
		ReDrawDD();
		return;
	}

	/* ����ɔ����A���[�N���Z�b�g */
	nDrawTop = 400;
	nDrawBottom = 0;
	nDrawLeft = 640;
	nDrawRight = 0;
	SetDrawFlag(FALSE);
}
#endif

#if XM7_VER == 1 && defined(L4CARD)
/*
 *	640x400�A�P�F���[�h
 *	�p���b�g�ݒ�
 */
static void FASTCALL PaletL4(void)
{
	int i;
	int base;

	/* �t���O���Z�b�g����Ă��Ȃ���΁A�������Ȃ� */
	if (!bPaletFlag) {
		return;
	}

	if (nPixelFormat == 3) {
		/* 640x400�A�O���t�B�b�N�p���b�g */
		for (i=0; i<3; i++) {
			/* �p���b�g�J�n�ʒu��ݒ� */
			if (bGreenMonitor) {
				base = 16;
			}
			else {
				base = 0;
			}

			if (crt_flag) {
				if ((i == 0) || (multi_page & (0x08 << i))) {
					rgbTTLL4DD[i] = (DWORD)Palet16Tc_L4[(ttl_palet[0] & 0x0f) + base];
				}
				else {
					rgbTTLL4DD[i] = (DWORD)Palet16Tc_L4[(ttl_palet[1] & 0x0f) + base];
				}
			}
			else {
				rgbTTLL4DD[i] = (DWORD)Palet16Tc_L4[base];
			}
		}
	}
	else {
		/* �p���b�g�J�n�ʒu��ݒ� */
		if (bGreenMonitor) {
			if (nPixelFormat == 1) {
				base = 32;
			}
			else {
				base = 48;
			}
		}
		else {
			if (nPixelFormat == 1) {
				base = 0;
			}
			else {
				base = 16;
			}
		}

		/* 640x400�A�O���t�B�b�N�p���b�g */
		for (i=0; i<3; i++) {
			if (crt_flag) {
				if ((i == 0) || (multi_page & (0x08 << i))) {
					rgbTTLL4DD[i] = (DWORD)Palet16_L4[(ttl_palet[0] & 0x0f) + base];
				}
				else {
					rgbTTLL4DD[i] = (DWORD)Palet16_L4[(ttl_palet[1] & 0x0f) + base];
				}
			}
			else {
				rgbTTLL4DD[i] = (DWORD)Palet16_L4[base];
			}
		}
	}

	/* �t���O���~�낷 */
	bPaletFlag = FALSE;
}

/*
 *	�e�L�X�g���
 *	�����_�����O
 */
static void FASTCALL RenderTextDD(BYTE *p, DWORD pitch,
						int first, int last, int left, int right)
{
	DWORD	tmp;
	int		x, y, x2;
	WORD	addr;
	DWORD	col;
	BYTE	cursor_start, cursor_end, cursor_type;
	BYTE	raster, lines, maxx;
	BYTE	chr, atr, chr_dat;
	BYTE	line, line_old;
	int		xsize;

	/* �\��OFF�Ȃ牽�����Ȃ� */
	if (!crt_flag) {
		return;
	}

	/* cursor */
	cursor_start	= (BYTE)(crtc_register[10] & 0x1f);
	cursor_end		= (BYTE)(crtc_register[11] & 0x1f);
	cursor_type		= (BYTE)((crtc_register[10] & 0x60) >> 5);

	/* ��ʕ\����� */
	maxx			= (BYTE)(crtc_register[1] << 1);
	lines			= (BYTE)((crtc_register[9] & 0x1f) + 1);

	if (width40_flag) {
		xsize = 16;
	}
	else {
		xsize = 8;
	}

	for (x=left / xsize; x<right / xsize; x++) {
		line_old = 255;
		for (y=first; y<last; y++) {
			raster = (BYTE)(y % lines);
			line = (BYTE)(y / lines);
			if (line != line_old) {
				addr = (WORD)((text_start_addr + (line * maxx + x) * 2) & 0x0ffe);
				chr = tvram_c[addr + 0];
				atr = tvram_c[addr + 1];

				if (atr & 0x20) {
					/* Intensity attribute ON */
					if (bGreenMonitor) {
						if (nPixelFormat == 3) {
							col = Palet16Tc_L4[(atr & 0x07) + 24];
						}
						else if (nPixelFormat == 1) {
							col = Palet16_L4[(atr & 0x07) + 40];
						}
						else {
							col = Palet16_L4[(atr & 0x07) + 56];
						}
					}
					else {
						if (nPixelFormat == 3) {
							col = Palet16Tc_L4[(atr & 0x07) + 8];
						}
						else if (nPixelFormat == 1) {
							col = Palet16_L4[(atr & 0x07) + 8];
						}
						else {
							col = Palet16_L4[(atr & 0x07) + 24];
						}
					}
				}
				else {
					/* Intensity attribute OFF */
					if (bGreenMonitor) {
						if (nPixelFormat == 3) {
							col = Palet16Tc_L4[(atr & 0x07) + 16];
						}
						else if (nPixelFormat == 1) {
							col = Palet16_L4[(atr & 0x07) + 32];
						}
						else {
							col = Palet16_L4[(atr & 0x07) + 48];
						}
					}
					else {
						if (nPixelFormat == 3) {
							col = Palet16Tc_L4[(atr & 0x07) + 0];
						}
						else if (nPixelFormat == 1) {
							col = Palet16_L4[(atr & 0x07) + 0];
						}
						else {
							col = Palet16_L4[(atr & 0x07) + 16];
						}
					}
				}
			}

			/* Blink attribute */
			if ((!(atr & 0x10) || text_blink) && (raster < 16)) {
				chr_dat = subcg_l4[(WORD)(chr * 16) + raster];
			}
			else {
				chr_dat = 0x00;
			}

			/* Reverse attribute */
			if (atr & 0x08) {
				chr_dat = (BYTE)~chr_dat;
			}

			/* �ق�ł����āA���[���� */
			if ((addr == cursor_addr) && (cursor_type != 1) &&
				(cursor_blink || (cursor_type == 0))) {
				if ((raster >= cursor_start) && (raster <= cursor_end)) {
					chr_dat = (BYTE)~chr_dat;
				}
			}

			if (chr_dat) {
				for (x2=0; x2<8; x2++) {
					if (chr_dat & (1 << (7-x2))) {
						if (nPixelFormat == 3) {
							if (width40_flag) {
								tmp = y * pitch + (((x << 3) + x2) << 3);
								*(DWORD *)(&p[tmp + 0]) = col;
								*(DWORD *)(&p[tmp + 4]) = col;
							}
							else {
								tmp = y * pitch + (((x << 3) + x2) << 2);
								*(DWORD *)(&p[tmp + 0]) = col;
							}
						}
						else {
							if (width40_flag) {
								tmp = y * pitch + (((x << 3) + x2) << 2);
								*(WORD *)(&p[tmp + 0]) = (WORD)col;
								*(WORD *)(&p[tmp + 2]) = (WORD)col;
							}
							else {
								tmp = y * pitch + (((x << 3) + x2) << 1);
								*(WORD *)(&p[tmp + 0]) = (WORD)col;
							}
						}
					}
				}
			}

			line_old = line;
		}
	}
}

/*
 *	640x400�A�P�F���[�h
 *	�`��
 */
static void FASTCALL DrawL4(void)
{
	DDSURFACEDESC ddsd;
	HRESULT hResult;
	RECT rect;
	RECT drect;
	BYTE *p;

	/* �I�[���N���A */
	AllClear(TRUE);

	/* �p���b�g�ݒ� */
	PaletL4();

	/* �X�e�[�^�X���C�� */
	StatusLine();

	/* �����_�����O�`�F�b�N */
	if ((nDrawTop >= nDrawBottom) || (nDrawLeft >= nDrawRight)) {
		return;
	}

	/* �T�[�t�F�C�X�����b�N */
	memset(&ddsd, 0, sizeof(ddsd));
	ddsd.dwSize = sizeof(ddsd);
	hResult = lpdds[1]->Lock(NULL, &ddsd,
							 DDLOCK_WAIT | DDLOCK_SURFACEMEMORYPTR, NULL);
	if (hResult != DD_OK) {
		/* �T�[�t�F�C�X�����X�g���Ă���΁A���X�g�A */
		if (hResult == DDERR_SURFACELOST) {
			lpdds[1]->Restore();
		}
		/* ����͑S�̈�X�V */
		ReDrawDD();
		return;
	}

	/* �����_�����O */
	p = (BYTE *)ddsd.lpSurface;
	if (nDDResolution == DDRES_480LINE) {
		p += (ddsd.lPitch * 40);
	}
	else if (nDDResolution == DDRES_SXGA) {
		p += (ddsd.lPitch * 56);
	}

	RenderL4(p, ddsd.lPitch, nDrawTop, nDrawBottom);
	RenderTextDD(p, ddsd.lPitch, nDrawTop, nDrawBottom, nDrawLeft, nDrawRight);

	/* �T�[�t�F�C�X���A�����b�N */
	lpdds[1]->Unlock(ddsd.lpSurface);

	/* Blt */
	rect.top = nDrawTop;
	rect.left = nDrawLeft;
	rect.right = nDrawRight;
	rect.bottom = nDrawBottom;
	drect = rect;
	if (nDDResolution == DDRES_480LINE) {
		rect.top += 40;
		rect.bottom += 40;
		drect.top += 40;
		drect.bottom += 40;
	}
	else if (nDDResolution == DDRES_WUXGA) {
		drect.top *= 3;
		drect.bottom *= 3;
		drect.left *= 3;
		drect.right *= 3;
	}
	else if (nDDResolution == DDRES_SXGA) {
		rect.top += 56;
		rect.bottom += 56;
		drect.left *= 2;
		drect.right *= 2;
		drect.top = rect.top * 2;
		drect.bottom = rect.bottom * 2;
	}
	else if (nDDResolution == DDRES_WXGA800) {
		drect.top *= 2;
		drect.bottom *= 2;
		drect.left *= 2;
		drect.right *= 2;
	}
	hResult = lpdds[0]->Blt(&drect, lpdds[1], &rect, DDBLT_WAIT, NULL);
	if (hResult != DD_OK) {
		/* �T�[�t�F�C�X�����X�g���Ă���΁A���X�g�A */
		if (hResult == DDERR_SURFACELOST) {
			lpdds[0]->Restore();
			lpdds[1]->Restore();
		}
		/* ����͑S�̈�X�V */
		ReDrawDD();
		return;
	}

	/* ����ɔ����A���[�N���Z�b�g */
	nDrawTop = 400;
	nDrawBottom = 0;
	nDrawLeft = 640;
	nDrawRight = 0;
	SetDrawFlag(FALSE);
}
#endif

#if XM7_VER >= 2
/*
 *	320x200�A�A�i���O���[�h
 *	�p���b�g�ݒ�
 */
static void FASTCALL Palet320(void)
{
	int i, j;
	DWORD r, g, b;
	int amask;
	WORD color;

	/* �t���O���Z�b�g����Ă��Ȃ���΁A�������Ȃ� */
	if (!bPaletFlag) {
		return;
	}

	/* �A�i���O�}�X�N���쐬 */
	amask = 0;
	if (!(multi_page & 0x10)) {
		amask |= 0x000f;
	}
	if (!(multi_page & 0x20)) {
		amask |= 0x00f0;
	}
	if (!(multi_page & 0x40)) {
		amask |= 0x0f00;
	}

	for (i=0; i<4096; i++) {
		/* �ŉ��ʂ���5bit�Â�B,G,R */
#if XM7_VER == 2
		/* TTL RGB���� */
		if (crt_flag && bTTLMonitor) {
			r = 0;
			g = 0;
			b = 0;
			j = 0;
			/* R */
			if ((i & amask) & 0x0080) {
				j |= 2;
			}

			/* G */
			if ((i & amask) & 0x0800) {
				j |= 4;
			}

			/* B */
			if ((i & amask) & 0x0008) {
				j |= 1;
			}

			/* RGB���� */
			if (ttl_palet[j & 7] & 0x01) {
				b = 0x0f;
			}
			if (ttl_palet[j & 7] & 0x02) {
				r = 0x0f;
			}
			if (ttl_palet[j & 7] & 0x04) {
				g = 0x0f;
			}
		}
		else
#endif
		if (crt_flag) {
			j = i & amask;
			r = (WORD)apalet_r[j];
			g = (WORD)apalet_g[j];
			b = (WORD)apalet_b[j];
		}
		else {
			r = 0;
			g = 0;
			b = 0;
		}

		/* �s�N�Z���^�C�v�ɉ����ADWORD�f�[�^���쐬 */
		if (nPixelFormat >= 3) {
			/* R8bit, G8bit, B8bit (TrueColor) */
			r = (DWORD)((r << 20) | (r << 16));
			g = (DWORD)((g << 12) | (g <<  8));
			b = (DWORD)((b <<  4) | (b      ));

			/* �Z�b�g */
			rgbAnalogDD[i] = (DWORD)(r | g | b);
		}
		else {
			if (nPixelFormat == 1) {
				/* R5bit, G6bit, B5bit�^�C�v */
				r <<= 12;
				if (r > 0) {
					r |= 0x0800;
				}

				g <<= 7;
				if (g > 0) {
					g |= 0x0060;
				}

				b <<= 1;
				if (b > 0) {
					b |= 0x0001;
				}
			}
			else if (nPixelFormat == 2) {
				/* R5bit, G5bit, B5bit�^�C�v */
				r <<= 11;
				if (r > 0) {
					r |= 0x0400;
				}

				g <<= 6;
				if (g > 0) {
					g |= 0x0020;
				}

				b <<= 1;
				if (b > 0) {
					b |= 0x0001;
				}
			}

			/* �Z�b�g */
			color = (WORD)(r | g | b);
			rgbAnalogDD[i] = (DWORD)((color << 16) | color);
		}
	}

	/* �t���O�~�낷 */
	bPaletFlag = FALSE;
}

/*
 *	320x200�A�A�i���O���[�h
 *	�`��
 */
static void FASTCALL Draw320(void)
{
	DDSURFACEDESC ddsd;
	HRESULT hResult;
	RECT rect;
	RECT drect;
	BYTE *p;
#if XM7_VER >= 3
	WORD wdtop,wdbtm;
#endif

	/* �I�[���N���A */
	AllClear(TRUE);

	/* �p���b�g�ݒ� */
	Palet320();

	/* �X�e�[�^�X���C�� */
	StatusLine();

	/* �����_�����O�`�F�b�N */
	if ((nDrawTop >= nDrawBottom) || (nDrawLeft >= nDrawRight)) {
		return;
	}

	/* �T�[�t�F�C�X�����b�N */
	memset(&ddsd, 0, sizeof(ddsd));
	ddsd.dwSize = sizeof(ddsd);
	hResult = lpdds[1]->Lock(NULL, &ddsd,
							 DDLOCK_WAIT | DDLOCK_SURFACEMEMORYPTR, NULL);
	if (hResult != DD_OK) {
		/* �T�[�t�F�C�X�����X�g���Ă���΁A���X�g�A */
		if (hResult == DDERR_SURFACELOST) {
			lpdds[1]->Restore();
		}
		/* ����͑S�̈�X�V */
		ReDrawDD();
		return;
	}

	/* �����_�����O */
	p = (BYTE *)ddsd.lpSurface;
	if (nDDResolution == DDRES_480LINE) {
		p += (ddsd.lPitch * 40);
	}
	else if (nDDResolution == DDRES_SXGA) {
		p += (ddsd.lPitch * 56);
	}

#if XM7_VER >= 3
	/* �E�B���h�E�I�[�v���� */
	if (window_open) {
		/* �E�B���h�E�O �㑤�̕`�� */
		if ((nDrawTop >> 1) < window_dy1) {
			Render320(p, ddsd.lPitch, nDrawTop >> 1, window_dy1);
		}

		/* �E�B���h�E���̕`�� */
		if ((nDrawTop >> 1) > window_dy1) {
			wdtop = (WORD)(nDrawTop >> 1);
		}
		else {
			wdtop = window_dy1;
		}

		if ((nDrawBottom >> 1) < window_dy2) {
			wdbtm = (WORD)(nDrawBottom >> 1);
		}
		else {
			wdbtm = window_dy2;
		}

		if (wdbtm > wdtop) {
			Render320w(p, ddsd.lPitch, wdtop, wdbtm, window_dx1, window_dx2);
		}

		/* �E�B���h�E�O �����̕`�� */
		if ((nDrawBottom >> 1) > window_dy2) {
			Render320(p, ddsd.lPitch, window_dy2, nDrawBottom >> 1);
		}
	}
	else {
		Render320(p, ddsd.lPitch, nDrawTop>>1, nDrawBottom>>1);
	}
	RenderFullScan(p, ddsd.lPitch);
#else
	Render320(p, ddsd.lPitch, nDrawTop>>1, nDrawBottom>>1);
#if defined(FMTV151)
	DrawV2_DD(p, ddsd.lPitch);
#endif
	RenderFullScan(p, ddsd.lPitch);
#endif

	/* �T�[�t�F�C�X���A�����b�N */
	lpdds[1]->Unlock(ddsd.lpSurface);

	/* Blt */
	rect.top = nDrawTop;
	rect.left = nDrawLeft;
	rect.right = nDrawRight;
	rect.bottom = nDrawBottom;
	drect = rect;
	if (nDDResolution == DDRES_480LINE) {
		rect.top += 40;
		rect.bottom += 40;
		drect.top += 40;
		drect.bottom += 40;
	}
	else if (nDDResolution == DDRES_WUXGA) {
		drect.top *= 3;
		drect.bottom *= 3;
		drect.left *= 3;
		drect.right *= 3;
	}
	else if (nDDResolution == DDRES_SXGA) {
		rect.top += 56;
		rect.bottom += 56;
		drect.left *= 2;
		drect.right *= 2;
		drect.top = rect.top * 2;
		drect.bottom = rect.bottom * 2;
	}
	else if (nDDResolution == DDRES_WXGA800) {
		drect.top *= 2;
		drect.bottom *= 2;
		drect.left *= 2;
		drect.right *= 2;
	}
	hResult = lpdds[0]->Blt(&drect, lpdds[1], &rect, DDBLT_WAIT, NULL);
	if (hResult != DD_OK) {
		/* �T�[�t�F�C�X�����X�g���Ă���΁A���X�g�A */
		if (hResult == DDERR_SURFACELOST) {
			lpdds[0]->Restore();
			lpdds[1]->Restore();
		}
		/* ����͑S�̈�X�V */
		ReDrawDD();
		return;
	}

	/* ����ɔ����A���[�N���Z�b�g */
	nDrawTop = 400;
	nDrawBottom = 0;
	nDrawLeft = 640;
	nDrawRight = 0;
	SetDrawFlag(FALSE);
}
#endif

#if XM7_VER >= 3
/*
 *	320x200�A26���F���[�h
 *	�`��
 */
static void FASTCALL Draw256k(void)
{
	DDSURFACEDESC ddsd;
	HRESULT hResult;
	RECT rect;
	RECT drect;
	BYTE *p;

	/* �I�[���N���A */
	AllClear(TRUE);

	/* �X�e�[�^�X���C�� */
	StatusLine();

	/* �����_�����O�`�F�b�N */
	if ((nDrawTop >= nDrawBottom) || (nDrawLeft >= nDrawRight)) {
		return;
	}

	/* �T�[�t�F�C�X�����b�N */
	memset(&ddsd, 0, sizeof(ddsd));
	ddsd.dwSize = sizeof(ddsd);
	hResult = lpdds[1]->Lock(NULL, &ddsd,
							 DDLOCK_WAIT | DDLOCK_SURFACEMEMORYPTR, NULL);
	if (hResult != DD_OK) {
		/* �T�[�t�F�C�X�����X�g���Ă���΁A���X�g�A */
		if (hResult == DDERR_SURFACELOST) {
			lpdds[1]->Restore();
		}
		/* ����͑S�̈�X�V */
		ReDrawDD();
		return;
	}

	/* �����_�����O */
	p = (BYTE *)ddsd.lpSurface;
	if (nDDResolution == DDRES_480LINE) {
		p += (ddsd.lPitch * 40);
	}
	else if (nDDResolution == DDRES_SXGA) {
		p += (ddsd.lPitch * 56);
	}

	if (crt_flag) {
		Render256k(p, ddsd.lPitch, nDrawTop >> 1, nDrawBottom >> 1, multi_page);
	}
	else {
		Render256k(p, ddsd.lPitch, nDrawTop >> 1, nDrawBottom >> 1, 0xff);
	}
	RenderFullScan(p, ddsd.lPitch);

	/* �T�[�t�F�C�X���A�����b�N */
	lpdds[1]->Unlock(ddsd.lpSurface);

	/* Blt */
	rect.top = nDrawTop;
	rect.left = nDrawLeft;
	rect.right = nDrawRight;
	rect.bottom = nDrawBottom;
	drect = rect;
	if (nDDResolution == DDRES_480LINE) {
		rect.top += 40;
		rect.bottom += 40;
		drect.top += 40;
		drect.bottom += 40;
	}
	else if (nDDResolution == DDRES_WUXGA) {
		drect.top *= 3;
		drect.bottom *= 3;
		drect.left *= 3;
		drect.right *= 3;
	}
	else if (nDDResolution == DDRES_SXGA) {
		rect.top += 56;
		rect.bottom += 56;
		drect.left *= 2;
		drect.right *= 2;
		drect.top = rect.top * 2;
		drect.bottom = rect.bottom * 2;
	}
	else if (nDDResolution == DDRES_WXGA800) {
		drect.top *= 2;
		drect.bottom *= 2;
		drect.left *= 2;
		drect.right *= 2;
	}
	hResult = lpdds[0]->Blt(&drect, lpdds[1], &rect, DDBLT_WAIT, NULL);
	if (hResult != DD_OK) {
		/* �T�[�t�F�C�X�����X�g���Ă���΁A���X�g�A */
		if (hResult == DDERR_SURFACELOST) {
			lpdds[0]->Restore();
			lpdds[1]->Restore();
		}
		/* ����͑S�̈�X�V */
		ReDrawDD();
		return;
	}

	/* ����ɔ����A���[�N���Z�b�g */
	nDrawTop = 400;
	nDrawBottom = 0;
	nDrawLeft = 640;
	nDrawRight = 0;
	SetDrawFlag(FALSE);
}
#endif

/*
 *	�`��`�F�b�N
 */
#if XM7_VER >= 2 || (XM7_VER == 1 && defined(L4CARD))
static void FASTCALL DrawDDCheck(void)
{
#if XM7_VER >= 3
	/* ����Ȃ��蔲��(�H */
	if (nMode != screen_mode) {
		ReDrawDD();
		nMode = screen_mode;
	}
#elif XM7_VER >= 2
	/* 320x200 */
	if (mode320) {
		if (!bAnalog) {
			ReDrawDD();
			bAnalog = TRUE;
		}
		return;
	}

	/* 640x200 */
	if (bAnalog) {
		ReDrawDD();
		bAnalog = FALSE;
	}
#elif XM7_VER == 1 && defined(L4CARD)
	/* 640x400 */
	if (enable_400line) {
		if (!b400Line) {
			ReDrawDD();
			b400Line = TRUE;
		}
		return;
	}

	/* 640x200 */
	if (b400Line) {
		ReDrawDD();
		b400Line = FALSE;
	}
#endif
}
#endif

/*
 *	�`��
 */
void FASTCALL DrawDD(void)
{
	/* ���� */
#if XM7_VER >= 2 || (XM7_VER == 1 && defined(L4CARD))
	DrawDDCheck();
#endif

	/* �`�� */
#if XM7_VER >= 3
	switch (nMode) {
		case SCR_400LINE	:	Draw400l();
								return;
		case SCR_262144		:	Draw256k();
								return;
		case SCR_4096		:	Draw320();
								return;
		case SCR_200LINE	:	Draw640();
								return;
	}
#elif XM7_VER >= 2
	if (bAnalog) {
		Draw320();
		return;
	}
#elif XM7_VER == 1 && defined(L4CARD)
	if (b400Line) {
		DrawL4();
		return;
	}
#endif
	Draw640();
}

/*
 *	640x200�A�f�W�^�����[�h
 *	���X�^�`��
 */
static void FASTCALL DrawRaster640(int nRaster)
{
	DDSURFACEDESC ddsd;
	HRESULT hResult;
	BYTE *p;

	/* �\���͈͊O�̏ꍇ�͉������Ȃ� */
	if (nRaster >= 200) {
		return;
	}

	/* �p���b�g�ݒ� */
	Palet640();

	/* �T�[�t�F�C�X�����b�N */
	memset(&ddsd, 0, sizeof(ddsd));
	ddsd.dwSize = sizeof(ddsd);
	hResult = lpdds[1]->Lock(NULL, &ddsd,
							 DDLOCK_WAIT | DDLOCK_SURFACEMEMORYPTR, NULL);
	if (hResult != DD_OK) {
		/* �T�[�t�F�C�X�����X�g���Ă���΁A���X�g�A */
		if (hResult == DDERR_SURFACELOST) {
			lpdds[1]->Restore();
		}
		/* ����͑S�̈�X�V */
		ReDrawDD();
		return;
	}

	/* �����_�����O */
	p = (BYTE *)ddsd.lpSurface;
	if (nDDResolution == DDRES_480LINE) {
		p += (ddsd.lPitch * 40);
	}
	else if (nDDResolution == DDRES_SXGA) {
		p += (ddsd.lPitch * 56);
	}

#if XM7_VER >= 3
	/* �E�B���h�E�I�[�v���� */
	if (window_open) {
		/* �E�B���h�E�O�̕`�� */
		if ((nRaster < window_dy1) || (nRaster > window_dy2)) {
			if (bPseudo400Line) {
				Render640c(p, ddsd.lPitch, nRaster, nRaster + 1);
			}
			else {
				Render640(p, ddsd.lPitch, nRaster, nRaster + 1, 2);
			}
		}
		else {
			if (bPseudo400Line) {
				Render640cw(p, ddsd.lPitch,
					nRaster, nRaster + 1, window_dx1, window_dx2);
			}
			else {
				Render640w(p, ddsd.lPitch,
					nRaster, nRaster + 1, window_dx1, window_dx2, 2);
			}
		}
	}
	else {
		if (bPseudo400Line) {
			Render640c(p, ddsd.lPitch, nRaster, nRaster + 1);
		}
		else {
			Render640(p, ddsd.lPitch, nRaster, nRaster + 1, 2);
		}
	}
#elif XM7_VER == 2
	if (bPseudo400Line) {
		Render640c(p, ddsd.lPitch, nRaster, nRaster + 1);
	}
	else {
		Render640(p, ddsd.lPitch, nRaster, nRaster + 1);
	}
#else
	if (bPseudo400Line) {
		Render640m(p, ddsd.lPitch, nRaster, nRaster + 1);
	}
	else {
		Render640(p, ddsd.lPitch, nRaster, nRaster + 1);
	}
#endif

	/* �T�[�t�F�C�X���A�����b�N */
	lpdds[1]->Unlock(ddsd.lpSurface);
}

#if XM7_VER >= 3
/*
 *	640x400�A�f�W�^�����[�h
 *	���X�^�`��
 */
static void FASTCALL DrawRaster400l(int nRaster)
{
	DDSURFACEDESC ddsd;
	HRESULT hResult;
	BYTE *p;

	/* �\���͈͊O�̏ꍇ�͉������Ȃ� */
	if (nRaster >= 400) {
		return;
	}

	/* �p���b�g�ݒ� */
	Palet640();

	/* �T�[�t�F�C�X�����b�N */
	memset(&ddsd, 0, sizeof(ddsd));
	ddsd.dwSize = sizeof(ddsd);
	hResult = lpdds[1]->Lock(NULL, &ddsd,
							 DDLOCK_WAIT | DDLOCK_SURFACEMEMORYPTR, NULL);
	if (hResult != DD_OK) {
		/* �T�[�t�F�C�X�����X�g���Ă���΁A���X�g�A */
		if (hResult == DDERR_SURFACELOST) {
			lpdds[1]->Restore();
		}
		/* ����͑S�̈�X�V */
		ReDrawDD();
		return;
	}

	/* �����_�����O */
	p = (BYTE *)ddsd.lpSurface;
	if (nDDResolution == DDRES_480LINE) {
		p += (ddsd.lPitch * 40);
	}
	else if (nDDResolution == DDRES_SXGA) {
		p += (ddsd.lPitch * 56);
	}

	/* �E�B���h�E�I�[�v���� */
	if (window_open) {
		/* �E�B���h�E�O�̕`�� */
		if ((nRaster < window_dy1) || (nRaster > window_dy2)) {
			Render640(p, ddsd.lPitch, nRaster, nRaster + 1, 1);
		}
		else {
			Render640w(p, ddsd.lPitch, nRaster, nRaster + 1, window_dx1, window_dx2, 1);
		}
	}
	else {
		Render640(p, ddsd.lPitch, nRaster, nRaster + 1, 1);
	}

	/* �T�[�t�F�C�X���A�����b�N */
	lpdds[1]->Unlock(ddsd.lpSurface);
}
#endif

#if XM7_VER == 1 && defined(L4CARD)
/*
 *	�e�L�X�g���
 *	���X�^�`��
 */
static void FASTCALL RenderTextRasterDD(BYTE *p, DWORD pitch,
						int nRaster, int left, int right)
{
	DWORD	tmp;
	int		x, x2;
	WORD	addr;
	DWORD	col;
	BYTE	cursor_start, cursor_end, cursor_type;
	BYTE	raster, lines, maxx;
	BYTE	chr, atr, chr_dat;
	BYTE	line;
	int		xsize;

	/* �\��OFF�Ȃ牽�����Ȃ� */
	if (!crt_flag) {
		return;
	}

	/* cursor */
	cursor_start	= (BYTE)(crtc_register[10] & 0x1f);
	cursor_end		= (BYTE)(crtc_register[11] & 0x1f);
	cursor_type		= (BYTE)((crtc_register[10] & 0x60) >> 5);

	/* ��ʕ\����� */
	maxx			= (BYTE)(crtc_register[1] << 1);
	lines			= (BYTE)((crtc_register[9] & 0x1f) + 1);

	if (width40_flag) {
		xsize = 16;
	}
	else {
		xsize = 8;
	}

	for (x=left / xsize; x<right / xsize; x++) {
		raster = (BYTE)(nRaster % lines);
		line = (BYTE)(nRaster / lines);
		addr = (WORD)((text_start_addr + (line * maxx + x) * 2) & 0x0ffe);
		chr = tvram_c[addr + 0];
		atr = tvram_c[addr + 1];

		if (atr & 0x20) {
			/* Intensity attribute ON */
			if (bGreenMonitor) {
				if (nPixelFormat == 3) {
					col = Palet16Tc_L4[(atr & 0x07) + 24];
				}
				else if (nPixelFormat == 1) {
					col = Palet16_L4[(atr & 0x07) + 40];
				}
				else {
					col = Palet16_L4[(atr & 0x07) + 56];
				}
			}
			else {
				if (nPixelFormat == 3) {
					col = Palet16Tc_L4[(atr & 0x07) + 8];
				}
				else if (nPixelFormat == 1) {
					col = Palet16_L4[(atr & 0x07) + 8];
				}
				else {
					col = Palet16_L4[(atr & 0x07) + 24];
				}
			}
		}
		else {
			/* Intensity attribute OFF */
			if (bGreenMonitor) {
				if (nPixelFormat == 3) {
					col = Palet16Tc_L4[(atr & 0x07) + 16];
				}
				else if (nPixelFormat == 1) {
					col = Palet16_L4[(atr & 0x07) + 32];
				}
				else {
					col = Palet16_L4[(atr & 0x07) + 48];
				}
			}
			else {
				if (nPixelFormat == 3) {
					col = Palet16Tc_L4[(atr & 0x07) + 0];
				}
				else if (nPixelFormat == 1) {
					col = Palet16_L4[(atr & 0x07) + 0];
				}
				else {
					col = Palet16_L4[(atr & 0x07) + 16];
				}
			}
		}

		/* Blink attribute */
		if ((!(atr & 0x10) || text_blink) && (raster < 16)) {
			chr_dat = subcg_l4[(WORD)(chr * 16) + raster];
		}
		else {
			chr_dat = 0x00;
		}

		/* Reverse attribute */
		if (atr & 0x08) {
			chr_dat = (BYTE)~chr_dat;
		}

		/* �ق�ł����āA���[���� */
		if ((addr == cursor_addr) && (cursor_type != 1) &&
			(cursor_blink || (cursor_type == 0))) {
			if ((raster >= cursor_start) && (raster <= cursor_end)) {
				chr_dat = (BYTE)~chr_dat;
			}
		}

		if (chr_dat) {
			for (x2=0; x2<8; x2++) {
				if (chr_dat & (1 << (7-x2))) {
					if (nPixelFormat == 3) {
						if (width40_flag) {
							tmp = nRaster * pitch + (((x << 3) + x2) << 3);
							*(DWORD *)(&p[tmp + 0]) = col;
							*(DWORD *)(&p[tmp + 4]) = col;
						}
						else {
							tmp = nRaster * pitch + (((x << 3) + x2) << 2);
							*(DWORD *)(&p[tmp + 0]) = col;
						}
					}
					else {
						if (width40_flag) {
							tmp = nRaster * pitch + (((x << 3) + x2) << 2);
							*(WORD *)(&p[tmp + 0]) = (WORD)col;
							*(WORD *)(&p[tmp + 2]) = (WORD)col;
						}
						else {
							tmp = nRaster * pitch + (((x << 3) + x2) << 1);
							*(WORD *)(&p[tmp + 0]) = (WORD)col;
						}
					}
				}
			}
		}
	}
}

/*
 *	640x400�A�P�F���[�h
 *	���X�^�`��
 */
static void FASTCALL DrawRasterL4(int nRaster)
{
	DDSURFACEDESC ddsd;
	HRESULT hResult;
	BYTE *p;

	/* �\���͈͊O�̏ꍇ�͉������Ȃ� */
	if (nRaster >= 400) {
		return;
	}

	/* �p���b�g�ݒ� */
	PaletL4();

	/* �T�[�t�F�C�X�����b�N */
	memset(&ddsd, 0, sizeof(ddsd));
	ddsd.dwSize = sizeof(ddsd);
	hResult = lpdds[1]->Lock(NULL, &ddsd,
							 DDLOCK_WAIT | DDLOCK_SURFACEMEMORYPTR, NULL);
	if (hResult != DD_OK) {
		/* �T�[�t�F�C�X�����X�g���Ă���΁A���X�g�A */
		if (hResult == DDERR_SURFACELOST) {
			lpdds[1]->Restore();
		}
		/* ����͑S�̈�X�V */
		ReDrawDD();
		return;
	}

	/* �����_�����O */
	p = (BYTE *)ddsd.lpSurface;
	if (nDDResolution == DDRES_480LINE) {
		p += (ddsd.lPitch * 40);
	}
	else if (nDDResolution == DDRES_SXGA) {
		p += (ddsd.lPitch * 56);
	}

	RenderL4(p, ddsd.lPitch, nRaster, nRaster + 1);
	RenderTextRasterDD(p, ddsd.lPitch, nRaster, 0, 640);

	/* �T�[�t�F�C�X���A�����b�N */
	lpdds[1]->Unlock(ddsd.lpSurface);
}
#endif

#if XM7_VER >= 2
/*
 *	320x200�A�A�i���O���[�h
 *	���X�^�`��
 */
static void FASTCALL DrawRaster320(int nRaster)
{
	DDSURFACEDESC ddsd;
	HRESULT hResult;
	BYTE *p;

	/* �\���͈͊O�̏ꍇ�͉������Ȃ� */
	if (nRaster >= 200) {
		return;
	}

	/* �p���b�g�ݒ� */
	Palet320();

	/* �T�[�t�F�C�X�����b�N */
	memset(&ddsd, 0, sizeof(ddsd));
	ddsd.dwSize = sizeof(ddsd);
	hResult = lpdds[1]->Lock(NULL, &ddsd,
							 DDLOCK_WAIT | DDLOCK_SURFACEMEMORYPTR, NULL);
	if (hResult != DD_OK) {
		/* �T�[�t�F�C�X�����X�g���Ă���΁A���X�g�A */
		if (hResult == DDERR_SURFACELOST) {
			lpdds[1]->Restore();
		}
		/* ����͑S�̈�X�V */
		ReDrawDD();
		return;
	}

	/* �����_�����O */
	p = (BYTE *)ddsd.lpSurface;
	if (nDDResolution == DDRES_480LINE) {
		p += (ddsd.lPitch * 40);
	}
	else if (nDDResolution == DDRES_SXGA) {
		p += (ddsd.lPitch * 56);
	}

#if XM7_VER >= 3
	/* �E�B���h�E�I�[�v���� */
	if (window_open) {
		/* �E�B���h�E�O�̕`�� */
		if ((nRaster < window_dy1) || (nRaster > window_dy2)) {
			Render320(p, ddsd.lPitch, nRaster, nRaster + 1);
		}
		else {
			Render320w(p, ddsd.lPitch, nRaster, nRaster + 1, window_dx1, window_dx2);
		}
	}
	else {
		Render320(p, ddsd.lPitch, nRaster, nRaster + 1);
	}
#else
	Render320(p, ddsd.lPitch, nRaster, nRaster + 1);
#endif

	/* �T�[�t�F�C�X���A�����b�N */
	lpdds[1]->Unlock(ddsd.lpSurface);
}
#endif

#if XM7_VER >= 3
/*
 *	320x200�A26���F���[�h
 *	���X�^�`��
 */
static void FASTCALL DrawRaster256k(int nRaster)
{
	DDSURFACEDESC ddsd;
	HRESULT hResult;
	BYTE *p;

	/* �\���͈͊O�̏ꍇ�͉������Ȃ� */
	if (nRaster >= 200) {
		return;
	}

	/* �T�[�t�F�C�X�����b�N */
	memset(&ddsd, 0, sizeof(ddsd));
	ddsd.dwSize = sizeof(ddsd);
	hResult = lpdds[1]->Lock(NULL, &ddsd,
							 DDLOCK_WAIT | DDLOCK_SURFACEMEMORYPTR, NULL);
	if (hResult != DD_OK) {
		/* �T�[�t�F�C�X�����X�g���Ă���΁A���X�g�A */
		if (hResult == DDERR_SURFACELOST) {
			lpdds[1]->Restore();
		}
		/* ����͑S�̈�X�V */
		ReDrawDD();
		return;
	}

	/* �����_�����O */
	p = (BYTE *)ddsd.lpSurface;
	if (nDDResolution == DDRES_480LINE) {
		p += (ddsd.lPitch * 40);
	}
	else if (nDDResolution == DDRES_SXGA) {
		p += (ddsd.lPitch * 56);
	}

	if (crt_flag) {
		Render256k(p, ddsd.lPitch, nRaster, nRaster + 1, multi_page);
	}
	else {
		Render256k(p, ddsd.lPitch, nRaster, nRaster + 1, 0xff);
	}

	/* �T�[�t�F�C�X���A�����b�N */
	lpdds[1]->Unlock(ddsd.lpSurface);
}
#endif

/*
 *	���X�^�P�ʃ����_�����O
 *	�`��
 */
void FASTCALL DrawPostRenderDD(void)
{
	DDSURFACEDESC ddsd;
	HRESULT hResult;
	RECT rect;
	RECT drect;
	BYTE *p;

	/* �I�[���N���A */
	AllClear(FALSE);

	/* �X�e�[�^�X���C�� */
	StatusLine();

	/* �T�[�t�F�C�X�����b�N */
	memset(&ddsd, 0, sizeof(ddsd));
	ddsd.dwSize = sizeof(ddsd);
	hResult = lpdds[1]->Lock(NULL, &ddsd,
							 DDLOCK_WAIT | DDLOCK_SURFACEMEMORYPTR, NULL);
	if (hResult != DD_OK) {
		/* �T�[�t�F�C�X�����X�g���Ă���΁A���X�g�A */
		if (hResult == DDERR_SURFACELOST) {
			lpdds[1]->Restore();
		}
		/* ����͑S�̈�X�V */
		ReDrawDD();
		return;
	}

	/* �����_�����O�㏈�� */
	p = (BYTE *)ddsd.lpSurface;
	if (nDDResolution == DDRES_480LINE) {
		p += (ddsd.lPitch * 40);
	}
	else if (nDDResolution == DDRES_SXGA) {
		p += (ddsd.lPitch * 56);
	}

#if XM7_VER >= 3
	if ((nMode != SCR_400LINE) && !bPseudo400Line) {
		RenderFullScan(p, ddsd.lPitch);
	}
#elif XM7_VER == 2 && defined(FMTV151)
	DrawV2_DD(p, ddsd.lPitch);
	if (!bPseudo400Line) {
		RenderFullScan(p, ddsd.lPitch);
	}
#elif XM7_VER == 1
#if defined(L4CARD)
	if (!b400Line && !bPseudo400Line) {
#else
	if (!bPseudo400Line) {
#endif
		RenderFullScan(p, ddsd.lPitch);
	}
#else
	if (!bPseudo400Line) {
		RenderFullScan(p, ddsd.lPitch);
	}
#endif

	/* �T�[�t�F�C�X���A�����b�N */
	lpdds[1]->Unlock(ddsd.lpSurface);

	/* Blt */
	rect.left = 0;
	rect.right = 640;

	/* ���X�^�P�ʃ����_�����O���ɂЂ�����Ԃ邱�Ƃ�����̂� */
	if (nDrawTop >= nDrawBottom) {
		rect.top = 0;
		rect.bottom = 400;
	}
	else {
		rect.top = nDrawTop;
		rect.bottom = nDrawBottom;
	}

	drect = rect;

	if (nDDResolution == DDRES_480LINE) {
		rect.top += 40;
		rect.bottom += 40;
		drect.top += 40;
		drect.bottom += 40;
	}
	else if (nDDResolution == DDRES_WUXGA) {
		drect.top *= 3;
		drect.bottom *= 3;
		drect.left *= 3;
		drect.right *= 3;
	}
	else if (nDDResolution == DDRES_SXGA) {
		rect.top += 56;
		rect.bottom += 56;
		drect.left *= 2;
		drect.right *= 2;
		drect.top = rect.top * 2;
		drect.bottom = rect.bottom * 2;
	}
	else if (nDDResolution == DDRES_WXGA800) {
		drect.top *= 2;
		drect.bottom *= 2;
		drect.left *= 2;
		drect.right *= 2;
	}
	hResult = lpdds[0]->Blt(&drect, lpdds[1], &rect, DDBLT_WAIT, NULL);
	if (hResult != DD_OK) {
		/* �T�[�t�F�C�X�����X�g���Ă���΁A���X�g�A */
		if (hResult == DDERR_SURFACELOST) {
			lpdds[0]->Restore();
			lpdds[1]->Restore();
		}
		/* ����͑S�̈�X�V */
		ReDrawDD();
		return;
	}

	/* ����ɔ����A���[�N���Z�b�g */
	nDrawTop = 400;
	nDrawBottom = 0;
	nDrawLeft = 640;
	nDrawRight = 0;
	SetDrawFlag(TRUE);
}

/*
 *	���X�^�P�ʃ����_�����O
 *	�e���X�^�����_�����O
 */
void FASTCALL DrawRasterDD(int nRaster)
{
	/* ���� */
#if XM7_VER >= 2 || (XM7_VER == 1 && defined(L4CARD))
	DrawDDCheck();
#endif

#if XM7_VER >= 3
	/* �����ꂩ���g���ĕ`�� */
	switch (nMode) {
		case SCR_400LINE	:	DrawRaster400l(nRaster);
								return;
		case SCR_262144		:	DrawRaster256k(nRaster);
								return;
		case SCR_4096		:	DrawRaster320(nRaster);
								return;
		case SCR_200LINE	:	DrawRaster640(nRaster);
								return;
	}
#elif XM7_VER >= 2
	/* �ǂ��炩���g���ĕ`�� */
	if (bAnalog) {
		DrawRaster320(nRaster);
	}
	else {
		DrawRaster640(nRaster);
	}
#elif defined(L4CARD)
	/* �ǂ��炩���g���ĕ`�� */
	if (b400Line) {
		DrawRasterL4(nRaster);
	}
	else {
		DrawRaster640(nRaster);
	}
#else
	DrawRaster640(nRaster);
#endif
}

/*
 *	���j���[�J�n
 */
void FASTCALL EnterMenuDD(HWND hWnd)
{
	ASSERT(hWnd);

	/* �N���b�p�[�̗L���Ŕ���ł��� */
	if (!lpddc) {
		return;
	}

	/* �N���b�p�[�Z�b�g */
	LockVM();
	lpdds[0]->SetClipper(lpddc);
	UnlockVM();

	/* �}�E�X�J�[�\��on */
	if (!bMouseCursor) {
		ShowCursor(TRUE);
		bMouseCursor = TRUE;
	}

#if defined(MOUSE)
	/* �}�E�X�L���v�`����~ */
	SetMouseCapture(FALSE);
#endif

	/* ���j���[�o�[��`�� */
	if (bWin8flag) {
		/* Windows 8�ł͌��̃��C�����j���[�𕜌����� */
		if (hWnd == hMainWnd) {
			SetMenu(hMainWnd, hMainMenu);
		}
	}
	DrawMenuBar(hMainWnd);
}

/*
 *	���j���[�I��
 */
void FASTCALL ExitMenuDD(void)
{
	/* �N���b�p�[�̗L���Ŕ���ł��� */
	if (!lpddc) {
		return;
	}

	/* �N���b�p�[���� */
	LockVM();
	lpdds[0]->SetClipper(NULL);
	UnlockVM();

#if defined(MOUSE)
	/* �}�E�X�L���v�`���ݒ� */
	SetMouseCapture(TRUE);
#endif

	/* �}�E�X�J�[�\��OFF */
	if (bMouseCursor) {
		ShowCursor(FALSE);
		bMouseCursor = FALSE;
	}

	/* ���j���[�o�[������ */
	if (bWin8flag) {
		/* Windows 8�ł̓��C�����j���[���ꎞ�I�ɉ�������j���[�o�[��`�� */
		SetMenu(hMainWnd, NULL);
		DrawMenuBar(hMainWnd);
	}

	/* �ĕ\�� */
	ReDrawDD();
}

/*-[ VM�Ƃ̐ڑ� ]-----------------------------------------------------------*/

/*
 *	VRAM�Z�b�g
 */
void FASTCALL VramDD(WORD addr)
{
	WORD x;
	WORD y;

	/* y���W�Z�o */
#if XM7_VER >= 3
	switch (nMode) {
		case SCR_400LINE	:	addr &= 0x7fff;
								x = (WORD)((addr % 80) << 3);
								y = (WORD)(addr / 80);
								break;
		case SCR_262144		:
		case SCR_4096		:	addr &= 0x1fff;
								x = (WORD)((addr % 40) << 4);
								y = (WORD)((addr / 40) << 1);
								break;
		case SCR_200LINE	:	addr &= 0x3fff;
								x = (WORD)((addr % 80) << 3);
								y = (WORD)((addr / 80) << 1);
								break;
	}
#elif XM7_VER >= 2
	if (bAnalog) {
		addr &= 0x1fff;
		x = (WORD)((addr % 40) << 4);
		y = (WORD)((addr / 40) << 1);
	}
	else {
		addr &= 0x3fff;
		x = (WORD)((addr % 80) << 3);
		y = (WORD)((addr / 80) << 1);
	}
#elif defined(L4CARD)
	if (b400Line) {
		addr -= vram_offset[0];
		addr &= 0x7fff;
		x = (WORD)((addr % 80) << 3);
		y = (WORD)(addr / 80);
	}
	else {
		addr &= 0x3fff;
		x = (WORD)((addr % 80) << 3);
		y = (WORD)((addr / 80) << 1);
	}
#else
	addr &= 0x3fff;
	x = (WORD)((addr % 80) << 3);
	y = (WORD)((addr / 80) << 1);
#endif

	/* �I�[�o�[�`�F�b�N */
	if ((x >= 640) || (y >= 400)) {
		return;
	}

	/* �ĕ`��t���O��ݒ� */
	DDDrawFlag[(y >> 3) * 80 + (x >> 3)] = 1;

	/* ���������X�V */
	if (nDrawTop > y) {
		nDrawTop = y;
	}
	if (nDrawBottom <= y) {
#if XM7_VER >= 3
		if (nMode == SCR_400LINE) {
			nDrawBottom = (WORD)(y + 1);
		}
		else {
			nDrawBottom = (WORD)(y + 2);
		}
#else
		nDrawBottom = (WORD)(y + 2);
#endif
	}

	/* ���������X�V */
	if (nDrawLeft > x) {
		nDrawLeft = x;
	}
	if (nDrawRight <= x) {
#if XM7_VER >= 2
#if XM7_VER >= 3
		if (nMode & SCR_ANALOG) {
#else
		if (bAnalog) {
#endif
			nDrawRight = (WORD)(x + 16);
		}
		else {
			nDrawRight = (WORD)(x + 8);
		}
#else
		nDrawRight = (WORD)(x + 8);
#endif
	}
}

#if XM7_VER == 1 && defined(L4CARD)
/*
 *	�e�L�X�gVRAM�Z�b�g
 */
void FASTCALL TvramDD(WORD addr)
{
	WORD ysize;
	WORD x;
	WORD y;
	BYTE maxy;
	WORD yy;

	maxy = (BYTE)(crtc_register[6] & 0x7f);

	/* ���W�Z�o */
	addr = (WORD)((addr - text_start_addr) & 0x0ffe);
	ysize = (WORD)((crtc_register[9] & 0x1f) + 1);
	if (width40_flag) {
		x = (WORD)((addr % 80) * 8);
		y = (WORD)(addr / 80);
	}
	else {
		x = (WORD)((addr % 160) * 4);
		y = (WORD)(addr / 160);
	}

	/* �I�[�o�[�`�F�b�N */
	if (y >= maxy) {
		return;
	}
	y = (WORD)(y * ysize);
	if (y > 400 - ysize) {
		y = (WORD)(400 - ysize);
	}

	/* �ĕ`��t���O��ݒ� */
	for (yy = y; yy < (WORD)(y + ysize); yy += (WORD)8) {
		DDDrawFlag[(yy >> 3) * 80 + (x >> 3)] = 1;
		if (width40_flag) {
			DDDrawFlag[(yy >> 3) * 80 + (x >> 3) + 1] = 1;
		}
	}

	/* ���������X�V */
	if (nDrawTop > y) {
		nDrawTop = y;
	}
	if (nDrawBottom <= (y + ysize - 1)) {
		nDrawBottom = (WORD)(y + ysize);
	}

	/* ���������X�V */
	if (nDrawLeft > x) {
		nDrawLeft = x;
	}
	if (nDrawRight <= x) {
		if (width40_flag) {
			nDrawRight = (WORD)(x + 16);
		}
		else {
			nDrawRight = (WORD)(x + 8);
		}
	}
}

/*
 *	�e�L�X�gVRAM�ĕ`��v��
 */
void FASTCALL ReDrawTVRamDD(void)
{
	WORD maxy;

	maxy = (WORD)((crtc_register[6] & 0x7f) * ((crtc_register[9] & 0x1f) + 1));

	nDrawTop = 0;
	if (nDrawBottom < maxy) {
		nDrawBottom = maxy;
	}
	nDrawLeft = 0;
	nDrawRight = 640;
	SetDrawFlag(TRUE);
}
#endif

/*
 *	TTL�p���b�g�Z�b�g
 */
void FASTCALL DigitalDD(void)
{
	nDrawTop = 0;
	nDrawBottom = 400;
	nDrawLeft = 0;
	nDrawRight = 640;
	bPaletFlag = TRUE;
	SetDrawFlag(TRUE);
}

/*
 *	�A�i���O�p���b�g�Z�b�g
 */
void FASTCALL AnalogDD(void)
{
	nDrawTop = 0;
	nDrawBottom = 400;
	nDrawLeft = 0;
	nDrawRight = 640;
	bPaletFlag = TRUE;
	SetDrawFlag(TRUE);
}

/*
 *	�ĕ`��v��
 */
void FASTCALL ReDrawDD(void)
{
	/* �S�̈惌���_�����O */
	nDrawTop = 0;
	nDrawBottom = 400;
	nDrawLeft = 0;
	nDrawRight = 640;
	bPaletFlag = TRUE;
	bClearFlag = TRUE;
	SetDrawFlag(TRUE);
	SetDirtyFlag(0, 400, TRUE);
}

#if XM7_VER >= 3
/*
 *	�n�[�h�E�F�A�E�B���h�E�ʒm
 */
void FASTCALL WindowDD(void)
{
	WORD tmpLeft, tmpRight;
	WORD tmpTop, tmpBottom;
	WORD tmpDx1, tmpDx2;
	WORD tmpDy1, tmpDy2;
	BYTE *p;
	int i;

	/* 26���F���[�h���͉������Ȃ� */
	if (nMode == SCR_262144) {
		return;
	}

	/* �O�����ăN���b�s���O���� */
	window_clip(nMode);

	/* �E�B���h�E�T�C�Y��␳ */
	tmpDx1 = window_dx1;
	tmpDy1 = window_dy1;
	tmpDx2 = window_dx2;
	tmpDy2 = window_dy2;
	if (nMode != SCR_400LINE) {
		tmpDy1 <<= 1;
		tmpDy2 <<= 1;
	}
	if (nMode == SCR_4096) {
		tmpDx1 <<= 1;
		tmpDx2 <<= 1;
	}

	if (bWindowOpen != window_open) {
		if (window_open) {
			/* �E�B���h�E���J�����ꍇ */
			tmpLeft = tmpDx1;
			tmpRight = tmpDx2;
			tmpTop = tmpDy1;
			tmpBottom = tmpDy2;
		}
		else {
			/* �E�B���h�E������ꍇ */
			tmpLeft = nWindowDx1;
			tmpRight = nWindowDx2;
			tmpTop = nWindowDy1;
			tmpBottom = nWindowDy2;
		}
	}
	else {
		if (window_open) {
			/* �X�V�̈�T�C�Y�����݂̂��̂ɐݒ� */
			tmpTop = nDrawTop;
			tmpBottom = nDrawBottom;
			tmpLeft = nDrawLeft;
			tmpRight = nDrawRight;

			/* ���W�ύX�`�F�b�N */
			if (!((nWindowDx1 == tmpDx1) &&
				  (nWindowDy1 == tmpDy1) &&
				  (nWindowDx2 == tmpDx2) &&
				  (nWindowDy2 == tmpDy2))) {
				/* ����X */
				if (nWindowDx1 < tmpDx1) {
					tmpLeft = nWindowDx1;
				}
				else {
					tmpLeft = tmpDx1;
				}

				/* �E��X */
				if (nWindowDx2 > tmpDx2) {
					tmpRight = nWindowDx2;
				}
				else {
					tmpRight = tmpDx2;
				}

				/* ����Y */
				if (nWindowDy1 < tmpDy1) {
					tmpTop = nWindowDy1;
				}
				else {
					tmpTop = tmpDy1;
				}

				/* �E��Y */
				if (nWindowDy2 > tmpDy2) {
					tmpBottom = nWindowDy2;
				}
				else {
					tmpBottom = tmpDy2;
				}
			}
		}
		else {
			/* �E�B���h�E���J���Ă��Ȃ��̂ŉ������Ȃ� */
			return;
		}
	}

	/* �����O�̍ĕ`��̈�Ɣ�r���čL����Η̈���X�V */
	if (tmpLeft < nDrawLeft) {
		nDrawLeft = tmpLeft;
	}
	if (tmpRight > nDrawRight) {
		nDrawRight = tmpRight;
	}
	if (tmpTop < nDrawTop) {
		nDrawTop = tmpTop;
	}
	if (tmpBottom > nDrawBottom) {
		nDrawBottom = tmpBottom;
	}

	/* �ĕ`��t���O���X�V */
	if ((nDrawLeft < nDrawRight) && (nDrawTop < nDrawBottom)) {
		if (nMode == SCR_400LINE) {
			SetDirtyFlag(nDrawTop, nDrawBottom + 1, TRUE);
		}
		else {
			SetDirtyFlag(nDrawTop >> 1, (nDrawBottom >> 1) + 1, TRUE);
		}
		p = &DDDrawFlag[(nDrawTop >> 3) * 80 + (nDrawLeft >> 3)];
		for (i = (nDrawTop >> 3); i < ((nDrawBottom + 7) >> 3) ; i++) {
			memset(p, 1, (nDrawRight - nDrawLeft) >> 3);
			p += 80;
		}
	}

	/* �E�B���h�E�I�[�v����Ԃ�ۑ� */
	bWindowOpen = window_open;
	nWindowDx1 = tmpDx1;
	nWindowDy1 = tmpDy1;
	nWindowDx2 = tmpDx2;
	nWindowDy2 = tmpDy2;
}
#endif

#endif	/* _WIN32 */
