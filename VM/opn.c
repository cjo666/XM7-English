/*
 *	FM-7 EMULATOR "XM7"
 *
 *	Copyright (C) 1999-2017 �o�h�D(Twitter:@xm6_original)
 *	Copyright (C) 2001-2017 Ryu Takegami (Twitter:@RyuTakegami)
 *
 *	[ OPN/WHG/THG (YM2203) ]
 *
 *	RHG����
 *	  2001.11.19		���Z�b�g���E�X�e�[�g�t�@�C�����[�h�����̃L�[�I�t��
 *						FM�����f�B�X�v���C�ɔ��f����Ȃ������C��
 *	  2002.03.22		�^�C�}�̃C�x���g�ݒ�^�C�~���O���C��
 *	  2002.06.15		FM-7���[�h���̓Ɨ�PSG�G�~�����[�V�����Ή��ɔ�����PSG
 *						I/O�A�N�Z�X���̃o�[�W�����`�F�b�N��ǉ�
 *	  2002.06.29		�v���X�P�[���ݒ莞�ɃR�}���h���W�X�^���f�[�^���W�X�^��
 *						���ŃA�N�Z�X����Ɛ������ݒ肪�s���Ȃ������C��
 *	  2002.08.18		FM-7���[�h����OPN�������@�\��ǉ�
 *	  2002.08.23		OPN�������@�\�֌W����������������
 *	  2003.06.19		�^�C�}�̋������ꕔ�C��
 *	  2003.07.18		�^�C�}�̋������ďC��
 *	  2004.04.22		�^�C�}�̋������āX�C��(��
 *	  2004.08.13		�\�[�X�Ǘ���������̂���OPN�n�̃\�[�X�𓝍�
 *						(����Ɍ����ڂ���≘���Ȃ����c)
 *	  2005.05.13		PSG���[�h���̃r�b�g�}�X�N��ǉ� (from MS�~PLAYer...)
 *	  2010.06.18		FM-8���[�h���ɕW��OPN���L�������������C��
 *	  2012.04.20		�ꉞOPN�g�p�t���O�������B����ł͓��ɈӖ��͂���܂���
 *						OPN�L���t���O/OPN�g�p�t���O���X�e�[�g�f�[�^�ɕۑ�/���A
 *						����悤�ɂ����BWHG/THG�ƃf�[�^�\�������킹�邽�߂ɃC
 *						���M�����[�ȏ��ԂƂȂ��Ă��܂��B
 */

#include <string.h>
#include "xm7.h"
#include "opn.h"
#include "device.h"
#include "mainetc.h"
#include "event.h"
#include "mouse.h"

/*
 *	�O���[�o�� ���[�N
 */
BOOL opn_enable;						/* OPN�L���E�����t���O(7 only) */
BOOL opn_use;							/* OPN�g�p�t���O */
BOOL whg_enable;						/* WHG�L���E�����t���O */
BOOL whg_use;							/* WHG�g�p�t���O */
BOOL thg_enable;						/* THG�L���E�����t���O */
BOOL thg_use;							/* THG�g�p�t���O */
#if XM7_VER == 1
BOOL fmx_flag;							/* FM-X PSG���[�h�t���O */
BOOL fmx_use;							/* FM-X PSG �g�p�t���O */
#endif

BYTE opn_reg[3][256];					/* OPN���W�X�^ */
BOOL opn_key[3][4];						/* OPN�L�[�I���t���O */
BOOL opn_timera[3];						/* �^�C�}�[A����t���O */
BOOL opn_timerb[3];						/* �^�C�}�[B����t���O */
DWORD opn_timera_tick[3];				/* �^�C�}�[A�Ԋu */
DWORD opn_timerb_tick[3];				/* �^�C�}�[B�Ԋu */
BYTE opn_scale[3];						/* �v���X�P�[�� */

/*
 *	�X�^�e�B�b�N ���[�N
 */
static BYTE opn_pstate[3];				/* �|�[�g��� */
static BYTE opn_selreg[3];				/* �Z���N�g���W�X�^ */
static BYTE opn_seldat[3];				/* �Z���N�g�f�[�^ */
static BOOL opn_timera_int[3];			/* �^�C�}�[A�I�[�o�[�t���[ */
static BOOL opn_timerb_int[3];			/* �^�C�}�[B�I�[�o�[�t���[ */
static BOOL opn_timera_en[3];			/* �^�C�}�[A�C�l�[�u�� */
static BOOL opn_timerb_en[3];			/* �^�C�}�[B�C�l�[�u�� */

/*
 *	�v���g�^�C�v�錾
 */
static void FASTCALL opn_notify_no(int no, BYTE reg, BYTE dat);
static void FASTCALL opn_cleanup_common(int no);
static void FASTCALL opn_reset_common(int no);

static BOOL FASTCALL opn_timera_event(void);
static BOOL FASTCALL whg_timera_event(void);
static BOOL FASTCALL thg_timera_event(void);
static BOOL FASTCALL opn_timerb_event(void);
static BOOL FASTCALL whg_timerb_event(void);
static BOOL FASTCALL thg_timerb_event(void);
static void FASTCALL opn_timera_calc(int no);
static void FASTCALL opn_timerb_calc(int no);


/*
 *	OPN
 *	������
 */
BOOL FASTCALL opn_init(void)
{
	memset(opn_reg, 0, sizeof(opn_reg));

	/* �f�t�H���g��OPN�L�� */
	opn_enable = TRUE;
#if XM7_VER == 1
	opn_use = FALSE;
	fmx_flag = FALSE;
	fmx_use = FALSE;
#else
	if (fm7_ver >= 2) {
		opn_use = TRUE;
	}
	else {
		opn_use = FALSE;
	}
#endif

	return TRUE;
}

/*
 *	OPN
 *	�N���[���A�b�v
 */
void FASTCALL opn_cleanup(void)
{
	opn_cleanup_common(OPN_STD);
}

/*
 *	OPN
 *	���Z�b�g
 */
void FASTCALL opn_reset(void)
{
	/* ���샂�[�h�ɂ���Ďg�p�t���O��ݒ肷�� */
#if XM7_VER == 1
	opn_use = FALSE;
	fmx_use = FALSE;
#else
	if (fm7_ver >= 2) {
		opn_use = TRUE;
	}
	else {
		opn_use = FALSE;
	}
#endif

	/* ���Z�b�g */
	opn_reset_common(OPN_STD);
}

