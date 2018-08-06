/*
 *	FM-7 EMULATOR "XM7"
 *
 *	Copyright (C) 1999-2017 �o�h�D(Twitter:@xm6_original)
 *	Copyright (C) 2001-2017 Ryu Takegami (Twitter:@RyuTakegami)
 *
 *	[ Win32API MIDI ]
 */

#ifdef _WIN32

#if defined(MIDI)

#define STRICT
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <mmsystem.h>
#include "xm7.h"
#include "device.h"
#include "midi.h"
#include "w32.h"
#include "w32_midi.h"
#include "w32_sch.h"

/*
 *	�O���[�o�� ���[�N
 */
char szMidiDevice[256];					/* MIDI�f�o�C�X�� */
int nMidiDelay;							/* MIDI�����x������ */
BOOL bMidiDelayMode;					/* MIDI�����x�����[�h */

/*
 *	�X�^�e�B�b�N ���[�N
 */
static HMIDIOUT	hOut = 0;				/* MIDI OUT�n���h��*/
static MIDIHDR	hOutHdr;				/* MIDI OUT�w�b�_*/
static char		szOpenDevice[256];		/* �I�[�v�����̃f�o�C�X�� */

static MIDISEND	SendBuf[MAXSENDBUF];	/* MIDI�f�[�^�o�b�t�@ */
static BYTE		ExclBuf[MAXEXCLBUF];	/* �G�N�X�N���[�V�u�f�[�^�o�b�t�@ */
static int		nMidiCtrl;				/* ���M�����b�Z�[�W�̃^�C�v*/
static int		nMidiPos;				/* ���M�����b�Z�[�W�̃f�[�^�J�E���^ */
static int		nMidiSysCount;			/* �R�������b�Z�[�W�X�L�b�v�J�E���^ */
static BOOL		bMidiExcvWait;			/* �G�N�X�N���[�V�u���M���t���O */
static DWORD	dwSendReadPtr;			/* MIDI���b�Z�[�W  ���M�J�E���^ */
static DWORD	dwSendWritePtr;			/* MIDI���b�Z�[�W  �����J�E���^ */
static DWORD	dwExclWritePtr;			/* �G�N�X�N���[�V�u�����J�E���^ */
static BYTE		nMidiLastData;			/* �Ō�ɑ��M�����f�[�^ */

/*
 *	�v���g�^�C�v�錾
 */
static void FASTCALL AllNoteOff(void);
static void FASTCALL WaitLastExcvOut(void);


/*
 *	������
 */
void FASTCALL InitMIDI(void)
{
	MIDIOUTCAPS moc;
	int nDeviceID;
	UINT i;

	/* ���[�N�G���A������ */
	nMidiCtrl = MIDICTRL_READY;
	nMidiLastData = 0x80;
	bMidiExcvWait = FALSE;
	dwSendReadPtr = 0;
	dwSendWritePtr = 0;

	if (!hOut) {
		/* ���O����v����f�o�C�X������ */
		nDeviceID = (int)MIDI_MAPPER;
		for (i=0; i<midiOutGetNumDevs(); i++) {
			midiOutGetDevCaps(i, &moc, sizeof(moc));
			if (!strcmp(moc.szPname, szMidiDevice)) {
				nDeviceID = (int)i;
				break;
			}
		}

		/* MIDI�|�[�g�I�[�v�� */
		if (nDeviceID >= 0) {
			if (midiOutOpen(&hOut, (UINT)nDeviceID, 0, 0, CALLBACK_NULL)
								== MMSYSERR_NOERROR) {
				AllNoteOff();
				strncpy(szOpenDevice, moc.szPname, sizeof(szOpenDevice));
				return;
			}
		}

		/* �I�[�v�����s */
		hOut = NULL;
		strncpy(szOpenDevice, "", sizeof(szOpenDevice));
	}
}


/*
 *	�N���[���A�b�v
 */
void FASTCALL CleanMIDI(void)
{
	if (hOut) {
		AllNoteOff();
		midiOutReset(hOut);
		midiOutClose(hOut);
		hOut = NULL;
	}
}

/*
 *	�Z���N�g��ԃ`�F�b�N
 */
BOOL FASTCALL SelectCheckMIDI(char *device)
{
	/* �g�����Ƃ��Ă���|�[�g�Ǝg�p���̃|�[�g����v���Ă��邩�`�F�b�N */
	if (strcmp(szOpenDevice, device)) {
		return FALSE;
	}

	return TRUE;
}

/*
 *	���Z�b�g���f�o�C�X�Z���N�g
 */
