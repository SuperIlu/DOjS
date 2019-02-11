/**
 ** colors.c ---- color management functions
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
#include "memcopy.h"
#include "memfill.h"

static void (*DACload)(int c,int r,int g,int b) = NULL;

static void loadcolor(int c,int r,int g,int b)
{
	CLRINFO->ctable[c].r = (r &= CLRINFO->mask[0]);
	CLRINFO->ctable[c].g = (g &= CLRINFO->mask[1]);
	CLRINFO->ctable[c].b = (b &= CLRINFO->mask[2]);
	if(DACload) (*DACload)(c,r,g,b);
}

static void setbits(char *prec,char *pos)
{
	int i,tmp;
	CLRINFO->norm = 0;
	for(i = 0; i < 3; i++,prec++,pos++) {
	    CLRINFO->prec[i]  = *prec;
	    CLRINFO->pos[i]   = *prec + *pos - 1;
	    CLRINFO->mask[i]  = (tmp = 0xff ^ (0xff >> *prec));
	    CLRINFO->round[i] = (tmp >> 1) & ~tmp;
	    CLRINFO->shift[i] = (tmp = CLRINFO->pos[i] - 7);
	    CLRINFO->norm     = imax(CLRINFO->norm,(-tmp));
	}
	CLRINFO->shift[0] += CLRINFO->norm;
	CLRINFO->shift[1] += CLRINFO->norm;
	CLRINFO->shift[2] += CLRINFO->norm;
}

void GrRefreshColors(void)
{
	int i;
	for(i = 0; i < (int)CLRINFO->ncolors; i++) {
	    if(CLRINFO->ctable[i].defined) loadcolor(
		(int)(i),
		CLRINFO->ctable[i].r,
		CLRINFO->ctable[i].g,
		CLRINFO->ctable[i].b
	    );
	}
}

/* _GR_firstFreeColor is normally zero but some systems may protect some
** colors for other programs (eg. X11). In this case we don't touch them.
** These variables are only used in palette modes
*/
#ifdef __XWIN__
int _GR_firstFreeColor =  0;
int _GR_lastFreeColor  = -1;
#else
#define _GR_firstFreeColor 0
#endif

int _GrResetColors(void)
{
#       define NSAVED 16
	static char *infosave = NULL;
	static int  firsttime = TRUE;
	int i;
	if(firsttime) {
		infosave = malloc(offsetof(struct _GR_colorInfo,ctable[NSAVED]));
		if ( infosave ) {
			memcpy(infosave,CLRINFO,offsetof(struct _GR_colorInfo,ctable[NSAVED]));
		}
	    firsttime = FALSE;
	}
	sttzero(CLRINFO);
	if(DRVINFO->actmode.extinfo->mode == GR_frameText) {
		if ( infosave ) {
			memcpy(CLRINFO,infosave,sizeof(infosave));
			return TRUE;
		}
		return FALSE;
	}
	DACload = DRVINFO->actmode.extinfo->loadcolor;
	CLRINFO->black   = GrNOCOLOR;
	CLRINFO->white   = GrNOCOLOR;
	CLRINFO->ncolors = DRVINFO->actmode.bpp>=32 ? 0 : (1L << DRVINFO->actmode.bpp);
	if ( ((CLRINFO->ncolors-1)&GrCVALUEMASK) != (CLRINFO->ncolors-1) ) {
	    /* can happen on 32bpp systems. */
	    int cbpp = 0;
	    for(i=0; i < 3; ++i)
		cbpp += DRVINFO->actmode.extinfo->cprec[i];
	    CLRINFO->ncolors = 1L << cbpp;
	}
	setbits(
	    DRVINFO->actmode.extinfo->cprec,
	    DRVINFO->actmode.extinfo->cpos
	);
	switch(DRVINFO->actmode.bpp) {
	  case 4:
	  case 8:
#ifdef __XWIN__
	    if (_GR_lastFreeColor >= _GR_firstFreeColor)
	      CLRINFO->nfree = _GR_lastFreeColor - _GR_firstFreeColor + 1;
	    else
#endif
	      CLRINFO->nfree = CLRINFO->ncolors - _GR_firstFreeColor;
		if ( infosave ) {
	      for(i = 0; i < NSAVED; i++) {
		  loadcolor(
		      (i + _GR_firstFreeColor),
		      ((struct _GR_colorInfo *)(infosave))->ctable[i].r,
		      ((struct _GR_colorInfo *)(infosave))->ctable[i].g,
		      ((struct _GR_colorInfo *)(infosave))->ctable[i].b
		  );
		  CLRINFO->ctable[i].defined = TRUE;
		  }
	    }
	    break;
	  default:
	    CLRINFO->RGBmode = TRUE;
	    break;
	}
	return ((CLRINFO->ncolors-1)&GrCVALUEMASK) == CLRINFO->ncolors-1;
}
 
