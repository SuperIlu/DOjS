/**
 ** fontdemo.c ---- demonstrate a font
 **
 ** Copyright (C) 2002 Dimitar Zhekov
 ** E-Mail: jimmy@is-vn.bg
 **
 ** This is a test/demo file of the GRX graphics library.
 ** You can use GRX test/demo files as you want.
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
#include <stdlib.h>
#include <string.h>

#include "grx20.h"
#include "grxkeys.h"

static GrTextOption opt;
static int curx = 0, cury = 0;
/* deltax and deltay are the additional columns/lines between characters */
static int deltax = 0, deltay = 0;

static void gnewl(void)
{
	cury += GrCharHeight('A', &opt) + deltay;
	curx = 0;
	if(cury + GrCharHeight('A', &opt) > GrSizeY() + deltay) {
	    if(GrKeyRead() == GrKey_F10) {
		GrUnloadFont(opt.txo_font);
		exit(0);
	    }
	    GrClearScreen(opt.txo_bgcolor.v);
	    cury = 0;
	}
}

/* all control characters are displayed 1:1 */
static void gputc(int c)
{
	if(curx + GrCharWidth(c, &opt) + deltax > GrSizeX()) gnewl();
	GrDrawChar(c, curx, cury, &opt);
	curx += GrCharWidth(c, &opt) + deltax;
}

static void gputs(const char *s)
{
	while(*s != '\0') gputc((unsigned char) *s++);
	gnewl();
}

static void revert(void)
{
	GrColor color;

	color = opt.txo_fgcolor.v;
	opt.txo_fgcolor.v = opt.txo_bgcolor.v | (color & GR_UNDERLINE_TEXT);
	opt.txo_bgcolor.v = color & ~GR_UNDERLINE_TEXT;
	GrClearScreen(color);
	curx = cury = 0;
}

