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

#if 0 && ((GRX_VERSION_API-0)>=0x0225)
/* GRX has floodfill */

void floodfill(int x, int y, int border)
{
  _DO_INIT_CHECK;
  x += VL;
  y += VT+PY;

  if ( FPATT == SOLID_FILL || FPATT == EMPTY_FILL) {
    GrColor color = (FPATT==SOLID_FILL) ? FILL : COLBG;
    GrFloodFill( x, y, (GrColor)border, color);
  } else {
    FILLP.gp_bmp_fgcolor = FILL;
    FILLP.gp_bmp_bgcolor = COLBG;
    GrPatternFloodFill( x, y, (GrColor)border, &FILLP);
  }
}

#else
/* use BCC2GRX flood fill function */

#include <setjmp.h>

static int lx, ly, mx, my;
static GrColor _border;
static GrColor _color;
static jmp_buf error;

typedef unsigned char element;       /* for 1bit/pixel images */
typedef unsigned short line_index;   /* start index table */
static element **done  = NULL;       /* bitmap of already processed pixel */
static element **start = NULL;       /* pixel that need to be processed */
static int elements;                 /* no. bytes in each bitmap line */
static line_index *start_flg = NULL; /* !=0: index+1 of first start element !=0 */
				     /* ==0: nothing to do */

#define bits_per_element  (sizeof(element)*8)
#define offset_div        (bits_per_element)
#define calc_bit(x)       ( ((element)1) << ((x)&(bits_per_element-1)))
#define calc_high_bits(x) ( (~((element)0)) << ((x)&(bits_per_element-1)))
#define calc_ofs(x)       ((x) / offset_div)

/* ----------------------------------------------------------------- */
#if defined(__GNUC__)
#define _BGI_INLINE_	__inline__
#elif defined(_MSC_VER)
#define _BGI_INLINE_	_inline
#else
#define _BGI_INLINE_
#endif
/* ----------------------------------------------------------------- */

/* -------- internal line buffer functions
**
**  (x,y) scaled to (0..mx,0..my)
*/

static _BGI_INLINE_ element *generate_line(element **buf, int y) {
  if (buf[y] == NULL)
    if ( (buf[y] = calloc(sizeof(element),elements)) == NULL)
      longjmp(error,1);
  return buf[y];
}

static _BGI_INLINE_ void mark_line( element **buf, int x1, int x2, int y) {
  element *l = generate_line(buf,y);
  element *anf, *ende;
  int start_bit, stop_bit;

  anf  = &l[calc_ofs(x1)];
  ende = &l[calc_ofs(x2)];
  start_bit = calc_high_bits(x1);
  if (anf != ende) {
    *(anf++) |= start_bit;
    if (anf != ende)
      memset(anf,~0,(ende-anf)*sizeof(element));
    start_bit = ~0;
  }
  /* start_bit rejects all invalid low bits, let stop_bit discard
     all invalid high bits, but make sure stop_bit won't get zero */
  stop_bit = ~calc_high_bits(x2+1);
  if ( stop_bit )
    start_bit &= stop_bit;
  *ende |= start_bit;
}

static _BGI_INLINE_ void set_pix(element **buf, int x, int y) {
  element *l = generate_line(buf,y);
  l[calc_ofs(x)] |= calc_bit(x);
}

static _BGI_INLINE_ int test_pix(element **buf, int x, int y) {
  element *l = buf[y];
  if (l != NULL)
    return (l[calc_ofs(x)] & calc_bit(x)) != 0;
  return FALSE;
}

static _BGI_INLINE_ int test_screen(int x, int y) {
  return (GrPixelNC(x+lx,y+ly) == _border);
}

static _BGI_INLINE_ int test_pixel(int x, int y) {
  if (test_pix(done,x,y)) return TRUE;
  if (test_screen(x,y)) {
    set_pix(done,x,y);
    return TRUE;
  }
  return FALSE;
}

static _BGI_INLINE_ void SetStartFlag(int x, int y) {
    int _x = calc_ofs(x);
    if (   !start_flg[y]
	|| _x<start_flg[y] ) start_flg[y] = _x+1;
}

/* -------- screen & buffer fill functions
**
**  (x,y) scaled to (0..mx,0..my)
**  (0,0) maps to (lx,ly) on screen
*/

