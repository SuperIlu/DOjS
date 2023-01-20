_TEXT	segment dword use32 public 'CODE'	;size is 338
_TEXT	ends
_DATA	segment dword use32 public 'DATA'	;size is 128
_DATA	ends
FLAT	group	

	public	_w32_in_checksum_fast
	public	__w32_in_checksum_fast
_TEXT	segment
	assume	CS:_TEXT
;; !\file chksum0.asm
;;
;; Copyright (c) 1985-1998 Microsoft Corporation
;; This file is part of the Microsoft Research IPv6 Network Protocol Stack.
;; You should have received a copy of the Microsoft End-User License Agreement
;; for this software along with this release; see the file "license.txt".
;; If not, please see http://www.research.microsoft.com/msripv6/license.htm,
;; or write to Microsoft Research, One Microsoft Way, Redmond, WA 98052-6399.
;;
;; Abstract:
;;
;; This module implements a function to compute the internet checksum
;; of a buffer.
;;
;; Apr 2000 G.Vanem:
;; - Changes for DOSX targets and various compiler naming styles.
;;
;
;ifdef DOSX    ; only for DOSX-targets
;
;PAGE 66, 132
;
;PUBLIC _w32_in_checksum_fast, __w32_in_checksum_fast
;
;LOOP_UNROLLING_BITS  equ  5
;LOOP_UNROLLING       equ  (1 SHL LOOP_UNROLLING_BITS)
;
;.386
;
;ifdef X32VM
;  .MODEL SMALL,C
;else
;  .MODEL FLAT,C
;endif
;
;.CODE
;
;
;buf  equ 12                        ; stack offset to source address
;len  equ 16                        ; stack offset to length in words
;
;to_checksum_last_word:
;       jmp  checksum_last_word
L0:		jmp	near ptr L123
;
;to_checksum_done:
;       jmp  checksum_done
L5:		jmp	near ptr L132
;
;to_checksum_dword_loop_done:
;       jmp  checksum_dword_loop_done
LA:		jmp	near ptr L11A
;
;;
;; WORD cdecl in_checksum_fast (const void *buf, unsigned len);
;;
;
;_w32_in_checksum_fast:
;__w32_in_checksum_fast:
;       push ebx
_w32_in_checksum_fast:
__w32_in_checksum_fast:
		push	EBX
;       push esi
		push	ESI
;
;       mov  ecx, [esp+len]          ; get length in bytes
		mov	ECX,010h[ESP]
;       sub  eax, eax                ; clear computed checksum
		sub	EAX,EAX
;       test ecx, ecx                ; any bytes to checksum at all?
		test	ECX,ECX
;       jz   short to_checksum_done  ; no bytes to checksum
		je	L5
;;
;; if the checksum buffer is not word aligned, then add the first byte of
;; the buffer to the input checksum.
;;
;       mov  esi, [esp+buf]          ; get source address
		mov	ESI,0Ch[ESP]
;       sub  edx, edx                ; set up to load word into EDX below
		sub	EDX,EDX
;       test esi, 1                  ; check if buffer word aligned
		test	ESI,1
;       jz   short checksum_word_aligned ; if zf, buffer word aligned
		je	L2F
;       mov  ah, [esi]               ; get first byte (we know we'll have
		mov	AH,[ESI]
;                                    ;  to swap at the end)
;       inc  esi                     ; increment buffer address
		inc	ESI
;       dec  ecx                     ; decrement number of bytes
		dec	ECX
;       jz   short to_checksum_done  ; if zf set, no more bytes
		je	L5
;
;;
;; If the buffer is not an even number of of bytes, then initialize
;; the computed checksum with the last byte of the buffer.
;;
;
;checksum_word_aligned:
;       shr  ecx, 1                  ; convert to word count
L2F:		shr	ECX,1
;       jnc  short checksum_start    ; if nc, even number of bytes
		jae	L38
;       mov  al, [esi+ecx*2]         ; initialize the computed checksum
		mov	AL,[ECX*2][ESI]
;       jz   short to_checksum_done  ; if zf set, no more bytes
		je	L5
