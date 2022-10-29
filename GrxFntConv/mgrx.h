/**
 ** mgrx.h ---- MGRX API functions and data structure declarations
 **
 ** Copyright (c) 1995 Csaba Biegl, 820 Stirrup Dr, Nashville, TN 37221
 ** [e-mail: csaba@vuse.vanderbilt.edu]
 **
 ** Copyright (C) 2006-2020 Mariano Alvarez Fernandez
 ** [e-mail: malfer@telefonica.net]
 ** Heavily modified for MGRX
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

#ifndef __MGRX_H_INCLUDED__
#define __MGRX_H_INCLUDED__

/* ================================================================== */
/*       COMPILER -- CPU -- SYSTEM SPECIFIC VERSION STUFF             */
/* ================================================================== */

/* Version of MGRX API */

#define MGRX_VERSION_API 0x0134

/* these are the supported configurations: */
#define MGRX_VERSION_GCC_386_DJGPP 1    /* DJGPP v2 */
#define MGRX_VERSION_GCC_386_LINUX 2    /* console framebuffer i386 */
#define MGRX_VERSION_GCC_386_X11 3      /* X11 version i386 */
#define MGRX_VERSION_GCC_386_WIN32 4    /* WIN32 using Mingw32 */
#define MGRX_VERSION_GCC_X86_64_LINUX 5 /* console framebuffer x86_64 */
#define MGRX_VERSION_GCC_X86_64_X11 6   /* X11 version x86_64 */
#define MGRX_VERSION_GCC_ARM_LINUX 7    /* console framebuffer arm */
#define MGRX_VERSION_GCC_ARM_X11 8      /* X11 version arm */

#ifdef __GNUC__
#ifdef __DJGPP__
#define MGRX_VERSION MGRX_VERSION_GCC_386_DJGPP
#endif
#if defined(__XWIN__)
#if defined(__linux__) && defined(__i386__)
#define MGRX_VERSION MGRX_VERSION_GCC_386_X11
#endif
#if defined(__linux__) && defined(__x86_64__)
#define MGRX_VERSION MGRX_VERSION_GCC_X86_64_X11
#endif
#if defined(__linux__) && defined(__arm__)
#define MGRX_VERSION MGRX_VERSION_GCC_ARM_X11
#endif
#else
#if defined(__linux__) && defined(__i386__)
#define MGRX_VERSION MGRX_VERSION_GCC_386_LINUX
#endif
#if defined(__linux__) && defined(__x86_64__)
#define MGRX_VERSION MGRX_VERSION_GCC_X86_64_LINUX
#endif
#if defined(__linux__) && defined(__arm__)
#define MGRX_VERSION MGRX_VERSION_GCC_ARM_LINUX
#endif
#endif
#ifdef __WIN32__
#define MGRX_VERSION MGRX_VERSION_GCC_386_WIN32
#endif
#endif /* __GNUC__ */

#ifndef MGRX_VERSION
#error MGRX is not supported on your COMPILER/CPU/OPERATING SYSTEM!
#endif

