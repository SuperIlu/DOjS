{ GRX color allocation test.

  Copyright (C) 2001 Frank Heckenbach <frank@pascal.gnu.de>

  This file is free software; as a special exception the author
  gives unlimited permission to copy and/or distribute it, with or
  without modifications, as long as this notice is preserved.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY, to the extent permitted by law; without
  even the implied warranty of MERCHANTABILITY or FITNESS FOR A
  PARTICULAR PURPOSE. }

{$X+}

program ColorTest;

uses GRX;

var
  Dummy, i, x, y, j: Integer = 0;
  Color: array [0 .. 1023] of Integer;
  s : string[255];

begin
  Dummy := GrSetMode (Gr_Width_Height_BPP_Graphics, 800, 300, 16, 0, 0);
  for x := 0 to 1023 do
    begin
      { GrAllocCell; GrSetColor (x, x, x, x); }
      if x < 256 then
        Color[x] := GrAllocColor (x, x, x)
      else if x < 512 then
        Color[x] := GrAllocColor (x - 256, 0, 0)
      else if x < 768 then
        Color[x] := GrAllocColor (0, x - 512, 0)
      else
        Color[x] := GrAllocColor (0, 0, x - 768);
      Inc (j);
      if Color[x] <= $100000 then Inc (i)
    end;
  WriteStr (s, i, ' of ', j, ' colors allocated.');
  for y := 0 to 299 do
    for x := 0 to 799 do
      GrPlot (x, y, Color[(x + y) mod 1024]);
  GrTextXY(300, 10, s, GrWhite, GrNoColor);
  i := GrKeyRead
end.
