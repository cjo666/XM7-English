/*
 *	FM-7 EMULATOR "XM7"
 *
 *	Copyright (C) 1999-2017 ＰＩ．(Twitter:@xm6_original)
 *	Copyright (C) 2001-2017 Ryu Takegami (Twitter:@RyuTakegami)
 *
 *	[ MMR,TWR / I/O型RAMディスクカード ]
 *
 *	RHG履歴
 *	  2002.06.04		MR2のI/O型RAMディスク機能に仮対応
 *	  2002.07.29		MR2関連部を少し手直し
 *						MMRでの再配置禁止領域からCRTC I/O(AV40)を除外した
 *	  2002.09.25		R2D2/RD512仮対応(投げやり)
 *	  2003.06.19		MMR関連レジスタ変更時の速度補正を導入
 *	  2003.09.30		$FD0Bのキャリアディテクトビットを新設
 *	  2004.12.01		V1.1でMMR I/Oのリードアクセスができない問題を修正
 *						(な、なんだってー)
 *	  2006.08.15		FM-77/FM77AVモード時にMMRレジスタの読み取り結果が
 *						下位6ビットしか有効にならない仕様を変更
 *						(日本語通信カード JTESTEB)
 *	  2010.06.09		初回起動時のみMMRをクリアするように実機に合わせて
 *						仕様を変更
 *	  2010.06.18		FM-77モード時の$3Fバンクの処理を実機に合わせて変更
 *	  2012.04.22		テキストウィンドウ高速モードフラグを実装。ただし、
 *						現時点では実際の挙動には何の影響も及ぼしません。
 *	  2015.02.06		V1.1において64KB拡張RAMカードの動作をサポート
 *						(GUI実装などはしていません)
 *+	  2016.06.26		400ラインタイミング出力フラグを実装
 *						(実際の表示とは関係なく動作します。誰得過ぎ)
 */

#include <stdlib.h>
#include <string.h>
#include "xm7.h"
#include "device.h"
#include "mmr.h"
#include "subctrl.h"
#include "jcard.h"
#include "rs232c.h"

/*
 *	謎
 */
#if defined(RAMDISK)
#define	MR2
#define	RD512
#define	R2D2
#endif

/*
 *	グローバル ワーク
 */
BOOL mmr_flag;							/* MMR有効フラグ */
BYTE mmr_seg;							/* MMRセグメント */
BOOL mmr_modify;						/* MMR状態変更フラグ */
#if XM7_VER >= 3
BYTE mmr_reg[0x80];						/* MMRレジスタ */
BOOL twr_flag;							/* TWR有効フラグ */
BYTE twr_reg;							/* TWRレジスタ */
BOOL mmr_ext;							/* 拡張MMR有効フラグ */
BOOL mmr_fastmode;						/* MMR高速フラグ */
BOOL mmr_extram;						/* 拡張RAM有効フラグ */
BOOL mmr_fast_refresh;					/* 高速リフレッシュフラグ */
BOOL mmr_768kbmode;						/* 拡張RAM動作モード */
BOOL twr_fastmode;						/* TWR高速モード */
BOOL dsp_400linetiming;					/* 400ラインタイミング出力フラグ */
#else
BYTE mmr_reg[0x40];						/* MMRレジスタ */
BOOL twr_flag;							/* TWR有効フラグ */
BYTE twr_reg;							/* TWRレジスタ */
#if XM7_VER == 1
BYTE bnk_reg;							/* 拡張RAMバンクセレクトレジスタ */
BOOL mmr_64kbmode;						/* 拡張RAM動作モード */
#endif
#endif

/* I/O型RAMディスクカード */
#if XM7_VER >= 3
#if defined(MR2)
BYTE mr2_nowcnt;						/* MR2 セクタカウント */
WORD mr2_secreg;						/* MR2 セクタレジスタ */
#endif
#if defined(RD512)
WORD rd512_secreg;						/* RD512 セクタレジスタ */
#endif
#if defined(R2D2)
BYTE r2d2_nowcnt;						/* R2D2 セクタカウント */
WORD r2d2_secreg;						/* R2D2 セクタレジスタ */
#endif
#endif

/*
 *	MMR
 *	初期化
 */
BOOL FASTCALL mmr_init(void)
{
	/* 初回起動時のみMMRをクリアするように変更 */
	memset(mmr_reg, 0, sizeof(mmr_reg));

#if XM7_VER >= 3
	/* 768KB 拡張RAMカードをディセーブル・768KB有効化 */
	mmr_extram = FALSE;
	mmr_768kbmode = TRUE;

	/* 400ラインタイミング出力をディセーブル */
	dsp_400linetiming = FALSE;
#elif XM7_VER == 1
	/* 拡張RAMは192KB動作モード */
	mmr_64kbmode = FALSE;
#endif

	return TRUE;
}

/*
 *	MMR
 *	クリーンアップ
 */
void FASTCALL mmr_cleanup(void)
{
}

/*
 *	MMR
 *	リセット
 */
