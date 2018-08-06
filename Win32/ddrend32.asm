;
; FM-7 EMULATOR "XM7"
;
; Copyright (C) 1999-2017 ＰＩ．(Twitter:@xm6_original)
; Copyright (C) 2001-2017 Ryu Takegami (Twitter:@RyuTakegami)
;
; [ Win32API レンダリング(DirectDraw・32bit TrueColor) ]
;
; RHG履歴
;	2002.06.21		更新が必要な部分だけレンダリングするように変更
;	2002.09.22		32bpp→24bpp変換ルーチンを追加
;	2003.02.11		8色モードレンダラ(200line/400line)を統合
;					24bppレンダラ復活に伴い32bpp→24bpp変換ルーチンを削除
;	2010.01.13		TrueColor時の輝度変換をテーブル化
;	2014.03.16		V1/V2対応処理を追加
;

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
		extern	_rgbTTLDD
		extern	_rgbAnalogDD
		extern	_DDDrawFlag
		extern	_truecolorbrightness

		section	.text class=CODE align=16 use32
		global	_Render640Tc32DD2
		global	_Render640wTc32DD2
		global	_Render320Tc32DD
		global	_Render320wTc32DD
		global	_Render256kTc32DD
		global	_Render640cTc32DD
		global	_Render640cwTc32DD
	%elif XM7_VER >= 2
		section	.data class=DATA align=4 use32
		extern	_vram_c
		extern	_vram_dptr
		extern	_rgbTTLDD
		extern	_rgbAnalogDD
		extern	_DDDrawFlag

		section	.text class=CODE align=16 use32
		global	_Render640Tc32DD
		global	_Render320Tc32DD
		global	_Render640cTc32DD
	%else
		section	.data class=DATA align=4 use32
		extern	_vram_c
		extern	_rgbTTLDD
		extern	_DDDrawFlag

		section	.text class=CODE align=16 use32
		global	_Render640Tc32DD
		global	_Render640mTc32DD
	%endif

	%if XM7_VER <= 2

;
; 640x200、デジタルモード
; DirectDrawレンダリング
;
; static void Render640(LPVOID lpSurface, LONG lPitch, int first, int last)
; lpSurface	ebp+8
; lPitch	ebp+12
; first		ebp+16
; last		ebp+20
;
		align	4
_Render640Tc32DD:
		push	ebp
		mov	ebp,esp
		push	ebx
		push	esi
		push	edi
; サーフェイス設定
		mov	edi,[ebp+8]
		mov	eax,[ebp+12]
		imul	eax,[ebp+16]
		add	edi,eax
		add	edi,eax
; ピッチを前もって計算
		mov	esi,[ebp+12]
		sub	esi,2560
		mov	[ebp+12],esi
; VRAMアドレス設定
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
; ライン数設定
		mov	edx,[ebp+20]
		sub	edx,eax
		jz	near .exit
; １ラインループ
		align	4
.line_loop:
		lea	ecx,[eax*4]
		push	eax
		and	ecx,0xfffffff0
		push	edx
		lea	ecx,[ecx*4+ecx]
		mov	ebx,80
; １バイトループ
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
		mov	ecx,[_rgbTTLDD+ecx*4]
		mov	[edi],ecx
; bit6
		xor	ecx,ecx
		add	al,al
		adc	ecx,ecx
		add	ah,ah
		adc	ecx,ecx
		add	dh,dh
		adc	ecx,ecx
		mov	ecx,[_rgbTTLDD+ecx*4]
		mov	[edi+4],ecx
; bit5
		xor	ecx,ecx
		add	al,al
		adc	ecx,ecx
		add	ah,ah
		adc	ecx,ecx
		add	dh,dh
		adc	ecx,ecx
		mov	ecx,[_rgbTTLDD+ecx*4]
		mov	[edi+8],ecx
; bit4
		xor	ecx,ecx
		add	al,al
		adc	ecx,ecx
		add	ah,ah
		adc	ecx,ecx
		add	dh,dh
		adc	ecx,ecx
		mov	ecx,[_rgbTTLDD+ecx*4]
		mov	[edi+12],ecx
; bit3
		xor	ecx,ecx
		add	al,al
		adc	ecx,ecx
		add	ah,ah
		adc	ecx,ecx
		add	dh,dh
		adc	ecx,ecx
		mov	ecx,[_rgbTTLDD+ecx*4]
		mov	[edi+16],ecx
; bit2
		xor	ecx,ecx
		add	al,al
		adc	ecx,ecx
		add	ah,ah
		adc	ecx,ecx
		add	dh,dh
		adc	ecx,ecx
		mov	ecx,[_rgbTTLDD+ecx*4]
		mov	[edi+20],ecx
