/*
 *	FM-7 EMULATOR "XM7"
 *
 *	Copyright (C) 1999-2017 �o�h�D(Twitter:@xm6_original)
 *	Copyright (C) 2001-2017 Ryu Takegami (Twitter:@RyuTakegami)
 *
 *	[ Win32API �T�u�E�B���h�E�Q ]
 *
 *	RHG����
 *	  2001.07.25		V2���̃T�u�V�X�e���E�B���h�E�̃T�C�Y�𒲐�
 *	  2001.07.27		�T�u�V�X�e���I�[��RAM���[�h����CG�o���N������ɕ\��
 *						����Ȃ������C���E���łɏ����ύX(��)
 *	  2002.01.23		�T�uCPU�R���g���[���E�C���h�E�ɃS�~���\����������
 *						�C��(V2)
 *	  2002.02.12		FM�������W�X�^�\������THG���W�X�^�̓��e�Ƃ���WHG���W�X
 *						�^���\������Ă��������C��
 *	  2002.05.07		�_�����Z/������ԃE�B���h�E��V2.1�ȗ��̕����𐋂���(��
 *						�T�uCPU�R���g���[���E�B���h�E�ɂ��肻���łȂ����Ȃ�����
 *						�A�N�e�B�u/�\���y�[�W�AVRAM�I�t�Z�b�g��ǉ����A�S�̂̍�
 *						�ڔz�u��ύX
 *	  2002.06.15		FM-7���[�h�ł̓Ɨ�PSG�G�~�����[�V�����ɑΉ�
 *						FM�������W�X�^�E�B���h�E�EFM�����f�B�X�v���C�E�B���h�E
 *						�̃��T�C�Y�����ŋH�ɗ�����o�O���C���c�����Ǝv��(��
 *						THG�g�p���ɂ�FM�����f�B�X�v���C�E�B���h�E���E�B���h�E
 *						���ɔ[�܂�悤�ɁATHG�g�p���̂݌��ՃT�C�Y�̏c�������k
 *						������悤�ɂ���
 *	  2002.07.17		DMA�]���o�C�g���E"Update Track"�̕\�����������Ȃ�����
 *						�C��
 *	  2002.10.11		FM�����f�B�X�v���C�E�C���h�E�ŋH��FM�p�[�g�\���������
 *						���Ƃ���������C���c�����Ǝv��(��
 *	  2003.03.17		FM�����f�B�X�v���C�E�C���h�E�̉��ʕ\��(FM�p�[�g)���o��
 *						���ő�̃L�����A��TL�����ɂ���悤�ɕύX
 *	  2003.03.29		26���F/400���C�����̃A�N�e�B�uVRAM�o���N�\���ɑΉ�
 *	  2004.08.13		OPN/WHG/THG��opn.c�ւ̓����ɍ��킹���ύX�������Ȃ�
 *	  2008.01.20		FM�����f�B�X�v���C�E�C���h�E�Ƀ~���[�g�@�\��ǉ�
 *	  2012.04.20		FM�������W�X�^/�f�B�X�v���C�E�B���h�E��FM-7���[�h��PSG
 *						�P�ƕ\���ɑΉ�
 *	  2012.04.21		FM�����f�B�X�v���C�E�B���h�E�̃��x�����[�^�ɋ�؂����
 *						�ǉ�
 *	  2012.04.24		FM�����g�p����FM�����f�B�X�v���C�E�B���h�E���J���Ɨ���
 *						������C��
 *						FM�������W�X�^�E�B���h�E�̖��O���ς��Ȃ��ꍇ�������
 *						����C��
 *						FM�������W�X�^�E�B���h�E/FM�����f�B�X�v���C�E�B���h�E��
 *						�A�C�R�������Ă���ꍇ�̃E�B���h�E���ύX�ɑΉ�
 *	  2012.04.25		FM�������W�X�^�E�B���h�E��OPN/PSG�̍��ڕ\���ʒu���C��
 *	  2012.06.30		�o�u���������R���g���[���E�C���h�E�̏�����XM7dash���
 *						�ڐA
 *	  2015.03.13		�T�u�E�C���h�E�̃|�b�v�A�b�v�Ή�
 *						�|�b�v�A�b�v���[�h����THG�\�����c�ɋ��߂�̂���߂�
 */

#ifdef _WIN32

#define STRICT
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <assert.h>
#include <stdlib.h>
#include "xm7.h"
#include "fdc.h"
#include "opn.h"
#include "subctrl.h"
#if XM7_VER >= 3
#include "dmac.h"
#endif
#include "display.h"
#include "aluline.h"
#if XM7_VER == 1
#if defined(BUBBLE)
#include "bubble.h"
#endif
#endif
#include "w32.h"
#include "w32_res.h"
#include "w32_snd.h"
#include "w32_sub.h"
#include "w32_kbd.h"
#include "w32_draw.h"

/*
 *	�X�^�e�B�b�N ���[�N
 */
static BYTE *pFDC;						/* FDC Draw�o�b�t�@ */
static BYTE *pOPNReg;					/* OPN���W�X�^ Draw�o�b�t�@ */
static UINT nOPNReg;					/* OPN���W�X�^ ���x�� */
static BYTE *pOPNDisp;					/* OPN�f�B�X�v���C �r�b�g�}�b�v�o�b�t�@ */
static HBITMAP hOPNDisp;				/* OPN�f�B�X�v���C �r�b�g�}�b�v�n���h�� */
static RECT rOPNDisp;					/* OPN�f�B�X�v���C ��` */
static UINT nOPNDisp;					/* OPN�f�B�X�v���C ���x�� */
static int knOPNDisp[18];				/* OPN�f�B�X�v���C ���Ճ��[�N */
static int ktOPNDisp[18];				/* OPN�f�B�X�v���C ���Ճ��[�N */
static BYTE cnOPNDisp[18][49 * 2];		/* OPN�f�B�X�v���C �������[�N */
static BYTE ctOPNDisp[18][49 * 2];		/* OPN�f�B�X�v���C �������[�N */
static int lnOPNDisp[18];				/* OPN�f�B�X�v���C ���x�����[�N */
static int ltOPNDisp[18];				/* OPN�f�B�X�v���C ���x�����[�N */
static BYTE *pSubCtrl;					/* �T�u�R���g���[�� Draw�o�b�t�@ */
#if XM7_VER >= 2
static BYTE *pALULine;					/* �_�����Z/������� Draw�o�b�t�@ */
#endif
#if XM7_VER == 1
#if defined(BUBBLE)
static BYTE *pBMC;						/* BMC Draw�o�b�t�@ */
#endif
#endif

/*
 *	�p���b�g�e�[�u��
 */
static const RGBQUAD rgbOPNDisp[] = {
	/*  B     G     R   Reserve */
	{ 0x00, 0x00, 0x00, 0x00 },
	{ 0xff, 0x00, 0x00, 0x00 },
	{ 0x00, 0x00, 0xff, 0x00 },
	{ 0xff, 0x00, 0xff, 0x00 },
	{ 0x00, 0xff, 0x00, 0x00 },
	{ 0xff, 0xff, 0x00, 0x00 },
	{ 0x00, 0xff, 0xff, 0x00 },
	{ 0xff, 0xff, 0xff, 0x00 },

	{ 0x3f, 0x3f, 0x3f, 0x00 },	/* �ÊD */
	{ 0xff, 0xbf, 0x00, 0x00 },	/* ���F */
	{ 0x00, 0xdf, 0xff, 0x00 },	/* ���F */
	{ 0x00, 0xaf, 0x7f, 0x00 },	/* ���� */
	{ 0xbf, 0x9f, 0xff, 0x00 },	/* �Î� */
	{ 0x9f, 0xff, 0x3f, 0x00 },	/* �G�������h�O���[��(��) */
	{ 0x7f, 0xbf, 0x00, 0x00 }, /* �G�������h�O���[��(��) */
	{ 0xcf, 0xcf, 0xcf, 0x00 },	/* ���D */
};

/*-[ �T�uCPU�R���g���[���E�C���h�E ]-----------------------------------------*/

/*
 *	�T�uCPU�R���g���[���E�C���h�E
 *	�Z�b�g�A�b�v(���LRAM)
 */
static void FASTCALL SetupSubCtrlShared(BYTE *p, int cx, int y)
{
	char string[128];
	char temp[4];
	int i, j;

	ASSERT(p);
	ASSERT(cx > 0);

	/* �^�C�g�� */
	strncpy(string, "Shared RAM:", sizeof(string));
	memcpy(&p[cx * y], string, strlen(string));
	y++;

	/* ���[�v */
	for (i=0; i<8; i++) {
		/* ������쐬 */
		_snprintf(string, sizeof(string), "+%02X:", i * 16);
		for (j=0; j<16; j++) {
			_snprintf(temp, sizeof(temp), " %02X", shared_ram[i * 16 + j]);
			strncat(string, temp, sizeof(string) - strlen(string) - 1);
		}

		/* �Z�b�g */
		memcpy(&p[cx * y], string, strlen(string));
		y++;
	}
}

/*
 *	�T�uCPU�R���g���[���E�C���h�E
 *	�Z�b�g�A�b�v(�t���O)
 */
static void FASTCALL SetupSubCtrlFlag(BYTE *p, int cx, int x, int y,
									char *title, BOOL flag)
{
	char string[32];

	ASSERT(p);
	ASSERT(cx > 0);

	/* ������ */
	memset(string, 0x20, sizeof(string));

	/* �^�C�g���Z�b�g */
	memcpy(string, title, strlen(title));

	/* On�܂���Off */
	if (flag) {
		strncpy(&string[19], " On", sizeof(string)-19);
	}
	else {
		strncpy(&string[19], "Off", sizeof(string)-19);
	}

	/* �Z�b�g */
	memcpy(&p[cx * y + x], string, strlen(string));
}


#if XM7_VER == 1 && defined(L4CARD)
/*
 *	�T�uCPU�R���g���[���E�C���h�E
 *	�Z�b�g�A�b�v(CRTC���W�X�^)
 */
static void FASTCALL SetupSubCtrlCRTC(BYTE *p, int cx, int y)
{
	char string[128];
	char temp[4];
	int i, j;

	ASSERT(p);
	ASSERT(cx > 0);

	/* �^�C�g�� */
	strncpy(string, "CRTC Register:", sizeof(string));
	memcpy(&p[cx * y], string, strlen(string));
	y++;

	/* ���[�v */
	for (i=0; i<2; i++) {
		/* ������쐬 */
		_snprintf(string, sizeof(string), "+%02X:", i * 16);
		for (j=0; j<16; j++) {
			_snprintf(temp, sizeof(temp), " %02X", crtc_register[i * 16 + j]);
			if (((i * 16 + j) == crtc_regnum) && enable_400line) {
				temp[1] |= 0x80;
				temp[2] |= 0x80;
			}
			strncat(string, temp, sizeof(string) - strlen(string) - 1);
		}

		/* �Z�b�g */
		memcpy(&p[cx * y], string, strlen(string));
		y++;
	}
}
#endif

/*
 *	�T�uCPU�R���g���[���E�C���h�E
 *	�Z�b�g�A�b�v
 */
