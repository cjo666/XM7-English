/*
 *	FM-7 EMULATOR "XM7"
 *
 *	Copyright (C) 1999-2017 �o�h�D(Twitter:@xm6_original)
 *	Copyright (C) 2001-2017 Ryu Takegami (Twitter:@RyuTakegami)
 *
 *	[ Win32API �T�u�E�B���h�E�P ]
 *
 *	RHG����
 *	  2002.03.06	�R���e�L�X�g���j���[����̃u���[�N�|�C���g�ݒ�@�\�����
 *					�������_���v�E�B���h�E�̃X�N���[���o�[���h���b�O�����Ƃ���
 *					$FF80�`$FFFF�̓��e���\���ł��Ȃ������C��
 *	  2002.03.10	�t�A�Z���u���E�B���h�E�̃X�N���[���o�[�������C��($FFEF�܂�)
 *	  2002.05.04	�t�A�Z���u���E�B���h�E�ɋ��|�̋����W�����v�@�\��ǉ�
 *					�t�A�Z���u���ƃ������_���v�̃R���e�L�X�g���j���[��Ɨ���
 *	  2002.05.23	�t�A�Z���u���E�B���h�E��PC�����ݒ��Ɨ�
 *	  2002.09.12	�t�A�Z���u���E�������_���v�E�B���h�E�̃T�C�Y�ύX�ɑΉ�
 *					�������_���v�E�B���h�E��DP�EI/O�̈�ւ̃W�����v�@�\��ǉ�
 *					�t�A�Z���u���E�B���h�E�̃X�N���[���o�[�������ďC��
 *	  2002.10.04	DrawWindowText�n�֐��ł̃��T�C�Y�������ꍇ�̈��S��������
 *	  2003.01.10	�������_���v�E�B���h�E��ASCII�L�����N�^�\���E�`�F�b�N�T��
 *					��ǉ��E�E�B���h�E�̃��T�C�Y����������
 *	  2003.01.26	�������_���v�E�B���h�E�Ɋ����\���E���������e�ύX�@�\��ǉ�
 *					CPU���W�X�^�E�B���h�E�ɃR���e�L�X�g���j���[�E���W�X�^���e
 *					�ύX�@�\��ǉ�
 *	  2003.01.27	�������_���v�E�B���h�E�Ƀ����������@�\��ǉ�
 *					�t�A�Z���u���E�������_���v�E�B���h�E�ɃX�^�b�N�W�����v�E
 *					�u���[�N�|�C���g�W�����v�@�\��ǉ�
 *	  2003.03.21	TileWindows/CascadeWindows���̃T�C�Y�ύX�������C��
 *	  2003.05.09	CPU���W�X�^�E�t�A�Z���u���E�������_���v�E�B���h�E�̑Ώ�
 *					CPU�������������
 *					�񊿎����[�hASCII�_���v�ł�0x80�̕\�����C��
 *	  2003.07.17	���������e�ύX��Ƀ������_���v�E�B���h�E�̓��e���X�V����
 *					�Ȃ������C��
 *	  2003.11.21	3�ȏ��CPU(���{��ʐM�J�[�h�Ƃ��X�e���I�~���[�W�b�N�{�b
 *					�N�X�Ƃ�)�ɑΉ��ł���悤�ɏ��������܂���(��
 *	  2004.09.18	�������_���v�E�C���h�E�̃y�[�W�A�b�v/�_�E�����ɃE�C���h�E
 *					�T�C�Y�ɍ��킹�ăA�b�v/�_�E������ω�������悤�ɕύX
 *	  2012.03.06	�X�P�W���[���E�B���h�E�̕�����`�揈�����ɃC�x���g���I��
 *					�����ASSERT�Ɉ�������������C��
 *	  2015.03.12	�������_���v�E�C���h�E�̕����A�h���X�Ή���
 *	  2015.03.13	�T�u�E�C���h�E�̃|�b�v�A�b�v�Ή�
 *	  2015.11.13	�A�h���X�q�X�g���𕨗��A�h���X/�_���A�h���X�ʂɎ��悤�ύX
 *	  2016.09.04	Windows10�ŋN�������CPU���W�X�^�E�C���h�E���J�����Ƃ����
 *					����������C��
 *					CPU���W�X�^�E�C���h�E�̃R���e�L�X�g���j���[��D���W�X�^�̓�
 *					�e�ύX���ł��Ȃ������C��
 */

#ifdef _WIN32

#define STRICT
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include <mmsystem.h>
#include <assert.h>
#include <stdlib.h>
#include "xm7.h"
#include "event.h"
#include "mmr.h"
#include "jsubsys.h"
#include "mouse.h"
#include "w32.h"
#include "w32_res.h"
#include "w32_sch.h"
#include "w32_sub.h"
#include "w32_kbd.h"

/*
 *	�O���[�o�� ���[�N
 */
HWND hSubWnd[SWND_MAXNUM];				/* �T�u�E�C���h�E */
BOOL bShowSubWindow[SWND_MAXNUM];		/* �T�u�E�B���h�E�\����� */
BOOL bPopupSwnd = TRUE;					/* �T�u�E�B���h�E�|�b�v�A�b�v��� */

/*
 *	�X�^�e�B�b�N ���[�N
 */
static DWORD AddrHistoryPhys[16];		/* �����A�h���X �A�h���X�q�X�g�� */
static DWORD AddrBufPhys;				/* �����A�h���X �A�h���X�o�b�t�@ */
static int AddrNumPhys;					/* �����A�h���X �A�h���X�q�X�g���� */
static BYTE nAddrDlgCPU;				/* �A�h���X�ݒ�Ώ�CPU */
static BOOL bAddrDlgLinearAddr;			/* �A�h���X�ݒ�Ώۃ�������� */
static WORD AddrHistoryLogi[16];		/* �_���A�h���X �A�h���X�q�X�g�� */
static WORD AddrBufLogi;				/* �_���A�h���X �A�h���X�o�b�t�@ */
static int AddrNumLogi;					/* �_���A�h���X �A�h���X�q�X�g����*/

static BYTE nBreakCPU;					/* �u���[�N�|�C���g �ݒ�Ώ�CPU */
static BYTE *pBreakPoint;				/* �u���[�N�|�C���g Draw�o�b�t�@ */
static HMENU hBreakPoint;				/* �u���[�N�|�C���g ���j���[�n���h�� */
static POINT PosBreakPoint;				/* �u���[�N�|�C���g �}�E�X�ʒu */
static BYTE *pScheduler;				/* �X�P�W���[�� Draw�o�b�t�@ */
static DWORD dwSchedulerTime[2];		/* �X�P�W���[�� ������ */
static DWORD dwSchedulerFrame[2];		/* �X�P�W���[�� �t���[���� */
static BYTE *pCPURegister[MAXCPU];		/* CPU���W�X�^ Draw�o�b�t�@ */
static HMENU hCPURegister[MAXCPU];		/* CPU���W�X�^ ���j���[�n���h�� */
static BYTE *pDisAsm[MAXCPU];			/* �t�A�Z���u�� Draw�o�b�t�@ */
static DWORD dwDisAsm[MAXCPU];			/* �t�A�Z���u�� �A�h���X */
static HMENU hDisAsm[MAXCPU];			/* �t�A�Z���u�� ���j���[�n���h�� */
static POINT PosDisAsmPoint;			/* �t�A�Z���u�� �}�E�X�ʒu */
static BYTE nHeightDisAsm[MAXCPU];		/* �t�A�Z���u�� �E�B���h�E�c�T�C�Y */
static BOOL bDisPhysicalAddr;			/* �t�A�Z���u�� �����A�h���X���[�h */
static BYTE *pMemory[MAXCPU];			/* �������_���v Draw�o�b�t�@ */
static DWORD dwMemory[MAXCPU];			/* �������_���v �A�h���X */
static HMENU hMemory[MAXCPU];			/* �������_���v ���j���[�n���h�� */
static BYTE nHeightMemory[MAXCPU];		/* �������_���v �E�B���h�E�c�T�C�Y */
static BOOL bAsciiDump[MAXCPU];			/* �������_���v ASCII�\�� */
static BOOL bKanjiDump[MAXCPU];			/* �������_���v �����\�� */
static BOOL bDumpPhysicalAddr;			/* �������_���v �����A�h���X���[�h */
#if XM7_VER >= 3
static BOOL bExtendMMRMode;				/* �������_���v �g��MMR���[�h */
#endif
#if XM7_VER >= 2
static int nFM7Ver;						/* �������_���v �@���ʕۑ��̈� */
#else
static int nFMsubtype;					/* �������_���v �@���ʕۑ��̈� */
#endif
static BYTE nMemDlgCPU;					/* �������ύX �ύX�Ώ�CPU */
static DWORD dwMemDlgAddr;				/* �������ύX �A�h���X */
static WORD nMemDlgByte;				/* �������ύX �A�h���X���e */
static BYTE nRegDlgCPU;					/* ���W�X�^�ύX �ύX�Ώ�CPU */
static BYTE nRegDlgNo;					/* ���W�X�^�ύX �ύX�Ώۃ��W�X�^ */
static BYTE nMemSrchCPU;				/* ���������� �����Ώ�CPU */
static DWORD dwMemSrchAddr;				/* ���������� �����J�n�A�h���X */
static DWORD dwMemSrchAddrSave[MAXCPU];	/* ���������� �����J�n�A�h���X main */
static char szMemSrchString[128];		/* ���������� �����Ώۃf�[�^ */
static WORD wStackAddr[16];				/* �V�X�e���X�^�b�N�W�����v�A�h���X */
static WORD wBreakAddr[16];				/* �u���[�N�|�C���g�W�����v�A�h���X */

/*
 *	�v���g�^�C�v�錾
 */
static void FASTCALL PaintMemory(HWND hWnd);

/*
 *	�V�t�gJIS����}�N��+����ӂ�
 */
#define isKanji(p)	((BYTE)((p ^ 0x20) - 0xa1) <= 0x3b)
#define isKanji2(p)	(((BYTE)(p + 3) >= 0x43) && (p != 0x7f))
#define isJapanese	(PRIMARYLANGID(GetUserDefaultLangID()) == LANG_JAPANESE)

/*-[ �ėp�E�B���h�E���� ]----------------------------------------------------*/

/*
 *	�T�u�E�B���h�E
 *	�T�C�Y�ύX
 */
static void FASTCALL WindowSizing(HWND hWnd, LPRECT rect, BYTE **buf)
{
	RECT wrect;
	POINT tpoint;
	POINT bpoint;
	int tframe;
	int bframe;
	int x;
	int y;
	BYTE *p;

	/* �t���[���E�^�C�g���o�[�̃T�C�Y���擾 */
	bframe = GetSystemMetrics(SM_CYSIZEFRAME);
	tframe = GetSystemMetrics(SM_CYCAPTION) + bframe;

	/* �V�N���C�A���g�̈���v�Z */
	tpoint.x = 0;
	tpoint.y = rect->top + tframe;
	bpoint.x = 0;
	bpoint.y = rect->bottom - bframe;
	ScreenToClient(hWnd, &tpoint);
	ScreenToClient(hWnd, &bpoint);

	/* �N���C�A���g�̈�𕶎����̔{���ɍ��킹�� */
	tpoint.y = (tpoint.y / lCharHeight) * lCharHeight;
	bpoint.y = (bpoint.y / lCharHeight) * lCharHeight;

	/* �X�N���[�����W�֕ϊ� */
	ClientToScreen(hWnd, &tpoint);
	ClientToScreen(hWnd, &bpoint);

	/* �E�B���h�E�T�C�Y�𒲐� */
	GetWindowRect(hWnd, &wrect);
	rect->left = wrect.left;
	rect->top = tpoint.y;
	rect->right = wrect.right;
	rect->bottom = bpoint.y;
	x = rect->right / lCharWidth;
	y = rect->bottom / lCharHeight;

	/* �c�����̂ݒ���(��������AdjustWindowRect��Ɍ��̒l����) */
	if (bPopupSwnd) {
		AdjustWindowRect(rect,	WS_POPUP | WS_OVERLAPPED | WS_SYSMENU |
								WS_CAPTION | WS_VISIBLE | 
								WS_MINIMIZEBOX | WS_SIZEBOX, FALSE);
	}
	else {
		AdjustWindowRect(rect,	WS_CHILD | WS_OVERLAPPED | WS_SYSMENU |
								WS_CAPTION | WS_VISIBLE | 
								WS_MINIMIZEBOX | WS_CLIPSIBLINGS | 
								WS_SIZEBOX, FALSE);
	}
	rect->left = wrect.left;
	rect->right = wrect.right;

	/* �o�b�t�@���Ď擾���A0xFF�Ŗ��߂� */
	p = realloc(*buf, 2 * x * y);
	ASSERT(p);
	*buf = p;
	memset(p, 0xff, 2 * x * y);
}

/*
 *	�T�u�E�B���h�E
 *	�e�L�X�g�`��
 */
void FASTCALL DrawWindowText(HDC hDC, BYTE *ptr, int x, int y)
{
	BYTE *p;
	BYTE *q;
	int xx, yy;
	HFONT hBackup;

	/* �t�H���g�Z���N�g */
	hBackup = SelectObject(hDC, hFont);

	/* ��r�`�� */
	p = ptr;
	q = &p[x * y];
	for (yy=0; yy<y; yy++) {
		for (xx=0; xx<x; xx++) {
			if (ptr) {
				if (*p != *q) {
					TextOut(hDC, xx * lCharWidth, yy * lCharHeight,
						(LPCTSTR)p, 1);
					*q = *p;
				}
				p++;
				q++;
			}
		}
	}

	/* �I�� */
	SelectObject(hDC, hBackup);
}

/*
 *	�T�u�E�B���h�E
 *	�e�L�X�g�`�� (�����\���Ή�)
 */
void FASTCALL DrawWindowTextKanji(HDC hDC, BYTE *ptr, int x, int y)
{
	BYTE *p;
	BYTE *q;
	int xx, yy;
	HFONT hBackup;

	/* �t�H���g�Z���N�g */
	hBackup = SelectObject(hDC, hFont);

	/* ��r�`�� */
	for (yy=0; yy<y; yy++) {
		for (xx=0; xx<x; xx++) {
			p = &ptr[x * yy + xx];
			q = &p[x * y];
			if (ptr) {
				if (isKanji(*p)) {
					if (*(WORD *)p != *(WORD *)q) {
						TextOut(hDC, xx * lCharWidth, yy * lCharHeight,
							(LPCTSTR)p, 2);
						*(WORD *)q = *(WORD *)p;
					}
					xx++;
				}
				else {
					if (*p != *q) {
						TextOut(hDC, xx * lCharWidth, yy * lCharHeight,
							(LPCTSTR)p, 1);
						*q = *p;
					}
				}
			}
		}
	}

	/* �I�� */
	SelectObject(hDC, hBackup);
}

/*
 *	�T�u�E�B���h�E
 *	�e�L�X�g�`��(���]�t��)
 */
void FASTCALL DrawWindowText2(HDC hDC, BYTE *ptr, int x, int y)
{
	BYTE *p;
	BYTE *q;
	int xx, yy;
	HFONT hBackup;
	COLORREF tcolor, bcolor;
	char dat;

	/* �t�H���g�Z���N�g */
	hBackup = SelectObject(hDC, hFont);

	/* ��r�`�� */
	p = ptr;
	q = &p[x * y];
	for (yy=0; yy<y; yy++) {
		for (xx=0; xx<x; xx++) {
			if (ptr) {
				if (*p != *q) {
					if (*p & 0x80) {
						/* ���]�\�� */
						dat = (char)(*p & 0x7f);
						bcolor = GetBkColor(hDC);
						tcolor = GetTextColor(hDC);
						SetTextColor(hDC, bcolor);
						SetBkColor(hDC, tcolor);
						TextOut(hDC, xx * lCharWidth, yy * lCharHeight, &dat, 1);
						SetTextColor(hDC, tcolor);
						SetBkColor(hDC, bcolor);
					}
					else {
						/* �ʏ�\�� */
						TextOut(hDC, xx * lCharWidth, yy * lCharHeight,
							(LPCTSTR)p, 1);
					}
					*q = *p;
				}
				p++;
				q++;
			}
		}
	}

	/* �I�� */
	SelectObject(hDC, hBackup);
}

/*
 *	�T�u�E�C���h�E
 *	WM_DESTROY����
 */
void FASTCALL DestroySubWindow(HWND hWnd, BYTE **pBuf, HMENU hmenu)
{
	int i;

	/* ���C���E�C���h�E�֎����ʒm */
	for (i=0; i<SWND_MAXNUM; i++) {
		if (hSubWnd[i] == hWnd) {
			hSubWnd[i] = NULL;

			/* ���j���[�폜 */
			if (hmenu) {
				DestroyMenu(hmenu);
			}

			/* Draw�o�b�t�@�폜 */
			if (pBuf && *pBuf) {
				free(*pBuf);
				*pBuf = NULL;
			}
		}
	}

	/* �h���[�E�C���h�E���ĕ`�� */
	InvalidateRect(hDrawWnd, NULL, FALSE);
}

/*
 *	�T�u�E�B���h�E
 *	�ʒu���Z�o
 */
void FASTCALL PositioningSubWindow(HWND hParent, LPRECT rect, int index)
{
	RECT wrect;

	/* �T�u�E�B���h�E�ʒu���Z�o */
	rect->left = lCharWidth * index;
	rect->top = lCharHeight * index;
	rect->right = 0;
	rect->bottom = 0;
	if (rect->top >= 380) {
		rect->top -= 380;
	}
	if (bPopupSwnd) {
		GetWindowRect(hParent, (LPRECT)&wrect);
		rect->left += wrect.left;
		rect->top += wrect.top;
	}
}

/*-[ �E�B���h�E���[�N������ ]------------------------------------------------*/

/*
 *	�T�u�E�B���h�E
 *	���[�N������
 */
void FASTCALL InitSubWndWork(void)
{
	int i;

	for (i=0; i<MAXCPU; i++) {
		nHeightDisAsm[i] = 8;			/* �t�A�Z���u�� �E�B���h�E�c�T�C�Y */
		nHeightMemory[i] = 8;			/* �������_���v �E�B���h�E�c�T�C�Y */
		bAsciiDump[i] = FALSE;			/* �������_���v ASCII�\�� */
		bKanjiDump[i] = FALSE;			/* �������_���v �����\�� */
		dwMemSrchAddrSave[i] = 0;		/* ���������� �����J�n�A�h���X */
	}
	bDisPhysicalAddr = FALSE;
	bDumpPhysicalAddr = FALSE;
}

/*-[ �����̂Ăʂ� ]----------------------------------------------------------*/

/*
 *	�������������[�h��Ԏ擾
 */
static BOOL FASTCALL isLinearAddrMode(BOOL bPhysicalAddr)
{
	if (fm7_ver <= 1) {
#if XM7_VER == 1
		if (fm_subtype != FMSUB_FM77) {
			return FALSE;
		}
#else
		return FALSE;
#endif
	}

	return bPhysicalAddr;
}

/*
 *	���������e�ύX�_�C�A���O�E���̑�
 *	�ő僁������Ԏ擾
 */
static DWORD FASTCALL GetMaxMemArea(BOOL bPhysicalAddr)
{
	if (fm7_ver <= 1) {
#if XM7_VER == 1
		if ((fm_subtype != FMSUB_FM77) || !bPhysicalAddr) {
			return 0x10000;
		}
		else {
			return 0x40000;
		}
#else
		return 0x10000;
#endif
	}
	else if ((fm7_ver <= 2) && bPhysicalAddr) {
		return 0x40000;
	}
#if XM7_VER >= 3
	else if (isLinearAddrMode(bPhysicalAddr)) {
		if (mmr_ext) {
			return 0x100000;
		}
		else {
			return 0x40000;
		}
	}
#endif

	return 0x10000;
}

/*-[ �A�h���X�q�X�g�� ]------------------------------------------------------*/

/*
 *	�A�h���X�q�X�g��
 *	�R���{�{�b�N�X�ւ̑}��
 */
static void FASTCALL InsertHistory(HWND hWnd, BOOL bIsLinearAddr)
{
	int i;
	char string[128];

	/* �q�X�g����}�� */
	(void)ComboBox_ResetContent(hWnd);
	if ((nAddrDlgCPU == MAINCPU) && isLinearAddrMode(bIsLinearAddr)) {
		for (i=AddrNumPhys; i>0; i--) {
			_snprintf(string, sizeof(string), "%05X",
			AddrHistoryPhys[i - 1] &
				(GetMaxMemArea(bIsLinearAddr) - 1));
			(void)ComboBox_AddString(hWnd, string);
		}
	}
	else {
		for (i=AddrNumLogi; i>0; i--) {
			_snprintf(string, sizeof(string), "%04X", AddrHistoryLogi[i - 1]);
			(void)ComboBox_AddString(hWnd, string);
		}
	}
}

/*
 *	�A�h���X�q�X�g��
 *	�ǉ�
 */
static void FASTCALL AddAddrHistory(DWORD addr, BOOL bIsLinearAddr)
{
	int i;

	/* ��d�o�^�`�F�b�N */
	if ((nAddrDlgCPU == MAINCPU) && isLinearAddrMode(bIsLinearAddr)) {
		for (i=0; i<AddrNumPhys; i++) {
			if (AddrHistoryPhys[i] == addr) {
				return;
			}
		}

		/* �q�X�g�����V�t�g�A�}���A�J�E���g�A�b�v */
		for (i=14; i>=0; i--) {
			AddrHistoryPhys[i + 1] = AddrHistoryPhys[i];
		}
		AddrHistoryPhys[0] = addr;
		if (AddrNumPhys < 16) {
			AddrNumPhys++;
		}
	}
	else {
		/* ��d�o�^�`�F�b�N */
		for (i=0; i<AddrNumLogi; i++) {
			if (AddrHistoryLogi[i] == (WORD)(addr & 0xffff)) {
				return;
			}
		}

		/* �q�X�g�����V�t�g�A�}���A�J�E���g�A�b�v */
		for (i=14; i>=0; i--) {
			AddrHistoryLogi[i + 1] = AddrHistoryLogi[i];
		}
		AddrHistoryLogi[0] = (WORD)(addr & 0xffff);
		if (AddrNumLogi < 16) {
			AddrNumLogi++;
		}
	}
}

/*-[ �A�h���X���̓_�C�A���O ]------------------------------------------------*/

/*
 *	�A�h���X���̓_�C�A���O
 *	�_�C�A���O������
 */
static BOOL FASTCALL AddrDlgInit(HWND hDlg)
{
	HWND hWnd;
	RECT prect;
	RECT drect;
	char string[128];

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

	/* �R���{�{�b�N�X���� */
	hWnd = GetDlgItem(hDlg, IDC_ADDRCOMBO);
	ASSERT(hWnd);
	InsertHistory(hWnd, bAddrDlgLinearAddr);

	/* �A�h���X��ݒ� */
	if ((nAddrDlgCPU == MAINCPU) && isLinearAddrMode(bAddrDlgLinearAddr)) {
		_snprintf(string, sizeof(string), "%05X",
			AddrBufPhys & (GetMaxMemArea(bAddrDlgLinearAddr)-1));
	}
	else {
		_snprintf(string, sizeof(string), "%04X", AddrBufLogi);
	}
	ComboBox_SetText(hWnd, string);

	return TRUE;
}

/*
 *	�A�h���X���̓_�C�A���O
 *	�_�C�A���OOK
 */
static void FASTCALL AddrDlgOK(HWND hDlg)
{
	DWORD addr;
	HWND hWnd;
	char string[128];

	ASSERT(hDlg);

	/* �R���{�{�b�N�X���� */
	hWnd = GetDlgItem(hDlg, IDC_ADDRCOMBO);
	ASSERT(hWnd);

	/* ���݂̒l���擾 */
	ComboBox_GetText(hWnd, string, sizeof(string) - 1);
	addr = (DWORD)strtol(string, NULL, 16);
	if ((nAddrDlgCPU == MAINCPU) && bAddrDlgLinearAddr) {
		AddrBufPhys = (addr & (GetMaxMemArea(bAddrDlgLinearAddr) - 1));
		AddAddrHistory(AddrBufPhys, bAddrDlgLinearAddr);
	}
	else {
		AddrBufLogi = (WORD)(addr & 0xffff);
		AddAddrHistory(AddrBufLogi, FALSE);
	}
}

