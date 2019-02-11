/**
 ** fdv_bgi.h -- Borland BGI font file format
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
 ** Contributions by: (See "doc/credits.doc" for details)
 ** Hartmut Schirmer (hsc@techfak.uni-kiel.de)
 **
 **/

#ifndef __FDV_BGI_H_INCLUDED__
#define __FDV_BGI_H_INCLUDED__

/* This is based on the font code in Hartmut Schirmer's BCC2GRX package */

#ifdef  __GNUC__
#define PACKED  __attribute ((packed))
#else
#define PACKED
#endif

/*
 * Stroked BGI font file layout:
 *
 * +-------------------------------------------------------------------+
 * | ASCII copyright message, starts with "PK\b\b", terminated with ^Z |
 * +-------------------------------------------------------------------+
 * | BGIfontFileHeader structure                                       |
 * +-------------------------------------------------------------------+
 * | BGIfontHeaderType structure (at offset 'header_size')             |
 * +-------------------------------------------------------------------+
 * | table of unsigned short offsets into the data table               |
 * +-------------------------------------------------------------------+
 * | table of unsigned char character widths                           |
 * +-------------------------------------------------------------------+
 * | data table containing stroke vector coordinates and commands      |
 * +-------------------------------------------------------------------+
 */

typedef struct {
    GR_int16u header_size  PACKED; /* Version 2.0 Header Format */
    char      font_name[4] PACKED; /* Font Internal Name */
    GR_int16u font_size    PACKED; /* file size in bytes */
    GR_int8u  font_major   PACKED; /* Driver Version Information */
    GR_int8u  font_minor   PACKED;
    GR_int8u  bgi_major    PACKED; /* BGI Revision Information */
    GR_int8u  bgi_minor    PACKED;
} BGIfontFileHeader;

typedef struct {
    char       sig          PACKED; /* SIGNATURE byte */
    GR_int16u  nchrs        PACKED; /* number of characters in file */
    char       unused1      PACKED; /* Currently Undefined */
    GR_int8u   firstch      PACKED; /* first character in file */
    GR_int16u  cdefs        PACKED; /* offset to char definitions */
    GR_int8u   scan_flag    PACKED; /* (!= 0) -> set is scanable */
    char       org_to_cap   PACKED; /* Height from origin to top of capitol */
    char       org_to_base  PACKED; /* Height from origin to baseline */
    char       org_to_dec   PACKED; /* Height from origin to bot of decender */
    char       unused2[5]   PACKED; /* Currently undefined */
} BGIfontHeaderType;

#define FILEMARKER      "PK\b\b"        /* file should start with this */
#define MARKEREND       ('Z' - 0x40)    /* end of copyright message */
#define SIGBYTE         ('+')           /* value of signature byte */

/*
 * macros to parse data words in the stroke vector table
 */
#if BYTE_ORDER==LITTLE_ENDIAN
#define SV_COMMAND(w)   ((w) & 0x8080U)
#define SV_XCOORD(w)    ((int)(signed char)((w) << 1) >> 1)
#define SV_YCOORD(w)    ((int)(signed char)((w) >> 7) >> 1)

#define SVC_END         0x0000U
#define SVC_MOVE        0x0080U
#define SVC_SCAN        0x8000U
#define SVC_DRAW        0x8080U
#else
/* BIG_ENDIAN system */
#define SV_COMMAND(w)   ((w) & 0x8080U)
#define SV_YCOORD(w)    ((int)(signed char)((w) << 1) >> 1)
#define SV_XCOORD(w)    ((int)(signed char)((w) >> 7) >> 1)

#define SVC_END         0x0000U
#define SVC_MOVE        0x8000U
#define SVC_SCAN        0x0080U
#define SVC_DRAW        0x8080U
#endif

#endif  /* whole file */
