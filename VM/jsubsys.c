/*
 *	FM-7 EMULATOR "XM7"
 *
 *	Copyright (C) 1999-2017 �o�h�D(Twitter:@xm6_original)
 *	Copyright (C) 2001-2017 Ryu Takegami (Twitter:@RyuTakegami)
 *
 *	[ ���{��ʐM�J�[�h ]
 *
 *	RHG����
 *	  2008.01.24	kanji.c���烁�C����I/F���𕪗�
 *	  2010.12.09	���{��ʐM�J�[�h������ROM�̃A�N�Z�X�ɑΉ�
 */

#if XM7_VER == 1
#if defined(JSUB)

#include <stdlib.h>
#include <string.h>
#include "xm7.h"
#include "jsubsys.h"
#include "kanji.h"
#include "mainetc.h"
#include "device.h"
#include "event.h"

/*
 *	�O���[�o�� ���[�N
 */
cpu6809_t jsubcpu;					/* ���{��T�uCPU���W�X�^��� */
BYTE *jsubrom;						/* ���{��T�u���j�^ROM */
BYTE *jdicrom;						/* ����ROM(���{��ʐM�J�[�h) */
BYTE *jsub_sram;					/* ���[�NSRAM */
BOOL jsub_available;				/* ���{��T�u�V�X�e���g�p�\�t���O */
BOOL jsub_enable;					/* ���{��T�u�V�X�e���g�p�t���O */
BOOL jsub_haltflag;					/* ���{��T�u�V�X�e��HALT�t���O */
BYTE jsub_dicbank;					/* ����ROM�o���N */
BYTE jsub_address;					/* RCB�A�h���X�J�E���^ */
WORD jsub_kanji_addr;				/* ����ROM�A�h���X���W�X�^ */

/*
 *	�v���g�^�C�v�錾
 */
void jsub_reset(void);
void jsub_line(void);
void jsub_exec(void);
BOOL jsubcpu_event(void);

/*
 *	���{��T�uCPU
 *	������
 */
BOOL FASTCALL jsubsys_init(void)
{
	/* ��x�A�S�ăN���A */
	jsubrom = NULL;
	jdicrom = NULL;
	jsub_sram = NULL;
	jsub_available = TRUE;
	jsub_enable = TRUE;
	jsub_address = 0;

	/* �������m�� */
	jsubrom = (BYTE *)malloc(0x4000);
	if (jsubrom == NULL) {
		return FALSE;
	}
	jdicrom = (BYTE *)malloc(0x40000);
	if (jdicrom == NULL) {
		return FALSE;
	}
	jsub_sram = (BYTE *)malloc(0x2000);
	if (jsub_sram == NULL) {
		return FALSE;
	}

	/* ROM�t�@�C���ǂݍ��� */
	if (!file_load(JSUBSYS_ROM, jsubrom, 0x4000)) {
		jsub_available = FALSE;
	}
	if (!file_load(DICT_ROM, jdicrom, 0x40000)) {
		jsub_available = FALSE;
	}

	/* V1L2�����Ƀp�b�`���� */
	if (jsubrom[0x000d] == 0x8f) {
		jsubrom[0x000d] = 0x88;
	}

	/* �������A�N�Z�X�֐��ݒ� */
	jsubcpu.readmem = jsubmem_readb;
	jsubcpu.writemem = jsubmem_writeb;

	return TRUE;
}

/*
 *	���{��T�uCPU
 *	�N���[���A�b�v
 */
void FASTCALL jsubsys_cleanup(void)
{
	ASSERT(jsubrom);
	ASSERT(jdicrom);
	ASSERT(jsub_sram);

	if (jsubrom) {
		free(jsubrom);
	}
	if (jdicrom) {
		free(jdicrom);
	}
	if (jsub_sram) {
		free(jsub_sram);
	}
}

/*
 *	���{��T�uCPU
 *	���Z�b�g
 */
void FASTCALL jsubsys_reset(void)
{
	jsub_address = 0;
	jsub_haltflag = FALSE;
	jsub_kanji_addr = 0;

	if (jsub_available) {
		jsub_reset();
	}
}

/*
 *	���{��T�uCPU
 *	�P�s���s
 */
void FASTCALL jsubcpu_execline(void)
{
	if (jsub_available && jsub_enable) {
		jsub_line();
	}
}

/*
 *	���{��T�uCPU
 *	���s
 */
void FASTCALL jsubcpu_exec(void)
{
	if (jsub_available && jsub_enable) {
		jsub_exec();
	}
}

/*
 *	���{��T�uCPU
 *	NMI���荞�ݐݒ�
 */
void FASTCALL jsubcpu_nmi(void)
{
}

/*
 *	���{��T�uCPU
 *	FIRQ���荞�ݐݒ�
 */
void FASTCALL jsubcpu_firq(void)
{
}

/*
 *	���{��T�uCPU
 *	IRQ���荞�ݐݒ�
 */
void FASTCALL jsubcpu_irq(void)
{
}

/*
 *	���{��T�uCPU������
 *	�P�o�C�g�擾
 */
