/**
 ** vesa_rm.c ---- the GRX 2.0 VESA BIOS interface: real mode banking code
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
 ** Contributions by: (See "doc/credits.doc" for details)
 ** Hartmut Schirmer (hsc@techfak.uni-kiel.de)
 **/

static void RM_setbank(int bk)
{
	Int86Regs r;

	sttzero(&r);
	IREG_AX(r) = VESA_FUNC + VESA_PAGE_CTL;
	IREG_BX(r) = _GrVidDrvVESAwrbank;
	IREG_DX(r) = bk << _GrVidDrvVESAbanksft;
	int10(&r);
	if(_GrVidDrvVESArdbank >= 0) {
	    IREG_AX(r) = VESA_FUNC + VESA_PAGE_CTL;
	    IREG_BX(r) = _GrVidDrvVESArdbank;
	    IREG_DX(r) = bk << _GrVidDrvVESAbanksft;
	    int10(&r);
	}
	setup_far_selector(SCRN->gc_selector);
}

static void RM_setrwbanks(int rb,int wb)
{
	Int86Regs r;

	sttzero(&r);
	IREG_AX(r) = VESA_FUNC + VESA_PAGE_CTL;
	IREG_BX(r) = _GrVidDrvVESAwrbank;
	IREG_DX(r) = wb << _GrVidDrvVESAbanksft;
	int10(&r);
	IREG_AX(r) = VESA_FUNC + VESA_PAGE_CTL;
	IREG_BX(r) = _GrVidDrvVESArdbank;
	IREG_DX(r) = rb << _GrVidDrvVESAbanksft;
	int10(&r);
	setup_far_selector(SCRN->gc_selector);
}

#ifdef __TURBOC__

static void far (*VESAbankfn)(void);

static void RM_protsetbank(int bk)
{
	_DX = bk << _GrVidDrvVESAbanksft;
	_BX = _GrVidDrvVESAwrbank;
	_AX = VESA_FUNC + VESA_PAGE_CTL;
	(*VESAbankfn)();
	if(_GrVidDrvVESArdbank >= 0) {
	    _DX = bk << _GrVidDrvVESAbanksft;
	    _BX = _GrVidDrvVESArdbank;
	    _AX = VESA_FUNC + VESA_PAGE_CTL;
	    (*VESAbankfn)();
	}
}

static void RM_protsetrwbanks(int rb,int wb)
{
	_DX = wb << _GrVidDrvVESAbanksft;
	_BX = _GrVidDrvVESAwrbank;
	_AX = VESA_FUNC + VESA_PAGE_CTL;
	(*VESAbankfn)();
	_DX = rb << _GrVidDrvVESAbanksft;
	_BX = _GrVidDrvVESArdbank;
	_AX = VESA_FUNC + VESA_PAGE_CTL;
	(*VESAbankfn)();
}

#endif
