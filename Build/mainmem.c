/*
 *	FM-7 EMULATOR "XM7"
 *
 *	Copyright (C) 1999-2017 ＰＩ．(Twitter:@xm6_original)
 *	Copyright (C) 2001-2017 Ryu Takegami (Twitter:@RyuTakegami)
 *
 *	[ メインCPUメモリ ]
 *
 *	RHG履歴
 *	  2002.03.11		I/Oアクセス時のメモリレディ動作を再現するようにした
 *	  2002.05.06		FM-7モード時のブートROM領域の扱いを(少しだけ ^^;)変更
 *	  2003.11.20		XM7 V1.1対応
 *	  2008.01.24		kanji.cからの日本語サブI/F切り離しに関する変更
 *	  2010.06.18		FM-77モード時に合わせてメインメモリ確保良を変更(V1のみ)
 *	  2010.07.28		V2/V3のFM77AVモード時にAV40/20以降のイニシエータROMを使
 *						用した際に、初代AV相当の動作として新ブートの転送禁止お
 *						よび旧ブートを利用するパッチをリセット時に動的に当てる
 *						ように変更 (V3の40EXモード時には新ブートを利用します)
 *						V2/V3の機種判別領域の設定を6バイト分行うように変更
 *	  2010.12.10		V3のBOOT_MMR.ROM対応化
 *	  2012.04.24		FM-NEW7初期ロットの新ブートROM(TL11-12)/FM-8初期ブート
 *						ROM(SM11-14)の高速起動パッチに対応
 *	  2013.12.13		FM-NEW7初期ロットの新ブートROM+F-BASIC V3.5という組み合
 *						わせ(出荷版ではあり得ない)でホットリセットができない事
 *						象に対処
 *	  2014.07.16		FM-7モードのみ、BASIC ROM領域に書き込んだ場合に裏RAMに
 *						書き込みが行われるように仕様変更 (なんかそうらしい)
 */

#include <stdlib.h>
#include <string.h>
#include "xm7.h"
#include "device.h"
#include "subctrl.h"
#include "ttlpalet.h"
#include "fdc.h"
#include "mainetc.h"
#include "multipag.h"
#include "kanji.h"
#include "tapelp.h"
#include "opn.h"
#include "mmr.h"
#include "apalet.h"
#include "rs232c.h"
#if defined(MIDI)
#include "midi.h"
#endif
#if XM7_VER >= 2
#include "jcard.h"
#endif
#if XM7_VER >= 3
#include "dmac.h"
#endif
#if XM7_VER == 1
#if defined(JSUB)
#include "jsubsys.h"
#endif
#if defined(BUBBLE)
#include "bubble.h"
#endif
#endif
#include "ptm.h"
#include "mouse.h"

/*
 *	グローバル ワーク
 */
BYTE *mainram_a;						/* RAM (表RAM)        $8000 */
BYTE *mainram_b;						/* RAM (裏RAM+α)     $7C80 */
BYTE *basic_rom;						/* ROM (F-BASIC)      $7C00 */
BYTE *main_io;							/* メインCPU I/O       $100 */
BOOL basicrom_en;						/* BASIC ROMイネーブルフラグ */
#if XM7_VER == 1
BYTE *basic_rom8;						/* ROM (F-BASIC V1.0) $7C00 */
BYTE *boot_bas;							/* ブート(BASIC,FM-7)  $200 */
BYTE *boot_dos;							/* ブート(DOS,FM-7)    $200 */
BYTE *boot_bas8;						/* ブート(BASIC,FM-8) $200 */
BYTE *boot_dos8;						/* ブート(DOS,FM-8)   $200 */
BYTE *boot_bbl8;						/* ブート(BBL,FM-8)   $200 */
BYTE *boot_bas_patch;					/* ブート(BASIC,NEW7p) $200 */
BYTE *boot_mmr;							/* ブート(隠し)        $200 */
#if defined(BUBBLE)
BOOL bubble_available;					/* バブル使用可能フラグ */
#endif
#endif
BYTE *boot_ram;							/* ブートRAM           $200 */
BOOL bootram_rw;						/* ブートRAM 書き込みフラグ */
BOOL rom_ram_write;						/* FM-7モード時ROM領域書込挙動変更 */

BYTE *extram_a;							/* 拡張RAM           $10000 */
#if XM7_VER >= 3
BYTE *extram_c;							/* AV40拡張RAM       $C0000 */
BYTE *boot_mmr;							/* ブート(隠し)        $200 */
#endif

#if XM7_VER >= 2
BYTE *init_rom;							/* イニシエータROM    $2000 */
BOOL initrom_en;						/* イニシエータROMイネーブルフラグ */
#if XM7_VER >= 3
BOOL init_is_exsx;						/* イニシエータROM EX/SXフラグ */
#endif
#endif


/*
 *	スタティック ワーク
 */
#if XM7_VER >= 2
static BYTE *patch_branewboottfr;		/* 新ブート転送用処理へのBRA命令 */
static BYTE *patch_jmpnewboot;			/* 新ブートへのJMP命令 */
#endif
static BYTE ioaccess_count;				/* I/O領域アクセスカウンタ */
static BOOL ioaccess_flag;				/* I/Oアクセスウェイト調整フラグ */

#if XM7_VER >= 3
/*
 *	新ブートへのパッチ1
 *	USART初期化(実体はパッチ2),論理ドライブ変更,IPLへのジャンプ
 */
static const BYTE bootpatch1[] = {
	0x0f, 0x10, 0xbd, 0x7f, 0x80, 0x86, 0x02, 0x97,
	0x10, 0x4f, 0xbc, 0xfb, 0xfe, 0x27, 0x19, 0xe6,
	0xe4, 0xc1, 0xff, 0x27, 0x13, 0xc1, 0x01, 0x23,
	0x0f, 0xcc, 0x00, 0xfd, 0x1f, 0x9b, 0x17, 0xfe,
	0x2b, 0x1f, 0x8b, 0x4c, 0xe6, 0xe4, 0xc0, 0x02,
	0x1f, 0x8b, 0x6e, 0x84,
};

