/*
 *	FM-7 EMULATOR "XM7"
 *
 *	Copyright (C) 1999-2017 �o�h�D(Twitter:@xm6_original)
 *	Copyright (C) 2001-2017 Ryu Takegami (Twitter:@RyuTakegami)
 *
 *	[ Win32API �R���g���[���o�[ ]
 *
 *	RHG����
 *	  2003.05.25		ResizeStatus�֐���ǉ�
 *	  2004.05.03		WinXP�Ή�������DLL�ǂݍ��݃`�F�b�N�̈��S��������
 *	  2004.08.31		2D/2DD/VFD/�^�C�g���Ȃ�D77�̕\��������ύX
 *	  2010.08.05		�C���W�P�[�^�����XM6�ɍ��킹���d�l�ɕύX
 *	  2010.10.03		2�{�g��\�����[�h�ɑΉ�
 *	  2012.06.03		�X�e�[�^�X�o�[�̕\����Ԃ̏����l��SW_HIDE�ɕύX
 *	  2012.08.01		CAP/����/INS�L�[�\�����b�Z�[�W�����\�[�X����ǂނ悤��
 *						�ύX
 *	  2012.10.10		�L���v�V�����̃o�b�t�@�T�C�Y��256�o�C�g�ɑ���
 */

#ifdef _WIN32

#define STRICT
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winbase.h>
#include <commctrl.h>
#include <stdlib.h>
#include <assert.h>
#include "xm7.h"
#include "keyboard.h"
#include "tapelp.h"
#include "display.h"
#include "ttlpalet.h"
#include "apalet.h"
#include "subctrl.h"
#include "fdc.h"
#include "bubble.h"
#include "w32.h"
#include "w32_bar.h"
#include "w32_sch.h"
#include "w32_draw.h"
#include "w32_res.h"

/*
 *	�O���[�o�� ���[�N
 */
HWND hStatusBar;						/* �X�e�[�^�X�o�[ */
BOOL bStatusMessage;					/* �X�e�[�^�X���b�Z�[�W�\����� */
int uPaneX[3];							/* Drag&Drop�p�y�C���ʒu */

/*
 *	�X�^�e�B�b�N ���[�N
 */
static char szIdleMessage[128];			/* �A�C�h�����b�Z�[�W */
static char szStatusMessage[128];		/* �X�e�[�^�X���b�Z�[�W */
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
static char szDrive[2][128];			/* �t���b�s�[�h���C�u */
static int nTape;						/* �e�[�v */
static UINT uResID;						/* ���\�[�XID�ۑ� */
static int nStatusBorder;				/* �X�e�[�^�X�o�[�̃{�[�_�[�`���� */
static BOOL (*IsAppThemed)(void);		/* �e�[�}�K�p�m�F�֐�(uxtheme.dll) */
static HINSTANCE hInstTheme;			/* uxtheme.dll�̃n���h�� */
static BOOL bThemed;					/* �e�[�}�K�p��� */

/*-[ �X�e�[�^�X�o�[ ]-------------------------------------------------------*/

/*
 *	�A�N�Z�X�}�N��
 */
#define Status_SetParts(hwnd, nParts, aWidths) \
	SendMessage((hwnd), SB_SETPARTS, (WPARAM) nParts, (LPARAM) (LPINT) aWidths)

#define Status_SetText(hwnd, iPart, uType, szText) \
	SendMessage((hwnd), SB_SETTEXT, (WPARAM) (iPart | uType), (LPARAM) (LPSTR) szText)

/*
 *	�y�C����`
 */
#define PANE_DEFAULT	0
#define PANE_DRIVE1		1
#define PANE_DRIVE0		2
#define PANE_TAPE		3
#define PANE_CAP		4
#define PANE_KANA		5
#define PANE_INS		6


/*
 *	�X�e�[�^�X�o�[�쐬
 */
