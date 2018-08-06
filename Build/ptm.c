/*
 *	FM-7 EMULATOR "XM7"
 *
 *	Copyright (C) 1999-2017 �o�h�D(Twitter:@xm6_original)
 *	Copyright (C) 2001-2017 Ryu Takegami (Twitter:@RyuTakegami)
 *
 *	[ �v���O���}�u���^�C�}(MB8873H) �ȈՔ� ]
 *
 */

#if defined(MOUSE)

#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "xm7.h"
#include "device.h"
#include "mainetc.h"
#include "ptm.h"
#include "mouse.h"
#include "event.h"

/*
 *	�O���[�o�� ���[�N
 */
int ptm_counter[6];						/* �J�E���^ */
int ptm_counter_preset[6];				/* �J�E���^�v���Z�b�g�l */

BYTE ptm_mode_select[3];				/* ���샂�[�h */
BOOL ptm_running_flag[3];				/* ���쒆�t���O */

BOOL ptm_out_flag[3];					/* �o�͋��t���O */
BOOL ptm_irq_flag_int[3];				/* ���荞�݃t���O */
BOOL ptm_irq_mask_int[3];					/* ���荞�݃}�X�N�t���O */
BOOL ptm_mode_16bit[3];					/* 16�r�b�g�J�E���g���[�h */
BOOL ptm_clock_type[3];					/* �N���b�N�� */

BOOL ptm_preset_mode;					/* �v���Z�b�g(InternalReset)���[�h */
BOOL ptm_select_reg1;					/* ���W�X�^�I����� */
BOOL ptm_clock_divide;					/* �N���b�N�������[�h */

/*
 *	�X�^�e�B�b�N ���[�N
 */
static BOOL ptm_counter_select[3];		/* �f���A�����[�h���W�X�^�I����� */
static BYTE ptm_timer_adjust;			/* �^�C�}�Ԋu�ߎ��p�J�E���^ */
static BYTE ptm_read_latch_buffer;		/* ���[�h�p���b�`�o�b�t�@ */
static BYTE ptm_write_latch_buffer;		/* ���C�g�p���b�`�o�b�t�@ */

/*
 *	�v���g�^�C�v�錾
 */
static BOOL FASTCALL ptm_event(void);


/*
 *	�v���O���}�u���^�C�}
 *	������
 */
BOOL FASTCALL ptm_init(void)
{
	return TRUE;
}

/*
 *	�v���O���}�u���^�C�}
 *	�N���[���A�b�v
 */
void FASTCALL ptm_cleanup(void)
{
}

/*
 *	�v���O���}�u���^�C�}
 *	�J�E���^���Z�b�g
 */
static void FASTCALL ptm_counter_reset(void)
{
	int i;

	for (i=0; i<6; i++) {
		ptm_counter[i] = 0;
		ptm_counter_preset[i] = 0;
	}

	for (i=0; i<3; i++) {
		ptm_mode_select[i] = 0;
		ptm_running_flag[i] = FALSE;
		ptm_counter_select[i] = FALSE;
		ptm_clock_type[i] = FALSE;
	}

	ptm_preset_mode = TRUE;
}

/*
 *	�v���O���}�u���^�C�}
 *	���Z�b�g
 */
