/*
 *	FM-7 EMULATOR "XM7"
 *
 *	Copyright (C) 1999-2017 �o�h�D(Twitter:@xm6_original)
 *	Copyright (C) 2001-2017 Ryu Takegami (Twitter:@RyuTakegami)
 *
 *	[ ���C��CPU�e��I/O ]
 *
 *	RHG����
 *	  2003.05.28		BEEP ON�̏�ԂŃX�e�[�g�t�@�C����ǂݍ���ł�BEEP���~
 *						�܂�Ȃ������C��
 *	  2003.08.12		�X�s�[�J�t���O�̏�������Y��Ă����̂��C��(�`
 *	  2005.11.12		FM-8���[�h���Ƀ^�C�}�������~����悤�ɕύX
 *	  2010.07.28		FM-8���[�h���ɂ�$FD0F��R/W��BASIC ROM/��RAM�̑I������
 *						����悤�ɕύX
 */

#include <string.h>
#include "xm7.h"
#include "mainetc.h"
#include "keyboard.h"
#include "opn.h"
#include "device.h"
#include "event.h"

/*
 *	�O���[�o�� ���[�N
 */
BOOL key_irq_flag;					/* �L�[�{�[�h���荞�� �v�� */
BOOL key_irq_mask;					/* �L�[�{�[�h���荞�� �}�X�N */
BOOL lp_irq_flag;					/* �v�����^���荞�� �v�� */
BOOL lp_irq_mask;					/* �v�����^���荞�� �}�X�N */
BOOL timer_irq_flag;				/* �^�C�}�[���荞�� �v�� */
BOOL timer_irq_mask;				/* �^�C�}�[���荞�� �}�X�N */

BOOL mfd_irq_flag;					/* FDC���荞�� �t���O */
BOOL mfd_irq_mask;					/* FDC���荞�� �}�X�N */
BOOL txrdy_irq_flag;				/* TxRDY���荞�� �t���O */
BOOL txrdy_irq_mask;				/* TxRDY���荞�� �}�X�N */
BOOL rxrdy_irq_flag;				/* RxRDY���荞�� �t���O */
BOOL rxrdy_irq_mask;				/* RxRDY���荞�� �}�X�N */
BOOL syndet_irq_flag;				/* SYNDET���荞�� �t���O */
BOOL syndet_irq_mask;				/* SYNDET���荞�� �}�X�N */
#if defined(MOUSE)
BOOL ptm_irq_flag;					/* PTM���荞�� �t���O */
BOOL ptm_irq_mask;					/* PTM���荞�� �}�X�N */
#endif
	
BOOL opn_irq_flag;					/* OPN���荞�� �t���O */
BOOL whg_irq_flag;					/* WHG���荞�� �t���O */
BOOL thg_irq_flag;					/* THG���荞�� �t���O */
#if XM7_VER >= 3
BOOL dma_irq_flag;					/* DMA���荞�� �t���O */
#endif
BOOL beep_flag;						/* BEEP�t���O */
BOOL speaker_flag;					/* �X�s�[�J�t���O */
#if XM7_VER == 1
BYTE banksel_en;					/* �o���N�؂芷���C�l�[�u���t���O */
#endif

/*
 *	�X�^�e�B�b�N ���[�N
 */
#if XM7_VER == 1 
#if defined(BUBBLE)
static int boot_mode_reset;			/* ���Z�b�g���̃u�[�gROM�o���N */
#endif
#endif

/*
 *	�v���g�^�C�v�錾
 */
static BOOL FASTCALL mainetc_event(void);	/* 2.03ms �^�C�}�[�C�x���g */

/*
 *	���C��CPU I/O
 *	������
 */
BOOL FASTCALL mainetc_init(void)
{
#if XM7_VER == 1
	/* FM-8���[�h�̕W���̓o���N�؂芷������ */
	banksel_en = BANKSEL_DISABLE;
#if defined(BUBBLE)
	boot_mode_reset = BOOT_BASIC;
#endif
#endif

	return TRUE;
}

/*
 *	���C��CPU I/O
 *	�N���[���A�b�v
 */
void FASTCALL mainetc_cleanup(void)
{
}

/*
 *	���C��CPU I/O
 *	���Z�b�g
 */
