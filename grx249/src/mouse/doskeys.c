/**
 ** doskeys.c ---- auxiliary DOS keyboard input functions
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
 **
 **/

#include "libgrx.h"
#include "grxkeys.h"
#include "int86.h"
#include "memfill.h"

#ifdef __DJGPP__
#include <pc.h>
#endif

#if defined(__WATCOMC__) || defined(__TURBOC__)
#include <conio.h>
#endif

#define USE_AT_BIOS

#ifdef  USE_AT_BIOS
#define KBD_BIOS_BASE   0x10
#else
#define KBD_BIOS_BASE   0
#endif

#if defined(__TURBOC__) || defined(__WATCOMC__) /* GS - Watcom C++ 11.0 */

int getkey(void)
{
	Int86Regs r;
	sttzero(&r);
	IREG_AX(r) = (KBD_BIOS_BASE + 0) << 8;
	int16(&r);
	switch(IREG_AL(r)) {
#ifdef USE_AT_BIOS
	  case 0xe0:
#endif
	  case 0x00:
	    return(IREG_AH(r) + 0x100);
	  default:
	    return(IREG_AL(r));
	}
}

int getxkey(void)
{
	Int86Regs r;
	sttzero(&r);
	IREG_AX(r) = (KBD_BIOS_BASE + 0) << 8;
	int16(&r);
	switch(IREG_AL(r)) {
#ifdef USE_AT_BIOS
	  case 0xe0:
	    return(IREG_AH(r) + 0x200);
#endif
	  case 0x00:
	    return(IREG_AH(r) + 0x100);
	  default:
	    return(IREG_AL(r));
	}
}

#endif

int getkbstat(void)
{
	Int86Regs r;
	sttzero(&r);
	IREG_AX(r) = (KBD_BIOS_BASE + 2) << 8;
	int16(&r);
	return(IREG_AL(r));
}


/*
** new functions to replace the old style
**   kbhit / getch / getkey / getxkey / getkbstat
** keyboard interface
*/
int GrKeyPressed(void) {
  return kbhit();
}

GrKeyType GrKeyRead(void) {
  int key = getkey();

  switch (key) {
    case 0x197: key = GrKey_Alt_Home;      break;
    case 0x198: key = GrKey_Alt_Up;        break;
    case 0x199: key = GrKey_Alt_PageUp;    break;
    case 0x19b: key = GrKey_Alt_Left;      break;
    case 0x19d: key = GrKey_Alt_Right;     break;
    case 0x19f: key = GrKey_Alt_End;       break;
    case 0x1a0: key = GrKey_Alt_Down;      break;
    case 0x1a1: key = GrKey_Alt_PageDown;  break;
    case 0x1a2: key = GrKey_Alt_Insert;    break;
    case 0x1a3: key = GrKey_Alt_Delete;    break;
  }
  return (GrKeyType) key;
}

int GrKeyStat(void) {
  return getkbstat();
}
