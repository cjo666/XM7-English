/*
 *	FM-7 EMULATOR "XM7"
 *
 *	Copyright (C) 1999-2017 �o�h�D(Twitter:@xm6_original)
 *	Copyright (C) 2001-2017 Ryu Takegami (Twitter:@RyuTakegami)
 *
 *	[ ���C��CPU ]
 *
 *	RHG����
 *	  2002.05.07		C�����CPU�R�A�ɍđΉ�
 *	  2003.11.21		XM7 V1.1�Ή��ɔ����A�e�[�v�J�E���^�̐��x������
 *	  2010.11.15		FM-8���[�h���Ƀt���b�s�[�����IRQ���֎~����悤�ύX
 */

#include "xm7.h"
#include "subctrl.h"
#include "keyboard.h"
#include "mainetc.h"
#include "tapelp.h"
#include "device.h"

/*
 *	�O���[�o�� ���[�N
 */
cpu6809_t maincpu;						/* ���C��CPU���W�X�^���(6809) */
#if XM7_VER == 1 && defined(Z80CARD)
KMZ80_CONTEXT mainz80;					/* ���C��CPU�R���e�L�X�g(Z80) */
BOOL main_z80mode;						/* ���C��CPU Z80�I���t���O */
DWORD z80_cycles;						/* Z80�T�C�N�����v�Z�p�J�E���^ */
DWORD z80_cycles_sub;					/* Z80�T�C�N�����␳�p�J�E���^ */
BOOL z80_irq_through;					/* IRQ�M���ʉ߃t���O */
BOOL z80_firq_through;					/* FIRQ�M���ʉ߃t���O */
BOOL z80_nmi_through;					/* NMI�M���ʉ߃t���O */
#endif

/*
 *	�v���g�^�C�v�錾
 */
void main_reset(void);
void main_line(void);
void main_exec(void);

#if XM7_VER == 1 && defined(Z80CARD)
/*
 *	���C��CPU(Z80)
 *	���������[�h�p���b�p�֐�
 */
static Uint32 mainmem_readb_z80(void *u, Uint32 a)
{
	UNUSED(u);

	return (Uint32)mainmem_readb((WORD)a);
}

/*
 *	���C��CPU(Z80)
 *	���������C�g�p���b�p�֐�
 */
static void mainmem_writeb_z80(void *u, Uint32 a, Uint32 d)
{
	UNUSED(u);

	mainmem_writeb((WORD)a, (BYTE)d);
}

/*
 *	���C��CPU(Z80)
 *	I/O���[�h�p���b�p�֐�
 */
static Uint32 mainio_readb_z80(void *u, Uint32 a)
{
	UNUSED(u);
	UNUSED(a);

	return 0xff;
}

/*
 *	���C��CPU(Z80)
 *	I/O���C�g�p���b�p�֐�
 */
static void mainio_writeb_z80(void *u, Uint32 a, Uint32 d)
{
	UNUSED(u);
	UNUSED(a);
	UNUSED(d);
}
#endif

/*
 *	���C��CPU
 *	������
 */
BOOL FASTCALL maincpu_init(void)
{
	maincpu.readmem = mainmem_readb;
	maincpu.writemem = mainmem_writeb;
#if XM7_VER == 1 && defined(Z80CARD)
	main_z80mode = FALSE;
	z80_irq_through = FALSE;
	z80_firq_through = FALSE;
	z80_nmi_through = FALSE;
	mainz80.memread = mainmem_readb_z80;
	mainz80.memwrite = mainmem_writeb_z80;
	mainz80.ioread = mainio_readb_z80;
	mainz80.iowrite = mainio_writeb_z80;
#endif

	return TRUE;
}

/*
 *	���C��CPU
 *	�N���[���A�b�v
 */
void FASTCALL maincpu_cleanup(void)
{
	return;
}

/*
 *	���C��CPU
 *	���Z�b�g
 */
