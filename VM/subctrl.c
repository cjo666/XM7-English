/*
 *	FM-7 EMULATOR "XM7"
 *
 *	Copyright (C) 1999-2017 �o�h�D(Twitter:@xm6_original)
 *	Copyright (C) 2001-2017 Ryu Takegami (Twitter:@RyuTakegami)
 *
 *	[ �T�uCPU�R���g���[�� ]
 *
 *	RHG����
 *	  2001.07.27		display_setpointer(��display_setparam)�̎d�l�ύX�ɔ���
 *						�ύX
 *	  2001.08.14		BUSY�M���̒x�����s���悤�ɕύX
 *	  2001.08.27		��ʃ��[�h�؂�ւ����̕s�v�ȍĕ`���}��
 *	  2001.09.02		�T�u�o���N�؂�ւ�������ALU�̃��Z�b�g��ǉ�
 *	  2001.11.24		�T�uHALT�^�C�~���O��ύX
 *	  2001.04.01		�X�e�[�g�t�@�C��Ver7.01�̃��[�h�Ɏ��s��������C��
 *	  2002.12.04		�z�b�g���Z�b�g�p�̍׍H��ǉ�
 *	  2003.03.29		�X�e�[�g���[�h���̃T�uI/F���W�X�^�̕��A�ɑΉ�
 *	  2003.06.02		CLR���߂�BUSY�t���O�𗎂Ƃ��ƃf�b�h���b�N������ɑΏ�
 *	  2003.11.21		$FD04 bit5(����ROM�ڑ��̐؂芷���@�\)�ɑΉ�
 */

#include <string.h>
#include "xm7.h"
#include "keyboard.h"
#include "subctrl.h"
#include "device.h"
#include "display.h"
#include "aluline.h"
#include "ttlpalet.h"
#include "multipag.h"
#include "fdc.h"
#include "mmr.h"
#include "kanji.h"

/*
 *	�O���[�o�� ���[�N
 */
BOOL subhalt_flag;						/* �T�uHALT�t���O */
BOOL subbusy_flag;						/* �T�uBUSY�t���O */
BOOL subcancel_flag;					/* �T�u�L�����Z���t���O */
BOOL subattn_flag;						/* �T�u�A�e���V�����t���O */
BOOL subhalt_request;					/* �T�uHALT���N�G�X�g�t���O */
BOOL subcancel_request;					/* �T�u�L�����Z�����N�G�X�g�t���O */
BYTE shared_ram[0x80];					/* ���LRAM */
BOOL subreset_flag;						/* �T�u�ċN���t���O */
BOOL extdet_disable;					/* EXTDET�f�B�Z�[�u�� */
BYTE busy_CLR_count;					/* BUSY($D40A) CLR���ߎ��s���J�E���^ */
#if XM7_VER >= 2
BOOL mode320;							/* 320x200���[�h */
#if XM7_VER >= 3
BOOL mode400l;							/* 640x400���[�h */
BOOL mode256k;							/* 26���F���[�h */
BOOL subram_protect;					/* �T�u���j�^RAM�v���e�N�g */
BOOL subreset_halt;						/* HALT���ċN���t���O */
BYTE subif_dat;							/* �T�uI/F���W�X�^ ��ʃ��[�h�ۑ� */
BOOL subkanji_flag;						/* ����ROM �T�uCPU�ڑ��t���O */
BOOL submode_fix;						/* FM77AV�pOS-9�΍�t���O */
#endif
#endif
#if XM7_VER == 1 && defined(L4CARD)
BOOL select_400line;					/* ���C����400���C���I���t���O */
BOOL subkanji_flag;						/* ����ROM �T�uCPU�ڑ��t���O */
#endif


/*
 *	�T�uCPU�R���g���[��
 *	������
 */
BOOL FASTCALL subctrl_init(void)
{
#if XM7_VER == 3
	submode_fix = FALSE;
#endif
	extdet_disable = FALSE;

	return TRUE;
}

