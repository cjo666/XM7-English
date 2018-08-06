/*
 *	FM-7 EMULATOR "XM7"
 *
 *	Copyright (C) 1999-2017 �o�h�D(Twitter:@xm6_original)
 *	Copyright (C) 2001-2017 Ryu Takegami (Twitter:@RyuTakegami)
 *
 *	[ OPN/WHG/THG(YM2203) ]
 */

#ifndef _opn_h_
#define _opn_h_

/*
 *	�萔��`
 */
#define OPN_STD				0				/* �W��OPN */
#define OPN_WHG				1				/* WHG OPN */
#define OPN_THG				2				/* THG OPN */

#define OPN_INACTIVE		0x00			/* �C���A�N�e�B�u�R�}���h */
#define OPN_READDAT			0x01			/* ���[�h�f�[�^�R�}���h */
#define OPN_WRITEDAT		0x02			/* ���C�g�f�[�^�R�}���h */
#define OPN_ADDRESS			0x03			/* ���b�`�A�h���X�R�}���h */
#define OPN_READSTAT		0x04			/* ���[�h�X�e�[�^�X�R�}���h */
#define OPN_JOYSTICK		0x09			/* �W���C�X�e�B�b�N�R�}���h */

#define	OPN_CLOCK			12288			/* OPN��N���b�N(1.2288MHz) */
#define	PSG_CLOCK_MSX		1789772			/* MSX PSG��N���b�N */

#ifdef __cplusplus
extern "C" {
#endif
/*
 *	��v�G���g��
 */
BOOL FASTCALL opn_init(void);
										/* ������ */
void FASTCALL opn_cleanup(void);
										/* �N���[���A�b�v */
void FASTCALL opn_reset(void);
										/* ���Z�b�g */
BOOL FASTCALL opn_readb(WORD addr, BYTE *dat);
										/* OPN �������ǂݏo�� */
BOOL FASTCALL opn_writeb(WORD addr, BYTE dat);
										/* OPN �������������� */
BOOL FASTCALL opn_save(int fileh);
										/* OPN �Z�[�u */
BOOL FASTCALL opn_load(int fileh, int ver);
										/* OPN ���[�h */

BOOL FASTCALL whg_init(void);
										/* WHG ������ */
void FASTCALL whg_cleanup(void);
										/* WHG �N���[���A�b�v */
void FASTCALL whg_reset(void);
										/* WHG ���Z�b�g */
BOOL FASTCALL whg_readb(WORD addr, BYTE *dat);
										/* WHG �������ǂݏo�� */
BOOL FASTCALL whg_writeb(WORD addr, BYTE dat);
										/* WHG �������������� */
BOOL FASTCALL whg_save(int fileh);
										/* WHG �Z�[�u */
BOOL FASTCALL whg_load(int fileh, int ver);
										/* WHG ���[�h */

BOOL FASTCALL thg_init(void);
										/* THG ������ */
void FASTCALL thg_cleanup(void);
										/* THG �N���[���A�b�v */
void FASTCALL thg_reset(void);
										/* THG ���Z�b�g */
BOOL FASTCALL thg_readb(WORD addr, BYTE *dat);
										/* THG �������ǂݏo�� */
BOOL FASTCALL thg_writeb(WORD addr, BYTE dat);
										/* THG �������������� */
BOOL FASTCALL thg_save(int fileh);
										/* THG �Z�[�u */
BOOL FASTCALL thg_load(int fileh, int ver);
										/* THG ���[�h */

/*
 *	��v���[�N
 */
extern BOOL opn_enable;
										/* OPN�L���E�����t���O(7 only) */
extern BOOL opn_use;
										/* OPN�g�p�t���O */
extern BOOL whg_enable;
										/* WHG�L���E�����t���O */
extern BOOL whg_use;
										/* WHG�g�p�t���O */
extern BOOL thg_enable;
										/* THG�L���E�����t���O */
extern BOOL thg_use;
										/* THG�g�p�t���O */
#if XM7_VER == 1
extern BOOL fmx_flag;
										/* FM-X PSG���[�h�t���O */
extern BOOL fmx_use;
										/* FM-X PSG �g�p�t���O */
#endif

extern BYTE opn_reg[3][256];
										/* OPN���W�X�^ */
extern BOOL opn_key[3][4];
										/* OPN�L�[�I���t���O */
extern BOOL opn_timera[3];
										/* �^�C�}�[A����t���O */
extern BOOL opn_timerb[3];
										/* �^�C�}�[B����t���O */
extern DWORD opn_timera_tick[3];
										/* �^�C�}�[A�Ԋu(us) */
extern DWORD opn_timerb_tick[3];
										/* �^�C�}�[B�Ԋu(us) */
extern BYTE opn_scale[3];
										/* �v���X�P�[�� */
#ifdef __cplusplus
}
#endif

#endif	/* _opn_h_ */
