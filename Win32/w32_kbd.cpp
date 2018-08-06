/*
 *	FM-7 EMULATOR "XM7"
 *
 *	Copyright (C) 1999-2017 �o�h�D(Twitter:@xm6_original)
 *	Copyright (C) 2001-2017 Ryu Takegami (Twitter:@RyuTakegami)
 *
 *	[ Win32API �L�[�{�[�h�E�W���C�X�e�B�b�N�E�}�E�X ]
 *
 *	RHG����
 *	  2001.12.19		�}�E�X���͕�(DirectInput�o�[�W����)�𓝍�
 *	  2002.01.05		WinNT�n�ł̃L�[���͕s��ւ̑΍�
 *	  2002.05.23		�}�E�X�����{�^���ł̃}�E�X�G�~�����[�V�����؊��ɑΉ�
 *	  2002.06.19		ALT�L�[�Ƃ̓����Ō��𖳌�������悤�ɂ���
 *	  2002.07.30		�W���C�X�e�B�b�N/�}�E�X�̃|�[�����O�Ԋu��`�擯���ɕύX
 *						0.5�b���ɃW���C�X�e�B�b�N�̐ڑ��`�F�b�N���s���悤�ɂ���
 *	  2002.08.15		XP�ł�DirectInput�̗���s���Ȏd�l�ύX(?)�ւ̑΍��ǉ�
 *	  2002.08.25		�^�����A���^�C���L�[�X�L�����̏��������P
 *						�J�[�\���L�[�ł̃e���L�[�G�~�����[�V�������ɋ^�����A��
 *						�^�C���L�[�X�L�����������Ȃ������C��
 *						�������XP�΍��NT�n�S�ĂŎg�p����悤�ɕύX
 *	  2002.12.25		�}�E�X�L���v�`�����������C��
 *	  2003.01.02		�}�E�X�L���v�`���I�����ɃJ�[�\���ʒu�̕�����ǉ�
 *	  2003.01.12		�N������Ƀ}�E�X�{�^����������ԂɂȂ�����C��
 *	  2003.04.22		NT�΍��Break�����ɃV�X�e���^�C�}�𗘗p����悤�ɕύX
 *	  2003.04.23		�X�L�������[�h����NT�΍􏈗�������
 *	  2003.05.27		BREAK�L�[��v�΍�L�[�Ɋ��蓖�Ă��ꍇ�̓����ύX
 *						�L�[�}�b�v�ύX���̃|�[�����O������NT�΍􏈗���ǉ�
 *	  2003.05.28		�e���L�[�ϊ����J�[�\���L�[����������Ԃ���΂ߕ�����
 *						�ϊ��ɂ��Ή�
 *	  2004.05.03		�z�C�[���ł̃}�E�X�L���v�`�����[�h�؂芷���𓱓�
 *	  2010.01.14		Vista�ȍ~�ł̉E�V�t�g�L�[���ւ̑΍���s����
 *	  2010.01.19		�L�[�A�T�C�����̉E�V�t�g�L�[���ւ̑΍���s����
 *	  2012.05.14		DirectInput�ł̃}�E�X���[�h�؂芷��������p�~
 *	  2015.02.03		DirectInput8�ɑΉ�(VC++ only)
 *	  2017.03.19		�L�[�{�[�h��ʎ������肪�p��L�[�{�[�h�Ő���ɓ��삵��
 *						���Ȃ����������C��
 */

#ifdef _WIN32

#define STRICT
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#ifdef DINPUT8
#define DIRECTINPUT_VERSION		0x0800		/* DirectX8���w�� */
#else
#define DIRECTINPUT_VERSION		0x0300		/* DirectX3���w�� */
#endif
#include <dinput.h>
#include <mmsystem.h>
#include <assert.h>
#include "xm7.h"
#include "mainetc.h"
#include "keyboard.h"
#include "device.h"
#include "mouse.h"
#include "event.h"
#include "w32.h"
#include "w32_sch.h"
#include "w32_kbd.h"
#include "w32_bar.h"
#include "w32_res.h"
#include "w32_draw.h"

/*
 *	�O���[�o�� ���[�N
 */
BYTE kbd_map[256];							/* �L�[�{�[�h �}�b�v */
BYTE kbd_table[256];						/* �Ή�����FM-7�����R�[�h */
int nJoyType[2];							/* �W���C�X�e�B�b�N�^�C�v */
int nJoyRapid[2][2];						/* �A�˃^�C�v */
int nJoyCode[2][7];							/* �����R�[�h */
BOOL bKbdReal;								/* �^�����A���^�C���L�[�X�L���� */
BOOL bTenCursor;							/* �e���L�[�ϊ� */
BOOL bArrow8Dir;							/* �e���L�[�ϊ� 8�������[�h */
BOOL bNTkeyPushFlag[4];						/* NT�΍��p�L�[�����t���O */
BOOL bNTkeyMakeFlag[128];					/* NT�΍��pMake���t���O */
BOOL bNTkbMode;								/* NT�΍����t���O */
#if defined(MOUSE)
BYTE uMidBtnMode;							/* �����{�^����Ԏ擾���[�h */
BOOL bDetectMouse;							/* �}�E�X�m�F�t���O */
#endif
#if defined(KBDPASTE)
UINT uPasteWait;							/* �y�[�X�g�҂�����(ms) */
UINT uPasteWaitCntl;						/* �y�[�X�g�҂����ԃR���g���[���R�[�h */
#endif

/*
 *	�X�^�e�B�b�N ���[�N
 */
#ifdef DINPUT8
static LPDIRECTINPUT8 lpdi;					/* DirectInput */

static LPDIRECTINPUTDEVICE8 lpkbd;			/* �L�[�{�[�h�f�o�C�X */
#else
static LPDIRECTINPUT lpdi;					/* DirectInput */

static LPDIRECTINPUTDEVICE lpkbd;			/* �L�[�{�[�h�f�o�C�X */
#endif

	static DWORD keytime;						/* �L�[�{�[�h�|�[�����O���� */
static DWORD menuexittime;					/* ���j���[�����o�����ԃ^�C�} */
static DWORD dmyreadtime;					/* �L�[�R�[�h�_�~�[���[�h�^�C�} */
static BOOL bDummyRead;						/* �L�[�R�[�h�_�~�[���[�h�t���O */

static BYTE nKeyBuffer[KEYBUFFER_SIZE];		/* �����L�[�o�b�t�@ */
static BYTE nKeyReadPtr;					/* �����L�[�o�b�t�@�Ǐo�|�C���^ */
static BYTE nKeyWritePtr;					/* �����L�[�o�b�t�@�����|�C���^ */
static BYTE nLastKey;						/* �Ō�ɉ����ꂽ�L�[(FULL) */
static BYTE nLastKey2;						/* �Ō�ɉ����ꂽ�L�[(10KEY) */
static BYTE nTenDir;						/* �e���L�[�ϊ� �����f�[�^ */
static BYTE nTenDir2;						/* �e���L�[�ϊ� �����f�[�^2 */

static BYTE joydat[3];						/* �W���C�X�e�B�b�N�f�[�^ */
static BYTE joybk[2];						/* �W���C�X�e�B�b�N�o�b�N�A�b�v */
static int joyrapid[2][2];					/* �W���C�X�e�B�b�N�A�˃J�E���^ */
static DWORD joytime;						/* �W���C�X�e�B�b�N�|�[�����O���� */
static DWORD joytime2;						/* �W���C�X�e�B�b�N�|�[�����O���� */
static BOOL joyplugged[2];					/* �W���C�X�e�B�b�N�ڑ��t���O */

#if defined(MOUSE)
#ifdef DINPUT8
static LPDIRECTINPUTDEVICE8 lpmos;			/* �}�E�X�f�o�C�X */
#else
static LPDIRECTINPUTDEVICE lpmos;			/* �}�E�X�f�o�C�X */
#endif

	static DWORD mostime;						/* �}�E�X�|�[�����O���� */
static BOOL bCapture;						/* �}�E�X�L���v�`���t���O(Win32) */
static int nMouseX;							/* �}�E�X X���ړ����� */
static int nMouseY;							/* �}�E�X Y���ړ����� */
static BYTE nMouseButton;					/* �}�E�X �{�^��������� */
static BYTE nMouseButtons;					/* �}�E�X �{�^���� */
static BOOL bMouseCursor;					/* �}�E�X �J�[�\���\����� */
static int nMouseSX;						/* �}�E�X X���W�ۑ� */
static int nMouseSY;						/* �}�E�X Y���W�ۑ� */
#endif

#if defined(KBDPASTE)
static PTSTR strPaste;						/* �y�[�X�g�f�[�^ */
static PTSTR strPtr;						/* �y�[�X�g�|�C���^ */
static char cPreKey;						/* ���O�̃R�[�h */
static UINT uWaitCnt;						/* �y�[�X�g�҂��J�E���^ */
#endif


