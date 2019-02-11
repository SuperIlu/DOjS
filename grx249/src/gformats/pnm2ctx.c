/**
 ** pnm2ctx.c ---- loads a context from a PNM file or PNM buffer
 **
 ** Copyright (C) 2000,2001 Mariano Alvarez Fernandez
 ** [e-mail: malfer@teleline.es]
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
 ** Contributions by Josu Onandia (jonandia@fagorautomation.es) 10/03/2001
 **   _GrLoadContextFromPpm optimized (applied to Pbm and Pgm too)
 **
 **/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "grx20.h"

typedef struct{
  int method;  /* 0=file, 1=buffer */
  FILE *file;
  const char *buffer;
  int bufferpointer;
  } inputstruct;

/**/

static size_t inputread( void *buffer, size_t size, size_t number,
                         inputstruct *is )
{
  if( is->method == 0 )
    return fread( buffer,size,number,is->file );
  else{
    memcpy( buffer,&(is->buffer[is->bufferpointer]),size*number );
    is->bufferpointer += size * number;
    return number;
    }
}

/**/

static int inputgetc( inputstruct *is )
{
  if( is->method == 0 )
    return fgetc( is->file );
  else
    return is->buffer[is->bufferpointer++];
}

/**/

int inputungetc( int c, inputstruct *is )
{
  if( is->method == 0 )
    return ungetc( c,is->file );
  else{
    is->bufferpointer--;
    return c;
    }
}

/**/

static int skipspaces( inputstruct *is )
{
  int c;
  
  while( 1 ){
    if( (c = inputgetc( is )) == EOF ) return -1;
    if( c == '#' ) // it's a comment
      while( 1 ){
        if( (c = inputgetc( is )) == EOF ) return -1;
        if( c == '\n' ) break;
        }
    if( c != ' ' && c != '\t' && c != '\n' && c != '\r' ){
      inputungetc( c,is );
      return 0;
      }
    }
}

/**/

static int readnumber( inputstruct *is )
{
  char buf[81];
  int count;
  
  count = 0;
  while( 1 ){
    if( (buf[count] = inputgetc( is )) == EOF ) return -1;
    if( buf[count] == ' ' || buf[count] == '\t' || buf[count] == '\n' ){
      inputungetc( buf[count],is );
      break;
      }
    if( count > 80 ) break;
    count++;
    }
  buf[count] = '\0';
  return atoi( buf );
}

/**/

static int loaddata( inputstruct *is, int *width, int *height, int *maxval )
{
  unsigned char buf[10];
  int r;

  if( inputread( buf,1,2,is ) != 2 ) return -1;
  if( buf[0] != 'P' ) return -1;
  r = buf[1] - '0';
  if( (r < PLAINPBMFORMAT) || (r > PPMFORMAT) ) return -1;
  if( skipspaces( is ) != 0 ) return -1;
  if( (*width = readnumber( is )) < 0 ) return -1;
  if( skipspaces( is ) != 0 ) return -1;
  if( (*height = readnumber( is )) < 0 ) return -1;
  if( (r == PLAINPBMFORMAT) || (r == PBMFORMAT) )
    *maxval = 1;
  else{
    if( skipspaces( is ) != 0 ) return -1;
    if( (*maxval = readnumber( is )) < 0 ) return -1;
    }
  inputgetc( is );  // skip \n
  return r;
}

/**/

static int _GrLoadContextFromPbm( inputstruct *is, int width, int height )
{
  int x, y;
  int maxwidth, maxheight;
  GrColor color;
  int currentbyte, isbyteread = 0;
  int currentbit = 0;
  GrColor *pColors=NULL;
  int res = 0;

  maxwidth = (width > GrSizeX()) ? GrSizeX() : width;
  maxheight = (height > GrSizeY()) ? GrSizeY() : height;

  pColors = malloc( maxwidth * sizeof(GrColor) );
  if(pColors == NULL) { res = -1; goto salida; }

  for( y=0; y<maxheight; y++ ){
    for( x=0; x<width; x++ ){
      if( !isbyteread ){
        if( inputread( &currentbyte,1,1,is ) != 1 ) { res = -1; goto salida; }
        isbyteread = 1;
        currentbit = 7;
        }
      if( x < maxwidth ){
        color = currentbyte & (1 << currentbit) ? GrBlack() : GrWhite();
        pColors[x] = color;
        }
      currentbit--;
      if( currentbit < 0 ) isbyteread = 0;
      }
    GrPutScanline( 0,maxwidth-1,y,pColors,GrWRITE );
    isbyteread = 0;
    }

salida:
  if( pColors != NULL ) free( pColors );
  return res;
}

