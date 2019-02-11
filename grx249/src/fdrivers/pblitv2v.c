/**
 ** pblitv2v.c ---- video to video bitblt routine for packed (8, 16, 24, 32 bpp) modes
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
#include "allocate.h"
#include "arith.h"
#include "mempeek.h"

/* frame offset address calculation */
#define FOFS(x,y,lo) umuladd32((y),(lo),(x))

#ifndef __XWIN__
static INLINE
void dualpageblt(GrFrame *dst,int dx,int dy,
		 GrFrame *src,int sx,int sy,
		 int w,int h)
{
	unsigned long doff,soff;
	int   dskip,sskip;
	int   rb,rbb;
	int   wb,wbb;

	GRX_ENTER();
	if(dy > sy) {
	    dy   += (h - 1);
	    sy   += (h - 1);
	    dskip = -(dst->gf_lineoffset + w);
	    sskip = -(src->gf_lineoffset + w);
	}
	else {
	    dskip = dst->gf_lineoffset - w;
	    sskip = src->gf_lineoffset - w;
	}
	doff = FOFS(dx,dy,dst->gf_lineoffset);
	soff = FOFS(sx,sy,src->gf_lineoffset);
	wbb  = (-1);
	rbb  = (-1);
	setup_far_selector(dst->gf_selector);
	do {
	    unsigned w1 = BANKLFT(doff);
	    unsigned w2 = BANKLFT(soff);
	    unsigned w3,ww;
	    w1 = umin(w,w1);
	    w2 = umin(w,w2);
	    usort(w1,w2);
	    w3 = w  - w2;
	    w2 = w2 - w1;
	    if(w2 == 0) w2=w3 , w3=0;
	    do {
		char far *dptr = &dst->gf_baseaddr[0][BANKPOS(doff)];
		char far *sptr = &src->gf_baseaddr[0][BANKPOS(soff)];
		wb = BANKNUM(doff);
		rb = BANKNUM(soff);
		if((rbb - rb) | (wbb - wb)) SRWBANK((rbb = rb),(wbb = wb));
		doff += w1;
		soff += w1;
#ifndef MISALIGNED_16bit_OK
		do {
		    poke_b_f(dptr,peek_b_f(sptr));
		    dptr++; sptr++;
		} while (w1--);
#else
		if(w1 >= 3) {
		    if((int)(dptr) & 1) {
			poke_b_f(dptr,peek_b_f(sptr));
			dptr++; sptr++; w1--;
		    }
		    if((int)(dptr) & 2) {
			poke_w_f(dptr,peek_w_f(sptr));
			dptr += 2; sptr += 2; w1 -= 2;
		    }
		    if((ww = w1 >> 2) > 0) do {
			poke_l_f(dptr,peek_l_f(sptr));
			dptr += 4; sptr += 4;
		    } while(--ww != 0);
		}
		if(w1 & 2) {
		    poke_w_f(dptr,peek_w_f(sptr));
		    dptr += 2; sptr += 2;
		}
		if(w1 & 1) {
		    poke_b_f(dptr,peek_b_f(sptr));
		}
#endif
		w1 = w2;
		w2 = w3;
		w3 = 0;
	    } while(w1 != 0);
	    doff += dskip;
	    soff += sskip;
	} while(--h != 0);
	GRX_LEAVE();
}
#endif /* !__XWIN__ */

#define TMPSIZE 16384

void _GrFrDrvPackedBitBltV2V(GrFrame *dst,int dx,int dy,
			     GrFrame *src,int sx,int sy,
			     int w,int h,GrColor op)
{

	GRX_ENTER();
#ifndef __XWIN__
	if((C_OPER(op) == C_WRITE) && DRVINFO->splitbanks &&
	   ((dy != sy) || (sx >= dx))) {
	    dualpageblt(dst,dx,dy,src,sx,sy,w,h);
	} else
#endif
	{
	  GrFrame tmp;
	  int tmpx,tmpn;
	  tmp.gf_lineoffset = (w + 7) & ~3;
	  tmpn = umax(umin(h,(TMPSIZE / tmp.gf_lineoffset)),1);
	  tmpx = tmp.gf_lineoffset * tmpn;
#ifdef SMALL_STACK
	  tmp.gf_baseaddr[0] = _GrTempBufferAlloc(tmpx);
#else
	  setup_alloca();
	  tmp.gf_baseaddr[0] = alloca((size_t)tmpx);
#endif
	  if(tmp.gf_baseaddr[0]) {
	    int ydir = 0;
	    tmpx = sx & 3;
	    if(dy > sy) {
		dy  += h;
		sy  += h;
		ydir = ~0;
	    }
	    do {
		int cnt = umin(h,tmpn);
		dy -= (ydir & cnt);
		sy -= (ydir & cnt);
		_GrFrDrvPackedBitBltV2R(&tmp,tmpx,0,src,sx,sy,w,cnt,GrWRITE);
		_GrFrDrvPackedBitBltR2V(dst,dx,dy,&tmp,tmpx,0,w,cnt,op);
		dy += (~ydir & cnt);
		sy += (~ydir & cnt);
		h  -= cnt;
	    } while(h != 0);
	  }
#ifndef SMALL_STACK
	  reset_alloca();
#endif
	}
	GRX_LEAVE();
}

