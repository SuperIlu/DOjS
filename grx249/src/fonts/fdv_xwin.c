/**
 ** fdv_xwin.c -- driver X Windows fonts
 **
 ** Author:     Ulrich Leodolter
 ** E-mail:     ulrich@lab1.psy.univie.ac.at
 ** Date:       Thu Sep 28 11:21:46 1995
 ** RCSId:      $Id$
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
 ** Contributions by Dimitar Zhekov (jimmy@is-vn.bg) May 11 2003
 **   - use the default underline height instead of font descent
 **   - use a separate X display and window (how costly is that?)
 **   - use the real font name and font family whenever possible
 **/

#include <stdio.h>
#include <string.h>

#include "libgrx.h"
#include "libxwin.h"
#include <X11/Xatom.h>
#include "grfontdv.h"
#include "allocate.h"
#include "arith.h"

static Display *        fontdsp = NULL;
static Window           fontwin = None;
static XFontStruct *    fontp = NULL;
static Pixmap           fontbmp = None;
static GC               fontgc = None;

static unsigned char    swap_byte[256];
static unsigned char    swap_byte_inited = 0;

static void init_swap_byte (void)
{
  if (!swap_byte_inited) {
    unsigned int i;
    for (i = 0; i < 256; i++) {
      swap_byte[i] = (  ((i&0x01)<<7) | ((i&0x02)<<5)
		      | ((i&0x04)<<3) | ((i&0x08)<<1)
		      | ((i&0x10)>>1) | ((i&0x20)>>3)
		      | ((i&0x40)>>5) | ((i&0x80)>>7));
    }
    swap_byte_inited = 1;
  }
}

static void cleanup(void)
{
  if (fontdsp != NULL) {
    if (fontp != NULL) XFreeFont (fontdsp, fontp);
    if (fontbmp != None) XFreePixmap (fontdsp, fontbmp);
    if (fontgc != None) XFreeGC (fontdsp, fontgc);
    if (fontwin != None) XDestroyWindow (fontdsp, fontwin);
    XCloseDisplay (fontdsp);
  }
  fontdsp = NULL;
  fontwin = None;
  fontp = NULL;
  fontbmp = None;
  fontgc = None;
}

static int openfile(char *fname)
{
  int res;
  Window root;
  int i, numchars;

  res = FALSE;
  init_swap_byte();
  cleanup();
  fontdsp = XOpenDisplay ("");
  if (fontdsp == NULL)  goto done;
  root = DefaultRootWindow (fontdsp);
  fontwin = XCreateSimpleWindow (fontdsp, root, 0, 0, 1, 1, 0, 0, 0);
  if (fontwin == None)  goto done;

  fontp = XLoadQueryFont (fontdsp, fname);
  if (fontp == NULL)    goto done;

  numchars = fontp->max_char_or_byte2 - fontp->min_char_or_byte2 + 1;
  fontbmp = XCreatePixmap (fontdsp, fontwin,
			   numchars * fontp->max_bounds.width,
			   fontp->ascent + fontp->descent, 1);
  if (fontbmp == None)  goto done;
  fontgc = XCreateGC (fontdsp, fontbmp, 0L, NULL);
  if (fontgc == None)   goto done;
  XSetFont (fontdsp, fontgc, fontp->fid);
  XSetForeground (fontdsp, fontgc, 0);
  XFillRectangle (fontdsp, fontbmp, fontgc, 0, 0,
		  numchars * fontp->max_bounds.width,
		  fontp->ascent + fontp->descent);
  XSetForeground (fontdsp, fontgc, 1);
  XSetBackground (fontdsp, fontgc, 0);
  for (i = 0; i < numchars; i++) {
    char c = fontp->min_char_or_byte2 + i;
    XDrawString (fontdsp, fontbmp, fontgc,
		 i * fontp->max_bounds.width,
		 fontp->ascent, &c, 1);
  }
  res = TRUE;
done:
  if (!res) cleanup();
  return(res);
}

static int header(GrFontHeader *hdr)
{
  unsigned long card32;
  char *value;

  if (fontp == NULL) return(FALSE);
  strcpy(hdr->family, "xwin");
  if (XGetFontProperty (fontp, XA_FAMILY_NAME, &card32)) {
    value = XGetAtomName (fontdsp, card32);
    if (value != NULL) {
      strncpy(hdr->family, value, 99);
      hdr->family[99] = '\0';
      XFree (value);
    }
  }
  hdr->proportional = (fontp->per_char == NULL) ? FALSE : TRUE;
  hdr->scalable   = FALSE;
  hdr->preloaded  = FALSE;
  hdr->modified   = GR_FONTCVT_NONE;
  hdr->width      = fontp->max_bounds.width;
  hdr->height     = fontp->ascent + fontp->descent;
  hdr->baseline   = fontp->ascent;
  hdr->ulheight   = imax(1,(hdr->height / 15));
  hdr->ulpos      = hdr->height - hdr->ulheight;
  hdr->minchar    = fontp->min_char_or_byte2;
  hdr->numchars   = fontp->max_char_or_byte2 - fontp->min_char_or_byte2 + 1;
  strncpy(hdr->name, hdr->family, 89);
  hdr->name[89] = '\0';
  sprintf(hdr->name + strlen(hdr->name), "-%d", hdr->height);
  if (XGetFontProperty (fontp, XA_FONT_NAME, &card32)) {
    value = XGetAtomName (fontdsp, card32);
    if (value != NULL) {
      strncpy(hdr->name, value, 99);
      hdr->name[99] = '\0';
      XFree (value);
    }
  }
  return(TRUE);
}

static int charwdt(int chr)
{
  int width;
  if (fontp == NULL)    return(-1);
  if (chr < fontp->min_char_or_byte2)   return(-1);
  if (chr > fontp->max_char_or_byte2)   return(-1);
  if (fontp->per_char == NULL)          return(fontp->max_bounds.width);
  width = fontp->per_char[chr - fontp->min_char_or_byte2].width;
  if (width <= 0)
    return fontp->per_char[fontp->default_char - fontp->min_char_or_byte2].width;
  return(width);
}

static int bitmap(int chr,int w,int h,char *buffer)
{
  XImage *img;
  int x, y, bpl;
  unsigned char *data;

  if (fontp == NULL || fontbmp == None) return(FALSE);
  if ((w <= 0) || (w != charwdt(chr)))  return(FALSE);
  if ((h <= 0) || (h != (fontp->ascent + fontp->descent)))      return(FALSE);
  if (fontdsp == NULL)  return(FALSE);
  img = XGetImage (fontdsp,
		   fontbmp,
		   (chr - fontp->min_char_or_byte2) * fontp->max_bounds.width,
		   0,
		   w,
		   h,
		   AllPlanes,
		   ZPixmap);
  if (img == NULL)      return(FALSE);
  data = (unsigned char *)(img->data);
  bpl = (w + 7) >> 3;
  for (y = 0; y < h; y++) {
    for (x = 0; x < bpl; x++) buffer[x] = swap_byte[data[x]];
    buffer += bpl;
    data += img->bytes_per_line;
  }
  XDestroyImage (img);
  return(TRUE);
}

GrFontDriver _GrFontDriverXWIN = {
    "XWIN",                             /* driver name (doc only) */
    "",                                 /* font file extension */
    FALSE,                              /* scalable */
    openfile,                           /* file open and check routine */
    header,                             /* font header reader routine */
    charwdt,                            /* character width reader routine */
    bitmap,                             /* character bitmap reader routine */
    cleanup                             /* cleanup routine */
};

