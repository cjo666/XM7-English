/*
 *	FM-7 EMULATOR "XM7"
 *
 *	Copyright (C) 1999-2017 �o�h�D(Twitter:@xm6_original)
 *	Copyright (C) 2001-2017 Ryu Takegami (Twitter:@RyuTakegami)
 *
 *	[ MMR,TWR / I/O�^RAM�f�B�X�N�J�[�h ]
 *
 *	RHG����
 *	  2002.06.04		MR2��I/O�^RAM�f�B�X�N�@�\�ɉ��Ή�
 *	  2002.07.29		MR2�֘A���������蒼��
 *						MMR�ł̍Ĕz�u�֎~�̈悩��CRTC I/O(AV40)�����O����
 *	  2002.09.25		R2D2/RD512���Ή�(�������)
 *	  2003.06.19		MMR�֘A���W�X�^�ύX���̑��x�␳�𓱓�
 *	  2003.09.30		$FD0B�̃L�����A�f�B�e�N�g�r�b�g��V��
 *	  2004.12.01		V1.1��MMR I/O�̃��[�h�A�N�Z�X���ł��Ȃ������C��
 *						(�ȁA�Ȃ񂾂��ā[)
 *	  2006.08.15		FM-77/FM77AV���[�h����MMR���W�X�^�̓ǂݎ�茋�ʂ�
 *						����6�r�b�g�����L���ɂȂ�Ȃ��d�l��ύX
 *						(���{��ʐM�J�[�h JTESTEB)
 *	  2010.06.09		����N�����̂�MMR���N���A����悤�Ɏ��@�ɍ��킹��
 *						�d�l��ύX
 *	  2010.06.18		FM-77���[�h����$3F�o���N�̏��������@�ɍ��킹�ĕύX
 *	  2012.04.22		�e�L�X�g�E�B���h�E�������[�h�t���O�������B�������A
 *						�����_�ł͎��ۂ̋����ɂ͉��̉e�����y�ڂ��܂���B
 *	  2015.02.06		V1.1�ɂ�����64KB�g��RAM�J�[�h�̓�����T�|�[�g
 *						(GUI�����Ȃǂ͂��Ă��܂���)
 *+	  2016.06.26		400���C���^�C�~���O�o�̓t���O������
 *						(���ۂ̕\���Ƃ͊֌W�Ȃ����삵�܂��B�N���߂�)
 */

#include <stdlib.h>
#include <string.h>
#include "xm7.h"
#include "device.h"
#include "mmr.h"
#include "subctrl.h"
#include "jcard.h"
#include "rs232c.h"

/*
 *	��
 */
#if defined(RAMDISK)
#define	MR2
#define	RD512
#define	R2D2
#endif

/*
 *	�O���[�o�� ���[�N
 */
BOOL mmr_flag;							/* MMR�L���t���O */
BYTE mmr_seg;							/* MMR�Z�O�����g */
BOOL mmr_modify;						/* MMR��ԕύX�t���O */
#if XM7_VER >= 3
BYTE mmr_reg[0x80];						/* MMR���W�X�^ */
BOOL twr_flag;							/* TWR�L���t���O */
BYTE twr_reg;							/* TWR���W�X�^ */
BOOL mmr_ext;							/* �g��MMR�L���t���O */
BOOL mmr_fastmode;						/* MMR�����t���O */
BOOL mmr_extram;						/* �g��RAM�L���t���O */
BOOL mmr_fast_refresh;					/* �������t���b�V���t���O */
BOOL mmr_768kbmode;						/* �g��RAM���샂�[�h */
BOOL twr_fastmode;						/* TWR�������[�h */
BOOL dsp_400linetiming;					/* 400���C���^�C�~���O�o�̓t���O */
#else
BYTE mmr_reg[0x40];						/* MMR���W�X�^ */
BOOL twr_flag;							/* TWR�L���t���O */
BYTE twr_reg;							/* TWR���W�X�^ */
#if XM7_VER == 1
BYTE bnk_reg;							/* �g��RAM�o���N�Z���N�g���W�X�^ */
BOOL mmr_64kbmode;						/* �g��RAM���샂�[�h */
#endif
#endif

/* I/O�^RAM�f�B�X�N�J�[�h */
#if XM7_VER >= 3
#if defined(MR2)
BYTE mr2_nowcnt;						/* MR2 �Z�N�^�J�E���g */
WORD mr2_secreg;						/* MR2 �Z�N�^���W�X�^ */
#endif
#if defined(RD512)
WORD rd512_secreg;						/* RD512 �Z�N�^���W�X�^ */
#endif
#if defined(R2D2)
BYTE r2d2_nowcnt;						/* R2D2 �Z�N�^�J�E���g */
WORD r2d2_secreg;						/* R2D2 �Z�N�^���W�X�^ */
#endif
#endif

/*
 *	MMR
 *	������
 */
BOOL FASTCALL mmr_init(void)
{
	/* ����N�����̂�MMR���N���A����悤�ɕύX */
	memset(mmr_reg, 0, sizeof(mmr_reg));

#if XM7_VER >= 3
	/* 768KB �g��RAM�J�[�h���f�B�Z�[�u���E768KB�L���� */
	mmr_extram = FALSE;
	mmr_768kbmode = TRUE;

	/* 400���C���^�C�~���O�o�͂��f�B�Z�[�u�� */
	dsp_400linetiming = FALSE;
#elif XM7_VER == 1
	/* �g��RAM��192KB���샂�[�h */
	mmr_64kbmode = FALSE;
#endif

	return TRUE;
}

