/**
 ** grfontdv.h ---- font driver declarations
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

#ifndef __GRFONTDV_H_INCLUDED__
#define __GRFONTDV_H_INCLUDED__

#ifndef __GRX20_H_INCLUDED__
#include "grx20.h"
#endif

/*
** The following definitions shouldn't be used in portable
** or binary distributed programs for compatibility with
** future versions of GRX
*/
#ifdef USE_GRX_INTERNAL_DEFINITIONS

/*
 * Font driver header. Font drivers are used to load various font file
 * formats into the internal bitmap ('GrFont') representation.
 */
typedef struct _GR_fontDriver {
    char  *name;                        /* font format name (doc only) */
    char  *ext;                         /* font file name extension */
    int    scalable;                    /* scalable font file format */
    int  (*openfile)(char *fname);
    int  (*header)(GrFontHeader *hdr);
    int  (*charwdt)(int chr);
    int  (*bitmap)(int chr,int w,int h,char *buffer);
    void (*cleanup)(void);
} GrFontDriver;

extern GrFontDriver
/*
 * Available font drivers in GRX
 */
_GrFontDriverGRX,                       /* native GRX bitmap fonts */
_GrFontDriverBGI,                       /* Borland BGI font driver */
#ifdef __XWIN__
_GrFontDriverXWIN,                      /* X font driver */
#endif
_GrFontDriverRAW,                       /* RAW data/linux PSF font driver */
_GrFontDriverFNA,                       /* ASCII font driver */
_GrFontDriverWIN,                       /* MS Windows font resource driver */
/*
 * This is a NULL-terminated table of font driver descriptor pointers. Users
 * can provide their own table with only the desired (or additional) drivers.
 * Otherwise the table from the GRX library is linked, which includes ALL
 * currently available drivers (i.e. the ones above).
 */
*_GrFontDriverTable[];

/*
 * Various bits of font related global data
 */
extern struct _GR_fontFileInfo {
    int     npath;                      /* number of dirs to search */
    char  **path;                       /* the search directories */
} _GrFontFileInfo;

/*
 * utilities
 */
GrFont *_GrBuildFont(
    const GrFontHeader *hdr,
    int  cvt,
    int  width,
    int  height,
    int  minch,
    int  maxch,
    int  (*charwdt)(int chr),
    int  (*bitmap)(int chr,int w,int h,char far *buffer),
    int  canscale
);

#endif /* USE_GRX_INTERNAL_DEFINITIONS */

#endif /* whole file */

