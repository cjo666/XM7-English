/*
 *	FM-7 EMULATOR "XM7"
 *
 *	Copyright (C) 1999-2017 �o�h�D(Twitter:@xm6_original)
 *	Copyright (C) 2001-2017 Ryu Takegami (Twitter:@RyuTakegami)
 *
 *	[ ���ʒ�` ]
 */

#ifndef _xm7_h_
#define _xm7_h_

#include <stdio.h>
#if XM7_VER == 1 && defined(Z80CARD)
#include "kmz80.h"
#endif

/*
 *	�o�[�W����
 */
#if XM7_VER >= 3
#define	VERSION		"V3.4"
#elif XM7_VER >= 2
#define	VERSION		"V2.9"
#else
#define	VERSION		"V1.1"
#endif
#define	LEVEL		"L77SX"
//#define	BETAVER
//#define BETANO		"4"
#define	DATE		"2017/12/23"

/*
 *	�萔�A�^��`
 */

/* �ėp�萔 */
#ifndef FALSE
#define FALSE			0
#define TRUE			(!FALSE)
#endif
#ifndef NULL
#define NULL			((void)0)
#endif

/* �f�f */
#ifndef ASSERT
#ifdef _DEBUG
#include <assert.h>
#define ASSERT(exp)		assert(exp)
#else
#define ASSERT(exp)		((void)0)
#endif
#endif

/* ���g�p�ϐ��̏��� */
#ifndef UNUSED
#define UNUSED(x)		((void)(x))
#endif

/* �œK�� */
#if defined(_WIN32) && (defined(__BORLANDC__) || (defined(_MSC_VER) && (defined(_M_IX86))))
#define FASTCALL		__fastcall
#else
#define FASTCALL
#endif

/* ��{�^��` */
typedef unsigned char BYTE;
typedef unsigned short WORD;
typedef unsigned long DWORD;
typedef int BOOL;

/* CPU���W�X�^��` */
#ifdef _WIN32
#pragma pack(push, 1)
#endif
typedef struct {
	BYTE cc;
	BYTE dp;
	union {
		struct {
#if defined(ANDROID)
			BYTE b;
			BYTE a;
#endif
#if defined(_WIN32)
			BYTE b;
			BYTE a;
#endif
#if defined(__MSDOS__)
			BYTE b;
			BYTE a;
#endif
#if defined(FMT)
			BYTE b;
			BYTE a;
#endif
#if defined(HUMAN68K)
			BYTE a;
			BYTE b;
#endif
#if defined(_XWIN)
			BYTE b __attribute__((aligned(1)));
			BYTE a __attribute__((aligned(1)));
#endif
		} h;
		WORD d;
	} acc;
	WORD x;
	WORD y;
	WORD u;
	WORD s;
	WORD pc;
	WORD intr;
	WORD cycle;
	WORD total;
	BYTE (FASTCALL *readmem)(WORD);
	void (FASTCALL *writemem)(WORD, BYTE);
} cpu6809_t;
#ifdef _WIN32
#pragma pack(pop)
#endif

/* ���荞�ݒ�` */
#define INTR_NMI		0x0001			/* NMI���荞�� */
#define INTR_FIRQ		0x0002			/* FIRQ���荞�� */
#define INTR_IRQ		0x0004			/* IRQ���荞�� */

#define INTR_SLOAD		0x0010			/* ���Z�b�g��S��ݒ� */
#define INTR_SYNC_IN	0x0020			/* SYNC���s�� */
#define INTR_SYNC_OUT	0x0040			/* SYNC�I���\ */
#define INTR_CWAI_IN	0x0080			/* CWAI���s�� */
#define INTR_CWAI_OUT	0x0100			/* CWAI�I���\ */
#define INTR_NMI_PLS	0x1000			/* NMI�p���X�M�� */
#define INTR_FIRQ_PLS	0x2000			/* FIRQ�p���X�M�� */
#define INTR_IRQ_PLS	0x4000			/* IRQ�p���X�M�� */
#define INTR_HALT		0x8000			/* HALT���ߎ��s�� */

