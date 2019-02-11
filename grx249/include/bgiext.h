/**
 ** BCC2GRX  -  Interfacing Borland based graphics programs to LIBGRX
 ** Copyright (C) 1993-97 by Hartmut Schirmer
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

#ifndef __BGIEXT_H
#define __BGIEXT_H

#include <libbcc.h>

#ifdef __cplusplus
extern "C" {
#endif

extern unsigned char _dac_g256[][3];    /* 256 shading dac values */
extern unsigned char _dac_normal[][3];  /* 256 standard colors    */

extern void setrgbdefaults(void);
extern void setrgbgray256(void);

#ifdef __cplusplus
}
#endif

#endif
