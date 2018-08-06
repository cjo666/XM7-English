/*
 *	FM-7 EMULATOR "XM7"
 *
 *	Copyright (C) 1999-2017 �o�h�D(Twitter:@xm6_original)
 *	Copyright (C) 2001-2017 Ryu Takegami (Twitter:@RyuTakegami)
 *
 *	[ �_�����Z�E�������(MB61VH010/011) ]
 *
 *	RHG����
 *	  2001.07.23(��)	��ʃ��[�h�̈�����ύX
 *	  2001.11.22		������ԃA���S���Y����ύX
 *	  2002.01.06		���C���X�^�C���p�^�[�����W�X�^�̓��e��_��ł��Ƃ�
 *						���[�e�[�g����悤�ɕύX
 *						������Ԏ��Ƀ}�X�N���W�X�^��ی삵�Ȃ��悤�ɕύX
 *	  2002.03.13		������ԕ���BUSY�ɂȂ��Ă��鎞�Ԃ��`�悵���h�b�g����
 *						���킹�ĕω�����悤�ɕύX
 *	  2003.04.06		������ԑS�̓��[�h�̓����ύX
 *	  2003.06.19		BUSY���Ԃ̌v�Z��1�h�b�g�P�ʂ���1�o�C�g�P�ʂɕύX
 *						(���̂ق������s���Ԃ����@�ɋ߂��悤�ȋC������c)
 *	  2004.03.17		���Ӗ��ɃR�����g���B(���@�N
 *						�ϐ����𐮗�
 *	  2004.09.21		�R�}���h1�̋��������F�������݂ɏC��
 *	  2010.07.14		alu_disable�̏����l�����@�ɍ��킹0x0f�ɕύX
 *	  2012.05.04		DEATH FORCE�̃��[�J�[���S�ŕs�����������̂�
 *						alu_disable�̏����l��0x00�ɖ߂�
 */

#if XM7_VER >= 2

#include <string.h>
#include <stdio.h>
#include "xm7.h"
#include "aluline.h"
#include "event.h"
#include "display.h"
#include "subctrl.h"
#include "multipag.h"
#include "device.h"

/*
 *	�O���[�o�� ���[�N
 */
BYTE alu_command;						/* �_�����Z �R�}���h */
BYTE alu_color;							/* �_�����Z �J���[ */
BYTE alu_mask;							/* �_�����Z �}�X�N�r�b�g */
BYTE alu_cmpstat;						/* �_�����Z ��r�X�e�[�^�X */
BYTE alu_cmpdat[8];						/* �_�����Z ��r�f�[�^ */
BYTE alu_disable;						/* �_�����Z �֎~�o���N */
BYTE alu_tiledat[3];					/* �_�����Z �^�C���p�^�[�� */

BOOL line_busy;							/* ������� BUSY */
WORD line_offset;						/* ������� �A�h���X�I�t�Z�b�g */
WORD line_style;						/* ������� ���C���X�^�C�� */
WORD line_x0;							/* ������� X0 */
WORD line_y0;							/* ������� Y0 */
WORD line_x1;							/* ������� X1 */
WORD line_y1;							/* ������� Y1 */
BOOL line_boost;						/* ������� �S���͕`��t���O */

/*
 *  �X�^�e�B�b�N ���[�N
 */
static BYTE alu_vram_bank;				/* �_�����Z ������VRAM�o���N */
static WORD line_addr_old;				/* ������� �O�񏈗�����VRAM�A�h���X */
static BYTE line_mask;					/* ������� �}�X�N�r�b�g */
static WORD line_count;					/* ������� �J�E���^(������) */
static BYTE line_count_sub;				/* ������� �J�E���^(������) */

/*
 *	�v���g�^�C�v�錾
 */
static void FASTCALL alu_cmp(WORD addr);	/* �R���y�A */


/*
 *	�_�����Z�E�������
 *	������
 */
BOOL FASTCALL aluline_init(void)
{
	/* ������ԏ�������BUSY��ԂƂ��� */
	line_boost = FALSE;

	return TRUE;
}

/*
 *	�_�����Z�E�������
 *	�N���[���A�b�v
 */
void FASTCALL aluline_cleanup(void)
{
}

/*
 *	�_�����Z�E�������
 *	���Z�b�g
 */