HWND FASTCALL CreateStatus(HWND hParent)
{
	HWND hWnd;

	ASSERT(hParent);

	/* �e�[�}�K�p�t���O������ */
	bThemed = FALSE;

	/* ���b�Z�[�W�����[�h */
	if (LoadString(hAppInstance, IDS_IDLEMESSAGE,
					szIdleMessage, sizeof(szIdleMessage)) == 0) {
		szIdleMessage[0] = '\0';
	}
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

	/* �X�e�[�^�X���b�Z�[�W�������� */
	szStatusMessage[0] = '\0';
	bStatusMessage = FALSE;
	uResID = 0;

	/* �X�e�[�^�X�o�[���쐬 */
	hWnd = CreateStatusWindow(WS_CHILD | CCS_BOTTOM | CCS_NORESIZE,
								"", hParent, ID_STATUS_BAR);
	ChangeStatusBorder(hWnd);
	ResizeStatus(hParent, hWnd);
	ShowWindow(hWnd, SW_HIDE);

	return hWnd;
}

/*
 *	�L���v�V�����`��
 */
static void FASTCALL DrawMainCaption(void)
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

#if XM7_VER == 1 
#if defined(BUBBLE)
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
#endif

	/* ��r�`�� */
	string[255] = '\0';
	if (memcmp(szCaption, string, strlen(string) + 1) != 0) {
		strncpy(szCaption, string, sizeof(szCaption));
		SetWindowText(hMainWnd, szCaption);
	}
}

/*
 *	�A�C�R�����`�F�b�N�A�ύX
 */
static void FASTCALL CheckIcon(void)
{
	HICON hIcon;

#if XM7_VER == 1
	/* ��v�`�F�b�N */
	if (fm_subtype == nAppIcon) {
		return;
	}

	/* �V�A�C�R�������[�h�A�Z�b�g */
	switch (fm_subtype) {
		case FMSUB_FM8:
			hIcon = LoadIcon(hAppInstance, MAKEINTRESOURCE(IDI_FM8ICON));
			break;
		case FMSUB_FM7:
			hIcon = LoadIcon(hAppInstance, MAKEINTRESOURCE(IDI_FM7ICON));
			break;
		case FMSUB_FM77:
			hIcon = LoadIcon(hAppInstance, MAKEINTRESOURCE(IDI_FM77ICON));
			break;
		default:
			hIcon = NULL;
			ASSERT(FALSE);
			break;
	}
	SendMessage(hMainWnd, WM_SETICON, ICON_BIG, (LPARAM)hIcon);
	SendMessage(hMainWnd, WM_SETICON, ICON_SMALL, (LPARAM)hIcon);

	/* ���A�C�R�����폜 */
	DestroyIcon(hAppIcon);

	/* �L�� */
	hAppIcon = hIcon;
	nAppIcon = fm_subtype;
#else
	/* ��v�`�F�b�N */
	if (fm7_ver == nAppIcon) {
		return;
	}

	/* �V�A�C�R�������[�h�A�Z�b�g */
	switch (fm7_ver) {
		case 1:
			hIcon = LoadIcon(hAppInstance, MAKEINTRESOURCE(IDI_FM7ICON));
			break;
		case 2:
			hIcon = LoadIcon(hAppInstance, MAKEINTRESOURCE(IDI_77AVICON));
			break;
#if XM7_VER >= 3
		case 3:
			hIcon = LoadIcon(hAppInstance, MAKEINTRESOURCE(IDI_40EXICON2));
			break;
#endif
		default:
			hIcon = NULL;
			ASSERT(FALSE);
			break;
	}
	SendMessage(hMainWnd, WM_SETICON, ICON_BIG, (LPARAM)hIcon);
	SendMessage(hMainWnd, WM_SETICON, ICON_SMALL, (LPARAM)hIcon);

	/* ���A�C�R�����폜 */
	DestroyIcon(hAppIcon);

	/* �L�� */
	hAppIcon = hIcon;
	nAppIcon = fm7_ver;
#endif
}

/*
 *	CAP�L�[�`��
 */
static void FASTCALL DrawCAP(void)
{
	int num;

	/* �ԍ����� */
	if (caps_flag) {
		num = 1;
	}
	else {
		num = 0;
	}

	/* �����Ȃ牽�����Ȃ� */
	if (nCAP == num) {
		return;
	}

	/* �`��A���[�N�X�V */
	nCAP = num;
	Status_SetText(hStatusBar, PANE_CAP, SBT_OWNERDRAW, PANE_CAP);
}

/*
 *	���ȃL�[�`��
 */