/*
 *	MMR
 *	�N���[���A�b�v
 */
void FASTCALL mmr_cleanup(void)
{
}

/*
 *	MMR
 *	���Z�b�g
 */
void FASTCALL mmr_reset(void)
{
	/* MMR/TWR */
	mmr_flag = FALSE;
	twr_flag = FALSE;
	mmr_seg = 0;
	twr_reg = 0;
	mmr_modify = FALSE;
#if XM7_VER == 1
	bnk_reg = 0x02;
#endif

#if XM7_VER >= 3
	/* MMR/TWR(AV40�g��) */
	mmr_ext = FALSE;
	mmr_fastmode = FALSE;
	mmr_fast_refresh = FALSE;
	twr_fastmode = FALSE;

	/* I/O�^RAM�f�B�X�N */
#if defined(MR2)
	mr2_nowcnt = 0;
	mr2_secreg = 0;
#endif
#if defined(RD512)
	rd512_secreg = 0;
#endif
#if defined(R2D2)
	r2d2_nowcnt = 0;
	r2d2_secreg = 0;
#endif
#endif
}

/*-[ �������}�l�[�W�� ]-----------------------------------------------------*/

/*
 *	TWR�A�h���X�ϊ�
 */
BOOL FASTCALL mmr_trans_twr(WORD addr, DWORD *taddr)
{
	/* TWR�L���� */
	if (!twr_flag) {
		return FALSE;
	}

	/* �A�h���X�v���`�F�b�N */
	if ((addr < 0x7c00) || (addr > 0x7fff)) {
		return FALSE;
	}

	/* TWR���W�X�^���ϊ� */
	*taddr = (DWORD)twr_reg;
	*taddr *= 256;
	*taddr += addr;
	*taddr &= 0xffff;
#if XM7_VER == 1
	/* FM-77(not AV)�̃e�L�X�g��Ԃɍ��킹�ăA�h���X��␳ */
	*taddr |= 0x20000;
#endif

	return TRUE;
}

/*
 *	MMR
 *	�A�h���X�ϊ�
 */
DWORD FASTCALL mmr_trans_mmr(WORD addr)
{
	DWORD maddr;
	int offset;

	/* MMR�L���� */
	if (!mmr_flag) {
		return (DWORD)(0x30000 | addr);
	}

	/* MMR���W�X�^���擾 */
	offset = (int)addr;
	offset >>= 12;

#if XM7_VER >= 3
	/* �g��MMR��off�Ȃ�A�Z�O�����g��0�`3�܂� */
	if (mmr_ext) {
		offset |= (mmr_seg * 0x10);
	}
	else {
		offset |= ((mmr_seg & 0x03) * 0x10);
	}
#else
	offset |= ((mmr_seg & 0x03) * 0x10);
#endif

	/* �g��MMR��off�Ȃ�A6bit�̂ݗL�� */
	maddr = (DWORD)mmr_reg[offset];
#if XM7_VER >= 3
	if (!mmr_ext) {
		maddr &= 0x3f;
	}
#else
	maddr &= 0x3f;
#endif
	maddr <<= 12;

	/* ����12�r�b�g�ƍ��� */
	addr &= 0xfff;
	maddr |= addr;

	return maddr;
}

/*
 *	MMR
 *	�����A�h���X���_���A�h���X�ϊ�
 */
DWORD FASTCALL mmr_trans_phys_to_logi(DWORD target)
{
	DWORD addr;
	BYTE mmr_dat, target_seg;
	int i;

	/* ����16�r�b�g���擾�A�풓�̈�͉���16bit�����̂܂ܕԂ� */
	addr = (DWORD)(target & 0xffff);
	if ((addr >= 0xfc00) && (addr <= 0xffff)) {
		return (DWORD)addr;
	}

	/* MMR�̓��e���畨���A�h���X���Z�o(����?) */
	for (i = 0; i < 16; i++) {
#if xm7_VER >= 3
		if (mmr_ext) {
			mmr_dat = mmr_reg[((mmr_seg & 7) * 0x10) + i];
			target_seg = (BYTE)(target >> 12);
		}
		else {
			mmr_dat = (BYTE)(mmr_reg[((mmr_seg & 3) * 0x10) + i] & 0x3f);
			target_seg = (BYTE)((target >> 12) & 0x3f);
		}
#else
		mmr_dat = (BYTE)(mmr_reg[((mmr_seg & 3) * 0x10) + i] & 0x3f);
		target_seg = (BYTE)((target >> 12) & 0x3f);
#endif
		if (target_seg == mmr_dat) {
			return ((DWORD)i << 12) | (addr & 0xfff);
		}
	}

	/* �_���A�h���X�Ɋ��蓖�Ă��Ă��Ȃ��ꍇ�A����16bit�����̂܂ܕԂ� */
	return (DWORD)(addr & 0x0ffff);
}

/*
 *	���C��CPU�o�X
 *	�P�o�C�g�ǂݏo��
 */
