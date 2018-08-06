/*
 *	FM-7 EMULATOR "XM7"
 *
 *	Copyright (C) 1999-2017 �o�h�D(Twitter:@xm6_original)
 *	Copyright (C) 2001-2017 Ryu Takegami (Twitter:@RyuTakegami)
 *
 *	[ �⏕�c�[�� ]
 *
 *	RHG����
 *	  2001.10.03		�k���摜�L���v�`���@�\��ǉ�
 *	  2002.03.04		8�F���[�h�̃p���b�g��񂪂��������Ȃ�����C��
 *	  2002.03.27		VTP��T77�ϊ����g�����̂ɂȂ�Ȃ������C��
 *	  2002.05.04		F-BASIC���[�U�f�B�X�N�쐬�@�\��ǉ�
 *	  2002.09.09		VTP���ϊ���GAP�o�͕���VC++�Ōx�����o�Ȃ��悤�ɕύX
 *	  2002.10.27		8�F���[�h�摜�k���̍��F�A���S���Y����ύX
 *	  2003.08.12		���[�U�f�B�X�N��IPL��ORCC #$50��ǉ�
 *	  2004.03.17		2D��D77�ϊ�������2DD�x�^�C���[�W�ɑΉ�
 *						2D��D77�ϊ��̃��f�B�A�^�C�v�������݂𖾎��I�ɂ���
 *	  2004.12.15		77L4 400���C���摜�L���v�`����ǉ�
 *	  2004.12.22		77L4 400���C���k���摜�L���v�`�����C���e���V�e�B�r
 *						�b�g�Ή��ɕύX
 *	  2010.01.13		TrueColor���̋P�x�ϊ����e�[�u����
 *	  2012.07.01		�u�����N�o�u���J�Z�b�g�쐬�@�\��XM7dash����ڐA
 *	  2014.03.28		��ʃL���v�`���@�\���O���[�����j�^���[�h/TTL���j�^
 *						���[�h�ɑΉ�
 */


#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "xm7.h"
#include "multipag.h"
#include "ttlpalet.h"
#include "apalet.h"
#include "subctrl.h"
#include "display.h"
#include "device.h"
#include "tools.h"


/*
 *	�f�W�^���p���b�g�e�[�u��
 *	(RGBQUAD����)
 */
static BYTE bmp_palet_table[] = {
	0x00, 0x00, 0x00, 0x00,
	0xff, 0x00, 0x00, 0x00,
	0x00, 0x00, 0xff, 0x00,
	0xff, 0x00, 0xff, 0x00,
	0x00, 0xff, 0x00, 0x00,
	0xff, 0xff, 0x00, 0x00,
	0x00, 0xff, 0xff, 0x00,
	0xff, 0xff, 0xff, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x1c, 0x00, 0x00,
	0x00, 0x4c, 0x00, 0x00,
	0x00, 0x68, 0x00, 0x00,
	0x00, 0x96, 0x00, 0x00,
	0x00, 0xb2, 0x00, 0x00,
	0x00, 0xe2, 0x00, 0x00,
	0x00, 0xff, 0x00, 0x00,
};

#if (XM7_VER == 1) && defined(L4CARD)
static BYTE bmp_palet_table_16[] = {
	0x00, 0x00, 0x00, 0x00,
	0xbb, 0x00, 0x00, 0x00,
	0x00, 0x00, 0xbb, 0x00,
	0xbb, 0x00, 0xbb, 0x00,
	0x00, 0xbb, 0x00, 0x00,
	0xbb, 0xbb, 0x00, 0x00,
	0x00, 0xbb, 0xbb, 0x00,
	0xbb, 0xbb, 0xbb, 0x00,
	0x44, 0x44, 0x44, 0x00,
	0xff, 0x44, 0x44, 0x00,
	0x44, 0x44, 0xff, 0x00,
	0xff, 0x44, 0xff, 0x00,
	0x44, 0xff, 0x44, 0x00,
	0xff, 0xff, 0x44, 0x00,
	0x44, 0xff, 0xff, 0x00,
	0xff, 0xff, 0xff, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x34, 0x00, 0x00,
	0x00, 0x5e, 0x00, 0x00,
	0x00, 0x69, 0x00, 0x00,
	0x00, 0xa0, 0x00, 0x00,
	0x00, 0xa6, 0x00, 0x00,
	0x00, 0xb5, 0x00, 0x00,
	0x00, 0xbb, 0x00, 0x00,
	0x00, 0x45, 0x00, 0x00,
	0x00, 0x60, 0x00, 0x00,
	0x00, 0x8c, 0x00, 0x00,
	0x00, 0x99, 0x00, 0x00,
	0x00, 0xde, 0x00, 0x00,
	0x00, 0xf8, 0x00, 0x00,
	0x00, 0xff, 0x00, 0x00,
};
#endif

#if XM7_VER == 2
/* HighColor�p�p���b�g�e�[�u�� */
static WORD bmp_palet_table_ttl[] = {
	0x0000 | 0x0000 | 0x0000,
	0x0000 | 0x0000 | 0x001f,
	0x7c00 | 0x0000 | 0x0000,
	0x7c00 | 0x0000 | 0x001f,
	0x0000 | 0x03e0 | 0x0000,
	0x0000 | 0x03e0 | 0x001f,
	0x7c00 | 0x03e0 | 0x0000,
	0x7c00 | 0x03e0 | 0x001f
};
#endif

/*
 *	�摜�k���p
 */
#define GAMMA200L	1.483239697419133	/* �摜�k�����̃��␳�l (200LINE) */
#define GAMMA400L	1.217883285630907	/* �摜�k�����̃��␳�l (400LINE) */
static BYTE color_bit_mask[3] = { 1, 4, 2 };
static DWORD color_add_data[5];


/*
 *	�u�����N�f�B�X�N�쐬 �T�u
 */
static BOOL FASTCALL make_d77_sub(int fileh, DWORD dat)
{
	BYTE buf[4];

	buf[0] = (BYTE)(dat & 0xff);
	buf[1] = (BYTE)((dat >> 8) & 0xff);
	buf[2] = (BYTE)((dat >> 16) & 0xff);
	buf[3] = (BYTE)((dat >> 24) & 0xff);

	return file_write(fileh, buf, 4);
}

/*
 *	�u�����N�f�B�X�N�쐬
 */
BOOL FASTCALL make_new_d77(char *fname, char *name, BOOL mode2dd)
{
	int fileh;
	BYTE header[0x2b0];
	DWORD offset;
	int i;
	int j;

	/* assert */
	ASSERT(fname);

	/* unused */
#if XM7_VER <= 2
	UNUSED(mode2dd);
#endif

	/* �t�@�C���I�[�v�� */
	fileh = file_open(fname, OPEN_W);
	if (fileh == -1) {
		return FALSE;
	}

	/* �w�b�_�쐬 */
	memset(header, 0, sizeof(header));
	if (name != NULL) {
		for (i=0; i<16; i++) {
			if (name[i] == '\0') {
				break;
			}
			header[i] = name[i];
		}
	}
	else {
		strcpy((char*)header, "Default");
	}

	/* ���x(2D,2DD,2HD) */
#if XM7_VER >= 3
	if (mode2dd) {
		header[0x1b] = 0x10;
	}
#endif

	/* �w�b�_�������� */
	if (!file_write(fileh, header, 0x1c)) {
		file_close(fileh);
		return FALSE;
	}

	/* �t�@�C���g�[�^���T�C�Y */
#if XM7_VER >= 3
	if (mode2dd) {
		offset = 0x0e1ab0;
	}
	else {
		offset = 0x073ab0;
	}
#else
	offset = 0x073ab0;
#endif
	if (!make_d77_sub(fileh, offset)) {
		file_close(fileh);
		return FALSE;
	}

	/* �g���b�N�I�t�Z�b�g */
	offset = 0x2b0;
	for (i=0; i<84; i++) {
		if (!make_d77_sub(fileh, offset)) {
			file_close(fileh);
			return FALSE;
		}
		offset += 0x1600;
	}
#if XM7_VER >= 3
	if (mode2dd) {
		for (i=0; i<80; i++) {
			if (!make_d77_sub(fileh, offset)) {
				file_close(fileh);
				return FALSE;
			}
			offset += 0x1600;
		}
	}
	else {
		/* �w�b�_�������� */
		if (!file_write(fileh, &header[0x170], 0x2b0 - 0x170)) {
			file_close(fileh);
			return FALSE;
		}
	}
#else
	/* �w�b�_�������� */
	if (!file_write(fileh, &header[0x170], 0x2b0 - 0x170)) {
		file_close(fileh);
		return FALSE;
	}
#endif

	/* �k���f�[�^�������� */
	memset(header, 0, sizeof(header));
	for (i=0; i<84; i++) {
		for (j=0; j<11; j++) {
			if (!file_write(fileh, header, 0x200)) {
				file_close(fileh);
				return FALSE;
			}
		}
	}
#if XM7_VER >= 3
	if (mode2dd) {
		for (i=0; i<80; i++) {
			for (j=0; j<11; j++) {
				if (!file_write(fileh, header, 0x200)) {
					file_close(fileh);
					return FALSE;
				}
			}
		}
	}
#endif

	/* ok */
	file_close(fileh);
	return TRUE;
}

/*
 *	���[�U�f�B�X�N�쐬
 */
BOOL FASTCALL make_new_userdisk(char *fname, char *name, BOOL mode2dd)
{
	static const BYTE dummyipl[9] = {
		0x1a, 0x50,			/* ORCC	#$50	*/
		0x86, 0x41,			/* LDA	#$41	*/
		0xb7, 0xfd, 0x03,	/* STA	$FD03	*/
		0x20, 0xfe			/* BRA	*		*/
	};

	int fileh;
	BYTE header[0x2b0];
	DWORD offset;
	int i;
	int j;
	int k;

	/* assert */
	ASSERT(fname);

	/* unused */
#if XM7_VER <= 2
	UNUSED(mode2dd);
#endif

	/* �t�@�C���I�[�v�� */
	fileh = file_open(fname, OPEN_W);
	if (fileh == -1) {
		return FALSE;
	}

	/* �w�b�_�쐬 */
	memset(header, 0, sizeof(header));
	if (name != NULL) {
		for (i=0; i<16; i++) {
			if (name[i] == '\0') {
				break;
			}
			header[i] = name[i];
		}
	}
	else {
		strcpy((char*)header, "Default");
	}

	/* ���x(2D,2DD,2HD) */
#if XM7_VER >= 3
	if (mode2dd) {
		header[0x1b] = 0x10;
	}
#endif

	/* �w�b�_�������� */
	if (!file_write(fileh, header, 0x1c)) {
		file_close(fileh);
		return FALSE;
	}

	/* �t�@�C���g�[�^���T�C�Y */
#if XM7_VER >= 3
	if (mode2dd) {
		offset = 0x0aa2b0;
	}
	else {
		offset = 0x0552b0;
	}
#else
	offset = 0x0552b0;
#endif
	if (!make_d77_sub(fileh, offset)) {
		file_close(fileh);
		return FALSE;
	}

	/* �g���b�N�I�t�Z�b�g */
	offset = 0x2b0;
#if XM7_VER >= 3
	if (mode2dd) {
		k = 160;
	}
	else {
		k = 80;
	}
#else
	k = 80;
#endif
	for (i=0; i<164; i++) {
		if (i >= k) {
			offset = 0x0000;
		}
		if (!make_d77_sub(fileh, offset)) {
			file_close(fileh);
			return FALSE;
		}
		if (offset) {
			offset += 0x1100;
		}
	}

	/* �k���f�[�^�������� */
	memset(header, 0, sizeof(header));
	for (i=0; i<k; i++) {
		for (j=1; j<=16; j++) {
			memset(header, 0, 0x10);
			header[0] = (BYTE)(i >> 1);
			header[1] = (BYTE)(i & 1);
			header[2] = (BYTE)j;
			header[3] = 0x01;
			header[4] = 0x10;
			header[14] = 0x00;
			header[15] = 0x01;
			if (!file_write(fileh, header, 0x10)) {
				file_close(fileh);
				return FALSE;
			}

			if ((i == 0) && (j == 1)) {
				/* �_�~�[IPL�Z�N�^�쐬 */
				memset(header, 0x00, 0x100);
				memcpy(header, dummyipl, sizeof(dummyipl));
			}
			else if ((i == 0) && (j == 3)) {
				/* ID�Z�N�^�쐬 */
				memset(header, 0x00, 0x100);
#if XM7_VER >= 3
				if (mode2dd) {
					header[0] = 0x45;
				}
				else {
					header[0] = 0x53;
				}
#else
				header[0] = 0x53;
#endif
				header[1] = 0x20;
				header[2] = 0x20;
			}
			else if ((i == 2) || (i == 3)) {
				/* FAT/�f�B���N�g���쐬 */
				memset(header, 0xff, 0x100);
				if ((i == 2) && (j == 1)) {
					header[0] = 0x00;
				}
			}
			else {
				/* �ʏ�Z�N�^�쐬 */
				memset(header, 0xe5, 0x100);
			}
			if (!file_write(fileh, header, 0x100)) {
				file_close(fileh);
				return FALSE;
			}
		}
	}

	/* ok */
	file_close(fileh);
	return TRUE;
}

/*
 *	�u�����N�e�[�v�쐬
 */
BOOL FASTCALL make_new_t77(char *fname)
{
	int fileh;

	ASSERT(fname);

	/* �t�@�C���I�[�v�� */
	fileh = file_open(fname, OPEN_W);
	if (fileh == -1) {
		return FALSE;
	}

	/* �w�b�_�������� */
	if (!file_write(fileh, (BYTE*)"XM7 TAPE IMAGE 0", 16)) {
		file_close(fileh);
		return FALSE;
	}

	/* ���� */
	file_close(fileh);
	return TRUE;
}


#if XM7_VER == 1
#if defined(BUBBLE)
/*
 *	�u�����N�o�u���J�Z�b�g�쐬
 */