; bit1
		xor	ecx,ecx
		add	al,al
		adc	ecx,ecx
		add	ah,ah
		adc	ecx,ecx
		add	dh,dh
		adc	ecx,ecx
		mov	ecx,[_rgbTTLDD+ecx*4]
		mov	[edi+24],ecx
; bit0
		xor	ecx,ecx
		add	al,al
		adc	ecx,ecx
		add	ah,ah
		adc	ecx,ecx
		add	dh,dh
		adc	ecx,ecx
		mov	ecx,[_rgbTTLDD+ecx*4]
		mov	[edi+28],ecx
; 次のバイトへ
		pop	ecx
.next_byte:
		add	edi,32
		inc	esi
		inc	ecx
		dec	ebx
		jnz	near .byte_loop

; 次のラインへ
		pop	edx
		pop	eax
; インターレース分をスキップ
.next_line:
		add	edi,[ebp+12]
		add	edi,2560
		add	edi,[ebp+12]
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

	%else

;
; 640x200/640x400、デジタルモード
; DirectDrawレンダリング
;
; static void Render640(LPVOID lpSurface, LONG lPitch, int first, int last,
;						int scale)
; lpSurface	ebp+8
; lPitch	ebp+12
; first		ebp+16
; last		ebp+20
;
		align	4
_Render640Tc32DD2:
		push	ebp
		mov	ebp,esp
		push	ebx
		push	esi
		push	edi
; サーフェイス設定
		mov	edi,[ebp+8]
		mov	eax,[ebp+12]
		mov	ebx,[ebp+24]
		imul	eax,[ebp+16]
		imul	eax,ebx
		add	edi,eax
; ピッチを前もって計算
		mov	esi,[ebp+12]
		imul	esi,ebx
		sub	esi,2560
		mov	[ebp+12],esi
; VRAMアドレス設定
		mov	esi,[_vram_dptr]
		add	esi,0x8000
		mov	ecx,[ebp+16]
		mov	eax,ecx
		imul	ecx,80
		add	esi,ecx
; ライン数設定
		mov	edx,[ebp+20]
		sub	edx,eax
		jz	near .exit
; １ラインループ
		align	4
.line_loop:
		lea	ecx,[eax*2]
		imul	ecx,[ebp+24]
		push	eax
		and	ecx,0xfffffff0
		push	edx
		lea	ecx,[ecx*4+ecx]
		mov	ebx,80
; １バイトループ
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
		mov	ecx,[_rgbTTLDD+ecx*4]
		mov	[edi],ecx
; bit6
		xor	ecx,ecx
		add	al,al
		adc	ecx,ecx
		add	ah,ah
		adc	ecx,ecx
		add	dh,dh
		adc	ecx,ecx
		mov	ecx,[_rgbTTLDD+ecx*4]
		mov	[edi+4],ecx
; bit5
		xor	ecx,ecx
		add	al,al
		adc	ecx,ecx
		add	ah,ah
		adc	ecx,ecx
		add	dh,dh
		adc	ecx,ecx
		mov	ecx,[_rgbTTLDD+ecx*4]
		mov	[edi+8],ecx
; bit4
		xor	ecx,ecx
		add	al,al
		adc	ecx,ecx
		add	ah,ah
		adc	ecx,ecx
		add	dh,dh
		adc	ecx,ecx
		mov	ecx,[_rgbTTLDD+ecx*4]
		mov	[edi+12],ecx
; bit3
		xor	ecx,ecx
		add	al,al
		adc	ecx,ecx
		add	ah,ah
		adc	ecx,ecx
		add	dh,dh
		adc	ecx,ecx
		mov	ecx,[_rgbTTLDD+ecx*4]
		mov	[edi+16],ecx
; bit2
		xor	ecx,ecx
		add	al,al
		adc	ecx,ecx
		add	ah,ah
		adc	ecx,ecx
		add	dh,dh
		adc	ecx,ecx
		mov	ecx,[_rgbTTLDD+ecx*4]
		mov	[edi+20],ecx
; bit1
		xor	ecx,ecx
		add	al,al
		adc	ecx,ecx
		add	ah,ah
		adc	ecx,ecx
		add	dh,dh
		adc	ecx,ecx
		mov	ecx,[_rgbTTLDD+ecx*4]
		mov	[edi+24],ecx
; bit0
		xor	ecx,ecx
		add	al,al
		adc	ecx,ecx
		add	ah,ah
		adc	ecx,ecx
		add	dh,dh
		adc	ecx,ecx
		mov	ecx,[_rgbTTLDD+ecx*4]
		mov	[edi+28],ecx
; 次のバイトへ
		pop	ecx
.next_byte:
		add	edi,32
		inc	esi
		inc	ecx
		dec	ebx
		jnz	near .byte_loop