/*
 *	WHG
 *	������
 */
BOOL FASTCALL whg_init(void)
{
	/* WHG�L�� */
	whg_enable = TRUE;
	whg_use = FALSE;

	return TRUE;
}

/*
 *	WHG
 *	�N���[���A�b�v
 */
void FASTCALL whg_cleanup(void)
{
	opn_cleanup_common(OPN_WHG);
}

/*
 *	WHG
 *	���Z�b�g
 */
void FASTCALL whg_reset(void)
{
	/* �g�p�t���O�������� */
	whg_use = FALSE;

	/* ���Z�b�g */
	opn_reset_common(OPN_WHG);
}

/*
 *	THG
 *	������
 */
BOOL FASTCALL thg_init(void)
{
	/* THG�L�� */
	thg_enable = FALSE;
	thg_use = FALSE;

	return TRUE;
}

/*
 *	THG
 *	�N���[���A�b�v
 */
void FASTCALL thg_cleanup(void)
{
	opn_cleanup_common(OPN_THG);
}

/*
 *	THG
 *	���Z�b�g
 */
void FASTCALL thg_reset(void)
{
	/* �g�p�t���O�������� */
	thg_use = FALSE;

	/* ���Z�b�g */
	opn_reset_common(OPN_THG);
}

/*-[ ���ʏ���(Reset/Cleanup) ]-----------------------------------------------*/

/*
 *	OPN
 *	�N���[���A�b�v���ʏ���
 */
static void FASTCALL opn_cleanup_common(int no)
{
	BYTE i;

	/* PSG */
	for (i=0; i<6; i++) {
		opn_notify_no(no, i, 0);
	}
	opn_notify_no(no, 7, 0xff);

	/* TL=$7F */
	for (i=0x40; i<0x50; i++) {
		if ((i & 0x03) == 3) {
			continue;
		}
		opn_notify_no(no, i, 0x7f);
	}

	/* �L�[�I�t */
	for (i=0; i<3; i++) {
		opn_notify_no(no, 0x28, i);
		opn_key[no][i] = FALSE;
	}
}

/*
 *	OPN
 *	���Z�b�g���ʏ���
 */
static void FASTCALL opn_reset_common(int no)
{
	BYTE i;

	/* ���W�X�^�N���A */
	memset(opn_reg[no], 0, sizeof(opn_reg[no]));

	/* �^�C�}�[OFF */
	opn_timera[no] = FALSE;
	opn_timerb[no] = FALSE;

	/* I/O������ */
	opn_pstate[no] = OPN_INACTIVE;
	opn_selreg[no] = 0;
	opn_seldat[no] = 0;

	/* �f�o�C�X */
	opn_timera_int[no] = FALSE;
	opn_timerb_int[no] = FALSE;
	opn_timera_tick[no] = 0;
	opn_timerb_tick[no] = 0;
	opn_timera_en[no] = FALSE;
	opn_timerb_en[no] = FALSE;
	opn_scale[no] = 3;

	/* PSG������ */
	for (i=0; i<14;i++) {
		if (i == 7) {
			opn_notify_no(no, i, 0xff);
			opn_reg[no][i] = 0xff;
		}
		else {
			opn_notify_no(no, i, 0);
		}
	}

	/* MUL,DT */
	for (i=0x30; i<0x40; i++) {
		if ((i & 0x03) == 3) {
			continue;
		}
		opn_notify_no(no, i, 0);
	}

	/* TL=$7F */
	for (i=0x40; i<0x50; i++) {
		if ((i & 0x03) == 3) {
			continue;
		}
		opn_notify_no(no, i, 0x7f);
		opn_reg[no][i] = 0x7f;
	}

	/* AR=$1F */
	for (i=0x50; i<0x60; i++) {
		if ((i & 0x03) == 3) {
			continue;
		}
		opn_notify_no(no, i, 0x1f);
		opn_reg[no][i] = 0x1f;
	}

	/* ���̑� */
	for (i=0x60; i<0xb4; i++) {
		if ((i & 0x03) == 3) {
			continue;
		}
		opn_notify_no(no, i, 0);
	}

	/* SL,RR */
	for (i=0x80; i<0x90; i++) {
		if ((i & 0x03) == 3) {
			continue;
		}
		opn_notify_no(no, i, 0xff);
		opn_reg[no][i] = 0xff;
	}

	/* �L�[�I�t */
	for (i=0; i<3; i++) {
		opn_notify_no(no, 0x28, i);
		opn_key[no][i] = FALSE;
	}

	/* ���[�h */
	opn_notify_no(no, 0x27, 0);
}

/*-[ ���ʏ���(VM I/F) ]------------------------------------------------------*/

/*
 *	OPN
 *	���W�X�^�������ݒʒm
 */
static void FASTCALL opn_notify_no(int no, BYTE reg, BYTE dat)
{
	/* �e�����̒ʒm�֐����ĂԂ��� */
	switch (no) {
		case OPN_STD:
			opn_notify(reg, dat);
			break;
		case OPN_WHG:
			whg_notify(reg, dat);
			break;
		case OPN_THG:
			thg_notify(reg, dat);
			break;
		default:
			ASSERT(FALSE);
			break;
	}
}

/*-[ ���ʏ���(�^�C�}�[) ]----------------------------------------------------*/

/*
 *	OPN
 *	IRQ�t���O���ʏ���
 */
static void FASTCALL opn_set_irq_flag(int no, BOOL flag)
{
	switch (no) {
		case OPN_STD:
			opn_irq_flag = flag;
			break;
		case OPN_WHG:
			whg_irq_flag = flag;
			break;
		case OPN_THG:
			thg_irq_flag = flag;
			break;
		default:
			ASSERT(FALSE);
			break;
	}

#if XM7_VER == 1
	if (fm_subtype == FMSUB_FM8) {
		/* ���肦�Ȃ��� */
		return;
	}
#endif
	maincpu_irq();
}

/*
 *	OPN
 *	�^�C�}�[A�I�[�o�t���[���ʏ���
 */
