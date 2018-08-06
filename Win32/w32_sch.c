/*
 *	FM-7 EMULATOR "XM7"
 *
 *	Copyright (C) 1999-2017 �o�h�D(Twitter:@xm6_original)
 *	Copyright (C) 2001-2017 Ryu Takegami (Twitter:@RyuTakegami)
 *
 *	[ Win32API �X�P�W���[�� ]
 *
 *	RHG����
 *	  2001.12.25		�X���b�h�I���w��/�m�F�����̈��S������(�o�h�D)
 *						WinNT�n�ł̃X�P�W���[�����x������(�o�h�D)
 *	  2002.05.07		�_�����Z/������ԃE�B���h�E�̍X�V������ǉ�
 *	  2002.06.06		�X���b�h�̗D�揇�ʐݒ��SetPriorityClass���g���Ă�����
 *						���C��
 *	  2002.07.30		�W���C�X�e�B�b�N,�}�E�X�̃|�[�����O�Ԋu��`�擯���ɕύX
 *	  2002.08.09		�t�A�Z���u���E�B���h�E��PC�񓯊����[�h�ł�VM��~���͋�
 *						���I��PC�����ɂȂ�悤�ɕύX
 *	  2002.08.11		��X�y�b�N�}�V��������CPU���x�������ŗ��Ƃ����[�h��ǉ�
 *	  2002.11.13		�X�P�W���[���X���b�h�̗D�揇�ʂ�ABOVE NORMAL�ɕύX
 *	  2002.12.25		�}�E�X�G�~�����[�V�����g�p���Ƀu���[�N�|�C���g�Œ�~��
 *						���ꍇ�̏��������P(?)
 *	  2003.01.02		�S�͋쓮���̗D�揇�ʐݒ�����P(NT only)
 *	  2003.01.13		�T�X�y���h�΍􏈗���ǉ�
 *	  2003.02.26		�e�[�v�������[�h���̗D�揇�ʐݒ�����P(NT only)
 *	  2004.01.24		�T�E���h�����̎��Ԑ��x������
 *	  2012.03.06		Windows Vista/Windows 7�ɂ����ăN���e�B�J���Z�N�V������
 *						�d�l���ς���Ă����̂�Sleep��UnlockVM�̏������C��
 */

#ifdef _WIN32

#define STRICT
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <mmsystem.h>
#include <assert.h>
#include <process.h>
#include "xm7.h"
#include "tapelp.h"
#include "display.h"
#include "subctrl.h"
#include "device.h"
#include "mouse.h"
#include "w32.h"
#include "w32_sch.h"
#include "w32_draw.h"
#include "w32_snd.h"
#include "w32_kbd.h"
#include "w32_bar.h"
#include "w32_sub.h"
#include "w32_res.h"

#ifdef ROMEO
#include "juliet.h"
#endif

/*
 *	�O���[�o�� ���[�N
 */
DWORD dwExecTotal;						/* ���s�g�[�^������(us) */
DWORD dwDrawTotal;						/* �`��g�[�^���� */
DWORD uTimerResolution;					/* �^�C�}�[���x */
BOOL bTapeFullSpeed;					/* �e�[�v�������[�h�t���O */
#if !defined(DISABLE_FULLSPEED)
BOOL bFullSpeed;						/* �S�͋쓮�t���O */
BOOL bAutoSpeedAdjust;					/* ���x���������t���O */
#endif
DWORD dwNowTime;						/* timeGetTime�̒l */
BOOL bTapeModeType;						/* �e�[�v�������[�h�^�C�v */

#if defined(DEBUG)
DWORD dwCpuExecTime;					/* CPU���s���� */
DWORD dwDrawExecTime;					/* �����_���EBLT���s���� */
DWORD dwPollExecTime;					/* �|�[�����O���s���� */
#endif

/*
 *	�X�^�e�B�b�N ���[�N
 */