; 次のラインへ
		pop	edx
		pop	eax
; インターレース分をスキップ
.next_line:
		add	edi,[ebp+12]
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
; 640x200/640x400、デジタルモード
; DirectDrawレンダリング(Window)
;
; static void Render640w(LPVOID lpSurface, LONG lPitch,
;			int first, int last, int firstx, int lastx, int pitch)
; lpSurface	ebp+8
; lPitch	ebp+12
; first		ebp+16
; last		ebp+20
;
		align	4
_Render640wTc32DD2:
		push	ebp
		mov	ebp,esp
		push	ebx
		push	esi
		push	edi
; サーフェイス設定
		mov	edi,[ebp+8]
		mov	eax,[ebp+12]
		mov	ebx,[ebp+32]
		imul	eax,[ebp+16]
		imul	eax,ebx
		add	edi,eax
; ピッチを前もって計算
		mov	esi,[ebp+12]
		imul	esi,ebx
		sub	esi,2560
		mov	[ebp+12],esi
; VRAMアドレス設定
		mov	esi,0x8000
		mov	ecx,[ebp+16]
		mov	eax,ecx
		imul	ecx,80
		add	esi,ecx
; ライン数設定
		mov	edx,[ebp+20]
		sub	edx,eax
		jz	near .exit
; ウィンドウ切り替えポインタ設定
		shr	dword [ebp+24],3
		shr	dword [ebp+28],3
		mov	cl,[ebp+24]
		mov	ch,[ebp+28]
; １ラインループ
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
		mov	ebx,[ebx*4+_rgbTTLDD]
		mov	[edi],ebx
; bit 6
		xor	ebx,ebx
		add	al,al
		adc	ebx,ebx
		add	ah,ah
		adc	ebx,ebx
		add	dl,dl
		adc	ebx,ebx
		mov	ebx,[ebx*4+_rgbTTLDD]
		mov	[edi+4],ebx
; bit 5
		xor	ebx,ebx
		add	al,al
		adc	ebx,ebx
		add	ah,ah
		adc	ebx,ebx
		add	dl,dl
		adc	ebx,ebx
		mov	ebx,[ebx*4+_rgbTTLDD]
		mov	[edi+8],ebx
; bit 4
		xor	ebx,ebx
		add	al,al
		adc	ebx,ebx
		add	ah,ah
		adc	ebx,ebx
		add	dl,dl
		adc	ebx,ebx
		mov	ebx,[ebx*4+_rgbTTLDD]
		mov	[edi+12],ebx
; bit 3
		xor	ebx,ebx
		add	al,al
		adc	ebx,ebx
		add	ah,ah
		adc	ebx,ebx
		add	dl,dl
		adc	ebx,ebx
		mov	ebx,[ebx*4+_rgbTTLDD]
		mov	[edi+16],ebx
; bit 2
		xor	ebx,ebx
		add	al,al
		adc	ebx,ebx
		add	ah,ah
		adc	ebx,ebx
		add	dl,dl
		adc	ebx,ebx
		mov	ebx,[ebx*4+_rgbTTLDD]
		mov	[edi+20],ebx
; bit 1
		xor	ebx,ebx
		add	al,al
		adc	ebx,ebx
		add	ah,ah
		adc	ebx,ebx
		add	dl,dl
		adc	ebx,ebx
		mov	ebx,[ebx*4+_rgbTTLDD]
		mov	[edi+24],ebx
; bit 0
		xor	ebx,ebx
		add	al,al
		adc	ebx,ebx
		add	ah,ah
		adc	ebx,ebx
		add	dl,dl
		adc	ebx,ebx
		mov	ebx,[ebx*4+_rgbTTLDD]
		mov	[edi+28],ebx
; 次のバイトへ
		pop	ebx
		pop	eax
.next_byte:
		inc	esi
		add	edi,32
		inc	eax
		inc	ebx
		jmp	near .byte_loop

		align	4
.next_line:
		pop	ebp
		pop	edx
		pop	eax
.next_line2:
; 次のラインへ
		add	edi,[ebp+12]
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
; DirectDrawレンダリング
;
; static void Render320(LPVOID lpSurface, LONG lPitch, int first, int last)
; lpSurface	ebp+8
; lPitch	ebp+12
; first		ebp+16
; last		ebp+20
;
		align	4
_Render320Tc32DD:
		push	ebp
		mov	ebp,esp
		push	ebx
		push	esi
		push	edi
; サーフェイス設定
		mov	edi,[ebp+8]
		mov	eax,[ebp+12]
		imul	eax,[ebp+16]
		add	edi,eax
		add	edi,eax
; ピッチを前もって計算
		mov	esi,[ebp+12]
		sub	esi,2560
		mov	[ebp+12],esi