BOOL FASTCALL mmr_extrb(WORD *addr, BYTE *dat)
{
	DWORD raddr, rsegment;

	/* $FC00�`$FFFF�͏풓��� */
	if (*addr >= 0xfc00) {
		return FALSE;
	}

	/* TWR,MMR��ʂ� */
	if (!mmr_trans_twr(*addr, &raddr)) {
		raddr = mmr_trans_mmr(*addr);
	}

	rsegment = (raddr & 0xf0000);

	/* �W����� */
	if (rsegment == 0x30000) {
		/* MMR�͍Ĕz�u�֎~ */
#if XM7_VER >= 2
		if (fm7_ver >= 2) {
			if ((raddr >= 0x3fd80) && (raddr <= 0x3fd97)) {
				*dat = 0xff;
				return TRUE;
			}
		}
#endif

#if XM7_VER == 1
		/* BASIC ROM�����RAM�E���LRAM�EI/O�̈�ɂ�ROM/RAM�������� */
		if ((raddr >= 0x3fc00) && (raddr < 0x3fe00)) {
			if (basicrom_en) {
				*dat = 0x00;
			}
			else {
				*dat = mainram_b[raddr - 0x38000];
			}
			return TRUE;
		}

		/* �u�[�g�̈�ɂ�ROM/RAM�������� */
		if (raddr >= 0x3fe00) {
			if (basicrom_en) {
				if (available_mmrboot) {
					*dat = boot_mmr[raddr - 0x3fe00];
				}
				else {
					*dat = boot_bas[raddr - 0x3fe00];
				}
			}
			else {
				*dat = boot_ram[raddr - 0x3fe00];
			}
			return TRUE;
		}
#endif

		/* $30�Z�O�����g */
		*addr = (WORD)(raddr & 0xffff);
		return FALSE;
	}

#if XM7_VER >= 2
	/* FM77AV �g��RAM */
	if (rsegment == 0x00000) {
		*dat = extram_a[raddr & 0xffff];
		return TRUE;
	}

	/* �T�u�V�X�e�� */
	if (rsegment == 0x10000) {
		if (subhalt_flag) {
			*dat = submem_readb((WORD)(raddr & 0xffff));
		}
		else {
			*dat = 0xff;
		}
		return TRUE;
	}

	/* ���{��J�[�h */
	if (rsegment == 0x20000) {
#if XM7_VER >= 2
		*dat = jcard_readb((WORD)(raddr & 0xffff));
#else
		*dat = 0xff;
#endif
		return TRUE;
	}

#if XM7_VER >= 3
	/* 768KB �g��RAM */
	if (rsegment >= 0x40000) {
		if (mmr_extram) {
			if (mmr_768kbmode || (rsegment <= 0x70000)) {
				*dat = extram_c[raddr - 0x40000];
			}
		}
		else {
			*dat = 0xff;
		}
		return TRUE;
	}
#endif

	return FALSE;
#else
	/* FM-77 �g��RAM */
	if (mmr_64kbmode) {
		if ((rsegment >> 16) == (DWORD)bnk_reg) {
			*dat = extram_a[(DWORD)((raddr & 0xffff) | 0x20000)];
		}
		else {
			*dat = 0xff;
		}
		return TRUE;
	}

	*dat = extram_a[raddr];
	return TRUE;
#endif
}

/*
 *	���C��CPU�o�X
 *	�P�o�C�g�ǂݏo��(I/O�Ȃ�)
 */
BOOL FASTCALL mmr_extbnio(WORD *addr, BYTE *dat)
{
	DWORD raddr, rsegment;

	/* $FC00�`$FFFF�͏풓��� */
	if (*addr >= 0xfc00) {
		return FALSE;
	}

	/* TWR,MMR��ʂ� */
	if (!mmr_trans_twr(*addr, &raddr)) {
		raddr = mmr_trans_mmr(*addr);
	}

	rsegment = (raddr & 0xf0000);

	/* �W����� */
	if (rsegment == 0x30000) {
		/* MMR�͍Ĕz�u�֎~ */
#if XM7_VER >= 2
		if (fm7_ver >= 2) {
			if ((raddr >= 0x3fd80) && (raddr <= 0x3fd97)) {
				*dat = 0xff;
				return TRUE;
			}
		}
#endif

#if XM7_VER == 1
		/* BASIC ROM�����RAM�E���LRAM�EI/O�̈�ɂ�ROM/RAM�������� */
		if ((raddr >= 0x3fc00) && (raddr < 0x3fe00)) {
			if (basicrom_en) {
				*dat = 0x00;
			}
			else {
				*dat = mainram_b[raddr - 0x38000];
			}
			return TRUE;
		}

		/* �u�[�g�̈�ɂ�ROM/RAM�������� */
		if (raddr >= 0x3fe00) {
			if (basicrom_en) {
				if (available_mmrboot) {
					*dat = boot_mmr[raddr - 0x3fe00];
				}
				else {
					*dat = boot_bas[raddr - 0x3fe00];
				}
			}
			else {
				*dat = boot_ram[raddr - 0x3fe00];
			}
			return TRUE;
		}
#endif

		/* $30�Z�O�����g */
		*addr = (WORD)(raddr & 0xffff);
		return FALSE;
	}

#if XM7_VER >= 2
	/* FM77AV �g��RAM */
	if (rsegment == 0x00000) {
		*dat = extram_a[raddr & 0xffff];
		return TRUE;
	}

	/* �T�u�V�X�e�� */
	if (rsegment == 0x10000) {
		if (subhalt_flag) {
			*dat = submem_readbnio((WORD)(raddr & 0xffff));
		}
		else {
			*dat = 0xff;
		}
		return TRUE;
	}

	/* ���{��J�[�h */
	if (rsegment == 0x20000) {
#if XM7_VER >= 2
		*dat = jcard_readb((WORD)(raddr & 0xffff));
#else
		*dat = 0xff;
#endif
		return TRUE;
	}

#if XM7_VER >= 3
	/* 768KB �g��RAM */
	if (rsegment >= 0x40000) {
		if (mmr_extram) {
			if (mmr_768kbmode || (rsegment <= 0x70000)) {
				*dat = extram_c[raddr - 0x40000];
			}
		}
		else {
			*dat = 0xff;
		}
		return TRUE;
	}
#endif

	return FALSE;
#else
	/* FM-77 �g��RAM */
	if (mmr_64kbmode) {
		if ((rsegment >> 16) == (DWORD)bnk_reg) {
			*dat = extram_a[(DWORD)((raddr & 0xffff) | 0x20000)];
		}
		else {
			*dat = 0xff;
		}
		return TRUE;
	}

	*dat = extram_a[raddr];
	return TRUE;
#endif
}

