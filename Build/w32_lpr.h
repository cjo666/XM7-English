/*
 *	FM-7 EMULATOR "XM7LP"
 *
 *	Copyright (C) 1999-2017 �o�h�D(Twitter:@xm6_original)
 *	Copyright (C) 2001-2017 Ryu Takegami (Twitter:@RyuTakegami)
 *	line printer support 2010-2013 by Ben.JP
 *
 *	[ Win32 �v�����^�o�� ]
 */

#ifdef _WIN32

#ifndef _w32_lpr_h_
#define _w32_lpr_h_

#if defined(LPRINT)

#ifdef __cplusplus
extern "C" {
#endif

/*
 *	��v�G���g��
 */

/*
 *	��v���[�N
 */
extern BOOL lpr_use_os_font;
										/* OS�̃t�H���g�𗘗p���� */
extern BOOL lpr_output_kanji;
										/* �������o�͂��� */

#ifdef __cplusplus
}
#endif

#endif	/* LPRINT */
#endif	/* _w32_lpr_h_ */
#endif	/* _WIN32 */