/*
 *	DirectInput�R�[�h��FM-7 �����R�[�h
 *	�R�[�h�Ώƕ\(106�L�[�{�[�h�p)
 */
static BYTE kbd_106_table[] = {
	DIK_ESCAPE,			0x5c,			/* BREAK(ESC) */
	DIK_F1,				0x5d,			/* PF1 */
	DIK_F2,				0x5e,			/* PF2 */
	DIK_F3,				0x5f,			/* PF3 */
	DIK_F4,				0x60,			/* PF4 */
	DIK_F5,				0x61,			/* PF5 */
	DIK_F6,				0x62,			/* PF6 */
	DIK_F7,				0x63,			/* PF7 */
	DIK_F8,				0x64,			/* PF8 */
	DIK_F9,				0x65,			/* PF9 */
	DIK_F10,			0x66,			/* PF10 */

	DIK_KANJI,			0x01,			/* ESC(���p/�S�p) */
	DIK_1,				0x02,			/* 1 */
	DIK_2,				0x03,			/* 2 */
	DIK_3,				0x04,			/* 3 */
	DIK_4,				0x05,			/* 4 */
	DIK_5,				0x06,			/* 5 */
	DIK_6,				0x07,			/* 6 */
	DIK_7,				0x08,			/* 7 */
	DIK_8,				0x09,			/* 8 */
	DIK_9,				0x0a,			/* 9 */
	DIK_0,				0x0b,			/* 0 */
	DIK_MINUS,			0x0c,			/* - */
	DIK_CIRCUMFLEX,		0x0d,			/* ^ */
	DIK_YEN,			0x0e,			/* \ */
	DIK_BACK,			0x0f,			/* BS */

	DIK_TAB,			0x10,			/* TAB */
	DIK_Q,				0x11,			/* Q */
	DIK_W,				0x12,			/* W */
	DIK_E,				0x13,			/* E */
	DIK_R,				0x14,			/* R */
	DIK_T,				0x15,			/* T */
	DIK_Y,				0x16,			/* Y */
	DIK_U,				0x17,			/* U */
	DIK_I,				0x18,			/* I */
	DIK_O,				0x19,			/* O */
	DIK_P,				0x1a,			/* P */
	DIK_AT,				0x1b,			/* @ */
	DIK_LBRACKET,		0x1c,			/* [ */
	DIK_RETURN,			0x1d,			/* CR */

	DIK_LCONTROL,		0x52,			/* CTRL(��Ctrl) */
	DIK_A,				0x1e,			/* A */
	DIK_S,				0x1f,			/* S */
	DIK_D,				0x20,			/* D */
	DIK_F,				0x21,			/* F */
	DIK_G,				0x22,			/* G */
	DIK_H,				0x23,			/* H */
	DIK_J,				0x24,			/* J */
	DIK_K,				0x25,			/* K */
	DIK_L,				0x26,			/* L */
	DIK_SEMICOLON,		0x27,			/* ; */
	DIK_COLON,			0x28,			/* : */
	DIK_RBRACKET,		0x29,			/* ] */

	DIK_LSHIFT,			0x53,			/* ��SHIFT */
	DIK_Z,				0x2a,			/* Z */
	DIK_X,				0x2b,			/* X */
	DIK_C,				0x2c,			/* C */
	DIK_V,				0x2d,			/* V */
	DIK_B,				0x2e,			/* B */
	DIK_N,				0x2f,			/* N */
	DIK_M,				0x30,			/* M */
	DIK_COMMA,			0x31,			/* , */
	DIK_PERIOD,			0x32,			/* . */
	DIK_SLASH,			0x33,			/* / */
	DIK_BACKSLASH,		0x34,			/* _ */
	DIK_RSHIFT,			0x54,			/* �ESHIFT */

	DIK_CAPITAL,		0x55,			/* CAP(Caps Lock) */
	DIK_NOCONVERT,		0x56,			/* GRAPH(���ϊ�) */
	DIK_CONVERT,		0x57,			/* ��SPACE(�ϊ�) */
	DIK_KANA,			0x58,			/* ��SPACE(�J�^�J�i) */
	DIK_SPACE,			0x35,			/* �ESPACE(SPACE) */
	DIK_RCONTROL,		0x5a,			/* ����(�ECtrl) */

	DIK_INSERT,			0x48,			/* INS(Insert) */
	DIK_DELETE,			0x4b,			/* DEL(Delete) */
	DIK_UP,				0x4d,			/* �� */
	DIK_LEFT,			0x4f,			/* �� */
	DIK_DOWN,			0x50,			/* �� */
	DIK_RIGHT,			0x51,			/* �� */

	DIK_HOME,			0x49,			/* EL(Home) */
	DIK_PRIOR,			0x4a,			/* CLS(Page Up) */
	DIK_END,			0x4c,			/* DUP(End) */
	DIK_NEXT,			0x4e,			/* HOME(Page Down) */

	DIK_MULTIPLY,		0x36,			/* Tenkey * */
	DIK_DIVIDE,			0x37,			/* Tenkey / */
	DIK_ADD,			0x38,			/* Tenkey + */
	DIK_SUBTRACT,		0x39,			/* Tenkey - */
	DIK_NUMPAD7,		0x3a,			/* Tenkey 7 */
	DIK_NUMPAD8,		0x3b,			/* Tenkey 8 */
	DIK_NUMPAD9,		0x3c,			/* Tenkey 9 */
	DIK_NUMPAD4,		0x3e,			/* Tenkey 4 */
	DIK_NUMPAD5,		0x3f,			/* Tenkey 5 */
	DIK_NUMPAD6,		0x40,			/* Tenkey 6 */
	DIK_NUMPAD1,		0x42,			/* Tenkey 1 */
	DIK_NUMPAD2,		0x43,			/* Tenkey 2 */
	DIK_NUMPAD3,		0x44,			/* Tenkey 3 */
	DIK_NUMPAD0,		0x46,			/* Tenkey 0 */
	DIK_DECIMAL,		0x47,			/* Tenkey . */
	DIK_NUMPADENTER,	0x45			/* Tenkey CR */
};

/*
 *	DirectInput�R�[�h��FM-7 �����R�[�h
 *	�R�[�h�Ώƕ\(NEC PC-98�L�[�{�[�h�p)
 */
