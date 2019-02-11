/**
 ** genellip.c ---- generate points for an ellipse or ellipse arc
 **
 ** Copyright (c) 1995 Csaba Biegl, 820 Stirrup Dr, Nashville, TN 37221
 ** [e-mail: csaba@vuse.vanderbilt.edu]
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
#include "arith.h"

#define MAXPTS  (GR_MAX_ELLIPSE_POINTS & (~15))
#define SEGLEN  5		/* preferred lenght of line segments on arc */
#define TRIGMGN 16384		/* scale factor for sine table */
#define PERIOD  1024		/* number of points in sine table */
#define PHALF	(PERIOD / 2)
#define PQUART  (PERIOD / 4)

static int sintab[PQUART + 1] = {
	0,   101,   201,   302,	  402,	 503,	603,   704,
      804,   904,  1005,  1105,  1205,  1306,  1406,  1506,
     1606,  1706,  1806,  1906,  2006,  2105,  2205,  2305,
     2404,  2503,  2603,  2702,  2801,  2900,  2999,  3098,
     3196,  3295,  3393,  3492,  3590,  3688,  3786,  3883,
     3981,  4078,  4176,  4273,  4370,  4467,  4563,  4660,
     4756,  4852,  4948,  5044,  5139,  5235,  5330,  5425,
     5520,  5614,  5708,  5803,  5897,  5990,  6084,  6177,
     6270,  6363,  6455,  6547,  6639,  6731,  6823,  6914,
     7005,  7096,  7186,  7276,  7366,  7456,  7545,  7635,
     7723,  7812,  7900,  7988,  8076,  8163,  8250,  8337,
     8423,  8509,  8595,  8680,  8765,  8850,  8935,  9019,
     9102,  9186,  9269,  9352,  9434,  9516,  9598,  9679,
     9760,  9841,  9921, 10001, 10080, 10159, 10238, 10316,
    10394, 10471, 10549, 10625, 10702, 10778, 10853, 10928,
    11003, 11077, 11151, 11224, 11297, 11370, 11442, 11514,
    11585, 11656, 11727, 11797, 11866, 11935, 12004, 12072,
    12140, 12207, 12274, 12340, 12406, 12472, 12537, 12601,
    12665, 12729, 12792, 12854, 12916, 12978, 13039, 13100,
    13160, 13219, 13279, 13337, 13395, 13453, 13510, 13567,
    13623, 13678, 13733, 13788, 13842, 13896, 13949, 14001,
    14053, 14104, 14155, 14206, 14256, 14305, 14354, 14402,
    14449, 14497, 14543, 14589, 14635, 14680, 14724, 14768,
    14811, 14854, 14896, 14937, 14978, 15019, 15059, 15098,
    15137, 15175, 15213, 15250, 15286, 15322, 15357, 15392,
    15426, 15460, 15493, 15525, 15557, 15588, 15619, 15649,
    15679, 15707, 15736, 15763, 15791, 15817, 15843, 15868,
    15893, 15917, 15941, 15964, 15986, 16008, 16029, 16049,
    16069, 16088, 16107, 16125, 16143, 16160, 16176, 16192,
    16207, 16221, 16235, 16248, 16261, 16273, 16284, 16295,
    16305, 16315, 16324, 16332, 16340, 16347, 16353, 16359,
    16364, 16369, 16373, 16376, 16379, 16381, 16383, 16384,
    16384
};

static int last_xs = 0,last_ys = 0;
static int last_xe = 0,last_ye = 0;
static int last_xc = 0,last_yc = 0;

static void GrSinCos(int n,int cx,int cy,int rx,int ry,int *pt)
{
	int cval,sval;
	switch((n &= (PERIOD - 1)) / PQUART) {
	  case 0:
	    sval =  sintab[n];
	    cval =  sintab[PQUART - n];
	    break;
	  case 1:
	    sval =  sintab[PHALF - n];
	    cval = -sintab[n - PQUART];
	    break;
	  case 2:
	    sval = -sintab[n - PHALF];
	    cval = -sintab[PERIOD - PQUART - n];
	    break;
	  default: /* must be 3 */
	    sval = -sintab[PERIOD - n];
	    cval =  sintab[n - PERIOD + PQUART];
	    break;
	}
	pt[0] = cx; pt[0] += irscale(rx,cval,TRIGMGN);
	pt[1] = cy; pt[1] -= irscale(ry,sval,TRIGMGN);
}

int GrGenerateEllipseArc(int cx,int cy,int rx,int ry,int start,int end,int pt[][2])
{
	int npts = urscale((iabs(rx) + iabs(ry)),314,(SEGLEN * 100));
	int step,closed;
	start = irscale(start,PERIOD,GR_MAX_ANGLE_VALUE) & (PERIOD - 1);
	end   = irscale(end,  PERIOD,GR_MAX_ANGLE_VALUE) & (PERIOD - 1);
	if(start == end) {
	    closed = TRUE;
	    end += PERIOD;
	}
	else {
	    if(start > end) end += PERIOD;
	    closed = FALSE;
	}
	npts = urscale(npts,(end - start),PERIOD);
	npts = umax(npts,16);
	npts = umin(npts,MAXPTS);
	if(closed) {
	    for(step = 1; (PERIOD / step) > npts; step <<= 1);
	    end -= step;
	    npts = 0;
	}
	else {
	    int start2 = end - start - 1;
	    step = umax(1,((end - start) / npts));
	    while(((start2 + step) / step) >= MAXPTS) step++;
	    start2 = end - (((end - start) / step) * step);
	    npts = 0;
	    if(start2 > start) {
		GrSinCos(start,cx,cy,rx,ry,pt[0]);
		start = start2;
		npts++;
	    }
	}
	while(start <= end) {
	    GrSinCos(start,cx,cy,rx,ry,pt[npts]);
	    start += step;
	    npts++;
	}
	last_xc = cx;
	last_yc = cy;
	last_xs = pt[0][0];
	last_ys = pt[0][1];
	last_xe = pt[npts - 1][0];
	last_ye = pt[npts - 1][1];
	return(npts);
}

int GrGenerateEllipse(int xc,int yc,int rx,int ry,int pt[][2])
{
	return(GrGenerateEllipseArc(xc,yc,rx,ry,0,0,pt));
}

void GrLastArcCoords(int *xs,int *ys,int *xe,int *ye,int *xc,int *yc)
{
	*xs = last_xs; *ys = last_ys;
	*xe = last_xe; *ye = last_ye;
	*xc = last_xc; *yc = last_yc;
}