/* �u���[�N�|�C���g��` */
#define BREAKP_NOTUSE	0				/* ���g�p */
#define BREAKP_ENABLED	1				/* �g�p�� */
#define BREAKP_DISABLED	2				/* �֎~�� */
#define BREAKP_STOPPED	3				/* ��~�� */
#define BREAKP_MAXNUM	16				/* �u���[�N�|�C���g�̌� */
#define BREAKP_MAXNUM_OLD	8			/* �u���[�N�|�C���g�̌� (�`V3.1L10) */
typedef struct {
	int flag;							/* ��̃t���O */
	int cpu;							/* CPU��� */
#if defined(MMRTEST)
	DWORD addr;							/* �u���[�N�|�C���g�A�h���X */
	BYTE bank;							/* �u���[�N�|�C���g�o���N */
#else
	WORD addr;							/* �u���[�N�|�C���g�A�h���X */
#endif
} breakp_t;

/* �C�x���g��` */
#define EVENT_NOTUSE	0				/* ���g�p */
#define EVENT_ENABLED	1				/* �g�p�� */
#define EVENT_DISABLED	2				/* �֎~�� */
#define	EVENT_MAXNUM	32				/* �C�x���g�̌� */
#define EVENT_MAXNUM_L31	18			/* V3.0L31�ł̃C�x���g�̌� */
#define	EVENT_MAXNUM_L30	16			/* V3.0L30�ł̃C�x���g�̌� */
typedef struct {
	int flag;							/* ��̃t���O */
	DWORD current;						/* �J�����g���ԃJ�E���^ */
	DWORD reload;						/* �����[�h���ԃJ�E���^ */
	BOOL (FASTCALL *callback)(void);	/* �R�[���o�b�N�֐� */
} event_t;

/* CPU���s�T�C�N���� */
#define MAINCYCLES		1794			/* ���C��CPU MMR OFF */
#define MAINCYCLES_MMR	1565			/* ���C��CPU MMR ON */
#define MAINCYCLES_FMMR	2016			/* ���C��CPU ����MMR */
#define SUBCYCLES		2000			/* �T�uCPU */
#define MAINCYCLES_LOW	1095			/* ���C��CPU MMR OFF(�ᑬ) */
#define SUBCYCLES_LOW	999				/* �T�uCPU(�ᑬ) */
#define JSUBCYCLES		1228			/* ���{��T�uCPU */

/* �n�[�h�E�F�A�T�u�o�[�W���� */
#define FMSUB_FM7		0x00			/* FM-7 */
#define FMSUB_FM77		0x01			/* FM-77 */
#define FMSUB_FM8		0x02			/* FUJITSU MICRO 8 */
#define	FMSUB_DEFAULT	0x00			/* �f�t�H���g�ݒ�(for V2/V3) */

/* ���̑��萔 */
#define MAINCPU			0				/* ���C��CPU(6809) */
#define SUBCPU			1				/* �T�uCPU */
#define JSUBCPU			2				/* ���{��T�uCPU */
#define MAINZ80			3				/* ���C��CPU(Z80) */
#define CPU_VOID		255				/* ����(�ǂ�CPU�ł��Ȃ�) */
#if XM7_VER >= 2
#define MAXCPU			2				/* �ő�CPU:2��(V2,V3) */
#else
#if defined(Z80CARD)
#define MAXCPU			4				/* �ő�CPU:4��(V1,Z80�L����) */
#else
#define MAXCPU			3				/* �ő�CPU:3��(V1,Z80������) */
#endif
#endif

#define BOOT_BASIC		0				/* BASIC���[�h */
#define BOOT_DOS		1				/* DOS���[�h */
#define BOOT_BUBBLE		2				/* �o�u�����[�h */

#define STATELOAD_ERROR		0			/* �X�e�[�g���[�h�G���[ */
#define STATELOAD_SUCCESS	1			/* �X�e�[�g���[�h���� */
#define STATELOAD_OPENERR	-1			/* �t�@�C���I�[�v���G���[ */
#define STATELOAD_HEADERR	-2			/* �w�b�_�G���[ */
#define STATELOAD_VERERR	-3			/* �o�[�W�����G���[ */

