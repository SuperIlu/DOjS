{$X+}

program Colors;
{
 * test and demo program for the Graph unit
 *
 * Please read the copyright notices of graph.pas
 *
 * This file is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * Author : Sven Hilscher
 * e-mail : sven@rufus.central.de
}

uses
  Graph;

var
  i, grDriver, grMode, ErrCode: Integer;

procedure ShowColors(Mode: Word);
var
  y: Word;
  f1, f2: Real;
begin
  { Show only 8 bit, hicolor and truecolor modes }
  if GetMaxColor < 255 then return;
  { Show only 640 x 480 modes }
  { if (GetMaxX <> 639) or (GetMaxY <> 479) then return; }

  f1 := GetMaxY / 64;
  f2 := GetMaxY / 256;

  { Red }
  for y := 0 to GetMaxY do begin
    if GetMaxColor = 255 then begin
      SetRGBPalette(trunc(y / f1), trunc(y / f1), 0, 0);
      SetColor(trunc(y / f1))
    end
    else
      SetRGBColor(trunc(y / f2), 0, 0);
    Line(0, y, GetMaxX div 4, y)
  end;

  { Green }
  for y := 0 to GetMaxY do begin
    if GetMaxColor = 255 then begin
      SetRGBPalette(64 + trunc(y / f1), 0, trunc(y / f1), 0);
      SetColor(64 + trunc(y / f1))
    end
    else
      SetRGBColor(0, trunc(y / f2), 0);
    Line(GetMaxX div 4, y, GetMaxX div 2, y)
  end;

  { Blue }
  for y := 0 to GetMaxY do begin
    if GetMaxColor = 255 then begin
      SetRGBPalette(128 + trunc(y / f1), 0, 0, trunc(y / f1));
      SetColor(128 + trunc(y / f1))
    end
    else
      SetRGBColor(0, 0, trunc(y / f2));
    Line(GetMaxX div 2, y, GetMaxX - GetMaxX div 4, y)
  end;

  { Gray }
  for y := 0 to GetMaxY do begin
    if GetMaxColor = 255 then begin
      SetRGBPalette(192 + trunc(y / f1), trunc(y / f1), trunc(y / f1), trunc(y / f1));
      SetColor(192 + trunc(y / f1))
    end
    else
      SetRGBColor(trunc(y / f2), trunc(y / f2), trunc(y / f2));
    Line(GetMaxX - GetMaxX div 4, y, GetMaxX, y)
  end;

  if GetMaxColor = 255 then
    SetColor(255)
  else
    SetColor(White);
  OutTextXY(10, 2, GetModeName(Mode)) ;
  Rectangle(0,0,GetMaxX,GetMaxY);
  ReadKey
end;

begin
  grDriver := Detect;
  InitGraph(grDriver, grMode,'../../chr');
  ErrCode := GraphResult;
  if ErrCode = GrOk then
  begin  { Do graphics }
    for i := 0 to GetMaxMode do
    begin
      SetGraphMode(i);
      ShowColors(i)
    end;
    CloseGraph
  end
  else begin
    Writeln('Graphics error:', GraphErrorMsg(ErrCode));
    Write  ('Press Enter ...');
    ReadLn
  end
end.

