/*
 *	FM-7 EMULATOR "XM7"
 *
 *	Copyright (C) 1999-2017 �o�h�D(Twitter:@xm6_original)
 *	Copyright (C) 2001-2017 Ryu Takegami (Twitter:@RyuTakegami)
 *
 *	[ �o�u�������� �R���g���[�� (32KB��p��) ]
 */

#if XM7_VER == 1 
#if defined(BUBBLE) || (!defined(BUBBLE) && defined(XM7PURE))

#include <string.h>
#include <stdlib.h>
#include "xm7.h"
#include "device.h"
#include "bubble.h"
#include "fdc.h"
#include "opn.h"
#include "event.h"

#if !defined(XM7PURE) || defined(BUBBLE)
/*
 *	�O���[�o�� ���[�N
 */

/* ���W�X�^ */
BYTE bmc_datareg;						/* �f�[�^���W�X�^ */
BYTE bmc_command;						/* �R�}���h���W�X�^ */
BYTE bmc_status;						/* �X�e�[�^�X���W�X�^ */
BYTE bmc_errorreg;						/* �G���[�X�e�[�^�X���W�X�^ */
WORD bmc_pagereg;						/* �y�[�W���W�X�^ */
WORD bmc_countreg;						/* �y�[�W�J�E���g���W�X�^ */

/* �O�����[�N */
WORD bmc_totalcnt;						/* �g�[�^���J�E���^ */
WORD bmc_nowcnt;						/* �J�����g�J�E���^ */
BYTE bmc_unit;							/* ���j�b�g */
BYTE bmc_ready[BMC_UNITS_32];			/* ���f�B��� */
BOOL bmc_teject[BMC_UNITS_32];			/* �ꎞ�C�W�F�N�g */
BOOL bmc_writep[BMC_UNITS_32];			/* �������݋֎~��� */

char bmc_fname[BMC_UNITS_32][256+1];	/* �t�@�C���� */
char bmc_name[BMC_UNITS_32][BMC_MEDIAS][17];	/* �C���[�W�� */
BOOL bmc_fwritep[BMC_UNITS_32];			/* ���C�g�v���e�N�g��� */
BYTE bmc_header[BMC_UNITS_32][0x20];	/* B77�t�@�C���w�b�_ */
BYTE bmc_medias[BMC_UNITS_32];			/* ���f�B�A���� */
BYTE bmc_media[BMC_UNITS_32];			/* ���f�B�A�Z���N�g��� */
BYTE bmc_access[BMC_UNITS_32];			/* �A�N�Z�XLED */

BOOL bmc_enable;						/* �L���E�����t���O */
BOOL bmc_use;							/* �g�p�t���O */

/*
 *	�X�^�e�B�b�N ���[�N
 */
static BYTE bmc_buffer[BMC_PSIZE_32];	/* 32byte�f�[�^�o�b�t�@ */
static BYTE *bmc_dataptr;				/* �f�[�^�|�C���^ */
static DWORD bmc_offset;				/* �I�t�Z�b�g */
static DWORD bmc_foffset[BMC_UNITS_32][BMC_MEDIAS];
static DWORD bmc_fsize[BMC_UNITS_32];
#if defined(FDDSND)
static BOOL bmc_wait;					/* �E�F�C�g���[�h���s�t���O */
#endif

/*
 *
 */
static void FASTCALL bmc_make_stat(void);		/* �X�e�[�^�X�쐬 */
static BOOL FASTCALL bmc_lost_event(void);		/* ���X�g�f�[�^�C�x���g */
static void FASTCALL bmc_read_data(void);		/* �y�[�W�ǂݍ��݊J�n */
static void FASTCALL bmc_write_data(void);		/* �y�[�W���ݍ��݊J�n */


/*
 *	�o�u�������� �R���g���[��
 *	������
 */
BOOL FASTCALL bmc_init(void)
{
	/* �t�@�C���֌W�����Z�b�g */
	bmc_totalcnt = 0;
	bmc_nowcnt = 0;

	memset(bmc_ready, BMC_TYPE_NOTREADY, sizeof(bmc_ready));
	memset(bmc_teject, FALSE, sizeof(bmc_teject));
	memset(bmc_writep, FALSE, sizeof(bmc_writep));
	memset(bmc_fname, 0, sizeof(bmc_fname));
	memset(bmc_fwritep, FALSE, sizeof(bmc_fwritep));
	memset(bmc_medias, 0, sizeof(bmc_medias));

	/* �t�@�C���I�t�Z�b�g��S�ăN���A */
	memset(bmc_foffset, 0, sizeof(bmc_foffset));

	/* �f�t�H���g�͖��� */
	bmc_enable = FALSE;

	/* �E�F�C�g�}�����[�h�t���O������ */
#if defined(FDDSND)
	bmc_wait = FALSE;
#endif

	return TRUE;
}

/*
 *	�o�u�������� �R���g���[��
 *	�N���[���A�b�v
 */
void FASTCALL bmc_cleanup(void)
{
	/* �t�@�C���֌W�����Z�b�g */
	memset(bmc_ready, 0, sizeof(bmc_ready));
}

/*
 *	�o�u�������� �R���g���[��
 *	���Z�b�g
 */
void FASTCALL bmc_reset(void)
{
	/* �������W�X�^�����Z�b�g */
	bmc_datareg = 0;
	bmc_command = 0;
	bmc_status = BMC_ST_NONE;
	bmc_errorreg = 0;
	bmc_pagereg = 0;
	bmc_countreg = 0;

	memset(bmc_access, 0, sizeof(bmc_access));
	bmc_dataptr = 0;
	bmc_offset = 0;

	bmc_use = FALSE;

	/* �X�e�[�^�X�쐬 */
	bmc_make_stat();
}

/*-[ �t�@�C���Ǘ� ]---------------------------------------------------------*/

/*
 *	�y�[�W�������ݏI��
 */
static BOOL FASTCALL bmc_write_page(void)
{
	DWORD offset;
	int handle;

	/* assert */
	ASSERT(bmc_ready[bmc_unit] != BMC_TYPE_NOTREADY);
	ASSERT(bmc_dataptr);
	ASSERT(bmc_totalcnt > 0);

	/* �I�t�Z�b�g�Z�o */
	offset = (DWORD)((bmc_pagereg & BMC_MAXADDR_32) * BMC_PSIZE_32);
	if (bmc_ready[bmc_unit] == BMC_TYPE_B77) {
		if (bmc_fsize[bmc_unit] < offset + BMC_PSIZE_32 + 0x0020) {
			return FALSE;
		}
		offset += *(DWORD *)(&bmc_header[bmc_unit][0x0014]);
	}

	/* �������� */
	handle = file_open(bmc_fname[bmc_unit], OPEN_RW);
	if (handle == -1) {
		return FALSE;
	}
	if (!file_seek(handle, offset)) {
		file_close(handle);
		return FALSE;
	}
	if (!file_write(handle, bmc_dataptr, bmc_totalcnt)) {
		file_close(handle);
		return FALSE;
	}
	file_close(handle);

	return TRUE;
}