/*
 *	新ブートへのパッチ2
 *	USART初期化処理(パッチ1からの続き)
 */
static const BYTE bootpatch2[] = {
	0xce, 0xfd, 0x06, 0x8d, 0x0d, 0xce, 0xfd, 0x24,
	0x86, 0x04, 0x8d, 0x06, 0x33, 0x42, 0x4a, 0x26,
	0xf9, 0x39, 0x6f, 0x41, 0x6f, 0x41, 0x6f, 0x41,
	0xc6, 0x40, 0xe7, 0x41, 0x39,
};
#endif


/*
 *	メインCPUメモリ
 *	初期化
 */
BOOL FASTCALL mainmem_init(void)
{
	int i;
	BYTE *p;

	/* 一度、全てクリア */
	mainram_a = NULL;
	mainram_b = NULL;
	basic_rom = NULL;
	main_io = NULL;
	extram_a = NULL;
#if XM7_VER == 1
	boot_bas = NULL;
	boot_dos = NULL;
	boot_mmr = NULL;
	boot_bas_patch = NULL;
	boot_bas8 = NULL;
	boot_dos8 = NULL;
	boot_bbl8 = NULL;
#if defined(BUBBLE)
	bubble_available = TRUE;
#endif
#else
	patch_branewboottfr = NULL;
	patch_jmpnewboot = NULL;
#if XM7_VER >= 3
	boot_mmr = NULL;
	extram_c = NULL;
#endif
	init_rom = NULL;
#endif
	boot_ram = NULL;
	rom_ram_write = TRUE;

	/* RAM */
	mainram_a = (BYTE *)malloc(0x8000);
	if (mainram_a == NULL) {
		return FALSE;
	}
#if XM7_VER == 1
	mainram_b = (BYTE *)malloc(0x7e00);
#else
	mainram_b = (BYTE *)malloc(0x7c80);
#endif
	if (mainram_b == NULL) {
		return FALSE;
	}

	/* BASIC ROM, I/O */
	basic_rom = (BYTE *)malloc(0x7c00);
	if (basic_rom == NULL) {
		return FALSE;
	}
#if XM7_VER == 1
	basic_rom8 = (BYTE *)malloc(0x7c00);
	if (basic_rom8 == NULL) {
		return FALSE;
	}
#endif
	main_io = (BYTE *)malloc(0x0100);
	if (main_io == NULL) {
		return FALSE;
	}

	/* 拡張RAM、イニシエータROM */
#if XM7_VER >= 2
	extram_a = (BYTE *)malloc(0x10000);
	if (extram_a == NULL) {
		return FALSE;
	}
#if XM7_VER >= 3
	extram_c = (BYTE *)malloc(0xc0000);
	if (extram_c == NULL) {
		return FALSE;
	}
#endif
	init_rom = (BYTE *)malloc(0x2000);
	if (init_rom == NULL) {
		return FALSE;
	}
#else
	/* V1 (400ラインセット，192KB) */
	extram_a = (BYTE *)malloc(0x30000);
	if (extram_a == NULL) {
		return FALSE;
	}
#endif

	/* ブートROM/RAM */
#if XM7_VER == 1
	boot_bas = (BYTE *)malloc(0x200);
	if (boot_bas == NULL) {
		return FALSE;
	}
	boot_dos = (BYTE *)malloc(0x200);
	if (boot_dos == NULL) {
		return FALSE;
	}
	boot_bas_patch = (BYTE *)malloc(0x200);
	if (boot_bas_patch == NULL) {
		return FALSE;
	}
	boot_bas8 = (BYTE *)malloc(0x200);
	if (boot_bas8 == NULL) {
		return FALSE;
	}
	boot_dos8 = (BYTE *)malloc(0x200);
	if (boot_dos8 == NULL) {
		return FALSE;
	}
	boot_bbl8 = (BYTE *)malloc(0x200);
	if (boot_bbl8 == NULL) {
		return FALSE;
	}
#endif
#if (XM7_VER == 1) || (XM7_VER >= 3)
	boot_mmr = (BYTE *)malloc(0x200);
	if (boot_mmr == NULL) {
		return FALSE;
	}
#endif
	boot_ram = (BYTE *)malloc(0x200);
	if (boot_ram == NULL) {
		return FALSE;
	}

	/* ROMファイル読み込み */
	if (!file_load(FBASIC_ROM, basic_rom, 0x7c00)) {
#if XM7_VER == 1
		available_fm7roms = FALSE;
#else
		return FALSE;
#endif
	}
#if XM7_VER == 1
	if (!file_load(FBASIC10_ROM, basic_rom8, 0x7c00)) {
		available_fm8roms = FALSE;
	}
#endif

#if XM7_VER >= 2
	if (!file_load(INITIATE_ROM, init_rom, 0x2000)) {
		return FALSE;
	}

	/* 旧ブート高速起動(AVシリーズ共通) */
	for (i=0; i<2; i++) {
		p = &init_rom[0x1800 + i * 0x200];
		if (p[0x14f] == 0x26) {
			p[0x14f] = 0x21;
		}
		if (p[0x153] == 0x26) {
			p[0x153] = 0x21;
		}
	}

#if XM7_VER >= 3
	/* 新ブート高速起動・ドライブ対応変更抑制(AV20EX/AV40EX/AV40SX) */
	p = &init_rom[0x1c00];
	if ((p[0x8f] == 0x10) && (p[0x90] == 0x23)) {
		/* 先頭2バイトしか見てないけど、まぁいいか… */
		memcpy(&p[0x0247], bootpatch1, sizeof(bootpatch1));
		memcpy(&p[0x0380], bootpatch2, sizeof(bootpatch2));
		p[0x08f] = 0x16;
		p[0x090] = 0x01;
		p[0x091] = 0xb5;
		if (p[0x166] == 0x27) {
			p[0x166] = 0x21;
		}
		if (p[0x1d6] == 0x26) {
			p[0x1d6] = 0x21;
		}
	}

	/* 新ブート高速起動・ドライブ対応変更抑制(AV40/AV20) */
	if ((p[0x74] == 0x10) && (p[0x75] == 0x23)) {
		/* これも先頭2バイトしか見てないけど、まぁいいか… */
		memcpy(&p[0x0207], bootpatch1, sizeof(bootpatch1));
		memcpy(&p[0x0380], bootpatch2, sizeof(bootpatch2));
		p[0x074] = 0x16;
		p[0x075] = 0x01;
		p[0x076] = 0x90;
		p[0x227] = 0x50;	/* パッチのAV20EX/40EX/40SXとの違いはこれだけ */
		if (p[0xf3] == 0x27) {
			p[0xf3] = 0x21;
		}
		if (p[0x196] == 0x26) {
			p[0x196] = 0x21;
		}
	}
#endif

	/* 新ブート関連処理のアドレスを検索・記憶 */
	for (i=0; i<0xb00; i++) {
		/* 新ブート転送処理へのBRA命令 */
		if (!patch_branewboottfr) {
			if ((init_rom[i + 0] == 0x20) && (init_rom[i + 1] == 0xd7)) {
				patch_branewboottfr = &init_rom[i];
			}
		}

		/* 新ブートへのJMP命令(イニシエータの末尾に存在する) */
		if (!patch_jmpnewboot) {
			if ((init_rom[i + 0] == 0x7e) && (init_rom[i + 1] == 0x50) &&
				(init_rom[i + 2] == 0x00)) {
				patch_jmpnewboot = &init_rom[i];
			}
		}

		/* 両方発見したらループを抜ける */
		if (patch_branewboottfr && patch_jmpnewboot) {
			break;
		}
	}
#if XM7_VER >= 3
	if (!file_load(BOOTMMR_ROM, boot_mmr, 0x200)) {
		available_mmrboot = FALSE;
	}
#endif
#else
	if (!file_load(BOOTBAS_ROM, boot_bas, 0x1e0)) {
		available_fm7roms = FALSE;
	}
	if (!file_load(BOOTDOS_ROM, boot_dos, 0x1e0)) {
		available_fm7roms = FALSE;
	}
	if (!file_load(BOOTMMR_ROM, boot_mmr, 0x1e0)) {
		available_mmrboot = FALSE;
	}
	if (!file_load(BOOTBAS8_ROM, boot_bas8, 0x1e0)) {
		available_fm8roms = FALSE;
	}
	if (!file_load(BOOTDOS8_ROM, boot_dos8, 0x1e0)) {
		available_fm8roms = FALSE;
	}
#if defined(BUBBLE)
	if (!file_load(BOOTBBL8_ROM, boot_bbl8, 0x1e0)) {
		bubble_available = FALSE;
	}
#endif

	/* 旧ブート高速起動(FM-7/NEW7(1st lot)/8) */
	for (i=0; i<6; i++) {
		switch (i) {
			case 0:		p = boot_bas;
						break;
			case 1:		p = boot_dos;
						break;
			case 2:		p = boot_mmr;
						break;
			case 3:		p = boot_bas8;
						break;
			case 4:		p = boot_dos8;
						break;
			case 5:		p = boot_bbl8;
						break;
			default:	ASSERT(FALSE);
						break;
		}

		if (p[0x14f] == 0x26) {
			p[0x14f] = 0x21;
		}
		if (p[0x153] == 0x26) {
			p[0x153] = 0x21;
		}
		if (p[0x161] == 0x26) {
			p[0x161] = 0x21;
		}
		if (p[0x155] == 0x26) {
			p[0x155] = 0x21;
		}
		if (p[0x16b] == 0x26) {
			p[0x16b] = 0x21;
		}
		if (p[0x165] == 0x26) {
			p[0x165] = 0x21;
		}
		if (p[0x155] == 0x26) {
			p[0x155] = 0x21;
		}
		p[0x1fe] = 0xfe;
		p[0x1ff] = 0x00;
	}

	/* FM-NEW7初期ロットのブートROM+F-BASIC V3.5でのホットリセット対処 */
	memcpy(boot_bas_patch, boot_bas, 0x200);
	if ((boot_bas_patch[0x01d] == 0x96) && (boot_bas_patch[0x01e] == 0x0f)) {
		boot_bas_patch[0x01d] = 0x12;
		boot_bas_patch[0x01e] = 0x12;
	}
#endif

	/* ブートRAMクリア */
	/* (女神転生対策・リセット時に全領域を初期化しなくなったため) */
	memset(boot_ram, 0, 0x200);

#if XM7_VER >= 3
	/* EX/SXのイニシエータROMかチェックする */
	if (init_rom[0xb10] == '1') {
		init_is_exsx = TRUE;
	}
	else {
		init_is_exsx = FALSE;
	}
#endif

#if XM7_VER == 1
	/* FM-7/FM-8どちらのROMも不完全な場合、初期化失敗とする */
if (!available_fm8roms && !available_fm7roms) {
		return FALSE;
	}
#endif

	return TRUE;
}