;
;;
;; Compute checksum in large blocks of dwords, with one partial word up front if
;; necessary to get dword alignment, and another partial word at the end if
;; needed.
;;
;
;;
;; Compute checksum on the leading word, if that's necessary to get dword
;; alignment.
;;
;
;checksum_start:
;       test esi, 2                  ; check if source dword aligned
L38:		test	ESI,2
;       jz   short checksum_dword_aligned ; source is already dword aligned
		je	L49
;       mov  dx, [esi]               ; get first word to checksum
		mov	DX,[ESI]
;       add  esi, 2                  ; update source address
		add	ESI,2
;       add  eax, edx                ; update partial checksum
		add	EAX,EDX
;                                    ;  (no carry is possible, because EAX
;                                    ;  and EDX are both 16-bit values)
;       dec  ecx                     ; count off this word (zero case gets
		dec	ECX
;                                    ;  picked up below)
;
;;
;; Checksum as many words as possible by processing a dword at a time.
;;
;
;checksum_dword_aligned:
;       push ecx                     ; so we can tell if there's a trailing
L49:		push	ECX
;                                    ;  word later
;       shr  ecx, 1                  ; # of dwords to checksum
		shr	ECX,1
;       jz   short to_checksum_last_word ; no dwords to checksum
		je	L0
;
;       mov  edx, [esi]              ; preload the first dword
		mov	EDX,[ESI]
;       add  esi, 4                  ; point to the next dword
		add	ESI,4
;       dec  ecx                     ; count off the dword we just loaded
		dec	ECX
;       jz   short to_checksum_dword_loop_done
		je	LA
;                                    ; skip the loop if that was the only
;                                    ;  dword
;       mov  ebx, ecx                ; EBX = # of dwords left to checksum
		mov	EBX,ECX
;       add  ecx, LOOP_UNROLLING-1   ; round up loop count
		add	ECX,01Fh
;       shr  ecx, LOOP_UNROLLING_BITS; convert from word count to unrolled
		shr	ECX,5
;                                    ;  loop count
;       and  ebx, LOOP_UNROLLING-1   ; # of partial dwords to do in first
		and	EBX,01Fh
;                                    ;  loop
;       jz   short checksum_dword_loop ; special-case when no partial loop,
		je	L6E
;                                    ;  because fixup below doesn't work
;                                    ;  in that case (carry flag is
;                                    ;  cleared at this point, as required
;                                    ;  at loop entry)
;       lea  esi, [esi+ebx*4-(LOOP_UNROLLING*4)]
		lea	ESI,-080h[EBX*4][ESI]
;                                    ; adjust buffer pointer back to
;                                    ;  compensate for hardwired displacement
;                                    ;  at loop entry point
;                                    ; ***doesn't change carry flag***
;       jmp  loop_entries[ebx*4]     ; enter the loop to do the first,
		jmp	dword ptr FLAT:_DATA[00h][EBX*4]
;                                    ; partial iteration, after which we can
;                                    ; just do 64-word blocks
;                                    ; ***doesn't change carry flag***
;
;checksum_dword_loop:
;
;LOOP_ENTRY MACRO X
;      loop_entry_&X:
;      adc  eax, edx
L6E:		adc	EAX,EDX
;      mov  edx, [esi+X]
		mov	EDX,[ESI]
		adc	EAX,EDX
		mov	EDX,4[ESI]
		adc	EAX,EDX
		mov	EDX,8[ESI]
		adc	EAX,EDX
		mov	EDX,0Ch[ESI]
		adc	EAX,EDX
		mov	EDX,010h[ESI]
		adc	EAX,EDX
		mov	EDX,014h[ESI]
		adc	EAX,EDX
		mov	EDX,018h[ESI]
		adc	EAX,EDX
		mov	EDX,01Ch[ESI]
		adc	EAX,EDX
		mov	EDX,020h[ESI]
		adc	EAX,EDX
		mov	EDX,024h[ESI]
		adc	EAX,EDX
		mov	EDX,028h[ESI]
		adc	EAX,EDX
		mov	EDX,02Ch[ESI]
		adc	EAX,EDX
		mov	EDX,030h[ESI]
		adc	EAX,EDX
		mov	EDX,034h[ESI]
		adc	EAX,EDX
		mov	EDX,038h[ESI]
		adc	EAX,EDX
		mov	EDX,03Ch[ESI]
		adc	EAX,EDX
		mov	EDX,040h[ESI]
		adc	EAX,EDX
		mov	EDX,044h[ESI]
		adc	EAX,EDX
		mov	EDX,048h[ESI]
		adc	EAX,EDX
		mov	EDX,04Ch[ESI]
		adc	EAX,EDX
		mov	EDX,050h[ESI]
		adc	EAX,EDX
		mov	EDX,054h[ESI]
		adc	EAX,EDX
		mov	EDX,058h[ESI]
		adc	EAX,EDX
		mov	EDX,05Ch[ESI]
		adc	EAX,EDX
		mov	EDX,060h[ESI]
		adc	EAX,EDX
		mov	EDX,064h[ESI]
		adc	EAX,EDX
		mov	EDX,068h[ESI]
		adc	EAX,EDX
		mov	EDX,06Ch[ESI]
		adc	EAX,EDX
		mov	EDX,070h[ESI]
		adc	EAX,EDX
		mov	EDX,074h[ESI]
		adc	EAX,EDX
		mov	EDX,078h[ESI]
		adc	EAX,EDX
		mov	EDX,07Ch[ESI]
