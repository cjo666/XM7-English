/*
 *	FM-7 EMULATOR "XM7"
 *
 *	Copyright (C) 1999-2017 �o�h�D(Twitter:@xm6_original)
 *	Copyright (C) 2001-2017 Ryu Takegami (Twitter:@RyuTakegami)
 *
 *	[ �f�B�X�v���C ]
 *
 *	RHG����
 *	  2001.07.23(��)	��ʃ��[�h�̈�����ύX
 *	  2001.07.27		�s�v�ȍĕ`��̂��߂ɑ��x���ቺ��������C��
 *	  2001.07.29		400���C�����[�h���̃|�C���^�ݒ�l���������������C��
 *	  2001.11.10		400���C�����[�h���̃X�N���[���������C��(��)
 *	  2001.11.19		VRAM�̔z�u�����@�ɍ��킹�����Ƃɔ����ύX����(^^;
 *	  2001.11.25		�n�[�h�E�F�A�E�B���h�E�̋�����ύX
 *	  2002.03.06		FM-7���[�h����$D430�ւ̏������݂��L���ɂȂ�����C��
 *	  2002.05.25		V2�R���p�C�����ɕs�v�ȃR�[�h�𐶐����Ȃ��悤�ɏC��
 *						4096�F�E26���F�̃X�N���[�������𓝍�
 *	  2002.11.01		C��memcpy400l��ǉ�
 *	  2003.02.20		200���C�����[�h��400���C��/4096�F���[�h�ŕʂɂȂ��Ă�
 *						��VRAM�\���|�C���^(�\�E��)�𓝍�
 *	  2003.03.05		��ʃ��[�h��200���C��8�F���[�h�ȊO�̏�ԂŃT�u���j�^��
 *						�؂�ւ���Ɖ�ʂ����������Ȃ邱�Ƃ���������C��
 *	  2003.03.29		�n�[�h�E�F�A�E�B���h�E���W�X�^�������ݕ���ύX
 *						�n�[�h�E�F�A�E�B���h�E���W�X�^�̗L���r�b�g����ύX
 *	  2003.06.02		CLR���߂�BUSY�t���O�𗎂Ƃ��ƃf�b�h���b�N������ɑ�
 *						��
 *	  2003.11.20		XM7 V1.1�Ή�
 *						FM-7���[�h����NMI�}�X�N��������\���������������C
 *						��
 *	  2003.11.21		$FD04 bit5(����ROM�ڑ��̐؂芷���@�\)�ɑΉ�
 *	  2004.01.24		���Z�b�g���ɒ��O��VRAM�I�t�Z�b�g�ɂ��������ăX�N���[��
 *						���s���悤�ɕύX
 *	  2004.11.20		vsync_notify�̌Ăяo����\�����Ԃ̏I�����ɕύX
 *	  2010.01.13		TrueColor���̋P�x�ϊ��e�[�u����ǉ�
 *	  2010.12.09		FM77AV40/20�n���璊�o����JIS83��������ROM�f�[�^�ɂ���
 *						�āAFM-77���[�h��JIS78�����G�~�����[�V������ǉ�
 *	  2011.04.09		FM-77���[�h�Ŋ���ROM�A�h���X�v�Z���Ԉ���Ă����̂��C��
 *	  2011.05.30		���샂�[�h�ʂ�JIS78�����t�H���g��JIS83�����t�H���g�̓�
 *						�����ł���悤�ύX
 *	  2012.05.29		�X�e�[�g���[�h����400���C���J�[�h���݃t���O�Ɖ�ʃ��[
 *						�h�̐������`�F�b�N���s���悤�ɂ���
 *	  2012.08.15		�n�[�h�E�F�A�E�B���h�E���W�X�^�̏�������FALSE��Ԃ���
 *						���������C��
 *	  2013.02.12		���X�^�����_�����O�ɑΉ�
 */

#include <stdlib.h>
#include <string.h>
#include "xm7.h"
#include "display.h"
#include "subctrl.h"
#include "device.h"
#include "ttlpalet.h"
#include "multipag.h"
#include "mainetc.h"
#include "aluline.h"
#include "keyboard.h"
#include "kanji.h"
#include "event.h"

/*
 *	�O���[�o�� ���[�N
 */
BOOL crt_flag;							/* CRT ON�t���O */
BOOL vrama_flag;						/* VRAM�A�N�Z�X�t���O */
WORD vram_offset[2];					/* VRAM�I�t�Z�b�g���W�X�^ */
WORD crtc_offset[2];					/* CRTC�I�t�Z�b�g */
BOOL vram_offset_flag;					/* �g��VRAM�I�t�Z�b�g���W�X�^�t���O */
BOOL vsync_flag;						/* VSYNC�t���O */
BOOL draw_aftervsync;					/* ��ʕ`��ʒm�^�C�~���O */
int now_raster;							/* ���݃��X�^�ʒu */
BOOL magus_patch;						/* MAGUS�΍􏈗��t���O */

BOOL blank_flag;						/* �u�����L���O�t���O */
#if XM7_VER >= 2
BOOL subnmi_flag;						/* �T�uNMI�C�l�[�u���t���O */

BYTE vram_active;						/* �A�N�e�B�u�y�[�W */
BYTE *vram_aptr;						/* VRAM�A�N�e�B�v�|�C���^ */
BYTE vram_display;						/* �\���y�[�W */
BYTE *vram_dptr;						/* VRAM�\���|�C���^ */
#endif


#if XM7_VER >= 3
/* FM77AV40 */
BYTE screen_mode;						/* ��ʃ��[�h */
BYTE subram_vrambank;					/* �A�N�e�B�u�y�[�W(400line/26���F) */

WORD sub_kanji_addr;					/* ����ROM�A�h���X */
BOOL sub_kanji_bank;					/* ����ROM�I�� */

/* FM77AV40EX */
WORD window_x1, window_dx1;				/* �E�B���h�E X1 */
WORD window_y1, window_dy1;				/* �E�B���h�E Y1 */
WORD window_x2, window_dx2;				/* �E�B���h�E X2 */
WORD window_y2, window_dy2;				/* �E�B���h�E Y2 */
BOOL window_open;						/* �E�C���h�E�I�[�v���t���O */

BYTE block_active;						/* �A�N�e�B�u�u���b�N */
BYTE *vram_ablk;						/* �A�N�e�B�u�u���b�N�|�C���^ */
BYTE block_display;						/* �\���u���b�N */
BYTE *vram_bdptr;						/* ���\���u���b�N�|�C���^ */
BYTE *vram_dblk;						/* �\���u���b�N�|�C���^2 */
BYTE *vram_bdblk;						/* ���\���u���b�N�|�C���^2 */
#endif

#if XM7_VER == 1 && defined(L4CARD)
BOOL width40_flag;						/* width flag */
BOOL cursor_lsb;						/* �J�[�\���A�h���XLSB */
BOOL enable_400line;					/* 400���C�����[�h�t���O */
BOOL workram_select;					/* ���[�NRAM�Z���N�g�t���O */
WORD sub_kanji_addr;					/* ����ROM�A�h���X */

BYTE crtc_register[0x20];				/* CRTC���W�X�^ */
BYTE crtc_regnum;						/* CRTC���W�X�^�ԍ����W�X�^ */

WORD text_start_addr;					/* �e�L�X�g�X�^�[�g�A�h���X */
WORD cursor_addr;						/* �J�[�\���A�h���X */
BOOL text_blink;						/* �e�L�X�g�u�����N��� */
BOOL cursor_blink;						/* �J�[�\���u�����N��� */
#endif

/*
 *	�X�^�e�B�b�N ���[�N
 */
static BYTE *vram_buf;					/* VRAM�X�N���[���o�b�t�@ */
static BYTE vram_offset_count[2];		/* VRAM�I�t�Z�b�g�ݒ�J�E���^ */
static WORD blank_count;				/* �u�����N�J�E���^ */
#if XM7_VER == 1 && defined(L4CARD)
static DWORD text_scroll_count;			/* TVRAM�I�t�Z�b�g�ݒ�J�E���^ */
static DWORD text_cursor_count;			/* TVRAM�J�[�\���ʒu�ݒ�J�E���^ */
static BYTE text_blink_count;			/* �e�L�X�g�u�����N�J�E���^ */
static BYTE cursor_blink_count;			/* �J�[�\���u�����N�J�E���^ */
#endif

