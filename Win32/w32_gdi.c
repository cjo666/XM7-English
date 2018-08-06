/*
 *	FM-7 EMULATOR "XM7"
 *
 *	Copyright (C) 1999-2017 �o�h�D(Twitter:@xm6_original)
 *	Copyright (C) 2001-2017 Ryu Takegami (Twitter:@RyuTakegami)
 *
 *	[ Win32API �E�C���h�E�\�� ]
 *
 *	RHG����
 *	  2002.03.11		TrueColor�����_�����O���[�h�ɑΉ�
 *	  2002.05.31		4096�F�����_���̃��������C�g��4�o�C�g�P�ʂōs���悤�ύX
 *	  2002.06.05		26���F���[�h����32�r�b�gTrueColor�\���ɑΉ�
 *	  2002.06.20		8�F���[�h�̃����_�����O��4bpp�ōs���悤�ɕύX
 *	  2002.06.21		�X�V���K�v�ȕ������������_�����O�EBLT����悤�ɕύX
 *						�n�[�h�E�F�A�E�B���h�E�p�ʒm�֐���V��
 *	  2002.07.13		BITMAPINFO�̊m�ۃT�C�Y���C��
 *	  2002.07.20		V2.7����4096�F���[�h�ɂ���Ɨ�����o�O���C��(�΂�
 *	  2003.02.11		200���C�����[�h��400���C�����[�h�̃����_���𓝍�
 *	  2004.03.17		�^��400���C���A�_�v�^�Ή�
 *	  2008.01.20		���Ȃ��������Ƃɂ���
 *	  2010.10.03		2�{�g��\�����[�h������
 *	  2012.07.01		�O���[�����j�^���[�h�ɑΉ�
 *	  2013.02.12		���X�^�����_�����O�ɑΉ�
 +	  2013.07.13		V3�ł�4096�F���[�h�����_�����O��32bpp�ɕύX
 *	  2013.08.22		��L�ύX�����X�^�P�ʃ����_�����O���݂̂ɕύX
 */

#ifdef _WIN32

#define STRICT
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <assert.h>
#include <stdlib.h>
#include "xm7.h"
#include "subctrl.h"
#include "display.h"
#include "multipag.h"
#include "ttlpalet.h"
#include "apalet.h"
#include "mouse.h"
#include "w32.h"
#include "w32_draw.h"
#include "w32_gdi.h"
#include "w32_kbd.h"
#include "w32_sub.h"

/*
 *	�O���[�o�� ���[�N
 */
DWORD rgbTTLGDI[16];						/* �f�W�^���p���b�g */
DWORD rgbAnalogGDI[4096];					/* �A�i���O�p���b�g */
BYTE *pBitsGDI;								/* �r�b�g�f�[�^ */
BYTE GDIDrawFlag[4000];						/* 8x8 �ĕ`��̈�t���O */
#if XM7_VER == 1 && defined(L4CARD)
DWORD rgbTTLL4GDI[32];						/* �f�W�^���p���b�g (L4) */
#endif

/*
 *	�X�^�e�B�b�N ���[�N
 */
#if XM7_VER >= 3
static BYTE nMode;							/* ��ʃ��[�h */
#elif XM7_VER >= 2
static BOOL bAnalog;						/* �A�i���O���[�h�t���O */
#elif XM7_VER == 1 && defined(L4CARD)
static BOOL b400Line;						/* 400���C�����[�h�t���O */
#endif
static BYTE nNowBpp;						/* ���݂̃r�b�g�[�x */
static BOOL bMouseCursor;					/* �}�E�X�J�[�\���t���O */
static HBITMAP hBitmap;						/* �r�b�g�}�b�v �n���h�� */
static WORD nDrawTop;						/* �`��͈͏� */
static WORD nDrawBottom;					/* �`��͈͉� */
static WORD nDrawLeft;						/* �`��͈͍� */
static WORD nDrawRight;						/* �`��͈͉E */
static BOOL bPaletFlag;						/* �p���b�g�ύX�t���O */
static BOOL bClearFlag;						/* �N���A�t���O */
static BOOL bSelectCancelFlag;				/* �Z���N�g�L�����Z���t���O */
#if XM7_VER >= 3
static BOOL bWindowOpen;					/* �n�[�h�E�F�A�E�B���h�E��� */
static WORD nWindowDx1;						/* �E�B���h�E����X���W */
static WORD nWindowDy1;						/* �E�B���h�E����Y���W */
static WORD nWindowDx2;						/* �E�B���h�E�E��X���W */
static WORD nWindowDy2;						/* �E�B���h�E�E��Y���W */
#endif

/*
 *	�A�Z���u���֐��̂��߂̃v���g�^�C�v�錾
 */
#ifdef __cplusplus
extern "C" {
#endif
#if XM7_VER >= 3
extern void Render640GDI2(int first, int last, int pitch, int scale);
extern void Render640wGDI2(int first, int last, int firstx, int lastx, int pitch, int scale);
extern void Render320GDI(int first, int last);
extern void Render320wGDI(int first, int last, int firstx, int lastx);
extern void Render320GDI32bpp(int first, int last);
extern void Render320wGDI32bpp(int first, int last, int firstx, int lastx);
extern void Render256kGDI(int first, int last, int multipage);
extern void Render640cGDI(int first, int last);
extern void Render640cwGDI(int first, int last, int firstx, int lastx);
#elif XM7_VER == 2
extern void Render640GDI(int first, int last);
extern void Render320GDI(int first, int last);
extern void Render640cGDI(int first, int last);
#else
extern void Render640GDI(int first, int last);
extern void RenderL4GDI(int first, int last);
extern void Render640mGDI(int first, int last);
#endif
#ifdef __cplusplus
}
#endif

/*
 *	�v���g�^�C�v�錾
 */
static void FASTCALL SetDrawFlag(BOOL flag);

/*
 *	������
 */
void FASTCALL InitGDI(void)
{
	/* ���[�N�G���A������ */
#if XM7_VER >= 3
	nMode = SCR_200LINE;
#elif XM7_VER >= 2
	bAnalog = FALSE;
#elif XM7_VER == 1 && defined(L4CARD)
	b400Line = FALSE;
#endif
	nNowBpp = 4;
	bMouseCursor = TRUE;
	hBitmap = NULL;
	pBitsGDI = NULL;
	memset(rgbTTLGDI, 0, sizeof(rgbTTLGDI));
#if XM7_VER >= 2
	memset(rgbAnalogGDI, 0, sizeof(rgbAnalogGDI));
#elif XM7_VER == 1 && defined(L4CARD)
	memset(rgbTTLL4GDI, 0, sizeof(rgbTTLL4GDI));
#endif

	nDrawTop = 0;
	nDrawBottom = 400;
	nDrawLeft = 0;
	nDrawRight = 640;
	bPaletFlag = FALSE;
	bSelectCancelFlag = FALSE;
	SetDrawFlag(FALSE);

#if XM7_VER >= 3
	bWindowOpen = FALSE;
	nWindowDx1 = 640;
	nWindowDy1 = 400;
	nWindowDx2 = 0;
	nWindowDy2 = 0;
#endif
}

/*
 *	�N���[���A�b�v
 */
void FASTCALL CleanGDI(void)
{
	if (hBitmap) {
		DeleteObject(hBitmap);
		hBitmap = NULL;
		pBitsGDI = NULL;
	}
}

/*
 *	GDI�Z���N�g����
 */