static void FASTCALL SetupSubCtrl(BYTE *p, int x, int y)
{
#if XM7_VER >= 3
#define	YOFS	7
#else
#define	YOFS	6
#endif

	char string[128];

	ASSERT(p);
	ASSERT(x > 0);
	ASSERT(y > 0);

	/* ��U�X�y�[�X�Ŗ��߂� */
	memset(p, 0x20, x * y);

	/* FM-7�݊��t���O */
	SetupSubCtrlFlag(p, x,  0, 0, "Sub Halt", subhalt_flag);
	SetupSubCtrlFlag(p, x,  0, 1, "Busy Flag", subbusy_flag);
	SetupSubCtrlFlag(p, x,  0, 2, "CRT Display", crt_flag);
	SetupSubCtrlFlag(p, x, 30, 0, "Cancel IRQ", subcancel_flag);
	SetupSubCtrlFlag(p, x, 30, 1, "Attention FIRQ", subattn_flag);
	SetupSubCtrlFlag(p, x, 30, 2, "VRAM Access Flag", vrama_flag);

#if XM7_VER >= 2
	/* �T�u���j�^ROM */
	strncpy(string, "Sub Monitor", sizeof(string));
	memcpy(&p[x * 4 + 0], string, strlen(string));
	switch (subrom_bank) {
		case 0:
			strncpy(string, "Type-C", sizeof(string));
			break;
		case 1:
			strncpy(string, "Type-A", sizeof(string));
			break;
		case 2:
			strncpy(string, "Type-B", sizeof(string));
			break;
		case 3:
			strncpy(string, "CG ROM", sizeof(string));
			break;
#if XM7_VER >= 3
		case 4:
			strncpy(string, "   RAM", sizeof(string));

			/* Type-D/E��F�� */
			if ((subramde[0x1fe0] == 'S') &&
				(subramde[0x1fe1] == 'U') &&
				(subramde[0x1fe2] == 'B') &&
				(subramde[0x1fe3] == '8')) {
				if (subramde[0x1fe4] == 'A') {
					strncpy(string,"Type-D", sizeof(string));
				}
				else if (subramde[0x1fe4] == 'B') {
					strncpy(string,"Type-E", sizeof(string));
				}
			}
			break;
#endif
		default:
			ASSERT(FALSE);
			break;
	}
	memcpy(&p[x * 4 + 16], string, strlen(string));

	/* 320���[�h */
	strncpy(string, "Display Mode", sizeof(string));
	memcpy(&p[x * 4 + 30], string, strlen(string));
#if XM7_VER >= 3
	if (mode400l) {
		strncpy(string, "640x400", sizeof(string));
	}
	else if (mode256k) {
		strncpy(string, "262,144", sizeof(string));
	}
	else if (mode320) {
#else
	if (mode320) {
#endif
		strncpy(string, "320x200", sizeof(string));
	}
	else {
		strncpy(string, "640x200", sizeof(string));
	}
	memcpy(&p[x * 4 + 45], string, strlen(string));

	/* CG�o���N */
#if XM7_VER >= 3
	if (subrom_bank == 4) {
		strncpy(string, "CG RAM", sizeof(string));
		memcpy(&p[x * 5 + 0], string, strlen(string));
		_snprintf(string, sizeof(string), "Bank %1d", cgram_bank);
	}
	else {
		strncpy(string, "CG ROM", sizeof(string));
		memcpy(&p[x * 5 + 0], string, strlen(string));
		_snprintf(string, sizeof(string), "Bank %1d", cgrom_bank);
	}
#else
	strncpy(string, "CG ROM", sizeof(string));
	memcpy(&p[x * 5 + 0], string, strlen(string));
	_snprintf(string, sizeof(string), "Bank %1d", cgrom_bank);
#endif
	memcpy(&p[x * 5 + 16], string, strlen(string));

	/* �T�u���Z�b�g */
	strncpy(string, "Sub Reset", sizeof(string));
	memcpy(&p[x * 5 + 30], string, strlen(string));
	if (subreset_flag) {
		strncpy(string, "Software", sizeof(string));
	}
	else {
		strncpy(string, "Hardware", sizeof(string));
	}
	memcpy(&p[x * 5 + 44], string, strlen(string));

	/* VRAM�I�t�Z�b�g���W�X�^0 */
	strncpy(string, "VRAM Offset 0", sizeof(string));
	memcpy(&p[x * (YOFS + 0) + 30], string, strlen(string));
	_snprintf(string, 128, "$%04X", vram_offset[0]);
	memcpy(&p[x * (YOFS + 0) + 47], string, strlen(string));

	/* VRAM�I�t�Z�b�g���W�X�^1 */
	strncpy(string, "VRAM Offset 1", sizeof(string));
	memcpy(&p[x * (YOFS + 1) + 30], string, strlen(string));
	_snprintf(string, sizeof(string), "$%04X", vram_offset[1]);
	memcpy(&p[x * (YOFS + 1) + 47], string, strlen(string));

	/* �A�N�e�B�u�y�[�W */
	strncpy(string, "Active Page", sizeof(string));
	memcpy(&p[x * (YOFS + 0) + 0], string, strlen(string));
#if XM7_VER >= 3
	if (screen_mode & SCR_AV40) {
		_snprintf(string, sizeof(string), "Page %1d", subram_vrambank);
	}
	else {
		_snprintf(string, sizeof(string), "Page %1d", vram_active);
	}
#else
	_snprintf(string, sizeof(string), "Page %1d", vram_active);
#endif
	memcpy(&p[x * (YOFS + 0) + 16], string, strlen(string));

	/* �\���y�[�W */
	strncpy(string, "Display Page", sizeof(string));
	memcpy(&p[x * (YOFS + 1) + 0], string, strlen(string));
	_snprintf(string, sizeof(string), "Page %1d", vram_display);
	memcpy(&p[x * (YOFS + 1) + 16], string, strlen(string));

#if XM7_VER >= 3
	/* �R���\�[��RAM�o���N */
	strncpy(string, "Console RAM", sizeof(string));
	memcpy(&p[x * 6 + 0], string, strlen(string));
	if (subrom_bank == 4) {
		_snprintf(string, sizeof(string), "Bank %1d", consram_bank);
	}
	else {
		_snprintf(string, sizeof(string), "Bank 0");
	}
	memcpy(&p[x * 6 + 16], string, strlen(string));

	/* �T�u���j�^RAM�v���e�N�g */
	strncpy(string, "Sub Protect", sizeof(string));
	memcpy(&p[x * 6 + 30], string, strlen(string));
	if (subram_protect) {
		strncpy(string, " Enable", sizeof(string));
	}
	else {
		strncpy(string, "Disable", sizeof(string));
	}
	memcpy(&p[x * 6 + 45], string, strlen(string));

	/* �A�N�e�B�u�u���b�N */
	strncpy(string, "Active Block", sizeof(string));
	memcpy(&p[x * 9 + 0], string, strlen(string));
	_snprintf(string, sizeof(string), "Block %1d", block_active);
	memcpy(&p[x * 9 + 15], string, strlen(string));

	/* �\���u���b�N */
	strncpy(string, "Display Block", sizeof(string));
	memcpy(&p[x * 10 + 0], string, strlen(string));
	_snprintf(string, sizeof(string), "Block %1d", block_display);
	memcpy(&p[x * 10 + 15], string, strlen(string));

	/* �n�[�h�E�F�A�E�B���h�E */
	strncpy(string, "Window Start", sizeof(string));
	memcpy(&p[x * 9 + 30], string, strlen(string));
	strncpy(string, "Window End", sizeof(string));
	memcpy(&p[x * 10 + 30], string, strlen(string));
	if (window_open) {
		_snprintf(string, sizeof(string), "(%3d,%3d)", window_dx1, window_dy1);
		memcpy(&p[x * 9 + 43], string, strlen(string));
		_snprintf(string, sizeof(string), "(%3d,%3d)",
			window_dx2 - 1, window_dy2 - 1);
	}
	else {
		strncpy(string, "(  0,  0)", sizeof(string));
		memcpy(&p[x * 9 + 43], string, strlen(string));
	}
	memcpy(&p[x * 10 + 43], string, strlen(string));

	/* ���LRAM */
	SetupSubCtrlShared(p, x, 12);
#else
	/* ���LRAM */
	SetupSubCtrlShared(p, x, 9);
#endif
#elif defined(L4CARD)
	/* ��ʃ��[�h */
	strncpy(string, "Display Mode", sizeof(string));
	memcpy(&p[x * 4 + 0], string, strlen(string));
	if (enable_400line) {
		strncpy(string, "640x400", sizeof(string));
	}
	else {
		strncpy(string, "640x200", sizeof(string));
	}
	memcpy(&p[x * 4 + 15], string, strlen(string));

	/* VRAM�I�t�Z�b�g���W�X�^ */
	strncpy(string, "VRAM Offset", sizeof(string));
	memcpy(&p[x * 4 + 30], string, strlen(string));
	_snprintf(string, sizeof(string), "$%04X", vram_offset[0]);
	memcpy(&p[x * 4 + 47], string, strlen(string));

	/* �e�L�X�g�X�^�[�g�A�h���X */
	strncpy(string, "Text Start Addr", sizeof(string));
	memcpy(&p[x * 5 + 0], string, strlen(string));
	_snprintf(string, sizeof(string), "$%04X", text_start_addr);
	memcpy(&p[x * 5 + 17], string, strlen(string));

	/* �J�[�\���A�h���X���W�X�^ */
	strncpy(string, "Cursor Address", sizeof(string));
	memcpy(&p[x * 5 + 30], string, strlen(string));
	_snprintf(string, sizeof(string), "$%04X", cursor_addr);
	memcpy(&p[x * 5 + 47], string, strlen(string));

	/* CRTC���W�X�^ */
	SetupSubCtrlCRTC(p, x, 7);

	/* ���LRAM(V1,L4) */
	SetupSubCtrlShared(p, x, 11);
#else
	/* ���LRAM(V1,L2) */
	SetupSubCtrlShared(p, x, 4);
#endif
#undef	YOFS
}


/*
 *	�T�uCPU�R���g���[���E�C���h�E
 *	�`��
 */
static void FASTCALL DrawSubCtrl(HWND hWnd, HDC hDC)
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
	if (!pSubCtrl) {
		return;
	}
	SetupSubCtrl(pSubCtrl, x, y);

	/* �`�� */
#if XM7_VER == 1 && defined(L4CARD)
	DrawWindowText2(hDC, pSubCtrl, x, y);
#else
	DrawWindowText(hDC, pSubCtrl, x, y);
#endif
}

/*
 *	�T�uCPU�R���g���[���E�C���h�E
 *	���t���b�V��
 */
void FASTCALL RefreshSubCtrl(void)
{
	HWND hWnd;
	HDC hDC;

	/* ��ɌĂ΂��̂ŁA���݃`�F�b�N���邱�� */
	if (hSubWnd[SWND_SUBCTRL] == NULL) {
		return;
	}

	/* �`�� */
	hWnd = hSubWnd[SWND_SUBCTRL];
	hDC = GetDC(hWnd);
	DrawSubCtrl(hWnd, hDC);
	ReleaseDC(hWnd, hDC);
}

/*
 *	�T�uCPU�R���g���[���E�C���h�E
 *	�ĕ`��
 */
static void FASTCALL PaintSubCtrl(HWND hWnd)
{
	HDC hDC;
	PAINTSTRUCT ps;
	RECT rect;
	BYTE *p;
	int x, y;

	ASSERT(hWnd);

	/* �|�C���^��ݒ�(���݂��Ȃ���Ή������Ȃ�) */
	p = pSubCtrl;
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
	DrawSubCtrl(hWnd, hDC);
	EndPaint(hWnd, &ps);
}

/*
 *	�T�uCPU�R���g���[���E�C���h�E
 *	�E�C���h�E�v���V�[�W��
 */
static LRESULT CALLBACK SubCtrlProc(HWND hWnd, UINT message,
								 WPARAM wParam, LPARAM lParam)
{
	/* ���b�Z�[�W�� */
	switch (message) {
		/* �E�C���h�E�ĕ`�� */
		case WM_PAINT:
			/* ���b�N���K�v */
			LockVM();
			PaintSubCtrl(hWnd);
			UnlockVM();
			return 0;

		/* �E�C���h�E�폜 */
		case WM_DESTROY:
			LockVM();

			/* ���C���E�C���h�E�֎����ʒm */
			DestroySubWindow(hWnd, &pSubCtrl, NULL);

			UnlockVM();
			break;
	}

	/* �f�t�H���g �E�C���h�E�v���V�[�W�� */
	return DefWindowProc(hWnd, message, wParam, lParam);
}

/*
 *	�T�uCPU�R���g���[���E�C���h�E
 *	�E�C���h�E�쐬
 */
HWND FASTCALL CreateSubCtrl(HWND hParent, int index)
{
	WNDCLASSEX wcex;
	char szClassName[] = "XM7_SubCtrl";
	char szWndName[128];
	RECT rect;
	RECT crect, wrect;
	HWND hWnd;
	DWORD dwStyle;

	ASSERT(hParent);

	/* �E�C���h�E��`���v�Z */
	PositioningSubWindow(hParent, &rect, index);
	rect.right = lCharWidth * 52;
#if XM7_VER >= 3
	rect.bottom = lCharHeight * 21;
#elif XM7_VER >= 2
	rect.bottom = lCharHeight * 18;
#elif defined(L4CARD)
	rect.bottom = lCharHeight * 20;
#else
	rect.bottom = lCharHeight * 13;
#endif

	/* �E�C���h�E�^�C�g��������A�o�b�t�@�m�� */
	LoadString(hAppInstance, IDS_SWND_SUBCTRL,
				szWndName, sizeof(szWndName));
	pSubCtrl = malloc(2 * (rect.right / lCharWidth) *
								(rect.bottom / lCharHeight));

	/* �E�C���h�E�N���X�̓o�^ */
	memset(&wcex, 0, sizeof(wcex));
	wcex.cbSize = sizeof(wcex);
	wcex.style = CS_VREDRAW | CS_HREDRAW;
	wcex.lpfnWndProc = SubCtrlProc;
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

/*-[ �_�����Z/������ԃE�C���h�E ]-------------------------------------------*/

#if XM7_VER >= 2
/*
 *	�_�����Z/������ԃE�C���h�E
 *	�Z�b�g�A�b�v
 */
static void FASTCALL SetupALULine(BYTE *p, int x, int y)
{
	static const char *ALUString[8] = {
		"PSET", "----", " OR ", " AND", " XOR", " NOT", "TILE", " CMP"
	};

	char string[128];
	int i;

	ASSERT(p);
	ASSERT(x > 0);
	ASSERT(y > 0);

	/* ��U�X�y�[�X�Ŗ��߂� */
	memset(p, 0x20, x * y);

	/* �_�����Z�R�}���h */
	strncpy(string, "ALU", sizeof(string));
	memcpy(&p[x * 0 + 0], string, strlen(string));
	if (alu_command & 0x80) {
		strncpy(string, ALUString[alu_command & 0x07], sizeof(string));
	}
	else {
		strncpy(string, " Off", sizeof(string));
	}
	memcpy(&p[x * 0 + 17], string, strlen(string));

	/* �������݃��[�h */
	strncpy(string, "Write Mode", sizeof(string));
	memcpy(&p[x * 1 + 0], string, strlen(string));
	if (alu_command & 0x40) {
		if (alu_command & 0x20) {
			strncpy(string, "Not Eq", sizeof(string));
		}
		else {
			strncpy(string, " Equal", sizeof(string));
		}
	}
	else {
		strncpy(string, "Always", sizeof(string));
	}
	memcpy(&p[x * 1 + 15], string, strlen(string));

	/* ���Z�J���[�f�[�^ */
	_snprintf(string, sizeof(string), "Color               %01d",
		alu_color & 0x07);
	memcpy(&p[x * 2 + 0], string, strlen(string));

	/* �}�X�N�r�b�g */
	_snprintf(string, sizeof(string), "Mask               %02X", alu_mask);
	memcpy(&p[x * 3 + 0], string, strlen(string));

	/* �R���y�A�r�b�g */
	strncpy(string, "Compare      --------", sizeof(string));
	for (i=0; i<8; i++) {
		if (~alu_cmpdat[i] & 0x80) {
			string[13 + i] = (char)(0x30 | (alu_cmpdat[i] & 0x07));
		}
	}
	memcpy(&p[x * 4 + 0], string, strlen(string));

	/* �f�B�Z�[�u���o���N */
	strncpy(string, "Disable           BRG", sizeof(string));
	for (i=0; i<3; i++) {
		if (~alu_disable & (1 << i)) {
			string[18 + i] = '-';
		}
	}
	memcpy(&p[x * 5 + 0], string, strlen(string));

	/* �^�C���y�C���g�f�[�^ */
	_snprintf(string, sizeof(string), "TILE   B:%02X R:%02X G:%02X",
		alu_tiledat[0], alu_tiledat[1], alu_tiledat[2]);
	memcpy(&p[x * 6 + 0], string, strlen(string));

	/* ������ԃX�e�[�^�X */
	strncpy(string, "Line LSI", sizeof(string));
	memcpy(&p[x * 0 + 25], string, strlen(string));
	if (line_busy) {
		strncpy(string, " Busy", sizeof(string));
	}
	else {
		strncpy(string, "Ready", sizeof(string));
	}
	memcpy(&p[x * 0 + 41], string, strlen(string));

	/* �A�h���X�I�t�Z�b�g */
	_snprintf(string, sizeof(string), "Offset           %04X", line_offset);
	memcpy(&p[x * 1 + 25], string, strlen(string));

	/* ���C���X�^�C�� */
	_snprintf(string, sizeof(string), "Line Style       %04X", line_style);
	memcpy(&p[x * 2 + 25], string, strlen(string));

	/* �n�_�E�I�_ */
	_snprintf(string, sizeof(string), "Line X0           %3d", line_x0);
	memcpy(&p[x * 3 + 25], string, strlen(string));
	_snprintf(string, sizeof(string), "Line Y0           %3d", line_y0);
	memcpy(&p[x * 4 + 25], string, strlen(string));
	_snprintf(string, sizeof(string), "Line X1           %3d", line_x1);
	memcpy(&p[x * 5 + 25], string, strlen(string));
	_snprintf(string, sizeof(string), "Line Y1           %3d", line_y1);
	memcpy(&p[x * 6 + 25], string, strlen(string));
}

/*
 *	�_�����Z/������ԃE�C���h�E
 *	�`��
 */
static void FASTCALL DrawALULine(HWND hWnd, HDC hDC)
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
	if (!pALULine) {
		return;
	}
	SetupALULine(pALULine, x, y);

	/* �`�� */
	DrawWindowText(hDC, pALULine, x, y);
}

