/**
 ** fdv_raw.h -- driver for raw font file format
 **
 ** Copyright (C) 2003 Dimitar Zhekov
 ** [e-mail: jimmy@is-vn.bg]
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

#ifndef __FDV_RAW_H_INCLUDED__
#define __FDV_RAW_H_INCLUDED__

#ifdef __GNUC__
#define PACKED __attribute__((packed))
#else
#define PACKED
#endif

#define PSF1_MAGIC0     0x36
#define PSF1_MAGIC1     0x04
#define PSF1_MODE512    0x01
#define PSF1_UNICODE    0x02
#define PSF1_HDRSIZE    0x04

#define PSF2_MAGIC0     0x72
#define PSF2_MAGIC1     0xb5
#define PSF2_MAGIC2     0x4a
#define PSF2_MAGIC3     0x86
#define PSF2_UNICODE    0x01

typedef struct _GR_fontFileHeaderPSF {  /* the header */
    GR_int8u id[2];
    GR_int8u mode;			/* or psf2 id[2] */
    GR_int8u size;			/* or psf2 id[3] */
    GR_int32u version;
    GR_int32u offset;
    GR_int32u flags;
    GR_int32u numchars;
    GR_int32u charsize;
    GR_int32u height, width;
} PACKED GrFontFileHeaderPSF;

#endif
