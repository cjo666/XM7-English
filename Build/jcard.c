/*
 *	FM-7 EMULATOR "XM7"
 *
 *	Copyright (C) 1999-2017 �o�h�D(Twitter:@xm6_original)
 *	Copyright (C) 2001-2017 Ryu Takegami (Twitter:@RyuTakegami)
 *
 *	[ ���{��J�[�h / �g���T�u�V�X�e��ROM ]
 *
 *	RHG����
 *	  2003.03.29		�����E�g��ROM�̓ǂݏo�������𐮗�
 *	  2005.10.07		�g��ROM�G���A�̓��e��ǂݍ��ݎ��ɓW�J����悤�ɕύX
 *	  2010.12.09		�g��ROM�G���A�̊���ROM�̈��JIS78�����ɕύX
 *	  2010.12.19		�g��ROM�G���A�̃u�[�g�̈���B���u�[�gROM�Ή��ɕύX
 *	  2012.06.29		�g��ROM�G���A�̊����̈�̊����̈�(?)��JIS78�����ɏC��
 */

#if XM7_VER >= 2

#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "xm7.h"
#include "mmr.h"
#include "device.h"
#include "kanji.h"
#include "jcard.h"

/*
 *	�O���[�o�� ���[�N
 */
BYTE *dicrom;							/* ����ROM $40000 */
BYTE *dicram;							/* �w�KRAM $2000 */
BYTE *extram_b;							/* �g��RAM $10000 */

BYTE dicrom_bank;						/* ����ROM�o���N�ԍ� */
#if XM7_VER == 3
BOOL extrom_sel;						/* ����ROM/�g��ROM�I���t���O */
#endif
BOOL dicrom_en;							/* ����ROM�A�N�e�B�u */
BOOL dicram_en;							/* �w�KRAM�A�N�e�B�u */

#if XM7_VER == 3
BYTE *extrom;							/* �g��ROM $20000 */
#endif

#if XM7_VER == 2
BOOL jcard_available;					/* ���{��J�[�h�g�p�ۃt���O */
#endif
BOOL jcard_enable;						/* ���{��J�[�h�L���t���O */


/*
 *	���{��J�[�h
 *	������
 */
BOOL FASTCALL jcard_init(void)
{
	/* ���[�N�G���A������ */
	extram_b = NULL;
	dicrom = NULL;
	dicram = NULL;
#if XM7_VER >= 3
	extrom = NULL;
#endif
#if XM7_VER == 2
	jcard_available = TRUE;
#endif
	jcard_enable = FALSE;

	/* ���{���� �g��RAM */
	extram_b = (BYTE *)malloc(0x10000);
	if (extram_b == NULL) {
#if XM7_VER == 2
		jcard_available = FALSE;
		return TRUE;
#else
		return FALSE;
#endif
	}

	/* ����ROM */
	dicrom = (BYTE *)malloc(0x40000);
	if (dicrom == NULL) {
#if XM7_VER == 2
		jcard_available = FALSE;
		return TRUE;
#else
		return FALSE;
#endif
	}
	if (!file_load(DICT_ROM, dicrom, 0x40000)) {
#if XM7_VER == 2
		jcard_available = FALSE;
		return TRUE;
#else
		return FALSE;
#endif
	}

	/* �w�KRAM�ǂݍ��� */
	dicram = (BYTE *)malloc(0x2000);
	if (dicram == NULL) {
#if XM7_VER == 2
		jcard_available = FALSE;
		return TRUE;
#else
		return FALSE;
#endif
	}
	if (!file_load(DICT_RAM, dicram, 0x2000)) {
		/* �t�@�C�������݂��Ȃ��B������ */
		memset(dicram, 0xff, 0x2000);
	}

	/* �g��ROM */
#if XM7_VER >= 3
	extrom = (BYTE *)malloc(0x20000);
	if (extrom == NULL) {
		return FALSE;
	}
	memset(extrom, 0xff, 0x20000);
	if (!file_load(EXTSUB_ROM, extrom, 0xc000)) {
		return FALSE;
	}
	else {
		/* �o���N56�`63��BASIC ROM�A�B���u�[�gROM�̓��e���R�s�[ */
		memcpy(&extrom[0x18000], basic_rom, 0x7c00);
		if (available_mmrboot) {
			memcpy(&extrom[0x1fe00], boot_mmr, 0x200);
		}
		else {
			memcpy(&extrom[0x1fe00], &init_rom[0x1a00], 0x1e0);
			memset(&extrom[0x1ffe0], 0, 32);
			extrom[0x1fffe] = 0xfe;
			extrom[0x1ffff] = 0x00;
		}
	}
#endif

	return TRUE;
}

/*
 *	���{��J�[�h
 *	�N���[���A�b�v
 */
void FASTCALL jcard_cleanup(void)
{
	ASSERT(extram_b);
	ASSERT(dicram);
	ASSERT(dicrom);
#if XM7_VER >= 3
	ASSERT(extrom);
#endif

	/* �������r���Ŏ��s�����ꍇ���l�� */
#if XM7_VER >= 3
	if (extrom) {
		free(extrom);
	}
#endif
	if (dicram) {
		/* �w�KRAM�̓��e���t�@�C���ɏ����o�� */
		file_save(DICT_RAM, dicram, 0x2000);
		free(dicram);
	}
	if (dicrom) {
		free(dicrom);
	}
	if (extram_b) {
		free(extram_b);
	}
}

/*
 *	���{��J�[�h
 *	���Z�b�g
 */