static BOOL FASTCALL SelectSub(HWND hWnd)
{
	BITMAPINFOHEADER *pbmi;
	HDC hDC;

	ASSERT(hWnd);

	/* DIB�Z�N�V���������ɑ��݂���ꍇ�A�j������ */
	if (hBitmap) {
		DeleteObject(hBitmap);
		hBitmap = NULL;
		pBitsGDI = NULL;
	}

	/* �������m�� */
	pbmi = (BITMAPINFOHEADER*)malloc(sizeof(BITMAPINFOHEADER) +
									 sizeof(RGBQUAD) * 256);
	if (!pbmi) {
		return FALSE;
	}
	memset(pbmi, 0, sizeof(BITMAPINFOHEADER) + sizeof(RGBQUAD) * 256);

	/* �r�b�g�}�b�v���쐬 */
	pbmi->biSize = sizeof(BITMAPINFOHEADER);
	pbmi->biWidth = 640;
	pbmi->biHeight = -400;
	pbmi->biPlanes = 1;
	pbmi->biBitCount = nNowBpp;
	pbmi->biCompression = BI_RGB;

	/* DC�擾�ADIB�Z�N�V�����쐬 */
	hDC = GetDC(hWnd);
	hBitmap = CreateDIBSection(hDC, (BITMAPINFO*)pbmi, DIB_RGB_COLORS,
								(void**)&pBitsGDI, NULL, 0);
	ReleaseDC(hWnd, hDC);
	free(pbmi);
	if (!hBitmap) {
		return FALSE;
	}

	/* �S�G���A���A��x�N���A */
	if (!bRasterRendering) {
		memset(pBitsGDI, 0, (640 * 400 * nNowBpp) >> 3);
		bClearFlag = TRUE;
	}

	return TRUE;
}

/*
 *	�S�Ă̍ĕ`��t���O��ݒ�
 */
static void FASTCALL SetDrawFlag(BOOL flag)
{
	memset(GDIDrawFlag, (BYTE)flag, sizeof(GDIDrawFlag));
}

/*
 *  640x200�A�f�W�^�����[�h
 *	�Z���N�g
 */
static BOOL FASTCALL Select640(HWND hWnd)
{
	if ((!pBitsGDI) || (nNowBpp != 4)) {
		nNowBpp = 4;
		if (!SelectSub(hWnd)) {
			return FALSE;
		}
	}

	/* �S�̈斳�� */
	nDrawTop = 0;
	nDrawBottom = 400;
	nDrawLeft = 0;
	nDrawRight = 640;
	bPaletFlag = TRUE;
	SetDrawFlag(TRUE);

#if XM7_VER >= 3
	/* �f�W�^��/200���C�����[�h */
	nMode = SCR_200LINE;
#elif XM7_VER >= 2
	/* �f�W�^�����[�h */
	bAnalog = FALSE;
#elif XM7_VER == 1 && defined(L4CARD)
	/* 200���C�����[�h */
	b400Line = FALSE;
#endif

	return TRUE;
}

#if XM7_VER == 1 && defined(L4CARD)
/*
 *  640x400�A�P�F���[�h
 *	�Z���N�g
 */
static BOOL FASTCALL SelectL4(HWND hWnd)
{
	if ((!pBitsGDI) || (nNowBpp != 8)) {
		nNowBpp = 8;
		if (!SelectSub(hWnd)) {
			return FALSE;
		}
	}

	/* �S�̈斳�� */
	nDrawTop = 0;
	nDrawBottom = 400;
	nDrawLeft = 0;
	nDrawRight = 640;
	bPaletFlag = TRUE;
	SetDrawFlag(TRUE);

	/* 400���C�����[�h */
	b400Line = TRUE;

	return TRUE;
}
#endif

#if XM7_VER >= 3
/*
 *  640x400�A�f�W�^�����[�h
 *	�Z���N�g
 */
static BOOL FASTCALL Select400l(HWND hWnd)
{
	if ((!pBitsGDI) || (nNowBpp != 4)) {
		nNowBpp = 4;
		if (!SelectSub(hWnd)) {
			return FALSE;
		}
	}

	/* �S�̈斳�� */
	nDrawTop = 0;
	nDrawBottom = 400;
	nDrawLeft = 0;
	nDrawRight = 640;
	bPaletFlag = TRUE;
	SetDrawFlag(TRUE);

	/* �f�W�^��/400���C�����[�h */
	nMode = SCR_400LINE;

	return TRUE;
}
#endif

#if XM7_VER >= 2
/*
 *  320x200�A�A�i���O���[�h
 *	�Z���N�g
 */
static BOOL FASTCALL Select320(HWND hWnd)
{
#if XM7_VER == 2
	if ((!pBitsGDI) || (nNowBpp != 16)) {
		nNowBpp = 16;
		if (!SelectSub(hWnd)) {
			return FALSE;
		}
	}
#else
	if (bRasterRendering) {
		if ((!pBitsGDI) || (nNowBpp != 32)) {
			nNowBpp = 32;
			if (!SelectSub(hWnd)) {
				return FALSE;
			}
		}
	}
	else {
		if ((!pBitsGDI) || (nNowBpp != 16)) {
			nNowBpp = 16;
			if (!SelectSub(hWnd)) {
				return FALSE;
			}
		}
	}
#endif

	/* �S�̈斳�� */
	nDrawTop = 0;
	nDrawBottom = 400;
	nDrawLeft = 0;
	nDrawRight = 640;
	bPaletFlag = TRUE;
	SetDrawFlag(TRUE);

#if XM7_VER >= 3
	/* �A�i���O/200���C�����[�h */
	nMode = SCR_4096;
#else
	/* �A�i���O���[�h */
	bAnalog = TRUE;
#endif

	return TRUE;
}
#endif

#if XM7_VER >= 3
/*
 *  320x200�A26���F���[�h
 *	�Z���N�g
 */
static BOOL FASTCALL Select256k(HWND hWnd)
{
	if ((!pBitsGDI) || (nNowBpp != 32)) {
		nNowBpp = 32;
		if (!SelectSub(hWnd)) {
			return FALSE;
		}
	}

	/* �S�̈斳�� */
	nDrawTop = 0;
	nDrawBottom = 400;
	nDrawLeft = 0;
	nDrawRight = 640;
	bPaletFlag = TRUE;
	SetDrawFlag(TRUE);

	/* �A�i���O(26���F)/200���C�����[�h */
	nMode = SCR_262144;

	return TRUE;
}
#endif

/*
 *	�Z���N�g�`�F�b�N
 */
static BOOL FASTCALL SelectCheck(void)
{
	/* �����Z���N�g�L�����Z������ */
	if (bSelectCancelFlag) {
		bSelectCancelFlag = FALSE;
		return FALSE;
	}

#if XM7_VER >= 3
	/* ����Ȃ��蔲��(�H */
	if (nMode == screen_mode) {
		return TRUE;
	}
	else {
		return FALSE;
	}
#elif XM7_VER >= 2
	/* 320x200 */
	if (mode320) {
		if (bAnalog) {
			return TRUE;
		}
		else {
			return FALSE;
		}
	}

	/* 640x200 */
	if (!bAnalog) {
		return TRUE;
	}
	else {
		return FALSE;
	}
#elif defined(L4CARD)
	/* 640x400 */
	if (enable_400line && enable_400linecard) {
		if (b400Line) {
			return TRUE;
		}
		else {
			return FALSE;
		}
	}

	/* 640x200 */
	if (!b400Line) {
		return TRUE;
	}
	else {
		return FALSE;
	}
#else
	return TRUE;
#endif
}

/*
 *	�Z���N�g
 */
BOOL FASTCALL SelectGDI(HWND hWnd)
{
	ASSERT(hWnd);

	/* ���������Ȃ� */
	if (!pBitsGDI) {
#if XM7_VER >= 3
		switch (screen_mode) {
			case SCR_400LINE	:	return Select400l(hWnd);
			case SCR_262144		:	return Select256k(hWnd);
			case SCR_4096		:	return Select320(hWnd);
			case SCR_200LINE	:	return Select640(hWnd);
		}
#elif XM7_VER >= 2
		if (mode320) {
			return Select320(hWnd);
		}
		return Select640(hWnd);
#elif defined(L4CARD)
		if (enable_400line && enable_400linecard) {
			return SelectL4(hWnd);
		}
		return Select640(hWnd);
#else
		return Select640(hWnd);
#endif
	}

	/* ��v���Ă��邩�`�F�b�N */
	if (SelectCheck()) {
		return TRUE;
	}

	/* �Z���N�g */
#if XM7_VER >= 3
	switch (screen_mode) {
		case SCR_400LINE	:	return Select400l(hWnd);
		case SCR_262144		:	return Select256k(hWnd);
		case SCR_4096		:	return Select320(hWnd);
		default				:	return Select640(hWnd);
	}
#else
#if XM7_VER >= 2
	if (mode320) {
		return Select320(hWnd);
	}
#elif defined(L4CARD)
	if (enable_400line && enable_400linecard) {
		return SelectL4(hWnd);
	}
#endif
	return Select640(hWnd);
#endif
}

