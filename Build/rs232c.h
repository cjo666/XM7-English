/*
 *	FM-7 EMULATOR "XM7"
 *
 *	Copyright (C) 1999-2017 �o�h�D(Twitter:@xm6_original)
 *	Copyright (C) 2001-2017 Ryu Takegami (Twitter:@RyuTakegami)
 *
 *	[ RS-232C�C���^�t�F�[�X ]
 */

#ifndef _rs232c_h_
#define _rs232c_h_

/*
 *	�萔��`
 */

/* �X�e�[�^�X���W�X�^ */
#define	RSS_DSR			0x80
#define	RSS_SYNDET		0x40
#define	RSS_FRAMEERR	0x20
#define	RSS_OVERRUN		0x10
#define	RSS_PARITYERR	0x08
#define	RSS_TXEMPTY		0x04
#define	RSS_RXRDY		0x02
#define	RSS_TXRDY		0x01

/* ���[�h�R�}���h���W�X�^ */
#define	RSM_STOPBITM	0xc0
#define	RSM_STOPBIT1	0x40
#define	RSM_STOPBIT15	0x80
#define	RSM_STOPBIT2	0xc0
#define	RSM_PARITYM		0x30
#define	RSM_PARITYEVEN	0x20
#define	RSM_PARITYEN	0x10
#define	RSM_CHARLENM	0x0c
#define	RSM_CHARLEN5	0x00
#define	RSM_CHARLEN6	0x04
#define	RSM_CHARLEN7	0x08
#define	RSM_CHARLEN8	0x0c
#define	RSM_BAUDDIVM	0x03
#define	RSM_BAUDDIV1	0x01
#define	RSM_BAUDDIV16	0x02
#define	RSM_BAUDDIV64	0x03

/* �R�}���h���W�X�^ */
#define	RSC_EH			0x80
#define	RSC_IR			0x40
#define	RSC_RTS			0x20
#define	RSC_ER			0x10
#define	RSC_SBRK		0x08
#define	RSC_RXE			0x04
#define	RSC_DTR			0x02
#define	RSC_TXEN		0x01

/* FM77AV40/20 �N���b�N�E�{�[���[�g�ݒ背�W�X�^ */
#define	RSCB_CLKM		0xe0
#define	RSCB_BAUDM		0x1c
#define	RSCB_CD			0x04

/* FM77AV40/20 �g��DTR���䃌�W�X�^ */
#define	RSEX_DTR		0x04
#define	RSEX_RSENABLE	0x01


#if defined(RSC)

#ifdef __cplusplus
extern "C" {
#endif
/*
 *	��v�G���g��
 */
BOOL FASTCALL rs232c_init(void);
										/* ������ */
void FASTCALL rs232c_cleanup(void);
										/* �N���[���A�b�v */
void FASTCALL rs232c_reset(void);
										/* ���Z�b�g */
void FASTCALL rs232c_txrdy(BOOL flag);
										/* TxRDY/TxEMPTY�M���ω� */
void FASTCALL rs232c_txempty_request(void);
										/* TxEMPTY�C�x���g�o�^ */
void FASTCALL rs232c_rxrdy(BOOL flag);
										/* RxRDY�M���ω� */
void FASTCALL rs232c_rxrdy_request(void);
										/* RxRDY�C�x���g�o�^ */
void FASTCALL rs232c_syndet(BOOL flag);
										/* SYNDET�M���ω� */
void FASTCALL rs232c_calc_timing(void);
										/* ����M�^�C�~���O�v�Z */
BOOL FASTCALL rs232c_readb(WORD addr, BYTE *dat);
										/* 1�o�C�g�ǂݏo�� */
BOOL FASTCALL rs232c_writeb(WORD addr, BYTE dat);
										/* 1�o�C�g�������� */
BOOL FASTCALL rs232c_save(int fileh);
										/* �Z�[�u */
BOOL FASTCALL rs232c_load(int fileh, int ver);
										/* ���[�h */

/*
 *	��v���[�N
 */
extern BOOL	rs_use;
										/* RS-232C�g�p�t���O */
extern BOOL	rs_mask;
										/* RS-232C�@�\�}�X�N */
extern BOOL	rs_enable;
										/* RS-232C�L�� */
extern BOOL	rs_selectmc;
										/* ���[�h�R�}���h�I����� */
extern BYTE	rs_modecmd;
										/* ���[�h�R�}���h���W�X�^ */
extern BYTE	rs_command;
										/* �R�}���h���W�X�^ */
extern BYTE	rs_baudrate;
										/* �{�[���[�g�ݒ背�W�X�^ */
extern BYTE	rs_baudrate_v2;
										/* �{�[���[�g�ݒ�(V1/V2�p) */
extern BOOL rs_dtrmask;
										/* DTR�M���}�X�N�t���O */
extern BOOL rs_cd;
										/* CD�M�� */
extern BOOL rs_cts;
										/* CTS�M�� */
#ifdef __cplusplus
}
#endif

#endif	/* XM7_VER >= 3 */
#endif	/* _rs232c_h_ */
