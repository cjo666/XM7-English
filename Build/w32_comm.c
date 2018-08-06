/*
 *	FM-7 EMULATOR "XM7"
 *
 *	Copyright (C) 1999-2017 �o�h�D(Twitter:@xm6_original)
 *	Copyright (C) 2001-2017 Ryu Takegami (Twitter:@RyuTakegami)
 *
 *	[ Win32API �V���A���|�[�g ]
 *
 *	RHG����
 *	  2003.08.28		�V��(���g�Ȃ�)
 *	  2003.09.30		�@�\����
 *	  2003.11.01		�f�o�C�X�I�[�v���Ɏ��s�����ꍇ�̏�����ǉ�
 *	  2011.07.26		�X���b�h�쐬�E�I���֐���_beginthreadex/_endthreadex��
 *						�ύX
 */

#ifdef _WIN32

#if defined(RSC)

#define STRICT
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <commctrl.h>
#include <imm.h>
#include <shellapi.h>
#include <stdlib.h>
#include <assert.h>
#include <mmsystem.h>
#include <process.h>
#include "xm7.h"
#include "mainetc.h"
#include "device.h"
#include "rs232c.h"
#include "w32.h"
#include "w32_res.h"
#include "w32_comm.h"


/*
 *	�O���[�o�� ���[�N
 */
int nCommPortNo;						/* �g�p����V���A���|�[�g�ԍ� */

/*
 *	�X�^�e�B�b�N ���[�N
 */
static CRITICAL_SECTION CSection;		/* �N���e�B�J���Z�N�V���� */
static HANDLE hThreadRcv;				/* ��M�X���b�h�n���h�� */
static UINT uThResult;					/* �X���b�h�߂�l */
static BOOL bCommCloseReq;				/* �X���b�h�I���v���t���O */
static BOOL bCriticalSection;

static HANDLE hComm;					/* �V���A���|�[�g�n���h�� */
static int nOpenPort;					/* �g�p���̃|�[�g�ԍ� */
static COMMPROP commprop;				/* */
static OVERLAPPED Overlap;				/* OVERLAPPED�\���� */
static DCB dcb;							/* DCB�\���� */
static COMMTIMEOUTS CommTimeout;		/* TIMEOUT�\���� */

extern BYTE rs_status;					/* �X�e�[�^�X���W�X�^ */
static BYTE uRsRcvData[4096];			/* ��M�f�[�^�o�b�t�@ */
static DWORD dwRsRcvCount;				/* ��M�f�[�^�T�C�Y */
static DWORD dwRsRcvPoint;				/* ��M�f�[�^�ǂݍ��݈ʒu */


/*
 *	�v���g�^�C�v�錾
 */
static UINT WINAPI ThreadRcv(LPVOID);	/* �X���b�h�֐� */

/*
 *	�ʐM���x�e�[�u��
 */
static const DWORD dwBaudrateSlow[8] = {
	CBR_300,	CBR_600,	CBR_1200,	CBR_2400,
	CBR_4800,	CBR_9600,	CBR_19200,	CBR_38400
};
static const DWORD dwBaudrateFast[8] = {
	CBR_1200,	CBR_2400,	CBR_4800,	CBR_9600,
	CBR_19200,	CBR_38400,	CBR_38400,	CBR_38400
};

/*
 *	�V���A���|�[�g
 *	������
 */
