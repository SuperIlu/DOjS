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

#define __BCCGRX_C

#include "bccgrx00.h"

#define MAX_MODES 256

/* ----------------------------------------------------------------- */

static char copyright[]="Copyright (C) 1993-1994  Hartmut Schirmer";
int           __gr_Mode = 0;                    /* actual graphics mode     */
int           __gr_INIT = FALSE;                /* TRUE after initgraph()   */
char          __gr_BGICHR[128];                 /* Path to .chr files       */
int           __gr_MaxMode = 0;                 /* Last available mode      */
int           __gr_Result = grOk;               /* stores error code        */
int           __gr_X, __gr_Y;                   /* graphics cursor pos      */
int           __gr_vpl, __gr_vpt,               /* actual viewport          */
	      __gr_vpr, __gr_vpb;
int           __gr_color;                       /* drawing color            */
int           __gr_colorbg;                     /* background color         */
int           __gr_colorfill;                   /* fill color               */
GrColor       __gr_WR = GrWRITE;                /* Write mode               */
int           __gr_lstyle = SOLID_LINE;         /* Actual line style        */
int           __gr_fpatno = SOLID_FILL;         /* Actual filling pattern   */
int           __gr_Xasp = 10000;                /* Aspect ratio             */
int           __gr_Yasp = 10000;
int           __gr_clip = TRUE;                 /* actual clipping state    */

int           __gr_Y_page_offs = 0;             /* Y offset for page simulation */

int           __gr_ADAPTER = GR_VGA;            /* Adapter used             */
int           __gr_TextLineStyle = 0;           /* use setlinestyle() while
						   plotting .chr fonts      */

int          (*__gr_initgraph_hook)(void)=NULL;  /* hook for overriding mode */
                                                /* setting in initgraph     */
int          (*__gr_closegraph_hook)(void)=NULL;

GrPattern      __gr_fillpattern;                /* GRX filling settings     */
GrLineOption   __gr_Line;                       /* GRX line settings        */

