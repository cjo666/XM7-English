/*
 *	FM-7 EMULATOR "XM7"
 *
 *	Copyright (C) 1999-2017 �o�h�D(Twitter:@xm6_original)
 *	Copyright (C) 2001-2017 Ryu Takegami (Twitter:@RyuTakegami)
 *
 *	[ DMAC(HD6844) AV40�d�l�Ή��� ]
 *
 *	RHG����
 *	  2001.05.08	�������W�X�^�̂S�`�����l���Ή���/�f�[�^�`�F�C���Ή�
 *					HALT�T�C�N���X�`�[���]�����̃N���b�N�������΂������ǉ�
 *					�]������BCR��0�ɂ����ꍇ�̋������C��
 *	  2001.05.09	�X�e�[�g�t�@�C���������ݏ����̐V�@�\�Ή�(�o�[�W����9��p)
 *	  2001.11.15	�]���I�����Ă�BUSY�t���O�������Ȃ������C��
 *					�]���I������TxRQ�r�b�g������ɗ���������C��
 *					DEND�r�b�g�����[�h�A�N�Z�X�ŃN���A�����悤�ɏC��
 *					���̑��A�ׂ��������̋��������@�ɍ��킹�ďC��
 *					AV40�d�l�Ή��łƃ\�[�X�𓝍�
 *	  2002.02.12	BCR���������ɂ�钆�f����BUSY�t���O�������Ȃ������C��
 *	  2002.02.16	BCR���������ɂ�钆�f����DEND�t���O�������Ȃ������C��
 *	  2002.02.19	IRQ����������ςȂ��ɂȂ�����C��(�c�c�c)
 *	  2002.05.09	DMA�]�������t���O��V��
 *	  2002.05.11	FDC�R�}���h���s����100��s���DMA�]���J�n����悤�ɕύX
 *	  2002.07.17	���荞�ݐ��䃌�W�X�^ bit7�̋������C��
 *	  2002.12.09	FDC�R�}���h���s����]���J�n�܂ł̃E�F�C�g��50��s�ɕύX
 *	  2004.08.16	�o�[�X�g�]���̎����������܂Ƃ��ɂ���
 */

#if XM7_VER >= 3

#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "xm7.h"
#include "device.h"
#include "mainetc.h"
#include "dmac.h"
#include "fdc.h"
#include "mmr.h"
#include "event.h"

/*
 *	�O���[�o�� ���[�N
 */
WORD dma_adr[4];						/* �A�h���X���W�X�^ */
WORD dma_bcr[4];						/* �]���ꐔ���W�X�^ */
BYTE dma_chcr[4];						/* �`���l�����䃌�W�X�^ */
BYTE dma_pcr;							/* �D�搧�䃌�W�X�^ */
BYTE dma_icr;							/* ���荞�ݐ��䃌�W�X�^ */
BYTE dma_dcr;							/* �f�[�^�`�F�C�����䃌�W�X�^ */

BYTE dma_reg;							/* ���ݑI������Ă��郌�W�X�^�ԍ� */
BOOL dma_flag;							/* DMA�]�����s���t���O */
BOOL dma_burst_transfer;				/* DMA�o�[�X�g�]�����s���t���O */


/*
 *	�v���g�^�C�v�錾
 */
static BOOL FASTCALL dmac_start_ch0(void);	/* DMA�]���J�n�C�x���g */


/*
 *	DMAC
 *	������
 */
BOOL FASTCALL dmac_init(void)
{
	return TRUE;
}

/*
 *	DMAC
 *	�N���[���A�b�v
 */
void FASTCALL dmac_cleanup(void)
{
}

/*
 *	DMAC
 *	���Z�b�g
 */
void FASTCALL dmac_reset(void)
{
	int i;

	/* DMAC�������W�X�^������ */
	for (i=0; i<4; i++) {
		dma_adr[i] = 0xFFFF;
		dma_bcr[i] = 0xFFFF;
		dma_chcr[i] = 0x00;
	}
	dma_pcr = 0x00;
	dma_icr = 0x00;
	dma_dcr = 0x00;

	/* ���[�N������ */
	dma_reg = 0x00;
	dma_flag = FALSE;
	dma_burst_transfer = FALSE;
}


/*
 *	DMAC
 *	�]���J�n
 */
void FASTCALL dmac_start(void)
{
	/* �o�[�W�����`�F�b�N */
	if (fm7_ver < 3) {
		return;
	}

	/* ����DMA�]�����쒆�ł���Ή������Ȃ� */
	if (dma_flag) {
		return;
	}

	/* �]���꒷���W�X�^(BCR)�`�F�b�N */
	if (dma_bcr[0] == 0) {
		return;
	}

	/* DMA�g�p�t���O(TxRQ)�`�F�b�N */
	if (!(dma_pcr & 0x01)) {
		return;
	}

	/* DMA�]���J�n �C�x���g�ݒ� */
	schedule_setevent(EVENT_FDC_DMA, 50, dmac_start_ch0);
}

