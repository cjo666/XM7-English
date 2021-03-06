/*
 *	FM-7 EMULATOR "XM7"
 *
 *	Copyright (C) 1999-2017 ＰＩ．(Twitter:@xm6_original)
 *	Copyright (C) 2001-2017 Ryu Takegami (Twitter:@RyuTakegami)
 *
 *	[ MIDIアダプタ(◎h!FM掲載MIDIカード?) ]
 *
 *	RHG履歴
 *	  2002.09.26		新設
 */

#if defined(MIDI)

#include <string.h>
#include "xm7.h"
#include "midi.h"
#include "device.h"
#include "rs232c.h"

/*
 *	グローバル ワーク
 */
BOOL midi_busy;
BOOL midi_txenable;
BOOL midi_selectmc;
BYTE midi_modereg;


/*
 *	MIDIアダプタ
 *	初期化
 */
BOOL FASTCALL midi_init(void)
{
	midi_busy = FALSE;

	return TRUE;
}

/*
 *	MIDIアダプタ
 *	クリーンアップ
 */
void FASTCALL midi_cleanup(void)
{
}

/*
 *	MIDIアダプタ
 *	リセット
 */
void FASTCALL midi_reset(void)
{
	midi_txenable = FALSE;
	midi_selectmc = TRUE;
	midi_modereg = 0xff;

	midi_reset_notify();
}

/*
 *	MIDIアダプタ
 *	１バイト読み出し
 */
BOOL FASTCALL midi_readb(WORD addr, BYTE *dat)
{
#if XM7_VER == 1
	if (fm_subtype == FMSUB_FM8) {
		return FALSE;
	}
#endif

	switch (addr) {
		case 0xfdea:	/* USART DATA */
			/* MIDI INには非対応 */
			/* 読み出し結果を変更 (kaikiraw…のbug) */
			*dat = 0xff;
			return TRUE;

		case 0xfdeb:	/* USART STATUS */
			if (midi_busy) {
				*dat = 0x02;
			}
			else {
				*dat = 0x07;
			}
			return TRUE;
	}

	return FALSE;
}

/*
 *	MIDIアダプタ
 *	１バイト書き込み
 */
BOOL FASTCALL midi_writeb(WORD addr, BYTE dat)
{
#if XM7_VER == 1
	if (fm_subtype == FMSUB_FM8) {
		return FALSE;
	}
#endif

	switch (addr) {
		case 0xfdea:	/* USART DATA */
			/* TxE=1 かつモードコマンドレジスタ設定値が正常な場合に送信 */
			if (midi_txenable && (midi_modereg == 0x4e)) {
				midi_notify(dat);
			}
			return TRUE;

		case 0xfdeb:	/* USART COMMAND */
			if (midi_selectmc) {
				/* モードコマンドレジスタ */
				midi_modereg = dat;
				midi_selectmc = FALSE;
			}
			else {
				/* コマンドレジスタ */

				/* TXE */
				if (dat & RSC_TXEN) {
					midi_txenable = TRUE;
				}
				else {
					midi_txenable = FALSE;
				}

				/* 内部リセット */
				if (dat & RSC_IR) {
					midi_modereg = 0xff;
					midi_txenable = FALSE;
					midi_selectmc = TRUE;
				}
			}

			return TRUE;
	}

	return FALSE;
}

/*
 *	MIDIアダプタ
 *	セーブ
 */
BOOL FASTCALL midi_save(int fileh)
{
	if (!file_bool_write(fileh, midi_txenable)) {
		return FALSE;
	}
	if (!file_bool_write(fileh, midi_selectmc)) {
		return FALSE;
	}
	if (!file_byte_write(fileh, midi_modereg)) {
		return FALSE;
	}
	
	return TRUE;
}

/*
 *	MIDIアダプタ
 *	ロード
 */
BOOL FASTCALL midi_load(int fileh, int ver)
{
	/* バージョンチェック */
	if (ver < 200) {
		return FALSE;
	}
#if XM7_VER >= 3
	if (((ver >= 500) && (ver < 715)) || ((ver >= 800) && (ver < 915))) {
#elif XM7_VER >= 2
	if (ver < 715) {
#else
	if (ver < 305) {
#endif
		return TRUE;
	}

	if (!file_bool_read(fileh, &midi_txenable)) {
		return FALSE;
	}
	if (!file_bool_read(fileh, &midi_selectmc)) {
		return FALSE;
	}
	if (!file_byte_read(fileh, &midi_modereg)) {
		return FALSE;
	}
	
	return TRUE;
}

#endif	/* MIDI */
