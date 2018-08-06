/*
 *	FM-7 EMULATOR "XM7"
 *
 *	Copyright (C) 1999-2017 �o�h�D(Twitter:@xm6_original)
 *	Copyright (C) 2001-2017 Ryu Takegami (Twitter:@RyuTakegami)
 *
 *	[ Win32API �T�u�E�B���h�E�R ]
 *
 *	RHG����
 *	  2001.07.25		V2����MMR�E�B���h�E�̃T�C�Y�𒲐�
 *	  2001.11.15		DMAC�E�B���h�E��ǉ�
 *	  2002.07.17		�g�p���Ă��Ȃ�DMAC�E�B���h�E�ł̔��]�\����������p�~
 *	  2013.12.14		V3�ł̃������Ǘ��E�B���h�E���o�J�ł������Ă݂�
 *	  2015.03.13		�T�u�E�C���h�E�̃|�b�v�A�b�v�Ή�
 */

#ifdef _WIN32

#define STRICT
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <assert.h>
#include <stdlib.h>
#include "xm7.h"
#include "keyboard.h"
#include "rtc.h"
#include "mmr.h"
#if XM7_VER >= 3
#include "jcard.h"
#include "dmac.h"
#endif
#include "subctrl.h"
#include "display.h"
#include "multipag.h"
#include "ttlpalet.h"
#include "apalet.h"
#include "mouse.h"
#include "w32.h"
#include "w32_res.h"
#include "w32_sub.h"

/*
 *	�O���[�o�� ���[�N
 */
BOOL bPaletteRefresh;					/* �p���b�g���t���b�V�� */

/*
 *	�X�^�e�B�b�N ���[�N
 */
static BYTE *pKeyboard;					/* �L�[�{�[�h Draw�o�b�t�@ */
static BYTE *pMMR;						/* MMR Draw�o�b�t�@ */
#if XM7_VER >= 3
static BYTE *pDMAC;						/* DMAC Draw�o�b�t�@ */
#endif
static BYTE *pPaletteReg;				/* �p���b�g���W�X�^ �r�b�g�}�b�v�o�b�t�@ */
static HBITMAP hPaletteReg;				/* �p���b�g���W�X�^ �r�b�g�}�b�v�n���h�� */
static RECT rPaletteReg;				/* �p���b�g���W�X�^ ��` */

/*
 *	�p���b�g���W�X�^�E�C���h�E�p�F�e�[�u��
 */
static const DWORD Palet16Tc[] = {
	0x00000000,	0x000000ff,	0x00ff0000,	0x00ff00ff,
	0x0000ff00,	0x0000ffff,	0x00ffff00,	0x00ffffff,
#if XM7_VER == 1 && defined(L4CARD)
	0x00000000,	0x000000bb,	0x00bb0000,	0x00bb00bb,
	0x0000bb00,	0x0000bbbb,	0x00bbbb00,	0x00bbbbbb,
	0x00444444,	0x004444ff,	0x00ff4444,	0x00ff44ff,
	0x0044ff44,	0x0044ffff,	0x00ffff44,	0x00ffffff,
#endif
};

/*-[ MMR�E�C���h�E ]---------------------------------------------------------*/

/*
 *	MMR�E�C���h�E
 *	MMR�}�b�v �Z�b�g�A�b�v
 */
