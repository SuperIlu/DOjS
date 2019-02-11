/**
 ** grx20.h ---- GRX 2.x API functions and data structure declarations
 **
 ** Copyright (c) 1995 Csaba Biegl, 820 Stirrup Dr, Nashville, TN 37221
 ** [e-mail: csaba@vuse.vanderbilt.edu]
 **
 ** This file is part of the GRX graphics library.
 **
 ** The GRX graphics library is free software; you can redistribute it
 ** and/or modify it under some conditions; see the "copying.grx" file
 ** for details.
 **
 ** This library is distributed in the hope that it will be useful,
 ** but WITHOUT ANY WARRANTY; without even the implied warranty of
 ** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 **
 **/

#ifndef __GRX20_H_INCLUDED__
#define __GRX20_H_INCLUDED__

/* ================================================================== */
/*       COMPILER -- CPU -- SYSTEM SPECIFIC VERSION STUFF             */
/* ================================================================== */

/* Version of GRX API
**
** usage:
**    #include <grx20.h>
**    #ifndef GRX_VERSION_API
**    #ifdef  GRX_VERSION
**    #define GRX_VERSION_API 0x0200
**    #else
**    #define GRX_VERSION_API 0x0103
**    #endif
**    #endif
*/
#define GRX_VERSION_API 0x0249

/* these are the supported configurations: */
#define GRX_VERSION_TCC_8086_DOS        1       /* also works with BCC */
#define GRX_VERSION_GCC_386_GO32        2       /* deprecated, don't use it */
#define GRX_VERSION_GCC_386_DJGPP       2       /* DJGPP v2 */
#define GRX_VERSION_GCC_386_LINUX       3       /* the real stuff */
#define GRX_VERSION_GENERIC_X11         4       /* generic X11 version */
#define GRX_VERSION_WATCOM_DOS4GW       5       /* GS - Watcom C++ 11.0 32 Bit */
/*#define GRX_VERSION_WATCOM_REAL_MODE  6*/     /* GS - Watcom C++ 11.0 16 Bit - TODO! */
#define GRX_VERSION_GCC_386_WIN32       7       /* WIN32 using Mingw32 */
#define GRX_VERSION_MSC_386_WIN32       8       /* WIN32 using MS-VC */
#define GRX_VERSION_GCC_386_CYG32       9       /* WIN32 using CYGWIN */
#define GRX_VERSION_GCC_386_X11        10       /* X11 version */
#define GRX_VERSION_GCC_X86_64_LINUX   11       /* console framebuffer 64 */
#define GRX_VERSION_GCC_X86_64_X11     12       /* X11 version 64 */

#define GRXMain main  /* From the 2.4.6 version We don't need this */
                      /* anymore, but it is here for previous apps */

#ifdef  __TURBOC__
#define GRX_VERSION     GRX_VERSION_TCC_8086_DOS
#endif

#ifdef  __GNUC__
#ifdef  __DJGPP__
#define GRX_VERSION     GRX_VERSION_GCC_386_DJGPP
#endif
#if defined(__XWIN__)
#if defined(__linux__) && defined(__i386__)
#define GRX_VERSION     GRX_VERSION_GCC_386_X11
#endif
#if defined(__linux__) && defined(__x86_64__)
#define GRX_VERSION     GRX_VERSION_GCC_X86_64_X11
#endif
#else
#if defined(__linux__) && defined(__i386__)
#define GRX_VERSION     GRX_VERSION_GCC_386_LINUX
#endif
#if defined(__linux__) && defined(__x86_64__)
#define GRX_VERSION     GRX_VERSION_GCC_X86_64_LINUX
#endif
#endif
#ifdef  __WIN32__
#define GRX_VERSION     GRX_VERSION_GCC_386_WIN32
#endif
#ifdef __CYGWIN32__
#define GRX_VERSION     GRX_VERSION_GCC_386_CYG32
#define __WIN32__
#endif
#endif /* __GNUC__ */

#ifdef  __WATCOMC__     /* GS - Watcom C++ 11.0 */
#ifdef  __DOS__
#ifdef  __386__
#define GRX_VERSION     GRX_VERSION_WATCOM_DOS4GW
#else
/* #define GRX_VERSION GRX_VERSION_WATCOM_REAL_MODE  - I haven't tested GRX in 16 bit*/
#endif /* __386__ */
#endif /* __DOS__ */
#endif /* __WATCOMC__ */

#ifdef _MSC_VER
#ifdef _WIN32
#ifdef _M_IX86
#define GRX_VERSION     GRX_VERSION_MSC_386_WIN32
#if !defined(__WIN32__)
#define __WIN32__ _WIN32
#endif
#endif /* _M_IX86  */
#endif /* _WIN32   */
#endif /* _MSC_VER */

#ifndef GRX_VERSION
#if defined(unix) || defined(__unix) || defined(__unix__) || defined(_AIX)
#define GRX_VERSION     GRX_VERSION_GENERIC_X11
#endif
#endif

#ifndef GRX_VERSION
#error  GRX is not supported on your COMPILER/CPU/OPERATING SYSTEM!
#endif

#if (GRX_VERSION==GRX_VERSION_WATCOM_DOS4GW)
#define near
#define far
#define huge
#endif

#if !defined(__TURBOC__) && (GRX_VERSION!=GRX_VERSION_WATCOM_REAL_MODE)
#ifndef near            /* get rid of these stupid keywords */
#define near
#endif
#ifndef far
#define far
#endif
#ifndef huge
#define huge
#endif
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* a couple of forward declarations ... */
typedef struct _GR_frameDriver  GrFrameDriver;
typedef struct _GR_videoDriver  GrVideoDriver;
typedef struct _GR_videoMode    GrVideoMode;
typedef struct _GR_videoModeExt GrVideoModeExt;
typedef struct _GR_frame        GrFrame;
typedef struct _GR_context      GrContext;

/* ================================================================== */
/*                        SYSTEM TYPE DEF's                           */
/* ================================================================== */

/* need unsigned 32 bit integer for color stuff */
#if defined(__TURBOC__) && defined(__MSDOS__)
/* TCC && BCC are 16 bit compilers */
typedef unsigned long int GrColor;
#else
/* all other platforms (GCC on i386 or x86_64 and ALPHA) have 32 bit ints */
typedef unsigned int GrColor;
#endif

/* ================================================================== */
/*                           MODE SETTING                             */
/* ================================================================== */

/*
 * available video modes (for 'GrSetMode')
 */
typedef enum _GR_graphicsModes {
	GR_unknown_mode = (-1),             /* initial state */
	/* ============= modes which clear the video memory ============= */
	GR_80_25_text = 0,                  /* Extra parameters for GrSetMode: */
	GR_default_text,
	GR_width_height_text,               /* int w,int h */
	GR_biggest_text,
	GR_320_200_graphics,
	GR_default_graphics,
	GR_width_height_graphics,           /* int w,int h */
	GR_biggest_noninterlaced_graphics,
	GR_biggest_graphics,
	GR_width_height_color_graphics,     /* int w,int h,GrColor nc */
	GR_width_height_color_text,         /* int w,int h,GrColor nc */
	GR_custom_graphics,                 /* int w,int h,GrColor nc,int vx,int vy */
	/* ==== equivalent modes which do not clear the video memory ==== */
	GR_NC_80_25_text,
	GR_NC_default_text,
	GR_NC_width_height_text,            /* int w,int h */
	GR_NC_biggest_text,
	GR_NC_320_200_graphics,
	GR_NC_default_graphics,
	GR_NC_width_height_graphics,        /* int w,int h */
	GR_NC_biggest_noninterlaced_graphics,
	GR_NC_biggest_graphics,
	GR_NC_width_height_color_graphics,  /* int w,int h,GrColor nc */
	GR_NC_width_height_color_text,      /* int w,int h,GrColor nc */
	GR_NC_custom_graphics,              /* int w,int h,GrColor nc,int vx,int vy */
	/* ==== plane instead of color based modes ==== */
	/* colors = 1 << bpp  >>> resort enum for GRX3 <<< */
	GR_width_height_bpp_graphics,       /* int w,int h,int bpp */
	GR_width_height_bpp_text,           /* int w,int h,int bpp */
	GR_custom_bpp_graphics,             /* int w,int h,int bpp,int vx,int vy */
	GR_NC_width_height_bpp_graphics,    /* int w,int h,int bpp */
	GR_NC_width_height_bpp_text,        /* int w,int h,int bpp */
	GR_NC_custom_bpp_graphics           /* int w,int h,int bpp,int vx,int vy */
} GrGraphicsMode;

/*
 * Available frame modes (video memory layouts)
 */
typedef enum _GR_frameModes {
	/* ====== video frame buffer modes ====== */
	GR_frameUndef,                      /* undefined */
	GR_frameText,                       /* text modes */
	GR_frameHERC1,                      /* Hercules mono */
	GR_frameEGAVGA1,                    /* EGA VGA mono */
	GR_frameEGA4,                       /* EGA 16 color */
	GR_frameSVGA4,                      /* (Super) VGA 16 color */
	GR_frameSVGA8,                      /* (Super) VGA 256 color */
	GR_frameVGA8X,                      /* VGA 256 color mode X */
	GR_frameSVGA16,                     /* Super VGA 32768/65536 color */
	GR_frameSVGA24,                     /* Super VGA 16M color */
	GR_frameSVGA32L,                    /* Super VGA 16M color padded #1 */
	GR_frameSVGA32H,                    /* Super VGA 16M color padded #2 */
	/* ==== modes provided by the X11 driver ===== */
	GR_frameXWIN1   = GR_frameEGAVGA1,
	GR_frameXWIN4   = GR_frameSVGA4,
	GR_frameXWIN8   = GR_frameSVGA8,
	GR_frameXWIN16  = GR_frameSVGA16,
	GR_frameXWIN24  = GR_frameSVGA24,
	GR_frameXWIN32L = GR_frameSVGA32L,
	GR_frameXWIN32H = GR_frameSVGA32H,
	/* ==== modes provided by the WIN32 driver ===== */
	GR_frameWIN32_1   = GR_frameEGAVGA1,
	GR_frameWIN32_4   = GR_frameSVGA4,
	GR_frameWIN32_8   = GR_frameSVGA8,
	GR_frameWIN32_16  = GR_frameSVGA16,
	GR_frameWIN32_24  = GR_frameSVGA24,
	GR_frameWIN32_32L = GR_frameSVGA32L,
	GR_frameWIN32_32H = GR_frameSVGA32H,
	/* ==== modes provided by the SDL driver ===== */
	GR_frameSDL8   = GR_frameSVGA8,
	GR_frameSDL16  = GR_frameSVGA16,
	GR_frameSDL24  = GR_frameSVGA24,
	GR_frameSDL32L = GR_frameSVGA32L,
	GR_frameSDL32H = GR_frameSVGA32H,
	/* ==== linear frame buffer modes  ====== */
	GR_frameSVGA8_LFB,                  /* (Super) VGA 256 color */
	GR_frameSVGA16_LFB,                 /* Super VGA 32768/65536 color */
	GR_frameSVGA24_LFB,                 /* Super VGA 16M color */
	GR_frameSVGA32L_LFB,                /* Super VGA 16M color padded #1 */
	GR_frameSVGA32H_LFB,                /* Super VGA 16M color padded #2 */
	/* ====== system RAM frame buffer modes ====== */
	GR_frameRAM1,                       /* mono */
	GR_frameRAM4,                       /* 16 color planar */
	GR_frameRAM8,                       /* 256 color */
	GR_frameRAM16,                      /* 32768/65536 color */
	GR_frameRAM24,                      /* 16M color */
	GR_frameRAM32L,                     /* 16M color padded #1 */
	GR_frameRAM32H,                     /* 16M color padded #2 */
	GR_frameRAM3x8,                     /* 16M color planar (image mode) */
	/* ====== markers for scanning modes ====== */
	GR_firstTextFrameMode     = GR_frameText,
	GR_lastTextFrameMode      = GR_frameText,
	GR_firstGraphicsFrameMode = GR_frameHERC1,
	GR_lastGraphicsFrameMode  = GR_frameSVGA32H_LFB,
	GR_firstRAMframeMode      = GR_frameRAM1,
	GR_lastRAMframeMode       = GR_frameRAM3x8
} GrFrameMode;

