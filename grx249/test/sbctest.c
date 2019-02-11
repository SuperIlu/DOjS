/**
 ** sbctest.c ---- test subcontext
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
 **
 **/

#include <string.h>
#include "test.h"

static void drawpf( int border, GrPattern *pat );
static void drawp( int border, GrLinePattern *grlp );

TESTFUNC(sbctest)
{
  char bits[] = {0, 76, 50, 0, 0, 76, 60, 0};
  GrContext *grc, *grsc;
  GrPattern *pat1, *pat2;
  GrLineOption grl;
  GrLinePattern grlp;
  GrFont *grf;
  GrTextOption grt;

  grc = GrCreateContext( 300,300,NULL,NULL );
  if( grc == NULL ) return;
  grsc = GrCreateSubContext( 10,10,290,290,grc,NULL );
  if( grsc == NULL ) return;
  pat1 = GrBuildPixmapFromBits( bits,8,8,GrWhite(),GrBlack() );
  if( pat1 == NULL ) return;
  pat2 = GrBuildPixmapFromBits( bits,8,8,GrBlack(),GrWhite() );
  if( pat2 == NULL ) return;
  grf = GrLoadFont( "lucb40.fnt" );
  if( grf == NULL ){
    grf = GrLoadFont( "../fonts/lucb40.fnt" );
    if( grf == NULL ) return;
    }

  GrBox( 19,19,320,320,GrWhite() );

  GrTextXY( 0,0,"White drawing on context       ",GrWhite(),GrBlack() );
  GrSetContext( grc );
  GrClearContext( GrBlack() );
  drawing( 10,10,280,280,GrWhite(),GrNOCOLOR );
  GrSetContext( NULL );
  GrBitBlt( NULL,20,20,grc,0,0,299,299,GrWRITE );
  GrKeyRead();

  GrTextXY( 0,0,"Black drawing on subcontext    ",GrWhite(),GrBlack() );
  GrSetContext( grsc );
  drawing( 0,0,280,280,GrBlack(),GrNOCOLOR );
  GrSetContext( NULL );
  GrBitBlt( NULL,20,20,grc,0,0,299,299,GrWRITE );
  GrKeyRead();

  GrTextXY( 0,0,"Pattern drawing on context     ",GrWhite(),GrBlack() );
  GrSetContext( grc );
  GrClearContext( GrBlack() );
  drawpf( 10,pat1 );
  GrSetContext( NULL );
  GrBitBlt( NULL,20,20,grc,0,0,299,299,GrWRITE );
  GrKeyRead();

  GrTextXY( 0,0,"Pattern drawing on subcontext  ",GrWhite(),GrBlack() );
  GrSetContext( grsc );
  GrClearContext( GrBlack() );
  drawpf( 0,pat2 );
  GrSetContext( NULL );
  GrBitBlt( NULL,20,20,grc,0,0,299,299,GrXOR );
  GrKeyRead();

  grl.lno_color = GrWhite();
  grl.lno_width = 3;
  grl.lno_pattlen = 0;
  grlp.lnp_pattern = pat1;
  grlp.lnp_option = &grl;

  GrTextXY( 0,0,"Patterned drawing on context   ",GrWhite(),GrBlack() );
  GrSetContext( grc );
  GrClearContext( GrBlack() );
  grlp.lnp_pattern = pat1;
  drawp( 10,&grlp );
  GrSetContext( NULL );
  GrBitBlt( NULL,20,20,grc,0,0,299,299,GrWRITE );
  GrKeyRead();

  GrTextXY( 0,0,"Patterned drawing on subcontext",GrWhite(),GrBlack() );
  GrSetContext( grsc );
  GrClearContext( GrBlack() );
  grlp.lnp_pattern = pat2;
  drawp( 0,&grlp );
  GrSetContext( NULL );
  GrBitBlt( NULL,20,20,grc,0,0,299,299,GrXOR );
  GrKeyRead();

  grt.txo_fgcolor.v = GrWhite();
  grt.txo_bgcolor.v = GrBlack() | GrOR;
  grt.txo_font = grf;
  grt.txo_direct = GR_TEXT_RIGHT;
  grt.txo_xalign = GR_ALIGN_LEFT;
  grt.txo_yalign = GR_ALIGN_CENTER;
  grt.txo_chrtype = GR_BYTE_TEXT;

  GrTextXY( 0,0,"Patterned text on context      ",GrWhite(),GrBlack() );
  GrSetContext( grc );
  GrClearContext( GrBlack() );
  GrPatternDrawString( "Hello all",9,20,60,&grt,pat1 );
  GrPatternDrawChar( 'G',20,120,&grt,pat1 );
  GrPatternDrawStringExt( "Hola a todos",12,20,180,&grt,pat1 );
  GrSetContext( NULL );
  GrBitBlt( NULL,20,20,grc,0,0,299,299,GrWRITE );
  GrKeyRead();

  GrTextXY( 0,0,"Patterned text on subcontext   ",GrWhite(),GrBlack() );
  GrSetContext( grsc );
  GrClearContext( GrBlack() );
  GrPatternDrawString( "Hello all",9,10,50,&grt,pat2 );
  GrPatternDrawChar( 'G',10,110,&grt,pat2 );
  GrPatternDrawStringExt( "Hola a todos",12,10,170,&grt,pat2 );
  GrSetContext( NULL );
  GrBitBlt( NULL,20,20,grc,0,0,299,299,GrXOR );
  GrKeyRead();

  GrUnloadFont( grf );
  GrDestroyPattern( pat2 );
  GrDestroyPattern( pat1 );
  GrDestroyContext( grsc );
  GrDestroyContext( grc );
}

