;
; FM-7 EMULATOR "XM7"
;
; Copyright (C) 1999-2017 ＰＩ．(Twitter:@xm6_original)
; Copyright (C) 2001-2017 Ryu Takegami (Twitter:@RyuTakegami)
;
; [ Win32API レンダリング(DirectDraw・24bit TrueColor) ]
;
; RHG履歴
;	2002.06.21		更新が必要な部分だけレンダリングするように変更
;	2003.02.11		フカーツ
;					8色モードレンダラ(200line/400line)を統合
;	2010.01.13		TrueColor時の輝度変換をテーブル化
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
		global	_Render640Tc24DD2
		global	_Render640wTc24DD2
		global	_Render320Tc24DD
		global	_Render320wTc24DD
		global	_Render256kTc24DD
		global	_Render640cTc24DD
		global	_Render640cwTc24DD

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
_Render640Tc24DD2:
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
		sub	esi,1920
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
		mov	dl,[_rgbTTLDD+ecx*4+2]
		mov	cx,[_rgbTTLDD+ecx*4]
		mov	[edi],cx
		mov	[edi+2],dl
; bit6
		xor	ecx,ecx
		add	al,al
		adc	ecx,ecx
		add	ah,ah
		adc	ecx,ecx
		add	dh,dh
		adc	ecx,ecx
		mov	dl,[_rgbTTLDD+ecx*4]
		mov	cx,[_rgbTTLDD+ecx*4+1]
		mov	[edi+3],dl
		mov	[edi+4],cx
; bit5
		xor	ecx,ecx
		add	al,al
		adc	ecx,ecx
		add	ah,ah
		adc	ecx,ecx
		add	dh,dh
		adc	ecx,ecx
		mov	dl,[_rgbTTLDD+ecx*4+2]
		mov	cx,[_rgbTTLDD+ecx*4]
		mov	[edi+6],cx
		mov	[edi+8],dl
; bit4
		xor	ecx,ecx
		add	al,al
		adc	ecx,ecx
		add	ah,ah
		adc	ecx,ecx
		add	dh,dh
		adc	ecx,ecx
		mov	dl,[_rgbTTLDD+ecx*4]
		mov	cx,[_rgbTTLDD+ecx*4+1]
		mov	[edi+9],dl
		mov	[edi+10],cx
; bit3
		xor	ecx,ecx
		add	al,al
		adc	ecx,ecx
		add	ah,ah
		adc	ecx,ecx
		add	dh,dh
		adc	ecx,ecx
		mov	dl,[_rgbTTLDD+ecx*4+2]
		mov	cx,[_rgbTTLDD+ecx*4]
		mov	[edi+12],cx
		mov	[edi+14],dl
; bit2
		xor	ecx,ecx
		add	al,al
		adc	ecx,ecx
		add	ah,ah
		adc	ecx,ecx
		add	dh,dh
		adc	ecx,ecx
		mov	dl,[_rgbTTLDD+ecx*4]
		mov	cx,[_rgbTTLDD+ecx*4+1]
		mov	[edi+15],dl
		mov	[edi+16],cx
; bit1
		xor	ecx,ecx
		add	al,al
		adc	ecx,ecx
		add	ah,ah
		adc	ecx,ecx
		add	dh,dh
		adc	ecx,ecx
		mov	dl,[_rgbTTLDD+ecx*4+2]
		mov	cx,[_rgbTTLDD+ecx*4]
		mov	[edi+18],cx
		mov	[edi+20],dl
; bit0
		xor	ecx,ecx
		add	al,al
		adc	ecx,ecx
		add	ah,ah
		adc	ecx,ecx
		add	dh,dh
		adc	ecx,ecx
		mov	dl,[_rgbTTLDD+ecx*4]
		mov	cx,[_rgbTTLDD+ecx*4+1]
		mov	[edi+21],dl
		mov	[edi+22],cx
; 次のバイトへ
		pop	ecx
.next_byte:
		add	edi,24
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
_Render640wTc24DD2:
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
		sub	esi,1920
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
		mov	dh,[_rgbTTLDD+ebx*4+2]
		mov	bx,[_rgbTTLDD+ebx*4]
		mov	[edi],bx
		mov	[edi+2],dh
