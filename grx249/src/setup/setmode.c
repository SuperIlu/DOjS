/**
 ** setmode.c ---- video mode setup
 **
 ** Copyright (c) 1995 Csaba Biegl, 820 Stirrup Dr, Nashville, TN 37221
 ** [e-mail: csaba@vuse.vanderbilt.edu]
 ** Hartmut Schirmer (hsc@techfak.uni-kiel.de)
 ** Andrzej Lawa [FidoNet: Andrzej Lawa 2:480/19.77]
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

#include <ctype.h>
#include <stdarg.h>

#include "libgrx.h"
#include "arith.h"
#include "memfill.h"
#include "memcopy.h"
#include "grdriver.h"

GrVideoMode * _gr_selectmode(GrVideoDriver *drv,int w,int h,int bpp,
	    int txt,unsigned int *ep)
{
#       define ERROR(des,act)   ((des > act) ? ((des - act) + 20000) : (act - des))
	int  n;
	unsigned int cerr,serr,err[2];
	GrVideoMode *best,*mp;
	GRX_ENTER();
	best = NULL;
	mp = drv->modes;
	for(n = drv->nmodes; --n >= 0; mp++) {
	    if(!mp->present) continue;
	    if(!mp->extinfo) continue;
	    if((mp->extinfo->mode != GR_frameText) ? txt : !txt) continue;
	    cerr = ERROR(bpp,mp->bpp);
	    serr = ERROR(w,mp->width) + ERROR(h,mp->height);
	    if(((ep) ? FALSE : ((ep = err),TRUE)) ||
	       ((cerr <  ep[0])) ||
	       ((cerr == ep[0]) && (serr < ep[1]))) {
		best  = mp;
                if (!cerr && !serr) break;
		ep[0] = cerr;
                ep[1] = serr;
	    }
	}
	if(drv->inherit) {
	    mp = (drv->inherit->selectmode) (drv->inherit,w,h,bpp,txt,ep);
	    if(mp) best = mp;
	}
	GRX_RETURN(best);
}

static int buildframedriver(GrVideoMode *mp,GrFrameDriver *drv)
{
	GrFrameDriver *d1, *d2;
	int res = TRUE;
	GRX_ENTER();
	res = TRUE;
	d1 = _GrFindFrameDriver(mp->extinfo->mode);
	d2 = mp->extinfo->drv;
	if(d1) sttcopy(drv,d1);
	if(d2) {
	    int compl = TRUE;
#           define MERGE(F) if(d2->F) drv->F = d2->F; else compl = FALSE;
	    MERGE(readpixel);
	    MERGE(drawpixel);
	    MERGE(drawhline);
	    MERGE(drawvline);
	    MERGE(drawblock);
	    MERGE(drawline);
	    MERGE(drawbitmap);
	    MERGE(drawpattern);
	    MERGE(bitblt);
	    MERGE(bltv2r);
	    MERGE(bltr2v);
	    MERGE(getindexedscanline);
	    MERGE(putscanline);
	    if(compl) {
		memcpy(drv,d2,offsetof(GrFrameDriver,readpixel));
		goto done; /* TRUE */
	    }
	    if(!d1) { res = FALSE; goto done; }
	    if((d2->mode == d1->mode) &&
	       (d2->rmode == d1->rmode) &&
	       (d1->is_video ? d2->is_video : !d2->is_video) &&
	       (d2->row_align <= d1->row_align) && !(d1->row_align % d2->row_align) &&
	       (d2->num_planes == d1->num_planes) &&
	       (d2->max_plane_size >= d1->max_plane_size) &&
	       (d2->bits_per_pixel == d1->bits_per_pixel)) {
		drv->init = d2->init ? d2->init : d1->init;
		goto done; /* TRUE */
	    }
	}
	if(!d1) { res = FALSE; goto done; }
	sttcopy(drv,d1);
done:   GRX_RETURN(res);
}

