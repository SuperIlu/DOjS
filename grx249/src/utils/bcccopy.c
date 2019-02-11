/**
 ** bcccopy.c ---- optimized BCC memory copy operations
 **
 ** Copyright (c) 1998 Hartmut Schirmer
 **
 ** This file is part of the GRX graphics library.
 **
 ** The GRX graphics library is free software; you can redistribute it
 ** and/or modify it under some conditions; see the "copying.grx" file
 ** for details.
 **
 ** This library is distributed in the hope that it will be useful,
 ** but WITHOUT ANY WARRANTY; without even the implied warranty of
 ** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 **
 **/

#include "libgrx.h"
#include "mempeek.h"
#include "memcopy.h"

#pragma inline
#pragma warn -rvl

void far *_GR_fwdcopy_set(void near *ap, void far *d, void far *s, unsigned cnt)
{
  asm         push    ds
  asm         mov     cx, cnt
  asm         mov     ax, ap
  asm         les     di, d
  asm         lds     si, s
  asm         or      cx,cx
  asm         jz      short done
  asm         cld
  asm         test    al,1
  asm         jz      short noalign
  asm         movsb
  asm         dec     cx
  asm         jz      short done
      noalign:
  asm         shr     cx, 1
  asm         jz      short noword
  asm         rep movsw
      noword:
  asm         jnc     short done
  asm         movsb
      done:
  asm         mov     dx, es
  asm         mov     ax, di
  asm         pop     ds
}

#define COPYOPR(FN,INS)                                                       \
  void far * FN (void near *ap, void far *d, void far *s, unsigned cnt)       \
  {                                                                           \
      asm         push    ds                                                ; \
      asm         mov     cx, cnt                                           ; \
      asm         mov     ax, ap                                            ; \
      asm         les     di, d                                             ; \
      asm         lds     si, s                                             ; \
      asm         or      cx,cx                                             ; \
      asm         jz      short done                                        ; \
      asm         cld                                                       ; \
      asm         test    al,1                                              ; \
      asm         jz      short noalign                                     ; \
      asm         lodsb                                                     ; \
      asm         INS     es:[di], al                                       ; \
      asm         inc     di                                                ; \
      asm         dec     cx                                                ; \
      asm         jz      short done                                        ; \
	  noalign:                                                            \
      asm         shr     cx, 1                                             ; \
      asm         jz      short noword                                      ; \
      asm         pushf                                                     ; \
      asm         mov     bx,2                                              ; \
      asm         inc     cx                                                ; \
      asm         shr     cx, 1                                             ; \
      asm         jnc     short odd                                         ; \
	  loop:                                                               \
      asm         lodsw                                                     ; \
      asm         INS     es:[di], ax                                       ; \
      asm         add     di, bx                                            ; \
	  odd:                                                                \
      asm         lodsw                                                     ; \
      asm         INS     es:[di], ax                                       ; \
      asm         add     di, bx                                            ; \
      asm         dec     cx                                                ; \
      asm         jnz     short loop                                        ; \
      asm         popf                                                      ; \
	  noword:                                                             \
      asm         jnc     short done                                        ; \
      asm         lodsb                                                     ; \
      asm         INS     es:[di], al                                       ; \
      asm         inc     di                                                ; \
	  done:                                                               \
      asm         mov     dx, es                                            ; \
      asm         mov     ax, di                                            ; \
      asm         pop     ds                                                ; \
  }

COPYOPR(_GR_fwdcopy_xor,xor)
COPYOPR(_GR_fwdcopy_or,or)
COPYOPR(_GR_fwdcopy_and,and)
