/**
 ** bccfil32.c ---- optimized BCC memory fill operations (32bit)
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
#include "memfill.h"

#pragma inline
#pragma warn -rvl

#define COLFL(FN,INS) \
  void far * FN (void far *P,int O, unsigned long V, unsigned C) { \
    asm         les     di, P                      ;               \
    asm         mov     cx, C                      ;               \
    asm         or      cx,cx                      ;               \
    asm         jz      short _##FN##3             ;               \
    asm         mov     bx, O                      ;               \
    _DX = ((GR_int16u *)&V)[1];                                    \
    _AX = ((GR_int16u *)&V)[0];                                    \
    asm         inc     cx                         ;               \
    asm         shr     cx,1                       ;               \
    asm         jnc     short _##FN##2             ;               \
       _##FN##1:                                                   \
    asm         INS     es:[di],ax                 ;               \
    asm         INS     es:[di+2],dx               ;               \
    asm         add     di,bx                      ;               \
       _##FN##2:                                                   \
    asm         INS     es:[di],ax                 ;               \
    asm         INS     es:[di+2],dx               ;               \
    asm         add     di,bx                      ;               \
    asm         dec     cx                         ;               \
    asm         jnz     short _##FN##1             ;               \
	_##FN##3:                                                  \
    asm         mov     dx,es                      ;               \
    asm         mov     ax,di                      ;               \
  }

COLFL(_GR_colfill_l_mov,MOV_INS)
COLFL(_GR_colfill_l_xor,XOR_INS)
COLFL(_GR_colfill_l_or,OR_INS)
COLFL(_GR_colfill_l_and,AND_INS)