;      ENDM
;
;LOOP_ENTRY 0
;LOOP_ENTRY 4
;LOOP_ENTRY 8
;LOOP_ENTRY 12
;LOOP_ENTRY 16
;LOOP_ENTRY 20
;LOOP_ENTRY 24
;LOOP_ENTRY 28
;LOOP_ENTRY 32
;LOOP_ENTRY 36
;LOOP_ENTRY 40
;LOOP_ENTRY 44
;LOOP_ENTRY 48
;LOOP_ENTRY 52
;LOOP_ENTRY 56
;LOOP_ENTRY 60
;LOOP_ENTRY 64
;LOOP_ENTRY 68
;LOOP_ENTRY 72
;LOOP_ENTRY 76
;LOOP_ENTRY 80
;LOOP_ENTRY 84
;LOOP_ENTRY 88
;LOOP_ENTRY 92
;LOOP_ENTRY 96
;LOOP_ENTRY 100
;LOOP_ENTRY 104
;LOOP_ENTRY 108
;LOOP_ENTRY 112
;LOOP_ENTRY 116
;LOOP_ENTRY 120
;LOOP_ENTRY 124   ; 4*(LOOP_UNROLLING-1)
;
;
;checksum_dword_loop_end:
;       lea  esi, [esi+LOOP_UNROLLING*4]  ; update source address
		lea	ESI,080h[ESI]
;       dec  ecx                     ; count off unrolled loop iteration
		dec	ECX
;       jnz  checksum_dword_loop     ; do more blocks
		jne	L6E
;
;checksum_dword_loop_done:
;       adc  eax, edx                ; finish dword checksum
L11A:		adc	EAX,EDX
;       mov  edx, 0                  ; prepare to load trailing word
		mov	EDX,0
;       adc  eax, edx
		adc	EAX,EDX
;
;;
;; Compute checksum on the trailing word, if there is one.
;; High word of EDX = 0 at this point
;; Carry flag set iff there's a trailing word to do at this point
;;
;
;checksum_last_word:
;       pop  ecx                     ; get back word count
L123:		pop	ECX
;       test ecx, 1                  ; is there a trailing word?
		test	ECX,1
;       jz   short checksum_done     ; no trailing word
		je	L132
;       add  ax, [esi]               ; add in the trailing word
		add	AX,[ESI]
;       adc  eax, 0
		adc	EAX,0
;
;checksum_done:
;       mov  ecx, eax                ; fold the checksum to 16 bits
L132:		mov	ECX,EAX
;       ror  ecx, 16
		ror	ECX,010h
;       add  eax, ecx
		add	EAX,ECX
;       mov  ebx, [esp+buf]
		mov	EBX,0Ch[ESP]
;       shr  eax, 16
		shr	EAX,010h
;       test ebx, 1                  ; check if buffer word aligned
		test	EBX,1
;       jz   short checksum_combine  ; if zf set, buffer word aligned
		je	L14C
;       ror  ax, 8                   ; byte aligned--swap bytes back
		ror	AX,8