#ifdef __cplusplus
extern "C" {
#endif

#include <time.h>

/* a couple of forward declarations ... */
typedef struct _GR_frameDriver GrFrameDriver;
typedef struct _GR_videoDriver GrVideoDriver;
typedef struct _GR_videoMode GrVideoMode;
typedef struct _GR_videoModeExt GrVideoModeExt;
typedef struct _GR_frame GrFrame;
typedef struct _GR_context GrContext;

/* ================================================================== */
/*                        SYSTEM TYPE DEF's                           */
/* ================================================================== */

/* all supported platforms have 32 bit ints */
typedef unsigned int GrColor;

/* ================================================================== */
/*                           MODE SETTING                             */
/* ================================================================== */

/*
 * available video modes (for 'GrSetMode')
 */
typedef enum _GR_graphicsModes {
    GR_unknown_mode = (-1), /* initial state */
    /* ============= modes which clear the video memory ============= */
    GR_default_text = 0, /* Extra parameters for GrSetMode: */
    GR_320_200_graphics,
    GR_default_graphics,
    GR_width_height_graphics, /* int w,int h */
    GR_biggest_graphics,
    GR_width_height_color_graphics, /* int w,int h,GrColor nc */
    GR_custom_graphics,             /* int w,int h,GrColor nc,int vx,int vy */
    /* ==== equivalent modes which do not clear the video memory ==== */
    GR_NC_default_text,
    GR_NC_320_200_graphics,
    GR_NC_default_graphics,
    GR_NC_width_height_graphics, /* int w,int h */
    GR_NC_biggest_graphics,
    GR_NC_width_height_color_graphics, /* int w,int h,GrColor nc */
    GR_NC_custom_graphics,             /* int w,int h,GrColor nc,int vx,int vy */
    /* ==== bpp instead number of color based modes ==== */
    /* colors = 1 << bpp */
    GR_width_height_bpp_graphics,    /* int w,int h,int bpp */
    GR_custom_bpp_graphics,          /* int w,int h,int bpp,int vx,int vy */
    GR_NC_width_height_bpp_graphics, /* int w,int h,int bpp */
    GR_NC_custom_bpp_graphics        /* int w,int h,int bpp,int vx,int vy */
} GrGraphicsMode;

/*
 * Available frame modes (video memory layouts)
 */
typedef enum _GR_frameModes {
    /* ==== MSDOS video frame buffer modes ==== */
    GR_frameUndef,   /* undefined */
    GR_frameText,    /* text modes */
    GR_frameHERC1,   /* Hercules mono (deleted) */
    GR_frameEGAVGA1, /* EGA VGA mono */
    GR_frameEGA4,    /* EGA 16 color */
    GR_frameSVGA4,   /* (Super) VGA 16 color */
    GR_frameSVGA8,   /* (Super) VGA 256 color */
    GR_frameVGA8X,   /* VGA 256 color mode X */
    GR_frameSVGA16,  /* Super VGA 32768/65536 color */
    GR_frameSVGA24,  /* Super VGA 16M color */
    GR_frameSVGA32L, /* Super VGA 16M color padded #1 */
    GR_frameSVGA32H, /* Super VGA 16M color padded #2 */
    /* ==== linear frame buffer modes  ==== */
    GR_frameSVGA8_LFB,   /* (Super) VGA 256 color */
    GR_frameSVGA16_LFB,  /* Super VGA 32768/65536 color */
    GR_frameSVGA24_LFB,  /* Super VGA 16M color */
    GR_frameSVGA32L_LFB, /* Super VGA 16M color padded #1 */
    GR_frameSVGA32H_LFB, /* Super VGA 16M color padded #2 */
    /* ==== modes provided by the X11 driver ==== */
    GR_frameXWIN1,   /* 1bpp B&W */
    GR_frameXWIN4,   /* 4bpp paletted mode */
    GR_frameXWIN8,   /* 8bpp paletted mode */
    GR_frameXWIN16,  /* 16bpp */
    GR_frameXWIN24,  /* 24bpp */
    GR_frameXWIN32L, /* 32bpp (24bpp padded low) */
    GR_frameXWIN32H, /* 32bpp (24bpp padded high) */
    /* ==== modes provided by the WIN32 driver ==== */
    GR_frameWIN32_1,   /* 1bpp (to do) */
    GR_frameWIN32_4,   /* 4bpp (to do) */
    GR_frameWIN32_8,   /* 8bpp paletted mode */
    GR_frameWIN32_16,  /* 16bpp (to do) */
    GR_frameWIN32_24,  /* 24bpp */
    GR_frameWIN32_32L, /* 32bpp low (to do) */
    GR_frameWIN32_32H, /* 32bpp high (to do) */
    /* ==== modes provided by the linux fb driver ==== */
    GR_frameLNXFB_1,   /* 1bpp (to do) */
    GR_frameLNXFB_4,   /* 4bpp (to do) */
    GR_frameLNXFB_8,   /* 8bpp (to do) */
    GR_frameLNXFB_16,  /* 16bpp */
    GR_frameLNXFB_24,  /* 24bpp (to do) */
    GR_frameLNXFB_32L, /* 32bpp (24bpp padded low) */
    GR_frameLNXFB_32H, /* 32bpp (24bpp padded high) */
    /* ==== system RAM frame buffer modes ==== */
    GR_frameRAM1,   /* mono */
    GR_frameRAM4,   /* 16 color planar */
    GR_frameRAM8,   /* 256 color */
    GR_frameRAM16,  /* 32768/65536 color */
    GR_frameRAM24,  /* 16M color */
    GR_frameRAM32L, /* 16M color padded #1 */
    GR_frameRAM32H, /* 16M color padded #2 */
    /* ==== markers for scanning modes ==== */
    GR_firstTextFrameMode = GR_frameText,
    GR_lastTextFrameMode = GR_frameText,
    GR_firstGraphicsFrameMode = GR_frameHERC1,
    GR_lastGraphicsFrameMode = GR_frameLNXFB_32H,
    GR_firstRAMframeMode = GR_frameRAM1,
    GR_lastRAMframeMode = GR_frameRAM32H
} GrFrameMode;

/*
 * supported video adapter types
 */
typedef enum _GR_videoAdapters {
    GR_UNKNOWN = (-1), /* not known (before driver set) */
    GR_VGA,            /* VGA adapter */
    GR_EGA,            /* EGA adapter */
    GR_XWIN,           /* X11 driver */
    GR_WIN32,          /* WIN32 driver */
    GR_LNXFB,          /* Linux framebuffer */
    GR_MEM             /* memory only driver */
} GrVideoAdapter;

/*
 * The video driver descriptor structure
 */
struct _GR_videoDriver {
    char *name;                      /* driver name */
    enum _GR_videoAdapters adapter;  /* adapter type */
    struct _GR_videoDriver *inherit; /* inherit video modes from this */
    struct _GR_videoMode *modes;     /* table of supported modes */
    int nmodes;                      /* number of modes */
    int (*detect)(void);             /* detection routine */
    int (*init)(char *options);      /* initialization routine */
    void (*reset)(void);             /* reset routine */
    GrVideoMode *(*selectmode)(      /* special mode select routine */
                               GrVideoDriver *drv, int w, int h, int bpp, int txt, unsigned int *ep);
    unsigned drvflags;
};
/* bits in the drvflags field: */
#define GR_DRIVERF_USER_RESOLUTION 1
/* set if driver supports user setable arbitrary resolution */

/*
 * Video driver mode descriptor structure
 */
struct _GR_videoMode {
    char present;                     /* is it really available? */
    char bpp;                         /* log2 of # of colors */
    short width, height;              /* video mode geometry */
    short mode;                       /* BIOS mode number (if any) */
    int lineoffset;                   /* scan line length */
    int privdata;                     /* driver can use it for anything */
    struct _GR_videoModeExt *extinfo; /* extra info (maybe shared) */
};

/*
 * Video driver mode descriptor extension structure. This is a separate
 * structure accessed via a pointer from the main mode descriptor. The
 * reason for this is that frequently several modes can share the same
 * extended info.
 */
struct _GR_videoModeExt {
    enum _GR_frameModes mode;    /* frame driver for this video mode */
    struct _GR_frameDriver *drv; /* optional frame driver override */
    char *frame;                 /* frame buffer address */
    char cprec[3];               /* color component precisions */
    char cpos[3];                /* color component bit positions */
    int flags;                   /* mode flag bits; see "grdriver.h" */
    int (*setup)(GrVideoMode *md, int noclear);
    int (*setvsize)(GrVideoMode *md, int w, int h, GrVideoMode *result);
    int (*scroll)(GrVideoMode *md, int x, int y, int result[2]);
    void (*setbank)(int bk);
    void (*setrwbanks)(int rb, int wb);
    void (*loadcolor)(int c, int r, int g, int b);
    int LFB_Selector;
};

/*
 * The frame driver descriptor structure.
 */
struct _GR_frameDriver {
    enum _GR_frameModes mode;  /* supported frame access mode */
    enum _GR_frameModes rmode; /* matching RAM frame (if video) */
    int is_video;              /* video RAM frame driver ? */
    int row_align;             /* scan line size alignment */
    int num_planes;            /* number of planes */
    int bits_per_pixel;        /* bits per pixel */
    long max_plane_size;       /* maximum plane size in bytes */
    int (*init)(GrVideoMode *md);
    GrColor (*readpixel)(GrFrame *c, int x, int y);
    void (*drawpixel)(int x, int y, GrColor c);
    void (*drawline)(int x, int y, int dx, int dy, GrColor c);
    void (*drawhline)(int x, int y, int w, GrColor c);
    void (*drawvline)(int x, int y, int h, GrColor c);
    void (*drawblock)(int x, int y, int w, int h, GrColor c);
    void (*drawbitmap)(int x, int y, int w, int h, char *bmp, int pitch, int start, GrColor fg, GrColor bg);
    void (*drawpattern)(int x, int y, int w, char patt, GrColor fg, GrColor bg);
    void (*bitblt)(GrFrame *dst, int dx, int dy, GrFrame *src, int x, int y, int w, int h, GrColor op);
    void (*bltv2r)(GrFrame *dst, int dx, int dy, GrFrame *src, int x, int y, int w, int h, GrColor op);
    void (*bltr2v)(GrFrame *dst, int dx, int dy, GrFrame *src, int x, int y, int w, int h, GrColor op);
    /* new functions in v2.3 */
    GrColor *(*getindexedscanline)(GrFrame *c, int x, int y, int w, int *indx);
    /* will return an array of pixel values pv[] read from frame   */
    /*    if indx == NULL: pv[i=0..w-1] = readpixel(x+i,y)         */
    /*    else             pv[i=0..w-1] = readpixel(x+indx[i],y)   */
    void (*putscanline)(int x, int y, int w, const GrColor *scl, GrColor op);
    /** will draw scl[i=0..w-1] to frame:                          */
    /*    if (scl[i] != skipcolor) drawpixel(x+i,y,(scl[i] | op))  */
};

/*
 * driver and mode info structure
 */
extern const struct _GR_driverInfo {
    struct _GR_videoDriver *vdriver;    /* the current video driver */
    struct _GR_videoMode *curmode;      /* current video mode pointer */
    struct _GR_videoMode actmode;       /* copy of above, resized if virtual */
    struct _GR_frameDriver fdriver;     /* frame driver for the current context */
    struct _GR_frameDriver sdriver;     /* frame driver for the screen */
    struct _GR_frameDriver tdriver;     /* a dummy driver for text modes */
    enum _GR_graphicsModes mcode;       /* code for the current mode */
    int deftw, defth;                   /* default text mode size */
    int defgw, defgh;                   /* default graphics mode size */
    GrColor deftc, defgc;               /* default text and graphics colors */
    int biggw, biggh;                   /* bigest graphics mode size */
    int vposx, vposy;                   /* current virtual viewport position */
    int errsfatal;                      /* if set, exit upon errors */
    int moderestore;                    /* restore startup video mode if set */
    int splitbanks;                     /* indicates separate R/W banks */
    int curbank;                        /* currently mapped bank */
    void (*mdsethook)(void);            /* callback for mode set */
    void (*setbank)(int bk);            /* banking routine */
    void (*setrwbanks)(int rb, int wb); /* split banking routine */
} *const GrDriverInfo;

/*
 * setup stuff
 */
int GrSetDriver(char *drvspec);
int GrSetMode(GrGraphicsMode which, ...);
int GrSetViewport(int xpos, int ypos);
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
GrFrameMode GrCurrentFrameMode(void);
GrFrameMode GrScreenFrameMode(void);
GrFrameMode GrCoreFrameMode(void);
char *GrFrameDriverName(GrFrameMode m);

const GrVideoDriver *GrCurrentVideoDriver(void);
const GrVideoMode *GrCurrentVideoMode(void);
const GrVideoMode *GrVirtualVideoMode(void);
const GrFrameDriver *GrCurrentFrameDriver(void);
const GrFrameDriver *GrScreenFrameDriver(void);
const GrVideoMode *GrFirstVideoMode(GrFrameMode fmode);
const GrVideoMode *GrNextVideoMode(const GrVideoMode *prev);

int GrScreenX(void);
int GrScreenY(void);
int GrVirtualX(void);
int GrVirtualY(void);
int GrViewportX(void);
int GrViewportY(void);

int GrScreenIsVirtual(void);

/*
 * RAM context geometry and memory allocation inquiry stuff
 */
int GrFrameNumPlanes(GrFrameMode md);
int GrFrameLineOffset(GrFrameMode md, int width);
long GrFramePlaneSize(GrFrameMode md, int w, int h);
long GrFrameContextSize(GrFrameMode md, int w, int h);

int GrNumPlanes(void);
int GrLineOffset(int width);
long GrPlaneSize(int w, int h);
long GrContextSize(int w, int h);

/*
 * inline implementation for some of the above
 */
#ifndef GRX_SKIP_INLINES
#define GrAdapterType() (GrDriverInfo->vdriver ? GrDriverInfo->vdriver->adapter : GR_UNKNOWN)
#define GrCurrentMode() (GrDriverInfo->mcode)
#define GrCurrentFrameMode() (GrDriverInfo->fdriver.mode)
#define GrScreenFrameMode() (GrDriverInfo->sdriver.mode)
#define GrCoreFrameMode() (GrDriverInfo->sdriver.rmode)

#define GrCurrentVideoDriver() ((const GrVideoDriver *)(GrDriverInfo->vdriver))
#define GrCurrentVideoMode() ((const GrVideoMode *)(GrDriverInfo->curmode))
#define GrVirtualVideoMode() ((const GrVideoMode *)(&GrDriverInfo->actmode))
#define GrCurrentFrameDriver() ((const GrFrameDriver *)(&GrDriverInfo->fdriver))
#define GrScreenFrameDriver() ((const GrFrameDriver *)(&GrDriverInfo->sdriver))

#define GrIsFixedMode() (!(GrCurrentVideoDriver()->drvflags & GR_DRIVERF_USER_RESOLUTION))

#define GrScreenX() (GrCurrentVideoMode()->width)
#define GrScreenY() (GrCurrentVideoMode()->height)
#define GrVirtualX() (GrVirtualVideoMode()->width)
#define GrVirtualY() (GrVirtualVideoMode()->height)
#define GrViewportX() (GrDriverInfo->vposx)
#define GrViewportY() (GrDriverInfo->vposy)

#define GrScreenIsVirtual() ((GrScreenX() + GrScreenY()) < (GrVirtualX() + GrVirtualY()))

#define GrNumPlanes() GrFrameNumPlanes(GrCoreFrameMode())
#define GrLineOffset(w) GrFrameLineOffset(GrCoreFrameMode(), w)
#define GrPlaneSize(w, h) GrFramePlaneSize(GrCoreFrameMode(), w, h)
#define GrContextSize(w, h) GrFrameContextSize(GrCoreFrameMode(), w, h)
#endif /* GRX_SKIP_INLINES */

/* ================================================================== */
/*              FRAME BUFFER, CONTEXT AND CLIPPING STUFF              */
/* ================================================================== */

#define MGRX_GF_MYCONTEXT 1  // Set if context or pixmap was created by the lib
#define MGRX_GF_MYFRAME 2    // Set if frame memory was created by the lib

struct _GR_frame {
    char *gf_baseaddr[4];              /* base address of frame memory */
    short gf_selector;                 /* frame memory segment selector */
    char gf_onscreen;                  /* is it in video memory ? */
    char gf_memflags;                  /* memory allocation flags */
    int gf_lineoffset;                 /* offset to next scan line in bytes */
    struct _GR_frameDriver *gf_driver; /* frame access functions */
};

struct _GR_context {
    struct _GR_frame gc_frame;   /* frame buffer info */
    struct _GR_context *gc_root; /* context which owns frame */
    int gc_xmax;                 /* max X coord (width  - 1) */
    int gc_ymax;                 /* max Y coord (height - 1) */
    int gc_xoffset;              /* X offset from root's base */
    int gc_yoffset;              /* Y offset from root's base */
    int gc_xcliplo;              /* low X clipping limit */
    int gc_ycliplo;              /* low Y clipping limit */
    int gc_xcliphi;              /* high X clipping limit */
    int gc_ycliphi;              /* high Y clipping limit */
    int gc_usrxbase;             /* user window min X coordinate */
    int gc_usrybase;             /* user window min Y coordinate */
    int gc_usrwidth;             /* user window width  */
    int gc_usrheight;            /* user window height */
#define gc_baseaddr gc_frame.gf_baseaddr
#define gc_selector gc_frame.gf_selector
#define gc_onscreen gc_frame.gf_onscreen
#define gc_memflags gc_frame.gf_memflags
#define gc_lineoffset gc_frame.gf_lineoffset
#define gc_driver gc_frame.gf_driver
};

extern const struct _GR_contextInfo {
    struct _GR_context current; /* the current context */
    struct _GR_context screen;  /* the screen context */
} *const GrContextInfo;

GrContext *GrCreateContext(int w, int h, char *memory[4], GrContext *where);
GrContext *GrCreateFrameContext(GrFrameMode md, int w, int h, char *memory[4], GrContext *where);
GrContext *GrCreateSubContext(int x1, int y1, int x2, int y2, const GrContext *parent, GrContext *where);
GrContext *GrSaveContext(GrContext *where);

GrContext *GrCurrentContext(void);
GrContext *GrScreenContext(void);

void GrDestroyContext(GrContext *context);
void GrResizeSubContext(GrContext *context, int x1, int y1, int x2, int y2);
void GrSetContext(const GrContext *context);

void GrSetClipBox(int x1, int y1, int x2, int y2);
void GrSetClipBoxC(GrContext *c, int x1, int y1, int x2, int y2);
void GrGetClipBox(int *x1p, int *y1p, int *x2p, int *y2p);
void GrGetClipBoxC(const GrContext *c, int *x1p, int *y1p, int *x2p, int *y2p);
void GrResetClipBox(void);
void GrResetClipBoxC(GrContext *c);

int GrMaxX(void);
int GrMaxY(void);
int GrSizeX(void);
int GrSizeY(void);
int GrLowX(void);
int GrLowY(void);
int GrHighX(void);
int GrHighY(void);

#ifndef GRX_SKIP_INLINES
#define GrCreateContext(w, h, m, c) (GrCreateFrameContext(GrCoreFrameMode(), w, h, m, c))
#define GrCurrentContext() ((GrContext *)(&GrContextInfo->current))
#define GrScreenContext() ((GrContext *)(&GrContextInfo->screen))
#define GrMaxX() (GrCurrentContext()->gc_xmax)
#define GrMaxY() (GrCurrentContext()->gc_ymax)
#define GrSizeX() (GrMaxX() + 1)
#define GrSizeY() (GrMaxY() + 1)
#define GrLowX() (GrCurrentContext()->gc_xcliplo)
#define GrLowY() (GrCurrentContext()->gc_ycliplo)
#define GrHighX() (GrCurrentContext()->gc_xcliphi)
#define GrHighY() (GrCurrentContext()->gc_ycliphi)
#define GrGetClipBoxC(C, x1p, y1p, x2p, y2p) \
    do {                                     \
        *(x1p) = (C)->gc_xcliplo;            \
        *(y1p) = (C)->gc_ycliplo;            \
        *(x2p) = (C)->gc_xcliphi;            \
        *(y2p) = (C)->gc_ycliphi;            \
    } while (0)
#define GrGetClipBox(x1p, y1p, x2p, y2p) \
    do {                                 \
        *(x1p) = GrLowX();               \
        *(y1p) = GrLowY();               \
        *(x2p) = GrHighX();              \
        *(y2p) = GrHighY();              \
    } while (0)
#endif /* GRX_SKIP_INLINES */

/* ================================================================== */
/*                            COLOR STUFF                             */
/* ================================================================== */

/*
 * Flags to 'OR' to colors for various operations
 */
#define GrWRITE 0UL               /* write color */
#define GrXOR 0x01000000UL        /* to "XOR" any color to the screen */
#define GrOR 0x02000000UL         /* to "OR" to the screen */
#define GrAND 0x03000000UL        /* to "AND" to the screen */
#define GrIMAGE 0x04000000UL      /* BLIT: write, except given color */
#define GrCVALUEMASK 0x00ffffffUL /* color value mask */
#define GrCMODEMASK 0xff000000UL  /* color operation mask */
#define GrNOCOLOR (GrXOR | 0)     /* GrNOCOLOR is used for "no" color */

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
    GrColor ncolors;               /* number of colors */
    GrColor nfree;                 /* number of unallocated colors */
    GrColor black;                 /* the black color */
    GrColor white;                 /* the white color */
    unsigned int RGBmode;          /* set when RGB mode */
    unsigned int prec[3];          /* color field precisions */
    unsigned int pos[3];           /* color field positions */
    unsigned int mask[3];          /* masks for significant bits */
    unsigned int round[3];         /* add these for rounding */
    unsigned int shift[3];         /* shifts for (un)packing color */
    unsigned int norm;             /* normalization for (un)packing */
    struct {                       /* color table for non-RGB modes */
        unsigned char r, g, b;     /* loaded components */
        unsigned int defined : 1;  /* r,g,b values are valid if set */
        unsigned int writable : 1; /* can be changed by 'GrSetColor' */
        unsigned long int nused;   /* usage count */
    } ctable[256];
} *const GrColorInfo;