/*
 * supported video adapter types
 */
typedef enum _GR_videoAdapters {
	GR_UNKNOWN = (-1),                  /* not known (before driver set) */
	GR_VGA,                             /* VGA adapter */
	GR_EGA,                             /* EGA adapter */
	GR_HERC,                            /* Hercules mono adapter */
	GR_8514A,                           /* 8514A or compatible */
	GR_S3,                              /* S3 graphics accelerator */
	GR_XWIN,                            /* X11 driver */
	GR_WIN32,                           /* WIN32 driver */
	GR_LNXFB,                           /* Linux framebuffer */
	GR_SDL,                             /* SDL driver */
	GR_MEM                              /* memory only driver */
} GrVideoAdapter;

/*
 * The video driver descriptor structure
 */
struct _GR_videoDriver {
	char   *name;                       /* driver name */
	enum   _GR_videoAdapters adapter;   /* adapter type */
	struct _GR_videoDriver  *inherit;   /* inherit video modes from this */
	struct _GR_videoMode    *modes;     /* table of supported modes */
	int     nmodes;                     /* number of modes */
	int   (*detect)(void);
	int   (*init)(char *options);
	void  (*reset)(void);
	GrVideoMode * (*selectmode)(GrVideoDriver *drv,int w,int h,int bpp,
					int txt,unsigned int *ep);
	unsigned  drvflags;
};
/* bits in the drvflags field: */
#define GR_DRIVERF_USER_RESOLUTION 1
  /* set if driver supports user setable arbitrary resolution */


/*
 * Video driver mode descriptor structure
 */
struct _GR_videoMode {
	char    present;                    /* is it really available? */
	char    bpp;                        /* log2 of # of colors */
	short   width,height;               /* video mode geometry */
	short   mode;                       /* BIOS mode number (if any) */
	int     lineoffset;                 /* scan line length */
	int     privdata;                   /* driver can use it for anything */
	struct _GR_videoModeExt *extinfo;   /* extra info (maybe shared) */
};

/*
 * Video driver mode descriptor extension structure. This is a separate
 * structure accessed via a pointer from the main mode descriptor. The
 * reason for this is that frequently several modes can share the same
 * extended info.
 */
struct _GR_videoModeExt {
	enum   _GR_frameModes   mode;       /* frame driver for this video mode */
	struct _GR_frameDriver *drv;        /* optional frame driver override */
	char    far *frame;                 /* frame buffer address */
	char    cprec[3];                   /* color component precisions */
	char    cpos[3];                    /* color component bit positions */
	int     flags;                      /* mode flag bits; see "grdriver.h" */
	int   (*setup)(GrVideoMode *md,int noclear);
	int   (*setvsize)(GrVideoMode *md,int w,int h,GrVideoMode *result);
	int   (*scroll)(GrVideoMode *md,int x,int y,int result[2]);
	void  (*setbank)(int bk);
	void  (*setrwbanks)(int rb,int wb);
	void  (*loadcolor)(int c,int r,int g,int b);
	int     LFB_Selector;
};

/*
 * The frame driver descriptor structure.
 */
struct _GR_frameDriver {
    enum    _GR_frameModes mode;         /* supported frame access mode */
    enum    _GR_frameModes rmode;        /* matching RAM frame (if video) */
    int      is_video;                   /* video RAM frame driver ? */
    int      row_align;                  /* scan line size alignment */
    int      num_planes;                 /* number of planes */
    int      bits_per_pixel;             /* bits per pixel */
    long     max_plane_size;             /* maximum plane size in bytes */
    int      (*init)(GrVideoMode *md);
    GrColor  (*readpixel)(GrFrame *c,int x,int y);
    void     (*drawpixel)(int x,int y,GrColor c);
    void     (*drawline)(int x,int y,int dx,int dy,GrColor c);
    void     (*drawhline)(int x,int y,int w,GrColor c);
    void     (*drawvline)(int x,int y,int h,GrColor c);
    void     (*drawblock)(int x,int y,int w,int h,GrColor c);
    void     (*drawbitmap)(int x,int y,int w,int h,char far *bmp,int pitch,int start,GrColor fg,GrColor bg);
    void     (*drawpattern)(int x,int y,int w,char patt,GrColor fg,GrColor bg);
    void     (*bitblt)(GrFrame *dst,int dx,int dy,GrFrame *src,int x,int y,int w,int h,GrColor op);
    void     (*bltv2r)(GrFrame *dst,int dx,int dy,GrFrame *src,int x,int y,int w,int h,GrColor op);
    void     (*bltr2v)(GrFrame *dst,int dx,int dy,GrFrame *src,int x,int y,int w,int h,GrColor op);
    /* new functions in v2.3 */
    GrColor far *(*getindexedscanline)(GrFrame *c,int x, int y, int w, int *indx);
      /* will return an array of pixel values pv[] read from frame   */
      /*    if indx == NULL: pv[i=0..w-1] = readpixel(x+i,y)         */
      /*    else             pv[i=0..w-1] = readpixel(x+indx[i],y)   */
    void     (*putscanline)(int x, int y, int w,const GrColor far *scl, GrColor op);
      /** will draw scl[i=0..w-1] to frame:                          */
      /*    if (scl[i] != skipcolor) drawpixel(x+i,y,(scl[i] | op))  */
};

/*
 * driver and mode info structure
 */
extern const struct _GR_driverInfo {
	struct _GR_videoDriver  *vdriver;   /* the current video driver */
	struct _GR_videoMode    *curmode;   /* current video mode pointer */
	struct _GR_videoMode     actmode;   /* copy of above, resized if virtual */
	struct _GR_frameDriver   fdriver;   /* frame driver for the current context */
	struct _GR_frameDriver   sdriver;   /* frame driver for the screen */
	struct _GR_frameDriver   tdriver;   /* a dummy driver for text modes */
	enum   _GR_graphicsModes mcode;     /* code for the current mode */
	int     deftw,defth;                /* default text mode size */
	int     defgw,defgh;                /* default graphics mode size */
	GrColor deftc,defgc;                /* default text and graphics colors */
	int     vposx,vposy;                /* current virtual viewport position */
	int     errsfatal;                  /* if set, exit upon errors */
	int     moderestore;                /* restore startup video mode if set */
	int     splitbanks;                 /* indicates separate R/W banks */
	int     curbank;                    /* currently mapped bank */
	void  (*mdsethook)(void);           /* callback for mode set */
	void  (*setbank)(int bk);           /* banking routine */
	void  (*setrwbanks)(int rb,int wb); /* split banking routine */
} * const GrDriverInfo;

/*
 * setup stuff
 */
int  GrSetDriver(char *drvspec);
int  GrSetMode(GrGraphicsMode which,...);
int  GrSetViewport(int xpos,int ypos);
void GrSetModeHook(void (*hookfunc)(void));
void GrSetModeRestore(int restoreFlag);
void GrSetErrorHandling(int exitIfError);
void GrSetEGAVGAmonoDrawnPlane(int plane);
void GrSetEGAVGAmonoShownPlane(int plane);

unsigned GrGetLibraryVersion(void);
unsigned GrGetLibrarySystem(void);

/*
 * inquiry stuff ---- many of these are actually macros (see below)
 */
GrGraphicsMode GrCurrentMode(void);
GrVideoAdapter GrAdapterType(void);
GrFrameMode    GrCurrentFrameMode(void);
GrFrameMode    GrScreenFrameMode(void);
GrFrameMode    GrCoreFrameMode(void);

const GrVideoDriver *GrCurrentVideoDriver(void);
const GrVideoMode   *GrCurrentVideoMode(void);
const GrVideoMode   *GrVirtualVideoMode(void);
const GrFrameDriver *GrCurrentFrameDriver(void);
const GrFrameDriver *GrScreenFrameDriver(void);
const GrVideoMode   *GrFirstVideoMode(GrFrameMode fmode);
const GrVideoMode   *GrNextVideoMode(const GrVideoMode *prev);

int  GrScreenX(void);
int  GrScreenY(void);
int  GrVirtualX(void);
int  GrVirtualY(void);
int  GrViewportX(void);
int  GrViewportY(void);

int  GrScreenIsVirtual(void);

/*
 * RAM context geometry and memory allocation inquiry stuff
 */
int  GrFrameNumPlanes(GrFrameMode md);
int  GrFrameLineOffset(GrFrameMode md,int width);
long GrFramePlaneSize(GrFrameMode md,int w,int h);
long GrFrameContextSize(GrFrameMode md,int w,int h);

int  GrNumPlanes(void);
int  GrLineOffset(int width);
long GrPlaneSize(int w,int h);
long GrContextSize(int w,int h);

/*
 * inline implementation for some of the above
 */
#ifndef GRX_SKIP_INLINES
#define GrAdapterType()         (GrDriverInfo->vdriver ? GrDriverInfo->vdriver->adapter : GR_UNKNOWN)
#define GrCurrentMode()         (GrDriverInfo->mcode)
#define GrCurrentFrameMode()    (GrDriverInfo->fdriver.mode)
#define GrScreenFrameMode()     (GrDriverInfo->sdriver.mode)
#define GrCoreFrameMode()       (GrDriverInfo->sdriver.rmode)

