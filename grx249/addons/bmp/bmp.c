/*
**  <bmp.c>     - BMP read/write file
**                by Michal Stencl Copyright (c) 1998
**              - read  BMP 2, 4, 8 bpp
**              - write BMP 8, 24   bpp
**  <e-mail>    - [stenclpmd@ba.telecom.sk]
**
*/

#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#if defined(_MSC_VER) && defined(_WIN32)
#include <io.h>
#else
#include <unistd.h>
#endif
#include <fcntl.h>
#include <sys/stat.h>
#include <string.h>
#ifdef __MSDOS__
#  include <io.h>
#endif

#include "libgrx.h"
#include "clipping.h"

#if defined(__MSDOS__) || defined(MSDOS)
#define BIN_WR (O_WRONLY | O_BINARY)
#define BIN_RD (O_RDONLY | O_BINARY)
#else
#define BIN_WR O_WRONLY
#define BIN_RD O_RDONLY
#endif

#define BIN_CREAT  (BIN_WR|O_CREAT)

#ifndef S_IREAD
#define S_IREAD S_IRUSR
#endif
#ifndef S_IWRITE
#define S_IWRITE S_IWUSR
#endif

#define CREAT_PERM S_IREAD|S_IWRITE

#ifndef  TRUE
#define  TRUE 1
#endif

#ifndef  FALSE
#define  FALSE 0
#endif

#define  BMPFILEHEADERSIZE    14
#define  BMPINFOHEADERSIZE    40

#define  BI_RGB               0L
#define  BI_RLE8              1L
#define  BI_RLE4              2L

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
/* _GR_bmpimagecolors                                                       */
/* ************************************************************************ */
struct _GR_bmpimagecolors {
	GR_int8u           *bp_palette; /* (R, G, B, Reserved) * | 2 | 16 | 256 */
	GrColor            *bp_colormap;
	int                 bp_numcolors;
};

/* ************************************************************************ */
/* _GR_bmpimage                                                             */
/* ************************************************************************ */
struct _GR_bmpimage {
	GrBitmapFileHeader   *bi_bmpfileheader;
	GrBitmapInfoHeader   *bi_bmpinfoheader;
	GrBmpImageColors     *bi_bmpimagecolors;
	GR_int16s             bi_erasepalette;
	char                 *bi_map;
};

#define  bi_width      bi_bmpinfoheader->bn_width
#define  bi_height     bi_bmpinfoheader->bn_height
#define  bi_palette    bi_bmpimagecolors->bp_palette
#define  bi_colormap   bi_bmpimagecolors->bp_colormap
#define  bi_numcolors  bi_bmpimagecolors->bp_numcolors

int            GrLoadBmpFileHeader ( int _handle, GrBitmapFileHeader* _fileheader );
int            GrLoadBmpInfoHeader ( int _handle, GrBitmapInfoHeader* _infoheader );
static unsigned char *__GrLoadPaletteBmp ( int _handle, unsigned long _paloffset, int _colors );
static unsigned char *GrLoadPaletteBmp ( int _handle, int *_col, GrBitmapInfoHeader *_iheader );
static char   *GrLoadImageFromBmpBiRgb ( int _handle, unsigned long _offset, long _maxbufsize, int _colors, GrBitmapInfoHeader *_infoheader );
static char   *GrLoadImageFromBmpBiRle8 ( int _handle, unsigned long _offset, unsigned long _maxbufsize, int _colors, GrBitmapInfoHeader *_infoheader );
static char   *GrLoadImageFromBmpBiRle4 ( int _handle, unsigned long _offset, unsigned long _maxbufsize, int _colors, GrBitmapInfoHeader *_infoheader );
static char   *GrLoadImageFromBmp ( int _handle, unsigned long _offset, int _colors, GrBitmapInfoHeader *_infoheader );

