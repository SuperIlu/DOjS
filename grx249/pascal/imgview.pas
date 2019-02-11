{ PNG and JPEG viewer demo for GRX.

  Copyright (C) 2001 Frank Heckenbach <frank@pascal.gnu.de>

  This file is free software; as a special exception the author
  gives unlimited permission to copy and/or distribute it, with or
  without modifications, as long as this notice is preserved.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY, to the extent permitted by law; without
  even the implied warranty of MERCHANTABILITY or FITNESS FOR A
  PARTICULAR PURPOSE. }

program ImgViewer;

uses GPC, GRX;

var
  GraphicsActive: Boolean = False;

procedure CloseGraph;
var Dummy: Integer;
begin
  if GraphicsActive then Dummy := GrSetMode (Gr_Default_Text, 0, 0, 0, 0, 0);
  GraphicsActive := False
end;

procedure Warning (const Msg: String);
begin
  WriteLn (StdErr, ParamStr (0), ': ', Msg)
end;

procedure Error (const Msg: String);
begin
  CloseGraph;
  Warning (Msg);
  Halt (1)
end;

procedure OpenGraph (GWidth, GHeight: Integer);
var Width, Height: Integer;
begin
  Width := (GWidth + 7) div 8 * 8;
  Height := GHeight;
  if GrSetMode (Gr_Width_Height_BPP_Graphics, Width, Height, 24, 0, 0) <> 1 then
    Error ('could not initialize graphics');
  GraphicsActive := True
end;

var
  ImgFileName: TString;
  ArgStart, ArgN, Key: Integer;
  ImgWidth, ImgHeight: CInteger;
  grc: GrContextPtr;
  Centered: Boolean;

begin
  { Handle command-line arguments }
  Centered := ParamStr (1) = '-c';
  ArgStart := 1 + Ord (Centered);
  if ParamCount < ArgStart then
    begin
      WriteLn (StdErr, 'Usage: ', ParamStr (0), ' [-c] PNG-or-JPEG-file...');
      WriteLn (StdErr, '  -c  Center images in a larger window');
      Halt (1)
    end;

  { Check if library supports PNG and/or JPEG }
  if GrPngSupport = 0 then
    if GrJpegSupport = 0 then
      Error ('your version of GRX was compiled without PNG and JPEG support, sorry.')
    else
      Warning ('your version of GRX was compiled without PNG support, PNGs will not be recognized')
  else if GrJpegSupport = 0 then
    Warning ('your version of GRX was compiled without JPEG support, JPEGs will not be recognized');

  for ArgN := ArgStart to ParamCount do
    begin
      ImgFileName := ParamStr (ArgN);
      if not FileExists (ImgFileName) then
        Error ('file `' + ImgFileName + ''' not found');

      { PNG files }
      { Check if file is PNG and get its size }
      if GrQueryPng (ImgFileName, ImgWidth, ImgHeight) = 0 then
        begin
          if not Centered then
            begin

              { Simple example: window size = image size }
              OpenGraph (ImgWidth, ImgHeight);
              if GrLoadContextFromPng (nil, ImgFileName, 0) <> 0 then
                Error ('could not load ' + ImgFileName)

            end
          else
            begin

              { More complex example: window larger than image, display image centered }
              OpenGraph (2 * ImgWidth, 2 * ImgHeight);
              GrClearScreen (GrWhite);
              grc := GrCreateSubContext (Max (0, (GrScreenX - ImgWidth) div 2),
                                         Max (0, (GrScreenY - ImgHeight) div 2),
                                         GrScreenX, GrScreenY, nil, nil);
              if GrLoadContextFromPng (grc, ImgFileName, 0) <> 0 then
                Error ('could not load ' + ImgFileName);
              GrDestroyContext (grc)

            end
        end

      { Same for JPEG files (see the comments for PNG above) }
      else if GrQueryJpeg (ImgFileName, ImgWidth, ImgHeight) = 0 then
        begin
          if not Centered then
            begin
              OpenGraph (ImgWidth, ImgHeight);
              if GrLoadContextFromJpeg (nil, ImgFileName, 1) <> 0 then
                Error ('could not load ' + ImgFileName)
            end
          else
            begin
              OpenGraph (2 * ImgWidth, 2 * ImgHeight);
              GrClearScreen (GrWhite);
              grc := GrCreateSubContext (Max (0, (GrScreenX - ImgWidth) div 2),
                                         Max (0, (GrScreenY - ImgHeight) div 2),
                                         GrScreenX, GrScreenY, nil, nil);
              if GrLoadContextFromJpeg (grc, ImgFileName, 1) <> 0 then
                Error ('could not load ' + ImgFileName);
              GrDestroyContext (grc)
            end
        end

      else
        Error (ImgFileName + ' does not seem to be a PNG or JPEG file');
      Key := GrKeyRead;
      CloseGraph
    end
end.
