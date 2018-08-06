/*
 *	FM-7 EMULATOR "XM7"
 *
 *	Copyright (C) 1999-2017 �o�h�D(Twitter:@xm6_original)
 *	Copyright (C) 2001-2017 Ryu Takegami (Twitter:@RyuTakegami)
 *
 *	[ RS-232C�C���^�t�F�[�X ]
 *
 *	RHG����
 *	  2003.09.30		�V��
 *	  2010.11.09		FM77AV�ȑO�̋@��ɑΉ��ł���悤�}�V���o�[�W�����`�F
 *						�b�N��ǉ�
 */

#if defined(RSC)

#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "xm7.h"
#include "event.h"
#include "device.h"
#include "mainetc.h"
#include "rs232c.h"

/*
 *	�O���[�o�� ���[�N
 */
BOOL	rs_use;						/* RS-232C�g�p�t���O */
BOOL	rs_mask;					/* RS-232C�@�\�}�X�N */
BOOL	rs_enable;					/* RS-232C�L�����t���O(AV40/20�ȍ~) */
BOOL	rs_selectmc;				/* ���[�h�R�}���h�I����� */
BYTE	rs_modecmd;					/* ���[�h�R�}���h���W�X�^ */
BYTE	rs_command;					/* �R�}���h���W�X�^ */
BYTE	rs_status;					/* �X�e�[�^�X���W�X�^ */
BYTE	rs_baudrate;				/* �{�[���[�g�ݒ背�W�X�^ */
BYTE	rs_baudrate_v2;				/* �{�[���[�g�ݒ�(V1/V2�p) */
BOOL	rs_dtrmask;					/* DTR�M���}�X�N�t���O */
BOOL	rs_cd;						/* CD�M�� */
BOOL	rs_cts;						/* CTS�M�� */

/*
 *	�X�^�e�B�b�N ���[�N
 */
static DWORD	sndrcv_timing;		/* �f�[�^����M�^�C�~���O(us) */


/*
 *	RS-232C
 *	������
 */
BOOL FASTCALL rs232c_init(void)
{
	rs_use = FALSE;
	rs_mask = TRUE;
	rs_cd = FALSE;

	return TRUE;
}

/*
 *	RS-232C
 *	�N���[���A�b�v
 */
void FASTCALL rs232c_cleanup(void)
{
}

/*
 *	RS-232C
 *	���Z�b�g
 */
void FASTCALL rs232c_reset(void)
{
	/* ���[�N�G���A������ */
	rs_enable = FALSE;
	rs_selectmc = TRUE;
#if XM7_VER >= 3
	if (fm7_ver <= 2) {
		rs_baudrate = (BYTE)(rs_baudrate_v2 << 2);
	}
	else {
		rs_baudrate = 0x00;
	}
#else
	rs_baudrate = (BYTE)(rs_baudrate_v2 << 2);
#endif
	rs_modecmd = 0x0c;
	rs_command = 0x40;
	rs_dtrmask = FALSE;
	rs_cts = FALSE;
	rs_status = (RSS_TXRDY | RSS_TXEMPTY);

	/* ���Z�b�g�ʒm */
	rs232c_reset_notify();

	/* ���W�X�^�ݒ� */
	rs232c_setbaudrate(rs_baudrate);
	rs232c_writemodecmd(rs_modecmd);
	rs232c_writecommand(rs_command);
	rs232c_calc_timing();
}

/*
 *	RS-232C
 *	TxRDY�M���ω�
 */