; VRAMアドレス設定
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
; ライン数設定
		mov	edx,[ebp+20]
		sub	edx,eax
		jz	near .exit
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
		add	edi,8
		dec	ebx
		jnz	.r_loop
		sub	edi,64
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
; パレット変換
		and	ecx,4095
		mov	ecx,[ecx*4+_rgbAnalogDD]
		mov	[edi],ecx
		mov	[edi+4],ecx
		add	edi,8
		dec	ebx
		jnz	.b_loop
; 次のバイトへ
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
		add	edi,64
		jmp	.next_byte0

.next_line:
		pop	edx
		pop	eax
; インターレース分をスキップ
.next_line2:
		add	edi,[ebp+12]
		add	edi,2560
; 次のラインへ
		add	edi,[ebp+12]
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


	%if XM7_VER >= 3

;
; 320x200、アナログモード
; DirectDrawレンダリング(Window)
;
; static void Render320w(LPVOID lpSurface, LONG lPitch,
;			int first, int last, int firstx, int lastx)
; lpSurface	ebp+8
; lPitch	ebp+12
; first		ebp+16
; last		ebp+20
;
		align	4
_Render320wTc32DD:
		push	ebp
		mov	ebp,esp
		push	ebx
		push	esi
		push	edi
; サーフェイス設定
		mov	edi,[ebp+8]
		mov	eax,[ebp+12]
		imul	eax,[ebp+16]
		add	edi,eax
		add	edi,eax
; ピッチを前もって計算
		mov	esi,[ebp+12]
		sub	esi,2560
		mov	[ebp+12],esi
; VRAMアドレス設定
		mov	ecx,[ebp+16]
		mov	eax,ecx
		imul	ecx,40
		lea	esi,[ecx+0xc000]
; ライン数設定
		mov	edx,[ebp+20]
		sub	edx,eax
		jz	near .exit
; ウインドウ切り替えポイント補正
		shr	dword [ebp+24],3
		shr	dword [ebp+28],3
; １ラインループ
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
; １バイトループ
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
; パレット変換
		and	ecx,4095
		mov	ecx,[ecx*4+_rgbAnalogDD]
		mov	[edi],ecx
		mov	[edi+4],ecx
		add	edi,8
		dec	ebx
		jnz	.b_loop
; 次のバイトへ
		pop	ebx
		pop	eax
.next_byte0:
		inc	esi
		add	eax,2
		inc	ebx
		jmp	near .byte_loop
.next_byte2:
		add	edi,64
		jmp	.next_byte0

; インターレース分をスキップ
		align	4
.next_line:
		pop	ebp
		pop	edx
		pop	eax
.next_line2:
		add	edi,[ebp+12]
		add	edi,2560
; 次のラインへ
		add	edi,[ebp+12]
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
; DirectDrawレンダリング
;
; static void Render256k(LPVOID lpSurface, LONG lPitch, int first, int last)
; lpSurface	ebp+8
; lPitch	ebp+12
; first		ebp+16
; last		ebp+20
;
		align	4
_Render256kTc32DD:
		push	ebp
		mov	ebp,esp
		push	ebx
		push	esi
		push	edi
; サーフェイス設定
		mov	edi,[ebp+8]
		mov	eax,[ebp+12]
		imul	eax,[ebp+16]
		add	edi,eax
		add	edi,eax
; ピッチを前もって計算
		mov	esi,[ebp+12]
		sub	esi,2560
		mov	[ebp+12],esi
; VRAMアドレス設定
		mov	esi,[_vram_c]
		add	esi,0x0000c000
		mov	ecx,[ebp+16]
		mov	eax,ecx
		imul	ecx,40
		add	esi,ecx
; ライン数設定
		mov	edx,[ebp+20]
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
		cmp	byte [_DDDrawFlag + ebx], 0
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
		test	byte [esp+24+28],0x20
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
		test	byte [esp+24+28],0x40
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
		test	byte [esp+24+28],0x10
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
.next_byte0:
		inc	esi
		add	ebx,2
		dec	ecx
		jnz	near .byte_loop
		jmp	.next_line
;
.next_byte2:
		add	edi,64
		jmp	.next_byte0

.next_line:
		pop	edx
		pop	eax
; インターレース分をスキップ
.next_line2:
		add	edi,[esp+12+12]
		add	edi,2560
; 次のラインへ
		add	edi,[esp+12+12]
		inc	eax
		dec	edx
		jnz	near .line_loop
; 終了
		pop	edi
		pop	esi
		pop	ebx
		pop	ebp
		ret

	%endif

	%if XM7_VER == 1

_Render640mTc32DD:
		push	ebp
		mov	ebp,esp
		push	ebx
		push	esi
		push	edi
