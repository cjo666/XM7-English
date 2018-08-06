/*
 *	FM-7 EMULATOR "XM7"
 *
 *	Copyright (C) 1999-2017 �o�h�D(Twitter:@xm6_original)
 *	Copyright (C) 2001-2017 Ryu Takegami (Twitter:@RyuTakegami)
 *
 *	[ �t���b�s�[�f�B�X�N �R���g���[��(MB8877A) ]
 *
 *	RHG����
 *	  2001.12.03		�f�B�X�N�C���[�W�̃}�E���g���̓t�@�C�����I�[�v��������
 *						�܂̏�ԂɂȂ�悤�ɕύX (towns�ł̂�)
 *						V2��2DD�̃f�B�X�N�C���[�W���}�E���g�ł���̂��C��
 *	  2001.12.09		2DD�C���[�W��2D���[�h�œǂނƃg���b�N�␳���s���Ȃ�
 *						�����C��
 *	  2002.02.09		�Z�N�^���̃w�b�_���̂����擪�̂P�o�C�g(�g���b�N�ԍ�)
 *						���g���b�N�f�[�^�ɔ��f����Ȃ������C�� (Y.Sato)
 *	  2002.02.26		D77�t�@�C���̃Z�N�^����������RECORD TYPE�G���[��CRC�G
 *						���[�������ɔ������Ȃ������C�� (Apollo)
 *	  2002.04.25		�T�C�h���̔�r����bit0�̂ݔ�r����悤�ɕύX
 *	  2002.05.03		2DD�؂芷�����s�������Ƀg���b�N�̍ēǂݍ��݂��s���悤
 *						�ɕύX
 *	  2002.05.06		�g���b�N���W�X�^�E�Z�N�^���W�X�^���R�}���h���s��ɏ���
 *						���������̋������C��
 *						READ ADDRESS�R�}���h���s��ɓǂݏo����ID�t�B�[���h�̃g
 *						���b�N�ԍ����Z�N�^���W�X�^�ɓ���悤�ɕύX
 *	  2002.05.24		fdc_readsec��ID�t�B�[���h�̃T�C�h���ƃT�C�h���W�X�^��
 *						��r�����Ȃ��悤�ɕύX
 *	  2002.11.10		�E�F�C�g�}���E�V�[�N�������ɑΉ�
 *	  2002.12.16		WRITE TRACK�R�}���h���s����FORCE INTERRUPT�R�}���h��
 *						�s���ď�����ł��؂����ꍇ�ɂ��f�[�^�̏������݂��s����
 *						���ɕύX
 *	  2003.02.08		�E�F�C�g���쎞�̃V�[�N�R�}���h�ł�FDC���荞�݃^�C�~��
 *						�O���C��
 *	  2003.02.25		�E�F�C�g�Ȃ����쎞�̃V�[�N�n�R�}���h�ł�IRQ On�EFDC��
 *						�荞�݃^�C�~���O���C��
 *	  2003.03.15		Write Track����DELETED DATA MARK�����ɑΉ�
 *						Read Track�����Ńf�[�^���O��SYNC/DATA ADDRESS MARK��
 *						����Y��Ă����̂��C��
 *	  2003.04.02		Read Track���̏�Ԃ�fdc_ready�ɔ��f����Ȃ������C��
 *	  2004.03.17		2DD�x�^�C���[�W�Ή�
 *						2D/2DD/VFD�t�@�C���̃g���b�N�␳�������C��
 *	  2004.03.23		D77�t�@�C���̊�g���b�N����Ń~�X���Ă����̂��C��
 *	  2010.07.08		SEEK�R�}���h���s��̃g���b�N���W�X�^�ɃZ�b�g����l��
 *						�f�[�^���W�X�^�ɃZ�b�g�����l�ɕύX (OS-9 L1 V1.2 L1.2)
 *	  2010.07.26		�u�[�g�t���O��p�~
 *	  2010.08.07		�ꎞ�C�W�F�N�g���ꂽ���f�B�A�����݂����ԂŃJ�����g
 *						�h���C�u�ȊO�Ƀ��f�B�A���Z�b�g���ꂽ���Ɉꎞ�C�W�F�N
 *						�g�������I�ɉ������������C��
 *	  2010.11.21		�t�@�C�����̐�����256�o�C�g�܂łɊɘa
 *						���f�B�A�}�E���g���̃t�@�C�����T�C�Y�`�F�b�N������
 *	  2012.10.11		��J�����g�h���C�u�Ƀ}�E���g����D77�t�@�C�����̃��f�B
 *						�A�ύX�������ۂɓ����g���b�N�o�b�t�@�������������Ă�
 *						�܂������C��
 *	  2017.11.28		V1/V2�ł�2DD�C���[�W�A�N�Z�X(�����g���b�N�̂�)�ɑΉ�
 */

#include <string.h>
#include <stdlib.h>
#include "xm7.h"
#include "device.h"
#include "fdc.h"
#include "mainetc.h"
#if XM7_VER >= 3
#include "dmac.h"
#endif
#include "event.h"

/*
 *	�O���[�o�� ���[�N
 */
BOOL fdc_enable;					/* FDC�C�l�[�u�� */
BYTE fdc_command;					/* FDC�R�}���h */
BYTE fdc_status;					/* FDC�X�e�[�^�X */
BYTE fdc_trkreg;					/* �g���b�N���W�X�^ */
BYTE fdc_secreg;					/* �Z�N�^���W�X�^ */
BYTE fdc_datareg;					/* �f�[�^���W�X�^ */
BYTE fdc_sidereg;					/* �T�C�h���W�X�^ */
BYTE fdc_drvreg;					/* �_���h���C�u */
#if XM7_VER >= 3
BYTE fdc_drvregP;					/* �����h���C�u */
#endif
BYTE fdc_motor;						/* ���[�^ */
BYTE fdc_drqirq;					/* DRQ�����IRQ */

BYTE fdc_cmdtype;					/* �R�}���h�^�C�v */
WORD fdc_totalcnt;					/* �g�[�^���J�E���^ */
WORD fdc_nowcnt;					/* �J�����g�J�E���^ */
BYTE fdc_ready[FDC_DRIVES];			/* ���f�B��� */
BOOL fdc_teject[FDC_DRIVES];		/* �ꎞ�C�W�F�N�g */
BOOL fdc_writep[FDC_DRIVES];		/* ���C�g�v���e�N�g��� */
BYTE fdc_track[FDC_DRIVES];			/* ���g���b�N */

#if XM7_VER >= 3
BYTE fdc_logidrv;					/* �_���h���C�u�ԍ� */
BYTE fdc_physdrv[FDC_DRIVES];		/* �_��/�����h���C�u�̑Ή� */
BYTE fdc_2ddmode;					/* 2D���[�h�I����� */
#endif

char fdc_fname[FDC_DRIVES][256+1];	/* �t�@�C���� */
char fdc_name[FDC_DRIVES][FDC_MEDIAS][17];
BOOL fdc_fwritep[FDC_DRIVES];		/* ���C�g�v���e�N�g���(�t�@�C�����x��) */
BYTE fdc_header[FDC_DRIVES][0x2b0];	/* D77�t�@�C���w�b�_ */
BYTE fdc_medias[FDC_DRIVES];		/* ���f�B�A���� */
BYTE fdc_media[FDC_DRIVES];			/* ���f�B�A�Z���N�g��� */
BYTE fdc_access[FDC_DRIVES];		/* �A�N�Z�XLED */

#if defined(FDDSND)
BOOL fdc_waitmode;					/* FDC�A�N�Z�X�E�F�C�g */
BOOL fdc_sound;						/* FDD�V�[�N�������t���O */
#endif


/*
 *	�X�^�e�B�b�N ���[�N
 */
static BYTE fdc_buffer[0x2000];					/* �f�[�^�o�b�t�@ */
static BYTE *fdc_dataptr;						/* �f�[�^�|�C���^ */
static DWORD fdc_seekofs[FDC_DRIVES];			/* �V�[�N�I�t�Z�b�g */
static DWORD fdc_secofs[FDC_DRIVES];			/* �Z�N�^�I�t�Z�b�g */
static DWORD fdc_foffset[FDC_DRIVES][FDC_MEDIAS];
static WORD fdc_trklen[FDC_DRIVES];				/* �g���b�N�f�[�^���� */
static BOOL fdc_seekvct;						/* �V�[�N����(Trk0:TRUE) */
static BYTE fdc_indexcnt;						/* INDEX�z�[�� �J�E���^ */
#if defined(FDDSND)
static BOOL fdc_wait;							/* �E�F�C�g���[�h���s�t���O */
static int fdc_seek_track;						/* waitmode�p�V�[�N�J�E���^ */
#endif


/*
 *	�X�e�b�v���[�g�e�[�u��
 */
#if defined(FDDSND)
static const int fdc_steprate[4] = { 6000, 12000, 20000, 30000 };
#endif


/*
 *	�v���g�^�C�v�錾
 */
static void FASTCALL fdc_readbuf(int drive);	/* �P�g���b�N���ǂݍ��� */
static BOOL FASTCALL fdc_lost_event(void);		/* LOST DATA�C�x���g */


/*
 *	FDC
 *	������
 */
BOOL FASTCALL fdc_init(void)
{
	int i;

	/* FDC�C�l�[�u�� */
	fdc_enable = TRUE;

	/* �t���b�s�[�t�@�C���֌W�����Z�b�g */
	for (i=0; i<FDC_DRIVES; i++) {
		fdc_ready[i] = FDC_TYPE_NOTREADY;
		fdc_teject[i] = FALSE;
		fdc_fwritep[i] = FALSE;
		fdc_medias[i] = 0;
	}

	/* �t�@�C���I�t�Z�b�g��S�ăN���A */
	memset(fdc_foffset, 0, sizeof(fdc_foffset));

	/* �E�F�C�g�}�����[�h�t���O������ */
#if defined(FDDSND)
	fdc_waitmode = FALSE;
	fdc_sound = FALSE;
	fdc_wait = FALSE;
#endif

	return TRUE;
}

/*
 *	FDC
 *	�N���[���A�b�v
 */
void FASTCALL fdc_cleanup(void)
{
	int i;

	/* �t���b�s�[�t�@�C���֌W�����Z�b�g */
	for (i=0; i<FDC_DRIVES; i++) {
		fdc_ready[i] = FDC_TYPE_NOTREADY;
	}
}

/*
 *	FDC
 *	���Z�b�g
 */
void FASTCALL fdc_reset(void)
{
#if XM7_VER >= 3
	int i;
#endif

	/* FDC�������W�X�^�����Z�b�g */
	fdc_command = 0xff;
	fdc_status = 0;
	fdc_trkreg = 0;
	fdc_secreg = 0;
	fdc_datareg = 0;
	fdc_sidereg = 0;
	fdc_drvreg = 0;
#if XM7_VER >= 3
	fdc_drvregP = 0;
#endif
	fdc_motor = 0;

#if XM7_VER >= 3
	fdc_logidrv = 0;
	fdc_2ddmode = FALSE;

	/* �_���h���C�u�������h���C�u�ɐݒ� */
	for (i=0; i<FDC_DRIVES; i++) {
		fdc_physdrv[i] = (BYTE)i;
	}
#endif

	fdc_cmdtype = 0;
	fdc_seekvct = 0;
	memset(fdc_track, 0, sizeof(fdc_track));
	fdc_dataptr = NULL;
	memset(fdc_access, 0, sizeof(fdc_access));

	/* �f�[�^�o�b�t�@�֓ǂݍ��� */
	fdc_readbuf(fdc_drvreg);
}

/*-[ CRC�v�Z ]--------------------------------------------------------------*/

/*
 *	CRC�v�Z�e�[�u��
 */
static WORD crc_table[256] = {
	0x0000,  0x1021,  0x2042,  0x3063,  0x4084,  0x50a5,  0x60c6,  0x70e7,
	0x8108,  0x9129,  0xa14a,  0xb16b,  0xc18c,  0xd1ad,  0xe1ce,  0xf1ef,
	0x1231,  0x0210,  0x3273,  0x2252,  0x52b5,  0x4294,  0x72f7,  0x62d6,
	0x9339,  0x8318,  0xb37b,  0xa35a,  0xd3bd,  0xc39c,  0xf3ff,  0xe3de,
	0x2462,  0x3443,  0x0420,  0x1401,  0x64e6,  0x74c7,  0x44a4,  0x5485,
	0xa56a,  0xb54b,  0x8528,  0x9509,  0xe5ee,  0xf5cf,  0xc5ac,  0xd58d,
	0x3653,  0x2672,  0x1611,  0x0630,  0x76d7,  0x66f6,  0x5695,  0x46b4,
	0xb75b,  0xa77a,  0x9719,  0x8738,  0xf7df,  0xe7fe,  0xd79d,  0xc7bc,
	0x48c4,  0x58e5,  0x6886,  0x78a7,  0x0840,  0x1861,  0x2802,  0x3823,
	0xc9cc,  0xd9ed,  0xe98e,  0xf9af,  0x8948,  0x9969,  0xa90a,  0xb92b,
	0x5af5,  0x4ad4,  0x7ab7,  0x6a96,  0x1a71,  0x0a50,  0x3a33,  0x2a12,
	0xdbfd,  0xcbdc,  0xfbbf,  0xeb9e,  0x9b79,  0x8b58,  0xbb3b,  0xab1a,
	0x6ca6,  0x7c87,  0x4ce4,  0x5cc5,  0x2c22,  0x3c03,  0x0c60,  0x1c41,
	0xedae,  0xfd8f,  0xcdec,  0xddcd,  0xad2a,  0xbd0b,  0x8d68,  0x9d49,
	0x7e97,  0x6eb6,  0x5ed5,  0x4ef4,  0x3e13,  0x2e32,  0x1e51,  0x0e70,
	0xff9f,  0xefbe,  0xdfdd,  0xcffc,  0xbf1b,  0xaf3a,  0x9f59,  0x8f78,
	0x9188,  0x81a9,  0xb1ca,  0xa1eb,  0xd10c,  0xc12d,  0xf14e,  0xe16f,
	0x1080,  0x00a1,  0x30c2,  0x20e3,  0x5004,  0x4025,  0x7046,  0x6067,
	0x83b9,  0x9398,  0xa3fb,  0xb3da,  0xc33d,  0xd31c,  0xe37f,  0xf35e,
	0x02b1,  0x1290,  0x22f3,  0x32d2,  0x4235,  0x5214,  0x6277,  0x7256,
	0xb5ea,  0xa5cb,  0x95a8,  0x8589,  0xf56e,  0xe54f,  0xd52c,  0xc50d,
	0x34e2,  0x24c3,  0x14a0,  0x0481,  0x7466,  0x6447,  0x5424,  0x4405,
	0xa7db,  0xb7fa,  0x8799,  0x97b8,  0xe75f,  0xf77e,  0xc71d,  0xd73c,
	0x26d3,  0x36f2,  0x0691,  0x16b0,  0x6657,  0x7676,  0x4615,  0x5634,
	0xd94c,  0xc96d,  0xf90e,  0xe92f,  0x99c8,  0x89e9,  0xb98a,  0xa9ab,
	0x5844,  0x4865,  0x7806,  0x6827,  0x18c0,  0x08e1,  0x3882,  0x28a3,
	0xcb7d,  0xdb5c,  0xeb3f,  0xfb1e,  0x8bf9,  0x9bd8,  0xabbb,  0xbb9a,
	0x4a75,  0x5a54,  0x6a37,  0x7a16,  0x0af1,  0x1ad0,  0x2ab3,  0x3a92,
	0xfd2e,  0xed0f,  0xdd6c,  0xcd4d,  0xbdaa,  0xad8b,  0x9de8,  0x8dc9,
	0x7c26,  0x6c07,  0x5c64,  0x4c45,  0x3ca2,  0x2c83,  0x1ce0,  0x0cc1,
	0xef1f,  0xff3e,  0xcf5d,  0xdf7c,  0xaf9b,  0xbfba,  0x8fd9,  0x9ff8,
	0x6e17,  0x7e36,  0x4e55,  0x5e74,  0x2e93,  0x3eb2,  0x0ed1,  0x1ef0
};