static BOOL FASTCALL opn_timera_event_main(int no)
{
	ASSERT ((no >= 0) && (no <= 2));

	/* �C�l�[�u���� */
	if (opn_timera_en[no]) {
		/* �I�[�o�[�t���[�A�N�V�������L���� */
		if (opn_timera[no]) {
			opn_timera[no] = FALSE;
			opn_timera_int[no] = TRUE;

			/* ���荞�݂������� */
			opn_set_irq_flag(no, TRUE);
		}

		/* CSM�����������[�h�ł̃L�[�I�� */
		opn_notify_no(no, 0xff, 0);
	}

	/* �^�C�}�[�Đݒ� */
	opn_timera_calc(no);

	/* �^�C�}�[�͉񂵑����� */
	return TRUE;
}

/*
 *	OPN
 *	�^�C�}�[B�I�[�o�t���[���ʏ���
 */
static BOOL FASTCALL opn_timerb_event_main(int no)
{
	ASSERT ((no >= 0) && (no <= 2));

	/* �C�l�[�u���� */
	if (opn_timerb_en[no]) {
		/* �I�[�o�[�t���[�A�N�V�������L���� */
		if (opn_timerb[no]) {
			/* �t���O�ύX */
			opn_timerb[no] = FALSE;
			opn_timerb_int[no] = TRUE;

			/* ���荞�݂������� */
			opn_set_irq_flag(no, TRUE);
		}
	}

	/* �^�C�}�[�Đݒ� */
	opn_timerb_calc(no);

	/* �^�C�}�[�͉񂵑����� */
	return TRUE;
}

/*
 *	OPN
 *	�^�C�}�[A�C���^�[�o���Z�o���ʏ���
 */
static void FASTCALL opn_timera_calc(int no)
{
	DWORD t;
	BYTE temp;

	ASSERT ((no >= 0) && (no <= 2));

	t = opn_reg[no][0x24];
	t *= 4;
	temp = (BYTE)(opn_reg[no][0x25] & 3);
	t |= temp;
	t &= 0x3ff;
	t = (1024 - t);
	t *= opn_scale[no];
	t *= 12;
	t *= 10000;
	t /= OPN_CLOCK;

	/* �^�C�}�[�l��ݒ� */
	if (t != opn_timera_tick[no]) {
		opn_timera_tick[no] = t;
		switch (no) {
			case OPN_STD:
				schedule_setevent(EVENT_OPN_A, opn_timera_tick[OPN_STD],
					opn_timera_event);
				break;
			case OPN_WHG:
				schedule_setevent(EVENT_WHG_A, opn_timera_tick[OPN_WHG],
					whg_timera_event);
				break;
			case OPN_THG:
				schedule_setevent(EVENT_THG_A, opn_timera_tick[OPN_THG],
					thg_timera_event);
				break;
			default:
				ASSERT(FALSE);
				break;
		}
	}
}

/*
 *	OPN
 *	�^�C�}�[B�C���^�[�o���Z�o���ʏ���
 */
static void FASTCALL opn_timerb_calc(int no)
{
	DWORD t;

	ASSERT ((no >= 0) && (no <= 2));

	t = opn_reg[no][0x26];
	t = (256 - t);
	t *= 192;
	t *= opn_scale[no];
	t *= 10000;
	t /= OPN_CLOCK;

	/* �^�C�}�[�l��ݒ� */
	if (t != opn_timerb_tick[no]) {
		opn_timerb_tick[no] = t;
		switch (no) {
			case OPN_STD:
				schedule_setevent(EVENT_OPN_B, opn_timerb_tick[OPN_STD],
					opn_timerb_event);
				break;
			case OPN_WHG:
				schedule_setevent(EVENT_WHG_B, opn_timerb_tick[OPN_WHG],
					whg_timerb_event);
				break;
			case OPN_THG:
				schedule_setevent(EVENT_THG_B, opn_timerb_tick[OPN_THG],
					thg_timerb_event);
				break;
			default:
				ASSERT(FALSE);
				break;
		}
	}
}

/*-[ ���ʏ���(���W�X�^�A���C) ]----------------------------------------------*/

/*
 *	OPN
 *	���W�X�^�A���C���ǂݏo��
 */
static BYTE FASTCALL opn_readreg(int no, BYTE reg)
{
	/* ���W�X�^�r�b�g�}�X�N�f�[�^(PSG) */
	static const BYTE opn_bitmask[16] = {
		0xff,	0x0f,	0xff,	0x0f,
		0xff,	0x0f,	0x1f,	0xff,
		0x1f,	0x1f,	0x1f,	0xff,
		0xff,	0x0f,	0xff,	0xff,
	};

	ASSERT ((no >= 0) && (no <= 2));

	/* FM�������͓ǂݏo���Ȃ� */
	if (reg >= 0x10) {
		return 0xff;
	}

	return (BYTE)(opn_reg[no][reg] & opn_bitmask[reg]);
}

/*
 *	OPN
 *	���W�X�^�A���C�֏�������
 */