void GrResetColors(void);
void GrSetRGBcolorMode(void);
void GrRefreshColors(void);

GrColor GrNumColors(void);
GrColor GrNumFreeColors(void);

GrColor GrBlack(void);
GrColor GrWhite(void);

GrColor GrBuildRGBcolorT(int r, int g, int b);
GrColor GrBuildRGBcolorR(int r, int g, int b);
int GrRGBcolorRed(GrColor c);
int GrRGBcolorGreen(GrColor c);
int GrRGBcolorBlue(GrColor c);

GrColor GrAllocColor(int r, int g, int b);   /* shared, read-only */
GrColor GrAllocColorID(int r, int g, int b); /* potentially inlined version */
GrColor GrAllocColor2(long hcolor);          /* shared, read-only, 0xRRGGBB */
GrColor GrAllocColor2ID(long hcolor);        /* potentially inlined version */
GrColor GrAllocCell(void);                   /* unshared, read-write */

GrColor *GrAllocEgaColors(void); /* shared, read-only standard EGA colors */

void GrSetColor(GrColor c, int r, int g, int b);
void GrFreeColor(GrColor c);
void GrFreeCell(GrColor c);

void GrQueryColor(GrColor c, int *r, int *g, int *b);
void GrQueryColorID(GrColor c, int *r, int *g, int *b);
void GrQueryColor2(GrColor c, long *hcolor);
void GrQueryColor2ID(GrColor c, long *hcolor);

int GrColorSaveBufferSize(void);
void GrSaveColors(void *buffer);
void GrRestoreColors(void *buffer);

#ifndef GRX_SKIP_INLINES
#define GrColorValue(c) ((GrColor)(c)&GrCVALUEMASK)
#define GrColorMode(c) ((GrColor)(c)&GrCMODEMASK)
#define GrWriteModeColor(c) (GrColorValue(c) | GrWRITE)
#define GrXorModeColor(c) (GrColorValue(c) | GrXOR)
#define GrOrModeColor(c) (GrColorValue(c) | GrOR)
#define GrAndModeColor(c) (GrColorValue(c) | GrAND)
#define GrImageModeColor(c) (GrColorValue(c) | GrIMAGE)
#define GrNumColors() (GrColorInfo->ncolors)
#define GrNumFreeColors() (GrColorInfo->nfree)
#define GrBlack() ((GrColorInfo->black == GrNOCOLOR) ? (GrBlack)() : GrColorInfo->black)
#define GrWhite() ((GrColorInfo->white == GrNOCOLOR) ? (GrWhite)() : GrColorInfo->white)
#define GrBuildRGBcolorT(r, g, b)                                                                                                                 \
    ((((GrColor)((int)(r)&GrColorInfo->mask[0]) << GrColorInfo->shift[0]) | ((GrColor)((int)(g)&GrColorInfo->mask[1]) << GrColorInfo->shift[1]) | \
      ((GrColor)((int)(b)&GrColorInfo->mask[2]) << GrColorInfo->shift[2])) >>                                                                     \
     GrColorInfo->norm)
#define GrBuildRGBcolorR(r, g, b)                                                                                    \
    GrBuildRGBcolorT((((unsigned int)(r)) > GrColorInfo->mask[0]) ? 255 : (unsigned int)(r) + GrColorInfo->round[0], \
                     (((unsigned int)(g)) > GrColorInfo->mask[1]) ? 255 : (unsigned int)(g) + GrColorInfo->round[1], \
                     (((unsigned int)(b)) > GrColorInfo->mask[2]) ? 255 : (unsigned int)(b) + GrColorInfo->round[2])
#define GrRGBcolorRed(c) ((int)(((GrColor)(c) << GrColorInfo->norm) >> GrColorInfo->shift[0]) & (GrColorInfo->mask[0]))
#define GrRGBcolorGreen(c) ((int)(((GrColor)(c) << GrColorInfo->norm) >> GrColorInfo->shift[1]) & (GrColorInfo->mask[1]))
#define GrRGBcolorBlue(c) ((int)(((GrColor)(c) << GrColorInfo->norm) >> GrColorInfo->shift[2]) & (GrColorInfo->mask[2]))
#define GrAllocColorID(r, g, b) (GrColorInfo->RGBmode ? GrBuildRGBcolorR(r, g, b) : GrAllocColor(r, g, b))
#define GrAllocColor2(hcolor) (GrAllocColor(((hcolor & 0xff0000) >> 16), ((hcolor & 0x00ff00) >> 8), (hcolor & 0x0000ff)))
#define GrAllocColor2ID(hcolor) (GrAllocColorID(((hcolor & 0xff0000) >> 16), ((hcolor & 0x00ff00) >> 8), (hcolor & 0x0000ff)))
#define GrQueryColorID(c, r, g, b)                                                                  \
    do {                                                                                            \
        if (GrColorInfo->RGBmode) {                                                                 \
            *(r) = GrRGBcolorRed(c);                                                                \
            *(g) = GrRGBcolorGreen(c);                                                              \
            *(b) = GrRGBcolorBlue(c);                                                               \
            break;                                                                                  \
        }                                                                                           \
        if (((GrColor)(c) < GrColorInfo->ncolors) && (GrColorInfo->ctable[(GrColor)(c)].defined)) { \
            *(r) = GrColorInfo->ctable[(GrColor)(c)].r;                                             \
            *(g) = GrColorInfo->ctable[(GrColor)(c)].g;                                             \
            *(b) = GrColorInfo->ctable[(GrColor)(c)].b;                                             \
            break;                                                                                  \
        }                                                                                           \
        *(r) = *(g) = *(b) = 0;                                                                     \
    } while (0)