BOOL FASTCALL InitCommPort(void)
{
	char buf[256];
	char string[256];
	char comportname[256];

	/* �܂��N���A */
	if (hComm != INVALID_HANDLE_VALUE) {
		CloseHandle(hComm);
	}
	hComm = INVALID_HANDLE_VALUE;
	rs_mask = TRUE;
	bCriticalSection = FALSE;

	/* rs_use��FALSE�Ȃ�V���A���|�[�g�͎g�p���Ȃ� */
	if (!rs_use) {
		nOpenPort = 0;
		return FALSE;
	}

	/* �Ƃ肠�����A����Ă݂� */
	_snprintf(comportname, sizeof(comportname), "COM%d", nCommPortNo);
	hComm = CreateFile(comportname,
				GENERIC_READ | GENERIC_WRITE, 0, NULL,
				OPEN_EXISTING, FILE_FLAG_OVERLAPPED, NULL);

	/* �Ή��f�o�C�X�`�F�b�N */
	if (hComm != INVALID_HANDLE_VALUE) {
		commprop.wPacketLength = sizeof(commprop);
		GetCommProperties(hComm, &commprop);
	}
	else {
		/* CreateFile�Ɏ��s���Ă���̂Ńf�o�C�X��ʂ�K���� */
		commprop.dwProvSubType = 0xffffffff;
	}
	if ((commprop.dwProvSubType != PST_RS232) &&
		(commprop.dwProvSubType != PST_MODEM) &&
		(commprop.dwProvSubType != PST_UNSPECIFIED)) {
		LoadString(hAppInstance, IDS_COMM_OPENERROR, buf, sizeof(buf));
		_snprintf(string, sizeof(string), buf, comportname);
		MessageBox(hMainWnd, string, "XM7", MB_OK | MB_ICONERROR);

		if (hComm != INVALID_HANDLE_VALUE) {
			CloseHandle(hComm);
			hComm = INVALID_HANDLE_VALUE;
		}
		nOpenPort = 0;

		return FALSE;
	}

	/* �^�C���A�E�g���Ԃ�ݒ� */
	CommTimeout.ReadIntervalTimeout = 0;
	CommTimeout.ReadTotalTimeoutMultiplier = 0;
	CommTimeout.ReadTotalTimeoutConstant = 0;
	CommTimeout.WriteTotalTimeoutMultiplier = 0;
	CommTimeout.WriteTotalTimeoutConstant = 0;
	SetCommTimeouts(hComm,&CommTimeout);

	/* �����ݒ� */
	dcb.DCBlength = sizeof(dcb);
	GetCommState(hComm, &dcb);

	dcb.BaudRate = CBR_1200;
	dcb.fBinary = TRUE;
	dcb.fParity = FALSE;
	dcb.fOutxCtsFlow = FALSE;
	dcb.fOutxDsrFlow = FALSE;
	dcb.fDtrControl = DTR_CONTROL_DISABLE;
	dcb.fDsrSensitivity = FALSE;
	dcb.fTXContinueOnXoff = FALSE;
	dcb.fOutX = FALSE;
	dcb.fInX = FALSE;
	dcb.fErrorChar = TRUE;
	dcb.fNull = TRUE;
	dcb.fRtsControl = RTS_CONTROL_DISABLE;
	dcb.fAbortOnError = FALSE;
	dcb.wReserved = 0;
	dcb.ByteSize = 8;
	dcb.Parity = NOPARITY;
	dcb.StopBits = ONESTOPBIT;
	dcb.ErrorChar = '?';
	SetCommState(hComm, &dcb);

	/* ��M�X���b�h�쐬 */
	hThreadRcv = (HANDLE)_beginthreadex(NULL, 0, ThreadRcv, 0, 0, &uThResult);
	if (!hThreadRcv) {
		LoadString(hAppInstance, IDS_COMM_THREADERROR, string, sizeof(string));
		MessageBox(hMainWnd, string, "XM7", MB_OK | MB_ICONERROR);

		CloseHandle(hComm);
		hComm = INVALID_HANDLE_VALUE;
		rs_mask = TRUE;
		nOpenPort = 0;

		return FALSE;
	}

	/* �I�[�v�������|�[�g�ԍ���ێ� */
	nOpenPort = nCommPortNo;
	rs_mask = FALSE;

	return TRUE;
}

/*
 *	�V���A���|�[�g
 *	�N���[���A�b�v
 */
void FASTCALL CleanCommPort(void)
{
	DWORD dwExitCode;

	/* �X���b�h������I�����Ă��Ȃ���΁A�I��点�� */
	while (hThreadRcv && !uThResult) {
		bCommCloseReq = TRUE;
		while (TRUE) {
			GetExitCodeThread(hThreadRcv, &dwExitCode);
			if (dwExitCode == STILL_ACTIVE) {
				WaitForSingleObject(hThreadRcv, 10);
			}
			else {
				CloseHandle(hThreadRcv);
				hComm = INVALID_HANDLE_VALUE;
				rs_mask = TRUE;
				break;
			}
		}
	}
}

/*
 *	�Z���N�g��ԃ`�F�b�N
 */
BOOL FASTCALL SelectCheckCommPort(int port)
{
	/* �g�����Ƃ��Ă���|�[�g�Ǝg�p���̃|�[�g����v���Ă��Ȃ����`�F�b�N */
	if (nOpenPort != port) {
		return FALSE;
	}

	return TRUE;
}