static void FASTCALL DrawKANA(void)
{
	int num;

	/* �ԍ����� */
	if (kana_flag) {
		num = 1;
	}
	else {
		num = 0;
	}

	/* �����Ȃ牽�����Ȃ� */
	if (nKANA == num) {
		return;
	}

	/* �`��A���[�N�X�V */
	nKANA = num;
	Status_SetText(hStatusBar, PANE_KANA, SBT_OWNERDRAW, PANE_KANA);
}

/*
 *	INS�L�[�`��
 */
static void FASTCALL DrawINS(void)
{
	int num;

	/* �ԍ����� */
	if (ins_flag) {
		num = 1;
	}
	else {
		num = 0;
	}

	/* �����Ȃ牽�����Ȃ� */
	if (nINS == num) {
		return;
	}

	/* �`��A���[�N�X�V */
	nINS = num;
	Status_SetText(hStatusBar, PANE_INS, SBT_OWNERDRAW, PANE_INS);
}

/*
 *	�h���C�u�`��
 */
static void FASTCALL DrawDrive(int drive, UINT nID)
{
	char string[_MAX_FNAME + _MAX_EXT];
	char buffer[_MAX_FNAME + _MAX_EXT + 4];
	char drive_[_MAX_DRIVE];
	char dir[_MAX_DIR];
	char fname[_MAX_FNAME];
	char ext[_MAX_EXT];
	int num;
	char *name;

	ASSERT((drive >= 0) && (drive <= 1));

	/* �ԍ��Z�b�g */
	if ((fdc_ready[drive] == FDC_TYPE_NOTREADY) || (fdc_teject[drive])) {
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
			return;
		}
	}

	/* �`�� */
	nDrive[drive] = num;
	strncpy(szDrive[drive], name, sizeof(szDrive[drive]));
	Status_SetText(hStatusBar, nID, SBT_OWNERDRAW, nID);
}

/*
 *	�e�[�v�`��
 */
static void FASTCALL DrawTape(void)
{
	int num;

	/* �i���o�[�v�Z */
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
		return;
	}

	/* �`�� */
	nTape = num;
	Status_SetText(hStatusBar, PANE_TAPE, SBT_OWNERDRAW, PANE_TAPE);
}

/*
 *	�`��
 */
void FASTCALL DrawStatus(void)
{
	/* �E�C���h�E�`�F�b�N */
	if (!hMainWnd) {
		return;
	}

	DrawMainCaption();
	CheckIcon();

	/* �S��ʁA�X�e�[�^�X�o�[�`�F�b�N */
	if (bFullScreen || !hStatusBar) {
		return;
	}

	DrawCAP();
	DrawKANA();
	DrawINS();
	DrawDrive(0, PANE_DRIVE0);
	DrawDrive(1, PANE_DRIVE1);
	DrawTape();
}

/*
 *	�ĕ`��
 */
void FASTCALL PaintStatus(void)
{
	/* �L�����[�N�����ׂăN���A���� */
	szCaption[0] = '\0';
	nCAP = -1;
	nKANA = -1;
	nINS = -1;
	nDrive[0] = -1;
	nDrive[1] = -1;
	szDrive[0][0] = '\0';
	szDrive[1][0] = '\0';
	nTape = -1;

	/* �`�� */
	DrawStatus();
}

/*
 *	�L�����N�^�`��
 */
