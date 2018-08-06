;
; FM-7 EMULATOR "XM7"
;
; Copyright (C) 1999-2017 �o�h�D(Twitter:@xm6_original)
; Copyright (C) 2001-2017 Ryu Takegami (Twitter:@RyuTakegami)
;
; [ Win32API �����_�����O(400line) ]
;
; RHG����
;	2014.03.16		32�r�b�g�J���[�����_����ǉ�
;

	%ifdef L4CARD
	%if XM7_VER == 1

;
; �O����`
;
		section	.data class=DATA align=4 use32
		extern	_vram_c
		extern	_vram_offset
		extern	_pBitsGDI
		extern	_rgbTTLL4DD
		extern	_GDIDrawFlag
		extern	_DDDrawFlag

		section	.text class=CODE align=16 use32
		global	_RenderL4GDI
		global	_RenderL4DD
		global	_RenderL4Tc32DD

;
; GDI�`��p�W�J�σr�b�g�p�^�[��
;
fntptn:
		dd	00000000h,01000000h,00010000h,01010000h
		dd	00000100h,01000100h,00010100h,01010100h
		dd	00000001h,01000001h,00010001h,01010001h
		dd	00000101h,01000101h,00010101h,01010101h

;
; 640x400�A�P�F���[�h
; GDI�����_�����O
;
; static void RenderL4(int first, int last)
; first		ebp+8
; last		ebp+12
;
		align	4
_RenderL4GDI:
		push	ebp
		mov	ebp,esp
		push	ebx
		push	esi
		push	edi
; �����|�C���^�A�J�E���^���v�Z
		movzx	eax,word [_vram_offset]
		mov	esi,[ebp+8]
		imul	esi,80
		add	esi,eax
		and	esi,7fffh
		mov	edi,[_pBitsGDI]
		mov	ecx,[ebp+8]
		mov	eax,ecx
		imul	eax,10
		shl	eax,6
		add	edi,eax
		mov	edx,[ebp+12]
		sub	edx,ecx
		jz	near .exit
; �P���C�����[�v
		align	4
.line_loop:
		lea	ebx,[ecx*2]
		mov	ebp,80
		and	ebx,0xfffffff0
		push	edx
		push	ecx
		mov	edx,[_vram_c]
		lea	ebx,[ebx*4+ebx]
		xor	ecx,ecx
; �P�o�C�g���[�v
		align	4
.byte_loop:
		cmp	byte [_GDIDrawFlag + ebx], 0
		je	.next_byte

		push	ebx
;
		movzx	ebx,byte [edx+esi]
		cmp	esi,4000h
		mov	eax,ebx
		setb	cl
		shr	ebx,4
		and	eax,15
		mov	ebx,[fntptn+ebx*4]
		mov	eax,[fntptn+eax*4]
		shl	ebx,cl
		shl	eax,cl
; ��������
.skip:
		mov	[edi],ebx
		mov	[edi+4],eax
; ���̃o�C�g��
		pop	ebx
.next_byte:
		inc	esi
		and	esi,07fffh
		add	edi,8
		inc	ebx
		dec	ebp
		jnz	near .byte_loop
; ���̃��C����
		pop	ecx
		pop	edx
.next_line:
		inc	ecx
		dec	edx
		jnz	near .line_loop
		jmp	near .exit
; �I��
.exit:
		pop	edi
		pop	esi
		pop	ebx
		pop	ebp
		ret

;
; 640x400�A�P�F���[�h
; DirectDraw�����_�����O
;
; static void RenderL4(LPVOID lpSurface, LONG lPitch, int first, int last)
; lpSurface	ebp+8
; lPitch	ebp+12
; first		ebp+16
; last		ebp+20
;
		align	4
_RenderL4DD:
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
; �s�b�`��O�����Čv�Z
		mov	esi,[ebp+12]
		sub	esi,1280
		mov	[ebp+12],esi
; VRAM�A�h���X�ݒ�
		mov	esi,[_vram_offset]
		mov	ecx,[ebp+16]
		mov	eax,ecx
		imul	ecx,80
		add	esi,ecx
		and	esi,7fffh
; ���C�����ݒ�
		mov	edx,[ebp+20]
		sub	edx,eax
		jz	near .exit
; �P���C�����[�v
		align	4
.line_loop:
		lea	ecx,[eax*2]
		push	eax
		and	ecx,0xfffffff0
		push	edx
		lea	ecx,[ecx*4+ecx]
		mov	ebx,80
		mov	edx,[_vram_c]
; �P�o�C�g���[�v
		align	4
.byte_loop:
		cmp	byte [_DDDrawFlag + ecx], 0
		je	.next_byte

		push	ecx
;
		mov	ch,[esi+edx]
		cmp	esi,4000h
		setb	cl
; bit7
		xor	eax,eax
		add	ch,ch
		adc	al,al
		shl	al,cl
		mov	ax,[_rgbTTLL4DD+eax*4]
		mov	[edi],ax
; bit6
		xor	eax,eax
		add	ch,ch
		adc	al,al
		shl	al,cl
		mov	ax,[_rgbTTLL4DD+eax*4]
		mov	[edi+2],ax
; bit5
		xor	eax,eax
		add	ch,ch
		adc	al,al
		shl	al,cl
		mov	ax,[_rgbTTLL4DD+eax*4]
		mov	[edi+4],ax
; bit4
		xor	eax,eax
		add	ch,ch
		adc	al,al
		shl	al,cl
		mov	ax,[_rgbTTLL4DD+eax*4]
		mov	[edi+6],ax
; bit3
		xor	eax,eax
		add	ch,ch
		adc	al,al
		shl	al,cl
		mov	ax,[_rgbTTLL4DD+eax*4]
		mov	[edi+8],ax
; bit2
		xor	eax,eax
		add	ch,ch
		adc	al,al
		shl	al,cl
		mov	ax,[_rgbTTLL4DD+eax*4]
		mov	[edi+10],ax
; bit1
		xor	eax,eax
		add	ch,ch
		adc	al,al
		shl	al,cl
		mov	ax,[_rgbTTLL4DD+eax*4]
		mov	[edi+12],ax
; bit0
		xor	eax,eax
		add	ch,ch
		adc	al,al
		shl	al,cl
		mov	ax,[_rgbTTLL4DD+eax*4]
		mov	[edi+14],ax
; ���̃o�C�g��
		pop	ecx
.next_byte:
		add	edi,16
		inc	esi
		and	esi,7fffh
		inc	ecx
		dec	ebx
		jnz	near .byte_loop

; ���̃��C����
		pop	edx
		pop	eax
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
; 640x400�A�P�F���[�h
; DirectDraw�����_�����O
;
; static void RenderL4(LPVOID lpSurface, LONG lPitch, int first, int last)
; lpSurface	ebp+8
; lPitch	ebp+12
; first		ebp+16
; last		ebp+20
;
		align	4
_RenderL4Tc32DD:
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
; �s�b�`��O�����Čv�Z
		mov	esi,[ebp+12]
		sub	esi,2560
		mov	[ebp+12],esi
; VRAM�A�h���X�ݒ�
		mov	esi,[_vram_offset]
		mov	ecx,[ebp+16]
		mov	eax,ecx
		imul	ecx,80
		add	esi,ecx
		and	esi,7fffh
; ���C�����ݒ�
		mov	edx,[ebp+20]
		sub	edx,eax
		jz	near .exit
; �P���C�����[�v
		align	4
.line_loop:
		lea	ecx,[eax*2]
		push	eax
		and	ecx,0xfffffff0
		push	edx
		lea	ecx,[ecx*4+ecx]
		mov	ebx,80
		mov	edx,[_vram_c]
; �P�o�C�g���[�v
		align	4
.byte_loop:
		cmp	byte [_DDDrawFlag + ecx], 0
		je	.next_byte

		push	ecx
;
		mov	ch,[esi+edx]
		cmp	esi,4000h
		setb	cl
; bit7
		xor	eax,eax
		add	ch,ch
		adc	al,al
		shl	al,cl
		mov	eax,[_rgbTTLL4DD+eax*4]
		mov	[edi],eax
; bit6
		xor	eax,eax
		add	ch,ch
		adc	al,al
		shl	al,cl
		mov	eax,[_rgbTTLL4DD+eax*4]
		mov	[edi+4],eax
; bit5
		xor	eax,eax
		add	ch,ch
		adc	al,al
		shl	al,cl
		mov	eax,[_rgbTTLL4DD+eax*4]
		mov	[edi+8],eax
; bit4
		xor	eax,eax
		add	ch,ch
		adc	al,al
		shl	al,cl
		mov	eax,[_rgbTTLL4DD+eax*4]
		mov	[edi+12],eax
; bit3
		xor	eax,eax
		add	ch,ch
		adc	al,al
		shl	al,cl
		mov	eax,[_rgbTTLL4DD+eax*4]
		mov	[edi+16],eax
; bit2
		xor	eax,eax
		add	ch,ch
		adc	al,al
		shl	al,cl
		mov	eax,[_rgbTTLL4DD+eax*4]
		mov	[edi+20],eax
; bit1
		xor	eax,eax
		add	ch,ch
		adc	al,al
		shl	al,cl
		mov	eax,[_rgbTTLL4DD+eax*4]
		mov	[edi+24],eax
; bit0
		xor	eax,eax
		add	ch,ch
		adc	al,al
		shl	al,cl
		mov	eax,[_rgbTTLL4DD+eax*4]
		mov	[edi+28],eax
; ���̃o�C�g��
		pop	ecx
.next_byte:
		add	edi,32
		inc	esi
		and	esi,7fffh
		inc	ecx
		dec	ebx
		jnz	near .byte_loop

; ���̃��C����
		pop	edx
		pop	eax
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

	%endif
	%endif