/*
 *	16bit CRC���v�Z���A�Z�b�g
 */
static void FASTCALL calc_crc(BYTE *addr, int size)
{
	WORD crc;

	/* ������ */
	crc = 0xffff;

	/* �v�Z */
	while (size > 0) {
		crc = (WORD)((crc << 8) ^ crc_table[(BYTE)(crc >> 8) ^ (BYTE)*addr++]);
		size--;
	}

	/* �����Q�o�C�g�ɃZ�b�g(�r�b�O�G���f�B�A��) */
	*addr++ = (BYTE)(crc >> 8);
	*addr = (BYTE)(crc & 0xff);
}

/*
 *	�������v�Z
 */
static BYTE FASTCALL calc_rand(void)
{
	static WORD rand_s = 0x7f28;
	WORD tmp1, tmp2, tmp3;

	tmp1 = rand_s;
	tmp2 = (WORD)(tmp1 & 255);
	tmp1 = (WORD)((tmp1 << 1) + 1 + rand_s);
	tmp3 = (WORD)(((tmp1 >> 8) + tmp2) & 255);
	tmp1 &= 255;
	rand_s = (WORD)((tmp3 << 8) | tmp1);

	return (BYTE)tmp3;
}

/*-[ �t�@�C���Ǘ� ]---------------------------------------------------------*/

/*
 *	Read Track�f�[�^�쐬
 */
static void FASTCALL fdc_make_track(void)
{
	int i;
	int j;
	int gap3;
	int track;
	WORD count;
	WORD secs;
	WORD size;
	BOOL flag;
	BYTE *p;
	BYTE *q;
	BOOL ddm;
	int max_track;

	/* �A���t�H�[�}�b�g�`�F�b�N */
	flag = FALSE;
	if (fdc_ready[fdc_drvreg] != FDC_TYPE_D77) {
		/* 2D/2DD/VFD�t�@�C���̃A���t�H�[�}�b�g�`�F�b�N�����͋��� */
		if (fdc_ready[fdc_drvreg] == FDC_TYPE_2DD) {
			max_track = 80;
		}
		else {
			max_track = 40;
		}

		if (fdc_track[fdc_drvreg] >= max_track) {
			/* �A���t�H�[�}�b�g */
			flag = TRUE;
		}
#if XM7_VER >= 3
		if (((fdc_ready[fdc_drvreg] != FDC_TYPE_2DD) && fdc_2ddmode) &&
			((fdc_track[fdc_drvreg] % 2) == 1)) {
			/* 2D�C���[�W��2DD�A�N�Z�X���A��g���b�N�̓A���t�H�[�}�b�g */
			flag = TRUE;
		}
#endif
	}
	else {
		if ((fdc_buffer[4] == 0) && (fdc_buffer[5] == 0)) {
			/* �A���t�H�[�}�b�g */
			flag = TRUE;
		}
#if XM7_VER >= 3
		if (((fdc_header[fdc_drvreg][0x001b] == 0x00) && fdc_2ddmode) &&
			((fdc_track[fdc_drvreg] % 2) == 1)) {
			/* 2D�C���[�W��2DD�A�N�Z�X���A��g���b�N�̓A���t�H�[�}�b�g */
			flag = TRUE;
		}
#endif
	}

	/* �A���t�H�[�}�b�g�Ȃ�A�����_���f�[�^�쐬 */
	if (flag) {
		p = fdc_buffer;
		for (i=0; i<0x1800; i++) {
			*p++ = calc_rand();
		}

		/* �f�[�^�|�C���^�A�J�E���^�ݒ� */
		fdc_dataptr = fdc_buffer;
		fdc_totalcnt = 0x1800;
		fdc_nowcnt = 0;
		return;
	}

	/* �Z�N�^���Z�o�A�f�[�^�ړ� */
	if ((fdc_ready[fdc_drvreg] == FDC_TYPE_2D) ||
		(fdc_ready[fdc_drvreg] == FDC_TYPE_2DD)) {
		secs = 16;
		q = &fdc_buffer[0x1000];
		memcpy(q, fdc_buffer, 0x1000);
	}
	else if (fdc_ready[fdc_drvreg] == FDC_TYPE_VFD) {
#if XM7_VER >= 3
		if (fdc_2ddmode) {
			/* ���f�B�A�^�C�v(2D) != �h���C�u���[�h(2DD) */
			track = (fdc_track[fdc_drvreg] >> 1);
		}
		else {
			/* ���f�B�A�^�C�v == �h���C�u���[�h �␳�Ȃ� */
			track = fdc_track[fdc_drvreg];
		}
#else
		track = fdc_track[fdc_drvreg];
#endif
		count = fdc_header[fdc_drvreg][track * 6 + 4];
		if (count == 0) {
			count = 128;
		}
		else {
			count *= (WORD)256;
		}
		count *= (WORD)fdc_header[fdc_drvreg][track * 6 + 5];

		/* �f�[�^�R�s�[ */
		q = &fdc_buffer[0x2000 - count];
		for (j = (count - 1); j >= 0; j--) {
			q[j] = fdc_buffer[j];
		}
	}
	else {
		secs = (WORD)(fdc_buffer[0x0005] * 256);
		secs += (WORD)(fdc_buffer[0x0004]);
		count = 0;
		/* �S�Z�N�^�܂���āA�T�C�Y���v�� */
		for (j=0; j<secs; j++) {
			p = &fdc_buffer[count];
			count += (WORD)((p[0x000f] * 256 + p[0x000e]));
			count += (WORD)0x10;
		}
		/* �f�[�^�R�s�[ */
		q = &fdc_buffer[0x2000 - count];
		for (j = (count - 1); j >= 0; j--) {
			q[j] = fdc_buffer[j];
		}
	}

	/* GAP3���� */
	if (secs <= 5) {
		gap3 = 0x74;
	}
	else {
		if (secs <= 10) {
			gap3 = 0x54;
		}
		else {
			if (secs <= 16) {
				gap3 = 0x33;
			}
			else {
				gap3 = 0x10;
			}
		}
	}

	/* �o�b�t�@������ */
	p = fdc_buffer;
	count = 0;

	/* GAP0 */
	for (i=0; i<80; i++) {
		*p++ = 0x4e;
	}
	count += (WORD)80;

	/* SYNC */
	for (i=0; i<12; i++) {
		*p++ = 0;
	}
	count += (WORD)12;

	/* INDEX MARK */
	*p++ = 0xc2;
	*p++ = 0xc2;
	*p++ = 0xc2;
	*p++ = 0xfc;
	count += (WORD)4;

	/* GAP1 */
	for (i=0; i<50; i++) {
		*p++ = 0x4e;
	}
	count += (WORD)50;

	/* �Z�N�^���[�v */
	for (j=0; j<secs; j++) {
		/* SYNC */
		for (i=0; i<12; i++) {
			*p++ = 0;
		}
		count += (WORD)12;

		/* ID ADDRESS MARK */
		p[0] = 0xa1;
		p[1] = 0xa1;
		p[2] = 0xa1;
		p[3] = 0xfe;

		/* ID */
		if ((fdc_ready[fdc_drvreg] == FDC_TYPE_2D) ||
			(fdc_ready[fdc_drvreg] == FDC_TYPE_2DD)) {
#if XM7_VER >= 3
			if ((fdc_ready[fdc_drvreg] == FDC_TYPE_2D) && fdc_2ddmode) {
				/* ���f�B�A�^�C�v(2D) != �h���C�u���[�h(2DD) */
				p[4] = (BYTE)(fdc_track[fdc_drvreg] >> 1);
			}
			else if ((fdc_ready[fdc_drvreg] == FDC_TYPE_2DD) && !fdc_2ddmode) {
				/* ���f�B�A�^�C�v(2DD) != �h���C�u���[�h(2D) */
				p[4] = (BYTE)(fdc_track[fdc_drvreg] << 1);
			}
			else {
				/* ���f�B�A�^�C�v == �h���C�u���[�h �␳�Ȃ� */
				p[4] = fdc_track[fdc_drvreg];
			}
#else
			if (fdc_ready[fdc_drvreg] == FDC_TYPE_2DD) {
				/* ���f�B�A�^�C�v(2DD) != �h���C�u���[�h(2D) */
				p[4] = (BYTE)(fdc_track[fdc_drvreg] << 1);
			}
			else {
				/* ���f�B�A�^�C�v == �h���C�u���[�h �␳�Ȃ� */
				p[4] = fdc_track[fdc_drvreg];
			}
#endif
			p[5] = fdc_sidereg;
			p[6] = (BYTE)(j + 1);
			p[7] = 1;
			size = 0x100;
			ddm = FALSE;
		}
		else if (fdc_ready[fdc_drvreg] == FDC_TYPE_VFD) {
#if XM7_VER >= 3
			if (fdc_2ddmode) {
				/* ���f�B�A�^�C�v(2D) != �h���C�u���[�h(2DD) */
				p[4] = (BYTE)(fdc_track[fdc_drvreg] >> 1);
			}
			else {
				/* ���f�B�A�^�C�v == �h���C�u���[�h �␳�Ȃ� */
				p[4] = fdc_track[fdc_drvreg];
			}
#else
			p[4] = fdc_track[fdc_drvreg];
#endif
			p[5] = fdc_sidereg;
			p[6] = (BYTE)(j + 1);
			p[7] = fdc_header[fdc_drvreg][p[4] * 6 + 4];
			if (p[7] == 0) {
				size = 128;
			}
			else {
				size = (WORD)(p[7] * 256);
			}
			ddm = FALSE;
		}
		else {
			memcpy(&p[4], q, 4);
			size = (WORD)(q[0x000f] * 256 + q[0x000e]);
			if (q[0x0007] != 0x00) {
				ddm = TRUE;
			}
			else {
				ddm = FALSE;
			}
			q += 0x0010;
		}
		calc_crc(p, 8);
		p += (8 + 2);
		count += (WORD)(8 + 2);

		/* GAP2 */
		for (i=0; i<22; i++) {
			*p++ = 0x4e;
		}
		count += (WORD)22;

		/* SYNC */
		for (i=0; i<12; i++) {
			*p++ = 0;
		}
		count += (WORD)12;

		/* DATA ADDRESS MARK */
		p[0] = 0xa1;
		p[1] = 0xa1;
		p[2] = 0xa1;
		if (ddm) {
			/* DELETED DATA MARK */
			p[3] = 0xf8;
		}
		else {
			/* DATA MARK */
			p[3] = 0xfb;
		}

		/* �f�[�^ */
		memcpy(&p[4], q, size);
		q += size;
		calc_crc(p, size + 4);
		p += (size + 4 + 2);
		count += (WORD)(size + 4 + 2);

		/* GAP3 */
		for (i=0; i<gap3; i++) {
			*p++ = 0x4e;
		}
		count += (WORD)gap3;
	}

	/* GAP4 */
	j = (0x1800 - count);
	if (j < 0x1800) {
		for (i=0; i<j; i++) {
			*p++ = 0x4e;
		}
		count += (WORD)j;
	}

	/* �f�[�^�|�C���^�A�J�E���^�ݒ� */
	fdc_dataptr = fdc_buffer;
	fdc_totalcnt = count;
	fdc_nowcnt = 0;
}

/*
 *	ID�t�B�[���h���o�b�t�@�ɍ��
 *	�J�E���^�A�f�[�^�|�C���^��ݒ�
 */
static void FASTCALL fdc_makeaddr(int index)
{
	int i;
	BYTE *p;
	WORD offset;
	WORD size;

	/* �]���̂��߂̃e���|�����o�b�t�@�́A�ʏ�o�b�t�@�� */
	/* �Ō����ׂ��Đ݂��� */
	p = &fdc_buffer[0x1ff0];

	/* ID ADDRESS MARK */
	p[0] = 0xa1;
	p[1] = 0xa1;
	p[2] = 0xa1;
	p[3] = 0xfe;

	if ((fdc_ready[fdc_drvreg] == FDC_TYPE_2D) ||
		(fdc_ready[fdc_drvreg] == FDC_TYPE_2DD)) {
#if XM7_VER >= 3
		/* 2D/2DD�̏ꍇ�AC,H,R,N�͊m�肷�� */
		if ((fdc_ready[fdc_drvreg] == FDC_TYPE_2D) && fdc_2ddmode) {
			/* ���f�B�A�^�C�v(2D) != �h���C�u���[�h(2DD) */
			p[4] = (BYTE)(fdc_track[fdc_drvreg] >> 1);
		}
		else if ((fdc_ready[fdc_drvreg] == FDC_TYPE_2DD) && !fdc_2ddmode) {
			/* ���f�B�A�^�C�v(2DD) != �h���C�u���[�h(2D) */
			p[4] = (BYTE)(fdc_track[fdc_drvreg] << 1);
		}
		else {
			/* ���f�B�A�^�C�v == �h���C�u���[�h �␳�Ȃ� */
			p[4] = fdc_track[fdc_drvreg];
		}
#else
		if (fdc_ready[fdc_drvreg] == FDC_TYPE_2DD) {
			/* ���f�B�A�^�C�v(2DD) != �h���C�u���[�h(2D) */
			p[4] = (BYTE)(fdc_track[fdc_drvreg] << 1);
		}
		else {
			/* ���f�B�A�^�C�v == �h���C�u���[�h �␳�Ȃ� */
			p[4] = fdc_track[fdc_drvreg];
		}
#endif
		p[5] = fdc_sidereg;
		p[6] = (BYTE)(index + 1);
		p[7] = 1;
	}
	else if (fdc_ready[fdc_drvreg] == FDC_TYPE_VFD) {
		/* VFD�̏ꍇ�AC,H,R�͊m��AN�̓w�b�_����擾 */
#if XM7_VER >= 3
		if (fdc_2ddmode) {
			/* ���f�B�A�^�C�v(2D) != �h���C�u���[�h(2DD) */
			p[4] = (BYTE)(fdc_track[fdc_drvreg] >> 1);
		}
		else {
			/* ���f�B�A�^�C�v == �h���C�u���[�h �␳�Ȃ� */
			p[4] = fdc_track[fdc_drvreg];
		}
#else
		p[4] = fdc_track[fdc_drvreg];
#endif
		p[5] = fdc_sidereg;
		p[6] = (BYTE)(index + 1);
		p[7] = fdc_header[fdc_drvreg][p[0] * 6 + 4];
	}
	else {
		/* �o�b�t�@��ړI�̃Z�N�^�܂Ői�߂� */
		offset = 0;
		i = 0;
		while (i < index) {
			/* �Z�N�^�T�C�Y���擾 */
			size = fdc_buffer[offset + 0x000f];
			size *= (WORD)256;
			size |= fdc_buffer[offset + 0x000e];

			/* ���̃Z�N�^�֐i�߂� */
			offset += size;
			offset += (WORD)0x10;

			i++;
		}

		/* C,H,R,N�R�s�[ */
		memcpy(&p[4], &fdc_buffer[offset], 4);
	}

	/* CRC */
	calc_crc(p, 8);

	/* �f�[�^�|�C���^�A�J�E���^�ݒ� */
	fdc_dataptr = &fdc_buffer[0x1ff4];
	fdc_totalcnt = 6;
	fdc_nowcnt = 0;

	/* �����ŁA�Z�N�^���W�X�^�Ƀg���b�N�ԍ���ݒ� */
	fdc_secreg = p[4];
}

/*
 *	�C���f�b�N�X�J�E���^�����̃Z�N�^�ֈڂ�
 */
