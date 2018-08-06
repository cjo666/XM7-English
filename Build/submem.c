/*
 *	FM-7 EMULATOR "XM7"
 *
 *	Copyright (C) 1999-2017 �o�h�D(Twitter:@xm6_original)
 *	Copyright (C) 2001-2017 Ryu Takegami (Twitter:@RyuTakegami)
 *
 *	[ �T�uCPU������ ]
 *
 *	RHG����
 *	  2001.07.23(��)	V2�ł�VRAM�p�������m�ۗʂ�96KB�ɖ߂���
 *	  2001.11.05		V2��$D800�`$FFFF�Ƀ��C�g�A�N�Z�X����ƃ�������j�󂷂�
 *						�����C��
 *	  2003.11.20		XM7 V1.1�Ή�(���Ⴊ�ݑ�p���c)
 *	  2007.01.13		V1.1��400���C���e�L�X�g�t�H���g��40EX�p�g��ROM�C���[�W
 *						�ő�p�ł���悤�ɕύX(�命���ɂ͊֌W�Ȃ�)
 *	  2010.06.09		���@�ʂ�I/O�̈�̃~���[�̈�𔽉f������悤�ɕύX
 *	  2010.09.29		FM-8��ROM�C���[�W�����Ȃ��ꍇ�Ƀh���C�u�ԍ�/FM�����f�B
 *						�X�v���C�̕����\��������łȂ����ɑΏ�
 *	  2012.05.29		�X�e�[�g���[�h����400���C���J�[�h�̑��݃`�F�b�N��p�~
 *	  2017.05.05		FM-8/7���[�h����VRAM�A�N�Z�X�t���O�𗧂Ă��Ƀu�����L��
 *						�O���ԊO��VRAM�A�N�Z�X�����ꍇ�A�����ƂȂ�悤�ɋ�����
 *						�ύX
 */

#include <stdlib.h>
#include <string.h>
#include "xm7.h"
#include "display.h"
#include "subctrl.h"
#include "device.h"
#include "keyboard.h"
#include "multipag.h"
#include "aluline.h"
#if XM7_VER == 1 && defined(L4CARD)
#include "l4font.h"
#endif

/*
 *	�O���[�o�� ���[�N
 */
BYTE *vram_c;						/* VRAM(�^�C�vC) $C000(���o���N�L��) */
BYTE *subrom_c;						/* ROM (�^�C�vC) $2800 */
BYTE *sub_ram;						/* �R���\�[��RAM $1680 */
BYTE *sub_io;						/* �T�uCPU I/O   $0100 */
#if XM7_VER >= 3
BYTE *subramde;						/* RAM (TypeD/E) $2000 */
BYTE *subramcg;						/* RAM (�t�H���g)$4000 */
BYTE *subramcn;						/* ���R���\�[��  $2000 */
#endif

#if XM7_VER >= 2
BYTE *vram_b;						/* VRAM(�^�C�vB) $C000(���o���N�L��) */
BYTE *subrom_a;						/* ROM (�^�C�vA) $2000 */
BYTE *subrom_b;						/* ROM (�^�C�vB) $2000 */
BYTE *subromcg;						/* ROM (�t�H���g)$2000 */

BYTE subrom_bank;					/* �T�u�V�X�e��ROM�o���N */
BYTE cgrom_bank;					/* CGROM�o���N */
#if XM7_VER >= 3
BYTE cgram_bank;					/* CGRAM�o���N */
BYTE consram_bank;					/* �R���\�[��RAM�o���N */
#endif	/* XM7_VER >= 3 */
#endif	/* XM7_VER >= 2 */

#if XM7_VER == 1
BYTE *subrom_8;						/* ROM (FM-8)    $2800 */
BYTE *subramcg;						/* RAM (PCG)      $800 */
BOOL pcg_flag;						/* PCG�t���O */
#if defined(L4CARD)
BYTE *tvram_c;						/* Text VRAM     $1000 */
BYTE *subrom_l4;					/* ROM(400���C��)$4800 */
BYTE *subcg_l4;						/* CG (400���C��)$1000 */
BOOL enable_400linecard;			/* 400���C���J�[�h�C�l�[�u�� */
BOOL detect_400linecard;			/* 400���C���J�[�h�����t���O */
BOOL detect_400linecard_tmp;		/* 400���C���J�[�h�����t���O */
BOOL ankcg_force_internal = FALSE;	/* �����t�H���g�����g�p�t���O */
#endif
#endif


/*
 *	�T�uCPU������
 *	������
 */
