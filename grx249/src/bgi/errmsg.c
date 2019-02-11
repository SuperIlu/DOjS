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

/* ----------------------------------------------------------------- */
char *grapherrormsg(int errorcode)
{
  switch (errorcode) {
    case grOk             : return "No error";
    case grNoInitGraph    : return "graphics not initialized";
    case grNotDetected    : return "Graphics hardware not detected";
    case grFileNotFound   : return "Device driver file not found";
    case grInvalidDriver  : return "Invalid device driver file";
    case grNoLoadMem      : return "Not enough memory to load driver";
    case grNoScanMem      : return "Out of memory in scan fill";
    case grNoFloodMem     : return "Out of memory in flood fill";
    case grFontNotFound   : return "Font file not found";
    case grNoFontMem      : return "Not enough memory to load font";
    case grInvalidMode    : return "Invalid graphics mode";
    case grError          : return "Graphics error";
    case grIOerror        : return "Graphics I/O error";
    case grInvalidFont    : return "Invalid font file";
    case grInvalidFontNum : return "Invalid font number";
    case grInvalidVersion : return "Invalid File Version Number";
  }
  return "Unknown graphics error";
}