void FASTCALL mmr_reset(void)
{
	/* MMR/TWR */
	mmr_flag = FALSE;
	twr_flag = FALSE;
	mmr_seg = 0;
	twr_reg = 0;
	mmr_modify = FALSE;
#if XM7_VER == 1
	bnk_reg = 0x02;
#endif

#if XM7_VER >= 3
	/* MMR/TWR(AV40拡張) */
	mmr_ext = FALSE;
	mmr_fastmode = FALSE;
	mmr_fast_refresh = FALSE;
	twr_fastmode = FALSE;

	/* I/O型RAMディスク */
#if defined(MR2)
	mr2_nowcnt = 0;
	mr2_secreg = 0;
#endif
#if defined(RD512)
	rd512_secreg = 0;
#endif
#if defined(R2D2)
	r2d2_nowcnt = 0;
	r2d2_secreg = 0;
#endif
#endif
}

/*-[ メモリマネージャ ]-----------------------------------------------------*/

/*
 *	TWRアドレス変換
 */
BOOL FASTCALL mmr_trans_twr(WORD addr, DWORD *taddr)
{
	/* TWR有効か */
	if (!twr_flag) {
		return FALSE;
	}

	/* アドレス要件チェック */
	if ((addr < 0x7c00) || (addr > 0x7fff)) {
		return FALSE;
	}

	/* TWRレジスタより変換 */
	*taddr = (DWORD)twr_reg;
	*taddr *= 256;
	*taddr += addr;
	*taddr &= 0xffff;
#if XM7_VER == 1
	/* FM-77(not AV)のテキスト空間に合わせてアドレスを補正 */
	*taddr |= 0x20000;
#endif

	return TRUE;
}

/*
 *	MMR
 *	アドレス変換
 */
DWORD FASTCALL mmr_trans_mmr(WORD addr)
{
	DWORD maddr;
	int offset;

	/* MMR有効か */
	if (!mmr_flag) {
		return (DWORD)(0x30000 | addr);
	}

	/* MMRレジスタより取得 */
	offset = (int)addr;
	offset >>= 12;

#if XM7_VER >= 3
	/* 拡張MMRがoffなら、セグメントは0～3まで */
	if (mmr_ext) {
		offset |= (mmr_seg * 0x10);
	}
	else {
		offset |= ((mmr_seg & 0x03) * 0x10);
	}
#else
	offset |= ((mmr_seg & 0x03) * 0x10);
#endif

	/* 拡張MMRがoffなら、6bitのみ有効 */
	maddr = (DWORD)mmr_reg[offset];
#if XM7_VER >= 3
	if (!mmr_ext) {
		maddr &= 0x3f;
	}
#else
	maddr &= 0x3f;
#endif
	maddr <<= 12;

	/* 下位12ビットと合成 */
	addr &= 0xfff;
	maddr |= addr;

	return maddr;
}

/*
 *	MMR
 *	物理アドレス→論理アドレス変換
 */
DWORD FASTCALL mmr_trans_phys_to_logi(DWORD target)
{
	DWORD addr;
	BYTE mmr_dat, target_seg;
	int i;

	/* 下位16ビットを取得、常駐領域は下位16bitをそのまま返す */
	addr = (DWORD)(target & 0xffff);
	if ((addr >= 0xfc00) && (addr <= 0xffff)) {
		return (DWORD)addr;
	}

	/* MMRの内容から物理アドレスを算出(検索?) */
	for (i = 0; i < 16; i++) {
#if xm7_VER >= 3
		if (mmr_ext) {
			mmr_dat = mmr_reg[((mmr_seg & 7) * 0x10) + i];
			target_seg = (BYTE)(target >> 12);
		}
		else {
			mmr_dat = (BYTE)(mmr_reg[((mmr_seg & 3) * 0x10) + i] & 0x3f);
			target_seg = (BYTE)((target >> 12) & 0x3f);
		}
#else
		mmr_dat = (BYTE)(mmr_reg[((mmr_seg & 3) * 0x10) + i] & 0x3f);
		target_seg = (BYTE)((target >> 12) & 0x3f);
#endif
		if (target_seg == mmr_dat) {
			return ((DWORD)i << 12) | (addr & 0xfff);
		}
	}

	/* 論理アドレスに割り当てられていない場合、下位16bitをそのまま返す */
	return (DWORD)(addr & 0x0ffff);
}

/*
 *	メインCPUバス
 *	１バイト読み出し
 */
