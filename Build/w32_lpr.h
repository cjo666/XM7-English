/*
 *	FM-7 EMULATOR "XM7LP"
 *
 *	Copyright (C) 1999-2017 ＰＩ．(Twitter:@xm6_original)
 *	Copyright (C) 2001-2017 Ryu Takegami (Twitter:@RyuTakegami)
 *	line printer support 2010-2013 by Ben.JP
 *
 *	[ Win32 プリンタ出力 ]
 */

#ifdef _WIN32

#ifndef _w32_lpr_h_
#define _w32_lpr_h_

#if defined(LPRINT)

#ifdef __cplusplus
extern "C" {
#endif

/*
 *	主要エントリ
 */

/*
 *	主要ワーク
 */
extern BOOL lpr_use_os_font;
										/* OSのフォントを利用する */
extern BOOL lpr_output_kanji;
										/* 漢字を出力する */

#ifdef __cplusplus
}
#endif

#endif	/* LPRINT */
#endif	/* _w32_lpr_h_ */
#endif	/* _WIN32 */
