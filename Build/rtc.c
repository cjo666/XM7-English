/*
 *	FM-7 EMULATOR "XM7"
 *
 *	Copyright (C) 1999-2017 �o�h�D(Twitter:@xm6_original)
 *	Copyright (C) 2001-2017 Ryu Takegami (Twitter:@RyuTakegami)
 *
 *	[ ���A���^�C���N���b�N(MS58321) ]
 *
 *	RHG����
 *	  2002.07.17		�����A�W���X�g����ms�P�ʂŎ����������s���悤�ɕύX
 *						�����A�W���X�g����AM/PM�t���O������ɐݒ肳��Ȃ����Ƃ�
 *						��������C��
 */

#if XM7_VER >= 2

#include <assert.h>
#include <time.h>
#include "xm7.h"
#include "event.h"
#include "rtc.h"
#include "device.h"

/*
 *	�O���[�o�� ���[�N
 */
BYTE rtc_year;							/* ���v �N(00�`99) */
BYTE rtc_month;							/* ���v ��(1�`12) */
BYTE rtc_day;							/* ���v ��(0�`31) */
BYTE rtc_week;							/* ���v �j��(0�`6) */
BYTE rtc_hour;							/* ���v ��(0�`12 or 0�`24h) */
BYTE rtc_minute;						/* ���v ��(0�`59) */
BYTE rtc_second;						/* ���v �b(0�`59) */
BOOL rtc_24h;							/* ���v 12h/24h�؂�ւ� */
BOOL rtc_pm;							/* ���v AM/PM�t���O */
BYTE rtc_leap;							/* ���v �[�N����[�� */

/*
 *	�X�^�e�B�b�N ���[�N
 */
static BOOL rtc_init_flag;				/* ���v �������t���O */
static time_t rtc_ltime;				/* ���v �����(�����p) */

/*
 *	���|�����Ή��e�[�u��
 */
