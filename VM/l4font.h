/*
 *	FM-7 EMULATOR "XM7"
 *
 *	Copyright (C) 1999-2017 �o�h�D(Twitter:@xm6_original)
 *	Copyright (C) 2001-2017 Ryu Takegami (Twitter:@RyuTakegami)
 *
 *	[ 400���C�����[�h�p�����t�H���g(designed by Ryu Takegami) ]
 */

static const BYTE subcg_internal[] = {
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 	// 00
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x78, 0x80, 0x80, 0x70, 0x08, 0x08, 0xf0, 0x00, 	// 01
	0x22, 0x22, 0x22, 0x3e, 0x22, 0x22, 0x22, 0x00, 
	0x78, 0x80, 0x80, 0x70, 0x08, 0x08, 0xf0, 0x00, 	// 02
	0x22, 0x22, 0x14, 0x08, 0x14, 0x22, 0x22, 0x00, 
	0xf8, 0x80, 0x80, 0xf0, 0x80, 0x80, 0xf8, 0x00, 	// 03
	0x22, 0x22, 0x14, 0x08, 0x14, 0x22, 0x22, 0x00, 
	0xf8, 0x80, 0x80, 0xf0, 0x80, 0x80, 0xf8, 0x00, 	// 04
	0x3e, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x00, 
	0xf8, 0x80, 0x80, 0xf0, 0x80, 0x80, 0xf8, 0x00, 	// 05
	0x1c, 0x22, 0x22, 0x22, 0x2a, 0x24, 0x1a, 0x00, 
	0x20, 0x20, 0x50, 0x50, 0xf8, 0x88, 0x88, 0x00, 	// 06
	0x22, 0x24, 0x28, 0x30, 0x28, 0x24, 0x22, 0x00, 
	0xf0, 0x88, 0x88, 0xf0, 0x88, 0x88, 0xf0, 0x00, 	// 07
	0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x3e, 0x00, 
	0xf0, 0x88, 0x88, 0xf0, 0x88, 0x88, 0xf0, 0x00, 	// 08
	0x1e, 0x20, 0x20, 0x1c, 0x02, 0x02, 0x3c, 0x00, 
	0x88, 0x88, 0x88, 0xf8, 0x88, 0x88, 0x88, 0x00, 	// 09
	0x3e, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x00, 
	0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0xf8, 0x00, 	// 0A
	0x3e, 0x20, 0x20, 0x3c, 0x20, 0x20, 0x20, 0x00, 
	0x88, 0x88, 0x88, 0xf8, 0x88, 0x88, 0x88, 0x00, 	// 0B
	0x22, 0x36, 0x2a, 0x22, 0x22, 0x22, 0x22, 0x00, 
	0x70, 0x88, 0x80, 0x80, 0x80, 0x88, 0x70, 0x00, 	// 0C
	0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x3e, 0x00, 
	0x70, 0x88, 0x80, 0x80, 0x80, 0x88, 0x70, 0x00, 	// 0D
	0x3c, 0x22, 0x22, 0x3c, 0x28, 0x24, 0x22, 0x00, 
	0x78, 0x80, 0x80, 0x70, 0x08, 0x08, 0xf0, 0x00, 	// 0E
	0x1c, 0x22, 0x22, 0x22, 0x22, 0x22, 0x1c, 0x00, 
	0x78, 0x80, 0x80, 0x70, 0x08, 0x08, 0xf0, 0x00, 	// 0F
	0x1c, 0x08, 0x08, 0x08, 0x08, 0x08, 0x1c, 0x00, 
	0xf0, 0x88, 0x88, 0x88, 0x88, 0x88, 0xf0, 0x00, 	// 10
	0x3e, 0x20, 0x20, 0x3c, 0x20, 0x20, 0x3e, 0x00, 
	0xf0, 0x88, 0x88, 0x88, 0x88, 0x88, 0xf0, 0x00, 	// 11
	0x04, 0x0c, 0x04, 0x04, 0x04, 0x04, 0x04, 0x00, 
	0xf0, 0x88, 0x88, 0x88, 0x88, 0x88, 0xf0, 0x00, 	// 12
	0x1c, 0x22, 0x02, 0x04, 0x08, 0x10, 0x3e, 0x00, 
	0xf0, 0x88, 0x88, 0x88, 0x88, 0x88, 0xf0, 0x00, 	// 13
	0x1c, 0x22, 0x02, 0x1c, 0x02, 0x22, 0x1c, 0x00, 
	0xf0, 0x88, 0x88, 0x88, 0x88, 0x88, 0xf0, 0x00, 	// 14
	0x04, 0x0c, 0x14, 0x14, 0x3e, 0x04, 0x04, 0x00, 
	0x88, 0x88, 0xc8, 0xa8, 0x98, 0x88, 0x88, 0x00, 	// 15
	0x22, 0x24, 0x28, 0x30, 0x28, 0x24, 0x22, 0x00, 
	0x78, 0x80, 0x80, 0x70, 0x08, 0x08, 0xf0, 0x00, 	// 16
	0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x3e, 0x00, 
	0xf8, 0x80, 0x80, 0xf0, 0x80, 0x80, 0xf8, 0x00, 	// 17
	0x3c, 0x22, 0x22, 0x3c, 0x22, 0x22, 0x3c, 0x00, 
	0x70, 0x88, 0x80, 0x80, 0x80, 0x88, 0x70, 0x00, 	// 18
	0x22, 0x22, 0x32, 0x2a, 0x26, 0x22, 0x22, 0x00, 
	0xf8, 0x80, 0x80, 0xf0, 0x80, 0x80, 0xf8, 0x00, 	// 19
	0x22, 0x36, 0x2a, 0x22, 0x22, 0x22, 0x22, 0x00, 
	0x78, 0x80, 0x80, 0x70, 0x08, 0x08, 0xf0, 0x00, 	// 1A
	0x3c, 0x22, 0x22, 0x3c, 0x22, 0x22, 0x3c, 0x00, 
	0xf8, 0x80, 0x80, 0xf0, 0x80, 0x80, 0xf8, 0x00, 	// 1B
	0x1c, 0x22, 0x20, 0x20, 0x20, 0x22, 0x1c, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 0x04, 	// 1C
	0xfe, 0x04, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x20, 0x40, 	// 1D
	0xfe, 0x40, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x10, 0x38, 0x54, 0x10, 0x10, 0x10, 	// 1E
	0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x00, 0x00, 
	0x00, 0x00, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 	// 1F
	0x10, 0x10, 0x10, 0x54, 0x38, 0x10, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 	// 20
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 	// 21
	0x10, 0x00, 0x00, 0x00, 0x10, 0x10, 0x00, 0x00, 
	0x00, 0x6c, 0x6c, 0x24, 0x48, 0x00, 0x00, 0x00, 	// 22
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x04, 0x24, 0x26, 0x3c, 0x64, 0x24, 0x24, 	// 23
	0x26, 0x3c, 0x64, 0x24, 0x24, 0x20, 0x00, 0x00, 
	0x00, 0x10, 0x10, 0x7c, 0x92, 0x90, 0x50, 0x38, 	// 24
	0x14, 0x12, 0x92, 0x7c, 0x10, 0x10, 0x00, 0x00, 
	0x00, 0x00, 0x40, 0xa0, 0xa2, 0x44, 0x08, 0x10, 	// 25
	0x20, 0x44, 0x8a, 0x0a, 0x04, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x70, 0x88, 0x80, 0x80, 0x80, 0x40, 	// 26
	0x60, 0x90, 0x8a, 0x84, 0x8c, 0x72, 0x00, 0x00, 
	0x00, 0x18, 0x18, 0x08, 0x10, 0x00, 0x00, 0x00, 	// 27
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x02, 0x04, 0x08, 0x08, 0x10, 0x10, 0x10, 	// 28
	0x10, 0x10, 0x08, 0x08, 0x04, 0x02, 0x00, 0x00, 
	0x00, 0x80, 0x40, 0x20, 0x20, 0x10, 0x10, 0x10, 	// 29
	0x10, 0x10, 0x20, 0x20, 0x40, 0x80, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x10, 0x92, 0x54, 0x38, 	// 2A
	0x54, 0x92, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x10, 0x10, 0x10, 0xfe, 	// 2B
	0x10, 0x10, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 	// 2C
	0x00, 0x00, 0x00, 0x60, 0x60, 0x20, 0x40, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xfe, 	// 2D
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 	// 2E
	0x00, 0x00, 0x00, 0x60, 0x60, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x02, 0x04, 0x08, 0x10, 	// 2F
	0x20, 0x40, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x7c, 0x82, 0x82, 0x86, 0x8a, 0x8a, 0x92, 	// 30
	0xa2, 0xa2, 0xc2, 0x82, 0x82, 0x7c, 0x00, 0x00, 
	0x00, 0x10, 0x30, 0x50, 0x10, 0x10, 0x10, 0x10, 	// 31
	0x10, 0x10, 0x10, 0x10, 0x10, 0x7c, 0x00, 0x00, 
	0x00, 0x38, 0x44, 0x82, 0x82, 0x02, 0x04, 0x08, 	// 32
	0x10, 0x20, 0x40, 0x80, 0x80, 0xfe, 0x00, 0x00, 
	0x00, 0x38, 0x44, 0x82, 0x82, 0x02, 0x04, 0x38, 	// 33
	0x04, 0x02, 0x82, 0x82, 0x44, 0x38, 0x00, 0x00, 
	0x00, 0x04, 0x0c, 0x0c, 0x14, 0x14, 0x24, 0x24, 	// 34
	0x44, 0x44, 0xfe, 0x04, 0x04, 0x04, 0x00, 0x00, 
	0x00, 0xfe, 0x80, 0x80, 0x80, 0xb8, 0xc4, 0x82, 	// 35
	0x02, 0x02, 0x02, 0x82, 0x44, 0x38, 0x00, 0x00, 
	0x00, 0x38, 0x44, 0x82, 0x80, 0x80, 0xb8, 0xc4, 	// 36
	0x82, 0x82, 0x82, 0x82, 0x44, 0x38, 0x00, 0x00, 
	0x00, 0xfe, 0x82, 0x82, 0x04, 0x04, 0x08, 0x08, 	// 37
	0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x00, 0x00, 
	0x00, 0x38, 0x44, 0x82, 0x82, 0x82, 0x44, 0x38, 	// 38
	0x44, 0x82, 0x82, 0x82, 0x44, 0x38, 0x00, 0x00, 
	0x00, 0x38, 0x44, 0x82, 0x82, 0x82, 0x82, 0x46, 	// 39
	0x3a, 0x02, 0x02, 0x82, 0x44, 0x38, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x18, 0x18, 0x00, 0x00, 	// 3A
	0x00, 0x00, 0x18, 0x18, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x18, 0x18, 0x00, 0x00, 	// 3B
	0x00, 0x00, 0x18, 0x18, 0x08, 0x10, 0x00, 0x00, 
	0x00, 0x00, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 	// 3C
	0x20, 0x10, 0x08, 0x04, 0x02, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x7e, 0x00, 0x00, 	// 3D
	0x00, 0x7e, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 	// 3E
	0x04, 0x08, 0x10, 0x20, 0x40, 0x00, 0x00, 0x00, 
	0x00, 0x38, 0x44, 0x82, 0x82, 0x82, 0x44, 0x08, 	// 3F
	0x10, 0x10, 0x00, 0x00, 0x10, 0x10, 0x00, 0x00, 
	0x00, 0x38, 0x44, 0x82, 0x92, 0xaa, 0xaa, 0xaa, 	// 40
	0xaa, 0xaa, 0x9c, 0x80, 0x42, 0x3c, 0x00, 0x00, 
	0x00, 0x10, 0x10, 0x28, 0x28, 0x28, 0x44, 0x44, 	// 41
	0x44, 0x7c, 0x82, 0x82, 0x82, 0x82, 0x00, 0x00, 
	0x00, 0xf8, 0x44, 0x42, 0x42, 0x42, 0x44, 0x78, 	// 42
	0x44, 0x42, 0x42, 0x42, 0x44, 0xf8, 0x00, 0x00, 
	0x00, 0x38, 0x44, 0x82, 0x82, 0x80, 0x80, 0x80, 	// 43
	0x80, 0x80, 0x82, 0x82, 0x44, 0x38, 0x00, 0x00, 
	0x00, 0xf8, 0x44, 0x42, 0x42, 0x42, 0x42, 0x42, 	// 44
	0x42, 0x42, 0x42, 0x42, 0x44, 0xf8, 0x00, 0x00, 
	0x00, 0xfe, 0x80, 0x80, 0x80, 0x80, 0x80, 0xfc, 	// 45
	0x80, 0x80, 0x80, 0x80, 0x80, 0xfe, 0x00, 0x00, 
	0x00, 0xfe, 0x80, 0x80, 0x80, 0x80, 0x80, 0xfc, 	// 46
	0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x00, 0x00, 
	0x00, 0x38, 0x44, 0x82, 0x82, 0x80, 0x80, 0x9e, 	// 47
	0x82, 0x82, 0x82, 0x82, 0x46, 0x3a, 0x00, 0x00, 
	0x00, 0x82, 0x82, 0x82, 0x82, 0x82, 0x82, 0xfe, 	// 48
	0x82, 0x82, 0x82, 0x82, 0x82, 0x82, 0x00, 0x00, 
	0x00, 0x38, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 	// 49
	0x10, 0x10, 0x10, 0x10, 0x10, 0x38, 0x00, 0x00, 
	0x00, 0x0e, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 	// 4A
	0x04, 0x04, 0x04, 0x84, 0x88, 0x70, 0x00, 0x00, 
	0x00, 0x82, 0x82, 0x84, 0x88, 0x88, 0x90, 0xe0, 	// 4B
	0x90, 0x88, 0x88, 0x84, 0x82, 0x82, 0x00, 0x00, 
	0x00, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 	// 4C
	0x80, 0x80, 0x80, 0x80, 0x80, 0xfe, 0x00, 0x00, 
	0x00, 0x82, 0x82, 0xc6, 0xc6, 0xaa, 0xaa, 0x92, 	// 4D
	0x92, 0x82, 0x82, 0x82, 0x82, 0x82, 0x00, 0x00, 
	0x00, 0x82, 0xc2, 0xc2, 0xa2, 0xa2, 0x92, 0x92, 	// 4E
	0x8a, 0x8a, 0x86, 0x86, 0x82, 0x82, 0x00, 0x00, 
	0x00, 0x38, 0x44, 0x82, 0x82, 0x82, 0x82, 0x82, 	// 4F
	0x82, 0x82, 0x82, 0x82, 0x44, 0x38, 0x00, 0x00, 
	0x00, 0xf8, 0x84, 0x82, 0x82, 0x82, 0x84, 0xf8, 	// 50
	0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x00, 0x00, 
	0x00, 0x38, 0x44, 0x82, 0x82, 0x82, 0x82, 0x82, 	// 51
	0x82, 0x82, 0xba, 0xc6, 0x44, 0x3a, 0x00, 0x00, 
	0x00, 0xf8, 0x84, 0x82, 0x82, 0x82, 0x84, 0xf8, 	// 52
	0x88, 0x88, 0x84, 0x84, 0x82, 0x82, 0x00, 0x00, 
	0x00, 0x78, 0x84, 0x80, 0x80, 0x40, 0x20, 0x10, 	// 53
	0x08, 0x04, 0x02, 0x02, 0x82, 0x7c, 0x00, 0x00, 
	0x00, 0xfe, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 	// 54
	0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x00, 0x00, 
	0x00, 0x82, 0x82, 0x82, 0x82, 0x82, 0x82, 0x82, 	// 55
	0x82, 0x82, 0x82, 0x82, 0x44, 0x38, 0x00, 0x00, 
	0x00, 0x82, 0x82, 0x82, 0x82, 0x44, 0x44, 0x44, 	// 56
	0x44, 0x28, 0x28, 0x28, 0x10, 0x10, 0x00, 0x00, 
	0x00, 0x82, 0x92, 0x92, 0x92, 0x92, 0xaa, 0xaa, 	// 57
	0xaa, 0xaa, 0x44, 0x44, 0x44, 0x44, 0x00, 0x00, 
	0x00, 0x82, 0x82, 0x44, 0x44, 0x28, 0x28, 0x10, 	// 58
	0x28, 0x28, 0x44, 0x44, 0x82, 0x82, 0x00, 0x00, 
	0x00, 0x82, 0x82, 0x44, 0x44, 0x28, 0x28, 0x10, 	// 59
	0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x00, 0x00, 
	0x00, 0xfe, 0x02, 0x04, 0x04, 0x08, 0x08, 0x10, 	// 5A
	0x20, 0x20, 0x40, 0x40, 0x80, 0xfe, 0x00, 0x00, 
	0x00, 0x1e, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 	// 5B
	0x10, 0x10, 0x10, 0x10, 0x10, 0x1e, 0x00, 0x00, 
	0x00, 0x82, 0x82, 0x44, 0x44, 0x28, 0xfe, 0x10, 	// 5C
	0x10, 0xfe, 0x10, 0x10, 0x10, 0x10, 0x00, 0x00, 
	0x00, 0xf0, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 	// 5D
	0x10, 0x10, 0x10, 0x10, 0x10, 0xf0, 0x00, 0x00, 
	0x00, 0x10, 0x28, 0x44, 0x00, 0x00, 0x00, 0x00, 	// 5E
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 	// 5F
	0x00, 0x00, 0x00, 0x00, 0x00, 0xfe, 0x00, 0x00, 
	0x00, 0x20, 0x30, 0x18, 0x08, 0x00, 0x00, 0x00, 	// 60
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x78, 0x04, 	// 61
	0x3a, 0x46, 0x82, 0x82, 0x46, 0x3a, 0x00, 0x00, 
	0x00, 0x80, 0x80, 0x80, 0x80, 0x80, 0xb8, 0xc4, 	// 62
	0x82, 0x82, 0x82, 0x82, 0xc4, 0xb8, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x38, 0x44, 	// 63
	0x82, 0x80, 0x80, 0x82, 0x44, 0x38, 0x00, 0x00, 
	0x00, 0x02, 0x02, 0x02, 0x02, 0x02, 0x3a, 0x46, 	// 64
	0x82, 0x82, 0x82, 0x82, 0x46, 0x3a, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x38, 0x44, 	// 65
	0x82, 0xfe, 0x80, 0x82, 0x44, 0x38, 0x00, 0x00, 
	0x00, 0x0c, 0x12, 0x20, 0x20, 0x20, 0xfc, 0x20, 	// 66
	0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3a, 0x46, 	// 67
	0x82, 0x82, 0x82, 0x46, 0x3a, 0x02, 0x44, 0x38, 
	0x00, 0x80, 0x80, 0x80, 0x80, 0x80, 0xb8, 0xc4, 	// 68
	0x82, 0x82, 0x82, 0x82, 0x82, 0x82, 0x00, 0x00, 
	0x00, 0x00, 0x10, 0x10, 0x00, 0x00, 0x30, 0x10, 	// 69
	0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x00, 0x00, 
	0x00, 0x00, 0x04, 0x04, 0x00, 0x00, 0x0c, 0x04, 	// 6A
	0x04, 0x04, 0x04, 0x04, 0x04, 0x44, 0x48, 0x30, 
	0x00, 0x80, 0x80, 0x80, 0x80, 0x80, 0x84, 0x88, 	// 6B
	0x90, 0xa0, 0xd0, 0x88, 0x84, 0x82, 0x00, 0x00, 
	0x00, 0x30, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 	// 6C
	0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xec, 0x92, 	// 6D
	0x92, 0x92, 0x92, 0x92, 0x92, 0x92, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xb8, 0xc4, 	// 6E
	0x82, 0x82, 0x82, 0x82, 0x82, 0x82, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x38, 0x44, 	// 6F
	0x82, 0x82, 0x82, 0x82, 0x44, 0x38, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xb8, 0xc4, 	// 70
	0x82, 0x82, 0x82, 0xc4, 0xb8, 0x80, 0x80, 0x80, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3a, 0x46, 	// 71
	0x82, 0x82, 0x82, 0x46, 0x3a, 0x02, 0x02, 0x02, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x9c, 0xa2, 	// 72
	0xc0, 0x80, 0x80, 0x80, 0x80, 0x80, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x78, 0x84, 	// 73
	0x40, 0x30, 0x0c, 0x02, 0x82, 0x7c, 0x00, 0x00, 
	0x00, 0x20, 0x20, 0x20, 0x20, 0x20, 0xfc, 0x20, 	// 74
	0x20, 0x20, 0x20, 0x20, 0x22, 0x1c, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x82, 0x82, 	// 75
	0x82, 0x82, 0x82, 0x82, 0x46, 0x3a, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x82, 0x82, 	// 76
	0x44, 0x44, 0x28, 0x28, 0x10, 0x10, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x82, 0x92, 	// 77
	0x92, 0xaa, 0xaa, 0x44, 0x44, 0x44, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x82, 0x44, 	// 78
	0x28, 0x10, 0x10, 0x28, 0x44, 0x82, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x82, 0x82, 	// 79
	0x44, 0x44, 0x28, 0x28, 0x10, 0x10, 0x20, 0xc0, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xfe, 0x04, 	// 7A
	0x08, 0x10, 0x20, 0x40, 0x80, 0xfe, 0x00, 0x00, 
	0x00, 0x06, 0x08, 0x08, 0x08, 0x08, 0x08, 0x10, 	// 7B
	0x08, 0x08, 0x08, 0x08, 0x08, 0x06, 0x00, 0x00, 
	0x00, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 	// 7C
	0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x00, 0x00, 
	0x00, 0xc0, 0x20, 0x20, 0x20, 0x20, 0x20, 0x10, 	// 7D
	0x20, 0x20, 0x20, 0x20, 0x20, 0xc0, 0x00, 0x00, 
	0x00, 0xfe, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 	// 7E
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0xf0, 0x88, 0x88, 0x88, 0x88, 0xf0, 0x00, 	// 7F
	0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x3e, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 	// 80
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 	// 81
	0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 	// 82
	0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 	// 83
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 	// 84
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
	0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 	// 85
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
	0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 	// 86
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 	// 87
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
	0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 	// 88
	0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 
	0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 	// 89
	0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 
	0xe0, 0xe0, 0xe0, 0xe0, 0xe0, 0xe0, 0xe0, 0xe0, 	// 8A
	0xe0, 0xe0, 0xe0, 0xe0, 0xe0, 0xe0, 0xe0, 0xe0, 
	0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 	// 8B
	0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 
	0xf8, 0xf8, 0xf8, 0xf8, 0xf8, 0xf8, 0xf8, 0xf8, 	// 8C
	0xf8, 0xf8, 0xf8, 0xf8, 0xf8, 0xf8, 0xf8, 0xf8, 
	0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 	// 8D
	0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 
	0xfe, 0xfe, 0xfe, 0xfe, 0xfe, 0xfe, 0xfe, 0xfe, 	// 8E
	0xfe, 0xfe, 0xfe, 0xfe, 0xfe, 0xfe, 0xfe, 0xfe, 
	0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 	// 8F
	0xff, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 
	0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 	// 90
	0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 	// 91
	0xff, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 
	0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 	// 92
	0xf0, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 
	0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 	// 93
	0x1f, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 
	0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 	// 94
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 	// 95
	0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 	// 96
	0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 
	0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 	// 97
	0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 	// 98
	0x1f, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 	// 99
	0xf0, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 
	0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 	// 9A
	0x1f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 	// 9B
	0xf0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 	// 9C
	0x03, 0x04, 0x08, 0x08, 0x10, 0x10, 0x10, 0x10, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 	// 9D
	0x80, 0x40, 0x20, 0x20, 0x10, 0x10, 0x10, 0x10, 
	0x10, 0x10, 0x10, 0x10, 0x08, 0x08, 0x08, 0x04, 	// 9E
	0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x10, 0x10, 0x10, 0x10, 0x20, 0x20, 0x20, 0x40, 	// 9F
	0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 	// A0
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 	// A1
	0x00, 0x00, 0x60, 0x90, 0x90, 0x60, 0x00, 0x00, 
	0x00, 0x1e, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 	// A2
	0x10, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 	// A3
	0x10, 0x10, 0x10, 0x10, 0x10, 0xf0, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 	// A4
	0x00, 0x00, 0x80, 0x40, 0x20, 0x10, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 	// A5
	0x18, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0xfe, 0x02, 0x02, 0x02, 0xfe, 0x02, 	// A6
	0x02, 0x02, 0x04, 0x04, 0x18, 0x60, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xfe, 0x02, 	// A7
	0x14, 0x18, 0x10, 0x20, 0x20, 0x40, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x04, 0x08, 	// A8
	0x18, 0x68, 0x08, 0x08, 0x08, 0x08, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x10, 0x10, 0xfe, 0x82, 	// A9
	0x82, 0x82, 0x04, 0x04, 0x08, 0x30, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xfe, 0x10, 	// AA
	0x10, 0x10, 0x10, 0x10, 0x10, 0xfe, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x04, 0x04, 0xfe, 0x04, 	// AB
	0x0c, 0x14, 0x24, 0x44, 0x84, 0x0c, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x20, 0x20, 0x3e, 0xe2, 	// AC
	0x22, 0x24, 0x10, 0x10, 0x10, 0x10, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7c, 0x04, 	// AD
	0x04, 0x04, 0x04, 0x04, 0x04, 0xfe, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xfe, 0x02, 	// AE
	0x02, 0xfe, 0x02, 0x02, 0x02, 0xfe, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x92, 0x92, 	// AF
	0x92, 0x02, 0x04, 0x04, 0x18, 0x60, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 	// B0
	0x7e, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0xfe, 0x02, 0x02, 0x14, 0x18, 0x10, 	// B1
	0x10, 0x10, 0x10, 0x20, 0x20, 0x40, 0x00, 0x00, 
	0x00, 0x02, 0x02, 0x04, 0x04, 0x08, 0x18, 0x28, 	// B2
	0xc8, 0x08, 0x08, 0x08, 0x08, 0x08, 0x00, 0x00, 
	0x00, 0x10, 0x10, 0xfe, 0x82, 0x82, 0x82, 0x02, 	// B3
	0x02, 0x02, 0x04, 0x04, 0x08, 0x30, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0xfe, 0x10, 0x10, 0x10, 0x10, 	// B4
	0x10, 0x10, 0x10, 0x10, 0x10, 0xfe, 0x00, 0x00, 
	0x00, 0x04, 0x04, 0xfe, 0x0c, 0x0c, 0x14, 0x14, 	// B5
	0x24, 0x24, 0x44, 0x84, 0x04, 0x0c, 0x00, 0x00, 
	0x00, 0x10, 0x10, 0xfe, 0x12, 0x12, 0x12, 0x12, 	// B6
	0x12, 0x22, 0x22, 0x42, 0x42, 0x84, 0x00, 0x00, 
	0x00, 0x10, 0x10, 0x1e, 0xf0, 0x10, 0x10, 0x10, 	// B7
	0x1e, 0xe8, 0x08, 0x08, 0x08, 0x08, 0x00, 0x00, 
	0x00, 0x20, 0x20, 0x3e, 0x42, 0x42, 0x82, 0x02, 	// B8
	0x02, 0x04, 0x04, 0x08, 0x10, 0x60, 0x00, 0x00, 
	0x00, 0x40, 0x40, 0x7e, 0x48, 0x48, 0x88, 0x08, 	// B9
	0x08, 0x08, 0x10, 0x10, 0x20, 0x40, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0xfe, 0x02, 0x02, 0x02, 0x02, 	// BA
	0x02, 0x02, 0x02, 0x02, 0x02, 0xfe, 0x00, 0x00, 
	0x00, 0x44, 0x44, 0x44, 0xfe, 0x44, 0x44, 0x44, 	// BB
	0x04, 0x04, 0x08, 0x08, 0x10, 0x60, 0x00, 0x00, 
	0x00, 0x00, 0x60, 0x10, 0x02, 0xc2, 0x22, 0x02, 	// BC
	0x02, 0x04, 0x04, 0x08, 0x10, 0xe0, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0xfe, 0x02, 0x02, 0x04, 0x04, 	// BD
	0x08, 0x08, 0x10, 0x28, 0x44, 0x82, 0x00, 0x00, 
	0x00, 0x40, 0x40, 0xfe, 0x42, 0x42, 0x44, 0x48, 	// BE
	0x40, 0x40, 0x40, 0x40, 0x20, 0x1e, 0x00, 0x00, 
	0x00, 0x00, 0x82, 0x42, 0x42, 0x22, 0x22, 0x02, 	// BF
	0x02, 0x04, 0x04, 0x08, 0x10, 0x60, 0x00, 0x00, 
	0x00, 0x20, 0x20, 0x3e, 0x42, 0x42, 0xa2, 0x12, 	// C0
	0x0a, 0x04, 0x04, 0x08, 0x10, 0x60, 0x00, 0x00, 
	0x00, 0x00, 0x0c, 0x70, 0x10, 0x10, 0x10, 0xfe, 	// C1
	0x10, 0x10, 0x10, 0x20, 0x20, 0x40, 0x00, 0x00, 
	0x00, 0x00, 0x92, 0x92, 0x92, 0x92, 0x02, 0x02, 	// C2
	0x02, 0x04, 0x04, 0x08, 0x10, 0x60, 0x00, 0x00, 
	0x00, 0x00, 0x7c, 0x00, 0x00, 0x00, 0xfe, 0x10, 	// C3
	0x10, 0x10, 0x10, 0x20, 0x20, 0x40, 0x00, 0x00, 
	0x00, 0x40, 0x40, 0x40, 0x40, 0x40, 0x60, 0x50, 	// C4
	0x48, 0x44, 0x40, 0x40, 0x40, 0x40, 0x00, 0x00, 
	0x00, 0x10, 0x10, 0x10, 0xfe, 0x10, 0x10, 0x10, 	// C5
	0x10, 0x10, 0x10, 0x20, 0x20, 0x40, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x7c, 0x00, 0x00, 0x00, 0x00, 	// C6
	0x00, 0x00, 0x00, 0x00, 0x00, 0xfe, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0xfe, 0x02, 0x02, 0x04, 0x04, 	// C7
	0x48, 0x28, 0x10, 0x28, 0x44, 0x80, 0x00, 0x00, 
	0x00, 0x10, 0x10, 0xfe, 0x02, 0x04, 0x04, 0x08, 	// C8
	0x10, 0x34, 0xd2, 0x10, 0x10, 0x10, 0x00, 0x00, 
	0x00, 0x00, 0x02, 0x02, 0x02, 0x02, 0x02, 0x04, 	// C9
	0x04, 0x08, 0x08, 0x10, 0x20, 0xc0, 0x00, 0x00, 
	0x00, 0x00, 0x08, 0x04, 0x02, 0x42, 0x42, 0x42, 	// CA
	0x42, 0x42, 0x42, 0x42, 0x42, 0x82, 0x00, 0x00, 
	0x00, 0x00, 0x80, 0x80, 0x80, 0x80, 0xfe, 0x80, 	// CB
	0x80, 0x80, 0x80, 0x80, 0x80, 0x7e, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0xfe, 0x02, 0x02, 0x02, 0x02, 	// CC
	0x02, 0x04, 0x04, 0x08, 0x10, 0x60, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x20, 0x50, 0x50, 0x88, 0x88, 	// CD
	0x04, 0x04, 0x04, 0x02, 0x02, 0x02, 0x00, 0x00, 
	0x00, 0x10, 0x10, 0x10, 0xfe, 0x10, 0x10, 0x54, 	// CE
	0x54, 0x52, 0x92, 0x10, 0x10, 0x10, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0xfe, 0x02, 0x02, 0x02, 0x04, 	// CF
	0x04, 0x48, 0x28, 0x10, 0x08, 0x04, 0x00, 0x00, 
	0x00, 0x00, 0x70, 0x0c, 0x00, 0x00, 0x00, 0x70, 	// D0
	0x0c, 0x00, 0x00, 0x00, 0xf8, 0x06, 0x00, 0x00, 
	0x00, 0x00, 0x10, 0x10, 0x10, 0x20, 0x20, 0x20, 	// D1
	0x44, 0x44, 0x44, 0x84, 0x9a, 0xe2, 0x00, 0x00, 
	0x00, 0x00, 0x02, 0x02, 0x02, 0x02, 0x04, 0x24, 	// D2
	0x14, 0x08, 0x0c, 0x12, 0x20, 0xc0, 0x00, 0x00, 
	0x00, 0x00, 0xfe, 0x20, 0x20, 0x20, 0x20, 0xfe, 	// D3
	0x20, 0x20, 0x20, 0x20, 0x20, 0x1e, 0x00, 0x00, 
	0x00, 0x20, 0x20, 0x3e, 0xe2, 0x22, 0x24, 0x20, 	// D4
	0x20, 0x10, 0x10, 0x10, 0x10, 0x10, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x7c, 0x04, 0x04, 0x04, 0x04, 	// D5
	0x04, 0x04, 0x04, 0x04, 0x04, 0xfe, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0xfe, 0x02, 0x02, 0x02, 0x02, 	// D6
	0xfe, 0x02, 0x02, 0x02, 0x02, 0xfe, 0x00, 0x00, 
	0x00, 0x00, 0x7c, 0x00, 0x00, 0xfe, 0x02, 0x02, 	// D7
	0x02, 0x04, 0x04, 0x08, 0x10, 0x60, 0x00, 0x00, 
	0x00, 0x02, 0x42, 0x42, 0x42, 0x42, 0x42, 0x42, 	// D8
	0x02, 0x02, 0x04, 0x04, 0x08, 0x30, 0x00, 0x00, 
	0x00, 0x00, 0x10, 0x50, 0x50, 0x50, 0x50, 0x50, 	// D9
	0x50, 0x52, 0x52, 0x54, 0x58, 0x90, 0x00, 0x00, 
	0x00, 0x00, 0x40, 0x40, 0x40, 0x40, 0x40, 0x42, 	// DA
	0x42, 0x44, 0x44, 0x48, 0x50, 0x60, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0xfe, 0x82, 0x82, 0x82, 0x82, 	// DB
	0x82, 0x82, 0x82, 0x82, 0x82, 0xfe, 0x00, 0x00, 
	0x00, 0x00, 0xfe, 0x82, 0x82, 0x82, 0x02, 0x02, 	// DC
	0x02, 0x02, 0x04, 0x04, 0x08, 0x30, 0x00, 0x00, 
	0x00, 0x00, 0xc0, 0x22, 0x02, 0x02, 0x02, 0x02, 	// DD
	0x02, 0x04, 0x04, 0x08, 0x10, 0xe0, 0x00, 0x00, 
	0x00, 0x20, 0x90, 0x40, 0x00, 0x00, 0x00, 0x00, 	// DE
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x60, 0x90, 0x90, 0x60, 0x00, 0x00, 0x00, 	// DF
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 	// E0
	0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x10, 0x10, 0x10, 0x10, 0x10, 0x1f, 0x10, 0x10, 	// E1
	0x10, 0x10, 0x1f, 0x10, 0x10, 0x10, 0x10, 0x10, 
	0x10, 0x10, 0x10, 0x10, 0x10, 0xff, 0x10, 0x10, 	// E2
	0x10, 0x10, 0xff, 0x10, 0x10, 0x10, 0x10, 0x10, 
	0x08, 0x08, 0x08, 0x08, 0x08, 0xf8, 0x08, 0x08, 	// E3
	0x08, 0x08, 0xf8, 0x08, 0x08, 0x08, 0x08, 0x08, 
	0x01, 0x01, 0x03, 0x03, 0x07, 0x07, 0x0f, 0x0f, 	// E4
	0x1f, 0x1f, 0x3f, 0x3f, 0x7f, 0x7f, 0xff, 0xff, 
	0x80, 0x80, 0xc0, 0xc0, 0xe0, 0xe0, 0xf0, 0xf0, 	// E5
	0xf8, 0xf8, 0xfc, 0xfc, 0xfe, 0xfe, 0xff, 0xff, 
	0xff, 0xff, 0x7f, 0x7f, 0x3f, 0x3f, 0x1f, 0x1f, 	// E6
	0x0f, 0x0f, 0x07, 0x07, 0x03, 0x03, 0x01, 0x01, 
	0xff, 0xff, 0xfe, 0xfe, 0xfc, 0xfc, 0xf8, 0xf8, 	// E7
	0xf0, 0xf0, 0xe0, 0xe0, 0xc0, 0xc0, 0x80, 0x80, 
	0x00, 0x10, 0x38, 0x38, 0x7c, 0x7c, 0xfe, 0xfe, 	// E8
	0xfe, 0xfe, 0x7c, 0x10, 0x38, 0x7c, 0x00, 0x00, 
	0x00, 0x6c, 0x6c, 0xfe, 0xfe, 0xfe, 0xfe, 0xfe, 	// E9
	0x7c, 0x7c, 0x7c, 0x38, 0x38, 0x10, 0x00, 0x00, 
	0x00, 0x10, 0x10, 0x38, 0x38, 0x7c, 0x7c, 0xfe, 	// EA
	0x7c, 0x7c, 0x38, 0x38, 0x10, 0x10, 0x00, 0x00, 
	0x00, 0x00, 0x10, 0x38, 0x38, 0x38, 0x54, 0xfe, 	// EB
	0xfe, 0xfe, 0x54, 0x10, 0x38, 0x7c, 0x00, 0x00, 
	0x00, 0x00, 0x38, 0x7c, 0x7c, 0xfe, 0xfe, 0xfe, 	// EC
	0xfe, 0xfe, 0xfe, 0x7c, 0x7c, 0x38, 0x00, 0x00, 
	0x00, 0x00, 0x38, 0x44, 0x44, 0x82, 0x82, 0x82, 	// ED
	0x82, 0x82, 0x82, 0x44, 0x44, 0x38, 0x00, 0x00, 
	0x01, 0x01, 0x02, 0x02, 0x04, 0x04, 0x08, 0x08, 	// EE
	0x10, 0x10, 0x20, 0x20, 0x40, 0x40, 0x80, 0x80, 
	0x80, 0x80, 0x40, 0x40, 0x20, 0x20, 0x10, 0x10, 	// EF
	0x08, 0x08, 0x04, 0x04, 0x02, 0x02, 0x01, 0x01, 
	0x81, 0x81, 0x42, 0x42, 0x24, 0x24, 0x18, 0x18, 	// F0
	0x18, 0x18, 0x24, 0x24, 0x42, 0x42, 0x81, 0x81, 
	0x00, 0x00, 0x00, 0xfe, 0x92, 0x92, 0x92, 0xfe, 	// F1
	0x82, 0x82, 0x82, 0x82, 0x82, 0x86, 0x00, 0x00, 
	0x00, 0x00, 0x40, 0x40, 0x7e, 0x88, 0x88, 0x7e, 	// F2
	0x48, 0x48, 0xfe, 0x08, 0x08, 0x08, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x7e, 0x42, 0x42, 0x7e, 0x42, 	// F3
	0x42, 0x7e, 0x42, 0x42, 0x82, 0x86, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x7e, 0x42, 0x42, 0x42, 0x42, 	// F4
	0x7e, 0x42, 0x42, 0x42, 0x42, 0x7e, 0x00, 0x00, 
	0x00, 0x00, 0x02, 0x02, 0xef, 0xa2, 0xbf, 0xa2, 	// F5
	0xff, 0xa2, 0xaa, 0xaa, 0xe2, 0x06, 0x00, 0x00, 
	0x00, 0x00, 0x2c, 0x24, 0x24, 0x42, 0x42, 0xbd, 	// F6
	0x95, 0x14, 0x14, 0x24, 0x24, 0x4c, 0x00, 0x00, 
	0x00, 0x00, 0x24, 0x44, 0xd5, 0x55, 0x55, 0xe4, 	// F7
	0x4d, 0xe1, 0xe2, 0xc2, 0x44, 0x48, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0xfe, 0x00, 0x00, 0xfe, 0x10, 	// F8
	0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x00, 0x00, 
	0x00, 0x00, 0x10, 0x10, 0xfe, 0x10, 0x10, 0xfe, 	// F9
	0x92, 0x92, 0x92, 0x96, 0x10, 0x10, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0xfe, 0x80, 0x84, 0xa4, 0x98, 	// FA
	0x88, 0x94, 0xa0, 0x80, 0xfe, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0xf8, 0xa8, 0xaf, 0xaa, 0xfa, 0xaa, 	// FB
	0xaa, 0xaa, 0xfa, 0x02, 0x02, 0x06, 0x00, 0x00, 
	0x00, 0x00, 0x22, 0x22, 0x22, 0xff, 0x22, 0x72, 	// FC
	0x6a, 0xaa, 0xaa, 0xa2, 0x22, 0x26, 0x00, 0x00, 
	0x00, 0x00, 0x10, 0x10, 0x10, 0x10, 0x10, 0x20, 	// FD
	0x30, 0x28, 0x48, 0x44, 0x84, 0x82, 0x00, 0x00, 
	0xaa, 0x55, 0xaa, 0x55, 0xaa, 0x55, 0xaa, 0x55, 	// FE
	0xaa, 0x55, 0xaa, 0x55, 0xaa, 0x55, 0xaa, 0x55, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 	// FF
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
};