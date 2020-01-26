/*
** Copyright (c) 1995, 3Dfx Interactive, Inc.
** All Rights Reserved.
**
** This is UNPUBLISHED PROPRIETARY SOURCE CODE of 3Dfx Interactive, Inc.;
** the contents of this file may not be disclosed to third parties, copied or
** duplicated in any form, in whole or in part, without the prior written
** permission of 3Dfx Interactive, Inc.
**
** RESTRICTED RIGHTS LEGEND:
** Use, duplication or disclosure by the Government is subject to restrictions
** as set forth in subdivision (c)(1)(ii) of the Rights in Technical Data
** and Computer Software clause at DFARS 252.227-7013, and/or in similar or
** successor clauses in the FAR, DOD or NASA FAR Supplement. Unpublished  -
** rights reserved under the Copyright Laws of the United States.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "texusint.h"


typedef struct _tgaHeader{
    FxU8 IDLength;
    FxU8 CMapType;
    FxU8 ImgType;
    FxU8 CMapStartLo;
    FxU8 CMapStartHi;
    FxU8 CMapLengthLo;
    FxU8 CMapLengthHi;
    FxU8 CMapDepth;
    FxU8 XOffSetLo;
    FxU8 XOffSetHi;
    FxU8 YOffSetLo;
    FxU8 YOffSetHi;
    FxU8 WidthLo;
    FxU8 WidthHi;
    FxU8 HeightLo;
    FxU8 HeightHi;
    FxU8 PixelDepth;
    FxU8 ImageDescriptor;
} TgaHeader;

/* Definitions for image types. */
#define TGA_NULL	0
#define TGA_CMAP	1
#define TGA_TRUE	2
#define TGA_MONO	3
#define TGA_CMAP_RLE	9
#define TGA_TRUE_RLE	10
#define TGA_MONO_RLE	11

FxBool
_txReadTGAHeader( FILE *stream, FxU32 cookie, TxMip *info)
{
    TgaHeader	*tgaHeader = (TgaHeader *) info->pal;
    int			i;

    // Fill up rest of the TGA header. 
    if ( fread( &(tgaHeader->ImgType), 1, sizeof(TgaHeader)-2, stream ) != 
    	sizeof(TgaHeader)-2) {
    	txPanic("Unexpected end of file.");
    	return FXFALSE;
    }
    tgaHeader->IDLength	= (FxU8) ((cookie >>  8) & 0xFF);
    tgaHeader->CMapType = (FxU8) ((cookie      ) & 0xFF);

    // Optionally, skip the image id fields.
    for (i= (tgaHeader->IDLength) & 0xFF; i; i--) {
    	int	c;

    	if ((c = getc(stream)) == EOF) {
    		txPanic("Unexpected EOF.");
    		return FXFALSE;
    	}
    }

    // 3Dfx specific part here.
    info->width  = tgaHeader->WidthHi << 8 | tgaHeader->WidthLo;
    info->height = tgaHeader->HeightHi << 8 | tgaHeader->HeightLo;
    info->depth  = 1;

    if ((info->width <= 0) || (info->height <= 0)) {
    	txError("TGA Image: width or height is 0.");
    	return FXFALSE;
    }

    switch(tgaHeader->ImgType) {
    case TGA_MONO:
    case TGA_MONO_RLE:			// True color image.
    	if (tgaHeader->PixelDepth != 8) {
	    txError("TGA Image: Mono image is not 8 bits/pixel.");
	    return FXFALSE;
    	}
    	info->format = GR_TEXFMT_I_8;
    	break;

    case TGA_TRUE:
    case TGA_TRUE_RLE:
    	switch (tgaHeader->PixelDepth ) {
    	case	15: 
    	case	16: 
			info->format = GR_TEXFMT_ARGB_1555; break;
    	case	24: 
    	case	32: 
			info->format = GR_TEXFMT_ARGB_8888; break;
    	default:	
    			txError("TGA Image: True color image is not 24/32 bits/pixel.");
    			return FXFALSE; 
    			break;
    	}
    	break;
      
    case TGA_CMAP:
    case TGA_CMAP_RLE:			// Color mapped image.
    	if ( tgaHeader->CMapType     != 1 ) {
    		txError("TGA Image: Color-mapped TGA image has no palette");
    		return FXFALSE;
    	}
    	if (((tgaHeader->CMapLengthLo + tgaHeader->CMapLengthHi * 256L)
    	    +(tgaHeader->CMapStartLo + tgaHeader->CMapStartHi * 256L)) > 256){
    		txError("TGA Image: Color-mapped image has > 256 colors");
    		return FXFALSE;
    	}
    	info->format = GR_TEXFMT_P_8;
    	break;

    default:
    	txError("TGA Image: unsupported format");
    	return  FXFALSE;
    }
    info->size = info->width*info->height*GR_TEXFMT_SIZE(info->format);
    info->size >>= 3; // convert to bytes

    return FXTRUE;
}

