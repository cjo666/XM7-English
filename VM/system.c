/*
 *	FM-7 EMULATOR "XM7"
 *
 *	Copyright (C) 1999-2017 �o�h�D(Twitter:@xm6_original)
 *	Copyright (C) 2001-2017 Ryu Takegami (Twitter:@RyuTakegami)
 *
 *	[ �V�X�e���Ǘ� ]
 */

#include <string.h>
#include <stdlib.h>
#include "xm7.h"
#include "display.h"
#include "ttlpalet.h"
#include "subctrl.h"
#include "keyboard.h"
#include "fdc.h"
#include "mainetc.h"
#include "multipag.h"
#include "kanji.h"
#include "tapelp.h"
#include "display.h"
#include "opn.h"
#include "mmr.h"
#include "aluline.h"
#include "apalet.h"
#include "rtc.h"
#include "mouse.h"
#include "rs232c.h"
#include "jsubsys.h"
#include "bubble.h"
#if defined(MIDI)
#include "midi.h"
#endif
#if XM7_VER >= 2
#include "jcard.h"
#endif
#if XM7_VER >= 3
#include "dmac.h"
#endif
#include "ptm.h"
#include "device.h"
#include "event.h"

/*
 *	�O���[�o�� ���[�N
 */
int fm7_ver;							/* �n�[�h�E�F�A�o�[�W���� */
int boot_mode;							/* �N�����[�h BASIC/DOS */
BYTE fm_subtype;						/* �n�[�h�E�F�A�T�u�o�[�W���� */
#if XM7_VER == 1
BOOL lowspeed_mode;						/* ����N���b�N���[�h */
BOOL available_fm8roms;					/* FM-8 ROM�g�p�\�t���O */
BOOL available_fm7roms;					/* FM-7 ROM�g�p�\�t���O */
#endif
#if (XM7_VER == 1) || (XM7_VER >= 3)
BOOL available_mmrboot;					/* �B���u�[�gROM�g�p�\�t���O */
#endif

BOOL hotreset_flag;						/* ���C���z�b�g���Z�b�g�t���O */
BOOL reset_flag;						/* �V�X�e�����Z�b�g�t���O */
BYTE fetch_op;							/* ���O�Ƀt�F�b�`�������� */

/*
 *	�X�e�[�g�t�@�C���w�b�_
 */
#if XM7_VER >= 3
const char *state_header = "XM7 VM STATE 921";
#elif XM7_VER >= 2
const char *state_header = "XM7 VM STATE 721";
#else
const char *state_header = "XM7 VM STATE 311";
#endif

/*
 *	�V�X�e��
 *	������
 */
BOOL FASTCALL system_init(void)
{
	/* ���[�h�ݒ� */
#if XM7_VER >= 3
	fm7_ver = 3;					/* FM77AV40EX�����ɐݒ� */
	available_mmrboot = TRUE;
	fm_subtype = FMSUB_DEFAULT;
#elif XM7_VER >= 2
	fm7_ver = 2;					/* FM77AV�����ɐݒ� */
	fm_subtype = FMSUB_DEFAULT;
#else
	fm7_ver = 1;					/* FM-77+192KB RAM�����ɐݒ� */
	fm_subtype = FMSUB_FM77;
	lowspeed_mode = FALSE;
	available_fm8roms = TRUE;
	available_fm7roms = TRUE;
	available_mmrboot = TRUE;
#endif
	boot_mode = BOOT_BASIC;			/* BASIC MODE */

	hotreset_flag = FALSE;
	reset_flag = FALSE;

	/* �X�P�W���[���A�������o�X */
	if (!schedule_init()) {
		return FALSE;
	}
	if (!mmr_init()) {
		return FALSE;
	}

	/* �������ACPU */
	if (!mainmem_init()) {
		return FALSE;
	}
	if (!submem_init()) {
		return FALSE;
	}
	if (!maincpu_init()) {
		return FALSE;
	}
	if (!subcpu_init()) {
		return FALSE;
	}
#if XM7_VER == 1
#if defined(JSUB)
	if (!jsubsys_init()) {
		return FALSE;
	}
#endif
#endif

	/* ���̑��f�o�C�X */
	if (!display_init()) {
		return FALSE;
	}
	if (!ttlpalet_init()) {
		return FALSE;
	}
	if (!subctrl_init()) {
		return FALSE;
	}
	if (!keyboard_init()) {
		return FALSE;
	}
	if (!fdc_init()) {
		return FALSE;
	}
	if (!mainetc_init()) {
		return FALSE;
	}
	if (!multipag_init()) {
		return FALSE;
	}
	if (!kanji_init()) {
		return FALSE;
	}
	if (!tapelp_init()) {
		return FALSE;
	}
	if (!opn_init()) {
		return FALSE;
	}
	if (!whg_init()) {
		return FALSE;
	}
	if (!thg_init()) {
		return FALSE;
	}
#if XM7_VER >= 2
	if (!aluline_init()) {
		return FALSE;
	}
	if (!apalet_init()) {
		return FALSE;
	}
	if (!rtc_init()) {
		return FALSE;
	}
#endif
#if defined(MIDI)
	if (!midi_init()) {
		return FALSE;
	}
#endif
#if XM7_VER >= 2
	if (!jcard_init()) {
		return FALSE;
	}
#endif
#if XM7_VER >= 3
	if (!dmac_init()) {
		return FALSE;
	}
#endif
#if defined(MOUSE)
	if (!mos_init()) {
		return FALSE;
	}
	if (!ptm_init()) {
		return FALSE;
	}
#endif
#if XM7_VER == 1
#if defined(BUBBLE)
	if (!bmc_init()) {
		return FALSE;
	}
#endif
#endif

	return TRUE;
}

