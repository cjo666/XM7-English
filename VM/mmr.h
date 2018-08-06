/*
 *	FM-7 EMULATOR "XM7"
 *
 *	Copyright (C) 1999-2017 �o�h�D(Twitter:@xm6_original)
 *	Copyright (C) 2001-2017 Ryu Takegami (Twitter:@RyuTakegami)
 *
 *	[ MMR,TWR / I/O�^RAM�f�B�X�N ]
 */

#ifndef _mmr_h_
#define _mmr_h_

#ifdef __cplusplus
extern "C" {
#endif
/*
 *	��v�G���g��
 */
BOOL FASTCALL mmr_init(void);
										/* ������ */
void FASTCALL mmr_cleanup(void);
										/* �N���[���A�b�v */
void FASTCALL mmr_reset(void);
										/* ���Z�b�g */
BOOL FASTCALL mmr_trans_twr(WORD addr, DWORD *taddr);
										/* TWR�A�h���X�ϊ� */
DWORD FASTCALL mmr_trans_mmr(WORD addr);
										/* MMR�A�h���X�ϊ� */
DWORD FASTCALL mmr_trans_phys_to_logi(DWORD target);
										/* �������_���A�h���X�ϊ� */
BOOL FASTCALL mmr_readb(WORD addr, BYTE *dat);
										/* �������ǂݏo�� */
BOOL FASTCALL mmr_writeb(WORD addr, BYTE dat);
										/* �������������� */
BOOL FASTCALL mmr_extrb(WORD *addr, BYTE *dat);
										/* MMR�o�R�ǂݏo�� */
BOOL FASTCALL mmr_extbnio(WORD *addr, BYTE *dat);
										/* MMR�o�R�ǂݏo��(I/O�Ȃ�) */
BOOL FASTCALL mmr_extwb(WORD *addr, BYTE dat);
										/* MMR�o�R�������� */
BOOL FASTCALL mmr_save(int fileh);
										/* �Z�[�u */
BOOL FASTCALL mmr_load(int fileh, int ver);
										/* ���[�h */

/*
 *	��v���[�N
 */
extern BOOL mmr_flag;
										/* MMR�L���t���O */
extern BYTE mmr_seg;
										/* MMR�Z�O�����g */
extern BOOL mmr_modify;
										/* MMR��ԕύX�t���O */
#if XM7_VER >= 3
extern BYTE mmr_reg[0x80];
										/* MMR���W�X�^ */
extern BOOL mmr_ext;
										/* �g��MMR�L���t���O */
extern BOOL mmr_fastmode;
										/* MMR�����t���O */
extern BOOL mmr_extram;
										/* �g��RAM�L���t���O */
extern BOOL mmr_fast_refresh;
										/* �������t���b�V���t���O */
extern BOOL twr_fastmode;
										/* TWR�������[�h */
extern BOOL mmr_768kbmode;
										/* MMR���[�h(TRUE=768KB,FALSE=256KB) */
extern BOOL dsp_400linetiming;
										/* 400���C���^�C�~���O�o�̓t���O */
#else
extern BYTE mmr_reg[0x40];
										/* MMR���W�X�^ */
#endif
#if XM7_VER == 1
extern BYTE bnk_reg;
										/* �g��RAM�o���N�Z���N�g���W�X�^ */
extern BOOL mmr_64kbmode;
										/* MMR���[�h (TRUE=64KB,FALSE=192KB) */
#endif

extern BOOL twr_flag;
										/* TWR�L���t���O */
extern BYTE twr_reg;
										/* TWR���W�X�^ */
#ifdef __cplusplus
}
#endif

#endif	/* _mmr_h_ */