void FASTCALL aluline_reset(void)
{
	/* �S�Ẵ��W�X�^�������� */
	alu_command = 0;
	alu_color = 0;
	alu_mask = 0;
	alu_cmpstat = 0;
	memset(alu_cmpdat, 0x80, sizeof(alu_cmpdat));
	alu_disable = 0x00;
	memset(alu_tiledat, 0, sizeof(alu_tiledat));

	line_busy = FALSE;
	line_offset = 0;
	line_style = 0;
	line_x0 = 0;
	line_y0 = 0;
	line_x1 = 0;
	line_y1 = 0;

	alu_vram_bank = 0;
	line_addr_old = 0xffff;
	line_mask = 0xff;
	line_count = 0;
	line_count_sub = 0;
}

/*-[ �_�����Z ]-------------------------------------------------------------*/

/*
 *	�_�����Z
 *	VRAM�ǂݏo��
 */
static BYTE FASTCALL alu_read(WORD addr)
{
	/* �A�N�e�B�u�v���[���łȂ��ꍇ�A0xFF��Ԃ� */
	if (multi_page & (1 << alu_vram_bank)) {
		return 0xff;
	}

#if XM7_VER >= 3
	if (mode400l) {
		if (addr >= 0x8000) {
			/* 400���C������0x8000�`0xBFFF�ɂ̓����������݂��Ȃ� */
			return 0xff;
		}
		else {
			return vram_ablk[alu_vram_bank * 0x8000 + addr];
		}
	}
	else {
		return vram_aptr[alu_vram_bank * 0x8000 + addr];
	}
#else
	return vram_aptr[alu_vram_bank * 0x4000 + addr];
#endif
}

/*
 *	�_�����Z
 *	VRAM�ǂݏo��(�v���[���w��)
 */
static BYTE FASTCALL alu_read_plane(WORD addr, int plane)
{
	/* �A�N�e�B�u�v���[���łȂ��ꍇ�A0xFF��Ԃ� */
	if (multi_page & (1 << plane)) {
		return 0xff;
	}

#if XM7_VER >= 3
	if (mode400l) {
		if (addr >= 0x8000) {
			/* 400���C������0x8000�`0xBFFF�ɂ̓����������݂��Ȃ� */
			return 0xff;
		}
		else {
			return vram_ablk[plane * 0x8000 + addr];
		}
	}
	else {
		return vram_aptr[plane * 0x8000 + addr];
	}
#else
	return vram_aptr[plane * 0x4000 + addr];
#endif
}

/*
 *	�_�����Z
 *	VRAM��������
 */
static void FASTCALL alu_write(WORD addr, BYTE dat)
{
	/* �A�N�e�B�u�v���[���łȂ��ꍇNOP */
	if (multi_page & (1 << alu_vram_bank)) {
		return;
	}

#if XM7_VER >= 3
	if (mode400l) {
		/* 400���C������0x8000�`0xBFFF�ɂ̓����������݂��Ȃ��̂�NOP */
		if (addr < 0x8000) {
			if (vram_ablk[alu_vram_bank * 0x8000 + addr] != dat) {
				vram_ablk[alu_vram_bank * 0x8000 + addr] = dat;
				vram_notify(addr, dat);
			}
		}
	}
	else {
		if (vram_aptr[alu_vram_bank * 0x8000 + addr] != dat) {
			vram_aptr[alu_vram_bank * 0x8000 + addr] = dat;
			vram_notify(addr, dat);
		}
	}
#else
	if (vram_aptr[alu_vram_bank * 0x4000 + addr] != dat) {
		vram_aptr[alu_vram_bank * 0x4000 + addr] = dat;
		vram_notify(addr, dat);
	}
#endif
}

/*
 *	�_�����Z
 *	�������݃T�u(��r�������ݕt)
 */
static void FASTCALL alu_writesub(WORD addr, BYTE dat)
{
	BYTE temp;

	/* ��ɏ������݉\�� */
	if ((alu_command & 0x40) == 0) {
		alu_write(addr, dat);
		return;
	}

	/* �C�R�[���������݂��ANOT�C�R�[���������݂� */
	if (alu_command & 0x20) {
		/* NOT�C�R�[���ŏ������� */
		temp = alu_read(addr);
		temp &= alu_cmpstat;
		dat &= (BYTE)(~alu_cmpstat);
	}
	else {
		/* �C�R�[���ŏ������� */
		temp = alu_read(addr);
		temp &= (BYTE)(~alu_cmpstat);
		dat &= alu_cmpstat;
	}

	/* �f�[�^��VRAM�ɏ������� */
	alu_write(addr, (BYTE)(temp | dat));
}

/*
 *	�_�����Z
 *	PSET
 */
