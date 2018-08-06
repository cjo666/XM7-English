/*
 *	FM-7 EMULATOR "XM7"
 *
 *	Copyright (C) 1999-2017 ＰＩ．(Twitter:@xm6_original)
 *	Copyright (C) 2001-2017 Ryu Takegami (Twitter:@RyuTakegami)
 *	line printer support 2010-2013 by Ben.JP
 *
 *	[ Win32API プリンタ出力 ]
 */

#ifdef _WIN32

/* Borland C++におけるインライン展開できないWarningの抑制 */
#ifdef __BORLANDC__
#pragma warn -inl
#endif

#if defined(LPRINT)

#define STRICT
//#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <assert.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//#include <strsafe.h>	// for StringCbPrintf(), etc.	/* BC++には存在しないようです… */
#include <io.h>
#include <fcntl.h>
#include <sys/stat.h>

#include "xm7.h"
#include "device.h"
#include "w32.h"
#include "w32_lpr.h"
#include "w32_sch.h"
#include "w32_snd.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 *	グローバル ワーク
 */
// GLOBAL variables
BOOL lpr_use_os_font;					/* OSのフォントを利用する */ // use OS fonts for printing
BOOL lpr_output_kanji;					/* 漢字を出力する */ // output kanji from printer

/*
 *	スタティック ワーク
 */
// STATIC defines, constants & variables
#define TABSTOP 8
#define N_CHAR_FONTS (256)

static const int TWIPS_PER_INCH = 1440;	/* 1インチ辺りのTWIP数 */ // twips per inch
static const double MM_PER_INCH = 25.4;	/* 1インチ辺りのミリメートル数 */ // milimeters per inch
static const int PT_PER_INCH = 72;		/* 1インチ辺りのポイント数 */ // points per inch

static const int DEF_LP_UNIT = 72;		/* 規定の改行幅の単位 */ // 1/DEF_LP_UNIT inch
static const int DEF_IPLPU = 12;		/* 規定の改行幅（1/lp_unitインチ単位） */ // default value per line in the unit of 1/lp_unit-inch
static const int DEF_CPI = 10;			/* 規定のインチ辺り文字数 */ // default charactors per inch
static const int DEF_LPI = 6;			/* 規定のインチ辺り行数 */ // default lines per inch
static const int DEF_AREA_WIDTH = 8;	/* 規定の印刷領域の幅（インチ単位） */ // default area width per inch
static const int DEF_AREA_HEIGHT = 11;	/* 規定の印刷領域の高さ（インチ単位） */ // default area height per inch
static const double DEF_MARGIN_LEFT_MM = 3.4;	/* 規定の左マージン（mm単位） */ // default left-margin [mm]
static const double DEF_MARGIN_TOP_MM = 11.7;	/* 規定の上マージン（mm単位） */ // default top-margin [mm]
//static const double DEF_MARGIN_BOTTOM_MM = 5.9;	/* 規定の下マージン（mm単位） */ // default bottom-margin [mm]
static const int DEF_NUM_COLUMNS = 80;	/* 規定の桁数 */

static BYTE CharFontImg[N_CHAR_FONTS][8];	/* 文字のビットイメージ */ // bit images of charactors

/*
 *	ユーティリティ関数
 */
// utility functions
static const int ROUND(const double val) {
	return (int)(val+0.5);
}

static const int TRUNC(const double val) {
	return (int)(val);
}

/*
 *	デバッグ／エラー メッセージ
 */
// DEBUG error message
#if defined(DEBUG_LPRINT)
#define PRINT_ERROR(s) print_error(s)
#define PRINT_DEBUG(s) print_debug(s)

void
print_error(const char *funcname)
{
	LPVOID lpMsgBuf;
	FormatMessage(
	    FORMAT_MESSAGE_ALLOCATE_BUFFER |
	    FORMAT_MESSAGE_FROM_SYSTEM |
	    FORMAT_MESSAGE_IGNORE_INSERTS,
	    NULL,
	    GetLastError(),
	    MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
	    (LPTSTR)&lpMsgBuf,
	    0,
	    NULL
	);
	MessageBox(NULL, (LPCTSTR)lpMsgBuf, (LPCTSTR)funcname, MB_OK | MB_ICONEXCLAMATION);
	LocalFree(lpMsgBuf);
}

void
print_debug(const LPVOID lpMsgBuf)
{
	MessageBox(NULL, (LPCTSTR)lpMsgBuf, (LPCTSTR)"DEBUG", MB_OK | MB_ICONEXCLAMATION);
}
#else
#define PRINT_ERROR(s) 
#define PRINT_DEBUG(s) 
#endif

/*
 *	スタティック関数
 */
// static functions

// numer/denom INCHES -> TWIPS
static const int
INCHES2TWIPS(const int numer, const int denom)
{
	return ROUND((double)TWIPS_PER_INCH*numer/denom);
}

// POINTS -> TWIPS
static const int
POINTS2TWIPS(const double pt)
{
	return ROUND(TWIPS_PER_INCH*pt/PT_PER_INCH);
}

/* プリントバッファ */ // print-buffer
#define N_MAX_PRT_BUF_LEN (256)

typedef struct {
	BYTE code;			/* 文字コード */
	int attr;			/* 文字の属性 */
} PrtBuffChar;

/* フォント */ // fonts
const double PT_FONT = 9.;				/* フォントのポイント数 */ // points of font
const double PT_FONT_KANJI = 9.;		/* 漢字フォントのポイント数 */ // points of kanji font

enum AnkFont {
	FONT_ANK_NORMAL=0,	/* ANKフォント：ノーマル */

	N_NUM_ANKFONT,
	FONT_ANK_PREV=N_NUM_ANKFONT	/* ANKフォント：前回 */
};

enum KanjiFont {
	FONT_KANJI_NORMAL=0,	/* 漢字フォント：ノーマル */

