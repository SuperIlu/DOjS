/**
 ** patternf.c ---- data structure for standard pattern filler
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
#include "shapes.h"

GrFiller _GrPatternFiller = {
   _GrDrawPatternedPixel,
   _GrDrawPatternedLine,
   _GrFillPatternedScanLine
};
