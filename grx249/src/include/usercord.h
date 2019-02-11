/**
 ** usercord.h
 **
 ** Copyright (C) 1992, Csaba Biegl
 **   820 Stirrup Dr, Nashville, TN, 37221
 **   csaba@vuse.vanderbilt.edu
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

#include "usrscale.h"

#define U2SX(x,c) \
    SCALE(x,((x) - (c)->gc_usrxbase),(c)->gc_xmax,(c)->gc_usrwidth)
#define U2SY(y,c) \
    SCALE(y,((y) - (c)->gc_usrybase),(c)->gc_ymax,(c)->gc_usrheight)

#define S2UX(x,c) do {                                          \
    SCALE(x,x,(c)->gc_usrwidth,(c)->gc_xmax);                   \
    (x) += (c)->gc_usrxbase;                                    \
} while(0)

#define S2UY(y,c) do {                                          \
    SCALE(y,y,(c)->gc_usrheight,(c)->gc_ymax);                  \
    (y) += (c)->gc_usrybase;                                    \
} while(0)