BOOL FASTCALL submem_init(void)
{
	/* ��x�A�S�ăN���A */
	vram_c = NULL;
	subrom_c = NULL;
	sub_ram = NULL;
	sub_io = NULL;
#if XM7_VER == 1
	subrom_8 = NULL;
	subramcg = NULL;
	pcg_flag = FALSE;
#endif

#if XM7_VER >= 2
	vram_b = NULL;
	subrom_a = NULL;
	subrom_b = NULL;
	subromcg = NULL;
#if XM7_VER >= 3
	subramde = NULL;
	subramcg = NULL;
	subramcn = NULL;
#endif
#endif

#if XM7_VER == 1 && defined(L4CARD)
	tvram_c = NULL;
	subrom_l4 = NULL;
	subcg_l4 = NULL;
#endif

	/* �������m��(�^�C�vC) */
#if XM7_VER >= 3
	vram_c = (BYTE *)malloc(0x30000);
#elif XM7_VER >= 2
	vram_c = (BYTE *)malloc(0x18000);
#else
	vram_c = (BYTE *)malloc(0xc000);
#endif
	if (vram_c == NULL) {
		return FALSE;
	}
	subrom_c = (BYTE *)malloc(0x2800);
	if (subrom_c == NULL) {
		return FALSE;
	}
#if XM7_VER == 1
	subrom_8 = (BYTE *)malloc(0x2800);
	if (subrom_8 == NULL) {
		return FALSE;
	}
#endif

	/* �������m��(�^�C�vA,B) */
#if XM7_VER >= 2
	vram_b = vram_c + 0xc000;
	subrom_a = (BYTE *)malloc(0x2000);
	if (subrom_a == NULL) {
		return FALSE;
	}
	subrom_b = (BYTE *)malloc(0x2000);
	if (subrom_b == NULL) {
		return FALSE;
	}
	subromcg = (BYTE *)malloc(0x2000);
	if (subromcg == NULL) {
		return FALSE;
	}

#if XM7_VER >= 3
	/* �������m��(�^�C�vD,E) */
	subramcn = (BYTE *)malloc(0x2000);
	if (subramcn == NULL) {
		return FALSE;
	}
	subramcg = (BYTE *)malloc(0x4000);
	if (subramcg == NULL) {
		return FALSE;
	}
	subramde = (BYTE *)malloc(0x2000);
	if (subramde == NULL) {
		return FALSE;
	}
#endif
#endif

	/* �������m��(����) */
	sub_ram = (BYTE *)malloc(0x1680);
	if (sub_ram == NULL) {
		return FALSE;
	}
	sub_io = (BYTE *)malloc(0x0100);
	if (sub_io == NULL) {
		return FALSE;
	}

#if XM7_VER == 1
	/* �������m��(����) */
	subramcg = (BYTE *)malloc(0x0800);
	if (subramcg == NULL) {
		return FALSE;
	}

#if defined(L4CARD)
	/* �������m��(L4) */
	tvram_c = (BYTE *)malloc(0x1000);
	if (tvram_c == NULL) {
		return FALSE;
	}
	subrom_l4 = (BYTE *)malloc(0x4800);
	if (subrom_l4 == NULL) {
		return FALSE;
	}
	subcg_l4 = (BYTE *)malloc(0x1000);
	if (subcg_l4 == NULL) {
		return FALSE;
	}
#endif
#endif

	/* ROM�t�@�C���ǂݍ��� */
	if (!file_load(SUBSYSC_ROM, subrom_c, 0x2800)) {
#if XM7_VER == 1
		available_fm7roms = FALSE;
#else
		return FALSE;
#endif
	}
#if XM7_VER >= 2
	if (!file_load(SUBSYSA_ROM, subrom_a, 0x2000)) {
		return FALSE;
	}
	if (!file_load(SUBSYSB_ROM, subrom_b, 0x2000)) {
		return FALSE;
	}
	if (!file_load(SUBSYSCG_ROM, subromcg, 0x2000)) {
		return FALSE;
	}
#endif

#if XM7_VER == 1
	if (!file_load(SUBSYS8_ROM, subrom_8, 0x2800)) {
		available_fm8roms = FALSE;
	}

#if defined(L4CARD)
	/* 400���C���Ή����� */
	detect_400linecard_tmp = FALSE;
	if (!file_load(SUBSYSL4_ROM, subrom_l4, 0x4800)) {
		enable_400linecard = FALSE;
		detect_400linecard = FALSE;
	}
	else {
		enable_400linecard = TRUE;
		detect_400linecard = TRUE;

		/* ANKCG16.ROM,EXTSUB.ROM�̂ǂ�����Ȃ��ꍇ�����t�H���g���g�p */
		if (ankcg_force_internal) {
			memcpy(subcg_l4, subcg_internal, 4096);
		}
		else {
			if (!file_load(ANKCG16_ROM, subcg_l4, 0x1000)) {
				if (!file_load(EXTSUB_ROM, subcg_l4, 0x1000)) {
					memcpy(subcg_l4, subcg_internal, 4096);
				}
			}
		}
	}
#endif

	if (!available_fm8roms && !available_fm7roms) {
		return FALSE;
	}

	/* PCG�G���A�ւ�CG���e�]�� */
	if (!available_fm7roms) {
		memcpy(subramcg, subrom_c, 0x0800);
	}
	else {	
		memcpy(subramcg, subrom_8, 0x0800);
	}

	/* FM-8��ROM�C���[�W�����Ȃ��ꍇ�̃h���C�u�ԍ��\�����ւ̑Ώ� */
	/* �{���A�擪��2048�o�C�g�̂݃R�s�[����΂悢���O�̂��ߑS�̂��R�s�[ */
	if (available_fm8roms && !available_fm7roms) {
		memcpy(subrom_c, subrom_8, 0x2800);
	}
#endif

	return TRUE;
}

