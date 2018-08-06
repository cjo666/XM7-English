/*
 *	FM-7 EMULATOR "XM7"
 *
 *	Copyright (C) 1999-2017 �o�h�D(Twitter:@xm6_original)
 *	Copyright (C) 2001-2017 Ryu Takegami (Twitter:@RyuTakegami)
 *	line printer support 2010-2013 by Ben.JP
 *
 *	[ �J�Z�b�g�e�[�v���v�����^ ]
 *
 *	RHG����
 *	  2016.06.26		�v�����^���O�o�̓��[�h�ւ̍đΉ�
 */

#include <stdlib.h>
#include <string.h>
#include "xm7.h"
#include "tapelp.h"
#include "mainetc.h"
#include "device.h"
#include "event.h"

/*
 *	�O���[�o�� ���[�N
 */
BOOL tape_in;							/* �e�[�v ���̓f�[�^ */
BOOL tape_out;							/* �e�[�v �o�̓f�[�^ */
BOOL tape_motor;						/* �e�[�v ���[�^ */
BOOL tape_rec;							/* �e�[�v REC�t���O */
BOOL tape_writep;						/* �e�[�v �������݋֎~ */
WORD tape_count;						/* �e�[�v �T�C�N���J�E���^ */
DWORD tape_subcnt;						/* �e�[�v �T�u�J�E���^ */
int tape_fileh;							/* �e�[�v �t�@�C���n���h�� */
DWORD tape_offset;						/* �e�[�v �t�@�C���I�t�Z�b�g */
char tape_fname[256+1];					/* �e�[�v �t�@�C���l�[�� */

WORD tape_incnt;						/* �e�[�v �ǂݍ��݃J�E���^ */
DWORD tape_fsize;						/* �e�[�v �t�@�C���T�C�Y */
BOOL tape_fetch;						/* �e�[�v �f�[�^�t�F�b�`�t���O */
BYTE *tape_savebuf;						/* �e�[�v �������݃o�b�t�@ */
WORD tape_saveptr;						/* �e�[�v �������݃|�C���^ */

BOOL tape_monitor;						/* �e�[�v �e�[�v�����j�^�t���O */
#if defined(FDDSND)
BOOL tape_sound;						/* �e�[�v �����[���o�̓t���O */
#endif

#if defined(LPRINT)
BYTE lp_use;							/* �v�����^ �g�p���[�h */
#endif
BYTE lp_data;							/* �v�����^ �o�̓f�[�^ */
BOOL lp_busy;							/* �v�����^ BUSY�t���O */
BOOL lp_error;							/* �v�����^ �G���[�t���O */
BOOL lp_pe;								/* �v�����^ PE�t���O */
BOOL lp_ackng;							/* �v�����^ ACK�t���O */
BOOL lp_online;							/* �v�����^ �I�����C�� */
BOOL lp_strobe;							/* �v�����^ �X�g���[�u */
int lp_fileh;							/* �v�����^ �t�@�C���n���h�� */

char lp_fname[256+1];					/* �v�����^ �t�@�C���l�[�� */

/*
 *	�X�^�e�B�b�N ���[�N
 */
static BYTE lp_openuse;						/* �v�����^ OPEN���̎g�p���[�h */

/*
 *	�v���g�^�C�v�錾
 */
static void FASTCALL tape_flush(void);	/* �e�[�v�������݃o�b�t�@�t���b�V�� */


/*
 *	�J�Z�b�g�e�[�v���v�����^
 *	������
 */