BOOL FASTCALL mmr_extrb(WORD *addr, BYTE *dat)
{
	DWORD raddr, rsegment;

	/* $FC00～$FFFFは常駐空間 */
	if (*addr >= 0xfc00) {
		return FALSE;
	}

	/* TWR,MMRを通す */
	if (!mmr_trans_twr(*addr, &raddr)) {
		raddr = mmr_trans_mmr(*addr);
	}

	rsegment = (raddr & 0xf0000);

	/* 標準空間 */
	if (rsegment == 0x30000) {
		/* MMRは再配置禁止 */
#if XM7_VER >= 2
		if (fm7_ver >= 2) {
			if ((raddr >= 0x3fd80) && (raddr <= 0x3fd97)) {
				*dat = 0xff;
				return TRUE;
			}
		}
#endif

#if XM7_VER == 1
		/* BASIC ROM直後のRAM・共有RAM・I/O領域にはROM/RAMが見える */
		if ((raddr >= 0x3fc00) && (raddr < 0x3fe00)) {
			if (basicrom_en) {
				*dat = 0x00;
			}
			else {
				*dat = mainram_b[raddr - 0x38000];
			}
			return TRUE;
		}

		/* ブート領域にはROM/RAMが見える */
		if (raddr >= 0x3fe00) {
			if (basicrom_en) {
				if (available_mmrboot) {
					*dat = boot_mmr[raddr - 0x3fe00];
				}
				else {
					*dat = boot_bas[raddr - 0x3fe00];
				}
			}
			else {
				*dat = boot_ram[raddr - 0x3fe00];
			}
			return TRUE;
		}
#endif

		/* $30セグメント */
		*addr = (WORD)(raddr & 0xffff);
		return FALSE;
	}

#if XM7_VER >= 2
	/* FM77AV 拡張RAM */
	if (rsegment == 0x00000) {
		*dat = extram_a[raddr & 0xffff];
		return TRUE;
	}

	/* サブシステム */
	if (rsegment == 0x10000) {
		if (subhalt_flag) {
			*dat = submem_readb((WORD)(raddr & 0xffff));
		}
		else {
			*dat = 0xff;
		}
		return TRUE;
	}

	/* 日本語カード */
	if (rsegment == 0x20000) {
#if XM7_VER >= 2
		*dat = jcard_readb((WORD)(raddr & 0xffff));
#else
		*dat = 0xff;
#endif
		return TRUE;
	}

#if XM7_VER >= 3
	/* 768KB 拡張RAM */
	if (rsegment >= 0x40000) {
		if (mmr_extram) {
			if (mmr_768kbmode || (rsegment <= 0x70000)) {
				*dat = extram_c[raddr - 0x40000];
			}
		}
		else {
			*dat = 0xff;
		}
		return TRUE;
	}
#endif

	return FALSE;
#else
	/* FM-77 拡張RAM */
	if (mmr_64kbmode) {
		if ((rsegment >> 16) == (DWORD)bnk_reg) {
			*dat = extram_a[(DWORD)((raddr & 0xffff) | 0x20000)];
		}
		else {
			*dat = 0xff;
		}
		return TRUE;
	}

	*dat = extram_a[raddr];
	return TRUE;
#endif
}

/*
 *	メインCPUバス
 *	１バイト読み出し(I/Oなし)
 */
BOOL FASTCALL mmr_extbnio(WORD *addr, BYTE *dat)
{
	DWORD raddr, rsegment;

	/* $FC00～$FFFFは常駐空間 */
	if (*addr >= 0xfc00) {
		return FALSE;
	}

	/* TWR,MMRを通す */
	if (!mmr_trans_twr(*addr, &raddr)) {
		raddr = mmr_trans_mmr(*addr);
	}

	rsegment = (raddr & 0xf0000);

	/* 標準空間 */
	if (rsegment == 0x30000) {
		/* MMRは再配置禁止 */
#if XM7_VER >= 2
		if (fm7_ver >= 2) {
			if ((raddr >= 0x3fd80) && (raddr <= 0x3fd97)) {
				*dat = 0xff;
				return TRUE;
			}
		}
#endif

#if XM7_VER == 1
		/* BASIC ROM直後のRAM・共有RAM・I/O領域にはROM/RAMが見える */
		if ((raddr >= 0x3fc00) && (raddr < 0x3fe00)) {
			if (basicrom_en) {
				*dat = 0x00;
			}
			else {
				*dat = mainram_b[raddr - 0x38000];
			}
			return TRUE;
		}

		/* ブート領域にはROM/RAMが見える */
		if (raddr >= 0x3fe00) {
			if (basicrom_en) {
				if (available_mmrboot) {
					*dat = boot_mmr[raddr - 0x3fe00];
				}
				else {
					*dat = boot_bas[raddr - 0x3fe00];
				}
			}
			else {
				*dat = boot_ram[raddr - 0x3fe00];
			}
			return TRUE;
		}
#endif

		/* $30セグメント */
		*addr = (WORD)(raddr & 0xffff);
		return FALSE;
	}

#if XM7_VER >= 2
	/* FM77AV 拡張RAM */
	if (rsegment == 0x00000) {
		*dat = extram_a[raddr & 0xffff];
		return TRUE;
	}

	/* サブシステム */
	if (rsegment == 0x10000) {
		if (subhalt_flag) {
			*dat = submem_readbnio((WORD)(raddr & 0xffff));
		}
		else {
			*dat = 0xff;
		}
		return TRUE;
	}

	/* 日本語カード */
	if (rsegment == 0x20000) {
#if XM7_VER >= 2
		*dat = jcard_readb((WORD)(raddr & 0xffff));
#else
		*dat = 0xff;
#endif
		return TRUE;
	}

#if XM7_VER >= 3
	/* 768KB 拡張RAM */
	if (rsegment >= 0x40000) {
		if (mmr_extram) {
			if (mmr_768kbmode || (rsegment <= 0x70000)) {
				*dat = extram_c[raddr - 0x40000];
			}
		}
		else {
			*dat = 0xff;
		}
		return TRUE;
	}
#endif

	return FALSE;
#else
	/* FM-77 拡張RAM */
	if (mmr_64kbmode) {
		if ((rsegment >> 16) == (DWORD)bnk_reg) {
			*dat = extram_a[(DWORD)((raddr & 0xffff) | 0x20000)];
		}
		else {
			*dat = 0xff;
		}
		return TRUE;
	}

	*dat = extram_a[raddr];
	return TRUE;
#endif
}

