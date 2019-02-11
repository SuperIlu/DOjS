/**
 ** dumpfont.c ---- write a C source file from a font in memory
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
#include <stdio.h>

#include "libgrx.h"

/* ----------------------------------------------------------------------- */
static char bitmaphdr[] =

"/**\n"
" ** %s ---- GRX 2.0 font converted to C by 'GrDumpFont()'\n"
" **/\n"
"\n"
"#define  %s     FONTNAME_TEMPORARY_REDIRECTION\n"
"#include \"grx20.h\"\n"
"#undef   %s\n"
"\n"
"static unsigned char far %s[] = {\n";

/* ----------------------------------------------------------------------- */
static char fonthdr[] =

"};\n"
"\n"
"struct {\n"
"    GrFont        theFont;\n"
"    GrFontChrInfo rest[%d];\n"
"} %s = {\n"
"    {\n"
"        {                           /* font header */\n"
"            %-24s"                 "/* font name */\n"
"            %-24s"                 "/* font family name */\n"
"            %d,  \t\t    "         "/* characters have varying width */\n"
"            0,                      /* derived from a scalable font */\n"
"            1,                      /* font permanently linked into program */\n"
"            GR_FONTCVT_NONE,        /* 'tweaked' font (resized, etc..) */\n"
"            %d,  \t\t    "         "/* width (average when proportional) */\n"
"            %d,  \t\t    "         "/* font height */\n"
"            %d,  \t\t    "         "/* baseline pixel pos (from top) */\n"
"            %d,  \t\t    "         "/* underline pixel pos (from top) */\n"
"            %d,  \t\t    "         "/* underline width */\n"
"            %d,  \t\t    "         "/* lowest character code in font */\n"
"            %d   \t\t    "         "/* number of characters in font */\n"
"        },\n"
"        (char *)%-20s"             "/* character bitmap array */\n"
"        0,                          /* auxiliary bitmap */\n"
"        %d,\t\t\t    "             "/* width of narrowest character */\n"
"        %d,\t\t\t    "             "/* width of widest character */\n"
"        0,                          /* allocated size of auxiliary bitmap */\n"
"        0,                          /* free space in auxiliary bitmap */\n"
"        {  0\t\t"      "},          /* converted character bitmap offsets */\n"
"        {{ %d,\t0\t"   "}}          /* first character info */\n"
"    },\n"
"    {\n";

/* ----------------------------------------------------------------------- */
static char charinfo[] =

"        {  %d,\t%d\t"  "}%c         /* info for character %-3d */\n";

/* ----------------------------------------------------------------------- */
static char fontend[] =

"    }\n"
"};\n\n";

/* ----------------------------------------------------------------------- */

void GrDumpFont(const GrFont *f,char *CsymbolName,char *fileName)
{
	unsigned int i;
	int  offset;
	char filname[200];
	char fntname[200];
	char famname[200];
	char bitname[200];
	char *p;
	FILE *fp = fopen(fileName,"w");
	if(!fp) return;
	strcpy(filname,fileName);
	for(p = filname; *p; p++) *p = toupper(*p);
	sprintf(fntname,"\"%s",f->h.name);
	if((p = strrchr(fntname,'.')) != 0) *p = '\0';
	strcat(fntname,"\",");
	sprintf(famname,"\"%s\",",f->h.family);
	sprintf(bitname,"%s_bits",CsymbolName);
	fprintf(
	    fp,
	    bitmaphdr,
	    filname,
	    CsymbolName,
	    CsymbolName,
	    bitname
	);
	for(i = 0; i < f->h.numchars; i++) {
	    int  chr = i + f->h.minchar;
	    int  len = GrFontCharBitmapSize(f,chr);
	    int  pos = 0,j;
	    char far *bmp = GrFontCharBitmap(f,chr);
	    fprintf(fp,"\t/* character %d */\n\t",chr);
	    for(j = 0; j < len; j++) {
		fprintf(fp,"0x%02x",(bmp[j] & 0xff));
		if((j + 1) != len) {
		    putc(',',fp);
		    if(++pos != 12) continue;
		    fputs("\n\t",fp);
		    pos = 0;
		}
	    }
	    if((i + 1) != f->h.numchars) {
		fputs(",\n",fp);
	    }
	}
	fprintf(fp,
	    fonthdr,
	    f->h.numchars - 1,
	    CsymbolName,
	    fntname,
	    famname,
	    f->h.proportional,
	    f->h.width,
	    f->h.height,
	    f->h.baseline,
	    f->h.ulpos,
	    f->h.ulheight,
	    f->h.minchar,
	    f->h.numchars,
	    (strcat(bitname,","),bitname),
	    f->minwidth,
	    f->maxwidth,
	    GrFontCharWidth(f,f->h.minchar)
	);
	offset = GrFontCharBitmapSize(f,f->h.minchar);
	for(i = 1; i < f->h.numchars; i++) {
	    int chr = i + f->h.minchar;
	    fprintf(
		fp,
		charinfo,
		GrFontCharWidth(f,chr),
		offset,
		((i == (f->h.numchars - 1)) ? ' ' : ','),
		chr
	    );
	    offset += GrFontCharBitmapSize(f,chr);
	}
	fputs(fontend,fp);
	fclose(fp);
}