BOOL FASTCALL tapelp_init(void)
{
	/* ���[�N�G���A������ */
	tape_savebuf = NULL;

	/* �e�[�v */
	tape_fileh = -1;
	tape_fname[0] = '\0';
	tape_offset = 0;
	tape_fsize = 0;
	tape_writep = FALSE;
	tape_fetch = FALSE;
	tape_saveptr = 0;
	tape_monitor = FALSE;
	tape_motor = FALSE;
#if defined(FDDSND)
	tape_sound = FALSE;
#endif

	/* �v�����^ */
	lp_fileh = -1;
#if defined(LPRINT)
	lp_use = LP_EMULATION;
	lp_openuse = LP_EMULATION;
	lp_setfile(LP_TEMPFILENAME);
#else
	lp_fname[0] = '\0';
#endif

	/* �e�[�v�������݃o�b�t�@ */
	tape_savebuf = (BYTE *)malloc(TAPE_SAVEBUFSIZE);
	if (tape_savebuf == NULL) {
		return FALSE;
	}

	return TRUE;
}

/*
 *	�J�Z�b�g�e�[�v���v�����^
 *	�N���[���A�b�v
 */
void FASTCALL tapelp_cleanup(void)
{
	ASSERT(tape_savebuf);

	/* �t�@�C�����J���Ă���΁A���� */
	if (tape_fileh != -1) {
		tape_flush();
		file_close(tape_fileh);
		tape_fileh = -1;
	}

	/* ���[�^OFF */
	tape_motor = FALSE;

	/* �t�@�C�����J���Ă���΁A���� */
#if 1
	lp_setfile(NULL);
#else
	if (lp_fileh != -1) {
#if defined(LPRINT)
		switch (lp_openuse) {
			case LP_EMULATION:
				lp_closefile();
				break;
			case LP_LOG:
				file_close(lp_fileh);
				break;
			case LP_JASTSOUND:
			case LP_DISABLE:
				break;
			default:
				ASSERT(FALSE);
		}
#else
		file_close(lp_fileh);
#endif
		lp_fileh = -1;
	}

#if defined(LPRINT)
	/* ����p�ꎞ�t�@�C���폜 */
	if ((lp_openuse == LP_EMULATION) && (lp_fileh == -1)) {
		lp_removefile();
	}
#endif
#endif

	/* �������r���Ŏ��s�����ꍇ���l�� */
	if (tape_savebuf) {
		free(tape_savebuf);
	}
}

/*
 *	�J�Z�b�g�e�[�v���v�����^
 *	���Z�b�g
 */
void FASTCALL tapelp_reset(void)
{
	/* ���o�̓f�[�^������΃t���b�V������ */
	tape_flush();

#if defined(FDDSND)
	/* �J�Z�b�g���[�^OFF */
	if (tape_motor && tape_sound) {
		wav_notify(SOUND_CMTMOTOROFF);
	}
#endif

	tape_motor = FALSE;
	tape_rec = FALSE;
	tape_count = 0;
	tape_in = TRUE;
	tape_out = FALSE;
	tape_incnt = 0;
	tape_subcnt = 0;
	tape_fetch = FALSE;

	lp_busy = FALSE;
	lp_error = FALSE;
	lp_ackng = TRUE;
	lp_pe = FALSE;
	lp_online = FALSE;
	lp_strobe = FALSE;
}

/*-[ �v�����^ ]-------------------------------------------------------------*/

/*
 *	�v�����^
 *	�f�[�^�o��
 */
static void FASTCALL lp_output(BYTE dat)
{
	/* �I�[�v���`�F�b�N */
	if (lp_fileh == -1) {
		return;
	}

	/* �A�y���h */
#if defined(LPRINT)
	switch (lp_use) {
		case LP_EMULATION:
			lp_writefile(&dat, 1);
			break;
		case LP_LOG:
			file_write(lp_fileh, &dat, 1);
			break;
		case LP_JASTSOUND:
		case LP_DISABLE:
			break;
		default:
			ASSERT(FALSE);
	}
#else
	file_write(lp_fileh, &dat, 1);
#endif
}

/*
 *	�v�����^
 *	�t�@�C�����ݒ�
 */