/*
 *	メインCPUバス
 *	１バイト書き込み
 */
BOOL FASTCALL mmr_extwb(WORD *addr, BYTE dat)
{
	DWORD raddr, rsegment;

	/* $FC00～$FFFFは常駐空間 */
	if (*addr >= 0xfc00) {
		return FALSE;
	}

	/* TWR,MMRを通す */
	if (!mmr_trans_twr(*addr, &raddr)) {
		raddr = mmr_trans_mmr(*addr);
	}

	rsegment = (raddr & 0xf0000);

	/* 標準空間 */
	if (rsegment == 0x30000) {
		/* MMRは再配置禁止 */
#if XM7_VER >= 2
		if (fm7_ver >= 2) {
			if ((raddr >= 0x3fd80) && (raddr <= 0x3fd97)) {
				return TRUE;
			}
		}
#endif

#if XM7_VER == 1
		/* BASIC ROM直後のRAM・共有RAM・I/O領域にはROM/RAMが見える */
		if ((raddr >= 0x3fc00) && (raddr < 0x3fe00)) {
			if (!basicrom_en) {
				mainram_b[raddr - 0x38000] = dat;
			}
			return TRUE;
		}

		/* ブート領域にはROM/RAMが見える */
		if (raddr >= 0x3fe00) {
			if (!basicrom_en) {
				boot_ram[raddr - 0x3fe00] = dat;
			}
			return TRUE;
		}
#endif

		/* $30セグメント */
		*addr = (WORD)(raddr & 0xffff);
		return FALSE;
	}

#if XM7_VER >= 2
	/* FM77AV 拡張RAM */
	if (rsegment == 0x00000) {
		extram_a[raddr & 0xffff] = dat;
		return TRUE;
	}

	/* サブシステム */
	if (rsegment == 0x10000) {
		if (subhalt_flag) {
			submem_writeb((WORD)(raddr & 0xffff), dat);
		}
		return TRUE;
	}

	/* 日本語カード */
	if (rsegment == 0x20000) {
#if XM7_VER >= 2
		jcard_writeb((WORD)(raddr & 0xffff), dat);
#endif
		return TRUE;
	}

#if XM7_VER >= 3
	/* AV40拡張RAM */
	if (rsegment >= 0x40000) {
		if (mmr_extram) {
			if (mmr_768kbmode || (rsegment <= 0x70000)) {
				extram_c[raddr - 0x40000] = dat;
			}
		}
		return TRUE;
	}
#endif

	return FALSE;
#else
	/* FM-77 拡張RAM */
	if (mmr_64kbmode) {
		if ((rsegment >> 16) == (DWORD)bnk_reg) {
			extram_a[(DWORD)((raddr & 0xffff) | 0x20000)] = dat;
		}
		return TRUE;
	}

	extram_a[raddr] = dat;
	return TRUE;
#endif

}

/*-[ MR2 I/O型RAMディスク ]-------------------------------------------------*/

#if (XM7_VER >= 3) && defined(MR2)
/*
 *	MR2
 *	アドレス計算
 */
static DWORD FASTCALL mr2_address(void)
{
	DWORD tmp;

	ASSERT (XM7_VER >= 3);

	if (mr2_secreg <= 0x0bff) {
		/* 計算方法は適当なので違っている可能性、大(^^; 2002/07/29 */
		tmp = (0xbff - mr2_secreg) << 8;
		tmp |= mr2_nowcnt;
	}
	else {
		tmp = 0;
	}

	return tmp;
}

/*
 *	MR2
 *	データリード
 */
static BYTE FASTCALL mr2_read_data(void)
{
	BYTE dat;

	ASSERT (XM7_VER >= 3);

	/* セクタレジスタが異常の場合は読み出せない */
	if (mr2_secreg <= 0x0bff) {
		dat = extram_c[mr2_address()];
		mr2_nowcnt ++;
	}
	else {
		dat = 0xff;
	}

	return dat;
}

/*
 *	MR2
 *	データライト
 */
static void FASTCALL mr2_write_data(BYTE dat)
{
	ASSERT (XM7_VER >= 3);

	/* セクタレジスタが異常の場合は書き込めない */
	if (mr2_secreg <= 0x0bff) {
		extram_c[mr2_address()] = dat;
		mr2_nowcnt ++;
	}
}

/*
 *	MR2
 *	セクタレジスタ更新
 */
