/*
 *	FM-7 EMULATOR "XM7"
 *
 *	Copyright (C) 1999-2017 �o�h�D(Twitter:@xm6_original)
 *	Copyright (C) 2001-2017 Ryu Takegami (Twitter:@RyuTakegami)
 *
 *	[ Win32API �\�� ]
 *
 *	RHG����
 *	  2002.04.01		��f�B�X�v���C�o���N�����������̒ʒm����}����400���C
 *						�����[�h���̂ݗ}��(�Ӗ��s���c)
 *	  2002.04.23		����200���C�����[�h�ł��}��(���@�N)
 *	  2002.06.20		200���C��8�F���[�h�̗��o���N�ւ̏������ݎ��͒ʒm�����
 *						�}������悤�ɂ���(�Ƃ������Ȃ���܂���o�O�o���肵��)
 *						�n�[�h�E�F�A�E�B���h�E�p�ʒm�֐���V��
 *	  2002.09.15		V2�߂��o�[�W�����ƃ\�[�X�𓝍�
 *	  2004.03.17		�^��400���C���A�_�v�^�Ή�
 *	  2008.01.20		���Ȃ��������Ƃɂ���
 *	  2012.05.01		�t���X�N���[����ԕۑ������ւ̑Ή�
 *	  2012.07.01		�O���[�����j�^���[�h�ɑΉ�
 *	  2013.02.12		���X�^�����_�����O�ɑΉ�
 */

#ifdef _WIN32

#define STRICT
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <assert.h>
#include "xm7.h"
#include "device.h"
#include "subctrl.h"
#include "display.h"
#include "multipag.h"
#include "w32.h"
#include "w32_gdi.h"
#include "w32_dd.h"
#include "w32_draw.h"
#include "w32_sub.h"

/*
 *	�O���[�o�� ���[�N
 */
BOOL bFullScreen;							/* �t���X�N���[���t���O */
BOOL bFullRequested;						/* �t���X�N���[���v�� */
BOOL bDrawSelected;							/* �Z���N�g�ς݃t���O */
BOOL bFullScan;								/* �t���X�L����(Window) */
BOOL bFullScanFS;							/* �t���X�L����(FullScreen) */
BOOL bDoubleSize;							/* 2�{�g��t���O */
BOOL bPseudo400Line;						/* �^��400���C���t���O */
#if XM7_VER == 1
BOOL bGreenMonitor;							/* �O���[�����j�^�t���O */
#endif
#if XM7_VER == 2
BOOL bTTLMonitor;							/* TTL���j�^�t���O */
#endif
BOOL bRasterRendering;						/* ���X�^�����_�����O�t���O */

/*
 *	�X�^�e�B�b�N ���[�N
 */
static BOOL bNextFrameRender;				/* ���t���[���`��t���O */
static BOOL bDirtyLine[400];				/* �v���������t���O */

/*
 *	������
 */
void FASTCALL InitDraw(void)
{
	/* ���[�N�G���A������ */
	bFullScreen = FALSE;
	bFullRequested = FALSE;
	bDrawSelected = FALSE;
	bFullScan = FALSE;
	bFullScanFS = FALSE;
	bDoubleSize = FALSE;
	bPseudo400Line = FALSE;
#if XM7_VER == 1
	bGreenMonitor = FALSE;
#endif
#if XM7_VER == 2
	bTTLMonitor = FALSE;
#endif
	bRasterRendering = FALSE;
	bNextFrameRender = FALSE;
	memset(bDirtyLine, 0, sizeof(bDirtyLine));

	/* �N�����GDI�Ȃ̂ŁAGDI�������� */
	InitGDI();
}

/*
 *	�N���[���A�b�v
 */
void FASTCALL CleanDraw(void)
{
	/* �t���X�N���[���t���O�ɉ����āA�N���[���A�b�v */
	if (bFullScreen) {
		CleanDD();
	}
	else {
		CleanGDI();
	}
}

/*
 *  �Z���N�g
 */
BOOL FASTCALL SelectDraw(HWND hWnd)
{
	ASSERT(hWnd);

	/* �Z���N�g�ς� */
	bDrawSelected = TRUE;

	/* �N�����GDI��I�� */
	return SelectGDI(hWnd);
}

