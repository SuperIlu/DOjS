{ Converted by Sven Hilscher eMail sven@rufus.lan-ks.de
  from header file grx20.h

  grx20.h ---- GRX 2.x API functions and data structure declarations

  Copyright (c) 1995 Csaba Biegl, 820 Stirrup Dr, Nashville, TN 37221
  [e-mail: csaba@vuse.vanderbilt.edu]

  This file is part of the GRX graphics library.

  The GRX graphics library is free software; you can redistribute it
  and/or modify it under some conditions; see the "copying.grx" file
  for details.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. }

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

unit GRX;

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

const
  MaxVarSize = MaxInt div 8;

type
  MemPtr      = ^Char;
{$if __GPC_RELEASE__ >= 20030424}
  GrColor     = Integer attribute (Size = 32);  { color and operation type, must be 32bit }
{$else}
  GrColor     = Integer (32);  { color and operation type, must be 32bit }
{$endif}
  GrColorPtr  = ^GrColor;
  GrColors    = array [0 .. MaxVarSize div SizeOf (GrColor) - 1] of GrColor;
  GrColorsPtr = ^GrColors;

  { This definition is compatible with the grx
    definition 'int pts[][2]' used to define polygons }
  PointType = record
    x, y: CInteger
  end;

const
  { these are the supported configurations: }
  GRX_Version_TCC_8086_DOS     =  1;  { also works with BCC }
  GRX_Version_GCC_386_GO32     =  2;  { DJGPP }
  GRX_Version_GCC_386_Linux    =  3;  { the real stuff }
  GRX_Version_Generic_X11      =  4;  { generic X11 version }
  GRX_Version_Watcom_DOS4GW    =  5;  { Watcom C++ 11.0 32 Bit }
  GRX_VERSION_GCC_386_WIN32    =  7;  { WIN32 using Mingw32 }
  GRX_VERSION_MSC_386_WIN32    =  8;  { WIN32 using MS-VC }
  GRX_VERSION_GCC_386_CYG32    =  9;  { WIN32 using CYGWIN }
  GRX_VERSION_GCC_386_X11      = 10;  { X11 version }
  GRX_VERSION_GCC_X86_64_LINUX = 11;  { console framebuffer 64 }
  GRX_VERSION_GCC_X86_64_X11   = 12;  { X11 version 64 }

  { available video modes (for 'GrSetMode') }

  Gr_Unknown_Mode                      = -1; { initial state }
  { ============= modes which clear the video memory ============= }
  Gr_80_25_Text                        = 0;  { Extra parameters for GrSetMode: }
  Gr_Default_Text                      = 1;
  Gr_Width_Height_Text                 = 2;  { Integer w,Integer h }
  Gr_Biggest_Text                      = 3;
  Gr_320_200_Graphics                  = 4;
  Gr_Default_Graphics                  = 5;
  Gr_Width_Height_Graphics             = 6;  { Integer w,Integer h }
  Gr_Biggest_Noninterlaced_Graphics    = 7;
  Gr_Biggest_Graphics                  = 8;
  Gr_Width_Height_Color_Graphics       = 9;  { Integer w,Integer h,Cardinal nc }
  Gr_Width_Height_Color_Text           = 10; { Integer w,Integer h,Cardinal nc }
  Gr_Custom_Graphics                   = 11; { Integer w,Integer h,Cardinal nc,Integer vx,Integer vy }
  { ==== equivalent modes which do not clear the video memory ==== }
  Gr_NC_80_25_Text                     = 12;
  Gr_NC_Default_Text                   = 13;
  Gr_NC_Width_Height_Text              = 14; { Integer w,Integer h }
  Gr_NC_Biggest_Text                   = 15;
  Gr_NC_320_200_Graphics               = 16;
  Gr_NC_Default_Graphics               = 17;
  Gr_NC_Width_Height_Graphics          = 18; { Integer w,Integer h }
  Gr_NC_Biggest_Noninterlaced_Graphics = 19;
  Gr_NC_Biggest_Graphics               = 20;
  Gr_NC_Width_Height_Color_Graphics    = 21; { Integer w,Integer h,Cardinal nc }
  Gr_NC_Width_Height_Color_Text        = 22; { Integer w,Integer h,Cardinal nc }
  Gr_NC_Custom_Graphics                = 23; { Integer w,Integer h,Cardinal nc,Integer vx,Integer vy }
  { ==== plane instead of color based modes ==== }
  { colors = 1 << bpp  >>> resort enum for GRX3 <<< }
  Gr_Width_Height_BPP_Graphics         = 24; { Integer w,Integer h,Integer bpp }
  Gr_Width_Height_BPP_Text             = 25; { Integer w,Integer h,Integer bpp }
  Gr_Custom_BPP_Graphics               = 26; { Integer w,Integer h,Integer bpp,Integer vx,Integer vy }
  Gr_NC_Width_Height_BPP_Graphics      = 27; { Integer w,Integer h,Integer bpp }
  Gr_NC_Width_Height_BPP_Text          = 28; { Integer w,Integer h,Integer bpp }
  Gr_NC_Custom_BPP_Graphics            = 29; { Integer w,Integer h,Integer bpp,Integer vx,Integer vy }

  { Available frame modes (video memory layouts) }

  { ====== video frame buffer modes ====== }
  Gr_FrameUndef   =  0;       { undefined }
  Gr_FrameText    =  1;       { text modes }
  Gr_FrameHerc1   =  2;       { Hercules mono }
  Gr_FrameEGAVGA1 =  3;       { EGA VGA mono }
  Gr_FrameEGA4    =  4;       { EGA 16 color }
  Gr_FrameSVGA4   =  5;       { (Super) VGA 16 color }
  Gr_FrameSVGA8   =  6;       { (Super) VGA 256 color }
  Gr_FrameVGA8X   =  7;       { VGA 256 color mode X }
  Gr_FrameSVGA16  =  8;       { Super VGA 32768/65536 color }
  Gr_FrameSVGA24  =  9;       { Super VGA 16M color }
  Gr_FrameSVGA32L = 10;       { Super VGA 16M color padded #1 }
  Gr_FrameSVGA32H = 11;       { Super VGA 16M color padded #2 }
  { ==== modes provided by the X11 driver ===== }
  Gr_FrameXWin1   = Gr_FrameEGAVGA1;
  Gr_FrameXWin4   = Gr_FrameSVGA4;
  Gr_FrameXWin8   = Gr_FrameSVGA8;
  Gr_FrameXWin16  = Gr_FrameSVGA16;
  Gr_FrameXWin24  = Gr_FrameSVGA24;
  Gr_FrameXWin32L = Gr_FrameSVGA32L;
  Gr_FrameXWin32H = Gr_FrameSVGA32H;
  { ==== modes provided by the WIN32 driver ===== }
  Gr_FrameWIN32_1   = Gr_FrameEGAVGA1;
  Gr_FrameWIN32_4   = Gr_FrameSVGA4;
  Gr_FrameWIN32_8   = Gr_FrameSVGA8;
  Gr_FrameWIN32_16  = Gr_FrameSVGA16;
  Gr_FrameWIN32_24  = Gr_FrameSVGA24;
  Gr_FrameWIN32_32L = Gr_FrameSVGA32L;
  Gr_FrameWIN32_32H = Gr_FrameSVGA32H;
  { ==== modes provided by the SDL driver ===== }
  Gr_FrameSDL8   = Gr_FrameSVGA8;
  Gr_FrameSDL16  = Gr_FrameSVGA16;
  Gr_FrameSDL24  = Gr_FrameSVGA24;
  Gr_FrameSDL32L = Gr_FrameSVGA32L;
  Gr_FrameSDL32H = Gr_FrameSVGA32H;
  { ==== linear frame buffer modes  ====== }
  Gr_FrameSVGA8_LFB   = 12;   { (Super) VGA 256 color }
  Gr_FrameSVGA16_LFB  = 13;   { Super VGA 32768/65536 color }
  Gr_FrameSVGA24_LFB  = 14;   { Super VGA 16M color }
  Gr_FrameSVGA32L_LFB = 15;   { Super VGA 16M color padded #1 }
  Gr_FrameSVGA32H_LFB = 16;   { Super VGA 16M color padded #2 }
  { ====== system RAM frame buffer modes ====== }
  Gr_FrameRAM1    = 17;       { mono }
  Gr_FrameRAM4    = 18;       { 16 color planar }
  Gr_FrameRAM8    = 19;       { 256 color }
  Gr_FrameRAM16   = 20;       { 32768/65536 color }
  Gr_FrameRAM24   = 21;       { 16M color }
  Gr_FrameRAM32L  = 22;       { 16M color padded #1 }
  Gr_FrameRAM32H  = 23;       { 16M color padded #2 }
  Gr_FrameRAM3x8  = 24;       { 16M color planar (image mode) }
  { ====== markers for scanning modes ====== }
  Gr_FirstTextFrameMode     = Gr_FrameText;
  Gr_LastTextFrameMode      = Gr_FrameText;
  Gr_FirstGraphicsFrameMode = Gr_FrameHerc1;
  Gr_LastGraphicsFrameMode  = Gr_FrameSVGA32H_LFB;
  Gr_FirstRAMframeMode      = Gr_FrameRAM1;
  Gr_LastRAMframeMode       = Gr_FrameRAM3x8;

  { supported video adapter types }

  Gr_Unknown = -1;          { not known (before driver set) }
  Gr_VGA     =  0;          { VGA adapter }
  Gr_EGA     =  1;          { EGA adapter }
  Gr_Herc    =  2;          { Hercules mono adapter }
  Gr_8514A   =  3;          { 8514A or compatible }
  Gr_S3      =  4;          { S3 graphics accelerator }
  Gr_XWin    =  5;          { X11 driver }
  Gr_WIN32   =  6;          { WIN32 driver }
  Gr_LNXFB   =  7;          { Linux framebuffer }
  Gr_SDL     =  8;          { SDL driver }
  Gr_Mem     =  9;          { memory only driver }

  Gr_Max_Polygon_Pointers = 1000000;
  Gr_Max_Ellipse_Pointers = 1024 + 5;
  Gr_Max_Angle_Value      = 3600;
  Gr_Arc_Style_Open       = 0;
  Gr_Arc_Style_Close1     = 1;
  Gr_Arc_Style_Close2     = 2;

  { bits in the GrVideoDriver.drvflags field: }
  Gr_DriverF_User_Resolution = 1; { set if driver supports user setable arbitrary resolution }

type

  GrVideoModePtr = ^GrVideoMode;

  { The video driver descriptor structure }
  { struct _GR_videoDriver }
  GrVideoDriverPtr = ^GrVideoDriver;
  GrVideoDriver = record
    Name      : CString;           { driver name }
    Adapter   : CInteger;          { adapter type }
    Inherit   : GrVideoDriverPtr;  { inherit video modes from this }
    Modes     : GrVideoModePtr;    { table of supported modes }
    NModes    : CInteger;          { number of modes }
    Detect    : function: CInteger;
    Init      : function (Options: CString): CInteger;
    Reset     : procedure;
    SelectMode: function (Drv: GrVideoDriverPtr; w, h, BPP, txt: CInteger; var ep: CCardinal): GrVideoModePtr;
    DrvFlags  : CCardinal
  end;

  { Video driver mode descriptor structure }
  { struct _GR_videoMode }
  GrVideoMode = record
    Present: ByteBool;        { is it really available? }
    BPP: Byte;                { log2 of # of colors }
    Width, Height,            { video mode geometry }
    Mode: ShortInt;           { BIOS mode number (if any) }
    LineOffset,               { scan line length }
    PrivData: CInteger;       { driver can use it for anything }
    ExtInfo: ^GrVideoModeExt  { extra info (maybe shared) }
  end;

  { Video driver mode descriptor extension structure. This is a separate
    structure accessed via a pointer from the main mode descriptor. The
    reason for this is that frequently several modes can share the same
    extended info. }

  { struct _GR_videoModeExt }
  Int2 = array [0 .. 1] of CInteger;
  GrVideoModeExt = record
    Mode        : CInteger;                    { frame driver for this video mode }
    Drv         : ^GrFrameDriver;              { optional frame driver override }
    Frame       : MemPtr;                      { frame buffer address }
    CPrec       : array [1 .. 3] of ByteCard;  { color component precisions }
    CPos        : array [1 .. 3] of ByteCard;  { color component bit positions }
    Flags       : CInteger;                    { mode flag bits; see "grdriver.h" }
    Setup       : function (var md: GrVideoMode; NoClear: CInteger): CInteger;
    SetVSize    : function (var md: GrVideoMode; w, h: CInteger; var Result: GrVideoMode): CInteger;
    Scroll      : function (var md: GrVideoMode; x, y: CInteger; var Result: Int2): CInteger;
    SetBank     : procedure (bk: CInteger);
    SetRWBanks  : procedure (rb, wb: CInteger);
    LoadColor   : procedure (c, r, g, b: CInteger);
    LFB_Selector: CInteger
  end;

  GrFrameDriverPtr = ^GrFrameDriver;

  GrFrameType = record
    gf_BaseAddr  : array [0 .. 3] of MemPtr;  { base address of frame memory }
    gf_Selector  : ShortInt;                  { frame memory segment selector }
    gf_OnScreen  : ByteBool;                  { is it in video memory ? }
    gf_MemFlags  : Byte;                      { memory allocation flags }
    gf_LineOffset: CInteger;                  { offset to next scan line in bytes }
    gf_Driver    : GrFrameDriverPtr           { frame access functions }
  end;

  { The frame driver descriptor structure. }

  { struct _GR_frameDriver }
  GrFrameDriver = record
    Mode,                         { supported frame access mode }
    RMode,                        { matching RAM frame (if video) }
    Is_Video,                     { video RAM frame driver ? }
    Row_Align,                    { scan line size alignment }
    Num_Planes,                   { number of planes }
    Bits_Per_Pixel    : CInteger; { bits per pixel }
    Max_Plane_Size    : MedInt;   { maximum plane size in bytes }
    Init              : function (var md: GrVideoMode): CInteger;
    ReadPixel         : function (var c: GrFrameType; x, y: CInteger): GrColor;
    DrawPixel         : procedure (x, y: CInteger; c: GrColor);
    DrawLine          : procedure (x, y, dx, dy: CInteger; c: GrColor);
    DrawHLine         : procedure (x, y, w: CInteger; c: GrColor);
    DrawVLine         : procedure (x, y, h: CInteger; c: GrColor);
    DrawBlock         : procedure (x, y, w, h: CInteger; c: GrColor);
    DrawBitmap        : procedure (x, y, w, h: CInteger; BMP: MemPtr; Pitch, Start: CInteger; fg, bg: GrColor);
    DrawPattern       : procedure (x, y, w: CInteger; Patt: Byte; fg, bg: GrColor);
    BitBlt            : procedure (var Dst: GrFrameType; dx, dy: CInteger; var Src: GrFrameType; x, y, w, h: CInteger; Op: GrColor);
    BltV2R            : procedure (var Dst: GrFrameType; dx, dy: CInteger; var Src: GrFrameType; x, y, w, h: CInteger; Op: GrColor);
    BltR2V            : procedure (var Dst: GrFrameType; dx, dy: CInteger; var Src: GrFrameType; x, y, w, h: CInteger; Op: GrColor);
    GetIndexedScanline: function (var c: GrFrameType; x, y, w: CInteger; var Index: CInteger): GrColorsPtr;
    PutScanLine       : procedure (x, y, w: CInteger; scl: GrColorsPtr; Op: GrColor);
  end;

  { driver and mode info structure }

  { extern const struct _GR_driverInfo }
  GrDriverInfoType = record
    VDriver    : GrVideoDriverPtr;              { the current video driver }
    CurMode    : GrVideoModePtr;                { current video mode pointer }
    ActMode    : GrVideoMode;                   { copy of above, resized if virtual }
    FDriver,                                    { frame driver for the current context }
    SDriver,                                    { frame driver for the screen }
    TDriver    : GrFrameDriver;                 { a dummy driver for text modes }
    MCode,                                      { code for the current mode }
    DefTW,DefTH,                                { default text mode size }
    DefGW,DefGH: CInteger;                      { default graphics mode size }
    DefTC,DefGC: GrColor;                       { default text and graphics colors }
    VPosX,VPosY,                                { current virtual viewport position }
    ErrsFatal,                                  { if set, exit upon errors }
    ModeRestore,                                { restore startup video mode if set }
    SplitBanks,                                 { indicates separate R/W banks }
    CurBank    : CInteger;                      { currently mapped bank }
    MdSetHook  : procedure;                     { callback for mode set }
    SetBank    : procedure (bk: CInteger);      { banking routine }
    SetRWBanks : procedure (rb, wb: CInteger);  { split banking routine }
  end;

var
  GrDriverInfo: ^GrDriverInfoType; varasmname 'GrDriverInfo';

{ setup stuff }

function  GrSetDriver(DrvSpec: CString):CInteger; asmname 'GrSetDriver';
function  GrSetMode(m, w, h, nc, vx, vy:CInteger):CInteger; asmname 'GrSetMode';
function  GrSetViewport(XPos, YPos: CInteger):CInteger; asmname 'GrSetViewport';
procedure GrSetModeHook(HookFunc:Pointer); asmname 'GrSetModeHook';
procedure GrSetModeRestore(RestoreFlag:Boolean); asmname 'GrSetModeRestore';

procedure GrSetErrorHandling(ExitIfError:Boolean); asmname 'GrSetErrorHandling';
procedure GrSetEGAVGAmonoDrawnPlane(Plane:CInteger); asmname 'GrSetEGAVGAmonoDrawnPlane';
procedure GrSetEGAVGAmonoShownPlane(Plane:CInteger); asmname 'GrSetEGAVGAmonoShownPlane';

function  GrGetLibraryVersion: CInteger; asmname 'GrGetLibraryVersion';
function  GrGetLibrarySystem: CInteger; asmname 'GrGetLibrarySystem';

{ inquiry stuff ---- many of these can be macros }

function  GrCurrentMode:CInteger; asmname 'GrCurrentMode';
function  GrAdapterType:CInteger; asmname 'GrAdapterType';
function  GrCurrentFrameMode:CInteger; asmname 'GrCurrentFrameMode';
function  GrScreenFrameMode:CInteger; asmname 'GrScreenFrameMode';
function  GrCoreFrameMode:CInteger; asmname 'GrCoreFrameMode';

function  GrCurrentVideoDriver: GrVideoDriverPtr; asmname 'GrCurrentVideoDriver' ;
function  GrCurrentVideoMode:GrVideoModePtr; asmname 'GrCurrentVideoMode';
function  GrVirtualVideoMode:GrVideoModePtr; asmname 'GrVirtualVideoMode';
function  GrCurrentFrameDriver: GrFrameDriverPtr; asmname 'GrCurrentFrameDriver';
function  GrScreenFrameDriver: GrFrameDriverPtr; asmname 'GrScreenFrameDriver';

function  GrFirstVideoMode(FMode:CInteger):GrVideoModePtr; asmname 'GrFirstVideoMode';
function  GrNextVideoMode(Prev:GrVideoModePtr):GrVideoModePtr; asmname 'GrNextVideoMode';

function  GrScreenX:CInteger; asmname 'GrScreenX';
function  GrScreenY:CInteger; asmname 'GrScreenY';
function  GrVirtualX:CInteger; asmname 'GrVirtualX';
function  GrVirtualY:CInteger; asmname 'GrVirtualY';
function  GrViewportX:CInteger; asmname 'GrViewportX';
function  GrViewportY:CInteger; asmname 'GrViewportY';
function  GrScreenIsVirtual:Boolean; asmname 'GrScreenIsVirtual';

{ RAM context geometry and memory allocation inquiry stuff }

function  GrFrameNumPlanes(md:CInteger):CInteger; asmname 'GrFrameNumPlanes';
function  GrFrameLineOffset(md,Width:CInteger):CInteger; asmname 'GrFrameLineOffset';
function  GrFramePlaneSize(md,w,h:CInteger):CInteger; asmname 'GrFramePlaneSize';
function  GrFrameContextSize(md,w,h:CInteger):CInteger; asmname 'GrFrameContextSize';

function  GrNumPlanes:CInteger; asmname 'GrNumPlanes';
function  GrLineOffset(Width:CInteger):CInteger; asmname 'GrLineOffset';
function  GrPlaneSize(w,h:CInteger):CInteger; asmname 'GrPlaneSize';
function  GrContextSize(w,h:CInteger):CInteger; asmname 'GrContextSize';

{ ==================================================================
            FRAME BUFFER, CONTEXT AND CLIPPING STUFF
  ================================================================== }

type
  { struct _GR_context }
  GrContextPtr = ^GrContext;
  GrContext = record
    gc_Frame: GrFrameType;  { frame buffer info }
    gc_Root: GrContextPtr;  { context which owns frame }
    gc_XMax,                { max X coord (width  - 1) }
    gc_YMax,                { max Y coord (height - 1) }
    gc_XOffset,             { X offset from root's base }
    gc_YOffset,             { Y offset from root's base }
    gc_XClipLo,             { low X clipping limit }
    gc_YClipLo,             { low Y clipping limit }
    gc_XClipHi,             { high X clipping limit }
    gc_YClipHi,             { high Y clipping limit }
    gc_UsrXBase,            { user window min X coordinate }
    gc_UsrYBase,            { user window min Y coordinate }
    gc_UsrWidth,            { user window width  }
    gc_UsrHeight: CInteger  { user window height }
  end;

  { extern const struct _GR_contextInfo }
  GrContextInfoType = record
    Current,           { the current context }
    Screen: GrContext  { the screen context }
  end;

var
  GrContextInfo: GrContextInfoType; varasmname 'GrContextInfo';

function  GrCreateContext(w, h: CInteger; Memory: MemPtr; Where: GrContextPtr): GrContextPtr; asmname 'GrCreateContext';
function  GrCreateFrameContext(md: ByteCard; w, h: CInteger; Memory: MemPtr; Where: GrContextPtr): GrContextPtr; asmname 'GrCreateFrameContext';
function  GrCreateSubContext(x1, y1, x2, y2: CInteger; Parent, Where: GrContextPtr): GrContextPtr; asmname 'GrCreateSubContext';
function  GrSaveContext(Where: GrContextPtr): GrContextPtr; asmname 'GrSaveContext';
function  GrCurrentContext: GrContextPtr; asmname 'GrCurrentContext';
function  GrScreenContext: GrContextPtr; asmname 'GrScreenContext';

procedure GrDestroyContext(Context: GrContextPtr); asmname 'GrDestroyContext';
procedure GrResizeSubContext(Context: GrContextPtr; x1, y1, x2, y2: CInteger); asmname 'GrResizeSubContext';
procedure GrSetContext(Context: GrContextPtr); asmname 'GrSetContext';

procedure GrSetClipBox(x1, y1, x2, y2:CInteger); asmname 'GrSetClipBox';
procedure GrGetClipBox(var x1p, y1p, x2p, y2p: CInteger); asmname 'GrGetClipBox';
procedure GrResetClipBox; asmname 'GrResetClipBox';

function  GrMaxX: CInteger; asmname 'GrMaxX';
function  GrMaxY: CInteger; asmname 'GrMaxY';
function  GrSizeX:CInteger; asmname 'GrSizeX';
function  GrSizeY:CInteger; asmname 'GrSizeY';
function  GrLowX: CInteger; asmname 'GrLowX';
function  GrLowY: CInteger; asmname 'GrLowY';
function  GrHighX:CInteger; asmname 'GrHighX';
function  GrHighY:CInteger; asmname 'GrHighY';

{ ==================================================================
                           COLOR STUFF
  ================================================================== }

{ Flags to 'OR' to colors for various operations }
const
  GrWrite      = 0                ;  { write color }
  GrXor        = $01000000        ;  { to "xor" any color to the screen }
  GrOr         = $02000000        ;  { to "or" to the screen }
  GrAnd        = $03000000        ;  { to "and" to the screen }
  GrImage      = $04000000        ;  { BLIT: write, except given color }
  GrCValueMask = $00ffffff        ;  { color value mask }
  GrCModeMask  = $ff000000        ;  { color operation mask }
  GrNoColor    = GrXor or 0       ;  { GrNoColor is used for "no" color }

function  GrColorValue(c:GrColor):GrColor; asmname 'GrColorValue';
function  GrColorMode(c:GrColor):GrColor; asmname 'GrColorMode';
function  GrWriteModeColor(c:GrColor):GrColor; asmname 'GrWriteModeColor';
function  GrXorModeColor(c:GrColor):GrColor; asmname 'GrXorModeColor';
function  GrOrModeColor(c:GrColor):GrColor; asmname 'GrOrModeColor';
function  GrAndModeColor(c:GrColor):GrColor; asmname 'GrAndModeColor';
function  GrImageModeColor(c:GrColor):GrColor; asmname 'GrImageModeColor';

procedure GrResetColors; asmname 'GrResetColors';
procedure GrSetRGBcolorMode; asmname 'GrSetRGBcolorMode';
procedure GrRefreshColors; asmname 'GrRefreshColors';

function  GrNumColors:GrColor; asmname 'GrNumColors';
function  GrNumFreeColors:GrColor; asmname 'GrNumFreeColors';

function  GrBlack:GrColor; asmname 'GrBlack';
function  GrWhite:GrColor; asmname 'GrWhite';

function  GrBuildRGBcolorT(r,g,b:CInteger):GrColor; asmname 'GrBuildRGBcolorT';
function  GrBuildRGBcolorR(r,g,b:CInteger):GrColor; asmname 'GrBuildRGBcolorR';
function  GrRGBcolorRed(c:GrColor):CInteger ; asmname 'GrRGBcolorRed';
function  GrRGBcolorGreen(c:GrColor):CInteger ; asmname 'GrRGBcolorGreen';
function  GrRGBcolorBlue(c:GrColor):CInteger ; asmname 'GrRGBcolorBlue';

function  GrAllocColor(r,g,b:CInteger):GrColor; asmname 'GrAllocColor'; { shared, read-only }
function  GrAllocColorID(r,g,b:CInteger):GrColor; asmname 'GrAllocColorID'; { potentially inlined version }
function  GrAllocColor2(hcolor:MedInt):GrColor; asmname 'GrAllocColor2'; { $RRGGBB shared, read-only }
function  GrAllocColor2ID(hcolor:MedInt):GrColor; asmname 'GrAllocColor2ID'; { potentially inlined version }
function  GrAllocCell:GrColor; asmname 'GrAllocCell'; { unshared, read-write }

function  GrAllocEgaColors:GrColorsPtr; asmname 'GrAllocEgaColors'; { shared, read-only standard EGA colors }

procedure GrSetColor(c:GrColor; r,g,b:CInteger); asmname 'GrSetColor';
procedure GrFreeColor(c:GrColor); asmname 'GrFreeColor';
procedure GrFreeCell(c:GrColor);  asmname 'GrFreeCell';

procedure GrQueryColor(c:GrColor; var r,g,b:CInteger); asmname 'GrQueryColor';
procedure GrQueryColorID(c:GrColor; var r,g,b:CInteger); asmname 'GrQueryColorID';
procedure GrQueryColor2(c:GrColor; var hcolor:MedInt); asmname 'GrQueryColor2';
procedure GrQueryColor2ID(c:GrColor; var hcolor:MedInt); asmname 'GrQueryColor2ID';

function  GrColorSaveBufferSize:CInteger ; asmname 'GrColorSaveBufferSize';
procedure GrSaveColors(Buffer:Pointer); asmname 'GrSaveColors';
procedure GrRestoreColors(Buffer:Pointer); asmname 'GrRestoreColors';

{ ==================================================================
                        GRAPHICS PRIMITIVES
  ================================================================== }

type
  { framed box colors }
  GrFBoxColors = record
    fbx_IntColor,
    fbx_TopColor,
    fbx_RightColor,
    fbx_BottomColor,
    fbx_LeftColor: GrColor
  end;

procedure GrClearScreen(bg:GrColor); asmname 'GrClearScreen';
procedure GrClearContext(bg:GrColor); asmname 'GrClearContext';
procedure GrClearContextC(ctx:GrContextPtr; bg:GrColor); asmname 'GrClearContextC';
procedure GrClearClipBox(bg:GrColor); asmname 'GrClearClipBox';
procedure GrPlot(x, y: CInteger; c:GrColor); asmname 'GrPlot';
procedure GrLine(x1, y1, x2, y2: CInteger; c:GrColor); asmname 'GrLine';
procedure GrHLine(x1, x2, y:CInteger; c:GrColor); asmname 'GrHLine';
procedure GrVLine(x, y1, y2: CInteger; c: GrColor); asmname 'GrVLine';
procedure GrBox(x1, y1, x2, y2: CInteger; c: GrColor); asmname 'GrBox';
procedure GrFilledBox(x1, y1, x2, y2: CInteger; c: GrColor); asmname 'GrFilledBox';
procedure GrFramedBox(x1, y1, x2, y2, Wdt: CInteger; protected var c: GrFBoxColors); asmname 'GrFramedBox';
function  GrGenerateEllipse(xc, yc, xa, ya:CInteger; var Points{ : array of PointType }):CInteger; asmname 'GrGenerateEllipse';
function  GrGenerateEllipseArc(xc, yc, xa, ya, Start, Ende: CInteger; var Points{ : array of PointType }):CInteger; asmname 'GrGenerateEllipseArc';
procedure GrLastArcCoords(var xs, ys, xe, ye, xc, yc: CInteger); asmname 'GrLastArcCoords';
procedure GrCircle(xc, yc, r: CInteger; c: GrColor); asmname 'GrCircle';
procedure GrEllipse(xc, yc, xa, ya: CInteger; c: GrColor); asmname 'GrEllipse';
procedure GrCircleArc(xc, yc, r, Start, Ende, Style: CInteger; c: GrColor); asmname 'GrCircleArc';
procedure GrEllipseArc(xc, yc, xa, ya, Start, Ende, Style: CInteger; c: GrColor); asmname 'GrEllipseArc';
procedure GrFilledCircle(xc, yc, r: CInteger; c: GrColor); asmname 'GrFilledCircle';
procedure GrFilledEllipse(xc, yc, xa, ya: CInteger; c: GrColor); asmname 'GrFilledEllipse';
procedure GrFilledCircleArc(xc, yc, r, Start, Ende, Style: CInteger; c: GrColor); asmname 'GrFilledCircleArc';
procedure GrFilledEllipseArc(xc, yc, xa, ya, Start, Ende, Style: CInteger; c: GrColor); asmname 'GrFilledEllipseArc';
procedure GrPolyLine(NumPts: CInteger; var Points{ : array of PointType }; c: GrColor); asmname 'GrPolyLine';
procedure GrPolygon(NumPts: CInteger; var Points{ : array of PointType }; c: GrColor); asmname 'GrPolygon';
procedure GrFilledConvexPolygon(NumPts: CInteger; var Points{ : array of PointType }; c: GrColor); asmname 'GrFilledConvexPolygon';
procedure GrFilledPolygon(NumPts: CInteger; var Points{ : array of PointType }; c: GrColor); asmname 'GrFilledPolygon';
procedure GrBitBlt(Dest: GrContextPtr; x, y: CInteger; Src: GrContextPtr; x1, y1, x2, y2: CInteger; Op: GrColor); asmname 'GrBitBlt';
function  GrPixel(x, y:CInteger):GrColor; asmname 'GrPixel';
function  GrPixelC(c: GrContextPtr; x, y: CInteger): GrColor; asmname 'GrPixelC';

procedure GrFloodFill(x, y: CInteger; Border, c: GrColor); asmname 'GrFloodFill';
procedure GrFloodSpill(x1, y1, x2, y2: CInteger; old_c, new_c: GrColor); asmname 'GrFloodSpill';
procedure GrFloodSpill2( x1,  y1,  x2,  y2: CInteger; old_c1, new_c1, old_c2, new_c2: GrColor); asmname 'GrFloodSpill2';
procedure GrFloodSpillC(var ctx: GrContext; x1,  y1,  x2,  y2: CInteger; old_c, new_c: GrColor); asmname 'GrFloodSpillC';
procedure GrFloodSpillC2(var ctx: GrContext; x1,  y1,  x2,  y2: CInteger; old_c1, new_c1, old_c2, new_c2: GrColor); asmname 'GrFloodSpillC2';

function GrGetScanline(x1,x2,yy: CInteger): GrColorsPtr; asmname 'GrGetScanline';
function GrGetScanlineC(ctx: GrContextPtr; x1,x2,yy: CInteger): GrColorsPtr; asmname 'GrGetScanlineC';
{ Input   ctx: source context, if NULL the current context is used }
{         x1 : first x coordinate read                             }
{         x2 : last  x coordinate read                             }
{         yy : y coordinate                                        }
{ Output  nil : error / no data (clipping occurred)                }
{         else                                                     }
{           p[0..w]: pixel values read                             }
{                      (w = |x2-y1|)                               }
{           Output data is valid until next GRX call !             }

procedure GrPutScanline(x1,x2,yy: CInteger;c: GrColorsPtr; Op: GrColor); asmname 'GrPutScanline';
{ Input   x1: first x coordinate to be set                         }
{         x2: last  x coordinate to be set                         }
{         yy: y coordinate                                         }
{         c : c[0..(|x2-x1|] hold the pixel data                   }
{         Op: Operation (GrWRITE/GrXOR/GrOR/GrAND/GrIMAGE)         }
{                                                                  }
{ Note    c[..] data must fit GrCVALUEMASK otherwise the results   }
{         are implementation dependend.                            }
{         => You can't supply operation code with the pixel data!  }


{ ==================================================================
        NON CLIPPING DRAWING PRIMITIVES
  ================================================================== }

procedure GrPlotNC(x, y: CInteger; c: GrColor); asmname 'GrPlotNC';
procedure GrLineNC(x1, y1, x2, y2: CInteger; c: GrColor); asmname 'GrLineNC';
procedure GrHLineNC(x1, x2, y: CInteger; c: GrColor); asmname 'GrHLineNC';
procedure GrVLineNC(x, y1, y2: CInteger; c: GrColor); asmname 'GrVLineNC';
procedure GrBoxNC(x1, y1, x2, y2: CInteger; c: GrColor); asmname 'GrBoxNC';
procedure GrFilledBoxNC(x1, y1, x2, y2: CInteger; c: GrColor); asmname 'GrFilledBoxNC';
procedure GrFramedBoxNC(x1, y1, x2, y2, Wdt: CInteger; protected var c: GrFBoxColors); asmname 'GrFramedBoxNC';
procedure GrBitBltNC(Dest: GrContextPtr; x, y: CInteger; Src: GrContextPtr; x1, y1, x2, y2: CInteger; Op: GrColor); asmname 'GrBitBltNC';
function  GrPixelNC(x, y:CInteger):GrColor; asmname 'GrPixelNC';
function  GrPixelCNC(c: GrContextPtr; x, y: CInteger): GrColor; asmname 'GrPixelCNC';

{ ==================================================================
           FONTS AND TEXT PRIMITIVES
  ================================================================== }

const
  { text drawing directions }
  Gr_Text_Right      = 0;  { normal }
  Gr_Text_Down       = 1;  { downward }
  Gr_Text_Left       = 2;  { upside down, right to left }
  Gr_Text_Up         = 3;  { upward }
  Gr_Text_Default    = Gr_Text_Right;
  { Gr_Text_Is_Vertical(d) = ((d) and 1); }

  { text alignment options }
  Gr_Align_Left      = 0;  { X only }
  Gr_Align_Top       = 0;  { Y only }
  Gr_Align_Center    = 1;  { X, Y   }
  Gr_Align_Right     = 2;  { X only }
  Gr_Align_Bottom    = 2;  { Y only }
  Gr_Align_Baseline  = 3;  { Y only }
  Gr_Align_Default   = Gr_Align_Left;

  { character types in text strings }
  Gr_Byte_Text    = 0;  { one byte per character }
  Gr_Word_Text    = 1;  { two bytes per character }
  Gr_Attr_Text    = 2;  { chr w/ PC style attribute byte }

  { OR this to the foreground color value for underlined text when
    using Gr_Byte_Text or Gr_Word_Text modes. }
  Gr_Underline_Text  =  GrXor shl 4;

  { Font conversion flags for 'GrLoadConvertedFont'. OR them as desired. }
  Gr_FontCvt_None       =  0;  { no conversion }
  Gr_FontCvt_SkipChars  =  1;  { load only selected characters }
  Gr_FontCvt_Resize     =  2;  { resize the font }
  Gr_FontCvt_Italicize  =  4;  { tilt font for "italic" look }
  Gr_FontCvt_Boldify    =  8;  { make a "bold"(er) font  }
  Gr_FontCvt_Fixify     = 16;  { convert prop. font to fixed wdt }
  Gr_FontCvt_Proportion = 32;  { convert fixed font to prop. wdt }

{ font structures }
type
  { font descriptor }
  GrFontHeader = record
    Name,                { font name }
    Family: CString;     { font family name }
    Proportional,        { characters have varying width }
    Scalable,            { derived from a scalable font }
    Preloaded,           { set when linked Integero program }
    Modified: ByteBool;  { "tweaked" font (resized, etc..) }
    Width,               { width (proportional=>average) }
    Height,              { font height }
    Baseline,            { baseline pixel pos (from top) }
    ULPos,               { underline pixel pos (from top) }
    ULHeight,            { underline width }
    MinChar,             { lowest character code in font }
    NumChars: CCardinal  { number of characters in font }
  end;

  { character descriptor }
  GrFontChrInfo = record
    Width,             { width of this character }
    Offset: CCardinal  { offset from start of bitmap }
  end;

  { the complete font }
  AuxOffsType = array [0 .. 6] of CInteger;
  GrFont = record
    h: GrFontHeader;        { the font info structure }
    BitMap,                 { character bitmap array }
    AuxMap: MemPtr;         { map for rotated & underline chrs }
    MinWidth,               { width of narrowest character }
    MaxWidth,               { width of widest character }
    AuxSize,                { allocated size of auxiliary map }
    AuxNext: CCardinal;     { next free byte in auxiliary map }
    AuxOffs: ^AuxOffsType;  { offsets to completed aux chars }
    ChrInfo: array [1 .. 1] of GrFontChrInfo  { character info (not act. size) }
  end;
  GrFontPtr = ^GrFont;

var
  GrFont_PC6x8 : GrFont; varasmname 'GrFont_PC6x8';
  GrFont_PC8x8 : GrFont; varasmname 'GrFont_PC8x8';
  GrFont_PC8x14: GrFont; varasmname 'GrFont_PC8x14';
  GrFont_PC8x16: GrFont; varasmname 'GrFont_PC8x16';
  GrDefaultFont: GrFont; varasmname 'GrFont_PC8x14';

function GrLoadFont(FontName: CString): GrFontPtr; asmname 'GrLoadFont';
function GrLoadConvertedFont(FontName: CString; cvt, w, h, MinCh, MaxCh: CInteger): GrFontPtr; asmname 'GrLoadConvertedFont';
function GrBuildConvertedFont(protected var From: GrFont; cvt, w, h, MinCh, MaxCh: CInteger): GrFontPtr; asmname 'GrBuildConvertedFont';

procedure GrUnloadFont(var Font: GrFont); asmname 'GrUnloadFont';
procedure GrDumpFont(protected var f: GrFont; CsymbolName, FileName: CString); asmname 'GrDumpFont';
procedure GrSetFontPath(path_list: CString); asmname 'GrSetFontPath';

function  GrFontCharPresent(protected var Font: GrFont; Chr: CInteger): Boolean; asmname 'GrFontCharPresent';
function  GrFontCharWidth(protected var Font: GrFont; Chr: CInteger): CInteger; asmname 'GrFontCharWidth';
function  GrFontCharHeight(protected var Font: GrFont; Chr: CInteger): CInteger; asmname 'GrFontCharHeight';
function  GrFontCharBmpRowSize(protected var Font: GrFont; Chr: CInteger): CInteger; asmname 'GrFontCharBmpRowSize';
function  GrFontCharBitmapSize(protected var Font: GrFont; Chr: CInteger): CInteger; asmname 'GrFontCharBitmapSize';
function  GrFontStringWidth(protected var Font: GrFont; Text: CString; Len, Typ_e: CInteger): CInteger; asmname 'GrFontStringWidth';
function  GrFontStringHeight(protected var Font: GrFont; Text: CString; Len, Typ_e: CInteger): CInteger; asmname 'GrFontStringHeight';
function  GrProportionalTextWidth(protected var Font: GrFont; Text: CString; Len, Typ_e: CInteger): CInteger; asmname 'GrProportionalTextWidth';
function  GrBuildAuxiliaryBitmap(var Font: GrFont; Chr, Dir, ul: CInteger): MemPtr; asmname 'GrBuildAuxiliaryBitmap';
function  GrFontCharBitmap(protected var Font: GrFont; Chr: CInteger): MemPtr; asmname 'GrFontCharBitmap';
function  GrFontCharAuxBmp(var Font: GrFont; Chr, Dir, ul: CInteger): MemPtr; asmname 'GrFontCharAuxBmp';

type
  GrColorTableP = ^GrColors;

  { text color union }
  GrTextColor = record
  case 1 .. 2 of
    1: (v: GrColor);           { color value for "direct" text }
    2: (p: GrColorTableP);     { color table for attribute text }
  end;

  { text drawing option structure }
  GrTextOption = record
    txo_Font: GrFontPtr;       { font to be used }
    txo_FgColor,               { foreground color }
    txo_BgColor: GrTextColor;  { background color }
    txo_ChrType,               { character type (see above) }
    txo_Direct,                { direction (see above) }
    txo_XAlign,                { X alignment (see above) }
    txo_YAlign: ByteCard       { Y alignment (see above) }
  end;

  { fixed font text window desc. }
  GrTextRegion = record
    txr_Font: GrFontPtr;       { font to be used }
    txr_FgColor,               { foreground color }
    txr_BgColor: GrTextColor;  { background color }
    txr_Buffer,                { pointer to text buffer }
    txr_Backup: Pointer;       { optional backup buffer }
    txr_Width,                 { width of area in chars }
    txr_Height,                { height of area in chars }
    txr_LineOffset,            { offset in buffer(s) between rows }
    txr_XPos,                  { upper left corner X coordinate }
    txr_YPos: CInteger;        { upper left corner Y coordinate }
    txr_ChrType: ByteCard      { character type (see above) }
  end;

function  GrCharWidth(Chr: CInteger; protected var Opt: GrTextOption): CInteger; asmname 'GrCharWidth';
function  GrCharHeight(Chr: CInteger; protected var Opt: GrTextOption): CInteger; asmname 'GrCharHeight';
procedure GrCharSize(Chr: CInteger; protected var Opt: GrTextOption; var w, h: CInteger); asmname 'GrCharSize';
function  GrStringWidth(Text: CString; Length: CInteger; protected var Opt: GrTextOption): CInteger; asmname 'GrStringWidth';
function  GrStringHeight(Text: CString; Length: CInteger; protected var Opt: GrTextOption): CInteger; asmname 'GrStringHeight';
procedure GrStringSize(Text: CString; Length: CInteger; protected var Opt: GrTextOption;var w, h: CInteger); asmname 'GrStringSize';

procedure GrDrawChar(Chr, x, y: CInteger; protected var Opt: GrTextOption); asmname 'GrDrawChar';
procedure GrDrawString(Text: CString; Length, x, y: CInteger; protected var Opt: GrTextOption); asmname 'GrDrawString';
procedure GrTextXY(x, y: CInteger; Text: CString; fg, bg: GrColor); asmname 'GrTextXY';

procedure GrDumpChar(Chr, Col, Row: CInteger; protected var r: GrTextRegion); asmname 'GrDumpChar';
procedure GrDumpText(Col, Row, Wdt, Hgt: CInteger; protected var r: GrTextRegion); asmname 'GrDumpText';
procedure GrDumpTextRegion(protected var r: GrTextRegion); asmname 'GrDumpTextRegion';

{ =================================================================
        THICK AND DASHED LINE DRAWING PRIMITIVES
  ================================================================== }

{ custom line option structure
  zero or one dash pattern length means the line is continuous
  the dash pattern always begins with a drawn section }

type
  GrLineOption = record
    Lno_Color  : GrColor;   { color used to draw line }
    Lno_Width,              { width of the line }
    Lno_PattLen: CInteger;  { length of the dash pattern }
    Lno_DashPat: MemPtr     { draw/nodraw pattern }
  end;

procedure GrCustomLine(x1, y1, x2, y2: CInteger; protected var o: GrLineOption); asmname 'GrCustomLine';
procedure GrCustomBox(x1, y1, x2, y2: CInteger; protected var o: GrLineOption); asmname 'GrCustomBox';
procedure GrCustomCircle(xc, yc, r: CInteger; protected var o: GrLineOption); asmname 'GrCustomCircle';
procedure GrCustomEllipse(xc, yc, r: CInteger; protected var o: GrLineOption); asmname 'GrCustomEllipse';
procedure GrCustomCircleArc(xc, yc, r, Start, Ende, Style: CInteger; protected var o: GrLineOption); asmname 'GrCustomCircleArc';
procedure GrCustomEllipseArc(xc, yc, xa, ya, Start, Ende, Style: CInteger; protected var o: GrLineOption); asmname 'GrCustomEllipseArc';
procedure GrCustomPolyLine(NumPts: CInteger; var Points{ : array of PointType }; protected var o: GrLineOption); asmname 'GrCustomPolyLine';
procedure GrCustomPolygon(NumPts: CInteger; var Points{ : array of PointType }; protected var o: GrLineOption); asmname 'GrCustomPolygon';

{ ==================================================================
           PATTERNED DRAWING AND FILLING PRIMITIVES
  ================================================================== }

{ BITMAP: a mode independent way to specify a fill pattern of two
  colors. It is always 8 pixels wide (1 byte per scan line), its
  height is user-defined. SET THE TYPE FLAG TO ZERO!!! }

type
  GrBitmap = record
    bmp_IsPixMap,           { type flag for pattern union }
    bmp_Height: CInteger;   { bitmap height }
    bmp_Data: MemPtr;       { pointer to the bit pattern }
    bmp_FgColor,            { foreground color for fill }
    bmp_BgColor: GrColor;   { background color for fill }
    bmp_MemFlags: CInteger  { set if dynamically allocated }
  end;

  { PIXMAP: a fill pattern stored in a layout identical to the video RAM
      for filling using 'bitblt'-s. It is mode dependent, typically one
      of the library functions is used to build it. KEEP THE TYPE FLAG
      NONZERO!!! }

  GrPixmap = record
    pxp_IsPixMap,            { type flag for pattern union }
    pxp_Width,               { pixmap width (in pixels)  }
    pxp_Height: CInteger;    { pixmap height (in pixels) }
    pxp_Oper: GrColor;       { bitblt mode (SET, OR, XOR, AND, IMAGE) }
    pxp_Source: GrFrameType  { source context for fill }
  end;

  { Fill pattern union -- can either be a bitmap or a pixmap }

  GrPattern = record
  case 1 .. 3 of
    1: (gp_IsPixMap: CInteger);   { nonzero for pixmaps }
    2: (gp_BitMap  : GrBitmap);  { fill bitmap }
    3: (gp_PixMap  : GrPixmap);  { fill pixmap }
  end;
  GrPatternPtr = ^GrPattern;

  { Draw pattern for line drawings -- specifies both the:
      (1) fill pattern, and the
      (2) custom line drawing option }
  GrLinePattern = record
    lnp_Pattern: GrPatternPtr;  { fill pattern }
    lnp_Option : ^GrLineOption  { width + dash pattern }
  end;
  GrLinePatternPtr = GrLinePattern;

function  GrBuildPixmap(protected var Pixels; w, h: CInteger; protected var Colors { : array of GrColor }): GrPatternPtr; asmname 'GrBuildPixmap';
function  GrBuildPixmapFromBits(protected var Bits; w, h: CInteger; fgc, bgc: GrColor): GrPatternPtr; asmname 'GrBuildPixmapFromBits';
function  GrConvertToPixmap(Src: GrContextPtr): GrPatternPtr; asmname 'GrConvertToPixmap';

procedure GrDestroyPattern(p: GrPatternPtr); asmname 'GrDestroyPattern';

procedure GrPatternedLine(x1, y1, x2, y2: CInteger; lp: GrLinePatternPtr); asmname 'GrPatternedLine';
procedure GrPatternedBox(x1, y1, x2, y2: CInteger; lp: GrLinePatternPtr); asmname 'GrPatternedBox';
procedure GrPatternedCircle(xc, yc, r: CInteger; lp: GrLinePatternPtr); asmname 'GrPatternedCircle';
procedure GrPatternedEllipse(xc, yc, xa, ya: CInteger; lp: GrLinePatternPtr); asmname 'GrPatternedEllipse';
procedure GrPatternedCircleArc(xc, yc, r, Start, Ende, Style: CInteger; lp: GrLinePatternPtr); asmname 'GrPatternedCircleArc';
procedure GrPatternedEllipseArc(xc, yc, xa, ya, Start, Ende, Style: CInteger; lp: GrLinePatternPtr); asmname 'GrPatternedEllipseArc';
procedure GrPatternedPolyLine(NumPts: CInteger; var Points{ : array of PointType }; lp: GrLinePatternPtr); asmname 'GrPatternedPolyLine';
procedure GrPatternedPolygon(NumPts: CInteger; var Points{ : array of PointType }; lp: GrLinePatternPtr); asmname 'GrPatternedPolygon';

procedure GrPatternFilledPlot(x, y: CInteger; p: GrPatternPtr); asmname 'GrPatternFilledPlot';
procedure GrPatternFilledLine(x1, y1, x2, y2: CInteger; p: GrPatternPtr); asmname 'GrPatternFilledLine';
procedure GrPatternFilledBox(x1, y1, x2, y2: CInteger; p: GrPatternPtr); asmname 'GrPatternFilledBox';
procedure GrPatternFilledCircle(xc, yc, r: CInteger; p: GrPatternPtr); asmname 'GrPatternFilledCircle';
procedure GrPatternFilledEllipse(xc, yc, xa, ya: CInteger; p: GrPatternPtr); asmname 'GrPatternFilledEllipse';
procedure GrPatternFilledCircleArc(xc, yc, r, Start, Ende, Style: CInteger; p: GrPatternPtr); asmname 'GrPatternFilledCircleArc';
procedure GrPatternFilledEllipseArc(xc, yc, xa, ya, Start, Ende, Style: CInteger; p: GrPatternPtr); asmname 'GrPatternFilledEllipseArc';
procedure GrPatternFilledConvexPolygon(NumPts: CInteger; var Points{ : array of PointType }; p: GrPatternPtr); asmname 'GrPatternFilledConvexPolygon';
procedure GrPatternFilledPolygon(NumPts: CInteger; var Points{ : array of PointType }; p: GrPatternPtr); asmname 'GrPatternFilledPolygon';
procedure GrPatternFloodFill(x, y: CInteger; Border: GrColor; p: GrPatternPtr); asmname 'GrPatternFloodFill';

procedure GrPatternDrawChar(Chr, x, y: CInteger; protected var Opt: GrTextOption; p: GrPatternPtr); asmname 'GrPatternDrawChar';
procedure GrPatternDrawString(Text: CString; Length, x, y: CInteger; protected var Opt: GrTextOption; p: GrPatternPtr); asmname 'GrPatternDrawString';
procedure GrPatternDrawStringExt(Text: CString; Length, x, y: CInteger; protected var Opt: GrTextOption; p: GrPatternPtr); asmname 'GrPatternDrawStringExt';


{ ==================================================================
                        IMAGE MANIPULATION
  ================================================================== }

{ <image.h>   - Image Utility
                by Michal Stencl Copyright (c) 1998 for GRX
  <e-mail>    - [stenclpmd@ba.telecom.sk] }

type
  GrImagePtr = ^GrPixmap;

{ Flags for GrImageInverse() }
const
  Gr_Image_Inverse_LR = 1;  { inverse left right }
  Gr_Image_Inverse_TD = 2;  { inverse top down   }

function  GrImageBuild(protected var Pixels; w, h: CInteger; protected var Colors { : array of GrColor }): GrImagePtr; asmname 'GrImageBuild';
procedure GrImageDestroy(i: GrImagePtr); asmname 'GrImageDestroy';
procedure GrImageDisplay(x,y: CInteger; i: GrImagePtr); asmname 'GrImageDisplay';
procedure GrImageDisplayExt(x1,y1,x2,y2: CInteger; i: GrImagePtr); asmname 'GrImageDisplayExt';
procedure GrImageFilledBoxAlign(xo,yo,x1,y1,x2,y2: CInteger; p: GrImagePtr); asmname 'GrImageFilledBoxAlign';
procedure GrImageHLineAlign(xo,yo,x,y,Width: CInteger; p: GrImagePtr); asmname 'GrImageHLineAlign';
procedure GrImagePlotAlign(xo,yo,x,y: CInteger; p: GrImagePtr); asmname 'GrImagePlotAlign';

function  GrImageInverse(p: GrImagePtr; Flag: CInteger): GrImagePtr; asmname 'GrImageInverse';
function  GrImageStretch(p: GrImagePtr; NWidth, NHeight: CInteger): GrImagePtr; asmname 'GrImageStretch';

function  GrImageFromPattern(p: GrPatternPtr): GrImagePtr; asmname 'GrImageFromPattern';
function  GrImageFromContext(c: GrContextPtr): GrImagePtr; asmname 'GrImageFromContext';
function  GrImageBuildUsedAsPattern(protected var Pixels; w, h: CInteger; protected var Colors { : array of GrColor }): GrImagePtr; asmname 'GrImageBuildUsedAsPattern';

function  GrPatternFromImage(i: GrImagePtr): GrPatternPtr; asmname 'GrPatternFromImage';

{ ==================================================================
                 DRAWING IN USER WINDOW COORDINATES
  ================================================================== }

procedure GrSetUserWindow(x1, y1, x2, y2: CInteger); asmname 'GrSetUserWindow';
procedure GrGetUserWindow(var x1, y1, x2, y2: CInteger); asmname 'GrGetUserWindow';
procedure GrGetScreenCoord(var x, y: CInteger); asmname 'GrGetScreenCoord';
procedure GrGetUserCoord(var x, y: CInteger); asmname 'GrGetUserCoord';

procedure GrUsrPlot(x, y: CInteger; c:GrColor); asmname 'GrUsrPlot';
procedure GrUsrLine(x1, y1, x2, y2: CInteger; c:GrColor); asmname 'GrUsrLine';
procedure GrUsrHLine(x1, x2, y:CInteger; c:GrColor); asmname 'GrUsrHLine';
procedure GrUsrVLine(x, y1, y2: CInteger; c: GrColor); asmname 'GrUsrVLine';
procedure GrUsrBox(x1, y1, x2, y2: CInteger; c: GrColor); asmname 'GrUsrBox';
procedure GrUsrFilledBox(x1, y1, x2, y2: CInteger; c: GrColor); asmname 'GrUsrFilledBox';
procedure GrUsrFramedBox(x1, y1, x2, y2, Wdt: CInteger; c: GrFBoxColors); asmname 'GrUsrFramedBox';

procedure GrUsrCircle(xc, yc, r: CInteger; c: GrColor); asmname 'GrUsrCircle';
procedure GrUsrEllipse(xc, yc, xa, ya: CInteger; c: GrColor); asmname 'GrUsrEllipse';
procedure GrUsrCircleArc(xc, yc, r, Start, Ende, Style: CInteger; c: GrColor); asmname 'GrUsrCircleArc';
procedure GrUsrEllipseArc(xc, yc, xa, ya, Start, Ende, Style: CInteger; c: GrColor); asmname 'GrUsrEllipseArc';
procedure GrUsrFilledCircle(xc, yc, r: CInteger; c: GrColor); asmname 'GrUsrFilledCircle';
procedure GrUsrFilledEllipse(xc, yc, xa, ya: CInteger; c: GrColor); asmname 'GrUsrFilledEllipse';
procedure GrUsrFilledCircleArc(xc, yc, r, Start, Ende, Style: CInteger; c: GrColor); asmname 'GrUsrFilledCircleArc';
procedure GrUsrFilledEllipseArc(xc, yc, xa, ya, Start, Ende, Style: CInteger; c: GrColor); asmname 'GrUsrFilledEllipseArc';
procedure GrUsrPolyLine(NumPts: CInteger; var Points{ : array of PointType }; c: GrColor); asmname 'GrUsrPolyLine';
procedure GrUsrPolygon(NumPts: CInteger; var Points{ : array of PointType }; c: GrColor); asmname 'GrUsrPolygon';
procedure GrUsrFilledConvexPolygon(NumPts: CInteger; var Points{ : array of PointType }; c: GrColor); asmname 'GrUsrFilledConvexPolygon';
procedure GrUsrFilledPolygon(NumPts: CInteger; var Points{ : array of PointType }; c: GrColor); asmname 'GrUsrFilledPolygon';
procedure GrUsrFloodFill(x, y: CInteger; Border, c: GrColor); asmname 'GrUsrFloodFill';

function  GrUsrPixel(x, y:CInteger):GrColor; asmname 'GrUsrPixel';
function  GrUsrPixelC(c: GrContextPtr; x, y: CInteger):GrColor; asmname 'GrUsrPixelC';

procedure GrUsrCustomLine(x1, y1, x2, y2: CInteger; protected var o: GrLineOption); asmname 'GrUsrCustomLine';
procedure GrUsrCustomBox(x1, y1, x2, y2: CInteger; protected var o: GrLineOption); asmname 'GrUsrCustomBox';
procedure GrUsrCustomCircle(xc, yc, r: CInteger; protected var o: GrLineOption); asmname 'GrUsrCustomCircle';
procedure GrUsrCustomEllipse(xc, yc, r: CInteger; protected var o: GrLineOption); asmname 'GrUsrCustomEllipse';
procedure GrUsrCustomCircleArc(xc, yc, r, Start, Ende, Style: CInteger; protected var o: GrLineOption); asmname 'GrUsrCustomCircleArc';
procedure GrUsrCustomEllipseArc(xc, yc, xa, ya, Start, Ende, Style: CInteger; protected var o: GrLineOption); asmname 'GrUsrCustomEllipseArc';
procedure GrUsrCustomPolyLine(NumPts: CInteger; var Points{ : array of PointType }; protected var o: GrLineOption); asmname 'GrUsrCustomPolyLine';
procedure GrUsrCustomPolygon(NumPts: CInteger; var Points{ : array of PointType }; protected var o: GrLineOption); asmname 'GrUsrCustomPolygon';

procedure GrUsrPatternedLine(x1, y1, x2, y2: CInteger; lp: GrLinePatternPtr); asmname 'GrUsrPatternedLine';
procedure GrUsrPatternedBox(x1, y1, x2, y2: CInteger; lp: GrLinePatternPtr); asmname 'GrUsrPatternedBox';
procedure GrUsrPatternedCircle(xc, yc, r: CInteger; lp: GrLinePatternPtr); asmname 'GrUsrPatternedCircle';
procedure GrUsrPatternedEllipse(xc, yc, xa, ya: CInteger; lp: GrLinePatternPtr); asmname 'GrUsrPatternedEllipse';
procedure GrUsrPatternedCircleArc(xc, yc, r, Start, Ende, Style: CInteger; lp: GrLinePatternPtr); asmname 'GrUsrPatternedCircleArc';
procedure GrUsrPatternedEllipseArc(xc, yc, xa, ya, Start, Ende, Style: CInteger; lp: GrLinePatternPtr); asmname 'GrUsrPatternedEllipseArc';
procedure GrUsrPatternedPolyLine(NumPts: CInteger; var Points{ : array of PointType }; lp: GrLinePatternPtr); asmname 'GrUsrPatternedPolyLine';
procedure GrUsrPatternedPolygon(NumPts: CInteger; var Points{ : array of PointType }; lp: GrLinePatternPtr); asmname 'GrUsrPatternedPolygon';

procedure GrUsrPatternFilledPlot(x, y: CInteger; p: GrPatternPtr); asmname 'GrPatternFilledPlot';
procedure GrUsrPatternFilledLine(x1, y1, x2, y2: CInteger; p: GrPatternPtr); asmname 'GrUsrPatternFilledLine';
procedure GrUsrPatternFilledBox(x1, y1, x2, y2: CInteger; p: GrPatternPtr); asmname 'GrUsrPatternFilledBox';
procedure GrUsrPatternFilledCircle(xc, yc, r: CInteger; p: GrPatternPtr); asmname 'GrUsrPatternFilledCircle';
procedure GrUsrPatternFilledEllipse(xc, yc, xa, ya: CInteger; p: GrPatternPtr); asmname 'GrUsrPatternFilledEllipse';
procedure GrUsrPatternFilledCircleArc(xc, yc, r, Start, Ende, Style: CInteger; p: GrPatternPtr); asmname 'GrUsrPatternFilledCircleArc';
procedure GrUsrPatternFilledEllipseArc(xc, yc, xa, ya, Start, Ende, Style: CInteger; p: GrPatternPtr); asmname 'GrUsrPatternFilledEllipseArc';
procedure GrUsrPatternFilledConvexPolygon(NumPts: CInteger; var Points{ : array of PointType }; p: GrPatternPtr); asmname 'GrUsrPatternFilledConvexPolygon';
procedure GrUsrPatternFilledPolygon(NumPts: CInteger; var Points{ : array of PointType }; p: GrPatternPtr); asmname 'GrUsrPatternFilledPolygon';
procedure GrUsrPatternFloodFill(x, y: CInteger; Border: GrColor; p: GrPatternPtr); asmname 'GrUsrPatternFloodFill';

procedure GrUsrDrawChar(Chr, x, y: CInteger; protected var Opt: GrTextOption); asmname 'GrUsrDrawChar';
procedure GrUsrDrawString(Text: CString; Length, x, y: CInteger; protected var Opt: GrTextOption); asmname 'GrUsrDrawString';
procedure GrUsrTextXY(x, y: CInteger; Text: CString; fg, bg: CInteger); asmname 'GrUsrTextXY';

{ ==================================================================
                  GRAPHICS CURSOR UTILITIES
  ================================================================== }

type
  GrCursor = record
    Work: GrContext;     { work areas (4) }
    XCord,YCord,         { cursor position on screen }
    XSize,YSize,         { cursor size }
    XOffs,YOffs,         { LU corner to hot point offset }
    XWork,YWork,         { save/work area sizes }
    XWPos,YWPos,         { save/work area position on screen }
    Displayed: CInteger  { set if displayed }
  end;
  GrCursorPtr = ^GrCursor;

function  GrBuildCursor(Pixels: Pointer; Pitch, w, h, xo, yo: CInteger; protected var Colors { : array of GrColor }): GrCursorPtr; asmname 'GrBuildCursor';
procedure GrDestroyCursor(Cursor: GrCursorPtr); asmname 'GrDestroyCursor';
procedure GrDisplayCursor(Cursor: GrCursorPtr); asmname 'GrDisplayCursor';
procedure GrEraseCursor(Cursor: GrCursorPtr); asmname 'GrEraseCursor';
procedure GrMoveCursor(Cursor: GrCursorPtr; x, y: CInteger); asmname 'GrMoveCursor';

{ ==================================================================
       MOUSE AND KEYBOARD INPUT UTILITIES
  ================================================================== }

const
  Gr_M_Motion        = $0001;      { mouse event flag bits }
  Gr_M_Left_Down     = $0002;
  Gr_M_Left_Up       = $0004;
  Gr_M_Right_Down    = $0008;
  Gr_M_Right_Up      = $0010;
  Gr_M_Middle_Down   = $0020;
  Gr_M_Middle_Up     = $0040;
  Gr_M_P4_Down       = $0400;
  Gr_M_P4_Up         = $0800;
  Gr_M_P5_Down       = $2000;
  Gr_M_P5_Up         = $4000;
  Gr_M_Button_Down   = Gr_M_Left_Down or Gr_M_Middle_Down or Gr_M_Right_Down or Gr_M_P4_Down or Gr_M_P5_Down;
  Gr_M_Button_Up     = Gr_M_Left_Up   or Gr_M_Middle_Up   or Gr_M_Right_Up   or Gr_M_P4_Up   or Gr_M_P5_Up;
  Gr_M_Button_Change = Gr_M_Button_Up or Gr_M_Button_Down;

  Gr_M_Left          = $01;        { mouse button index bits }
  Gr_M_Right         = $02;
  Gr_M_Middle        = $04;
  Gr_M_P4            = $08;
  Gr_M_P5            = $10;

  Gr_M_KeyPress      = $0080;      { other event flag bits }
  Gr_M_Poll          = $0100;
  Gr_M_NoPaint       = $0200;
  Gr_Command         = $1000;
  Gr_M_Event         = Gr_M_Motion or Gr_M_KeyPress or Gr_M_Button_Change or Gr_Command;

  Gr_KB_RightShift   = $01;    { Keybd states: right shift key depressed }
  Gr_KB_LeftShift    = $02;    { left shift key depressed }
  Gr_KB_Ctrl         = $04;    { CTRL depressed }
  Gr_KB_Alt          = $08;    { ALT depressed }
  Gr_KB_ScrolLock    = $10;    { SCROLL LOCK active }
  Gr_KB_NumLock      = $20;    { NUM LOCK active }
  Gr_KB_CapsLock     = $40;    { CAPS LOCK active }
  Gr_KB_Insert       = $80;    { INSERT state active }
  Gr_KB_Shift        = Gr_KB_LeftShift or Gr_KB_RightShift;

  Gr_M_Cur_Normal    = 0;       { MOUSE CURSOR modes: just the cursor }
  Gr_M_Cur_Rubber    = 1;       { rectangular rubber band (XOR-d to the screen) }
  Gr_M_Cur_Line      = 2;       { line attached to the cursor }
  Gr_M_Cur_Box       = 3;       { rectangular box dragged by the cursor }
  Gr_M_Cur_Cross     = 4;       { crosshair }

  Gr_M_Queue_Size    = 128;     { default queue size }

type
  { mouse event buffer structure }
  GrMouseEvent = record
    Flags,             { event type flags (see above) }
    x, y,              { mouse coordinates }
    Buttons,           { mouse button state }
    Key,               { key code from keyboard }
    KBStat: CInteger;  { keybd status (ALT, CTRL, etc..) }
    DTime: MedInt      { time since last event (msec) }
  end;
  GrMouseEventPtr = ^GrMouseEvent;

function  GrMouseDetect: Boolean; asmname 'GrMouseDetect';
procedure GrMouseEventMode(Dummy: CInteger); asmname 'GrMouseEventMode';
procedure GrMouseInit; asmname 'GrMouseInit';
procedure GrMouseInitN(Queue_Size: CInteger); asmname 'GrMouseInitN';
procedure GrMouseUnInit; asmname 'GrMouseUnInit';
procedure GrMouseSetSpeed(SPMult, SPDiv: CInteger); asmname 'GrMouseSetSpeed';
procedure GrMouseSetAccel(Thresh, Accel: CInteger); asmname 'GrMouseSetAccel';
procedure GrMouseSetLimits(x1, y1, x2, y2: CInteger); asmname 'GrMouseSetLimits';
procedure GrMouseGetLimits(var x1, y1, x2, y2: CInteger); asmname 'GrMouseGetLimits';
procedure GrMouseWarp(x, y: CInteger); asmname 'GrMouseWarp';
procedure GrMouseEventEnable(Enable_KB, Enable_MS: Boolean); asmname 'GrMouseEventEnable';
procedure GrMouseGetEvent(Flags: CInteger; Event: GrMouseEventPtr); asmname 'GrMouseGetEvent';
procedure GrMouseGetEventT(Flags: CInteger; Event: GrMouseEventPtr; timout_msecs: CInteger); asmname 'GrMouseGetEventT';
function  GrMousePendingEvent: CInteger; asmname 'GrMousePendingEvent';

function  GrMouseGetCursor: GrCursorPtr; asmname 'GrMouseGetCursor';
procedure GrMouseSetCursor(Cursor: GrCursorPtr); asmname 'GrMouseSetCursor';
procedure GrMouseSetColors(fg,bg: GrColor); asmname 'GrMouseSetColors';
procedure GrMouseSetCursorMode(Mode: CInteger; x1, y1, x2, y2: CInteger; c: CInteger); asmname 'GrMouseSetCursorMode';
procedure GrMouseDisplayCursor; asmname 'GrMouseDisplayCursor';
procedure GrMouseEraseCursor; asmname 'GrMouseEraseCursor';
procedure GrMouseUpdateCursor; asmname 'GrMouseUpdateCursor';
function  GrMouseCursorIsDisplayed: Boolean; asmname 'GrMouseCursorIsDisplayed';

function  GrMouseBlock(c: GrContextPtr; x1, y1, x2, y2: CInteger): CInteger; asmname 'GrMouseBlock';
procedure GrMouseUnBlock(Return_Value_From_GrMouseBlock: CInteger); asmname 'GrMouseUnBlock';

{ ==================================================================
                         KEYBOARD INTERFACE
  ================================================================== }

const
  GrKey_NoKey              = $0000;  { no key available }
  GrKey_OutsideValidRange  = $0100;  { key typed but code outside 1 .. GrKey_LastDefinedKeycode }

  { standard ASCII key codes }
  GrKey_Control_A          = $0001;
  GrKey_Control_B          = $0002;
  GrKey_Control_C          = $0003;
  GrKey_Control_D          = $0004;
  GrKey_Control_E          = $0005;
  GrKey_Control_F          = $0006;
  GrKey_Control_G          = $0007;
  GrKey_Control_H          = $0008;
  GrKey_Control_I          = $0009;
  GrKey_Control_J          = $000a;
  GrKey_Control_K          = $000b;
  GrKey_Control_L          = $000c;
  GrKey_Control_M          = $000d;
  GrKey_Control_N          = $000e;
  GrKey_Control_O          = $000f;
  GrKey_Control_P          = $0010;
  GrKey_Control_Q          = $0011;
  GrKey_Control_R          = $0012;
  GrKey_Control_S          = $0013;
  GrKey_Control_T          = $0014;
  GrKey_Control_U          = $0015;
  GrKey_Control_V          = $0016;
  GrKey_Control_W          = $0017;
  GrKey_Control_X          = $0018;
  GrKey_Control_Y          = $0019;
  GrKey_Control_Z          = $001a;
  GrKey_Control_LBracket   = $001b;
  GrKey_Control_BackSlash  = $001c;
  GrKey_Control_RBracket   = $001d;
  GrKey_Control_Caret      = $001e;
  GrKey_Control_Underscore = $001f;
  GrKey_Space              = $0020;
  GrKey_ExclamationPoint   = $0021;
  GrKey_DoubleQuote        = $0022;
  GrKey_Hash               = $0023;
  GrKey_Dollar             = $0024;
  GrKey_Percent            = $0025;
  GrKey_Ampersand          = $0026;
  GrKey_Quote              = $0027;
  GrKey_LParen             = $0028;
  GrKey_RParen             = $0029;
  GrKey_Star               = $002a;
  GrKey_Plus               = $002b;
  GrKey_Comma              = $002c;
  GrKey_Dash               = $002d;
  GrKey_Period             = $002e;
  GrKey_Slash              = $002f;
  GrKey_0                  = $0030;
  GrKey_1                  = $0031;
  GrKey_2                  = $0032;
  GrKey_3                  = $0033;
  GrKey_4                  = $0034;
  GrKey_5                  = $0035;
  GrKey_6                  = $0036;
  GrKey_7                  = $0037;
  GrKey_8                  = $0038;
  GrKey_9                  = $0039;
  GrKey_Colon              = $003a;
  GrKey_SemiColon          = $003b;
  GrKey_LAngle             = $003c;
  GrKey_Equals             = $003d;
  GrKey_RAngle             = $003e;
  GrKey_QuestionMark       = $003f;
  GrKey_At                 = $0040;
  GrKey_A                  = $0041;
  GrKey_B                  = $0042;
  GrKey_C                  = $0043;
  GrKey_D                  = $0044;
  GrKey_E                  = $0045;
  GrKey_F                  = $0046;
  GrKey_G                  = $0047;
  GrKey_H                  = $0048;
  GrKey_I                  = $0049;
  GrKey_J                  = $004a;
  GrKey_K                  = $004b;
  GrKey_L                  = $004c;
  GrKey_M                  = $004d;
  GrKey_N                  = $004e;
  GrKey_O                  = $004f;
  GrKey_P                  = $0050;
  GrKey_Q                  = $0051;
  GrKey_R                  = $0052;
  GrKey_S                  = $0053;
  GrKey_T                  = $0054;
  GrKey_U                  = $0055;
  GrKey_V                  = $0056;
  GrKey_W                  = $0057;
  GrKey_X                  = $0058;
  GrKey_Y                  = $0059;
  GrKey_Z                  = $005a;
  GrKey_LBracket           = $005b;
  GrKey_BackSlash          = $005c;
  GrKey_RBracket           = $005d;
  GrKey_Caret              = $005e;
  GrKey_UnderScore         = $005f;
  GrKey_BackQuote          = $0060;
  GrKey_Small_a            = $0061;
  GrKey_Small_b            = $0062;
  GrKey_Small_c            = $0063;
  GrKey_Small_d            = $0064;
  GrKey_Small_e            = $0065;
  GrKey_Small_f            = $0066;
  GrKey_Small_g            = $0067;
  GrKey_Small_h            = $0068;
  GrKey_Small_i            = $0069;
  GrKey_Small_j            = $006a;
  GrKey_Small_k            = $006b;
  GrKey_Small_l            = $006c;
  GrKey_Small_m            = $006d;
  GrKey_Small_n            = $006e;
  GrKey_Small_o            = $006f;
  GrKey_Small_p            = $0070;
  GrKey_Small_q            = $0071;
  GrKey_Small_r            = $0072;
  GrKey_Small_s            = $0073;
  GrKey_Small_t            = $0074;
  GrKey_Small_u            = $0075;
  GrKey_Small_v            = $0076;
  GrKey_Small_w            = $0077;
  GrKey_Small_x            = $0078;
  GrKey_Small_y            = $0079;
  GrKey_Small_z            = $007a;
  GrKey_LBrace             = $007b;
  GrKey_Pipe               = $007c;
  GrKey_RBrace             = $007d;
  GrKey_Tilde              = $007e;
  GrKey_Control_Backspace  = $007f;

  { extended key codes as defined in DJGPP }
  GrKey_Alt_Escape         = $0101;
  GrKey_Control_At         = $0103;
  GrKey_Alt_Backspace      = $010e;
  GrKey_BackTab            = $010f;
  GrKey_Alt_Q              = $0110;
  GrKey_Alt_W              = $0111;
  GrKey_Alt_E              = $0112;
  GrKey_Alt_R              = $0113;
  GrKey_Alt_T              = $0114;
  GrKey_Alt_Y              = $0115;
  GrKey_Alt_U              = $0116;
  GrKey_Alt_I              = $0117;
  GrKey_Alt_O              = $0118;
  GrKey_Alt_P              = $0119;
  GrKey_Alt_LBracket       = $011a;
  GrKey_Alt_RBracket       = $011b;
  GrKey_Alt_Return         = $011c;
  GrKey_Alt_A              = $011e;
  GrKey_Alt_S              = $011f;
  GrKey_Alt_D              = $0120;
  GrKey_Alt_F              = $0121;
  GrKey_Alt_G              = $0122;
  GrKey_Alt_H              = $0123;
  GrKey_Alt_J              = $0124;
  GrKey_Alt_K              = $0125;
  GrKey_Alt_L              = $0126;
  GrKey_Alt_Semicolon      = $0127;
  GrKey_Alt_Quote          = $0128;
  GrKey_Alt_Backquote      = $0129;
  GrKey_Alt_Backslash      = $012b;
  GrKey_Alt_Z              = $012c;
  GrKey_Alt_X              = $012d;
  GrKey_Alt_C              = $012e;
  GrKey_Alt_V              = $012f;
  GrKey_Alt_B              = $0130;
  GrKey_Alt_N              = $0131;
  GrKey_Alt_M              = $0132;
  GrKey_Alt_Comma          = $0133;
  GrKey_Alt_Period         = $0134;
  GrKey_Alt_Slash          = $0135;
  GrKey_Alt_KPStar         = $0137;
  GrKey_F1                 = $013b;
  GrKey_F2                 = $013c;
  GrKey_F3                 = $013d;
  GrKey_F4                 = $013e;
  GrKey_F5                 = $013f;
  GrKey_F6                 = $0140;
  GrKey_F7                 = $0141;
  GrKey_F8                 = $0142;
  GrKey_F9                 = $0143;
  GrKey_F10                = $0144;
  GrKey_Home               = $0147;
  GrKey_Up                 = $0148;
  GrKey_PageUp             = $0149;
  GrKey_Alt_KPMinus        = $014a;
  GrKey_Left               = $014b;
  GrKey_Center             = $014c;
  GrKey_Right              = $014d;
  GrKey_Alt_KPPlus         = $014e;
  GrKey_End                = $014f;
  GrKey_Down               = $0150;
  GrKey_PageDown           = $0151;
  GrKey_Insert             = $0152;
  GrKey_Delete             = $0153;
  GrKey_Shift_F1           = $0154;
  GrKey_Shift_F2           = $0155;
  GrKey_Shift_F3           = $0156;
  GrKey_Shift_F4           = $0157;
  GrKey_Shift_F5           = $0158;
  GrKey_Shift_F6           = $0159;
  GrKey_Shift_F7           = $015a;
  GrKey_Shift_F8           = $015b;
  GrKey_Shift_F9           = $015c;
  GrKey_Shift_F10          = $015d;
  GrKey_Control_F1         = $015e;
  GrKey_Control_F2         = $015f;
  GrKey_Control_F3         = $0160;
  GrKey_Control_F4         = $0161;
  GrKey_Control_F5         = $0162;
  GrKey_Control_F6         = $0163;
  GrKey_Control_F7         = $0164;
  GrKey_Control_F8         = $0165;
  GrKey_Control_F9         = $0166;
  GrKey_Control_F10        = $0167;
  GrKey_Alt_F1             = $0168;
  GrKey_Alt_F2             = $0169;
  GrKey_Alt_F3             = $016a;
  GrKey_Alt_F4             = $016b;
  GrKey_Alt_F5             = $016c;
  GrKey_Alt_F6             = $016d;
  GrKey_Alt_F7             = $016e;
  GrKey_Alt_F8             = $016f;
  GrKey_Alt_F9             = $0170;
  GrKey_Alt_F10            = $0171;
  GrKey_Control_Print      = $0172;
  GrKey_Control_Left       = $0173;
  GrKey_Control_Right      = $0174;
  GrKey_Control_End        = $0175;
  GrKey_Control_PageDown   = $0176;
  GrKey_Control_Home       = $0177;
  GrKey_Alt_1              = $0178;
  GrKey_Alt_2              = $0179;
  GrKey_Alt_3              = $017a;
  GrKey_Alt_4              = $017b;
  GrKey_Alt_5              = $017c;
  GrKey_Alt_6              = $017d;
  GrKey_Alt_7              = $017e;
  GrKey_Alt_8              = $017f;
  GrKey_Alt_9              = $0180;
  GrKey_Alt_0              = $0181;
  GrKey_Alt_Dash           = $0182;
  GrKey_Alt_Equals         = $0183;
  GrKey_Control_PageUp     = $0184;
  GrKey_F11                = $0185;
  GrKey_F12                = $0186;
  GrKey_Shift_F11          = $0187;
  GrKey_Shift_F12          = $0188;
  GrKey_Control_F11        = $0189;
  GrKey_Control_F12        = $018a;
  GrKey_Alt_F11            = $018b;
  GrKey_Alt_F12            = $018c;
  GrKey_Control_Up         = $018d;
  GrKey_Control_KPDash     = $018e;
  GrKey_Control_Center     = $018f;
  GrKey_Control_KPPlus     = $0190;
  GrKey_Control_Down       = $0191;
  GrKey_Control_Insert     = $0192;
  GrKey_Control_Delete     = $0193;
  GrKey_Control_Tab        = $0194;
  GrKey_Control_KPSlash    = $0195;
  GrKey_Control_KPStar     = $0196;
  GrKey_Alt_KPSlash        = $01a4;
  GrKey_Alt_Tab            = $01a5;
  GrKey_Alt_Enter          = $01a6;

  { some additional codes not in DJGPP }
  GrKey_Alt_LAngle         = $01b0;
  GrKey_Alt_RAngle         = $01b1;
  GrKey_Alt_At             = $01b2;
  GrKey_Alt_LBrace         = $01b3;
  GrKey_Alt_Pipe           = $01b4;
  GrKey_Alt_RBrace         = $01b5;
  GrKey_Print              = $01b6;
  GrKey_Shift_Insert       = $01b7;
  GrKey_Shift_Home         = $01b8;
  GrKey_Shift_End          = $01b9;
  GrKey_Shift_PageUp       = $01ba;
  GrKey_Shift_PageDown     = $01bb;
  GrKey_Alt_Up             = $01bc;
  GrKey_Alt_Left           = $01bd;
  GrKey_Alt_Center         = $01be;
  GrKey_Alt_Right          = $01c0;
  GrKey_Alt_Down           = $01c1;
  GrKey_Alt_Insert         = $01c2;
  GrKey_Alt_Delete         = $01c3;
  GrKey_Alt_Home           = $01c4;
  GrKey_Alt_End            = $01c5;
  GrKey_Alt_PageUp         = $01c6;
  GrKey_Alt_PageDown       = $01c7;
  GrKey_Shift_Up           = $01c8;
  GrKey_Shift_Down         = $01c9;
  GrKey_Shift_Right        = $01ca;
  GrKey_Shift_Left         = $01cb;

  { this may be usefull for table allocation ... }
  GrKey_LastDefinedKeycode = GrKey_Shift_Left;

  { some well known synomyms }
  GrKey_BackSpace          = GrKey_Control_H;
  GrKey_Tab                = GrKey_Control_I;
  GrKey_LineFeed           = GrKey_Control_J;
  GrKey_Escape             = GrKey_Control_LBracket;
  GrKey_Return             = GrKey_Control_M;

type
  GrKeyType = ShortInt;

{ new functions to replace the old style
    kbhit / getch / getkey / getxkey / getkbstat
  keyboard interface }
function GrKeyPressed: CInteger; asmname 'GrKeyPressed';
function GrKeyRead: GrKeyType; asmname 'GrKeyRead';
function GrKeyStat: CInteger; asmname 'GrKeyStat';

{ ==================================================================
                MISCELLANEOUS UTILITIY FUNCTIONS
  ================================================================== }

procedure GrResizeGrayMap (var Map; Pitch, ow, oh, nw, nh: CInteger); asmname 'GrResizeGrayMap';
function  GrMatchString (Pattern, Strg: CString): CInteger; asmname 'GrMatchString';
procedure GrSetWindowTitle (Title: CString); asmname 'GrSetWindowTitle';
procedure GrSleep (MSec: CInteger); asmname 'GrSleep';
procedure GrFlush; asmname 'GrFlush';

{ ==================================================================
                            PNM FUNCTIONS
  ================================================================== }

{ The PNM formats, grx support load/save of binaries formats (4,5,6) only }

const
  PlainPBMFormat = 1;
  PlainPGMFormat = 2;
  PlainPPMFormat = 3;
  PBMFormat      = 4;
  PGMFormat      = 5;
  PPMFormat      = 6;

{ The PNM functions }

function GrSaveContextToPbm (grc: GrContextPtr; FileName, Comment: CString): CInteger; asmname 'GrSaveContextToPbm';
function GrSaveContextToPgm (grc: GrContextPtr; FileName, Comment: CString): CInteger; asmname 'GrSaveContextToPgm';
function GrSaveContextToPpm (grc: GrContextPtr; FileName, Comment: CString): CInteger; asmname 'GrSaveContextToPpm';
function GrLoadContextFromPnm (grc: GrContextPtr; FileName: CString): CInteger; asmname 'GrLoadContextFromPnm';
function GrQueryPnm (FileName: CString; var Width, Height, MaxVal: CInteger): CInteger; asmname 'GrQueryPnm';
function GrLoadContextFromPnmBuffer (grc: GrContextPtr; protected var Buffer): CInteger; asmname 'GrLoadContextFromPnmBuffer';
function GrQueryPnmBuffer (protected var Buffer; var Width, Height, MaxVal: CInteger): CInteger; asmname 'GrQueryPnmBuffer';

{ ==================================================================
                             ADDON FUNCTIONS
   these functions may not be installed or available on all systems
  ================================================================== }

function GrPngSupport: CInteger; asmname 'GrPngSupport';
function GrSaveContextToPng (grc: GrContextPtr; FileName: CString): CInteger; asmname 'GrSaveContextToPng';
function GrLoadContextFromPng (grc: GrContextPtr; FileName: CString; UseAlpha: CInteger): CInteger; asmname 'GrLoadContextFromPng';
function GrQueryPng (FileName: CString; var Width, Height: CInteger): CInteger; asmname 'GrQueryPng';

{ SaveContextToTiff - Dump a context in a TIFF file

  Arguments:
    cxt:   Context to be saved (nil -> use current context)
    tiffn: Name of tiff file
    compr: Compression method (see tiff.h), 0: automatic selection
    docn:  string saved in the tiff file (DOCUMENTNAME tag)

   Returns  0 on success
           -1 on error

  requires tifflib by  Sam Leffler (sam@engr.sgi.com)
         available at  ftp://ftp.sgi.com/graphics/tiff }
function SaveContextToTiff(Cxt: GrContextPtr; TiffN: CString; Compr: CInteger; DocN: CString): CInteger; asmname 'SaveContextToTiff';

{ GrSaveContextToJpeg - Dump a context in a JPEG file

  Arguments:
    grc:      Context to be saved (nil -> use current context)
    Filename: Name of the jpeg file
    accuracy: Accuracy percentage (100 for no loss of quality)

   Returns  0 on success
           -1 on error

  requires jpeg-6a by  IJG (Independent JPEG Group)
         available at  ftp.uu.net as graphics/jpeg/jpegsrc.v6a.tar.gz }
function GrSaveContextToJpeg(grc: GrContextPtr; FileName: CString; Accuracy: CInteger): CInteger; asmname 'GrSaveContextToJpeg';
function GrSaveContextToGrayJpeg(grc: GrContextPtr; FileName: CString; Accuracy: CInteger): CInteger; asmname 'GrSaveContextToGrayJpeg';
function GrJpegSupport: CInteger; asmname 'GrJpegSupport';
function GrLoadContextFromJpeg(grc: GrContextPtr; FileName: CString; Scale: CInteger): CInteger; asmname 'GrLoadContextFromJpeg';
function GrQueryJpeg(FileName: CString; var Width, Height: CInteger): CInteger; asmname 'GrQueryJpeg';

implementation

end.
