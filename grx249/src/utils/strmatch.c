/**
 ** strmatch.c ---- a string matcher. Similar to UNIX filename matching
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
#include "libgrx.h"

int GrMatchString(const char *pat,const char *str)
{
	int i,n,escape = FALSE;
	union {
	    struct { const char *s; int l; } str[20];
	    struct { unsigned char loc; unsigned char hic; } chr[50];
	} m;
	for( ; ; ) {
	    if(*pat == '\0') {
		return((*str == '\0') ? TRUE : FALSE);
	    }
	    if(!escape) switch(*pat) {
	      case '\\':
		escape = TRUE;
		pat++;
		continue;
	      case '?':
		if(*str == '\0') return(FALSE);
		pat++;
		str++;
		continue;
	      case '*':
		if(*(++pat) == '\0') {
		    return(TRUE);
		}
		while(*str != '\0') {
		    if(GrMatchString(pat,str)) return(TRUE);
		    str++;
		}
		return(FALSE);
	      case '[':
		for(i = FALSE, n = 0; ; ) {
		    switch(*(++pat)) {
		      case '\0':
			return(FALSE);
		      case '\\':
			escape = TRUE;
			continue;
		      case ']':
			if(!escape) {
			    pat++;
			    break;
			}
		      case '-':
			if(!escape) {
			    i = TRUE;
			    continue;
			}
		      default:
			if(i && (n > 0)) {
			    m.chr[n - 1].hic = *pat;
			}
			else {
			    m.chr[n].loc = m.chr[n].hic = *pat;
			    n++;
			}
			i = escape = FALSE;
			continue;
		    }
		    break;
		}
		for(i = 0; ; i++) {
		    if(i == n) return(FALSE);
		    if((unsigned char)(*str) < m.chr[i].loc) continue;
		    if((unsigned char)(*str) > m.chr[i].hic) continue;
		    str++;
		    break;
		}
		continue;
	      case '{':
		for(m.str[n = i = 0].s = pat + 1; ; ) {
		    switch(*(++pat)) {
		      case '\0':
			return(FALSE);
		      case '\\':
			escape = TRUE;
			continue;
		      case '}':
			if(!escape) {
			    m.str[n++].l = i;
			    pat++;
			    break;
			}
		      case ',':
			if(!escape) {
			    m.str[n++].l = i;
			    m.str[n  ].s = pat + 1;
			    i = 0;
			    continue;
			}
		      default:
			i++;
			continue;
		    }
		    break;
		}
		for(i = 0; i < n; i++) {
		    if(strncmp(str,m.str[i].s,(size_t)m.str[i].l)!=0) continue;
		    if(GrMatchString(pat,&str[m.str[i].l]) == 0) continue;
		    return(TRUE);
		}
		return(FALSE);
	    }
	    if(*pat != *str) return(FALSE);
	    escape = FALSE;
	    pat++;
	    str++;
	}
}