/**/

static int _GrLoadContextFromPgm( inputstruct *is, int width,
                                  int height, int maxval )
{
  int x, y;
  int needcoloradjust = 0;
  int maxwidth, maxheight;
  double coloradjust = 255.0;
  GrColor *pColors=NULL;
  unsigned char *pData=NULL, *pCursor;
  int res = 0;

  maxwidth = (width > GrSizeX()) ? GrSizeX() : width;
  maxheight = (height > GrSizeY()) ? GrSizeY() : height;

  if( maxval < 255 ){
    needcoloradjust = 1;
    coloradjust = 255.0 / maxval;
    }

  pData = NULL;
  pColors = malloc( maxwidth * sizeof(GrColor) );
  if(pColors == NULL) { res = -1; goto salida; }
  pData = malloc( width * sizeof(char) );
  if(pData == NULL) { res = -1; goto salida; }

  for( y=0; y<maxheight; y++ ){
    if( inputread( pData,1,width,is ) != width ) { res = -1; goto salida; }
    pCursor = pData;
    for( x=0; x<maxwidth; x++ ){
      if( needcoloradjust )
        *pCursor *= coloradjust;
      pColors[x] = GrAllocColor( *pCursor,*pCursor,*pCursor );
      pCursor += 1;
      }
    GrPutScanline( 0,maxwidth-1,y,pColors,GrWRITE );
    }

salida:
  if( pColors != NULL ) free( pColors );
  if( pData != NULL ) free( pData );
  return res;
}

/**/

static int _GrLoadContextFromPpm( inputstruct *is, int width,
                                  int height, int maxval )
{
  int x, y;
  int needcoloradjust = 0;
  int maxwidth, maxheight;
  double coloradjust = 255.0;
  GrColor *pColors=NULL;
  unsigned char *pRGB=NULL, *pCursor;
  int res = 0;

  maxwidth = (width > GrSizeX()) ? GrSizeX() : width;
  maxheight = (height > GrSizeY()) ? GrSizeY() : height;

  if( maxval < 255 ){
    needcoloradjust = 1;
    coloradjust = 255.0 / maxval;
    }

  pRGB = NULL;
  pColors = malloc( maxwidth * sizeof(GrColor) );
  if(pColors == NULL) { res = -1; goto salida; }
  pRGB = malloc( width * 3 * sizeof(char) );
  if(pRGB == NULL) { res = -1; goto salida; }

  for( y=0; y<maxheight; y++ ){
    if( inputread( pRGB,3,width,is ) != width ) { res = -1; goto salida; }
    pCursor = pRGB;
    for( x=0; x<maxwidth; x++ ){
      if( needcoloradjust ){
        pCursor[0] *= coloradjust;
        pCursor[1] *= coloradjust;
        pCursor[2] *= coloradjust;
        }
      pColors[x] = GrAllocColor( pCursor[0],pCursor[1],pCursor[2] );
      pCursor += 3;
      }
    GrPutScanline( 0,maxwidth-1,y,pColors,GrWRITE );
    }

salida:
  if( pColors != NULL ) free( pColors );
  if( pRGB != NULL ) free( pRGB );
  return res;
}

/*
** GrLoadContextFromPnm - Load a context from a PNM file
**
** Support only PBM, PGM and PPM binary files with maxval < 256
**
** If context dimensions are lesser than pnm dimensions,
** the routine loads as much as it can
**
** If color mode is not in RGB mode, the routine allocates as
** much colors as it can
**
** Arguments:
**   ctx:   Context to be loaded (NULL -> use current context)
**   pnmfn: Name of pnm file
**
** Returns  0 on success
**         -1 on error
*/