; サーフェイス設定
		mov	edi,[ebp+8]
		mov	eax,[ebp+12]
		imul	eax,[ebp+16]
		add	edi,eax
		add	edi,eax
; ピッチを前もって計算
		mov	esi,[ebp+12]
		sub	esi,2560
		mov	[ebp+12],esi
; VRAMアドレス設定
		mov	esi,[_vram_c]
		add	esi,0x4000
		mov	ecx,[ebp+16]
		mov	eax,ecx
		imul	ecx,80
		add	esi,ecx
; ライン数設定
		mov	edx,[ebp+20]
		sub	edx,eax
		jz	near .exit
; １ラインループ
		align	4
.line_loop:
		lea	ecx,[eax*4]
		push	eax
		and	ecx,0xfffffff0
		push	edx
		lea	ecx,[ecx*4+ecx]
		mov	ebx,80
; １バイトループ
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
; 偶数ライン
; bit7
		xor	ecx,ecx
		add	al,al
		adc	ecx,ecx
		add	ah,ah
		adc	ecx,ecx
		add	dh,dh
		adc	ecx,ecx
		mov	ecx,[_rgbTTLDD+ecx*4]
		mov	[edi],ecx
; bit6
		xor	ecx,ecx
		add	al,al
		adc	ecx,ecx
		add	ah,ah
		adc	ecx,ecx
		add	dh,dh
		adc	ecx,ecx
		mov	ecx,[_rgbTTLDD+ecx*4]
		mov	[edi+4],ecx
; bit5
		xor	ecx,ecx
		add	al,al
		adc	ecx,ecx
		add	ah,ah
		adc	ecx,ecx
		add	dh,dh
		adc	ecx,ecx
		mov	ecx,[_rgbTTLDD+ecx*4]
		mov	[edi+8],ecx
; bit4
		xor	ecx,ecx
		add	al,al
		adc	ecx,ecx
		add	ah,ah
		adc	ecx,ecx
		add	dh,dh
		adc	ecx,ecx
		mov	ecx,[_rgbTTLDD+ecx*4]
		mov	[edi+12],ecx
; bit3
		xor	ecx,ecx
		add	al,al
		adc	ecx,ecx
		add	ah,ah
		adc	ecx,ecx
		add	dh,dh
		adc	ecx,ecx
		mov	ecx,[_rgbTTLDD+ecx*4]
		mov	[edi+16],ecx
; bit2
		xor	ecx,ecx
		add	al,al
		adc	ecx,ecx
		add	ah,ah
		adc	ecx,ecx
		add	dh,dh
		adc	ecx,ecx
		mov	ecx,[_rgbTTLDD+ecx*4]
		mov	[edi+20],ecx
; bit1
		xor	ecx,ecx
		add	al,al
		adc	ecx,ecx
		add	ah,ah
		adc	ecx,ecx
		add	dh,dh
		adc	ecx,ecx
		mov	ecx,[_rgbTTLDD+ecx*4]
		mov	[edi+24],ecx
; bit0
		xor	ecx,ecx
		add	al,al
		adc	ecx,ecx
		add	ah,ah
		adc	ecx,ecx
		add	dh,dh
		adc	ecx,ecx
		mov	ecx,[_rgbTTLDD+ecx*4]
		mov	[edi+28],ecx
;
; 奇数ライン
		add	edi, 2560
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
		mov	ecx,[_rgbTTLDD+ecx*4+32]
		mov	[edi],ecx
; bit6
		xor	ecx,ecx
		add	al,al
		adc	ecx,ecx
		add	ah,ah
		adc	ecx,ecx
		add	dh,dh
		adc	ecx,ecx
		mov	ecx,[_rgbTTLDD+ecx*4+32]
		mov	[edi+4],ecx
; bit5
		xor	ecx,ecx
		add	al,al
		adc	ecx,ecx
		add	ah,ah
		adc	ecx,ecx
		add	dh,dh
		adc	ecx,ecx
		mov	ecx,[_rgbTTLDD+ecx*4+32]
		mov	[edi+8],ecx
; bit4
		xor	ecx,ecx
		add	al,al
		adc	ecx,ecx
		add	ah,ah
		adc	ecx,ecx
		add	dh,dh
		adc	ecx,ecx
		mov	ecx,[_rgbTTLDD+ecx*4+32]
		mov	[edi+12],ecx
; bit3
		xor	ecx,ecx
		add	al,al
		adc	ecx,ecx
		add	ah,ah
		adc	ecx,ecx
		add	dh,dh
		adc	ecx,ecx
		mov	ecx,[_rgbTTLDD+ecx*4+32]
		mov	[edi+16],ecx