void FASTCALL mainetc_reset(void)
{
	/* ���荞�݃t���O������ */
	key_irq_flag = FALSE;
	key_irq_mask = TRUE;
	lp_irq_flag = FALSE;
	lp_irq_mask = TRUE;
	timer_irq_flag = FALSE;
	timer_irq_mask = TRUE;

	mfd_irq_flag = FALSE;
	mfd_irq_mask = TRUE;
	txrdy_irq_flag = FALSE;
	txrdy_irq_mask = TRUE;
	rxrdy_irq_flag = FALSE;
	rxrdy_irq_mask = TRUE;
	syndet_irq_flag = FALSE;
	syndet_irq_mask = TRUE;
#if defined(MOUSE)
	ptm_irq_flag = FALSE;
	ptm_irq_mask = TRUE;
#endif

	opn_irq_flag = FALSE;
	whg_irq_flag = FALSE;
	thg_irq_flag = FALSE;
#if XM7_VER >= 3
	dma_irq_flag = FALSE;
#endif

	/* BEEP�t���O������ */
	beep_flag = FALSE;
	speaker_flag = FALSE;

#if XM7_VER == 1
	/* �C�x���g��ǉ�(FM-8���[�h������) */
	if (fm_subtype != FMSUB_FM8) {
		schedule_setevent(EVENT_MAINTIMER, 2034, mainetc_event);
	}
	else {
		schedule_delevent(EVENT_MAINTIMER);
	}
#else
	/* �C�x���g��ǉ� */
	schedule_setevent(EVENT_MAINTIMER, 2034, mainetc_event);
#endif

	/* �ʒm */
	beep_notify();

	/* ���Z�b�g���̃u�[�g���[�h���L�� */
#if XM7_VER == 1 
#if defined(BUBBLE)
	boot_mode_reset = boot_mode;
#endif
#endif
}

/*
 *	BEEP�I���C�x���g
 */
BOOL FASTCALL mainetc_beep(void)
{
	/* BEEP��~ */
	beep_flag = FALSE;

	/* ���ȃC�x���g�폜���́ATRUE�ɂ��� */
	schedule_delevent(EVENT_BEEP);

	/* �ʒm */
	beep_notify();

	return TRUE;
}

/*
 *	�^�C�}�[���荞�݃C�x���g
 */
static BOOL FASTCALL mainetc_event(void)
{
	/* 2.03ms���Ƃ�CLK�ŁAmask�̔��]��DFF�œ��͂��� */
	timer_irq_flag = !timer_irq_mask;
	maincpu_irq();

	/* CLK��4.9152MHz�Ȃ̂ŁA2.0345ms�P�ʂŔ������� */
	if (event[EVENT_MAINTIMER].reload == 2034) {
		schedule_setevent(EVENT_MAINTIMER, 2035, mainetc_event);
	}
	else {
		schedule_setevent(EVENT_MAINTIMER, 2034, mainetc_event);
	}

	return TRUE;
}

/*
 *	FDC���荞��
 *	(fdc.c���R�}���h����E�ُ�I�����A�t�H�[�X�C���^���v�g�ŌĂ΂��)
 */
void FASTCALL mainetc_fdc(void)
{
#if XM7_VER == 1
	/* �}�X�N����Ă���΁A�������Ȃ� */
	if ((fm_subtype == FMSUB_FM8) || mfd_irq_mask) {
		return;
	}
#else
	/* �}�X�N����Ă���΁A�������Ȃ� */
	if (mfd_irq_mask) {
		return;
	}
#endif

	/* ���C��CPU��IRQ���荞�݂������� */
	mfd_irq_flag = TRUE;

	/* ���� */
	maincpu_irq();
}

/*
 *	LP���荞��
 *	(tapelp.c���v�����^�f�[�^�o�͎��ɌĂ΂��)
 */
void FASTCALL mainetc_lp(void)
{
#if XM7_VER == 1
	/* �}�X�N����Ă���΁A�������Ȃ� */
	if ((fm_subtype == FMSUB_FM8) || lp_irq_mask) {
		return;
	}
#else
	/* �}�X�N����Ă���΁A�������Ȃ� */
	if (lp_irq_mask) {
		return;
	}
#endif

	/* ���C��CPU��IRQ���荞�݂������� */
	lp_irq_flag = TRUE;

	/* ���� */
	maincpu_irq();
}

