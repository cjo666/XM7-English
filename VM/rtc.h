/*
 *	FM-7 EMULATOR "XM7"
 *
 *	Copyright (C) 1999-2017 �o�h�D(Twitter:@xm6_original)
 *	Copyright (C) 2001-2017 Ryu Takegami (Twitter:@RyuTakegami)
 *
 *	[ ���A���^�C���N���b�N (MS58321) ]
 */

#ifndef _rtc_h_
#define _rtc_h_

#if XM7_VER >= 2

#ifdef __cplusplus
extern "C" {
#endif
/*
 *	��v�G���g��
 */
BOOL FASTCALL rtc_init(void);
										/* ������ */
void FASTCALL rtc_cleanup(void);
										/* �N���[���A�b�v */
void FASTCALL rtc_reset(void);
										/* ���Z�b�g */
void FASTCALL rtc_time_adjust(void);
										/* �����A�W���X�g */
void FASTCALL rtc_set(BYTE *packet);
										/* ���v�Z�b�g */
void FASTCALL rtc_get(BYTE *packet);
										/* ���v�擾 */
BOOL FASTCALL rtc_save(int fileh);
										/* �Z�[�u */
BOOL FASTCALL rtc_load(int fileh, int ver);
										/* ���[�h */

/*
 *	��v���[�N
 */
extern BYTE rtc_year;
										/* ���v �N(00�`99) */
extern BYTE rtc_month;
										/* ���v ��(1�`12) */
extern BYTE rtc_day;
										/* ���v ��(0�`31) */
extern BYTE rtc_week;
										/* ���v �j��(0�`6) */
extern BYTE rtc_hour;
										/* ���v ��(0�`12 or 0�`24h) */
extern BYTE rtc_minute;
										/* ���v ��(0�`59) */
extern BYTE rtc_second;
										/* ���v �b(0�`59) */
extern BOOL rtc_24h;
										/* ���v 12h/24h�؂�ւ� */
extern BOOL rtc_pm;
										/* ���v AM/PM�t���O */
extern BYTE rtc_leap;
										/* ���v �[�N����[�� */

#ifdef __cplusplus
}
#endif

#endif	/* XM7_VER >= 2 */
#endif	/* _rtc_h_ */