/*
 *	�T�uCPU�R���g���[��
 *	�N���[���A�b�v
 */
void FASTCALL subctrl_cleanup(void)
{
}

/*
 *	�T�uCPU�R���g���[��
 *	���Z�b�g
 */
void FASTCALL subctrl_reset(void)
{
	subhalt_request = FALSE;
	subhalt_flag = FALSE;
	subbusy_flag = TRUE;
	subcancel_request = FALSE;
	subcancel_flag = FALSE;
	subattn_flag = FALSE;
	subreset_flag = FALSE;
	busy_CLR_count = 0;
#if XM7_VER >= 2
	mode320 = FALSE;
#if XM7_VER >= 3
	mode400l = FALSE;
	mode256k = FALSE;
	screen_mode = SCR_200LINE;
	subram_protect = TRUE;
	subreset_halt = FALSE;
	subkanji_flag = FALSE;
	subif_dat = 0x08;
#endif
#endif

#if XM7_VER == 1 && defined(L4CARD)
	select_400line = FALSE;
	enable_400line = FALSE;
	subkanji_flag = FALSE;
#endif

	memset(shared_ram, 0xff, sizeof(shared_ram));
}

/*
 *	�T�uCPU�R���g���[��
 *	HALT/CANCEL �A�N�m���b�W
 */
void FASTCALL subctrl_halt_ack(void)
{
	subhalt_flag = subhalt_request;
	subcancel_flag = subcancel_request;

	/* HALT���ɂ�BUSY�M����ON�ɂ��� */
	if (subhalt_request) {
		subbusy_flag = TRUE;
	}
}

/*
 *	�T�uCPU�R���g���[��
 *	�P�o�C�g�ǂݏo��
 */