/*
 *	���C��CPU I/O
 *	�P�o�C�g�ǂݏo��
 */
BOOL FASTCALL mainetc_readb(WORD addr, BYTE *dat)
{
	BYTE ret;

	switch (addr) {
		/* �L�[�{�[�h ��� */
		case 0xfd00:
#if XM7_VER == 1
			if (fm_subtype == FMSUB_FM8) {
#if defined(XM7PURE)
				ret = 0xff;
				if (lowspeed_mode) {
					ret &= (BYTE)0xfe;
				}

				*dat = ret;
				return TRUE;
#else
				/* �g(����)�� */
				*dat = 0xff;
				return TRUE;
#endif
			}
#endif

			if (key_fm7 & 0x0100) {
				ret = 0xff;
			}
			else {
				ret = 0x7f;
			}
#if XM7_VER == 1
			if (lowspeed_mode) {
				ret &= (BYTE)0xfe;
			}
#endif
			*dat = ret;
			return TRUE;

		/* �L�[�{�[�h ���� */
		case 0xfd01:
#if XM7_VER == 1
			if (fm_subtype == FMSUB_FM8) {
				return FALSE;
			}
#endif

			*dat = (BYTE)(key_fm7 & 0xff);
			key_irq_flag = FALSE;
			maincpu_irq();
			subcpu_firq();
			return TRUE;

		/* IRQ�v������ */
		case 0xfd03:
#if XM7_VER == 1
			if (fm_subtype == FMSUB_FM8) {
				*dat = 0xff;
				return TRUE;
			}
#endif

			ret = 0xff;
			if ((key_irq_flag) && !(key_irq_mask)) {
				ret &= ~0x01;
			}
			if (lp_irq_flag) {
				ret &= ~0x02;
				lp_irq_flag = FALSE;
			}
			if (timer_irq_flag) {
				ret &= ~0x04;
				timer_irq_flag = FALSE;
			}
			if (mfd_irq_flag ||
				txrdy_irq_flag ||
				rxrdy_irq_flag ||
				syndet_irq_flag ||
#if XM7_VER >= 3
				dma_irq_flag ||
#endif
#if defined(MOUSE)
				ptm_irq_flag ||
#endif
				opn_irq_flag ||
				whg_irq_flag ||
				thg_irq_flag) {
				ret &= ~0x08;
			}
			*dat = ret;
			maincpu_irq();
			return TRUE;

		/* BASIC ROM */
		case 0xfd0f:
			*dat = 0xff;
#if XM7_VER == 1
			if ((fm_subtype == FMSUB_FM8) && (banksel_en == BANKSEL_DISABLE)) {
				return TRUE;
			}
#endif
			basicrom_en = TRUE;

#if XM7_VER == 1
			/* FM-8���[�h���̓u�[�g���[�h���؂�ւ��d�l */
			if ((fm_subtype == FMSUB_FM8) &&
				(banksel_en == BANKSEL_ENABLE_DIPSW)) {
				boot_mode = BOOT_BASIC;
			}
#endif

			return TRUE;
	}

	return FALSE;
}

/*
 *	���C��CPU I/O
 *	�P�o�C�g��������
 */