/*
 *	�V�X�e��
 *	�N���[���A�b�v
 */
void FASTCALL system_cleanup(void)
{
	/* ���̑��f�o�C�X */
#if XM7_VER == 1
#if defined(BUBBLE)
	bmc_cleanup();
#endif
#endif
#if defined(MOUSE)
	ptm_cleanup();
	mos_cleanup();
#endif
#if XM7_VER >= 3
	dmac_cleanup();
#endif
#if XM7_VER >= 2
	jcard_cleanup();
#endif
#if defined(MIDI)
	midi_cleanup();
#endif
#if XM7_VER >= 2
	rtc_cleanup();
	apalet_cleanup();
	aluline_cleanup();
#endif
	thg_cleanup();
	whg_cleanup();
	opn_cleanup();
	tapelp_cleanup();
	kanji_cleanup();
	multipag_cleanup();
	mainetc_cleanup();
	fdc_cleanup();
	keyboard_cleanup();
	subctrl_cleanup();
	ttlpalet_cleanup();
	display_cleanup();

	/* �������ACPU */
#if XM7_VER == 1
#if defined(JSUB)
	jsubsys_cleanup();
#endif
#endif
	subcpu_cleanup();
	maincpu_cleanup();
	submem_cleanup();
	mainmem_cleanup();

	/* �X�P�W���[���A�������o�X */
	mmr_cleanup();
	schedule_cleanup();
}

/*
 *	�V�X�e��
 *	���Z�b�g
 */
void FASTCALL system_reset(void)
{
	/* �X�P�W���[���A�������o�X */
	schedule_reset();
	mmr_reset();

	/* ���̑��f�o�C�X */
	display_reset();
	ttlpalet_reset();
	subctrl_reset();
	keyboard_reset();
	fdc_reset();
	mainetc_reset();
	multipag_reset();
	kanji_reset();
	tapelp_reset();
	opn_reset();
	whg_reset();
	thg_reset();
#if XM7_VER >= 2
	aluline_reset();
	apalet_reset();
	rtc_reset();
#endif
#if defined(MIDI)
	midi_reset();
#endif
#if defined(RSC)
	rs232c_reset();
#endif
#if XM7_VER >= 2
	jcard_reset();
#endif
#if XM7_VER >= 3
	dmac_reset();
#endif
#if defined(MOUSE)
	mos_reset();
	ptm_reset();
#endif
#if XM7_VER == 1
#if defined(BUBBLE)
	bmc_reset();
#endif
#endif

	/* �������ACPU */
	mainmem_reset();
	submem_reset();
	maincpu_reset();
	subcpu_reset();
#if XM7_VER == 1
#if defined(JSUB)
	jsubsys_reset();
#endif
#endif

	/* ��ʍĕ`�� */
	display_setpointer(FALSE);
	display_notify();

	/* �V�X�e�����Z�b�g�t���O��ݒ� */
	reset_flag = TRUE;

	/* �t�F�b�`���ߏ����� */
	fetch_op = 0;
}