/*
 *	���Z�b�g���f�o�C�X�Z���N�g
 */
void FASTCALL rs232c_reset_notify(void)
{
	/* �g�����Ƃ��Ă���|�[�g�Ǝg�p���̃|�[�g����v���Ă��Ȃ����`�F�b�N */
	if (nOpenPort != nCommPortNo) {
		/* �|�[�g���J����Ă���ꍇ�͂�������N���[���A�b�v */
		if (nOpenPort) {
			CleanCommPort();
		}

		/* ���߂ď����� */
		InitCommPort();
	}
}


/*
 *	���b�N
 */
void FASTCALL LockRS(void)
{
	if (bCriticalSection) {
		EnterCriticalSection(&CSection);
	}
}

/*
 *	�A�����b�N
 */
void FASTCALL UnlockRS(void)
{
	if (bCriticalSection) {
		LeaveCriticalSection(&CSection);
	}
}

/*
 *	�f�[�^���M
 */
void FASTCALL rs232c_senddata(BYTE dat)
{
	DWORD dwErrors;
	DWORD dwTransfer;
	COMSTAT comstat;

	/* RS-232C���������̏ꍇ�͉������Ȃ� */
	if (rs_mask || hComm == INVALID_HANDLE_VALUE) {
		return;
	}

	/* TxE=1,CTS=L�̏ꍇ�A���M���\ */
	if (!rs_cts && (rs_command & RSC_TXEN) && (rs_status & RSS_TXRDY)) {
		/* �|�[�����O�X���b�h�����b�N */
		LockRS();

		/* ��������TxEMPTY/TxRDY�𗎂Ƃ� */
		rs232c_txrdy(FALSE);

		/* 1�o�C�g���M */
		ClearCommError(hComm, &dwErrors, &comstat);
		if (!WriteFile(hComm, &dat, 1, NULL, &Overlap)) {
			if (GetLastError() == ERROR_IO_PENDING) {
				GetOverlappedResult(hComm, &Overlap, &dwTransfer, TRUE);
			}
		}

		/* TxRDY ON/TxEMPTY�v�� */
		rs232c_txempty_request();

		/* �|�[�����O�X���b�h���A�����b�N */
		UnlockRS();
	}
}

/*
 *	�f�[�^��M
 */
BYTE FASTCALL rs232c_receivedata(void)
{
	BYTE dat;

	/* RS-232C���������̏ꍇ�͉������Ȃ� */
	if (rs_mask || hComm == INVALID_HANDLE_VALUE) {
		return 0xff;
	}

	/* �|�[�����O�X���b�h�����b�N */
	LockRS();

	/* �o�b�t�@����f�[�^����肾�� */
	dat = uRsRcvData[dwRsRcvPoint];

	/* RxE=1�̏ꍇ�A��M���\ */
	if (rs_command & RSC_RXE) {
		dwRsRcvPoint ++;

		/* �f�[�^���܂�����ꍇ�ARxRDY�v�� */
		if (dwRsRcvPoint < dwRsRcvCount) {
			rs232c_rxrdy_request();
		}
		else {
			if (dwRsRcvCount) {
				uRsRcvData[0] = uRsRcvData[dwRsRcvCount];
			}
			dwRsRcvCount = 0;
			dwRsRcvPoint = 0;
		}
	}

	/* RxRDY�𗎂Ƃ� */
	rs232c_rxrdy(FALSE);

	/* �|�[�����O�X���b�h���A�����b�N */
	UnlockRS();

	return dat;
}

/*
 *	�X�e�[�^�X���W�X�^�ǂݏo��
 */
BYTE FASTCALL rs232c_readstatus(void)
{
	/* ����ł͒P���ɃX�e�[�^�X���W�X�^�̓��e��Ԃ����� */
	return rs_status;
}

/*
 *	���[�h�R�}���h���W�X�^��������
 */