	N_NUM_KANJIFONT
};

enum FontAttr {
	ATTR_WIDE_CHAR = (1<<0),
	ATTR_KANJI = (1<<31)
};	/* フォント属性 */ // font attribute


#ifdef __cplusplus
}
#endif


/* プリントバッファ */ // print-buffer

class LprPrintBuffer {

public:
	LprPrintBuffer()
	{
	}

	~LprPrintBuffer()
	{
	}

public:
	/* 一文字消去 */ // clear one charactor
	void Clear(const int i)
	{
		prtBuff[i].code = 0;
		prtBuff[i].attr = 0;
	}

	/* 全文字消去 */ // clear all charactors
	void ClearAll(void)
	{
		for (int i=0; i < N_MAX_PRT_BUF_LEN; i++) {
			Clear(i);
		}

		numCurrentChars = 0;
	}

	/* バッファが空か否か */ // if buffer is empty
	BOOL IsEmpty(void)
	{
		return (numCurrentChars == 0);
	}

	/* バッファがフルか否か */ // if buffer is full
	BOOL IsBufferFull(void)
	{
		return (numCurrentChars >= numColumns);
	}

	/* 一文字追加 */ // add one charactor
	void AddChar(const BYTE code, const int attr)
	{
		assert(numCurrentChars >= 0 && numCurrentChars < N_MAX_PRT_BUF_LEN);
		assert((code >= 0x20 && code <= 0x7e) || (code >= 0x80));

		prtBuff[numCurrentChars].code = code;
		prtBuff[numCurrentChars].attr = attr;
		numCurrentChars++;
	}

	/* バッファ末尾の文字を消去 */ // delete one charactor last added
	BOOL DeleteLastChar(void)
	{
		assert(numCurrentChars >= 0 && numCurrentChars <= N_MAX_PRT_BUF_LEN);

		if (numCurrentChars > 0) {
			numCurrentChars--;
			Clear(numCurrentChars);
			return TRUE;
		}

		return FALSE;
	}

	/* 位置indexの一文字 */ // one charactor at index
	PrtBuffChar GetAt(const int index)
	{
		PrtBuffChar buffChar;

		assert(index >= 0 && index < N_MAX_PRT_BUF_LEN);

		buffChar.code = 0;

		if (index >= 0 && index < numColumns) {
			buffChar.code = prtBuff[index].code;
			buffChar.attr = prtBuff[index].attr;
		}

		return buffChar;
	}

	/* 桁数の設定 */ // set # of columns
	void setNumColumns(const int num)
	{
		numColumns = num;
	}

	/* 桁数の取得 */ // get # of columns
	int getNumColumns(void)
	{
		return numColumns;
	}

	/* 現在の桁数の取得 */ // get current # of columns
	int getNumCurrentChars(void)
	{
		return numCurrentChars;
	}

private:
	PrtBuffChar prtBuff[N_MAX_PRT_BUF_LEN];	/* プリントバッファ */
	int numColumns;					/* バッファの桁数 */
	int numCurrentChars;			/* バッファ中の現在の文字数 */
};


/* フォント */ // fonts
class LprFonts {

private:
	HFONT hFontAnk[N_NUM_ANKFONT+1];	/* フォントハンドル（ANK）：(+1)はFONT_ANK_PREVの分 */ // font handle (ANK)
	HFONT hFontKanji[N_NUM_KANJIFONT];	/* フォントハンドル（漢字） */ // font handle (KANJI)

public:
	LprFonts()
	{
	}

	~LprFonts()
	{
	}

public:
	/* フォントの作成 */
	BOOL create(HDC hdc, SIZE& sizeChar)
	{
		LOGFONT lf;
		HFONT hf;

		(void)hdc;

		/* ANKフォント：直前 */
		hFontAnk[FONT_ANK_PREV] = (HFONT)GetStockObject(SYSTEM_FONT);

		/* ANKフォント：ノーマル */
		ZeroMemory(&lf, sizeof(lf));
		lf.lfHeight = POINTS2TWIPS(PT_FONT);
		lf.lfCharSet = SHIFTJIS_CHARSET;
		lf.lfPitchAndFamily = FIXED_PITCH;
		hf = CreateFontIndirect(&lf);
		if (hf == NULL) {
			PRINT_ERROR("CreateFontIndirect()");
			return FALSE;
		}
		hFontAnk[FONT_ANK_NORMAL] = hf;

		/* 文字サイズ(ANK)の設定 */
		sizeChar.cx = INCHES2TWIPS(1, DEF_CPI);
		sizeChar.cy = lf.lfHeight;

		/* 漢字フォント：ノーマル */
		ZeroMemory(&lf, sizeof(lf));
		lf.lfHeight = POINTS2TWIPS(PT_FONT_KANJI);
		lf.lfCharSet = SHIFTJIS_CHARSET;
		lf.lfPitchAndFamily = FIXED_PITCH;
		hf = CreateFontIndirect(&lf);
		if (hf == NULL) {
			PRINT_ERROR("CreateFontIndirect()");
			return FALSE;
		}
		hFontKanji[FONT_KANJI_NORMAL] = hf;

#if defined(DEBUG_LPRINT)
	//	{
	//	TCHAR msg[1024];
	//	StringCbPrintf(msg, sizeof(msg), "sizeChar=(%d,%d)\n", sizeChar.cx,sizeChar.cy);
	//		/* StringCbPrintf()等はBC++には存在しないようです… */
	//	_snprintf(msg, sizeof(msg), "sizeChar=(%d,%d)\n", sizeChar.cx,sizeChar.cy);
	//	PRINT_DEBUG(msg);
	//	}
#endif

		return TRUE;
	}

