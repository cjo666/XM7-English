/*
 *	FM-7 EMULATOR "XM7"
 *
 *	Copyright (C) 1999-2017 �o�h�D(Twitter:@xm6_original)
 *	Copyright (C) 2001-2017 Ryu Takegami (Twitter:@RyuTakegami)
 *
 *	[ �A�i���O�p���b�g ]
 *
 *	RHG����
 *	  2002.05.25		�s�v�ȉ�ʍĕ`��ʒm��}������悤�ɂ���
 *	  2015.02.06		�p���b�g���W�X�^�E�C���h�E�V�݂ɔ����A�C�j�V�G�[�^ROM��
 *						�L���ȂƂ��ɂ͋����I�ɕύX�ʒm�𔭍s����悤�ɕύX
 *						(���ꂪ�Ȃ���VC++�ŃR���p�C�������ۂɕs�����������)
 */

#if XM7_VER >= 2

#include <string.h>
#include "xm7.h"
#include "device.h"
#include "apalet.h"
#include "display.h"
#include "subctrl.h"

/*
 *	�O���[�o�� ���[�N
 */
WORD apalet_no;							/* �I���p���b�g�ԍ� */
BYTE apalet_b[4096];					/* B���x��(0-15) */
BYTE apalet_r[4096];					/* R���x��(0-15) */
BYTE apalet_g[4096];					/* G���x��(0-15) */

/*
 *	�A�i���O�p���b�g
 *	������
 */
BOOL FASTCALL apalet_init(void)
{
	return TRUE;
}

/*
 *	�A�i���O�p���b�g
 *	�N���[���A�b�v
 */
void FASTCALL apalet_cleanup(void)
{
}

/*
 *	�A�i���O�p���b�g
 *	���Z�b�g
 */
void FASTCALL apalet_reset(void)
{
	apalet_no = 0;

	memset(apalet_b, 0, sizeof(apalet_b));
	memset(apalet_r, 0, sizeof(apalet_r));
	memset(apalet_g, 0, sizeof(apalet_g));
}

/*-[ �������}�b�v�hI/O ]----------------------------------------------------*/

/*
 *	�A�i���O�p���b�g
 *	�P�o�C�g�ǂݏo��
 */
BOOL FASTCALL apalet_readb(WORD addr, BYTE *dat)
{
	/* �o�[�W�����`�F�b�N */
	if (fm7_ver < 2) {
		return FALSE;
	}

	/* �A�h���X�`�F�b�N */
	if ((addr >= 0xfd30) && (addr <= 0xfd34)) {
#if XM7_VER >= 3
		/* FM77AV�͓ǂݏo���ł��Ȃ� */
		if (fm7_ver < 3) {
			*dat = 0xff;
			return TRUE;
		}

		/* FM77AV40EX�͓ǂݏo���\ */
		switch (addr) {
			/* �p���b�g�ԍ���� */
			case 0xfd30:
				*dat = (BYTE)(0xf0 | (apalet_no >> 8));
				return TRUE;

			/* �p���b�g�ԍ����� */
			case 0xfd31:
				*dat = (BYTE)(0xf0 | (apalet_no & 0xff));
				return TRUE;

			/* B���x�� */
			case 0xfd32:
				*dat = (BYTE)(0xf0 | (apalet_b[apalet_no] & 0x0f));
				return TRUE;

			/* R���x�� */
			case 0xfd33:
				*dat = (BYTE)(0xf0 | (apalet_r[apalet_no] & 0x0f));
				return TRUE;

			/* G���x�� */
			case 0xfd34:
				*dat = (BYTE)(0xf0 | (apalet_g[apalet_no] & 0x0f));
				return TRUE;
		}
#else
		/* FM77AV�͓ǂݏo���ł��Ȃ� */
		*dat = 0xff;
		return TRUE;
#endif
	}

	return FALSE;
}

/*
 *	�A�i���O�p���b�g
 *	�P�o�C�g��������
 */
BOOL FASTCALL apalet_writeb(WORD addr, BYTE dat)
{
	/* �o�[�W�����`�F�b�N */
	if (fm7_ver < 2) {
		return FALSE;
	}

	switch (addr) {
		/* �p���b�g�ԍ���� */
		case 0xfd30:
			apalet_no &= (WORD)0xff;
			apalet_no |= (WORD)((dat & 0x0f) << 8);
			return TRUE;

		/* �p���b�g�ԍ����� */
		case 0xfd31:
			apalet_no &= (WORD)(0xf00);
			apalet_no |= (WORD)(dat);
			return TRUE;

		/* B���x�� */
		case 0xfd32:
			if (apalet_b[apalet_no] != (dat & 0x0f)) {
				apalet_b[apalet_no] = (BYTE)(dat & 0x0f);
				refpalet_notify();
#if XM7_VER >= 3
				if (screen_mode == SCR_4096) {
#else
				if (mode320) {
#endif
					apalet_notify();
				}
			}
			return TRUE;

		/* R���x�� */
		case 0xfd33:
			if (apalet_r[apalet_no] != (dat & 0x0f)) {
				apalet_r[apalet_no] = (BYTE)(dat & 0x0f);
				refpalet_notify();
#if XM7_VER >= 3
				if (screen_mode == SCR_4096) {
#else
				if (mode320) {
#endif
					apalet_notify();
				}
			}
			return TRUE;

		/* G���x�� */
		case 0xfd34:
			if (apalet_g[apalet_no] != (dat & 0x0f)) {
				apalet_g[apalet_no] = (BYTE)(dat & 0x0f);
				refpalet_notify();
#if XM7_VER >= 3
				if (screen_mode == SCR_4096) {
#else
				if (mode320) {
#endif
					apalet_notify();
				}
			}
			return TRUE;
	}

	return FALSE;
}

/*-[ �t�@�C��I/O ]----------------------------------------------------------*/

/*
 *	�A�i���O�p���b�g
 *	�Z�[�u
 */
BOOL FASTCALL apalet_save(int fileh)
{
	if (!file_word_write(fileh, apalet_no)) {
		return FALSE;
	}
	if (!file_write(fileh, apalet_b, sizeof(apalet_b))) {
		return FALSE;
	}
	if (!file_write(fileh, apalet_r, sizeof(apalet_r))) {
		return FALSE;
	}
	if (!file_write(fileh, apalet_g, sizeof(apalet_g))) {
		return FALSE;
	}

	return TRUE;
}

/*
 *	�A�i���O�p���b�g
 *	���[�h
 */
BOOL FASTCALL apalet_load(int fileh, int ver)
{
	/* �o�[�W�����`�F�b�N */
	if (ver < 200) {
		return FALSE;
	}

	if (!file_word_read(fileh, &apalet_no)) {
		return FALSE;
	}
	if (!file_read(fileh, apalet_b, sizeof(apalet_b))) {
		return FALSE;
	}
	if (!file_read(fileh, apalet_r, sizeof(apalet_r))) {
		return FALSE;
	}
	if (!file_read(fileh, apalet_g, sizeof(apalet_g))) {
		return FALSE;
	}

	return TRUE;
}

#endif	/* XM7_VER >= 2 */
