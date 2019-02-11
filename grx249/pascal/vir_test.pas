program Vir_Test;

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
   x1, y1,
   MouseMode, DrawMode,
   xc, yc,
   x, y, xv, yv, c : Integer;
   m : Integer;
   evt: GrMouseEvent;
   Key : Char;
   Finito : Boolean;
   Conti: GrContext;
   ContiPtr : GrContextPtr;

begin
   x  :=  640;
   y  :=  480;
   xv :=  1024;
   yv :=  1024;
   c  :=  65536;
   xc :=  40;
   yc :=  30;

   m := GrSetMode(Gr_Custom_Graphics,x,y,c,xv,yv);
   GrSetModeRestore(False);

   xv := GrVirtualX;
   yv := GrVirtualY;

   TestFunc;
   GrTextXY(10, 10, ' Use the Keys <1> <2> <3> <4> <0> and press the mousebutton ', GrBlack, GrWhite);
   GrTextXY(10, 30, ' In "mode 3" use "<" and ">" to resize the area  "Q" = Quit ', GrBlack, GrWhite);

   if GrMouseDetect then begin
     GrMouseEventMode(1);
     GrMouseInit;
     GrMouseSetColors(GrAllocColor(200,50,150),GrBlack);
     { GrMouseSetLimits(10, 10, xv-10, yv-10); }
     GrMouseDisplayCursor;
     Finito := False;
     repeat
       GrMouseGetEvent(Gr_M_Event,@evt);
       if (evt.Flags and Gr_M_KeyPress) > 0 then begin
         Key := Chr (evt.Key and $FF);
         case Key of
           'Q', 'q': Finito := True;
           'W', 'w': GrMouseWarp(2, 2);
           'U', 'u': GrMouseUpdateCursor;
           '>': if (xc < x) and (MouseMode = 3) and ((evt.Flags and Gr_M_Left_Down) = 0) then begin
                  xc := xc + 8;
                  yc := yc + 6;
                  GrMouseEraseCursor;
                  GrMouseSetCursorMode(Gr_M_Cur_Box, -xc div 2, -yc div 2, xc div 2, yc div 2,GrWhite);
                  GrMouseDisplayCursor;
                end;
           '<': if (xc > 8) and (MouseMode = 3) and ((evt.Flags and Gr_M_Left_Down) = 0) then begin
                  xc := xc - 8;
                  yc := yc - 6;
                  GrMouseEraseCursor;
                  GrMouseSetCursorMode(Gr_M_Cur_Box, -xc div 2, -yc div 2, xc div 2, yc div 2,GrWhite);
                  GrMouseDisplayCursor;
                end;
           '1': begin
                  GrMouseEraseCursor;
                  GrMouseSetCursorMode(Gr_M_Cur_Normal,0,0,0,0,0);
                  GrMouseDisplayCursor;
                  MouseMode := 1;
                end;
           '2': begin
                  GrMouseEraseCursor;
                  GrMouseSetCursorMode(Gr_M_Cur_Normal,0,0,0,0,0);
                  GrMouseDisplayCursor;
                  MouseMode := 2;
                end;
           '3': begin
                  GrMouseEraseCursor;
                  GrMouseSetCursorMode(Gr_M_Cur_Box, -xc div 2, -yc div 2, xc div 2, yc div 2,GrWhite);
                  GrMouseDisplayCursor;
                  MouseMode := 3;
                end;
           '0': begin
                  GrMouseEraseCursor;
                  GrMouseSetCursorMode(Gr_M_Cur_Normal,0,0,0,0,0);
                  GrMouseDisplayCursor;
                  MouseMode := 0;
                end;
           '4': begin
                  GrMouseEraseCursor;
                  GrMouseSetCursorMode(Gr_M_Cur_Normal,0,0,0,0,0);
                  GrMouseDisplayCursor;
                  MouseMode := 4;
                end;
           '5': begin
                  GrMouseEraseCursor;
                  GrMouseSetCursorMode(Gr_M_Cur_Cross,0,0,GrWhite,0,0);
                  GrMouseDisplayCursor;
                  MouseMode := 4;
                end;
         end;
       end;
       if (evt.Flags and Gr_M_Left_Down) > 0 then begin
         case MouseMode of
           1: begin
                x1 := evt.x;
                y1 := evt.y;
                GrMouseEraseCursor;
                GrMouseSetCursorMode(Gr_M_Cur_Rubber,evt.x,evt.y,GrWhite,0,0);
                GrMouseDisplayCursor;
              end;
           2: begin
                x1 := evt.x;
                y1 := evt.y;
                GrMouseEraseCursor;
                GrMouseSetCursorMode(Gr_M_Cur_Line,evt.x,evt.y,GrWhite,0,0);
                GrMouseDisplayCursor;
              end;
           3: begin
                x1 := evt.x;
                y1 := evt.y;  { Why I have to save the position ??? }
                ContiPtr := GrCreateContext(xc + 1, yc + 1, Nil, @Conti);
                GrMouseEraseCursor;
                GrBitBlt(@Conti, 0, 0, GrScreenContext, x1 - (xc div 2), y1 - (yc div 2), x1 + (xc div 2), y1 + (yc div 2), GrWrite);
                GrMouseDisplayCursor;
              end;
           4: begin
                x1 := evt.x;
                y1 := evt.y;
                DrawMode := 4;
              end;
         end;
       end;
       if (evt.Flags and Gr_M_Left_Up) > 0 then begin
         case MouseMode of
           1: begin
                GrBox(x1, y1, evt.x, evt.y, GrAllocColor(50,100,150));
                GrMouseEraseCursor;
                GrMouseSetCursorMode(Gr_M_Cur_Normal,0,0,0,0,0);
                GrMouseDisplayCursor;
              end;
           2: begin
                GrLine(x1, y1, evt.x, evt.y, GrAllocColor(150,100,50));
                GrMouseEraseCursor;
                GrMouseSetCursorMode(Gr_M_Cur_Normal,0,0,0,0,0);
                GrMouseDisplayCursor;
              end;
           3: begin
                GrMouseEraseCursor;
                GrBitBlt(GrScreenContext, evt.x - (xc div 2), evt.y - (yc div 2), @Conti, 0, 0, xc, yc, GrWrite);
                GrDestroyContext(@Conti);
                GrMouseDisplayCursor;
              end;
           4: DrawMode := 0;
         end;
       end;
       if DrawMode = 4 then begin
         GrLine(x1, y1, evt.x, evt.y, GrWhite);
         x1 := evt.x;
         y1 := evt.y;
       end;
       if (evt.Flags and Gr_M_Right_Down) > 0 then begin
       end;
       { Move the visible Part of the virtual screen }
       m := GrSetViewport(evt.x - x div 2, evt.y - y div 2);
     until Finito;
     GrMouseUnInit;
   end;
end.

