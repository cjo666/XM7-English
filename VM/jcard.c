/*
 *	FM-7 EMULATOR "XM7"
 *
 *	Copyright (C) 1999-2017 ＰＩ．(Twitter:@xm6_original)
 *	Copyright (C) 2001-2017 Ryu Takegami (Twitter:@RyuTakegami)
 *
 *	[ 日本語カード / 拡張サブシステムROM ]
 *
 *	RHG履歴
 *	  2003.03.29		辞書・拡張ROMの読み出し処理を整理
 *	  2005.10.07		拡張ROMエリアの内容を読み込み時に展開するように変更
 *	  2010.12.09		拡張ROMエリアの漢字ROM領域をJIS78準拠に変更
 *	  2010.12.19		拡張ROMエリアのブート領域を隠しブートROM対応に変更
 *	  2012.06.29		拡張ROMエリアの漢字領域の漢字領域(?)をJIS78準拠に修正
 */

#if XM7_VER >= 2

#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "xm7.h"
#include "mmr.h"
#include "device.h"
#include "kanji.h"
#include "jcard.h"

/*
 *	グローバル ワーク
 */
BYTE *dicrom;							/* 辞書ROM $40000 */
BYTE *dicram;							/* 学習RAM $2000 */
BYTE *extram_b;							/* 拡張RAM $10000 */

BYTE dicrom_bank;						/* 辞書ROMバンク番号 */
#if XM7_VER == 3
BOOL extrom_sel;						/* 辞書ROM/拡張ROM選択フラグ */
#endif
BOOL dicrom_en;							/* 辞書ROMアクティブ */
BOOL dicram_en;							/* 学習RAMアクティブ */

#if XM7_VER == 3
BYTE *extrom;							/* 拡張ROM $20000 */
#endif

#if XM7_VER == 2
BOOL jcard_available;					/* 日本語カード使用可否フラグ */
#endif
BOOL jcard_enable;						/* 日本語カード有効フラグ */


/*
 *	日本語カード
 *	初期化
 */
BOOL FASTCALL jcard_init(void)
{
	/* ワークエリア初期化 */
	extram_b = NULL;
	dicrom = NULL;
	dicram = NULL;
#if XM7_VER >= 3
	extrom = NULL;
#endif
#if XM7_VER == 2
	jcard_available = TRUE;
#endif
	jcard_enable = FALSE;

	/* 日本語空間 拡張RAM */
	extram_b = (BYTE *)malloc(0x10000);
	if (extram_b == NULL) {
#if XM7_VER == 2
		jcard_available = FALSE;
		return TRUE;
#else
		return FALSE;
#endif
	}

	/* 辞書ROM */
	dicrom = (BYTE *)malloc(0x40000);
	if (dicrom == NULL) {
#if XM7_VER == 2
		jcard_available = FALSE;
		return TRUE;
#else
		return FALSE;
#endif
	}
	if (!file_load(DICT_ROM, dicrom, 0x40000)) {
#if XM7_VER == 2
		jcard_available = FALSE;
		return TRUE;
#else
		return FALSE;
#endif
	}

	/* 学習RAM読み込み */
	dicram = (BYTE *)malloc(0x2000);
	if (dicram == NULL) {
#if XM7_VER == 2
		jcard_available = FALSE;
		return TRUE;
#else
		return FALSE;
#endif
	}
	if (!file_load(DICT_RAM, dicram, 0x2000)) {
		/* ファイルが存在しない。初期化 */
		memset(dicram, 0xff, 0x2000);
	}

	/* 拡張ROM */
#if XM7_VER >= 3
	extrom = (BYTE *)malloc(0x20000);
	if (extrom == NULL) {
		return FALSE;
	}
	memset(extrom, 0xff, 0x20000);
	if (!file_load(EXTSUB_ROM, extrom, 0xc000)) {
		return FALSE;
	}
	else {
		/* バンク56～63にBASIC ROM、隠しブートROMの内容をコピー */
		memcpy(&extrom[0x18000], basic_rom, 0x7c00);
		if (available_mmrboot) {
			memcpy(&extrom[0x1fe00], boot_mmr, 0x200);
		}
		else {
			memcpy(&extrom[0x1fe00], &init_rom[0x1a00], 0x1e0);
			memset(&extrom[0x1ffe0], 0, 32);
			extrom[0x1fffe] = 0xfe;
			extrom[0x1ffff] = 0x00;
		}
	}
#endif

	return TRUE;
}

/*
 *	日本語カード
 *	クリーンアップ
 */
void FASTCALL jcard_cleanup(void)
{
	ASSERT(extram_b);
	ASSERT(dicram);
	ASSERT(dicrom);
#if XM7_VER >= 3
	ASSERT(extrom);
#endif

	/* 初期化途中で失敗した場合を考慮 */
#if XM7_VER >= 3
	if (extrom) {
		free(extrom);
	}
#endif
	if (dicram) {
		/* 学習RAMの内容をファイルに書き出す */
		file_save(DICT_RAM, dicram, 0x2000);
		free(dicram);
	}
	if (dicrom) {
		free(dicrom);
	}
	if (extram_b) {
		free(extram_b);
	}
}

/*
 *	日本語カード
 *	リセット
 */
void FASTCALL jcard_reset(void)
{
	dicrom_bank = 0;
	dicram_en = FALSE;
	dicrom_en = FALSE;
#if XM7_VER >= 3
	extrom_sel = FALSE;
#endif
}