/*
 * ROM�t�@�C������`
 */
#define FBASIC_ROM		"FBASIC30.ROM"	/* F-BASIC V3.0 */
#define INITIATE_ROM	"INITIATE.ROM"	/* FM77AV INITIATER ROM */
#define BOOTBAS_ROM		"BOOT_BAS.ROM"	/* FM-7 BASIC BOOT */
#define BOOTDOS_ROM		"BOOT_DOS.ROM"	/* FM-7 5inch DOS BOOT */
#define BOOTBBL_ROM		"BOOT_BBL.ROM"	/* FM-7 Bubble BOOT */
#define BOOT1MB_ROM		"BOOT_1MB.ROM"	/* FM-77 1MB BOOT */
#define BOOTMMR_ROM		"BOOT_MMR.ROM"	/* FM-77 MMR BOOT */
#define FBASIC10_ROM	"FBASIC10.ROM"	/* F-BASIC V1.0 */
#define BOOTBAS8_ROM	"BOOTBAS8.ROM"	/* MICRO 8 BASIC BOOT */
#define BOOTDOS8_ROM	"BOOTDOS8.ROM"	/* MICRO 8 5inch DOS BOOT */
#define BOOTBBL8_ROM	"BOOTBBL8.ROM"	/* MICRO 8 Bubble BOOT */
#define BOOTSFD8_ROM	"BOOTSFD8.ROM"	/* MICRO 8 8inch DOS BOOT */
#define BOOTDBG8_ROM	"BOOTDBG8.ROM"	/* MICRO 8 F-BASIC DEBUG BOOT */
#define SUBSYSC_ROM		"SUBSYS_C.ROM"	/* FM-7 SUBSYSTEM */
#define SUBSYS8_ROM		"SUBSYS_8.ROM"	/* MICRO 8 SUBSYSTEM */
#define SUBSYSL4_ROM	"SUBSYSL4.ROM"	/* FM-77L4 400LINE SUBSYSTEM */
#define SUBSYSA_ROM		"SUBSYS_A.ROM"	/* FM77AV Type-A SUBSYSTEM 3 */
#define SUBSYSB_ROM		"SUBSYS_B.ROM"	/* FM77AV Type-B SUBSYSTEM 3 */
#define SUBSYSCG_ROM	"SUBSYSCG.ROM"	/* FM77AV CGROM/SUBSYSTEM 1/2 */
#define ANKCG16_ROM		"ANKCG16.ROM"	/* FM-77L4 400LINE ANK FONT */
#define KANJI_ROM_J78	"KANJI.ROM"		/* JIS78 LEVEL 1 KANJI ROM */
#define KANJI_ROM		"KANJI1.ROM"	/* JIS83 LEVEL 1 KANJI ROM */
#define KANJI_ROM2		"KANJI2.ROM"	/* JIS83 LEVEL 2 KANJI ROM */
#define JSUBSYS_ROM		"JSUBMON.ROM"	/* FM77-101 DICTIONARY ACCESS ROM */
#define DICT_ROM		"DICROM.ROM"	/* FM77-101/FM77-211 DICTIONARY ROM */
#define	DICT_RAM		"USERDIC.DAT"	/* FM77-211 BATTERY BACKUP RAM */
#define EXTSUB_ROM		"EXTSUB.ROM"	/* FM77AV40EX/SX EXTEND SUBSYSTEM */


/*
 *	Borland C++�ł̋S�̂悤��Warning�΍�
 */
#ifdef __BORLANDC__
#undef MAKELONG
#define MAKELONG(l, h)			(LONG)(((WORD)(l)) | ((DWORD)(h) << 16))
#undef MAKEWPARAM
#define MAKEWPARAM(l, h)		(WPARAM)(MAKELONG(l, h))
#undef MAKELPARAM
#define MAKELPARAM(l, h)		(LPARAM)(MAKELONG(l, h))
#endif