static BYTE rtc_day_table[] = {			/* 2����28���ɃZ�b�g */
	31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

/*
 *	�v���g�^�C�v�錾
 */
static BOOL FASTCALL rtc_event(void);	/* ���v�C�x���g */
static BOOL FASTCALL rtc_event_adjust(void);	/* ���v�C�x���g(���������p) */
static void FASTCALL rtc_time_adjust_sub(void);	/* �����A�W���X�g�T�u */


/*
 *	���v
 *	������
 */
BOOL FASTCALL rtc_init(void)
{
	/* ���v�����Z�b�g�B24h, �[�N0 */
	rtc_24h = TRUE;
	rtc_leap = 0;

	/* �������t���O�����Z�b�g */
	rtc_init_flag = FALSE;

	return TRUE;
}

/*
 *	���v
 *	�N���[���A�b�v
 */
void FASTCALL rtc_cleanup(void)
{
}

/*
 *	���v
 *	���Z�b�g
 */
void FASTCALL rtc_reset(void)
{
	/* �N�����̂ݎ�����ݒ肷��K�v������ */
	if (!rtc_init_flag) {
		rtc_time_adjust();
		rtc_init_flag = TRUE;
	}
}

/*
 *	���v
 *	�����A�W���X�g
 */
void FASTCALL rtc_time_adjust(void)
{
	/* �������񎞍��������s�� */
	rtc_time_adjust_sub();

	/* ������������� */
	rtc_ltime = 0;

	/* ���������C�x���g��ݒ� */
	schedule_setevent(EVENT_RTC, 1000, rtc_event_adjust);
}

/*
 *	���v
 *	�����A�W���X�g�C�x���g
 */
static BOOL FASTCALL rtc_event_adjust(void)
{
	if (rtc_ltime == 0) {
		/* ��������擾 */
		rtc_ltime = time(NULL);
	}
	else {
		/* �������ς��܂őҋ@ */
		if (rtc_ltime != time(NULL)) {
			/* �ēx�A�����������s��(�C�x���g�̍Đݒ���s����) */
			rtc_time_adjust_sub();
		}
	}

	return TRUE;
}

/*
 *	���v
 *	�����A�W���X�g�T�u
 */
static void FASTCALL rtc_time_adjust_sub(void)
{
	time_t ltime;
	struct tm *now;

	/* ���݂̎��Ԃ�ǂݎ��A�Z�b�g */
	ltime = time(NULL);
	now = localtime(&ltime);
	rtc_year = (BYTE)(now->tm_year % 100);
	rtc_month = (BYTE)(now->tm_mon + 1);
	rtc_day = (BYTE)now->tm_mday;
	rtc_week = (BYTE)now->tm_wday;
	rtc_hour = (BYTE)now->tm_hour;
	rtc_minute = (BYTE)now->tm_min;
	rtc_second = (BYTE)now->tm_sec;

	/* AM/PM�t���O��ݒ� */
	if (rtc_hour >= 12) {
		rtc_pm = TRUE;
	}
	else {
		rtc_pm = FALSE;
	}

	/* �C�x���g��ݒ� */
	schedule_setevent(EVENT_RTC, 1000 * 1000, rtc_event);
}

/*
 *	���v
 *	�C�x���g(1sec)
 */
static BOOL FASTCALL rtc_event(void)
{
	/* �b�A�b�v */
	rtc_second++;
	if (rtc_second < 60) {
		return TRUE;
	}
	rtc_second = 0;

	/* ���A�b�v */
	rtc_minute++;
	if (rtc_minute < 60) {
		return TRUE;
	}
	rtc_minute = 0;

	/* ���A�b�v */
	rtc_hour++;
	if (rtc_24h) {
		/* 24h */
		if (rtc_hour >= 12) {
			rtc_pm = TRUE;
		}
		else {
			rtc_pm = FALSE;
		}
		if (rtc_hour < 24) {
			return TRUE;
		}
	}
	else {
		/* 12h */
		if (rtc_pm) {
			/* PM */
			if (rtc_hour < 12) {
				return TRUE;
			}
		}
		else {
			/* AM */
			if (rtc_hour < 12) {
				return TRUE;
			}
			rtc_hour = 0;
			rtc_pm = TRUE;
			return TRUE;
		}
	}
	rtc_hour = 0;
	rtc_pm = FALSE;

	/* �j���A�b�v */
	rtc_week++;
	if (rtc_week > 6) {
		rtc_week = 0;
	}

	/* ���A�b�v */
	rtc_day++;
	if (rtc_day <= rtc_day_table[rtc_month]) {
		return TRUE;
	}

	/* �[�N�̃`�F�b�N���A�����œ���� */
	if ((rtc_month == 2) && (rtc_day == 29)) {
		/* 2��29���Ȃ̂ŁA�[�N�Ȃ�return TRUE */
		if (rtc_leap == 0) {
			if ((rtc_year % 4) == 0) {
				return TRUE;
			}
		}
		else {
			if ((rtc_year % 4) == (4 - rtc_leap)) {
				return TRUE;
			}
		}
	}
	rtc_day = 1;

	/* ���A�b�v */
	rtc_month++;
	if (rtc_month <= 12) {
		return TRUE;
	}
	rtc_month = 0;

	/* �N�A�b�v */
	rtc_year++;
	if (rtc_year > 99) {
		rtc_year = 0;
	}

	return TRUE;
}

/*-[ I/O(�L�[�{�[�h�R���g���[�����) ]--------------------------------------*/

/*
 *	���v
 *	�f�[�^�Z�b�g(��������)
 */
void FASTCALL rtc_set(BYTE *packet)
{
	BYTE dat;

	ASSERT(packet);

	/* �o�[�W�����`�F�b�N */
	if (fm7_ver < 2) {
		return;
	}

	/* �N */
	dat = *packet++;
	rtc_year = (BYTE)((dat >> 4) * 10);
	rtc_year |= (BYTE)(dat & 0x0f);

	/* �� */
	dat = *packet++;
	rtc_month = (BYTE)((dat >> 4) * 10);
	rtc_month |= (BYTE)(dat & 0x0f);

	/* �� + �[�N */
	dat = *packet++;
	rtc_day = (BYTE)(((dat & 0x30) >> 4) * 10);
	rtc_day |= (BYTE)(dat & 0x0f);

	/* �j��,12/24,���̏�� */
	dat = *packet++;
	rtc_week = (BYTE)((dat >> 4) & 0x07);
	rtc_hour = (BYTE)((dat & 0x03) * 10);
	if (dat & 0x08) {
		rtc_24h = TRUE;
	}
	else {
		rtc_24h = FALSE;
	}
	if (dat & 0x04) {
		rtc_pm = TRUE;
	}
	else {
		rtc_pm = FALSE;
	}

	/* ���̉��ʁA���̏�� */
	dat = *packet++;
	rtc_hour |= (BYTE)(dat >> 4);
	rtc_minute = (BYTE)((dat & 0x0f) * 10);

	/* ���̉��ʁA�b�̏�� */
	dat = *packet++;
	rtc_minute |= (BYTE)(dat >> 4);
	rtc_second = (BYTE)((dat & 0x0f) * 10);

	/* �b�̉��� */
	dat = *packet;
	rtc_second |= (BYTE)(dat >> 4);
}

/*
 *	���v
 *	�f�[�^�擾(�ǂݍ���)
 */
void FASTCALL rtc_get(BYTE *packet)
{
	BYTE dat;

	ASSERT(packet);

	/* �o�[�W�����`�F�b�N */
	if (fm7_ver < 2) {
		return;
	}

	/* �N */
	dat = (BYTE)((rtc_year / 10) << 4);
	dat |= (BYTE)(rtc_year % 10);
	*packet++ = dat;

	/* �� */
	dat = (BYTE)((rtc_month / 10) << 4);
	dat |= (BYTE)(rtc_month % 10);
	*packet++ = dat;

	/* �� + �[�N */
	dat = (BYTE)((rtc_day / 10) << 4);
	dat |= (BYTE)(rtc_day % 10);
	dat |= (BYTE)(rtc_leap * 64);
	*packet++ = dat;

	/* �j��,12/24,���̏�� */
	dat = (BYTE)(rtc_week << 4);
	dat |= (BYTE)(rtc_hour / 10);
	if (rtc_24h) {
		dat |= 0x08;
	}
	if (!rtc_24h && rtc_pm) {
		dat |= 0x04;
	}
	*packet++ = dat;

	/* ���̉��ʁA���̏�� */
	dat = (BYTE)((rtc_hour % 10) << 4);
	dat |= (BYTE)(rtc_minute / 10);
	*packet++ = dat;

	/* ���̉��ʁA�b�̏�� */
	dat = (BYTE)((rtc_minute % 10) << 4);
	dat |= (BYTE)(rtc_second / 10);
	*packet++ = dat;

	/* �b�̉��� */
	dat = (BYTE)((rtc_second % 10) << 4);
	*packet = dat;
}

/*-[ �t�@�C��I/O ]----------------------------------------------------------*/

/*
 *	���v
 *	�Z�[�u
 */
BOOL FASTCALL rtc_save(int fileh)
{
	if (!file_bool_write(fileh, rtc_24h)) {
		return FALSE;
	}

	if (!file_bool_write(fileh, rtc_pm)) {
		return FALSE;
	}

	if (!file_byte_write(fileh, rtc_leap)) {
		return FALSE;
	}

	return TRUE;
}

/*
 *	���v
 *	���[�h
 */
BOOL FASTCALL rtc_load(int fileh, int ver)
{
	/* �o�[�W�����`�F�b�N */
	if (ver < 200) {
		return FALSE;
	}

	if (!file_bool_read(fileh, &rtc_24h)) {
		return FALSE;
	}

	if (!file_bool_read(fileh, &rtc_pm)) {
		return FALSE;
	}

	if (!file_byte_read(fileh, &rtc_leap)) {
		return FALSE;
	}

	/* ���݂̎��Ԃ�ǂݎ��A�Z�b�g */
	rtc_time_adjust();

	return TRUE;
}

#endif	/* XM7_VER >= 2 */
