/*
 *	FM-7 EMULATOR "XM7"
 *
 *	Copyright (C) 1999-2017 ＰＩ．(Twitter:@xm6_original)
 *	Copyright (C) 2001-2017 Ryu Takegami (Twitter:@RyuTakegami)
 *
 *	[ Win32 シリアルポート ]
 */

#ifdef _WIN32

#ifndef _w32_comm_h_
#define _w32_comm_h_

#if defined(RSC)

#ifdef __cplusplus
extern "C" {
#endif
/*
 *	主要エントリ
 */
BOOL FASTCALL InitCommPort(void);
										/* 初期化 */
void FASTCALL CleanCommPort(void);
										/* クリーンアップ */
BOOL FASTCALL SelectCommPort(void);
										/* セレクト */
BOOL FASTCALL SelectCheckCommPort(int port);
										/* セレクト状態チェック */

/*
 *	主要ワーク
 */
extern int nCommPortNo;
										/* 使用するポート番号 */
#ifdef __cplusplus
}
#endif

#endif	/* XM7_VER >= 3 */
#endif	/* _w32_comm_h_ */
#endif	/* _WIN32 */
