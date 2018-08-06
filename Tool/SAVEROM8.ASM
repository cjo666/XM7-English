*
*	FM-7 EMULATOR "XM7"
*
*	Copyright (C) 1999-2017 ＰＩ．(Twitter:@xm6_original)
*	Copyright (C) 2001-2017 Ryu Takegami (Twitter:@RyuTakegami)
*	                        MICRO 8 Support by Toshio Fukui
*
*	[ ROMセーバ(XM7 V1.1対応) ]
*
*	対象機種・生成メディアタイプに応じてアセンブル時にオプション指定すること
*		-dDISKTYPE=0	360KB(2D)  通常フォーマット
*		-dDISKTYPE=1	720KB(2DD) 1トラック抜きフォーマット(使用不可)
*		-dDISKTYPE=2	720KB(2DD) 通常フォーマット(使用不可)
*		-dFBLOADM		DISK BASIC用
*

	if	defined(FBLOADM)
				ORG		$0FE0
	else
				ORG		$1000
	endif

*
*	定数定義 (メインシステム)
*
STACK			EQU		$0F00			* スタック最終
BUFFER			EQU		$0F80			* 汎用バッファ
KANJI_BUF		EQU		$0FE8			* 漢字バッファ
DISK_BUF		EQU		$2800			* ディスクバッファ
SHARED			EQU		$FC80			* 共有RAM

*
*	定数定義 (メインI/O)
*
KEYDAT			EQU		$FD01			* キーデータレジスタ
SUBMODE			EQU		$FD04			* サブシステムモードレジスタ
SUBSIN			EQU		$FD05			* サブシステムインタフェースレジスタ
RS232D			EQU		$FD06			* USART データレジスタ
RS232S			EQU		$FD07			* USART ステータスレジスタ
RS232C			EQU		RS232S			* USART コマンドレジスタ
RSBAUD			EQU		$FD0B			* RS-232C ボーレートレジスタ
RSENABLE		EQU		$FD0C			* RS-232C イネーブルレジスタ
BANKSEL			EQU		$FD0F			* ROM/裏RAMセレクトレジスタ
INITDIS			EQU		$FD10			* イニシエータROM ディセーブルレジスタ
SUBSTAT			EQU		$FD12			* サブモードステータスレジスタ
SUBBANK			EQU		$FD13			* サブバンクレジスタ
FDCSTAT			EQU		$FD18			* FDC ステータスレジスタ
FDCCMD			EQU		FDCSTAT			* FDC コマンドレジスタ
FDCTRACK		EQU		$FD19			* FDC トラックレジスタ
FDCSECTOR		EQU		$FD1A			* FDC セクタレジスタ
FDCDATA			EQU		$FD1B			* FDC データレジスタ
FDDSIDE			EQU		$FD1C			* FDD サイドレジスタ
FDDDRIVE		EQU		$FD1D			* FDD ドライブレジスタ
FDDMODE			EQU		$FD1E			* FDD モードレジスタ
FDDDRQ			EQU		$FD1F			* FDC DRQ/IRQレジスタ
KNJ1ADR			EQU		$FD20			* 第一水準漢字 アドレスレジスタ
KNJ1DATA		EQU		$FD22			* 第一水準漢字 データレジスタ
KNJ2ADR			EQU		$FD2C			* 第二水準漢字 アドレスレジスタ
KNJ2DATA		EQU		$FD2E			* 第二水準漢字 データレジスタ
DICBANK			EQU		$FD2E			* 辞書ROM バンクレジスタ
MMR				EQU		$FD80			* MMR メモリマッピングレジスタ
MSGR			EQU		$FD90			* MMR セグメントレジスタ
MMSR			EQU		$FD93			* MMR モードセレクトレジスタ
MSSW1			EQU		$FD94			* モードセレクトスイッチ1 (40/20)
MSSW2			EQU		$FD95			* モードセレクトスイッチ2 (EX)

*
*	定数定義 (サブI/O)
*
SUBMODEL4		EQU		$D405			* サブモードレジスタ (L4)
CRTDSP			EQU		$D408			* CRT ON/OFFレジスタ
VACC			EQU		$D409			* VRAMアクセスフラグ
ALUCMD			EQU		$D410			* 論理演算コマンドレジスタ
ALUCOL			EQU		$D411			* 論理演算カラーレジスタ
ALUMASK			EQU		$D412			* 論理演算マスクレジスタ
ALUDIS			EQU		$D41B			* 論理演算バンクディセーブルレジスタ
KENCDAT			EQU		$D431			* キーエンコーダデータレジスタ
KENCSTA			EQU		$D432			* キーエンコーダステータスレジスタ