static HANDLE hThread;					/* �X���b�h�n���h�� */
static UINT uThResult;					/* �X���b�h�߂�l */
static int nPriority;					/* �X���b�h���s�D�揇�� */
static BOOL bDrawVsync;					/* VSYNC�t���O(�`��p) */
static BOOL bPollVsync;					/* VSYNC�t���O(�|�[�����O�p) */
static DWORD dwTempTime;				/* dwTempTime(ms) */
static DWORD dwExecTime;				/* ���s����(ms) */
static int nFrameSkip;					/* �t���[���X�L�b�v��(ms) */
static BOOL bRunningBak;				/* ���s�t���O backup */
static BOOL bFastMode;					/* �������s���t���O backup */

static DWORD nSpeedCheck;				/* ���x�����p�J�E���^(ms) */
static DWORD dwChkTime;					/* ���x���������(ms) */
static DWORD dwSleepCount;				/* �X���[�v�� */

#if defined(DEBUG)
static LONGLONG lFreq;					/* QueryPerformanceFrequency�̒l */
static LONGLONG lStartTime;				/* �������ԕێ��p */
#endif


/*
 *	�v���g�^�C�v�錾
 */
static UINT WINAPI ThreadSch(LPVOID);			/* �X���b�h�֐� */

/*
 *	������
 */
void FASTCALL InitSch(void)
{
	/* ���[�N�G���A������ */
	hThread = NULL;
	uThResult = 0;
	nPriority = THREAD_PRIORITY_NORMAL;
	bDrawVsync = TRUE;
	bPollVsync = TRUE;
	bRunningBak = FALSE;
	bFastMode = FALSE;

	/* �O���[�o�����[�N */
	dwExecTotal = 0;
	dwDrawTotal = 0;
	dwNowTime = 0;
	bTapeFullSpeed = FALSE;
#if !defined(DISABLE_FULLSPEED)
	bFullSpeed = FALSE;
	bAutoSpeedAdjust = FALSE;
#endif
	uTimerResolution = 1;
	bTapeModeType = FALSE;

#if defined(DEBUG)
	dwCpuExecTime = 0;
	dwDrawExecTime = 0;
	dwPollExecTime = 0;
#endif

	return;
}

/*
 *	�N���[���A�b�v
 */
void FASTCALL CleanSch(void)
{
	DWORD dwExitCode;

	/* �X���b�h������I�����Ă��Ȃ���΁A�I��点�� */
	while (hThread && !uThResult) {
		bCloseReq = TRUE;
		while (TRUE) {
			GetExitCodeThread(hThread, &dwExitCode);
			if (dwExitCode == STILL_ACTIVE) {
				WaitForSingleObject(hThread, 10);
			}
			else {
				CloseHandle(hThread);
				break;
			}
		}
	}

	/* �^�C�}�[�Ԋu��߂� */
	timeEndPeriod(uTimerResolution);
}

/*
 *	�Z���N�g
 */
BOOL FASTCALL SelectSch(void)
{
	TIMECAPS	caps;

#if defined(DEBUG)
	if (!(QueryPerformanceFrequency((LARGE_INTEGER *)&lFreq))) {
		lFreq = 0;
	}
#endif

	/* �^�C�}�[�Ԋu��1ms�� */
	timeGetDevCaps(&caps, sizeof(TIMECAPS));
	if (uTimerResolution < caps.wPeriodMin) {
		uTimerResolution = caps.wPeriodMin;
	}
	if (uTimerResolution > caps.wPeriodMax) {
		uTimerResolution = caps.wPeriodMax;
	}
	timeBeginPeriod(uTimerResolution);

	/* �X���b�h�N�� */
	hThread = (HANDLE)_beginthreadex(NULL, 0, ThreadSch, 0, 0, &uThResult);
	if (hThread == NULL) {
		return FALSE;
	}

	/* �X���b�h�D�揇�ʂ�ݒ� */
	if (bNTflag && bHighPriority) {
		nPriority = THREAD_PRIORITY_ABOVE_NORMAL;
	}
	else {
		nPriority = THREAD_PRIORITY_NORMAL;
	}
	SetThreadPriority(hThread, nPriority);

	return TRUE;
}

/*
 *	VSYNC�ʒm
 */
void FASTCALL vsync_notify(void)
{
	bDrawVsync = TRUE;
	bPollVsync = TRUE;
}