static void solid_fill(int x, int y) {
  int sx;

  sx = x;
  while ( sx > 0 && !test_pixel(sx-1,y))
    --sx;
  while ( x < mx && !test_pixel(x+1,y))
    ++x;
  GrHLine( sx+lx, x+lx, y+ly, _color);
  mark_line( done, sx, x, y);
  if (y>0)  { mark_line( start, sx, x, y-1);
	      SetStartFlag(sx,y-1);          }
  if (y<my) { mark_line( start, sx, x, y+1);
	      SetStartFlag(sx,y+1);          }
}

static void pattern_fill(int x, int y) {
  int sx;

  sx = x;
  while ( sx > 0 && !test_pixel(sx-1,y))
    --sx;
  while ( x < mx && !test_pixel(x+1,y))
    ++x;
  GrPatternFilledLine( sx+lx, y+ly, x+lx, y+ly, &FILLP);
  mark_line( done, sx, x, y);
  if (y>0)  { mark_line( start, sx, x, y-1);
	      SetStartFlag(sx,y-1);          }
  if (y<my) { mark_line( start, sx, x, y+1);
	      SetStartFlag(sx,y+1);          }
}

typedef void (*fill_func)(int x, int y);

/* -------- find fill area
**
**  (x,y) scaled to (0..mx,0..my)
*/

static void work(fill_func fill) {
  int top;
  element *s, *d;
  int rescan, dir = 1;

  top = 0;
  do {
    rescan = 0;
    while (0 <= top && top <= my) {
ThisLine:
      if ( (s=start[top]) != NULL && start_flg[top]) {
	int i;
	if ( (d=done[top]) != NULL) {
	  i= start_flg[top]-1;
	  s += i; d += i;
	  for ( ; i < elements ; ++i)
	     *(s++) &= ~*(d++);
	  s = start[top];
	}
	i=start_flg[top]-1; start_flg[top] = 0;
	for ( s += i; i < elements ; ++i, ++s)
	  if (*s) {
	    element b = 1;
	    int x = i*offset_div;
	    do {
	      while ( !(*s&b)) { ++x; b <<= 1; }
	      if (test_screen(x,top)) {
		set_pix(done,x,top);
		*s ^= b;
	      } else {
		SetStartFlag(x,top);
		fill(x,top);
		rescan = 1;
		goto ThisLine;
	      }
	    } while (*s);
	  }
      }
      top += dir;
    }
    dir = -dir;
    top += dir;
  } while (rescan);
}

void floodfill(int x, int y, int border)
{
  int _x, _y;

  _DO_INIT_CHECK;
  if (__gr_clip) {
    lx = VL; ly = VT+PY;
    mx = VR; my = VB+PY;
  } else {
    lx = 0; ly = 0+PY;
    mx = getmaxx();
    my = getmaxy()+PY;
  }
  x += VL;
  y += VT+PY;
  _border = border;
  if ( x < lx || y < ly || x > mx || y > my || GrPixel(x,y) == _border)
    return;

  mx -= lx; _x = x - lx;
  my -= ly; _y = y - ly;
  done      = calloc(sizeof(element *),  my+1);
  start     = calloc(sizeof(element *),  my+1);
  start_flg = calloc(sizeof(line_index), my+1);
  if (done==NULL || start==NULL || start_flg==NULL) {
    ERR = grNoFloodMem;
    goto FreeMem;
  }

  if (setjmp(error) == 0) {
    elements = calc_ofs(mx + bits_per_element) + 1;

    set_pix(start, _x, _y);
    SetStartFlag(_x,_y);
    if ( FPATT == SOLID_FILL || FPATT == EMPTY_FILL) {
      _color = (FPATT==SOLID_FILL) ? FILL : COLBG;
      work(solid_fill);
    } else {
      FILLP.gp_bmp_fgcolor = FILL;
      FILLP.gp_bmp_bgcolor = COLBG;
      work(pattern_fill);
    }
  } else {
    /* generate_line() called longjmp() : out of memory error */
    ERR = grNoFloodMem;
  }

FreeMem:
  if (done != NULL) {
    int i;
    for (i=my; i >= 0; --i)
      if (done[i] != NULL)
	free(done[i]);
    free(done);
    done = NULL;
  }
  if (start != NULL) {
    int i;
    for (i=my; i >= 0; --i)
      if (start[i] != NULL)
	free(start[i]);
    free(start);
    start = NULL;
  }
  if (start_flg != NULL) {
    free(start_flg);
    start_flg = NULL;
  }
}

#endif