void FASTCALL midi_reset_notify(void)
{
	/* �g�����Ƃ��Ă���|�[�g�Ǝg�p���̃|�[�g����v���Ă��邩�`�F�b�N */
	if (strcmp(szOpenDevice, szMidiDevice)) {
		/* �|�[�g���J����Ă���ꍇ�͂�������N���[���A�b�v */
		if (hOut) {
			CleanMIDI();
		}

		/* ���߂ď����� */
		InitMIDI();
	}
}


/*
 *	�S�`�����l���L�[�I�t
 */
static void FASTCALL AllNoteOff(void)
{
	DWORD msg;

	if (hOut) {
		WaitLastExcvOut();
		for (msg=0x7bb0; msg<0x7bc0; msg++) {
			midiOutShortMsg(hOut, msg);
		}
	}
}


/*
 *	�G�N�X�N���[�V�u���M�ҋ@
 */
static void FASTCALL WaitLastExcvOut(void)
{
	if (bMidiExcvWait) {
		while (midiOutUnprepareHeader(hOut, &hOutHdr,
			sizeof(MIDIHDR)) == MIDIERR_STILLPLAYING);
		bMidiExcvWait = FALSE;
	}
}


/*
 *	�G�N�X�N���[�V�u���M
 */
static void FASTCALL MidiSendExcv(DWORD ptr, int len)
{
	char buf[1024];
	int i;

	/* �f�[�^����U�o�b�t�@�ɓ]�� */
	for (i=0; i<len; i++) {
		buf[i] = ExclBuf[(i + ptr) % MAXEXCLBUF];
	}

	/* �G�N�X�N���[�V�u���M */
	hOutHdr.lpData = (char *)buf;
	hOutHdr.dwFlags = 0;
	hOutHdr.dwBufferLength = len;
	midiOutPrepareHeader(hOut, &hOutHdr, sizeof(MIDIHDR));
	midiOutLongMsg(hOut, &hOutHdr, sizeof(MIDIHDR));

	/* �G�N�X�N���[�V�u���M���t���O�𗧂Ă� */
	bMidiExcvWait = TRUE;
}


/*
 *	�f�[�^�ʒm
 */