/*
 *	�v���g�^�C�v�錾
 */
static BOOL FASTCALL subcpu_event(void); /* �T�uCPU�^�C�}�C�x���g */
static BOOL FASTCALL display_vsync(void);/* VSYNC�C�x���g */
static BOOL FASTCALL display_blank(void);/* VBLANK,HBLANK�C�x���g */
static void FASTCALL display_setup(void);/* �C�x���g�Z�b�g�A�b�v */
static void FASTCALL vram_scroll(WORD offset);/* VRAM�X�N���[�� */
#if XM7_VER == 1 && defined(L4CARD)
static BOOL FASTCALL display_text_blink(void);/* �e�L�X�g�u�����N�C�x���g */
static BOOL FASTCALL display_cursor_blink(void);/* �J�[�\���u�����N�C�x���g */
#endif

#if (XM7_VER >= 3) && (defined(_OMF) || defined(_WIN32))
extern void memcpy400l(BYTE *, BYTE *, int);
										/* 400���C���X�N���[���p�������]�� */
#endif

/*
 *	24/32bit Color�p26���F���[�h�P�x�e�[�u��
 */
#if XM7_VER >= 3
const BYTE truecolorbrightness[64] = {
	  0,	  4,	  8,	 12,	 16,	 20,	 24,	 28,
	 32,	 36,	 40,	 45,	 49,	 53,	 57,	 61,
	 65,	 69,	 73,	 77,	 81,	 85,	 89,	 93,
	 97,	101,	105,	109,	113,	117,	121,	125,
	130,	134,	138,	142,	146,	150,	154,	158,
	162,	166,	170,	174,	178,	182,	186,	190,
	194,	198,	202,	206,	210,	215,	219,	223,
	227,	231,	235,	239,	243,	247,	251,	255,
};
#endif


/*
 *	�f�B�X�v���C
 *	������
 */
BOOL FASTCALL display_init(void)
{
	/* VRAM�X�N���[���p�o�b�t�@���m�� */
#if XM7_VER >= 3 || (XM7_VER == 1 && defined(L4CARD))
	vram_buf = (BYTE *)malloc(0x8000);
#else
	vram_buf = (BYTE *)malloc(0x4000);
#endif
	if (vram_buf == NULL) {
		return FALSE;
	}

#if XM7_VER == 1
#if defined(L4CARD)
	/* �����������200���C�����[�h�ɐݒ� */
	enable_400line = FALSE;
#endif
#endif

	/* �`��ʒm�^�C�~���O��VSYNC�� */
	draw_aftervsync = TRUE;

	/* MAGUS�p�b�`�͖��� */
	magus_patch = FALSE;

	/* CRTC�I�t�Z�b�g���[�N���N���A */
	memset(crtc_offset, 0, sizeof(crtc_offset));

	return TRUE;
}

/*
 *	�f�B�X�v���C
 *	�N���[���A�b�v
 */
void FASTCALL display_cleanup(void)
{
	ASSERT(vram_buf);
	if (vram_buf) {
		free(vram_buf);
	}
}

/*
 *	�f�B�X�v���C
 *	���Z�b�g
 */
void FASTCALL display_reset(void)
{
#if XM7_VER >= 2
	int i;
#endif

	/* ���Z�b�g���̃X�N���[���ʒu�␳ */
#if XM7_VER >= 2
	for (i=0; i<2; i++) {
		vram_scroll((WORD)-crtc_offset[i]);
	}
#else
	vram_scroll((WORD)-crtc_offset[0]);
#endif

	/* CRT���W�X�^ */
	crt_flag = FALSE;
	vrama_flag = FALSE;
	memset(vram_offset, 0, sizeof(vram_offset));
	memset(crtc_offset, 0, sizeof(crtc_offset));
	vram_offset_flag = FALSE;
	memset(vram_offset_count, 0, sizeof(vram_offset_count));
	now_raster = 0;

	/* ���荞�݁A�C�x���g */
	blank_flag = TRUE;
#if XM7_VER >= 2
	subnmi_flag = TRUE;
	vsync_flag = FALSE;

	/* �A�N�e�B�u�y�[�W�A�\���y�[�W������ */
	vram_active = 0;
	vram_aptr = vram_c;
	vram_display = 0;
	vram_dptr = vram_c;
#if XM7_VER >= 3
	subram_vrambank = 0;

	/* �n�[�h�E�F�A�E�B���h�E������ */
	window_x1 = window_x2 = 0;
	window_y1 = window_y2 = 0;
	window_dx1 = window_dx2 = 0;
	window_dy1 = window_dy2 = 0;
	window_open = FALSE;

	/* �A�N�e�B�u�u���b�N�A�\���u���b�N������ */
	block_active = 0;
	block_display = 0;
	vram_ablk = vram_c;
	vram_dblk = vram_c;
	vram_bdptr = vram_c + 0x18000;
	vram_bdblk = vram_bdptr;

	/* �T�u������ROM������ */
	sub_kanji_addr = 0;
	sub_kanji_bank = FALSE;
#endif	/* XM7_VER >= 3 */
#endif	/* XM7_VER >= 2 */

#if XM7_VER == 1
	/* ���荞�݁A�C�x���g */
	vsync_flag = FALSE;
	blank_flag = TRUE;

#if defined(L4CARD)
	/* TextVRAM */
	text_cursor_count = 0;
	text_scroll_count = 0;
	text_start_addr = 0x0000;
	text_blink = TRUE;
	cursor_addr = 0x0000;
	cursor_blink = TRUE;
	text_blink_count = 0;
	cursor_blink_count = 0;
	workram_select = FALSE;

	/* CRTC */
	crtc_regnum = 0;
	memset(crtc_register, 0, sizeof(crtc_register));

	/* �T�u������ROM������ */
	sub_kanji_addr = 0;

	/* 400���C�����[�h���̓e�L�X�g�u�����N�C�x���g��ǉ� */
	if (enable_400line && enable_400linecard) {
		schedule_setevent(EVENT_TEXT_BLINK, 160*1000, display_text_blink);
	}
	else {
		schedule_delevent(EVENT_TEXT_BLINK);
	}
#endif
#endif

	/* 20ms���ƂɋN�����C�x���g��ǉ� */
	schedule_setevent(EVENT_SUBTIMER, 20000, subcpu_event);

	/* VSYNC, VBLANK, HBLANK�̃Z�b�g�A�b�v */
	display_setup();
}

#if XM7_VER == 1 && defined(L4CARD)
/*
 *	�e�L�X�g�u�����N�C�x���g
 */
static BOOL FASTCALL display_text_blink(void)
{
	WORD addr;
	WORD ofsaddr;

	/* 400���C�����[�h�łȂ���΃C�x���g���폜���ċA�� */
	if (!(enable_400line && enable_400linecard)) {
		schedule_delevent(EVENT_TEXT_BLINK);
		return TRUE;
	}

	/* �e�L�X�g�u�����N (320ms) */
	if (text_blink_count ++ >= 2) {
		/* �u�����N��Ԃ𔽓] */
		text_blink = (!text_blink);

		/* �u�����N�L�����N�^������ */
		for (addr = 0; addr < 4000; addr += (WORD)2) {
			ofsaddr = (WORD)((addr + text_start_addr) & 0xFFE);
			if (tvram_c[ofsaddr + 1] & 0x10) {
				tvram_notify(ofsaddr, 0);
			}
		}

		/* �J�E���^�����Z�b�g */
		text_blink_count = 0;
	}

	/* �J�[�\���u�����N (320/160ms) */
	if (crtc_register[10] & 0x40) {
		cursor_blink_count++;
		if ((cursor_blink_count >= 2) || !(crtc_register[10] & 0x20)) {
			/* �u�����N��Ԃ𔽓] */
			cursor_blink = (!cursor_blink);

			/* �J�[�\���ʒu���ĕ\�� */
			tvram_notify(cursor_addr, 0);

			/* �J�E���^�����Z�b�g */
			cursor_blink_count = 0;
		}
	}

	return TRUE;
}
#endif

/*
 *	VSYNC�C�x���g
 */