static int FASTCALL fdc_next_index(void)
{
	int max_track;
	int track;
	int secs;

	ASSERT(fdc_ready[fdc_drvreg] != FDC_TYPE_NOTREADY);

	/* �f�B�X�N�^�C�v���`�F�b�N */
	if ((fdc_ready[fdc_drvreg] == FDC_TYPE_2D) ||
		(fdc_ready[fdc_drvreg] == FDC_TYPE_2DD)) {
		/* 2D/2DD�Ȃ�16�Z�N�^�Œ� */
		fdc_indexcnt = (BYTE)((fdc_indexcnt + 1) & 0x0f);
		track = fdc_track[fdc_drvreg];
#if XM7_VER >= 3
		if ((fdc_ready[fdc_drvreg] == FDC_TYPE_2D) && fdc_2ddmode) {
			/* ���f�B�A�^�C�v(2D) && �h���C�u���[�h(2DD) */
			track >>= 1;
		}
		else if ((fdc_ready[fdc_drvreg] == FDC_TYPE_2DD) && !fdc_2ddmode) {
			/* ���f�B�A�^�C�v(2DD) && �h���C�u���[�h(2D) */
			track <<= 1;
		}
#else
		if (fdc_ready[fdc_drvreg] == FDC_TYPE_2DD) {
			/* ���f�B�A�^�C�v(2DD) && �h���C�u���[�h(2D) */
			track <<= 1;
		}
#endif

#if XM7_VER >= 3
		if (fdc_2ddmode) {
			max_track = 80;
		}
		else {
			max_track = 40;
		}
#else
		max_track = 40;
#endif
		if (track >= max_track) {
			return -1;
		}
		return fdc_indexcnt;
	}

	/* D77/VFD */
	ASSERT( (fdc_ready[fdc_drvreg] == FDC_TYPE_D77) ||
			(fdc_ready[fdc_drvreg] == FDC_TYPE_VFD));

	if (fdc_ready[fdc_drvreg] == FDC_TYPE_VFD) {
		track = fdc_track[fdc_drvreg];
#if XM7_VER >= 3
		if (fdc_2ddmode) {
			track >>= 1;
		}
#endif
		secs = fdc_header[fdc_drvreg][track * 6 + 5];
	}
	else if (fdc_ready[fdc_drvreg] == FDC_TYPE_D77) {
		secs = fdc_buffer[0x0005];
		secs *= 256;
		secs |= fdc_buffer[0x0004];
	}
	if (secs == 0) {
		/* �A���t�H�[�}�b�g */
		fdc_indexcnt = (BYTE)((fdc_indexcnt + 1) & 0x0f);
		return -1;
	}
	else {
		fdc_indexcnt++;
		if (fdc_indexcnt >= secs) {
			fdc_indexcnt = 0;
		}
		return fdc_indexcnt;
	}
}

/*
 *	ID�}�[�N��T��
 */
static BOOL FASTCALL fdc_idmark(WORD *p)
{
	WORD offset;
	BYTE dat;

	/* A1 A1 A1 FE��������F5 F5 F5 FE */
	offset = *p;

	while (offset < fdc_totalcnt) {
		dat = fdc_buffer[offset++];
		if ((dat != 0xa1) && (dat != 0xf5)) {
			continue;
		}
		dat = fdc_buffer[offset++];
		if ((dat != 0xa1) && (dat != 0xf5)) {
			continue;
		}
		dat = fdc_buffer[offset++];
		if ((dat != 0xa1) && (dat != 0xf5)) {
			continue;
		}
		dat = fdc_buffer[offset++];
		if (dat == 0xfe) {
			*p = offset;
			return TRUE;
		}
	}

	*p = offset;
	return FALSE;
}

/*
 *	�f�[�^�}�[�N��T��
 */
static BOOL FASTCALL fdc_datamark(WORD *p, BOOL *deleted_mark)
{
	WORD offset;
	BYTE dat;

	/* data mark : A1 A1 A1 FB��������F5 F5 F5 FB */
	/* deleted data mark : A1 A1 A1 F8��������F5 F5 F5 F8 */
	offset = *p;

	while (offset < fdc_totalcnt) {
		dat = fdc_buffer[offset++];
		if ((dat != 0xa1) && (dat != 0xf5)) {
			continue;
		}
		dat = fdc_buffer[offset++];
		if ((dat != 0xa1) && (dat != 0xf5)) {
			continue;
		}
		dat = fdc_buffer[offset++];
		if ((dat != 0xa1) && (dat != 0xf5)) {
			continue;
		}
		dat = fdc_buffer[offset++];
		if ((dat == 0xfb) || (dat == 0xf8)) {
			*p = offset;
			if (dat == 0xfb) {
				/* 0xFB : DATA MARK */
				*deleted_mark = FALSE;
			}
			else {
				/* 0xF8 : DELETED DATA MARK */
				*deleted_mark = TRUE;
			}
			return TRUE;
		}
	}

	*p = offset;
	return FALSE;
}

/*
 *	�g���b�N�������ݏI��
 */
static BOOL FASTCALL fdc_writetrk(void)
{
	int total;
	int sectors;
	WORD offset;
	WORD seclen;
	WORD writep;
	int i;
	int handle;
	BOOL ddm;

	/* ������ */
	total = 0;
	sectors = 0;

	/* �Z�N�^���ƁA�f�[�^���̃g�[�^���T�C�Y�𐔂��� */
	offset = 0;
	while (offset < fdc_totalcnt) {
		/* ID�}�[�N��T�� */
		if (!fdc_idmark(&offset)) {
			break;
		}
		/* C,H,R,N�̎���$F7�� */
		offset += (WORD)4;
		if (offset >= fdc_totalcnt) {
			return FALSE;
		}
		if (fdc_buffer[offset] != 0xf7) {
			return FALSE;
		}
		offset++;
		/* �f�[�^�}�[�N��T�� */
		if (!fdc_datamark(&offset, &ddm)) {
			return FALSE;
		}
		/* �����O�X���v�Z���A$F7��T�� */
		seclen = 0;
		while(offset < fdc_totalcnt) {
			if (fdc_buffer[offset] == 0xf7) {
				break;
			}
			offset++;
			seclen++;
		}
		if (offset >= fdc_totalcnt) {
			return FALSE;
		}
		/* �Z�N�^ok */
		total += seclen;
		sectors++;
		offset++;
	}

	/* 2D/2DD���f�B�A�̏ꍇ�Atotal=0x1000, sectors=0x10���K�{ */
	if ((fdc_ready[fdc_drvreg] == FDC_TYPE_2D) ||
		(fdc_ready[fdc_drvreg] == FDC_TYPE_2DD)) {
		/* total, sectors�̌��� */
		if (total != 0x1000) {
			return FALSE;
		}
		if (sectors != 16) {
			return FALSE;
		}

		/* �{����C,H,N�Ȃǃ`�F�b�N���ׂ� */
		return TRUE;
	}

	/* VFD���f�B�A�̏ꍇ�A�����܂� */
	if (fdc_ready[fdc_drvreg] == FDC_TYPE_VFD) {
		/* �{����C,H,N�Ȃǃ`�F�b�N���ׂ� */
		return TRUE;
	}

	/* �Z�N�^���Ƃ�0x10�Ԃ�A�]�v�ɂ����� */
	if ((total + (sectors * 0x10)) > fdc_trklen[fdc_drvreg]) {
		return FALSE;
	}

	/* �������ޕK�v���Ȃ��ꍇ�͐���I�� */
	if ((sectors == 0) && (total == 0)) {
		return TRUE;
	}

	/* ���܂邱�Ƃ��킩�����̂ŁA�f�[�^���쐬���� */
	writep = 0;
	offset = 0;
	for (i=0; i<sectors; i++) {
		/* ID�}�[�N������ */
		fdc_idmark(&offset);

		/* 0x10�w�b�_�̍쐬 */
		fdc_buffer[writep++] = fdc_buffer[offset++];
		fdc_buffer[writep++] = fdc_buffer[offset++];
		fdc_buffer[writep++] = fdc_buffer[offset++];
		fdc_buffer[writep++] = fdc_buffer[offset++];
		fdc_buffer[writep++] = (BYTE)(sectors & 0xff);
		fdc_buffer[writep++] = (BYTE)(sectors >> 8);
		fdc_buffer[writep++] = 0x00;
		offset++;

		/* �f�[�^�}�[�N�̏������� */
		fdc_datamark(&offset, &ddm);
		if (ddm) {
			fdc_buffer[writep++] = 0x10;
		}
		else {
			fdc_buffer[writep++] = 0x00;
		}

		/* ���U�[�u�G���A���N���A */
		/* �Z�N�^�����O�X�͌�ŏ������� */
		memset(&fdc_buffer[writep], 0, 8);
		writep += (WORD)8;

		/* �����O�X�𐔂��R�s�[ */
		seclen = 0;
		while (fdc_buffer[offset] != 0xf7) {
			fdc_buffer[writep++] = fdc_buffer[offset++];
			seclen++;
		}
		offset++;

		/* �Z�N�^�����O�X��ݒ� */
		fdc_buffer[writep - seclen - 2] = (BYTE)(seclen & 0xff);
		fdc_buffer[writep - seclen - 1] = (BYTE)(seclen >> 8);
	}

	/* �t�@�C���Ƀf�[�^���������� */
	handle = file_open(fdc_fname[fdc_drvreg], OPEN_RW);
	if (handle == -1) {
		return FALSE;
	}
	if (!file_seek(handle, fdc_seekofs[fdc_drvreg])) {
		file_close(handle);
		return FALSE;
	}
	if (!file_write(handle, fdc_buffer, (sectors * 0x10) + total)) {
		file_close(handle);
		return FALSE;
	}
	file_close(handle);

	/* ����I�� */
	return TRUE;
}

/*
 *	�Z�N�^�������ݏI��
 */
static BOOL FASTCALL fdc_writesec(void)
{
	DWORD offset;
	int handle;

	/* assert */
	ASSERT(fdc_drvreg < FDC_DRIVES);
	ASSERT(fdc_ready[fdc_drvreg] != FDC_TYPE_NOTREADY);
	ASSERT(fdc_dataptr);
	ASSERT(fdc_totalcnt > 0);

	/* �I�t�Z�b�g�Z�o */
	offset = fdc_seekofs[fdc_drvreg];
	offset += fdc_secofs[fdc_drvreg];

	/* �������� */
	handle = file_open(fdc_fname[fdc_drvreg], OPEN_RW);
	if (handle == -1) {
		return FALSE;
	}
	if (!file_seek(handle, offset)) {
		file_close(handle);
		return FALSE;
	}
	if (!file_write(handle, fdc_dataptr, fdc_totalcnt)) {
		file_close(handle);
		return FALSE;
	}
	file_close(handle);

	return TRUE;
}

/*
 *	�g���b�N�A�Z�N�^�A�T�C�h�ƈ�v����Z�N�^������
 *	�J�E���^�A�f�[�^�|�C���^��ݒ�
 */
static BYTE FASTCALL fdc_readsec(BYTE track, BYTE sector, BYTE side, BOOL sidecmp)
{
	int secs;
	int	len;
	int i;
	WORD offset;
	WORD size;
	BYTE stat;
	DWORD vfdoffset;
	int fdctrack;

	/* assert */
	ASSERT(fdc_drvreg < FDC_DRIVES);
	ASSERT(fdc_ready[fdc_drvreg] != FDC_TYPE_NOTREADY);

	/* �w�b�_�̂���g���b�N�ԍ�(��r�p)�������ݒ� */
	fdctrack = fdc_track[fdc_drvreg];

	/* 2D/2DD�t�@�C���̏ꍇ */
	if ((fdc_ready[fdc_drvreg] == FDC_TYPE_2D) ||
		(fdc_ready[fdc_drvreg] == FDC_TYPE_2DD)) {
#if XM7_VER >= 3
		if ((fdc_ready[fdc_drvreg] == FDC_TYPE_2D) && fdc_2ddmode) {
			/* ���f�B�A�^�C�v(2D) != �h���C�u���[�h(2DD) */
			if ((fdctrack % 2) == 1) {
				/* 2D�C���[�W��2DD�A�N�Z�X���A��g���b�N�A�N�Z�X�̓G���[ */
				return FDC_ST_RECNFND;
			}
			fdctrack = (BYTE)(fdctrack >> 1);
		}
		else if ((fdc_ready[fdc_drvreg] == FDC_TYPE_2DD) && !fdc_2ddmode) {
			/* ���f�B�A�^�C�v(2DD) != �h���C�u���[�h(2D) */
			fdctrack = (BYTE)(fdctrack << 1);
		}
#else
		if (fdc_ready[fdc_drvreg] == FDC_TYPE_2DD) {
			/* ���f�B�A�^�C�v(2DD) != �h���C�u���[�h(2D) */
			fdctrack = (BYTE)(fdctrack << 1);
		}
#endif
		if (track != fdctrack) {
			return FDC_ST_RECNFND;
		}
		if (side != fdc_sidereg) {
			return FDC_ST_RECNFND;
		}
		if ((sector < 1) || (sector > 16)) {
			return FDC_ST_RECNFND;
		}

		/* �f�[�^�|�C���^�ݒ� */
		fdc_dataptr = &fdc_buffer[(sector - 1) * 0x0100];
		fdc_secofs[fdc_drvreg] = (sector - 1) * 0x0100;

		/* �J�E���^�ݒ� */
		fdc_totalcnt = 0x0100;
		fdc_nowcnt = 0;

		return 0;
	}

	/* VFD�t�@�C���̏ꍇ */
	if (fdc_ready[fdc_drvreg] == FDC_TYPE_VFD) {
#if XM7_VER >= 3
		if (fdc_2ddmode) {
			/* ���f�B�A�^�C�v(2D) != �h���C�u���[�h(2DD) */
			if ((fdctrack % 2) == 1) {
				/* 2D�C���[�W��2DD�A�N�Z�X���A��g���b�N�A�N�Z�X�̓G���[ */
				return FDC_ST_RECNFND;
			}
			fdctrack = (BYTE)(fdctrack >> 1);
		}
#endif
		if (track != fdctrack) {
			return FDC_ST_RECNFND;
		}
		if (side != fdc_sidereg) {
			return FDC_ST_RECNFND;
		}

		/* �g���b�N�ւ̃I�t�Z�b�g���`�F�b�N */
		vfdoffset  = fdc_header[fdc_drvreg][track * 6 + 3];
		vfdoffset *= (DWORD)256;
		vfdoffset |= fdc_header[fdc_drvreg][track * 6 + 2];
		vfdoffset *= (DWORD)256;
		vfdoffset |= fdc_header[fdc_drvreg][track * 6 + 1];
		vfdoffset *= (DWORD)256;
		vfdoffset |= fdc_header[fdc_drvreg][track * 6 + 0];
		if (vfdoffset == 0) {
			return FDC_ST_RECNFND;
		}

		/* �g���b�N���̃Z�N�^�����`�F�b�N */
		len = fdc_header[fdc_drvreg][track * 6 + 4];
		if (len == 0) {
			len = 128;
		}
		else {
			len *= 256;
		}
		size = fdc_header[fdc_drvreg][track * 6 + 5];
		if ((sector < 1) || (sector > size)) {
			return FDC_ST_RECNFND;
		}

		/* �f�[�^�|�C���^�ݒ� */
		fdc_dataptr = &fdc_buffer[(sector - 1) * len];
		fdc_secofs[fdc_drvreg] = (sector - 1) * len;

		/* �J�E���^�ݒ� */
		fdc_totalcnt = (WORD)len;
		fdc_nowcnt = 0;

		return 0;
	}

	/* D77�t�@�C���̏ꍇ */
	secs = fdc_buffer[0x0005];
	secs *= (WORD)256;
	secs |= fdc_buffer[0x0004];
	if (secs == 0) {
		return FDC_ST_RECNFND;
	}

#if XM7_VER >= 3
	/* 2D�C���[�W��2DD�A�N�Z�X���A��g���b�N�A�N�Z�X�̓G���[ */
	if ((fdc_header[fdc_drvreg][0x001b] == 0x00) && fdc_2ddmode) {
		if ((fdctrack % 2) == 1) {
			return FDC_ST_RECNFND;
		}
	}
#endif

	offset = 0;
	/* �Z�N�^���[�v */
	for (i=0; i<secs; i++) {
		/* ���̃Z�N�^�̃T�C�Y���Ɏ擾 */
		size = fdc_buffer[offset + 0x000f];
		size *= (WORD)256;
		size |= fdc_buffer[offset + 0x000e];

		/* C,H,R����v����Z�N�^�����邩 */
		if (fdc_buffer[offset + 0] != track) {
			offset += size;
			offset += (WORD)0x10;
			continue;
		}
		/* �T�C�h����bit0�ȊO�͍l�����Ȃ��悤�ɕύX */
		if (sidecmp) {
			if ((BYTE)(fdc_buffer[offset + 1] & 1) != side) {
				offset += size;
				offset += (WORD)0x10;
				continue;
			}
		}
		if (fdc_buffer[offset + 2] != sector) {
			offset += size;
			offset += (WORD)0x10;
			continue;
		}

		/* �P���`�F�b�N */
		if (fdc_buffer[offset + 0x0006] != 0) {
			continue;
		}

		/* �f�[�^�|�C���^�A�J�E���^�ݒ� */
		fdc_dataptr = &fdc_buffer[offset + 0x0010];
		fdc_secofs[fdc_drvreg] = offset + 0x0010;
		fdc_totalcnt = size;
		fdc_nowcnt = 0;

		/* �X�e�[�^�X�ݒ� */
		stat = 0;
		if (fdc_buffer[offset + 0x0007] != 0) {
			stat |= FDC_ST_RECTYPE;
		}
		if (fdc_buffer[offset + 0x0008] == 0xb0) {
			/* �O�ɐݒ肵���X�e�[�^�X�������ɂȂ�����C�� (��ݼ޼�) */
			stat |= FDC_ST_CRCERR;
		}

		return stat;
	}

	/* �S�Z�N�^�������������A������Ȃ����� */
	return FDC_ST_RECNFND;
}

