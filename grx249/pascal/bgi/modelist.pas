program ModeList;
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

uses Graph;

var
  i,
  grDriver,
  grMode,
  ErrCode: Integer;

begin
  { InitGraph is not needed for GetMaxMode and GetModeName in GPC }
  grDriver := Detect;
  {grDriver := InstallUserDriver('SVGA256', nil);} { Not used in GPC }
  InitGraph(grDriver, grMode,'../../chr');
  ErrCode := GraphResult;
  if ErrCode = GrOk then
  begin  { Do graphics }
    (*$ifdef __GPC__ *)
      CloseGraph;
    (*$endif*)
    for i := 0 to GetMaxMode do begin
      WriteLn(i:4, ' ' + GetModeName(i))
    end;
    ReadLn;
    (*$ifndef __GPC__ *)
      CloseGraph
    (*$endif*)
  end
  else begin
    WriteLn('Graphics error:', GraphErrorMsg(ErrCode));
    Write  ('Press Enter ...');
    ReadLn
  end
end.
