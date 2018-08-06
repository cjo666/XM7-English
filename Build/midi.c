/*
 *	FM-7 EMULATOR "XM7"
 *
 *	Copyright (C) 1999-2017 �o�h�D(Twitter:@xm6_original)
 *	Copyright (C) 2001-2017 Ryu Takegami (Twitter:@RyuTakegami)
 *
 *	[ MIDI�A�_�v�^(��h!FM�f��MIDI�J�[�h?) ]
 *
 *	RHG����
 *	  2002.09.26		�V��
 */

#if defined(MIDI)

#include <string.h>
#include "xm7.h"
#include "midi.h"
#include "device.h"
#include "rs232c.h"

/*
 *	�O���[�o�� ���[�N
 */
BOOL midi_busy;
BOOL midi_txenable;
BOOL midi_selectmc;
BYTE midi_modereg;


/*
 *	MIDI�A�_�v�^
 *	������
 */
BOOL FASTCALL midi_init(void)
{
	midi_busy = FALSE;

	return TRUE;
}

/*
 *	MIDI�A�_�v�^
 *	�N���[���A�b�v
 */
void FASTCALL midi_cleanup(void)
{
}

/*
 *	MIDI�A�_�v�^
 *	���Z�b�g
 */
void FASTCALL midi_reset(void)
{
	midi_txenable = FALSE;
	midi_selectmc = TRUE;
	midi_modereg = 0xff;

	midi_reset_notify();
}

/*
 *	MIDI�A�_�v�^
 *	�P�o�C�g�ǂݏo��
 */
BOOL FASTCALL midi_readb(WORD addr, BYTE *dat)
{
#if XM7_VER == 1
	if (fm_subtype == FMSUB_FM8) {
		return FALSE;
	}
#endif

	switch (addr) {
		case 0xfdea:	/* USART DATA */
			/* MIDI IN�ɂ͔�Ή� */
			/* �ǂݏo�����ʂ�ύX (kaikiraw�c��bug) */
			*dat = 0xff;
			return TRUE;

		case 0xfdeb:	/* USART STATUS */
			if (midi_busy) {
				*dat = 0x02;
			}
			else {
				*dat = 0x07;
			}
			return TRUE;
	}

	return FALSE;
}

/*
 *	MIDI�A�_�v�^
 *	�P�o�C�g��������
 */
BOOL FASTCALL midi_writeb(WORD addr, BYTE dat)
{
#if XM7_VER == 1
	if (fm_subtype == FMSUB_FM8) {
		return FALSE;
	}
#endif

	switch (addr) {
		case 0xfdea:	/* USART DATA */
			/* TxE=1 �����[�h�R�}���h���W�X�^�ݒ�l������ȏꍇ�ɑ��M */
			if (midi_txenable && (midi_modereg == 0x4e)) {
				midi_notify(dat);
			}
			return TRUE;

		case 0xfdeb:	/* USART COMMAND */
			if (midi_selectmc) {
				/* ���[�h�R�}���h���W�X�^ */
				midi_modereg = dat;
				midi_selectmc = FALSE;
			}
			else {
				/* �R�}���h���W�X�^ */

				/* TXE */
				if (dat & RSC_TXEN) {
					midi_txenable = TRUE;
				}
				else {
					midi_txenable = FALSE;
				}

				/* �������Z�b�g */
				if (dat & RSC_IR) {
					midi_modereg = 0xff;
					midi_txenable = FALSE;
					midi_selectmc = TRUE;
				}
			}

			return TRUE;
	}

	return FALSE;
}

/*
 *	MIDI�A�_�v�^
 *	�Z�[�u
 */
BOOL FASTCALL midi_save(int fileh)
{
	if (!file_bool_write(fileh, midi_txenable)) {
		return FALSE;
	}
	if (!file_bool_write(fileh, midi_selectmc)) {
		return FALSE;
	}
	if (!file_byte_write(fileh, midi_modereg)) {
		return FALSE;
	}
	
	return TRUE;
}

/*
 *	MIDI�A�_�v�^
 *	���[�h
 */
BOOL FASTCALL midi_load(int fileh, int ver)
{
	/* �o�[�W�����`�F�b�N */
	if (ver < 200) {
		return FALSE;
	}
#if XM7_VER >= 3
	if (((ver >= 500) && (ver < 715)) || ((ver >= 800) && (ver < 915))) {
#elif XM7_VER >= 2
	if (ver < 715) {
#else
	if (ver < 305) {
#endif
		return TRUE;
	}

	if (!file_bool_read(fileh, &midi_txenable)) {
		return FALSE;
	}
	if (!file_bool_read(fileh, &midi_selectmc)) {
		return FALSE;
	}
	if (!file_byte_read(fileh, &midi_modereg)) {
		return FALSE;
	}
	
	return TRUE;
}

#endif	/* MIDI */