static void FASTCALL alu_pset(WORD addr)
{
	BYTE dat;
	BYTE mask;
	BYTE bit;

	/* VRAM�I�t�Z�b�g���擾�A�r�b�g�ʒu������ */
	alu_vram_bank = 0;
	bit = 0x01;

#if XM7_VER >= 3
	if (mode400l) {
		addr &= 0x7fff;
	}
	else {
		addr &= 0x3fff;
	}
#else
	addr &= 0x3fff;
#endif

	/* ��r�������ݎ��́A��ɔ�r�f�[�^���擾 */
	if (alu_command & 0x40) {
		alu_cmp(addr);
	}

	/* �e�o���N�ɑ΂��ď������s��(�����o���N�̓X�L�b�v) */
	while (alu_vram_bank < 3) {
		if (!(alu_disable & bit)) {
			/* ���Z�J���[�f�[�^���A�f�[�^�쐬 */
			if (alu_color & bit) {
				dat = 0xff;
			}
			else {
				dat = 0;
			}

			/* ���Z�Ȃ�(PSET) */
			mask = alu_read(addr);

			/* �}�X�N�r�b�g�̏��� */
			dat &= (BYTE)(~alu_mask);
			mask &= alu_mask;
			dat |= mask;

			/* �������� */
			alu_writesub(addr, dat);
		}

		/* ���̃o���N�� */
		alu_vram_bank ++;
		bit <<= 1;
	}
}

/*
 *	�_�����Z
 *	�R�}���h1(�֎~�E���F��������)
 */
static void FASTCALL alu_prohibit(WORD addr)
{
	BYTE dat;
	BYTE mask;
	BYTE bit;

	/* VRAM�I�t�Z�b�g���擾�A�r�b�g�ʒu������ */
	alu_vram_bank = 0;
	bit = 0x01;

#if XM7_VER >= 3
	if (mode400l) {
		addr &= 0x7fff;
	}
	else {
		addr &= 0x3fff;
	}
#else
	addr &= 0x3fff;
#endif

	/* ��r�������ݎ��́A��ɔ�r�f�[�^���擾 */
	if (alu_command & 0x40) {
		alu_cmp(addr);
	}

	/* �e�o���N�ɑ΂��ď������s��(�����o���N�̓X�L�b�v) */
	while (alu_vram_bank < 3) {
		if (!(alu_disable & bit)) {
			/* ���Z�Ȃ� */
			mask = alu_read(addr);

			/* �}�X�N�r�b�g�̏��� */
			dat = (mask & alu_mask);

			/* �������� */
			alu_writesub(addr, dat);
		}

		/* ���̃o���N�� */
		alu_vram_bank ++;
		bit <<= 1;
	}
}

/*
 *	�_�����Z
 *	OR
 */
static void FASTCALL alu_or(WORD addr)
{
	BYTE dat;
	BYTE mask;
	BYTE bit;

	/* VRAM�I�t�Z�b�g���擾�A�r�b�g�ʒu������ */
	alu_vram_bank = 0;
	bit = 0x01;

#if XM7_VER >= 3
	if (mode400l) {
		addr &= 0x7fff;
	}
	else {
		addr &= 0x3fff;
	}
#else
	addr &= 0x3fff;
#endif


	/* ��r�������ݎ��́A��ɔ�r�f�[�^���擾 */
	if (alu_command & 0x40) {
		alu_cmp(addr);
	}

	/* �e�o���N�ɑ΂��ď������s��(�����o���N�̓X�L�b�v) */
	while (alu_vram_bank < 3) {
		if (!(alu_disable & bit)) {
			/* ���Z�J���[�f�[�^���A�f�[�^�쐬 */
			if (alu_color & bit) {
				dat = 0xff;
			}
			else {
				dat = 0;
			}

			/* ���Z */
			mask = alu_read(addr);
			dat |= mask;

			/* �}�X�N�r�b�g�̏��� */
			dat &= (BYTE)(~alu_mask);
			mask &= alu_mask;
			dat |= mask;

			/* �������� */
			alu_writesub(addr, dat);
		}

		/* ���̃o���N�� */
		alu_vram_bank ++;
		bit <<= 1;
	}
}

/*
 *	�_�����Z
 *	AND
 */