	/* ANKフォントの選択 */
	BOOL select_ank(HDC hdc, enum AnkFont index)
	{
		HFONT hf, hfo;

		hf = hFontAnk[index];
		hfo = (HFONT)SelectObject(hdc, hf);
		if (hfo == NULL) {
			PRINT_ERROR("fonts.select_ank.SelectObject()");
			return FALSE;
		}
		hFontAnk[FONT_ANK_PREV] = hfo;

#if defined(DEBUG_LPRINT)
	//	{
	//		TCHAR msg[1024];
	//		TCHAR szFaceName[LF_FACESIZE];
	//		ZeroMemory(msg, sizeof(msg));
	//		if (GetTextFace(hdc, LF_FACESIZE, szFaceName)) {
	//			_snprintf(msg, sizeof(msg), "FaceName: %s\n", szFaceName);
	//		}
	//		PRINT_DEBUG(msg);
	//	}
#endif

		return TRUE;
	}

	/* 漢字フォントの選択 */
	BOOL select_kanji(HDC hdc, enum KanjiFont index)
	{
		HFONT hf, hfo;

		hf = hFontKanji[index];
		hfo = (HFONT)SelectObject(hdc, hf);
		if (hfo == NULL) {
			PRINT_ERROR("fonts.select_kanji.SelectObject()");
			return FALSE;
		}

		return TRUE;
	}

	/* フォントの削除 */
	BOOL delete_f(HDC hdc)
	{
		HFONT hf;
		int i;

		hf = (HFONT)GetStockObject(SYSTEM_FONT);
		if (hf == NULL) {
			PRINT_ERROR("fonts.delete_f.GetStockObject()");
		}

		if (SelectObject(hdc, hf) == NULL) {
			PRINT_ERROR("fonts.delete_f.SelectObject()");
		}

		for (i=0; i < N_NUM_ANKFONT; i++) {
			if (!DeleteObject(hFontAnk[i])) {
				PRINT_ERROR("fonts.delete_f.DeleteObject()");
			}
		}

		for (i=0; i < N_NUM_KANJIFONT; i++) {
			if (!DeleteObject(hFontKanji[i])) {
				PRINT_ERROR("fonts.delete_f.DeleteObject()");
			}
		}

		return TRUE;
	}

};

/* 描画ルーチン */ // Renderer

class LprRenderer {

private:
	TCHAR *szFullPathName;			/* 印刷用一時ファイルのフルパス名 */ // full path name of temporary file for printing
	HANDLE hFile;					/* 印刷用一時ファイルのハンドル */ // handle of temporary file for printing

	int x, y;						/* 印字位置座標(x,y)（ピクセル単位） */ // printing position (x,y) in pixels
	SIZE sizeArea;	 				/* 印字領域 */ // size of device area
	SIZE sizePrintable;	 			/* 印字可能領域 */ // size of printable area
	SIZE sizeChar;	 				/* 文字のSIZE */ // size of one charactor
	SIZE sizeSpacing;	 			/* 文字SPACINGのSIZE */ // size of char-spacing
	int nMarginTop; 					/* 上マージン値 */ // height of top-margin
	int nMarginBottom; 				/* 下マージン値 */ // height of bottom-margin

	int lp_unit;						/* 改行幅の単位（1/lp_unitインチ） */ // 1/lp_unit inch
	int cpi;							/* インチ辺り文字数 */ // charactors per inch
	int lpi;							/* インチ辺り行数 */ // lines per inch

	int fKanji;							/* 漢字モードフラグ（実際はカウンタ） */ // KANJI mode flag (counter)
	BOOL bWideChar;						/* 拡大文字フラグ */

private:
	/* プリントバッファ */ // print-buffer
	LprPrintBuffer prtBuff;

	/* フォント */ // fonts
	LprFonts *fonts;

public:
	LprRenderer(TCHAR *s, HANDLE h)
	 : prtBuff()
	{
		szFullPathName = s;
		hFile = h;
	}

	~LprRenderer()
	{
	}

private:
	/* 印刷可能領域の設定 */
	BOOL set_printable_area(HDC hdc)
	{
		SIZE sz_d, sz_dm, sz_dp;
		SIZE sz_lm;

		sz_d.cx = GetDeviceCaps(hdc, PHYSICALWIDTH);
		sz_d.cy = GetDeviceCaps(hdc, PHYSICALHEIGHT);

		sz_dm.cx = GetDeviceCaps(hdc, PHYSICALOFFSETX);
		sz_dm.cy = GetDeviceCaps(hdc, PHYSICALOFFSETY);

		sz_dp.cx = sz_d.cx - (sz_dm.cx << 1);
		sz_dp.cy = sz_d.cy - sz_dm.cy;

#if defined(DEBUG_LPRINT)
	//	{
	//	TCHAR msg[1024];
	//	_snprintf(msg,sizeof(msg),"sz_d=(%d,%d),sz_dm=(%d,%d),sz_dp=(%d,%d)\n",
	//		sz_d.cx,sz_d.cy, sz_dm.cx,sz_dm.cy, sz_dp.cx,sz_dp.cy);
	//	PRINT_DEBUG(msg);
	//	}
#endif

		sizeArea.cx = TWIPS_PER_INCH * DEF_AREA_WIDTH;
		sizeArea.cy = TWIPS_PER_INCH * DEF_AREA_HEIGHT;

		sz_lm.cx = TRUNC(TWIPS_PER_INCH*DEF_MARGIN_LEFT_MM/MM_PER_INCH);
		sz_lm.cy = TRUNC(TWIPS_PER_INCH*DEF_MARGIN_TOP_MM/MM_PER_INCH);

		if (!DPtoLP(hdc, (LPPOINT)&sz_dm, 1)) {
			PRINT_ERROR("DPtoLP()");
			return FALSE;
		}
		sz_dm.cy = -(sz_dm.cy);

		sz_lm.cx -= sz_dm.cx;
		sz_lm.cy -= sz_dm.cy;

		if (!SetWindowOrgEx(hdc, -sz_lm.cx, sz_lm.cy, NULL)) {
			PRINT_ERROR("SetWindowOrgEx()");
			return FALSE;
		}

		if (!DPtoLP(hdc, (LPPOINT)&sz_dp, 1)) {
			PRINT_ERROR("DPtoLP()");
			return FALSE;
		}
		sz_dp.cy = -(sz_dp.cy);

		if (sizeArea.cx > sz_dp.cx) {
			sizeArea.cx = sz_dp.cx;
		}
		if (sizeArea.cy > sz_dp.cy) {
			sizeArea.cy = sz_dp.cy;
		}

	#if defined(DEBUG_LPRINT)
	//	{	// DEBUG
	//	TCHAR msg[1024];
	//	_snprintf(msg, sizeof(msg), "sizeArea=(%d,%d), sz_lm=(%d,%d)\n",
	//		sizeArea.cx,sizeArea.cy, sz_lm.cx,sz_lm.cy);
	//	PRINT_DEBUG(msg);
	//	}
	#endif

		return TRUE;
	}

