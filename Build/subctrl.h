/*
 *	FM-7 EMULATOR "XM7"
 *
 *	Copyright (C) 1999-2017 �o�h�D(Twitter:@xm6_original)
 *	Copyright (C) 2001-2017 Ryu Takegami (Twitter:@RyuTakegami)
 *
 *	[ �T�uCPU�R���g���[�� ]
 */

#ifndef _subctrl_h_
#define _subctrl_h_

#ifdef __cplusplus
extern "C" {
#endif
/*
 *	��v�G���g��
 */
BOOL FASTCALL subctrl_init(void);
										/* ������ */
void FASTCALL subctrl_cleanup(void);
										/* �N���[���A�b�v */
void FASTCALL subctrl_reset(void);
										/* ���Z�b�g */
void FASTCALL subctrl_halt_ack(void);
										/* HALT/CANCEL�A�N�m���b�W */
BOOL FASTCALL subctrl_readb(WORD addr, BYTE *dat);
										/* �������ǂݏo�� */
BOOL FASTCALL subctrl_writeb(WORD addr, BYTE dat);
										/* �������������� */
BOOL FASTCALL subctrl_save(int fileh);
										/* �Z�[�u */
BOOL FASTCALL subctrl_load(int fileh, int ver);
										/* ���[�h */

/*
 *	��v���[�N
 */
extern BOOL subhalt_flag;
										/* �T�uHALT�t���O */
extern BOOL subbusy_flag;
										/* �T�uBUSY�t���O */
extern BOOL subcancel_flag;
										/* �T�u�L�����Z���t���O */
extern BOOL subattn_flag;
										/* �T�u�A�e���V�����t���O */
extern BOOL subhalt_request;
										/* �T�uHALT���N�G�X�g�t���O */
extern BOOL subcancel_request;
										/* �T�u�L�����Z�����N�G�X�g�t���O */
extern BYTE shared_ram[0x80];
										/* ���LRAM */
extern BOOL subreset_flag;
										/* �T�u�ċN���t���O */
extern BYTE busy_CLR_count;
										/* BUSY($D40A) CLR���ߎ��s���J�E���^ */
extern BOOL extdet_disable;
										/* EXTDET�f�B�Z�[�u�� */
extern BOOL mode320;
										/* 320x200���[�h */
#if XM7_VER >= 3
extern BOOL mode400l;
										/* 640x400���[�h */
extern BOOL mode256k;
										/* 26���F���[�h */
extern BOOL subram_protect;
										/* �T�u���j�^RAM�v���e�N�g */
extern BOOL subreset_halt;
										/* HALT���ċN���t���O */
extern BOOL subkanji_flag;
										/* ����ROM �T�uCPU�ڑ��t���O */
extern BOOL submode_fix;
										/* FM77AV�pOS-9�΍�t���O */
#endif
#if XM7_VER == 1 && defined(L4CARD)
extern BOOL select_400line;
										/* 400���C���J�[�h���[�h */
extern BOOL subkanji_flag;
										/* ����ROM �T�uCPU�ڑ��t���O */
#endif
#ifdef __cplusplus
}
#endif

#endif	/* _subctrl_h_ */
