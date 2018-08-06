/*
 *	FM-7 EMULATOR "XM7"
 *
 *	Copyright (C) 1999-2017 �o�h�D(Twitter:@xm6_original)
 *	Copyright (C) 2001-2017 Ryu Takegami (Twitter:@RyuTakegami)
 *
 *	[ MIDI�A�_�v�^ (��h!FM�f��MIDI�J�[�h?) ]
 */

#ifndef _midi_h_
#define _midi_h_

#if defined(MIDI)

#ifdef __cplusplus
extern "C" {
#endif
/*
 *	��v�G���g��
 */
BOOL FASTCALL midi_init(void);
										/* ������ */
void FASTCALL midi_cleanup(void);
										/* �N���[���A�b�v */
void FASTCALL midi_reset(void);
										/* ���Z�b�g */
BOOL FASTCALL midi_readb(WORD addr, BYTE *dat);
										/* �������ǂݏo�� */
BOOL FASTCALL midi_writeb(WORD addr, BYTE dat);
										/* �������������� */
BOOL FASTCALL midi_save(int fileh);
										/* �Z�[�u */
BOOL FASTCALL midi_load(int fileh, int ver);
										/* ���[�h */

/*
 *	��v���[�N
 */
extern BOOL midi_busy;
										/* MIDI�o�b�t�@�I�[�o�[�t���[ */
#ifdef __cplusplus
}
#endif

#endif	/* MIDI */
#endif	/* _midi_h_ */