#define GrQueryColor2ID(c, hcolor)                                                                  \
    do {                                                                                            \
        if (GrColorInfo->RGBmode) {                                                                 \
            *(hcolor) = GrRGBcolorRed(c) << 16;                                                     \
            *(hcolor) |= GrRGBcolorGreen(c) << 8;                                                   \
            *(hcolor) |= GrRGBcolorBlue(c);                                                         \
            break;                                                                                  \
        }                                                                                           \
        if (((GrColor)(c) < GrColorInfo->ncolors) && (GrColorInfo->ctable[(GrColor)(c)].defined)) { \
            *(hcolor) = GrColorInfo->ctable[(GrColor)(c)].r;                                        \
            *(hcolor) = GrColorInfo->ctable[(GrColor)(c)].g;                                        \
            *(hcolor) = GrColorInfo->ctable[(GrColor)(c)].b;                                        \
            break;                                                                                  \
        }                                                                                           \
        *(hcolor) = 0;                                                                              \
    } while (0)
#endif /* GRX_SKIP_INLINES */

/*
 * color table (for primitives using several colors):
 *   it is an array of colors with the first element being
 *   the number of colors in the table
 */
typedef GrColor *GrColorTableP;

#define GR_CTABLE_SIZE(table) ((table) ? (unsigned int)((table)[0]) : 0U)
#define GR_CTABLE_COLOR(table, index) (((unsigned)(index) < GR_CTABLE_SIZE(table)) ? (table)[((unsigned)(index)) + 1] : GrNOCOLOR)
#define GR_CTABLE_ALLOCSIZE(ncolors) ((ncolors) + 1)

/* ================================================================== */
/*                       GRAPHICS PRIMITIVES                          */
/* ================================================================== */

#define GR_MAX_POLYGON_POINTS (1000000)
#define GR_MAX_ELLIPSE_POINTS (1024 + 5)
#define GR_MAX_ANGLE_VALUE (3600)
#define GR_ARC_STYLE_OPEN 0
#define GR_ARC_STYLE_CLOSE1 1
#define GR_ARC_STYLE_CLOSE2 2

