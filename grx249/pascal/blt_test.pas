{$X+}

Program Blt_Test;

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
   x, y, xv, yv, BPP,
   m : Integer;

begin
   x  :=  1000;
   y  :=  1000;
   xv :=  1280;
   yv :=  1024;
   BPP:=  24;

(* m := GrSetMode(Gr_Width_Height_BPP_Graphics,x,y,bpp,0,0);  *)
   m := GrSetMode(Gr_Custom_BPP_Graphics,x,y,BPP,xv,yv);

   TestFunc;
   GrCircle(400,400,200,GrWhite);
   GrCircle(400,400,205,GrWhite);
   GrLineNC(0, 0, GrScreenX-1, GrScreenY-1, GrWhite);
   GrLineNC(0, GrScreenY-1, GrScreenX-1, 0, GrWhite);
   GrKeyRead;
   GrTextXY(0,0,'GrScreenContext',GrWhite,GrBlack);
   GrBitBltNC(GrScreenContext, 200, 200, GrScreenContext, 0, 0, 200, 200, GrWrite);
   GrKeyRead;
   GrBitBltNC(GrScreenContext, 300, 300, GrScreenContext, 0, 0, 200, 200, GrOr);
   GrKeyRead;
   GrBitBltNC(GrScreenContext, 400, 400, GrScreenContext, 0, 0, 200, 200, GrAnd);
   GrKeyRead;
   GrBitBltNC(GrScreenContext, 500, 500, GrScreenContext, 0, 0, 200, 200, GrXor);
   GrKeyRead;
   GrBitBltNC(GrScreenContext, 600, 600, GrScreenContext, 0, 0, 200, 200, GrImage);
   GrKeyRead;
end.

