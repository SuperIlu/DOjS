/**
 ** BCC2GRX  -  Interfacing Borland based graphics programs to LIBGRX
 ** Copyright (C) 1993-97 by Hartmut Schirmer
 **
 **
 ** Contact :                Hartmut Schirmer
 **                          Feldstrasse 118
 **                  D-24105 Kiel
 **                          Germany
 **
 ** e-mail : hsc@techfak.uni-kiel.de
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

#include "bccgrx00.h"

void set_BGI_mode(int *graphdriver, int *graphmode)
{
  int rx = 0;
  int ry = 0;
  int rc = 0;

  switch (*graphdriver) {
    case VGA     : switch (*graphmode) {
		     case VGALO : rx = 640; ry = 200; rc = 16; break;
		     case VGAMED: rx = 640; ry = 350; rc = 16; break;
		     case VGAHI : rx = 640; ry = 480; rc = 16; break;
		   }
		   break;
    case IBM8514 : switch (*graphmode) {
		     case IBM8514LO: rx =  640; ry = 480; rc = 256; break;
		     case IBM8514HI: rx = 1024; ry = 768; rc = 256; break;
		   }
		   break;
    case HERCMONO: if (*graphmode == HERCMONOHI) {
		     rx = 720;
		     ry = ((__gr_ADAPTER == GR_HERC) ? 348 : 350);
		     rc = ((__gr_ADAPTER == GR_HERC) ?   2 :  16);
		   }
		   break;
    case CGA     :
    case MCGA    :
    case ATT400  : switch (*graphmode) {
		     case CGAC0 :
		     case CGAC1 :
		     case CGAC2 :
		     case CGAC3 : rx = 320; ry = 200;            break;
		     case CGAHI : /* == MCGAMED == ATT400MED */
				  rx = 640; ry = 200;            break;
		     case MCGAHI: /* == ATT400HI */
				  switch (*graphdriver) {
				    case MCGA  : rx = 640; ry = 480; break;
				    case ATT400: rx = 640; ry = 400; break;
				  }
				  break;
		   }
		   rc = 16;
		   break;
    case EGA64   : switch (*graphmode) {
		     case EGA64LO : rx = 640; ry = 200; rc = 16;     break;
		     case EGA64HI : rx = 640;
				    ry = 350;
				    rc = ((__gr_ADAPTER == GR_EGA) ? 4 : 16);
				    break;
		   }
		   break;
    case EGA     : switch (*graphmode) {
		     case EGALO : rx = 640; ry = 200; rc = 16;       break;
		     case EGAHI : rx = 640; ry = 350; rc = 16;       break;
		   }
		   break;
    case EGAMONO : if (*graphmode == EGAMONOHI || *graphmode == EGAMONOHI_PAS) {
		     rx = 640; ry = 350; rc = 16;
		   }
		   break;
    case PC3270  : if (*graphmode == PC3270HI) {
		     rx = 720; ry = 350; rc = 16;
		   }
		   break;
    /* ---- extende modes from BCC++ 4.5 */
    /* unknown modes - assume 16 colors */
    case ATTDEB:
    case COMPAQ:
    case GENOA5:
    case GENOA6:
    case OAK:
    case TECMAR:
    case TOSHIBA:
    case VIDEO7:
    case VIDEO7II:
    case S3:
    case ATIGUP:
    /* 16 color modes */
    case ATI16:
    case PARADIS16:
    case SVGA16:
    case TRIDENT16:
    case TRIDENT256:
    case TSENG316:
    case TSENG416:
    case VESA16:        rc = 16;
			goto ExModes;
    /* 256 color modes */
    case ATI256:
    case PARADIS256:
    case SVGA256:
    case TSENG3256:
    case TSENG4256:
    case VESA256:
    case VGA256:        rc = 256;
			goto ExModes;
    /* 32k color modes */
    case ATI32K:
    case SVGA32K:
    case TSENG432K:
    case VESA32K:       rc = 1<<15;
			goto ExModes;
    /* 64k color modes */
    case SVGA64K:
    case VESA64K:       rc = 1<<16;
			goto ExModes;
    /* 16M color modes */
    case VESA16M:       rc = 1<<24;
		ExModes:switch (*graphmode) {
			  case RES640x350  : rx =  640; ry =  350; break;
			  case RES640x480  : rx =  640; ry =  480; break;
			  case RES800x600  : rx =  800; ry =  600; break;
			  case RES1024x768 : rx = 1024; ry =  768; break;
			  case RES1280x1024: rx = 1280; ry = 1024; break;
			}
			break;
    default      : ERR = grInvalidDriver;
		   return;
  }

  if (rx != 0 && ry != 0 && rc != 0) {
    set_BGI_mode_pages(1);
    __gr_BGI_w = rx;
    __gr_BGI_h = ry;
    __gr_BGI_c = rc;
    *graphdriver = NATIVE_GRX;
    *graphmode   = GRX_BGI_EMULATION;
  } else
    ERR = grInvalidMode;
}