typedef struct { /* framed box colors */
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
void GrPlot(int x, int y, GrColor c);
void GrLine(int x1, int y1, int x2, int y2, GrColor c);
void GrHLine(int x1, int x2, int y, GrColor c);
void GrVLine(int x, int y1, int y2, GrColor c);
void GrBox(int x1, int y1, int x2, int y2, GrColor c);
void GrFilledBox(int x1, int y1, int x2, int y2, GrColor c);
void GrFramedBox(int x1, int y1, int x2, int y2, int wdt, const GrFBoxColors *c);
int GrGenerateEllipse(int xc, int yc, int xa, int ya, int points[GR_MAX_ELLIPSE_POINTS][2]);
int GrGenerateEllipseArc(int xc, int yc, int xa, int ya, int start, int end, int points[GR_MAX_ELLIPSE_POINTS][2]);
void GrLastArcCoords(int *xs, int *ys, int *xe, int *ye, int *xc, int *yc);
void GrCircle(int xc, int yc, int r, GrColor c);
void GrEllipse(int xc, int yc, int xa, int ya, GrColor c);
void GrCircleArc(int xc, int yc, int r, int start, int end, int style, GrColor c);
void GrEllipseArc(int xc, int yc, int xa, int ya, int start, int end, int style, GrColor c);
void GrFilledCircle(int xc, int yc, int r, GrColor c);
void GrFilledEllipse(int xc, int yc, int xa, int ya, GrColor c);
void GrFilledCircleArc(int xc, int yc, int r, int start, int end, int style, GrColor c);
void GrFilledEllipseArc(int xc, int yc, int xa, int ya, int start, int end, int style, GrColor c);
void GrPolyLine(int numpts, int points[][2], GrColor c);
void GrPolygon(int numpts, int points[][2], GrColor c);
void GrFilledConvexPolygon(int numpts, int points[][2], GrColor c);
void GrFilledPolygon(int numpts, int points[][2], GrColor c);
void GrBitBlt(GrContext *dst, int x, int y, GrContext *src, int x1, int y1, int x2, int y2, GrColor op);
void GrBitBlt1bpp(GrContext *dst, int dx, int dy, GrContext *src, int x1, int y1, int x2, int y2, GrColor fg, GrColor bg);
void GrFloodFill(int x, int y, GrColor border, GrColor c);
void GrFloodSpill(int x1, int y1, int x2, int y2, GrColor old_c, GrColor new_c);
void GrFloodSpill2(int x1, int y1, int x2, int y2, GrColor old_c1, GrColor new_c1, GrColor old_c2, GrColor new_c2);
void GrFloodSpillC(GrContext *ctx, int x1, int y1, int x2, int y2, GrColor old_c, GrColor new_c);
void GrFloodSpillC2(GrContext *ctx, int x1, int y1, int x2, int y2, GrColor old_c1, GrColor new_c1, GrColor old_c2, GrColor new_c2);
void GrStretchBlt(GrContext *dst, int dx1, int dy1, int dx2, int dy2, GrContext *src, int x1, int y1, int x2, int y2, GrColor oper);

GrColor GrPixel(int x, int y);
GrColor GrPixelC(GrContext *c, int x, int y);

const GrColor *GrGetScanline(int x1, int x2, int yy);
const GrColor *GrGetScanlineC(GrContext *ctx, int x1, int x2, int yy);
void GrPutScanline(int x1, int x2, int yy, const GrColor *c, GrColor op);

#ifndef GRX_SKIP_INLINES
#define GrGetScanline(x1, x2, yy) GrGetScanlineC(NULL, (x1), (x2), (yy))
#endif

/* ================================================================== */
/*                 NON CLIPPING DRAWING PRIMITIVES                    */
/* ================================================================== */

void GrPlotNC(int x, int y, GrColor c);
void GrLineNC(int x1, int y1, int x2, int y2, GrColor c);
void GrHLineNC(int x1, int x2, int y, GrColor c);
void GrVLineNC(int x, int y1, int y2, GrColor c);
void GrBoxNC(int x1, int y1, int x2, int y2, GrColor c);
void GrFilledBoxNC(int x1, int y1, int x2, int y2, GrColor c);
void GrFramedBoxNC(int x1, int y1, int x2, int y2, int wdt, const GrFBoxColors *c);
void GrBitBltNC(GrContext *dst, int x, int y, GrContext *src, int x1, int y1, int x2, int y2, GrColor op);

GrColor GrPixelNC(int x, int y);
GrColor GrPixelCNC(GrContext *c, int x, int y);

#ifndef GRX_SKIP_INLINES
#define GrPlotNC(x, y, c) ((*GrCurrentFrameDriver()->drawpixel)(((x) + GrCurrentContext()->gc_xoffset), ((y) + GrCurrentContext()->gc_yoffset), ((c))))
#define GrPixelNC(x, y) \
    ((*GrCurrentFrameDriver()->readpixel)((GrFrame *)(&GrCurrentContext()->gc_frame), ((x) + GrCurrentContext()->gc_xoffset), ((y) + GrCurrentContext()->gc_yoffset)))
#define GrPixelCNC(c, x, y) ((*(c)->gc_driver->readpixel)((&(c)->gc_frame), ((x) + (c)->gc_xoffset), ((y) + (c)->gc_yoffset)))
#endif /* GRX_SKIP_INLINES */

/* ================================================================== */
/*                   FONTS AND TEXT PRIMITIVES                        */
/* ================================================================== */

/*
 * text drawing directions
 */
#define GR_TEXT_RIGHT 0 /* normal */
#define GR_TEXT_DOWN 1  /* downward */
#define GR_TEXT_LEFT 2  /* upside down, right to left */
#define GR_TEXT_UP 3    /* upward */
#define GR_TEXT_DEFAULT GR_TEXT_RIGHT
#define GR_TEXT_IS_VERTICAL(d) ((d)&1)

/*
 * text alignment options
 */
#define GR_ALIGN_LEFT 0     /* X only */
#define GR_ALIGN_TOP 0      /* Y only */
#define GR_ALIGN_CENTER 1   /* X, Y   */
#define GR_ALIGN_RIGHT 2    /* X only */
#define GR_ALIGN_BOTTOM 2   /* Y only */
#define GR_ALIGN_BASELINE 3 /* Y only */
#define GR_ALIGN_DEFAULT GR_ALIGN_LEFT

/*
 * character types in text strings
 */
#define GR_BYTE_TEXT 0       /* 1 byte per character, unknow encoding */
#define GR_WORD_TEXT 1       /* 2 bytes per character, unknow encoding */
#define GR_CP437_TEXT 2      /* 1 bpc standard DOS encoding */
#define GR_CP850_TEXT 3      /* 1 bpc latin1 DOS encoding */
#define GR_CP1252_TEXT 4     /* 1 bpc standard Win encoding */
#define GR_ISO_8859_1_TEXT 5 /* 1 bpc latin1 standard in some Linux */
#define GR_UTF8_TEXT 6       /* multibyte UTF-8 Unicode, restricted to 4 bytes */
#define GR_UCS2_TEXT 7       /* 2 bpc restricted Unicode, only BMP range */

/*
 * OR this to the foreground color value for underlined text
 */
#define GR_UNDERLINE_TEXT (GrXOR << 4)

/*
 * Font conversion flags for 'GrLoadConvertedFont'. OR them as desired.
 */
#define GR_FONTCVT_NONE 0        /* no conversion */
#define GR_FONTCVT_SKIPCHARS 1   /* load only selected characters */
#define GR_FONTCVT_RESIZE 2      /* resize the font */
#define GR_FONTCVT_ITALICIZE 4   /* tilt font for "italic" look */
#define GR_FONTCVT_BOLDIFY 8     /* make a "bold"(er) font  */
#define GR_FONTCVT_FIXIFY 16     /* convert prop. font to fixed wdt */
#define GR_FONTCVT_PROPORTION 32 /* convert fixed font to prop. wdt */

/*
 * Font encoding, to recode from user encoding to display
 */
#define GR_FONTENC_UNKNOWN 0    /* unknow encoding (no recode) */
#define GR_FONTENC_CP437 1      /* standard dos encoding */
#define GR_FONTENC_CP850 2      /* standard dos encoding */
#define GR_FONTENC_CP1252 3     /* standard Win encoding */
#define GR_FONTENC_ISO_8859_1 4 /* standard latin encoding */
#define GR_FONTENC_UNICODE 5    /* direct UNICODE encoding */
#define GR_FONTENC_MGRX512 6    /* custom MGRX 512 char encoding */
#define GR_FONTENC_ISO_8859_5 7 /* ASCII + Cyrillic */
#define GR_FONTENC_ISO_8859_7 8 /* ASCII + Greek */
#define GR_FONTENC_CP437EXT 9   /* CP437 + ISO-8859-1 + CP1252 */
#define GR_FONTENC_LASTENC 9    /* last encoding, for checks */

/*
 * font structures
 */
typedef struct _GR_fontHeader { /* font descriptor */
    char *name;                 /* font name */
    char *family;               /* font family name */
    char proportional;          /* characters have varying width */
    char scalable;              /* derived from a scalable font */
    char preloaded;             /* set when linked into program */
    char modified;              /* "tweaked" font (resized, etc..) */
    unsigned int width;         /* width (proportional=>average) */
    unsigned int height;        /* font height */
    unsigned int baseline;      /* baseline pixel pos (from top) */
    unsigned int ulpos;         /* underline pixel pos (from top) */
    unsigned int ulheight;      /* underline width */
    unsigned int minchar;       /* lowest character code in font */
    unsigned int numchars;      /* number of characters in font */
    unsigned int encoding;      /* font encoding (if known) */
} GrFontHeader;

typedef struct _GR_fontChrInfo { /* character descriptor */
    unsigned int width;          /* width of this character */
    unsigned int offset;         /* offset from start of bitmap */
} GrFontChrInfo;

typedef struct _GR_font {              /* the complete font */
    struct _GR_fontHeader h;           /* the font info structure */
    char *bitmap;                      /* character bitmap array */
    char *auxmap;                      /* map for rotated & underline chrs */
    unsigned int minwidth;             /* width of narrowest character */
    unsigned int maxwidth;             /* width of widest character */
    unsigned int auxsize;              /* allocated size of auxiliary map */
    unsigned int auxnext;              /* next free byte in auxiliary map */
    unsigned int *auxoffs[7];          /* offsets to completed aux chars */
    struct _GR_fontChrInfo chrinfo[1]; /* character info (not act. size) */
} GrFont;

/* built-in fonts */
extern GrFont GrFont_PC6x8;
extern GrFont GrFont_PC8x8;
extern GrFont GrFont_PC8x14;
extern GrFont GrFont_PC8x16;
extern GrFont GrFont_PX8x18;
extern GrFont GrFont_PX11x22;
extern GrFont GrFont_PX14x28;
/* #define GrDefaultFont   GrFont_PC8x14  Use GrGetDefaultFont() instead */

GrFont *GrGetDefaultFont(); /* GrFont_PC8x14 if not changed */
void GrSetDefaultFont(GrFont *font);

GrFont *GrLoadFont(char *name);
GrFont *GrLoadConvertedFont(char *name, int cvt, int w, int h, int minch, int maxch);
GrFont *GrBuildConvertedFont(const GrFont *from, int cvt, int w, int h, int minch, int maxch);

void GrUnloadFont(GrFont *font);
int GrDumpFont(const GrFont *font, char *CsymbolName, char *fileName);
int GrDumpFnaFont(const GrFont *font, char *fileName);
int GrDumpGrxFont(const GrFont *font, char *fileName);
void GrSetFontPath(char *path_list);

/*
 * In these functions chr is a font char index, not a real char
 * recode it before use if you want to work with real chars
 */
int GrFontCharPresent(const GrFont *font, unsigned int chr);
int GrFontCharWidth(const GrFont *font, unsigned int chr);
int GrFontCharHeight(const GrFont *font, unsigned int chr);
int GrFontCharBmpRowSize(const GrFont *font, unsigned int chr);
int GrFontCharBitmapSize(const GrFont *font, unsigned int chr);

char *GrBuildAuxiliaryBitmap(GrFont *font, unsigned int chr, int dir, int ul);
char *GrFontCharBitmap(const GrFont *font, unsigned int chr);
char *GrFontCharAuxBmp(GrFont *font, unsigned int chr, int dir, int ul);

typedef struct _GR_textOption { /* text drawing option structure */
    struct _GR_font *txo_font;  /* font to be used */
    GrColor txo_fgcolor;        /* foreground color */
    GrColor txo_bgcolor;        /* background color */
    char txo_chrtype;           /* character type (see above) */
    char txo_direct;            /* direction (see above) */
    char txo_xalign;            /* X alignment (see above) */
    char txo_yalign;            /* Y alignment (see above) */
} GrTextOption;

/*
 * The recode function from char type encoding to the font encoding
 * remember UTF-8 chr must be a char[4] packed in a long
 */
int GrFontNeedRecode(const GrFont *font, int chrtype);
unsigned int GrFontCharRecode(const GrFont *font, long chr, int chrtype);
unsigned short *GrFontTextRecode(const GrFont *font, const void *text, int length, int chrtype);
long GrFontCharReverseRecode(const GrFont *font, unsigned int chr, int chrtype);

void GrFontSetEncoding(GrFont *font, int fontencoding);
char *GrStrFontEncoding(int fontencoding);

/*
 * These functions will internally recode chars if needed from
 * char type encoding to the font encoding
 */
int GrFontStringWidth(const GrFont *font, const void *text, int len, int chrtype);
int GrFontStringHeight(const GrFont *font, const void *text, int len, int chrtype);

int GrCharWidth(long chr, const GrTextOption *opt);
int GrCharHeight(long chr, const GrTextOption *opt);
void GrCharSize(long chr, const GrTextOption *opt, int *w, int *h);
int GrStringWidth(const void *text, int length, const GrTextOption *opt);
int GrStringHeight(const void *text, int length, const GrTextOption *opt);
void GrStringSize(const void *text, int length, const GrTextOption *opt, int *w, int *h);

void GrDrawChar(long chr, int x, int y, const GrTextOption *opt);
void GrDrawString(void *text, int length, int x, int y, const GrTextOption *opt);
void GrTextXY(int x, int y, char *text, GrColor fg, GrColor bg);

#ifndef GRX_SKIP_INLINES
#define GrFontCharPresent(f, ch) (((unsigned int)(ch) < (f)->h.minchar) ? 0 : (((unsigned int)(ch) - (f)->h.minchar) < (f)->h.numchars))
#define GrFontCharWidth(f, ch) (GrFontCharPresent(f, ch) ? (int)(f)->chrinfo[(unsigned int)(ch) - (f)->h.minchar].width : (f)->h.width)
#define GrFontCharHeight(f, ch) ((f)->h.height)
#define GrFontCharBmpRowSize(f, ch) (GrFontCharPresent(f, ch) ? (((f)->chrinfo[(unsigned int)(ch) - (f)->h.minchar].width + 7) >> 3) : 0)
#define GrFontCharBitmapSize(f, ch) (GrFontCharBmpRowSize(f, ch) * (f)->h.height)
#define GrFontCharBitmap(f, ch) (GrFontCharPresent(f, ch) ? &(f)->bitmap[(f)->chrinfo[(unsigned int)(ch) - (f)->h.minchar].offset] : (char *)0)
#define GrFontCharAuxBmp(f, ch, dir, ul) ((((dir) == GR_TEXT_DEFAULT) && !(ul)) ? GrFontCharBitmap(f, ch) : GrBuildAuxiliaryBitmap((f), (ch), (dir), (ul)))
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
    GrColor lno_color;          /* color used to draw line */
    int lno_width;              /* width of the line */
    int lno_pattlen;            /* length of the dash pattern */
    unsigned char *lno_dashpat; /* draw/nodraw pattern */
} GrLineOption;

void GrCustomLine(int x1, int y1, int x2, int y2, const GrLineOption *o);
void GrCustomBox(int x1, int y1, int x2, int y2, const GrLineOption *o);
void GrCustomCircle(int xc, int yc, int r, const GrLineOption *o);
void GrCustomEllipse(int xc, int yc, int xa, int ya, const GrLineOption *o);
void GrCustomCircleArc(int xc, int yc, int r, int start, int end, int style, const GrLineOption *o);
void GrCustomEllipseArc(int xc, int yc, int xa, int ya, int start, int end, int style, const GrLineOption *o);
void GrCustomPolyLine(int numpts, int points[][2], const GrLineOption *o);
void GrCustomPolygon(int numpts, int points[][2], const GrLineOption *o);

/* ================================================================== */
/*             PATTERNED DRAWING AND FILLING PRIMITIVES               */
/* ================================================================== */

#define GR_PTYPE_BITMAP 0
#define GR_PTYPE_PIXMAP 1
#define GR_PTYPE_GRADIENT 2

/*
 * BITMAP: a mode independent way to specify a fill pattern of two
 *   colors. It is always 8 pixels wide (1 byte per scan line), its
 *   height is user-defined. SET THE TYPE FLAG TO GR_PTYPE_BITMAP!!!
 */
typedef struct _GR_bitmap {
    int bmp_ptype;       /* type flag for pattern union */
    int bmp_height;      /* bitmap height */
    char *bmp_data;      /* pointer to the bit pattern */
    GrColor bmp_fgcolor; /* foreground color for fill */
    GrColor bmp_bgcolor; /* background color for fill */
    int bmp_memflags;    /* set if dynamically allocated */
} GrBitmap;

/*
 * PIXMAP: a fill pattern stored in a layout identical to the video RAM
 *   for filling using 'bitblt'-s. It is mode dependent, typically one
 *   of the library functions is used to build it. SET THE TYPE FLAG TO
 *   GR_PTYPE_PIXMAP!!!
 */