static BYTE kbd_98_table[] = {
	DIK_SYSRQ,			0x5c,			/* BREAK(COPY) */
	DIK_F1,				0x5d,			/* PF1 */
	DIK_F2,				0x5e,			/* PF2 */
	DIK_F3,				0x5f,			/* PF3 */
	DIK_F4,				0x60,			/* PF4 */
	DIK_F5,				0x61,			/* PF5 */
	DIK_F6,				0x62,			/* PF6 */
	DIK_F7,				0x63,			/* PF7 */
	DIK_F8,				0x64,			/* PF8 */
	DIK_F9,				0x65,			/* PF9 */
	DIK_F10,			0x66,			/* PF10 */

	DIK_ESCAPE,			0x01,			/* ESC */
	DIK_1,				0x02,			/* 1 */
	DIK_2,				0x03,			/* 2 */
	DIK_3,				0x04,			/* 3 */
	DIK_4,				0x05,			/* 4 */
	DIK_5,				0x06,			/* 5 */
	DIK_6,				0x07,			/* 6 */
	DIK_7,				0x08,			/* 7 */
	DIK_8,				0x09,			/* 8 */
	DIK_9,				0x0a,			/* 9 */
	DIK_0,				0x0b,			/* 0 */
	DIK_MINUS,			0x0c,			/* - */
	DIK_CIRCUMFLEX,		0x0d,			/* ^ */
	DIK_YEN,			0x0e,			/* \ */
	DIK_BACK,			0x0f,			/* BS */

	DIK_TAB,			0x10,			/* TAB */
	DIK_Q,				0x11,			/* Q */
	DIK_W,				0x12,			/* W */
	DIK_E,				0x13,			/* E */
	DIK_R,				0x14,			/* R */
	DIK_T,				0x15,			/* T */
	DIK_Y,				0x16,			/* Y */
	DIK_U,				0x17,			/* U */
	DIK_I,				0x18,			/* I */
	DIK_O,				0x19,			/* O */
	DIK_P,				0x1a,			/* P */
	DIK_AT,				0x1b,			/* @ */
	DIK_LBRACKET,		0x1c,			/* [ */
	DIK_RETURN,			0x1d,			/* CR */

	DIK_LCONTROL,		0x52,			/* CTRL(��Ctrl) */
	DIK_A,				0x1e,			/* A */
	DIK_S,				0x1f,			/* S */
	DIK_D,				0x20,			/* D */
	DIK_F,				0x21,			/* F */
	DIK_G,				0x22,			/* G */
	DIK_H,				0x23,			/* H */
	DIK_J,				0x24,			/* J */
	DIK_K,				0x25,			/* K */
	DIK_L,				0x26,			/* L */
	DIK_SEMICOLON,		0x27,			/* ; */
	DIK_COLON,			0x28,			/* : */
	DIK_RBRACKET,		0x29,			/* ] */

	DIK_LSHIFT,			0x53,			/* ��SHIFT(SHIFT) */
	DIK_Z,				0x2a,			/* Z */
	DIK_X,				0x2b,			/* X */
	DIK_C,				0x2c,			/* C */
	DIK_V,				0x2d,			/* V */
	DIK_B,				0x2e,			/* B */
	DIK_N,				0x2f,			/* N */
	DIK_M,				0x30,			/* M */
	DIK_COMMA,			0x31,			/* , */
	DIK_PERIOD,			0x32,			/* . */
	DIK_SLASH,			0x33,			/* / */
	DIK_UNDERLINE,		0x34,			/* _ */
	DIK_RSHIFT,			0x54,			/* �ESHIFT(SHIFT) */

	DIK_CAPITAL,		0x55,			/* CAP(CAPS) */
	DIK_NOCONVERT,		0x56,			/* GRAPH(NFER) */
	/* ��SPACE ���蓖�ĂȂ� */
	DIK_SPACE,			0x35,			/* �ESPACE(SPACE) */
	DIK_KANJI,			0x58,			/* ��SPACE(XFER) */
	DIK_KANA,			0x5a,			/* ����(�J�i) */

	DIK_INSERT,			0x48,			/* INS(Insert) */
	DIK_DELETE,			0x4b,			/* DEL(Delete) */
	DIK_UP,				0x4d,			/* �� */
	DIK_LEFT,			0x4f,			/* �� */
	DIK_DOWN,			0x50,			/* �� */
	DIK_RIGHT,			0x51,			/* �� */

	DIK_HOME,			0x49,			/* EL(HOME CLR) */
	DIK_PRIOR,			0x4a,			/* CLS(ROLL DOWN) */
	DIK_END,			0x4c,			/* DUP(HELP) */
	DIK_NEXT,			0x4e,			/* HOME(ROLL UP) */

	DIK_MULTIPLY,		0x36,			/* Tenkey * */
	DIK_DIVIDE,			0x37,			/* Tenkey / */
	DIK_ADD,			0x38,			/* Tenkey + */
	DIK_SUBTRACT,		0x39,			/* Tenkey - */
	DIK_NUMPAD7,		0x3a,			/* Tenkey 7 */
	DIK_NUMPAD8,		0x3b,			/* Tenkey 8 */
	DIK_NUMPAD9,		0x3c,			/* Tenkey 9 */
	DIK_NUMPADEQUALS,	0x3d,			/* Tenkey = */
	DIK_NUMPAD4,		0x3e,			/* Tenkey 4 */
	DIK_NUMPAD5,		0x3f,			/* Tenkey 5 */
	DIK_NUMPAD6,		0x40,			/* Tenkey 6 */
	DIK_NUMPADCOMMA,	0x41,			/* Tenkey , */
	DIK_NUMPAD1,		0x42,			/* Tenkey 1 */
	DIK_NUMPAD2,		0x43,			/* Tenkey 2 */
	DIK_NUMPAD3,		0x44,			/* Tenkey 3 */
	DIK_NUMPAD0,		0x46,			/* Tenkey 0 */
	DIK_DECIMAL,		0x47,			/* Tenkey . */
	DIK_NUMPADENTER,	0x45			/* Tenkey CR */
};

/*
 *	DirectInput�R�[�h��FM-7 �����R�[�h
 *	�R�[�h�Ώƕ\(101�L�[�{�[�h�p)
 */
static BYTE kbd_101_table[] = {
	DIK_ESCAPE,			0x5c,			/* BREAK(ESC) */
	DIK_F1,				0x5d,			/* PF1 */
	DIK_F2,				0x5e,			/* PF2 */
	DIK_F3,				0x5f,			/* PF3 */
	DIK_F4,				0x60,			/* PF4 */
	DIK_F5,				0x61,			/* PF5 */
	DIK_F6,				0x62,			/* PF6 */
	DIK_F7,				0x63,			/* PF7 */
	DIK_F8,				0x64,			/* PF8 */
	DIK_F9,				0x65,			/* PF9 */
	DIK_F10,			0x66,			/* PF10 */

	DIK_GRAVE,			0x01,			/* ESC(`) */
	DIK_1,				0x02,			/* 1 */
	DIK_2,				0x03,			/* 2 */
	DIK_3,				0x04,			/* 3 */
	DIK_4,				0x05,			/* 4 */
	DIK_5,				0x06,			/* 5 */
	DIK_6,				0x07,			/* 6 */
	DIK_7,				0x08,			/* 7 */
	DIK_8,				0x09,			/* 8 */
	DIK_9,				0x0a,			/* 9 */
	DIK_0,				0x0b,			/* 0 */
	DIK_MINUS,			0x0c,			/* - */
	DIK_EQUALS,			0x0d,			/* ^(=) */
	DIK_BACKSLASH,		0x0e,			/* \ */
	DIK_BACK,			0x0f,			/* BS */

	DIK_TAB,			0x10,			/* TAB */
	DIK_Q,				0x11,			/* Q */
	DIK_W,				0x12,			/* W */
	DIK_E,				0x13,			/* E */
	DIK_R,				0x14,			/* R */
	DIK_T,				0x15,			/* T */
	DIK_Y,				0x16,			/* Y */
	DIK_U,				0x17,			/* U */
	DIK_I,				0x18,			/* I */
	DIK_O,				0x19,			/* O */
	DIK_P,				0x1a,			/* P */
	DIK_LBRACKET,		0x1b,			/* @([) */
	DIK_RBRACKET,		0x1c,			/* [(]) */
	DIK_RETURN,			0x1d,			/* CR */

	DIK_LCONTROL,		0x52,			/* CTRL(��Ctrl) */
	DIK_A,				0x1e,			/* A */
	DIK_S,				0x1f,			/* S */
	DIK_D,				0x20,			/* D */
	DIK_F,				0x21,			/* F */
	DIK_G,				0x22,			/* G */
	DIK_H,				0x23,			/* H */
	DIK_J,				0x24,			/* J */
	DIK_K,				0x25,			/* K */
	DIK_L,				0x26,			/* L */
	DIK_SEMICOLON,		0x27,			/* ; */
	DIK_APOSTROPHE,		0x28,			/* :(') */
	/* ] ���蓖�ĂȂ� */

	DIK_LSHIFT,			0x53,			/* ��SHIFT */
	DIK_Z,				0x2a,			/* Z */
	DIK_X,				0x2b,			/* X */
	DIK_C,				0x2c,			/* C */
	DIK_V,				0x2d,			/* V */
	DIK_B,				0x2e,			/* B */
	DIK_N,				0x2f,			/* N */
	DIK_M,				0x30,			/* M */
	DIK_COMMA,			0x31,			/* , */
	DIK_PERIOD,			0x32,			/* . */
	DIK_SLASH,			0x33,			/* / */
	/* _ ���蓖�ĂȂ� */
	DIK_RSHIFT,			0x54,			/* �ESHIFT */

	DIK_CAPITAL,		0x55,			/* CAP(Caps Lock) */
	DIK_NUMLOCK,		0x56,			/* GRAPH(Num Lock) */
	/* ���X�y�[�X���蓖�ĂȂ� */
	/* ���X�y�[�X���蓖�ĂȂ� */
	DIK_SPACE,			0x35,			/* �ESPACE(SPACE) */
	DIK_RCONTROL,		0x5a,			/* ����(�ECtrl) */

	DIK_INSERT,			0x48,			/* INS(Insert) */
	DIK_DELETE,			0x4b,			/* DEL(Delete) */
	DIK_UP,				0x4d,			/* �� */
	DIK_LEFT,			0x4f,			/* �� */
	DIK_DOWN,			0x50,			/* �� */
	DIK_RIGHT,			0x51,			/* �� */

	DIK_HOME,			0x49,			/* EL(Home) */
	DIK_PRIOR,			0x4a,			/* CLS(Page Up) */
	DIK_END,			0x4c,			/* DUP(End) */
	DIK_NEXT,			0x4e,			/* HOME(Page Down) */

	DIK_MULTIPLY,		0x36,			/* Tenkey * */
	DIK_DIVIDE,			0x37,			/* Tenkey / */
	DIK_ADD,			0x38,			/* Tenkey + */
	DIK_SUBTRACT,		0x39,			/* Tenkey - */
	DIK_NUMPAD7,		0x3a,			/* Tenkey 7 */
	DIK_NUMPAD8,		0x3b,			/* Tenkey 8 */
	DIK_NUMPAD9,		0x3c,			/* Tenkey 9 */
	DIK_NUMPAD4,		0x3e,			/* Tenkey 4 */
	DIK_NUMPAD5,		0x3f,			/* Tenkey 5 */
	DIK_NUMPAD6,		0x40,			/* Tenkey 6 */
	DIK_NUMPAD1,		0x42,			/* Tenkey 1 */
	DIK_NUMPAD2,		0x43,			/* Tenkey 2 */
	DIK_NUMPAD3,		0x44,			/* Tenkey 3 */
	DIK_NUMPAD0,		0x46,			/* Tenkey 0 */
	DIK_DECIMAL,		0x47,			/* Tenkey . */
	DIK_NUMPADENTER,	0x45			/* Tenkey CR */
};