void FASTCALL jcard_reset(void)
{
	dicrom_bank = 0;
	dicram_en = FALSE;
	dicrom_en = FALSE;
#if XM7_VER >= 3
	extrom_sel = FALSE;
#endif
}

/*
 *	���{��J�[�h
 *	�P�o�C�g�ǂݍ���
 */
BYTE FASTCALL jcard_readb(WORD addr)
{
	DWORD dicrom_addr;

	/* FM77AV�V���[�Y�̂݃T�|�[�g */
	if (fm7_ver < 2) {
		return 0xff;
	}

#if XM7_VER == 2
	/* ���{��J�[�h���g���Ȃ��ꍇ��0xff��Ԃ����� */
	if (!jcard_available || !jcard_enable) {
		return 0xff;
	}
#else
	/* ���{��J�[�h��������0xff��Ԃ����� */
	if ((fm7_ver == 2) && !jcard_enable) {
		return 0xff;
	}
#endif

	/* $28000-$29FFF : �w�KRAM */
	if ((addr >= 0x8000) && (addr < 0xa000)) {
		if (dicram_en) {
			return dicram[addr - 0x8000];
		}
	}

	/* $2E000-$2EFFF : ����ROM or �g��ROM */
	if ((addr >= 0xe000) && (addr < 0xf000)) {
		/* ����ROM���L���� */
		if (dicrom_en) {
			addr &= (WORD)0x0fff;
			dicrom_addr = (dicrom_bank << 12);

#if XM7_VER >= 3
			/* �g��ROM���L���� */
			if (extrom_sel) {
				/* �o���N0�`31 : ��1��������(JIS78����) */
				if ((dicrom_bank >= 6) && (dicrom_bank < 8) &&
					!kanji_asis_flag) {
					return (BYTE)(addr & 1);
				}
				if (dicrom_bank < 32) {
					return kanji_rom_jis78[addr | dicrom_addr];
				}

				/* �o���N32�`43 : �g���T�u�V�X�e��ROM(extsub.rom) */
				/* �o���N56�`63 : F-BASIC V3.0 ROM ($8000-$EFFF) */
				/* �o���N63     : DOS���[�hBOOT���ۂ����� ($FE00-$FFDF) */
				/* �o���N63     : ���荞�݃x�N�^�̈� ($FFE0-$FFFF) */
				return extrom[addr | (dicrom_addr - 0x20000)];
			}
			else {
				/* ����ROM */
				return dicrom[addr | dicrom_addr];
			}
#else
			/* ����ROM */
			return dicrom[addr | dicrom_addr];
#endif
		}
	}

	/* �g��RAM */
	return extram_b[addr];
}

/*
 *	���{��J�[�h
 *	�P�o�C�g��������
 */
void FASTCALL jcard_writeb(WORD addr, BYTE dat)
{
	/* FM77AV�V���[�Y�̂݃T�|�[�g */
	if (fm7_ver < 2) {
		return;
	}

#if XM7_VER == 2
	/* ���{��J�[�h���g���Ȃ��ꍇ�͋A�� */
	if (!jcard_available || !jcard_enable) {
		return;
	}
#else
	/* ���{��J�[�h�������͋A�� */
	if ((fm7_ver == 2) && !jcard_enable) {
		return;
	}
#endif

	/* $28000-$29FFF : �w�KRAM */
	if ((addr >= 0x8000) && (addr < 0xa000)) {
		if (dicram_en) {
			dicram[addr - 0x8000] = dat;
			return;
		}
	}

	/* �g��RAM (����ROM�̑I����ԂɊւ�炸�������݉\) */
	extram_b[addr] = dat;
}

/*
 *	���{��J�[�h
 *	�Z�[�u
 */
BOOL FASTCALL jcard_save(int fileh)
{
	if (!file_byte_write(fileh, dicrom_bank)) {
		return FALSE;
	}
#if XM7_VER >= 3
	if (!file_bool_write(fileh, extrom_sel)) {
		return FALSE;
	}
#endif
	if (!file_bool_write(fileh, dicrom_en)) {
		return FALSE;
	}
	if (!file_bool_write(fileh, dicram_en)) {
		return FALSE;
	}
	if (!file_write(fileh, extram_b, 0x10000)) {
		return FALSE;
	}
	if (!file_bool_write(fileh, jcard_enable)) {
		return FALSE;
	}

	return TRUE;
}

/*
 *	���{��J�[�h
 *	���[�h
 */
BOOL FASTCALL jcard_load(int fileh, int ver)
{
	/* �o�[�W�����`�F�b�N */
	if (ver < 721) {
		dicrom_bank = 0;
#if XM7_VER >= 3
		extrom_sel = FALSE;
#endif
		dicram_en = FALSE;
		dicrom_en = FALSE;
		return TRUE;
	}

	if (!file_byte_read(fileh, &dicrom_bank)) {
		return FALSE;
	}
#if XM7_VER >= 3
	if (ver >= 800) {
		if (!file_bool_read(fileh, &extrom_sel)) {
			return FALSE;
		}
	}
#endif
	if (!file_bool_read(fileh, &dicrom_en)) {
		return FALSE;
	}
	if (!file_bool_read(fileh, &dicram_en)) {
		return FALSE;
	}
	if (!file_read(fileh, extram_b, 0x10000)) {
		return FALSE;
	}
#if XM7_VER >= 3
	if ((ver >= 921) || ((ver >= 721) && (ver <= 799))) {
#else
	if ((ver >= 721) && (ver <= 799)) {
#endif
		if (!file_bool_read(fileh, &jcard_enable)) {
			return FALSE;
		}
	}

	return TRUE;
}

#endif	/* XM7_VER >= 2 */