/* exported functions */
int            GrFreeBmpImageColors ( GrBmpImageColors *_pal );
int            GrAllocBmpImageColors ( GrBmpImage *_bmp, GrBmpImageColors *_pal );
GrBmpImage    *GrLoadBmpImage ( char *_filename );
GrPattern     *GrConvertBmpImageToPattern ( GrBmpImage *_bmp );
GrPattern     *GrConvertBmpImageToStaticPattern ( GrBmpImage *_bmp );
void           GrUnloadBmpImage ( GrBmpImage *_bmp );
int            GrSaveBmpImage ( char *_filename, GrContext *_c, int _x1, int _y1, int _x2, int _y2 );
unsigned long  GrBmpImageWidth ( GrBmpImage* _bmp );
unsigned long  GrBmpImageHeight ( GrBmpImage* _bmp );
char          *GrBmpImagePalette ( GrBmpImage* _bmp );
GrColor       *GrBmpImageColorMap ( GrBmpImage* _bmp );
GrColor        GrBmpImageNumColors ( GrBmpImage* _bmp );
/* end of exported functions */

/* ************************************************************************ */
int  GrLoadBmpFileHeader ( int _handle, GrBitmapFileHeader *_fileheader )
/* ************************************************************************ */
{
  if (( !_fileheader ) || ( _handle == -1 )) return FALSE;
  memset(_fileheader, 0, BMPFILEHEADERSIZE);
  lseek(_handle, SEEK_SET, 0);
  read(_handle, &_fileheader->bf_type, 2);
  read(_handle, &_fileheader->bf_size, 4);
  read(_handle, &_fileheader->bf_reserved1, 2);
  read(_handle, &_fileheader->bf_reserved2, 2);
  read(_handle, &_fileheader->bf_offbits, 4);
  return TRUE;
}

/* ************************************************************************ */
int  GrLoadBmpInfoHeader ( int _handle, GrBitmapInfoHeader *_infoheader )
/* ************************************************************************ */
{
  if (( !_infoheader ) || ( _handle == -1 )) return FALSE;
  lseek(_handle, SEEK_SET, BMPFILEHEADERSIZE);
  _infoheader->bn_size = 0;
  read(_handle, &_infoheader->bn_size, 4);
  memset(_infoheader, 0, _infoheader->bn_size);
  read(_handle, &_infoheader->bn_width, 4);
  if ( _infoheader->bn_width % 4 )
    _infoheader->bn_width += 4 - (_infoheader->bn_width % 4);
  read(_handle, &_infoheader->bn_height, 4);
  read(_handle, &_infoheader->bn_planes, 2);
  read(_handle, &_infoheader->bn_bitcount, 2);
  read(_handle, &_infoheader->bn_compression, 4);
  read(_handle, &_infoheader->bn_sizeimage, 4);
  read(_handle, &_infoheader->bn_xpelspermeter, 4);
  read(_handle, &_infoheader->bn_ypelspermeter, 4);
  read(_handle, &_infoheader->bn_clrused, 4);
  read(_handle, &_infoheader->bn_clrimportant, 4);
  return TRUE;
}

/* ************************************************************************ */
static unsigned char *__GrLoadPaletteBmp ( int _handle, unsigned long _paloffset, int _colors )
/* ************************************************************************ */
{
  unsigned char *palette;
  if ( _handle == -1 ) return NULL;
  palette = (unsigned char*)malloc(_colors * 4);
  if ( !palette ) return NULL;
  lseek(_handle, SEEK_SET, _paloffset);
  read(_handle, palette, _colors * 4);
  return palette;
}

/* ************************************************************************ */
static unsigned char *GrLoadPaletteBmp ( int _handle, int *_col, GrBitmapInfoHeader* _iheader )
/* ************************************************************************ */
{
  unsigned char *palette;
  unsigned long  paloffset;
  *_col = -1;
  if (( _handle == -1 ) || ( !_iheader )) return NULL;
  palette = NULL;
  paloffset = BMPFILEHEADERSIZE + _iheader->bn_size;
  switch ( _iheader->bn_bitcount )
  {
    case 1  : { *_col = 2; palette = __GrLoadPaletteBmp(_handle, paloffset, 2); } break;
    case 4  : { *_col = 16; palette = __GrLoadPaletteBmp(_handle, paloffset, 16); } break;
    case 8  : { *_col = 256; palette = __GrLoadPaletteBmp(_handle, paloffset, 256); } break;
    case 24 : { *_col = 0; palette = NULL; } break;
    default : *_col = -1;
  }
  if (( !palette ) && ( *_col != 0 )) *_col = -1;
  return palette;
}

/* ************************************************************************ */
static char *GrLoadImageFromBmpBiRgb ( int _handle, unsigned long _offset, long _maxbufsize, int _colors,
  GrBitmapInfoHeader *_infoheader )
