/*
 *	FM-7 EMULATOR "XM7"
 *
 *	Copyright (C) 1999-2017 �o�h�D(Twitter:@xm6_original)
 *	Copyright (C) 2001-2017 Ryu Takegami (Twitter:@RyuTakegami)
 *
 *	[ �X�P�W���[�� ]
 *
 *	RHG����
 *	  2001.11.19		���X�e�[�g�t�@�C��(�C�x���g16�^�C�v)�̃��[�h�ɑΉ�
 *	  2001.11.24		�T�uHALT�^�C�~���O��ύX
 *	  2002.03.06		VM���z���ԃJ�E���^(�@��ˑ����Ƃ͕�)��ǉ�
 *	  2002.03.07		�ݒ�\�u���[�N�|�C���g����16�ɕύX
 *	  2002.06.21		VRAM�A�N�Z�X�t���OON���̃T�uCPU���x��^����(^^;)������
 *						��3����1�ɕύX
 *	  2002.07.17		���Z�b�g����RTC�C�x���g�̂݃N���A���Ȃ��悤�ɕύX
 *	  2002.08.03		�S�͋쓮���s����DMAC�𓮂����Ȃ��悤�ɕύX
 *	  2002.11.12		���s�T�C�N�����̗��_�l�Ǝ��ۂ̒l�𒲐����鏈����ǉ�
 *						�T�uHALT���̎��s�T�C�N���ݒ���@��ύX
 *						�T�uCPU�����߂����s�������邱�Ƃ���������C��
 *	  2002.12.17		FDD�E�F�C�g���[�h���A�f�B�X�N���쒆�͑S�͋쓮���Ȃ��悤
 *						�ɕύX
 *	  2003.05.02		�T�uCPU�̃f�t�H���g���s�T�C�N������ύX
 *	  2003.06.03		���C��CPU�E�T�uCPU�̃f�t�H���g���s�T�C�N�����������
 *	  2003.06.19		MMR�֘A���W�X�^�ύX���̑��x�␳�𓱓�
 *	  2003.11.21		XM7 V1.1�Ή�
 *						�u���[�N�|�C���g�����CPU�`�F�b�N��������
 *	  2004.01.24		�T�E���h�����̎��Ԑ��x������
 *	  2004.01.25		�e�[�v�����j�^������tapelp.c����ړ�
 *	  2008.01.20		���Ȃ��������Ƃɂ���
 *	  2012.04.20		���s�T�C�N����/�T�C�N���X�`�[�����[�h�̃X�e�[�g�f�[�^��
 *						�̕ۑ�/���A�@�\������
 *						���炭�g�r�s���ɂȂ��Ă���exec0��p�~
 *	  2017.03.07		���C��CPU�̃f�t�H���g���s�T�C�N�����������
 *						�ᑬ���[�h�̑��x�����@�ɋ߂����邽�߂̃`���[�j���O
 */

#include <string.h>
#include "xm7.h"
#include "subctrl.h"
#include "display.h"
#include "mmr.h"
#include "fdc.h"
#include "jsubsys.h"
#if XM7_VER >= 3
#include "dmac.h"
#endif
#include "device.h"
#include "event.h"
#include "tapelp.h"

/*
 *	�O���[�o�� ���[�N
 */
BOOL run_flag;							/* ���쒆�t���O */
BOOL stopreq_flag;						/* ��~�v���t���O */
event_t event[EVENT_MAXNUM];			/* �C�x���g �f�[�^ */
breakp_t breakp[BREAKP_MAXNUM];			/* �u���[�N�|�C���g �f�[�^ */
WORD main_runadr;						/* ���C��CPU�̑O����s�A�h���X */
WORD sub_runadr;						/* �T�uCPU�̑O����s�A�h���X */
DWORD main_speed;						/* ���C��CPU�X�s�[�h */
DWORD mmr_speed;						/* ���C��(MMR)�X�s�[�h */
#if XM7_VER >= 3
DWORD fmmr_speed;						/* ���C��(����MMR)�X�s�[�h */
#endif
DWORD sub_speed;						/* �T�uCPU�X�s�[�h */
WORD main_overcycles;					/* ���C��CPU�I�[�o�[�T�C�N�� */
WORD sub_overcycles;					/* �T�uCPU�I�[�o�[�T�C�N�� */
BOOL cycle_steal;						/* �T�C�N���X�`�[���t���O */
BOOL cycle_steal_default;				/* �T�C�N���X�`�[���f�t�H���g�t���O */
BOOL subclock_mode;						/* �T�uCPU ��u�����L���O���^�C�~���O */
DWORD speed_ratio;						/* CPU���쑬�x(%) */
DWORD vmtime;							/* VM���z���� */
#if XM7_VER == 1
DWORD main_speed_low;					/* ���C��CPU�X�s�[�h(�ᑬ) */
DWORD sub_speed_low;					/* �T�uCPU�X�s�[�h(�ᑬ) */
BOOL motoron_lowspeed;					/* CMT�����[ON�������ᑬ���[�h */
#if defined(JSUB)
DWORD jsub_speed;						/* ���{��T�uCPU�X�s�[�h */
WORD jsub_overcycles;					/* ���{��T�uCPU�I�[�o�[�T�C�N�� */
WORD jsub_runadr;						/* ���{��T�uCPU�̑O����s�A�h���X */
#endif
#if defined(Z80CARD)
WORD mainz80_runadr;					/* ���C��CPU(Z80)�̑O����s�A�h���X */
#endif
#endif

