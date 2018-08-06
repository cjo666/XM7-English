/*
 *	FM-7 EMULATOR "XM7"
 *
 *	Copyright (C) 1999-2017 �o�h�D(Twitter:@xm6_original)
 *	Copyright (C) 2001-2017 Ryu Takegami (Twitter:@RyuTakegami)
 *
 *	[ Win32API MIDI ]
 */

#if defined(_WIN32)
#if defined(MIDI)

#ifndef _w32_midi_h_
#define _w32_midi_h_

/*
 *	�^��`
 */
typedef struct {
	DWORD		time;			/* �R�}���h���sVM���� */
	int			length;			/* �f�[�^�o�C�g�� */
	DWORD		exclptr;		/* �G�N�X�N���[�V�u�f�[�^�̊J�n�|�C���g */
	BYTE		data[4];		/* ���M�f�[�^ */
} MIDISEND;

/*
 *	�萔��`
 */
#define	MIDICTRL_READY		0
#define	MIDICTRL_2BYTES		1
#define	MIDICTRL_3BYTES		2
#define	MIDICTRL_EXCLUSIVE	3
#define	MIDICTRL_TIMECODE	4
#define MIDICTRL_SYSTEM		5

#define	MIDI_EXCLUSIVE		0xf0
#define MIDI_TIMECODE		0xf1
#define MIDI_SONGPOS		0xf2
#define MIDI_SONGSELECT		0xf3
#define	MIDI_TUNEREQUEST	0xf6
#define	MIDI_EOX			0xf7
#define	MIDI_TIMING			0xf8
#define MIDI_START			0xfa
#define MIDI_CONTINUE		0xfb
#define	MIDI_STOP			0xfc
#define	MIDI_ACTIVESENSE	0xfe
#define	MIDI_SYSTEMRESET	0xff

#define	MAXSENDBUF			1024
#define	MAXEXCLBUF			4096

/*
 *	��v�G���g��
 */
#ifdef __cplusplus
extern "C" {
#endif
void FASTCALL InitMIDI(void);
										/* ������ */
void FASTCALL CleanMIDI(void);
										/* �N���[���A�b�v */
BOOL FASTCALL SelectMIDI(void);
										/* �Z���N�g */
BOOL FASTCALL SelectCheckMIDI(char *device);
										/* �Z���N�g��ԃ`�F�b�N */

void FASTCALL MidiSendData(DWORD tick);
										/* �f�[�^�o�� */

/*
 *	��v���[�N
 */
extern char szMidiDevice[256];
										/* MIDI�f�o�C�X�� */
extern int nMidiDelay;
										/* MIDI�����x������ */
extern BOOL bMidiDelayMode;
										/* MIDI�����x�����[�h */
#ifdef __cplusplus
}
#endif

#endif	/* _w32_midi_h_ */
#endif
#endif	/* _WIN32 */