*
*	ワークエリア定義
*
RS_FLAG			EQU		$00			* シリアル出力フラグ
DRIVE			EQU		$01			* アクセスドライブ
TRACK			EQU		$02			* アクセストラック
DRIVE0_T		EQU		$03			* ドライブ0のトラック
DRIVE1_T		EQU		$04			* ドライブ1のトラック
SIDE			EQU		$05			* アクセスサイド
SECTOR			EQU		$06			* アクセスセクタ
SECTORS			EQU		$07			* 連続セクタ数
CLUSTER			EQU		$08			* ファイル先頭クラスタ
CLUSTERS		EQU		$0A			* トータルクラスタ数(1クラスタ1KB)
LASTBYTE		EQU		$0C			* 最終クラスタのバイト数
COLOR			EQU		$0E			* 漢字カラーコード
TYPE			EQU		$0F			* FDCコマンドタイプ
C_TRACK			EQU		$10			* カレントトラック
C_SIDE			EQU		$11			* カレントサイド
C_SECTOR		EQU		$12			* カレントセクタ
C_SECTORS		EQU		$13			* カレントセクタ数
FM77_FLAG		EQU		RS_FLAG		* FM-77フラグ
L4_FLAG			EQU		COLOR		* 400ラインフラグ (L4)

*
*	アセンブル条件定義
*
M_FM7			EQU		0			* FM-7シリーズ全機種
M_FM77AV		EQU		1			* FM77AVシリーズ全機種
M_FM77AV40		EQU		2			* FM77AV40/EX/SX
M_FM77AVDIC		EQU		3			* FM77AV 辞書ROM
M_KANJI			EQU		4			* FM77AV40EX/SX(漢字ROM)
M_FM8			EQU		8			* FM-8
D_NORM2D		EQU		0			* 2D (360KB) 通常フォーマット
D_SKIP2DD		EQU		1			* 2DD(720KB) １トラック抜きフォーマット
D_NORM2DD		EQU		2			* 2DD(720KB) 通常フォーマット

*
*	実機CMT I/F経由強制動作用 転送プログラム
*
	if	defined(FBLOADM)
ENTRY			ORCC	#$50
				LEAX	<START,PCR
				LDU		#$1000
				LDY		#((PRGEND-START)|1)+1
TFRLOOP			LDD		,X++
				STD		,U++
				LEAY	-2,Y
				BNE		TFRLOOP
				JMP		$1000

				ORG		$1000
	endif

*
*	プログラムスタート
*
START			BRA		MAIN

*
*	プログラムヘッダ
*
HEADER			FCC		/TINY-DOS(mini) for FM-7  /
				FCC		/Copyright 1991-1992,1999 by ＰＩ．/
				FCB		$0D,$0A,$00

HEADER2			EQU		*
				FCC		/SAVEROM8/
				FCC		/ V1.37  /
				FCC		"Copyright 1999 ＰＩ．/ 2003-2017 Ryu Takegami"
				FCB		$0D,$0A,$00

*
*	メインプログラム
*
				ORG		START+$80
MAIN			ORCC	#$50
	if	defined(FBLOADM)
				LDS		#STACK
	else
				LEAS	STACK,PCR
	endif

* ハードウェア初期化
	if	defined(RSDRIVER)
				LBSR	RS_INIT
	endif
				LBSR	INIT
				LBSR	FM7_INIT

* 起動タイトル表示
	if	(DISKTYPE==D_NORM2D)||(defined(FBLOADM))
				LEAX	TITLE_MSG,PCR
				LBSR	PUT_STRING
				LEAX	TITLE_MSG2,PCR
				LBSR	PUT_STRING
				LEAX	SPACE_MSG,PCR
				LBSR	PUT_STRING
				LBSR	KEY_SPACE
	endif

* ディスクフォーマット
				LBSR	DISK_RST
				LBSR	MS_FORMAT

