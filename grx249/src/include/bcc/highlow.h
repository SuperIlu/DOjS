/**
 ** highlow.h ---- combining two BYTES into one WORD -- Borland-C++ special
 **
 ** Copyright (c) 1997 Hartmut Schirmer
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

/* combine two bytes into one word: optimized x86 version */
#define highlow(hi,lo) ( _AH = (hi), _AL = (lo), _AX )