BOOL FASTCALL make_new_bubble(char *fname, char *name)
{
	static const char volumelabel[] = "VOL00000";

	int fileh;
	BYTE buffer[0x40];
	BYTE header[0x20];
	DWORD i;

	/* assert */
	ASSERT(fname);

	/* �t�@�C���I�[�v�� */
	fileh = file_open(fname, OPEN_W);
	if (fileh == -1) {
		return FALSE;
	}

	/* �w�b�_�쐬 */
	if (name != NULL) {
		memset(header, 0, sizeof(header));
		for (i=0; i<16; i++) {
			if (name[i] == '\0') {
				break;
			}
			header[i] = name[i];
		}

		/* �T�C�Y(32KB�Œ�) */
		header[0x1b] = 0x80;

		/* �w�b�_�������� */
		if (!file_write(fileh, header, 0x1c)) {
			file_close(fileh);
			return FALSE;
		}

		/* �t�@�C���g�[�^���T�C�Y */
		if (!make_d77_sub(fileh, 0x008020)) {
			file_close(fileh);
			return FALSE;
		}
	}

	/* �k���f�[�^�������� */
	for (i=0; i<=0x03ff; i++) {
		memset(buffer, 0, sizeof(buffer));
		if (i == 0) {
			/* ID�Z�N�^�쐬 */
			memcpy((char *)buffer, volumelabel, strlen(volumelabel));
			buffer[8] = 0x08;
			buffer[9] = 0x00;
			buffer[10] = 0x00;
			buffer[11] = 0x01;
		}
		if (!file_write(fileh, buffer, 32)) {
			file_close(fileh);
			return FALSE;
		}
	}

	/* ok */
	file_close(fileh);
	return TRUE;
}
#endif	/* defined(BUBBLE) */
#endif	/* XM7_VER == 1 */

/*
 *	VFD��D77�ϊ�
 */
BOOL FASTCALL conv_vfd_to_d77(char *src, char *dst, char *name)
{
	int files;
	int filed;
	BYTE vfd_h[480];
	BYTE d77_h[0x2b0];
	BYTE *buffer;
	int trk;
	int sec;
	int secs;
	int len;
	int trklen;
	DWORD offset;
	DWORD srclen;
	DWORD wrlen;
	BYTE *header;
	BYTE *ptr;

	/* assert */
	ASSERT(src);
	ASSERT(dst);
	ASSERT(name);

	/* ���[�N�������m�� */
	buffer = (BYTE *)malloc(8192);
	if (buffer == NULL) {
		return FALSE;
	}

	/* VFD�t�@�C���I�[�v�� */
	files = file_open(src, OPEN_R);
	if (files == -1) {
		free(buffer);
		return FALSE;
	}

	/* �����ŁA�t�@�C���T�C�Y���擾���Ă��� */
	srclen = file_getsize(files);

	/* VFD�w�b�_�ǂݍ��� */
	if (!file_read(files, vfd_h, sizeof(vfd_h))) {
		free(buffer);
		file_close(files);
		return FALSE;
	}

	/* D77�t�@�C���쐬 */
	filed = file_open(dst, OPEN_W);
	if (filed == -1) {
		free(buffer);
		file_close(files);
		return FALSE;
	}

	/* �w�b�_�쐬 */
	memset(d77_h, 0, sizeof(d77_h));
	if (strlen(name) <= 16) {
		strcpy((char*)d77_h, name);
	}
	else {
		memcpy(d77_h, name, 16);
	}

	/* ��U�A�w�b�_������ */
	if (!file_write(filed, d77_h, sizeof(d77_h))) {
		free(buffer);
		file_close(files);
		file_close(filed);
		return FALSE;
	}

	/* �������݃|�C���^�������� */
	wrlen = sizeof(d77_h);

	/* �g���b�N���[�v */
	header = vfd_h;
	for (trk=0; trk<80; trk++) {
		/* �w�b�_�擾 */
		offset = header[3];
		offset *= 256;
		offset |= header[2];
		offset *= 256;
		offset |= header[1];
		offset *= 256;
		offset |= header[0];
		header += 4;
		len = *header++;
		len &= 7;
		if (len >= 4) {
			len = 3;
		}
		secs = *header++;

		/* secs=0�ւ̑Ή� */
		if (secs == 0) {
			continue;
		}
		else {
			/* �������݃|�C���^���L�� */
			d77_h[trk * 4 + 0x20 + 3] = (BYTE)(wrlen >> 24);
			d77_h[trk * 4 + 0x20 + 2] = (BYTE)((wrlen >> 16) & 255);
			d77_h[trk * 4 + 0x20 + 1] = (BYTE)((wrlen >> 8) & 255);
			d77_h[trk * 4 + 0x20 + 0] = (BYTE)(wrlen & 255);
		}

		/* �g���b�N�����v�Z */
		switch (len) {
			case 0:
				trklen = secs * (128 + 16);
				break;
			case 1:
				trklen = secs * (256 + 16);
				break;
			case 2:
				trklen = secs * (512 + 16);
				break;
			case 3:
				trklen = secs * (1024 + 16);
				break;
		}

		/* �w�b�_���� */
		if ((offset > srclen) | (trklen > 8192)) {
			free(buffer);
			file_close(files);
			file_close(filed);
			return FALSE;
		}

		/* �V�[�N */
		if (!file_seek(files, offset)) {
			free(buffer);
			file_close(files);
			file_close(filed);
			return FALSE;
		}

		/* �Z�N�^���[�v */
		ptr = buffer;
		for (sec=1; sec<=secs; sec++) {
			memset(ptr, 0, 0x10);
			/* C,H,R,N */
			ptr[0] = (BYTE)(trk >> 1);
			ptr[1] = (BYTE)(trk & 1);
			ptr[2] = (BYTE)sec;
			ptr[3] = (BYTE)len;
			/* �Z�N�^�� */
			ptr[4] = (BYTE)(secs);
			/* �Z�N�^�����f�[�^�ǂݍ��� */
			switch (len) {
				case 0:
					ptr[0x0e] = 0x80;
					ptr += 0x10;
					file_read(files, ptr, 128);
					ptr += 128;
					break;
				case 1:
					ptr[0x0f] = 0x01;
					ptr += 0x10;
					file_read(files, ptr, 256);
					ptr += 256;
					break;
				case 2:
					ptr[0x0f] = 0x02;
					ptr += 0x10;
					file_read(files, ptr, 512);
					ptr += 512;
					break;
				case 3:
					ptr[0x0f] = 0x04;
					ptr += 0x10;
					file_read(files, ptr, 1024);
					ptr += 1024;
					break;
			}
		}

		/* �ꊇ�������� */
		if (!file_write(filed, buffer, trklen)) {
			free(buffer);
			file_close(files);
			file_close(filed);
			return FALSE;
		}

		/* �������݃|�C���^��i�߂� */
		wrlen += trklen;
	}

	/* �t�@�C���T�C�Y�ݒ� */
	d77_h[0x1f] = (BYTE)((wrlen >> 24) & 0xff);
	d77_h[0x1e] = (BYTE)((wrlen >> 16) & 0xff);
	d77_h[0x1d] = (BYTE)((wrlen >> 8) & 0xff);
	d77_h[0x1c] = (BYTE)(wrlen & 0xff);

	/* �ēx�A�w�b�_����������� */
	if (!file_seek(filed, 0)) {
		free(buffer);
		file_close(files);
		file_close(filed);
		return FALSE;
	}
	if (!file_write(filed, d77_h, sizeof(d77_h))) {
		free(buffer);
		file_close(files);
		file_close(filed);
		return FALSE;
	}

	/* ���ׂďI�� */
	free(buffer);
	file_close(files);
	file_close(filed);
	return TRUE;
}

/*
 *	2D/2DD��D77�ϊ�
 */