; bit 6
		xor	ebx,ebx
		add	al,al
		adc	ebx,ebx
		add	ah,ah
		adc	ebx,ebx
		add	dl,dl
		adc	ebx,ebx
		mov	dh,[_rgbTTLDD+ebx*4]
		mov	bx,[_rgbTTLDD+ebx*4+1]
		mov	[edi+3],dh
		mov	[edi+4],bx
; bit 5
		xor	ebx,ebx
		add	al,al
		adc	ebx,ebx
		add	ah,ah
		adc	ebx,ebx
		add	dl,dl
		adc	ebx,ebx
		mov	dh,[_rgbTTLDD+ebx*4+2]
		mov	bx,[_rgbTTLDD+ebx*4]
		mov	[edi+6],bx
		mov	[edi+8],dh
; bit 4
		xor	ebx,ebx
		add	al,al
		adc	ebx,ebx
		add	ah,ah
		adc	ebx,ebx
		add	dl,dl
		adc	ebx,ebx
		mov	dh,[_rgbTTLDD+ebx*4]
		mov	bx,[_rgbTTLDD+ebx*4+1]
		mov	[edi+9],dh
		mov	[edi+10],bx
; bit 3
		xor	ebx,ebx
		add	al,al
		adc	ebx,ebx
		add	ah,ah
		adc	ebx,ebx
		add	dl,dl
		adc	ebx,ebx
		mov	dh,[_rgbTTLDD+ebx*4+2]
		mov	bx,[_rgbTTLDD+ebx*4]
		mov	[edi+12],bx
		mov	[edi+14],dh
; bit 2
		xor	ebx,ebx
		add	al,al
		adc	ebx,ebx
		add	ah,ah
		adc	ebx,ebx
		add	dl,dl
		adc	ebx,ebx
		mov	dh,[_rgbTTLDD+ebx*4]
		mov	bx,[_rgbTTLDD+ebx*4+1]
		mov	[edi+15],dh
		mov	[edi+16],bx
; bit 1
		xor	ebx,ebx
		add	al,al
		adc	ebx,ebx
		add	ah,ah
		adc	ebx,ebx
		add	dl,dl
		adc	ebx,ebx
		mov	dh,[_rgbTTLDD+ebx*4+2]
		mov	bx,[_rgbTTLDD+ebx*4]
		mov	[edi+18],bx
		mov	[edi+20],dh
; bit 0
		xor	ebx,ebx
		add	al,al
		adc	ebx,ebx
		add	ah,ah
		adc	ebx,ebx
		add	dl,dl
		adc	ebx,ebx
		mov	dh,[_rgbTTLDD+ebx*4]
		mov	bx,[_rgbTTLDD+ebx*4+1]
		mov	[edi+21],dh
		mov	[edi+22],bx
; 次のバイトへ
		pop	ebx
		pop	eax
.next_byte:
		inc	esi
		add	edi,24
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


;
; 320x200、アナログモード
; DirectDrawレンダリング(24bit)
;
; static void Render320(LPVOID lpSurface, LONG lPitch, int first, int last)
; lpSurface	ebp+8
; lPitch	ebp+12
; first		ebp+16
; last		ebp+20
;
		align	4
_Render320Tc24DD:
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
		sub	esi,1920
		mov	[ebp+12],esi
; VRAMアドレス設定
		mov	esi,[_vram_dptr]
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
		mov	al,[esi+0x4000]
		mov	ah,[esi+0x6000]
		mov	dl,[esi+0x8000]
		mov	dh,[esi+0xa000]
		mov	bh,8
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
		mov	[edi],cx
		add	edi,6
		dec	bh
		jnz	.g_loop
		sub	edi,48
; R
		mov	al,[esi-0x4000]
		mov	ah,[esi-0x2000]
		mov	dl,[esi-0x0000]
		mov	dh,[esi+0x2000]
		mov	bh,8
; Rループ
.r_loop:
		mov	cx,[edi]
		add	al,al
		adc	ecx,ecx
		add	ah,ah
		adc	ecx,ecx
		add	dl,dl
		adc	ecx,ecx
		add	dh,dh
		adc	ecx,ecx
		mov	[edi],cx
		add	edi,6
		dec	bh
		jnz	.r_loop
		sub	edi,48