static void FASTCALL alu_and(WORD addr)
{
	BYTE dat;
	BYTE mask;
	BYTE bit;

	/* VRAM�I�t�Z�b�g���擾�A�r�b�g�ʒu������ */
	alu_vram_bank = 0;
	bit = 0x01;

#if XM7_VER >= 3
	if (mode400l) {
		addr &= 0x7fff;
	}
	else {
		addr &= 0x3fff;
	}
#else
	addr &= 0x3fff;
#endif

	/* ��r�������ݎ��́A��ɔ�r�f�[�^���擾 */
	if (alu_command & 0x40) {
		alu_cmp(addr);
	}

	/* �e�o���N�ɑ΂��ď������s��(�����o���N�̓X�L�b�v) */
	while (alu_vram_bank < 3) {
		if (!(alu_disable & bit)) {
			/* ���Z�J���[�f�[�^���A�f�[�^�쐬 */
			if (alu_color & bit) {
				dat = 0xff;
			}
			else {
				dat = 0;
			}

			/* ���Z */
			mask = alu_read(addr);
			dat &= mask;

			/* �}�X�N�r�b�g�̏��� */
			dat &= (BYTE)(~alu_mask);
			mask &= alu_mask;
			dat |= mask;

			/* �������� */
			alu_writesub(addr, dat);
		}

		/* ���̃o���N�� */
		alu_vram_bank ++;
		bit <<= 1;
	}
}

/*
 *	�_�����Z
 *	XOR
 */
static void FASTCALL alu_xor(WORD addr)
{
	BYTE dat;
	BYTE mask;
	BYTE bit;

	/* VRAM�I�t�Z�b�g���擾�A�r�b�g�ʒu������ */
	alu_vram_bank = 0;
	bit = 0x01;

#if XM7_VER >= 3
	if (mode400l) {
		addr &= 0x7fff;
	}
	else {
		addr &= 0x3fff;
	}
#else
	addr &= 0x3fff;
#endif

	/* ��r�������ݎ��́A��ɔ�r�f�[�^���擾 */
	if (alu_command & 0x40) {
		alu_cmp(addr);
	}

	/* �e�o���N�ɑ΂��ď������s��(�����o���N�̓X�L�b�v) */
	while (alu_vram_bank < 3) {
		if (!(alu_disable & bit)) {
			/* ���Z�J���[�f�[�^���A�f�[�^�쐬 */
			if (alu_color & bit) {
				dat = 0xff;
			}
			else {
				dat = 0;
			}

			/* ���Z */
			mask = alu_read(addr);
			dat ^= mask;

			/* �}�X�N�r�b�g�̏��� */
			dat &= (BYTE)(~alu_mask);
			mask &= alu_mask;
			dat |= mask;

			/* �������� */
			alu_writesub(addr, dat);
		}

		/* ���̃o���N�� */
		alu_vram_bank ++;
		bit <<= 1;
	}
}

/*
 *	�_�����Z
 *	NOT
 */
static void FASTCALL alu_not(WORD addr)
{
	BYTE dat;
	BYTE mask;
	BYTE bit;

	/* VRAM�I�t�Z�b�g���擾�A�r�b�g�ʒu������ */
	alu_vram_bank = 0;
	bit = 0x01;

#if XM7_VER >= 3
	if (mode400l) {
		addr &= 0x7fff;
	}
	else {
		addr &= 0x3fff;
	}
#else
	addr &= 0x3fff;
#endif

	/* ��r�������ݎ��́A��ɔ�r�f�[�^���擾 */
	if (alu_command & 0x40) {
		alu_cmp(addr);
	}

	/* �e�o���N�ɑ΂��ď������s��(�����o���N�̓X�L�b�v) */
	while (alu_vram_bank < 3) {
		if (!(alu_disable & bit)) {
			/* ���Z(NOT) */
			mask = alu_read(addr);
			dat = (BYTE)(~mask);

			/* �}�X�N�r�b�g�̏��� */
			dat &= (BYTE)(~alu_mask);
			mask &= alu_mask;
			dat |= mask;

			/* �������� */
			alu_writesub(addr, dat);
		}

		/* ���̃o���N�� */
		alu_vram_bank ++;
		bit <<= 1;
	}
}

/*
 *	�_�����Z
 *	�^�C���y�C���g
 */
static void FASTCALL alu_tile(WORD addr)
{
	BYTE bit;
	BYTE mask;
	BYTE dat;

	/* VRAM�I�t�Z�b�g���擾�A�r�b�g�ʒu������ */
	alu_vram_bank = 0;
	bit = 0x01;

#if XM7_VER >= 3
	if (mode400l) {
		addr &= 0x7fff;
	}
	else {
	 	addr &= 0x3fff;
	}
#else
	 addr &= 0x3fff;
#endif

	/* ��r�������ݎ��́A��ɔ�r�f�[�^���擾 */
	if (alu_command & 0x40) {
		alu_cmp(addr);
	}

	/* �e�o���N�ɑ΂��ď������s��(�����o���N�̓X�L�b�v) */
	while (alu_vram_bank < 3) {
		if (!(alu_disable & bit)) {
			/* �f�[�^�쐬 */
			dat = alu_tiledat[alu_vram_bank];

			/* �}�X�N�r�b�g�̏��� */
			dat &= (BYTE)(~alu_mask);
			mask = alu_read(addr);
			mask &= alu_mask;
			dat |= mask;

			/* �������� */
			alu_writesub(addr, dat);
		}

		/* ���̃o���N�� */
		alu_vram_bank ++;
		bit <<= 1;
	}
}