int main(int argc, char **argv)
{
	int i, n;
	char *s;
	char *bad = NULL;
	int c;

	int width = 0, height = 0, bpp = 8, gray = 192, attributes = 0;

	char *name, *testname;
	GrFontHeader *hdr;
	FILE *f;
	char buffer[0x20];
	GrKeyType key;

	/* unfortunately not all systems support getopt() */
	for(i = 1; i < argc; i++) {
	    s = argv[i];
	    if(*s != '-' || ((c = *++s) == '\0') || *++s != '\0') break;
	    if(c == '-') {
		i++;
		break;
	    }
	    if(++i == argc) {
		fprintf(stderr, "-%c: argument required\n", c);
		return(1);
	    }
	    if(sscanf(argv[i], "%d", &n) != 1 || n < 0) {
		fprintf(stderr, "%s: invalid argument\n", argv[i]);
		exit(1);
	    }
	    switch(c) {
		case 'x' : width = n; break;
		case 'y' : height = n; break;
		case 'b' : if((bpp = n) < 2 || bpp > 32) bad = "bpp"; break;
		case 'g' : if((gray = n) > 255) bad = "gray"; break;
		case 'X' : if((deltax = n) > 31) bad = "deltax"; break;
		case 'Y' : if((deltay = n) > 31) bad = "deltay"; break;
		case 'a' : if((attributes = n) > 3) bad = "attributes"; break;
		default  : {
		    fprintf(stderr, "-%c: invalid option\n", c);
		    return(1);
		}
	    }
	    if(bad) {
		fprintf(stderr, "%d: %s out of range\n", n, bad);
		return(1);
	    }
	}

	if(i == argc) {
	    printf(
		"usage:\tfontdemo [-x WIDTH] [-y HEIGHT] [-b BPP] [-g COMPONENT]\n"
		      "\t[-X DELTAX] [-Y DELTAY] [-a ATTRIBUTES] FONT [FILE...]\n"
	    );
	    return(0);
	}

	name = argv[i++];
	opt.txo_font = GrLoadFont(name);
        if(opt.txo_font == NULL && (testname = malloc(strlen(name) + 10)) != NULL) {
            /* try again, this is a test and the path can not been set yet */
#if defined(__MSDOS__) || defined(__WIN32__)
            sprintf( testname,"..\\fonts\\%s",name );
#else
            sprintf( testname,"../fonts/%s",name );
#endif
            opt.txo_font = GrLoadFont(testname);
	    free(testname);
        }
	if(opt.txo_font == NULL) {
	    fprintf(stderr, "%s: load error\n", name);
	    return(1);
	}
	hdr = &opt.txo_font->h;

	if(height == 0) {
	    if(width == 0) {
		switch(hdr->height) {
		    case 8 :
		    case 14 : height = 400; break;
		    case 24 : height = 768; break;
		    default : height = hdr->height * 30;
		}
	    }
	    else height = width * 3 / 4;
	}
	if(width == 0) width = height == 400 ? 640 : height * 4 / 3;

	GrSetMode(GR_width_height_bpp_graphics, width, height, bpp);
	if(!gray || (opt.txo_fgcolor.v = GrAllocColor(gray, gray, gray)) == GrNOCOLOR) opt.txo_fgcolor.v = GrWhite();
	if(attributes & 0x02) opt.txo_fgcolor.v |= GR_UNDERLINE_TEXT;
	opt.txo_bgcolor.v = GrBlack();
	if(attributes & 0x01) revert();
	opt.txo_chrtype = GR_BYTE_TEXT;
	opt.txo_direct = GR_TEXT_RIGHT;
	opt.txo_xalign = GR_ALIGN_LEFT;
	opt.txo_yalign = GR_ALIGN_TOP;

	sprintf(buffer, "%s %dx%d", hdr->name, GrCharWidth('A', &opt), GrCharHeight('A', &opt));
	gputs(buffer);
	sprintf(buffer, "%dx%d@%lu", GrSizeX(), GrSizeY(), (unsigned long) GrNumColors());
	gputs(buffer);
	gnewl();

	gputs("THE QUICK BROWN FOX JUMPS OVER THE LAZY DOG");
	gputs("the quick brown fox jumps over the lazy dog");
	gnewl();

	if(hdr->minchar <= 0xC0 && hdr->minchar + hdr->numchars >= 0x100) {
	    gputs("БЪРЗАТА КАФЯВА ЛИСИЦА ПРЕСКАЧА ЛЕНИВОТО КУЧЕ");
	    gputs("бързата кафява лисица прескача ленивото куче");
	    gnewl();
	}

	/* ascii table, or to be precise, a full table of the current font */
	opt.txo_chrtype = GR_WORD_TEXT;
	for(c = 0; c < hdr->numchars; c++) {
	    gputc(hdr->minchar + c);
	    if(c % 0x20 == 0x1F) gnewl();
	}
	gnewl();
	if(c % 0x20 != 0) gnewl();
	opt.txo_chrtype = GR_BYTE_TEXT;

	while(i < argc) {
	    name = argv[i++];
	    if((f = fopen(name, "r")) == NULL) {
		perror(name);
		return(1);
	    }
	    while((c = getc(f)) != EOF) if(c != '\n') gputc(c); else gnewl();
	    if(ferror(f) != 0 || fclose(f) != 0) {
		perror(name);
		return(1);
	    }
	}

	/* enter and esc are < 0x100 and displayed 1:1 */
	gputs("F1-new line  F5-toggle reverse  F7-toggle underline  F10-exit");
	gnewl();

	while((key = GrKeyRead()) != GrKey_F10) {
	    if(key == GrKey_F1) gnewl();
	    else if(key == GrKey_F5) revert();
	    else if(key == GrKey_F7) opt.txo_fgcolor.v ^= GR_UNDERLINE_TEXT;
	    else if(key < 0x100) gputc(key);
	}

	GrUnloadFont(opt.txo_font);

	return(0);
}