; B
		mov	al,[esi-0xc000]
		mov	ah,[esi-0xa000]
		mov	dl,[esi-0x8000]
		mov	dh,[esi-0x6000]
		mov	bh,8
; Bループ
.b_loop:
		mov	cx,[edi]
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
		mov	bl,[ecx*4+_rgbAnalogDD+2]
		mov	ecx,[ecx*4+_rgbAnalogDD]
		mov	[edi],cx
		mov	[edi+2],bl
		mov	[edi+3],cl
		mov	[edi+5],bl
		mov	[edi+4],ch
		add	edi,6
		dec	bh
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
.next_byte2:
		add	edi,48
		jmp	.next_byte0

.next_line:
		pop	edx
		pop	eax
; インターレース分をスキップ
.next_line2:
		add	edi,[ebp+12]
		add	edi,1920
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
; 320x200、アナログモード
; DirectDrawレンダリング(Window,24bit)
;
; static void Render320w(LPVOID lpSurface, LONG lPitch,
;			int first, int last, int firstx, int lastx)
; lpSurface	ebp+8
; lPitch	ebp+12
; first		ebp+16
; last		ebp+20
;
		align	4
_Render320wTc24DD:
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
		sub	esi,1920
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
		mov	bh,8
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
		mov	[edi],cx
		add	edi,6
		dec	bh
		jnz	.g_loop
		sub	edi,48
; R
		mov	al,[esi+ebp-0x4000]
		mov	ah,[esi+ebp-0x2000]
		mov	dl,[esi+ebp-0x0000]
		mov	dh,[esi+ebp+0x2000]
		mov	bh,8
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
		mov	[edi],cx
		add	edi,6
		dec	bh
		jnz	.r_loop
		sub	edi,48
; B
		mov	al,[esi+ebp-0xc000]
		mov	ah,[esi+ebp-0xa000]
		mov	dl,[esi+ebp-0x8000]
		mov	dh,[esi+ebp-0x6000]
		mov	bh,8
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
		mov	bl,[ecx*4+_rgbAnalogDD+2]
		mov	ecx,[ecx*4+_rgbAnalogDD]
		mov	[edi],cx
		mov	[edi+2],bl
		mov	[edi+3],cl
		mov	[edi+5],bl
		mov	[edi+4],ch
		add	edi,6
		dec	bh
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
		add	edi,48
		jmp	.next_byte0

; インターレース分をスキップ
		align	4
.next_line:
		pop	ebp
		pop	edx
		pop	eax
.next_line2:
		add	edi,[ebp+12]
		add	edi,1920
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
; DirectDrawレンダリング(24bit)
;
; static void Render256k(LPVOID lpSurface, LONG lPitch, int first, int last)
; lpSurface	ebp+8
; lPitch	ebp+12
; first		ebp+16
; last		ebp+20
;
		align	4
_Render256kTc24DD:
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
		sub	esi,1920
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

; 48byte clear
		xor		ecx,ecx
		mov		edx,edi
		mov		eax,12

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
		mov	[edi+5],cl
		add	edi,6
		dec	ebp
		jnz	.r_loop
		sub	edi,48

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
		mov	[edi+4],cl
		add	edi,6
		dec	ebp
		jnz	.g_loop
		sub	edi,48

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
		mov	[edi+3],cl
		add	edi,6
		dec	ebp
		jnz	.b_loop
		jmp	near .next_byte
; B SKIP
.skip_blue:
		add	edi,48

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
		add	edi,48
		jmp	.next_byte0

.next_line:
		pop	edx
		pop	eax
; インターレース分をスキップ
.next_line2:
		add	edi,[esp+12+12]
		add	edi,1920
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
_Render640cTc24DD:
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
		sub	esi,1920
		mov	[ebp+12],esi
; VRAMアドレス設定
		mov	esi,[_vram_dblk]
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
		mov	dl,[_rgbTTLDD+ecx*4+2]
		mov	cx,[_rgbTTLDD+ecx*4]
		mov	[edi],cx
		mov	[edi+2],dl
; bit6
		xor	ecx,ecx
		add	al,al
		adc	ecx,ecx
		add	ah,ah
		adc	ecx,ecx
		add	dh,dh
		adc	ecx,ecx
		mov	dl,[_rgbTTLDD+ecx*4]
		mov	cx,[_rgbTTLDD+ecx*4+1]
		mov	[edi+3],dl
		mov	[edi+4],cx
