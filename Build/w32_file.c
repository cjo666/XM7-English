/*
 *	FM-7 EMULATOR "XM7"
 *
 *	Copyright (C) 1999-2017 ＰＩ．(Twitter:@xm6_original)
 *	Copyright (C) 2001-2017 Ryu Takegami (Twitter:@RyuTakegami)
 *
 *	[ Win32API ファイルI/O ]
 *
 *	RHG履歴
 *	  2001.12.25		CD-ROMでR/Wオープンが成功する問題を修正(by ＰＩ．)
 *	  2002.09.20		既に存在するファイルを書き込みモードでオープンした時に
 *						ファイルサイズが初期化されない問題を修正
 */

#ifdef _WIN32

#define STRICT
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <assert.h>
#include <io.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <stdlib.h>
#include "xm7.h"
#include "device.h"

/*
 *	ファイルロード(ROM専用)
 */
BOOL FASTCALL file_load(char *fname, BYTE *buf, int size)
{
	char path[_MAX_PATH];
	char drvname[_MAX_DRIVE];
	char dirname[_MAX_DIR];
	char filename[_MAX_FNAME];
	char extname[_MAX_EXT];

	int handle;

	/* assert */
	ASSERT(fname);
	ASSERT(buf);
	ASSERT(size > 0);

	/* ファイル名と拡張子を取り出す */
	_splitpath(fname, NULL, NULL, filename, extname);

	/* XM7のパスを得て */
	memset(path, 0, sizeof(path));
	GetModuleFileName(NULL, path, sizeof(path));
	_splitpath(path, drvname, dirname, NULL, NULL);

	/* パス合成 */
	_makepath(path, drvname, dirname, filename, extname);

	handle = open(path, O_BINARY | O_RDONLY);
	if (handle == -1) {
		return FALSE;
	}

	if (read(handle, buf, size) != size) {
		close(handle);
		return FALSE;
	}

	close(handle);
	return TRUE;
}

/*
 *	ファイルセーブ(学習RAM専用)
 */
BOOL FASTCALL file_save(char *fname, BYTE *buf, int size)
{
	char path[_MAX_PATH];
	char drvname[_MAX_DRIVE];
	char dirname[_MAX_DIR];
	char filename[_MAX_FNAME];
	char extname[_MAX_EXT];

	int handle;

	/* assert */
	ASSERT(fname);
	ASSERT(buf);
	ASSERT(size > 0);

	/* ファイル名と拡張子を取り出す */
	_splitpath(fname, NULL, NULL, filename, extname);

	/* XM7のパスを得て */
	memset(path, 0, sizeof(path));
	GetModuleFileName(NULL, path, sizeof(path));
	_splitpath(path, drvname, dirname, NULL, NULL);

	/* パス合成 */
	_makepath(path, drvname, dirname, filename, extname);

	handle = open(path, O_BINARY | O_CREAT | O_WRONLY, S_IWRITE);
	if (handle == -1) {
		return FALSE;
	}

	if (write(handle, buf, size) != size) {
		close(handle);
		return FALSE;
	}

	close(handle);
	return TRUE;
}

/*
 *	ファイルオープン
 */
int FASTCALL file_open(char *fname, int mode)
{
	/* assert */
	ASSERT(fname);

	switch (mode) {
		case OPEN_R:
			return open(fname, O_BINARY | O_RDONLY);
		case OPEN_W:
			return open(fname, O_BINARY | O_CREAT | O_TRUNC | O_WRONLY,
						S_IWRITE);
		case OPEN_RW:
			/* CD-ROMからの読み込みはRWが成功してしまう */
			if (access(fname, 0x06) != 0) {
				return -1;
			}
			return open(fname, O_BINARY | O_RDWR);

	}

	ASSERT(FALSE);
	return -1;
}

/*
 *	ファイルクローズ
 */
void FASTCALL file_close(int handle)
{
	/* assert */
	ASSERT(handle >= 0);

	close(handle);
}

/*
 *	ファイルサイズ取得
 */
DWORD FASTCALL file_getsize(int handle)
{
	long now;
	long end;

	/* assert */
	ASSERT(handle >= 0);

	now = tell(handle);
	if (now == -1) {
		return 0;
	}

	end = lseek(handle, 0L, SEEK_END);
	lseek(handle, now, SEEK_SET);

	return end;
}

/*
 *	ファイルシーク
 */
BOOL FASTCALL file_seek(int handle, DWORD offset)
{
	long now;

	/* assert */
	ASSERT(handle >= 0);

	now = lseek(handle, offset, SEEK_SET);
	if (now != (long)offset) {
		return FALSE;
	}

	return TRUE;
}

/*
 *	ファイル読み出し
 */
BOOL FASTCALL file_read(int handle, BYTE *ptr, DWORD size)
{
	unsigned int cnt;

	/* assert */
	ASSERT(handle >= 0);
	ASSERT(ptr);
	ASSERT(size > 0);

	cnt = read(handle, ptr, size);
	if (cnt != size) {
		return FALSE;
	}

	return TRUE;
}

/*
 *	ファイル書き込み
 */
BOOL FASTCALL file_write(int handle, BYTE *ptr, DWORD size)
{
	unsigned int cnt;

	/* assert */
	ASSERT(handle >= 0);
	ASSERT(ptr);
	ASSERT(size > 0);

	cnt = write(handle, ptr, size);
	if (cnt != size) {
		return FALSE;
	}

	return TRUE;
}

#endif	/* _WIN32 */
