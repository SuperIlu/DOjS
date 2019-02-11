/**
 ** bccfil08.c ---- optimized BCC memory fill operations (8bit)
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
  void far * FN (void far *P, int O, unsigned char V, unsigned C) { \
    asm         les     di, P                      ;                \
    asm         mov     cx, C                      ;                \
    asm         or      cx,cx                      ;                \
    asm         jz      short _##FN##3             ;                \
    asm         mov     bx, O                      ;                \
    asm         mov     al, V                      ;                \
    asm         inc     cx                         ;                \
    asm         shr     cx,1                       ;                \
    asm         jnc     short _##FN##2             ;                \
       _##FN##1:                                                    \
    asm         INS     es:[di],al                 ;                \
    asm         add     di,bx                      ;                \
       _##FN##2:                                                    \
    asm         INS     es:[di],al                 ;                \
    asm         add     di,bx                      ;                \
    asm         dec     cx                         ;                \
    asm         jnz     short _##FN##1             ;                \
	_##FN##3:                                                   \
    asm         mov     dx,es                      ;                \
    asm         mov     ax,di                      ;                \
  }

COLFL(_GR_colfill_b_mov,MOV_INS)
COLFL(_GR_colfill_b_xor,XOR_INS)
COLFL(_GR_colfill_b_or,OR_INS)
COLFL(_GR_colfill_b_and,AND_INS)


void far *_GR_repfill_b(void far *P,unsigned int V, unsigned int C) {
  asm         les     di, P
  asm         mov     cx, C
  asm         or      cx,cx
  asm         jz      short _rpfb3
  asm         cld
  asm         mov     ax, V
  asm         test    di,1
  asm         jz      short _rpfb1
  asm         stosb
  asm         dec     cx
      _rpfb1:
  asm         shr     cx,1
  asm         jz      _rpfb2
  asm         pushf
  asm         cld
  asm         rep     stosw
  asm         popf
      _rpfb2:
  asm         jnc     _rpfb3
  asm         stosb
      _rpfb3:
  asm         mov     dx,es
  asm         mov     ax,di
}

#define REPFB_OP(FN,INS)                                          \
  void far * FN (void far *P,unsigned int V, unsigned int C) {    \
      asm         mov     cx, C                    ;              \
      asm         push    ds                       ;              \
      asm         lds     di, P                    ;              \
      asm         or      cx,cx                    ;              \
      asm         jz      short _##FN##4           ;              \
      asm         cld                              ;              \
      asm         mov     ax, V                    ;              \
      asm         test    di,1                     ;              \
      asm         jz      short _##FN##1           ;              \
      asm         INS     [di],al                  ;              \
      asm         inc     di                       ;              \
      asm         dec     cx                       ;              \
	 _##FN##1:                                                \
      asm         shr     cx,1                     ;              \
      asm         jz      short _##FN##3           ;              \
      asm         pushf                            ;              \
      asm         inc     cx                       ;              \
      asm         shr     cx,1                     ;              \
      asm         jnc     short _##FN##5           ;              \
	 _##FN##2:                                                \
      asm         INS     [di],ax                  ;              \
      asm         inc     di                       ;              \
      asm         inc     di                       ;              \
	 _##FN##5:                                                \
      asm         INS     [di],ax                  ;              \
      asm         inc     di                       ;              \
      asm         inc     di                       ;              \
      asm         dec     cx                       ;              \
      asm         jnz     short _##FN##2           ;              \
      asm         popf                             ;              \
	 _##FN##3:                                                \
      asm         jnc     _##FN##4                 ;              \
      asm         INS     [di],al                  ;              \
      asm         inc     di                       ;              \
	 _##FN##4:                                                \
      asm         mov     dx,ds                    ;              \
      asm         mov     ax,di                    ;              \
      asm         pop     ds                       ;              \
  }

REPFB_OP(_GR_repfill_b_xor,XOR_INS)
REPFB_OP(_GR_repfill_b_or,OR_INS)
REPFB_OP(_GR_repfill_b_and,AND_INS)