BOOL FASTCALL subctrl_readb(WORD addr, BYTE *dat)
{
	BYTE ret;

	switch (addr) {
		/* �T�uCPU �A�e���V�������荞�݁ABreak�L�[���荞�� */
		case 0xfd04:
			/* BUSY�t���O */
			if (subbusy_flag) {
				ret = 0xff;
			}
			else {
				ret = 0x7f;
			}
			/* �A�e���V�����t���O */
			if (subattn_flag) {
				ret &= ~0x01;
				subattn_flag = FALSE;
			}
			/* Break�L�[�t���O */
			if (break_flag || hotreset_flag) {
				ret &= ~0x02;

				/* �z�b�g���Z�b�g���� */
				if (hotreset_flag) {
#if XM7_VER >= 2
					/* �C�j�V�G�[�^ROM�������Ȃ�z�b�g���Z�b�g���� */
					if (!initrom_en) {
						break_flag = FALSE;
						hotreset_flag = FALSE;
					}
#else
					break_flag = FALSE;
					hotreset_flag = FALSE;
#endif
				}
			}

#if XM7_VER == 3
			/* FM77AV�pOS-9�΍�t���O�̏��� */
			if (submode_fix) {
				ret &= ~0x10;
			}
#endif

#if XM7_VER == 1 && defined(L4CARD)
			/* �ȉ��AFM-77���[�h�̂ݗL�� */
			if (fm_subtype == FMSUB_FM77) {
				/* ����ROM�؂芷�� */
				if (subkanji_flag) {
					ret &= ~0x20;
				}

				/* 400���C���J�[�h */
				if (enable_400linecard) {
					ret &= ~0x10;
				}

				/* 400���C���J�[�h���[�h */
				if (enable_400linecard && select_400line) {
					ret &= ~0x08;
				}
			}
#endif

			*dat = ret;
			maincpu_firq();
			return TRUE;

		/* �T�u�C���^�t�F�[�X */
		case 0xfd05:
#if XM7_VER == 1
			if (extdet_disable || (!fdc_enable && (fm_subtype != FMSUB_FM77))){
#else
			if (extdet_disable || (!fdc_enable && (fm7_ver == 1))) {
#endif
				ret = 0xff;
			}
			else {
				ret = 0xfe;
			}
			if (subbusy_flag) {
				ret &= (BYTE)0xff;
			}
			else {
				ret &= (BYTE)0x7f;
			}

			*dat = ret;
			return TRUE;

		/* �T�u���[�h�X�e�[�^�X */
#if XM7_VER >= 2
		case 0xfd12:
			ret = 0xff;
			if (fm7_ver >= 2) {
				/* 320/640 */
				if (!mode320) {
					ret &= ~0x40;
				}

				/* �u�����N�X�e�[�^�X */
				if (blank_flag) {
					ret &= ~0x02;
				}
				/* VSYNC�X�e�[�^�X */
				if (!vsync_flag) {
					ret &= ~0x01;
				}
			}
			*dat = ret;
			return TRUE;

		/* �T�u�o���N�؂�ւ�(Write Only) */
		case 0xfd13:
			*dat = 0xff;
			return TRUE;
#endif
	}

	return FALSE;
}

/*
 *	�T�uCPU�R���g���[��
 *	�P�o�C�g��������
 */
BOOL FASTCALL subctrl_writeb(WORD addr, BYTE dat)
{
	switch (addr) {
#if XM7_VER >= 3
		/* �T�u�C���^�t�F�[�X AV40�g������ */
		case 0xfd04:
			if (fm7_ver >= 3) {
				/* bit2:�T�u���j�^�v���e�N�g */
				if (dat & 0x04) {
					subram_protect = FALSE;
				}
				else {
					subram_protect = TRUE;
				}
				/* bit3:400���C�����[�h�I�� */
				if (dat & 0x08) {
					mode400l = FALSE;
				}
				else {
					mode400l = TRUE;
				}
				/* bit4:26���F���[�h�I�� */
				/*      bit3��4�̗�����ON�������ꍇ�Amode400l��D�� */
				if ((dat & 0x10) && !mode400l) {
					mode256k = TRUE;
				}
				else {
					mode256k = FALSE;
				}
				/* bit5:����ROM�ڑ��؂芷�� */
				if (dat & 0x20) {
					subkanji_flag = FALSE;
				}
				else {
					subkanji_flag = TRUE;
				}

				/* ��ʃ��[�h���؂�ւ�����ꍇ�����ĕ`�� */
				if ((BYTE)(dat & 0x18) != subif_dat) {
					display_setpointer(TRUE);
				}
				subif_dat = (BYTE)(dat & 0x18);
			}
			return TRUE;
#endif

#if XM7_VER == 1 && defined(L4CARD)
		/* �T�u�C���^�t�F�[�X FM-77�g������ */
		case 0xfd04:
			/* �ȉ��AFM-77���[�h�̂ݗL�� */
			if (fm_subtype == FMSUB_FM77) {
				/* bit3:400���C���J�[�h���[�h */
				if (dat & 0x08) {
					select_400line = FALSE;
				}
				else {
					select_400line = TRUE;
				}

				/* bit5:����ROM�ڑ��؂芷�� */
				if (dat & 0x20) {
					subkanji_flag = FALSE;
				}
				else {
					subkanji_flag = TRUE;
				}
			}
			return TRUE;
#endif

		/* �T�u�R���g���[�� */
		case 0xfd05:
#if XM7_VER == 1 && defined(Z80CARD)
			if (dat & 0x01) {
				if (!main_z80mode) {
					mmr_modify = TRUE;
				}
				main_z80mode = TRUE;
			}
			else {
				if (main_z80mode) {
					mmr_modify = TRUE;
				}
				main_z80mode = FALSE;
			}
#endif
			if (dat & 0x80) {
				/* �T�uHALT���N�G�X�g */
				subhalt_request = TRUE;
			}
			else {
				/* �T�uRUN���N�G�X�g */
				subhalt_request = FALSE;

				/* �o���N�؂�ւ���́A���Z�b�g���� */
#if XM7_VER >= 3

				if (subreset_halt) {
					subcpu_reset();
					subreset_halt = FALSE;
				}
#endif
			}
			if (dat & 0x40) {
				/* �L�����Z��IRQ */
				subcancel_request = TRUE;
			}
			subcpu_irq();
			return TRUE;

		/* �T�u���[�h�؂�ւ� */
#if XM7_VER >= 2
		case 0xfd12:
			if (fm7_ver >= 2) {
				/* ��ʃ��[�h���؂�ւ�����ꍇ�����ĕ`�� */
				if (dat & 0x40) {
					if (!mode320) {
						mode320 = TRUE;
						display_setpointer(TRUE);
					}
				}
				else {
					if (mode320) {
						mode320 = FALSE;
						display_setpointer(TRUE);
					}
				}
			}
			return TRUE;

		/* �T�u�o���N�؂�ւ� */
		case 0xfd13:
			if (fm7_ver >= 2) {
				/* �o���N�؂�ւ� */
#if XM7_VER >= 3
				if ((fm7_ver >= 3) && (dat & 0x04)) {
					subrom_bank = 4;
				}
				else {
					subrom_bank = (BYTE)(dat & 0x03);
				}
#else
				subrom_bank = (BYTE)(dat & 0x03);
#endif

				/* ���Z�b�g */
#if XM7_VER >= 3
				if (!subhalt_flag) {
					subcpu_reset();
					subreset_halt = FALSE;
				}
				else {
					subreset_halt = TRUE;
				}
#else
				subcpu_reset();
#endif

				/* �t���O�ރZ�b�g */
				subreset_flag = TRUE;
				subbusy_flag = TRUE;

				/* �\���nI/O�����Z�b�g���� (FM77AV �f���v���O����) */
				aluline_reset();
				ttlpalet_reset();
				multipag_reset();

				/* CRT���W�X�^�����Z�b�g���� */
				display_reset();
				display_notify();

				/* INS LED������������ */
				ins_flag = FALSE;
			}
			return TRUE;
#endif
	}

	return FALSE;
}

/*
 *	�T�uCPU�R���g���[��
 *	�Z�[�u
 */
BOOL FASTCALL subctrl_save(int fileh)
{
	if (!file_bool_write(fileh, subhalt_flag)) {
		return FALSE;
	}

	if (!file_bool_write(fileh, subbusy_flag)) {
		return FALSE;
	}

	if (!file_bool_write(fileh, subcancel_flag)) {
		return FALSE;
	}

	if (!file_bool_write(fileh, subattn_flag)) {
		return FALSE;
	}

	if (!file_write(fileh, shared_ram, 0x80)) {
		return FALSE;
	}

	if (!file_bool_write(fileh, subreset_flag)) {
		return FALSE;
	}

#if XM7_VER >= 2
	if (!file_bool_write(fileh, mode320)) {
		return FALSE;
	}
#endif

	/* Ver7�g�� */
	if (!file_bool_write(fileh, subhalt_request)) {
		return FALSE;
	}
	if (!file_bool_write(fileh, subcancel_request)) {
		return FALSE;
	}

	/* Ver711�g�� */
	if (!file_byte_write(fileh, busy_CLR_count)) {
		return FALSE;
	}

#if XM7_VER >= 3
	/* Ver8�g�� */
	if (!file_bool_write(fileh, mode400l)) {
		return FALSE;
	}
	if (!file_bool_write(fileh, mode256k)) {
		return FALSE;
	}
	if (!file_bool_write(fileh, subram_protect)) {
		return FALSE;
	}
	if (!file_bool_write(fileh, subreset_halt)) {
		return FALSE;
	}
	if (!file_bool_write(fileh, subkanji_flag)) {
		return FALSE;
	}
#endif

#if XM7_VER == 1 && defined(L4CARD)
	/* XM7 V1.1 / FM-77L4�g�� */
	if (!file_bool_write(fileh, select_400line)) {
		return FALSE;
	}
	if (!file_bool_write(fileh, subkanji_flag)) {
		return FALSE;
	}
#endif

	return TRUE;
}

/*
 *	�T�uCPU�R���g���[��
 *	���[�h
 */
BOOL FASTCALL subctrl_load(int fileh, int ver)
{
	/* �o�[�W�����`�F�b�N */
	if (ver < 200) {
		return FALSE;
	}

	if (!file_bool_read(fileh, &subhalt_flag)) {
		return FALSE;
	}

	if (!file_bool_read(fileh, &subbusy_flag)) {
		return FALSE;
	}

	if (!file_bool_read(fileh, &subcancel_flag)) {
		return FALSE;
	}

	if (!file_bool_read(fileh, &subattn_flag)) {
		return FALSE;
	}

	if (!file_read(fileh, shared_ram, 0x80)) {
		return FALSE;
	}

	if (!file_bool_read(fileh, &subreset_flag)) {
		return FALSE;
	}

#if XM7_VER >= 2
	if (!file_bool_read(fileh, &mode320)) {
		return FALSE;
	}
#endif

	/* Ver7�g�� */
#if XM7_VER >= 3
	if (((ver >= 700) && (ver <= 799)) || (ver >= 900)) {
#elif XM7_VER >= 2
	if (ver >= 700) {
#else
	if (ver >= 300) {
#endif
		if (!file_bool_read(fileh, &subhalt_request)) {
			return FALSE;
		}
		if (!file_bool_read(fileh, &subcancel_request)) {
			return FALSE;
		}
	}
	else {
		subhalt_request = subhalt_flag;
		subcancel_request = subcancel_flag;
	}

	/* Ver711�g�� */
#if XM7_VER >= 3
	if (((ver >= 711) && (ver <= 799)) || (ver >= 911)) {
#elif XM7_VER >= 2
	if (ver >= 711) {
#else
	if (ver >= 300) {
#endif
		if (!file_byte_read(fileh, &busy_CLR_count)) {
			return FALSE;
		}
	}
	else {
		busy_CLR_count = 0;
	}

#if XM7_VER >= 3
	/* Ver8�g�� */
	if (ver < 800) {
		mode400l = FALSE;
		mode256k = FALSE;
		subif_dat = 0x08;
		subram_protect = FALSE;
		subreset_halt = FALSE;

		/* �|�C���^���\�� */
		display_setpointer(TRUE);
		return TRUE;
	}

	if (!file_bool_read(fileh, &mode400l)) {
		return FALSE;
	}
	if (!file_bool_read(fileh, &mode256k)) {
		return FALSE;
	}
	if (!file_bool_read(fileh, &subram_protect)) {
		return FALSE;
	}
	if (!file_bool_read(fileh, &subreset_halt)) {
		return FALSE;
	}

	/* Ver9.06�g�� */
	if (ver >= 906) {
		if (!file_bool_read(fileh, &subkanji_flag)) {
			return FALSE;
		}
	}
	else {
		if (mode400l) {
			subkanji_flag = TRUE;
		}
		else {
			subkanji_flag = FALSE;
		}
	}

	/* �T�uI/F Reg. ��ʃ��[�h���A */
	subif_dat = 0x00;
	if (!mode400l) {
		subif_dat |= (BYTE)0x08;
	}
	if (mode256k) {
		subif_dat |= (BYTE)0x10;
	}
#endif

#if XM7_VER == 1 && defined(L4CARD)
	/* XM7 V1.1 / FM-77L4�g�� */
	if (!file_bool_read(fileh, &select_400line)) {
		return FALSE;
	}
	/* 400���C���J�[�h�������`�F�b�N */
	if (select_400line && !detect_400linecard) {
		return FALSE;
	}
	if (!file_bool_read(fileh, &subkanji_flag)) {
		return FALSE;
	}
#endif

	/* �|�C���^���\�� */
	display_setpointer(TRUE);

	return TRUE;
}
