/*
 *	FM-7 EMULATOR "XM7"
 *
 *	Copyright (C) 1999-2017 ＰＩ．(Twitter:@xm6_original)
 *	Copyright (C) 2001-2017 Ryu Takegami (Twitter:@RyuTakegami)
 *
 *	[ キーボードエンコーダ隠しメッセージ ]
 */

static const BYTE key_hidden_message[] = {
	0xDC, 0xC0, 0xBC, 0xCA, 0x46, 0x4D, 0x2D, 0x37, 
	0x37, 0x41, 0x56, 0xB7, 0xB0, 0xCE, 0xDE, 0xB0, 
	0xC4, 0xDE, 0xB4, 0xDD, 0xBA, 0xB0, 0xC0, 0xDE, 
	0xC3, 0xDE, 0xBD, 0xA1, 0xB6, 0xDE, 0xB2, 0xD8, 
	0xAC, 0xB8, 0xBE, 0xAF, 0xB9, 0xB2, 0x3D, 0xD6, 
	0xBA, 0xD4, 0xCF, 0xA4, 0xB4, 0xDD, 0xBA, 0xB0, 
	0xC4, 0xDE, 0xCC, 0xDE, 0x3D, 0xC0, 0xB6, 0xB2, 
	0xB9, 0xA4, 0x53, 0x55, 0x42, 0xB2, 0xDD, 0xC0, 
	0xB0, 0xCC, 0xAA, 0xB0, 0xBD, 0xCC, 0xDE, 0x3D, 
	0xB6, 0xD8, 0xD4, 0xA4, 0xC3, 0xBD, 0xC4, 0xCC, 
	0xDF, 0xDB, 0xBB, 0xB8, 0xBE, 0xB2, 0x3D, 0xB1, 
	0xC0, 0xB8, 0xDE, 0xC1, 0xA4, 0xBA, 0xDE, 0xB2, 
	0xB9, 0xDD, 0xCA, 0xDE, 0xDD, 0x3D, 0xB2, 0xCF, 
	0xD1, 0xD7, 0xA5, 0xB5, 0xB6, 0xA5, 0xA5, 0xA5, 
	0xB2, 0xBC, 0xDE, 0xAE, 0xB3, 0xC9, 0xD2, 0xDD, 
	0xCA, 0xDE, 0xB0, 0xC3, 0xDE, 0xBB, 0xB8, 0xBE, 
	0xB2, 0xBB, 0xDA, 0xCF, 0xBC, 0xC0, 0xA1, 0xC5, 
	0xB5, 0xA4, 0xBA, 0xC9, 0xB6, 0xB8, 0xDA, 0xD2, 
	0xAF, 0xBE, 0xB0, 0xBC, 0xDE, 0xCA, 0xA4, 0xC0, 
	0xB6, 0xB2, 0xB9, 0xB6, 0xDE, 0x31, 0x39, 0x38, 
	0x35, 0xC8, 0xDD, 0x38, 0xB6, 0xDE, 0xC2, 0x32, 
	0x32, 0xC6, 0xC1, 0xC6, 0xBC, 0xB6, 0xB9, 0xC0, 
	0xD3, 0xC9, 0xC3, 0xDE, 0xBD, 0xA1, 0xBC, 0xDE, 
	0xC2, 0xCA, 0xA4, 0xCE, 0xB6, 0xC6, 0xD3, 0xB6, 
	0xB8, 0xDA, 0xB7, 0xC9, 0xB3, 0xB6, 0xDE, 0xC0, 
	0xB8, 0xBB, 0xDD, 0xB1, 0xD9, 0xC9, 0xC3, 0xDE, 
	0xBD, 0xB6, 0xDE, 0xA4, 0xD0, 0xC2, 0xB9, 0xC3, 
	0xD3, 0xC0, 0xDE, 0xDA, 0xC6, 0xD3, 0xB5, 0xBC, 
	0xB4, 0xC5, 0xB2, 0xC3, 0xDE, 0xB8, 0xC0, 0xDE, 
	0xBB, 0xB2, 0xC8, 0xA1, 0xBF, 0xDA, 0xC3, 0xDE, 
	0xCA, 0xBA, 0xDE, 0xB7, 0xB9, 0xDE, 0xDD, 0xD6, 
	0xB3, 0x21, 0x21, 0x00, 
};