/*
 *	日本語カード
 *	１バイト読み込み
 */
BYTE FASTCALL jcard_readb(WORD addr)
{
	DWORD dicrom_addr;

	/* FM77AVシリーズのみサポート */
	if (fm7_ver < 2) {
		return 0xff;
	}

#if XM7_VER == 2
	/* 日本語カードが使えない場合は0xffを返すだけ */
	if (!jcard_available || !jcard_enable) {
		return 0xff;
	}
#else
	/* 日本語カード無効時は0xffを返すだけ */
	if ((fm7_ver == 2) && !jcard_enable) {
		return 0xff;
	}
#endif

	/* $28000-$29FFF : 学習RAM */
	if ((addr >= 0x8000) && (addr < 0xa000)) {
		if (dicram_en) {
			return dicram[addr - 0x8000];
		}
	}

	/* $2E000-$2EFFF : 辞書ROM or 拡張ROM */
	if ((addr >= 0xe000) && (addr < 0xf000)) {
		/* 辞書ROMが有効か */
		if (dicrom_en) {
			addr &= (WORD)0x0fff;
			dicrom_addr = (dicrom_bank << 12);

#if XM7_VER >= 3
			/* 拡張ROMが有効か */
			if (extrom_sel) {
				/* バンク0～31 : 第1水準漢字(JIS78準拠) */
				if ((dicrom_bank >= 6) && (dicrom_bank < 8) &&
					!kanji_asis_flag) {
					return (BYTE)(addr & 1);
				}
				if (dicrom_bank < 32) {
					return kanji_rom_jis78[addr | dicrom_addr];
				}

				/* バンク32～43 : 拡張サブシステムROM(extsub.rom) */
				/* バンク56～63 : F-BASIC V3.0 ROM ($8000-$EFFF) */
				/* バンク63     : DOSモードBOOTっぽいもの ($FE00-$FFDF) */
				/* バンク63     : 割り込みベクタ領域 ($FFE0-$FFFF) */
				return extrom[addr | (dicrom_addr - 0x20000)];
			}
			else {
				/* 辞書ROM */
				return dicrom[addr | dicrom_addr];
			}
#else
			/* 辞書ROM */
			return dicrom[addr | dicrom_addr];
#endif
		}
	}

	/* 拡張RAM */
	return extram_b[addr];
}

/*
 *	日本語カード
 *	１バイト書き込み
 */
void FASTCALL jcard_writeb(WORD addr, BYTE dat)
{
	/* FM77AVシリーズのみサポート */
	if (fm7_ver < 2) {
		return;
	}

#if XM7_VER == 2
	/* 日本語カードが使えない場合は帰る */
	if (!jcard_available || !jcard_enable) {
		return;
	}
#else
	/* 日本語カード無効時は帰る */
	if ((fm7_ver == 2) && !jcard_enable) {
		return;
	}
#endif

	/* $28000-$29FFF : 学習RAM */
	if ((addr >= 0x8000) && (addr < 0xa000)) {
		if (dicram_en) {
			dicram[addr - 0x8000] = dat;
			return;
		}
	}

	/* 拡張RAM (辞書ROMの選択状態に関わらず書き込み可能) */
	extram_b[addr] = dat;
}

/*
 *	日本語カード
 *	セーブ
 */
BOOL FASTCALL jcard_save(int fileh)
{
	if (!file_byte_write(fileh, dicrom_bank)) {
		return FALSE;
	}
#if XM7_VER >= 3
	if (!file_bool_write(fileh, extrom_sel)) {
		return FALSE;
	}
#endif
	if (!file_bool_write(fileh, dicrom_en)) {
		return FALSE;
	}
	if (!file_bool_write(fileh, dicram_en)) {
		return FALSE;
	}
	if (!file_write(fileh, extram_b, 0x10000)) {
		return FALSE;
	}
	if (!file_bool_write(fileh, jcard_enable)) {
		return FALSE;
	}

	return TRUE;
}

/*
 *	日本語カード
 *	ロード
 */
BOOL FASTCALL jcard_load(int fileh, int ver)
{
	/* バージョンチェック */
	if (ver < 721) {
		dicrom_bank = 0;
#if XM7_VER >= 3
		extrom_sel = FALSE;
#endif
		dicram_en = FALSE;
		dicrom_en = FALSE;
		return TRUE;
	}

	if (!file_byte_read(fileh, &dicrom_bank)) {
		return FALSE;
	}
#if XM7_VER >= 3
	if (ver >= 800) {
		if (!file_bool_read(fileh, &extrom_sel)) {
			return FALSE;
		}
	}
#endif
	if (!file_bool_read(fileh, &dicrom_en)) {
		return FALSE;
	}
	if (!file_bool_read(fileh, &dicram_en)) {
		return FALSE;
	}
	if (!file_read(fileh, extram_b, 0x10000)) {
		return FALSE;
	}
#if XM7_VER >= 3
	if ((ver >= 921) || ((ver >= 721) && (ver <= 799))) {
#else
	if ((ver >= 721) && (ver <= 799)) {
#endif
		if (!file_bool_read(fileh, &jcard_enable)) {
			return FALSE;
		}
	}

	return TRUE;
}

#endif	/* XM7_VER >= 2 */
