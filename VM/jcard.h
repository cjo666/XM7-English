/*
 *	FM-7 EMULATOR "XM7"
 *
 *	Copyright (C) 1999-2017 ＰＩ．(Twitter:@xm6_original)
 *	Copyright (C) 2001-2017 Ryu Takegami (Twitter:@RyuTakegami)
 *
 *	[ 日本語カード / 拡張サブシステムROM ]
 */

#ifndef _jcard_h_
#define _jcard_h_

#if XM7_VER >= 2

#ifdef __cplusplus
extern "C" {
#endif
/*
 *	主要エントリ
 */
BOOL FASTCALL jcard_init(void);
										/* 初期化 */
void FASTCALL jcard_cleanup(void);
										/* クリーンアップ */
void FASTCALL jcard_reset(void);
										/* リセット */
BYTE FASTCALL jcard_readb(WORD addr);
										/* メモリ読み出し */
void FASTCALL jcard_writeb(WORD addr, BYTE dat);
										/* メモリ書き込み */
BOOL FASTCALL jcard_save(int fileh);
										/* セーブ */
BOOL FASTCALL jcard_load(int fileh, int ver);
										/* ロード */

/*
 *	メモリ (FM77AV40・日本語カード部)
 */
extern BYTE *extram_b;
										/* 拡張RAM         $10000 */
extern BYTE *dicrom;
										/* 辞書ROM         $40000 */
#if XM7_VER >= 3
extern BYTE *extrom;
										/* 拡張ROM(EX/SX)  $20000 */
#endif
extern BYTE *dicram;
										/* 学習RAM          $2000 */

/*
 *	主要ワーク
 */
extern BYTE dicrom_bank;
										/* 辞書ROMバンク番号 */
extern BOOL dicrom_en;
										/* 辞書ROMアクティブ */
extern BOOL dicram_en;
										/* 学習RAMアクティブ */
#if XM7_VER >= 3
extern BOOL extrom_sel;
										/* 辞書ROM/拡張ROM選択フラグ */
#endif
#if XM7_VER == 2
extern BOOL jcard_available;
										/* 日本語カード使用可否フラグ */
#endif
extern BOOL jcard_enable;
										/* 日本語カード有効フラグ */
#ifdef __cplusplus
}
#endif

#endif	/* XM7_VER >= 3 */
#endif	/* _jcard_h_ */
