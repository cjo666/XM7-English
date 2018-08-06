/*
 *	FM-7 EMULATOR "XM7"
 *
 *	Copyright (C) 1999-2017 �o�h�D(Twitter:@xm6_original)
 *	Copyright (C) 2001-2017 Ryu Takegami (Twitter:@RyuTakegami)
 *
 *	[ �C���e���W�F���g�}�E�X/FM�}�E�X ]
 */

#if defined(MOUSE)

#ifndef _mouse_h_
#define _mouse_h_

#ifdef __cplusplus
extern "C" {
#endif
/*
 *	��v�G���g��
 */
BOOL FASTCALL mos_init(void);
										/* ������ */
void FASTCALL mos_cleanup(void);
										/* �N���[���A�b�v */
void FASTCALL mos_reset(void);
										/* ���Z�b�g */
void FASTCALL mos_strobe_signal(BOOL strb);
										/* �X�g���[�u�M������ */
BYTE FASTCALL mos_readdata(BYTE trigger);
										/* �f�[�^�ǂݍ��� */
BOOL FASTCALL mos_save(int fileh);
										/* �Z�[�u */
BOOL FASTCALL mos_load(int fileh, int ver);
										/* ���[�h */
BOOL FASTCALL mouse_readb(WORD addr, BYTE *dat);
										/* �}�E�X�Z�b�g I/O�ǂݏo�� */
BOOL FASTCALL mouse_writeb(WORD addr, BYTE dat);
										/* �}�E�X�Z�b�g I/O�������� */

/*
 *	��v���[�N
 */
extern BYTE mos_port;
										/* �}�E�X�ڑ��|�[�g */
extern BOOL mos_capture;
										/* �}�E�X�L���v�`���t���O */
#ifdef __cplusplus
}
#endif

#endif	/* _mouse_h_ */

#endif	/* MOUSE */