void FASTCALL lp_setfile(char *fname)
{
	/* ��x�J���Ă���΁A���� */
	if (lp_fileh != -1) {
#if defined(LPRINT)
		switch (lp_openuse) {
			case LP_EMULATION:
				lp_closefile();
				break;
			case LP_LOG:
				file_close(lp_fileh);
				break;
			case LP_JASTSOUND:
			case LP_DISABLE:
				break;
			default:
				break; /* ASSERT(FALSE); */
		}
#else
		file_close(lp_fileh);
#endif
		lp_fileh = -1;
	}

#if defined(LPRINT)
	/* ����p�ꎞ�t�@�C���폜 */
	if ((lp_openuse == LP_EMULATION) && (lp_fileh == -1)) {
		lp_removefile();
	}
#endif

	/* �t�@�C�����Z�b�g */
	if (fname == NULL) {
		lp_fname[0] = '\0';
		return;
	}
	strcpy(lp_fname, fname);

	/* �I�[�v�����Ă��Ȃ���΁C�J�� */
	if (lp_fileh == -1) {
		if (lp_fname[0] != '\0') {
#if defined(LPRINT)
			switch (lp_use) {
				case LP_EMULATION:
					lp_fileh = lp_openfile(lp_fname);
					break;
				case LP_LOG:
					lp_fileh = file_open(lp_fname, OPEN_W);
					break;
				case LP_JASTSOUND:
				case LP_DISABLE:
					break;
				default:
					ASSERT(FALSE);
			}
#else
			lp_fileh = file_open(lp_fname, OPEN_W);
#endif
		}
	}

	lp_openuse = lp_use;
}

#if defined(LPRINT)
/*
 *	�v�����^
 *	���
 */
void FASTCALL lp_print(void)
{
	if ((lp_use == LP_EMULATION) && (lp_fileh != -1)) {
		lp_printfile();
	}
}
#endif

/*-[ �e�[�v ]---------------------------------------------------------------*/

/*
 *	�e�[�v
 *	�������݃o�b�t�@�̃t���b�V��
 */
static void FASTCALL tape_flush(void)
{
	if (tape_fileh != -1) {
		/* �^����ԂŃo�b�t�@���Ƀf�[�^������΃t�@�C���ɏ����o�� */
		if ((tape_rec) && (tape_saveptr > 0)) {
			file_write(tape_fileh, tape_savebuf, tape_saveptr);
		}
	}

	/* �������݃|�C���^�������� */
	tape_saveptr = 0;
}

/*
 *	�e�[�v
 *	�P�o�C�g��������
 */
static void FASTCALL tape_byte_write(BYTE dat)
{
	/* �o�b�t�@�Ƀf�[�^��ǉ� */
	tape_savebuf[tape_saveptr++] = dat;

	/* �o�b�t�@�������ς��ɂȂ�����f�[�^�������o�� */
	if (tape_saveptr >= TAPE_SAVEBUFSIZE) {
		tape_flush();
	}
}

/*
 *	�e�[�v
 *	�f�[�^����
 */
static void FASTCALL tape_input(BOOL flag)
{
	BYTE high;
	BYTE low;
	WORD dat;

	/* ���[�^������Ă��邩 */
	if (!tape_motor) {
		return;
	}

	/* �^������Ă���Γ��͂ł��Ȃ� */
	if (tape_rec) {
		return;
	}

	/* �{�ԂłȂ��ꍇ�A���Ƀf�[�^�t�F�b�`���Ă���ꍇ�͉������Ȃ� */
	if (!flag && tape_fetch) {
		return;
	}

	/* �V���O���J�E���^�����̓J�E���^���z���Ă���΁A0�ɂ��� */
	while (tape_count >= tape_incnt) {
		tape_count -= tape_incnt;
		tape_incnt = 0;

		/* �f�[�^�t�F�b�` */
		tape_in = FALSE;

		if (tape_fileh == -1) {
			return;
		}

		if (tape_offset >= tape_fsize) {
			return;
		}

		if (!file_seek(tape_fileh, tape_offset)) {
			return;
		}
		if (!file_read(tape_fileh, &high, 1)) {
			return;
		}
		if (!file_read(tape_fileh, &low, 1)) {
			return;
		}

		/* �f�[�^�ݒ� */
		dat = (WORD)(high * 256 + low);
		if (dat > 0x7fff) {
			tape_in = TRUE;
		}

		/* �f�[�^�t�F�b�`�ς݃t���O��ݒ� */
		tape_fetch = !flag;

		/* �{�Ԃ̓��͂łȂ��ꍇ�͂����܂� */
		if (!flag) {
			return;
		}

		/* �J�E���^�ݒ� */
		tape_incnt = (WORD)(dat & 0x7fff);

		/* �J�E���^���J�肷�� */
		if (tape_count > tape_incnt) {
			tape_count -= tape_incnt;
			tape_incnt = 0;
		}
		else {
			tape_incnt -= tape_count;
			tape_count = 0;
		}

		/* �I�t�Z�b�g�X�V */
		tape_offset += 2;
	}
}