/*
 *	WinNT�n�L�[���͑΍�
 *	�R�[�h�e�[�u��
 */
static const BYTE DI_keyno[] = { DIK_KANJI, DIK_KANA, DIK_CAPITAL };

/*
 *	�J�[�\���L�[ �� �e���L�[�ϊ�
 *	�����R�[�h�Ώƕ\
 */
static BYTE TenDirTable[16] = {
	0x00, 0x3b, 0x43, 0xff, 0x3e, 0x3a, 0x42, 0x3e,
	0x40, 0x3c, 0x44, 0x40, 0xff, 0x3b, 0x43, 0xff
};


/*
 *	�L�[�{�[�h
 *	�f�t�H���g�}�b�v�擾
 */
void FASTCALL GetDefMapKbd(BYTE *pMap, int mode)
{
	int i;
	int type;

	ASSERT(pMap);
	ASSERT((mode >= 0) && (mode <= 3));

	/* ��x�A���ׂăN���A */
	memset(pMap, 0, 256);

	/* �L�[�{�[�h�^�C�v�擾 */
	switch (mode) {
		/* �������� */
		case 0:
			type = GetKeyboardType(1);
			if ((type & 0xf00) == 0xd00) {
				type = 0xd00;
			}
			else {
				if (type != 1) {
					if (GetKeyboardType(0) == 7) {
						type = 0;
					}
					else {
						type = 1;
					}
				}
			}
			break;
		/* 106 */
		case 1:
			type = 0;
			break;
		/* PC-98 */
		case 2:
			type = 0xd00;
			break;
		/* 101 */
		case 3:
			type = 1;
			break;
		/* ���̑� */
		default:
			ASSERT(FALSE);
			break;
	}

	/* �ϊ��e�[�u���Ƀf�t�H���g�l���Z�b�g */
	if (type & 0xd00) {
		/* PC-98 */
		for (i=0; i<sizeof(kbd_98_table)/2; i++) {
			pMap[kbd_98_table[i * 2]] = kbd_98_table[i * 2 + 1];
		}
		return;
	}

	if (type == 0) {
		/* 106 */
		for (i=0; i<sizeof(kbd_106_table)/2; i++) {
			pMap[kbd_106_table[i * 2]] = kbd_106_table[i * 2 + 1];
		}
	}
	else {
		/* 101 */
		for (i=0; i<sizeof(kbd_101_table)/2; i++) {
			pMap[kbd_101_table[i * 2]] = kbd_101_table[i * 2 + 1];
		}
	}
}

/*
 *	�L�[�{�[�h
 *	�}�b�v�ݒ�
 */
void FASTCALL SetMapKbd(BYTE *pMap)
{
	ASSERT(pMap);

	/* �R�s�[���邾�� */
	memcpy(kbd_table, pMap, sizeof(kbd_table));

	/* NT/PC-9801�΍� : �΍�t���O(bit7)��L���ɂ��� */
	if (bNTkbMode) {
		kbd_table[DIK_KANJI] |= 0x80;
		kbd_table[DIK_CAPITAL] |= 0x80;
		kbd_table[DIK_KANA] |= 0x80;
	}
}

/*
 *	������
 */
void FASTCALL InitKbd(void)
{
	BYTE tmp_kbdmap[256];

	/* NT�΍􏈗� */
	if (bNTflag || (GetKeyboardType(1) & 0xd00)) {
		bNTkbMode = TRUE;
	}
	else {
		bNTkbMode = FALSE;
	}

	/* ���[�N�G���A������(�L�[�{�[�h) */
	lpdi = NULL;
	lpkbd = NULL;
	memset(kbd_map, 0, sizeof(kbd_map));
	memset(kbd_table, 0, sizeof(kbd_table));
	keytime = 0;

	/* ���[�N�G���A������(�L�[�{�[�h NT/PC-9801�΍�) */
	memset(bNTkeyPushFlag, 0, sizeof(bNTkeyPushFlag));
	memset(bNTkeyMakeFlag, 0, sizeof(bNTkeyMakeFlag));
	bDummyRead = TRUE;
	dmyreadtime = 0;

	/* ���[�N�G���A������(�����L�[�o�b�t�@/�^�����A���^�C���L�[�X�L����) */
	memset(nKeyBuffer, 0, sizeof(nKeyBuffer));
	nKeyReadPtr = 0;
	nKeyWritePtr = 0;
	nLastKey = 0;
	nLastKey2 = 0;
	bKbdReal = FALSE;

	/* ���[�N�G���A������(�e���L�[�ϊ�) */
	bArrow8Dir = TRUE;
	nTenDir = 0;
	nTenDir2 = 0;

	/* ���[�N�G���A������(�W���C�X�e�B�b�N) */
	joytime = 0;
	joytime2 = 0;
	memset(joydat, 0, sizeof(joydat));
	memset(joybk, 0, sizeof(joybk));
	memset(joyrapid, 0, sizeof(joyrapid));
	joyplugged[0] = TRUE;
	joyplugged[1] = TRUE;

#if defined(MOUSE)
	/* ���[�N�G���A������(�}�E�X) */
	mostime = 0;
	bCapture = FALSE;
	nMouseX = 0;
	nMouseY = 0;
	bMouseCursor = TRUE;
	nMouseButton = 0xf0;
	uMidBtnMode = MOSCAP_NONE;
#endif

	/* �f�t�H���g�}�b�v��ݒ� */
	GetDefMapKbd(tmp_kbdmap, 0);
	SetMapKbd(tmp_kbdmap);

	/* �e���L�[�G�~�����[�V���� */
	bTenCursor = FALSE;

#if defined(KBDPASTE)
	/* �L�[�y�[�X�g�f�[�^ */
	strPaste = NULL;
	strPtr = NULL;
	cPreKey = 0;
	uWaitCnt = 0;
	uPasteWait = 0;
	uPasteWaitCntl = 0;
#endif
}

/*
 *	�N���[���A�b�v
 */
void FASTCALL CleanKbd(void)
{
	/* DirectInputDevice(�L�[�{�[�h)����� */
	if (lpkbd) {
		lpkbd->Unacquire();
		lpkbd->Release();
		lpkbd = NULL;
	}

#if defined(MOUSE)
	/* DirectInputDevice(�}�E�X)����� */
	if (lpmos) {
		lpmos->Unacquire();
		lpmos->Release();
		lpmos = NULL;
	}
#endif

	/* DirectInput����� */
	if (lpdi) {
		lpdi->Release();
		lpdi = NULL;
	}

#if defined(KBDPASTE)
	if (strPaste) {
		free(strPaste);
		strPaste = NULL;
		strPtr = NULL;
		uWaitCnt = 0;
	}
#endif
}

/*
 *	�Z���N�g
 */
BOOL FASTCALL SelectKbd(HWND hWnd)
{
	DIDEVCAPS caps;

	ASSERT(hWnd);

#ifdef DINPUT8
	/* DirectInput�I�u�W�F�N�g���쐬 */
	if (FAILED(DirectInput8Create(hAppInstance, DIRECTINPUT_VERSION,
							IID_IDirectInput8A, (void**)&lpdi, NULL))) {
		return FALSE;
	}
#else
	/* DirectInput�I�u�W�F�N�g���쐬 */
	if (FAILED(DirectInputCreate(hAppInstance, DIRECTINPUT_VERSION,
							&lpdi, NULL))) {
		return FALSE;
	}
#endif

	/* �L�[�{�[�h�f�o�C�X���쐬 */
	if (FAILED(lpdi->CreateDevice(GUID_SysKeyboard, &lpkbd, NULL))) {
		return FALSE;
	}

	/* �L�[�{�[�h�f�[�^�`����ݒ� */
	if (FAILED(lpkbd->SetDataFormat(&c_dfDIKeyboard))) {
		return FALSE;
	}

	/* �������x����ݒ� */
	if (FAILED(lpkbd->SetCooperativeLevel(hWnd,
						DISCL_BACKGROUND | DISCL_NONEXCLUSIVE))) {
		return FALSE;
	}

#if defined(MOUSE)
	/* �}�E�X�f�o�C�X���쐬 */
	bDetectMouse = FALSE;
	if (SUCCEEDED(lpdi->CreateDevice(GUID_SysMouse, &lpmos, NULL))) {
		/* �}�E�X�f�[�^�`����ݒ� */
		if (SUCCEEDED(lpmos->SetDataFormat(&c_dfDIMouse))) {
			/* �������x����ݒ� */
			if (SUCCEEDED(lpmos->SetCooperativeLevel(hWnd,
						DISCL_BACKGROUND | DISCL_NONEXCLUSIVE))) {
				/* �}�E�X�{�^�������擾 */
				caps.dwSize = sizeof(DIDEVCAPS);
				if (SUCCEEDED(lpmos->GetCapabilities(&caps))) {
					nMouseButtons = (BYTE)caps.dwButtons;
					bDetectMouse = TRUE;
				}
			}
		}
	}
#endif

	return TRUE;
}