static void FASTCALL opn_writereg(int no, BYTE reg, BYTE dat)
{
#if defined(MOUSE)
	BOOL strobe;
	BYTE mask;
#endif

	ASSERT ((no >= 0) && (no <= 2));

	if (no == OPN_STD) {
		/* �W��OPN�t���O�I�� */
		opn_use = TRUE;
	}

	/* �^�C�}�[���� */
	/* ���̃��W�X�^�͔��ɓ���B�ǂ�������Ȃ��܂܈����Ă���l���唼�ł́H */
	if (reg == 0x27) {
		/* �I�[�o�[�t���[�t���O�̃N���A */
		if (dat & 0x10) {
			opn_timera_int[no] = FALSE;
		}
		if (dat & 0x20) {
			opn_timerb_int[no] = FALSE;
		}

		/* ������������A���荞�݂𗎂Ƃ� */
		if (!opn_timera_int[no] && !opn_timerb_int[no]) {
			opn_set_irq_flag(no, FALSE);
		}

		/* �^�C�}�[A */
		if (dat & 0x01) {
			/* 0��1�Ń^�C�}�[�l�����[�h�A����ȊO�ł��^�C�}�[on */
			if ((opn_reg[no][0x27] & 0x01) == 0) {
				if (!opn_timera[no]) {
					/* �I�[�o�[�t���[�ς݂̏ꍇ�A�Đݒ� */
					opn_timera_tick[no] = 0;
				}
				opn_timera_calc(no);
			}
			opn_timera_en[no] = TRUE;
		}
		else {
			opn_timera_en[no] = FALSE;
		}
		if (dat & 0x04) {
			opn_timera[no] = TRUE;
		}
		else {
			opn_timera[no] = FALSE;
		}

		/* �^�C�}�[B */
		if (dat & 0x02) {
			/* 0��1�Ń^�C�}�[�l�����[�h�A����ȊO�ł��^�C�}�[on */
			if ((opn_reg[no][0x27] & 0x02) == 0) {
				if (!opn_timerb[no]) {
					/* �I�[�o�[�t���[�ς݂̏ꍇ�A�Đݒ� */
					opn_timerb_tick[no] = 0;
				}
				opn_timerb_calc(no);
			}
			opn_timerb_en[no] = TRUE;
		}
		else {
			opn_timerb_en[no] = FALSE;
		}
		if (dat & 0x08) {
			opn_timerb[no] = TRUE;
		}
		else {
			opn_timerb[no] = FALSE;
		}
	}

	/* �f�[�^�L�� */
	opn_reg[no][reg] = dat;

	switch (reg) {
#if defined(MOUSE)
		/* �o�̓|�[�g */
		case 0x0f:
			/* �}�E�X�G�~�����[�V�����͕W��OPN�̂ݗL�� */
			if ((no == OPN_STD) && mos_capture && (mos_port != 3)) {
				if (mos_port == 1) {
					mask = 0x10;
				}
				else {
					mask = 0x20;
				}

				if (opn_reg[no][15] & mask) {
					strobe = TRUE;
				}
				else {
					strobe = FALSE;
				}
				mos_strobe_signal(strobe);
			}
			return;
#endif

		/* �v���X�P�[���P */
		case 0x2d:
			if (opn_scale[no] != 3) {
				opn_scale[no] = 6;
				opn_timera_calc(no);
				opn_timerb_calc(no);
			}
			return;

		/* �v���X�P�[���Q */
		case 0x2e:
			opn_scale[no] = 3;
			opn_timera_calc(no);
			opn_timerb_calc(no);
			return;

		/* �v���X�P�[���R */
		case 0x2f:
			opn_scale[no] = 2;
			opn_timera_calc(no);
			opn_timerb_calc(no);
			return;

		/* �^�C�}�[A */
		case 0x24:
		case 0x25:
			opn_timera_tick[no] = 0;
			return;

		/* �^�C�}�[B */
		case 0x26:
			opn_timerb_tick[no] = 0;
			return;
	}

	/* �o�͐���i�� */
	if ((reg >= 14) && (reg <= 0x26)) {
		return;
	}
	if ((reg >= 0x29) && (reg <= 0x2c)) {
		return;
	}

	/* �L�[�I�� */
	if (reg == 0x28) {
		if (dat >= 16) {
			opn_key[no][dat & 0x03] = TRUE;
		}
		else {
			opn_key[no][dat & 0x03] = FALSE;
		}
	}

	/* �o�� */
	opn_notify_no(no, reg, dat);
}

/*-[ ���ʏ���(I/O�|�[�g) ]---------------------------------------------------*/

/*
 *	OPN
 *	�f�[�^���W�X�^�ǂݍ��݋��ʏ���
 */
static void FASTCALL opn_read_data_reg(int no, BYTE *dat)
{
#if defined(MOUSE)
	BYTE port, trigger;
#endif

	ASSERT ((no >= 0) && (no <= 2));

	switch (opn_pstate[no]) {
		/* �ʏ�R�}���h */
		case OPN_INACTIVE:
		case OPN_READDAT:
		case OPN_WRITEDAT:
		case OPN_ADDRESS:
			*dat = opn_seldat[no];
			break;

		/* �X�e�[�^�X�ǂݏo�� */
		case OPN_READSTAT:
			*dat = 0;
			if (opn_timera_int[no]) {
				*dat |= 0x01;
			}
			if (opn_timerb_int[no]) {
				*dat |= 0x02;
			}
			break;

		/* �W���C�X�e�B�b�N�ǂݎ�� */
		case OPN_JOYSTICK:
			if (opn_selreg[no] == 14) {
				/* �W���C�X�e�B�b�N�|�[�g�G�~�����[�V�����͕W��OPN�̂� */
				if (no == OPN_STD) {
#if defined(MOUSE)
					if (mos_capture && (mos_port != 3)) {
						/* �}�E�X */
						if (mos_port == 1) {
							port = 0x00;
							trigger = (BYTE)(opn_reg[no][15] & 0x03);
						}
						else {
							port = 0x40;
							trigger = (BYTE)((opn_reg[no][15] >> 2) & 0x03);
						}

						if ((opn_reg[no][15] & 0xc0) == port) {
							*dat = (BYTE)(mos_readdata(trigger) | 0xc0);
							break;
						}
					}
#endif
					if ((opn_reg[no][15] & 0xf0) == 0x20) {
						/* �W���C�X�e�B�b�N�P */
						*dat = (BYTE)(~joy_request(0) | 0xc0);
						break;
					}
					if ((opn_reg[no][15] & 0xf0) == 0x50) {
						/* �W���C�X�e�B�b�N�Q */
						*dat = (BYTE)(~joy_request(1) | 0xc0);
						break;
					}
				}

				/* ����ȊO */
				*dat = 0xff;
			}
			else {
				/* ���W�X�^��14�łȂ���΁AFF�ȊO��Ԃ� */
				/* HOW MANY ROBOT�΍� */
				*dat = 0;
			}
			break;
	}
}

/*
 *	OPN
 *	���荞�݃X�e�[�^�X�ǂݍ��݋��ʏ���
 */
static void FASTCALL opn_read_interrupt_reg(int no, BYTE *dat)
{
	ASSERT ((no >= 0) && (no <= 2));

	if (opn_timera_int[no] || opn_timerb_int[no]) {
		*dat = 0xf7;
	}
	else {
		*dat = 0xff;
	}
}

/*
 *	OPN
 *	�R�}���h���W�X�^�������݋��ʏ���
 */