/*
 *	メインCPUメモリ
 *	クリーンアップ
 */
void FASTCALL mainmem_cleanup(void)
{
	ASSERT(mainram_a);
	ASSERT(mainram_b);
	ASSERT(basic_rom);
	ASSERT(main_io);
	ASSERT(extram_a);
#if XM7_VER >= 3
	ASSERT(extram_c);
#endif
#if XM7_VER >= 2
	ASSERT(init_rom);
#endif

#if XM7_VER == 1
	ASSERT(basic_rom8);
	ASSERT(boot_bas);
	ASSERT(boot_dos);
	ASSERT(boot_bas8);
	ASSERT(boot_dos8);
	ASSERT(boot_bbl8);
#endif
#if (XM7_VER == 1) || (XM7_VER >= 3)
	ASSERT(boot_mmr);
#endif
	ASSERT(boot_ram);

	/* 初期化途中で失敗した場合を考慮 */
	if (mainram_a) {
		free(mainram_a);
	}
	if (mainram_b) {
		free(mainram_b);
	}
	if (basic_rom) {
		free(basic_rom);
	}
	if (main_io) {
		free(main_io);
	}

	if (extram_a) {
		free(extram_a);
	}
#if XM7_VER >= 3
	if (extram_c) {
		free(extram_c);
	}
#endif
#if XM7_VER >= 2
	if (init_rom) {
		free(init_rom);
	}
#endif

#if XM7_VER == 1
	if (basic_rom8) {
		free(basic_rom8);
	}
	if (boot_bas) {
		free(boot_bas);
	}
	if (boot_dos) {
		free(boot_dos);
	}
	if (boot_bas_patch) {
		free(boot_bas_patch);
	}
	if (boot_bas8) {
		free(boot_bas8);
	}
	if (boot_dos8) {
		free(boot_dos8);
	}
	if (boot_bbl8) {
		free(boot_bbl8);
	}
#endif
#if (XM7_VER == 1) || (XM7_VER >= 3)
	if (boot_mmr) {
		free(boot_mmr);
	}
#endif
	if (boot_ram) {
		free(boot_ram);
	}
}