	/* 印字パラメータのリセット */
	BOOL reset_params(HDC hdc)
	{
		Kanji_Clear();
		x = 0;
		y = 0;

		sizePrintable = sizeArea;

		lp_unit = DEF_LP_UNIT;
		cpi = DEF_CPI;
		lpi = DEF_LPI;

		sizeSpacing.cx = INCHES2TWIPS(1, cpi);
		sizeSpacing.cy = INCHES2TWIPS(1, lpi);

	#if defined(DEBUG_LPRINT)
	//	{
	//	TCHAR msg[1024];
	//	_snprintf(msg,sizeof(msg),"sizeSpacing=(%d,%d)\n", sizeSpacing.cx,sizeSpacing.cy);
	//	PRINT_DEBUG(msg);
	//	}
	#endif

		prtBuff.setNumColumns(DEF_NUM_COLUMNS);
		prtBuff.ClearAll();

		if (!fonts->select_ank(hdc, FONT_ANK_NORMAL)) {
			return FALSE;
		}

		return TRUE;
	}

	/*
	 *	グラフィック描画関数
	 */
	// GRAPHIC DRAWING FUNCTIONS

	/* プリンタの1ドットを描画する */ // draw one dot of printer
	BOOL draw_one_dot(HDC hdc, const int left, const int top)
	{
		HBRUSH hbr, hbrOrg;
		int right, bottom;

		hbr = (HBRUSH)GetStockObject(BLACK_BRUSH);
		hbrOrg = (HBRUSH)SelectObject(hdc, hbr);

		right = left + INCHES2TWIPS(1, 60);
		bottom = top + INCHES2TWIPS(1, 72);

		if (!Ellipse(hdc, left, -top, right, -bottom)) {
			PRINT_ERROR("Ellipse()");
			return FALSE;
		}

		SelectObject(hdc, hbrOrg);

		return TRUE;
	}

	/* 1文字をフォントを使って描画する */ // draw one charactor-text
	BOOL draw_one_char_text(HDC hdc, const BYTE data)
	{
		TCHAR tch;

		tch = (TCHAR)data;
		if (!TextOut(hdc, x, -y, &tch, 1)) {
			PRINT_ERROR("TextOut()");
			return FALSE;
		}

		return TRUE;
	}

	/* 1文字のドットイメージを描画する */ // draw one charactor-image
	BOOL draw_one_char_image(HDC hdc, const int code)
	{
		int i, j;
		int left, top;
		int index;

		index = code;
		assert(index < N_CHAR_FONTS);

		for (i=0; i < 8; i++) {
			for (j=0; j < 8; j++) {
				if (CharFontImg[index][i] & (1<<(7-j))) {
					left = x + ((j*sizeChar.cx)>>3);
					top  = y + ((i*sizeChar.cy)>>3);
					if (!draw_one_dot(hdc, left, top)) {
						return FALSE;
					}
				}
			}
		}

		return TRUE;
	}

	/* 1文字消去する */ // erase one charactor
	BOOL erase_one_char(HDC hdc)
	{
		HBRUSH hbr;
		RECT rect;

		hbr = (HBRUSH)GetStockObject(WHITE_BRUSH);

		rect.left = x;
		rect.top = -y;
		rect.right = x + sizeSpacing.cx;
		rect.bottom = -(y + sizeSpacing.cy);

		if (!FillRect(hdc, &rect, hbr)) {
			PRINT_ERROR("FillRect()");
			return FALSE;
		}

		return TRUE;
	}

	/* 印字可能文字 */ // printable charactors
	BOOL draw_printable_char(HDC hdc, const BYTE data)
	{
		if (lpr_use_os_font) {
			if (!draw_one_char_text(hdc, data)) {
				return FALSE;
			}
		}
		else {
			if (!draw_one_char_image(hdc, data)) {
				return FALSE;
			}
		}
		x += sizeSpacing.cx;

		return TRUE;
	}

	/* グラフィック文字 */ // graphic charactors
	BOOL draw_graph_char(HDC hdc, const BYTE data)
	{
		if (!draw_one_char_image(hdc, data)) {
			return FALSE;
		}
		x += sizeSpacing.cx;

		return TRUE;
	}

	/* 漢字関数 */ // KANJI functions

	void jis2sjis(BYTE jis[2])
	{
		BYTE sjis[2];

		sjis[0] = jis[0];
		sjis[1] = jis[1];

		if(sjis[0] & 1)
			sjis[1] += (BYTE)0x1f;
		else
			sjis[1] += (BYTE)0x7d;

		sjis[0]=(BYTE)(((sjis[0]-0x21)>>1) + 0x81);

		if(sjis[0] >= 0xa0) sjis[0] += (BYTE)0x40;
		if(sjis[1] >= 0x7f) sjis[1]++;

		jis[0] = sjis[0];
		jis[1] = sjis[1];
	}