/*
 *	�L�[�{�[�h
 *	�����L�[�o�b�t�@�o�^
 */
static void FASTCALL PushKeyCode(BYTE code)
{
	int i;

	if (nKeyWritePtr < KEYBUFFER_SIZE) {
		nKeyBuffer[nKeyWritePtr++] = code;
	}
	else {
		for (i=0; i<KEYBUFFER_SIZE; i++) {
			nKeyBuffer[i] = nKeyBuffer[i + 1];
		}
		nKeyBuffer[KEYBUFFER_SIZE - 1] = code;

		if (nKeyReadPtr > 0) {
			nKeyReadPtr --;
		}
	}
}

/*
 *	�L�[�{�[�h �e���L�[�ϊ� 
 *	�����R�[�h�ϊ�
 */
static BYTE FASTCALL Cur2Ten_DirCode(BYTE code)
{
	switch (code) {
		case 0x4d:	/* �� */
			return 0x01;
		case 0x4f:	/* �� */
			return 0x04;
		case 0x50:	/* �� */
			return 0x02;
		case 0x51:	/* �� */
			return 0x08;
	}

	return 0;
}

/*
 *	�L�[�{�[�h �e���L�[�ϊ� 
 *	�}�X�N�R�[�h�ϊ�
 */
static BYTE FASTCALL Cur2Ten_MaskCode(BYTE code)
{
	switch (code) {
		case 0x4d:	/* �� */
		case 0x50:	/* �� */
			return 0xfc;
		case 0x4f:	/* �� */
		case 0x51:	/* �� */
			return 0xf3;
	}

	return 0xff;
}

/*
 *	�L�[�{�[�h
 *	�J�[�\���L�[���e���L�[�ϊ� Make
 */
static BOOL FASTCALL Cur2Ten_Make(BYTE code)
{
	BYTE nTenDirOld;
	BYTE dircode;

	/* �ȑO�̕����R�[�h��ۑ� */
	nTenDirOld = nTenDir;

	/* �L�[�R�[�h�ϊ� */
	dircode = Cur2Ten_DirCode(code);
	if (dircode) {
		if (bArrow8Dir) {
			/* 8���� */
			nTenDir2 = (BYTE)(nTenDir & (BYTE)~Cur2Ten_MaskCode(code));
			nTenDir &= Cur2Ten_MaskCode(code);
			nTenDir |= dircode;
		}
		else {
			if (key_format == KEY_FORMAT_SCAN) {
				if (TenDirTable[dircode]) {
					keyboard_make(TenDirTable[dircode]);
				}
				return TRUE;
			}

			/* 4���� */
			nTenDir2 = (BYTE)(nTenDir & (BYTE)Cur2Ten_MaskCode(code));
			nTenDir = dircode;
		}

		/* �e�[�u�����e��0xff�̏ꍇ�A�����ȑg�ݍ��킹 */
		if (TenDirTable[nTenDir] == 0xff) {
			nTenDir = nTenDirOld;
			return TRUE;
		}

		/* �L�[�R�[�h���s */
		if (nTenDir != nTenDirOld) {
			if (TenDirTable[nTenDirOld]) {
				keyboard_break(TenDirTable[nTenDirOld]);
				PushKeyCode(TenDirTable[nTenDir]);
			}
			else {
				keyboard_make(TenDirTable[nTenDir]);
			}
		}
		return TRUE;
	}

	return FALSE;
}

/*
 *	�L�[�{�[�h
 *	�J�[�\���L�[���e���L�[�ϊ� Break
 */
static BOOL FASTCALL Cur2Ten_Break(BYTE code)
{
	BYTE nTenDirOld;
	BYTE dircode;

	/* �ȑO�̕����R�[�h��ۑ� */
	nTenDirOld = nTenDir;

	/* �L�[�R�[�h�ϊ� */
	dircode = Cur2Ten_DirCode(code);
	if (dircode) {
		if ((key_format == KEY_FORMAT_SCAN) && !bArrow8Dir) {
			if (TenDirTable[dircode]) {
				keyboard_break(TenDirTable[dircode]);
			}
			return TRUE;
		}

		nTenDir2 &= (BYTE)~dircode;
		if (nTenDir2) {
			nTenDir |= nTenDir2;
			nTenDir2 = 0;
		}
		nTenDir &= (BYTE)~dircode;

		/* �e�[�u�����e��0xff�̏ꍇ�A�����ȑg�ݍ��킹 */
		if (TenDirTable[nTenDir] == 0xff) {
			nTenDir = nTenDirOld;
			return TRUE;
		}

		/* �L�[�R�[�h���s */
		if (nTenDir != nTenDirOld) {
			keyboard_break(TenDirTable[nTenDirOld]);
			if (TenDirTable[nTenDir]) {
				PushKeyCode(TenDirTable[nTenDir]);
			}
			else if (key_format != KEY_FORMAT_SCAN) {
				/* ��~����"5"�𔭍s */
				PushKeyCode(0x3f);
				PushKeyCode(0xbf);
			}
		}
		return TRUE;
	}

	return FALSE;
}

/*
 *	�L�[�{�[�h
 *	���j���[�����o���^�C�}�ݒ�
 */
void FASTCALL SetMenuExitTimer(void)
{
	bMenuExit = TRUE;
	menuexittime = dwExecTotal;
}

/*
 *	�L�[�{�[�h
 *	�|�[�����O
 */