/* ************************************************************************ */
{
  char *map = NULL;
  char *buffer = NULL;
  unsigned long width;
  unsigned long size;
  unsigned long i;
  if (( _handle == -1 ) || ( !_infoheader ) || ( _infoheader->bn_bitcount < 1 ))
    return NULL;
  width = _infoheader->bn_width;
  size  = width;
  i     = size;
  lseek(_handle, SEEK_SET, _offset);
  if ( _infoheader->bn_bitcount == 1 )
  {
    unsigned char  bits[8], n;
    unsigned long w, bufsize;
    int   j, k;
    char *runmap;
    _maxbufsize = _maxbufsize * 8;
    map = (char *)malloc(_maxbufsize);
    bufsize = (unsigned long)ceil((float)width / 8);
    buffer = (char *)malloc(bufsize);
    runmap = NULL;
    if ( !map || !buffer )
    {
      if (map)    free(map);
      if (buffer) free(buffer);
      return NULL;
    }
    while ( i <= _maxbufsize )
    {
      read(_handle, buffer, bufsize);
      runmap = &map[_maxbufsize - i];
      for ( w = 0; w < width; w++)
      {
	j = w % 8;
	if ( !j )
	     {
	       n = buffer[w / 8];
	  for ( k = 0; k < 8; k++ )
	  {
		 bits[7 - k] = n & 1;
		 n = n >> 1;
	       }
	}
	runmap[w] = bits[j];
      }
      i += size;
    }
  }
  if ( _infoheader->bn_bitcount == 4 )
  {
    unsigned long bufsize;
    char *runmap;
    unsigned char  bits[2], n;
    unsigned long w;
    int   j, q;
    _maxbufsize = _maxbufsize * 2;
    map = (char *)malloc(_maxbufsize);
    bufsize = (unsigned long)ceil((float)width / 2);
    buffer = (char*)malloc(bufsize);
    runmap = NULL;
    if ( !map || !buffer )
    {
      if (map)    free(map);
      if (buffer) free(buffer);
      return NULL;
    }
    while ( i <= _maxbufsize )
    {
      read(_handle, buffer, bufsize);
      runmap = &map[_maxbufsize - i];
      for ( w = 0; w < width; w++)
      {
	j = w % 2;
	if ( !j )
	{
	       n = buffer[w / 2];
	       q = n & 255;
	       bits[1] = q & 15;
	       q = q >> 4;
	       bits[0] = q & 15;
	       n = n >> 8;
	}
	runmap[w] = bits[j];
      }
      i += size;
    }
  }
  if ( _infoheader->bn_bitcount == 8 )
  {
    unsigned long bufsize;
    map = (char*)malloc(_maxbufsize);
    bufsize = size;
    if ( !map ) return NULL;
    while ( i <= _maxbufsize )
    {
      read(_handle, &map[_maxbufsize - i], bufsize);
      i += bufsize;
    }
  }
  if ( _infoheader->bn_bitcount == 24 )
  {
  }
  if (buffer) free(buffer);
  return map;
}

/* ************************************************************************ */
static char *GrLoadImageFromBmpBiRle8 ( int _handle, unsigned long _offset, unsigned long _maxbufsize, int _colors,
  GrBitmapInfoHeader *_infoheader )
/* ************************************************************************ */
{
  return NULL; /* this version not contains Rle8 yet */
}

/* ************************************************************************ */
static char *GrLoadImageFromBmpBiRle4 ( int _handle, unsigned long _offset, unsigned long _maxbufsize, int _colors,
  GrBitmapInfoHeader *_infoheader )
/* ************************************************************************ */
{
  return NULL; /* this version not contains Rle4 yet */
}

/* ************************************************************************ */
static char *GrLoadImageFromBmp ( int _handle, unsigned long _offset, int _colors, GrBitmapInfoHeader *_infoheader )
/* ************************************************************************ */
{
  char* map;
  int maxbufsize;
  if (( _handle == -1 ) || ( !_infoheader )) return NULL;
  map = NULL;
  maxbufsize = _infoheader->bn_sizeimage;
  switch ( _infoheader->bn_compression ) {
    case BI_RGB  :
      {
	if ( !maxbufsize )
	  maxbufsize = _infoheader->bn_width * _infoheader->bn_height;
	map = GrLoadImageFromBmpBiRgb(_handle, _offset, maxbufsize, _colors, _infoheader); break;
      }
    case BI_RLE8 :
      map = GrLoadImageFromBmpBiRle8(_handle, _offset, maxbufsize, _colors, _infoheader); break;
    case BI_RLE4 :
      map = GrLoadImageFromBmpBiRle4(_handle, _offset, maxbufsize, _colors, _infoheader); break;
  }
  return map;
}