/*
 *	�g���b�N�ǂݍ���
 */
static void FASTCALL fdc_readbuf(int drive)
{
	DWORD offset;
	DWORD min_offset;
	DWORD len;
	DWORD secs;
	int trkside;
	int handle;
	int max_track;
	int trk;

	/* �h���C�u�`�F�b�N */
	if ((drive < 0) || (drive >= FDC_DRIVES)) {
		return;
	}

	/* ���f�B�`�F�b�N */
	if (fdc_ready[drive] == FDC_TYPE_NOTREADY) {
		return;
	}

	/* �C���f�b�N�X�z�[���J�E���^���N���A */
	fdc_indexcnt = 0;

	/* �g���b�N�~�Q�{�T�C�h */
	trkside = fdc_track[drive] * 2 + fdc_sidereg;

	/*
	 * 2D/2DD�t�@�C��
	 */
	if ((fdc_ready[fdc_drvreg] == FDC_TYPE_2D) ||
		(fdc_ready[fdc_drvreg] == FDC_TYPE_2DD)) {
		if (fdc_ready[fdc_drvreg] == FDC_TYPE_2D) {
			max_track = 80;

#if XM7_VER >= 3
			/* 2D�t�@�C����2DD�ǂݏo���̏ꍇ�̕␳ */
			if (fdc_2ddmode) {
				trkside = (fdc_track[drive] >> 1) * 2 + fdc_sidereg;
			}
#endif
		}
		else {
			max_track = 160;

#if XM7_VER >= 3
			/* 2DD�t�@�C����2D�ǂݏo���̏ꍇ�̕␳ */
			if (!fdc_2ddmode) {
				trkside = fdc_track[drive] * 4 + fdc_sidereg;
			}
#else
			trkside = fdc_track[drive] * 4 + fdc_sidereg;
#endif
		}

		/* �I�[�o�[�`�F�b�N */
		if (trkside >= max_track) {
			memset(fdc_buffer, 0, 0x1000);
			return;
		}

		/* �I�t�Z�b�g�Z�o */
		offset = (DWORD)trkside;
		offset *= 0x1000;
		fdc_seekofs[drive] = offset;
		fdc_trklen[drive] = 0x1000;

		/* �ǂݍ��� */
		memset(fdc_buffer, 0, 0x1000);
		handle = file_open(fdc_fname[drive], OPEN_R);
		if (handle == -1) {
			return;
		}
		if (!file_seek(handle, offset)) {
			file_close(handle);
			return;
		}
		file_read(handle, fdc_buffer, 0x1000);
		file_close(handle);
		return;
	}

	/*
	 * VFD�t�@�C��
	 */
	if (fdc_ready[drive] == FDC_TYPE_VFD) {
#if XM7_VER >= 3
		/* 2DD���[�h�̏ꍇ�A�␳ */
		if (fdc_2ddmode) {
			trkside = (fdc_track[drive] >> 1) * 2 + fdc_sidereg;
		}
#endif

		/* �͈̓`�F�b�N */
		if (trkside >= 80) {
			memset(fdc_buffer, 0, 0x1000);
			return;
		}

		/* �I�t�Z�b�g�Z�o */
		offset  = fdc_header[drive][trkside * 6 + 3];
		offset *= 256;
		offset |= fdc_header[drive][trkside * 6 + 2];
		offset *= 256;
		offset |= fdc_header[drive][trkside * 6 + 1];
		offset *= 256;
		offset |= fdc_header[drive][trkside * 6 + 0];
		fdc_seekofs[drive] = offset;

		len		= fdc_header[drive][trkside * 6 + 4];
		secs	= fdc_header[drive][trkside * 6 + 5];
		if (len == 0) {
			len = 128;
		}
		else {
			len *= 256;
		}
		fdc_trklen[drive] = (WORD)(len * secs);

		/* �ǂݍ��� */
		memset(fdc_buffer, 0, fdc_trklen[drive]);
		handle = file_open(fdc_fname[drive], OPEN_R);
		if (handle == -1) {
			return;
		}
		if (!file_seek(handle, offset)) {
			file_close(handle);
			return;
		}
		file_read(handle, fdc_buffer, fdc_trklen[drive]);
		file_close(handle);
		return;
	}

	/*
	 * D77�t�@�C��
	 */
	if (fdc_header[drive][0x001b] == 0x00) {
		max_track = 84;
	}
	else {
		max_track = 164;
	}

	/* �A�N�Z�X���Ă������ŏI�g���b�N�̎Z�o */
	min_offset = (0x0020 + max_track * 4);
	for (trk = 0; trk < max_track; trk++) {
		if ((DWORD)(0x0020 + trk * 4) >= min_offset) {
			break;
		}

		offset  = fdc_header[drive][0x0020 + trk * 4 + 3];
		offset *= 256;
		offset |= fdc_header[drive][0x0020 + trk * 4 + 2];
		offset *= 256;
		offset |= fdc_header[drive][0x0020 + trk * 4 + 1];
		offset *= 256;
		offset |= fdc_header[drive][0x0020 + trk * 4 + 0];
		if ((offset < min_offset) && (offset >= 0x20)) {
			min_offset = offset;
		}
	}
	max_track = (min_offset - 0x20) / 4;

#if XM7_VER >= 3
	if (fdc_2ddmode) {
		/* 2D�t�@�C����2DD�ǂ݂����ꍇ�A�����ŕ␳ */
		if (fdc_header[drive][0x001b] == 0x00) {
			if ((fdc_track[drive] % 2) == 1) {
				/* ��g���b�N�A�N�Z�X�A�Z�N�^��0 */
				fdc_buffer[4] = 0;
				fdc_buffer[5] = 0;
				return;
			}
			trkside = (fdc_track[drive] >> 1) * 2 + fdc_sidereg;
		}
	}
	else {
		/* 2DD�t�@�C����2D�ǂ݂����ꍇ�A�����ŕ␳ (AV40��pOS-9) */
		if (fdc_header[drive][0x001b] == 0x10) {
			trkside = fdc_track[drive] * 4 + fdc_sidereg;
		}
	}
#else
	/* 2DD�t�@�C����2D�ǂ݂����ꍇ�A�����ŕ␳ (AV40��pOS-9) */
	if (fdc_header[drive][0x001b] == 0x10) {
		trkside = fdc_track[drive] * 4 + fdc_sidereg;
	}
#endif

	/* �I�[�o�[�`�F�b�N */
	if (trkside >= max_track) {
		/* �g���b�N�I�[�o�[�A�Z�N�^��0 */
		fdc_buffer[4] = 0;
		fdc_buffer[5] = 0;
		return;
	}

	/* �w�b�_�ɏ]���A�I�t�Z�b�g�E�����O�X���Z�o */
	offset = *(DWORD *)(&fdc_header[drive][0x0020 + trkside * 4]);
	if (offset == 0) {
		/* ���݂��Ȃ��g���b�N */
		fdc_buffer[4] = 0;
		fdc_buffer[5] = 0;
		return;
	}

	len = *(DWORD *)(&fdc_header[drive][0x0020 + (trkside + 1) * 4]);
	if ((len == 0) || ((trkside + 1) >= max_track)) {
		/* �ŏI�g���b�N */
		len = *(DWORD *)(&fdc_header[drive][0x0014]);
	}
	len -= offset;
	if (len > 0x2000) {
		len = 0x2000;
	}
	fdc_seekofs[drive] = offset;
	fdc_trklen[drive] = (WORD)len;

	/* �V�[�N�A�ǂݍ��� */
	memset(fdc_buffer, 0, 0x2000);
	handle = file_open(fdc_fname[drive], OPEN_R);
	if (handle == -1) {
		return;
	}
	if (!file_seek(handle, offset)) {
		file_close(handle);
		return;
	}
	file_read(handle, fdc_buffer, len);
	file_close(handle);
}

/*
 *	D77�t�@�C�� �w�b�_�ǂݍ���
 */
static BOOL FASTCALL fdc_readhead(int drive, int index)
{
	int i;
	DWORD offset;
	DWORD temp;
	int handle;

	/* assert */
	ASSERT((drive >= 0) && (drive < FDC_DRIVES));
	ASSERT((index >= 0) && (index < FDC_MEDIAS));
	ASSERT(fdc_ready[drive] == FDC_TYPE_D77);

	/* �I�t�Z�b�g���� */
	offset = fdc_foffset[drive][index];

	/* �V�[�N�A�ǂݍ��� */
	handle = file_open(fdc_fname[drive], OPEN_R);
	if (handle == -1) {
		return FALSE;
	}
	if (!file_seek(handle, offset)) {
		file_close(handle);
		return FALSE;
	}
	if (!file_read(handle, fdc_header[drive], 0x2b0)) {
		file_close(handle);
		return FALSE;
	}
	file_close(handle);

	/* �^�C�v�`�F�b�N�A���C�g�v���e�N�g�ݒ� */
	if ((fdc_header[drive][0x001b] != 0x00) &&
		(fdc_header[drive][0x001b] != 0x10)) {
		/* 2D/2DD�łȂ� */
		return FALSE;
	}
	if (fdc_fwritep[drive]) {
		fdc_writep[drive] = TRUE;
	}
	else {
		if (fdc_header[drive][0x001a] & 0x10) {
			fdc_writep[drive] = TRUE;
		}
		else {
			fdc_writep[drive] = FALSE;
		}
	}

	/* �g���b�N�I�t�Z�b�g��ݒ� */
	for (i=0; i<164; i++) {
		temp = 0;
		temp |= fdc_header[drive][0x0020 + i * 4 + 3];
		temp *= 256;
		temp |= fdc_header[drive][0x0020 + i * 4 + 2];
		temp *= 256;
		temp |= fdc_header[drive][0x0020 + i * 4 + 1];
		temp *= 256;
		temp |= fdc_header[drive][0x0020 + i * 4 + 0];

		if (temp != 0) {
			/* �f�[�^���� */
			temp += offset;
			*(DWORD *)(&fdc_header[drive][0x0020 + i * 4]) = temp;
		}
	}

	/* �f�B�X�N�T�C�Y�{�I�t�Z�b�g */
	temp = 0;
	temp |= fdc_header[drive][0x001c + 3];
	temp *= 256;
	temp |= fdc_header[drive][0x001c + 2];
	temp *= 256;
	temp |= fdc_header[drive][0x001c + 1];
	temp *= 256;
	temp |= fdc_header[drive][0x001c + 0];
	temp += offset;
	*(DWORD *)(&fdc_header[drive][0x0014]) = temp;

	return TRUE;
}

/*
 *	VFD�t�@�C�� �w�b�_�ǂݍ���
 */
static BOOL FASTCALL fdc_readhead_vfd(int drive)
{
	int handle;

	/* assert */
	ASSERT((drive >= 0) && (drive < FDC_DRIVES));
	ASSERT(fdc_ready[drive] == FDC_TYPE_VFD);

	/* �V�[�N�A�ǂݍ��� */
	handle = file_open(fdc_fname[drive], OPEN_R);
	if (handle == -1) {
		return FALSE;
	}
	if (!file_seek(handle, 0)) {
		file_close(handle);
		return FALSE;
	}
	if (!file_read(handle, fdc_header[drive], 0x1e0)) {
		file_close(handle);
		return FALSE;
	}
	file_close(handle);

	return TRUE;
}

/*
 *	���݂̃��f�B�A�̃��C�g�v���e�N�g��؂�ւ���
 */
BOOL FASTCALL fdc_setwritep(int drive, BOOL writep)
{
	BYTE header[0x2b0];
	DWORD offset;
	int handle;

	/* assert */
	ASSERT((drive >= 0) && (drive < FDC_DRIVES));
	ASSERT((writep == TRUE) || (writep == FALSE));

	/* ���f�B�łȂ���΂Ȃ�Ȃ� */
	if (fdc_ready[drive] == FDC_TYPE_NOTREADY) {
		return FALSE;
	}

	/* �t�@�C�����������ݕs�Ȃ�_�� */
	if (fdc_fwritep[drive]) {
		return FALSE;
	}

	/* �ǂݍ��݁A�ݒ�A�������� */
	if (fdc_ready[drive] == FDC_TYPE_D77) {
		offset = fdc_foffset[drive][fdc_media[drive]];
		handle = file_open(fdc_fname[drive], OPEN_RW);
		if (handle == -1) {
			return FALSE;
		}
		if (!file_seek(handle, offset)) {
			file_close(handle);
			return FALSE;
		}
		if (!file_read(handle, header, 0x2b0)) {
			file_close(handle);
			return FALSE;
		}
		if (writep) {
			header[0x1a] |= 0x10;
		}
		else {
			header[0x1a] &= ~0x10;
		}
		if (!file_seek(handle, offset)) {
			file_close(handle);
			return FALSE;
		}
		if (!file_write(handle, header, 0x2b0)) {
			file_close(handle);
			return FALSE;
		}
	}

	/* ���� */
	file_close(handle);
	fdc_writep[drive] = writep;
	return TRUE;
}

/*
 *	���f�B�A�ԍ���ݒ�
 */