/*-[ �`�� ]-----------------------------------------------------------------*/

#if XM7_VER <= 2 && defined(FMTV151)
/*
 *	V2���� (4bpp)
 */
static void FASTCALL DrawV2_4bpp(void)
{
	BYTE	x, y;
	int		xx, yy;
	WORD	col;
	BYTE	*pbits;

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

	pbits = (BYTE *)(pBitsGDI + V2YPoint * 640 + (V2XPoint >> 1));

	for (y = 0; y < V2YSize; y ++) {
		for (x = 0; x < V2XSize; x ++) {
			if (nV2data[y * V2XSize + x]) {
				col = (WORD)(10 - nV2data[y * V2XSize + x]);
				for (yy = y * V2YPxSz; yy < (y + 1) * V2YPxSz; yy ++) {
					for (xx = x * V2XPxSz; xx < (x + 1) * V2XPxSz; xx ++) {
						if (xx & 1) {
							pbits[yy * 640 + (xx >> 1)] &= (BYTE)0xf0;
							pbits[yy * 640 + (xx >> 1)] |= col;
							if (bPseudo400Line) {
								pbits[yy * 640+320+(xx >> 1)] &= (BYTE)0xf0;
								pbits[yy * 640+320+(xx >> 1)] |= col;
							}
						}
						else {
							pbits[yy * 640 + (xx >> 1)] &= (BYTE)0x0f;
							pbits[yy * 640 + (xx >> 1)] |= (BYTE)(col << 4);
							if (bPseudo400Line) {
								pbits[yy*640+320+(xx>>1)] &= (BYTE)0x0f;
								pbits[yy*640+320+(xx>>1)] |= (BYTE)(col << 4);
							}
						}
					}
				}
			}
		}
	}
}

/*
 *	V2���� (16bpp)
 */
static void FASTCALL DrawV2_16bpp(void)
{
	BYTE	x, y;
	int		xx, yy;
	WORD	col;
	WORD	*pbits;

	/* �p���b�g�e�[�u�� */
	static const WORD V2rgbTable[] = {
		0x0000 | 0x0000 | 0x0000,
		0x0000 | 0x03e0 | 0x0000,
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

	pbits = (WORD *)(pBitsGDI + V2YPoint * 2560 + V2XPoint * 2);

	for (y = 0; y < V2YSize; y ++) {
		for (x = 0; x < V2XSize; x ++) {
			if (nV2data[y * V2XSize + x]) {
				col = V2rgbTable[nV2data[y * V2XSize + x]];
				for (yy = y * V2YPxSz; yy < (y + 1) * V2YPxSz; yy ++) {
					for (xx = x * V2XPxSz; xx < (x + 1) * V2XPxSz; xx ++) {
						pbits[yy * 1280 + xx] = col;
					}
				}
			}
		}
	}
}
#endif

/*
 *	�I�[���N���A
 */
static void FASTCALL AllClear(void)
{
	/* ���ׂăN���A */
	memset(pBitsGDI, 0, (640 * 400 * nNowBpp) >> 3);

	/* �S�̈�������_�����O�ΏۂƂ��� */
	nDrawTop = 0;
	nDrawBottom = 400;
	nDrawLeft = 0;
	nDrawRight = 640;
	SetDrawFlag(TRUE);

	bClearFlag = FALSE;
}

/*
 *	�t���X�L����
 */
static void FASTCALL RenderFullScan(void)
{
	BYTE *p;
	BYTE *q;
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

	/* �|�C���^������ */
	p = &pBitsGDI[(top * 640 * nNowBpp) >> 3];
	q = p + ((640 * nNowBpp) >> 3);

	/* ���[�v */
	for (u=top; u<bottom; u += (WORD)2) {
		memcpy(q, p, (640 * nNowBpp) >> 3);
		p += ((640 * 2 * nNowBpp) >> 3);
		q += ((640 * 2 * nNowBpp) >> 3);
	}
}

/*
 *	����C���ݒ� (�f�W�^�����[�h)
 */
static void FASTCALL RenderSetOddLineDigital(void)
{
	BYTE *p;
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

	/* �|�C���^������ */
	p = &pBitsGDI[((top + 1) * 640 * nNowBpp) >> 3];

	/* ���[�v */
	for (u=top; u<bottom; u += (WORD)2) {
		memset(p, 0x88, (640 * nNowBpp) >> 3);
		p += ((640 * 2 * nNowBpp) >> 3);
	}
}

/*
 *	����C���ݒ� (�A�i���O���[�h)
 */
#if XM7_VER >= 2
static void FASTCALL RenderSetOddLineAnalog(void)
{
	BYTE *p;
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

	/* �|�C���^������ */
	p = &pBitsGDI[((top + 1) * 640 * nNowBpp) >> 3];

	/* ���[�v */
	for (u=top; u<bottom; u += (WORD)2) {
		memset(p, 0x00, (640 * nNowBpp) >> 3);
		p += ((640 * 2 * nNowBpp) >> 3);
	}
}
#endif

/*
 *	640x200/400�A�f�W�^�����[�h
 *	�p���b�g�ݒ�
 */
static void FASTCALL Palet640(void)
{
	int i;
	int vpage;
	BYTE col;

	/* �p���b�g�e�[�u�� */
	static DWORD rgbTable[] = {
		0x00000000,
		0x000000ff,
		0x00ff0000,
		0x00ff00ff,
		0x0000ff00,
		0x0000ffff,
		0x00ffff00,
		0x00ffffff,
#if XM7_VER == 1
		0x00000000,
		0x00001c00,
		0x00004c00,
		0x00006800,
		0x00009600,
		0x0000b200,
		0x0000e200,
		0x0000ff00,
#endif
	};

	/* �}���`�y�[�W���A�\���v���[�����𓾂� */
	vpage = (~(multi_page >> 4)) & 0x07;

	/* 640x200/400�A�f�W�^���p��	�b�g */
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
			if (bPseudo400Line) {
				if (bGreenMonitor) {
					if (col & 2) {
						rgbTTLGDI[i + 8] = rgbTable[15];
					}
					else {
						rgbTTLGDI[i + 8] = rgbTable[8];
					}
					if (col & 4) {
						rgbTTLGDI[i] = rgbTable[15];
					}
					else {
						rgbTTLGDI[i] = rgbTable[8];
					}
				}
				else {
					if (col & 2) {
						rgbTTLGDI[i + 8] = rgbTable[7];
					}
					else {
						rgbTTLGDI[i + 8] = rgbTable[0];
					}
					if (col & 4) {
						rgbTTLGDI[i] = rgbTable[7];
					}
					else {
						rgbTTLGDI[i] = rgbTable[0];
					}
				}
			}
			else {
				if (bGreenMonitor) {
					rgbTTLGDI[i] = rgbTable[col + 8];
				}
				else {
					rgbTTLGDI[i] = rgbTable[col];
				}
			}
#else
			rgbTTLGDI[i] = rgbTable[col];
#endif
		}
		else {
			/* CRT OFF */
			rgbTTLGDI[i] = rgbTable[0];
		}
	}

	/* FMTV-151 V2�`�����l���R�[���p */