/*====++====================================================================*/
/*                          EXPORTED FUNCTIONS                              */
/*==++======================================================================*/

/* ************************************************************************ */
int  GrFreeBmpImageColors ( GrBmpImageColors *_pal )
/* ************************************************************************ */
{
  if (( !_pal ) || ( !_pal->bp_colormap )) return FALSE;
  if ( _pal->bp_palette )
  {
    int i;
    GrColor *colors = _pal->bp_colormap;
    colors[0] = _pal->bp_numcolors;
    for ( i = 0; i < _pal->bp_numcolors; i++ )
      GrFreeColor(colors[i+1]);
    free(_pal->bp_palette);
    _pal->bp_palette = NULL;
    _pal->bp_numcolors = 0;
    return TRUE;
  }
  return FALSE;
}

/* ************************************************************************ */
int  GrAllocBmpImageColors ( GrBmpImage *_bmp, GrBmpImageColors *_pal )
/* ************************************************************************ */
{
  if (( !_bmp ) || ( _bmp->bi_colormap != NULL ) || (_bmp->bi_numcolors < 2 ))
    return FALSE;
  _bmp->bi_erasepalette = TRUE;
  if ( _bmp->bi_palette )
  {
    int i;
    GrColor *colors = malloc(sizeof(GrColor)*(_bmp->bi_numcolors+1));
    if ( !colors ) return FALSE;
    colors[0] = _bmp->bi_numcolors;
    for ( i = 0; i < _bmp->bi_numcolors; i++ )
      colors[i+1] = GrAllocColor(_bmp->bi_palette[i*4+2], _bmp->bi_palette[i*4+1], _bmp->bi_palette[i*4+0]);
    _bmp->bi_colormap = colors;
    if ( _pal )
    {
      _bmp->bi_erasepalette = FALSE;
      memcpy(_pal,_bmp->bi_bmpimagecolors,sizeof(GrBmpImageColors));
      _bmp->bi_palette  = NULL;
      _bmp->bi_numcolors = 0;
    }
    return TRUE;
  }
  return FALSE;
}

/* ************************************************************************ */
GrBmpImage *GrLoadBmpImage ( char *_filename )
/* ************************************************************************ */
{
  #define defClose {              \
    close(handle);                \
    if (bmpimage) free(bmpimage); \
    return NULL;                  \
  }
  #define ADD2PTR(p,o) ((void *) ((char *)(p)+(o)) )
  int handle;
  GrBmpImage *bmpimage = NULL;
  if ( (handle = open(_filename, BIN_RD)) != -1 ) {
    bmpimage = malloc( sizeof(GrBmpImage)
		      +sizeof(GrBitmapFileHeader)
		      +sizeof(GrBitmapInfoHeader)
		      +sizeof(GrBmpImageColors));
    if ( !bmpimage ) defClose;
    memset(bmpimage, 0, sizeof(GrBmpImage)
		       +sizeof(GrBitmapFileHeader)
		       +sizeof(GrBitmapInfoHeader)
		       +sizeof(GrBmpImageColors));
    bmpimage->bi_bmpfileheader   = ADD2PTR(bmpimage, sizeof(GrBmpImage));
    bmpimage->bi_bmpinfoheader   = ADD2PTR(bmpimage, sizeof(GrBmpImage)
						 +sizeof(GrBitmapFileHeader));
    bmpimage->bi_bmpimagecolors = ADD2PTR(bmpimage, sizeof(GrBmpImage)
						 +sizeof(GrBitmapFileHeader)
						 +sizeof(GrBitmapInfoHeader));
    bmpimage->bi_erasepalette = TRUE;
    if ( !GrLoadBmpFileHeader(handle, bmpimage->bi_bmpfileheader) )
      defClose;
    if ( bmpimage->bi_bmpfileheader->bf_type != 19778 ) /* MAGIC NUMBER */
      defClose;
    if ( !GrLoadBmpInfoHeader(handle, bmpimage->bi_bmpinfoheader) )
      defClose;
    bmpimage->bi_palette = GrLoadPaletteBmp(handle, &(bmpimage->bi_numcolors), bmpimage->bi_bmpinfoheader);
    if ( bmpimage->bi_numcolors == -1 )
      defClose;
    bmpimage->bi_map = GrLoadImageFromBmp(handle, bmpimage->bi_bmpfileheader->bf_offbits - BMPFILEHEADERSIZE, bmpimage->bi_numcolors, bmpimage->bi_bmpinfoheader);
    if ( !bmpimage->bi_map )
      defClose;
  }
  #undef defClose
  return bmpimage;
}