	BOOL draw_kanji(HDC hdc, BYTE *d2)
	{
		TCHAR t2[2];

		jis2sjis(d2);
		t2[0] = d2[0];
		t2[1] = d2[1];
		if (!TextOut(hdc, x, -y, t2, 2)) {
			PRINT_ERROR("TextOut()");
			return FALSE;
		}
		x += sizeSpacing.cx << 1;

		return TRUE;
	}

	BOOL Kanji_Is_ON(void)
	{
		return (fKanji > 0);
	}

	void Kanji_Clear(void)
	{
		fKanji = 0;
	}

	/* フォントの属性 */ // font attribute
	int attribute(void)
	{
		int attr = 0;
		if (bWideChar) attr |= ATTR_WIDE_CHAR;
		if (Kanji_Is_ON()) attr |= ATTR_KANJI;
		return attr;
	}

	/* ヘルパ関数 */ // helper functions

	BOOL Space(HDC hdc)
	{
		if (!draw_printable_char(hdc, 0x20)) {
			return FALSE;
		}

		return TRUE;
	}

	BOOL NewLine(HDC hdc)
	{
		(void)hdc;

		x = 0;
		y += sizeSpacing.cy;

		return TRUE;
	}

	BOOL NewPage(HDC hdc)
	{
		if (!EndPage(hdc)) {
			PRINT_ERROR("EndPage()");
			return FALSE;
		}
		if (!StartPage(hdc)) {
			PRINT_ERROR("StartPage()");
			return FALSE;
		}

		x = 0;
		y = 0;

		return TRUE;
	}

	/* 一文字印字 */
	BOOL PrintChar(HDC hdc, int *index)
	{
		BYTE data, d2[2];
		int attr;
		int i;
		PrtBuffChar buffChar;

		i = *index;
		buffChar = prtBuff.GetAt(i);
		data = buffChar.code;
		attr = buffChar.attr;

		// auto NewLine and NewPage
		if (sizePrintable.cx - x < sizeSpacing.cx) {
			if (!NewLine(hdc)) {
				return FALSE;
			}
		}
		if (sizePrintable.cy - y < sizeSpacing.cy) {
			if (!NewPage(hdc)) {
				return FALSE;
			}
		}

		if (data >= 0x21 && data <= 0x7e) {
			if (lpr_output_kanji && (attr & ATTR_KANJI)) {
				// kanji chars (JIS)
				d2[0] = data;
				i++;
				buffChar = prtBuff.GetAt(i);
				d2[1] = buffChar.code;
				if (!draw_kanji(hdc, d2)) {
					return FALSE;
				}
			}
			else {
				// printable ascii chars
				if (!draw_printable_char(hdc, data)) {
					return FALSE;
				}
			}
		}
		else if (data >= 0xa1 && data <= 0xdf) {
			// 8bit-katakana chars
			if (!draw_printable_char(hdc, data)) {
				return FALSE;
			}
		}
		else if ((data >= 0x80 && data <= 0xa0) || (data >= 0xe0)) {
			// extended graphic chars
			if (!draw_graph_char(hdc, data)) {
				return FALSE;
			}
		}
		else if (data == 0x20) { // SPACE
			if (!Space(hdc)) {
				return FALSE;
			}
		}
		else {
		}

		i++;
		*index = i;

		return TRUE;
	}

	/* 一行印字 */
	BOOL PrintLine(HDC hdc)
	{
		int index;

		index = 0;
		while (index < prtBuff.getNumCurrentChars()) {
			if (!PrintChar(hdc, &index)) {
				return FALSE;
			}
		}

		prtBuff.ClearAll();

		return TRUE;
	}

	BOOL AddCharToPrtBuff(HDC hdc, const BYTE code, const int attr)
	{
		/* バッファフル印字 */
		if (prtBuff.IsBufferFull()) {
			if (!PrintLine(hdc)) {
				return FALSE;
			}
			if (!NewLine(hdc)) {
				return FALSE;
			}
		}

		prtBuff.AddChar(code, attr);

		return TRUE;
	}

	void set_line_spacing(const int inch_per_base, const int base)
	{
		sizeSpacing.cy = INCHES2TWIPS(inch_per_base, base);
	//	fprintf(stderr,"sizeSpacing.cy=%d\n",sizeSpacing.cy);
	}

	BOOL draw_bit_image(HDC hdc, int nBitImgLen, int dpi)
	{
		BYTE data;
		DWORD dwRead;
		int left, top;
		int i, j;
		SIZE sz_sp;

		sz_sp.cx = INCHES2TWIPS(1, dpi)<<3;
		sz_sp.cy = INCHES2TWIPS(1, 72)<<3;

		// one charactor consits of 8x8 "bits".
		for (j=0; j < nBitImgLen; j++) {
			if (!ReadFile(hFile, &data, 1, &dwRead, NULL)) {
				PRINT_ERROR("ReadFile()");
				return FALSE;
			}
			for (i=0; i < 8; i++) {
				if (data & (1<<(7-i))) {
					left = x + ((j*sz_sp.cx)>>3);
					top  = y + ((i*sz_sp.cy)>>3);
					if (!draw_one_dot(hdc, left, top)) {
						return FALSE;
					}
				}
			}
		}

		x += (sz_sp.cx * nBitImgLen)>>3;

		return TRUE;
	}

private:
	/* プリンタコマンド */ // PRINTER COMMANDS
	BOOL CarriageReturn(HDC hdc)
	{
		if (!PrintLine(hdc)) {
			return FALSE;
		}

		x = 0;

		return TRUE;
	}