#if XM7_VER == 2 && defined(FMTV151)
	rgbTTLGDI[8] = rgbTable[0];
	rgbTTLGDI[9] = rgbTable[4];
#endif
}

#if XM7_VER == 1 && defined(L4CARD)
/*
 *	640x400�A�P�F���[�h
 *	�p���b�g�ݒ�
 */
static void FASTCALL PaletL4(void)
{
	int i;

	/* �p���b�g�e�[�u��(16�F) */
	static const DWORD rgbTable16[] = {
		0x00000000,
		0x000000bb,
		0x00bb0000,
		0x00bb00bb,
		0x0000bb00,
		0x0000bbbb,
		0x00bbbb00,
		0x00bbbbbb,
		0x00444444,
		0x004444ff,
		0x00ff4444,
		0x00ff44ff,
		0x0044ff44,
		0x0044ffff,
		0x00ffff44,
		0x00ffffff,
		0x00000000,
		0x00003300,
		0x00006500,
		0x00007100,
		0x0000a900,
		0x0000af00,
		0x0000bd00,
		0x0000c200,
		0x00004a00,
		0x00006700,
		0x00009600,
		0x0000a200,
		0x0000e200,
		0x0000e900,
		0x0000f900,
		0x0000ff00,
	};

	/* 640x400�A�e�L�X�g�p���b�g */
	for (i=0; i<16; i++) {
		if (crt_flag) {
			/* CRT ON */
			if (bGreenMonitor) {
				rgbTTLL4GDI[i + 16] = rgbTable16[i + 16];
			}
			else {
				rgbTTLL4GDI[i + 16] = rgbTable16[i];
			}
		}
		else {
			/* CRT OFF */
			rgbTTLL4GDI[i + 16] = rgbTable16[0];
		}
	}

	/* 640x400�A�O���t�B�b�N�p���b�g */
	for (i=0; i<3; i++) {
		if (crt_flag) {
			if (bGreenMonitor) {
				if ((i == 0) || (multi_page & (0x08 << i))) {
					rgbTTLL4GDI[i] = rgbTable16[(ttl_palet[0] & 0x0f) + 16];
				}
				else {
					rgbTTLL4GDI[i] = rgbTable16[(ttl_palet[1] & 0x0f) + 16];
				}
			}
			else {
				if ((i == 0) || (multi_page & (0x08 << i))) {
					rgbTTLL4GDI[i] = rgbTable16[ttl_palet[0] & 0x0f];
				}
				else {
					rgbTTLL4GDI[i] = rgbTable16[ttl_palet[1] & 0x0f];
				}
			}
		}
		else {
			rgbTTLL4GDI[i] = rgbTable16[0];
		}
	}
}
#endif

/*
 *	640x200�A�f�W�^�����[�h
 *	�`��
 */
static void FASTCALL Draw640(HWND hWnd, HDC hDC)
{
	HDC hMemDC;
	HBITMAP hDefBitmap;
#if XM7_VER >= 3
	WORD wdtop, wdbtm;
#endif

	ASSERT(hWnd);
	ASSERT(hDC);

	/* �p���b�g�ݒ� */
	if (bPaletFlag) {
		Palet640();
	}

	/* �N���A���� */
	if (bClearFlag) {
		AllClear();
	}

	/* �����_�����O */
	if ((nDrawTop < nDrawBottom) && (nDrawLeft < nDrawRight)) {
#if XM7_VER >= 3
		/* �E�B���h�E�I�[�v���� */
		if (window_open) {
			/* �E�B���h�E�O �㑤�̕`�� */
			if ((nDrawTop >> 1) < window_dy1) {
				if (bPseudo400Line) {
					Render640cGDI(nDrawTop >> 1, window_dy1);
				}
				else {
					Render640GDI2(nDrawTop >> 1, window_dy1, 320, 4);
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
					Render640cwGDI(wdtop, wdbtm, window_dx1, window_dx2);
				}
				else {
					Render640wGDI2(wdtop, wdbtm, window_dx1, window_dx2,320,4);
				}
			}

			/* �E�B���h�E�O �����̕`�� */
			if ((nDrawBottom >> 1) > window_dy2) {
				if (bPseudo400Line) {
					Render640cGDI(window_dy2, nDrawBottom >> 1);
				}
				else {
					Render640GDI2(window_dy2, nDrawBottom >> 1, 320, 4);
				}
			}
		}
		else {
			if (bPseudo400Line) {
				Render640cGDI(nDrawTop >> 1, nDrawBottom >> 1);
			}
			else {
				Render640GDI2(nDrawTop >> 1, nDrawBottom >> 1, 320, 4);
			}
		}
#elif XM7_VER == 2
		if (bPseudo400Line) {
			Render640cGDI(nDrawTop >> 1, nDrawBottom >> 1);
		}
		else {
			Render640GDI(nDrawTop >> 1, nDrawBottom >> 1);
		}
#if defined(FMTV151)
		DrawV2_4bpp();
#endif	/* FMTV-151 */
#else
		if (bPseudo400Line) {
			Render640mGDI(nDrawTop >> 1, nDrawBottom >> 1);
		}
		else {
			Render640GDI(nDrawTop >> 1, nDrawBottom >> 1);
		}
#endif	/* XM7_VER >= 3 */

		if (!bPseudo400Line) {
			if (bFullScan) {
				RenderFullScan();
			}
			else {
				RenderSetOddLineDigital();
			}
		}
	}
	else {
		/* �p���b�g���ύX����Ă��Ȃ��ꍇ�͉������Ȃ� */
		if (!bPaletFlag) {
			return;
		}
	}

	/* �p���b�g���ύX����Ă���ꍇ�͑S�̈���ĕ`�� */
	if (bPaletFlag) {
		nDrawTop = 0;
		nDrawBottom = 400;
		nDrawLeft = 0;
		nDrawRight = 640;
		SetDrawFlag(TRUE);
	}

	/* ������DC�쐬�A�I�u�W�F�N�g�I�� */
	hMemDC = CreateCompatibleDC(hDC);
	ASSERT(hMemDC);
	hDefBitmap = (HBITMAP)SelectObject(hMemDC, hBitmap);

	/* �f�X�N�g�b�v�̏󋵂�����ł́A���ۂ����蓾��l */
	if (hDefBitmap) {
		SetDIBColorTable(hMemDC, 0, 16, (RGBQUAD *)rgbTTLGDI);
		if (bDoubleSize) {
			StretchBlt(hDC, nDrawLeft * 2, nDrawTop * 2,
				(nDrawRight - nDrawLeft) * 2, (nDrawBottom - nDrawTop) * 2,
				hMemDC, nDrawLeft, nDrawTop,
				(nDrawRight - nDrawLeft), (nDrawBottom - nDrawTop), SRCCOPY);
		}
		else {
			BitBlt(hDC, nDrawLeft, nDrawTop,
				(nDrawRight - nDrawLeft), (nDrawBottom - nDrawTop),
				hMemDC, nDrawLeft, nDrawTop, SRCCOPY);
		}
		SelectObject(hMemDC, hDefBitmap);

		/* ����ɔ����A���[�N���Z�b�g */
		nDrawTop = 400;
		nDrawBottom = 0;
		nDrawLeft = 640;
		nDrawRight = 0;
		bPaletFlag = FALSE;
		SetDrawFlag(FALSE);
	}
	else {
		/* �ĕ`����N���� */
		InvalidateRect(hWnd, NULL, FALSE);
	}

	/* ������DC�폜 */
	DeleteDC(hMemDC);
}

#if XM7_VER >= 3
/*
 *	640x400�A�f�W�^�����[�h
 *	�`��
 */