void FASTCALL PollKbd(void)
{
	HRESULT hr;
	BYTE buf[256];
	int i;
	BYTE fm7;
	BOOL bFlag;
	static BYTE key_last = 0;

#if defined(KBDPASTE)
	/* �����ŃJ�E���g */
	if (uWaitCnt < (unsigned int)-1) {
		uWaitCnt ++;
	}
#endif

	/* DirectInput�`�F�b�N */
	if (!lpkbd) {
		return;
	}

	/* ���j���[�����o���`�F�b�N */
	if (bMenuExit) {
		if ((dwExecTotal - menuexittime) < 300000) {
			return;
		}
		bMenuExit = FALSE;
	}

	/* �ŏ����`�F�b�N */
	if (bMinimize) {
		return;
	}

	/* �A�N�e�B�x�[�g�`�F�b�N */
	if (!bActivate) {
		/* ��A�N�e�B�u���̓_�~�[���[�h�^�C�}�X�V */
		bDummyRead = TRUE;
		dmyreadtime = dwExecTotal;
		return;
	}

	/* �_�~�[���[�h���ԃ`�F�b�N */
	if (bDummyRead && ((dwExecTotal - dmyreadtime) > 100000)) {
		bDummyRead = FALSE;
	}

	/* ���ԃ`�F�b�N */
	if ((dwExecTotal - keytime) < 20000) {
		return;
	}
	keytime = dwExecTotal;

#if defined(KBDPASTE)
	/* �y�[�X�g�o�b�t�@���Ƀf�[�^�����܂��Ă���ꍇ�A��ɏ��� */
	if (strPtr) {
		/* �X�L�������[�h���͑ΏۊO�Ƃ��� */
		if (key_format == KEY_FORMAT_SCAN) {
			/* �̂Ă܂��I */
			free(strPaste);
			strPaste = NULL;
			strPtr = NULL;
			uWaitCnt = 0;
		}
		else if (*strPtr) {
			if (((cPreKey & 0xe0) == 0) || (cPreKey == 0x7f)) {
				if (uWaitCnt - 1 >= uPasteWaitCntl) {
					if (keyboard_paste(*strPtr)) {
						/* �y�[�X�g�����̂Ń|�C���^�������߂� */
						cPreKey = (*strPtr);
						strPtr++;
						uWaitCnt = 0;
					}
				}
			}
			else {
				if (uWaitCnt - 1 >= uPasteWait) {
					if (keyboard_paste(*strPtr)) {
						/* �y�[�X�g�����̂Ń|�C���^�������߂� */
						cPreKey = (*strPtr);
						strPtr++;
						uWaitCnt = 0;
					}
				}
			}
			/* �y�[�X�g�������͑��̃L�[�����͂��Ȃ� */
			return;
		}
		else {
			/* �y�[�X�g�I������̂ŉ�� */
			free(strPaste);
			strPaste = NULL;
			strPtr = NULL;
			uWaitCnt = 0;
		}
	}
#endif

	/* �L�[�o�b�t�@���ɃR�[�h�����܂��Ă���ꍇ�A��ɏ��� */
	if (nKeyReadPtr < nKeyWritePtr) {
		fm7 = nKeyBuffer[nKeyReadPtr++];
		if (fm7 == 0xff) {
			/* 0xff : �L�[���s�[�g�^�C�}�ύX */
			keyboard_repeat();
		}
		else if (fm7 & 0x80) {
			/* 0x80-0xfe : Break�R�[�h */
			keyboard_break((BYTE)(fm7 & 0x7f));
		}
		else {
			/* 0x00-0x7f : Make�R�[�h */
			keyboard_make(fm7);
		}

		return;
	}
	else {
		nKeyReadPtr = 0;
		nKeyWritePtr = 0;
	}

	/* �f�o�C�X�̃A�N�Z�X�����擾(����擾���Ă��悢) */
	hr = lpkbd->Acquire();
	if ((hr != DI_OK) && (hr != S_FALSE)) {
		return;
	}

	/* �f�o�C�X��Ԃ��擾 */
	if (lpkbd->GetDeviceState(sizeof(buf), buf) != DI_OK) {
		return;
	}

	/* NT/PC-9801�΍􏈗� : �L�[�������̃t���O���] */
	if (bNTkbMode) {
		for (i = 0; i < 3; i ++) {
			buf[DI_keyno[i]] = kbd_map[DI_keyno[i]];
			if (bNTkeyPushFlag[i]) {
				buf[DI_keyno[i]] ^= (BYTE)0x80;
				bNTkeyPushFlag[i] = FALSE;
			}
		}
	}

	/* Vista/Windows7 �ESHIFT�΍􏈗� */
#ifndef DINPUT8
	if (bVistaflag) {
		if (bNTkeyPushFlag[KNT_RSHIFT]) {
			buf[DIK_RSHIFT] |= 0x80;
		}
		else {
			buf[DIK_RSHIFT] &= (BYTE)~0x80;
		}
	}
#endif

	/* �t���Ooff */
	bFlag = FALSE;

	/* ���܂ł̏�ԂƔ�r���āA���ɕϊ����� */
	for (i=0; i<sizeof(buf); i++) {
		if (((buf[i] & 0x80) != (kbd_map[i] & 0x80)) && (!bFlag)) {
			if ((buf[i] & 0x80) || (kbd_table[i] & 0x80)) {
				/* �L�[���� */
				fm7 = (BYTE)(kbd_table[i] & 0x7f);

				/* ALT�L�[�Ƃ̓��������͖��� */
				if ((!kbd_table[DIK_LMENU]) && (buf[DIK_LMENU] & 0x80)) {
					fm7 = 0;
				}
				if ((!kbd_table[DIK_RMENU]) && (buf[DIK_RMENU] & 0x80)) {
					fm7 = 0;
				}

				if (fm7 > 0) {
					if (!bMenuLoop) {
						if (!(bDummyRead && (kbd_table[i] & 0x80))) {

							if ((fm7 >= 0x4d) && (fm7 <= 0x51) && bTenCursor) {
								/* �X�L�������[�h���͖{���̃R�[�h�����s */
								if ((key_format == KEY_FORMAT_SCAN) && (fm7 != 0x4e)) {
									PushKeyCode(fm7);
								}

								/* �X�L�����R�[�h�ϊ� */
								if (Cur2Ten_Make(fm7)) {
									fm7 = 0;
								}
							}

							/* NT/PC-9801�΍� : �L�[Break�p�^�C�}�o�^ */
							if (kbd_table[i] & 0x80) {
								if (bNTkeyMakeFlag[kbd_table[i] & 0x7f]) {
									KillTimer(hMainWnd, kbd_table[i]);
									if ((key_format != KEY_FORMAT_SCAN) && (fm7 != 0x5c)) {
										keyboard_break(fm7);
										PushKeyCode(fm7);
									}
								}
								else {
									keyboard_make(fm7);
								}
								bNTkeyMakeFlag[kbd_table[i] & 0x7f] = TRUE;
								SetTimer(hMainWnd, kbd_table[i], 100, NULL);
							}
							else if (fm7) {
								keyboard_make(fm7);
							}
							bDummyRead = FALSE;

							/* �^�����A���^�C���L�[�X�L���� */
							if (bKbdReal && (key_format != KEY_FORMAT_SCAN)) {
								if ((fm7 >= 0x3a) && (fm7 <= 0x46)) {
									/* �ȑO������Ă����L�[������΍Ĕ��s */
									if (nLastKey && key_repeat_flag) {
										PushKeyCode(nLastKey);
									}
									PushKeyCode(0xff);

									/* �Ĕ��s�p�ɃL�[�R�[�h���L�� */
									if (fm7 == 0x3f) {
										nLastKey2 = 0;
									}
									else {
										nLastKey2 = fm7;
									}
								}
								else {
									/* �Ĕ��s�p�ɃL�[�R�[�h���L�� */
									if (fm7 != 0x5c) {
										nLastKey = fm7;
									}
									else {
										nLastKey = 0;
									}
								}
							}
						}

						if (fm7 != 0x5c) {
							key_last = fm7;
						}

						bFlag = TRUE;
					}
				}
			}
			else {
				/* �L�[������ */
				fm7 = (BYTE)(kbd_table[i] & 0x7f);
				if (fm7 > 0) {
					if ((fm7 >= 0x4d) && (fm7 <= 0x51) && bTenCursor) {
						/* �X�L�������[�h���͖{���̃R�[�h�����s */
						if ((key_format == KEY_FORMAT_SCAN) && (fm7 != 0x4e)) {
							PushKeyCode((BYTE)(fm7 | 0x80));
						}

						/* �X�L�����R�[�h�ϊ� */
						if (Cur2Ten_Break(fm7)) {
							fm7 = 0;
						}
					}

					/* �^�����A���^�C���L�[�X�L���� */
					if (bKbdReal && (key_format != KEY_FORMAT_SCAN)) {
						if ((fm7 >= 0x3a) && (fm7 <= 0x46)) {
							/* �e���L�[�̏ꍇ */
							PushKeyCode((BYTE)(fm7 | 0x80));

							if (nLastKey2 == fm7) {
								PushKeyCode(0x3f);
								PushKeyCode(0xbf);
								nLastKey2 = 0;
							}

							/* �ȑO�e���L�[�ȊO��������Ă����ꍇ�A�Ĕ��s */
							if (nLastKey && key_repeat_flag) {
								PushKeyCode(nLastKey);
								PushKeyCode(0xff);
								key_last = nLastKey;
							}
						}
						else {
							/* �e���L�[�ȊO�̏ꍇ */
							keyboard_break(fm7);
							if (nLastKey == fm7) {
								nLastKey = 0;
							}

							/* �ȑO�e���L�[��������Ă����ꍇ�A�Ĕ��s */
							if (nLastKey2 && (nLastKey2 != key_last)) {
								PushKeyCode(nLastKey2);
								PushKeyCode(0xff);
								key_last = nLastKey2;
							}
						}
					}
					else if (fm7) {
						keyboard_break(fm7);
					}

					bFlag = TRUE;
				}
			}

			/* �f�[�^���R�s�[ */
			kbd_map[i] = buf[i];
		}
	}
}

/*
 *	�L�[�{�[�h
 *	�|�[�����O���L�[���擾
 *	��VM�̃��b�N�͍s���Ă��Ȃ��̂Œ���
 */
BOOL FASTCALL GetKbd(BYTE *pBuf)
{
	HRESULT hr;
	int i;

	ASSERT(pBuf);

	/* �������N���A */
	memset(pBuf, 0, 256);

	/* DirectInput�`�F�b�N */
	if (!lpkbd) {
		return FALSE;
	}

	/* �f�o�C�X�̃A�N�Z�X�����擾(����擾���Ă��悢) */
	hr = lpkbd->Acquire();
	if ((hr != DI_OK) && (hr != S_FALSE)) {
		return FALSE;
	}

	/* �f�o�C�X��Ԃ��擾 */
	if (lpkbd->GetDeviceState(256, pBuf) != DI_OK) {
		return FALSE;
	}

	/* NT�΍� */
	if (bNTkbMode) {
		for (i=0; i<3; i++) {
			pBuf[DI_keyno[i]] = 0;
		}
		if ((GetAsyncKeyState(VKNT_KANJI1) |
			 GetAsyncKeyState(VKNT_KANJI2)) & 1) {
			pBuf[DIK_KANJI] = 0x80;
		}
		if ((GetAsyncKeyState(VKNT_CAPITAL) |
			 GetAsyncKeyState(VK_CAPITAL)) & 1) {
			pBuf[DIK_CAPITAL] = 0x80;
		}
		if ((GetAsyncKeyState(VKNT_KANA) |
			 GetAsyncKeyState(VK_KANA)) & 1) {
			pBuf[DIK_KANA] = 0x80;
		}
	}

#ifndef DINPUT8
	/* Vista/Windows7 �ESHIFT�΍􏈗� */
	if (bVistaflag) {
		/* Vista/Window7�ł�0�Ԃ͏�ɖ��� */
		pBuf[0x00] = 0x00;

		/* �E�V�t�g�L�[�`�F�b�N */
		if (GetAsyncKeyState(VK_RSHIFT)) {
			pBuf[DIK_RSHIFT] = 0x80;
		}
	}
#endif

	return TRUE;
}