/*
 *	�X�^�e�B�b�N ���[�N
 */
static BOOL break_flag;					/* �u���[�N�|�C���g�L���t���O */

/*
 *	�X�P�W���[��
 *	������
 */
BOOL FASTCALL schedule_init(void)
{
	run_flag = FALSE;
	stopreq_flag = FALSE;
	break_flag = FALSE;
	memset(breakp, 0, sizeof(breakp));
	memset(event, 0, sizeof(event));

	/* CPU���x�����ݒ� */
	main_speed = MAINCYCLES * 10;
	mmr_speed = MAINCYCLES_MMR * 10;
#if XM7_VER >= 3
	fmmr_speed = MAINCYCLES_FMMR * 10;
#endif
	sub_speed = SUBCYCLES * 10;
	cycle_steal = TRUE;
	cycle_steal_default = TRUE;
#if XM7_VER == 1
	subclock_mode = TRUE;
#else
	subclock_mode = FALSE;
#endif
	main_overcycles = 0;
	sub_overcycles = 0;
#if XM7_VER == 1
	main_speed_low = MAINCYCLES_LOW * 10;
	sub_speed_low = SUBCYCLES_LOW * 10;
	motoron_lowspeed = TRUE;
#if defined(JSUB)
	jsub_speed = JSUBCYCLES * 10;
	jsub_overcycles = 0;
#endif
#endif

	/* ���z���ԏ����� */
	vmtime = 0;

	return TRUE;
}

/*
 *	�X�P�W���[��
 *	�N���[���A�b�v
 */
void FASTCALL schedule_cleanup(void)
{
}

/*
 *	�X�P�W���[��
 *	���Z�b�g
 */
void FASTCALL schedule_reset(void)
{
	int i;

	/* RTC�C�x���g�ȊO�̃X�P�W���[�����N���A */
	/* (XM7�N�������RTC�C�x���g�� schedule_init �ŃN���A����Ă���) */
	for (i=0; i<EVENT_MAXNUM; i++) {
		if (i != EVENT_RTC) {
			memset(&event[i], 0, sizeof(event_t));
		}
	}

	/* �J�E���^�N���A */
	maincpu.total = 0;
	subcpu.total = 0;
	main_overcycles = 0;
	sub_overcycles = 0;
#if XM7_VER == 1
#if defined(JSUB)
	jsubcpu.total = 0;
	jsub_overcycles = 0;
#endif
#endif

	/* �O��̎��s�A�h���X�������� */
	main_runadr = 0xFFFF;
	sub_runadr = 0xFFFF;
#if XM7_VER == 1
#if defined(JSUB)
	jsub_runadr = 0xFFFF;
#endif
#if defined(Z80CARD)
	mainz80_runadr = 0xFFFF;
#endif
#endif

	/* ���z���ԏ����� */
	vmtime = 0;

	/* ���s���x�䗦������ */
	speed_ratio = 10000;

	/* �T�C�N���X�`�[���ݒ菉���� */
	cycle_steal = cycle_steal_default;
}

/*
 *	�X�P�W���[��
 *	���s�T�C�N�����擾
 */
