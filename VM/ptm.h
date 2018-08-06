/*
 *	FM-7 EMULATOR "XM7"
 *
 *	Copyright (C) 1999-2017 �o�h�D(Twitter:@xm6_original)
 *	Copyright (C) 2001-2017 Ryu Takegami (Twitter:@RyuTakegami)
 *
 *	[ �v���O���}�u���^�C�}(MB8873H) �ȈՔ� ]
 */

#if defined(MOUSE)

#ifndef _ptm_h_
#define _ptm_h_

#ifdef __cplusplus
extern "C" {
#endif
/*
 *	��v�G���g��
 */
BOOL FASTCALL ptm_init(void);
										/* ������ */
void FASTCALL ptm_cleanup(void);
										/* �N���[���A�b�v */
void FASTCALL ptm_reset(void);
										/* ���Z�b�g */
BOOL FASTCALL ptm_readb(WORD addr, BYTE *dat);
										/* �������ǂݏo�� */
BOOL FASTCALL ptm_writeb(WORD addr, BYTE dat);
										/* �������������� */
BOOL FASTCALL ptm_save(int fileh);
										/* �Z�[�u */
BOOL FASTCALL ptm_load(int fileh, int ver);
										/* ���[�h */

/*
 *	�O���[�o�� ���[�N
 */
extern int ptm_counter[6];	
										/* �J�E���^ */
extern int ptm_counter_preset[6];
										/* �J�E���^�v���Z�b�g�l */

extern BYTE ptm_mode_select[3];
										/* ���샂�[�h */
extern BOOL ptm_running_flag[3];
										/* ���쒆�t���O */

extern BOOL ptm_out_flag[3];
										/* �o�͋��t���O */
extern BOOL ptm_irq_flag_int[3];
										/* ���荞�݃t���O */
extern BOOL ptm_irq_mask_int[3];
										/* ���荞�݃}�X�N�t���O */
extern BOOL ptm_mode_16bit[3];
										/* 16�r�b�g�J�E���g���[�h */
extern BOOL ptm_clock_type[3];
										/* �N���b�N�� */

extern BOOL ptm_preset_mode;
										/* �v���Z�b�g(InternalReset)���[�h */
extern BOOL ptm_select_reg1;
										/* ���W�X�^�I����� */
extern BOOL ptm_clock_divide;
										/* �N���b�N�������[�h */
#ifdef __cplusplus
}
#endif

#endif	/* _ptm_h_ */
#endif