void FASTCALL rs232c_txrdy(BOOL flag)
{
	/* �X�e�[�^�X�X�V */
	if (flag) {
		rs_status |= RSS_TXRDY;
	}
	else {
		rs_status &= (BYTE)~(RSS_TXRDY | RSS_TXEMPTY);
	}

	/* ���荞�� */
#if XM7_VER == 1
	if ((fm_subtype == FMSUB_FM8 || !txrdy_irq_mask) &&
		 !rs_cts && (rs_status & RSS_TXEMPTY)) {
#else
	if (!txrdy_irq_mask && !rs_cts && (rs_status & RSS_TXEMPTY)) {
#endif
		txrdy_irq_flag = TRUE;
	}
	else {
		txrdy_irq_flag = FALSE;
	}
	maincpu_irq();
}

/*
 *	RS-232C
 *	TxEMPTY�C�x���g
 */
BOOL FASTCALL rs232c_txempty_event(void)
{
	/* TxEMPTY ON */
	rs_status |= RSS_TXEMPTY;

	/* TxRDY���荞�� */
	rs232c_txrdy(TRUE);

	/* �C�x���g�폜 */
	schedule_delevent(EVENT_RS_TXTIMING);

	return TRUE;
}

/*
 *	RS-232C
 *	TxEMPTY�C�x���g�o�^
 */
void FASTCALL rs232c_txempty_request(void)
{
	/* TxRDY ON */
	rs232c_txrdy(TRUE);

	/* �C�x���g�o�^ */
	schedule_setevent(EVENT_RS_TXTIMING, sndrcv_timing, rs232c_txempty_event);
}

/*
 *	RS-232C
 *	RxRDY�M���ω�
 */
void FASTCALL rs232c_rxrdy(BOOL flag)
{
	/* �X�e�[�^�X�X�V */
	if (flag) {
		rs_status |= RSS_RXRDY;
	}
	else {
		rs_status &= (BYTE)~RSS_RXRDY;
	}

	/* ���荞�� */
#if XM7_VER == 1
	if ((fm_subtype == FMSUB_FM8 || !rxrdy_irq_mask) &&
		(rs_status & RSS_RXRDY)) {
#else
	if (!rxrdy_irq_mask && (rs_status & RSS_RXRDY)) {
#endif
		rxrdy_irq_flag = TRUE;
	}
	else {
		rxrdy_irq_flag = FALSE;
	}
	maincpu_irq();
}

/*
 *	RS-232C
 *	RxRDY�C�x���g
 */
BOOL FASTCALL rs232c_rxrdy_event(void)
{
	/* RxRDY ON/���荞�� */
	rs232c_rxrdy(TRUE);

	/* �C�x���g�폜 */
	schedule_delevent(EVENT_RS_RXTIMING);

	return TRUE;
}

/*
 *	RS-232C
 *	RxRDY�C�x���g�o�^
 */
void FASTCALL rs232c_rxrdy_request(void)
{
	/* �C�x���g�o�^�c���� */
	schedule_setevent(EVENT_RS_RXTIMING, sndrcv_timing, rs232c_rxrdy_event);
}

/*
 *	RS-232C
 *	SYNDET�M���ω�
 */
void FASTCALL rs232c_syndet(BOOL flag)
{
	/* �X�e�[�^�X�X�V */
	if (flag) {
		rs_status |= RSS_SYNDET;
	}
	else {
		rs_status &= (BYTE)~RSS_SYNDET;
	}

	/* ���荞�� */
#if XM7_VER == 1
	if ((fm_subtype == FMSUB_FM8 || !syndet_irq_mask) &&
		(rs_status & RSS_SYNDET)) {
#else
	if (!syndet_irq_mask && (rs_status & RSS_SYNDET)) {
#endif
		syndet_irq_flag = TRUE;
	}
	else {
		syndet_irq_flag = FALSE;
	}
	maincpu_irq();
}

/*
 *	RS-232C
 *	����M�^�C�~���O�v�Z
 */
void FASTCALL rs232c_calc_timing(void)
{
	DWORD baudrate;
	BYTE databits;

	/* �{�[���[�g���v�Z */
	baudrate = 300 << ((rs_baudrate & RSCB_BAUDM) >> 2);
	if ((rs_modecmd & RSM_BAUDDIVM) == RSM_BAUDDIV16) {
		/* FAST���[�h */
		baudrate *= 4;
	}

	/* �L�����N�^�����擾 */
	switch (rs_modecmd & RSM_CHARLENM) {
		case RSM_CHARLEN5	:	databits = 5;
								break;
		case RSM_CHARLEN6	:	databits = 6;
								break;
		case RSM_CHARLEN7	:	databits = 7;
								break;
		case RSM_CHARLEN8	:	databits = 8;
								break;
	}

	/* �X�^�[�g/�X�g�b�v�r�b�g�������Z */
	if ((rs_modecmd & RSM_STOPBITM) == RSM_STOPBIT1) {
		databits += (BYTE)2;
	}
	else {
		databits += (BYTE)3;
	}

	/* 1�o�C�g������̑��o���Ԃ���s�P�ʂŋ��߂� */
	sndrcv_timing = (10000000 / (baudrate / databits));
	sndrcv_timing = (sndrcv_timing + 5) / 10;
}


/*
 *	RS-232C
 *	�P�o�C�g�ǂݍ���
 */
BOOL FASTCALL rs232c_readb(WORD addr, BYTE *dat)
{
	switch (addr) {
		case 0xfd06 :	/* USART�f�[�^���W�X�^ */
#if XM7_VER >= 3
			if (rs_use && ((fm7_ver <= 2) || rs_enable) && !rs_mask) {
#else
			if (rs_use && !rs_mask) {
#endif
				if (rs_selectmc) {
					*dat = 0;
				}
				else {
					*dat = rs232c_receivedata();
				}
				return TRUE;
			}
			break;

		case 0xfd07 :	/* USART�X�e�[�^�X���W�X�^ */
#if XM7_VER >= 3
			if (rs_use && ((fm7_ver <= 2) || rs_enable) && !rs_mask) {
#else
			if (rs_use && !rs_mask) {
#endif
				*dat = rs232c_readstatus();

				/* SYNDET�M���𗎂Ƃ� */
				if (rs_status & RSS_SYNDET) {
					rs232c_syndet(FALSE);
				}

				/* �����������瑼�̊��荞�݂������邩�� */
				txrdy_irq_flag = FALSE;
				rxrdy_irq_flag = FALSE;
				maincpu_irq();

				return TRUE;
			}
			break;
	}

	return FALSE;
}

/*
 *	RS-232C
 *	�P�o�C�g��������
 */
BOOL FASTCALL rs232c_writeb(WORD addr, BYTE dat)
{
#if XM7_VER >= 3
	BOOL flag;
#endif

	switch (addr) {
		case 0xfd06 :	/* USART�f�[�^���W�X�^ */
#if XM7_VER >= 3
			if (rs_use && ((fm7_ver <= 2) || rs_enable) && !rs_selectmc &&
				!rs_mask) {
#else
			if (rs_use && !rs_selectmc && !rs_mask) {
#endif
				rs232c_senddata(dat);
				return TRUE;
			}
			break;

		case 0xfd07 :	/* USART�R�}���h���W�X�^ */
#if XM7_VER >= 3
			if (rs_use && ((fm7_ver <= 2) || rs_enable) && !rs_mask) {
#else
			if (rs_use && !rs_mask) {
#endif
				if (rs_selectmc) {
					rs232c_writemodecmd(dat);
					rs_modecmd = dat;
					rs_selectmc = FALSE;

					/* ����M�^�C�~���O�v�Z */
					rs232c_calc_timing();
				}
				else {
					rs232c_writecommand(dat);
					rs_command = dat;

					if (dat & RSC_IR) {
						/* �������Z�b�g */
						rs_selectmc = TRUE;
					}
				}
				return TRUE;
			}
			break;

#if XM7_VER >= 3
		case 0xfd0b :	/* FM77AV40/20 �N���b�N�E�{�[���[�g�ݒ背�W�X�^ */
			if ((fm7_ver >= 3) && !rs_mask) {
				rs232c_setbaudrate(dat);
				rs_baudrate = dat;

				/* ����M�^�C�~���O�v�Z */
				rs232c_calc_timing();
			}

			return TRUE;

		case 0xfd0c :	/* FM77AV40/20 �g��DTR���W�X�^ */
			if (fm7_ver >= 3) {
				/* bit0:RS-232C�L�� */
				if (dat & RSEX_RSENABLE) {
					rs_enable = TRUE;
				}
				else {
					rs_enable = FALSE;
				}

				/* bit2:DTR�o�͋֎~ */
				flag = rs_dtrmask;
				if (dat & RSEX_DTR) {
					rs_dtrmask = TRUE;
					flag = !flag;
				}
				else {
					rs_dtrmask = FALSE;
				}
				if (rs_enable && !rs_mask && flag && (rs_command & RSC_DTR)) {
					rs232c_writecommand(rs_command);
				}
				return TRUE;
			}
#endif

		default:
			return FALSE;
	}

	return FALSE;
}

/*
 *	RS-232C
 *	�Z�[�u
 */
BOOL FASTCALL rs232c_save(int fileh)
{
	if (!file_bool_write(fileh, rs_mask)) {
		return FALSE;
	}
	if (!file_bool_write(fileh, rs_enable)) {
		return FALSE;
	}
	if (!file_bool_write(fileh, rs_selectmc)) {
		return FALSE;
	}
	if (!file_byte_write(fileh, rs_modecmd)) {
		return FALSE;
	}
	if (!file_byte_write(fileh, rs_command)) {
		return FALSE;
	}
	if (!file_byte_write(fileh, rs_status)) {
		return FALSE;
	}
	if (!file_byte_write(fileh, rs_baudrate)) {
		return FALSE;
	}
	if (!file_bool_write(fileh, rs_dtrmask)) {
		return FALSE;
	}
	if (!file_bool_write(fileh, rs_cd)) {
		return FALSE;
	}
	if (!file_bool_write(fileh, rs_cts)) {
		return FALSE;
	}

	return TRUE;
}

/*
 *	RS-232C
 *	���[�h
 */
BOOL FASTCALL rs232c_load(int fileh, int ver)
{
	/* �o�[�W�����`�F�b�N */
	if (ver < 200) {
		return FALSE;
	}
#if XM7_VER >= 3
	if (((ver >= 500) && (ver < 715)) || ((ver >= 800) && (ver < 915))) {
#elif XM7_VER >= 2
	if (ver < 715) {
#else
	if (ver < 305) {
#endif
		return TRUE;
	}

	if (!file_bool_read(fileh, &rs_mask)) {
		return FALSE;
	}
	if (!file_bool_read(fileh, &rs_enable)) {
		return FALSE;
	}
	if (!file_bool_read(fileh, &rs_selectmc)) {
		return FALSE;
	}
	if (!file_byte_read(fileh, &rs_modecmd)) {
		return FALSE;
	}
	if (!file_byte_read(fileh, &rs_command)) {
		return FALSE;
	}
	if (!file_byte_read(fileh, &rs_status)) {
		return FALSE;
	}
	if (!file_byte_read(fileh, &rs_baudrate)) {
		return FALSE;
	}
	if (!file_bool_read(fileh, &rs_dtrmask)) {
		return FALSE;
	}
	if (!file_bool_read(fileh, &rs_cd)) {
		return FALSE;
	}
	if (!file_bool_read(fileh, &rs_cts)) {
		return FALSE;
	}

	/* �C�x���g */
	schedule_handle(EVENT_RS_TXTIMING, rs232c_txempty_event);
	schedule_handle(EVENT_RS_RXTIMING, rs232c_rxrdy_event);

	/* �|�[�g�ݒ� */
	rs232c_setbaudrate(rs_baudrate);
	rs232c_writemodecmd(rs_modecmd);
	rs232c_writecommand(rs_command);
	rs232c_calc_timing();

	return TRUE;
}

#endif		/* RSC */