/*
 *	�y�[�W�ǂݍ���
 */
static BOOL FASTCALL bmc_readbuf(void)
{
	int handle;
	DWORD offset;

	/* �y�[�W�A�h���X�`�F�b�N */
	if (bmc_unit >= BMC_UNITS_32) {
		memset(bmc_buffer, 0, BMC_PSIZE_32);
		return FALSE;
	}

	/* ���f�B�`�F�b�N */
	if (bmc_ready[bmc_unit] == BMC_TYPE_NOTREADY) {
		return FALSE;
	}

	/* �I�t�Z�b�g�Z�o */
	offset = (DWORD)(bmc_pagereg & BMC_MAXADDR_32);
	offset *= BMC_PSIZE_32;
	if (bmc_ready[bmc_unit] == BMC_TYPE_B77) {
		if (bmc_fsize[bmc_unit] < offset + BMC_PSIZE_32 + 0x0020) {
			return FALSE;
		}
		offset += *(DWORD *)(&bmc_header[bmc_unit][0x0014]);
	}
	bmc_offset = offset;

	/* �ǂݍ��� */
	memset(bmc_buffer, 0, BMC_PSIZE_32);
	handle = file_open(bmc_fname[bmc_unit], OPEN_R);
	if (handle == -1) {
		return FALSE;
	}
	if (!file_seek(handle, offset)) {
		file_close(handle);
		return FALSE;
	}
	file_read(handle, bmc_buffer, BMC_PSIZE_32);
	file_close(handle);
	return TRUE;
}

/*
 *	B77�t�@�C�� �w�b�_�ǂݍ���
 */
static BOOL FASTCALL bmc_readhead(int unit, int index)
{
	DWORD offset;
	DWORD temp;
	int handle;

	/* assert */
	ASSERT((unit >= 0) && (unit < BMC_UNITS_32));
	ASSERT((index >= 0) && (index < BMC_MEDIAS));
	ASSERT(bmc_ready[unit] == BMC_TYPE_B77);

	/* �I�t�Z�b�g���� */
	offset = bmc_foffset[unit][index];

	/* �V�[�N�A�ǂݍ��� */
	handle = file_open(bmc_fname[unit], OPEN_R);
	if (handle == -1) {
		return FALSE;
	}
	if (!file_seek(handle, offset)) {
		file_close(handle);
		return FALSE;
	}
	if (!file_read(handle, bmc_header[unit], 0x20)) {
		file_close(handle);
		return FALSE;
	}
	file_close(handle);

	/* �J�Z�b�g�T�C�Y */
	temp = 0;
	temp |= bmc_header[unit][0x001c + 3];
	temp *= 256;
	temp |= bmc_header[unit][0x001c + 2];
	temp *= 256;
	temp |= bmc_header[unit][0x001c + 1];
	temp *= 256;
	temp |= bmc_header[unit][0x001c + 0];
	bmc_fsize[unit] = temp;

	/* �^�C�v�`�F�b�N */
	if (bmc_header[unit][0x001b] != 0x80) {
		/* 32KB�łȂ� */
		return FALSE;
	}

	/* ���C�g�v���e�N�g�ݒ� */
	if (bmc_fwritep[unit]) {
		bmc_writep[unit] = TRUE;
	}
	else {
		if (bmc_header[unit][0x001a] & 0x10) {
			bmc_writep[unit] = TRUE;
		}
		else {
			bmc_writep[unit] = FALSE;
		}
	}

	/* �I�t�Z�b�g */
	*(DWORD *)(&bmc_header[unit][0x0014]) = offset + 0x0020;

	return TRUE;
}

/*
 *	���݂̃��f�B�A�̃��C�g�v���e�N�g��؂�ւ���
 */
BOOL FASTCALL bmc_setwritep(int unit, BOOL writep)
{
	BYTE header[0x2b0];
	DWORD offset;
	int handle;

	/* assert */
	ASSERT((unit >= 0) && (unit < 2));
	ASSERT((writep == TRUE) || (writep == FALSE));

	/* ���f�B�łȂ���΂Ȃ�Ȃ� */
	if (bmc_ready[unit] == BMC_TYPE_NOTREADY) {
		return FALSE;
	}

	/* �t�@�C�����������ݕs�Ȃ�_�� */
	if (bmc_fwritep[unit]) {
		return FALSE;
	}

	if (bmc_ready[unit] == BMC_TYPE_B77) {
		offset = bmc_foffset[unit][bmc_media[unit]];
		handle = file_open(bmc_fname[unit], OPEN_RW);
		if (handle == -1) {
			return FALSE;
		}
		if (!file_seek(handle, offset)) {
			file_close(handle);
			return FALSE;
		}
		if (!file_read(handle, header, 0x20)) {
			file_close(handle);
			return FALSE;
		}
		if (writep) {
			header[0x001a] |= 0x10;
		}
		else {
			header[0x001a] &= ~0x10;
		}
		if (!file_seek(handle, offset)) {
			file_close(handle);
			return FALSE;
		}
		if (!file_write(handle, header, 0x20)) {
			file_close(handle);
			return FALSE;
		}

		file_close(handle);
	}

	/* ���� */
	bmc_writep[unit] = writep;

	return TRUE;
}


/*
 *	���f�B�A�ԍ���ݒ�
 */
BOOL FASTCALL bmc_setmedia(int unit, int index)
{
	/* assert */
	ASSERT((unit >= 0) && (unit <= 1));
	ASSERT((index >= 0) && (index < BMC_MEDIAS));

	/* ���f�B��Ԃ� */
	if (bmc_ready[unit] == BMC_TYPE_NOTREADY) {
		return FALSE;
	}

	/* 32KB�t�@�C���̏ꍇ�Aindex = 0�� */
	if ((bmc_ready[unit] == BMC_TYPE_32) && (index != 0)) {
		return FALSE;
	}

	/* index > 0 �Ȃ�Abmc_foffset�𒲂ׂ�>0���K�v */
	if (index > 0) {
		if (bmc_foffset[unit][index] == 0) {
			return FALSE;
		}
	}

	/* B77�t�@�C���̏ꍇ�A�w�b�_�ǂݍ��� */
	if (bmc_ready[unit] == BMC_TYPE_B77) {
		/* ���C�g�v���e�N�g�͓����Őݒ� */
		if (!bmc_readhead(unit, index)) {
			return FALSE;
		}
	}
	else {
		/* 32KB�t�@�C���Ȃ�A�t�@�C�������ɏ]�� */
		bmc_writep[unit] = bmc_fwritep[unit];
	}

	/* ���f�B�A���������ꂽ�ꍇ�A�ꎞ�C�W�F�N�g���������� */
	if (bmc_media[unit] != (BYTE)index) {
		bmc_media[unit] = (BYTE)index;
		bmc_teject[unit] = FALSE;
	}

	bmc_make_stat();

	return TRUE;
}