BOOL FASTCALL fdc_setmedia(int drive, int index)
{
	/* assert */
	ASSERT((drive >= 0) && (drive < FDC_DRIVES));
	ASSERT((index >= 0) && (index < FDC_MEDIAS));

	/* ���f�B��Ԃ� */
	if (fdc_ready[drive] == FDC_TYPE_NOTREADY) {
		return FALSE;
	}

	/* 2D/2DD�t�@�C���̏ꍇ�Aindex = 0�� */
	if (((fdc_ready[drive] == FDC_TYPE_2D) ||
		 (fdc_ready[drive] == FDC_TYPE_2DD)) && (index != 0)) {
		return FALSE;
	}

	/* VFD�t�@�C���̏ꍇ�Aindex = 0�����`�F�b�N���ăw�b�_�ǂݍ��� */
	if (fdc_ready[drive] == FDC_TYPE_VFD) {
		if (index != 0) {
			return FALSE;
		}
		if (!fdc_readhead_vfd(drive)) {
			return FALSE;
		}
	}

	/* index > 0 �Ȃ�Afdc_foffset�𒲂ׂ�>0���K�v */
	if (index > 0) {
		if (fdc_foffset[drive][index] == 0) {
			return FALSE;
		}
	}

	/* D77�t�@�C���̏ꍇ�A�w�b�_�ǂݍ��� */
	if (fdc_ready[drive] == FDC_TYPE_D77) {
		/* ���C�g�v���e�N�g�͓����Őݒ� */
		if (!fdc_readhead(drive, index)) {
			return FALSE;
		}
	}
	else {
		/* 2D/2DD/VFD�t�@�C���Ȃ�A�t�@�C�������ɏ]�� */
		fdc_writep[drive] = fdc_fwritep[drive];
	}

	/* ���f�B�A���������ꂽ�ꍇ�A�ꎞ�C�W�F�N�g���������� */
	if (fdc_media[drive] != index) {
		fdc_teject[drive] = FALSE;
	}

	/* �f�[�^�o�b�t�@�ǂݍ��݁A���[�N�Z�[�u */
	fdc_media[drive] = (BYTE)index;
	if (drive == fdc_drvreg) {
		/* �J�����g�h���C�u�ƈ�v�����ꍇ�̂݃o�b�t�@�ǂݍ��݂��s�� */
		fdc_readbuf(drive);
	}

	return TRUE;
}

/*
 *	D77�t�@�C����́A���f�B�A������і��̎擾
 */
static int FASTCALL fdc_chkd77(int drive)
{
	int i;
	int handle;
	int count;
	DWORD offset;
	DWORD len;
	BYTE buf[0x20];

	/* ������ */
	for (i=0; i<FDC_MEDIAS; i++) {
		fdc_foffset[drive][i] = 0;
		fdc_name[drive][i][0] = '\0';
	}
	count = 0;
	offset = 0;

	/* �t�@�C���I�[�v�� */
	handle = file_open(fdc_fname[drive], OPEN_R);
	if (handle == -1) {
		return count;
	}

	/* ���f�B�A���[�v */
	while (count < FDC_MEDIAS) {
		/* �V�[�N */
		if (!file_seek(handle, offset)) {
			file_close(handle);
			return count;
		}

		/* �ǂݍ��� */
		if (!file_read(handle, buf, 0x0020)) {
			file_close(handle);
			return count;
		}

		/* �^�C�v�`�F�b�N�B2D,2DD�̂ݑΉ� */
		if ((buf[0x1b] != 0) && (buf[0x1b] != 0x10)) {
			file_close(handle);
			return count;
		}

		/* ok,�t�@�C�����A�I�t�Z�b�g�i�[ */
		buf[17] = '\0';
		memcpy(fdc_name[drive][count], buf, 17);
		fdc_foffset[drive][count] = offset;

		/* next���� */
		len = 0;
		len |= buf[0x1f];
		len *= 256;
		len |= buf[0x1e];
		len *= 256;
		len |= buf[0x1d];
		len *= 256;
		len |= buf[0x1c];
		offset += len;
		count++;
	}

	/* �ő僁�f�B�A�����ɒB���� */
	file_close(handle);
	return count;
}

/*
 *	VFD�t�@�C���`�F�b�N
 */
static int FASTCALL fdc_chkvfd(int drive)
{
	int	i;
	int handle;
	BYTE buf[0x20];

	/* ������ */
	for (i=0; i<FDC_MEDIAS; i++) {
		fdc_foffset[drive][i] = 0;
		fdc_name[drive][i][0] = '\0';
	}

	/* �t�@�C���I�[�v�� */
	handle = file_open(fdc_fname[drive], OPEN_R);
	if (handle == -1) {
		return 0;
	}

	/* �w�b�_�̃`�F�b�N */
	if (!file_seek(handle, 0)) {
		file_close(handle);
		return 0;
	}
	if (!file_read(handle, buf, 0x0020)) {
		file_close(handle);
		return 0;
	}
	file_close(handle);

	if (	(buf[0x00] != 0xe0) || (buf[0x01] != 0x01) ||
			(buf[0x02] != 0x00) || (buf[0x03] != 0x00)) {
		/* �g���b�N0�̃I�t�Z�b�g��0x01e0�ȊO�Ȃ�VFD�ł͂Ȃ�(�蔲�����c) */
		return 0;
	}

	return 1;
}

/*
 *	�f�B�X�N�t�@�C����ݒ�
 */
int FASTCALL fdc_setdisk(int drive, char *fname)
{
	BOOL writep;
	int handle;
	DWORD fsize;
	int count;
	BOOL flag;
	int i;

	ASSERT((drive >= 0) && (drive < FDC_DRIVES));

	/* �m�b�g���f�B�ɂ���ꍇ */
	if (fname == NULL) {
		fdc_ready[drive] = FDC_TYPE_NOTREADY;
		fdc_fname[drive][0] = '\0';
		return 1;
	}

	/* �t�@�C�����I�[�v�����A�t�@�C���T�C�Y�𒲂ׂ� */
	if (strlen(fname) < sizeof(fdc_fname[drive])) {
		strcpy(fdc_fname[drive], fname);
	}
	else {
		fdc_ready[drive] = FDC_TYPE_NOTREADY;
		fdc_fname[drive][0] = '\0';
		return 1;
	}
	writep = FALSE;
	handle = file_open(fdc_fname[drive], OPEN_RW);
	if (handle == -1) {
		handle = file_open(fdc_fname[drive], OPEN_R);
		if (handle == -1) {
			return 0;
		}
		writep = TRUE;
	}
	fsize = file_getsize(handle);
	file_close(handle);

	/*
	 * 2D�t�@�C��
	 */
	if (fsize == 327680) {
		/* �^�C�v�A�������ݑ����ݒ� */
		fdc_ready[drive] = FDC_TYPE_2D;
		fdc_fwritep[drive] = writep;

		/* ���f�B�A�ݒ� */
		if (!fdc_setmedia(drive, 0)) {
			fdc_ready[drive] = FDC_TYPE_NOTREADY;
			return 0;
		}

		/* �����B�ꎞ�C�W�F�N�g���� */
		fdc_teject[drive] = FALSE;
		fdc_medias[drive] = 1;
		return 1;
	}

	/*
	 * 2DD�t�@�C��
	 */
	if (fsize == 655360) {
		/* �^�C�v�A�������ݑ����ݒ� */
		fdc_ready[drive] = FDC_TYPE_2DD;
		fdc_fwritep[drive] = writep;

		/* ���f�B�A�ݒ� */
		if (!fdc_setmedia(drive, 0)) {
			fdc_ready[drive] = FDC_TYPE_NOTREADY;
			return 0;
		}

		/* �����B�ꎞ�C�W�F�N�g���� */
		fdc_teject[drive] = FALSE;
		fdc_medias[drive] = 1;
		return 1;
	}

	/*
	 * VFD�t�@�C��
	 */
	/* �t�@�C������ */
	if (fdc_chkvfd(drive) != 0) {
		fdc_ready[drive] = FDC_TYPE_VFD;
		fdc_fwritep[drive] = writep;

		/* ���f�B�A�ݒ� */
		if (!fdc_setmedia(drive, 0)) {
			fdc_ready[drive] = FDC_TYPE_NOTREADY;
			return 0;
		}

		/* �����B�ꎞ�C�W�F�N�g���� */
		fdc_teject[drive] = FALSE;
		fdc_medias[drive] = 1;
		return 1;
	}

	/*
	 * D77�t�@�C��
	 */
	fdc_ready[drive] = FDC_TYPE_D77;
	fdc_fwritep[drive] = writep;

	/* �t�@�C������ */
	count = fdc_chkd77(drive);
	if (count == 0){
		fdc_ready[drive] = FDC_TYPE_NOTREADY;
		return 0;
	}

	/* ���f�B�A�ݒ� */
	flag = FALSE;
	for (i = 0; i < 16; i++) {
		if (fdc_setmedia(drive, i)) {
			flag = TRUE;
			break;
		}
	}
	if (!flag) {
		fdc_ready[drive] = FDC_TYPE_NOTREADY;
		return 0;
	}

	/* �����B�ꎞ�C�W�F�N�g���� */
	fdc_teject[drive] = FALSE;
	fdc_medias[drive] = (BYTE)count;
	return count;
}

/*-[ FDC�R�}���h �E�F�C�g���[�h���� ]---------------------------------------*/

/*
 *	FDC DataRequest�C�x���g (WAIT)
 */
#if defined(FDDSND)
static BOOL FASTCALL fdc_drq_event(void)
{
	/* ���X�g�f�[�^�`�F�b�N */
	if (fdc_drqirq & 0x80) {
		return fdc_lost_event();
	}

	fdc_status |= FDC_ST_DRQ;
	fdc_drqirq |= 0x80;

	return TRUE;
}

/*
 *	FDC �V�[�N�C�x���g (WAIT)
 */
static BOOL FASTCALL fdd_seek_event(void)
{
	/* �V�[�N�������I�������� */
	if (fdc_seek_track < 0) {
		/* FDC���荞�� */
		mainetc_fdc();
		fdc_drqirq = 0x40;

		/* BUSY�t���O�𗎂Ƃ� */
		fdc_status &= ~FDC_ST_BUSY;
		fdc_access[fdc_drvreg] = FDC_ACCESS_READY;
		schedule_delevent(EVENT_FDD_SEEK);
	}
	else if (fdc_seek_track-- == 0) {
		/* �V�[�N�I����̓E�F�C�g��}�� */
		schedule_setevent(EVENT_FDD_SEEK, 20000, fdd_seek_event);
	}
	else {
		/* �T�E���h���� */
		if (fdc_sound) {
			wav_notify(SOUND_FDDSEEK);
		}
	}

	return TRUE;
}

/*
 *	FDC �V�[�N�C�x���g�ݒ� (SOUND ON)
 */
static void FASTCALL fdc_setseekevent(BYTE track)
{
	/* �V�[�N����g���b�N����ۑ� */
	fdc_seek_track = track;

	/* �C�x���g�ݒ� */
	if (track > 0) {
		schedule_setevent(EVENT_FDD_SEEK, fdc_steprate[fdc_command & 3], fdd_seek_event);
	}
	else {
		/* �w�b�h�ړ����Ȃ��Ă����΂炭�̊Ԃ�BUSY��Ԃɂ��� */
		schedule_setevent(EVENT_FDD_SEEK, 300, fdd_seek_event);
	}
}
#endif

/*-[ FDC�R�}���h ]----------------------------------------------------------*/

/*
 *	FDC �X�e�[�^�X�쐬
 */
static void FASTCALL fdc_make_stat(void)
{
	/* �h���C�u�`�F�b�N */
	if (fdc_drvreg >= FDC_DRIVES) {
		fdc_status |= FDC_ST_NOTREADY;
		return;
	}

	/* �L���ȃh���C�u */
	if (fdc_ready[fdc_drvreg] == FDC_TYPE_NOTREADY) {
		fdc_status |= FDC_ST_NOTREADY;
	}
	else {
		fdc_status &= (BYTE)(~FDC_ST_NOTREADY);
	}

	/* �ꎞ�C�W�F�N�g */
	if (fdc_teject[fdc_drvreg]) {
		fdc_status |= FDC_ST_NOTREADY;
	}

	/* ���C�g�v���e�N�g(Read�n�R�}���h��0) */
	if ((fdc_cmdtype == 2) || (fdc_cmdtype == 4) || (fdc_cmdtype == 6)) {
		fdc_status &= ~FDC_ST_WRITEP;
	}
	else {
		if (fdc_writep[fdc_drvreg] || (fdc_status & FDC_ST_NOTREADY)) {
			fdc_status |= FDC_ST_WRITEP;
		}
		else {
			fdc_status &= ~FDC_ST_WRITEP;
		}
	}

	/* TYPE I �̂� */
	if (fdc_cmdtype != 1) {
		return;
	}

	/* TRACK00 */
	if (fdc_track[fdc_drvreg] == 0) {
		fdc_status |= FDC_ST_TRACK00;
	}
	else {
		fdc_status &= ~FDC_ST_TRACK00;
	}

	/* index */
	if (!(fdc_status & FDC_ST_NOTREADY)) {
		if (fdc_indexcnt == 0) {
			fdc_status |= FDC_ST_INDEX;
		}
		else {
			fdc_status &= ~FDC_ST_INDEX;
		}
		fdc_next_index();
	}
}

/*
 *	TYPE I
 *	RESTORE
 */
static void FASTCALL fdc_restore(void)
{
#if defined(FDDSND)
	BYTE prevtrk;
#endif

	/* TYPE I */
	fdc_cmdtype = 1;
	fdc_status = 0;

	/* �h���C�u�`�F�b�N */
	if (fdc_drvreg >= FDC_DRIVES) {
		mainetc_fdc();
		fdc_drqirq = 0x40;
		fdc_make_stat();
		return;
	}

#if defined(FDDSND)
	/* ���݂̃g���b�N�ԍ���ۑ� */
	prevtrk = fdc_track[fdc_drvreg];
#endif

	/* �g���b�N��r�A���X�g�A�A�ǂݍ��� */
	if (fdc_track[fdc_drvreg] != 0) {
		fdc_track[fdc_drvreg] = 0;
		fdc_readbuf(fdc_drvreg);
	}
	fdc_seekvct = TRUE;

	/* �����A�b�v�f�[�g(�f�[�^���W�X�^��0�ɂ���) */
	fdc_trkreg = 0;
	fdc_datareg = 0;

	/* FDC���荞�� */
	fdc_drqirq = 0x00;
#if defined(FDDSND)
	if (fdc_wait) {
		fdc_setseekevent(prevtrk);
	}
	else {
		if (!mfd_irq_mask) {
			mainetc_fdc();
			fdc_drqirq |= (BYTE)0x40;
		}
	}
#else
	if (!mfd_irq_mask) {
		mainetc_fdc();
		fdc_drqirq |= (BYTE)0x40;
	}
#endif

	/* �X�e�[�^�X���� */
	fdc_status = FDC_ST_BUSY;
	if (fdc_command & 0x08) {
		fdc_status |= FDC_ST_HEADENG;
	}
	fdc_make_stat();

	/* �A�N�Z�X(SEEK) */
	fdc_access[fdc_drvreg] = FDC_ACCESS_SEEK;
}

/*
 *	TYPE I
 *	SEEK
 */
