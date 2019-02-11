{$X+}

program Vir_Test;

uses GRX;

var
  i, f, m : Integer;
  gvm : GrVideoMode;
  fivm : GrVideoModePtr;

begin
  { intializes grx20 }
  m := GrSetMode(Gr_Default_Text, 0, 0, 0, 0, 0);

  GrSetModeRestore(False);

  for f := Gr_Unknown_Mode to Gr_FrameSVGA32H do begin
    fivm := GrFirstVideoMode(f);
    while fivm <> Nil do begin
      gvm := fivm^;
      i := i + 1;
      Write  (i:3);
      Write  (f:3);
      Write  (gvm.Mode:6);
      Write  (gvm.Width:5, 'x',gvm.Height:4, 'x', gvm.BPP:3);
      WriteLn;
      fivm := GrNextVideoMode(fivm)
    end
  end;
  Readln
end.
