/*
 *	FM-7 EMULATOR "XM7"
 *
 *	Copyright (C) 1999-2017 �o�h�D(Twitter:@xm6_original)
 *	Copyright (C) 2001-2017 Ryu Takegami (Twitter:@RyuTakegami)
 *
 *	[ �C���e���W�F���g�}�E�X ]
 *
 *	RHG����
 *	  2002.07.22		�^�C���A�E�g�܂ł̎��Ԃ�300��s(T1max+T2max)�ɕύX
 *						�X�g���[�u�M���̏�Ԃ��ω����Ă��Ȃ��ꍇ�̓^�C���A�E�g
 *						�C�x���g��o�^���Ȃ��悤�ɕύX
 *	  2002.07.29		�^�C���A�E�g�܂ł̎��Ԃ�450��s�ɕύX
 *	  2003.08.20		�^�C���A�E�g�C�x���g�܂��������ύX
 */

#if defined(MOUSE)

#include <assert.h>
#include <string.h>
#include "xm7.h"
#include "device.h"
#include "mainetc.h"
#include "event.h"
#include "mouse.h"

/*
 *	�O���[�o�� ���[�N
 */
BYTE mos_port;							/* �}�E�X�ڑ��|�[�g */
BOOL mos_capture;						/* �}�E�X�L���v�`���t���O */

/*
 *	�X�^�e�B�b�N ���[�N
 */
static BYTE mos_x;						/* �w�ړ����� (������:+ �E����:-) */
static BYTE mos_y;						/* �x�ړ����� (�����:+ ������:-) */
static BYTE mos_phase;					/* �t�F�[�Y�J�E���^ */
static BOOL mos_strobe;					/* �X�g���[�u�M�����(�ۑ��p) */
static BYTE mosset_x;					/* MS X�ړ����� (������:- �E����:+) */
static BYTE mosset_y;					/* MS Y�ړ����� (�����:- ������:+) */
static BYTE mosset_phase;				/* MS �t�F�[�Y�J�E���^ */


/*
 *	�C���e���W�F���g�}�E�X
 *	������
 */
BOOL FASTCALL mos_init(void)
{
	/* �}�E�X�L���v�`�����~���� */
	mos_port = 1;
	mos_capture = FALSE;

	return TRUE;
}

/*
 *	�C���e���W�F���g�}�E�X
 *	�N���[���A�b�v
 */
void FASTCALL mos_cleanup(void)
{
}

/*
 *	�C���e���W�F���g�}�E�X
 *	���Z�b�g
 */
void FASTCALL mos_reset(void)
{
	/* ���[�N�G���A������ */
	mos_x = 0;
	mos_y = 0;
	mos_phase = 0;
	mos_strobe = FALSE;
	mosset_x = 0;
	mosset_y = 0;
	mosset_phase = 0;
}

/*
 *	�}�E�X�Z�b�g
 *	�P�o�C�g�ǂݍ���
 */
BOOL FASTCALL mouse_readb(WORD addr, BYTE *dat)
{
	if (mos_port != 3) {
		return FALSE;
	}

	switch (addr) {
		/* STATUS REGISTER */
		case 0xfde8 : 
			if (!mos_capture || (mos_port != 3)) {
				*dat = 0x80;
				return TRUE;
			}

			/* �t�F�[�Y�J�E���^�ɏ]���ăf�[�^���쐬 */
			mosset_phase ++;
			switch (mosset_phase) {
				case 1 :	/* �w���ʃj�u�� */
							*dat = (BYTE)(mosset_x & 0x0f);
							break;
				case 2 :	/* �w��ʃj�u�� */
							*dat = (BYTE)((mosset_x >> 4) & 0x0f);
							break;
				case 3 :	/* �x���ʃj�u�� */
							*dat = (BYTE)(mosset_y & 0x0f);
							break;
				case 4 :	/* �x��ʃj�u�� */
							*dat = (BYTE)((mosset_y >> 4) & 0x0f);
							mosset_phase = 0;
							break;
			}

			/* �{�^��������ԃf�[�^������ */
			*dat |= (BYTE)((~mosbtn_request() & 0x30) | 0x80);

			return TRUE;
	}

	return FALSE;
}

/*
 *	�}�E�X�Z�b�g
 *	�P�o�C�g��������
 */
