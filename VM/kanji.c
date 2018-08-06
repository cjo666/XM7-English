/*
 *	FM-7 EMULATOR "XM7"
 *
 *	Copyright (C) 1999-2017 ＰＩ．(Twitter:@xm6_original)
 *	Copyright (C) 2001-2017 Ryu Takegami (Twitter:@RyuTakegami)
 *
 *	[ 漢字ROM (非漢字・第一水準・第二水準) ]
 *
 *	RHG履歴
 *	  2001.07.23(爆)	V2での漢字ROM用メモリ確保量を128KBに戻した
 *	  2002.05.06		アドレスレジスタ($FD20/$FD21/$FD2C/$FD2D)をWrite Only
 *						に変更
 *	  2002.11.01		データ読み出し処理をなんとなく変更
 *	  2003.11.21		$FD04 bit5(漢字ROM接続の切り換え機能)に対応
 *	  2006.04.27		日本語通信カード用処理を追加
 *	  2006.11.08		FM-8モードで日本語通信カード処理が無効になるように変更
 *	  2008.01.24		日本語サブシステムI/F部分をjsubsys.cに切り離す
 *	  2010.07.28		V1においてKANJI.ROMがない場合の処理を変更
 *	  2010.12.09		FM77AV40/20系から抽出したJIS83準拠漢字ROMデータにおいて
 *						FM-8/7/77/FM77AVモードのJIS78準拠エミュレーションを追加
 *	  2011.05.30		動作モード別にJIS78準拠フォントとJIS83準拠フォントの同
 *						居ができるよう変更
 *	  2012.05.28		V1/V2においてKANJI.ROMとKANJI1.ROMの読み込み優先順位を
 *						修正
 *						V1においてKANJI.ROMが読み込めない問題を修正
 *	  2012.05.29		これ以上あれこれ言われないようにkanji_rom_modeをstatic
 *						宣言に変更(ほかのソースから参照していないので)
 *	  2012.07.05		V3においてKANJI.ROMがない場合にFM-7/FM77AVモード時に漢
 *						字が表示できない問題を修正
 *						V3においてJIS78エミュレーションの字体変更についても対応
 *	  2012.07.06		V2においてもJIS78エミュレーションの字体変更について対応
 */

#include <stdlib.h>
#include <string.h>
#include "xm7.h"
#include "kanji.h"
#include "subctrl.h"
#include "device.h"
#if XM7_VER >= 2
#include "jcard.h"
#endif

/*
 *	グローバル ワーク
 */
WORD kanji_addr;						/* アドレスレジスタ(共通) */
BYTE *kanji_rom;						/* 第１水準ROM */
BYTE *kanji_rom2;						/* 第２水準ROM */
#if XM7_VER >= 3
BYTE *kanji_rom_jis78;					/* 第１水準ROM (JIS78準拠) */
#endif
BOOL kanji_asis_flag;					/* JIS78エミュレーション無効化フラグ */

/*
 *	スタティック ワーク
 */
static BOOL kanji_rom_available;		/* 漢字ROM使用可能フラグ */

/*
 *	プロトタイプ宣言
 */
static void FASTCALL kanji_make_jis78(BYTE *rom);


/*
 *	漢字ROM
 *	初期化
 */
