/*
 *	FM-7 EMULATOR "XM7"
 *
 *	Copyright (C) 1999-2017 ＰＩ．(Twitter:@xm6_original)
 *	Copyright (C) 2001-2017 Ryu Takegami (Twitter:@RyuTakegami)
 *
 *	[ MIDIアダプタ (◎h!FM掲載MIDIカード?) ]
 */

#ifndef _midi_h_
#define _midi_h_

#if defined(MIDI)

#ifdef __cplusplus
extern "C" {
#endif
/*
 *	主要エントリ
 */
BOOL FASTCALL midi_init(void);
										/* 初期化 */
void FASTCALL midi_cleanup(void);
										/* クリーンアップ */
void FASTCALL midi_reset(void);
										/* リセット */
BOOL FASTCALL midi_readb(WORD addr, BYTE *dat);
										/* メモリ読み出し */
BOOL FASTCALL midi_writeb(WORD addr, BYTE dat);
										/* メモリ書き込み */
BOOL FASTCALL midi_save(int fileh);
										/* セーブ */
BOOL FASTCALL midi_load(int fileh, int ver);
										/* ロード */

/*
 *	主要ワーク
 */
extern BOOL midi_busy;
										/* MIDIバッファオーバーフロー */
#ifdef __cplusplus
}
#endif

#endif	/* MIDI */
#endif	/* _midi_h_ */
