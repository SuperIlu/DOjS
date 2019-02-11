/**
 ** fontpath.c ---- set up the font search path
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
#include <ctype.h>

#include "libgrx.h"
#include "grfontdv.h"
#include "allocate.h"
#include "arith.h"

void GrSetFontPath(char *p)
{
	int  chr,totlen = 0,npath,plen = 0,dc = TRUE;
	char path[200],*plist[100];
	if(!p || (*p == '\0')) return;
        for (npath = 0; npath < itemsof(plist); ++npath)
          plist[npath] = NULL;
        npath = 0;
	setup_ALLOC();
	path[0] = '\0';
	while(((chr = *p++) != '\0') || (plen > 0)) {
	    int pathchr = TRUE;
	    switch(chr) {
	      case ':':
#if defined(__MSDOS__) || defined(__WIN32__)
		if((plen == 1) && isalpha(path[0])) break;
#endif
	      case ';':
		pathchr = FALSE;
		break;
	      case '\0':
		p--;
		pathchr = FALSE;
		break;
#if defined(__MSDOS__) || defined(__WIN32__)
	      case '\\':
		chr = '/';
	      case '/':
	        dc = TRUE;
		break;
#endif
	      default:
#ifdef __DJGPP__
                /* allow syntax /dev/env/DJDIR */
		if ((plen == 9)	&& (strncmp(path,"/dev/env/",9)==0))
		    dc = FALSE;
		if (dc)
#endif
#if defined(__MSDOS__) || defined(__WIN32__)
		    chr = tolower(chr);
#endif
		if(isspace(chr)) pathchr = FALSE;
		break;
	    }
	    if(pathchr) {
		path[plen++] = chr;
		continue;
	    }
	    if(plen > 0) {
		if(path[plen - 1] != '/') path[plen++] = '/';
		path[plen++] = '\0';
		plist[npath] = ALLOC((size_t)plen);
		if(plist[npath] == NULL) goto error;
		strcpy(plist[npath],path);
		totlen += plen;
		plen = 0;
		if(++npath == itemsof(plist)) break;
	    }
	}
	if(_GrFontFileInfo.path != NULL) free(_GrFontFileInfo.path);
	_GrFontFileInfo.path  = NULL;
	_GrFontFileInfo.npath = npath;
	if(npath > 0) {
	    _GrFontFileInfo.path = malloc((sizeof(char *) * npath) + totlen);
	    if(_GrFontFileInfo.path == NULL) goto error;
	    p = (char *)(&_GrFontFileInfo.path[npath]);
	    for(plen = 0; plen < npath; plen++) {
		_GrFontFileInfo.path[plen] = p;
		strcpy(p,plist[plen]);
		p += strlen(p) + 1;
	    }
	}
	goto done;
      error:
	if(_GrFontFileInfo.path != NULL) free(_GrFontFileInfo.path);
	_GrFontFileInfo.path  = NULL;
	_GrFontFileInfo.npath = 0;
      done:
        for (npath = 0; npath < itemsof(plist); ++npath)
          FREE(plist[npath]);
	reset_ALLOC();
}