#if defined(DEBUG)
/*
 *	���s���ԑ���J�n
 */
void FASTCALL StartCpuTime(void)
{
	if (lFreq) {
		QueryPerformanceCounter((LARGE_INTEGER *)&lStartTime);
	}
}

/*
 *	���s���ԑ���I��
 */
DWORD FASTCALL GetCpuTime(void)
{
	LONGLONG lTempTime;

	if (lFreq) {
		QueryPerformanceCounter((LARGE_INTEGER *)&lTempTime);
		return (DWORD)(((lTempTime - lStartTime) * 1000000) / lFreq);
	}
	else {
		return (DWORD)0;
	}
}
#endif

/*
 *	1ms���s
 */
void FASTCALL ExecSch(void)
{
	DWORD dwCount;
	DWORD dwExec;

#if defined(DEBUG)
	StartCpuTime();
#endif

	/* �|�[�����O */
	PollKbd();
	if (bPollVsync) {
		PollJoy();
#ifdef MOUSE
		PollMos();
#endif
		bPollVsync = FALSE;
	}

#if defined(DEBUG)
	dwPollExecTime += GetCpuTime();

	StartCpuTime();
#endif

	/* �T�E���h���� */
	ProcessSnd(FALSE);

	dwCount = 1000;
	while (dwCount > 0) {
		/* ���~�v�����オ���Ă���΁A�����Ƀ��^�[�� */
		if (stopreq_flag) {
			run_flag = FALSE;
			break;
		}

		/* �����Ŏ��s */
		dwExec = schedule_exec(dwCount);
		dwCount -= dwExec;
		dwSoundTotal += dwExec;
	}

	/* �g�[�^���^�C������ */
	dwExecTotal += (1000 - dwCount);

#if defined(DEBUG)
	dwCpuExecTime += GetCpuTime();
#endif
}

/*
 *	�`��
 */
static void FASTCALL DrawSch(void)
{
	HDC hDC;

#if defined(DEBUG)
	StartCpuTime();
#endif

	/* �h���[�E�C���h�E */
	hDC = GetDC(hDrawWnd);
	OnDraw(hDrawWnd, hDC);
	ReleaseDC(hDrawWnd, hDC);

	/* �T�u�E�C���h�E�̍X�V */
	if (!bVistaflag) {
		DrawSubWindow();
	}

#if defined(DEBUG)
	dwDrawExecTime += GetCpuTime();
#endif

	/* �J�E���^�A�b�v */
	dwDrawTotal++;

#if defined(DEBUG)
	dwCpuExecTime = 0;
	dwDrawExecTime = 0;
	dwPollExecTime = 0;
#endif
}

/*
 *	�T�u�E�C���h�E�`��
 */
void FASTCALL DrawSubWindow(void)
{
	/* �T�u�E�C���h�E(Sync���̂�) */
	if (bSync) {
		RefreshBreakPoint();
		RefreshScheduler();
		RefreshCPURegister();
		if (bSyncDisasm[0] && run_flag) {
			AddrDisAsm(MAINCPU, maincpu.pc);
		}
		if (bSyncDisasm[1] && run_flag) {
			AddrDisAsm(SUBCPU, subcpu.pc);
		}
#if XM7_VER == 1
#if defined(JSUB)
		if (bSyncDisasm[2] && run_flag) {
			AddrDisAsm(JSUBCPU, jsubcpu.pc);
		}
#endif
#if defined(Z80CARD)
		if (bSyncDisasm[3] && run_flag) {
			AddrDisAsm(MAINZ80, (WORD)mainz80.pc);
		}
#endif
#endif
		RefreshDisAsm();
		RefreshMemory();

		RefreshFDC();
		RefreshOPNReg();
		RefreshOPNDisp();
		RefreshPaletteReg();
		RefreshSubCtrl();
		RefreshKeyboard();
		RefreshMMR();
#if XM7_VER == 1
#if defined(BUBBLE)
		RefreshBMC();
#endif
#endif
#if XM7_VER >= 2
		RefreshALULine();
#endif
#if XM7_VER >= 3
		RefreshDMAC();
#endif
	}
}