BYTE FASTCALL jsubmem_readb(WORD addr)
{
	/* ���{��T�u���j�^ROM */
	if (addr >= 0xc000) {
		return jsubrom[addr - 0xc000];
	}

	/* ����ROM */
	if ((addr >= 0xa000) && (addr <= 0xafff)) {
		return jdicrom[(addr & 0xfff) | (jsub_dicbank << 12)];
	}

	/* SRAM */
	if ((addr >= 0x8000) && (addr <= 0x9ffe)) {
		return jsub_sram[addr - 0x8000];
	}

	/* �����o���N�E�����t���O���W�X�^ */
	if (addr == 0x9fff) {
		return jsub_dicbank;
	}

	return 0xff;
}

/*
 *	���{��T�uCPU������
 *	�P�o�C�g�擾(I/O�Ȃ�)
 */
BYTE FASTCALL jsubmem_readbnio(WORD addr)
{
	/* ���{��T�u���j�^ROM */
	if (addr >= 0xc000) {
		return jsubrom[addr - 0xc000];
	}

	/* ����ROM */
	if ((addr >= 0xa000) && (addr <= 0xafff)) {
		return jdicrom[(addr & 0xfff) | (jsub_dicbank << 12)];
	}

	/* SRAM */
	if ((addr >= 0x8000) && (addr <= 0x9ffe)) {
		return jsub_sram[addr - 0x8000];
	}

	/* �����o���N�E�����t���O���W�X�^ */
	if (addr == 0x9fff) {
		return (BYTE)jsub_dicbank;
	}

	ASSERT(FALSE);
	return 0xff;
}

/*
 *	���{��T�uCPU������
 *	�P�o�C�g��������
 */
void FASTCALL jsubmem_writeb(WORD addr, BYTE dat)
{
	/* �����o���N�E�����t���O���W�X�^ */
	if (addr == 0x9fff) {
		if (dat & 0x80) {
			jsub_haltflag = FALSE;
		}
		else {
			jsub_haltflag = TRUE;
		}

		jsub_dicbank = (BYTE)(dat & 0x3f);
		return;
	}

	/* SRAM */
	if ((addr >= 0x8000) && (addr <= 0x9ffe)) {
		jsub_sram[addr - 0x8000] = dat;
		return;
	}

	return;
}

/*
 *	���{��T�u�V�X�e��
 *	���C���������RCB�P�o�C�g�ǂݍ���
 */
BYTE FASTCALL jsub_readrcb(void)
{
	return jsub_sram[0x1f00 | (jsub_address++)];
}

/*
 *	���{��T�u�V�X�e��
 *	���C���������RCB�P�o�C�g��������
 */
void FASTCALL jsub_writercb(BYTE dat)
{
	jsub_sram[0x1f00 | (jsub_address++)] = dat;
}

/*
 *	���{��T�u�V�X�e��
 *	�A�h���X�J�E���^�N���A
 */
void FASTCALL jsub_clear_address(void)
{
	jsub_address = 0;
}

/*
 *	���{��T�u�V�X�e���C���^�t�F�[�X
 *	�P�o�C�g�ǂݏo��
 */
BOOL FASTCALL jsub_readb(WORD addr, BYTE *dat)
{
	int offset;

	/* FM-8���[�h���E���{��ʐM�J�[�h�������͖��� */
	if ((fm_subtype == FMSUB_FM8) || !jsub_enable) {
		return FALSE;
	}

	switch (addr) {
		/* ���{��T�u�V�X�e�������t���O */
		case 0xfd28:
			*dat = 0xff;
			if (jsub_haltflag) {
				*dat &= (BYTE)~0x80;
			}
			return TRUE;
	
		/* RCB�f�[�^�ǂݍ��� */
		case 0xfd29:
			if (jsub_haltflag) {
				*dat = jsub_readrcb();
			}
			else {
				*dat = 0xff;
			}
			return TRUE;

		/* ��1�����f�[�^ */
		case 0xfd2a:		/* ��1�f�[�^LEFT */
		case 0xfd2b:		/* ��1�f�[�^RIGHT */
			if (fm_subtype == FMSUB_FM77) {
				offset = jsub_kanji_addr << 1;
				if ((offset >= 0x6000) && (offset < 0x8000) &&
					 !kanji_asis_flag) {
					/* $6000�`$7FFF�͖���`�̈� */
					*dat = (BYTE)(addr & 1);
				}
				else {
					/* �ʏ�̈� */
					*dat = kanji_rom[offset + (addr & 1)];
				}
				return TRUE;
			}

			return FALSE;
	}

	return FALSE;
}

/*
 *	���{��T�u�V�X�e���C���^�t�F�[�X
 *	�P�o�C�g��������
 */