/*
 *	�T�uCPU������
 *	�N���[���A�b�v
 */
void FASTCALL submem_cleanup(void)
{
	ASSERT(vram_c);
	ASSERT(subrom_c);
	ASSERT(sub_ram);
	ASSERT(sub_io);
#if XM7_VER >= 2
	ASSERT(subrom_a);
	ASSERT(subrom_b);
	ASSERT(subromcg);
#if XM7_VER >= 3
	ASSERT(subramcn);
	ASSERT(subramcg);
	ASSERT(subramde);
#endif
#endif
#if XM7_VER == 1
	ASSERT(subrom_8);
	ASSERT(subramcg);
#if defined(L4CARD)
	ASSERT(tvram_c);
	ASSERT(subrom_l4);
	ASSERT(subcg_l4);
#endif
#endif

	/* �������r���Ŏ��s�����ꍇ���l�� */
	if (vram_c) {
		free(vram_c);
	}
	if (subrom_c) {
		free(subrom_c);
	}

#if XM7_VER >= 2
	if (subrom_a) {
		free(subrom_a);
	}
	if (subrom_b) {
		free(subrom_b);
	}
	if (subromcg) {
		free(subromcg);
	}

#if XM7_VER >= 3
	if (subramcn) {
		free(subramcn);
	}
	if (subramcg) {
		free(subramcg);
	}
	if (subramde) {
		free(subramde);
	}
#endif
#endif

	if (sub_ram) {
		free(sub_ram);
	}
	if (sub_io) {
		free(sub_io);
	}

#if XM7_VER == 1
	if (subrom_8) {
		free(subrom_8);
	}
	if (subramcg) {
		free(subramcg);
	}

#if defined(L4CARD)
	if (tvram_c) {
		free(tvram_c);
	}
	if (subrom_l4) {
		free(subrom_l4);
	}
	if (subcg_l4) {
		free(subcg_l4);
	}
#endif
#endif
}

/*
 *	�T�uCPU������
 *	���Z�b�g
 */
void FASTCALL submem_reset(void)
{
#if XM7_VER >= 2
	/* VRAM�A�N�e�B�u�y�[�W */
	vram_aptr = vram_c;
	vram_active = 0;

	/* �o���N�N���A */
	subrom_bank = 0;
	cgrom_bank = 0;
#if XM7_VER >= 3
	cgram_bank = 0;
	consram_bank = 0;
#endif
#endif

	/* I/O��� �N���A */
	memset(sub_io, 0xff, 0x0100);
}

/*
 *	�T�uCPU������
 *	�P�o�C�g�擾
 */