void GrResetColors(void)
{
	_GrResetColors();
}

void GrSetRGBcolorMode(void)
{
	if(!CLRINFO->RGBmode) {
	    GrColor c;
	    switch(CLRINFO->ncolors) {
		case 16L:  setbits("\1\2\1","\3\1\0"); break;
		case 256L: setbits("\3\3\2","\5\2\0"); break;
		default:   return;
	    }
	    CLRINFO->RGBmode = TRUE;
	    CLRINFO->nfree   = 0L;
	    CLRINFO->black   = 0L;
	    CLRINFO->white   = CLRINFO->ncolors - 1L;
	    for(c = 0; c < CLRINFO->ncolors; c++) loadcolor(
		(int)(c),
		(int)GrRGBcolorRed(c),
		(int)GrRGBcolorGreen(c),
		(int)GrRGBcolorBlue(c)
	    );
	}
}

#define ROUNDCOLORCOMP(x,n) (                                   \
    ((unsigned int)(x) >= CLRINFO->mask[n]) ?                   \
	CLRINFO->mask[n] :                                      \
	(((x) + CLRINFO->round[n]) & CLRINFO->mask[n])          \
)

GrColor GrAllocColor(int r,int g,int b)
{
        GrColor res;

        GRX_ENTER();
        res = GrNOCOLOR;
	r = ROUNDCOLORCOMP(r,0);
	g = ROUNDCOLORCOMP(g,1);
	b = ROUNDCOLORCOMP(b,2);
	if(CLRINFO->RGBmode) {
            res = GrBuildRGBcolorT(r,g,b);
	}
	else {
            GR_int32u minerr = 1000;
	    int i;
	    int free_ = (-1),allfree = (-1),best = (-1);
	    int ndef = (int)CLRINFO->ncolors - (int)CLRINFO->nfree;
            DBGPRINTF(DBG_COLOR,("Allocating color: r=%d, g=%d, b=%d\n",r,g,b));
	    for(i = 0; i < (int)CLRINFO->ncolors; i++) {
		if(CLRINFO->ctable[i].defined) {
		    if(!CLRINFO->ctable[i].writable) {
                        GR_int32u err = 0;
                        GR_int16u colerr;
                        colerr = iabs(r - CLRINFO->ctable[i].r);
                        colerr = colerr * colerr;
                        err += colerr;
                        colerr = iabs(g - CLRINFO->ctable[i].g);
                        colerr = colerr * colerr;
                        err += colerr;
                        colerr = iabs(b - CLRINFO->ctable[i].b);
                        colerr = colerr * colerr;
                        err += colerr;
			if(err < minerr) {
                            DBGPRINTF(DBG_COLOR,("New best color %d (err=%ld): r=%d, g=%d, b=%d\n", i, err, \
                              (int)CLRINFO->ctable[i].r,(int)CLRINFO->ctable[i].g,(int)CLRINFO->ctable[i].b));
			    best = i;
			    if((minerr = err) == 0) goto foundbest;
			}
			if((free_ <= 0) && !CLRINFO->ctable[i].nused) {
                            DBGPRINTF(DBG_COLOR,("First free color: r=%d\n", i));
			    free_ = i;
			}
		    }
		    if(CLRINFO->ctable[i].nused) ndef--;
		}
		else {
		    if(allfree < 0) allfree = i;
		}
		if((allfree >= 0) && (ndef <= 0)) {
                    DBGPRINTF(DBG_COLOR,("Found a usable color: allfree = %d, ndef = %d\n", allfree, ndef));
		    break;
		}
	    }
	    if(allfree >= 0) {
                DBGPRINTF(DBG_COLOR,("Using %d as free color (free=%d)\n", allfree, free_));
		free_ = allfree;
	    }
	    if(free_ >= 0) {
                DBGPRINTF(DBG_COLOR,("Allocating %d\n", free_));
		CLRINFO->ctable[free_].defined  = TRUE;
		CLRINFO->ctable[free_].writable = FALSE;
		CLRINFO->ctable[free_].nused    = 1;
		CLRINFO->nfree--;
		loadcolor(free_,r,g,b);
		res = free_;
                goto done;
	    }
	  foundbest:
	    if(best >= 0) {
                DBGPRINTF(DBG_COLOR,("Using best %d\n", best));
		if(!CLRINFO->ctable[best].nused) CLRINFO->nfree--;
		CLRINFO->ctable[best].nused++;
		res = best;
                goto done;
	    }
	}
done:   GRX_RETURN(res);
}

