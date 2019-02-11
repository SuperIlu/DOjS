/**
 ** memmode.h ---- determine how to access video and system RAM
 **                Watcom-C++ code
 **
 ** Copyright (c) 1995 Csaba Biegl, 820 Stirrup Dr, Nashville, TN 37221
 ** [e-mail: csaba@vuse.vanderbilt.edu]
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

#include <i86.h>
#include "int86.h"
#undef MK_FP

#ifdef __386__
/* WATCOM DOS4GW protected mode */
#define  MK_FP(s,o)      (void *)(                              \
    ( ( ( (long)(unsigned short)(s) ) << 4 ) +                  \
    (unsigned short)(o))                                        \
)
#define  MK_DOS_ADDR(s,o)  (void *)((s):>(o))
#else
#define  MK_FP(s,o)      (void far *)(                          \
    ( ( ( (long)(unsigned short)(s) ) << 4 ) +                  \
    (unsigned short)(o))                                        \
)
#define  MK_DOS_ADDR(s,o)  (void far *)((s):>(o))

#endif /* __386__ */

#define  LINP_PTR(p)    (p)
#define  LINP_SEL(p)    0
#define  FP86_SEG(p)    ((unsigned short)((unsigned long)(p)>>16))
#define  FP86_OFF(p)    ((unsigned short)((int)(p)))

/*
The DECLARE_XFER_BUFFER macro allocates a DOS memory block.
Used by the int10x macro.
*/
#ifdef __386__
/* WATCOM DOS4GW protected mode */
#pragma off (unreferenced);
static short int10_segment = 0; /*this is not nice! - hack to load real segment for int10*/
/* allocate a real mode segment so we can return info from VESA int10 */
#define  DECLARE_XFER_BUFFER(size)                                           \
	void _far * xfer_ptr;                                                \
	Int86Regs BuffRegs;                                                  \
	do {                                                                 \
	  sttzero((&BuffRegs));                                              \
	  IREG_AX(BuffRegs) = 0x100;                                         \
	  IREG_BX(BuffRegs) = 64;                                            \
	  int386 ( 0x31, &(BuffRegs.Normal), &(BuffRegs.Normal) );           \
	  int10_segment = IREG_AX(BuffRegs);                                 \
	  xfer_ptr = IREG_DX(BuffRegs) :> (void near *)0;                    \
	  _fmemset ( xfer_ptr, 0xaa, 256 );                                  \
	} while (0)
/* access the real mode segment */
#define  XFER_BUFFER xfer_ptr
/* clean up the DOS memory allocated by DOS4GW DPMI macro above */
#define DELETE_XFER_BUFFER                                                   \
	do {                                                                 \
	  sttzero((&BuffRegs));                                              \
	  IREG_AX(BuffRegs) = 0x101;                                         \
	  IREG_DX(BuffRegs) = FP_SEG(xfer_ptr);                              \
	  int386x(0x31,&(BuffRegs.Normal),&(BuffRegs.Normal),&(BuffRegs.Extended) ); \
	} while (0)
#else /* __386__ */
/* WATCOM 16 Bit DOS - hey we're in real mode! */
#define  DECLARE_XFER_BUFFER(size)       char XFER_BUFFER[size]
#define  DELETE_XFER_BUFFER
#endif /* !__386__ */