/*
 *	�V�X�e��
 *	�z�b�g���Z�b�g
 */
void FASTCALL system_hotreset(void)
{
#if XM7_VER == 1
	BOOL flag;
#endif

	/* TWR���L���Ȃ�N�����[�h��DOS�ɐ؂�ւ� */
#if XM7_VER >= 2
	if (twr_flag) {
		boot_mode = BOOT_DOS;
	}
#else
	flag = twr_flag;
#endif

	/* �V�X�e�����Z�b�g */
	system_reset();

	/* BREAK�L�[��������Ԃɂ��� */
	break_flag = TRUE;
	maincpu_firq();

	/* �z�b�g���Z�b�g�t���O�L�� */
	hotreset_flag = TRUE;

#if XM7_VER == 1
	/* ���Z�b�g���O��TWR���L���������痠RAM��I�� */
	if (flag) {
		basicrom_en = FALSE;
	}
#endif
}

/*
 *	�V�X�e��
 *	TAB+���Z�b�g
 */
#if XM7_VER >= 3
void FASTCALL system_tabreset(void)
{
	/* �V�X�e�����Z�b�g */
	system_reset();

	/* TAB�L�[�R�[�h���s */
	key_fm7 = 0x09;
}
#endif

/*
 *	�V�X�e��
 *	�t�@�C���Z�[�u
 */
BOOL FASTCALL system_save(char *filename)
{
	int fileh;
	BOOL flag;
	ASSERT(filename);

	/* �t�@�C���I�[�v�� */
	fileh = file_open(filename, OPEN_W);
	if (fileh == -1) {
		return FALSE;
	}

	/* �t���O������ */
	flag = TRUE;

	/* �w�b�_���Z�[�u */
	if (!file_write(fileh, (BYTE *)state_header, 16)) {
		flag = FALSE;
	}

	/* �V�X�e�����[�N */
	if (!file_word_write(fileh, (WORD)fm7_ver)) {
		return FALSE;
	}
	if (!file_word_write(fileh, (WORD)boot_mode)) {
		return FALSE;
	}
	if (!file_byte_write(fileh, fm_subtype)) {
		return FALSE;
	}
#if XM7_VER == 1
	if (!file_bool_write(fileh, lowspeed_mode)) {
		return FALSE;
	}
	if (!file_bool_write(fileh, available_fm8roms)) {
		return FALSE;
	}
	if (!file_bool_write(fileh, available_fm7roms)) {
		return FALSE;
	}
#endif

	/* ���ԂɌĂяo�� */
	if (!mainmem_save(fileh)) {
		flag = FALSE;
	}
	if (!submem_save(fileh)) {
		flag = FALSE;
	}
	if (!maincpu_save(fileh)) {
		flag = FALSE;
	}
	if (!subcpu_save(fileh)) {
		flag = FALSE;
	}
	if (!schedule_save(fileh)) {
		flag = FALSE;
	}
	if (!display_save(fileh)) {
		flag = FALSE;
	}
	if (!ttlpalet_save(fileh)) {
		flag = FALSE;
	}
	if (!subctrl_save(fileh)) {
		flag = FALSE;
	}
	if (!keyboard_save(fileh)) {
		flag = FALSE;
	}
	if (!fdc_save(fileh)) {
		flag = FALSE;
	}
	if (!mainetc_save(fileh)) {
		flag = FALSE;
	}
	if (!multipag_save(fileh)) {
		flag = FALSE;
	}
	if (!kanji_save(fileh)) {
		flag = FALSE;
	}
	if (!tapelp_save(fileh)) {
		flag = FALSE;
	}
	if (!opn_save(fileh)) {
		flag = FALSE;
	}
	if (!mmr_save(fileh)) {
		flag = FALSE;
	}
#if XM7_VER >= 2
	if (!aluline_save(fileh)) {
		flag = FALSE;
	}
	if (!rtc_save(fileh)) {
		flag = FALSE;
	}
	if (!apalet_save(fileh)) {
		flag = FALSE;
	}
#endif
	if (!whg_save(fileh)) {
		flag = FALSE;
	}
	if (!thg_save(fileh)) {
		flag = FALSE;
	}
#if XM7_VER >= 2
	if (!jcard_save(fileh)) {
		flag = FALSE;
	}
#endif
#if XM7_VER >= 3
	if (!dmac_save(fileh)) {
		flag = FALSE;
	}
#endif
#if defined(MOUSE)
	if (!mos_save(fileh)) {
		flag = FALSE;
	}
#endif
#if defined(RSC)
	if (!rs232c_save(fileh)) {
		flag = FALSE;
	}
#endif
#if defined(MIDI)
	if (!midi_save(fileh)) {
		flag = FALSE;
	}
#endif
#if XM7_VER == 1
#if defined(JSUB)
	if (!jsubsys_save(fileh)) {
		flag = FALSE;
	}
#endif
#if defined(BUBBLE) || (!defined(BUBBLE) && defined(XM7PURE))
	if (!bmc_save(fileh)) {
		flag = FALSE;
	}
#endif
#endif
#if defined(MOUSE)
	if (!ptm_save(fileh)) {
		flag = FALSE;
	}
#endif

	file_close(fileh);
	return flag;
}