BOOL FASTCALL kanji_init(void)
{
	/* 初期化 */
	kanji_rom_available = FALSE;

	/* メモリ確保 */
#if XM7_VER >= 3
	kanji_rom = (BYTE *)malloc(0x60000);
#else
	kanji_rom = (BYTE *)malloc(0x40000);
#endif
	if (!kanji_rom) {
		return FALSE;
	}

	/* 第1水準(JIS78) ファイル読み込み */
#if XM7_VER >= 3
	kanji_rom_jis78 = (BYTE *)(kanji_rom + 0x40000);
	if (file_load(KANJI_ROM_J78, kanji_rom_jis78, 0x20000)) {
#else
	if (file_load(KANJI_ROM_J78, kanji_rom, 0x20000)) {
#endif
		kanji_rom_available = TRUE;
	}

	/* 第1水準(JIS83) ファイル読み込み */
#if XM7_VER >= 3
	if (!file_load(KANJI_ROM, kanji_rom, 0x20000)) {
		return FALSE;
	}
#else
	if (!kanji_rom_available) {
		if (file_load(KANJI_ROM, kanji_rom, 0x20000)) {
			kanji_rom_available = TRUE;
		}
#if XM7_VER >= 2
		else {
			return FALSE;
		}
#endif
	}
#endif

	/* 第2水準 ファイル読み込み */
	kanji_rom2 = (BYTE *)(kanji_rom + 0x20000);
	if (!file_load(KANJI_ROM2, kanji_rom2, 0x20000)) {
#if XM7_VER >= 3
		return FALSE;
#else
#if XM7_VER == 2
		jcard_available = FALSE;
#endif
		kanji_rom2 = NULL;
#endif
	}

#if XM7_VER >= 3
	/* 漢字ROMエミュレーションをJIS78フォントの有無で判断する */
	if (!kanji_rom_available) {
		memcpy(kanji_rom_jis78, kanji_rom, 0x20000);

		/* JIS78準拠漢字ROMデータ生成 */
		kanji_make_jis78(kanji_rom_jis78);
	}
#else
	if (kanji_rom_available) {
		/* 強制的にJIS78漢字ROMエミュレーションを行う */
		kanji_make_jis78(kanji_rom);
	}
#endif

	return TRUE;
}

/*
 *	漢字ROM
 *	クリーンアップ
 */
void FASTCALL kanji_cleanup(void)
{
	ASSERT(kanji_rom);
	if (kanji_rom) {
		free(kanji_rom);
		kanji_rom = NULL;
		kanji_rom2 = NULL;
#if XM7_VER >= 3
		kanji_rom_jis78 = NULL;
#endif
	}
}

/*
 *	漢字ROM
 *	リセット
 */
void FASTCALL kanji_reset(void)
{
	kanji_addr = 0;
}

/*
 *	漢字ROM
 *	１バイト読み出し
 */
BOOL FASTCALL kanji_readb(WORD addr, BYTE *dat)
{
	int offset;

	switch (addr) {
		/* アドレス(第1・第2共通)上位・下位 */
		case 0xfd20:
		case 0xfd21:
#if XM7_VER >= 3
		case 0xfd2c:
		case 0xfd2d:
#endif
			*dat = 0xff;
			return TRUE;

		/* 第1水準データ */
		case 0xfd22:		/* 第1データLEFT */
		case 0xfd23:		/* 第1データRIGHT */
#if XM7_VER == 1
			if (!kanji_rom_available) {
				*dat = 0xff;
				return TRUE;
			}
#endif

#if XM7_VER >= 3 || (XM7_VER == 1 && defined(L4CARD))
			if (subkanji_flag) {
				*dat = 0xff;
				return TRUE;
			}
#endif

			offset = kanji_addr << 1;
			if ((fm7_ver <= 1) && (offset >= 0x6000) && (offset < 0x8000) &&
				!kanji_asis_flag) {
				/* FM-7モード時の$6000～$7FFFは未定義領域 */
				*dat = (BYTE)(addr & 1);
			}
			else {
				/* 通常領域 */
#if XM7_VER >= 3
				if (fm7_ver >= 3) {
					*dat = kanji_rom[offset + (addr & 1)];
				}
				else {
					*dat = kanji_rom_jis78[offset + (addr & 1)];
				}
#else
				*dat = kanji_rom[offset + (addr & 1)];
#endif
			}
			return TRUE;

#if XM7_VER >= 2
		/* 第2水準データ */
		case 0xfd2e:		/* 第2データLEFT */
		case 0xfd2f:		/* 第2データRIGHT */
#if XM7_VER >= 3
			if ((fm7_ver < 2) || subkanji_flag) {
				*dat = 0xff;
				return TRUE;
			}
			if ((fm7_ver == 2) && !jcard_enable) {
				*dat = 0xff;
				return TRUE;
			}
#elif XM7_VER == 2
			if (fm7_ver < 2) {
				*dat = 0xff;
				return TRUE;
			}
			if (!jcard_available || !jcard_enable) {
				*dat = 0xff;
				return TRUE;
			}
#endif
			offset = kanji_addr << 1;
			*dat = kanji_rom2[offset + (addr & 1)];
			return TRUE;
#endif
	}

	return FALSE;
}

/*
 *	漢字ROM
 *	１バイト書き込み
 */
BOOL FASTCALL kanji_writeb(WORD addr, BYTE dat)
{
	switch (addr) {
		/* アドレス(第1・第2共通)上位 */
#if XM7_VER >= 2
		case 0xfd2c:
			if (fm7_ver < 2) {
				return TRUE;
			}
#if XM7_VER == 2
			if (!jcard_available || !jcard_enable) {
				return TRUE;
			}
			if ((fm7_ver == 2) && !jcard_enable) {
				return TRUE;
			}
#endif
#endif
		case 0xfd20:
			kanji_addr &= 0x00ff;
			kanji_addr |= (WORD)(dat << 8);
			return TRUE;

		/* アドレス(第1・第2共通)下位 */
#if XM7_VER >= 2
		case 0xfd2d:
			if (fm7_ver < 2) {
				return TRUE;
			}
#if XM7_VER == 2
			if (!jcard_available || !jcard_enable) {
				return TRUE;
			}
			if ((fm7_ver == 2) && !jcard_enable) {
				return TRUE;
			}
#endif
#endif
		case 0xfd21:
			kanji_addr &= 0xff00;
			kanji_addr |= dat;
			return TRUE;

		/* データ */
		case 0xfd22:
		case 0xfd23:
#if XM7_VER >= 2
		case 0xfd2f:
#endif
			return TRUE;

#if XM7_VER >= 2
		/* 日本語カード バンクセレクト */
		case 0xfd2e:
			if (fm7_ver < 2) {
				return TRUE;
			}
#if XM7_VER == 2
			if (!jcard_available || !jcard_enable) {
				return TRUE;
			}
#elif XM7_VER >= 3
			if ((fm7_ver == 2) && !jcard_enable) {
				return TRUE;
			}
#endif

			/* bit7:学習RAM */
			if (dat & 0x80) {
				dicram_en = TRUE;
			}
			else {
				dicram_en = FALSE;
			}

			/* bit6:辞書ROMイネーブル */
			if (dat & 0x40) {
				dicrom_en = TRUE;
			}
			else {
				dicrom_en = FALSE;
			}

			/* bit0-5:辞書ROMバンク */
			dicrom_bank = (BYTE)(dat & 0x3f);
			return TRUE;
#endif
	}

	return FALSE;
}

/*
 *	漢字ROM
 *	JIS78準拠領域作成
 */
static void FASTCALL kanji_make_jis78(BYTE *rom)
{
	/* JIS78準拠未定義文字テーブル(初代FM77AV準拠) */
	static const DWORD jis78_table[] = {
		0xffffffff, 0x00000001, 0xffff8001, 0xfc00ffff,
		0x00000001, 0x00000001, 0xfe000001, 0x00000001,
		0xffffffff, 0x80000000, 0xffffffff, 0xf8000001,
		0xfff00000, 0xff800000, 0xffffffff, 0xfffc0000,
		0xffffffff, 0x00000000, 0xffffffff, 0xf8000001,
		0x00000000, 0x00000000, 0xfe000001, 0x0001fffc,
	};

	/* JIS78第1水準⇔JIS83第2水準入替漢字テーブル */
	static const DWORD convert_table[][2] = {
		/* 第1水準code addr		  第2水準code addr */
		{/*鯵 0x3033*/0x4130,	/*鰺 0x724D*/0xE4D0},
		{/*鴬 0x3229*/0x4490,	/*鶯 0x7274*/0xD540},
		{/*蛎 0x3342*/0x6620,	/*蠣 0x695A*/0x93A0},
		{/*撹 0x3349*/0x6690,	/*攪 0x5978*/0x5380},
		{/*竃 0x3376*/0x8760,	/*竈 0x635E*/0x87E0},
		{/*潅 0x3443*/0x6830,	/*灌 0x5E75*/0x5D50},
		{/*諌 0x3452*/0x6920,	/*諫 0x6B5D*/0x97D0},
		{/*頚 0x375B*/0x6FB0,	/*頸 0x7074*/0xD140},
		{/*砿 0x395C*/0x73C0,	/*礦 0x6268*/0xA480},
		{/*蕊 0x3C49*/0x7890,	/*蘂 0x6922*/0x7220},
		{/*靭 0x3F59*/0x7F90,	/*靱 0x7057*/0xE170},
		{/*賎 0x4128*/0xA280,	/*賤 0x6C4D*/0x98D0},
		{/*壷 0x445B*/0xC9B0,	/*壺 0x5464*/0x4840},
		{/*砺 0x4557*/0xCB70,	/*礪 0x626A*/0xA4A0},
		{/*梼 0x456E*/0xEAE0,	/*檮 0x5B6D*/0x56D0},
		{/*涛 0x4573*/0xEB30,	/*濤 0x5E39*/0x1D90},
		{/*迩 0x4676*/0xED60,	/*邇 0x6D6E*/0xBAE0},
		{/*蝿 0x4768*/0xEE80,	/*蠅 0x6A24*/0x7440},
		{/*桧 0x4930*/0xB300,	/*檜 0x5B58*/0x3780},
		{/*侭 0x4B79*/0xF790,	/*儘 0x5056*/0x2160},
		{/*薮 0x4C79*/0xF990,	/*藪 0x692E*/0x72E0},
		{/*篭 0x4F36*/0xBF60,	/*籠 0x6446*/0x8860},
		{/*尭 0x3646*/0x6C60,	/*堯 0x7421*/0xC810},
		{/*槙 0x4B6A*/0xF6A0,	/*槇 0x7422*/0xC820},
		{/*遥 0x4D5A*/0xDBA0,	/*遙 0x7423*/0xC830},
		{0, 0},
	};

	DWORD addr;
	DWORD i;

	/* パッチ当て不要フラグが立っている場合は処理をスキップ */
	if (kanji_asis_flag) {
		return;
	}

	/* JIS78準拠漢字ROMでは未定義になっている部分を未定義とする */
	for (addr = 0; addr < 0x6000; addr += 32) {
		if (jis78_table[addr >> 10] & (1 << (addr >> 5))) {
			/* 未定義フォント作成 */
			for (i = 0; i < 32; i += 2) {
				rom[addr + i + 0] = 0x00;
				rom[addr + i + 1] = 0x01;
			}
		}
	}

#if XM7_VER <= 2
	/* KANJI2.ROMがない場合はこのまま帰る */
	if (!kanji_rom2) {
		return;
	}
#endif

	/* JIS83で入れ替えられた漢字を第2水準漢字ROMから上書きする */
	for (addr = 0; convert_table[addr][0]; addr ++) {
		/* フォントコピー */
		for (i = 0; i < 32; i ++) {
			rom[convert_table[addr][0] * 2 + i] =
				kanji_rom2[convert_table[addr][1] * 2 + i];
		}
	}
}

/*
 *	漢字ROM
 *	セーブ
 */
BOOL FASTCALL kanji_save(int fileh)
{
	if (!file_word_write(fileh, kanji_addr)) {
		return FALSE;
	}

	return TRUE;
}

/*
 *	漢字ROM
 *	ロード
 */
BOOL FASTCALL kanji_load(int fileh, int ver)
{
	/* バージョンチェック */
	if (ver < 200) {
		return FALSE;
	}

	if (!file_word_read(fileh, &kanji_addr)) {
		return FALSE;
	}

	return TRUE;
}