; bit2
		xor	ecx,ecx
		add	al,al
		adc	ecx,ecx
		add	ah,ah
		adc	ecx,ecx
		add	dh,dh
		adc	ecx,ecx
		mov	ecx,[_rgbTTLDD+ecx*4+32]
		mov	[edi+20],ecx
; bit1
		xor	ecx,ecx
		add	al,al
		adc	ecx,ecx
		add	ah,ah
		adc	ecx,ecx
		add	dh,dh
		adc	ecx,ecx
		mov	ecx,[_rgbTTLDD+ecx*4+32]
		mov	[edi+24],ecx
; bit0
		xor	ecx,ecx
		add	al,al
		adc	ecx,ecx
		add	ah,ah
		adc	ecx,ecx
		add	dh,dh
		adc	ecx,ecx
		mov	ecx,[_rgbTTLDD+ecx*4+32]
		mov	[edi+28],ecx
;
		sub	edi, 2560
		sub	edi, [ebp+12]
; 次のバイトへ
		pop	ecx
.next_byte:
		add	edi,32
		inc	esi
		inc	ecx
		dec	ebx
		jnz	near .byte_loop

; 次のラインへ
		pop	edx
		pop	eax
; インターレース分をスキップ
.next_line:
		add	edi,[ebp+12]
		add	edi,2560
		add	edi,[ebp+12]
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

; 640x200、デジタルモード (2バンク疑似400ライン)
; DirectDrawレンダリング
;
; static void Render640(LPVOID lpSurface, LONG lPitch, int first, int last)
; lpSurface	ebp+8
; lPitch	ebp+12
; first		ebp+16
; last		ebp+20
;
		align	4
_Render640cTc32DD:
		push	ebp
		mov	ebp,esp
		push	ebx
		push	esi
		push	edi
; サーフェイス設定
		mov	edi,[ebp+8]
		mov	eax,[ebp+12]
		imul	eax,[ebp+16]
		add	edi,eax
		add	edi,eax
; ピッチを前もって計算
		mov	esi,[ebp+12]
		sub	esi,2560
		mov	[ebp+12],esi
; VRAMアドレス設定
	%if XM7_VER >= 3
		mov	esi,[_vram_dblk]
	%else
		mov	esi,[_vram_c]
	%endif
		mov	ecx,[ebp+16]
		mov	eax,ecx
		imul	ecx,80
		add	esi,ecx
; ライン数設定
		mov	edx,[ebp+20]
		sub	edx,eax
		jz	near .exit
; １ラインループ
		align	4
.line_loop:
		lea	ecx,[eax*4]
		push	eax
		and	ecx,0xfffffff0
		push	edx
		lea	ecx,[ecx*4+ecx]
		mov	ebx,80
; １バイトループ
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
		mov	ecx,[_rgbTTLDD+ecx*4]
		mov	[edi],ecx
; bit6
		xor	ecx,ecx
		add	al,al
		adc	ecx,ecx
		add	ah,ah
		adc	ecx,ecx
		add	dh,dh
		adc	ecx,ecx
		mov	ecx,[_rgbTTLDD+ecx*4]
		mov	[edi+4],ecx
; bit5
		xor	ecx,ecx
		add	al,al
		adc	ecx,ecx
		add	ah,ah
		adc	ecx,ecx
		add	dh,dh
		adc	ecx,ecx
		mov	ecx,[_rgbTTLDD+ecx*4]
		mov	[edi+8],ecx
; bit4
		xor	ecx,ecx
		add	al,al
		adc	ecx,ecx
		add	ah,ah
		adc	ecx,ecx
		add	dh,dh
		adc	ecx,ecx
		mov	ecx,[_rgbTTLDD+ecx*4]
		mov	[edi+12],ecx
; bit3
		xor	ecx,ecx
		add	al,al
		adc	ecx,ecx
		add	ah,ah
		adc	ecx,ecx
		add	dh,dh
		adc	ecx,ecx
		mov	ecx,[_rgbTTLDD+ecx*4]
		mov	[edi+16],ecx
; bit2
		xor	ecx,ecx
		add	al,al
		adc	ecx,ecx
		add	ah,ah
		adc	ecx,ecx
		add	dh,dh
		adc	ecx,ecx
 		mov	ecx,[_rgbTTLDD+ecx*4]
		mov	[edi+20],ecx
; bit1
		xor	ecx,ecx
		add	al,al
		adc	ecx,ecx
		add	ah,ah
		adc	ecx,ecx
		add	dh,dh
		adc	ecx,ecx
		mov	ecx,[_rgbTTLDD+ecx*4]
		mov	[edi+24],ecx
; bit0
		xor	ecx,ecx
		add	al,al
		adc	ecx,ecx
		add	ah,ah
		adc	ecx,ecx
		add	dh,dh
		adc	ecx,ecx
		mov	ecx,[_rgbTTLDD+ecx*4]
		mov	[edi+28],ecx
