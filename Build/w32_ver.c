/*
 *	FM-7 EMULATOR "XM7"
 *
 *	Copyright (C) 1999-2017 �o�h�D(Twitter:@xm6_original)
 *	Copyright (C) 2001-2017 Ryu Takegami (Twitter:@RyuTakegami)
 *
 *	[ Win32API �o�[�W�������_�C�A���O ]
 *
 *	RHG����
 *	  2002.05.07		�����N�摝��(�΂�
 *	  2002.07.17		�}�E�X�|�C���^�`�󂪕ς�����܂܂ɂȂ�����C��
 *	  2002.09.09		�o�[�W����������̐ݒ���@��ύX (VC++�΍�)
 *	  2010.05.11		�t���X�N���[�����̃����N��N���b�N�𖳌��ɂ���
 */

#ifdef _WIN32

#define STRICT
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <shellapi.h>
#include <assert.h>
#include "xm7.h"
#include "w32.h"
#include "w32_res.h"
#include "w32_draw.h"
#include "w32_kbd.h"

/*
 *	�O���[�o�� ���[�N
 */
BOOL bGravestone = FALSE;				/* !? */

/*
 *	�X�^�e�B�b�N ���[�N
 */
static RECT AboutURLRect[4];			/* URL�e�L�X�g��` */
static BOOL bAboutURLHit[4];			/* URL�t�H�[�J�X�t���O */
static char pszAboutURL[4][128];		/* URL�e�L�X�g */
static RECT IconRect;					/* �A�C�R����` */
static HICON hVerIcon;					/* �A�C�R���n���h�� */


/*
 *	�_�C�A���O������
 */