static void FASTCALL mr2_update_sector(WORD addr, BYTE dat)
{
	ASSERT (XM7_VER >= 3);

	/* セクタレジスタを更新 */
	if (addr == 0xfd9e) {
		mr2_secreg &= (WORD)0x00ff;
		mr2_secreg |= (WORD)((dat & 0x0f) << 8);
	}
	else {
		mr2_secreg &= (WORD)0x0f00;
		mr2_secreg |= (WORD)dat;
	}

	/* 範囲チェック */
	if (mr2_secreg >= 0x0c00) {
		ASSERT(FALSE);
		mr2_secreg = 0x0c00;
	}

	/* セクタカウンタをリセット */
	mr2_nowcnt = 0;
}
#endif

/*-[ RD512 I/O型RAMディスク ]-----------------------------------------------*/

#if (XM7_VER >= 3) && defined(RD512)
/*
 *	RD512
 *	アドレス計算
 */
static DWORD FASTCALL rd512_address(WORD addr)
{
	ASSERT (XM7_VER >= 3);

	return (DWORD)(rd512_secreg + ((addr & 0x07) << 16) + 0x40000);
}

/*
 *	RD512
 *	データリード
 */
static BYTE FASTCALL rd512_read_data(WORD addr)
{
	BYTE dat;

	ASSERT (XM7_VER >= 3);

	if ((addr >= 0xfd48) && (addr <= 0xfd4f)) {
		dat = extram_c[rd512_address(addr)];
	}
	else {
		dat = 0xff;
	}

	return dat;
}

/*
 *	RD512
 *	データライト
 */
static void FASTCALL rd512_write_data(WORD addr, BYTE dat)
{
	ASSERT (XM7_VER >= 3);

	if ((addr >= 0xfd48) && (addr <= 0xfd4f)) {
		extram_c[rd512_address(addr)] = dat;
	}
}

/*
 *	RD512
 *	セクタレジスタ更新
 */
static void FASTCALL rd512_update_sector(WORD addr, BYTE dat)
{
	ASSERT (XM7_VER >= 3);

	/* セクタレジスタを更新 */
	if (addr == 0xfd41) {
		rd512_secreg &= (WORD)0x00ff;
		rd512_secreg |= (WORD)((dat & 0xff) << 8);
	}
	else {
		rd512_secreg &= (WORD)0xff00;
		rd512_secreg |= (WORD)dat;
	}
}
#endif

/*-[ R2D2 I/O型RAMディスク ]------------------------------------------------*/

#if (XM7_VER >= 3) && defined(R2D2)
/*
 *	R2D2
 *	アドレス計算
 */
static DWORD FASTCALL r2d2_address(WORD addr)
{
	DWORD tmp;

	ASSERT (XM7_VER >= 3);

	tmp =  (DWORD)(r2d2_secreg << 8);
	tmp |= (DWORD)(r2d2_nowcnt & 0xfe);
	tmp |= (DWORD)(addr & 0x01);
	tmp += 0x40000;

	return tmp;
}

/*
 *	R2D2
 *	データリード
 */
static BYTE FASTCALL r2d2_read_data(WORD addr)
{
	BYTE dat;

	ASSERT (XM7_VER >= 3);

	if (r2d2_secreg <= 0x07ff) {
		dat = extram_c[r2d2_address(addr)];
	}
	r2d2_nowcnt ++;

	return dat;
}

/*
 *	R2D2
 *	データライト
 */
static void FASTCALL r2d2_write_data(WORD addr, BYTE dat)
{
	ASSERT (XM7_VER >= 3);

	if (r2d2_secreg <= 0x07ff) {
		extram_c[r2d2_address(addr)] = dat;
	}
	r2d2_nowcnt ++;
}

/*
 *	R2D2
 *	セクタレジスタ更新
 */
static void FASTCALL r2d2_update_sector(WORD addr, BYTE dat)
{
	ASSERT (XM7_VER >= 3);

	/* セクタレジスタを更新 */
	if (addr == 0xfd62) {
		r2d2_secreg &= (WORD)0x00ff;
		r2d2_secreg |= (WORD)((dat & 0x07) << 8);
	}
	else {
		r2d2_secreg &= (WORD)0x0700;
		r2d2_secreg |= (WORD)dat;
	}

	/* カウンタ初期化 */
	r2d2_nowcnt = 0;
}
#endif

/*-[ メモリマップドI/O ]----------------------------------------------------*/

/*
 *	MMR
 *	１バイト読み出し
 */