BOOL FASTCALL mainetc_writeb(WORD addr, BYTE dat)
{
	switch (addr) {
		/* ���荞�݃}�X�N */
		case 0xfd02:
#if XM7_VER == 1
			if (fm_subtype == FMSUB_FM8) {
				return TRUE;
			}
#endif

			if (dat & 0x80) {
				syndet_irq_mask = FALSE;
			}
			else {
				syndet_irq_mask = TRUE;
			}
			if (dat & 0x40) {
				rxrdy_irq_mask = FALSE;
			}
			else {
				rxrdy_irq_mask = TRUE;
			}
			if (dat & 0x20) {
				txrdy_irq_mask = FALSE;
			}
			else {
				txrdy_irq_mask = TRUE;
			}
			if (dat & 0x10) {
				mfd_irq_mask = FALSE;
			}
			else {
				mfd_irq_mask = TRUE;
			}
			if (dat & 0x04) {
				timer_irq_mask = FALSE;
			}
			else {
				timer_irq_mask = TRUE;
			}
			if (dat & 0x02) {
				lp_irq_mask = FALSE;
			}
			else {
				lp_irq_mask = TRUE;
			}
			if (dat & 0x01) {
				key_irq_mask = FALSE;
			}
			else {
				key_irq_mask = TRUE;
			}
			maincpu_irq();
			subcpu_firq();
			return TRUE;

		/* BEEP */
		case 0xfd03:
			/* �X�s�[�J�t���O�̏��� */
			if (dat & 0x01) {
				speaker_flag = TRUE;
			}
			else {
				speaker_flag = FALSE;
			}
			if (dat & 0x40) {
				/* �P��BEEP */
				beep_flag = TRUE;
				schedule_setevent(EVENT_BEEP, 205000, mainetc_beep);

				/* �ʒm */
				beep_notify();
			}
			else {
				if (dat & 0x80) {
					/* �A��BEEP */
					beep_flag = TRUE;
				}
				else {
					/* BEEP OFF */
					beep_flag = FALSE;
				}

				/* �ʒm */
				beep_notify();
			}
			return TRUE;

		/* BASIC ROM */
		case 0xfd0f:
#if XM7_VER == 1
			if ((fm_subtype == FMSUB_FM8) && (banksel_en == BANKSEL_DISABLE)) {
				return TRUE;
			}
#endif
			basicrom_en = FALSE;

#if XM7_VER == 1
			/* FM-8���[�h���̓u�[�g���[�h���؂�ւ��d�l */
			if ((fm_subtype == FMSUB_FM8) &&
				(banksel_en == BANKSEL_ENABLE_DIPSW)) {
#if defined(BUBBLE)
				if (boot_mode_reset == BOOT_BASIC) {
					boot_mode = BOOT_DOS;
				}
				else {
					boot_mode = boot_mode_reset;
				}
#else
				boot_mode = BOOT_DOS;
#endif
			}
#endif

			return TRUE;
	}

	return FALSE;
}

/*
 *	���C��CPU I/O
 *	�Z�[�u
 */
BOOL FASTCALL mainetc_save(int fileh)
{
	if (!file_bool_write(fileh, key_irq_flag)) {
		return FALSE;
	}
	if (!file_bool_write(fileh, key_irq_mask)) {
		return FALSE;
	}
	if (!file_bool_write(fileh, lp_irq_flag)) {
		return FALSE;
	}
	if (!file_bool_write(fileh, lp_irq_mask)) {
		return FALSE;
	}
	if (!file_bool_write(fileh, timer_irq_flag)) {
		return FALSE;
	}
	if (!file_bool_write(fileh, timer_irq_mask)) {
		return FALSE;
	}

	if (!file_bool_write(fileh, mfd_irq_flag)) {
		return FALSE;
	}
	if (!file_bool_write(fileh, mfd_irq_mask)) {
		return FALSE;
	}
	if (!file_bool_write(fileh, txrdy_irq_flag)) {
		return FALSE;
	}
	if (!file_bool_write(fileh, txrdy_irq_mask)) {
		return FALSE;
	}
	if (!file_bool_write(fileh, rxrdy_irq_flag)) {
		return FALSE;
	}
	if (!file_bool_write(fileh, rxrdy_irq_mask)) {
		return FALSE;
	}
	if (!file_bool_write(fileh, syndet_irq_flag)) {
		return FALSE;
	}
	if (!file_bool_write(fileh, syndet_irq_mask)) {
		return FALSE;
	}
#if defined(MOUSE)
	if (!file_bool_write(fileh, ptm_irq_flag)) {
		return FALSE;
	}
	if (!file_bool_write(fileh, ptm_irq_mask)) {
		return FALSE;
	}
#endif
	if (!file_bool_write(fileh, opn_irq_flag)) {
		return FALSE;
	}
	if (!file_bool_write(fileh, whg_irq_flag)) {
		return FALSE;
	}

	/* Ver6,8�ǉ� */
	if (!file_bool_write(fileh, thg_irq_flag)) {
		return FALSE;
	}
#if XM7_VER >= 3
	/* Ver8�ǉ� */
	if (!file_bool_write(fileh, dma_irq_flag)) {
		return FALSE;
	}
#endif

	if (!file_bool_write(fileh, beep_flag)) {
		return FALSE;
	}
	if (!file_bool_write(fileh, speaker_flag)) {
		return FALSE;
	}

#if XM7_VER == 1
	if (!file_byte_write(fileh, banksel_en)) {
		return FALSE;
	}
#endif

	return TRUE;
}

