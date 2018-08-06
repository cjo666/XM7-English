*
*	FM-7 EMULATOR "XM7"
*
*	Copyright (C) 1999-2017 �o�h�D(Twitter:@xm6_original)
*	Copyright (C) 2001-2017 Ryu Takegami (Twitter:@RyuTakegami)
*
*	[ ROM�Z�[�o(XM7 V1/V2/V3�Ή�) ]
*
*	�Ώۋ@��E�������f�B�A�^�C�v�ɉ����ăA�Z���u�����ɃI�v�V�����w�肷�邱��
*		-dMODEL=0		FM-7�V���[�Y�S�@��   (SAVEROM7)
*		-dMODEL=1		FM77AV�V���[�Y�S�@�� (SAVEROM)
*		-dMODEL=2		FM77AV40/EX/SX       (SAVEROM2)
*		-dMODEL=3		FM77AV40EX/SX        (SAVEKNJ)
*		-dMODEL=4		FM77AV ����ROM       (SAVEJDAV)
*		-dDISKTYPE=0	360KB(2D)  �ʏ�t�H�[�}�b�g
*		-dDISKTYPE=1	720KB(2DD) 1�g���b�N�����t�H�[�}�b�g
*		-dDISKTYPE=2	720KB(2DD) �ʏ�t�H�[�}�b�g
*		-dFBLOADM		DISK BASIC�p
*

	if	defined(FBLOADM)
				ORG		$0FE0
	else
				ORG		$1000
	endif

*
*	�萔��` (���C���V�X�e��)
*
STACK			EQU		$0F00			* �X�^�b�N�ŏI
BUFFER			EQU		$0F80			* �ėp�o�b�t�@
KANJI_BUF		EQU		$0FE8			* �����o�b�t�@
DISK_BUF		EQU		$2800			* �f�B�X�N�o�b�t�@
SHARED			EQU		$FC80			* ���LRAM

*
*	�萔��` (���C��I/O)
*
KEYDAT			EQU		$FD01			* �L�[�f�[�^���W�X�^
SUBMODE			EQU		$FD04			* �T�u�V�X�e�����[�h���W�X�^
SUBSIN			EQU		$FD05			* �T�u�V�X�e���C���^�t�F�[�X���W�X�^
RS232D			EQU		$FD06			* USART �f�[�^���W�X�^
RS232S			EQU		$FD07			* USART �X�e�[�^�X���W�X�^
RS232C			EQU		RS232S			* USART �R�}���h���W�X�^
RSBAUD			EQU		$FD0B			* RS-232C �{�[���[�g���W�X�^
RSENABLE		EQU		$FD0C			* RS-232C �C�l�[�u�����W�X�^
BANKSEL			EQU		$FD0F			* ROM/��RAM�Z���N�g���W�X�^
INITDIS			EQU		$FD10			* �C�j�V�G�[�^ROM �f�B�Z�[�u�����W�X�^
SUBSTAT			EQU		$FD12			* �T�u���[�h�X�e�[�^�X���W�X�^
SUBBANK			EQU		$FD13			* �T�u�o���N���W�X�^
FDCSTAT			EQU		$FD18			* FDC �X�e�[�^�X���W�X�^
FDCCMD			EQU		FDCSTAT			* FDC �R�}���h���W�X�^
FDCTRACK		EQU		$FD19			* FDC �g���b�N���W�X�^
FDCSECTOR		EQU		$FD1A			* FDC �Z�N�^���W�X�^
FDCDATA			EQU		$FD1B			* FDC �f�[�^���W�X�^
FDDSIDE			EQU		$FD1C			* FDD �T�C�h���W�X�^
FDDDRIVE		EQU		$FD1D			* FDD �h���C�u���W�X�^
FDDMODE			EQU		$FD1E			* FDD ���[�h���W�X�^
FDDDRQ			EQU		$FD1F			* FDC DRQ/IRQ���W�X�^
KNJ1ADR			EQU		$FD20			* ��ꐅ������ �A�h���X���W�X�^
KNJ1DATA		EQU		$FD22			* ��ꐅ������ �f�[�^���W�X�^
KNJ2ADR			EQU		$FD2C			* ��񐅏����� �A�h���X���W�X�^
KNJ2DATA		EQU		$FD2E			* ��񐅏����� �f�[�^���W�X�^
DICBANK			EQU		$FD2E			* ����ROM �o���N���W�X�^
MMR				EQU		$FD80			* MMR �������}�b�s���O���W�X�^
MSGR			EQU		$FD90			* MMR �Z�O�����g���W�X�^
MMSR			EQU		$FD93			* MMR ���[�h�Z���N�g���W�X�^
MSSW1			EQU		$FD94			* ���[�h�Z���N�g�X�C�b�`1 (40/20)
MSSW2			EQU		$FD95			* ���[�h�Z���N�g�X�C�b�`2 (EX)

*
*	�萔��` (�T�uI/O)
*
SUBMODEL4		EQU		$D405			* �T�u���[�h���W�X�^ (L4)
CRTDSP			EQU		$D408			* CRT ON/OFF���W�X�^
VACC			EQU		$D409			* VRAM�A�N�Z�X�t���O
ALUCMD			EQU		$D410			* �_�����Z�R�}���h���W�X�^
ALUCOL			EQU		$D411			* �_�����Z�J���[���W�X�^
ALUMASK			EQU		$D412			* �_�����Z�}�X�N���W�X�^
ALUDIS			EQU		$D41B			* �_�����Z�o���N�f�B�Z�[�u�����W�X�^
KENCDAT			EQU		$D431			* �L�[�G���R�[�_�f�[�^���W�X�^
KENCSTA			EQU		$D432			* �L�[�G���R�[�_�X�e�[�^�X���W�X�^

*
*	���[�N�G���A��`
*
RS_FLAG			EQU		$00			* �V���A���o�̓t���O
DRIVE			EQU		$01			* �A�N�Z�X�h���C�u
TRACK			EQU		$02			* �A�N�Z�X�g���b�N
DRIVE0_T		EQU		$03			* �h���C�u0�̃g���b�N
DRIVE1_T		EQU		$04			* �h���C�u1�̃g���b�N
SIDE			EQU		$05			* �A�N�Z�X�T�C�h
SECTOR			EQU		$06			* �A�N�Z�X�Z�N�^
SECTORS			EQU		$07			* �A���Z�N�^��
CLUSTER			EQU		$08			* �t�@�C���擪�N���X�^
CLUSTERS		EQU		$0A			* �g�[�^���N���X�^��(1�N���X�^1KB)
LASTBYTE		EQU		$0C			* �ŏI�N���X�^�̃o�C�g��
COLOR			EQU		$0E			* �����J���[�R�[�h
TYPE			EQU		$0F			* FDC�R�}���h�^�C�v
C_TRACK			EQU		$10			* �J�����g�g���b�N
C_SIDE			EQU		$11			* �J�����g�T�C�h
C_SECTOR		EQU		$12			* �J�����g�Z�N�^
C_SECTORS		EQU		$13			* �J�����g�Z�N�^��
FM77_FLAG		EQU		RS_FLAG		* FM-77�t���O
L4_FLAG			EQU		COLOR		* 400���C���t���O (L4)

*
*	�A�Z���u��������`
*
M_FM7			EQU		0			* FM-7�V���[�Y�S�@��
M_FM77AV		EQU		1			* FM77AV�V���[�Y�S�@��
M_FM77AV40		EQU		2			* FM77AV40/EX/SX
M_FM77AVDIC		EQU		3			* FM77AV ����ROM
M_KANJI			EQU		4			* FM77AV40EX/SX(����ROM)
D_NORM2D		EQU		0			* 2D (360KB) �ʏ�t�H�[�}�b�g
D_SKIP2DD		EQU		1			* 2DD(720KB) �P�g���b�N�����t�H�[�}�b�g
D_NORM2DD		EQU		2			* 2DD(720KB) �ʏ�t�H�[�}�b�g

*
*	���@CMT I/F�o�R��������p �]���v���O����
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
*	�v���O�����X�^�[�g
*
START			BRA		MAIN

*
*	�v���O�����w�b�_
*
HEADER			FCC		/TINY-DOS(mini) for FM-7  /
				FCC		/Copyright 1991-1992,1999 by �o�h�D/
				FCB		$0D,$0A,$00

HEADER2			EQU		*
	if	(MODEL==M_FM77AV)
				FCC		/SAVEROM /
	elsif	(MODEL==M_FM77AV40)
				FCC		/SAVEROM2/
	elsif	(MODEL==M_FM7)
				FCC		/SAVEROM7/
	elsif	(MODEL==M_FM77AVDIC)
				FCC		/SAVEJDAV/
	else
				FCC		/SAVEKNJ /
	endif
	if	(MODEL!=M_FM77AVDIC)
				FCC		/ V1.37  /
	else
				FCC		/ V1.00  /
	endif
				FCC		"Copyright 1999 �o�h�D/ 2003-2017 Ryu Takegami"
				FCB		$0D,$0A,$00

*
*	���C���v���O����
*
				ORG		START+$80
MAIN			ORCC	#$50
	if	defined(FBLOADM)
				LDS		#STACK
	else
				LEAS	STACK,PCR
	endif

* �n�[�h�E�F�A������
	if	defined(RSDRIVER)
				LBSR	RS_INIT
	endif
				LBSR	INIT
	if	(MODEL>=M_FM77AV)
				LBSR	FM77AV_INIT
	else
				LBSR	FM7_INIT
	endif

* SAVEJDIC�ł̎���ROM�m�F
	if	(MODEL==M_FM77AVDIC)
				LBSR	DICCHK
				LBCC	DICOK
* �G���[���b�Z�[�W��\�����Ė������[�v
				LEAX	DICROM_EMSG,PCR
				LBSR	PUT_STRING
				BRA		*
DICOK			EQU		*
	endif

* �N���^�C�g���\��
	if	(DISKTYPE==D_NORM2D)||(defined(FBLOADM))
				LEAX	TITLE_MSG,PCR
				LBSR	PUT_STRING
				LEAX	TITLE_MSG2,PCR
				LBSR	PUT_STRING
				LEAX	SPACE_MSG,PCR
				LBSR	PUT_STRING
				LBSR	KEY_SPACE
	endif

* �f�B�X�N�t�H�[�}�b�g
				LBSR	DISK_RST
				LBSR	MS_FORMAT

* ���C�����Z�[�u����
MAINSAVE		EQU		*
	if	(MODEL!=M_FM77AVDIC)
				LBSR	FBASIC
				LBSR	BOOT_BAS
				LBSR	BOOT_DOS
	endif
	if	(MODEL==M_FM7)
				LBSR	BOOT_1MB
				LBSR	BOOT_MMR
	endif
	if	(MODEL==M_FM77AV||MODEL==M_FM77AV40)
				LBSR	INITIATE
	endif
	if	(MODEL!=M_FM77AVDIC)
				LBSR	KANJI
	endif
	if	(MODEL==M_FM77AV||MODEL==M_FM77AV40)
				LBSR	KANJI_2
	endif
	if	(MODEL==M_FM77AV40||MODEL==M_FM77AVDIC)
				LBSR	DICCHK
				LBSR	DICROM
	endif

* �T�u���Z�[�u����
SUBSAVE			EQU		*
	if	(MODEL==M_FM77AV||MODEL==M_FM77AV40)
				LBSR	SUBSYS_A
				LBSR	SUBSYS_B
				LBSR	SUBSYS_C
				LBSR	SUBSYS_CG
	elsif (MODEL==M_FM7)
				LBSR	SUBSYS_C
				LBSR	SUBSYS_L4
	endif
	if	(MODEL==M_FM77AV40)
				LBSR	EXTSUB
				LBSR	BOOT_MMR
	endif

* ����(���b�Z�[�W�\���E���[�v)
				LBSR	CLS
				LEAX	COMPLETE_MSG,PCR
				LBSR	PUT_STRING
				BRA		*


*
* ������
*
INIT			LEAX	START,PCR
				LEAX	-$100,X
				TFR		X,D
				TFR		A,DP
* ���[�N�N���A
				CLRA
INIT1			CLR	,X+
				DECA
				BNE	INIT1
* ��ʏ�����
				LEAX	SUBINIT_CMD,PCR
				LDB		#$09
				LBSR	SUB_CMD
				LEAX	SUBCONS_CMD,PCR
				LDB		#$02
				LBSR	SUB_CMD
* FM77AV�`�F�b�N
	if	(MODEL==M_FM77AV)||(MODEL==M_FM77AVDIC)
				LBSR	IS_FM77AV
	elsif	(MODEL==M_FM77AV40)
				LBSR	IS_SUPPORT2DD
	endif
	if	(MODEL>=M_FM77AV)
				BCC		INIT2
				LEAX	FM77AV_MSG,PCR
				LBSR	PRINT
				BRA		*
	endif
INIT2			RTS

*--[ ROM�Z�[�o(�S�@�틤�ʕ�) ]------------------------------------------------

	if (MODEL!=M_FM77AVDIC)

*
*	F-BASIC ROM
*
FBASIC			LBSR	CLS
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
*	KANJI ROM (��ꐅ��)
*
KANJI			CLRA
				CLRB
				STD		KNJ1ADR
				LDD		KNJ1DATA
				CMPD	#$FFFF
				BEQ		KANJI5

				LBSR	CLS
*	if	(MODEL!=M_FM77AV40)
				LDD		#$0017
				STD		KNJ1ADR
				LDD		KNJ1DATA
				CMPD	#$FFFF
				BEQ		KANJI1
				LEAX	KANJI_MSG,PCR
				LBSR	PUT_STRING
				LEAX	KANJI_FN,PCR
				BRA		KANJI2
*	endif
KANJI1			LEAX	KANJI1_MSG,PCR
				LBSR	PUT_STRING
				LEAX	KANJI1_FN,PCR
KANJI2			LDD		#$80
				STD		<CLUSTERS
				LDD		#1024
				STD		<LASTBYTE
				LBSR	FILE_CREATE
* 8�񃋁[�v
				LDX		#$0000
KANJI3			LDU		#$4000
KANJI4			STX		KNJ1ADR
				LDD		KNJ1DATA
				STD		,U++
				LEAX	1,X
				CMPU	#$8000
				BNE		KANJI4
* �Z�[�u
				PSHS	X
				LDX		#$4000
				LDU		#16
				LBSR	FILE_WRITE
				PULS	X
* NEXT
				CMPX	#$0000
				BNE		KANJI3
KANJI5			RTS

	endif

*--[ ROM�Z�[�o(FM-7�V���[�Y) ]------------------------------------------------

	if	(MODEL==M_FM7)

*
*	BOOT(BASIC) ROM
*
BOOT_BAS		LBSR	CLS
				LBSR	IS_FM77AV
				BCC		BOOT_BAS6
BOOT_BAS1		LDX		$FE03
				CMPX	#$FE61
				BEQ		BOOT_BAS5
				CMPX	#$FE87
				BEQ		BOOT_BAS5
				LDA		FM77_FLAG
				BEQ		BOOT_BAS2
				LEAX	BOOTB_DIPSW_MSG,PCR
				BRA		BOOT_BAS3
BOOT_BAS2		LEAX	BOOTB_DIP77_MSG,PCR
BOOT_BAS3		LBSR	PRINT
BOOT_BAS4		LDX		$FE03
				CMPX	#$FE61
				BEQ		BOOT_BAS5
				CMPX	#$FE87
				BNE		BOOT_BAS4
BOOT_BAS5		LBSR	CLS
BOOT_BAS6		LEAX	BOOTBAS_MSG,PCR
				LBSR	PRINT
				LEAX	BOOTBAS_FN,PCR
				LDD		#1
				STD		<CLUSTERS
				LDD		#512
				STD		<LASTBYTE
				LBSR	FILE_CREATE
				LBSR	IS_FM77AV
				BCC		BOOT_BAS7
				LDX		#$FE00
				BRA		BOOT_BAS8
BOOT_BAS7		CLR		INITDIS
				LDX		#$7800
BOOT_BAS8		LDU		#1
				LBSR	FILE_WRITE
				LDA		#$02
				STA		INITDIS
				RTS

*
*	BOOT(DOS) ROM
*
BOOT_DOS		LBSR	CLS
				LBSR	IS_FM77AV
				BCC		BOOT_DOS6
BOOT_DOS1		LDX		$FE03
				CMPX	#$FE65
				BEQ		BOOT_DOS5
				CMPX	#$FE71
				BEQ		BOOT_DOS5
				LDA		FM77_FLAG
				BEQ		BOOT_DOS2
				LEAX	BOOTD_DIPSW_MSG,PCR
				BRA		BOOT_DOS3
BOOT_DOS2		LEAX	BOOTD_DIP77_MSG,PCR
BOOT_DOS3		LBSR	PRINT
BOOT_DOS4		LDX		$FE03
				CMPX	#$FE65
				BEQ		BOOT_DOS5
				CMPX	#$FE71
				BNE		BOOT_DOS4
BOOT_DOS5		LBSR	CLS
BOOT_DOS6		LEAX	BOOTDOS_MSG,PCR
				LBSR	PRINT
				LEAX	BOOTDOS_FN,PCR
				LDD		#1
				STD		<CLUSTERS
				LDD		#512
				STD		<LASTBYTE
				LBSR	FILE_CREATE
				LBSR	IS_FM77AV
				BCC		BOOT_DOS7
				LDX		#$FE00
				BRA		BOOT_DOS8
BOOT_DOS7		CLR		INITDIS
				LDX		#$7A00
BOOT_DOS8		LDU		#1
				LBSR	FILE_WRITE
				LDA		#$02
				STA		INITDIS
				RTS

*
*	BOOT(1MB DOS) ROM
*
BOOT_1MB		LBSR	CLS
				LBSR	IS_FM77AV
				BCC		BOOT_1MB4
				TST		<FM77_FLAG
				BNE		BOOT_1MB4
BOOT_1MB1		LDX		$FE03
				CMPX	#$FE8D
				BEQ		BOOT_1MB3
				LEAX	BOOTS_DIP77_MSG,PCR
				LBSR	PRINT
BOOT_1MB2		LDX		$FE03
				CMPX	#$FE8D
				BNE		BOOT_1MB2
BOOT_1MB3		LBSR	CLS
				LEAX	BOOT1MB_MSG,PCR
				LBSR	PRINT
				LEAX	BOOT1MB_FN,PCR
				LDD		#1
				STD		<CLUSTERS
				LDD		#512
				STD		<LASTBYTE
				LBSR	FILE_CREATE
				LDX		#$FE00
				LDU		#1
				LBSR	FILE_WRITE
BOOT_1MB4		RTS

*
*	BOOT(MMR $3F BANK) ROM
*
BOOT_MMR		LBSR	CLS
				LBSR	IS_FM77AV
				BCC		BOOT_MMR2
				TST		<FM77_FLAG
				BNE		BOOT_MMR2
BOOT_MMR1		LEAX	BOOTMMR_MSG,PCR
				LBSR	PRINT
				LEAX	BOOTMMR_FN,PCR
				LDD		#$3F80
				STA		MMR+4
				STB		MMSR
				LDD		#1
				STD		<CLUSTERS
				LDD		#512
				STD		<LASTBYTE
				LBSR	FILE_CREATE
				LDX		#$4E00
				LDU		#1
				LBSR	FILE_WRITE
				LDD		#$3400
				STA		MMR+4
				STB		MMSR
BOOT_MMR2		RTS

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
* �T�u���j�^�]��
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
* �Z�[�u
				LDX		#$4800
				LDU		#10
				LBSR	FILE_WRITE
				RTS

*
*	SUBSYSTEM (400LINE) ROM
*
SUBSYS_L4		LDA		<FM77_FLAG
				BNE		SUBSYS_L4_4
* 400���C���J�[�h���ڃ`�F�b�N
				LDA		SUBMODE
				BITA	#$10
				BNE		SUBSYS_L4_4
* 400���C�����[�h�Ɉڍs
				LBSR	INITL4
*
				LBSR	CLS
				LEAX	SUBSYSL4_MSG,PCR
				LBSR	PRINT
				LEAX	SUBSYSL4_FN,PCR
				LDD		#18
				STD		<CLUSTERS
				LDD		#1024
				STD		<LASTBYTE
				LBSR	FILE_CREATE
* ��RAM���Z���N�g
				STA		BANKSEL
* �T�u���j�^�O���]��(�]���R�}���h�̓^�C�vC�p�Ƌ��p)
				LDX		#$9800
				LDY		#$4000
SUBSYS_L4_1		PSHS	X,Y
				STX		SRCADR,PCR
				LEAX	TFR_CMD,PCR
				LDB		#TFR_CMDe-TFR_CMD
				LBSR	SUB_CMD
				PULS	X,Y
				LBSR	SUB_HALT
				LDU		#$FCC0
				LDB		#$40
SUBSYS_L4_2		LDA		,U+
				STA		,Y+
				DECB
				BNE		SUBSYS_L4_2
				LDA		#$80
				STA		SHARED
				CLR		SUBSIN
				LEAX	$40,X
				BEQ		SUBSYS_L4_3
				CMPX	#$C000
				BNE		SUBSYS_L4_1
				LDX		#$E000
				BRA		SUBSYS_L4_1
* �Z�[�u
SUBSYS_L4_3		LDX		#$4000
				LDU		#18
				LBSR	FILE_WRITE
* F-BASIC ROM���Z���N�g
				TST		BANKSEL
* 200���C�����[�h�ɖ߂�
				LBSR	INITL2
SUBSYS_L4_4		RTS

*
* �T�u���j�^�]���p�����e�i���X�R�}���h
*
TFR_CMD			FCB		$3F,$59,$41,$4D,$41,$55,$43,$48,$49
				FCB		$91
SRCADR			FDB		$0000,$D3C0,$0040
				FCB		$90
TFR_CMDe		EQU		*

	elsif (MODEL==M_FM77AV||MODEL==M_FM77AV40)

*--[ ROM�Z�[�o(FM77AV�V���[�Y���ʕ�) ]----------------------------------------

*
* BOOT(BASIC) ROM
*
BOOT_BAS		LBSR	CLS
				LEAX	BOOT_MSG,PCR
				LBSR	PUT_STRING
				LEAX	BOOTBAS_FN,PCR
				LDD		#1
				STD		<CLUSTERS
				LDD		#512
				STD		<LASTBYTE
				LBSR	FILE_CREATE
				LDX		#$7800
				LDU		#1
				CLR		INITDIS
				LBSR	FILE_WRITE
				LDA		#$02
				STA		INITDIS
				RTS

*
* BOOT(DOS) ROM
*
BOOT_DOS		LEAX	BOOTDOS_FN,PCR
				LDD		#1
				STD		<CLUSTERS
				LDD		#512
				STD		<LASTBYTE
				LBSR	FILE_CREATE
				LDX		#$7A00
				LDU		#1
				CLR		INITDIS
				LBSR	FILE_WRITE
				LDA		#$02
				STA		INITDIS
				RTS

*
* SUBSYSTEM (TYPE-C) ROM
*
SUBSYS_C		CLR		SUBBANK
				CLR		SUBSIN
				LBSR	SUB_HALT
				LBSR	CLS
				LEAX	SUBSYSC_MSG,PCR
				LBSR	PUT_STRING
				LEAX	SUBSYSC_FN,PCR
				LDD		#10
				STD		<CLUSTERS
				LDD		#1024
				STD		<LASTBYTE
				LBSR	FILE_CREATE
				LDD		#$1D1E
				STD		MMR+4
				INCB
				STB		MMR+6
				LDX		#$4800
				LDU		#10
				LBSR	FILE_WRITE
				LDD		#$3435
				STD		MMR+4
				INCB
				STB		MMR+6
				RTS

*
* INITIATE ROM
*
INITIATE		LBSR	CLS
				LEAX	INITIATE_MSG,PCR
				LBSR	PUT_STRING
				LEAX	INITIATE_FN,PCR
				LDD		#8
				STD		<CLUSTERS
				LDD		#1024
				STD		<LASTBYTE
				LBSR	FILE_CREATE
				LDX		#$6000
				LDU		#8
				CLR		INITDIS
				LBSR	FILE_WRITE
				LDA		#$02
				STA		INITDIS
				RTS

*
* SUBSYSTEM (TYPE-A) ROM
*
SUBSYS_A		LDA		#$01
				STA		SUBBANK
				CLR		SUBSIN
				LBSR	SUB_HALT
				LBSR	CLS
				LEAX	SUBSYSA_MSG,PCR
				LBSR	PUT_STRING
				LEAX	SUBSYSA_FN,PCR
				LDD		#8
				STD		<CLUSTERS
				LDD		#1024
				STD		<LASTBYTE
				LBSR	FILE_CREATE
				LDD		#$1E1F
				STD		MMR+4
				LDX		#$4000
				LDU		#8
				LBSR	FILE_WRITE
				LDD		#$3435
				STD		MMR+4
				RTS

*
* SUBSYSTEM (TYPE-B) ROM
*
SUBSYS_B		LDA		#$02
				STA		SUBBANK
				CLR		SUBSIN
				LBSR	SUB_HALT
				CLR		SUBSTAT
				LBSR	CLS
				LEAX	SUBSYSB_MSG,PCR
				LBSR	PUT_STRING
				LEAX	SUBSYSB_FN,PCR
				LDD		#8
				STD		<CLUSTERS
				LDD		#1024
				STD		<LASTBYTE
				LBSR	FILE_CREATE
				LDD		#$1E1F
				STD		MMR+4
				LDX		#$4000
				LDU		#8
				LBSR	FILE_WRITE
				LDD		#$3435
				STD		MMR+4
				RTS

*
* SUBSYSTEM (CG) ROM
*
SUBSYS_CG		CLR		SUBBANK
				CLR		SUBSIN
				LBSR	SUB_HALT
				LBSR	CLS
				LEAX	SUBSYSCG_MSG,PCR
				LBSR	PUT_STRING
				LEAX	SUBSYSCG_FN,PCR
				LDD		#8
				STD		<CLUSTERS
				LDD		#1024
				STD		<LASTBYTE
				LBSR	FILE_CREATE
				LDA		#$1D
				STA		MMR+4
* �S�񏑂�����
				CLRA
SUBSYS_CG1		PSHS	A
				STA		$4430
				LDX		#$4800
				LDU		#2
				LBSR	FILE_WRITE
				PULS	A
				INCA
				CMPA	#$04
				BCS		SUBSYS_CG1
* �I��
				LDA		#$34
				STA		MMR+4
				RTS

*
*	KANJI ROM (��Q����,�`�F�b�N��������)
*
KANJI_2			EQU		*
* ��2��������ROM���ڃ`�F�b�N(200���C���@�n�p)
				CLRA
				CLRB
				STD		KNJ2ADR
				LDD		KNJ2DATA
				CMPD	#$FFFF
				BEQ		KANJI2_3

				LBSR	CLS
				LEAX	KANJI2_MSG,PCR
				LBSR	PUT_STRING
				LEAX	KANJI2_FN,PCR
				LDD		#$80
				STD		<CLUSTERS
				LDD		#1024
				STD		<LASTBYTE
				LBSR	FILE_CREATE
* 8�񃋁[�v
				LDX		#$0000
KANJI2_1		LDU		#$4000
KANJI2_2		STX		KNJ2ADR
				LDD		KNJ2DATA
				STD		,U++
				LEAX	1,X
				CMPU	#$8000
				BNE		KANJI2_2
* �Z�[�u
				PSHS	X
				LDX		#$4000
				LDU		#16
				LBSR	FILE_WRITE
				PULS	X
* NEXT
				CMPX	#$0000
				BNE		KANJI2_1
KANJI2_3		RTS

	endif

*--[ ROM�Z�[�o(FM77AV40�n/���{��J�[�h) ]-------------------------------------

	if	(MODEL==M_FM77AV40||MODEL==M_FM77AVDIC)

*
*	����ROM���݃`�F�b�N
*
*	RESULT: CY	�Ȃ�(ON),����(OFF)
*
DICCHK			PSHS	D
				LDD		#$2E40
				STA		MMR+4
				STB		DICBANK
				CLR		MSSW2
				LDB		#$34

* �o���N�؂芷�����s��ꂽ���`�F�b�N����
				CLR		$4080
				LDA		$4080
				CLR		DICBANK
				CMPA	$4080
				BEQ		DICCHK2

* ����ROM����
DICCHK1			STB		MMR+4
				PULS	D
				ANDCC	#$FE
				RTS

* ����ROM�Ȃ�
DICCHK2			STB		MMR+4
				PULS	D
				ORCC	#$01
				RTS

*
*	����ROM
*
DICROM			LDD		#$2E40
				STA		MMR+4

* ����ROM�}�b�s���O�E�Z���N�g
				STB		DICBANK
				CLR		MSSW2

				LBSR	CLS
				LEAX	DICROM_MSG,PCR
				LBSR	PUT_STRING
				LEAX	DICROM_FN,PCR
				LDD		#$100
				STD		<CLUSTERS
				LDD		#1024
				STD		<LASTBYTE
				LBSR	FILE_CREATE

* 64�񃋁[�v
				LDA		#$40
DICROM1			PSHS	A
				STA		DICBANK
				LDX		#$4000
				LDU		#4
				LBSR	FILE_WRITE
				PULS	A
				INCA
				BPL		DICROM1

* NEXT
DICEXIT			LDD		#$3400
				STA		MMR+4
				STB		DICBANK
				RTS

	endif

*--[ ROM�Z�[�o(FM77AV40�n�ŗL��) ]--------------------------------------------

	if	(MODEL==M_FM77AV40)

*
*	�g���T�u�V�X�e��ROM(AV40EX/SX only)
*
EXTSUB			CLR		INITDIS
				LDA		$6B0E
				LDB		$6B10
				CMPD	#$3431
				BNE		EXTSUB2

* �g��ROM�}�b�s���O�E�Z���N�g
				LDD		#$2E88
				STA		MMR+4
				STB		MSSW2

				LBSR	CLS
				LEAX	EXTSUB_MSG,PCR
				LBSR	PUT_STRING
				LEAX	EXTSUB_FN,PCR
				LDD		#$30
				STD		<CLUSTERS
				LDD		#1024
				STD		<LASTBYTE
				LBSR	FILE_CREATE
* 12�񃋁[�v
				LDA		#$60
EXTSUB1			PSHS	A
				STA		DICBANK
				LDX		#$4000
				LDU		#4
				LBSR	FILE_WRITE
				PULS	A
				INCA
				CMPA	#$6C
				BLO		EXTSUB1
* NEXT
EXTSUB2			LDD		#$3402
				STA		MMR+4
				CLR		DICBANK
				CLR		MSSW2
				STB		INITDIS
				RTS

*
*	FM-77��BOOT(MMR $3F BANK)����ROM
*
BOOT_MMR		CLR		INITDIS
				LDA		$6B0E
				LDB		$6B10
				CMPD	#$3431
				BNE		BOOT_MMR2

* �g��ROM�}�b�s���O�E�Z���N�g
				LDD		#$2E88
				STA		MMR+4
				STB		MSSW2

				LBSR	CLS
				LEAX	BOOTMMR_MSG,PCR
				LBSR	PUT_STRING
				LEAX	BOOTMMR_FN,PCR
				LDD		#1
				STD		<CLUSTERS
				LDD		#512
				STD		<LASTBYTE
				LBSR	FILE_CREATE
				LDA		#$7F
				STA		DICBANK
				LDX		#$4E00
				LDU		#4
				LBSR	FILE_WRITE
* NEXT
BOOT_MMR2		LDD		#$3402
				STA		MMR+4
				CLR		DICBANK
				CLR		MSSW2
				STB		INITDIS
				RTS

	endif

*--[ �@�픻�� ]---------------------------------------------------------------

*
*	FM77AV����
*
*	RESULT: CY	FM-7(ON),FM77AV(OFF)
*
IS_FM77AV		PSHS	A
* �W��RAM�֐؂�ւ��A$00����������
				LDA		#$02
				STA		INITDIS
				CLR		$6000
* �C�j�V�G�[�^ROM�֐؂�ւ��A�ǂݏo���`�F�b�N
				CLR		INITDIS
				TST		$6000
				BEQ		IS_FM77AV1
* �Y�ꂸ�ɕW��RAM�ɖ߂��Ă���
				STA		INITDIS
				PULS	A
				ANDCC	#$FE
				RTS
* ������͖߂��K�v�͂Ȃ��̂������...
IS_FM77AV1		STA		INITDIS
				PULS	A
				ORCC	#$01
				RTS

*
*	2DD�Ή��@����
*
*	RESULT: CY	2DD���Ή�(ON),2DD�Ή�(OFF)
*
IS_SUPPORT2DD	PSHS	A
* �W��RAM�֐؂�ւ��A$00����������
				LDA		#$02
				STA		INITDIS
				CLR		$6B0E
* �C�j�V�G�[�^ROM�֐؂�ւ��A�ǂݏo���`�F�b�N
				CLR		INITDIS
				LDA		$6B0E
				CMPA	#$34
				BEQ		IS_SUPPORT2DD1
				CMPA	#$32
				BNE		IS_SUPPORT2DD2
* �Y�ꂸ�ɕW��RAM�ɖ߂��Ă���
IS_SUPPORT2DD1	LDA		#$02
				STA		INITDIS
				PULS	A
				ANDCC	#$FE
				RTS
* �Y�ꂸ�ɕW��RAM�ɖ߂��Ă���
IS_SUPPORT2DD2	LDA		#$02
				STA		INITDIS
				PULS	A
				ORCC	#$01
				RTS

*--[ FM-7 ]-------------------------------------------------------------------

	if	(MODEL==M_FM7)

*
*	FM-7������ (77�`�F�b�N�t��)
*
FM7_INIT		LBSR	MMR_INIT
				TST		BANKSEL
				LDD		#$0201
				STA		INITDIS
* �u�[�gROM�̗�RAM���g�p�ł��邩�m�F (�g�p�ł���Ȃ�FM-77)
				STB		MMSR
				LDB		$FE02
				CLR		$FE02
				LDA		$FE02
				STA		<FM77_FLAG
				STB		$FE02
				CLR		MMSR
				RTS

*
*	�X�y�[�X�҂�
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
*	�X�y�[�X���̓`�F�b�N
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
*	��ʃN���A
*
CLS				EQU		*
				TST		L4_FLAG
				BEQ		CLS_L2
* 400���C������ERASE�R�}���h�𗘗p(�����̎蔲��)
				LEAX	CLSL4_CMD,PCR
				LDB		#3
				LBRA	SUB_CMD
* 200���C�����͓Ǝ��v���O�����ŏ���
CLS_L2			EQU		*
				LEAX	CLS_CMD,PCR
				LDB		#CLS_CMDe-CLS_CMD
				LBRA	SUB_CMD

*
*	�T�uCPU ��ʃN���A�v���O����
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

*
*	�T�uCPU ��ʃN���A�R�}���h (L4)
*
CLSL4_CMD		FCB		$02,$00,$00

	endif

*--[ FM77AV ]-----------------------------------------------------------------

	if	(MODEL>=M_FM77AV)

*
*	FM77AV������
*
FM77AV_INIT		LBSR	MMR_INIT
				LBSR	SUB_HALT
				LBSR	MMR_VRAM
* �L�[�G���R�[�_�ɑ΂��A�X�L�����R�[�h���w��
				LDA		#$00
				STA		KENCDAT
FM77AV_INIT1	LDA		KENCSTA
				LSRA
				BCC		FM77AV_INIT1
				LDA		#$02
				STA		KENCDAT
FM77AV_INIT2	LDA		KENCSTA
				LSRA
				BCC		FM77AV_INIT2
				LBSR	MMR_BASIC
				RTS

*
*	MMR������
*
*	USEREG: A,B,X
*
MMR_INIT		CLR		MMSR
				CLR		MSSW1
				CLR		MSSW2
				LDB		#3
MMR_INIT1		STB		MSGR
				LDX		#MMR+0
				LDA		#$30
MMR_INIT2		STA		,X+
				INCA
				CMPA	#$40
				BCS		MMR_INIT2
				DECB
				BPL		MMR_INIT1
* VRAM�p�Z�O�����g(SEGMENT 1)������
				LDB		#1
				STB		MSGR
				LDX		#MMR+8
				LDB		#4
				LDA		#$10
MMR_INIT3		STA		,X+
				INCA
				DECB
				BNE		MMR_INIT3
				LDA		#$1D
				STA		MMR+$D
*
				CLR		MSGR
				LDA		#$80
				STA		MMSR
				RTS

*
*	MMR�ݒ�(VRAM�A�N�Z�X)
*
*	USEREG: A
*
MMR_VRAM		STA		BANKSEL
				LDA		#1
				STA		MSGR
* �_�����ZON
				LDA		#$08
				STA		ALUDIS
				LDA		#$80
				STA		ALUCMD
				RTS

*
*	MMR�ݒ�(F-BASIC ROM)
*
MMR_BASIC		EQU		*
				CLR		MSGR
				TST		BANKSEL
				RTS

*
*	�X�y�[�X�҂�
*
KEY_SPACE		LDA		KEYDAT
* BREAK�R�[�h�����`�F�b�N
* ���̃\�[�X�ł͉����L�[�����󉟂����Ȃ��Ɠ��삵�Ȃ��悤�Ȃ̂�
				CMPA	#$57+$80
				BEQ		KEY_SPACE
				CMPA	#$58+$80
				BEQ		KEY_SPACE
				CMPA	#$35+$80
				BEQ		KEY_SPACE
* �������܂ő҂�
KEY_SPACE1		LDA		KEYDAT
				BMI		KEY_SPACE1
				CMPA	#$57
				BEQ		KEY_SPACE2
				CMPA	#$58
				BEQ		KEY_SPACE2
				CMPA	#$35
				BNE		KEY_SPACE1
* �������܂ő҂�
KEY_SPACE2		LDA		KEYDAT
				BPL		KEY_SPACE2
				RTS

	endif

*--[ FM-77L4 ]----------------------------------------------------------------

	if	(MODEL==M_FM7)

*
*	MMR������
*
*	USEREG: A,B,X
*
MMR_INIT		CLR		MMSR
				CLR		MSSW1
				CLR		MSSW2
				LDB		#3
MMR_INIT1		STB		MSGR
				LDX		#MMR+0
				LDA		#$30
MMR_INIT2		STA		,X+
				INCA
				CMPA	#$40
				BCS		MMR_INIT2
				DECB
				BPL		MMR_INIT1
*
				RTS

*
* 400���C�����[�h������
*
INITL4			LDA		SUBMODE
				BITA	#$10
				LBNE	INITL4_1
				LDA		#$FF
				STA		L4_FLAG
				LBSR	SUBCHG
* ��ʏ�����
				LEAX	SUBINIT_L4_CMD,PCR
				LDB		#$08
				LBSR	SUB_CMD
				LEAX	SUBCONS_L4_CMD,PCR
				LDB		#$03
				LBSR	SUB_CMD
* �p���b�g������
				LBSR	INITL2_2
				LDD		#$000F
				STD		$FD38
INITL4_1		RTS

*
* �T�u�V�X�e�����[�h�؂芷��
*
SUBCHG			BSR		SUBCHG3
				LDD		#$40E8
				STA		SUBSIN
				CLR		SUBSIN
SUBCHG1			LDA		SUBSIN
				BMI		SUBCHG1
				STB		SUBMODE
SUBCHG2			LDA		SUBSIN
				BPL		SUBCHG2
SUBCHG3			LDA		#$E0
				STA		SUBMODE
				LEAX	INITL4_CMD,PCR
				LDB		#INITL4_CMDe-INITL4_CMD
				LBRA	SUB_CMD

*
* 200���C�����[�h������
*
INITL2			LDA		SUBMODE
				BITA	#$10
				LBNE	INITL2_2
				LDD		#$00CC
				STA		L4_FLAG
				STB		SUBMODE
INITL2_1		LDA		SUBSIN
				BPL		INITL2_1
* ��ʏ�����
				LEAX	SUBINIT_CMD,PCR
				LDB		#$09
				LBSR	SUB_CMD
				LEAX	SUBCONS_CMD,PCR
				LDB		#$02
				LBSR	SUB_CMD
* �p���b�g������ (�C���e���V�e�B�r�b�g�͍l�����Ȃ�)
INITL2_2		LDA		#$07
				LDX		#$FD40
INITL2_3		STA		,-X
				DECA
				BPL		INITL2_3
INITL2_4		RTS

	endif

*--[ �����o��(FM77AV) ]-------------------------------------------------------

	if	(MODEL>=M_FM77AV)

*
*	������\��
*
*	PARAM.: X	������
*			U	VRAM�A�h���X
*
PUT_STRING		LBSR	MMR_VRAM
PUT_STRING1		LDA		,X+
				BEQ		PUT_STRING4
				CMPA	#$11
				BEQ		PUT_STRING2
				CMPA	#$12
				BEQ		PUT_STRING3
				LDB		,X+
				LBSR	PUT_KANJI
				LEAU	2,U
				BRA		PUT_STRING1
* �F�w��
PUT_STRING2		LDA		,X+
				STA		<COLOR
				BRA		PUT_STRING1
* ���W�w��(8dot�P��)
PUT_STRING3		LDA		1,X
				LDB		#$A0
				MUL
				LSLB
				ROLA
				LSLB
				ROLA
				ORB		,X
				LEAX	2,X
				TFR		D,U
				BRA		PUT_STRING1
* �I��
PUT_STRING4		LBSR	MMR_BASIC
				RTS

*
*	�����\��
*
*	PARAM.: D	SHIFT JIS
*			U	VRAM�A�h���X
*
PUT_KANJI		PSHS	U,X
				LBSR	SHIFT_TO_JIS
				LEAU	KANJI_BUF,PCR
				LBSR	GET_KANJI
				LEAX	KANJI_BUF,PCR
				LDU		2,S
				LEAU	$8000,U
* 16x12 �N���A
				CLR		ALUCOL
				CLR		ALUMASK
				LDB		#12
PUT_KANJI1		TST		,U
				TST		1,U
				LEAU	$50,U
				DECB
				BNE		PUT_KANJI1
				LEAU	-$3C0,U
* 16x12 �`��
				LDA		<COLOR
				STA		ALUCOL
				LDB		#12
PUT_KANJI2		LDA		,X+
				STA		ALUMASK
				TST		,U
				LDA		,X+
				STA		ALUMASK
				TST		1,U
				LEAU	$50,U
				DECB
				BNE		PUT_KANJI2
				PULS	X,U,PC

*
*	�����p�^�[���擾
*
*	PARAM.: D	JIS
*			U	�o�b�t�@(32�o�C�g)
*
GET_KANJI		PSHS	X,B,A
* RA0-RA4
				PSHS	B,A
				LSLB
				ROLA
				LSLB
				ROLA
				LSLB
				ROLA
				LSLB
				ROLA
				ANDA	#$01
				STD		,S
* RA5-RA7
				LDA		2,S
				LSLA
				ANDA	#$0E
				ORA		,S
				STA		,S
* RA8
				LDD		2,S
				ANDA	#$70
				ANDB	#$60
				CMPD	#$2060
				BEQ		GET_KANJI1
				LDA		2,S
				LSLA
				ANDA	#$10
				ORA		,S
				STA		,S
				BRA		GET_KANJI2
GET_KANJI1		LDA		#$10
				ORA		,S
				STA		,S
* RA9,RA10,RA11
GET_KANJI2		LDD		2,S
				LSRB
				LSRB
				LSRB
				LSRB
				LSRB
				ANDB	#$03
				PSHS	B
				LSRA
				LSRA
				ANDA	#$1C
				ORA		,S+
				LEAX	GET_KANJI_TBL,PCR
				CLRB
GET_KANJI3		CMPA	,X+
				BEQ		GET_KANJI4
				INCB
				CMPB	#$08
				BCS		GET_KANJI3
				CLRB
GET_KANJI4		TFR		B,A
				LSLA
				LSLA
				LSLA
				LSLA
				LSLA
				ORA		,S
				LDB		1,S
				TFR		D,X
				LEAS	4,S
* ����ROM�A�h���X=X
				LDA		#4
				PSHS	A
GET_KANJI5		STX		KNJ1ADR
				LDD		KNJ1DATA
				STD		,U
				LEAX	1,X
				STX		KNJ1ADR
				LDD		KNJ1DATA
				ORA		,U
				ORB		1,U
				COMA
				COMB
				STD		,U++
				LEAX	1,X
				STX		KNJ1ADR
				LDD		KNJ1DATA
				COMA
				COMB
				STD		,U++
				LEAX	1,X
				STX		KNJ1ADR
				LDD		KNJ1DATA
				COMA
				COMB
				STD		,U++
				LEAX	1,X
				DEC		,S
				BNE		GET_KANJI5
				PULS	A,X,PC

*
*	SHIFT JIS��JIS�ϊ�
*
*	PARAM.: D	SHIFT JIS
*	RESULT: D	JIS
*
SHIFT_TO_JIS	PSHS	A
				ADDA	,S+
				SUBB	#$1F
				BMI		SHIFT_TO_JIS1
				CMPB	#$61
				ADCB	#$DE
SHIFT_TO_JIS1	ADDD	#$1FA1
				ANDA	#$7F
				ANDB	#$7F
				RTS

*
* ��ʃN���A
*
* USEREG: A,B,X,Y,U
*
CLS				LBSR	MMR_VRAM
				CLR		ALUCOL
				CLR		ALUMASK
				LDU		#$8000
				LDB		#200
				PSHS	B
CLS1			PULU	A,B,X,Y
				PULU	A,B,X,Y
				PULU	A,B,X,Y
				PULU	A,B,X,Y
				PULU	A,B,X,Y
				PULU	A,B,X,Y
				PULU	A,B,X,Y
				PULU	A,B,X,Y
				PULU	A,B,X,Y
				PULU	A,B,X,Y
				PULU	A,B,X,Y
				PULU	A,B,X,Y
				PULU	A,B,X,Y
				PULU	A,B
				DEC		,S
				BNE		CLS1
				PULS	B
				LBRA	MMR_BASIC
	endif

*--[ �T�uCPU ]----------------------------------------------------------------

*
*	�T�uCPU HALT
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
*	�T�uCPU �R�}���h�o��
*
*	PARAM.: X	�R�}���h�f�[�^($FC82�`)
*			B	���M����
*	USEREG: A,B,X,U
*
SUB_CMD			LBSR	SUB_HALT
* �f�[�^��������
				LDU		#SHARED
				CLR		,U+
				CLR		,U+
SUB_CMD1		LDA		,X+
				STA		,U+
				DECB
				BNE		SUB_CMD1
* �T�uCPU ���s
				CLR		SUBSIN
				RTS

	if	(MODEL==M_FM7)

*
*	������o�� (FM-7�p,ANK��p)
*
*	PARAM.: X	������
*	USEREG: A,B,X,U
*
PUT_STRING		EQU		*
PRINT			TFR		X,U
* �������𐔂���
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

	else

*
*	������o�� (FM77AV�p)
*
*	PARAM.: X	������
*	USEREG: A,B,X,U
*
PRINT			TFR	X,U
* �������𐔂���
				CLRB
PRINT1			TST		,U+
				BEQ		PRINT2
				INCB
				BRA		PRINT1
* �o�b�t�@�ɃZ�b�g
PRINT2			LEAU	BUFFER,PCR
				LDA		#$03
				STA		,U+
				STB		,U+
PRINT3			LDA		,X+
				STA		,U+
				DECB
				BNE		PRINT3
* ���s
				LEAX	BUFFER,PCR
				LDB		1,X
				INCB
				INCB
				LBRA	SUB_CMD

	endif

*--[ RS-232C�h���C�o ]--------------------------------------------------------

	if	defined(RSDRIVER)
	include	rsdriver.asm
	endif

*--[ DOS ]--------------------------------------------------------------------

	include	diskdrv.asm

*--[ DATA ]-------------------------------------------------------------------

	if	(MODEL>=M_FM77AV)

*
*	�T�uCPU �R�}���h (FM77AV)
*
SUBINIT_CMD		FCB		$01,$00,40,25,0,25,0,1,0
SUBCONS_CMD		FCB		$0C,$06
INKEY_CMD		FCB		$29,$01

*
*	����ROM�ϊ��e�[�u��
*
GET_KANJI_TBL	FCB		$09,$0A,$0D,$0E
				FCB		$0F,$11,$12,$13

	else

*
*	�T�uCPU �R�}���h (FM-7)
*
SUBINIT_CMD		FCB		$01,$00,40,25,0,25,0,1,0
SUBCONS_CMD		FCB		$0C,$06
INKEY_CMD		FCB		$29,$03
INKEY_CHK_CMD	FCB		$29,$00

*
*	�T�uCPU �R�}���h (FM-77L4)
*
SUBINIT_L4_CMD	FCB		$01,$07,40,25,0,25,0,1
SUBCONS_L4_CMD	FCB		$0C,$0E,$D6
INITL4_CMD		FCB		$3F,$59,$41,$4D,$41,$55,$43,$48,$49
				FCB		$93,$D3,$8F,$90
				STA		$D408
				LDA		SUBMODEL4
				ANDA	#$FD
				STA		SUBMODEL4
				RTS
INITL4_CMDe		EQU		*

	endif

*
*	�t�@�C���l�[��
*

	if	(MODEL!=M_FM77AVDIC)

FBASIC_FN		FCC		/FBASIC30ROM/,0
BOOTBAS_FN		FCC		/BOOT_BASROM/,0
BOOTDOS_FN		FCC		/BOOT_DOSROM/,0
SUBSYSC_FN		FCC		/SUBSYS_CROM/,0
KANJI_FN		FCC		/KANJI   ROM/,0
KANJI1_FN		FCC		/KANJI1  ROM/,0

	endif

	if	(MODEL==M_FM77AV||MODEL==M_FM77AV40)

INITIATE_FN		FCC		/INITIATEROM/,0
SUBSYSA_FN		FCC		/SUBSYS_AROM/,0
SUBSYSB_FN		FCC		/SUBSYS_BROM/,0
SUBSYSCG_FN		FCC		/SUBSYSCGROM/,0
KANJI2_FN		FCC		/KANJI2  ROM/,0

	endif

	if	(MODEL==M_FM77AV40||MODEL==M_FM77AVDIC)

DICROM_FN		FCC		/DICROM  ROM/,0

	endif

	if	(MODEL==M_FM77AV40)

EXTSUB_FN		FCC		/EXTSUB  ROM/,0
BOOTMMR_FN		FCC		/BOOT_MMRROM/,0

	endif

	if	(MODEL==M_FM7)

BOOT1MB_FN		FCC		/BOOT_1MBROM/,0
BOOTMMR_FN		FCC		/BOOT_MMRROM/,0
SUBSYSL4_FN		FCC		/SUBSYSL4ROM/,0

	endif

*
*	�\�����b�Z�[�W(�N����)
*
	if (MODEL==M_FM77AV||MODEL==M_FM77AVDIC)&&(DISKTYPE!=D_NORM2DD)
FM77AV_MSG		FCB		$12,12,11
				FCC		"FM77AV SERIES ONLY!",0
	elsif	((MODEL==M_FM77AV40)||(DISKTYPE==D_NORM2DD))
FM77AV_MSG		FCB		$12,$0A,$0B
				FCC		"FM77AV20/40/EX/SX ONLY!",0
	endif

	if	(MODEL>=M_FM77AV)&&(defined(FBLOADM)||(DISKTYPE==D_NORM2D))
TITLE_MSG		FCB		$11,$07,$12,$17,$06
				FCC		/�w�l�V�@�q�n�l�Z�[�u���[�e�B���e�B/
				FCB		$12,$1C,$08
				FCC		/�i�b�j�@�P�X�X�X�@�o�h�D/
				FCB		$12,$14,$0a
				FCC		/�i�b�j�@�Q�O�O�R�`�Q�O�P�V�@�������݂肤/,0
TITLE_MSG2		FCC		$12,$12,$10
	endif
	if	(MODEL>=M_FM77AV)&&defined(FBLOADM)&&(DISKTYPE==D_NORM2DD)
				FCC		/�h���C�u�O�Ƀu�����N�Q�c�c�f�B�X�N���Z�b�g����/,0
	elsif	(MODEL>=M_FM77AV)&&defined(FBLOADM)&&(DISKTYPE!=D_NORM2DD)
				FCC		/�h���C�u�O�Ƀu�����N�Q�c�f�B�X�N���Z�b�g����/,0
	endif
	if	(MODEL>=M_FM77AV)&&(defined(FBLOADM)||(DISKTYPE!=D_NORM2DD))
FORMAT_MSG		FCB		$11,$07,$12,$1D,$09
				FCC		/�t�H�[�}�b�g���Ă��܂�/,0
	elsif	(MODEL>=M_FM77AV)
FORMAT_MSG		FCB		$11,$07,$12,$1B,$09
				FCC		/�_���t�H�[�}�b�g���Ă��܂�/,0
	elsif	(MODEL==M_FM7)&&(defined(FBLOADM)||(DISKTYPE==D_NORM2D))
TITLE_MSG		FCB		$11,$07,$12,$06,$06
				FCC		/XM7 ROM SAVE UTILITY for FM-7/
				FCB		$12,$02,$08
				FCC		"(C) 1999 PI. / 2003-2017 Ryu Takegami",0
TITLE_MSG2		FCB		$11,$04,$12,$06,$10
				FCC		/Set blank disk to drive 0 and/,0
FORMAT_MSG		FCB		$11,$07,$12,$0A,$09
				FCC		/Disk formatting .../,0
	elsif	(MODEL==M_FM7)
FORMAT_MSG		FCB		$11,$07,$12,$0A,$09
				FCC		/Disk formatting .../,0
	endif

*
* �\�����b�Z�[�W(ROM�Z�[�u��)
*
	if	(MODEL==M_FM77AV||MODEL==M_FM77AV40)
FBASIC_MSG		FCB		$11,$07,$12,$13,$09
				FCC		/�e�|�a�`�r�h�b�@�q�n�l�@���Z�[�u���Ă��܂�/,0
BOOT_MSG		FCB		$11,$07,$12,$18,$09
				FCC		/�u�[�g�q�n�l�@���Z�[�u���Ă��܂�/,0
	elsif (MODEL==M_FM7)
FBASIC_MSG		FCB		$11,$07,$12,$09,$09
				FCC		/Saving F-BASIC ROM .../,0
BOOTBAS_MSG		FCB		$11,$07,$12,$05,$09
				FCC		/Saving BOOT ROM(BASIC mode) .../,0
BOOTDOS_MSG		FCB		$11,$07,$12,$06,$09
				FCC		/Saving BOOT ROM(DOS mode) .../,0
BOOT1MB_MSG		FCB		$11,$07,$12,$06,$09
				FCC		/Saving BOOT ROM(1MB mode) .../,0
BOOTMMR_MSG		FCB		$11,$07,$12,$07,$09
				FCC		/Saving BOOT ROM(Hidden) .../,0
KANJI_MSG		FCB		$11,$07,$12,$07,$09
				FCC		/Saving Kanji ROM (JIS78) .../,0
KANJI1_MSG		FCB		$11,$07,$12,$07,$09
				FCC		/Saving Kanji ROM (JIS83) .../,0
BOOTB_DIPSW_MSG	FCB		$11,$07,$12,$02,$09
				FCC		/Please turn ON DIP SW 1 and DIP SW 2./,0
BOOTD_DIPSW_MSG	FCB		$11,$07,$12,$07,$09
				FCC		/Please turn ON DIP SW 1 and/
				FCB		$12,$0b,$0b
				FCC		/turn OFF DIP SW 2./,0
BOOTB_DIP77_MSG	FCB		$11,$07,$12,$08,$09
				FCC		/Please push BASIC switch./,0
BOOTD_DIP77_MSG	FCB		$11,$07,$12,$08,$09
				FCC		/Please push BOOT2 switch./,0
BOOTS_DIP77_MSG	FCB		$11,$07,$12,$08,$09
				FCC		/Please push BOOT1 switch./,0
	endif
	if	(MODEL==M_FM77AV||MODEL==M_FM77AV40)
KANJI1_MSG		FCB		$11,$07,$12,$0F,$09
				FCC		/��P���������q�n�l�i�i�h�r�W�R�j�@���Z�[�u���Ă��܂�/,0
KANJI2_MSG		FCB		$11,$07,$12,$0F,$09
				FCC		/��Q���������q�n�l�i�i�h�r�W�R�j�@���Z�[�u���Ă��܂�/,0
	endif
	if	(MODEL==M_FM77AV||MODEL==M_FM77AV40)
KANJI_MSG		FCB		$11,$07,$12,$0F,$09
				FCC		/��P���������q�n�l�i�i�h�r�V�W�j�@���Z�[�u���Ă��܂�/,0
	endif

	if	(MODEL==M_FM77AV||MODEL==M_FM77AV40)
INITIATE_MSG	FCB		$11,$07,$12,$15,$09
				FCC		/�C�j�V�G�[�^�q�n�l�@���Z�[�u���Ă��܂�/,0
SUBSYSA_MSG		FCB		$11,$07,$12,$12,$09
				FCC		/�T�u�V�X�e���q�n�l�i�`�j�@���Z�[�u���Ă��܂�/,0
SUBSYSB_MSG		FCB		$11,$07,$12,$12,$09
				FCC		/�T�u�V�X�e���q�n�l�i�a�j�@���Z�[�u���Ă��܂�/,0
SUBSYSC_MSG		FCB		$11,$07,$12,$12,$09
				FCC		/�T�u�V�X�e���q�n�l�i�b�j�@���Z�[�u���Ă��܂�/,0
SUBSYSCG_MSG	FCB		$11,$07,$12,$11,$09
				FCC		/�T�u�V�X�e���q�n�l�i�b�f�j�@���Z�[�u���Ă��܂�/,0
	elsif (MODEL==M_FM7)
SUBSYSC_MSG		FCB		$11,$07,$12,$02,$09
				FCC		/Saving Sub monitor ROM (200Line) .../,0
SUBSYSL4_MSG	FCB		$11,$07,$12,$02,$09
				FCC		/Saving Sub monitor ROM (400Line) .../,0
	endif
	if	(MODEL==M_FM77AV40||MODEL==M_FM77AVDIC)
DICROM_MSG		FCB		$11,$07,$12,$19,$09
				FCC		/�����q�n�l�@���Z�[�u���Ă��܂�/,0
	endif
	if	(MODEL==M_FM77AVDIC)
DICROM_EMSG		FCB		$11,$02,$12,$18,$09
				FCC		/���{��J�[�h����������Ă��܂���/,0
	endif
	if	(MODEL==M_FM77AV40)
EXTSUB_MSG		FCB		$11,$07,$12,$19,$09
				FCC		/�g���q�n�l�@���Z�[�u���Ă��܂�/,0
BOOTMMR_MSG		FCB		$11,$07,$12,$16,$09
				FCC		/�B���u�[�g�q�n�l�@���Z�[�u���Ă��܂�/,0
	endif


*
* �\�����b�Z�[�W(�G���[)
*
	if	(MODEL>=M_FM77AV)
NOTREADY_MSG	FCB		$11,$02,$12,$19,$10
				FCC		/�f�B�X�N���Z�b�g����Ă��܂���/,0
WRPROTECT_MSG	FCB		$11,$02,$12,$1A,$10
				FCC		/���C�g�v���e�N�g����Ă��܂�/,0
SEEKERR_MSG		FCB		$11,$02,$12,$20,$10
				FCC		/�V�[�N�G���[�ł�/,0
READERR_MSG		FCB		$11,$02,$12,$1F,$10
				FCC		/�ǂݍ��݃G���[�ł�/,0
WRITEERR_MSG	FCB		$11,$02,$12,$1F,$10
				FCC		/�������݃G���[�ł�/,0
SPACE_MSG		FCB		$11,$04,$12,$1A,$12
				FCC		/�X�y�[�X/
				FCB		$11,$07
				FCC		/�L�[�������Ă�������/,0
CLEAR_MSG		FCB		$11,$07,$12,$19,$10
				FCC		/�@�@�@�@�@�@�@�@�@�@�@�@�@�@�@/
				FCB		$12,$1A,$12
				FCC		/�@�@�@�@�@�@�@�@�@�@�@�@�@�@/,0
COMPLETE_MSG	FCB		$11,$07,$12,$22,$09
				FCC		/�I�����܂���/,0

	else

NOTREADY_MSG	FCB		$11,$02,$12,$0C,$10
				FCC		/Drive not ready./,0
WRPROTECT_MSG	FCB		$11,$02,$12,$0A,$10
				FCC		/Disk write protected./,0
SEEKERR_MSG		FCB		$11,$02,$12,$0F,$10
				FCC		/Seek error./,0
READERR_MSG		FCB		$11,$02,$12,$0F,$10
				FCC		/Read error./,0
WRITEERR_MSG	FCB		$11,$02,$12,$0F,$10
				FCC		/Write error./,0
SPACE_MSG		FCB		$11,$04,$12,$0D,$12
				FCC		/Push SPACE key./,0
CLEAR_MSG		FCB		$11,$07,$12,$0A,$10
				FCC		/                     /
				FCB		$12,$0D,$12
				FCC		/               /,0
COMPLETE_MSG	FCB		$11,$07,$12,$0F,$09
				FCC		/Completed./,0

	endif

*
* �v���O�����I��
*
PRGEND			EQU		*
				END