#define GrCurrentVideoDriver()  ((const GrVideoDriver *)( GrDriverInfo->vdriver))
#define GrCurrentVideoMode()    ((const GrVideoMode   *)( GrDriverInfo->curmode))
#define GrVirtualVideoMode()    ((const GrVideoMode   *)(&GrDriverInfo->actmode))
#define GrCurrentFrameDriver()  ((const GrFrameDriver *)(&GrDriverInfo->fdriver))
#define GrScreenFrameDriver()   ((const GrFrameDriver *)(&GrDriverInfo->sdriver))

#define GrIsFixedMode()      (!(  GrCurrentVideoDriver()->drvflags \
				   & GR_DRIVERF_USER_RESOLUTION))

#define GrScreenX()             (GrCurrentVideoMode()->width)
#define GrScreenY()             (GrCurrentVideoMode()->height)
#define GrVirtualX()            (GrVirtualVideoMode()->width)
#define GrVirtualY()            (GrVirtualVideoMode()->height)
#define GrViewportX()           (GrDriverInfo->vposx)
#define GrViewportY()           (GrDriverInfo->vposy)

#define GrScreenIsVirtual()     ((GrScreenX() + GrScreenY()) < (GrVirtualX() + GrVirtualY()))

#define GrNumPlanes()           GrFrameNumPlanes(GrCoreFrameMode())
#define GrLineOffset(w)         GrFrameLineOffset(GrCoreFrameMode(),w)
#define GrPlaneSize(w,h)        GrFramePlaneSize(GrCoreFrameMode(),w,h)
#define GrContextSize(w,h)      GrFrameContextSize(GrCoreFrameMode(),w,h)
#endif  /* GRX_SKIP_INLINES */


/* ================================================================== */
/*              FRAME BUFFER, CONTEXT AND CLIPPING STUFF              */
/* ================================================================== */

struct _GR_frame {
	char    far *gf_baseaddr[4];        /* base address of frame memory */
	short   gf_selector;                /* frame memory segment selector */
	char    gf_onscreen;                /* is it in video memory ? */
	char    gf_memflags;                /* memory allocation flags */
	int     gf_lineoffset;              /* offset to next scan line in bytes */
	struct _GR_frameDriver *gf_driver;  /* frame access functions */
};

struct _GR_context {
	struct _GR_frame    gc_frame;       /* frame buffer info */
	struct _GR_context *gc_root;        /* context which owns frame */
	int    gc_xmax;                     /* max X coord (width  - 1) */
	int    gc_ymax;                     /* max Y coord (height - 1) */
	int    gc_xoffset;                  /* X offset from root's base */
	int    gc_yoffset;                  /* Y offset from root's base */
	int    gc_xcliplo;                  /* low X clipping limit */
	int    gc_ycliplo;                  /* low Y clipping limit */
	int    gc_xcliphi;                  /* high X clipping limit */
	int    gc_ycliphi;                  /* high Y clipping limit */
	int    gc_usrxbase;                 /* user window min X coordinate */
	int    gc_usrybase;                 /* user window min Y coordinate */
	int    gc_usrwidth;                 /* user window width  */
	int    gc_usrheight;                /* user window height */
#   define gc_baseaddr                  gc_frame.gf_baseaddr
#   define gc_selector                  gc_frame.gf_selector
#   define gc_onscreen                  gc_frame.gf_onscreen
#   define gc_memflags                  gc_frame.gf_memflags
#   define gc_lineoffset                gc_frame.gf_lineoffset
#   define gc_driver                    gc_frame.gf_driver
};

extern const struct _GR_contextInfo {
	struct _GR_context current;         /* the current context */
	struct _GR_context screen;          /* the screen context */
} * const GrContextInfo;

GrContext *GrCreateContext(int w,int h,char far *memory[4],GrContext *where);
GrContext *GrCreateFrameContext(GrFrameMode md,int w,int h,char far *memory[4],GrContext *where);
GrContext *GrCreateSubContext(int x1,int y1,int x2,int y2,const GrContext *parent,GrContext *where);
GrContext *GrSaveContext(GrContext *where);

GrContext *GrCurrentContext(void);
GrContext *GrScreenContext(void);

void  GrDestroyContext(GrContext *context);
void  GrResizeSubContext(GrContext *context,int x1,int y1,int x2,int y2);
void  GrSetContext(const GrContext *context);

void  GrSetClipBox(int x1,int y1,int x2,int y2);
void  GrSetClipBoxC(GrContext *c,int x1,int y1,int x2,int y2);
void  GrGetClipBox(int *x1p,int *y1p,int *x2p,int *y2p);
void  GrGetClipBoxC(const GrContext *c,int *x1p,int *y1p,int *x2p,int *y2p);
void  GrResetClipBox(void);
void  GrResetClipBoxC(GrContext *c);

int   GrMaxX(void);
int   GrMaxY(void);
int   GrSizeX(void);
int   GrSizeY(void);
int   GrLowX(void);
int   GrLowY(void);
int   GrHighX(void);
int   GrHighY(void);

#ifndef GRX_SKIP_INLINES
#define GrCreateContext(w,h,m,c) (GrCreateFrameContext(GrCoreFrameMode(),w,h,m,c))
#define GrCurrentContext()       ((GrContext *)(&GrContextInfo->current))
#define GrScreenContext()        ((GrContext *)(&GrContextInfo->screen))
#define GrMaxX()                 (GrCurrentContext()->gc_xmax)
#define GrMaxY()                 (GrCurrentContext()->gc_ymax)
#define GrSizeX()                (GrMaxX() + 1)
#define GrSizeY()                (GrMaxY() + 1)
#define GrLowX()                 (GrCurrentContext()->gc_xcliplo)
#define GrLowY()                 (GrCurrentContext()->gc_ycliplo)
#define GrHighX()                (GrCurrentContext()->gc_xcliphi)
#define GrHighY()                (GrCurrentContext()->gc_ycliphi)
#define GrGetClipBoxC(C,x1p,y1p,x2p,y2p) do {           \
	*(x1p) = (C)->gc_xcliplo;                           \
	*(y1p) = (C)->gc_ycliplo;                           \
	*(x2p) = (C)->gc_xcliphi;                           \
	*(y2p) = (C)->gc_ycliphi;                           \
} while(0)
#define GrGetClipBox(x1p,y1p,x2p,y2p) do {              \
	*(x1p) = GrLowX();                                  \
	*(y1p) = GrLowY();                                  \
	*(x2p) = GrHighX();                                 \
	*(y2p) = GrHighY();                                 \
} while(0)
#endif  /* GRX_SKIP_INLINES */

/* ================================================================== */
/*                            COLOR STUFF                             */
/* ================================================================== */

/*
 * Flags to 'OR' to colors for various operations
 */
#define GrWRITE         0UL             /* write color */
#define GrXOR           0x01000000UL    /* to "XOR" any color to the screen */
#define GrOR            0x02000000UL    /* to "OR" to the screen */
#define GrAND           0x03000000UL    /* to "AND" to the screen */
#define GrIMAGE         0x04000000UL    /* BLIT: write, except given color */
#define GrCVALUEMASK    0x00ffffffUL    /* color value mask */
#define GrCMODEMASK     0xff000000UL    /* color operation mask */
#define GrNOCOLOR       (GrXOR | 0)     /* GrNOCOLOR is used for "no" color */

GrColor GrColorValue(GrColor c);
GrColor GrColorMode(GrColor c);
GrColor GrWriteModeColor(GrColor c);
GrColor GrXorModeColor(GrColor c);
GrColor GrOrModeColor(GrColor c);
GrColor GrAndModeColor(GrColor c);
GrColor GrImageModeColor(GrColor c);

/*
 * color system info structure (all [3] arrays are [r,g,b])
 */
extern const struct _GR_colorInfo {
	GrColor       ncolors;              /* number of colors */
	GrColor       nfree;                /* number of unallocated colors */
	GrColor       black;                /* the black color */
	GrColor       white;                /* the white color */
	unsigned int  RGBmode;              /* set when RGB mode */
	unsigned int  prec[3];              /* color field precisions */
	unsigned int  pos[3];               /* color field positions */
	unsigned int  mask[3];              /* masks for significant bits */
	unsigned int  round[3];             /* add these for rounding */
	unsigned int  shift[3];             /* shifts for (un)packing color */
	unsigned int  norm;                 /* normalization for (un)packing */
	struct {                            /* color table for non-RGB modes */
		unsigned char r,g,b;            /* loaded components */
		unsigned int  defined:1;        /* r,g,b values are valid if set */
		unsigned int  writable:1;       /* can be changed by 'GrSetColor' */
		unsigned long int nused;        /* usage count */
	} ctable[256];
} * const GrColorInfo;

void    GrResetColors(void);
void    GrSetRGBcolorMode(void);
void    GrRefreshColors(void);

GrColor GrNumColors(void);
GrColor GrNumFreeColors(void);

GrColor GrBlack(void);
GrColor GrWhite(void);

GrColor GrBuildRGBcolorT(int r,int g,int b);
GrColor GrBuildRGBcolorR(int r,int g,int b);
int     GrRGBcolorRed(GrColor c);
int     GrRGBcolorGreen(GrColor c);
int     GrRGBcolorBlue(GrColor c);

GrColor GrAllocColor(int r,int g,int b);   /* shared, read-only */
GrColor GrAllocColorID(int r,int g,int b); /* potentially inlined version */
GrColor GrAllocColor2(long hcolor);        /* shared, read-only, 0xRRGGBB */
GrColor GrAllocColor2ID(long hcolor);      /* potentially inlined version */
GrColor GrAllocCell(void);                 /* unshared, read-write */

GrColor *GrAllocEgaColors(void);           /* shared, read-only standard EGA colors */

void    GrSetColor(GrColor c,int r,int g,int b);
void    GrFreeColor(GrColor c);
void    GrFreeCell(GrColor c);

void    GrQueryColor(GrColor c,int *r,int *g,int *b);
void    GrQueryColorID(GrColor c,int *r,int *g,int *b);
void    GrQueryColor2(GrColor c,long *hcolor);
void    GrQueryColor2ID(GrColor c,long *hcolor);

int     GrColorSaveBufferSize(void);
void    GrSaveColors(void *buffer);
void    GrRestoreColors(void *buffer);

