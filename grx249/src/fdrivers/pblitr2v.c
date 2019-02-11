/**
 ** pblitr2v.c ---- RAM to video bitblt routine for packed (8, 16, 24, 32 bpp) modes
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
 **/

#include "libgrx.h"
#include "grdriver.h"
#include "arith.h"
#include "mempeek.h"
#include "memcopy.h"
#include "pblit.h"

/* frame offset address calculation */
#define FOFS(x,y,lo) umuladd32((y),(lo),(x))

void _GrFrDrvPackedBitBltR2V(GrFrame *dst,int dx,int dy,
			     GrFrame *src,int sx,int sy,
			     int w,int h,GrColor op)
{
	GR_int32u doff;
	char far *dptr, *sptr;
	unsigned int dskip, sskip;
	int oper;
	GR_int8u cval;

	GRX_ENTER();
	dskip = dst->gf_lineoffset;
	doff  = FOFS(dx,dy,dskip);
	sskip = src->gf_lineoffset;
	sptr  = &src->gf_baseaddr[0][FOFS(sx,sy,sskip)];
	dskip -= w;
	sskip -= w;
	oper  = C_OPER(op);
	cval  = (GR_int8u)op;

#       define DOICPY()   DOIMGCOPY(FW,_f,_n,w1)

	setup_far_selector(dst->gf_selector);
	do {
	    unsigned int w1 = BANKLFT(doff);
	    unsigned int w2 = w - (w1 = umin(w,w1));
	    do {
		dptr = &dst->gf_baseaddr[0][BANKPOS(doff)];
		CHKBANK(BANKNUM(doff));
		doff += w1;
		if (w1) switch(oper) {
		    case C_IMAGE: DOICPY();                           break;
		    case C_XOR:   fwdcopy_f_xor(dptr,dptr,sptr,w1);   break;
		    case C_OR:    fwdcopy_f_or (dptr,dptr,sptr,w1);   break;
		    case C_AND:   fwdcopy_f_and(dptr,dptr,sptr,w1);   break;
		    default:      fwdcopy_f_set(dptr,dptr,sptr,w1);   break;
		}
		w1 = w2;
		w2 = 0;
	    } while(w1 != 0);
	    doff += dskip;
	    sptr += sskip;
	} while(--h != 0);
	GRX_LEAVE();
}

