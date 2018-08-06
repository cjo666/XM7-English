/*
 *	FM-7 EMULATOR "XM7"
 *
 *	Copyright (C) 1999-2017 �o�h�D(Twitter:@xm6_original)
 *	Copyright (C) 2001-2017 Ryu Takegami (Twitter:@RyuTakegami)
 *
 *	[ Win32API �T�u�E�C���h�E ]
 */

#ifdef _WIN32

#ifndef _w32_sub_h_
#define _w32_sub_h_

/*
 *	�T�u�E�C���h�E��`
 */
#define SWND_BREAKPOINT			0		/* �u���[�N�|�C���g */
#define SWND_SCHEDULER			1		/* �X�P�W���[�� */
#if XM7_VER == 1
#define SWND_CPUREG_MAIN		2		/* CPU���W�X�^ ���C��(6809) */
#define SWND_CPUREG_SUB			3		/* CPU���W�X�^ �T�u */
#define SWND_CPUREG_JSUB		4		/* CPU���W�X�^ ���{��T�u */
#define SWND_CPUREG_Z80			5		/* CPU���W�X�^ ���C��(Z80) */
#define SWND_DISASM_MAIN		6		/* �t�A�Z���u�� ���C��(6809) */
#define SWND_DISASM_SUB			7		/* �t�A�Z���u�� �T�u */
#define SWND_DISASM_JSUB		8		/* �t�A�Z���u�� ���{��T�u */
#define SWND_DISASM_Z80			9		/* �t�A�Z���u�� ���C��(Z80) */
#define SWND_MEMORY_MAIN		10		/* �������_���v ���C�� */
#define SWND_MEMORY_SUB			11		/* �������_���v �T�u */
#define SWND_MEMORY_JSUB		12		/* �������_���v ���{��T�u */
#define SWND_FDC				13		/* FDC */
#define SWND_OPNREG				14		/* OPN���W�X�^ */
#define SWND_OPNDISP			15		/* OPN�f�B�X�v���C */
#define SWND_SUBCTRL			16		/* �T�uCPU�R���g���[�� */
#define SWND_KEYBOARD			17		/* �L�[�{�[�h */
#define SWND_MMR				18		/* MMR */
#define SWND_PALETTE			19		/* �p���b�g���W�X�^ */
#define SWND_BMC				20		/* �o�u���������R���g���[�� */
#define SWND_MAXNUM				21
#else
#define SWND_CPUREG_MAIN		2		/* CPU���W�X�^ ���C�� */
#define SWND_CPUREG_SUB			3		/* CPU���W�X�^ �T�u */
#define SWND_DISASM_MAIN		4		/* �t�A�Z���u�� ���C�� */
#define SWND_DISASM_SUB			5		/* �t�A�Z���u�� �T�u */
#define SWND_MEMORY_MAIN		6		/* �������_���v ���C�� */
#define SWND_MEMORY_SUB			7		/* �������_���v �T�u */
#define SWND_FDC				8		/* FDC */
#define SWND_OPNREG				9		/* OPN���W�X�^ */
#define SWND_OPNDISP			10		/* OPN�f�B�X�v���C */
#define SWND_SUBCTRL			11		/* �T�uCPU�R���g���[�� */
#define SWND_ALULINE			12		/* �_�����Z/������� */
#define SWND_KEYBOARD			13		/* �L�[�{�[�h */
#define SWND_MMR				14		/* MMR */
#define SWND_PALETTE			15		/* �p���b�g���W�X�^ */
#if XM7_VER >= 3
#define	SWND_DMAC				16		/* DMAC */
#define SWND_MAXNUM				17		/* �ő�T�u�E�C���h�E�� */
#else
#define SWND_MAXNUM				16		/* �ő�T�u�E�C���h�E�� */
#endif
#endif