#if defined(KBDPASTE)
/*
 *	�L�[�{�[�h
 *	�y�[�X�g�i�N���b�v�{�[�h�j
 */
void FASTCALL PasteClipboardKbd(HWND hWnd)
{
	BOOL b;
	HGLOBAL hg;
	PTSTR strClip;

	ASSERT(hWnd);

	b = OpenClipboard(hWnd);
	hg = GetClipboardData(CF_TEXT);
	if (b && hg) {
		strClip = (PTSTR)GlobalLock(hg);
		if (strPaste) {
			free(strPaste);
			strPaste = NULL;
			strPtr = NULL;
			uWaitCnt = 0;
		}
		strPtr = strPaste = (PTSTR)malloc(GlobalSize(hg));
		if (strPaste) {
			lstrcpy(strPaste, strClip);
		}
		GlobalUnlock(hg);
		CloseClipboard();
	}
}

/*
 *	�L�[�{�[�h
 *	�y�[�X�g�i�����j
 */
void FASTCALL PasteKbd(char *pstring)
{
	PTSTR strKbd;

	strKbd = (PTSTR)pstring;
	if (strPaste) {
		free(strPaste);
		strPaste = NULL;
		strPtr = NULL;
		uWaitCnt = 0;
	}
	strPtr = strPaste = (PTSTR)malloc(strlen(pstring));
	if (strPaste) {
		lstrcpy(strPaste, strKbd);
	}
}
#endif

/*
 *	�W���C�X�e�B�b�N
 *	�f�o�C�X���ǂݍ���
 */
static BYTE FASTCALL GetJoy(int index, BOOL flag)
{
	int num;
	JOYINFOEX jiex;
	MMRESULT result;
	BYTE ret;

	/* assert */
	ASSERT((index == 0) || (index == 1));

	/* �T�|�[�g�W���C�X�e�B�b�N�����擾 */
	num = joyGetNumDevs();
	if (index >= num) {
		joyplugged[index] = FALSE;
		return 0;
	}

	/* �f�[�^�N���A */
	ret = 0;

	/* �f�[�^�擾 */
	memset(&jiex, 0, sizeof(jiex));
	jiex.dwSize = sizeof(jiex);
	jiex.dwFlags = JOY_RETURNX | JOY_RETURNY | JOY_RETURNBUTTONS |
					JOY_RETURNPOV | JOY_RETURNCENTERED;
	result = joyGetPosEx(index, &jiex);
	if (result == JOYERR_NOERROR) {
		joyplugged[index] = TRUE;

		/* �f�[�^�]��(����) */
		if (jiex.dwXpos < 0x4000) {
			ret |= 0x04;
		}
		if (jiex.dwXpos > 0xc000) {
			ret |= 0x08;
		}
		if (jiex.dwYpos < 0x4000) {
			ret |= 0x01;
		}
		if (jiex.dwYpos > 0xc000) {
			ret |= 0x02;
		}
		switch (jiex.dwPOV) {  
			case 0:  
				ret |= 0x01;
				break;
			case 4500:  
				ret |= 0x09;
				break;
			case 9000:  
				ret |= 0x08;
				break;
			case 13500:  
				ret |= 0x0a;
				break;
			case 18000:  
				ret |= 0x02;
				break;
			case 22500:  
				ret |= 0x06;
				break;
			case 27000:  
				ret |= 0x04;
				break;
			case 31500:  
				ret |= 0x05;
				break;
		}  

		/* �f�[�^�]��(�{�^��) */
		if (jiex.dwButtons & JOY_BUTTON1) {
			ret |= 0x20;
		}
		if (jiex.dwButtons & JOY_BUTTON2) {
			ret |= 0x10;
		}
	}
	else if (flag) {
		joyplugged[index] = FALSE;
		ret = 0;
	}

	return ret;
}

/*
 *	�W���C�X�e�B�b�N
 *	�A�˃J�E���^�e�[�u��(�Б��A20ms�P��)
 */
static const BYTE JoyRapidCounter[] = {
	0,			/* �Ȃ� */
	25,			/* 1�V���b�g */
	12,			/* 2�V���b�g */
	8,			/* 3�V���b�g */
	6,			/* 4�V���b�g */
	5,			/* 5�V���b�g */
	4,			/* 6�V���b�g */
	3,			/* 8�V���b�g */
	2,			/* 12�V���b�g */
	1			/* 25�V���b�g */
};

/*
 *	�W���C�X�e�B�b�N
 *	�f�o�C�X���ǂݍ���(�A�˂�)
 */
static BYTE FASTCALL GetRapidJoy(int index, BOOL flag)
{
	int i;
	BYTE bit;
	BYTE dat;

	/* assert */
	ASSERT((index == 0) || (index == 1));

	/* ��ڑ��`�F�b�N1 (�ڑ��`�F�b�N���͒ʂ�) */
	if ((!flag) && (!joyplugged[index])) {
		return 0x00;
	}

	/* �f�[�^�擾 */
	dat = GetJoy(index, flag);

	/* ��ڑ��`�F�b�N2 */
	if (!joyplugged[index]) {
		return 0x00;
	}

	/* �{�^���`�F�b�N */
	bit = 0x10;
	for (i=0; i<2; i++) {
		if ((dat & bit) && (nJoyRapid[index][i] > 0)) {
			/* �A�˂���ŉ�����Ă���B�J�E���^�`�F�b�N */
			if (joyrapid[index][i] == 0) {
				/* �����J�E���^���� */
				joyrapid[index][i] = JoyRapidCounter[nJoyRapid[index][i]];
			}
			else {
				/* �J�E���^�f�N�������g */
				joyrapid[index][i]--;
				if ((joyrapid[index][i] & 0xff) == 0) {
					/* ���]�^�C�~���O�Ȃ̂ŁA���Ԃ����Z���Ĕ��] */
					joyrapid[index][i] += JoyRapidCounter[nJoyRapid[index][i]];
					joyrapid[index][i] ^= 0x100;
				}
			}
			/* �{�^����������Ă��Ȃ��悤�ɐU�镑�� */
			if (joyrapid[index][i] >= 0x100) {
				dat &= (BYTE)(~bit);
			}
		}
		else {
			/* �{�^����������ĂȂ��̂ŁA�A�˃J�E���^�N���A */
			joyrapid[index][i] = 0;
		}
		/* ���̃r�b�g�� */
		bit <<= 1;
	}

	return dat;
}

/*
 *	�W���C�X�e�B�b�N
 *	�R�[�h�ϊ�
 */
static BYTE FASTCALL PollJoyCode(int code)
{
	/* 0x70�����͖��� */
	if (code < 0x70) {
		return 0;
	}

	/* 0x70����㉺���E */
	switch (code) {
		/* �� */
		case 0x70:
			return 0x01;
		/* �� */
		case 0x71:
			return 0x02;
		/* �� */
		case 0x72:
			return 0x04;
		/* �E */
		case 0x73:
			return 0x08;
		/* A�{�^�� */
		case 0x74:
			return 0x10;
		/* B�{�^�� */
		case 0x75:
			return 0x20;
		/* ����ȊO */
		default:
			ASSERT(FALSE);
			break;
	}

	return 0;
}

/*
 *	�W���C�X�e�B�b�N
 *	�|�[�����O(�W���C�X�e�B�b�N)
 */
static BYTE FASTCALL PollJoySub(int index, BYTE dat)
{
	int i;
	BYTE ret;
	BYTE bit;

	/* assert */
	ASSERT((index == 0) || (index == 1));

	/* �I���f�[�^�N���A */
	ret = 0;

	/* ���� */
	bit = 0x01;
	for (i=0; i<4; i++) {
		/* �{�^����������Ă��邩 */
		if (dat & bit) {
			/* �R�[�h�ϊ� */
			ret |= PollJoyCode(nJoyCode[index][i]);
		}
		bit <<= 1;
	}

	/* �Z���^�[�`�F�b�N */
	if ((dat & 0x0f) == 0) {
		if ((joybk[index] & 0x0f) != 0) {
			ret |= PollJoyCode(nJoyCode[index][4]);
		}
	}

	/* �{�^�� */
	if (dat & 0x10) {
		ret |= PollJoyCode(nJoyCode[index][5]);
	}
	if (dat & 0x20) {
		ret |= PollJoyCode(nJoyCode[index][6]);
	}

	return ret;
}

/*
 *	�W���C�X�e�B�b�N
 *	�|�[�����O(�L�[�{�[�h)
 */