; bit5
		xor	ecx,ecx
		add	al,al
		adc	ecx,ecx
		add	ah,ah
		adc	ecx,ecx
		add	dh,dh
		adc	ecx,ecx
		mov	dl,[_rgbTTLDD+ecx*4+2]
		mov	cx,[_rgbTTLDD+ecx*4]
		mov	[edi+6],cx
		mov	[edi+8],dl
; bit4
		xor	ecx,ecx
		add	al,al
		adc	ecx,ecx
		add	ah,ah
		adc	ecx,ecx
		add	dh,dh
		adc	ecx,ecx
		mov	dl,[_rgbTTLDD+ecx*4]
		mov	cx,[_rgbTTLDD+ecx*4+1]
		mov	[edi+9],dl
		mov	[edi+10],cx
; bit3
		xor	ecx,ecx
		add	al,al
		adc	ecx,ecx
		add	ah,ah
		adc	ecx,ecx
		add	dh,dh
		adc	ecx,ecx
		mov	dl,[_rgbTTLDD+ecx*4+2]
		mov	cx,[_rgbTTLDD+ecx*4]
		mov	[edi+12],cx
		mov	[edi+14],dl
; bit2
		xor	ecx,ecx
		add	al,al
		adc	ecx,ecx
		add	ah,ah
		adc	ecx,ecx
		add	dh,dh
		adc	ecx,ecx
		mov	dl,[_rgbTTLDD+ecx*4]
		mov	cx,[_rgbTTLDD+ecx*4+1]
		mov	[edi+15],dl
		mov	[edi+16],cx
; bit1
		xor	ecx,ecx
		add	al,al
		adc	ecx,ecx
		add	ah,ah
		adc	ecx,ecx
		add	dh,dh
		adc	ecx,ecx
		mov	dl,[_rgbTTLDD+ecx*4+2]
		mov	cx,[_rgbTTLDD+ecx*4]
		mov	[edi+18],cx
		mov	[edi+20],dl
; bit0
		xor	ecx,ecx
		add	al,al
		adc	ecx,ecx
		add	ah,ah
		adc	ecx,ecx
		add	dh,dh
		adc	ecx,ecx
		mov	dl,[_rgbTTLDD+ecx*4]
		mov	cx,[_rgbTTLDD+ecx*4+1]
		mov	[edi+21],dl
		mov	[edi+22],cx
;
		add	edi, 1920
		add	edi, [ebp+12]
		mov	al,[esi+0xc000]
		mov	ah,[esi+0x4000]
		mov	dh,[esi-0x4000]
; bit7
		xor	ecx,ecx
		add	al,al
		adc	ecx,ecx
		add	ah,ah
		adc	ecx,ecx
		add	dh,dh
		adc	ecx,ecx
		mov	dl,[_rgbTTLDD+ecx*4+2]
		mov	cx,[_rgbTTLDD+ecx*4]
		mov	[edi],cx
		mov	[edi+2],dl
; bit6
		xor	ecx,ecx
		add	al,al
		adc	ecx,ecx
		add	ah,ah
		adc	ecx,ecx
		add	dh,dh
		adc	ecx,ecx
		mov	dl,[_rgbTTLDD+ecx*4]
		mov	cx,[_rgbTTLDD+ecx*4+1]
		mov	[edi+3],dl
		mov	[edi+4],cx
; bit5
		xor	ecx,ecx
		add	al,al
		adc	ecx,ecx
		add	ah,ah
		adc	ecx,ecx
		add	dh,dh
		adc	ecx,ecx
		mov	dl,[_rgbTTLDD+ecx*4+2]
		mov	cx,[_rgbTTLDD+ecx*4]
		mov	[edi+6],cx
		mov	[edi+8],dl
; bit4
		xor	ecx,ecx
		add	al,al
		adc	ecx,ecx
		add	ah,ah
		adc	ecx,ecx
		add	dh,dh
		adc	ecx,ecx
		mov	dl,[_rgbTTLDD+ecx*4]
		mov	cx,[_rgbTTLDD+ecx*4+1]
		mov	[edi+9],dl
		mov	[edi+10],cx