/*
 *	���s���Z�b�g
 *	��VM�̃��b�N�͍s���Ă��Ȃ��̂Œ���
 */
void FASTCALL ResetSch(void)
{
	nFrameSkip = 0;
	dwExecTime = timeGetTime();
}

/*
 *	���x�������Z�b�g
 */
void FASTCALL ResetSpeedAdjuster(void)
{
	nSpeedCheck = 0;
	dwSleepCount = 0;
	dwChkTime = timeGetTime();
}

/*
 *	�X���b�h�֐�
 */
static UINT WINAPI ThreadSch(LPVOID param)
{
	BOOL fast_mode;
	int tmp;

	UNUSED(param);

	/* ������ */
	ResetSch();
	ResetSpeedAdjuster();

	/* �������[�v(�N���[�Y�w��������ΏI��) */
	while (!bCloseReq) {
		/* �����Ȃ胍�b�N */
		LockVM();

		/* ���s�w�����ω��������`�F�b�N */
		if (bRunningBak != run_flag) {
			bRunningBak = run_flag;

#ifdef ROMEO
			/* YMF288���~���[�g */
			ROMEO_Mute(!run_flag);
#endif

#ifdef MOUSE
			/* �u���[�N�|�C���g��~���̃}�E�X�J�[�\���΍�(--; */
			if (hMainWnd && !run_flag && mos_capture && !bFullScreen) {
				PostMessage(hMainWnd, WM_USER + 2, 0, 0);
			}
#endif
		}

		/* ���s�w�����Ȃ���΁A�X���[�v */
		if (!run_flag) {
			/* ����������ăX���[�v */
			ProcessSnd(TRUE);
			UnlockVM();
			Sleep(10);
			ResetSch();
			ResetSpeedAdjuster();
			continue;
		}

		/* ���Z�b�g���̓J�E���^�ނ������� */
		if (reset_flag) {
			ResetSpeedAdjuster();
#ifdef ROMEO
			if (bRomeo) {
				/* ���łɂ�݂����Z�b�g */
				juliet_YMF288Reset();
			}
#endif
			reset_flag = FALSE;
		}

		/* ���Ԃ��擾(49���ł̃��[�v���l��) */
		dwNowTime = timeGetTime();
		dwTempTime = dwNowTime;
		if (dwTempTime < dwExecTime) {
			dwExecTime = 0;
		}

		/* ���Ԃ��r */
		if (dwTempTime <= dwExecTime) {
			/* ���Ԃ��]���Ă��邪�A�`��ł��邩 */
			if (bDrawVsync) {
				DrawSch();
				nFrameSkip = 0;
				bDrawVsync = FALSE;
			}

			/* �ēx�A���Ԃ��擾(49���ł̃��[�v���l��) */
			dwNowTime = timeGetTime();
			dwTempTime = dwNowTime;
			if (dwTempTime < dwExecTime) {
				dwExecTime = 0;
			}
			if (dwTempTime > dwExecTime) {
				UnlockVM();
				continue;
			}

			/* �������[�h��Ԃ��ω�������A�X���b�h�D�揇�ʂ�ύX */
			if (bNTflag) {
#if defined(DISABLE_FULLSPEED)
				fast_mode = (tape_motor && bTapeFullSpeed);
#else
				fast_mode = ((tape_motor && bTapeFullSpeed) || bFullSpeed);
#endif
				if (fast_mode != bFastMode) {
					bFastMode = fast_mode;
					if (fast_mode) {
						SetThreadPriority(
							hThread, THREAD_PRIORITY_BELOW_NORMAL);
					}
					else {
						SetThreadPriority(hThread, nPriority);
					}
				}
			}

			/* ���Ԃɗ]�T������̂ŁA�e�[�v�������[�h���� */
			if ((!tape_motor || !bTapeFullSpeed) || !bTapeModeType) {
				dwSleepCount ++;
#if defined(DISABLE_FULLSPEED)
				if (tape_motor && bTapeFullSpeed && !bTapeModeType) {
#else
				if (bFullSpeed || (tape_motor && bTapeFullSpeed &&
					!bTapeModeType)) {
#endif
					/* �S�͋쓮���[�h ���܂������Ԃ�CPU�𓮂��� */
					UnlockVM();

					/* �Ƃ肠�������̃v���Z�X�ɐ�����ڂ� */
					if (bNTflag) {
						Sleep(0);
					}

					while (!stopreq_flag) {
						if (dwTempTime != timeGetTime()) {
							break;
						}

						/* �e�[�v�������[�h���̓��C���̂ݑS�͋쓮 */
#if defined(DISABLE_FULLSPEED)
						if (tape_motor && bTapeFullSpeed) {
#else
						if (tape_motor && bTapeFullSpeed && !bFullSpeed) {
#endif
							schedule_main_fullspeed();
						}
						else {
							schedule_fullspeed();
						}
					}
					continue;
				}
				else {
					UnlockVM();
					Sleep(1);
					continue;
				}
			}

			/* �e�[�v�������[�h */
			dwExecTime = dwTempTime - 1;
			if (dwExecTime > dwTempTime) {
				dwExecTime++;
			}
		}

		/* ���s */
		ExecSch();
		nFrameSkip++;
		nSpeedCheck++;
		dwExecTime++;

		/* �������x���� */
		if (nSpeedCheck >= 200) {
#if defined(DISABLE_FULLSPEED)
			speed_ratio = 10000;
#else
			if (bAutoSpeedAdjust) {
				/* ���Ԃ��擾(49���ł̃��[�v���l��) */
				dwTempTime = timeGetTime();
				if (dwTempTime < dwChkTime) {
					dwChkTime = 0;
				}

				/* ���x�����Ԋu+�X���[�v���ԂƎ��ۂ̎��s���Ԃ̔䗦�����߂� */
				speed_ratio = (nSpeedCheck + dwSleepCount) * speed_ratio;
				speed_ratio /= (dwTempTime - dwChkTime);

				/* CPU���x�䗦��100%����5%�̊Ԃɐ������� */
				if (speed_ratio > 10000) {
					speed_ratio = 10000;
				}
				else if (speed_ratio < 500) {
					speed_ratio = 500;
				}
			}
			else {
				/* ���x������������100%�Œ� */
				speed_ratio = 10000;
			}
#endif

			/* �J�E���^�ނ������� */
			ResetSpeedAdjuster();
		}

		/* �I���΍�ŁA�����Ŕ����� */
		if (bCloseReq) {
			UnlockVM();
			break;
		}

		/* Break�΍� */
		if (!run_flag) {
			DrawSch();
			bDrawVsync = FALSE;
			nFrameSkip = 0;
			UnlockVM();
			continue;
		}

#if defined(DISABLE_FULLSPEED)
		tmp = 500;
#else
		/* �X�L�b�v�J�E���^���K��l�ȉ��Ȃ�A�����Ď��s */
		if (bAutoSpeedAdjust) {
			tmp = (10000 - speed_ratio) / 10;

			/* 2fps�`15fps/30fps�̊Ԃɐ��� */
#if XM7_VER >= 2
#if XM7_VER >= 3
			if (screen_mode & SCR_ANALOG) {
#else
			if (mode320) {
#endif
				/* 4096�F/26���F���[�h�ł͍ō�15fps */
				if (tmp < 66) {
					tmp = 66;
				}
			}
			else {
				/* 8�F���[�h�ł͍ō�30fps */
				if (tmp < 33) {
					tmp = 33;
				}
			}
#else
			if (tmp < 33) {
				tmp = 33;
			}
#endif
			if (tmp > 500) {
				tmp = 500;
			}
		}
		else {
			tmp = 500;
		}
#endif

		if (nFrameSkip >= tmp) {
			/* ���`�悪�����Ă���̂ŁA�����ň��`�� */
			DrawSch();
			ResetSch();
			bDrawVsync = FALSE;
		}
		UnlockVM();
	}

	/* �I���𖾎����邽�߁A�v���t���O���~�낷 */
	bCloseReq = FALSE;

	/* �X���b�h�I�� */
	_endthreadex(TRUE);

	return TRUE;
}

#endif	/* _WIN32 */
