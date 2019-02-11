{ Graph  -  Interfacing BGI based graphics programs to LIBGRX
  Copyright (C) 96 by Sven Hilscher

  This file is part of the GRX graphics library.

  The GRX graphics library is free software; you can redistribute it
  and/or modify it under some conditions; see the "copying.grx" file
  for details.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

  Author: Sven Hilscher
  e-mail: sven@rufus.lan-ks.de

  Version 1.0 12/96

  22/02/01 patches by Maurice Lombardi <Maurice.Lombardi@ujf-grenoble.fr>
  Necessary to correct the drawpoly and fillpoly declarations.

  Maurice Lombardi, Frank Heckenbach <frank@pascal.gnu.de>: Some changes
  to make this file compile with more recent versions of GPC. }

{ Define this if you don't want this unit to declare a KeyPressed
  and a ReadKey function. }
{.$define NO_GRAPH_KEY_FUNCTIONS}

{ Define this if you want to use the Linux console version.
  (Ignored on non-Linux systems.) }
{.$define LINUX_CONSOLE}
  {and this if you use the svgalib (instead of only the framebuffer) driver}
{.$define SVGALIB}

{ Define this if you want to use the SDL driver in mingw or x11}
{.$define __SDL__}

{$gnu-pascal,I-}
{$ifdef __DJGPP__}
  {$L grx20}
{$elif defined (__MINGW32__)}
  {$ifdef __SDL__}
    {$L grx20S, SDL}
  {$else}
    {$L grx20}
  {$endif}
{$elif defined (_WIN32)}
  {$L grxW32, vfs.c, user32, gdi32}
{$elif defined (linux) and defined (LINUX_CONSOLE)}
  {$L grx20}
  {$ifdef SVGALIB}
     {$L vga}
  {$endif}
{$else}
  {$ifdef __SDL__}
    {$L grx20S, SDL, pthread, X11}
  {$else}
    {$L grx20X, X11}
  {$endif}
{$endif}

{ Uncomment those of the following libraries that you need }
{.$L tiff}
{.$L jpeg}
{.$L png}
{.$L z}
{.$L socket}

unit Graph;

interface

{$if __GPC_RELEASE__ >= 20030303}
{$define asmname external name}
{$define varasmname external name}
{$else}
{$define varasmname external; asmname}
{$endif}
{$if __GPC_RELEASE__ < 20040917}
type
  CInteger = Integer;
  CCardinal = Cardinal;
{$endif}

type
  PByte = ^Byte;
  WrkString = String[255];