#ifndef GRX_SKIP_INLINES
#define GrColorValue(c)         ((GrColor)(c) & GrCVALUEMASK)
#define GrColorMode(c)          ((GrColor)(c) & GrCMODEMASK)
#define GrWriteModeColor(c)     (GrColorValue(c) | GrWRITE)
#define GrXorModeColor(c)       (GrColorValue(c) | GrXOR)
#define GrOrModeColor(c)        (GrColorValue(c) | GrOR)
#define GrAndModeColor(c)       (GrColorValue(c) | GrAND)
#define GrImageModeColor(c)     (GrColorValue(c) | GrIMAGE)
#define GrNumColors()           (GrColorInfo->ncolors)
#define GrNumFreeColors()       (GrColorInfo->nfree)
#define GrBlack() (                                                            \
	(GrColorInfo->black == GrNOCOLOR) ?                                    \
	(GrBlack)() :                                                          \
	GrColorInfo->black                                                     \
)
#define GrWhite() (                                                            \
	(GrColorInfo->white == GrNOCOLOR) ?                                    \
	(GrWhite)() :                                                          \
	GrColorInfo->white                                                     \
)
#define GrBuildRGBcolorT(r,g,b) ((                                             \
	((GrColor)((int)(r) & GrColorInfo->mask[0]) << GrColorInfo->shift[0]) |\
	((GrColor)((int)(g) & GrColorInfo->mask[1]) << GrColorInfo->shift[1]) |\
	((GrColor)((int)(b) & GrColorInfo->mask[2]) << GrColorInfo->shift[2])  \
	) >> GrColorInfo->norm                                                 \
)
#define GrBuildRGBcolorR(r,g,b) GrBuildRGBcolorT(                              \
	(((unsigned int)(r)) > GrColorInfo->mask[0]) ? 255 : (unsigned int)(r) + GrColorInfo->round[0], \
	(((unsigned int)(g)) > GrColorInfo->mask[1]) ? 255 : (unsigned int)(g) + GrColorInfo->round[1], \
	(((unsigned int)(b)) > GrColorInfo->mask[2]) ? 255 : (unsigned int)(b) + GrColorInfo->round[2]  \
)
#define GrRGBcolorRed(c) (                                                     \
	(int)(((GrColor)(c) << GrColorInfo->norm) >> GrColorInfo->shift[0]) &  \
	(GrColorInfo->mask[0])                                                 \
)
#define GrRGBcolorGreen(c) (                                                   \
	(int)(((GrColor)(c) << GrColorInfo->norm) >> GrColorInfo->shift[1]) &  \
	(GrColorInfo->mask[1])                                                 \
)
#define GrRGBcolorBlue(c) (                                                    \
	(int)(((GrColor)(c) << GrColorInfo->norm) >> GrColorInfo->shift[2]) &  \
	(GrColorInfo->mask[2])                                                 \
)
#define GrAllocColorID(r,g,b) (GrColorInfo->RGBmode ?                          \
	GrBuildRGBcolorR(r,g,b) :                                              \
	GrAllocColor(r,g,b)                                                    \
)
#define GrAllocColor2(hcolor) (GrAllocColor(                                   \
        ((hcolor & 0xff0000) >> 16),                                           \
        ((hcolor & 0x00ff00) >> 8),                                            \
        (hcolor & 0x0000ff))                                                   \
)
#define GrAllocColor2ID(hcolor) (GrAllocColorID(                               \
        ((hcolor & 0xff0000) >> 16),                                           \
        ((hcolor & 0x00ff00) >> 8),                                            \
        (hcolor & 0x0000ff))                                                   \
)
#define GrQueryColorID(c,r,g,b) do {                                           \
	if(GrColorInfo->RGBmode) {                                             \
	*(r) = GrRGBcolorRed(c);                                               \
	*(g) = GrRGBcolorGreen(c);                                             \
	*(b) = GrRGBcolorBlue(c);                                              \
	break;                                                                 \
	}                                                                      \
	if(((GrColor)(c) < GrColorInfo->ncolors) &&                            \
	   (GrColorInfo->ctable[(GrColor)(c)].defined)) {                      \
	*(r) = GrColorInfo->ctable[(GrColor)(c)].r;                            \
	*(g) = GrColorInfo->ctable[(GrColor)(c)].g;                            \
	*(b) = GrColorInfo->ctable[(GrColor)(c)].b;                            \
	break;                                                                 \
	}                                                                      \
	*(r) = *(g) = *(b) = 0;                                                \
} while(0)
#define GrQueryColor2ID(c,hcolor) do {                                         \
	if(GrColorInfo->RGBmode) {                                             \
        *(hcolor) = GrRGBcolorRed(c) << 16;                                    \
        *(hcolor) |= GrRGBcolorGreen(c) << 8;                                  \
        *(hcolor) |= GrRGBcolorBlue(c);                                        \
	break;                                                                 \
	}                                                                      \
	if(((GrColor)(c) < GrColorInfo->ncolors) &&                            \
	   (GrColorInfo->ctable[(GrColor)(c)].defined)) {                      \
        *(hcolor) = GrColorInfo->ctable[(GrColor)(c)].r;                       \
        *(hcolor) = GrColorInfo->ctable[(GrColor)(c)].g;                       \
        *(hcolor) = GrColorInfo->ctable[(GrColor)(c)].b;                       \
	break;                                                                 \
	}                                                                      \
        *(hcolor) = 0;                                                         \
} while(0)
#endif  /* GRX_SKIP_INLINES */

/*
 * color table (for primitives using several colors):
 *   it is an array of colors with the first element being
 *   the number of colors in the table
 */
typedef GrColor *GrColorTableP;

#define GR_CTABLE_SIZE(table) (                                                \
	(table) ? (unsigned int)((table)[0]) : 0U                              \
)
#define GR_CTABLE_COLOR(table,index) (                                         \
	((unsigned)(index) < GR_CTABLE_SIZE(table)) ?                          \
	(table)[((unsigned)(index)) + 1] :                                     \
	GrNOCOLOR                                                              \
)
#define GR_CTABLE_ALLOCSIZE(ncolors)    ((ncolors) + 1)

/* ================================================================== */
/*                       GRAPHICS PRIMITIVES                          */
/* ================================================================== */

#ifdef  __TURBOC__
/* this is for GRX compiled with SMALL_STACK: */
#define GR_MAX_POLYGON_POINTS   (8192)
#define GR_MAX_ELLIPSE_POINTS   (1024 + 5)
/* old values without SMALL_STACK: */
/* #define GR_MAX_POLYGON_POINTS   (512) */
/* #define GR_MAX_ELLIPSE_POINTS   (256 + 5) */
#else
#define GR_MAX_POLYGON_POINTS   (1000000)
#define GR_MAX_ELLIPSE_POINTS   (1024 + 5)
#endif
#define GR_MAX_ANGLE_VALUE      (3600)
#define GR_ARC_STYLE_OPEN       0
#define GR_ARC_STYLE_CLOSE1     1
#define GR_ARC_STYLE_CLOSE2     2

typedef struct {                        /* framed box colors */
	GrColor fbx_intcolor;
	GrColor fbx_topcolor;
	GrColor fbx_rightcolor;
	GrColor fbx_bottomcolor;
	GrColor fbx_leftcolor;
} GrFBoxColors;

void GrClearScreen(GrColor bg);
void GrClearContext(GrColor bg);
void GrClearContextC(GrContext *ctx, GrColor bg);
void GrClearClipBox(GrColor bg);
void GrPlot(int x,int y,GrColor c);
void GrLine(int x1,int y1,int x2,int y2,GrColor c);
void GrHLine(int x1,int x2,int y,GrColor c);
void GrVLine(int x,int y1,int y2,GrColor c);
void GrBox(int x1,int y1,int x2,int y2,GrColor c);
void GrFilledBox(int x1,int y1,int x2,int y2,GrColor c);
void GrFramedBox(int x1,int y1,int x2,int y2,int wdt,const GrFBoxColors *c);
int  GrGenerateEllipse(int xc,int yc,int xa,int ya,int points[GR_MAX_ELLIPSE_POINTS][2]);
int  GrGenerateEllipseArc(int xc,int yc,int xa,int ya,int start,int end,int points[GR_MAX_ELLIPSE_POINTS][2]);
void GrLastArcCoords(int *xs,int *ys,int *xe,int *ye,int *xc,int *yc);
void GrCircle(int xc,int yc,int r,GrColor c);
void GrEllipse(int xc,int yc,int xa,int ya,GrColor c);
void GrCircleArc(int xc,int yc,int r,int start,int end,int style,GrColor c);
void GrEllipseArc(int xc,int yc,int xa,int ya,int start,int end,int style,GrColor c);
void GrFilledCircle(int xc,int yc,int r,GrColor c);
void GrFilledEllipse(int xc,int yc,int xa,int ya,GrColor c);
void GrFilledCircleArc(int xc,int yc,int r,int start,int end,int style,GrColor c);
void GrFilledEllipseArc(int xc,int yc,int xa,int ya,int start,int end,int style,GrColor c);
void GrPolyLine(int numpts,int points[][2],GrColor c);
void GrPolygon(int numpts,int points[][2],GrColor c);
void GrFilledConvexPolygon(int numpts,int points[][2],GrColor c);
void GrFilledPolygon(int numpts,int points[][2],GrColor c);
void GrBitBlt(GrContext *dst,int x,int y,GrContext *src,int x1,int y1,int x2,int y2,GrColor op);
void GrBitBlt1bpp(GrContext *dst,int dx,int dy,GrContext *src,int x1,int y1,int x2,int y2,GrColor fg,GrColor bg);
void GrFloodFill(int x, int y, GrColor border, GrColor c);
void GrFloodSpill(int x1, int y1, int x2, int y2, GrColor old_c, GrColor new_c);
void GrFloodSpill2(int x1, int y1, int x2, int y2, GrColor old_c1, GrColor new_c1, GrColor old_c2, GrColor new_c2);
void GrFloodSpillC(GrContext *ctx, int x1, int y1, int x2, int y2, GrColor old_c, GrColor new_c);
void GrFloodSpillC2(GrContext *ctx, int x1, int y1, int x2, int y2, GrColor old_c1, GrColor new_c1, GrColor old_c2, GrColor new_c2);

GrColor GrPixel(int x,int y);
GrColor GrPixelC(GrContext *c,int x,int y);

const GrColor *GrGetScanline(int x1,int x2,int yy);
const GrColor *GrGetScanlineC(GrContext *ctx,int x1,int x2,int yy);
/* Input   ctx: source context, if NULL the current context is used */
/*         x1 : first x coordinate read                             */
/*         x2 : last  x coordinate read                             */
/*         yy : y coordinate                                        */
/* Output  NULL     : error / no data (clipping occured)            */
/*         else                                                     */
/*           p[0..w]: pixel values read                             */
/*                      (w = |x2-y1|)                               */
/*           Output data is valid until next GRX call !             */