/*
 *	DMAC Ch.0(FDC)
 *	�]���J�n�C�x���g
 */
static BOOL FASTCALL dmac_start_ch0(void)
{
	/* �C�x���g���폜 */
	schedule_delevent(EVENT_FDC_DMA);

	/* DMA�X�^�[�g */
	dma_flag = TRUE;
	dma_chcr[0] = (BYTE)((dma_chcr[0] & 0x0f) | 0x40);

	return TRUE;
}

/*
 *	DMAC Ch.0(FDC)
 *	DMA�]������
 */
void FASTCALL dmac_exec(void)
{
	BYTE dat;
	BYTE seg;

	/* TxRQ�`�F�b�N */
	if (!(dma_pcr & 0x01)) {
		return;
	}

	/* ���̎��_��BCR��0�Ȃ�]��/���荞�ݔ����������ɏ����𒆎~ */
	if (dma_bcr[0] == 0) {
		dma_flag = FALSE;
		dma_chcr[0] = (BYTE)((dma_chcr[0] & 0x0f) | 0x80);
		return;
	}

	/* �o�[�X�g���[�h�̃`�F�b�N�������Ȃ� */
	if ((dma_chcr[0] & 0x02) && !dma_burst_transfer) {
		dma_burst_transfer = TRUE;
	}

	/* FDC�����DRQ���󂯁A�]������ */
	fdc_readb(0xfd1f, &dat);
	if (!(dat & 0x80)) {
		if (dma_burst_transfer) {
			/* �X�P�W���[�����O�̊֌W��A���C��CPU�N���b�N��i�߂� */
			maincpu.total += (WORD)2;
		}
		return;
	}

	/* DMA�]�������s�BMMR�͏�ɃZ�O�����g0�ɂȂ� */
	seg = mmr_seg;
	mmr_seg = 0;

	if (dma_chcr[0] & 0x01) {
		/* 1:��������FDC (Write) */
		dat = mainmem_readb(dma_adr[0]);
		fdc_writeb(0xfd1b, dat);
	}
	else {
		/* 0:FDC�������� (Read) */
		fdc_readb(0xfd1b, &dat);
		mainmem_writeb(dma_adr[0], dat);
	}

	/* MMR�Z�O�����g���A */
	mmr_seg = seg;

	/* �A�h���X�X�V */
	if (dma_chcr[0] & 0x08) {
		/* �A�h���XDOWN */
		dma_adr[0]--;
	}
	else {
		/* �A�h���XUP */
		dma_adr[0]++;
	}

	/* ���C��CPU�N���b�N�̈������΂����s�� */
	maincpu.total += (WORD)3;

	/* �]���T�C�Y�X�V */
	dma_bcr[0]--;

	/* �]���I���`�F�b�N */
	if (dma_bcr[0] == 0) {
#if defined(DMAC_AV40)
		if ((dma_dcr & 0x07) == 0x01) {
			/* �f�[�^�`�F�C�� */
			dma_adr[0] = dma_adr[3];
			dma_bcr[0] = dma_bcr[3];
			dma_bcr[3] = 0;
		}
		else {
			/* DMA�I�� */
			dma_flag = FALSE;
			dma_burst_transfer = FALSE;
			dma_chcr[0] = (BYTE)((dma_chcr[0] & 0x0f) | 0x80);
			dma_dcr |= 0x10;

			/* ���荞�݂��C�l�[�u���Ȃ�A���荞�݂������� */
			if (dma_icr & 0x01) {
				dma_icr |= 0x80;
				dma_irq_flag = TRUE;
				maincpu_irq();
			}
			return;
		}
#else
		/* DMA�I�� */
		dma_flag = FALSE;
		dma_burst_transfer = FALSE;
		dma_chcr[0] = (BYTE)((dma_chcr[0] & 0x0f) | 0x80);
		dma_dcr |= 0x10;

		/* ���荞�݂��C�l�[�u���Ȃ�A���荞�݂������� */
		if (dma_icr & 0x01) {
			dma_icr |= 0x80;
			dma_irq_flag = TRUE;
			maincpu_irq();
		}
		return;
#endif
	}
}

/*
 *	DMAC
 *	���W�X�^�ǂݍ���
 */