/*
 *	���[�h�؂�ւ�
 */
void FASTCALL ModeDraw(HWND hWnd, BOOL bFullFlag)
{
	ASSERT(hWnd);

	/* ����ƈ�v���Ă���΁A�ς���K�v�Ȃ� */
	if (bFullFlag == bFullScreen) {
		return;
	}

	if (bFullFlag) {
		/* �t���X�N���[���� */
		CleanGDI();
		InitDD();
		bFullScreen = TRUE;
		if (!SelectDD()) {
			/* �t���X�N���[�����s */
			bFullScreen = FALSE;
			CleanDD();
			InitGDI();
			SelectGDI(hWnd);
		}
	}
	else {
		/* �E�C���h�E�� */
		bFullScreen = FALSE;
		CleanDD();
		InitGDI();
		if (!SelectGDI(hWnd)) {
			/* �E�C���h�E���s */
			CleanGDI();
			bFullScreen = TRUE;
			InitDD();
			SelectDD();
		}
	}

	/* �ĕ`��̂��ߏ����׍H */
	display_notify();
}

/*
 *	�`��(�ʏ�)
 */
void FASTCALL OnDraw(HWND hWnd, HDC hDC)
{
	ASSERT(hWnd);
	ASSERT(hDC);

	/* �G���[�Ȃ牽�����Ȃ� */
	if (nErrorCode == 0) {
		if (bFullScreen) {
			if (bRasterRendering) {
				DrawPostRenderDD();
			}
			else {
				DrawDD();
			}
		}
		else {
			if (bRasterRendering) {
				DrawPostRenderGDI(hWnd, hDC);
			}
			else {
				DrawGDI(hWnd, hDC);
			}
		}
	}
}

/*
 *	�`��(WM_PAINT)
 */
void FASTCALL OnPaint(HWND hWnd)
{
	PAINTSTRUCT ps;
	HDC hDC;

	ASSERT(hWnd);

	hDC = BeginPaint(hWnd, &ps);

	/* �G���[�Ȃ牽�����Ȃ� */
	if (nErrorCode == 0) {
		/* �ĕ`��w�� */
		if (bFullScreen) {
			ReDrawDD();
		}
		else {
			ReDrawGDI();
		}

		/* �`�� */
		OnDraw(hWnd, hDC);
	}

	EndPaint(hWnd, &ps);
}

/*
 *	���j���[�J�n
 */
void FASTCALL EnterMenu(HWND hWnd)
{
	if (bFullScreen) {
		EnterMenuDD(hWnd);
	}
	else {
		EnterMenuGDI(hWnd);
	}
}

/*
 *	���j���[�I��
 */
void FASTCALL ExitMenu(void)
{
	if (bFullScreen) {
		ExitMenuDD();
	}
	else {
		ExitMenuGDI();
	}
}

/*-[ VM�Ƃ̐ڑ� ]-----------------------------------------------------------*/

/*
 *	�ĕ`�惉�X�^�ꊇ�ݒ�
 */
void FASTCALL SetDirtyFlag(int top, int bottom, BOOL flag)
{
	int y;

	if (bRasterRendering) {
		for (y = top; y < bottom; y++) {
			bDirtyLine[y] = flag;
		}
	}
}

/*
 *	VRAM�������ݒʒm
 */
