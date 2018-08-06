/*
 *	FM-7 EMULATOR "XM7"
 *
 *	Copyright (C) 1999-2017 ＰＩ．(Twitter:@xm6_original)
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
 *	グローバル ワーク
 */
char szMidiDevice[256];					/* MIDIデバイス名 */
int nMidiDelay;							/* MIDI発音遅延時間 */
BOOL bMidiDelayMode;					/* MIDI発音遅延モード */

/*
 *	スタティック ワーク
 */
static HMIDIOUT	hOut = 0;				/* MIDI OUTハンドル*/
static MIDIHDR	hOutHdr;				/* MIDI OUTヘッダ*/
static char		szOpenDevice[256];		/* オープン中のデバイス名 */

static MIDISEND	SendBuf[MAXSENDBUF];	/* MIDIデータバッファ */
static BYTE		ExclBuf[MAXEXCLBUF];	/* エクスクルーシブデータバッファ */
static int		nMidiCtrl;				/* 送信中メッセージのタイプ*/
static int		nMidiPos;				/* 送信中メッセージのデータカウンタ */
static int		nMidiSysCount;			/* コモンメッセージスキップカウンタ */
static BOOL		bMidiExcvWait;			/* エクスクルーシブ送信中フラグ */
static DWORD	dwSendReadPtr;			/* MIDIメッセージ  送信カウンタ */
static DWORD	dwSendWritePtr;			/* MIDIメッセージ  書込カウンタ */
static DWORD	dwExclWritePtr;			/* エクスクルーシブ書込カウンタ */
static BYTE		nMidiLastData;			/* 最後に送信したデータ */

/*
 *	プロトタイプ宣言
 */
static void FASTCALL AllNoteOff(void);
static void FASTCALL WaitLastExcvOut(void);


/*
 *	初期化
 */
void FASTCALL InitMIDI(void)
{
	MIDIOUTCAPS moc;
	int nDeviceID;
	UINT i;

	/* ワークエリア初期化 */
	nMidiCtrl = MIDICTRL_READY;
	nMidiLastData = 0x80;
	bMidiExcvWait = FALSE;
	dwSendReadPtr = 0;
	dwSendWritePtr = 0;

	if (!hOut) {
		/* 名前が一致するデバイスを検索 */
		nDeviceID = (int)MIDI_MAPPER;
		for (i=0; i<midiOutGetNumDevs(); i++) {
			midiOutGetDevCaps(i, &moc, sizeof(moc));
			if (!strcmp(moc.szPname, szMidiDevice)) {
				nDeviceID = (int)i;
				break;
			}
		}

		/* MIDIポートオープン */
		if (nDeviceID >= 0) {
			if (midiOutOpen(&hOut, (UINT)nDeviceID, 0, 0, CALLBACK_NULL)
								== MMSYSERR_NOERROR) {
				AllNoteOff();
				strncpy(szOpenDevice, moc.szPname, sizeof(szOpenDevice));
				return;
			}
		}

		/* オープン失敗 */
		hOut = NULL;
		strncpy(szOpenDevice, "", sizeof(szOpenDevice));
	}
}


/*
 *	クリーンアップ
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
 *	セレクト状態チェック
 */
BOOL FASTCALL SelectCheckMIDI(char *device)
{
	/* 使おうとしているポートと使用中のポートが一致しているかチェック */
	if (strcmp(szOpenDevice, device)) {
		return FALSE;
	}

	return TRUE;
}

/*
 *	リセット時デバイスセレクト
 */
void FASTCALL midi_reset_notify(void)
{
	/* 使おうとしているポートと使用中のポートが一致しているかチェック */
	if (strcmp(szOpenDevice, szMidiDevice)) {
		/* ポートが開かれている場合はいったんクリーンアップ */
		if (hOut) {
			CleanMIDI();
		}

		/* 改めて初期化 */
		InitMIDI();
	}
}


/*
 *	全チャンネルキーオフ
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
 *	エクスクルーシブ送信待機
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
 *	エクスクルーシブ送信
 */
static void FASTCALL MidiSendExcv(DWORD ptr, int len)
{
	char buf[1024];
	int i;

	/* データを一旦バッファに転送 */
	for (i=0; i<len; i++) {
		buf[i] = ExclBuf[(i + ptr) % MAXEXCLBUF];
	}

	/* エクスクルーシブ送信 */
	hOutHdr.lpData = (char *)buf;
	hOutHdr.dwFlags = 0;
	hOutHdr.dwBufferLength = len;
	midiOutPrepareHeader(hOut, &hOutHdr, sizeof(MIDIHDR));
	midiOutLongMsg(hOut, &hOutHdr, sizeof(MIDIHDR));

	/* エクスクルーシブ送信中フラグを立てる */
	bMidiExcvWait = TRUE;
}


/*
 *	データ通知
 */
