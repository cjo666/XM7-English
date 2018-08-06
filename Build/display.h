/*
 *	FM-7 EMULATOR "XM7"
 *
 *	Copyright (C) 1999-2017 �o�h�D(Twitter:@xm6_original)
 *	Copyright (C) 2001-2017 Ryu Takegami (Twitter:@RyuTakegami)
 *
 *	[ �f�B�X�v���C ]
 */

#ifndef _display_h_
#define _display_h_

/*
 *	��ʃ��[�h��`
 */
#define	SCR_200LINE	0x00
#define	SCR_4096	0x01
#define	SCR_400LINE	0x02
#define	SCR_262144	0x03
#define	SCR_ANALOG	0x01
#define	SCR_AV40	0x02

#ifdef __cplusplus
extern "C" {
#endif
/*
 *	��v�G���g��
 */
BOOL FASTCALL display_init(void);
										/* ������ */
void FASTCALL display_cleanup(void);
										/* �N���[���A�b�v */
void FASTCALL display_reset(void);
										/* ���Z�b�g */
BOOL FASTCALL display_readb(WORD addr, BYTE *dat);
										/* �������ǂݏo�� */
BOOL FASTCALL display_writeb(WORD addr, BYTE dat);
										/* �������������� */
BOOL FASTCALL display_save(int fileh);
										/* �Z�[�u */
BOOL FASTCALL display_load(int fileh, int ver);
										/* ���[�h */

void FASTCALL display_setpointer(BOOL redraw);
										/* �֘A���[�N�̍Đݒ� */
#if XM7_VER >= 3
void FASTCALL window_clip(int mode);
										/* �E�C���h�E�N���b�s���O */
BOOL FASTCALL fix_vram_address(void);
										/* 400���C�����[�h�pVRAM�z�u�␳ */
#endif

/*
 *	��v���[�N
 */
extern BOOL crt_flag;
										/* CRT�\���t���O */
extern BOOL vrama_flag;
										/* VRAM�A�N�Z�X�t���O */
extern WORD vram_offset[2];
										/* VRAM�I�t�Z�b�g���W�X�^ */
extern BOOL vram_offset_flag;
										/* �g��VRAM�I�t�Z�b�g�t���O */
extern BOOL vsync_flag;
										/* VSYNC�t���O */
extern BOOL blank_flag;
										/* �u�����L���O�t���O */
extern BOOL draw_aftervsync;
										/* ��ʕ`��ʒm�^�C�~���O */
extern int now_raster;
										/* ���݃��X�^�ʒu */
extern BOOL magus_patch;
										/* MAGUS�p�b�` */

#if XM7_VER >= 2
extern BOOL subnmi_flag;
										/* �T�uNMI�C�l�[�u���t���O */
extern BYTE vram_active;
										/* �A�N�e�B�u�y�[�W */
extern BYTE *vram_aptr;
										/* VRAM�A�N�e�B�u�|�C���^ */
extern BYTE vram_display;
										/* �\���y�[�W */
extern BYTE *vram_dptr;
										/* VRAM�\���|�C���^ */

#if XM7_VER >= 3
/* FM77AV40 */
extern BYTE screen_mode;
										/* ��ʃ��[�h */
extern BYTE subram_vrambank;
										/* �A�N�e�B�u�y�[�W(400line/26���F) */

/* FM77AV40EX */
extern WORD window_x1, window_dx1;
										/* �n�[�h�E�F�A�E�B���h�E X1 */
extern WORD window_y1, window_dy1;
										/* �n�[�h�E�F�A�E�B���h�E Y1 */
extern WORD window_x2, window_dx2;
										/* �n�[�h�E�F�A�E�B���h�E X2 */
extern WORD window_y2, window_dy2;
										/* �n�[�h�E�F�A�E�B���h�E Y2 */
extern BOOL window_open;
										/* �E�B���h�E�I�[�v���t���O */
extern BYTE block_active;
										/* �A�N�e�B�u�u���b�N */
extern BYTE block_display;
										/* �\���u���b�N */
extern BYTE *vram_ablk;
										/* �A�N�e�B�u�u���b�N�|�C���^ */
extern BYTE *vram_bdptr;
										/* ���\���u���b�N�|�C���^ */
extern BYTE *vram_dblk;
										/* �\���u���b�N�|�C���^ */
extern BYTE *vram_bdblk;
										/* ���\���u���b�N�|�C���^2 */
#endif
#endif

#if XM7_VER == 1 && defined(L4CARD)
extern BOOL width40_flag;
										/* WIDTH40���[�h�t���O */
extern BOOL cursor_lsb;
										/* �J�[�\���A�h���XLSB */
extern BOOL enable_400line;
										/* 400���C�����[�h�t���O */
extern BOOL workram_select;
										/* ���[�NRAM�I���t���O */
extern BYTE crtc_register[0x20];
										/* CRTC���W�X�^ */
extern BYTE crtc_regnum;
										/* CRTC���W�X�^�ԍ����W�X�^ */
extern WORD text_start_addr;
										/* �e�L�X�g�X�^�[�g�A�h���X */
extern BOOL text_blink;
										/* �e�L�X�g�u�����N��� */
extern WORD cursor_addr;
										/* �J�[�\���A�h���X */
extern BOOL cursor_blink;
										/* �J�[�\���u�����N��� */
#endif
#if XM7_VER >= 3
extern const BYTE truecolorbrightness[64];
										/* 24/32bit Color�p�P�x�e�[�u�� */
#endif

#ifdef __cplusplus
}
#endif

#endif	/* _display_h_ */