/*
 *	�A�h���X���̓_�C�A���O
 *	�_�C�A���O�v���V�[�W��
 */
static BOOL CALLBACK AddrDlgProc(HWND hDlg, UINT iMsg,
									WPARAM wParam, LPARAM lParam)
{
	UNUSED(lParam);

	switch (iMsg) {
		/* �_�C�A���O������ */
		case WM_INITDIALOG:
			return AddrDlgInit(hDlg);

		/* �R�}���h���� */
		case WM_COMMAND:
			switch (LOWORD(wParam)) {
				/* OK�E�L�����Z�� */
				case IDOK:
				case IDCANCEL:
					if (LOWORD(wParam) == IDOK) {
						AddrDlgOK(hDlg);
					}
					EndDialog(hDlg, LOWORD(wParam));
					InvalidateRect(hDrawWnd, NULL, FALSE);
					return TRUE;
			}
			break;
	}

	/* ����ȊO�́AFALSE */
	return FALSE;
}

/*
 *	�A�h���X����
 */
static BOOL FASTCALL AddrDlg(HWND hWnd, DWORD *pAddr, BYTE nCPU,
							 BOOL bLinearAddr)
{
	int ret;

	ASSERT(hWnd);
	ASSERT(pAddr);

	/* �����A�h���X���[�h���Z�b�g */
	bAddrDlgLinearAddr = bLinearAddr;

	/* �A�h���X���Z�b�g */
#if XM7_VER == 1 && defined(Z80CARD)
	if (((nCPU == MAINCPU) || (nCPU == MAINZ80)) && bAddrDlgLinearAddr) {
#else
	if ((nCPU == MAINCPU) && bAddrDlgLinearAddr) {
#endif
		AddrBufPhys = *pAddr;
	}
	else {
		AddrBufLogi = *(WORD *)pAddr;
	}

	/* ���[�_���_�C�A���O���s */
	ret = DialogBox(hAppInstance, MAKEINTRESOURCE(IDD_ADDRDLG), hWnd, AddrDlgProc);
	if (ret != IDOK) {
		return FALSE;
	}

	/* �A�h���X���Z�b�g���A�A�� */
#if XM7_VER == 1 && defined(Z80CARD)
	if (((nCPU == MAINCPU) || (nCPU == MAINZ80)) && bAddrDlgLinearAddr) {
#else
	if ((nCPU == MAINCPU) && bAddrDlgLinearAddr) {
#endif
		*pAddr = AddrBufPhys;
	}
	else {
		*(WORD *)pAddr = AddrBufLogi;
	}

	return TRUE;
}

/*-[ �u���[�N�|�C���g�ݒ�_�C�A���O ]----------------------------------------*/

/*
 *	�u���[�N�|�C���g�ݒ�_�C�A���O
 *	�_�C�A���O������
 */
static BOOL FASTCALL BreakPointDlgInit(HWND hDlg)
{
	HWND hWnd;
	RECT prect;
	RECT drect;
	char string[128];

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

	/* �R���{�{�b�N�X���� */
	hWnd = GetDlgItem(hDlg, IDC_BREAKP_ADDRCOMBO);
	ASSERT(hWnd);
	InsertHistory(hWnd, FALSE);

	/* �A�h���X��ݒ� */
	_snprintf(string, sizeof(string), "%04X", AddrBufLogi);
	ComboBox_SetText(hWnd, string);

	/* �Ώ�CPU��ݒ� */
	switch (nBreakCPU) {
		case MAINCPU :	CheckDlgButton(hDlg, IDC_BREAKP_CPU_MAIN, BST_CHECKED);
						break;
		case SUBCPU :	CheckDlgButton(hDlg, IDC_BREAKP_CPU_SUB, BST_CHECKED);
						break;
#if XM7_VER == 1
#if defined(JSUB)
		case JSUBCPU :	CheckDlgButton(hDlg, IDC_BREAKP_CPU_JSUB, BST_CHECKED);
						break;
#endif
#if defined(Z80CARD)
		case MAINZ80 :	CheckDlgButton(hDlg, IDC_BREAKP_CPU_MAINZ80, BST_CHECKED);
						break;
#endif
#endif
	}

	return TRUE;
}

/*
 *	�u���[�N�|�C���g�ݒ�_�C�A���O
 *	�_�C�A���OOK
 */
static void FASTCALL BreakPointDlgOK(HWND hDlg)
{
	HWND hWnd;
	char string[128];

	ASSERT(hDlg);

	/* �R���{�{�b�N�X���� */
	hWnd = GetDlgItem(hDlg, IDC_BREAKP_ADDRCOMBO);
	ASSERT(hWnd);

	/* �Ώ�CPU���擾 */
	if (IsDlgButtonChecked(hDlg, IDC_BREAKP_CPU_MAIN) == BST_CHECKED) {
		nBreakCPU = MAINCPU;
	}
#if XM7_VER == 1
#if defined(JSUB)
	else if (IsDlgButtonChecked(hDlg, IDC_BREAKP_CPU_JSUB) == BST_CHECKED) {
		nBreakCPU = JSUBCPU;
	}
#endif
#if defined(Z80CARD)
	else if (IsDlgButtonChecked(hDlg, IDC_BREAKP_CPU_MAINZ80) == BST_CHECKED) {
		nBreakCPU = MAINZ80;
	}
#endif
#endif
	else {
		nBreakCPU = SUBCPU;
	}

	/* ���݂̒l���擾 */
	ComboBox_GetText(hWnd, string, sizeof(string) - 1);
	AddrBufLogi = (WORD)strtol(string, NULL, 16);

	/* �q�X�g���ǉ� */
	AddAddrHistory(AddrBufLogi, FALSE);
}

/*
 *	�u���[�N�|�C���g�ݒ�_�C�A���O
 *	�_�C�A���O�v���V�[�W��
 */
static BOOL CALLBACK BreakPointDlgProc(HWND hDlg, UINT iMsg,
									WPARAM wParam, LPARAM lParam)
{
	UNUSED(lParam);

	switch (iMsg) {
		/* �_�C�A���O������ */
		case WM_INITDIALOG:
			return BreakPointDlgInit(hDlg);

		/* �R�}���h���� */
		case WM_COMMAND:
			switch (LOWORD(wParam)) {
				/* OK�E�L�����Z�� */
				case IDOK:
				case IDCANCEL:
					if (LOWORD(wParam) == IDOK) {
						BreakPointDlgOK(hDlg);
					}
					EndDialog(hDlg, LOWORD(wParam));
					InvalidateRect(hDrawWnd, NULL, FALSE);
					return TRUE;
			}
			break;
	}

	/* ����ȊO�́AFALSE */
	return FALSE;
}

/*
 *	�u���[�N�|�C���g�ݒ�
 */
static BOOL FASTCALL BreakPointDlg(HWND hWnd, int num)
{
	int ret;

	ASSERT(hWnd);

	/* �A�h���X���Z�b�g */
	if (breakp[num].flag != BREAKP_NOTUSE) {
		AddrBufLogi = breakp[num].addr;
		nBreakCPU = (BYTE)breakp[num].cpu;
	}
	else {
		AddrBufLogi = 0;
		nBreakCPU = MAINCPU;
	}

	/* ���[�_���_�C�A���O���s */
	ret = DialogBox(hAppInstance, MAKEINTRESOURCE(IDD_BREAKPDLG), hWnd, BreakPointDlgProc);
	if (ret != IDOK) {
		return FALSE;
	}

	/* �A�h���X���Z�b�g���A�A�� */
	schedule_setbreak2(num, nBreakCPU, (WORD)AddrBufLogi);
	return TRUE;
}

/*-[ �u���[�N�|�C���g ]------------------------------------------------------*/

/*
 *	�u���[�N�|�C���g�E�C���h�E
 *	�Z�b�g�A�b�v
 */
static void FASTCALL SetupBreakPoint(BYTE *p, int x, int y)
{
	int i;
	char string[128];
	char tmp[128];

	ASSERT(p);
	ASSERT(x > 0);
	ASSERT(y > 0);

	/* VM�����b�N */
	LockVM();

	/* ��U�X�y�[�X�Ŗ��߂� */
	memset(p, 0x20, x * y);

	/* �u���[�N�|�C���g���[�v */
	for (i=0; i<BREAKP_MAXNUM; i++) {
		/* ������쐬 */
		string[0] = '\0';
		if (breakp[i].flag == BREAKP_NOTUSE) {
			_snprintf(string, sizeof(string), "%2d ------------------", i + 1);
		}
		else {
			switch (breakp[i].cpu) {
				case MAINCPU :	strncpy(tmp, "Main", sizeof(tmp));
								break;
				case SUBCPU :	strncpy(tmp, "Sub ", sizeof(tmp));
								break;
#if XM7_VER == 1
#if defined(JSUB)
				case JSUBCPU :	strncpy(tmp, "Jsub", sizeof(tmp));
								break;
#endif
#if defined(Z80CARD)
				case MAINZ80 : strncpy(tmp, "Z80 ", sizeof(tmp));
								break;
#endif
#endif
			}
			_snprintf(string, sizeof(string), "%2d %s %04X ",
				i + 1, tmp, breakp[i].addr);
			switch (breakp[i].flag) {
				case BREAKP_ENABLED:
					strncat(string, " Enabled", sizeof(string) - strlen(string) - 1);
					break;
				case BREAKP_DISABLED:
					strncat(string, "Disabled", sizeof(string) - strlen(string) - 1);
					break;
			}
		}

		/* �R�s�[ */
		memcpy(&p[x * (i % 8) + 23 * (i / 8)], string, strlen(string));
	}

	/* VM���A�����b�N */
	UnlockVM();
}

/*
 *	�u���[�N�|�C���g�E�C���h�E
 *	�`��
 */
static void FASTCALL DrawBreakPoint(HWND hWnd, HDC hDC)
{
	RECT rect;
	int x, y;

	ASSERT(hWnd);
	ASSERT(hDC);

	/* �E�C���h�E�W�I���g���𓾂� */
	GetClientRect(hWnd, &rect);
	x = rect.right / lCharWidth;
	y = rect.bottom / lCharHeight;
	if ((x == 0) || (y == 0)) {
		return;
	}

	/* �Z�b�g�A�b�v */
	if (!pBreakPoint) {
		return;
	}
	SetupBreakPoint(pBreakPoint, x, y);

	/* �`�� */
	DrawWindowText(hDC, pBreakPoint, x, y);
}

/*
 *	�u���[�N�|�C���g�E�C���h�E
 *	���t���b�V��
 */
void FASTCALL RefreshBreakPoint(void)
{
	HWND hWnd;
	HDC hDC;

	/* ��ɌĂ΂��̂ŁA���݃`�F�b�N���邱�� */
	if (hSubWnd[SWND_BREAKPOINT] == NULL) {
		return;
	}

	/* �`�� */
	hWnd = hSubWnd[SWND_BREAKPOINT];
	hDC = GetDC(hWnd);
	DrawBreakPoint(hWnd, hDC);
	ReleaseDC(hWnd, hDC);
}

/*
 *	�u���[�N�|�C���g�E�C���h�E
 *	�ĕ`��
 */
static void FASTCALL PaintBreakPoint(HWND hWnd)
{
	HDC hDC;
	PAINTSTRUCT ps;
	RECT rect;
	BYTE *p;
	int x, y;

	ASSERT(hWnd);

	/* �|�C���^��ݒ�(���݂��Ȃ���Ή������Ȃ�) */
	p = pBreakPoint;
	if (!p) {
		return;
	}

	/* �E�C���h�E�W�I���g���𓾂� */
	GetClientRect(hWnd, &rect);
	x = rect.right / lCharWidth;
	y = rect.bottom / lCharHeight;

	/* �㔼�G���A��FF�Ŗ��߂� */
	if ((x > 0) && (y > 0)) {
		memset(&p[x * y], 0xff, x * y);
	}

	/* �`�� */
	hDC = BeginPaint(hWnd, &ps);
	ASSERT(hDC);
	DrawBreakPoint(hWnd, hDC);
	EndPaint(hWnd, &ps);
}

/*
 *	�u���[�N�|�C���g
 *	�R�}���h
 */
static void FASTCALL CmdBreakPoint(HWND hWnd, WORD wID)
{
	int x, y;
	int num;
	POINT point;

	ASSERT(hWnd);

	/* �C���f�b�N�X�ԍ��擾 */
	point = PosBreakPoint;
	x = point.x / lCharWidth;
	y = point.y / lCharHeight;
	num = (x / 23) * 8 + y;
	if ((num < 0) || ((num < 8) && (x > 20)) || (num >= BREAKP_MAXNUM)) {
		return;
	}

	/* �R�}���h�� */
	switch (wID) {
		/* �W�����v */
		case IDM_BREAKP_JUMP:
			if ((num < 8) && (x > 20)) {
				break;
			}
			if (breakp[num].flag != BREAKP_NOTUSE) {
				AddrDisAsm((BYTE)breakp[num].cpu, breakp[num].addr);
			}
			break;

		/* �ݒ� */
		case IDM_BREAKP_SET:
			if ((num < 8) && (x > 20)) {
				break;
			}
			BreakPointDlg(hWnd, num);
			break;

		/* �C�l�[�u�� */
		case IDM_BREAKP_ENABLE:
			if ((num < 8) && (x > 20)) {
				break;
			}
			if (breakp[num].flag == BREAKP_DISABLED) {
				breakp[num].flag = BREAKP_ENABLED;
				InvalidateRect(hWnd, NULL, FALSE);
				RefreshDisAsm();
			}
			break;

		/* �f�B�Z�[�u�� */
		case IDM_BREAKP_DISABLE:
			if ((num < 8) && (x > 20)) {
				break;
			}
			if (breakp[num].flag == BREAKP_ENABLED) {
				breakp[num].flag = BREAKP_DISABLED;
				InvalidateRect(hWnd, NULL, FALSE);
				RefreshDisAsm();
			}
			break;

		/* �N���A */
		case IDM_BREAKP_CLEAR:
			if ((num < 8) && (x > 20)) {
				break;
			}
			if (breakp[num].flag != BREAKP_NOTUSE) {
				schedule_setbreak(breakp[num].cpu, breakp[num].addr);
			}
			InvalidateRect(hWnd, NULL, FALSE);
			RefreshDisAsm();
			break;

		/* �S�ăN���A */
		case IDM_BREAKP_ALL:
			for (num=0; num<BREAKP_MAXNUM; num++) {
				if (breakp[num].flag != BREAKP_NOTUSE) {
					schedule_setbreak(breakp[num].cpu, breakp[num].addr);
				}
			}
			InvalidateRect(hWnd, NULL, FALSE);
			RefreshDisAsm();
			break;
	}
}

/*
 *	�u���[�N�|�C���g�E�C���h�E
 *	�E�C���h�E�v���V�[�W��
 */
static LRESULT CALLBACK BreakPointProc(HWND hWnd, UINT message,
								 WPARAM wParam, LPARAM lParam)
{
	POINT point;
	int i, x, y;
	HMENU hMenu;

	/* ���b�Z�[�W�� */
	switch (message) {
		/* �E�C���h�E�ĕ`�� */
		case WM_PAINT:
			/* ���b�N���K�v */
			LockVM();
			PaintBreakPoint(hWnd);
			UnlockVM();
			return 0;

		/* ���N���b�N */
		case WM_LBUTTONDOWN:
		case WM_LBUTTONDBLCLK:
#if defined(MOUSE)
			/* �}�E�X�G�~�����[�V�����g�p���͖��� */
			if (mos_capture && !stopreq_flag && run_flag) {
				return FALSE;
			}
#endif

			/* �J�[�\���ʒu����A���� */
			x = LOWORD(lParam) / lCharWidth;
			y = HIWORD(lParam) / lCharHeight;
			i = (x / 23) * 8 + y;
			if ((i < 8) && (x > 20)) {
				return 0;
			}
			if ((i >= 0) && (i < BREAKP_MAXNUM)) {
				if (breakp[i].flag != BREAKP_NOTUSE) {
					AddrDisAsm((BYTE)breakp[i].cpu, breakp[i].addr);
				}
			}
			return 0;

		/* �R���e�L�X�g���j���[ */
		case WM_RBUTTONDOWN:
#if defined(MOUSE)
			/* �}�E�X�G�~�����[�V�����g�p���͖��� */
			if (mos_capture && !stopreq_flag && run_flag) {
				return FALSE;
			}
#endif

			point.x = LOWORD(lParam);
			point.y = HIWORD(lParam);
			PosBreakPoint = point;
			hMenu = GetSubMenu(hBreakPoint, 0);
			ClientToScreen(hWnd, &point);
			TrackPopupMenu(hMenu, 0, point.x, point.y, 0, hWnd, NULL);
			return 0;

		/* �E�C���h�E�폜 */
		case WM_DESTROY:
			LockVM();

			/* ���j���[�폜 */
			DestroyMenu(hBreakPoint);

			/* ���C���E�C���h�E�֎����ʒm */
			DestroySubWindow(hWnd, &pBreakPoint, NULL);

			UnlockVM();
			break;

		/* �R�}���h */
		case WM_COMMAND:
			CmdBreakPoint(hWnd, LOWORD(wParam));
			break;
	}

	/* �f�t�H���g �E�C���h�E�v���V�[�W�� */
	return DefWindowProc(hWnd, message, wParam, lParam);
}

/*
 *	�u���[�N�|�C���g�E�C���h�E
 *	�E�C���h�E�쐬
 */
HWND FASTCALL CreateBreakPoint(HWND hParent, int index)
{
	WNDCLASSEX wcex;
	char szClassName[] = "XM7_BreakPoint";
	char szWndName[128];
	RECT rect;
	RECT crect, wrect;
	HWND hWnd;
	DWORD dwStyle;

	ASSERT(hParent);

	/* �E�C���h�E��`���v�Z */
	PositioningSubWindow(hParent, &rect, index);
	rect.right = lCharWidth * 44;
	rect.bottom = lCharHeight * (BREAKP_MAXNUM / 2);

	/* �E�C���h�E�^�C�g��������A�o�b�t�@�m�� */
	LoadString(hAppInstance, IDS_SWND_BREAKPOINT,
				szWndName, sizeof(szWndName));
	pBreakPoint = malloc(2 * (rect.right / lCharWidth) *
								(rect.bottom / lCharHeight));

	/* ���j���[�����[�h */
	hBreakPoint = LoadMenu(hAppInstance, MAKEINTRESOURCE(IDR_BREAKPOINTMENU));

	/* �E�C���h�E�N���X�̓o�^ */
	memset(&wcex, 0, sizeof(wcex));
	wcex.cbSize = sizeof(wcex);
	wcex.style = CS_VREDRAW | CS_HREDRAW;
	wcex.lpfnWndProc = BreakPointProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hAppInstance;
	wcex.hIcon = LoadIcon(hAppInstance, MAKEINTRESOURCE(IDI_WNDICON));
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	wcex.lpszMenuName = NULL;
	wcex.lpszClassName = szClassName;
	wcex.hIconSm = LoadIcon(hAppInstance, MAKEINTRESOURCE(IDI_WNDICON));
	RegisterClassEx(&wcex);

	/* �E�C���h�E�쐬 */
	if (bPopupSwnd) {
		dwStyle =	WS_POPUP | WS_OVERLAPPED | WS_SYSMENU | WS_CAPTION |
					WS_VISIBLE | WS_MINIMIZEBOX | WS_BORDER;
	}
	else {
		dwStyle = WS_CHILD | WS_OVERLAPPED | WS_SYSMENU | WS_CAPTION |
					WS_VISIBLE | WS_MINIMIZEBOX | WS_CLIPSIBLINGS;
	}
	hWnd = CreateWindow(szClassName,
						szWndName,
						dwStyle,
						rect.left,
						rect.top,
						rect.right,
						rect.bottom,
						hParent,
						NULL,
						hAppInstance,
						NULL);

	/* �L���Ȃ�A�T�C�Y�␳���Ď�O�ɒu�� */
	if (hWnd) {
		GetWindowRect(hWnd, &wrect);
		GetClientRect(hWnd, &crect);
		wrect.right += (rect.right - crect.right);
		wrect.bottom += (rect.bottom - crect.bottom);
		SetWindowPos(hWnd, HWND_TOP, wrect.left, wrect.top,
			wrect.right - wrect.left, wrect.bottom - wrect.top, SWP_NOMOVE);
	}

	/* �|�b�v�A�b�v�E�C���h�E���̓A�N�e�B�u�E�C���h�E��O�ʂɕύX */
	if (bPopupSwnd) {
		SetForegroundWindow(hMainWnd);
	}

	/* ���ʂ������A�� */
	return hWnd;
}

/*-[ �X�P�W���[���E�C���h�E ]------------------------------------------------*/

static char *pszSchedulerTitle[] = {
	"Main Timer",
	"Sub  Timer",
	"OPN Timer A",
	"OPN Timer B",
	"Key Repeat",
	"BEEP",
	"V-SYNC",
	"V/H-BLANK",
#if XM7_VER >= 2
	"Line LSI",
	"RTC Clock",	/* V3.2L02�` */
#else
	"Text Blink",
	"(Reserved)",
#endif
	"WHG Timer A",
	"WHG Timer B",
	"THG Timer A",
	"THG Timer B",
	"FDC (M.Sec)",
	"FDC (Lost)",
	"FDD (Seek)",
	"Tape Sound",
#if defined(MOUSE)
	"Mouse Lost",
#else
	"(Reserved)",
#endif
#if XM7_VER >= 3
	"DMA Start",
#else
	"(Reserved)",
#endif
#if XM7_VER >= 2
	"KeyEnc ACK",
#else
	"(Reserved)",
#endif
#if defined(RSC)
	"RS TxTiming",
	"RS RxTiming",
#else
	"(Reserved)",
	"(Reserved)",
#endif
#if XM7_VER >= 2
	"KeyEnc BEEP",
	"KeyEnc MSG",
#else
	"(Reserved)",
	"(Reserved)",
#endif
#if defined(MOUSE)
	"PTM Timer",
#else
	"(Reserved)",
#endif
#if XM7_VER == 1 && defined(BUBBLE)
	"Bubble Lost",
	"BubbleMPage",
#else
	"(Reserved)",
	"(Reserved)",
#endif
	"(Reserved)",
	"(Reserved)",
	"(Reserved)",
	"(Reserved)",
};

/*
 *	�X�P�W���[���E�C���h�E
 *	�Z�b�g�A�b�v
 */
static void FASTCALL SetupScheduler(BYTE *p, int x, int y)
{
	int i;
	int j;
	char string[128];
	DWORD dwFrame;
	DWORD dwTime;

	ASSERT(p);
	ASSERT(x > 0);
	ASSERT(y > 0);

	/* VM�����b�N */
	LockVM();

	/* ��U�X�y�[�X�Ŗ��߂� */
	memset(p, 0x20, x * y);

	/* ���s���� */
	_snprintf(string, sizeof(string), "Execute Time %12d ms",
		dwExecTotal / 1000);
	memcpy(p, string, strlen(string));

	/* �t���[�����g�[�^�� */
	dwFrame = dwDrawTotal - dwSchedulerFrame[0];
	dwTime = timeGetTime() - dwSchedulerTime[0];
	if (dwTime > 0) {
		dwFrame = dwFrame * 100000 / dwTime;
	}
	else {
		dwSchedulerTime[0] = timeGetTime();
		dwFrame = 0;
	}
	_snprintf(string, sizeof(string), "Frame Rate (Total) %3d.%02d fps",
		dwFrame / 100, dwFrame % 100);
	memcpy(&p[x * 1], string, strlen(string));

	/* �t���[����2sec */
	dwTime = timeGetTime() - dwSchedulerTime[1];
	if (dwTime > 0) {
		dwFrame = dwDrawTotal - dwSchedulerFrame[1];
		dwFrame = dwFrame * 100000 / dwTime;
	}
	else {
		dwFrame = 0;
	}
	if ((dwTime < 0) || (dwTime > 2000)) {
		dwSchedulerTime[1] = timeGetTime();
		dwSchedulerFrame[1] = dwDrawTotal;
	}
	_snprintf(string, sizeof(string), "Frame Rate (Latest)%3d.%02d fps",
		dwFrame / 100, dwFrame % 100);
	memcpy(&p[x * 2], string, strlen(string));

	/* ���[�v */
	j = 4;
	for (i=0; i<EVENT_MAXNUM; i++) {
		/* �^�C�g�� */
		memcpy(&p[x * j], pszSchedulerTitle[i], strlen(pszSchedulerTitle[i]));

		/* �J�����g�A�����[�h */
		if (event[i].flag != EVENT_NOTUSE) {
			_snprintf(string, sizeof(string), "%4d.%03dms",
				event[i].current / 1000, event[i].current % 1000);
			memcpy(&p[x * j + 12], string, strlen(string));

			_snprintf(string, sizeof(string), "(%4d.%03dms)",
				event[i].reload / 1000, event[i].reload % 1000);
			memcpy(&p[x * j + 23], string, strlen(string));

			/* �X�e�[�^�X */
			switch (event[i].flag) {
				case EVENT_ENABLED:
					strncpy(string, " Enabled", sizeof(string));
					break;

				case EVENT_DISABLED:
					strncpy(string, "Disabled", sizeof(string));
					break;

				default:
					ASSERT(FALSE);
					break;
			}
		}
		else {
			strncpy(string, "", sizeof(string));
		}

		memcpy(&p[x * j + 36], string, strlen(string));
		j++;
	}

	/* �I�[�o�[�T�C�N���� */
	strncpy(string,  "|  Over Cycle", sizeof(string));
	memcpy(&p[x * 0 + 30], string, strlen(string));
	_snprintf(string, sizeof(string), "| Main %7d", main_overcycles);
	memcpy(&p[x * 1 + 30], string, strlen(string));
	_snprintf(string, sizeof(string), "| Sub  %7d", sub_overcycles);
	memcpy(&p[x * 2 + 30], string, strlen(string));

#if defined(DEBUG)
	if (dwFrame == 0) {
		dwTime = 100000;
	}
	else {
		dwTime = 100000 / dwFrame;
	}

	/* ��������(�ł΂����΁`�����p) */
	_snprintf(string, sizeof(string), "Exec Time (CPU)  %4d.%03d ms (%3d\%)",
		dwCpuExecTime / 1000, dwCpuExecTime % 1000, dwCpuExecTime / dwTime);
	memcpy(&p[x * (j + 1)], string, strlen(string));

	_snprintf(string, sizeof(string), "Exec Time (DRAW) %4d.%03d ms (%3d\%)",
		dwDrawExecTime / 1000, dwDrawExecTime % 1000, dwDrawExecTime / dwTime);
	memcpy(&p[x * (j + 2)], string, strlen(string));

	_snprintf(string, sizeof(string), "Exec Time (POLL) %4d.%03d ms (%3d\%)",
		dwPollExecTime / 1000, dwPollExecTime % 1000, dwPollExecTime / dwTime);
	memcpy(&p[x * (j + 3)], string, strlen(string));

	i = (dwTime * 100) - (dwCpuExecTime + dwDrawExecTime + dwPollExecTime);
	if (i < 0) {
		i = 0;
	}
	_snprintf(string, sizeof(string), "Idle Time        %4d.%03d ms (%3d\%)",
		i / 1000, i % 1000, i / dwTime);
	memcpy(&p[x * (j + 4)], string, strlen(string));
#endif

	/* VM���A�����b�N */
	UnlockVM();
}

/*
 *	�X�P�W���[���E�C���h�E
 *	�`��
 */
static void FASTCALL DrawScheduler(HWND hWnd, HDC hDC)
{
	RECT rect;
	int x, y;

	ASSERT(hWnd);
	ASSERT(hDC);

	/* �E�C���h�E�W�I���g���𓾂� */
	GetClientRect(hWnd, &rect);
	x = rect.right / lCharWidth;
	y = rect.bottom / lCharHeight;
	if ((x == 0) || (y == 0)) {
		return;
	}

	/* �Z�b�g�A�b�v */
	if (!pScheduler) {
		return;
	}
	SetupScheduler(pScheduler, x, y);

	/* �`�� */
	DrawWindowText(hDC, pScheduler, x, y);
}

/*
 *	�X�P�W���[���E�C���h�E
 *	���t���b�V��
 */
void FASTCALL RefreshScheduler(void)
{
	HWND hWnd;
	HDC hDC;

	/* ��ɌĂ΂��̂ŁA���݃`�F�b�N���邱�� */
	if (hSubWnd[SWND_SCHEDULER] == NULL) {
		return;
	}

	/* �`�� */
	hWnd = hSubWnd[SWND_SCHEDULER];
	hDC = GetDC(hWnd);
	DrawScheduler(hWnd, hDC);
	ReleaseDC(hWnd, hDC);
}

/*
 *	�X�P�W���[���E�C���h�E
 *	�ĕ`��
 */
static void FASTCALL PaintScheduler(HWND hWnd)
{
	HDC hDC;
	PAINTSTRUCT ps;
	RECT rect;
	BYTE *p;
	int x, y;

	ASSERT(hWnd);

	/* �|�C���^��ݒ�(���݂��Ȃ���Ή������Ȃ�) */
	p = pScheduler;
	if (!p) {
		return;
	}

	/* �E�C���h�E�W�I���g���𓾂� */
	GetClientRect(hWnd, &rect);
	x = rect.right / lCharWidth;
	y = rect.bottom / lCharHeight;

	/* �㔼�G���A��FF�Ŗ��߂� */
	if ((x > 0) && (y > 0)) {
		memset(&p[x * y], 0xff, x * y);
	}

	/* �`�� */
	hDC = BeginPaint(hWnd, &ps);
	ASSERT(hDC);
	DrawScheduler(hWnd, hDC);
	EndPaint(hWnd, &ps);
}

/*
 *	�X�P�W���[���E�C���h�E
 *	�E�C���h�E�v���V�[�W��
 */
static LRESULT CALLBACK SchedulerProc(HWND hWnd, UINT message,
								 WPARAM wParam, LPARAM lParam)
{
	/* ���b�Z�[�W�� */
	switch (message) {
		/* �E�C���h�E�ĕ`�� */
		case WM_PAINT:
			/* ���b�N���K�v */
			LockVM();
			PaintScheduler(hWnd);
			UnlockVM();
			return 0;

		/* �E�C���h�E�폜 */
		case WM_DESTROY:
			LockVM();

			/* ���C���E�C���h�E�֎����ʒm */
			DestroySubWindow(hWnd, &pScheduler, NULL);

			UnlockVM();
			break;
	}

	/* �f�t�H���g �E�C���h�E�v���V�[�W�� */
	return DefWindowProc(hWnd, message, wParam, lParam);
}

/*
 *	�X�P�W���[���E�C���h�E
 *	�E�C���h�E�쐬
 */
HWND FASTCALL CreateScheduler(HWND hParent, int index)
{
	WNDCLASSEX wcex;
	char szClassName[] = "XM7_Scheduler";
	char szWndName[128];
	RECT rect;
	RECT crect, wrect;
	HWND hWnd;
	DWORD dwStyle;

	ASSERT(hParent);

	/* �E�C���h�E��`���v�Z */
	PositioningSubWindow(hParent, &rect, index);
	rect.right = lCharWidth * 44;
#if defined(DEBUG)
	rect.bottom = lCharHeight * (EVENT_MAXNUM + 9);
#else
	rect.bottom = lCharHeight * (EVENT_MAXNUM + 4);
#endif

	/* �E�C���h�E�^�C�g��������A�o�b�t�@�m�� */
	LoadString(hAppInstance, IDS_SWND_SCHEDULER,
				szWndName, sizeof(szWndName));
	pScheduler = malloc(2 * (rect.right / lCharWidth) *
								(rect.bottom / lCharHeight));

	/* ���ԁA�t���[���������� */
	dwSchedulerTime[0] = timeGetTime();
	dwSchedulerTime[1] = dwSchedulerTime[0];
	dwSchedulerFrame[0] = dwDrawTotal;
	dwSchedulerFrame[1] = dwSchedulerFrame[0];

	/* �E�C���h�E�N���X�̓o�^ */
	memset(&wcex, 0, sizeof(wcex));
	wcex.cbSize = sizeof(wcex);
	wcex.style = CS_VREDRAW | CS_HREDRAW;
	wcex.lpfnWndProc = SchedulerProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hAppInstance;
	wcex.hIcon = LoadIcon(hAppInstance, MAKEINTRESOURCE(IDI_WNDICON));
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	wcex.lpszMenuName = NULL;
	wcex.lpszClassName = szClassName;
	wcex.hIconSm = LoadIcon(hAppInstance, MAKEINTRESOURCE(IDI_WNDICON));
	RegisterClassEx(&wcex);

	/* �E�C���h�E�쐬 */
	if (bPopupSwnd) {
		dwStyle =	WS_POPUP | WS_OVERLAPPED | WS_SYSMENU | WS_CAPTION |
					WS_VISIBLE | WS_MINIMIZEBOX | WS_BORDER;
	}
	else {
		dwStyle = WS_CHILD | WS_OVERLAPPED | WS_SYSMENU | WS_CAPTION |
					WS_VISIBLE | WS_MINIMIZEBOX | WS_CLIPSIBLINGS;
	}
	hWnd = CreateWindow(szClassName,
						szWndName,
						dwStyle,
						rect.left,
						rect.top,
						rect.right,
						rect.bottom,
						hParent,
						NULL,
						hAppInstance,
						NULL);

	/* �L���Ȃ�A�T�C�Y�␳���Ď�O�ɒu�� */
	if (hWnd) {
		GetWindowRect(hWnd, &wrect);
		GetClientRect(hWnd, &crect);
		wrect.right += (rect.right - crect.right);
		wrect.bottom += (rect.bottom - crect.bottom);
		SetWindowPos(hWnd, HWND_TOP, wrect.left, wrect.top,
			wrect.right - wrect.left, wrect.bottom - wrect.top, SWP_NOMOVE);
	}

	/* �|�b�v�A�b�v�E�C���h�E���̓A�N�e�B�u�E�C���h�E��O�ʂɕύX */
	if (bPopupSwnd) {
		SetForegroundWindow(hMainWnd);
	}

	/* ���ʂ������A�� */
	return hWnd;
}

/*-[ ���W�X�^���e�ύX�_�C�A���O ]--------------------------------------------*/

/*
 *	���W�X�^���e�ύX�_�C�A���O
 *	�_�C�A���O������
 */
static BOOL FASTCALL CPURegDlgInit(HWND hDlg)
{
	static const char *RegName[] = {
		"CC", "A", "B", "DP", "X", "Y", "U", "S", "D", "", "IR", "PC"
	};
#if XM7_VER == 1 && defined(Z80CARD)
	static const char *RegNameZ80[] = {
		"AF",	"BC",	"DE",	"HL",	"IX",	"IY",	"SP",	"PC",
	};
#endif

	HWND hWnd;
	RECT prect;
	RECT drect;
	cpu6809_t *pReg;
	char tmp[16];
	char tmp2[128];
	char string[128];
	WORD dat;
	UINT id;

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

	/* �����ݒ� */
	switch (nRegDlgCPU) {
		case MAINCPU :	id = IDS_SWND_CPU_MAIN;
						pReg = &maincpu;
						break;
		case SUBCPU :	id = IDS_SWND_CPU_SUB;
						pReg = &subcpu;
						break;
#if XM7_VER == 1
#if defined(JSUB)
		case JSUBCPU :	id = IDS_SWND_CPU_JSUB;
						pReg = &jsubcpu;
						break;
#endif
#if defined(Z80CARD)
		case MAINZ80 :	id = IDS_SWND_CPU_MAINZ80;
						pReg = NULL;
						break;
#endif
#endif
	}
	LoadString(hAppInstance, id, tmp, sizeof(tmp));

	/* �A�h���X��ݒ� */
	hWnd = GetDlgItem(hDlg, IDC_REGLABEL);
	ASSERT(hWnd);
#if XM7_VER == 1 && defined(Z80CARD)
	if (nRegDlgCPU == MAINZ80) {
		if (LoadString(hAppInstance, IDS_SWND_REG_MESSAGE,
						tmp2, sizeof(tmp2)) == 0) {
			_snprintf(string, sizeof(string), "%s���W�X�^ (%s)",
				RegNameZ80[nRegDlgNo], "Z80");
		}
		else {
			_snprintf(string, sizeof(string), tmp2,
				RegNameZ80[nRegDlgNo], "Z80");
		}
	}
	else {
		if (LoadString(hAppInstance, IDS_SWND_REG_MESSAGE,
						tmp2, sizeof(tmp2)) == 0) {
			_snprintf(string, sizeof(string), "%s���W�X�^ (%s)",
				RegName[nRegDlgNo], tmp);
		}
		else {
			_snprintf(string, sizeof(string), tmp2, RegName[nRegDlgNo], tmp);
		}
	}
#else
	if (LoadString(hAppInstance, IDS_SWND_REG_MESSAGE,
					tmp2, sizeof(tmp2)) == 0) {
		_snprintf(string, sizeof(string), "%s���W�X�^ (%s)",
			RegName[nRegDlgNo], tmp);
	}
	else {
		_snprintf(string, sizeof(string), tmp2, RegName[nRegDlgNo], tmp);
	}
#endif
	SetWindowText(hWnd, string);

	/* ���f�[�^��ݒ� */
	hWnd = GetDlgItem(hDlg, IDC_REGEDIT);
	ASSERT(hWnd);

#if XM7_VER == 1 && defined(Z80CARD)
	if (nRegDlgCPU == MAINZ80) {
		switch (nRegDlgNo) {
			case 0:	/* AF */
					dat = (WORD)((mainz80.regs8[REGID_A] << 8) |
						mainz80.regs8[REGID_F]);
					break;
			case 1:	/* BC */
					dat = (WORD)((mainz80.regs8[REGID_B] << 8) |
						mainz80.regs8[REGID_C]);
					break;
			case 2:	/* DE */
					dat = (WORD)((mainz80.regs8[REGID_D] << 8) |
						mainz80.regs8[REGID_E]);
					break;
			case 3:	/* HL */
					dat = (WORD)((mainz80.regs8[REGID_H] << 8) |
						mainz80.regs8[REGID_L]);
					break;
			case 4:	/* IX */
					dat = (WORD)((mainz80.regs8[REGID_IXH] << 8) |
						mainz80.regs8[REGID_IXL]);
					break;
			case 5:	/* IY */
					dat = (WORD)((mainz80.regs8[REGID_IYH] << 8) |
						mainz80.regs8[REGID_IYL]);
					break;
			case 6:	/* SP */
					dat = mainz80.sp;
					break;
			case 7:	/* PC */
					dat = mainz80.pc;
					break;
			default:
					ASSERT(FALSE);
		}
		_snprintf(string, sizeof(string), "%04X", dat);
		SetWindowText(hWnd, string);

		return TRUE;
	}
#endif

	switch (nRegDlgNo) {
		case 0:	/* CC */
				dat = pReg -> cc;
				break;
		case 1:	/* A */
				dat = pReg -> acc.h.a;
				break;
		case 2:	/* B */
				dat = pReg -> acc.h.b;
				break;
		case 3:	/* DP */
				dat = pReg -> dp;
				break;
		case 4:	/* X */
				dat = pReg -> x;
				break;
		case 5:	/* Y */
				dat = pReg -> y;
				break;
		case 6:	/* U */
				dat = pReg -> u;
				break;
		case 7:	/* S */
				dat = pReg -> s;
				break;
		case 8:	/* D */
				dat = pReg -> acc.d;
				break;
		case 11:	/* PC */
				dat = pReg -> pc;
				break;
		default:
				ASSERT(FALSE);
	}
	if (nRegDlgNo < 4) {
		/* 8�r�b�g���W�X�^ */
		_snprintf(string, sizeof(string), "%02X", dat);
	}
	else {
		/* 16�r�b�g���W�X�^ */
		_snprintf(string, sizeof(string), "%04X", dat);
	}
	SetWindowText(hWnd, string);

	return TRUE;
}

/*
 *	���W�X�^���e�ύX�_�C�A���O
 *	�_�C�A���OOK
 */
static void FASTCALL CPURegDlgOK(HWND hDlg)
{
	HWND hWnd;
	cpu6809_t *pReg;
	char string[128];
	WORD dat;

	ASSERT(hDlg);
	LockVM();

	/* �����ݒ� */
	switch (nRegDlgCPU) {
		case MAINCPU :	pReg = &maincpu;
						break;
		case SUBCPU :	pReg = &subcpu;
						break;
#if XM7_VER == 1
#if defined(JSUB)
		case JSUBCPU :	pReg = &jsubcpu;
						break;
#endif
#endif
	}

	/* ���݂̒l���擾 */
	hWnd = GetDlgItem(hDlg, IDC_REGEDIT);
	ASSERT(hWnd);
	GetWindowText(hWnd, string, sizeof(string) - 1);
	dat = (WORD)strtol(string, NULL, 16);

#if XM7_VER == 1 && defined(Z80CARD)
	if (nRegDlgCPU == MAINZ80) {
		/* �f�[�^��ݒ� */
		switch (nRegDlgNo) {
			case 0:	/* AF */
					mainz80.regs8[REGID_A] = (BYTE)((dat >> 8) & 0xff);
					mainz80.regs8[REGID_F] = (BYTE)(dat & 0xff);
					break;
			case 1:	/* BC */
					mainz80.regs8[REGID_B] = (BYTE)((dat >> 8) & 0xff);
					mainz80.regs8[REGID_C] = (BYTE)(dat & 0xff);
					break;
			case 2:	/* DE */
					mainz80.regs8[REGID_D] = (BYTE)((dat >> 8) & 0xff);
					mainz80.regs8[REGID_E] = (BYTE)(dat & 0xff);
					break;
			case 3:	/* HL */
					mainz80.regs8[REGID_H] = (BYTE)((dat >> 8) & 0xff);
					mainz80.regs8[REGID_L] = (BYTE)(dat & 0xff);
					break;
			case 4:	/* IX */
					mainz80.regs8[REGID_IXH] = (BYTE)((dat >> 8) & 0xff);
					mainz80.regs8[REGID_IXL] = (BYTE)(dat & 0xff);
					break;
			case 5:	/* IY */
					mainz80.regs8[REGID_IYH] = (BYTE)((dat >> 8) & 0xff);
					mainz80.regs8[REGID_IYL] = (BYTE)(dat & 0xff);
					break;
			case 6:	/* SP */
					mainz80.sp = dat;
					break;
			case 7:	/* PC */
					mainz80.pc = dat;
					break;
			default:
					ASSERT(FALSE);
		}
	}
	else {
		/* �f�[�^��ݒ� */
		switch (nRegDlgNo) {
			case 0:	/* CC */
					pReg -> cc = (BYTE)dat;
					break;
			case 1:	/* A */
					pReg -> acc.h.a = (BYTE)dat;
					break;
			case 2:	/* B */
					pReg -> acc.h.b = (BYTE)dat;
					break;
			case 3:	/* DP */
					pReg -> dp = (BYTE)dat;
					break;
			case 4:	/* X */
					pReg -> x = dat;
					break;
			case 5:	/* Y */
					pReg -> y = dat;
					break;
			case 6:	/* U */
					pReg -> u = dat;
					break;
			case 7:	/* S */
					pReg -> s = dat;
					break;
			case 8:	/* D */
					pReg -> acc.d = dat;
					break;
			case 11:	/* PC */
					pReg -> pc = dat;
					break;
			default:
					ASSERT(FALSE);
		}
	}
#else
	/* �f�[�^��ݒ� */
	switch (nRegDlgNo) {
		case 0:	/* CC */
				pReg -> cc = (BYTE)dat;
				break;
		case 1:	/* A */
				pReg -> acc.h.a = (BYTE)dat;
				break;
		case 2:	/* B */
				pReg -> acc.h.b = (BYTE)dat;
				break;
		case 3:	/* DP */
				pReg -> dp = (BYTE)dat;
				break;
		case 4:	/* X */
				pReg -> x = dat;
				break;
		case 5:	/* Y */
				pReg -> y = dat;
				break;
		case 6:	/* U */
				pReg -> u = dat;
				break;
		case 7:	/* S */
				pReg -> s = dat;
				break;
		case 8:	/* D */
				pReg -> acc.d = dat;
				break;
		case 11:	/* PC */
				pReg -> pc = dat;
				break;
		default:
				ASSERT(FALSE);
	}
#endif

	UnlockVM();
}

/*
 *	���W�X�^���e�ύX�_�C�A���O
 *	�_�C�A���O�v���V�[�W��
 */
static BOOL CALLBACK CPURegDlgProc(HWND hDlg, UINT iMsg,
									WPARAM wParam, LPARAM lParam)
{
	UNUSED(lParam);

	switch (iMsg) {
		/* �_�C�A���O������ */
		case WM_INITDIALOG:
			return CPURegDlgInit(hDlg);

		/* �R�}���h���� */
		case WM_COMMAND:
			switch (LOWORD(wParam)) {
				/* OK�E�L�����Z�� */
				case IDOK:
				case IDCANCEL:
					if (LOWORD(wParam) == IDOK) {
						CPURegDlgOK(hDlg);
					}
					EndDialog(hDlg, LOWORD(wParam));
					InvalidateRect(hDrawWnd, NULL, FALSE);
					return TRUE;
			}
			break;
	}

	/* ����ȊO�́AFALSE */
	return FALSE;
}

/*
 *	���W�X�^���e�ύX
 */
static BOOL FASTCALL CPURegDlg(HWND hWnd, BYTE nCPU, int x, int y)
{
	int ret;

	ASSERT(hWnd);

	/* �p�����[�^��ۑ��E�ݒ� */
	nRegDlgCPU = nCPU;

#if XM7_VER == 1 && defined(Z80CARD)
	if (nCPU == MAINZ80) {
		/* ���W�X�^�ԍ����Z�b�g */
		nRegDlgNo = (BYTE)(((x / 20) * 4) + y);

		/* ���W�`�F�b�N */
		if ((x % 20) >= 8) {
			return FALSE;
		}
	}
	else {
		/* ���W�X�^�ԍ����Z�b�g */
		nRegDlgNo = (BYTE)(((x / 10) * 4) + y);

		/* IR���W�X�^�E���W�`�F�b�N */
		if (((nRegDlgNo == 8) && (x != -1)) ||
			(nRegDlgNo == 9) || (nRegDlgNo == 10) || ((x % 10) >= 7)) {
			return FALSE;
		}
	}
#else
	/* ���W�X�^�ԍ����Z�b�g */
	nRegDlgNo = (BYTE)(((x / 10) * 4) + y);

	/* IR���W�X�^�E���W�`�F�b�N */
	if (((nRegDlgNo == 8) && (x != -1)) ||
		 (nRegDlgNo == 9) || (nRegDlgNo == 10) || ((x % 10) >= 7)) {
		return FALSE;
	}
#endif

	/* ���[�_���_�C�A���O���s */
	ret = DialogBox(hAppInstance, MAKEINTRESOURCE(IDD_REGDLG), hWnd, CPURegDlgProc);
	if (ret != IDOK) {
		return FALSE;
	}
	return TRUE;
}

/*-[ CPU���W�X�^�E�C���h�E ]-------------------------------------------------*/

/*
 *	CPU���W�X�^�E�C���h�E
 *	�Z�b�g�A�b�v
 */
static void FASTCALL SetupCPURegister(BYTE nCPU, BYTE *p, int x, int y)
{
	char buf[128];
	cpu6809_t *pReg;

	ASSERT(nCPU < MAXCPU);
	ASSERT(p);
	ASSERT(x > 0);
	ASSERT(y > 0);

	/* ��U�X�y�[�X�Ŗ��߂� */
	memset(p, 0x20, x * y);

	/* ���W�X�^�o�b�t�@�𓾂� */
	switch (nCPU) {
		case MAINCPU :	pReg = &maincpu;
						break;
		case SUBCPU :	pReg = &subcpu;
						break;
#if XM7_VER == 1
#if defined(JSUB)
		case JSUBCPU :	pReg = &jsubcpu;
						break;
#endif
#endif
	}

	/* �Z�b�g */
	_snprintf(buf, sizeof(buf), 
		"CC   %02X   X  %04X", pReg->cc, pReg->x);
	memcpy(&p[0 * x + 0], buf, strlen(buf));
	_snprintf(buf, sizeof(buf), 
		"A    %02X   Y  %04X", pReg->acc.h.a, pReg->y);
	memcpy(&p[1 * x + 0], buf, strlen(buf));
	_snprintf(buf, sizeof(buf), 
		"B    %02X   U  %04X   IR %04X", pReg->acc.h.b, pReg->u, pReg->intr);
	memcpy(&p[2 * x + 0], buf, strlen(buf));
	_snprintf(buf, sizeof(buf), 
		"DP   %02X   S  %04X   PC %04X", pReg->dp, pReg->s, pReg->pc);
	memcpy(&p[3 * x + 0], buf, strlen(buf));
}

/*
 *	CPU���W�X�^�E�C���h�E(Z80)
 *	�Z�b�g�A�b�v
 */
#if XM7_VER == 1 && defined(Z80CARD)
static void FASTCALL SetupCPURegisterZ80(BYTE nCPU, BYTE *p, int x, int y)
{
	char buf[128];

	ASSERT(nCPU != MAINZ80);
	ASSERT(p);
	ASSERT(x > 0);
	ASSERT(y > 0);
	UNUSED(nCPU);

	/* ��U�X�y�[�X�Ŗ��߂� */
	memset(p, 0x20, x * y);

	/* �Z�b�g */
	_snprintf(buf, sizeof(buf), 
		"AF  %02X%02X  AF' %04X  IX  %02X%02X",
		mainz80.regs8[REGID_A], mainz80.regs8[REGID_F], mainz80.saf,
		mainz80.regs8[REGID_IXH], mainz80.regs8[REGID_IXL]);
	memcpy(&p[0 * x + 0], buf, strlen(buf));
	_snprintf(buf, sizeof(buf), 
		"BC  %02X%02X  BC' %04X  IY  %02X%02X",
		mainz80.regs8[REGID_B], mainz80.regs8[REGID_C], mainz80.sbc,
		mainz80.regs8[REGID_IYH], mainz80.regs8[REGID_IYL]);
	memcpy(&p[1 * x + 0], buf, strlen(buf));
	_snprintf(buf, sizeof(buf), 
		"DE  %02X%02X  DE' %04X  SP  %04X",
		mainz80.regs8[REGID_D], mainz80.regs8[REGID_E], mainz80.sde,
		mainz80.sp);
	memcpy(&p[2 * x + 0], buf, strlen(buf));
	_snprintf(buf, sizeof(buf), 
		"HL  %02X%02X  HL' %04X  PC  %04X",
		mainz80.regs8[REGID_H], mainz80.regs8[REGID_L], mainz80.shl,
		mainz80.pc);
	memcpy(&p[3 * x + 0], buf, strlen(buf));
}
#endif

/*
 *	CPU���W�X�^�E�C���h�E
 *	�`��
 */
static void FASTCALL DrawCPURegister(HWND hWnd, HDC hDC, BYTE nCPU)
{
	RECT rect;
	BYTE *p;
	int x, y;

	ASSERT(hWnd);
	ASSERT(hDC);

	/* Draw�o�b�t�@�𓾂�(���݂��Ȃ���Ή������Ȃ�) */
	p = pCPURegister[nCPU];
	if (!p) {
		return;
	}

	/* �E�C���h�E�W�I���g���𓾂� */
	GetClientRect(hWnd, &rect);
	x = rect.right / lCharWidth;
	y = rect.bottom / lCharHeight;
	if ((x == 0) || (y == 0)) {
		return;
	}

	/* �Z�b�g�A�b�v */
#if XM7_VER == 1 && defined(Z80CARD)
	if (nCPU == MAINZ80) {
		SetupCPURegisterZ80(nCPU, p, x, y);
	}
	else {
		SetupCPURegister(nCPU, p, x, y);
	}
#else
	SetupCPURegister(nCPU, p, x, y);
#endif

	/* �`�� */
	DrawWindowText(hDC, p, x, y);
}

/*
 *	CPU���W�X�^�E�C���h�E
 *	���t���b�V��
 */
void FASTCALL RefreshCPURegister(void)
{
	HWND hWnd;
	HDC hDC;

	/* ���C��CPU */
	if (hSubWnd[SWND_CPUREG_MAIN]) {
		hWnd = hSubWnd[SWND_CPUREG_MAIN];
		hDC = GetDC(hWnd);
		DrawCPURegister(hWnd, hDC, MAINCPU);
		ReleaseDC(hWnd, hDC);
	}

	/* �T�uCPU */
	if (hSubWnd[SWND_CPUREG_SUB]) {
		hWnd = hSubWnd[SWND_CPUREG_SUB];
		hDC = GetDC(hWnd);
		DrawCPURegister(hWnd, hDC, SUBCPU);
		ReleaseDC(hWnd, hDC);
	}

#if XM7_VER == 1
#if defined(JSUB)
	/* ���{��T�uCPU */
	if (hSubWnd[SWND_CPUREG_JSUB]) {
		hWnd = hSubWnd[SWND_CPUREG_JSUB];
		hDC = GetDC(hWnd);
		DrawCPURegister(hWnd, hDC, JSUBCPU);
		ReleaseDC(hWnd, hDC);
	}
#endif

#if defined(JSUB)
	/* ���C��CPU(Z80) */
	if (hSubWnd[SWND_CPUREG_Z80]) {
		hWnd = hSubWnd[SWND_CPUREG_Z80];
		hDC = GetDC(hWnd);
		DrawCPURegister(hWnd, hDC, MAINZ80);
		ReleaseDC(hWnd, hDC);
	}
#endif
#endif
}

/*
 *	CPU���W�X�^�E�C���h�E
 *	�ĕ`��
 */
static void FASTCALL PaintCPURegister(HWND hWnd)
{
	HDC hDC;
	PAINTSTRUCT ps;
	int i;
	RECT rect;
	BYTE *p;
	int x, y;
	BYTE nCPU;
	
	ASSERT(hWnd);

	/* Draw�o�b�t�@�𓾂�(���݂��Ȃ���Ή������Ȃ�) */
	for (i=0; i<SWND_MAXNUM; i++) {
		if (hSubWnd[i] == hWnd) {
			nCPU = (BYTE)((i - SWND_CPUREG_MAIN) % MAXCPU);
			p = pCPURegister[nCPU];
			break;
		}
	}

	if (!p) {
		return;
	}

	/* �E�C���h�E�W�I���g���𓾂� */
	GetClientRect(hWnd, &rect);
	x = rect.right / lCharWidth;
	y = rect.bottom / lCharHeight;

	/* �㔼�G���A��FF�Ŗ��߂� */
	if ((x > 0) && (y > 0)) {
		memset(&p[x * y], 0xff, x * y);
	}

	/* �`�� */
	hDC = BeginPaint(hWnd, &ps);
	ASSERT(hDC);
	DrawCPURegister(hWnd, hDC, nCPU);
	EndPaint(hWnd, &ps);
}

/*
 *	CPU���W�X�^�E�C���h�E
 *	�E�C���h�E�v���V�[�W��
 */
static LRESULT CALLBACK CPURegisterProc(HWND hWnd, UINT message,
								 WPARAM wParam, LPARAM lParam, BYTE nCPU)
{
	HMENU hMenu;
	POINT point;

	/* ���b�Z�[�W�� */
	switch (message) {
		/* �E�C���h�E�ĕ`�� */
		case WM_PAINT:
			LockVM();
			PaintCPURegister(hWnd);
			UnlockVM();
			return 0;

		/* �E�C���h�E�폜 */
		case WM_DESTROY:
			LockVM();

			/* ���C���E�C���h�E�֎����ʒm */
			DestroySubWindow(hWnd, &pCPURegister[nCPU], hCPURegister[nCPU]);

			UnlockVM();
			break;

		/* ���W�X�^���e�ύX */
		case WM_LBUTTONDBLCLK:
#if defined(MOUSE)
			/* �}�E�X�G�~�����[�V�����g�p���͖��� */
			if (mos_capture && !stopreq_flag && run_flag) {
				return FALSE;
			}
#endif

			/* �J�[�\���ʒu����A���� */
			point.x = LOWORD(lParam) / lCharWidth;
			point.y = HIWORD(lParam) / lCharHeight;
			CPURegDlg(hWnd, nCPU, point.x, point.y);
			return 0;

		/* �R���e�L�X�g���j���[ */
		case WM_CONTEXTMENU:
#if defined(MOUSE)
			/* �}�E�X�G�~�����[�V�����g�p���͖��� */
			if (mos_capture && !stopreq_flag && run_flag) {
				return FALSE;
			}
#endif

			/* �R���e�L�X�g���j���[�����[�h */
			hMenu = GetSubMenu(hCPURegister[nCPU], 0);

			/* �R���e�L�X�g���j���[�����s */
			point.x = LOWORD(lParam);
			point.y = HIWORD(lParam);
			TrackPopupMenu(hMenu, 0, point.x, point.y, 0, hWnd, NULL);
			return 0;

		/* �R�}���h */
		case WM_COMMAND:
#if XM7_VER == 1 && defined(Z80CARD)
			if (nCPU == MAINZ80) {
				CPURegDlg(hWnd, nCPU, -1, LOWORD(wParam) - IDM_REG_AF);
			}
			else {
				CPURegDlg(hWnd, nCPU, -1, LOWORD(wParam) - IDM_REG_CC);
			}
#else
			CPURegDlg(hWnd, nCPU, -1, LOWORD(wParam) - IDM_REG_CC);
#endif
			break;
	}

	/* �f�t�H���g �E�C���h�E�v���V�[�W�� */
	return DefWindowProc(hWnd, message, wParam, lParam);
}

/*
 *	CPU���W�X�^�E�C���h�E(���C��)
 *	�E�C���h�E�v���V�[�W��
 */
static LRESULT CALLBACK CPURegisterProcMain(HWND hWnd, UINT message,
								 WPARAM wParam, LPARAM lParam)
{
	return CPURegisterProc(hWnd, message, wParam, lParam, MAINCPU);
}

/*
 *	CPU���W�X�^�E�C���h�E(�T�u)
 *	�E�C���h�E�v���V�[�W��
 */
static LRESULT CALLBACK CPURegisterProcSub(HWND hWnd, UINT message,
								 WPARAM wParam, LPARAM lParam)
{
	return CPURegisterProc(hWnd, message, wParam, lParam, SUBCPU);
}

#if XM7_VER == 1
/*
 *	CPU���W�X�^�E�C���h�E(���{��T�u)
 *	�E�C���h�E�v���V�[�W��
 */
#if defined(JSUB)
static LRESULT CALLBACK CPURegisterProcJsub(HWND hWnd, UINT message,
								 WPARAM wParam, LPARAM lParam)
{
	return CPURegisterProc(hWnd, message, wParam, lParam, JSUBCPU);
}
#endif

/*
 *	CPU���W�X�^�E�C���h�E(���C��Z80)
 *	�E�C���h�E�v���V�[�W��
 */
#if defined(Z80CARD)
static LRESULT CALLBACK CPURegisterProcZ80(HWND hWnd, UINT message,
								 WPARAM wParam, LPARAM lParam)
{
	return CPURegisterProc(hWnd, message, wParam, lParam, MAINZ80);
}
#endif
#endif

/*
 *	CPU���W�X�^�E�C���h�E
 *	�E�C���h�E�쐬
 */
HWND FASTCALL CreateCPURegister(HWND hParent, BYTE nCPU, int index)
{
	WNDCLASSEX wcex;
	char szClassNameMain[] = "XM7_CPURegisterMain";
	char szClassNameSub[] = "XM7_CPURegisterSub";
#if XM7_VER == 1
#if defined(JSUB)
	char szClassNameJsub[] = "XM7_CPURegisterJsub";
#endif
#if defined(Z80CARD)
	char szClassNameZ80[] = "XM7_CPURegisterZ80";
#endif
#endif
	char *szClassName;
	char szWndName[128];
	RECT rect;
	RECT crect, wrect;
	HWND hWnd;
	UINT id;
	DWORD dwStyle;

	ASSERT(hParent);

	/* �E�C���h�E��`���v�Z */
	PositioningSubWindow(hParent, &rect, index);
	rect.right = lCharWidth * 27;
	rect.bottom = lCharHeight * 4;

	/* �E�C���h�E�^�C�g��������A�o�b�t�@�m�� */
	switch (nCPU) {
		case MAINCPU :	id = IDS_SWND_CPUREG_MAIN;
						szClassName = szClassNameMain;
						break;
		case SUBCPU :	id = IDS_SWND_CPUREG_SUB;
						szClassName = szClassNameSub;
						break;
#if XM7_VER == 1
#if defined(JSUB)
		case JSUBCPU :	id = IDS_SWND_CPUREG_JSUB;
						szClassName = szClassNameJsub;
						break;
#endif
#if defined(Z80CARD)
		case MAINZ80 :	id = IDS_SWND_CPUREG_Z80;
						szClassName = szClassNameZ80;
						rect.right = lCharWidth * 28;
						rect.bottom = lCharHeight * 4;
						break;
#endif
#endif
	}
	LoadString(hAppInstance, id, szWndName, sizeof(szWndName));
	pCPURegister[nCPU] = malloc(2 * (rect.right / lCharWidth) *
							(rect.bottom / lCharHeight));
#if XM7_VER == 1 && defined(Z80CARD)
	if (nCPU == MAINZ80) {
		hCPURegister[nCPU] = LoadMenu(hAppInstance,
								MAKEINTRESOURCE(IDR_CPUREGZ80MENU));
	}
	else {
		hCPURegister[nCPU] = LoadMenu(hAppInstance,
								MAKEINTRESOURCE(IDR_CPUREGMENU));
	}
#else
	hCPURegister[nCPU] = LoadMenu(hAppInstance,
							MAKEINTRESOURCE(IDR_CPUREGMENU));
#endif

	/* �E�C���h�E�N���X�̓o�^ */
	memset(&wcex, 0, sizeof(wcex));
	wcex.cbSize = sizeof(wcex);
	wcex.style = CS_VREDRAW | CS_HREDRAW | CS_DBLCLKS;
	switch (nCPU) {
		case MAINCPU :	wcex.lpfnWndProc = CPURegisterProcMain;
						break;
		case SUBCPU :	wcex.lpfnWndProc = CPURegisterProcSub;
						break;
#if XM7_VER == 1
#if defined(JSUB)
		case JSUBCPU :	wcex.lpfnWndProc = CPURegisterProcJsub;
						break;
#endif
#if defined(Z80CARD)
		case MAINZ80 :	wcex.lpfnWndProc = CPURegisterProcZ80;
						break;
#endif
#endif
	}
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hAppInstance;
	wcex.hIcon = LoadIcon(hAppInstance, MAKEINTRESOURCE(IDI_WNDICON));
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	wcex.lpszMenuName = NULL;
	wcex.lpszClassName = szClassName;
	wcex.hIconSm = LoadIcon(hAppInstance, MAKEINTRESOURCE(IDI_WNDICON));
	RegisterClassEx(&wcex);

	/* �E�C���h�E�쐬 */
	if (bPopupSwnd) {
		dwStyle =	WS_POPUP | WS_OVERLAPPED | WS_SYSMENU | WS_CAPTION |
					WS_VISIBLE | WS_MINIMIZEBOX | WS_BORDER;
	}
	else {
		dwStyle = WS_CHILD | WS_OVERLAPPED | WS_SYSMENU | WS_CAPTION |
					WS_VISIBLE | WS_MINIMIZEBOX | WS_CLIPSIBLINGS;
	}
	hWnd = CreateWindow(szClassName,
						szWndName,
						dwStyle,
						rect.left,
						rect.top,
						rect.right,
						rect.bottom,
						hParent,
						NULL,
						hAppInstance,
						NULL);

	/* �L���Ȃ�A�T�C�Y�␳���Ď�O�ɒu�� */
	if (hWnd) {
		GetWindowRect(hWnd, &wrect);
		GetClientRect(hWnd, &crect);
		wrect.right += (rect.right - crect.right);
		wrect.bottom += (rect.bottom - crect.bottom);
		SetWindowPos(hWnd, HWND_TOP, wrect.left, wrect.top,
			wrect.right - wrect.left, wrect.bottom - wrect.top, SWP_NOMOVE);
	}

	/* �|�b�v�A�b�v�E�C���h�E���̓A�N�e�B�u�E�C���h�E��O�ʂɕύX */
	if (bPopupSwnd) {
		SetForegroundWindow(hMainWnd);
	}

	/* ���ʂ������A�� */
	return hWnd;
}

/*-[ �X�^�b�N�E�u���[�N�|�C���g�W�����v ]------------------------------------*/

/*
 *	�X�^�b�N�W�����v���j���[�}��
 */
static void FASTCALL InsertStackJumpMenu(HMENU hMenu, BYTE nCPU, UINT id)
{
	BYTE (FASTCALL *readbnio)(WORD addr);
	MENUITEMINFO mii;
	char string[128];
	WORD addr;
	int i;

	/* ���j���[�\���̏����� */
	memset(&mii, 0, sizeof(mii));
	mii.cbSize = 44;
	mii.fMask = MIIM_TYPE | MIIM_STATE | MIIM_ID;
	mii.fType = MFT_STRING;
	mii.fState = MFS_ENABLED;

	/* ���j���[���ׂč폜 */
	while (GetMenuItemCount(hMenu) > 0) {
		DeleteMenu(hMenu, 0, MF_BYPOSITION);
	}

	/* ���ڂ�ǉ����� */
	for (i=15; i>=0; i--) {
		mii.wID = id + i;
		switch (nCPU) {
#if XM7_VER == 1 && defined(Z80CARD)
			case MAINZ80 :
#endif
			case MAINCPU :	readbnio = mainmem_readbnio;
							break;
			case SUBCPU :	readbnio = submem_readbnio;
							break;
#if XM7_VER == 1
#if defined(JSUB)
			case JSUBCPU :	readbnio = jsubmem_readbnio;
							break;
#endif
#endif
		}
#if XM7_VER == 1 && defined(Z80CARD)
		if (nCPU  == MAINZ80) {
			addr = (WORD)(readbnio((WORD)(mainz80.sp + i + 1)) << 8);
			addr |= (WORD)readbnio((WORD)(mainz80.sp + i));
		}
		else {
			addr = (WORD)(readbnio((WORD)(maincpu.s + i)) << 8);
			addr |= (WORD)readbnio((WORD)(maincpu.s + i + 1));
		}
#else
		addr = (WORD)(readbnio((WORD)(maincpu.s + i)) << 8);
		addr |= (WORD)readbnio((WORD)(maincpu.s + i + 1));
#endif

		wStackAddr[i] = addr;
#if XM7_VER == 1 && defined(Z80CARD)
		if (nCPU  == MAINZ80) {
			_snprintf(string, sizeof(string), "SP+%d ($%04X)", i, addr);
		}
		else {
			_snprintf(string, sizeof(string), "%d,S ($%04X)", i, addr);
		}
#else
		_snprintf(string, sizeof(string), "%d,S ($%04X)", i, addr);
#endif
		mii.dwTypeData = string;
		mii.cch = strlen(string);
		InsertMenuItem(hMenu, 0, TRUE, &mii);
	}
}

/*
 *	�u���[�N�|�C���g�W�����v���j���[�}��
 */
static void FASTCALL InsertBreakPointMenu(HMENU hMenu, BYTE nCPU, UINT id)
{
	MENUITEMINFO mii;
	char string[128];
	int no;
	int i;

	/* ���j���[�\���̏����� */
	memset(&mii, 0, sizeof(mii));
	mii.cbSize = 44;
	mii.fMask = MIIM_TYPE | MIIM_STATE | MIIM_ID;
	mii.fType = MFT_STRING;
	mii.fState = MFS_ENABLED;

	/* ���j���[���ׂč폜 */
	while (GetMenuItemCount(hMenu) > 0) {
		DeleteMenu(hMenu, 0, MF_BYPOSITION);
	}

	/* ���ڂ�ǉ����� */
	no = 0;
	for (i=15; i>=0; i--) {
		if ((breakp[i].flag != BREAKP_NOTUSE) && (breakp[i].cpu == nCPU)) {
			mii.wID = id + no;
			wBreakAddr[no++] = breakp[i].addr;
			_snprintf(string, sizeof(string), "%d : $%04X",
				i+1, breakp[i].addr);
			mii.dwTypeData = string;
			mii.cch = strlen(string);
			InsertMenuItem(hMenu, 0, TRUE, &mii);
		}
	}
	if (no == 0) {
		/* 1���u���[�N�|�C���g���Ȃ��ꍇ */
		mii.wID = id;
		mii.fState = MFS_GRAYED;
		LoadString(hAppInstance, IDS_SWND_BREAKPOINT_NONE,
				string, sizeof(string));
		mii.dwTypeData = string;
		mii.cch = strlen(string);
		InsertMenuItem(hMenu, 0, TRUE, &mii);
	}
}

/*
 *	�X�^�b�N�W�����v���j���[�}��
 */
static void FASTCALL InsertPhysicalMemMenu(HMENU hMenu, BYTE nCPU, UINT id)
{
	MENUITEMINFO mii;
	char string[128];
	int i;

	UNUSED(nCPU);
	
	/* ���j���[�\���̏����� */
	memset(&mii, 0, sizeof(mii));
	mii.cbSize = 44;
	mii.fMask = MIIM_TYPE | MIIM_STATE | MIIM_ID;
	mii.fType = MFT_STRING;
	mii.fState = MFS_ENABLED;

	/* ���j���[���ׂč폜 */
	while (GetMenuItemCount(hMenu) > 0) {
		DeleteMenu(hMenu, 0, MF_BYPOSITION);
	}

	/* ���ڂ�ǉ����� */
#if XM7_VER >= 3
	if (mmr_ext) {
		i = 15;
	}
	else {
		i = 3;
	}
#else
	i = 3;
#endif
	for (; i>=0; i--) {
		mii.wID = id + i;
		_snprintf(string, sizeof(string), "$%1X0000-$%1XFFFF", i, i);
		mii.dwTypeData = string;
		mii.cch = strlen(string);
		InsertMenuItem(hMenu, 0, TRUE, &mii);
	}
}

/*-[ �t�A�Z���u���E�C���h�E ]------------------------------------------------*/

/*
 *	�t�A�Z���u���E�C���h�E
 *	�A�h���X�ݒ�
 */
void FASTCALL AddrDisAsm(BYTE nCPU, DWORD dwAddr)
{
	SCROLLINFO sif;
	HWND hWnd;

	/* �E�C���h�E�W�I���g���𓾂� */
	hWnd = NULL;
	switch (nCPU) {
		case MAINCPU :	hWnd = hSubWnd[SWND_DISASM_MAIN];
						break;
		case SUBCPU :	hWnd = hSubWnd[SWND_DISASM_SUB];
						break;
#if XM7_VER == 1
#if defined(JSUB)
		case JSUBCPU :	hWnd = hSubWnd[SWND_DISASM_JSUB];
						break;
#endif
#if defined(Z80CARD)
		case MAINZ80 :	hWnd = hSubWnd[SWND_DISASM_Z80];
						break;
#endif
#endif
	}
	if (!hWnd) {
		/* �E�C���h�E���Ȃ���΂��̂܂܃��^�[�� */
		return;
	}

	/* SCROLLINFO�\���̂̏����� */
	memset(&sif, 0, sizeof(sif));
	sif.cbSize = sizeof(sif);
	sif.fMask = SIF_POS;
	sif.nPos = (int)dwAddr;

	/* ���݃`�F�b�N���ݒ� */
	dwDisAsm[nCPU] = dwAddr;
	SetScrollInfo(hWnd, SB_VERT, &sif, TRUE);

	/* ���t���b�V�� */
	RefreshDisAsm();
}

/*
 *	�t�A�Z���u���E�B���h�E
 *	���T�C�Y
 */
static void FASTCALL ResizeDisAsm(HWND hWnd, WORD cx, WORD cy, BYTE nCPU)
{
	RECT crect;
	RECT wrect;
	int width;

	ASSERT(hWnd);

	/* �ŏ����̏ꍇ�́A�������Ȃ� */
	if ((cx == 0) && (cy == 0)) {
		return;
	}

	/* �T�C�Y�ύX */
	width = 54;
	nHeightDisAsm[nCPU] = (WORD)(cy / lCharHeight);
	if (nHeightDisAsm[nCPU] < 1) {
		nHeightDisAsm[nCPU] = 1;
	}

	/* �E�B���h�E��`�ύX */
	GetWindowRect(hWnd, &wrect);
	GetClientRect(hWnd, &crect);
	wrect.right -= wrect.left;
	wrect.right -= crect.right;
	wrect.right += (width * lCharWidth);
	wrect.bottom -= wrect.top;
	wrect.bottom -= crect.bottom;
	wrect.bottom += (nHeightDisAsm[nCPU] * lCharHeight);
	SetWindowPos(hWnd, HWND_TOP, 0, 0, wrect.right, wrect.bottom,
							SWP_NOZORDER | SWP_NOMOVE);
}

/*
 *	�t�A�Z���u���E�C���h�E
 *	�Z�b�g�A�b�v
 */
static void FASTCALL SetupDisAsm(BYTE nCPU, BYTE *p, int x, int y)
{
	char string[128];
	char tmp[16];
	int addr;
	int i;
	int j;
	int k;
	int ret;
	WORD pc;

	ASSERT(nCPU < MAXCPU);
	ASSERT(p);
	ASSERT(x > 0);
	ASSERT(y > 0);

	/* ��U�X�y�[�X�Ŗ��߂� */
	memset(p, 0x20, x * y);

	/* �����ݒ� */
	switch (nCPU) {
		case MAINCPU :	pc = maincpu.pc;
						break;
		case SUBCPU :	pc = subcpu.pc;
						break;
#if XM7_VER == 1
#if defined(JSUB)
		case JSUBCPU :	pc = jsubcpu.pc;
						break;
#endif
#if defined(Z80CARD)
		case MAINZ80 :	pc = (WORD)mainz80.pc;
						break;
#endif
#endif
	}
	addr = (int)dwDisAsm[nCPU];

	for (i=0; i<y; i++) {
		 /* �t�A�Z���u�� */
#if XM7_VER == 1 && defined(Z80CARD)
		if (nCPU == MAINZ80) {
			ret = disline80(nCPU, (WORD)addr, string);
		}
		else {
			ret = disline(nCPU, (WORD)addr, string);
		}
#else
		ret = disline(nCPU, (WORD)addr, string);
#endif

		/* �Z�b�g */
#if XM7_VER == 1 && defined(Z80CARD)
		if (((nCPU == MAINCPU) || (nCPU == MAINZ80)) &&
			 isLinearAddrMode(bDisPhysicalAddr)) {
#else
		if ((nCPU == MAINCPU) && isLinearAddrMode(bDisPhysicalAddr)) {
#endif
			memcpy(&p[x * i + 9], string, strlen(string));
			_snprintf(tmp, sizeof(tmp), "%05X", mmr_trans_mmr((WORD)addr));
			memcpy(&p[x * i + 3], tmp, 5);
		}
		else {
			memcpy(&p[x * i + 3], string, strlen(string));
		}

		/* �}�[�N */
		if (pc == addr) {
			p[x * i + 2] = '>';
		}
		for (j=0; j<BREAKP_MAXNUM; j++) {
			if (breakp[j].addr != addr) {
				continue;
			}
			if (breakp[j].cpu != nCPU) {
				continue;
			}
			if (breakp[j].flag == BREAKP_NOTUSE) {
				continue;
			}
			if (breakp[j].flag == BREAKP_DISABLED) {
				continue;
			}

			/* �u���[�N�|�C���g */
			_snprintf(tmp, sizeof(tmp), "%2d", j + 1);
			memcpy(&p[x * i + 0], tmp, 2);

			/* ���] */
			for (k=0; k<x; k++) {
				 p[x * i + k] = (BYTE)(p[x * i + k] | 0x80);
			}
		}

		/* ���Z�A�I�[�o�[�`�F�b�N */
		addr += ret;
		if (addr >= 0x10000) {
			break;
		}
	}
}

/*
 *	�t�A�Z���u���E�C���h�E
 *	�`��
 */
static void FASTCALL DrawDisAsm(HWND hWnd, HDC hDC)
{
	int i;
	BYTE nCPU;
	RECT rect;
	BYTE *p;
	int x, y;

	ASSERT(hWnd);
	ASSERT(hDC);

	/* Draw�o�b�t�@�𓾂�(���݂��Ȃ���Ή������Ȃ�) */
	for (i=0; i<SWND_MAXNUM; i++) {
		if (hSubWnd[i] == hWnd) {
			nCPU = (BYTE)((i - SWND_DISASM_MAIN) % MAXCPU);
			p = pDisAsm[nCPU];
			break;
		}
	}
	if (!p) {
		return;
	}

	/* �E�C���h�E�W�I���g���𓾂� */
	GetClientRect(hWnd, &rect);
	x = rect.right / lCharWidth;
	y = rect.bottom / lCharHeight;
	if ((x == 0) || (y == 0)) {
		return;
	}

	/* �Z�b�g�A�b�v */
	SetupDisAsm(nCPU, p, x, y);

	/* �`�� */
	DrawWindowText2(hDC, p, x, y);
}

/*
 *	�t�A�Z���u���E�C���h�E
 *	���t���b�V��
 */
void FASTCALL RefreshDisAsm(void)
{
	HWND hWnd;
	HDC hDC;

	/* ���C��CPU */
	if (hSubWnd[SWND_DISASM_MAIN]) {
		hWnd = hSubWnd[SWND_DISASM_MAIN];
		hDC = GetDC(hWnd);
		DrawDisAsm(hWnd, hDC);
		ReleaseDC(hWnd, hDC);
	}

	/* �T�uCPU */
	if (hSubWnd[SWND_DISASM_SUB]) {
		hWnd = hSubWnd[SWND_DISASM_SUB];
		hDC = GetDC(hWnd);
		DrawDisAsm(hWnd, hDC);
		ReleaseDC(hWnd, hDC);
	}

#if XM7_VER == 1
#if defined(JSUB)
	/* ���{��T�uCPU */
	if (hSubWnd[SWND_DISASM_JSUB]) {
		hWnd = hSubWnd[SWND_DISASM_JSUB];
		hDC = GetDC(hWnd);
		DrawDisAsm(hWnd, hDC);
		ReleaseDC(hWnd, hDC);
	}
#endif

#if defined(JSUB)
	/* ���C��CPU(Z80) */
	if (hSubWnd[SWND_DISASM_Z80]) {
		hWnd = hSubWnd[SWND_DISASM_Z80];
		hDC = GetDC(hWnd);
		DrawDisAsm(hWnd, hDC);
		ReleaseDC(hWnd, hDC);
	}
#endif
#endif
}

/*
 *	�t�A�Z���u���E�C���h�E
 *	�ĕ`��
 */
static void FASTCALL PaintDisAsm(HWND hWnd)
{
	HDC hDC;
	PAINTSTRUCT ps;
	int i;
	RECT rect;
	BYTE *p;
	int x, y;

	ASSERT(hWnd);

	/* Draw�o�b�t�@�𓾂�(���݂��Ȃ���Ή������Ȃ�) */
	for (i=0; i<SWND_MAXNUM; i++) {
		if (hSubWnd[i] == hWnd) {
			p = pDisAsm[((i - SWND_DISASM_MAIN) % MAXCPU)];
			break;
		}
	}

	if (!p) {
		return;
	}

	/* �E�C���h�E�W�I���g���𓾂� */
	GetClientRect(hWnd, &rect);
	x = rect.right / lCharWidth;
	y = rect.bottom / lCharHeight;

	/* �㔼�G���A��FF�Ŗ��߂� */
	if ((x > 0) && (y > 0)) {
		memset(&p[x * y], 0xff, x * y);
	}

	/* �`�� */
	hDC = BeginPaint(hWnd, &ps);
	ASSERT(hDC);
	DrawDisAsm(hWnd, hDC);
	EndPaint(hWnd, &ps);
}

/*
 *	�t�A�Z���u���E�C���h�E
 *	���N���b�N
 */
static void FASTCALL LButtonDisAsm(HWND hWnd, POINT point, BOOL mode)
{
	char string[128];
	int addr;
	BOOL flag;
	BYTE nCPU;
	int i;
	int y;
	int ret;

	ASSERT(hWnd);

	/* �s�J�E���g�𓾂� */
	y = point.y / lCharHeight;

	/* ���ۂɋt�A�Z���u�����Ă݂� */
	for (i=0; i<SWND_MAXNUM; i++) {
		if (hSubWnd[i] == hWnd) {
			nCPU = (BYTE)((i - SWND_CPUREG_MAIN) % MAXCPU);
			addr = (int)dwDisAsm[nCPU];
			break;
		}
	}

	/* �t�A�Z���u�� ���[�v */
	for (i=0; i<y; i++) {
#if XM7_VER == 1 && defined(Z80CARD)
		if (nCPU == MAINZ80) {
			ret = disline80(nCPU, (WORD)addr, string);
		}
		else {
			ret = disline(nCPU, (WORD)addr, string);
		}
#else
		ret = disline(nCPU, (WORD)addr, string);
#endif
		addr += ret;
		if (addr >= 0x10000) {
			return;
		}
	}

	if (mode) {
		/* �����W�����v */
		switch (nCPU) {
			case MAINCPU :	maincpu.pc = (WORD)addr;
							break;
			case SUBCPU :	subcpu.pc = (WORD)addr;
							break;
#if XM7_VER == 1
#if defined(JSUB)
			case JSUBCPU :	jsubcpu.pc = (WORD)addr;
							break;
#endif
#if defined(Z80CARD)
			case MAINZ80 :	mainz80.pc = (Uint32)addr;
							break;
#endif
#endif
		}
	}
	else {
		/* �u���[�N�|�C���g on/off */
		flag = FALSE;
		for (i=0; i<BREAKP_MAXNUM; i++) {
			if ((breakp[i].cpu == nCPU) && (breakp[i].addr == addr)) {
				if (breakp[i].flag != BREAKP_NOTUSE) {
					breakp[i].flag = BREAKP_NOTUSE;
					flag = TRUE;
					break;
				}
			}
		}
		if (!flag) {
			schedule_setbreak(nCPU, (WORD)addr);
		}

		/* �u���[�N�|�C���g�E�C���h�E���A�ĕ`�悷�� */
		if (hSubWnd[SWND_BREAKPOINT]) {
			InvalidateRect(hSubWnd[SWND_BREAKPOINT], NULL, FALSE);
		}
	}

	/* �ĕ`�� */
	InvalidateRect(hWnd, NULL, FALSE);
}

/*
 *	�t�A�Z���u���E�C���h�E
 *	�R�}���h����
 */
static void FASTCALL CmdDisAsm(HWND hWnd, WORD wID, BYTE nCPU)
{
	DWORD target;
	cpu6809_t *cpu;

	/* CPU�\���̌��� */
	switch (nCPU) {
#if XM7_VER == 1 && defined(Z80CARD)
		case MAINZ80 :
#endif
		case MAINCPU :	cpu = &maincpu;
						break;
		case SUBCPU :	cpu = &subcpu;
						break;
#if XM7_VER == 1
#if defined(JSUB)
		case JSUBCPU :	cpu = &jsubcpu;
						break;
#endif
#endif
	}

	/* �^�[�Q�b�g�A�h���X���� */
	switch (wID) {
		case IDM_DIS_ADDR:
#if XM7_VER == 1 && defined(Z80CARD)
			if (((nCPU == MAINCPU) || (nCPU == MAINZ80)) &&
				 isLinearAddrMode(bDisPhysicalAddr)) {
#else
			if ((nCPU == MAINCPU) && isLinearAddrMode(bDisPhysicalAddr)) {
#endif
				target = mmr_trans_mmr((WORD)dwDisAsm[nCPU]);
				if (!AddrDlg(hWnd, &target, nCPU, TRUE)) {
					return;
				}
				target = mmr_trans_phys_to_logi(target);
			}
			else {
				target = dwDisAsm[nCPU];
				if (!AddrDlg(hWnd, &target, nCPU, FALSE)) {
					return;
				}
				target &= (WORD)0xffff;
			}
			break;
		case IDM_DIS_PHYSADDR:
#if XM7_VER == 1 && defined(Z80CARD)
			if ((nCPU == MAINCPU) || (nCPU == MAINZ80)) {
#else
			if (nCPU == MAINCPU) {
#endif
				bDisPhysicalAddr = !bDisPhysicalAddr;
			}
			return;

		case IDM_DIS_PC:
			target = cpu->pc;
			break;
		case IDM_DIS_X:
			target = cpu->x;
			break;
		case IDM_DIS_Y:
			target = cpu->y;
			break;
		case IDM_DIS_U:
			target = cpu->u;
			break;
		case IDM_DIS_S:
			target = cpu->s;
			break;
#if XM7_VER == 1 && defined(Z80CARD)
		case IDM_DIS_PC_Z80:
			target = mainz80.pc;
			break;
		case IDM_DIS_BC:
			target = (WORD)((mainz80.regs8[REGID_B]) << 8 |
				mainz80.regs8[REGID_C]);
			break;
		case IDM_DIS_DE:
			target = (WORD)((mainz80.regs8[REGID_D]) << 8 |
				mainz80.regs8[REGID_E]);
			break;
		case IDM_DIS_HL:
			target = (WORD)((mainz80.regs8[REGID_H]) << 8 |
				mainz80.regs8[REGID_L]);
			break;
		case IDM_DIS_IX:
			target = (WORD)((mainz80.regs8[REGID_IXH]) << 8 |
				mainz80.regs8[REGID_IXL]);
			break;
		case IDM_DIS_IY:
			target = (WORD)((mainz80.regs8[REGID_IYH]) << 8 |
				mainz80.regs8[REGID_IYL]);
			break;
		case IDM_DIS_SP:
			target = mainz80.sp;
			break;
#endif

		case IDM_DIS_RESET:
			target = (WORD)((cpu->readmem(0xfffe) << 8) | cpu->readmem(0xffff));
			break;
		case IDM_DIS_NMI:
			target = (WORD)((cpu->readmem(0xfffc) << 8) | cpu->readmem(0xfffd));
			break;
		case IDM_DIS_SWI:
			target = (WORD)((cpu->readmem(0xfffa) << 8) | cpu->readmem(0xfffb));
			break;
		case IDM_DIS_IRQ:
			target = (WORD)((cpu->readmem(0xfff8) << 8) | cpu->readmem(0xfff9));
			break;
		case IDM_DIS_FIRQ:
			target = (WORD)((cpu->readmem(0xfff6) << 8) | cpu->readmem(0xfff7));
			break;
		case IDM_DIS_SWI2:
			target = (WORD)((cpu->readmem(0xfff4) << 8) | cpu->readmem(0xfff5));
			break;
		case IDM_DIS_SWI3:
			target = (WORD)((cpu->readmem(0xfff2) << 8) | cpu->readmem(0xfff3));
			break;
		case IDM_DIS_JUMP:
			LButtonDisAsm(hWnd, PosDisAsmPoint, TRUE);
#if XM7_VER == 1 && defined(Z80CARD)
			if (nCPU == MAINZ80) {
				target = (WORD)mainz80.pc;
			}
			else {
				target = cpu->pc;
			}
#else
			target = cpu->pc;
#endif
			break;
		case IDM_DIS_SYNCPC:
			bSyncDisasm[nCPU] = (!bSyncDisasm[nCPU]);
			return;
		default:
			if ((wID >= IDM_DIS_STACK) && (wID <= IDM_DIS_STACK + 15)) {
				target = (WORD)wStackAddr[wID - IDM_DIS_STACK];
			}
			else if ((wID >= IDM_DIS_BREAK) && (wID <= IDM_DIS_BREAK + 15)) {
				target = (WORD)wBreakAddr[wID - IDM_DIS_BREAK];
			}
			break;
	}

	/* �ݒ聕�X�V */
	AddrDisAsm(nCPU, (WORD)target);
}

/*
 *	�t�A�Z���u���E�C���h�E
 *	�E�C���h�E�v���V�[�W��
 */
static LRESULT CALLBACK DisAsmProc(HWND hWnd, UINT message,
								 WPARAM wParam, LPARAM lParam, BYTE nCPU)
{
	static BOOL bResize = FALSE;	/* �蓮���T�C�Y�t���O */
	DWORD dwAddr;
	char string[128];
	int ret;
	int i;
	POINT point;
	HMENU hMenu;
	RECT rect;

	/* ���b�Z�[�W�� */
	switch (message) {
		/* �E�C���h�E�ĕ`�� */
		case WM_PAINT:
			/* ���b�N���K�v */
			LockVM();
			PaintDisAsm(hWnd);
			UnlockVM();
			return 0;

		/* �E�C���h�E�폜 */
		case WM_DESTROY:
			LockVM();

			/* ���C���E�C���h�E�֎����ʒm */
			DestroySubWindow(hWnd, &pDisAsm[nCPU], hDisAsm[nCPU]);

			UnlockVM();
			break;

		/* �E�C���h�E�T�C�Y�ύX */
		case WM_SIZE:
			if (bResize) {
				bResize = FALSE;
				ResizeDisAsm(hWnd, LOWORD(lParam), HIWORD(lParam), nCPU);
			}
			break;

		/* �E�C���h�E�T�C�Y�ύX�� */
		case WM_SIZING:
			WindowSizing(hWnd, (LPRECT)lParam, &pDisAsm[nCPU]);
			bResize = TRUE;
			break;

		/* �E�B���h�E�T�C�Y�ύX���b�Z�[�W */
		case WM_WINDOWPOSCHANGING:
			bResize = TRUE;
			break;

		/* �ŏ��T�C�Y���� */
		case WM_GETMINMAXINFO:
			rect.left = 0;
			rect.right = 54 * lCharWidth + GetSystemMetrics(SM_CXVSCROLL);
			rect.top = 0;
			rect.bottom = lCharHeight;
			if (bPopupSwnd) {
				AdjustWindowRect(&rect, WS_POPUP | WS_OVERLAPPED | WS_SYSMENU |
										WS_CAPTION | WS_VISIBLE | 
										WS_MINIMIZEBOX | WS_SIZEBOX, FALSE);
			}
			else {
				AdjustWindowRect(&rect, WS_CHILD | WS_OVERLAPPED | WS_SYSMENU |
										WS_CAPTION | WS_VISIBLE | 
										WS_MINIMIZEBOX | WS_CLIPSIBLINGS | 
										WS_SIZEBOX, FALSE);
			}
			((LPMINMAXINFO)lParam)->ptMinTrackSize.x = rect.right - rect.left;
			((LPMINMAXINFO)lParam)->ptMaxTrackSize.x = rect.right - rect.left;
			((LPMINMAXINFO)lParam)->ptMinTrackSize.y = rect.bottom - rect.top;
			return 0;

		/* �R���e�L�X�g���j���[ */
		case WM_RBUTTONDOWN:
#if defined(MOUSE)
			/* �}�E�X�G�~�����[�V�����g�p���͖��� */
			if (mos_capture && !stopreq_flag && run_flag) {
				return FALSE;
			}
#endif

			/* �R���e�L�X�g���j���[�����[�h */
			hMenu = GetSubMenu(hDisAsm[nCPU], 0);
			CheckMenuSub(hMenu, IDM_DIS_SYNCPC, bSyncDisasm[nCPU]);
			EnableMenuSub(hMenu, IDM_DIS_SYNCPC, bSync);

			/* �����A�h���X���[�h�ݒ� */
#if XM7_VER >= 2
			if ((nCPU == MAINCPU) && (fm7_ver >= 2)) {
#else
#if XM7_VER == 1 && defined(Z80CARD)
			if (((nCPU == MAINCPU) || (nCPU == MAINZ80)) &&
				 (fm_subtype == FMSUB_FM77)) {
#else
			if ((nCPU == MAINCPU) && (fm_subtype == FMSUB_FM77)) {
#endif
#endif
				CheckMenuSub(hMenu, IDM_DIS_PHYSADDR, bDisPhysicalAddr);
				EnableMenuSub(hMenu, IDM_DIS_PHYSADDR, TRUE);
			}
			else {
				CheckMenuSub(hMenu, IDM_DIS_PHYSADDR, FALSE);
				EnableMenuSub(hMenu, IDM_DIS_PHYSADDR, FALSE);
			}

#if XM7_VER == 1 && defined(Z80CARD)
			/* ���荞�݃x�N�^���j���[OFF */
			if (nCPU == MAINZ80) {
				EnableMenuItem(hMenu, 6, MF_BYPOSITION | MF_GRAYED);
			}
#endif

			/* �V�X�e���X�^�b�N�W�����v���j���[�ݒ� */
			InsertStackJumpMenu(GetSubMenu(hMenu, 7), nCPU, IDM_DIS_STACK);

			/* �u���[�N�|�C���g���j���[�ݒ� */
			InsertBreakPointMenu(GetSubMenu(hMenu, 8), nCPU, IDM_DIS_BREAK);

			/* �R���e�L�X�g���j���[�����s */
			point.x = LOWORD(lParam);
			point.y = HIWORD(lParam);
			PosDisAsmPoint = point;
			ClientToScreen(hWnd, &point);
			TrackPopupMenu(hMenu, 0, point.x, point.y, 0, hWnd, NULL);
			return 0;

		/* ���N���b�N�E�_�u���N���b�N */
		case WM_LBUTTONDOWN:
		case WM_LBUTTONDBLCLK:
#if defined(MOUSE)
			/* �}�E�X�G�~�����[�V�����g�p���͖��� */
			if (mos_capture && !stopreq_flag && run_flag) {
				return FALSE;
			}
#endif

			point.x = LOWORD(lParam);
			point.y = HIWORD(lParam);
			LButtonDisAsm(hWnd, point, FALSE);
			break;

		/* �R�}���h */
		case WM_COMMAND:
			CmdDisAsm(hWnd, LOWORD(wParam), nCPU);
			break;

		/* �����X�N���[���o�[ */
		case WM_VSCROLL:
			/* �^�C�v���� */
			dwAddr = dwDisAsm[nCPU];

			/* �A�N�V������ */
			switch (LOWORD(wParam)) {
				/* �g�b�v */
				case SB_TOP:
					dwAddr = 0;
					break;
				/* �I�[ */
				case SB_BOTTOM:
					dwAddr = 0xffff;
					break;
				/* �P�s�� */
				case SB_LINEUP:
					if (dwAddr > 0) {
						dwAddr--;
					}
					break;
				/* �P�s��(�����͍H�v) */
				case SB_LINEDOWN:
#if XM7_VER == 1 && defined(Z80CARD)
					if (nCPU == MAINZ80) {
						ret = disline80(nCPU, (WORD)dwAddr, string);
					}
					else {
						ret = disline(nCPU, (WORD)dwAddr, string);
					}
#else
					ret = disline(nCPU, (WORD)dwAddr, string);
#endif
					i = (int)dwAddr;
					i += ret;
					if (i < 0x10000) {
						dwAddr += (WORD)ret;
					}
					break;
				/* �y�[�W�A�b�v */
				case SB_PAGEUP:
					if (dwAddr < 0x80) {
						dwAddr = 0;
					}
					else {
						dwAddr -= (WORD)0x80;
					}
					break;
				/* �y�[�W�_�E�� */
				case SB_PAGEDOWN:
					if (dwAddr >= 0xff80) {
						dwAddr = 0xffff;
					}
					else {
						dwAddr += (WORD)0x80;
					}
					break;
				/* ���ڎw�� */
				case SB_THUMBTRACK:
					dwAddr = HIWORD(wParam);
					if (dwAddr >= 0xffff) {
						dwAddr = 0xffff;
					}
					break;
			}
			AddrDisAsm(nCPU, dwAddr);
			RefreshDisAsm();
			break;
	}

	/* �f�t�H���g �E�C���h�E�v���V�[�W�� */
	return DefWindowProc(hWnd, message, wParam, lParam);
}

/*
 *	�t�A�Z���u���E�C���h�E(���C��)
 *	�E�C���h�E�v���V�[�W��
 */
static LRESULT CALLBACK DisAsmProcMain(HWND hWnd, UINT message,
								 WPARAM wParam, LPARAM lParam)
{
	return DisAsmProc(hWnd, message, wParam, lParam, MAINCPU);
}

/*
 *	�t�A�Z���u���E�C���h�E(�T�u)
 *	�E�C���h�E�v���V�[�W��
 */
static LRESULT CALLBACK DisAsmProcSub(HWND hWnd, UINT message,
								 WPARAM wParam, LPARAM lParam)
{
	return DisAsmProc(hWnd, message, wParam, lParam, SUBCPU);
}

#if XM7_VER == 1
/*
 *	�t�A�Z���u���E�C���h�E(���{��T�u)
 *	�E�C���h�E�v���V�[�W��
 */
#if defined(JSUB)
static LRESULT CALLBACK DisAsmProcJsub(HWND hWnd, UINT message,
								 WPARAM wParam, LPARAM lParam)
{
	return DisAsmProc(hWnd, message, wParam, lParam, JSUBCPU);
}
#endif

/*
 *	�t�A�Z���u���E�C���h�E(���C��Z80)
 *	�E�C���h�E�v���V�[�W��
 */
#if defined(Z80CARD)
static LRESULT CALLBACK DisAsmProcZ80(HWND hWnd, UINT message,
								 WPARAM wParam, LPARAM lParam)
{
	return DisAsmProc(hWnd, message, wParam, lParam, MAINZ80);
}
#endif
#endif

/*
 *	�t�A�Z���u���E�C���h�E
 *	�E�C���h�E�쐬
 */
HWND FASTCALL CreateDisAsm(HWND hParent, BYTE nCPU, int index)
{
	WNDCLASSEX wcex;
	char szClassNameMain[] = "XM7_DisAsmMain";
	char szClassNameSub[] = "XM7_DisAsmSub";
#if XM7_VER == 1
#if defined(JSUB)
	char szClassNameJsub[] = "XM7_DisAsmJsub";
#endif
#if defined(Z80CARD)
	char szClassNameZ80[] = "XM7_DisAsmZ80";
#endif
#endif
	char *szClassName;
	char szWndName[128];
	RECT rect;
	RECT crect, wrect;
	HWND hWnd;
	SCROLLINFO si;
	UINT id;
	WORD pc;
	DWORD dwStyle;

	ASSERT(hParent);

	/* �E�C���h�E��`���v�Z */
	PositioningSubWindow(hParent, &rect, index);
	rect.right = lCharWidth * 54;

	/* �E�C���h�E�^�C�g��������A�o�b�t�@�m�ہA���j�����[�h */
	switch (nCPU) {
		case MAINCPU :	id = IDS_SWND_DISASM_MAIN;
						pc = maincpu.pc;
						szClassName = szClassNameMain;
						break;
		case SUBCPU :	id = IDS_SWND_DISASM_SUB;
						pc = subcpu.pc;
						szClassName = szClassNameSub;
						break;
#if XM7_VER == 1
#if defined(JSUB)
		case JSUBCPU :	id = IDS_SWND_DISASM_JSUB;
						pc = jsubcpu.pc;
						szClassName = szClassNameJsub;
						break;
#endif
#if defined(Z80CARD)
		case MAINZ80 :	id = IDS_SWND_DISASM_Z80;
						pc = (WORD)mainz80.pc;
						szClassName = szClassNameZ80;
						break;
#endif
#endif
	}
	rect.bottom = lCharHeight * nHeightDisAsm[nCPU];
	LoadString(hAppInstance, id, szWndName, sizeof(szWndName));
	pDisAsm[nCPU] = malloc(2 * (rect.right / lCharWidth) * nHeightDisAsm[nCPU]);
	dwDisAsm[nCPU] = pc;
#if XM7_VER == 1 && defined(Z80CARD)
	if (nCPU == MAINZ80) {
		hDisAsm[nCPU] = LoadMenu(hAppInstance, MAKEINTRESOURCE(IDR_DISASMZ80MENU));
	}
	else {
		hDisAsm[nCPU] = LoadMenu(hAppInstance, MAKEINTRESOURCE(IDR_DISASMMENU));
	}
#else
	hDisAsm[nCPU] = LoadMenu(hAppInstance, MAKEINTRESOURCE(IDR_DISASMMENU));
#endif

	/* �E�C���h�E�N���X�̓o�^ */
	memset(&wcex, 0, sizeof(wcex));
	wcex.cbSize = sizeof(wcex);
	wcex.style = CS_VREDRAW | CS_HREDRAW;
	switch (nCPU) {
		case MAINCPU :	wcex.lpfnWndProc = DisAsmProcMain;
						break;
		case SUBCPU :	wcex.lpfnWndProc = DisAsmProcSub;
						break;
#if XM7_VER == 1
#if defined(JSUB)
		case JSUBCPU :	wcex.lpfnWndProc = DisAsmProcJsub;
						break;
#endif
#if defined(Z80CARD)
		case MAINZ80 :	wcex.lpfnWndProc = DisAsmProcZ80;
						break;
#endif
#endif
	}
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hAppInstance;
	wcex.hIcon = LoadIcon(hAppInstance, MAKEINTRESOURCE(IDI_WNDICON));
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	wcex.lpszMenuName = NULL;
	wcex.lpszClassName = szClassName;
	wcex.hIconSm = LoadIcon(hAppInstance, MAKEINTRESOURCE(IDI_WNDICON));
	RegisterClassEx(&wcex);

	/* �E�C���h�E�쐬 */
	if (bPopupSwnd) {
		dwStyle =	WS_POPUP | WS_OVERLAPPED | WS_SYSMENU | WS_CAPTION |
					WS_VISIBLE | WS_MINIMIZEBOX | WS_VSCROLL | WS_SIZEBOX;
	}
	else {
		dwStyle =	WS_CHILD | WS_OVERLAPPED | WS_SYSMENU | WS_CAPTION |
					WS_VISIBLE | WS_MINIMIZEBOX | WS_CLIPSIBLINGS |
					WS_VSCROLL | WS_SIZEBOX;
	}
	hWnd = CreateWindow(szClassName,
						szWndName,
						dwStyle,
						rect.left,
						rect.top,
						rect.right,
						rect.bottom,
						hParent,
						NULL,
						hAppInstance,
						NULL);

	/* �L���Ȃ�A�T�C�Y�␳���Ď�O�ɒu�� */
	if (hWnd) {
		GetWindowRect(hWnd, &wrect);
		GetClientRect(hWnd, &crect);
		wrect.right += (rect.right - crect.right);
		wrect.bottom += (rect.bottom - crect.bottom);
		SetWindowPos(hWnd, HWND_TOP, wrect.left, wrect.top,
			wrect.right - wrect.left, wrect.bottom - wrect.top, SWP_NOMOVE);

		/* �X�N���[���o�[�̐ݒ肪�K�v */
		memset(&si, 0, sizeof(si));
		si.cbSize = sizeof(si);
		si.fMask = SIF_ALL;
		si.nMin = 0;
		si.nMax = 0x100fe;
		si.nPage = 0x100;
		si.nPos = pc;
		SetScrollInfo(hWnd, SB_VERT, &si, TRUE);
	}

	/* �|�b�v�A�b�v�E�C���h�E���̓A�N�e�B�u�E�C���h�E��O�ʂɕύX */
	if (bPopupSwnd) {
		SetForegroundWindow(hMainWnd);
	}

	/* ���ʂ������A�� */
	return hWnd;
}

/*-[ ���������e�ύX�_�C�A���O ]----------------------------------------------*/

/*
 *	���������e�ύX�_�C�A���O
 *	�_�C�A���O������
 */
static BOOL FASTCALL MemoryChangeDlgInit(HWND hDlg)
{
	HWND hWnd;
	RECT prect;
	RECT drect;
	char tmp[16];
	char string[128];
	UINT id;

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

	/* �A�h���X��ݒ� */
	hWnd = GetDlgItem(hDlg, IDC_MEMLABEL);
	ASSERT(hWnd);
	switch (nMemDlgCPU) {
		case MAINCPU :	if (isLinearAddrMode(bDumpPhysicalAddr)) {
							id = IDS_SWND_CPU_MAIN_P;
						}
						else {
							id = IDS_SWND_CPU_MAIN;
						}
						break;
		case SUBCPU :	id = IDS_SWND_CPU_SUB;
						break;
#if XM7_VER == 1
#if defined(JSUB)
		case JSUBCPU :	id = IDS_SWND_CPU_JSUB;
						break;
#endif
#endif
	}
	LoadString(hAppInstance, id, tmp, sizeof(tmp));
	if ((nMemDlgCPU == MAINCPU) && isLinearAddrMode(bDumpPhysicalAddr)) {
		_snprintf(string, sizeof(string), "$%05X (%s)",
			dwMemDlgAddr & (GetMaxMemArea(bDumpPhysicalAddr) - 1), tmp);
	}
	else {
		_snprintf(string, sizeof(string), "$%04X (%s)",
			dwMemDlgAddr & 0xffff, tmp);
	}
	SetWindowText(hWnd, string);

	/* ���f�[�^��ݒ� */
	hWnd = GetDlgItem(hDlg, IDC_MEMEDIT);
	ASSERT(hWnd);
	_snprintf(string, sizeof(string), "%02X", nMemDlgByte);
	SetWindowText(hWnd, string);

	return TRUE;
}

/*
 *	���������e�ύX�_�C�A���O
 *	�_�C�A���O�v���V�[�W��
 */
static BOOL CALLBACK MemoryChangeDlgProc(HWND hDlg, UINT iMsg,
									WPARAM wParam, LPARAM lParam)
{
	HWND hWnd;
	char string[128];

	UNUSED(lParam);

	switch (iMsg) {
		/* �_�C�A���O������ */
		case WM_INITDIALOG:
			return MemoryChangeDlgInit(hDlg);

		/* �R�}���h���� */
		case WM_COMMAND:
			switch (LOWORD(wParam)) {
				/* OK�E�L�����Z�� */
				case IDOK:
				case IDCANCEL:
					if (LOWORD(wParam) == IDOK) {
						/* ���݂̒l���擾 */
						hWnd = GetDlgItem(hDlg, IDC_MEMEDIT);
						ASSERT(hWnd);
						GetWindowText(hWnd, string, sizeof(string) - 1);
						if (string[0] == 0x27) {
							nMemDlgByte = (BYTE)string[1];
						}
						else {
							nMemDlgByte = (BYTE)strtol(string, NULL, 16);
						}
					}
					EndDialog(hDlg, LOWORD(wParam));
					InvalidateRect(hDrawWnd, NULL, FALSE);
					return TRUE;
			}
			break;
	}

	/* ����ȊO�́AFALSE */
	return FALSE;
}

/*
 *	���������e�ύX
 */
static BOOL FASTCALL MemoryChange(HWND hWnd, BYTE nCPU, int x, int y)
{
	BYTE (FASTCALL *readbnio)(WORD addr);
	void (FASTCALL *writeb)(WORD addr, BYTE dat);
	int height;
	int ret;

	ASSERT(hWnd);

	/* �p�����[�^��ۑ��E�ݒ� */
	switch (nCPU) {
		case MAINCPU :	readbnio = mainmem_readbnio;
						writeb = mainmem_writeb;
						break;
		case SUBCPU :	readbnio = submem_readbnio;
						writeb = submem_writeb;
						break;
#if XM7_VER == 1
#if defined(JSUB)
		case JSUBCPU :	readbnio = jsubmem_readbnio;
						writeb = jsubmem_writeb;
						break;
#endif
#endif
	}
	ASSERT(readbnio);
	ASSERT(writeb);

	nMemDlgCPU = nCPU;
	height = nHeightMemory[nCPU];
	if (bAsciiDump[nCPU]) {
		height -= 3;
	}

	/* �A�h���X���Z�b�g */
	if ((x < 7) || (x > 54) || (((x - 7) % 3) == 2) ||
		(y < 0) || (y >= height)) {
		/* ���W���͈͊O */
		return FALSE;
	}

	/* �A�h���X�ƌ��f�[�^���擾 */
	if ((nMemDlgCPU == MAINCPU) && isLinearAddrMode(bDumpPhysicalAddr)) {
		dwMemDlgAddr  = (DWORD)(dwMemory[nCPU] & 0xffff0);
		dwMemDlgAddr += (DWORD)((x - 7) / 3);
		dwMemDlgAddr += (DWORD)(y * 16);

		/* ���f�[�^���Z�b�g */
		nMemDlgByte = mainmem_readbnio_p(dwMemDlgAddr);
	}
	else {
		dwMemDlgAddr  = (WORD)(dwMemory[nCPU] & 0xfff0);
		dwMemDlgAddr += (WORD)((x - 7) / 3);
		dwMemDlgAddr += (WORD)(y * 16);

		/* ���f�[�^���Z�b�g */
		nMemDlgByte = readbnio((WORD)dwMemDlgAddr);
	}


	/* ���[�_���_�C�A���O���s */
	ret = DialogBox(hAppInstance, MAKEINTRESOURCE(IDD_MEMDLG), hWnd, MemoryChangeDlgProc);
	if (ret != IDOK) {
		return FALSE;
	}

	/* �f�[�^���Z�b�g */
	LockVM();
	if ((nMemDlgCPU == MAINCPU) && isLinearAddrMode(bDumpPhysicalAddr)) {
		mainmem_writeb_p(dwMemDlgAddr, (BYTE)nMemDlgByte);
	}
	else {
		writeb((WORD)dwMemDlgAddr, (BYTE)nMemDlgByte);
	}
	InvalidateRect(hWnd, NULL, FALSE);
	UnlockVM();

	return TRUE;
}

/*-[ �����������_�C�A���O ]--------------------------------------------------*/

/*
 *	�����������_�C�A���O
 *	�_�C�A���O������
 */
static BOOL FASTCALL MemorySearchDlgInit(HWND hDlg)
{
	HWND hWnd;
	RECT prect;
	RECT drect;
	char string[128];
	DWORD addr;

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

	/* �R���{�{�b�N�X���� */
	hWnd = GetDlgItem(hDlg, IDC_SEARCHADDR);
	ASSERT(hWnd);
	InsertHistory(hWnd, bDumpPhysicalAddr);

	/* �A�h���X��ݒ� */
	addr = dwMemSrchAddrSave[nMemSrchCPU];
	if ((nMemSrchCPU == MAINCPU) && isLinearAddrMode(bDumpPhysicalAddr)) {
		_snprintf(string, sizeof(string), "%05X",
			addr & (GetMaxMemArea(bDumpPhysicalAddr) - 1));
	}
	else {
		_snprintf(string, sizeof(string), "%04X", addr & 0xffff);
	}
	ComboBox_SetText(hWnd, string);

	/* �����������ݒ� */
	hWnd = GetDlgItem(hDlg, IDC_SEARCHDATA);
	ASSERT(hWnd);
	SetWindowText(hWnd, (LPSTR)szMemSrchString);

	return TRUE;
}

/*
 *	�����������_�C�A���O
 *	�_�C�A���OOK
 */
static void FASTCALL MemorySearchDlgOK(HWND hDlg)
{
	HWND hWnd;
	char string[128];

	ASSERT(hDlg);

	/* ���݂̒l���擾 */
	hWnd = GetDlgItem(hDlg, IDC_SEARCHDATA);
	ASSERT(hWnd);
	GetWindowText(hWnd, szMemSrchString, sizeof(szMemSrchString) - 1);

	/* �R���{�{�b�N�X���� */
	hWnd = GetDlgItem(hDlg, IDC_SEARCHADDR);
	ASSERT(hWnd);

	/* ���݂̒l���擾 */
	ComboBox_GetText(hWnd, string, sizeof(string) - 1);
	dwMemSrchAddr = (DWORD)strtol(string, NULL, 16);
	if ((nMemSrchCPU == MAINCPU) && isLinearAddrMode(bDumpPhysicalAddr)) {
		dwMemSrchAddr &= (GetMaxMemArea(bDumpPhysicalAddr) - 1);
	}
	else {
		dwMemSrchAddr &= 0xffff;
	}

	/* �q�X�g���ǉ� */
	AddAddrHistory(dwMemSrchAddr, bDumpPhysicalAddr);
}

/*
 *	�����������_�C�A���O
 *	�_�C�A���O�v���V�[�W��
 */
static BOOL CALLBACK MemorySearchDlgProc(HWND hDlg, UINT iMsg,
									WPARAM wParam, LPARAM lParam)
{
	UNUSED(lParam);

	switch (iMsg) {
		/* �_�C�A���O������ */
		case WM_INITDIALOG:
			return MemorySearchDlgInit(hDlg);

		/* �R�}���h���� */
		case WM_COMMAND:
			switch (LOWORD(wParam)) {
				/* OK�E�L�����Z�� */
				case IDOK:
				case IDCANCEL:
					if (LOWORD(wParam) == IDOK) {
						MemorySearchDlgOK(hDlg);
					}
					EndDialog(hDlg, LOWORD(wParam));
					InvalidateRect(hDrawWnd, NULL, FALSE);
					return TRUE;
			}
			break;
	}

	/* ����ȊO�́AFALSE */
	return FALSE;
}

/*
 *	����������
 */
static BOOL FASTCALL MemorySearch(HWND hWnd, BYTE nCPU)
{
	BYTE (FASTCALL *readbnio)(WORD addr);
	BYTE searchdata[128];
	char string[128];
	char tmp[128];
	char *p;
	char *q;
	BOOL flag;
	BYTE dat;
	DWORD datasize;
	DWORD i;
	DWORD j;
	int ret;

	ASSERT(hWnd);

	/* ���[�_���_�C�A���O���s */
	nMemSrchCPU = nCPU;
	ret = DialogBox(hAppInstance, MAKEINTRESOURCE(IDD_SEARCHDLG), hWnd, MemorySearchDlgProc);
	if (ret != IDOK) {
		return FALSE;
	}

	/* ���[�N�ݒ� */
	switch (nCPU) {
		case MAINCPU :	readbnio = mainmem_readbnio;
						break;
		case SUBCPU :	readbnio = submem_readbnio;
						break;
#if XM7_VER == 1
#if defined(JSUB)
		case JSUBCPU :	readbnio = jsubmem_readbnio;
						break;
#endif
#endif
	}
	dwMemSrchAddrSave[nCPU] = dwMemSrchAddr;
	memset(searchdata, 0, sizeof(searchdata));
	memset(string, 0, sizeof(string));
	memset(tmp, 0, sizeof(tmp));

	/* �����������ϊ� */
	if (szMemSrchString[0] == '#') {
		/* 16�i���Ƃ��ċ�����镶���̂ݔ����o�� */
		p = &szMemSrchString[1];
		q = string;
		while (*p != '\0') {
			if ((	((*p >= '0') && (*p <= '9')) ||
					((*p >= 'A') && (*p <= 'F')) ||
					((*p >= 'a') && (*p <= 'f')))) {
				*q++ = *p;
			}
			p++;
		}

		/* �f�[�^��֕ϊ� */
		datasize = 0;
		p = string;

		while (*p != '\0') {
			memcpy(tmp, p, 2);
			searchdata[datasize ++] = (BYTE)strtol(tmp, NULL, 16);
			p += 2;
		}
	}
	else {
		/* ������w�� */
		datasize = strlen(szMemSrchString);
		memcpy(searchdata, szMemSrchString, datasize);
	}

	/* �����f�[�^���Ȃ��ꍇ�������Ȃ� */
	if (datasize == 0) {
		return FALSE;
	}

	/* ����������f�[�^������ */
	flag = FALSE;
#if XM7_VER == 1 && defined(Z80CARD)
	if (((nCPU == MAINCPU) || (nCPU == MAINZ80)) &&
		 isLinearAddrMode(bDumpPhysicalAddr)) {
#else
	if ((nCPU == MAINCPU) && isLinearAddrMode(bDumpPhysicalAddr)) {
#endif
		for (i=dwMemSrchAddr; i<GetMaxMemArea(bDumpPhysicalAddr) - datasize; i++) {
			dat = mainmem_readbnio_p(i);

			/* �擪�f�[�^����v�����ꍇ�Ɏc��̃f�[�^���`�F�b�N*/
			if (searchdata[0] == dat) {
				for (j=1; j<datasize; j++) {
					if (searchdata[j] != mainmem_readbnio_p(i + j)) {
						break;
					}
				}
				if (j == datasize) {
					/* ���� */
					LoadString(hAppInstance, IDS_SWND_SRCH_FOUND_P, tmp, sizeof(tmp));
					_snprintf(string, sizeof(string), tmp, i);
					flag = TRUE;
					break;
				}
			}
		}
	}
	else {
		for (i=(WORD)dwMemSrchAddr; i<65536 - datasize; i++) {
			dat = readbnio((WORD)i);

			/* �擪�f�[�^����v�����ꍇ�Ɏc��̃f�[�^���`�F�b�N*/
			if (searchdata[0] == dat) {
				for (j=1; j<datasize; j++) {
					if (searchdata[j] != readbnio((WORD)(i + j))) {
						break;
					}
				}
				if (j == datasize) {
					/* ���� */
					LoadString(hAppInstance, IDS_SWND_SRCH_FOUND, tmp, sizeof(tmp));
					_snprintf(string, sizeof(string), tmp, i);
					flag = TRUE;
					break;
				}
			}
		}
	}

	if (!flag) {
		/* �����ł��� */
		LoadString(hAppInstance, IDS_SWND_SRCH_NOTFOUND, string, sizeof(string));
	}

	/* �������ʂ�\�� */
	MessageBox(hMainWnd, string, "XM7", MB_OK | MB_ICONINFORMATION);
	SetMenuExitTimer();

	if (flag) {
		/* �����ł����ꍇ�͂��̃A�h���X�ɔ�΂� */
		AddrMemory(nCPU, i);

		/* �����J�n�A�h���X���X�V */
#if XM7_VER == 1 && defined(Z80CARD)
		if (((nCPU == MAINCPU) || (nCPU == MAINZ80)) &&
			 isLinearAddrMode(bDumpPhysicalAddr)) {
#else
		if ((nCPU == MAINCPU) && isLinearAddrMode(bDumpPhysicalAddr)) {
#endif
			dwMemSrchAddrSave[nCPU] = (DWORD)(i + datasize);
		}
		else {
			dwMemSrchAddrSave[nCPU] = (WORD)(i + datasize);
		}
	}

	return TRUE;
}

/*-[ �������_���v�E�C���h�E ]------------------------------------------------*/

/*
 *	�������_���v�E�C���h�E
 *	�A�h���X�ݒ�
 */
void FASTCALL AddrMemory(BYTE nCPU, DWORD dwAddr)
{
	SCROLLINFO sif;
	int height;

	/* �A�h���X�ړ��͈͂𐧌� */
#if XM7_VER == 1 && defined(Z80CARD)
	if (((nCPU == MAINCPU) || (nCPU == MAINZ80)) &&
		 isLinearAddrMode(bDumpPhysicalAddr)) {
#else
	if ((nCPU == MAINCPU) && isLinearAddrMode(bDumpPhysicalAddr)) {
#endif
		dwAddr &= (DWORD)(GetMaxMemArea(bDumpPhysicalAddr) - 1);
	}
	else {
		dwAddr &= (DWORD)(0xfff0);
	}
	height = nHeightMemory[nCPU];
	if (bAsciiDump[nCPU]) {
		height -= 3;
	}
	if (dwAddr >= ((GetMaxMemArea(bDumpPhysicalAddr) + 0x0f) - height * 16)) {
		dwAddr = GetMaxMemArea(bDumpPhysicalAddr) - (height * 16);
	}

	memset(&sif, 0, sizeof(sif));
	sif.cbSize = sizeof(sif);
	sif.fMask = SIF_POS;
	sif.nPos = (int)(dwAddr >> 4);

	/* ���݃`�F�b�N���ݒ� */
	dwMemory[nCPU] = dwAddr;
	switch (nCPU) {
		case MAINCPU :	if (hSubWnd[SWND_MEMORY_MAIN]) {
							SetScrollInfo(hSubWnd[SWND_MEMORY_MAIN], SB_VERT,
								&sif, TRUE);
						}
						break;
		case SUBCPU :	if (hSubWnd[SWND_MEMORY_SUB]) {
							SetScrollInfo(hSubWnd[SWND_MEMORY_SUB], SB_VERT,
								&sif, TRUE);
						}
						break;
#if XM7_VER == 1
#if defined(JSUB)
		case JSUBCPU :	if (hSubWnd[SWND_MEMORY_JSUB]) {
							SetScrollInfo(hSubWnd[SWND_MEMORY_JSUB], SB_VERT,
								&sif, TRUE);
						}
						break;
#endif
#endif
	}

	/* ���t���b�V�� */
	RefreshMemory();
}

/*
 *	�������_���v�E�B���h�E
 *	���T�C�Y
 */
static void FASTCALL ResizeMemory(HWND hWnd, WORD cx, WORD cy, BYTE nCPU)
{
	RECT crect;
	RECT wrect;
	SCROLLINFO si;
	BYTE *p;
	int width;
	int height;

	ASSERT(hWnd);

	/* �ŏ����̏ꍇ�́A�������Ȃ� */
	if ((cx == 0) && (cy == 0)) {
		return;
	}

	/* �c�T�C�Y��ύX */
	nHeightMemory[nCPU] = (WORD)(cy / lCharHeight);
	if (nHeightMemory[nCPU] < 1) {
		nHeightMemory[nCPU] = 1;
	}

	/* ���T�C�Y��ύX */
	if (bAsciiDump[nCPU]) {
		width = 76;
	}
	else {
		width = 54;
	}

	/* �E�B���h�E��`�ύX */
	GetWindowRect(hWnd, &wrect);
	GetClientRect(hWnd, &crect);
	wrect.right -= wrect.left;
	wrect.right -= crect.right;
	wrect.right += (width * lCharWidth);
	wrect.bottom -= wrect.top;
	wrect.bottom -= crect.bottom;
	wrect.bottom += (nHeightMemory[nCPU] * lCharHeight);
	SetWindowPos(hWnd, HWND_TOP, 0, 0, wrect.right, wrect.bottom,
							SWP_NOZORDER | SWP_NOMOVE);

	/* �o�b�t�@���Ď擾���A0xFF�Ŗ��߂� */
	p = realloc(pMemory[nCPU], 2 * width * nHeightMemory[nCPU]);
	ASSERT(p);
	pMemory[nCPU] = p;
	memset(p, 0xff, 2 * width * nHeightMemory[nCPU]);

	/* �X�N���[���o�[�̐ݒ� */
	memset(&si, 0, sizeof(si));
	height = nHeightMemory[nCPU];
	if (bAsciiDump[nCPU]) {
		height -= 3;
	}
	si.cbSize = sizeof(si);
	si.fMask = SIF_ALL;
	si.nMin = 0;
	si.nMax = ((GetMaxMemArea(bDumpPhysicalAddr) >> 4) | 0x0f) - height;
	si.nPage = 0x10;
	si.nPos = (dwMemory[nCPU] >> 4);
	if (si.nPos > si.nMax) {
		si.nPos = si.nMax;
	}
	SetScrollInfo(hWnd, SB_VERT, &si, TRUE);

	if (bPopupSwnd) {
		SetForegroundWindow(hMainWnd);
	}
}

/*
 *	�������_���v�E�B���h�E
 *	���T�C�Y(�s���w��)
 */
static void FASTCALL ResizeMemoryLines(HWND hWnd, int lines, BYTE nCPU)
{
	RECT rect;

	GetWindowRect(hWnd, &rect);
	ResizeMemory(hWnd, 	(WORD)(rect.right - rect.left + 1), 
						(WORD)(lCharHeight * lines), nCPU);
}

/*
 *	�������_���v�E�C���h�E
 *	�Z�b�g�A�b�v
 */
static void FASTCALL SetupMemory(BYTE nCPU, BYTE *p, int x, int y)
{
	BYTE (FASTCALL *readbnio)(WORD addr);
	char string[128];
	char temp[32];
	DWORD addr;
	int i;
	int j;
	int height;
	BYTE mem[16];
	BYTE tsum[16];
	BYTE sum;
	BYTE knjsave;
	int offset;
	BOOL isj;

	ASSERT(nCPU < MAXCPU);
	ASSERT(p);
	ASSERT(x > 0);
	ASSERT(y > 0);

	/* ��U�X�y�[�X�Ŗ��߂� */
	memset(p, 0x20, x * y);
	memset(tsum, 0, 16);

	/* ���[�N������ */
	knjsave = 0;
	isj = isJapanese;

	/* �A�h���X�擾 */
#if XM7_VER == 1 && defined(Z80CARD)
	if (((nCPU == MAINCPU) || (nCPU == MAINZ80)) &&
		 isLinearAddrMode(bDumpPhysicalAddr)) {
#else
	if ((nCPU == MAINCPU) && isLinearAddrMode(bDumpPhysicalAddr)) {
#endif
		addr = (DWORD)(dwMemory[nCPU] & 0xffff0);
	}
	else {
		addr = (WORD)(dwMemory[nCPU] & 0xfff0);
	}
	height = nHeightMemory[nCPU];
	switch (nCPU) {
		case MAINCPU :	readbnio = mainmem_readbnio;
						break;
		case SUBCPU :	readbnio = submem_readbnio;
						break;
#if XM7_VER == 1
#if defined(JSUB)
		case JSUBCPU :	readbnio = jsubmem_readbnio;
						break;
#endif
#endif
	}

	/* ASCII�_���v���[�h �㕔�K�C�h */
	if (bAsciiDump[nCPU]) {
		strncpy(string, "Addr  ", sizeof(string));
		for (j=0; j<16; j++) {
			_snprintf(temp, sizeof(temp), " +%01X", j);
			strncat(string, temp, sizeof(string) - strlen(string) - 1);
		}
		strncat(string, " Sum", sizeof(string) - strlen(string) - 1);

		/* �R�s�[ */
		memcpy(p, string, strlen(string));

		offset = 1;
		height -= 3;
	}
	else {
		offset = 0;
	}

	/* ���[�v */
	for (i=0; i<height; i++) {
		/* �쐬 */
#if XM7_VER == 1 && defined(Z80CARD)
	if (((nCPU == MAINCPU) || (nCPU == MAINZ80)) &&
		 isLinearAddrMode(bDumpPhysicalAddr)) {
#else
		if ((nCPU == MAINCPU) && isLinearAddrMode(bDumpPhysicalAddr)) {
#endif
			_snprintf(string, sizeof(string), "%05X:", addr);
		}
		else {
			_snprintf(string, sizeof(string), "%04X :", (WORD)(addr & 0xffff));
		}
		sum = 0;
		for (j=0; j<16; j++) {
#if XM7_VER == 1 && defined(Z80CARD)
			if (((nCPU == MAINCPU) || (nCPU == MAINZ80)) &&
				 isLinearAddrMode(bDumpPhysicalAddr)) {
#else
			if ((nCPU == MAINCPU) && isLinearAddrMode(bDumpPhysicalAddr)) {
#endif
				mem[j] = mainmem_readbnio_p((DWORD)(addr + j));
			}
			else {
				mem[j] = readbnio((WORD)(addr + j));
			}
			_snprintf(temp, sizeof(temp), " %02X", mem[j]);
			tsum[j] += mem[j];
			sum += mem[j];
			strncat(string, temp, sizeof(string) - strlen(string) - 1);
		}

		if (bAsciiDump[nCPU]) {
			/* �`�F�b�N�T���E������K�C�h */
			_snprintf(temp, sizeof(temp), " :%02X  ................", sum);
			strncat(string, temp, sizeof(string) - strlen(string) - 1);

			/* �����R�[�h��2�s�ɂ܂�����ꍇ�̏��� */
			if (isj && isKanji(knjsave)) {
				string[59] = knjsave;
				knjsave = 1;
			}

			/* ASCII�_���v */
			for (j=0; j<16; j++) {
				if ((mem[j] < 0x20) || (mem[j] == 0x7f) || (mem[j] >= 0xfd)) {
					/* �R���g���[���R�[�h(��\��) */
					knjsave = 0;
				}
				else if (!isj && (mem[j] & 0x80)) {
					/* ����{�ꃂ�[�h 0x80�`0xFF(��\��) */
					knjsave = 0;
				}
				else if (bKanjiDump[nCPU] && isKanji(mem[j]) && (knjsave == 0)) {
					/* �������� */

					if (j == 15) {
						/* ���������s�ɂ܂�����ꍇ�͔�\�� */
						string[j + 60] = ' ';
						knjsave = mem[j];
					}
					else {
						if (isKanji2(mem[j + 1])) {
							/* ������������2�o�C�g�ƂȂ�ꍇ */
							string[j + 60] = mem[j];
							knjsave = 1;
						}
						else {
							/* �R�[�h�ُ�(��\��) */
							knjsave = 0;
						}
					}
				}
				else {
					if ((mem[j] == 0xa0) && (knjsave == 0)) {
						/* �f�[�^��$A0�ŃV�t�gJIS��2�o�C�g�łȂ��ꍇ */
						string[j + 60] = ' ';
					}
					else if ((!isKanji(mem[j]) && (mem[j] != 0x80)) || (knjsave == 1)) {
						/* ANK���� �܂��� �V�t�gJIS��2�o�C�g */
						string[j + 60] = mem[j];
					}
					knjsave = 0;
				}
			}
		}

		/* �R�s�[ */
		memcpy(&p[x * (i + offset)], string, strlen(string));

		/* ���� */
		addr = (DWORD)(addr + 0x0010) & (GetMaxMemArea(bDumpPhysicalAddr) - 1);
		if (addr == 0) {
			break;
		}
	}

	/* ASCII�_���v���[�h �����r���E�c�T���E�u���b�N�T�� */
	if (bAsciiDump[nCPU]) {
		/* �r���쐬 */
		memset(string, '-', 76);
		memset(&string[58], ' ', 2);
		string[77] = '\0';

		/* �R�s�[ */
		memcpy(&p[x * (height + 1)], string, strlen(string));

		/* �c�T���쐬 */
		_snprintf(string, sizeof(string), "Sum  :", addr);
		sum = 0;
		for (j=0; j<16; j++) {
			_snprintf(temp, sizeof(temp), " %02X", tsum[j]);
			sum += tsum[j];
			strncat(string, temp, sizeof(string) - strlen(string) - 1);
		}

		/* �`�F�b�N�T�� */
		_snprintf(temp, sizeof(temp), " :%02X", sum);
		strncat(string, temp, sizeof(string) - strlen(string) - 1);

		/* �R�s�[ */
		memcpy(&p[x * (height + 2)], string, strlen(string));
	}
}

/*
 *	�������_���v�E�C���h�E
 *	�`��
 */
static void FASTCALL DrawMemory(HWND hWnd, HDC hDC)
{
	int i;
	BYTE nCPU;
	RECT rect;
	BYTE *p;
	int x, y;

	ASSERT(hWnd);
	ASSERT(hDC);

	/* Draw�o�b�t�@�𓾂�(���݂��Ȃ���Ή������Ȃ�) */
	for (i=0; i<SWND_MAXNUM; i++) {
		if (hSubWnd[i] == hWnd) {
			nCPU = (BYTE)((i - SWND_MEMORY_MAIN) % MAXCPU);
			p = pMemory[nCPU];
			break;
		}
	}
	if (!p) {
		return;
	}

	/* �E�C���h�E�W�I���g���𓾂� */
	GetClientRect(hWnd, &rect);
	x = rect.right / lCharWidth;
	y = rect.bottom / lCharHeight;
	if ((x == 0) || (y == 0)) {
		return;
	}

	/* �Z�b�g�A�b�v */
	SetupMemory(nCPU, p, x, y);

	/* �`�� */
	if (bKanjiDump[nCPU] && isJapanese) {
		DrawWindowTextKanji(hDC, p, x, y);
	}
	else {
		DrawWindowText(hDC, p, x, y);
	}
}

/*
 *	�������_���v�E�C���h�E
 *	���t���b�V��
 */
void FASTCALL RefreshMemory(void)
{
	HWND hWnd;
	HDC hDC;
	DWORD dwAddr;
	int height;

	/* ���C��CPU */
	if (hSubWnd[SWND_MEMORY_MAIN]) {
		hWnd = hSubWnd[SWND_MEMORY_MAIN];
#if XM7_VER >= 3
		if ((bExtendMMRMode != mmr_ext) || (nFM7Ver != fm7_ver)) {
#elif XM7_VER == 2
		if (nFM7Ver != fm7_ver) {
#else
		if (nFMsubtype != fm_subtype) {
#endif
#if XM7_VER >= 2
			nFM7Ver = fm7_ver;
#if XM7_VER >= 3
			bExtendMMRMode = mmr_ext;
#endif
#else
			nFMsubtype = fm_subtype;
#endif
			if (bDumpPhysicalAddr) {
#if XM7_VER >= 2
				if (fm7_ver == 1) {
#else
				if (fm_subtype != FMSUB_FM77) {
#endif
					dwMemory[MAINCPU] = mmr_trans_phys_to_logi(
						dwMemory[MAINCPU]);
				}
				else {
					dwMemory[MAINCPU] = mmr_trans_mmr(
						(WORD)(dwMemory[MAINCPU] & 0xffff));
				}
			}
			height = nHeightMemory[MAINCPU];
			if (bAsciiDump[MAINCPU]) {
				height -= 3;
			}
			ResizeMemoryLines(hWnd, nHeightMemory[MAINCPU], MAINCPU);
			dwAddr = GetMaxMemArea(bDumpPhysicalAddr) - (height * 0x10);
			if (dwMemory[MAINCPU] >= dwAddr) {
				dwMemory[MAINCPU] = dwAddr;
			}
			AddrMemory(MAINCPU, dwMemory[MAINCPU]);
			PaintMemory(hWnd);
		}
		else {
			hDC = GetDC(hWnd);
			DrawMemory(hWnd, hDC);
			ReleaseDC(hWnd, hDC);
		}
	}

	/* �T�uCPU */
	if (hSubWnd[SWND_MEMORY_SUB]) {
		hWnd = hSubWnd[SWND_MEMORY_SUB];
		hDC = GetDC(hWnd);
		DrawMemory(hWnd, hDC);
		ReleaseDC(hWnd, hDC);
	}

#if XM7_VER == 1
#if defined(JSUB)
	/* ���{��T�uCPU */
	if (hSubWnd[SWND_MEMORY_JSUB]) {
		hWnd = hSubWnd[SWND_MEMORY_JSUB];
		hDC = GetDC(hWnd);
		DrawMemory(hWnd, hDC);
		ReleaseDC(hWnd, hDC);
	}
#endif
#endif
}

/*
 *	�������_���v�E�C���h�E
 *	�ĕ`��
 */
static void FASTCALL PaintMemory(HWND hWnd)
{
	HDC hDC;
	PAINTSTRUCT ps;
	int i;
	RECT rect;
	BYTE *p;
	int x, y;

	ASSERT(hWnd);

	/* Draw�o�b�t�@�𓾂�(���݂��Ȃ���Ή������Ȃ�) */
	for (i=0; i<SWND_MAXNUM; i++) {
		if (hSubWnd[i] == hWnd) {
			p = pMemory[((i - SWND_MEMORY_MAIN) % MAXCPU)];
			break;
		}
	}

	if (!p) {
		return;
	}

	/* �E�C���h�E�W�I���g���𓾂� */
	GetClientRect(hWnd, &rect);
	x = rect.right / lCharWidth;
	y = rect.bottom / lCharHeight;

	/* �㔼�G���A��FF�Ŗ��߂� */
	if ((x > 0) && (y > 0)) {
		memset(&p[x * y], 0xff, x * y);
	}

	/* �`�� */
	hDC = BeginPaint(hWnd, &ps);
	ASSERT(hDC);
	DrawMemory(hWnd, hDC);
	EndPaint(hWnd, &ps);
}

/*
 *	�������_���v�E�C���h�E
 *	�R�}���h����
 */
static void FASTCALL CmdMemory(HWND hWnd, WORD wID, BYTE nCPU)
{
	DWORD target;
	cpu6809_t *cpu;
	BYTE height;
#if XM7_VER == 1 && defined(Z80CARD)
	WORD tmp;
#endif

	/* CPU�\���̌��� */
	switch (nCPU) {
		case MAINCPU :	cpu = &maincpu;
						break;
		case SUBCPU :	cpu = &subcpu;
						break;
#if XM7_VER == 1
#if defined(JSUB)
		case JSUBCPU :	cpu = &jsubcpu;
						break;
#endif
#endif
	}

	/* �^�[�Q�b�g�A�h���X���� */
	switch (wID) {
		case IDM_DMP_ADDR:
			target = dwMemory[nCPU];
#if XM7_VER == 1 && defined(Z80CARD)
			if (((nCPU == MAINCPU) || (nCPU == MAINZ80)) &&
				 isLinearAddrMode(bDumpPhysicalAddr)) {
#else
			if ((nCPU == MAINCPU) && isLinearAddrMode(bDumpPhysicalAddr)) {
#endif
				if (!AddrDlg(hWnd, &target, nCPU, TRUE)) {
					return;
				}
			}
			else {
				if (!AddrDlg(hWnd, &target, nCPU, FALSE)) {
					return;
				}
			}
			target &= GetMaxMemArea(bDumpPhysicalAddr) - 1;
			break;

		case IDM_DMP_BLK128:
		case IDM_DMP_BLK256:
			if (wID == IDM_DMP_BLK128) {
				height = 8;
			}
			else if (wID == IDM_DMP_BLK256) {
				height = 16;
			}
			if (bAsciiDump[nCPU]) {
				height += (BYTE)3;
			}
			ResizeMemoryLines(hWnd, height, nCPU);
			return;

		case IDM_DMP_ASCII:
			bAsciiDump[nCPU] = !bAsciiDump[nCPU];

			/* �c�T�C�Y�ύX */
			height = nHeightMemory[nCPU];
			if (bAsciiDump[nCPU]) {
				height += (BYTE)3;
			}
			else {
				height -= (BYTE)3;
			}
			ResizeMemoryLines(hWnd, height, nCPU);
			PaintMemory(hWnd);
			return;

		case IDM_DMP_KANJI:
			bKanjiDump[nCPU] = !bKanjiDump[nCPU];
			InvalidateRect(hWnd, NULL, FALSE);
			return;

		case IDM_DMP_SEARCH:
			MemorySearch(hWnd, nCPU);
			return;

		case IDM_DMP_PHYSADDR:
#if XM7_VER == 1
			if ((fm_subtype != FMSUB_FM77) || nCPU != MAINCPU) {
#else
			if ((fm7_ver <= 1) || nCPU != MAINCPU) {
#endif
				break;
			}

			if (bDumpPhysicalAddr) {
				target = mmr_trans_phys_to_logi(dwMemory[nCPU]);
			}
			else {
				target = mmr_trans_mmr((WORD)(dwMemory[nCPU] & 0xffff));
			}
			bDumpPhysicalAddr = !bDumpPhysicalAddr;
			ResizeMemoryLines(hWnd, nHeightMemory[nCPU], nCPU);
			break;

		case IDM_DMP_PC:
#if XM7_VER == 1 && defined(Z80CARD)
			if (((nCPU == MAINCPU) || (nCPU == MAINZ80)) &&
				 isLinearAddrMode(bDumpPhysicalAddr)) {
#else
			if ((nCPU == MAINCPU) && isLinearAddrMode(bDumpPhysicalAddr)) {
#endif
				target = mmr_trans_mmr(cpu->pc);
			}
			else {
				target = (DWORD)cpu->pc;
			}
			break;
		case IDM_DMP_X:
#if XM7_VER == 1 && defined(Z80CARD)
			if (((nCPU == MAINCPU) || (nCPU == MAINZ80)) &&
				 isLinearAddrMode(bDumpPhysicalAddr)) {
#else
			if ((nCPU == MAINCPU) && isLinearAddrMode(bDumpPhysicalAddr)) {
#endif
				target = mmr_trans_mmr(cpu->x);
			}
			else {
				target = (DWORD)cpu->x;
			}
			break;
		case IDM_DMP_Y:
#if XM7_VER == 1 && defined(Z80CARD)
			if (((nCPU == MAINCPU) || (nCPU == MAINZ80)) &&
				 isLinearAddrMode(bDumpPhysicalAddr)) {
#else
			if ((nCPU == MAINCPU) && isLinearAddrMode(bDumpPhysicalAddr)) {
#endif
				target = mmr_trans_mmr(cpu->y);
			}
			else {
				target = (DWORD)cpu->y;
			}
			break;
		case IDM_DMP_U:
#if XM7_VER == 1 && defined(Z80CARD)
			if (((nCPU == MAINCPU) || (nCPU == MAINZ80)) &&
				 isLinearAddrMode(bDumpPhysicalAddr)) {
#else
			if ((nCPU == MAINCPU) && isLinearAddrMode(bDumpPhysicalAddr)) {
#endif
				target = mmr_trans_mmr(cpu->u);
			}
			else {
				target = (DWORD)cpu->u;
			}
			break;
		case IDM_DMP_S:
#if XM7_VER == 1 && defined(Z80CARD)
			if (((nCPU == MAINCPU) || (nCPU == MAINZ80)) &&
				 isLinearAddrMode(bDumpPhysicalAddr)) {
#else
			if ((nCPU == MAINCPU) && isLinearAddrMode(bDumpPhysicalAddr)) {
#endif
				target = mmr_trans_mmr(cpu->s);
			}
			else {
				target = (DWORD)cpu->s;
			}
			break;
		case IDM_DMP_DP:
#if XM7_VER == 1 && defined(Z80CARD)
			if (((nCPU == MAINCPU) || (nCPU == MAINZ80)) &&
				 isLinearAddrMode(bDumpPhysicalAddr)) {
#else
			if ((nCPU == MAINCPU) && isLinearAddrMode(bDumpPhysicalAddr)) {
#endif
				target = mmr_trans_mmr((WORD)(cpu->dp << 8));
			}
			else {
				target = (DWORD)(cpu->dp << 8);
			}
			break;

#if XM7_VER == 1 && defined(Z80CARD)
			case IDM_DMP_PC_Z80:
				tmp = mainz80.pc;
				if (isLinearAddrMode(bDumpPhysicalAddr)) {
					target = mmr_trans_mmr(tmp);
				}
				else {
					target = (DWORD)tmp;
				}
				break;
			case IDM_DMP_BC:
				tmp = (WORD)((mainz80.regs8[REGID_B]) << 8 |
					mainz80.regs8[REGID_C]);
				if (isLinearAddrMode(bDumpPhysicalAddr)) {
					target = mmr_trans_mmr(tmp);
				}
				else {
					target = (DWORD)tmp;
				}
				break;
			case IDM_DMP_DE:
				tmp = (WORD)((mainz80.regs8[REGID_D]) << 8 |
					mainz80.regs8[REGID_E]);
				if (isLinearAddrMode(bDumpPhysicalAddr)) {
					target = mmr_trans_mmr(tmp);
				}
				else {
					target = (DWORD)tmp;
				}
				break;
			case IDM_DMP_HL:
				tmp = (WORD)((mainz80.regs8[REGID_H]) << 8 |
					mainz80.regs8[REGID_L]);
				if (isLinearAddrMode(bDumpPhysicalAddr)) {
					target = mmr_trans_mmr(tmp);
				}
				else {
					target = (DWORD)tmp;
				}
				break;
			case IDM_DMP_IX:
				tmp = (WORD)((mainz80.regs8[REGID_IXH]) << 8 |
					mainz80.regs8[REGID_IXL]);
				if (isLinearAddrMode(bDumpPhysicalAddr)) {
					target = mmr_trans_mmr(tmp);
				}
				else {
					target = (DWORD)tmp;
				}
				break;
			case IDM_DMP_IY:
				tmp = (WORD)((mainz80.regs8[REGID_IYH]) << 8 |
					mainz80.regs8[REGID_IYL]);
				if (isLinearAddrMode(bDumpPhysicalAddr)) {
					target = mmr_trans_mmr(tmp);
				}
				else {
					target = (DWORD)tmp;
				}
				break;
			case IDM_DMP_SP:
				tmp = mainz80.sp;
				if (isLinearAddrMode(bDumpPhysicalAddr)) {
					target = mmr_trans_mmr(tmp);
				}
				else {
					target = (DWORD)tmp;
				}
				break;
#endif

		case IDM_DMP_RESET:
			target = (WORD)((cpu->readmem(0xfffe) << 8) | cpu->readmem(0xffff));
#if XM7_VER == 1 && defined(Z80CARD)
			if (((nCPU == MAINCPU) || (nCPU == MAINZ80)) &&
				 isLinearAddrMode(bDumpPhysicalAddr)) {
#else
			if ((nCPU == MAINCPU) && isLinearAddrMode(bDumpPhysicalAddr)) {
#endif
				target = (DWORD)mmr_trans_mmr((WORD)target);
			}
			break;
		case IDM_DMP_NMI:
			target = (WORD)((cpu->readmem(0xfffc) << 8) | cpu->readmem(0xfffd));
#if XM7_VER == 1 && defined(Z80CARD)
			if (((nCPU == MAINCPU) || (nCPU == MAINZ80)) &&
				 isLinearAddrMode(bDumpPhysicalAddr)) {
#else
			if ((nCPU == MAINCPU) && isLinearAddrMode(bDumpPhysicalAddr)) {
#endif
				target = (DWORD)mmr_trans_mmr((WORD)target);
			}
			break;
		case IDM_DMP_SWI:
			target = (WORD)((cpu->readmem(0xfffa) << 8) | cpu->readmem(0xfffb));
#if XM7_VER == 1 && defined(Z80CARD)
			if (((nCPU == MAINCPU) || (nCPU == MAINZ80)) &&
				 isLinearAddrMode(bDumpPhysicalAddr)) {
#else
			if ((nCPU == MAINCPU) && isLinearAddrMode(bDumpPhysicalAddr)) {
#endif
				target = (DWORD)mmr_trans_mmr((WORD)target);
			}
			break;
		case IDM_DMP_IRQ:
			target = (WORD)((cpu->readmem(0xfff8) << 8) | cpu->readmem(0xfff9));
#if XM7_VER == 1 && defined(Z80CARD)
			if (((nCPU == MAINCPU) || (nCPU == MAINZ80)) &&
				 isLinearAddrMode(bDumpPhysicalAddr)) {
#else
			if ((nCPU == MAINCPU) && isLinearAddrMode(bDumpPhysicalAddr)) {
#endif
				target = (DWORD)mmr_trans_mmr((WORD)target);
			}
			break;
		case IDM_DMP_FIRQ:
			target = (WORD)((cpu->readmem(0xfff6) << 8) | cpu->readmem(0xfff7));
#if XM7_VER == 1 && defined(Z80CARD)
			if (((nCPU == MAINCPU) || (nCPU == MAINZ80)) &&
				 isLinearAddrMode(bDumpPhysicalAddr)) {
#else
			if ((nCPU == MAINCPU) && isLinearAddrMode(bDumpPhysicalAddr)) {
#endif
				target = (DWORD)mmr_trans_mmr((WORD)target);
			}
			break;
		case IDM_DMP_SWI2:
			target = (WORD)((cpu->readmem(0xfff4) << 8) | cpu->readmem(0xfff5));
#if XM7_VER == 1 && defined(Z80CARD)
			if (((nCPU == MAINCPU) || (nCPU == MAINZ80)) &&
				 isLinearAddrMode(bDumpPhysicalAddr)) {
#else
			if ((nCPU == MAINCPU) && isLinearAddrMode(bDumpPhysicalAddr)) {
#endif
				target = (DWORD)mmr_trans_mmr((WORD)target);
			}
			break;
		case IDM_DMP_SWI3:
			target = (WORD)((cpu->readmem(0xfff2) << 8) | cpu->readmem(0xfff3));
#if XM7_VER == 1 && defined(Z80CARD)
			if (((nCPU == MAINCPU) || (nCPU == MAINZ80)) &&
				 isLinearAddrMode(bDumpPhysicalAddr)) {
#else
			if ((nCPU == MAINCPU) && isLinearAddrMode(bDumpPhysicalAddr)) {
#endif
				target = (DWORD)mmr_trans_mmr((WORD)target);
			}
			break;
		case IDM_DMP_IO:
			switch (nCPU) {
				case MAINCPU :	if (bDumpPhysicalAddr) {
									bDumpPhysicalAddr = FALSE;
									ResizeMemoryLines(hWnd,
										nHeightMemory[nCPU], nCPU);
								}
								target = 0xfd00;
								break;
				case SUBCPU :	target = 0xd400;
								break;
				default :		target = 0x0000;
			}
			break;
		default:
			if ((wID >= IDM_DMP_STACK) && (wID <= IDM_DMP_STACK + 15)) {
				target = mmr_trans_mmr((WORD)wStackAddr[wID - IDM_DMP_STACK]);
			}
			if ((wID >= IDM_DMP_00000) && (wID <= IDM_DMP_F0000)) {
				target = (wID - IDM_DMP_00000) * 0x10000;
			}
			break;
	}

	/* �ݒ聕�X�V */
	AddrMemory(nCPU, target);
}

/*
 *	�������_���v�E�C���h�E
 *	�E�C���h�E�v���V�[�W��
 */
static LRESULT CALLBACK MemoryProc(HWND hWnd, UINT message,
								 WPARAM wParam, LPARAM lParam, BYTE nCPU)
{
	static BOOL bResize = FALSE;		/* �蓮���T�C�Y�t���O */
	HMENU hMenu;
	POINT point;
	DWORD dwAddr;
	int height;
	UINT idcheck;
	RECT rect;
#if XM7_VER == 1 && defined(Z80CARD)
	int i;
#endif

	/* ���b�Z�[�W�� */
	switch (message) {
		/* �E�C���h�E�ĕ`�� */
		case WM_PAINT:
			/* ���b�N���K�v */
			LockVM();
			PaintMemory(hWnd);
			UnlockVM();
			return 0;

		/* �E�C���h�E�폜 */
		case WM_DESTROY:
			LockVM();

			/* ���C���E�C���h�E�֎����ʒm */
			DestroySubWindow(hWnd, &pMemory[nCPU], hMemory[nCPU]);

			UnlockVM();
			break;

		/* �E�C���h�E�T�C�Y�ύX */
		case WM_SIZE:
			if (bResize) {
				bResize = FALSE;
				ResizeMemory(hWnd, LOWORD(lParam), HIWORD(lParam), nCPU);
				height = nHeightMemory[nCPU];
				if (bAsciiDump[nCPU]) {
					height -= 3;
				}
#if XM7_VER == 1
				if ((nCPU == MAINCPU) || (nCPU == MAINZ80)) {
#else
				if (nCPU == MAINCPU) {
#endif
					dwAddr = GetMaxMemArea(bDumpPhysicalAddr) - (height * 0x10);
				}
				else {
					dwAddr = 0x10000 - (height * 0x10);
				}
				if (dwMemory[nCPU] >= dwAddr) {
					AddrMemory(nCPU, dwAddr);
					PaintMemory(hWnd);
				}
			}
			break;

		/* �E�C���h�E�T�C�Y�ύX�� */
		case WM_SIZING:
			WindowSizing(hWnd, (LPRECT)lParam, &pMemory[nCPU]);
			bResize = TRUE;
			break;

		/* �E�B���h�E�T�C�Y�ύX���b�Z�[�W */
		case WM_WINDOWPOSCHANGING:
			bResize = TRUE;
			break;

		/* �ŏ��T�C�Y���� */
		case WM_GETMINMAXINFO:
			rect.left = 0;
			rect.right = 54 * lCharWidth + GetSystemMetrics(SM_CXVSCROLL);
			rect.top = 0;
			rect.bottom = lCharHeight;

			/* ASCII�_���v���[�h���̏c�T���X�y�[�X���l�� */
			if (bAsciiDump[nCPU]) {
				rect.right += (22 * lCharWidth);
				rect.bottom += (lCharHeight * 3);
			}

			if (bPopupSwnd) {
				AdjustWindowRect(&rect, WS_POPUP | WS_OVERLAPPED | WS_SYSMENU |
										WS_CAPTION | WS_VISIBLE | 
										WS_MINIMIZEBOX | WS_SIZEBOX, FALSE);
			}
			else {
				AdjustWindowRect(&rect, WS_CHILD | WS_OVERLAPPED | WS_SYSMENU |
										WS_CAPTION | WS_VISIBLE | 
										WS_MINIMIZEBOX | WS_CLIPSIBLINGS | 
										WS_SIZEBOX, FALSE);
			}
			((LPMINMAXINFO)lParam)->ptMinTrackSize.x = rect.right - rect.left;
			((LPMINMAXINFO)lParam)->ptMaxTrackSize.x = rect.right - rect.left;
			((LPMINMAXINFO)lParam)->ptMinTrackSize.y = rect.bottom - rect.top;
			return 0;

		/* ���������e�ύX */
		case WM_LBUTTONDBLCLK:
#if defined(MOUSE)
			/* �}�E�X�G�~�����[�V�����g�p���͖��� */
			if (mos_capture && !stopreq_flag && run_flag) {
				return FALSE;
			}
#endif

			/* �J�[�\���ʒu����A���� */
			point.x = LOWORD(lParam) / lCharWidth;
			point.y = HIWORD(lParam) / lCharHeight;
			if (bAsciiDump[nCPU]) {
				point.y --;
			}

			MemoryChange(hWnd, nCPU, point.x, point.y);
			return 0;

		/* �R���e�L�X�g���j���[ */
		case WM_CONTEXTMENU:
#if defined(MOUSE)
			/* �}�E�X�G�~�����[�V�����g�p���͖��� */
			if (mos_capture && !stopreq_flag && run_flag) {
				return FALSE;
			}
#endif

			/* �T�u���j���[�����o�� */
			hMenu = GetSubMenu(hMemory[nCPU], 0);
			height = nHeightMemory[nCPU];
			if (bAsciiDump[nCPU]) {
				height -= 3;
			}

			/* �\���T�C�Y�`�F�b�N */
			CheckMenuSub(hMenu, IDM_DMP_BLK128, FALSE);
			CheckMenuSub(hMenu, IDM_DMP_BLK256, FALSE);
			if (height == 8) {
				idcheck = IDM_DMP_BLK128;
			}
			else if (height == 16) {
				idcheck = IDM_DMP_BLK256;
			}
			else {
				idcheck = 0;
			}
			if (idcheck > 0) {
				CheckMenuRadioItem(hMenu, IDM_DMP_BLK128, IDM_DMP_BLK256, idcheck, MF_BYCOMMAND);
			}

			/* ASCII�_���v�`�F�b�N */
			CheckMenuSub(hMenu, IDM_DMP_ASCII, bAsciiDump[nCPU]);

			/* �����\���`�F�b�N */
			if (isJapanese) {
				EnableMenuSub(hMenu, IDM_DMP_KANJI, bAsciiDump[nCPU]);
				CheckMenuSub(hMenu, IDM_DMP_KANJI, bKanjiDump[nCPU]);
			}
			else {
				EnableMenuSub(hMenu, IDM_DMP_KANJI, FALSE);
				CheckMenuSub(hMenu, IDM_DMP_KANJI, FALSE);
			}

#if XM7_VER == 1
#if defined(JSUB)
			/* I/O�̈�`�F�b�N */
			if (nCPU == JSUBCPU) {
				EnableMenuSub(hMenu, IDM_DMP_IO, FALSE);
			}
			else {
				EnableMenuSub(hMenu, IDM_DMP_IO, TRUE);
			}
#endif
#endif

			/* �����A�h���X */
#if XM7_VER == 1
#if defined(Z80CARD)
			if ((fm_subtype == FMSUB_FM77) &&
				((nCPU == MAINCPU) || (nCPU == MAINZ80))) {
#else
			if ((fm_subtype == FMSUB_FM77) && (nCPU == MAINCPU)) {
#endif
#else
			if ((fm7_ver >= 2) && (nCPU == MAINCPU)) {
#endif
				EnableMenuSub(hMenu, IDM_DMP_PHYSADDR, TRUE);
				CheckMenuSub(hMenu, IDM_DMP_PHYSADDR, bDumpPhysicalAddr);
				if (bDumpPhysicalAddr) {
					EnableMenuItem(hMenu, 9, MF_BYPOSITION | MF_ENABLED);
				}
				else {
					EnableMenuItem(hMenu, 9, MF_BYPOSITION | MF_GRAYED);
				}
			}
			else {
				EnableMenuSub(hMenu, IDM_DMP_PHYSADDR, FALSE);
				CheckMenuSub(hMenu, IDM_DMP_PHYSADDR, FALSE);
				EnableMenuItem(hMenu, 9, MF_BYPOSITION | MF_GRAYED);
			}
			InsertPhysicalMemMenu(GetSubMenu(hMenu, 9), nCPU, IDM_DMP_00000);

#if XM7_VER == 1 && defined(Z80CARD)
			/* �T�uCPU/���{��T�uCPU����Z80���W�X�^�폜 */
			if ((nCPU != MAINCPU) && (nCPU != MAINZ80)) {
				for (i = 6; i < 13; i++) {
					DeleteMenu(GetSubMenu(hMenu, 11), 6, MF_BYPOSITION);
				}
			}
#endif

			/* �V�X�e���X�^�b�N�W�����v���j���[�ݒ� */
			InsertStackJumpMenu(GetSubMenu(hMenu, 13), nCPU, IDM_DMP_STACK);

			/* �R���e�L�X�g���j���[�����s */
			point.x = LOWORD(lParam);
			point.y = HIWORD(lParam);
			TrackPopupMenu(hMenu, 0, point.x, point.y, 0, hWnd, NULL);
			return 0;

		/* �R�}���h */
		case WM_COMMAND:
			CmdMemory(hWnd, LOWORD(wParam), nCPU);
			break;

		/* �����X�N���[���o�[ */
		case WM_VSCROLL:
			/* �^�C�v���� */
			dwAddr = dwMemory[nCPU];
			height = nHeightMemory[nCPU];
			if (bAsciiDump[nCPU]) {
				height -= 3;
			}

			/* �A�N�V������ */
			switch (LOWORD(wParam)) {
				/* �g�b�v */
				case SB_TOP:
					dwAddr = 0;
					break;
				/* �I�[ */
				case SB_BOTTOM:
#if XM7_VER == 1 && defined(Z80CARD)
					if ((nCPU == MAINCPU) || (nCPU == MAINZ80)) {
#else
					if (nCPU == MAINCPU) {
#endif
						dwAddr = (DWORD)(GetMaxMemArea(bDumpPhysicalAddr) - height * 16);
					}
					else {
						dwAddr = (DWORD)(0x10000 - height * 16);
					}
					break;
				/* �P�s�� */
				case SB_LINEUP:
					if (dwAddr >= 0x0010) {
						dwAddr -= (WORD)0x0010;
					}
					break;
				/* �P�s�� */
				case SB_LINEDOWN:
#if XM7_VER == 1 && defined(Z80CARD)
					if ((nCPU == MAINCPU) || (nCPU == MAINZ80)) {
#else
					if (nCPU == MAINCPU) {
#endif
						if (dwAddr < GetMaxMemArea(bDumpPhysicalAddr) - (DWORD)(height * 16)) {
							dwAddr += (DWORD)0x0010;
						}
					}
					else {
						if (dwAddr < 0x10000 - (DWORD)(height * 16)) {
							dwAddr += (DWORD)0x0010;
						}
					}
					break;
				/* �y�[�W�A�b�v */
				case SB_PAGEUP:
					if (dwAddr <= (DWORD)(height * 16)) {
						dwAddr = 0;
					}
					else {
						dwAddr -= (DWORD)(height * 0x10);
#if XM7_VER == 1 && defined(Z80CARD)
						if ((nCPU == MAINCPU) || (nCPU == MAINZ80)) {
#else
						if (nCPU == MAINCPU) {
#endif
							dwAddr &= (GetMaxMemArea(bDumpPhysicalAddr) - 1);
						}
						else {
							dwAddr &= 0xffff;
						}
					}
					break;
				/* �y�[�W�_�E�� */
				case SB_PAGEDOWN:
#if XM7_VER == 1 && defined(Z80CARD)
					if ((nCPU == MAINCPU) || (nCPU == MAINZ80)) {
#else
					if (nCPU == MAINCPU) {
#endif
						if (dwAddr >= (GetMaxMemArea(bDumpPhysicalAddr) - (height * 16))) {
							dwAddr = GetMaxMemArea(bDumpPhysicalAddr) - (height * 16);
						}
						else {
							dwAddr += (DWORD)(height * 16);
							dwAddr &= (GetMaxMemArea(bDumpPhysicalAddr) - 1);
						}
					}
					else {
						if (dwAddr >= (0x10000 - (DWORD)(height * 16))) {
							dwAddr = (0x10000 - (DWORD)(height * 16));
						}
						else {
							dwAddr += (DWORD)(height * 16);
							dwAddr &= 0xffff;
						}
					}
					break;
				/* ���ڎw�� */
				case SB_THUMBTRACK:
#if XM7_VER == 1 && defined(Z80CARD)
					if ((nCPU == MAINCPU) || (nCPU == MAINZ80)) {
#else
					if (nCPU == MAINCPU) {
#endif
						dwAddr = (DWORD)(HIWORD(wParam) * 16) &
							(GetMaxMemArea(bDumpPhysicalAddr) - 1);
						if (dwAddr >= ((GetMaxMemArea(bDumpPhysicalAddr) + 0x0f) -
							(DWORD)(height * 16))) {
							dwAddr = GetMaxMemArea(bDumpPhysicalAddr) - (DWORD)(height * 16);
						}
					}
					else {
						dwAddr = (DWORD)(HIWORD(wParam) * 16) & 0xffff;
						if (dwAddr >= (0x1000f - (DWORD)(height * 16))) {
							dwAddr = 0x10000 - (DWORD)(height * 16);
						}
					}
					break;
			}
			dwAddr &= (GetMaxMemArea(bDumpPhysicalAddr) - 0x10);
			AddrMemory(nCPU, dwAddr);
			RefreshMemory();
			break;
	}

	/* �f�t�H���g �E�C���h�E�v���V�[�W�� */
	return DefWindowProc(hWnd, message, wParam, lParam);
}

/*
 *	�������_���v�E�C���h�E(���C��)
 *	�E�C���h�E�v���V�[�W��
 */
static LRESULT CALLBACK MemoryProcMain(HWND hWnd, UINT message,
								 WPARAM wParam, LPARAM lParam)
{
	return MemoryProc(hWnd, message, wParam, lParam, MAINCPU);
}

/*
 *	�������_���v�E�C���h�E(�T�u)
 *	�E�C���h�E�v���V�[�W��
 */
static LRESULT CALLBACK MemoryProcSub(HWND hWnd, UINT message,
								 WPARAM wParam, LPARAM lParam)
{
	return MemoryProc(hWnd, message, wParam, lParam, SUBCPU);
}

#if XM7_VER == 1
/*
 *	�������_���v�E�C���h�E(���{��T�u)
 *	�E�C���h�E�v���V�[�W��
 */
#if defined(JSUB)
static LRESULT CALLBACK MemoryProcJsub(HWND hWnd, UINT message,
								 WPARAM wParam, LPARAM lParam)
{
	return MemoryProc(hWnd, message, wParam, lParam, JSUBCPU);
}
#endif
#endif

/*
 *	�������_���v�E�C���h�E
 *	�E�C���h�E�쐬
 */
HWND FASTCALL CreateMemory(HWND hParent, BYTE nCPU, int index)
{
	WNDCLASSEX wcex;
	char szClassNameMain[] = "XM7_MemoryMain";
	char szClassNameSub[] = "XM7_MemorySub";
#if XM7_VER == 1
#if defined(JSUB)
	char szClassNameJsub[] = "XM7_MemoryJsub";
#endif
#endif
	char *szClassName;
	char szWndName[128];
	RECT rect;
	RECT crect, wrect;
	HWND hWnd;
	int height;
	SCROLLINFO si;
	UINT id;
	DWORD pc;
	DWORD dwStyle;

	ASSERT(hParent);

	/* MMR�g�����[�h�A�@��o�[�W����/�T�u�o�[�W�����̏����l��ݒ� */
#if XM7_VER >= 2
	nFM7Ver = fm7_ver;
#if XM7_VER >= 3
	bExtendMMRMode = mmr_ext;
#endif
#else
	nFMsubtype = fm_subtype;
#endif

	/* �E�C���h�E��`���v�Z */
	PositioningSubWindow(hParent, &rect, index);

	/* �E�C���h�E�^�C�g��������A�o�b�t�@�m�ہA���j���[���[�h */
	switch (nCPU) {
		case MAINCPU :	id = IDS_SWND_MEMORY_MAIN;
						if (bDumpPhysicalAddr) {
							pc = maincpu.pc;
							if (!mmr_trans_twr(maincpu.pc, &pc)) {
								pc = mmr_trans_mmr(maincpu.pc);
							}
						}
						else {
							pc = maincpu.pc;
						}
						szClassName = szClassNameMain;
						break;
		case SUBCPU :	id = IDS_SWND_MEMORY_SUB;
						pc = subcpu.pc;
						szClassName = szClassNameSub;
						break;
#if XM7_VER == 1
#if defined(JSUB)
		case JSUBCPU :	id = IDS_SWND_MEMORY_JSUB;
						pc = jsubcpu.pc;
						szClassName = szClassNameJsub;
						break;
#endif
#endif
	}
	rect.bottom = lCharHeight * nHeightMemory[nCPU];
	LoadString(hAppInstance, id, szWndName, sizeof(szWndName));
	pMemory[nCPU] = malloc(2 * 77 * nHeightMemory[nCPU]);
	dwMemory[nCPU] = pc;
#if XM7_VER == 1 && defined(Z80CARD)
	if (nCPU == MAINCPU) {
		hMemory[nCPU] = LoadMenu(hAppInstance, MAKEINTRESOURCE(IDR_MEMDMPMAINMENU));
	}
	else {
		hMemory[nCPU] = LoadMenu(hAppInstance, MAKEINTRESOURCE(IDR_MEMDMPMENU));
	}
#else
	hMemory[nCPU] = LoadMenu(hAppInstance, MAKEINTRESOURCE(IDR_MEMDMPMENU));
#endif

	/* �E�B���h�E������ */
	if (bAsciiDump[nCPU]) {
		rect.right = lCharWidth * 76;
	}
	else {
		rect.right = lCharWidth * 54;
	}

	/* �E�C���h�E�N���X�̓o�^ */
	memset(&wcex, 0, sizeof(wcex));
	wcex.cbSize = sizeof(wcex);
	wcex.style = CS_VREDRAW | CS_HREDRAW | CS_DBLCLKS;
	switch (nCPU) {
		case MAINCPU :	wcex.lpfnWndProc = MemoryProcMain;
						break;
		case SUBCPU :	wcex.lpfnWndProc = MemoryProcSub;
						break;
#if XM7_VER == 1
#if defined(JSUB)
		case JSUBCPU :	wcex.lpfnWndProc = MemoryProcJsub;
						break;
#endif
#endif
	}
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hAppInstance;
	wcex.hIcon = LoadIcon(hAppInstance, MAKEINTRESOURCE(IDI_WNDICON));
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	wcex.lpszMenuName = NULL;
	wcex.lpszClassName = szClassName;
	wcex.hIconSm = LoadIcon(hAppInstance, MAKEINTRESOURCE(IDI_WNDICON));
	RegisterClassEx(&wcex);

	/* �E�C���h�E�쐬 */
	if (bPopupSwnd) {
		dwStyle =	WS_POPUP | WS_OVERLAPPED | WS_SYSMENU | WS_CAPTION |
					WS_VISIBLE | WS_MINIMIZEBOX | WS_VSCROLL | WS_SIZEBOX;
	}
	else {
		dwStyle =	WS_CHILD | WS_OVERLAPPED | WS_SYSMENU | WS_CAPTION |
					WS_VISIBLE | WS_MINIMIZEBOX | WS_CLIPSIBLINGS |
					WS_VSCROLL | WS_SIZEBOX;
	}
	hWnd = CreateWindow(szClassName,
						szWndName,
						dwStyle,
						rect.left,
						rect.top,
						rect.right,
						rect.bottom,
						hParent,
						NULL,
						hAppInstance,
						NULL);

	/* �L���Ȃ�A�T�C�Y�␳���Ď�O�ɒu�� */
	if (hWnd) {
		GetWindowRect(hWnd, &wrect);
		GetClientRect(hWnd, &crect);
		wrect.right += (rect.right - crect.right);
		wrect.bottom += (rect.bottom - crect.bottom);
		SetWindowPos(hWnd, HWND_TOP, wrect.left, wrect.top,
			wrect.right - wrect.left, wrect.bottom - wrect.top, SWP_NOMOVE);

		/* �X�N���[���o�[�̐ݒ肪�K�v */
		memset(&si, 0, sizeof(si));
		si.cbSize = sizeof(si);
		si.fMask = SIF_ALL;
		si.nMin = 0;
		si.nPage = 0x10;
		height = nHeightMemory[nCPU];
		si.nPos = (pc >> 4);
		if (bAsciiDump) {
			height -= 3;
		}
		si.nMax = ((GetMaxMemArea(bDumpPhysicalAddr) >> 4) + 0x0f) - height;
		SetScrollInfo(hWnd, SB_VERT, &si, TRUE);
	}

	/* �|�b�v�A�b�v�E�C���h�E���̓A�N�e�B�u�E�C���h�E��O�ʂɕύX */
	if (bPopupSwnd) {
		SetForegroundWindow(hMainWnd);
	}

	/* ���ʂ������A�� */
	return hWnd;
}

#endif	/* _WIN32 */