/*
 *	�e�[�v
 *	�f�[�^�o��
 */
static void FASTCALL tape_output(BOOL flag)
{
	WORD dat;
	BYTE high, low;

	/* �e�[�v������Ă��邩 */
	if (!tape_motor) {
		return;
	}

	/* �^������ */
	if (!tape_rec) {
		return;
	}

	/* �J�E���^������Ă��邩 */
	if (tape_count == 0) {
		return;
	}

	/* �������݉\�� */
	if (tape_writep) {
		return;
	}

	/* �t�@�C�����I�[�v������Ă���΁A�f�[�^�������� */
	dat = tape_count;
	if (dat >= 0x8000) {
		dat = 0x7fff;
	}
	if (flag) {
		dat |= 0x8000;
	}
	high = (BYTE)(dat >> 8);
	low = (BYTE)(dat & 0xff);
	if (tape_fileh != -1) {
		tape_byte_write(high);
		tape_byte_write(low);

		tape_offset += 2;
		if (tape_offset >= tape_fsize) {
			tape_fsize = tape_offset;
		}
	}

	/* �J�E���^�����Z�b�g */
	tape_count = 0;
	tape_subcnt = 0;
}

/*
 *	�e�[�v
 *	�}�[�J�o��
 */
static void FASTCALL tape_mark(void)
{
	/* �e�[�v������Ă��邩 */
	if (!tape_motor) {
		return;
	}

	/* �^������ */
	if (!tape_rec) {
		return;
	}

	/* �������݉\�� */
	if (tape_writep) {
		return;
	}

	/* �t�@�C�����I�[�v������Ă���΁A�f�[�^�������� */
	if (tape_fileh != -1) {
		tape_byte_write(0);
		tape_byte_write(0);

		tape_offset += 2;
		if (tape_offset >= tape_fsize) {
			tape_fsize = tape_offset;
		}
	}
}

/*
 *	�e�[�v
 *	�����߂�
 */
void FASTCALL tape_rew(void)
{
	WORD dat;

	/* �������� */
	if (tape_fileh == -1) {
		return;
	}

	/* assert */
	ASSERT(tape_fsize >= 16);
	ASSERT(tape_offset >= 16);
	ASSERT(!(tape_fsize & 0x01));
	ASSERT(!(tape_offset & 0x01));

	/* �^�����Ȃ炢������t���b�V�� */
	tape_flush();

	while (tape_offset > 16) {
		/* �Q�o�C�g�O�ɖ߂�A�ǂݍ��� */
		tape_offset -= 2;
		if (!file_seek(tape_fileh, tape_offset)) {
			return;
		}
		file_read(tape_fileh, (BYTE *)&dat, 2);

		/* $0000�Ȃ�A�����ɐݒ� */
		if (dat == 0) {
			file_seek(tape_fileh, tape_offset);
			return;
		}

		/* ���ܓǂݍ��񂾕������߂� */
		if (!file_seek(tape_fileh, tape_offset)) {
			return;
		}
	}
}