unsigned char __gr_fpatterns[][8] = {     /* BGI fill patterns        */
  { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   /* EMPTY_FILL        */
  { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},   /* SOLID_FILL        */
  { 0xFF, 0xFF, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00},   /* LINE_FILL         */
  { 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80},   /* LTSLASH_FILL      */
  { 0xE0, 0xC1, 0x83, 0x07, 0x0E, 0x1C, 0x38, 0x70},   /* SLASH_FILL        */
  { 0xF0, 0x78, 0x3C, 0x1E, 0x0F, 0x87, 0xC3, 0xE1},   /* BKSLASH_FILL      */
  { 0xA5, 0xD2, 0x69, 0xB4, 0x5A, 0x2D, 0x96, 0x4B},   /* LTBKSLASH_FILL    */
  { 0xFF, 0x88, 0x88, 0x88, 0xFF, 0x88, 0x88, 0x88},   /* HATCH_FILL        */
  { 0x81, 0x42, 0x24, 0x18, 0x18, 0x24, 0x42, 0x81},   /* XHATCH_FILL       */
  { 0xCC, 0x33, 0xCC, 0x33, 0xCC, 0x33, 0xCC, 0x33},   /* INTERLEAVE_FILL   */
  { 0x80, 0x00, 0x08, 0x00, 0x80, 0x00, 0x08, 0x00},   /* WIDE_DOT_FILL     */
  { 0x88, 0x00, 0x22, 0x00, 0x88, 0x00, 0x22, 0x00},   /* CLOSE_DOT_FILL    */
  { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}    /* USER_FILL         */
};

#ifdef GRX_VERSION
int __gr_BGI_p = 1;   /* requested / available # of pages  (valid: 1,2) */
#endif
int __gr_BGI_w = 640; /* resulution and colors needed to emulate        */
int __gr_BGI_h = 480; /* BGI driver modes                               */
int __gr_BGI_c = 16;  /* default : Standard VGA                         */


#ifdef __GNUC__
#define NULL_IS_EMPTY(s) ((s) ? : "")
#else
#define NULL_IS_EMPTY(s) ((s) ? (s) : "")
#endif

/* ----------------------------------------------------------------- */

GraphicsMode *__gr_Modes = NULL;

static int ModeCompare(GraphicsMode *a, GraphicsMode *b) {
  int size_a, size_b;
  if (a->colors < b->colors) return -1;
  if (a->colors > b->colors) return +1;
  size_a = a->width * a->height;
  size_b = b->width * b->height;
  if (size_a < size_b) return -1;
  if (size_a > size_b) return +1;
  if (a->width < b->width) return -1;
  if (a->width > b->width) return +1;
  return 0;
}

static void NewMode(int w, int h, long c) {
  GraphicsMode *p, *q, *qn;
  p = malloc(sizeof(GraphicsMode));
  if (p != NULL) {
    p->width = w;
    p->height = h;
    p->colors = c;
    p->next = NULL;
    if (__gr_Modes == NULL) {
      __gr_Modes = p;
      return;
    }
    switch(ModeCompare(p,__gr_Modes)) {
      case -1 : /* p < __gr_Modes */
		p->next = __gr_Modes;
		__gr_Modes = p;
		return;
      case  0 : /* p == __gr_Modes */
		free(p);
		return;
    }
    q = __gr_Modes;
    while ((qn=q->next) != NULL) {
      switch (ModeCompare(p,qn)) {
	case -1 : /* p < q->next */
		  p->next = qn;
		  q->next = p;
		  return;
	case  0 : /* p == q->next */
		  free(p);
		  return;
      }
      q = q->next;
    }
    /* append to end */
    p->next = q->next;
    q->next = p;
  }
}

int __gr_getmode_whc(int mode, int *w, int *h, long *c) {
  GraphicsMode *p;
  p = __gr_Modes;
  mode -= __FIRST_DRIVER_SPECIFIC_MODE;
  if (mode < 0)
    return FALSE;
  while (p != NULL && mode-->0)
    p = p->next;
  if (p==NULL) return FALSE;
  *w = p->width;
  *h = p->height;
  *c = p->colors;
  return TRUE;
}

#ifdef GRX_VERSION

  /* ---- GRX v2.0 */
  static void build_mode_table(void) {
    GrFrameMode fmode;
    const GrVideoMode *mp;
    char ubpp;

    for (fmode = GR_firstGraphicsFrameMode;
	   fmode <= GR_lastGraphicsFrameMode;
	     ++fmode ) {
      mp = GrFirstVideoMode(fmode);
      while (mp != NULL) {
	ubpp = mp->bpp;
	if(ubpp > 24) ubpp = 24;
#if defined(__TURBOC__) && defined(__MSDOS__)
	if(ubpp < 15)
#endif
	NewMode(mp->width, mp->height, 1L << ubpp);
	mp = GrNextVideoMode(mp);
      }
    }
  }

  #define COLOR_NR2DRV(col) (col)

#else

  /* ---- GRX v1.0x */
  #ifdef GR_DRV_VER_GRD
  #define _GRD GR_DRV_VER_GRD
  #define _GRN GR_DRV_VER_GRN
  #define _VDR GR_DRV_VER_VDR
  #endif

  static long COLOR_NR2DRV(int col) {
    switch (col) {
      case 1<<16 : return 0xC010;
      case 1<<24 : return 0xC018;
    }
    return col;
  }

  static long COLOR_DRV2NR(int c) {
    if ( (c & 0xC000) == 0xC000)
      c = 1<<(c & 0x00ff);
    return c;
  }

  static int color_ok(long c) {
    /* Check if GRX supports requested color mode */
    int old_colors, res;
    extern int _GrNumColors;  /* happy hacking ... */
    c = COLOR_DRV2NR(c);
    switch (c) {
      case 1<<15 :
      case 1<<16 :
      case 1<<24 : old_colors = _GrNumColors;
		   _GrNumColors = c;
		   res = GrLineOffset(128) != 0;
		   _GrNumColors = old_colors;
		   return res;
    }
    return TRUE; /* No way to check other colors ... */
  }

  #ifdef _VDR
  static int driver = _GRD;
  #endif

  static int CheckTableEntry(GR_DRIVER_MODE_ENTRY *gm) {
    if (gm == NULL) return FALSE;
    if (!color_ok(gm->number_of_colors) )
      return FALSE;
    #ifdef _VDR
      if (driver == _VDR)
	return gm->mode.vdr.BIOS_mode != 0xFFF;
      if (driver == _GRN)
	return gm->mode.grn.BIOS_mode != 0xFF;
      return FALSE;
    #else
      return gm->BIOS_mode != 0xFF;
    #endif
  }

  static void build_mode_table(void) {
    GR_DRIVER_MODE_ENTRY *tm, *gm;
    #ifdef _VDR
    driver = 0xFFFF0000 &
    #endif
    GrGetDriverModes(&tm, &gm);

    while (gm->width != 0 && gm->height != 0 && gm->number_of_colors != 0) {
      if (CheckTableEntry(gm))
	NewMode(gm->width, gm->height, COLOR_DRV2NR(gm->number_of_colors));
      ++gm;
    }
  }

#endif

/* ----------------------------------------------------------------- */

void __gr_set_up_modes(void)
{
  int mode;
  GraphicsMode *p;
  static int DidInit = FALSE;

  if (DidInit) return;
  if (strlen(copyright) != sizeof(copyright)-1)
    ; // exit(1);
# ifdef __linux__
   /* vga_init(); */
# endif
  MM = 1;
  GrSetMode( GrCurrentMode());                       /* Init grx */
  __gr_ADAPTER = GrAdapterType();
  if (__gr_ADAPTER == GR_S3)
    __gr_ADAPTER = GR_VGA;
  build_mode_table();
  p = __gr_Modes;
  mode = __FIRST_DRIVER_SPECIFIC_MODE-1;
  while (p != NULL) {
    ++mode;
    p = p->next;
  }
  MM = mode;
  DidInit = TRUE;
}

/* ----------------------------------------------------------------- */

int __gr_White(void) {
  return GrNumColors() > 256 ? GrWhite() : 15;
}

int getmaxx(void) {
  return GrScreenX()-1;
}

int getmaxy(void) {
  return GrScreenY()-1;
}

/* ----------------------------------------------------------------- */
void graphdefaults(void)
{
  ERR          = grOk;
  moveto( 0, 0);
  COL = FILL   = WHITE;
  COLBG        = BLACK;
  __gr_WR      = GrWRITE;

  _DO_INIT_CHECK;
  GrSetContext( NULL);         /* ViewPort == Full screen */
#ifdef GRX_VERSION
  GrSetViewport(0,0);
  PY = 0;
#endif
  VL = VT = 0;
  VR = getmaxx();
  VB = getmaxy();
  __gr_clip = TRUE;

  __gr_Xasp = 10000;
  __gr_Yasp = __gr_Xasp * ((VR+1)*3L) / ((VB+1)*4L);

  __gr_lstyle     = SOLID_LINE;
  LNE.lno_pattlen = 0;
  LNE.lno_dashpat = NULL;
  LNE.lno_width   = 1;

  FPATT = SOLID_FILL;
  FILLP.gp_ispixmap    = 0;    /* Bitmap fill pattern */
  FILLP.gp_bmp_data    = (unsigned char *)&__gr_fpatterns[FPATT];
  FILLP.gp_bmp_height  = 8;
  FILLP.gp_bmp_fgcolor = COL;
  FILLP.gp_bmp_bgcolor = COLBG;

  __gr_TextLineStyle = 0;
}

/* ----------------------------------------------------------------- */

int __gr_adaptcnv(int grx) {
  switch (grx) {
    case GR_VGA   : return VGA;
    case GR_EGA   : return EGA;
    case GR_8514A : return IBM8514;
    case GR_HERC  : return HERCMONO;
  }
  return grx;
}

/* ----------------------------------------------------------------- */
void setgraphmode(int mode)
{
  int w, h;
  long c;

  _DO_INIT_CHECK;
  switch (mode) {
    case GRX_DEFAULT_GRAPHICS:
#ifdef GRX_VERSION
      w = GrDriverInfo->defgw;
      h = GrDriverInfo->defgh;
      c = GrDriverInfo->defgc;
      goto Default;
#else
      GrSetMode( GR_default_graphics);
      break;
#endif
    case GRX_BIGGEST_NONINTERLACED_GRAPHICS:
      GrSetMode( GR_biggest_noninterlaced_graphics);
      break;
    case GRX_BIGGEST_GRAPHICS:
      GrSetMode( GR_biggest_graphics);
      break;
    case GRX_BGI_EMULATION:
#ifdef GRX_VERSION
      if (__gr_BGI_p > 1) {
	__gr_BGI_p = 2;
	GrSetMode( GR_custom_graphics , __gr_BGI_w, __gr_BGI_h,
		   COLOR_NR2DRV(__gr_BGI_c), __gr_BGI_w, 2*__gr_BGI_h);
	if (GrVirtualY() < 2*GrScreenY()) __gr_BGI_p = 1;
      } else
#endif
	GrSetMode( GR_width_height_color_graphics, __gr_BGI_w,
		   __gr_BGI_h, COLOR_NR2DRV(__gr_BGI_c));
      break;
    default:
      if (mode < __FIRST_DRIVER_SPECIFIC_MODE || mode > MM) {
	ERR = grInvalidMode;
	return;
      }
      if (!__gr_getmode_whc(mode, &w, &h, &c)) {
	ERR = grInvalidMode;
	return;
      }
#ifdef GRX_VERSION
Default:
      if (__gr_BGI_p > 1) {
	__gr_BGI_p = 2;
	GrSetMode( GR_custom_graphics , w, h, COLOR_NR2DRV(c), w, 2*h);
	if (GrVirtualY() < 2*GrScreenY()) __gr_BGI_p = 1;
      } else
#endif
	GrSetMode( GR_width_height_color_graphics, w, h, COLOR_NR2DRV(c));
      break;
  }
  __gr_Mode = mode;
  graphdefaults();
  GrClearScreen(BLACK);
}

/* ----------------------------------------------------------------- */
void __gr_initgraph(int *graphdriver, int *graphmode) {
  ERR = grOk;
  if (__gr_initgraph_hook) {
    (*__gr_initgraph_hook) ();
    if (ERR != grOk) {
      __gr_INIT = FALSE;
      return;
    }
  } else if (!__gr_INIT) {
    __gr_set_up_modes();
    if (ERR != grOk) return;
    if ( *graphdriver != NATIVE_GRX || *graphmode < 0 || *graphmode > MM)
      *graphmode = 0;
    __gr_INIT = TRUE;
    setgraphmode(*graphmode);
    if (ERR != grOk) {
      __gr_INIT = FALSE;
      return;
    }
  }
  if (*graphmode == 0) *graphdriver = __gr_adaptcnv(__gr_ADAPTER);
		  else *graphdriver = NATIVE_GRX;
}



void closegraph(void)
{
  if (__gr_closegraph_hook) {
    (*__gr_closegraph_hook) ();
    __gr_initgraph_hook = NULL;
    __gr_closegraph_hook = NULL;
  } else
  restorecrtmode();
  __gr_INIT = FALSE;
}



void  __gr_set_libbcc_init_hooks (
                      int (*init) (void) ,
                      int (*close) (void) )
{
   __gr_initgraph_hook  = init;
   __gr_closegraph_hook = close;
}



void initgraph(int *graphdriver, int *graphmode, char *pathtodriver) {
  __gr_initgraph(graphdriver, graphmode);
  strcpy(__gr_BGICHR, NULL_IS_EMPTY(pathtodriver));
}
