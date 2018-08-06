/*
 *	FM-7 EMULATOR "XM7"
 *
 *	Copyright (C) 1999-2017 �o�h�D(Twitter:@xm6_original)
 *	Copyright (C) 2001-2017 Ryu Takegami (Twitter:@RyuTakegami)
 *
 *	[ ����ROM (�񊿎��E��ꐅ���E��񐅏�) ]
 *
 *	RHG����
 *	  2001.07.23(��)	V2�ł̊���ROM�p�������m�ۗʂ�128KB�ɖ߂���
 *	  2002.05.06		�A�h���X���W�X�^($FD20/$FD21/$FD2C/$FD2D)��Write Only
 *						�ɕύX
 *	  2002.11.01		�f�[�^�ǂݏo���������Ȃ�ƂȂ��ύX
 *	  2003.11.21		$FD04 bit5(����ROM�ڑ��̐؂芷���@�\)�ɑΉ�
 *	  2006.04.27		���{��ʐM�J�[�h�p������ǉ�
 *	  2006.11.08		FM-8���[�h�œ��{��ʐM�J�[�h�����������ɂȂ�悤�ɕύX
 *	  2008.01.24		���{��T�u�V�X�e��I/F������jsubsys.c�ɐ؂藣��
 *	  2010.07.28		V1�ɂ�����KANJI.ROM���Ȃ��ꍇ�̏�����ύX
 *	  2010.12.09		FM77AV40/20�n���璊�o����JIS83��������ROM�f�[�^�ɂ�����
 *						FM-8/7/77/FM77AV���[�h��JIS78�����G�~�����[�V������ǉ�
 *	  2011.05.30		���샂�[�h�ʂ�JIS78�����t�H���g��JIS83�����t�H���g�̓�
 *						�����ł���悤�ύX
 *	  2012.05.28		V1/V2�ɂ�����KANJI.ROM��KANJI1.ROM�̓ǂݍ��ݗD�揇�ʂ�
 *						�C��
 *						V1�ɂ�����KANJI.ROM���ǂݍ��߂Ȃ������C��
 *	  2012.05.29		����ȏ゠�ꂱ�ꌾ���Ȃ��悤��kanji_rom_mode��static
 *						�錾�ɕύX(�ق��̃\�[�X����Q�Ƃ��Ă��Ȃ��̂�)
 *	  2012.07.05		V3�ɂ�����KANJI.ROM���Ȃ��ꍇ��FM-7/FM77AV���[�h���Ɋ�
 *						�����\���ł��Ȃ������C��
 *						V3�ɂ�����JIS78�G�~�����[�V�����̎��̕ύX�ɂ��Ă��Ή�
 *	  2012.07.06		V2�ɂ����Ă�JIS78�G�~�����[�V�����̎��̕ύX�ɂ��đΉ�
 */

#include <stdlib.h>
#include <string.h>
#include "xm7.h"
#include "kanji.h"
#include "subctrl.h"
#include "device.h"
#if XM7_VER >= 2
#include "jcard.h"
#endif

/*
 *	�O���[�o�� ���[�N
 */
WORD kanji_addr;						/* �A�h���X���W�X�^(����) */
BYTE *kanji_rom;						/* ��P����ROM */
BYTE *kanji_rom2;						/* ��Q����ROM */
#if XM7_VER >= 3
BYTE *kanji_rom_jis78;					/* ��P����ROM (JIS78����) */
#endif
BOOL kanji_asis_flag;					/* JIS78�G�~�����[�V�����������t���O */

/*
 *	�X�^�e�B�b�N ���[�N
 */
static BOOL kanji_rom_available;		/* ����ROM�g�p�\�t���O */

/*
 *	�v���g�^�C�v�錾
 */
static void FASTCALL kanji_make_jis78(BYTE *rom);


/*
 *	����ROM
 *	������
 */
