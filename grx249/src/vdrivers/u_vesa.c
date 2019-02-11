/**
 ** u_vesa.c ---- interface utility functions to VESA BIOS inquiry calls
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
 ** Hartmut Schirmer (hsc@xaphod.techfak.uni-kiel.d400.de)
 **/

#include "libgrx.h"
#include "allocate.h"
#include "vesa.h"
#include "mempeek.h"
#include "memfill.h"
#include "int86.h"

#ifndef IREG_AX

int _GrViDrvVESAgetVGAinfo(VESAvgaInfoBlock *ib)
{
	return(FALSE);
}
int _GrViDrvVESAgetModeInfo(int mode,VESAmodeInfoBlock *ib)
{
	return(FALSE);
}

VESApmInfoBlock * _GrViDrvVESAgetPMinfo(void)
{
	return(NULL);
}

#else

#if defined(__WATCOMC__) && defined(__386__)
#define FAR _far
#else
#define FAR far
#endif /* __WATCOMC__ && __386__*/

int _GrViDrvVESAgetVGAinfo(VESAvgaInfoBlock *ib)
{
	static char  *nmcopy = NULL;
	static short *mdcopy = NULL;
	Int86Regs regs;
	short FAR *mp;
	short *modes;
	char FAR *p1;
	char *p2;
	int  ii;
	DECLARE_XFER_BUFFER(1000);
	/*
	 * copy VBE 2.0 tag into XFER buffer
	 */
	setup_far_selector(LINP_SEL(XFER_BUFFER));
	p1 = (char FAR *)LINP_PTR(XFER_BUFFER);
	poke_b_f(p1,'V'); ++p1;
	poke_b_f(p1,'B'); ++p1;
	poke_b_f(p1,'E'); ++p1;
	poke_b_f(p1,'2'); ++p1;
	/*
	 * set up registers and call interrupt
	 */
	sttzero(&regs);
	IREG_AX(regs) = VESA_FUNC + VESA_VGA_INFO;
	IREG_ES(regs) = FP_SEG(XFER_BUFFER);
	IREG_DI(regs) = FP_OFF(XFER_BUFFER);
	int10x(&regs);
	if(IREG_AX(regs) != VESA_SUCCESS) {
		DELETE_XFER_BUFFER;
		return(FALSE);
	}
	/*
	 * copy VESA info block into accessible memory
	 */
	setup_far_selector(LINP_SEL(XFER_BUFFER));
	p1 = (char FAR *)LINP_PTR(XFER_BUFFER);
	p2 = (char *)ib;
	for(ii = sizeof(*ib); --ii >= 0; p1++,p2++) *p2 = peek_b_f(p1);

	if ( ib->VESAsignature[0] != 'V' ||
	     ib->VESAsignature[1] != 'E' ||
	     ib->VESAsignature[2] != 'S' ||
	     ib->VESAsignature[3] != 'A'  ) {
	  DELETE_XFER_BUFFER;
	  return(FALSE);
	}
	/*
	 * allocate space and copy mode list into accessible memory
	 */
	mp = LINP_PTR(MK_FP(FP86_SEG(ib->VideoModePtr),FP86_OFF(ib->VideoModePtr)));
	p1 = (char FAR *)mp;
	for(ii = 1; (short)peek_w_f(mp) != (short)(-1); mp++,ii++);
	modes = mdcopy = realloc(mdcopy,ii * sizeof(short));
	if(!modes) { DELETE_XFER_BUFFER; return(FALSE); }
	ib->VideoModePtr = modes;
	mp = (short far *)p1;
	for( ; --ii >= 0; mp++,modes++) *modes = peek_w_f(mp);
	/*
	 * allocate space and copy ID string into accessible memory
	 */
	p1 = LINP_PTR(MK_FP(FP86_SEG(ib->OEMstringPtr),FP86_OFF(ib->OEMstringPtr)));
	mp = (short FAR *)p1;
	for(ii = 1; (char)peek_b_f(p1) != (char)(0); p1++,ii++);
	p2 = nmcopy = realloc(nmcopy,ii * sizeof(char));
	if(!p2) { DELETE_XFER_BUFFER; return(FALSE); }
	ib->OEMstringPtr = p2;
	p1 = (char FAR *)mp;
	for( ; --ii >= 0; p1++,p2++) *p2 = peek_b_f(p1);
	DELETE_XFER_BUFFER;
	return(TRUE);
}

