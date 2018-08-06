/*
 *	FM-7 EMULATOR "XM7"
 *
 *	Copyright (C) 1999-2017 �o�h�D(Twitter:@xm6_original)
 *	Copyright (C) 2001-2017 Ryu Takegami (Twitter:@RyuTakegami)
 *
 *	[ TTL�p���b�g(MB15021) ]
 *
 *	RHG����
 *	  2002.01.28		MB15021�̎d�l�ɍ��킹�ăf�[�^���S�r�b�g�����A40EX���̂�
 *						�ǂݏo���f�[�^�̃}�X�N�������s���悤�ɕύX
 *						(2003.01.14 AV�ł�4�r�b�g�ǂݏ����Ή����m�F)
 *	  2002.05.25		�s�v�ȉ�ʍĕ`��ʒm��}������悤�ɂ���
 */

#include <string.h>
#include "xm7.h"
#include "ttlpalet.h"
#include "device.h"
#include "display.h"
#include "subctrl.h"

/*
 *	�O���[�o�� ���[�N
 */
BYTE ttl_palet[8];			/* TTL�p���b�g�f�[�^ */

/*
 *	TTL�p���b�g
 *	������
 */
BOOL FASTCALL ttlpalet_init(void)
{
	return TRUE;
}

/*
 *	TTL�p���b�g
 *	�N���[���A�b�v
 */
void FASTCALL ttlpalet_cleanup(void)
{
}

/*
 *	TTL�p���b�g
 *	���Z�b�g
 */
void FASTCALL ttlpalet_reset(void)
{
	int i;
	
	/* ���ׂĂ̐F�������� */
	for (i=0; i<8; i++) {
		ttl_palet[i] = (BYTE)(i | 0x08);
	}

	/* �ʒm */
	ttlpalet_notify();
}

/*
 *	TTL�p���b�g
 *	�P�o�C�g�ǂݏo��
 */
BOOL FASTCALL ttlpalet_readb(WORD addr, BYTE *dat)
{
#if XM7_VER == 1
	/* FM-8���[�h�ł͖��� */
	if (fm_subtype == FMSUB_FM8) {
		return FALSE;
	}
#endif

	/* �͈̓`�F�b�N�A�ǂݏo�� */
	if ((addr >= 0xfd38) && (addr <= 0xfd3f)) {
		ASSERT((WORD)(addr - 0xfd38) <= 7);

		/* ��ʃj�u����0xF0������ */
#if XM7_VER >= 3
		if (fm7_ver == 3) {
			/* FM77AV40EX�ł͉���3�r�b�g�̂ݗL�� */
			*dat = (BYTE)((ttl_palet[(WORD)(addr - 0xfd38)] & 0x07) | 0xf0);
		}
		else {
			/* FM-7/FM77AV(MB15021)�ł͉���4�r�b�g���L�� */
			*dat = (BYTE)(ttl_palet[(WORD)(addr - 0xfd38)] | 0xf0);
		}
#else
		/* FM-7/FM77AV(MB15021)�ł͉���4�r�b�g���L�� */
		*dat = (BYTE)(ttl_palet[(WORD)(addr - 0xfd38)] | 0xf0);
#endif

		return TRUE;
	}

	return FALSE;
}

/*
 *	TTL�p���b�g
 *	�P�o�C�g��������
 */
BOOL FASTCALL ttlpalet_writeb(WORD addr, BYTE dat)
{
	int no;

#if XM7_VER == 1
	/* FM-8���[�h�ł͖��� */
	if (fm_subtype == FMSUB_FM8) {
		return FALSE;
	}
#endif

	/* �͈̓`�F�b�N�A�������� */
	if ((addr >= 0xfd38) && (addr <= 0xfd3f)) {
		no = addr - 0xfd38;
		if (ttl_palet[no] != (dat & 0x0f)) {
			ttl_palet[no] = (BYTE)(dat & 0x0f);
			refpalet_notify();

			/* �ʒm */
#if XM7_VER >= 2
#if XM7_VER >= 3
			if (!(screen_mode & SCR_ANALOG)) {
#else
			if (!mode320) {
#endif
				ttlpalet_notify();
			}
#else
			ttlpalet_notify();
#endif
		}

		return TRUE;
	}

	return FALSE;
}

/*
 *	TTL�p���b�g
 *	�Z�[�u
 */
BOOL FASTCALL ttlpalet_save(int fileh)
{
	return file_write(fileh, ttl_palet, 8);
}

/*
 *	TTL�p���b�g
 *	���[�h
 */
BOOL FASTCALL ttlpalet_load(int fileh, int ver)
{
	/* �o�[�W�����`�F�b�N */
	if (ver < 200) {
		return FALSE;
	}

	return file_read(fileh, ttl_palet, 8);
}