void GrPutScanline(int x1,int x2,int yy,const GrColor *c, GrColor op);
/* Input   x1 : first x coordinate to be set                        */
/*         x2 : last  x coordinate to be set                        */
/*         yy : y coordinate                                        */
/*         c  : c[0..(|x2-x1|] hold the pixel data                  */
/*         op : Operation (GrWRITE/GrXOR/GrOR/GrAND/GrIMAGE)        */
/*                                                                  */
/* Note    c[..] data must fit GrCVALUEMASK otherwise the results   */
/*         are implementation dependend.                            */
/*         => You can't supply operation code with the pixel data!  */


#ifndef GRX_SKIP_INLINES
#define GrGetScanline(x1,x2,yy) \
	GrGetScanlineC(NULL,(x1),(x2),(yy))
#endif

/* ================================================================== */
/*                 NON CLIPPING DRAWING PRIMITIVES                    */
/* ================================================================== */

void GrPlotNC(int x,int y,GrColor c);
void GrLineNC(int x1,int y1,int x2,int y2,GrColor c);
void GrHLineNC(int x1,int x2,int y,GrColor c);
void GrVLineNC(int x,int y1,int y2,GrColor c);
void GrBoxNC(int x1,int y1,int x2,int y2,GrColor c);
void GrFilledBoxNC(int x1,int y1,int x2,int y2,GrColor c);
void GrFramedBoxNC(int x1,int y1,int x2,int y2,int wdt,const GrFBoxColors *c);
void GrBitBltNC(GrContext *dst,int x,int y,GrContext *src,int x1,int y1,int x2,int y2,GrColor op);

GrColor GrPixelNC(int x,int y);
GrColor GrPixelCNC(GrContext *c,int x,int y);

#ifndef GRX_SKIP_INLINES
#define GrPlotNC(x,y,c) (                                                      \
	(*GrCurrentFrameDriver()->drawpixel)(                                  \
	((x) + GrCurrentContext()->gc_xoffset),                                \
	((y) + GrCurrentContext()->gc_yoffset),                                \
	((c))                                                                  \
	)                                                                      \
)
#define GrPixelNC(x,y) (                                                       \
	(*GrCurrentFrameDriver()->readpixel)(                                  \
	(GrFrame *)(&GrCurrentContext()->gc_frame),                            \
	((x) + GrCurrentContext()->gc_xoffset),                                \
	((y) + GrCurrentContext()->gc_yoffset)                                 \
	)                                                                      \
)
#define GrPixelCNC(c,x,y) (                                                    \
	(*(c)->gc_driver->readpixel)(                                          \
	(&(c)->gc_frame),                                                      \
	((x) + (c)->gc_xoffset),                                               \
	((y) + (c)->gc_yoffset)                                                \
	)                                                                      \
)
#endif  /* GRX_SKIP_INLINES */

/* ================================================================== */
/*                   FONTS AND TEXT PRIMITIVES                        */
/* ================================================================== */

/*
 * text drawing directions
 */
#define GR_TEXT_RIGHT           0       /* normal */
#define GR_TEXT_DOWN            1       /* downward */
#define GR_TEXT_LEFT            2       /* upside down, right to left */
#define GR_TEXT_UP              3       /* upward */
#define GR_TEXT_DEFAULT         GR_TEXT_RIGHT
#define GR_TEXT_IS_VERTICAL(d)  ((d) & 1)

/*
 * text alignment options
 */
#define GR_ALIGN_LEFT           0       /* X only */
#define GR_ALIGN_TOP            0       /* Y only */
#define GR_ALIGN_CENTER         1       /* X, Y   */
#define GR_ALIGN_RIGHT          2       /* X only */
#define GR_ALIGN_BOTTOM         2       /* Y only */
#define GR_ALIGN_BASELINE       3       /* Y only */
#define GR_ALIGN_DEFAULT        GR_ALIGN_LEFT

/*
 * character types in text strings
 */
#define GR_BYTE_TEXT            0       /* one byte per character */
#define GR_WORD_TEXT            1       /* two bytes per character */
#define GR_ATTR_TEXT            2       /* chr w/ PC style attribute byte */

/*
 * macros to access components of various string/character types
 */
#define GR_TEXTCHR_SIZE(ty)     (((ty) == GR_BYTE_TEXT) ? sizeof(char) : sizeof(short))
#define GR_TEXTCHR_CODE(ch,ty)  (((ty) == GR_WORD_TEXT) ? (unsigned short)(ch) : (unsigned char)(ch))
#define GR_TEXTCHR_ATTR(ch,ty)  (((ty) == GR_ATTR_TEXT) ? ((unsigned short)(ch) >> 8) : 0)
#define GR_TEXTSTR_CODE(pt,ty)  (((ty) == GR_WORD_TEXT) ? ((unsigned short *)(pt))[0] : ((unsigned char *)(pt))[0])
#define GR_TEXTSTR_ATTR(pt,ty)  (((ty) == GR_ATTR_TEXT) ? ((unsigned char *)(pt))[1] : 0)

/*
 * text attribute macros for the GR_ATTR_TEXT type
 * _GR_textattrintensevideo drives if the eighth bit is used for
 * underline (false, default) or more background colors (true)
 */
extern int _GR_textattrintensevideo;

#define GR_BUILD_ATTR(fg,bg,ul) (_GR_textattrintensevideo ? \
                                (((fg) & 15) | (((bg) & 15) << 4)) \
                                : \
                                (((fg) & 15) | (((bg) & 7) << 4) | ((ul) ? 128 : 0)) \
                                )
#define GR_ATTR_FGCOLOR(attr)   (((attr)     ) &  15)
#define GR_ATTR_BGCOLOR(attr)   (_GR_textattrintensevideo ? \
                                (((attr) >> 4) &  15) \
                                : \
                                (((attr) >> 4) &   7) \
                                )
#define GR_ATTR_UNDERLINE(attr) (_GR_textattrintensevideo ? \
                                (0) \
                                : \
                                (((attr)     ) & 128) \
                                )

/*
 * OR this to the foreground color value for underlined text when
 * using GR_BYTE_TEXT or GR_WORD_TEXT modes.
 */
#define GR_UNDERLINE_TEXT       (GrXOR << 4)

/*
 * Font conversion flags for 'GrLoadConvertedFont'. OR them as desired.
 */
#define GR_FONTCVT_NONE         0       /* no conversion */
#define GR_FONTCVT_SKIPCHARS    1       /* load only selected characters */
#define GR_FONTCVT_RESIZE       2       /* resize the font */
#define GR_FONTCVT_ITALICIZE    4       /* tilt font for "italic" look */
#define GR_FONTCVT_BOLDIFY      8       /* make a "bold"(er) font  */
#define GR_FONTCVT_FIXIFY       16      /* convert prop. font to fixed wdt */
#define GR_FONTCVT_PROPORTION   32      /* convert fixed font to prop. wdt */

/*
 * font structures
 */
typedef struct _GR_fontHeader {         /* font descriptor */
	char    *name;                      /* font name */
	char    *family;                    /* font family name */
	char     proportional;              /* characters have varying width */
	char     scalable;                  /* derived from a scalable font */
	char     preloaded;                 /* set when linked into program */
	char     modified;                  /* "tweaked" font (resized, etc..) */
	unsigned int  width;                /* width (proportional=>average) */
	unsigned int  height;               /* font height */
	unsigned int  baseline;             /* baseline pixel pos (from top) */
	unsigned int  ulpos;                /* underline pixel pos (from top) */
	unsigned int  ulheight;             /* underline width */
	unsigned int  minchar;              /* lowest character code in font */
	unsigned int  numchars;             /* number of characters in font */
} GrFontHeader;

typedef struct _GR_fontChrInfo {        /* character descriptor */
	unsigned int  width;                /* width of this character */
	unsigned int  offset;               /* offset from start of bitmap */
} GrFontChrInfo;

typedef struct _GR_font {               /* the complete font */
	struct  _GR_fontHeader  h;          /* the font info structure */
	char     far *bitmap;               /* character bitmap array */
	char     far *auxmap;               /* map for rotated & underline chrs */
	unsigned int  minwidth;             /* width of narrowest character */
	unsigned int  maxwidth;             /* width of widest character */
	unsigned int  auxsize;              /* allocated size of auxiliary map */
	unsigned int  auxnext;              /* next free byte in auxiliary map */
	unsigned int  far      *auxoffs[7]; /* offsets to completed aux chars */
	struct  _GR_fontChrInfo chrinfo[1]; /* character info (not act. size) */
} GrFont;

extern  GrFont          GrFont_PC6x8;
extern  GrFont          GrFont_PC8x8;
extern  GrFont          GrFont_PC8x14;
extern  GrFont          GrFont_PC8x16;
#define GrDefaultFont   GrFont_PC8x14

GrFont *GrLoadFont(char *name);
GrFont *GrLoadConvertedFont(char *name,int cvt,int w,int h,int minch,int maxch);
GrFont *GrBuildConvertedFont(const GrFont *from,int cvt,int w,int h,int minch,int maxch);

void GrUnloadFont(GrFont *font);
void GrDumpFont(const GrFont *f,char *CsymbolName,char *fileName);
void GrDumpFnaFont(const GrFont *f, char *fileName);
void GrSetFontPath(char *path_list);

int  GrFontCharPresent(const GrFont *font,int chr);
int  GrFontCharWidth(const GrFont *font,int chr);
int  GrFontCharHeight(const GrFont *font,int chr);
int  GrFontCharBmpRowSize(const GrFont *font,int chr);
int  GrFontCharBitmapSize(const GrFont *font,int chr);
int  GrFontStringWidth(const GrFont *font,void *text,int len,int type);
int  GrFontStringHeight(const GrFont *font,void *text,int len,int type);
int  GrProportionalTextWidth(const GrFont *font,const void *text,int len,int type);

char far *GrBuildAuxiliaryBitmap(GrFont *font,int chr,int dir,int ul);
char far *GrFontCharBitmap(const GrFont *font,int chr);
char far *GrFontCharAuxBmp(GrFont *font,int chr,int dir,int ul);

typedef union _GR_textColor {           /* text color union */
	GrColor       v;                    /* color value for "direct" text */
	GrColorTableP p;                    /* color table for attribute text */
} GrTextColor;

typedef struct _GR_textOption {         /* text drawing option structure */
	struct _GR_font     *txo_font;      /* font to be used */
	union  _GR_textColor txo_fgcolor;   /* foreground color */
	union  _GR_textColor txo_bgcolor;   /* background color */
	char    txo_chrtype;                /* character type (see above) */
	char    txo_direct;                 /* direction (see above) */
	char    txo_xalign;                 /* X alignment (see above) */
	char    txo_yalign;                 /* Y alignment (see above) */
} GrTextOption;