typedef struct _GR_pixmap {
    int pxp_ptype;               /* type flag for pattern union */
    int pxp_width;               /* pixmap width (in pixels)  */
    int pxp_height;              /* pixmap height (in pixels) */
    GrColor pxp_oper;            /* bitblt mode (SET, OR, XOR, AND, IMAGE) */
    struct _GR_frame pxp_source; /* source context for fill */
} GrPixmap;

/*
 * GRADIENT: a fill pattern that change color in the direction defined by a
 *   vector (linar gradient) or from the distance to a point (radial gradient).
 *   Only can be used in rgb mode. SET THE TYPE FLAG TO GR_PTYPE_GRADIENT!!!
 */
typedef struct {
    int xi, yi, xf, yf;  // vector that define the gradient
    float dist_if;       // distance from i to f (calculated)
} GrGrdLinearData;

typedef struct {
    int xc, yc, r;  // center and radius that define the gradient
} GrGrdRadialData;

typedef struct {
    int dist;   // normalized distance (0 to 255)
    GrColor c;  // color
} GrGrdStop;

#define GR_LINEAR_GRADIENT 0
#define GR_RADIAL_GRADIENT 1

#define GR_GRADIENT_MAXSTOPS 10

typedef struct {
    int grd_ptype;     // type flag for pattern union
    int grd_mode;      // gradient mode (linear or radial)
    GrColor grd_oper;  // bitblt mode (SET, OR, XOR, AND, IMAGE)
    int grd_memflags;  // set if dynamically allocated
    union {
        GrGrdLinearData grd_ld;  // linear data
        GrGrdRadialData grd_rd;  // radial data
    };
    int grd_nstops;                            // num of stops
    GrGrdStop grd_stop[GR_GRADIENT_MAXSTOPS];  // stop definition
    int grd_genctbl;                           // color table has been generated 1=yes 0=no
    GrColor grd_ctbl[257];                     // color table ctbl[0]=num_colors=256 ever
} GrGradient;

/*
 * Fill pattern union -- can either be a bitmap, a pixmap or a gradient
 */
typedef union _GR_pattern {
    int gp_ptype;           /* type flag */
    GrBitmap gp_bitmap;     /* fill bitmap */
    GrPixmap gp_pixmap;     /* fill pixmap */
    GrGradient gp_gradient; /* fill gradient */
} GrPattern;

#define gp_bmp_data gp_bitmap.bmp_data
#define gp_bmp_height gp_bitmap.bmp_height
#define gp_bmp_fgcolor gp_bitmap.bmp_fgcolor
#define gp_bmp_bgcolor gp_bitmap.bmp_bgcolor

#define gp_pxp_width gp_pixmap.pxp_width
#define gp_pxp_height gp_pixmap.pxp_height
#define gp_pxp_oper gp_pixmap.pxp_oper
#define gp_pxp_source gp_pixmap.pxp_source

#define gp_grd_mode gp_gradient.grd_mode
#define gp_grd_oper gp_gradient.grd_oper
#define gp_grd_memflags gp_gradient.grd_memflags
#define gp_grd_ld gp_gradient.grd_ld
#define gp_grd_rd gp_gradient.grd_rd
#define gp_grd_nstops gp_gradient.grd_nstops
#define gp_grd_stop gp_gradient.grd_stop
#define gp_grd_genctbl gp_gradient.grd_genctbl
#define gp_grd_ctbl gp_gradient.grd_ctbl

GrPattern *GrBuildPixmap(const char *pixels, int w, int h, const GrColorTableP ct);
GrPattern *GrBuildPixmapNR(const char *pixels, int w, int h, const GrColorTableP ct);
GrPattern *GrBuildPixmapFromBits(const char *bits, int w, int h, GrColor fgc, GrColor bgc);
GrPattern *GrBuildPixmapFromBitsNR(const char *bits, int w, int h, GrColor fgc, GrColor bgc);
GrPattern *GrConvertToPixmap(GrContext *src);

GrPattern *GrCreateLinGradient(int xi, int yi, int xf, int yf);
GrPattern *GrCreateRadGradient(int xc, int yc, int r);
int GrAddGradientStop(GrPattern *p, int dist, GrColor c);
int GrGenGradientColorTbl(GrPattern *p);
GrColor GrGradientColor(GrPattern *p, int x, int y, int xo, int yo);
GrPattern *GrBuildPixmapFromGradient(GrPattern *p, int xo, int yo, int w, int h);

/*
 * Draw pattern for line drawings -- specifies both the:
 *   (1) fill pattern, and the
 *   (2) custom line drawing option
 */
typedef struct {
    GrPattern *lnp_pattern;   /* fill pattern */
    GrLineOption *lnp_option; /* width + dash pattern */
} GrLinePattern;

void GrDestroyPattern(GrPattern *p);

void GrPatternedLine(int x1, int y1, int x2, int y2, GrLinePattern *lp);
void GrPatternedBox(int x1, int y1, int x2, int y2, GrLinePattern *lp);
void GrPatternedCircle(int xc, int yc, int r, GrLinePattern *lp);
void GrPatternedEllipse(int xc, int yc, int xa, int ya, GrLinePattern *lp);
void GrPatternedCircleArc(int xc, int yc, int r, int start, int end, int style, GrLinePattern *lp);
void GrPatternedEllipseArc(int xc, int yc, int xa, int ya, int start, int end, int style, GrLinePattern *lp);
void GrPatternedPolyLine(int numpts, int points[][2], GrLinePattern *lp);
void GrPatternedPolygon(int numpts, int points[][2], GrLinePattern *lp);

void GrPatternFilledPlot(int x, int y, GrPattern *p);
void GrPatternFilledLine(int x1, int y1, int x2, int y2, GrPattern *p);
void GrPatternFilledBox(int x1, int y1, int x2, int y2, GrPattern *p);
void GrPatternFilledCircle(int xc, int yc, int r, GrPattern *p);
void GrPatternFilledEllipse(int xc, int yc, int xa, int ya, GrPattern *p);
void GrPatternFilledCircleArc(int xc, int yc, int r, int start, int end, int style, GrPattern *p);
void GrPatternFilledEllipseArc(int xc, int yc, int xa, int ya, int start, int end, int style, GrPattern *p);
void GrPatternFilledConvexPolygon(int numpts, int points[][2], GrPattern *p);
void GrPatternFilledPolygon(int numpts, int points[][2], GrPattern *p);
void GrPatternFloodFill(int x, int y, GrColor border, GrPattern *p);

void GrPatternDrawChar(long chr, int x, int y, const GrTextOption *opt, GrPattern *p);
void GrPatternDrawString(void *text, int length, int x, int y, const GrTextOption *opt, GrPattern *p);
void GrPatternDrawCharExt(long chr, int x, int y, const GrTextOption *opt, GrPattern *p);
void GrPatternDrawStringExt(void *text, int length, int x, int y, const GrTextOption *opt, GrPattern *p);

void GrPatndAlignLine(int xo, int yo, int x1, int y1, int x2, int y2, GrLinePattern *lp);
void GrPatndAlignBox(int xo, int yo, int x1, int y1, int x2, int y2, GrLinePattern *lp);
void GrPatndAlignCircle(int xo, int yo, int xc, int yc, int r, GrLinePattern *lp);
void GrPatndAlignEllipse(int xo, int yo, int xc, int yc, int xa, int ya, GrLinePattern *lp);
void GrPatndAlignCircleArc(int xo, int yo, int xc, int yc, int r, int start, int end, int style, GrLinePattern *lp);
void GrPatndAlignEllipseArc(int xo, int yo, int xc, int yc, int xa, int ya, int start, int end, int style, GrLinePattern *lp);
void GrPatndAlignPolyLine(int xo, int yo, int numpts, int points[][2], GrLinePattern *lp);
void GrPatndAlignPolygon(int xo, int yo, int numpts, int points[][2], GrLinePattern *lp);

void GrPatAlignFilledPlot(int xo, int yo, int x, int y, GrPattern *p);
void GrPatAlignFilledLine(int xo, int yo, int x1, int y1, int x2, int y2, GrPattern *p);
void GrPatAlignFilledBox(int xo, int yo, int x1, int y1, int x2, int y2, GrPattern *p);
void GrPatAlignFilledCircle(int xo, int yo, int xc, int yc, int r, GrPattern *p);
void GrPatAlignFilledEllipse(int xo, int yo, int xc, int yc, int xa, int ya, GrPattern *p);
void GrPatAlignFilledCircleArc(int xo, int yo, int xc, int yc, int r, int start, int end, int style, GrPattern *p);
void GrPatAlignFilledEllipseArc(int xo, int yo, int xc, int yc, int xa, int ya, int start, int end, int style, GrPattern *p);
void GrPatAlignFilledConvexPolygon(int xo, int yo, int n, int pt[][2], GrPattern *p);
void GrPatAlignFilledPolygon(int xo, int yo, int n, int pt[][2], GrPattern *p);
void GrPatAlignFloodFill(int xo, int yo, int x, int y, GrColor border, GrPattern *p);

void GrPixmapDisplay(int x, int y, GrPixmap *p);
void GrPixmapDisplayExt(int x1, int y1, int x2, int y2, GrPixmap *p);

#define GR_PIXMAP_INVLR 0x01 /* inverse left right */
#define GR_PIXMAP_INVTD 0x02 /* inverse top down */

GrPixmap *GrPixmapInverse(GrPixmap *p, int flag);
GrPixmap *GrPixmapStretch(GrPixmap *p, int nwidth, int nheight);

/* ================================================================== */
/*                      IMAGE MANIPULATION                            */
/* THESE FUNCTIONS ARE DEPRECATED AND WILL BE DELETED IN THE FUTURE   */
/* ================================================================== */

/*
 *  by Michal Stencl Copyright (c) 1998 for GRX
 *  <e-mail>    - [stenclpmd@ba.telecom.sk]
 */

#ifndef GrImage
#define GrImage GrPixmap
#endif

/* Flags for GrImageInverse() */

#define GR_IMAGE_INVERSE_LR 0x01 /* inverse left right */
#define GR_IMAGE_INVERSE_TD 0x02 /* inverse top down */

