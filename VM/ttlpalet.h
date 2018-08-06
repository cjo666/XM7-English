/*
 *	FM-7 EMULATOR "XM7"
 *
 *	Copyright (C) 1999-2017 �o�h�D(Twitter:@xm6_original)
 *	Copyright (C) 2001-2017 Ryu Takegami (Twitter:@RyuTakegami)
 *
 *	[ TTL�p���b�g(MB15021) ]
 */

#ifndef _ttlpalet_h_
#define _ttlpalet_h_

#ifdef __cplusplus
extern "C" {
#endif
/*
 *	��v�G���g��
 */
BOOL FASTCALL ttlpalet_init(void);
										/* ������ */
void FASTCALL ttlpalet_cleanup(void);
										/* �N���[���A�b�v */
void FASTCALL ttlpalet_reset(void);
										/* ���Z�b�g */
BOOL FASTCALL ttlpalet_readb(WORD addr, BYTE *dat);
										/* �������ǂݏo�� */
BOOL FASTCALL ttlpalet_writeb(WORD addr, BYTE dat);
										/* �������������� */
BOOL FASTCALL ttlpalet_save(int fileh);
										/* �Z�[�u */
BOOL FASTCALL ttlpalet_load(int fileh, int ver);
										/* ���[�h */

/*
 *	��v���[�N
 */
extern BYTE ttl_palet[8];
										/* TTL�p���b�g�f�[�^ */
#ifdef __cplusplus
}
#endif

#endif	/* _ttlpalet_h_ */