static void FASTCALL opn_write_command_reg(int no, BYTE dat)
{
	ASSERT ((no >= 0) && (no <= 2));

	switch (dat & 0x0f) {
		/* �C���A�N�e�B�u(�����`�Ȃ��A�f�[�^���W�X�^����������) */
		case OPN_INACTIVE:
			opn_pstate[no] = OPN_INACTIVE;
			break;
		/* �f�[�^�ǂݏo�� */
		case OPN_READDAT:
			opn_pstate[no] = OPN_READDAT;
			opn_seldat[no] = opn_readreg(no, opn_selreg[no]);
			break;
		/* �f�[�^�������� */
		case OPN_WRITEDAT:
			opn_pstate[no] = OPN_WRITEDAT;
			opn_writereg(no, opn_selreg[no], opn_seldat[no]);
			break;
		/* ���b�`�A�h���X */
		case OPN_ADDRESS:
			opn_pstate[no] = OPN_ADDRESS;
			opn_selreg[no] = opn_seldat[no];

			/* �v���X�P�[���̓A�h���X�w��݂̂�ok */
			if ((opn_selreg[no] >= 0x2d) && (opn_selreg[no] <= 0x2f)) {
				opn_seldat[no] = 0;
				opn_writereg(no, opn_selreg[no], opn_seldat[no]);
			}
			break;
		/* ���[�h�X�e�[�^�X */
		case OPN_READSTAT:
			opn_pstate[no] = OPN_READSTAT;
			break;
		/* �W���C�X�e�B�b�N�ǂݎ�� */
		case OPN_JOYSTICK:
			opn_pstate[no] = OPN_JOYSTICK;
			break;
	}
}

/*
 *	OPN
 *	�f�[�^���W�X�^�������݋��ʏ���
 */
static void FASTCALL opn_write_data_reg(int no, BYTE dat)
{
	ASSERT ((no >= 0) && (no <= 2));

	/* �f�[�^���L�� */
	opn_seldat[no] = dat;

	/* �C���A�N�e�B�u�ȊO�̏ꍇ�́A����̓�����s�� */
	switch (opn_pstate[no]) {
		/* �f�[�^�������� */
		case OPN_WRITEDAT:
			opn_writereg(no, opn_selreg[no], opn_seldat[no]);
			break;
		/* ���b�`�A�h���X */
		case OPN_ADDRESS:
			opn_selreg[no] = opn_seldat[no];

			/* �v���X�P�[���̓A�h���X�w��݂̂�ok */
			if ((opn_selreg[no] >= 0x2d) && (opn_selreg[no] <= 0x2f)) {
				opn_seldat[no] = 0;
				opn_writereg(no, opn_selreg[no], opn_seldat[no]);
			}
			break;
	}
}

/*-[ ���ʏ���(�X�e�[�g) ]----------------------------------------------------*/

/*
 *	OPN
 *	�X�e�[�g�Z�[�u���ʏ���
 */
static BOOL FASTCALL opn_save_common(int no, int fileh)
{
	ASSERT ((no >= 0) && (no <= 2));

	if (!file_write(fileh, opn_reg[no], 256)) {
		return FALSE;
	}
	if (!file_bool_write(fileh, opn_timera[no])) {
		return FALSE;
	}
	if (!file_bool_write(fileh, opn_timerb[no])) {
		return FALSE;
	}
	if (!file_dword_write(fileh, opn_timera_tick[no])) {
		return FALSE;
	}
	if (!file_dword_write(fileh, opn_timerb_tick[no])) {
		return FALSE;
	}
	if (!file_byte_write(fileh, opn_scale[no])) {
		return FALSE;
	}

	if (!file_byte_write(fileh, opn_pstate[no])) {
		return FALSE;
	}
	if (!file_byte_write(fileh, opn_selreg[no])) {
		return FALSE;
	}
	if (!file_byte_write(fileh, opn_seldat[no])) {
		return FALSE;
	}
	if (!file_bool_write(fileh, opn_timera_int[no])) {
		return FALSE;
	}
	if (!file_bool_write(fileh, opn_timerb_int[no])) {
		return FALSE;
	}
	if (!file_bool_write(fileh, opn_timera_en[no])) {
		return FALSE;
	}
	if (!file_bool_write(fileh, opn_timerb_en[no])) {
		return FALSE;
	}

	return TRUE;
}

/*
 *	OPN
 *	�X�e�[�g���[�h���ʏ���
 */
static BOOL FASTCALL opn_load_common(int no, int fileh)
{
	ASSERT ((no >= 0) && (no <= 2));

	if (!file_read(fileh, opn_reg[no], 256)) {
		return FALSE;
	}
	if (!file_bool_read(fileh, &opn_timera[no])) {
		return FALSE;
	}
	if (!file_bool_read(fileh, &opn_timerb[no])) {
		return FALSE;
	}
	if (!file_dword_read(fileh, &opn_timera_tick[no])) {
		return FALSE;
	}
	if (!file_dword_read(fileh, &opn_timerb_tick[no])) {
		return FALSE;
	}
	if (!file_byte_read(fileh, &opn_scale[no])) {
		return FALSE;
	}

	if (!file_byte_read(fileh, &opn_pstate[no])) {
		return FALSE;
	}
	if (!file_byte_read(fileh, &opn_selreg[no])) {
		return FALSE;
	}
	if (!file_byte_read(fileh, &opn_seldat[no])) {
		return FALSE;
	}
	if (!file_bool_read(fileh, &opn_timera_int[no])) {
		return FALSE;
	}
	if (!file_bool_read(fileh, &opn_timerb_int[no])) {
		return FALSE;
	}
	if (!file_bool_read(fileh, &opn_timera_en[no])) {
		return FALSE;
	}
	if (!file_bool_read(fileh, &opn_timerb_en[no])) {
		return FALSE;
	}

	return TRUE;
}

/*-[ �W��OPN ]---------------------------------------------------------------*/

/*
 *	OPN
 *	�^�C�}�[A�I�[�o�t���[
 */
static BOOL FASTCALL opn_timera_event(void)
{
	return opn_timera_event_main(OPN_STD);
}

/*
 *	OPN
 *	�^�C�}�[B�I�[�o�t���[
 */
static BOOL FASTCALL opn_timerb_event(void)
{
	return opn_timerb_event_main(OPN_STD);
}

/*
 *	OPN
 *	�P�o�C�g�ǂݏo��
 */
