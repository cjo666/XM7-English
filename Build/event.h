/*
 *	FM-7 EMULATOR "XM7"
 *
 *	Copyright (C) 1999-2017 �o�h�D(Twitter:@xm6_original)
 *	Copyright (C) 2001-2017 Ryu Takegami (Twitter:@RyuTakegami)
 *
 *	[ �C�x���gID ]
 */

#ifndef _event_h_
#define _event_h_

/*
 *	�C�x���g��`
 */
#define EVENT_MAINTIMER			0		/* ���C��CPU 2.03ms�^�C�} */
#define EVENT_SUBTIMER			1		/* �T�uCPU 20ms�^�C�} */
#define EVENT_OPN_A				2		/* OPN �^�C�}A */
#define EVENT_OPN_B				3		/* OPN �^�C�}B */
#define EVENT_KEYBOARD			4		/* �L�[�{�[�h ���s�[�g�^�C�} */
#define EVENT_BEEP				5		/* BEEP�� �P���^�C�} */
#define EVENT_VSYNC				6		/* VSYNC */
#define EVENT_BLANK				7		/* BLANK */
#define EVENT_LINE				8		/* �������LSI(V2/V3) */
#define	EVENT_TEXT_BLINK		8		/* �e�L�X�g�u�����L���O(V1) */
#define EVENT_RTC				9		/* ���v 1sec(V2/V3) */
#define	EVENT_SUB_RESET			9		/* ���[�h�؂芷���T�u���Z�b�g(V1) */
#define EVENT_WHG_A				10		/* WHG �^�C�}A */
#define EVENT_WHG_B				11		/* WHG �^�C�}B */
#define EVENT_THG_A				12		/* THG �^�C�}A */
#define EVENT_THG_B				13		/* THG �^�C�}B */
#define EVENT_FDC_M				14		/* FDC �}���`�Z�N�^ */
#define EVENT_FDC_L				15		/* FDC ���X�g�f�[�^ */
#define	EVENT_FDD_SEEK			16		/* FDD �V�[�N����E�F�C�g */
#define	EVENT_TAPEMON			17		/* �e�[�v�����T���v�����O�^�C�} */
#define	EVENT_MOUSE				18		/* �}�E�X�f�[�^���X�g */
#define	EVENT_FDC_DMA			19		/* FDC DMA�]���J�n(V3) */
#define	EVENT_KEYENC_ACK		20		/* �L�[�G���R�[�_ ACK���M */
#define	EVENT_RS_TXTIMING		21		/* RS-232C ���M�^�C�~���O */
#define	EVENT_RS_RXTIMING		22		/* RS-232C ��M�^�C�~���O */
#define	EVENT_KEYENC_BEEP		23		/* �L�[�G���R�[�_BEEP �P���^�C�} */
#define	EVENT_KEYENC_MESSAGE	24		/* �L�[�G���R�[�_ �B�����b�Z�[�W */
#define	EVENT_PTM_TIMER			25		/* PTM�^�C�} */
#define	EVENT_BMC_LOST			26		/* �o�u�������� ���X�g�f�[�^ */
#define	EVENT_BMC_MULTI			27		/* �o�u�������� �}���`�y�[�W */


#endif	/* _event_h_ */
