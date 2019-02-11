/**
 ** fdv_win.h -- driver for Windows resource font file format
 **
 ** Copyright (C) 2002 Dimitar Zhekov
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

#ifndef __FDV_WIN_H_INCLUDED__
#define __FDV_WIN_H_INCLUDED__

#ifdef __GNUC__
#define PACKED __attribute__((packed))
#else
#define PACKED
#endif

typedef struct _GR_resourceFileHeaderWIN
{
    GR_int8u  type_ff;
    GR_int16u type_id;
    GR_int8u  name_ff;
    GR_int16u name_id;
    GR_int16u flags;
    GR_int32u size;
} PACKED GrResourceFileHeaderWIN;

typedef struct _GR_fontFileHeaderWIN {  /* the header */
    GR_int16u version;
    GR_int32u size;
    GR_int8   copyright[60];
    GR_int16u type;
    GR_int16u points;
    GR_int16u vert_res;
    GR_int16u horiz_res;
    GR_int16u ascent;
    GR_int16u internal_leading;
    GR_int16u external_leading;
    GR_int8u  italic;
    GR_int8u  underline;
    GR_int8u  strike_out;
    GR_int16u weight;
    GR_int8u  char_set;
    GR_int16u pix_width;
    GR_int16u pix_height;
    GR_int8u  pitch_and_family;
    GR_int16u avg_width;
    GR_int16u max_width;
    GR_int8u  first_char;
    GR_int8u  last_char;
    GR_int8u  default_char;
    GR_int8u  break_char;
    GR_int16u width_bytes;
    GR_int32u device;
    GR_int32u face;
    GR_int32u bits_pointer;
    GR_int32u bits_offset;
    GR_int8u  reserved;
} PACKED GrFontFileHeaderWIN;

typedef struct _GR_charHeaderWIN
{
    GR_int16u width;
    GR_int16u offset;
} PACKED GrCharHeaderWIN;

#endif