/*
 *	�_�����Z/������ԃE�C���h�E
 *	���t���b�V��
 */
void FASTCALL RefreshALULine(void)
{
	HWND hWnd;
	HDC hDC;

	/* ��ɌĂ΂��̂ŁA���݃`�F�b�N���邱�� */
	if (hSubWnd[SWND_ALULINE] == NULL) {
		return;
	}

	/* �`�� */
	hWnd = hSubWnd[SWND_ALULINE];
	hDC = GetDC(hWnd);
	DrawALULine(hWnd, hDC);
	ReleaseDC(hWnd, hDC);
}

/*
 *	�_�����Z/������ԃE�C���h�E
 *	�ĕ`��
 */
static void FASTCALL PaintALULine(HWND hWnd)
{
	HDC hDC;
	PAINTSTRUCT ps;
	RECT rect;
	BYTE *p;
	int x, y;

	ASSERT(hWnd);

	/* �|�C���^��ݒ�(���݂��Ȃ���Ή������Ȃ�) */
	p = pALULine;
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
	DrawALULine(hWnd, hDC);
	EndPaint(hWnd, &ps);
}

/*
 *	�_�����Z/������ԃE�C���h�E
 *	�E�C���h�E�v���V�[�W��
 */
static LRESULT CALLBACK ALULineProc(HWND hWnd, UINT message,
								 WPARAM wParam, LPARAM lParam)
{
	/* ���b�Z�[�W�� */
	switch (message) {
		/* �E�C���h�E�ĕ`�� */
		case WM_PAINT:
			/* ���b�N���K�v */
			LockVM();
			PaintALULine(hWnd);
			UnlockVM();
			return 0;

		/* �E�C���h�E�폜 */
		case WM_DESTROY:
			LockVM();

			/* ���C���E�C���h�E�֎����ʒm */
			DestroySubWindow(hWnd, &pALULine, NULL);

			UnlockVM();
			break;
	}

	/* �f�t�H���g �E�C���h�E�v���V�[�W�� */
	return DefWindowProc(hWnd, message, wParam, lParam);
}

/*
 *	�_�����Z/������ԃE�C���h�E
 *	�E�C���h�E�쐬
 */
