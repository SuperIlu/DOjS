{$X+}

program Text_Test;

uses GRX;

procedure TestFunc;
var
  x, y, ww, wh, ii, jj, c : Integer;

begin
   x := GrSizeX;
   y := GrSizeY;
   ww := round((x-10)/32);
   wh := round((y-10)/8);

   GrSetRGBcolorMode;
   for ii := 0 to 7 do
     for jj := 0 to 31 do begin
       c := ii*32+jj;
       {gives the same color independently of BPP: not all drivers have good BPP=8}
       c := GrAllocColor(c and 2#11100000,(c and 2#11100) shl 3, (c and 2#11) shl 6);
       GrFilledBox(5+jj*ww,5+ii*wh,5+jj*ww+ww-1,5+ii*wh+wh-1,c);
     end;
end; { TestFunc }

var
   x, y, xv, yv, c,
   m : Integer;
   St : String(255);
   o  : GrTextOption;

begin
   x  :=  640;
   y  :=  480;
   xv :=  640;
   yv :=  480;
   c  :=  65536;

   m := GrSetMode(Gr_Width_Height_Color_Graphics,x,y,c,xv,yv);
   { M := GrSetMode(Gr_Custom_Graphics,x,y,c,xv,yv); }

   TestFunc;
   GrCircle(400,400,200,GrWhite);
   GrCircle(400,400,205,GrWhite);
   GrLineNC(0, 0, GrScreenX-1, GrScreenY-1, GrWhite);
   GrLineNC(0, GrScreenY-1, GrScreenX-1, 0, GrWhite);

   { o.txo_font    := @GrFont_PC8x16; }
   o.txo_Font    := GrLoadFont('pc8x16.fnt');
   o.txo_FgColor.v := GrBlack;
   o.txo_BgColor.v := GrWhite;
   o.txo_ChrType := Gr_Byte_Text;
   o.txo_Direct  := Gr_Text_Default;
   o.txo_XAlign  := Gr_Align_Default;
   o.txo_YAlign  := Gr_Align_Default;

   GrDrawString('This is simple text', 16, 20, 20, o);
   St := 'This text is more complex';
   o.txo_Font    := @GrFont_PC8x8;
   GrDrawString(St, Length(St), 20, 50, o);
   GrTextXY(20, 80, 'This is another text', GrBlack, GrWhite);

   o.txo_Font    := GrLoadFont('helv22.fnt');
   GrDrawString(St, Length(St), 20, 100, o);

   { GrLoadConvertedFont(var name: Char; cvt, w, h, minch, maxch: Integer): GrFontPt;}
   o.txo_Font    := GrLoadConvertedFont('helv22.fnt',
                  { Gr_FontCvt_None       =  0;
                    Gr_FontCvt_Skipchars  =  1;
                    Gr_FontCvt_Resize     =  2;
                    Gr_FontCvt_Italicize  =  4;
                    Gr_FontCvt_Boldify  =  8;
                    Gr_FontCvt_Fixify     = 16;
                    Gr_FontCvt_Proportion = 32; }
                    Gr_FontCvt_Resize     or
                    Gr_FontCvt_Italicize,
                    20, 30,
                    32, 127);
   GrDrawString(St, Length(St), 20, 130, o);

   GrKeyRead;
end.