/* ************************************************************************ */
GrPattern *GrConvertBmpImageToPattern ( GrBmpImage *_bmp )
/* ************************************************************************ */
{
  if (( !_bmp ) || ( !_bmp->bi_map )) return NULL;
  return GrBuildPixmap(_bmp->bi_map, _bmp->bi_width, _bmp->bi_height, _bmp->bi_colormap);
}

/* ************************************************************************ */
GrPattern *GrConvertBmpImageToStaticPattern ( GrBmpImage *_bmp )
/* ************************************************************************ */
{
  GrPattern *p = NULL;
  if ( _bmp && _bmp->bi_map )
  {
    p = GrBuildPixmap(_bmp->bi_map, _bmp->bi_width, _bmp->bi_height, _bmp->bi_colormap);
    if ( p ) GrUnloadBmpImage(_bmp);
  }
  return p;
}

/* ************************************************************************ */
void  GrUnloadBmpImage ( GrBmpImage *_bmp )
/* ************************************************************************ */
{
  if ( !_bmp ) return;
  if ( _bmp->bi_erasepalette )
    GrFreeBmpImageColors(_bmp->bi_bmpimagecolors);
  _bmp->bi_palette = NULL;
  _bmp->bi_numcolors = 0;
  if ( _bmp->bi_map ) free(_bmp->bi_map);
  free(_bmp);
  _bmp = NULL;
}