static BOOL FASTCALL display_vsync(void)
{
	if (!vsync_flag) {
		if ((blank_count & 0xfff) > 0) {
			/* �ꏄ���� */
			display_setup();
			return TRUE;
		}

		/* ���ꂩ�琂������ */
		vsync_flag = TRUE;
#if (XM7_VER >= 3) || (XM7_VER == 1 && defined(L4CARD))
		if (blank_count & 0x1000) {
			/* 400���C��(24kHz���[�h) 0.33ms */
			schedule_setevent(EVENT_VSYNC, 330, display_vsync);
		}
		else {
			/* 200���C��(15kHz���[�h) 0.51ms */
			schedule_setevent(EVENT_VSYNC, 510, display_vsync);
		}
#else
		/* 200���C��(15kHz���[�h) 0.51ms */
		schedule_setevent(EVENT_VSYNC, 510, display_vsync);
#endif

		/* �r�f�I�f�B�W�^�C�Y */
#if XM7_VER >= 2
		if (digitize_enable) {
			if (digitize_keywait || simpose_mode == 0x03) {
				digitize_notify();
			}
		}
#else
		blank_count ++;
#endif
	}
	else {
		/* ���ꂩ�琂���\�� */
		now_raster = 0;
		vblankperiod_notify();
		vsync_flag = FALSE;
#if (XM7_VER >= 3) || (XM7_VER == 1 && defined(L4CARD))
		if (blank_count & 0x1000) {
			/* 400���C��(24kHz���[�h) 0.98ms + 16.4ms */
			schedule_setevent(EVENT_VSYNC, 980 + 16400, display_vsync);
		}
		else {
			/* 200���C��(15kHz���[�h) 1.91ms + 12.7ms */
			schedule_setevent(EVENT_VSYNC, 1910 + 12700, display_vsync);
		}
#else
		/* 200���C��(15kHz���[�h) 1.91ms + 12.7ms */
		schedule_setevent(EVENT_VSYNC, 1910 + 12700, display_vsync);
#endif

		/* VSYNC�ʒm */
		if (draw_aftervsync) {
			vsync_notify();
		}
	}

	return TRUE;
}

/*
 *	VBLANK,HBLANK�C�x���g
 */
static BOOL FASTCALL display_blank(void)
{
	if ((blank_count & 0xfff) == 0) {
		if (blank_flag) {
			/* ���ꂩ�珉�� */
#if (XM7_VER >= 3) || (XM7_VER == 1 && defined(L4CARD))
			if (blank_count & 0x1000) {
				/* 400���C���B11us */
				schedule_setevent(EVENT_BLANK, 11, display_blank);
			}
			else {
				/* 200���C���B24us */
				schedule_setevent(EVENT_BLANK, 24, display_blank);
			}
#else
			/* 200���C���B24us */
			schedule_setevent(EVENT_BLANK, 24, display_blank);
#endif
			blank_count++;
			return TRUE;
		}
	}

	if (blank_flag) {
		/* ���ꂩ�琅���\������ */
		blank_flag = FALSE;
#if (XM7_VER >= 3) || (XM7_VER == 1 && defined(L4CARD))
		if (blank_count & 0x1000) {
			/* 400���C���B30us */
			schedule_setevent(EVENT_BLANK, 30, display_blank);
		}
		else {
			/* 200���C���B39us�܂���40us */
			schedule_setevent(EVENT_BLANK, 39 + (blank_count & 1), display_blank);
		}
#else
		/* 200���C���B39us�܂���40us */
		schedule_setevent(EVENT_BLANK, 39 + (blank_count & 1), display_blank);
#endif
		blank_count++;
		return TRUE;
	}
	else {
		/* ���ꂩ�琅���������� */
		hblank_notify();
		now_raster ++;
		blank_flag = TRUE;
#if (XM7_VER >= 3) || (XM7_VER == 1 && defined(L4CARD))
		if (blank_count & 0x1000) {
			/* 400���C���B11us */
			schedule_setevent(EVENT_BLANK, 11, display_blank);
		}
		else {
			/* 200���C���B24us */
			schedule_setevent(EVENT_BLANK, 24, display_blank);
		}
#else
		/* 200���C���B24us */
		schedule_setevent(EVENT_BLANK, 24, display_blank);
#endif

		return TRUE;
	}
}

/*
 *	�C�x���g�Z�b�g�A�b�v
 */