/*
 *	���C��CPU�o�X
 *	�P�o�C�g��������
 */
BOOL FASTCALL mmr_extwb(WORD *addr, BYTE dat)
{
	DWORD raddr, rsegment;

	/* $FC00�`$FFFF�͏풓��� */
	if (*addr >= 0xfc00) {
		return FALSE;
	}

	/* TWR,MMR��ʂ� */
	if (!mmr_trans_twr(*addr, &raddr)) {
		raddr = mmr_trans_mmr(*addr);
	}

	rsegment = (raddr & 0xf0000);

	/* �W����� */
	if (rsegment == 0x30000) {
		/* MMR�͍Ĕz�u�֎~ */
#if XM7_VER >= 2
		if (fm7_ver >= 2) {
			if ((raddr >= 0x3fd80) && (raddr <= 0x3fd97)) {
				return TRUE;
			}
		}
#endif

#if XM7_VER == 1
		/* BASIC ROM�����RAM�E���LRAM�EI/O�̈�ɂ�ROM/RAM�������� */
		if ((raddr >= 0x3fc00) && (raddr < 0x3fe00)) {
			if (!basicrom_en) {
				mainram_b[raddr - 0x38000] = dat;
			}
			return TRUE;
		}

		/* �u�[�g�̈�ɂ�ROM/RAM�������� */
		if (raddr >= 0x3fe00) {
			if (!basicrom_en) {
				boot_ram[raddr - 0x3fe00] = dat;
			}
			return TRUE;
		}
#endif

		/* $30�Z�O�����g */
		*addr = (WORD)(raddr & 0xffff);
		return FALSE;
	}

#if XM7_VER >= 2
	/* FM77AV �g��RAM */
	if (rsegment == 0x00000) {
		extram_a[raddr & 0xffff] = dat;
		return TRUE;
	}

	/* �T�u�V�X�e�� */
	if (rsegment == 0x10000) {
		if (subhalt_flag) {
			submem_writeb((WORD)(raddr & 0xffff), dat);
		}
		return TRUE;
	}

	/* ���{��J�[�h */
	if (rsegment == 0x20000) {
#if XM7_VER >= 2
		jcard_writeb((WORD)(raddr & 0xffff), dat);
#endif
		return TRUE;
	}

#if XM7_VER >= 3
	/* AV40�g��RAM */
	if (rsegment >= 0x40000) {
		if (mmr_extram) {
			if (mmr_768kbmode || (rsegment <= 0x70000)) {
				extram_c[raddr - 0x40000] = dat;
			}
		}
		return TRUE;
	}
#endif

	return FALSE;
#else
	/* FM-77 �g��RAM */
	if (mmr_64kbmode) {
		if ((rsegment >> 16) == (DWORD)bnk_reg) {
			extram_a[(DWORD)((raddr & 0xffff) | 0x20000)] = dat;
		}
		return TRUE;
	}

	extram_a[raddr] = dat;
	return TRUE;
#endif

}

/*-[ MR2 I/O�^RAM�f�B�X�N ]-------------------------------------------------*/

#if (XM7_VER >= 3) && defined(MR2)
/*
 *	MR2
 *	�A�h���X�v�Z
 */
static DWORD FASTCALL mr2_address(void)
{
	DWORD tmp;

	ASSERT (XM7_VER >= 3);

	if (mr2_secreg <= 0x0bff) {
		/* �v�Z���@�͓K���Ȃ̂ň���Ă���\���A��(^^; 2002/07/29 */
		tmp = (0xbff - mr2_secreg) << 8;
		tmp |= mr2_nowcnt;
	}
	else {
		tmp = 0;
	}

	return tmp;
}

/*
 *	MR2
 *	�f�[�^���[�h
 */
static BYTE FASTCALL mr2_read_data(void)
{
	BYTE dat;

	ASSERT (XM7_VER >= 3);

	/* �Z�N�^���W�X�^���ُ�̏ꍇ�͓ǂݏo���Ȃ� */
	if (mr2_secreg <= 0x0bff) {
		dat = extram_c[mr2_address()];
		mr2_nowcnt ++;
	}
	else {
		dat = 0xff;
	}

	return dat;
}

/*
 *	MR2
 *	�f�[�^���C�g
 */
static void FASTCALL mr2_write_data(BYTE dat)
{
	ASSERT (XM7_VER >= 3);

	/* �Z�N�^���W�X�^���ُ�̏ꍇ�͏������߂Ȃ� */
	if (mr2_secreg <= 0x0bff) {
		extram_c[mr2_address()] = dat;
		mr2_nowcnt ++;
	}
}

