/**
 ** usrscale.h
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

#ifndef _SCALE_H_
#define _SCALE_H_

#include "arith.h"

#define SCALE(var,arg,nom,den) do {    \
    (var) = iscale((arg),(nom),(den)); \
} while(0)

# define USCALE(var,arg,nom,den) do {  \
    (var) = uscale((arg),(nom),(den)); \
} while(0)

#endif /* whole file */