#ifdef __cplusplus
extern "C" {
#endif
/*
 *	��v�G���g��
 */
void FASTCALL InitSubWndWork(void);
										/* �T�u�E�B���h�E���[�N������ */

HWND FASTCALL CreateBreakPoint(HWND hParent, int index);
										/* �u���[�N�|�C���g�E�C���h�E �쐬 */
void FASTCALL RefreshBreakPoint(void);
										/* �u���[�N�|�C���g�E�C���h�E ���t���b�V�� */
HWND FASTCALL CreateScheduler(HWND hParent, int index);
										/* �X�P�W���[���E�C���h�E �쐬 */
void FASTCALL RefreshScheduler(void);
										/* �X�P�W���[���E�C���h�E ���t���b�V�� */
HWND FASTCALL CreateCPURegister(HWND hParent, BYTE nCPU, int index);
										/* CPU���W�X�^�E�C���h�E �쐬 */
void FASTCALL RefreshCPURegister(void);
										/* CPU���W�X�^�E�C���h�E ���t���b�V�� */
HWND FASTCALL CreateDisAsm(HWND hParent, BYTE nCPU, int index);
										/* �t�A�Z���u���E�C���h�E �쐬 */
void FASTCALL RefreshDisAsm(void);
										/* �t�A�Z���u���E�C���h�E ���t���b�V�� */
void FASTCALL AddrDisAsm(BYTE nCPU, DWORD dwAddr);
										/* �t�A�Z���u���E�C���h�E �A�h���X�w�� */
HWND FASTCALL CreateMemory(HWND hParent, BYTE nCPU, int index);
										/* �������_���v�E�C���h�E �쐬 */
void FASTCALL RefreshMemory(void);
										/* �������_���v�E�C���h�E ���t���b�V�� */
void FASTCALL AddrMemory(BYTE nCPU, DWORD dwAddr);
										/* �������_���v�E�C���h�E �A�h���X�w�� */

HWND FASTCALL CreateFDC(HWND hParent, int index);
										/* FDC�E�C���h�E �쐬 */
void FASTCALL RefreshFDC(void);
										/* FDC�E�C���h�E ���t���b�V�� */
#if XM7_VER ==1 && defined(BUBBLE)
HWND FASTCALL CreateBMC(HWND hParent, int index);
										/* BMC�E�C���h�E �쐬 */
void FASTCALL RefreshBMC(void);
										/* BMC�E�C���h�E ���t���b�V�� */
#endif
HWND FASTCALL CreateOPNReg(HWND hParent, int index);
										/* OPN���W�X�^�E�C���h�E �쐬 */
void FASTCALL RefreshOPNReg(void);
										/* OPN���W�X�^�E�C���h�E ���t���b�V�� */
void FASTCALL ReSizeOPNReg(void);
										/* OPN���W�X�^�E�C���h�E ���T�C�Y */
HWND FASTCALL CreateSubCtrl(HWND hParent, int index);
										/* �T�uCPU�R���g���[���E�C���h�E �쐬 */
void FASTCALL RefreshSubCtrl(void);
										/* �T�uCPU�R���g���[���E�C���h�E ���t���b�V�� */
HWND FASTCALL CreateALULine(HWND hParent, int index);
										/* �_�����Z/������ԃE�C���h�E �쐬 */
void FASTCALL RefreshALULine(void);
										/* �_�����Z/������ԃE�C���h�E ���t���b�V�� */
HWND FASTCALL CreateOPNDisp(HWND hParent, int index);
										/* OPN�f�B�X�v���C�E�C���h�E �쐬 */
void FASTCALL RefreshOPNDisp(void);
										/* OPN�f�B�X�v���C�E�C���h�E ���t���b�V�� */
void FASTCALL ReSizeOPNDisp(void);
										/* OPN�f�B�X�v���C�E�C���h�E ���T�C�Y */
HWND FASTCALL CreateKeyboard(HWND hParent, int index);
										/* �L�[�{�[�h�E�C���h�E �쐬 */
void FASTCALL RefreshKeyboard(void);
										/* �L�[�{�[�h�E�C���h�E ���t���b�V�� */
HWND FASTCALL CreateMMR(HWND hParent, int index);
										/* MMR�E�C���h�E �쐬 */
void FASTCALL RefreshMMR(void);
										/* MMR�E�C���h�E ���t���b�V�� */
#if XM7_VER >= 3
HWND FASTCALL CreateDMAC(HWND hParent, int index);
										/* DMAC�E�C���h�E �쐬 */
void FASTCALL RefreshDMAC(void);
										/* DMAC�E�C���h�E ���t���b�V�� */
#endif
HWND FASTCALL CreatePaletteReg(HWND hParent, int index);
										/* �p���b�g���W�X�^�E�C���h�E �쐬 */
void FASTCALL RefreshPaletteReg(void);
										/* �p���b�g���W�X�^�E�C���h�E ���t���b�V�� */

/*
 *	�����ėp���[�`��
 */
void FASTCALL DrawWindowText(HDC hDC, BYTE *ptr, int x, int y);
										/* �e�L�X�g�`�� */
void FASTCALL DrawWindowText2(HDC hDC, BYTE *ptr, int x, int y);
										/* �e�L�X�g�`��(���]�t��) */
void FASTCALL DestroySubWindow(HWND hWnd, BYTE **pBuf, HMENU hmenu);
										/* �T�u�E�C���h�EDestroy */
void FASTCALL PositioningSubWindow(HWND hParent, LPRECT rect, int index);
										/* �T�u�E�B���h�E�ʒu�Z�o */

/*
 *	��v���[�N
 */
extern HWND hSubWnd[SWND_MAXNUM];
										/* �T�u�E�C���h�E */
extern BOOL bShowSubWindow[SWND_MAXNUM];
										/* �T�u�E�C���h�E�\����� */
extern BOOL bPopupSwnd;
										/* �T�u�E�B���h�E�|�b�v�A�b�v��� */
extern BOOL bPaletteRefresh;
										/* �p���b�g���t���b�V�� */
#ifdef __cplusplus
}
#endif

#endif	/* _w32_sub_h_ */
#endif	/* _WIN32 */
