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

void setviewport(int left, int top, int right, int bottom, int clip)
{
  int mm;

  _DO_INIT_CHECK;
  GrSetContext( NULL);         /* ViewPort == Full screen */
  if (left > right) SWAP(int,left,right);
  if (bottom < top) SWAP(int,bottom,top);
  if (left < 0) left = 0;
  if (right > (mm=getmaxx())) right = mm;
  if (top < 0) top = 0;
  if (bottom > (mm=getmaxy())) bottom = mm;
  __gr_clip = clip;
  VL = left;
  VR = right;
  VT = top;
  VB = bottom;
  __gr_Reset_ClipBox();
  moveto( 0,0);
}