void FASTCALL midi_notify(BYTE mes)
{
	/* MIDI OUTが存在しない場合は何もしない */
	if (!hOut) {
		return;
	}

	/* バッファが一杯の場合はデータを捨てる */
	if (midi_busy) {
		return;
	}

	/* リアルタイムメッセージ処理 */
	switch (mes) {
		case MIDI_TIMING:
		case MIDI_START:
		case MIDI_CONTINUE:
		case MIDI_STOP:
		case MIDI_ACTIVESENSE:
		case MIDI_SYSTEMRESET:
			return;
	}

	/* 第1バイト処理 */
	if (nMidiCtrl == MIDICTRL_READY) {
		if (mes & 0x80) {
			SendBuf[dwSendWritePtr].data[0] = mes;
			SendBuf[dwSendWritePtr].time = dwNowTime;
			nMidiPos = 1;

			/* ステータスメッセージ */
			switch (mes & 0xf0) {
				case 0x80:		/* ノートオフ */
				case 0x90:		/* ノートオン */
				case 0xa0:		/* ポリフォニックキープレッシャ */
				case 0xb0:		/* コントロールチェンジ */
				case 0xe0:		/* ピッチベンダ */
					nMidiLastData = mes;
					SendBuf[dwSendWritePtr].length = 3;
					nMidiCtrl = MIDICTRL_3BYTES;
					break;

				case 0xc0:		/* プログラムチェンジ */
				case 0xd0:		/* チャンネルプレッシャ */
					SendBuf[dwSendWritePtr].length = 2;
					nMidiCtrl = MIDICTRL_2BYTES;
					break;

				default:
					switch (mes) {
						case MIDI_EXCLUSIVE:	/* システムエクスクルーシブ */
							nMidiCtrl = MIDICTRL_EXCLUSIVE;
							SendBuf[dwSendWritePtr].exclptr = dwExclWritePtr;
							ExclBuf[dwExclWritePtr ++] = 0xf0;
							if (dwExclWritePtr >= MAXEXCLBUF) {
								dwExclWritePtr = 0;
							}
							break;
						case MIDI_SONGPOS:		/* ソングポジション */
							nMidiCtrl = MIDICTRL_SYSTEM;
							nMidiSysCount = 3;
							break;
						case MIDI_SONGSELECT:	/* ソングセレクト */
							nMidiCtrl = MIDICTRL_SYSTEM;
							nMidiSysCount = 2;
							break;
						case MIDI_TIMECODE:		/* タイムコード */
						case MIDI_TUNEREQUEST:	/* チューンリクエスト */
							nMidiCtrl = MIDICTRL_SYSTEM;
							nMidiSysCount = 1;
							break;
						default:				/* それ以外 */
							return;
					}
					break;
			}
		}
		else {
			/* ランニングステータス処理 */
			SendBuf[dwSendWritePtr].length = 3;
			SendBuf[dwSendWritePtr].data[0] = nMidiLastData;
			SendBuf[dwSendWritePtr].data[1] = mes;
			nMidiPos = 2;
			nMidiCtrl = MIDICTRL_3BYTES;
		}
	}
	else {
		/* 第2バイト以降の処理 */
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
					/* メッセージ終了 */
					SendBuf[dwSendWritePtr].length = nMidiPos;
					dwSendWritePtr = (WORD)((dwSendWritePtr + 1) % MAXSENDBUF);
					if (dwSendWritePtr == dwSendReadPtr) {
						midi_busy = TRUE;
					}
					nMidiCtrl = MIDICTRL_READY;
				}
				else if (nMidiPos >= 1024) {
					/* バッファオーバーフロー */
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
 *	データ送信 (遅延憑き)
 */
void FASTCALL MidiSendData(DWORD tick)
{
	DWORD sendmsg;
	DWORD tim;

	for (;;) {
		if (dwSendReadPtr == dwSendWritePtr) {
			/* 最終書き込みポイントと一致した場合、終了 */
			midi_busy = FALSE;
			return;
		}

		/* 遅延時間分経過したかチェック */
		tim = SendBuf[dwSendReadPtr].time;
		if (dwNowTime >= tim) {
			if ((dwNowTime - tim) < tick) {
				return;
			}
		}

		/* エクスクルーシブメッセージの送信終了を待つ */
		WaitLastExcvOut();

		if (SendBuf[dwSendReadPtr].data[0] == 0xf0) {
			/* エクスクルーシブメッセージ送信 */
			MidiSendExcv(SendBuf[dwSendReadPtr].exclptr,
				SendBuf[dwSendReadPtr].length);
		}
		else {
			/* エクスクルーシブ以外のメッセージを送信 */
			sendmsg =  SendBuf[dwSendReadPtr].data[0];
			sendmsg |= SendBuf[dwSendReadPtr].data[1] << 8;
			if (SendBuf[dwSendReadPtr].length == 3) {
				sendmsg |= SendBuf[dwSendReadPtr].data[2] << 16;
			}
			midiOutShortMsg(hOut, sendmsg);
		}

		/* カウンタ更新 */
		dwSendReadPtr = (WORD)((dwSendReadPtr + 1) % MAXSENDBUF);
	}
}

#endif	/* MIDI */
#endif	/* _WIN32 */