static void FASTCALL fdc_seek(void)
{
	BYTE target;
#if defined(FDDSND)
	BYTE prevtrk;
#endif

	/* TYPE I */
	fdc_cmdtype = 1;
	fdc_status = 0;

	/* �h���C�u�`�F�b�N */
	if (fdc_drvreg >= FDC_DRIVES) {
		mainetc_fdc();
		fdc_drqirq = 0x40;
		fdc_make_stat();
		return;
	}

	/* ��Ƀx���t�@�C */
	if (fdc_command & 0x04) {
		if (fdc_trkreg != fdc_track[fdc_drvreg]) {
			fdc_status |= FDC_ST_SEEKERR;
			/* �����C��(��ՂĂ����΍�) */
			fdc_trkreg = fdc_track[fdc_drvreg];
		}
	}

#if defined(FDDSND)
	/* ���݂̃g���b�N�ԍ���ۑ� */
	prevtrk = fdc_track[fdc_drvreg];
#endif

	/* ���΃V�[�N */
	target = (BYTE)(fdc_track[fdc_drvreg] + fdc_datareg - fdc_trkreg);
	if (fdc_datareg > fdc_trkreg) {
		fdc_seekvct = FALSE;
#if XM7_VER >= 3
		if (fdc_2ddmode) {
			if (target > 81) {
				target = 81;
			}
		}
		else {
			if (target > 41) {
				target = 41;
			}
		}
#else
		if (target > 41) {
			target = 41;
		}
#endif
	}
	else {
		fdc_seekvct = TRUE;
		/* �g���b�N�ԍ��̃I�[�o�[�t���[��h�~ (OS-9�΍�) */
		if (fdc_track[fdc_drvreg] < (fdc_trkreg - fdc_datareg)) {
			target = 0;
		}
	}

	/* �g���b�N��r�A�V�[�N�A�ǂݍ��� */
	if (fdc_track[fdc_drvreg] != target) {
		fdc_track[fdc_drvreg] = target;
		fdc_readbuf(fdc_drvreg);
	}

	/* �A�b�v�f�[�g */
	fdc_trkreg = fdc_datareg;	/* �f�[�^���W�X�^�̒l�����f�����͗l */

	/* FDC���荞�� */
	fdc_drqirq = 0x00;
#if defined(FDDSND)
	if (fdc_wait) {
		fdc_setseekevent((BYTE)abs(target - prevtrk));
	}
	else {
		if (!mfd_irq_mask) {
			mainetc_fdc();
			fdc_drqirq |= (BYTE)0x40;
		}
	}
#else
	if (!mfd_irq_mask) {
		mainetc_fdc();
		fdc_drqirq |= (BYTE)0x40;
	}
#endif

	/* �X�e�[�^�X���� */
	fdc_status |= FDC_ST_BUSY;
	if (fdc_command & 0x08) {
		fdc_status |= FDC_ST_HEADENG;
	}
	fdc_make_stat();

	/* �A�N�Z�X(SEEK) */
	fdc_access[fdc_drvreg] = FDC_ACCESS_SEEK;
}

/*
 *	TYPE I
 *	STEP IN
 */
static void FASTCALL fdc_step_in(void)
{
#if defined(FDDSND)
	BYTE prevtrk;
#endif

	/* TYPE I */
	fdc_cmdtype = 1;
	fdc_status = 0;

	/* �h���C�u�`�F�b�N */
	if (fdc_drvreg >= FDC_DRIVES) {
		mainetc_fdc();
		fdc_drqirq = 0x40;
		fdc_make_stat();
		return;
	}

#if defined(FDDSND)
	/* ���݂̃g���b�N�ԍ���ۑ� */
	prevtrk = fdc_track[fdc_drvreg];
#endif

	/* ��Ƀx���t�@�C */
	if (fdc_command & 0x04) {
		if (fdc_trkreg != fdc_track[fdc_drvreg]) {
			fdc_status |= FDC_ST_SEEKERR;
		}
	}

	/* �X�e�b�v�A�ǂݍ��� */
#if XM7_VER >= 3
	if (fdc_2ddmode) {
		if (fdc_track[fdc_drvreg] < 81) {
			fdc_track[fdc_drvreg]++;
			fdc_readbuf(fdc_drvreg);
		}
	}
	else {
		if (fdc_track[fdc_drvreg] < 41) {
			fdc_track[fdc_drvreg]++;
			fdc_readbuf(fdc_drvreg);
		}
	}
#else
	if (fdc_track[fdc_drvreg] < 41) {
		fdc_track[fdc_drvreg]++;
		fdc_readbuf(fdc_drvreg);
	}
#endif
	fdc_seekvct = FALSE;

	/* �A�b�v�f�[�g */
	if (fdc_command & 0x10) {
		if (fdc_trkreg < 255) {
			fdc_trkreg++;	/* �g���b�N���W�X�^���̂̍X�V�݂̂Ǝv���� */
		}
	}

	/* FDC���荞�� */
	fdc_drqirq = 0x00;
#if defined(FDDSND)
	if (fdc_wait && (prevtrk != fdc_track[fdc_drvreg])) {
		fdc_setseekevent(1);
	}
	else {
		if (!mfd_irq_mask) {
			mainetc_fdc();
			fdc_drqirq |= (BYTE)0x40;
		}
	}
#else
	if (!mfd_irq_mask) {
		mainetc_fdc();
		fdc_drqirq |= (BYTE)0x40;
	}
#endif

	/* �X�e�[�^�X���� */
	fdc_status |= FDC_ST_BUSY;
	if (fdc_command & 0x08) {
		fdc_status |= FDC_ST_HEADENG;
	}
	fdc_make_stat();

	/* �A�N�Z�X(SEEK) */
	fdc_access[fdc_drvreg] = FDC_ACCESS_SEEK;
}

/*
 *	TYPE I
 *	STEP OUT
 */
static void FASTCALL fdc_step_out(void)
{
#if defined(FDDSND)
	BYTE prevtrk;
#endif

	/* TYPE I */
	fdc_cmdtype = 1;
	fdc_status = 0;

	/* �h���C�u�`�F�b�N */
	if (fdc_drvreg >= FDC_DRIVES) {
		mainetc_fdc();
		fdc_drqirq = 0x40;
		fdc_make_stat();
		return;
	}

#if defined(FDDSND)
	/* ���݂̃g���b�N�ԍ���ۑ� */
	prevtrk = fdc_track[fdc_drvreg];
#endif

	/* ��Ƀx���t�@�C */
	if (fdc_command & 0x04) {
		if (fdc_trkreg != fdc_track[fdc_drvreg]) {
			fdc_status |= FDC_ST_SEEKERR;
		}
	}

	/* �X�e�b�v�A�ǂݍ��� */
	if (fdc_track[fdc_drvreg] != 0) {
		fdc_track[fdc_drvreg]--;
		fdc_readbuf(fdc_drvreg);
	}
	fdc_seekvct = TRUE;

	/* �A�b�v�f�[�g */
	if (fdc_command & 0x10) {
		if (fdc_trkreg > 0) {
			fdc_trkreg--;	/* �g���b�N���W�X�^���̂̍X�V�݂̂Ǝv���� */
		}
	}

	/* FDC���荞�� */
	fdc_drqirq = 0x00;
#if defined(FDDSND)
	if (fdc_wait && (prevtrk != fdc_track[fdc_drvreg])) {
		fdc_setseekevent(1);
	}
	else {
		if (!mfd_irq_mask) {
			mainetc_fdc();
			fdc_drqirq |= (BYTE)0x40;
		}
	}
#else
	if (!mfd_irq_mask) {
		mainetc_fdc();
		fdc_drqirq |= (BYTE)0x40;
	}
#endif

	/* �X�e�[�^�X���� */
	fdc_status |= FDC_ST_BUSY;
	if (fdc_command & 0x08) {
		fdc_status |= FDC_ST_HEADENG;
	}
	fdc_make_stat();

	/* �A�N�Z�X(SEEK) */
	fdc_access[fdc_drvreg] = FDC_ACCESS_SEEK;
}

/*
 *	TYPE I
 *	STEP
 */
static void FASTCALL fdc_step(void)
{
	if (fdc_seekvct) {
		fdc_step_out();
	}
	else {
		fdc_step_in();
	}
}

/*
 *	TYPE II, III
 *	READ/WRITE �T�u
 */
static BOOL FASTCALL fdc_rw_sub(void)
{
	fdc_status = 0;

	/* �h���C�u�`�F�b�N */
	if (fdc_drvreg >= FDC_DRIVES) {
		fdc_make_stat();
		/* FDC���荞�� */
		mainetc_fdc();
		fdc_drqirq = 0x40;
		return FALSE;
	}

	/* NOT READY�`�F�b�N */
	if ((fdc_ready[fdc_drvreg] == FDC_TYPE_NOTREADY) || fdc_teject[fdc_drvreg]) {
		fdc_make_stat();
		/* FDC���荞�� */
		mainetc_fdc();
		fdc_drqirq = 0x40;
		return FALSE;
	}

	return TRUE;
}

/*
 *	TYPE II
 *	READ DATA
 */
static void FASTCALL fdc_read_data(void)
{
	BYTE stat;

	/* TYPE II, Read */
	fdc_cmdtype = 2;

	/* ��{�`�F�b�N */
	if (!fdc_rw_sub()) {
		return;
	}

	/* �Z�N�^���� */
	if (fdc_command & 0x02) {
		stat = fdc_readsec(fdc_trkreg, fdc_secreg, (BYTE)((fdc_command & 0x08) >> 3), TRUE);
	}
	else {
		stat = fdc_readsec(fdc_trkreg, fdc_secreg, fdc_sidereg, FALSE);
	}

	/* ��ɃX�e�[�^�X��ݒ肷�� */
	fdc_status = stat;

	/* RECORD NOT FOUND ? */
	if (fdc_status & FDC_ST_RECNFND) {
		/* FDC���荞�� */
		mainetc_fdc();
		fdc_drqirq = 0x40;
		fdc_access[fdc_drvreg] = FDC_ACCESS_READY;
		return;
	}

	/* �ŏ��̃f�[�^��ݒ� */
	fdc_status |= FDC_ST_BUSY;
	fdc_datareg = fdc_dataptr[0];
#if defined(FDDSND)
	if (fdc_wait) {
		schedule_setevent(EVENT_FDC_L, FDC_LOST_TIME * 48, fdc_drq_event);
		fdc_drqirq = 0x00;
	}
	else {
		fdc_status |= FDC_ST_DRQ;
		fdc_drqirq = 0x80;
	}
#else
	fdc_status |= FDC_ST_DRQ;
	fdc_drqirq = 0x80;
#endif

	/* �A�N�Z�X(READ) */
	fdc_access[fdc_drvreg] = FDC_ACCESS_READ;

	/* DMA�]���J�n�A���X�g�f�[�^�C�x���g�ݒ� */
#if XM7_VER >= 3
	dmac_start();
#endif
#if defined(FDDSND)
	if (!fdc_wait) {
		schedule_setevent(EVENT_FDC_L, 30 * 1000, fdc_lost_event);
	}
#else
	schedule_setevent(EVENT_FDC_L, 30 * 1000, fdc_lost_event);
#endif
}

/*
 *	TYPE II
 *	WRITE DATA
 */
static void FASTCALL fdc_write_data(void)
{
	BYTE stat;

	/* TYPE II, Write */
	fdc_cmdtype = 3;

	/* ��{�`�F�b�N */
	if (!fdc_rw_sub()) {
		return;
	}

	/* WRITE PROTECT�`�F�b�N */
	if (fdc_writep[fdc_drvreg] != 0) {
		fdc_make_stat();
		/* FDC���荞�� */
		mainetc_fdc();
		fdc_drqirq = 0x40;
		return;
	}

	/* �Z�N�^���� */
	if (fdc_command & 0x02) {
		stat = fdc_readsec(fdc_trkreg, fdc_secreg, (BYTE)((fdc_command & 0x08) >> 3), TRUE);
	}
	else {
		stat = fdc_readsec(fdc_trkreg, fdc_secreg, fdc_sidereg, FALSE);
	}

	/* ��ɃX�e�[�^�X��ݒ肷�� */
	fdc_status = stat;

	/* RECORD NOT FOUND ? */
	if (fdc_status & FDC_ST_RECNFND) {
		fdc_status = FDC_ST_RECNFND;
		/* FDC���荞�� */
		mainetc_fdc();
		fdc_drqirq = 0x40;
		fdc_access[fdc_drvreg] = FDC_ACCESS_READY;
		return;
	}

	/* DRQ */
#if defined(FDDSND)
	fdc_status = FDC_ST_BUSY;
	if (fdc_wait) {
		schedule_setevent(EVENT_FDC_L, FDC_LOST_TIME * 48, fdc_drq_event);
		fdc_drqirq = 0x00;
	}
	else {
		fdc_status |= FDC_ST_DRQ;
		fdc_drqirq = 0x80;
	}
#else
	fdc_status = FDC_ST_BUSY | FDC_ST_DRQ;
	fdc_drqirq = 0x80;
#endif

	/* �A�N�Z�X(WRITE) */
	fdc_access[fdc_drvreg] = FDC_ACCESS_WRITE;

	/* DMA�]���J�n�A���X�g�f�[�^�C�x���g�ݒ� */
#if XM7_VER >= 3
	dmac_start();
#endif
#if defined(FDDSND)
	if (!fdc_wait) {
		schedule_setevent(EVENT_FDC_L, 30 * 1000, fdc_lost_event);
	}
#else
	schedule_setevent(EVENT_FDC_L, 30 * 1000, fdc_lost_event);
#endif
}

/*
 *	TYPE III
 *	READ ADDRESS
 */
static void FASTCALL fdc_read_addr(void)
{
	int idx;

	/* TYPE III, Read Address */
	fdc_cmdtype = 4;

	/* ��{�`�F�b�N */
	if (!fdc_rw_sub()) {
		return;
	}

	/* �g���b�N�擪����̑��΃Z�N�^�ԍ����擾 */
	idx = fdc_next_index();

	/* �A���t�H�[�}�b�g�`�F�b�N */
	if (idx == -1) {
		fdc_status = FDC_ST_RECNFND;
		/* FDC���荞�� */
		mainetc_fdc();
		fdc_drqirq = 0x40;
		return;
	}

	/* �f�[�^�쐬 */
	fdc_makeaddr(idx);

	/* �ŏ��̃f�[�^��ݒ� */
	fdc_status |= FDC_ST_BUSY;
	fdc_datareg = fdc_dataptr[0];
#if defined(FDDSND)
	if (fdc_wait) {
		schedule_setevent(EVENT_FDC_L, FDC_LOST_TIME * 4, fdc_drq_event);
		fdc_drqirq = 0x00;
	}
	else {
		fdc_status |= FDC_ST_DRQ;
		fdc_drqirq = 0x80;
	}
#else
	fdc_status |= FDC_ST_DRQ;
	fdc_drqirq = 0x80;
#endif

	/* �A�N�Z�X(READ) */
	fdc_access[fdc_drvreg] = FDC_ACCESS_READ;

	/* DMA�]���J�n�A���X�g�f�[�^�C�x���g�ݒ� */
#if XM7_VER >= 3
	dmac_start();
#endif
#if defined(FDDSND)
	if (!fdc_wait) {
		schedule_setevent(EVENT_FDC_L, 10 * 1000, fdc_lost_event);
	}
#else
	schedule_setevent(EVENT_FDC_L, 10 * 1000, fdc_lost_event);
#endif
}

/*
 *	TYPE III
 *	READ TRACK
 */
static void FASTCALL fdc_read_track(void)
{
	/* TYPE III, Read Track */
	fdc_cmdtype = 6;

	/* ��{�`�F�b�N */
	if (!fdc_rw_sub()) {
		return;
	}

	/* �f�[�^�쐬 */
	fdc_make_track();

	/* �ŏ��̃f�[�^��ݒ� */
	fdc_status |= FDC_ST_BUSY;
	fdc_datareg = fdc_dataptr[0];
#if defined(FDDSND)
	if (fdc_wait) {
		schedule_setevent(EVENT_FDC_L, FDC_LOST_TIME, fdc_drq_event);
		fdc_drqirq = 0x00;
	}
	else {
		fdc_status |= FDC_ST_DRQ;
		fdc_drqirq = 0x80;
	}
#else
	fdc_status |= FDC_ST_DRQ;
	fdc_drqirq = 0x80;
#endif

	/* �A�N�Z�X(READ) */
	fdc_access[fdc_drvreg] = FDC_ACCESS_READ;

	/* DMA�]���J�n�A���X�g�f�[�^�C�x���g�ݒ� */
#if XM7_VER >= 3
	dmac_start();
#endif
#if defined(FDDSND)
	if (!fdc_wait) {
		schedule_setevent(EVENT_FDC_L, 150 * 1000, fdc_lost_event);
	}
#else
	schedule_setevent(EVENT_FDC_L, 150 * 1000, fdc_lost_event);
#endif
}

/*
 *	TYPE III
 *	WRITE TRACK
 */
