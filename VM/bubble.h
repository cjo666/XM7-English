/*
 *	FM-7 EMULATOR "XM7"
 *
 *	Copyright (C) 1999-2017 �o�h�D(Twitter:@xm6_original)
 *	Copyright (C) 2001-2017 Ryu Takegami (Twitter:@RyuTakegami)
 *
 *	[ �o�u�������� �R���g���[�� (32KB��p��) ]
 */

#ifndef _bubble_h_
#define _bubble_h_

#if XM7_VER == 1 
#if defined(BUBBLE) || (!defined(BUBBLE) && defined(XM7PURE))

/*
 *	�萔��`
 */
#define BMC_UNITS_32		2			/* 32KB�T�|�[�g���j�b�g�� */
#define BMC_MEDIAS			16			/* B77�t�@�C���Ɋ܂܂��ő�{�� */

#define BMC_ST_BUSY			0x01		/* BUSY */
#define BMC_ST_ERROR		0x02		/* ERROR ANALYSIS */
#define BMC_ST_WRITEP		0x04		/* �������ݕی� */
#define BMC_ST_READY		0x08		/* ���f�B�A�}�� */
#define BMC_ST_NONE			0x10		/* ���'1' */
#define BMC_ST_RDA			0x20		/* CAN READ */
#define BMC_ST_TDRA			0x40		/* CAN WRITE */
#define BMC_ST_CME			0x80		/* COMMAND END */

#define BMC_ES_UNDEF		0x01		/* UNDEFINED COMMAND ERROR */
#define BMC_ES_NOMAKER		0x02		/* NO MARKER */
#define BMC_ES_MANYBAD		0x04		/* MANY BAD LOOP */
#define BMC_ES_TRANSFER		0x08		/* TRANSFER MISSING */
#define BMC_ES_CRCERR		0x10		/* CRC ERROR */
#define BMC_ES_PAGEOVER		0x20		/* PAGE ADDRESS OVER ERROR */
#define BMC_ES_NONE			0x40		/* ---- */
#define BMC_ES_EJECT		0x80		/* EJECT ERROR */

#define BMC_TYPE_NOTREADY	0			/* �t�@�C���Ȃ� */
#define BMC_TYPE_32			1			/* 32KB�t�@�C�����}�E���g */
#define BMC_TYPE_128		2			/* 128KB�t�@�C�����}�E���g(dash) */
#define BMC_TYPE_B77		3			/* B77�t�@�C�����}�E���g */

#define BMC_ACCESS_READY	0			/* �A�N�Z�X�Ȃ� */
#define BMC_ACCESS_READ		1			/* �ǂݍ��݌n�A�N�Z�X */
#define BMC_ACCESS_WRITE	2			/* �������݌n�A�N�Z�X */
#define BMC_ACCESS_NOTREADY	3			/* ���j�b�g�̏������ł��Ă��Ȃ� */
#define BMC_ACCESS_TEJECT	4			/* ���j�b�g���ꎞ�C�W�F�N�g�� */
#define BMC_LOST_TIME		300			/* wait����LOST DATA�܂ł̎��� */

#define BMC_PSIZE_32		0x20		/* 32KB�y�[�W�T�C�Y */
#define BMC_MAXADDR_32		0x03ff		/* 32KB�ŏI�y�[�W�A�h���X */

#ifdef __cplusplus
extern "C" {
#endif
#if !defined(XM7PURE) || defined(BUBBLE)
/*
 *	��v�G���g��
 */
BOOL FASTCALL bmc_init(void);
										/* ������ */
void FASTCALL bmc_cleanup(void);
										/* �N���[���A�b�v */
void FASTCALL bmc_reset(void);
										/* ���Z�b�g */
BOOL FASTCALL bmc_readb(WORD addr, BYTE *dat);
										/* �������ǂݏo�� */
BOOL FASTCALL bmc_writeb(WORD addr, BYTE dat);
										/* �������������� */
BOOL FASTCALL bmc_save(int fileh);
										/* �Z�[�u */
BOOL FASTCALL bmc_load(int fileh, int ver);
										/* ���[�h */
int FASTCALL bmc_setfile(int unit, char *fname);
										/* �Z�b�g */
BOOL FASTCALL bmc_setmedia(int unit, int index);
										/* ���f�B�A�ԍ���ݒ� */
BOOL FASTCALL bmc_setwritep(int unit, BOOL writep);
										/* ���C�g�v���e�N�g�w�� */

/*
 *	��v���[�N
 */
extern BYTE bmc_datareg;
										/* $FD10   �f�[�^���W�X�^ */
extern BYTE bmc_command;
										/* $FD11   �R�}���h���W�X�^ */
extern BYTE bmc_status;
										/* $FD12   �X�e�[�^�X���W�X�^ */
extern BYTE bmc_errorreg;
										/* $FD13   �G���[�X�e�[�^�X���W�X�^ */
extern WORD bmc_pagereg;
										/* $FD14-5 �y�[�W���W�X�^ */
extern WORD bmc_countreg;
										/* $FD16-7 �y�[�W�J�E���g���W�X�^ */

extern WORD bmc_totalcnt;
										/* �g�[�^���J�E���^ */
extern WORD bmc_nowcnt;
										/* �J�����g�J�E���^ */
extern BYTE bmc_unit;
										/* ���j�b�g */
extern BYTE bmc_ready[BMC_UNITS_32];
										/* ���f�B��� */
extern BOOL bmc_teject[BMC_UNITS_32];
										/* �ꎞ�C�W�F�N�g */
extern BOOL bmc_writep[BMC_UNITS_32];
										/* �������݋֎~��� */

extern char bmc_fname[BMC_UNITS_32][256+1];
										/* �t�@�C���� */
extern char bmc_name[BMC_UNITS_32][BMC_MEDIAS][17];
										/* ���f�B�A���Ƃ̖��O */
extern BOOL bmc_fwritep[BMC_UNITS_32];
										/* �������݋֎~���(�t�@�C���P��) */
extern BYTE bmc_header[BMC_UNITS_32][0x20];
										/* B77�t�@�C���w�b�_ */
extern BYTE bmc_medias[BMC_UNITS_32];
										/* ���f�B�A���� */
extern BYTE bmc_media[BMC_UNITS_32];
										/* ���f�B�A�Z���N�g��� */
extern BYTE bmc_access[BMC_UNITS_32];
										/* �A�N�Z�XLED */
extern BOOL bmc_enable;
										/* �L���E�����t���O */
extern BOOL bmc_use;
										/* �g�p�t���O */
#endif	/* !defined(XM7PURE) */

/*
 *	��v�G���g�� (�X�e�[�g�t�@�C���֌W)
 */
BOOL FASTCALL bmc_save(int fileh);
										/* �Z�[�u */
BOOL FASTCALL bmc_load(int fileh, int ver);
										/* ���[�h */

#ifdef __cplusplus
}
#endif

#endif	/* defined(BUBBLE) */
#endif	/* XM7_VER == 1 */

#endif	/* _bubble_h_ */
