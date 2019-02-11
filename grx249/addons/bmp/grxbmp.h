/*
**  <grxbmp.h>  - BMP read/write file
**                by Michal Stencl Copyright (c) 1998
**              - read  BMP 2, 4, 8 bpp
**              - write BMP 8, 24   bpp
**  <e-mail>    - [stenclpmd@ba.telecom.sk]
**
*/

#include "libgrx.h"

#define _GrBitmapPointerTypes_DEFINED_
typedef struct _GR_bitmapfileheader GrBitmapFileHeader;
typedef struct _GR_bitmapinfoheader GrBitmapInfoHeader;
typedef struct _GR_bmpimagecolors   GrBmpImageColors;
typedef struct _GR_bmpimage         GrBmpImage;

/* ************************************************************************ */
/* _GR_bitmapfileheader                                                     */
/* ************************************************************************ */
struct _GR_bitmapfileheader {
	GR_int16u  bf_type;
	GR_int32u  bf_size;
	GR_int16u  bf_reserved1;
	GR_int16u  bf_reserved2;
	GR_int32u  bf_offbits;
};

/* ************************************************************************ */
/* _GR_bitmapinfoheader                                                     */
/* ************************************************************************ */
struct _GR_bitmapinfoheader {
	GR_int32u   bn_size;
	GR_int32u   bn_width;
	GR_int32u   bn_height;
	GR_int16u   bn_planes;
	GR_int16u   bn_bitcount;
	GR_int32u   bn_compression;
	GR_int32u   bn_sizeimage;
	GR_int32u   bn_xpelspermeter;
	GR_int32u   bn_ypelspermeter;
	GR_int32u   bn_clrused;
	GR_int32u   bn_clrimportant;
};

/* ************************************************************************ */
/* _GR_bmpimagecolors  IMPORTANT                                            */
/* ************************************************************************ */
struct _GR_bmpimagecolors {
		GR_int8u           *bp_palette; /* (R, G, B, Reserved) * | 2 | 16 | 256 */
		GrColor            *bp_colormap;
		int                 bp_numcolors;
};

/* ************************************************************************ */
/* _GR_bmpimage   IMPORTANT                                                 */
/* ************************************************************************ */
struct _GR_bmpimage {
		GrBitmapFileHeader   *bi_bmpfileheader;
		GrBitmapInfoHeader   *bi_bmpinfoheader;
		GrBmpImageColors     *bi_bmpimagecolors;
		GR_int16s             bi_erasepalette;
		char                 *bi_map;
};

GrBmpImage    *GrLoadBmpImage ( char *_filename );
int            GrSaveBmpImage ( char *_filename, GrContext *_c, int _x1, int _y1, int _x2, int _y2 );
void           GrUnloadBmpImage ( GrBmpImage *_bmp );
int            GrAllocBmpImageColors ( GrBmpImage *_bmp, GrBmpImageColors *_pal );
int            GrFreeBmpImageColors ( GrBmpImageColors *_pal );
GrPattern     *GrConvertBmpImageToPattern ( GrBmpImage *_bmp );
GrPattern     *GrConvertBmpImageToStaticPattern ( GrBmpImage *_bmp );
unsigned long  GrBmpImageWidth ( GrBmpImage* _bmp );
unsigned long  GrBmpImageHeight ( GrBmpImage* _bmp );
char          *GrBmpImagePalette ( GrBmpImage* _bmp );
GrColor       *GrBmpImageColorMap ( GrBmpImage* _bmp );
GrColor        GrBmpImageNumColors ( GrBmpImage* _bmp );

