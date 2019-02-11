/**
 ** pblitr2r.c ---- RAM to RAM bitblt routine for packed (8,16,24,32 bpp) modes
 **
 ** Copyright (c) 1995 Csaba Biegl, 820 Stirrup Dr, Nashville, TN 37221
 ** [e-mail: csaba@vuse.vanderbilt.edu].
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
 ** Contributions by: (See "doc/credits.doc" for details)
 ** Hartmut Schirmer (hsc@techfak.uni-kiel.de)
 **
 **/

#ifdef GRX_USE_RAM3x8

/* name of blit function to be created */
#define BLITFUNC _GrFrDrvPackedBitBltR2Rpl
/* create function with additional 'int plane' as last arg */
#define PLANE_ARG

#else /* GRX_USE_RAM3x8 */

/* name of blit function to be created */
#define BLITFUNC _GrFrDrvPackedBitBltR2R

#endif /* GRX_USE_RAM3x8 */

/* only RAM access, use _n to keep preprocessor happy */

#define WRITE_FAR _n
#define READ_FAR _n

/* source and destination may overlap */
#define BLIT_CAN_OVERLAP

#include "fdrivers/pblit_nb.h"


#ifdef GRX_USE_RAM3x8
void _GrFrDrvPackedBitBltR2R(GrFrame *dst,int dx,int dy,
			     GrFrame *src,int sx,int sy,
			     int w,int h,GrColor op)
{
   _GrFrDrvPackedBitBltR2Rpl(dst,dx,dy,src,sx,sy,w,h,op,0);
}
#endif
