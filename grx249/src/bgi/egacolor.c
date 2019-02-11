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

#ifdef __GNUC__
#define _tc(r,g,b) ((r)<<2),((g)<<2),((b)<<2)
#define UNSIGNED unsigned char
#else
#define _hi15(r,g,b) ( (((r)>>1)<<10)|(((g)>>1)<<5)|((b)>>1) )
#define _hi16(r,g,b) ( (((r)>>1)<<11)|((g)<<5)|((b)>>1) )
#define _tc(r,g,b)   ( ((r)<<2) | ((g)<<2) | ((b)<<2) )
#define UNSIGNED unsigned
static unsigned short _hi15_ega[16] = {
	_hi15( 0, 0, 0), _hi15( 0, 0,42), _hi15( 0,42, 0), _hi15( 0,42,42),
	_hi15(42, 0, 0), _hi15(42, 0,42), _hi15(42,21, 0), _hi15(42,42,42),
	_hi15(21,21,21), _hi15(21,21,63), _hi15(21,63,21), _hi15(21,63,63),
	_hi15(63,21,21), _hi15(63,21,63), _hi15(63,63,21), _hi15(63,63,63)
};
static unsigned short _hi16_ega[16] = {
	_hi16( 0, 0, 0), _hi16( 0, 0,42), _hi16( 0,42, 0), _hi16( 0,42,42),
	_hi16(42, 0, 0), _hi16(42, 0,42), _hi16(42,21, 0), _hi16(42,42,42),
	_hi16(21,21,21), _hi16(21,21,63), _hi16(21,63,21), _hi16(21,63,63),
	_hi16(63,21,21), _hi16(63,21,63), _hi16(63,63,21), _hi16(63,63,63)
};
#endif

static UNSIGNED _tc_ega[] = {
	_tc( 0, 0, 0), _tc( 0, 0,42), _tc( 0,42, 0), _tc( 0,42,42),
	_tc(42, 0, 0), _tc(42, 0,42), _tc(42,21, 0), _tc(42,42,42),
	_tc(21,21,21), _tc(21,21,63), _tc(21,63,21), _tc(21,63,63),
	_tc(63,21,21), _tc(63,21,63), _tc(63,63,21), _tc(63,63,63)
};

#ifdef __GNUC__

int _ega_color(int color)
{
  _DO_INIT_CHECK_RV(0);
  switch(GrNumColors()) {
    case 1L<<15:
    case 1L<<16:
    case 1L<<24:
      if ((color&15) == 15)
	color = GrWhite();
      else {
	int oldc = COL;
	color = (color&15)*3;
	color = setrgbcolor(_tc_ega[color], _tc_ega[color+1], _tc_ega[color+2]);
	COL = oldc;
      }
      break;
  }
  return color;
}

#else

int _ega_color(int color)
{
  switch (getmaxcolor()+1) {
    case 1L<<15: return _hi15_ega[color&15];
#if 0
    case 1L<<16: return _hi16_ega[color&15];
    case 1L<<24: return _tc_ega[color&15];
#endif
  }
  return color;
}
#endif