/*
 *	MR2
 *	�Z�N�^���W�X�^�X�V
 */
static void FASTCALL mr2_update_sector(WORD addr, BYTE dat)
{
	ASSERT (XM7_VER >= 3);

	/* �Z�N�^���W�X�^���X�V */
	if (addr == 0xfd9e) {
		mr2_secreg &= (WORD)0x00ff;
		mr2_secreg |= (WORD)((dat & 0x0f) << 8);
	}
	else {
		mr2_secreg &= (WORD)0x0f00;
		mr2_secreg |= (WORD)dat;
	}

	/* �͈̓`�F�b�N */
	if (mr2_secreg >= 0x0c00) {
		ASSERT(FALSE);
		mr2_secreg = 0x0c00;
	}

	/* �Z�N�^�J�E���^�����Z�b�g */
	mr2_nowcnt = 0;
}
#endif

/*-[ RD512 I/O�^RAM�f�B�X�N ]-----------------------------------------------*/

#if (XM7_VER >= 3) && defined(RD512)
/*
 *	RD512
 *	�A�h���X�v�Z
 */
static DWORD FASTCALL rd512_address(WORD addr)
{
	ASSERT (XM7_VER >= 3);

	return (DWORD)(rd512_secreg + ((addr & 0x07) << 16) + 0x40000);
}

/*
 *	RD512
 *	�f�[�^���[�h
 */
static BYTE FASTCALL rd512_read_data(WORD addr)
{
	BYTE dat;

	ASSERT (XM7_VER >= 3);

	if ((addr >= 0xfd48) && (addr <= 0xfd4f)) {
		dat = extram_c[rd512_address(addr)];
	}
	else {
		dat = 0xff;
	}

	return dat;
}

/*
 *	RD512
 *	�f�[�^���C�g
 */
static void FASTCALL rd512_write_data(WORD addr, BYTE dat)
{
	ASSERT (XM7_VER >= 3);

	if ((addr >= 0xfd48) && (addr <= 0xfd4f)) {
		extram_c[rd512_address(addr)] = dat;
	}
}

/*
 *	RD512
 *	�Z�N�^���W�X�^�X�V
 */
static void FASTCALL rd512_update_sector(WORD addr, BYTE dat)
{
	ASSERT (XM7_VER >= 3);

	/* �Z�N�^���W�X�^���X�V */
	if (addr == 0xfd41) {
		rd512_secreg &= (WORD)0x00ff;
		rd512_secreg |= (WORD)((dat & 0xff) << 8);
	}
	else {
		rd512_secreg &= (WORD)0xff00;
		rd512_secreg |= (WORD)dat;
	}
}
#endif

/*-[ R2D2 I/O�^RAM�f�B�X�N ]------------------------------------------------*/

#if (XM7_VER >= 3) && defined(R2D2)
/*
 *	R2D2
 *	�A�h���X�v�Z
 */
static DWORD FASTCALL r2d2_address(WORD addr)
{
	DWORD tmp;

	ASSERT (XM7_VER >= 3);

	tmp =  (DWORD)(r2d2_secreg << 8);
	tmp |= (DWORD)(r2d2_nowcnt & 0xfe);
	tmp |= (DWORD)(addr & 0x01);
	tmp += 0x40000;

	return tmp;
}

/*
 *	R2D2
 *	�f�[�^���[�h
 */
static BYTE FASTCALL r2d2_read_data(WORD addr)
{
	BYTE dat;

	ASSERT (XM7_VER >= 3);

	if (r2d2_secreg <= 0x07ff) {
		dat = extram_c[r2d2_address(addr)];
	}
	r2d2_nowcnt ++;

	return dat;
}

/*
 *	R2D2
 *	�f�[�^���C�g
 */
static void FASTCALL r2d2_write_data(WORD addr, BYTE dat)
{
	ASSERT (XM7_VER >= 3);

	if (r2d2_secreg <= 0x07ff) {
		extram_c[r2d2_address(addr)] = dat;
	}
	r2d2_nowcnt ++;
}

/*
 *	R2D2
 *	�Z�N�^���W�X�^�X�V
 */
static void FASTCALL r2d2_update_sector(WORD addr, BYTE dat)
{
	ASSERT (XM7_VER >= 3);

	/* �Z�N�^���W�X�^���X�V */
	if (addr == 0xfd62) {
		r2d2_secreg &= (WORD)0x00ff;
		r2d2_secreg |= (WORD)((dat & 0x07) << 8);
	}
	else {
		r2d2_secreg &= (WORD)0x0700;
		r2d2_secreg |= (WORD)dat;
	}

	/* �J�E���^������ */
	r2d2_nowcnt = 0;
}
#endif

/*-[ �������}�b�v�hI/O ]----------------------------------------------------*/

/*
 *	MMR
 *	�P�o�C�g�ǂݏo��
 */