	BOOL LineFeed(HDC hdc)
	{
		if (!PrintLine(hdc)) {
			return FALSE;
		}

		return NewLine(hdc);
	}

	BOOL FormFeed(HDC hdc)
	{
		if (!PrintLine(hdc)) {
			return FALSE;
		}

		return NewPage(hdc);
	}

	BOOL VertTab(HDC hdc)
	{
		/* MB27403及びMX80IIIではVTはLFと同じ */
		return LineFeed(hdc);
	}

	BOOL HorzTab(HDC hdc)
	{
		int col, tab;

		if (!PrintLine(hdc)) {
			return FALSE;
		}

		col = x/sizeSpacing.cx;
		tab = TABSTOP - (col % TABSTOP);
		x += sizeSpacing.cx * tab;

		return TRUE;
	}

	BOOL Delete(HDC hdc)
	{
		if (!PrintLine(hdc)) {
			return FALSE;
		}

		x -= sizeSpacing.cx;
		if (x < 0) x = 0;
		if (!erase_one_char(hdc)) {
			return FALSE;
		}

		return TRUE;
	}

	BOOL BackSpace(HDC hdc)
	{
		/* MB27403ではBSはDELと同じ */
		return Delete(hdc);
	}

private:
	/* ESCAPEシーケンス */ // ESCAPE SEQUENCES
	BOOL Esc_AtMark(HDC hdc)
	{
		(void)hdc;

		if (!reset_params(hdc)) {
			return FALSE;
		}

		return TRUE;
	}

	BOOL Esc_0(HDC hdc)
	{
		(void)hdc;

		set_line_spacing(1, 8);

		return TRUE;
	}

	BOOL Esc_1(HDC hdc)
	{
		(void)hdc;

		set_line_spacing(7, lp_unit);

		return TRUE;
	}

	BOOL Esc_2(HDC hdc)
	{
		(void)hdc;

		set_line_spacing(1, 6);

		return TRUE;
	}

	BOOL Esc_3(HDC hdc)
	{
		BYTE data;
		DWORD dwRead;
		int n;	// n/lp_unit-inch Line Spacing

		(void)hdc;

		if (!ReadFile(hFile, &data, 1, &dwRead, NULL)) {
			PRINT_ERROR("ReadFile()");
			return FALSE;
		}
		n = (int)data;
		set_line_spacing(n, lp_unit*3);

		return TRUE;
	}

	BOOL Esc_A(HDC hdc)
	{
		BYTE data;
		DWORD dwRead;
		int n;	// n/lp_unit-inch Line Spacing

		(void)hdc;

		if (!ReadFile(hFile, &data, 1, &dwRead, NULL)) {
			PRINT_ERROR("ReadFile()");
			return FALSE;
		}
		n = (int)data;
		set_line_spacing(n, lp_unit);

		return TRUE;
	}

	BOOL Esc_K(HDC hdc)
	{
		BYTE data2[2];
		DWORD dwRead;
		int nBitImgLen;	 // bit image length

		if (!PrintLine(hdc)) {
			return FALSE;
		}

		if (!ReadFile(hFile, data2, 2, &dwRead, NULL)) {
			PRINT_ERROR("ReadFile()");
			return FALSE;
		}
		nBitImgLen = ((int)data2[1]<<8) | (int)data2[0];

		if (!draw_bit_image(hdc, nBitImgLen, 60)) {
			return FALSE;
		}

		return TRUE;
	}

	BOOL Esc_L(HDC hdc)
	{
		BYTE data2[2];
		DWORD dwRead;
		int nBitImgLen;	 // bit image length

		if (!PrintLine(hdc)) {
			return FALSE;
		}

		if (!ReadFile(hFile, data2, 2, &dwRead, NULL)) {
			PRINT_ERROR("ReadFile()");
			return FALSE;
		}
		nBitImgLen = ((int)data2[1]<<8) | (int)data2[0];

		if (!draw_bit_image(hdc, nBitImgLen, 120)) {
			return FALSE;
		}

		return TRUE;
	}

private:
	/* FS(漢字)シーケンス */ // FS(KANJI) SEQUENCES
	BOOL Kanji_ON(HDC hdc)
	{
		if (!Kanji_Is_ON()) {
			if (!fonts->select_kanji(hdc, FONT_KANJI_NORMAL)) {
				return FALSE;
			}
		}

		fKanji++;

		return TRUE;
	}