static int buildcontext(GrVideoMode *mp,GrFrameDriver *fdp,GrContext *cxt)
{
	long plsize;
	int res;
	GRX_ENTER();
	res = FALSE;
	plsize = umul32(mp->lineoffset,mp->height);
	DBGPRINTF(DBG_SETMD,("buildcontext - Mode Frame buffer = 0x%x\n",mp->extinfo->frame));
	DBGPRINTF(DBG_SETMD,("buildcontext - Mode Frame selector = 0x%x\n",mp->extinfo->LFB_Selector));
	sttzero(cxt);
#if !defined(__TURBOC__) \
 && !(defined(__WATCOMC__) && !defined(__386__)) \
 && !(defined(__XWIN__) && !defined(XF86DGA_FRAMEBUFFER) && !defined(__SDL__))
	if(mp->extinfo->flags&GR_VMODEF_LINEAR)
	{
	    DBGPRINTF(DBG_SETMD,("buildcontext - Linear Mode\n"));
	    cxt->gc_baseaddr[0] =
	    cxt->gc_baseaddr[1] =
	    cxt->gc_baseaddr[2] =
	    cxt->gc_baseaddr[3] = LINP_PTR(mp->extinfo->frame);
	    cxt->gc_selector    = mp->extinfo->LFB_Selector;
	} else
#endif /* !__TURBOC__ && !( __WATCOMC__ && !__386__) && !(__XWIN__ && !XF86DGA_FRAMEBUFFER && !__SDL__) */
	if (mp->extinfo->flags&GR_VMODEF_MEMORY)
	{
	    DBGPRINTF(DBG_SETMD,("buildcontext - Memory Mode\n"));
	    if(plsize > fdp->max_plane_size) goto done; /* FALSE */
	    if(mp->lineoffset % fdp->row_align) goto done; /* FALSE */
	    DBGPRINTF(DBG_SETMD,("buildcontext - mp->present    = %d\n",mp->present));
	    DBGPRINTF(DBG_SETMD,("buildcontext - mp->bpp        = %d\n",mp->bpp));
	    DBGPRINTF(DBG_SETMD,("buildcontext - mp->width      = %d\n",mp->width));
	    DBGPRINTF(DBG_SETMD,("buildcontext - mp->height     = %d\n",mp->height));
	    DBGPRINTF(DBG_SETMD,("buildcontext - mp->mode       = %d\n",mp->mode));
	    DBGPRINTF(DBG_SETMD,("buildcontext - mp->lineoffset = %d\n",mp->lineoffset));
	    DBGPRINTF(DBG_SETMD,("buildcontext - mp->ext->mode  = %d\n",mp->extinfo->mode));
	    DBGPRINTF(DBG_SETMD,("buildcontext - mp->ext->flags = %x\n",mp->extinfo->flags));
#ifdef GRX_USE_RAM3x8
	    if (mp->bpp==24)
	      {
		 int m_incr = mp->lineoffset*mp->height;
		 cxt->gc_baseaddr[0] = mp->extinfo->frame;
		 cxt->gc_baseaddr[1] = cxt->gc_baseaddr[0] + m_incr;
		 cxt->gc_baseaddr[2] = cxt->gc_baseaddr[1] + m_incr;
		 cxt->gc_baseaddr[3] = NULL;
	      }
            else
#endif
	    if (mp->bpp==4)
	      {
		 int m_incr = mp->lineoffset*mp->height;
		 cxt->gc_baseaddr[0] = mp->extinfo->frame;
		 cxt->gc_baseaddr[1] = cxt->gc_baseaddr[0] + m_incr;
		 cxt->gc_baseaddr[2] = cxt->gc_baseaddr[1] + m_incr;
		 cxt->gc_baseaddr[3] = cxt->gc_baseaddr[2] + m_incr;
	      }
	    else
	      {
		 cxt->gc_baseaddr[0] =
		 cxt->gc_baseaddr[1] =
		 cxt->gc_baseaddr[2] =
		 cxt->gc_baseaddr[3] = mp->extinfo->frame;
	      }
#if defined(__TURBOC__) || (defined(__WATCOMC__) && !defined(__386__))
	    cxt->gc_selector    = LINP_SEL(mp->extinfo->frame);
#endif
	}
	else
	{
	    if(plsize > fdp->max_plane_size) goto done; /* FALSE */
	    if(!mp->extinfo->setbank && (plsize > 0x10000L)) goto done; /* FALSE */
	    if(mp->lineoffset % fdp->row_align) goto done; /* FALSE */
	    cxt->gc_baseaddr[0] =
	    cxt->gc_baseaddr[1] =
	    cxt->gc_baseaddr[2] =
	    cxt->gc_baseaddr[3] = LINP_PTR(mp->extinfo->frame);
	    cxt->gc_selector    = LINP_SEL(mp->extinfo->frame);
	}
	cxt->gc_onscreen    = !(mp->extinfo->flags&GR_VMODEF_MEMORY);
	/* Why do we default to screen driver ?? */
	cxt->gc_onscreen    = TRUE;
	cxt->gc_lineoffset  = mp->lineoffset;
	cxt->gc_xcliphi     = cxt->gc_xmax = mp->width  - 1;
	cxt->gc_ycliphi     = cxt->gc_ymax = mp->height - 1;
	cxt->gc_driver      = &DRVINFO->sdriver;

	res = TRUE;

	DBGPRINTF(DBG_SETMD,("buildcontext - context buffer 0 = 0x%x\n",cxt->gc_baseaddr[0]));
	DBGPRINTF(DBG_SETMD,("buildcontext - context buffer 1 = 0x%x\n",cxt->gc_baseaddr[1]));
	DBGPRINTF(DBG_SETMD,("buildcontext - context buffer 2 = 0x%x\n",cxt->gc_baseaddr[2]));
	DBGPRINTF(DBG_SETMD,("buildcontext - context buffer 3 = 0x%x\n",cxt->gc_baseaddr[3]));
done:
	GRX_RETURN(res);
}

