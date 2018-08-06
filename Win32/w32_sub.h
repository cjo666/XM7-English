/*
 *	FM-7 EMULATOR "XM7"
 *
 *	Copyright (C) 1999-2017 ＰＩ．(Twitter:@xm6_original)
 *	Copyright (C) 2001-2017 Ryu Takegami (Twitter:@RyuTakegami)
 *
 *	[ Win32API サブウインドウ ]
 */

#ifdef _WIN32

#ifndef _w32_sub_h_
#define _w32_sub_h_

/*
 *	サブウインドウ定義
 */
#define SWND_BREAKPOINT			0		/* ブレークポイント */
#define SWND_SCHEDULER			1		/* スケジューラ */
#if XM7_VER == 1
#define SWND_CPUREG_MAIN		2		/* CPUレジスタ メイン(6809) */
#define SWND_CPUREG_SUB			3		/* CPUレジスタ サブ */
#define SWND_CPUREG_JSUB		4		/* CPUレジスタ 日本語サブ */
#define SWND_CPUREG_Z80			5		/* CPUレジスタ メイン(Z80) */
#define SWND_DISASM_MAIN		6		/* 逆アセンブル メイン(6809) */
#define SWND_DISASM_SUB			7		/* 逆アセンブル サブ */
#define SWND_DISASM_JSUB		8		/* 逆アセンブル 日本語サブ */
#define SWND_DISASM_Z80			9		/* 逆アセンブル メイン(Z80) */
#define SWND_MEMORY_MAIN		10		/* メモリダンプ メイン */
#define SWND_MEMORY_SUB			11		/* メモリダンプ サブ */
#define SWND_MEMORY_JSUB		12		/* メモリダンプ 日本語サブ */
#define SWND_FDC				13		/* FDC */
#define SWND_OPNREG				14		/* OPNレジスタ */
#define SWND_OPNDISP			15		/* OPNディスプレイ */
#define SWND_SUBCTRL			16		/* サブCPUコントロール */
#define SWND_KEYBOARD			17		/* キーボード */
#define SWND_MMR				18		/* MMR */
#define SWND_PALETTE			19		/* パレットレジスタ */
#define SWND_BMC				20		/* バブルメモリコントローラ */
#define SWND_MAXNUM				21
#else
#define SWND_CPUREG_MAIN		2		/* CPUレジスタ メイン */
#define SWND_CPUREG_SUB			3		/* CPUレジスタ サブ */
#define SWND_DISASM_MAIN		4		/* 逆アセンブル メイン */
#define SWND_DISASM_SUB			5		/* 逆アセンブル サブ */
#define SWND_MEMORY_MAIN		6		/* メモリダンプ メイン */
#define SWND_MEMORY_SUB			7		/* メモリダンプ サブ */
#define SWND_FDC				8		/* FDC */
#define SWND_OPNREG				9		/* OPNレジスタ */
#define SWND_OPNDISP			10		/* OPNディスプレイ */
#define SWND_SUBCTRL			11		/* サブCPUコントロール */
#define SWND_ALULINE			12		/* 論理演算/直線補間 */
#define SWND_KEYBOARD			13		/* キーボード */
#define SWND_MMR				14		/* MMR */
#define SWND_PALETTE			15		/* パレットレジスタ */
#if XM7_VER >= 3
#define	SWND_DMAC				16		/* DMAC */
#define SWND_MAXNUM				17		/* 最大サブウインドウ数 */
#else
#define SWND_MAXNUM				16		/* 最大サブウインドウ数 */
#endif
#endif