static void FASTCALL SetupMMRMap(BYTE *p, int y, int x)
{
	char string[128];
	char temp[16];
	int i, j;
	BYTE tmp;

	ASSERT(p);
	ASSERT(y > 0);
	ASSERT(x > 0);

	/* �^�C�g�� */
	strncpy(string, "MMR Map:", sizeof(string));
	memcpy(&p[x * y], string, strlen(string));
	y++;

	/* �ey���Ƃ� */
#if XM7_VER >= 3
	for (i=0; i<8; i++) {
#else
	for (i=0; i<4; i++) {
#endif
		/* 16�i�_���v���쐬 */
		_snprintf(string, sizeof(string), "Segment %d-", i);
		for (j=0; j<16; j++) {
			tmp = mmr_reg[i * 0x10 + j];
#if XM7_VER >= 3
			if (!mmr_ext) {
				tmp &= 0x3f;
			}
#else
			tmp &= 0x3f;
#endif
			_snprintf(temp, sizeof(temp), "%02X", tmp);
			strncat(string, temp, sizeof(string) - strlen(string) - 1);
		}

		/* MMR�g�p���ŃZ�O�����g���v����΁A���]������ */
		if (mmr_flag && (mmr_seg == i)) {
			for (j=10;;j++) {
				if (string[j] == '\0') {
					break;
				}
				string[j] |= 0x80;
			}
		}

		/* �Z�b�g */
		memcpy(&p[x * y], string, strlen(string));
		y++;
	}
}

/*
 *	MMR�E�C���h�E
 *	�������}�b�v �Z�b�g�A�b�v
 */
static void FASTCALL SetupMMRMemory(BYTE *p, int y, int x)
{
	char string[128];
	char temp[16];
	BYTE mem[16];
	int i, j, k;

	ASSERT(p);
	ASSERT(y > 0);
	ASSERT(x > 0);

	/* mem��16�o�C�g�ɁA��������Ԃ�ݒ� */
	for (i=0; i<16; i++) {
		mem[i] = (BYTE)(0x30 + i);
	}
	/* MMR */
	if (mmr_flag) {
		for (i=0; i<16; i++) {
#if XM7_VER >= 3
			if (mmr_ext) {
				mem[i] = mmr_reg[mmr_seg * 0x10 + i];
			}
			else {
				mem[i] = (BYTE)(mmr_reg[(mmr_seg & 3) * 0x10 + i] & 0x3f);
			}
#else
			mem[i] = (BYTE)(mmr_reg[(mmr_seg & 3) * 0x10 + i] & 0x3f);
#endif
		}
	}

	/* �^�C�g�� */
	strncpy(string, "Physical Memory Map:", sizeof(string));
	memcpy(&p[x * y], string, strlen(string));
	y++;

	/* �ey���Ƃ� */
#if XM7_VER == 3
	for (i=0; i<16; i++) {
#else
	for (i=0; i<4; i++) {
#endif
		/* 16�i�_���v���쐬 */
		switch (i) {
#if XM7_VER >= 2
			case 0:
#if XM7_VER == 3
				strncpy(string, "Extend 0  ", sizeof(string));
#else
				strncpy(string, "Extend    ", sizeof(string));
#endif
				break;
			case 1:
				strncpy(string, "Sub CPU   ", sizeof(string));
				break;
			case 2:
				strncpy(string, "Japanese  ", sizeof(string));
				break;
			/* ����͂Ђǂ��� */
#if XM7_VER == 3
			case 4:
			case 5:
			case 6:
			case 7:
			case 8:
			case 9:
			case 10:
			case 11:
			case 12:
			case 13:
			case 14:
			case 15:
				_snprintf(string, sizeof(string),  "Extend %X  ", i);
				break;
#endif
#else
			case 0:
			case 1:
			case 2:
				_snprintf(string, sizeof(string),  "Extend %X  ", i);
				break;
#endif
			case 3:
				strncpy(string, "Standard  ", sizeof(string));
				break;
			default:
				/* ���肦�Ȃ��� */
				ASSERT(FALSE);
		}
		for (j=0; j<16; j++) {
			_snprintf(temp, sizeof(temp), "%02X", i * 0x10 + j);
			/* ���]���� */
			for (k=0; k<16; k++) {
				if (mem[k] == (i * 0x10 + j)) {
					temp[0] |= 0x80;
					temp[1] |= 0x80;
				}
			}
			/* �ǉ� */
			strncat(string, temp, sizeof(string) - strlen(string) - 1);
		}

		/* �Z�b�g */
		memcpy(&p[x * y], string, strlen(string));
		y++;
	}
}

/*
 *	MMR�E�C���h�E
 *	�Z�b�g�A�b�v
 */
static void FASTCALL SetupMMR(BYTE *p, int x, int y)
{
	char string[128];

	ASSERT(p);
	ASSERT(x > 0);
	ASSERT(y > 0);

	/* ��U�X�y�[�X�Ŗ��߂� */
	memset(p, 0x20, x * y);

	/* MMR */
	if (mmr_flag) {
		_snprintf(string, sizeof(string),  "MMR  Enable(Seg:%1d)", mmr_seg);
	}
	else {
		strncpy(string, "MMR        Disable", sizeof(string));
	}
	memcpy(&p[x * 0 + 0], string, strlen(string));

	/* TWR */
	if (twr_flag) {
		_snprintf(string, sizeof(string),  "TWR  Enable($%02X00)",
			(twr_reg + 0x7c) & 0xff);
	}
	else {
		strncpy(string, "TWR        Disable", sizeof(string));
	}
	memcpy(&p[x * 1 + 0], string, strlen(string));

#if XM7_VER >= 3
	/* ����ROM */
	if (!dicrom_en) {
		strncpy(string, "DIC        Disable", sizeof(string));
	}
	else {
		if (extrom_sel) {
			_snprintf(string, sizeof(string),  "DIC  ExtROM(Bank %2d)",
				dicrom_bank);
		}
		else {
			_snprintf(string, sizeof(string),  "DIC  Enable(Bank %2d)",
				dicrom_bank);
		}
	}
	memcpy(&p[x * 2 + 0], string, strlen(string));
#endif

#if XM7_VER >= 2
	/* �u�[�gRAM */
	strncpy(string, "Boot RAM         R/", sizeof(string));
	if (bootram_rw) {
		strncat(string, "W", sizeof(string) - strlen(string) - 1);
	}
	else {
		strncat(string, "O", sizeof(string) - strlen(string) - 1);
	}
	memcpy(&p[x * 0 + 22], string, strlen(string));

	/* �C�j�V�G�[�gROM */
	strncpy(string, "Initiate ROM ", sizeof(string));
	if (initrom_en) {
		strncat(string, " Enable", sizeof(string) - strlen(string) - 1);
	}
	else {
		strncat(string, "Disable", sizeof(string) - strlen(string) - 1);
	}
	memcpy(&p[x * 1 + 22], string, strlen(string));
#else
	/* F-BASIC ROM */
	strncpy(string, "F-BASIC ROM  ", sizeof(string));
	if (!basicrom_en) {
		strncat(string, "Disable", sizeof(string) - strlen(string) - 1);
	}
	else {
		strncat(string, " Enable", sizeof(string) - strlen(string) - 1);
	}
	memcpy(&p[x * 0 + 22], string, strlen(string));

	/* �u�[�gROM/RAM */
	strncpy(string, "Boot Area        ", sizeof(string));
	if (bootram_rw) {
		strncat(string, "RAM", sizeof(string) - strlen(string) - 1);
	}
	else {
		strncat(string, "ROM", sizeof(string) - strlen(string) - 1);
	}
	memcpy(&p[x * 1 + 22], string, strlen(string));
#endif

#if XM7_VER >= 3
	/* �g��MMR */
	strncpy(string, "Extend MMR   ", sizeof(string));
	if (mmr_ext) {
		strncat(string, " Enable", sizeof(string) - strlen(string) - 1);
	}
	else {
		strncat(string, "Disable", sizeof(string) - strlen(string) - 1);
	}
	memcpy(&p[x * 2 + 22], string, strlen(string));

	/* MMR�}�b�v */
	SetupMMRMap(p, 4, x);

	/* �������}�b�v */
	SetupMMRMemory(p, 14, x);
#else
	/* MMR�}�b�v */
	SetupMMRMap(p, 3, x);

	/* �������}�b�v */
	SetupMMRMemory(p, 9, x);
#endif
}

/*
 *	MMR�E�C���h�E
 *	�`��
 */
static void FASTCALL DrawMMR(HWND hWnd, HDC hDC)
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
	if (!pMMR) {
		return;
	}
	SetupMMR(pMMR, x, y);

	/* �`�� */
	DrawWindowText2(hDC, pMMR, x, y);
}

/*
 *	MMR�E�C���h�E
 *	���t���b�V��
 */
void FASTCALL RefreshMMR(void)
{
	HWND hWnd;
	HDC hDC;

	/* ��ɌĂ΂��̂ŁA���݃`�F�b�N���邱�� */
	if (hSubWnd[SWND_MMR] == NULL) {
		return;
	}

	/* �`�� */
	hWnd = hSubWnd[SWND_MMR];
	hDC = GetDC(hWnd);
	DrawMMR(hWnd, hDC);
	ReleaseDC(hWnd, hDC);
}

/*
 *	MMR�E�C���h�E
 *	�ĕ`��
 */
static void FASTCALL PaintMMR(HWND hWnd)
{
	HDC hDC;
	PAINTSTRUCT ps;
	RECT rect;
	BYTE *p;
	int x, y;

	ASSERT(hWnd);

	/* �|�C���^��ݒ�(���݂��Ȃ���Ή������Ȃ�) */
	p = pMMR;
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
	DrawMMR(hWnd, hDC);
	EndPaint(hWnd, &ps);
}

/*
 *	MMR�E�C���h�E
 *	�E�C���h�E�v���V�[�W��
 */
static LRESULT CALLBACK MMRProc(HWND hWnd, UINT message,
								 WPARAM wParam, LPARAM lParam)
{
	/* ���b�Z�[�W�� */
	switch (message) {
		/* �E�C���h�E�ĕ`�� */
		case WM_PAINT:
			/* ���b�N���K�v */
			LockVM();
			PaintMMR(hWnd);
			UnlockVM();
			return 0;

		/* �E�C���h�E�폜 */
		case WM_DESTROY:
			LockVM();

			/* ���C���E�C���h�E�֎����ʒm */
			DestroySubWindow(hWnd, &pMMR, NULL);

			UnlockVM();
			break;
	}

	/* �f�t�H���g �E�C���h�E�v���V�[�W�� */
	return DefWindowProc(hWnd, message, wParam, lParam);
}

/*
 *	MMR�E�C���h�E
 *	�E�C���h�E�쐬
 */
HWND FASTCALL CreateMMR(HWND hParent, int index)
{
	WNDCLASSEX wcex;
	char szClassName[] = "XM7_MMR";
	char szWndName[128];
	RECT rect;
	RECT crect, wrect;
	HWND hWnd;
	DWORD dwStyle;
	
	ASSERT(hParent);

	/* �E�C���h�E��`���v�Z */
	PositioningSubWindow(hParent, &rect, index);
	rect.right = lCharWidth * 42;
#if XM7_VER >= 3
	rect.bottom = lCharHeight * 31;
#else
	rect.bottom = lCharHeight * 14;
#endif

	/* �E�C���h�E�^�C�g��������A�o�b�t�@�m�� */
	LoadString(hAppInstance, IDS_SWND_MMR,
				szWndName, sizeof(szWndName));
	pMMR = malloc(2 * (rect.right / lCharWidth) *
								(rect.bottom / lCharHeight));

	/* �E�C���h�E�N���X�̓o�^ */
	memset(&wcex, 0, sizeof(wcex));
	wcex.cbSize = sizeof(wcex);
	wcex.style = CS_VREDRAW | CS_HREDRAW;
	wcex.lpfnWndProc = MMRProc;
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

/*-[ �L�[�{�[�h�E�C���h�E ]--------------------------------------------------*/

/*
 *	�L�[�{�[�h�E�C���h�E
 *	�Z�b�g�A�b�v(�t���O)
 */
static void FASTCALL SetupKeyboardFlag(BYTE *p, char *title, BOOL flag)
{
	char string[20];
	int i;

	/* ������ */
	memset(string, 0x20, 18);
	string[17] = '\0';

	/* �R�s�[ */
	for (i=0; i<17; i++) {
		if (title[i] == '\0') {
			break;
		}
		string[i] = title[i];
	}

	/* �Z�b�g */
	memcpy(p, string, strlen(string));

	/* �t���O�ɉ����Đݒ� */
#if XM7_VER >= 2
	if (flag) {
		strncpy(&string[14], " On", sizeof(string)-14);
	}
	else {
		strncpy(&string[14], "Off", sizeof(string)-14);
	}

	/* �Z�b�g */
	memcpy(p, string, 17);
#else
	if (flag) {
		strncpy(&string[15], " On", sizeof(string)-15);
	}
	else {
		strncpy(&string[15], "Off", sizeof(string)-15);
	}

	/* �Z�b�g */
	memcpy(p, string, 18);
#endif
}


/*
 *	�L�[�{�[�h�E�C���h�E
 *	�Z�b�g�A�b�v
 */
static void FASTCALL SetupKeyboard(BYTE *p, int x, int y)
{
	char string[128];

	ASSERT(p);
	ASSERT(x > 0);
	ASSERT(y > 0);

	/* ��U�X�y�[�X�Ŗ��߂� */
	memset(p, 0x20, x * y);

#if XM7_VER >= 2
	/* �R�[�h */
	_snprintf(string, sizeof(string),  "Logical Code     %04X", key_fm7);
	memcpy(&p[x * 0 + 0], string, strlen(string));
	_snprintf(string, sizeof(string),  "Scan Code          %02X", key_scan);
	memcpy(&p[x * 1 + 0], string, strlen(string));

	/* �R�[�h�t�H�[�}�b�g */
	strncpy(string, "Code Format ", sizeof(string));
	switch (key_format) {
		case KEY_FORMAT_9BIT:
			strncat(string, "     FM-7", sizeof(string) - strlen(string) - 1);
			break;
		case KEY_FORMAT_FM16B:
			strncat(string, "FM-16Beta", sizeof(string) - strlen(string) - 1);
			break;
		case KEY_FORMAT_SCAN:
			strncat(string, "     Scan", sizeof(string) - strlen(string) - 1);
			break;
		default:
			ASSERT(FALSE);
	}
	memcpy(&p[x * 2 + 0], string, strlen(string));

	/* �L�[���s�[�g */
	strncpy(string, "Key Repeat", sizeof(string));
	memcpy(&p[x * 4 + 0], string, strlen(string));
	if (key_repeat_flag) {
		strncpy(string, " Enable", sizeof(string));
	}
	else {
		strncpy(string, "Disable", sizeof(string));
	}
	memcpy(&p[x * 4 + 14], string, strlen(string));
	_snprintf(string, sizeof(string),  "Repeat Time 1  %4dms",
		key_repeat_time1 / 1000);
	memcpy(&p[x * 5 + 0], string, strlen(string));
	_snprintf(string, sizeof(string),  "Repeat Time 2  %4dms",
		key_repeat_time2 / 1000);
	memcpy(&p[x * 6 + 0], string, strlen(string));

	/* �t���O */
	SetupKeyboardFlag(&p[x * 0 + 24], "CAP  LED" , caps_flag);
	SetupKeyboardFlag(&p[x * 1 + 24], "KANA LED", kana_flag);
	SetupKeyboardFlag(&p[x * 2 + 24], "INS  LED", ins_flag);
	SetupKeyboardFlag(&p[x * 3 + 24], "SHIFT", shift_flag);
	SetupKeyboardFlag(&p[x * 4 + 24], "CTRL", ctrl_flag);
	SetupKeyboardFlag(&p[x * 5 + 24], "GRAPH", graph_flag);
	SetupKeyboardFlag(&p[x * 6 + 24], "BREAK", break_flag);

	/* ���v */
	_snprintf(string, sizeof(string),  "RTC Date     %02d/%02d/%02d",
		rtc_year, rtc_month, rtc_day);
	memcpy(&p[x * 8 + 0], string, strlen(string));
	_snprintf(string, sizeof(string),  "RTC Time     %02d:%02d:%02d",
		rtc_hour, rtc_minute, rtc_second);
	memcpy(&p[x * 9 + 0], string, strlen(string));
	_snprintf(string, sizeof(string),  "RTC 12h/24h");
	memcpy(&p[x * 10 + 0], string, strlen(string));
	if (rtc_24h) {
		strncpy(string, "24h", sizeof(string));
	}
	else {
		strncpy(string, "12h", sizeof(string));
	}
	memcpy(&p[x * 10 + 18], string, strlen(string));
	strncpy(string, "RTC AM/PM", sizeof(string));
	memcpy(&p[x * 11 + 0], string, strlen(string));
	if (rtc_pm) {
		strncpy(string, "PM", 128);
	}
	else {
		strncpy(string, "AM", 128);
	}
	memcpy(&p[x * 11 + 19], string, strlen(string));
	strncpy(string, "RTC Leap Mode", sizeof(string));
	memcpy(&p[x * 12 + 0], string, strlen(string));
	_snprintf(string, sizeof(string),  "%1d", rtc_leap);
	memcpy(&p[x * 12 + 20], string, strlen(string));

	/* �X�[�p�[�C���|�[�Y */
	strncpy(string, "Super Impose   ", sizeof(string));
	switch (simpose_mode) {
		case 0:
		case 3:	/* �f�B�W�^�C�Y���[�h��PC�\���� */
			strncat(string, "PC", sizeof(string) - strlen(string) - 1);
			break;
		case 1:
			strncat(string, "On", sizeof(string) - strlen(string) - 1);
			break;
		case 2:
			strncat(string, "TV", sizeof(string) - strlen(string) - 1);
			break;
	}
	memcpy(&p[x * 8 + 24], string, strlen(string));
	strncpy(string, "Brightness   ", sizeof(string));
	if (simpose_half) {
		strncat(string, "Half", sizeof(string) - strlen(string) - 1);
	}
	else {
		strncat(string, "Full", sizeof(string) - strlen(string) - 1);
	}
	memcpy(&p[x * 9 + 24], string, strlen(string));

	/* �f�B�W�^�C�Y */
	strncpy(string, "Digitize  ", sizeof(string));
	if ((digitize_enable) && (simpose_mode == 3)) {
		strncat(string, " Enable", sizeof(string) - strlen(string) - 1);
	}
	else {
		strncat(string, "Disable", sizeof(string) - strlen(string) - 1);
	}
	memcpy(&p[x * 11 + 24], string, strlen(string));
	strncpy(string, "Key Wait    ", sizeof(string));
	if (digitize_keywait) {
		strncat(string, " Wait", sizeof(string) - strlen(string) - 1);
	}
	else {
		strncat(string, "Ready", sizeof(string) - strlen(string) - 1);
	}
	memcpy(&p[x * 12 + 24], string, strlen(string));
#else
	/* �R�[�h */
	_snprintf(string, sizeof(string),  "Logical Code  %04X", key_fm7);
	memcpy(&p[x * 0 + 0], string, strlen(string));
	_snprintf(string, sizeof(string),  "Scan Code       %02X", key_scan);
	memcpy(&p[x * 1 + 0], string, strlen(string));

	/* �L�[���s�[�g */
	strncpy(string, "Key Repeat", sizeof(string));
	memcpy(&p[x * 2 + 0], string, strlen(string));
	if (key_repeat_flag) {
		strncpy(string, " Enable", sizeof(string));
	}
	else {
		strncpy(string, "Disable", sizeof(string));
	}
	memcpy(&p[x * 2 + 11], string, strlen(string));

	/* �t���O */
	SetupKeyboardFlag(&p[x * 4], "CAP  LED" , caps_flag);
	SetupKeyboardFlag(&p[x * 5], "KANA LED", kana_flag);
	SetupKeyboardFlag(&p[x * 6], "INS  LED", ins_flag);
	SetupKeyboardFlag(&p[x * 3 + 23], "SHIFT", shift_flag);
	SetupKeyboardFlag(&p[x * 4 + 23], "CTRL", ctrl_flag);
	SetupKeyboardFlag(&p[x * 5 + 23], "GRAPH", graph_flag);
	SetupKeyboardFlag(&p[x * 6 + 23], "BREAK", break_flag);
#endif
}

/*
 *	�L�[�{�[�h�E�C���h�E
 *	�`��
 */
static void FASTCALL DrawKeyboard(HWND hWnd, HDC hDC)
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
	if (!pKeyboard) {
		return;
	}
	SetupKeyboard(pKeyboard, x, y);

	/* �`�� */
	DrawWindowText(hDC, pKeyboard, x, y);
}

/*
 *	�L�[�{�[�h�E�C���h�E
 *	���t���b�V��
 */
void FASTCALL RefreshKeyboard(void)
{
	HWND hWnd;
	HDC hDC;

	/* ��ɌĂ΂��̂ŁA���݃`�F�b�N���邱�� */
	if (hSubWnd[SWND_KEYBOARD] == NULL) {
		return;
	}

	/* �`�� */
	hWnd = hSubWnd[SWND_KEYBOARD];
	hDC = GetDC(hWnd);
	DrawKeyboard(hWnd, hDC);
	ReleaseDC(hWnd, hDC);
}

/*
 *	�L�[�{�[�h�E�C���h�E
 *	�ĕ`��
 */
static void FASTCALL PaintKeyboard(HWND hWnd)
{
	HDC hDC;
	PAINTSTRUCT ps;
	RECT rect;
	BYTE *p;
	int x, y;

	ASSERT(hWnd);

	/* �|�C���^��ݒ�(���݂��Ȃ���Ή������Ȃ�) */
	p = pKeyboard;
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
	DrawKeyboard(hWnd, hDC);
	EndPaint(hWnd, &ps);
}

/*
 *	�L�[�{�[�h�E�C���h�E
 *	�E�C���h�E�v���V�[�W��
 */
static LRESULT CALLBACK KeyboardProc(HWND hWnd, UINT message,
								 WPARAM wParam, LPARAM lParam)
{
	/* ���b�Z�[�W�� */
	switch (message) {
		/* �E�C���h�E�ĕ`�� */
		case WM_PAINT:
			/* ���b�N���K�v */
			LockVM();
			PaintKeyboard(hWnd);
			UnlockVM();
			return 0;

		/* �E�C���h�E�폜 */
		case WM_DESTROY:
			LockVM();

			/* ���C���E�C���h�E�֎����ʒm */
			DestroySubWindow(hWnd, &pKeyboard, NULL);

			UnlockVM();
			break;
	}

	/* �f�t�H���g �E�C���h�E�v���V�[�W�� */
	return DefWindowProc(hWnd, message, wParam, lParam);
}

/*
 *	�L�[�{�[�h�E�C���h�E
 *	�E�C���h�E�쐬
 */
HWND FASTCALL CreateKeyboard(HWND hParent, int index)
{
	WNDCLASSEX wcex;
	char szClassName[] = "XM7_Keyboard";
	char szWndName[128];
	RECT rect;
	RECT crect, wrect;
	HWND hWnd;
	DWORD dwStyle;

	ASSERT(hParent);

	/* �E�C���h�E��`���v�Z */
	PositioningSubWindow(hParent, &rect, index);
#if XM7_VER >= 2
	rect.right = lCharWidth * 41;
	rect.bottom = lCharHeight * 13;
#else
	rect.right = lCharWidth * 41;
	rect.bottom = lCharHeight * 7;
#endif

	/* �E�C���h�E�^�C�g��������A�o�b�t�@�m�� */
	LoadString(hAppInstance, IDS_SWND_KEYBOARD,
				szWndName, sizeof(szWndName));
	pKeyboard = malloc(2 * (rect.right / lCharWidth) *
								(rect.bottom / lCharHeight));

	/* �E�C���h�E�N���X�̓o�^ */
	memset(&wcex, 0, sizeof(wcex));
	wcex.cbSize = sizeof(wcex);
	wcex.style = CS_VREDRAW | CS_HREDRAW;
	wcex.lpfnWndProc = KeyboardProc;
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

/*-[ DMAC�E�C���h�E ]--------------------------------------------------------*/

#if XM7_VER >= 3
/*
 *	DMAC�E�C���h�E
 *	�Z�b�g�A�b�v
 */
static void FASTCALL SetupDMAC(BYTE *p, int x, int y)
{
	char string[128];

	ASSERT(p);
	ASSERT(x > 0);
	ASSERT(y > 0);

	/* ��U�X�y�[�X�Ŗ��߂� */
	memset(p, 0x20, x * y);

	/* CHCR - BUSY */
	strncpy(string, "Status", sizeof(string));
	memcpy(&p[x * 0 + 0], string, strlen(string));

	if (dma_chcr[0] & 0x40) {
		strncpy(string, " Busy", sizeof(string));
	}
	else {
		strncpy(string, "Ready", sizeof(string));
	}
	memcpy(&p[x * 0 + 25], string, strlen(string));

	/* CHCR - MODE */
	strncpy(string, "Request Mode", sizeof(string));
	memcpy(&p[x * 1 + 0], string, strlen(string));

	if (dma_chcr[0] & 0x02) {
		strncpy(string, "      Burst", sizeof(string));
	}
	else {
		strncpy(string, "Cycle Steal", sizeof(string));
	}
	memcpy(&p[x * 1 + 19], string, strlen(string));

	/* CHCR - R/W */
	strncpy(string, "Direction", sizeof(string));
	memcpy(&p[x * 2 + 0], string, strlen(string));

	if (dma_chcr[0] & 0x01) {
		strncpy(string, "Memory->Device", sizeof(string));
	}
	else {
		strncpy(string, "Device->Memory", sizeof(string));
	}
	memcpy(&p[x * 2 + 16], string, strlen(string));

	/* CHCR - DIRECTION */
	strncpy(string, "Address Inc/Dec", sizeof(string));
	memcpy(&p[x * 3 + 0], string, strlen(string));

	if (dma_chcr[0] & 0x08) {
		strncpy(string, "Decrement", sizeof(string));
	}
	else {
		strncpy(string, "Increment", sizeof(string));
	}
	memcpy(&p[x * 3 + 21], string, strlen(string));

	/* ADR */
	_snprintf(string, sizeof(string),  "Address                  $%04X",
		dma_adr[0]);
	memcpy(&p[x * 4 + 0], string, strlen(string));

	/* BCR */
	_snprintf(string, sizeof(string),  "Byte Count               $%04X",
		dma_bcr[0]);
	memcpy(&p[x * 5 + 0], string, strlen(string));

	/* CHCR - DEND */
	strncpy(string, "DEND", sizeof(string));
	memcpy(&p[x * 6 + 0], string, strlen(string));

	if (dma_chcr[0] & 0x80) {
		strncpy(string, " Enable", sizeof(string));
	}
	else {
		strncpy(string, "Disable", sizeof(string));
	}
	memcpy(&p[x * 6 + 23], string, strlen(string));

	/* PCR - TxEN */
	strncpy(string, "TxRQ", sizeof(string));
	memcpy(&p[x * 7 + 0], string, strlen(string));

	if (dma_pcr & 0x01) {
		strncpy(string, " Enable", sizeof(string));
	}
	else {
		strncpy(string, "Disable", sizeof(string));
	}
	memcpy(&p[x * 7 + 23], string, strlen(string));

	/* ICR - IRQ ENABLE */
	strncpy(string, "IRQ", sizeof(string));
	memcpy(&p[x * 8 + 0], string, strlen(string));

	if (dma_icr & 0x01) {
		if (dma_icr & 0x80) {
			strncpy(string, " Enable", sizeof(string));
		}
		else {
			strncpy(string, "Disable", sizeof(string));
		}
	}
	else {
		strncpy(string, "   Mask", sizeof(string));
	}
	memcpy(&p[x * 8 + 23], string, strlen(string));
}

/*
 *	DMAC�E�C���h�E
 *	�`��
 */
static void FASTCALL DrawDMAC(HWND hWnd, HDC hDC)
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
	if (!pDMAC) {
		return;
	}
	SetupDMAC(pDMAC, x, y);

	/* �`�� */
	DrawWindowText(hDC, pDMAC, x, y);
}

/*
 *	DMAC�E�C���h�E
 *	���t���b�V��
 */
void FASTCALL RefreshDMAC(void)
{
	HWND hWnd;
	HDC hDC;

	/* ��ɌĂ΂��̂ŁA���݃`�F�b�N���邱�� */
	if (hSubWnd[SWND_DMAC] == NULL) {
		return;
	}

	/* �`�� */
	hWnd = hSubWnd[SWND_DMAC];
	hDC = GetDC(hWnd);
	DrawDMAC(hWnd, hDC);
	ReleaseDC(hWnd, hDC);
}

/*
 *	DMAC�E�C���h�E
 *	�ĕ`��
 */
static void FASTCALL PaintDMAC(HWND hWnd)
{
	HDC hDC;
	PAINTSTRUCT ps;
	RECT rect;
	BYTE *p;
	int x, y;

	ASSERT(hWnd);

	/* �|�C���^��ݒ�(���݂��Ȃ���Ή������Ȃ�) */
	p = pDMAC;
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
	DrawDMAC(hWnd, hDC);
	EndPaint(hWnd, &ps);
}

/*
 *	DMAC�E�C���h�E
 *	�E�C���h�E�v���V�[�W��
 */
static LRESULT CALLBACK DMACProc(HWND hWnd, UINT message,
								 WPARAM wParam, LPARAM lParam)
{
	/* ���b�Z�[�W�� */
	switch (message) {
		/* �E�C���h�E�ĕ`�� */
		case WM_PAINT:
			/* ���b�N���K�v */
			LockVM();
			PaintDMAC(hWnd);
			UnlockVM();
			return 0;

		/* �E�C���h�E�폜 */
		case WM_DESTROY:
			LockVM();

			/* ���C���E�C���h�E�֎����ʒm */
			DestroySubWindow(hWnd, &pDMAC, NULL);

			UnlockVM();
			break;
	}

	/* �f�t�H���g �E�C���h�E�v���V�[�W�� */
	return DefWindowProc(hWnd, message, wParam, lParam);
}

/*
 *	DMAC�E�C���h�E
 *	�E�C���h�E�쐬
 */
HWND FASTCALL CreateDMAC(HWND hParent, int index)
{
	WNDCLASSEX wcex;
	char szClassName[] = "XM7_DMAC";
	char szWndName[128];
	RECT rect;
	RECT crect, wrect;
	HWND hWnd;
	DWORD dwStyle;

	ASSERT(hParent);

	/* �E�C���h�E��`���v�Z */
	PositioningSubWindow(hParent, &rect, index);
	rect.right = lCharWidth * 30;
	rect.bottom = lCharHeight * 9;

	/* �E�C���h�E�^�C�g��������A�o�b�t�@�m�� */
	LoadString(hAppInstance, IDS_SWND_DMAC,
				szWndName, sizeof(szWndName));
	pDMAC = malloc(2 * (rect.right / lCharWidth) *
								(rect.bottom / lCharHeight));

	/* �E�C���h�E�N���X�̓o�^ */
	memset(&wcex, 0, sizeof(wcex));
	wcex.cbSize = sizeof(wcex);
	wcex.style = CS_VREDRAW | CS_HREDRAW;
	wcex.lpfnWndProc = DMACProc;
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

#endif	/* XM7_VER */

/*-[ �p���b�g���W�X�^�E�C���h�E ]--------------------------------------------*/

/*
 *	�p���b�g���W�X�^�E�C���h�E
 *	�h�b�g�`��
 */
static void FASTCALL PSetPaletteReg(int x, int y, int color)
{
	ASSERT((x >= 0) && (x < rPaletteReg.right));
	ASSERT((y >= 0) && (y < rPaletteReg.bottom));

	/* �������� */
	if (!pPaletteReg) {
		return;
	}
	*(DWORD *)(&pPaletteReg[((y * rPaletteReg.right) + x) * 4]) = color;
}

/*
 *	�p���b�g���W�X�^�E�C���h�E
 *	�{�b�N�X�t�B���`��
 */
static void FASTCALL BfPaletteReg(int x, int y, int cx, int cy, int color)
{
	int i;
	int j;

	for (i=0; i<cy; i++) {
		for (j=0; j<cx; j++) {
			PSetPaletteReg(x + j, y, color);
		}
		y++;
	}
}

/*
 *	�p���b�g���W�X�^�E�C���h�E
 *	�L�����N�^�`��
 */
static void FASTCALL ChrPaletteReg(char c, int x, int y, int fc, int bc)
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
				PSetPaletteReg(x, y, fc);
			}
			else {
				PSetPaletteReg(x, y, bc);
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
 *	�p���b�g���W�X�^�E�C���h�E
 *	�L�����N�^�`��
 */
static void FASTCALL StrPaletteReg(char *str, int x, int y, int fc, int bc)
{
	/* ����Ȃ��蔲���B */
	while (*str) {
		ChrPaletteReg(*str++, x, y, fc, bc);
		x += 8;
	}

}

/*
 *	�p���b�g���W�X�^�E�C���h�E
 *	�Z�b�g�A�b�v
 */
static BOOL FASTCALL SetupPaletteReg(void)
{
	int color, col;
	BYTE vpage;
#if XM7_VER >= 2
	int x, y;
	int r, g, b;
	int amask;
#endif

	/* �ĕ`���ԃ`�F�b�N */
	if (!bPaletteRefresh) {
		/* �ĕ`�悪�K�v�łȂ��Ȃ牽�����Ȃ��Ŗ߂� */
		return FALSE;
	}
	bPaletteRefresh = FALSE;

	/* �}���`�y�[�W */
	StrPaletteReg("ACTIVE PAGE", 0, 0 * 8, 0xFFFFFF, 0x000000);
	StrPaletteReg("---", 168, 0 * 8, 0x777777, 0x000000);
	if (!(multi_page & 0x04)) {
		ChrPaletteReg('G', 168, 0 * 8, 0x00FF00, 0x000000);
	}
	if (!(multi_page & 0x02)) {
		ChrPaletteReg('R', 176, 0 * 8, 0xFF0000, 0x000000);
	}
	if (!(multi_page & 0x01)) {
		ChrPaletteReg('B', 184, 0 * 8, 0x0000FF, 0x000000);
	}
	StrPaletteReg("DISPLAY PAGE", 0, 1 * 8, 0xFFFFFF, 0x000000);
	StrPaletteReg("---", 168, 1 * 8, 0x777777, 0x000000);
	if (!(multi_page & 0x40)) {
		ChrPaletteReg('G', 168, 1 * 8, 0x00FF00, 0x000000);
	}
	if (!(multi_page & 0x20)) {
		ChrPaletteReg('R', 176, 1 * 8, 0xFF0000, 0x000000);
	}
	if (!(multi_page & 0x10)) {
		ChrPaletteReg('B', 184, 1 * 8, 0x0000FF, 0x000000);
	}

	/* TTL�p���b�g */
	BfPaletteReg(0, 3 * 8 + 6, 192, 1, 0xFFFFFF);
	StrPaletteReg("TTL PALETTE", 0, 3 * 8 - 2, 0xFFFFFF, 0x000000);
	for (color = 0; color < 8; color ++) {
		/* �}���`�y�[�W���A�\���v���[�����𓾂� */
		vpage = (BYTE)((~(multi_page >> 4)) & 0x07);
#if XM7_VER == 1 && defined(L4CARD)
		if (enable_400line && enable_400linecard) {
			col = Palet16Tc[ttl_palet[(color & vpage)] + 8];
		}
		else {
			col = Palet16Tc[ttl_palet[(color & vpage)] & 7];
		}
#else
		col = Palet16Tc[ttl_palet[(color & vpage)] & 7];
#endif
		BfPaletteReg(color * 24, 4 * 8, 24, 16, col);
		ChrPaletteReg((BYTE)(color + 0x30), color * 24 + 16, 5 * 8,
			0xFFFFFF, 0x000000);
	}

#if XM7_VER >= 2
	/* �A�i���O�p���b�g */
	BfPaletteReg(0, 7 * 8 + 6, 192, 1, 0xFFFFFF);
	StrPaletteReg("ANALOG PALETTE", 0, 7 * 8 - 2, 0xFFFFFF, 0x000000);
	for (r = 0; r < 16; r++) {
		for (g = 0; g < 16; g++) {
			for (b = 0; b < 16; b++) {
				color = 0;

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

				/* �p���b�g�f�[�^���擾 */
				col = ((g * 0x100) + (r * 0x10) + b) & amask;

				/* R */
				color |= (WORD)apalet_r[col] * 0x11;
				color <<= 8;

				/* G */
				color |= (WORD)apalet_g[col] * 0x11;
				color <<= 8;

				/* B */
				color |= (WORD)apalet_b[col] * 0x11;

				/* �`�� */
				x = (((g & 3) << 4) + b);
				y = (((g & 12) << 2) + r);
				BfPaletteReg(x * 3, y * 2 + (8 * 8), 3, 2, color);
			}
		}
	}
#endif

	return TRUE;
}