static void FASTCALL DrawChar(HDC hDC, BYTE c, int x, int y, UINT color)
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
 *	�I�[�i�[�h���[
 */
void FASTCALL OwnerDrawStatus(DRAWITEMSTRUCT *pDI)
{
	HBRUSH hBrush;
	COLORREF fColor;
	COLORREF bColor;
	RECT srect;
	char string[_MAX_FNAME + _MAX_EXT + 4];
	int drive;

	ASSERT(pDI);

	/* ������A�F������ */
	switch (pDI->itemData) {
		/* �t���b�s�[�f�B�X�N */
		case PANE_DRIVE1:
		case PANE_DRIVE0:
			if (pDI->itemData == PANE_DRIVE0) {
				drive = 0;
			}
			else {
				drive = 1;
			}
			if ((nDrive[drive] == FDC_ACCESS_NOTREADY) &&
				(!fdc_teject[drive])) {
				strncpy(string, "", sizeof(string));
			}
			else {
				strncpy(string, szDrive[drive], sizeof(string));
			}
			fColor = RGB(255, 255, 255);
			bColor = RGB(63, 63, 63);
			if ((nDrive[drive] == FDC_ACCESS_NOTREADY) ||
				(fdc_teject[drive])) {
				bColor = RGB(0, 0, 0);
			}
			if (nDrive[drive] == FDC_ACCESS_READ) {
				bColor = RGB(191, 0, 0);
			}
			if (nDrive[drive] == FDC_ACCESS_WRITE) {
				bColor = RGB(0, 0, 191);
			}
			break;

		/* �e�[�v */
		case PANE_TAPE:
			if (nTape >= 30000) {
				string[0] = '\0';
			}
			else {
				_snprintf(string, sizeof(string), "%04d", nTape % 10000);
			}
			fColor = RGB(255, 255, 255);
			bColor = RGB(63, 63, 63);
			if (nTape >= 10000) {
				if (nTape >= 30000) {
					bColor = RGB(0, 0, 0);
				}
				else if (nTape >= 20000) {
					bColor = RGB(0, 0, 191);
				}
				else {
					bColor = RGB(191, 0, 0);
				}
			}
			break;

		/* CAP */
		case PANE_CAP:
			fColor = RGB(255, 255, 255);
			strncpy(string, szCAPMessage, sizeof(string));
			if (nCAP) {
				bColor = RGB(255, 0, 0);
			}
			else {
				bColor = RGB(0, 0, 0);
			}
			break;

		/* ���� */
		case PANE_KANA:
			fColor = RGB(255, 255, 255);
			strncpy(string, szKANAMessage, sizeof(string));
			if (nKANA) {
				bColor = RGB(255, 0, 0);
			}
			else {
				bColor = RGB(0, 0, 0);
			}
			break;

		/* INS */
		case PANE_INS:
			fColor = RGB(255, 255, 255);
			strncpy(string, szINSMessage, sizeof(string));
			if (nINS) {
				bColor = RGB(255, 0, 0);
			}
			else {
				bColor = RGB(0, 0, 0);
			}
			break;

		/* ����ȊO */
		default:
			ASSERT(FALSE);
			return;
	}

	/* �u���V�œh�� */
	hBrush = CreateSolidBrush(bColor);
	if (hBrush) {
		FillRect(pDI->hDC, &(pDI->rcItem), hBrush);
		DeleteObject(hBrush);
	}

	/* �e�L�X�g�̉e��`�� */
	srect.left = pDI->rcItem.left + 1;
	srect.right = pDI->rcItem.right + 1;
	srect.top = pDI->rcItem.top + 1;
	srect.bottom = pDI->rcItem.bottom + 1;
	SetTextColor(pDI->hDC, RGB(0, 0, 0));
	SetBkMode(pDI->hDC, TRANSPARENT);
	DrawText(pDI->hDC, string, strlen(string), &srect,
				DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);

	/* �e�L�X�g��`�� */
	SetTextColor(pDI->hDC, fColor);
	SetBkMode(pDI->hDC, TRANSPARENT);	/* for WinXP Luna */
	DrawText(pDI->hDC, string, strlen(string), &(pDI->rcItem),
				DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);

	/* �h���C�u�ԍ��`�� */
	if ((pDI->itemData == PANE_DRIVE1) || (pDI->itemData == PANE_DRIVE0)) {
		DrawChar(pDI->hDC, (BYTE)(0x30 + drive),
				 pDI->rcItem.right - 8, pDI->rcItem.bottom - 8,
				 RGB(255, 255, 255));
	}
}

/*
 *	�T�C�Y�ύX
 */
void FASTCALL SizeStatus(LONG cx)
{
	HDC hDC;
	TEXTMETRIC tm;
	LONG cw;
	UINT uPane[7];

	ASSERT(cx > 0);
	ASSERT(hStatusBar);

	/* �e�L�X�g���g���b�N���擾 */
	hDC = GetDC(hStatusBar);
	GetTextMetrics(hDC, &tm);
	ReleaseDC(hStatusBar, hDC);
	cw = tm.tmAveCharWidth;

	/* �y�C���T�C�Y������(�y�C���E�[�̈ʒu��ݒ�) */
	uPane[PANE_INS] = cx;
	uPane[PANE_KANA] = uPane[PANE_INS] - cw * 4;
	uPane[PANE_CAP] = uPane[PANE_KANA] - cw * 4;
	uPane[PANE_TAPE] = uPane[PANE_CAP] - cw * 4;
	uPane[PANE_DRIVE0] = uPane[PANE_TAPE] - cw * 5;
	uPane[PANE_DRIVE1] = uPane[PANE_DRIVE0] - cw * 16;
	uPane[PANE_DEFAULT] = uPane[PANE_DRIVE1] - cw * 16;

	/* Drag&Drop�p�y�C�����W��ݒ� */
	uPaneX[0] = uPane[PANE_DEFAULT];
	uPaneX[1] = uPane[PANE_DRIVE1];
	uPaneX[2] = uPane[PANE_DRIVE0];

	/* �y�C���T�C�Y�ݒ� */
	Status_SetParts(hStatusBar, sizeof(uPane)/sizeof(UINT), uPane);

	/* �ĕ`�� */
	PaintStatus();
}

/*
 *	�X�e�[�^�X�o�[���T�C�Y
 */
void FASTCALL ResizeStatus(HWND hwnd, HWND hstatus)
{
	RECT rect;
	int height;

	/* �w�i�F�ύX */
	SendMessage(hstatus, SB_SETBKCOLOR, 0, GetSysColor(COLOR_3DFACE));

	/* ������␳ */
	GetWindowRect(hMainWnd, &rect);
	if (bDoubleSize) {
		height = 800;
	}
	else {
		height = 400;
	}
	MoveWindow(hstatus, 0, height, (rect.right - rect.left),
			   GetSystemMetrics(SM_CYMENU), TRUE);

	/* �t���[���E�C���h�E�̃T�C�Y��␳ */
	GetClientRect(hwnd, &rect);
	PostMessage(hwnd, WM_SIZE, 0, MAKELPARAM(rect.right, rect.bottom));
}

/*
 *	�X�e�[�^�X���b�Z�[�W�\��
 */
void FASTCALL SetStatusMessage(UINT ID)
{
	ASSERT(hStatusBar);

	/* �X�e�[�^�X�o�[���\������Ă��Ȃ���΁A�������Ȃ� */
	if (!IsWindowVisible(hStatusBar)) {
		return;
	}

	/* �w��ID�̕����񃊃\�[�X���[�h�����݂� */
	if (LoadString(hAppInstance, ID, szStatusMessage, sizeof(szStatusMessage)) != 0) {
		/* �X�e�[�^�X���b�Z�[�W���Z�b�g */
		Status_SetText(hStatusBar, 0, nStatusBorder, szStatusMessage);

		/* �X�e�[�^�X���b�Z�[�W�p�^�C�}��ݒ肷�� */
		if (bStatusMessage) {
			KillTimer(hMainWnd, ID_STATUS_BAR);
		}
		SetTimer(hMainWnd, ID_STATUS_BAR, 1500, NULL);

		/* �X�e�[�^�X���b�Z�[�W�\���t���O�𗧂Ă� */
		bStatusMessage = TRUE;
	}
	else {
		/* ���\�[�X���[�h�Ɏ��s�����ꍇ�A�\���t���O���~�낷 */
		bStatusMessage = FALSE;
	}
}

/*
 *	�X�e�[�^�X���b�Z�[�W�I��
 */
void FASTCALL EndStatusMessage(void)
{
	char buffer[128];

	ASSERT(hStatusBar);

	/* ���Ƃ��ƃX�e�[�^�X���b�Z�[�W���\������ĂȂ���΁A�������Ȃ� */
	if (!bStatusMessage) {
		return;
	}

	/* �X�e�[�^�X���b�Z�[�W�\���t���O���~�낷 */
	bStatusMessage = FALSE;

	/* �X�e�[�^�X�o�[���\������Ă��Ȃ���΁A�������Ȃ� */
	if (!IsWindowVisible(hStatusBar)) {
		return;
	}

	if (uResID != 0) {
		/* �����񃊃\�[�X���[�h�����݂� */
		if (LoadString(hAppInstance, uResID, buffer, sizeof(buffer)) == 0) {
			buffer[0] = '\0';
		}
	}
	else {
		/* �A�C�h�����b�Z�[�W���R�s�[ */
		strncpy(buffer, szIdleMessage, sizeof(buffer));
	}

	/* �Z�b�g */
	Status_SetText(hStatusBar, 0, nStatusBorder, buffer);
}

/*
 *	���j���[�Z���N�g
 */
void FASTCALL OnMenuSelect(WPARAM wParam)
{
	char buffer[128];
	UINT uID;

	ASSERT(hStatusBar);

	/* �X�e�[�^�X�o�[���\������Ă��Ȃ���΁A�������Ȃ� */
	if (!IsWindowVisible(hStatusBar)) {
		return;
	}

	/* ���[�h���郊�\�[�XID������ */
	uID = (UINT)LOWORD(wParam);
	if (((uID >= IDM_D0MEDIA00) && (uID <= IDM_D0MEDIA15)) ||
		((uID >= IDM_D1MEDIA00) && (uID <= IDM_D1MEDIA15))) {
		/* ���f�B�A���� */
		uID = IDS_MEDIA_CHANGE;
	}

	/* �����񃊃\�[�X���[�h�����݂� */
	if (LoadString(hAppInstance, uID, buffer, sizeof(buffer)) == 0) {
		buffer[0] = '\0';
	}

	/* �Z�b�g */
	Status_SetText(hStatusBar, 0, nStatusBorder, buffer);

	/* ���\�[�XID(�������[�h�t���O)��ۑ� */
	uResID = uID;
}

/*
 *	���j���[�I��
 */
void FASTCALL OnExitMenuLoop(void)
{
	ASSERT(hStatusBar);

	/* �X�e�[�^�X�o�[���\������Ă��Ȃ���΁A�������Ȃ� */
	if (!IsWindowVisible(hStatusBar)) {
		return;
	}

	/* �A�C�h�����b�Z�[�W��\�� */
	if (bStatusMessage) {
		Status_SetText(hStatusBar, 0, nStatusBorder, szStatusMessage);
	}
	else {
		Status_SetText(hStatusBar, 0, nStatusBorder, szIdleMessage);
	}

	/* ���\�[�XID(�������[�h�t���O)��ۑ� */
	uResID = 0;
}

/*-[ WindowsXP�Ή����� ]----------------------------------------------------*/

/*
 *	WindowsXP �r�W���A���X�^�C���Ή��p���� 08/14
 *	UXTHEME.DLL������
 */
void FASTCALL InitThemeDLL(void)
{
	/* uxtheme.dll�����[�h */
	hInstTheme = LoadLibrary("uxtheme.dll");

	/* �֐��̐擪�A�h���X��ݒ� */
	if (hInstTheme) {
		IsAppThemed = (BOOL(*)(void))GetProcAddress(hInstTheme, "IsAppThemed");

		if (!IsAppThemed) {
			/* ���s */
			CleanThemeDLL();
		}
	}
}

/*
 *	WindowsXP �r�W���A���X�^�C���Ή��p����
 *	UXTHEME.DLL�N���[���A�b�v
 */
void FASTCALL CleanThemeDLL(void)
{
	/* DLL���ǂݍ��܂�Ă�����J�� */
	if (hInstTheme) {
		FreeLibrary(hInstTheme);
		IsAppThemed = NULL;
		hInstTheme = NULL;
	}
}

/*
 *	WindowsXP �r�W���A���X�^�C���Ή��p����
 *	�X�e�[�^�X���b�Z�[�W���{�[�_�[�`���ԕύX
 */
void FASTCALL ChangeStatusBorder(HWND hwnd)
{
	/* �e�[�}�K�p��Ԃł͘g��`�悷�� */
	if (hInstTheme && IsAppThemed) {
		if (!IsAppThemed()) {
			nStatusBorder = SBT_NOBORDERS;
			bThemed = FALSE;
		}
		else {
			nStatusBorder = 0;
			bThemed = TRUE;
		}
	}
	else {
		nStatusBorder = SBT_NOBORDERS;
		bThemed = FALSE;
	}

	/* �A�C�h�����b�Z�[�W��\�� */
	if (bStatusMessage) {
		Status_SetText(hwnd, 0, nStatusBorder, szStatusMessage);
	}
	else {
		Status_SetText(hwnd, 0, nStatusBorder, szIdleMessage);
	}
}

#endif	/* _WIN32 */