/*
 *	�_�����Z
 *	�R���y�A
 */
static void FASTCALL alu_cmp(WORD addr)
{
	BYTE color;
	BYTE bit;
	int i, j;
	BOOL flag;
	BYTE dat;
	BYTE b, r, g;
	BYTE disflag;

	/* �A�h���X�}�X�N */
#if XM7_VER >= 3
	if (mode400l) {
		addr &= 0x7fff;
	}
	else {
		addr &= 0x3fff;
	}
#else
	addr &= 0x3fff;
#endif

	/* �J���[�f�[�^�擾 */
	b = alu_read_plane(addr, 0);
	r = alu_read_plane(addr, 1);
	g = alu_read_plane(addr, 2);

	/* �o���N�f�B�Z�[�u�����l������(���_�]���΍�) */
	disflag = (BYTE)((~alu_disable) & 0x07);

	/* ��r���K�v */
	dat = 0;
	bit = 0x80;
	for (i=0; i<8; i++) {
		/* �F���쐬 */
		color = 0;
		if (b & bit) {
			color |= 0x01;
		}
		if (r & bit) {
			color |= 0x02;
		}
		if (g & bit) {
			color |= 0x04;
		}

		/* 8�̐F�X���b�g���܂���āA�ǂꂩ��v������̂����邩 */
		flag = FALSE;
		for (j=0; j<8; j++) {
			if ((alu_cmpdat[j] & 0x80) == 0) {
				if ((alu_cmpdat[j] & disflag) == (color & disflag)) {
					flag = TRUE;
					break;
				}
			}
		}

		/* �C�R�[����1��ݒ� */
		if (flag) {
			dat |= bit;
		}

		/* ���� */
		bit >>= 1;
	}

	/* �f�[�^�ݒ� */
	alu_cmpstat = dat;
}

/*-[ ������� ]-------------------------------------------------------------*/

/*
 *	�������
 *	�_�����Z��H�N��
 */
void FASTCALL aluline_exec(WORD addr)
{
	/* �A�h���X�`�F�b�N */
	if (addr >= 0x8000) {
		line_mask = 0xff;
		return;
	}

	/* �}�X�N��ݒ� */
	alu_mask = line_mask;
	line_mask = 0xff;

	/* �_�����Z */
	switch (alu_command & 0x07) {
		/* PSET */
		case 0:
			alu_pset(addr);
			break;
		/* �֎~(���F��������) */
		case 1:
			alu_prohibit(addr);
			break;
		/* OR */
		case 2:
			alu_or(addr);
			break;
		/* AND */
		case 3:
			alu_and(addr);
			break;
		/* XOR */
		case 4:
			alu_xor(addr);
			break;
		/* NOT */
		case 5:
			alu_not(addr);
			break;
		/* �^�C���y�C���g */
		case 6:
			alu_tile(addr);
			break;
		/* �R���y�A */
		case 7:
			alu_cmp(addr);
			break;
	}

	/* �J�E���g�A�b�v */
	line_count ++;
}

/*
 *	�������
 *	�_�`��
 */