/*
 *	メインCPUメモリ
 *	リセット
 */
void FASTCALL mainmem_reset(void)
{
	/* I/O空間・I/Oアクセスカウンタ初期化 */
	memset(main_io, 0xff, 0x0100);
	ioaccess_count = 0;
	ioaccess_flag = FALSE;

	/* BASICモードであれば、F-BASIC ROMをイネーブル */
	if (boot_mode == BOOT_BASIC) {
		basicrom_en = TRUE;
	}
	else {
		basicrom_en = FALSE;
	}

#if XM7_VER >= 2
	if (fm7_ver >= 2) {
		/* AV/40EXモード : イニシエータON、ブートRAM書き込み可 */
		initrom_en = TRUE;
		bootram_rw = TRUE;
	}
	else {
		/* FM-7モード : イニシエータOFF、ブートRAM書き込み禁止 */
		initrom_en = FALSE;
		bootram_rw = FALSE;
	}

	/* ブートRAM セットアップ */
	/* 割り込みベクタ以外を一旦すべてクリア(女神転生対策) */
	memset(boot_ram, 0, 0x1f0);

	/* FM-7モード時にはブートRAMの内容を転送する */
	mainmem_transfer_boot();

	/* イニシエータROM ハードウェアバージョン */
	switch (fm7_ver) {
		case 1:
			/* FM-7 */
			break;
		case 2:
			/* FM77AV */
			memset(&init_rom[0x0b0e], 0xff, 6);

			/* イニシエータがFM77AV相当の動作となるようにパッチ */
			if (patch_branewboottfr && patch_jmpnewboot) {
				patch_branewboottfr[0x0000] = 0x21;
				patch_jmpnewboot[0x0001] = 0xfe;
				patch_jmpnewboot[0x0002] = 0x00;
			}
			break;
#if XM7_VER >= 3
		case 3:
			/* FM77AV40EX */
			init_rom[0xb0e] = '4';
			init_rom[0xb0f] = '0';
			init_rom[0xb10] = '1';
			init_rom[0xb11] = 'M';
			init_rom[0xb12] = 'a';
			init_rom[0xb13] = '.';

			/* イニシエータがFM77AV40EX本来の動作となるよう元に戻す */
			if (patch_branewboottfr && patch_jmpnewboot) {
				patch_branewboottfr[0x0000] = 0x20;
				patch_jmpnewboot[0x0001] = 0x50;
				patch_jmpnewboot[0x0002] = 0x00;
			}
			break;
#endif
	}
#else
	/* ブート領域書き込み禁止 */
	bootram_rw = FALSE;
#endif
}

/*
 *	FM-7モード用 ブートRAM転送
 */
#if XM7_VER >= 2
void FASTCALL mainmem_transfer_boot(void)
{
	if (fm7_ver == 1) {
		if (boot_mode == BOOT_BASIC) {
			memcpy(boot_ram, &init_rom[0x1800], 0x1e0);
		}
		else {
			memcpy(boot_ram, &init_rom[0x1a00], 0x1e0);
		}
		boot_ram[0x1fe] = 0xfe;
		boot_ram[0x1ff] = 0x00;
	}
}
#endif

/*
 *	I/Oアクセス時のメモリレディ処理
 */