/*
 *	�p���b�g���W�X�^�E�C���h�E
 *	�`��
 */
static void FASTCALL DrawPaletteReg(HDC hDC, BOOL flag)
{
	HDC hMemDC;
	HBITMAP hBitmap;

	ASSERT(hDC);

	/* �r�b�g�}�b�v�n���h���E�|�C���^�������Ȃ�A�������Ȃ� */
	if (!hPaletteReg || !pPaletteReg) {
		return;
	}

	/* �Z�b�g�A�b�v���`��`�F�b�N */
	if (!SetupPaletteReg() && !flag) {
		return;
	}

	/* �r�b�g�}�b�v�n���h���E�|�C���^���L����������x�`�F�b�N */
	if (!hPaletteReg || !pPaletteReg) {
		return;
	}

	/* ������DC���쐬���A�r�b�g�}�b�v���Z���N�g */
	hMemDC = CreateCompatibleDC(hDC);
	ASSERT(hMemDC);
	hBitmap = (HBITMAP)SelectObject(hMemDC, hPaletteReg);

	/* �f�X�N�g�b�v�̏󋵂�����ł́A���ۂ����蓾��l */
	if (hBitmap) {
		/* BitBlt */
		BitBlt(hDC, 0, 0,
				rPaletteReg.right, rPaletteReg.bottom, hMemDC,
				0, 0, SRCCOPY);

		/* �I�u�W�F�N�g�ăZ���N�g */
		SelectObject(hMemDC, hBitmap);
	}

	/* ������DC�폜 */
	DeleteDC(hMemDC);
}