static void FASTCALL Draw400l(HWND hWnd, HDC hDC)
{
	HDC hMemDC;
	HBITMAP hDefBitmap;
	WORD wdtop, wdbtm;

	ASSERT(hWnd);
	ASSERT(hDC);

	/* �p���b�g�ݒ� */
	if (bPaletFlag) {
		Palet640();
	}

	/* �N���A���� */
	if (bClearFlag) {
		AllClear();
	}

	/* �����_�����O */
	if ((nDrawTop < nDrawBottom) && (nDrawLeft < nDrawRight)) {
		/* �E�B���h�E�I�[�v���� */
		if (window_open) {
			/* �E�B���h�E�O �㑤�̕`�� */
			if (nDrawTop < window_dy1) {
				Render640GDI2(nDrawTop, window_dy1, 0, 2);
			}

			/* �E�B���h�E���̕`�� */
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
				Render640wGDI2(wdtop, wdbtm, window_dx1, window_dx2, 0, 2);
			}

			/* �E�B���h�E�O �����̕`�� */
			if (nDrawBottom > window_dy2) {
				Render640GDI2(window_dy2, nDrawBottom, 0, 2);
			}
		}
		else {
			Render640GDI2(nDrawTop, nDrawBottom, 0, 2);
		}
	}
	else {
		/* �p���b�g���ύX����Ă��Ȃ��ꍇ�͉������Ȃ� */
		if (!bPaletFlag) {
			return;
		}
	}

	/* �p���b�g���ύX����Ă���ꍇ�͑S�̈���ĕ`�� */
	if (bPaletFlag) {
		nDrawTop = 0;
		nDrawBottom = 400;
		nDrawLeft = 0;
		nDrawRight = 640;
		SetDrawFlag(TRUE);
	}

	/* ������DC�쐬�A�I�u�W�F�N�g�I�� */
	hMemDC = CreateCompatibleDC(hDC);
	ASSERT(hMemDC);
	hDefBitmap = (HBITMAP)SelectObject(hMemDC, hBitmap);

	/* �f�X�N�g�b�v�̏󋵂�����ł́A���ۂ����蓾��l */
	if (hDefBitmap) {
		SetDIBColorTable(hMemDC, 0, 16, (RGBQUAD *)rgbTTLGDI);
		if (bDoubleSize) {
			StretchBlt(hDC, nDrawLeft * 2, nDrawTop * 2,
				(nDrawRight - nDrawLeft) * 2, (nDrawBottom - nDrawTop) * 2,
				hMemDC, nDrawLeft, nDrawTop,
				(nDrawRight - nDrawLeft), (nDrawBottom - nDrawTop), SRCCOPY);
		}
		else {
			BitBlt(hDC, nDrawLeft, nDrawTop,
				(nDrawRight - nDrawLeft), (nDrawBottom - nDrawTop),
				hMemDC, nDrawLeft, nDrawTop, SRCCOPY);
		}
		SelectObject(hMemDC, hDefBitmap);

		/* ����ɔ����A���[�N���Z�b�g */
		nDrawTop = 400;
		nDrawBottom = 0;
		nDrawLeft = 640;
		nDrawRight = 0;
		bPaletFlag = FALSE;
		SetDrawFlag(FALSE);
	}
	else {
		/* �ĕ`����N���� */
		InvalidateRect(hWnd, NULL, FALSE);
	}

	/* ������DC�폜 */
	DeleteDC(hMemDC);
}
#endif

#if XM7_VER == 1 && defined(L4CARD)
/*
 *	�e�L�X�g���
 *	�����_�����O
 */
static void FASTCALL RenderTextGDI(int first, int last, int left, int right)
{
	DWORD	tmp;
	int		x, y, x2;
	WORD	addr;
	BYTE	cursor_start, cursor_end, cursor_type;
	BYTE	raster, lines, maxx;
	BYTE	chr, atr, chr_dat;
	BYTE	line, line_old;
	BYTE	col;
	BYTE	*p;
	int		xsize;

	/* �\��OFF�̏ꍇ�A�������Ȃ� */
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

	/* 1�����������X�T�C�Y */
	if (width40_flag) {
		xsize = 16;
	}
	else {
		xsize = 8;
	}

	for (x = (left / xsize); x < (right / xsize); x++) {
		line_old = 255;
		for (y = first; y < last; y++) {
			p = (BYTE *)pBitsGDI + y * 640;
			raster = (BYTE)(y % lines);
			line = (BYTE)(y / lines);
			if (line != line_old) {
				addr = (WORD)((text_start_addr + (line * maxx + x) * 2) & 0x0ffe);
				chr = tvram_c[addr + 0];
				atr = tvram_c[addr + 1];

				/* �A�g���r���[�g����3�r�b�g����`��F��ݒ� */
				col = (BYTE)((atr & 0x07) | 0x10);

				/* �C���e���V�e�B�A�g���r���[�g���� */
				if (atr & 0x20) {
					col |= (BYTE)8;
				}
			}

			/* �t�H���g�f�[�^�擾(�u�����N�A�g���r���[�g�������܂�) */
			if ((!(atr & 0x10) || text_blink) && (raster < 16)) {
				chr_dat = subcg_l4[(WORD)(chr * 16) + raster];
			}
			else {
				chr_dat = 0x00;
			}

			/* ���o�[�X�A�g���r���[�g���� */
			if (atr & 0x08) {
				chr_dat ^= (BYTE)0xFF;
			}

			/* �J�[�\������ */
			if ((addr == cursor_addr) && (cursor_type != 1) &&
				(cursor_blink || (cursor_type == 0))) {
				if ((raster >= cursor_start) && (raster <= cursor_end)) {
					chr_dat ^= (BYTE)0xFF;
				}
			}

			/* �`�揈�� */
			if (chr_dat) {
				for (x2 = 0; x2 < 8; x2++) {
					tmp = (DWORD)((x << 3) + x2);
					if (chr_dat & (1 << (7 ^ x2))) {
						if (width40_flag) {
							p[tmp * 2 + 0] = col;
							p[tmp * 2 + 1] = col;
						}
						else {
							p[tmp] = col;
						}
					}
				}
			}

			/* ����`�悵���s�ʒu��ۑ� */
			line_old = line;
		}
	}
}

/*
 *	640x400�A�P�F���[�h
 *	�`��
 */
static void FASTCALL DrawL4(HWND hWnd, HDC hDC)
{
	HDC hMemDC;
	HBITMAP hDefBitmap;

	ASSERT(hWnd);
	ASSERT(hDC);

	/* �p���b�g�ݒ� */
	if (bPaletFlag) {
		PaletL4();
	}

	/* �N���A���� */
	if (bClearFlag) {
		AllClear();
	}

	/* �����_�����O */
	if ((nDrawTop < nDrawBottom) && (nDrawLeft < nDrawRight)) {
		RenderL4GDI(nDrawTop, nDrawBottom);
		RenderTextGDI(nDrawTop, nDrawBottom, nDrawLeft, nDrawRight);
	}
	else {
		/* �p���b�g���ύX����Ă��Ȃ��ꍇ�͉������Ȃ� */
		if (!bPaletFlag) {
			return;
		}
	}

	/* �p���b�g���ύX����Ă���ꍇ�͑S�̈���ĕ`�� */
	if (bPaletFlag) {
		nDrawTop = 0;
		nDrawBottom = 400;
		nDrawLeft = 0;
		nDrawRight = 640;
		SetDrawFlag(TRUE);
	}

	/* ������DC�쐬�A�I�u�W�F�N�g�I�� */
	hMemDC = CreateCompatibleDC(hDC);
	ASSERT(hMemDC);
	hDefBitmap = (HBITMAP)SelectObject(hMemDC, hBitmap);

	/* �f�X�N�g�b�v�̏󋵂�����ł́A���ۂ����蓾��l */
	if (hDefBitmap) {
		SetDIBColorTable(hMemDC, 0, 32, (RGBQUAD *)rgbTTLL4GDI);
		if (bDoubleSize) {
			StretchBlt(hDC, nDrawLeft * 2, nDrawTop * 2,
				(nDrawRight - nDrawLeft) * 2, (nDrawBottom - nDrawTop) * 2,
				hMemDC, nDrawLeft, nDrawTop,
				(nDrawRight - nDrawLeft), (nDrawBottom - nDrawTop), SRCCOPY);
		}
		else {
			BitBlt(hDC, nDrawLeft, nDrawTop,
				(nDrawRight - nDrawLeft), (nDrawBottom - nDrawTop),
				hMemDC, nDrawLeft, nDrawTop, SRCCOPY);
		}
		SelectObject(hMemDC, hDefBitmap);

		/* ����ɔ����A���[�N���Z�b�g */
		nDrawTop = 400;
		nDrawBottom = 0;
		nDrawLeft = 640;
		nDrawRight = 0;
		bPaletFlag = FALSE;
		SetDrawFlag(FALSE);
	}
	else {
		/* �ĕ`����N���� */
		InvalidateRect(hWnd, NULL, FALSE);
	}

	/* ������DC�폜 */
	DeleteDC(hMemDC);
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
	DWORD color;
	DWORD r, g, b;
	int amask;

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
		color = 0;
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
			r = apalet_r[j];
			g = apalet_g[j];
			b = apalet_b[j];
		}
		else {
			r = 0;
			g = 0;
			b = 0;
		}