static void FASTCALL aluline_pset(int x, int y)
{
	WORD addr;
	static BYTE mask[] = {0x7f, 0xbf, 0xdf, 0xef, 0xf7, 0xfb, 0xfd, 0xfe};

	/* �_�����Z�̃f�[�^�o�X�ɓ���̂ŁA�_�����Zon���K�v */
	if (!(alu_command & 0x80)) {
		return;
	}

	/* ��ʃ��[�h����A�A�h���X�Z�o */
#if XM7_VER >= 3
	if (screen_mode & SCR_ANALOG) {
		addr = (WORD)(y * 40 + (x >> 3));
	}
	else {
		addr = (WORD)(y * 80 + (x >> 3));
	}
#else
	if (mode320) {
		addr = (WORD)(y * 40 + (x >> 3));
	}
	else {
		addr = (WORD)(y * 80 + (x >> 3));
	}
#endif

	/* �I�t�Z�b�g�������� */
	addr += line_offset;
#if XM7_VER >= 3
	if (mode400l) {
		addr &= 0x7fff;
	}
	else {
		addr &= 0x3fff;
	}
#else
	addr &= 0x3fff;
#endif

	/* �O��̃A�h���X�ƍ���̃A�h���X����v���Ȃ��ꍇ�_�����Z���s�� */
	if (line_addr_old != addr) {
		/* �_�����Z�𓮂��� */
		aluline_exec(line_addr_old);
		line_addr_old = addr;
	}

	/* ���C���X�^�C�� */
	/* �X�^�C���r�b�g�������Ă���ꍇ�̂� */
	if (line_style & 0x8000) {
		/* �}�X�N��ݒ� */
		line_mask &= mask[x & 0x07];
	}

	/* ���C���X�^�C�����W�X�^�������[�e�[�g */
	line_style = (WORD)((line_style << 1) | (line_style >> 15));
}

/*
 *	�����`��(DDA)
 *	�Q�l����:6809�@�B��ɂ��O���t�B�b�N�����̋Z�@ ��6�� (��h!FM '86/5)
 */
static void FASTCALL aluline_line(void)
{
	int x1, x2, y1, y2;
	int dx, dy, ux, uy, r;

	/* �f�[�^�擾 */
	x1 = (int)line_x0;
	x2 = (int)line_x1;
	y1 = (int)line_y0;
	y2 = (int)line_y1;

	/* �J�E���^������ */
	line_count = 0;
	line_addr_old = 0xffff;
	line_mask = 0xff;

	/* �����l�̌v�Z */
	dx = x2 - x1;
	dy = y2 - y1;
	if (dx < 0) {
		ux = -1;
		dx = -dx;
	}
	else {
		ux = 1;
	}
	if (dy < 0) {
		uy = -1;
		dy = -dy;
	}
	else {
		uy = 1;
	}

	if ((dx == 0) && (dy == 0)) {
		/* �P��_�`�� */
		aluline_pset(x1, y1);
	}
	else if (dx == 0) {
		/* X������̏ꍇ (����) */
		for (;;) {
			aluline_pset(x1, y1);
			if (y1 == y2) {
				break;
			}
			y1 += uy;
		}
	}
	else if (dy == 0) {
		/* Y������̏ꍇ (����) */
		for (;;) {
			aluline_pset(x1, y1);
			if (x1 == x2) {
				break;
			}
			x1 += ux;
		}
	}
	else if (dx >= dy) {
		/* ���C�����[�v1 (DX >= DY) */
		r = dx >> 1;
		for (;;) {
			aluline_pset(x1, y1);
			if (x1 == x2) {
				break;
			}

			x1 += ux;
			r -= dy;
			if (r < 0) {
				r += dx;
				y1 += uy;
			}
		}
	}
	else {
		/* ���C�����[�v2 (DX < DY) */
		r = dy >> 1;
		for (;;) {
			aluline_pset(x1, y1);
			if (y1 == y2) {
				break;
			}

			y1 += uy;
			r -= dx;
			if (r < 0) {
				r += dy;
				x1 += ux;
			}
		}
	}

	/* �Ō�̃o�C�g�̘_�����Z */
	aluline_exec(line_addr_old);
}


/*
 *	�������
 *	�C�x���g
 */
static BOOL FASTCALL aluline_event(void)
{
	/* ������Ԃ�READY�ɂ��� */
	line_busy = FALSE;

	schedule_delevent(EVENT_LINE);
	return TRUE;
}

/*-[ �������}�b�v�hI/O ]----------------------------------------------------*/

/*
 *	�_�����Z�E�������
 *	�P�o�C�g�ǂݏo��
 */
BOOL FASTCALL aluline_readb(WORD addr, BYTE *dat)
{
	/* �o�[�W�����`�F�b�N */
	if (fm7_ver < 2) {
		return FALSE;
	}

	switch (addr) {
		/* �_�����Z �R�}���h */
		case 0xd410:
			*dat = alu_command;
			return TRUE;

		/* �_�����Z �J���[ */
		case 0xd411:
			*dat = alu_color;
			return TRUE;

		/* �_�����Z �}�X�N�r�b�g */
		case 0xd412:
			*dat = alu_mask;
			return TRUE;

		/* �_�����Z ��r�X�e�[�^�X */
		case 0xd413:
			*dat = alu_cmpstat;
			return TRUE;

		/* �_�����Z �����o���N */
		case 0xd41b:
			*dat = alu_disable;
			return TRUE;
	}

	/* �_�����Z ��r�f�[�^ */
	if ((addr >= 0xd413) && (addr <= 0xd41a)) {
		*dat = 0xff;
		return TRUE;
	}

	/* �_�����Z �^�C���p�^�[�� */
	if ((addr >= 0xd41c) && (addr <= 0xd41e)) {
		*dat = 0xff;
		return TRUE;
	}

	/* ������� */
	if ((addr >= 0xd420) && (addr <= 0xd42b)) {
		*dat = 0xff;
		return TRUE;
	}

	return FALSE;
}