* メイン側セーブ処理
MAINSAVE		EQU		*
				LBSR	FBASIC
				LBSR	BOOT_BAS
				LBSR	BOOT_DOS
				LBSR	BOOT_BBL
				LBSR	BOOT_DBG
				LBSR	KANJI

* サブ側セーブ処理
SUBSAVE			EQU		*
				LBSR	SUBSYS_C

* 完了(メッセージ表示・ループ)
				LBSR	CLS
				LEAX	COMPLETE_MSG,PCR
				LBSR	PUT_STRING
				BRA		*


*
* 初期化
*
INIT			LEAX	START,PCR
				LEAX	-$100,X
				TFR		X,D
				TFR		A,DP
* ワーククリア
				CLRA
INIT1			CLR	,X+
				DECA
				BNE	INIT1
* 画面初期化
				LEAX	SUBINIT_CMD,PCR
				LDB		#$09
				LBSR	SUB_CMD
				LEAX	SUBCONS_CMD,PCR
				LDB		#$02
				LBSR	SUB_CMD
INIT2			RTS

*--[ ROMセーバ(全機種共通部) ]------------------------------------------------

*
*	F-BASIC ROM
*
FBASIC			LBSR	CLS
				LDX		$FE03
				CMPX	#$FE67		* FM-8 BASIC
				BEQ		FBASIC2
				LEAX	BOOTB_DIPSW_MSG,PCR
				LBSR	PRINT
FBASIC1			LDX		$FE03
				CMPX	#$FE67		* FM-8 BASIC
				BNE		FBASIC1
FBASIC2			LBSR	CLS
				LEAX	FBASIC_MSG,PCR
				LBSR	PUT_STRING
				LEAX	FBASIC_FN,PCR
				LDD		#31
				STD		<CLUSTERS
				LDD		#1024
				STD		<LASTBYTE
				LBSR	FILE_CREATE
				LDX		#$8000
				LDU		#31
				LBSR	FILE_WRITE
				RTS

*
*	KANJI ROM (第一水準)
*
KANJI			CLRA
				CLRB
				STD		KNJ1ADR
				LDD		KNJ1DATA
				CMPD	#$FFFF
				BEQ		KANJI5

				LBSR	CLS
				LEAX	KANJI_MSG,PCR
				LBSR	PUT_STRING
				LEAX	KANJI_FN,PCR
				LDD		#$80
				STD		<CLUSTERS
				LDD		#1024
				STD		<LASTBYTE
				LBSR	FILE_CREATE
* 8回ループ
				LDX		#$0000
KANJI1			LDU		#$4000
KANJI2			STX		KNJ1ADR
				LDD		KNJ1DATA
				STD		,U++
				LEAX	1,X
				CMPU	#$8000
				BNE		KANJI2
* セーブ
				PSHS	X
				LDX		#$4000
				LDU		#16
				LBSR	FILE_WRITE
				PULS	X
* NEXT
				CMPX	#$0000
				BNE		KANJI1
KANJI5			RTS

*--[ ROMセーバ(FM-8) ]--------------------------------------------------------

*
*	BOOT(BASIC) ROM
*
BOOT_BAS		LBSR	CLS
				LDX		$FE03
				CMPX	#$FE67		* FM-8 BASIC
				BEQ		BOOT_BAS2
				LEAX	BOOTB_DIPSW_MSG,PCR
				LBSR	PRINT
BOOT_BAS1		LDX		$FE03
				CMPX	#$FE67		* FM-8 BASIC
				BNE		BOOT_BAS1
				LBSR	CLS
BOOT_BAS2		LEAX	BOOTBAS_MSG,PCR
				LBSR	PRINT
				LEAX	BOOTBAS_FN,PCR
				LDD		#1
				STD		<CLUSTERS
				LDD		#512
				STD		<LASTBYTE
				LBSR	FILE_CREATE
				LDU		#1
				LBSR	FILE_WRITE
				RTS

*
*	BOOT(DOS) ROM
*
BOOT_DOS		LBSR	CLS
				LDX		$FE03
				CMPX	#$FE6B		* FM-8 DOS
				BEQ		BOOT_DOS2
				LEAX	BOOTD_DIPSW_MSG,PCR
				LBSR	PRINT
BOOT_DOS1		LDX		$FE03
				CMPX	#$FE6B		* FM-8 DOS
				BNE		BOOT_DOS1
