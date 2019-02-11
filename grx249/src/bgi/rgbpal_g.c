/**
 ** BCC2GRX  -  Interfacing Borland based graphics programs to LIBGRX
 ** Copyright (C) 1993-97 by Hartmut Schirmer
 **
 **
 ** Contact :                Hartmut Schirmer
 **                          Feldstrasse 118
 **                  D-24105 Kiel
 **                          Germany
 **
 ** e-mail : hsc@techfak.uni-kiel.de
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

#include "bccgrx00.h"

#if 0
/* The following io routines were taken from Csaba Biegl's
   GRX v2.0  source. Many thanks Csaba ! */
#define __INLINE_LOW_PORT_TEST__(P)  (                  \
    (__builtin_constant_p((P))) &&                      \
    ((unsigned)(P) < 0x100U)                            \
)
#define inportb(P) ({                                   \
    register unsigned char _value;                      \
    if(__INLINE_LOW_PORT_TEST__(P)) __asm__ volatile(   \
	"inb %1,%0"                                     \
	: "=a"  (_value)                                \
	: "n"   ((unsigned short)(P))                   \
    );                                                  \
    else __asm__ volatile(                              \
	"inb %1,%0"                                     \
	: "=a"  (_value)                                \
	: "d"   ((unsigned short)(P))                   \
    );                                                  \
    _value;                                             \
})

#define outportb(p,v) ({                                \
    __asm__ volatile(                                   \
	"outb %0,%1"                                    \
	: /* no outputs */                              \
	: "a" ((unsigned char)(v)),                     \
	  "d" ((unsigned short)(p))                     \
    );                                                  \
})

static volatile int dummy;

#define WAIT() do {               \
  dummy += inportb(0x80);         \
} while(0)

void __getrgbpalette(int color, int *red, int *green, int *blue) {
#ifdef GO32
  _DO_INIT_CHECK;
  WAIT();
  outportb(0x3c8, color&0xff);
  WAIT();
  *red = inportb(0x3c9)<<2;
  WAIT();
  *green = inportb(0x3c9)<<2;
  WAIT();
  *blue = inportb(0x3c9)<<2;
#else
  *red = *green = *blue = 0;
#endif
}

#else
void __getrgbpalette(int color, int *red, int *green, int *blue) {
  GrQueryColor(color,red,green,blue);
}
#endif
