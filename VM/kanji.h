/*
 *	FM-7 EMULATOR "XM7"
 *
 *	Copyright (C) 1999-2017 �o�h�D(Twitter:@xm6_original)
 *	Copyright (C) 2001-2017 Ryu Takegami (Twitter:@RyuTakegami)
 *
 *	[ ����ROM ]
 */

#ifndef _kanji_h_
#define _kanji_h_

#ifdef __cplusplus
extern "C" {
#endif
/*
 *	��v�G���g��
 */
BOOL FASTCALL kanji_init(void);
										/* ������ */
void FASTCALL kanji_cleanup(void);
										/* �N���[���A�b�v */
void FASTCALL kanji_reset(void);
										/* ���Z�b�g */
BOOL FASTCALL kanji_readb(WORD addr, BYTE *dat);
										/* �������ǂݏo�� */
BOOL FASTCALL kanji_writeb(WORD addr, BYTE dat);
										/* �������������� */
BOOL FASTCALL kanji_save(int fileh);
										/* �Z�[�u */
BOOL FASTCALL kanji_load(int fileh, int ver);
										/* ���[�h */

/*
 *	��v���[�N
 */
extern WORD kanji_addr;
										/* �A�h���X���W�X�^(����) */
extern BYTE *kanji_rom;
										/* ��P����ROM */
#if XM7_VER >= 3
extern BYTE *kanji_rom_jis78;
										/* ��P����ROM(JIS78����) */
extern BYTE *kanji_rom2;
										/* ��Q����ROM */
#endif
extern BOOL kanji_asis_flag;
										/* JIS78�G�~�����[�V�����������t���O */
#ifdef __cplusplus
}
#endif

#endif	/* _kanji_h_ */