static void FASTCALL AboutDlgInit(HWND hDlg)
{
	/* �o�[�W���������� */
	#if XM7_VER == 2 && defined(FMTV151)
		#if defined(LEVEL)
			/* ���x������ */
			#define	VER		VERSION" "LEVEL"-V2�߂�"
			#define	VER_IN	VERSION" "LEVEL"-V2"
		#else
			/* ���x���Ȃ� */
			#define	VER		VERSION"-V2�߂�"
			#define	VER_IN	VERSION"-V2"
		#endif
	#else
		#if defined(LEVEL)
			/* ���x������ */
			#define	VER		VERSION" "LEVEL
			#define	VER_IN	VER
		#else
			/* ���x���Ȃ� */
			#define	VER		VERSION
			#define	VER_IN	VER
		#endif
	#endif

	#if defined(BETAVER)
		#if defined(BETANO)
			/* ��2�ȍ~ */
			#define	VERSTR		VER"��"BETANO" ("DATE")"
			#define	VERSTR_IN	VER_IN"-beta"BETANO" ("DATE")"
		#else
			/* ��1(�ԍ��\�L�Ȃ�) */
			#define	VERSTR		VER"�� ("DATE")"
			#define	VERSTR_IN	VER_IN"-beta ("DATE")"
		#endif
	#else
		/* ������ */
		#define	VERSTR			VER" ("DATE")"
		#define	VERSTR_IN		VER_IN" ("DATE")"
	#endif

	HWND hWnd;
	RECT prect;
	RECT drect;
	POINT point;
	int i;

	ASSERT(hDlg);

	/* �o�[�W�����������ݒ� */
	hWnd = GetDlgItem(hDlg, IDC_VERSION);
	SetWindowText(hWnd, VERSTR);

	/* �����񃊃\�[�X�����[�h */
#if defined(ROMEO)
	for (i=0; i<4; i++) {
#else
	for (i=0; i<3; i++) {
#endif
		pszAboutURL[i][0] = '\0';
		LoadString(hAppInstance, IDS_ABOUTURL + i, pszAboutURL[i],
			sizeof(pszAboutURL[i]));
	}

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

	/* IDC_URL�̃N���C�A���g���W���擾 */
#if defined(ROMEO)
	for (i=0; i<4; i++) {
#else
	for (i=0; i<3; i++) {
#endif
		hWnd = GetDlgItem(hDlg, IDC_URL + i);
		ASSERT(hWnd);
		GetWindowRect(hWnd, &prect);

		point.x = prect.left;
		point.y = prect.top;
		ScreenToClient(hDlg, &point);
		AboutURLRect[i].left = point.x;
		AboutURLRect[i].top = point.y;

		point.x = prect.right;
		point.y = prect.bottom;
		ScreenToClient(hDlg, &point);
		AboutURLRect[i].right = point.x;
		AboutURLRect[i].bottom = point.y;

		/* IDC_URL���폜 */
		DestroyWindow(hWnd);

		/* ���̑� */
		bAboutURLHit[i] = FALSE;
	}

	/* IDC_ABOUTICON�̃N���C�A���g���W���擾 */
	hWnd = GetDlgItem(hDlg, IDC_ABOUTICON);
	ASSERT(hWnd);
	GetWindowRect(hWnd, &prect);

	point.x = prect.left;
	point.y = prect.top;
	ScreenToClient(hDlg, &point);
	IconRect.left = point.x;
	IconRect.top = point.y;

	point.x = prect.right;
	point.y = prect.bottom;
	ScreenToClient(hDlg, &point);
	IconRect.right = point.x;
	IconRect.bottom = point.y;

	/* IDC_ABOUTICON���폜 */
	DestroyWindow(hWnd);

	/* �A�C�R�������[�h */
#if XM7_VER >= 2
	switch (fm7_ver) {
		case 1:
			hVerIcon = LoadIcon(hAppInstance, MAKEINTRESOURCE(IDI_FM7ICON));
			break;
		case 2:
			hVerIcon = LoadIcon(hAppInstance, MAKEINTRESOURCE(IDI_77AVICON));
			break;
#if XM7_VER >= 3
		case 3:
#if defined(DEBUG)
			hVerIcon = LoadIcon(hAppInstance, MAKEINTRESOURCE(IDI_40SXICON));
#else
			if (bGravestone) {
				hVerIcon = LoadIcon(hAppInstance,
					 MAKEINTRESOURCE(IDI_40SXICON));
			}
			else {
				hVerIcon = LoadIcon(hAppInstance,
					 MAKEINTRESOURCE(IDI_40EXICON));
			}
#endif
			break;
#endif
		default:
			hVerIcon = NULL;
			ASSERT(FALSE);
			break;
	}
#else
	switch (fm_subtype) {
		case FMSUB_FM8:
			hVerIcon = LoadIcon(hAppInstance, MAKEINTRESOURCE(IDI_FM8ICON));
			break;
		case FMSUB_FM7:
			hVerIcon = LoadIcon(hAppInstance, MAKEINTRESOURCE(IDI_FM7ICON));
			break;
		case FMSUB_FM77:
		default:
			hVerIcon = LoadIcon(hAppInstance, MAKEINTRESOURCE(IDI_FM77ICON));
			break;
	}
#endif

	ASSERT(hVerIcon);

	#undef VER
}

/*
 *	�_�C�A���O�`��
 */
static void AboutDlgPaint(HWND hDlg)
{
	HDC hDC;
	PAINTSTRUCT ps;
	HFONT hFont;
	HFONT hDefFont;
	TEXTMETRIC tm;
	LOGFONT lf;
	int i;

	ASSERT(hDlg);

	/* DC���擾 */
	hDC = BeginPaint(hDlg, &ps);

	/* GUI�t�H���g�̃��g���b�N�𓾂� */
	hFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
	hDefFont = (HFONT)SelectObject(hDC, hFont);
	GetTextMetrics(hDC, &tm);
	memset(&lf, 0, sizeof(lf));
	GetTextFace(hDC, LF_FACESIZE, lf.lfFaceName);
	SelectObject(hDC, hDefFont);

	/* �A���_�[���C����t�������t�H���g���쐬 */
	lf.lfHeight = tm.tmHeight;
	lf.lfWidth = 0;
	lf.lfEscapement = 0;
	lf.lfOrientation = 0;
	lf.lfWeight = FW_DONTCARE;
	lf.lfItalic = tm.tmItalic;
	lf.lfUnderline = TRUE;
	lf.lfStrikeOut = tm.tmStruckOut;
	lf.lfCharSet = tm.tmCharSet;
	lf.lfOutPrecision = OUT_DEFAULT_PRECIS;
	lf.lfClipPrecision = CLIP_DEFAULT_PRECIS;
	lf.lfQuality = DEFAULT_QUALITY;
	lf.lfPitchAndFamily = tm.tmPitchAndFamily;
	hFont = CreateFontIndirect(&lf);
	hDefFont = (HFONT)SelectObject(hDC, hFont);

	/* �`�� */
#if defined(ROMEO)
	for (i=0; i<4; i++) {
#else
	for (i=0; i<3; i++) {
#endif
		if (bAboutURLHit[i]) {
			SetTextColor(hDC, RGB(255, 0, 0));
		}
		else {
			SetTextColor(hDC, RGB(0, 0, 255));
		}
		SetBkColor(hDC, GetSysColor(COLOR_3DFACE));
		SetBkMode(hDC, OPAQUE);
		DrawText(hDC, pszAboutURL[i], strlen(pszAboutURL[i]),
			&AboutURLRect[i], DT_CENTER | DT_SINGLELINE);
	}

	/* �t�H���g��߂� */
	SelectObject(hDC, hDefFont);
	DeleteObject(hFont);

	/* �A�C�R���`�� */
	DrawIcon(hDC, IconRect.left, IconRect.top, hVerIcon);

	/* DC��� */
	EndPaint(hDlg, &ps);
}

/*
 *	�_�C�A���O�v���V�[�W��
 */
static BOOL CALLBACK AboutDlgProc(HWND hDlg, UINT iMsg,
									WPARAM wParam, LPARAM lParam)
{
	POINT point;
	BOOL bFlag;
	HCURSOR hCursor;
	int i;

	switch (iMsg) {
		/* �_�C�A���O������ */
		case WM_INITDIALOG:
			AboutDlgInit(hDlg);
			return TRUE;

		/* �ĕ`�� */
		case WM_PAINT:
			AboutDlgPaint(hDlg);
			return 0;

		/* �R�}���h */
		case WM_COMMAND:
			switch (LOWORD(wParam)) {
				/* �I�� */
				case IDOK:
				case IDCANCEL:
					DestroyIcon(hVerIcon);
					EndDialog(hDlg, 0);
					InvalidateRect(hDrawWnd, NULL, FALSE);
					return TRUE;
			}
			break;

		/* �̈�`�F�b�N */
		case WM_NCHITTEST:
			point.x = LOWORD(lParam);
			point.y = HIWORD(lParam);
			ScreenToClient(hDlg, &point);
			/* �o�h�D���T�C�g�΍� */
#if defined(ROMEO)
			for (i=1; i<4; i++) {
#else
			for (i=1; i<3; i++) {
#endif
				bFlag = PtInRect(&AboutURLRect[i], point);
				/* �t���O���قȂ�����A�X�V���čĕ`�� */
				if (bFlag != bAboutURLHit[i]) {
					bAboutURLHit[i] = bFlag;
					InvalidateRect(hDlg, &AboutURLRect[i], FALSE);
				}
			}
			break;

		/* �J�[�\���ݒ� */
		case WM_SETCURSOR:
			/* �o�h�D���T�C�g�΍� */
#if defined(ROMEO)
			for (i=1; i<4; i++) {
#else
			for (i=1; i<3; i++) {
#endif
				if (bAboutURLHit[i]) {
					/* OS�̃o�[�W�����ɂ���Ă�IDC_HAND�͎��s���� */
					hCursor = LoadCursor(NULL, MAKEINTRESOURCE(32649));
					if (!hCursor) {
						hCursor = LoadCursor(NULL, IDC_IBEAM);
					}
					SetCursor(hCursor);
					return TRUE;
				}
			}
			break;

		case WM_LBUTTONDOWN:
		case WM_LBUTTONDBLCLK:
			/* �t���X�N���[�����̓u���E�U���N�����Ȃ� */
			if (bFullScreen) {
				break;
			}
			
#if defined(ROMEO)
			for (i=1; i<4; i++) {
#else
			for (i=1; i<3; i++) {
#endif
				if (bAboutURLHit[i]) {
					ShellExecute(hDlg, NULL, pszAboutURL[i], NULL, NULL,
						SW_SHOWNORMAL);
					return TRUE;
				}
			}
			break;
	}

	/* ����ȊO��FALSE */
	return FALSE;
}

/*
 *	�o�[�W�������
 */
void FASTCALL OnAbout(HWND hWnd)
{
	ASSERT(hWnd);

	/* ���[�_���_�C�A���O���s */
	DialogBox(hAppInstance, MAKEINTRESOURCE(IDD_ABOUTDLG), hWnd, AboutDlgProc);
	SetMenuExitTimer();
}

#endif	/* _WIN32 */