#if XM7_VER >= 3
		if (bRasterRendering) {
			/* R */
			color |= (WORD)r * 0x11;
			color <<= 8;

			/* G */
			color |= (WORD)g * 0x11;
			color <<= 8;

			/* B */
			color |= (WORD)b * 0x11;

			/* �Z�b�g */
			rgbAnalogGDI[i] = color;

			continue;
		}
#endif

		/* R */
		r <<= 1;
		if (r > 0) {
			r |= 0x01;
		}
		color |= (WORD)r;
		color <<= 5;

		/* G */
		g <<= 1;
		if (g > 0) {
			g |= 0x01;
		}
		color |= (WORD)g;
		color <<= 5;

		/* B */
		b <<= 1;
		if (b > 0) {
			b |= 0x01;
		}
		color |= (WORD)b;

		/* �Z�b�g */
		rgbAnalogGDI[i] = (DWORD)((color << 16) | color);
	}
}

/*
 *	320x200�A�A�i���O���[�h
 *	�`��
 */
static void FASTCALL Draw320(HWND hWnd, HDC hDC)
{
	HDC hMemDC;
	HBITMAP hDefBitmap;
#if XM7_VER >= 3
	WORD wdtop, wdbtm;
#endif

	ASSERT(hWnd);
	ASSERT(hDC);

	/* �p���b�g�ݒ� */
	if (bPaletFlag) {
		Palet320();
	}

	/* �N���A���� */
	if (bClearFlag) {
		AllClear();
	}

	/* �����_�����O */
	if (nDrawTop >= nDrawBottom) {
		return;
	}

#if XM7_VER >= 3
	/* �E�B���h�E�I�[�v���� */
	if (window_open) {
		/* �E�B���h�E�O �㑤�̕`�� */
		if ((nDrawTop >> 1) < window_dy1) {
			Render320GDI(nDrawTop >> 1, window_dy1);
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
			Render320wGDI(wdtop, wdbtm, window_dx1, window_dx2);
		}

		/* �E�B���h�E�O �����̕`�� */
		if ((nDrawBottom >> 1) > window_dy2) {
			Render320GDI(window_dy2, nDrawBottom >> 1);
		}
	}
	else {
		Render320GDI(nDrawTop >> 1, nDrawBottom >> 1);
	}
#else
	Render320GDI(nDrawTop >> 1, nDrawBottom >> 1);
#if defined(FMTV151)
	DrawV2_16bpp();
#endif
#endif

	if (bFullScan) {
		RenderFullScan();
	}

	/* ������DC�쐬�A�I�u�W�F�N�g�I�� */
	hMemDC = CreateCompatibleDC(hDC);
	ASSERT(hMemDC);
	hDefBitmap = (HBITMAP)SelectObject(hMemDC, hBitmap);

	/* �f�X�N�g�b�v�̏󋵂�����ł́A���ۂ����蓾��l */
	if (hDefBitmap) {
		if (bDoubleSize) {
			StretchBlt(hDC, nDrawLeft * 2, nDrawTop * 2,
				(nDrawRight - nDrawLeft) * 2, (nDrawBottom - nDrawTop) * 2,
				hMemDC, nDrawLeft, nDrawTop,
				(nDrawRight - nDrawLeft), (nDrawBottom - nDrawTop), SRCCOPY);
		}
		else {
			BitBlt(hDC, nDrawLeft, nDrawTop,
				(nDrawRight - nDrawLeft), (nDrawBottom - nDrawTop),
				hMemDC, nDrawLeft, nDrawTop, SRCCOPY);
		}
		SelectObject(hMemDC, hDefBitmap);

		/* ����ɔ����A���[�N���Z�b�g */
		nDrawTop = 400;
		nDrawBottom = 0;
		nDrawLeft = 640;
		nDrawRight = 0;
		bPaletFlag = FALSE;
		SetDrawFlag(FALSE);
	}
	else {
		/* �ĕ`����N���� */
		InvalidateRect(hWnd, NULL, FALSE);
	}

	/* ������DC��� */
	DeleteDC(hMemDC);
}
#endif

#if XM7_VER >= 3
/*
 *	320x200�A26���F���[�h
 *	�`��
 */
static void FASTCALL Draw256k(HWND hWnd, HDC hDC)
{
	HDC hMemDC;
	HBITMAP hDefBitmap;

	ASSERT(hWnd);
	ASSERT(hDC);

	/* �N���A���� */
	if (bClearFlag) {
		AllClear();
	}

	/* �����_�����O */
	if (nDrawTop >= nDrawBottom) {
		return;
	}
	if (crt_flag) {
		Render256kGDI(nDrawTop >> 1, nDrawBottom >> 1, multi_page);
	}
	else {
		Render256kGDI(nDrawTop >> 1, nDrawBottom >> 1, 0xff);
	}
	if (bFullScan) {
		RenderFullScan();
	}

	/* ������DC�쐬�A�I�u�W�F�N�g�I�� */
	hMemDC = CreateCompatibleDC(hDC);
	ASSERT(hMemDC);
	hDefBitmap = (HBITMAP)SelectObject(hMemDC, hBitmap);

	/* �f�X�N�g�b�v�̏󋵂�����ł́A���ۂ����蓾��l */
	if (hDefBitmap) {
		if (bDoubleSize) {
			StretchBlt(hDC, nDrawLeft * 2, nDrawTop * 2,
				(nDrawRight - nDrawLeft) * 2, (nDrawBottom - nDrawTop) * 2,
				hMemDC, nDrawLeft, nDrawTop,
				(nDrawRight - nDrawLeft), (nDrawBottom - nDrawTop), SRCCOPY);
		}
		else {
			BitBlt(hDC, nDrawLeft, nDrawTop,
				(nDrawRight - nDrawLeft), (nDrawBottom - nDrawTop),
				hMemDC, nDrawLeft, nDrawTop, SRCCOPY);
		}
		SelectObject(hMemDC, hDefBitmap);

		/* ����ɔ����A���[�N���Z�b�g */
		nDrawTop = 400;
		nDrawBottom = 0;
		nDrawLeft = 640;
		nDrawRight = 0;
		bPaletFlag = FALSE;
		SetDrawFlag(FALSE);
	}
	else {
		/* �ĕ`����N���� */
		InvalidateRect(hWnd, NULL, FALSE);
	}

	/* ������DC��� */
	DeleteDC(hMemDC);
}
#endif

/*
 *	�`��
 */