int GrLoadContextFromPnm( GrContext *grc, char *pnmfn )
{
  inputstruct is = {0, NULL, NULL, 0};
  GrContext grcaux;
  int r = -1;
  int format, width, height, maxval;

  if( (is.file = fopen( pnmfn,"rb" )) == NULL ) return -1;

  GrSaveContext( &grcaux );
  if( grc != NULL ) GrSetContext( grc );

  format = loaddata( &is,&width,&height,&maxval );
  if( maxval > 255 ) goto ENDFUNCTION;
  if( (format < PBMFORMAT) || (format > PPMFORMAT) ) goto ENDFUNCTION;

  switch( format ){
    case PBMFORMAT: r = _GrLoadContextFromPbm( &is,width,height );
                    break;
    case PGMFORMAT: r = _GrLoadContextFromPgm( &is,width,height,maxval );
                    break;
    case PPMFORMAT: r = _GrLoadContextFromPpm( &is,width,height,maxval );
                    break;
    }

ENDFUNCTION:
  GrSetContext( &grcaux );
  fclose( is.file );

  return r;
}

/*
** GrQueryPnm - Query format, width and height data from a PNM file
**
** Arguments:
**   pnmfn:   Name of pnm file
**   width:   return pnm width
**   height:  return pnm height
**   maxval:  max color component value
**
** Returns  1 to 6 on success (PNM FORMAT)
**         -1 on error
*/

int GrQueryPnm( char *pnmfn, int *width, int *height, int *maxval )
{
  inputstruct is = {0, NULL, NULL, 0};
  int r;

  if( (is.file = fopen( pnmfn,"rb" )) == NULL ) return -1;

  r = loaddata( &is,width,height,maxval );

  fclose( is.file );

  return r;
}

/*
** GrLoadContextFromPnmBuffer - Load a context from a PNM buffer
**
** Support only PBM, PGM and PPM binary buffers with maxval < 256
**
** If context dimensions are lesser than pnm dimensions,
** the routine loads as much as it can
**
** If color mode is not in RGB mode, the routine allocates as
** much colors as it can
**
** Arguments:
**   ctx:    Context to be loaded (NULL -> use current context)
**   pnmbuf: Buffer that holds data
**
** Returns  0 on success
**         -1 on error
*/

int GrLoadContextFromPnmBuffer( GrContext *grc, const char *pnmbuf )
{
  inputstruct is = {1, NULL, NULL, 0};
  GrContext grcaux;
  int r = -1;
  int format, width, height, maxval;

  is.buffer = pnmbuf;
  
  GrSaveContext( &grcaux );
  if( grc != NULL ) GrSetContext( grc );

  format = loaddata( &is,&width,&height,&maxval );
  if( maxval > 255 ) goto ENDFUNCTION;
  if( (format < PBMFORMAT) || (format > PPMFORMAT) ) goto ENDFUNCTION;

  switch( format ){
    case PBMFORMAT: r = _GrLoadContextFromPbm( &is,width,height );
                    break;
    case PGMFORMAT: r = _GrLoadContextFromPgm( &is,width,height,maxval );
                    break;
    case PPMFORMAT: r = _GrLoadContextFromPpm( &is,width,height,maxval );
                    break;
    }

ENDFUNCTION:
  GrSetContext( &grcaux );

  return r;
}

/*
** GrQueryPnmBuffer - Query format, width and height data from a PNM buffer
**
** Arguments:
**   pnmbuf:  Buffer that holds data
**   width:   return pnm width
**   height:  return pnm height
**   maxval:  max color component value
**
** Returns  1 to 6 on success (PNM FORMAT)
**         -1 on error
*/

int GrQueryPnmBuffer( const char *pnmbuf, int *width, int *height, int *maxval )
{
  inputstruct is = {1, NULL, NULL, 0};
  int r;

  is.buffer = pnmbuf;

  r = loaddata( &is,width,height,maxval );

  return r;
}