BOOL FASTCALL jsub_writeb(WORD addr, BYTE dat)
{
	/* FM-8���[�h���E���{��ʐM�J�[�h�������͖��� */
	if ((fm_subtype == FMSUB_FM8) || !jsub_enable) {
		return FALSE;
	}

	switch (addr) {
		/* �A�h���X��� */
		case 0xfd28:
			jsub_kanji_addr &= 0x00ff;
			jsub_kanji_addr |= (WORD)(dat << 8);
			return TRUE;

		/* �A�h���X���� */
		case 0xfd29:
			jsub_kanji_addr &= 0xff00;
			jsub_kanji_addr |= dat;
			return TRUE;

		/* ���{��T�u�V�X�e�������t���O */
		case 0xfd2a:
			if (dat & 0x80) {
				jsub_haltflag = FALSE;
			}
			else {
				jsub_haltflag = TRUE;
				jsub_clear_address();
			}
			return TRUE;

		/* RCB�f�[�^�������� */
		case 0xfd2b:
			if (jsub_haltflag) {
				jsub_writercb(dat);
			}
			return TRUE;
	}

	return FALSE;
}

/*
 *	���{��T�u�V�X�e��
 *	�Z�[�u
 */
BOOL FASTCALL jsubsys_save(int fileh)
{
	/* �v���b�g�t�H�[�����Ƃ̃p�b�L���O����������邽�߁A���� */
	if (!file_byte_write(fileh, jsubcpu.cc)) {
		return FALSE;
	}

	if (!file_byte_write(fileh, jsubcpu.dp)) {
		return FALSE;
	}

	if (!file_word_write(fileh, jsubcpu.acc.d)) {
		return FALSE;
	}

	if (!file_word_write(fileh, jsubcpu.x)) {
		return FALSE;
	}

	if (!file_word_write(fileh, jsubcpu.y)) {
		return FALSE;
	}

	if (!file_word_write(fileh, jsubcpu.u)) {
		return FALSE;
	}

	if (!file_word_write(fileh, jsubcpu.s)) {
		return FALSE;
	}

	if (!file_word_write(fileh, jsubcpu.pc)) {
		return FALSE;
	}

	if (!file_word_write(fileh, jsubcpu.intr)) {
		return FALSE;
	}

	if (!file_word_write(fileh, jsubcpu.cycle)) {
		return FALSE;
	}

	if (!file_word_write(fileh, jsubcpu.total)) {
		return FALSE;
	}

	if (!file_bool_write(fileh, jsub_enable)) {
		return FALSE;
	}
	if (!file_bool_write(fileh, jsub_haltflag)) {
		return FALSE;
	}
	if (!file_byte_write(fileh, jsub_dicbank)) {
		return FALSE;
	}
	if (!file_byte_write(fileh, jsub_address)) {
		return FALSE;
	}
	if (!file_write(fileh, jsub_sram, 0x2000)) {
		return FALSE;
	}
	if (!file_word_write(fileh, jsub_kanji_addr)) {
		return FALSE;
	}

	return TRUE;
}

/*
 *	���{��T�u�V�X�e��
 *	���[�h
 */
BOOL FASTCALL jsubsys_load(int fileh, int ver)
{
	/* �o�[�W�����`�F�b�N */
	if (ver < 200) {
		return FALSE;
	}

	if (ver >= 301 && ver <= 499) {
		/* �v���b�g�t�H�[�����Ƃ̃p�b�L���O����������邽�߁A���� */
		if (!file_byte_read(fileh, &jsubcpu.cc)) {
			return FALSE;
		}

		if (!file_byte_read(fileh, &jsubcpu.dp)) {
			return FALSE;
		}

		if (!file_word_read(fileh, &jsubcpu.acc.d)) {
			return FALSE;
		}

		if (!file_word_read(fileh, &jsubcpu.x)) {
			return FALSE;
		}

		if (!file_word_read(fileh, &jsubcpu.y)) {
			return FALSE;
		}

		if (!file_word_read(fileh, &jsubcpu.u)) {
			return FALSE;
		}

		if (!file_word_read(fileh, &jsubcpu.s)) {
			return FALSE;
		}

		if (!file_word_read(fileh, &jsubcpu.pc)) {
			return FALSE;
		}

		if (!file_word_read(fileh, &jsubcpu.intr)) {
			return FALSE;
		}

		if (!file_word_read(fileh, &jsubcpu.cycle)) {
			return FALSE;
		}

		if (!file_word_read(fileh, &jsubcpu.total)) {
			return FALSE;
		}

		if (!file_bool_read(fileh, &jsub_enable)) {
			return FALSE;
		}
		if (!file_bool_read(fileh, &jsub_haltflag)) {
			return FALSE;
		}
		if (!file_byte_read(fileh, &jsub_dicbank)) {
			return FALSE;
		}
		if (!file_byte_read(fileh, &jsub_address)) {
			return FALSE;
		}
		if (!file_read(fileh, jsub_sram, 0x2000)) {
			return FALSE;
		}
		if (ver >= 305 && ver <= 499) {
			if (!file_word_read(fileh, &jsub_kanji_addr)) {
				return FALSE;
			}
		}
	}

	/* ���{��T�u���g�p�s�̏ꍇ�f�B�Z�[�u�� */
	if (!jsub_available) {
		jsub_enable = FALSE;
	}

	return TRUE;
}

#endif
#endif