void FASTCALL DrawGDI(HWND hWnd, HDC hDC)
{
	ASSERT(hWnd);
	ASSERT(hDC);

	/* 640-320 �����؂�ւ� */
	SelectGDI(hWnd);

#if XM7_VER >= 3
	/* �����ꂩ���g���ĕ`�� */
	switch (nMode) {
		case SCR_400LINE	:	Draw400l(hWnd, hDC);
								return;
		case SCR_262144		:	Draw256k(hWnd, hDC);
								return;
		case SCR_4096		:	Draw320(hWnd, hDC);
								return;
		case SCR_200LINE	:	Draw640(hWnd, hDC);
								return;
	}
#elif XM7_VER >= 2
	/* �ǂ��炩���g���ĕ`�� */
	if (bAnalog) {
		Draw320(hWnd, hDC);
	}
	else {
		Draw640(hWnd, hDC);
	}
#elif defined(L4CARD)
	/* �ǂ��炩���g���ĕ`�� */
	if (b400Line) {
		DrawL4(hWnd, hDC);
	}
	else {
		Draw640(hWnd, hDC);
	}
#else
	Draw640(hWnd, hDC);
#endif
}

/*
 *	640x200�A�f�W�^�����[�h
 *	���X�^�`��
 */
static void FASTCALL DrawRaster640(int nRaster)
{
	/* �\���͈͊O�̏ꍇ�͉������Ȃ� */
	if (nRaster >= 200) {
		return;
	}

	/* �����_�����O */
#if XM7_VER >= 3
	/* �E�B���h�E�I�[�v���� */
	if (window_open) {
		/* �E�B���h�E�O�̕`�� */
		if ((nRaster < window_dy1) || (nRaster > window_dy2)) {
			if (bPseudo400Line) {
				Render640cGDI(nRaster, nRaster + 1);
			}
			else {
				Render640GDI2(nRaster, nRaster + 1, 320, 4);
			}
		}
		else {
			if (bPseudo400Line) {
				Render640cwGDI(nRaster, nRaster + 1, window_dx1, window_dx2);
			}
			else {
				Render640wGDI2(nRaster,nRaster+1,window_dx1,window_dx2,320,4);
			}
		}
	}
	else {
		if (bPseudo400Line) {
			Render640cGDI(nRaster, nRaster + 1);
		}
		else {
			Render640GDI2(nRaster, nRaster + 1, 320, 4);
		}
	}
#elif XM7_VER == 2
	if (bPseudo400Line) {
		Render640cGDI(nRaster, nRaster + 1);
	}
	else {
		Render640GDI(nRaster, nRaster + 1);
	}
#else
	if (bPseudo400Line) {
		Render640mGDI(nRaster, nRaster + 1);
	}
	else {
		Render640GDI(nRaster, nRaster + 1);
	}
#endif	/* XM7_VER >= 3 */
}

#if XM7_VER >= 3
/*
 *	640x400�A�f�W�^�����[�h
 *	���X�^�`��
 */
static void FASTCALL DrawRaster400l(int nRaster)
{
	/* �\���͈͊O�̏ꍇ�͉������Ȃ� */
	if (nRaster >= 400) {
		return;
	}

	/* �����_�����O */
	/* �E�B���h�E�I�[�v���� */
	if (window_open) {
		/* �E�B���h�E�O�̕`�� */
		if ((nRaster < window_dy1) || (nRaster > window_dy2)) {
			Render640GDI2(nRaster, nRaster + 1, 0, 2);
		}
		else {
			Render640wGDI2(nRaster, nRaster + 1, window_dx1, window_dx2, 0, 2);
		}
	}
	else {
		Render640GDI2(nRaster, nRaster + 1, 0, 2);
	}
}
#endif

#if XM7_VER == 1 && defined(L4CARD)
/*
 *	�e�L�X�g���
 *	���X�^�`��
 */
