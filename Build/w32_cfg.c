/*
 *	FM-7 EMULATOR "XM7"
 *
 *	Copyright (C) 1999-2017 �o�h�D(Twitter:@xm6_original)
 *	Copyright (C) 2001-2017 Ryu Takegami (Twitter:@RyuTakegami)
 *
 *	[ Win32API �R���t�B�M�����[�V���� ]
 *
 *	RHG����
 *	  2001.07.27		�W���C�X�e�B�b�N�̃f�t�H���g�ݒ���u�Ȃ��v�ɕύX
 *	  2001.12.26		�T�E���h�o�b�t�@���̐ݒ�͈͂�40ms�`2000ms�ɕύX
 *	  2002.03.10		�ݒ�I����̍ĕ`�挋�ʂ��������������C��
 *	  2002.04.05		FM������48kHz/88.2kHz/96kHz�����ݒ�ɑΉ�
 *	  2002.05.29		CPU���x/�T�E���h�o�b�t�@�T�C�Y/BEEP���g���̒��ڎw���
 *						�Ή�
 *	  2002.06.15		����@���ύX�����ꍇ��VM�����Z�b�g����悤�ɕύX
 *	  2002.09.08		�L�[���̓_�C�A���O��IME�̓�����֎~����悤�ɏC��
 *	  2002.09.09		INI�t�@�C���̃T�C�N���X�`�[���ݒ������̕ϐ����\�L��
 *						���킹��"CycleSteal"�ɕύX
 *	  2002.09.12		�t�@�C���I���f�t�H���g�f�B���N�g���̕ۑ��ɑΉ�
 *	  2002.09.16		�ݒ�y�[�W���Ƃ̃y�[�W�쐬�֐���p�~
 *	  2002.10.21		�f�o�b�O�o�[�W�����R���p�C�����ɑS�ʃy�[�W������������
 *						����������C��
 *	  2002.12.09		V2�߂��Ń`�����l���R�[���̐������o�Ȃ������C��
 *	  2003.01.19		�T�E���h�o�b�t�@�̃f�t�H���g�T�C�Y��100ms�ɕύX
 *						F10�Ɋ��蓖�Ă��L�[��F11�ւ̐U��ւ���p�~
 *	  2003.01.21		Keyboard�Z�N�V�����ۑ��܂����R�b�\���ύX(��
 *	  2003.03.09		�t�@�C���I���f�t�H���g�f�B���N�g���̎�ޕʕۑ��ɑΉ�
 *	  2003.05.02		�T�uCPU�̃f�t�H���g���s�T�C�N������ύX
 *	  2003.06.03		���C��CPU�E�T�uCPU�̃f�t�H���g���s�T�C�N�����������
 *	  2003.10.21		�t���X�L�������[�h���E�B���h�E���[�h�ƃt���X�N���[��
 *						���[�h�ŕʁX�ɐݒ�ł���悤�ɂ���
 *	  2004.03.17		�^��400���C�����[�h�֘A�̐ݒ荀�ڂ�ǉ�
 *	  2004.10.06		400���C���J�[�h�֘A�̐ݒ荀�ڂ�ǉ�
 *	  2005.10.15		�}�E�X���[�h�؂�ւ�����̐ݒ荀�ڂ�ǉ�
 *	  2005.11.12		�L�[�}�b�v�ݒ��FM-8�ł̃L�[���̂�ǉ�
 *	  2010.01.23		���ʒ����y�[�W������
 *	  2010.04.24		�N�����[�h�̕ۑ��ɑΉ�
 *	  2012.05.01		�t���X�N���[����ԕۑ������ւ̑Ή�
 *	  2012.05.28		�J�[�\���L�[�ɂ��e���L�[�G�~�����[�V�����̃`�F�b�N��
 *						����Ȃ������C��
 *	  2012.06.03		�X�e�[�^�X�o�[�̕\����Ԃ̕ۑ��ɑΉ�
 *	  2012.07.01		�o�u���������֘A�̍��ڂ�ǉ�
 *	  2012.08.01		�t���X�N���[�����̉�ʊg��ɑΉ�
 */

#ifdef _WIN32

#define STRICT
#define WIN32_LEAN_AND_MEAN
#define NONAMELESSUNION
#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include <prsht.h>
#ifdef DINPUT8
#define DIRECTINPUT_VERSION		0x0800		/* DirectX8���w�� */
#else
#define DIRECTINPUT_VERSION		0x0300		/* DirectX3���w�� */
#endif
#include <dinput.h>
#include <mmsystem.h>
#include <stdlib.h>
#include <assert.h>
#include "xm7.h"
#include "device.h"
#include "mainetc.h"
#include "subctrl.h"
#include "fdc.h"
#include "tapelp.h"
#include "opn.h"
#include "keyboard.h"
#include "mmr.h"
#include "mouse.h"
#include "aluline.h"
#include "display.h"
#include "kanji.h"
#include "jcard.h"
#include "jsubsys.h"
#include "rs232c.h"
#include "bubble.h"
#include "w32.h"
#include "w32_bar.h"
#include "w32_cfg.h"
#include "w32_sch.h"
#include "w32_snd.h"
#include "w32_kbd.h"
#include "w32_dd.h"
#include "w32_draw.h"
#include "w32_gdi.h"
#include "w32_sub.h"
#include "w32_res.h"
#include "w32_comm.h"
#include "w32_midi.h"
#include "juliet.h"
#include "w32_lpr.h"


/*
 *	�ݒ�f�[�^��`
 */
typedef struct {
	int fm7_ver;						/* �n�[�h�E�F�A�o�[�W���� */
	int boot_mode;						/* �u�[�g���[�h */
	BOOL cycle_steal;					/* �T�C�N���X�`�[���t���O */
	BOOL subclock_mode;					/* �T�uCPU ��u�����L���O���^�C�~���O */
	DWORD main_speed;					/* ���C��CPU�X�s�[�h */
	DWORD mmr_speed;					/* ���C��CPU(MMR)�X�s�[�h */
#if XM7_VER >= 3
	DWORD fmmr_speed;					/* ���C��CPU(����MMR)�X�s�[�h */
#endif
	DWORD sub_speed;					/* �T�uCPU�X�s�[�h */
	DWORD uTimerResolution;				/* �}���`���f�B�A�^�C�}�[���x */
	BOOL bTapeFull;						/* �e�[�v���[�^���̑��x�t���O */
#if !defined(DISABLE_FULLSPEED)
	BOOL bCPUFull;						/* �S�͋쓮�t���O */
	BOOL bSpeedAdjust;					/* �������x���� */
#endif
	BOOL bTapeMode;						/* �e�[�v���[�^���x����^�C�v */
#if XM7_VER == 1
	BYTE fm_subtype;					/* �n�[�h�E�F�A�T�u�o�[�W���� */
	BOOL lowspeed_mode;					/* CPU����N���b�N���[�h */
	DWORD main_speed_low;				/* ���C��CPU�X�s�[�h(�ᑬ) */
	DWORD sub_speed_low;				/* �T�uCPU�X�s�[�h(�ᑬ) */
#if defined(JSUB)
	DWORD jsub_speed;					/* ���{��T�uCPU�X�s�[�h */
#endif
#endif

	int nSampleRate;					/* �T���v�����O���[�g */
	int nSoundBuffer;					/* �T�E���h�o�b�t�@�T�C�Y */
	int nBeepFreq;						/* BEEP���g�� */
	BOOL bInterpolation;				/* �T�E���h��ԏo�� */
	int uStereoOut;						/* �o�̓��[�h */
	BOOL bForceStereo;					/* �����X�e���I�o�� */
	BOOL bTapeMon;						/* �e�[�v�����j�^ */
#if defined(ROMEO)
	BOOL bUseRomeo;						/* ��݂��g�p�t���O */
#endif
	UINT uChSeparation;					/* �X�e���I�`�����l���Z�p���[�V���� */
	int nFMVolume;						/* FM�����{�����[�� */
	int nPSGVolume;						/* PSG�{�����[�� */
	int nBeepVolume;					/* BEEP���{�����[�� */
	int nCMTVolume;						/* CMT�����j�^�{�����[�� */
	int nWaveVolume;					/* �e����ʉ��{�����[�� */

	BYTE KeyMap[256];					/* �L�[�ϊ��e�[�u�� */
	BOOL bKbdReal;						/* �[�����A���^�C���L�[�X�L���� */
	BOOL bTenCursor;					/* �����L�[���e���L�[�ɑΉ� */
	BOOL bArrow8Dir;					/* �e���L�[�ϊ� 8�������[�h */
#if defined(KBDPASTE)
	UINT uPasteWait;					/* �\��t�����̕����P�ʑ҂�����(ms) */
	UINT uPasteWaitCntl;				/* �\��t�����̃R���g���[���R�[�h�P�ʑ҂�����(ms) */
#endif

	int nJoyType[2];					/* �W���C�X�e�B�b�N�^�C�v */
	int nJoyRapid[2][2];				/* �W���C�X�e�B�b�N�A�� */
	int nJoyCode[2][7];					/* �W���C�X�e�B�b�N�R�[�h */

	BYTE nDDResolutionMode;				/* �t���X�N���[�����̉𑜓x */
	BOOL bFullScan;						/* �t���X�L����(�E�B���h�E���[�h) */
	BOOL bFullScanFS;					/* �t���X�L����(�t���X�N���[��) */
	BOOL bFullScreen;					/* �t���X�N���[�����[�h */
	BOOL bDoubleSize;					/* 2�{�\���t���O */
	BOOL bDD480Status;					/* 640x480�㉺�X�e�[�^�X */
	BOOL bDDtruecolor;					/* TrueColor�D��t���O */
	BOOL bRasterRender;					/* ���X�^�����_�����O */
	BOOL bDrawAfterVSYNC;				/* �`��VSYNC�^�C�~���O */
	BOOL bHideStatus;					/* �X�e�[�^�X�o�[��\�� */
#if XM7_VER == 1
	BOOL bGreenMonitor;					/* �O���[�����j�^���[�h */
#endif
#if XM7_VER == 2
	BOOL bTTLMonitor;					/* TTL���j�^���[�h */
#endif
	BOOL bPseudo400Line;				/* �^��400���C�����[�h */

	BOOL bOPNEnable;					/* OPN�L���t���O(7 only) */
	BOOL bWHGEnable;					/* WHG�L���t���O */
	BOOL bTHGEnable;					/* THG�L���t���O */
#if XM7_VER == 1
	BOOL bFMXEnable;					/* FM-X PSG�L���t���O */
#endif
#if XM7_VER >= 2
	BOOL bDigitizeEnable;				/* �f�B�W�^�C�Y�L���t���O */
	BOOL bJCardEnable;					/* ���{��J�[�h�L���t���O */
#endif
#if ((XM7_VER >= 3) || defined(FMTV151))
	BOOL bExtRAMEnable;					/* �g��RAM�L���t���O */
	BYTE uExtRAMMode;					/* �g��RAM���샂�[�h */
#endif
#if XM7_VER == 1
#if defined(L4CARD)
	BOOL b400LineCardEnable;			/* 400���C���J�[�h�L���t���O */
	BYTE uExtRAMMode;					/* �g��RAM���샂�[�h */
#endif
#if defined(JSUB)
	BOOL bJSubEnable;					/* ���{��T�u�V�X�e���L���t���O */
#endif
#if defined(BUBBLE)
	BOOL bBubbleEnable;					/* �o�u���������L���t���O */
#endif
	BYTE uBankSelectEnable;				/* �o���N�؂芷���C�l�[�u���t���O */
#endif
#if defined(MOUSE)
	BOOL bMouseCapture;					/* �}�E�X�L���v�`���t���O */
	BYTE nMousePort;					/* �}�E�X�ڑ��|�[�g */
	BYTE uMidBtnMode;					/* �����{�^����Ԏ擾���[�h */
#endif

#if defined(FDDSND)
	BOOL bFddWait;						/* FDD�E�F�C�g */
	BOOL bFddSound;						/* FDD�V�[�N�T�E���h */
#endif

#if defined(RSC)
	BOOL bCommPortEnable;				/* RS-232C�G�~�����[�V�����L���t���O */
	BYTE uCommPortBps;					/* �V���A���|�[�g�ʐM���x */
	int nCommPortNo;					/* �V���A���|�[�g�ԍ� */
#endif

#if defined(MIDI)
	char szMidiDevice[256];				/* MIDI�f�o�C�X�� */
	int nMidiDelay;						/* MIDI�����x������ */
	BOOL bMidiDelayMode;				/* MIDI�x����SoundBuffer�ɍ��킹�� */
#endif

#if defined(LPRINT)
	BYTE uPrinterEnable;				/* �v�����^�G�~�����[�V�������[�h */
	BOOL bLprUseOsFont;					/* OS�̃t�H���g�𗘗p���� */
	BOOL bLprOutputKanji;				/* �������o�͂��� */
	char szLprLogPath[256+1];			/* �v�����^���O�o�̓p�X */
#endif

#if XM7_VER == 1
	BOOL bPcgFlag;						/* PCG�t���O */
#endif
#if XM7_VER >= 2
	BOOL bLineBoost;					/* ������ԑS���͕`��t���O */
#endif
#if XM7_VER >= 3
	BOOL bGravestone;					/* !? */
	BOOL b400LineTiming;				/* ??? */
	BOOL bSubModeFix;					/* FM77AV�pOS-9�΍�t���O */
#endif

	BOOL bPopupSwnd;					/* �T�u�E�B���h�E�|�b�v�A�b�v��� */
	BOOL bOFNCentering;					/* �t�@�C���_�C�A���O�̃Z���^�����O */
	BOOL bMagusPatch;					/* MAGUS�΍􏈗� */
	BOOL bRomRamWrite;					/* FM-7���[�h���̗�RAM���������ύX */
	BOOL bFdcEnable;					/* FDC�C�l�[�u�� */
	BOOL bExtDetDisable;				/* EXTDET�f�B�Z�[�u�� */
#if XM7_VER == 1
	BOOL bMotorOnLowSpeed;				/* CMT���[�^ON�������ᑬ���[�h */
#endif
#if defined(KBDPASTE)
	BOOL bKeyStrokeModeless;			/* �L�[���͎x���_�C�A���O���[�h */
#endif
} configdat_t;

/*
 *	�X�^�e�B�b�N ���[�N
 */
static UINT uPropertyState;				/* �v���p�e�B�V�[�g�i�s�� */
static UINT uPropertyHelp;				/* �w���vID */
static UINT KbdPageSelectID;			/* �L�[�{�[�h�_�C�A���O */
static UINT KbdPageCurrentKey;			/* �L�[�{�[�h�_�C�A���O */
static BYTE KbdPageMap[256];			/* �L�[�{�[�h�_�C�A���O */
static UINT JoyPageIdx;					/* �W���C�X�e�B�b�N�y�[�W */ 
static configdat_t configdat;			/* �R���t�B�O�p�f�[�^ */
static configdat_t propdat;				/* �v���p�e�B�V�[�g�p�f�[�^ */
static char szIniFile[_MAX_PATH];		/* INI�t�@�C���� */
static char *pszSection;				/* �Z�N�V������ */


/*
 *	�v���g�^�C�v�錾
 */
static void FASTCALL SheetInit(HWND hDlg);

/*
 *	�R�����R���g���[���ւ̃A�N�Z�X�}�N��
 */
#define UpDown_GetPos(hwnd) \
	(DWORD)SendMessage((hwnd), UDM_GETPOS, 0, 0L)

#define UpDown_SetPos(hwnd, nPos) \
	SendMessage((hwnd), UDM_SETPOS, 0, MAKELPARAM(nPos, 0))

#define UpDown_SetRange(hwnd, nUpper, nLower) \
	SendMessage((hwnd), UDM_SETRANGE, 0, MAKELPARAM(nUpper, nLower))

/*
 *	�p�X�ۑ��p�L�[��
 */
static const char *InitDirStr[] = {
	"DiskImageDir",
	"TapeImageDir",
	"StateFileDir",
	"BMPFileDir",
	"WAVFileDir",
#if XM7_VER == 1
#if defined(BUBBLE)
	"BubbleImageDir",
#endif
#endif
	"LptLogFileDir",
};

/*-[ �ݒ�f�[�^ ]-----------------------------------------------------------*/

/*
 *	�ݒ�f�[�^
 *	�t�@�C�����w��
 */
static void FASTCALL SetCfgFile(void)
{
	char path[_MAX_PATH];
	char drive[_MAX_DRIVE];
	char dir[_MAX_DIR];
	char fname[_MAX_FNAME];

	/* INI�t�@�C�����ݒ� */
	GetModuleFileName(NULL, path, sizeof(path));
	_splitpath(path, drive, dir, fname, NULL);
	strncpy(path, drive, sizeof(path));
	strncat(path, dir, sizeof(path) - strlen(path) - 1);
	strncat(path, fname, sizeof(path) - strlen(path) - 1);
	strncat(path, ".INI", sizeof(path) - strlen(path) - 1);

	strncpy(szIniFile, path, sizeof(szIniFile));
}

/*
 *	�ݒ�f�[�^
 *	�Z�N�V�������w��
 */
static void FASTCALL SetCfgSection(char *section)
{
	ASSERT(section);

	/* �Z�N�V�������ݒ� */
	pszSection = section;
}

/*
 *	�ݒ�f�[�^
 *	���[�h(������)
 */
static BOOL LoadCfgString(char *key, char *buf, int length)
{
	ASSERT(key);

	GetPrivateProfileString(pszSection, key, ";", buf, length, szIniFile);

	if (buf[0] == ';') {
		return FALSE;
	}
	return TRUE;
}

/*
 *	�ݒ�f�[�^
 *	���[�h(int)
 */
static int LoadCfgInt(char *key, int def)
{
	ASSERT(key);

	return (int)GetPrivateProfileInt(pszSection, key, def, szIniFile);
}

/*
 *	�ݒ�f�[�^
 *	���[�h(BOOL)
 */
static BOOL FASTCALL LoadCfgBool(char *key, BOOL def)
{
	int dat;

	ASSERT(key);

	/* �ǂݍ��� */
	if (def) {
		dat = LoadCfgInt(key, 1);
	}
	else {
		dat = LoadCfgInt(key, 0);
	}

	/* �]�� */
	if (dat != 0) {
		return TRUE;
	}
	else {
		return FALSE;
	}
}

/*
 *	�ݒ�f�[�^
 *	���[�h
 */
