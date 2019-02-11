/**
 ** u_vsvirt.c ---- virtual screen utility functions using VESA BIOS calls
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
#include "grdriver.h"
#include "vesa.h"
#include "arith.h"
#include "mempeek.h"
#include "memfill.h"
#include "int86.h"

int _GrViDrvVESAsetVirtualSize(GrVideoMode *md,int w,int h,GrVideoMode *result)
{
	Int86Regs r;
	sttzero(&r);
	IREG_AX(r) = VESA_FUNC + VESA_SCAN_LNLEN;
	IREG_BX(r) = 0;
	IREG_CX(r) = w;
#ifdef __WATCOMC__
      int10x(&r);
#else
	int10(&r);
#endif
	if(IREG_AX(r) == VESA_SUCCESS) {
	    result->lineoffset = IREG_BX(r);
	    result->width      = IREG_CX(r);
	    result->height     = umin(IREG_DX(r),h);
	    return(TRUE);
	}
	return(FALSE);
}

/*
** VESA 2.0 has a PM function for VESA_DISP_START
** Bad news: register values are incompatible with
** real mode function. PM adaption should be done
*/
int _GrViDrvVESAvirtualScroll(GrVideoMode *md,int x,int y,int result[2])
{
	Int86Regs r;
	sttzero(&r);
	IREG_AX(r) = VESA_FUNC + VESA_DISP_START;
	IREG_BX(r) = 0;
	IREG_CX(r) = x;
	IREG_DX(r) = y;
#ifdef __WATCOMC__
      int10x(&r);
#else
	int10(&r);
#endif
	if(IREG_AX(r) == VESA_SUCCESS) {
	    IREG_AX(r) = VESA_FUNC + VESA_DISP_START;
	    IREG_BX(r) = 1;
#ifdef __WATCOMC__
	int10x(&r);
#else
	    int10(&r);
#endif
	    result[0] = IREG_CX(r);
	    result[1] = IREG_DX(r);
	    return(TRUE);
	}
	return(FALSE);
}