BOOL FASTCALL opn_readb(WORD addr, BYTE *dat)
{
	/* �L���t���O���`�F�b�N�A�����Ȃ牽�����Ȃ� */
#if XM7_VER >= 2
	if ((fm7_ver == 1) && (!opn_enable)) {
#else
	if ((fm_subtype == FMSUB_FM8) || !opn_enable) {
#endif
		return FALSE;
	}

	switch (addr) {
		/* �R�}���h���W�X�^�͓ǂݏo���֎~ */
#if XM7_VER >= 2
		case 0xfd0d:
			/* FM-7���[�h�ł�THG SSG���g�p���邽�ߖ��� */
			if (fm7_ver == 1) {
				return FALSE;
			}
#endif
		case 0xfd15:
			*dat = 0xff;
			return TRUE;

		/* �f�[�^���W�X�^ */
#if XM7_VER >= 2
		case 0xfd0e:
			/* FM-7���[�h�ł�THG SSG���g�p���邽�ߖ��� */
			if (fm7_ver == 1) {
				return FALSE;
			}
#endif
		case 0xfd16:
			opn_read_data_reg(OPN_STD, dat);
			return TRUE;

		/* �g�����荞�݃X�e�[�^�X */
		case 0xfd17:
			opn_read_interrupt_reg(OPN_STD, dat);
			if (ptm_irq_flag) {
				*dat &= (BYTE)~0x04;
			}
			return TRUE;
	}

	return FALSE;
}

/*
 *	OPN
 *	�P�o�C�g��������
 */
BOOL FASTCALL opn_writeb(WORD addr, BYTE dat)
{
	/* �L���t���O���`�F�b�N�A�����Ȃ牽�����Ȃ� */
#if XM7_VER >= 2
	if ((fm7_ver == 1) && (!opn_enable)) {
#else
	if ((fm_subtype == FMSUB_FM8) || !opn_enable) {
#endif
		return FALSE;
	}

	switch (addr) {
		/* OPN�R�}���h���W�X�^ */
		case 0xfd0d:
			/* FM-7���[�h�ł�THG SSG���g�p���邽�ߖ��� */
			if (fm7_ver == 1) {
				return FALSE;
			}
			/* PSG�A�N�Z�X���͉���2bit�̂ݗL�� */
			dat &= 0x03;
		case 0xfd15:
			opn_write_command_reg(OPN_STD, dat);
			return TRUE;

		/* �f�[�^���W�X�^ */
		case 0xfd0e:
			/* FM-7���[�h�ł�THG SSG���g�p���邽�ߖ��� */
			if (fm7_ver == 1) {
				return FALSE;
			}
		case 0xfd16:
			opn_write_data_reg(OPN_STD, dat);
			return TRUE;

		/* �}�E�X�Z�b�g���荞�݃}�X�N���W�X�^ */
		case 0xfd17:
			if (dat & 0x04) {
				ptm_irq_mask = FALSE;
			}
			else {
				ptm_irq_mask = TRUE;
			}
			return TRUE;
	}

	return FALSE;
}

/*
 *	OPN
 *	�Z�[�u
 */
BOOL FASTCALL opn_save(int fileh)
{
	if (!file_bool_write(fileh, opn_enable)) {
		return FALSE;
	}
	if (!file_bool_write(fileh, opn_use)) {
		return FALSE;
	}
	if (!opn_save_common(OPN_STD, fileh)) {
		return FALSE;
	}

	return TRUE;
}

/*
 *	OPN
 *	���[�h
 */
BOOL FASTCALL opn_load(int fileh, int ver)
{
	int i;

	/* �o�[�W�����`�F�b�N */
	if (ver < 200) {
		return FALSE;
	}

	/* Ver9.16/7.16/3.06�g�� */
#if XM7_VER >= 3
	if ((ver >= 916) || ((ver >= 716) && (ver <= 799))) {
#elif XM7_VER >= 2
	if (ver >= 716) {
#else
	if (ver >= 306) {
#endif
		if (!file_bool_read(fileh, &opn_enable)) {
			return FALSE;
		}
		if (!file_bool_read(fileh, &opn_use)) {
			return FALSE;
		}
	}

	if (!opn_load_common(OPN_STD, fileh)) {
		return FALSE;
	}

	/* OPN���W�X�^���� */
	opn_notify(0x27, (BYTE)(opn_reg[OPN_STD][0x27]));

	for (i=0; i<3; i++) {
		opn_notify(0x28, (BYTE)i);
		opn_key[OPN_STD][i] = FALSE;
	}

	for (i=0; i<13; i++) {
		if ((i < 8) || (i > 10)) {
			opn_notify((BYTE)i, opn_reg[OPN_STD][i]);
		}
		else {
			opn_notify((BYTE)i, 0);
		}
	}

	for (i=0x30; i<0xb4; i++) {
		opn_notify((BYTE)i, opn_reg[OPN_STD][i]);
	}

	/* �C�x���g */
	schedule_handle(EVENT_OPN_A, opn_timera_event);
	schedule_handle(EVENT_OPN_B, opn_timerb_event);

	return TRUE;
}

/*-[ WHG OPN ]---------------------------------------------------------------*/

/*
 *	WHG
 *	�^�C�}�[A�I�[�o�t���[
 */
static BOOL FASTCALL whg_timera_event(void)
{
	return opn_timera_event_main(OPN_WHG);
}

/*
 *	WHG
 *	�^�C�}�[B�I�[�o�t���[
 */
static BOOL FASTCALL whg_timerb_event(void)
{
	return opn_timerb_event_main(OPN_WHG);
}

/*
 *	WHG
 *	�P�o�C�g�ǂݏo��
 */
BOOL FASTCALL whg_readb(WORD addr, BYTE *dat)
{
	/* �L���t���O���`�F�b�N�A�����Ȃ牽�����Ȃ� */
#if XM7_VER == 1
	if (fm_subtype == FMSUB_FM8) {
		return FALSE;
	}
#endif

	switch (addr) {
		/* �R�}���h���W�X�^�͓ǂݏo���֎~ */
#if XM7_VER == 1
		case 0xfd2c:
#endif
		case 0xfd45:
			/* �L���t���O���`�F�b�N�A�����Ȃ牽�����Ȃ� */
#if XM7_VER == 1
			if ((!whg_enable && (addr == 0xfd45)) ||
				(!fmx_flag   && (addr == 0xfd2c))) {
#else
			if ((!whg_enable) && (addr == 0xfd45)) {
#endif
				return FALSE;
			}

			*dat = 0xff;
			return TRUE;

		/* �f�[�^���W�X�^ */
#if XM7_VER == 1
		case 0xfd2d:
#endif
		case 0xfd46:
			/* �L���t���O���`�F�b�N�A�����Ȃ牽�����Ȃ� */
#if XM7_VER == 1
			if ((!whg_enable && (addr == 0xfd46)) ||
				(!fmx_flag   && (addr == 0xfd2d))) {
#else
			if ((!whg_enable) && (addr == 0xfd46)) {
#endif
				return FALSE;
			}

			opn_read_data_reg(OPN_WHG, dat);
			return TRUE;

		/* �g�����荞�݃X�e�[�^�X */
		case 0xfd47:
			if (!whg_enable) {
				return FALSE;
			}
			opn_read_interrupt_reg(OPN_WHG, dat);
			return TRUE;
	}

	return FALSE;
}

/*
 *	WHG
 *	�P�o�C�g��������
 */
BOOL FASTCALL whg_writeb(WORD addr, BYTE dat)
{
	/* �L���t���O���`�F�b�N�A�����Ȃ牽�����Ȃ� */
#if XM7_VER == 1
	if (fm_subtype == FMSUB_FM8) {
		return FALSE;
	}
#endif

	switch (addr) {
		/* WHG/FM-X PSG�R�}���h���W�X�^ */
#if XM7_VER == 1
		case 0xfd2c:
			/* PSG�A�N�Z�X���͉���2bit�̂ݗL�� */
			dat &= 0x03;
#endif
		case 0xfd45:
			/* �L���t���O���`�F�b�N�A�����Ȃ牽�����Ȃ� */
#if XM7_VER == 1
			if ((!whg_enable && (addr == 0xfd45)) ||
				(!fmx_flag   && (addr == 0xfd2c))) {
#else
			if ((!whg_enable) && (addr == 0xfd45)) {
#endif
				return FALSE;
			}

			/* WHG�t���OON�EPSG���W�X�^�}�X�N���� */
			if ((dat & 0x0f) == OPN_WRITEDAT) {
				if (addr == 0xfd45) {
					/* WHG I/O�A�N�Z�X���̓t���O�I�� */
					whg_use = TRUE;
				}
#if XM7_VER == 1
				else {
					if (opn_selreg[OPN_WHG] >= 0x10) {
						/* ���W�X�^��0x10�ȏ�Ȃ牽�����Ȃ� */
						return TRUE;
					}
					/* �t���O�I�� */
					fmx_use = TRUE;
				}
#endif
			}

			opn_write_command_reg(OPN_WHG, dat);
			return TRUE;

		/* �f�[�^���W�X�^ */
#if XM7_VER == 1
		case 0xfd2d:
#endif
		case 0xfd46:
			/* �L���t���O���`�F�b�N�A�����Ȃ牽�����Ȃ� */
#if XM7_VER == 1
			if ((!whg_enable && (addr == 0xfd46)) ||
				(!fmx_flag   && (addr == 0xfd2d))) {
#else
			if ((!whg_enable) && (addr == 0xfd46)) {
#endif
				return FALSE;
			}

			/* WHG�t���OON�EPSG���W�X�^�}�X�N���� */
			if (opn_pstate[OPN_WHG] == OPN_WRITEDAT) {
				if (addr == 0xfd46) {
					/* WHG I/O�A�N�Z�X���̓t���O�I�� */
					whg_use = TRUE;
				}
#if XM7_VER == 1
				else {
					if (opn_selreg[OPN_WHG] >= 0x10) {
						/* ���W�X�^��0x10�ȏ�Ȃ牽�����Ȃ� */
						return TRUE;
					}
					/* �t���O�I�� */
					fmx_use = TRUE;
				}
#endif
			}

			opn_write_data_reg(OPN_WHG, dat);
			return TRUE;
	}

	return FALSE;
}

/*
 *	WHG
 *	�Z�[�u
 */
BOOL FASTCALL whg_save(int fileh)
{
	if (!file_bool_write(fileh, whg_enable)) {
		return FALSE;
	}
	if (!file_bool_write(fileh, whg_use)) {
		return FALSE;
	}
	if (!opn_save_common(OPN_WHG, fileh)) {
		return FALSE;
	}

	return TRUE;
}

/*
 *	WHG
 *	���[�h
 */
BOOL FASTCALL whg_load(int fileh, int ver)
{
	int i;

	/* �t�@�C���o�[�W����3�Œǉ� */
	if (ver < 300) {
		whg_use = FALSE;
		return TRUE;
	}

	if (!file_bool_read(fileh, &whg_enable)) {
		return FALSE;
	}
	if (!file_bool_read(fileh, &whg_use)) {
		return FALSE;
	}
	if (!opn_load_common(OPN_WHG, fileh)) {
		return FALSE;
	}

	/* WHG���W�X�^���� */
	whg_notify(0x27, (BYTE)(opn_reg[OPN_WHG][0x27]));

	for (i=0; i<3; i++) {
		whg_notify(0x28, (BYTE)i);
		opn_key[OPN_WHG][i] = FALSE;
	}

	for (i=0; i<13; i++) {
		if ((i < 8) || (i > 10)) {
			whg_notify((BYTE)i, opn_reg[OPN_WHG][i]);
		}
		else {
			whg_notify((BYTE)i, 0);
		}
	}

	for (i=0x30; i<0xb4; i++) {
		whg_notify((BYTE)i, opn_reg[OPN_WHG][i]);
	}

	/* �C�x���g */
	schedule_handle(EVENT_WHG_A, whg_timera_event);
	schedule_handle(EVENT_WHG_B, whg_timerb_event);

	return TRUE;
}

/*-[ THG OPN ]---------------------------------------------------------------*/

/*
 *	THG
 *	�^�C�}�[A�I�[�o�t���[
 */
static BOOL FASTCALL thg_timera_event(void)
{
	return opn_timera_event_main(OPN_THG);
}

/*
 *	THG
 *	�^�C�}�[B�I�[�o�t���[
 */
static BOOL FASTCALL thg_timerb_event(void)
{
	return opn_timerb_event_main(OPN_THG);
}

/*
 *	THG
 *	�P�o�C�g�ǂݏo��
 */
BOOL FASTCALL thg_readb(WORD addr, BYTE *dat)
{
	switch (addr) {
		/* �R�}���h���W�X�^�͓ǂݏo���֎~ */
		case 0xfd0d:
#if XM7_VER >= 2
			/* FM-7���[�h�̂ݗL�� */
			if (fm7_ver != 1) {
				return FALSE;
			}
#endif
		case 0xfd51:
			/* �L���t���O���`�F�b�N�A�����Ȃ牽�����Ȃ� */
			if ((!thg_enable) && (addr == 0xfd51)) {
				return FALSE;
			}
#if XM7_VER == 1
			if ((fm_subtype == FMSUB_FM8) && (addr == 0xfd51)) {
				return FALSE;
			}
#endif

			*dat = 0xff;
			return TRUE;

		/* �f�[�^���W�X�^ */
		case 0xfd0e:
#if XM7_VER >= 2
			/* FM-7���[�h�̂ݗL�� */
			if (fm7_ver != 1) {
				return FALSE;
			}
#endif
		case 0xfd52:
			/* �L���t���O���`�F�b�N�A�����Ȃ牽�����Ȃ� */
			if ((!thg_enable) && (addr == 0xfd52)) {
				return FALSE;
			}
#if XM7_VER == 1
			if ((fm_subtype == FMSUB_FM8) && (addr == 0xfd52)) {
				return FALSE;
			}
#endif

			opn_read_data_reg(OPN_THG, dat);
			return TRUE;

		/* �g�����荞�݃X�e�[�^�X */
		case 0xfd53:
#if XM7_VER == 1
			if (fm_subtype == FMSUB_FM8) {
				return FALSE;
			}
#endif
			opn_read_interrupt_reg(OPN_THG, dat);
			return TRUE;
	}

	return FALSE;
}

/*
 *	THG
 *	�P�o�C�g��������
 */
BOOL FASTCALL thg_writeb(WORD addr, BYTE dat)
{
	switch (addr) {
		/* THG�R�}���h���W�X�^ */
		case 0xfd0d:
#if XM7_VER >= 2
			/* FM-7���[�h�̂ݗL�� */
			if (fm7_ver != 1) {
				return FALSE;
			}
#endif
			/* PSG�A�N�Z�X���͉���2bit�̂ݗL�� */
			dat &= 0x03;
		case 0xfd51:
			/* �L���t���O���`�F�b�N�A�����Ȃ牽�����Ȃ� */
			if ((!thg_enable) && (addr == 0xfd51)) {
				return FALSE;
			}
#if XM7_VER == 1
			if ((fm_subtype == FMSUB_FM8) && (addr == 0xfd51)) {
				return FALSE;
			}
#endif

			/* THG�t���OON�EPSG���W�X�^�}�X�N���� */
			if ((dat & 0x0f) == OPN_WRITEDAT) {
				if (addr == 0xfd51) {
					/* THG I/O�A�N�Z�X���̓t���O�I�� */
					thg_use = TRUE;
				}
				else if (opn_selreg[OPN_THG] >= 0x10) {
					/* PSG I/O�A�N�Z�X���A���W�X�^��0x10�ȏ�Ȃ牽�����Ȃ� */
					return TRUE;
				}
			}

			opn_write_command_reg(OPN_THG, dat);
			return TRUE;

		/* �f�[�^���W�X�^ */
		case 0xfd0e:
#if XM7_VER >= 2
			/* FM-7���[�h�̂ݗL�� */
			if (fm7_ver != 1) {
				return FALSE;
			}
#endif
		case 0xfd52:
			/* �L���t���O���`�F�b�N�A�����Ȃ牽�����Ȃ� */
			if ((!thg_enable) && (addr == 0xfd52)) {
				return FALSE;
			}
#if XM7_VER == 1
			if ((fm_subtype == FMSUB_FM8) && (addr == 0xfd52)) {
				return FALSE;
			}
#endif

			/* THG�t���OON�EPSG���W�X�^�}�X�N���� */
			if (opn_pstate[OPN_THG] == OPN_WRITEDAT) {
				if (addr == 0xfd52) {
					/* THG I/O�A�N�Z�X���̓t���O�I�� */
					thg_use = TRUE;
				}
				else if (opn_selreg[OPN_THG] >= 0x10) {
					/* PSG I/O�A�N�Z�X���A���W�X�^��0x10�ȏ�Ȃ牽�����Ȃ� */
					return TRUE;
				}
			}

			opn_write_data_reg(OPN_THG, dat);
			return TRUE;
	}

	return FALSE;
}

/*
 *	THG
 *	�Z�[�u
 */
BOOL FASTCALL thg_save(int fileh)
{
	if (!file_bool_write(fileh, thg_enable)) {
		return FALSE;
	}
	if (!file_bool_write(fileh, thg_use)) {
		return FALSE;
	}
	if (!opn_save_common(OPN_THG, fileh)) {
		return FALSE;
	}

	return TRUE;
}

/*
 *	THG
 *	���[�h
 */
BOOL FASTCALL thg_load(int fileh, int ver)
{
	int i;

	/* �t�@�C���o�[�W����6�Œǉ� */
	if (ver < 200) {
		return FALSE;
	}
#if XM7_VER >= 2
	if (ver < 600) {
		thg_use = FALSE;
		return TRUE;
	}
#endif

	if (!file_bool_read(fileh, &thg_enable)) {
		return FALSE;
	}
	if (!file_bool_read(fileh, &thg_use)) {
		return FALSE;
	}
	if (!opn_load_common(OPN_THG, fileh)) {
		return FALSE;
	}

	/* THG���W�X�^���� */
	thg_notify(0x27, (BYTE)(opn_reg[OPN_THG][0x27]));

	for (i=0; i<3; i++) {
		thg_notify(0x28, (BYTE)i);
		opn_key[OPN_THG][i] = FALSE;
	}

	for (i=0; i<13; i++) {
		if ((i < 8) || (i > 10)) {
			thg_notify((BYTE)i, opn_reg[OPN_THG][i]);
		}
		else {
			thg_notify((BYTE)i, 0);
		}
	}

	for (i=0x30; i<0xb4; i++) {
		thg_notify((BYTE)i, opn_reg[OPN_THG][i]);
	}

	/* �C�x���g */
	schedule_handle(EVENT_THG_A, thg_timera_event);
	schedule_handle(EVENT_THG_B, thg_timerb_event);

	return TRUE;
}
