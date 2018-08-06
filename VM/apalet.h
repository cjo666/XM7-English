/*
 *	FM-7 EMULATOR "XM7"
 *
 *	Copyright (C) 1999-2017 �o�h�D(Twitter:@xm6_original)
 *	Copyright (C) 2001-2017 Ryu Takegami (Twitter:@RyuTakegami)
 *
 *	[ �A�i���O�p���b�g ]
 */

#ifndef _apalet_h_
#define _apalet_h_

#if XM7_VER >= 2

#ifdef __cplusplus
extern "C" {
#endif
/*
 *	��v�G���g��
 */
BOOL FASTCALL apalet_init(void);
										/* ������ */
void FASTCALL apalet_cleanup(void);
										/* �N���[���A�b�v */
void FASTCALL apalet_reset(void);
										/* ���Z�b�g */
BOOL FASTCALL apalet_readb(WORD addr, BYTE *dat);
										/* �������ǂݏo�� */
BOOL FASTCALL apalet_writeb(WORD addr, BYTE dat);
										/* �������������� */
BOOL FASTCALL apalet_save(int fileh);
										/* �Z�[�u */
BOOL FASTCALL apalet_load(int fileh, int ver);
										/* ���[�h */

/*
 *	��v���[�N
 */
extern WORD apalet_no;
										/* �p���b�g�i���o */
extern BYTE apalet_b[4096];
										/* B���x��(0-15) */
extern BYTE apalet_r[4096];
										/* R���x��(0-15) */
extern BYTE apalet_g[4096];
										/* G���x��(0-15) */

#ifdef __cplusplus
}
#endif

#endif	/* XM7_VER >= 2 */
#endif	/* _apalet_h_ */
