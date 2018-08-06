/*
 *	FM-7 EMULATOR "XM7"
 *
 *	Copyright (C) 1999-2017 �o�h�D(Twitter:@xm6_original)
 *	Copyright (C) 2001-2017 Ryu Takegami (Twitter:@RyuTakegami)
 *
 *	[ ���{��ʐM�J�[�h ]
 */

#ifndef _jsubsys_h_
#define _jsubsys_h_

#if XM7_VER == 1
#if defined(JSUB)

#ifdef __cplusplus
extern "C" {
#endif

/*
 *	�G���g��
 */
BOOL FASTCALL jsubsys_init(void);
										/* ���{��T�u�V�X�e�� ������ */
void FASTCALL jsubsys_cleanup(void);
										/* ���{��T�u�V�X�e�� �N���[���A�b�v */
void FASTCALL jsubsys_reset(void);
										/* ���{��T�u�V�X�e�� ���Z�b�g */
void FASTCALL jsubcpu_execline(void);
										/* ���{��T�uCPU �P�s���s */
void FASTCALL jsubcpu_exec(void);
										/* ���{��T�uCPU ���s */
void FASTCALL jsubcpu_nmi(void);
										/* ���{��T�uCPU NMI���荞�ݗv�� */
void FASTCALL jsubcpu_firq(void);
										/* ���{��T�uCPU FIRQ���荞�ݗv�� */
void FASTCALL jsubcpu_irq(void);
										/* ���{��T�uCPU IRQ���荞�ݗv�� */
BYTE FASTCALL jsubmem_readb(WORD addr);
										/* ���{��T�u������ �ǂݏo�� */
BYTE FASTCALL jsubmem_readbnio(WORD addr);
										/* ���{��T�u������ �ǂݏo��(I/O��) */
void FASTCALL jsubmem_writeb(WORD addr, BYTE dat);
										/* ���{��T�u������ �������� */
BOOL FASTCALL jsubsys_readb(WORD addr, BYTE *dat);
										/* ���{��T�uI/O �ǂݏo�� */
BOOL FASTCALL jsubsys_writeb(WORD addr, BYTE dat);
										/* ���{��T�uI/O �������� */
BOOL FASTCALL jsub_readb(WORD addr, BYTE *dat);
										/* ���{��T�uI/F �ǂݏo�� */
BOOL FASTCALL jsub_writeb(WORD addr, BYTE dat);
										/* ���{��T�uI/F �������� */
BOOL FASTCALL jsubsys_save(int fileh);
										/* ���{��T�u�V�X�e�� �Z�[�u */
BOOL FASTCALL jsubsys_load(int fileh, int ver);
										/* ���{��T�u�V�X�e�� ���[�h */
extern BYTE FASTCALL jsub_readrcb(void);
										/* RCB���[�h */
extern void FASTCALL jsub_writercb(BYTE);
										/* RCB���C�g */
extern void FASTCALL jsub_clear_address(void);
										/* RCB�A�h���X�J�E���^�N���A */

/*
 *	���[�N�G���A
 */
extern BOOL jsub_haltflag;
										/* ���{��T�u�V�X�e��HALT�t���O */
extern BOOL jsub_available;
										/* ���{��T�u�g�p�\�t���O */
extern BOOL jsub_enable;
										/* ���{��T�u�g�p�t���O */
#ifdef __cplusplus
}
#endif

#endif
#endif

#endif	/* _jsubsys_h_ */