; bit3
		xor	ecx,ecx
		add	al,al
		adc	ecx,ecx
		add	ah,ah
		adc	ecx,ecx
		add	dh,dh
		adc	ecx,ecx
		mov	dl,[_rgbTTLDD+ecx*4+2]
		mov	cx,[_rgbTTLDD+ecx*4]
		mov	[edi+12],cx
		mov	[edi+14],dl
; bit2
		xor	ecx,ecx
		add	al,al
		adc	ecx,ecx
		add	ah,ah
		adc	ecx,ecx
		add	dh,dh
		adc	ecx,ecx
		mov	dl,[_rgbTTLDD+ecx*4]
		mov	cx,[_rgbTTLDD+ecx*4+1]
		mov	[edi+15],dl
		mov	[edi+16],cx
; bit1
		xor	ecx,ecx
		add	al,al
		adc	ecx,ecx
		add	ah,ah
		adc	ecx,ecx
		add	dh,dh
		adc	ecx,ecx
		mov	dl,[_rgbTTLDD+ecx*4+2]
		mov	cx,[_rgbTTLDD+ecx*4]
		mov	[edi+18],cx
		mov	[edi+20],dl
; bit0
		xor	ecx,ecx
		add	al,al
		adc	ecx,ecx
		add	ah,ah
		adc	ecx,ecx
		add	dh,dh
		adc	ecx,ecx
		mov	dl,[_rgbTTLDD+ecx*4]
		mov	cx,[_rgbTTLDD+ecx*4+1]
		mov	[edi+21],dl
		mov	[edi+22],cx
;
		sub	edi, 1920
		sub	edi, [ebp+12]
; 次のバイトへ
		pop	ecx
.next_byte:
		add	edi,24
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
		add	edi,1920
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
_Render640cwTc24DD:
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
		sub	esi,1920
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
		mov	dh,[_rgbTTLDD+ebx*4+2]
		mov	bx,[_rgbTTLDD+ebx*4]
		mov	[edi],bx
		mov	[edi+2],dh
; bit 6
		xor	ebx,ebx
		add	al,al
		adc	ebx,ebx
		add	ah,ah
		adc	ebx,ebx
		add	dl,dl
		adc	ebx,ebx
		mov	dh,[_rgbTTLDD+ebx*4]
		mov	bx,[_rgbTTLDD+ebx*4+1]
		mov	[edi+3],dh
		mov	[edi+4],bx
; bit 5
		xor	ebx,ebx
		add	al,al
		adc	ebx,ebx
		add	ah,ah
		adc	ebx,ebx
		add	dl,dl
		adc	ebx,ebx
		mov	dh,[_rgbTTLDD+ebx*4+2]
		mov	bx,[_rgbTTLDD+ebx*4]
		mov	[edi+6],bx
		mov	[edi+8],dh
; bit 4
		xor	ebx,ebx
		add	al,al
		adc	ebx,ebx
		add	ah,ah
		adc	ebx,ebx
		add	dl,dl
		adc	ebx,ebx
		mov	dh,[_rgbTTLDD+ebx*4]
		mov	bx,[_rgbTTLDD+ebx*4+1]
		mov	[edi+9],dh
		mov	[edi+10],bx
; bit 3
		xor	ebx,ebx
		add	al,al
		adc	ebx,ebx
		add	ah,ah
		adc	ebx,ebx
		add	dl,dl
		adc	ebx,ebx
		mov	dh,[_rgbTTLDD+ebx*4+2]
		mov	bx,[_rgbTTLDD+ebx*4]
		mov	[edi+12],bx
		mov	[edi+14],dh
; bit 2
		xor	ebx,ebx
		add	al,al
		adc	ebx,ebx
		add	ah,ah
		adc	ebx,ebx
		add	dl,dl
		adc	ebx,ebx
		mov	dh,[_rgbTTLDD+ebx*4]
		mov	bx,[_rgbTTLDD+ebx*4+1]
		mov	[edi+15],dh
		mov	[edi+16],bx