void FASTCALL LoadCfg(void)
{
	int i;
	int j;
	char string[128];
	char path[_MAX_PATH];
	char dir[_MAX_DIR];
	char InitDir[_MAX_DIR + _MAX_PATH];
	BOOL flag;
	static const int JoyTable[] = {
		0x70, 0x71, 0x72, 0x73, 0, 0x74, 0x75
	};

	SetCfgFile();

	/* General�Z�N�V���� */
	SetCfgSection("General");
	if (!LoadCfgString("Directory", InitDir, MAX_PATH)) {
		GetModuleFileName(NULL, path, sizeof(path));
		_splitpath(path, InitDir, dir, NULL, NULL);
		if (dir[strlen(dir)-1] == '\\') {
			/* �Ō�̃p�X��؂�L���͋����I�ɍ�� */
			dir[strlen(dir)-1] = '\0';
		}
		strncat(InitDir, dir, sizeof(InitDir) - strlen(InitDir) - 1);
	}
#if XM7_VER == 1
#if defined(BUBBLE)
#if defined(LPRINT)
	for (i=0; i<7; i++) {
#else
	for (i=0; i<6; i++) {
#endif
#else
	for (i=0; i<5; i++) {
#endif
#else
#if defined(LPRINT)
	for (i=0; i<6; i++) {
#else
	for (i=0; i<5; i++) {
#endif
#endif
		if (!LoadCfgString((char *)InitDirStr[i], InitialDir[i], MAX_PATH)) {
			strncpy(InitialDir[i], InitDir, sizeof(InitDir));
		}
	}

#if XM7_VER >= 2
#if XM7_VER >= 3
	configdat.fm7_ver = LoadCfgInt("Version", 3);
	if ((configdat.fm7_ver < 1) || (configdat.fm7_ver > 3)) {
		configdat.fm7_ver = 3;
	}
#else
	configdat.fm7_ver = LoadCfgInt("Version", 2);
	if ((configdat.fm7_ver < 1) || (configdat.fm7_ver > 2)) {
		configdat.fm7_ver = 2;
	}
#endif
#else
	configdat.fm7_ver = 1;
	configdat.fm_subtype = (BYTE)LoadCfgInt("SubVersion", FMSUB_FM77);
	configdat.lowspeed_mode = LoadCfgBool("LowSpeedMode", FALSE);
#endif
	configdat.boot_mode = LoadCfgInt("BootMode", BOOT_BASIC);
#if XM7_VER == 1 
#if defined(BUBBLE)
	if ((configdat.boot_mode < BOOT_BASIC) || (configdat.boot_mode > BOOT_BUBBLE)) {
		configdat.boot_mode = BOOT_BASIC;
	}
	if (configdat.boot_mode == BOOT_BUBBLE && !bubble_available) {
		configdat.boot_mode = BOOT_DOS;
	}
#else
	if ((configdat.boot_mode < BOOT_BASIC) || (configdat.boot_mode > BOOT_DOS)) {
		configdat.boot_mode = BOOT_BASIC;
	}
#endif
#else
	if ((configdat.boot_mode < BOOT_BASIC) || (configdat.boot_mode > BOOT_DOS)) {
		configdat.boot_mode = BOOT_BASIC;
	}
#endif

	i = LoadCfgInt("CycleSteel", 9999);
	if ((i < 0) || (i > 1)) {
		configdat.cycle_steal = LoadCfgBool("CycleSteal", TRUE);
	}
	else {
		configdat.cycle_steal = (BOOL)i;
	}
	configdat.subclock_mode = LoadCfgBool("SubClockMode", FALSE);
	configdat.main_speed = LoadCfgInt("MainSpeed", MAINCYCLES);
	if ((configdat.main_speed < 1) || (configdat.main_speed > 9999)) {
		configdat.main_speed = MAINCYCLES;
	}
	configdat.mmr_speed = LoadCfgInt("MMRSpeed", MAINCYCLES_MMR);
	if ((configdat.mmr_speed < 1) || (configdat.mmr_speed > 9999)) {
		configdat.mmr_speed = MAINCYCLES_MMR;
	}
#if XM7_VER >= 3
	configdat.fmmr_speed = LoadCfgInt("FastMMRSpeed", MAINCYCLES_FMMR);
	if ((configdat.fmmr_speed < 1) || (configdat.fmmr_speed > 9999)) {
		configdat.fmmr_speed = MAINCYCLES_FMMR;
	}
#endif
	configdat.sub_speed = LoadCfgInt("SubSpeed", SUBCYCLES);
	if ((configdat.sub_speed < 1) || (configdat.sub_speed > 9999)) {
		configdat.sub_speed = SUBCYCLES;
	}
#if XM7_VER == 1
	configdat.main_speed_low = LoadCfgInt("MainSpeedLow", MAINCYCLES_LOW);
	if ((configdat.main_speed_low < 1) || (configdat.main_speed_low > 9999)) {
		configdat.main_speed_low = MAINCYCLES_LOW;
	}
	configdat.sub_speed_low = LoadCfgInt("SubSpeedLow", SUBCYCLES_LOW);
	if ((configdat.sub_speed_low < 1) || (configdat.sub_speed_low > 9999)) {
		configdat.sub_speed_low = SUBCYCLES_LOW;
	}
#if defined(JSUB)
	configdat.jsub_speed = LoadCfgInt("JsubSpeed", JSUBCYCLES);
	if ((configdat.jsub_speed < 1) || (configdat.jsub_speed > 9999)) {
		configdat.jsub_speed = SUBCYCLES;
	}
#endif
#endif
	configdat.bTapeFull = LoadCfgBool("TapeFullSpeed", TRUE);
#if !defined(DISABLE_FULLSPEED)
	configdat.bCPUFull = LoadCfgBool("FullSpeed", FALSE);
	configdat.bSpeedAdjust = LoadCfgBool("AutoSpeedAdjust", FALSE);
#endif
	configdat.bTapeMode = LoadCfgBool("TapeFullSpeedMode", FALSE);
#if defined(FDDSND)
	configdat.bFddWait = LoadCfgInt("FDDWait", FALSE);
#endif

	/* General�Z�N�V����(�E�B���h�E�ʒu) */
	WinPos.x = LoadCfgInt("WindowPosX", -99999);
	WinPos.y = LoadCfgInt("WindowPosY", -99999);

	/* �S�ʃy�[�W(�B��) */
	bHighPriority = LoadCfgBool("HighPriority", FALSE);
	configdat.uTimerResolution = LoadCfgInt("TimerResolution", 1);
	if ((configdat.uTimerResolution < 1) || (configdat.uTimerResolution > 10)) {
		configdat.uTimerResolution = 1;
	}

	/* Sound�Z�N�V���� */
	SetCfgSection("Sound");
	configdat.nSampleRate = LoadCfgInt("SampleRate", 44100);
	if ((configdat.nSampleRate != 0) &&
		(configdat.nSampleRate != 22050) &&
		(configdat.nSampleRate != 25600) &&
		(configdat.nSampleRate != 44100) &&
		(configdat.nSampleRate != 48000) &&
		(configdat.nSampleRate != 51200) &&
		(configdat.nSampleRate != 88200) &&
		(configdat.nSampleRate != 96000)) {
		configdat.nSampleRate = 44100;
	}
	configdat.nSoundBuffer = LoadCfgInt("SoundBuffer", 100);
	if ((configdat.nSoundBuffer < 40) || (configdat.nSoundBuffer > 1000)) {
		configdat.nSoundBuffer = 100;
	}
	configdat.nBeepFreq = LoadCfgInt("BeepFreq", 1200);
	if ((configdat.nBeepFreq < 100) || (configdat.nBeepFreq > 9999)) {
		configdat.nBeepFreq = 1200;
	}
	configdat.bInterpolation = LoadCfgBool("FMHQmode", TRUE);
	configdat.uStereoOut = LoadCfgInt("StereoOut", 0);
	if ((configdat.uStereoOut < 0) || (configdat.uStereoOut > 4)) {
		configdat.uStereoOut = 0;
	}
	configdat.bTapeMon = LoadCfgBool("TapeMon", FALSE);
	configdat.bForceStereo = LoadCfgInt("ForceStereoOutput", FALSE);
#if defined(ROMEO)
	configdat.bUseRomeo = LoadCfgBool("UseROMEO", TRUE);
#endif
#if defined(FDDSND)
	configdat.bFddSound = LoadCfgBool("FDDSound", FALSE);
#endif
	configdat.uChSeparation = LoadCfgInt("ChannelSeparation",
		CHSEPARATION_DEFAULT);
	if (configdat.uChSeparation > 16) {
		configdat.uChSeparation = CHSEPARATION_DEFAULT;
	}
	configdat.nFMVolume = LoadCfgInt("FMVolume", FMVOLUME_DEFAULT);
	if ((configdat.nFMVolume < -96) || (configdat.nFMVolume > 10)) {
		configdat.nFMVolume = FMVOLUME_DEFAULT;
	}
	configdat.nPSGVolume = LoadCfgInt("PSGVolume", PSGVOLUME_DEFAULT);
	if ((configdat.nPSGVolume < -96) || (configdat.nPSGVolume > 10)) {
		configdat.nPSGVolume = PSGVOLUME_DEFAULT;
	}
	configdat.nBeepVolume = LoadCfgInt("BeepVolume", BEEPVOLUME_DEFAULT);
	if ((configdat.nBeepVolume < -96) || (configdat.nBeepVolume > 0)) {
		configdat.nBeepVolume = BEEPVOLUME_DEFAULT;
	}
	configdat.nCMTVolume = LoadCfgInt("CMTVolume", CMTVOLUME_DEFAULT);
	if ((configdat.nCMTVolume < -96) || (configdat.nCMTVolume > 0)) {
		configdat.nCMTVolume = CMTVOLUME_DEFAULT;
	}
#if defined(FDDSND)
	configdat.nWaveVolume = LoadCfgInt("WaveVolume", WAVEVOLUME_DEFAULT);
	if ((configdat.nWaveVolume < -96) || (configdat.nWaveVolume > 0)) {
		configdat.nWaveVolume = WAVEVOLUME_DEFAULT;
	}
#endif

	/* Keyboard�Z�N�V���� */
	SetCfgSection("Keyboard");
	configdat.bKbdReal = LoadCfgBool("RealTimeKeyScan", FALSE);
	configdat.bTenCursor = LoadCfgBool("UseArrowFor10Key", FALSE);
	configdat.bArrow8Dir = LoadCfgBool("Arrow8Dir", TRUE);
	flag = FALSE;
	for (i=0; i<256; i++) {
		_snprintf(string, sizeof(string), "Key%d", i);
		j = i;
		configdat.KeyMap[j] = (BYTE)LoadCfgInt(string, 0);
		/* �ǂꂩ��ł����[�h�ł�����Aok */
		if (configdat.KeyMap[j] != 0) {
			flag = TRUE;
		}
	}
	/* �t���O���~��Ă���΁A�f�t�H���g�̃}�b�v�����炤 */
	if (!flag) {
		GetDefMapKbd(configdat.KeyMap, 0);
	}
#if defined(KBDPASTE)
	configdat.uPasteWait = LoadCfgInt("PasteWait", 0);
	configdat.uPasteWaitCntl = LoadCfgInt("PasteWaitCntl", 0);
	configdat.bKeyStrokeModeless = LoadCfgBool("KeyStrokeModeless", FALSE);
#endif

	/* JoyStick�Z�N�V���� */
	SetCfgSection("JoyStick");
	for (i=0; i<2; i++) {
		_snprintf(string, sizeof(string), "Type%d", i);
		configdat.nJoyType[i] = LoadCfgInt(string, 0);
		if ((configdat.nJoyType[i] < 0) || (configdat.nJoyType[i] > 4)) {
			configdat.nJoyType[i] = 0;
		}

		for (j=0; j<2; j++) {
			_snprintf(string, sizeof(string), "Rapid%d", i * 10 + j);
			configdat.nJoyRapid[i][j] = LoadCfgInt(string, 0);
			if ((configdat.nJoyRapid[i][j] < 0) || (configdat.nJoyRapid[i][j] > 9)) {
				configdat.nJoyRapid[i][j] = 0;
			}

		}

		flag = TRUE;
		for (j=0; j<7; j++) {
			_snprintf(string, sizeof(string), "Code%d", i * 10 + j);
			configdat.nJoyCode[i][j] = LoadCfgInt(string, -1);
			if ((configdat.nJoyCode[i][j] < 0) || (configdat.nJoyCode[i][j] > 0x75)) {
				flag = FALSE;
			}
		}
		/* �����W�G���[�Ȃ珉���l�ݒ� */
		if (!flag) {
			for (j=0; j<7; j++) {
				configdat.nJoyCode[i][j] = JoyTable[j];
			}
		}
	}

	/* Screen�Z�N�V���� */
	SetCfgSection("Screen");
	configdat.nDDResolutionMode = (BYTE)LoadCfgInt("DD480Line", 255);
	if (configdat.nDDResolutionMode == 255) {
		configdat.nDDResolutionMode = (BYTE)LoadCfgInt("DDResolution", 1);
	}
	if (configdat.nDDResolutionMode >= 5) {
		configdat.nDDResolutionMode = DDRES_480LINE;
	}
	configdat.bFullScan = LoadCfgBool("FullScan", FALSE);
	configdat.bFullScanFS = LoadCfgBool("FullScanFS", FALSE);
	configdat.bFullScreen = LoadCfgBool("FullScreen", FALSE);
	configdat.bDoubleSize = LoadCfgBool("DoubleSize", FALSE);
	configdat.bDD480Status = LoadCfgBool("DD480Status", TRUE);
	configdat.bRasterRender = LoadCfgBool("RasterRender", FALSE);
	configdat.bDrawAfterVSYNC = LoadCfgBool("DrawAfterVSYNC", TRUE);
#if XM7_VER == 3
	configdat.bDDtruecolor = LoadCfgBool("DDTrueColor", TRUE);
#else
	configdat.bDDtruecolor = LoadCfgBool("DDTrueColor", FALSE);
#endif
	configdat.bHideStatus = LoadCfgBool("HideStatusBar", FALSE);
#if XM7_VER == 1
	configdat.bGreenMonitor = LoadCfgBool("GreenMonitor", FALSE);
#endif
#if XM7_VER == 2
	configdat.bTTLMonitor = LoadCfgBool("TTLMonitor", FALSE);
#endif
	configdat.bPseudo400Line = LoadCfgBool("Pseudo400Line", FALSE);

	/* Option�Z�N�V���� */
	SetCfgSection("Option");
	configdat.bOPNEnable = LoadCfgBool("OPNEnable", TRUE);
	configdat.bWHGEnable = LoadCfgBool("WHGEnable", TRUE);
	configdat.bTHGEnable = LoadCfgBool("THGEnable", TRUE);
#if XM7_VER == 1
	configdat.bFMXEnable = LoadCfgBool("FMXEnable", FALSE);
#endif
#if XM7_VER >= 2
	configdat.bDigitizeEnable = LoadCfgBool("DigitizeEnable", TRUE);
	configdat.bJCardEnable = LoadCfgBool("JCardEnable", FALSE);
#endif
#if XM7_VER >= 3
	configdat.bExtRAMEnable = LoadCfgBool("ExtRAMEnable", FALSE);
	configdat.uExtRAMMode = (BYTE)LoadCfgInt("ExtRAMMode", 5);
	if ((configdat.uExtRAMMode < 4) || (configdat.uExtRAMMode > 5)) {
		configdat.uExtRAMMode = 5;
	}
#elif defined(FMTV151)
	configdat.bExtRAMEnable = bFMTV151;
#endif
#if XM7_VER == 1
#if defined(L4CARD)
	configdat.b400LineCardEnable = LoadCfgBool("400LineCardEnable", TRUE);
	configdat.uExtRAMMode = (BYTE)LoadCfgInt("ExtRAMMode", 2);
	if ((configdat.uExtRAMMode < 1) || (configdat.uExtRAMMode > 2)) {
		configdat.uExtRAMMode = 2;
	}
#endif
#if defined(JSUB)
	configdat.bJSubEnable = LoadCfgBool("JSubEnable", TRUE);
#endif
#if defined(BUBBLE)
	configdat.bBubbleEnable = LoadCfgBool("BubbleEnable", FALSE);
#endif
	configdat.uBankSelectEnable = LoadCfgInt("BankSelectEnable", 0);
#endif
#if defined(MOUSE)
	configdat.bMouseCapture = LoadCfgBool("MouseEmulation", FALSE);
	configdat.nMousePort = (BYTE)LoadCfgInt("MousePort", 1);
	if ((configdat.nMousePort < 1) || (configdat.nMousePort > 3)) {
		configdat.nMousePort = 1;
	}
	configdat.uMidBtnMode = (BYTE)LoadCfgInt("MidBtnMode", MOSCAP_NONE);
	if (configdat.uMidBtnMode > 2) {
		configdat.uMidBtnMode = 0;
	}
#endif

	/* Ports�Z�N�V���� */
#if defined(RSC) || defined(MIDI)
	SetCfgSection("Ports");
#if defined(RSC)
	configdat.bCommPortEnable = LoadCfgBool("CommPortEnable", FALSE);
	configdat.uCommPortBps = (BYTE)LoadCfgInt("CommPortBps", 0);
	if (configdat.uCommPortBps > 4) {
		configdat.uCommPortBps = 0;
	}
	configdat.nCommPortNo = LoadCfgInt("CommPort", 1);
	if ((configdat.nCommPortNo < 1) || (configdat.nCommPortNo > 16)) {
		configdat.nCommPortNo = 1;
	}
#endif

#if defined(MIDI)
	if (!LoadCfgString("MidiPort", configdat.szMidiDevice, 256)) {
		strncpy(configdat.szMidiDevice, "", sizeof(configdat.szMidiDevice));
	}
	configdat.nMidiDelay = LoadCfgInt("MidiDelay", 100);
	if ((configdat.nMidiDelay < 0) || (configdat.nMidiDelay > 1000)) {
		configdat.nMidiDelay = 100;
	}
	configdat.bMidiDelayMode = LoadCfgBool("MidiDelayMode", TRUE);
#endif
#endif	/* RSC/MIDI */

	/* Printer�Z�N�V���� */
#if defined(LPRINT)
	SetCfgSection("Printer");
	configdat.uPrinterEnable = (BYTE)LoadCfgInt("PrinterEnable", 0);
#if defined(JASTSOUND)
	if (configdat.uPrinterEnable > LP_JASTSOUND) {
#else
	if (configdat.uPrinterEnable > LP_LOG) {
#endif
		configdat.uPrinterEnable = 0;
	}
	configdat.bLprUseOsFont = LoadCfgBool("LprUseOsFont", FALSE);
	configdat.bLprOutputKanji = LoadCfgBool("LprOutputKanji", FALSE);
	if (!LoadCfgString("LprOutPath", configdat.szLprLogPath, 256)) {
		strncpy(configdat.szLprLogPath, "\0", sizeof(configdat.szLprLogPath));
	}
#endif /* LPRINT */

	/* Misc�Z�N�V���� */
	SetCfgSection("Misc");
	configdat.bPopupSwnd = LoadCfgBool("PopupSwnd", TRUE);
#if XM7_VER == 3
	configdat.bSubModeFix = LoadCfgBool("SubModeFix", FALSE);
#endif
	configdat.bOFNCentering = LoadCfgBool("OFNCentering", FALSE);
	configdat.bMagusPatch = LoadCfgBool("MagusPatch", FALSE);
	configdat.bRomRamWrite = LoadCfgBool("RomRamWrite", FALSE);
	configdat.bFdcEnable = LoadCfgBool("FdcEnable", TRUE);
#if XM7_VER == 1
	configdat.bPcgFlag = LoadCfgBool("PCGFlag", FALSE);
	configdat.bMotorOnLowSpeed = LoadCfgBool("MotorOnLowSpeed", TRUE);
#endif

	/* Unofficial�Z�N�V���� */
	SetCfgSection("Unofficial");
	kanji_asis_flag = LoadCfgBool("KanjiAsIs", FALSE);
#if XM7_VER >= 2
	configdat.bLineBoost = LoadCfgBool("LineBoost", FALSE);
#endif
#if XM7_VER >= 3
	configdat.bGravestone = LoadCfgBool("Gravestone", FALSE);
	configdat.b400LineTiming = LoadCfgBool("400LineTiming", FALSE);
#endif
#if XM7_VER == 1 && defined(L4CARD)
	ankcg_force_internal = LoadCfgBool("ForceInternalFont", FALSE);
#endif
	configdat.bExtDetDisable = LoadCfgBool("ExtDetDisable", FALSE);
}

/*
 *	�ݒ�f�[�^
 *	���[�h(2�{�g�僂�[�h��p)
 */
BOOL FASTCALL LoadCfg_DoubleSize(void)
{
	/* 2�{�g�僂�[�h�̏�Ԃ�ǂݍ���ŕԂ� */
	SetCfgFile();
	SetCfgSection("Screen");
	return LoadCfgBool("DoubleSize", FALSE);
}

/*
 *	�ݒ�f�[�^
 *	���[�h(����ݒ胂�[�h��p)
 */
BOOL FASTCALL LoadCfg_LanguageMode(void)
{
	/* 2�{�g�僂�[�h�̏�Ԃ�ǂݍ���ŕԂ� */
	SetCfgFile();
	SetCfgSection("Unofficial");
	return LoadCfgBool("LanguageMode", FALSE);
}

/*
 *	�ݒ�f�[�^
 *	�폜
 */
static void FASTCALL DeleteCfg(char *key)
{
	ASSERT(key);

	WritePrivateProfileString(pszSection, key, NULL, szIniFile);
}

/*
 *	�ݒ�f�[�^
 *	�Z�[�u(������)
 */
static void FASTCALL SaveCfgString(char *key, char *string)
{
	ASSERT(key);
	ASSERT(string);

	WritePrivateProfileString(pszSection, key, string, szIniFile);
}

/*
 *	�ݒ�f�[�^
 *	�Z�[�u(�S�o�C�gint)
 */
static void FASTCALL SaveCfgInt(char *key, int dat)
{
	char string[128];

	ASSERT(key);

	_snprintf(string, sizeof(string), "%d", dat);
	SaveCfgString(key, string);
}

/*
 *	�ݒ�f�[�^
 *	�Z�[�u(BOOL)
 */
static void FASTCALL SaveCfgBool(char *key, BOOL dat)
{
	ASSERT(key);

	if (dat) {
		SaveCfgInt(key, 1);
	}
	else {
		SaveCfgInt(key, 0);
	}
}

/*
 *	�ݒ�f�[�^
 *	�Z�[�u
 */
void FASTCALL SaveCfg(void)
{
	int i;
	int j;
	char string[128];

	SetCfgFile();

	/* General�Z�N�V���� */
	SetCfgSection("General");
	DeleteCfg("Directory");		/* V3.3L20�Ŏ�ʖ��̕ۑ��ɑΉ� */
	DeleteCfg("CycleSteel");	/* V3.2L01��CycleSteal�ɃL�[���̂�ύX */
#if XM7_VER == 1
#if defined(BUBBLE)
#if defined(LPRINT)
	for (i=0; i<7; i++) {
#else
	for (i=0; i<6; i++) {
#endif
#else
	for (i=0; i<5; i++) {
#endif
#else
#if defined(LPRINT)
	for (i=0; i<6; i++) {
#else
	for (i=0; i<5; i++) {
#endif
#endif
		SaveCfgString((char *)InitDirStr[i], InitialDir[i]);
	}
#if XM7_VER >= 2
	SaveCfgInt("Version", configdat.fm7_ver);
#else
	SaveCfgInt("SubVersion", configdat.fm_subtype);
	SaveCfgBool("LowSpeedMode", configdat.lowspeed_mode);
#endif
	SaveCfgInt("BootMode", configdat.boot_mode);
	SaveCfgBool("CycleSteal", configdat.cycle_steal);
	SaveCfgBool("SubClockMode", configdat.subclock_mode);
	SaveCfgInt("MainSpeed", configdat.main_speed);
	SaveCfgInt("MMRSpeed", configdat.mmr_speed);
#if XM7_VER >= 3
	SaveCfgInt("FastMMRSpeed", configdat.fmmr_speed);
#endif
	SaveCfgInt("SubSpeed", configdat.sub_speed);
#if XM7_VER == 1
	SaveCfgInt("MainSpeedLow", configdat.main_speed_low);
	SaveCfgInt("SubSpeedLow", configdat.sub_speed_low);
#if defined(JSUB)
	SaveCfgInt("JsubSpeed", configdat.jsub_speed);
#endif
#endif
	SaveCfgBool("TapeFullSpeed", configdat.bTapeFull);
	SaveCfgBool("TapeFullSpeedMode", configdat.bTapeMode);
#if !defined(DISABLE_FULLSPEED)
	SaveCfgBool("FullSpeed", configdat.bCPUFull);
	SaveCfgBool("AutoSpeedAdjust", configdat.bSpeedAdjust);
#endif
#if defined(FDDSND)
	SaveCfgBool("FDDWait", configdat.bFddWait);
#endif

	/* General�Z�N�V����(�E�B���h�E�ʒu) */
	SaveCfgInt("WindowPosX", WinPos.x);
	SaveCfgInt("WindowPosY", WinPos.y);

	/* Sound�Z�N�V���� */
	SetCfgSection("Sound");
	SaveCfgInt("SampleRate", configdat.nSampleRate);
	SaveCfgInt("SoundBuffer", configdat.nSoundBuffer);
	SaveCfgInt("BeepFreq", configdat.nBeepFreq);
	SaveCfgBool("FMHQmode", configdat.bInterpolation);
	SaveCfgInt("StereoOut", configdat.uStereoOut);
	SaveCfgBool("TapeMon", configdat.bTapeMon);
#if defined(ROMEO)
	SaveCfgBool("UseROMEO", configdat.bUseRomeo);
#endif
#if defined(FDDSND)
	SaveCfgBool("FDDSound", configdat.bFddSound);
#endif
	SaveCfgInt("ChannelSeparation", configdat.uChSeparation);
	SaveCfgInt("FMVolume", configdat.nFMVolume);
	SaveCfgInt("PSGVolume", configdat.nPSGVolume);
	SaveCfgInt("BeepVolume", configdat.nBeepVolume);
	SaveCfgInt("CMTVolume", configdat.nCMTVolume);
#if defined(FDDSND)
	SaveCfgInt("WaveVolume", configdat.nWaveVolume);
#endif

	/* Keyboard�Z�N�V���� */
	SetCfgSection("Keyboard");
	SaveCfgBool("RealTimeKeyScan", configdat.bKbdReal);
	SaveCfgBool("UseArrowFor10Key", configdat.bTenCursor);
	SaveCfgBool("Arrow8Dir", configdat.bArrow8Dir);
	for (i=0; i<256; i++) {
		_snprintf(string, sizeof(string), "Key%d", i);
		DeleteCfg(string);
		if (configdat.KeyMap[i] != 0) {
			SaveCfgInt(string, (BYTE)(configdat.KeyMap[i] & 0x7f));
		}
	}
#if defined(KBDPASTE)
	SaveCfgInt("PasteWait", configdat.uPasteWait);
	SaveCfgInt("PasteWaitCntl", configdat.uPasteWaitCntl);
#endif

	/* JoyStick�Z�N�V���� */
	SetCfgSection("JoyStick");
	for (i=0; i<2; i++) {
		_snprintf(string, sizeof(string), "Type%d", i);
		SaveCfgInt(string, configdat.nJoyType[i]);

		for (j=0; j<2; j++) {
			_snprintf(string, sizeof(string), "Rapid%d", i * 10 + j);
			SaveCfgInt(string, configdat.nJoyRapid[i][j]);
		}

		for (j=0; j<7; j++) {
			_snprintf(string, sizeof(string), "Code%d", i * 10 + j);
			SaveCfgInt(string, configdat.nJoyCode[i][j]);
		}
	}

	/* Screen�Z�N�V���� */
	SetCfgSection("Screen");
	DeleteCfg("DD480Line");		/* V3.4L51��DDResolution�ɃL�[���̂�ύX */
	DeleteCfg("DrawTiming");	/* V3.4L60�Ń��X�^�����_�����O���ڂɂ��p�~ */
	SaveCfgInt("DDResolution", configdat.nDDResolutionMode);
	SaveCfgBool("FullScan", configdat.bFullScan);
	SaveCfgBool("FullScanFS", configdat.bFullScanFS);
	SaveCfgBool("FullScreen", configdat.bFullScreen);
	SaveCfgBool("DoubleSize", configdat.bDoubleSize);
	SaveCfgBool("DD480Status", configdat.bDD480Status);
	SaveCfgBool("RasterRender", configdat.bRasterRender);
	SaveCfgBool("DrawAfterVSYNC", configdat.bDrawAfterVSYNC);
	SaveCfgBool("DDTrueColor", configdat.bDDtruecolor);
	SaveCfgBool("HideStatusBar", configdat.bHideStatus);
#if XM7_VER == 1
	SaveCfgBool("GreenMonitor", configdat.bGreenMonitor);
#endif
#if XM7_VER == 2
	SaveCfgBool("TTLMonitor", configdat.bTTLMonitor);
#endif
	SaveCfgBool("Pseudo400Line", configdat.bPseudo400Line);

	/* Option�Z�N�V���� */
	SetCfgSection("Option");
	DeleteCfg("SubBusyDelay");	/* V3.1�Ŕp�~ */
	SaveCfgBool("OPNEnable", configdat.bOPNEnable);
	SaveCfgBool("WHGEnable", configdat.bWHGEnable);
	SaveCfgBool("THGEnable", configdat.bTHGEnable);
#if XM7_VER >= 2
	SaveCfgBool("DigitizeEnable", configdat.bDigitizeEnable);
	SaveCfgBool("JCardEnable", configdat.bJCardEnable);
#endif
#if XM7_VER >= 3
	SaveCfgBool("ExtRAMEnable", configdat.bExtRAMEnable);
	SaveCfgInt("ExtRAMMode", configdat.uExtRAMMode);
#endif
#if XM7_VER == 1
#if defined(L4CARD)
	SaveCfgBool("400LineCardEnable", configdat.b400LineCardEnable);
	SaveCfgInt("ExtRAMMode", configdat.uExtRAMMode);
#endif
#if defined(JSUB)
	SaveCfgBool("JSubEnable", configdat.bJSubEnable);
#endif
#if defined(BUBBLE)
	SaveCfgBool("BubbleEnable", configdat.bBubbleEnable);
#endif
	SaveCfgInt("BankSelectEnable", configdat.uBankSelectEnable);
#endif
#if defined(MOUSE)
	SaveCfgBool("MouseEmulation", configdat.bMouseCapture);
	SaveCfgInt("MousePort", configdat.nMousePort);
	SaveCfgInt("MidBtnMode", configdat.uMidBtnMode);
#endif

	/* Ports�Z�N�V���� */
#if defined(RSC) || defined(MIDI)
	SetCfgSection("Ports");
#if defined(RSC)
	SaveCfgBool("CommPortEnable", configdat.bCommPortEnable);
	SaveCfgInt("CommPortBps", configdat.uCommPortBps);
	SaveCfgInt("CommPort", configdat.nCommPortNo);
#endif
#if defined(MIDI)
	SaveCfgString("MidiPort", configdat.szMidiDevice);
	SaveCfgInt("MidiDelay", configdat.nMidiDelay);
	SaveCfgBool("MidiDelayMode", configdat.bMidiDelayMode);
#endif
#endif	/* RSC/MIDI */

	/* Printer�Z�N�V���� */
#if defined(LPRINT)
	SetCfgSection("Printer");
	SaveCfgInt("PrinterEnable", configdat.uPrinterEnable);
	SaveCfgBool("LprUseOsFont", configdat.bLprUseOsFont);
	SaveCfgBool("LprOutputKanji", configdat.bLprOutputKanji);
	SaveCfgString("LprOutPath", configdat.szLprLogPath);
#endif /* LPRINT */

	/* Misc�Z�N�V���� */
	SetCfgSection("Misc");
	SaveCfgBool("MagusPatch", configdat.bMagusPatch);
	SaveCfgBool("RomRamWrite", configdat.bRomRamWrite);
	SaveCfgBool("OFNCentering", configdat.bOFNCentering);
	SaveCfgBool("PopupSwnd", configdat.bPopupSwnd);
	SaveCfgBool("FdcEnable", configdat.bFdcEnable);
#if XM7_VER == 1
	SaveCfgBool("MotorOnLowSpeed", configdat.bMotorOnLowSpeed);
#endif
}

/*
 *	�ݒ�f�[�^�K�p
 *	��VM�̃��b�N�͍s���Ă��Ȃ��̂Œ���
 */
void FASTCALL ApplyCfg(void)
{
	RECT rect;
	int i;
	char tmp[128];
	char buffer[256+128+1];

	tmp[0] = '\0';
	buffer[0] = '\0';

	/* General�Z�N�V���� */
	fm7_ver = configdat.fm7_ver;
	boot_mode = configdat.boot_mode;
	cycle_steal_default = configdat.cycle_steal;
	subclock_mode = configdat.subclock_mode;
	main_speed = configdat.main_speed * 10;
	mmr_speed = configdat.mmr_speed * 10;
#if XM7_VER >= 3
	fmmr_speed = configdat.fmmr_speed * 10;
#endif
	sub_speed = configdat.sub_speed * 10;
	bTapeFullSpeed = configdat.bTapeFull;
#if !defined(DISABLE_FULLSPEED)
	bFullSpeed = configdat.bCPUFull;
	bAutoSpeedAdjust = configdat.bSpeedAdjust;
#endif
	uTimerResolution = configdat.uTimerResolution;
#if defined(FDDSND)
	fdc_waitmode = configdat.bFddWait;
#endif
	bTapeModeType = configdat.bTapeMode;
#if XM7_VER == 1
	/* �g�p�s�ȃ��[�h���I������Ă���ꍇ�A�����I�Ƀ��[�h�ύX */
	if ((configdat.fm_subtype == FMSUB_FM8) && !available_fm8roms) {
		configdat.fm_subtype = FMSUB_FM77;
	}
	else if ((configdat.fm_subtype != FMSUB_FM8) && !available_fm7roms) {
		configdat.fm_subtype = FMSUB_FM8;
	}

	fm_subtype = configdat.fm_subtype;
	lowspeed_mode = configdat.lowspeed_mode;
	main_speed_low = configdat.main_speed_low * 10;
	sub_speed_low = configdat.sub_speed_low * 10;
#if defined(JSUB)
	jsub_speed = configdat.jsub_speed * 10;
#endif
#endif

	/* Sound�Z�N�V���� */
	nSampleRate = configdat.nSampleRate;
	nSoundBuffer = configdat.nSoundBuffer;
	nBeepFreq = configdat.nBeepFreq;
	bInterpolation = configdat.bInterpolation;
	uStereoOut = configdat.uStereoOut;
	bForceStereo = configdat.bForceStereo;
	bTapeMon = configdat.bTapeMon;
	tape_monitor = configdat.bTapeMon;
#if defined(ROMEO)
	if (bRomeo) {
		bUseRomeo = configdat.bUseRomeo;
		juliet_YMF288Mute(!bUseRomeo);
	}
	else {
		bUseRomeo = FALSE;
	}
#endif
	uChSeparation = configdat.uChSeparation;
	nFMVolume = configdat.nFMVolume;
	nPSGVolume = configdat.nPSGVolume;
	nBeepVolume = configdat.nBeepVolume;
	nCMTVolume = configdat.nCMTVolume;
#if defined(FDDSND)
	fdc_sound = configdat.bFddSound;
	tape_sound = configdat.bFddSound;
	nWaveVolume = configdat.nWaveVolume;
#endif
	ApplySnd();

	/* Keyboard�Z�N�V���� */
	SetMapKbd(configdat.KeyMap);
	bKbdReal = configdat.bKbdReal;
	bTenCursor = configdat.bTenCursor;
	bArrow8Dir = configdat.bArrow8Dir;
#if defined(KBDPASTE)
	uPasteWait = configdat.uPasteWait;
	uPasteWaitCntl = configdat.uPasteWaitCntl;
	bKeyStrokeModeless = configdat.bKeyStrokeModeless;
#endif

	/* JoyStick�Z�N�V���� */
	memcpy(nJoyType, configdat.nJoyType, sizeof(nJoyType));
	memcpy(nJoyRapid, configdat.nJoyRapid, sizeof(nJoyRapid));
	memcpy(nJoyCode, configdat.nJoyCode, sizeof(nJoyCode));

	/* Screen�Z�N�V���� */
	InvalidateRect(hDrawWnd, NULL, FALSE);
	nDDResolutionMode = configdat.nDDResolutionMode;
	bFullScan = configdat.bFullScan;
	bFullScanFS = configdat.bFullScanFS;
	if (bDrawSelected) {
		if (bFullScreen != configdat.bFullScreen) {
			PostMessage(hMainWnd, WM_COMMAND, IDM_FULLSCREEN, 0);
		}
	}
	else {
		bFullRequested = configdat.bFullScreen;
		bFullScreen = FALSE;
	}
	bDoubleSize = configdat.bDoubleSize;
	bDD480Status = configdat.bDD480Status;
	if (bRasterRendering != configdat.bRasterRender) {
		SelectCancelGDI();
	}
	bRasterRendering = configdat.bRasterRender;
	draw_aftervsync = configdat.bDrawAfterVSYNC;
	bDDtruecolor = configdat.bDDtruecolor;
	bHideStatus = configdat.bHideStatus;
#if XM7_VER == 1
	bGreenMonitor = configdat.bGreenMonitor;
#endif
#if XM7_VER == 2
	bTTLMonitor = configdat.bTTLMonitor;
#endif
	bPseudo400Line = configdat.bPseudo400Line;
#if defined(DISABLE_FULLSPEED)
	line_boost = configdat.bLineBoost;
#else
#if XM7_VER >= 2
	if (bFullSpeed) {
		line_boost = TRUE;
	}
	else {
		line_boost = configdat.bLineBoost;
	}
#endif
#endif

	/* Option�Z�N�V���� */
	opn_enable = configdat.bOPNEnable;
	whg_enable = configdat.bWHGEnable;
	thg_enable = configdat.bTHGEnable;
#if XM7_VER == 1
	fmx_flag = configdat.bFMXEnable;
#endif
#if XM7_VER >= 2
	digitize_enable = configdat.bDigitizeEnable;
	if ((jcard_enable != configdat.bJCardEnable) && (fm7_ver == 2)) {
		jcard_enable = configdat.bJCardEnable;
		system_reset();
	}
	else {
		jcard_enable = configdat.bJCardEnable;
	}
#endif
#if XM7_VER >= 3
	mmr_extram = configdat.bExtRAMEnable;

	/* �g��RAM���샂�[�h(�Ȃ���XM7dash�݊�) */
	if (configdat.uExtRAMMode == 5) {
		mmr_768kbmode = TRUE;
	}
	else {
		mmr_768kbmode = FALSE;
	}
#elif defined(FMTV151)
	bFMTV151 = configdat.bExtRAMEnable;
#endif
#if XM7_VER == 1
#if defined(L4CARD)
	if (detect_400linecard) {
		/* Enable��Disable����400���C�����[�h�ɂȂ��Ă��鎞�͋������Z�b�g */
		if (enable_400linecard && !configdat.b400LineCardEnable &&
			enable_400line) {
			system_reset();
			OnRefresh(hMainWnd);
		}
		enable_400linecard = configdat.b400LineCardEnable;
	}
	else {
		enable_400linecard = FALSE;
	}

	/* �g��RAM���샂�[�h(�Ȃ���XM7dash�݊�) */
	if (configdat.uExtRAMMode == 1) {
		mmr_64kbmode = TRUE;
	}
	else {
		mmr_64kbmode = FALSE;
	}
#endif
#if defined(JSUB)
	if (jsub_available) {
		jsub_enable = configdat.bJSubEnable;
	}
	else {
		jsub_enable = FALSE;
	}
#endif
#if defined(BUBBLE)
	bmc_enable = configdat.bBubbleEnable;
#endif
	banksel_en = configdat.uBankSelectEnable;
#endif
#if defined(MOUSE)
	mos_capture = configdat.bMouseCapture;
	if (((mos_port >= 1) && (mos_port <= 2) && (configdat.nMousePort == 3)) ||
		 ((configdat.nMousePort >= 1) && (configdat.nMousePort <= 2) &&
		 (mos_port == 3))) {
		mos_port = configdat.nMousePort;
		system_reset();
	}
	else {
		mos_port = configdat.nMousePort;
	}
	uMidBtnMode = configdat.uMidBtnMode;
#endif

	/* Ports�Z�N�V���� */
#if defined(RSC)
	rs_use = configdat.bCommPortEnable;
	rs_baudrate_v2 = configdat.uCommPortBps;
	nCommPortNo = configdat.nCommPortNo;
#endif
#if defined(MIDI)
	strncpy(szMidiDevice, configdat.szMidiDevice, sizeof(szMidiDevice));
	nMidiDelay = configdat.nMidiDelay;
	bMidiDelayMode = configdat.bMidiDelayMode;
#endif

	/* Printer�Z�N�V���� */
#if defined(LPRINT)
	if ((lp_use != configdat.uPrinterEnable) ||
		((configdat.uPrinterEnable == LP_LOG) &&
		(strcmp(lp_fname, configdat.szLprLogPath) != 0))) {
		/* ���[�h�؂�ւ����̃t�@�C���N���[�Y/�I�[�v�������������ōs�� */
		lp_use = configdat.uPrinterEnable;
		switch (lp_use) {
			case LP_EMULATION:
				lp_setfile(LP_TEMPFILENAME);
				break;
			case LP_LOG:
				if (configdat.szLprLogPath[0] == '\0') {
					lp_setfile(NULL);
				}
				else {
					lp_setfile(configdat.szLprLogPath);
					if (lp_fileh == -1) {
						LoadString(hAppInstance, IDS_LPRLOGERROR,
							tmp, sizeof(tmp));
						_snprintf(buffer, sizeof(buffer), tmp,
							configdat.szLprLogPath);
						MessageBox(hMainWnd, buffer, "XM7",
							MB_ICONEXCLAMATION | MB_OK);
					}
				}
				break;
			case LP_JASTSOUND:
			case LP_DISABLE:
				lp_setfile(NULL);
				break;
			default:
				ASSERT(FALSE);
		}
	}

	lpr_use_os_font = configdat.bLprUseOsFont;
	lpr_output_kanji = configdat.bLprOutputKanji;
#endif

	/* Misc�Z�N�V���� */
	if (bPopupSwnd != configdat.bPopupSwnd) {
		for (i=0; i<SWND_MAXNUM; i++) {
			if (hSubWnd[i]) {
				DestroyWindow(hSubWnd[i]);
				hSubWnd[i] = NULL;
			}
		}
	}
	bPopupSwnd = configdat.bPopupSwnd;
#if XM7_VER == 1
	pcg_flag = configdat.bPcgFlag;
	motoron_lowspeed = configdat.bMotorOnLowSpeed;
#endif
#if XM7_VER == 3
	submode_fix = configdat.bSubModeFix;
#endif
	bOFNCentering = configdat.bOFNCentering;
	magus_patch = configdat.bMagusPatch;
	rom_ram_write = configdat.bRomRamWrite;
	fdc_enable = configdat.bFdcEnable;

	/* Unofficial�Z�N�V���� */
#if XM7_VER >= 3
	bGravestone = configdat.bGravestone;
	dsp_400linetiming = configdat.b400LineTiming;
#endif
	extdet_disable = configdat.bExtDetDisable;

	/* �X�e�[�^�X�o�[�̃T�C�Y���� */
	if (hStatusBar) {
		if (bHideStatus) {
			/* ���� */
			ShowWindow(hStatusBar, SW_HIDE);
		}
		else {
			/* �\�� */
			ShowWindow(hStatusBar, SW_SHOW);
		}

		/* �t���[���E�C���h�E�̃T�C�Y��␳ */
		GetClientRect(hMainWnd, &rect);
		PostMessage(hMainWnd, WM_SIZE, 0, MAKELPARAM(rect.right, rect.bottom));
	}

	/* �\�����e���X�V */
	if (hMainWnd) {
		OnSize(hMainWnd, 640, 400);
	}
	InvalidateRect(hDrawWnd, NULL, FALSE);
	display_notify();
}

/*
 *	�ݒ�f�[�^�擾
 */
void FASTCALL GetCfg(void)
{
	/* General�Z�N�V���� */
	configdat.fm7_ver = fm7_ver;
	configdat.boot_mode = boot_mode;
	configdat.cycle_steal = cycle_steal_default;
	configdat.subclock_mode = subclock_mode;
	configdat.main_speed = main_speed / 10;
	configdat.mmr_speed = mmr_speed / 10;
#if XM7_VER >= 3
	configdat.fmmr_speed = fmmr_speed / 10;
#endif
	configdat.sub_speed = sub_speed / 10;
	configdat.bTapeFull = bTapeFullSpeed;
#if !defined(DISABLE_FULLSPEED)
	configdat.bCPUFull = bFullSpeed;
	configdat.bSpeedAdjust = bAutoSpeedAdjust;
#endif
	configdat.bTapeMode = bTapeModeType;
#if XM7_VER == 1
	configdat.fm_subtype = fm_subtype;
	configdat.lowspeed_mode = lowspeed_mode;
	configdat.main_speed_low = main_speed_low / 10;
	configdat.sub_speed_low = sub_speed_low / 10;
#if defined(JSUB)
	configdat.jsub_speed = jsub_speed / 10;
#endif
#endif

	/* Screen�Z�N�V���� */
	configdat.bFullScreen = bFullScreen;
	configdat.bHideStatus = bHideStatus;

	/* Option�Z�N�V���� */
	configdat.bOPNEnable = opn_enable;
	configdat.bWHGEnable = whg_enable;
	configdat.bTHGEnable = thg_enable;
#if XM7_VER >= 2
	configdat.bDigitizeEnable = digitize_enable;
#if XM7_VER == 2
	if (jcard_available) {
		configdat.bJCardEnable = jcard_enable;
	}
	else {
		configdat.bJCardEnable = FALSE;
	}
#else
	configdat.bJCardEnable = jcard_enable;
#endif
#endif
#if XM7_VER >= 3
	configdat.bExtRAMEnable = mmr_extram;
#endif
#if XM7_VER == 1
#if defined(L4CARD)
	if (detect_400linecard) {
		configdat.b400LineCardEnable = enable_400linecard;
	}
	else {
		configdat.b400LineCardEnable = FALSE;
	}
#endif
#if defined(JSUB)
	if (jsub_available) {
		configdat.bJSubEnable = jsub_enable;
	}
	else {
		configdat.bJSubEnable = FALSE;
	}
#endif
#if defined(BUBBLE)
	configdat.bBubbleEnable = bmc_enable;
#endif
	configdat.uBankSelectEnable = banksel_en;
#endif
}

/*
 *	����@��Đݒ�
 */
void FASTCALL SetMachineVersion(void)
{
	configdat.fm7_ver = fm7_ver;
#if XM7_VER == 1
	configdat.fm_subtype = fm_subtype;
#endif
}

/*-[ �w���v�T�|�[�g ]-------------------------------------------------------*/

/*
 *	�T�|�[�gID���X�g
 *	(��ɗ�����̂قǗD��)
 */
static const UINT PageHelpList[] = {
#if XM7_VER >= 3
	IDC_GP_FM7,
	IDC_GP_FM77AV,
	IDC_GP_AV40EX,
#elif XM7_VER == 1
	IDC_GP_FM8,
	IDC_GP_FM7,
	IDC_GP_FM77,
	IDC_GP_HIGHSPEED,
#else
	IDC_GP_MACHINEG,
#endif
	IDC_GP_HIGHSPEED,
	IDC_GP_MIDSPEED,
	IDC_GP_LOWSPEED,
	IDC_GP_TAPESPEED,
	IDC_GP_TAPESPEEDMODE,
#if !defined(DISABLE_FULLSPEED)
	IDC_GP_FULLSPEED,
	IDC_GP_AUTOSPEEDADJUST,
#endif
#if defined(FDDSND)
	IDC_GP_FDDWAIT,
#endif
	IDC_GP_CPUDEFAULT,
	IDC_GP_SPEEDG,
	IDC_SP_HQMODE,
	IDC_SP_RATEG,
	IDC_SP_BUFFERG,
	IDC_SP_STEREO,
	IDC_SP_TAPEMON,
#if defined(FDDSND)
	IDC_SP_FDDSOUND,
#endif
#if defined(ROMEO)
	IDC_SP_ROMEO,
#endif
	IDC_SP_BEEPG,
	IDC_VP_DEFAULT,
	IDC_VP_SEPARATIONG,
	IDC_VP_VOLUMEG,
	IDC_KP_LIST,
	IDC_KP_101B,
	IDC_KP_106B,
	IDC_KP_98B,
	IDC_KP_USEARROWFOR10,
	IDC_KP_ARROW8DIR,
	IDC_KP_KBDREAL,
	IDC_JP_PORTG,
	IDC_JP_TYPEG,
	IDC_JP_RAPIDG,
	IDC_JP_CODEG,
#if defined(MIDI)
	IDC_POP_MIDIDLY,
	IDC_POP_MIDIDLYSB,
	IDC_POP_MIDIG,
#endif
#if defined(RSC)
	IDC_POP_COMENABLE,
	IDC_POP_COMBPSC,
	IDC_POP_COMG,
#endif
#if defined(LPRINT)
	IDC_LPP_LPREMUENABLE,
	IDC_LPP_LPROSFNT,
	IDC_LPP_LPRKANJI,
	IDC_LPP_LPRLOGENABLE,
	IDC_LPP_LPRLOGPATH,
	IDC_LPP_LPRLOGPATHNAME,
	IDC_LPP_LPRLOGDIALOG,
#if defined(JASTSOUND)
	IDC_LPP_LPRJASTSOUNDENABLE,
#endif
	IDC_LPP_LPRDISABLE,
	IDC_LPP_LPRG,
#endif
	IDC_SCP_MODEC,
	IDC_SCP_24K,
	IDC_SCP_24KFS,
	IDC_SCP_DOUBLESIZE,
	IDC_SCP_CAPTIONB,
	IDC_SCP_TRUECOLOR,
	IDC_SCP_RASTERRENDER,
	IDC_SCP_PSEUDO400LINE,
	IDC_SCP_MODEG,
	IDC_OP_OPNB,
	IDC_OP_WHGB,
	IDC_OP_THGB,
	IDC_OP_DIGITIZEG,
#if XM7_VER == 1
#if defined(L4CARD)
	IDC_OP_400LINEG,
#endif
#if defined(JSUB)
	IDC_OP_JSUBG,
#endif
#if defined(BUBBLE)
	IDC_OP_BMCG,
#endif
	IDC_SCP_GREENMONITOR,
	IDC_AP_BANKSELB,
	IDC_AP_MOTORON_LOWSPEED,
#endif
#if XM7_VER == 2
	IDC_SCP_TTLMONITOR,
#endif
#if XM7_VER >= 2
	IDC_OP_JCARDG,
#endif
#if ((XM7_VER >= 3) || defined(FMTV151))
	IDC_OP_RAMG,
#endif
#if defined(MOUSE)
	IDC_OP_MOUSEEM,
	IDC_OP_MOUSESW,
	IDC_OP_MOUSEG,
#endif
	IDC_AP_MAGUSPATCH,
	IDC_AP_ROMRAMWRITE,
	IDC_AP_OFNCENTERING,
	IDC_AP_POPUPSWND,
	IDC_AP_FDCDISABLE,
	0
};

/*
 *	�y�[�W�w���v
 */
static void FASTCALL PageHelp(HWND hDlg, UINT uID)
{
	POINT point;
	RECT rect;
	HWND hWnd;
	int i;
	char string[256];
	UINT uHelpID;

	ASSERT(hDlg);

	/* �|�C���g�쐬 */
	GetCursorPos(&point);

	/* �w���v���X�g�ɍڂ��Ă���ID����� */
	for (i=0; ;i++) {
		/* �I���`�F�b�N */
		if (PageHelpList[i] == 0) {
			break;
		}

		/* �E�C���h�E�n���h���擾 */
		hWnd = GetDlgItem(hDlg, PageHelpList[i]);
		if (!hWnd) {
			continue;
		}
		if (!IsWindowVisible(hWnd)) {
			continue;
		}

		/* ��`�𓾂āAPtInRect�Ń`�F�b�N���� */
		GetWindowRect(hWnd, &rect);
		if (!PtInRect(&rect, point)) {
			continue;
		}

		/* �L���b�V���`�F�b�N */
		if (PageHelpList[i] == uPropertyHelp) {
			return;
		}
		uPropertyHelp = PageHelpList[i];

		/* �ꕔ���\�[�XID�̓���ւ� */
		uHelpID = uPropertyHelp;
#if defined(MOUSE)
		if (uPropertyHelp == IDC_OP_MOUSESWC) {
			uHelpID = IDC_OP_MOUSESW;
		}
#endif

		/* �����񃊃\�[�X�����[�h�A�ݒ� */
		string[0] = '\0';
		LoadString(hAppInstance, uHelpID, string, sizeof(string));
		hWnd = GetDlgItem(hDlg, uID);
		if (hWnd) {
			SetWindowText(hWnd, string);
		}

		/* �ݒ�I�� */
		return;
	}

	/* �w���v���X�g�͈͊O�̋�`�B������Ȃ� */
	if (uPropertyHelp == 0) {
		return;
	}
	uPropertyHelp = 0;

	string[0] = '\0';
	hWnd = GetDlgItem(hDlg, uID);
	if (hWnd) {
		SetWindowText(hWnd, string);
	}
}

/*-[ �S�ʃy�[�W ]-----------------------------------------------------------*/

/*
 *	�S�ʃy�[�W
 *	�_�C�A���O������
 */
static void FASTCALL GeneralPageInit(HWND hDlg)
{
	HWND hWnd;
	char string[128];

	ASSERT(hDlg);

	/* �V�[�g������ */
	SheetInit(hDlg);

	/* ����@�� */
#if XM7_VER >= 2
	switch (propdat.fm7_ver) {
		case 1:
			CheckDlgButton(hDlg, IDC_GP_FM7, BST_CHECKED);
			break;
		case 2:
			CheckDlgButton(hDlg, IDC_GP_FM77AV, BST_CHECKED);
			break;
#if XM7_VER >= 3
		case 3:
			CheckDlgButton(hDlg, IDC_GP_AV40EX, BST_CHECKED);
			break;
#endif
	}
#else
	if (propdat.fm_subtype == FMSUB_FM8) {
		CheckDlgButton(hDlg, IDC_GP_FM8, BST_CHECKED);
	}
	else if (propdat.fm_subtype == FMSUB_FM7) {
		CheckDlgButton(hDlg, IDC_GP_FM7, BST_CHECKED);
	}
	else {
		CheckDlgButton(hDlg, IDC_GP_FM77, BST_CHECKED);
	}

	hWnd = GetDlgItem(hDlg, IDC_GP_FM8);
	EnableWindow(hWnd, available_fm8roms);
	hWnd = GetDlgItem(hDlg, IDC_GP_FM7);
	EnableWindow(hWnd, available_fm7roms);
	hWnd = GetDlgItem(hDlg, IDC_GP_FM77);
	EnableWindow(hWnd, available_fm7roms);

	/* ���C��CPU����N���b�N */
	if (propdat.lowspeed_mode) {
		CheckDlgButton(hDlg, IDC_GP_MAIN1MHZ, BST_CHECKED);
	}
	else {
		CheckDlgButton(hDlg, IDC_GP_MAIN2MHZ, BST_CHECKED);
	}
#endif

	/* ���샂�[�h */
	if (propdat.cycle_steal) {
		CheckDlgButton(hDlg, IDC_GP_HIGHSPEED, BST_CHECKED);
	}
	else if (!propdat.subclock_mode) {
		CheckDlgButton(hDlg, IDC_GP_MIDSPEED, BST_CHECKED);
	}
	else {
		CheckDlgButton(hDlg, IDC_GP_LOWSPEED, BST_CHECKED);
	}

	/* CPU�I�� */
	hWnd = GetDlgItem(hDlg, IDC_GP_CPUCOMBO);
	ASSERT(hWnd);
	string[0] = '\0';
	(void)ComboBox_ResetContent(hWnd);
	LoadString(hAppInstance, IDS_GP_MAINCPU, string, sizeof(string));
	(void)ComboBox_AddString(hWnd, string);
#if XM7_VER >= 2
	LoadString(hAppInstance, IDS_GP_MAINMMR, string, sizeof(string));
	(void)ComboBox_AddString(hWnd, string);
#if XM7_VER >= 3
	LoadString(hAppInstance, IDS_GP_FASTMMR, string, sizeof(string));
	(void)ComboBox_AddString(hWnd, string);
#endif
	LoadString(hAppInstance, IDS_GP_SUBCPU, string, sizeof(string));
	(void)ComboBox_AddString(hWnd, string);
#else
	LoadString(hAppInstance, IDS_GP_MAINCPU_LOW, string, sizeof(string));
	(void)ComboBox_AddString(hWnd, string);
	LoadString(hAppInstance, IDS_GP_MAINMMR, string, sizeof(string));
	(void)ComboBox_AddString(hWnd, string);
	LoadString(hAppInstance, IDS_GP_SUBCPU, string, sizeof(string));
	(void)ComboBox_AddString(hWnd, string);
	LoadString(hAppInstance, IDS_GP_SUBCPU_LOW, string, sizeof(string));
	(void)ComboBox_AddString(hWnd, string);
#if defined(JSUB)
	LoadString(hAppInstance, IDS_GP_JSUBCPU, string, sizeof(string));
	(void)ComboBox_AddString(hWnd, string);
#endif
#endif
	(void)ComboBox_SetCurSel(hWnd, 0);

	/* CPU���x */
	hWnd = GetDlgItem(hDlg, IDC_GP_CPUSPIN);
	ASSERT(hWnd);
	UpDown_SetRange(hWnd, 9999, 1);
	UpDown_SetPos(hWnd, propdat.main_speed);

	/* �e�[�v���[�^�t���O */
	if (propdat.bTapeFull) {
		CheckDlgButton(hDlg, IDC_GP_TAPESPEED, BST_CHECKED);
	}
	else {
		CheckDlgButton(hDlg, IDC_GP_TAPESPEED, BST_UNCHECKED);
	}

	/* �e�[�v���[�^���[�h�t���O */
	if (propdat.bTapeMode) {
		CheckDlgButton(hDlg, IDC_GP_TAPESPEEDMODE, BST_CHECKED);
	}
	else {
		CheckDlgButton(hDlg, IDC_GP_TAPESPEEDMODE, BST_UNCHECKED);
	}

#if defined(DISABLE_FULLSPEED)
	hWnd = GetDlgItem(hDlg, IDC_GP_FULLSPEED);
	EnableWindow(hWnd, FALSE);
	hWnd = GetDlgItem(hDlg, IDC_GP_AUTOSPEEDADJUST);
	EnableWindow(hWnd, FALSE);
#else
	/* �S�͋쓮�t���O */
	if (propdat.bCPUFull) {
		CheckDlgButton(hDlg, IDC_GP_FULLSPEED, BST_CHECKED);
	}
	else {
		CheckDlgButton(hDlg, IDC_GP_FULLSPEED, BST_UNCHECKED);
	}

	/* �������x�����t���O */
	if (propdat.bSpeedAdjust) {
		CheckDlgButton(hDlg, IDC_GP_AUTOSPEEDADJUST, BST_CHECKED);
	}
	else {
		CheckDlgButton(hDlg, IDC_GP_AUTOSPEEDADJUST, BST_UNCHECKED);
	}
#endif

	/* FDD�E�F�C�g�t���O */
#if defined(FDDSND)
	if (propdat.bFddWait) {
		CheckDlgButton(hDlg, IDC_GP_FDDWAIT, BST_CHECKED);
	}
	else {
		CheckDlgButton(hDlg, IDC_GP_FDDWAIT, BST_UNCHECKED);
	}
#endif
}

/*
 *	�S�ʃy�[�W
 *	�R�}���h
 */
static void FASTCALL GeneralPageCmd(HWND hDlg, WORD wID, WORD wNotifyCode, HWND hWnd)
{
	int index;
	int cycles;
	HWND hSpin;
	HWND hCombo;

	ASSERT(hDlg);

	/* ID�� */
	switch (wID) {
		/* CPU���x */
		case IDC_GP_CPUTEXT:
			if (wNotifyCode == EN_CHANGE) {
				cycles = GetDlgItemInt(hDlg, IDC_GP_CPUTEXT, 0, FALSE);
				if (cycles < 1) {
					cycles = 1;
				}
				else if (cycles > 9999) {
					cycles = 9999;
				}

				/* �l���i�[ */
				hCombo = GetDlgItem(hDlg, IDC_GP_CPUCOMBO);
				index = ComboBox_GetCurSel(hCombo);
				switch (index) {
					case 0:
						propdat.main_speed = (DWORD)cycles;
						break;
#if XM7_VER >= 2
					case 1:
						propdat.mmr_speed = (DWORD)cycles;
						break;
					case 2:
#if XM7_VER >= 3
						propdat.fmmr_speed = (DWORD)cycles;
						break;
					case 3:
#endif
						propdat.sub_speed = (DWORD)cycles;
						break;
#else
					case 1:
						propdat.main_speed_low = (DWORD)cycles;
						break;
					case 2:
						propdat.mmr_speed = (DWORD)cycles;
						break;
					case 3:
						propdat.sub_speed = (DWORD)cycles;
						break;
					case 4:
						propdat.sub_speed_low = (DWORD)cycles;
						break;
#if defined(JSUB)
					case 5:
						propdat.jsub_speed = (DWORD)cycles;
						break;
#endif
#endif
					case CB_ERR:
						/* ���ږ��I�� */
						break;
					default:
						ASSERT(FALSE);
						break;
				}
			}
			break;

		/* CPU�I���R���{�{�b�N�X */
		case IDC_GP_CPUCOMBO:
			/* �I��Ώۂ��ς������A�V�����l�����[�h */
			if (wNotifyCode == CBN_SELCHANGE) {
				index = ComboBox_GetCurSel(hWnd);
				hSpin = GetDlgItem(hDlg, IDC_GP_CPUSPIN);
				ASSERT(hSpin);
				switch (index) {
					case 0:
						UpDown_SetPos(hSpin, propdat.main_speed);
						break;
#if XM7_VER >= 2
					case 1:
						UpDown_SetPos(hSpin, propdat.mmr_speed);
						break;
					case 2:
#if XM7_VER >= 3
						UpDown_SetPos(hSpin, propdat.fmmr_speed);
						break;
					case 3:
#endif
						UpDown_SetPos(hSpin, propdat.sub_speed);
						break;
#else
					case 1:
						UpDown_SetPos(hSpin, propdat.main_speed_low);
						break;
					case 2:
						UpDown_SetPos(hSpin, propdat.mmr_speed);
						break;
					case 3:
						UpDown_SetPos(hSpin, propdat.sub_speed);
						break;
					case 4:
						UpDown_SetPos(hSpin, propdat.sub_speed_low);
						break;
#if defined(JSUB)
					case 5:
						UpDown_SetPos(hSpin, propdat.jsub_speed);
						break;
#endif
#endif
					default:
						ASSERT(FALSE);
				}
			}
			break;

		case IDC_GP_CPUDEFAULT:
			/* �f�t�H���g�l��ݒ� */
			propdat.main_speed = MAINCYCLES;
			propdat.mmr_speed = MAINCYCLES_MMR;
#if XM7_VER >= 3
			propdat.fmmr_speed = MAINCYCLES_FMMR;
#endif
			propdat.sub_speed = SUBCYCLES;
#if XM7_VER == 1
			propdat.main_speed_low = MAINCYCLES_LOW;
			propdat.sub_speed_low = SUBCYCLES_LOW;
#if defined(JSUB)
			propdat.jsub_speed = JSUBCYCLES;
#endif
#endif

			/* �V�����l�����[�h */
			hCombo = GetDlgItem(hDlg, IDC_GP_CPUCOMBO);
			index = ComboBox_GetCurSel(hCombo);
			hSpin = GetDlgItem(hDlg, IDC_GP_CPUSPIN);
			ASSERT(hSpin);
			switch (index) {
				case 0:
					UpDown_SetPos(hSpin, propdat.main_speed);
					break;
#if XM7_VER >= 2
				case 1:
					UpDown_SetPos(hSpin, propdat.mmr_speed);
					break;
				case 2:
#if XM7_VER >= 3
					UpDown_SetPos(hSpin, propdat.fmmr_speed);
					break;
				case 3:
#endif
					UpDown_SetPos(hSpin, propdat.sub_speed);
					break;
#else
				case 1:
					UpDown_SetPos(hSpin, propdat.main_speed_low);
					break;
				case 2:
					UpDown_SetPos(hSpin, propdat.mmr_speed);
					break;
				case 3:
					UpDown_SetPos(hSpin, propdat.sub_speed);
					break;
				case 4:
					UpDown_SetPos(hSpin, propdat.sub_speed_low);
					break;
#if defined(JSUB)
				case 5:
					UpDown_SetPos(hSpin, propdat.jsub_speed);
					break;
#endif
#endif
				default:
					ASSERT(FALSE);
			}
			break;
	}
}

/*
 *	�S�ʃy�[�W
 *	�����X�N���[��
 */
static void FASTCALL GeneralPageVScroll(HWND hDlg, WORD wPos, HWND hWnd)
{
	int index;
	HWND hCombo;

	/* �`�F�b�N */
	if (hWnd != GetDlgItem(hDlg, IDC_GP_CPUSPIN)) {
		return;
	}

	/* �l���i�[ */
	hCombo = GetDlgItem(hDlg, IDC_GP_CPUCOMBO);
	index = ComboBox_GetCurSel(hCombo);
	switch (index) {
		case 0:
			propdat.main_speed = (DWORD)wPos;
			break;
#if XM7_VER >= 2
		case 1:
			propdat.mmr_speed = (DWORD)wPos;
			break;
		case 2:
#if XM7_VER >= 3
			propdat.fmmr_speed = (DWORD)wPos;
			break;
		case 3:
#endif
			propdat.sub_speed = (DWORD)wPos;
			break;
#else
		case 1:
			propdat.main_speed_low = (DWORD)wPos;
			break;
		case 2:
			propdat.mmr_speed = (DWORD)wPos;
			break;
		case 3:
			propdat.sub_speed = (DWORD)wPos;
			break;
		case 4:
			propdat.sub_speed_low = (DWORD)wPos;
			break;
#if defined(JSUB)
		case 5:
			propdat.jsub_speed = (DWORD)wPos;
			break;
#endif
#endif
		default:
			ASSERT(FALSE);
			break;
	}
}

/*
 *	�S�ʃy�[�W
 *	�K�p
 */
static void FASTCALL GeneralPageApply(HWND hDlg)
{
	/* �X�e�[�g�ύX */
	uPropertyState = 2;

	/* FM-7�o�[�W���� */
#if XM7_VER >= 2
	if (IsDlgButtonChecked(hDlg, IDC_GP_FM7) == BST_CHECKED) {
		propdat.fm7_ver = 1;
	}
#if XM7_VER >= 3
	else if (IsDlgButtonChecked(hDlg, IDC_GP_FM77AV) == BST_CHECKED) {
		propdat.fm7_ver = 2;
	}
	else
	{
		propdat.fm7_ver = 3;
	}
#else
	else {
		propdat.fm7_ver = 2;
	}
#endif
#else
	if (IsDlgButtonChecked(hDlg, IDC_GP_FM8) == BST_CHECKED) {
		propdat.fm_subtype = FMSUB_FM8;
	}
	else if (IsDlgButtonChecked(hDlg, IDC_GP_FM7) == BST_CHECKED) {
		propdat.fm_subtype = FMSUB_FM7;
	}
	else {
		propdat.fm_subtype = FMSUB_FM77;
	}

	/* ���C��CPU����N���b�N */
	if (IsDlgButtonChecked(hDlg, IDC_GP_MAIN1MHZ) == BST_CHECKED) {
		propdat.lowspeed_mode = TRUE;
	}
	else {
		propdat.lowspeed_mode = FALSE;
	}
#endif

	/* �T�C�N���X�`�[�� */
	if (IsDlgButtonChecked(hDlg, IDC_GP_HIGHSPEED) == BST_CHECKED) {
		propdat.cycle_steal = TRUE;
		propdat.subclock_mode = FALSE;
	}
	else if (IsDlgButtonChecked(hDlg, IDC_GP_MIDSPEED) == BST_CHECKED) {
		propdat.cycle_steal = FALSE;
		propdat.subclock_mode = FALSE;
	}
	else {
		propdat.cycle_steal = FALSE;
		propdat.subclock_mode = TRUE;
	}

	/* �e�[�v�������[�h */
	if (IsDlgButtonChecked(hDlg, IDC_GP_TAPESPEED) == BST_CHECKED) {
		propdat.bTapeFull = TRUE;
	}
	else {
		propdat.bTapeFull = FALSE;
	}

	/* �e�[�v�������[�h����^�C�v */
	if (IsDlgButtonChecked(hDlg, IDC_GP_TAPESPEEDMODE) == BST_CHECKED) {
		propdat.bTapeMode = TRUE;
	}
	else {
		propdat.bTapeMode = FALSE;
	}

#if !defined(DISABLE_FULLSPEED)
	/* �S�͋쓮 */
	if (IsDlgButtonChecked(hDlg, IDC_GP_FULLSPEED) == BST_CHECKED) {
		propdat.bCPUFull = TRUE;
	}
	else {
		propdat.bCPUFull = FALSE;
	}

	/* �������x���� */
	if (IsDlgButtonChecked(hDlg, IDC_GP_AUTOSPEEDADJUST) == BST_CHECKED) {
		propdat.bSpeedAdjust = TRUE;
	}
	else {
		propdat.bSpeedAdjust = FALSE;
	}
#endif

	/* FDD�E�F�C�g */
#if defined(FDDSND)
	if (IsDlgButtonChecked(hDlg, IDC_GP_FDDWAIT) == BST_CHECKED) {
		propdat.bFddWait = TRUE;
	}
	else {
		propdat.bFddWait = FALSE;
	}
#endif
}

/*
 *	�S�ʃy�[�W
 *	�_�C�A���O�v���V�[�W��
 */
static BOOL CALLBACK GeneralPageProc(HWND hDlg, UINT msg,
									 WPARAM wParam, LPARAM lParam)
{
	LPNMHDR pnmh;

	switch (msg) {
		/* ������ */
		case WM_INITDIALOG:
			GeneralPageInit(hDlg);
			return TRUE;

		/* �R�}���h */
		case WM_COMMAND:
			GeneralPageCmd(hDlg, LOWORD(wParam), HIWORD(wParam), (HWND)lParam);
			return TRUE;

		/* �ʒm */
		case WM_NOTIFY:
			pnmh = (LPNMHDR)lParam;
			if (pnmh->code == PSN_APPLY) {
				GeneralPageApply(hDlg);
				return TRUE;
			}
			break;

		/* �����X�N���[�� */
		case WM_VSCROLL:
			GeneralPageVScroll(hDlg, HIWORD(wParam), (HWND)lParam);
			break;

		/* �J�[�\���ݒ� */
		case WM_SETCURSOR:
			if (HIWORD(lParam) == WM_MOUSEMOVE) {
				PageHelp(hDlg, IDC_GP_HELP);
			}
			break;
	}

	return FALSE;
}

/*-[ �T�E���h�y�[�W ]-------------------------------------------------------*/

/*
 *	�T�E���h�y�[�W
 *	�R�}���h
 */
static void FASTCALL SoundPageCmd(HWND hDlg, WORD wID, WORD wNotifyCode, HWND hWnd)
{
	int tmp;
	HWND hSpin;

	ASSERT(hDlg);
	UNUSED(hWnd);

	/* ID�� */
	switch (wID) {
		/* �T�E���h�o�b�t�@ */
		case IDC_SP_BUFEDIT:
			if (wNotifyCode == EN_CHANGE) {
				tmp = GetDlgItemInt(hDlg, IDC_SP_BUFEDIT, 0, FALSE) / 10;
				if (tmp < 4) {
					tmp = 4;
				}
				else if (tmp > 100) {
					tmp = 100;
				}

				hSpin = GetDlgItem(hDlg, IDC_SP_BUFSPIN);
				ASSERT(hSpin);
				UpDown_SetPos(hSpin, tmp);
			}
			break;

		/* BEEP���g�� */
		case IDC_SP_BEEPEDIT:
			if (wNotifyCode == EN_CHANGE) {
				tmp = GetDlgItemInt(hDlg, IDC_SP_BEEPEDIT, 0, FALSE);
				if (tmp < 100) {
					tmp = 100;
				}
				else if (tmp > 9999) {
					tmp = 9999;
				}

				hSpin = GetDlgItem(hDlg, IDC_SP_BEEPSPIN);
				ASSERT(hSpin);
				UpDown_SetPos(hSpin, tmp);
			}
			break;
		}
}

/*
 *	�T�E���h�y�[�W
 *	�����X�N���[��
 */
static void FASTCALL SoundPageVScroll(HWND hDlg, WORD wPos, HWND hWnd)
{
	HWND hBuddyWnd;
	char string[128];

	ASSERT(hDlg);
	ASSERT(hWnd);

	/* �E�C���h�E�n���h�����`�F�b�N */
	if (hWnd == GetDlgItem(hDlg, IDC_SP_BUFSPIN)) {
		/* �T�E���h�o�b�t�@ */
		/* �|�W�V��������A�o�f�B�E�C���h�E�ɒl��ݒ� */
		hBuddyWnd = GetDlgItem(hDlg, IDC_SP_BUFEDIT);
		ASSERT(hBuddyWnd);
		_snprintf(string, sizeof(string), "%d", wPos * 10);
		SetWindowText(hBuddyWnd, string);
	}
	else if (hWnd == GetDlgItem(hDlg, IDC_SP_BEEPSPIN)) {
		/* BEEP���g�� */
		/* �|�W�V��������A�o�f�B�E�C���h�E�ɒl��ݒ� */
		hBuddyWnd = GetDlgItem(hDlg, IDC_SP_BEEPEDIT);
		ASSERT(hBuddyWnd);
		_snprintf(string, sizeof(string), "%d", wPos);
		SetWindowText(hBuddyWnd, string);
	}
}

/*
 *	�T�E���h�y�[�W
 *	�_�C�A���O������
 */
static void FASTCALL SoundPageInit(HWND hDlg)
{
	HWND hWnd;
	char string[128];

	/* �V�[�g������ */
	SheetInit(hDlg);

	/* �T���v�����O���[�g */
	switch (propdat.nSampleRate) {
		case 96000:
			CheckDlgButton(hDlg, IDC_SP_96K, BST_CHECKED);
			break;
		case 88200:
			CheckDlgButton(hDlg, IDC_SP_88K, BST_CHECKED);
			break;
		case 51200:
			CheckDlgButton(hDlg, IDC_SP_51K, BST_CHECKED);
			break;
		case 48000:
			CheckDlgButton(hDlg, IDC_SP_48K, BST_CHECKED);
			break;
		case 44100:
			CheckDlgButton(hDlg, IDC_SP_44K, BST_CHECKED);
			break;
		case 25600:
			CheckDlgButton(hDlg, IDC_SP_25K, BST_CHECKED);
			break;
		case 22050:
			CheckDlgButton(hDlg, IDC_SP_22K, BST_CHECKED);
			break;
		case 0:
			CheckDlgButton(hDlg, IDC_SP_NONE, BST_CHECKED);
			break;
		default:
			ASSERT(FALSE);
			break;
	}

	/* �T�E���h�o�b�t�@ */
	hWnd = GetDlgItem(hDlg, IDC_SP_BUFSPIN);
	ASSERT(hWnd);
	UpDown_SetRange(hWnd, 100, 4);
	UpDown_SetPos(hWnd, propdat.nSoundBuffer / 10);
	SoundPageVScroll(hDlg, LOWORD(UpDown_GetPos(hWnd)), hWnd);

	/* BEEP���g�� */
	hWnd = GetDlgItem(hDlg, IDC_SP_BEEPSPIN);
	ASSERT(hWnd);
	UpDown_SetRange(hWnd, 9999, 100);
	UpDown_SetPos(hWnd, propdat.nBeepFreq);
	SoundPageVScroll(hDlg, LOWORD(UpDown_GetPos(hWnd)), hWnd);

	/* FM���i���������[�h */
	if (propdat.bInterpolation) {
		CheckDlgButton(hDlg, IDC_SP_HQMODE, BST_CHECKED);
	}
	else {
		CheckDlgButton(hDlg, IDC_SP_HQMODE, BST_UNCHECKED);
	}

	/* �o�̓��[�h */
	hWnd = GetDlgItem(hDlg, IDC_SP_STEREO);
	ASSERT(hWnd);
	string[0] = '\0';
	(void)ComboBox_ResetContent(hWnd);
	LoadString(hAppInstance, IDS_SP_MONO, string, sizeof(string));
	(void)ComboBox_AddString(hWnd, string);
	LoadString(hAppInstance, IDS_SP_STEREO, string, sizeof(string));
	(void)ComboBox_AddString(hWnd, string);
	LoadString(hAppInstance, IDS_SP_STEREO_WHGREV, string, sizeof(string));
	(void)ComboBox_AddString(hWnd, string);
	LoadString(hAppInstance, IDS_SP_STEREO_THG, string, sizeof(string));
	(void)ComboBox_AddString(hWnd, string);
	LoadString(hAppInstance, IDS_SP_STEREO_THGREV, string, sizeof(string));
	(void)ComboBox_AddString(hWnd, string);
	(void)ComboBox_SetCurSel(hWnd, propdat.uStereoOut);

	/* �e�[�v�����j�^ */
	if (propdat.bTapeMon) {
		CheckDlgButton(hDlg, IDC_SP_TAPEMON, BST_CHECKED);
	}
	else {
		CheckDlgButton(hDlg, IDC_SP_TAPEMON, BST_UNCHECKED);
	}

	/* ��݂� */
#if defined(ROMEO)
	if (propdat.bUseRomeo) {
		CheckDlgButton(hDlg, IDC_SP_ROMEO, BST_CHECKED);
	}
	else {
		CheckDlgButton(hDlg, IDC_SP_ROMEO, BST_UNCHECKED);
	}
	if (!bRomeo) {
		hWnd = GetDlgItem(hDlg, IDC_SP_ROMEO);
		EnableWindow(hWnd, FALSE);
	}
#endif

	/* FDD�V�[�N�� */
#if defined(FDDSND)
	if (propdat.bFddSound) {
		CheckDlgButton(hDlg, IDC_SP_FDDSOUND, BST_CHECKED);
	}
	else {
		CheckDlgButton(hDlg, IDC_SP_FDDSOUND, BST_UNCHECKED);
	}
#endif
}

/*
 *	�T�E���h�y�[�W
 *	�K�p
 */
static void FASTCALL SoundPageApply(HWND hDlg)
{
	HWND hWnd;
	UINT uPos;

	/* �X�e�[�g�ύX */
	uPropertyState = 2;

	/* �T���v�����O���[�g */
	if (IsDlgButtonChecked(hDlg, IDC_SP_96K) == BST_CHECKED) {
		propdat.nSampleRate = 96000;
	}
	if (IsDlgButtonChecked(hDlg, IDC_SP_88K) == BST_CHECKED) {
		propdat.nSampleRate = 88200;
	}
	if (IsDlgButtonChecked(hDlg, IDC_SP_51K) == BST_CHECKED) {
		propdat.nSampleRate = 51200;
	}
	if (IsDlgButtonChecked(hDlg, IDC_SP_48K) == BST_CHECKED) {
		propdat.nSampleRate = 48000;
	}
	if (IsDlgButtonChecked(hDlg, IDC_SP_44K) == BST_CHECKED) {
		propdat.nSampleRate = 44100;
	}
	if (IsDlgButtonChecked(hDlg, IDC_SP_25K) == BST_CHECKED) {
		propdat.nSampleRate = 25600;
	}
	if (IsDlgButtonChecked(hDlg, IDC_SP_22K) == BST_CHECKED) {
		propdat.nSampleRate = 22050;
	}
	if (IsDlgButtonChecked(hDlg, IDC_SP_NONE) == BST_CHECKED) {
		propdat.nSampleRate = 0;
	}

	/* �T�E���h�o�b�t�@ */
	hWnd = GetDlgItem(hDlg, IDC_SP_BUFSPIN);
	ASSERT(hWnd);
	uPos = LOWORD(UpDown_GetPos(hWnd));
	propdat.nSoundBuffer = uPos * 10;

	/* BEEP���g�� */
	hWnd = GetDlgItem(hDlg, IDC_SP_BEEPSPIN);
	ASSERT(hWnd);
	propdat.nBeepFreq = LOWORD(UpDown_GetPos(hWnd));

	/* FM���i���������[�h */
	if (IsDlgButtonChecked(hDlg, IDC_SP_HQMODE) == BST_CHECKED) {
		propdat.bInterpolation = TRUE;
	}
	else {
		propdat.bInterpolation = FALSE;
	}

	/* �o�̓��[�h */
	hWnd = GetDlgItem(hDlg, IDC_SP_STEREO);
	propdat.uStereoOut = ComboBox_GetCurSel(hWnd);

	/* �e�[�v�����j�^ */
	if (IsDlgButtonChecked(hDlg, IDC_SP_TAPEMON) == BST_CHECKED) {
		propdat.bTapeMon = TRUE;
	}
	else {
		propdat.bTapeMon = FALSE;
	}

	/* ��݂� */
#if defined(ROMEO)
	if ((IsDlgButtonChecked(hDlg, IDC_SP_ROMEO) == BST_CHECKED) && bRomeo) {
		propdat.bUseRomeo = TRUE;
	}
	else {
		propdat.bUseRomeo = FALSE;
	}
#endif

	/* FDD�V�[�N�� */
#if defined(FDDSND)
	if (IsDlgButtonChecked(hDlg, IDC_SP_FDDSOUND) == BST_CHECKED) {
		propdat.bFddSound = TRUE;
	}
	else {
		propdat.bFddSound = FALSE;
	}
#endif
}

/*
 *	�T�E���h�y�[�W
 *	�_�C�A���O�v���V�[�W��
 */
static BOOL CALLBACK SoundPageProc(HWND hDlg, UINT msg,
									 WPARAM wParam, LPARAM lParam)
{
	LPNMHDR pnmh;

	switch (msg) {
		/* ������ */
		case WM_INITDIALOG:
			SoundPageInit(hDlg);
			return TRUE;

		/* �R�}���h */
		case WM_COMMAND:
			SoundPageCmd(hDlg, LOWORD(wParam), HIWORD(wParam), (HWND)lParam);
			return TRUE;

		/* �ʒm */
		case WM_NOTIFY:
			pnmh = (LPNMHDR)lParam;
			if (pnmh->code == PSN_APPLY) {
				SoundPageApply(hDlg);
				return TRUE;
			}
			break;

		/* �����X�N���[�� */
		case WM_VSCROLL:
			SoundPageVScroll(hDlg, HIWORD(wParam), (HWND)lParam);
			break;

		/* �J�[�\���ݒ� */
		case WM_SETCURSOR:
			if (HIWORD(lParam) == WM_MOUSEMOVE) {
				PageHelp(hDlg, IDC_SP_HELP);
			}
			break;
	}

	return FALSE;
}

/*-[ ���ʒ����y�[�W ]-------------------------------------------------------*/

/*
 *	���ʒ����y�[�W
 *	�X���C�_�[�A�b�v�f�[�g
 */
static void FASTCALL UpdateVolumeSlider(HWND hDlg)
{
	UINT uSp;
	int nFM;
	int nPSG;
	int nBeep;
	int nCMT;
	int nWav;
	char string[128];

	/* �e�G�f�B�b�g�{�b�N�X�ɃX���C�_�[�̒l��ݒ� */
	uSp = SendDlgItemMessage(hDlg, IDC_VP_SEPARATION, TBM_GETPOS, 0, 0);
	_snprintf(string, sizeof(string), "%u", uSp);
	Edit_SetText(GetDlgItem(hDlg, IDC_VP_SEPARATIONTEXT), string);
	nFM = SendDlgItemMessage(hDlg, IDC_VP_FMVOLUME, TBM_GETPOS, 0, 0);
	_snprintf(string, sizeof(string), "%d", nFM);
	Edit_SetText(GetDlgItem(hDlg, IDC_VP_FMVOLUMETEXT), string);
	nPSG = SendDlgItemMessage(hDlg, IDC_VP_PSGVOLUME, TBM_GETPOS, 0, 0);
	_snprintf(string, sizeof(string), "%d", nPSG);
	Edit_SetText(GetDlgItem(hDlg, IDC_VP_PSGVOLUMETEXT), string);
	nBeep = SendDlgItemMessage(hDlg, IDC_VP_BEEPVOLUME, TBM_GETPOS, 0, 0);
	_snprintf(string, sizeof(string), "%d", nBeep);
	Edit_SetText(GetDlgItem(hDlg, IDC_VP_BEEPVOLUMETEXT), string);
	nCMT = SendDlgItemMessage(hDlg, IDC_VP_CMTVOLUME, TBM_GETPOS, 0, 0);
	_snprintf(string, sizeof(string), "%d", nCMT);
	Edit_SetText(GetDlgItem(hDlg, IDC_VP_CMTVOLUMETEXT), string);
#if defined(FDDSND)
	nWav = SendDlgItemMessage(hDlg, IDC_VP_WAVEVOLUME, TBM_GETPOS, 0, 0);
	_snprintf(string, sizeof(string), "%d", nWav);
	Edit_SetText(GetDlgItem(hDlg, IDC_VP_WAVEVOLUMETEXT), string);
#endif

	/* �{�����[���ݒ� */
	SetSoundVolume2(uSp, nFM, nPSG, nBeep, nCMT, nWav);
}

/*
 *	���ʒ����y�[�W
 *	�_�C�A���O������
 */
static void FASTCALL VolumePageInit(HWND hDlg)
{
	ASSERT(hDlg);

	/* �V�[�g������ */
	SheetInit(hDlg);

	/* �`�����l���Z�p���[�V���� */
	SendDlgItemMessage(hDlg, IDC_VP_SEPARATION, TBM_SETLINESIZE, 0, 1);
	SendDlgItemMessage(hDlg, IDC_VP_SEPARATION, TBM_SETPAGESIZE, 0, 4);
	SendDlgItemMessage(hDlg, IDC_VP_SEPARATION, TBM_SETRANGE, TRUE,
		MAKELONG(0, 16));
	SendDlgItemMessage(hDlg, IDC_VP_SEPARATION, TBM_SETPOS, TRUE,
		propdat.uChSeparation);

	/* FM���� */
	SendDlgItemMessage(hDlg, IDC_VP_FMVOLUME, TBM_SETLINESIZE, 0, 1);
	SendDlgItemMessage(hDlg, IDC_VP_FMVOLUME, TBM_SETPAGESIZE, 0, 6);
	SendDlgItemMessage(hDlg, IDC_VP_FMVOLUME, TBM_SETRANGE, TRUE,
		MAKELONG(-96, 20));
	SendDlgItemMessage(hDlg, IDC_VP_FMVOLUME, TBM_SETPOS, TRUE,
		propdat.nFMVolume);

	/* PSG */
	SendDlgItemMessage(hDlg, IDC_VP_PSGVOLUME, TBM_SETLINESIZE, 0, 1);
	SendDlgItemMessage(hDlg, IDC_VP_PSGVOLUME, TBM_SETPAGESIZE, 0, 6);
	SendDlgItemMessage(hDlg, IDC_VP_PSGVOLUME, TBM_SETRANGE, TRUE,
		MAKELONG(-96, 20));
	SendDlgItemMessage(hDlg, IDC_VP_PSGVOLUME, TBM_SETPOS, TRUE,
		propdat.nPSGVolume);

	/* BEEP */
	SendDlgItemMessage(hDlg, IDC_VP_BEEPVOLUME, TBM_SETLINESIZE, 0, 1);
	SendDlgItemMessage(hDlg, IDC_VP_BEEPVOLUME, TBM_SETPAGESIZE, 0, 6);
	SendDlgItemMessage(hDlg, IDC_VP_BEEPVOLUME, TBM_SETRANGE, TRUE,
		MAKELONG(-96, 0));
	SendDlgItemMessage(hDlg, IDC_VP_BEEPVOLUME, TBM_SETPOS, TRUE,
		propdat.nBeepVolume);

	/* CMT�����j�^ */
	SendDlgItemMessage(hDlg, IDC_VP_CMTVOLUME, TBM_SETLINESIZE, 0, 1);
	SendDlgItemMessage(hDlg, IDC_VP_CMTVOLUME, TBM_SETPAGESIZE, 0, 6);
	SendDlgItemMessage(hDlg, IDC_VP_CMTVOLUME, TBM_SETRANGE, TRUE,
		MAKELONG(-96, 0));
	SendDlgItemMessage(hDlg, IDC_VP_CMTVOLUME, TBM_SETPOS, TRUE,
		propdat.nCMTVolume);

	/* �e����ʉ� */
#if defined(FDDSND)
	SendDlgItemMessage(hDlg, IDC_VP_WAVEVOLUME, TBM_SETLINESIZE, 0, 1);
	SendDlgItemMessage(hDlg, IDC_VP_WAVEVOLUME, TBM_SETPAGESIZE, 0, 6);
	SendDlgItemMessage(hDlg, IDC_VP_WAVEVOLUME, TBM_SETRANGE, TRUE,
		MAKELONG(-96, 0));
	SendDlgItemMessage(hDlg, IDC_VP_WAVEVOLUME, TBM_SETPOS, TRUE,
		propdat.nWaveVolume);
#endif

	/* �X���C�_�[�A�b�v�f�[�g */
	UpdateVolumeSlider(hDlg);
}

/*
 *	���ʒ����y�[�W
 *	�K�p
 */
static void FASTCALL VolumePageApply(HWND hDlg)
{
	/* �X�e�[�g�ύX */
	uPropertyState = 2;

	/* �`�����l���Z�p���[�V���� */
	propdat.uChSeparation = SendDlgItemMessage(hDlg, IDC_VP_SEPARATION,
		TBM_GETPOS, 0, 0);

	/* FM���� */
	propdat.nFMVolume = SendDlgItemMessage(hDlg, IDC_VP_FMVOLUME,
		TBM_GETPOS, 0, 0);

	/* PSG */
	propdat.nPSGVolume = SendDlgItemMessage(hDlg, IDC_VP_PSGVOLUME,
		TBM_GETPOS, 0, 0);

	/* BEEP */
	propdat.nBeepVolume = SendDlgItemMessage(hDlg, IDC_VP_BEEPVOLUME,
		TBM_GETPOS, 0, 0);

	/* CMT�����j�^ */
	propdat.nCMTVolume = SendDlgItemMessage(hDlg, IDC_VP_CMTVOLUME,
		TBM_GETPOS, 0, 0);

	/* �e����ʉ� */
#if defined(FDDSND)
	propdat.nWaveVolume = SendDlgItemMessage(hDlg, IDC_VP_WAVEVOLUME,
		TBM_GETPOS, 0, 0);
#endif
}

/*
 *	���ʒ����y�[�W
 *	�_�C�A���O�v���V�[�W��
 */
static BOOL CALLBACK VolumePageProc(HWND hDlg, UINT msg,
									WPARAM wParam, LPARAM lParam)
{
	LPNMHDR pnmh;

	switch (msg) {
		/* �_�C�A���O������ */
		case WM_INITDIALOG:
			VolumePageInit(hDlg);
			return TRUE;

		/* �R�}���h */
		case WM_COMMAND:
			if (LOWORD(wParam) == IDC_VP_DEFAULT) {
				SendDlgItemMessage(hDlg, IDC_VP_SEPARATION, TBM_SETPOS,
					TRUE, CHSEPARATION_DEFAULT);
				SendDlgItemMessage(hDlg, IDC_VP_FMVOLUME, TBM_SETPOS,
					TRUE, FMVOLUME_DEFAULT);
				SendDlgItemMessage(hDlg, IDC_VP_PSGVOLUME, TBM_SETPOS,
					TRUE, PSGVOLUME_DEFAULT);
				SendDlgItemMessage(hDlg, IDC_VP_BEEPVOLUME, TBM_SETPOS,
					TRUE, BEEPVOLUME_DEFAULT);
				SendDlgItemMessage(hDlg, IDC_VP_CMTVOLUME, TBM_SETPOS,
					TRUE, CMTVOLUME_DEFAULT);
#if defined(FDDSND)
				SendDlgItemMessage(hDlg, IDC_VP_WAVEVOLUME, TBM_SETPOS,
					TRUE, WAVEVOLUME_DEFAULT);
#endif
				UpdateVolumeSlider(hDlg);
				return TRUE;
			}
			break;

		/* �ʒm */
		case WM_NOTIFY:
			pnmh = (LPNMHDR)lParam;
			if (pnmh->code == PSN_APPLY) {
				VolumePageApply(hDlg);
				return TRUE;
			}
			if (pnmh->code == PSN_QUERYCANCEL) {
				SetSoundVolume();
				return TRUE;
			}
			break;

		/* �c�X�N���[�� */
		case WM_HSCROLL:
			UpdateVolumeSlider(hDlg);
			return FALSE;

		/* �J�[�\���ݒ� */
		case WM_SETCURSOR:
			if (HIWORD(lParam) == WM_MOUSEMOVE) {
				PageHelp(hDlg, IDC_VP_HELP);
			}
			break;
		}

	/* ����ȊO��FALSE */
	return FALSE;
}

/*-[ �L�[���̓_�C�A���O ]---------------------------------------------------*/

/*
 *	�L�[�{�[�h�y�[�W
 *	DirectInput �L�[�R�[�h�e�[�u��
 */
static char *KbdPageDirectInput[] = {
	NULL,					/* 0x00 */
	"DIK_ESCAPE",			/* 0x01 */
	"DIK_1",				/* 0x02 */
	"DIK_2",				/* 0x03 */
	"DIK_3",				/* 0x04 */
	"DIK_4",				/* 0x05 */
	"DIK_5",				/* 0x06 */
	"DIK_6",				/* 0x07 */
	"DIK_7",				/* 0x08 */
	"DIK_8",				/* 0x09 */
	"DIK_9",				/* 0x0A */
	"DIK_0",				/* 0x0B */
	"DIK_MINUS",			/* 0x0C */
	"DIK_EQUALS",			/* 0x0D */
	"DIK_BACK",				/* 0x0E */
	"DIK_TAB",				/* 0x0F */
	"DIK_Q",				/* 0x10 */
	"DIK_W",				/* 0x11 */
	"DIK_E",				/* 0x12 */
	"DIK_R",				/* 0x13 */
	"DIK_T",				/* 0x14 */
	"DIK_Y",				/* 0x15 */
	"DIK_U",				/* 0x16 */
	"DIK_I",				/* 0x17 */
	"DIK_O",				/* 0x18 */
	"DIK_P",				/* 0x19 */
	"DIK_LBRACKET",			/* 0x1A */
	"DIK_RBRACKET",			/* 0x1B */
	"DIK_RETURN",			/* 0x1C */
	"DIK_LCONTROL",			/* 0x1D */
	"DIK_A",				/* 0x1E */
	"DIK_S",				/* 0x1F */
	"DIK_D",				/* 0x20 */
	"DIK_F",				/* 0x21 */
	"DIK_G",				/* 0x22 */
	"DIK_H",				/* 0x23 */
	"DIK_J",				/* 0x24 */
	"DIK_K",				/* 0x25 */
	"DIK_L",				/* 0x26 */
	"DIK_SEMICOLON",		/* 0x27 */
	"DIK_APOSTROPHE",		/* 0x28 */
	"DIK_GRAVE",			/* 0x29 */
	"DIK_LSHIFT",			/* 0x2A */
	"DIK_BACKSLASH",		/* 0x2B */
	"DIK_Z",				/* 0x2C */
	"DIK_X",				/* 0x2D */
	"DIK_C",				/* 0x2E */
	"DIK_V",				/* 0x2F */
	"DIK_B",				/* 0x30 */
	"DIK_N",				/* 0x31 */
	"DIK_M",				/* 0x32 */
	"DIK_COMMA",			/* 0x33 */
	"DIK_PERIOD",			/* 0x34 */
	"DIK_SLASH",			/* 0x35 */
	"DIK_RSHIFT",			/* 0x36 */
	"DIK_MULTIPLY",			/* 0x37 */
	"DIK_LMENU",			/* 0x38 */
	"DIK_SPACE",			/* 0x39 */
	"DIK_CAPITAL",			/* 0x3A */
	"DIK_F1",				/* 0x3B */
	"DIK_F2",				/* 0x3C */
	"DIK_F3",				/* 0x3D */
	"DIK_F4",				/* 0x3E */
	"DIK_F5",				/* 0x3F */
	"DIK_F6",				/* 0x40 */
	"DIK_F7",				/* 0x41 */
	"DIK_F8",				/* 0x42 */
	"DIK_F9",				/* 0x43 */
	"DIK_F10",				/* 0x44 */
	"DIK_NUMLOCK",			/* 0x45 */
	"DIK_SCROLL",			/* 0x46 */
	"DIK_NUMPAD7",			/* 0x47 */
	"DIK_NUMPAD8",			/* 0x48 */
	"DIK_NUMPAD9",			/* 0x49 */
	"DIK_SUBTRACT",			/* 0x4A */
	"DIK_NUMPAD4",			/* 0x4B */
	"DIK_NUMPAD5",			/* 0x4C */
	"DIK_NUMPAD6",			/* 0x4D */
	"DIK_ADD",				/* 0x4E */
	"DIK_NUMPAD1",			/* 0x4F */
	"DIK_NUMPAD2",			/* 0x50 */
	"DIK_NUMPAD3",			/* 0x51 */
	"DIK_NUMPAD0",			/* 0x52 */
	"DIK_DECIMAL",			/* 0x53 */
	NULL,					/* 0x54 */
	NULL,					/* 0x55 */
	"DIK_OEM_102",			/* 0x56 */
	"DIK_F11",				/* 0x57 */
	"DIK_F12",				/* 0x58 */
	NULL,					/* 0x59 */
	NULL,					/* 0x5A */
	NULL,					/* 0x5B */
	NULL,					/* 0x5C */
	NULL,					/* 0x5D */
	NULL,					/* 0x5E */
	NULL,					/* 0x5F */
	NULL,					/* 0x60 */
	NULL,					/* 0x61 */
	NULL,					/* 0x62 */
	NULL,					/* 0x63 */
	"DIK_F13",				/* 0x64 */
	"DIK_F14",				/* 0x65 */
	"DIK_F15",				/* 0x66 */
	NULL,					/* 0x67 */
	NULL,					/* 0x68 */
	NULL,					/* 0x69 */
	NULL,					/* 0x6A */
	NULL,					/* 0x6B */
	NULL,					/* 0x6C */
	NULL,					/* 0x6D */
	NULL,					/* 0x6E */
	NULL,					/* 0x6F */
	"DIK_KANA",				/* 0x70 */
	NULL,					/* 0x71 */
	NULL,					/* 0x72 */
	"DIK_ABNT_C1",			/* 0x73 */
	NULL,					/* 0x74 */
	NULL,					/* 0x75 */
	NULL,					/* 0x76 */
	NULL,					/* 0x77 */
	NULL,					/* 0x78 */
	"DIK_CONVERT",			/* 0x79 */
	NULL,					/* 0x7A */
	"DIK_NOCONVERT",		/* 0x7B */
	NULL,					/* 0x7C */
	"DIK_YEN",				/* 0x7D */
	"DIK_ABNT_C2",			/* 0x7E */
	NULL,					/* 0x7F */
	NULL,					/* 0x80 */
	NULL,					/* 0x81 */
	NULL,					/* 0x82 */
	NULL,					/* 0x83 */
	NULL,					/* 0x84 */
	NULL,					/* 0x85 */
	NULL,					/* 0x86 */
	NULL,					/* 0x87 */
	NULL,					/* 0x88 */
	NULL,					/* 0x89 */
	NULL,					/* 0x8A */
	NULL,					/* 0x8B */
	NULL,					/* 0x8C */
	"DIK_NUMPADEQUALS",		/* 0x8D */
	NULL,					/* 0x8E */
	NULL,					/* 0x8F */
	"DIK_PREVTRACK",		/* 0x90 */
	"DIK_AT",				/* 0x91 */
	"DIK_COLON",			/* 0x92 */
	"DIK_UNDERLINE",		/* 0x93 */
	"DIK_KANJI",			/* 0x94 */
	"DIK_STOP",				/* 0x95 */
	"DIK_AX",				/* 0x96 */
	"DIK_UNLABELED",		/* 0x97 */
	NULL,					/* 0x98 */
	"DIK_NEXTTRACK",		/* 0x99 */
	NULL,					/* 0x9A */
	NULL,					/* 0x9B */
	"DIK_NUMPADENTER",		/* 0x9C */
	"DIK_RCONTROL",			/* 0x9D */
	NULL,					/* 0x9E */
	NULL,					/* 0x9F */
	"DIK_MUTE",				/* 0xA0 */
	"DIK_CALCULATOR",		/* 0xA1 */
	"DIK_PLAYPAUSE",		/* 0xA2 */
	NULL,					/* 0xA3 */
	"DIK_MEDIASTOP",		/* 0xA4 */
	NULL,					/* 0xA5 */
	NULL,					/* 0xA6 */
	NULL,					/* 0xA7 */
	NULL,					/* 0xA8 */
	NULL,					/* 0xA9 */
	NULL,					/* 0xAA */
	NULL,					/* 0xAB */
	NULL,					/* 0xAC */
	NULL,					/* 0xAD */
	"DIK_VOLUMEDOWN",		/* 0xAE */
	NULL,					/* 0xAF */
	"DIK_VOLUMEUP",			/* 0xB0 */
	NULL,					/* 0xB1 */
	"DIK_WEBHOME",			/* 0xB2 */
	"DIK_NUMPADCOMMA",		/* 0xB3 */
	NULL,					/* 0xB4 */
	"DIK_DIVIDE",			/* 0xB5 */
	NULL,					/* 0xB6 */
	"DIK_SYSRQ",			/* 0xB7 */
	"DIK_RMENU",			/* 0xB8 */
	NULL,					/* 0xB9 */
	NULL,					/* 0xBA */
	NULL,					/* 0xBB */
	NULL,					/* 0xBC */
	NULL,					/* 0xBD */
	NULL,					/* 0xBE */
	NULL,					/* 0xBF */
	NULL,					/* 0xC0 */
	NULL,					/* 0xC1 */
	NULL,					/* 0xC2 */
	NULL,					/* 0xC3 */
	NULL,					/* 0xC4 */
	"DIK_PAUSE",			/* 0xC5 */
	NULL,					/* 0xC6 */
	"DIK_HOME",				/* 0xC7 */
	"DIK_UP",				/* 0xC8 */
	"DIK_PRIOR",			/* 0xC9 */
	NULL,					/* 0xCA */
	"DIK_LEFT",				/* 0xCB */
	NULL,					/* 0xCC */
	"DIK_RIGHT",			/* 0xCD */
	NULL,					/* 0xCE */
	"DIK_END",				/* 0xCF */
	"DIK_DOWN",				/* 0xD0 */
	"DIK_NEXT",				/* 0xD1 */
	"DIK_INSERT",			/* 0xD2 */
	"DIK_DELETE",			/* 0xD3 */
	NULL,					/* 0xD4 */
	NULL,					/* 0xD5 */
	NULL,					/* 0xD6 */
	NULL,					/* 0xD7 */
	NULL,					/* 0xD8 */
	NULL,					/* 0xD9 */
	NULL,					/* 0xDA */
	"DIK_LWIN",				/* 0xDB */
	"DIK_RWIN",				/* 0xDC */
	"DIK_APPS",				/* 0xDD */
	"DIK_POWER",			/* 0xDE */
	"DIK_SLEEP",			/* 0xDF */
	NULL,					/* 0xE0 */
	NULL,					/* 0xE1 */
	NULL,					/* 0xE2 */
	"DIK_WAKE",				/* 0xE3 */
	NULL,					/* 0xE4 */
	"DIK_WEBSEARCH",		/* 0xE5 */
	"DIK_WEBFAVORITES",		/* 0xE6 */
	"DIK_WEBREFRESH",		/* 0xE7 */
	"DIK_WEBSTOP",			/* 0xE8 */
	"DIK_WEBFORWARD",		/* 0xE9 */
	"DIK_WEBBACK",			/* 0xEA */
	"DIK_MYCOMPUTER",		/* 0xEB */
	"DIK_MAIL",				/* 0xEC */
	"DIK_MEDIASELECT",		/* 0xED */
	NULL,					/* 0xEE */
	NULL,					/* 0xEF */
	NULL,					/* 0xF0 */
	NULL,					/* 0xF1 */
	NULL,					/* 0xF2 */
	NULL,					/* 0xF3 */
	NULL,					/* 0xF4 */
	NULL,					/* 0xF5 */
	NULL,					/* 0xF6 */
	NULL,					/* 0xF7 */
	NULL,					/* 0xF8 */
	NULL,					/* 0xF9 */
	NULL,					/* 0xFA */
	NULL,					/* 0xFB */
	NULL,					/* 0xFC */
	NULL,					/* 0xFD */
	NULL,					/* 0xFE */
	NULL					/* 0xFF */
};

/*
 *	�L�[�{�[�h�y�[�W
 *	FM77AV �L�[�R�[�h�e�[�u��
 */
#if XM7_VER >= 2
static char *KbdPageFM77AV[] = {
	NULL, NULL,				/* 0x00 */
	"ESC", NULL,			/* 0x01 */
	"1", "��",				/* 0x02 */
	"2", "��",				/* 0x03 */
	"3", "��",				/* 0x04 */
	"4", "��",				/* 0x05 */
	"5", "��",				/* 0x06 */
	"6", "��",				/* 0x07 */
	"7", "��",				/* 0x08 */
	"8", "��",				/* 0x09 */
	"9", "��",				/* 0x0A */
	"0", "��",				/* 0x0B */
	"-", "��",				/* 0x0C */
	"^", "��",				/* 0x0D */
	"\\", "�[",				/* 0x0E */
	"BS", NULL,				/* 0x0F */
	"TAB", NULL,			/* 0x10 */
	"Q", "��",				/* 0x11 */
	"W", "��",				/* 0x12 */
	"E", "��",				/* 0x13 */
	"R", "��",				/* 0x14 */
	"T", "��",				/* 0x15 */
	"Y", "��",				/* 0x16 */
	"U", "��",				/* 0x17 */
	"I", "��",				/* 0x18 */
	"O", "��",				/* 0x19 */
	"P", "��",				/* 0x1A */
	"@", "�J",				/* 0x1B */
	"[", "�K",				/* 0x1C */
	"RETURN", NULL,			/* 0x1D */
	"A", "��",				/* 0x1E */
	"S", "��",				/* 0x1F */
	"D", "��",				/* 0x20 */
	"F", "��",				/* 0x21 */
	"G", "��",				/* 0x22 */
	"H", "��",				/* 0x23 */
	"J", "��",				/* 0x24 */
	"K", "��",				/* 0x25 */
	"L", "��",				/* 0x26 */
	";", "��",				/* 0x27 */
	":", "��",				/* 0x28 */
	"]", "��",				/* 0x29 */
	"Z", "��",				/* 0x2A */
	"X", "��",				/* 0x2B */
	"C", "��",				/* 0x2C */
	"V", "��",				/* 0x2D */
	"B", "��",				/* 0x2E */
	"N", "��",				/* 0x2F */
	"M", "��",				/* 0x30 */
	",", "��",				/* 0x31 */
	".", "��",				/* 0x32 */
	"/", "��",				/* 0x33 */
	"_", "��",				/* 0x34 */
	"SPACE(�E)", NULL,		/* 0x35 */
	"*", "�e���L�[",		/* 0x36 */
	"/", "�e���L�[",		/* 0x37 */
	"+", "�e���L�[",		/* 0x38 */
	"-", "�e���L�[",		/* 0x39 */
	"7", "�e���L�[",		/* 0x3A */
	"8", "�e���L�[",		/* 0x3B */
	"9", "�e���L�[",		/* 0x3C */
	"=", "�e���L�[",		/* 0x3D */
	"4", "�e���L�[",		/* 0x3E */
	"5", "�e���L�[",		/* 0x3F */
	"6", "�e���L�[",		/* 0x40 */
	",", "�e���L�[",		/* 0x41 */
	"1", "�e���L�[",		/* 0x42 */
	"2", "�e���L�[",		/* 0x43 */
	"3", "�e���L�[",		/* 0x44 */
	"RETURN", "�e���L�[",	/* 0x45 */
	"0", "�e���L�[",		/* 0x46 */
	".", "�e���L�[",		/* 0x47 */
	"INS", NULL,			/* 0x48 */
	"EL", NULL,				/* 0x49 */
	"CLS", NULL,			/* 0x4A */
	"DEL", NULL,			/* 0x4B */
	"DUP", NULL,			/* 0x4C */
	"��", NULL,				/* 0x4D */
	"HOME", NULL,			/* 0x4E */
	"��", NULL,				/* 0x4F */
	"��", NULL,				/* 0x50 */
	"��", NULL,				/* 0x51 */
	"CTRL", NULL,			/* 0x52 */
	"SHIFT(��)", NULL,		/* 0x53 */
	"SHIFT(�E)", NULL,		/* 0x54 */
	"CAP", NULL,			/* 0x55 */
	"GRAPH", NULL,			/* 0x56 */
	"SPACE(��)", NULL,		/* 0x57 */
	"SPACE(��)", NULL,		/* 0x58 */
	NULL, NULL,				/* 0x59 */
	"����", NULL,			/* 0x5A */
	NULL, NULL,				/* 0x5B */
	"BREAK", NULL,			/* 0x5C */
	"PF1",	NULL,			/* 0x5D */
	"PF2",	NULL,			/* 0x5E */
	"PF3",	NULL,			/* 0x5F */
	"PF4",	NULL,			/* 0x60 */
	"PF5",	NULL,			/* 0x61 */
	"PF6",	NULL,			/* 0x62 */
	"PF7",	NULL,			/* 0x63 */
	"PF8",	NULL,			/* 0x64 */
	"PF9",	NULL,			/* 0x65 */
	"PF10",	NULL			/* 0x66 */
};
#else
static char *KbdPageFM77AV[] = {
	NULL, NULL,				/* 0x00 */
	"ESC", NULL,			/* 0x01 */
	"1", "�k",				/* 0x02 */
	"2", "�t",				/* 0x03 */
	"3", "�A",				/* 0x04 */
	"4", "�E",				/* 0x05 */
	"5", "�G",				/* 0x06 */
	"6", "�I",				/* 0x07 */
	"7", "��",				/* 0x08 */
	"8", "��",				/* 0x09 */
	"9", "��",				/* 0x0A */
	"0", "��",				/* 0x0B */
	"-", "�z",				/* 0x0C */
	"^", "�w",				/* 0x0D */
	"\\", "�[",				/* 0x0E */
	"BS", NULL,				/* 0x0F */
	"TAB", NULL,			/* 0x10 */
	"Q", "�^",				/* 0x11 */
	"W", "�e",				/* 0x12 */
	"E", "�C",				/* 0x13 */
	"R", "�X",				/* 0x14 */
	"T", "�J",				/* 0x15 */
	"Y", "��",				/* 0x16 */
	"U", "�i",				/* 0x17 */
	"I", "�j",				/* 0x18 */
	"O", "��",				/* 0x19 */
	"P", "�Z",				/* 0x1A */
	"@", "�J",				/* 0x1B */
	"[", "�K",				/* 0x1C */
	"RETURN", NULL,			/* 0x1D */
	"A", "�`",				/* 0x1E */
	"S", "�g",				/* 0x1F */
	"D", "�V",				/* 0x20 */
	"F", "�n",				/* 0x21 */
	"G", "�L",				/* 0x22 */
	"H", "�N",				/* 0x23 */
	"J", "�}",				/* 0x24 */
	"K", "�m",				/* 0x25 */
	"L", "��",				/* 0x26 */
	";", "��",				/* 0x27 */
	":", "�P",				/* 0x28 */
	"]", "��",				/* 0x29 */
	"Z", "�c",				/* 0x2A */
	"X", "�T",				/* 0x2B */
	"C", "�\",				/* 0x2C */
	"V", "�q",				/* 0x2D */
	"B", "�R",				/* 0x2E */
	"N", "�~",				/* 0x2F */
	"M", "��",				/* 0x30 */
	",", "�l",				/* 0x31 */
	".", "��",				/* 0x32 */
	"/", "��",				/* 0x33 */
	"_", "��",				/* 0x34 */
	"SPACE", NULL,			/* 0x35 */
	"*", "�e���L�[",		/* 0x36 */
	"/", "�e���L�[",		/* 0x37 */
	"+", "�e���L�[",		/* 0x38 */
	"-", "�e���L�[",		/* 0x39 */
	"7", "�e���L�[",		/* 0x3A */
	"8", "�e���L�[",		/* 0x3B */
	"9", "�e���L�[",		/* 0x3C */
	"=", "�e���L�[",		/* 0x3D */
	"4", "�e���L�[",		/* 0x3E */
	"5", "�e���L�[",		/* 0x3F */
	"6", "�e���L�[",		/* 0x40 */
	",", "�e���L�[",		/* 0x41 */
	"1", "�e���L�[",		/* 0x42 */
	"2", "�e���L�[",		/* 0x43 */
	"3", "�e���L�[",		/* 0x44 */
	"RETURN", "�e���L�[",	/* 0x45 */
	"0", "�e���L�[",		/* 0x46 */
	".", "�e���L�[",		/* 0x47 */
	"INS", NULL,			/* 0x48 */
	"EL", NULL,				/* 0x49 */
	"CLS", "(CLEAR)",		/* 0x4A */
	"DEL", NULL,			/* 0x4B */
	"DUP", NULL,			/* 0x4C */
	"��", NULL,				/* 0x4D */
	"HOME", NULL,			/* 0x4E */
	"��", NULL,				/* 0x4F */
	"��", NULL,				/* 0x50 */
	"��", NULL,				/* 0x51 */
	"CTRL", NULL,			/* 0x52 */
	"SHIFT(��)", NULL,		/* 0x53 */
	"SHIFT(�E)", NULL,		/* 0x54 */
	"CAP", NULL,			/* 0x55 */
	"GRAPH", NULL,			/* 0x56 */
	NULL, NULL,				/* 0x57 */
	NULL, NULL,				/* 0x58 */
	NULL, NULL,				/* 0x59 */
	"�J�i", NULL,			/* 0x5A */
	NULL, NULL,				/* 0x5B */
	"BREAK", "(STOP)",		/* 0x5C */
	"PF1",	NULL,			/* 0x5D */
	"PF2",	NULL,			/* 0x5E */
	"PF3",	NULL,			/* 0x5F */
	"PF4",	NULL,			/* 0x60 */
	"PF5",	NULL,			/* 0x61 */
	"PF6",	NULL,			/* 0x62 */
	"PF7",	NULL,			/* 0x63 */
	"PF8",	NULL,			/* 0x64 */
	"PF9",	NULL,			/* 0x65 */
	"PF10",	NULL			/* 0x66 */
};
#endif

/*
 *	�C���f�b�N�X��FM77AV �L�[�R�[�h
 */
static int FASTCALL KbdPageIndex2FM77AV(int index)
{
	int i;

	for (i=0; i<sizeof(KbdPageFM77AV)/sizeof(char *)/2; i++) {
		/* NULL�̃L�[�̓X�L�b�v */
		if (KbdPageFM77AV[i * 2] == NULL) {
			continue;
		}

		/* NULL�łȂ���΁A�`�F�b�N���f�N�������g */
		if (index == 0) {
			return i;
		}
		index--;
	}

	/* �G���[ */
	return 0;
}

/*
 *	FM77AV �L�[�R�[�h���C���f�b�N�X
 */
static int FASTCALL KbdPageFM77AV2Index(int keycode)
{
	int i;
	int index;

	index = 0;
	for (i=0; i<sizeof(KbdPageFM77AV)/sizeof(char *)/2; i++) {
		/* �L�[�R�[�h�ɓ��B������I�� */
		if (i == keycode) {
			break;
		}

		/* NULL�̃L�[�̓X�L�b�v */
		if (KbdPageFM77AV[i * 2] == NULL) {
			continue;
		}

		/* NULL�łȂ���΁A�C���N�������g */
		index++;
	}

	return index;
}

/*
 *	FM77AV �L�[�R�[�h��DirectInput �L�[�R�[�h
 */
static int FASTCALL KbdPageFM77AV2DirectInput(int fm)
{
	int i;

	/* ���� */
	for (i=0; i<256; i++) {
		if (propdat.KeyMap[i] == fm) {
			return i;
		}
	}

	/* �G���[ */
	return 0;
}

/*
 *	�L�[���̓_�C�A���O
 *	�_�C�A���O������
 */
static void FASTCALL KeyInDlgInit(HWND hDlg)
{
	HWND hWnd;
	RECT prect;
	RECT drect;
	int fm;
	int di;
	char formstr[128];
	char string[128];

	ASSERT(hDlg);

	/* �e�E�C���h�E�̒����ɐݒ� */
	hWnd = GetParent(hDlg);
	GetWindowRect(hWnd, &prect);
	GetWindowRect(hDlg, &drect);
	drect.right -= drect.left;
	drect.bottom -= drect.top;
	drect.left = (prect.right - prect.left) / 2 + prect.left;
	drect.left -= (drect.right / 2);
	drect.top = (prect.bottom - prect.top) / 2 + prect.top;
	drect.top -= (drect.bottom / 2);
	MoveWindow(hDlg, drect.left, drect.top, drect.right, drect.bottom, FALSE);

	/* �L�[�ԍ��e�L�X�g������ */
	fm = KbdPageIndex2FM77AV(KbdPageSelectID);
	formstr[0] = '\0';
	LoadString(hAppInstance, IDC_KEYIN_LABEL, formstr, sizeof(formstr));
	_snprintf(string, sizeof(string), formstr, fm);
	hWnd = GetDlgItem(hDlg, IDC_KEYIN_LABEL);
	SetWindowText(hWnd, string);

	/* DirectInput�L�[�e�L�X�g������ */
	di = KbdPageFM77AV2DirectInput(fm);
	ASSERT((di >= 0) && (di < 256));
	hWnd = GetDlgItem(hDlg, IDC_KEYIN_KEY);
	if (KbdPageDirectInput[di]) {
		SetWindowText(hWnd, KbdPageDirectInput[di]);
	}
	else {
		LoadString(hAppInstance, IDC_KEYIN_KEY, string, sizeof(string));
		SetWindowText(hWnd, string);
	}

	/* �O���[�o�����[�N������ */
	KbdPageCurrentKey = di;
	GetKbd(KbdPageMap);

	/* �^�C�}�[���X�^�[�g */
	SetTimer(hDlg, IDD_KEYINDLG, 50, NULL);
}

/*
 *	�L�[���̓_�C�A���O
 *	�^�C�}�[
 */
static void FASTCALL KeyInTimer(HWND hDlg)
{
	BYTE buf[256];
	int i;
	HWND hWnd;
	char string[128];

	ASSERT(hDlg);

	/* �L�[���� */
	LockVM();
	GetKbd(buf);
	UnlockVM();

	/* ���񉟂��ꂽ�L�[����� */
	for (i=0; i<256; i++) {
		if ((KbdPageMap[i] ^ buf[i]) & 0x80) {
			break;
		}
	}

	/* �L�[�}�b�v���X�V���A�`�F�b�N */
	memcpy(KbdPageMap, buf, 256);
	if (i >= 256) {
		return;
	}

	/* �L�[�̔ԍ��A�e�L�X�g���Z�b�g */
	KbdPageCurrentKey = i;

	hWnd = GetDlgItem(hDlg, IDC_KEYIN_KEY);
	if (KbdPageDirectInput[i]) {
		SetWindowText(hWnd, KbdPageDirectInput[i]);
	}
	else {
		LoadString(hAppInstance, IDC_KEYIN_KEY, string, sizeof(string));
		SetWindowText(hWnd, string);
	}
}

/*
 *	�L�[���̓_�C�A���O
 *	�_�C�A���O�v���V�[�W��
 */
static BOOL CALLBACK KeyInDlgProc(HWND hDlg, UINT iMsg,
									WPARAM wParam, LPARAM lParam)
{
	UNUSED(lParam);

	switch (iMsg) {
		/* �_�C�A���O������ */
		case WM_INITDIALOG:
			KeyInDlgInit(hDlg);

			/* IME�I�t */
			EnableIME(hDlg, FALSE);

			return TRUE;

		/* �R�}���h */
		case WM_COMMAND:
			if (LOWORD(wParam) != IDCANCEL) {
				return TRUE;
			}
			/* ESC�L�[�����m���邽�߂̍H�v */
			if (KbdPageCurrentKey == 0x01) {
				EndDialog(hDlg, IDCANCEL);
				return TRUE;
			}
			KeyInTimer(hDlg);
			if (KbdPageCurrentKey == 0x01) {
				return TRUE;
			}
			EndDialog(hDlg, IDCANCEL);
			return TRUE;

		/* �^�C�}�[ */
		case WM_TIMER:
			KeyInTimer(hDlg);
			return TRUE;

		/* �E�N���b�N */
		case WM_RBUTTONDOWN:
			EndDialog(hDlg, IDOK);
			return TRUE;

		/* �E�C���h�E�j�� */
		case WM_DESTROY:
			KillTimer(hDlg, IDD_KEYINDLG);

			/* IME�I�� */
			EnableIME(hDlg, TRUE);

			break;
	}

	/* ����ȊO��FALSE */
	return FALSE;
}

/*-[ �L�[�{�[�h�y�[�W ]-----------------------------------------------------*/

/*
 *	�L�[�{�[�h�y�[�W
 *	�_�C�A���O������(�w�b�_�[�A�C�e��)
 */
static void FASTCALL KbdPageInitColumn(HWND hWnd)
{
	int i;
	char string[128];
	TEXTMETRIC tm;
	HDC hDC;
	LV_COLUMN lvc;
	static const UINT uHeaderTable[] = {
		IDS_KP_KEYNO,
		IDS_KP_KEYFM,
		IDS_KP_KEYKANA,
		IDS_KP_KEYDI
	};

	ASSERT(hWnd);

	/* �e�L�X�g���g���b�N���擾 */
	hDC = GetDC(hWnd);
	GetTextMetrics(hDC, &tm);
	ReleaseDC(hWnd, hDC);

	/* �}�����[�v */
	for (i=0; i<(sizeof(uHeaderTable)/sizeof(UINT)); i++) {
		/* �e�L�X�g�����[�h */
		LoadString(hAppInstance, uHeaderTable[i], string, sizeof(string));

		/* �J�����\���̂��쐬 */
		lvc.mask = LVCF_FMT | LVCF_SUBITEM | LVCF_WIDTH | LVCF_TEXT;
		lvc.iSubItem = i;
		lvc.fmt = LVCFMT_LEFT;
		lvc.cx = (strlen(string) + 1) * tm.tmAveCharWidth;
		lvc.pszText = string;
		lvc.cchTextMax = strlen(string);

		/* �J�����}�� */
		ListView_InsertColumn(hWnd, i, &lvc);
	}
}

/*
 *	�L�[�{�[�h�y�[�W
 *	�T�u�A�C�e���ꊇ�ݒ�
 */
static void FASTCALL KbdPageSubItem(HWND hDlg)
{
	HWND hWnd;
	int i;
	int j;
	int index;

	/* ���X�g�R���g���[���擾 */
	hWnd = GetDlgItem(hDlg, IDC_KP_LIST);
	ASSERT(hWnd);

	/* �A�C�e���}�� */
	for (index=0; ; index++) {
		/* FM77AV�L�[�ԍ��𓾂� */
		i = KbdPageIndex2FM77AV(index);
		if (i == 0) {
			break;
		}

		/* �Y������DirectX�L�[�ԍ��𓾂� */
		j = KbdPageFM77AV2DirectInput(i);

		/* ������Z�b�g */
		if (KbdPageDirectInput[j]) {
			ListView_SetItemText(hWnd, index, 3, KbdPageDirectInput[j]);
		}
		else {
			ListView_SetItemText(hWnd, index, 3, "");
		}
	}
}

/*
 *	�L�[�{�[�h�y�[�W
 *	�_�C�A���O������
 */
static void FASTCALL KbdPageInit(HWND hDlg)
{
	HWND hWnd;
	LV_ITEM lvi;
	int index;
	int i;
	char string[128];

	/* �V�[�g������ */
	SheetInit(hDlg);

	/* ���X�g�R���g���[���擾 */
	hWnd = GetDlgItem(hDlg, IDC_KP_LIST);
	ASSERT(hWnd);

	/* �J�����}�� */
	KbdPageInitColumn(hWnd);

	/* �A�C�e���}�� */
	for (index=0; ; index++) {
		/* FM77AV�L�[�ԍ��𓾂� */
		i = KbdPageIndex2FM77AV(index);
		if (i == 0) {
			break;
		}

		/* �A�C�e���}�� */
		_snprintf(string, sizeof(string), "%02X", i);
		lvi.mask = LVIF_TEXT;
		lvi.pszText = string;
		lvi.cchTextMax = strlen(string);
		lvi.iItem = index;
		lvi.iSubItem = 0;
		ListView_InsertItem(hWnd, &lvi);

		/* �T�u�A�C�e���~�Q���Z�b�g */
		if (KbdPageFM77AV[i * 2 + 0]) {
			ListView_SetItemText(hWnd, index, 1, KbdPageFM77AV[i * 2 + 0]);
		}
		if (KbdPageFM77AV[i * 2 + 1]) {
			ListView_SetItemText(hWnd, index, 2, KbdPageFM77AV[i * 2 + 1]);
		}
	}

	/* �T�u�A�C�e�� */
	KbdPageSubItem(hDlg);

	/* �e���L�[�G�~�����[�V���� */
	if (propdat.bTenCursor) {
		CheckDlgButton(hDlg, IDC_KP_USEARROWFOR10, BST_CHECKED);
	}
	else {
		CheckDlgButton(hDlg, IDC_KP_USEARROWFOR10, BST_UNCHECKED);
	}

	/* �e���L�[�G�~�����[�V����8�������[�h */
	if (propdat.bArrow8Dir) {
		CheckDlgButton(hDlg, IDC_KP_ARROW8DIR, BST_CHECKED);
	}
	else {
		CheckDlgButton(hDlg, IDC_KP_ARROW8DIR, BST_UNCHECKED);
	}

	/* �^�����A���^�C���L�[�X�L���� */
	if (propdat.bKbdReal) {
		CheckDlgButton(hDlg, IDC_KP_KBDREAL, BST_CHECKED);
	}
	else {
		CheckDlgButton(hDlg, IDC_KP_KBDREAL, BST_UNCHECKED);
	}
}

/*
 *	�L�[�{�[�h�y�[�W
 *	�R�}���h
 */
static void FASTCALL KbdPageCmd(HWND hDlg, WORD wID, WORD wNotifyCode, HWND hWnd)
{
	ASSERT(hDlg);
	UNUSED(wNotifyCode);
	UNUSED(hWnd);

	switch (wID) {
		/* 106�L�[�{�[�h */
		case IDC_KP_106B:
			GetDefMapKbd(propdat.KeyMap, 1);
			KbdPageSubItem(hDlg);
			break;

		/* PC-98�L�[�{�[�h */
		case IDC_KP_98B:
			GetDefMapKbd(propdat.KeyMap, 2);
			KbdPageSubItem(hDlg);
			break;

		/* 101�L�[�{�[�h */
		case IDC_KP_101B:
			GetDefMapKbd(propdat.KeyMap, 3);
			KbdPageSubItem(hDlg);
			break;
	}
}

/*
 *	�L�[�{�[�h�y�[�W
 *	�_�u���N���b�N
 */
static void FASTCALL KbdPageDblClk(HWND hDlg)
{
	HWND hWnd;
	int count;
	int i;
	int j;

	ASSERT(hDlg);

	/* ���X�g�R���g���[���擾 */
	hWnd = GetDlgItem(hDlg, IDC_KP_LIST);
	ASSERT(hWnd);

	/* �I������Ă���A�C�e����������΁A���^�[�� */
	if (ListView_GetSelectedCount(hWnd) == 0) {
		return;
	}

	/* �o�^����Ă���A�C�e���̌����擾 */
	count = ListView_GetItemCount(hWnd);

	/* �Z���N�g����Ă���C���f�b�N�X�𓾂� */
	for (i=0; i<count; i++) {
		if (ListView_GetItemState(hWnd, i, LVIS_SELECTED)) {
			break;
		}
	}

	/* �O���[�o���֋L�� */
	ASSERT(i < count);
	KbdPageSelectID = i;

	/* ���[�_���_�C�A���O���s */
	if (DialogBox(hAppInstance, MAKEINTRESOURCE(IDD_KEYINDLG),
						hDlg, (DLGPROC)KeyInDlgProc) == IDOK) {
		/* �}�b�v���C�� */
		j = KbdPageIndex2FM77AV(i);
		for (i=0; i<256; i++) {
			if (propdat.KeyMap[i] == j) {
				propdat.KeyMap[i] = 0;
			}
		}
		propdat.KeyMap[KbdPageCurrentKey] = (BYTE)j;

		/* �ĕ`�� */
		KbdPageSubItem(hDlg);
	}
}

/*
 *	�L�[�{�[�h�y�[�W
 *	�K�p
 */
static void FASTCALL KbdPageApply(HWND hDlg)
{
	/* �X�e�[�g�ύX */
	uPropertyState = 2;

	/* �e���L�[�G�~�����[�V���� */
	if (IsDlgButtonChecked(hDlg, IDC_KP_USEARROWFOR10) == BST_CHECKED) {
		propdat.bTenCursor = TRUE;
	}
	else {
		propdat.bTenCursor = FALSE;
	}

	/* �e���L�[�G�~�����[�V���� 8�������[�h */
	if (IsDlgButtonChecked(hDlg, IDC_KP_ARROW8DIR) == BST_CHECKED) {
		propdat.bArrow8Dir = TRUE;
	}
	else {
		propdat.bArrow8Dir = FALSE;
	}

	/* �^�����A���^�C���L�[�X�L���� */
	if (IsDlgButtonChecked(hDlg, IDC_KP_KBDREAL) == BST_CHECKED) {
		propdat.bKbdReal = TRUE;
	}
	else {
		propdat.bKbdReal = FALSE;
	}
}

/*
 *	�L�[�{�[�h�y�[�W
 *	�_�C�A���O�v���V�[�W��
 */
static BOOL CALLBACK KbdPageProc(HWND hDlg, UINT msg,
									 WPARAM wParam, LPARAM lParam)
{
	LPNMHDR pnmh;
	LV_KEYDOWN *plkd;

	switch (msg) {
		/* ������ */
		case WM_INITDIALOG:
			KbdPageInit(hDlg);
			return TRUE;

		/* �R�}���h */
		case WM_COMMAND:
			KbdPageCmd(hDlg, LOWORD(wParam), HIWORD(wParam), (HWND)lParam);
			return TRUE;

		/* �ʒm */
		case WM_NOTIFY:
			pnmh = (LPNMHDR)lParam;
			/* �y�[�W�I�� */
			if (pnmh->code == PSN_APPLY) {
				KbdPageApply(hDlg);
				return TRUE;
			}
			/* ���X�g�r���[ �_�u���N���b�N */
			if ((pnmh->idFrom == IDC_KP_LIST) && (pnmh->code == NM_DBLCLK)) {
				KbdPageDblClk(hDlg);
				return TRUE;
			}
			/* ���X�g�r���[ ����L�[���� */
			if ((pnmh->idFrom == IDC_KP_LIST) && (pnmh->code == LVN_KEYDOWN)) {
				plkd = (LV_KEYDOWN*)pnmh;
				/* SPACE�����͂��ꂽ��L�[�I�� */
				if (plkd->wVKey == VK_SPACE) {
					KbdPageDblClk(hDlg);
					return TRUE;
				}
			}
			break;

		/* �J�[�\���ݒ� */
		case WM_SETCURSOR:
			if (HIWORD(lParam) == WM_MOUSEMOVE) {
				PageHelp(hDlg, IDC_KP_HELP);
			}
			break;
	}

	return FALSE;
}

/*-[ �W���C�X�e�B�b�N�y�[�W ]-----------------------------------------------*/

/*
 *	�W���C�X�e�B�b�N�y�[�W
 *	�R���{�{�b�N�X�e�[�u��
 */
static const UINT JoyPageComboTable[] = {
	IDC_JP_UPC,
	IDC_JP_DOWNC,
	IDC_JP_LEFTC,
	IDC_JP_RIGHTC,
	IDC_JP_CENTERC,
	IDC_JP_AC,
	IDC_JP_BC
};

/*
 *	�W���C�X�e�B�b�N�y�[�W
 *	�R�[�h�e�[�u��
 */
static const UINT JoyPageCodeTable[] = {
	IDS_JP_TYPE0,
	IDC_JP_UP,
	IDC_JP_DOWN,
	IDC_JP_LEFT,
	IDC_JP_RIGHT,
	IDC_JP_A,
	IDC_JP_B
};

/*
 *	�W���C�X�e�B�b�N�y�[�W
 *	�R���{�{�b�N�X�Z���N�g
 */
static void FASTCALL JoyPageCombo(HWND hDlg, WORD wID)
{
	int i;
	int j;
	HWND hWnd;
	char string[128];

	ASSERT(hDlg);

	/* ID�`�F�b�N */
	if (wID != IDC_JP_TYPEC) {
		return;
	}

	/* "�g�p���Ȃ�"���b�Z�[�W�����[�h */
	string[0] = '\0';
	LoadString(hAppInstance, IDS_JP_TYPE0, string, sizeof(string));

	/* �R���{�b�N�X���N���A */
	for (i=0; i<sizeof(JoyPageComboTable)/sizeof(UINT); i++) {
		hWnd = GetDlgItem(hDlg, JoyPageComboTable[i]);
		ASSERT(hWnd);
		(void)ComboBox_ResetContent(hWnd);
	}

	/* �^�C�v�擾 */
	hWnd = GetDlgItem(hDlg, IDC_JP_TYPEC);
	ASSERT(hWnd);
	j = ComboBox_GetCurSel(hWnd);

	/* �L�[�{�[�h�̏ꍇ */
	if (j == 3) {
		for (i=0; i<sizeof(JoyPageComboTable)/sizeof(UINT); i++) {
			/* �n���h���擾 */
			hWnd = GetDlgItem(hDlg, JoyPageComboTable[i]);
			ASSERT(hWnd);

			/* �L�[�R�[�h�}�� */
			(void)ComboBox_AddString(hWnd, string);
			for (j=0; j<sizeof(KbdPageFM77AV)/sizeof(char*); j+=2) {
				if (KbdPageFM77AV[j]) {
					(void)ComboBox_AddString(hWnd, KbdPageFM77AV[j]);
				}
			}
			/* �L�[�R�[�h�ɃJ�[�\�����킹 */
			if (propdat.nJoyCode[JoyPageIdx][i] == 0) {
				(void)ComboBox_SetCurSel(hWnd, 0);
				continue;
			}
			if (propdat.nJoyCode[JoyPageIdx][i] > 0x66) {
				(void)ComboBox_SetCurSel(hWnd, 0);
				continue;
			}
			j = KbdPageFM77AV2Index(propdat.nJoyCode[JoyPageIdx][i]);
			(void)ComboBox_SetCurSel(hWnd, j + 1);
		}
		return;
	}

	/* �W���C�X�e�B�b�N�̏ꍇ */
	for (i=0; i<sizeof(JoyPageComboTable)/sizeof(UINT); i++) {
		/* �n���h���擾 */
		hWnd = GetDlgItem(hDlg, JoyPageComboTable[i]);
		ASSERT(hWnd);

		/* ������}�� */
		for (j=0; j<sizeof(JoyPageCodeTable)/sizeof(UINT); j++) {
			string[0] = '\0';
			LoadString(hAppInstance, JoyPageCodeTable[j], string, sizeof(string));
			(void)ComboBox_AddString(hWnd, string);
		}

		/* �J�[�\���ݒ� */
		if (propdat.nJoyCode[JoyPageIdx][i] < 0x70) {
			(void)ComboBox_SetCurSel(hWnd, 0);
			continue;
		}
		if (propdat.nJoyCode[JoyPageIdx][i] < 0x74) {
			(void)ComboBox_SetCurSel(hWnd, propdat.nJoyCode[JoyPageIdx][i] - 0x70 + 1);
			continue;
		}
		(void)ComboBox_SetCurSel(hWnd, propdat.nJoyCode[JoyPageIdx][i] - 0x74 + 5);
	}
}

/*
 *	�W���C�X�e�B�b�N�y�[�W
 *	�f�[�^�Z�b�g
 */
static void FASTCALL JoyPageSet(HWND hDlg)
{
	HWND hWnd;

	ASSERT(hDlg);
	ASSERT((JoyPageIdx == 0) || (JoyPageIdx == 1));

	/* �A�˃R���{�{�b�N�X */
	hWnd = GetDlgItem(hDlg, IDC_JP_RAPIDAC);
	ASSERT(hWnd);
	(void)ComboBox_SetCurSel(hWnd, propdat.nJoyRapid[JoyPageIdx][0]);
	hWnd = GetDlgItem(hDlg, IDC_JP_RAPIDBC);
	ASSERT(hWnd);
	(void)ComboBox_SetCurSel(hWnd, propdat.nJoyRapid[JoyPageIdx][1]);

	/* �^�C�v�R���{�{�b�N�X */
	hWnd = GetDlgItem(hDlg, IDC_JP_TYPEC);
	ASSERT(hWnd);
	(void)ComboBox_SetCurSel(hWnd, propdat.nJoyType[JoyPageIdx]);

	/* �R�[�h���� */
	JoyPageCombo(hDlg, IDC_JP_TYPEC);
}

/*
 *	�W���C�X�e�B�b�N�y�[�W
 *	�f�[�^�擾
 */
static void FASTCALL JoyPageGet(HWND hDlg)
{
	HWND hWnd;
	int i;
	int j;

	ASSERT(hDlg);
	ASSERT((JoyPageIdx == 0) || (JoyPageIdx == 1));

	/* �A�˃R���{�{�b�N�X */
	hWnd = GetDlgItem(hDlg, IDC_JP_RAPIDAC);
	ASSERT(hWnd);
	propdat.nJoyRapid[JoyPageIdx][0] = ComboBox_GetCurSel(hWnd);
	hWnd = GetDlgItem(hDlg, IDC_JP_RAPIDBC);
	ASSERT(hWnd);
	propdat.nJoyRapid[JoyPageIdx][1] = ComboBox_GetCurSel(hWnd);

	/* �^�C�v�R���{�{�b�N�X */
	hWnd = GetDlgItem(hDlg, IDC_JP_TYPEC);
	ASSERT(hWnd);
	propdat.nJoyType[JoyPageIdx] = ComboBox_GetCurSel(hWnd);

	/* �R�[�h */
	if (propdat.nJoyType[JoyPageIdx] == 3) {
		/* �L�[�{�[�h */
		for (i=0; i<sizeof(JoyPageComboTable)/sizeof(UINT); i++) {
			/* �n���h���擾 */
			hWnd = GetDlgItem(hDlg, JoyPageComboTable[i]);
			ASSERT(hWnd);

			/* �R�[�h�ϊ��A�Z�b�g */
			j = ComboBox_GetCurSel(hWnd);
			if (j != 0) {
				j = KbdPageIndex2FM77AV(j - 1);
			}
			propdat.nJoyCode[JoyPageIdx][i] = j;
		}
		return;
	}

	/* �W���C�X�e�B�b�N */
	for (i=0; i<sizeof(JoyPageComboTable)/sizeof(UINT); i++) {
		/* �n���h���擾 */
		hWnd = GetDlgItem(hDlg, JoyPageComboTable[i]);
		ASSERT(hWnd);

		/* �R�[�h�ϊ��A�Z�b�g */
		j = ComboBox_GetCurSel(hWnd);
		if (j == 0) {
			propdat.nJoyCode[JoyPageIdx][i] = 0;
			continue;
		}
		if (j < 5) {
			propdat.nJoyCode[JoyPageIdx][i] = (j - 1) + 0x70;
			continue;
		}
		propdat.nJoyCode[JoyPageIdx][i] = (j - 5) + 0x74;
	}
}

/*
 *	�W���C�X�e�B�b�N�y�[�W
 *	�{�^�������ꂽ
 */
static void FASTCALL JoyPageButton(HWND hDlg, WORD wID)
{
	ASSERT(hDlg);

	switch (wID) {
		/* �|�[�g1 ��I�� */
		case IDC_JP_PORT1:
			JoyPageGet(hDlg);
			JoyPageIdx = 0;
			JoyPageSet(hDlg);
			break;

		/* �|�[�g2 ��I�� */
		case IDC_JP_PORT2:
			JoyPageGet(hDlg);
			JoyPageIdx = 1;
			JoyPageSet(hDlg);
			break;

		/* ����ȊO */
		default:
			ASSERT(FALSE);
			break;
	}
}

/*
 *	�W���C�X�e�B�b�N�y�[�W
 *	�_�C�A���O������
 */
static void FASTCALL JoyPageInit(HWND hDlg)
{
	HWND hWnd[2];
	int i;
	char string[128];
#if defined(LPRINT)
	int nJoyStick;
#endif

	/* �V�[�g������ */
	ASSERT(hDlg);
	SheetInit(hDlg);

	/* �^�C�v�R���{�{�b�N�X */
	hWnd[0] = GetDlgItem(hDlg, IDC_JP_TYPEC);
	ASSERT(hWnd[0]);
	(void)ComboBox_ResetContent(hWnd[0]);
#if defined(LPRINT)
	/* �v�����^�g�p */
	if (lp_use) {
		nJoyStick = 4;	/* "�d�g�V����JOY I/F" ������ */
	}
	else {
		nJoyStick = 5;	/* "�d�g�V����JOY I/F" ���܂� */
	}
	for (i=0; i<nJoyStick; i++) {
#else
	for (i=0; i<5; i++) {
#endif
		string[0] = '\0';
		LoadString(hAppInstance, IDS_JP_TYPE0 + i, string, sizeof(string));
		(void)ComboBox_AddString(hWnd[0], string);
	}
	(void)ComboBox_SetCurSel(hWnd[0], 0);

	/* �A�˃R���{�{�b�N�X */
	hWnd[0] = GetDlgItem(hDlg, IDC_JP_RAPIDAC);
	ASSERT(hWnd[0]);
	(void)ComboBox_ResetContent(hWnd[0]);
	hWnd[1] = GetDlgItem(hDlg, IDC_JP_RAPIDBC);
	ASSERT(hWnd[1]);
	(void)ComboBox_ResetContent(hWnd[1]);
	for (i=0; i<10; i++) {
		string[0] = '\0';
		LoadString(hAppInstance, IDS_JP_RAPID0 + i, string, sizeof(string));
		(void)ComboBox_AddString(hWnd[0], string);
		(void)ComboBox_AddString(hWnd[1], string);
	}
	(void)ComboBox_SetCurSel(hWnd[0], 0);
	(void)ComboBox_SetCurSel(hWnd[1], 0);

	/* �|�[�g�I���O���[�v�{�^�� */
	CheckDlgButton(hDlg, IDC_JP_PORT1, BST_CHECKED);
	JoyPageIdx = 0;
	JoyPageSet(hDlg);
}

/*
 *	�W���C�X�e�B�b�N�y�[�W
 *	�K�p
 */
static void FASTCALL JoyPageApply(HWND hDlg)
{
	ASSERT(hDlg);

	/* �X�e�[�g�ύX */
	uPropertyState = 2;

	/* �f�[�^�擾 */
	JoyPageGet(hDlg);
}

/*
 *	�W���C�X�e�B�b�N�y�[�W
 *	�_�C�A���O�v���V�[�W��
 */
static BOOL CALLBACK JoyPageProc(HWND hDlg, UINT msg,
									 WPARAM wParam, LPARAM lParam)
{
	LPNMHDR pnmh;

	switch (msg) {
		/* ������ */
		case WM_INITDIALOG:
			JoyPageInit(hDlg);
			return TRUE;

		/* �R�}���h */
		case WM_COMMAND:
			switch (HIWORD(wParam)) {
				/* �{�^���N���b�N */
				case BN_CLICKED:
					JoyPageButton(hDlg, LOWORD(wParam));
					break;
				/* �R���{�I�� */
				case CBN_SELCHANGE:
					JoyPageCombo(hDlg, LOWORD(wParam));
					break;
			}
			return TRUE;

		/* �ʒm */
		case WM_NOTIFY:
			pnmh = (LPNMHDR)lParam;
			if (pnmh->code == PSN_APPLY) {
				JoyPageApply(hDlg);
				return TRUE;
			}
			break;

		/* �J�[�\���ݒ� */
		case WM_SETCURSOR:
			if (HIWORD(lParam) == WM_MOUSEMOVE) {
				PageHelp(hDlg, IDC_JP_HELP);
			}
			break;
	}

	return FALSE;
}

/*-[ �X�N���[���y�[�W ]-----------------------------------------------------*/

/*
 *	�X�N���[���y�[�W
 *	�_�C�A���O������
 */
static void FASTCALL ScrPageInit(HWND hDlg)
{
	HWND hWnd;
	char string[128];
	int i;

	/* �V�[�g������ */
	ASSERT(hDlg);
	SheetInit(hDlg);

	/* �S��ʗD�惂�[�h */
	hWnd = GetDlgItem(hDlg, IDC_SCP_MODEC);
	ASSERT(hWnd);
	string[0] = '\0';
	(void)ComboBox_ResetContent(hWnd);
	for (i=0; i<5; i++) {
		LoadString(hAppInstance, IDS_SCP_400LINE + i, string, sizeof(string));
		(void)ComboBox_AddString(hWnd, string);
	}
	(void)ComboBox_SetCurSel(hWnd, propdat.nDDResolutionMode);

	/* �E�B���h�E���[�h���t���X�L����(24k) */
	if (propdat.bFullScan) {
		CheckDlgButton(hDlg, IDC_SCP_24K, BST_CHECKED);
	}
	else {
		CheckDlgButton(hDlg, IDC_SCP_24K, BST_UNCHECKED);
	}

	/* �t���X�N���[�����t���X�L����(24k) */
	if (propdat.bFullScanFS) {
		CheckDlgButton(hDlg, IDC_SCP_24KFS, BST_CHECKED);
	}
	else {
		CheckDlgButton(hDlg, IDC_SCP_24KFS, BST_UNCHECKED);
	}

	/* �E�B���h�E���[�h��2�{�\�� */
	if (propdat.bDoubleSize) {
		CheckDlgButton(hDlg, IDC_SCP_DOUBLESIZE, BST_CHECKED);
	}
	else {
		CheckDlgButton(hDlg, IDC_SCP_DOUBLESIZE, BST_UNCHECKED);
	}

	/* �㉺�X�e�[�^�X */
	if (propdat.bDD480Status) {
		CheckDlgButton(hDlg, IDC_SCP_CAPTIONB, BST_CHECKED);
	}
	else {
		CheckDlgButton(hDlg, IDC_SCP_CAPTIONB, BST_UNCHECKED);
	}

	/* ��ʕ`��ʒm�^�C�~���O */
	if (propdat.bRasterRender) {
		CheckDlgButton(hDlg, IDC_SCP_RASTERRENDER, BST_CHECKED);
	}
	else {
		CheckDlgButton(hDlg, IDC_SCP_RASTERRENDER, BST_UNCHECKED);
	}

#if XM7_VER == 1
	/* �O���[�����j�^���[�h */
	if (propdat.bGreenMonitor) {
		CheckDlgButton(hDlg, IDC_SCP_GREENMONITOR, BST_CHECKED);
	}
	else {
		CheckDlgButton(hDlg, IDC_SCP_GREENMONITOR, BST_UNCHECKED);
	}
#endif

#if XM7_VER == 2
	/* TTL���j�^���[�h */
	if (propdat.bTTLMonitor) {
		CheckDlgButton(hDlg, IDC_SCP_TTLMONITOR, BST_CHECKED);
	}
	else {
		CheckDlgButton(hDlg, IDC_SCP_TTLMONITOR, BST_UNCHECKED);
	}
#endif

	/* TrueColor�D�� */
	if (propdat.bDDtruecolor || bWin8flag) {
		CheckDlgButton(hDlg, IDC_SCP_TRUECOLOR, BST_CHECKED);
	}
	else {
		CheckDlgButton(hDlg, IDC_SCP_TRUECOLOR, BST_UNCHECKED);
	}
	hWnd = GetDlgItem(hDlg, IDC_SCP_TRUECOLOR);
	EnableWindow(hWnd, !bWin8flag);

	/* �^��400���C�� */
	if (propdat.bPseudo400Line) {
		CheckDlgButton(hDlg, IDC_SCP_PSEUDO400LINE, BST_CHECKED);
	}
	else {
		CheckDlgButton(hDlg, IDC_SCP_PSEUDO400LINE, BST_UNCHECKED);
	}
}

/*
 *	�X�N���[���y�[�W
 *	�K�p
 */
static void FASTCALL ScrPageApply(HWND hDlg)
{
	HWND hWnd;

	ASSERT(hDlg);

	/* �X�e�[�g�ύX */
	uPropertyState = 2;

	/* �S��ʗD�惂�[�h */
	hWnd = GetDlgItem(hDlg, IDC_SCP_MODEC);
	ASSERT(hWnd);
	propdat.nDDResolutionMode = (BYTE)ComboBox_GetCurSel(hWnd);
	if (propdat.nDDResolutionMode > 4) {
		propdat.nDDResolutionMode = DDRES_480LINE;
	}

	/* �E�B���h�E���[�h���t���X�L����(24k) */
	if (IsDlgButtonChecked(hDlg, IDC_SCP_24K) == BST_CHECKED) {
		propdat.bFullScan = TRUE;
	}
	else {
		propdat.bFullScan = FALSE;
	}

	/* �t���X�N���[�����t���X�L����(24k) */
	if (IsDlgButtonChecked(hDlg, IDC_SCP_24KFS) == BST_CHECKED) {
		propdat.bFullScanFS = TRUE;
	}
	else {
		propdat.bFullScanFS = FALSE;
	}

	/* �E�B���h�E���[�h��2�{�\�� */
	if (IsDlgButtonChecked(hDlg, IDC_SCP_DOUBLESIZE) == BST_CHECKED) {
		propdat.bDoubleSize = TRUE;
	}
	else {
		propdat.bDoubleSize = FALSE;
	}

	/* �㉺�X�e�[�^�X */
	if (IsDlgButtonChecked(hDlg, IDC_SCP_CAPTIONB) == BST_CHECKED) {
		propdat.bDD480Status = TRUE;
	}
	else {
		propdat.bDD480Status = FALSE;
	}

	/* ���X�^�����_�����O */
	if (IsDlgButtonChecked(hDlg, IDC_SCP_RASTERRENDER) == BST_CHECKED) {
		propdat.bRasterRender = TRUE;
	}
	else {
		propdat.bRasterRender = FALSE;
	}

#if XM7_VER == 1
	/* �O���[�����j�^���[�h */
	if (IsDlgButtonChecked(hDlg, IDC_SCP_GREENMONITOR) == BST_CHECKED) {
		propdat.bGreenMonitor = TRUE;
	}
	else {
		propdat.bGreenMonitor = FALSE;
	}
#endif

#if XM7_VER == 2
	/* TTL���j�^���[�h */
	if (IsDlgButtonChecked(hDlg, IDC_SCP_TTLMONITOR) == BST_CHECKED) {
		propdat.bTTLMonitor = TRUE;
	}
	else {
		propdat.bTTLMonitor = FALSE;
	}
#endif

	/* TrueColor�D�� */
	if (IsDlgButtonChecked(hDlg, IDC_SCP_TRUECOLOR) == BST_CHECKED) {
		propdat.bDDtruecolor = TRUE;
	}
	else {
		propdat.bDDtruecolor = FALSE;
	}

	/* �^���S�O�O���C�����[�h */
	if (IsDlgButtonChecked(hDlg, IDC_SCP_PSEUDO400LINE) == BST_CHECKED) {
		propdat.bPseudo400Line = TRUE;
	}
	else {
		propdat.bPseudo400Line = FALSE;
	}
}

/*
 *	�X�N���[���y�[�W
 *	�_�C�A���O�v���V�[�W��
 */
static BOOL CALLBACK ScrPageProc(HWND hDlg, UINT msg,
									 WPARAM wParam, LPARAM lParam)
{
	LPNMHDR pnmh;

	UNUSED(wParam);

	switch (msg) {
		/* ������ */
		case WM_INITDIALOG:
			ScrPageInit(hDlg);
			return TRUE;

		/* �ʒm */
		case WM_NOTIFY:
			pnmh = (LPNMHDR)lParam;
			if (pnmh->code == PSN_APPLY) {
				ScrPageApply(hDlg);
				return TRUE;
			}
			break;

		/* �J�[�\���ݒ� */
		case WM_SETCURSOR:
			if (HIWORD(lParam) == WM_MOUSEMOVE) {
				PageHelp(hDlg, IDC_SCP_HELP);
			}
			break;
	}

	return FALSE;
}

/*-[ �|�[�g�y�[�W ]---------------------------------------------------------*/

#if defined(MIDI) || defined(RSC)

#if defined(MIDI)
/*
 *	�|�[�g�y�[�W
 *	�R�}���h
 */
static void FASTCALL PortPageCmd(HWND hDlg, WORD wID, WORD wNotifyCode, HWND hWnd)
{
	HWND hSpin;
	BOOL flag;
	int tmp;

	ASSERT(hDlg);

	/* ID�� */
	switch (wID) {
		/* MIDI�����x������ */
		case IDC_POP_MIDIDLYEDIT:
			if (wNotifyCode == EN_CHANGE) {
				tmp = GetDlgItemInt(hDlg, IDC_POP_MIDIDLYEDIT, 0, FALSE) / 10;
				if (tmp < 0) {
					tmp = 0;
				}
				else if (tmp > 100) {
					tmp = 100;
				}

				hSpin = GetDlgItem(hDlg, IDC_POP_MIDIDLYSPIN);
				ASSERT(hSpin);
				UpDown_SetPos(hSpin, tmp);
			}
			break;

		/* MIDI�����x�����T�E���h�o�b�t�@���ƈ�v������ */
		case IDC_POP_MIDIDLYSB:
			if (IsDlgButtonChecked(hDlg, IDC_POP_MIDIDLYSB) == BST_CHECKED) {
				flag = FALSE;
			}
			else {
				flag = TRUE;
			}
			hWnd = GetDlgItem(hDlg, IDC_POP_MIDIDLYSPIN);
			EnableWindow(hWnd, flag);
			hWnd = GetDlgItem(hDlg, IDC_POP_MIDIDLYEDIT);
			EnableWindow(hWnd, flag);
			break;
	}
}

/*
 *	�|�[�g�y�[�W
 *	�����X�N���[��
 */
static void FASTCALL PortPageVScroll(HWND hDlg, WORD wPos, HWND hWnd)
{
	HWND hBuddyWnd;
	char string[128];

	ASSERT(hDlg);
	ASSERT(hWnd);

	/* �E�C���h�E�n���h�����`�F�b�N */
	if (hWnd == GetDlgItem(hDlg, IDC_POP_MIDIDLYSPIN)) {
		/* MIDI�����x������ */
		/* �|�W�V��������A�o�f�B�E�C���h�E�ɒl��ݒ� */
		hBuddyWnd = GetDlgItem(hDlg, IDC_POP_MIDIDLYEDIT);
		ASSERT(hBuddyWnd);
		_snprintf(string, sizeof(string), "%d", wPos * 10);
		SetWindowText(hBuddyWnd, string);
	}
}
#endif

/*
 *	�|�[�g�y�[�W
 *	�_�C�A���O������
 */
static void FASTCALL PortPageInit(HWND hDlg)
{
#if defined(MIDI)
	MIDIOUTCAPS moc;
#endif
	HWND hWnd;
	char string[128];
	int i, numsel;

	/* �V�[�g������ */
	ASSERT(hDlg);
	SheetInit(hDlg);

#if defined(MIDI)
	/* MIDI�|�[�g�I�� */
	numsel = 0;
	hWnd = GetDlgItem(hDlg, IDC_POP_MIDIDEVC);
	ASSERT(hWnd);
	string[0] = '\0';
	(void)ComboBox_ResetContent(hWnd);
	LoadString(hAppInstance, IDS_POP_NONE, string, sizeof(string));
	(void)ComboBox_AddString(hWnd, string);
	for (i=0; i<(int)midiOutGetNumDevs(); i++) {
		midiOutGetDevCaps(i, &moc, sizeof(moc));
		(void)ComboBox_AddString(hWnd, moc.szPname);
		if (!strcmp(moc.szPname, propdat.szMidiDevice)) {
			numsel = i + 1;
		}
	}
	(void)ComboBox_SetCurSel(hWnd, numsel);

	/* MIDI�����x������ */
	hWnd = GetDlgItem(hDlg, IDC_POP_MIDIDLYSPIN);
	ASSERT(hWnd);
	UpDown_SetRange(hWnd, 100, 0);
	UpDown_SetPos(hWnd, propdat.nMidiDelay / 10);
	PortPageVScroll(hDlg, LOWORD(UpDown_GetPos(hWnd)), hWnd);
	EnableWindow(hWnd, !propdat.bMidiDelayMode);
	hWnd = GetDlgItem(hDlg, IDC_POP_MIDIDLYEDIT);
	EnableWindow(hWnd, !propdat.bMidiDelayMode);

	/* MIDI�����x�����T�E���h�o�b�t�@���ƈ�v������ */
	if (propdat.bMidiDelayMode) {
		CheckDlgButton(hDlg, IDC_POP_MIDIDLYSB, BST_CHECKED);
	}
	else {
		CheckDlgButton(hDlg, IDC_POP_MIDIDLYSB, BST_UNCHECKED);
	}
#endif

#if defined(RSC)
	/* RS-232C�g�p */
	if (propdat.bCommPortEnable) {
		CheckDlgButton(hDlg, IDC_POP_COMENABLE, BST_CHECKED);
	}
	else {
		CheckDlgButton(hDlg, IDC_POP_COMENABLE, BST_UNCHECKED);
	}

	/* RS-232C�{�[���[�g�I�� */
	hWnd = GetDlgItem(hDlg, IDC_POP_COMBPSC);
	ASSERT(hWnd);
	string[0] = '\0';
	(void)ComboBox_ResetContent(hWnd);
	for (i=0; i<5; i++) {
		LoadString(hAppInstance, IDS_POP_COM300 + i, string, sizeof(string));
		(void)ComboBox_AddString(hWnd, string);
	}
	(void)ComboBox_SetCurSel(hWnd, propdat.uCommPortBps);

	/* RS-232C�|�[�g�I�� */
	hWnd = GetDlgItem(hDlg, IDC_POP_COMPORTC);
	ASSERT(hWnd);
	string[0] = '\0';
	(void)ComboBox_ResetContent(hWnd);
	for (i=1; i<=16; i++) {
		_snprintf(string, sizeof(string), "COM%d", i);
		(void)ComboBox_AddString(hWnd, string);
	}
	(void)ComboBox_SetCurSel(hWnd, propdat.nCommPortNo - 1);
#endif
}

/*
 *	�|�[�g�y�[�W
 *	�K�p
 */
static void FASTCALL PortPageApply(HWND hDlg)
{
	MIDIOUTCAPS moc;
	HWND hWnd;
	UINT uPos;
	int i;

	ASSERT(hDlg);

	/* �X�e�[�g�ύX */
	uPropertyState = 2;

#if defined(MIDI)
	/* MIDI�|�[�g */
	hWnd = GetDlgItem(hDlg, IDC_POP_MIDIDEVC);
	i = ComboBox_GetCurSel(hWnd);
	if (i == 0) {
		strncpy(propdat.szMidiDevice, "", sizeof(propdat.szMidiDevice));
	}
	else {
		midiOutGetDevCaps(i - 1, &moc, sizeof(moc));
		strncpy(propdat.szMidiDevice, moc.szPname, 
				sizeof(propdat.szMidiDevice));
	}

	/* MIDI�����x������ */
	hWnd = GetDlgItem(hDlg, IDC_POP_MIDIDLYSPIN);
	ASSERT(hWnd);
	uPos = LOWORD(UpDown_GetPos(hWnd));
	propdat.nMidiDelay = uPos * 10;

	/* MIDI�����x�����T�E���h�o�b�t�@���ƈ�v������ */
	if (IsDlgButtonChecked(hDlg, IDC_POP_MIDIDLYSB) == BST_CHECKED) {
		propdat.bMidiDelayMode = TRUE;
	}
	else {
		propdat.bMidiDelayMode = FALSE;
	}
#endif

#if defined(RSC)
	/* RS-232C�g�p */
	if (IsDlgButtonChecked(hDlg, IDC_POP_COMENABLE) == BST_CHECKED) {
		propdat.bCommPortEnable = TRUE;
	}
	else {
		propdat.bCommPortEnable = FALSE;
	}

	/* RS-232C�{�[���[�g */
	hWnd = GetDlgItem(hDlg, IDC_POP_COMBPSC);
	propdat.uCommPortBps = (BYTE)ComboBox_GetCurSel(hWnd);

	/* RS-232C�|�[�g */
	hWnd = GetDlgItem(hDlg, IDC_POP_COMPORTC);
	propdat.nCommPortNo = ComboBox_GetCurSel(hWnd) + 1;
#endif
}

/*
 *	�|�[�g�y�[�W
 *	�_�C�A���O�v���V�[�W��
 */
static BOOL CALLBACK PortPageProc(HWND hDlg, UINT msg,
									 WPARAM wParam, LPARAM lParam)
{
	LPNMHDR pnmh;

	switch (msg) {
		/* ������ */
		case WM_INITDIALOG:
			PortPageInit(hDlg);
			return TRUE;

#if defined(MIDI)
		/* �R�}���h */
		case WM_COMMAND:
			PortPageCmd(hDlg, LOWORD(wParam), HIWORD(wParam), (HWND)lParam);
			return TRUE;
#endif

		/* �ʒm */
		case WM_NOTIFY:
			pnmh = (LPNMHDR)lParam;
			if (pnmh->code == PSN_APPLY) {
				PortPageApply(hDlg);
				return TRUE;
			}
			break;

#if defined(MIDI)
		/* �����X�N���[�� */
		case WM_VSCROLL:
			PortPageVScroll(hDlg, HIWORD(wParam), (HWND)lParam);
			break;
#endif

		/* �J�[�\���ݒ� */
		case WM_SETCURSOR:
			if (HIWORD(lParam) == WM_MOUSEMOVE) {
				PageHelp(hDlg, IDC_POP_HELP);
			}
			break;
	}

	return FALSE;
}

#endif	/* RSC/MIDI */

/*-[ �v�����^�y�[�W ]--------------------------------------------------------*/

#if defined(LPRINT)

/*
 *	�v�����^�y�[�W
 *	�_�C�A���O������
 */
static void FASTCALL LprPageInit(HWND hDlg)
{
#if !defined(JASTSOUND)
HWND hWnd;
#endif

	/* �V�[�g������ */
	ASSERT(hDlg);
	SheetInit(hDlg);

	/* �v�����^�G�~�����[�V�������[�h */
	switch (propdat.uPrinterEnable) {
		case LP_EMULATION:
			CheckDlgButton(hDlg, IDC_LPP_LPREMUENABLE, BST_CHECKED);
			break;
		case LP_LOG:
			CheckDlgButton(hDlg, IDC_LPP_LPRLOGENABLE, BST_CHECKED);
			break;
#if defined(JASTSOUND)
		case LP_JASTSOUND:
			CheckDlgButton(hDlg, IDC_LPP_LPRJASTSOUNDENABLE, BST_CHECKED);
			break;
#endif
		case LP_DISABLE:
			CheckDlgButton(hDlg, IDC_LPP_LPRDISABLE, BST_CHECKED);
			break;
		default:
			ASSERT(FALSE);
	}

#if !defined(JASTSOUND)
	hWnd = GetDlgItem(hDlg, IDC_LPP_LPRJASTSOUNDENABLE);
	EnableWindow(hWnd, FALSE);
#endif

	/* OS�̃t�H���g�𗘗p���� */
	if (propdat.bLprUseOsFont) {
		CheckDlgButton(hDlg, IDC_LPP_LPROSFNT, BST_CHECKED);
	}
	else {
		CheckDlgButton(hDlg, IDC_LPP_LPROSFNT, BST_UNCHECKED);
	}

	/* �������o�͂��� */
	if (propdat.bLprOutputKanji) {
		CheckDlgButton(hDlg, IDC_LPP_LPRKANJI, BST_CHECKED);
	}
	else {
		CheckDlgButton(hDlg, IDC_LPP_LPRKANJI, BST_UNCHECKED);
	}

	/* �v�����^���O�o�̓p�X */
	SetWindowText(GetDlgItem(hDlg, IDC_LPP_LPRLOGPATHNAME),
		propdat.szLprLogPath);
}

/*
 *	�v�����^�y�[�W
 *	�R�}���h
 */
static void FASTCALL LprPageCmd(HWND hDlg, WORD wID, WORD wNotifyCode, HWND hWnd)
{
	char tmp[MAX_PATH + 1];
	char path[256 + 1];

	ASSERT(hDlg);
	UNUSED(wNotifyCode);
	UNUSED(hWnd);

	/* ID�� */
	switch (wID) {
		/* �v�����^���O�o�̓p�X */
		case IDC_LPP_LPRLOGDIALOG:
#if XM7_VER == 1
#if defined(BUBBLE)
			if (!FileSelectSub(FALSE, IDS_LPRFILTER, tmp, NULL, 6)) {
#else
			if (!FileSelectSub(FALSE, IDS_LPRFILTER, tmp, NULL, 5)) {
#endif
#else
			if (!FileSelectSub(FALSE, IDS_LPRFILTER, tmp, NULL, 5)) {
#endif
				return;
			}
			_snprintf(path, sizeof(path), tmp);
			SetWindowText(GetDlgItem(hDlg, IDC_LPP_LPRLOGPATHNAME), path);
			break;
	}
}

/*
 *	�v�����^�y�[�W
 *	�K�p
 */
static void FASTCALL LprPageApply(HWND hDlg)
{
	ASSERT(hDlg);

	/* �X�e�[�g�ύX */
	uPropertyState = 2;

	/* �v�����^�g�p */
	if (IsDlgButtonChecked(hDlg, IDC_LPP_LPREMUENABLE) == BST_CHECKED) {
		propdat.uPrinterEnable = LP_EMULATION;
	}
	else if (IsDlgButtonChecked(hDlg, IDC_LPP_LPRLOGENABLE) == BST_CHECKED) {
		propdat.uPrinterEnable = LP_LOG;
	}
#if defined(JASTSOUND)
	else if (IsDlgButtonChecked(hDlg, IDC_LPP_LPRJASTSOUNDENABLE) == BST_CHECKED) {
		propdat.uPrinterEnable = LP_JASTSOUND;
	}
#endif
	else {
		propdat.uPrinterEnable = LP_DISABLE;
	}

	/* OS�̃t�H���g�𗘗p���� */
	if (IsDlgButtonChecked(hDlg, IDC_LPP_LPROSFNT) == BST_CHECKED) {
		propdat.bLprUseOsFont = TRUE;
	}
	else {
		propdat.bLprUseOsFont = FALSE;
	}

	/* �������o�͂��� */
	if (IsDlgButtonChecked(hDlg, IDC_LPP_LPRKANJI) == BST_CHECKED) {
		propdat.bLprOutputKanji = TRUE;
	}
	else {
		propdat.bLprOutputKanji = FALSE;
	}

	/* �v�����^���O�o�̓p�X */
	GetWindowText(GetDlgItem(hDlg, IDC_LPP_LPRLOGPATHNAME),
		propdat.szLprLogPath, 256);
}

/*
 *	�v�����^�y�[�W
 *	�_�C�A���O�v���V�[�W��
 */
static BOOL CALLBACK LprPageProc(HWND hDlg, UINT msg,
									 WPARAM wParam, LPARAM lParam)
{
	LPNMHDR pnmh;

	UNUSED(wParam);

	switch (msg) {
		/* ������ */
		case WM_INITDIALOG:
			LprPageInit(hDlg);
			return TRUE;

		/* �R�}���h */
		case WM_COMMAND:
			LprPageCmd(hDlg, LOWORD(wParam), HIWORD(wParam), (HWND)lParam);
			return TRUE;

		/* �ʒm */
		case WM_NOTIFY:
			pnmh = (LPNMHDR)lParam;
			if (pnmh->code == PSN_APPLY) {
				LprPageApply(hDlg);
				return TRUE;
			}
			break;

		/* �J�[�\���ݒ� */
		case WM_SETCURSOR:
			if (HIWORD(lParam) == WM_MOUSEMOVE) {
				PageHelp(hDlg, IDC_LPP_HELP);
			}
			break;
	}

	return FALSE;
}

#endif /* LPRINT */

/*-[ �I�v�V�����y�[�W ]-----------------------------------------------------*/

/*
 *	�I�v�V�����y�[�W
 *	�_�C�A���O������
 */
static void FASTCALL OptPageInit(HWND hDlg)
{
#if (XM7_VER == 1 && defined(L4CARD)) || defined(MOUSE)
	HWND hWnd;
#if defined(MOUSE)
	char string[128];
#endif
#endif

	/* �V�[�g������ */
	ASSERT(hDlg);
	SheetInit(hDlg);

	/* OPN */
	if (propdat.bOPNEnable) {
		CheckDlgButton(hDlg, IDC_OP_OPNB, BST_CHECKED);
	}
	else {
		CheckDlgButton(hDlg, IDC_OP_OPNB, BST_UNCHECKED);
	}

	/* WHG */
	if (propdat.bWHGEnable) {
		CheckDlgButton(hDlg, IDC_OP_WHGB, BST_CHECKED);
	}
	else {
		CheckDlgButton(hDlg, IDC_OP_WHGB, BST_UNCHECKED);
	}

	/* THG */
	if (propdat.bTHGEnable) {
		CheckDlgButton(hDlg, IDC_OP_THGB, BST_CHECKED);
	}
	else {
		CheckDlgButton(hDlg, IDC_OP_THGB, BST_UNCHECKED);
	}

#if XM7_VER >= 2
	/* �r�f�I�f�B�W�^�C�Y */
	if (propdat.bDigitizeEnable) {
		CheckDlgButton(hDlg, IDC_OP_DIGITIZEB, BST_CHECKED);
	}
	else {
		CheckDlgButton(hDlg, IDC_OP_DIGITIZEB, BST_UNCHECKED);
	}
#endif

#if XM7_VER >= 2
	/* ���{��J�[�h */
	if (propdat.bJCardEnable) {
		CheckDlgButton(hDlg, IDC_OP_JCARDB, BST_CHECKED);
	}
	else {
		CheckDlgButton(hDlg, IDC_OP_JCARDB, BST_UNCHECKED);
	}
#if XM7_VER == 2
	if (!jcard_available) {
		hWnd = GetDlgItem(hDlg, IDC_OP_JCARDB);
		EnableWindow(hWnd, FALSE);
	}
#endif
#endif

#if ((XM7_VER >= 3) || defined(FMTV151))
	/* �g��RAM/FMTV-151 */
	if (propdat.bExtRAMEnable) {
		CheckDlgButton(hDlg, IDC_OP_RAMB, BST_CHECKED);
	}
	else {
		CheckDlgButton(hDlg, IDC_OP_RAMB, BST_UNCHECKED);
	}
#endif

#if XM7_VER == 1
#if defined(L4CARD)
	/* 400���C���J�[�h */
	if (detect_400linecard && propdat.b400LineCardEnable) {
		CheckDlgButton(hDlg, IDC_OP_400LINEB, BST_CHECKED);
	}
	else {
		CheckDlgButton(hDlg, IDC_OP_400LINEB, BST_UNCHECKED);
	}
	if (!detect_400linecard) {
		hWnd = GetDlgItem(hDlg, IDC_OP_400LINEB);
		EnableWindow(hWnd, FALSE);
	}
#endif

#if defined(JSUB)
	/* ���{��T�u�V�X�e�� */
	if (propdat.bJSubEnable) {
		CheckDlgButton(hDlg, IDC_OP_JSUBB, BST_CHECKED);
	}
	else {
		CheckDlgButton(hDlg, IDC_OP_JSUBB, BST_UNCHECKED);
	}
	if (!jsub_available) {
		hWnd = GetDlgItem(hDlg, IDC_OP_JSUBB);
		EnableWindow(hWnd, FALSE);
	}
#endif

#if defined(BUBBLE)
	/* �o�u�������� */
	if (propdat.bBubbleEnable) {
		CheckDlgButton(hDlg, IDC_OP_BMCB, BST_CHECKED);
	}
	else {
		CheckDlgButton(hDlg, IDC_OP_BMCB, BST_UNCHECKED);
	}
#endif
#endif

#if defined(MOUSE)
	/* �}�E�X�G�~�����[�V���� */
	if (propdat.bMouseCapture) {
		CheckDlgButton(hDlg, IDC_OP_MOUSEEM, BST_CHECKED);
	}
	else {
		CheckDlgButton(hDlg, IDC_OP_MOUSEEM, BST_UNCHECKED);
	}

	/* �}�E�X�ڑ��|�[�g */
	if (propdat.nMousePort == 1) {
		CheckDlgButton(hDlg, IDC_OP_MOUSE_PORT1, BST_CHECKED);
	}
	else if (propdat.nMousePort == 2) {
		CheckDlgButton(hDlg, IDC_OP_MOUSE_PORT2, BST_CHECKED);
	}
	else {
		CheckDlgButton(hDlg, IDC_OP_MOUSE_FMMOUSE, BST_CHECKED);
	}

	/* �}�E�X���[�h�؂�ւ����� */
	hWnd = GetDlgItem(hDlg, IDC_OP_MOUSESWC);
	ASSERT(hWnd);
	string[0] = '\0';
	(void)ComboBox_ResetContent(hWnd);
	LoadString(hAppInstance, IDS_OP_MOUSESW_1, string, sizeof(string));
	(void)ComboBox_AddString(hWnd, string);
	LoadString(hAppInstance, IDS_OP_MOUSESW_2, string, sizeof(string));
	(void)ComboBox_AddString(hWnd, string);
	LoadString(hAppInstance, IDS_OP_MOUSESW_3, string, sizeof(string));
	(void)ComboBox_AddString(hWnd, string);
	(void)ComboBox_SetCurSel(hWnd, propdat.uMidBtnMode);
#endif
}

/*
 *	�I�v�V�����y�[�W
 *	�K�p
 */
static void FASTCALL OptPageApply(HWND hDlg)
{
#if defined(MOUSE)
	HWND hWnd;
#endif

	ASSERT(hDlg);

	/* �X�e�[�g�ύX */
	uPropertyState = 2;

	/* OPN */
	if (IsDlgButtonChecked(hDlg, IDC_OP_OPNB) == BST_CHECKED) {
		propdat.bOPNEnable = TRUE;
	}
	else {
		propdat.bOPNEnable = FALSE;
	}

	/* WHG */
	if (IsDlgButtonChecked(hDlg, IDC_OP_WHGB) == BST_CHECKED) {
		propdat.bWHGEnable = TRUE;
	}
	else {
		propdat.bWHGEnable = FALSE;
	}

	/* THG */
	if (IsDlgButtonChecked(hDlg, IDC_OP_THGB) == BST_CHECKED) {
		propdat.bTHGEnable = TRUE;
	}
	else {
		propdat.bTHGEnable = FALSE;
	}

#if XM7_VER >= 2
	/* �r�f�I�f�B�W�^�C�Y */
	if (IsDlgButtonChecked(hDlg, IDC_OP_DIGITIZEB) == BST_CHECKED) {
		propdat.bDigitizeEnable = TRUE;
	}
	else {
		propdat.bDigitizeEnable = FALSE;
	}
#endif

#if XM7_VER >= 2
	/* ���{��J�[�h */
	if (IsDlgButtonChecked(hDlg, IDC_OP_JCARDB) == BST_CHECKED) {
		propdat.bJCardEnable = TRUE;
	}
	else {
		propdat.bJCardEnable = FALSE;
	}
#endif

#if ((XM7_VER >= 3) || defined(FMTV151))
	/* �g��RAM/FMTV-151 */
	if (IsDlgButtonChecked(hDlg, IDC_OP_RAMB) == BST_CHECKED) {
		propdat.bExtRAMEnable = TRUE;
	}
	else {
		propdat.bExtRAMEnable = FALSE;
	}
#endif

#if XM7_VER == 1
#if defined(L4CARD)
	/* 400���C���J�[�h */
	if (IsDlgButtonChecked(hDlg, IDC_OP_400LINEB) == BST_CHECKED) {
		propdat.b400LineCardEnable = TRUE;
	}
	else {
		propdat.b400LineCardEnable = FALSE;
	}
#endif

#if defined(JSUB)
	/* ���{��T�u�V�X�e�� */
	if (IsDlgButtonChecked(hDlg, IDC_OP_JSUBB) == BST_CHECKED) {
		propdat.bJSubEnable = TRUE;
	}
	else {
		propdat.bJSubEnable = FALSE;
	}
#endif

#if defined(BUBBLE)
	/* �o�u�������� */
	if (IsDlgButtonChecked(hDlg, IDC_OP_BMCB) == BST_CHECKED) {
		propdat.bBubbleEnable = TRUE;
	}
	else {
		propdat.bBubbleEnable = FALSE;
	}
#endif
#endif

#if defined(MOUSE)
	/* �}�E�X�G�~�����[�V���� */
	if (IsDlgButtonChecked(hDlg, IDC_OP_MOUSEEM) == BST_CHECKED) {
		propdat.bMouseCapture = TRUE;
	}
	else {
		propdat.bMouseCapture = FALSE;
	}

	/* �}�E�X�ڑ��|�[�g */
	if (IsDlgButtonChecked(hDlg, IDC_OP_MOUSE_PORT1) == BST_CHECKED) {
		propdat.nMousePort = 1;
	}
	else if (IsDlgButtonChecked(hDlg, IDC_OP_MOUSE_PORT2) == BST_CHECKED) {
		propdat.nMousePort = 2;
	}
	else {
		propdat.nMousePort = 3;
	}

	/* �}�E�X���[�h�؂�ւ����� */
	hWnd = GetDlgItem(hDlg, IDC_OP_MOUSESWC);
	propdat.uMidBtnMode = (BYTE)ComboBox_GetCurSel(hWnd);
#endif
}

/*
 *	�I�v�V�����y�[�W
 *	�_�C�A���O�v���V�[�W��
 */
static BOOL CALLBACK OptPageProc(HWND hDlg, UINT msg,
									 WPARAM wParam, LPARAM lParam)
{
	LPNMHDR pnmh;

	UNUSED(wParam);

	switch (msg) {
		/* ������ */
		case WM_INITDIALOG:
			OptPageInit(hDlg);
			return TRUE;

		/* �ʒm */
		case WM_NOTIFY:
			pnmh = (LPNMHDR)lParam;
			if (pnmh->code == PSN_APPLY) {
				OptPageApply(hDlg);
				return TRUE;
			}
			break;

		/* �J�[�\���ݒ� */
		case WM_SETCURSOR:
			if (HIWORD(lParam) == WM_MOUSEMOVE) {
				PageHelp(hDlg, IDC_OP_HELP);
			}
			break;
	}

	return FALSE;
}

/*-[ �����E���̑��y�[�W ]---------------------------------------------------*/

/*
 *	�����E���̑��y�[�W
 *	�_�C�A���O������
 */
static void FASTCALL AltPageInit(HWND hDlg)
{
#if XM7_VER == 1
	char string[256];
	HWND hWnd;
#endif

	/* �V�[�g������ */
	ASSERT(hDlg);
	SheetInit(hDlg);

	/* MAGUS�΍􏈗� */
	if (propdat.bMagusPatch) {
		CheckDlgButton(hDlg, IDC_AP_MAGUSPATCH, BST_CHECKED);
	}
	else {
		CheckDlgButton(hDlg, IDC_AP_MAGUSPATCH, BST_UNCHECKED);
	}

#if XM7_VER == 1
	/* �o���N�؂芷���C�l�[�u�� */
	hWnd = GetDlgItem(hDlg, IDC_AP_BANKSELB);
	ASSERT(hWnd);
	string[0] = '\0';
	(void)ComboBox_ResetContent(hWnd);
	LoadString(hAppInstance, IDS_AP_BANKSEL_OFF, string, sizeof(string));
	(void)ComboBox_AddString(hWnd, string);
	LoadString(hAppInstance, IDS_AP_BANKSEL_ON, string, sizeof(string));
	(void)ComboBox_AddString(hWnd, string);
	LoadString(hAppInstance, IDS_AP_BANKSEL_ON_DIPSW, string, sizeof(string));
	(void)ComboBox_AddString(hWnd, string);
	(void)ComboBox_SetCurSel(hWnd, configdat.uBankSelectEnable);

	/* FM-8���[�h���e�[�v���[�^ON�������ᑬ���[�h */
	if (propdat.bMotorOnLowSpeed) {
		CheckDlgButton(hDlg, IDC_AP_MOTORON_LOWSPEED, BST_CHECKED);
	}
	else {
		CheckDlgButton(hDlg, IDC_AP_MOTORON_LOWSPEED, BST_UNCHECKED);
	}
#endif

	/* FM-7���[�h����RAM���������ύX */
	if (propdat.bRomRamWrite) {
		CheckDlgButton(hDlg, IDC_AP_ROMRAMWRITE, BST_CHECKED);
	}
	else {
		CheckDlgButton(hDlg, IDC_AP_ROMRAMWRITE, BST_UNCHECKED);
	}

	/* FDC�؂藣��(��:�`�F�b�N��Ԃ̓t���O�Ƌt�ł�) */
	if (propdat.bFdcEnable) {
		CheckDlgButton(hDlg, IDC_AP_FDCDISABLE, BST_UNCHECKED);
	}
	else {
		CheckDlgButton(hDlg, IDC_AP_FDCDISABLE, BST_CHECKED);
	}

	/* �T�u�E�C���h�E�̃|�b�v�A�b�v�� */
	if (propdat.bPopupSwnd) {
		CheckDlgButton(hDlg, IDC_AP_POPUPSWND, BST_CHECKED);
	}
	else {
		CheckDlgButton(hDlg, IDC_AP_POPUPSWND, BST_UNCHECKED);
	}

	/* �t�@�C���I���_�C�A���O�̃Z���^�����O */
	if (propdat.bOFNCentering) {
		CheckDlgButton(hDlg, IDC_AP_OFNCENTERING, BST_UNCHECKED);
	}
	else {
		CheckDlgButton(hDlg, IDC_AP_OFNCENTERING, BST_CHECKED);
	}

}

/*
 *	�����E���̑��y�[�W
 *	�K�p
 */
static void FASTCALL AltPageApply(HWND hDlg)
{
	ASSERT(hDlg);

	/* �X�e�[�g�ύX */
	uPropertyState = 2;

	/* MAGUS�΍􏈗� */
	if (IsDlgButtonChecked(hDlg, IDC_AP_MAGUSPATCH) == BST_CHECKED) {
		propdat.bMagusPatch = TRUE;
	}
	else {
		propdat.bMagusPatch = FALSE;
	}

#if XM7_VER == 1
	/* �o���N�؂芷���C�l�[�u�� */
	propdat.uBankSelectEnable =
		ComboBox_GetCurSel(GetDlgItem(hDlg, IDC_AP_BANKSELB));

	/* FM-8���[�h���J�Z�b�g���[�^ON�������ᑬ���[�h */
	if (IsDlgButtonChecked(hDlg, IDC_AP_MOTORON_LOWSPEED) == BST_CHECKED) {
		propdat.bMotorOnLowSpeed = TRUE;
	}
	else {
		propdat.bMotorOnLowSpeed = FALSE;
	}
#endif

	/* FM-7���[�h����RAM���������ύX */
	if (IsDlgButtonChecked(hDlg, IDC_AP_ROMRAMWRITE) == BST_CHECKED) {
		propdat.bRomRamWrite = TRUE;
	}
	else {
		propdat.bRomRamWrite = FALSE;
	}

	/* FDC�؂藣��(��:�`�F�b�N��Ԃ̓t���O�Ƌt�ł�) */
	if (IsDlgButtonChecked(hDlg, IDC_AP_FDCDISABLE) == BST_CHECKED) {
		propdat.bFdcEnable = FALSE;
	}
	else {
		propdat.bFdcEnable = TRUE;
	}

	/* �T�u�E�C���h�E�̃|�b�v�A�b�v�� */
	if (IsDlgButtonChecked(hDlg, IDC_AP_POPUPSWND) == BST_CHECKED) {
		propdat.bPopupSwnd = TRUE;
	}
	else {
		propdat.bPopupSwnd = FALSE;
	}

	/* �t�@�C���I���_�C�A���O�̃Z���^�����O */
	if (IsDlgButtonChecked(hDlg, IDC_AP_OFNCENTERING) == BST_CHECKED) {
		propdat.bOFNCentering = FALSE;
	}
	else {
		propdat.bOFNCentering = TRUE;
	}
}

/*
 *	�����E���̑��y�[�W
 *	�_�C�A���O�v���V�[�W��
 */
static BOOL CALLBACK AltPageProc(HWND hDlg, UINT msg,
									 WPARAM wParam, LPARAM lParam)
{
	LPNMHDR pnmh;

	UNUSED(wParam);

	switch (msg) {
		/* ������ */
		case WM_INITDIALOG:
			AltPageInit(hDlg);
			return TRUE;

		/* �ʒm */
		case WM_NOTIFY:
			pnmh = (LPNMHDR)lParam;
			if (pnmh->code == PSN_APPLY) {
				AltPageApply(hDlg);
				return TRUE;
			}
			break;

		/* �J�[�\���ݒ� */
		case WM_SETCURSOR:
			if (HIWORD(lParam) == WM_MOUSEMOVE) {
				PageHelp(hDlg, IDC_OP_HELP);
			}
			break;
	}

	return FALSE;
}

/*-[ �v���p�e�B�V�[�g ]-----------------------------------------------------*/

/*
 *	�v���p�e�B�V�[�g
 *	�y�[�W�쐬
 */
static HPROPSHEETPAGE FASTCALL PageCreate(UINT TemplateID,
			BOOL (CALLBACK *DlgProc)(HWND, UINT, WPARAM, LPARAM))
{
	PROPSHEETPAGE psp;

	/* �\���̂��쐬 */
	memset(&psp, 0, sizeof(PROPSHEETPAGE));
	psp.dwSize = PROPSHEETPAGE_V1_SIZE;
	psp.dwFlags = 0;
	psp.hInstance = hAppInstance;
	psp.u.pszTemplate = MAKEINTRESOURCE(TemplateID);
	psp.pfnDlgProc = (DLGPROC)DlgProc;

	return CreatePropertySheetPage(&psp);
}

/*
 *	�v���p�e�B�V�[�g
 *	������
 */
static void FASTCALL SheetInit(HWND hDlg)
{
	RECT drect;
	RECT prect;
	LONG lStyleEx;
	HWND hWnd;

	/* �������t���O���`�F�b�N�A�V�[�g�������ς݂ɐݒ� */
	if (uPropertyState > 0) {
		return;
	}
	uPropertyState = 1;

	/* �v���p�e�B�V�[�g����w���v�{�^�������� */
	lStyleEx = GetWindowLong(GetParent(hDlg), GWL_EXSTYLE);
	lStyleEx &= ~WS_EX_CONTEXTHELP;
	SetWindowLong(GetParent(hDlg), GWL_EXSTYLE, lStyleEx);

	/* �^�u�̕����s�\���� */
	hWnd = PropSheet_GetTabControl(GetParent(hDlg));
	lStyleEx = GetWindowLong(hWnd, GWL_STYLE);
	lStyleEx &= ~TCS_SINGLELINE;
	lStyleEx |= TCS_MULTILINE;
	SetWindowLong(hWnd, GWL_STYLE, lStyleEx);

	/* �v���p�e�B�V�[�g���A���C���E�C���h�E�̒����Ɋ񂹂� */
	GetWindowRect(hMainWnd, &prect);
	GetWindowRect(GetParent(hDlg), &drect);
	drect.right -= drect.left;
	drect.bottom -= drect.top;
	drect.left = (prect.right - prect.left) / 2 + prect.left;
	drect.left -= (drect.right / 2);
	drect.top = (prect.bottom - prect.top) / 2 + prect.top;
	drect.top -= (drect.bottom / 2);
	MoveWindow(GetParent(hDlg), drect.left, drect.top, drect.right, drect.bottom, FALSE);

}

/*
 *	�ݒ�(C)
 */
void FASTCALL OnConfig(HWND hWnd)
{
	PROPSHEETHEADER pshead;
	HPROPSHEETPAGE hpspage[16];
	int i;
	int ver;
	int page;

	ASSERT(hWnd);

	/* �f�[�^�]�� */
	propdat = configdat;

	/* �v���p�e�B�y�[�W�쐬 */
	page = 0;
	hpspage[page++] = PageCreate(IDD_GENERALPAGE, GeneralPageProc);
	hpspage[page++] = PageCreate(IDD_SOUNDPAGE, SoundPageProc);
	hpspage[page++] = PageCreate(IDD_VOLUMEPAGE, VolumePageProc);
	hpspage[page++] = PageCreate(IDD_KBDPAGE, KbdPageProc);
	hpspage[page++] = PageCreate(IDD_JOYPAGE, JoyPageProc);
	hpspage[page++] = PageCreate(IDD_SCRPAGE, ScrPageProc);
#if defined(RSC) || defined(MIDI)
	hpspage[page++] = PageCreate(IDD_PORTPAGE, PortPageProc);
#endif
#if defined(LPRINT)
	hpspage[page++] = PageCreate(IDD_LPRPAGE, LprPageProc);
#endif
	hpspage[page++] = PageCreate(IDD_OPTPAGE, OptPageProc);
	hpspage[page++] = PageCreate(IDD_ALTERPAGE, AltPageProc);

	/* �v���p�e�B�y�[�W�`�F�b�N */
	for (i=0; i<page; i++) {
		if (!hpspage[i]) {
			return;
		}
	}

	/* �v���p�e�B�V�[�g�쐬 */
	memset(&pshead, 0, sizeof(pshead));
	pshead.dwSize = PROPSHEETHEADER_V1_SIZE;
	pshead.dwFlags = PSH_NOAPPLYNOW;
	pshead.hwndParent = hWnd;
	pshead.hInstance = hAppInstance;
	pshead.pszCaption = MAKEINTRESOURCE(IDS_CONFIGCAPTION);
	pshead.nPages = page;
	pshead.u2.nStartPage = 0;
	pshead.u3.phpage = hpspage;

	/* �v���p�e�B�V�[�g���s */
	uPropertyState = 0;
	uPropertyHelp = 0;
	PropertySheet(&pshead);

	/* ���ʂ�ok�ȊO�Ȃ�I�� */
	if (uPropertyState != 2) {
		return;
	}

	/* ok�Ȃ̂ŁA�f�[�^�]�� */
	configdat = propdat;

	/* �K�p */
	LockVM();
#if XM7_VER >= 2
	ver = fm7_ver;
#else
	ver = fm_subtype;
#endif
	ApplyCfg();

	/* ����@��ύX�𔺂��ꍇ�A���Z�b�g���� */
#if XM7_VER == 1 && defined(BUBBLE)
	if ((fm_subtype != FMSUB_FM8) && (boot_mode == BOOT_BUBBLE)) {
		boot_mode = BOOT_DOS;
		GetCfg();
	}
#endif

#if XM7_VER >= 2
	if (ver != fm7_ver) {
#else
	if (ver != fm_subtype) {
#endif
		system_reset();
		OnRefresh(hMainWnd);
	}

	UnlockVM();
}
#endif	/* _WIN32 */