static void FASTCALL PollJoyKbd(int index, BYTE dat)
{
	BYTE bit;
	int i;

	/* �㉺���E */
	bit = 0x01;
	for (i=0; i<4; i++) {
		if (dat & bit) {
			/* ���߂ĉ����ꂽ��Amake���s */
			if ((joybk[index] & bit) == 0) {
				if ((nJoyCode[index][i] > 0) && (nJoyCode[index][i] <= 0x66)) {
					keyboard_make((BYTE)nJoyCode[index][i]);
				}
			}
		}
		else {
			/* ���߂ė����ꂽ��Abreak���s */
			if ((joybk[index] & bit) != 0) {
				if ((nJoyCode[index][i] > 0) && (nJoyCode[index][i] <= 0x66)) {
					keyboard_break((BYTE)nJoyCode[index][i]);
				}
			}
		}
		bit <<= 1;
	}

	/* �Z���^�[�`�F�b�N */
	if ((dat & 0x0f) == 0) {
		if ((joybk[index] & 0x0f) != 0) {
			/* make/break�𑱂��ďo�� */
			if ((nJoyCode[index][4] > 0) && (nJoyCode[index][4] <= 0x66)) {
				keyboard_make((BYTE)nJoyCode[index][4]);
				keyboard_break((BYTE)nJoyCode[index][4]);
			}
		}
	}

	/* �{�^�� */
	bit = 0x10;
	for (i=0; i<2; i++) {
		if (dat & bit) {
			/* ���߂ĉ�������Amake���s */
			if ((joybk[index] & bit) == 0) {
				if ((nJoyCode[index][i + 5] > 0) && (nJoyCode[index][i + 5] <= 0x66)) {
					keyboard_make((BYTE)nJoyCode[index][i + 5]);
				}
			}
		}
		else {
			/* ���߂ė����ꂽ��Abreak���s */
			if ((joybk[index] & bit) != 0) {
				if ((nJoyCode[index][i + 5] > 0) && (nJoyCode[index][i + 5] <= 0x66)) {
					keyboard_break((BYTE)nJoyCode[index][i + 5]);
				}
			}
		}
		bit <<= 1;
	}
}

/*
 *	�W���C�X�e�B�b�N
 *	�|�[�����O
 */
void FASTCALL PollJoy(void)
{
	BYTE dat;
	BOOL check;
	int i;

	/* �Ԋu�`�F�b�N(�|�[�����O�p) */
	if ((dwExecTotal - joytime) < 10000) {
		return;
	}
	joytime = dwExecTotal;

	/* �f�[�^���N���A */
	memset(joydat, 0, sizeof(joydat));

	/* �����`�F�b�N */
	if ((nJoyType[0] == 0) && (nJoyType[1] == 0)) {
		return;
	}

	/* �Ԋu�`�F�b�N(�ڑ��`�F�b�N�p) */
	if ((dwExecTotal - joytime2) >= 500000) {
		joytime2 = dwExecTotal;
		check = TRUE;
	}
	else {
		check = FALSE;
	}

	/* �f�o�C�X���[�v */
	for (i=0; i<2; i++) {
		/* �f�[�^�擾(�A�˂�) */
		dat = GetRapidJoy(i, check);
		if (!joyplugged[i]) {
			continue;
		}

		/* �^�C�v�� */
		switch (nJoyType[i]) {
			/* �W���C�X�e�B�b�N�|�[�g1 */
			case 1:
				joydat[0] = PollJoySub(i, dat);
				break;
			/* �W���C�X�e�B�b�N�|�[�g2 */
			case 2:
				joydat[1] = PollJoySub(i, dat);
				break;
			/* �L�[�{�[�h */
			case 3:
				PollJoyKbd(i, dat);
				break;
			/* �d�g�V���ЃW���C�X�e�B�b�N */
			case 4:
				joydat[2] = PollJoySub(i, dat);
				break;
		}

		/* �f�[�^�X�V */
		joybk[i] = dat;
	}
}

/*
 *	�W���C�X�e�B�b�N
 *	�f�[�^���N�G�X�g
 */
BYTE FASTCALL joy_request(BYTE no)
{
/*	ASSERT((no >= 0) && (no < 3)); */
	ASSERT(no < 3);

	return joydat[no];
}


#if defined(MOUSE)
/*
 *	�}�E�X
 *	�|�[�����O
 */
void FASTCALL PollMos(void)
{
	DIMOUSESTATE buf;
	HRESULT hr;

	/* DirectInput�`�F�b�N */
	if (!lpmos) {
		return;
	}

	/* �}�E�X���݃`�F�b�N */
	if (!bDetectMouse) {
		return;
	}

	/* �A�N�e�B�x�[�g�`�F�b�N */
	if (!bActivate) {
		return;
	}

	/* ���ԃ`�F�b�N */
	if ((dwExecTotal - mostime) < 10000) {
		return;
	}
	mostime = dwExecTotal;

	/* �f�o�C�X�̃A�N�Z�X�����擾(����擾���Ă��悢) */
	hr = lpmos->Acquire();
	if ((hr != DI_OK) && (hr != S_FALSE)) {
		return;
	}

	/* �f�o�C�X��Ԃ��擾 */
	if (lpmos->GetDeviceState(sizeof(DIMOUSESTATE), &buf) != DI_OK) {
		return;
	}

	/* �ړ������E�{�^����Ԃ̐ݒ� */
	if (bCapture && mos_capture) {
		/* �}�E�X�ړ�������~�� */
		nMouseX -= buf.lX;
		nMouseY -= buf.lY;

		/* �{�^����Ԃ�ݒ� */
		nMouseButton = 0xf0;
		if (buf.rgbButtons[0]) {
			/* ���{�^������ */
			nMouseButton &= ~0x10;
		}
		if (buf.rgbButtons[1]) {
			/* �E�{�^������ */
			nMouseButton &= ~0x20;
		}
	}
}


/*
 *	�}�E�X
 *	�L���v�`����Ԑݒ�
 */
void FASTCALL SetMouseCapture(BOOL en)
{
	DIMOUSESTATE buf;
	HRESULT hr;
	RECT rect;
	POINT center;

	ASSERT(hDrawWnd);

	/* �L���v�`����~��/VM��~��/��A�N�e�B�u���͋����I�ɖ��� */
	if (!mos_capture || stopreq_flag || !run_flag || !bActivate || !lpmos) {
		en = FALSE;
	}

	/* �J�[�\���\��/���� */
	if (bMouseCursor == en) {
		ShowCursor(!en);
		bMouseCursor = !en;
	}

	/* �L���v�`����Ԃɕω����Ȃ���΋A�� */
	if (bCapture == en) {
		return;
	}

	if (en) {
		if (hDrawWnd) {
			/* �J�[�\���ʒu��ۑ� */
			GetCursorPos(&center);
			nMouseSX = center.x;
			nMouseSY = center.y;

			/* �`��E�B���h�E�̎l���̍��W�����߂� */
			GetWindowRect(hDrawWnd, &rect);

			/* �l���̍��W���璆�S���W�����߂� */
			center.x = (rect.right + rect.left) / 2;
			center.y = (rect.bottom + rect.top) / 2;

			/* �J�[�\�����E�B���h�E�����ɌŒ� */
			rect.left   = center.x;
			rect.right  = center.x;
			rect.top    = center.y;
			rect.bottom = center.y;
			SetCursorPos(center.x, center.y);
			ClipCursor(&rect);
		}
	}
	else {
		/* �N���b�v���� */
		ClipCursor(0);

		/* �J�[�\���ʒu�𕜌� */
		SetCursorPos(nMouseSX, nMouseSY);
	}

	/* �L���v�`����Ԃ�ۑ� */
	bCapture = en;

	/* �}�E�X�ړ��������N���A */
	nMouseX = 0;
	nMouseY = 0;

	/* �f�o�C�X��Ԃ���ǂ� */
	if (lpmos) {
		hr = lpmos->Acquire();
		if ((hr == DI_OK) || (hr == S_FALSE)) {
			lpmos->GetDeviceState(sizeof(DIMOUSESTATE), &buf);
		}
	}
}

/*
 *	�}�E�X
 *	�ړ��f�[�^���N�G�X�g
 */
void FASTCALL mospos_request(BYTE *move_x, BYTE *move_y)
{
	if (bCapture) {
		/* �ړ������𕄍��t���W�r�b�g�͈̔͂Ɏ��߂� */
		if (nMouseX > 127) {
			nMouseX = 127;
		}
		else if (nMouseX < -127) {
			nMouseX = -127;
		}

		if (nMouseY > 127) {
			nMouseY = 127;
		}
		else if (nMouseY < -127) {
			nMouseY = -127;
		}

		*move_x = (BYTE)nMouseX;
		*move_y = (BYTE)nMouseY;
	}
	else {
		/* �L���v�`���ꎞ��~���͈ړ����Ă��Ȃ����Ƃɂ��� */
		*move_x = 0;
		*move_y = 0;
	}

	/* �}�E�X�ړ��������N���A */
	nMouseX = 0;
	nMouseY = 0;
}

/*
 *	�}�E�X
 *	�{�^���f�[�^���N�G�X�g
 */
BYTE FASTCALL mosbtn_request(void)
{
	/* �{�^������Ԃ� */
	return nMouseButton;
}

#endif	/* MOUSE */

#endif	/* _WIN32 */
