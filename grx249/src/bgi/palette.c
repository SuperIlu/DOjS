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

#define DEFAULT_PALETTE                                                  \
  { /*size:*/16,                                                         \
    /*colors:*/ {EGA_BLACK, EGA_BLUE, EGA_GREEN, EGA_CYAN, EGA_RED,      \
                 EGA_MAGENTA, EGA_BROWN, EGA_LIGHTGRAY, EGA_DARKGRAY,    \
                 EGA_LIGHTBLUE, EGA_LIGHTGREEN, EGA_LIGHTCYAN,           \
                 EGA_LIGHTRED, EGA_LIGHTMAGENTA, EGA_YELLOW, EGA_WHITE} }

struct palettetype __gr_EGAdef = DEFAULT_PALETTE;
static struct palettetype UsrPal = DEFAULT_PALETTE;

/* ----------------------------------------------------------------- */

void __gr_setpalette(int colornum, int color)
{
#ifdef __DJGPP__
# include <dpmi.h>
//# include <go32.h>
  _go32_dpmi_registers regs;

  _DO_INIT_CHECK;
  colornum &= 0x0f;
  color    &= 0x3f;
  UsrPal.colors[colornum] = color;

  memset(&regs, 0, sizeof(regs));
  regs.x.ax = 0x1000;
  regs.x.bx = colornum | (color << 8);
  /* real mode interruts may be called by
       _go32_dpmi_simulate_int() (v1 & v2)
     and
       __dpmi_simulate_real_mode_interrupt() (v2 only)

     Under v2 the _go32_dpmi_simulate_int is actually a macro
     referencing __dpmi_simulate_real_mode_interrupt(). Undefining
     this macro makes the library linkable under both DJGPP v1 and
     v2 since there _is_ a compatible _go32_dpmi_simulate_int() in
     the v2 library! Don't worry about the compiler warning here ! */
  #undef _go32_dpmi_simulate_int
  _go32_dpmi_simulate_int(0x10,&regs);
#endif
}
/* ----------------------------------------------------------------- */
void getpalette(struct palettetype  *palette)
{
  _DO_INIT_CHECK;
  *palette = UsrPal;
}

/* ----------------------------------------------------------------- */
void setallpalette(const struct palettetype *palette)
{
  int i, col;

  _DO_INIT_CHECK;
  if (palette == NULL)
    return;
  for (i=0; i < palette->size; ++i)
    if ( (col = palette->colors[i]) >= 0)
      __gr_setpalette( i, col);
}
