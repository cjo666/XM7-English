;
; FM-7 EMULATOR "XM7"
;
; Copyright (C) 1999-2017 ＰＩ．(Twitter:@xm6_original)
; Copyright (C) 2001-2017 Ryu Takegami (Twitter:@RyuTakegami)
;
; [ Win32API レンダリング(GDI) ]
;
; RHG履歴
;	2002.05.31		4096色レンダラのメモリライトを4バイト単位で行うよう変更
;	2002.06.20		8色モードのレンダリングを4bppで行うように変更
;	2002.06.21		更新が必要な部分だけレンダリングするように変更
;	2003.02.11		V3用8色モードレンダラ(200line/400line)を統合
;	2004.02.09		疑似400ライン系レンダラを追加
;	2008.01.20		↑なかったことにした(ぉ
;	2010.01.13		TrueColor時の輝度変換をテーブル化
;	2012.12.01		NASM2系でもアセンブルが通るように修正
;	2013.07.12		V3用4096色レンダリング処理を32bppで行うように変更
;	2013.08.22		V3用4096色32bppレンダラをラスタ単位レンダリング時のみ使用
;					するよう変更;

;
; 外部定義
;
	%if	XM7_VER >= 3
		section	.data class=DATA align=4 use32
		extern	_vram_c
		extern	_vram_dptr
		extern	_vram_bdptr
		extern	_vram_dblk
		extern	_vram_bdblk
		extern	_vram_offset
		extern	_crt_flag
		extern	_pBitsGDI
		extern	_rgbTTLGDI
		extern	_rgbAnalogGDI
		extern	_GDIDrawFlag
		extern	_truecolorbrightness

		section	.text class=CODE align=16 use32
		global	_Render640GDI2
		global	_Render640wGDI2
		global	_Render640cGDI
		global	_Render640cwGDI
		global	_Render320GDI
		global	_Render320wGDI
		global	_Render320GDI32bpp
		global	_Render320wGDI32bpp
		global	_Render256kGDI
	%elif XM7_VER >= 2
		section	.data class=DATA align=4 use32
		extern	_vram_c
		extern	_vram_dptr
		extern	_vram_offset
		extern	_crt_flag
		extern	_pBitsGDI
		extern	_rgbTTLGDI
		extern	_rgbAnalogGDI
		extern	_GDIDrawFlag

		section	.text class=CODE align=16 use32
		global	_Render640GDI
		global	_Render640cGDI
		global	_Render320GDI
	%else
		section	.data class=DATA align=4 use32
		extern	_vram_c
		extern	_vram_offset
		extern	_crt_flag
		extern	_pBitsGDI
		extern	_rgbTTLGDI
		extern	_GDIDrawFlag

		section	.text class=CODE align=16 use32
		global	_Render640GDI
		global	_Render640mGDI
	%endif

	%if XM7_VER <= 2

;
; 640x200、デジタルモード
; GDIレンダリング
;
; static void Render640(int first, int last)
; first		ebp+8
; last		ebp+12
;
		align	4
_Render640GDI:
		push	ebp
		mov	ebp,esp
		push	ebx
		push	esi
		push	edi
; 初期ポインタ、カウンタを計算
	%if XM7_VER >= 2
		mov	esi,[_vram_dptr]
	%else
		mov	esi,[_vram_c]
	%endif
		mov	eax,[ebp+8]
		imul	eax,80
		add	esi,eax
		mov	edi,[_pBitsGDI]
		mov	ecx,[ebp+8]
		mov	eax,ecx
		imul	eax,10
		shl	eax,6
		add	edi,eax
		mov	edx,[ebp+12]
		sub	edx,ecx
		jz	near .exit
; １ラインループ
		align	4
.line_loop:
		lea	ebx,[ecx*4]
		mov	ebp,80
		and	ebx,0xfffffff0
		push	edx
		lea	ebx,[ebx*4+ebx]
; １バイトループ
		align	4
.byte_loop:
		cmp	byte [_GDIDrawFlag + ebx], 0
		je	.next_byte

		push	ebx

		cmp	byte [_crt_flag], 0
		jne	.bytedraw
		xor	ebx,ebx
		jmp	.bytewrite

		align	4
.bytedraw:
		mov	al,[esi+0x8000]
		mov	ah,[esi+0x4000]
		mov	dl,[esi]
; bit 7
		xor	ebx,ebx
		add	al,al
		adc	ebx,ebx
		add	ah,ah
		adc	ebx,ebx
		add	dl,dl
		adc	ebx,ebx
; bit 6
		add	ebx,ebx
		add	al,al
		adc	ebx,ebx
		add	ah,ah
		adc	ebx,ebx
		add	dl,dl
		adc	ebx,ebx
; bit 5
		add	ebx,ebx
		add	al,al
		adc	ebx,ebx
		add	ah,ah
		adc	ebx,ebx
		add	dl,dl
		adc	ebx,ebx
; bit 4
		add	ebx,ebx
		add	al,al
		adc	ebx,ebx
		add	ah,ah
		adc	ebx,ebx
		add	dl,dl
		adc	ebx,ebx
; bit 3
		add	ebx,ebx
		add	al,al
		adc	ebx,ebx
		add	ah,ah
		adc	ebx,ebx
		add	dl,dl
		adc	ebx,ebx
; bit 2
		add	ebx,ebx
		add	al,al
		adc	ebx,ebx
		add	ah,ah
		adc	ebx,ebx
		add	dl,dl
		adc	ebx,ebx
; bit 1
		add	ebx,ebx
		add	al,al
		adc	ebx,ebx
		add	ah,ah
		adc	ebx,ebx
		add	dl,dl
		adc	ebx,ebx
; bit 0
		add	ebx,ebx
		add	al,al
		adc	ebx,ebx
		add	ah,ah
		adc	ebx,ebx
		add	dl,dl
		adc	ebx,ebx
; 書き込み
.bytewrite:
		bswap	ebx
		mov	[edi],ebx
; 次のバイトへ
		pop	ebx
.next_byte:
		inc	esi
		add	edi,4
		inc	ebx
		dec	ebp
		jnz	near .byte_loop
; 次のラインへ
		pop	edx
		add	edi,320
.next_line:
		inc	ecx
		dec	edx
		jnz	near .line_loop
		jmp	near .exit
; 終了
.exit:
		pop	edi
		pop	esi
		pop	ebx
		pop	ebp
		ret

	%else

;
; 640x200/640x400、デジタルモード
; GDIレンダリング
;
; static void Render640(int first, int last, int pitch, int scale)
; first		ebp+8
; last		ebp+12
;
		align	4
_Render640GDI2:
		push	ebp
		mov	ebp,esp
		push	ebx
		push	esi
		push	edi
; 初期ポインタ、カウンタを計算
		mov	esi,[_vram_dptr]
		mov	eax,[ebp+8]
		imul	eax,80
		add	esi,eax
		mov	edi,[_pBitsGDI]
		mov	ecx,[ebp+8]
		mov	eax,ecx
		imul	eax,[ebp+20]
		lea	eax,[eax*4+eax]
		shl	eax,5
		add	edi,eax
		mov	edx,[ebp+12]
		sub	edx,ecx
		jz	near .exit
; １ラインループ
		align	4
.line_loop:
		mov	ebx,ecx
		imul	ebx,[esp+20+12]
		mov	ebp,80
		and	ebx,0xfffffff0
		push	edx
		lea	ebx,[ebx*4+ebx]
; １バイトループ
		align	4
.byte_loop:
		cmp	byte [_GDIDrawFlag + ebx], 0
		je	.next_byte

		push	ebx

		cmp	byte [_crt_flag], 0
		jne	.bytedraw
		xor	ebx,ebx
		jmp	.bytewrite

		align	4
.bytedraw:
		mov	al,[esi+0x10000]
		mov	ah,[esi+0x8000]
		mov	dl,[esi]
; bit 7
		xor	ebx,ebx
		add	al,al
		adc	ebx,ebx
		add	ah,ah
		adc	ebx,ebx
		add	dl,dl
		adc	ebx,ebx
; bit 6
		add	ebx,ebx
		add	al,al
		adc	ebx,ebx
		add	ah,ah
		adc	ebx,ebx
		add	dl,dl
		adc	ebx,ebx
; bit 5
		add	ebx,ebx
		add	al,al
		adc	ebx,ebx
		add	ah,ah
		adc	ebx,ebx
		add	dl,dl
		adc	ebx,ebx
; bit 4
		add	ebx,ebx
		add	al,al
		adc	ebx,ebx
		add	ah,ah
		adc	ebx,ebx
		add	dl,dl
		adc	ebx,ebx
; bit 3
		add	ebx,ebx
		add	al,al
		adc	ebx,ebx
		add	ah,ah
		adc	ebx,ebx
		add	dl,dl
		adc	ebx,ebx
; bit 2
		add	ebx,ebx
		add	al,al
		adc	ebx,ebx
		add	ah,ah
		adc	ebx,ebx
		add	dl,dl
		adc	ebx,ebx
; bit 1
		add	ebx,ebx
		add	al,al
		adc	ebx,ebx
		add	ah,ah
		adc	ebx,ebx
		add	dl,dl
		adc	ebx,ebx
; bit 0
		add	ebx,ebx
		add	al,al
		adc	ebx,ebx
		add	ah,ah
		adc	ebx,ebx
		add	dl,dl
		adc	ebx,ebx
; 書き込み
.bytewrite:
		bswap	ebx
		mov	[edi],ebx
; 次のバイトへ
		pop	ebx
.next_byte:
		inc	esi
		add	edi,4
		inc	ebx
		dec	ebp
		jnz	near .byte_loop
; 次のラインへ
		pop	edx
		add	edi,[esp+16+12]
.next_line:
		inc	ecx
		dec	edx
		jnz	near .line_loop
		jmp	near .exit
; 終了
.exit:
		pop	edi
		pop	esi
		pop	ebx
		pop	ebp
		ret

;
; 640x200/640x400、デジタルモード
; GDIレンダリング(Window)
;
; static void Render640w(int first, int last,int firstx, int lastx)
; first		ebp+8
; last		ebp+12
;
		align	4
_Render640wGDI2:
		push	ebp
		mov	ebp,esp
		push	ebx
		push	esi
		push	edi
; 初期ポインタ、カウンタを計算
		mov	esi,[ebp+8]
		imul	esi,80
		mov	edi,[_pBitsGDI]
		mov	ecx,[ebp+8]
		mov	eax,ecx
		imul	ecx,[ebp+28]
		lea	ecx,[ecx*4+ecx]
		shl	ecx,5
		add	edi,ecx
		mov	edx,[ebp+12]
		sub	edx,eax
		jz	near .exit
;
		shr	dword [ebp+16],3
		shr	dword [ebp+20],3
		mov	cl,[ebp+16]
		mov	ch,[ebp+20]
; １ラインループ
		align	4
.line_loop:
		push	eax
		push	edx
		imul	eax,[esp+28+20]
		xor	ebx,ebx
		and	eax,0xfffffff0
		mov	ebp,[_vram_dptr]
		lea	eax,[eax*4+eax]
; １バイトループ
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
		cmp	byte [_GDIDrawFlag + eax], 0
		je	.next_byte

		push	eax
		push	ebx

		cmp	byte [_crt_flag], 0
		jne	.bytedraw
		xor	ebx,ebx
		jmp	.bytewrite

		align	4
.bytedraw:
		mov	al,[esi+ebp+0x10000]
		mov	ah,[esi+ebp+0x8000]
		mov	dl,[esi+ebp]
; bit 7
		xor	ebx,ebx
		add	al,al
		adc	ebx,ebx
		add	ah,ah
		adc	ebx,ebx
		add	dl,dl
		adc	ebx,ebx
; bit 6
		add	ebx,ebx
		add	al,al
		adc	ebx,ebx
		add	ah,ah
		adc	ebx,ebx
		add	dl,dl
		adc	ebx,ebx
; bit 5
		add	ebx,ebx
		add	al,al
		adc	ebx,ebx
		add	ah,ah
		adc	ebx,ebx
		add	dl,dl
		adc	ebx,ebx
; bit 4
		add	ebx,ebx
		add	al,al
		adc	ebx,ebx
		add	ah,ah
		adc	ebx,ebx
		add	dl,dl
		adc	ebx,ebx
; bit 3
		add	ebx,ebx
		add	al,al
		adc	ebx,ebx
		add	ah,ah
		adc	ebx,ebx
		add	dl,dl
		adc	ebx,ebx
; bit 2
		add	ebx,ebx
		add	al,al
		adc	ebx,ebx
		add	ah,ah
		adc	ebx,ebx
		add	dl,dl
		adc	ebx,ebx
; bit 1
		add	ebx,ebx
		add	al,al
		adc	ebx,ebx
		add	ah,ah
		adc	ebx,ebx
		add	dl,dl
		adc	ebx,ebx
; bit 0
		add	ebx,ebx
		add	al,al
		adc	ebx,ebx
		add	ah,ah
		adc	ebx,ebx
		add	dl,dl
		adc	ebx,ebx
; 書き込み
.bytewrite:
		bswap	ebx
		mov	[edi],ebx
; 次のバイトへ
		pop	ebx
		pop	eax
.next_byte:
		inc	esi
		add	edi,4
		inc	eax
		inc	ebx
		jmp	near .byte_loop

; 次のラインへ
		align	4
.next_line:
		pop	edx
		pop	eax
		add	edi,[esp+24+12]
.next_line2:
		inc	eax
		dec	edx
		jnz	near .line_loop
; 終了
.exit:
		pop	edi
		pop	esi
		pop	ebx
		pop	ebp
		ret

	%endif

	%if XM7_VER >= 2

;
; 320x200、アナログモード
; GDIレンダリング
;
; static void Render320(int first, int last)
; first		ebp+8
; last		ebp+12
;
		align	4
_Render320GDI:
		push	ebp
		mov	ebp,esp
		push	ebx
		push	esi
		push	edi
; 初期ポインタ、カウンタを計算
	%if	XM7_VER >= 3
		mov	esi,[_vram_dptr]
	%else
		mov	esi,[_vram_c]
	%endif
		add	esi,0xc000
		mov	eax,[ebp+8]
		imul	eax,40
		add	esi,eax
		mov	edi,[_pBitsGDI]
		mov	ecx,[ebp+8]
		mov	eax,ecx
		imul	ecx,10
		shl	ecx,8
		add	edi,ecx
		mov	edx,[ebp+12]
		sub	edx,eax
		jz	near .exit
		mov	ebp,_rgbAnalogGDI
; １ラインループ
		align	4
.line_loop:
		lea	ebx,[eax*4]
		mov	ecx,40
		push	eax
		and	ebx,0xfffffff0
		push	edx
		lea	ebx,[ebx*4+ebx]
; １バイトループ
		align	4
.byte_loop:
		cmp	byte [_GDIDrawFlag + ebx], 0
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
; Gループ
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
; Rループ
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
; Bループ
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
; パレット変換
		mov	ecx,[ebp+ecx*4]
		mov	[edi],ecx
.next_byte:
		add	edi,4
		dec	ebx
		jnz	.b_loop
; 次のバイトへ
		pop	ebx
		pop	ecx
.next_byte3:
		inc	esi
		add	ebx,2
		dec	ecx
		jnz	near .byte_loop
; 次のラインへ
.next_line:
		pop	edx
		pop	eax
		add	edi,1280
.next_line2:
		inc	eax
		dec	edx
		jnz	near .line_loop
		jmp	.exit
;
		align	4
.next_byte2:
		add	edi,32
		jmp	.next_byte3
; 終了
.exit:
		pop	edi
		pop	esi
		pop	ebx
		pop	ebp
		ret

	%endif

	%if XM7_VER >= 3

;
; 320x200、アナログモード
; GDIレンダリング (32bit True Color)
;
; static void Render320(int first, int last)
; first		ebp+8
; last		ebp+12
;
		align	4
_Render320GDI32bpp:
		push	ebp
		mov	ebp,esp
		push	ebx
		push	esi
		push	edi
; 初期ポインタ、カウンタを計算
	%if	XM7_VER >= 3
		mov	esi,[_vram_dptr]
	%else
		mov	esi,[_vram_c]
	%endif
		add	esi,0xc000
		mov	eax,[ebp+8]
		imul	eax,40
		add	esi,eax
		mov	edi,[_pBitsGDI]
		mov	ecx,[ebp+8]
		mov	eax,ecx
		imul	ecx,10
		shl	ecx,9
		add	edi,ecx
		mov	edx,[ebp+12]
		sub	edx,eax
		jz	near .exit
		mov	ebp,_rgbAnalogGDI
; １ラインループ
		align	4
.line_loop:
		lea	ebx,[eax*4]
		mov	ecx,40
		push	eax
		and	ebx,0xfffffff0
		push	edx
		lea	ebx,[ebx*4+ebx]
; １バイトループ
		align	4
.byte_loop:
		cmp	byte [_GDIDrawFlag + ebx], 0
		je	.next_byte2

		push	ecx
		push	ebx
; G
		mov	al,[esi+0x4000]
		mov	ah,[esi+0x6000]
		mov	dl,[esi+0x8000]
		mov	dh,[esi+0xa000]
		mov	ebx,8
; Gループ
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
		add	edi,8
		dec	ebx
		jnz	.g_loop
		sub	edi,64
; R
		mov	al,[esi-0x4000]
		mov	ah,[esi-0x2000]
		mov	dl,[esi-0x0000]
		mov	dh,[esi+0x2000]
		mov	ebx,8
; Rループ
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
		add	edi,8
		dec	ebx
		jnz	.r_loop
		sub	edi,64
; B
		mov	al,[esi-0xc000]
		mov	ah,[esi-0xa000]
		mov	dl,[esi-0x8000]
		mov	dh,[esi-0x6000]
		mov	ebx,8
; Bループ
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
; パレット変換
		mov	ecx,[ebp+ecx*4]
		mov	[edi],ecx
		mov	[edi+4],ecx
.next_byte:
		add	edi,8
		dec	ebx
		jnz	.b_loop
; 次のバイトへ
		pop	ebx
		pop	ecx
.next_byte3:
		inc	esi
		add	ebx,2
		dec	ecx
		jnz	near .byte_loop
; 次のラインへ
.next_line:
		pop	edx
		pop	eax
		add	edi,2560
.next_line2:
		inc	eax
		dec	edx
		jnz	near .line_loop
		jmp	.exit
;
		align	4
.next_byte2:
		add	edi,64
		jmp	.next_byte3
; 終了
.exit:
		pop	edi
		pop	esi
		pop	ebx
		pop	ebp
		ret

;
; 320x200、アナログモード
; GDIレンダリング(Window)
;
; static void Render320w(int first, int last, int firstx, int lastx)
; first		ebp+8
; last		ebp+12
;
		align	4
_Render320wGDI:
		push	ebp
		mov	ebp,esp
		push	ebx
		push	esi
		push	edi
; 初期ポインタ、カウンタを計算
		mov	esi,0xc000
		mov	eax,[ebp+8]
		imul	eax,40
		add	esi,eax
		mov	edi,[_pBitsGDI]
		mov	ecx,[ebp+8]
		mov	eax,ecx
		imul	ecx,10
		shl	ecx,8
		add	edi,ecx
		mov	edx,[ebp+12]
		sub	edx,eax
		jz	near .exit
;
		shr	dword [ebp+16],3
		shr	dword [ebp+20],3
; １ラインループ
		align	4
.line_loop:
		push	eax
		push	edx
		lea	eax,[eax*4]
		xor	ebx,ebx
		and	eax,0xfffffff0
		mov	ebp,[_vram_dptr]
		lea	eax,[eax*4+eax]
; １バイトループ
		align	4
.byte_loop:
		cmp	ebx,40
		jae	near .next_line

		cmp	bl,byte [esp+16+20]
		jz	.change_dbank2
		cmp	bl,byte [esp+20+20]
		jnz	.draw_byte
.change_dbank1:
		mov	ebp,[_vram_dptr]
		jmp	.draw_byte
.change_dbank2:
		mov	ebp,[_vram_bdptr]

.draw_byte:
		cmp	byte [_GDIDrawFlag + eax], 0
		je	.next_byte2

		push	eax
		push	ebx
; G
		mov	al,[esi+ebp+0x4000]
		mov	ah,[esi+ebp+0x6000]
		mov	dl,[esi+ebp+0x8000]
		mov	dh,[esi+ebp+0xa000]
		mov	ebx,8
; Gループ
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
; Rループ
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
; Bループ
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
; パレット変換
		mov	ecx,[ecx*4+_rgbAnalogGDI]
		mov	[edi],ecx
		add	edi,4
		dec	ebx
		jnz	.b_loop
; 次のバイトへ
		pop	ebx
		pop	eax
.next_byte:
		inc	esi
		add	eax,2
		inc	ebx
		jmp	near .byte_loop
;
		align	4
.next_byte2:
		add	edi,32
		jmp	.next_byte

; 次のラインへ
		align	4
.next_line:
		pop	edx
		pop	eax
		add	edi,1280
.next_line2:
		inc	eax
		dec	edx
		jnz	near .line_loop
; 終了
.exit:
		pop	edi
		pop	esi
		pop	ebx
		pop	ebp
		ret

;
; 320x200、アナログモード
; GDIレンダリング(Window,32bit True Color)
;
; static void Render320w(int first, int last, int firstx, int lastx)
; first		ebp+8
; last		ebp+12
;
		align	4
_Render320wGDI32bpp:
		push	ebp
		mov	ebp,esp
		push	ebx
		push	esi
		push	edi
; 初期ポインタ、カウンタを計算
		mov	esi,0xc000
		mov	eax,[ebp+8]
		imul	eax,40
		add	esi,eax
		mov	edi,[_pBitsGDI]
		mov	ecx,[ebp+8]
		mov	eax,ecx
		imul	ecx,10
		shl	ecx,9
		add	edi,ecx
		mov	edx,[ebp+12]
		sub	edx,eax
		jz	near .exit
;
		shr	dword [ebp+16],3
		shr	dword [ebp+20],3
; １ラインループ
		align	4
.line_loop:
		push	eax
		push	edx
		lea	eax,[eax*4]
		xor	ebx,ebx
		and	eax,0xfffffff0
		mov	ebp,[_vram_dptr]
		lea	eax,[eax*4+eax]
; １バイトループ
		align	4
.byte_loop:
		cmp	ebx,40
		jae	near .next_line

		cmp	bl,byte [esp+16+20]
		jz	.change_dbank2
		cmp	bl,byte [esp+20+20]
		jnz	.draw_byte
.change_dbank1:
		mov	ebp,[_vram_dptr]
		jmp	.draw_byte
.change_dbank2:
		mov	ebp,[_vram_bdptr]

.draw_byte:
		cmp	byte [_GDIDrawFlag + eax], 0
		je	.next_byte2

		push	eax
		push	ebx
; G
		mov	al,[esi+ebp+0x4000]
		mov	ah,[esi+ebp+0x6000]
		mov	dl,[esi+ebp+0x8000]
		mov	dh,[esi+ebp+0xa000]
		mov	ebx,8
; Gループ
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
		add	edi,8
		dec	ebx
		jnz	.g_loop
		sub	edi,64
; R
		mov	al,[esi+ebp-0x4000]
		mov	ah,[esi+ebp-0x2000]
		mov	dl,[esi+ebp-0x0000]
		mov	dh,[esi+ebp+0x2000]
		mov	ebx,8
; Rループ
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
		add	edi,8
		dec	ebx
		jnz	.r_loop
		sub	edi,64
; B
		mov	al,[esi+ebp-0xc000]
		mov	ah,[esi+ebp-0xa000]
		mov	dl,[esi+ebp-0x8000]
		mov	dh,[esi+ebp-0x6000]
		mov	ebx,8
; Bループ
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
; パレット変換
		mov	ecx,[ecx*4+_rgbAnalogGDI]
		mov	[edi],ecx
		mov	[edi+4],ecx
		add	edi,8
		dec	ebx
		jnz	.b_loop
; 次のバイトへ
		pop	ebx
		pop	eax
.next_byte:
		inc	esi
		add	eax,2
		inc	ebx
		jmp	near .byte_loop
;
		align	4
.next_byte2:
		add	edi,64
		jmp	.next_byte

; 次のラインへ
		align	4
.next_line:
		pop	edx
		pop	eax
		add	edi,2560
.next_line2:
		inc	eax
		dec	edx
		jnz	near .line_loop
; 終了
.exit:
		pop	edi
		pop	esi
		pop	ebx
		pop	ebp
		ret

;
; 320x200、26万色モード
; GDIレンダリング (32bit True Color)
;
; static void Render256k(int first, int last)
; first		ebp+8
; last		ebp+12
;
		align	4
_Render256kGDI:
		push	ebp
		mov	ebp,esp
		push	ebx
		push	esi
		push	edi
; 初期ポインタ、カウンタを計算
		mov	esi,[_vram_c]
		add	esi,0xc000
		mov	eax,[ebp+8]
		imul	eax,40
		add	esi,eax
		mov	edi,[_pBitsGDI]
		mov	ecx,[ebp+8]
		mov	eax,ecx
		imul	ecx,10
		shl	ecx,9
		add	edi,ecx
		mov	edx,[ebp+12]
		sub	edx,eax
; １ラインループ
		align	4
.line_loop:
		lea	ebx,[eax*4]
		mov	ecx,40
		push	eax
		and	ebx,0xfffffff0
		push	edx
		lea	ebx,[ebx*4+ebx]
; １バイトループ
		align	4
.byte_loop:
		cmp	byte [_GDIDrawFlag + ebx], 0
		je	.next_byte2

		push	ecx
		push	ebx

; 64byte clear
		xor		ecx,ecx
		mov		edx,edi
		mov		eax,16

		align	4
.clr_loop:
		mov		[edx],ecx
		add		edx,4
		dec		eax
		jnz		.clr_loop

; R
		test	byte [esp+16+28],0x20
		jnz		near .green

		mov	al,[esi-0x4000]
		mov	ah,[esi-0x2000]
		mov	dl,[esi-0x0000]
		mov	dh,[esi+0x2000]
		mov	bl,[esi+0x14000]
		mov	bh,[esi+0x16000]
		mov	ebp,8
; Rループ
.r_loop:
		xor	ecx,ecx
		add	al,al
		adc	cl,cl
		add	ah,ah
		adc	cl,cl
		add	dl,dl
		adc	cl,cl
		add	dh,dh
		adc	cl,cl
		add	bl,bl
		adc	cl,cl
		add	bh,bh
		adc	cl,cl
		mov	cl,[_truecolorbrightness+ecx]
		mov	[edi+2],cl
		mov	[edi+6],cl
		add	edi,8
		dec	ebp
		jnz	.r_loop
		sub	edi,64

; G
		align	4
.green:
		test	byte [esp+16+28],0x40
		jnz		near .blue

		mov	al,[esi+0x4000]
		mov	ah,[esi+0x6000]
		mov	dl,[esi+0x8000]
		mov	dh,[esi+0xa000]
		mov	bl,[esi+0x1c000]
		mov	bh,[esi+0x1e000]
		mov	ebp,8
; Gループ
.g_loop:
		xor	ecx,ecx
		add	al,al
		adc	cl,cl
		add	ah,ah
		adc	cl,cl
		add	dl,dl
		adc	cl,cl
		add	dh,dh
		adc	cl,cl
		add	bl,bl
		adc	cl,cl
		add	bh,bh
		adc	cl,cl
		mov	cl,[_truecolorbrightness+ecx]
		mov	[edi+1],cl
		mov	[edi+5],cl
		add	edi,8
		dec	ebp
		jnz	.g_loop
		sub	edi,64

; B
		align	4
.blue:
		test	byte [esp+16+28],0x10
		jnz		near .skip_blue

		mov	al,[esi-0xc000]
		mov	ah,[esi-0xa000]
		mov	dl,[esi-0x8000]
		mov	dh,[esi-0x6000]
		mov	bl,[esi+0xc000]
		mov	bh,[esi+0xe000]
		mov	ebp,8
; Bループ
.b_loop:
		xor	ecx,ecx
		add	al,al
		adc	cl,cl
		add	ah,ah
		adc	cl,cl
		add	dl,dl
		adc	cl,cl
		add	dh,dh
		adc	cl,cl
		add	bl,bl
		adc	cl,cl
		add	bh,bh
		adc	cl,cl
		mov	cl,[_truecolorbrightness+ecx]
		mov	[edi],cl
		mov	[edi+4],cl
		add	edi,8
		dec	ebp
		jnz	.b_loop
		jmp	near .next_byte
.skip_blue:
		add	edi,64

; 次のバイトへ
		align	4
.next_byte:
		pop	ebx
		pop	ecx
.next_byte3:
		inc	esi
		add	ebx,2
		dec	ecx
		jnz	near .byte_loop
; 次のラインへ
		pop	edx
		pop	eax
		add	edi,2560
.next_line2:
		inc	eax
		dec	edx
		jnz	near .line_loop
		jmp	.exit
;
		align	4
.next_byte2:
		add	edi,64
		jmp	.next_byte3
; 終了
.exit:
		pop	edi
		pop	esi
		pop	ebx
		pop	ebp
		ret

	%endif

	%if XM7_VER == 1
;
; 640x200、デジタルモード(疑似400ラインアダプタ)
; GDIレンダリング
;
; static void Render640(int first, int last)
; first		ebp+8
; last		ebp+12
;
		align	4
_Render640mGDI:
		push	ebp
		mov	ebp,esp
		push	ebx
		push	esi
		push	edi
; 初期ポインタ、カウンタを計算
		mov	esi,[_vram_c]
		mov	eax,[ebp+8]
		imul	eax,80
		add	esi,eax
		mov	edi,[_pBitsGDI]
		mov	ecx,[ebp+8]
		mov	eax,ecx
		imul	eax,10
		shl	eax,6
		add	edi,eax
		mov	edx,[ebp+12]
		sub	edx,ecx
		jz	near .exit
; １ラインループ
		align	4
.line_loop:
		lea	ebx,[ecx*4]
		mov	ebp,80
		and	ebx,0xfffffff0
		push	edx

		lea	ebx,[ebx*4+ebx]
; １バイトループ
		align	4
.byte_loop:
		cmp	byte [_GDIDrawFlag + ebx], 0
		je	.next_byte

		push	ebx
;
	%if	XM7_VER >= 3
		mov	al,[esi+0x10000]
		mov	ah,[esi+0x8000]
		mov	dl,[esi]
	%else
		mov	al,[esi+0x8000]
		mov	ah,[esi+0x4000]
		mov	dl,[esi]
	%endif
; bit 7
		xor	ebx,ebx
		add	al,al
		adc	ebx,ebx
		add	ah,ah
		adc	ebx,ebx
		add	dl,dl
		adc	ebx,ebx
; bit 6
		add	ebx,ebx
		add	al,al
		adc	ebx,ebx
		add	ah,ah
		adc	ebx,ebx
		add	dl,dl
		adc	ebx,ebx
; bit 5
		add	ebx,ebx
		add	al,al
		adc	ebx,ebx
		add	ah,ah
		adc	ebx,ebx
		add	dl,dl
		adc	ebx,ebx
; bit 4
		add	ebx,ebx
		add	al,al
		adc	ebx,ebx
		add	ah,ah
		adc	ebx,ebx
		add	dl,dl
		adc	ebx,ebx
; bit 3
		add	ebx,ebx
		add	al,al
		adc	ebx,ebx
		add	ah,ah
		adc	ebx,ebx
		add	dl,dl
		adc	ebx,ebx
; bit 2
		add	ebx,ebx
		add	al,al
		adc	ebx,ebx
		add	ah,ah
		adc	ebx,ebx
		add	dl,dl
		adc	ebx,ebx
; bit 1
		add	ebx,ebx
		add	al,al
		adc	ebx,ebx
		add	ah,ah
		adc	ebx,ebx
		add	dl,dl
		adc	ebx,ebx
; bit 0
		add	ebx,ebx
		add	al,al
		adc	ebx,ebx
		add	ah,ah
		adc	ebx,ebx
		add	dl,dl
		adc	ebx,ebx
; 書き込み
		bswap	ebx
		mov	[edi], ebx
		or	ebx, 88888888h
		mov	[edi+320], ebx
; 次のバイトへ
		pop	ebx
.next_byte:
		inc	esi
		add	edi,4
		inc	ebx
		dec	ebp
		jnz	near .byte_loop
; 次のラインへ
		pop	edx
		add	edi,320
.next_line:
		inc	ecx
		dec	edx
		jnz	near .line_loop
		jmp	near .exit
; 終了
.exit:
		pop	edi
		pop	esi
		pop	ebx
		pop	ebp
		ret

	%endif

	%if XM7_VER >= 2

;
; 640x200、デジタルモード(2バンク疑似400ライン)
; GDIレンダリング
;
; static void Render640(int first, int last)
; first		ebp+8
; last		ebp+12
;
		align	4
_Render640cGDI:
		push	ebp
		mov	ebp,esp
		push	ebx
		push	esi
		push	edi
; 初期ポインタ、カウンタを計算
	%if	XM7_VER >= 3
		mov	esi,[_vram_dblk]
	%else
		mov	esi,[_vram_c]
	%endif
		mov	eax,[ebp+8]
		imul	eax,80
		add	esi,eax
		mov	edi,[_pBitsGDI]
		mov	ecx,[ebp+8]
		mov	eax,ecx
		imul	eax,10
		shl	eax,6
		add	edi,eax
		mov	edx,[ebp+12]
		sub	edx,ecx
		jz	near .exit
; １ラインループ
		align	4
.line_loop:
		lea	ebx,[ecx*4]
		mov	ebp,80
		and	ebx,0xfffffff0
		push	edx

		lea	ebx,[ebx*4+ebx]
; １バイトループ
		align	4
.byte_loop:
		cmp	byte [_GDIDrawFlag + ebx], 0
		je	.next_byte

		push	ebx
;
	%if	XM7_VER >= 3
		mov	al,[esi+0x10000]
		mov	ah,[esi+0x8000]
		mov	dl,[esi]
	%else
		mov	al,[esi+0x8000]
		mov	ah,[esi+0x4000]
		mov	dl,[esi]
	%endif
; bit 7
		xor	ebx,ebx
		add	al,al
		adc	ebx,ebx
		add	ah,ah
		adc	ebx,ebx
		add	dl,dl
		adc	ebx,ebx
; bit 6
		add	ebx,ebx
		add	al,al
		adc	ebx,ebx
		add	ah,ah
		adc	ebx,ebx
		add	dl,dl
		adc	ebx,ebx
; bit 5
		add	ebx,ebx
		add	al,al
		adc	ebx,ebx
		add	ah,ah
		adc	ebx,ebx
		add	dl,dl
		adc	ebx,ebx
; bit 4
		add	ebx,ebx
		add	al,al
		adc	ebx,ebx
		add	ah,ah
		adc	ebx,ebx
		add	dl,dl
		adc	ebx,ebx
; bit 3
		add	ebx,ebx
		add	al,al
		adc	ebx,ebx
		add	ah,ah
		adc	ebx,ebx
		add	dl,dl
		adc	ebx,ebx
; bit 2
		add	ebx,ebx
		add	al,al
		adc	ebx,ebx
		add	ah,ah
		adc	ebx,ebx
		add	dl,dl
		adc	ebx,ebx
; bit 1
		add	ebx,ebx
		add	al,al
		adc	ebx,ebx
		add	ah,ah
		adc	ebx,ebx
		add	dl,dl
		adc	ebx,ebx
; bit 0
		add	ebx,ebx
		add	al,al
		adc	ebx,ebx
		add	ah,ah
		adc	ebx,ebx
		add	dl,dl
		adc	ebx,ebx
; 書き込み
		bswap	ebx
		mov	[edi], ebx
;
	%if	XM7_VER >= 3
		mov	al,[esi+0x14000]
		mov	ah,[esi+0xc000]
		mov	dl,[esi+0x4000]
	%else
		mov	al,[esi+0x14000]
		mov	ah,[esi+0x10000]
		mov	dl,[esi+0xc000]
	%endif
; bit 7
		xor	ebx,ebx
		add	al,al
		adc	ebx,ebx
		add	ah,ah
		adc	ebx,ebx
		add	dl,dl
		adc	ebx,ebx
; bit 6
		add	ebx,ebx
		add	al,al
		adc	ebx,ebx
		add	ah,ah
		adc	ebx,ebx
		add	dl,dl
		adc	ebx,ebx
; bit 5
		add	ebx,ebx
		add	al,al
		adc	ebx,ebx
		add	ah,ah
		adc	ebx,ebx
		add	dl,dl
		adc	ebx,ebx
; bit 4
		add	ebx,ebx
		add	al,al
		adc	ebx,ebx
		add	ah,ah
		adc	ebx,ebx
		add	dl,dl
		adc	ebx,ebx
; bit 3
		add	ebx,ebx
		add	al,al
		adc	ebx,ebx
		add	ah,ah
		adc	ebx,ebx
		add	dl,dl
		adc	ebx,ebx
; bit 2
		add	ebx,ebx
		add	al,al
		adc	ebx,ebx
		add	ah,ah
		adc	ebx,ebx
		add	dl,dl
		adc	ebx,ebx
; bit 1
		add	ebx,ebx
		add	al,al
		adc	ebx,ebx
		add	ah,ah
		adc	ebx,ebx
		add	dl,dl
		adc	ebx,ebx
; bit 0
		add	ebx,ebx
		add	al,al
		adc	ebx,ebx
		add	ah,ah
		adc	ebx,ebx
		add	dl,dl
		adc	ebx,ebx
; 書き込み
		bswap	ebx
		mov	[edi+320], ebx
; 次のバイトへ
		pop	ebx
.next_byte:
		inc	esi
		add	edi,4
		inc	ebx
		dec	ebp
		jnz	near .byte_loop
; 次のラインへ
		pop	edx
		add	edi, 320
.next_line:
		inc	ecx
		dec	edx
		jnz	near .line_loop
		jmp	near .exit
; 終了
.exit:
		pop	edi
		pop	esi
		pop	ebx
		pop	ebp
		ret

	%endif

	%if XM7_VER >= 3

;
; 640x200、デジタルモード (2バンク疑似400ライン)
; GDIレンダリング(Window)
;
; static void Render640w(int first, int last,int firstx, int lastx)
; first		ebp+8
; last		ebp+12
;
		align	4
_Render640cwGDI:
		push	ebp
		mov	ebp,esp
		push	ebx
		push	esi
		push	edi
; 初期ポインタ、カウンタを計算
		mov	esi,[ebp+8]
		imul	esi,80
		mov	edi,[_pBitsGDI]
		mov	ecx,[ebp+8]
		mov	eax,ecx
		lea	ecx,[ecx*4+ecx]
		shl	ecx,7
		add	edi,ecx
		mov	edx,[ebp+12]
		sub	edx,eax
		jz	near .exit
;
		shr	dword [ebp+16],3
		shr	dword [ebp+20],3
		mov	cl,[ebp+16]
		mov	ch,[ebp+20]
; １ラインループ
		align	4
.line_loop:
		push	eax
		push	edx
		shl		eax, 2
		xor	ebx,ebx
		and	eax,0xfffffff0
		mov	ebp,[_vram_dblk]
		lea	eax,[eax*4+eax]
; １バイトループ
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
		cmp	byte [_GDIDrawFlag + eax], 0
		je	.next_byte

		push	eax
		push	ebx
;
		mov	al,[esi+ebp+0x10000]
		mov	ah,[esi+ebp+0x8000]
		mov	dl,[esi+ebp]
; bit 7
		xor	ebx,ebx
		add	al,al
		adc	ebx,ebx
		add	ah,ah
		adc	ebx,ebx
		add	dl,dl
		adc	ebx,ebx
; bit 6
		add	ebx,ebx
		add	al,al
		adc	ebx,ebx
		add	ah,ah
		adc	ebx,ebx
		add	dl,dl
		adc	ebx,ebx
; bit 5
		add	ebx,ebx
		add	al,al
		adc	ebx,ebx
		add	ah,ah
		adc	ebx,ebx
		add	dl,dl
		adc	ebx,ebx
; bit 4
		add	ebx,ebx
		add	al,al
		adc	ebx,ebx
		add	ah,ah
		adc	ebx,ebx
		add	dl,dl
		adc	ebx,ebx
; bit 3
		add	ebx,ebx
		add	al,al
		adc	ebx,ebx
		add	ah,ah
		adc	ebx,ebx
		add	dl,dl
		adc	ebx,ebx
; bit 2
		add	ebx,ebx
		add	al,al
		adc	ebx,ebx
		add	ah,ah
		adc	ebx,ebx
		add	dl,dl
		adc	ebx,ebx
; bit 1
		add	ebx,ebx
		add	al,al
		adc	ebx,ebx
		add	ah,ah
		adc	ebx,ebx
		add	dl,dl
		adc	ebx,ebx
; bit 0
		add	ebx,ebx
		add	al,al
		adc	ebx,ebx
		add	ah,ah
		adc	ebx,ebx
		add	dl,dl
		adc	ebx,ebx
; 書き込み
		bswap	ebx
		mov	[edi],ebx
;
		mov	al,[esi+ebp+0x14000]
		mov	ah,[esi+ebp+0xc000]
		mov	dl,[esi+ebp+0x4000]
; bit 7
		xor	ebx,ebx
		add	al,al
		adc	ebx,ebx
		add	ah,ah
		adc	ebx,ebx
		add	dl,dl
		adc	ebx,ebx
; bit 6
		add	ebx,ebx
		add	al,al
		adc	ebx,ebx
		add	ah,ah
		adc	ebx,ebx
		add	dl,dl
		adc	ebx,ebx
; bit 5
		add	ebx,ebx
		add	al,al
		adc	ebx,ebx
		add	ah,ah
		adc	ebx,ebx
		add	dl,dl
		adc	ebx,ebx
; bit 4
		add	ebx,ebx
		add	al,al
		adc	ebx,ebx
		add	ah,ah
		adc	ebx,ebx
		add	dl,dl
		adc	ebx,ebx
; bit 3
		add	ebx,ebx
		add	al,al
		adc	ebx,ebx
		add	ah,ah
		adc	ebx,ebx
		add	dl,dl
		adc	ebx,ebx
; bit 2
		add	ebx,ebx
		add	al,al
		adc	ebx,ebx
		add	ah,ah
		adc	ebx,ebx
		add	dl,dl
		adc	ebx,ebx
; bit 1
		add	ebx,ebx
		add	al,al
		adc	ebx,ebx
		add	ah,ah
		adc	ebx,ebx
		add	dl,dl
		adc	ebx,ebx
; bit 0
		add	ebx,ebx
		add	al,al
		adc	ebx,ebx
		add	ah,ah
		adc	ebx,ebx
		add	dl,dl
		adc	ebx,ebx
; 書き込み
		bswap	ebx
		mov	[edi+320],ebx
; 次のバイトへ
		pop	ebx
		pop	eax
.next_byte:
		inc	esi
		add	edi,4
		inc	eax
		inc	ebx
		jmp	near .byte_loop

; 次のラインへ
		align	4
.next_line:
		pop	edx
		pop	eax
		add	edi,320
.next_line2:
		inc	eax
		dec	edx
		jnz	near .line_loop
; 終了
.exit:
		pop	edi
		pop	esi
		pop	ebx
		pop	ebp
		ret

	%endif


;
; プログラム終了
;
		end