typedef struct {                        /* fixed font text window desc. */
	struct _GR_font     *txr_font;      /* font to be used */
	union  _GR_textColor txr_fgcolor;   /* foreground color */
	union  _GR_textColor txr_bgcolor;   /* background color */
	void   *txr_buffer;                 /* pointer to text buffer */
	void   *txr_backup;                 /* optional backup buffer */
	int     txr_width;                  /* width of area in chars */
	int     txr_height;                 /* height of area in chars */
	int     txr_lineoffset;             /* offset in buffer(s) between rows */
	int     txr_xpos;                   /* upper left corner X coordinate */
	int     txr_ypos;                   /* upper left corner Y coordinate */
	char    txr_chrtype;                /* character type (see above) */
} GrTextRegion;

int  GrCharWidth(int chr,const GrTextOption *opt);
int  GrCharHeight(int chr,const GrTextOption *opt);
void GrCharSize(int chr,const GrTextOption *opt,int *w,int *h);
int  GrStringWidth(void *text,int length,const GrTextOption *opt);
int  GrStringHeight(void *text,int length,const GrTextOption *opt);
void GrStringSize(void *text,int length,const GrTextOption *opt,int *w,int *h);

void GrDrawChar(int chr,int x,int y,const GrTextOption *opt);
void GrDrawString(void *text,int length,int x,int y,const GrTextOption *opt);
void GrTextXY(int x,int y,char *text,GrColor fg,GrColor bg);

void GrDumpChar(int chr,int col,int row,const GrTextRegion *r);
void GrDumpText(int col,int row,int wdt,int hgt,const GrTextRegion *r);
void GrDumpTextRegion(const GrTextRegion *r);

#ifndef GRX_SKIP_INLINES
#define GrFontCharPresent(f,ch) (                                              \
	((unsigned int)(ch) - (f)->h.minchar) < (f)->h.numchars                \
)
#define GrFontCharWidth(f,ch) (                                                \
	GrFontCharPresent(f,ch) ?                                              \
	(int)(f)->chrinfo[(unsigned int)(ch) - (f)->h.minchar].width :         \
	(f)->h.width                                                           \
)
#define GrFontCharHeight(f,ch) (                                               \
	(f)->h.height                                                          \
)
#define GrFontCharBmpRowSize(f,ch) (                                           \
	GrFontCharPresent(f,ch) ?                                              \
	(((f)->chrinfo[(unsigned int)(ch) - (f)->h.minchar].width + 7) >> 3) : \
	0                                                                      \
)
#define GrFontCharBitmapSize(f,ch) (                                           \
	GrFontCharBmpRowSize(f,ch) * (f)->h.height                             \
)
#define GrFontStringWidth(f,t,l,tp) (                                          \
	(f)->h.proportional ?                                                  \
	GrProportionalTextWidth((f),(t),(l),(tp)) :                            \
	(f)->h.width * (l)                                                     \
)
#define GrFontStringHeight(f,t,l,tp) (                                         \
	(f)->h.height                                                          \
)
#define GrFontCharBitmap(f,ch) (                                               \
	GrFontCharPresent(f,ch) ?                                              \
	&(f)->bitmap[(f)->chrinfo[(unsigned int)(ch) - (f)->h.minchar].offset]:\
	(char far *)0                                                          \
)
#define GrFontCharAuxBmp(f,ch,dir,ul) (                                        \
	(((dir) == GR_TEXT_DEFAULT) && !(ul)) ?                                \
	GrFontCharBitmap(f,ch) :                                               \
	GrBuildAuxiliaryBitmap((f),(ch),(dir),(ul))                            \
)
#define GrCharWidth(c,o) (                                                     \
	GR_TEXT_IS_VERTICAL((o)->txo_direct) ?                                 \
	GrFontCharHeight((o)->txo_font,GR_TEXTCHR_CODE(c,(o)->txo_chrtype)) :  \
	GrFontCharWidth( (o)->txo_font,GR_TEXTCHR_CODE(c,(o)->txo_chrtype))    \
)
#define GrCharHeight(c,o) (                                                    \
	GR_TEXT_IS_VERTICAL((o)->txo_direct) ?                                 \
	GrFontCharWidth( (o)->txo_font,GR_TEXTCHR_CODE(c,(o)->txo_chrtype)) :  \
	GrFontCharHeight((o)->txo_font,GR_TEXTCHR_CODE(c,(o)->txo_chrtype))    \
)
#define GrCharSize(c,o,wp,hp) do {                                             \
	*(wp) = GrCharHeight(c,o);                                             \
	*(hp) = GrCharWidth( c,o);                                             \
} while(0)
#define GrStringWidth(t,l,o) (                                                 \
	GR_TEXT_IS_VERTICAL((o)->txo_direct) ?                                 \
	GrFontStringHeight((o)->txo_font,(t),(l),(o)->txo_chrtype) :           \
	GrFontStringWidth( (o)->txo_font,(t),(l),(o)->txo_chrtype)             \
)
#define GrStringHeight(t,l,o) (                                                \
	GR_TEXT_IS_VERTICAL((o)->txo_direct) ?                                 \
	GrFontStringWidth( (o)->txo_font,(t),(l),(o)->txo_chrtype) :           \
	GrFontStringHeight((o)->txo_font,(t),(l),(o)->txo_chrtype)             \
)
#define GrStringSize(t,l,o,wp,hp) do {                                         \
	*(wp) = GrStringWidth( t,l,o);                                         \
	*(hp) = GrStringHeight(t,l,o);                                         \
} while(0)
#endif /* GRX_SKIP_INLINES */

/* ================================================================== */
/*            THICK AND DASHED LINE DRAWING PRIMITIVES                */
/* ================================================================== */

/*
 * custom line option structure
 *   zero or one dash pattern length means the line is continuous
 *   the dash pattern always begins with a drawn section
 */
typedef struct {
	GrColor lno_color;                  /* color used to draw line */
	int     lno_width;                  /* width of the line */
	int     lno_pattlen;                /* length of the dash pattern */
	unsigned char *lno_dashpat;         /* draw/nodraw pattern */
} GrLineOption;

void GrCustomLine(int x1,int y1,int x2,int y2,const GrLineOption *o);
void GrCustomBox(int x1,int y1,int x2,int y2,const GrLineOption *o);
void GrCustomCircle(int xc,int yc,int r,const GrLineOption *o);
void GrCustomEllipse(int xc,int yc,int xa,int ya,const GrLineOption *o);
void GrCustomCircleArc(int xc,int yc,int r,int start,int end,int style,const GrLineOption *o);
void GrCustomEllipseArc(int xc,int yc,int xa,int ya,int start,int end,int style,const GrLineOption *o);
void GrCustomPolyLine(int numpts,int points[][2],const GrLineOption *o);
void GrCustomPolygon(int numpts,int points[][2],const GrLineOption *o);

/* ================================================================== */
/*             PATTERNED DRAWING AND FILLING PRIMITIVES               */
/* ================================================================== */

/*
 * BITMAP: a mode independent way to specify a fill pattern of two
 *   colors. It is always 8 pixels wide (1 byte per scan line), its
 *   height is user-defined. SET THE TYPE FLAG TO ZERO!!!
 */
typedef struct _GR_bitmap {
	int     bmp_ispixmap;               /* type flag for pattern union */
	int     bmp_height;                 /* bitmap height */
	char   *bmp_data;                   /* pointer to the bit pattern */
	GrColor bmp_fgcolor;                /* foreground color for fill */
	GrColor bmp_bgcolor;                /* background color for fill */
	int     bmp_memflags;               /* set if dynamically allocated */
} GrBitmap;

/*
 * PIXMAP: a fill pattern stored in a layout identical to the video RAM
 *   for filling using 'bitblt'-s. It is mode dependent, typically one
 *   of the library functions is used to build it. KEEP THE TYPE FLAG
 *   NONZERO!!!
 */
typedef struct _GR_pixmap {
	int     pxp_ispixmap;               /* type flag for pattern union */
	int     pxp_width;                  /* pixmap width (in pixels)  */
	int     pxp_height;                 /* pixmap height (in pixels) */
	GrColor pxp_oper;                   /* bitblt mode (SET, OR, XOR, AND, IMAGE) */
	struct _GR_frame pxp_source;        /* source context for fill */
} GrPixmap;

/*
 * Fill pattern union -- can either be a bitmap or a pixmap
 */
typedef union _GR_pattern {
	int      gp_ispixmap;               /* nonzero for pixmaps */
	GrBitmap gp_bitmap;                 /* fill bitmap */
	GrPixmap gp_pixmap;                 /* fill pixmap */
} GrPattern;

#define gp_bmp_data                     gp_bitmap.bmp_data
#define gp_bmp_height                   gp_bitmap.bmp_height
#define gp_bmp_fgcolor                  gp_bitmap.bmp_fgcolor
#define gp_bmp_bgcolor                  gp_bitmap.bmp_bgcolor

#define gp_pxp_width                    gp_pixmap.pxp_width
#define gp_pxp_height                   gp_pixmap.pxp_height
#define gp_pxp_oper                     gp_pixmap.pxp_oper
#define gp_pxp_source                   gp_pixmap.pxp_source

/*
 * Draw pattern for line drawings -- specifies both the:
 *   (1) fill pattern, and the
 *   (2) custom line drawing option
 */
typedef struct {
	GrPattern     *lnp_pattern;         /* fill pattern */
	GrLineOption  *lnp_option;          /* width + dash pattern */
} GrLinePattern;

GrPattern *GrBuildPixmap(const char *pixels,int w,int h,const GrColorTableP colors);
GrPattern *GrBuildPixmapFromBits(const char *bits,int w,int h,GrColor fgc,GrColor bgc);
GrPattern *GrConvertToPixmap(GrContext *src);

void GrDestroyPattern(GrPattern *p);

void GrPatternedLine(int x1,int y1,int x2,int y2,GrLinePattern *lp);
void GrPatternedBox(int x1,int y1,int x2,int y2,GrLinePattern *lp);
void GrPatternedCircle(int xc,int yc,int r,GrLinePattern *lp);
void GrPatternedEllipse(int xc,int yc,int xa,int ya,GrLinePattern *lp);
void GrPatternedCircleArc(int xc,int yc,int r,int start,int end,int style,GrLinePattern *lp);
void GrPatternedEllipseArc(int xc,int yc,int xa,int ya,int start,int end,int style,GrLinePattern *lp);
void GrPatternedPolyLine(int numpts,int points[][2],GrLinePattern *lp);
void GrPatternedPolygon(int numpts,int points[][2],GrLinePattern *lp);