#ifdef __cplusplus
extern "C" {
#endif
/*
 *	��v�G���g��
 */

/* �V�X�e��(system.c) */
BOOL FASTCALL system_init(void);
										/* �V�X�e�� ������ */
void FASTCALL system_cleanup(void);
										/* �V�X�e�� �N���[���A�b�v */
void FASTCALL system_reset(void);
										/* �V�X�e�� ���Z�b�g */
void FASTCALL system_hotreset(void);
										/* �V�X�e�� �z�b�g���Z�b�g */
#if XM7_VER >= 3
void FASTCALL system_tabreset(void);
										/* �V�X�e�� TAB+���Z�b�g */
#endif
BOOL FASTCALL system_save(char *filename);
										/* �V�X�e�� �Z�[�u */
int FASTCALL system_load(char *filename);
										/* �V�X�e�� ���[�h */

/* �X�P�W���[��(schedule.c) */
BOOL FASTCALL schedule_init(void);
										/* �X�P�W���[�� ������ */
void FASTCALL schedule_cleanup(void);
										/* �X�P�W���[�� �N���[���A�b�v */
void FASTCALL schedule_reset(void);
										/* �X�P�W���[�� ���Z�b�g */
DWORD FASTCALL schedule_exec(DWORD microsec);
										/* ���s */
void FASTCALL schedule_main_fullspeed(void);
										/* ���C�������S�͋쓮(tape�p) */
void FASTCALL schedule_fullspeed(void);
										/* �S�͋쓮 */
void FASTCALL schedule_trace(void);
										/* �g���[�X */
BOOL FASTCALL schedule_setbreak(int cpu, WORD addr);
										/* �u���[�N�|�C���g�ݒ� */
BOOL FASTCALL schedule_setbreak2(int num, int cpu, WORD addr);
										/* �u���[�N�|�C���g�ݒ�(�ʒu�w��) */
BOOL FASTCALL schedule_setevent(int id, DWORD microsec, BOOL (FASTCALL *func)(void));
										/* �C�x���g�ǉ� */
BOOL FASTCALL schedule_delevent(int id);
										/* �C�x���g�폜 */
void FASTCALL schedule_handle(int id, BOOL (FASTCALL *func)(void));
										/* �C�x���g�n���h���ݒ� */
BOOL FASTCALL schedule_save(int fileh);
										/* �X�P�W���[�� �Z�[�u */
#if XM7_VER == 1
BOOL FASTCALL schedule_load(int fileh, int ver);
										/* �X�P�W���[�� ���[�h (V1) */
#else
BOOL FASTCALL schedule_load(int fileh, int ver, BOOL old);
										/* �X�P�W���[�� ���[�h */
#endif

/* �t�A�Z���u��(disasm.c) */
int FASTCALL disline(int cputype, WORD pc, char *buffer);
										/* �P�s�t�A�Z���u�� */

/* Z80�t�A�Z���u��(disasm80.c) */
int FASTCALL disline80(int cputype, WORD pc, char *buffer);
										/* �P�s�t�A�Z���u�� */

/* ���C��CPU������(mainmem.c) */
BOOL FASTCALL mainmem_init(void);
										/* ���C��CPU������ ������ */
void FASTCALL mainmem_cleanup(void);
										/* ���C��CPU������ �N���[���A�b�v */
void FASTCALL mainmem_reset(void);
										/* ���C��CPU������ ���Z�b�g */
void FASTCALL mainmem_transfer_boot(void);
										/* ���C��CPU������ �u�[�g�]�� */
BYTE FASTCALL mainmem_readb(WORD addr);
										/* ���C��CPU������ �ǂݏo�� */
BYTE FASTCALL mainmem_readbnio(WORD addr);
										/* ���C��CPU������ �ǂݏo��(I/O�Ȃ�) */
void FASTCALL mainmem_writeb(WORD addr, BYTE dat);
										/* ���C��CPU������ �������� */
BYTE FASTCALL mainmem_readb_p(DWORD raddr);
										/* ���C��CPU������ �ǂݏo��(����) */
BYTE FASTCALL mainmem_readbnio_p(DWORD raddr);
										/* ���C��CPU������ �ǂݏo��(����2) */
void FASTCALL mainmem_writeb_p(DWORD raddr, BYTE dat);
										/* ���C��CPU������ ��������(����) */
BOOL FASTCALL mainmem_save(int fileh);
										/* ���C��CPU������ �Z�[�u */
BOOL FASTCALL mainmem_load(int fileh, int ver);
										/* ���C��CPU������ ���[�h */

/* �T�uCPU������(submem.c) */
BOOL FASTCALL submem_init(void);
										/* �T�uCPU������ ������ */
void FASTCALL submem_cleanup(void);
										/* �T�uCPU������ �N���[���A�b�v */
void FASTCALL submem_reset(void);
										/* �T�uCPU������ ���Z�b�g */
BYTE FASTCALL submem_readb(WORD addr);
										/* �T�uCPU������ �ǂݏo�� */
BYTE FASTCALL submem_readbnio(WORD addr);
										/* �T�uCPU������ �ǂݏo��(I/O�Ȃ�) */
void FASTCALL submem_writeb(WORD addr, BYTE dat);
										/* �T�uCPU������ �������� */
BOOL FASTCALL submem_save(int fileh);
										/* �T�uCPU������ �Z�[�u */
BOOL FASTCALL submem_load(int fileh, int ver);
										/* �T�uCPU������ ���[�h */

/* ���C��CPU(maincpu.c) */
BOOL FASTCALL maincpu_init(void);
										/* ���C��CPU ������ */
void FASTCALL maincpu_cleanup(void);
										/* ���C��CPU �N���[���A�b�v */
void FASTCALL maincpu_reset(void);
										/* ���C��CPU ���Z�b�g */
void FASTCALL maincpu_execline(void);
										/* ���C��CPU �P�s���s */
void FASTCALL maincpu_exec(void);
										/* ���C��CPU ���s */
void FASTCALL maincpu_nmi(void);
										/* ���C��CPU NMI���荞�ݗv�� */
void FASTCALL maincpu_firq(void);
										/* ���C��CPU FIRQ���荞�ݗv�� */
void FASTCALL maincpu_irq(void);
										/* ���C��CPU IRQ���荞�ݗv�� */
BOOL FASTCALL maincpu_save(int fileh);
										/* ���C��CPU �Z�[�u */
BOOL FASTCALL maincpu_load(int fileh, int ver);
										/* ���C��CPU ���[�h */

/* �T�uCPU(subcpu.c) */
BOOL FASTCALL subcpu_init(void);
										/* �T�uCPU ������ */
void FASTCALL subcpu_cleanup(void);
										/* �T�uCPU �N���[���A�b�v */
void FASTCALL subcpu_reset(void);
										/* �T�uCPU ���Z�b�g */
void FASTCALL subcpu_execline(void);
										/* �T�uCPU �P�s���s */
void FASTCALL subcpu_exec(void);
										/* �T�uCPU ���s */
void FASTCALL subcpu_nmi(void);
										/* �T�uCPU NMI���荞�ݗv�� */
void FASTCALL subcpu_firq(void);
										/* �T�uCPU FIRQ���荞�ݗv�� */
void FASTCALL subcpu_irq(void);
										/* �T�uCPU IRQ���荞�ݗv�� */
BOOL FASTCALL subcpu_save(int fileh);
										/* �T�uCPU �Z�[�u */
BOOL FASTCALL subcpu_load(int fileh, int ver);
										/* �T�uCPU ���[�h */

/*
 *	CPU�A���̑���v���[�N�G���A
 */
extern cpu6809_t maincpu;
										/* ���C��CPU */
extern cpu6809_t subcpu;
										/* �T�uCPU */
#if XM7_VER == 1
extern cpu6809_t jsubcpu;
										/* ���{��T�uCPU */
#if defined(Z80CARD)
extern KMZ80_CONTEXT mainz80;
										/* ���C��CPU�R���e�L�X�g(Z80) */
extern BOOL main_z80mode;
										/* ���C��CPU�I���t���O(FALSE:6809,TRUE:Z80) */
#endif
#endif
extern DWORD main_cycles;
										/* ���C��CPU���s�T�C�N���� */
extern int fm7_ver;
										/* ����o�[�W���� */
extern breakp_t breakp[BREAKP_MAXNUM];
										/* �u���[�N�|�C���g */
extern event_t event[EVENT_MAXNUM];
										/* �C�x���g */
extern BOOL run_flag;
										/* ����t���O */
extern BOOL stopreq_flag;
										/* ��~�v���t���O */
extern DWORD main_speed;
										/* ���C��CPU�X�s�[�h */
extern DWORD mmr_speed;
										/* ���C��CPU(MMR)�X�s�[�h */
#if XM7_VER >= 3
extern DWORD fmmr_speed;
										/* ���C��CPU(����MMR)�X�s�[�h */
#endif
extern DWORD sub_speed;
										/* �T�uCPU�X�s�[�h */
extern DWORD speed_ratio;
										/* �T�uCPU�X�s�[�h */
extern BOOL cycle_steal;
										/* �T�C�N���X�`�[���t���O */
extern BOOL cycle_steal_default;
										/* �T�C�N���X�`�[���f�t�H���g�t���O */
extern BOOL subclock_mode;
										/* �T�uCPU ��u�����L���O���^�C�~���O */
extern DWORD vmtime;
										/* VM���z���� */
extern BOOL reset_flag;
										/* �V�X�e�����Z�b�g�t���O */
extern BOOL hotreset_flag;
										/* ���C���z�b�g���Z�b�g�t���O */
extern WORD main_overcycles;
										/* ���C��CPU�I�[�o�[�T�C�N���� */
extern WORD sub_overcycles;
										/* �T�uCPU�I�[�o�[�T�C�N���� */
#if defined(DEBUG)
extern DWORD main_cycle;
										/* ���C��CPU���s�T�C�N�����J�E���^ */
extern DWORD sub_cycle;
										/* �T�uCPU���s�T�C�N�����J�E���^ */
#endif
extern BYTE fetch_op;
										/* ���O�Ƀt�F�b�`�������߃R�[�h */
#if XM7_VER == 1
extern BYTE fm_subtype;
										/* �n�[�h�E�F�A�T�u�o�[�W���� */
extern BOOL lowspeed_mode;
										/* CPU����N���b�N���[�h */
extern BOOL motoron_lowspeed;
										/* CMT�����[ON�������ᑬ���[�h */
extern DWORD main_speed_low;
										/* ���C��CPU�X�s�[�h(�ᑬ) */
extern DWORD sub_speed_low;
										/* �T�uCPU�X�s�[�h(�ᑬ) */
#if defined(JSUB)
extern DWORD jsub_speed;
										/* ���{��T�uCPU�X�s�[�h */
extern WORD jsub_overcycles;
										/* ���{��T�uCPU�I�[�o�[�T�C�N���� */
#endif
#if defined(Z80CARD)
extern BOOL main_z80mode;
										/* ���C��CPU Z80����t���O */
#endif
#endif

/*
 *	������
 */
extern BYTE *mainram_a;
										/* RAM (�\RAM)      $8000 */
extern BYTE *mainram_b;
										/* RAM (��RAM)      $7C80 */
extern BYTE *basic_rom;
										/* ROM (F-BASIC)    $7C00 */
extern BYTE *main_io;
										/* ���C��CPU I/O    $0100 */
extern BYTE *vram_c;
										/* VRAM(�^�C�vC)    $C000 ($30000) */
extern BYTE *subrom_c;
										/* ROM (�^�C�vC)    $2800 */
extern BYTE *sub_ram;
										/* �R���\�[��RAM    $1680 */
extern BYTE *sub_io;
										/* �T�uCPU I/O      $0100 */
#if XM7_VER == 1
extern BYTE *basic_rom8;
										/* ROM (F-BASIC1.0) $7C00 */
extern BYTE *boot_bas;
										/* BOOT (BASIC)      $200 */
extern BYTE *boot_dos;
										/* BOOT (DOS���[�h)  $200 */
extern BYTE *boot_bas8;
										/* BOOT (BASIC,FM-8) $200 */
extern BYTE *boot_dos8;
										/* BOOT (DOS,FM-8)   $200 */
extern BYTE *subrom_c;
										/* ROM (FM-8)       $2800 */
extern BYTE *subramcg;
										/* RAM (PCG)         $800 */
extern BOOL pcg_flag;
										/* PCG�t���O */
extern BOOL available_fm7roms;
										/* FM-7 ROM�g�p�\�t���O */
extern BOOL available_fm8roms;
										/* MICRO 8 ROM�g�p�\�t���O */
#if defined(BUBBLE)
extern BOOL bubble_available;
										/* �o�u���g�p�\�t���O */
#endif
#endif
#if (XM7_VER == 1) || (XM7_VER >= 3)
extern BYTE *boot_mmr;
										/* BOOT (�B��)       $200 */
extern BOOL available_mmrboot;
										/* FM-77 MMR�u�[�gROM�g�p�\�t���O */
#endif
extern BOOL boot_mode;
										/* �N�����[�h */
extern BOOL basicrom_en;
										/* F-BASIC V3.0 ROM �C�l�[�u�� */
extern BYTE *boot_ram;
										/* BOOT (RAM)        $200 */
extern BOOL bootram_rw;
										/* �u�[�gRAM �������݉\ */
extern BOOL rom_ram_write;
										/* FM-7���[�h��ROM�̈揑���̋��� */
#if XM7_VER >= 3
extern BOOL init_is_exsx;
										/* �C�j�V�G�[�^ROM EX/SX�t���O */
#endif

/*
 *	������ (FM-77)
 */
#if XM7_VER == 1
extern BYTE *extram_a;
										/* RAM (FM-77)     $30000 */
#if defined(L4CARD)
extern BYTE *tvram_c;
										/* �e�L�X�gVRAM(L4) $1000 */
extern BYTE *subrom_l4;
										/* �T�u���j�^ (L4)  $4800 */
extern BYTE *subcg_l4;
										/* CGROM (L4)       $1000 */
extern BOOL ankcg_force_internal;
										/* �����t�H���g�����g�p�t���O */
extern BOOL enable_400linecard;
										/* 400���C���L���t���O */
extern BOOL detect_400linecard;
										/* 400���C���J�[�h�����t���O */
extern BOOL detect_400linecard_tmp;
										/* 400���C���J�[�h�����t���O(tmp) */
#endif
#endif

/*
 *	������ (FM77AV)
 */
#if XM7_VER >= 2
extern BYTE *extram_a;
										/* RAM (FM77AV)    $10000 */
extern BYTE *init_rom;
										/* �C�j�V�G�[�^ROM  $2000 */

extern BYTE *subrom_a;
										/* ROM (�^�C�vA)    $2000 */
extern BYTE *subrom_b;
										/* ROM (�^�C�vB)    $2000 */
extern BYTE *subromcg;
										/* ROM (CG)         $2000 */
extern BYTE *vram_b;
										/* VRAM (�^�C�vA,B) $C000 */

extern BOOL initrom_en;
										/* �C�j�V�G�[�^ROM �C�l�[�u�� */
extern BYTE subrom_bank;
										/* �T�u�V�X�e��ROM/RAM�o���N */
extern BYTE cgrom_bank;
										/* CGROM�o���N */
#endif

/*
 *	������ (FM77AV40�E���{��J�[�h������)
 */
#if XM7_VER >= 3
extern BYTE *extram_c;
										/* RAM (FM77AV40)  $40000 */
extern BYTE *subramde;
										/* RAM (TypeD/E)    $2000 */
extern BYTE *subramcg;
										/* RAM (�t�H���g)   $4000 */
extern BYTE *subramcn;
										/* ���R���\�[��RAM  $2000 */

extern BYTE cgram_bank;
										/* CGRAM�o���N */
extern BYTE consram_bank;
										/* �R���\�[��RAM�o���N */
#endif

#ifdef __cplusplus
}
#endif

#endif	/* _xm7_h_ */
