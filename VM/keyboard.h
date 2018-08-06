/*
 *	FM-7 EMULATOR "XM7"
 *
 *	Copyright (C) 1999-2017 �o�h�D(Twitter:@xm6_original)
 *	Copyright (C) 2001-2017 Ryu Takegami (Twitter:@RyuTakegami)
 *
 *	[ �L�[�{�[�h ]
 */

#ifndef _keyboard_h_
#define _keyboard_h_

/*
 *	�萔��`
 */
#define KEY_FORMAT_9BIT		0			/* FM-7�݊� */
#define KEY_FORMAT_FM16B	1			/* FM-16���݊� */
#define KEY_FORMAT_SCAN		2			/* �����R�[�h���[�h */

#ifdef __cplusplus
extern "C" {
#endif
/*
 *	��v�G���g��
 */
BOOL FASTCALL keyboard_init(void);
										/* ������ */
void FASTCALL keyboard_cleanup(void);
										/* �N���[���A�b�v */
void FASTCALL keyboard_reset(void);
										/* ���Z�b�g */
BOOL FASTCALL keyboard_readb(WORD addr, BYTE *dat);
										/* �������ǂݏo�� */
BOOL FASTCALL keyboard_writeb(WORD addr, BYTE dat);
										/* �������������� */
BOOL FASTCALL keyboard_save(int fileh);
										/* �Z�[�u */
BOOL FASTCALL keyboard_load(int fileh, int ver);
										/* ���[�h */
void FASTCALL keyboard_repeat(void);
										/* �L�[���s�[�g�^�C�}�ύX */
void FASTCALL keyboard_make(BYTE dat);
										/* �L�[���� */
void FASTCALL keyboard_break(BYTE dat);
										/* �L�[������ */
#if defined(KBDPASTE)
BOOL FASTCALL keyboard_paste(BYTE dat);
										/* �L�[�y�[�X�g */
#endif

/*
 *	��v���[�N
 */
extern BOOL caps_flag;
										/* CAPS �t���O */
extern BOOL kana_flag;
										/* �J�i �t���O */
extern BOOL ins_flag;
										/* INS �t���O */
extern BOOL shift_flag;
										/* SHIFT�L�[�t���O */
extern BOOL lshift_flag;
										/* ��SHIFT�L�[�t���O */
extern BOOL rshift_flag;
										/* �ESHIFT�L�[�t���O */
extern BOOL ctrl_flag;
										/* CTRL�L�[�t���O */
extern BOOL graph_flag;
										/* GRAPH�L�[�t���O */
extern BOOL break_flag;
										/* Break�L�[�t���O */
extern BYTE key_scan;
										/* �L�[�R�[�h(����, Make/Break���p) */
extern WORD key_fm7;
										/* �L�[�R�[�h(FM-7�݊�) */
extern BOOL key_repeat_flag;
										/* �L�[���s�[�g�L���t���O */

extern BYTE key_format;
										/* �R�[�h�t�H�[�}�b�g */
extern DWORD key_repeat_time1;
										/* �L�[���s�[�g�J�n���� */
extern DWORD key_repeat_time2;
										/* �L�[���s�[�g�Ԋu */

extern BYTE simpose_mode;
										/* �X�[�p�[�C���|�[�Y ���[�h */
extern BOOL simpose_half;
										/* �X�[�p�[�C���|�[�Y �n�[�t�g�[�� */
extern BOOL digitize_enable;
										/* �f�B�W�^�C�Y�L���E�����t���O */
extern BOOL digitize_keywait;
										/* �f�B�W�^�C�Y�L�[�҂� */
#if XM7_VER >= 2
extern BOOL keyenc_beep_flag;
										/* �L�[�G���R�[�_�P��BEEP�t���O */
#else
extern BOOL key_mode_flag;
										/* FM-8���[�h����FM-7�݊��L�[����
										   ���[�h�t���O */
#endif
#ifdef __cplusplus
}
#endif

#endif	/* _keyboard_h_ */