/***/

static void drawpf( int border, GrPattern *pat )
{
  int pt1[4][2] = {{130,200},{140,240},{150,250},{160,180}};
  int pt2[4][2] = {{230,200},{235,240},{246,250},{258,180}};
  int ptaux[4][2];
  int i,j;
  
  GrPatternFilledBox( 0+border,0+border,93+border,93+border,pat );
  GrPatternFilledCircle( 139+border,46+border,45,pat );
  GrPatternFilledEllipse( 232+border,46+border,45,35,pat );
  GrPatternFilledCircleArc( 46+border,139+border,45,-300,600,
                            GR_ARC_STYLE_CLOSE2,pat );
  GrPatternFilledEllipseArc( 139+border,139+border,45,35,-700,400,
                             GR_ARC_STYLE_CLOSE2,pat );
  GrPatternFilledLine( 188+border,139+border,278+border,139+border,pat );
  GrPatternFilledPlot( 47+border,228+border,pat );
  GrPatternFilledPlot( 47+border,229+border,pat );
  GrPatternFilledPlot( 47+border,230+border,pat );
  GrPatternFilledPlot( 47+border,231+border,pat );
  GrPatternFilledPlot( 47+border,232+border,pat );
  for( i=0; i<4; i++ )
    for( j=0; j<2; j++ )
      ptaux[i][j] = pt1[i][j] + border;
  GrPatternFilledPolygon( 4,ptaux,pat );
  for( i=0; i<4; i++ )
    for( j=0; j<2; j++ )
      ptaux[i][j] = pt2[i][j] + border;
  GrPatternFilledConvexPolygon( 4,ptaux,pat );
}

/***/

static void drawp( int border, GrLinePattern *grlp )
{
  int pt1[4][2] = {{130,200},{140,240},{150,250},{160,180}};
  int pt2[4][2] = {{230,200},{235,240},{246,250},{258,180}};
  int ptaux[4][2];
  int i,j;
  
  GrPatternedBox( 0+border,0+border,93+border,93+border,grlp );
  GrPatternedCircle( 139+border,46+border,45,grlp );
  GrPatternedEllipse( 232+border,46+border,45,35,grlp );
  GrPatternedCircleArc( 46+border,139+border,45,-300,600,
                        GR_ARC_STYLE_CLOSE2,grlp );
  GrPatternedEllipseArc( 139+border,139+border,45,35,-700,400,
                         GR_ARC_STYLE_CLOSE2,grlp );
  GrPatternedLine( 188+border,139+border,278+border,139+border,grlp );
  for( i=0; i<4; i++ )
    for( j=0; j<2; j++ )
      ptaux[i][j] = pt1[i][j] + border;
  GrPatternedPolygon( 4,ptaux,grlp );
  for( i=0; i<4; i++ )
    for( j=0; j<2; j++ )
      ptaux[i][j] = pt2[i][j] + border;
  GrPatternedPolyLine( 4,ptaux,grlp );
}

