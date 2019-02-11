program KeyTest;

uses GRX;

var
  MouseMode, DrawMode,
  m, x, y, co : Integer;
  evt: GrMouseEvent;
  Key : Char;
  Finito : boolean;

procedure MouseRoutine;
   var st: String[80];
begin
  if (evt.Flags and Gr_M_KeyPress) > 0 then begin
    Key := Chr (evt.Key and $FF);
    writestr(st,'Key :', Key:2, Ord(Key):4, evt.Key mod 256: 4, evt.Key div 256: 4, evt.Key: 6);
    GrTextXY(evt.x,evt.y,st,GrWhite,GrBlack);
    case evt.Key of
      81, 113 : Finito := true; {'Q', 'q'}
    end;
  end;
end;

begin
  x  := 640;
  y  := 480;
  co := 256;
  m  := GrSetMode(Gr_Default_Graphics,x,y,co,0,0);

  if GrMouseDetect then begin
    GrMouseEventMode(1);
    GrMouseInit;
    GrMouseDisplayCursor;
    DrawMode  := 0;
    MouseMode := 4;
  end else
    exit;

  GrTextXY(0,0,'Use ''Q'' to Quit',GrWhite,GrBlack);
  repeat
    Finito  := false;
    GrMouseGetEventT(Gr_M_Event,@evt,0);
    if evt.Flags > 0 then MouseRoutine;
  until Finito
end.