static void FASTCALL RenderTextRasterGDI(int nRaster, int left, int right)
{
	DWORD	tmp;
	int		x, x2;
	WORD	addr;
	BYTE	cursor_start, cursor_end, cursor_type;
	BYTE	raster, lines, maxx;
	BYTE	chr, atr, chr_dat;
	BYTE	line;
	BYTE	col;
	BYTE	*p;
	int		xsize;

	/* �\��OFF�̏ꍇ�A�������Ȃ� */
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

	/* 1�����������X�T�C�Y */
	if (width40_flag) {
		xsize = 16;
	}
	else {
		xsize = 8;
	}

	for (x = (left / xsize); x < (right / xsize); x++) {
		p = (BYTE *)pBitsGDI + nRaster * 640;
		raster = (BYTE)(nRaster % lines);
		line = (BYTE)(nRaster / lines);
		addr = (WORD)((text_start_addr + (line * maxx + x) * 2) & 0x0ffe);
		chr = tvram_c[addr + 0];
		atr = tvram_c[addr + 1];

		/* �A�g���r���[�g����3�r�b�g����`��F��ݒ� */
		col = (BYTE)((atr & 0x07) | 0x10);

		/* �C���e���V�e�B�A�g���r���[�g���� */
		if (atr & 0x20) {
			col |= (BYTE)8;
		}

		/* �t�H���g�f�[�^�擾(�u�����N�A�g���r���[�g�������܂�) */
		if ((!(atr & 0x10) || text_blink) && (raster < 16)) {
			chr_dat = subcg_l4[(WORD)(chr * 16) + (nRaster % lines)];
		}
		else {
			chr_dat = 0x00;
		}

		/* ���o�[�X�A�g���r���[�g���� */
		if (atr & 0x08) {
			chr_dat ^= (BYTE)0xFF;
		}

		/* �J�[�\������ */
		if ((addr == cursor_addr) && (cursor_type != 1) &&
			(cursor_blink || (cursor_type == 0))) {
			if ((raster >= cursor_start) && (raster <= cursor_end)) {
				chr_dat ^= (BYTE)0xFF;
			}
		}

		/* �`�揈�� */
		if (chr_dat) {
			for (x2 = 0; x2 < 8; x2++) {
				tmp = (DWORD)((x << 3) + x2);
				if (chr_dat & (1 << (7 ^ x2))) {
					if (width40_flag) {
						p[tmp * 2 + 0] = col;
						p[tmp * 2 + 1] = col;
					}
					else {
						p[tmp] = col;
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
	/* �\���͈͊O�̏ꍇ�͉������Ȃ� */
	if (nRaster >= 400) {
		return;
	}

	/* �����_�����O */
	RenderL4GDI(nRaster, nRaster + 1);
	RenderTextRasterGDI(nRaster, 0, 640);
}
#endif

#if XM7_VER >= 2
/*
 *	320x200�A�A�i���O���[�h
 *	���X�^�`��
 */
static void FASTCALL DrawRaster320(int nRaster)
{
	/* �\���͈͊O�̏ꍇ�͉������Ȃ� */
	if (nRaster >= 200) {
		return;
	}

	/* �p���b�g�ݒ� */
	Palet320();

	/* �����_�����O */
	/* �E�B���h�E�I�[�v���� */
#if XM7_VER >= 3
	if (window_open) {
		/* �E�B���h�E�O�̕`�� */
		if ((nRaster < window_dy1) || (nRaster > window_dy2)) {
			Render320GDI32bpp(nRaster, nRaster + 1);
		}
		else {
			Render320wGDI32bpp(nRaster, nRaster + 1, window_dx1, window_dx2);
		}
	}
	else {
		Render320GDI32bpp(nRaster, nRaster + 1);
	}
#else
	Render320GDI(nRaster, nRaster + 1);
#endif
}
#endif

#if XM7_VER >= 3
/*
 *	320x200�A26���F���[�h
 *	���X�^�`��
 */
static void FASTCALL DrawRaster256k(int nRaster)
{
	/* �\���͈͊O�̏ꍇ�͉������Ȃ� */
	if (nRaster >= 200) {
		return;
	}

	/* �����_�����O */
	if (crt_flag) {
		Render256kGDI(nRaster, nRaster + 1, multi_page);
	}
	else {
		Render256kGDI(nRaster, nRaster + 1, 0xff);
	}
}
#endif

/*
 *	���X�^�P�ʃ����_�����O
 *	�`��
 */
void FASTCALL DrawPostRenderGDI(HWND hWnd, HDC hDC)
{
	HDC hMemDC;
	HBITMAP hDefBitmap;
	WORD top, bottom;

	ASSERT(hWnd);
	ASSERT(hDC);

	/* �p���b�g�ݒ� */
#if XM7_VER >= 3
	if (nMode == SCR_400LINE) {
		if (bPaletFlag) {
			Palet640();
		}
	}
	else if (nMode == SCR_200LINE) {
		if (bPaletFlag) {
			Palet640();
		}
		if (!bPseudo400Line) {
			if (bFullScan) {
				RenderFullScan();
			}
			else {
				RenderSetOddLineDigital();
			}
		}
	}
	else {
		if (bFullScan) {
			RenderFullScan();
		}
		else {
			RenderSetOddLineAnalog();
		}
	}
#elif XM7_VER >= 2
	if (bAnalog) {
#if defined(FMTV151)
		DrawV2_16bpp();
#endif
		if (bFullScan) {
			RenderFullScan();
		}
		else {
			RenderSetOddLineAnalog();
		}
	}
	else {
#if defined(FMTV151)
		DrawV2_4bpp();
#endif
		if (bPaletFlag) {
			Palet640();
		}
		if (!bPseudo400Line) {
			if (bFullScan) {
				RenderFullScan();
			}
			else {
				RenderSetOddLineDigital();
			}
		}
	}
#elif defined(L4CARD)
	if (b400Line) {
		if (bPaletFlag) {
			PaletL4();
		}
	}
	else {
		if (bPaletFlag) {
			Palet640();
		}
		if (!bPseudo400Line) {
			if (bFullScan) {
				RenderFullScan();
			}
			else {
				RenderSetOddLineDigital();
			}
		}
	}
#else
	if (bPaletFlag) {
		Palet640();
	}
	if (!bPseudo400Line) {
		if (bFullScan) {
			RenderFullScan();
		}
		else {
			RenderSetOddLineDigital();
		}
	}
#endif

	/* ������DC�쐬�A�I�u�W�F�N�g�I�� */
	hMemDC = CreateCompatibleDC(hDC);
	ASSERT(hMemDC);
	hDefBitmap = (HBITMAP)SelectObject(hMemDC, hBitmap);

	/* ���X�^�P�ʃ����_�����O���ɂЂ�����Ԃ邱�Ƃ�����̂� */
	if (nDrawTop >= nDrawBottom) {
		top = 0;
		bottom = 400;
	}
	else {
		top = nDrawTop;
		bottom = nDrawBottom;
	}

	/* �f�X�N�g�b�v�̏󋵂�����ł́A���ۂ����蓾��l */
	if (hDefBitmap) {
	/* �p���b�g�ݒ� */
#if XM7_VER >= 3
		switch (nMode) {
			case SCR_400LINE	:
			case SCR_200LINE	:
				SetDIBColorTable(hMemDC, 0, 16, (RGBQUAD *)rgbTTLGDI);
				break;
		}
#elif XM7_VER >= 2
		if (!bAnalog) {
			SetDIBColorTable(hMemDC, 0, 16, (RGBQUAD *)rgbTTLGDI);
		}
#elif defined(L4CARD)
		if (b400Line) {
			SetDIBColorTable(hMemDC, 0, 32, (RGBQUAD *)rgbTTLL4GDI);
		}
		else {
			SetDIBColorTable(hMemDC, 0, 16, (RGBQUAD *)rgbTTLGDI);
		}
#else
		SetDIBColorTable(hMemDC, 0, 16, (RGBQUAD *)rgbTTLGDI);
#endif

		if (bDoubleSize) {
			StretchBlt(hDC, 0, top * 2, 1280, (bottom - top) * 2,
				hMemDC, 0, top, 640, (bottom - top), SRCCOPY);
		}
		else {
			BitBlt(hDC, 0, top, 640, (bottom - top),
				hMemDC, 0, top, SRCCOPY);
		}
		SelectObject(hMemDC, hDefBitmap);

		/* ����ɔ����A���[�N���Z�b�g */
		nDrawTop = 400;
		nDrawBottom = 0;
		nDrawLeft = 640;
		nDrawRight = 0;
		SetDrawFlag(TRUE);
	}
	else {
		/* �ĕ`����N���� */
		InvalidateRect(hWnd, NULL, FALSE);
	}

	/* ������DC�폜 */
	DeleteDC(hMemDC);
}

/*
 *	���X�^�P�ʃ����_�����O
 *	�e���X�^�����_�����O
 */
void FASTCALL DrawRasterGDI(int nRaster)
{
	/* 640-320 �����؂�ւ� */
	SelectGDI(hDrawWnd);

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
void FASTCALL EnterMenuGDI(HWND hWnd)
{
	ASSERT(hWnd);
	UNUSED(hWnd);

	/* �r�b�g�}�b�v�n���h���̗L���Ŕ���ł��� */
	if (!hBitmap) {
		return;
	}

	/* �}�E�X�J�[�\��on */
	if (!bMouseCursor) {
		ShowCursor(TRUE);
		bMouseCursor = TRUE;
	}

#if defined(MOUSE)
	/* �}�E�X�L���v�`����~ */
	SetMouseCapture(FALSE);
#endif
}

/*
 *	���j���[�I��
 */
void FASTCALL ExitMenuGDI(void)
{
	/* �r�b�g�}�b�v�n���h���̗L���Ŕ���ł��� */
	if (!hBitmap) {
		return;
	}

	/* �}�E�X�J�[�\��on */
	if (!bMouseCursor) {
		ShowCursor(TRUE);
		bMouseCursor = TRUE;
	}

#if defined(MOUSE)
	/* �}�E�X�L���v�`���J�n */
	SetMouseCapture(TRUE);
#endif
}

/*-[ VM�Ƃ̐ڑ� ]-----------------------------------------------------------*/

/*
 *	VRAM�Z�b�g
 */
void FASTCALL VramGDI(WORD addr)
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
	GDIDrawFlag[(y >> 3) * 80 + (x >> 3)] = 1;

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
void FASTCALL TvramGDI(WORD addr)
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
		GDIDrawFlag[(yy >> 3) * 80 + (x >> 3)] = 1;
		if (width40_flag) {
			GDIDrawFlag[(yy >> 3) * 80 + (x >> 3) + 1] = 1;
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
void FASTCALL ReDrawTVRamGDI(void)
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
void FASTCALL DigitalGDI(void)
{
	/* �s�v�ȃ����_�����O��}�����邽�߁A�̈�ݒ�͕`�掞�ɍs�� */
	bPaletFlag = TRUE;
}

/*
 *	�A�i���O�p���b�g�Z�b�g
 */
void FASTCALL AnalogGDI(void)
{
	bPaletFlag = TRUE;
	nDrawTop = 0;
	nDrawBottom = 400;
	nDrawLeft = 0;
	nDrawRight = 640;
	SetDrawFlag(TRUE);
}

/*
 *	�ĕ`��v��
 */
void FASTCALL ReDrawGDI(void)
{
	/* �ĕ`�� */
	nDrawTop = 0;
	nDrawBottom = 400;
	nDrawLeft = 0;
	nDrawRight = 640;
	bPaletFlag = TRUE;
	bClearFlag = TRUE;
	SetDrawFlag(TRUE);
}

#if XM7_VER >= 3
/*
 *	�n�[�h�E�F�A�E�B���h�E�ʒm
 */
void FASTCALL WindowGDI(void)
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
		p = &GDIDrawFlag[(nDrawTop >> 3) * 80 + (nDrawLeft >> 3)];
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

/*
 *	�Z���N�g�L�����Z������
 */
void SelectCancelGDI(void)
{
	if (!bFullScreen) {
		bSelectCancelFlag = TRUE;
		SetDirtyFlag(0, 400, TRUE);
		SetDrawFlag(TRUE);
	}
}

#endif	/* _WIN32 */
