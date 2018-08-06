/*
 *	FM-7 EMULATOR "XM7"
 *
 *	Copyright (C) 1999-2017 �o�h�D(Twitter:@xm6_original)
 *	Copyright (C) 2001-2017 Ryu Takegami (Twitter:@RyuTakegami)
 *
 *	[ Win32API �R���t�B�M�����[�V���� ]
 */

#ifdef _WIN32

#ifndef _w32_cfg_h_
#define _w32_cfg_h_

#ifdef __cplusplus
extern "C" {
#endif
/*
 *	��v�G���g��
 */
void FASTCALL LoadCfg(void);
										/* �ݒ胍�[�h */
BOOL FASTCALL LoadCfg_DoubleSize(void);
										/* �ݒ胍�[�h(2�{�g���p) */
BOOL FASTCALL LoadCfg_LanguageMode(void);
										/* �ݒ胍�[�h(���ꃂ�[�h��p) */
void FASTCALL SaveCfg(void);
										/* �ݒ�Z�[�u */
void FASTCALL ApplyCfg(void);
										/* �ݒ�K�p */
void FASTCALL GetCfg(void);
										/* �ݒ�擾 */
void FASTCALL SetMachineVersion(void);
										/* ����@��Đݒ� */
void FASTCALL OnConfig(HWND hWnd);
										/* �ݒ�_�C�A���O */

/*
 *	��v���[�N
 */
#ifdef __cplusplus
}
#endif

#endif	/* _w32_cfg_h_ */
#endif	/* _WIN32 */