static int errhdlr(char *msg)
{
	if(DRVINFO->errsfatal) {
	    DRVINFO->moderestore = TRUE;
	    _GrCloseVideoDriver();
	    fprintf(stderr,"GrSetMode: %s\n",msg);
	    exit(1);
	}
	return(FALSE);
}

int GrSetMode(GrGraphicsMode which,...)
{
	int  w,h,pl,vw,vh;
	int  t,noclear,res;
	GrColor c;
	va_list ap;
	GRX_ENTER();
	pl = 0;
	vw = 0;
	vh = 0;
	t = FALSE;
	noclear = FALSE;
	c = 0;
	res = FALSE;
	DBGPRINTF(DBG_SETMD,("Mode: %d\n",(int)which));
	if(DRVINFO->vdriver == NULL) {
	    GrSetDriver(NULL);
	    if(DRVINFO->vdriver == NULL) {
		res = errhdlr("could not find suitable video driver");
		goto done;
	    }
	}
	va_start(ap,which);
	switch(which) {
	  case GR_NC_80_25_text:
	    noclear = TRUE;
	  case GR_80_25_text:
	    w = 80;
	    h = 25;
	    c = DRVINFO->deftc;
	    t = TRUE;
	    break;
	  case GR_NC_default_text:
	    noclear = TRUE;
	  case GR_default_text:
	    w = DRVINFO->deftw;
	    h = DRVINFO->defth;
	    c = DRVINFO->deftc;
	    t = TRUE;
	    break;
	  case GR_NC_width_height_text:
	    noclear = TRUE;
	  case GR_width_height_text:
	    w = va_arg(ap,int);
	    h = va_arg(ap,int);
	    c = DRVINFO->deftc;
	    t = TRUE;
	    break;
	  case GR_NC_biggest_text:
	    noclear = TRUE;
	  case GR_biggest_text:
	    w = 200;
	    h = 80;
	    c = DRVINFO->deftc;
	    t = TRUE;
	    break;
	  case GR_NC_width_height_color_text:
	    noclear = TRUE;
	  case GR_width_height_color_text:
	    w = va_arg(ap,int);
	    h = va_arg(ap,int);
	    c = va_arg(ap,GrColor);
	    t = TRUE;
	    break;
	  case GR_NC_width_height_bpp_text:
	    noclear = TRUE;
	  case GR_width_height_bpp_text:
	    w  = va_arg(ap,int);
	    h  = va_arg(ap,int);
	    pl = va_arg(ap,int);
	    t  = TRUE;
	    break;
	  case GR_NC_320_200_graphics:
	    noclear = TRUE;
	  case GR_320_200_graphics:
	    w = 320;
	    h = 200;
	    c = DRVINFO->defgc;
	    break;
	  case GR_NC_default_graphics:
	    noclear = TRUE;
	  case GR_default_graphics:
	    w = DRVINFO->defgw;
	    h = DRVINFO->defgh;
	    c = DRVINFO->defgc;
	    break;
	  case GR_NC_width_height_graphics:
	    noclear = TRUE;
	  case GR_width_height_graphics:
	    w = va_arg(ap,int);
	    h = va_arg(ap,int);
	    c = DRVINFO->defgc;
	    break;
	  case GR_NC_biggest_noninterlaced_graphics:
	    noclear = TRUE;
	  case GR_biggest_noninterlaced_graphics:
	    w = 800;
	    h = 600;
	    c = DRVINFO->defgc;
	    break;
	  case GR_NC_biggest_graphics:
	    noclear = TRUE;
	  case GR_biggest_graphics:
	    w = 4096;
	    h = 4096;
	    c = DRVINFO->defgc;
	    break;
	  case GR_NC_width_height_color_graphics:
	    noclear = TRUE;
	  case GR_width_height_color_graphics:
	    w = va_arg(ap,int);
	    h = va_arg(ap,int);
	    c = va_arg(ap,GrColor);
	    break;
	  case GR_NC_width_height_bpp_graphics:
	    noclear = TRUE;
	  case GR_width_height_bpp_graphics:
	    w  = va_arg(ap,int);
	    h  = va_arg(ap,int);
	    pl = va_arg(ap,int);
	    break;
	  case GR_NC_custom_graphics:
	    noclear = TRUE;
	  case GR_custom_graphics:
	    w  = va_arg(ap,int);
	    h  = va_arg(ap,int);
	    c  = va_arg(ap,GrColor);
	    vw = va_arg(ap,int);
	    vh = va_arg(ap,int);
	    break;
	  case GR_NC_custom_bpp_graphics:
	    noclear = TRUE;
	  case GR_custom_bpp_graphics:
	    w  = va_arg(ap,int);
	    h  = va_arg(ap,int);
	    pl = va_arg(ap,int);
	    vw = va_arg(ap,int);
	    vh = va_arg(ap,int);
	    break;
	  default:
	    va_end(ap);
	    res = errhdlr("unknown video mode");
	    goto done;
	}
	va_end(ap);
	if (c)
	  for(pl = 1; (pl < 32) && ((1UL << pl) < (GrColor)c); pl++) ;
	for( ; ; ) {
	    GrContext     cxt;
	    GrFrameDriver fdr;
	    GrVideoMode  *mdp,vmd;
	    mdp = (DRVINFO->vdriver->selectmode)(DRVINFO->vdriver,w,h,pl,t,NULL);
	    if(!mdp) {
		res = errhdlr("could not find suitable video mode");
		goto done;
	    }
	    sttcopy(&vmd,mdp);
	    if((t || buildframedriver(&vmd,&fdr)) &&
	       (*vmd.extinfo->setup)(&vmd,noclear) &&
	       (t || buildcontext(&vmd,&fdr,&cxt))) {
		if((!t) &&
		   ((vw > vmd.width) || (vh > vmd.height)) &&
		   (vmd.extinfo->setvsize != NULL) &&
		   (vmd.extinfo->scroll   != NULL)) {
		    int ww = vmd.width  = imax(vw,vmd.width);
		    int hh = vmd.height = imax(vh,vmd.height);
		    if(!(*vmd.extinfo->setvsize)(&vmd,ww,hh,&vmd) ||
		       !buildcontext(&vmd,&fdr,&cxt)) {
			sttcopy(&vmd,mdp);
			buildcontext(&vmd,&fdr,&cxt);
			(*vmd.extinfo->setup)(&vmd,noclear);
		    }
		}
		DBGPRINTF(DBG_SETMD,("GrMouseUnInit ...\n"));
		GrMouseUnInit();
		DBGPRINTF(DBG_SETMD,("GrMouseUnInit done\n"));
		DRVINFO->setbank    = (void (*)(int    ))_GrDummyFunction;
		DRVINFO->setrwbanks = (void (*)(int,int))_GrDummyFunction;
		DRVINFO->curbank    = (-1);
		DRVINFO->splitbanks = FALSE;
		if(!t) {
		    if(vmd.extinfo->setbank) {
			DRVINFO->setbank = vmd.extinfo->setbank;
		    }
		    if(vmd.extinfo->setrwbanks) {
			DRVINFO->setrwbanks = vmd.extinfo->setrwbanks;
			DRVINFO->splitbanks = TRUE;
		    }
		    if(umul32(vmd.lineoffset,vmd.height) <= 0x10000L) {
			DRVINFO->splitbanks = TRUE;
		    }
		}
		else {
		    sttzero(&cxt);
		    sttcopy(&fdr,&DRVINFO->tdriver);
		    cxt.gc_driver = &DRVINFO->tdriver;
		}
		sttcopy(&CXTINFO->current,&cxt);
		sttcopy(&CXTINFO->screen, &cxt);
		sttcopy(&DRVINFO->fdriver,&fdr);
		sttcopy(&DRVINFO->sdriver,&fdr);
		sttcopy(&DRVINFO->actmode,&vmd);
		DRVINFO->curmode = mdp;
		DRVINFO->mcode   = which;
		DRVINFO->vposx   = 0;
		DRVINFO->vposy   = 0;
		DBGPRINTF(DBG_SETMD,("GrResetColors ...\n"));
                if ( !_GrResetColors() ) {
                    res = errhdlr("could not set color mode");
                    goto done;
                }
		DBGPRINTF(DBG_SETMD,("GrResetColors done\n"));
		if(fdr.init) {
		    DBGPRINTF(DBG_SETMD,("fdr.init ...\n"));
		    (*fdr.init)(&DRVINFO->actmode);
		    DBGPRINTF(DBG_SETMD,("fdr.init done\n"));
		}
		if(DRVINFO->mdsethook) {
		    DBGPRINTF(DBG_SETMD,("mdsethook ...\n"));
		    (*DRVINFO->mdsethook)();
		    DBGPRINTF(DBG_SETMD,("mdsethook done\n"));
		}
		DBGPRINTF(DBG_SETMD,("GrSetMode complete\n"));
		res = TRUE;
		goto done;
	    }
	    mdp->present = FALSE;
	}
done:   GRX_RETURN(res);
}