	BOOL Kanji_OFF(HDC hdc)
	{
		if (Kanji_Is_ON()) {
			fKanji--;
		}

		if (!Kanji_Is_ON()) {
			if (!fonts->select_ank(hdc, FONT_ANK_PREV)) {
				return FALSE;
			}
		}

		return TRUE;
	}

private:
	/* 描画メインルーチン */ // Renderer
	BOOL render(HDC hdc)
	{
		DWORD dwRead;
		BYTE data;
		int attr;

		while (TRUE) {
			if (!ReadFile(hFile, &data, 1, &dwRead, NULL)) {
				PRINT_ERROR("ReadFile()");
				return FALSE;
			}

			// check EOF
			if (dwRead == 0) {
				break;
			}

			// put one charactor to print-buffer
			if (data >= 0x20 && data <= 0x7e  || (data >= 0x80)) {
				attr = attribute();
				if (!AddCharToPrtBuff(hdc, data, attr)) {
					return FALSE;
				}
			}
			// PRINTER COMMANDS
			else if (data == 0x0d) { // CR
				if (!CarriageReturn(hdc)) {
					return FALSE;
				}
			}
			else if (data == 0x0a) { // LF
				if (!LineFeed(hdc)) {
					return FALSE;
				}
			}
			else if (data == 0x0c) { // FF
				if (!FormFeed(hdc)) {
					return FALSE;
				}
			}
			else if (data == 0x0b) { // VT
				if (!VertTab(hdc)) {
					return FALSE;
				}
			}
			else if (data == 0x09) { // HT
				if (!HorzTab(hdc)) {
					return FALSE;
				}
			}
			else if (data == 0x08) { // BS
				if (!BackSpace(hdc)) {
					return FALSE;
				}
			}
			else if (data == 0x7f) { // DEL
				if (!Delete(hdc)) {
					return FALSE;
				}
			}
			// ESCAPE SEQUENCES
			else if (data == 0x1b) { // ESC
				if (!ReadFile(hFile, &data, 1, &dwRead, NULL)) {
					PRINT_ERROR("ReadFile()");
					return FALSE;
				}
				if (data == '@') {
					if (!Esc_AtMark(hdc)) {
						return FALSE;
					}
				}
				else if (data == '0') {
					if (!Esc_0(hdc)) {
						return FALSE;
					}
				}
				else if (data == '1') {
					if (!Esc_1(hdc)) {
						return FALSE;
					}
				}
				else if (data == '2') {
					if (!Esc_2(hdc)) {
						return FALSE;
					}
				}
				else if (data == '3') {
					if (!Esc_3(hdc)) {
						return FALSE;
					}
				}
				else if (data == 'A') {
					if (!Esc_A(hdc)) {
						return FALSE;
					}
				}
				else if (data == 'K') {
					if (!Esc_K(hdc)) {
						return FALSE;
					}
				}
				else if (data == 'L') {
					if (!Esc_L(hdc)) {
						return FALSE;
					}
				}
				else {
				}
			}
			else if (data == 0x1c) { // FS (Kanji)
				if (!ReadFile(hFile, &data, 1, &dwRead, NULL)) {
					PRINT_ERROR("ReadFile()");
					return FALSE;
				}
				// FS(Kanji) SEQUENCES
				if (data == 0x26) {
					if (!Kanji_ON(hdc)) {
						return FALSE;
					}
				}
				else if (data == 0x2e) {
					if (!Kanji_OFF(hdc)) {
						return FALSE;
					}
				}
				else {
				}
			}
			else {	// default
			}
		}

		return TRUE;
	}

public:
	/* 印刷 */ // printing
	BOOL print(PRINTDLG& pd)
	{
		DOCINFO docinfo;

		if (SetMapMode(pd.hDC, MM_TWIPS) == 0) {
			return FALSE;
		}

		fonts = new LprFonts();

		if (!fonts->create(pd.hDC, sizeChar)) {
			return FALSE;
		}

		if (!set_printable_area(pd.hDC)) {
			fonts->delete_f(pd.hDC);
			delete fonts;
			return FALSE;
		}

		if (!reset_params(pd.hDC)) {
			fonts->delete_f(pd.hDC);
			delete fonts;
			return FALSE;
		}

		docinfo.cbSize = sizeof(DOCINFO);
		docinfo.lpszDocName = szFullPathName;
		docinfo.lpszOutput = NULL; // to Printer
		docinfo.lpszDatatype = NULL;
		docinfo.fwType = 0;

		if (StartDoc(pd.hDC, &docinfo) == 0) {
			PRINT_ERROR("StartDoc()");
			fonts->delete_f(pd.hDC);
			delete fonts;
			return FALSE;
		}

		if (!StartPage(pd.hDC)) {
			PRINT_ERROR("StartPage()");
			fonts->delete_f(pd.hDC);
			delete fonts;
			return FALSE;
		}

		if (!render(pd.hDC)) {
			fonts->delete_f(pd.hDC);
			delete fonts;
			return FALSE;
		}

		if (!EndPage(pd.hDC)) {
			PRINT_ERROR("EndPage()");
			fonts->delete_f(pd.hDC);
			delete fonts;
			return FALSE;
		}

		if (!EndDoc(pd.hDC)) {
			PRINT_ERROR("EndDoc()");
			fonts->delete_f(pd.hDC);
			delete fonts;
			return FALSE;
		}

		fonts->delete_f(pd.hDC);
		delete fonts;

		return TRUE;
	}

};

/* プリンタ出力 */ // printer output

class Lpr {

public:
	Lpr()
	{
	}

	~Lpr()
	{
	}

private:
	TCHAR szFullPathName[_MAX_PATH];	/* 印刷用一時ファイルのフルパス名 */ // full path name of temporary file for printing
	HANDLE hFile;						/* 印刷用一時ファイルのハンドル */ // handle of temporary file for printing

private:
	/* 印刷用一時ファイルrewind */
	// rewind temporary file for printing
	BOOL rewind_file(void)
	{
		if (SetFilePointer(hFile, 0, NULL, FILE_BEGIN)
		    == INVALID_SET_FILE_POINTER ) {
			PRINT_ERROR("SetFilePointer()");
			return FALSE;
		}

		return TRUE;
	}

	/* 印刷用一時ファイルtruncate */
	// truncate temporary file for printing
	BOOL trunc_file(void)
	{
		if (SetFilePointer(hFile, 0, NULL, FILE_BEGIN)
		    == INVALID_SET_FILE_POINTER ) {
			PRINT_ERROR("SetFilePointer()");
			return FALSE;
		}

		if (!SetEndOfFile(hFile)) {
			PRINT_ERROR("SetEndOfFile()");
			return FALSE;
		}

		return TRUE;
	}

	/* 印刷用の文字フォントイメージをロードする */
	// load font image for printing
	BOOL load_font_image(void)
	{
		int i;

		/* file_loadを使用するように変更 */
#if XM7_VER == 1
		if (!file_load(SUBSYSC_ROM, (BYTE *)CharFontImg, 2048)) {
			if (!file_load(SUBSYS8_ROM, (BYTE *)CharFontImg, 2048)) {
				PRINT_ERROR("init.file_load()");
				return FALSE;
			}
		}
#else
		if (!file_load(SUBSYSC_ROM, (BYTE *)CharFontImg, 2048)) {
			PRINT_ERROR("init.file_load()");
			return FALSE;
		}
#endif

	//	{	// DEBUG
	//		int i, j;
	//		for (i=0; i < N_CHAR_FONTS; i++) {
	//			for (j=0; j < 8; j++) {
	//				fprintf(stderr, "%02x ", CharFontImg[i][j]);
	//			}
	//			fprintf(stderr, "\n");
	//		}
	//	}

		return TRUE;
	}

