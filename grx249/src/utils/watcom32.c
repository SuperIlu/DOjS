/**
 ** watcom32.c ---- Watcom32 support functions
 **
 ** Copyright (c) 1998 Hartmut Schirmer & Gary Sands
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
#include "int86.h"
#include "memfill.h"

void wc32_int10x(Int86Regs *iregp, int nSeg) {
   static struct rminfo {
	   long EDI;
	   long ESI;
	   long EBP;
	   long reserved_by_system;
	   long EBX;
	   long EDX;
	   long ECX;
	   long EAX;
	   short flags;
	   short ES,DS,FS,GS,IP,CS,SP,SS;
   } RMI;
   _fmemset ( &RMI,0,sizeof(RMI) );
   RMI.EDI = IREG_DI(*iregp);
   RMI.ESI = IREG_SI(*iregp);
   RMI.EBX = IREG_BX(*iregp);
   RMI.EDX = IREG_DX(*iregp);
   RMI.ECX = IREG_CX(*iregp);
   RMI.EAX = IREG_AX(*iregp);
   /* RMI.ES = IREG_ES(iregp); */
   RMI.ES = nSeg;
   RMI.DS = IREG_DS(*iregp);
   sttzero(iregp);
   IREG_AX(*iregp) = 0x300;
   IREG_BL(*iregp) = 0x10;
   IREG_BH(*iregp) = 0;
   IREG_CX(*iregp) = 0;
   IREG_ES(*iregp) = FP_SEG(&RMI);
   IREG_EDI(*iregp) = FP_OFF(&RMI);
   int386x( 0x31, &(iregp->Normal), &(iregp->Normal), &(iregp->Extended) );
   sttzero(iregp);
   IREG_DI(*iregp) = RMI.EDI;
   IREG_SI(*iregp) = RMI.ESI;
   IREG_BX(*iregp) = RMI.EBX;
   IREG_DX(*iregp) = RMI.EDX;
   IREG_CX(*iregp) = RMI.ECX;
   IREG_AX(*iregp) = RMI.EAX;
   IREG_ES(*iregp) = RMI.ES;
   IREG_DS(*iregp) = RMI.DS;
}