static void FASTCALL display_setup(void)
{
#if (XM7_VER >= 3) || (XM7_VER == 1 && defined(L4CARD))
	/* 200���C��,400���C���̔��� */
#if XM7_VER >= 3
	if (screen_mode == SCR_400LINE) {
#else
	if (enable_400line && enable_400linecard) {
#endif
		blank_count = 0x1000;
		schedule_setevent(EVENT_VSYNC, 340, display_vsync);
		schedule_setevent(EVENT_BLANK, 1650, display_blank);
	}
	else {
		blank_count = 0;
		schedule_setevent(EVENT_VSYNC, 1520, display_vsync);
		schedule_setevent(EVENT_BLANK, 3940, display_blank);
	}
#else
	blank_count = 0;
	schedule_setevent(EVENT_VSYNC, 1520, display_vsync);
	schedule_setevent(EVENT_BLANK, 3940, display_blank);

	/* �����u�����L���O���� */
	vsync_flag = FALSE;
	blank_flag = TRUE;
#endif

	/* VSYNC�ʒm */
	if (!draw_aftervsync) {
		vsync_notify();
	}
}

/*
 *	�T�uCPU
 *	�C�x���g����
 */
static BOOL FASTCALL subcpu_event(void)
{
#if XM7_VER >= 2
	/* �O�̂��߁A�`�F�b�N */
	if (!subnmi_flag && (fm7_ver >= 2)) {
		return FALSE;
	}
#endif

	/* NMI���荞�݂��N���� */
	subcpu_nmi();
	return TRUE;
}

/*
 *	�|�C���^�Đݒ�
 */
void FASTCALL display_setpointer(BOOL redraw)
{
#if XM7_VER >= 3
	/* ��ʃ��[�h�ԍ� */
	if (mode400l) {
		screen_mode = SCR_400LINE;
	}
	else if (mode256k) {
		screen_mode = SCR_262144;
	}
	else if (mode320) {
		screen_mode = SCR_4096;
	}
	else {
		screen_mode = SCR_200LINE;
	}
#endif

#if XM7_VER >= 2
	/* �A�N�e�B�u�|�C���^�E�A�N�e�B�u�u���b�N�|�C���^ */
	vram_aptr = vram_c;
#if XM7_VER >= 3
	vram_ablk = vram_c;
	switch (screen_mode) {
		case SCR_400LINE	:	/* 640x400 8�F */
								vram_aptr += (subram_vrambank * 0x8000);
								if (block_active) {
									vram_aptr += 0x18000;
									vram_ablk += 0x18000;
								}
								break;
		case SCR_262144		:	/* 320x200 262144�F */
								switch (subram_vrambank) {
									case	0	: break;
									case	1	: vram_aptr += 0x4000;
												  break;
									case	2	: vram_aptr += 0x18000;
												  break;
								}
								break;
		default				:	/* 640x200���A320x200 4096�F */
								if (vram_active) {
									vram_aptr += 0x4000;
								}
								if (block_active) {
									vram_aptr += 0x18000;
								}
	}
#else
	if (vram_active) {
		vram_aptr += 0xc000;
	}
#endif

	/* �\���|�C���^�A���\���u���b�N�|�C���^ */
	vram_dptr = vram_c;
#if XM7_VER >= 3
	vram_dblk = vram_c;
	vram_bdblk = vram_c;
	if ((screen_mode == SCR_200LINE) && vram_display) {
		vram_dptr += 0x4000;
	}
	vram_bdptr = vram_dptr;
	if (block_display) {
		vram_dptr += 0x18000;
		vram_dblk += 0x18000;
	}
	else {
		vram_bdptr += 0x18000;
		vram_bdblk += 0x18000;
	}
#else
	if (!mode320 && vram_display) {
		vram_dptr += 0xc000;
	}
#endif	/* XM7_VER >= 3 */
#endif	/* XM7_VER >= 2 */

	/* �K�v�Ȃ�ĕ`�� */
	if (redraw) {
		display_notify();
	}
}

/*
 *	�n�[�h�E�F�A�E�C���h�E
 *	�I�[�v���`�F�b�N�E���������͈͌���
 */
#if XM7_VER >= 3
static void FASTCALL check_window_open(void)
{
	/* Xs<Xe and Ys<Ye �ł���΁A�E�B���h�E�I�[�v�� */
	if ((window_x1 < window_x2) && (window_y1 < window_y2)) {
		window_open = TRUE;
	}
	else {
		window_open = FALSE;
		window_dx1 = 0;
		window_dx2 = 0;
		window_dy1 = 0;
		window_dy2 = 0;
	}

	/* �ĕ`��w�� */
	window_notify();
}

/*
 *	�n�[�h�E�F�A�E�C���h�E
 *	�N���b�s���O
 */
void FASTCALL window_clip(int mode)
{
	static const WORD max_x[4] = { 640, 320, 640, 320 };
	static const WORD max_y[4] = { 200, 200, 400, 200 };

	ASSERT((mode >= 0) && (mode <= 2));

	/* X�N���b�s���O */
	window_dx1 = window_x1;
	window_dx2 = window_x2;
	if (window_dx1 > max_x[mode]) {
		window_dx1 = max_x[mode];
	}
	if (window_dx2 > max_x[mode]) {
		window_dx2 = max_x[mode];
	}

	/* Y�N���b�s���O */
	window_dy1 = window_y1;
	window_dy2 = window_y2;
	if (window_dy1 > max_y[mode]) {
		window_dy1 = max_y[mode];
	}
	if (window_dy2 > max_y[mode]) {
		window_dy2 = max_y[mode];
	}
}
#endif

/*
 *	4096�F/262144�F���[�h�p VRAM�X�N���[��
 */
#if XM7_VER >= 2
static void FASTCALL vram_scroll_analog(WORD offset, DWORD addr)
{
	int i;
	BYTE *vram;

#if XM7_VER >= 3
	for (i=0; i<3; i++) {
		vram = (BYTE *)((vram_c + addr) + 0x8000 * i);

		/* �e���|�����o�b�t�@�փR�s�[ */
		memcpy(vram_buf, vram, offset);
		memcpy(&vram_buf[0x2000], &vram[0x2000], offset);

		/* �O�֋l�߂� */
		memcpy(vram, (vram + offset), 0x4000 - offset);

		/* �e���|�����o�b�t�@��蕜�� */
		memcpy(vram + (0x2000 - offset), vram_buf, offset);
		memcpy(vram + (0x4000 - offset), &vram_buf[0x2000], offset);
	}
#else
	for (i=0; i<6; i++) {
		vram = (BYTE *)((vram_c + addr) + 0x2000 * i);

		/* �e���|�����o�b�t�@�փR�s�[ */
		memcpy(vram_buf, vram, offset);

		/* �O�֋l�߂� */
		memcpy(vram, (vram + offset), 0x2000 - offset);

		/* �e���|�����o�b�t�@��蕜�� */
		memcpy(vram + (0x2000 - offset), vram_buf, offset);
	}
#endif
}
#endif

/*
 *	400���C�����[�hVRAM�X�N���[���p�������]�� (C��)
 */
#if (XM7_VER >= 3) && (!(defined(_OMF) || defined(_WIN32)))
static void FASTCALL memcpy400l(BYTE *dest, BYTE *src, WORD siz)
{
	siz >>= 1;
	while (siz) {
		*dest = *src;
		src += 2;
		dest += 2;
		siz --;
	}
}
#endif

/*
 *	VRAM�X�N���[��
 */
static void FASTCALL vram_scroll(WORD offset)
{
	int i;
	BYTE *vram;

	if (offset == 0) {
		return;
	}

#if XM7_VER >= 2
#if XM7_VER >= 3
	/* 400���C�� */
	if (screen_mode == SCR_400LINE) {
		/* 400���C���� �I�t�Z�b�g�}�X�N */
		offset &= 0x3fff;
		offset <<= 1;

		/* ���[�v */
		for (i=0; i<3; i++) {
			vram = (BYTE *)(vram_c + 0x8000 * i + vram_active);

			/* �e���|�����o�b�t�@�փR�s�[ */
			memcpy400l(vram_buf, vram, offset);

			/* �O�֋l�߂� */
			memcpy400l(vram, (vram + offset), 0x8000 - offset);

			/* �e���|�����o�b�t�@��蕜�� */
			memcpy400l(vram + (0x8000 - offset), vram_buf, offset);
		}
		return;
	}

	/* 4096�F/262144�F */
	if (screen_mode & SCR_ANALOG) {
#else
	/* 4096�F */
	if (mode320) {
#endif
		/* 320�� �I�t�Z�b�g�}�X�N */
		offset &= 0x1fff;

		if (vram_active == 1) {
			/* �o���N1 */
#if XM7_VER >= 3
			vram_scroll_analog(offset, 0x04000);
#else
			vram_scroll_analog(offset, 0x0c000);
#endif
		}
		else {
			/* �o���N0 */
			vram_scroll_analog(offset, 0x00000);
#if XM7_VER >= 3
			if (screen_mode == SCR_262144) {
				/* 26���F���[�h���̓o���N2�������ɃX�N���[�� */
				vram_scroll_analog(offset, 0x18000);
			}
#endif
		}

		return;
	}
#endif	/* XM7_VER >= 2 */

#if XM7_VER == 1 && defined(L4CARD)
	if (enable_400line && enable_400linecard) {
		/* 400���C���P�F���̓����_�����ŃX�N���[�����s�� */
		/* (���C��$FD37�̋����Č��̊֌W) */
		return;
	}
#endif

	/* 8�F */
	offset &= 0x3fff;

	/* ���[�v */
	for (i=0; i<3; i++) {
#if XM7_VER >= 3
		vram = (BYTE *)(vram_c + 0x8000 * i);
		if (vram_active) {
			vram += 0x4000;
		}
#elif XM7_VER >= 2
		vram = (BYTE *)(vram_c + 0x4000 * i);
		if (vram_active) {
			vram += 0xc000;
		}
#else
		vram = (BYTE *)(vram_c + 0x4000 * i);
#endif

		/* �e���|�����o�b�t�@�փR�s�[ */
		memcpy(vram_buf, vram, offset);

		/* �O�֋l�߂� */
		memcpy(vram, (vram + offset), 0x4000 - offset);

		/* �e���|�����o�b�t�@��蕜�� */
		memcpy(vram + (0x4000 - offset), vram_buf, offset);
	}
}

/*
 *	400���C�����[�h�pVRAM�z�u�␳
 */
#if XM7_VER >= 3
BOOL FASTCALL fix_vram_address(void)
{
	DWORD	i;

	for (i=0; i<0x30000; i+=0x18000) {
		memcpy(&vram_buf[0x00000], &vram_c[0x04000 + i], 0x4000);
		memcpy(&vram_buf[0x04000], &vram_c[0x10000 + i], 0x4000);
		memcpy(&vram_c[0x04000 + i], &vram_c[0x08000 + i], 0x4000);
		memcpy(&vram_c[0x10000 + i], &vram_c[0x0c000 + i], 0x4000);
		memcpy(&vram_c[0x08000 + i], &vram_buf[0x04000], 0x4000);
		memcpy(&vram_c[0x0c000 + i], &vram_buf[0x00000], 0x4000);
	}

	return TRUE;
}
#endif

#if XM7_VER == 1 && defined(L4CARD)
/*
 *	�J�[�\�� �ʒu�E�u�����N���x�Đݒ�
 */
void FASTCALL cursor_setup(void)
{
	WORD cursor_addr_old;

	/* �J�[�\���ړ� */
	cursor_addr_old = cursor_addr;
	cursor_addr  = (WORD)(crtc_register[14] << 10);
	cursor_addr |= (WORD)(crtc_register[15] << 2);
	if (cursor_lsb) {
		cursor_addr += (WORD)2;
	}
	cursor_addr &= (WORD)0xFFF;

	/* ���J�[�\�����W�ƐV�J�[�\�����W�����ꂼ��ĕ`�� */
	tvram_notify(cursor_addr_old, 0);
	tvram_notify(cursor_addr, 0);
}

/*
 *	CRTC(HD6845) �P�o�C�g�ǂݍ���
 */
static BYTE FASTCALL crtc_readb(BYTE reg)
{
	if ((reg >= 0x0c) && (reg <= 0x11)) {
		/* 0x0c�`0x11�͋L�����Ă���l��Ԃ� */
		return crtc_register[reg];
	}
	else {
		/* 0x0c�`0x11�ȊO�̓��C�g�I�����[ */
		return 0xff;
	}
}

/*
 *	CRTC(HD6845) �P�o�C�g��������
 */
static void FASTCALL crtc_writeb(BYTE reg, BYTE dat)
{
	/* �������񂾒l��ۑ� */
	crtc_register[reg] = dat;

	switch (reg) {
		/* �J�[�\���T�C�Y */
		case 0x0a :
		case 0x0b :
			cursor_setup();
			break;

		/* �e�L�X�g�X�^�[�g�A�h���X(���) */
		case 0x0c :
			text_start_addr &= 0x03FC;
			text_start_addr |= (WORD)((dat & 0x03) << 10);
			text_scroll_count ++;
			if ((text_scroll_count & 1) == 0) {
				tvram_redraw_notify();
			}
			break;

		/* �e�L�X�g�X�^�[�g�A�h���X(����) */
		case 0x0d :
			text_start_addr &= 0xFC00;
			text_start_addr |= (WORD)(dat << 2);
			text_scroll_count ++;
			if ((text_scroll_count & 1) == 0) {
				tvram_redraw_notify();
			}
			break;

		/* �J�[�\���A�h���X */
		case 0x0e :
		case 0x0f :
			text_cursor_count ++;
			if ((text_cursor_count & 1) == 0) {
				cursor_setup();
			}
			break;
	}
}
#endif

/*
 *	�f�B�X�v���C
 *	�P�o�C�g�ǂݍ���
 *	�����C���|�T�u�C���^�t�F�[�X�M�������܂�
 */
BOOL FASTCALL display_readb(WORD addr, BYTE *dat)
{
	BYTE ret;
#if XM7_VER >= 3 || (XM7_VER == 1 && defined(L4CARD))
	int offset;
#endif	/* XM7_VER >= 3 */

	switch (addr) {
		/* �L�����Z��IRQ ACK */
		case 0xd402: 
			subcancel_flag = FALSE;
			subcancel_request = FALSE;
			subcpu_irq();
			*dat = 0xff;
			return TRUE;

		/* BEEP */
		case 0xd403:
			beep_flag = TRUE;
			schedule_setevent(EVENT_BEEP, 205000, mainetc_beep);
			*dat = 0xff;

			/* �ʒm */
			beep_notify();
			return TRUE;

		/* �A�e���V����IRQ ON */
		case 0xd404:
			subattn_flag = TRUE;
			*dat = 0xff;
			maincpu_firq();
			return TRUE;

#if XM7_VER == 1 && defined(L4CARD)
		/* �T�u���[�h���W�X�^ */
		case 0xd405:
			/* FM-77���[�h�̂ݗL�� */
			if (fm_subtype != FMSUB_FM77) {
				return FALSE;
			}

			ret = 0xff;

			/* bit7�`1��400���C�����̂ݗL�� */
			if (enable_400linecard) {
				/* �u�����L���O */
				if (blank_flag) {
					ret &= (BYTE)~0x80;
				}

				/* VSYNC */
				if (!vsync_flag) {
					ret &= (BYTE)~0x40;
				}

				/* ���C���� */
				if (select_400line) {
					ret &= ~0x20;
				}

				/* CURSOR LSB */
				if (!cursor_lsb) {
					ret &= (BYTE)~0x10;
				}

				/* WIDTH */
				if (width40_flag) {
					ret &= (BYTE)~0x08;
				}

				/* ���[�NRAM�I�� */
				if (!workram_select) {
					ret &= (BYTE)~0x04;
				}

				/* 400���C�����[�h */
				if (enable_400line) {
					ret &= ~0x02;
				}
			}

			/* �T�C�N���X�`�[�� */
			if (cycle_steal) {
				ret &= ~0x01;
			}

			*dat = ret;
			return TRUE;
#endif

#if XM7_VER >= 3 || (XM7_VER == 1 && defined(L4CARD))
		/* �T�u����ROM */
		case 0xd406:		/* �T�u����LEFT */
		case 0xd407:		/* �T�u����RIGHT */
#if XM7_VER >= 3
			if ((fm7_ver >= 3) && subkanji_flag) {
				/* �A�h���X�̓��[�h�P�ʂŁA8bit�̂ݎ擾 */
				offset = sub_kanji_addr << 1;
				if (sub_kanji_bank) {
					*dat = kanji_rom2[offset + (addr & 1)];
				}
				else {
					*dat = kanji_rom[offset + (addr & 1)];
				}
			}
#else
			/* FM-77���[�h�̂ݗL�� */
			if (fm_subtype != FMSUB_FM77) {
				return FALSE;
			}

			if (subkanji_flag) {
				/* �A�h���X�̓��[�h�P�ʂŁA8bit�̂ݎ擾 */
				offset = sub_kanji_addr << 1;
				if ((offset >= 0x6000) && (offset < 0x8000) &&
					 !kanji_asis_flag) {
					/* FM-7���[�h����$6000�`$7FFF�͖���`�̈� */
					*dat = (BYTE)(addr & 1);
				}
				else {
					/* �ʏ�̈� */
					*dat = kanji_rom[offset + (addr & 1)];
				}
			}
#endif
			else {
				*dat = 0xff;
			}
			return TRUE;
#endif

		/* CRT ON */
		case 0xd408:
			if (!crt_flag) {
				crt_flag = TRUE;
				/* CRT OFF��ON */
				display_notify();
			}
			*dat = 0xff;
			return TRUE;

		/* VRAM�A�N�Z�X ON */
		case 0xd409:
			vrama_flag = TRUE;
			*dat = 0xff;
			return TRUE;

		/* BUSY�t���O OFF */
		case 0xd40a:
			subbusy_flag = FALSE;
			*dat = 0xff;
			return TRUE;

#if XM7_VER == 1 && defined(L4CARD)
		/* CRTC�A�h���X���W�X�^(400LINE CARD) */
		case 0xd40b:
			/* FM-77���[�h�̂ݗL�� */
			if (fm_subtype != FMSUB_FM77) {
				return FALSE;
			}

			if (enable_400linecard) {
				*dat = (BYTE)(crtc_regnum & 0x1f);
			}
			else {
				*dat = 0xff;
			}
			return TRUE;

		/* CRTC�f�[�^���W�X�^(400LINE CARD) */
		case 0xd40c:
			/* FM-77���[�h�̂ݗL�� */
			if (fm_subtype != FMSUB_FM77) {
				return FALSE;
			}

			if (enable_400linecard) {
				*dat = (BYTE)crtc_readb(crtc_regnum);
			}
			else {
				*dat = 0xff;
			}
			return TRUE;
#endif

#if XM7_VER >= 3
		/* FM77AV40 �T�uRAM�o���N�Z���N�g/�T�u����ROM�Z���N�g */
		case 0xd42e:
			/* fm7_ver�Ɋւ�炸�A�ǂݏo���o���Ȃ� */
			*dat = 0xff;
			return TRUE;

		/* FM77AV40 400���C��/26���F�pVRAM�o���N�Z���N�g */
		case 0xd42f:
			if (fm7_ver >= 3) {
				*dat = (BYTE)(0xfc | (subram_vrambank & 3));
			}
			else {
				*dat = 0xff;
			}
			return TRUE;
#endif

#if XM7_VER >= 2
		/* FM77AV MISC���W�X�^ */
		case 0xd430:
			if (fm7_ver >= 2) {
				ret = 0xff;

				/* �u�����L���O */
				if (blank_flag) {
					ret &= (BYTE)~0x80;
				}

				/* ������� */
				if (line_busy) {
					ret &= (BYTE)~0x10;
					/* LINE BOOST���͈�񂾂�BUSY�������� */
					if (line_boost) {
						line_busy = FALSE;
						schedule_delevent(EVENT_LINE);
					}
				}

				/* VSYNC */
				if (!vsync_flag) {
					ret &= (BYTE)~0x04;
				}

				/* �T�uRESET�X�e�[�^�X */
				if (!subreset_flag) {
					ret &= (BYTE)~0x01;
				}

				*dat = ret;
				return TRUE;
			}

			return FALSE;
#endif
	}

	return FALSE;
}

/*
 *	�f�B�X�v���C
 *	�P�o�C�g��������
 *	�����C���|�T�u�C���^�t�F�[�X�M�������܂�
 */
BOOL FASTCALL display_writeb(WORD addr, BYTE dat)
{
	WORD offset;
#if XM7_VER >= 2
	BOOL redraw_flag;
#endif

	switch (addr) {
#if XM7_VER == 1
		/* �T�u���[�h���W�X�^ */
		case 0xd405:
			/* FM-77���[�h�̂ݗL�� */
			if (fm_subtype != FMSUB_FM77) {
				return FALSE;
			}

			/* �T�C�N���X�`�[�� */
			if (dat & 0x01) {
				cycle_steal = FALSE;
			}
			else {
				cycle_steal = TRUE;
			}

#if defined(L4CARD)
			/* bit4�`1��400���C���J�[�h�L�����̂ݗL�� */
			if (enable_400linecard) {
				/* CURSOR LSB */
				if (dat & 0x10) {
					cursor_lsb = TRUE;
				}
				else {
					cursor_lsb = FALSE;
				}

				/* WIDTH */
				if (dat & 0x08) {
					width40_flag = FALSE;
				}
				else {
					width40_flag = TRUE;
				}

				/* ���[�NRAM�I�� */
				if (dat & 0x04) {
					workram_select = TRUE;
				}
				else {
					workram_select = FALSE;
				}

				/* 400���C���J�[�h ���[�h�؂芷�� */
				if (dat & 0x02) {
					if (!enable_400line) {
						return TRUE;
					}
					enable_400line = FALSE;
				}
				else {
					if (enable_400line) {
						return TRUE;
					}
					enable_400line = TRUE;
				}

				/* �t���O�ރZ�b�g */
				subreset_flag = TRUE;
				subbusy_flag = TRUE;

				/* CRT���W�X�^�����Z�b�g���� */
				display_reset();
				display_notify();

				/* INS LED������������ */
				ins_flag = FALSE;

				/* �T�uCPU�����Z�b�g */
				subcpu_reset();
			}
#endif

			return TRUE;
#endif

#if XM7_VER >= 3 || (XM7_VER == 1 && defined(L4CARD))
		/* �T�u����ROM �A�h���X��� */
		case 0xd406:
#if XM7_VER >= 3
			if (fm7_ver >= 3) {
				sub_kanji_addr &= (WORD)0x00ff;
				sub_kanji_addr |= (WORD)(dat << 8);
			}
#else
			/* FM-77���[�h�̂ݗL�� */
			if (fm_subtype != FMSUB_FM77) {
				return FALSE;
			}

			sub_kanji_addr &= (WORD)0x00ff;
			sub_kanji_addr |= (WORD)(dat << 8);
#endif
			return TRUE;

		/* �T�u����ROM �A�h���X���� */
		case 0xd407:
#if XM7_VER >= 3
			if (fm7_ver >= 3) {
				sub_kanji_addr &= (WORD)0xff00;
				sub_kanji_addr |= (WORD)dat;
			}
#else
			/* FM-77���[�h�̂ݗL�� */
			if (fm_subtype != FMSUB_FM77) {
				return FALSE;
			}

			sub_kanji_addr &= (WORD)0xff00;
			sub_kanji_addr |= (WORD)dat;
#endif
			return TRUE;
#endif

		/* CRT OFF */
		case 0xd408:
			if (crt_flag) {
				/* CRT ON��OFF */
				crt_flag = FALSE;
				display_notify();
			}
			crt_flag = FALSE;
			return TRUE;

		/* VRAM�A�N�Z�X OFF */
		case 0xd409:
			vrama_flag = FALSE;
			return TRUE;

		/* BUSY�t���O ON */
		case 0xd40a:
			if (magus_patch) {
				/* CLR���߃`�F�b�N */
				if ((fetch_op == 0x0f) ||
					(fetch_op == 0x6f) ||
					(fetch_op == 0x7f)) {
					if (fetch_op == 0x0f) {
						busy_CLR_count = 1;
					}
					else {
						busy_CLR_count = 2;
					}
					subbusy_flag = FALSE;
				}
				else {
					subbusy_flag = TRUE;
				}
			}
			else {
				subbusy_flag = TRUE;
			}
			return TRUE;

#if XM7_VER == 1 && defined(L4CARD)
		/* CRTC�A�h���X���W�X�^(400LINE CARD) */
		case 0xd40b:
			/* FM-77���[�h�̂ݗL�� */
			if (fm_subtype != FMSUB_FM77) {
				return FALSE;
			}

			if (enable_400linecard) {
				crtc_regnum = (BYTE)(dat & 0x1f);
			}
			return TRUE;

		/* CRTC�f�[�^���W�X�^(400LINE CARD) */
		case 0xd40c:
			/* FM-77���[�h�̂ݗL�� */
			if (fm_subtype != FMSUB_FM77) {
				return FALSE;
			}

			if (enable_400linecard) {
				crtc_writeb(crtc_regnum, dat);
			}
			return TRUE;
#endif

		/* VRAM�I�t�Z�b�g�A�h���X */
		case 0xd40e:
		case 0xd40f:
			if (addr == 0xd40e) {
				/* VRAM�I�t�Z�b�g�A�h���X ��� */
#if XM7_VER == 1 && defined(L4CARD)
				/* ��ʃ��[�h�ɂ���ėL���r�b�g�����ω����� */
				if (enable_400line && enable_400linecard) {
					offset = (WORD)(dat & 0x7f);
				}
				else {
					offset = (WORD)(dat & 0x3f);
				}
#else
				offset = (WORD)(dat & 0x3f);
#endif
				offset <<= 8;
#if XM7_VER >= 2
				offset |= (WORD)(vram_offset[vram_active] & 0xff);
#else
				offset |= (WORD)(vram_offset[0] & 0xff);
#endif
			}
			else {
				/* VRAM�I�t�Z�b�g�A�h���X ���� */
#if XM7_VER >= 2
				/* �g���I�t�Z�b�g�t���O��OFF�Ȃ�A����5bit�͖��� */
				if (!vram_offset_flag) {
					dat &= 0xe0;
				}
				offset = (WORD)(vram_offset[vram_active] & 0x3f00);
				offset |= (WORD)dat;
#else
#if XM7_VER == 1 && defined(L4CARD)
				/* ��ʃ��[�h�ɂ���ėL���r�b�g�����ω����� */
				if (enable_400line && enable_400linecard) {
					offset = (WORD)(vram_offset[0] & 0x7f00);
					offset |= (WORD)(dat & 0xfe);
				}
				else {
					offset = (WORD)(vram_offset[0] & 0x3f00);
					offset |= (WORD)(dat & 0xe0);
				}
#else
				offset = (WORD)(vram_offset[0] & 0x3f00);
				offset |= (WORD)(dat & 0xe0);
#endif
#endif
			}

#if XM7_VER >= 2
			vram_offset[vram_active] = offset;
			/* �J�E���g�A�b�v�A�X�N���[�� */
			vram_offset_count[vram_active]++;
			if ((vram_offset_count[vram_active] & 1) == 0) {
				vram_scroll((WORD)(vram_offset[vram_active] -
									crtc_offset[vram_active]));
				crtc_offset[vram_active] = vram_offset[vram_active];
				display_notify();
			}
#else
			vram_offset[0] = offset;
			/* �J�E���g�A�b�v�A�X�N���[�� */
			vram_offset_count[0]++;
			if ((vram_offset_count[0] & 1) == 0) {
				vram_scroll((WORD)(vram_offset[0] - crtc_offset[0]));
				crtc_offset[0] = vram_offset[0];
				display_notify();
			}
#endif
			return TRUE;

#if XM7_VER >= 3
		/* FM77AV40 �T�uRAM�o���N�Z���N�g/�T�u����ROM�Z���N�g */
		case 0xd42e:
			if (fm7_ver >= 3) {
				/* bit0-2:CGRAM�o���N�Z���N�g */
				cgram_bank = (BYTE)(dat & 0x07);

				/* bit3,4:�R���\�[��RAM�o���N�Z���N�g */
				consram_bank = (BYTE)((dat >> 3) & 0x03);
				if (consram_bank == 3) {
					/* �o���N3�͑��݂��Ȃ� */
					ASSERT(FALSE);
					consram_bank = 0;
				}

				/* bit7:��1�����E��2�����Z���N�g */
				if (dat & 0x80) {
					/* ��2���� */
					sub_kanji_bank = TRUE;
				}
				else {
					/* ��1���� */
					sub_kanji_bank = FALSE;
				}
			}
			return TRUE;

		/* FM77AV40 400���C��/26���F�pVRAM�o���N�Z���N�g */
		case 0xd42f:
			if (fm7_ver >= 3) {
				subram_vrambank = (BYTE)(dat & 0x03);
				if (subram_vrambank == 3) {
					/* �o���N3�͑��݂��Ȃ� */
					ASSERT(FALSE);
					subram_vrambank = 0;
				}

				/* �|�C���^���č\�� */
				display_setpointer(FALSE);
			}
			return TRUE;
#endif

#if XM7_VER >= 2
		/* FM77AV MISC���W�X�^ */
		case 0xd430:
			if (fm7_ver >= 2) {
				redraw_flag = FALSE;

				/* NMI�}�X�N */
				if (dat & 0x80) {
					subnmi_flag = FALSE;
					event[EVENT_SUBTIMER].flag = EVENT_DISABLED;
					subcpu.intr &= ~INTR_NMI;
				}
				else {
					subnmi_flag = TRUE;
					event[EVENT_SUBTIMER].flag = EVENT_ENABLED;
				}

				/* �A�N�e�B�u�y�[�W */
				if (dat & 0x20) {
					vram_active = 1;
				}
				else {
					vram_active = 0;
				}

				/* �f�B�X�v���C�y�[�W */
				if (dat & 0x40) {
					if (vram_display == 0) {
						vram_display = 1;

						/* 200���C��8�F���[�h�ł͉�ʍĕ`�悪�K�v */
#if XM7_VER >= 3
						if (screen_mode == SCR_200LINE) {
#else
						if (!mode320) {
#endif
							redraw_flag = TRUE;
						}
					}
				}
				else {
					if (vram_display == 1) {
						vram_display = 0;

						/* 200���C��8�F���[�h�ł͉�ʍĕ`�悪�K�v */
#if XM7_VER >= 3
						if (screen_mode == SCR_200LINE) {
#else
						if (!mode320) {
#endif
							redraw_flag = TRUE;
						}
					}
				}

				/* �g��VRAM�I�t�Z�b�g���W�X�^ */
				if (dat & 0x04) {
					vram_offset_flag = TRUE;
				}
				else {
					vram_offset_flag = FALSE;
				}

				/* CGROM�o���N */
				cgrom_bank = (BYTE)(dat & 0x03);

				/* �|�C���^�č\���E��ʍĕ`�� */
				display_setpointer(redraw_flag);
			}

			return TRUE;
#endif

#if XM7_VER >= 3
		/* FM77AV40EX VRAM�u���b�N�Z���N�g */
		case 0xd433:
			if (fm7_ver >= 3) {
				redraw_flag = FALSE;

				/* bit0:�A�N�e�B�u�u���b�N�Z���N�g */
				if (dat & 0x01) {
					block_active = 1;
				}
				else {
					block_active = 0;
				}

				/* bit4:�\���u���b�N�Z���N�g */
				if (dat & 0x10) {
					if (block_display == 0) {
						block_display = 1;
						if (screen_mode != SCR_262144) {
							redraw_flag = TRUE;
						}
					}
				}
				else {
					if (block_display == 1) {
						block_display = 0;
						if (screen_mode != SCR_262144) {
							redraw_flag = TRUE;
						}
					}
				}

				/* �|�C���^�č\���E��ʍĕ`�� */
				display_setpointer(redraw_flag);
			}
			return TRUE;

		/* FM77AV40EX �n�[�h�E�F�A�E�B���h�E */
		case 0xd438:			/* X�E�B���h�E�X�^�[�g�A�h���X(���) */
		case 0xd439:			/* X�E�B���h�E�X�^�[�g�A�h���X(����) */
		case 0xd43a:			/* X�E�B���h�E�G���h�A�h���X(���) */
		case 0xd43b:			/* X�E�B���h�E�G���h�A�h���X(����) */
		case 0xd43c:			/* Y�E�B���h�E�X�^�[�g�A�h���X(���) */
		case 0xd43d:			/* Y�E�B���h�E�X�^�[�g�A�h���X(����) */
		case 0xd43e:			/* Y�E�B���h�E�G���h�A�h���X(���) */
		case 0xd43f:			/* Y�E�B���h�E�G���h�A�h���X(����) */
			if (fm7_ver <= 2) {
				return TRUE;
			}

			switch (addr & 7) {
				case 0:	/* X�E�B���h�E�X�^�[�g�A�h���X(���) */
						window_x1 &= (WORD)0x00f8;
						window_x1 |= (WORD)((dat & 0x03) << 8);
						break;
				case 1:	/* X�E�B���h�E�X�^�[�g�A�h���X(����) */
						window_x1 &= (WORD)0x0300;
						window_x1 |= (WORD)(dat & 0xf8);
						break;
				case 2:	/* X�E�B���h�E�G���h�A�h���X(���) */
						window_x2 &= (WORD)0x00f8;
						window_x2 |= (WORD)((dat & 0x03) << 8);
						break;
				case 3:	/* X�E�B���h�E�G���h�A�h���X(����) */
						window_x2 &= (WORD)0x0300;
						window_x2 |= (WORD)(dat & 0xf8);
						break;
				case 4:	/* Y�E�B���h�E�X�^�[�g�A�h���X(���) */
						window_y1 &= (WORD)0x00ff;
						window_y1 |= (WORD)((dat & 0x01) << 8);
						break;
				case 5:	/* Y�E�B���h�E�X�^�[�g�A�h���X(����) */
						window_y1 &= (WORD)0x0100;
						window_y1 |= (WORD)dat;
						break;
				case 6:	/* Y�E�B���h�E�G���h�A�h���X(���) */
						window_y2 &= (WORD)0x00ff;
						window_y2 |= (WORD)((dat & 0x01) << 8);
						break;
				case 7:	/* Y�E�B���h�E�G���h�A�h���X(����) */
						window_y2 &= (WORD)0x0100;
						window_y2 |= (WORD)dat;
						break;
				default:
						ASSERT(FALSE);
			}

			check_window_open();
			return TRUE;
#endif
	}

	return FALSE;
}

/*
 *	�f�B�X�v���C
 *	�Z�[�u
 */
BOOL FASTCALL display_save(int fileh)
{
	if (!file_bool_write(fileh, crt_flag)) {
		return FALSE;
	}
	if (!file_bool_write(fileh, vrama_flag)) {
		return FALSE;
	}

#if XM7_VER >= 2
	if (!file_bool_write(fileh, subnmi_flag)) {
		return FALSE;
	}
#endif
	if (!file_bool_write(fileh, vsync_flag)) {
		return FALSE;
	}
	if (!file_bool_write(fileh, blank_flag)) {
		return FALSE;
	}

	if (!file_bool_write(fileh, vram_offset_flag)) {
		return FALSE;
	}
#if XM7_VER >= 2
	if (!file_word_write(fileh, vram_offset[0])) {
		return FALSE;
	}
	if (!file_word_write(fileh, vram_offset[1])) {
		return FALSE;
	}
	if (!file_word_write(fileh, crtc_offset[0])) {
		return FALSE;
	}
	if (!file_word_write(fileh, crtc_offset[1])) {
		return FALSE;
	}

	if (!file_byte_write(fileh, vram_active)) {
		return FALSE;
	}
	if (!file_byte_write(fileh, vram_display)) {
		return FALSE;
	}
#else
	if (!file_word_write(fileh, vram_offset[0])) {
		return FALSE;
	}
	if (!file_word_write(fileh, crtc_offset[0])) {
		return FALSE;
	}
#endif

	/* Ver6�g�� */
	if (!file_word_write(fileh, blank_count)) {
		return FALSE;
	}

#if XM7_VER >= 3
	/* Ver8�g�� */
	if (!file_byte_write(fileh, subram_vrambank)) {
		return FALSE;
	}
	if (!file_word_write(fileh, sub_kanji_addr)) {
		return FALSE;
	}
	if (!file_bool_write(fileh, sub_kanji_bank)) {
		return FALSE;
	}

	if (!file_word_write(fileh, window_x1)) {
		return FALSE;
	}
	if (!file_word_write(fileh, window_dx1)) {
		return FALSE;
	}
	if (!file_word_write(fileh, window_y1)) {
		return FALSE;
	}
	if (!file_word_write(fileh, window_dy1)) {
		return FALSE;
	}
	if (!file_word_write(fileh, window_x2)) {
		return FALSE;
	}
	if (!file_word_write(fileh, window_dx2)) {
		return FALSE;
	}
	if (!file_word_write(fileh, window_y2)) {
		return FALSE;
	}
	if (!file_word_write(fileh, window_dy2)) {
		return FALSE;
	}
	if (!file_bool_write(fileh, window_open)) {
		return FALSE;
	}

	if (!file_byte_write(fileh, block_active)) {
		return FALSE;
	}
	if (!file_byte_write(fileh, block_display)) {
		return FALSE;
	}
#endif

#if XM7_VER == 1 && defined(L4CARD)
	/* XM7 V1.1 / FM-77L4�e�L�X�gVRAM�܂�� */
	if (!file_bool_write(fileh, width40_flag)) {
		return FALSE;
	}
	if (!file_bool_write(fileh, cursor_lsb)) {
		return FALSE;
	}
	if (!file_bool_write(fileh, text_blink)) {
		return FALSE;
	}
	if (!file_bool_write(fileh, cursor_blink)) {
		return FALSE;
	}
	if (!file_word_write(fileh, text_start_addr)) {
		return FALSE;
	}
	if (!file_word_write(fileh, cursor_addr)) {
		return FALSE;
	}

	if (!file_bool_write(fileh, enable_400line)) {
		return FALSE;
	}
	if (!file_bool_write(fileh, workram_select)) {
		return FALSE;
	}
	if (!file_word_write(fileh, sub_kanji_addr)) {
		return FALSE;
	}

	if (!file_byte_write(fileh, crtc_regnum)) {
		return FALSE;
	}
	if (!file_write(fileh, crtc_register, 0x20)) {
		return FALSE;
	}
#endif

	/* Ver9.18/Ver7.18/Ver3.08�g�� */
	if (!file_dword_write(fileh, now_raster)) {
		return FALSE;
	}

	/* Ver3.10�g�� */
#if XM7_VER == 1 && defined(L4CARD)
	if (!file_byte_write(fileh, 0xff)) {
		return FALSE;
	}
#endif

	return TRUE;
}

/*
 *	�f�B�X�v���C
 *	���[�h
 */
BOOL FASTCALL display_load(int fileh, int ver)
{
#if XM7_VER == 1
#if defined(L4CARD)
	BYTE tmp;
#endif
#endif

	/* �o�[�W�����`�F�b�N */
	if (ver < 200) {
		return FALSE;
	}

	if (!file_bool_read(fileh, &crt_flag)) {
		return FALSE;
	}
	if (!file_bool_read(fileh, &vrama_flag)) {
		return FALSE;
	}

#if XM7_VER >= 2
	if (!file_bool_read(fileh, &subnmi_flag)) {
		return FALSE;
	}
#endif
	if (!file_bool_read(fileh, &vsync_flag)) {
		return FALSE;
	}
	if (!file_bool_read(fileh, &blank_flag)) {
		return FALSE;
	}

	if (!file_bool_read(fileh, &vram_offset_flag)) {
		return FALSE;
	}
#if XM7_VER >= 2
	if (!file_word_read(fileh, &vram_offset[0])) {
		return FALSE;
	}
	if (!file_word_read(fileh, &vram_offset[1])) {
		return FALSE;
	}
	if (!file_word_read(fileh, &crtc_offset[0])) {
		return FALSE;
	}
	if (!file_word_read(fileh, &crtc_offset[1])) {
		return FALSE;
	}

	if (!file_byte_read(fileh, &vram_active)) {
		return FALSE;
	}
	if (!file_byte_read(fileh, &vram_display)) {
		return FALSE;
	}

	/* �C�x���g */
	schedule_handle(EVENT_SUBTIMER, subcpu_event);
	schedule_handle(EVENT_VSYNC, display_vsync);
	schedule_handle(EVENT_BLANK, display_blank);
#else
	if (!file_word_read(fileh, &vram_offset[0])) {
		return FALSE;
	}
	if (!file_word_read(fileh, &crtc_offset[0])) {
		return FALSE;
	}

	/* �C�x���g */
	schedule_handle(EVENT_SUBTIMER, subcpu_event);
	schedule_handle(EVENT_VSYNC, display_vsync);
#endif

	/* Ver6�g�� */
#if XM7_VER >= 2
	if (ver >= 600) {
#else
	if (ver >= 300) {
#endif
		if (!file_word_read(fileh, &blank_count)) {
			return FALSE;
		}
	}

#if XM7_VER >= 3
	/* Ver8�g�� */
	if (ver >= 800) {
		if (!file_byte_read(fileh, &subram_vrambank)) {
			return FALSE;
		}
		if (!file_word_read(fileh, &sub_kanji_addr)) {
			return FALSE;
		}
		if (!file_bool_read(fileh, &sub_kanji_bank)) {
			return FALSE;
		}

		if (!file_word_read(fileh, &window_x1)) {
			return FALSE;
		}
		if (!file_word_read(fileh, &window_dx1)) {
			return FALSE;
		}
		if (!file_word_read(fileh, &window_y1)) {
			return FALSE;
		}
		if (!file_word_read(fileh, &window_dy1)) {
			return FALSE;
		}
		if (!file_word_read(fileh, &window_x2)) {
			return FALSE;
		}
		if (!file_word_read(fileh, &window_dx2)) {
			return FALSE;
		}
		if (!file_word_read(fileh, &window_y2)) {
			return FALSE;
		}
		if (!file_word_read(fileh, &window_dy2)) {
			return FALSE;
		}
		if (!file_bool_read(fileh, &window_open)) {
			return FALSE;
		}

		if (!file_byte_read(fileh, &block_active)) {
			return FALSE;
		}
		if (!file_byte_read(fileh, &block_display)) {
			return FALSE;
		}
	}
	else {
		subram_vrambank = 0;
		sub_kanji_addr = 0;
		sub_kanji_bank = FALSE;

		window_x1 = 0;
		window_dx1 = 0;
		window_x2 = 0;
		window_dx2 = 0;
		window_y1 = 0;
		window_dy1 = 0;
		window_y2 = 0;
		window_dy2 = 0;
		window_open = FALSE;

		block_active = 0;
		block_display = 0;
	}
#endif

#if XM7_VER == 1 && defined(L4CARD)
	/* XM7 V1.1 / FM-77L4�e�L�X�gVRAM�܂�� */
	if (!file_bool_read(fileh, &width40_flag)) {
		return FALSE;
	}
	if (!file_bool_read(fileh, &cursor_lsb)) {
		return FALSE;
	}
	if (!file_bool_read(fileh, &text_blink)) {
		return FALSE;
	}
	if (!file_bool_read(fileh, &cursor_blink)) {
		return FALSE;
	}
	if (!file_word_read(fileh, &text_start_addr)) {
		return FALSE;
	}
	if (!file_word_read(fileh, &cursor_addr)) {
		return FALSE;
	}

	if (!file_bool_read(fileh, &enable_400line)) {
		return FALSE;
	}
	/* 400���C���J�[�h�������`�F�b�N */
	if (enable_400line && !detect_400linecard) {
		return FALSE;
	}

	if (!file_bool_read(fileh, &workram_select)) {
		return FALSE;
	}
	if (!file_word_read(fileh, &sub_kanji_addr)) {
		return FALSE;
	}

	if (!file_byte_read(fileh, &crtc_regnum)) {
		return FALSE;
	}
	if (!file_read(fileh, crtc_register, 0x20)) {
		return FALSE;
	}

	/* �C�x���g */
	schedule_handle(EVENT_TEXT_BLINK, display_text_blink);
#endif

	/* Ver9.18/Ver7.18/Ver3.08�g�� */
#if XM7_VER >= 3
	if ((ver >= 918) || ((ver >= 718) && (ver <= 799))) {
#elif XM7_VER >= 2
	if ((ver >= 718) && (ver <= 799)) {
#else
	if (ver >= 308) {
#endif
		if (!file_dword_read(fileh, (DWORD *)&now_raster)) {
			return FALSE;
		}
	}
	else {
		now_raster = 0;
	}

	/* Ver3.10�g�� */
#if XM7_VER == 1
#if defined(L4CARD)
	if (ver >= 310) {
		if (!file_byte_read(fileh, &tmp)) {
			return FALSE;
		}
	}
#endif
#endif

	/* �|�C���^���\�� */
	display_setpointer(TRUE);
	display_setup();

	return TRUE;
}
