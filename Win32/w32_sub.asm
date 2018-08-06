;
; FM-7 EMULATOR "XM7"
;
; Copyright (C) 1999-2017 ＰＩ．(Twitter:@xm6_original)
; Copyright (C) 2001-2017 Ryu Takegami (Twitter:@RyuTakegami)
;
; [ Win32 アセンブラサブ ]
;
; RHG履歴
;	2003.08.12		非MMX版波形クリッピング処理から分岐命令を排除(イミナシ)

;
; 外部定義
;
		section	.text class=CODE align=16 use32
		global	_CopySndBufMMX
		global	_CopySndBuf
		global	_CheckMMX
		global	_CheckCMOV

;
; サウンドバッファへのコピー (MMX)
;
; void CopySndBufMMX(DWORD *src, WORD *dest, int count)
; src		ebp+8
; dest		ebp+12
; count		ebp+16
;
		align	4
_CopySndBufMMX:
		push	ebp
		mov		ebp, esp
		push	edi
		push	edx

		mov		edx, [ebp + 8]
		mov		edi, [ebp + 12]
		mov		ecx, [ebp + 16]

		mov		eax, ecx
		and		eax, 1
		shr		ecx, 1
		je		.copysbm1

		align	4
.loop:
		add		edx, 8
		add		edi, 4
		movq	mm0, [edx - 8]
		packssdw	mm0, mm0
		movd	[edi - 4], mm0
		dec		ecx
		jnz		.loop

.copysbm1:
		and		eax, eax
		jz		.exit

		movq	mm0, [edx]
		packssdw	mm0, mm0
		movd	mm0, eax
		mov		[edi], ax

.exit:
		emms

		pop		edx
		pop		edi
		pop		ebp
		ret


;
; サウンドバッファへのコピー (バカ丸出し寸前コードｗ
;
; void CopySndBuf(DWORD *src, WORD *dest, int count)
; src		ebp+8
; dest		ebp+12
; count		ebp+16
;
		align	4
_CopySndBuf:
		push	ebp
		mov		ebp, esp
		push	esi
		push	edi
		push	ebx
		push	edx

		mov		edx, [ebp + 8]
		mov		edi, [ebp + 12]
		mov		ecx, [ebp + 16]

		and		ecx, ecx
		je		.exit

		align	4
.loop:
		mov		ebx, [edx]
		lea		esi, [ebx + 32768]
		add		edi, 2
		xor		eax, eax
		add		edx, 4
		test	esi, 0xFFFF0000
		setz	al
		mov		esi, ebx
		neg		eax
		and		ebx, eax
		shr		esi, 31
		not		eax
		add		esi, 32767
		and		esi, eax
		or		ebx, esi
		mov		[edi - 2], bx
		dec		ecx
		jnz		.loop

.exit:
		pop		edx
		pop		ebx
		pop		edi
		pop		esi
		pop		ebp
		ret


;
; MMX命令が使用可能かチェック
;
; BOOL CheckMMX(void)
;
		align	4
_CheckMMX:
		push	edi
		mov	edi,0x00800000
		jmp	_CheckInst

;
; CMOV命令が使用可能かチェック
;
; BOOL CheckCMOV(void)
;
		align	4
_CheckCMOV:
		push	edi
		mov	edi,0x00008000
		jmp	_CheckInst

;
; CheckMMX/CheckCMOV 共通処理
;
		align	4
_CheckInst:
		push	ebx
		push	edx
;	cpuid命令が使用可能かチェック
		pushfd
		pop	eax
		or	eax,0x00200000
		push	eax
		popfd
		pushfd
		pop	eax
		and	eax,0x00200000
		jz	.nommxinst

;	MMX命令がサポートされているかチェック
		mov	eax,1
		cpuid
		and	edx,edi
		jz	.nommxinst
		mov	eax,1
		jmp	.exit

.nommxinst:
		xor	eax,eax

.exit:
		pop	edx
		pop	ebx
		pop	edi

		ret


;
; プログラム終了
;
		end
