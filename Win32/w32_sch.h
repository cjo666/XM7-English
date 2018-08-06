/*
 *	FM-7 EMULATOR "XM7"
 *
 *	Copyright (C) 1999-2017 �o�h�D(Twitter:@xm6_original)
 *	Copyright (C) 2001-2017 Ryu Takegami (Twitter:@RyuTakegami)
 *
 *	[ Win32API �X�P�W���[�� ]
 */

#ifdef _WIN32

#ifndef _w32_sch_h_
#define _w32_sch_h_

#ifdef __cplusplus
extern "C" {
#endif
/*
 *	��v�G���g��
 */
void FASTCALL InitSch(void);
										/* ������ */
void FASTCALL CleanSch(void);
										/* �N���[���A�b�v */
BOOL FASTCALL SelectSch(void);
										/* �Z���N�g */
void FASTCALL ResetSch(void);
										/* ���s���Z�b�g */
void FASTCALL DrawSubWindow(void);
										/* �T�u�E�C���h�E�`�� */

/*
 *	��v���[�N
 */
extern DWORD dwExecTotal;
										/* ���s���ԃg�[�^�� */
extern DWORD dwDrawTotal;
										/* �`��񐔃g�[�^�� */
extern DWORD dwSoundTotal;
										/* �T�E���h���ԃg�[�^�� */
extern DWORD uTimerResolution;
										/* �^�C�}�[���x */
extern BOOL bTapeFullSpeed;
										/* �e�[�v�������[�h */
extern BOOL bFullSpeed;
										/* �S�͋쓮 */
extern BOOL bAutoSpeedAdjust;	
										/* ���x���������t���O */
extern DWORD dwNowTime;
										/* timeGetTime�̒l */
extern BOOL bTapeModeType;
										/* �e�[�v�������[�h�^�C�v */
#if defined(DEBUG)
extern DWORD dwCpuExecTime;
										/* CPU���s���� */
extern DWORD dwDrawExecTime;
										/* �����_���EBLT���s���� */
extern DWORD dwPollExecTime;
										/* �|�[�����O���s���� */
extern DWORD dwMainCycle;
										/* 1ms������̃��C��CPU���s�T�C�N���� */
extern DWORD dwSubCycle;
										/* 1ms������̃T�uCPU���s�T�C�N���� */
extern DWORD dwMainCycleAvg;
										/* 100ms�Ԃ̃��C�����s�T�C�N������ */
extern DWORD dwSubCycleAvg;
										/* 100ms�Ԃ̃T�u���s�T�C�N������ */
#endif
extern BYTE nTimePeriod;
										/* �^�C�}���x�ݒ�w�� */
#ifdef __cplusplus
}
#endif

#endif	/* _w32_sch_h_ */
#endif	/* _WIN32 */