VESApmInfoBlock * _GrViDrvVESAgetPMinfo(void) {
  Int86Regs r;
  static VESApmInfoBlock *ib = NULL;
  unsigned Length, ii;
  char FAR *p1;
  char *p2;

  sttzero(&r);
  IREG_AX(r) = VESA_FUNC + VESA_PM_INTERF;
  IREG_BX(r) = 0x0000;
  DBGPRINTF(DBG_DRIVER,("Getting protected mode interface\n"));
  int10x(&r);
  if(IREG_AX(r) != VESA_SUCCESS)
    return(NULL);
  /* Now we have : CX    = length of table and routines (bytes)
		   ES:DI = pointer to table
			    ES_DI+00: offset PM set window routine
				 +02: offset PM set display start
				 +04: offset PM set primary palette
				 +06: offset PM description table  */
  Length = (unsigned)IREG_CX(r);
  if( Length == 0) return(NULL);
  ib = realloc(ib, Length + VESApmInfoBlock_BASEOFF);
  if (ib == NULL) return(NULL);

  ib->RealMode_SEG   = IREG_ES(r);
  ib->RealMode_OFF   = IREG_DI(r);
  ib->PhysicalLength = IREG_CX(r);

  setup_far_selector(LINP_SEL(MK_FP( IREG_ES(r),IREG_DI(r))));
  p1 = LINP_PTR( MK_FP( IREG_ES(r),IREG_DI(r) ) );
  p2 = (char *) &(ib->SetWindow_off);
  for(ii = 0; ii < Length; p1++,ii++) *(p2++) = peek_b_f(p1);
  DBGPRINTF(DBG_DRIVER,("Protected Mode Interface :-\n" ));
  DBGPRINTF(DBG_DRIVER,("  Real mode address = 0x%04x:0x%04x\n",ib->RealMode_SEG,ib->RealMode_OFF));
  DBGPRINTF(DBG_DRIVER,("  Length = 0x%08x\n",ib->PhysicalLength));
  DBGPRINTF(DBG_DRIVER,("  SetWindow function offset = 0x%08x\n",ib->SetWindow_off));
  DBGPRINTF(DBG_DRIVER,("  SetDisplayStart function offset = 0x%08x\n",ib->DisplStart_off));
  DBGPRINTF(DBG_DRIVER,("  Primary Palette function offset = 0x%08x\n",ib->PPalette_off));
  DBGPRINTF(DBG_DRIVER,("  Resource Sub-Table offset = 0x%08x\n",ib->SubTable_off));
#ifdef DUMP_PM_TABLE
  {
    static int once = 0;
    if (!once) {
      int len;
      FILE *dump = fopen("vesapm.s", "wt");
      once = 1;
      if (dump) {
	p2 = (char *) &(ib->SetWindow_off);
	fprintf(dump, ".text\n_vesa_pm_table:\n");
	for (len=0; len < Length; ++len) {
	  fprintf(dump, "\t.byte %u\n", *(unsigned char *)p2);
	  ++p2;
	}
	fclose(dump);
      }
      dump = fopen("vesapm.dmp", "wb");
      if (dump) {
	p2 = (char *) &(ib->SetWindow_off);
	fwrite(p2,Length,1,dump);
	fclose(dump);
      }
    }
  }
#endif
  return(ib);
}

int _GrViDrvVESAgetModeInfo(int mode,VESAmodeInfoBlock *ib)
{
	Int86Regs regs;
	char FAR *p1;
	char *p2;
	int  ii;
	DECLARE_XFER_BUFFER(1000);
	/*
	 * set up registers and call interrupt
	 */
	sttzero(&regs);
	IREG_AX(regs) = VESA_FUNC + VESA_MODE_INFO;
	IREG_CX(regs) = mode;
	IREG_ES(regs) = FP_SEG(XFER_BUFFER);
	IREG_DI(regs) = FP_OFF(XFER_BUFFER);
	int10x(&regs);
	if(IREG_AX(regs) != VESA_SUCCESS) {
		DELETE_XFER_BUFFER;
		return(FALSE);
	}
	/*
	 * copy VESA info block into accessible memory
	 */
	setup_far_selector(LINP_SEL(XFER_BUFFER));
	p1 = (char FAR *)LINP_PTR(XFER_BUFFER);
	p2 = (char *)ib;
	for(ii = sizeof(*ib); --ii >= 0; p1++,p2++) *p2 = peek_b_f(p1);
	DELETE_XFER_BUFFER;
	return((ib->ModeAttributes & MODE_SUPPORTED) ? TRUE : FALSE);
}

#endif