static void FASTCALL mainmem_iowait(void)
{
	BYTE tmp;

#if XM7_VER == 1
	/* 1.2MHzモードではI/Oウェイトがかからない */
	if (lowspeed_mode) {
		return;
	}
#endif

	/* クロック引き延ばしに必要なI/Oアクセス回数を設定 */
#if XM7_VER >= 3
	if ((mmr_flag || twr_flag) && !mmr_fastmode && ioaccess_flag) {
#else
	if ((mmr_flag || twr_flag) && ioaccess_flag) {
#endif
		/* MMRオン(2回目)  3回のアクセスで1サイクル */
		tmp = 3;
	}
	else {
		/* MMRオフ/高速モード/MMRオン(1回目)  2回のアクセスで1サイクル */
		tmp = 2;
	}

	/* 規定回数以上のI/Oアクセスがあれば、クロックを引き延ばす */
	ioaccess_count ++;
	if (ioaccess_count >= tmp) {
		maincpu.total ++;
		ioaccess_count = 0;

		ioaccess_flag = !ioaccess_flag;
	}
}


/*
 *	メインCPUメモリ
 *	１バイト取得
 */
BYTE FASTCALL mainmem_readb(WORD addr)
{
	BYTE dat;

	/* MMR, TWRチェック */
	if (mmr_flag || twr_flag) {
		/* MMR、TWRを通す */
		if (mmr_extrb(&addr, &dat)) {
			return dat;
		}
	}

	/* メインRAM(表) */
#if XM7_VER >= 2
	if (addr < 0x6000) {
		return mainram_a[addr];
	}
	if (addr < 0x8000) {
		if (initrom_en) {
			return init_rom[addr - 0x6000];
		}
		else {
			return mainram_a[addr];
		}
	}
#else
	if (addr < 0x8000) {
		return mainram_a[addr];
	}
#endif

	/* BASIC ROM or メインRAM(裏) */
	if (addr < 0xfc00) {
		if (basicrom_en) {
#if XM7_VER == 1
			if (fm_subtype == FMSUB_FM8) {
				return basic_rom8[addr - 0x8000];
			}
#endif
			return basic_rom[addr - 0x8000];
		}
		else {
			return mainram_b[addr - 0x8000];
		}
	}

	/* メインROMの直後 */
	if (addr < 0xfc80) {
		return mainram_b[addr - 0x8000];
	}

	/* 共有RAM */
	if (addr < 0xfd00) {
		if (subhalt_flag) {
			return shared_ram[(WORD)(addr - 0xfc80)];
		}
		else {
			return 0xff;
		}
	}

	/* ブートROM/RAM */
#if XM7_VER >= 2
	if ((addr >= 0xfffe) && (initrom_en)) {
		/* リセットベクタ */
		mainmem_iowait();
		return init_rom[addr - 0xe000];
	}
#endif
	if (addr >= 0xfe00) {
		if (addr <= 0xffdf) {
			mainmem_iowait();
		}
#if XM7_VER >= 2
		return boot_ram[addr - 0xfe00];
#else
		if (((addr >= 0xffe0) && (addr < 0xfffe)) ||
			((fm_subtype == FMSUB_FM77) && bootram_rw)) {
			/* FM-77 RAM */
			return boot_ram[addr - 0xfe00];
		}
		else {
			if (fm_subtype == FMSUB_FM8) {
				/* FM-8 BOOT ROM */
				if (boot_mode == BOOT_BASIC) {
					return boot_bas8[addr - 0xfe00];
				}
				else if (boot_mode == BOOT_DOS) {
					return boot_dos8[addr - 0xfe00];
				}
				else {
					return boot_bbl8[addr - 0xfe00];
				}
			}
			else {
				/* FM-7 BOOT ROM */
				if (boot_mode == BOOT_BASIC) {
					if (fm_subtype == FMSUB_FM7) {
						return boot_bas[addr - 0xfe00];
					}
					else {
						/* FM-77モード時はパッチ適用ブートを見せる */
						return boot_bas_patch[addr - 0xfe00];
					}
				}
				else {
					return boot_dos[addr - 0xfe00];
				}
			}
		}
#endif
	}

	/*
	 *	I/O空間
	 */
	mainmem_iowait();
	if (mainetc_readb(addr, &dat)) {
		return dat;
	}
	if (ttlpalet_readb(addr, &dat)) {
		return dat;
	}
	if (subctrl_readb(addr, &dat)) {
		return dat;
	}
	if (multipag_readb(addr, &dat)) {
		return dat;
	}
	if (fdc_readb(addr, &dat)) {
		return dat;
	}
	if (kanji_readb(addr, &dat)) {
		return dat;
	}
	if (tapelp_readb(addr, &dat)) {
		return dat;
	}
	if (opn_readb(addr, &dat)) {
		return dat;
	}
	if (whg_readb(addr, &dat)) {
		return dat;
	}
	if (thg_readb(addr, &dat)) {
		return dat;
	}
	if (mmr_readb(addr, &dat)) {
		return dat;
	}
#if XM7_VER >= 2
	if (apalet_readb(addr, &dat)) {
		return dat;
	}
#endif
#if defined(MIDI)
	if (midi_readb(addr, &dat)) {
		return dat;
	}
#endif
#if defined(RSC)
	if (rs232c_readb(addr, &dat)) {
		return dat;
	}
#endif
#if XM7_VER >= 3
	if (dmac_readb(addr, &dat)) {
		return dat;
	}
#endif
#if XM7_VER == 1
#if defined(JSUB)
	if (jsub_readb(addr, &dat)) {
		return dat;
	}
#endif
#if defined(BUBBLE)
	if (bmc_readb(addr, &dat)) {
		return dat;
	}
#endif
#endif
#if defined(MOUSE)
	if (ptm_readb(addr, &dat)) {
		return dat;
	}
	if (mouse_readb(addr, &dat)) {
		return dat;
	}
#endif

	return 0xff;
}

/*
 *	メインCPUメモリ
 *	１バイト取得(I/Oなし)
 */
BYTE FASTCALL mainmem_readbnio(WORD addr)
{
	BYTE dat;

	/* MMR, TWRチェック */
	if (mmr_flag || twr_flag) {
		/* MMR、TWRを通す */
		if (mmr_extbnio(&addr, &dat)) {
			return dat;
		}
	}

	/* メインRAM(表) */
#if XM7_VER >= 2
	if (addr < 0x6000) {
		return mainram_a[addr];
	}
	if (addr < 0x8000) {
		if (initrom_en) {
			return init_rom[addr - 0x6000];
		}
		else {
			return mainram_a[addr];
		}
	}
#else
	if (addr < 0x8000) {
		return mainram_a[addr];
	}
#endif

	/* BASIC ROM or メインRAM(裏) */
	if (addr < 0xfc00) {
		if (basicrom_en) {
#if XM7_VER == 1
			if (fm_subtype == FMSUB_FM8) {
				return basic_rom8[addr - 0x8000];
			}
#endif
			return basic_rom[addr - 0x8000];
		}
		else {
			return mainram_b[addr - 0x8000];
		}
	}

	/* メインROMの直後 */
	if (addr < 0xfc80) {
		return mainram_b[addr - 0x8000];
	}

	/* 共有RAM */
	if (addr < 0xfd00) {
		if (subhalt_flag) {
			return shared_ram[(WORD)(addr - 0xfc80)];
		}
		else {
			return 0xff;
		}
	}

	/* ブートROM/RAM */
#if XM7_VER >= 2
	if ((addr >= 0xfffe) && (initrom_en)) {
		/* リセットベクタ */
		return init_rom[addr - 0xe000];
	}
#endif
	if (addr >= 0xfe00) {
#if XM7_VER >= 2
		return boot_ram[addr - 0xfe00];
#else
		if (((addr >= 0xffe0) && (addr < 0xfffe)) ||
			((fm_subtype == FMSUB_FM77) && bootram_rw)) {
			return boot_ram[addr - 0xfe00];
		}
		else {
			if (fm_subtype == FMSUB_FM8) {
				if (boot_mode == BOOT_BASIC) {
					return boot_bas8[addr - 0xfe00];
				}
				else if (boot_mode == BOOT_DOS) {
					return boot_dos8[addr - 0xfe00];
				}
				else {
					return boot_bbl8[addr - 0xfe00];
				}
			}
			else {
				if (boot_mode == BOOT_BASIC) {
					if (fm_subtype == FMSUB_FM7) {
						return boot_bas[addr - 0xfe00];
					}
					else {
						return boot_bas_patch[addr - 0xfe00];
					}
				}
				else {
					return boot_dos[addr - 0xfe00];
				}
			}
		}
#endif
	}

	/* I/O空間 */
	ASSERT((addr >= 0xfd00) && (addr < 0xfe00));
	return main_io[addr - 0xfd00];
}

/*
 *	メインCPUメモリ
 *	１バイト書き込み
 */
void FASTCALL mainmem_writeb(WORD addr, BYTE dat)
{
	/* MMR, TWRチェック */
	if (mmr_flag || twr_flag) {
		/* MMR、TWRを通す */
		if (mmr_extwb(&addr, dat)) {
			return;
		}
	}

	/* メインRAM(表) */
#if XM7_VER >= 2
	if (addr < 0x6000) {
		mainram_a[addr] = dat;
		return;
	}
	if (addr < 0x8000) {
		if (!initrom_en) {
			mainram_a[addr] = dat;
		}
		return;
	}
#else
	if (addr < 0x8000) {
		mainram_a[addr] = dat;
		return;
	}
#endif

	/* BASIC ROM or メインRAM(裏) */
	if (addr < 0xfc00) {
		if (basicrom_en) {
			/* ROM内RCBでBIOSを呼び出すケース */
#if XM7_VER == 1
			if ((fm_subtype != FMSUB_FM8) && rom_ram_write) {
#else
			if ((fm7_ver == 1) && rom_ram_write) {
#endif
				/* FM-7ではF-BASIC ROM選択時にも裏RAMに書き込めるようなので */
				mainram_b[addr - 0x8000] = dat;
			}
			return;
		}
		else {
			mainram_b[addr - 0x8000] = dat;
			return;
		}
	}

	/* メインROMの直後 */
	if (addr < 0xfc80) {
		mainram_b[addr - 0x8000] = dat;
		return;
	}

	/* 共有RAM */
	if (addr < 0xfd00) {
		if (subhalt_flag) {
			shared_ram[(WORD)(addr - 0xfc80)] = dat;
			return;
		}
		else {
			/* BASE09対策 */
			return;
		}
	}

	/* ブートRAM */
	if (addr >= 0xfe00) {
#if XM7_VER >= 2
		if ((bootram_rw) && (fm7_ver >= 2)) {
#else
		if ((fm_subtype == FMSUB_FM77) && bootram_rw) {
#endif
			boot_ram[addr - 0xfe00] = dat;
			if ((addr <= 0xffdf) || (addr >= 0xfffe)) {
				mainmem_iowait();
			}
			return;
		}

		/* ブートワークRAM、ベクタ */
		if ((addr >= 0xffe0) && (addr < 0xfffe)) {
			boot_ram[addr - 0xfe00] = dat;
		}
		return;
	}

	/*
	 *	I/O空間
	 */
	ASSERT((addr >= 0xfd00) && (addr < 0xfe00));
	main_io[(WORD)(addr - 0xfd00)] = dat;
	mainmem_iowait();

	if (mainetc_writeb(addr, dat)) {
		return;
	}
	if (ttlpalet_writeb(addr, dat)) {
		return;
	}
	if (subctrl_writeb(addr, dat)) {
		return;
	}
	if (multipag_writeb(addr, dat)) {
		return;
	}
	if (fdc_writeb(addr, dat)) {
		return;
	}
	if (kanji_writeb(addr, dat)) {
		return;
	}
	if (tapelp_writeb(addr, dat)) {
		return;
	}
	if (opn_writeb(addr, dat)) {
		return;
	}
	if (whg_writeb(addr, dat)) {
		return;
	}
	if (thg_writeb(addr, dat)) {
		return;
	}
	if (mmr_writeb(addr, dat)) {
		return;
	}
#if XM7_VER >= 2
	if (apalet_writeb(addr, dat)) {
		return;
	}
#endif
#if defined(MIDI)
	if (midi_writeb(addr, dat)) {
		return;
	}
#endif
#if defined(RSC)
	if (rs232c_writeb(addr, dat)) {
		return;
	}
#endif
#if XM7_VER >= 3
	if (dmac_writeb(addr, dat)) {
		return;
	}
#endif
#if XM7_VER == 1
#if defined(JSUB)
	if (jsub_writeb(addr, dat)) {
		return;
	}
#endif
#if defined(BUBBLE)
	if (bmc_writeb(addr, dat)) {
		return;
	}
#endif
#endif
#if defined(MOUSE)
	if (ptm_writeb(addr, dat)) {
		return;
	}
	if (mouse_writeb(addr, dat)) {
		return;
	}
#endif

	return;
}

/*
 *	メインCPU物理アドレスバス
 *	１バイト読み出し(I/Oなし)
 */
BYTE FASTCALL mainmem_readbnio_p(DWORD raddr)
{
	DWORD rsegment;
	WORD addr;

	rsegment = (raddr & 0xf0000);
	addr = (WORD)(raddr & 0xffff);

	/* 標準空間 */
	if (rsegment == 0x30000) {
#if XM7_VER == 1
		/* BASIC ROM直後のRAM・共有RAM・I/O領域にはROM/RAMが見える */
		if ((raddr >= 0x3fc00) && (raddr < 0x3fe00)) {
			if (basicrom_en) {
				return 0x00;
			}
			return mainram_b[raddr - 0x38000];
		}

		/* ブート領域にはROM/RAMが見える */
		if (raddr >= 0x3fe00) {
			if (basicrom_en) {
				if (available_mmrboot) {
					return boot_mmr[raddr - 0x3fe00];
				}
				else {
					return boot_bas[raddr - 0x3fe00];
				}
			}
			else {
				return boot_ram[raddr - 0x3fe00];
			}
		}
#endif

		/* $30セグメント */
		/* メインRAM(表) */
#if XM7_VER >= 2
		if (addr < 0x6000) {
			return mainram_a[addr];
		}
		if (addr < 0x8000) {
			if (initrom_en) {
				return init_rom[addr - 0x6000];
			}
			else {
				return mainram_a[addr];
			}
		}
#else
		if (addr < 0x8000) {
			return mainram_a[addr];
		}
#endif

		/* BASIC ROM or メインRAM(裏) */
		if (addr < 0xfc00) {
			if (basicrom_en) {
#if XM7_VER == 1
				if (fm_subtype == FMSUB_FM8) {
					return basic_rom8[addr - 0x8000];
				}
#endif
				return basic_rom[addr - 0x8000];
			}
			else {
				return mainram_b[addr - 0x8000];
			}
		}

		/* 常駐領域の処理 */
		return mainmem_readbnio(addr);
	}

#if XM7_VER >= 2
	/* FM77AV 拡張RAM */
	if (rsegment == 0x00000) {
		return extram_a[raddr & 0xffff];
	}

	/* サブシステム */
	if (rsegment == 0x10000) {
		if (subhalt_flag) {
			return submem_readbnio((WORD)(raddr & 0xffff));
		}
		else {
			return 0xff;
		}
	}

	/* 日本語カード */
	if (rsegment == 0x20000) {
#if XM7_VER >= 2
		return jcard_readb((WORD)(raddr & 0xffff));
#else
		return 0xff;
#endif
	}

#if XM7_VER >= 3
	/* 768KB 拡張RAM */
	if (rsegment >= 0x40000) {
		if (mmr_extram) {
			return extram_c[raddr - 0x40000];
		}
		else {
			return 0xff;
		}
	}
#endif

	/* まず来ない */
	ASSERT(FALSE);
	return 0xff;
#elif XM7_VER == 1
	/* FM-77 拡張RAM */
	if (mmr_64kbmode) {
		if ((rsegment >> 16) == (DWORD)bnk_reg) {
			return extram_a[(DWORD)((raddr & 0xffff) | 0x20000)];
		}
		else {
			return 0xff;
		}
	}

	return extram_a[raddr];
#endif
}

/*
 *	メインCPU物理アドレスバス
 *	１バイト書き込み
 */
void FASTCALL mainmem_writeb_p(DWORD raddr, BYTE dat)
{
	DWORD rsegment;
	WORD addr;

	rsegment = (raddr & 0xf0000);
	addr = (WORD)(raddr & 0xffff);

	/* 標準空間 */
	if (rsegment == 0x30000) {
#if XM7_VER == 1
		/* BASIC ROM直後のRAM・共有RAM・I/O領域にはROM/RAMが見える */
		if ((raddr >= 0x3fc00) && (raddr < 0x3fe00)) {
			if (!basicrom_en) {
				mainram_b[raddr - 0x38000] = dat;
			}
			return;
		}

		/* ブート領域にはROM/RAMが見える */
		if (raddr >= 0x3fe00) {
			if (!basicrom_en) {
				boot_ram[raddr - 0x3fe00] = dat;
			}
			return;
		}
#endif

		/* $30セグメント */
		/* メインRAM(表) */
#if XM7_VER >= 2
		if (addr < 0x6000) {
			mainram_a[addr] = dat;
			return;
		}
		if (addr < 0x8000) {
			if (!initrom_en) {
				mainram_a[addr] = dat;
			}
			return;
		}
#else
		if (addr < 0x8000) {
			mainram_a[addr] = dat;
			return;
		}
#endif

		/* BASIC ROM or メインRAM(裏) */
		if (addr < 0xfc00) {
			if (basicrom_en) {
				/* ROM内RCBでBIOSを呼び出すケース */
#if XM7_VER == 1
			if ((fm_subtype != FMSUB_FM8) && rom_ram_write) {
#else
			if ((fm7_ver == 1) && rom_ram_write) {
#endif
					/* FM-7ではBASIC ROM選択時にも裏RAMに書き込めるようなので */
					mainram_b[addr - 0x8000] = dat;
				}
				return;
			}
			else {
				mainram_b[addr - 0x8000] = dat;
				return;
			}
		}

		/* 常駐領域の処理 */
		mainmem_writeb(addr, dat);
		return;
	}

#if XM7_VER >= 2
	/* FM77AV 拡張RAM */
	if (rsegment == 0x00000) {
		extram_a[raddr & 0xffff] = dat;
		return;
	}

	/* サブシステム */
	if (rsegment == 0x10000) {
		if (subhalt_flag) {
			submem_writeb((WORD)(raddr & 0xffff), dat);
		}
		return;
	}

	/* 日本語カード */
	if (rsegment == 0x20000) {
#if XM7_VER >= 2
		jcard_writeb((WORD)(raddr & 0xffff), dat);
#endif
		return;
	}

#if XM7_VER >= 3
	/* AV40拡張RAM */
	if (rsegment >= 0x40000) {
		if (mmr_extram) {
			extram_c[raddr - 0x40000] = dat;
		}
		return;
	}
#endif
#else
	/* FM-77 拡張RAM */
	if (mmr_64kbmode) {
		if ((rsegment >> 16) == (DWORD)bnk_reg) {
			extram_a[(DWORD)((raddr & 0xffff) | 0x20000)] = dat;
		}
		return;
	}

	extram_a[raddr] = dat;
	return;
#endif
}

/*
 *	メインCPUメモリ
 *	セーブ
 */
BOOL FASTCALL mainmem_save(int fileh)
{
	if (!file_write(fileh, mainram_a, 0x8000)) {
		return FALSE;
	}
#if XM7_VER == 1
	if (!file_write(fileh, mainram_b, 0x7e00)) {
#else
	if (!file_write(fileh, mainram_b, 0x7c80)) {
#endif
		return FALSE;
	}

	if (!file_write(fileh, main_io, 0x100)) {
		return FALSE;
	}

	if (!file_write(fileh, extram_a, 0x8000)) {
		return FALSE;
	}
	if (!file_write(fileh, &extram_a[0x8000], 0x8000)) {
		return FALSE;
	}
#if XM7_VER == 1
	if (!file_write(fileh, &extram_a[0x10000], 0x8000)) {
		return FALSE;
	}
	if (!file_write(fileh, &extram_a[0x18000], 0x8000)) {
		return FALSE;
	}
	if (!file_write(fileh, &extram_a[0x20000], 0x8000)) {
		return FALSE;
	}
	if (!file_write(fileh, &extram_a[0x28000], 0x8000)) {
		return FALSE;
	}
#endif
	if (!file_write(fileh, boot_ram, 0x200)) {
		return FALSE;
	}

	if (!file_bool_write(fileh, basicrom_en)) {
		return FALSE;
	}
#if XM7_VER >= 2
	if (!file_bool_write(fileh, initrom_en)) {
		return FALSE;
	}
#endif
	if (!file_bool_write(fileh, bootram_rw)) {
		return FALSE;
	}

#if XM7_VER >= 3
	/* Ver8拡張 */
	if (!file_write(fileh, &init_rom[0x0b0e], 3)) {
		return FALSE;
	}
#endif

	return TRUE;
}

/*
 *	メインCPUメモリ
 *	ロード
 */
BOOL FASTCALL mainmem_load(int fileh, int ver)
{
	/* バージョンチェック */
	if (ver < 200) {
		return FALSE;
	}

	if (!file_read(fileh, mainram_a, 0x8000)) {
		return FALSE;
	}
#if XM7_VER == 1
	/* Ver302拡張 */
	if (ver <= 301) {
		if (!file_read(fileh, mainram_b, 0x7c80)) {
			return FALSE;
		}
		memset(&mainram_b[0x7c80], 0, 0x180);
	}
	else {
		if (!file_read(fileh, mainram_b, 0x7e00)) {
			return FALSE;
		}
	}
#else
	if (!file_read(fileh, mainram_b, 0x7c80)) {
		return FALSE;
	}
#endif

	if (!file_read(fileh, main_io, 0x100)) {
		return FALSE;
	}

	if (!file_read(fileh, extram_a, 0x8000)) {
		return FALSE;
	}
	if (!file_read(fileh, &extram_a[0x8000], 0x8000)) {
		return FALSE;
	}
#if XM7_VER == 1
	if (!file_read(fileh, &extram_a[0x10000], 0x8000)) {
		return FALSE;
	}
	if (!file_read(fileh, &extram_a[0x18000], 0x8000)) {
		return FALSE;
	}
	if (!file_read(fileh, &extram_a[0x20000], 0x8000)) {
		return FALSE;
	}
	if (!file_read(fileh, &extram_a[0x28000], 0x8000)) {
		return FALSE;
	}
#endif
	if (!file_read(fileh, boot_ram, 0x200)) {
		return FALSE;
	}

	if (!file_bool_read(fileh, &basicrom_en)) {
		return FALSE;
	}
#if XM7_VER >= 2
	if (!file_bool_read(fileh, &initrom_en)) {
		return FALSE;
	}
#endif
	if (!file_bool_read(fileh, &bootram_rw)) {
		return FALSE;
	}

#if XM7_VER >= 3
	/* Ver8拡張 */
	if (ver >= 800) {
		if (!file_read(fileh, &init_rom[0x0b0e], 3)) {
			return FALSE;
		}
	}
#endif

	return TRUE;
}
