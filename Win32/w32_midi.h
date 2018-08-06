/*
 *	FM-7 EMULATOR "XM7"
 *
 *	Copyright (C) 1999-2017 ＰＩ．(Twitter:@xm6_original)
 *	Copyright (C) 2001-2017 Ryu Takegami (Twitter:@RyuTakegami)
 *
 *	[ Win32API MIDI ]
 */

#if defined(_WIN32)
#if defined(MIDI)

#ifndef _w32_midi_h_
#define _w32_midi_h_

/*
 *	型定義
 */
typedef struct {
	DWORD		time;			/* コマンド発行VM時間 */
	int			length;			/* データバイト数 */
	DWORD		exclptr;		/* エクスクルーシブデータの開始ポイント */
	BYTE		data[4];		/* 送信データ */
} MIDISEND;

/*
 *	定数定義
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
 *	主要エントリ
 */
#ifdef __cplusplus
extern "C" {
#endif
void FASTCALL InitMIDI(void);
										/* 初期化 */
void FASTCALL CleanMIDI(void);
										/* クリーンアップ */
BOOL FASTCALL SelectMIDI(void);
										/* セレクト */
BOOL FASTCALL SelectCheckMIDI(char *device);
										/* セレクト状態チェック */

void FASTCALL MidiSendData(DWORD tick);
										/* データ出力 */

/*
 *	主要ワーク
 */
extern char szMidiDevice[256];
										/* MIDIデバイス名 */
extern int nMidiDelay;
										/* MIDI発音遅延時間 */
extern BOOL bMidiDelayMode;
										/* MIDI発音遅延モード */
#ifdef __cplusplus
}
#endif

#endif	/* _w32_midi_h_ */
#endif
#endif	/* _WIN32 */