/* ************************************************************************ */
int  GrSaveBmpImage ( char *_filename, GrContext *_c, int _x1, int _y1, int _x2, int _y2 )
/* ************************************************************************ */
{
  int handle;
  unsigned long width, height;
  unsigned char palette[256*4];
  int r, g, b;
  char* line;
  unsigned long yy, xx;
  GrColor pixcol;
  GrBitmapFileHeader fileheader;
  GrBitmapInfoHeader infoheader;
  GrColor colors, i;
  GrContext safe;

  if ( !_c ) _c = (GrContext *)GrCurrentContext();

/*
  handle = creat(_filename, S_IWRITE);
  if ( handle < 0 )
  {
    close(handle);
    return FALSE;
  }
*/
  handle = open(_filename, BIN_CREAT, CREAT_PERM);
  if ( handle < 0 ) return FALSE;

  clip_box_(_c, _x1, _y1, _x2, _y2, CLIP_EMPTY_MACRO_ARG, CLIP_EMPTY_MACRO_ARG);

  width = _x2 - _x1;
  height = _y2 - _y1;

  safe = *GrCurrentContext();
  GrSetContext(_c);
  colors = GrNumColors();
  GrSetContext(&safe);

  if ( width % 4 ) width += 4 - (width % 4);

  /*=========    FILEHEADER    =========*/
  fileheader.bf_type      = 19778;
  if ( colors == 256 )
    fileheader.bf_size = BMPINFOHEADERSIZE + BMPFILEHEADERSIZE + 256*4 + width*height;
  else
    fileheader.bf_size = BMPINFOHEADERSIZE + BMPFILEHEADERSIZE + (width*height*3);
  fileheader.bf_reserved1 = 0;
  fileheader.bf_reserved2 = 0;
  if ( colors == 256 )
    fileheader.bf_offbits = BMPINFOHEADERSIZE + BMPFILEHEADERSIZE + 256*4;
  else
    fileheader.bf_offbits = BMPINFOHEADERSIZE + BMPFILEHEADERSIZE;

  /*=========    INFOHEADER    =========*/
  infoheader.bn_size   = BMPINFOHEADERSIZE;
  infoheader.bn_width  = width;
  infoheader.bn_height = height;
  infoheader.bn_planes = 1;
  infoheader.bn_bitcount = ( colors == 256 ) ? 8 : 24;
  infoheader.bn_compression = BI_RGB;
  infoheader.bn_sizeimage = width*height*(infoheader.bn_bitcount / 8);
  infoheader.bn_xpelspermeter = 0L;
  infoheader.bn_ypelspermeter = 0L;
  infoheader.bn_clrused = 0L;
  infoheader.bn_clrimportant = 0L;

  /*=========      PALETTE     =========*/
  if ( colors == 256 )
  {
    for ( i = 0; i < colors; i++ )
    {
      GrQueryColor(i, &r, &g, &b);
      palette[(i*4)]   = (unsigned char)b;
      palette[(i*4)+1] = (unsigned char)g;
      palette[(i*4)+2] = (unsigned char)r;
      palette[(i*4)+3] = 0;
    }
  }

  line = (char *)malloc(width*(infoheader.bn_bitcount / 8));
  if ( !line )
  {
    close(handle);
    return FALSE;
  }
  /*========= WRITE FILEHEADER =========*/
  write(handle, &fileheader.bf_type, 2);
  write(handle, &fileheader.bf_size, 4);
  write(handle, &fileheader.bf_reserved1, 2);
  write(handle, &fileheader.bf_reserved2, 2);
  write(handle, &fileheader.bf_offbits, 4);

  /*========= WRITE INFOHEADER =========*/
  write(handle, &infoheader.bn_size, 4);
  write(handle, &infoheader.bn_width, 4);
  write(handle, &infoheader.bn_height, 4);
  write(handle, &infoheader.bn_planes, 2);
  write(handle, &infoheader.bn_bitcount, 2);
  write(handle, &infoheader.bn_compression, 4);
  write(handle, &infoheader.bn_sizeimage, 4);
  write(handle, &infoheader.bn_xpelspermeter, 4);
  write(handle, &infoheader.bn_ypelspermeter, 4);
  write(handle, &infoheader.bn_clrused, 4);
  write(handle, &infoheader.bn_clrimportant, 4);

  /*=========   WRITE PALETTE  =========*/
  if ( colors == 256 ) write(handle, palette, 256*4);

  /*=========     WRITE MAP    =========*/
  yy = height;
  do {
    xx = 0;
    do {
      pixcol = GrPixelC(_c,_x1+xx,_y1+yy);
      if ( colors == 256 ) line[xx] = pixcol;
      else
      {
	line[(xx*3)+0] = GrRGBcolorBlue(pixcol);
	line[(xx*3)+1] = GrRGBcolorGreen(pixcol);;
	line[(xx*3)+2] = GrRGBcolorRed(pixcol);;
      }
    } while(++xx < width);
    write(handle, line, width*(infoheader.bn_bitcount / 8));
  } while(--yy > 0);
  free((void *)line);
  close(handle);
  return TRUE;
}

/* ************************************************************************ */
unsigned long  GrBmpImageWidth ( GrBmpImage* _bmp )
/* ************************************************************************ */
{
  return ( _bmp && _bmp->bi_bmpinfoheader ) ? _bmp->bi_width : 0L;
}

/* ************************************************************************ */
unsigned long  GrBmpImageHeight ( GrBmpImage* _bmp )
/* ************************************************************************ */
{
  return ( _bmp && _bmp->bi_bmpinfoheader ) ? _bmp->bi_height : 0L;
}

/* ************************************************************************ */
char  *GrBmpImagePalette ( GrBmpImage* _bmp )
/* ************************************************************************ */
{
  return (char *)(( _bmp && _bmp->bi_bmpimagecolors ) ? _bmp->bi_palette : NULL);
}

/* ************************************************************************ */
GrColor  *GrBmpImageColorMap ( GrBmpImage* _bmp )
/* ************************************************************************ */
{
  return ( _bmp && _bmp->bi_bmpimagecolors ) ? _bmp->bi_colormap : NULL;
}

/* ************************************************************************ */
GrColor  GrGetBmpImageNumColors ( GrBmpImage* _bmp )
/* ************************************************************************ */
{
  return ( _bmp && _bmp->bi_bmpimagecolors ) ? _bmp->bi_numcolors : 0;
}

