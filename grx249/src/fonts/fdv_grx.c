/**
 ** fdv_grx.c -- driver for GRX native font file format
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

#include <stdio.h>
#include <string.h>

#include "libgrx.h"
#include "grfontdv.h"
#include "allocate.h"
#include "fonts/fdv_grx.h"

#ifndef  SEEK_SET
#define  SEEK_SET       0
#endif

static GrFontFileHeaderGRX fhdr;
static FILE   *fontfp = NULL;
static GR_int16u far *wtable = NULL;
static unsigned int wtsize = 0;
static int     nextch = 0;

#if BYTE_ORDER==BIG_ENDIAN
#include "ordswap.h"
static void swap_header(void) {
    GRX_ENTER();
    _GR_swap32u(&fhdr.magic);
    _GR_swap32u(&fhdr.bmpsize);
    _GR_swap16u(&fhdr.width);
    _GR_swap16u(&fhdr.height);
    _GR_swap16u(&fhdr.minchar);
    _GR_swap16u(&fhdr.maxchar);
    _GR_swap16u(&fhdr.isfixed);
    _GR_swap16u(&fhdr.reserved);
    _GR_swap16u(&fhdr.baseline);
    _GR_swap16u(&fhdr.undwidth);
    /* no need to change fnname  && family */
    GRX_LEAVE();
}

static void swap_wtable(void) {
  GR_int16u far *wt;
  unsigned int   ws;
  GRX_ENTER();
  wt = wtable;
  ws = wtsize / sizeof(GR_int16u);
  while (ws-- > 0) {
    _GR_swap16u(wt);
    ++wt;
  }
  GRX_LEAVE();
}
#endif

static void cleanup(void)
{
	GRX_ENTER();
	if(fontfp != NULL) fclose(fontfp);
	if(wtable != NULL) farfree(wtable);
	fontfp = NULL;
	wtable = NULL;
	nextch = 0;
	wtsize = 0;
	GRX_LEAVE();
}

static int openfile(char *fname)
{
	int res;
#if BYTE_ORDER==BIG_ENDIAN
	int swap;
#endif
	GRX_ENTER();
	res = FALSE;
	cleanup();
        fontfp = fopen(fname,"rb");
	if(fontfp == NULL) {
            DBGPRINTF(DBG_FONT,("fopen(\"%s\") failed\n", fname));
	    goto done;
	}
	if(fread(&fhdr,sizeof(fhdr),1,fontfp) != 1) {
	    DBGPRINTF(DBG_FONT,("reading header failed\n"));
	    goto done;
	}
#if BYTE_ORDER==BIG_ENDIAN
	swap = 0;
	if(fhdr.magic == GRX_FONTMAGIC_SWAPPED) {
	  swap = 1;
	  DBGPRINTF(DBG_FONT,("swaping header byte order\n"));
	  swap_header();
	}
#endif
	if(fhdr.magic != GRX_FONTMAGIC) {
	    DBGPRINTF(DBG_FONT,("font magic doesn't fit: %lx != %lx\n", \
		   (unsigned long)fhdr.magic,(unsigned long)GRX_FONTMAGIC));
	    goto done;
	}
	if(!fhdr.isfixed) {
	    wtsize = sizeof(GR_int16u) * (fhdr.maxchar - fhdr.minchar + 1);
	    wtable = farmalloc(wtsize);
	    if(wtable == NULL) {
		DBGPRINTF(DBG_FONT,("Allocating wtable failed\n"));
		goto done;
	    }
	    if(fread(wtable,wtsize,1,fontfp) != 1) {
		DBGPRINTF(DBG_FONT,("Loading wtable failed\n"));
		goto done;
	    }
#if BYTE_ORDER==BIG_ENDIAN
	    if (swap) {
	      DBGPRINTF(DBG_FONT,("swaping wtable byte order\n"));
	      swap_wtable();
	    }
#endif
	}
	nextch = fhdr.minchar;
	res = TRUE;
done:   if (!res) cleanup();
	GRX_RETURN(res);
}

static int header(GrFontHeader *hdr)
{
	int res;
	GRX_ENTER();
	res = FALSE;
	if(fontfp != NULL) {
	    memcpy(hdr->name,  fhdr.fnname,sizeof(fhdr.fnname));
	    memcpy(hdr->family,fhdr.family,sizeof(fhdr.family));
	    hdr->name  [sizeof(fhdr.fnname)] = '\0';
	    hdr->family[sizeof(fhdr.family)] = '\0';
	    hdr->proportional = fhdr.isfixed ? FALSE : TRUE;
	    hdr->scalable     = FALSE;
	    hdr->preloaded    = FALSE;
	    hdr->modified     = GR_FONTCVT_NONE;
	    hdr->width        = fhdr.width;
	    hdr->height       = fhdr.height;
	    hdr->baseline     = fhdr.baseline;
	    hdr->ulpos        = fhdr.height - fhdr.undwidth;
	    hdr->ulheight     = fhdr.undwidth;
	    hdr->minchar      = fhdr.minchar;
	    hdr->numchars     = fhdr.maxchar - fhdr.minchar + 1;
	    res = TRUE;
	}
	GRX_RETURN(res);
}

static int charwdt(int chr)
{
	int res;
	GRX_ENTER();
	res = -1;
	if(fontfp != NULL &&
	   chr >= fhdr.minchar &&
	   chr <= fhdr.maxchar    )
	  res = (fhdr.isfixed ? fhdr.width : wtable[chr - fhdr.minchar]);
	GRX_RETURN(res);
}

static int bitmap(int chr,int w,int h,char *buffer)
{
	int res;
	GRX_ENTER();
	res = FALSE;
	if( (w > 0) && (w == charwdt(chr))
	  &&(h > 0) && (h == fhdr.height) ) {
	    if(chr != nextch) {
		long fpos = sizeof(fhdr) + (fhdr.isfixed ? 0 : wtsize);
		for(nextch = fhdr.minchar; nextch != chr; nextch++) {
		    fpos += ((charwdt(nextch) + 7) >> 3) * fhdr.height;
		}
		fseek(fontfp,fpos,SEEK_SET);
	    }
	    nextch = chr + 1;
	    res = fread(buffer,(((w + 7) >> 3) * h),1,fontfp) == 1;
	}
	GRX_RETURN(res);
}

GrFontDriver _GrFontDriverGRX = {
    "GRX",                              /* driver name (doc only) */
    ".fnt",                             /* font file extension */
    FALSE,                              /* scalable */
    openfile,                           /* file open and check routine */
    header,                             /* font header reader routine */
    charwdt,                            /* character width reader routine */
    bitmap,                             /* character bitmap reader routine */
    cleanup                             /* cleanup routine */
};