/*
 *	�V�X�e��
 *	�t�@�C�����[�h
 */
int FASTCALL system_load(char *filename)
{
	int fileh;
	int ver;
	char header[16];
	BOOL flag;
#if XM7_VER >= 2
	BOOL old_scheduler;
	int filesize;
#endif
	WORD tmp;
#if XM7_VER == 1
	BOOL temp;
#endif

	ASSERT(filename);

	/* �t�@�C���I�[�v�� */
	fileh = file_open(filename, OPEN_R);
	if (fileh == -1) {
		return STATELOAD_OPENERR;
	}

	/* �t���O������ */
	flag = TRUE;
#if XM7_VER >= 2
	old_scheduler = FALSE;
#endif

	/* �w�b�_�����[�h */
	if (!file_read(fileh, (BYTE *)header, 16)) {
		flag = FALSE;
	}
	else {
		if (memcmp(header, "XM7 VM STATE ", 13) != 0) {
			flag = FALSE;
		}
	}

	/* �w�b�_�`�F�b�N */
	if (!flag) {
		file_close(fileh);
		return STATELOAD_HEADERR;
	}

	/* �t�@�C���o�[�W�����擾�A�o�[�W����2�ȏオ�Ώ� */
	if (header[13] != 0x20) {
		ver = (int)	((BYTE)(header[13] - 0x30) * 100) +
					((BYTE)(header[14] - 0x30) * 10) +
					((BYTE)(header[15] - 0x30));
	}
	else {
		ver = (int)(BYTE)(header[15]);
		ver -= 0x30;
		ver *= 100;
	}
#if XM7_VER >= 3
	/* V3 : Ver5�����̓��[�h�ł��Ȃ� */
	if ((ver < 200) || ((ver >= 300) && (ver <= 499))) {
		file_close(fileh);
		return STATELOAD_VERERR;
	}
#elif XM7_VER >= 2
	/* V2 : Ver8�ȏ�EVer5�����̓��[�h�ł��Ȃ� */
	if ((ver >= 800) || (ver < 200) || ((ver >= 300) && (ver <= 499))) {
		file_close(fileh);
		return STATELOAD_VERERR;
	}
#else
	/* V1 : Ver5�ȏ�EVer2�����̓��[�h�ł��Ȃ� */
	if ((ver >= 500) || (ver < 200)) {
		file_close(fileh);
		return STATELOAD_VERERR;
	}
#endif
	if (ver >	(int)((BYTE)(state_header[13] - 0x30) * 100) +
					 ((BYTE)(state_header[14] - 0x30) * 10) +
					 ((BYTE)(state_header[15] - 0x30))) {
		file_close(fileh);
		return STATELOAD_VERERR;
	}

#if XM7_VER >= 2
	/* V3.0L30/V2.5L20�ȑO�̃X�e�[�g�t�@�C���ɑΏ� */
	filesize = file_getsize(fileh);
	if (ver <= 500) {
		old_scheduler = TRUE;
	}
	else {
#if XM7_VER >= 3
		if ((filesize == 454758) ||		/* XM7 V3.0 (����RAM����) */
			(filesize == 1241190) ||	/* XM7 V3.0 (����RAM�L��) */
			(filesize == 258030)) {		/* XM7 V2 */
			old_scheduler = TRUE;
		}
#else
		if (filesize == 258030) {		/* XM7 V2 */
			old_scheduler = TRUE;
		}
#endif
	}
#endif

	/* �V�X�e�����[�N */
	if (!file_word_read(fileh, &tmp)) {
		return FALSE;
	}
	fm7_ver = (int)tmp;
	if (!file_word_read(fileh, &tmp)) {
		return FALSE;
	}
	boot_mode = (int)tmp;
#if XM7_VER >= 3
	if (((ver >= 719) && (ver <= 799)) || (ver >= 919)) {
#elif XM7_VER >= 2
	if (ver >= 719) {
#else
	{
#endif
		if (!file_byte_read(fileh, &fm_subtype)) {
			return FALSE;
		}
	}
#if XM7_VER == 1
	if (!file_bool_read(fileh, &lowspeed_mode)) {
		return FALSE;
	}

	/* �Z�[�u���ɗL��������ROM�f�[�^�����[�h���ɗL���łȂ��ꍇ�G���[ */
	if (!file_bool_read(fileh, &temp)) {
		return FALSE;
	}
	if (!available_fm8roms && temp) {
		return FALSE;
	}
	if (!file_bool_read(fileh, &temp)) {
		return FALSE;
	}
	if (!available_fm7roms && temp) {
		return FALSE;
	}
#endif

	/* ���ԂɌĂяo�� */
	if (!mainmem_load(fileh, ver)) {
		flag = FALSE;
	}
	if (!submem_load(fileh, ver)) {
		flag = FALSE;
	}
	if (!maincpu_load(fileh, ver)) {
		flag = FALSE;
	}
	if (!subcpu_load(fileh, ver)) {
		flag = FALSE;
	}
#if XM7_VER == 1
	if (!schedule_load(fileh, ver)) {
		flag = FALSE;
	}
#else
	if (!schedule_load(fileh, ver, old_scheduler)) {
		flag = FALSE;
	}
#endif
	if (!display_load(fileh, ver)) {
		flag = FALSE;
	}
	if (!ttlpalet_load(fileh, ver)) {
		flag = FALSE;
	}
	if (!subctrl_load(fileh, ver)) {
		flag = FALSE;
	}
	if (!keyboard_load(fileh, ver)) {
		flag = FALSE;
	}
	if (!fdc_load(fileh, ver)) {
		flag = FALSE;
	}
	if (!mainetc_load(fileh, ver)) {
		flag = FALSE;
	}
	if (!multipag_load(fileh, ver)) {
		flag = FALSE;
	}
	if (!kanji_load(fileh, ver)) {
		flag = FALSE;
	}
	if (!tapelp_load(fileh, ver)) {
		flag = FALSE;
	}
	if (!opn_load(fileh, ver)) {
		flag = FALSE;
	}
	if (!mmr_load(fileh, ver)) {
		flag = FALSE;
	}
#if XM7_VER >= 2
	if (!aluline_load(fileh, ver)) {
		flag = FALSE;
	}
	if (!rtc_load(fileh, ver)) {
		flag = FALSE;
	}
	if (!apalet_load(fileh, ver)) {
		flag = FALSE;
	}
#endif
	if (!whg_load(fileh, ver)) {
		flag = FALSE;
	}
	if (!thg_load(fileh, ver)) {
		flag = FALSE;
	}
#if XM7_VER >= 2
	if (!jcard_load(fileh, ver)) {
		flag = FALSE;
	}
#endif
#if XM7_VER >= 3
	if (!dmac_load(fileh, ver)) {
		flag = FALSE;
	}
#endif
#if defined(MOUSE)
	if (!mos_load(fileh, ver)) {
		flag = FALSE;
	}
#endif
#if defined(RSC)
	if (!rs232c_load(fileh, ver)) {
		flag = FALSE;
	}
#endif
#if defined(MIDI)
	if (!midi_load(fileh, ver)) {
		flag = FALSE;
	}
#endif
#if XM7_VER == 1
#if defined(JSUB)
	if (!jsubsys_load(fileh, ver)) {
		flag = FALSE;
	}
#endif
#if defined(BUBBLE) || (!defined(BUBBLE) && defined(XM7PURE))
	if (!bmc_load(fileh, ver)) {
		flag = FALSE;
	}
#endif
#endif
#if defined(MOUSE)
	if (!ptm_load(fileh, ver)) {
		flag = FALSE;
	}
#endif
	file_close(fileh);

#if XM7_VER >= 3
	/* 400���C�����[�h�̂�VRAM�z�u�␳���K�v */
	if (mode400l) {
		if (!fix_vram_address()) {
			flag = FALSE;
		}
	}
#endif

	/* ��ʍĕ`�� */
	display_notify();

	/* CPU���x�䗦�ݒ� */
	speed_ratio = 10000;

	if (!flag) {
		return STATELOAD_ERROR;
	}

	return STATELOAD_SUCCESS;
}

/*
 *	�t�@�C���ǂݍ���(BYTE)
 */
BOOL FASTCALL file_byte_read(int fileh, BYTE *dat)
{
	return file_read(fileh, dat, 1);
}

/*
 *	�t�@�C���ǂݍ���(WORD)
 */
BOOL FASTCALL file_word_read(int fileh, WORD *dat)
{
	BYTE tmp;

	if (!file_read(fileh, &tmp, 1)) {
		return FALSE;
	}
	*dat = tmp;

	if (!file_read(fileh, &tmp, 1)) {
		return FALSE;
	}
	*dat <<= 8;
	*dat |= tmp;

	return TRUE;
}

/*
 *	�t�@�C���ǂݍ���(DWORD)
 */
BOOL FASTCALL file_dword_read(int fileh, DWORD *dat)
{
	BYTE tmp;

	if (!file_read(fileh, &tmp, 1)) {
		return FALSE;
	}
	*dat = tmp;

	if (!file_read(fileh, &tmp, 1)) {
		return FALSE;
	}
	*dat *= 256;
	*dat |= tmp;

	if (!file_read(fileh, &tmp, 1)) {
		return FALSE;
	}
	*dat *= 256;
	*dat |= tmp;

	if (!file_read(fileh, &tmp, 1)) {
		return FALSE;
	}
	*dat *= 256;
	*dat |= tmp;

	return TRUE;
}

/*
 *	�t�@�C���ǂݍ���(BOOL)
 */
BOOL FASTCALL file_bool_read(int fileh, BOOL *dat)
{
	BYTE tmp;

	if (!file_read(fileh, &tmp, 1)) {
		return FALSE;
	}

	switch (tmp) {
		case 0:
			*dat = FALSE;
			return TRUE;
		case 0xff:
			*dat = TRUE;
			return TRUE;
	}

	return FALSE;
}

/*
 *	�t�@�C����������(BYTE)
 */
BOOL FASTCALL file_byte_write(int fileh, BYTE dat)
{
	return file_write(fileh, &dat, 1);
}

/*
 *	�t�@�C����������(WORD)
 */
BOOL FASTCALL file_word_write(int fileh, WORD dat)
{
	BYTE tmp;

	tmp = (BYTE)(dat >> 8);
	if (!file_write(fileh, &tmp, 1)) {
		return FALSE;
	}

	tmp = (BYTE)(dat & 0xff);
	if (!file_write(fileh, &tmp, 1)) {
		return FALSE;
	}

	return TRUE;
}

/*
 *	�t�@�C����������(DWORD)
 */
BOOL FASTCALL file_dword_write(int fileh, DWORD dat)
{
	BYTE tmp;

	tmp = (BYTE)(dat >> 24);
	if (!file_write(fileh, &tmp, 1)) {
		return FALSE;
	}

	tmp = (BYTE)(dat >> 16);
	if (!file_write(fileh, &tmp, 1)) {
		return FALSE;
	}

	tmp = (BYTE)(dat >> 8);
	if (!file_write(fileh, &tmp, 1)) {
		return FALSE;
	}

	tmp = (BYTE)(dat & 0xff);
	if (!file_write(fileh, &tmp, 1)) {
		return FALSE;
	}

	return TRUE;
}

/*
 *	�t�@�C����������(BOOL)
 */
BOOL FASTCALL file_bool_write(int fileh, BOOL dat)
{
	BYTE tmp;

	if (dat) {
		tmp = 0xff;
	}
	else {
		tmp = 0;
	}

	if (!file_write(fileh, &tmp, 1)) {
		return FALSE;
	}

	return TRUE;
}