BOOL FASTCALL mmr_readb(WORD addr, BYTE *dat)
{
	BYTE tmp;

	/* �o�[�W�����`�F�b�N */
#if XM7_VER >= 2
	if (fm7_ver < 2) {
		return FALSE;
	}
#else
	if (fm_subtype != FMSUB_FM77) {
		return FALSE;
	}
#endif

	switch (addr) {
#if XM7_VER >= 2
		/* �u�[�g�X�e�[�^�X */
		case 0xfd0b:
			if (boot_mode == BOOT_BASIC) {
				*dat = 0xfe;
			}
			else {
				*dat = 0xff;
			}

#if defined(RSC)
			/* RS-232C CD�M�� */
			if (rs_cd) {
				*dat &= (BYTE)~RSCB_CD;
			}
#endif
			return TRUE;

		/* �C�j�V�G�[�^ROM */
		case 0xfd10:
			*dat = 0xff;
			return TRUE;
#endif

#if XM7_VER == 1
		/* �g��RAM�o���N�Z���N�g���W�X�^ */
		case 0xfd2f:
			*dat = (BYTE)(0xfc | (bnk_reg & 0x03));
			return TRUE;
#endif

		/* MMR�Z�O�����g */
		case 0xfd90:
			*dat = 0xff;
			return TRUE;

		/* TWR�I�t�Z�b�g */
		case 0xfd92:
			*dat = 0xff;
			return TRUE;

		/* ���[�h�Z���N�g */
		case 0xfd93:
			tmp = 0xff;
			if (!mmr_flag) {
				tmp &= (BYTE)(~0x80);
			}
			if (!twr_flag) {
				tmp &= ~0x40;
			}
			if (!bootram_rw) {
				tmp &= ~1;
			}
			*dat = tmp;
			return TRUE;

#if XM7_VER >= 3
		/* �g��MMR/CPU�X�s�[�h */
		case 0xfd94:
			*dat = 0xff;
			return TRUE;

		/* ���[�h�Z���N�g�Q */
		case 0xfd95:
			tmp = 0xff;
			if (fm7_ver >= 3) {
				/* bit7:�g��ROM�Z���N�g */
				if (extrom_sel) {
					tmp &= (BYTE)~0x80;
				}
				/* bit4:MMR�g�p���̑��x�ቺ��}�~ */
				if (!mmr_fastmode) {
					tmp &= (BYTE)~0x08;
				}
				/* bit0:400���C���^�C�~���O�o�̓X�e�[�^�X */
				/*      XM7�ł�1(200���C���o��)�Œ� �c���������B */
				if (dsp_400linetiming) {
					tmp &= (BYTE)~0x01;
				}
			}

			*dat = tmp;
			return TRUE;

#if defined(MR2)
		/* MR2 �f�[�^���W�X�^ */
		case 0xfd9c:
			if (mmr_extram && mmr_768kbmode) {
				*dat = mr2_read_data();
			}
			else {
				*dat = 0xff;
			}
			return TRUE;

		/* MR2 �Z�N�^���W�X�^(Write Only) */
		case 0xfd9e:
		case 0xfd9f:
			*dat = 0xff;
			return TRUE;
#endif

#if defined(RD512)
		/* RD512 �Z�N�^���W�X�^(Write Only) */
		case 0xfd40:
		case 0xfd41:
			*dat = 0xff;
			return TRUE;

		/* RD512 �f�[�^���W�X�^ */
		case 0xfd48:
		case 0xfd49:
		case 0xfd4a:
		case 0xfd4b:
		case 0xfd4c:
		case 0xfd4d:
		case 0xfd4e:
		case 0xfd4f:
			if (mmr_extram) {
				*dat = rd512_read_data(addr);
			}
			else {
				*dat = 0xff;
			}
			return TRUE;
#endif

#if defined(R2D2)
		/* R2D2 �f�[�^���W�X�^ */
		case 0xfd60:
		case 0xfd61:
			if (mmr_extram) {
				*dat = r2d2_read_data(addr);
			}
			else {
				*dat = 0xff;
			}
			return TRUE;

		/* R2D2 �Z�N�^���W�X�^(Write Only) */
		case 0xfd62:
		case 0xfd63:
			*dat = 0xff;
			return TRUE;
#endif
#endif
	}

	/* MMR���W�X�^ */
	if ((addr >= 0xfd80) && (addr <= 0xfd8f)) {
#if XM7_VER >= 3
		if (mmr_ext) {
			tmp = mmr_reg[mmr_seg * 0x10 + (addr - 0xfd80)];
		}
		else {
			tmp = mmr_reg[(mmr_seg & 3) * 0x10 + (addr - 0xfd80)];
			/* tmp &= 0x3f; */
		}
#else
		tmp = mmr_reg[(mmr_seg & 3) * 0x10 + (addr - 0xfd80)];
		/* tmp &= 0x3f; */
#endif
		*dat = tmp;
		return TRUE;
	}

	return FALSE;
}

/*
 *	MMR
 *	�P�o�C�g��������
 */