void FASTCALL maincpu_reset(void)
{
	main_reset();
#if XM7_VER == 1 && defined(Z80CARD)
	kmz80_reset(&mainz80);
	main_z80mode = FALSE;
	z80_cycles = z80_cycles_sub = 0;
#endif
}

/*
 *	���C��CPU
 *	�P�s���s
 */
void FASTCALL maincpu_execline(void)
{
	DWORD cnt;

#if XM7_VER >= 2
	cnt = 0x100;
#else
	if (lowspeed_mode ||
		((fm_subtype == FMSUB_FM8) && tape_motor && motoron_lowspeed)) {
		cnt = 0x09e;
	}
	else {
		cnt = 0x100;
	}
#endif

	/* 1�s���s */
#if XM7_VER == 1 && defined(Z80CARD)
	if (main_z80mode) {
		z80_cycles = kmz80_exec(&mainz80, 1);
		z80_cycles_sub = (z80_cycles_sub + (z80_cycles % 2));
		maincpu.cycle = (WORD)((z80_cycles / 2) + (z80_cycles_sub / 2));
		maincpu.total += maincpu.cycle;
		z80_cycles_sub %= 2;
	}
	else {
		main_line();
	}
#else
	main_line();
#endif

	/* �e�[�v�J�E���^���� */
	if (tape_motor) {
		tape_subcnt += (DWORD)(maincpu.cycle << 4);
		if (tape_subcnt >= cnt) {
			tape_subcnt -= cnt;
			tape_count++;
			if (tape_count == 0) {
				tape_count = 0xffff;
			}
		}
	}
}

/*
 *	���C��CPU
 *	���s
 */
void FASTCALL maincpu_exec(void)
{
	DWORD cnt;

#if XM7_VER >= 2
	cnt = 0x100;
#else
	if (lowspeed_mode ||
		((fm_subtype == FMSUB_FM8) && tape_motor && motoron_lowspeed)) {
		cnt = 0x09e;
	}
	else {
		cnt = 0x100;
	}
#endif

	/* ���s */
#if XM7_VER == 1 && defined(Z80CARD)
	if (main_z80mode) {
		z80_cycles = kmz80_exec(&mainz80, 1);
		z80_cycles_sub = (z80_cycles_sub + (z80_cycles % 2));
		maincpu.cycle = (WORD)((z80_cycles / 2) + (z80_cycles_sub / 2));
		maincpu.total += maincpu.cycle;
		z80_cycles_sub %= 2;
	}
	else {
		main_exec();
	}
#else
	main_exec();
#endif

	/* �e�[�v�J�E���^���� */
	if (tape_motor) {
		tape_subcnt += (DWORD)(maincpu.cycle << 4);
		if (tape_subcnt >= cnt) {
			tape_subcnt -= cnt;
			tape_count++;
			if (tape_count == 0) {
				tape_count = 0xffff;
			}
		}
	}
}

/*
 *	���C��CPU
 *	NMI���荞�ݐݒ�
 */
void FASTCALL maincpu_nmi(void)
{
	/* FM-77��1MB FDD�Ή�(IRQ)�Ŏg���Ă��邪�A���Ή� */
	if (maincpu.intr & INTR_SLOAD){
		maincpu.intr |= INTR_NMI;
#if XM7_VER == 1 && defined(Z80CARD)
		if (z80_nmi_through) {
			mainz80.regs8[REGID_NMIREQ] |= 0x01;
		}
#endif
	}
}

/*
 *	���C��CPU
 *	FIRQ���荞�ݐݒ�
 */