	/* 初期化 */
	// initialization
	BOOL init(void)
	{
		// init PUBLIC variables
		lpr_use_os_font = FALSE;	// DEFAULT
		lpr_output_kanji = FALSE;	// DEFAULT

		// init PRIVATE variables

		if (!load_font_image()) {
			return FALSE;
		}

		return TRUE;
	}

public:
	/* 印刷 */ // printing
	BOOL print(void)
	{
		PRINTDLG pd;

		LockVM();
		StopSnd();

		memset(&pd,0,sizeof(PRINTDLG));	// unused area must be init'd to NULL
		pd.lStructSize = sizeof(PRINTDLG);
		pd.hwndOwner = hMainWnd;
		pd.hDevMode = NULL;	// must be init'd to NULL
		pd.hDevNames = NULL;	// must be init'd to NULL
		pd.Flags = PD_RETURNDC;
		pd.Flags |= PD_ALLPAGES;
		pd.Flags |= PD_NOPAGENUMS;
		pd.Flags |= PD_NOSELECTION;
		if (!PrintDlg(&pd)) {
			PlaySnd();
			ResetSch();
			UnlockVM();
			return FALSE;
		}

		if (!rewind_file()) {
			DeleteDC(pd.hDC);
			PlaySnd();
			ResetSch();
			UnlockVM();
			return FALSE;
		}

		LprRenderer renderer(szFullPathName, hFile);

		if (!renderer.print(pd)) {
			DeleteDC(pd.hDC);
			PlaySnd();
			ResetSch();
			UnlockVM();
			return FALSE;
		}

		DeleteDC(pd.hDC);

		if (!trunc_file()) {
			PlaySnd();
			ResetSch();
			UnlockVM();
			return FALSE;
		}

		PlaySnd();
		ResetSch();
		UnlockVM();
		return TRUE;
	}

	/* 印刷用一時ファイルオープン */ // open temporary file for printing
	int open(const char *fname)
	{
		TCHAR szDirName[_MAX_PATH];

	#if defined(DEBUG_LPRINT)
		if (!GetCurrentDirectory(_MAX_PATH, szDirName)) {
			PRINT_ERROR("GetCurrentDirectory()");
			return (-1);
		}
	#else
		if (!GetTempPath(_MAX_PATH, szDirName)) {
			PRINT_ERROR("GetTempPath()");
			return (-1);
		}
	#endif

		if (!GetTempFileName(szDirName, (LPCTSTR)fname, 0, szFullPathName)) {
			PRINT_ERROR("GetTempFileName()");
			return (-1);
		}

		if ((hFile=CreateFile
		    (szFullPathName,
		    GENERIC_READ|GENERIC_WRITE,
	#if defined(DEBUG_LPRINT)
		    FILE_SHARE_READ,
	#else
		    0,
	#endif
		    NULL,
		    CREATE_ALWAYS,
		    FILE_ATTRIBUTE_NORMAL,
		    NULL)
		    )==INVALID_HANDLE_VALUE) {
			PRINT_ERROR("CreateFile()");
			return (-1);
		}

		if (!init()) {
			return (-1);
		}

		return (int)hFile;
	}

	/* 印刷用一時ファイルクローズ */ // close temporary file for printing
	void close(void)
	{
		CloseHandle(hFile);
	}

	/* 印刷用一時ファイル書き込み */ // write to temporary file for printing
	BOOL write(BYTE *ptr, DWORD size)
	{
		DWORD dwWritten;

		if (!WriteFile(hFile, ptr, size, &dwWritten, NULL)) {
			PRINT_ERROR("WriteFile()");
			return FALSE;
		}

		return TRUE;
	}

	/* 印刷用一時ファイル削除 */ // remove temporary file for printing
	void remove(void)
	{
		if (!DeleteFile(szFullPathName)) {
			PRINT_ERROR("DeleteFile()");
		}
	}

};

/*
 *	スタティック ワーク
 */
// STATIC variables
static Lpr *lpr = NULL;


#ifdef __cplusplus
extern "C" {
#endif

/*
 *	グローバル関数
 */
// global functions

/*
 *	印刷
 */
// printing
BOOL FASTCALL lp_printfile(void)
{
	return lpr->print();
}

/*
 *	印刷用一時ファイルオープン
 */
// open temporary file for printing
int FASTCALL lp_openfile(char *fname)
{
	if (lpr == NULL) {
		lpr = new Lpr();
		return lpr->open(fname);
	}
	else {
		return -1;
	}
}

/*
 *	印刷用一時ファイルクローズ
 */
// close temporary file for printing
void FASTCALL lp_closefile(void)
{
	if (lpr != NULL) {
		lpr->close();
	}
}

/*
 *	印刷用一時ファイル書き込み
 */
// write to temporary file for printing
BOOL FASTCALL lp_writefile(BYTE *ptr, DWORD size)
{
	if (lpr != NULL) {
		return lpr->write(ptr, size);
	}
	else {
		return FALSE;
	}
}

/*
 *	印刷用一時ファイル削除
 */
// remove temporary file for printing
void FASTCALL lp_removefile(void)
{
	if (lpr != NULL) {
		lpr->remove();
		delete lpr;
		lpr = NULL;
	}
}

#ifdef __cplusplus
}
#endif

#endif	/* LPRINT */
#endif	/* _WIN32 */
