/*
 *	FM-7 EMULATOR "XM7"
 *
 *	Copyright (C) 1999-2017 �o�h�D(Twitter:@xm6_original)
 *	Copyright (C) 2001-2017 Ryu Takegami (Twitter:@RyuTakegami)
 *
 *	[ �}���`�y�[�W ]
 *
 *	RHG����
 *	  2002.05.25		�s�v�ȉ�ʍĕ`��ʒm��}������悤�ɂ���
 *	  2002.07.12		��ʍĕ`��̕K�v���𔻒肷�鎞�ɃA�N�e�B�u�y�[�W�ݒ��
 *						��������悤�ɕύX
 */

#include <string.h>
#include "xm7.h"
#include "multipag.h"
#include "display.h"
#include "ttlpalet.h"
#include "subctrl.h"
#include "apalet.h"
#include "device.h"

/*
 *	�O���[�o�� ���[�N
 */
BYTE multi_page;						/* �}���`�y�[�W ���[�N */

/*
 *	�}���`�y�[�W
 *	������
 */
BOOL FASTCALL multipag_init(void)
{
	return TRUE;
}

/*
 *	�}���`�y�[�W
 *	�N���[���A�b�v
 */
void FASTCALL multipag_cleanup(void)
{
}

/*
 *	�}���`�y�[�W
 *	���Z�b�g
 */
void FASTCALL multipag_reset(void)
{
	multi_page = 0;
}

/*
 *	�}���`�y�[�W
 *	�P�o�C�g�ǂݏo��
 */
BOOL FASTCALL multipag_readb(WORD addr, BYTE *dat)
{
#if XM7_VER == 1
	/* FM-8���[�h�ł͖��� */
	if (fm_subtype == FMSUB_FM8) {
		return FALSE;
	}
#endif

	if (addr != 0xfd37) {
		return FALSE;
	}

	/* ���FF���ǂݏo����� */
	*dat = 0xff;
	return TRUE;
}

/*
 *	�}���`�y�[�W
 *	�P�o�C�g��������
 */
BOOL FASTCALL multipag_writeb(WORD addr, BYTE dat)
{
#if XM7_VER == 1
	/* FM-8���[�h�ł͖��� */
	if (fm_subtype == FMSUB_FM8) {
		return FALSE;
	}
#endif

	if (addr != 0xfd37) {
		return FALSE;
	}

	if ((BYTE)(multi_page & 0x70) == (BYTE)(dat & 0x70)) {
		/* �\����ԂɕύX���Ȃ��ꍇ�̓f�[�^�L���������ċA�� */
		multi_page = dat;
		return TRUE;
	}

	/* �f�[�^�L�� */
	multi_page = dat;
	refpalet_notify();

	/* �p���b�g�Đݒ� */
	if (crt_flag) {
#if XM7_VER >= 2
#if XM7_VER >= 3
		if (!(screen_mode & SCR_ANALOG)) {
#else
		if (!mode320) {
#endif
			ttlpalet_notify();
		}
		else {
			apalet_notify();
		}
#else
		ttlpalet_notify();
#endif
	}

	return TRUE;
}

/*
 *	�}���`�y�[�W
 *	�Z�[�u
 */
BOOL FASTCALL multipag_save(int fileh)
{
	return file_byte_write(fileh, multi_page);
}

/*
 *	�}���`�y�[�W
 *	���[�h
 */
BOOL FASTCALL multipag_load(int fileh, int ver)
{
	/* �o�[�W�����`�F�b�N */
	if (ver < 200) {
		return FALSE;
	}

	return file_byte_read(fileh, &multi_page);
}
