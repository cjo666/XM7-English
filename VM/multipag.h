/*
 *	FM-7 EMULATOR "XM7"
 *
 *	Copyright (C) 1999-2017 �o�h�D(Twitter:@xm6_original)
 *	Copyright (C) 2001-2017 Ryu Takegami (Twitter:@RyuTakegami)
 *
 *	[ �}���`�y�[�W ]
 */

#ifndef _multipag_h_
#define _multipag_h_

#ifdef __cplusplus
extern "C" {
#endif
/*
 *	��v�G���g��
 */
BOOL FASTCALL multipag_init(void);
										/* ������ */
void FASTCALL multipag_cleanup(void);
										/* �N���[���A�b�v */
void FASTCALL multipag_reset(void);
										/* ���Z�b�g */
BOOL FASTCALL multipag_readb(WORD addr, BYTE *dat);
										/* �������ǂݏo�� */
BOOL FASTCALL multipag_writeb(WORD addr, BYTE dat);
										/* �������������� */
BOOL FASTCALL multipag_save(int fileh);
										/* �Z�[�u */
BOOL FASTCALL multipag_load(int fileh, int ver);
										/* ���[�h */

/*
 *	��v���[�N
 */
extern BYTE multi_page;
										/* �}���`�y�[�W */
#ifdef __cplusplus
}
#endif

#endif	/* _multipag_h_ */