/*
 *	�e�[�v
 *	�ŏ��܂Ŋ����߂�
 */
void FASTCALL tape_rewtop(void)
{
	WORD dat;

	/* �������� */
	if (tape_fileh == -1) {
		return;
	}

	/* assert */
	ASSERT(tape_fsize >= 16);
	ASSERT(tape_offset >= 16);
	ASSERT(!(tape_fsize & 0x01));
	ASSERT(!(tape_offset & 0x01));

	/* �^�����Ȃ炢������t���b�V�� */
	tape_flush();

	/* �����I�ɍŏ��܂Ŋ����߂� */
	tape_offset = 16;
	file_seek(tape_fileh, tape_offset);
}

/*
 *	�e�[�v
 *	������
 */
void FASTCALL tape_ff(void)
{
	WORD dat;

	/* �������� */
	if (tape_fileh == -1) {
		return;
	}

	/* assert */
	ASSERT(tape_fsize >= 16);
	ASSERT(tape_offset >= 16);
	ASSERT(!(tape_fsize & 0x01));
	ASSERT(!(tape_offset & 0x01));

	/* �^�����Ȃ炢������t���b�V�� */
	tape_flush();

	while (tape_offset < tape_fsize) {
		/* ��֐i�߂� */
		tape_offset += 2;
		if (tape_offset >= tape_fsize){
			return;
		}
		if (!file_seek(tape_fileh, tape_offset)) {
			return;
		}
		file_read(tape_fileh, (BYTE *)&dat, 2);

		/* $0000�Ȃ�A���̎��ɐݒ� */
		if (dat == 0) {
			tape_offset += 2;
			if (tape_offset >= tape_fsize) {
				tape_fsize = tape_offset;
			}
			return;
		}
	}
}

/*
 *	�e�[�v
 *	�t�@�C�����ݒ�
 */
void FASTCALL tape_setfile(char *fname)
{
	char *header = "XM7 TAPE IMAGE 0";
	char buf[17];

	/* ��x�J���Ă���΁A���� */
	if (tape_fileh != -1) {
		tape_flush();
		file_close(tape_fileh);
		tape_fileh = -1;
		tape_writep = FALSE;
	}

	/* �t�@�C�����Z�b�g */
	if (fname == NULL) {
		tape_fname[0] = '\0';
	}
	else {
		if (strlen(fname) < sizeof(tape_fname)) {
			strcpy(tape_fname, fname);
		}
		else {
			tape_fname[0] = '\0';
		}
	}

	/* �t�@�C���I�[�v�������݂� */
	if (tape_fname[0] != '\0') {
		tape_fileh = file_open(tape_fname, OPEN_RW);
		if (tape_fileh != -1) {
			tape_writep = FALSE;
		}
		else {
			tape_fileh = file_open(tape_fname, OPEN_R);
			tape_writep = TRUE;
		}
	}

	/* �J���Ă���΁A�w�b�_��ǂݍ��݃`�F�b�N */
	if (tape_fileh != -1) {
		memset(buf, 0, sizeof(buf));
		file_read(tape_fileh, (BYTE*)buf, 16);
		if (strcmp(buf, header) != 0) {
			file_close(tape_fileh);
			tape_fileh = -1;
			tape_writep = FALSE;
		}
	}

	/* �t���O�̏��� */
	tape_setrec(FALSE);
	tape_count = 0;
	tape_incnt = 0;
	tape_subcnt = 0;

	/* �t�@�C�����J���Ă���΁A�t�@�C���T�C�Y�A�I�t�Z�b�g������ */
	if (tape_fileh != -1) {
		tape_fsize = file_getsize(tape_fileh);
		tape_offset = 16;
	}
}

/*
 *	�e�[�v
 *	�^���t���O�ݒ�
 */
