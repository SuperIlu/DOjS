/**
 ** u_egavga.c ---- common EGA/VGA utilities for video drivers
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
#include "int86.h"
#include "vesa.h"
#include "memfill.h"

int _GrViDrvDetectEGAVGA(void)
{
	Int86Regs r;
	sttzero(&r);
	/* check for EGA/VGA by trying to read a palette register */
	IREG_AX(r) = 0x1007;
	IREG_BX(r) = 0xff00;
	int10(&r);
	return ( (unsigned char)IREG_BH(r) != (unsigned char)0xff );
}

static int detectvga(void)
{
	Int86Regs r;
	sttzero(&r);
	/* check for VGA by trying to read a DAC register */
	IREG_AX(r) = 0x1015;
	IREG_BX(r) = 0;
	IREG_CX(r) = 0xffff;
	IREG_DX(r) = 0xffff;
	int10(&r);
	return( (   (unsigned short)(IREG_CX(r) & IREG_DX(r))
		 != (unsigned short)0xffff                  ) ? TRUE : FALSE);
}

int _GrViDrvDetectEGA(void)
{
	return((_GrViDrvDetectEGAVGA() && !detectvga()) ? TRUE : FALSE);
}

int _GrViDrvDetectVGA(void)
{
	return((_GrViDrvDetectEGAVGA() && detectvga()) ? TRUE : FALSE);
}

int _GrViDrvGetCurrentEGAVGAmode(void)
{
	Int86Regs r;
	sttzero(&r);
	IREG_AX(r) = VESA_FUNC + VESA_GET_MODE;
	int10(&r);
	if(IREG_AX(r) == VESA_SUCCESS) {
	    int mode = IREG_BX(r) & 0x7fff;
	    if(mode > 0x13) return(mode);
	}
	IREG_AX(r) = 0x0f00;
	int10(&r);
	return(IREG_AL(r) & 0x7f);
}

static int setmode(int mode,int noclear)
{
	Int86Regs r;
	sttzero(&r);
	IREG_AX(r) = VESA_FUNC + VESA_SET_MODE;
	IREG_BX(r) = (mode & 0x7fff) | (noclear ? 0x8000U : 0);
	int10(&r);
	if(IREG_AX(r) == VESA_SUCCESS) return(TRUE);
	IREG_AX(r) = (mode & 0x7f) | (noclear ? 0x80U : 0);
	int10(&r);
	return((_GrViDrvGetCurrentEGAVGAmode() == mode) ? TRUE : FALSE);
}

int _GrViDrvSetEGAVGAmode(GrVideoMode *mp,int noclear)
{
	return(setmode(mp->mode,noclear));
}

int _GrViDrvSetEGAVGAcustomTextMode(GrVideoMode *mp,int noclear)
{
	if(_GrViDrvSetEGAVGAmode(mp,noclear)) {
	    Int86Regs r;
	    sttzero(&r);
	    /* load 8x8 or 8x14 font */
	    IREG_AX(r) = (mp->height >= 50) ? 0x1112 : 0x1111;
	    int10(&r);
	    return(TRUE);
	}
	return(FALSE);
}

static int origmode = (-1);

int _GrViDrvInitEGAVGA(char *options)
{
	if(_GrViDrvDetectEGAVGA()) {
	    origmode = _GrViDrvGetCurrentEGAVGAmode();
	    return(TRUE);
	}
	return(FALSE);
}

void _GrViDrvResetEGAVGA(void)
{
	if((origmode != (-1)) && DRVINFO->moderestore) {
	    setmode(origmode,FALSE);
	}
}

GrVideoModeExt _GrViDrvEGAVGAtextModeExt = {
    GR_frameText,                       /* frame driver */
    NULL,                               /* frame driver override */
    MK_FP(0xb800,0),                    /* frame buffer address */
    { 0, 0, 0 },                        /* color precisions */
    { 0, 0, 0 },                        /* color component bit positions */
    0,                                  /* mode flag bits */
    _GrViDrvSetEGAVGAmode,              /* mode set */
    NULL,                               /* virtual size set */
    NULL,                               /* virtual scroll */
    NULL,                               /* bank set function */
    NULL,                               /* double bank set function */
    NULL                                /* color loader */
};

GrVideoModeExt _GrViDrvEGAVGAcustomTextModeExt = {
    GR_frameText,                       /* frame driver */
    NULL,                               /* frame driver override */
    MK_FP(0xb800,0),                    /* frame buffer address */
    { 0, 0, 0 },                        /* color precisions */
    { 0, 0, 0 },                        /* color component bit positions */
    0,                                  /* mode flag bits */
    _GrViDrvSetEGAVGAcustomTextMode,    /* mode set */
    NULL,                               /* virtual size set */
    NULL,                               /* virtual scroll */
    NULL,                               /* bank set function */
    NULL,                               /* double bank set function */
    NULL                                /* color loader */
};

