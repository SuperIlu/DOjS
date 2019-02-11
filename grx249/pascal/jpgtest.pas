(**
 ** jpgtest.pas ---- test the ctx2jpeg routines
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
 **)

{$X+}

program jpgtest;

uses GPC, GRX;

var grc: GrContextPtr;

procedure imagen( nf: String; Scale: Integer );
var
  grc        : GrContextPtr;
  Width, Height : Integer;
  s          : String[81];
  w, h       : CInteger;
begin
  GrQueryJpeg( nf,w,h );
  WriteStr( s,nf,' ',w,' ',h,' Scale 1/',Scale );
  Width := min(600 , w div Scale);
  Height := min(400 , h div Scale);
  GrClearScreen( GrAllocColor( 0,0,200 ) );

  GrBox( 10,40,10+Width+1,40+Height+1,GrWhite );
  grc := GrCreateSubContext( 11,41,11+Width-1,41+Height-1,NIL,NIL );
  GrLoadContextFromJpeg( grc,nf,Scale );
  GrDestroyContext( grc );

  GrTextXY( 10,10,s,GrBlack,GrWhite );
  GrTextXY( 10,50+Height,'Press any key to continue',GrBlack,GrWhite );
  GrKeyRead
end;

procedure nojpegsupport;
const
  s: array[0..5] of String[50] = (
       'Warning!',
       'You need libjpeg (http://www.ijg.org) and enable',
       'jpeg support in the GRX lib (edit makedefs.grx)',
       'to run this demo',
       ' ',
       'Press any key to continue...' );
var
  i: Integer;
begin
  GrClearScreen( GrAllocColor( 0,0,100 ) );
  for i:=0 to 5 do
    GrTextXY( 90,160+i*18,s[i],GrWhite,GrNoColor);
  GrKeyRead
end;

begin
   GrSetMode( Gr_Width_Height_BPP_Graphics,640,480,24,0,0 );

   if GrJpegSupport = 0 then begin
      nojpegsupport;
      GrSetMode(Gr_Default_Text,0,0,0,0,0);
      halt( 1 );
   end;

   imagen( '../test/jpeg1.jpg',1 );
   imagen( '../test/jpeg1.jpg',2 );
   imagen( '../test/jpeg1.jpg',4 );
   imagen( '../test/jpeg1.jpg',8 );
   imagen( '../test/jpeg2.jpg',1 );
   imagen( '../test/jpeg2.jpg',2 );
   imagen( '../test/jpeg2.jpg',4 );
   imagen( '../test/jpeg2.jpg',8 );

   GrClearScreen( GrAllocColor( 0,100,0 ) );
   grc := GrCreateSubContext( 10,40,10+400-1,40+300-1,NIL,NIL );
   GrLoadContextFromJpeg( grc,'../test/jpeg1.jpg',2 );
   GrDestroyContext( grc );
   grc := GrCreateSubContext( 210,150,210+400-1,150+300-1,NIL,NIL );
   GrLoadContextFromJpeg( grc,'../test/jpeg2.jpg',2 );
   GrDestroyContext( grc );

   GrTextXY( 10,10,'Press any key to save color and gray screen',GrBlack,GrWhite );
   GrKeyRead;

   GrSaveContextToJpeg( NIL,'p.jpg',75 );
   GrSaveContextToGrayJpeg( NIL,'pgray.jpg',75 );

   GrClearScreen( GrBlack );
   GrTextXY( 10,10,'Press any key to reload color screen       ',GrBlack,GrWhite );
   GrKeyRead;
   GrLoadContextFromJpeg( NIL,'p.jpg',1 );

   GrTextXY( 10,10,'Press any key to reload gray screen        ',GrBlack,GrWhite );
   GrKeyRead;
   GrClearScreen(GrBlack);
   GrLoadContextFromJpeg( NIL,'pgray.jpg',1 );

   GrTextXY( 10,10,'Press any key to end                       ',GrBlack,GrWhite );
   GrKeyRead;

   GrSetMode(Gr_Default_Text,0,0,0,0,0)
end.
