program Test_GetDefaultPalette;
{
 * test program for the Graph unit
}

uses Graph;

var
  i,
  grDriver,
  grMode,
  ErrCode: Integer;
  pal : PaletteType;
begin
  { InitGraph is not needed for GetMaxMode and GetModeName in GPC }
  { grDriver := Detect; }
  grDriver := InstallUserDriver('SVGA256', nil); { Not used in GPC }
  InitGraph(grDriver, grMode,'../../chr');
  ErrCode := GraphResult;
  if ErrCode = GrOk then
  begin  { Do graphics }
    GetDefaultPalette(pal);
    CloseGraph;
    i := pal.Size;
    WriteLn('Palette size: ', i);
    for i := 0 to pal.Size-1 do
      WriteLn('Color ', i:3, ' : ', pal.Colors[i]);
    ReadLn;
  end
  else begin
    WriteLn('Graphics error:', GraphErrorMsg(ErrCode));
    Write  ('Press Enter ...');
    ReadLn
  end
end.