;
;checksum_combine:
;       pop  esi
L14C:		pop	ESI
;       adc  eax, 0
		adc	EAX,0
;       pop  ebx
		pop	EBX
;       ret
		ret
_TEXT	ends
_DATA	segment
	db	000h,000h,000h,000h
	dd	offset _TEXT:__w32_in_checksum_fast[0F9h]
	dd	offset _TEXT:__w32_in_checksum_fast[0F4h]
	dd	offset _TEXT:__w32_in_checksum_fast[0EFh]
	dd	offset _TEXT:__w32_in_checksum_fast[0EAh]
	dd	offset _TEXT:__w32_in_checksum_fast[0E5h]
	dd	offset _TEXT:__w32_in_checksum_fast[0E0h]
	dd	offset _TEXT:__w32_in_checksum_fast[0DBh]
	dd	offset _TEXT:__w32_in_checksum_fast[0D6h]
	dd	offset _TEXT:__w32_in_checksum_fast[0D1h]
	dd	offset _TEXT:__w32_in_checksum_fast[0CCh]
	dd	offset _TEXT:__w32_in_checksum_fast[0C7h]
	dd	offset _TEXT:__w32_in_checksum_fast[0C2h]
	dd	offset _TEXT:__w32_in_checksum_fast[0BDh]
	dd	offset _TEXT:__w32_in_checksum_fast[0B8h]
	dd	offset _TEXT:__w32_in_checksum_fast[0B3h]
	dd	offset _TEXT:__w32_in_checksum_fast[0AEh]
	dd	offset _TEXT:__w32_in_checksum_fast[0A9h]
	dd	offset _TEXT:__w32_in_checksum_fast[0A4h]
	dd	offset _TEXT:__w32_in_checksum_fast[09Fh]
	dd	offset _TEXT:__w32_in_checksum_fast[09Ah]
	dd	offset _TEXT:__w32_in_checksum_fast[095h]
	dd	offset _TEXT:__w32_in_checksum_fast[090h]
	dd	offset _TEXT:__w32_in_checksum_fast[08Bh]
	dd	offset _TEXT:__w32_in_checksum_fast[086h]
	dd	offset _TEXT:__w32_in_checksum_fast[081h]
	dd	offset _TEXT:__w32_in_checksum_fast[07Ch]
	dd	offset _TEXT:__w32_in_checksum_fast[077h]
	dd	offset _TEXT:__w32_in_checksum_fast[072h]
	dd	offset _TEXT:__w32_in_checksum_fast[06Dh]
	dd	offset _TEXT:__w32_in_checksum_fast[068h]
	dd	offset _TEXT:__w32_in_checksum_fast[063h]
_DATA	ends
;
;.DATA
;ALIGN 4
;
;loop_entries label dword
;       dd  0
;       dd  loop_entry_124
;       dd  loop_entry_120
;       dd  loop_entry_116
;       dd  loop_entry_112
;       dd  loop_entry_108
;       dd  loop_entry_104
;       dd  loop_entry_100
;       dd  loop_entry_96
;       dd  loop_entry_92
;       dd  loop_entry_88
;       dd  loop_entry_84
;       dd  loop_entry_80
;       dd  loop_entry_76
;       dd  loop_entry_72
;       dd  loop_entry_68
;       dd  loop_entry_64
;       dd  loop_entry_60
;       dd  loop_entry_56
;       dd  loop_entry_52
;       dd  loop_entry_48
;       dd  loop_entry_44
;       dd  loop_entry_40
;       dd  loop_entry_36
;       dd  loop_entry_32
;       dd  loop_entry_28
;       dd  loop_entry_24
;       dd  loop_entry_20
;       dd  loop_entry_16
;       dd  loop_entry_12
;       dd  loop_entry_8
;       dd  loop_entry_4
;
;comment ~
;;
;; For Watcom register calls
;;
;_w32_in_checksum_fast_:
;       push  edx
;       push  ecx
;       push  eax  ; buf
;       push  edx  ; len
;       call  _w32_in_checksum_fast
;       pop   ecx
;       pop   edx
;       ret
;   ~
;
;endif ; DOSX
;
;end
	end