BOOT_DOS2		LBSR	CLS
				LEAX	BOOTDOS_MSG,PCR
				LBSR	PRINT
				LEAX	BOOTDOS_FN,PCR
				LDD		#1
				STD		<CLUSTERS
				LDD		#512
				STD		<LASTBYTE
				LBSR	FILE_CREATE
				LDU		#1
				LBSR	FILE_WRITE
				RTS

*
*	BOOT(BBL) ROM
*
BOOT_BBL		LBSR	CLS
				LDX		$FE03
				CMPX	#$FE5B		* FM-8 BBL
				BEQ		BOOT_BBL2
				LEAX	BOOTL_DIPSW_MSG,PCR
				LBSR	PRINT
BOOT_BBL1		LDX		$FE03
				CMPX	#$FE5B		* FM-8 BBL
				BNE		BOOT_BBL1
BOOT_BBL2		LBSR	CLS
				LEAX	BOOTBBL_MSG,PCR
				LBSR	PRINT
				LEAX	BOOTBBL_FN,PCR
				LDD		#1
				STD		<CLUSTERS
				LDD		#512
				STD		<LASTBYTE
				LBSR	FILE_CREATE
				LDX		#$FE00
				LDU		#1
				LBSR	FILE_WRITE
				RTS

*
*	BOOT(DBG) ROM
*
BOOT_DBG		LBSR	CLS
				LDX		$FE03
				CMPX	#$FE84		* FM-8 DBG
				BEQ		BOOT_DBG2
				LEAX	BOOTG_DIPSW_MSG,PCR
				LBSR	PRINT
BOOT_DBG1		LBSR	KEY_CHKSP
				BNE		BOOT_DBG2
				LDX		$FE03
				CMPX	#$FE84		* FM-8 DBG
				BNE		BOOT_DBG1
BOOT_DBG2		LBSR	CLS
				LEAX	BOOTDBG_MSG,PCR
				LBSR	PRINT
				LEAX	BOOTDBG_FN,PCR
				LDD		#1
				STD		<CLUSTERS
				LDD		#512
				STD		<LASTBYTE
				LBSR	FILE_CREATE
				CLR		INITDIS
				LDX		#$FE00
				LDU		#1
				LBSR	FILE_WRITE
				RTS

*
*	SUBSYSTEM (TYPE-C) ROM
*
SUBSYS_C		LBSR	CLS
				LEAX	SUBSYSC_MSG,PCR
				LBSR	PRINT
				LEAX	SUBSYSC_FN,PCR
				LDD		#10
				STD		<CLUSTERS
				LDD		#1024
				STD		<LASTBYTE
				LBSR	FILE_CREATE
* サブモニタ転送
				LDX		#$D800
				LDY		#$4800
SUBSYS_C1		PSHS	X,Y
				STX		SRCADR,PCR
				LEAX	TFR_CMD,PCR
				LDB		#TFR_CMDe-TFR_CMD
				LBSR	SUB_CMD
				PULS	X,Y
				LBSR	SUB_HALT
				LDU		#$FCC0
				LDB		#$40
SUBSYS_C2		LDA		,U+
				STA		,Y+
				DECB
				BNE		SUBSYS_C2
				LDA		#$80
				STA		SHARED
				CLR		SUBSIN
				LEAX	$40,X
				BNE		SUBSYS_C1
* セーブ
				LDX		#$4800
				LDU		#10
				LBSR	FILE_WRITE
				RTS

*
* サブモニタ転送用メンテナンスコマンド
*
TFR_CMD			FCB		$3F,$59,$41,$4D,$41,$55,$43,$48,$49
				FCB		$91
SRCADR			FDB		$0000,$D3C0,$0040
				FCB		$90
TFR_CMDe		EQU		*

*--[ FM-7 ]-------------------------------------------------------------------

*
*	FM-7初期化
*
FM7_INIT		EQU		*
				CLR		<FM77_FLAG
				RTS

*
*	スペース待ち
*
KEY_SPACE		EQU		*
KEY_SPACE1		LEAX	INKEY_CMD,PCR
				LDB		#2
				LBSR	SUB_CMD
				LBSR	SUB_HALT
				LDB		#$80
				STB		SHARED
				LDD		SHARED+3
				CLR		SUBSIN
				TSTB
				BEQ		KEY_SPACE1
				CMPA	#$20
				BNE		KEY_SPACE1
				RTS