; bit 1
		xor	ebx,ebx
		add	al,al
		adc	ebx,ebx
		add	ah,ah
		adc	ebx,ebx
		add	dl,dl
		adc	ebx,ebx
		mov	dh,[_rgbTTLDD+ebx*4+2]
		mov	bx,[_rgbTTLDD+ebx*4]
		mov	[edi+18],bx
		mov	[edi+20],dh
; bit 0
		xor	ebx,ebx
		add	al,al
		adc	ebx,ebx
		add	ah,ah
		adc	ebx,ebx
		add	dl,dl
		adc	ebx,ebx
		mov	dh,[_rgbTTLDD+ebx*4]
		mov	bx,[_rgbTTLDD+ebx*4+1]
		mov	[edi+21],dh
		mov	[edi+22],bx
;
		mov	al,[esi+ebp+0xc000]
		mov	ah,[esi+ebp+0x4000]
		mov	dl,[esi+ebp-0x4000]
		add	edi, 1920
		add	edi, [esp+12+32]
; bit 7
		xor	ebx,ebx
		add	al,al
		adc	ebx,ebx
		add	ah,ah
		adc	ebx,ebx
		add	dl,dl
		adc	ebx,ebx
		mov	dh,[_rgbTTLDD+ebx*4+2]
		mov	bx,[_rgbTTLDD+ebx*4]
		mov	[edi],bx
		mov	[edi+2],dh
; bit 6
		xor	ebx,ebx
		add	al,al
		adc	ebx,ebx
		add	ah,ah
		adc	ebx,ebx
		add	dl,dl
		adc	ebx,ebx
		mov	dh,[_rgbTTLDD+ebx*4]
		mov	bx,[_rgbTTLDD+ebx*4+1]
		mov	[edi+3],dh
		mov	[edi+4],bx
; bit 5
		xor	ebx,ebx
		add	al,al
		adc	ebx,ebx
		add	ah,ah
		adc	ebx,ebx
		add	dl,dl
		adc	ebx,ebx
		mov	dh,[_rgbTTLDD+ebx*4+2]
		mov	bx,[_rgbTTLDD+ebx*4]
		mov	[edi+6],bx
		mov	[edi+8],dh
; bit 4
		xor	ebx,ebx
		add	al,al
		adc	ebx,ebx
		add	ah,ah
		adc	ebx,ebx
		add	dl,dl
		adc	ebx,ebx
		mov	dh,[_rgbTTLDD+ebx*4]
		mov	bx,[_rgbTTLDD+ebx*4+1]
		mov	[edi+9],dh
		mov	[edi+10],bx
; bit 3
		xor	ebx,ebx
		add	al,al
		adc	ebx,ebx
		add	ah,ah
		adc	ebx,ebx
		add	dl,dl
		adc	ebx,ebx
		mov	dh,[_rgbTTLDD+ebx*4+2]
		mov	bx,[_rgbTTLDD+ebx*4]
		mov	[edi+12],bx
		mov	[edi+14],dh
; bit 2
		xor	ebx,ebx
		add	al,al
		adc	ebx,ebx
		add	ah,ah
		adc	ebx,ebx
		add	dl,dl
		adc	ebx,ebx
		mov	dh,[_rgbTTLDD+ebx*4]
		mov	bx,[_rgbTTLDD+ebx*4+1]
		mov	[edi+15],dh
		mov	[edi+16],bx
; bit 1
		xor	ebx,ebx
		add	al,al
		adc	ebx,ebx
		add	ah,ah
		adc	ebx,ebx
		add	dl,dl
		adc	ebx,ebx
		mov	dh,[_rgbTTLDD+ebx*4+2]
		mov	bx,[_rgbTTLDD+ebx*4]
		mov	[edi+18],bx
		mov	[edi+20],dh
; bit 0
		xor	ebx,ebx
		add	al,al
		adc	ebx,ebx
		add	ah,ah
		adc	ebx,ebx
		add	dl,dl
		adc	ebx,ebx
		mov	dh,[_rgbTTLDD+ebx*4]
		mov	bx,[_rgbTTLDD+ebx*4+1]
		mov	[edi+21],dh
		mov	[edi+22],bx
;
		sub	edi, 1920
		sub	edi, [esp+12+32]
; 次のバイトへ
		pop	ebx
		pop	eax
.next_byte:
		inc	esi
		add	edi,24
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
		add	edi, 1920
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
