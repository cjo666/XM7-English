/*
 *	FM-7 EMULATOR "XM7"
 *
 *	Copyright (C) 1999-2017 �o�h�D(Twitter:@xm6_original)
 *	Copyright (C) 2001-2017 Ryu Takegami (Twitter:@RyuTakegami)
 *
 *	[ Win32API �L�[�{�[�h�E�W���C�X�e�B�b�N�E�}�E�X ]
 */

#ifdef _WIN32

#ifndef _w32_kbd_h_
#define _w32_kbd_h_

/*
 *	�萔�A�^��`
 */
#define KNT_KANJI		0
#define KNT_KANA		1
#define KNT_CAPS		2
#define KNT_RSHIFT		3
#define KEYBUFFER_SIZE	64

#define MOSCAP_NONE		0
#define MOSCAP_WMESSAGE	1
#define MOSCAP_WHEELWM	2

#define VKNT_CAPITAL	0xf0
#define VKNT_KATAKANA	0xf1
#define VKNT_KANA		0xf2
#define VKNT_KANJI1		0xf3
#define VKNT_KANJI2		0xf4

#ifdef __cplusplus
extern "C" {
#endif
/*
 *	��v�G���g��
 */
void FASTCALL InitKbd(void);
										/* ������ */
void FASTCALL CleanKbd(void);
										/* �N���[���A�b�v */
BOOL FASTCALL SelectKbd(HWND hWnd);
										/* �Z���N�g */
void FASTCALL SetMenuExitTimer(void);
										/* ���j���[�����o���^�C�}�ݒ� */
void FASTCALL PollKbd(void);
										/* �L�[�{�[�h�|�[�����O */
void FASTCALL PollJoy(void);
										/* �W���C�X�e�B�b�N�|�[�����O */
void FASTCALL GetDefMapKbd(BYTE *pMap, int mode);
										/* �f�t�H���g�}�b�v�擾 */
void FASTCALL SetMapKbd(BYTE *pMap);
										/* �}�b�v�ݒ� */
BOOL FASTCALL GetKbd(BYTE *pBuf);
										/* �|�[�����O���L�[���擾 */
#if defined(KBDPASTE)
void FASTCALL PasteClipboardKbd(HWND hWnd);
										/*	�L�[�{�[�h�y�[�X�g */
void FASTCALL PasteKbd(char *pstring);
										/*	�L�[�{�[�h�y�[�X�g */
#endif
#if defined(MOUSE)
void FASTCALL PollMos(void);
										/* �}�E�X�|�[�����O */
void FASTCALL SetMouseCapture(BOOL en);
										/* �}�E�X�L���v�`���ݒ� */
#endif

/*
 *	��v���[�N
 */
extern BYTE kbd_map[256];
										/* �L�[�{�[�h �}�b�v */
extern BYTE kbd_table[256];
										/* �Ή�����FM-7�����R�[�h */
extern int nJoyType[2];
										/* �W���C�X�e�B�b�N�^�C�v */
extern int nJoyRapid[2][2];
										/* �A�˃^�C�v */
extern int nJoyCode[2][7];
										/* �����R�[�h */
extern BOOL bKbdReal;
										/* �[�����A���^�C���L�[�X�L���� */
extern BOOL bTenCursor;
										/* �e���L�[�ϊ� */
extern BOOL bArrow8Dir;
										/* �e���L�[�ϊ� 8�������[�h */
extern BOOL bNTkeyPushFlag[4];
										/* �L�[�����t���O(NT) */
extern BOOL bNTkeyMakeFlag[128];
										/* �L�[Make���t���O(NT) */
extern BOOL bNTkbMode;
										/* NT�΍����t���O */
#if defined(KBDPASTE)
extern UINT uPasteWait;
										/* �y�[�X�g�҂�����(ms) */
extern UINT uPasteWaitCntl;
										/* �y�[�X�g�҂�����(�R���g���[���R�[�h) */
#endif
#if defined(MOUSE)
extern BYTE uMidBtnMode;
										/* �����{�^����Ԏ擾���[�h */
extern BOOL bDetectMouse;
										/* �}�E�X�m�F�t���O */
#endif
#ifdef __cplusplus
}
#endif

#endif	/* _w32_kbd_h_ */
#endif	/* _WIN32 */