void FASTCALL vram_notify(WORD addr, BYTE dat)
{
	WORD y;

	UNUSED(dat);

#if XM7_VER >= 2
#if XM7_VER >= 3
	if (screen_mode == SCR_200LINE) {
#else
	if (!mode320) {
#endif
		if (vram_active != vram_display) {
			return;
		}
	}
#endif

	/* Y���W�Z�o */
	if (bRasterRendering) {
#if XM7_VER >= 3
		switch (screen_mode) {
			case SCR_400LINE	:	addr &= 0x7fff;
									y = (WORD)(addr / 80);
									break;
			case SCR_262144		:
			case SCR_4096		:	addr &= 0x1fff;
									y = (WORD)(addr / 40);
									break;
			case SCR_200LINE	:	addr &= 0x3fff;
									y = (WORD)(addr / 80);
									break;
		}
#elif XM7_VER >= 2
		if (mode320) {
			addr &= 0x1fff;
			y = (WORD)(addr / 40);
		}
		else {
			addr &= 0x3fff;
			y = (WORD)(addr / 80);
		}
#elif defined(L4CARD)
		if (enable_400line) {
			addr -= vram_offset[0];
			addr &= 0x7fff;
			y = (WORD)(addr / 80);
		}
		else {
			addr &= 0x3fff;
			y = (WORD)(addr / 80);
		}
#else
		addr &= 0x3fff;
		y = (WORD)(addr / 80);
#endif
		if (y < 400) {
			bDirtyLine[y] = TRUE;
		}
	}

	if (bFullScreen) {
		VramDD(addr);
	}
	else {
		VramGDI(addr);
	}
}


/*
 *	TTL�p���b�g�ʒm
 */
void FASTCALL ttlpalet_notify(void)
{
	if (bRasterRendering) {
		bNextFrameRender = TRUE;
		SetDirtyFlag(now_raster, 400, TRUE);
	}

	if (bFullScreen) {
		DigitalDD();
	}
	else {
		DigitalGDI();
	}
}

/*
 *	�A�i���O�p���b�g�ʒm
 */
void FASTCALL apalet_notify(void)
{
	if (bRasterRendering) {
		bNextFrameRender = TRUE;
		SetDirtyFlag(now_raster, 400, TRUE);
	}

	if (bFullScreen) {
		AnalogDD();
	}
	else {
		AnalogGDI();
	}
}

/*
 *  �p���b�g�ύX�ʒm
 */
void FASTCALL refpalet_notify(void)
{
	bPaletteRefresh = TRUE;
}

/*
 *  �ĕ`��v���ʒm
 */
void FASTCALL display_notify(void)
{
	int i;
	int raster;

	if (bRasterRendering) {
		bNextFrameRender = TRUE;
		SetDirtyFlag(0, 400, TRUE);
		if (!run_flag) {
			raster = now_raster;
			for (i = 0; i < 400; i++) {
				now_raster = i;
				hblank_notify();
			}
			now_raster = raster;
		}
	}

	if (bFullScreen) {
		/* ReDraw�͖��ʂȃN���A���܂ނ̂ŁA�����׍H */
		AnalogDD();
	}
	else {
		AnalogGDI();
	}
}

/*
 *	VBLANK�I���v���ʒm
 */
void FASTCALL vblankperiod_notify(void)
{
	BOOL flag;
	int y;
	int ymax;

	if (bRasterRendering) {
		/* ���̃t���[���������I�ɏ��������邩 */
		if (bNextFrameRender) {
			bNextFrameRender = FALSE;
			SetDirtyFlag(0, 400, TRUE);
		}
		else {
			/* �����������K�v���`�F�b�N */
#if XM7_VER >= 3
			if (screen_mode == SCR_400LINE) {
				ymax = 400;
			}
			else {
				ymax = 200;
			}
#elif XM7_VER == 1 && defined(L4CARD)
			if (enable_400line) {
				ymax = 400;
			}
			else {
				ymax = 200;
			}
#else
			ymax = 200;
#endif
			flag = FALSE;
			for (y = 0; y < ymax; y++) {
				flag |= bDirtyLine[y];
			}
			if (!flag) {
				return;
			}
		}

		/* �S�̈�������_�����O */
		if (bFullScreen) {
			/* ReDraw�͖��ʂȃN���A���܂ނ̂ŁA�����׍H */
			AnalogDD();
		}
		else {
			AnalogGDI();
		}
	}
}

/*
 *	HBLANK�v���ʒm
 */
void FASTCALL hblank_notify(void)
{
	/* VM�������Ă��Ȃ��ꍇ�͉������Ȃ� */
	if (!run_flag) {
		return;
	}

	if (bRasterRendering) {
		if (bDirtyLine[now_raster]) {
			bDirtyLine[now_raster] = FALSE;
			if (bFullScreen) {
				DrawRasterDD(now_raster);
			}
			else {
				DrawRasterGDI(now_raster);
			}
		}
	}
}

/*
 *	�f�B�W�^�C�Y�v���ʒm
 */
void FASTCALL digitize_notify(void)
{
}

#if XM7_VER >= 3
/*
 *	�n�[�h�E�F�A�E�B���h�E�ʒm
 */
void FASTCALL window_notify(void)
{
	if (bFullScreen) {
		WindowDD();
	}
	else {
		WindowGDI();
	}
}
#endif

#if XM7_VER == 1 && defined(L4CARD)
/*
 *	�e�L�X�gVRAM�������ݒʒm
 */
void FASTCALL tvram_notify(WORD addr, BYTE dat)
{
	WORD ysize;
	WORD y;

	UNUSED(dat);

	if (bRasterRendering) {
		/* Y���W�Z�o */
		addr = (WORD)((addr - text_start_addr) & 0x0ffe);
		ysize = (WORD)((crtc_register[9] & 0x1f) + 1);
		if (width40_flag) {
			y = (WORD)((addr / 80) * ysize);
		}
		else {
			y = (WORD)((addr / 160) * ysize);
		}

		/* �I�[�o�[�`�F�b�N */
		if (y >= 400) {
			return;
		}

		/* �ĕ`��t���O��ݒ� */
		SetDirtyFlag(y, y + ysize, TRUE);
	}

	if (bFullScreen) {
		TvramDD(addr);
	}
	else {
		TvramGDI(addr);
	}
}

/*
 *  �e�L�X�g�ĕ`��v���ʒm
 */
void FASTCALL tvram_redraw_notify(void)
{
	bNextFrameRender = TRUE;
	SetDirtyFlag(now_raster, 400, TRUE);

	if (bFullScreen) {
		ReDrawTVRamDD();
	}
	else {
		ReDrawTVRamGDI();
	}
}
#endif

/*-[ V2 ]-------------------------------------------------------------------*/

#if XM7_VER <= 2 && defined(FMTV151)
/*
 *	V2
 *	0=���߁A1=�΁A2=��
 */
const BYTE nV2data[] = {
/*						  1 1 1 1 1 1 1 1 1 1 2 2 2 2 2 2 2 2 */
/*		1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 */
		1,1,1,2,0,0,0,0,1,1,1,2,0,0,0,0,0,1,1,1,1,1,1,1,2,0,0,
		1,1,1,2,0,0,0,0,1,1,1,2,0,0,0,0,1,1,1,1,1,1,1,1,1,2,0,
		1,1,1,2,0,0,0,0,1,1,1,2,0,0,0,1,1,1,1,2,0,0,1,1,1,1,2,
		1,1,1,2,0,0,0,0,1,1,1,2,0,0,0,1,1,1,2,0,0,0,0,1,1,1,2,
		1,1,1,2,0,0,0,0,1,1,1,2,0,0,0,0,0,0,0,0,0,0,0,1,1,1,2,
		1,1,1,2,0,0,0,0,1,1,1,2,0,0,0,0,0,0,0,0,0,0,1,1,1,1,2,
		1,1,1,2,0,0,0,0,1,1,1,2,0,0,0,0,0,0,0,0,0,1,1,1,1,2,0,
		1,1,1,2,0,0,0,0,1,1,1,2,0,0,0,0,0,0,0,0,1,1,1,1,2,0,0,
		1,1,1,2,0,0,0,0,1,1,1,2,0,0,0,0,0,0,0,1,1,1,1,2,0,0,0,
		1,1,1,1,2,0,0,1,1,1,1,2,0,0,0,0,0,0,1,1,1,1,2,0,0,0,0,
		0,1,1,1,1,2,1,1,1,1,2,0,0,0,0,0,0,1,1,1,1,2,0,0,0,0,0,
		0,0,1,1,1,1,1,1,1,2,0,0,0,0,0,0,1,1,1,1,2,0,0,0,0,0,0,
		0,0,0,1,1,1,1,1,2,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,2,
		0,0,0,0,1,1,1,2,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,2,
};

#endif

#endif	/* _WIN32 */
