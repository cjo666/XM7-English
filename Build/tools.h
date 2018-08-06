/*
 *	FM-7 EMULATOR "XM7"
 *
 *	Copyright (C) 1999-2017 �o�h�D(Twitter:@xm6_original)
 *	Copyright (C) 2001-2017 Ryu Takegami (Twitter:@RyuTakegami)
 *
 *	[ �⏕�c�[�� ]
 */

#ifndef _tools_h_
#define _tools_h_

#ifdef __cplusplus
extern "C" {
#endif
/*
 *	��v�G���g��
 */
BOOL FASTCALL make_new_d77(char *fname, char *name, BOOL mode2dd);
										/* �u�����N�f�B�X�N�쐬 */
BOOL FASTCALL make_new_userdisk(char *fname, char *name, BOOL mode2dd);
										/* ���[�U�f�B�X�N�쐬 */
BOOL FASTCALL make_new_t77(char *fname);
										/* �u�����N�e�[�v�쐬 */
#if XM7_VER == 1 && defined(BUBBLE)
BOOL FASTCALL make_new_bubble(char *fname, char *name);
										/* �u�����N�o�u���J�Z�b�g�쐬 */
#endif
BOOL FASTCALL conv_vfd_to_d77(char *src, char *dst, char *name);
										/* VFD��D77�ϊ� */
BOOL FASTCALL conv_2d_to_d77(char *src, char *dst, char *name);
										/* 2D/2DD��D77�ϊ� */
BOOL FASTCALL conv_vtp_to_t77(char *src, char *dst);
										/* VTP��T77�ϊ� */
BOOL FASTCALL capture_to_bmp(char *fname, BOOL fullscan, BOOL mode, BOOL p400line);
										/* ��ʃL���v�`��(BMP) */
BOOL FASTCALL capture_to_bmp2(char *fname, BOOL mode, BOOL p400line);
										/* ��ʃL���v�`��(BMP�E�k���摜) */
void FASTCALL mix_color_init(double gamma);
										/* �摜�k���J���[�����e�[�u�������� */
WORD FASTCALL mix_color(BYTE *palet_table, BYTE palet_count, BOOL mode);
										/* �摜�k���J���[���� */
#if XM7_VER == 1 && defined(BUBBLE)
BOOL FASTCALL conv_bbl_to_b77(char *src, char *dst, char *name);
										/* BBL��B77�ϊ� */
#endif
#ifdef __cplusplus
}
#endif

#endif	/* _tools_h_ */