BYTE FASTCALL submem_readb(WORD addr)
{
	BYTE dat;

#if XM7_VER == 1 && defined(L4CARD)
	/* FM-77L4 400���C�����[�h���̏��� */
	if (enable_400line && enable_400linecard) {
		/* GVRAM/���[�NRAM */
		if (addr <= 0x7fff) {
			/* ���[�NRAM */
			if (workram_select) {
				if (multi_page & 4) {
					return 0xff;
				}
				return vram_c[0x8000 + ((addr + vram_offset[0]) & 0x3fff)];
			}

			/* GVRAM */
			addr += vram_offset[0];
			addr &= 0x7fff;
			if (multi_page & (1 << (addr >> 14))) {
				return 0xff;
			}
			return vram_c[addr];
		}

		/* �e�L�X�gVRAM */
		if ((addr >= 0x8000) && (addr <= 0x97ff)) {
			return tvram_c[addr & 0xfff];
		}

		/* �T�u���j�^ROM(�O��) */
		if ((addr >= 0x9800) && (addr <= 0xbfff)) {
			return subrom_l4[addr - 0x9800];
		}

		/* �T�u���j�^ROM(�㔼) */
		if (addr >= 0xe000) {
			return subrom_l4[addr - 0xb800];
		}
	}
#endif

	/* VRAM */
	if (addr < 0xc000) {
#if XM7_VER >= 3
		if ((fm7_ver >= 2) || (cycle_steal || vrama_flag || blank_flag)) {
			if (mode400l) {
				/* 400���C�� */
				if (addr >= 0x8000) {
					return 0xff;
				}
				aluline_extrb(addr);

				if (multi_page & (1 << subram_vrambank)) {
					return 0xff;
				}
				else {
					return vram_aptr[addr];
				}
			}
			else {
				/* 200���C�� */
				aluline_extrb(addr);
				if (multi_page & (1 << (addr >> 14))) {
					return 0xff;
				}
				else {
					return vram_aptr[((addr & 0xc000) << 1) | (addr & 0x3fff)];
				}
			}
		}
		else {
			return 0xff;
		}
#elif XM7_VER >= 2
		/* 200���C�� */
		if ((fm7_ver >= 2) || (cycle_steal || vrama_flag || blank_flag)) {
			aluline_extrb(addr);
			if (multi_page & (1 << (addr >> 14))) {
				return 0xff;
			}
			else {
				return vram_aptr[addr];
			}
		}
		else {
			return 0xff;
		}
#else
		/* 200���C�� */
		if ((fm_subtype == FMSUB_FM77) ||
			(cycle_steal || vrama_flag || blank_flag)) {
			if (multi_page & (1 << (addr >> 14))) {
				return 0xff;
			}
			else {
				return vram_c[addr];
			}
		}
		else {
			return 0xff;
		}
#endif
	}

#if XM7_VER >= 3
	/* �R���\�[��RAM(RAM���[�h��p) */
	if ((addr < 0xd000) && (subrom_bank == 4) && (consram_bank >= 1)) {
		return subramcn[(consram_bank - 1) * 0x1000 + (addr - 0xc000)];
	}
#endif

	/* ���[�NRAM */
	if (addr < 0xd380) {
		return sub_ram[addr - 0xc000];
	}

	/* ���LRAM */
	if (addr < 0xd400) {
		return shared_ram[(WORD)(addr - 0xd380)];
	}

#if XM7_VER >= 2
	/* �T�uROM */
	if (addr >= 0xe000) {
		switch (subrom_bank) {
			/* �^�C�vC */
			case 0:
				return subrom_c[addr - 0xd800];
			/* �^�C�vA */
			case 1:
				return subrom_a[addr - 0xe000];
			/* �^�C�vB */
			case 2:
				return subrom_b[addr - 0xe000];
			/* CGROM */
			case 3:
				return subromcg[addr - 0xe000];
#if XM7_VER >= 3
			/* �^�C�vD/E(RAM) */
			case 4:
				return subramde[addr - 0xe000];
#endif
			}
	}

	/* CGRAM,CGROM */
	if (addr >= 0xd800) {
#if XM7_VER >= 3
		if (subrom_bank == 4) {
			/* �T�uRAM �o���N */
			return subramcg[cgram_bank * 0x0800 + (addr - 0xd800)];
		}
		else if (fm7_ver >= 2) {
			/* �T�uROM �o���N */
			return subromcg[cgrom_bank * 0x0800 + (addr - 0xd800)];
		}
		else {
			/* �T�uROM Type-C */
			return subrom_c[addr - 0xd800];
		}
#else
		if (fm7_ver >= 2) {
			/* �T�uROM �o���N */
			return subromcg[cgrom_bank * 0x0800 + (addr - 0xd800)];
		}
		else {
			/* �T�uROM Type-C */
			return subrom_c[addr - 0xd800];
		}
#endif
	}

	/* ���[�NRAM */
	if (fm7_ver >= 2) {
		if (addr >= 0xd500){
			return sub_ram[(addr - 0xd500) + 0x1380];
		}
	}
#else
	if (addr >= 0xd800) {
		if ((addr < 0xe000) && pcg_flag) {
			/* PCG */
			return subramcg[addr - 0xd800];
		}
		if (fm_subtype == FMSUB_FM8) {
			/* FM-8�T�u���j�^ */
			return subrom_8[addr - 0xd800];
		}
		else {
			/* �^�C�vC */
			return subrom_c[addr - 0xd800];
		}
	}
#endif

	/*
	 *	�T�uI/O
	 */
#if XM7_VER == 1
	/* $D410�`$D7FF��$D400�`$D40F�̃~���[ */
	addr &= 0xfc0f;
#else
	if (fm7_ver == 1) {
		/* $D410�`$D7FF��$D400�`$D40F�̃~���[ */
		addr &= 0xfc0f;
	}
	else if (fm7_ver == 2) {
		/* $D440�`$D4FF��$D400�`$D43F�̃~���[ */
		addr &= 0xff3f;
	}
#endif

	/* �f�B�X�v���C */
	if (display_readb(addr, &dat)) {
		return dat;
	}
	/* �L�[�{�[�h */
	if (keyboard_readb(addr, &dat)) {
		return dat;
	}
	/* �_�����Z�E������� */
#if XM7_VER >= 2
	if (aluline_readb(addr, &dat)) {
		return dat;
	}
#endif

	return 0xff;
}