HWND FASTCALL CreateALULine(HWND hParent, int index)
{
	WNDCLASSEX wcex;
	char szClassName[] = "XM7_ALULine";
	char szWndName[128];
	RECT rect;
	RECT crect, wrect;
	HWND hWnd;
	DWORD dwStyle;

	ASSERT(hParent);

	/* �E�C���h�E��`���v�Z */
	PositioningSubWindow(hParent, &rect, index);
	rect.right = lCharWidth * 46;
	rect.bottom = lCharHeight * 7;

	/* �E�C���h�E�^�C�g��������A�o�b�t�@�m�� */
	LoadString(hAppInstance, IDS_SWND_ALULINE,
				szWndName, sizeof(szWndName));
	pALULine = malloc(2 * (rect.right / lCharWidth) *
								(rect.bottom / lCharHeight));

	/* �E�C���h�E�N���X�̓o�^ */
	memset(&wcex, 0, sizeof(wcex));
	wcex.cbSize = sizeof(wcex);
	wcex.style = CS_VREDRAW | CS_HREDRAW;
	wcex.lpfnWndProc = ALULineProc;
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
#endif

/*-[ OPN�f�B�X�v���C�E�C���h�E ]---------------------------------------------*/

/*
 *	OPN�f�B�X�v���C�E�C���h�E
 *	�h�b�g�`��
 */
static void FASTCALL PSetOPNDisp(int x, int y, int color)
{
	int w;
	BYTE *p;
	BYTE dat;

	ASSERT((x >= 0) && (x < rOPNDisp.right));
	ASSERT((y >= 0) && (y < rOPNDisp.bottom));
	ASSERT((color >= 0) && (color <= 15));

	/* rOPNDisp���A�r�b�g�}�b�v�̉��T�C�Y���v�Z(4�o�C�g�A���C�����g) */
	w = (((rOPNDisp.right / 2) + 3) >> 2) << 2;

	/* �A�h���X�A�f�[�^�擾 */
	if (!pOPNDisp) {
		return;
	}
	p = &pOPNDisp[w * y + (x >> 1)];
	dat = *p;

	/* �Q�ɕ����� */
	if (x & 1) {
		dat &= 0xf0;
		dat |= (BYTE)color;
	}
	else {
		dat &= 0x0f;
		dat |= (BYTE)(color << 4);
	}

	/* �������� */
	*p = dat;
}

/*
 *	OPN�f�B�X�v���C�E�C���h�E
 *	�{�b�N�X�t�B���`��
 */
static void FASTCALL BfOPNDisp(int x, int y, int cx, int cy, int color)
{
	int i;
	int j;

	ASSERT((color >= 0) && (color < 16));

	for (i=0; i<cy; i++) {
		for (j=0; j<cx; j++) {
			PSetOPNDisp(x + j, y, color);
		}
		y++;
	}
}

/*
 *	OPN�f�B�X�v���C�E�C���h�E
 *	���x�����[�^�`��
 */
static void FASTCALL LvlOPNDisp(int x, int y, int cx, int cy,
								int color1, int color2, int color3)
{
	int i;
	int j;

	ASSERT((color1 >= 0) && (color1 < 16));
	ASSERT((color2 >= 0) && (color2 < 16));
	ASSERT((color3 >= 0) && (color3 < 16));

	for (i=0; i<cy; i++) {
		for (j=0; j<cx; j++) {
			if ((j % 4) == 3) {
				PSetOPNDisp(x + j, y, 8);
			}
			else {
				if (j < 48) {
					PSetOPNDisp(x + j, y, color1);
				}
				else if (j < 80) {
					PSetOPNDisp(x + j, y, color2);
				}
				else {
					PSetOPNDisp(x + j, y, color3);
				}
			}
		}
		y++;
	}
}

/*
 *	OPN�f�B�X�v���C�E�C���h�E
 *	�L�����N�^�`��
 */
static void FASTCALL ChrOPNDisp(char c, int x, int y, int color)
{
	int i;
	int j;
	BYTE *p;
	BYTE dat;

	ASSERT((color >= 0) && (color < 16));

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
				PSetOPNDisp(x, y, color);
			}
			else {
				PSetOPNDisp(x, y, 0);
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
 *	OPN�f�B�X�v���C�E�C���h�E
 *	���Օ`��
 *
 *	��ʃj�u��	�I�N�^�[�u 0�`7
 *	���ʃj�u��	C, C#, D, D#, E, F, F#, G, G#, A, A#, B
 *	x, y�̓g���b�N��̊�_�Bcolor=-1�̓f�t�H���g�J���[
 */
static void FASTCALL KbdOPNDisp(int code, int x, int y, int color)
{
	int i;
	int j;
	int ys;

	/* x���W�B��=5dot���A��=4dot���B���Ɣ��̌��Ԃ�1dot�}�� */
	static const int x_table[] = {
		0, 0+4, 6, 6+4, 12, 18, 18+4, 24, 24+4, 30, 30+4, 36
	};
	/* 0:�E���� 1:���E���� 2:������ 3:���� */
	static const int type_table[] = {
		0, 3, 1 ,3, 2, 0, 3, 1, 3, 1, 3, 2
	};

	ASSERT(code <= 0x7f);
	ASSERT((code & 0x0f) <= 0x0b);

	/* �c�T�C�Y���� */
	if (((nOPNDisp & 7) == 2) && !bPopupSwnd) {
		ys = 6;
	}
	else {
		ys = 8;
	}

	/* x�ʒu���� */
	x = (code >> 4) * 42;
	code &= 0x0f;
	x += x_table[code];

	/* �F�ݒ� */
	if (color < 0) {
		if (type_table[code] < 3) {
			color = 15;
		}
		else {
			color = 8;
		}
	}

	/* �^�C�v�� */
	switch (type_table[code]) {
		/* �����B�E���ɍ������� */
		case 0:
			/* ���Ԃ������� */
			for (i=0; i<ys * 2; i++) {
				PSetOPNDisp(x, y + i, 0);
			}
			/* ������� */
			for (i=0; i<ys; i++) {
				for (j=1; j<=3; j++) {
					PSetOPNDisp(x + j, y + i, color);
				}
			}
			/* ������� */
			for (i=0; i<ys; i++) {
				for (j=1; j<=5; j++) {
					PSetOPNDisp(x + j, y + ys + i, color);
				}
			}
			break;

		/* �����B���E�ɍ������� */
		case 1:
			/* ���Ԃ������� */
			for (i=0; i<ys; i++) {
				PSetOPNDisp(x, y + ys + i, 0);
			}
			/* ������� */
			for (i=0; i<ys; i++) {
				for (j=3; j<=3; j++) {
					PSetOPNDisp(x + j, y + i, color);
				}
			}
			/* ������� */
			for (i=0; i<ys; i++) {
				for (j=1; j<=5; j++) {
					PSetOPNDisp(x + j, y + ys + i, color);
				}
			}
			break;

		/* �����B�����ɍ������� */
		case 2:
			/* ���Ԃ������� */
			for (i=0; i<ys; i++) {
				PSetOPNDisp(x, y + ys + i, 0);
			}
			/* ������� */
			for (i=0; i<ys; i++) {
				for (j=3; j<=5; j++) {
					PSetOPNDisp(x + j, y + i, color);
				}
			}
			/* ������� */
			for (i=0; i<ys; i++) {
				for (j=1; j<=5; j++) {
					PSetOPNDisp(x + j, y + ys + i, color);
				}
			}
			break;

		/* ���� */
		case 3:
			for (i=0; i<ys; i++) {
				for (j=0; j<5; j++) {
					PSetOPNDisp(x + j, y + i, color);
				}
			}
			break;

		default:
			ASSERT(FALSE);
	}
}

/*
 *	OPN�f�B�X�v���C�E�C���h�E
 *	���ՑS�`��
 *
 *	x, y�̓g���b�N��̊�_
 */
void FASTCALL AllKbdOPNDisp(int x, int y)
{
	int i;
	int j;

	/* �Q�d���[�v */
	for (i=0; i<8; i++) {
		for (j=0; j<12; j++) {
			KbdOPNDisp((i * 16) + j, x, y, -1);
		}
	}
}

/*
 *	OPN�f�B�X�v���C�E�C���h�E
 *	�Z�b�g�A�b�v
 */
static BOOL FASTCALL SetupOPNDisp(int *first, int *end)
{
	int i;
	int j;
	int y;
	int n;
	int ys;
	BOOL flag[18];

	/* ������ */
	memset(flag, 0, sizeof(flag));

	/* �i���o�[���� */
	if (nOPNDisp & 16) {
		n = 3;
	}
	else {
		n = (((nOPNDisp & 7) + 1) * 6);
		if (nOPNDisp & 8) {
			/* THG���g�p���̂�PSG��ǉ� */
			n += 3;
		}
	}
	if (((nOPNDisp & 7) == 2) && !bPopupSwnd) {
		ys = (8 + 12 + 1);
	}
	else {
		ys = (8 * 3 + 1);
	}

	/* ���� */
	y = 0;
	for (i=0; i<n; i++) {
		for (j=0; j<42; j++) {
			/* ��v�`�F�b�N */
			if (cnOPNDisp[i][j * 2] == ctOPNDisp[i][j * 2]) {
				if (cnOPNDisp[i][j*2+1] == ctOPNDisp[i][j*2+1]) {
					continue;
				}
			}

			/* �`�� */
			ChrOPNDisp(ctOPNDisp[i][j * 2], j * 8, y, ctOPNDisp[i][j * 2 + 1]);

			/* �R�s�[ */
			cnOPNDisp[i][j * 2] = ctOPNDisp[i][j * 2];
			cnOPNDisp[i][j * 2 + 1] = ctOPNDisp[i][j * 2 + 1];

			/* �t���O���� */
			flag[i] = TRUE;
		}

		/* ���� */
		y += ys;
	}

	/* ���� */
	y = 8;
	for (i=0; i<n; i++) {
		/* �����ԂȂ�A�L�[�I�t��Ԃɂ���̂��挈 */
		if (knOPNDisp[i] == -2) {
			AllKbdOPNDisp(0, y);
			knOPNDisp[i] = -1;
			flag[i] = TRUE;
		}

		/* �`�F�b�N */
		if (knOPNDisp[i] != ktOPNDisp[i]) {
			/* �L�[�I�t��ԂȂ�A���� */
			if (knOPNDisp[i] >= 0) {
				KbdOPNDisp(knOPNDisp[i], 0, y, -1);
			}
			/* �L�[�I���ɂ���Ȃ�A���� */
			if (ktOPNDisp[i] >= 0) {
				if (!(nOPNDisp & 16)) {
					if ((nOPNDisp & 8) &&
						(i >= (int)((nOPNDisp & 7) + 1) * 6)) {
						if (!GetMute((i % 3) + 15)) {
							KbdOPNDisp(ktOPNDisp[i], 0, y, 13);
						}
					}
					else if ((i % 6) < 3) {
						if (!GetMute(i)) {
							KbdOPNDisp(ktOPNDisp[i], 0, y, 10);
						}
					}
					else {
						if (!GetMute(i)) {
							KbdOPNDisp(ktOPNDisp[i], 0, y, 9);
						}
					}
				}
				else {
					if (!GetMute((i % 3) + 15)) {
						KbdOPNDisp(ktOPNDisp[i], 0, y, 13);
					}
				}
			}

			/* �X�V */
			knOPNDisp[i] = ktOPNDisp[i];
			flag[i] = TRUE;
		}

		/* ���� */
		y += ys;
	}

	/* ���x�� */
	y = 0;
	for (i=0; i<n; i++) {
		/* ��v�`�F�b�N */
		if (lnOPNDisp[i] != ltOPNDisp[i]) {
			/* �؂�ւ��_�ƂȂ�x���W�����߂� */
			if (ltOPNDisp[i] >= 445) {
				ltOPNDisp[i] = 444;
			}
			else if (ltOPNDisp[i] < 0) {
				ltOPNDisp[i] = 0;
			}
			j = 100 * ltOPNDisp[i];
			j /= 445;

			/* �`�� */
			BfOPNDisp(236, y, 100, 7, 8);
			LvlOPNDisp(236, y, j, 7, 14, 13, 12);

			/* �X�V */
			lnOPNDisp[i] = ltOPNDisp[i];
			flag[i] = TRUE;
		}

		/* ���� */
		y += ys;
	}

	/* flag���������� */
	y = -1;
	j = n;
	for (i=0; i<n; i++) {
		if (flag[i]) {
			/* ���������� */
			if (i < j) {
				j = i;
			}
			/* ��������� */
			if (y < i) {
				y = i;
			}
		}
	}

	/* j����y�܂ŁA��������΂悢 */
	if (y >= 0) {
		*first = j;
		*end = y;
		return TRUE;
	}

	/* �`��̕K�v�Ȃ� */
	return FALSE;
}

/*
 *	�����e�[�u��
 *	�������Ƃ̂������肵�������łȂ��A���̒��Ԃ̂������l��\��������
 */
static const double pitch_table[] = {
	31.772, 33.661, 35.663, 37.784, 40.030, 42.411, 44.933, 47.605, 50.435, 53.434, 56.612, 59.978,
	63.544, 67.323, 71.326, 75.567, 80.061, 84.822, 89.865, 95.209, 100.870, 106.869, 113.223, 119.956,
	127.089, 134.646, 142.652, 151.135, 160.122, 169.643, 179.731, 190.418, 201.741, 213.737, 226.446, 239.912,
	254.178, 269.292, 285.305, 302.270, 320.244, 339.287, 359.461, 380.836, 403.482, 427.474, 452.893, 479.823,
	508.356, 538.584, 570.610, 604.540, 640.488, 678.573, 718.923, 761.672, 806.964, 854.948, 905.786, 959.647,
	1016.711, 1077.168, 1141.220, 1209.080, 1280.975, 1357.146, 1437.846, 1523.345, 1613.927, 1709.896, 1811.572, 1919.293,
	2033.422, 2154.336, 2282.439, 2418.160, 2561.951, 2714.292, 2875.692, 3046.689, 3227.855, 3419.792, 3623.144, 3838.587,
	4066.845, 4308.672, 4564.878, 4836.319, 5123.901, 5428.584, 5751.384, 6093.378, 6455.709, 6839.585, 7246.287, 7677.173,
	8133.681
};

/*
 *	�������R�[�h�ϊ�
 *	-1�͔͈͊O������
 */
static int FASTCALL ConvOPNDisp(double freq)
{
	int i;
	int ret;

	/* �͈͊O���`�F�b�N */
	if (freq < pitch_table[0]) {
		return -1;
	}
	if (pitch_table[96] <= freq) {
		return -1;
	}

	/* ���[�v�A���� */
	ret = -1;
	for (i=0; i<96; i++) {
		if ((pitch_table[i] <= freq) && (freq <pitch_table[i + 1])) {
			/* i��ϊ� */
			ret = i / 12;
			ret <<= 4;
			ret += (i % 12);
			break;
		}
	}

	return ret;
}

/*
 *	OPN�f�B�X�v���C�E�C���h�E
 *	FM�������R�[�h�ϊ�
 */
static int FASTCALL FMConvOPNDisp(BYTE a4, BYTE a0, int scale)
{
	int oct;
	int fnum;
	double freq;
	double d;

	/* Octave, F-Number�����߂� */
	oct = (int)a4;
	oct >>= 3;
	fnum = (int)a4;
	fnum &= 0x07;
	fnum <<= 8;
	fnum |= (int)a0;

	/* ���g�������߂� */
	freq = OPN_CLOCK * 100;
	while (oct != 0) {
		freq *= 2;
		oct--;
	}
	freq *= fnum;
	d = (double)(1 << 20);
	d *= 12;
	d *= scale;
	freq /= d;

	/* �ϊ� */
	return ConvOPNDisp(freq);
}

/*
 *	OPN�f�B�X�v���C�E�C���h�E
 *	PSG�������R�[�h�ϊ�
 */
static int FASTCALL PSGConvOPNDisp(BYTE low, BYTE high, int scale)
{
	int pitch;
	double freq;
	double d;

	/* �s�b�`�Z�o */
	pitch = (int)high;
	pitch &= 0x0f;
	pitch <<= 8;
	pitch |= (int)low;

	/* ���g�������߂� */
	freq = OPN_CLOCK * 100;
	d = 8;
	d *= pitch;
	switch (scale) {
		case 3:
			d *= 2;
			break;
		case 6:
			d *= 4;
			break;
		case 2:
			d *= 1;
			break;
	}
	if (d == 0) {
		freq = 0;
	}
	else {
		freq /= d;
	}

	/* �ϊ� */
	return ConvOPNDisp(freq);
}

/*
 *	OPN�f�B�X�v���C�E�C���h�E
 *	�L�[�{�[�h�X�e�[�^�X
 */
static void FASTCALL StatKbdOPNDisp(void)
{
	int i;
	int tmp;

	/* THG PSG�I�t�Z�b�g���� */
	if (nOPNDisp == 8) {
		tmp = 6;
	}
	else if (nOPNDisp == 9) {
		tmp = 12;
	}
	else if (nOPNDisp == 2) {
		tmp = 15;
	}
	else {
		tmp = 0;
	}

	/* FM���� */
	for (i=0; i<3; i++) {
		if (nOPNDisp & 16) {
			break;
		}

		if (!opn_key[OPN_STD][i]) {
			ktOPNDisp[i + 0] = -1;
		}
		else {
			ktOPNDisp[i + 0] = FMConvOPNDisp(opn_reg[OPN_STD][0xa4 + i],
									opn_reg[OPN_STD][0xa0 + i],
									opn_scale[OPN_STD]);
		}

		if ((nOPNDisp & 7) >= 1) {
			if (!opn_key[OPN_WHG][i]) {
				ktOPNDisp[i + 6] = -1;
			}
			else {
				ktOPNDisp[i + 6] = FMConvOPNDisp(opn_reg[OPN_WHG][0xa4 + i],
										opn_reg[OPN_WHG][0xa0 + i],
										opn_scale[OPN_WHG]);
			}
		}

		if ((nOPNDisp & 7) >= 2) {
			if (!opn_key[OPN_THG][i]) {
				ktOPNDisp[i + 12] = -1;
			}
			else {
				ktOPNDisp[i + 12] = FMConvOPNDisp(opn_reg[OPN_THG][0xa4 + i],
										opn_reg[OPN_THG][0xa0 + i],
										opn_scale[OPN_THG]);
			}
		}
	}

	/* PSG���� */
	for (i=0; i<3; i++) {
		if (!(nOPNDisp & 16)) {
			if (ltOPNDisp[i + 3] > 0) {
				ktOPNDisp[i + 3] = PSGConvOPNDisp(
										opn_reg[OPN_STD][i * 2 + 0],
										opn_reg[OPN_STD][i * 2 + 1],
										opn_scale[OPN_STD]);
			}
			else {
				ktOPNDisp[i + 3] = -1;
			}

			if ((nOPNDisp & 7) >= 1) {
				if (ltOPNDisp[i + 9] > 0) {
					ktOPNDisp[i + 9] = PSGConvOPNDisp(
											opn_reg[OPN_WHG][i * 2 + 0],
											opn_reg[OPN_WHG][i * 2 + 1],
											opn_scale[OPN_WHG]);
				}
				else {
					ktOPNDisp[i + 9] = -1;
				}
			}

			if (tmp > 0) {
				if (ltOPNDisp[i + tmp] > 0) {
					ktOPNDisp[i + tmp] = PSGConvOPNDisp(
											opn_reg[OPN_THG][i * 2 + 0],
											opn_reg[OPN_THG][i * 2 + 1],
											opn_scale[OPN_THG]);
				}
				else {
					ktOPNDisp[i + tmp] = -1;
				}
			}
		}
		else {
			if (ltOPNDisp[i] > 0) {
				ktOPNDisp[i] = PSGConvOPNDisp(opn_reg[OPN_THG][i * 2 + 0],
										opn_reg[OPN_THG][i * 2 + 1],
										opn_scale[OPN_THG]);
			}
			else {
				ktOPNDisp[i] = -1;
			}
		}
	}
}

/*
 *	OPN�f�B�X�v���C�E�C���h�E
 *	���x���X�e�[�^�X
 */
static void FASTCALL StatLevOPNDisp(void)
{
	BYTE m;
	BYTE p;
	int i;

	/* ��U�擾���� */
	for (i=0; i<18; i++) {
		if (GetMute(i)) {
			ltOPNDisp[i] = 0;
		}
		else {
			ltOPNDisp[i] = GetLevelSnd(i);
		}
	}

	/* SSG�}�X�N�`�F�b�N:OPN */
	m = opn_reg[OPN_STD][7];
	p = 9;
	for (i=0; i<3; i++) {
		if ((m & p) == p) {
			ltOPNDisp[i + 0 + 3] = 0;
		}
		p <<= 1;
	}

	/* SSG�}�X�N�`�F�b�N:WHG */
	m = opn_reg[OPN_WHG][7];
	p = 9;
	for (i=0; i<3; i++) {
		if ((m & p) == p) {
			ltOPNDisp[i + 6 + 3] = 0;
		}
		p <<= 1;
	}

	/* SSG�}�X�N�`�F�b�N:THG */
	m = opn_reg[OPN_THG][7];
	p = 9;
	for (i=0; i<3; i++) {
		if ((m & p) == p) {
			ltOPNDisp[i + 12 + 3] = 0;
		}
		p <<= 1;
	}

	/* THG��Ԃ�PSG���Ɉړ� */
	if (nOPNDisp & 16) {
		for (i=0; i<3; i++) {
			ltOPNDisp[i] = ltOPNDisp[i + 15];
		}
	}
	else if (nOPNDisp == 8) {
		for (i=0; i<3; i++) {
			ltOPNDisp[i + 6] = ltOPNDisp[i + 15];
		}
	}
	else if (nOPNDisp == 9) {
		for (i=0; i<3; i++) {
			ltOPNDisp[i + 12] = ltOPNDisp[i + 15];
		}
	}
}

/*
 *	OPN�f�B�X�v���C�E�C���h�E
 *	������Z�b�g
 */
static void FASTCALL StrOPNDisp(char *string, int x, int y, int color)
{
	char ch;

	ASSERT(string);
	ASSERT((x >= 0) && (x < 49));
	ASSERT((y >= 0) && (y < 18));
	ASSERT((color >= 0) && (color < 16));

	/* �������[�v */
	for (;;) {
		/* �����擾 */
		ch = *string++;
		if (ch == '\0') {
			break;
		}

		/* x�`�F�b�N */
		if (x >= 49) {
			continue;
		}

		/* �Z�b�g */
		ctOPNDisp[y][x * 2 + 0] = ch;
		ctOPNDisp[y][x * 2 + 1] = (BYTE)color;
		x++;
	}
}

/*
 *	OPN�f�B�X�v���C�E�C���h�E
 *	�����f�[�^�Z�b�g
 */
static void FASTCALL StatChrOPNDisp(void)
{
	const BYTE cslot[4] = {2, 3, 3, 4};
	int i;
	int j;
	int n;
	int ch;
	char string[128];
	int no;
	BOOL fm_flag;
	BYTE alg;
	BYTE tl;
	int vols;

	/* �i���o�[���� */
	if (nOPNDisp & 16) {
		n = 0;
	}
	else {
		n = (((nOPNDisp & 7) + 1) * 6);
	}

	/* THG���g�p���̂�PSG��ǉ� */
	if (nOPNDisp & 8) {
		n += 3;
	}

	no = OPN_STD;
	for (i=0; i<n; i++) {
		/* �|�C���^�ݒ� */
		if ((i == 6) && (nOPNDisp != 8)) {
			no = OPN_WHG;
		}
		if ((i == 12) || ((nOPNDisp == 8) && (i == 6))) {
			no = OPN_THG;
		}
		if ((i == 0) && (nOPNDisp & 16)) {
			no = OPN_THG;
		}

		/* �`�����l�� */
		ch = i;
		if (nOPNDisp & 16) {
			ch = (i % 3) + 15;
			fm_flag = FALSE;
			_snprintf(string, 128, "PSG%1d", (i % 6) + 1);
		}
		else {
			if ((i % 6) < 3) {
				fm_flag = TRUE;
			}
			else {
				fm_flag = FALSE;
			}
			if (i < 6) {
				_snprintf(string, 128, "OPN%1d", (i % 6) + 1);
			}
			else {
				if (i < 12) {
					_snprintf(string, 128, "WHG%1d", (i % 6) + 1);
				}
				else {
					_snprintf(string, 128, "THG%1d", (i % 6) + 1);
				}
				if (nOPNDisp & 8) {
					if (i >= (int)((nOPNDisp & 7) + 1) * 6) {
						ch = (i % 3) + 15;
						fm_flag = FALSE;
						_snprintf(string, 128, "PSG%1d", (i % 6) + 1);
					}
				}
			}
		}
		StrOPNDisp(string, 0, i, 7);

		/* ���g�� */
		if (fm_flag) {
			j = opn_reg[no][0xa4 + (i % 3)];
			j <<= 8;
			j |= opn_reg[no][0xa0 + (i % 3)];
			_snprintf(string, sizeof(string), "F:$%04X", j);
		}
		else {
			j = opn_reg[no][(i % 3) * 2 + 1];
			j &= 0x0f;
			j <<= 8;
			j |= opn_reg[no][(i % 3) * 2 + 0];
			_snprintf(string, sizeof(string), "P:$%04X", j);
		}
		StrOPNDisp(string, 6, i, 12);

		/* �{�����[�� */
		if (GetMute(ch)) {
			strncpy(string, "MUTE          ", sizeof(string));
			StrOPNDisp(string, 14, i, 15);
		}
		else {
			if (fm_flag) {
				alg = (BYTE)(opn_reg[no][0xb0 + (i % 3)] & 0x07);
				if (alg >= 4) {
					alg -= (BYTE)4;
					vols = 127;
					for (j=0; j<cslot[alg]; j++) {
						tl = (BYTE)(opn_reg[no][0x4c - (j << 2) + (i % 3)] & 0x7f);
						if (vols > tl) {
							vols = tl;
						}
					}
				}
				else {
					vols = opn_reg[no][0x4c + (i % 3)];
				}

				/* 7bit�̂ݗL�� */
				vols &= 0x7f;
				_snprintf(string, 128, "V:%03d", 127 - vols);
			}
			else {
				j = opn_reg[no][8 + (i % 3)];
				/* 0�`15�ƁA16�ȏ�ɕ����� */
				j &= 0x1f;
				if (j >= 0x10) {
					j = 0x10;
				}
				_snprintf(string, 128, "V:%03d", j);
			}
			StrOPNDisp(string, 14, i, 12);

			if (fm_flag) {
				/* �L�[�I�� */
				if (opn_key[no][(i % 3)]) {
					strncpy(string, "KEYON   ", sizeof(string));
				}
				else {
					strncpy(string, "        ", sizeof(string));
				}
			}
			else {
				/* �~�L�T */
				j = opn_reg[no][7];
				if ((i % 3) == 1) {
					j >>= 1;
				}
				if ((i % 3) == 2) {
					j >>= 2;
				}
				j &= 0x09;

				switch (j) {
					case 0:
						_snprintf(string, 128, "T + N:%2d", (BYTE)(opn_reg[no][6] & 0x1f));
						break;
					case 1:
						_snprintf(string, 128, "NOISE:%2d", (BYTE)(opn_reg[no][6] & 0x1f));
						break;
					case 8:
						strncpy(string, "TONE    ", sizeof(string));
						break;
					case 9:
						strncpy(string, "        ", sizeof(string));
						break;
				}
			}
			StrOPNDisp(string, 20, i, 12);
		}
	}
}

/*
 *	OPN�f�B�X�v���C�E�C���h�E
 *	�`��
 */
static void FASTCALL DrawOPNDisp(HDC hDC, BOOL flag)
{
	HDC hMemDC;
	HBITMAP hBitmap;
	int first, end;
	int ys;

	ASSERT(hDC);

	/* �r�b�g�}�b�v�n���h���E�|�C���^�������Ȃ�A�������Ȃ� */
	if (!hOPNDisp || !pOPNDisp) {
		return;
	}

	/* �X�e�[�^�X�`�F�b�N */
	StatLevOPNDisp();
	StatKbdOPNDisp();
	StatChrOPNDisp();

	/* �Z�b�g�A�b�v���`��`�F�b�N */
	if (!SetupOPNDisp(&first, &end) && !flag) {
		return;
	}
	if (flag) {
		first = 0;
		if (nOPNDisp & 16) {
			end = 0;
		}
		else {
			end = (((nOPNDisp & 7) + 1) * 6);
		}

		/* THG���g�p���̂�PSG��ǉ� */
		if (nOPNDisp & 8) {
			end += 3;
		}
	}

	if (((nOPNDisp & 7) == 2) && !bPopupSwnd) {
		ys = (8 + 12 + 1);
	}
	else {
		ys = (8 * 3 + 1);
	}

	/* �r�b�g�}�b�v�n���h���E�|�C���^���L����������x�`�F�b�N */
	if (!hOPNDisp || !pOPNDisp) {
		return;
	}

	/* ������DC���쐬���A�r�b�g�}�b�v���Z���N�g */
	hMemDC = CreateCompatibleDC(hDC);
	ASSERT(hMemDC);
	hBitmap = (HBITMAP)SelectObject(hMemDC, hOPNDisp);

	/* �f�X�N�g�b�v�̏󋵂�����ł́A���ۂ����蓾��l */
	if (hBitmap) {
		/* �p���b�g�ݒ�ABitBlt */
		SetDIBColorTable(hMemDC, 0, 16, rgbOPNDisp);
		BitBlt(hDC,
				0, ys * first,
				rOPNDisp.right, ys * (end - first + 1), hMemDC,
				0, ys * first, SRCCOPY);

		/* �I�u�W�F�N�g�ăZ���N�g */
		SelectObject(hMemDC, hBitmap);
	}

	/* ������DC�폜 */
	DeleteDC(hMemDC);
}

/*
 *	OPN�f�B�X�v���C�E�C���h�E
 *	���t���b�V��
 */
void FASTCALL RefreshOPNDisp(void)
{
	HWND hWnd;
	HDC hDC;

	/* ��ɌĂ΂��̂ŁA���݃`�F�b�N���邱�� */
	if (hSubWnd[SWND_OPNDISP] == NULL) {
		return;
	}

	/* �`�� */
	hWnd = hSubWnd[SWND_OPNDISP];
	hDC = GetDC(hWnd);
	DrawOPNDisp(hDC, TRUE);
	ReleaseDC(hWnd, hDC);
}

/*
 *	OPN�f�B�X�v���C�E�C���h�E
 *	�ĕ`��
 */
static void FASTCALL PaintOPNDisp(HWND hWnd, BOOL flag)
{
	HDC hDC;
	PAINTSTRUCT ps;
	int i;

	ASSERT(hWnd);

	/* ���[�N�G���A�����ׂĖ���� */
	for (i=0; i<18; i++) {
		knOPNDisp[i] = -2;
		lnOPNDisp[i] = -1;
	}
	memset(cnOPNDisp, 0xff, sizeof(cnOPNDisp));

	/* �`�� */
	hDC = BeginPaint(hWnd, &ps);
	ASSERT(hDC);
	DrawOPNDisp(hDC, flag);
	EndPaint(hWnd, &ps);
}

/*
 *	OPN�f�B�X�v���C�E�C���h�E
 *	�~���[�g�����@�`�����l���v�Z
 */
static int CalcCh(LPARAM lParam)
{
	int y;
	int ch;
	
	y = HIWORD(lParam);
	
	if (((nOPNDisp & 7) == 2) && !bPopupSwnd) {
		/* THG */
		ch = y / (8 + 12 + 1);
	}
	else {
		ch = y / (8 * 3 + 1);
		if (nOPNDisp & 16) {
			/* PSG only */
			ch = (ch % 3) + 15;
		}
		else {
			if ((nOPNDisp & 8) && (ch >= (int)(((nOPNDisp & 7) + 1) * 6))) {
				ch = (ch % 3) + 15;
			}
		}
	}

	return ch;
}

/*
 *	OPN�f�B�X�v���C�E�C���h�E
 *	�E�C���h�E�v���V�[�W��
 */
static LRESULT CALLBACK OPNDispProc(HWND hWnd, UINT message,
								 WPARAM wParam, LPARAM lParam)
{
	int i;
	static BOOL bDblClk = FALSE;

	/* ���b�Z�[�W�� */
	switch (message) {
		/* �E�C���h�E�ĕ`�� */
		case WM_PAINT:
			/* ���b�N���K�v */
			LockVM();
			PaintOPNDisp(hWnd, TRUE);
			UnlockVM();
			return 0;

		/* �w�i�`�� */
		case WM_ERASEBKGND:
			return TRUE;

		/* �~���[�g */
		case WM_LBUTTONUP:
			if (bDblClk) {
				bDblClk = FALSE;
			}
			else {
				LockVM();
				SetMute(CalcCh(lParam), !GetMute(CalcCh(lParam)));
				PaintOPNDisp(hWnd, FALSE);
				UnlockVM();
			}
			return 0;

		/* �\�����t */
		case WM_LBUTTONDBLCLK:
			LockVM();
			for (i=0; i<18; i++) {
				SetMute(i, (BOOL)(i != CalcCh(lParam)));
			}
			PaintOPNDisp(hWnd, FALSE);
			UnlockVM();
			bDblClk = TRUE;
			return 0;

		/* �~���[�g�S���� */
		case WM_RBUTTONUP:
			LockVM();
			for (i=0; i<18; i++) {
				SetMute(i, FALSE);
			}
			PaintOPNDisp(hWnd, FALSE);
			UnlockVM();
			return 0;

		/* �E�C���h�E�폜 */
		case WM_DESTROY:
			LockVM();

			/* ���C���E�C���h�E�֎����ʒm */
			DestroySubWindow(hWnd, NULL, NULL);

			/* �r�b�g�}�b�v��� */
			if (hOPNDisp) {
				DeleteObject(hOPNDisp);
				hOPNDisp = NULL;
				pOPNDisp = NULL;
			}

			UnlockVM();
			break;
	}

	/* �f�t�H���g �E�C���h�E�v���V�[�W�� */
	return DefWindowProc(hWnd, message, wParam, lParam);
}

/*
 *	OPN�f�B�X�v���C�E�C���h�E
 *	������
 */
void FASTCALL InitOPNDisp(HWND hWnd)
{
	BITMAPINFOHEADER *pbmi;
	HDC hDC;
	int i;

	/* �S�̃��[�N������ */
	pOPNDisp = NULL;
	hOPNDisp = NULL;

	/* �\���Ǘ����[�N������ */
	for (i=0; i<18; i++) {
		knOPNDisp[i] = -2;
		ktOPNDisp[i] = -1;
		lnOPNDisp[i] = -1;
		ltOPNDisp[i] = 0;
	}
	memset(cnOPNDisp, 0xff, sizeof(cnOPNDisp));
	memset(ctOPNDisp, 0, sizeof(ctOPNDisp));

	/* �r�b�g�}�b�v�w�b�_���� */
	pbmi = (BITMAPINFOHEADER*)malloc(sizeof(BITMAPINFOHEADER)
										 + sizeof(RGBQUAD) * 16);
	if (pbmi) {
		memset(pbmi, 0, sizeof(BITMAPINFOHEADER) + sizeof(RGBQUAD) * 16);
		pbmi->biSize = sizeof(BITMAPINFOHEADER);
		pbmi->biWidth = rOPNDisp.right;
		pbmi->biHeight = -rOPNDisp.bottom;
		pbmi->biPlanes = 1;
		pbmi->biBitCount = 4;
		pbmi->biCompression = BI_RGB;

		/* DC�擾�ADIB�Z�N�V�����쐬 */
		hDC = GetDC(hWnd);
		hOPNDisp = CreateDIBSection(hDC, (BITMAPINFO*)pbmi, DIB_RGB_COLORS,
								(void**)&pOPNDisp, NULL, 0);
		ReleaseDC(hWnd, hDC);
		free(pbmi);
	}
}

/*
 *	OPN�f�B�X�v���C�E�C���h�E
 *	�T�C�Y�ύX
 */
void FASTCALL ReSizeOPNDisp(void)
{
	HWND hWnd;
	UINT uMode;
	RECT wrect;
	RECT crect;
	char szWndName[128];
	int n;
	int ys;

	/* ��ɌĂ΂��̂ŁA���݃`�F�b�N���邱�� */
	if (hSubWnd[SWND_OPNDISP] == NULL) {
		return;
	}

	/* �W�I���g���ύX�`�F�b�N */
	hWnd = hSubWnd[SWND_OPNDISP];
	uMode = 0;
	n = 0;
	if (opn_use) {
		uMode = 0;
		n = 6;
	}
	if (whg_use) {
		uMode = 1;
		n = 12;
	}
	if (thg_use) {
		uMode = 2;
		n = 18;
	}
	if ((uMode < 2) && (fm7_ver == 1)) {
		uMode |= 8;
		n += 3;
	}
	if (!opn_use && (fm7_ver == 1)) {
		uMode = 24;
		n = 3;
	}
	if (IsIconic(hWnd)) {
		/* �A�C�R�����`�F�b�N */
		uMode |= 0x80000000;
	}
	if (uMode == nOPNDisp) {
		return;
	}

	/* ���b�N */
	LockVM();

	/* �E�C���h�E�^�C�g����ύX */
	if ((uMode & 16) ^ (nOPNDisp & 16)) {
		if (opn_use) {
			LoadString(hAppInstance, IDS_SWND_OPNDISP,
						szWndName, sizeof(szWndName));
		}
		else {
			LoadString(hAppInstance, IDS_SWND_PSGDISP,
						szWndName, sizeof(szWndName));
		}
		SetWindowText(hWnd, szWndName);
	}

	/* ���[�h�`�F���W */
	nOPNDisp = uMode;

	/* �r�b�g�}�b�v��� */
	if (hOPNDisp) {
		DeleteObject(hOPNDisp);
		hOPNDisp = NULL;
		pOPNDisp = NULL;
	}

	/* ���̑������� */
	if (((nOPNDisp & 7) == 2) && !bPopupSwnd) {
		ys = (8 + 12 + 1);
	}
	else {
		ys = (8 * 3 + 1);
	}

	rOPNDisp.bottom = (ys * n);
	InitOPNDisp(hWnd);

	/* �E�B���h�E��`�ύX */
	GetWindowRect(hWnd, &wrect);
	GetClientRect(hWnd, &crect);
	wrect.right -= wrect.left;
	wrect.right -= crect.right;
	wrect.right += (42 * 8);
	wrect.bottom -= wrect.top;
	wrect.bottom -= crect.bottom;
	wrect.bottom += (ys * n);
	SetWindowPos(hWnd, HWND_TOP, 0, 0, wrect.right, wrect.bottom,
							SWP_NOZORDER | SWP_NOMOVE);

	if (bPopupSwnd) {
		SetForegroundWindow(hMainWnd);
	}

	/* �A�����b�N */
	UnlockVM();
}

/*
 *	OPN�f�B�X�v���C�E�C���h�E
 *	�E�C���h�E�쐬
 */
HWND FASTCALL CreateOPNDisp(HWND hParent, int index)
{
	WNDCLASSEX wcex;
	char szClassName[] = "XM7_OPNDisp";
	char szWndName[128];
	RECT rect;
	RECT crect, wrect;
	HWND hWnd;
	DWORD dwStyle;

	ASSERT(hParent);

	/* �E�C���h�E��`���v�Z */
	PositioningSubWindow(hParent, &rect, index);

	/* �E�C���h�E�����́A42�h�b�g * 8 �I�N�^�[�u */
	rect.right = 42 * 8;

	/* �E�C���h�E�c���́A8 * 3 + 1�h�b�g * 6 �`�����l�� */
	if (opn_use) {
		rect.bottom = (8 * 3 + 1) * 6;
	}
	else {
		rect.bottom = (8 * 3 + 1) * 3;
	}

	/* �E�C���h�E�^�C�g�������� */
	if (opn_use) {
		LoadString(hAppInstance, IDS_SWND_OPNDISP,
					szWndName, sizeof(szWndName));
	}
	else {
		LoadString(hAppInstance, IDS_SWND_PSGDISP,
					szWndName, sizeof(szWndName));
	}

	/* �E�C���h�E�N���X�̓o�^ */
	memset(&wcex, 0, sizeof(wcex));
	wcex.cbSize = sizeof(wcex);
	wcex.style = CS_DBLCLKS | CS_VREDRAW | CS_HREDRAW;
	wcex.lpfnWndProc = OPNDispProc;
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
		rOPNDisp = rect;
		if (opn_use) {
			nOPNDisp = 0;
		}
		else {
			nOPNDisp = 24;
		}

		/* ���̑������� */
		InitOPNDisp(hWnd);
	}

	/* �|�b�v�A�b�v�E�C���h�E���̓A�N�e�B�u�E�C���h�E��O�ʂɕύX */
	if (bPopupSwnd) {
		SetForegroundWindow(hMainWnd);
	}

	/* ���ʂ������A�� */
	return hWnd;
}

/*-[ OPN���W�X�^�E�C���h�E ]-------------------------------------------------*/

/*
 *	OPN���W�X�^�E�C���h�E
 *	�Z�b�g�A�b�v(OPN���W�X�^�Z�b�g)
 */
static void FASTCALL SetupOPNRegSub(BYTE *p, int cx, BYTE *reg, int x, int y, BOOL fm_flag)
{
	int i;
	int j;
	char string[128];

	/* X�����K�C�h�\�� */
	strncpy(string, "+0+1+2+3+4+5+6+7+8+9+A+B+C+D+E+F", sizeof(string));
	memcpy(&p[cx * y + x], string, strlen(string));

	/* ���[�v */
	for (i=0; i<16; i++) {
		if ((i != 0) && (nOPNReg & 16)) {
			continue;
		}

		for (j=0; j<16; j++) {
			if ((i == 0) || fm_flag) {
				_snprintf(string, sizeof(string), "%02X", reg[i * 16 + j]);
			}
			else {
				strncpy(string, "--", sizeof(string));
			}
			memcpy(&p[cx * (y + i + 2) + x + j * 2], string, strlen(string));
		}
	}
}

/*
 *	OPN���W�X�^�E�C���h�E
 *	�Z�b�g�A�b�v(�v���X�P�[��)
 */
static void FASTCALL SetupOPNRegPs(BYTE *p, int cx, int scale, int x, int y)
{
	char string[128];
	int i;

	/* �v���X�P�[�� */
	if (scale == 2) {
		i = 1;
	}
	else {
		i = (scale * 2) / 3;
	}
	if (nOPNReg & 16) {
		strncpy(string, "Prescaler   FM : -/-   PSG : 1/2", sizeof(string));
	}
	else {
		_snprintf(string, sizeof(string), "Prescaler   FM : 1/%d   PSG : 1/%d",
			scale, i);
	}
	memcpy(&p[cx * y + x], string, strlen(string));
}

/*
 *	OPN���W�X�^�E�C���h�E
 *	�Z�b�g�A�b�v
 */
static void FASTCALL SetupOPNReg(BYTE *p, int x, int y)
{
	char string[128];
	int i;

	ASSERT(p);
	ASSERT(x > 0);
	ASSERT(y > 0);

	/* ��U�X�y�[�X�Ŗ��߂� */
	memset(p, 0x20, x * y);

	/* Y�����K�C�h�\�� */
	for (i=0; i<16; i++) {
		if ((i != 0) && (nOPNReg & 16)) {
			continue;
		}

		_snprintf(string, sizeof(string), "+%02X", i * 16);
		if (nOPNReg & 16) {
			memcpy(&p[x * (i + 4) + 0], string, strlen(string));
		}
		else {
			memcpy(&p[x * (i + 6) + 0], string, strlen(string));
		}
	}

	/* PSG */
	if (nOPNReg & 16) {
#if defined(ROMEO)
		strncpy(string, "PSG (Standard / fmgen)", sizeof(string));
#else
		strncpy(string, "PSG (Standard)", sizeof(string));
#endif
		memcpy(&p[x * 0 + 4], string, strlen(string));
		SetupOPNRegSub(p, x, opn_reg[OPN_THG], 4, 2, FALSE);
	}
	else {
#if defined(ROMEO)
		/* OPN */
		if (bUseRomeo) {
			strncpy(string, "OPN (Standard / ROMEO)", sizeof(string));
		}
		else {
			strncpy(string, "OPN (Standard / fmgen)", sizeof(string));
		}
#else
		strncpy(string, "OPN (Standard)", sizeof(string));
#endif
		memcpy(&p[x * 0 + 4], string, strlen(string));
		SetupOPNRegPs(p, x, opn_scale[OPN_STD], 4, 2);
		SetupOPNRegSub(p, x, opn_reg[OPN_STD], 4, 4, TRUE);
	}

	/* WHG */
	if (((nOPNReg & 7) >= 1) && (x > 38)) {
#if defined(ROMEO)
		if (bUseRomeo) {
			strncpy(string, "WHG (Extension / ROMEO+fmgen)", sizeof(string));
		}
		else {
			strncpy(string, "WHG (Extension / fmgen)", sizeof(string));
		}
#else
		strncpy(string, "WHG (Extension)", sizeof(string));
#endif
		memcpy(&p[x * 0 + 4 + 32 + 2], string, strlen(string));
		SetupOPNRegPs(p, x, opn_scale[OPN_WHG], 38, 2);
		SetupOPNRegSub(p, x, opn_reg[OPN_WHG], 38, 4, TRUE);
	}
	else if ((nOPNReg == 8) && (x > 38)) {
#if defined(ROMEO)
		strncpy(string, "PSG (Standard / fmgen)", sizeof(string));
#else
		strncpy(string, "PSG (Standard)", sizeof(string));
#endif
		memcpy(&p[x * 0 + 4 + 32 + 2], string, strlen(string));
		SetupOPNRegSub(p, x, opn_reg[OPN_THG], 38, 4, FALSE);
	}

	/* THG */
	if (((nOPNReg & 7) >= 2) && (x > 72)) {
#if defined(ROMEO)
		strncpy(string, "THG (Extension / fmgen)", sizeof(string));
#else
		strncpy(string, "THG (Extension)", sizeof(string));
#endif
		memcpy(&p[x * 0 + 38 + 32 + 2], string, strlen(string));
		SetupOPNRegPs(p, x, opn_scale[OPN_THG], 72, 2);
		SetupOPNRegSub(p, x, opn_reg[OPN_THG], 72, 4, TRUE);
	}
	else if ((nOPNReg == 9) && (x > 72)) {
#if defined(ROMEO)
		strncpy(string, "PSG (Standard / fmgen)", sizeof(string));
#else
		strncpy(string, "PSG (Standard)", sizeof(string));
#endif
		memcpy(&p[x * 0 + 38 + 32 + 2], string, strlen(string));
		SetupOPNRegSub(p, x, opn_reg[OPN_THG], 72, 4, FALSE);
	}
}

/*
 *	OPN���W�X�^�E�C���h�E
 *	�`��
 */
static void FASTCALL DrawOPNReg(HWND hWnd, HDC hDC)
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
	if (!pOPNReg) {
		return;
	}
	SetupOPNReg(pOPNReg, x, y);

	/* �`�� */
	DrawWindowText(hDC, pOPNReg, x, y);
}

/*
 *	OPN���W�X�^�E�C���h�E
 *	���t���b�V��
 */
void FASTCALL RefreshOPNReg(void)
{
	HWND hWnd;
	HDC hDC;

	/* ��ɌĂ΂��̂ŁA���݃`�F�b�N���邱�� */
	if (hSubWnd[SWND_OPNREG] == NULL) {
		return;
	}

	/* �`�� */
	hWnd = hSubWnd[SWND_OPNREG];
	hDC = GetDC(hWnd);
	DrawOPNReg(hWnd, hDC);
	ReleaseDC(hWnd, hDC);
}

/*
 *	OPN���W�X�^�E�C���h�E
 *	�T�C�Y�ύX
 */
void FASTCALL ReSizeOPNReg(void)
{
	HWND hWnd;
	UINT uMode;
	RECT wrect;
	RECT crect;
	char szWndName[128];
	int cx;
	int cy;
	BYTE *p;

	/* ��ɌĂ΂��̂ŁA���݃`�F�b�N���邱�� */
	if (hSubWnd[SWND_OPNREG] == NULL) {
		return;
	}

	/* �W�I���g���ύX�`�F�b�N */
	hWnd = hSubWnd[SWND_OPNREG];
	uMode = 0;
	cx = 36;
	cy = 22;
	if (whg_use) {
		uMode = 1;
		cx = 36 + 32 + 2;
	}
	if (thg_use) {
		uMode = 2;
		cx = 36 + (32 + 2) * 2;
	}
	if ((uMode < 2) && (fm7_ver == 1)) {
		uMode |= 8;
		cx += 32 + 2;
	}
	if (!opn_use && (fm7_ver == 1)) {
		uMode = 24;
		cx = 36;
		cy = 5;
	}
	if (IsIconic(hWnd)) {
		/* �A�C�R�����`�F�b�N */
		uMode |= 0x80000000;
	}
	if (uMode == nOPNReg) {
		return;
	}

	/* ���b�N */
	LockVM();

	/* �E�C���h�E�^�C�g����ύX */
	if ((uMode & 16) ^ (nOPNReg & 16)) {
		if (opn_use) {
			LoadString(hAppInstance, IDS_SWND_OPNREG,
						szWndName, sizeof(szWndName));
		}
		else {
			LoadString(hAppInstance, IDS_SWND_PSGREG,
						szWndName, sizeof(szWndName));
		}
		SetWindowText(hWnd, szWndName);
	}

	/* ���[�h�`�F���W */
	nOPNReg = uMode;

	/* �o�b�t�@���Ď擾���A0xFF�Ŗ��߂� */
	p = realloc(pOPNReg, 2 * cx * cy);
	ASSERT(p);
	pOPNReg = p;
	memset(pOPNReg, 0xff, 2 * cx * cy);

	/* �E�B���h�E��`�ύX */
	GetWindowRect(hWnd, &wrect);
	GetClientRect(hWnd, &crect);
	wrect.right -= wrect.left;
	wrect.right -= crect.right;
	wrect.right += cx * lCharWidth;
	wrect.bottom -= wrect.top;
	wrect.bottom -= crect.bottom;
	wrect.bottom += cy * lCharHeight;
	SetWindowPos(hWnd, HWND_TOP, 0, 0, wrect.right, wrect.bottom,
							SWP_NOZORDER | SWP_NOMOVE);

	if (bPopupSwnd) {
		SetForegroundWindow(hMainWnd);
	}

	/* �A�����b�N */
	UnlockVM();
}

/*
 *	OPN���W�X�^�E�C���h�E
 *	�ĕ`��
 */
static void FASTCALL PaintOPNReg(HWND hWnd)
{
	HDC hDC;
	PAINTSTRUCT ps;
	RECT rect;
	BYTE *p;
	int x, y;

	ASSERT(hWnd);

	/* �|�C���^��ݒ�(���݂��Ȃ���Ή������Ȃ�) */
	p = pOPNReg;
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
	DrawOPNReg(hWnd, hDC);
	EndPaint(hWnd, &ps);
}

/*
 *	OPN���W�X�^�E�C���h�E
 *	�E�C���h�E�v���V�[�W��
 */
static LRESULT CALLBACK OPNRegProc(HWND hWnd, UINT message,
								 WPARAM wParam, LPARAM lParam)
{
	/* ���b�Z�[�W�� */
	switch (message) {
		/* �E�C���h�E�ĕ`�� */
		case WM_PAINT:
			/* ���b�N���K�v */
			LockVM();
			PaintOPNReg(hWnd);
			UnlockVM();
			return 0;

		/* �E�C���h�E�폜 */
		case WM_DESTROY:
			LockVM();

			/* ���C���E�C���h�E�֎����ʒm */
			DestroySubWindow(hWnd, &pOPNReg, NULL);

			UnlockVM();
			break;
	}

	/* �f�t�H���g �E�C���h�E�v���V�[�W�� */
	return DefWindowProc(hWnd, message, wParam, lParam);
}

/*
 *	OPN���W�X�^�E�C���h�E
 *	�E�C���h�E�쐬
 */
HWND FASTCALL CreateOPNReg(HWND hParent, int index)
{
	WNDCLASSEX wcex;
	char szClassName[] = "XM7_OPNReg";
	char szWndName[128];
	RECT rect;
	RECT crect, wrect;
	HWND hWnd;
	DWORD dwStyle;

	ASSERT(hParent);

	/* �E�C���h�E��`���v�Z */
	PositioningSubWindow(hParent, &rect, index);
	rect.right = lCharWidth * 36;
	if (opn_use) {
		rect.bottom = lCharHeight * 22;
	}
	else {
		rect.bottom = lCharHeight * 5;
	}

	/* �E�C���h�E�^�C�g��������A�o�b�t�@�m�� */
	if (opn_use) {
		LoadString(hAppInstance, IDS_SWND_OPNREG,
					szWndName, sizeof(szWndName));
		nOPNReg = 0;
	}
	else {
		LoadString(hAppInstance, IDS_SWND_PSGREG,
					szWndName, sizeof(szWndName));
		nOPNReg = 24;
	}
	pOPNReg = malloc(2 * (rect.right / lCharWidth) *
								(rect.bottom / lCharHeight));

	/* �E�C���h�E�N���X�̓o�^ */
	memset(&wcex, 0, sizeof(wcex));
	wcex.cbSize = sizeof(wcex);
	wcex.style = CS_VREDRAW | CS_HREDRAW;
	wcex.lpfnWndProc = OPNRegProc;
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

/*-[ FDC�E�C���h�E ]---------------------------------------------------------*/

/*
 *	FDC�E�C���h�E
 *	�Z�b�g�A�b�v(�t���O)
 */
static void FASTCALL SetupFDCFlag(BYTE *p, BYTE length, char *title, BYTE flag)
{
	char string[20];
	int i;

	/* ������ */
	memset(string, 0x20, length);
	string[length] = '\0';

	/* �R�s�[ */
	for (i=0; i<length; i++) {
		if (title[i] == '\0') {
			break;
		}
		string[i] = title[i];
	}

	/* �t���O�ɉ����Đݒ� */
	if (flag) {
		strncpy(&string[length - 3], " On", 3);
	}
	else {
		strncpy(&string[length - 3], "Off", 3);
	}

	/* �Z�b�g */
	memcpy(p, string, length);
}

/*
 *	FDC�E�C���h�E
 *	�Z�b�g�A�b�v(�R�}���h)
 */
static void FASTCALL SetupFDCCmd(BYTE *p, int x, int cx)
{
	const BYTE steprate[4] = { 6, 12, 20, 30 };

	BYTE high, low;
	char buffer[128];
	int y;

	ASSERT(p);
	ASSERT(cx > 0);

	/* ������ */
	y = 0;

#if XM7_VER == 1
	if (!fdc_enable && (fm_subtype != FMSUB_FM77)) {
#else
	if (!fdc_enable && (fm7_ver == 1)) {
#endif
		/* FDC�f�B�Z�[�u�� */
		strncpy(buffer, "DISABLE", sizeof(buffer));
		memcpy(&p[x + y * cx], buffer, strlen(buffer));
		return;
	}

#if XM7_VER >= 3
	/* 2DD���[�h */
	if (fdc_2ddmode) {
		strncpy(buffer, "2DD Mode", sizeof(buffer));
		memcpy(&p[x + 2 * cx], buffer, strlen(buffer));
	}

	/* DMA */
	if (dma_pcr & 0x01) {
		_snprintf(buffer, sizeof(buffer),  "DMA %04X", dma_bcr[0]);
		memcpy(&p[10 + 2 * cx], buffer, strlen(buffer));
	}
#endif

	if (fdc_command == 0xff) {
		/* �R�}���h���� */
		strncpy(buffer, "NO COMMAND", sizeof(buffer));
		memcpy(&p[x + y * cx], buffer, strlen(buffer));
		return;
	}

	high = (BYTE)(fdc_command >> 4);
	low = (BYTE)(fdc_command & 0x0f);
	if (high < 8) {
		/* TYPE I */
		switch (high) {
			/* RESTORE */
			case 0:
				strncpy(buffer, "RESTORE", sizeof(buffer));
				break;
			/* SEEK */
			case 1:
				strncpy(buffer, "SEEK", sizeof(buffer));
				break;
			/* STEP */
			case 2:
			case 3:
				strncpy(buffer, "STEP", sizeof(buffer));
				break;
			/* STEP IN */
			case 4:
			case 5:
				strncpy(buffer, "STEP IN", sizeof(buffer));
				break;
			/* STEP IN */
			case 6:
			case 7:
				strncpy(buffer, "STEP OUT", sizeof(buffer));
				break;
		}
		memcpy(&p[x + y * cx], buffer, strlen(buffer));
		y++;

		strncpy(buffer, "(TYPE I)", sizeof(buffer));
		memcpy(&p[x + y * cx], buffer, strlen(buffer));
		y += 3;

		_snprintf(buffer, sizeof(buffer),  "Step Rate     %2dms",
			steprate[low & 0x03]);
		memcpy(&p[x + y * cx], buffer, strlen(buffer));
		y++;

		SetupFDCFlag(&p[x + y * cx], 18, "Verify", (BYTE)(low & 0x04));
		y++;

		strncpy(buffer, "Head        ", sizeof(buffer));
		if (low & 0x08) {
			strncat(buffer, "  Load", sizeof(buffer) - strlen(buffer) - 1);
		}
		else {
			strncat(buffer, "Unload", sizeof(buffer) - strlen(buffer) - 1);
		}
		memcpy(&p[x + y * cx], buffer, strlen(buffer));
		y++;

		if (high >= 2) {
			SetupFDCFlag(&p[x + y * cx], 18, "Update Track", (BYTE)(low & 0x10));
		}
		return;
	}
	else if (high == 0x0d) {
		/* TYPE IV */
		strncpy(buffer, "FORCE INTERRUPT", sizeof(buffer));
		memcpy(&p[x + y * cx], buffer, strlen(buffer));
		y++;

		strncpy(buffer, "(TYPE IV)", sizeof(buffer));
		memcpy(&p[x + y * cx], buffer, strlen(buffer));
		y += 3;

		SetupFDCFlag(&p[x + (y + 0) * cx], 18, "READY  In", (BYTE)(low & 0x01));
		SetupFDCFlag(&p[x + (y + 1) * cx], 18, "READY Out", (BYTE)(low & 0x02));
		SetupFDCFlag(&p[x + (y + 2) * cx], 18, "INDEX", (BYTE)(low & 0x04));
		SetupFDCFlag(&p[x + (y + 3) * cx], 18, "One Shot", (BYTE)(low & 0x08));
	}
	else {
		/* TYPE II/III */
		switch (high) {
			case 0x08:
			case 0x09:
				strncpy(buffer, "READ DATA", sizeof(buffer));
				break;
			case 0x0a:
			case 0x0b:
				strncpy(buffer, "WRITE DATA", sizeof(buffer));
				break;
			case 0x0c:
				strncpy(buffer, "READ ADDRESS", sizeof(buffer));
				break;
			case 0x0e:
				strncpy(buffer, "READ TRACK", sizeof(buffer));
				break;
			case 0x0f:
				strncpy(buffer, "WRITE TRACK", sizeof(buffer));
				break;
		}
		memcpy(&p[x + y * cx], buffer, strlen(buffer));
		y++;

		if (high < 0x0c) {
			strncpy(buffer, "(TYPE II)", sizeof(buffer));
			memcpy(&p[x + y * cx], buffer, strlen(buffer));
			y += 2;

			SetupFDCFlag(&p[x + y * cx], 18, "Multi Sector", (BYTE)(high & 0x01));
			y++;

			strncpy(buffer, "Compare Side   ", sizeof(buffer));
			if (low & 0x02) {
				if (low & 0x08) {
					strncat(buffer, "  0", sizeof(buffer) - strlen(buffer) - 1);
				}
				else {
					strncat(buffer, "  1", sizeof(buffer) - strlen(buffer) - 1);
				}
			} else {
				strncat(buffer, "Off", sizeof(buffer) - strlen(buffer) - 1);
			}
			memcpy(&p[x + y * cx], buffer, strlen(buffer));
			y++;

			strncpy(buffer, "Addr. Mark ", sizeof(buffer));
			if (low & 0x01) {
				strncat(buffer, "Deleted", sizeof(buffer) - strlen(buffer) - 1);
			}
			else {
				strncat(buffer, " Normal", sizeof(buffer) - strlen(buffer) - 1);
			}
			memcpy(&p[x + y * cx], buffer, strlen(buffer));
			y++;
		}
		else {
			strncpy(buffer, "(TYPE III)", sizeof(buffer));
			memcpy(&p[x + y * cx], buffer, strlen(buffer));
			y += 5;
		}

		_snprintf(buffer, sizeof(buffer),  "Total Bytes   %04X", fdc_totalcnt);
		memcpy(&p[x + y * cx], buffer, strlen(buffer));
		y++;

		_snprintf(buffer, sizeof(buffer),  "Xfer. Bytes   %04X", fdc_nowcnt);
		memcpy(&p[x + y * cx], buffer, strlen(buffer));
	}
}

/*
 *	FDC�E�C���h�E
 *	�Z�b�g�A�b�v(���W�X�^)
 */
static void FASTCALL SetupFDCReg(BYTE *p, int x, int cx)
{
	int y;
	char buffer[128];

	ASSERT(p);
	ASSERT(cx > 0);

	/* ������ */
	y = 0;

#if XM7_VER >= 3
	_snprintf(buffer, sizeof(buffer), "Drive %1d/%1d",
		fdc_drvregP, fdc_drvreg);
#else
	_snprintf(buffer, sizeof(buffer),  "Drive   %1d", fdc_drvreg);
#endif
	if (fdc_motor) {
		strncat(buffer, "( On)", sizeof(buffer) - strlen(buffer) - 1);
	}
	else {
		strncat(buffer, "(Off)", sizeof(buffer) - strlen(buffer) - 1);
	}
	memcpy(&p[x + y * cx], buffer, strlen(buffer));
	y++;

	if (fdc_drvreg < FDC_DRIVES) {
		_snprintf(buffer, sizeof(buffer),  "Track       %02X",
			fdc_track[fdc_drvreg]);
		memcpy(&p[x + y * cx], buffer, strlen(buffer));
	}
	y++;

	_snprintf(buffer, sizeof(buffer),  "Track  Reg. %02X", fdc_trkreg);
	memcpy(&p[x + y * cx], buffer, strlen(buffer));
	y++;

	_snprintf(buffer, sizeof(buffer),  "Sector Reg. %02X", fdc_secreg);
	memcpy(&p[x + y * cx], buffer, strlen(buffer));
	y++;

	_snprintf(buffer, sizeof(buffer),  "Side   Reg. %02X", fdc_sidereg);
	memcpy(&p[x + y * cx], buffer, strlen(buffer));
	y++;

	_snprintf(buffer, sizeof(buffer),  "Data   Reg. %02X", fdc_datareg);
	memcpy(&p[x + y * cx], buffer, strlen(buffer));
	y++;

	SetupFDCFlag(&p[x + (y + 0) * cx], 14, "DRQ", (BYTE)(fdc_drqirq & 0x80));
	SetupFDCFlag(&p[x + (y + 1) * cx], 14, "IRQ", (BYTE)(fdc_drqirq & 0x40));
}

/*
 *	FDC�E�C���h�E
 *	�Z�b�g�A�b�v(�X�e�[�^�X)
 */
static void FASTCALL SetupFDCStat(BYTE *p, int x, int cx)
{
	int y;
	int type;
	int i;
	BYTE dat;
	BYTE bit;
	char buffer[128];

	ASSERT(p);
	ASSERT(cx > 0);

	/* ������ */
	y = 0;

	/* �^�C�v���� */
	type = 0;
	switch (fdc_cmdtype) {
		case 2:
			/* READ DATA */
			type = 1;
			break;
		case 3:
			/* WRITE DATA */
			type = 2;
			break;
		case 4:
			/* READ ADDRESS */
			type = 1;
			break;
		case 5:
			/* WRITE TRACK */
			type = 2;
			break;
		case 6:
			/* READ TRACK */
			type = 1;
			break;
		default:
			break;
	}

	/* �����ݒ� */
	dat = fdc_status;
	bit = 0x80;

	/* �W�r�b�g���[�v */
	for (i=7; i>=0; i--) {
		_snprintf(buffer, 128,  "bit%d ", i);
		if (dat & bit) {
			switch (i) {
				/* BUSY */
				case 0:
					strncat(buffer, "BUSY", sizeof(buffer) - strlen(buffer) - 1);
					break;
				/* INDEX or DATA REQUEST */
				case 1:
					if (type == 0) {
						strncat(buffer, "INDEX", sizeof(buffer) - strlen(buffer) - 1);
					}
					else {
						strncat(buffer, "DATA REQUEST", sizeof(buffer) - strlen(buffer) - 1);
					}
					break;
				/* TRACK00 or LOST DATA */
				case 2:
					if (type == 0) {
						strncat(buffer, "TRACK00", sizeof(buffer) - strlen(buffer) - 1);
					}
					else {
						strncat(buffer, "LOST DATA", sizeof(buffer) - strlen(buffer) - 1);
					}
					break;
				/* CRC ERROR */
				case 3:
					strncat(buffer, "CRC ERROR", sizeof(buffer) - strlen(buffer) - 1);
					break;
				/* SEEK ERROR or RECORD NOT FOUND */
				case 4:
					if (type == 0) {
						strncat(buffer, "SEEK ERROR", sizeof(buffer) - strlen(buffer) - 1);
					}
					else {
						strncat(buffer, "RECORD NOT FOUND", sizeof(buffer) - strlen(buffer) - 1);
					}
					break;
				/* HEAD ENGAGED or RECORD TYPE or WRITE FAULT */
				case 5:
					switch (type) {
						case 0:
							strncat(buffer, "HEAD ENGAGED", sizeof(buffer) - strlen(buffer) - 1);
							break;
						case 1:
							strncat(buffer, "RECORD TYPE", sizeof(buffer) - strlen(buffer) - 1);
							break;
						case 2:
							strncat(buffer, "WRITE FAULT", sizeof(buffer) - strlen(buffer) - 1);
							break;
					}
					break;
				/* WRITE PROTECT */
				case 6:
					strncat(buffer, "WRITE PROTECT", sizeof(buffer) - strlen(buffer) - 1);
					break;
				/* NOT READY */
				case 7:
					strncat(buffer, "NOT READY", sizeof(buffer) - strlen(buffer) - 1);
					break;
			}
		}
		else {
			strncat(buffer, "----------------", sizeof(buffer) - strlen(buffer) - 1);
		}
		bit >>= 1;
		memcpy(&p[x + y * cx], buffer, strlen(buffer));
		y++;
	}
}

/*
 *	FDC�E�C���h�E
 *	�Z�b�g�A�b�v
 */
static void FASTCALL SetupFDC(BYTE *p, int x, int y)
{
	ASSERT(p);
	ASSERT(x > 0);
	ASSERT(y > 0);

	/* ��U�X�y�[�X�Ŗ��߂� */
	memset(p, 0x20, x * y);

	/* �T�u�֐����Ă� */
	SetupFDCCmd(p, 0, x);
	SetupFDCReg(p, 20, x);
	SetupFDCStat(p, 36, x);
}

/*
 *	FDC�E�C���h�E
 *	�`��
 */
static void FASTCALL DrawFDC(HWND hWnd, HDC hDC)
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
	if (!pFDC) {
		return;
	}
	SetupFDC(pFDC, x, y);

	/* �`�� */
	DrawWindowText(hDC, pFDC, x, y);
}

/*
 *	FDC�E�C���h�E
 *	���t���b�V��
 */
void FASTCALL RefreshFDC(void)
{
	HWND hWnd;
	HDC hDC;

	/* ��ɌĂ΂��̂ŁA���݃`�F�b�N���邱�� */
	if (hSubWnd[SWND_FDC] == NULL) {
		return;
	}

	/* �`�� */
	hWnd = hSubWnd[SWND_FDC];
	hDC = GetDC(hWnd);
	DrawFDC(hWnd, hDC);
	ReleaseDC(hWnd, hDC);
}

/*
 *	FDC�E�C���h�E
 *	�ĕ`��
 */
static void FASTCALL PaintFDC(HWND hWnd)
{
	HDC hDC;
	PAINTSTRUCT ps;
	RECT rect;
	BYTE *p;
	int x, y;

	ASSERT(hWnd);

	/* �|�C���^��ݒ�(���݂��Ȃ���Ή������Ȃ�) */
	p = pFDC;
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
	DrawFDC(hWnd, hDC);
	EndPaint(hWnd, &ps);
}

/*
 *	FDC�E�C���h�E
 *	�E�C���h�E�v���V�[�W��
 */
static LRESULT CALLBACK FDCProc(HWND hWnd, UINT message,
								 WPARAM wParam, LPARAM lParam)
{
	/* ���b�Z�[�W�� */
	switch (message) {
		/* �E�C���h�E�ĕ`�� */
		case WM_PAINT:
			/* ���b�N���K�v */
			LockVM();
			PaintFDC(hWnd);
			UnlockVM();
			return 0;

		/* �E�C���h�E�폜 */
		case WM_DESTROY:
			LockVM();

			/* ���C���E�C���h�E�֎����ʒm */
			DestroySubWindow(hWnd, &pFDC, NULL);

			UnlockVM();
			break;
	}

	/* �f�t�H���g �E�C���h�E�v���V�[�W�� */
	return DefWindowProc(hWnd, message, wParam, lParam);
}

/*
 *	FDC�E�C���h�E
 *	�E�C���h�E�쐬
 */
HWND FASTCALL CreateFDC(HWND hParent, int index)
{
	WNDCLASSEX wcex;
	char szClassName[] = "XM7_FDC";
	char szWndName[128];
	RECT rect;
	RECT crect, wrect;
	HWND hWnd;
	DWORD dwStyle;

	ASSERT(hParent);

	/* �E�C���h�E��`���v�Z */
	PositioningSubWindow(hParent, &rect, index);
	rect.right = lCharWidth * 57;
	rect.bottom = lCharHeight * 8;

	/* �E�C���h�E�^�C�g��������A�o�b�t�@�m�� */
	LoadString(hAppInstance, IDS_SWND_FDC,
				szWndName, sizeof(szWndName));
	pFDC = malloc(2 * (rect.right / lCharWidth) *
								(rect.bottom / lCharHeight));

	/* �E�C���h�E�N���X�̓o�^ */
	memset(&wcex, 0, sizeof(wcex));
	wcex.cbSize = sizeof(wcex);
	wcex.style = CS_VREDRAW | CS_HREDRAW;
	wcex.lpfnWndProc = FDCProc;
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

/*-[ �o�u���������R���g���[���E�C���h�E ]------------------------------------*/

#if XM7_VER == 1
#if defined(BUBBLE)
/*
 *	�o�u���������R���g���[���E�C���h�E
 *	�Z�b�g�A�b�v(�R�}���h/���W�X�^)
 */
static void FASTCALL SetupBMCReg(BYTE *p, int x, int cx)
{
	char buffer[128];
	int y;

	ASSERT(p);
	ASSERT(cx > 0);

	/* ������ */
	y = 0;

	if (!bmc_enable || (fm_subtype != FMSUB_FM8)) {
		/* 32KB�o�u���J�Z�b�g���� */
		strncpy(buffer, "Disabled", sizeof(buffer));
		memcpy(&p[x + y * cx], buffer, strlen(buffer));
		return;
	}

	switch (bmc_command) {
		case 0x00:
			strncpy(buffer, "NO COMMAND", sizeof(buffer));
			break;
		case 0x01:
			strncpy(buffer, "READ", sizeof(buffer));
			break;
		case 0x02:
			strncpy(buffer, "WRITE", sizeof(buffer));
			break;
		case 0x04:
		case 0x0f:
			strncpy(buffer, "INITIALIZE", sizeof(buffer));
			break;
		default:
			_snprintf(buffer, sizeof(buffer),  "UNDEFINED     %02X", bmc_command & 0x0f);
			break;
	}
	memcpy(&p[x + y * cx], buffer, strlen(buffer));
	y+=2;

	_snprintf(buffer, sizeof(buffer),  "Data Reg.     %02X", bmc_datareg);
	memcpy(&p[x + y * cx], buffer, strlen(buffer));
	y++;

	_snprintf(buffer, sizeof(buffer),  "Unit          %2d", bmc_unit);
	memcpy(&p[x + y * cx], buffer, strlen(buffer));
	y++;

	_snprintf(buffer, sizeof(buffer),  "Page Reg.   %04X",
		bmc_pagereg & 0xfbff);
	memcpy(&p[x + y * cx], buffer, strlen(buffer));
	y++;

	_snprintf(buffer, sizeof(buffer),  "Count Reg.  %04X", bmc_countreg);
	memcpy(&p[x + y * cx], buffer, strlen(buffer));
	y++;

	_snprintf(buffer, sizeof(buffer),  "Total Bytes %04X", bmc_totalcnt);
	memcpy(&p[x + y * cx], buffer, strlen(buffer));
	y++;

	_snprintf(buffer, sizeof(buffer),  "Xfer. Bytes %04X", bmc_nowcnt);
	memcpy(&p[x + y * cx], buffer, strlen(buffer));
}

/*
 *	�o�u���������R���g���[���E�C���h�E
 *	�Z�b�g�A�b�v(�X�e�[�^�X)
 */
static void FASTCALL SetupBMCStat(BYTE *p, int x, int cx)
{
	int y;
	int i;
	BYTE bit;
	char buffer[128];

	ASSERT(p);
	ASSERT(cx > 0);

	/* ������ */
	y = 0;

	/* �����ݒ� */
	bit = 0x80;

	/* �W�r�b�g���[�v */
	for (i=7; i>=0; i--) {
		_snprintf(buffer, sizeof(buffer),  "bit%d ", i);
		if (bmc_status & bit) {
			switch (i) {
				/* BUSY */
				case 0:
					strncat(buffer, "BUSY", sizeof(buffer) - strlen(buffer) - 1);
					break;
				/* ERROR ANALYSIS */
				case 1:
					strncat(buffer, "ERROR ANALYSIS", sizeof(buffer) - strlen(buffer) - 1);
					break;
				/* WRITE PROTECT */
				case 2:
					strncat(buffer, "WRITE PROTECT", sizeof(buffer) - strlen(buffer) - 1);
					break;
				/* DEVICE READY */
				case 3:
					strncat(buffer, "DEVICE READY", sizeof(buffer) - strlen(buffer) - 1);
					break;
				/* ---------------- */
				case 4:
					strncat(buffer, "-----------------", sizeof(buffer) - strlen(buffer) - 1);
					break;
				/* RDA */
				case 5:
					strncat(buffer, "RDA", sizeof(buffer) - strlen(buffer) - 1);
					break;
				/* TDRA */
				case 6:
					strncat(buffer, "TDRA", sizeof(buffer) - strlen(buffer) - 1);
					break;
				/* CME */
				case 7:
					strncat(buffer, "CME", sizeof(buffer) - strlen(buffer) - 1);
					break;
			}
		}
		else {
			strncat(buffer, "-----------------", sizeof(buffer) - strlen(buffer) - 1);
		}
		bit >>= 1;
		memcpy(&p[x + y * cx], buffer, strlen(buffer));
		y++;
	}
}

/*
 *	�o�u���������R���g���[���E�C���h�E
 *	�Z�b�g�A�b�v(�G���[�X�e�[�^�X)
 */
static void FASTCALL SetupBMCErr(BYTE *p, int x, int cx)
{
	int y;
	int i;
	BYTE bit;
	char buffer[128];

	ASSERT(p);
	ASSERT(cx > 0);

	/* ������ */
	y = 0;

	/* �����ݒ� */
	bit = 0x80;

	/* �W�r�b�g���[�v */
	for (i=7; i>=0; i--) {
		if (bmc_errorreg & bit) {
			switch (i) {
				/* UNDEFINED COMMAND ERROR */
				case 0:
					strncpy(buffer, "UNDEFINED CMD ERR", sizeof(buffer));
					break;
				/* NO MARKER */
				case 1:
					strncpy(buffer, "NO MARKER", sizeof(buffer));
					break;
				/* MANY BAD LOOP */
				case 2:
					strncpy(buffer, "MANY BAD LOOP", sizeof(buffer));
					break;
				/* TRANSFER MISSING */
				case 3:
					strncpy(buffer, "TRANSFER MISSING", sizeof(buffer));
					break;
				/* CRC ERROR */
				case 4:
					strncpy(buffer, "CRC ERROR", sizeof(buffer));
					break;
				/* PAGE ADDRESS ERROR */
				case 5:
					strncpy(buffer, "PAGE ADDRESS ERR.", sizeof(buffer));
					break;
				/* DEVICE NOT READY */
				case 6:
				case 7:
					strncpy(buffer, "EJECT ERROR", sizeof(buffer));
					break;
			}
		}
		else {
			strncpy(buffer, "-----------------", sizeof(buffer));
		}
		bit >>= 1;
		memcpy(&p[x + y * cx], buffer, strlen(buffer));
		y++;
	}
}

/*
 *	�o�u���������R���g���[���E�C���h�E
 *	�Z�b�g�A�b�v
 */
static void FASTCALL SetupBMC(BYTE *p, int x, int y)
{
	ASSERT(p);
	ASSERT(x > 0);
	ASSERT(y > 0);

	/* ��U�X�y�[�X�Ŗ��߂� */
	memset(p, 0x20, x * y);

	/* �T�u�֐����Ă� */
	SetupBMCReg( p,  0, x);
	SetupBMCStat(p, 17, x);
	SetupBMCErr( p, 40, x);
}

/*
 *	�o�u���������R���g���[���E�C���h�E
 *	�`��
 */
static void FASTCALL DrawBMC(HWND hWnd, HDC hDC)
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
	if (!pBMC) {
		return;
	}
	SetupBMC(pBMC, x, y);

	/* �`�� */
	DrawWindowText(hDC, pBMC, x, y);
}

/*
 *	�o�u���������R���g���[���E�C���h�E
 *	���t���b�V��
 */
void FASTCALL RefreshBMC(void)
{
	HWND hWnd;
	HDC hDC;

	/* ��ɌĂ΂��̂ŁA���݃`�F�b�N���邱�� */
	if (hSubWnd[SWND_BMC] == NULL) {
		return;
	}

	/* �`�� */
	hWnd = hSubWnd[SWND_BMC];
	hDC = GetDC(hWnd);
	DrawBMC(hWnd, hDC);
	ReleaseDC(hWnd, hDC);
}

/*
 *	�o�u���������R���g���[���E�C���h�E
 *	�ĕ`��
 */
static void FASTCALL PaintBMC(HWND hWnd)
{
	HDC hDC;
	PAINTSTRUCT ps;
	RECT rect;
	BYTE *p;
	int x, y;

	ASSERT(hWnd);

	/* �|�C���^��ݒ�(���݂��Ȃ���Ή������Ȃ�) */
	p = pBMC;
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
	DrawBMC(hWnd, hDC);
	EndPaint(hWnd, &ps);
}

/*
 *	�o�u���������R���g���[���E�C���h�E
 *	�E�C���h�E�v���V�[�W��
 */
static LRESULT CALLBACK BMCProc(HWND hWnd, UINT message,
								 WPARAM wParam, LPARAM lParam)
{
	/* ���b�Z�[�W�� */
	switch (message) {
		/* �E�C���h�E�ĕ`�� */
		case WM_PAINT:
			/* ���b�N���K�v */
			LockVM();
			PaintBMC(hWnd);
			UnlockVM();
			return 0;

		/* �E�C���h�E�폜 */
		case WM_DESTROY:
			LockVM();

			/* ���C���E�C���h�E�֎����ʒm */
			DestroySubWindow(hWnd, &pBMC, NULL);

			UnlockVM();
			break;
	}

	/* �f�t�H���g �E�C���h�E�v���V�[�W�� */
	return DefWindowProc(hWnd, message, wParam, lParam);
}

/*
 *	�o�u���������R���g���[���E�C���h�E
 *	�E�C���h�E�쐬
 */
HWND FASTCALL CreateBMC(HWND hParent, int index)
{
	WNDCLASSEX wcex;
	char szClassName[] = "XM7_BMC";
	char szWndName[128];
	RECT rect;
	RECT crect, wrect;
	HWND hWnd;
	DWORD dwStyle;

	ASSERT(hParent);

	/* �E�C���h�E��`���v�Z */
	PositioningSubWindow(hParent, &rect, index);
	rect.right = lCharWidth * 57;
	rect.bottom = lCharHeight * 8;

	/* �E�C���h�E�^�C�g��������A�o�b�t�@�m�� */
	LoadString(hAppInstance, IDS_SWND_BMC,
				szWndName, sizeof(szWndName));
	pBMC = malloc(2 * (rect.right / lCharWidth) *
								(rect.bottom / lCharHeight));

	/* �E�C���h�E�N���X�̓o�^ */
	memset(&wcex, 0, sizeof(wcex));
	wcex.cbSize = sizeof(wcex);
	wcex.style = CS_VREDRAW | CS_HREDRAW;
	wcex.lpfnWndProc = BMCProc;
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
#endif
#endif

#endif	/* _WIN32 */