void GrPatternFilledPlot(int x,int y,GrPattern *p);
void GrPatternFilledLine(int x1,int y1,int x2,int y2,GrPattern *p);
void GrPatternFilledBox(int x1,int y1,int x2,int y2,GrPattern *p);
void GrPatternFilledCircle(int xc,int yc,int r,GrPattern *p);
void GrPatternFilledEllipse(int xc,int yc,int xa,int ya,GrPattern *p);
void GrPatternFilledCircleArc(int xc,int yc,int r,int start,int end,int style,GrPattern *p);
void GrPatternFilledEllipseArc(int xc,int yc,int xa,int ya,int start,int end,int style,GrPattern *p);
void GrPatternFilledConvexPolygon(int numpts,int points[][2],GrPattern *p);
void GrPatternFilledPolygon(int numpts,int points[][2],GrPattern *p);
void GrPatternFloodFill(int x, int y, GrColor border, GrPattern *p);

void GrPatternDrawChar(int chr,int x,int y,const GrTextOption *opt,GrPattern *p);
void GrPatternDrawString(void *text,int length,int x,int y,const GrTextOption *opt,GrPattern *p);
void GrPatternDrawStringExt(void *text,int length,int x,int y,const GrTextOption *opt,GrPattern *p);

/* ================================================================== */
/*                      IMAGE MANIPULATION                            */
/* ================================================================== */

/*
 *  by Michal Stencl Copyright (c) 1998 for GRX
 *  <e-mail>    - [stenclpmd@ba.telecom.sk]
 */

#ifndef GrImage
#define GrImage GrPixmap
#endif

/* Flags for GrImageInverse() */

#define GR_IMAGE_INVERSE_LR  0x01  /* inverse left right */
#define GR_IMAGE_INVERSE_TD  0x02  /* inverse top down */

GrImage *GrImageBuild(const char *pixels,int w,int h,const GrColorTableP colors);
void     GrImageDestroy(GrImage *i);
void     GrImageDisplay(int x,int y, GrImage *i);
void     GrImageDisplayExt(int x1,int y1,int x2,int y2, GrImage *i);
void     GrImageFilledBoxAlign(int xo,int yo,int x1,int y1,int x2,int y2,GrImage *p);
void     GrImageHLineAlign(int xo,int yo,int x,int y,int width,GrImage *p);
void     GrImagePlotAlign(int xo,int yo,int x,int y,GrImage *p);

GrImage *GrImageInverse(GrImage *p,int flag);
GrImage *GrImageStretch(GrImage *p,int nwidth,int nheight);

GrImage *GrImageFromPattern(GrPattern *p);
GrImage *GrImageFromContext(GrContext *c);
GrImage *GrImageBuildUsedAsPattern(const char *pixels,int w,int h,const GrColorTableP colors);

GrPattern *GrPatternFromImage(GrImage *p);


#ifndef GRX_SKIP_INLINES
#define GrImageFromPattern(p) \
	(((p) && (p)->gp_ispixmap) ? (&(p)->gp_pixmap) : NULL)
#define GrImageFromContext(c) \
	(GrImage *)GrConvertToPixmap(c)
#define GrPatternFromImage(p) \
	(GrPattern *)(p)
#define GrImageBuildUsedAsPattern(pixels,w,h,colors) \
	(GrImage *)GrBuildPixmap(pixels,w,h,colors);
#define GrImageDestroy(i)   \
	  GrDestroyPattern((GrPattern *)(i));
#endif

/* ================================================================== */
/*               DRAWING IN USER WINDOW COORDINATES                   */
/* ================================================================== */

void GrSetUserWindow(int x1,int y1,int x2,int y2);
void GrGetUserWindow(int *x1,int *y1,int *x2,int *y2);
void GrGetScreenCoord(int *x,int *y);
void GrGetUserCoord(int *x,int *y);

void GrUsrPlot(int x,int y,GrColor c);
void GrUsrLine(int x1,int y1,int x2,int y2,GrColor c);
void GrUsrHLine(int x1,int x2,int y,GrColor c);
void GrUsrVLine(int x,int y1,int y2,GrColor c);
void GrUsrBox(int x1,int y1,int x2,int y2,GrColor c);
void GrUsrFilledBox(int x1,int y1,int x2,int y2,GrColor c);
void GrUsrFramedBox(int x1,int y1,int x2,int y2,int wdt,GrFBoxColors *c);
void GrUsrCircle(int xc,int yc,int r,GrColor c);
void GrUsrEllipse(int xc,int yc,int xa,int ya,GrColor c);
void GrUsrCircleArc(int xc,int yc,int r,int start,int end,int style,GrColor c);
void GrUsrEllipseArc(int xc,int yc,int xa,int ya,int start,int end,int style,GrColor c);
void GrUsrFilledCircle(int xc,int yc,int r,GrColor c);
void GrUsrFilledEllipse(int xc,int yc,int xa,int ya,GrColor c);
void GrUsrFilledCircleArc(int xc,int yc,int r,int start,int end,int style,GrColor c);
void GrUsrFilledEllipseArc(int xc,int yc,int xa,int ya,int start,int end,int style,GrColor c);
void GrUsrPolyLine(int numpts,int points[][2],GrColor c);
void GrUsrPolygon(int numpts,int points[][2],GrColor c);
void GrUsrFilledConvexPolygon(int numpts,int points[][2],GrColor c);
void GrUsrFilledPolygon(int numpts,int points[][2],GrColor c);
void GrUsrFloodFill(int x, int y, GrColor border, GrColor c);

GrColor GrUsrPixel(int x,int y);
GrColor GrUsrPixelC(GrContext *c,int x,int y);

void GrUsrCustomLine(int x1,int y1,int x2,int y2,const GrLineOption *o);
void GrUsrCustomBox(int x1,int y1,int x2,int y2,const GrLineOption *o);
void GrUsrCustomCircle(int xc,int yc,int r,const GrLineOption *o);
void GrUsrCustomEllipse(int xc,int yc,int xa,int ya,const GrLineOption *o);
void GrUsrCustomCircleArc(int xc,int yc,int r,int start,int end,int style,const GrLineOption *o);
void GrUsrCustomEllipseArc(int xc,int yc,int xa,int ya,int start,int end,int style,const GrLineOption *o);
void GrUsrCustomPolyLine(int numpts,int points[][2],const GrLineOption *o);
void GrUsrCustomPolygon(int numpts,int points[][2],const GrLineOption *o);

void GrUsrPatternedLine(int x1,int y1,int x2,int y2,GrLinePattern *lp);
void GrUsrPatternedBox(int x1,int y1,int x2,int y2,GrLinePattern *lp);
void GrUsrPatternedCircle(int xc,int yc,int r,GrLinePattern *lp);
void GrUsrPatternedEllipse(int xc,int yc,int xa,int ya,GrLinePattern *lp);
void GrUsrPatternedCircleArc(int xc,int yc,int r,int start,int end,int style,GrLinePattern *lp);
void GrUsrPatternedEllipseArc(int xc,int yc,int xa,int ya,int start,int end,int style,GrLinePattern *lp);
void GrUsrPatternedPolyLine(int numpts,int points[][2],GrLinePattern *lp);
void GrUsrPatternedPolygon(int numpts,int points[][2],GrLinePattern *lp);

void GrUsrPatternFilledPlot(int x,int y,GrPattern *p);
void GrUsrPatternFilledLine(int x1,int y1,int x2,int y2,GrPattern *p);
void GrUsrPatternFilledBox(int x1,int y1,int x2,int y2,GrPattern *p);
void GrUsrPatternFilledCircle(int xc,int yc,int r,GrPattern *p);
void GrUsrPatternFilledEllipse(int xc,int yc,int xa,int ya,GrPattern *p);
void GrUsrPatternFilledCircleArc(int xc,int yc,int r,int start,int end,int style,GrPattern *p);
void GrUsrPatternFilledEllipseArc(int xc,int yc,int xa,int ya,int start,int end,int style,GrPattern *p);
void GrUsrPatternFilledConvexPolygon(int numpts,int points[][2],GrPattern *p);
void GrUsrPatternFilledPolygon(int numpts,int points[][2],GrPattern *p);
void GrUsrPatternFloodFill(int x, int y, GrColor border, GrPattern *p);

void GrUsrDrawChar(int chr,int x,int y,const GrTextOption *opt);
void GrUsrDrawString(char *text,int length,int x,int y,const GrTextOption *opt);
void GrUsrTextXY(int x,int y,char *text,GrColor fg,GrColor bg);

/* ================================================================== */
/*                    GRAPHICS CURSOR UTILITIES                       */
/* ================================================================== */

typedef struct _GR_cursor {
	struct _GR_context work;                    /* work areas (4) */
	int     xcord,ycord;                        /* cursor position on screen */
	int     xsize,ysize;                        /* cursor size */
	int     xoffs,yoffs;                        /* LU corner to hot point offset */
	int     xwork,ywork;                        /* save/work area sizes */
	int     xwpos,ywpos;                        /* save/work area position on screen */
	int     displayed;                          /* set if displayed */
} GrCursor;

GrCursor *GrBuildCursor(char far *pixels,int pitch,int w,int h,int xo,int yo,const GrColorTableP c);
void GrDestroyCursor(GrCursor *cursor);
void GrDisplayCursor(GrCursor *cursor);
void GrEraseCursor(GrCursor *cursor);
void GrMoveCursor(GrCursor *cursor,int x,int y);

/* ================================================================== */
/*               MOUSE AND KEYBOARD INPUT UTILITIES                   */
/* ================================================================== */

#define GR_M_MOTION         0x001               /* mouse event flag bits */
#define GR_M_LEFT_DOWN      0x002
#define GR_M_LEFT_UP        0x004
#define GR_M_RIGHT_DOWN     0x008
#define GR_M_RIGHT_UP       0x010
#define GR_M_MIDDLE_DOWN    0x020
#define GR_M_MIDDLE_UP      0x040
#define GR_M_P4_DOWN        0x400
#define GR_M_P4_UP          0x800
#define GR_M_P5_DOWN        0x2000
#define GR_M_P5_UP          0x4000
#define GR_M_BUTTON_DOWN    (GR_M_LEFT_DOWN | GR_M_MIDDLE_DOWN | GR_M_RIGHT_DOWN | GR_M_P4_DOWN | GR_M_P5_DOWN)
#define GR_M_BUTTON_UP      (GR_M_LEFT_UP   | GR_M_MIDDLE_UP   | GR_M_RIGHT_UP   | GR_M_P4_UP   | GR_M_P5_UP)
#define GR_M_BUTTON_CHANGE  (GR_M_BUTTON_UP | GR_M_BUTTON_DOWN )

#define GR_M_LEFT           0x01                /* mouse button index bits */
#define GR_M_RIGHT          0x02
#define GR_M_MIDDLE         0x04
#define GR_M_P4             0x08
#define GR_M_P5             0x10