void FASTCALL rs232c_writemodecmd(BYTE dat)
{
	/* RS-232C���������̏ꍇ�͉������Ȃ� */
	if (rs_mask || hComm == INVALID_HANDLE_VALUE) {
		return;
	}

	/* �|�[�����O�X���b�h�����b�N */
	LockRS();

	/* ���݂̐ݒ���擾 */
	dcb.DCBlength = sizeof(dcb);
	GetCommState(hComm, &dcb);

	/* �X�g�b�v�r�b�g */
	switch (dat & RSM_STOPBITM) {
		case RSM_STOPBIT1	:	dcb.StopBits = ONESTOPBIT;
								break;
		case RSM_STOPBIT15	:	dcb.StopBits = ONE5STOPBITS;
								break;
		case RSM_STOPBIT2	:	dcb.StopBits = TWOSTOPBITS;
								break;
	}

	/* �p���e�B */
	if (dat & RSM_PARITYEN) {
		dcb.fParity = TRUE;
		if (dat & RSM_PARITYEVEN) {
			dcb.Parity = EVENPARITY;
		}
		else {
			dcb.Parity = ODDPARITY;
		}
	}
	else {
		dcb.fParity = FALSE;
		dcb.Parity = NOPARITY;
	}

	/* �L�����N�^�� */
	switch (dat & RSM_CHARLENM) {
		case RSM_CHARLEN5	:	dcb.ByteSize = 5;
								break;
		case RSM_CHARLEN6	:	dcb.ByteSize = 6;
								break;
		case RSM_CHARLEN7	:	dcb.ByteSize = 7;
								break;
		case RSM_CHARLEN8	:	dcb.ByteSize = 8;
								break;
	}

	/* �N���b�N���� */
	if ((dat & RSM_BAUDDIVM) == RSM_BAUDDIV64) {
		dcb.BaudRate = dwBaudrateSlow[(rs_baudrate & RSCB_BAUDM) >> 2];
	}
	else {
		dcb.BaudRate = dwBaudrateFast[(rs_baudrate & RSCB_BAUDM) >> 2];
	}

	/* �ݒ� */
	SetCommState(hComm, &dcb);

	/* �|�[�����O�X���b�h���A�����b�N */
	UnlockRS();
}

/*
 *	�R�}���h���W�X�^��������
 */
void FASTCALL rs232c_writecommand(BYTE dat)
{
	/* RS-232C���������̏ꍇ�͉������Ȃ� */
	if (rs_mask || hComm == INVALID_HANDLE_VALUE) {
		return;
	}

	/* �|�[�����O�X���b�h�����b�N */
	LockRS();

	/* �G���[�N���A */
	if (dat & RSC_ER) {
		rs_status &= (BYTE)~(RSS_FRAMEERR | RSS_OVERRUN | RSS_PARITYERR);
	}

	/* RTS���� */
	if (dat & RSC_RTS) {
		EscapeCommFunction(hComm, SETRTS);
	}
	else {
		EscapeCommFunction(hComm, CLRRTS);
	}

	/* �u���[�N���� */
	if (dat & RSC_SBRK) {
		EscapeCommFunction(hComm, SETBREAK);
	}
	else {
		EscapeCommFunction(hComm, CLRBREAK);
	}

	/* DTR���� */
	if ((dat & RSC_DTR) && !rs_dtrmask) {
		EscapeCommFunction(hComm, SETDTR);
	}
	else {
		EscapeCommFunction(hComm, CLRDTR);
	}

	/* �|�[�����O�X���b�h���A�����b�N */
	UnlockRS();
}

/*
 *	�N���b�N�E�{�[���[�g�ݒ背�W�X�^��������
 */
void FASTCALL rs232c_setbaudrate(BYTE dat)
{
	/* RS-232C���������̏ꍇ�͉������Ȃ� */
	if (rs_mask || hComm == INVALID_HANDLE_VALUE) {
		return;
	}

	/* �|�[�����O�X���b�h�����b�N */
	LockRS();

	/* ���݂̐ݒ���擾 */
	dcb.DCBlength = sizeof(dcb);
	GetCommState(hComm, &dcb);

	/* �{�[���[�g��ݒ� */
	if ((rs_modecmd & RSM_BAUDDIVM) == RSM_BAUDDIV64) {
		dcb.BaudRate = dwBaudrateSlow[(dat & RSCB_BAUDM) >> 2];
	}
	else {
		dcb.BaudRate = dwBaudrateFast[(dat & RSCB_BAUDM) >> 2];
	}

	/* �ݒ� */
	SetCommState(hComm, &dcb);

	/* �|�[�����O�X���b�h���A�����b�N */
	UnlockRS();
}

/*
 *	�X���b�h�֐�
 */
