/*
 *	FM-7 EMULATOR "XM7"
 *
 *	Copyright (C) 1999-2017 �o�h�D(Twitter:@xm6_original)
 *	Copyright (C) 2001-2017 Ryu Takegami (Twitter:@RyuTakegami)
 *	line printer support 2010-2013 by Ben.JP
 *
 *	[ �J�Z�b�g�e�[�v���v�����^ ]
 */

#ifndef _tapelp_h_
#define _tapelp_h_

/*
 *	�萔�A�^��`
 */
#define	TAPE_SAVEBUFSIZE	4096			/* �e�[�v�������݃o�b�t�@�T�C�Y */
#define	TAPE_LOADBUFSIZE	4096			/* �e�[�v�ǂݍ��݃o�b�t�@�T�C�Y */
#define	LP_TEMPFILENAME		"xm7_lpr.$$$"	/* �v�����^ �e���|�����t�@�C���� */
#define	LP_DISABLE			0				/* �v�����^ �G�~�����[�V�������� */
#define	LP_EMULATION		1				/* �v�����^ �G�~�����[�V�����L�� */
#define	LP_LOG				2				/* �v�����^ ���O�t�@�C���o�� */
#define	LP_JASTSOUND		3				/* �v�����^ �W���X�g�T�E���h�o�� */

#ifdef __cplusplus
extern "C" {
#endif
/*
 *	��v�G���g��
 */
BOOL FASTCALL tapelp_init(void);
										/* ������ */
void FASTCALL tapelp_cleanup(void);
										/* �N���[���A�b�v */
void FASTCALL tapelp_reset(void);
										/* ���Z�b�g */
BOOL FASTCALL tapelp_readb(WORD addr, BYTE *dat);
										/* �������ǂݏo�� */
BOOL FASTCALL tapelp_writeb(WORD addr, BYTE dat);
										/* �������������� */
BOOL FASTCALL tapelp_save(int fileh);
										/* �Z�[�u */
BOOL FASTCALL tapelp_load(int fileh, int ver);
										/* ���[�h */
void FASTCALL lp_setfile(char *fname);
										/* �v�����^�t�@�C���ݒ� */
#if defined(LPRINT)
void FASTCALL lp_print(void);
										/* �v�����^�ֈ�� */
#endif
void FASTCALL tape_setfile(char *fname);
										/* �e�[�v�t�@�C���ݒ� */
void FASTCALL tape_setrec(BOOL flag);
										/* �e�[�v�^���t���O�ݒ� */
void FASTCALL tape_rew(void);
										/* �e�[�v�����߂� */
void FASTCALL tape_rewtop(void);
										/* �e�[�v���ŏ��܂Ŋ����߂� */
void FASTCALL tape_ff(void);
										/* �e�[�v������ */

/*
 *	��v���[�N
 */
extern BOOL tape_in;
										/* �e�[�v ���̓f�[�^ */
extern BOOL tape_out;
										/* �e�[�v �o�̓f�[�^ */
extern BOOL tape_motor;
										/* �e�[�v ���[�^ */
extern BOOL tape_rec;
										/* �e�[�v �^���� */
extern BOOL tape_writep;
										/* �e�[�v �^���s�� */
extern WORD tape_count;
										/* �e�[�v �J�E���^ */
extern DWORD tape_subcnt;
										/* �e�[�v �T�u�J�E���g */
extern int tape_fileh;
										/* �e�[�v �t�@�C���n���h�� */
extern DWORD tape_offset;
										/* �e�[�v �t�@�C���I�t�Z�b�g */
extern char tape_fname[256+1];
										/* �e�[�v �t�@�C���l�[�� */
extern BOOL tape_monitor;
										/* �e�[�v �e�[�v�����j�^�t���O */
extern BOOL tape_sound;
										/* �e�[�v �����[���o�̓t���O */

#if defined(LPRINT)
extern BYTE	lp_use;
										/* �v�����^ �g�p���[�h */
#endif
extern BYTE lp_data;
										/* �v�����^ �o�̓f�[�^ */
extern BOOL lp_busy;
										/* �v�����^ BUSY�t���O */
extern BOOL lp_error;
										/* �v�����^ �G���[�t���O */
extern BOOL lp_pe;
										/* �v�����^ PE�t���O */
extern BOOL lp_ackng;
										/* �v�����^ ACK�t���O */
extern BOOL lp_online;
										/* �v�����^ �I�����C�� */
extern BOOL lp_strobe;
										/* �v�����^ �X�g���[�u */
extern char lp_fname[256+1];
										/* �v�����^ �t�@�C���l�[�� */
extern int lp_fileh;
										/* �v�����^ �t�@�C���n���h�� */

#ifdef __cplusplus
}
#endif

#endif	/* _tapelp_h_ */