*
*	スペース入力チェック
*
KEY_CHKSP		LEAX	INKEY_CHK_CMD,PCR
				LDB		#2
				LBSR	SUB_CMD
				LBSR	SUB_HALT
				LDB		#$80
				STB		SHARED
				LDD		SHARED+3
				CLR		SUBSIN
				TSTB
				BEQ		KEY_CHKSP1
				CMPA	#$20
				BNE		KEY_CHKSP1
				FCB		$86
KEY_CHKSP1		CLRA
				RTS

*
*	画面クリア
*
CLS				EQU		*
				LEAX	CLS_CMD,PCR
				LDB		#CLS_CMDe-CLS_CMD
				LBRA	SUB_CMD

*
*	サブCPU 画面クリアプログラム
*
CLS_CMD			FCB		$3F,$59,$41,$4D,$41,$55,$43,$48,$49
				FCB		$93,$D3,$8F,$90
*
CLSENT			LDU		#$0000+80*200
				BSR		CLS1
				LDU		#$4000+80*200
				BSR		CLS1
				LDU		#$8000+80*200
CLS1			TST		VACC
				LDB		#100
				PSHS	B
				LDX		#0
				TFR		X,D
				TFR		X,Y
CLS2			PSHU	D,X,Y
				PSHU	D,X,Y
				PSHU	D,X,Y
				PSHU	D,X,Y
				PSHU	D,X,Y
				PSHU	D,X,Y
				PSHU	D,X,Y
				PSHU	D,X,Y
				PSHU	D,X,Y
				PSHU	D,X,Y
				PSHU	D,X,Y
				PSHU	D,X,Y
				PSHU	D,X,Y
				PSHU	D,X,Y
				PSHU	D,X,Y
				PSHU	D,X,Y
				PSHU	D,X,Y
				PSHU	D,X,Y
				PSHU	D,X,Y
				PSHU	D,X,Y
				PSHU	D,X,Y
				PSHU	D
				DEC		,S
				BNE		CLS2
				CLR		VACC
				PULS	B,PC
CLS_CMDe		EQU		*

*--[ サブCPU ]----------------------------------------------------------------

*
*	サブCPU HALT
*
*	USEREG: A
*
SUB_HALT		LDA		SUBSIN
				BMI		SUB_HALT
				LDA		#$80
				STA		SUBSIN
SUB_HALT1		LDA		SUBSIN
				BPL		SUB_HALT1
				RTS

*
*	サブCPU コマンド出力
*
*	PARAM.: X	コマンドデータ($FC82～)
*			B	送信長さ
*	USEREG: A,B,X,U
*
SUB_CMD			LBSR	SUB_HALT
* データ書き込み
				LDU		#SHARED
				CLR		,U+
				CLR		,U+
SUB_CMD1		LDA		,X+
				STA		,U+
				DECB
				BNE		SUB_CMD1
* サブCPU 実行
				CLR		SUBSIN
				RTS

*
*	文字列出力 (FM-7用,ANK専用)
*
*	PARAM.: X	文字列
*	USEREG: A,B,X,U
*
PUT_STRING		EQU		*
PRINT			TFR		X,U
* 文字数を数える
				CLRB
PRINT0			LEAU	BUFFER,PCR
				LDA		#$03
				CLRB
				STA		,U+
				STB		,U+
PRINT1			LDA		,X+
				TSTA
				BEQ		PRINT2
				STA		,U+
				INCB
				BRA		PRINT1
PRINT2			PSHS	A,X
				LEAX	BUFFER,PCR
				STB		1,X
				INCB
				INCB
				LBSR	SUB_CMD
				PULS	A,X
				RTS

*--[ RS-232Cドライバ ]--------------------------------------------------------

	if	defined(RSDRIVER)
	include	rsdriver.asm
	endif

*--[ DOS ]--------------------------------------------------------------------

	include	diskdrv.asm

*--[ DATA ]-------------------------------------------------------------------

*
*	サブCPU コマンド (FM-7)
*
SUBINIT_CMD		FCB		$01,$00,40,25,0,25,0,1,0
SUBCONS_CMD		FCB		$0C,$06
INKEY_CMD		FCB		$29,$03
INKEY_CHK_CMD	FCB		$29,$00

