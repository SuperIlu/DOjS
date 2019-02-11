/**
 ** loadfont.c ---- load a font from a disk file
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

#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#include "libgrx.h"
#include "grfontdv.h"

static GrFont *doit(char *fname,char *path,int cvt,int w,int h,int lo,int hi)
{
    GrFontDriver **fd;
    GrFontHeader hdr;
    GrFont *f, *res;
    char pathname[200];
    char tempstring[200];
    int  plen;
    GRX_ENTER();
    res = NULL;
    strcpy(pathname,path);
    strcat(pathname,fname);
    DBGPRINTF(DBG_FONT,("searching font %s\n", pathname));
    plen = strlen(pathname);
    hdr.name   = &tempstring[0];
    hdr.family = &tempstring[100];
    for(fd = _GrFontDriverTable; (*fd) != NULL; fd++) {
	DBGPRINTF(DBG_FONT,("Driver \"%s\", extension \"%s\"\n", (*fd)->name, (*fd)->ext));
	pathname[plen] = '\0';
	if(!((*fd)->openfile)(pathname)) {
	    strcpy(&pathname[plen],(*fd)->ext);
	    if(!((*fd)->openfile)(pathname)) continue;
	}
	if(!((*fd)->header)(&hdr)) {
	    DBGPRINTF(DBG_FONT,("fd->header failed for %s\n", pathname));
	    (*fd)->cleanup();
	    continue;
	}
	f = _GrBuildFont(
	    &hdr,
	    cvt,
	    w,h,
	    lo,hi,
	    (*fd)->charwdt,
	    (*fd)->bitmap,
	    (*fd)->scalable
	);
	if(!f) {
	    DBGPRINTF(DBG_FONT,("_GrBuildFont failed for %s\n", pathname));
	    (*fd)->cleanup();
	    continue;
	}
        (*fd)->cleanup();
	res = f;
	break;
    }
    GRX_RETURN(res);
}

GrFont *GrLoadConvertedFont(char *name,int cvt,int w,int h,int minc,int maxc)
{
    GrFont *f;
    int  chr,len,abspath,dc;
    char fname[200];
    GRX_ENTER();
    len = 0;
    abspath = FALSE;
    dc = TRUE;
    while((chr = *name++) != '\0') {
	switch(chr) {
#if defined(__MSDOS__) || defined(__WIN32__)
	  case ':':
	    abspath = TRUE;
	    break;
	  case '\\':
	    chr = '/';
#endif
	  case '/':
	    dc = TRUE;
	    if(len == 0) abspath = TRUE;
	    break;
	  default:
	    if(isspace(chr)) {
		if(len == 0) continue;
		name = "";
		chr  = '\0';
	    }
#ifdef __DJGPP__
            /* allow syntax /dev/env/DJDIR */
	    if ((len == 9) && (strncmp(fname,"/dev/env/",9)==0))
		dc = FALSE;
	    if (dc)
#endif
#if defined(__MSDOS__) || defined(__WIN32__)
		chr = tolower(chr);
#endif
	    break;
	}
	fname[len++] = chr;
    }
    fname[len] = '\0';
    f = doit(fname,"",cvt,w,h,minc,maxc);
    if((f == NULL) && !abspath) {
	if(_GrFontFileInfo.npath < 0) {
            char *fPath = getenv("GRXFONT");
#ifdef GRX_DEFAULT_FONT_PATH
            if (!fPath) fPath = GRX_DEFAULT_FONT_PATH;
#endif            
	    GrSetFontPath(fPath);
	}
	for(len = 0; len < _GrFontFileInfo.npath; len++) {
	    f = doit(fname,_GrFontFileInfo.path[len],cvt,w,h,minc,maxc);
	    if(f != NULL) break;
	}
    }
    GRX_RETURN(f);
}

GrFont *GrLoadFont(char *name)
{
    return(GrLoadConvertedFont(name,GR_FONTCVT_NONE,0,0,0,0));
}

