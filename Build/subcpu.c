/*
 *	FM-7 EMULATOR "XM7"
 *
 *	Copyright (C) 1999-2017 �o�h�D(Twitter:@xm6_original)
 *	Copyright (C) 2001-2017 Ryu Takegami (Twitter:@RyuTakegami)
 *
 *	[ �T�uCPU ]
 *
 *	RHG����
 *	  2002.05.07		C�����CPU�R�A�ɍđΉ�
 */

#include "xm7.h"
#include "subctrl.h"
#include "mainetc.h"
#include "device.h"
#include "event.h"

/*
 *	�O���[�o�� ���[�N
 */
cpu6809_t subcpu;						/* �T�uCPU���W�X�^��� */

/*
 *	�v���g�^�C�v�錾
 */
void sub_reset(void);
void sub_line(void);
void sub_exec(void);
BOOL subcpu_event(void);

/*
 *	�T�uCPU
 *	������
 */
BOOL FASTCALL subcpu_init(void)
{
	subcpu.readmem = submem_readb;
	subcpu.writemem = submem_writeb;

	return TRUE;
}

/*
 *	�T�uCPU
 *	�N���[���A�b�v
 */
void FASTCALL subcpu_cleanup(void)
{
	return;
}

/*
 *	�T�uCPU
 *	���Z�b�g
 */
void FASTCALL subcpu_reset(void)
{
	sub_reset();
}

/*
 *	�T�uCPU
 *	�P�s���s
 */
void FASTCALL subcpu_execline(void)
{
	sub_line();
}

/*
 *	�T�uCPU
 *	���s
 */
void FASTCALL subcpu_exec(void)
{
	sub_exec();
}

/*
 *	�T�uCPU
 *	NMI���荞�ݐݒ�
 */
void FASTCALL subcpu_nmi(void)
{
	if (subcpu.intr & INTR_SLOAD){
		subcpu.intr |= INTR_NMI;
	}
}

/*
 *	�T�uCPU
 *	FIRQ���荞�ݐݒ�
 */
void FASTCALL subcpu_firq(void)
{
	if (!key_irq_mask) {
		/* ���C��CPU�ɂȂ����Ă���ꍇ�́A���荞�݂Ȃ� */
		subcpu.intr &= ~INTR_FIRQ;
	}
	else {
		/* �L�[���荞�݂̗L���ŕ������ */
		if (key_irq_flag) {
			subcpu.intr |= INTR_FIRQ;
		}
		else {
			subcpu.intr &= ~INTR_FIRQ;
		}
	}
}

/*
 *	�T�uCPU
 *	IRQ���荞�ݐݒ�
 */
void FASTCALL subcpu_irq(void)
{
	/* �L�����Z��IRQ */
	if (subcancel_flag) {
		subcpu.intr |= INTR_IRQ;
	}
	else {
		subcpu.intr &= ~INTR_IRQ;
	}
}

/*
 *	�T�uCPU
 *	�Z�[�u
 */
BOOL FASTCALL subcpu_save(int fileh)
{
	/* �v���b�g�t�H�[�����Ƃ̃p�b�L���O����������邽�߁A���� */
	if (!file_byte_write(fileh, subcpu.cc)) {
		return FALSE;
	}

	if (!file_byte_write(fileh, subcpu.dp)) {
		return FALSE;
	}

	if (!file_word_write(fileh, subcpu.acc.d)) {
		return FALSE;
	}

	if (!file_word_write(fileh, subcpu.x)) {
		return FALSE;
	}

	if (!file_word_write(fileh, subcpu.y)) {
		return FALSE;
	}

	if (!file_word_write(fileh, subcpu.u)) {
		return FALSE;
	}

	if (!file_word_write(fileh, subcpu.s)) {
		return FALSE;
	}

	if (!file_word_write(fileh, subcpu.pc)) {
		return FALSE;
	}

	if (!file_word_write(fileh, subcpu.intr)) {
		return FALSE;
	}

	if (!file_word_write(fileh, subcpu.cycle)) {
		return FALSE;
	}

	if (!file_word_write(fileh, subcpu.total)) {
		return FALSE;
	}

	return TRUE;
}

/*
 *	�T�uCPU
 *	���[�h
 */
BOOL FASTCALL subcpu_load(int fileh, int ver)
{
	/* �o�[�W�����`�F�b�N */
	if (ver < 200) {
		return FALSE;
	}

	/* �v���b�g�t�H�[�����Ƃ̃p�b�L���O����������邽�߁A���� */
	if (!file_byte_read(fileh, &subcpu.cc)) {
		return FALSE;
	}

	if (!file_byte_read(fileh, &subcpu.dp)) {
		return FALSE;
	}

	if (!file_word_read(fileh, &subcpu.acc.d)) {
		return FALSE;
	}

	if (!file_word_read(fileh, &subcpu.x)) {
		return FALSE;
	}

	if (!file_word_read(fileh, &subcpu.y)) {
		return FALSE;
	}

	if (!file_word_read(fileh, &subcpu.u)) {
		return FALSE;
	}

	if (!file_word_read(fileh, &subcpu.s)) {
		return FALSE;
	}

	if (!file_word_read(fileh, &subcpu.pc)) {
		return FALSE;
	}

	if (!file_word_read(fileh, &subcpu.intr)) {
		return FALSE;
	}

	if (!file_word_read(fileh, &subcpu.cycle)) {
		return FALSE;
	}

	if (!file_word_read(fileh, &subcpu.total)) {
		return FALSE;
	}

	return TRUE;
}