;
		add	edi, 2560
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
		mov	ecx,[_rgbTTLDD+ecx*4]
		mov	[edi],ecx
; bit6
		xor	ecx,ecx
		add	al,al
		adc	ecx,ecx
		add	ah,ah
		adc	ecx,ecx
		add	dh,dh
		adc	ecx,ecx
		mov	ecx,[_rgbTTLDD+ecx*4]
		mov	[edi+4],ecx
; bit5
		xor	ecx,ecx
		add	al,al
		adc	ecx,ecx
		add	ah,ah
		adc	ecx,ecx
		add	dh,dh
		adc	ecx,ecx
		mov	ecx,[_rgbTTLDD+ecx*4]
		mov	[edi+8],ecx
; bit4
		xor	ecx,ecx
		add	al,al
		adc	ecx,ecx
		add	ah,ah
		adc	ecx,ecx
		add	dh,dh
		adc	ecx,ecx
		mov	ecx,[_rgbTTLDD+ecx*4]
		mov	[edi+12],ecx
; bit3
		xor	ecx,ecx
		add	al,al
		adc	ecx,ecx
		add	ah,ah
		adc	ecx,ecx
		add	dh,dh
		adc	ecx,ecx
		mov	ecx,[_rgbTTLDD+ecx*4]
		mov	[edi+16],ecx
; bit2
		xor	ecx,ecx
		add	al,al
		adc	ecx,ecx
		add	ah,ah
		adc	ecx,ecx
		add	dh,dh
		adc	ecx,ecx
 		mov	ecx,[_rgbTTLDD+ecx*4]
		mov	[edi+20],ecx
; bit1
		xor	ecx,ecx
		add	al,al
		adc	ecx,ecx
		add	ah,ah
		adc	ecx,ecx
		add	dh,dh
		adc	ecx,ecx
		mov	ecx,[_rgbTTLDD+ecx*4]
		mov	[edi+24],ecx
; bit0
		xor	ecx,ecx
		add	al,al
		adc	ecx,ecx
		add	ah,ah
		adc	ecx,ecx
		add	dh,dh
		adc	ecx,ecx
		mov	ecx,[_rgbTTLDD+ecx*4]
		mov	[edi+28],ecx
;
		sub	edi, 2560
		sub	edi, [ebp+12]
; 次のバイトへ
		pop	ecx
.next_byte:
		add	edi,32
		inc	esi
		inc	ecx
		dec	ebx
		jnz	near .byte_loop

; 次のラインへ
		pop	edx
		pop	eax
; インターレース分をスキップ
.next_line:
		add	edi,[ebp+12]
		add	edi,2560
		add	edi,[ebp+12]
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

	%if XM7_VER >= 3

;
; 640x200、デジタルモード (2バンク疑似400ライン)
; DirectDrawレンダリング(Window)
;
; static void Render640w(LPVOID lpSurface, LONG lPitch,
;			int first, int last, int firstx, int lastx)
; lpSurface	ebp+8
; lPitch	ebp+12
; first		ebp+16
; last		ebp+20
;
		align	4
_Render640cwTc32DD:
		push	ebp
		mov	ebp,esp
		push	ebx
		push	esi
		push	edi
; サーフェイス設定
		mov	edi,[ebp+8]
		mov	eax,[ebp+12]
		imul	eax,[ebp+16]
		add	edi,eax
		add	edi,eax
; ピッチを前もって計算
		mov	esi,[ebp+12]
		sub	esi,2560
		mov	[ebp+12],esi
; VRAMアドレス設定
		mov	esi,0x8000
		mov	ecx,[ebp+16]
		mov	eax,ecx
		imul	ecx,80
		add	esi,ecx
; ライン数設定
		mov	edx,[ebp+20]
		sub	edx,eax
		jz	near .exit
; ウィンドウ切り替えポインタ設定
		shr	dword [ebp+24],3
		shr	dword [ebp+28],3
		mov	cl,[ebp+24]
		mov	ch,[ebp+28]
; １ラインループ
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
		mov	ebx,[ebx*4+_rgbTTLDD]
		mov	[edi],ebx
; bit 6
		xor	ebx,ebx
		add	al,al
		adc	ebx,ebx
		add	ah,ah
		adc	ebx,ebx
		add	dl,dl
		adc	ebx,ebx
		mov	ebx,[ebx*4+_rgbTTLDD]
		mov	[edi+4],ebx
; bit 5
		xor	ebx,ebx
		add	al,al
		adc	ebx,ebx
		add	ah,ah
		adc	ebx,ebx
		add	dl,dl
		adc	ebx,ebx
		mov	ebx,[ebx*4+_rgbTTLDD]
		mov	[edi+8],ebx