GrImage *GrImageBuild(const char *pixels, int w, int h, const GrColorTableP colors);
void GrImageDestroy(GrImage *i);
void GrImageDisplay(int x, int y, GrImage *i);
void GrImageDisplayExt(int x1, int y1, int x2, int y2, GrImage *i);
void GrImageFilledBoxAlign(int xo, int yo, int x1, int y1, int x2, int y2, GrImage *p);
void GrImageHLineAlign(int xo, int yo, int x, int y, int width, GrImage *p);
void GrImagePlotAlign(int xo, int yo, int x, int y, GrImage *p);

GrImage *GrImageInverse(GrImage *p, int flag);
GrImage *GrImageStretch(GrImage *p, int nwidth, int nheight);

GrImage *GrImageFromPattern(GrPattern *p);
GrImage *GrImageFromContext(GrContext *c);
GrImage *GrImageBuildUsedAsPattern(const char *pixels, int w, int h, const GrColorTableP colors);

GrPattern *GrPatternFromImage(GrImage *p);

#ifndef GRX_SKIP_INLINES
#define GrImageFromPattern(p) (((p) && ((p)->gp_ptype == GR_PTYPE_PIXMAP)) ? (&(p)->gp_pixmap) : NULL)
#define GrImageFromContext(c) (GrImage *)GrConvertToPixmap(c)
#define GrPatternFromImage(p) (GrPattern *)(p)
#define GrImageBuildUsedAsPattern(pixels, w, h, colors) (GrImage *)GrBuildPixmap(pixels, w, h, colors);
#define GrImageDestroy(i) GrDestroyPattern((GrPattern *)(i));
#endif

/* ================================================================== */
/*               DRAWING IN USER WINDOW COORDINATES                   */
/* ================================================================== */

void GrSetUserWindow(int x1, int y1, int x2, int y2);
void GrGetUserWindow(int *x1, int *y1, int *x2, int *y2);
void GrGetScreenCoord(int *x, int *y);
void GrGetUserCoord(int *x, int *y);

void GrUsrPlot(int x, int y, GrColor c);
void GrUsrLine(int x1, int y1, int x2, int y2, GrColor c);
void GrUsrHLine(int x1, int x2, int y, GrColor c);
void GrUsrVLine(int x, int y1, int y2, GrColor c);
void GrUsrBox(int x1, int y1, int x2, int y2, GrColor c);
void GrUsrFilledBox(int x1, int y1, int x2, int y2, GrColor c);
void GrUsrFramedBox(int x1, int y1, int x2, int y2, int wdt, GrFBoxColors *c);
void GrUsrCircle(int xc, int yc, int r, GrColor c);
void GrUsrEllipse(int xc, int yc, int xa, int ya, GrColor c);
void GrUsrCircleArc(int xc, int yc, int r, int start, int end, int style, GrColor c);
void GrUsrEllipseArc(int xc, int yc, int xa, int ya, int start, int end, int style, GrColor c);
void GrUsrFilledCircle(int xc, int yc, int r, GrColor c);
void GrUsrFilledEllipse(int xc, int yc, int xa, int ya, GrColor c);
void GrUsrFilledCircleArc(int xc, int yc, int r, int start, int end, int style, GrColor c);
void GrUsrFilledEllipseArc(int xc, int yc, int xa, int ya, int start, int end, int style, GrColor c);
void GrUsrPolyLine(int numpts, int points[][2], GrColor c);
void GrUsrPolygon(int numpts, int points[][2], GrColor c);
void GrUsrFilledConvexPolygon(int numpts, int points[][2], GrColor c);
void GrUsrFilledPolygon(int numpts, int points[][2], GrColor c);
void GrUsrFloodFill(int x, int y, GrColor border, GrColor c);

GrColor GrUsrPixel(int x, int y);
GrColor GrUsrPixelC(GrContext *c, int x, int y);

void GrUsrCustomLine(int x1, int y1, int x2, int y2, const GrLineOption *o);
void GrUsrCustomBox(int x1, int y1, int x2, int y2, const GrLineOption *o);
void GrUsrCustomCircle(int xc, int yc, int r, const GrLineOption *o);
void GrUsrCustomEllipse(int xc, int yc, int xa, int ya, const GrLineOption *o);
void GrUsrCustomCircleArc(int xc, int yc, int r, int start, int end, int style, const GrLineOption *o);
void GrUsrCustomEllipseArc(int xc, int yc, int xa, int ya, int start, int end, int style, const GrLineOption *o);
void GrUsrCustomPolyLine(int numpts, int points[][2], const GrLineOption *o);
void GrUsrCustomPolygon(int numpts, int points[][2], const GrLineOption *o);

void GrUsrPatternedLine(int x1, int y1, int x2, int y2, GrLinePattern *lp);
void GrUsrPatternedBox(int x1, int y1, int x2, int y2, GrLinePattern *lp);
void GrUsrPatternedCircle(int xc, int yc, int r, GrLinePattern *lp);
void GrUsrPatternedEllipse(int xc, int yc, int xa, int ya, GrLinePattern *lp);
void GrUsrPatternedCircleArc(int xc, int yc, int r, int start, int end, int style, GrLinePattern *lp);
void GrUsrPatternedEllipseArc(int xc, int yc, int xa, int ya, int start, int end, int style, GrLinePattern *lp);
void GrUsrPatternedPolyLine(int numpts, int points[][2], GrLinePattern *lp);
void GrUsrPatternedPolygon(int numpts, int points[][2], GrLinePattern *lp);

void GrUsrPatternFilledPlot(int x, int y, GrPattern *p);
void GrUsrPatternFilledLine(int x1, int y1, int x2, int y2, GrPattern *p);
void GrUsrPatternFilledBox(int x1, int y1, int x2, int y2, GrPattern *p);
void GrUsrPatternFilledCircle(int xc, int yc, int r, GrPattern *p);
void GrUsrPatternFilledEllipse(int xc, int yc, int xa, int ya, GrPattern *p);
void GrUsrPatternFilledCircleArc(int xc, int yc, int r, int start, int end, int style, GrPattern *p);
void GrUsrPatternFilledEllipseArc(int xc, int yc, int xa, int ya, int start, int end, int style, GrPattern *p);
void GrUsrPatternFilledConvexPolygon(int numpts, int points[][2], GrPattern *p);
void GrUsrPatternFilledPolygon(int numpts, int points[][2], GrPattern *p);
void GrUsrPatternFloodFill(int x, int y, GrColor border, GrPattern *p);

void GrUsrDrawChar(long chr, int x, int y, const GrTextOption *opt);
void GrUsrDrawString(char *text, int length, int x, int y, const GrTextOption *opt);
void GrUsrTextXY(int x, int y, char *text, GrColor fg, GrColor bg);

/* ================================================================== */
/*                    GRAPHICS CURSOR UTILITIES                       */
/* ================================================================== */

typedef struct _GR_cursor {
    struct _GR_context work; /* work areas (4) */
    int xcord, ycord;        /* cursor position on screen */
    int xsize, ysize;        /* cursor size */
    int xoffs, yoffs;        /* LU corner to hot point offset */
    int xwork, ywork;        /* save/work area sizes */
    int xwpos, ywpos;        /* save/work area position on screen */
    int displayed;           /* set if displayed */
} GrCursor;

GrCursor *GrBuildCursor(char *pixels, int pitch, int w, int h, int xo, int yo, const GrColorTableP c);
void GrDestroyCursor(GrCursor *cursor);
void GrDisplayCursor(GrCursor *cursor);
void GrEraseCursor(GrCursor *cursor);
void GrMoveCursor(GrCursor *cursor, int x, int y);

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
#define PBMFORMAT 4
#define PGMFORMAT 5
#define PPMFORMAT 6

/* The PNM functions */

int GrSaveContextToPbm(GrContext *grc, char *pbmfn, char *docn);
int GrSaveContextToPgm(GrContext *grc, char *pgmfn, char *docn);
int GrSaveContextToPpm(GrContext *grc, char *ppmfn, char *docn);
int GrLoadContextFromPnm(GrContext *grc, char *pnmfn);
int GrQueryPnm(char *pnmfn, int *width, int *height, int *maxval);
int GrLoadContextFromPnmBuffer(GrContext *grc, const char *buffer);
int GrQueryPnmBuffer(const char *buffer, int *width, int *height, int *maxval);

/* ================================================================== */
/*                           PNG FUNCTIONS                            */
/*  these functions may not be installed or available on all system   */
/* ================================================================== */

int GrPngSupport(void);
int GrSaveContextToPng(GrContext *grc, char *pngfn);
int GrLoadContextFromPng(GrContext *grc, char *pngfn, int use_alpha);
int GrQueryPng(char *pngfn, int *width, int *height);

/* ================================================================== */
/*                          JPEG FUNCTIONS                            */
/*  these functions may not be installed or available on all system   */
/* ================================================================== */

int GrJpegSupport(void);
int GrLoadContextFromJpeg(GrContext *grc, char *jpegfn, int scale);
int GrQueryJpeg(char *jpegfn, int *width, int *height);
int GrSaveContextToJpeg(GrContext *grc, char *jpegfn, int quality);
int GrSaveContextToGrayJpeg(GrContext *grc, char *jpegfn, int quality);

/* ================================================================== */
/*               MISCELLANEOUS UTILITIY FUNCTIONS                     */
/* ================================================================== */

void GrResizeGrayMap(unsigned char *map, int pitch, int ow, int oh, int nw, int nh);
int GrMatchString(const char *pattern, const char *strg);
void GrSetWindowTitle(char *title);
void GrSleep(int msec);
long GrMsecTime(void);

/* ================================================================== */
/*                               EVENTS                               */
/* ================================================================== */

typedef struct {
    int type;                 /* event type */
    long time;                /* miliseconds */
    int kbstat;               /* kb status (count for GREV_EXPOSE) */
    union {                   /* GREV_KEY     GREV_MOUSE  GREV_MMOVE  GREV_EXPOSE */
        long p1;              /* key          subevent    but status  x           */
        unsigned char cp1[4]; /* for easy access to multibyte key (like UTF-8)    */
    };
    long p2; /* type/nbytes  x           x           y           */
    long p3; /* --           y           y           width       */
    long p4; /* --           --          --          height      */
} GrEvent;