/*
 *	�p���b�g���W�X�^�E�C���h�E
 *	���t���b�V��
 */
void FASTCALL RefreshPaletteReg(void)
{
	HWND hWnd;
	HDC hDC;

	/* ��ɌĂ΂��̂ŁA���݃`�F�b�N���邱�� */
	if (hSubWnd[SWND_PALETTE] == NULL) {
		return;
	}

	/* �`�� */
	hWnd = hSubWnd[SWND_PALETTE];
	hDC = GetDC(hWnd);
	DrawPaletteReg(hDC, TRUE);
	ReleaseDC(hWnd, hDC);
}

/*
 *	�p���b�g���W�X�^�E�C���h�E
 *	�ĕ`��
 */
static void FASTCALL PaintPaletteReg(HWND hWnd, BOOL flag)
{
	HDC hDC;
	PAINTSTRUCT ps;

	ASSERT(hWnd);

	/* �`�� */
	hDC = BeginPaint(hWnd, &ps);
	ASSERT(hDC);
	DrawPaletteReg(hDC, flag);
	EndPaint(hWnd, &ps);
}

/*
 *	�p���b�g���W�X�^�E�C���h�E
 *	�E�C���h�E�v���V�[�W��
 */
static LRESULT CALLBACK PaletteRegProc(HWND hWnd, UINT message,
								 WPARAM wParam, LPARAM lParam)
{
	/* ���b�Z�[�W�� */
	switch (message) {
		/* �E�C���h�E�ĕ`�� */
		case WM_PAINT:
			/* ���b�N���K�v */
			LockVM();
			PaintPaletteReg(hWnd, TRUE);
			UnlockVM();
			return 0;

		/* �w�i�`�� */
		case WM_ERASEBKGND:
			return TRUE;

		/* �E�C���h�E�폜 */
		case WM_DESTROY:
			LockVM();

			/* ���C���E�C���h�E�֎����ʒm */
			DestroySubWindow(hWnd, NULL, NULL);

			/* �r�b�g�}�b�v��� */
			if (hPaletteReg) {
				DeleteObject(hPaletteReg);
				hPaletteReg = NULL;
				pPaletteReg = NULL;
			}

			UnlockVM();
			break;
	}

	/* �f�t�H���g �E�C���h�E�v���V�[�W�� */
	return DefWindowProc(hWnd, message, wParam, lParam);
}