BOOL FASTCALL mmr_writeb(WORD addr, BYTE dat)
{
	/* �o�[�W�����`�F�b�N */
#if XM7_VER >= 2
	if (fm7_ver < 2) {
		return FALSE;
	}
#else
	if (fm_subtype != FMSUB_FM77) {
		return FALSE;
	}
#endif

	switch (addr) {
		/* �C�j�V�G�[�^ROM */
#if XM7_VER >= 2
		case 0xfd10:
			if (dat & 0x02) {
				initrom_en = FALSE;
			}
			else {
				initrom_en = TRUE;
			}
			return TRUE;
#endif

#if XM7_VER == 1
		/* �g��RAM�o���N�Z���N�g���W�X�^ */
		case 0xfd2f:
			bnk_reg = (BYTE)(dat & 0x03);
			return TRUE;
#endif

		/* MMR�Z�O�����g */
		case 0xfd90:
			mmr_seg = (BYTE)(dat & 0x07);
			return TRUE;

		/* TWR�I�t�Z�b�g */
		case 0xfd92:
			twr_reg = dat;
			return TRUE;

		/* ���[�h�Z���N�g */
		case 0xfd93:
			if (dat & 0x80) {
				if (!mmr_flag) {
					mmr_flag = TRUE;
#if XM7_VER >= 3
					if (!mmr_fastmode) {
						mmr_modify = TRUE;
					}
#else
					mmr_modify = TRUE;
#endif
				}
			}
			else {
				if (mmr_flag) {
					mmr_flag = FALSE;
#if XM7_VER >= 3
					if (!mmr_fastmode) {
						mmr_modify = TRUE;
					}
#else
					mmr_modify = TRUE;
#endif
				}
			}
			if (dat & 0x40) {
				if (!twr_flag) {
					twr_flag = TRUE;
#if XM7_VER >= 3
					if (!mmr_fastmode) {
						mmr_modify = TRUE;
					}
#else
					mmr_modify = TRUE;
#endif
				}
			}
			else {
				if (twr_flag) {
					twr_flag = FALSE;
#if XM7_VER >= 3
					if (!mmr_fastmode) {
						mmr_modify = TRUE;
					}
#else
					mmr_modify = TRUE;
#endif
				}
			}
			if (dat & 0x01) {
				bootram_rw = TRUE;
			}
			else {
				bootram_rw = FALSE;
			}
			return TRUE;

#if XM7_VER >= 3
		/* �g��MMR/CPU�X�s�[�h */
		case 0xfd94:
			if (fm7_ver >= 3) {
				/* bit7:�g��MMR */
				if (dat & 0x80) {
					mmr_ext = TRUE;
				}
				else {
					mmr_ext = FALSE;
				}
				/* bit2:���t���b�V���X�s�[�h */
				if (dat & 0x04) {
					if (!mmr_fast_refresh) {
						mmr_fast_refresh = TRUE;
						if (!mmr_fastmode) {
							mmr_modify = TRUE;
						}
					}
				}
				else {
					if (mmr_fast_refresh) {
						mmr_fast_refresh = FALSE;
						if (!mmr_fastmode) {
							mmr_modify = TRUE;
						}
					}
				}
				/* bit0:�E�B���h�E�X�s�[�h */
				if (dat & 0x01) {
					if (!twr_fastmode) {
						twr_fastmode = TRUE;
						if (!mmr_fastmode) {
							mmr_modify = TRUE;
						}
					}
				}
				else {
					if (twr_fastmode) {
						twr_fastmode = FALSE;
						if (!mmr_fastmode) {
							mmr_modify = TRUE;
						}
					}
				}
			}
			return TRUE;

		/* ���[�h�Z���N�g�Q */
		case 0xfd95:
			if (fm7_ver >= 3) {
				/* bit7:�g��ROM�Z���N�g */
				if (dat & 0x80) {
					extrom_sel = TRUE;
				}
				else {
					extrom_sel = FALSE;
				}
				/* bit4:MMR�g�p���̑��x�ቺ��}�~ */
				if (dat & 0x08) {
					if (!mmr_fastmode) {
						mmr_fastmode = TRUE;
						mmr_modify = TRUE;
					}
				}
				else {
					if (mmr_fastmode) {
						mmr_fastmode = FALSE;
						mmr_modify = TRUE;
					}
				}
			}
			return TRUE;

#if defined(MR2)
		/* MR2 �f�[�^���W�X�^ */
		case 0xfd9c:
			if (mmr_768kbmode) {
				mr2_write_data(dat);
			}
			return TRUE;

		/* MR2 �Z�N�^���W�X�^ */
		case 0xfd9e:
		case 0xfd9f:
			mr2_update_sector(addr, dat);
			return TRUE;
#endif

#if defined(RD512)
		/* RD512 �Z�N�^���W�X�^ */
		case 0xfd40:
		case 0xfd41:
			rd512_update_sector(addr, dat);
			return TRUE;

		/* RD512 �f�[�^���W�X�^ */
		case 0xfd48:
		case 0xfd49:
		case 0xfd4a:
		case 0xfd4b:
		case 0xfd4c:
		case 0xfd4d:
		case 0xfd4e:
		case 0xfd4f:
			if (mmr_extram) {
				rd512_write_data(addr, dat);
			}
			return TRUE;
#endif

#if defined(R2D2)
		/* R2D2 �f�[�^���W�X�^ */
		case 0xfd60:
		case 0xfd61:
			if (mmr_extram) {
				r2d2_write_data(addr, dat);
			}
			return TRUE;

		/* R2D2 �Z�N�^���W�X�^ */
		case 0xfd62:
		case 0xfd63:
			r2d2_update_sector(addr, dat);
			return TRUE;
#endif
#endif
	}

	/* MMR���W�X�^ */
	if ((addr >= 0xfd80) && (addr <= 0xfd8f)) {
#if XM7_VER >= 3
		/* �����ł̃f�[�^��8bit���ׂċL�� */
		if (mmr_ext) {
			mmr_reg[mmr_seg * 0x10 + (addr - 0xfd80)] = (BYTE)dat;
		}
		else {
			mmr_reg[(mmr_seg & 3) * 0x10 + (addr - 0xfd80)] = (BYTE)dat;
		}
#else
		mmr_reg[(mmr_seg & 3) * 0x10 + (addr - 0xfd80)] = (BYTE)dat;
#endif
		return TRUE;
	}

	return FALSE;
}