#define GREV_NULL 0   /* no event */
#define GREV_KEY 1    /* key pressed, p1=GRXkey (char or gr_keycode), p2=type or nbytes */
#define GREV_MOUSE 2  /* mouse event, p1=subevent, p2=x, p3=y */
#define GREV_MMOVE 3  /* mouse move event, p1=buttons status, p2=x, p3=y */
#define GREV_PREKEY 4 /* key event before be recoded, internal event, users don't see it */
#define GREV_EXPOSE 5 /* a window area must be redraw (generated only if user requests it */
#define GREV_WMEND 6  /* window manager wants ending (generated only if user requests it */
#define GREV_USER 100 /* user event */

#define GRKEY_KEYCODE 100 /* p1 is a special key, not a char */

#define GRMOUSE_LB_PRESSED 1   /* Left button pressed */
#define GRMOUSE_MB_PRESSED 2   /* Middle button pressed */
#define GRMOUSE_RB_PRESSED 3   /* Right button pressed */
#define GRMOUSE_LB_RELEASED 4  /* Left button released */
#define GRMOUSE_MB_RELEASED 5  /* Middle button released */
#define GRMOUSE_RB_RELEASED 6  /* Rigth button released */
#define GRMOUSE_B4_PRESSED 7   /* Button 4 pressed (scroll wheel) */
#define GRMOUSE_B4_RELEASED 8  /* Button 4 released (scroll wheel) */
#define GRMOUSE_B5_PRESSED 9   /* Button 5 pressed (scroll wheel) */
#define GRMOUSE_B5_RELEASED 10 /* Button 5 released (scroll wheel) */

#define GRMOUSE_LB_STATUS 1 /* Status bit for left button */
#define GRMOUSE_MB_STATUS 4 /* Status bit for middle button */
#define GRMOUSE_RB_STATUS 2 /* Status bit for right button */

#define GR_GEN_MMOVE_NEVER 0  /* Doesn't gen GREV_MMOVE (default) */
#define GR_GEN_MMOVE_IFBUT 1  /* Gen GREV_MMOVE if a button is pressed */
#define GR_GEN_MMOVE_ALWAYS 2 /* Gen GREV_MMOVE always */

#define GR_GEN_EXPOSE_NO 0  /* Doesn't gen GREV_EXPOSE (default) */
#define GR_GEN_EXPOSE_YES 1 /* Gen GREV_EXPOSE */

#define GR_GEN_WMEND_NO 0  /* Doesn't gen GREV_WNEND (default) */
#define GR_GEN_WMEND_YES 1 /* Gen GREV_WNEND */

#define GRKBS_RIGHTSHIFT 0x01 /* Keybd states: right shift key pressed */
#define GRKBS_LEFTSHIFT 0x02  /* left shift key pressed */
#define GRKBS_CTRL 0x04       /* CTRL pressed */
#define GRKBS_ALT 0x08        /* ALT pressed */
#define GRKBS_SCROLLOCK 0x10  /* SCROLL LOCK active */
#define GRKBS_NUMLOCK 0x20    /* NUM LOCK active */
#define GRKBS_CAPSLOCK 0x40   /* CAPS LOCK active */
#define GRKBS_INSERT 0x80     /* INSERT state active */
#define GRKBS_SHIFT (GRKBS_LEFTSHIFT | GRKBS_RIGHTSHIFT)

int GrEventInit(void);
void GrEventUnInit(void);
void GrEventFlush(void);
int GrEventCheck(void);
void GrEventRead(GrEvent *ev);
void GrEventWait(GrEvent *ev);
void GrEventWaitKeyOrClick(GrEvent *ev);
int GrEventEnqueue(GrEvent *ev);
int GrEventParEnqueue(int type, long p1, long p2, long p3, long p4);
int GrEventEnqueueFirst(GrEvent *ev);
int GrEventParEnqueueFirst(int type, long p1, long p2, long p3, long p4);
void GrEventGenMmove(int when);
void GrEventGenExpose(int when);
void GrEventGenWMEnd(int when);
int GrEventAddHook(int (*fn)(GrEvent *));
int GrEventDeleteHook(int (*fn)(GrEvent *));
/*
 * Supported kb and user encodings
 */
#define GRENC_CP437 0      /* standard DOS encoding */
#define GRENC_CP850 1      /* latin1 DOS encoding */
#define GRENC_CP1252 2     /* standard Win encoding */
#define GRENC_ISO_8859_1 3 /* standard in some Linux */
#define GRENC_UTF_8 4      /* multibyte unicode, standard in newest Linux */
#define GRENC_UCS_2 5      /* restricted unicode, 2 bytes, only BMP range */
#define GRENC_LASTENCODE 5 /* last encode, for checks */

int GrGetKbSysEncoding(void);

char *GrStrEncoding(int nenc);
int GrFindEncoding(char *strenc);

/*
 * These functions are global, no only kb related, keys are recoded to user encoding
 * and character type user strings can be deduced from user encoding
 */
int GrGetUserEncoding(void);
int GrSetUserEncoding(int enc);
char GrGetChrtypeForUserEncoding(void);

/*
 * utf-8 and recode utility functions
 */
int GrStrLen(const void *text, int chrtype);
unsigned short GrCharRecodeToUCS2(long chr, int chrtype);
long GrCharRecodeFromUCS2(long chr, int chrtype);
unsigned short *GrTextRecodeToUCS2(const void *text, int length, int chrtype);

int GrUTF8StrLen(unsigned char *s);
long GrNextUTF8Char(unsigned char *s, int *nb);
long GrUCS2ToUTF8(unsigned short ch);
unsigned short GrUTF8ToUCS2(unsigned char *s);
unsigned short *GrUTF8StrToUCS2Str(unsigned char *s, int *ulen, int maxlen);

/* ================================================================== */
/*                         MOUSE UTILITIES                            */
/* ================================================================== */

#define GR_M_CUR_NORMAL 0 /* MOUSE CURSOR modes: just the cursor */
#define GR_M_CUR_RUBBER 1 /* rectangular rubber band (XOR-d to the screen) */
#define GR_M_CUR_LINE 2   /* line attached to the cursor */
#define GR_M_CUR_BOX 3    /* rectangular box dragged by the cursor */

#define GR_MCUR_TYPE_ARROW 0 /* MOUSE CURSOR types: arrow cursor */
#define GR_MCUR_TYPE_CROSS 1 /* cross cursor */
/*
 * mouse status information
 */
extern const struct _GR_mouseInfo {
    int (*block)(GrContext *, int, int, int, int); /* mouse block function */
    void (*unblock)(int flags);                    /* mouse unblock function */
    struct _GR_cursor *cursor;                     /* the mouse cursor */
    int msstatus;                                  /* -1:missing, 0:unknown, 1:present, 2:initted */
    int displayed;                                 /* cursor is (generally) drawn */
    int blockflag;                                 /* cursor temp. erase/block flag */
    int docheck;                                   /* need to check before gr. op. to screen */
    int cursmode;                                  /* mouse cursor draw mode */
    int x1, y1, x2, y2;                            /* auxiliary params for some cursor draw modes */
    GrColor curscolor;                             /* color for some cursor draw modes */
    int owncursor;                                 /* auto generated cursor */
    int xpos, ypos;                                /* current mouse position */
    int bstatus;                                   /* buttons status */
    int genmmove;                                  /* when to generate GREV_MMOVE */
    int xmin, xmax;                                /* mouse movement X coordinate limits */
    int ymin, ymax;                                /* mouse movement Y coordinate limits */
    int spmult, spdiv;                             /* mouse cursor speed factors */
    int thresh, accel;                             /* mouse acceleration parameters */
    int moved;                                     /* mouse cursor movement flag */
} *const GrMouseInfo;

int GrMouseDetect(void);
void GrMouseSetSpeed(int spmult, int spdiv);
void GrMouseSetAccel(int thresh, int accel);
void GrMouseSetLimits(int x1, int y1, int x2, int y2);
void GrMouseGetLimits(int *x1, int *y1, int *x2, int *y2);
void GrMouseWarp(int x, int y);

GrCursor *GrMouseGetCursor(void);
void GrMouseSetCursor(GrCursor *cursor);
void GrMouseSetInternalCursor(int type, GrColor fg, GrColor bg);
void GrMouseSetCursorMode(int mode, ...);
void GrMouseDisplayCursor(void);
void GrMouseEraseCursor(void);
void GrMouseUpdateCursor(void);
int GrMouseCursorIsDisplayed(void);

int GrMouseBlock(GrContext *c, int x1, int y1, int x2, int y2);
void GrMouseUnBlock(int return_value_from_GrMouseBlock);

#ifndef GRX_SKIP_INLINES
#define GrMouseGetCursor() (GrMouseInfo->cursor)
#define GrMouseCursorIsDisplayed() (GrMouseInfo->displayed)
#define GrMouseGetLimits(x1p, y1p, x2p, y2p) \
    do {                                     \
        *(x1p) = GrMouseInfo->xmin;          \
        *(y1p) = GrMouseInfo->ymin;          \
        *(x2p) = GrMouseInfo->xmax;          \
        *(y2p) = GrMouseInfo->ymax;          \
    } while (0)
#define GrMouseBlock(c, x1, y1, x2, y2) \
    ((((c) ? (const GrContext *)(c) : GrCurrentContext())->gc_onscreen && (GrMouseInfo->docheck)) ? (*GrMouseInfo->block)((c), (x1), (y1), (x2), (y2)) : 0)
#define GrMouseUnBlock(f)                    \
    do {                                     \
        if ((f) && GrMouseInfo->displayed) { \
            (*GrMouseInfo->unblock)((f));    \
        }                                    \
    } while (0)
#endif /* GRX_SKIP_INLINES */

/* ================================================================== */
/*                             i18n catalogs                          */
/* ================================================================== */

/* A catalog implementation to help internationalize programs */
/* it doesn't depend of any other mgrx function               */
/* and it is Chrtype agnostic (this is why (void *) is used   */

#define GR_MAX_I18N_LANGS 20

int GrI18nInit(int nlg, int nstr, void *defstr);
void GrI18nEnd(void);
int GrI18nSetLabel(int lid, void *label);
void *GrI18nGetLabel(int lid);
int GrI18nSetLang(int lid);
void GrI18nAddStrings(int lid, int fsid, int nums, void **str);
void *GrI18nGetString(int sid);

#ifdef __cplusplus
}
#endif
#endif /* whole file */