/*
 *	�_�����Z�E�������
 *	�P�o�C�g��������
 */
BOOL FASTCALL aluline_writeb(WORD addr, BYTE dat)
{
	DWORD tmp;

	/* �o�[�W�����`�F�b�N */
	if (fm7_ver < 2) {
		return FALSE;
	}

	switch (addr) {
		/* �_�����Z �R�}���h */
		case 0xd410:
			alu_command = dat;
			return TRUE;

		/* �_�����Z �J���[ */
		case 0xd411:
			alu_color = dat;
			return TRUE;

		/* �_�����Z �}�X�N�r�b�g */
		case 0xd412:
			alu_mask = dat;
			return TRUE;

		/* �_�����Z �����o���N */
		case 0xd41b:
			alu_disable = dat;
			return TRUE;

		/* ������� �A�h���X�I�t�Z�b�g(A1���璍��) */
		case 0xd420:
			line_offset &= 0x01fe;
			line_offset |= (WORD)((dat * 512) & 0x3e00);
			return TRUE;
		case 0xd421:
			line_offset &= 0x3e00;
			line_offset |= (WORD)(dat * 2);
			return TRUE;

		/* ������� ���C���X�^�C�� */
		case 0xd422:
			line_style &= 0xff;
			line_style |= (WORD)(dat * 256);
			return TRUE;
		case 0xd423:
			line_style &= 0xff00;
			line_style |= (WORD)dat;
			return TRUE;

		/* ������� X0 */
		case 0xd424:
			line_x0 &= 0xff;
			line_x0 |= (WORD)(dat * 256);
			line_x0 &= 0x03ff;
			return TRUE;
		case 0xd425:
			line_x0 &= 0xff00;
			line_x0 |= (WORD)dat;
			return TRUE;

		/* ������� Y0 */
		case 0xd426:
			line_y0 &= 0xff;
			line_y0 |= (WORD)(dat * 256);
			line_y0 &= 0x01ff;
			return TRUE;
		case 0xd427:
			line_y0 &= 0xff00;
			line_y0 |= (WORD)dat;
			return TRUE;

		/* ������� X1 */
		case 0xd428:
			line_x1 &= 0xff;
			line_x1 |= (WORD)(dat * 256);
			line_x1 &= 0x03ff;
			return TRUE;
		case 0xd429:
			line_x1 &= 0xff00;
			line_x1 |= (WORD)dat;
			return TRUE;

		/* ������� Y1 */
		case 0xd42a:
			line_y1 &= 0xff;
			line_y1 |= (WORD)(dat * 256);
			line_y1 &= 0x01ff;
			return TRUE;
		case 0xd42b:
			line_y1 &= 0xff00;
			line_y1 |= (WORD)dat;

			/* �����Œ�����ԃX�^�[�g */
			aluline_line();

			/* ���͈��������A���΂炭BUSY�ɂ��Ă��� */
			/* BUSY��Ԃɂ��鎞�Ԃ��Z�o(1�o�C�g�`��=1/16��sec�Ɖ���) */
			tmp = (DWORD)(line_count >> 4);

			/* BUSY���Ԃ̏������������s�� */
			line_count_sub += (BYTE)(line_count & 0x0f);
			if (line_count_sub >= 0x10) {
				tmp ++;
				line_count_sub &= (BYTE)0x0f;
			}

			/* �v�Z���ʂ�1��s�ȏ�̏ꍇ��BUSY�ɂ��� */
			if (tmp > 0) {
				line_busy = TRUE;
				schedule_setevent(EVENT_LINE, tmp, aluline_event);
			}

			return TRUE;
	}

	/* �_�����Z ��r�f�[�^ */
	if ((addr >= 0xd413) && (addr <= 0xd41a)) {
		alu_cmpdat[addr - 0xd413] = dat;
		return TRUE;
	}

	/* �_�����Z �^�C���p�^�[�� */
	if ((addr >= 0xd41c) && (addr <= 0xd41e)) {
		alu_tiledat[addr - 0xd41c] = dat;
		return TRUE;
	}

	return FALSE;
}