static void FASTCALL fdc_write_track(void)
{
	fdc_status = 0;

	/* TYPE III, Write */
	fdc_cmdtype = 5;

	/* ��{�`�F�b�N */
	if (!fdc_rw_sub()) {
		return;
	}

	/* WRITE PROTECT�`�F�b�N */
	if (fdc_writep[fdc_drvreg] != 0) {
		fdc_make_stat();
		/* FDC���荞�� */
		mainetc_fdc();
		fdc_drqirq = 0x40;
		return;
	}

	/* DRQ */
#if defined(FDDSND)
	fdc_status = FDC_ST_BUSY;
	if (fdc_wait) {
		schedule_setevent(EVENT_FDC_L, FDC_LOST_TIME, fdc_drq_event);
		fdc_drqirq = 0x00;
	}
	else {
		fdc_status |= FDC_ST_DRQ;
		fdc_drqirq = 0x80;
	}
#else
	fdc_status = FDC_ST_BUSY | FDC_ST_DRQ;
	fdc_drqirq = 0x80;
#endif

	fdc_dataptr = fdc_buffer;
	fdc_totalcnt = 0x1800;
	fdc_nowcnt = 0;

	/* �A�N�Z�X(WRITE) */
	fdc_access[fdc_drvreg] = FDC_ACCESS_WRITE;

	/* DMA�]���J�n�A���X�g�f�[�^�C�x���g�ݒ� */
#if XM7_VER >= 3
	dmac_start();
#endif
#if defined(FDDSND)
	if (!fdc_wait) {
		schedule_setevent(EVENT_FDC_L, 150 * 1000, fdc_lost_event);
	}
#else
	schedule_setevent(EVENT_FDC_L, 150 * 1000, fdc_lost_event);
#endif
}

/*
 *	TYPE IV
 *	FORCE INTERRUPT
 */
static void FASTCALL fdc_force_intr(void)
{
	/* WRITE TRACK�I������ */
	if (fdc_cmdtype == 5) {
		if (!fdc_writetrk()) {
			fdc_status |= FDC_ST_WRITEFAULT;
		}
		/* �t�H�[�}�b�g�̂��ߎg�p�����o�b�t�@���C�� */
		fdc_readbuf(fdc_drvreg);
	}

	/* �A�N�Z�X��~ */
#if defined(FDDSND)
	if (!fdc_wait) {
		schedule_delevent(EVENT_FDC_L);
	}
#endif
	fdc_readbuf(fdc_drvreg);
	fdc_dataptr = NULL;

	/* �X�e�[�^�X���� */
	switch (fdc_access[fdc_drvreg]) {
		case FDC_ACCESS_READ:
		case FDC_ACCESS_WRITE:
			/* ���s���̃R�}���h�Ɠ����X�e�[�^�X������ */
			fdc_status &= ~FDC_ST_BUSY;
			break;

		case FDC_ACCESS_SEEK:
		case FDC_ACCESS_READY:
			fdc_status = 0;
			fdc_indexcnt = 0;
			fdc_make_stat();
			break;
	}

	/* �����ŃA�N�Z�XREADY�� */
	fdc_access[fdc_drvreg] = FDC_ACCESS_READY;

	/* �����ꂩ�������Ă���΁AIRQ����(�����Ƃ��Ă͕s�\��) */
	if (fdc_command & 0x0f) {
		mainetc_fdc();
		fdc_drqirq = 0x40;
	}
	else {
		fdc_drqirq = 0x00;
	}
}

/*
 *	�}���`�Z�N�^
 *	�C�x���g
 */
static BOOL FASTCALL fdc_multi_event(void)
{
	if (fdc_drqirq & 0x10) {
		fdc_drqirq = 0x08;
		/* �Z�N�^���W�X�^��Up�A�Z�N�^������ */
		fdc_secreg++;
		schedule_setevent(EVENT_FDC_M, 30, fdc_multi_event);
		return TRUE;
	}

	if (fdc_drqirq & 0x08) {
		fdc_drqirq = 0x00;
		/* �Z�N�^�������� */
		if (fdc_cmdtype == 2) {
			fdc_read_data();
			fdc_drqirq |= 0x20;
		}
		if (fdc_cmdtype == 3) {
			fdc_write_data();
			fdc_drqirq |= 0x20;
		}
	}

	/* �C�x���g�폜���ďI�� */
	schedule_delevent(EVENT_FDC_M);
	return TRUE;
}

/*
 *	���X�g�f�[�^
 *	�C�x���g
 */
static BOOL FASTCALL fdc_lost_event(void)
{
	if (fdc_dataptr && (fdc_drqirq & 0x80)) {
		/* �R�}���h�ł��؂�A���X�g�f�[�^ */
		fdc_dataptr = NULL;
		fdc_status |= FDC_ST_LOSTDATA;
		fdc_status &= ~FDC_ST_DRQ;
		fdc_status &= ~FDC_ST_BUSY;
		fdc_access[fdc_drvreg] = FDC_ACCESS_READY;

		/* ���荞�ݔ��� */
		mainetc_fdc();
		fdc_drqirq = 0x40;
	}

	/* �C�x���g�폜���ďI�� */
	schedule_delevent(EVENT_FDC_L);
	return TRUE;
}


/*
 *	�R�}���h����
 */
static void FASTCALL fdc_process_cmd(void)
{
	BYTE high;

	high = (BYTE)(fdc_command >> 4);

#if defined(FDDSND)
	/* �E�F�C�g�t���O�ݒ� */
	fdc_wait = fdc_waitmode;
#endif

	/* �f�[�^�]�������s���Ă���΁A�����~�߂� */
	fdc_dataptr = NULL;

	/* ���� */
	switch (high) {
		/* restore */
		case 0x00:
			fdc_restore();
			break;
		/* seek */
		case 0x01:
			fdc_seek();
			break;
		/* step */
		case 0x02:
		case 0x03:
			fdc_step();
			break;
		/* step in */
		case 0x04:
		case 0x05:
			fdc_step_in();
			break;
		/* step out */
		case 0x06:
		case 0x07:
			fdc_step_out();
			break;
		/* read data */
		case 0x08:
		case 0x09:
			fdc_read_data();
			break;
		/* write data */
		case 0x0a:
		case 0x0b:
			fdc_write_data();
			break;
		/* read address */
		case 0x0c:
			fdc_read_addr();
			break;
		/* force interrupt */
		case 0x0d:
			fdc_force_intr();
			break;
		/* read track */
		case 0x0e:
			fdc_read_track();
			break;
		/* write track */
		case 0x0f:
			fdc_write_track();
			break;
		/* ����ȊO */
		default:
			ASSERT(FALSE);
			break;
	}
}

/*
 *	FDC
 *	�P�o�C�g�ǂݏo��
 */
