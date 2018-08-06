/*
 *	FM-7 EMULATOR "XM7"
 *
 *	Copyright (C) 1999-2017 �o�h�D(Twitter:@xm6_original)
 *	Copyright (C) 2001-2017 Ryu Takegami (Twitter:@RyuTakegami)
 *
 *	[ ���C��CPU�e��I/O ]
 */

#ifndef _mainetc_h_
#define _mainetc_h_

/*
 *	�萔��`
 */
#define	BANKSEL_DISABLE			0
#define	BANKSEL_ENABLE			1
#define	BANKSEL_ENABLE_DIPSW	2

#ifdef __cplusplus
extern "C" {
#endif
/*
 *	��v�G���g��
 */
extern BOOL FASTCALL mainetc_init(void);
										/* ������ */
extern void FASTCALL mainetc_cleanup(void);
										/* �N���[���A�b�v */
extern void FASTCALL mainetc_reset(void);
										/* ���Z�b�g */
extern BOOL FASTCALL mainetc_readb(WORD addr, BYTE *dat);
										/* �������ǂݏo�� */
extern BOOL FASTCALL mainetc_writeb(WORD addr, BYTE dat);
										/* �������������� */
extern BOOL FASTCALL mainetc_save(int fileh);
										/* �Z�[�u */
extern BOOL FASTCALL mainetc_load(int fileh, int ver);
										/* ���[�h */
extern void FASTCALL mainetc_fdc(void);
										/* FDC���荞�� */
extern void FASTCALL mainetc_lp(void);
										/* ���C���v�����^���荞�� */
extern BOOL FASTCALL mainetc_beep(void);
										/* BEEP�C�x���g */

/*
 *	��v���[�N
 */
extern BOOL key_irq_flag;
										/* �L�[�{�[�h���荞�� �v�� */
extern BOOL key_irq_mask;
										/* �L�[�{�[�h���荞�� �}�X�N */
extern BOOL lp_irq_flag;
										/* �v�����^���荞�� �v�� */
extern BOOL lp_irq_mask;
										/* �v�����^���荞�� �}�X�N */
extern BOOL timer_irq_flag;
										/* �^�C�}�[���荞�� �v�� */
extern BOOL timer_irq_mask;
										/* �^�C�}�[���荞�� �}�X�N */
extern BOOL mfd_irq_flag;
										/* FDC���荞�� �t���O */
extern BOOL mfd_irq_mask;
										/* FDC���荞�� �}�X�N */
extern BOOL txrdy_irq_flag;
										/* TxRDY���荞�� �t���O */
extern BOOL txrdy_irq_mask;
										/* TxRDY���荞�� �}�X�N */
extern BOOL rxrdy_irq_flag;
										/* RxRDY���荞�� �t���O */
extern BOOL rxrdy_irq_mask;
										/* RxRDY���荞�� �}�X�N */
extern BOOL syndet_irq_flag;
										/* SYNDET���荞�� �t���O */
extern BOOL syndet_irq_mask;
										/* SYNDET���荞�� �}�X�N */
extern BOOL ptm_irq_flag;
										/* PTM���荞�� �t���O */
extern BOOL ptm_irq_mask;
										/* PTM���荞�� �}�X�N */
extern BOOL opn_irq_flag;
										/* OPN���荞�� �t���O */
extern BOOL whg_irq_flag;
										/* WHG���荞�� �t���O */
extern BOOL thg_irq_flag;
										/* THG���荞�� �t���O */
#if XM7_VER >= 3
extern BOOL dma_irq_flag;
										/* DMA���荞�� �t���O */
#endif
extern BOOL beep_flag;
										/* BEEP�L���t���O */
extern BOOL speaker_flag;
										/* �X�s�[�J�L���t���O */
#if XM7_VER == 1
extern BYTE banksel_en;
										/* �o���N�؂芷���C�l�[�u���t���O */
#endif
#ifdef __cplusplus
}
#endif

#endif	/* _mainetc_h_ */