*
*	ファイルネーム
*

FBASIC_FN		FCC		/FBASIC10ROM/,0
BOOTBAS_FN		FCC		/BOOTBAS8ROM/,0
BOOTDOS_FN		FCC		/BOOTDOS8ROM/,0
BOOTBBL_FN		FCC		/BOOTBBL8ROM/,0
BOOTDBG_FN		FCC		/BOOTDBG8ROM/,0
SUBSYSC_FN		FCC		/SUBSYS_8ROM/,0
KANJI_FN		FCC		/KANJI   ROM/,0
*
*	表示メッセージ(起動時)
*

	if	(DISKTYPE==D_NORM2D)
TITLE_MSG		FCB		$12,$06,$06,$11,$07
				FCC		/XM7 ROM SAVE UTILITY for FM-8/
				FCB		$12,$02,$08
				FCC		"(C) 1999 PI. / 2003-2017 Ryu Takegami",0
TITLE_MSG2		FCB		$12,$06,$10,$11,$04
				FCC		/Set blank disk to drive 0 and/,0
	endif
FORMAT_MSG		FCB		$12,$0A,$09,$11,$07
				FCC		/Disk formatting .../,0

*
* 表示メッセージ(ROMセーブ時)
*
FBASIC_MSG		FCB		$12,$09,$09,$11,$07
				FCC		/Saving F-BASIC ROM .../,0
BOOTBAS_MSG		FCB		$12,$05,$09,$11,$07
				FCC		/Saving BOOT ROM(BASIC mode) .../,0
BOOTDOS_MSG		FCB		$12,$06,$09,$11,$07
				FCC		/Saving BOOT ROM(DOS mode) .../,0
BOOTBBL_MSG		FCB		$12,$06,$09,$11,$07
				FCC		/Saving BOOT ROM(BBL mode) .../,0
BOOTDBG_MSG		FCB		$12,$07,$09,$11,$07
				FCC		/Saving BOOT ROM(DBG mode) .../,0
KANJI_MSG		FCB		$12,$07,$09,$11,$07
				FCC		/Saving Kanji ROM (JIS78) .../,0
BOOTB_DIPSW_MSG	FCB		$12,$02,$09,$11,$07
				FCC		/Please turn ON DIP SW 9 and DIP SW 10./,0
BOOTD_DIPSW_MSG	FCB		$12,$07,$09,$11,$07
				FCC		/Please turn OFF DIP SW 9 and/
				FCB		$12,$0b,$0b
				FCC		/turn ON DIP SW 10./,0
BOOTL_DIPSW_MSG	FCB		$12,$07,$09,$11,$07
				FCC		/Please turn ON DIP SW 9 and/
				FCB		$12,$0b,$0b
				FCC		/turn OFF DIP SW 10./,0
BOOTG_DIPSW_MSG	FCB		$12,$01,$09,$11,$07
				FCC		/Please turn OFF DIP SW 9 and DIP SW 10./,0
SUBSYSC_MSG		FCB		$12,$02,$09,$11,$07
				FCC		/Saving Sub monitor ROM (200Line) .../,0

*
* 表示メッセージ(エラー)
*
NOTREADY_MSG	FCB		$12,$0C,$10,$11,$02
				FCC		/Drive not ready./,0
WRPROTECT_MSG	FCB		$12,$0A,$10,$11,$02
				FCC		/Disk write protected./,0
SEEKERR_MSG		FCB		$12,$0F,$10,$11,$02
				FCC		/Seek error./,0
READERR_MSG		FCB		$12,$0F,$10,$11,$02
				FCC		/Read error./,0
WRITEERR_MSG	FCB		$12,$0F,$10,$11,$02
				FCC		/Write error./,0
SPACE_MSG		FCB		$12,$0D,$12,$11,$04
				FCC		/Push SPACE key./,0
CLEAR_MSG		FCB		$12,$0A,$10,$11,$07
				FCC		/                     /
				FCB		$12,$0d,$12
				FCC		/               /,0
COMPLETE_MSG	FCB		$12,$0F,$09,$11,$07
				FCC		/Completed./,0

*
* プログラム終了
*
PRGEND			EQU		*
				END
