/*
 *	FM-7 EMULATOR "XM7"
 *
 *	Copyright (C) 1999-2017 ＰＩ．(Twitter:@xm6_original)
 *	Copyright (C) 2001-2017 Ryu Takegami (Twitter:@RyuTakegami)
 *
 *	[ メインCPU各種I/O ]
 */

#ifndef _mainetc_h_
#define _mainetc_h_

/*
 *	定数定義
 */
#define	BANKSEL_DISABLE			0
#define	BANKSEL_ENABLE			1
#define	BANKSEL_ENABLE_DIPSW	2

#ifdef __cplusplus
extern "C" {
#endif
/*
 *	主要エントリ
 */
extern BOOL FASTCALL mainetc_init(void);
										/* 初期化 */
extern void FASTCALL mainetc_cleanup(void);
										/* クリーンアップ */
extern void FASTCALL mainetc_reset(void);
										/* リセット */
extern BOOL FASTCALL mainetc_readb(WORD addr, BYTE *dat);
										/* メモリ読み出し */
extern BOOL FASTCALL mainetc_writeb(WORD addr, BYTE dat);
										/* メモリ書き込み */
extern BOOL FASTCALL mainetc_save(int fileh);
										/* セーブ */
extern BOOL FASTCALL mainetc_load(int fileh, int ver);
										/* ロード */
extern void FASTCALL mainetc_fdc(void);
										/* FDC割り込み */
extern void FASTCALL mainetc_lp(void);
										/* ラインプリンタ割り込み */
extern BOOL FASTCALL mainetc_beep(void);
										/* BEEPイベント */

/*
 *	主要ワーク
 */
extern BOOL key_irq_flag;
										/* キーボード割り込み 要求 */
extern BOOL key_irq_mask;
										/* キーボード割り込み マスク */
extern BOOL lp_irq_flag;
										/* プリンタ割り込み 要求 */
extern BOOL lp_irq_mask;
										/* プリンタ割り込み マスク */
extern BOOL timer_irq_flag;
										/* タイマー割り込み 要求 */
extern BOOL timer_irq_mask;
										/* タイマー割り込み マスク */
extern BOOL mfd_irq_flag;
										/* FDC割り込み フラグ */
extern BOOL mfd_irq_mask;
										/* FDC割り込み マスク */
extern BOOL txrdy_irq_flag;
										/* TxRDY割り込み フラグ */
extern BOOL txrdy_irq_mask;
										/* TxRDY割り込み マスク */
extern BOOL rxrdy_irq_flag;
										/* RxRDY割り込み フラグ */
extern BOOL rxrdy_irq_mask;
										/* RxRDY割り込み マスク */
extern BOOL syndet_irq_flag;
										/* SYNDET割り込み フラグ */
extern BOOL syndet_irq_mask;
										/* SYNDET割り込み マスク */
extern BOOL ptm_irq_flag;
										/* PTM割り込み フラグ */
extern BOOL ptm_irq_mask;
										/* PTM割り込み マスク */
extern BOOL opn_irq_flag;
										/* OPN割り込み フラグ */
extern BOOL whg_irq_flag;
										/* WHG割り込み フラグ */
extern BOOL thg_irq_flag;
										/* THG割り込み フラグ */
#if XM7_VER >= 3
extern BOOL dma_irq_flag;
										/* DMA割り込み フラグ */
#endif
extern BOOL beep_flag;
										/* BEEP有効フラグ */
extern BOOL speaker_flag;
										/* スピーカ有効フラグ */
#if XM7_VER == 1
extern BYTE banksel_en;
										/* バンク切り換えイネーブルフラグ */
#endif
#ifdef __cplusplus
}
#endif

#endif	/* _mainetc_h_ */