BOOL FASTCALL kanji_init(void)
{
	/* ������ */
	kanji_rom_available = FALSE;

	/* �������m�� */
#if XM7_VER >= 3
	kanji_rom = (BYTE *)malloc(0x60000);
#else
	kanji_rom = (BYTE *)malloc(0x40000);
#endif
	if (!kanji_rom) {
		return FALSE;
	}

	/* ��1����(JIS78) �t�@�C���ǂݍ��� */
#if XM7_VER >= 3
	kanji_rom_jis78 = (BYTE *)(kanji_rom + 0x40000);
	if (file_load(KANJI_ROM_J78, kanji_rom_jis78, 0x20000)) {
#else
	if (file_load(KANJI_ROM_J78, kanji_rom, 0x20000)) {
#endif
		kanji_rom_available = TRUE;
	}

	/* ��1����(JIS83) �t�@�C���ǂݍ��� */
#if XM7_VER >= 3
	if (!file_load(KANJI_ROM, kanji_rom, 0x20000)) {
		return FALSE;
	}
#else
	if (!kanji_rom_available) {
		if (file_load(KANJI_ROM, kanji_rom, 0x20000)) {
			kanji_rom_available = TRUE;
		}
#if XM7_VER >= 2
		else {
			return FALSE;
		}
#endif
	}
#endif

	/* ��2���� �t�@�C���ǂݍ��� */
	kanji_rom2 = (BYTE *)(kanji_rom + 0x20000);
	if (!file_load(KANJI_ROM2, kanji_rom2, 0x20000)) {
#if XM7_VER >= 3
		return FALSE;
#else
#if XM7_VER == 2
		jcard_available = FALSE;
#endif
		kanji_rom2 = NULL;
#endif
	}

#if XM7_VER >= 3
	/* ����ROM�G�~�����[�V������JIS78�t�H���g�̗L���Ŕ��f���� */
	if (!kanji_rom_available) {
		memcpy(kanji_rom_jis78, kanji_rom, 0x20000);

		/* JIS78��������ROM�f�[�^���� */
		kanji_make_jis78(kanji_rom_jis78);
	}
#else
	if (kanji_rom_available) {
		/* �����I��JIS78����ROM�G�~�����[�V�������s�� */
		kanji_make_jis78(kanji_rom);
	}
#endif

	return TRUE;
}

/*
 *	����ROM
 *	�N���[���A�b�v
 */
void FASTCALL kanji_cleanup(void)
{
	ASSERT(kanji_rom);
	if (kanji_rom) {
		free(kanji_rom);
		kanji_rom = NULL;
		kanji_rom2 = NULL;
#if XM7_VER >= 3
		kanji_rom_jis78 = NULL;
#endif
	}
}

/*
 *	����ROM
 *	���Z�b�g
 */
void FASTCALL kanji_reset(void)
{
	kanji_addr = 0;
}

/*
 *	����ROM
 *	�P�o�C�g�ǂݏo��
 */
BOOL FASTCALL kanji_readb(WORD addr, BYTE *dat)
{
	int offset;

	switch (addr) {
		/* �A�h���X(��1�E��2����)��ʁE���� */
		case 0xfd20:
		case 0xfd21:
#if XM7_VER >= 3
		case 0xfd2c:
		case 0xfd2d:
#endif
			*dat = 0xff;
			return TRUE;

		/* ��1�����f�[�^ */
		case 0xfd22:		/* ��1�f�[�^LEFT */
		case 0xfd23:		/* ��1�f�[�^RIGHT */
#if XM7_VER == 1
			if (!kanji_rom_available) {
				*dat = 0xff;
				return TRUE;
			}
#endif

#if XM7_VER >= 3 || (XM7_VER == 1 && defined(L4CARD))
			if (subkanji_flag) {
				*dat = 0xff;
				return TRUE;
			}
#endif

			offset = kanji_addr << 1;
			if ((fm7_ver <= 1) && (offset >= 0x6000) && (offset < 0x8000) &&
				!kanji_asis_flag) {
				/* FM-7���[�h����$6000�`$7FFF�͖���`�̈� */
				*dat = (BYTE)(addr & 1);
			}
			else {
				/* �ʏ�̈� */
#if XM7_VER >= 3
				if (fm7_ver >= 3) {
					*dat = kanji_rom[offset + (addr & 1)];
				}
				else {
					*dat = kanji_rom_jis78[offset + (addr & 1)];
				}
#else
				*dat = kanji_rom[offset + (addr & 1)];
#endif
			}
			return TRUE;

#if XM7_VER >= 2
		/* ��2�����f�[�^ */
		case 0xfd2e:		/* ��2�f�[�^LEFT */
		case 0xfd2f:		/* ��2�f�[�^RIGHT */
#if XM7_VER >= 3
			if ((fm7_ver < 2) || subkanji_flag) {
				*dat = 0xff;
				return TRUE;
			}
			if ((fm7_ver == 2) && !jcard_enable) {
				*dat = 0xff;
				return TRUE;
			}
#elif XM7_VER == 2
			if (fm7_ver < 2) {
				*dat = 0xff;
				return TRUE;
			}
			if (!jcard_available || !jcard_enable) {
				*dat = 0xff;
				return TRUE;
			}
#endif
			offset = kanji_addr << 1;
			*dat = kanji_rom2[offset + (addr & 1)];
			return TRUE;
#endif
	}

	return FALSE;
}

/*
 *	����ROM
 *	�P�o�C�g��������
 */
BOOL FASTCALL kanji_writeb(WORD addr, BYTE dat)
{
	switch (addr) {
		/* �A�h���X(��1�E��2����)��� */
#if XM7_VER >= 2
		case 0xfd2c:
			if (fm7_ver < 2) {
				return TRUE;
			}
#if XM7_VER == 2
			if (!jcard_available || !jcard_enable) {
				return TRUE;
			}
			if ((fm7_ver == 2) && !jcard_enable) {
				return TRUE;
			}
#endif
#endif
		case 0xfd20:
			kanji_addr &= 0x00ff;
			kanji_addr |= (WORD)(dat << 8);
			return TRUE;

		/* �A�h���X(��1�E��2����)���� */
#if XM7_VER >= 2
		case 0xfd2d:
			if (fm7_ver < 2) {
				return TRUE;
			}
#if XM7_VER == 2
			if (!jcard_available || !jcard_enable) {
				return TRUE;
			}
			if ((fm7_ver == 2) && !jcard_enable) {
				return TRUE;
			}
#endif
#endif
		case 0xfd21:
			kanji_addr &= 0xff00;
			kanji_addr |= dat;
			return TRUE;

		/* �f�[�^ */
		case 0xfd22:
		case 0xfd23:
#if XM7_VER >= 2
		case 0xfd2f:
#endif
			return TRUE;

#if XM7_VER >= 2
		/* ���{��J�[�h �o���N�Z���N�g */
		case 0xfd2e:
			if (fm7_ver < 2) {
				return TRUE;
			}
#if XM7_VER == 2
			if (!jcard_available || !jcard_enable) {
				return TRUE;
			}
#elif XM7_VER >= 3
			if ((fm7_ver == 2) && !jcard_enable) {
				return TRUE;
			}
#endif

			/* bit7:�w�KRAM */
			if (dat & 0x80) {
				dicram_en = TRUE;
			}
			else {
				dicram_en = FALSE;
			}

			/* bit6:����ROM�C�l�[�u�� */
			if (dat & 0x40) {
				dicrom_en = TRUE;
			}
			else {
				dicrom_en = FALSE;
			}

			/* bit0-5:����ROM�o���N */
			dicrom_bank = (BYTE)(dat & 0x3f);
			return TRUE;
#endif
	}

	return FALSE;
}

/*
 *	����ROM
 *	JIS78�����̈�쐬
 */
static void FASTCALL kanji_make_jis78(BYTE *rom)
{
	/* JIS78��������`�����e�[�u��(����FM77AV����) */
	static const DWORD jis78_table[] = {
		0xffffffff, 0x00000001, 0xffff8001, 0xfc00ffff,
		0x00000001, 0x00000001, 0xfe000001, 0x00000001,
		0xffffffff, 0x80000000, 0xffffffff, 0xf8000001,
		0xfff00000, 0xff800000, 0xffffffff, 0xfffc0000,
		0xffffffff, 0x00000000, 0xffffffff, 0xf8000001,
		0x00000000, 0x00000000, 0xfe000001, 0x0001fffc,
	};

	/* JIS78��1������JIS83��2�������֊����e�[�u�� */
	static const DWORD convert_table[][2] = {
		/* ��1����code addr		  ��2����code addr */
		{/*�� 0x3033*/0x4130,	/*�� 0x724D*/0xE4D0},
		{/*�� 0x3229*/0x4490,	/*�� 0x7274*/0xD540},
		{/*�a 0x3342*/0x6620,	/*�y 0x695A*/0x93A0},
		{/*�h 0x3349*/0x6690,	/*�� 0x5978*/0x5380},
		{/*�� 0x3376*/0x8760,	/*�} 0x635E*/0x87E0},
		{/*�� 0x3443*/0x6830,	/*�� 0x5E75*/0x5D50},
		{/*�� 0x3452*/0x6920,	/*�| 0x6B5D*/0x97D0},
		{/*�z 0x375B*/0x6FB0,	/*�� 0x7074*/0xD140},
		{/*�{ 0x395C*/0x73C0,	/*�� 0x6268*/0xA480},
		{/*�� 0x3C49*/0x7890,	/*�A 0x6922*/0x7220},
		{/*�x 0x3F59*/0x7F90,	/*�� 0x7057*/0xE170},
		{/*�G 0x4128*/0xA280,	/*�� 0x6C4D*/0x98D0},
		{/*�� 0x445B*/0xC9B0,	/*�� 0x5464*/0x4840},
		{/*�v 0x4557*/0xCB70,	/*�� 0x626A*/0xA4A0},
		{/*�� 0x456E*/0xEAE0,	/*�� 0x5B6D*/0x56D0},
		{/*�� 0x4573*/0xEB30,	/*�� 0x5E39*/0x1D90},
		{/*�� 0x4676*/0xED60,	/*� 0x6D6E*/0xBAE0},
		{/*�� 0x4768*/0xEE80,	/*� 0x6A24*/0x7440},
		{/*�O 0x4930*/0xB300,	/*�w 0x5B58*/0x3780},
		{/*�� 0x4B79*/0xF790,	/*�� 0x5056*/0x2160},
		{/*�� 0x4C79*/0xF990,	/*�M 0x692E*/0x72E0},
		{/*�U 0x4F36*/0xBF60,	/*�� 0x6446*/0x8860},
		{/*�� 0x3646*/0x6C60,	/*� 0x7421*/0xC810},
		{/*�� 0x4B6A*/0xF6A0,	/*� 0x7422*/0xC820},
		{/*�y 0x4D5A*/0xDBA0,	/*� 0x7423*/0xC830},
		{0, 0},
	};

	DWORD addr;
	DWORD i;

	/* �p�b�`���ĕs�v�t���O�������Ă���ꍇ�͏������X�L�b�v */
	if (kanji_asis_flag) {
		return;
	}

	/* JIS78��������ROM�ł͖���`�ɂȂ��Ă��镔���𖢒�`�Ƃ��� */
	for (addr = 0; addr < 0x6000; addr += 32) {
		if (jis78_table[addr >> 10] & (1 << (addr >> 5))) {
			/* ����`�t�H���g�쐬 */
			for (i = 0; i < 32; i += 2) {
				rom[addr + i + 0] = 0x00;
				rom[addr + i + 1] = 0x01;
			}
		}
	}

#if XM7_VER <= 2
	/* KANJI2.ROM���Ȃ��ꍇ�͂��̂܂܋A�� */
	if (!kanji_rom2) {
		return;
	}
#endif

	/* JIS83�œ���ւ���ꂽ�������2��������ROM����㏑������ */
	for (addr = 0; convert_table[addr][0]; addr ++) {
		/* �t�H���g�R�s�[ */
		for (i = 0; i < 32; i ++) {
			rom[convert_table[addr][0] * 2 + i] =
				kanji_rom2[convert_table[addr][1] * 2 + i];
		}
	}
}

/*
 *	����ROM
 *	�Z�[�u
 */
BOOL FASTCALL kanji_save(int fileh)
{
	if (!file_word_write(fileh, kanji_addr)) {
		return FALSE;
	}

	return TRUE;
}

/*
 *	����ROM
 *	���[�h
 */
BOOL FASTCALL kanji_load(int fileh, int ver)
{
	/* �o�[�W�����`�F�b�N */
	if (ver < 200) {
		return FALSE;
	}

	if (!file_word_read(fileh, &kanji_addr)) {
		return FALSE;
	}

	return TRUE;
}