void FASTCALL ptm_reset(void)
{
	int i;

	ptm_counter_reset();

	for (i=0; i<3; i++) {
		ptm_out_flag[i] = FALSE;
		ptm_irq_flag_int[i] = FALSE;
		ptm_irq_mask_int[i] = TRUE;
		ptm_mode_16bit[i] = TRUE;
	}

	ptm_read_latch_buffer = 0x00;
	ptm_write_latch_buffer = 0x00;

	ptm_clock_divide = FALSE;
	ptm_select_reg1 = FALSE;

	/* �C�x���g */
#if XM7_VER == 1
	if ((fm_subtype == FMSUB_FM8) || (mos_port != 3)) {
#else
	if (mos_port != 3) {
#endif
		schedule_delevent(EVENT_PTM_TIMER);
	}
	else {
		schedule_setevent(EVENT_PTM_TIMER, 52, ptm_event);
	}
	ptm_timer_adjust = 0;
}

/*
 *	�v���O���}�u���^�C�}
 *	�C�x���g
 */
static BOOL FASTCALL ptm_event(void)
{
	BYTE i;

	/* �v���Z�b�g���[�h�Ȃ牽�����Ȃ� */
	if (ptm_preset_mode) {
		return TRUE;
	}

	/* 19.2kHz�^�C�}���ߎ����� */
	ptm_timer_adjust = (BYTE)((ptm_timer_adjust + 1) % 12);
	if (ptm_timer_adjust == 0) {
		schedule_setevent(EVENT_PTM_TIMER, 53, ptm_event);
	}
	else {
		schedule_setevent(EVENT_PTM_TIMER, 52, ptm_event);
	}

	/* �e�^�C�}�𓮂��� */
	for (i=0; i<3; i++) {
		if (ptm_running_flag[i]) {

			if (ptm_clock_type[i]) {
				ptm_counter[i] -= 104;
			}
			else {
				ptm_counter[i] --;
			}

			/* �ȉ��A�J�E���^���O�ȉ��ɂȂ����甭�� */
			if (ptm_counter[i] <= 0) {
				/* �t���O�𗧂Ă� */
				ptm_irq_flag_int[i] = TRUE;

				/* �}�X�N����Ă��Ȃ���Ί��荞�݂�������(CH0�ȊO) */
				if (!(ptm_irq_mask_int[i] || ptm_irq_mask)) {
					ptm_irq_flag = TRUE;
					maincpu_irq();
				}

				if (ptm_mode_select[i] & 0x20) {
					/* �V���O�����[�h�̏ꍇ�̓^�C�}������������� */
					ptm_running_flag[i] = FALSE;
				}
				else {
					/* �J�E���^���Z�b�g */
					if ((ptm_mode_16bit[i]) || (!ptm_counter_select[i])) {
						ptm_counter[i] += ptm_counter_preset[i];
					}
					else {
						ptm_counter[i] += ptm_counter_preset[i + 3];
					}
					ptm_counter_select[i] = !ptm_counter_select[i];
				}
			}
		}
	}

	return TRUE;
}

/*
 *	�J�E���^�v���Z�b�g
 */
static void FASTCALL ptm_preset_counter(BYTE reg)
{
	/* �\�J�E���^�ݒ� */
	ptm_counter_preset[reg] = ptm_counter[reg];
	if (ptm_clock_divide) {
		ptm_counter_preset[reg] *= 8;
	}

	/* �W�r�b�g���[�h���̗��J�E���^�ݒ� */
	if (!ptm_mode_16bit[reg]) {
		ptm_counter_preset[reg + 3] = ptm_counter[reg + 3];
		if (ptm_clock_divide) {
			ptm_counter_preset[reg + 3] *= 8;
		}
	}

	/* �t���O�ރ��Z�b�g */
	ptm_running_flag[reg] = TRUE;
	ptm_counter_select[reg] = FALSE;
	ptm_irq_flag_int[reg] = FALSE;
}

/*
 *	�R���g���[�����W�X�^�ݒ�
 */
static void FASTCALL ptm_control_set(BYTE reg, BYTE dat)
{
	/* bit7:�o�̓}�X�N */
	if (dat & 0x80) {
		ptm_out_flag[reg] = TRUE;
	}
	else {
		ptm_out_flag[reg] = FALSE;
	}

	/* bit6:���荞�݃}�X�N */
	if (dat & 0x40) {
		ptm_irq_mask_int[reg] = FALSE;
	}
	else {
		ptm_irq_mask_int[reg] = TRUE;
	}

	/* bit1:�N���b�N���I�� */
	if (dat & 0x02) {
		/* bit1=1 : E�N���b�N(2MHz)�(�܂Ƃ��ɑΉ����Ă��܂���) */
		ptm_clock_type[reg] = TRUE;
	}
	else {
		/* bit1=0 : C�N���b�N(19.2kHz)� */
		ptm_clock_type[reg] = FALSE;
	}

	/* bit2:���[�h�I�� */
	if (dat & 0x04) {
		/* bit2=1 : 8�r�b�g���[�h */
		ptm_mode_16bit[reg] = FALSE;
	}
	else {
		/* bit2=0 : 16�r�b�g���[�h */
		ptm_mode_16bit[reg] = TRUE;
	}

	/* bit5-3:���[�h�Z���N�g */
	ptm_mode_select[reg] = (BYTE)(dat & 0x38);
	switch(dat & 0x38) {
		case 0x10 :	/* �R���e�B�j���[���[�h(START) */
		case 0x30 :	/* �V���O�����[�h(START) */
					ptm_preset_counter(reg);
					break;

		case 0x00 :	/* �R���e�B�j���[���[�h(LATCH) */
		case 0x20 :	/* �V���O�����[�h(LATCH) */
					break;

		case 0x08 :	/* ���g����r���[�h(SHORT) */
		case 0x28 :	/* ���g����r���[�h(LONG) */
		case 0x18 :	/* �p���X��r���[�h(SHORT) */
		case 0x38 :	/* �p���X��r���[�h(LONG) */
					ptm_running_flag[reg] = FALSE;
					ptm_irq_flag_int[reg] = FALSE;
					break;
	}
}

/*
 *	�v���O���}�u���^�C�}
 *	�P�o�C�g�擾
 */
BOOL FASTCALL ptm_readb(WORD addr,BYTE *dat)
{
	BYTE tmp;
	BYTE reg;

#if XM7_VER == 1
	if ((fm_subtype == FMSUB_FM8) || (mos_port != 3)) {
#else
	if (mos_port != 3) {
#endif
		return FALSE;
	}

	switch (addr) {
		/* �X�e�[�^�X���W�X�^ */
		case 0xfde1:
			tmp = 0x00;
			if (ptm_irq_flag_int[0]) {
				tmp |= 0x01;
				ptm_irq_flag_int[0] = FALSE;
			}
			if (ptm_irq_flag_int[1]) {
				tmp |= 0x02;
				ptm_irq_flag_int[1] = FALSE;
			}
			if (ptm_irq_flag_int[2]) {
				tmp |= 0x04;
				ptm_irq_flag_int[2] = FALSE;
			}
			if (ptm_irq_flag) {
				tmp |= 0x80;
			}
			*dat = tmp;

			/* IRQ������ */
			ptm_irq_flag = FALSE;
			maincpu_irq();
			return TRUE;

		/* �J�E���^(��ʃo�C�g) */
		case 0xfde2:
		case 0xfde4:
		case 0xfde6:
			reg = (BYTE)((addr - 0xfde2) >> 1);
			if (ptm_mode_16bit[reg]) {
				/* �P�U�r�b�g���[�h */
				*dat = (BYTE)(ptm_counter[reg] >> 8);
				ptm_read_latch_buffer = (BYTE)(ptm_counter[reg] & 0xFF);
			}
			else {
				/* �W�r�b�g���[�h */
				*dat = (BYTE)(ptm_counter[reg] & 0xFF);
				ptm_read_latch_buffer = (BYTE)(ptm_counter[reg + 3] & 0xFF);
			}
			return TRUE;

		/* �J�E���^(���ʃo�C�g,���b�`�o�b�t�@) */
		case 0xfde3:
		case 0xfde5:
		case 0xfde7:
			*dat = ptm_read_latch_buffer;
			return TRUE;
	}

	return FALSE;
}

/*
 *	�v���O���}�u���^�C�}
 *	�P�o�C�g��������
 */
BOOL FASTCALL ptm_writeb(WORD addr, BYTE dat)
{
	BYTE reg;

#if XM7_VER == 1
	if ((fm_subtype == FMSUB_FM8) || (mos_port != 3)) {
#else
	if (mos_port != 3) {
#endif
		return FALSE;
	}

	switch (addr) {
		/* �R���g���[�����W�X�^ 1/3 */
		case 0xfde0:
			if (ptm_select_reg1) {
				/* bit0:�^�C�}�v���Z�b�g */
				if (dat & 0x01) {
					ptm_counter_reset();
				}
				else {
					ptm_control_set(0, dat);
					ptm_preset_mode = FALSE;
				}
			}
			else {
				/* bit0:�N���b�N�����r�b�g */
				if (dat & 0x01) {
					ptm_clock_divide = TRUE;
				}
				else {
					ptm_clock_divide = FALSE;
				}
				ptm_control_set(2, dat);
			}
			return TRUE;

		/* �R���g���[�����W�X�^ 2 */
		case 0xfde1:
			ptm_control_set(1, dat);
			/* bit0 : ���W�X�^�I���r�b�g */
			if (dat & 0x01) {
				ptm_select_reg1 = TRUE;
			}
			else {
				ptm_select_reg1 = FALSE;
			}
			return TRUE;

		/* �J�E���^(��ʃo�C�g,���b�`�o�b�t�@) */
		case 0xfde2:
		case 0xfde4:
		case 0xfde6:
			ptm_write_latch_buffer = (BYTE)dat;
			return TRUE;

		/* �J�E���^(���ʃo�C�g) */
		case 0xfde3:
		case 0xfde5:
		case 0xfde7:
			reg = (BYTE)((addr - 0xfde2) >> 1);
			if (ptm_mode_16bit[reg]) {
				/* �P�U�r�b�g���[�h */
				ptm_counter[reg] = (WORD)((ptm_write_latch_buffer << 8) | dat);
			}
			else {
				/* �W�r�b�g���[�h */
				ptm_counter[reg + 0] = (WORD)ptm_write_latch_buffer;
				ptm_counter[reg + 3] = (WORD)dat;
			}

			/* LATCH���[�h�̃J�E���^���Z�b�g���� */
			if (!(ptm_mode_select[reg] & 0x10)) {
				ptm_preset_counter(reg);
			}
			return TRUE;
	}

	return FALSE;
}

/*
 *	�v���O���}�u���^�C�}
 *	�Z�[�u
 */
BOOL FASTCALL ptm_save(int fileh)
{
	int i;

	for (i=0; i<6; i++) {
		if (!file_dword_write(fileh, ptm_counter[i])) {
			return FALSE;
		}
		if (!file_dword_write(fileh, ptm_counter_preset[i])) {
			return FALSE;
		}
	}

	for (i=0; i<3; i++) {
		if (!file_byte_write(fileh, ptm_mode_select[i])) {
			return FALSE;
		}
		if (!file_bool_write(fileh, ptm_running_flag[i])) {
			return FALSE;
		}

		if (!file_bool_write(fileh, ptm_out_flag[i])) {
			return FALSE;
		}
		if (!file_bool_write(fileh, ptm_irq_flag_int[i])) {
			return FALSE;
		}
		if (!file_bool_write(fileh, ptm_irq_mask_int[i])) {
			return FALSE;
		}
		if (!file_bool_write(fileh, ptm_mode_16bit[i])) {
			return FALSE;
		}
		if (!file_bool_write(fileh, ptm_clock_type[i])) {
			return FALSE;
		}
	}

	if (!file_bool_write(fileh, ptm_preset_mode)) {
		return FALSE;
	}
	if (!file_bool_write(fileh, ptm_select_reg1)) {
		return FALSE;
	}
	if (!file_bool_write(fileh, ptm_clock_divide)) {
		return FALSE;
	}

	return TRUE;
}

/*
 *	�v���O���}�u���^�C�}
 *	���[�h
 */
BOOL FASTCALL ptm_load(int fileh, int ver)
{
	int	i;

	/* �o�[�W�����`�F�b�N */
#if XM7_VER == 1
	if (ver < 309) {
#elif XM7_VER == 2
	if ((ver >= 500) && (ver < 719)) {
#else
	if (((ver >= 500) && (ver < 719)) || ((ver >= 800) && (ver < 919))) {
#endif
		return TRUE;
	}

	for (i=0; i<6; i++) {
		if (!file_dword_read(fileh, (DWORD *)&ptm_counter[i])) {
			return FALSE;
		}
		if (!file_dword_read(fileh, (DWORD *)&ptm_counter_preset[i])) {
			return FALSE;
		}
	}

	for (i=0; i<3; i++) {
		if (!file_byte_read(fileh, &ptm_mode_select[i])) {
			return FALSE;
		}
		if (!file_bool_read(fileh, &ptm_running_flag[i])) {
			return FALSE;
		}

		if (!file_bool_read(fileh, &ptm_out_flag[i])) {
			return FALSE;
		}
		if (!file_bool_read(fileh, &ptm_irq_flag_int[i])) {
			return FALSE;
		}
		if (!file_bool_read(fileh, &ptm_irq_mask_int[i])) {
			return FALSE;
		}
		if (!file_bool_read(fileh, &ptm_mode_16bit[i])) {
			return FALSE;
		}
		if (!file_bool_read(fileh, &ptm_clock_type[i])) {
			return FALSE;
		}
	}

	if (!file_bool_read(fileh, &ptm_preset_mode)) {
		return FALSE;
	}
	if (!file_bool_read(fileh, &ptm_select_reg1)) {
		return FALSE;
	}
	if (!file_bool_read(fileh, &ptm_clock_divide)) {
		return FALSE;
	}

	/* �C�x���g */
	schedule_handle(EVENT_PTM_TIMER, ptm_event);

	return TRUE;
}

#endif
