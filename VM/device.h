/*
 *	FM-7 EMULATOR "XM7"
 *
 *	Copyright (C) 1999-2017 �o�h�D(Twitter:@xm6_original)
 *	Copyright (C) 2001-2017 Ryu Takegami (Twitter:@RyuTakegami)
 *	line printer support 2010-2013 by Ben.JP
 *
 *	[ �f�o�C�X�ˑ��� ]
 */

#ifndef _device_h_
#define _device_h_

/*
 *	�萔��`
 */
#define OPEN_R		1					/* �ǂݍ��݃��[�h */
#define OPEN_W		2					/* �������݃��[�h */
#define OPEN_RW		3					/* �ǂݏ������[�h */

#define SOUND_STOP			255			/* �T�E���h��~ */
#define SOUND_CMTMOTORON	0			/* CMT���[�^���䃊���[ON */
#define SOUND_CMTMOTOROFF	1			/* CMT���[�^���䃊���[OFF */
#define SOUND_FDDSEEK		2			/* FDD�w�b�h�V�[�N */
#define SOUND_FDDHEADUP		3			/* FDD�w�b�h�A�b�v(���g�p) */
#define SOUND_FDDHEADDOWN	4			/* FDD�w�b�h�_�E��(���g�p) */

#ifdef __cplusplus
extern "C" {
#endif
/*
 *	��v�G���g��
 */

/* �`�� */
void FASTCALL vram_notify(WORD addr, BYTE dat);
										/* VRAM�������ݒʒm */
void FASTCALL ttlpalet_notify(void);
										/* �f�W�^���p���b�g�ύX�ʒm */
void FASTCALL display_notify(void);
										/* ��ʖ����ʒm(�O���t�B�b�N���) */
void FASTCALL vsync_notify(void);
										/* VSYNC�ʒm */
void FASTCALL vblankperiod_notify(void);
										/* VBLANK�I���ʒm */
void FASTCALL hblank_notify(void);
										/* HBLANK�ʒm */
void FASTCALL refpalet_notify(void);
										/* �p���b�g�X�V�ʒm */
#if XM7_VER == 1
void FASTCALL tvram_notify(WORD addr, BYTE dat);
										/* �e�L�X�gVRAM�������ݒʒm */
void FASTCALL tvram_redraw_notify(void);
										/* ��ʖ����ʒm(�e�L�X�g���) */
#endif
#if XM7_VER >= 2
void FASTCALL apalet_notify(void);
										/* �A�i���O�p���b�g�ύX�ʒm */
void FASTCALL digitize_notify(void);
										/* �f�B�W�^�C�Y�ʒm */
#if XM7_VER >= 3
void FASTCALL window_notify(void);
										/* �n�[�h�E�F�A�E�B���h�E�ύX�ʒm */
#endif
#endif

/* �T�E���h */
void FASTCALL opn_notify(BYTE reg, BYTE dat);
										/* OPN�o�͒ʒm */
void FASTCALL whg_notify(BYTE reg, BYTE dat);
										/* WHG�o�͒ʒm */
void FASTCALL thg_notify(BYTE reg, BYTE dat);
										/* THG�o�͒ʒm */
void FASTCALL beep_notify(void);
										/* BEEP�o�͒ʒm */
void FASTCALL tape_notify(BOOL flag);
										/* �e�[�v�o�͒ʒm */
void FASTCALL midi_notify(BYTE mes);
										/* MIDI�o�͒ʒm */
void FASTCALL midi_reset_notify(void);
										/* MIDI���Z�b�g�ʒm */
void FASTCALL wav_notify(BYTE no);
										/* �e����ʉ��o�͒ʒm */
void FASTCALL dac_notify(BYTE dat);
										/* �W���X�g�T�E���h�o�͒ʒm */
#if XM7_VER >= 2
void FASTCALL keyencbeep_notify(void);
										/* �L�[�G���R�[�_BEEP�o�͒ʒm */
#endif

/* �W���C�X�e�B�b�N�E�}�E�X */
BYTE FASTCALL joy_request(BYTE port);
										/* �W���C�X�e�B�b�N�v�� */
#if defined(MOUSE)
void FASTCALL mospos_request(BYTE *move_x, BYTE *move_y);
										/* �}�E�X�ړ������v�� */
BYTE FASTCALL mosbtn_request(void);
										/* �}�E�X�{�^����ԗv�� */
#endif

#if defined(RSC)
/* �V���A�� */
void FASTCALL rs232c_reset_notify(void);
										/* RS-232C���Z�b�g�ʒm */
void FASTCALL rs232c_senddata(BYTE dat);
										/* RS-232C�f�[�^���M */
BYTE FASTCALL rs232c_receivedata(void);
										/* RS-232C�f�[�^��M */
BYTE FASTCALL rs232c_readstatus(void);
										/* �X�e�[�^�X���W�X�^���[�h */
void FASTCALL rs232c_writemodecmd(BYTE dat);
										/* ���[�h�R�}���h���W�X�^���C�g */
void FASTCALL rs232c_writecommand(BYTE dat);
										/* �R�}���h���W�X�^���C�g */
void FASTCALL rs232c_setbaudrate(BYTE dat);
										/* �{�[���[�g���W�X�^���C�g */
#endif

#if defined(LPRINT)
/* �v�����^ */
BOOL FASTCALL lp_printfile(void);
										/* ��� */
int FASTCALL lp_openfile(char *fname);
										/* ����p�ꎞ�t�@�C���I�[�v�� */
void FASTCALL lp_closefile(void);
										/* ����p�ꎞ�t�@�C���N���[�Y */
BOOL FASTCALL lp_writefile(BYTE *ptr, DWORD size);
										/* ����p�ꎞ�t�@�C���������� */
void FASTCALL lp_removefile(void);
										/* ����p�ꎞ�t�@�C���폜 */
#endif

/* �t�@�C�� */
BOOL FASTCALL file_load(char *fname, BYTE *buf, int size);
										/* �t�@�C�����[�h(ROM��p) */
BOOL FASTCALL file_load2(char *fname, BYTE *buf, int ptr, int size);
										/* �C�ӈʒu����̃t�@�C�����[�h */
BOOL FASTCALL file_save(char *fname, BYTE *buf, int size);
										/* �t�@�C���Z�[�u(�w�KRAM��p) */
int FASTCALL file_open(char *fname, int mode);
										/* �t�@�C���I�[�v�� */
void FASTCALL file_close(int handle);
										/* �t�@�C���N���[�Y */
DWORD FASTCALL file_getsize(int handle);
										/* �t�@�C�������O�X�擾 */
BOOL FASTCALL file_seek(int handle, DWORD offset);
										/* �t�@�C���V�[�N */
BOOL FASTCALL file_read(int handle, BYTE *ptr, DWORD size);
										/* �t�@�C���ǂݍ��� */
BOOL FASTCALL file_write(int handle, BYTE *ptr, DWORD size);
										/* �t�@�C���������� */

/* �t�@�C���T�u(�v���b�g�t�H�[����ˑ��B���̂�system.c�ɂ���) */
BOOL FASTCALL file_byte_read(int handle, BYTE *dat);
										/* �o�C�g�ǂݍ��� */
BOOL FASTCALL file_word_read(int handle, WORD *dat);
										/* ���[�h�ǂݍ��� */
BOOL FASTCALL file_dword_read(int handle, DWORD *dat);
										/* �_�u�����[�h�ǂݍ��� */
BOOL FASTCALL file_bool_read(int handle, BOOL *dat);
										/* �u�[���ǂݍ��� */
BOOL FASTCALL file_byte_write(int handle, BYTE dat);
										/* �o�C�g�������� */
BOOL FASTCALL file_word_write(int handle, WORD dat);
										/* ���[�h�������� */
BOOL FASTCALL file_dword_write(int handle, DWORD dat);
										/* �_�u�����[�h�������� */
BOOL FASTCALL file_bool_write(int handle, BOOL dat);
										/* �u�[���������� */
#ifdef __cplusplus
}
#endif

#endif	/* _device_h_ */
