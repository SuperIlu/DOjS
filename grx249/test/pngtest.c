/**
 ** pngtest.c ---- test the ctx2png routines
 **
 ** Copyright (c) 2001 Mariano Alvarez Fernandez
 ** [e-mail: malfer@teleline.es]
 **
 ** This is a test/demo file of the GRX graphics library.
 ** You can use GRX test/demo files as you want.
 **
 ** The GRX graphics library is free software; you can redistribute it
 ** and/or modify it under some conditions; see the "copying.grx" file
 ** for details.
 **
 ** This library is distributed in the hope that it will be useful,
 ** but WITHOUT ANY WARRANTY; without even the implied warranty of
 ** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 **/

#include <stdio.h>
#include <stdlib.h>
#include "grx20.h"
#include "grxkeys.h"

void imagen( char *nf )
{
  GrContext *grc;
  int wide, high;
  char s[81];
  int w, h;

  GrQueryPng( nf,&w,&h );
  sprintf( s,"%s %dx%d",nf,w,h );
  wide = (w > 300) ? 300 : w;
  high = (h > 400) ? 400 : h;
  GrClearScreen( GrAllocColor( 0,0,200 ) );

  GrBox( 10,40,10+wide+1,40+high+1,GrWhite() );
  grc = GrCreateSubContext( 11,41,11+wide-1,41+high-1,NULL,NULL );
  GrLoadContextFromPng( grc,nf,0 );
  GrDestroyContext( grc );

  GrBox( 320,40,320+wide+1,40+high+1,GrWhite() );
  grc = GrCreateSubContext( 321,41,321+wide-1,41+high-1,NULL,NULL );
  GrLoadContextFromPng( grc,nf,1 );
  GrDestroyContext( grc );

  GrTextXY( 10,10,s,GrBlack(),GrWhite() );
  GrTextXY( 10,50+high,"Press any key to continue",GrBlack(),GrWhite() );
  GrKeyRead();
}

void nopngsupport( void )
{
  char *s[6] = {
    "Warning!",
    "You need libpng (http://www.libpng.org/pub/png/libpng.html)",
    "and enable png support in the GRX lib (edit makedefs.grx)",
    "to run this demo",
    " ",
    "Press any key to continue..." };
  int i;

  GrClearScreen( GrAllocColor( 0,0,100 ) );
  for( i=0; i<6; i++ )
    GrTextXY( 90,160+i*18,s[i],GrWhite(),GrNOCOLOR );
  GrKeyRead();
}


int main()
{
  GrContext *grc;

  GrSetMode( GR_width_height_bpp_graphics,640,480,24 );

  if( !GrPngSupport() ){
    nopngsupport();
    GrSetMode(GR_default_text);
    exit( 1 );
    }

  imagen( "pngcompo.png" );
  imagen( "pngowl.png" );
  imagen( "pngred.png" );

  GrClearScreen( GrAllocColor( 0,100,0 ) );
  grc = GrCreateSubContext( 191,121,191+256-1,121+240-1,NULL,NULL );
  GrLoadContextFromPng( grc,"pngred.png",1 );
  GrDestroyContext( grc );
  grc = GrCreateSubContext( 181,241,181+289-1,241+80-1,NULL,NULL );
  GrLoadContextFromPng( grc,"pngcompo.png",1 );
  GrDestroyContext( grc );

  GrTextXY( 10,10,"Press any key to save screen",GrBlack(),GrWhite() );
  GrKeyRead();
  GrSaveContextToPng( NULL,"output.png" );

  GrClearScreen( GrBlack() );
  GrTextXY( 10,10,"Press any key to reload screen",GrBlack(),GrWhite() );
  GrKeyRead();
  GrLoadContextFromPng( NULL,"output.png",0 );

  GrTextXY( 10,10,"Press any key to end          ",GrBlack(),GrWhite() );
  GrKeyRead();
  GrSetMode(GR_default_text);
  return 0;
}
