/**
 ** CTX2TIFF.C ---- saves a context in a TIFF file
 **
 ** Copyright (c) 1997 Hartmut Schirmer
 **
 ** requires tifflib by  Sam Leffler (sam@engr.sgi.com)
 **        available at  ftp://ftp.sgi.com/graphics/tiff
 **
 ** should work with every compiler supporting both,
 ** tifflib and GRX libraries
 **/

#include <stdlib.h>
#include <grx20.h>
#include <tiffio.h>
#include <string.h>

#define SCALE(x)        ((x)<<8)

#define puttobuf(r,x,depth,col) do {                      \
    switch (depth) {                                      \
      case 1 : if (col) {                                 \
		 int  offset = (x) >> 3;                  \
		 int  mask = 0x80 >> ((x) & 7);           \
		 r[offset] |= mask;                       \
	       }                                          \
	       break;                                     \
      case 8 : r[x] = (unsigned char)(col); break;        \
      case 24: r[3*(x)+0] = GrRGBcolorRed(col);           \
	       r[3*(x)+1] = GrRGBcolorGreen(col);         \
	       r[3*(x)+2] = GrRGBcolorBlue(col);          \
	       break;                                     \
    }                                                     \
} while(0)


/*
** SaveContextToTiff - Dump a context in a TIFF file
**
** Arguments:
**   ctx:   Context to be saved (NULL -> use current context)
**   tiffn: Name of tiff file
**   compr: Compression method (see tiff.h), 0: automatic selection
**   docn:  string saved in the tiff file (DOCUMENTNAME tag)
**
**  Returns  0 on success
**          -1 on error
*/
int
SaveContextToTiff(GrContext *ctx, char *tiffn, unsigned compr, char *docn) {
    int    depth, i, res;
    long   row;
    TIFF  *tif;
    long   width, height, colors;
    short  photometric;
    short  samplesperpixel;
    short  bitspersample;
    unsigned char *r;
    unsigned short red[256], green[256], blue[256];

    if (!ctx) ctx = (GrContext *)GrCurrentContext();
    if (!ctx) return -1;
    width  = ctx->gc_xmax+1;
    height = ctx->gc_ymax+1;
    colors = GrNumColors();
    if (colors < 2) return -1;
    if (colors ==   2)    depth = 1;  else
    if (colors <= 256)    depth = 8;  else
    if (colors <= 1L<<24) depth = 24; else return -1;

    if (!compr) { /* compr == 0 -> auto select compression */
      if (depth==1) compr = COMPRESSION_CCITTFAX4;
	       else compr = COMPRESSION_LZW;
    }

    switch (depth) {
    case 1:
	samplesperpixel = 1;
	bitspersample = 1;
	photometric = PHOTOMETRIC_MINISBLACK;
	break;
    case 8:
	samplesperpixel = 1;
	bitspersample = 8;
	photometric = PHOTOMETRIC_PALETTE;
	break;
    case 24:
	samplesperpixel = 3;
	bitspersample = 8;
	photometric = PHOTOMETRIC_RGB;
	break;
    default:
	return -1; /* shouldn't happen */
    }

    r = (unsigned char *) malloc( depth>1 ?
	samplesperpixel*width*sizeof(unsigned char) :
	(width+7)/8);
    if (!r) return -1;

    tif = TIFFOpen(tiffn, "w");
    if (tif == NULL) {
      free(r);
      return -1;
    }

    TIFFSetField(tif, TIFFTAG_IMAGEWIDTH, width);
    TIFFSetField(tif, TIFFTAG_IMAGELENGTH, height);
    TIFFSetField(tif, TIFFTAG_BITSPERSAMPLE, bitspersample);
    TIFFSetField(tif, TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT);
    TIFFSetField(tif, TIFFTAG_COMPRESSION, compr);
    TIFFSetField(tif, TIFFTAG_PHOTOMETRIC, photometric);
    if (docn)
      TIFFSetField(tif, TIFFTAG_DOCUMENTNAME, docn);
    TIFFSetField(tif, TIFFTAG_IMAGEDESCRIPTION, "GRX saved context");
    TIFFSetField(tif, TIFFTAG_SAMPLESPERPIXEL, samplesperpixel);
    TIFFSetField(tif, TIFFTAG_ROWSPERSTRIP, height);
    TIFFSetField(tif, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);

    memset(red, 0, sizeof(red));
    memset(green, 0, sizeof(green));
    memset(blue, 0, sizeof(blue));
    if (depth == 8) {
	for (i = 0; i < colors; i++) {
	    int r, g, b;
	    GrQueryColor(i, &r, &g, &b);
	    red[i] = SCALE(r);
	    green[i] = SCALE(g);
	    blue[i] = SCALE(b);
	}
	TIFFSetField(tif, TIFFTAG_COLORMAP, red, green, blue);
    }
    res = 0;
    for (row = 0; row < height; row++) {
      int x;
      GrColor c;
#if GRX_VERSION_API-0 >= 0x229
      const GrColor *rcb = GrGetScanlineC(ctx,0,width-1,row);
      if (rcb) {
	for (x=0; x < width; ++x) {
	  c = rcb[x];
	  puttobuf(r,x,depth,c);
	}
      } else
#endif
      {
	if (depth==1) memset (r,0,(width+7)/8);
	for (x=0; x < width; ++x) {
	  c = GrPixelC(ctx,x,row);
	  puttobuf(r,x,depth,c);
	}
      }
      if (TIFFWriteScanline(tif, r, row, 0) < 0) {
	  res = -1;
	  break;
      }
    }
    TIFFFlushData(tif);
    TIFFClose(tif);
    free(r);
    return res;
}

#ifdef TEST_CTX2TIFF
#include "../test/test.h"

TESTFUNC(wintest)
{
	int  x = GrSizeX();
	int  y = GrSizeY();
	int  ww = (x / 2) - 10;
	int  wh = (y / 2) - 10;
	long c;
	GrContext *w1 = GrCreateSubContext(5,5,ww+4,wh+4,NULL,NULL);
	GrContext *w2 = GrCreateSubContext(15+ww,5,ww+ww+14,wh+4,NULL,NULL);
	GrContext *w3 = GrCreateSubContext(5,15+wh,ww+4,wh+wh+14,NULL,NULL);
	GrContext *w4 = GrCreateSubContext(15+ww,15+wh,ww+ww+14,wh+wh+14,NULL,NULL);

	GrSetContext(w1);
	c = GrAllocColor(200,100,100);
	drawing(0,0,ww,wh,c,GrBlack());
	c = GrAllocColor(100,50,50);
	GrBox(0,0,ww-1,wh-1,c);

	GrSetContext(w2);
	c = GrAllocColor(100,200,200);
	drawing(0,0,ww,wh,c,GrBlack());
	c = GrAllocColor(50,100,100);
	GrBox(0,0,ww-1,wh-1,c);

	GrSetContext(w3);
	c = GrAllocColor(200,200,0);
	drawing(0,0,ww,wh,c,GrBlack());
	c = GrAllocColor(100,100,0);
	GrBox(0,0,ww-1,wh-1,c);

	GrSetContext(w4);
	c = GrAllocColor(0,100,200);
	drawing(0,0,ww,wh,c,GrBlack());
	c = GrAllocColor(255,0,100);
	GrBox(0,0,ww-1,wh-1,c);

	GrSetContext(NULL);

	SaveContextToTiff(NULL, "test.tif", 0, "Context2TIFF test file");
	getch();
}
#endif