/*
 *	�p���b�g���W�X�^�E�C���h�E
 *	������
 */
void FASTCALL InitPaletteReg(HWND hWnd)
{
	BITMAPINFOHEADER *pbmi;
	HDC hDC;

	/* �S�̃��[�N������ */
	pPaletteReg = NULL;
	hPaletteReg = NULL;

	/* �r�b�g�}�b�v�w�b�_���� */
	pbmi = (BITMAPINFOHEADER*)malloc(sizeof(BITMAPINFOHEADER));
	if (pbmi) {
		memset(pbmi, 0, sizeof(BITMAPINFOHEADER));
		pbmi->biSize = sizeof(BITMAPINFOHEADER);
		pbmi->biWidth = rPaletteReg.right;
		pbmi->biHeight = -rPaletteReg.bottom;
		pbmi->biPlanes = 1;
		pbmi->biBitCount = 32;
		pbmi->biCompression = BI_RGB;

		/* DC�擾�ADIB�Z�N�V�����쐬 */
		hDC = GetDC(hWnd);
		hPaletteReg = CreateDIBSection(hDC, (BITMAPINFO*)pbmi, DIB_RGB_COLORS,
								(void**)&pPaletteReg, NULL, 0);
		ReleaseDC(hWnd, hDC);
		free(pbmi);
	}
}

