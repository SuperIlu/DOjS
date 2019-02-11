(**
 ** polytest.pas ---- test polygon rendering
 **
 ** Copyright (c) 1995 Csaba Biegl, 820 Stirrup Dr, Nashville, TN 37221
 ** [e-mail: csaba@vuse.vanderbilt.edu]
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
 **)

{$R-}

program polytest;

uses GPC, GRX, Test;

type WrkString       = String[80];

var  f               : Text;
     Line,msg        : WrkString;
     nb              : Integer;
     convex,collect  : boolean;
     k               : GrKeyType;
     polygon         : array [0..300] of PointType;
     pEGA            : GrColorsPtr;
     black,white,red : GrColor;

procedure TestPoly(n:Integer; var Points: array of PointType; convex:boolean);
begin
   GrClearScreen(black);
   GrPolygon(n,Points,white);
   GrFilledPolygon(n,Points,(red or GrXor));
   k:=GrKeyRead;
   if convex or (n <= 3) then begin
      GrClearScreen(black);
      GrFilledPolygon(n,Points,white);
      GrFilledConvexPolygon(n,Points,(red or GrXor));
      k:=GrKeyRead
   end
end;

procedure SpeedTest;
var
   pts : array[0..3,0..1] of CInteger;
   ww  : Integer = GrSizeX div 10;
   hh  : Integer = GrSizeY div 10;
   sx  : Integer = (GrSizeX - 2*ww) div 32;
   sy  : Integer = (GrSizeY - 2*hh) div 32;
   ii,jj : Integer;
   color : GrColor;
   t1,t2,t3,mu1,mu2,mu3: CInteger;
begin
   GrClearScreen(black);
   t1 := GetCPUTime(mu1);
   pts[0][1] := 0;
   pts[1][1] := hh;
   pts[2][1] := 2*hh;
   pts[3][1] := hh;
   color := 0;
   for ii := 0 to 31 do begin
      pts[0][0] := ww;
      pts[1][0] := 2*ww;
      pts[2][0] := ww;
      pts[3][0] := 0;
      for jj := 0 to 31 do begin
         GrFilledPolygon(4,pts, pEGA^[color] or GrXor);
         color := (color + 1) and 15;
         Inc(pts[0][0],sx);
         Inc(pts[1][0],sx);
         Inc(pts[2][0],sx);
         Inc(pts[3][0],sx);
      end;
      Inc(pts[0][1],sy);
      Inc(pts[1][1],sy);
      Inc(pts[2][1],sy);
      Inc(pts[3][1],sy);
   end;
   t2 := GetCPUTime(mu2);
   pts[0][1] := 0;
   pts[1][1] := hh;
   pts[2][1] := 2*hh;
   pts[3][1] := hh;
   color := 0;
   for ii := 0 to 31 do begin
      pts[0][0] := ww;
      pts[1][0] := 2*ww;
      pts[2][0] := ww;
      pts[3][0] := 0;
      for jj := 0 to 31 do begin
         GrFilledConvexPolygon(4,pts, pEGA^[color] or GrXor);
         color := (color + 1) and 15;
         Inc(pts[0][0],sx);
         Inc(pts[1][0],sx);
         Inc(pts[2][0],sx);
         Inc(pts[3][0],sx);
      end;
      Inc(pts[0][1],sy);
      Inc(pts[1][1],sy);
      Inc(pts[2][1],sy);
      Inc(pts[3][1],sy);
   end;
   t3 := GetCPUTime(mu3);
   GrTextXY(0, 0, 'Times to scan 1024 polygons', white, black);
   WriteStr(msg, ' with GrFilledPolygon: ',((t2+mu2/1e6)-(t1+mu1/1e6)):0:2,' (s)');
   GrTextXY(0, 18, msg, white, black);
   WriteStr(msg, '  with GrFilledConvexPolygon: ',((t3+mu3/1e6)-(t2+mu2/1e6)):0:2,' (s)');
   GrTextXY(0, 36, msg, white, black);
end;

begin
   InitTest;
   pEGA:=GrAllocEgaColors;
   black:=pEGA^[0]; red:=pEGA^[12]; white:=pEGA^[15];

   Assign(f,'../test/polytest.dat'); Reset(f);
   collect:=false;
   while not eof(f) do begin
      ReadLn(f,Line);
      if not collect then begin
         if Copy(Line,1,5)='begin' then begin
            collect:=true;
            convex := Line[6]='c';
            nb:=0;
         end
      end else begin
         if Copy(Line,1,3)='end' then begin
            if nb>0 then TestPoly(nb,polygon,convex);
            collect:=false;
         end else begin
            with polygon[nb] do ReadStr(Line,x, y);
            Inc(nb);
         end;
      end;
   end;
   Close(f);
   SpeedTest;
   k:=GrKeyRead;
   EndTest
end.
