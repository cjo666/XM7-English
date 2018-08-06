/*
 *	FM-7 EMULATOR "XM7"
 *
 *	Copyright (C) 1999-2017 �o�h�D(Twitter:@xm6_original)
 *	Copyright (C) 2001-2017 Ryu Takegami (Twitter:@RyuTakegami)
 *
 *	[ Win32 �V���A���|�[�g ]
 */

#ifdef _WIN32

#ifndef _w32_comm_h_
#define _w32_comm_h_

#if defined(RSC)

#ifdef __cplusplus
extern "C" {
#endif
/*
 *	��v�G���g��
 */
BOOL FASTCALL InitCommPort(void);
										/* ������ */
void FASTCALL CleanCommPort(void);
										/* �N���[���A�b�v */
BOOL FASTCALL SelectCommPort(void);
										/* �Z���N�g */
BOOL FASTCALL SelectCheckCommPort(int port);
										/* �Z���N�g��ԃ`�F�b�N */

/*
 *	��v���[�N
 */
extern int nCommPortNo;
										/* �g�p����|�[�g�ԍ� */
#ifdef __cplusplus
}
#endif

#endif	/* XM7_VER >= 3 */
#endif	/* _w32_comm_h_ */
#endif	/* _WIN32 */