static FxBool
_txReadTGAColorMap(FILE *stream, const TgaHeader *tgaHeader, FxU32 *palette)
{
    int		cmapStart;
    int		cmapLength;
    int		cmapDepth;
    int		i;

    cmapStart   = tgaHeader->CMapStartLo;
    cmapStart  += tgaHeader->CMapStartHi * 256L;

    cmapLength  = tgaHeader->CMapLengthLo;
    cmapLength += tgaHeader->CMapLengthHi * 256L;

    cmapDepth   = tgaHeader->CMapDepth;

    if (tgaHeader->CMapType == 0) return FXTRUE;		// no colormap.

    /* Validate some parameters */
    if (cmapStart < 0) {
    	txError("TGA Image: Bad Color Map start value.");
    	return FXFALSE;
    }

    cmapDepth = (cmapDepth + 1) >> 3;		// to bytes.
    if ((cmapDepth <= 0) || (cmapDepth > 4)) {
    	txError("TGA Image: Bad Color Map depth.");
    	return FXFALSE;
    }

    // May have to skip the color map.
    if ((tgaHeader->ImgType != TGA_CMAP) && 
    	(tgaHeader->ImgType != TGA_CMAP_RLE)) {
    	/* True color, yet there is a palette, this is OK, just skip. */

    	cmapLength *= cmapDepth;
    	while (cmapLength--) {
    		int		c;

    		c = getc(stream);
    		if (c == EOF) {
    			txError("TGA Image: Unexpected EOF reading Color Map.");
    			return FXFALSE;
    		}
    	}
    	return FXTRUE;
    }

    // This is a real palette that's going to be used. 

    // Verify that it's not too large.
    if ((cmapStart + cmapLength) > 256) {
    	txError("TGA Image: Color Map > 256 entries.");
    	return FXFALSE;
    }


    // printf("cmapdepth = %d, start = %d, length = %d\n", cmapDepth,
    // 	cmapStart, cmapLength);
    for (i=0; i<256; i++) {
    	int	r, g, b, a;

    	if ((i < cmapStart) || (i >= (cmapStart + cmapLength))) {
    		palette[i] = 0;
    		// printf("Skipping palette entry %d\n", i);
    		continue;
    	}

    	// Read this colormap entry.
    	switch (cmapDepth) {
    	case 1:	// 8 bpp
    		r = getc(stream);
    		if (r == EOF) {
  		    txError("TGA Image: Unexpected End of File.");
		    return FXFALSE;
    		}
    		r &= 0xFF;
    		palette[i] = (r << 24) | (r << 16) | (r << 8) | (r);
   		break;

    	case 2:	// 15, 16 bpp.

   		b = getc(stream);
   		r = getc(stream);
   		if ((r == EOF) || (b == EOF)) {
   			txError("TGA Image: Unexpected End of File.");
   			return FXFALSE;
   		}
   		r &= 0xFF;
   		b &= 0xFF;
   		g = ((r & 0x3) << 6) + ((b & 0xE0) >> 2);
   		r = (r & 0x7C) << 1;
   		b = (b & 0x1F) << 3;

   		palette[i] = (r << 16) | (g << 8) | (b) | 0xFF000000L;
   		break;

    	case 3:
    	case 4:
   		b = getc(stream);
   		g = getc(stream);
   		r = getc(stream);
   		a = (cmapDepth == 4) ? getc(stream) : 0x0FF;

   		if ((r == EOF) || (g == EOF) || (b == EOF) | (a == EOF)) {
   			txError("TGA Image: Unexpected End of File.");
   			return FXFALSE;
   		}
   		palette[i] = (a << 24) | (r << 16) | (g << 8) | b;
   		// printf("Setting palette %3d to %.08x\n", i, palette[i]);
   		break;

    	default:
    		txError("TGA Image: Bad Color Map depth.");
    		return FXFALSE;
    	}
    }
    return FXTRUE;
}

static	int	tgaRLE, tgaRLEflag, tgaRLEcount, tgaRLEsav[4]; 

static	FxBool
_txReadTGARLEPixel( FILE *stream, FxU8 *data, int pixsize)
{
    int		c, i;

    // Run length encoded data Only
    if (tgaRLEcount == 0) {
    	// Need to restart the run.
    	if ( (tgaRLEcount = c = getc( stream )) == EOF) {
    		txError("TGA Image: Unexpected End of File.");
    		return FXFALSE;
    	}
    	tgaRLEflag = tgaRLEcount & 0x80;
    	tgaRLEcount = (tgaRLEcount & 0x7F) + 1;

    	if (tgaRLEflag) {
	    // Replicated color, read the color to be replicated 
    	    for (i=0; i<pixsize; i++) {
		if ( (c = getc( stream )) == EOF) {
    		    txError("TGA Image: Unexpected End of File\n");
    		    return FXFALSE;
    		}
    		tgaRLEsav[i] = (FxU8) c;
    	    }
    	}
    }

    // Now deliver the data either from input or from saved values.
    tgaRLEcount--;
    if (tgaRLEflag) {
    	// deliver from saved data.
    	for (i=0; i<pixsize; i++) *data++ = (FxU8) tgaRLEsav[i];
    } else {
    	for (i=0; i<pixsize; i++) {
    	    if ( (c = getc( stream )) == EOF) {
    		txError("TGA Image: Unexpected End of File\n");
    		return FXFALSE;
	    }
    	    *data++ = (FxU8) c;
    	}
    }
    return FXTRUE;
}