BOOL FASTCALL mmr_readb(WORD addr, BYTE *dat)
{
	BYTE tmp;

	/* バージョンチェック */
#if XM7_VER >= 2
	if (fm7_ver < 2) {
		return FALSE;
	}
#else
	if (fm_subtype != FMSUB_FM77) {
		return FALSE;
	}
#endif

	switch (addr) {
#if XM7_VER >= 2
		/* ブートステータス */
		case 0xfd0b:
			if (boot_mode == BOOT_BASIC) {
				*dat = 0xfe;
			}
			else {
				*dat = 0xff;
			}

#if defined(RSC)
			/* RS-232C CD信号 */
			if (rs_cd) {
				*dat &= (BYTE)~RSCB_CD;
			}
#endif
			return TRUE;

		/* イニシエータROM */
		case 0xfd10:
			*dat = 0xff;
			return TRUE;
#endif

#if XM7_VER == 1
		/* 拡張RAMバンクセレクトレジスタ */
		case 0xfd2f:
			*dat = (BYTE)(0xfc | (bnk_reg & 0x03));
			return TRUE;
#endif

		/* MMRセグメント */
		case 0xfd90:
			*dat = 0xff;
			return TRUE;

		/* TWRオフセット */
		case 0xfd92:
			*dat = 0xff;
			return TRUE;

		/* モードセレクト */
		case 0xfd93:
			tmp = 0xff;
			if (!mmr_flag) {
				tmp &= (BYTE)(~0x80);
			}
			if (!twr_flag) {
				tmp &= ~0x40;
			}
			if (!bootram_rw) {
				tmp &= ~1;
			}
			*dat = tmp;
			return TRUE;

#if XM7_VER >= 3
		/* 拡張MMR/CPUスピード */
		case 0xfd94:
			*dat = 0xff;
			return TRUE;

		/* モードセレクト２ */
		case 0xfd95:
			tmp = 0xff;
			if (fm7_ver >= 3) {
				/* bit7:拡張ROMセレクト */
				if (extrom_sel) {
					tmp &= (BYTE)~0x80;
				}
				/* bit4:MMR使用時の速度低下を抑止 */
				if (!mmr_fastmode) {
					tmp &= (BYTE)~0x08;
				}
				/* bit0:400ラインタイミング出力ステータス */
				/*      XM7では1(200ライン出力)固定 …だったが。 */
				if (dsp_400linetiming) {
					tmp &= (BYTE)~0x01;
				}
			}

			*dat = tmp;
			return TRUE;

#if defined(MR2)
		/* MR2 データレジスタ */
		case 0xfd9c:
			if (mmr_extram && mmr_768kbmode) {
				*dat = mr2_read_data();
			}
			else {
				*dat = 0xff;
			}
			return TRUE;

		/* MR2 セクタレジスタ(Write Only) */
		case 0xfd9e:
		case 0xfd9f:
			*dat = 0xff;
			return TRUE;
#endif

#if defined(RD512)
		/* RD512 セクタレジスタ(Write Only) */
		case 0xfd40:
		case 0xfd41:
			*dat = 0xff;
			return TRUE;

		/* RD512 データレジスタ */
		case 0xfd48:
		case 0xfd49:
		case 0xfd4a:
		case 0xfd4b:
		case 0xfd4c:
		case 0xfd4d:
		case 0xfd4e:
		case 0xfd4f:
			if (mmr_extram) {
				*dat = rd512_read_data(addr);
			}
			else {
				*dat = 0xff;
			}
			return TRUE;
#endif

#if defined(R2D2)
		/* R2D2 データレジスタ */
		case 0xfd60:
		case 0xfd61:
			if (mmr_extram) {
				*dat = r2d2_read_data(addr);
			}
			else {
				*dat = 0xff;
			}
			return TRUE;

		/* R2D2 セクタレジスタ(Write Only) */
		case 0xfd62:
		case 0xfd63:
			*dat = 0xff;
			return TRUE;
#endif
#endif
	}

	/* MMRレジスタ */
	if ((addr >= 0xfd80) && (addr <= 0xfd8f)) {
#if XM7_VER >= 3
		if (mmr_ext) {
			tmp = mmr_reg[mmr_seg * 0x10 + (addr - 0xfd80)];
		}
		else {
			tmp = mmr_reg[(mmr_seg & 3) * 0x10 + (addr - 0xfd80)];
			/* tmp &= 0x3f; */
		}
#else
		tmp = mmr_reg[(mmr_seg & 3) * 0x10 + (addr - 0xfd80)];
		/* tmp &= 0x3f; */
#endif
		*dat = tmp;
		return TRUE;
	}

	return FALSE;
}

/*
 *	MMR
 *	１バイト書き込み
 */