void FASTCALL midi_notify(BYTE mes)
{
	/* MIDI OUT�����݂��Ȃ��ꍇ�͉������Ȃ� */
	if (!hOut) {
		return;
	}

	/* �o�b�t�@����t�̏ꍇ�̓f�[�^���̂Ă� */
	if (midi_busy) {
		return;
	}

	/* ���A���^�C�����b�Z�[�W���� */
	switch (mes) {
		case MIDI_TIMING:
		case MIDI_START:
		case MIDI_CONTINUE:
		case MIDI_STOP:
		case MIDI_ACTIVESENSE:
		case MIDI_SYSTEMRESET:
			return;
	}

	/* ��1�o�C�g���� */
	if (nMidiCtrl == MIDICTRL_READY) {
		if (mes & 0x80) {
			SendBuf[dwSendWritePtr].data[0] = mes;
			SendBuf[dwSendWritePtr].time = dwNowTime;
			nMidiPos = 1;

			/* �X�e�[�^�X���b�Z�[�W */
			switch (mes & 0xf0) {
				case 0x80:		/* �m�[�g�I�t */
				case 0x90:		/* �m�[�g�I�� */
				case 0xa0:		/* �|���t�H�j�b�N�L�[�v���b�V�� */
				case 0xb0:		/* �R���g���[���`�F���W */
				case 0xe0:		/* �s�b�`�x���_ */
					nMidiLastData = mes;
					SendBuf[dwSendWritePtr].length = 3;
					nMidiCtrl = MIDICTRL_3BYTES;
					break;

				case 0xc0:		/* �v���O�����`�F���W */
				case 0xd0:		/* �`�����l���v���b�V�� */
					SendBuf[dwSendWritePtr].length = 2;
					nMidiCtrl = MIDICTRL_2BYTES;
					break;

				default:
					switch (mes) {
						case MIDI_EXCLUSIVE:	/* �V�X�e���G�N�X�N���[�V�u */
							nMidiCtrl = MIDICTRL_EXCLUSIVE;
							SendBuf[dwSendWritePtr].exclptr = dwExclWritePtr;
							ExclBuf[dwExclWritePtr ++] = 0xf0;
							if (dwExclWritePtr >= MAXEXCLBUF) {
								dwExclWritePtr = 0;
							}
							break;
						case MIDI_SONGPOS:		/* �\���O�|�W�V���� */
							nMidiCtrl = MIDICTRL_SYSTEM;
							nMidiSysCount = 3;
							break;
						case MIDI_SONGSELECT:	/* �\���O�Z���N�g */
							nMidiCtrl = MIDICTRL_SYSTEM;
							nMidiSysCount = 2;
							break;
						case MIDI_TIMECODE:		/* �^�C���R�[�h */
						case MIDI_TUNEREQUEST:	/* �`���[�����N�G�X�g */
							nMidiCtrl = MIDICTRL_SYSTEM;
							nMidiSysCount = 1;
							break;
						default:				/* ����ȊO */
							return;
					}
					break;
			}
		}
		else {
			/* �����j���O�X�e�[�^�X���� */
			SendBuf[dwSendWritePtr].length = 3;
			SendBuf[dwSendWritePtr].data[0] = nMidiLastData;
			SendBuf[dwSendWritePtr].data[1] = mes;
			nMidiPos = 2;
			nMidiCtrl = MIDICTRL_3BYTES;
		}
	}
	else {
		/* ��2�o�C�g�ȍ~�̏��� */
		switch (nMidiCtrl) {
			case MIDICTRL_2BYTES:
			case MIDICTRL_3BYTES:
				SendBuf[dwSendWritePtr].data[nMidiPos] = mes;

				nMidiPos ++;
				if (nMidiPos >= SendBuf[dwSendWritePtr].length) {
					dwSendWritePtr = (WORD)((dwSendWritePtr + 1) % MAXSENDBUF);
					if (dwSendWritePtr == dwSendReadPtr) {
						midi_busy = TRUE;
					}
					nMidiCtrl = MIDICTRL_READY;
				}
				break;

			case MIDICTRL_EXCLUSIVE:
				ExclBuf[dwExclWritePtr] = mes;
				dwExclWritePtr = (WORD)((dwExclWritePtr + 1) % MAXEXCLBUF);

				nMidiPos ++;
				if (mes == MIDI_EOX) {
					/* ���b�Z�[�W�I�� */
					SendBuf[dwSendWritePtr].length = nMidiPos;
					dwSendWritePtr = (WORD)((dwSendWritePtr + 1) % MAXSENDBUF);
					if (dwSendWritePtr == dwSendReadPtr) {
						midi_busy = TRUE;
					}
					nMidiCtrl = MIDICTRL_READY;
				}
				else if (nMidiPos >= 1024) {
					/* �o�b�t�@�I�[�o�[�t���[ */
					nMidiCtrl = MIDICTRL_READY;
				}
				break;

			case MIDICTRL_SYSTEM:
				nMidiPos ++;
				if (nMidiPos >= nMidiSysCount) {
					nMidiCtrl = MIDICTRL_READY;
				}
				break;
		}
	}
}

/*
 *	�f�[�^���M (�x���߂�)
 */
void FASTCALL MidiSendData(DWORD tick)
{
	DWORD sendmsg;
	DWORD tim;

	for (;;) {
		if (dwSendReadPtr == dwSendWritePtr) {
			/* �ŏI�������݃|�C���g�ƈ�v�����ꍇ�A�I�� */
			midi_busy = FALSE;
			return;
		}

		/* �x�����ԕ��o�߂������`�F�b�N */
		tim = SendBuf[dwSendReadPtr].time;
		if (dwNowTime >= tim) {
			if ((dwNowTime - tim) < tick) {
				return;
			}
		}

		/* �G�N�X�N���[�V�u���b�Z�[�W�̑��M�I����҂� */
		WaitLastExcvOut();

		if (SendBuf[dwSendReadPtr].data[0] == 0xf0) {
			/* �G�N�X�N���[�V�u���b�Z�[�W���M */
			MidiSendExcv(SendBuf[dwSendReadPtr].exclptr,
				SendBuf[dwSendReadPtr].length);
		}
		else {
			/* �G�N�X�N���[�V�u�ȊO�̃��b�Z�[�W�𑗐M */
			sendmsg =  SendBuf[dwSendReadPtr].data[0];
			sendmsg |= SendBuf[dwSendReadPtr].data[1] << 8;
			if (SendBuf[dwSendReadPtr].length == 3) {
				sendmsg |= SendBuf[dwSendReadPtr].data[2] << 16;
			}
			midiOutShortMsg(hOut, sendmsg);
		}

		/* �J�E���^�X�V */
		dwSendReadPtr = (WORD)((dwSendReadPtr + 1) % MAXSENDBUF);
	}
}

#endif	/* MIDI */
#endif	/* _WIN32 */