void FASTCALL tape_setrec(BOOL flag)
{
	/* ���[�^������Ă���΁A�}�[�J���������� */
	if (tape_motor && !tape_rec) {
		if (flag) {
			tape_rec = TRUE;
			tape_mark();
			return;
		}
	}
	else {
		/* �^���I���Ȃ�A�������݃o�b�t�@���t���b�V�� */
		if (tape_motor && tape_rec) {
			if (!flag) {
				tape_flush();
			}
		}
	}

	tape_rec = flag;
}

/*
 *	�e�[�v
 *	�T�E���h����
 */
static BOOL FASTCALL tape_outsnd(void)
{
	if (tape_motor) {
		if (tape_rec) {
			/* �^�� */
			if (!tape_writep) {
				tape_notify(tape_out);
			}
		}
		else {
			/* �Đ� */
			tape_input(FALSE);
			tape_notify(tape_in);
		}
	}

	return TRUE;
}

/*-[ ������R/W ]------------------------------------------------------------*/

/*
 *	�J�Z�b�g�e�[�v���v�����^
 *	�P�o�C�g�ǂݏo��
 */
BOOL FASTCALL tapelp_readb(WORD addr, BYTE *dat)
{
	BYTE ret;
	BYTE joy;

	/* �A�h���X�`�F�b�N */
	if (addr != 0xfd02) {
		return FALSE;
	}

	/* �v�����^ �X�e�[�^�X�쐬 */
	ret = 0x70;
	if (lp_busy) {
		ret |= 0x01;
	}
	if (!lp_error) {
		ret |= 0x02;
	}
	if (!lp_ackng) {
		ret |= 0x04;
	}
	if (lp_pe) {
		ret |= 0x08;
	}

	/* �v�����^���ڑ��Ȃ�A�d�g�V���ЃW���C�X�e�B�b�N */
#if defined(LPRINT)
	if (lp_use == LP_DISABLE) {
#else
	if ((lp_fileh == -1) || (lp_fname[0] == '\0')) {
#endif
		/* �������A�擾 */
		ret |= 0x0f;
		joy = joy_request(2);

		/* �E */
		if (!(lp_data & 0x01) && (joy & 0x08)) {
			ret &= ~0x08;
		}
		/* �� */
		if (!(lp_data & 0x02) && (joy & 0x04)) {
			ret &= ~0x08;
		}
		/* �� */
		if (!(lp_data & 0x04) && (joy & 0x01)) {
			ret &= ~0x08;
		}
		/* �� */
		if (!(lp_data & 0x08) && (joy & 0x02)) {
			ret &= ~0x08;
		}
		/* J2 */
		if (!(lp_data & 0x10) && (joy & 0x20)) {
			ret &= ~0x08;
		}
		/* J1 */
		if (!(lp_data & 0x20) && (joy & 0x10)) {
			ret &= ~0x08;
		}
	}

	/* �J�Z�b�g �f�[�^�쐬 */
	tape_input(TRUE);
	if (tape_in) {
		ret |= 0x80;
	}

	/* ok */
	*dat = ret;
	return TRUE;
}

/*
 *	�J�Z�b�g�e�[�v���v�����^
 *	�P�o�C�g��������
 */
BOOL FASTCALL tapelp_writeb(WORD addr, BYTE dat)
{
	switch (addr) {
		/* �J�Z�b�g����A�v�����^���� */
		case 0xfd00:
			/* �v�����^ �I�����C�� */
			if (dat & 0x80) {
				lp_online = FALSE;
			}
			else {
				lp_online = TRUE;
			}

			/* �v�����^ �X�g���[�u */
			if (dat & 0x40) {
				lp_strobe = TRUE;
			}
			else {
				if (lp_strobe && lp_online) {
					lp_output(lp_data);
					mainetc_lp();
				}
				lp_strobe = FALSE;
			}

			/* �e�[�v �o�̓f�[�^ */
			if (dat & 0x01) {
				if (!tape_out) {
					tape_output(FALSE);
				}
				tape_out = TRUE;
			}
			else {
				if (tape_out) {
					tape_output(TRUE);
				}
				tape_out = FALSE;
			}

			/* �e�[�v ���[�^ */
			if (dat & 0x02) {
				if (!tape_motor) {
					/* �V�K�X�^�[�g */
					tape_count = 0;
					tape_subcnt = 0;
					tape_motor = TRUE;
					if (tape_rec) {
						tape_mark();
					}
#if defined(FDDSND)
					if (tape_sound) {
						wav_notify(SOUND_CMTMOTORON);
					}
#endif
					schedule_setevent(EVENT_TAPEMON, 40, tape_outsnd);
				}
			}
			else {
#if defined(FDDSND)
				if (tape_motor && tape_sound) {
					wav_notify(SOUND_CMTMOTOROFF);
				}
#endif
				schedule_delevent(EVENT_TAPEMON);

				/* ���[�^��~ */
				tape_motor = FALSE;
				tape_flush();
			}

			return TRUE;

		/* �v�����^�o�̓f�[�^ */
		case 0xfd01:
#if defined(LPRINT) && defined(JASTSOUND)
			/* �W���X�g�T�E���h�o��(strobe�M���͍l�����Ȃ�) */
			if (lp_use == LP_JASTSOUND) {
				dac_notify(dat);
			}
#endif

			lp_data = dat;
			return TRUE;
	}

	return FALSE;
}

/*
 *	�J�Z�b�g�e�[�v���v�����^
 *	�Z�[�u
 */
BOOL FASTCALL tapelp_save(int fileh)
{
	BOOL tmp;

	/* �X�e�[�g�Z�[�u�O�ɏ������݃o�b�t�@���t���b�V�� */
	tape_flush();

	if (!file_bool_write(fileh, tape_in)) {
		return FALSE;
	}
	if (!file_bool_write(fileh, tape_out)) {
		return FALSE;
	}
	if (!file_bool_write(fileh, tape_motor)) {
		return FALSE;
	}
	if (!file_bool_write(fileh, tape_rec)) {
		return FALSE;
	}
	if (!file_bool_write(fileh, tape_writep)) {
		return FALSE;
	}
	if (!file_word_write(fileh, tape_count)) {
		return FALSE;
	}
	if (!file_dword_write(fileh, tape_subcnt)) {
		return FALSE;
	}

	if (!file_dword_write(fileh, tape_offset)) {
		return FALSE;
	}
	if (!file_write(fileh, (BYTE*)tape_fname, 256 + 1)) {
		return FALSE;
	}

	tmp = (tape_fileh != -1);
	if (!file_bool_write(fileh, tmp)) {
		return FALSE;
	}

	if (!file_word_write(fileh, tape_incnt)) {
		return FALSE;
	}
	if (!file_dword_write(fileh, tape_fsize)) {
		return FALSE;
	}

	if (!file_byte_write(fileh, lp_data)) {
		return FALSE;
	}
	if (!file_bool_write(fileh, lp_busy)) {
		return FALSE;
	}
	if (!file_bool_write(fileh, lp_error)) {
		return FALSE;
	}
	if (!file_bool_write(fileh, lp_pe)) {
		return FALSE;
	}
	if (!file_bool_write(fileh, lp_ackng)) {
		return FALSE;
	}
	if (!file_bool_write(fileh, lp_online)) {
		return FALSE;
	}
	if (!file_bool_write(fileh, lp_strobe)) {
		return FALSE;
	}

	if (!file_write(fileh, (BYTE*)lp_fname, 256 + 1)) {
		return FALSE;
	}
	tmp = (lp_fileh != -1);
	if (!file_bool_write(fileh, tmp)) {
		return FALSE;
	}

	return TRUE;
}

/*
 *	�J�Z�b�g�e�[�v���v�����^
 *	���[�h
 */
BOOL FASTCALL tapelp_load(int fileh, int ver)
{
	DWORD offset;
	char fname[256 + 1];
#if XM7_VER >= 2
	BYTE tmp;
#endif
	BOOL flag;
	int pathlen;

	/* �o�[�W�����`�F�b�N */
	if (ver < 200) {
		return FALSE;
	}

	/* �t�@�C�����̍ő啶���������� */
#if XM7_VER >= 3
	if (((ver >= 715) && (ver <= 799)) || (ver >= 915)) {
#elif XM7_VER >= 2
	if ((ver >= 715) && (ver <= 799)) {
#else
	if ((ver >= 305) && (ver <= 499)) {
#endif
		pathlen = 256;
	}
	else {
		pathlen = 128;
	}

	if (!file_bool_read(fileh, &tape_in)) {
		return FALSE;
	}
	if (!file_bool_read(fileh, &tape_out)) {
		return FALSE;
	}
	if (!file_bool_read(fileh, &tape_motor)) {
		return FALSE;
	}
	if (!file_bool_read(fileh, &tape_rec)) {
		return FALSE;
	}
	if (!file_bool_read(fileh, &tape_writep)) {
		return FALSE;
	}
	if (!file_word_read(fileh, &tape_count)) {
		return FALSE;
	}
#if XM7_VER >= 2
#if XM7_VER >= 3
	if ((ver >= 906) || ((ver >= 706) && (ver <= 799))) {
#else
	if ((ver >= 706) && (ver <= 799)) {
#endif
		if (!file_dword_read(fileh, &tape_subcnt)) {
			return FALSE;
		}
	}
	else {
		if (!file_byte_read(fileh, &tmp)) {
			return FALSE;
		}
		tape_subcnt = (tmp << 4);
	}
#else
	if (!file_dword_read(fileh, &tape_subcnt)) {
		return FALSE;
	}
#endif

	if (!file_dword_read(fileh, &offset)) {
		return FALSE;
	}
	if (!file_read(fileh, (BYTE*)fname, pathlen + 1)) {
		return FALSE;
	}
	if (!file_bool_read(fileh, &flag)) {
		return FALSE;
	}

	/* �}�E���g */
	tape_setfile(NULL);
	if (flag) {
		tape_setfile(fname);
		if ((tape_fileh != -1) && ((tape_fsize + 1) >= offset)) {
			file_seek(tape_fileh, offset);
			tape_offset = offset;
		}
	}

	if (!file_word_read(fileh, &tape_incnt)) {
		return FALSE;
	}
	/* tape_fsize�͖��� */
	if (!file_dword_read(fileh, &offset)) {
		return FALSE;
	}

	if (!file_byte_read(fileh, &lp_data)) {
		return FALSE;
	}
	if (!file_bool_read(fileh, &lp_busy)) {
		return FALSE;
	}
	if (!file_bool_read(fileh, &lp_error)) {
		return FALSE;
	}
	if (!file_bool_read(fileh, &lp_pe)) {
		return FALSE;
	}
	if (!file_bool_read(fileh, &lp_ackng)) {
		return FALSE;
	}
	if (!file_bool_read(fileh, &lp_online)) {
		return FALSE;
	}
	if (!file_bool_read(fileh, &lp_strobe)) {
		return FALSE;
	}

	if (!file_read(fileh, (BYTE*)fname, pathlen + 1)) {
		return FALSE;
	}
#if XM7_VER >= 3
	if (((ver >= 720) && (ver <= 799)) || (ver >= 920)) {
#elif XM7_VER >= 2
	if ((ver >= 720) && (ver <= 799)) {
#else
	if ((ver >= 310) && (ver <= 499)) {
#endif
		if (!file_bool_read(fileh, &flag)) {
			return FALSE;
		}

		/* �}�E���g */
		lp_setfile(NULL);
		if (flag) {
			lp_setfile(fname);
		}
	}

	schedule_handle(EVENT_TAPEMON, tape_outsnd);

	/* ���̑��̃��[�N�G���A�������� */
	tape_saveptr = 0;
	tape_fetch = FALSE;

	return TRUE;
}