BOOL FASTCALL mmr_writeb(WORD addr, BYTE dat)
{
	/* バージョンチェック */
#if XM7_VER >= 2
	if (fm7_ver < 2) {
		return FALSE;
	}
#else
	if (fm_subtype != FMSUB_FM77) {
		return FALSE;
	}
#endif

	switch (addr) {
		/* イニシエータROM */
#if XM7_VER >= 2
		case 0xfd10:
			if (dat & 0x02) {
				initrom_en = FALSE;
			}
			else {
				initrom_en = TRUE;
			}
			return TRUE;
#endif

#if XM7_VER == 1
		/* 拡張RAMバンクセレクトレジスタ */
		case 0xfd2f:
			bnk_reg = (BYTE)(dat & 0x03);
			return TRUE;
#endif

		/* MMRセグメント */
		case 0xfd90:
			mmr_seg = (BYTE)(dat & 0x07);
			return TRUE;

		/* TWRオフセット */
		case 0xfd92:
			twr_reg = dat;
			return TRUE;

		/* モードセレクト */
		case 0xfd93:
			if (dat & 0x80) {
				if (!mmr_flag) {
					mmr_flag = TRUE;
#if XM7_VER >= 3
					if (!mmr_fastmode) {
						mmr_modify = TRUE;
					}
#else
					mmr_modify = TRUE;
#endif
				}
			}
			else {
				if (mmr_flag) {
					mmr_flag = FALSE;
#if XM7_VER >= 3
					if (!mmr_fastmode) {
						mmr_modify = TRUE;
					}
#else
					mmr_modify = TRUE;
#endif
				}
			}
			if (dat & 0x40) {
				if (!twr_flag) {
					twr_flag = TRUE;
#if XM7_VER >= 3
					if (!mmr_fastmode) {
						mmr_modify = TRUE;
					}
#else
					mmr_modify = TRUE;
#endif
				}
			}
			else {
				if (twr_flag) {
					twr_flag = FALSE;
#if XM7_VER >= 3
					if (!mmr_fastmode) {
						mmr_modify = TRUE;
					}
#else
					mmr_modify = TRUE;
#endif
				}
			}
			if (dat & 0x01) {
				bootram_rw = TRUE;
			}
			else {
				bootram_rw = FALSE;
			}
			return TRUE;

#if XM7_VER >= 3
		/* 拡張MMR/CPUスピード */
		case 0xfd94:
			if (fm7_ver >= 3) {
				/* bit7:拡張MMR */
				if (dat & 0x80) {
					mmr_ext = TRUE;
				}
				else {
					mmr_ext = FALSE;
				}
				/* bit2:リフレッシュスピード */
				if (dat & 0x04) {
					if (!mmr_fast_refresh) {
						mmr_fast_refresh = TRUE;
						if (!mmr_fastmode) {
							mmr_modify = TRUE;
						}
					}
				}
				else {
					if (mmr_fast_refresh) {
						mmr_fast_refresh = FALSE;
						if (!mmr_fastmode) {
							mmr_modify = TRUE;
						}
					}
				}
				/* bit0:ウィンドウスピード */
				if (dat & 0x01) {
					if (!twr_fastmode) {
						twr_fastmode = TRUE;
						if (!mmr_fastmode) {
							mmr_modify = TRUE;
						}
					}
				}
				else {
					if (twr_fastmode) {
						twr_fastmode = FALSE;
						if (!mmr_fastmode) {
							mmr_modify = TRUE;
						}
					}
				}
			}
			return TRUE;

		/* モードセレクト２ */
		case 0xfd95:
			if (fm7_ver >= 3) {
				/* bit7:拡張ROMセレクト */
				if (dat & 0x80) {
					extrom_sel = TRUE;
				}
				else {
					extrom_sel = FALSE;
				}
				/* bit4:MMR使用時の速度低下を抑止 */
				if (dat & 0x08) {
					if (!mmr_fastmode) {
						mmr_fastmode = TRUE;
						mmr_modify = TRUE;
					}
				}
				else {
					if (mmr_fastmode) {
						mmr_fastmode = FALSE;
						mmr_modify = TRUE;
					}
				}
			}
			return TRUE;

#if defined(MR2)
		/* MR2 データレジスタ */
		case 0xfd9c:
			if (mmr_768kbmode) {
				mr2_write_data(dat);
			}
			return TRUE;

		/* MR2 セクタレジスタ */
		case 0xfd9e:
		case 0xfd9f:
			mr2_update_sector(addr, dat);
			return TRUE;
#endif

#if defined(RD512)
		/* RD512 セクタレジスタ */
		case 0xfd40:
		case 0xfd41:
			rd512_update_sector(addr, dat);
			return TRUE;

		/* RD512 データレジスタ */
		case 0xfd48:
		case 0xfd49:
		case 0xfd4a:
		case 0xfd4b:
		case 0xfd4c:
		case 0xfd4d:
		case 0xfd4e:
		case 0xfd4f:
			if (mmr_extram) {
				rd512_write_data(addr, dat);
			}
			return TRUE;
#endif

#if defined(R2D2)
		/* R2D2 データレジスタ */
		case 0xfd60:
		case 0xfd61:
			if (mmr_extram) {
				r2d2_write_data(addr, dat);
			}
			return TRUE;

		/* R2D2 セクタレジスタ */
		case 0xfd62:
		case 0xfd63:
			r2d2_update_sector(addr, dat);
			return TRUE;
#endif
#endif
	}

	/* MMRレジスタ */
	if ((addr >= 0xfd80) && (addr <= 0xfd8f)) {
#if XM7_VER >= 3
		/* ここでのデータは8bitすべて記憶 */
		if (mmr_ext) {
			mmr_reg[mmr_seg * 0x10 + (addr - 0xfd80)] = (BYTE)dat;
		}
		else {
			mmr_reg[(mmr_seg & 3) * 0x10 + (addr - 0xfd80)] = (BYTE)dat;
		}
#else
		mmr_reg[(mmr_seg & 3) * 0x10 + (addr - 0xfd80)] = (BYTE)dat;
#endif
		return TRUE;
	}

	return FALSE;
}

/*-[ ファイルI/O ]----------------------------------------------------------*/