/*
 *	B77�t�@�C����́A���f�B�A������і��̎擾
 */
static int FASTCALL bmc_chkb77(int unit)
{
	int i;
	int handle;
	int count;
	DWORD offset;
	DWORD len;
	BYTE buf[0x20];

	/* ������ */
	for (i=0; i<BMC_MEDIAS; i++) {
		bmc_foffset[unit][i] = 0;
		bmc_name[unit][i][0] = '\0';
	}
	count = 0;
	offset = 0;

	/* �t�@�C���I�[�v�� */
	handle = file_open(bmc_fname[unit], OPEN_R);
	if (handle == -1) {
		return count;
	}

	/* ���f�B�A���[�v */
	while (count < BMC_MEDIAS) {
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

		/* �^�C�v�`�F�b�N�B32KB�̂ݑΉ� */
		if (buf[0x001b] != 0x80) {
			file_close(handle);
			return count;
		}

		/* ok,�t�@�C�����A�I�t�Z�b�g�i�[ */
		buf[17] = '\0';
		memcpy(bmc_name[unit][count], buf, 17);
		bmc_foffset[unit][count] = offset;

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
 *	�t�@�C����ݒ�
 */
int FASTCALL bmc_setfile(int unit, char *fname)
{
	BOOL writep;
	int handle;
	DWORD fsize;
	int count;

	ASSERT((unit >= 0) && (unit < 2));

	/* �m�b�g���f�B�ɂ���ꍇ */
	if (fname == NULL) {
		bmc_ready[unit] = BMC_TYPE_NOTREADY;
		bmc_fname[unit][0] = '\0';
		bmc_make_stat();
		return 1;
	}

	/* �t�@�C�����I�[�v�����A�t�@�C���T�C�Y�𒲂ׂ� */
	if (strlen(fname) >= sizeof(bmc_fname[unit])) {
		bmc_ready[unit] = BMC_TYPE_NOTREADY;
		bmc_fname[unit][0] = '\0';
		bmc_make_stat();
		return 1;
	}
	writep = FALSE;
	handle = file_open(fname, OPEN_RW);
	if (handle == -1) {
		handle = file_open(fname, OPEN_R);
		if (handle == -1) {
			bmc_ready[unit] = BMC_TYPE_NOTREADY;
			bmc_make_stat();
			return 0;
		}
		writep = TRUE;
	}
	strcpy(bmc_fname[unit], fname);
	fsize = file_getsize(handle);
	file_close(handle);

	/*
	 * 32KB�t�@�C��
	 */
	if (fsize == 32768) {
		/* �^�C�v�A�������ݑ����ݒ� */
		bmc_ready[unit] = BMC_TYPE_32;
		bmc_fwritep[unit] = writep;

		/* ���f�B�A�ݒ� */
		if (!bmc_setmedia(unit, 0)) {
			bmc_ready[unit] = BMC_TYPE_NOTREADY;
			bmc_fname[unit][0] = '\0';
			bmc_make_stat();
			return 0;
		}

		/* �����B�ꎞ�C�W�F�N�g���� */
		bmc_teject[unit] = FALSE;
		bmc_medias[unit] = 1;
		bmc_make_stat();
		return 1;
	}

	/*
	 * B77�t�@�C��
	 */
	bmc_ready[unit] = BMC_TYPE_B77;
	bmc_fwritep[unit] = writep;

	/* �t�@�C������ */
	count = bmc_chkb77(unit);
	if (count != 0){
		/* ���f�B�A�ݒ� */
		if (bmc_setmedia(unit, 0)) {
			/* �����B�ꎞ�C�W�F�N�g���� */
			bmc_teject[unit] = FALSE;
			bmc_medias[unit] = (BYTE)count;
			bmc_make_stat();
			return count;
		}
	}

	bmc_ready[unit] = BMC_TYPE_NOTREADY;
	bmc_fname[unit][0] = '\0';
	bmc_make_stat();
	return 0;
}

/*-[ BMC�R�}���h �E�F�C�g���[�h���� ]---------------------------------------*/

#ifdef FDDSND
/*
 *	BMC DataRequest�C�x���g (WAIT)
 *	�C�x���g
 */
static BOOL FASTCALL bmc_drq_event(void)
{
	/* ���X�g�f�[�^�`�F�b�N */
	if (bmc_status & (BYTE)(BMC_ST_RDA | BMC_ST_TDRA)) {
		return bmc_lost_event();
	}

	/* ���� */
	switch ((BYTE)(bmc_command & 0x0f)) {
		/* bubble read */
		case 0x01:
			bmc_status |= (BYTE)BMC_ST_RDA;
			break;
		/* bubble write */
		case 0x02:
			bmc_status |= (BYTE)BMC_ST_TDRA;
			break;
	}

	return TRUE;
}
#endif

/*
 *	�}���`�y�[�W
 *	�C�x���g
 */
static BOOL FASTCALL bmc_multi_event(void)
{
	/* �y�[�W���W�X�^���C���N�������g */
	bmc_pagereg++;

	switch ((BYTE)(bmc_command & 0x0f)) {
		/* bubble read */
		case 0x01:
			bmc_read_data();
			bmc_nowcnt = 0;
			bmc_status |= (BYTE)BMC_ST_RDA;
			break;
		/* bubble write */
		case 0x02:
			bmc_write_data();
			bmc_nowcnt = 0;
			bmc_status |= (BYTE)BMC_ST_TDRA;
			break;
	}

	/* �C�x���g�폜���ďI�� */
	schedule_delevent(EVENT_BMC_MULTI);

	return TRUE;
}

/*
 *	���X�g�f�[�^
 *	�C�x���g
 */
static BOOL FASTCALL bmc_lost_event(void)
{
	if ((bmc_command & 0x0f) == 0x04) {
		/* �R�}���h�ł��؂�A���X�g�f�[�^ */
		bmc_dataptr = NULL;
		bmc_errorreg = 0;
		bmc_make_stat();
		bmc_status &= (BYTE)(~BMC_ST_BUSY);
		bmc_status |= (BYTE)(BMC_ST_CME);
		bmc_access[0] = (BYTE)BMC_ACCESS_READY;
		bmc_access[1] = (BYTE)BMC_ACCESS_READY;
	}
	if (bmc_dataptr) {
		/* �R�}���h�ł��؂�A���X�g�f�[�^ */
		bmc_dataptr = NULL;
		bmc_errorreg = (BYTE)BMC_ES_PAGEOVER;
		bmc_make_stat();
		bmc_status &= (BYTE)(~BMC_ST_BUSY);
		bmc_status |= (BYTE)(BMC_ST_CME | BMC_ST_ERROR);
		bmc_access[bmc_unit] = (BYTE)BMC_ACCESS_READY;
	}

	/* �C�x���g�폜���ďI�� */
	schedule_delevent(EVENT_BMC_LOST);

	return TRUE;
}

/*-[ BMC�R�}���h ]----------------------------------------------------------*/

/*
 *	�X�e�[�^�X�쐬
 */
static void FASTCALL bmc_make_stat(void)
{
	/* �L���ȃ��j�b�g */
	if (bmc_ready[bmc_unit] == BMC_TYPE_NOTREADY) {
		bmc_status &= (BYTE)(~BMC_ST_READY);
	}
	else {
		bmc_status |= (BYTE)BMC_ST_READY;
	}

	/* �ꎞ�C�W�F�N�g */
	if (bmc_teject[bmc_unit]) {
		bmc_status &= (BYTE)(~BMC_ST_READY);
	}

	/* ���C�g�v���e�N�g */
	if (bmc_writep[bmc_unit]) {
		bmc_status |= (BYTE)BMC_ST_WRITEP;
	}
	else {
		bmc_status &= (BYTE)(~BMC_ST_WRITEP);
	}
}

/*
 *	�o�u�������� �R���g���[��������
 */
static void FASTCALL bmc_initialize(void)
{
	/* ������ */
	bmc_status = (BYTE)BMC_ST_NONE;
	bmc_errorreg = (BYTE)0;

	/* �����A�b�v�f�[�g(�f�[�^���W�X�^��1�ɂ���) */
	bmc_datareg = (BYTE)1;
	bmc_pagereg = 0;
	bmc_countreg = 0;
	bmc_totalcnt = 0;
	bmc_nowcnt = 0;

#ifdef FDDSND
	if (bmc_wait) {
		schedule_setevent(EVENT_BMC_LOST, BMC_LOST_TIME * 100, bmc_lost_event);
	}
#endif

	/* �X�e�[�^�X��ݒ肷�� */
	bmc_status |= (BYTE)BMC_ST_BUSY;
	bmc_make_stat();

	/* �A�N�Z�X(READY) */
	if (bmc_ready[0] != BMC_TYPE_NOTREADY) {
		bmc_access[0] = (BYTE)BMC_ACCESS_READ;
	}
	if (bmc_ready[1] != BMC_TYPE_NOTREADY) {
		bmc_access[1] = (BYTE)BMC_ACCESS_READ;
	}
}

/*
 *	�o�u�������� �R���g���[������������
 */
static void FASTCALL bmc_force_initialize(void)
{
	/* ������ */
#ifdef FDDSND
	schedule_delevent(EVENT_BMC_LOST);
	schedule_delevent(EVENT_BMC_MULTI);
#endif
	bmc_datareg = (BYTE)0;
	bmc_status = (BYTE)BMC_ST_NONE;
	bmc_errorreg = (BYTE)0;

	/* �X�e�[�^�X��ݒ肷�� */
	bmc_make_stat();

	/* �A�N�Z�X(READY) */
	bmc_access[bmc_unit] = (BYTE)BMC_ACCESS_READY;
}

/*
 *	READ/WRITE �T�u
 */
static BOOL FASTCALL bmc_rw_sub(void)
{
	bmc_status = (BYTE)BMC_ST_NONE;
	bmc_errorreg = (BYTE)0;

	/* �y�[�W�A�h���X�`�F�b�N */
	if ((bmc_unit >= BMC_UNITS_32) ||
		(bmc_pagereg >= (BMC_MAXADDR_32 + 1) * 2)) {
		bmc_status |= (BYTE)(BMC_ST_CME | BMC_ST_ERROR);
		bmc_errorreg |= (BYTE)BMC_ES_PAGEOVER;
		bmc_make_stat();
		return FALSE;
	}

	/* NOT READY�`�F�b�N */
	if ((bmc_ready[bmc_unit] == BMC_TYPE_NOTREADY) ||
		bmc_teject[bmc_unit]) {
		bmc_status |= (BYTE)(BMC_ST_CME | BMC_ST_ERROR);
		bmc_errorreg |= (BYTE)BMC_ES_EJECT;
		bmc_make_stat();
		return FALSE;
	}

	return TRUE;
}

/*
 *	�y�[�W�ǂݍ��݊J�n
 */
static void FASTCALL bmc_read_data(void)
{
	/* ��{�`�F�b�N */
	if (!bmc_rw_sub()) {
		return;
	}

	/* �f�[�^�|�C���^�A�J�E���^�ݒ� */
	bmc_dataptr = bmc_buffer;
	bmc_offset = 0;
	bmc_totalcnt = BMC_PSIZE_32;
	bmc_nowcnt = 0;

	/* �f�[�^�o�b�t�@�ǂݍ��� */
	if (!bmc_readbuf()) {
		bmc_status |= (BYTE)(BMC_ST_CME | BMC_ST_ERROR);
		bmc_errorreg |= (BYTE)BMC_ES_NOMAKER;
		bmc_access[bmc_unit] = (BYTE)BMC_ACCESS_READ;
		return;
	}

	/* �ŏ��̃f�[�^��ݒ� */
	bmc_status |= (BYTE)BMC_ST_BUSY;
	bmc_datareg = bmc_dataptr[0];
#ifdef FDDSND
	if (bmc_wait) {
		schedule_setevent(EVENT_BMC_LOST, BMC_LOST_TIME * 48, bmc_drq_event);
	}
	else {
		bmc_status |= (BYTE)BMC_ST_RDA;
	}
#else
	bmc_status |= (BYTE)BMC_ST_RDA;
#endif

	/* �A�N�Z�X(READ) */
	bmc_access[bmc_unit] = (BYTE)BMC_ACCESS_READ;

	/* ���X�g�f�[�^�C�x���g�ݒ� */
#ifdef FDDSND
	if (!bmc_wait) {
		schedule_setevent(EVENT_BMC_LOST, 30 * 1000, bmc_lost_event);
	}
#else
	schedule_setevent(EVENT_BMC_LOST, 30 * 1000, bmc_lost_event);
#endif
}

/*
 *	�y�[�W���ݍ��݊J�n
 */
static void FASTCALL bmc_write_data(void)
{
	/* ��{�`�F�b�N */
	if (!bmc_rw_sub()) {
		return;
	}

	/* �f�[�^�|�C���^�A�J�E���^�ݒ� */
	bmc_dataptr = bmc_buffer;
	bmc_offset = 0;
	bmc_totalcnt = BMC_PSIZE_32;
	bmc_nowcnt = 0;

	/* WRITE PROTECT�`�F�b�N */
	if (bmc_writep[bmc_unit] != 0) {
		bmc_status |= (BYTE)(BMC_ST_CME | BMC_ST_ERROR);
		bmc_make_stat();
		return;
	}

	/* �X�e�[�^�X�ݒ� */
	bmc_status |= (BYTE)BMC_ST_BUSY;
#ifdef FDDSND
	if (bmc_wait) {
		schedule_setevent(EVENT_BMC_LOST, BMC_LOST_TIME * 48, bmc_drq_event);
	}
	else {
		bmc_status |= (BYTE)BMC_ST_TDRA;
	}
#else
	bmc_status |= (BYTE)BMC_ST_TDRA;
#endif

	/* �A�N�Z�X(WRITE) */
	bmc_access[bmc_unit] = (BYTE)BMC_ACCESS_WRITE;

	/* ���X�g�f�[�^�C�x���g�ݒ� */
#ifdef FDDSND
	if (!bmc_wait) {
		schedule_setevent(EVENT_BMC_LOST, 30 * 1000, bmc_lost_event);
	}
#else
	schedule_setevent(EVENT_BMC_LOST, 30 * 1000, bmc_lost_event);
#endif
}

/*
 *	�R�}���h����
 */
static void FASTCALL bmc_process_cmd(void)
{
	BYTE low;

	low = (BYTE)(bmc_command & 0x0f);

	/* BUSY����force initialize�ȊO�Ȃ�� */
	if ((bmc_status & BMC_ST_BUSY) && (low != 0x0f)) {
		return;
	}

#ifdef FDDSND
	/* �E�F�C�g�t���O�ݒ� */
	bmc_wait = fdc_waitmode;
#endif

	/* �f�[�^�]�������s���Ă���΁A�����~�߂� */
	bmc_dataptr = NULL;

	/* �R�}���h���s���Ƀy�[�W���烆�j�b�g���Z�o */
	bmc_unit = (BYTE)(bmc_pagereg >> 10);

	/* ���� */
	switch (low) {
		/* bubble read */
		case 0x01:
			bmc_read_data();
			break;
		/* bubble write */
		case 0x02:
			bmc_write_data();
			break;
		/* initialize */
		case 0x04:
			bmc_initialize();
			break;
		/* ? */
		case 0x00:
		case 0x07:
		case 0x08:
			/* �X�e�[�^�X��ݒ肷�� */
			bmc_status = (BYTE)(BMC_ST_CME | BMC_ST_NONE);
			bmc_errorreg = (BYTE)0;
			bmc_make_stat();
			break;
		/* (bubble read/write?) */
		case 0x09:
		case 0x0a:
		case 0x0b:
			/* �X�e�[�^�X��ݒ肷�� */
			bmc_status = (BYTE)(BMC_ST_CME | BMC_ST_NONE | BMC_ST_ERROR);
			bmc_errorreg = (BYTE)BMC_ES_PAGEOVER;
			break;
		/* ? */
		case 0x0c:
			/* �X�e�[�^�X��ݒ肷�� */
			bmc_datareg = 0;
			bmc_status = (BYTE)(BMC_ST_TDRA | BMC_ST_NONE | BMC_ST_BUSY);
			bmc_errorreg = (BYTE)0;
			bmc_make_stat();
			break;
		/* force initialize */
		case 0x0f:
			bmc_force_initialize();
			break;
		/* undefined */
		default:
			/* �X�e�[�^�X��ݒ肷�� */
			bmc_status = (BYTE)(BMC_ST_CME | BMC_ST_NONE | BMC_ST_ERROR);
			bmc_errorreg = (BYTE)BMC_ES_UNDEF;
			break;
	}

	/* READY�Ȃ烌�W�X�^������ */
	if (!(bmc_status & BMC_ST_BUSY)) {
		bmc_command &= (BYTE)0xf0;
		bmc_pagereg = 0;
		bmc_countreg = 0;
	}
}

/*
 *	�o�u�������� �R���g���[��
 *	�P�o�C�g�ǂݏo��
 */
BOOL FASTCALL bmc_readb(WORD addr, BYTE *dat)
{
	/* �L���E�����`�F�b�N */
	if (!bmc_enable) {
		return FALSE;
	}

	/* FM-8���[�h������ */
	if (fm_subtype != FMSUB_FM8) {
		return FALSE;
	}

	/* �A�h���X�`�F�b�N */
	if ((addr & 0xfff8) != 0xfd10) {
		return FALSE;
	}

	bmc_use = TRUE;

	switch (addr) {
		/* �f�[�^���W�X�^(BDATA) */
		case 0xfd10:
			*dat = bmc_datareg;
			/* �J�E���^���� */
			if (bmc_dataptr) {
				bmc_nowcnt++;
				if (bmc_nowcnt == bmc_totalcnt) {
#ifdef FDDSND
					if (bmc_wait) {
						schedule_delevent(EVENT_BMC_LOST);
					}
#endif
					bmc_status &= (BYTE)(~BMC_ST_BUSY);
					bmc_status &= (BYTE)(~BMC_ST_RDA);
					bmc_status |= (BYTE)BMC_ST_CME;

					bmc_countreg--;
					if ((bmc_countreg > 0) &&
						((bmc_pagereg & BMC_MAXADDR_32) < BMC_MAXADDR_32)) {
						/* �}���`�y�[�W���� */
						bmc_status |= (BYTE)BMC_ST_BUSY;
						bmc_status &= (BYTE)(~BMC_ST_CME);
						bmc_dataptr = NULL;
						schedule_setevent(EVENT_BMC_MULTI, 30, bmc_multi_event);

						return TRUE;
					}
					/* �V���O���y�[�W���� or �}���`�y�[�W�I������ */
					bmc_dataptr = NULL;

					/* �A�N�Z�X(READY) */
					bmc_access[bmc_unit] = (BYTE)BMC_ACCESS_READY;

					/* ���W�X�^������ */
					bmc_command &= (BYTE)0xf0;
					bmc_pagereg = 0;
					bmc_countreg = 0;
				}
				else {
					bmc_datareg = bmc_dataptr[bmc_nowcnt];
#ifdef FDDSND
					if (bmc_wait) {
						schedule_setevent(EVENT_BMC_LOST, BMC_LOST_TIME, bmc_drq_event);
						bmc_status &= (BYTE)(~BMC_ST_RDA);
					}
					else {
						bmc_status |= (BYTE)BMC_ST_RDA;
					}
#else
					bmc_status |= (BYTE)BMC_ST_RDA;
#endif
				}
			}
			return TRUE;

		/* �R�}���h���W�X�^(BCMD) */
		case 0xfd11:
			*dat = bmc_command;
			return TRUE;

		/* �X�e�[�^�X���W�X�^(BSTAT) */
		case 0xfd12:
			bmc_make_stat();
			*dat = bmc_status;
#ifdef FDDSND
			if (bmc_wait) {
				return TRUE;
			}
#endif
			/* BUSY���� */
			if ((bmc_status & BMC_ST_BUSY) && (bmc_dataptr == NULL)) {
				/* BUSY�t���O�𗎂Ƃ� */
				bmc_status &= (BYTE)(~BMC_ST_BUSY);
				bmc_access[bmc_unit] = (BYTE)BMC_ACCESS_READY;
			}
			/* CAN READ���� */
			if (((bmc_command & 0x0f) == 0x01) && (bmc_dataptr) &&
				!(bmc_status & BMC_ST_RDA)) {
				/* CAN READ�t���O�𗧂Ă� */
				bmc_status |= (BYTE)BMC_ST_RDA;
			}
			/* CAN WRITE���� */
			if (((bmc_command & 0x0f) == 0x02) && (bmc_dataptr) &&
				!(bmc_status & BMC_ST_TDRA)) {
				/* CAN WRITE�t���O�𗧂Ă� */
				bmc_status |= (BYTE)BMC_ST_TDRA;
			}
			/* initialize�㏈�� */
			if ((bmc_command & 0x0f) == 0x04) {
				/* BUSY�t���O�𗎂Ƃ� */
				bmc_status |= (BYTE)(BMC_ST_CME);
				if (bmc_ready[0] == BMC_ACCESS_READ) {
					bmc_access[0] = (BYTE)BMC_ACCESS_READY;
				}
				if (bmc_ready[1] == BMC_ACCESS_READ) {
					bmc_access[1] = (BYTE)BMC_ACCESS_READY;
				}
			}
			return TRUE;

		/* �G���[�X�e�[�^�X���W�X�^(BERRST) */
		case 0xfd13:
			*dat = bmc_errorreg;

			/* NOT BUSY�Ȃ�� */
			if (!(bmc_status & BMC_ST_BUSY)) {
				/* �G���[�X�e�[�^�X�𗎂Ƃ� */
				bmc_errorreg = 0;

				/* ERROR ANALYSIS�t���O�𗎂Ƃ� */
				bmc_status &= (BYTE)(~BMC_ST_ERROR);

				/* COMMAND END�t���O�𗎂Ƃ� */
				bmc_status &= (BYTE)(~BMC_ST_CME);
			}
			return TRUE;

		/* �y�[�W�A�h���X���W�X�^H(BPGADH) */
		case 0xfd14:
			*dat = (BYTE)(bmc_pagereg >> 8);
			return TRUE;

		/* �y�[�W�A�h���X���W�X�^L(BPGADL) */
		case 0xfd15:
			*dat = (BYTE)(bmc_pagereg & 0x00ff);
			return TRUE;

		/* �y�[�W�J�E���g���W�X�^H(BPGCTH) */
		case 0xfd16:
			*dat = (BYTE)(bmc_countreg >> 8);
			return TRUE;

		/* �y�[�W�J�E���g���W�X�^L(BPGCTL) */
		case 0xfd17:
			*dat = (BYTE)(bmc_countreg & 0x00ff);
			return TRUE;
	}

	/* 0��Ԃ� */
	*dat = 0;
	return TRUE;
}

/*
 *	�o�u�������� �R���g���[��
 *	�P�o�C�g��������
 */
BOOL FASTCALL bmc_writeb(WORD addr, BYTE dat)
{
	/* �L���E�����`�F�b�N */
	if (!bmc_enable) {
		return FALSE;
	}

	/* 32KB��FM-8���[�h������ */
	if (fm_subtype != FMSUB_FM8) {
		return FALSE;
	}

	/* �A�h���X�`�F�b�N */
	if ((addr & 0xfff8) != 0xfd10) {
		return FALSE;
	}

	bmc_use = TRUE;

	switch (addr) {
		/* �f�[�^���W�X�^(BDATA) */
		case 0xfd10:
			bmc_datareg = dat;
			/* �J�E���^���� */
			if (bmc_dataptr) {
				bmc_dataptr[bmc_nowcnt] = bmc_datareg;
				bmc_nowcnt++;
				if (bmc_nowcnt == bmc_totalcnt) {
#ifdef FDDSND
					if (bmc_wait) {
						schedule_delevent(EVENT_BMC_LOST);
					}
#endif
					bmc_status &= (BYTE)(~BMC_ST_BUSY);
					bmc_status &= (BYTE)(~BMC_ST_TDRA);
					bmc_status |= (BYTE)BMC_ST_CME;

					/* ���C�g�y�[�W���� */
					if (!bmc_write_page()) {
						bmc_status |= (BYTE)(BMC_ST_CME | BMC_ST_ERROR);
						bmc_errorreg |= (BYTE)BMC_ES_NOMAKER;
					}

					bmc_countreg--;
					if ((bmc_countreg > 0) &&
						((bmc_pagereg & BMC_MAXADDR_32) < BMC_MAXADDR_32)) {
						/* �}���`�y�[�W���� */
						bmc_status |= (BYTE)BMC_ST_BUSY;
						bmc_status &= (BYTE)(~BMC_ST_CME);
						bmc_dataptr = NULL;
						schedule_setevent(EVENT_BMC_MULTI, 30, bmc_multi_event);

						return TRUE;
					}

					/* �V���O���y�[�W���� or �}���`�y�[�W�I������ */
					bmc_dataptr = NULL;

					/* �A�N�Z�X(READY) */
					bmc_access[bmc_unit] = (BYTE)BMC_ACCESS_READY;

					/* ���W�X�^������ */
					bmc_command &= (BYTE)0xf0;
					bmc_pagereg = 0;
					bmc_countreg = 0;
				}
				else {
#ifdef FDDSND
					if (bmc_wait) {
						schedule_setevent(EVENT_BMC_LOST, BMC_LOST_TIME, bmc_drq_event);
						bmc_status &= (BYTE)(~BMC_ST_TDRA);
					}
					else {
						bmc_status |= (BYTE)BMC_ST_TDRA;
					}
#else
					bmc_status |= (BYTE)BMC_ST_TDRA;
#endif
				}
			}
			return TRUE;

		/* �R�}���h���W�X�^(BCMD) */
		case 0xfd11:
			bmc_command = dat;
			bmc_process_cmd();
			return TRUE;

		/* �y�[�W�A�h���X���W�X�^H(BPGADH) */
		case 0xfd14:
			bmc_pagereg &= 0x00ff;
			bmc_pagereg |= (WORD)(dat << 8);
			return TRUE;

		/* �y�[�W�A�h���X���W�X�^L(BPGADL) */
		case 0xfd15:
			bmc_pagereg &= 0xff00;
			bmc_pagereg |= dat;
			return TRUE;

		/* �y�[�W�J�E���g���W�X�^H(BPGCTH) */
		case 0xfd16:
			bmc_countreg &= 0x00ff;
			bmc_countreg |= (WORD)(dat << 8);
			return TRUE;

		/* �y�[�W�J�E���g���W�X�^L(BPGCTL) */
		case 0xfd17:
			bmc_countreg &= 0xff00;
			bmc_countreg |= dat;
			return TRUE;
	}

	return FALSE;
}

/*
 *	�o�u�������� �R���g���[��
 *	�Z�[�u
 */
BOOL FASTCALL bmc_save(int fileh)
{
	int i, bmc_units;
	DWORD size;

	bmc_units = BMC_UNITS_32;
	size = (DWORD)BMC_PSIZE_32;

	/* �t�@�C���֌W���Ɏ����Ă��� */
	for (i=0; i<bmc_units; i++) {
		if (!file_byte_write(fileh, bmc_ready[i])) {
			return FALSE;
		}
	}
	for (i=0; i<bmc_units; i++) {
		if (!file_write(fileh, (BYTE*)bmc_fname[i], 256 + 1)) {
			return FALSE;
		}
	}
	for (i=0; i<bmc_units; i++) {
		if (!file_byte_write(fileh, bmc_media[i])) {
			return FALSE;
		}
	}

	for (i=0; i<bmc_units; i++) {
		if (!file_bool_write(fileh, bmc_teject[i])) {
			return FALSE;
		}
	}

	/* �t�@�C���X�e�[�^�X */
	if (!file_write(fileh, bmc_buffer, size)) {
		return FALSE;
	}

	/* bmc_dataptr�͊��Ɉˑ�����f�[�^�|�C���^ */
	if (!bmc_dataptr) {
		if (!file_word_write(fileh, (WORD)size)) {
			return FALSE;
		}
	}
	else {
		if (!file_word_write(fileh, (WORD)(bmc_dataptr - &bmc_buffer[0]))) {
			return FALSE;
		}
	}

	if (!file_dword_write(fileh, bmc_offset)) {
		return FALSE;
	}

	/* I/O */
	if (!file_byte_write(fileh, bmc_datareg)) {
		return FALSE;
	}
	if (!file_byte_write(fileh, bmc_command)) {
		return FALSE;
	}
	if (!file_byte_write(fileh, bmc_status)) {
		return FALSE;
	}
	if (!file_byte_write(fileh, bmc_errorreg)) {
		return FALSE;
	}
	if (!file_word_write(fileh, bmc_pagereg)) {
		return FALSE;
	}
	if (!file_word_write(fileh, bmc_countreg)) {
		return FALSE;
	}

	/* ���̑� */
	if (!file_word_write(fileh, bmc_totalcnt)) {
		return FALSE;
	}
	if (!file_word_write(fileh, bmc_nowcnt)) {
		return FALSE;
	}
	for (i=0; i<bmc_units; i++) {
		if (!file_byte_write(fileh, bmc_access[i])) {
			return FALSE;
		}
	}

	/* �L���E�����`�F�b�N */
	if (!file_bool_write(fileh, bmc_enable)) {
		return FALSE;
	}
	if (!file_bool_write(fileh, bmc_use)) {
		return FALSE;
	}

#if defined(FDDSND)
	if (!file_bool_write(fileh, bmc_wait)) {
		return FALSE;
	}
#else
	if (!file_bool_write(fileh, FALSE)) {
		return FALSE;
	}
#endif

	return TRUE;
}

/*
 *	�o�u�������� �R���g���[��
 *	���[�h
 */
BOOL FASTCALL bmc_load(int fileh, int ver)
{
	int i, bmc_units;
	BYTE ready[BMC_UNITS_32];
	char fname[BMC_UNITS_32][256 + 1];
	BYTE media[BMC_UNITS_32];
	WORD offset;
	DWORD size;
#if !defined(FDDSND)
	BOOL tmp;
#endif

	/* �o�[�W�����`�F�b�N */
	if (ver < 200) {
		return FALSE;
	}

	if (ver < 307) {
		bmc_init();
		bmc_reset();
		return TRUE;
	}

	bmc_units = BMC_UNITS_32;
	size = (DWORD)BMC_PSIZE_32;

	/* �t�@�C���֌W���Ɏ����Ă��� */
	for (i=0; i<bmc_units; i++) {
		if (!file_byte_read(fileh, &ready[i])) {
			return FALSE;
		}
	}
	for (i=0; i<bmc_units; i++) {
		if (!file_read(fileh, (BYTE*)fname[i], 256 + 1)) {
			return FALSE;
		}
	}
	if (ver >= 308) {
		for (i=0; i<bmc_units; i++) {
			if (!file_byte_read(fileh, &media[i])) {
				return FALSE;
			}
		}
	}
	else {
		for (i=0; i<bmc_units; i++) {
			media[i] = 0;
		}
	}

	/* �ă}�E���g�����݂� */
	for (i=0; i<bmc_units; i++) {
		bmc_setfile(i, NULL);
		if (ready[i] != BMC_TYPE_NOTREADY) {
			bmc_setfile(i, fname[i]);
			if (bmc_ready[i] != BMC_TYPE_NOTREADY) {
				if (bmc_medias[i] >= (media[i] + 1)) {
					bmc_setmedia(i, media[i]);
				}
			}
		}
	}

	for (i=0; i<bmc_units; i++) {
		if (!file_bool_read(fileh, &bmc_teject[i])) {
			return FALSE;
		}
	}

	/* �t�@�C���X�e�[�^�X */
	if (!file_read(fileh, bmc_buffer, size)) {
		return FALSE;
	}

	/* bmc_dataptr�͊��Ɉˑ�����f�[�^�|�C���^ */
	if (!file_word_read(fileh, &offset)) {
		return FALSE;
	}
	if (offset >= (WORD)size) {
		bmc_dataptr = NULL;
	}
	else {
		bmc_dataptr = &bmc_buffer[offset];
	}

	if (!file_dword_read(fileh, &bmc_offset)) {
		return FALSE;
	}

	/* I/O */
	if (!file_byte_read(fileh, &bmc_datareg)) {
		return FALSE;
	}
	if (!file_byte_read(fileh, &bmc_command)) {
		return FALSE;
	}
	if (!file_byte_read(fileh, &bmc_status)) {
		return FALSE;
	}
	if (!file_byte_read(fileh, &bmc_errorreg)) {
		return FALSE;
	}
	if (!file_word_read(fileh, &bmc_pagereg)) {
		return FALSE;
	}
	if (!file_word_read(fileh, &bmc_countreg)) {
		return FALSE;
	}

	/* ���̑� */
	if (!file_word_read(fileh, &bmc_totalcnt)) {
		return FALSE;
	}
	if (!file_word_read(fileh, &bmc_nowcnt)) {
		return FALSE;
	}
	for (i=0; i<bmc_units; i++) {
		if (!file_byte_read(fileh, &bmc_access[i])) {
			return FALSE;
		}
	}

	/* �y�[�W���烆�j�b�g���Z�o */
	bmc_unit = (BYTE)(bmc_pagereg >> 10);

	/* �L���E�����`�F�b�N */
	if (!file_bool_read(fileh, &bmc_enable)) {
		return FALSE;
	}

	if (!file_bool_read(fileh, &bmc_use)) {
		return FALSE;
	}

#if defined(FDDSND)
	if (!file_bool_read(fileh, &bmc_wait)) {
		return FALSE;
	}
#else
	if (!file_bool_read(fileh, &tmp)) {
		return FALSE;
	}
#endif

	/* �C�x���g */
#if defined(FDDSND)
	if (bmc_wait) {
		schedule_handle(EVENT_BMC_LOST, bmc_drq_event);
	}
	else {
		schedule_handle(EVENT_BMC_LOST, bmc_lost_event);
	}
#else
	schedule_handle(EVENT_BMC_LOST, bmc_lost_event);
#endif
	schedule_handle(EVENT_BMC_MULTI, bmc_multi_event);

	return TRUE;
}
#else
/*
 *	�o�u�������� �R���g���[��
 *	�Z�[�u(�_�~�[)
 */
BOOL FASTCALL bmc_save(int fileh)
{
	BYTE tmp[256 + 1];
	int i, bmc_units;
	DWORD size;

	bmc_units = BMC_UNITS_32;
	size = (DWORD)BMC_PSIZE_32;
	memset(tmp, 0, sizeof(tmp));

	/* �t�@�C���֌W���Ɏ����Ă��� */
	for (i=0; i<bmc_units; i++) {
		if (!file_byte_write(fileh, 0)) {
			return FALSE;
		}
	}
	for (i=0; i<bmc_units; i++) {
		if (!file_write(fileh, tmp, 256 + 1)) {
			return FALSE;
		}
	}
	for (i=0; i<bmc_units; i++) {
		if (!file_byte_write(fileh, 0)) {
			return FALSE;
		}
	}

	for (i=0; i<bmc_units; i++) {
		if (!file_bool_write(fileh, 0)) {
			return FALSE;
		}
	}

	/* �t�@�C���X�e�[�^�X */
	if (!file_write(fileh, tmp, size)) {
		return FALSE;
	}

	/* bmc_dataptr�͊��Ɉˑ�����f�[�^�|�C���^ */
	if (!file_word_write(fileh, (WORD)size)) {
		return FALSE;
	}

	if (!file_dword_write(fileh, 0)) {
		return FALSE;
	}

	/* I/O */
	if (!file_byte_write(fileh, 0)) {
		return FALSE;
	}
	if (!file_byte_write(fileh, 0)) {
		return FALSE;
	}
	if (!file_byte_write(fileh, 0)) {
		return FALSE;
	}
	if (!file_byte_write(fileh, 0)) {
		return FALSE;
	}
	if (!file_word_write(fileh, 0)) {
		return FALSE;
	}
	if (!file_word_write(fileh, 0)) {
		return FALSE;
	}

	/* ���̑� */
	if (!file_word_write(fileh, 0)) {
		return FALSE;
	}
	if (!file_word_write(fileh, 0)) {
		return FALSE;
	}
	for (i=0; i<bmc_units; i++) {
		if (!file_byte_write(fileh, 0)) {
			return FALSE;
		}
	}

	/* �L���E�����`�F�b�N */
	if (!file_bool_write(fileh, FALSE)) {
		return FALSE;
	}
	if (!file_bool_write(fileh, FALSE)) {
		return FALSE;
	}

	if (!file_bool_write(fileh, FALSE)) {
		return FALSE;
	}

	return TRUE;
}

/*
 *	�o�u�������� �R���g���[��
 *	���[�h(�_�~�[)
 */
BOOL FASTCALL bmc_load(int fileh, int ver)
{
	BYTE tmp[256 + 1];
	int i, bmc_units;
	DWORD size;

	/* �o�[�W�����`�F�b�N */
	if (ver < 200) {
		return FALSE;
	}

	if (ver < 307) {
		return TRUE;
	}

	bmc_units = BMC_UNITS_32;
	size = (DWORD)BMC_PSIZE_32;

	/* �t�@�C���֌W���Ɏ����Ă��� */
	for (i=0; i<bmc_units; i++) {
		if (!file_byte_read(fileh, (BYTE*)tmp)) {
			return FALSE;
		}
	}
	for (i=0; i<bmc_units; i++) {
		if (!file_read(fileh, tmp, 256 + 1)) {
			return FALSE;
		}
	}
	if (ver >= 308) {
		for (i=0; i<bmc_units; i++) {
			if (!file_byte_read(fileh, (BYTE*)tmp)) {
				return FALSE;
			}
		}
	}

	for (i=0; i<bmc_units; i++) {
		if (!file_bool_read(fileh, (BOOL*)tmp)) {
			return FALSE;
		}
	}

	/* �t�@�C���X�e�[�^�X */
	if (!file_read(fileh, tmp, size)) {
		return FALSE;
	}

	/* bmc_dataptr�͊��Ɉˑ�����f�[�^�|�C���^ */
	if (!file_word_read(fileh, (WORD*)tmp)) {
		return FALSE;
	}

	if (!file_dword_read(fileh, (DWORD*)tmp)) {
		return FALSE;
	}

	/* I/O */
	if (!file_byte_read(fileh, (BYTE*)tmp)) {
		return FALSE;
	}
	if (!file_byte_read(fileh, (BYTE*)tmp)) {
		return FALSE;
	}
	if (!file_byte_read(fileh, (BYTE*)tmp)) {
		return FALSE;
	}
	if (!file_byte_read(fileh, (BYTE*)tmp)) {
		return FALSE;
	}
	if (!file_word_read(fileh, (WORD*)tmp)) {
		return FALSE;
	}
	if (!file_word_read(fileh, (WORD*)tmp)) {
		return FALSE;
	}

	/* ���̑� */
	if (!file_word_read(fileh, (WORD*)tmp)) {
		return FALSE;
	}
	if (!file_word_read(fileh, (WORD*)tmp)) {
		return FALSE;
	}
	for (i=0; i<bmc_units; i++) {
		if (!file_byte_read(fileh, (BYTE*)tmp)) {
			return FALSE;
		}
	}

	/* �L���E�����`�F�b�N */
	if (!file_bool_read(fileh, (BOOL*)tmp)) {
		return FALSE;
	}

	if (!file_bool_read(fileh, (BOOL*)tmp)) {
		return FALSE;
	}

	if (!file_bool_read(fileh, (BOOL*)tmp)) {
		return FALSE;
	}

	/* �C�x���g */
	schedule_delevent(EVENT_BMC_LOST);
	schedule_delevent(EVENT_BMC_MULTI);

	return TRUE;
}

#endif	/* !defined(XM7PURE) */
#endif	/* defined(BUBBLE) */
#endif	/* XM7_VER == 1 */