const
  GrOk             =  0;
  GrNoInitGraph    = -1;
  GrNotDetected    = -2;
  GrFileNotFound   = -3;
  GrInvalidDriver  = -4;
  GrNoLoadMem      = -5;
  GrNoScanMem      = -6;
  GrNoFloodMem     = -7;
  GrFontNotFound   = -8;
  GrNoFontMem      = -9;
  GrInvalidMode    = -10;
  GrError          = -11;
  GrIOError        = -12;
  GrInvalidFont    = -13;
  GrInvalidFontNum = -14;
  GrInvalidVersion = -18;

  Detect           = 0;
  Native_GRX       = -3;
  CurrentDriver    = -128;
  VGA              = 9;
  EGA              = 3;
  IBM8514          = 6;
  HercMono         = 7;
  EGA64            = 4;
  EGAMono          = 5;
  CGA              = 1;
  MCGA             = 2;
  ATT400           = 8;
  PC3270           = 10;

  { driver definitions from BC++ 4.5: }
  DetectX          = Detect;
  VGA256           = 11;
  ATTDEB           = 12;
  Toshiba          = 13;
  SVGA16           = 14;
  SVGA256          = 15;
  SVGA32K          = 16;
  SVGA64K          = 17;
  VESA16           = 18;
  VESA256          = 19;
  VESA32K          = 20;
  VESA64K          = 21;
  VESA16M          = 22;
  ATI16            = 23;
  ATI256           = 24;
  ATI32K           = 25;
  Compaq           = 26;
  Tseng316         = 27;
  Tseng3256        = 28;
  Tseng416         = 29;
  Tseng4256        = 30;
  Tseng432K        = 31;
  Genoa5           = 32;
  Genoa6           = 33;
  OAK              = 34;
  Paradis16        = 35;
  Paradis256       = 36;
  Tecmar           = 37;
  Trident16        = 38;
  Trident256       = 39;
  Video7           = 40;
  Video7ii         = 41;
  S3               = 42;
  ATIGUP           = 43;

  VGALo            =  0;
  VGAMed           =  1;
  VGAHi            =  2;
  IBM8514Lo        =  0;
  IBM8514Hi        =  1;
  HercMonoHi       =  0;
  CGAC0            =  0;
  CGAC1            =  1;
  CGAC2            =  2;
  CGAC3            =  3;
  CGAHi            =  4;
  MCGAC0           =  CGAC0;
  MCGAC1           =  CGAC1;
  MCGAC2           =  CGAC2;
  MCGAC3           =  CGAC3;
  MCGAMed          =  CGAHi;
  MCGAHi           =  5;
  ATT400C0         =  MCGAC0;
  ATT400C1         =  MCGAC1;
  ATT400C2         =  MCGAC2;
  ATT400C3         =  MCGAC3;
  ATT400Med        =  MCGAMed;
  ATT400Hi         =  MCGAHi;
  EGA64Lo          =  0;
  EGA64Hi          =  1;
  EGALo            =  0;
  EGAHi            =  1;
  EGAMonoHi        =  3;
  PC3270Hi         =  0;

  { mode definitions from BC++ 4.5: }
  Res640x350       =  0;
  Res640x480       =  1;
  Res800x600       =  2;
  Res1024x768      =  3;
  Res1280x1024     =  4;

  { NATIVE_GRX modes: }
  GRX_Default_Graphics               = 0;
  GRX_Biggest_Noninterlaced_Graphics = 1;
  GRX_Biggest_Graphics               = 2;
  GRX_BGI_Emulation                  = 3;
  First_Driver_Specific_Mode         = 4;

  EGABlack        =  0;
  EGABlue         =  1;
  EGAGreen        =  2;
  EGACyan         =  3;
  EGARed          =  4;
  EGAMagenta      =  5;
  EGABrown        = 20;
  EGALightGray    =  7;
  EGADarkGray     = 56;
  EGALightBlue    = 57;
  EGALightGreen   = 58;
  EGALightCyan    = 59;
  EGALightRed     = 60;
  EGALightMagenta = 61;
  EGAYellow       = 62;
  EGAWhite        = 63;

  SolidLn         =  0;
  DottedLn        =  1;
  CenterLn        =  2;
  DashedLn        =  3;
  UserBitLn       =  4;

  NormWidth       =  1;
  ThickWidth      =  3;

  DefaultFont     =  0;  { 8x8 bit mapped font }
  TriplexFont     =  1;
  SmallFont       =  2;
  SansSerifFont   =  3;
  GothicFont      =  4;
  ScriptFont      =  5;
  SimplexFont     =  6;
  TriplexScrFont  =  7;
  ComplexFont     =  8;
  EuropeanFont    =  9;
  BoldFont        =  10;

  HorizDir        =  0;  { left to right }
  VertDir         =  1;  { bottom to top }

  UserCharSize    =  0;  { user-defined char size }

  NormalPut       =  0;  { copy }
  CopyPut         =  0;  { copy }
  XORPut          =  1;  { xor }
  OrPut           =  2;  { or  }
  AndPut          =  3;  { and }
  NotPut          =  4;  { not }

  LeftText        =  0;
  CenterText      =  1;
  RightText       =  2;
  BottomText      =  0;
  TopText         =  2;

  MaxColors       = 15;

  { Clipping }
  ClipOn          = True;
  ClipOff         = False;

  { Bar3D }
  TopOn           = True;
  TopOff          = False;

  { FillStyles }
  EmptyFill       =  0;  { fills area in background color }
  SolidFill       =  1;  { fills area in solid fill color }
  LineFill        =  2;  { --- fill }
  LtSlashFill     =  3;  { /// fill }
  SlashFill       =  4;  { /// fill with thick lines }
  BkSlashFill     =  5;  { \\\ fill with thick lines }
  LtBkSlashFill   =  6;  { \\\ fill }
  HatchFill       =  7;  { light hatch fill }
  XHatchFill      =  8;  { heavy cross hatch fill }
  InterleaveFill  =  9;  { interleaving line fill }
  WideDotFill     = 10;  { Widely spaced dot fill }
  CloseDotFill    = 11;  { Closely spaced dot fill }
  UserFill        = 12;  { user defined fill }

type
  PaletteType = record
    Size  : Byte;
    Colors: array [0 .. MaxColors] of Byte
  end;

  LineSettingsType = record
    LineStyle: CInteger;
    UPattern : ShortCard;
    Thickness: CInteger
  end;

  TextSettingsType = record
    Font,
    Direction,
    CharSize,
    Horiz,
    Vert: CInteger
   end;

  FillSettingsType = record
    Pattern,
    Color: CInteger
  end;

  FillPatternType = array [1 .. 8] of Byte;

  { This definition is compatible with the grx
    definition `int pts[][2]' used to define polygons }
  PointType = record
    x, y: CInteger
  end;

  ViewPortType = record
    x1, y1, x2, y2: CInteger;
    Clip: WordBool
  end;

  ArcCoordsType = record
    x, y,
    XStart, YStart, XEnd, YEnd: CInteger
  end;

var
  GraphGetMemPtr : Pointer;  { Dummy! }
  GraphFreeMemPtr: Pointer;  { Dummy! }

{ BGI - API definitions }

procedure DetectGraph(var GraphDriver, GraphMode: Integer);
procedure InitGraph(var GraphDriver, GraphMode: Integer; PathToDriver: CString);
procedure SetGraphMode(Mode: CInteger); asmname 'setgraphmode';
function  GetModeName(ModeNumber: CInteger): WrkString;
procedure GraphDefaults; asmname 'graphdefaults';
function  GetDriverName: WrkString;
function  GraphErrorMsg(ErrorCode: CInteger): WrkString;
function  GetMaxX: CInteger; asmname 'getmaxx';
function  GetMaxY: CInteger; asmname 'getmaxy';
function  GetMaxColor: CInteger; asmname 'getmaxcolor';
procedure GetViewSettings(var ViewPort: ViewPortType); asmname 'getviewsettings';
procedure SetViewPort(Left, Top, Right, Bottom: CInteger; Clip: Boolean); asmname 'setviewport';
procedure GetLineSettings(var LineInfo: LineSettingsType); asmname 'getlinesettings';
procedure SetLineStyle(LineStyle: CInteger; Pattern: CInteger; Thickness: CInteger); asmname 'setlinestyle';
procedure ClearViewPort; asmname 'clearviewport';
function  GetPixel(x,y: CInteger): CInteger; asmname 'getpixel';
procedure PutPixel(x, y: CInteger; Pixel: CInteger); asmname 'putpixel';
procedure Bar3D(Left, Top, Right, Bottom: CInteger; Depth: CInteger; TopFlag: Boolean); asmname 'bar3d';
procedure Rectangle(Left, Top, Right, Bottom: CInteger); asmname 'rectangle';
procedure FillPoly(NumPoints: CCardinal; var PolyPoints { : array of PointType }); asmname 'fillpoly';
procedure FillEllipse(x, y: CInteger; XRadius, YRadius: CInteger); asmname 'fillellipse';
procedure GetArcCoords(var ArcCoords: ArcCoordsType); asmname 'getarccoords';
procedure FloodFill(x, y: CInteger; Border: CInteger); asmname 'floodfill';
procedure SetFillPattern(UPattern: FillPatternType; Color: CInteger); asmname 'setfillpattern';
procedure SetFillStyle(Pattern: CInteger; Color: CInteger); asmname 'setfillstyle';
procedure GetImage(Left, Top, Right, Bottom: CInteger; var BitMap); asmname 'getimage';
procedure PutImage(Left, Top: CInteger; var BitMap; Op: CInteger); asmname 'putimage';
function  ImageSize(Left, Top, Right, Bottom: CInteger): CInteger; asmname 'imagesize';
procedure GetTextSettings(var TextTypeInfo: TextSettingsType); asmname 'gettextsettings';
procedure SetTextJustify(Horiz, Vert: CInteger); asmname 'settextjustify';
procedure SetTextStyle(Font, Direction: CInteger; CharSize: CInteger); asmname 'settextstyle';
procedure SetRGBPalette(Color, Red, Green, Blue: CInteger); asmname 'setrgbpalette';
procedure SetUserCharSize(MultX, DivX, MultY, DivY: CInteger); asmname 'setusercharsize';
procedure SetWriteMode(Mode: CInteger); asmname 'setwritemode';
procedure OutText(TextString: CString); asmname 'outtext';
procedure OutTextXY(x, y: CInteger; TextString: CString); asmname 'outtextxy';
function  TextHeight(TextString: CString): CInteger; asmname 'textheight';
function  TextWidth(TextString: CString): CInteger; asmname 'textwidth';

function  RegisterBGIfont(Font: Pointer): CInteger; asmname 'registerbgifont';
function  InstallUserFont(FontName: CString): CInteger; asmname 'installuserfont';

function  GetPaletteSize: CInteger; asmname 'getpalettesize';
procedure GetPalette(var Palette: PaletteType); asmname 'getpalette';
procedure SetPalette(ColorNum: CCardinal; Color: CInteger); asmname '__gr_setpalette';
procedure SetAllPalette(protected var Palette: PaletteType); asmname 'setallpalette';

procedure RestoreCrtMode; asmname '__gr_restorecrtmode';
procedure CloseGraph; asmname '__gr_closegraph';
procedure SetColor(Color: CInteger); asmname '__gr_setcolor';
procedure SetBkColor(Color: CInteger); asmname '__gr_setbkcolor';
procedure Bar(Left, Top, Right, Bottom: CInteger); asmname '__gr_bar';
function  GraphResult:CInteger; asmname '__gr_graphresult';
procedure Line(x1, y1, x2, y2: CInteger); asmname '__gr_line';
procedure MoveTo(x, y: CInteger); asmname '__gr_moveto';
procedure Arc(x, y: CInteger; StAngle, EndAngle, Radius: CCardinal); asmname '__gr_arc';
procedure Circle(x, y: CInteger; Radius: CCardinal); asmname '__gr_circle';
procedure ClearDevice; asmname '__gr_cleardevice';
procedure DrawPoly(NumPoints: CCardinal; var PolyPoints { : array of PointType }); asmname '__gr_drawpoly';
procedure Ellipse(x, y: CInteger; StAngle, EndAngle: CCardinal; XRadius, YRadius: CCardinal); asmname '__gr_ellipse';
procedure GetAspectRatio(var XAsp, YAsp: Integer);
function  GetBkColor: CCardinal; asmname '__gr_getbkcolor';
function  GetColor: CCardinal; asmname '__gr_getcolor';
procedure GetFillPattern(var FillPattern: FillPatternType); asmname '__gr_getfillpattern';
procedure GetFillSettings(var FillInfo: FillSettingsType); asmname '__gr_getfillsettings';
function  GetMaxMode: CInteger; asmname '__gr_getmaxmode';
function  GetGraphMode: CInteger; asmname '__gr_getgraphmode';
function  GetX: CInteger; asmname '__gr_getx';
function  GetY: CInteger; asmname '__gr_gety';
function  InstallUserDriver(const DriverName: String; AutoDetectPtr: Pointer):CInteger;
procedure LineRel(dx, dy: CInteger); asmname '__gr_linerel';
procedure LineTo(x, y: CInteger); asmname '__gr_lineto';
procedure MoveRel(dx, dy: CInteger); asmname '__gr_moverel';
procedure PieSlice(x, y: CInteger; StAngle, EndAngle, Radius: CCardinal); asmname '__gr_pieslice';
function  RegisterBGIdriver(Driver: Pointer): CInteger;
procedure Sector(x, y: CInteger; StAngle,EndAngle, XRadius, YRadius: CCardinal); asmname '__gr_sector';
procedure SetAspectRatio(XAsp, YAsp: CInteger); asmname '__gr_setaspectratio';
procedure SetGraphBufSize(BufSize: CCardinal); asmname '__gr_setgraphbufsize';
procedure SetActivePage(Page: CCardinal); asmname '__gr_setactivepage';
procedure SetVisualPage(Page: CCardinal); asmname '__gr_setvisualpage';
procedure GetDefaultPalette(var Palette: PaletteType);
procedure GetModeRange(GraphDriver:CInteger; var LoMode, HiMode:Integer);

function  Black       : CInteger;
function  Blue        : CInteger;
function  Green       : CInteger;
function  Cyan        : CInteger;
function  Red         : CInteger;
function  Magenta     : CInteger;
function  Brown       : CInteger;
function  LightGray   : CInteger;
function  DarkGray    : CInteger;
function  LightBlue   : CInteger;
function  LightGreen  : CInteger;
function  LightCyan   : CInteger;
function  LightRed    : CInteger;
function  LightMagenta: CInteger;
function  Yellow      : CInteger;
function  White       : CInteger;

{ BGI - API extensions }

{ Linkable font files }
var
  Bold_Font: PByte; varasmname '_bold_font';
  Euro_Font: PByte; varasmname '_euro_font';
  Goth_Font: PByte; varasmname '_goth_font';
  Lcom_Font: PByte; varasmname '_lcom_font';
  Litt_Font: PByte; varasmname '_litt_font';
  Sans_Font: PByte; varasmname '_sans_font';
  Scri_Font: PByte; varasmname '_scri_font';
  Simp_Font: PByte; varasmname '_simp_font';
  Trip_Font: PByte; varasmname '_trip_font';
  Tscr_Font: PByte; varasmname '_tscr_font';


{ Translates BGI driver/mode into a driver/mode pair for this unit
  for usage with InitGraph / SetGraphMode }
procedure SetBGImode(var GDrv, GMode: CInteger); asmname 'set_BGI_mode';

{ Determines a driver/mode pair for InitGraph that will set
  up a graphics mode with the desired resolution and colors }
procedure SetBGImodeWHC(var GDrv, GMode: CInteger; Width, Height, Colors: CInteger); asmname '__gr_set_BGI_mode_whc';

{ enable multiple graphics pages by
    SetBGIModePages (2);
    InitGraph (gd, gm, '');
    if (GraphResult = grOk) and (GetBGIModePages=2) then PlayWithPages; }
procedure SetBGIModePages(p: CInteger); asmname '__gr_set_BGI_mode_pages';
function  GetBGIModePages: CInteger; asmname '__gr_get_BGI_mode_pages';

{ like GetMaxColor, GetMaxX, GetMaxY but for any available mode }
function  GetModeMaxColor(Mode: CInteger): CInteger; asmname '__gr_getmodemaxcolor';
function  GetModeMaxX(Mode: CInteger): CInteger; asmname '__gr_getmodemaxx';
function  GetModeMaxY(Mode: CInteger): CInteger; asmname '__gr_getmodemaxy';

{ Set the actual drawing color to r/g/b.
  Functional in HiColor/TrueColor modes only }
procedure SetRGBColor(Red, Green, Blue: CCardinal); asmname '__gr_setrgbcolor';

{ Returns the actual RGB palette. }
procedure GetRGBPalette(Color: CInteger; var Red, Green, Blue: CInteger); asmname '__getrgbpalette';

{ Transforms the predefined 16 EGA colors (RED, BLUE, MAGENTA, ...)
  into the correct color value for the active graphics mode }
function EGAColor(EGACol: CInteger): CInteger; asmname '_ega_color';

{ Enable/disable linestyle drawing of vector fonts.
  Currently only functional with GRX v1.x }
procedure TextLineStyle(On: Boolean); asmname '__gr_textlinestyle';

{ Counterparts of SetActivePage/SetVisualPage }
function  GetActivePage: CInteger; asmname '__gr_getactivepage';
function  GetVisualPage: CInteger; asmname '__gr_getvisualpage';

{$ifndef NO_GRAPH_KEY_FUNCTIONS}
function KeyPressed: Boolean; asmname 'kbhit';
function ReadKey: Char; asmname 'getch';
{$endif}

{flush the graphics: useful only in X windows when switching 
 between graphics and console windows open simultaneously}
procedure GrFlush; asmname 'GrFlush';

implementation

procedure dg(var GraphDriver, GraphMode: CInteger); asmname 'detectgraph';
procedure DetectGraph(var GraphDriver, GraphMode: Integer);
var gd, gm: CInteger;
begin
  gd := GraphDriver;
  gm := GraphMode;
  dg (gd, gm);
  GraphDriver := gd;
  GraphMode := gm
end;

procedure ig(var gd, gm: CInteger; PathToDriver: CString); asmname 'initgraph';
procedure InitGraph(var GraphDriver, GraphMode: Integer; PathToDriver: CString);
var gd, gm: CInteger;
begin
  gd := GraphDriver;
  gm := GraphMode;
  ig (gd, gm, PathToDriver);
  GraphDriver := gd;
  GraphMode := gm
end;

function gem(ec: CInteger): CString; asmname 'grapherrormsg';
function GraphErrorMsg(ErrorCode: CInteger): WrkString;
begin
  GraphErrorMsg := CString2String (gem (ErrorCode))
end;

function gmn(mn: CInteger): CString; asmname 'getmodename';
function GetModeName(ModeNumber: CInteger): WrkString;
begin
  GetModeName := CString2String (gmn (ModeNumber))
end;

function gdn: CString; asmname 'getdrivername';
function GetDriverName: WrkString;
begin
  GetDriverName := CString2String (gdn)
end;

procedure gar(var XAsp, YAsp: CInteger); asmname '__gr_getaspectratio';
procedure GetAspectRatio(var XAsp, YAsp: Integer);
var xa, ya: CInteger;
begin
  gar (xa, ya);
  XAsp := xa;
  YAsp := ya
end;

type pPT = ^PaletteType;
function gdp: pPT; asmname '__gr_getdefaultpalette';
procedure GetDefaultPalette(var Palette: PaletteType);
begin
 Palette := gdp^
end;

procedure gmr(GraphDriver:CInteger; var LoMode, HiMode:CInteger); asmname '__gr_getmoderange';
procedure GetModeRange(GraphDriver:CInteger; var LoMode, HiMode:Integer);
var lm, hm: CInteger;
begin
  gmr (GraphDriver, lm, hm);
  LoMode := lm;
  HiMode := hm
end;

function InstallUserDriver(const DriverName: String; AutoDetectPtr: Pointer):CInteger;
var
  Dummy1: Integer;
  Dummy2: Pointer;
begin
  Dummy1 := Length (DriverName);
  Dummy2 := AutoDetectPtr;
  InstallUserDriver := GrError
end;

function RegisterBGIdriver(Driver: Pointer): CInteger;
var Dummy: Pointer;
begin
  Dummy := Driver;
  RegisterBGIdriver := GrError
end;

function Black: CInteger;
begin
  Black := EGAColor (0)
end;

function Blue: CInteger;
begin
  Blue := EGAColor (1)
end;

function Green: CInteger;
begin
  Green := EGAColor (2)
end;

function Cyan: CInteger;
begin
  Cyan := EGAColor (3)
end;

function Red: CInteger;
begin
  Red := EGAColor (4)
end;

function Magenta: CInteger;
begin
  Magenta := EGAColor (5)
end;

function Brown: CInteger;
begin
  Brown := EGAColor (6)
end;

function LightGray: CInteger;
begin
  LightGray := EGAColor (7)
end;

function DarkGray: CInteger;
begin
  DarkGray := EGAColor (8)
end;

function LightBlue: CInteger;
begin
  LightBlue := EGAColor (9)
end;

function LightGreen: CInteger;
begin
  LightGreen := EGAColor (10)
end;

function LightCyan: CInteger;
begin
  LightCyan := EGAColor (11)
end;

function LightRed: CInteger;
begin
  LightRed := EGAColor (12)
end;

function LightMagenta: CInteger;
begin
  LightMagenta := EGAColor (13)
end;

function Yellow: CInteger;
begin
  Yellow := EGAColor (14)
end;

function White: CInteger;
begin
  White := EGAColor (15)
end;

end.
