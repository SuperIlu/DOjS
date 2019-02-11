/**
 ** dummyjpg.c ---- dummy jpeg funtions if not JPEG support in the library
 **
 ** Copyright (C) 2001 Mariano Alvarez Fernandez
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
 **/

#include <stdio.h>
#include <stdlib.h>
#include "grx20.h"

/*
** GrJpegSupport - Returns false
*/

int GrJpegSupport( void )
{
  return 0;
}

/*
** GrSaveContextToJpeg - Returns error
*/

int GrSaveContextToJpeg( GrContext *grc, char *jpegfn, int quality )
{
  return -1;
}

/*
** GrSaveContextToGrayJpeg - Returns error
*/

int GrSaveContextToGrayJpeg( GrContext *grc, char *jpegfn, int quality )
{
  return -1;
}

/*
** GrLoadContextFromJpeg - Returns error
*/

int GrLoadContextFromJpeg( GrContext *grc, char *jpegfn, int scale )
{
  return -1;
}

/*
** GrQueryJpeg - Returns error
*/

int GrQueryJpeg( char *jpegfn, int *width, int *height )
{
  return -1;
}