/*
 *	�T�uCPU������
 *	�P�o�C�g�擾(I/O�Ȃ�)
 */
BYTE FASTCALL submem_readbnio(WORD addr)
{
#if XM7_VER == 1 && defined(L4CARD)
	/* FM-77L4 400���C�����[�h���̏��� */
	if (enable_400line && enable_400linecard) {
		/* GVRAM/���[�NRAM */
		if (addr <= 0x7fff) {
			/* ���[�NRAM */
			if (workram_select) {
				if (multi_page & 4) {
					return 0xff;
				}
				return vram_c[0x8000 + ((addr + vram_offset[0]) & 0x3fff)];
			}

			/* GVRAM */
			addr += vram_offset[0];
			addr &= 0x7fff;
			if (multi_page & (1 << (addr >> 14))) {
				return 0xff;
			}
			return vram_c[addr];
		}

		/* �e�L�X�gVRAM */
		if ((addr >= 0x8000) && (addr <= 0x97ff)) {
			return tvram_c[addr & 0xfff];
		}

		/* �T�u���j�^ROM(�O��) */
		if ((addr >= 0x9800) && (addr <= 0xbfff)) {
			return subrom_l4[addr - 0x9800];
		}

		/* �T�u���j�^ROM(�㔼) */
		if (addr >= 0xe000) {
			return subrom_l4[addr - 0xb800];
		}
	}
#endif

	/* VRAM */
	if (addr < 0xc000) {
#if XM7_VER >= 3
		if (mode400l) {
			/* 400���C�� */
			if (addr < 0x8000) {
				return vram_aptr[addr];
			}
			else {
				return 0xff;
			}
		}
		else {
			/* 200���C�� */
			return vram_aptr[((addr & 0xc000) << 1) | (addr & 0x3fff)];
		}
#elif XM7_VER >= 2
		/* 200���C�� */
		return vram_aptr[addr];
#else
		/* 200���C�� */
		return vram_c[addr];
#endif
	}

#if XM7_VER >= 3
	/* �R���\�[��RAM(RAM���[�h��p) */
	if ((addr < 0xd000) && (subrom_bank == 4) && (consram_bank >= 1)) {
		return subramcn[(consram_bank - 1) * 0x1000 + (addr - 0xc000)];
	}
#endif

	/* ���[�NRAM */
	if (addr < 0xd380) {
		return sub_ram[addr - 0xc000];
	}

	/* ���LRAM */
	if (addr < 0xd400) {
		return shared_ram[(WORD)(addr - 0xd380)];
	}

	/* ���[�NRAM */
#if XM7_VER >= 2
	if (fm7_ver >= 2) {
		if ((addr >= 0xd500) && (addr < 0xd800)) {
			return sub_ram[(addr - 0xd500) + 0x1380];
		}
	}
#endif

	/* �T�uI/O */
	if (addr < 0xd800) {
#if XM7_VER == 1
		/* $D410�`$D7FF��$D400�`$D40F�̃~���[ */
		addr &= 0xfc0f;
#else
		if (fm7_ver == 1) {
			/* $D410�`$D7FF��$D400�`$D40F�̃~���[ */
			addr &= 0xfc0f;
		}
		else if (fm7_ver == 2) {
			/* $D440�`$D4FF��$D400�`$D43F�̃~���[ */
			addr &= 0xff3f;
		}
#endif
		return sub_io[addr - 0xd400];
	}

#if XM7_VER >= 2
	/* �T�uROM */
	if (addr >= 0xe000) {
		ASSERT(subrom_bank <= 4);
		switch (subrom_bank) {
			/* �^�C�vC */
			case 0:
				return subrom_c[addr - 0xd800];
			/* �^�C�vA */
			case 1:
				return subrom_a[addr - 0xe000];
			/* �^�C�vB */
			case 2:
				return subrom_b[addr - 0xe000];
			/* CGROM */
			case 3:
				return subromcg[addr - 0xe000];
#if XM7_VER >= 3
			/* �^�C�vD/E(RAM) */
			case 4:
				return subramde[addr - 0xe000];
#endif
			}
	}

	/* CGRAM,CGROM */
	if (addr >= 0xd800) {
#if XM7_VER >= 3
		if (subrom_bank == 4) {
			/* �T�uRAM �o���N */
			return subramcg[cgram_bank * 0x0800 + (addr - 0xd800)];
		}
		else if (fm7_ver >= 2) {
			/* �T�uROM �o���N */
			return subromcg[cgrom_bank * 0x0800 + (addr - 0xd800)];
		}
		else {
			/* �T�uROM Type-C */
			return subrom_c[addr - 0xd800];
		}
#else
		if (fm7_ver >= 2) {
			/* �T�uROM �o���N */
			return subromcg[cgrom_bank * 0x0800 + (addr - 0xd800)];
		}
		else {
			/* �T�uROM Type-C */
			return subrom_c[addr - 0xd800];
		}
#endif
	}
#else
	if (addr >= 0xd800) {
		if ((addr < 0xe000) && pcg_flag) {
			/* PCG */
			return subramcg[addr - 0xd800];
		}
		if (fm_subtype == FMSUB_FM8) {
			/* FM-8�T�u���j�^ */
			return subrom_8[addr - 0xd800];
		}
		else {
			/* �^�C�vC */
			return subrom_c[addr - 0xd800];
		}
	}
#endif

	/* �����ɂ͗��Ȃ� */
	ASSERT(FALSE);
	return 0;
}