void FASTCALL maincpu_firq(void)
{
	/* BREAK�L�[�y�сA�T�uCPU����̃A�e���V�������荞�� */
	if (break_flag || subattn_flag) {
		maincpu.intr |= INTR_FIRQ;
#if XM7_VER == 1 && defined(Z80CARD)
	if ((fm_subtype != FMSUB_FM8) && z80_firq_through) {
			mainz80.regs8[REGID_INTREQ] |= 0x01;
		}
#endif
	}
	else {
		maincpu.intr &= ~INTR_FIRQ;
#if XM7_VER == 1 && defined(Z80CARD)
		if ((fm_subtype != FMSUB_FM8) && z80_firq_through) {
			mainz80.regs8[REGID_INTREQ] &= (BYTE)~0x01;
		}
#endif
	}
}

/*
 *	���C��CPU
 *	IRQ���荞�ݐݒ�
 */
void FASTCALL maincpu_irq(void)
{
#if XM7_VER == 1
	/* FM-8���[�h����IRQ���荞�ݐݒ� */
	if (fm_subtype == FMSUB_FM8) {
		if (txrdy_irq_flag ||
			rxrdy_irq_flag ||
			syndet_irq_flag) {
			maincpu.intr |= INTR_IRQ;
#if defined(Z80CARD)
			if (z80_irq_through) {
				mainz80.regs8[REGID_INTREQ] |= 0x01;
			}
#endif
		}
		else {
			maincpu.intr &= ~INTR_IRQ;
#if defined(Z80CARD)
			if (z80_irq_through) {
				mainz80.regs8[REGID_INTREQ] &= (BYTE)~0x01;
			}
#endif
		}
		return;
	}
#endif

	/* IRQ���荞�ݐݒ� */
	if ((key_irq_flag && !(key_irq_mask)) ||
		timer_irq_flag ||
		lp_irq_flag ||
		mfd_irq_flag ||
		txrdy_irq_flag ||
		rxrdy_irq_flag ||
		syndet_irq_flag ||
#if XM7_VER >= 3
		dma_irq_flag ||
#endif
		ptm_irq_flag ||
		opn_irq_flag ||
		whg_irq_flag ||
		thg_irq_flag) {
		maincpu.intr |= INTR_IRQ;
#if XM7_VER == 1 && defined(Z80CARD)
		if (z80_irq_through) {
			mainz80.regs8[REGID_INTREQ] |= 0x01;
		}
#endif
	}
	else {
		maincpu.intr &= ~INTR_IRQ;
#if XM7_VER == 1 && defined(Z80CARD)
		if (z80_irq_through) {
			mainz80.regs8[REGID_INTREQ] &= (BYTE)~0x01;
		}
#endif
	}
}

/*
 *	���C��CPU
 *	�Z�[�u
 */
BOOL FASTCALL maincpu_save(int fileh)
{
	/* �v���b�g�t�H�[�����Ƃ̃p�b�L���O����������邽�߁A���� */
	if (!file_byte_write(fileh, maincpu.cc)) {
		return FALSE;
	}

	if (!file_byte_write(fileh, maincpu.dp)) {
		return FALSE;
	}

	if (!file_word_write(fileh, maincpu.acc.d)) {
		return FALSE;
	}

	if (!file_word_write(fileh, maincpu.x)) {
		return FALSE;
	}

	if (!file_word_write(fileh, maincpu.y)) {
		return FALSE;
	}

	if (!file_word_write(fileh, maincpu.u)) {
		return FALSE;
	}

	if (!file_word_write(fileh, maincpu.s)) {
		return FALSE;
	}

	if (!file_word_write(fileh, maincpu.pc)) {
		return FALSE;
	}

	if (!file_word_write(fileh, maincpu.intr)) {
		return FALSE;
	}

	if (!file_word_write(fileh, maincpu.cycle)) {
		return FALSE;
	}

	if (!file_word_write(fileh, maincpu.total)) {
		return FALSE;
	}

#if XM7_VER == 1 && defined(Z80CARD)
	if (!file_bool_write(fileh, main_z80mode)) {
		return FALSE;
	}
	if (!file_dword_write(fileh, z80_cycles)) {
		return FALSE;
	}
	if (!file_dword_write(fileh, z80_cycles_sub)) {
		return FALSE;
	}
	if (!file_write(fileh, mainz80.regs8, sizeof(mainz80.regs8))) {
		return FALSE;
	}
	if (!file_dword_write(fileh, mainz80.sp)) {
		return FALSE;
	}
	if (!file_dword_write(fileh, mainz80.pc)) {
		return FALSE;
	}
	if (!file_dword_write(fileh, mainz80.saf)) {
		return FALSE;
	}
	if (!file_dword_write(fileh, mainz80.sbc)) {
		return FALSE;
	}
	if (!file_dword_write(fileh, mainz80.sde)) {
		return FALSE;
	}
	if (!file_dword_write(fileh, mainz80.shl)) {
		return FALSE;
	}
	if (!file_dword_write(fileh, mainz80.t_fl)) {
		return FALSE;
	}
	if (!file_dword_write(fileh, mainz80.t_dx)) {
		return FALSE;
	}
#endif

	return TRUE;
}