GrColor GrAllocCell(void)
{
	if(!CLRINFO->RGBmode && CLRINFO->nfree) {
	    int i,free_ = (-1);
	    for(i = 0; i < (int)CLRINFO->ncolors; i++) {
		if(!CLRINFO->ctable[i].defined) {
		    free_ = i;
		    break;
		}
		if(!CLRINFO->ctable[i].nused) {
		    if(free_ < 0) free_ = i;
		}
	    }
	    if(free_ >= 0) {
		CLRINFO->ctable[free_].defined  = TRUE;
		CLRINFO->ctable[free_].writable = TRUE;
		CLRINFO->ctable[free_].nused    = 1;
		CLRINFO->nfree--;
		loadcolor(free_,0,0,0);
		return((GrColor)(free_));
	    }
	}
	return(GrNOCOLOR);
}

void GrFreeColor(GrColor c)
{
	if(!CLRINFO->RGBmode && ((GrColor)(c) < CLRINFO->ncolors) &&
	    !CLRINFO->ctable[(int)(c)].writable &&
	    CLRINFO->ctable[(int)(c)].defined &&
	    (--CLRINFO->ctable[(int)(c)].nused == 0)) {
		CLRINFO->nfree++;
		CLRINFO->ctable[(int)(c)].defined  = FALSE;
		CLRINFO->ctable[(int)(c)].writable = FALSE;
		CLRINFO->ctable[(int)(c)].nused    = 0;
	    }
}

void GrFreeCell(GrColor c)
{
        GRX_ENTER();
	if(!CLRINFO->RGBmode && ((GrColor)(c) < CLRINFO->ncolors)) {
	    if(CLRINFO->ctable[(int)(c)].writable) {
		CLRINFO->nfree++;
		CLRINFO->ctable[(int)(c)].defined  = FALSE;
		CLRINFO->ctable[(int)(c)].writable = FALSE;
		CLRINFO->ctable[(int)(c)].nused    = 0;
	    }
	}
        GRX_LEAVE();
}

void GrSetColor(GrColor c,int r,int g,int b)
{
        GRX_ENTER();
	if(!CLRINFO->RGBmode && ((GrColor)(c) < CLRINFO->ncolors)) {
	    if(!CLRINFO->ctable[(int)(c)].defined) {
		CLRINFO->ctable[(int)(c)].defined  = TRUE;
		CLRINFO->ctable[(int)(c)].nused    = 0;
	    }
	    if(!CLRINFO->ctable[(int)(c)].nused) {
		CLRINFO->ctable[(int)(c)].writable = TRUE;
		CLRINFO->ctable[(int)(c)].nused    = 1;
		CLRINFO->nfree--;
	    }
	    if(CLRINFO->ctable[(int)(c)].writable) loadcolor(
		(int)(c),
		(int)ROUNDCOLORCOMP(r,0),
		(int)ROUNDCOLORCOMP(g,1),
		(int)ROUNDCOLORCOMP(b,2)
	    );
	}
        GRX_LEAVE();
}

void GrQueryColor(GrColor c,int *r,int *g,int *b)
{
        GRX_ENTER();
	GrQueryColorID(c,r,g,b);
        GRX_LEAVE();
}

void GrQueryColor2(GrColor c,long *hcolor)
{
        GRX_ENTER();
        GrQueryColor2ID(c,hcolor);
        GRX_LEAVE();
}

#define CSAVE_MAGIC     0x7abf5698UL

typedef struct {
    GrColor magic;
    GrColor nc;
    struct _GR_colorInfo info;
} colorsave;

int GrColorSaveBufferSize(void)
{
	return(sizeof(colorsave));
}

void GrSaveColors(void *buffer)
{
	colorsave *cp = (colorsave *)buffer;
	cp->magic = CSAVE_MAGIC;
	cp->nc    = GrNumColors();
	sttcopy(&cp->info,CLRINFO);
}

void GrRestoreColors(void *buffer)
{
	colorsave *cp = (colorsave *)buffer;
	if((cp->magic == CSAVE_MAGIC) && (cp->nc == GrNumColors())) {
	    sttcopy(CLRINFO,&cp->info);
	    GrRefreshColors();
	}
}