/*
 *	MMR
 *	セーブ
 */
BOOL FASTCALL mmr_save(int fileh)
{
	if (!file_bool_write(fileh, mmr_flag)) {
		return FALSE;
	}
	if (!file_byte_write(fileh, mmr_seg)) {
		return FALSE;
	}

#if XM7_VER >= 3
	/* Ver8拡張部 */
	if (!file_write(fileh, mmr_reg, 0x80)) {
		return FALSE;
	}
	if (!file_bool_write(fileh, mmr_ext)) {
		return FALSE;
	}
	if (!file_bool_write(fileh, mmr_fastmode)) {
		return FALSE;
	}
	if (!file_bool_write(fileh, mmr_extram)) {
		return FALSE;
	}
	if (mmr_extram) {
		if (!file_write(fileh, extram_c, 0xc0000)) {
			return FALSE;
		}
	}
#if defined(MR2)
	if (!file_byte_write(fileh, mr2_nowcnt)) {
		return FALSE;
	}
	if (!file_word_write(fileh, mr2_secreg)) {
		return FALSE;
	}
#else
	if (!file_byte_write(fileh, 0)) {
		return FALSE;
	}
	if (!file_word_write(fileh, 0)) {
		return FALSE;
	}
#endif
#else
	if (!file_write(fileh, mmr_reg, 0x40)) {
		return FALSE;
	}
#endif
#if XM7_VER == 1
	if (!file_byte_write(fileh, bnk_reg)) {
		return FALSE;
	}
	if (!file_bool_write(fileh, mmr_64kbmode)) {
		return FALSE;
	}
#endif

	if (!file_bool_write(fileh, twr_flag)) {
		return FALSE;
	}
	if (!file_byte_write(fileh, twr_reg)) {
		return FALSE;
	}
#if XM7_VER >= 3
	if (!file_bool_write(fileh, twr_fastmode)) {
		return FALSE;
	}
	if (!file_bool_write(fileh, mmr_768kbmode)) {
		return FALSE;
	}
#endif

	return TRUE;
}

/*
 *	MMR
 *	ロード
 */
BOOL FASTCALL mmr_load(int fileh, int ver)
{
#if (XM7_VER >= 3) && !defined(MR2)
	WORD tmp;
#endif

	/* バージョンチェック */
	if (ver < 200) {
		return FALSE;
	}

	if (!file_bool_read(fileh, &mmr_flag)) {
		return FALSE;
	}
	if (!file_byte_read(fileh, &mmr_seg)) {
		return FALSE;
	}

#if XM7_VER >= 3
	/* ファイルバージョン8で拡張 */
	if (ver >= 800) {
		if (!file_read(fileh, mmr_reg, 0x80)) {
			return FALSE;
		}
		if (!file_bool_read(fileh, &mmr_ext)) {
			return FALSE;
		}
		if (!file_bool_read(fileh, &mmr_fastmode)) {
			return FALSE;
		}
		if (!file_bool_read(fileh, &mmr_extram)) {
			return FALSE;
		}
		if (mmr_extram) {
			if (!file_read(fileh, extram_c, 0xc0000)) {
				return FALSE;
			}
		}
		if (ver >= 902) {
#if defined(MR2)
			if (!file_byte_read(fileh, &mr2_nowcnt)) {
				return FALSE;
			}
			if (!file_word_read(fileh, &mr2_secreg)) {
				return FALSE;
			}
#else
			if (!file_byte_read(fileh, (BYTE *)&tmp)) {
				return FALSE;
			}
			if (!file_word_read(fileh, &tmp)) {
				return FALSE;
			}
#endif
		}
#if defined(MR2)
		else {
			mr2_nowcnt = 0;
			mr2_secreg = 0;
		}
#endif
	}
	else {
		/* Ver5互換 */
		if (!file_read(fileh, mmr_reg, 0x40)) {
			return FALSE;
		}
		mmr_ext = FALSE;
		mmr_fastmode = FALSE;
		mmr_extram = FALSE;
#if defined(MR2)
		mr2_nowcnt = 0;
		mr2_secreg = 0;
#endif
	}
#else
	if (!file_read(fileh, mmr_reg, 0x40)) {
		return FALSE;
	}
#endif
#if XM7_VER == 1
	if (ver >= 309) {
		if (!file_byte_read(fileh, &bnk_reg)) {
			return FALSE;
		}
		if (!file_bool_read(fileh, &mmr_64kbmode)) {
			return FALSE;
		}
	}
#endif

	if (!file_bool_read(fileh, &twr_flag)) {
		return FALSE;
	}
	if (!file_byte_read(fileh, &twr_reg)) {
		return FALSE;
	}

#if XM7_VER >= 3
	/* Ver9.16拡張 */
	if (ver >= 916) {
		if (!file_bool_read(fileh, &twr_fastmode)) {
			return FALSE;
		}
	}
	/* Ver9.20拡張 */
	if (ver >= 920) {
		if (!file_bool_read(fileh, &mmr_768kbmode)) {
			return FALSE;
		}
	}
#endif

	return TRUE;
}