/*
 *	���C��CPU
 *	���[�h
 */
BOOL FASTCALL maincpu_load(int fileh, int ver)
{
	/* �o�[�W�����`�F�b�N */
	if (ver < 200) {
		return FALSE;
	}

	/* �v���b�g�t�H�[�����Ƃ̃p�b�L���O����������邽�߁A���� */
	if (!file_byte_read(fileh, &maincpu.cc)) {
		return FALSE;
	}

	if (!file_byte_read(fileh, &maincpu.dp)) {
		return FALSE;
	}

	if (!file_word_read(fileh, &maincpu.acc.d)) {
		return FALSE;
	}

	if (!file_word_read(fileh, &maincpu.x)) {
		return FALSE;
	}

	if (!file_word_read(fileh, &maincpu.y)) {
		return FALSE;
	}

	if (!file_word_read(fileh, &maincpu.u)) {
		return FALSE;
	}

	if (!file_word_read(fileh, &maincpu.s)) {
		return FALSE;
	}

	if (!file_word_read(fileh, &maincpu.pc)) {
		return FALSE;
	}

	if (!file_word_read(fileh, &maincpu.intr)) {
		return FALSE;
	}

	if (!file_word_read(fileh, &maincpu.cycle)) {
		return FALSE;
	}

	if (!file_word_read(fileh, &maincpu.total)) {
		return FALSE;
	}

#if XM7_VER == 1 && defined(Z80CARD)
	if ((ver >= 310) && (ver <= 499)) {
		if (!file_bool_read(fileh, &main_z80mode)) {
			return FALSE;
		}
		if (!file_dword_read(fileh, &z80_cycles)) {
			return FALSE;
		}
		if (!file_dword_read(fileh, &z80_cycles_sub)) {
			return FALSE;
		}
		if (!file_read(fileh, mainz80.regs8, REGID_REGS8SIZE)) {
			return FALSE;
		}
		if (!file_dword_read(fileh, (DWORD *)&mainz80.sp)) {
			return FALSE;
		}
		if (!file_dword_read(fileh, (DWORD *)&mainz80.pc)) {
			return FALSE;
		}
		if (!file_dword_read(fileh, (DWORD *)&mainz80.saf)) {
			return FALSE;
		}
		if (!file_dword_read(fileh, (DWORD *)&mainz80.sbc)) {
			return FALSE;
		}
		if (!file_dword_read(fileh, (DWORD *)&mainz80.sde)) {
			return FALSE;
		}
		if (!file_dword_read(fileh, (DWORD *)&mainz80.shl)) {
			return FALSE;
		}
		if (!file_dword_read(fileh, (DWORD *)&mainz80.t_fl)) {
			return FALSE;
		}
		if (!file_dword_read(fileh, (DWORD *)&mainz80.t_dx)) {
			return FALSE;
		}
	}
	else {
		kmz80_reset(&mainz80);
		main_z80mode = FALSE;
		z80_cycles = z80_cycles_sub = 0;
	}
#endif

	return TRUE;
}