/*
 *	�T�uCPU������
 *	�P�o�C�g��������
 */
void FASTCALL submem_writeb(WORD addr, BYTE dat)
{
#if XM7_VER == 1 && defined(L4CARD)
	/* FM-77L4 400���C�����[�h���̏��� */
	if (enable_400line && enable_400linecard) {
		/* GVRAM/���[�NRAM */
		if (addr <= 0x7fff) {
			/* ���[�NRAM */
			if (workram_select) {
				if (!(multi_page & 4)) {
					vram_c[0x8000 + ((addr + vram_offset[0]) & 0x3fff)] = dat;
				}
				return;
			}

			/* GVRAM */
			addr += vram_offset[0];
			addr &= 0x7fff;
			if (!(multi_page & (1 << (addr >> 14)))) {
				/* �����f�[�^���������ނȂ�ANotify���Ȃ� */
				if (vram_c[addr] != dat) {
					vram_c[addr] = dat;
					vram_notify(addr, dat);
				}
			}
			return;
		}

		/* �e�L�X�gVRAM */
		if ((addr >= 0x8000) && (addr <= 0x97ff)) {
			tvram_c[addr & 0xfff] = dat;
			tvram_notify((WORD)(addr & 0xfff), dat);
			return;
		}

		/* �T�u���j�^ROM */
		if (((addr >= 0x9800) && (addr <= 0xbfff)) || (addr >= 0xe000)) {
			return;
		}
	}
#endif

	/* VRAM(�^�C�vC) */
	if (addr < 0xc000) {
#if XM7_VER >= 3
		if ((fm7_ver >= 2) || (cycle_steal || vrama_flag || blank_flag)) {
			if (mode400l) {
				/* 400���C�� */
				if (addr >= 0x8000) {
					return;
				}
				if (alu_command & 0x80) {
					aluline_extrb(addr);
					return;
				}
				if (!(multi_page & (1 << subram_vrambank))) {
					if (vram_aptr[addr] != dat) {
						/* �����f�[�^���������ނȂ�ANotify���Ȃ� */
						vram_aptr[addr] = dat;
						vram_notify(addr, dat);
					}
				}
			}
			else {
				if (alu_command & 0x80) {
					aluline_extrb(addr);
					return;
				}
				if (!(multi_page & (1 << (addr >> 14)))) {
					/* �����f�[�^���������ނȂ�ANotify���Ȃ� */
					if (vram_aptr[((addr & 0xc000) << 1)|(addr & 0x3fff)] != dat) {
						vram_aptr[((addr & 0xc000) << 1)|(addr & 0x3fff)] = dat;
						vram_notify(addr, dat);
					}
				}
			}
		}
#elif XM7_VER >= 2
		if ((fm7_ver >= 2) || (cycle_steal || vrama_flag || blank_flag)) {
			if (alu_command & 0x80) {
				aluline_extrb(addr);
				return;
			}
			if (!(multi_page & (1 << (addr >> 14)))) {
				/* �����f�[�^���������ނȂ�ANotify���Ȃ� */
				if (vram_aptr[addr] != dat) {
					vram_aptr[addr] = dat;
					vram_notify(addr, dat);
				}
			}
		}
#else
		if ((fm_subtype == FMSUB_FM77) ||
			(cycle_steal || vrama_flag || blank_flag)) {
			if (!(multi_page & (1 << (addr >> 14)))) {
				/* �����f�[�^���������ނȂ�ANotify���Ȃ� */
				if (vram_c[addr] != dat) {
					vram_c[addr] = dat;
					vram_notify(addr, dat);
				}
			}
		}
#endif
		return;
	}

#if XM7_VER >= 3
	/* �R���\�[��RAM(RAM���[�h��p) */
	if ((addr < 0xd000) && (subrom_bank == 4) && (consram_bank >= 1)) {
		subramcn[(consram_bank - 1) * 0x1000 + (addr - 0xc000)] = dat;
		return;
	}
#endif

	/* ���[�NRAM */
	if (addr < 0xd380) {
		sub_ram[addr - 0xc000] = dat;
		return;
	}

	/* ���LRAM */
	if (addr < 0xd400) {
		shared_ram[(WORD)(addr - 0xd380)] = dat;
		return;
	}

	/* ���[�NRAM */
#if XM7_VER >= 2
	if (fm7_ver >= 2) {
		if ((addr >= 0xd500) && (addr < 0xd800)) {
			sub_ram[(addr - 0xd500) + 0x1380] = dat;
			return;
		}
	}
#endif

	/* CGROM,�T�uRAM ��ROM���[�h�ł͏������݂ł��Ȃ� */
	if (addr >= 0xd800) {
#if XM7_VER >= 3
		/* �T�uRAM�łȂ����A�v���e�N�g����Ă��� */
		if ((subrom_bank != 4) || subram_protect) {
			return;
		}

		/* CGRAM */
		if (addr < 0xe000) {
			subramcg[cgram_bank * 0x0800 + (addr - 0xd800)] = dat;
			return;
		}

		/* �T�uRAM */
		subramde[addr - 0xe000] = dat;
		return;
#else
#if XM7_VER == 1
		/* PCG */
		if ((addr < 0xe000) && pcg_flag) {
			subramcg[addr - 0xd800] = dat;
			return;
		}
#endif

		/* V1/V2�ł͖������ŏ������ݕs�� */
		return;
#endif
	}

	/*
	 *	�T�uI/O
	 */
#if XM7_VER == 1
	/* $D410�`$D7FF��$D400�`$D40F�̃~���[ */
	addr &= 0xfc0f;
#else
	if (fm7_ver == 1) {
		/* $D410�`$D7FF��$D400�`$D40F�̃~���[ */
		addr &= 0xfc0f;
	}
	else if (fm7_ver == 2) {
		/* $D440�`$D4FF��$D400�`$D43F�̃~���[ */
		addr &= 0xff3f;
	}
#endif
	sub_io[addr - 0xd400] = dat;

	/* �f�B�X�v���C */
	if (display_writeb(addr, dat)) {
		return;
	}
	/* �L�[�{�[�h */
	if (keyboard_writeb(addr, dat)) {
		return;
	}
	/* �_�����Z�E������� */
#if XM7_VER >= 2
	if (aluline_writeb(addr, dat)) {
		return;
	}
#endif
}