BOOL FASTCALL fdc_readb(WORD addr, BYTE *dat)
{
#if XM7_VER >= 3
	BYTE tmp;
#endif

#if XM7_VER == 1
	if (!fdc_enable && (fm_subtype != FMSUB_FM77)) {
#else
	if (!fdc_enable && (fm7_ver == 1)) {
#endif
		return FALSE;
	}

	switch (addr) {
		/* �X�e�[�^�X���W�X�^ */
		case 0xfd18:
			fdc_make_stat();
			*dat = fdc_status;
			/* �X�e�[�^�X���`�F�b�N����(�������) */
			fdc_drqirq |= (BYTE)0x20;
			/* BUSY���� */
			if ((fdc_status & FDC_ST_BUSY) && (fdc_dataptr == NULL)) {
#if defined(FDDSND)
				if (!fdc_wait && (fdc_cmdtype == 1)) {
#else
				if (fdc_cmdtype == 1) {
#endif
					/* IRQ On */
					fdc_drqirq |= (BYTE)0x40;
				}
#if defined(FDDSND)
				if (!fdc_wait || (fdc_cmdtype != 1) || (fdc_drqirq & 0x40)) {
					/* BUSY�t���O�𗎂Ƃ� */
					fdc_status &= ~FDC_ST_BUSY;
					fdc_access[fdc_drvreg] = FDC_ACCESS_READY;
				}
#else
				/* BUSY�t���O�𗎂Ƃ� */
				fdc_status &= ~FDC_ST_BUSY;
				fdc_access[fdc_drvreg] = FDC_ACCESS_READY;
#endif
				/* ��x����BUSY�������� */
				return TRUE;
			}
			/* FDC���荞�݂��A�����Ŏ~�߂� */
			mfd_irq_flag = FALSE;
			fdc_drqirq &= ~0x40;
			maincpu_irq();
			return TRUE;

		/* �g���b�N���W�X�^ */
		case 0xfd19:
			*dat = fdc_trkreg;
			return TRUE;

		/* �Z�N�^���W�X�^ */
		case 0xfd1a:
			*dat = fdc_secreg;
			return TRUE;

		/* �f�[�^���W�X�^ */
		case 0xfd1b:
			*dat = fdc_datareg;
			/* �J�E���^���� */
			if (fdc_dataptr && (fdc_drqirq & 0x20)) {
				fdc_nowcnt++;
				/* DRQ,IRQ���� */
				if (fdc_nowcnt == fdc_totalcnt) {
#if defined(FDDSND)
					if (fdc_wait) {
						schedule_delevent(EVENT_FDC_L);
					}
#endif
					fdc_status &= ~FDC_ST_BUSY;
					fdc_status &= ~FDC_ST_DRQ;
					fdc_drqirq &= (BYTE)~0x80;

					if ((fdc_cmdtype == 2) && (fdc_command & 0x10)) {
						/* �}���`�Z�N�^���� */
						fdc_status |= FDC_ST_BUSY;
						fdc_dataptr = NULL;
						fdc_drqirq = 0x10;
						schedule_setevent(EVENT_FDC_M, 30, fdc_multi_event);
						return TRUE;
					}
					/* �V���O���Z�N�^ */
					fdc_dataptr = NULL;
					mainetc_fdc();
					fdc_drqirq = 0x40;
					fdc_access[fdc_drvreg] = FDC_ACCESS_READY;
					/* Read Track�͕K��LOST DATA�A�o�b�t�@�C�� */
					if (fdc_cmdtype == 6) {
						fdc_status |= FDC_ST_LOSTDATA;
						fdc_readbuf(fdc_drvreg);
					}
				}
				else {
					fdc_datareg = fdc_dataptr[fdc_nowcnt];
#if defined(FDDSND)
					if (fdc_wait) {
						schedule_setevent(EVENT_FDC_L, FDC_LOST_TIME, fdc_drq_event);
						fdc_status &= (BYTE)~FDC_ST_DRQ;
						fdc_drqirq &= (BYTE)~0x80;
					}
					else {
						fdc_drqirq |= (BYTE)0x80;
					}
#else
					fdc_drqirq |= (BYTE)0x80;
#endif
				}
			}
			return TRUE;

		/* �w�b�h���W�X�^ */
		case 0xfd1c:
			*dat = (BYTE)(fdc_sidereg | 0xfe);
			return TRUE;

		/* �h���C�u���W�X�^ */
		case 0xfd1d:
#if XM7_VER >= 3
			if (fdc_motor) {
				*dat = (BYTE)(0xbc | fdc_drvregP);
			}
			else {
				*dat = (BYTE)(0x3c | fdc_drvregP);
			}
#else
			if (fdc_motor) {
				*dat = (BYTE)(0xbc | fdc_drvreg);
			}
			else {
				*dat = (BYTE)(0x3c | fdc_drvreg);
			}
#endif
			return TRUE;

		/* ���[�h���W�X�^ */
		case 0xfd1e:
#if XM7_VER >= 3
			if (fm7_ver < 3) {
				*dat = 0xff;
				return TRUE;
			}

			/* �_���h���C�u�E�����h���C�u�Ή��ǂݏo�� */
			tmp = (BYTE)(fdc_logidrv << 2) | (fdc_physdrv[fdc_logidrv]);
			/* 2DD���[�h�ǂݏo�� */
			if (fdc_2ddmode) {
				*dat = (BYTE)(tmp | 0xb0);
			}
			else {
				*dat = (BYTE)(tmp | 0xf0);
			}
#else
			*dat = 0xFF;
#endif
			return TRUE;

		/* DRQ,IRQ */
		case 0xfd1f:
			*dat = (BYTE)(fdc_drqirq | 0x3f);
			/* DRQ�܂���IRQ���`�F�b�N����(�������) */
			fdc_drqirq |= (BYTE)0x20;
			/* �f�[�^�]�����Ŗ�����ΏI��(���z�̐_�a) */
			if (fdc_dataptr == NULL) {
#if defined(FDDSND)
				if (!fdc_wait && (fdc_cmdtype == 1) && (fdc_status & FDC_ST_BUSY)) {
#else
				if ((fdc_cmdtype == 1) && (fdc_status & FDC_ST_BUSY)) {
#endif
					/* IRQ On */
					fdc_drqirq |= (BYTE)0x40;
				}
#if defined(FDDSND)
				if (!fdc_wait || (fdc_cmdtype != 1) || (fdc_drqirq & 0x40)) {
					/* BUSY�t���O�𗎂Ƃ� */
					fdc_status &= ~FDC_ST_BUSY;
					fdc_access[fdc_drvreg] = FDC_ACCESS_READY;
				}
#else
				/* BUSY�t���O�𗎂Ƃ� */
				fdc_status &= ~FDC_ST_BUSY;
				fdc_access[fdc_drvreg] = FDC_ACCESS_READY;
#endif
			}
			return TRUE;
	}

	return FALSE;
}

/*
 *	FDC
 *	�P�o�C�g��������
 */
BOOL FASTCALL fdc_writeb(WORD addr, BYTE dat)
{
#if XM7_VER >= 3
	BOOL tmp;
#endif

#if XM7_VER == 1
	if (!fdc_enable && (fm_subtype != FMSUB_FM77)) {
#else
	if (!fdc_enable && (fm7_ver == 1)) {
#endif
		return FALSE;
	}

	switch (addr) {
		/* �R�}���h���W�X�^ */
		case 0xfd18:
			fdc_command = dat;
			fdc_process_cmd();
			return TRUE;

		/* �g���b�N���W�X�^ */
		case 0xfd19:
			fdc_trkreg = dat;
			/* �R�}���h���s��̃g���b�N���W�X�^���������΍� */
			/* (F-BASIC V3.3L3x/V3.4L2x VOLCOPY/SUBSET(VCOPYEB)) */
			if (((fdc_status & FDC_ST_BUSY) && (fdc_nowcnt == 0)) &&
				((fdc_cmdtype == 2) || (fdc_cmdtype == 3))) {
				fdc_process_cmd();
			}
			return TRUE;

		/* �Z�N�^���W�X�^ */
		case 0xfd1a:
			fdc_secreg = dat;
			/* �R�}���h���s��̃Z�N�^���W�X�^���������΍� */
			/* (F-BASIC V3.3L3x/V3.4L2x VOLCOPY/SUBSET(VCOPYEB)) */
			if (((fdc_status & FDC_ST_BUSY) && (fdc_nowcnt == 0)) &&
				((fdc_cmdtype == 2) || (fdc_cmdtype == 3))) {
				fdc_process_cmd();
			}
			return TRUE;

		/* �f�[�^���W�X�^ */
		case 0xfd1b:
			fdc_datareg = dat;
			/* �J�E���^���� */
			if (fdc_dataptr && (fdc_drqirq & 0x20)) {
				fdc_dataptr[fdc_nowcnt] = fdc_datareg;
				fdc_nowcnt++;
				/* DRQ,IRQ���� */
				if (fdc_nowcnt == fdc_totalcnt) {
#if defined(FDDSND)
					if (fdc_wait) {
						schedule_delevent(EVENT_FDC_L);
					}
#endif
					fdc_status &= ~FDC_ST_DRQ;
					fdc_drqirq &= (BYTE)(~0x80);
					fdc_status &= ~FDC_ST_BUSY;

					if (fdc_cmdtype == 3) {
						if (!fdc_writesec()) {
							fdc_status |= FDC_ST_WRITEFAULT;
						}
					}
					if (fdc_cmdtype == 5) {
						if (!fdc_writetrk()) {
							fdc_status |= FDC_ST_WRITEFAULT;
						}
						/* �t�H�[�}�b�g�̂��ߎg�p�����o�b�t�@���C�� */
						fdc_readbuf(fdc_drvreg);
					}
					if ((fdc_cmdtype == 3) && (fdc_command & 0x10)) {
						/* �}���`�Z�N�^���� */
						fdc_status |= FDC_ST_BUSY;
						fdc_dataptr = NULL;
						fdc_drqirq = 0x10;
						schedule_setevent(EVENT_FDC_M, 30, fdc_multi_event);
						return TRUE;
					}
					fdc_dataptr = NULL;
					mainetc_fdc();
					fdc_drqirq = 0x40;
					fdc_access[fdc_drvreg] = FDC_ACCESS_READY;
				}
				else {
#if defined(FDDSND)
					if (fdc_wait) {
						schedule_setevent(EVENT_FDC_L, FDC_LOST_TIME, fdc_drq_event);
						fdc_status &= (BYTE)~FDC_ST_DRQ;
						fdc_drqirq &= (BYTE)~0x80;
					}
					else {
						fdc_drqirq |= (BYTE)0x80;
					}
#else
					fdc_drqirq |= (BYTE)0x80;
#endif
				}
			}
			return TRUE;

		/* �w�b�h���W�X�^ */
		case 0xfd1c:
			if ((dat & 0x01) != fdc_sidereg) {
				fdc_sidereg = (BYTE)(dat & 0x01);
				fdc_readbuf(fdc_drvreg);
			}
			return TRUE;

		/* �h���C�u���W�X�^ */
		case 0xfd1d:
			/* �h���C�u�ύX�Ȃ�Afdc_readbuf */
#if XM7_VER >= 3
			if (fdc_drvregP != (dat & 0x03)) {
				fdc_drvregP = (BYTE)(dat & 0x03);
				fdc_drvreg = (BYTE)fdc_physdrv[fdc_drvregP];
				fdc_readbuf(fdc_drvreg);
			}
#else
			if (fdc_drvreg != (dat & 0x03)) {
				fdc_drvreg = (BYTE)(dat & 0x03);
				fdc_readbuf(fdc_drvreg);
			}
#endif
			fdc_motor = (BYTE)(dat & 0x80);
			/* �h���C�u�����Ȃ�A���[�^�~�߂� */
			if (fdc_drvreg >= FDC_DRIVES) {
				fdc_motor = 0;
			}
			return TRUE;

		/* ���[�h���W�X�^ */
		case 0xfd1e:
#if XM7_VER >= 3
			if (fm7_ver < 3) {
				return TRUE;
			}

			/* 2DD�؂芷�� */
			tmp = fdc_2ddmode;

			if (dat & 0x40) {
				fdc_2ddmode = FALSE;
			}
			else {
				fdc_2ddmode = TRUE;
			}

			/* �_���h���C�u�ƕ����h���C�u�̑Ή��ύX */
			fdc_logidrv = (BYTE)((dat & 0x0c) >> 2);
			if (dat & 0x10) {
				/* �������� */
				fdc_physdrv[fdc_logidrv] = (BYTE)(dat & 0x03);

				/* �J�����g�h���C�u�̐ݒ肪�ύX���ꂽ�ꍇ�̏��� */
				if (fdc_logidrv == fdc_drvregP) {
					fdc_drvreg = (BYTE)fdc_physdrv[fdc_logidrv];
					fdc_readbuf(fdc_drvreg);
					return TRUE;
				}
			}

			/* 2DD�؂芷�����s��ꂽ�ꍇ�͍ēǂݍ��� */
			if (fdc_2ddmode != tmp) {
				fdc_readbuf(fdc_drvreg);
			}
#endif
			return TRUE;
	}

	return FALSE;
}

/*
 *	FDC
 *	�Z�[�u
 */
BOOL FASTCALL fdc_save(int fileh)
{
	int i;

	/* �t�@�C���֌W���Ɏ����Ă��� */
	for (i=0; i<FDC_DRIVES; i++) {
		if (!file_byte_write(fileh, fdc_ready[i])) {
			return FALSE;
		}
	}
	for (i=0; i<FDC_DRIVES; i++) {
		if (!file_write(fileh, (BYTE*)fdc_fname[i], 256 + 1)) {
			return FALSE;
		}
	}
	for (i=0; i<FDC_DRIVES; i++) {
		if (!file_byte_write(fileh, fdc_media[i])) {
			return FALSE;
		}
	}

	/* Ver4�g�� */
	for (i=0; i<FDC_DRIVES; i++) {
		if (!file_bool_write(fileh, fdc_teject[i])) {
			return FALSE;
		}
	}

	/* �t�@�C���X�e�[�^�X */
	for (i=0; i<FDC_DRIVES; i++) {
		if (!file_byte_write(fileh, fdc_track[i])) {
			return FALSE;
		}
	}
	if (!file_write(fileh, fdc_buffer, 0x2000)) {
		return FALSE;
	}

	/* fdc_dataptr�͊��Ɉˑ�����f�[�^�|�C���^ */
	if (!fdc_dataptr) {
		if (!file_word_write(fileh, 0x2000)) {
			return FALSE;
		}
	}
	else {
		if (!file_word_write(fileh, (WORD)(fdc_dataptr - &fdc_buffer[0]))) {
			return FALSE;
		}
	}

	for (i=0; i<FDC_DRIVES; i++) {
		if (!file_dword_write(fileh, fdc_seekofs[i])) {
			return FALSE;
		}
	}
	for (i=0; i<FDC_DRIVES; i++) {
		if (!file_dword_write(fileh, fdc_secofs[i])) {
			return FALSE;
		}
	}

	/* I/O */
	if (!file_byte_write(fileh, fdc_command)) {
		return FALSE;
	}
	if (!file_byte_write(fileh, fdc_status)) {
		return FALSE;
	}
	if (!file_byte_write(fileh, fdc_trkreg)) {
		return FALSE;
	}
	if (!file_byte_write(fileh, fdc_secreg)) {
		return FALSE;
	}
	if (!file_byte_write(fileh, fdc_datareg)) {
		return FALSE;
	}
	if (!file_byte_write(fileh, fdc_sidereg)) {
		return FALSE;
	}
	if (!file_byte_write(fileh, fdc_drvreg)) {
		return FALSE;
	}
	if (!file_byte_write(fileh, fdc_motor)) {
		return FALSE;
	}
	if (!file_byte_write(fileh, fdc_drqirq)) {
		return FALSE;
	}
	if (!file_byte_write(fileh, fdc_cmdtype)) {
		return FALSE;
	}

	/* ���̑� */
	if (!file_word_write(fileh, fdc_totalcnt)) {
		return FALSE;
	}
	if (!file_word_write(fileh, fdc_nowcnt)) {
		return FALSE;
	}
	if (!file_bool_write(fileh, fdc_seekvct)) {
		return FALSE;
	}
	if (!file_byte_write(fileh, fdc_indexcnt)) {
		return FALSE;
	}
	for (i=0; i<FDC_DRIVES; i++) {
		if (!file_byte_write(fileh, fdc_access[i])) {
			return FALSE;
		}
	}
	if (!file_bool_write(fileh, TRUE)) {
		return FALSE;
	}

	/* Ver9.05/7.05�g�� */
#if defined(FDDSND)
	if (!file_bool_write(fileh, fdc_wait)) {
		return FALSE;
	}
#else
	if (!file_bool_write(fileh, FALSE)) {
		return FALSE;
	}
#endif

#if XM7_VER >= 3
	/* Ver8�g�� */
	if (!file_byte_write(fileh, fdc_2ddmode)) {
		return FALSE;
	}
	if (!file_byte_write(fileh, fdc_logidrv)) {
		return FALSE;
	}
	if (!file_byte_write(fileh, fdc_drvregP)) {
		return FALSE;
	}
	for (i=0; i<FDC_DRIVES; i++) {
		if (!file_byte_write(fileh, fdc_physdrv[i])) {
			return FALSE;
		}
	}
#endif

	if (!file_bool_write(fileh, fdc_enable)) {
		return FALSE;
	}

	return TRUE;
}

/*
 *	FDC
 *	���[�h
 */
BOOL FASTCALL fdc_load(int fileh, int ver)
{
	int i;
	BYTE ready[FDC_DRIVES];
	char fname[FDC_DRIVES][256 + 1];
	BYTE media[FDC_DRIVES];
	WORD offset;
	BOOL tmp;
	int pathlen;

	/* �o�[�W�����`�F�b�N */
	if (ver < 200) {
		return FALSE;
	}

	/* �t�@�C�����̍ő啶���������� */
#if XM7_VER >= 3
	if (((ver >= 715) && (ver <= 799)) || (ver >= 915)) {
#elif XM7_VER >= 2
	if ((ver >= 715) && (ver <= 799)) {
#else
	if ((ver >= 305) && (ver <= 499)) {
#endif
		pathlen = 256;
	}
	else {
		pathlen = 128;
	}

	/* �t�@�C���֌W���Ɏ����Ă��� */
	for (i=0; i<FDC_DRIVES; i++) {
		if (!file_byte_read(fileh, &ready[i])) {
			return FALSE;
		}
	}
	for (i=0; i<FDC_DRIVES; i++) {
		if (!file_read(fileh, (BYTE*)fname[i], pathlen + 1)) {
			return FALSE;
		}
	}
	for (i=0; i<FDC_DRIVES; i++) {
		if (!file_byte_read(fileh, &media[i])) {
			return FALSE;
		}
	}

	/* �ă}�E���g�����݂� */
	for (i=0; i<FDC_DRIVES; i++) {
		fdc_setdisk(i, NULL);
		if (ready[i] != FDC_TYPE_NOTREADY) {
			fdc_setdisk(i, fname[i]);
			if (fdc_ready[i] != FDC_TYPE_NOTREADY) {
				if (fdc_medias[i] >= (media[i] + 1)) {
					fdc_setmedia(i, media[i]);
				}
			}
		}
	}

	/* Ver4�g�� �c���ă`�F�b�N�����Ӗ����Ȃ���ł�(�` */
	for (i=0; i<FDC_DRIVES; i++) {
		if (!file_bool_read(fileh, &fdc_teject[i])) {
			return FALSE;
		}
	}

	/* �t�@�C���X�e�[�^�X */
	for (i=0; i<FDC_DRIVES; i++) {
		if (!file_byte_read(fileh, &fdc_track[i])) {
			return FALSE;
		}
	}
	if (!file_read(fileh, fdc_buffer, 0x2000)) {
		return FALSE;
	}

	/* fdc_dataptr�͊��Ɉˑ�����f�[�^�|�C���^ */
	if (!file_word_read(fileh, &offset)) {
		return FALSE;
	}
	if (offset >= 0x2000) {
		fdc_dataptr = NULL;
	}
	else {
		fdc_dataptr = &fdc_buffer[offset];
	}

	for (i=0; i<FDC_DRIVES; i++) {
		if (!file_dword_read(fileh, &fdc_seekofs[i])) {
			return FALSE;
		}
	}
	for (i=0; i<FDC_DRIVES; i++) {
		if (!file_dword_read(fileh, &fdc_secofs[i])) {
			return FALSE;
		}
	}

	/* I/O */
	if (!file_byte_read(fileh, &fdc_command)) {
		return FALSE;
	}
	if (!file_byte_read(fileh, &fdc_status)) {
		return FALSE;
	}
	if (!file_byte_read(fileh, &fdc_trkreg)) {
		return FALSE;
	}
	if (!file_byte_read(fileh, &fdc_secreg)) {
		return FALSE;
	}
	if (!file_byte_read(fileh, &fdc_datareg)) {
		return FALSE;
	}
	if (!file_byte_read(fileh, &fdc_sidereg)) {
		return FALSE;
	}
	if (!file_byte_read(fileh, &fdc_drvreg)) {
		return FALSE;
	}
	if (!file_byte_read(fileh, &fdc_motor)) {
		return FALSE;
	}
	if (!file_byte_read(fileh, &fdc_drqirq)) {
		return FALSE;
	}
	if (ver < 600) {
		fdc_drqirq |= (BYTE)0x20;
	}
	if (!file_byte_read(fileh, &fdc_cmdtype)) {
		return FALSE;
	}

	/* ���̑� */
	if (!file_word_read(fileh, &fdc_totalcnt)) {
		return FALSE;
	}
	if (!file_word_read(fileh, &fdc_nowcnt)) {
		return FALSE;
	}
	if (!file_bool_read(fileh, &fdc_seekvct)) {
		return FALSE;
	}
	if (!file_byte_read(fileh, &fdc_indexcnt)) {
		return FALSE;
	}
	for (i=0; i<FDC_DRIVES; i++) {
		if (!file_byte_read(fileh, &fdc_access[i])) {
			return FALSE;
		}
	}
	/* ���o�[�W�����Ƃ̌݊��p(�u�[�g�t���O) */
	if (!file_bool_read(fileh, &tmp)) {
		return FALSE;
	}

	/* �C�x���g */
	schedule_handle(EVENT_FDC_M, fdc_multi_event);
	schedule_handle(EVENT_FDC_L, fdc_lost_event);
#if defined(FDDSND)
	schedule_handle(EVENT_FDD_SEEK, fdd_seek_event);
#endif
#if XM7_VER >= 3
	if ((ver >= 905) || ((ver >= 705) && (ver <= 799))) {
#elif XM7_VER >= 2
	if (ver >= 705) {
#else
	if (ver >= 300) {
#endif
		/* Ver9.05/7.05�g�� */
#if defined(FDDSND)
		if (!file_bool_read(fileh, &fdc_wait)) {
			return FALSE;
		}
		/* FD�A�N�Z�X�E�F�C�g���L���ȏꍇ�̓C�x���g�n���h����ύX */
		if (fdc_wait) {
			schedule_handle(EVENT_FDC_L, fdc_drq_event);
		}
#else
		if (!file_bool_read(fileh, &tmp)) {
			return FALSE;
		}
#endif
	}
#if defined(FDDSND)
	else {
		/* �E�F�C�g���[�h��FALSE */
		fdc_wait = FALSE;

		/* �V�[�N�C�x���g(���z�b�g���Z�b�g�C�x���g)���폜 */
		schedule_delevent(EVENT_FDD_SEEK);
	}
#endif

#if XM7_VER >= 3
	/* Ver8�g�� */
	if (ver < 800) {
		fdc_2ddmode = FALSE;
		fdc_logidrv = 0;
		fdc_drvregP = fdc_drvreg;
		/* �_���h���C�u�������h���C�u�ɐݒ� */
		for (i=0; i<FDC_DRIVES; i++) {
			fdc_physdrv[i] = (BYTE)i;
		}
	}
	else {
		if (!file_byte_read(fileh, &fdc_2ddmode)) {
			return FALSE;
		}
		if (!file_byte_read(fileh, &fdc_logidrv)) {
			return FALSE;
		}
		if (!file_byte_read(fileh, &fdc_drvregP)) {
			return FALSE;
		}
		for (i=0; i<FDC_DRIVES; i++) {
			if (!file_byte_read(fileh, &fdc_physdrv[i])) {
				return FALSE;
			}
		}
	}
#endif

#if XM7_VER >= 3
	if (((ver >= 720) && (ver <= 799)) || (ver >= 920)) {
#elif XM7_VER >= 2
	if ((ver >= 720) && (ver <= 799)) {
#else
	if ((ver >= 310) && (ver <= 499)) {
#endif
		if (!file_bool_read(fileh, &fdc_enable)) {
			return FALSE;
		}
	}
	else {
		fdc_enable = TRUE;
	}

	return TRUE;
}