/*
 *	���C��CPU I/O
 *	���[�h
 */
BOOL FASTCALL mainetc_load(int fileh, int ver)
{
	/* �o�[�W�����`�F�b�N */
	if (ver < 200) {
		return FALSE;
	}

	if (!file_bool_read(fileh, &key_irq_flag)) {
		return FALSE;
	}
	if (!file_bool_read(fileh, &key_irq_mask)) {
		return FALSE;
	}
	if (!file_bool_read(fileh, &lp_irq_flag)) {
		return FALSE;
	}
	if (!file_bool_read(fileh, &lp_irq_mask)) {
		return FALSE;
	}
	if (!file_bool_read(fileh, &timer_irq_flag)) {
		return FALSE;
	}
	if (!file_bool_read(fileh, &timer_irq_mask)) {
		return FALSE;
	}

	if (!file_bool_read(fileh, &mfd_irq_flag)) {
		return FALSE;
	}
	if (!file_bool_read(fileh, &mfd_irq_mask)) {
		return FALSE;
	}
	if (!file_bool_read(fileh, &txrdy_irq_flag)) {
		return FALSE;
	}
	if (!file_bool_read(fileh, &txrdy_irq_mask)) {
		return FALSE;
	}
	if (!file_bool_read(fileh, &rxrdy_irq_flag)) {
		return FALSE;
	}
	if (!file_bool_read(fileh, &rxrdy_irq_mask)) {
		return FALSE;
	}
	if (!file_bool_read(fileh, &syndet_irq_flag)) {
		return FALSE;
	}
	if (!file_bool_read(fileh, &syndet_irq_mask)) {
		return FALSE;
	}
	/* Ver3.09/7.19/9.19�ǉ� */
#if defined(MOUSE)
#if XM7_VER == 1
	if (ver >= 309) {
#elif XM7_VER == 2
	if ((ver >= 719) && (ver <= 799)) {
#else
	if (((ver >= 719) && (ver <= 799)) || (ver >= 919)) {
#endif
		if (!file_bool_read(fileh, &ptm_irq_flag)) {
			return FALSE;
		}
		if (!file_bool_read(fileh, &ptm_irq_mask)) {
			return FALSE;
		}
	}
#endif

	if (!file_bool_read(fileh, &opn_irq_flag)) {
		return FALSE;
	}
	if (!file_bool_read(fileh, &whg_irq_flag)) {
		return FALSE;
	}

	/* Ver6�ǉ� */
#if XM7_VER >= 2
	if (ver >= 600) {
#else
	if (ver >= 300) {
#endif
		if (!file_bool_read(fileh, &thg_irq_flag)) {
			return FALSE;
		}
	}

#if XM7_VER >= 3
	/* Ver8�ǉ� */
	if (ver >= 800) {
		if (!file_bool_read(fileh, &dma_irq_flag)) {
			return FALSE;
		}
	}
#endif

	if (!file_bool_read(fileh, &beep_flag)) {
		return FALSE;
	}
	if (!file_bool_read(fileh, &speaker_flag)) {
		return FALSE;
	}

#if XM7_VER == 1
	if (ver >= 303) {
		if (!file_byte_read(fileh, &banksel_en)) {
			return FALSE;
		}
		if (banksel_en > BANKSEL_ENABLE_DIPSW) {
			banksel_en = BANKSEL_ENABLE;
		}
	}
	else {
		banksel_en = BANKSEL_DISABLE;
	}
#endif

	/* �C�x���g */
	schedule_handle(EVENT_MAINTIMER, mainetc_event);
	schedule_handle(EVENT_BEEP, mainetc_beep);

	/* �ʒm */
	beep_notify();

	return TRUE;
}