/*-[ �t�@�C��I/O ]----------------------------------------------------------*/

/*
 *	MMR
 *	�Z�[�u
 */
BOOL FASTCALL mmr_save(int fileh)
{
	if (!file_bool_write(fileh, mmr_flag)) {
		return FALSE;
	}
	if (!file_byte_write(fileh, mmr_seg)) {
		return FALSE;
	}

#if XM7_VER >= 3
	/* Ver8�g���� */
	if (!file_write(fileh, mmr_reg, 0x80)) {
		return FALSE;
	}
	if (!file_bool_write(fileh, mmr_ext)) {
		return FALSE;
	}
	if (!file_bool_write(fileh, mmr_fastmode)) {
		return FALSE;
	}
	if (!file_bool_write(fileh, mmr_extram)) {
		return FALSE;
	}
	if (mmr_extram) {
		if (!file_write(fileh, extram_c, 0xc0000)) {
			return FALSE;
		}
	}
#if defined(MR2)
	if (!file_byte_write(fileh, mr2_nowcnt)) {
		return FALSE;
	}
	if (!file_word_write(fileh, mr2_secreg)) {
		return FALSE;
	}
#else
	if (!file_byte_write(fileh, 0)) {
		return FALSE;
	}
	if (!file_word_write(fileh, 0)) {
		return FALSE;
	}
#endif
#else
	if (!file_write(fileh, mmr_reg, 0x40)) {
		return FALSE;
	}
#endif
#if XM7_VER == 1
	if (!file_byte_write(fileh, bnk_reg)) {
		return FALSE;
	}
	if (!file_bool_write(fileh, mmr_64kbmode)) {
		return FALSE;
	}
#endif

	if (!file_bool_write(fileh, twr_flag)) {
		return FALSE;
	}
	if (!file_byte_write(fileh, twr_reg)) {
		return FALSE;
	}
#if XM7_VER >= 3
	if (!file_bool_write(fileh, twr_fastmode)) {
		return FALSE;
	}
	if (!file_bool_write(fileh, mmr_768kbmode)) {
		return FALSE;
	}
#endif

	return TRUE;
}

/*
 *	MMR
 *	���[�h
 */
BOOL FASTCALL mmr_load(int fileh, int ver)
{
#if (XM7_VER >= 3) && !defined(MR2)
	WORD tmp;
#endif

	/* �o�[�W�����`�F�b�N */
	if (ver < 200) {
		return FALSE;
	}

	if (!file_bool_read(fileh, &mmr_flag)) {
		return FALSE;
	}
	if (!file_byte_read(fileh, &mmr_seg)) {
		return FALSE;
	}

#if XM7_VER >= 3
	/* �t�@�C���o�[�W����8�Ŋg�� */
	if (ver >= 800) {
		if (!file_read(fileh, mmr_reg, 0x80)) {
			return FALSE;
		}
		if (!file_bool_read(fileh, &mmr_ext)) {
			return FALSE;
		}
		if (!file_bool_read(fileh, &mmr_fastmode)) {
			return FALSE;
		}
		if (!file_bool_read(fileh, &mmr_extram)) {
			return FALSE;
		}
		if (mmr_extram) {
			if (!file_read(fileh, extram_c, 0xc0000)) {
				return FALSE;
			}
		}
		if (ver >= 902) {
#if defined(MR2)
			if (!file_byte_read(fileh, &mr2_nowcnt)) {
				return FALSE;
			}
			if (!file_word_read(fileh, &mr2_secreg)) {
				return FALSE;
			}
#else
			if (!file_byte_read(fileh, (BYTE *)&tmp)) {
				return FALSE;
			}
			if (!file_word_read(fileh, &tmp)) {
				return FALSE;
			}
#endif
		}
#if defined(MR2)
		else {
			mr2_nowcnt = 0;
			mr2_secreg = 0;
		}
#endif
	}
	else {
		/* Ver5�݊� */
		if (!file_read(fileh, mmr_reg, 0x40)) {
			return FALSE;
		}
		mmr_ext = FALSE;
		mmr_fastmode = FALSE;
		mmr_extram = FALSE;
#if defined(MR2)
		mr2_nowcnt = 0;
		mr2_secreg = 0;
#endif
	}
#else
	if (!file_read(fileh, mmr_reg, 0x40)) {
		return FALSE;
	}
#endif
#if XM7_VER == 1
	if (ver >= 309) {
		if (!file_byte_read(fileh, &bnk_reg)) {
			return FALSE;
		}
		if (!file_bool_read(fileh, &mmr_64kbmode)) {
			return FALSE;
		}
	}
#endif

	if (!file_bool_read(fileh, &twr_flag)) {
		return FALSE;
	}
	if (!file_byte_read(fileh, &twr_reg)) {
		return FALSE;
	}

#if XM7_VER >= 3
	/* Ver9.16�g�� */
	if (ver >= 916) {
		if (!file_bool_read(fileh, &twr_fastmode)) {
			return FALSE;
		}
	}
	/* Ver9.20�g�� */
	if (ver >= 920) {
		if (!file_bool_read(fileh, &mmr_768kbmode)) {
			return FALSE;
		}
	}
#endif

	return TRUE;
}