static UINT WINAPI ThreadRcv(LPVOID param)
{
	DWORD dwEvent;
	DWORD dwErrors;
	DWORD dwStat;
	DWORD dwTransfer;
	DWORD dwRcvCount;
	COMSTAT comstat;

	UNUSED(param);

	/* �N���e�B�J���Z�N�V�������쐬 */
	InitializeCriticalSection(&CSection);

	/* ���[�N�G���A������ */
	bCommCloseReq = FALSE;
	rs_status |= (RSS_TXRDY | RSS_TXEMPTY);
	dwRsRcvCount = 0;
	dwRsRcvPoint = 0;

	/* �񓯊�I/O�p�I�[�o�[���b�v�\���̂������� */
	memset(&Overlap, 0, sizeof(Overlap));
	Overlap.hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

	/* �C�x���g�}�X�N��ݒ� */
	SetCommMask(hComm,
		EV_BREAK | EV_CTS | EV_DSR | EV_ERR | EV_RLSD | EV_RXCHAR | EV_TXEMPTY);

	/* �N���e�B�J���Z�N�V�������䋖�� */
	bCriticalSection = TRUE;

	while (!bCloseReq) {
		WaitCommEvent(hComm, &dwEvent, &Overlap);
		if (GetLastError() == ERROR_IO_PENDING) {
			GetOverlappedResult(hComm, &Overlap, &dwTransfer, TRUE);
			ClearCommError(hComm, &dwErrors, &comstat);

			/* �u���[�N���o */
			if (dwEvent & EV_BREAK) {
				if (rs_command & RSC_EH) {
					rs232c_syndet(TRUE);
				}
			}

			/* CTS�M�����ω� */
			if (dwEvent & EV_CTS) {
				GetCommModemStatus(hComm, &dwStat);
				if (dwStat & MS_CTS_ON) {
					rs_cts = TRUE;
				}
				else {
					rs_cts = FALSE;
				}
			}

			/* DSR�M�����ω� */
			if (dwEvent & EV_DSR) {
				GetCommModemStatus(hComm, &dwStat);
				if (dwStat & MS_DSR_ON) {
					rs_status |= RSS_DSR;
				}
				else {
					rs_status &= (BYTE)~RSS_DSR;
				}
			}

			/* �G���[���� */
			if (dwEvent & EV_ERR) {
				if (dwErrors & CE_FRAME) {
					rs_status |= RSS_FRAMEERR;
				}
				if (dwErrors & CE_RXOVER) {
					rs_status |= RSS_OVERRUN;
				}
				if (dwErrors & CE_RXPARITY) {
					rs_status |= RSS_PARITYERR;
				}
			}

			/* CD�M�����ω� */
			if (dwEvent & EV_RLSD) {
				GetCommModemStatus(hComm, &dwStat);
				if (dwStat & MS_RLSD_ON) {
					rs_cd = TRUE;
				}
				else {
					rs_cd = FALSE;
				}
			}

			/* �f�[�^��M */
			if (dwEvent & EV_RXCHAR) {
				if (!ReadFile(hComm, &uRsRcvData[dwRsRcvCount],
					comstat.cbInQue, &dwRcvCount, &Overlap)) {
					if (GetLastError() == ERROR_IO_PENDING) {
						GetOverlappedResult(hComm, &Overlap, &dwTransfer,
							TRUE);
					}
				}

				/* ��M�f�[�^���������ꍇ�AIRQ�𔭐� */
				dwRsRcvCount += dwRcvCount;
				if ((dwRsRcvCount > 0) && (rs_command & RSC_RXE)) {
					rs232c_rxrdy(TRUE);
				}
			}

			/* �f�[�^���M���� */
			if (dwEvent & EV_TXEMPTY) {
				rs_status |= RSS_TXEMPTY;
			}
		}

		Sleep(1);
	}

	/* �X���b�h�I���v���t���O���~�낷 */
	bCommCloseReq = FALSE;

	/* �I�[�o�[���b�v�C�x���g���폜 */
	DeleteObject(Overlap.hEvent);

	/* �N���e�B�J���Z�N�V�������폜 */
	DeleteCriticalSection(&CSection);
	bCriticalSection = FALSE;

	/* �X���b�h�I�� */
	_endthreadex(TRUE);

	return TRUE;
}

#endif	/* RSC */
#endif	/* _WIN32 */