#ifdef __cplusplus
extern "C" {
#endif
/*
 *	主要エントリ
 */
void FASTCALL InitSubWndWork(void);
										/* サブウィンドウワーク初期化 */

HWND FASTCALL CreateBreakPoint(HWND hParent, int index);
										/* ブレークポイントウインドウ 作成 */
void FASTCALL RefreshBreakPoint(void);
										/* ブレークポイントウインドウ リフレッシュ */
HWND FASTCALL CreateScheduler(HWND hParent, int index);
										/* スケジューラウインドウ 作成 */
void FASTCALL RefreshScheduler(void);
										/* スケジューラウインドウ リフレッシュ */
HWND FASTCALL CreateCPURegister(HWND hParent, BYTE nCPU, int index);
										/* CPUレジスタウインドウ 作成 */
void FASTCALL RefreshCPURegister(void);
										/* CPUレジスタウインドウ リフレッシュ */
HWND FASTCALL CreateDisAsm(HWND hParent, BYTE nCPU, int index);
										/* 逆アセンブルウインドウ 作成 */
void FASTCALL RefreshDisAsm(void);
										/* 逆アセンブルウインドウ リフレッシュ */
void FASTCALL AddrDisAsm(BYTE nCPU, DWORD dwAddr);
										/* 逆アセンブルウインドウ アドレス指定 */
HWND FASTCALL CreateMemory(HWND hParent, BYTE nCPU, int index);
										/* メモリダンプウインドウ 作成 */
void FASTCALL RefreshMemory(void);
										/* メモリダンプウインドウ リフレッシュ */
void FASTCALL AddrMemory(BYTE nCPU, DWORD dwAddr);
										/* メモリダンプウインドウ アドレス指定 */

HWND FASTCALL CreateFDC(HWND hParent, int index);
										/* FDCウインドウ 作成 */
void FASTCALL RefreshFDC(void);
										/* FDCウインドウ リフレッシュ */
#if XM7_VER ==1 && defined(BUBBLE)
HWND FASTCALL CreateBMC(HWND hParent, int index);
										/* BMCウインドウ 作成 */
void FASTCALL RefreshBMC(void);
										/* BMCウインドウ リフレッシュ */
#endif
HWND FASTCALL CreateOPNReg(HWND hParent, int index);
										/* OPNレジスタウインドウ 作成 */
void FASTCALL RefreshOPNReg(void);
										/* OPNレジスタウインドウ リフレッシュ */
void FASTCALL ReSizeOPNReg(void);
										/* OPNレジスタウインドウ リサイズ */
HWND FASTCALL CreateSubCtrl(HWND hParent, int index);
										/* サブCPUコントロールウインドウ 作成 */
void FASTCALL RefreshSubCtrl(void);
										/* サブCPUコントロールウインドウ リフレッシュ */
HWND FASTCALL CreateALULine(HWND hParent, int index);
										/* 論理演算/直線補間ウインドウ 作成 */
void FASTCALL RefreshALULine(void);
										/* 論理演算/直線補間ウインドウ リフレッシュ */
HWND FASTCALL CreateOPNDisp(HWND hParent, int index);
										/* OPNディスプレイウインドウ 作成 */
void FASTCALL RefreshOPNDisp(void);
										/* OPNディスプレイウインドウ リフレッシュ */
void FASTCALL ReSizeOPNDisp(void);
										/* OPNディスプレイウインドウ リサイズ */
HWND FASTCALL CreateKeyboard(HWND hParent, int index);
										/* キーボードウインドウ 作成 */
void FASTCALL RefreshKeyboard(void);
										/* キーボードウインドウ リフレッシュ */
HWND FASTCALL CreateMMR(HWND hParent, int index);
										/* MMRウインドウ 作成 */
void FASTCALL RefreshMMR(void);
										/* MMRウインドウ リフレッシュ */
#if XM7_VER >= 3
HWND FASTCALL CreateDMAC(HWND hParent, int index);
										/* DMACウインドウ 作成 */
void FASTCALL RefreshDMAC(void);
										/* DMACウインドウ リフレッシュ */
#endif
HWND FASTCALL CreatePaletteReg(HWND hParent, int index);
										/* パレットレジスタウインドウ 作成 */
void FASTCALL RefreshPaletteReg(void);
										/* パレットレジスタウインドウ リフレッシュ */

/*
 *	内部汎用ルーチン
 */
void FASTCALL DrawWindowText(HDC hDC, BYTE *ptr, int x, int y);
										/* テキスト描画 */
void FASTCALL DrawWindowText2(HDC hDC, BYTE *ptr, int x, int y);
										/* テキスト描画(反転付き) */
void FASTCALL DestroySubWindow(HWND hWnd, BYTE **pBuf, HMENU hmenu);
										/* サブウインドウDestroy */
void FASTCALL PositioningSubWindow(HWND hParent, LPRECT rect, int index);
										/* サブウィンドウ位置算出 */

/*
 *	主要ワーク
 */
extern HWND hSubWnd[SWND_MAXNUM];
										/* サブウインドウ */
extern BOOL bShowSubWindow[SWND_MAXNUM];
										/* サブウインドウ表示状態 */
extern BOOL bPopupSwnd;
										/* サブウィンドウポップアップ状態 */
extern BOOL bPaletteRefresh;
										/* パレットリフレッシュ */
#ifdef __cplusplus
}
#endif

#endif	/* _w32_sub_h_ */
#endif	/* _WIN32 */