DWORD FASTCALL schedule_get_cycle(void)
{
	DWORD tmp;

#if XM7_VER >= 3
	if (mmr_fastmode) {
		tmp = fmmr_speed;
	}
	else
#endif
#if XM7_VER == 1
	if (lowspeed_mode ||
		((fm_subtype == FMSUB_FM8) && tape_motor && motoron_lowspeed)) {
		tmp = main_speed_low;
	}
	else {
#else
	{
#endif
		if (mmr_flag || twr_flag) {
			tmp = mmr_speed;
		}
		else {
			tmp = main_speed;
		}
	}

#if XM7_VER >= 3
	/* �������t���b�V�����[�h�̃T�C�N�����𐶐� */
	if (!mmr_fastmode && mmr_fast_refresh) {
		if (mmr_flag || twr_flag) {
			/* MMR/TWR�L���� ��8.9%�A�b�v */
			tmp = (DWORD)((tmp * 4461) >> 12);
		}
		else {
			/* MMR/TWR������ ��8.6%�A�b�v */
			tmp = (DWORD)((tmp * 4447) >> 12);
		}
	}
#endif

	/* CPU���x�䗦 */
	if (speed_ratio != 10000) {
		tmp = (tmp * speed_ratio) / 10000;
		if (tmp < 1) {
			tmp = 1;
		}
	}

	return tmp;
}

/*-[ �C�x���g ]-------------------------------------------------------------*/

/*
 *	�X�P�W���[��
 *	�C�x���g�ݒ�
 */
BOOL FASTCALL schedule_setevent(int id, DWORD microsec, BOOL (FASTCALL *func)(void))
{
	DWORD exec;

	ASSERT((id >= 0) && (id < EVENT_MAXNUM));
	ASSERT(func);

	if ((id < 0) || (id >= EVENT_MAXNUM)) {
		return FALSE;
	}
	if (microsec == 0) {
		event[id].flag = EVENT_NOTUSE;
		return FALSE;
	}

	/* �o�^ */
	event[id].current = microsec;
	event[id].reload = microsec;
	event[id].callback = func;
	event[id].flag = EVENT_ENABLED;

	/* ���s���Ȃ�A���Ԃ𑫂��Ă����K�v������(��ň�������) */
	if (run_flag) {
		exec = (DWORD)maincpu.total;
		exec *= 10000;
		exec /= schedule_get_cycle();
		event[id].current += exec;
	}

	return TRUE;
}

/*
 *	�X�P�W���[��
 *	�C�x���g�폜
 */
BOOL FASTCALL schedule_delevent(int id)
{
	ASSERT((id >= 0) && (id < EVENT_MAXNUM));

	if ((id < 0) || (id >= EVENT_MAXNUM)) {
		return FALSE;
	}

	/* ���g�p�� */
	event[id].flag = EVENT_NOTUSE;

	return TRUE;
}

/*
 *	�X�P�W���[��
 *	�C�x���g�n���h���ݒ�
 */
void FASTCALL schedule_handle(int id, BOOL (FASTCALL *func)(void))
{
	ASSERT((id >= 0) && (id < EVENT_MAXNUM));
	ASSERT(func);

	/* �R�[���o�b�N�֐���o�^����̂݁B����ȊO�͐G��Ȃ� */
	event[id].callback = func;
}

/*
 *	�X�P�W���[��
 *	�ŒZ���s���Ԓ���
 */
static DWORD FASTCALL schedule_chkevent(DWORD microsec)
{
	DWORD exectime;
	int i;

	/* �����ݒ� */
	exectime = microsec;

	/* �C�x���g������Ē��� */
	for (i=0; i<EVENT_MAXNUM; i++) {
		if (event[i].flag == EVENT_NOTUSE) {
			continue;
		}

		ASSERT(event[i].current > 0);
		ASSERT(event[i].reload > 0);

		if (event[i].current < exectime) {
			exectime = event[i].current;
		}
	}

	return exectime;
}

/*
 *	�X�P�W���[��
 *	�i�s����
 */
static void FASTCALL schedule_doevent(DWORD microsec)
{
	int i;

	for (i=0; i<EVENT_MAXNUM; i++) {
		if (event[i].flag == EVENT_NOTUSE) {
			continue;
		}

		ASSERT(event[i].current > 0);
		ASSERT(event[i].reload > 0);

		/* ���s���Ԃ����� */
		if (event[i].current < microsec) {
			event[i].current = 0;
		}
		else {
			event[i].current -= microsec;
		}

		/* �J�E���^��0�Ȃ� */
		if (event[i].current == 0) {
			/* ���Ԃ�ENABLE,DISABLE�ɂ�����炸�����[�h */
			event[i].current = event[i].reload;
			/* �R�[���o�b�N���s */
			if (event[i].flag == EVENT_ENABLED) {
				if (!event[i].callback()) {
					event[i].flag = EVENT_DISABLED;
				}
			}
		}
	}
}

/*-[ �u���[�N�|�C���g ]-----------------------------------------------------*/

/*
 *	�X�P�W���[��
 *	�u���[�N�|�C���g�Z�b�g(���łɃZ�b�g���Ă���Ώ���)
 */
BOOL FASTCALL schedule_setbreak(int cpu, WORD addr)
{
	int i;

	/* �܂��A�S�Ẵu���[�N�|�C���g���������A�����邩 */
	for (i=0; i<BREAKP_MAXNUM; i++) {
		if (breakp[i].flag != BREAKP_NOTUSE) {
			if ((breakp[i].cpu == cpu) && (breakp[i].addr == addr)) {
				break;
			}
		}
	}
	/* ������΁A�폜 */
	if (i != BREAKP_MAXNUM) {
		breakp[i].flag = BREAKP_NOTUSE;
		/* �u���[�N�L���t���O���`�F�b�N */
		break_flag = FALSE;
		for (i=0; i<BREAKP_MAXNUM; i++) {
			if (breakp[i].flag != BREAKP_NOTUSE) {
				break_flag = TRUE;
			}
		}
		return TRUE;
	}

	/* �󂫂𒲍� */
	for (i=0; i<BREAKP_MAXNUM; i++) {
		if (breakp[i].flag == BREAKP_NOTUSE) {
			break;
		}
	}
	/* ���ׂĖ��܂��Ă��邩 */
	if (i == BREAKP_MAXNUM) {
		return FALSE;
	}

	/* �Z�b�g */
	breakp[i].flag = BREAKP_ENABLED;
	breakp[i].cpu = cpu;
	breakp[i].addr = addr;

	/* �u���[�N�L�� */
	break_flag = TRUE;

	return TRUE;
}

/*
 *	�X�P�W���[��
 *	�u���[�N�|�C���g�Z�b�g(�ʒu�w��)
 */
BOOL FASTCALL schedule_setbreak2(int num, int cpu, WORD addr)
{
	/* �Z�b�g */
	if (breakp[num].flag != BREAKP_DISABLED) {
		breakp[num].flag = BREAKP_ENABLED;
	}
	breakp[num].cpu = cpu;
	breakp[num].addr = addr;

	/* �u���[�N�L�� */
	break_flag = TRUE;

	return TRUE;
}

/*
 *	�X�P�W���[��
 *	�u���[�N�`�F�b�N
 */
static BOOL FASTCALL schedule_chkbreak(void)
{
	int i;
	WORD main_prevrunadr;
	WORD sub_prevrunadr;
#if XM7_VER == 1
#if defined(JSUB)
	WORD jsub_prevrunadr;
#endif
#if defined(Z80CARD)
	WORD mainz80_prevrunadr;
#endif
#endif

	ASSERT(break_flag);

	/* �����A��₱�����c */
#if XM7_VER == 1 && defined(Z80CARD)
	if (!main_z80mode) {
		main_prevrunadr = main_runadr;
		main_runadr = maincpu.pc;
	}
	else {
		mainz80_prevrunadr = mainz80_runadr;
		mainz80_runadr = (WORD)mainz80.pc;
	}
#else
	main_prevrunadr = main_runadr;
	main_runadr = maincpu.pc;
#endif
	sub_prevrunadr = sub_runadr;
	sub_runadr = subcpu.pc;
#if XM7_VER == 1
#if defined(JSUB)
	jsub_prevrunadr = jsub_runadr;
	jsub_runadr = jsubcpu.pc;
#endif
#endif

	for (i=0; i<BREAKP_MAXNUM; i++) {
		if (breakp[i].flag == BREAKP_ENABLED) {
#if XM7_VER == 1 && defined(Z80CARD)
			/* ���C��CPU��Z80�̏ꍇ�A�������� */
			if ((breakp[i].cpu == MAINCPU) && !main_z80mode) {
#else
			if (breakp[i].cpu == MAINCPU) {
#endif
				if (breakp[i].addr == maincpu.pc) {
					if (maincpu.pc != main_prevrunadr) {
						return TRUE;
					}
				}
			}
			else if (breakp[i].cpu == SUBCPU) {
				if (breakp[i].addr == subcpu.pc) {
					if (subcpu.pc != sub_prevrunadr) {
						return TRUE;
					}
				}
			}
#if XM7_VER == 1
#if defined(JSUB)
			else if (breakp[i].cpu == JSUBCPU) {
				if (breakp[i].addr == jsubcpu.pc) {
					if (jsubcpu.pc != jsub_prevrunadr) {
						return TRUE;
					}
				}
			}
#endif
#if defined(Z80CARD)
			/* ���C��CPU��Z80�̏ꍇ�̂ݗL�� */
			else if ((breakp[i].cpu == MAINZ80) && main_z80mode) {
				if (breakp[i].addr == (WORD)mainz80.pc) {
					if ((WORD)mainz80.pc != mainz80_prevrunadr) {
						return TRUE;
					}
				}
			}
#endif
#endif
		}
	}

	return FALSE;
}

/*-[ ���s�� ]---------------------------------------------------------------*/

/*
 *	�g���[�X
 */
void FASTCALL schedule_trace(void)
{
	/* �P���ߎ��s */
#if XM7_VER >= 3
	if (!dma_burst_transfer || !dma_flag) {
		maincpu_execline();
	}
#else
	maincpu_execline();
#endif

#if XM7_VER >= 3
	/* DMA�]�� */
	if (dma_flag) {
		dmac_exec();
	}
#endif

	if ((!subhalt_flag || (subcpu.intr & INTR_HALT)) &&
		(cycle_steal || (subclock_mode || !(vrama_flag && !blank_flag)))) {
		subcpu_execline();
		/* VRAM�A�N�Z�X�t���OON�̏ꍇ,���v�N���b�N���3�{�ɂ��� */
		if (!cycle_steal && subclock_mode && vrama_flag) {
			subcpu.total += (WORD)(subcpu.cycle * 1.86f);
		}
	}
#if XM7_VER == 1
#if defined(JSUB)
	if (jsub_available && jsub_enable && !jsub_haltflag &&
		(fm_subtype != FMSUB_FM8)) {
		jsubcpu_execline();
	}
#endif
#endif

	/* HALT�v���ɉ��� (V3.1) */
	subctrl_halt_ack();

	/* �I�[�o�[�T�C�N�������N���A */
	main_overcycles = 0;
	sub_overcycles = 0;
#if XM7_VER == 1
#if defined(JSUB)
	jsub_overcycles = 0;
#endif
#endif
}

/*
 *	���C�������S�͋쓮
 */
void FASTCALL schedule_main_fullspeed(void)
{
	ASSERT(run_flag);
	ASSERT(!stopreq_flag);

#if XM7_VER >= 3
	if (dma_burst_transfer) {
		/* �o�[�X�g�]�����s���B���C��CPU�͓����Ȃ� */
		return;
	}
#endif

	if (break_flag) {
		/* �u���[�N�|�C���g���� */
		if (schedule_chkbreak()) {
			stopreq_flag = TRUE;
		}
		if (stopreq_flag) {
			return;
		}
	}

	/* ���C��CPU���s */
#if defined(FDDSND)
	if (!fdc_waitmode || !(fdc_status & FDC_ST_BUSY)) {
		maincpu_exec();
	}
#else
	maincpu_exec();
#endif

	/* HALT�v���ɉ��� (V3.1) */
	subctrl_halt_ack();

	/* �I�[�o�[�T�C�N�������N���A */
	main_overcycles = 0;
}

/*
 *	�S�͋쓮
 */
void FASTCALL schedule_fullspeed(void)
{
	ASSERT(run_flag);
	ASSERT(!stopreq_flag);

	if (break_flag) {
		/* �u���[�N�|�C���g���� */
		if (schedule_chkbreak()) {
			stopreq_flag = TRUE;
		}
		if (stopreq_flag) {
			return;
		}
	}

	/* ���C��CPU���s */
#if XM7_VER >= 3
	if (!dma_burst_transfer) {
#if defined(FDDSND)
		if (!fdc_waitmode || !(fdc_status & FDC_ST_BUSY)) {
			maincpu_exec();
		}
#else
		maincpu_exec();
#endif
	}
#else
#if defined(FDDSND)
	if (!fdc_waitmode || !(fdc_status & FDC_ST_BUSY)) {
		maincpu_exec();
	}
#else
	maincpu_exec();
#endif
#endif

	/* �T�uCPU���s */
	if ((!subhalt_flag || (subcpu.intr & INTR_HALT)) &&
		(cycle_steal || (subclock_mode || !(vrama_flag && !blank_flag)))) {
		subcpu_execline();
		/* VRAM�A�N�Z�X�t���OON�̏ꍇ,���v�N���b�N���3�{�ɂ��� */
		if (!cycle_steal && subclock_mode && vrama_flag) {
			subcpu.total += (WORD)(subcpu.cycle * 1.86f);
		}
	}

#if XM7_VER == 1
#if defined(JSUB)
	/* ���{��T�uCPU���s */
	if (jsub_available && jsub_enable && !jsub_haltflag &&
		(fm_subtype != FMSUB_FM8)) {
		jsubcpu_execline();
	}
#endif
#endif

	/* HALT�v���ɉ��� (V3.1) */
	subctrl_halt_ack();

	/* �I�[�o�[�T�C�N�������N���A */
	main_overcycles = 0;
	sub_overcycles = 0;
#if XM7_VER == 1
#if defined(JSUB)
	jsub_overcycles = 0;
#endif
#endif
}

/*
 *	���s
 */
DWORD FASTCALL schedule_exec(DWORD microsec)
{
	DWORD exec;
	DWORD exec2;
	DWORD count;
	DWORD cycle;
	DWORD ratio;
	WORD main;
	WORD sub;
	DWORD limit;
	DWORD tmp;
#if XM7_VER == 1
#if defined(JSUB)
	DWORD ratio_js;
	WORD jsub;
#endif
#endif
	extern DWORD dwSoundTotal;

	/* ASSERT(run_flag); */
	if (!run_flag) {
		return 0;
	}
	ASSERT(!stopreq_flag);

	/* �ŒZ�̎��s���Ԃ𓾂� */
	exec = schedule_chkevent(microsec);
	exec2 = 0;

	do {
		/* ���C��CPU�ƃT�uCPU�̓��쑬�x�䗦�����߂� */
		cycle = schedule_get_cycle();
#if XM7_VER == 1
		if (lowspeed_mode) {
			tmp = (sub_speed_low * speed_ratio) / 100000;
		}
		else {
			tmp = (sub_speed * speed_ratio) / 100000;
		}
#else
		tmp = (sub_speed * speed_ratio) / 100000;
#endif
		if (tmp < 1) {
			tmp = 1;
		}
		ratio = (tmp << 12);
		ratio /= (cycle / 10);
#if XM7_VER == 1
#if defined(JSUB)
		tmp = (jsub_speed * speed_ratio) / 100000;
		if (tmp < 1) {
			tmp = 1;
		}
		ratio_js = (tmp << 12);
		ratio_js /= (cycle / 10);
#endif
#endif

		/* CPU���ԂɊ��Z */
		count = cycle;
		count *= (exec - exec2);
		count /= 10000;
		main = (WORD)count;
		sub = (WORD)((main * ratio) >> 12);
#if XM7_VER == 1
#if defined(JSUB)
		jsub = (WORD)((main * ratio_js) >> 12);
#endif
#endif

		/* �J�E���^�E�t���O������ */
		maincpu.total = main_overcycles;
		subcpu.total = sub_overcycles;
#if XM7_VER == 1
#if defined(JSUB)
		jsubcpu.total = jsub_overcycles;
#endif
#endif
		mmr_modify = FALSE;

		if (cycle_steal) {
			if (break_flag) {
				/* �T�C�N���X�`�[������A�u���[�N�|�C���g���� */

				/* ���s */
				while ((maincpu.total < main) && !mmr_modify) {
					if (schedule_chkbreak()) {
						stopreq_flag = TRUE;
					}
					if (stopreq_flag) {
						break;
					}

#if XM7_VER >= 3
					/* ���C��CPU���s */
					if (!dma_burst_transfer) {
						maincpu_exec();
					}

					/* DMA�]�� */
					if (dma_flag) {
						dmac_exec();
					}
#else
					/* ���C��CPU���s */
					maincpu_exec();
#endif

					/* �T�u����CLR���߂��g�p����BUSY�t���O�𑀍삵���ꍇ�� */
					/* �㏈��(MAGUS) */
					if (busy_CLR_count) {
						busy_CLR_count --;
						if (busy_CLR_count == 0) {
							subbusy_flag = TRUE;
						}
					}

					/* �T�uCPU���s */
					limit = maincpu.total * ratio;
					if (subhalt_flag && !(subcpu.intr & INTR_HALT)) {
						/* ���C��CPU�Ƌ������� */
						subcpu.total = (WORD)(limit >> 12);
					}
					else {
						while (((DWORD)subcpu.total << 12) < limit) {
							if (schedule_chkbreak()) {
								stopreq_flag = TRUE;
							}
							if (stopreq_flag) {
								break;
							}
							subcpu_exec();
						}
					}

#if XM7_VER == 1
#if defined(JSUB)
					/* ���{��T�uCPU���s */
					limit = maincpu.total * ratio_js;
					while ((((DWORD)jsubcpu.total << 12) < limit) &&
							jsub_available && jsub_enable && !jsub_haltflag &&
							(fm_subtype != FMSUB_FM8)) {
						if (schedule_chkbreak()) {
							stopreq_flag = TRUE;
						}
						if (stopreq_flag) {
							break;
						}
						jsubcpu_exec();
					}
#endif
#endif
					/* HALT�v���ɉ��� (V3.1) */
					subctrl_halt_ack();
				}

				/* �u���[�N�����ꍇ�̏��� */
				if (stopreq_flag) {
					/* exec�����Ԃ�i�߂�(�����ĕ␳���Ȃ�) */
					run_flag = FALSE;
				}
			}
			else {
				/* �T�C�N���X�`�[������A�u���[�N�|�C���g�Ȃ� */

				/* ���s */
				while ((maincpu.total < main) && !mmr_modify) {
#if XM7_VER >= 3
					/* ���C��CPU���s */
					if (!dma_burst_transfer) {
						maincpu_exec();
					}

					/* DMA�]�� */
					if (dma_flag) {
						dmac_exec();
					}
#else
					/* ���C��CPU���s */
					maincpu_exec();
#endif

					/* �T�u����CLR���߂��g�p����BUSY�t���O�𑀍삵���ꍇ�� */
					/* �㏈��(MAGUS) */
					if (busy_CLR_count) {
						busy_CLR_count --;
						if (busy_CLR_count == 0) {
							subbusy_flag = TRUE;
						}
					}

					/* �T�uCPU���s */
					limit = maincpu.total * ratio;
					if (subhalt_flag && !(subcpu.intr & INTR_HALT)) {
						/* ���C��CPU�Ƌ������� */
						subcpu.total = (WORD)(limit >> 12);
					}
					else {
						while (((DWORD)subcpu.total << 12) < limit) {
							subcpu_exec();
						}
					}

#if XM7_VER == 1
#if defined(JSUB)
					/* ���{��T�uCPU���s */
					limit = maincpu.total * ratio_js;
					while ((((DWORD)jsubcpu.total << 12) < limit) &&
						jsub_available && jsub_enable && !jsub_haltflag &&
						(fm_subtype != FMSUB_FM8)) {
						jsubcpu_exec();
					}
#endif
#endif

					/* HALT�v���ɉ��� (V3.1) */
					subctrl_halt_ack();
				}
			}
		}
		else {
			if (break_flag) {
				/* �T�C�N���X�`�[���Ȃ��A�u���[�N�|�C���g���� */

				/* ���s */
				while ((maincpu.total < main) && !mmr_modify) {
					if (schedule_chkbreak()) {
						stopreq_flag = TRUE;
					}
					if (stopreq_flag) {
						break;
					}

#if XM7_VER >= 3
					/* ���C��CPU���s */
					if (!dma_burst_transfer) {
						maincpu_exec();
					}

					/* DMA�]�� */
					if (dma_flag) {
						dmac_exec();
					}
#else
					/* ���C��CPU���s */
					maincpu_exec();
#endif

					/* �T�u����CLR���߂��g�p����BUSY�t���O�𑀍삵���ꍇ�� */
					/* �㏈��(MAGUS) */
					if (busy_CLR_count) {
						busy_CLR_count --;
						if (busy_CLR_count == 0) {
							subbusy_flag = TRUE;
						}
					}

					/* �T�uCPU���s */
					limit = maincpu.total * ratio;
					if ((subhalt_flag && !(subcpu.intr & INTR_HALT)) ||
						(!subclock_mode && (vrama_flag && blank_flag))) {
						/* ���C��CPU�Ƌ������� */
						subcpu.total = (WORD)(limit >> 12);
					}
					else {
						while (((DWORD)subcpu.total << 12) < limit) {
							if (schedule_chkbreak()) {
								stopreq_flag = TRUE;
							}
							if (stopreq_flag) {
								break;
							}

							subcpu_exec();
							/* VRAM�A�N�Z�X�t���OON�̏ꍇ */
							/* ���v�N���b�N���3�{�ɂ��� */
							if (subclock_mode && vrama_flag) {
								subcpu.total += (WORD)(subcpu.cycle * 1.86f);
							}
						}
					}

#if XM7_VER == 1
#if defined(JSUB)
					/* ���{��T�uCPU���s */
					limit = maincpu.total * ratio_js;
					while ((((DWORD)jsubcpu.total << 12) < limit) &&
							jsub_available && jsub_enable && !jsub_haltflag &&
							(fm_subtype != FMSUB_FM8)) {
						if (schedule_chkbreak()) {
							stopreq_flag = TRUE;
						}
						if (stopreq_flag) {
							break;
						}
						jsubcpu_exec();
					}
#endif
#endif

					/* HALT�v���ɉ��� (V3.1) */
					subctrl_halt_ack();
				}

				/* �u���[�N�����ꍇ�̏��� */
				if (stopreq_flag) {
					/* exec�����Ԃ�i�߂�(�����ĕ␳���Ȃ�) */
					run_flag = FALSE;
				}
			}
			else {
				/* �T�C�N���X�`�[���Ȃ��A�u���[�N�|�C���g�Ȃ� */

				/* ���s */
				while ((maincpu.total < main) && !mmr_modify) {
#if XM7_VER >= 3
					/* ���C��CPU���s */
					if (!dma_burst_transfer) {
						maincpu_exec();
					}

					/* DMA�]�� */
					if (dma_flag) {
						dmac_exec();
					}
#else
					/* ���C��CPU���s */
					maincpu_exec();
#endif

					/* �T�u����CLR���߂��g�p����BUSY�t���O�𑀍삵���ꍇ�� */
					/* �㏈��(MAGUS) */
					if (busy_CLR_count) {
						busy_CLR_count --;
						if (busy_CLR_count == 0) {
							subbusy_flag = TRUE;
						}
					}

					/* �T�uCPU���s */
					limit = maincpu.total * ratio;
					if ((subhalt_flag && !(subcpu.intr & INTR_HALT)) ||
						(!subclock_mode && (vrama_flag && blank_flag))) {
						/* ���C��CPU�Ƌ������� */
						subcpu.total = (WORD)(limit >> 12);
					}
					else {
						while (((DWORD)subcpu.total << 12) < limit) {
							subcpu_exec();
							/* VRAM�A�N�Z�X�t���OON�̏ꍇ */
							/* ���v�N���b�N���3�{�ɂ��� */
							if (subclock_mode && vrama_flag) {
								subcpu.total += (WORD)(subcpu.cycle * 1.86f);
							}
						}
					}

#if XM7_VER == 1
#if defined(JSUB)
					/* ���{��T�uCPU���s */
					limit = maincpu.total * ratio_js;
					while ((((DWORD)jsubcpu.total << 12) < limit) &&
							jsub_available && jsub_enable && !jsub_haltflag &&
							(fm_subtype != FMSUB_FM8)) {
						jsubcpu_exec();
					}
#endif
#endif

					/* HALT�v���ɉ��� (V3.1) */
					subctrl_halt_ack();
				}
			}
		}

		/* MMR�֘A���W�X�^���ύX���ꂽ�ꍇ�A���s���Ԃ�␳ */
		if (mmr_modify) {
			exec2 += (maincpu.total * 10000) / cycle;
			main_overcycles = 0;
			sub_overcycles = 0;
#if XM7_VER == 1
#if defined(JSUB)
			jsub_overcycles = 0;
#endif
#endif
		}
	} while (mmr_modify && run_flag && (exec > exec2));

	/* �I�[�o�[�T�C�N������ */
	if (maincpu.total > main) {
		main_overcycles = (WORD)(maincpu.total - main);
	}
	else {
		main_overcycles = 0;
	}
	if (subcpu.total > sub) {
		sub_overcycles = (WORD)(subcpu.total - sub);
	}
	else {
		sub_overcycles = 0;
	}
#if XM7_VER == 1
#if defined(JSUB)
	if (jsubcpu.total > jsub) {
		jsub_overcycles = (WORD)(jsubcpu.total - jsub);
	}
	else {
		jsub_overcycles = 0;
	}
#endif
#endif

	/* �C�x���g���� */
	maincpu.total = 0;
	subcpu.total = 0;
#if XM7_VER == 1
#if defined(JSUB)
	jsubcpu.total = 0;
#endif
#endif
	schedule_doevent(exec);
	vmtime += exec;

	return exec;
}

/*-[ �t�@�C��I/O ]----------------------------------------------------------*/

/*
 *	�X�P�W���[��
 *	�Z�[�u
 */
BOOL FASTCALL schedule_save(int fileh)
{
	int i;

	if (!file_bool_write(fileh, run_flag)) {
		return FALSE;
	}

	if (!file_bool_write(fileh, stopreq_flag)) {
		return FALSE;
	}

	/* Ver901�g�� */
	if (!file_byte_write(fileh, BREAKP_MAXNUM)) {
		return FALSE;
	}

	/* �u���[�N�|�C���g */
	for (i=0; i<BREAKP_MAXNUM; i++) {
		if (!file_byte_write(fileh, (BYTE)breakp[i].flag)) {
			return FALSE;
		}
		if (!file_byte_write(fileh, (BYTE)breakp[i].cpu)) {
			return FALSE;
		}
		if (!file_word_write(fileh, breakp[i].addr)) {
			return FALSE;
		}
	}

	/* �C�x���g */

	/* Ver9�g�� */
	if (!file_byte_write(fileh, EVENT_MAXNUM)) {
		return FALSE;
	}

	for (i=0; i<EVENT_MAXNUM; i++) {
		/* �R�[���o�b�N�ȊO��ۑ� */
		if (!file_byte_write(fileh, (BYTE)event[i].flag)) {
			return FALSE;
		}
		if (!file_dword_write(fileh, event[i].current)) {
			return FALSE;
		}
		if (!file_dword_write(fileh, event[i].reload)) {
			return FALSE;
		}
	}

	/* Ver9.05/7.05�g�� */
	if (!file_word_write(fileh, main_overcycles)) {
		return FALSE;
	}
	if (!file_word_write(fileh, sub_overcycles)) {
		return FALSE;
	}

#if XM7_VER == 1
#if defined(JSUB)
	if (!file_word_write(fileh, jsub_overcycles)) {
		return FALSE;
	}
#endif
#endif

	/* Ver9.16/7.16/3.06�g�� */
	if (!file_bool_write(fileh, cycle_steal)) {
		return FALSE;
	}
	if (!file_dword_write(fileh, main_speed)) {
		return FALSE;
	}
	if (!file_dword_write(fileh, mmr_speed)) {
		return FALSE;
	}
#if XM7_VER >= 3
	if (!file_dword_write(fileh, fmmr_speed)) {
		return FALSE;
	}
#endif
	if (!file_dword_write(fileh, sub_speed)) {
		return FALSE;
	}
#if XM7_VER == 1
	if (!file_dword_write(fileh, main_speed_low)) {
		return FALSE;
	}
	if (!file_dword_write(fileh, sub_speed_low)) {
		return FALSE;
	}
#if defined(JSUB)
	if (!file_dword_write(fileh, jsub_speed)) {
		return FALSE;
	}
#endif
#endif

	return TRUE;
}

/*
 *	�X�P�W���[��
 *	���[�h
 */
#if XM7_VER == 1
BOOL FASTCALL schedule_load(int fileh, int ver)
#else
BOOL FASTCALL schedule_load(int fileh, int ver, BOOL old)
#endif
{
	int i;
	BYTE tmp;
	BYTE MAXNUM;

	/* �o�[�W�����`�F�b�N */
	if (ver < 200) {
		return FALSE;
	}

	if (!file_bool_read(fileh, &run_flag)) {
		return FALSE;
	}

	if (!file_bool_read(fileh, &stopreq_flag)) {
		return FALSE;
	}

	/* �u���[�N�|�C���g */
	/* Ver9�g�� */
#if XM7_VER >= 3
	if ((ver >= 901) || ((ver >= 701) && (ver <= 799))) {
#elif XM7_VER >= 2
	if (ver >= 701) {
#else
	if (ver >= 300) {
#endif
		if (!file_byte_read(fileh, &MAXNUM)) {
			return FALSE;
		}
	}
#if XM7_VER >= 2
	else {
		/* V1.1�ł͎�����g���Ȃ� */
		MAXNUM = BREAKP_MAXNUM_OLD;
	}
#endif

	/* �������񏉊������� */
	memset(breakp, 0, sizeof(breakp));

	for (i=0; i<MAXNUM; i++) {
		if (!file_byte_read(fileh, &tmp)) {
			return FALSE;
		}
		breakp[i].flag = (int)tmp;
		if (!file_byte_read(fileh, &tmp)) {
			return FALSE;
		}
		breakp[i].cpu = (int)tmp;
		if (!file_word_read(fileh, &breakp[i].addr)) {
			return FALSE;
		}
	}
	break_flag = FALSE;
	for (i=0; i<MAXNUM; i++) {
		if (breakp[i].flag != BREAKP_NOTUSE) {
			break_flag = TRUE;
		}
	}

	/* �C�x���g */
	/* Ver9�g�� */
#if XM7_VER >= 3
	if ((ver >= 900) || ((ver >= 700) && (ver <= 799))) {
#elif XM7_VER >= 2
	if (ver >= 700) {
#else
	if (ver >= 300) {
#endif
		if (!file_byte_read(fileh, &MAXNUM)) {
			return FALSE;
		}
	}
#if XM7_VER >= 2
	else {
		if (old) {
			MAXNUM = EVENT_MAXNUM_L30;
		}
		else {
			MAXNUM = EVENT_MAXNUM_L31;
		}
	}
#endif

	for (i=0; i<MAXNUM; i++) {
		/* �R�[���o�b�N�ȊO��ݒ� */
		if (!file_byte_read(fileh, &tmp)) {
			return FALSE;
		}
		event[i].flag = (int)tmp;
		if (!file_dword_read(fileh, &event[i].current)) {
			return FALSE;
		}
		if (!file_dword_read(fileh, &event[i].reload)) {
			return FALSE;
		}
	}

	/* Ver9.05/Ver7.05�g�� */
#if XM7_VER >= 3
	if ((ver >= 905) || ((ver >= 705) && (ver <= 799))) {
#elif XM7_VER >= 2
	if (ver >= 705) {
#else
	if (ver >= 300) {
#endif
		if (!file_word_read(fileh, &main_overcycles)) {
			return FALSE;
		}
		if (!file_word_read(fileh, &sub_overcycles)) {
			return FALSE;
		}
#if XM7_VER == 1
#if defined(JSUB)
		if (!file_word_read(fileh, &jsub_overcycles)) {
			return FALSE;
		}
#endif
#endif
	}
	else {
		main_overcycles = 0;
		sub_overcycles = 0;
#if XM7_VER == 1
#if defined(JSUB)
		jsub_overcycles = 0;
#endif
#endif
	}

	/* Ver9.16/7.16/3.06�g�� */
#if XM7_VER >= 3
	if ((ver >= 916) || ((ver >= 716) && (ver <= 799))) {
#elif XM7_VER >= 2
	if (ver >= 716) {
#else
	if (ver >= 306) {
#endif
		if (!file_bool_read(fileh, &cycle_steal)) {
			return FALSE;
		}
		if (!file_dword_read(fileh, &main_speed)) {
			return FALSE;
		}
		if (!file_dword_read(fileh, &mmr_speed)) {
			return FALSE;
		}
#if XM7_VER >= 3
		if (ver >= 916) {
			if (!file_dword_read(fileh, &fmmr_speed)) {
				return FALSE;
			}
		}
#endif
		if (!file_dword_read(fileh, &sub_speed)) {
			return FALSE;
		}
#if XM7_VER == 1
		if (!file_dword_read(fileh, &main_speed_low)) {
			return FALSE;
		}
		if (!file_dword_read(fileh, &sub_speed_low)) {
			return FALSE;
		}
#if defined(JSUB)
		if (!file_dword_read(fileh, &jsub_speed)) {
			return FALSE;
		}
#endif
#endif
	}

	/* ���s���x�䗦������ */
	speed_ratio = 10000;

	return TRUE;
}