static BYTE FASTCALL dmac_readreg(WORD addr)
{
	BYTE	tmp;

	switch (addr) {
#if defined(DMAC_AV40)
		/* �A�h���X���W�X�^(���) */
		case 0x00:
		case 0x04:
		case 0x08:
		case 0x0c:
			return (BYTE)((dma_adr[addr>>2] >> 8) & 0xff);

		/* �A�h���X���W�X�^(����) */
		case 0x01:
		case 0x05:
		case 0x09:
		case 0x0d:
			return (BYTE)(dma_adr[addr>>2] & 0xff);

		/* �]���ꐔ���W�X�^(���) */
		case 0x02:
		case 0x06:
		case 0x0a:
		case 0x0e:
			return (BYTE)((dma_bcr[addr>>2] >> 8) & 0xff);

		/* �]���ꐔ���W�X�^(����) */
		case 0x03:
		case 0x07:
		case 0x0b:
		case 0x0f:
			return (BYTE)(dma_bcr[addr>>2] & 0xff);

		/* �`���l�����䃌�W�X�^ */
		case 0x10:
		case 0x11:
		case 0x12:
		case 0x13:
			tmp = dma_chcr[addr-0x10];
			dma_chcr[addr-0x10] &= 0x7f;
			return tmp;
#else
		/* �A�h���X���W�X�^(���) */
		case 0x00:
			return (BYTE)((dma_adr[0] >> 8) & 0xff);

		/* �A�h���X���W�X�^(����) */
		case 0x01:
			return (BYTE)(dma_adr[0] & 0xff);

		/* �]���ꐔ���W�X�^(���) */
		case 0x02:
			return (BYTE)((dma_bcr[0] >> 8) & 0xff);

		/* �]���ꐔ���W�X�^(����) */
		case 0x03:
			return (BYTE)(dma_bcr[0] & 0xff);

		/* �`���l�����䃌�W�X�^ */
		case 0x10:
			tmp = dma_chcr[0];
			dma_chcr[0] &= 0x7f;
			return tmp;
#endif

		/* �D�搧�䃌�W�X�^ */
		case 0x14:
			return dma_pcr;

		/* ���荞�ݐ��䃌�W�X�^ */
		case 0x15:
			tmp = (BYTE)(((dma_dcr >> 4) | 0x80) & dma_icr);
			dma_dcr &= 0x0f;

			/* IRQ������ */
			dma_icr &= (BYTE)~0x80;
			dma_irq_flag = FALSE;
			maincpu_irq();
			return tmp;

		/* �f�[�^�`�F�C�����䃌�W�X�^ */
		case 0x16:
			return (BYTE)(dma_dcr & 0x0f);
	}

	/* ���̑��̃��W�X�^�̓f�R�[�h����Ă��Ȃ� */
	return 0x00;
}

/*
 *	DMAC
 *	���W�X�^��������
 */
static void FASTCALL dmac_writereg(WORD addr, BYTE dat)
{
	switch (addr) {
		/* �A�h���X���W�X�^(���) */
		case 0x00:
		case 0x04:
		case 0x08:
		case 0x0c:
			dma_adr[addr>>2] = (WORD)((dma_adr[addr>>2] & 0xff) | (dat << 8));
			return;

		/* �A�h���X���W�X�^(����) */
		case 0x01:
		case 0x05:
		case 0x09:
		case 0x0d:
			dma_adr[addr>>2] = (WORD)((dma_adr[addr>>2] & 0xff00) | dat);
			return;

		/* �]���ꐔ���W�X�^(���) */
		case 0x02:
		case 0x06:
		case 0x0a:
		case 0x0e:
			dma_bcr[addr>>2] = (WORD)((dma_bcr[addr>>2] & 0xff) | (dat << 8));
			return;

		/* �]���ꐔ���W�X�^(����) */
		case 0x03:
		case 0x07:
		case 0x0b:
		case 0x0f:
			dma_bcr[addr>>2] = (WORD)((dma_bcr[addr>>2] & 0xff00) | dat);
			return;

		/* �`���l�����䃌�W�X�^ */
		case 0x10:
		case 0x11:
		case 0x12:
		case 0x13:
			dma_chcr[addr-0x10] = (BYTE)((dma_chcr[addr-0x10] & 0xc0) | (dat & 0x0f));
			return;

		/* �D�搧�䃌�W�X�^ */
		case 0x14:
			dma_pcr = (BYTE)(dat & 0x8f);
			return;

		/* ���荞�ݐ��䃌�W�X�^ */
		case 0x15:
			dma_icr = (BYTE)((dma_icr & 0x80) | (dat & 0x0f));
			return;

		/* �f�[�^�`�F�C�����䃌�W�X�^ */
		case 0x16:
			dma_dcr = (BYTE)((dma_dcr & 0xf0) | (dat & 0x0f));
			return;
	}
}

/*
 *	DMAC
 *	�P�o�C�g�擾
 */