; bit 4
		xor	ebx,ebx
		add	al,al
		adc	ebx,ebx
		add	ah,ah
		adc	ebx,ebx
		add	dl,dl
		adc	ebx,ebx
		mov	ebx,[ebx*4+_rgbTTLDD]
		mov	[edi+12],ebx
; bit 3
		xor	ebx,ebx
		add	al,al
		adc	ebx,ebx
		add	ah,ah
		adc	ebx,ebx
		add	dl,dl
		adc	ebx,ebx
		mov	ebx,[ebx*4+_rgbTTLDD]
		mov	[edi+16],ebx
; bit 2
		xor	ebx,ebx
		add	al,al
		adc	ebx,ebx
		add	ah,ah
		adc	ebx,ebx
		add	dl,dl
		adc	ebx,ebx
		mov	ebx,[ebx*4+_rgbTTLDD]
		mov	[edi+20],ebx
; bit 1
		xor	ebx,ebx
		add	al,al
		adc	ebx,ebx
		add	ah,ah
		adc	ebx,ebx
		add	dl,dl
		adc	ebx,ebx
		mov	ebx,[ebx*4+_rgbTTLDD]
		mov	[edi+24],ebx
; bit 0
		xor	ebx,ebx
		add	al,al
		adc	ebx,ebx
		add	ah,ah
		adc	ebx,ebx
		add	dl,dl
		adc	ebx,ebx
		mov	ebx,[ebx*4+_rgbTTLDD]
		mov	[edi+28],ebx
;
		mov	al,[esi+ebp+0xc000]
		mov	ah,[esi+ebp+0x4000]
		mov	dl,[esi+ebp-0x4000]
		add	edi, 2560
		add	edi, [esp+12+32]
; bit 7
		xor	ebx,ebx
		add	al,al
		adc	ebx,ebx
		add	ah,ah
		adc	ebx,ebx
		add	dl,dl
		adc	ebx,ebx
		mov	ebx,[ebx*4+_rgbTTLDD]
		mov	[edi],ebx
; bit 6
		xor	ebx,ebx
		add	al,al
		adc	ebx,ebx
		add	ah,ah
		adc	ebx,ebx
		add	dl,dl
		adc	ebx,ebx
		mov	ebx,[ebx*4+_rgbTTLDD]
		mov	[edi+4],ebx
; bit 5
		xor	ebx,ebx
		add	al,al
		adc	ebx,ebx
		add	ah,ah
		adc	ebx,ebx
		add	dl,dl
		adc	ebx,ebx
		mov	ebx,[ebx*4+_rgbTTLDD]
		mov	[edi+8],ebx
; bit 4
		xor	ebx,ebx
		add	al,al
		adc	ebx,ebx
		add	ah,ah
		adc	ebx,ebx
		add	dl,dl
		adc	ebx,ebx
		mov	ebx,[ebx*4+_rgbTTLDD]
		mov	[edi+12],ebx
; bit 3
		xor	ebx,ebx
		add	al,al
		adc	ebx,ebx
		add	ah,ah
		adc	ebx,ebx
		add	dl,dl
		adc	ebx,ebx
		mov	ebx,[ebx*4+_rgbTTLDD]
		mov	[edi+16],ebx
; bit 2
		xor	ebx,ebx
		add	al,al
		adc	ebx,ebx
		add	ah,ah
		adc	ebx,ebx
		add	dl,dl
		adc	ebx,ebx
		mov	ebx,[ebx*4+_rgbTTLDD]
		mov	[edi+20],ebx
; bit 1
		xor	ebx,ebx
		add	al,al
		adc	ebx,ebx
		add	ah,ah
		adc	ebx,ebx
		add	dl,dl
		adc	ebx,ebx
		mov	ebx,[ebx*4+_rgbTTLDD]
		mov	[edi+24],ebx
; bit 0
		xor	ebx,ebx
		add	al,al
		adc	ebx,ebx
		add	ah,ah
		adc	ebx,ebx
		add	dl,dl
		adc	ebx,ebx
		mov	ebx,[ebx*4+_rgbTTLDD]
		mov	[edi+28],ebx
;
		sub	edi, 2560
		sub	edi, [esp+12+32]
; 次のバイトへ
		pop	ebx
		pop	eax
.next_byte:
		inc	esi
		add	edi,32
		inc	eax
		inc	ebx
		jmp	near .byte_loop

; インターレース分をスキップ
		align	4
.next_line:
		pop	ebp
		pop	edx
		pop	eax
.next_line2:
; 次のラインへ
		add	edi,[ebp+12]
		add	edi, 2560
		add	edi,[ebp+12]
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