/*
 *	�T�uCPU������
 *	�Z�[�u
 */
BOOL FASTCALL submem_save(int fileh)
{
#if XM7_VER >= 3
	/* ���܂łƓ��������ŏ������� */
	if (mode400l) {
		if (!file_write(fileh, vram_c, 0x18000)) {
			return FALSE;
		}
	}
	else {
		if (!file_write(fileh, vram_c, 0x4000)) {
			return FALSE;
		}
		if (!file_write(fileh, &vram_c[0x8000], 0x4000)) {
			return FALSE;
		}
		if (!file_write(fileh, &vram_c[0x10000], 0x4000)) {
			return FALSE;
		}
		if (!file_write(fileh, &vram_c[0x4000], 0x4000)) {
			return FALSE;
		}
		if (!file_write(fileh, &vram_c[0xc000], 0x4000)) {
			return FALSE;
		}
		if (!file_write(fileh, &vram_c[0x14000], 0x4000)) {
			return FALSE;
		}
	}
#else
	if (!file_write(fileh, vram_c, 0x6000)) {
		return FALSE;
	}
	if (!file_write(fileh, &vram_c[0x6000], 0x6000)) {
		return FALSE;
	}
#if XM7_VER >= 2
	if (!file_write(fileh, &vram_c[0xc000], 0x6000)) {
		return FALSE;
	}
	if (!file_write(fileh, &vram_c[0x12000], 0x6000)) {
		return FALSE;
	}
#endif
#endif

	if (!file_write(fileh, sub_ram, 0x1680)) {
		return FALSE;
	}

	if (!file_write(fileh, sub_io, 0x100)) {
		return FALSE;
	}

#if XM7_VER >= 2
	if (!file_byte_write(fileh, subrom_bank)) {
		return FALSE;
	}
	if (!file_byte_write(fileh, cgrom_bank)) {
		return FALSE;
	}

#if XM7_VER >= 3
	/* Ver8�g�� */
	if (mode400l) {
		if (!file_write(fileh, &vram_c[0x18000], 0x18000)) {
			return FALSE;
		}
	}
	else {
		if (!file_write(fileh, &vram_c[0x18000], 0x4000)) {
			return FALSE;
		}
		if (!file_write(fileh, &vram_c[0x20000], 0x4000)) {
			return FALSE;
		}
		if (!file_write(fileh, &vram_c[0x28000], 0x4000)) {
			return FALSE;
		}
		if (!file_write(fileh, &vram_c[0x1c000], 0x4000)) {
			return FALSE;
		}
		if (!file_write(fileh, &vram_c[0x24000], 0x4000)) {
			return FALSE;
		}
		if (!file_write(fileh, &vram_c[0x2c000], 0x4000)) {
			return FALSE;
		}
	}

	if (!file_write(fileh, subramde, 0x2000)) {
		return FALSE;
	}
	if (!file_write(fileh, subramcg, 0x4000)) {
		return FALSE;
	}
	if (!file_write(fileh, subramcn, 0x2000)) {
		return FALSE;
	}
	if (!file_byte_write(fileh, cgram_bank)) {
		return FALSE;
	}
	if (!file_byte_write(fileh, consram_bank)) {
		return FALSE;
	}
#endif
#endif

#if XM7_VER == 1 && defined(L4CARD)
	if (!file_write(fileh, tvram_c, 0x1000)) {
		return FALSE;
	}
	if (!file_bool_write(fileh, detect_400linecard)) {
		return FALSE;
	}
	if (!file_bool_write(fileh, enable_400linecard)) {
		return FALSE;
	}
	if (!file_write(fileh, subramcg, 0x0800)) {
		return FALSE;
	}
#endif

	return TRUE;
}

