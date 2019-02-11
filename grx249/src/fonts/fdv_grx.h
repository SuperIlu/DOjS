/**
 ** fdv_grx.h -- GRX native font file format
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

#ifndef __FDV_GRX_H_INCLUDED__
#define __FDV_GRX_H_INCLUDED__

/*
 *  font file structure:
 *  +-----------------------+
 *  |     FILE HEADER       |
 *  +-----------------------+
 *  |     PROPORTIONAL      |
 *  |     WIDTH TABLE       |
 *  |     (16 bit ints)     |
 *  |     (prop fonts only) |
 *  +-----------------------+
 *  |     BITMAP            |
 *  +-----------------------+
 */

#define GRX_NAMEWIDTH   16
#define GRX_FONTMAGIC   0x19590214L
#define GRX_FONTMAGIC_SWAPPED 0x14025919L

typedef struct _GR_fontFileHeaderGRX {  /* the header */
    GR_int32u magic;                  /* font file magic number */
    GR_int32u bmpsize;                /* character bitmap size */
    GR_int16u width;                  /* width (average for proportional) */
    GR_int16u height;                 /* font height */
    GR_int16u minchar;                /* lowest character code in font */
    GR_int16u maxchar;                /* highest character code in font */
    GR_int16u isfixed;                /* nonzero if fixed font */
    GR_int16u reserved;               /* ??? */
    GR_int16u baseline;               /* baseline from top of font */
    GR_int16u undwidth;               /* underline width (at bottom) */
    char      fnname[GRX_NAMEWIDTH];  /* font file name (w/o path) */
    char      family[GRX_NAMEWIDTH];  /* font family name */
} GrFontFileHeaderGRX;

#endif /* whole file */

