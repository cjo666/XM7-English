/*
 *	FM-7 EMULATOR "XM7"
 *
 *	Copyright (C) 1999-2017 �o�h�D(Twitter:@xm6_original)
 *	Copyright (C) 2001-2017 Ryu Takegami (Twitter:@RyuTakegami)
 *
 *	[ ���{��J�[�h / �g���T�u�V�X�e��ROM ]
 */

#ifndef _jcard_h_
#define _jcard_h_

#if XM7_VER >= 2

#ifdef __cplusplus
extern "C" {
#endif
/*
 *	��v�G���g��
 */
BOOL FASTCALL jcard_init(void);
										/* ������ */
void FASTCALL jcard_cleanup(void);
										/* �N���[���A�b�v */
void FASTCALL jcard_reset(void);
										/* ���Z�b�g */
BYTE FASTCALL jcard_readb(WORD addr);
										/* �������ǂݏo�� */
void FASTCALL jcard_writeb(WORD addr, BYTE dat);
										/* �������������� */
BOOL FASTCALL jcard_save(int fileh);
										/* �Z�[�u */
BOOL FASTCALL jcard_load(int fileh, int ver);
										/* ���[�h */

/*
 *	������ (FM77AV40�E���{��J�[�h��)
 */
extern BYTE *extram_b;
										/* �g��RAM         $10000 */
extern BYTE *dicrom;
										/* ����ROM         $40000 */
#if XM7_VER >= 3
extern BYTE *extrom;
										/* �g��ROM(EX/SX)  $20000 */
#endif
extern BYTE *dicram;
										/* �w�KRAM          $2000 */

/*
 *	��v���[�N
 */
extern BYTE dicrom_bank;
										/* ����ROM�o���N�ԍ� */
extern BOOL dicrom_en;
										/* ����ROM�A�N�e�B�u */
extern BOOL dicram_en;
										/* �w�KRAM�A�N�e�B�u */
#if XM7_VER >= 3
extern BOOL extrom_sel;
										/* ����ROM/�g��ROM�I���t���O */
#endif
#if XM7_VER == 2
extern BOOL jcard_available;
										/* ���{��J�[�h�g�p�ۃt���O */
#endif
extern BOOL jcard_enable;
										/* ���{��J�[�h�L���t���O */
#ifdef __cplusplus
}
#endif

#endif	/* XM7_VER >= 3 */
#endif	/* _jcard_h_ */
