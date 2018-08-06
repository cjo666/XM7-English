;
; FM-7 EMULATOR "XM7"
;
; Copyright (C) 1999-2017 �o�h�D(Twitter:@xm6_original)
; Copyright (C) 2001-2017 Ryu Takegami (Twitter:@RyuTakegami)
;
; [ Win32API �����_�����O(DirectDraw,High Color) ]
;
; RHG����
;	2002.05.31		4096�F�����_���̃��������C�g��4�o�C�g�P�ʂōs���悤�ύX
;	2002.06.21		�X�V���K�v�ȕ������������_�����O����悤�ɕύX
;	2003.02.11		V3�p8�F���[�h�����_��(200line/400line)�𓝍�
;	2004.02.09		�^��400���C���n�����_����ǉ�
;	2008.01.20		���Ȃ��������Ƃɂ���
;	2012.12.01		NASM2�n�ł��A�Z���u�����ʂ�悤�ɏC��


;
; �O����`
;
	%if	XM7_VER >= 3
		section	.data class=DATA align=4 use32
		extern	_vram_c
		extern	_vram_dptr
		extern	_vram_bdptr
		extern	_vram_dblk
		extern	_vram_bdblk
		extern	_rgbTTLDD
		extern	_rgbAnalogDD
		extern	_DDDrawFlag

		section	.text class=CODE align=16 use32
		global	_Render640DD2
		global	_Render640wDD2
		global	_Render320DD
		global	_Render320wDD
		global	_Render256k555DD
		global	_Render256k565DD
		global	_Render640cDD
		global	_Render640cwDD
	%elif XM7_VER >= 2
		section	.data class=DATA align=4 use32
		extern	_vram_c
		extern	_vram_dptr
		extern	_rgbTTLDD
		extern	_rgbAnalogDD
		extern	_DDDrawFlag

		section	.text class=CODE align=16 use32
		global	_Render640DD
		global	_Render320DD
		global	_Render640cDD
	%else
		section	.data class=DATA align=4 use32
		extern	_vram_c
		extern	_rgbTTLDD
		extern	_DDDrawFlag

		section	.text class=CODE align=16 use32
		global	_Render640DD
		global	_Render640mDD
	%endif

	%if XM7_VER <= 2

;
; 640x200�A�f�W�^�����[�h
; DirectDraw�����_�����O
;
; static void Render640(LPVOID lpSurface, LONG lPitch, int first, int last)
; lpSurface	ebp+8
; lPitch	ebp+12
; first		ebp+16
; last		ebp+20
;
		align	4
_Render640DD:
		push	ebp
		mov	ebp,esp
		push	ebx
		push	esi
		push	edi
; �T�[�t�F�C�X�ݒ�
		mov	edi,[ebp+8]
		mov	eax,[ebp+12]
		imul	eax,[ebp+16]
		add	edi,eax
		add	edi,eax
; �s�b�`��O�����Čv�Z
		mov	esi,[ebp+12]
		sub	esi,1280
		mov	[ebp+12],esi
; VRAM�A�h���X�ݒ�
	%if XM7_VER >= 2
		mov	esi,[_vram_dptr]
	%else
		mov	esi,[_vram_c]
	%endif
		add	esi,0x4000
		mov	ecx,[ebp+16]
		mov	eax,ecx
		imul	ecx,80
		add	esi,ecx
; ���C�����ݒ�
		mov	edx,[ebp+20]
		sub	edx,eax
		jz	near .exit
; �P���C�����[�v
		align	4
.line_loop:
		lea	ecx,[eax*4]
		push	eax
		and	ecx,0xfffffff0
		push	edx
		lea	ecx,[ecx*4+ecx]
		mov	ebx,80
; �P�o�C�g���[�v
		align	4
.byte_loop:
		cmp	byte [_DDDrawFlag + ecx], 0
		je	.next_byte

		push	ecx
;
		mov	al,[esi+0x4000]
		mov	ah,[esi]
		mov	dh,[esi-0x4000]
; bit7
		xor	ecx,ecx
		add	al,al
		adc	ecx,ecx
		add	ah,ah
		adc	ecx,ecx
		add	dh,dh
		adc	ecx,ecx
		mov	cx,[_rgbTTLDD+ecx*4]
		mov	[edi],cx
; bit6
		xor	ecx,ecx
		add	al,al
		adc	ecx,ecx
		add	ah,ah
		adc	ecx,ecx
		add	dh,dh
		adc	ecx,ecx
		mov	cx,[_rgbTTLDD+ecx*4]
		mov	[edi+2],cx
; bit5
		xor	ecx,ecx
		add	al,al
		adc	ecx,ecx
		add	ah,ah
		adc	ecx,ecx
		add	dh,dh
		adc	ecx,ecx
		mov	cx,[_rgbTTLDD+ecx*4]
		mov	[edi+4],cx
; bit4
		xor	ecx,ecx
		add	al,al
		adc	ecx,ecx
		add	ah,ah
		adc	ecx,ecx
		add	dh,dh
		adc	ecx,ecx
		mov	cx,[_rgbTTLDD+ecx*4]
		mov	[edi+6],cx
; bit3
		xor	ecx,ecx
		add	al,al
		adc	ecx,ecx
		add	ah,ah
		adc	ecx,ecx
		add	dh,dh
		adc	ecx,ecx
		mov	cx,[_rgbTTLDD+ecx*4]
		mov	[edi+8],cx
; bit2
		xor	ecx,ecx
		add	al,al
		adc	ecx,ecx
		add	ah,ah
		adc	ecx,ecx
		add	dh,dh
		adc	ecx,ecx
		mov	cx,[_rgbTTLDD+ecx*4]
		mov	[edi+10],cx
; bit1
		xor	ecx,ecx
		add	al,al
		adc	ecx,ecx
		add	ah,ah
		adc	ecx,ecx
		add	dh,dh
		adc	ecx,ecx
		mov	cx,[_rgbTTLDD+ecx*4]
		mov	[edi+12],cx
; bit0
		xor	ecx,ecx
		add	al,al
		adc	ecx,ecx
		add	ah,ah
		adc	ecx,ecx
		add	dh,dh
		adc	ecx,ecx
		mov	cx,[_rgbTTLDD+ecx*4]
		mov	[edi+14],cx
; ���̃o�C�g��
		pop	ecx
.next_byte:
		add	edi,16
		inc	esi
		inc	ecx
		dec	ebx
		jnz	near .byte_loop

; ���̃��C����
		pop	edx
		pop	eax
; �C���^�[���[�X�����X�L�b�v
.next_line:
		add	edi,[ebp+12]
		add	edi,1280
		add	edi,[ebp+12]
		inc	eax
		dec	edx
		jnz	near .line_loop
; �I��
.exit:
		pop	edi
		pop	esi
		pop	ebx
		pop	ebp
		ret

	%else

;
; 640x200/640x400�A�f�W�^�����[�h
; DirectDraw�����_�����O
;
; static void Render640(LPVOID lpSurface, LONG lPitch, int first, int last)
; lpSurface	ebp+8
; lPitch	ebp+12
; first		ebp+16
; last		ebp+20
;
		align	4
_Render640DD2:
		push	ebp
		mov	ebp,esp
		push	ebx
		push	esi
		push	edi
; �T�[�t�F�C�X�ݒ�
		mov	edi,[ebp+8]
		mov	eax,[ebp+12]
		mov	ebx,[ebp+24]
		imul	eax,[ebp+16]
		imul	eax,ebx
		add	edi,eax
; �s�b�`��O�����Čv�Z
		mov	esi,[ebp+12]
		imul	esi,ebx
		sub	esi,1280
		mov	[ebp+12],esi
; VRAM�A�h���X�ݒ�
		mov	esi,[_vram_dptr]
		add	esi,0x8000
		mov	ecx,[ebp+16]
		mov	eax,ecx
		imul	ecx,80
		add	esi,ecx
; ���C�����ݒ�
		mov	edx,[ebp+20]
		sub	edx,eax
		jz	near .exit
; �P���C�����[�v
		align	4
.line_loop:
		lea	ecx,[eax*2]
		imul	ecx,[ebp+24]
		push	eax
		and	ecx,0xfffffff0
		push	edx
		lea	ecx,[ecx*4+ecx]
		mov	ebx,80
; �P�o�C�g���[�v
		align	4
.byte_loop:
		cmp	byte [_DDDrawFlag + ecx], 0
		je	.next_byte

		push	ecx
;
		mov	al,[esi+0x8000]
		mov	ah,[esi]
		mov	dh,[esi-0x8000]
; bit7
		xor	ecx,ecx
		add	al,al
		adc	ecx,ecx
		add	ah,ah
		adc	ecx,ecx
		add	dh,dh
		adc	ecx,ecx
		mov	cx,[_rgbTTLDD+ecx*4]
		mov	[edi],cx
; bit6
		xor	ecx,ecx
		add	al,al
		adc	ecx,ecx
		add	ah,ah
		adc	ecx,ecx
		add	dh,dh
		adc	ecx,ecx
		mov	cx,[_rgbTTLDD+ecx*4]
		mov	[edi+2],cx
; bit5
		xor	ecx,ecx
		add	al,al
		adc	ecx,ecx
		add	ah,ah
		adc	ecx,ecx
		add	dh,dh
		adc	ecx,ecx
		mov	cx,[_rgbTTLDD+ecx*4]
		mov	[edi+4],cx
; bit4
		xor	ecx,ecx
		add	al,al
		adc	ecx,ecx
		add	ah,ah
		adc	ecx,ecx
		add	dh,dh
		adc	ecx,ecx
		mov	cx,[_rgbTTLDD+ecx*4]
		mov	[edi+6],cx
; bit3
		xor	ecx,ecx
		add	al,al
		adc	ecx,ecx
		add	ah,ah
		adc	ecx,ecx
		add	dh,dh
		adc	ecx,ecx
		mov	cx,[_rgbTTLDD+ecx*4]
		mov	[edi+8],cx
; bit2
		xor	ecx,ecx
		add	al,al
		adc	ecx,ecx
		add	ah,ah
		adc	ecx,ecx
		add	dh,dh
		adc	ecx,ecx
		mov	cx,[_rgbTTLDD+ecx*4]
		mov	[edi+10],cx
; bit1
		xor	ecx,ecx
		add	al,al
		adc	ecx,ecx
		add	ah,ah
		adc	ecx,ecx
		add	dh,dh
		adc	ecx,ecx
		mov	cx,[_rgbTTLDD+ecx*4]
		mov	[edi+12],cx
; bit0
		xor	ecx,ecx
		add	al,al
		adc	ecx,ecx
		add	ah,ah
		adc	ecx,ecx
		add	dh,dh
		adc	ecx,ecx
		mov	cx,[_rgbTTLDD+ecx*4]
		mov	[edi+14],cx
; ���̃o�C�g��
		pop	ecx
.next_byte:
		add	edi,16
		inc	esi
		inc	ecx
		dec	ebx
		jnz	near .byte_loop

; ���̃��C����
		pop	edx
		pop	eax
; �C���^�[���[�X�����X�L�b�v
.next_line:
		add	edi,[ebp+12]
		inc	eax
		dec	edx
		jnz	near .line_loop
; �I��
.exit:
		pop	edi
		pop	esi
		pop	ebx
		pop	ebp
		ret


;
; 640x200/640x400�A�f�W�^�����[�h
; DirectDraw�����_�����O(Window)
;
; static void Render640w(LPVOID lpSurface, LONG lPitch,
;			int first, int last, int firstx, int lastx)
; lpSurface	ebp+8
; lPitch	ebp+12
; first		ebp+16
; last		ebp+20
;
		align	4
_Render640wDD2:
		push	ebp
		mov	ebp,esp
		push	ebx
		push	esi
		push	edi
; �T�[�t�F�C�X�ݒ�
		mov	edi,[ebp+8]
		mov	eax,[ebp+12]
		mov	ebx,[ebp+32]
		imul	eax,[ebp+16]
		imul	ecx,ebx
		add	edi,eax
; �s�b�`��O�����Čv�Z
		mov	esi,[ebp+12]
		imul	esi,ebx
		sub	esi,1280
		mov	[ebp+12],esi
; VRAM�A�h���X�ݒ�
		mov	esi,0x8000
		mov	ecx,[ebp+16]
		mov	eax,ecx
		imul	ecx,80
		add	esi,ecx
; ���C�����ݒ�
		mov	edx,[ebp+20]
		sub	edx,eax
		jz	near .exit
; �E�B���h�E�؂�ւ��|�C���^�ݒ�
		shr	dword [ebp+24],3
		shr	dword [ebp+28],3
		mov	cl,[ebp+24]
		mov	ch,[ebp+28]
; �P���C�����[�v
		align	4
.line_loop:
		push	eax
		push	edx
		shl	eax,1
		imul	eax,[ebp+32]
		push	ebp
		xor	ebx,ebx
		and	eax,0xfffffff0
		mov	ebp,[_vram_dptr]
		lea	eax,[eax*4+eax]
; �P�o�C�g���[�v
		align	4
.byte_loop:
		cmp	ebx,80
		jae	near .next_line

		cmp	bl,cl
		jz	.change_dbank2
		cmp	bl,ch
		jnz	.draw_byte
.change_dbank1:
		mov	ebp,[_vram_dptr]
		jmp	.draw_byte
.change_dbank2:
		mov	ebp,[_vram_bdptr]

.draw_byte:
		cmp	dword [_DDDrawFlag + eax], 0
		je	.next_byte

		push	eax
		push	ebx
;
		mov	al,[esi+ebp+0x8000]
		mov	ah,[esi+ebp]
		mov	dl,[esi+ebp-0x8000]
; bit 7
		xor	ebx,ebx
		add	al,al
		adc	ebx,ebx
		add	ah,ah
		adc	ebx,ebx
		add	dl,dl
		adc	ebx,ebx
		mov	bx,[ebx*4+_rgbTTLDD]
		mov	[edi],bx
; bit 6
		xor	ebx,ebx
		add	al,al
		adc	ebx,ebx
		add	ah,ah
		adc	ebx,ebx
		add	dl,dl
		adc	ebx,ebx
		mov	bx,[ebx*4+_rgbTTLDD]
		mov	[edi+2],bx
; bit 5
		xor	ebx,ebx
		add	al,al
		adc	ebx,ebx
		add	ah,ah
		adc	ebx,ebx
		add	dl,dl
		adc	ebx,ebx
		mov	bx,[ebx*4+_rgbTTLDD]
		mov	[edi+4],bx
; bit 4
		xor	ebx,ebx
		add	al,al
		adc	ebx,ebx
		add	ah,ah
		adc	ebx,ebx
		add	dl,dl
		adc	ebx,ebx
		mov	bx,[ebx*4+_rgbTTLDD]
		mov	[edi+6],bx
; bit 3
		xor	ebx,ebx
		add	al,al
		adc	ebx,ebx
		add	ah,ah
		adc	ebx,ebx
		add	dl,dl
		adc	ebx,ebx
		mov	bx,[ebx*4+_rgbTTLDD]
		mov	[edi+8],bx
; bit 2
		xor	ebx,ebx
		add	al,al
		adc	ebx,ebx
		add	ah,ah
		adc	ebx,ebx
		add	dl,dl
		adc	ebx,ebx
		mov	bx,[ebx*4+_rgbTTLDD]
		mov	[edi+10],bx
; bit 1
		xor	ebx,ebx
		add	al,al
		adc	ebx,ebx
		add	ah,ah
		adc	ebx,ebx
		add	dl,dl
		adc	ebx,ebx
		mov	bx,[ebx*4+_rgbTTLDD]
		mov	[edi+12],bx
; bit 0
		xor	ebx,ebx
		add	al,al
		adc	ebx,ebx
		add	ah,ah
		adc	ebx,ebx
		add	dl,dl
		adc	ebx,ebx
		mov	bx,[ebx*4+_rgbTTLDD]
		mov	[edi+14],bx
; ���̃o�C�g��
		pop	ebx
		pop	eax
.next_byte:
		inc	esi
		add	edi,16
		inc	eax
		inc	ebx
		jmp	near .byte_loop

; �C���^�[���[�X�����X�L�b�v
		align	4
.next_line:
		pop	ebp
		pop	edx
		pop	eax
.next_line2:
; ���̃��C����
		add	edi,[ebp+12]
		inc	eax
		dec	edx
		jnz	near .line_loop
; �I��
.exit:
		pop	edi
		pop	esi
		pop	ebx
		pop	ebp
		ret

	%endif


	%if XM7_VER >= 2

;
; 320x200�A�A�i���O���[�h
; DirectDraw�����_�����O
;
; static void Render320(LPVOID lpSurface, LONG lPitch, int first, int last)
; lpSurface	ebp+8
; lPitch	ebp+12
; first		ebp+16
; last		ebp+20
;
		align	4
_Render320DD:
		push	ebp
		mov	ebp,esp
		push	ebx
		push	esi
		push	edi
; �T�[�t�F�C�X�ݒ�
		mov	edi,[ebp+8]
		mov	eax,[ebp+12]
		imul	eax,[ebp+16]
		add	edi,eax
		add	edi,eax
; �s�b�`��O�����Čv�Z
		mov	esi,[ebp+12]
		sub	esi,1280
		mov	[ebp+12],esi
; VRAM�A�h���X�ݒ�
	%if	XM7_VER >= 3
		mov	esi,[_vram_dptr]
	%else
		mov	esi,[_vram_c]
	%endif
		add	esi,0x0000c000
		mov	ecx,[ebp+16]
		mov	eax,ecx
		imul	ecx,40
		add	esi,ecx
; ���C�����ݒ�
		mov	edx,[ebp+20]
		sub	edx,eax
		jz	near .exit
; �P���C�����[�v
		align	4
.line_loop:
		lea	ebx,[eax*4]
		mov	ecx,40
		push	eax
		and	ebx,0xfffffff0
		push	edx
		lea	ebx,[ebx*4+ebx]
; �P�o�C�g���[�v
		align	4
.byte_loop:
		cmp	byte [_DDDrawFlag + ebx], 0
		je	.next_byte2

		push	ecx
		push	ebx
; G
%if XM7_VER >= 3
		mov	al,[esi+0x4000]
		mov	ah,[esi+0x6000]
		mov	dl,[esi+0x8000]
		mov	dh,[esi+0xa000]
%else
		mov	al,[esi-0x4000]
		mov	ah,[esi-0x2000]
		mov	dl,[esi+0x8000]
		mov	dh,[esi+0xa000]
%endif
		mov	ebx,8
; G���[�v
.g_loop:
		xor	ecx,ecx
		add	al,al
		adc	ecx,ecx
		add	ah,ah
		adc	ecx,ecx
		add	dl,dl
		adc	ecx,ecx
		add	dh,dh
		adc	ecx,ecx
		mov	[edi],ecx
		add	edi,4
		dec	ebx
		jnz	.g_loop
		sub	edi,32
; R
%if XM7_VER >= 3
		mov	al,[esi-0x4000]
		mov	ah,[esi-0x2000]
		mov	dl,[esi-0x0000]
		mov	dh,[esi+0x2000]
%else
		mov	al,[esi-0x8000]
		mov	ah,[esi-0x6000]
		mov	dl,[esi+0x4000]
		mov	dh,[esi+0x6000]
%endif
		mov	ebx,8
; R���[�v
.r_loop:
		mov	ecx,[edi]
		add	al,al
		adc	ecx,ecx
		add	ah,ah
		adc	ecx,ecx
		add	dl,dl
		adc	ecx,ecx
		add	dh,dh
		adc	ecx,ecx
		mov	[edi],ecx
		add	edi,4
		dec	ebx
		jnz	.r_loop
		sub	edi,32
; B
%if XM7_VER >= 3
		mov	al,[esi-0xc000]
		mov	ah,[esi-0xa000]
		mov	dl,[esi-0x8000]
		mov	dh,[esi-0x6000]
%else
		mov	al,[esi-0xc000]
		mov	ah,[esi-0xa000]
		mov	dl,[esi]
		mov	dh,[esi+0x2000]
%endif
		mov	ebx,8
; B���[�v
.b_loop:
		mov	ecx,[edi]
		add	al,al
		adc	ecx,ecx
		add	ah,ah
		adc	ecx,ecx
		add	dl,dl
		adc	ecx,ecx
		add	dh,dh
		adc	ecx,ecx
		and	ecx,0x00000fff
; �p���b�g�ϊ�
		mov	ecx,[ecx*4+_rgbAnalogDD]
		mov	[edi],ecx
		add	edi,4
		dec	ebx
		jnz	.b_loop
; ���̃o�C�g��
		pop	ebx
		pop	ecx
.next_byte0:
		inc	esi
		add	ebx,2
		dec	ecx
		jnz	near .byte_loop
		jmp	.next_line
;
.next_byte2:
		add	edi,32
		jmp	.next_byte0

.next_line:
		pop	edx
		pop	eax
; �C���^�[���[�X�����X�L�b�v
.next_line2:
		add	edi,[ebp+12]
		add	edi,1280
; ���̃��C����
		add	edi,[ebp+12]
		inc	eax
		dec	edx
		jnz	near .line_loop
; �I��
.exit:
		pop	edi
		pop	esi
		pop	ebx
		pop	ebp
		ret

	%endif

	%if	XM7_VER >= 3

;
; 320x200�A�A�i���O���[�h
; DirectDraw�����_�����O(Window)
;
; static void Render320w(LPVOID lpSurface, LONG lPitch,
;			int first, int last, int firstx, int lastx)
; lpSurface	ebp+8
; lPitch	ebp+12
; first		ebp+16
; last		ebp+20
;
		align	4
_Render320wDD:
		push	ebp
		mov	ebp,esp
		push	ebx
		push	esi
		push	edi
; �T�[�t�F�C�X�ݒ�
		mov	edi,[ebp+8]
		mov	eax,[ebp+12]
		imul	eax,[ebp+16]
		add	edi,eax
		add	edi,eax
; �s�b�`��O�����Čv�Z
		mov	esi,[ebp+12]
		sub	esi,1280
		mov	[ebp+12],esi
; VRAM�A�h���X�ݒ�
		mov	ecx,[ebp+16]
		mov	eax,ecx
		imul	ecx,40
		lea	esi,[ecx+0xc000]
; ���C�����ݒ�
		mov	edx,[ebp+20]
		sub	edx,eax
		jz	near .exit
; �E�C���h�E�؂�ւ��|�C���g�␳
		shr	dword [ebp+24],3
		shr	dword [ebp+28],3
; �P���C�����[�v
		align	4
.line_loop:
		push	eax
		push	edx
		push	ebp
		lea	eax,[eax*4]
		xor	ebx,ebx
		and	eax,0xfffffff0
		mov	ebp,[_vram_dptr]
		lea	eax,[eax*4+eax]
; �P�o�C�g���[�v
		align	4
.byte_loop:
		cmp	ebx,40
		jae	near .next_line

		cmp	bl,byte [esp+24+24]
		jz	.change_dbank2
		cmp	bl,byte [esp+28+24]
		jnz	.draw_byte
.change_dbank1:
		mov	ebp,[_vram_dptr]
		jmp	.draw_byte
.change_dbank2:
		mov	ebp,[_vram_bdptr]

.draw_byte:
		cmp	byte [_DDDrawFlag + eax], 0
		je	.next_byte2

		push	eax
		push	ebx
; G
		mov	al,[esi+ebp+0x4000]
		mov	ah,[esi+ebp+0x6000]
		mov	dl,[esi+ebp+0x8000]
		mov	dh,[esi+ebp+0xa000]
		mov	ebx,8
; G���[�v
.g_loop:
		xor	ecx,ecx
		add	al,al
		adc	ecx,ecx
		add	ah,ah
		adc	ecx,ecx
		add	dl,dl
		adc	ecx,ecx
		add	dh,dh
		adc	ecx,ecx
		mov	[edi],ecx
		add	edi,4
		dec	ebx
		jnz	.g_loop
		sub	edi,32
; R
		mov	al,[esi+ebp-0x4000]
		mov	ah,[esi+ebp-0x2000]
		mov	dl,[esi+ebp-0x0000]
		mov	dh,[esi+ebp+0x2000]
		mov	ebx,8
; R���[�v
.r_loop:
		mov	ecx,[edi]
		add	al,al
		adc	ecx,ecx
		add	ah,ah
		adc	ecx,ecx
		add	dl,dl
		adc	ecx,ecx
		add	dh,dh
		adc	ecx,ecx
		mov	[edi],ecx
		add	edi,4
		dec	ebx
		jnz	.r_loop
		sub	edi,32
; B
		mov	al,[esi+ebp-0xc000]
		mov	ah,[esi+ebp-0xa000]
		mov	dl,[esi+ebp-0x8000]
		mov	dh,[esi+ebp-0x6000]
		mov	ebx,8
; B���[�v
.b_loop:
		mov	ecx,[edi]
		add	al,al
		adc	ecx,ecx
		add	ah,ah
		adc	ecx,ecx
		add	dl,dl
		adc	ecx,ecx
		add	dh,dh
		adc	ecx,ecx
		and	ecx,0x00000fff
; �p���b�g�ϊ�
		mov	ecx,[ecx*4+_rgbAnalogDD]
		mov	[edi],ecx
		add	edi,4
		dec	ebx
		jnz	.b_loop
; ���̃o�C�g��
		pop	ebx
		pop	eax
.next_byte0:
		inc	esi
		add	eax,2
		inc	ebx
		jmp	near .byte_loop
.next_byte2:
		add	edi,32
		jmp	.next_byte0

; �C���^�[���[�X�����X�L�b�v
		align	4
.next_line:
		pop	ebp
		pop	edx
		pop	eax
.next_line2:
		add	edi,[ebp+12]
		add	edi,1280
; ���̃��C����
		add	edi,[ebp+12]
		inc	eax
		dec	edx
		jnz	near .line_loop
; �I��
.exit:
		pop	edi
		pop	esi
		pop	ebx
		pop	ebp
		ret

;
; 320x200�A26���F���[�h
; DirectDraw�����_�����O(555)
;
; static void Render256k(LPVOID lpSurface, LONG lPitch, int first, int last)
; lpSurface	ebp+8
; lPitch	ebp+12
; first		ebp+16
; last		ebp+20
;
		align	4
_Render256k555DD:
		push	ebp
		mov	ebp,esp
		push	ebx
		push	esi
		push	edi
; �T�[�t�F�C�X�ݒ�
		mov	edi,[ebp+8]
		mov	eax,[ebp+12]
		imul	eax,[ebp+16]
		add	edi,eax
		add	edi,eax
; �s�b�`��O�����Čv�Z
		mov	esi,[ebp+12]
		sub	esi,1280
		mov	[ebp+12],esi
; VRAM�A�h���X�ݒ�
		mov	esi,[_vram_c]
		add	esi,0x0000c000
		mov	ecx,[ebp+16]
		mov	eax,ecx
		imul	ecx,40
		add	esi,ecx
; ���C�����ݒ�
		mov	edx,[ebp+20]
		sub	edx,eax
; �P���C�����[�v
		align	4
.line_loop:
		lea	ebx,[eax*4]
		mov	ecx,40
		push	eax
		and	ebx,0xfffffff0
		push	edx
		lea	ebx,[ebx*4+ebx]
; �P�o�C�g���[�v
		align	4
.byte_loop:
		cmp	byte [_DDDrawFlag + ebx], 0
		je	.next_byte2

		push	ecx
		push	ebx
; R
		test	byte [ebp+24],0x20
		jnz		near .skip_red

		mov	al,[esi-0x4000]
		mov	ah,[esi-0x2000]
		mov	dl,[esi-0x0000]
		mov	dh,[esi+0x2000]
		mov	bl,[esi+0x14000]
		mov	bh,8
; R���[�v
.r_loop:
		xor	ecx,ecx
		add	al,al
		adc	ecx,ecx
		add	ah,ah
		adc	ecx,ecx
		add	dl,dl
		adc	ecx,ecx
		add	dh,dh
		adc	ecx,ecx
		add	bl,bl
		adc	ecx,ecx
		mov	[edi],ecx
		add	edi,4
		dec	bh
		jnz	.r_loop
		sub	edi,32
		jmp near .green

; R SKIP
		align	4
.skip_red:
		mov	bh,8
		xor	ecx,ecx
.skipr_loop:
		mov	[edi],ecx
		add	edi,4
		dec	bh
		jnz	.skipr_loop
		sub	edi,32

; G
		align	4
.green:
		test	byte [ebp+24],0x40
		jnz		near .skip_green

		mov	al,[esi+0x4000]
		mov	ah,[esi+0x6000]
		mov	dl,[esi+0x8000]
		mov	dh,[esi+0xa000]
		mov	bl,[esi+0x1c000]
		mov	bh,8
; G���[�v
.g_loop:
		mov	ecx,[edi]
		add	al,al
		adc	ecx,ecx
		add	ah,ah
		adc	ecx,ecx
		add	dl,dl
		adc	ecx,ecx
		add	dh,dh
		adc	ecx,ecx
		add	bl,bl
		adc	ecx,ecx
		mov	[edi],ecx
		add	edi,4
		dec	bh
		jnz	.g_loop
		sub	edi,32
		jmp	near .blue


; G SKIP
		align	4
.skip_green:
		mov	bh,8
.skipg_loop:
		shl	dword [edi],5
		add	edi,4
		dec	bh
		jnz	.skipg_loop
		sub	edi,32

; B
		align	4
.blue:
		test	byte [ebp+24],0x10
		jnz		near .skip_blue

		mov	al,[esi-0xc000]
		mov	ah,[esi-0xa000]
		mov	dl,[esi-0x8000]
		mov	dh,[esi-0x6000]
		mov	bl,[esi+0xc000]
		mov	bh,8
; B���[�v
.b_loop:
		mov	ecx,[edi]
		add	al,al
		adc	ecx,ecx
		add	ah,ah
		adc	ecx,ecx
		add	dl,dl
		adc	ecx,ecx
		add	dh,dh
		adc	ecx,ecx
		add	bl,bl
		adc	ecx,ecx
; �p���b�g�ϊ�
		and	ecx,0x00007fff
		mov	[edi],cx
		mov	[edi+2],cx
		add	edi,4
		dec	bh
		jnz	.b_loop
		jmp near .next_byte

; B SKIP
		align	4
.skip_blue:
		mov	bh,8
.skipb_loop:
		mov	ecx,[edi]
		shl	ecx,5

; �p���b�g�ϊ�
		and	ecx,0x00007fff
		mov	[edi],cx
		mov	[edi+2],cx
		add	edi,4
		dec	bh
		jnz	.skipb_loop

; ���̃o�C�g��
		align	4
.next_byte:
		pop	ebx
		pop	ecx
.next_byte0:
		inc	esi
		add	ebx,2
		dec	ecx
		jnz	near .byte_loop
		jmp	.next_line
.next_byte2:
		add	edi,32
		jmp	.next_byte0

.next_line:
		pop	edx
		pop	eax
.next_line2:
; �C���^�[���[�X�����X�L�b�v
		add	edi,[ebp+12]
		add	edi,1280
; ���̃��C����
		add	edi,[ebp+12]
		inc	eax
		dec	edx
		jnz	near .line_loop
; �I��
		pop	edi
		pop	esi
		pop	ebx
		pop	ebp
		ret

;
; 320x200�A26���F���[�h
; DirectDraw�����_�����O(565)
;
; static void Render256k(LPVOID lpSurface, LONG lPitch, int first, int last)
; lpSurface	ebp+8
; lPitch	ebp+12
; first		ebp+16
; last		ebp+20
;
		align	4
_Render256k565DD:
		push	ebp
		mov	ebp,esp
		push	ebx
		push	esi
		push	edi
; �T�[�t�F�C�X�ݒ�
		mov	edi,[ebp+8]
		mov	eax,[ebp+12]
		imul	eax,[ebp+16]
		add	edi,eax
		add	edi,eax
; �s�b�`��O�����Čv�Z
		mov	esi,[ebp+12]
		sub	esi,1280
		mov	[ebp+12],esi
; VRAM�A�h���X�ݒ�
		mov	esi,[_vram_c]
		add	esi,0x0000c000
		mov	ecx,[ebp+16]
		mov	eax,ecx
		imul	ecx,40
		add	esi,ecx
; ���C�����ݒ�
		mov	edx,[ebp+20]
		sub	edx,eax
; �P���C�����[�v
		align	4
.line_loop:
		lea	ebx,[eax*4]
		mov	ecx,40
		push	eax
		and	ebx,0xfffffff0
		push	edx
		lea	ebx,[ebx*4+ebx]
; �P�o�C�g���[�v
		align	4
.byte_loop:
		cmp	byte [_DDDrawFlag + ebx], 0
		je	.next_byte2

		push	ecx
		push	ebx

; R
		test	byte [ebp+24],0x20
		jnz		near .skip_red

		mov	al,[esi-0x4000]
		mov	ah,[esi-0x2000]
		mov	dl,[esi-0x0000]
		mov	dh,[esi+0x2000]
		mov	bl,[esi+0x14000]
		mov	bh,8
; R���[�v
.r_loop:
		xor	ecx,ecx
		add	al,al
		adc	ecx,ecx
		add	ah,ah
		adc	ecx,ecx
		add	dl,dl
		adc	ecx,ecx
		add	dh,dh
		adc	ecx,ecx
		add	bl,bl
		adc	ecx,ecx
		mov	[edi],ecx
		add	edi,4
		dec	bh
		jnz	.r_loop
		sub	edi,32
		jmp near .green

; R SKIP
		align	4
.skip_red:
		mov	bh,8
		xor	ecx,ecx
.skipr_loop:
		mov	[edi],ecx
		add	edi,4
		dec	bh
		jnz	.skipr_loop
		sub	edi,32

; G
		align	4
.green:
		test	byte [ebp+24],0x40
		jnz		near .skip_green

		push	ebp
		mov	al,[esi+0x4000]
		mov	ah,[esi+0x6000]
		mov	dl,[esi+0x8000]
		mov	dh,[esi+0xa000]
		mov	bl,[esi+0x1c000]
		mov	bh,[esi+0x1e000]
		mov	ebp,8
; G���[�v
.g_loop:
		mov	ecx,[edi]
		add	al,al
		adc	ecx,ecx
		add	ah,ah
		adc	ecx,ecx
		add	dl,dl
		adc	ecx,ecx
		add	dh,dh
		adc	ecx,ecx
		add	bl,bl
		adc	ecx,ecx
		add	bh,bh
		adc	ecx,ecx
		mov	[edi],ecx
		add	edi,4
		dec	ebp
		jnz	.g_loop
		sub	edi,32
		pop	ebp
		jmp	near .blue

; G SKIP
		align	4
.skip_green:
		mov	bh,8
.skipg_loop:
		shl	dword [edi],6
		add	edi,4
		dec	bh
		jnz	.skipg_loop
		sub	edi,32

; B
		align	4
.blue:
		test	byte [ebp+24],0x10
		jnz		near .skip_blue

		mov	al,[esi-0xc000]
		mov	ah,[esi-0xa000]
		mov	dl,[esi-0x8000]
		mov	dh,[esi-0x6000]
		mov	bl,[esi+0xc000]
		mov	bh,8
; B���[�v
.b_loop:
		mov	ecx,[edi]
		add	al,al
		adc	ecx,ecx
		add	ah,ah
		adc	ecx,ecx
		add	dl,dl
		adc	ecx,ecx
		add	dh,dh
		adc	ecx,ecx
		add	bl,bl
		adc	ecx,ecx
; �p���b�g�ϊ�
		and	ecx,0x0000ffff
		mov	[edi],cx
		mov	[edi+2],cx
		add	edi,4
		dec	bh
		jnz	.b_loop
		jmp near .next_byte

; B SKIP
		align	4
.skip_blue:
		mov	bh,8
.skipb_loop:
		mov	ecx,[edi]
		shl	ecx,5

; �p���b�g�ϊ�
		and	ecx,0x0000ffff
		mov	[edi],cx
		mov	[edi+2],cx
		add	edi,4
		dec	bh
		jnz	.skipb_loop

; ���̃o�C�g��
		align	4
.next_byte:
		pop	ebx
		pop	ecx
.next_byte0:
		inc	esi
		add	ebx,2
		dec	ecx
		jnz	near .byte_loop
		jmp	.next_line
;
.next_byte2:
		add	edi,32
		jmp	.next_byte0

.next_line:
		pop	edx
		pop	eax
; �C���^�[���[�X�����X�L�b�v
.next_line2:
		add	edi,[ebp+12]
		add	edi,1280
; ���̃��C����
		add	edi,[ebp+12]
		inc	eax
		dec	edx
		jnz	near .line_loop
; �I��
		pop	edi
		pop	esi
		pop	ebx
		pop	ebp
		ret

	%endif

	%if XM7_VER == 1
;
; 640x200�A�f�W�^�����[�h (�^��400���C���A�_�v�^)
; DirectDraw�����_�����O
;
; static void Render640(LPVOID lpSurface, LONG lPitch, int first, int last)
; lpSurface	ebp+8
; lPitch	ebp+12
; first		ebp+16
; last		ebp+20
;
		align	4
_Render640mDD:
		push	ebp
		mov	ebp,esp
		push	ebx
		push	esi
		push	edi
; �T�[�t�F�C�X�ݒ�
		mov	edi,[ebp+8]
		mov	eax,[ebp+12]
		imul	eax,[ebp+16]
		add	edi,eax
		add	edi,eax
; �s�b�`��O�����Čv�Z
		mov	esi,[ebp+12]
		sub	esi,1280
		mov	[ebp+12],esi
; VRAM�A�h���X�ݒ�
		mov	esi,[_vram_c]
		add	esi,0x4000
		mov	ecx,[ebp+16]
		mov	eax,ecx
		imul	ecx,80
		add	esi,ecx
; ���C�����ݒ�
		mov	edx,[ebp+20]
		sub	edx,eax
		jz	near .exit
; �P���C�����[�v
		align	4
.line_loop:
		lea	ecx,[eax*4]
		push	eax
		and	ecx,0xfffffff0
		push	edx
		lea	ecx,[ecx*4+ecx]
		mov	ebx,80
; �P�o�C�g���[�v
		align	4
.byte_loop:
		cmp	byte [_DDDrawFlag + ecx], 0
		je	.next_byte

		push	ecx
;
		mov	al,[esi+0x4000]
		mov	ah,[esi]
		mov	dh,[esi-0x4000]
		push	eax
		push	edx
; �������C��
; bit7
		xor	ecx,ecx
		add	al,al
		adc	ecx,ecx
		add	ah,ah
		adc	ecx,ecx
		add	dh,dh
		adc	ecx,ecx
		mov	cx,[_rgbTTLDD+ecx*4]
		mov	[edi],cx
; bit6
		xor	ecx,ecx
		add	al,al
		adc	ecx,ecx
		add	ah,ah
		adc	ecx,ecx
		add	dh,dh
		adc	ecx,ecx
		mov	cx,[_rgbTTLDD+ecx*4]
		mov	[edi+2],cx
; bit5
		xor	ecx,ecx
		add	al,al
		adc	ecx,ecx
		add	ah,ah
		adc	ecx,ecx
		add	dh,dh
		adc	ecx,ecx
		mov	cx,[_rgbTTLDD+ecx*4]
		mov	[edi+4],cx
; bit4
		xor	ecx,ecx
		add	al,al
		adc	ecx,ecx
		add	ah,ah
		adc	ecx,ecx
		add	dh,dh
		adc	ecx,ecx
		mov	cx,[_rgbTTLDD+ecx*4]
		mov	[edi+6],cx
; bit3
		xor	ecx,ecx
		add	al,al
		adc	ecx,ecx
		add	ah,ah
		adc	ecx,ecx
		add	dh,dh
		adc	ecx,ecx
		mov	cx,[_rgbTTLDD+ecx*4]
		mov	[edi+8],cx
; bit2
		xor	ecx,ecx
		add	al,al
		adc	ecx,ecx
		add	ah,ah
		adc	ecx,ecx
		add	dh,dh
		adc	ecx,ecx
		mov	cx,[_rgbTTLDD+ecx*4]
		mov	[edi+10],cx
; bit1
		xor	ecx,ecx
		add	al,al
		adc	ecx,ecx
		add	ah,ah
		adc	ecx,ecx
		add	dh,dh
		adc	ecx,ecx
		mov	cx,[_rgbTTLDD+ecx*4]
		mov	[edi+12],cx
; bit0
		xor	ecx,ecx
		add	al,al
		adc	ecx,ecx
		add	ah,ah
		adc	ecx,ecx
		add	dh,dh
		adc	ecx,ecx
		mov	cx,[_rgbTTLDD+ecx*4]
		mov	[edi+14],cx
;
; ����C��
		add	edi, 1280
		add	edi, [ebp+12]
		pop	edx
		pop	eax
; bit7
		xor	ecx,ecx
		add	al,al
		adc	ecx,ecx
		add	ah,ah
		adc	ecx,ecx
		add	dh,dh
		adc	ecx,ecx
		mov	cx,[_rgbTTLDD+ecx*4+32]
		mov	[edi],cx
; bit6
		xor	ecx,ecx
		add	al,al
		adc	ecx,ecx
		add	ah,ah
		adc	ecx,ecx
		add	dh,dh
		adc	ecx,ecx
		mov	cx,[_rgbTTLDD+ecx*4+32]
		mov	[edi+2],cx
; bit5
		xor	ecx,ecx
		add	al,al
		adc	ecx,ecx
		add	ah,ah
		adc	ecx,ecx
		add	dh,dh
		adc	ecx,ecx
		mov	cx,[_rgbTTLDD+ecx*4+32]
		mov	[edi+4],cx
; bit4
		xor	ecx,ecx
		add	al,al
		adc	ecx,ecx
		add	ah,ah
		adc	ecx,ecx
		add	dh,dh
		adc	ecx,ecx
		mov	cx,[_rgbTTLDD+ecx*4+32]
		mov	[edi+6],cx
; bit3
		xor	ecx,ecx
		add	al,al
		adc	ecx,ecx
		add	ah,ah
		adc	ecx,ecx
		add	dh,dh
		adc	ecx,ecx
		mov	cx,[_rgbTTLDD+ecx*4+32]
		mov	[edi+8],cx
; bit2
		xor	ecx,ecx
		add	al,al
		adc	ecx,ecx
		add	ah,ah
		adc	ecx,ecx
		add	dh,dh
		adc	ecx,ecx
		mov	cx,[_rgbTTLDD+ecx*4+32]
		mov	[edi+10],cx
; bit1
		xor	ecx,ecx
		add	al,al
		adc	ecx,ecx
		add	ah,ah
		adc	ecx,ecx
		add	dh,dh
		adc	ecx,ecx
		mov	cx,[_rgbTTLDD+ecx*4+32]
		mov	[edi+12],cx
; bit0
		xor	ecx,ecx
		add	al,al
		adc	ecx,ecx
		add	ah,ah
		adc	ecx,ecx
		add	dh,dh
		adc	ecx,ecx
		mov	cx,[_rgbTTLDD+ecx*4+32]
		mov	[edi+14],cx
;
		sub	edi, 1280
		sub	edi, [ebp+12]
; ���̃o�C�g��
		pop	ecx
.next_byte:
		add	edi,16
		inc	esi
		inc	ecx
		dec	ebx
		jnz	near .byte_loop

; ���̃��C����
		pop	edx
		pop	eax
; �C���^�[���[�X�����X�L�b�v
.next_line:
		add	edi,[ebp+12]
		add	edi,1280
		add	edi,[ebp+12]
		inc	eax
		dec	edx
		jnz	near .line_loop
; �I��
.exit:
		pop	edi
		pop	esi
		pop	ebx
		pop	ebp
		ret

	%endif

	%if	XM7_VER >= 2

; 640x200�A�f�W�^�����[�h (2�o���N�^��400���C��)
; DirectDraw�����_�����O
;
; static void Render640(LPVOID lpSurface, LONG lPitch, int first, int last)
; lpSurface	ebp+8
; lPitch	ebp+12
; first		ebp+16
; last		ebp+20
;
		align	4
_Render640cDD:
		push	ebp
		mov	ebp,esp
		push	ebx
		push	esi
		push	edi
; �T�[�t�F�C�X�ݒ�
		mov	edi,[ebp+8]
		mov	eax,[ebp+12]
		imul	eax,[ebp+16]
		add	edi,eax
		add	edi,eax
; �s�b�`��O�����Čv�Z
		mov	esi,[ebp+12]
		sub	esi,1280
		mov	[ebp+12],esi
; VRAM�A�h���X�ݒ�
	%if XM7_VER >= 3
		mov	esi,[_vram_dblk]
	%else
		mov	esi,[_vram_c]
	%endif
		mov	ecx,[ebp+16]
		mov	eax,ecx
		imul	ecx,80
		add	esi,ecx
; ���C�����ݒ�
		mov	edx,[ebp+20]
		sub	edx,eax
		jz	near .exit
; �P���C�����[�v
		align	4
.line_loop:
		lea	ecx,[eax*4]
		push	eax
		and	ecx,0xfffffff0
		push	edx
		lea	ecx,[ecx*4+ecx]
		mov	ebx,80
; �P�o�C�g���[�v
		align	4
.byte_loop:
		cmp	byte [_DDDrawFlag + ecx], 0
		je	.next_byte

		push	ecx
;
	%if	XM7_VER >= 3
		mov	al,[esi+0x10000]
		mov	ah,[esi+0x8000]
		mov	dh,[esi]
	%else
		mov	al,[esi+0x8000]
		mov	ah,[esi+0x4000]
		mov	dh,[esi]
	%endif
; bit7
		xor	ecx,ecx
		add	al,al
		adc	ecx,ecx
		add	ah,ah
		adc	ecx,ecx
		add	dh,dh
		adc	ecx,ecx
		mov	cx,[_rgbTTLDD+ecx*4]
		mov	[edi],cx
; bit6
		xor	ecx,ecx
		add	al,al
		adc	ecx,ecx
		add	ah,ah
		adc	ecx,ecx
		add	dh,dh
		adc	ecx,ecx
		mov	cx,[_rgbTTLDD+ecx*4]
		mov	[edi+2],cx
; bit5
		xor	ecx,ecx
		add	al,al
		adc	ecx,ecx
		add	ah,ah
		adc	ecx,ecx
		add	dh,dh
		adc	ecx,ecx
		mov	cx,[_rgbTTLDD+ecx*4]
		mov	[edi+4],cx
; bit4
		xor	ecx,ecx
		add	al,al
		adc	ecx,ecx
		add	ah,ah
		adc	ecx,ecx
		add	dh,dh
		adc	ecx,ecx
		mov	cx,[_rgbTTLDD+ecx*4]
		mov	[edi+6],cx
; bit3
		xor	ecx,ecx
		add	al,al
		adc	ecx,ecx
		add	ah,ah
		adc	ecx,ecx
		add	dh,dh
		adc	ecx,ecx
		mov	cx,[_rgbTTLDD+ecx*4]
		mov	[edi+8],cx
; bit2
		xor	ecx,ecx
		add	al,al
		adc	ecx,ecx
		add	ah,ah
		adc	ecx,ecx
		add	dh,dh
		adc	ecx,ecx
		mov	cx,[_rgbTTLDD+ecx*4]
		mov	[edi+10],cx
; bit1
		xor	ecx,ecx
		add	al,al
		adc	ecx,ecx
		add	ah,ah
		adc	ecx,ecx
		add	dh,dh
		adc	ecx,ecx
		mov	cx,[_rgbTTLDD+ecx*4]
		mov	[edi+12],cx
; bit0
		xor	ecx,ecx
		add	al,al
		adc	ecx,ecx
		add	ah,ah
		adc	ecx,ecx
		add	dh,dh
		adc	ecx,ecx
		mov	cx,[_rgbTTLDD+ecx*4]
		mov	[edi+14],cx
;
		add	edi, 1280
		add	edi, [ebp+12]
	%if	XM7_VER >= 3
		mov	al,[esi+0x14000]
		mov	ah,[esi+0xc000]
		mov	dh,[esi+0x4000]
	%else
		mov	al,[esi+0x14000]
		mov	ah,[esi+0x10000]
		mov	dh,[esi+0xc000]
	%endif
; bit7
		xor	ecx,ecx
		add	al,al
		adc	ecx,ecx
		add	ah,ah
		adc	ecx,ecx
		add	dh,dh
		adc	ecx,ecx
		mov	cx,[_rgbTTLDD+ecx*4]
		mov	[edi],cx
; bit6
		xor	ecx,ecx
		add	al,al
		adc	ecx,ecx
		add	ah,ah
		adc	ecx,ecx
		add	dh,dh
		adc	ecx,ecx
		mov	cx,[_rgbTTLDD+ecx*4]
		mov	[edi+2],cx
; bit5
		xor	ecx,ecx
		add	al,al
		adc	ecx,ecx
		add	ah,ah
		adc	ecx,ecx
		add	dh,dh
		adc	ecx,ecx
		mov	cx,[_rgbTTLDD+ecx*4]
		mov	[edi+4],cx
; bit4
		xor	ecx,ecx
		add	al,al
		adc	ecx,ecx
		add	ah,ah
		adc	ecx,ecx
		add	dh,dh
		adc	ecx,ecx
		mov	cx,[_rgbTTLDD+ecx*4]
		mov	[edi+6],cx
; bit3
		xor	ecx,ecx
		add	al,al
		adc	ecx,ecx
		add	ah,ah
		adc	ecx,ecx
		add	dh,dh
		adc	ecx,ecx
		mov	cx,[_rgbTTLDD+ecx*4]
		mov	[edi+8],cx
; bit2
		xor	ecx,ecx
		add	al,al
		adc	ecx,ecx
		add	ah,ah
		adc	ecx,ecx
		add	dh,dh
		adc	ecx,ecx
		mov	cx,[_rgbTTLDD+ecx*4]
		mov	[edi+10],cx
; bit1
		xor	ecx,ecx
		add	al,al
		adc	ecx,ecx
		add	ah,ah
		adc	ecx,ecx
		add	dh,dh
		adc	ecx,ecx
		mov	cx,[_rgbTTLDD+ecx*4]
		mov	[edi+12],cx
; bit0
		xor	ecx,ecx
		add	al,al
		adc	ecx,ecx
		add	ah,ah
		adc	ecx,ecx
		add	dh,dh
		adc	ecx,ecx
		mov	cx,[_rgbTTLDD+ecx*4]
		mov	[edi+14],cx
;
		sub	edi, 1280
		sub	edi, [ebp+12]
; ���̃o�C�g��
		pop	ecx
.next_byte:
		add	edi,16
		inc	esi
		inc	ecx
		dec	ebx
		jnz	near .byte_loop

; ���̃��C����
		pop	edx
		pop	eax
; �C���^�[���[�X�����X�L�b�v
.next_line:
		add	edi,[ebp+12]
		add	edi,1280
		add	edi,[ebp+12]
		inc	eax
		dec	edx
		jnz	near .line_loop
; �I��
.exit:
		pop	edi
		pop	esi
		pop	ebx
		pop	ebp
		ret

	%endif

	%if	XM7_VER >= 3

;
; 640x200�A�f�W�^�����[�h (2�o���N�^��400���C��)
; DirectDraw�����_�����O(Window)
;
; static void Render640w(LPVOID lpSurface, LONG lPitch,
;			int first, int last, int firstx, int lastx)
; lpSurface	ebp+8
; lPitch	ebp+12
; first		ebp+16
; last		ebp+20
;
		align	4
_Render640cwDD:
		push	ebp
		mov	ebp,esp
		push	ebx
		push	esi
		push	edi
; �T�[�t�F�C�X�ݒ�
		mov	edi,[ebp+8]
		mov	eax,[ebp+12]
		imul	eax,[ebp+16]
		add	edi,eax
		add	edi,eax
; �s�b�`��O�����Čv�Z
		mov	esi,[ebp+12]
		sub	esi,1280
		mov	[ebp+12],esi
; VRAM�A�h���X�ݒ�
		mov	esi,0x8000
		mov	ecx,[ebp+16]
		mov	eax,ecx
		imul	ecx,80
		add	esi,ecx
; ���C�����ݒ�
		mov	edx,[ebp+20]
		sub	edx,eax
		jz	near .exit
; �E�B���h�E�؂�ւ��|�C���^�ݒ�
		shr	dword [ebp+24],3
		shr	dword [ebp+28],3
		mov	cl,[ebp+24]
		mov	ch,[ebp+28]
; �P���C�����[�v
		align	4
.line_loop:
		push	eax
		push	edx
		shl	eax,2
		push	ebp
		xor	ebx,ebx
		and	eax,0xfffffff0
		mov	ebp,[_vram_dblk]
		lea	eax,[eax*4+eax]
; �P�o�C�g���[�v
		align	4
.byte_loop:
		cmp	ebx,80
		jae	near .next_line

		cmp	bl,cl
		jz	.change_dbank2
		cmp	bl,ch
		jnz	.draw_byte
.change_dbank1:
		mov	ebp,[_vram_dblk]
		jmp	.draw_byte
.change_dbank2:
		mov	ebp,[_vram_bdblk]

.draw_byte:
		cmp	dword [_DDDrawFlag + eax], 0
		je	.next_byte

		push	eax
		push	ebx
		mov	al,[esi+ebp+0x8000]
		mov	ah,[esi+ebp]
		mov	dl,[esi+ebp-0x8000]
; bit 7
		xor	ebx,ebx
		add	al,al
		adc	ebx,ebx
		add	ah,ah
		adc	ebx,ebx
		add	dl,dl
		adc	ebx,ebx
		mov	bx,[ebx*4+_rgbTTLDD]
		mov	[edi],bx
; bit 6
		xor	ebx,ebx
		add	al,al
		adc	ebx,ebx
		add	ah,ah
		adc	ebx,ebx
		add	dl,dl
		adc	ebx,ebx
		mov	bx,[ebx*4+_rgbTTLDD]
		mov	[edi+2],bx
; bit 5
		xor	ebx,ebx
		add	al,al
		adc	ebx,ebx
		add	ah,ah
		adc	ebx,ebx
		add	dl,dl
		adc	ebx,ebx
		mov	bx,[ebx*4+_rgbTTLDD]
		mov	[edi+4],bx
; bit 4
		xor	ebx,ebx
		add	al,al
		adc	ebx,ebx
		add	ah,ah
		adc	ebx,ebx
		add	dl,dl
		adc	ebx,ebx
		mov	bx,[ebx*4+_rgbTTLDD]
		mov	[edi+6],bx
; bit 3
		xor	ebx,ebx
		add	al,al
		adc	ebx,ebx
		add	ah,ah
		adc	ebx,ebx
		add	dl,dl
		adc	ebx,ebx
		mov	bx,[ebx*4+_rgbTTLDD]
		mov	[edi+8],bx
; bit 2
		xor	ebx,ebx
		add	al,al
		adc	ebx,ebx
		add	ah,ah
		adc	ebx,ebx
		add	dl,dl
		adc	ebx,ebx
		mov	bx,[ebx*4+_rgbTTLDD]
		mov	[edi+10],bx
; bit 1
		xor	ebx,ebx
		add	al,al
		adc	ebx,ebx
		add	ah,ah
		adc	ebx,ebx
		add	dl,dl
		adc	ebx,ebx
		mov	bx,[ebx*4+_rgbTTLDD]
		mov	[edi+12],bx
; bit 0
		xor	ebx,ebx
		add	al,al
		adc	ebx,ebx
		add	ah,ah
		adc	ebx,ebx
		add	dl,dl
		adc	ebx,ebx
		mov	bx,[ebx*4+_rgbTTLDD]
		mov	[edi+14],bx
;
		mov	al,[esi+ebp+0xc000]
		mov	ah,[esi+ebp+0x4000]
		mov	dl,[esi+ebp-0x4000]
		add	edi, 1280
		add	edi, [esp+12+32]
; bit 7
		xor	ebx,ebx
		add	al,al
		adc	ebx,ebx
		add	ah,ah
		adc	ebx,ebx
		add	dl,dl
		adc	ebx,ebx
		mov	bx,[ebx*4+_rgbTTLDD]
		mov	[edi],bx
; bit 6
		xor	ebx,ebx
		add	al,al
		adc	ebx,ebx
		add	ah,ah
		adc	ebx,ebx
		add	dl,dl
		adc	ebx,ebx
		mov	bx,[ebx*4+_rgbTTLDD]
		mov	[edi+2],bx
; bit 5
		xor	ebx,ebx
		add	al,al
		adc	ebx,ebx
		add	ah,ah
		adc	ebx,ebx
		add	dl,dl
		adc	ebx,ebx
		mov	bx,[ebx*4+_rgbTTLDD]
		mov	[edi+4],bx
; bit 4
		xor	ebx,ebx
		add	al,al
		adc	ebx,ebx
		add	ah,ah
		adc	ebx,ebx
		add	dl,dl
		adc	ebx,ebx
		mov	bx,[ebx*4+_rgbTTLDD]
		mov	[edi+6],bx
; bit 3
		xor	ebx,ebx
		add	al,al
		adc	ebx,ebx
		add	ah,ah
		adc	ebx,ebx
		add	dl,dl
		adc	ebx,ebx
		mov	bx,[ebx*4+_rgbTTLDD]
		mov	[edi+8],bx
; bit 2
		xor	ebx,ebx
		add	al,al
		adc	ebx,ebx
		add	ah,ah
		adc	ebx,ebx
		add	dl,dl
		adc	ebx,ebx
		mov	bx,[ebx*4+_rgbTTLDD]
		mov	[edi+10],bx
; bit 1
		xor	ebx,ebx
		add	al,al
		adc	ebx,ebx
		add	ah,ah
		adc	ebx,ebx
		add	dl,dl
		adc	ebx,ebx
		mov	bx,[ebx*4+_rgbTTLDD]
		mov	[edi+12],bx
; bit 0
		xor	ebx,ebx
		add	al,al
		adc	ebx,ebx
		add	ah,ah
		adc	ebx,ebx
		add	dl,dl
		adc	ebx,ebx
		mov	bx,[ebx*4+_rgbTTLDD]
		mov	[edi+14],bx
;
		sub	edi, 1280
		sub	edi, [esp+12+32]
; ���̃o�C�g��
		pop	ebx
		pop	eax
.next_byte:
		inc	esi
		add	edi,16
		inc	eax
		inc	ebx
		jmp	near .byte_loop

; �C���^�[���[�X�����X�L�b�v
		align	4
.next_line:
		pop	ebp
		pop	edx
		pop	eax
.next_line2:
; ���̃��C����
		add	edi,[ebp+12]
		add	edi, 1280
		add	edi,[ebp+12]
		inc	eax
		dec	edx
		jnz	near .line_loop
; �I��
.exit:
		pop	edi
		pop	esi
		pop	ebx
		pop	ebp
		ret

	%endif

;
; �v���O�����I��
;
		end
