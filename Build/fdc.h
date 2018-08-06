/*
 *	FM-7 EMULATOR "XM7"
 *
 *	Copyright (C) 1999-2017 �o�h�D(Twitter:@xm6_original)
 *	Copyright (C) 2001-2017 Ryu Takegami (Twitter:@RyuTakegami)
 *
 *	[ �t���b�s�[�f�B�X�N �R���g���[��(MB8877A) ]
 */

#ifndef _fdc_h_
#define _fdc_h_

/*
 *	�萔��`
 */
#define FDC_DRIVES			4			/* �T�|�[�g�h���C�u�� */
#define FDC_MEDIAS			16			/* D77�t�@�C���Ɋ܂܂��ő喇�� */

#define FDC_ST_BUSY			0x01		/* BUSY */
#define FDC_ST_INDEX		0x02		/* INDEX�z�[�����o */
#define FDC_ST_DRQ			0x02		/* �f�[�^�v�� */
#define FDC_ST_TRACK00		0x04		/* �g���b�N0 */
#define FDC_ST_LOSTDATA		0x04		/* �f�[�^���X�g */
#define FDC_ST_CRCERR		0x08		/* CRC�G���[ */
#define FDC_ST_SEEKERR		0x10		/* �V�[�N�G���[ */
#define FDC_ST_RECNFND		0x10		/* �Z�N�^�������s */
#define FDC_ST_HEADENG		0x20		/* �w�b�h�����t�� */
#define FDC_ST_RECTYPE		0x20		/* ���R�[�h�^�C�v�ُ� */
#define FDC_ST_WRITEFAULT	0x20		/* �������ݎ��s */
#define FDC_ST_WRITEP		0x40		/* �������ݕی� */
#define FDC_ST_NOTREADY		0x80		/* ���f�B�A���}�� */

#define FDC_TYPE_NOTREADY	0			/* �t�@�C���Ȃ� */
#define FDC_TYPE_2D			1			/* 2D�t�@�C�����}�E���g */
#define FDC_TYPE_D77		2			/* D77�t�@�C�����}�E���g */
#define FDC_TYPE_VFD		3			/* VFD�t�@�C�����}�E���g */
#define FDC_TYPE_2DD		4			/* 2DD�t�@�C�����}�E���g */

#define FDC_ACCESS_READY	0			/* �A�N�Z�X�Ȃ� */
#define FDC_ACCESS_SEEK		1			/* �V�[�N�n�A�N�Z�X */
#define FDC_ACCESS_READ		2			/* �ǂݍ��݌n�A�N�Z�X */
#define FDC_ACCESS_WRITE	3			/* �������݌n�A�N�Z�X */
#define FDC_ACCESS_NOTREADY	4			/* �h���C�u�̏������ł��Ă��Ȃ� */

#define FDC_LOST_TIME		48			/* wait����LOST DATA�܂ł̎��� */

#ifdef __cplusplus
extern "C" {
#endif
/*
 *	��v�G���g��
 */
BOOL FASTCALL fdc_init(void);
										/* ������ */
void FASTCALL fdc_cleanup(void);
										/* �N���[���A�b�v */
void FASTCALL fdc_reset(void);
										/* ���Z�b�g */
BOOL FASTCALL fdc_readb(WORD addr, BYTE *dat);
										/* �������ǂݏo�� */
BOOL FASTCALL fdc_writeb(WORD addr, BYTE dat);
										/* �������������� */
BOOL FASTCALL fdc_save(int fileh);
										/* �Z�[�u */
BOOL FASTCALL fdc_load(int fileh, int ver);
										/* ���[�h */
int FASTCALL fdc_setdisk(int drive, char *fname);
										/* �f�B�X�N�Z�b�g */
BOOL FASTCALL fdc_setmedia(int drive, int index);
										/* �f�B�X�N�t�@�C�������f�B�A�w�� */
BOOL FASTCALL fdc_setwritep(int drive, BOOL writep);
										/* ���C�g�v���e�N�g�w�� */

/*
 *	��v���[�N
 */
extern BOOL fdc_enable;
										/* FDC�C�l�[�u�� */
extern BYTE fdc_command;
										/* $FD18 FDC�R�}���h */
extern BYTE fdc_status;
										/* $FD18 FDC�X�e�[�^�X */
extern BYTE fdc_trkreg;
										/* $FD19 �g���b�N���W�X�^ */
extern BYTE fdc_secreg;
										/* $FD1A �Z�N�^���W�X�^ */
extern BYTE fdc_datareg;
										/* $FD1B �f�[�^���W�X�^ */
extern BYTE fdc_sidereg;
										/* $FD1C �T�C�h���W�X�^ */
extern BYTE fdc_motor;
										/* $FD1D ���[�^(on:0x80 off:0x00) */
extern BYTE fdc_drvreg;
										/* $FD1D �h���C�u(0-3) */
extern BYTE fdc_drqirq;
										/* $FD1F DRQ, IRQ, ���̑��t���O */


#if XM7_VER >= 3
extern BYTE fdc_drvregP;
										/*($FD1D)�����h���C�u�ԍ� */
extern BYTE fdc_logidrv;
										/* $FD1E �_���h���C�u�ԍ� */
extern BYTE fdc_physdrv[FDC_DRIVES];
										/* $FD1E �_��/�����h���C�u�̑Ή� */
extern BYTE fdc_2ddmode;
										/* $FD1E 2DD���[�h�Z���N�g */
#endif

extern BYTE fdc_cmdtype;
										/* �R�}���h�^�C�v */
extern WORD fdc_totalcnt;
										/* �g�[�^���J�E���^ */
extern WORD fdc_nowcnt;
										/* �J�����g�J�E���^ */
extern BYTE fdc_ready[FDC_DRIVES];
										/* ���f�B��� */
extern BOOL fdc_teject[FDC_DRIVES];
										/* �ꎞ�C�W�F�N�g */
extern BOOL fdc_writep[FDC_DRIVES];
										/* �������݋֎~��� */
extern BYTE fdc_track[FDC_DRIVES];
										/* ���g���b�N */

extern char fdc_fname[FDC_DRIVES][256+1];
										/* �t�@�C���l�[�� */
extern char fdc_name[FDC_DRIVES][FDC_MEDIAS][17];
										/* ���f�B�A���Ƃ̖��O */
extern BOOL fdc_fwritep[FDC_DRIVES];
										/* �������݋֎~���(�t�@�C���P��) */
extern BYTE fdc_header[FDC_DRIVES][0x2b0];
										/* D77�t�@�C���w�b�_ */
extern BYTE fdc_medias[FDC_DRIVES];
										/* ���f�B�A���� */
extern BYTE fdc_media[FDC_DRIVES];
										/* ���f�B�A�Z���N�g��� */
extern BYTE fdc_access[FDC_DRIVES];
										/* �A�N�Z�XLED */

#if defined(FDDSND)
extern BOOL fdc_waitmode;
										/* FDC�A�N�Z�X�E�F�C�g */
extern BOOL fdc_sound;
										/* FDD�V�[�N�������t���O */
#endif

#ifdef __cplusplus
}
#endif

#endif	/* _fdc_h_ */