#define GR_M_KEYPRESS       0x080               /* other event flag bits */
#define GR_M_POLL           0x100
#define GR_M_NOPAINT        0x200
#define GR_COMMAND          0x1000
#define GR_M_EVENT          (GR_M_MOTION | GR_M_KEYPRESS | GR_M_BUTTON_CHANGE | GR_COMMAND)

#define GR_KB_RIGHTSHIFT    0x01                /* Keybd states: right shift key depressed */
#define GR_KB_LEFTSHIFT     0x02                /* left shift key depressed */
#define GR_KB_CTRL          0x04                /* CTRL depressed */
#define GR_KB_ALT           0x08                /* ALT depressed */
#define GR_KB_SCROLLOCK     0x10                /* SCROLL LOCK active */
#define GR_KB_NUMLOCK       0x20                /* NUM LOCK active */
#define GR_KB_CAPSLOCK      0x40                /* CAPS LOCK active */
#define GR_KB_INSERT        0x80                /* INSERT state active */
#define GR_KB_SHIFT         (GR_KB_LEFTSHIFT | GR_KB_RIGHTSHIFT)

#define GR_M_CUR_NORMAL     0                   /* MOUSE CURSOR modes: just the cursor */
#define GR_M_CUR_RUBBER     1                   /* rectangular rubber band (XOR-d to the screen) */
#define GR_M_CUR_LINE       2                   /* line attached to the cursor */
#define GR_M_CUR_BOX        3                   /* rectangular box dragged by the cursor */

#define GR_M_QUEUE_SIZE     128                 /* default queue size */

typedef struct _GR_mouseEvent {                 /* mouse event buffer structure */
	int  flags;                                 /* event type flags (see above) */
	int  x,y;                                   /* mouse coordinates */
	int  buttons;                               /* mouse button state */
	int  key;                                   /* key code from keyboard */
	int  kbstat;                                /* keybd status (ALT, CTRL, etc..) */
	long dtime;                                 /* time since last event (msec) */
} GrMouseEvent;

/*
 * mouse status information
 */
extern const struct _GR_mouseInfo {
	int   (*block)(GrContext*,int,int,int,int); /* mouse block function */
	void  (*unblock)(int flags);                /* mouse unblock function */
	void  (*uninit)(void);                      /* mouse cleanupt function */
	struct _GR_cursor     *cursor;              /* the mouse cursor */
	struct _GR_mouseEvent *queue;               /* queue of pending input events */
	int     msstatus;                           /* -1:missing, 0:unknown, 1:present, 2:initted */
	int     displayed;                          /* cursor is (generally) drawn */
	int     blockflag;                          /* cursor temp. erase/block flag */
	int     docheck;                            /* need to check before gr. op. to screen */
	int     cursmode;                           /* mouse cursor draw mode */
	int     x1,y1,x2,y2;                        /* auxiliary params for some cursor draw modes */
	GrColor curscolor;                          /* color for some cursor draw modes */
	int     owncursor;                          /* auto generated cursor */
	int     xpos,ypos;                          /* current mouse position */
	int     xmin,xmax;                          /* mouse movement X coordinate limits */
	int     ymin,ymax;                          /* mouse movement Y coordinate limits */
	int     spmult,spdiv;                       /* mouse cursor speed factors */
	int     thresh,accel;                       /* mouse acceleration parameters */
	int     moved;                              /* mouse cursor movement flag */
	int     qsize;                              /* max size of the queue */
	int     qlength;                            /* current # of items in the queue */
	int     qread;                              /* read pointer for the queue */
	int     qwrite;                             /* write pointer for the queue */
} * const GrMouseInfo;

int  GrMouseDetect(void);
void GrMouseEventMode(int dummy);
void GrMouseInit(void);
void GrMouseInitN(int queue_size);
void GrMouseUnInit(void);
void GrMouseSetSpeed(int spmult,int spdiv);
void GrMouseSetAccel(int thresh,int accel);
void GrMouseSetLimits(int x1,int y1,int x2,int y2);
void GrMouseGetLimits(int *x1,int *y1,int *x2,int *y2);
void GrMouseWarp(int x,int y);
void GrMouseEventEnable(int enable_kb,int enable_ms);
void GrMouseGetEvent(int flags,GrMouseEvent *event);

void GrMouseGetEventT(int flags,GrMouseEvent *event,long timout_msecs);
/* Note:
**       event->dtime is only valid if any event occured (event->flags !=0)
**       otherwise it's set as -1.
**       Additionally event timing is now real world time. (X11 && Linux
**       used clock(), user process time, up to 2.28f)
*/

int  GrMousePendingEvent(void);

GrCursor *GrMouseGetCursor(void);
void GrMouseSetCursor(GrCursor *cursor);
void GrMouseSetColors(GrColor fg,GrColor bg);
void GrMouseSetCursorMode(int mode,...);
void GrMouseDisplayCursor(void);
void GrMouseEraseCursor(void);
void GrMouseUpdateCursor(void);
int  GrMouseCursorIsDisplayed(void);

int  GrMouseBlock(GrContext *c,int x1,int y1,int x2,int y2);
void GrMouseUnBlock(int return_value_from_GrMouseBlock);

#if 0
/* !! old style (before grx v2.26) keyboard interface    !!
   !! old functions still linkable but for compatibility !!
   !! across platforms and with future versions of GRX   !!
   !! one use functions from grkeys.h                    !! */
#ifndef __MSDOS__
int  kbhit(void);
int  getch(void);
#endif
#ifndef __DJGPP__
int  getkey(void);
int  getxkey(void);
#endif
int  getkbstat(void);
#endif
/* Why this ???
#ifdef __WATCOMC__
int  getxkey(void);
#endif
*/

#ifndef GRX_SKIP_INLINES
#define GrMouseEventMode(x)         /* nothing! */
#define GrMouseGetCursor()          (GrMouseInfo->cursor)
#define GrMouseCursorIsDisplayed()  (GrMouseInfo->displayed)
#define GrMouseInit()               GrMouseInitN(GR_M_QUEUE_SIZE);
#define GrMouseGetEvent(f,ev)       GrMouseGetEventT((f),(ev),(-1L));
#define GrMousePendingEvent() (                                                \
	GrMouseUpdateCursor(),                                                 \
   (GrMouseInfo->qlength > 0)                                                  \
)
#define GrMouseUnInit() do {                                                   \
	if(GrMouseInfo->uninit) {                                              \
	(*GrMouseInfo->uninit)();                                              \
	}                                                                      \
} while(0)
#define GrMouseGetLimits(x1p,y1p,x2p,y2p) do {                                 \
	*(x1p) = GrMouseInfo->xmin; *(y1p) = GrMouseInfo->ymin;                \
	*(x2p) = GrMouseInfo->xmax; *(y2p) = GrMouseInfo->ymax;                \
} while(0)
#define GrMouseBlock(c,x1,y1,x2,y2) (                                          \
	(((c) ? (const GrContext*)(c) : GrCurrentContext())->gc_onscreen &&    \
	 (GrMouseInfo->docheck)) ?                                             \
	(*GrMouseInfo->block)((c),(x1),(y1),(x2),(y2)) :                       \
	0                                                                      \
)
#define GrMouseUnBlock(f) do {                                                 \
	if((f) && GrMouseInfo->displayed) {                                    \
	(*GrMouseInfo->unblock)((f));                                          \
	}                                                                      \
} while(0)
#endif  /* GRX_SKIP_INLINES */

/* ================================================================== */
/*                           PNM FUNCTIONS                            */
/* ================================================================== */

/*
 *  The PNM formats, grx support load/save of
 *  binaries formats (4,5,6) only
 */

#define PLAINPBMFORMAT 1
#define PLAINPGMFORMAT 2
#define PLAINPPMFORMAT 3
#define PBMFORMAT      4
#define PGMFORMAT      5
#define PPMFORMAT      6

/* The PNM functions */

int GrSaveContextToPbm( GrContext *grc, char *pbmfn, char *docn );
int GrSaveContextToPgm( GrContext *grc, char *pgmfn, char *docn );
int GrSaveContextToPpm( GrContext *grc, char *ppmfn, char *docn );
int GrLoadContextFromPnm( GrContext *grc, char *pnmfn );
int GrQueryPnm( char *pnmfn, int *width, int *height, int *maxval );
int GrLoadContextFromPnmBuffer( GrContext *grc, const char *buffer );
int GrQueryPnmBuffer( const char *buffer, int *width, int *height, int *maxval );

/* ================================================================== */
/*                           PNG FUNCTIONS                            */
/*  these functions may not be installed or available on all system   */
/* ================================================================== */

int GrPngSupport( void );
int GrSaveContextToPng( GrContext *grc, char *pngfn );
int GrLoadContextFromPng( GrContext *grc, char *pngfn, int use_alpha );
int GrQueryPng( char *pngfn, int *width, int *height );

/* ================================================================== */
/*                          JPEG FUNCTIONS                            */
/*  these functions may not be installed or available on all system   */
/* ================================================================== */

int GrJpegSupport( void );
int GrLoadContextFromJpeg( GrContext *grc, char *jpegfn, int scale );
int GrQueryJpeg( char *jpegfn, int *width, int *height );
int GrSaveContextToJpeg( GrContext *grc, char *jpegfn, int quality );
int GrSaveContextToGrayJpeg( GrContext *grc, char *jpegfn, int quality );

/* ================================================================== */
/*               MISCELLANEOUS UTILITIY FUNCTIONS                     */
/* ================================================================== */

void GrResizeGrayMap(unsigned char far *map,int pitch,int ow,int oh,int nw,int nh);
int  GrMatchString(const char *pattern,const char *strg);
void GrSetWindowTitle(char *title);
void GrSleep(int msec);
void GrFlush(void);
long GrMsecTime(void);	

/* ================================================================== */
/*                        TIFF ADDON FUNCTIONS                        */
/*  these functions may not be installed or available on all system   */
/* ================================================================== */

/*
** SaveContextToTiff - Dump a context in a TIFF file
**
** Arguments:
**   cxt:   Context to be saved (NULL -> use current context)
**   tiffn: Name of tiff file
**   compr: Compression method (see tiff.h), 0: automatic selection
**   docn:  string saved in the tiff file (DOCUMENTNAME tag)
**
**  Returns  0 on success
**          -1 on error
**
** requires tifflib by  Sam Leffler (sam@engr.sgi.com)
**        available at  ftp://ftp.sgi.com/graphics/tiff
*/
int SaveContextToTiff(GrContext *cxt, char *tiffn, unsigned compr, char *docn);

#ifdef __cplusplus
}
#endif
#endif  /* whole file */