BOOL FASTCALL dmac_readb(WORD addr,BYTE *dat)
{
	/* �o�[�W�����`�F�b�N */
	if (fm7_ver < 3) {
		return FALSE;
	}

	switch (addr) {
		/* DMAC�A�h���X */
		case 0xfd98:
			*dat = dma_reg;
			return TRUE;

		/* DMAC�f�[�^ */
		case 0xfd99:
			*dat = dmac_readreg(dma_reg);
			return TRUE;
	}

	return FALSE;
}

/*
 *	DMAC
 *	�P�o�C�g��������
 */
BOOL FASTCALL dmac_writeb(WORD addr, BYTE dat)
{
	/* �o�[�W�����`�F�b�N */
	if (fm7_ver < 3) {
		return FALSE;
	}

	switch (addr) {
		/* DMAC�A�h���X */
		case 0xfd98:
			dma_reg = (BYTE)(dat & 0x1f);
			return TRUE;

		/* DMAC�f�[�^ */
		case 0xfd99:
			dmac_writereg(dma_reg, dat);
			return TRUE;
	}

	return FALSE;
}

/*
 *	DMAC
 *	�Z�[�u
 */
BOOL FASTCALL dmac_save(int fileh)
{
#if defined(DMAC_AV40)
	int	i;
#endif

	if (!file_word_write(fileh, dma_adr[0])) {
		return FALSE;
	}
	if (!file_word_write(fileh, dma_bcr[0])) {
		return FALSE;
	}
	if (!file_byte_write(fileh, dma_chcr[0])) {
		return FALSE;
	}
	if (!file_byte_write(fileh, dma_pcr)) {
		return FALSE;
	}
	if (!file_byte_write(fileh, dma_icr)) {
		return FALSE;
	}
	if (!file_byte_write(fileh, dma_reg)) {
		return FALSE;
	}
	if (!file_bool_write(fileh, dma_flag)) {
		return FALSE;
	}

	/* Ver9�g�� */
	if (!file_byte_write(fileh, dma_dcr)) {
		return FALSE;
	}

	/* Ver912�g�� */
	if (!file_bool_write(fileh, dma_burst_transfer)) {
		return FALSE;
	}

	/* Ver9�g���EAV40�d�l�� (40EX�d�l���Ƃ̌݊������Ȃ��Ȃ�̂Œ���) */
#if defined(DMAC_AV40)
	for (i=1;i<4;i++) {
		if (!file_word_write(fileh, dma_adr[i])) {
			return FALSE;
		}
		if (!file_word_write(fileh, dma_bcr[i])) {
			return FALSE;
		}
		if (!file_word_write(fileh, dma_chcr[i])) {
			return FALSE;
		}
	}
#endif

	return TRUE;
}

/*
 *	DMAC
 *	���[�h
 */
BOOL FASTCALL dmac_load(int fileh, int ver)
{
	int	i;

	/* �o�[�W�����`�F�b�N */
	if (ver < 800) {
		dmac_reset();
		return TRUE;
	}

	if (!file_word_read(fileh, &dma_adr[0])) {
		return FALSE;
	}
	if (!file_word_read(fileh, &dma_bcr[0])) {
		return FALSE;
	}
	if (!file_byte_read(fileh, &dma_chcr[0])) {
		return FALSE;
	}
	if (!file_byte_read(fileh, &dma_pcr)) {
		return FALSE;
	}
	if (!file_byte_read(fileh, &dma_icr)) {
		return FALSE;
	}
	if (!file_byte_read(fileh, &dma_reg)) {
		return FALSE;
	}
	if (!file_bool_read(fileh, &dma_flag)) {
		return FALSE;
	}

	/* Ver9�g�� */
	if (ver >= 900) {
		if (!file_byte_read(fileh, &dma_dcr)) {
			return FALSE;
		}

		/* Ver912�g�� */
		if (ver >= 912) {
			if (!file_bool_read(fileh, &dma_burst_transfer)) {
				return FALSE;
			}
		}
#if defined(DMAC_AV40)
		/* Ver9�g���EAV40�d�l�� (40EX�d�l���Ƃ̌݊������Ȃ��Ȃ�̂Œ���) */
		for (i=1; i<4; i++) {
			if (!file_word_read(fileh, &dma_adr[i])) {
				return FALSE;
			}
			if (!file_word_read(fileh, &dma_bcr[i])) {
				return FALSE;
			}
			if (!file_byte_read(fileh, &dma_chcr[i])) {
				return FALSE;
			}
		}
#endif
	}
	else {
		dma_dcr = 0x08;
	}

	/* Ch1�`3�������� */
#if !defined(DMAC_AV40)
	for (i=1; i<4; i++) {
		dma_adr[i] = 0xFFFF;
		dma_bcr[i] = 0xFFFF;
		dma_chcr[i] = 0x00;
	}
#endif

	return TRUE;
}

#endif	/* XM7_VER >= 3 */