/*
 *	�_�����Z�E�������
 *	VRAM�_�~�[���[�h
 */
void FASTCALL aluline_extrb(WORD addr)
{
	/* �_�����Z���L���� */
	if (alu_command & 0x80) {
		/* �R�}���h�� */
		switch (alu_command & 0x07) {
			/* PSET */
			case 0:
				alu_pset(addr);
				break;
			/* �֎~(���F��������) */
			case 1:
				alu_prohibit(addr);
				break;
			/* OR */
			case 2:
				alu_or(addr);
				break;
			/* AND */
			case 3:
				alu_and(addr);
				break;
			/* XOR */
			case 4:
				alu_xor(addr);
				break;
			/* NOT */
			case 5:
				alu_not(addr);
				break;
			/* �^�C���y�C���g */
			case 6:
				alu_tile(addr);
				break;
			/* �R���y�A */
			case 7:
				alu_cmp(addr);
				break;
		}
	}
}

/*-[ �t�@�C��I/O ]----------------------------------------------------------*/

/*
 *	�_�����Z�E�������
 *	�Z�[�u
 */
BOOL FASTCALL aluline_save(int fileh)
{
	if (!file_byte_write(fileh, alu_command)) {
		return FALSE;
	}
	if (!file_byte_write(fileh, alu_color)) {
		return FALSE;
	}
	if (!file_byte_write(fileh, alu_mask)) {
		return FALSE;
	}
	if (!file_byte_write(fileh, alu_cmpstat)) {
		return FALSE;
	}
	if (!file_write(fileh, alu_cmpdat, sizeof(alu_cmpdat))) {
		return FALSE;
	}
	if (!file_byte_write(fileh, alu_disable)) {
		return FALSE;
	}
	if (!file_write(fileh, alu_tiledat, sizeof(alu_tiledat))) {
		return FALSE;
	}

	if (!file_bool_write(fileh, line_busy)) {
		return FALSE;
	}

	if (!file_word_write(fileh, line_offset)) {
		return FALSE;
	}
	if (!file_word_write(fileh, line_style)) {
		return FALSE;
	}
	if (!file_word_write(fileh, line_x0)) {
		return FALSE;
	}
	if (!file_word_write(fileh, line_y0)) {
		return FALSE;
	}
	if (!file_word_write(fileh, line_x1)) {
		return FALSE;
	}
	if (!file_word_write(fileh, line_y1)) {
		return FALSE;
	}

	/* �]���o�[�W�����Ƃ̌݊��p�_�~�[ */
	if (!file_byte_write(fileh, 0)) {
		return FALSE;
	}

	return TRUE;
}

/*
 *	�_�����Z�E�������
 *	���[�h
 */
BOOL FASTCALL aluline_load(int fileh, int ver)
{
	BYTE tmp;

	/* �o�[�W�����`�F�b�N */
	if (ver < 200) {
		return FALSE;
	}

	if (!file_byte_read(fileh, &alu_command)) {
		return FALSE;
	}
	if (!file_byte_read(fileh, &alu_color)) {
		return FALSE;
	}
	if (!file_byte_read(fileh, &alu_mask)) {
		return FALSE;
	}
	if (!file_byte_read(fileh, &alu_cmpstat)) {
		return FALSE;
	}
	if (!file_read(fileh, alu_cmpdat, sizeof(alu_cmpdat))) {
		return FALSE;
	}
	if (!file_byte_read(fileh, &alu_disable)) {
		return FALSE;
	}
	if (!file_read(fileh, alu_tiledat, sizeof(alu_tiledat))) {
		return FALSE;
	}

	if (!file_bool_read(fileh, &line_busy)) {
		return FALSE;
	}

	if (!file_word_read(fileh, &line_offset)) {
		return FALSE;
	}
	if (!file_word_read(fileh, &line_style)) {
		return FALSE;
	}
	if (!file_word_read(fileh, &line_x0)) {
		return FALSE;
	}
	if (!file_word_read(fileh, &line_y0)) {
		return FALSE;
	}
	if (!file_word_read(fileh, &line_x1)) {
		return FALSE;
	}
	if (!file_word_read(fileh, &line_y1)) {
		return FALSE;
	}

	/* �]���o�[�W�����Ƃ̌݊��p�_�~�[ */
	if (!file_byte_read(fileh, &tmp)) {
		return FALSE;
	}

	/* �C�x���g */
	schedule_handle(EVENT_LINE, aluline_event);

	return TRUE;
}

#endif /* XM7_VER >= 2 */
