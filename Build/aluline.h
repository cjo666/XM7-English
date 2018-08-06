/*
 *	FM-7 EMULATOR "XM7"
 *
 *	Copyright (C) 1999-2017 �o�h�D(Twitter:@xm6_original)
 *	Copyright (C) 2001-2017 Ryu Takegami (Twitter:@RyuTakegami)
 *
 *	[ �_�����Z�E������� (MB61VH010/011) ]
 */

#ifndef _aluline_h_
#define _aluline_h_

#if XM7_VER >= 2

#ifdef __cplusplus
extern "C" {
#endif
/*
 *	��v�G���g��
 */
BOOL FASTCALL aluline_init(void);
										/* ������ */
void FASTCALL aluline_cleanup(void);
										/* �N���[���A�b�v */
void FASTCALL aluline_reset(void);
										/* ���Z�b�g */
BOOL FASTCALL aluline_readb(WORD addr, BYTE *dat);
										/* �������ǂݏo�� */
BOOL FASTCALL aluline_writeb(WORD addr, BYTE dat);
										/* �������������� */
void FASTCALL aluline_extrb(WORD addr);
										/* VRAM�_�~�[���[�h */
BOOL FASTCALL aluline_save(int fileh);
										/* �Z�[�u */
BOOL FASTCALL aluline_load(int fileh, int ver);
										/* ���[�h */

/*
 *	��v���[�N
 */
extern BYTE alu_command;
										/* �_�����Z �R�}���h */
extern BYTE alu_color;
										/* �_�����Z �J���[ */
extern BYTE alu_mask;
										/* �_�����Z �}�X�N�r�b�g */
extern BYTE alu_cmpstat;
										/* �_�����Z ��r�X�e�[�^�X */
extern BYTE alu_cmpdat[8];
										/* �_�����Z ��r�f�[�^ */
extern BYTE alu_disable;
										/* �_�����Z �֎~�o���N */
extern BYTE alu_tiledat[3];
										/* �_�����Z �^�C���p�^�[�� */

extern BOOL line_busy;
										/* ������� BUSY */
extern WORD line_offset;
										/* ������� �A�h���X�I�t�Z�b�g */
extern WORD line_style;
										/* ������� ���C���X�^�C�� */
extern WORD line_x0;
										/* ������� X0 */
extern WORD line_y0;
										/* ������� Y0 */
extern WORD line_x1;
										/* ������� X1 */
extern WORD line_y1;
										/* ������� Y1 */
extern BOOL line_boost;
										/* ������� �S���͕`��t���O */
#ifdef __cplusplus
}
#endif

#endif	/* XM7_VER >= 2 */
#endif	/* _aluline_h_ */