BOOL FASTCALL conv_2d_to_d77(char *src, char *dst, char *name)
{
	int files;
	int filed;
	BYTE d77_h[0x2b0];
	BYTE *buffer;
	BYTE *ptr;
	DWORD offset;
	int trk;
	int sec;
	int size;
	int max_track;

	/* assert */
	ASSERT(src);
	ASSERT(dst);
	ASSERT(name);

	/* ���[�N�������m�� */
	buffer = (BYTE *)malloc(0x1100);
	if (buffer == NULL) {
		return FALSE;
	}

	/* 2D�t�@�C���I�[�v���A�t�@�C���T�C�Y�`�F�b�N */
	files = file_open(src, OPEN_R);
	if (files == -1) {
		free(buffer);
		return FALSE;
	}
	size = file_getsize(files);
#if XM7_VER >= 3
	if ((size != 327680) && (size != 655360)) {
#else
	if (size != 327680) {
		file_close(files);
#endif
		free(buffer);
		return FALSE;
	}

	/* D77�t�@�C���쐬 */
	filed = file_open(dst, OPEN_W);
	if (filed == -1) {
		free(buffer);
		file_close(files);
		return FALSE;
	}

	/* �w�b�_�쐬 */
	memset(d77_h, 0, sizeof(d77_h));
	if (strlen(name) <= 16) {
		strcpy((char*)d77_h, name);
	}
	else {
		memcpy(d77_h, name, 16);
	}

	/* �t�@�C���T�C�Y */
#if XM7_VER >= 3
	if (size == 655360) {
		d77_h[0x1b] = 0x10;
		d77_h[0x1c] = 0xb0;
		d77_h[0x1d] = 0xa2;
		d77_h[0x1e] = 0x0a;
		max_track = 160;
	}
	else {
		d77_h[0x1b] = 0x00;
		d77_h[0x1c] = 0xb0;
		d77_h[0x1d] = 0x52;
		d77_h[0x1e] = 0x05;
		max_track = 80;
	}
#else
	d77_h[0x1b] = 0x00;
	d77_h[0x1c] = 0xb0;
	d77_h[0x1d] = 0x52;
	d77_h[0x1e] = 0x05;
	max_track = 80;
#endif

	/* �g���b�N�I�t�Z�b�g */
	offset = 0x2b0;
	for (trk=0; trk<max_track; trk++) {
		d77_h[0x20 + trk * 4 + 0] = (BYTE)(offset & 0xff);
		d77_h[0x20 + trk * 4 + 1] = (BYTE)((offset >> 8) & 0xff);
		d77_h[0x20 + trk * 4 + 2] = (BYTE)((offset >> 16) & 0xff);
		d77_h[0x20 + trk * 4 + 3] = (BYTE)((offset >> 24) & 0xff);
		offset += 0x1100;
	}

	/* �w�b�_�������� */
	if (!file_write(filed, d77_h, sizeof(d77_h))) {
		free(buffer);
		file_close(files);
		file_close(filed);
		return FALSE;
	}

	/* �g���b�N���[�v */
	for (trk=0; trk<max_track; trk++) {
		ptr = buffer;
		/* �Z�N�^���[�v */
		for (sec=1; sec<=16; sec++) {
			memset(ptr, 0, 0x10);
			/* C,H,R,N */
			ptr[0] = (BYTE)(trk >> 1);
			ptr[1] = (BYTE)(trk & 1);
			ptr[2] = (BYTE)sec;
			ptr[3] = 1;

			/* �Z�N�^���A�����O�X */
			ptr[4] = 16;
			ptr[0x0f] = 0x01;
			ptr += 0x10;

			/* �f�[�^�ǂݍ��� */
			file_read(files, ptr, 256);
			ptr += 256;
		}

		/* �ꊇ�������� */
		if (!file_write(filed, buffer, 0x1100)) {
			free(buffer);
			file_close(files);
			file_close(filed);
			return FALSE;
		}
	}

	/* ���ׂďI�� */
	free(buffer);
	file_close(files);
	file_close(filed);
	return TRUE;
}

/*
 *	VTP��T77�ϊ�
 *	�P�o�C�g�o��
 */
static BOOL FASTCALL vtp_conv_write(int handle, BYTE dat)
{
	int i;
	BYTE buf[44];

	/* �X�^�[�g�r�b�g�ݒ� */
	buf[0] = 0x00;
	buf[1] = 0x34;
	buf[2] = 0x80;
	buf[3] = 0x1a;
	buf[4] = 0x00;
	buf[5] = 0x1a;

	/* �X�g�b�v�r�b�g�ݒ� */
	buf[38] = 0x80;
	buf[39] = 0x2f;
	buf[40] = 0x00;
	buf[41] = 0x37;
	buf[42] = 0x80;
	buf[43] = 0x2f;

	/* 8�r�b�g���� */
	for (i=0; i<8; i++) {
		if (dat & 0x01) {
			buf[i * 4 + 6 + 0] = 0x80;
			buf[i * 4 + 6 + 1] = 0x30;
			buf[i * 4 + 6 + 2] = 0x00;
			buf[i * 4 + 6 + 3] = 0x30;
		}
		else {
			buf[i * 4 + 6 + 0] = 0x80;
			buf[i * 4 + 6 + 1] = 0x18;
			buf[i * 4 + 6 + 2] = 0x00;
			buf[i * 4 + 6 + 3] = 0x1a;
		}
		dat >>= 1;
	}

	/* 44�o�C�g�Ɋg�債�ď������� */
	if (!file_write(handle, buf, 44)) {
		return FALSE;
	}

	return TRUE;
}

/*
 *	VTP��T77�ϊ�
 */
BOOL FASTCALL conv_vtp_to_t77(char *src, char *dst)
{
	int files;
	int filed;
	int i;
	BYTE buf[44];
	BYTE hdr[4];
	char *header = "XM7 TAPE IMAGE 0";
	BYTE dat;
	int count;

	/* assert */
	ASSERT(src);
	ASSERT(dst);

	/* VTP�t�@�C���I�[�v�� */
	files = file_open(src, OPEN_R);
	if (files == -1) {
		return FALSE;
	}

	/* T77�t�@�C���쐬 */
	filed = file_open(dst, OPEN_W);
	if (filed == -1) {
		file_close(files);
		return FALSE;
	}

	/* �w�b�_�������� */
	if (!file_write(filed, (BYTE*)header, 16)) {
		file_close(filed);
		file_close(files);
		return FALSE;
	}

	/* T77�f�[�^�쐬 */
	while (TRUE) {
		/* �J�E���^������ */
		count = 0;

		/* �S�~�f�[�^�X�L�b�v */
		/* 32�ȏ�̘A������ 0xFF �𔭌��ł���܂ŌJ��Ԃ� */
		do {
			/* �r���Ńt�@�C�����I������ꍇ�͐���I�� */
			if (!file_read(files, &dat, 1)) {
				file_close(filed);
				file_close(files);
				return TRUE;
			}

			/* 0xFF�𔭌��ł�����J�E���^�����A����ȊO�Ȃ�J�E���^�N���A */
			if (dat == 0xff) {
				count ++;
			}
			else {
				count = 0;
			}
		} while (count < 32);

		/* 1�t�@�C�����̃f�[�^���쐬���� */
		do {
			/* �w�b�_���� */
			/* �r���Ńt�@�C�����I������ꍇ�͐���I�� */
			do {
				do {
					if (!file_read(files, &hdr[0], 1)) {
						file_close(filed);
						file_close(files);
						return TRUE;
					}
					count ++;
				} while (hdr[0] != 0x01);

				if (!file_read(files, &hdr[1], 1)) {
					file_close(filed);
					file_close(files);
					return TRUE;
				}
			} while ((hdr[0] != 0x01) || (hdr[1] != 0x3c));

			/* �c��̃w�b�_����ǂݍ��� */
			for (i = 2; i < 4; i++) {
				if (!file_read(files, &hdr[i], 1)) {
					file_close(filed);
					file_close(files);
					return FALSE;
				}
			}

			/* �t�@�C�����u���b�N�������ꍇ��Gap�̑O�Ƀ}�[�J��ݒ� */
			if (hdr[2] == 0x00) {
				buf[0] = 0;
				buf[1] = 0;
				buf[2] = 0x7f;
				buf[3] = 0xff;
				if (!file_write(filed, buf, 4)) {
					file_close(filed);
					file_close(files);
					return FALSE;
				}
			}

			/* Gap�������� */
			/* �f�[�^�͑S�� 0xFF �ɓ��ꂷ�� */
			for (i = 0; i < count; i++) {
				if (!vtp_conv_write(filed, 0xFF)) {
					file_close(filed);
					file_close(files);
					return FALSE;
				}
			}

			/* �w�b�_�������� */
			for (i = 0; i < 4; i++) {
				if (!vtp_conv_write(filed, hdr[i])) {
					file_close(filed);
					file_close(files);
					return FALSE;
				}
			}

			/* �f�[�^�E�`�F�b�N�T���������� */
			for (i = 0; i <= hdr[3]; i++) {
				if (!file_read(files, &dat, 1)) {
					file_close(filed);
					file_close(files);
					return FALSE;
				}
				if (!vtp_conv_write(filed, dat)) {
					file_close(filed);
					file_close(files);
					return FALSE;
				}
			}

			/* �J�E���^������ */
			count = 0;
		} while (hdr[2] != 0xFF);
	}
}

#if XM7_VER == 1
#if defined(BUBBLE)
/*
 *	BBL��B77�ϊ�
 */
BOOL FASTCALL conv_bbl_to_b77(char *src, char *dst, char *name)
{
	int files;
	int filed;
	BYTE buffer[0x40];
	BYTE b77_h[0x20];
	int size;
	int page;

	/* assert */
	ASSERT(src);
	ASSERT(dst);
	ASSERT(name);

	/* BBL�t�@�C���I�[�v���A�t�@�C���T�C�Y�`�F�b�N */
	files = file_open(src, OPEN_R);
	if (files == -1) {
		return FALSE;
	}
	size = file_getsize(files);
	if (size != 32768) {
		file_close(files);
		return FALSE;
	}

	/* B77�t�@�C���쐬 */
	filed = file_open(dst, OPEN_W);
	if (filed == -1) {
		file_close(files);
		return FALSE;
	}

	/* �w�b�_�쐬 */
	memset(b77_h, 0, sizeof(b77_h));
	if (strlen(name) <= 16) {
		strcpy((char*)b77_h, name);
	}
	else {
		memcpy(b77_h, name, 16);
	}

	/* �t�@�C���T�C�Y */
	b77_h[0x1b] = 0x80;
	b77_h[0x1c] = 0x20;
	b77_h[0x1d] = 0x80;
	b77_h[0x1e] = 0x00;

	/* �w�b�_�������� */
	if (!file_write(filed, b77_h, sizeof(b77_h))) {
		file_close(files);
		file_close(filed);
		return FALSE;
	}

	/* �y�[�W���[�v */
	for (page=0; page<0x0400; page++) {

		/* �f�[�^�ǂݍ��� */
		file_read(files, buffer, 0x0020);

		/* �f�[�^�������� */
		if (!file_write(filed, buffer, 0x0020)) {
			free(buffer);
			file_close(files);
			file_close(filed);
			return FALSE;
		}
	}

	/* ���ׂďI�� */
	file_close(files);
	file_close(filed);

	return TRUE;
}
#endif
#endif

/*
 *	BMP�w�b�_��������
 */
static BOOL FASTCALL bmp_header_sub(int fileh)
{
	BYTE filehdr[14];
	BYTE infohdr[40];

	ASSERT(fileh >= 0);

	/* �\���̃N���A */
	memset(filehdr, 0, sizeof(filehdr));
	memset(infohdr, 0, sizeof(infohdr));

	/* BITMAPFILEHEADER */
	filehdr[0] = 'B';
	filehdr[1] = 'M';

#if XM7_VER >= 3
	switch (screen_mode) {
		case SCR_262144 :	/* 262144�F �t�@�C���T�C�Y 14+40+768000 */
							filehdr[2] = 0x36;
							filehdr[3] = 0xb8;
							filehdr[4] = 0x0b;
							break;
		case SCR_4096	:	/* 4096�F �t�@�C���T�C�Y 14+40+512000 */
							filehdr[2] = 0x36;
							filehdr[3] = 0xd0;
							filehdr[4] = 0x07;
							break;
		default		:	/* 640x200/400 8�F �t�@�C���T�C�Y 14+40+16*4+128000 */
							filehdr[2] = 0x76;
							filehdr[3] = 0xf4;
							filehdr[4] = 0x01;
	}
#elif XM7_VER >= 2
	if (mode320) {
		/* 4096�F �t�@�C���T�C�Y 14+40+512000 */
		filehdr[2] = 0x36;
		filehdr[3] = 0xd0;
		filehdr[4] = 0x07;
	}
	else {
		/* 640x200 8�F �t�@�C���T�C�Y 14+40+16*4+128000 */
		filehdr[2] = 0x76;
		filehdr[3] = 0xf4;
		filehdr[4] = 0x01;
	}
#else
	/* 640x200 8�F �t�@�C���T�C�Y 14+40+16*4+128000 */
	filehdr[2] = 0x76;
	filehdr[3] = 0xf4;
	filehdr[4] = 0x01;
#endif

	/* �r�b�g�}�b�v�ւ̃I�t�Z�b�g */
#if XM7_VER >= 2
#if XM7_VER >= 3
	if (screen_mode & SCR_ANALOG) {
#else
	if (mode320) {
#endif
		/* 4096�F or 262144�F */
		filehdr[10] = 14 + 40;
	}
	else {
		/* 8�F */
		filehdr[10] = 14 + 40 + (16 * 4);
	}
#else
	/* 8�F */
	filehdr[10] = 14 + 40 + (16 * 4);
#endif

	/* BITMAPFILEHEADER �������� */
	if (!file_write(fileh, filehdr, sizeof(filehdr))) {
		return FALSE;
	}

	/* BITMAPINFOHEADER */
	infohdr[0] = 40;
	infohdr[4] = 0x80;
	infohdr[5] = 0x02;
	infohdr[8] = 0x90;
	infohdr[9] = 0x01;
	infohdr[12] = 0x01;
	/* BiBitCount */
#if XM7_VER >= 3
	switch (screen_mode) {
		case	SCR_262144	:	/* 262144�F */
								infohdr[14] = 24;
								break;
		case	SCR_4096	:	/* 4096�F */
								infohdr[14] = 16;
								break;
		default				:	/* 640x200/400 8�F */
								infohdr[14] = 4;
								break;
	}
#elif XM7_VER >= 2
	if (mode320) {
		infohdr[14] = 16;
	}
	else {
		infohdr[14] = 4;
	}
#else
	infohdr[14] = 4;
#endif

	/* BITMAPFILEHEADER �������� */
	if (!file_write(fileh, infohdr, sizeof(infohdr))) {
		return FALSE;
	}

	return TRUE;
}

/*
 *	8�F���[�h�摜�k���p �J���[�����e�[�u��������
 */
void FASTCALL mix_color_init(double gamma)
{
	double maxbrg;
	double brg;
	int i;

	maxbrg = pow(255., 1 / gamma);
	for (i=0; i<=4; i++) {
		brg = pow((double)(i << 6), 1 / gamma);
		color_add_data[i] = ((DWORD)((brg / maxbrg) * 31.) << 15);
	}
}

/*
 *	8�F���[�h�摜�k���p �J���[��������
 */
WORD FASTCALL mix_color(BYTE *palet_table, BYTE palet_count, BOOL mode)
{
	DWORD col;
	BYTE colcount;
	int i;
	int j;

	col = 0;
	if (mode) {
		colcount = 0;
		for (j=0; j<palet_count; j++) {
			colcount += (BYTE)(bmp_palet_table[palet_table[j] * 4 + 33]
						/ palet_count);
		}
		col = (colcount / 8) << 5;
	}
	else {
		for (i=0; i<3; i++) {
			colcount = 0;
			for (j=0; j<palet_count; j++) {
				if (palet_table[j] & color_bit_mask[i]) {
					colcount += (BYTE)(4 / palet_count);
				}
			}
			col = (col | color_add_data[colcount]) >> 5;
		}
	}

	return (WORD)(col & 0x7fff);
}

/*
 *	16�F���[�h�摜�k���p �J���[��������
 */
#if XM7_VER == 1 && defined(L4CARD)
WORD FASTCALL mix_color_16(double gamma, BYTE *palet_table, BYTE palet_count,
						   BOOL mode)
{
	DWORD col;
	DWORD colcount;
	DWORD brg;
	double maxbrg;
	int i;
	int j;

	maxbrg = pow(255., 1 / gamma);
	col = 0;

	if (mode) {
		colcount = 0;
		for (j=0; j<palet_count; j++) {
			colcount += bmp_palet_table_16[palet_table[j] * 4 + 65]
						/ palet_count;
		}
		col = (colcount / 8) << 5;
	}
	else {
		for (i=0; i<3; i++) {
			colcount = 0;
			for (j=0; j<palet_count; j++) {
				if (palet_table[j] & 8) {
					colcount += 0x44;
				}
				if (palet_table[j] & color_bit_mask[i]) {
					colcount += 0xbb;
				}
			}
			colcount /= palet_count;
			brg = (int)((pow((double)colcount, 1 / gamma) / maxbrg) * 31.) << 15;
			col = (col | brg) >> 5;
		}
	}

	return (WORD)(col & 0x7fff);
}
#endif

/*
 *	BMP�w�b�_�������� (�k���摜�p)
 */
static BOOL FASTCALL bmp_header_sub2(int fileh)
{
	BYTE filehdr[14];
	BYTE infohdr[40];

	ASSERT(fileh >= 0);

	/* �\���̃N���A */
	memset(filehdr, 0, sizeof(filehdr));
	memset(infohdr, 0, sizeof(infohdr));

	/* BITMAPFILEHEADER */
	filehdr[0] = 'B';
	filehdr[1] = 'M';

	/* BiBitCount */
#if XM7_VER >= 3
	if (screen_mode == SCR_262144) {
		/* �t�@�C���T�C�Y 14+40+192000 */
		/* 262144�F */
		filehdr[2] = 0x36;
		filehdr[3] = 0xee;
		filehdr[4] = 0x02;
	}
	else {
		/* �t�@�C���T�C�Y 14+40+128000 */
		/* 8�F/4096�F */
		filehdr[2] = 0x36;
		filehdr[3] = 0xf4;
		filehdr[4] = 0x01;
	}
#else
	/* �t�@�C���T�C�Y 14+40+128000 */
	/* 8�F/4096�F */
	filehdr[2] = 0x36;
	filehdr[3] = 0xf4;
	filehdr[4] = 0x01;
#endif

	/* �r�b�g�}�b�v�ւ̃I�t�Z�b�g */
	filehdr[10] = 14+40;

	/* BITMAPFILEHEADER �������� */
	if (!file_write(fileh, filehdr, sizeof(filehdr))) {
		return FALSE;
	}

	/* BITMAPINFOHEADER */
	infohdr[0] = 40;
	infohdr[4] = 0x40;
	infohdr[5] = 0x01;
	infohdr[8] = 0xC8;
	infohdr[9] = 0x00;
	infohdr[12] = 0x01;

	/* BiBitCount */
#if XM7_VER >= 3
	if (screen_mode == SCR_262144) {
		/* 262144�F */
		infohdr[14] = 24;
	}
	else {
		/* 8�F/4096�F */
		infohdr[14] = 16;
	}
#else
	/* 8�F/4096�F */
	infohdr[14] = 16;
#endif

	/* BITMAPFILEHEADER �������� */
	if (!file_write(fileh, infohdr, sizeof(infohdr))) {
		return FALSE;
	}

	return TRUE;
}

/*
 *	BMP�p���b�g��������(8�F/16�F�̂�)
 */
static BOOL FASTCALL bmp_palette_sub(int fileh, BOOL mode)
{
	int i;
	BYTE *p;
	int vpage;

	ASSERT(fileh >= 0);

	/* �\���y�[�W���l�� */
	vpage = (~(multi_page >> 4)) & 0x07;

#if XM7_VER == 1 && defined(L4CARD)
	if (enable_400line) {
		if (crt_flag) {
			/* �Œ�p���b�g16�F */
			if (mode) {
				/* �O���[�����j�^���[�h */
				if (!file_write(fileh, &bmp_palet_table_16[4 * 16], 4 * 16)) {
					return FALSE;
				}
			}
			else {
				/* �ʏ탂�[�h */
				if (!file_write(fileh, bmp_palet_table_16, 4 * 16)) {
					return FALSE;
				}
			}
		}
		else {
			/* ������16�F(�C�����ʓ|�Ȃ̂ŋ��ʏ���) */
			p = bmp_palet_table_16;
			for (i=0; i<16; i++) {
				if (!file_write(fileh, p, 4)) {
					return FALSE;
				}
			}
		}

		return TRUE;
	}
#endif

	/* �p���b�g���8�F */
	for (i=0; i<8; i++) {
		if (crt_flag) {
			if (mode) {
				/* �O���[�����j�^���[�h */
				p = &bmp_palet_table[((ttl_palet[i & vpage] & 0x07) + 8) * 4 ];
			}
			else {
				/* �ʏ탂�[�h */
				p = &bmp_palet_table[ (ttl_palet[i & vpage] & 0x07) * 4 ];
			}
		}
		else {
			p = bmp_palet_table;
		}
		if (!file_write(fileh, p, 4)) {
			return FALSE;
		}
	}

	/* ������8�F(�C�����ʓ|�Ȃ̂ŋ��ʏ���) */
	p = bmp_palet_table;
	for (i=0; i<8; i++) {
		if (!file_write(fileh, p, 4)) {
			return FALSE;
		}
	}

	return TRUE;
}

/*
 *  BMP�f�[�^��������(320���[�h)
 */
#if XM7_VER >= 2
static BOOL FASTCALL bmp_320_sub(int fileh, BOOL fullscan, BOOL mode)
{
	int x, y;
	int offset;
	int i;
	BYTE buffer[2][1280];
	int dat;
	BYTE bit;
	WORD color;
	int mask;
	BYTE *vramptr;
#if XM7_VER >= 3
	WORD dx1,dx2;
	BOOL winy;
#endif

	ASSERT(fileh >= 0);
#if XM7_VER >= 3
	UNUSED(mode);
#endif

	/* �����I�t�Z�b�g�ݒ� */
	offset = 40 * 199;

#if XM7_VER >= 3
	/* �E�B���h�E�̈�̃N���b�s���O���s�� */
	window_clip(1);
	dx1 = (WORD)(window_dx1 >> 3);
	dx2 = (WORD)(window_dx2 >> 3);
#endif

	/* �}�X�N�擾 */
	mask = 0;
#if XM7_VER == 2
	if (mode) {
		mask = ((~multi_page) >> 4) & 0x07;
	}
	else {
		if (!(multi_page & 0x10)) {
			mask |= 0x000f;
		}
		if (!(multi_page & 0x20)) {
			mask |= 0x00f0;
		}
		if (!(multi_page & 0x40)) {
			mask |= 0x0f00;
		}
	}
#else
	if (!(multi_page & 0x10)) {
		mask |= 0x000f;
	}
	if (!(multi_page & 0x20)) {
		mask |= 0x00f0;
	}
	if (!(multi_page & 0x40)) {
		mask |= 0x0f00;
	}
#endif

	/* 0�ŏ������� */
	memset(buffer[0], 0, sizeof(buffer[0]));

	/* y���[�v */
	for (y=0; y<200; y++) {
#if XM7_VER >= 3
		winy = (((199 - y) >= window_dy1) && ((199 - y) <= window_dy2));

		/* x���[�v */
		for (x=0; x<40; x++) {
			bit = 0x80;
			if (winy && (x >= dx1) && (x < dx2)) {
				/* �E�B���h�E��(���u���b�N) */
				vramptr = vram_bdptr;
			}
			else {
				/* �E�B���h�E�O(�\�u���b�N) */
				vramptr = vram_dptr;
			}
#else
		/* x���[�v */
		for (x=0; x<40; x++) {
			bit = 0x80;
			vramptr = vram_dptr;
#endif

			/* �r�b�g���[�v */
			for (i=0; i<8; i++) {
				dat = 0;

#if XM7_VER >= 3
				/* G�]�� */
				if (vramptr[offset + 0x10000] & bit) {
					dat |= 0x800;
				}
				if (vramptr[offset + 0x12000] & bit) {
					dat |= 0x400;
				}
				if (vramptr[offset + 0x14000] & bit) {
					dat |= 0x200;
				}
				if (vramptr[offset + 0x16000] & bit) {
					dat |= 0x100;
				}

				/* R�]�� */
				if (vramptr[offset + 0x08000] & bit) {
					dat |= 0x080;
				}
				if (vramptr[offset + 0x0a000] & bit) {
					dat |= 0x040;
				}
				if (vramptr[offset + 0x0c000] & bit) {
					dat |= 0x020;
				}
				if (vramptr[offset + 0x0e000] & bit) {
					dat |= 0x010;
				}

				/* B�]�� */
				if (vramptr[offset + 0x00000] & bit) {
					dat |= 0x008;
				}
				if (vramptr[offset + 0x02000] & bit) {
					dat |= 0x004;
				}
				if (vramptr[offset + 0x04000] & bit) {
					dat |= 0x002;
				}
				if (vramptr[offset + 0x06000] & bit) {
					dat |= 0x001;
				}

				/* �A�i���O�p���b�g���f�[�^�擾 */
				dat &= mask;
				color = apalet_r[dat];
				color <<= 1;
				if (apalet_r[dat] > 0) {
					color |= 1;
				}
				color <<= 4;

				color |= apalet_g[dat];
				color <<= 1;
				if (apalet_g[dat] > 0) {
					color |= 1;
				}
				color <<= 4;

				color |= apalet_b[dat];
				color <<= 1;
				if (apalet_b[dat] > 0) {
					color |= 1;
				}
#else
				if (mode) {
					/* G�]�� */
					if (vramptr[offset + 0x08000] & bit) {
						dat |= 0x04;
					}

					/* R�]�� */
					if (vramptr[offset + 0x04000] & bit) {
						dat |= 0x02;
					}

					/* B�]�� */
					if (vramptr[offset + 0x00000] & bit) {
						dat |= 0x01;
					}

					/* TTL�p���b�g���f�[�^�擾 */
					color = bmp_palet_table_ttl[ttl_palet[dat & mask] & 7];
				}
				else {
					/* G�]�� */
					if (vramptr[offset + 0x08000] & bit) {
						dat |= 0x800;
					}
					if (vramptr[offset + 0x0a000] & bit) {
						dat |= 0x400;
					}
					if (vramptr[offset + 0x14000] & bit) {
						dat |= 0x200;
					}
					if (vramptr[offset + 0x16000] & bit) {
						dat |= 0x100;
					}

					/* R�]�� */
					if (vramptr[offset + 0x04000] & bit) {
						dat |= 0x080;
					}
					if (vramptr[offset + 0x06000] & bit) {
						dat |= 0x040;
					}
					if (vramptr[offset + 0x10000] & bit) {
						dat |= 0x020;
					}
					if (vramptr[offset + 0x12000] & bit) {
						dat |= 0x010;
					}

					/* B�]�� */
					if (vramptr[offset + 0x00000] & bit) {
						dat |= 0x008;
					}
					if (vramptr[offset + 0x02000] & bit) {
						dat |= 0x004;
					}
					if (vramptr[offset + 0x0c000] & bit) {
						dat |= 0x002;
					}
					if (vramptr[offset + 0x0e000] & bit) {
						dat |= 0x001;
					}

					/* �A�i���O�p���b�g���f�[�^�擾 */
					dat &= mask;
					color = apalet_r[dat];
					color <<= 1;
					if (apalet_r[dat] > 0) {
						color |= 1;
					}
					color <<= 4;

					color |= apalet_g[dat];
					color <<= 1;
					if (apalet_g[dat] > 0) {
						color |= 1;
					}
					color <<= 4;

					color |= apalet_b[dat];
					color <<= 1;
					if (apalet_b[dat] > 0) {
						color |= 1;
					}
				}
#endif

				/* CRT�t���O */
				if (!crt_flag) {
					color = 0;
				}

				/* �Q�񑱂��ē������̂��������� */
				buffer[1][x * 32 + i * 4 + 0] = (BYTE)(color & 255);
				buffer[1][x * 32 + i * 4 + 1] = (BYTE)(color >> 8);
				buffer[1][x * 32 + i * 4 + 2] = (BYTE)(color & 255);
				buffer[1][x * 32 + i * 4 + 3] = (BYTE)(color >> 8);

				/* ���̃r�b�g�� */
				bit >>= 1;
			}
			offset++;
		}

		/* �t���X�L������� */
		if (fullscan) {
			memcpy(buffer[0], buffer[1], sizeof(buffer[1]));
		}

		/* �������� */
		if (!file_write(fileh, (BYTE *)buffer, sizeof(buffer))) {
			return FALSE;
		}

		/* ����y��(�߂�) */
		offset -= (40 * 2);
	}

	return TRUE;
}
#endif

#if XM7_VER >= 3
/*
 *  BMP�f�[�^��������(26���F���[�h)
 */
static BOOL FASTCALL bmp_256k_sub(int fileh, BOOL fullscan)
{
	int x, y;
	int offset;
	int i;
	BYTE buffer[2][1920];
	BYTE bit;
	BYTE r, g, b;
	
	ASSERT(fileh >= 0);

	/* �����I�t�Z�b�g�ݒ� */
	offset = 40 * 199;

	/* �O�ŏ������� */
	memset(buffer[0], 0, sizeof(buffer[0]));

	/* y���[�v */
	for (y=0; y<200; y++) {

		/* x���[�v */
		for (x=0; x<40; x++) {
			bit = 0x80;
			/* �r�b�g���[�v */
			for (i=0; i<8; i++) {
				r = g = b = 0;

				if (!(multi_page & 0x40)) {
					/* G�]�� */
					if (vram_c[offset + 0x10000] & bit) {
						g |= 0x20;
					}
					if (vram_c[offset + 0x12000] & bit) {
						g |= 0x10;
					}
					if (vram_c[offset + 0x14000] & bit) {
						g |= 0x08;
					}
					if (vram_c[offset + 0x16000] & bit) {
						g |= 0x04;
					}
					if (vram_c[offset + 0x28000] & bit) {
						g |= 0x02;
					}
					if (vram_c[offset + 0x2a000] & bit) {
						g |= 0x01;
					}
				}

				if (!(multi_page & 0x20)) {
					/* R�]�� */
					if (vram_c[offset + 0x08000] & bit) {
						r |= 0x20;
					}
					if (vram_c[offset + 0x0a000] & bit) {
						r |= 0x10;
					}
					if (vram_c[offset + 0x0c000] & bit) {
						r |= 0x08;
					}
					if (vram_c[offset + 0x0e000] & bit) {
						r |= 0x04;
					}
					if (vram_c[offset + 0x20000] & bit) {
						r |= 0x02;
					}
					if (vram_c[offset + 0x22000] & bit) {
						r |= 0x01;
					}
				}

				if (!(multi_page & 0x10)) {
					/* B�]�� */
					if (vram_c[offset + 0x00000] & bit) {
						b |= 0x20;
					}
					if (vram_c[offset + 0x02000] & bit) {
						b |= 0x10;
					}
					if (vram_c[offset + 0x04000] & bit) {
						b |= 0x08;
					}
					if (vram_c[offset + 0x06000] & bit) {
						b |= 0x04;
					}
					if (vram_c[offset + 0x18000] & bit) {
						b |= 0x02;
					}
					if (vram_c[offset + 0x1a000] & bit) {
						b |= 0x01;
					}
				}

				/* CRT�t���O */
				if (!crt_flag) {
					r = g = b = 0;
				}

				/* �Q�񑱂��ē������̂��������� */
				buffer[1][x * 48 + i * 6 + 0] = (BYTE)truecolorbrightness[b];
				buffer[1][x * 48 + i * 6 + 1] = (BYTE)truecolorbrightness[g];
				buffer[1][x * 48 + i * 6 + 2] = (BYTE)truecolorbrightness[r];
				buffer[1][x * 48 + i * 6 + 3] = (BYTE)truecolorbrightness[b];
				buffer[1][x * 48 + i * 6 + 4] = (BYTE)truecolorbrightness[g];
				buffer[1][x * 48 + i * 6 + 5] = (BYTE)truecolorbrightness[r];

				/* ���̃r�b�g�� */
				bit >>= 1;
			}
			offset++;
		}

		/* �t���X�L������� */
		if (fullscan) {
			memcpy(buffer[0], buffer[1], sizeof(buffer[1]));
		}

		/* �������� */
		if (!file_write(fileh, (BYTE *)buffer, sizeof(buffer))) {
			return FALSE;
		}

		/* ����y��(�߂�) */
		offset -= (40 * 2);
	}

	return TRUE;
}
#endif

/*
 *  BMP�f�[�^��������(640���[�h)
 */
static BOOL FASTCALL bmp_640_sub(int fileh, BOOL fullscan)
{
	int x, y;
	int i;
	int offset;
	BYTE bit;
	BYTE buffer[2][320];
	BYTE *vramptr;
#if XM7_VER >= 3
	WORD dx1, dx2;
	BOOL winy;
#endif

	ASSERT(fileh >= 0);

	/* �����I�t�Z�b�g�ݒ� */
	offset = 80 * 199;

#if XM7_VER >= 3
	/* �E�B���h�E�̈�̃N���b�s���O���s�� */
	window_clip(0);
	dx1 = (WORD)(window_dx1 >> 3);
	dx2 = (WORD)(window_dx2 >> 3);

	/* �J���[9�ŏ������� */
	memset(buffer[0], 0x99, sizeof(buffer[0]));

	/* y���[�v */
	for (y=0; y<200; y++) {
		winy = (((199 - y) >= window_dy1) && ((199 - y) <= window_dy2));

		/* ��U�N���A */
		memset(buffer[1], 0, sizeof(buffer[1]));

		/* x���[�v */
		for (x=0; x<80; x++) {
			bit = 0x80;
			if (winy && (x >= dx1) && (x < dx2)) {
				/* �E�B���h�E��(���u���b�N) */
				vramptr = vram_bdptr;
			}
			else {
				/* �E�B���h�E�O(�\�u���b�N) */
				vramptr = vram_dptr;
			}
#else
	/* �J���[9�ŏ������� */
	memset(buffer[0], 0x99, sizeof(buffer[0]));

	/* y���[�v */
	for (y=0; y<200; y++) {

		/* ��U�N���A */
		memset(buffer[1], 0, sizeof(buffer[1]));

		/* x���[�v */
		for (x=0; x<80; x++) {
			bit = 0x80;
#if XM7_VER >= 2
			vramptr = vram_dptr;
#else
			vramptr = vram_c;
#endif
#endif

			/* bit���[�v */
			for (i=0; i<4; i++) {
#if XM7_VER >= 3
				if (vramptr[offset + 0x00000] & bit) {
					buffer[1][x * 4 + i] |= 0x10;
				}
				if (vramptr[offset + 0x08000] & bit) {
					buffer[1][x * 4 + i] |= 0x20;
				}
				if (vramptr[offset + 0x10000] & bit) {
					buffer[1][x * 4 + i] |= 0x40;
				}
				bit >>= 1;

				if (vramptr[offset + 0x00000] & bit) {
					buffer[1][x * 4 + i] |= 0x01;
				}
				if (vramptr[offset + 0x08000] & bit) {
					buffer[1][x * 4 + i] |= 0x02;
				}
				if (vramptr[offset + 0x10000] & bit) {
					buffer[1][x * 4 + i] |= 0x04;
				}
				bit >>= 1;
#else
				if (vramptr[offset + 0x0000] & bit) {
					buffer[1][x * 4 + i] |= 0x10;
				}
				if (vramptr[offset + 0x4000] & bit) {
					buffer[1][x * 4 + i] |= 0x20;
				}
				if (vramptr[offset + 0x8000] & bit) {
					buffer[1][x * 4 + i] |= 0x40;
				}
				bit >>= 1;

				if (vramptr[offset + 0x0000] & bit) {
					buffer[1][x * 4 + i] |= 0x01;
				}
				if (vramptr[offset + 0x4000] & bit) {
					buffer[1][x * 4 + i] |= 0x02;
				}
				if (vramptr[offset + 0x8000] & bit) {
					buffer[1][x * 4 + i] |= 0x04;
				}
				bit >>= 1;
#endif
			}
			offset++;
		}

		/* �t���X�L������� */
		if (fullscan) {
			memcpy(buffer[0], buffer[1], sizeof(buffer[1]));
		}

		/* �������� */
		if (!file_write(fileh, (BYTE *)buffer, sizeof(buffer))) {
			return FALSE;
		}

		/* ����y��(�߂�) */
		offset -= (80 * 2);
	}

	return TRUE;
}

/*
 *  BMP�f�[�^��������(640���[�h,�J���[�^��400���C��)
 */
#if XM7_VER >= 2
static BOOL FASTCALL bmp_p400c_sub(int fileh)
{
	int x, y;
	int i;
	int offset;
	BYTE bit;
	BYTE buffer[2][320];
	BYTE *vramptr;
#if XM7_VER >= 3
	WORD dx1, dx2;
	BOOL winy;
#endif

	ASSERT(fileh >= 0);

	/* �����I�t�Z�b�g�ݒ� */
	offset = 80 * 199;

#if XM7_VER >= 3
	/* �E�B���h�E�̈�̃N���b�s���O���s�� */
	window_clip(0);
	dx1 = (WORD)(window_dx1 >> 3);
	dx2 = (WORD)(window_dx2 >> 3);

	/* y���[�v */
	for (y=0; y<200; y++) {
		winy = (((199 - y) >= window_dy1) && ((199 - y) <= window_dy2));

		/* ��U�N���A */
		memset(buffer, 0, sizeof(buffer));

		/* x���[�v */
		for (x=0; x<80; x++) {
			bit = 0x80;
			if (winy && (x >= dx1) && (x < dx2)) {
				/* �E�B���h�E��(���u���b�N) */
				vramptr = vram_bdblk;
			}
			else {
				/* �E�B���h�E�O(�\�u���b�N) */
				vramptr = vram_dblk;
			}
#else
	/* y���[�v */
	for (y=0; y<200; y++) {

		/* ��U�N���A */
		memset(buffer, 0, sizeof(buffer));

		/* x���[�v */
		for (x=0; x<80; x++) {
			bit = 0x80;
			vramptr = vram_c;
#endif

			/* bit���[�v */
			for (i=0; i<4; i++) {
#if XM7_VER >= 3
				if (vramptr[offset + 0x00000] & bit) {
					buffer[1][x * 4 + i] |= 0x10;
				}
				if (vramptr[offset + 0x08000] & bit) {
					buffer[1][x * 4 + i] |= 0x20;
				}
				if (vramptr[offset + 0x10000] & bit) {
					buffer[1][x * 4 + i] |= 0x40;
				}
				if (vramptr[offset + 0x04000] & bit) {
					buffer[0][x * 4 + i] |= 0x10;
				}
				if (vramptr[offset + 0x0c000] & bit) {
					buffer[0][x * 4 + i] |= 0x20;
				}
				if (vramptr[offset + 0x14000] & bit) {
					buffer[0][x * 4 + i] |= 0x40;
				}
				bit >>= 1;

				if (vramptr[offset + 0x00000] & bit) {
					buffer[1][x * 4 + i] |= 0x01;
				}
				if (vramptr[offset + 0x08000] & bit) {
					buffer[1][x * 4 + i] |= 0x02;
				}
				if (vramptr[offset + 0x10000] & bit) {
					buffer[1][x * 4 + i] |= 0x04;
				}
				if (vramptr[offset + 0x04000] & bit) {
					buffer[0][x * 4 + i] |= 0x01;
				}
				if (vramptr[offset + 0x0c000] & bit) {
					buffer[0][x * 4 + i] |= 0x02;
				}
				if (vramptr[offset + 0x14000] & bit) {
					buffer[0][x * 4 + i] |= 0x04;
				}
				bit >>= 1;
#else
				if (vramptr[offset + 0x00000] & bit) {
					buffer[1][x * 4 + i] |= 0x10;
				}
				if (vramptr[offset + 0x04000] & bit) {
					buffer[1][x * 4 + i] |= 0x20;
				}
				if (vramptr[offset + 0x08000] & bit) {
					buffer[1][x * 4 + i] |= 0x40;
				}
				if (vramptr[offset + 0x0c000] & bit) {
					buffer[0][x * 4 + i] |= 0x10;
				}
				if (vramptr[offset + 0x10000] & bit) {
					buffer[0][x * 4 + i] |= 0x20;
				}
				if (vramptr[offset + 0x14000] & bit) {
					buffer[0][x * 4 + i] |= 0x40;
				}
				bit >>= 1;

				if (vramptr[offset + 0x00000] & bit) {
					buffer[1][x * 4 + i] |= 0x01;
				}
				if (vramptr[offset + 0x04000] & bit) {
					buffer[1][x * 4 + i] |= 0x02;
				}
				if (vramptr[offset + 0x08000] & bit) {
					buffer[1][x * 4 + i] |= 0x04;
				}
				if (vramptr[offset + 0x0c000] & bit) {
					buffer[0][x * 4 + i] |= 0x01;
				}
				if (vramptr[offset + 0x10000] & bit) {
					buffer[0][x * 4 + i] |= 0x02;
				}
				if (vramptr[offset + 0x14000] & bit) {
					buffer[0][x * 4 + i] |= 0x04;
				}
				bit >>= 1;
#endif
			}
			offset++;
		}

		/* �������� */
		if (!file_write(fileh, (BYTE *)buffer, sizeof(buffer))) {
			return FALSE;
		}

		/* ����y��(�߂�) */
		offset -= (80 * 2);
	}

	return TRUE;
}
#endif

#if XM7_VER == 1
/*
 *  BMP�f�[�^��������(640���[�h,���m�N���^��400���C��)
 */
static BOOL FASTCALL bmp_p400m_sub(int fileh)
{
	int x, y;
	int i;
	int offset;
	BYTE bit;
	BYTE buffer[2][320];
	BYTE pal[8];
	BYTE *vramptr;
	BYTE col1, col2, col3, col4;

	ASSERT(fileh >= 0);

	/* �F�f�[�^���� */
	for (i=0; i<8; i++) {
		if (crt_flag) {
			pal[i] = (BYTE)(ttl_palet[i & ((~(multi_page >> 4)) & 7)] & 7);
		}
		else {
			pal[i] = 0;
		}
	}

	/* �����I�t�Z�b�g�ݒ� */
	offset = 80 * 199;

	/* �J���[0�ŏ������� */
	memset(buffer[0], 0x00, sizeof(buffer[0]));

	/* y���[�v */
	for (y=0; y<200; y++) {

		/* ��U�N���A */
		memset(buffer[1], 0, sizeof(buffer[1]));

		/* x���[�v */
		for (x=0; x<80; x++) {
			bit = 0x80;
			vramptr = vram_c;

			/* bit���[�v */
			for (i=0; i<4; i++) {
				col1 = 0;
				if (vramptr[offset + 0x0000] & bit) {
					col1 |= 0x01;
				}
				if (vramptr[offset + 0x4000] & bit) {
					col1 |= 0x02;
				}
				if (vramptr[offset + 0x8000] & bit) {
					col1 |= 0x04;
				}
				col1 = pal[col1];
				bit >>= 1;

				col2 = 0;
				if (vramptr[offset + 0x0000] & bit) {
					col2 |= 0x01;
				}
				if (vramptr[offset + 0x4000] & bit) {
					col2 |= 0x02;
				}
				if (vramptr[offset + 0x8000] & bit) {
					col2 |= 0x04;
				}
				col2 = pal[col2];
				bit >>= 1;

				if (col1 & 4) {
					col3 = 7;
				}
				else {
					col3 = 0;
				}
				if (col1 & 2) {
					col1 = 7;
				}
				else {
					col1 = 0;
				}
				if (col2 & 4) {
					col4 = 7;
				}
				else {
					col4 = 0;
				}
				if (col2 & 2) {
					col2 = 7;
				}
				else {
					col2 = 0;
				}

				buffer[0][x * 4 + i] = (BYTE)((col1 << 4) | col2);
				buffer[1][x * 4 + i] = (BYTE)((col3 << 4) | col4);
			}

			offset++;
		}

		/* �������� */
		if (!file_write(fileh, (BYTE *)buffer, sizeof(buffer))) {
			return FALSE;
		}

		/* ����y��(�߂�) */
		offset -= (80 * 2);
	}

	return TRUE;
}
#endif

#if XM7_VER >= 3
/*
 *  BMP�f�[�^��������(400���C�����[�h)
 */
static BOOL FASTCALL bmp_400l_sub(int fileh)
{
	int x, y;
	int i;
	int offset;
	BYTE bit;
	BYTE buffer[320];
	BYTE *vramptr;
	WORD dx1, dx2;
	BOOL winy;

	ASSERT(fileh >= 0);

	/* �����I�t�Z�b�g�ݒ� */
	offset = 80 * 399;

	/* �E�B���h�E�̈�̃N���b�s���O���s�� */
	window_clip(2);
	dx1 = (WORD)(window_dx1 >> 3);
	dx2 = (WORD)(window_dx2 >> 3);

	/* y���[�v */
	for (y=0; y<400; y++) {
		winy = (((399 - y) >= window_dy1) && ((399 - y) <= window_dy2));

		/* ��U�N���A */
		memset(buffer, 0, sizeof(buffer));

		/* x���[�v */
		for (x=0; x<80; x++) {
			bit = 0x80;
			if (winy && (x >= dx1) && (x < dx2)) {
				/* �E�B���h�E��(���u���b�N) */
				vramptr = vram_bdptr;
			}
			else {
				/* �E�B���h�E�O(�\�u���b�N) */
				vramptr = vram_dptr;
			}

			/* bit���[�v */
			for (i=0; i<4; i++) {
				if (vramptr[offset + 0x00000] & bit) {
					buffer[x * 4 + i] |= 0x10;
				}
				if (vramptr[offset + 0x08000] & bit) {
					buffer[x * 4 + i] |= 0x20;
				}
				if (vramptr[offset + 0x10000] & bit) {
					buffer[x * 4 + i] |= 0x40;
				}
				bit >>= 1;

				if (vramptr[offset + 0x00000] & bit) {
					buffer[x * 4 + i] |= 0x01;
				}
				if (vramptr[offset + 0x08000] & bit) {
					buffer[x * 4 + i] |= 0x02;
				}
				if (vramptr[offset + 0x10000] & bit) {
					buffer[x * 4 + i] |= 0x04;
				}
				bit >>= 1;
			}
			offset++;
		}

		/* �������� */
		if (!file_write(fileh, buffer, sizeof(buffer))) {
			return FALSE;
		}

		/* ����y��(�߂�) */
		offset -= (80 * 2);
	}

	return TRUE;
}
#endif

#if XM7_VER == 1 && defined(L4CARD)
/*
 *  BMP�f�[�^��������(L4 400���C�����[�h)
 */
static BOOL FASTCALL bmp_400l4_sub(int fileh, BOOL mode)
{
	int x, y;
	int i;
	int offset;
	BYTE bit, bit2;
	BYTE buffer[320];
	WORD taddr, gaddr, textbase;
	BYTE csr_st, csr_ed, csr_type;
	BYTE raster, lines;
	BYTE col, chr, atr, chr_dat;
	BOOL enable_page;

	ASSERT(fileh >= 0);
	UNUSED(mode);

	/* �����I�t�Z�b�g�ݒ� */
	offset = 80 * 399;

	/* �e�L�X�g�W�J �����ݒ�(�S��) */
	csr_st = (BYTE)(crtc_register[10] & 0x1f);
	csr_ed = (BYTE)(crtc_register[11] & 0x1f);
	csr_type = (BYTE)((crtc_register[10] & 0x60) >> 5);
	lines = (BYTE)((crtc_register[9] & 0x1f) + 1);

	/* y���[�v */
	for (y=0; y<400; y++) {
		/* ��U�N���A */
		memset(buffer, 0, sizeof(buffer));

		/* �e�L�X�g�W�J �����ݒ�(���X�^�P��) */
		textbase = (WORD)text_start_addr;
		textbase += (WORD)(((399 - y) / lines) * (crtc_register[1] << 2));
		textbase &= 0xffe;
		raster = (BYTE)((399 - y) % lines);

		/* x���[�v */
		for (x=0; x<80; x++) {
			bit = 0x80;
			if (!width40_flag || !(x & 1)) {
				bit2 = 0x80;

				/* �L�����N�^�R�[�h�E�A�g���r���[�g���擾 */
				if (width40_flag) {
					taddr = (WORD)((textbase + (x & ~1)) & 0xffe);
				}
				else {
					taddr = (WORD)((textbase + (x << 1)) & 0xffe);
				}
				chr = tvram_c[taddr + 0];
				atr = tvram_c[taddr + 1];

				/* �A�g���r���[�g����`��F��ݒ� */
				col = (BYTE)((atr & 0x07) | ((atr & 0x20) >> 2));

				/* �t�H���g�f�[�^�擾(�A�g���r���[�g/�J�[�\���������܂�) */
				if ((!(atr & 0x10) || text_blink) && (raster < 16)) {
					chr_dat = subcg_l4[(WORD)(chr << 4) + raster];
				}
				else {
					chr_dat = 0x00;
				}
				if (atr & 0x08) {
					chr_dat ^= (BYTE)0xff;
				}
				if (csr_type != 1) {
					if (((taddr == cursor_addr) &&
						(cursor_blink || !csr_type)) &&
						((raster >= csr_st) && (raster <= csr_ed))) {
						chr_dat ^= (BYTE)0xff;
					}
				}
			}

			/* GVRAM ���A�h���X���擾 */
			gaddr = (WORD)((offset + vram_offset[0]) & 0x7fff);
			enable_page = FALSE;
			if (gaddr >= 0x4000) {
				if (!(multi_page & 0x10)) {
					enable_page = TRUE;
				}
			}
			else {
				if (!(multi_page & 0x20)) {
					enable_page = TRUE;
				}
			}

			/* bit���[�v */
			for (i=0; i<4; i++) {
				if (chr_dat & bit2) {
					buffer[x * 4 + i] |= (BYTE)(col << 4);
				}
				else if ((vram_c[gaddr] & bit) && enable_page) {
					buffer[x * 4 + i] |= (BYTE)(ttl_palet[1] << 4);
				}
				else {
					buffer[x * 4 + i] |= (BYTE)(ttl_palet[0] << 4);
				}
				bit >>= 1;
				if (!width40_flag) {
					bit2 >>= 1;
				}

				if (chr_dat & bit2) {
					buffer[x * 4 + i] |= (BYTE)col;
				}
				else if ((vram_c[gaddr] & bit) && enable_page) {
					buffer[x * 4 + i] |= (BYTE)ttl_palet[1];
				}
				else {
					buffer[x * 4 + i] |= (BYTE)ttl_palet[0];
				}
				bit >>= 1;
				bit2 >>= 1;
			}
			offset++;
		}

		/* �������� */
		if (!file_write(fileh, buffer, sizeof(buffer))) {
			return FALSE;
		}

		/* ����y��(�߂�) */
		offset -= (80 * 2);
	}

	return TRUE;
}
#endif

/*
 *  BMP�f�[�^��������(320���[�h�E�k���摜)
 */
#if XM7_VER >= 2
static BOOL FASTCALL bmp_320_sub2(int fileh, BOOL mode)
{
	int x, y;
	int offset;
	int i;
	BYTE buffer[640];
	int dat;
	BYTE bit;
	WORD color;
	int mask;
	BYTE *vramptr;
#if XM7_VER >= 3
	WORD dx1,dx2;
	BOOL winy;
#endif

	ASSERT(fileh >= 0);
#if XM7_VER >= 3
	UNUSED(mode);
#endif

	/* �����I�t�Z�b�g�ݒ� */
	offset = 40 * 199;

#if XM7_VER >= 3
	/* �E�B���h�E�̈�̃N���b�s���O���s�� */
	window_clip(1);
	dx1 = (WORD)(window_dx1 >> 3);
	dx2 = (WORD)(window_dx2 >> 3);
#endif

	/* �}�X�N�擾 */
	mask = 0;
#if XM7_VER == 2
	if (mode) {
		mask = ((~multi_page) >> 4) & 0x07;
	}
	else {
		if (!(multi_page & 0x10)) {
			mask |= 0x000f;
		}
		if (!(multi_page & 0x20)) {
			mask |= 0x00f0;
		}
		if (!(multi_page & 0x40)) {
			mask |= 0x0f00;
		}
	}
#else
	if (!(multi_page & 0x10)) {
		mask |= 0x000f;
	}
	if (!(multi_page & 0x20)) {
		mask |= 0x00f0;
	}
	if (!(multi_page & 0x40)) {
		mask |= 0x0f00;
	}
#endif

	/* y���[�v */
	for (y=0; y<200; y++) {
#if XM7_VER >= 3
		winy = (((199 - y) >= window_dy1) && ((199 - y) <= window_dy2));

		/* x���[�v */
		for (x=0; x<40; x++) {
			bit = 0x80;
			if (winy && (x >= dx1) && (x < dx2)) {
				/* �E�B���h�E��(���u���b�N) */
				vramptr = vram_bdptr;
			}
			else {
				/* �E�B���h�E�O(�\�u���b�N) */
				vramptr = vram_dptr;
			}
#else
		/* x���[�v */
		for (x=0; x<40; x++) {
			bit = 0x80;
			vramptr = vram_dptr;
#endif

			/* �r�b�g���[�v */
			for (i=0; i<8; i++) {
				dat = 0;

#if XM7_VER >= 3
				/* G�]�� */
				if (vramptr[offset + 0x10000] & bit) {
					dat |= 0x800;
				}
				if (vramptr[offset + 0x12000] & bit) {
					dat |= 0x400;
				}
				if (vramptr[offset + 0x14000] & bit) {
					dat |= 0x200;
				}
				if (vramptr[offset + 0x16000] & bit) {
					dat |= 0x100;
				}

				/* R�]�� */
				if (vramptr[offset + 0x08000] & bit) {
					dat |= 0x080;
				}
				if (vramptr[offset + 0x0a000] & bit) {
					dat |= 0x040;
				}
				if (vramptr[offset + 0x0c000] & bit) {
					dat |= 0x020;
				}
				if (vramptr[offset + 0x0e000] & bit) {
					dat |= 0x010;
				}

				/* B�]�� */
				if (vramptr[offset + 0x00000] & bit) {
					dat |= 0x008;
				}
				if (vramptr[offset + 0x02000] & bit) {
					dat |= 0x004;
				}
				if (vramptr[offset + 0x04000] & bit) {
					dat |= 0x002;
				}
				if (vramptr[offset + 0x06000] & bit) {
					dat |= 0x001;
				}

				/* �A�i���O�p���b�g���f�[�^�擾 */
				dat &= mask;
				color = apalet_r[dat];
				color <<= 1;
				if (apalet_r[dat] > 0) {
					color |= 1;
				}
				color <<= 4;

				color |= apalet_g[dat];
				color <<= 1;
				if (apalet_g[dat] > 0) {
					color |= 1;
				}
				color <<= 4;

				color |= apalet_b[dat];
				color <<= 1;
				if (apalet_b[dat] > 0) {
					color |= 1;
				}
#else
				if (mode) {
					/* G�]�� */
					if (vramptr[offset + 0x08000] & bit) {
						dat |= 0x04;
					}

					/* R�]�� */
					if (vramptr[offset + 0x04000] & bit) {
						dat |= 0x02;
					}

					/* B�]�� */
					if (vramptr[offset + 0x00000] & bit) {
						dat |= 0x01;
					}

					/* TTL�p���b�g���f�[�^�擾 */
					color = bmp_palet_table_ttl[ttl_palet[dat & mask] & 7];
				}
				else {
					/* G�]�� */
					if (vramptr[offset + 0x08000] & bit) {
						dat |= 0x800;
					}
					if (vramptr[offset + 0x0a000] & bit) {
						dat |= 0x400;
					}
					if (vramptr[offset + 0x14000] & bit) {
						dat |= 0x200;
					}
					if (vramptr[offset + 0x16000] & bit) {
						dat |= 0x100;
					}

					/* R�]�� */
					if (vramptr[offset + 0x04000] & bit) {
						dat |= 0x080;
					}
					if (vramptr[offset + 0x06000] & bit) {
						dat |= 0x040;
					}
					if (vramptr[offset + 0x10000] & bit) {
						dat |= 0x020;
					}
					if (vramptr[offset + 0x12000] & bit) {
						dat |= 0x010;
					}

					/* B�]�� */
					if (vramptr[offset + 0x00000] & bit) {
						dat |= 0x008;
					}
					if (vramptr[offset + 0x02000] & bit) {
						dat |= 0x004;
					}
					if (vramptr[offset + 0x0c000] & bit) {
						dat |= 0x002;
					}
					if (vramptr[offset + 0x0e000] & bit) {
						dat |= 0x001;
					}

					/* �A�i���O�p���b�g���f�[�^�擾 */
					dat &= mask;
					color = apalet_r[dat];
					color <<= 1;
					if (apalet_r[dat] > 0) {
						color |= 1;
					}
					color <<= 4;

					color |= apalet_g[dat];
					color <<= 1;
					if (apalet_g[dat] > 0) {
						color |= 1;
					}
					color <<= 4;

					color |= apalet_b[dat];
					color <<= 1;
					if (apalet_b[dat] > 0) {
						color |= 1;
					}
				}
#endif

				/* CRT�t���O */
				if (!crt_flag) {
					color = 0;
				}

				buffer[x * 16 + i * 2 + 0] = (BYTE)(color & 255);
				buffer[x * 16 + i * 2 + 1] = (BYTE)(color >> 8);

				/* ���̃r�b�g�� */
				bit >>= 1;
			}
			offset++;
		}

		/* �������� */
		if (!file_write(fileh, buffer, sizeof(buffer))) {
			return FALSE;
		}

		/* ����y��(�߂�) */
		offset -= (40 * 2);
	}

	return TRUE;
}
#endif

#if XM7_VER >= 3
/*
 *  BMP�f�[�^��������(26���F���[�h�E�k���摜)
 */
static BOOL FASTCALL bmp_256k_sub2(int fileh)
{
	int x, y;
	int offset;
	int i;
	BYTE buffer[960];
	BYTE bit;
	BYTE r, g, b;

	ASSERT(fileh >= 0);

	/* �����I�t�Z�b�g�ݒ� */
	offset = 40 * 199;

	/* y���[�v */
	for (y=0; y<200; y++) {

		/* x���[�v */
		for (x=0; x<40; x++) {
			bit = 0x80;
			/* �r�b�g���[�v */
			for (i=0; i<8; i++) {
				r = g = b = 0;

				if (!(multi_page & 0x40)) {
					/* G�]�� */
					if (vram_c[offset + 0x10000] & bit) {
						g |= 0x20;
					}
					if (vram_c[offset + 0x12000] & bit) {
						g |= 0x10;
					}
					if (vram_c[offset + 0x14000] & bit) {
						g |= 0x08;
					}
					if (vram_c[offset + 0x16000] & bit) {
						g |= 0x04;
					}
					if (vram_c[offset + 0x28000] & bit) {
						g |= 0x02;
					}
					if (vram_c[offset + 0x2a000] & bit) {
						g |= 0x01;
					}
				}

				if (!(multi_page & 0x20)) {
					/* R�]�� */
					if (vram_c[offset + 0x08000] & bit) {
						r |= 0x20;
					}
					if (vram_c[offset + 0x0a000] & bit) {
						r |= 0x10;
					}
					if (vram_c[offset + 0x0c000] & bit) {
						r |= 0x08;
					}
					if (vram_c[offset + 0x0e000] & bit) {
						r |= 0x04;
					}
					if (vram_c[offset + 0x20000] & bit) {
						r |= 0x02;
					}
					if (vram_c[offset + 0x22000] & bit) {
						r |= 0x01;
					}
				}

				if (!(multi_page & 0x10)) {
					/* B�]�� */
					if (vram_c[offset + 0x00000] & bit) {
						b |= 0x20;
					}
					if (vram_c[offset + 0x02000] & bit) {
						b |= 0x10;
					}
					if (vram_c[offset + 0x04000] & bit) {
						b |= 0x08;
					}
					if (vram_c[offset + 0x06000] & bit) {
						b |= 0x04;
					}
					if (vram_c[offset + 0x18000] & bit) {
						b |= 0x02;
					}
					if (vram_c[offset + 0x1a000] & bit) {
						b |= 0x01;
					}
				}

				/* CRT�t���O */
				if (!crt_flag) {
					r = g = b = 0;
				}

				buffer[x * 24 + i * 3 + 0] = (BYTE)truecolorbrightness[b];
				buffer[x * 24 + i * 3 + 1] = (BYTE)truecolorbrightness[g];
				buffer[x * 24 + i * 3 + 2] = (BYTE)truecolorbrightness[r];

				/* ���̃r�b�g�� */
				bit >>= 1;
			}
			offset++;
		}

		/* �������� */
		if (!file_write(fileh, buffer, sizeof(buffer))) {
			return FALSE;
		}

		/* ����y��(�߂�) */
		offset -= (40 * 2);
	}

	return TRUE;
}
#endif

/*
 *  BMP�f�[�^��������(640���[�h�E�k���摜)
 */
static BOOL FASTCALL bmp_640_sub2(int fileh, BOOL mode)
{
	int x, y;
	int i;
	int offset;
	BYTE bit;
	BYTE buffer[640];
	BYTE *vramptr;
	BYTE pal[8];
	BYTE col[2];
	WORD color;
#if XM7_VER >= 3
	WORD dx1, dx2;
	BOOL winy;
#endif

	ASSERT(fileh >= 0);

	/* �F�����e�[�u�������� */
	mix_color_init(GAMMA200L);

	/* �F�f�[�^���� */
	for (i=0; i<8; i++) {
		if (crt_flag) {
			pal[i] = (BYTE)(ttl_palet[i & ((~(multi_page >> 4)) & 7)] & 7);
		}
		else {
			pal[i] = 0;
		}
	}

	/* �����I�t�Z�b�g�ݒ� */
	offset = 80 * 199;

#if XM7_VER >= 3
	/* �E�B���h�E�̈�̃N���b�s���O���s�� */
	window_clip(0);
	dx1 = (WORD)(window_dx1 >> 3);
	dx2 = (WORD)(window_dx2 >> 3);

	/* y���[�v */
	for (y=0; y<200; y++) {
		winy = (((199 - y) >= window_dy1) && ((199 - y) <= window_dy2));

		/* x���[�v */
		for (x=0; x<80; x++) {
			bit = 0x80;
			if (winy && (x >= dx1) && (x < dx2)) {
				/* �E�B���h�E��(���u���b�N) */
				vramptr = vram_bdptr;
			}
			else {
				/* �E�B���h�E�O(�\�u���b�N) */
				vramptr = vram_dptr;
			}
#else
	/* y���[�v */
	for (y=0; y<200; y++) {

		/* x���[�v */
		for (x=0; x<80; x++) {
			bit = 0x80;
#if XM7_VER >= 2
			vramptr = vram_dptr;
#else
			vramptr = vram_c;
#endif
#endif

			/* bit���[�v */
			for (i=0; i<4; i++) {
				col[0] = 0;
				col[1] = 0;

#if XM7_VER >= 3
				if (vramptr[offset + 0x00000] & bit) {
					col[0] |= 1;
				}
				if (vramptr[offset + 0x08000] & bit) {
					col[0] |= 2;
				}
				if (vramptr[offset + 0x10000] & bit) {
					col[0] |= 4;
				}
				bit >>= 1;

				if (vramptr[offset + 0x00000] & bit) {
					col[1] |= 1;
				}
				if (vramptr[offset + 0x08000] & bit) {
					col[1] |= 2;
				}
				if (vramptr[offset + 0x10000] & bit) {
					col[1] |= 4;
				}
				bit >>= 1;
#else
				if (vramptr[offset + 0x0000] & bit) {
					col[0] |= 1;
				}
				if (vramptr[offset + 0x4000] & bit) {
					col[0] |= 2;
				}
				if (vramptr[offset + 0x8000] & bit) {
					col[0] |= 4;
				}
				bit >>= 1;

				if (vramptr[offset + 0x0000] & bit) {
					col[1] |= 1;
				}
				if (vramptr[offset + 0x4000] & bit) {
					col[1] |= 2;
				}
				if (vramptr[offset + 0x8000] & bit) {
					col[1] |= 4;
				}
				bit >>= 1;
#endif

				col[0] = pal[col[0]];
				col[1] = pal[col[1]];
				color = mix_color(col, 2, mode);
				buffer[x * 8 + i * 2 + 0] = (BYTE)(color & 255);
				buffer[x * 8 + i * 2 + 1] = (BYTE)(color >> 8);
			}
			offset++;
		}

		/* �������� */
		if (!file_write(fileh, buffer, sizeof(buffer))) {
			return FALSE;
		}

		/* ����y��(�߂�) */
		offset -= (80 * 2);
	}

	return TRUE;
}
/*
 *  BMP�f�[�^��������(640���[�h�E�J���[�^��400���C���E�k���摜)
 */
#if XM7_VER >= 2
static BOOL FASTCALL bmp_p400c_sub2(int fileh)
{
	int x, y;
	int i;
	int offset;
	BYTE bit;
	BYTE buffer[640];
	BYTE *vramptr;
	BYTE pal[8];
	BYTE col[4];
	WORD color;
#if XM7_VER >= 3
	WORD dx1, dx2;
	BOOL winy;
#endif

	ASSERT(fileh >= 0);

	/* �F�����e�[�u�������� */
	mix_color_init(GAMMA400L);

	/* �F�f�[�^���� */
	for (i=0; i<8; i++) {
		if (crt_flag) {
			pal[i] = (BYTE)(ttl_palet[i & ((~(multi_page >> 4)) & 7)] & 7);
		}
		else {
			pal[i] = 0;
		}
	}

	/* �����I�t�Z�b�g�ݒ� */
	offset = 80 * 199;

#if XM7_VER >= 3
	/* �E�B���h�E�̈�̃N���b�s���O���s�� */
	window_clip(0);
	dx1 = (WORD)(window_dx1 >> 3);
	dx2 = (WORD)(window_dx2 >> 3);

	/* y���[�v */
	for (y=0; y<200; y++) {
		winy = (((199 - y) >= window_dy1) && ((199 - y) <= window_dy2));

		/* x���[�v */
		for (x=0; x<80; x++) {
			bit = 0x80;
			if (winy && (x >= dx1) && (x < dx2)) {
				/* �E�B���h�E��(���u���b�N) */
				vramptr = vram_bdblk;
			}
			else {
				/* �E�B���h�E�O(�\�u���b�N) */
				vramptr = vram_dblk;
			}
#else
	/* y���[�v */
	for (y=0; y<200; y++) {

		/* x���[�v */
		for (x=0; x<80; x++) {
			bit = 0x80;
			vramptr = vram_c;
#endif

			/* bit���[�v */
			for (i=0; i<4; i++) {
				col[0] = 0;
				col[1] = 0;
				col[2] = 0;
				col[3] = 0;

#if XM7_VER >= 3
				if (vramptr[offset + 0x00000] & bit) {
					col[0] |= 1;
				}
				if (vramptr[offset + 0x08000] & bit) {
					col[0] |= 2;
				}
				if (vramptr[offset + 0x10000] & bit) {
					col[0] |= 4;
				}
				if (vramptr[offset + 0x04000] & bit) {
					col[2] |= 1;
				}
				if (vramptr[offset + 0x0c000] & bit) {
					col[2] |= 2;
				}
				if (vramptr[offset + 0x14000] & bit) {
					col[2] |= 4;
				}
				bit >>= 1;

				if (vramptr[offset + 0x00000] & bit) {
					col[1] |= 1;
				}
				if (vramptr[offset + 0x08000] & bit) {
					col[1] |= 2;
				}
				if (vramptr[offset + 0x10000] & bit) {
					col[1] |= 4;
				}
				if (vramptr[offset + 0x04000] & bit) {
					col[3] |= 1;
				}
				if (vramptr[offset + 0x0c000] & bit) {
					col[3] |= 2;
				}
				if (vramptr[offset + 0x14000] & bit) {
					col[3] |= 4;
				}
				bit >>= 1;
#else
				if (vramptr[offset + 0x00000] & bit) {
					col[0] |= 1;
				}
				if (vramptr[offset + 0x04000] & bit) {
					col[0] |= 2;
				}
				if (vramptr[offset + 0x08000] & bit) {
					col[0] |= 4;
				}
				if (vramptr[offset + 0x0c000] & bit) {
					col[2] |= 1;
				}
				if (vramptr[offset + 0x10000] & bit) {
					col[2] |= 2;
				}
				if (vramptr[offset + 0x14000] & bit) {
					col[2] |= 4;
				}
				bit >>= 1;

				if (vramptr[offset + 0x0000] & bit) {
					col[1] |= 1;
				}
				if (vramptr[offset + 0x4000] & bit) {
					col[1] |= 2;
				}
				if (vramptr[offset + 0x8000] & bit) {
					col[1] |= 4;
				}
				if (vramptr[offset + 0x0c000] & bit) {
					col[3] |= 1;
				}
				if (vramptr[offset + 0x10000] & bit) {
					col[3] |= 2;
				}
				if (vramptr[offset + 0x14000] & bit) {
					col[3] |= 4;
				}
				bit >>= 1;
#endif

				col[0] = pal[col[0]];
				col[1] = pal[col[1]];
				col[2] = pal[col[2]];
				col[3] = pal[col[3]];
				color = mix_color(col, 4, FALSE);
				buffer[x * 8 + i * 2 + 0] = (BYTE)(color & 255);
				buffer[x * 8 + i * 2 + 1] = (BYTE)(color >> 8);
			}
			offset++;
		}

		/* �������� */
		if (!file_write(fileh, buffer, sizeof(buffer))) {
			return FALSE;
		}

		/* ����y��(�߂�) */
		offset -= (80 * 2);
	}

	return TRUE;
}
#endif

#if XM7_VER >= 3
/*
 *  BMP�f�[�^��������(400���C�����[�h�E�k���摜)
 */
static BOOL FASTCALL bmp_400l_sub2(int fileh)
{
	int x, y;
	int i;
	int offset;
	BYTE bit;
	BYTE buffer[640];
	BYTE *vramptr;
	BYTE pal[8];
	BYTE lbuf[640];
	BYTE pbuf[4];
	WORD color;
	BYTE col1, col2;
	WORD dx1, dx2;
	BOOL winy;

	ASSERT(fileh >= 0);

	/* �F�����e�[�u�������� */
	mix_color_init(GAMMA400L);

	/* �F�f�[�^���� */
	for (i=0; i<8; i++) {
		if (crt_flag) {
			pal[i] = (BYTE)(ttl_palet[i & ((~(multi_page >> 4)) & 7)] & 7);
		}
		else {
			pal[i] = 0;
		}
	}

	/* �����I�t�Z�b�g�ݒ� */
	offset = 80 * 399;

	/* �E�B���h�E�̈�̃N���b�s���O���s�� */
	window_clip(2);
	dx1 = (WORD)(window_dx1 >> 3);
	dx2 = (WORD)(window_dx2 >> 3);

	/* y���[�v */
	for (y=0; y<400; y++) {
		winy = (((399 - y) >= window_dy1) && ((399 - y) <= window_dy2));

		if ((y % 2) == 0) {
			/* �p���b�g�o�b�t�@������ */
			memset(lbuf, 0, sizeof(pbuf));
		}

		/* x���[�v */
		for (x=0; x<80; x++) {
			bit = 0x80;
			if (winy && (x >= dx1) && (x < dx2)) {
				/* �E�B���h�E��(���u���b�N) */
				vramptr = vram_bdptr;
			}
			else {
				/* �E�B���h�E�O(�\�u���b�N) */
				vramptr = vram_dptr;
			}

			/* bit���[�v */
			for (i=0; i<4; i++) {
				col1 = 0;
				col2 = 0;

				if (vramptr[offset + 0x00000] & bit) {
					col1 |= 1;
				}
				if (vramptr[offset + 0x08000] & bit) {
					col1 |= 2;
				}
				if (vramptr[offset + 0x10000] & bit) {
					col1 |= 4;
				}
				bit >>= 1;

				if (vramptr[offset + 0x00000] & bit) {
					col2 |= 1;
				}
				if (vramptr[offset + 0x08000] & bit) {
					col2 |= 2;
				}
				if (vramptr[offset + 0x10000] & bit) {
					col2 |= 4;
				}
				bit >>= 1;

				if ((y % 2) == 0) {
					lbuf[(x * 4 + i) * 2 + 0] = col1;
					lbuf[(x * 4 + i) * 2 + 1] = col2;
				}
				else {
					pbuf[0] = pal[lbuf[(x * 4 + i) * 2 + 0]];
					pbuf[1] = pal[lbuf[(x * 4 + i) * 2 + 1]];
					pbuf[2] = pal[col1];
					pbuf[3] = pal[col2];

					color = mix_color(pbuf, 4, FALSE);
					buffer[x * 8 + i * 2 + 0] = (BYTE)(color & 255);
					buffer[x * 8 + i * 2 + 1] = (BYTE)(color >> 8);
				}
			}
			offset++;
		}

		/* �������� */
		if ((y % 2) == 1) {
			if (!file_write(fileh, buffer, sizeof(buffer))) {
				return FALSE;
			}
		}

		/* ����y��(�߂�) */
		offset -= (80 * 2);
	}

	return TRUE;
}
#endif

#if XM7_VER == 1 && defined(L4CARD)
/*
 *  BMP�f�[�^��������(L4 400���C�����[�h�E�k���摜)
 */
static BOOL FASTCALL bmp_400l4_sub2(int fileh, BOOL mode)
{
	int x, y;
	int i;
	int offset;
	BYTE bit, bit2;
	BYTE buffer[640];
	BYTE lbuf[640];
	BYTE pbuf[4];
	WORD color;
	BYTE col1, col2;
	WORD taddr, gaddr, textbase;
	BYTE csr_st, csr_ed, csr_type;
	BYTE raster, lines;
	BYTE col, chr, atr, chr_dat;
	BOOL enable_page;

	ASSERT(fileh >= 0);

	/* �����I�t�Z�b�g�ݒ� */
	offset = 80 * 399;

	/* �e�L�X�g�W�J �����ݒ�(�S��) */
	csr_st = (BYTE)(crtc_register[10] & 0x1f);
	csr_ed = (BYTE)(crtc_register[11] & 0x1f);
	csr_type = (BYTE)((crtc_register[10] & 0x60) >> 5);
	lines = (BYTE)((crtc_register[9] & 0x1f) + 1);

	/* �p���b�g�o�b�t�@������ */
	memset(lbuf, 0, sizeof(pbuf));

	/* y���[�v */
	for (y=0; y<400; y++) {
		/* �e�L�X�g�W�J �����ݒ�(���X�^�P��) */
		textbase = (WORD)text_start_addr;
		textbase += (WORD)(((399 - y) / lines) * (crtc_register[1] << 2));
		textbase &= 0xffe;
		raster = (BYTE)((399 - y) % lines);

		/* x���[�v */
		for (x=0; x<80; x++) {
			bit = 0x80;
			if (!width40_flag || !(x & 1)) {
				bit2 = 0x80;

				/* �L�����N�^�R�[�h�E�A�g���r���[�g���擾 */
				if (width40_flag) {
					taddr = (WORD)((textbase + (x & ~1)) & 0xffe);
				}
				else {
					taddr = (WORD)((textbase + (x << 1)) & 0xffe);
				}
				chr = tvram_c[taddr + 0];
				atr = tvram_c[taddr + 1];

				/* �A�g���r���[�g����`��F��ݒ� */
				col = (BYTE)((atr & 0x07) | ((atr & 0x20) >> 2));

				/* �t�H���g�f�[�^�擾(�A�g���r���[�g/�J�[�\���������܂�) */
				if ((!(atr & 0x10) || text_blink) && (raster < 16)) {
					chr_dat = subcg_l4[(WORD)(chr << 4) + raster];
				}
				else {
					chr_dat = 0x00;
				}
				if (atr & 0x08) {
					chr_dat ^= (BYTE)0xff;
				}
				if (csr_type != 1) {
					if (((taddr == cursor_addr) &&
						(cursor_blink || !csr_type)) &&
						((raster >= csr_st) && (raster <= csr_ed))) {
						chr_dat ^= (BYTE)0xff;
					}
				}
			}

			/* GVRAM ���A�h���X���擾 */
			gaddr = (WORD)((offset + vram_offset[0]) & 0x7fff);
			enable_page = FALSE;
			if (gaddr >= 0x4000) {
				if (!(multi_page & 0x10)) {
					enable_page = TRUE;
				}
			}
			else {
				if (!(multi_page & 0x20)) {
					enable_page = TRUE;
				}
			}

			/* bit���[�v */
			for (i=0; i<4; i++) {
				if (chr_dat & bit2) {
					col1 = col;
				}
				else if ((vram_c[gaddr] & bit) && enable_page) {
					col1 = ttl_palet[1];
				}
				else {
					col1 = ttl_palet[0];
				}
				bit >>= 1;
				if (!width40_flag) {
					bit2 >>= 1;
				}

				if (chr_dat & bit2) {
					col2 = col;
				}
				else if ((vram_c[gaddr] & bit) && enable_page) {
					col2 = ttl_palet[1];
				}
				else {
					col2 = ttl_palet[0];
				}
				bit >>= 1;
				bit2 >>= 1;

				if ((y % 2) == 0) {
					lbuf[(x * 4 + i) * 2 + 0] = col1;
					lbuf[(x * 4 + i) * 2 + 1] = col2;
				}
				else {
					if (crt_flag) {
						pbuf[0] = lbuf[(x * 4 + i) * 2 + 0];
						pbuf[1] = lbuf[(x * 4 + i) * 2 + 1];
						pbuf[2] = col1;
						pbuf[3] = col2;

						color = mix_color_16(GAMMA400L, pbuf, 4, mode);
						buffer[x * 8 + i * 2 + 0] = (BYTE)(color & 255);
						buffer[x * 8 + i * 2 + 1] = (BYTE)(color >> 8);
					}
					else {
						buffer[x * 8 + i * 2 + 0] = 0;
						buffer[x * 8 + i * 2 + 1] = 0;
					}
				}
			}
			offset++;
		}

		/* 2���C�����Ƃɏ������� */
		if (y & 1) {
			if (!file_write(fileh, buffer, sizeof(buffer))) {
				return FALSE;
			}
		}

		/* ����y��(�߂�) */
		offset -= (80 * 2);
	}

	return TRUE;
}
#endif

#if XM7_VER == 1
/*
 *  BMP�f�[�^��������(�^��400���C���E�k���摜)
 */
static BOOL FASTCALL bmp_p400m_sub2(int fileh, BOOL mode)
{
	int x, y;
	int i;
	int offset;
	BYTE bit;
	BYTE buffer[640];
	BYTE pal[8];
	BYTE pbuf[4];
	WORD color;
	BYTE col;

	ASSERT(fileh >= 0);
	UNUSED(mode);

	/* �F�����e�[�u�������� */
	mix_color_init(GAMMA400L);

	/* �F�f�[�^���� */
	for (i=0; i<8; i++) {
		if (crt_flag) {
			pal[i] = (BYTE)(ttl_palet[i & ((~(multi_page >> 4)) & 7)] & 7);
		}
		else {
			pal[i] = 0;
		}
	}

	/* �����I�t�Z�b�g�ݒ� */
	offset = 80 * 199;

	/* y���[�v */
	for (y=0; y<400; y++) {
		/* x���[�v */
		for (x=0; x<80; x++) {
			bit = 0x80;

			/* bit���[�v */
			for (i=0; i<4; i++) {
				pbuf[0] = 0;
				pbuf[1] = 0;
				pbuf[2] = 0;
				pbuf[3] = 0;

				col = 0;
				if (vram_c[offset + 0x00000] & bit) {
					col |= 1;
				}
				if (vram_c[offset + 0x04000] & bit) {
					col |= 2;
				}
				if (vram_c[offset + 0x08000] & bit) {
					col |= 4;
				}
				bit >>= 1;

				if (pal[col] & 2) {
					pbuf[0] = 7;
				}
				if (pal[col] & 4) {
					pbuf[1] = 7;
				}

				col = 0;
				if (vram_c[offset + 0x00000] & bit) {
					col |= 1;
				}
				if (vram_c[offset + 0x04000] & bit) {
					col |= 2;
				}
				if (vram_c[offset + 0x08000] & bit) {
					col |= 4;
				}
				bit >>= 1;

				if (pal[col] & 2) {
					pbuf[2] = 7;
				}
				if (pal[col] & 4) {
					pbuf[3] = 7;
				}

				color = mix_color(pbuf, 4, mode);
				buffer[x * 8 + i * 2 + 0] = (BYTE)(color & 255);
				buffer[x * 8 + i * 2 + 1] = (BYTE)(color >> 8);
			}
			offset++;
		}

		/* �������� */
		if (!file_write(fileh, buffer, sizeof(buffer))) {
			return FALSE;
		}

		/* ����y��(�߂�) */
		offset -= 80 * 2;
	}

	return TRUE;
}
#endif

/*
 *	��ʃL���v�`��(BMP)
 */
BOOL FASTCALL capture_to_bmp(char *fname, BOOL fullscan, BOOL mode, BOOL p400line)
{
	int fileh;

#if XM7_VER == 1
	UNUSED(fullscan);
#endif
#if XM7_VER >= 3
	UNUSED(mode);
#endif
	ASSERT(fname);

	/* �t�@�C���I�[�v�� */
	fileh = file_open(fname, OPEN_W);
	if (fileh == -1) {
		return FALSE;
	}

	/* �w�b�_�������� */
	if (!bmp_header_sub(fileh)) {
		file_close(fileh);
		return FALSE;
	}

	/* �p���b�g�������� */
#if XM7_VER >= 2
#if XM7_VER >= 3
	if (!(screen_mode & SCR_ANALOG)) {
#else
	if (!mode320) {
#endif
		if (!bmp_palette_sub(fileh, FALSE)) {
			file_close(fileh);
			return FALSE;
		}
	}
#else
	if (!bmp_palette_sub(fileh, mode)) {
		file_close(fileh);
		return FALSE;
	}
#endif

	/* �{�̏������� */
#if XM7_VER >= 3
	switch (screen_mode) {
		case SCR_400LINE	:	/* 640�~400 8�F���[�h */
								if (!bmp_400l_sub(fileh)) {
									file_close(fileh);
									return FALSE;
								}
								break;
		case SCR_262144		:	/* 320�~200 26���F���[�h */
								if (!bmp_256k_sub(fileh, fullscan)) {
									file_close(fileh);
									return FALSE;
								}
								break;
		case SCR_4096		:	/* 320�~200 4096�F���[�h */
								if (!bmp_320_sub(fileh, fullscan, FALSE)) {
									file_close(fileh);
									return FALSE;
								}
								break;
		case SCR_200LINE	:	/* 640�~200 8�F���[�h */
								if (p400line) {
									if (!bmp_p400c_sub(fileh)) {
										file_close(fileh);
										return FALSE;
									}
								}
								else {
									if (!bmp_640_sub(fileh, fullscan)) {
										file_close(fileh);
										return FALSE;
									}
								}
								break;
	}
#elif XM7_VER >= 2
	if (mode320) {
		/* 320�~200 4096�F���[�h */
		if (!bmp_320_sub(fileh, fullscan, mode)) {
			file_close(fileh);
			return FALSE;
		}
	}
	else {
		/* 640�~200 8�F���[�h */
			if (p400line) {
				if (!bmp_p400c_sub(fileh)) {
					file_close(fileh);
					return FALSE;
				}
			}
		else {
			if (!bmp_640_sub(fileh, fullscan)) {
				file_close(fileh);
				return FALSE;
			}
		}
	}
#elif XM7_VER == 1 && defined(L4CARD)
	if (enable_400line) {
		/* 640�~400 �P�F���[�h */
		if (!bmp_400l4_sub(fileh, mode)) {
			file_close(fileh);
			return FALSE;
		}
	}
	else {
		if (p400line) {
			/* 640�~200 �^��400���C�����[�h */
			if (!bmp_p400m_sub(fileh)) {
				file_close(fileh);
				return FALSE;
			}
		}
		else {
			/* 640�~200 8�F���[�h */
			if (!bmp_640_sub(fileh, fullscan)) {
				file_close(fileh);
				return FALSE;
			}
		}
	}
#else
	if (p400line) {
		/* 640�~200 �^��400���C�����[�h */
		if (!bmp_p400m_sub(fileh)) {
			file_close(fileh);
			return FALSE;
		}
	}
	else {
		/* 640�~200 8�F���[�h */
		if (!bmp_640_sub(fileh, fullscan)) {
			file_close(fileh);
			return FALSE;
		}
	}
#endif

	/* ���� */
	file_close(fileh);
	return TRUE;
}

/*
 *	��ʃL���v�`��(BMP�E�k���摜)
 */
BOOL FASTCALL capture_to_bmp2(char *fname, BOOL mode, BOOL p400line)
{
	int fileh;

#if XM7_VER >= 3
	UNUSED(mode);
#endif
	ASSERT(fname);

	/* �t�@�C���I�[�v�� */
	fileh = file_open(fname, OPEN_W);
	if (fileh == -1) {
		return FALSE;
	}

	/* �w�b�_�������� */
	if (!bmp_header_sub2(fileh)) {
		file_close(fileh);
		return FALSE;
	}

	/* �{�̏������� */
#if XM7_VER >= 3
	switch (screen_mode) {
		case SCR_400LINE	:	/* 640�~400 8�F���[�h */
								if (!bmp_400l_sub2(fileh)) {
									file_close(fileh);
									return FALSE;
								}
								break;
		case SCR_262144		:	/* 320�~200 26���F���[�h */
								if (!bmp_256k_sub2(fileh)) {
									file_close(fileh);
									return FALSE;
								}
								break;
		case SCR_4096		:	/* 320�~200 4096�F���[�h */
								if (!bmp_320_sub2(fileh, FALSE)) {
									file_close(fileh);
									return FALSE;
								}
								break;
		case SCR_200LINE	:	/* 640�~200 8�F���[�h */
								if (p400line) {
									if (!bmp_p400c_sub2(fileh)) {
										file_close(fileh);
										return FALSE;
									}
								}
								else {
									if (!bmp_640_sub2(fileh, FALSE)) {
										file_close(fileh);
										return FALSE;
									}
								}
								break;
	}
#elif XM7_VER >= 2
	if (mode320) {
		/* 320�~200 4096�F���[�h */
		if (!bmp_320_sub2(fileh, mode)) {
			file_close(fileh);
			return FALSE;
		}
	}
	else {
		/* 640�~200 8�F���[�h */
			if (p400line) {
				if (!bmp_p400c_sub2(fileh)) {
					file_close(fileh);
					return FALSE;
				}
			}
		else {
			if (!bmp_640_sub2(fileh, FALSE)) {
				file_close(fileh);
				return FALSE;
			}
		}
	}
#elif XM7_VER == 1 && defined(L4CARD)
	if (enable_400line) {
		/* 640�~400 �P�F���[�h */
		if (!bmp_400l4_sub2(fileh, mode)) {
			file_close(fileh);
			return FALSE;
		}
	}
	else {
		if (p400line) {
			/* 640�~200 �^��400���C�����[�h */
			if (!bmp_p400m_sub2(fileh, mode)) {
				file_close(fileh);
				return FALSE;
			}
		}
		else {
			/* 640�~200 8�F���[�h */
			if (!bmp_640_sub2(fileh, mode)) {
				file_close(fileh);
				return FALSE;
			}
		}
	}
#else
	if (p400line) {
		/* 640�~200 �^��400���C�����[�h */
		if (!bmp_p400m_sub2(fileh, mode)) {
			file_close(fileh);
			return FALSE;
		}
	}
	else {
		/* 640�~200 8�F���[�h */
		if (!bmp_640_sub2(fileh, mode)) {
			file_close(fileh);
			return FALSE;
		}
	}
#endif

	/* ���� */
	file_close(fileh);
	return TRUE;
}