/*
 *	�T�uCPU������
 *	���[�h
 */
BOOL FASTCALL submem_load(int fileh, int ver)
{
	/* �o�[�W�����`�F�b�N */
	if (ver < 200) {
		return FALSE;
	}

#if XM7_VER >= 3
	/* ���܂łƓ��������œǂݏo�� */
	if (!file_read(fileh, vram_c, 0x4000)) {
		return FALSE;
	}
	if (!file_read(fileh, &vram_c[0x8000], 0x4000)) {
		return FALSE;
	}
	if (!file_read(fileh, &vram_c[0x10000], 0x4000)) {
		return FALSE;
	}
	if (!file_read(fileh, &vram_c[0x4000], 0x4000)) {
		return FALSE;
	}
	if (!file_read(fileh, &vram_c[0xc000], 0x4000)) {
		return FALSE;
	}
	if (!file_read(fileh, &vram_c[0x14000], 0x4000)) {
		return FALSE;
	}
#else
	if (!file_read(fileh, vram_c, 0x6000)) {
		return FALSE;
	}
	if (!file_read(fileh, &vram_c[0x6000], 0x6000)) {
		return FALSE;
	}
#if XM7_VER >= 2
	if (!file_read(fileh, &vram_c[0xc000], 0x6000)) {
		return FALSE;
	}
	if (!file_read(fileh, &vram_c[0x12000], 0x6000)) {
		return FALSE;
	}
#endif
#endif

	if (!file_read(fileh, sub_ram, 0x1680)) {
		return FALSE;
	}

	if (!file_read(fileh, sub_io, 0x100)) {
		return FALSE;
	}

#if XM7_VER >= 2
	if (!file_byte_read(fileh, &subrom_bank)) {
		return FALSE;
	}
	if (!file_byte_read(fileh, &cgrom_bank)) {
		return FALSE;
	}

#if XM7_VER >= 3
	/* Ver8�g�� */
	if (ver < 800) {
		consram_bank = 0;
		cgram_bank = 0;
		return TRUE;
	}

	/* ���܂łƓ��������œǂݏo�� */
	if (!file_read(fileh, &vram_c[0x18000], 0x4000)) {
		return FALSE;
	}
	if (!file_read(fileh, &vram_c[0x20000], 0x4000)) {
		return FALSE;
	}
	if (!file_read(fileh, &vram_c[0x28000], 0x4000)) {
		return FALSE;
	}
	if (!file_read(fileh, &vram_c[0x1c000], 0x4000)) {
		return FALSE;
	}
	if (!file_read(fileh, &vram_c[0x24000], 0x4000)) {
		return FALSE;
	}
	if (!file_read(fileh, &vram_c[0x2c000], 0x4000)) {
		return FALSE;
	}

	if (!file_read(fileh, subramde, 0x2000)) {
		return FALSE;
	}
	if (!file_read(fileh, subramcg, 0x4000)) {
		return FALSE;
	}
	if (!file_read(fileh, subramcn, 0x2000)) {
		return FALSE;
	}
	if (!file_byte_read(fileh, &cgram_bank)) {
		return FALSE;
	}
	if (!file_byte_read(fileh, &consram_bank)) {
		return FALSE;
	}
#endif
#endif

#if XM7_VER == 1 && defined(L4CARD)
	if (!file_read(fileh, tvram_c, 0x1000)) {
		return FALSE;
	}
	if (!file_bool_read(fileh, &detect_400linecard_tmp)) {
		return FALSE;
	}

	/* Ver3.06�g�� */
	if (ver >= 306) {
		if (!file_bool_read(fileh, &enable_400linecard)) {
			return FALSE;
		}
	}

	/* Ver3.10�g�� */
	if (ver >= 310) {
		if (!file_read(fileh, subramcg, 0x0800)) {
			return FALSE;
		}
	}
#endif

	return TRUE;
}