BOOL FASTCALL mouse_writeb(WORD addr, BYTE dat)
{
#if XM7_VER == 1
	if ((fm_subtype == FMSUB_FM8) || (mos_port != 3)) {
#else
	if (mos_port != 3) {
#endif
		return FALSE;
	}

	switch (addr) {
		/* CONTROL REGISTER */
		case 0xfde8: 
			/* bit0�̋@�\���悭�킩��Ȃ���Ńi�j�Q�Ɏ蔲�� */
			if (dat & 0x03) {
				/* �t�F�[�Y�J�E���^���Z�b�g */
				mosset_phase = 0;

				if (mos_capture && (mos_port == 3)) {
					/* �ړ������擾 */
					mospos_request(&mosset_x, &mosset_y);

					/* �������] */
					mosset_x = (BYTE)-mosset_x;
					mosset_y = (BYTE)-mosset_y;
				}
			}
			return TRUE;
	}

	return FALSE;
}

/*
 *	�C���e���W�F���g�}�E�X
 *	�^�C���A�E�g����
 */
static BOOL FASTCALL mos_timeout(void)
{
	/* �^�C���A�E�g�C�x���g���폜 */
	schedule_delevent(EVENT_MOUSE);

	/* �X�g���[�u�M���E�t�F�[�Y�J�E���^�����Z�b�g */
	mos_phase = 0;
	mos_strobe = FALSE;

	return TRUE;
}

/*
 *	�C���e���W�F���g�}�E�X
 *	�X�g���[�u�M������
 */
void FASTCALL mos_strobe_signal(BOOL strb)
{
	/* �X�g���[�u�M���̏�Ԃ��ω��������`�F�b�N */
	if (strb != mos_strobe) {
		/* �X�g���[�u�M���̏�Ԃ�ۑ� */
		mos_strobe = strb;

		if (mos_phase == 0) {
			/* �t�F�[�Y0�̎��Ɉړ���������荞�� */
			mospos_request(&mos_x, &mos_y);

			/* �^�C���A�E�g�C�x���g�̓o�^ */
			schedule_setevent(EVENT_MOUSE, 2000 , mos_timeout);
		}

		/* �t�F�[�Y�J�E���^���X�V */
		mos_phase = (BYTE)((mos_phase + 1) & 0x03);
	}
}

/*
 *	�C���e���W�F���g�}�E�X
 *	�f�[�^�ǂݍ���
 */
BYTE FASTCALL mos_readdata(BYTE trigger)
{
	BYTE ret;

	/* �t�F�[�Y�J�E���^�ɏ]���ăf�[�^���쐬 */
	switch (mos_phase) {
		case 1 :	/* �w��ʃj�u�� */
					ret = (BYTE)((mos_x >> 4) & 0x0f);
					break;
		case 2 :	/* �w���ʃj�u�� */
					ret = (BYTE)(mos_x & 0x0f);
					break;
		case 3 :	/* �x��ʃj�u�� */
					ret = (BYTE)((mos_y >> 4) & 0x0f);
					break;
		case 0 :	/* �x���ʃj�u�� */
					ret = (BYTE)(mos_y & 0x0f);
					break;
	}

	/* �{�^��������ԃf�[�^������ */
	ret |= (BYTE)((mosbtn_request() & (trigger << 4)) & 0x30);

	return ret;
}

/*
 *	�C���e���W�F���g�}�E�X/�}�E�X�Z�b�g
 *	�Z�[�u
 */
BOOL FASTCALL mos_save(int fileh)
{
	if (!file_byte_write(fileh, mos_x)) {
		return FALSE;
	}
	if (!file_byte_write(fileh, mos_y)) {
		return FALSE;
	}
	if (!file_byte_write(fileh, mos_phase)) {
		return FALSE;
	}
	if (!file_bool_write(fileh, mos_strobe)) {
		return FALSE;
	}
	if (!file_byte_write(fileh, mosset_x)) {
		return FALSE;
	}
	if (!file_byte_write(fileh, mosset_y)) {
		return FALSE;
	}
	if (!file_byte_write(fileh, mosset_phase)) {
		return FALSE;
	}

	return TRUE;
}

/*
 *	�C���e���W�F���g�}�E�X/�}�E�X�Z�b�g
 *	���[�h
 */
BOOL FASTCALL mos_load(int fileh, int ver)
{
	/* �o�[�W�����`�F�b�N */
	if (ver < 200) {
		return FALSE;
	}

	/* �������񃊃Z�b�g���� */
	mos_reset();

#if XM7_VER >= 3
	if ((ver >= 900) || ((ver >= 700) && (ver <= 799))) {
#elif XM7_VER >= 2
	if (ver >= 700) {
#else
	if ((ver >= 302) && (ver <= 399)) {
#endif
		if (!file_byte_read(fileh, &mos_x)) {
			return FALSE;
		}
		if (!file_byte_read(fileh, &mos_y)) {
			return FALSE;
		}
		if (!file_byte_read(fileh, &mos_phase)) {
			return FALSE;
		}
		if (!file_bool_read(fileh, &mos_strobe)) {
			return FALSE;
		}
	}
	/* Ver3.09/7.19/9.19�ǉ� */
#if XM7_VER == 1
	if (ver >= 309) {
#elif XM7_VER == 2
	if ((ver >= 719) && (ver <= 799)) {
#else
	if (((ver >= 719) && (ver <= 799)) || (ver >= 919)) {
#endif
		if (!file_byte_read(fileh, &mosset_x)) {
			return FALSE;
		}
		if (!file_byte_read(fileh, &mosset_y)) {
			return FALSE;
		}
		if (!file_byte_read(fileh, &mosset_phase)) {
			return FALSE;
		}
	}

	/* �C�x���g */
	schedule_handle(EVENT_MOUSE, mos_timeout);

	return TRUE;
}

#endif	/* MOUSE */