static FxBool
_txReadTGASpan( FILE *stream, FxU8 *data, int w, int pixsize)
{
    if (tgaRLE == 0) {
    	if ( fread( data, 1, w * pixsize, stream) != (FxU32)(w * pixsize)) {
    		txError("TGA Image: Unexpected End of File\n");
    		return FXFALSE;
    	}
    	return FXTRUE;
    }

    // Otherwise, RLE data.
    while (w--) {
    	if (!_txReadTGARLEPixel( stream, data, pixsize)) {
	    return FXFALSE;
    	}
    	data += pixsize;
    }
    return FXTRUE;
}

FxBool
_txReadTGAData( FILE *stream, TxMip *info)
{
    TgaHeader	*tgaHeader = (TgaHeader *) info->pal;
    int		i, stride;
    int 	bpp;			// bytesPerPixel
    FxU8*	data;
    int		BigEndian = 0xff000000;

    // printf("TxREAD TGA DATA\n");
    tgaRLEcount = 0;

    bpp = (tgaHeader->PixelDepth + 1) >> 3;

    switch (tgaHeader->ImgType) {
    case TGA_MONO:	tgaRLE = 0; info->format = GR_TEXFMT_I_8; break;
    case TGA_MONO_RLE:	tgaRLE = 1; info->format = GR_TEXFMT_I_8; break;

    case TGA_TRUE:	tgaRLE = 0; info->format = (bpp == 2) ? 
    			GR_TEXFMT_ARGB_1555 : GR_TEXFMT_ARGB_8888; break;
    case TGA_TRUE_RLE:	tgaRLE = 1; info->format = (bpp == 2) ? 
    			GR_TEXFMT_ARGB_1555 : GR_TEXFMT_ARGB_8888; break;

    case TGA_CMAP:	tgaRLE = 0; info->format = GR_TEXFMT_P_8; break;
    case TGA_CMAP_RLE:	tgaRLE = 1; info->format = GR_TEXFMT_P_8; break;
    }

    // printf("bpp = %d, rle = %d\n", bpp, tgaRLE);

    stride =  info->width * bpp;
    data   =  info->data[0];
    if ((tgaHeader->ImageDescriptor & 0x20) == 0) {
    	// Origin is lower left
    	data   = data + (info->height-1) * stride;
    	stride = -stride;
    }

    /* If there's a colormap, read it now. */
    if (!_txReadTGAColorMap(stream, tgaHeader, (FxU32 *) &(info->pal[0]))) 
    	return FXFALSE;
    // printf("read in color map\n");

    /* Read in all the data */
    for ( i = 0; i < info->height; i++) {
    	if (!_txReadTGASpan( stream, data, info->width, bpp)) {
    			txError("TGA Image: Unexpected end of file.");
    			return FXFALSE;
    	}
    	data += stride;
    }

    /*
     * BPP == 1 -> P8 or I8 formatted data.
     * BPP == 2 -> ARGB1555 formatted data.
     * BPP == 4 -> ARGB8888 formatted data.
     * BPP == 3  should be translated to ARGB8888 from RGB888.
     */
    // printf("Repacking\n");
    if (bpp == 3) {
    	int		npixels = info->width * info->height;
    	FxU8	*src = ((FxU8 *) info->data[0]) + (npixels - 1) * 3;
    	FxU8	*dst = ((FxU8 *) info->data[0]) + (npixels - 1) * 4;

    	while (npixels--) {
    		dst[3] = 0xFF;
    		dst[2] = src[2];
    		dst[1] = src[1];
    		dst[0] = src[0];
    		dst -= 4;
    		src -= 3;
    	}
    }
    // printf("Done\n");
    if (*(FxU8 *)&BigEndian) {
	/* Repack 16bpp and 32bpp cases */
        if (bpp == 2) {
    	    int		npixels = info->width * info->height;
    	    FxU16	*src = (FxU16 *) info->data[0];

    	    while (npixels--) {
            	*src = (*src << 8) | ((*src >> 8) & 0xff);
                src++;
    	    }
        }
        if ((bpp == 3) || (bpp == 4)) {
    	    int		npixels = info->width * info->height;
    	    FxU32	*src = (FxU32 *) info->data[0];

    	    while (npixels--) {
            	*src = (((*src      ) & 0xff)  << 24)|
            	       (((*src >>  8) & 0xff)  << 16)|
            	       (((*src >> 16) & 0xff)  <<  8)|
            	       (((*src >> 24) & 0xff)       );
                src++;
    	    }
        }
    }
    return FXTRUE;
}