/*
 *	�p���b�g���W�X�^�E�C���h�E
 *	�E�C���h�E�쐬
 */
HWND FASTCALL CreatePaletteReg(HWND hParent, int index)
{
	WNDCLASSEX wcex;
	char szClassName[] = "XM7_Palette";
	char szWndName[128];
	RECT rect;
	RECT crect, wrect;
	HWND hWnd;
	DWORD dwStyle;

	ASSERT(hParent);

	/* �t���O�Z�b�g(�E�C���h�E�쐬���ɂ͕K���ĕ`�悷��) */
	bPaletteRefresh = TRUE;

	/* �E�C���h�E��`���v�Z */
	PositioningSubWindow(hParent, &rect, index);

	/* �E�C���h�E�T�C�Y������ */
	rect.right = 192;
#if XM7_VER == 1
	rect.bottom = 6 * 8;
#else
	rect.bottom = 128 + 8 * 8;
#endif

	/* �E�C���h�E�^�C�g�������� */
	LoadString(hAppInstance, IDS_SWND_PALETTE, szWndName, sizeof(szWndName));

	/* �E�C���h�E�N���X�̓o�^ */
	memset(&wcex, 0, sizeof(wcex));
	wcex.cbSize = sizeof(wcex);
	wcex.style = CS_VREDRAW | CS_HREDRAW;
	wcex.lpfnWndProc = PaletteRegProc;;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hAppInstance;
	wcex.hIcon = LoadIcon(hAppInstance, MAKEINTRESOURCE(IDI_WNDICON));
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
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

		/* ��`��ۑ� */
		rPaletteReg = rect;

		/* ���̑������� */
		InitPaletteReg(hWnd);
	}

	/* �|�b�v�A�b�v�E�C���h�E���̓A�N�e�B�u�E�C���h�E��O�ʂɕύX */
	if (bPopupSwnd) {
		SetForegroundWindow(hMainWnd);
	}

	/* ���ʂ������A�� */
	return hWnd;
}

#endif	/* _WIN32 */
