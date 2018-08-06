/*
 *	FM-7 EMULATOR "XM7"
 *
 *	Copyright (C) 1999-2017 �o�h�D(Twitter:@xm6_original)
 *	Copyright (C) 2001-2017 Ryu Takegami (Twitter:@RyuTakegami)
 *
 *	[ DMAC(HD68B44) AV40�d�l�Ή��� ]
 */

#ifndef _dmac_h_
#define _dmac_h_

#if XM7_VER >= 3

#ifdef __cplusplus
extern "C" {
#endif
/*
 *	��v�G���g��
 */
BOOL FASTCALL dmac_init(void);
										/* ������ */
void FASTCALL dmac_cleanup(void);
										/* �N���[���A�b�v */
void FASTCALL dmac_reset(void);
										/* ���Z�b�g */
void FASTCALL dmac_start(void);
										/* �X�^�[�g */
void FASTCALL dmac_exec(void);
										/* �P�X�e�b�v�]�� */
BOOL FASTCALL dmac_readb(WORD addr, BYTE *dat);
										/* �������ǂݏo�� */
BOOL FASTCALL dmac_writeb(WORD addr, BYTE dat);
										/* �������������� */
BOOL FASTCALL dmac_save(int fileh);
										/* �Z�[�u */
BOOL FASTCALL dmac_load(int fileh, int ver);
										/* ���[�h */

/*
 *	�O���[�o�� ���[�N
 */
extern WORD dma_adr[4];
										/* �A�h���X���W�X�^ */
extern WORD dma_bcr[4];
										/* �]���ꐔ���W�X�^ */
extern BYTE dma_chcr[4];
										/* �`���l�����䃌�W�X�^ */
extern BYTE dma_dcr;
										/* �f�[�^�`�F�C�����䃌�W�X�^ */
extern BYTE dma_pcr;
										/* �D�搧�䃌�W�X�^ */
extern BYTE dma_icr;
										/* ���荞�ݐ��䃌�W�X�^ */
extern BYTE dmac_reg;
										/* ���ݑI������Ă��郌�W�X�^�ԍ� */
extern BYTE dma_comp;
										/* DMA�]�������t���O */
extern BOOL dma_flag;
										/* DMA����t���O */
extern BOOL dma_burst_transfer;
										/* DMA�o�[�X�g�]������t���O */
#ifdef __cplusplus
}
#endif

#endif	/* XM7_VER >= 3 */
#endif	/* _dmac_h_ */
