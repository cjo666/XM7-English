/*
 *	FM-7 EMULATOR "XM7"
 *
 *	Copyright (C) 1999-2017 ＰＩ．(Twitter:@xm6_original)
 *	Copyright (C) 2001-2017 Ryu Takegami (Twitter:@RyuTakegami)
 *
 *	[ Win32API シリアルポート ]
 *
 *	RHG履歴
 *	  2003.08.28		新設(中身なし)
 *	  2003.09.30		機能実装
 *	  2003.11.01		デバイスオープンに失敗した場合の処理を追加
 *	  2011.07.26		スレッド作成・終了関数を_beginthreadex/_endthreadexに
 *						変更
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
 *	グローバル ワーク
 */
int nCommPortNo;						/* 使用するシリアルポート番号 */

/*
 *	スタティック ワーク
 */
static CRITICAL_SECTION CSection;		/* クリティカルセクション */
static HANDLE hThreadRcv;				/* 受信スレッドハンドル */
static UINT uThResult;					/* スレッド戻り値 */
static BOOL bCommCloseReq;				/* スレッド終了要求フラグ */
static BOOL bCriticalSection;

static HANDLE hComm;					/* シリアルポートハンドル */
static int nOpenPort;					/* 使用中のポート番号 */
static COMMPROP commprop;				/* */
static OVERLAPPED Overlap;				/* OVERLAPPED構造体 */
static DCB dcb;							/* DCB構造体 */
static COMMTIMEOUTS CommTimeout;		/* TIMEOUT構造体 */

extern BYTE rs_status;					/* ステータスレジスタ */
static BYTE uRsRcvData[4096];			/* 受信データバッファ */
static DWORD dwRsRcvCount;				/* 受信データサイズ */
static DWORD dwRsRcvPoint;				/* 受信データ読み込み位置 */


/*
 *	プロトタイプ宣言
 */
static UINT WINAPI ThreadRcv(LPVOID);	/* スレッド関数 */

/*
 *	通信速度テーブル
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
 *	シリアルポート
 *	初期化
 */
