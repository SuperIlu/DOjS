/**
 ** bccfil24.c ---- optimized BCC memory fill operations for 24bpp
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
#include "access24.h"

#pragma inline
#pragma warn -rvl

#define REPF24_OP(FN,INS)                                         \
  void far * FN (void far *P,unsigned long V, unsigned int B) {   \
      _CX = (unsigned)B;                                          \
      _AX = (unsigned)(V);                                        \
      _DL = RD24BYTE(V,2);                                        \
      /* least sig byte: al, mid byte : ah, most sig byte : dl */ \
      asm         push    ds                              ;       \
      asm         lds     di, P                           ;       \
      asm         or      cx,cx                           ;       \
      asm         jz      short _##FN##done               ;       \
      asm         test    di,1                            ;       \
      asm         jz      short _##FN##1                  ;       \
      asm         INS     [di],al                         ;       \
      asm         inc     di                              ;       \
      asm         dec     cx                              ;       \
      asm         xchg    ah,al     /* rotate dl/ah/al */ ;       \
      asm         xchg    dl,ah                           ;       \
	 _##FN##1:                                                \
      asm         mov     dh,al     /* ax/dx/bx: 2 pix */ ;       \
      asm         mov     bl,ah                           ;       \
      asm         mov     bh,dl                           ;       \
      asm         mov     si,6                            ;       \
      asm         sub     cx,si                           ;       \
      asm         jb      short _##FN##3                  ;       \
	 _##FN##2:                                                \
      asm         INS     [di],ax                         ;       \
      asm         INS     [di+2],dx                       ;       \
      asm         INS     [di+4],bx                       ;       \
      asm         add     di,si                           ;       \
      asm         sub     cx,si                           ;       \
      asm         jnb     short _##FN##2                  ;       \
	 _##FN##3:                                                \
      asm         add     cx,si                           ;       \
      asm         je      short _##FN##done               ;       \
      /* 1..5 bytes left */                                       \
      asm         INS     [di],al                         ;       \
      asm         inc     di                              ;       \
      asm         dec     cx                              ;       \
      asm         je      short _##FN##done               ;       \
      asm         INS     [di],ah                         ;       \
      asm         inc     di                              ;       \
      asm         dec     cx                              ;       \
      asm         je      short _##FN##done               ;       \
      asm         INS     [di],dl                         ;       \
      asm         inc     di                              ;       \
      asm         dec     cx                              ;       \
      asm         je      short _##FN##done               ;       \
      asm         INS     [di],dh                         ;       \
      asm         inc     di                              ;       \
      asm         dec     cx                              ;       \
      asm         je      short _##FN##done               ;       \
      asm         INS     [di],bl                         ;       \
      asm         inc     di                              ;       \
	  _##FN##done:                                            \
      asm         mov     dx, ds                          ;       \
      asm         mov     ax, di                          ;       \
      asm         pop     ds                              ;       \
  }

REPF24_OP(_GR_repfill_24_set,MOV_INS)
REPF24_OP(_GR_repfill_24_xor,XOR_INS)
REPF24_OP(_GR_repfill_24_or,OR_INS)
REPF24_OP(_GR_repfill_24_and,AND_INS)