BOOL FASTCALL InitCommPort(void)
{
	char buf[256];
	char string[256];
	char comportname[256];

	/* まずクリア */
	if (hComm != INVALID_HANDLE_VALUE) {
		CloseHandle(hComm);
	}
	hComm = INVALID_HANDLE_VALUE;
	rs_mask = TRUE;
	bCriticalSection = FALSE;

	/* rs_useがFALSEならシリアルポートは使用しない */
	if (!rs_use) {
		nOpenPort = 0;
		return FALSE;
	}

	/* とりあえず、作ってみる */
	_snprintf(comportname, sizeof(comportname), "COM%d", nCommPortNo);
	hComm = CreateFile(comportname,
				GENERIC_READ | GENERIC_WRITE, 0, NULL,
				OPEN_EXISTING, FILE_FLAG_OVERLAPPED, NULL);

	/* 対応デバイスチェック */
	if (hComm != INVALID_HANDLE_VALUE) {
		commprop.wPacketLength = sizeof(commprop);
		GetCommProperties(hComm, &commprop);
	}
	else {
		/* CreateFileに失敗しているのでデバイス種別を適当に */
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

	/* タイムアウト時間を設定 */
	CommTimeout.ReadIntervalTimeout = 0;
	CommTimeout.ReadTotalTimeoutMultiplier = 0;
	CommTimeout.ReadTotalTimeoutConstant = 0;
	CommTimeout.WriteTotalTimeoutMultiplier = 0;
	CommTimeout.WriteTotalTimeoutConstant = 0;
	SetCommTimeouts(hComm,&CommTimeout);

	/* 初期設定 */
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

	/* 受信スレッド作成 */
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

	/* オープンしたポート番号を保持 */
	nOpenPort = nCommPortNo;
	rs_mask = FALSE;

	return TRUE;
}

/*
 *	シリアルポート
 *	クリーンアップ
 */
void FASTCALL CleanCommPort(void)
{
	DWORD dwExitCode;

	/* スレッドが万一終了していなければ、終わらせる */
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
 *	セレクト状態チェック
 */
BOOL FASTCALL SelectCheckCommPort(int port)
{
	/* 使おうとしているポートと使用中のポートが一致していないかチェック */
	if (nOpenPort != port) {
		return FALSE;
	}

	return TRUE;
}

/*
 *	リセット時デバイスセレクト
 */
void FASTCALL rs232c_reset_notify(void)
{
	/* 使おうとしているポートと使用中のポートが一致していないかチェック */
	if (nOpenPort != nCommPortNo) {
		/* ポートが開かれている場合はいったんクリーンアップ */
		if (nOpenPort) {
			CleanCommPort();
		}

		/* 改めて初期化 */
		InitCommPort();
	}
}


/*
 *	ロック
 */
void FASTCALL LockRS(void)
{
	if (bCriticalSection) {
		EnterCriticalSection(&CSection);
	}
}

/*
 *	アンロック
 */
void FASTCALL UnlockRS(void)
{
	if (bCriticalSection) {
		LeaveCriticalSection(&CSection);
	}
}

/*
 *	データ送信
 */
void FASTCALL rs232c_senddata(BYTE dat)
{
	DWORD dwErrors;
	DWORD dwTransfer;
	COMSTAT comstat;

	/* RS-232C部が無効の場合は何もしない */
	if (rs_mask || hComm == INVALID_HANDLE_VALUE) {
		return;
	}

	/* TxE=1,CTS=Lの場合、送信が可能 */
	if (!rs_cts && (rs_command & RSC_TXEN) && (rs_status & RSS_TXRDY)) {
		/* ポーリングスレッドをロック */
		LockRS();

		/* いったんTxEMPTY/TxRDYを落とす */
		rs232c_txrdy(FALSE);

		/* 1バイト送信 */
		ClearCommError(hComm, &dwErrors, &comstat);
		if (!WriteFile(hComm, &dat, 1, NULL, &Overlap)) {
			if (GetLastError() == ERROR_IO_PENDING) {
				GetOverlappedResult(hComm, &Overlap, &dwTransfer, TRUE);
			}
		}

		/* TxRDY ON/TxEMPTY要求 */
		rs232c_txempty_request();

		/* ポーリングスレッドをアンロック */
		UnlockRS();
	}
}

/*
 *	データ受信
 */
BYTE FASTCALL rs232c_receivedata(void)
{
	BYTE dat;

	/* RS-232C部が無効の場合は何もしない */
	if (rs_mask || hComm == INVALID_HANDLE_VALUE) {
		return 0xff;
	}

	/* ポーリングスレッドをロック */
	LockRS();

	/* バッファからデータを取りだす */
	dat = uRsRcvData[dwRsRcvPoint];

	/* RxE=1の場合、受信が可能 */
	if (rs_command & RSC_RXE) {
		dwRsRcvPoint ++;

		/* データがまだある場合、RxRDY要求 */
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

	/* RxRDYを落とす */
	rs232c_rxrdy(FALSE);

	/* ポーリングスレッドをアンロック */
	UnlockRS();

	return dat;
}

/*
 *	ステータスレジスタ読み出し
 */
BYTE FASTCALL rs232c_readstatus(void)
{
	/* 現状では単純にステータスレジスタの内容を返すだけ */
	return rs_status;
}

/*
 *	モードコマンドレジスタ書き込み
 */
void FASTCALL rs232c_writemodecmd(BYTE dat)
{
	/* RS-232C部が無効の場合は何もしない */
	if (rs_mask || hComm == INVALID_HANDLE_VALUE) {
		return;
	}

	/* ポーリングスレッドをロック */
	LockRS();

	/* 現在の設定を取得 */
	dcb.DCBlength = sizeof(dcb);
	GetCommState(hComm, &dcb);

	/* ストップビット */
	switch (dat & RSM_STOPBITM) {
		case RSM_STOPBIT1	:	dcb.StopBits = ONESTOPBIT;
								break;
		case RSM_STOPBIT15	:	dcb.StopBits = ONE5STOPBITS;
								break;
		case RSM_STOPBIT2	:	dcb.StopBits = TWOSTOPBITS;
								break;
	}

	/* パリティ */
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

	/* キャラクタ長 */
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

	/* クロック分周 */
	if ((dat & RSM_BAUDDIVM) == RSM_BAUDDIV64) {
		dcb.BaudRate = dwBaudrateSlow[(rs_baudrate & RSCB_BAUDM) >> 2];
	}
	else {
		dcb.BaudRate = dwBaudrateFast[(rs_baudrate & RSCB_BAUDM) >> 2];
	}

	/* 設定 */
	SetCommState(hComm, &dcb);

	/* ポーリングスレッドをアンロック */
	UnlockRS();
}

/*
 *	コマンドレジスタ書き込み
 */
void FASTCALL rs232c_writecommand(BYTE dat)
{
	/* RS-232C部が無効の場合は何もしない */
	if (rs_mask || hComm == INVALID_HANDLE_VALUE) {
		return;
	}

	/* ポーリングスレッドをロック */
	LockRS();

	/* エラークリア */
	if (dat & RSC_ER) {
		rs_status &= (BYTE)~(RSS_FRAMEERR | RSS_OVERRUN | RSS_PARITYERR);
	}

	/* RTS制御 */
	if (dat & RSC_RTS) {
		EscapeCommFunction(hComm, SETRTS);
	}
	else {
		EscapeCommFunction(hComm, CLRRTS);
	}

	/* ブレーク制御 */
	if (dat & RSC_SBRK) {
		EscapeCommFunction(hComm, SETBREAK);
	}
	else {
		EscapeCommFunction(hComm, CLRBREAK);
	}

	/* DTR制御 */
	if ((dat & RSC_DTR) && !rs_dtrmask) {
		EscapeCommFunction(hComm, SETDTR);
	}
	else {
		EscapeCommFunction(hComm, CLRDTR);
	}

	/* ポーリングスレッドをアンロック */
	UnlockRS();
}

/*
 *	クロック・ボーレート設定レジスタ書き込み
 */
void FASTCALL rs232c_setbaudrate(BYTE dat)
{
	/* RS-232C部が無効の場合は何もしない */
	if (rs_mask || hComm == INVALID_HANDLE_VALUE) {
		return;
	}

	/* ポーリングスレッドをロック */
	LockRS();

	/* 現在の設定を取得 */
	dcb.DCBlength = sizeof(dcb);
	GetCommState(hComm, &dcb);

	/* ボーレートを設定 */
	if ((rs_modecmd & RSM_BAUDDIVM) == RSM_BAUDDIV64) {
		dcb.BaudRate = dwBaudrateSlow[(dat & RSCB_BAUDM) >> 2];
	}
	else {
		dcb.BaudRate = dwBaudrateFast[(dat & RSCB_BAUDM) >> 2];
	}

	/* 設定 */
	SetCommState(hComm, &dcb);

	/* ポーリングスレッドをアンロック */
	UnlockRS();
}

/*
 *	スレッド関数
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

	/* クリティカルセクションを作成 */
	InitializeCriticalSection(&CSection);

	/* ワークエリア初期化 */
	bCommCloseReq = FALSE;
	rs_status |= (RSS_TXRDY | RSS_TXEMPTY);
	dwRsRcvCount = 0;
	dwRsRcvPoint = 0;

	/* 非同期I/O用オーバーラップ構造体を初期化 */
	memset(&Overlap, 0, sizeof(Overlap));
	Overlap.hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

	/* イベントマスクを設定 */
	SetCommMask(hComm,
		EV_BREAK | EV_CTS | EV_DSR | EV_ERR | EV_RLSD | EV_RXCHAR | EV_TXEMPTY);

	/* クリティカルセクション制御許可 */
	bCriticalSection = TRUE;

	while (!bCloseReq) {
		WaitCommEvent(hComm, &dwEvent, &Overlap);
		if (GetLastError() == ERROR_IO_PENDING) {
			GetOverlappedResult(hComm, &Overlap, &dwTransfer, TRUE);
			ClearCommError(hComm, &dwErrors, &comstat);

			/* ブレーク検出 */
			if (dwEvent & EV_BREAK) {
				if (rs_command & RSC_EH) {
					rs232c_syndet(TRUE);
				}
			}

			/* CTS信号が変化 */
			if (dwEvent & EV_CTS) {
				GetCommModemStatus(hComm, &dwStat);
				if (dwStat & MS_CTS_ON) {
					rs_cts = TRUE;
				}
				else {
					rs_cts = FALSE;
				}
			}

			/* DSR信号が変化 */
			if (dwEvent & EV_DSR) {
				GetCommModemStatus(hComm, &dwStat);
				if (dwStat & MS_DSR_ON) {
					rs_status |= RSS_DSR;
				}
				else {
					rs_status &= (BYTE)~RSS_DSR;
				}
			}

			/* エラー発生 */
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

			/* CD信号が変化 */
			if (dwEvent & EV_RLSD) {
				GetCommModemStatus(hComm, &dwStat);
				if (dwStat & MS_RLSD_ON) {
					rs_cd = TRUE;
				}
				else {
					rs_cd = FALSE;
				}
			}

			/* データ受信 */
			if (dwEvent & EV_RXCHAR) {
				if (!ReadFile(hComm, &uRsRcvData[dwRsRcvCount],
					comstat.cbInQue, &dwRcvCount, &Overlap)) {
					if (GetLastError() == ERROR_IO_PENDING) {
						GetOverlappedResult(hComm, &Overlap, &dwTransfer,
							TRUE);
					}
				}

				/* 受信データがあった場合、IRQを発生 */
				dwRsRcvCount += dwRcvCount;
				if ((dwRsRcvCount > 0) && (rs_command & RSC_RXE)) {
					rs232c_rxrdy(TRUE);
				}
			}

			/* データ送信完了 */
			if (dwEvent & EV_TXEMPTY) {
				rs_status |= RSS_TXEMPTY;
			}
		}

		Sleep(1);
	}

	/* スレッド終了要求フラグを降ろす */
	bCommCloseReq = FALSE;

	/* オーバーラップイベントを削除 */
	DeleteObject(Overlap.hEvent);

	/* クリティカルセクションを削除 */
	DeleteCriticalSection(&CSection);
	bCriticalSection = FALSE;

	/* スレッド終了 */
	_endthreadex(TRUE);

	return TRUE;
}

#endif	/* RSC */
#endif	/* _WIN32 */
