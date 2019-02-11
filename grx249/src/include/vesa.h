/**
 ** vesa.h ---- VESA BIOS extensions function codes, structures and return codes
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
 ** Contributions by: (See "doc/credits.doc" for details)
 ** Hartmut Schirmer (hsc@techfak.uni-kiel.de)
 **
 **/

#ifndef _VESA_H_included
#define _VESA_H_included

#define VESA_FUNC       0x4f00
#define VESA_SUCCESS    0x004f

#define VESA_VERSION(major,minor)       ((((major) & 0xff) << 8) | ((minor) & 0xff))
#define VESA_VERSION_MAJOR(vers)        (((vers) >> 8) & 0xff)
#define VESA_VERSION_MINOR(vers)        ((vers) & 0xff)

/*
 * VESA BIOS sub-function numbers
 */
#define VESA_VGA_INFO   0               /* get VGA adapter info */
#define VESA_MODE_INFO  1               /* get SVGA mode info */
#define VESA_SET_MODE   2               /* set video mode */
#define VESA_GET_MODE   3               /* get current video mode */
#define VESA_VGA_STATE  4               /* save/restore VGA state */
#define VESA_PAGE_CTL   5               /* memory control */
/* VESA 1.1 +++ */
#define VESA_SCAN_LNLEN 6               /* set/get scan line length */
#define VESA_DISP_START 7               /* set/get display start address */
/* VESA 1.2 +++ */
#define VESA_PAL_CNTRL  8               /* DAC palette control */
/* VESA 2.0 +++ */
#define VESA_PAL_GETSET 9               /* get/set DAC palette entries */
#define VESA_PM_INTERF  10              /* VBE protected mode interface */

#ifdef  __GNUC__
#define PACK            __attribute__((packed))
#else
#define PACK
#endif

#ifdef __WATCOMC__ /* GS - WATCOM C++ 11.0 */
#pragma pack ( 0 );
#define PACKTYPEMOD _Packed
#else
#define PACKTYPEMOD
#endif

/*
 * The VGA info structure (without padding)
 */
typedef PACKTYPEMOD struct {
    char        VESAsignature[4]  PACK;     /* should be "VESA" */
    short       VESAversion       PACK;     /* VESA version number */
    char   far *OEMstringPtr      PACK;     /* Pointer to OEM string */
    long        Capabilities      PACK;     /* capabilities of the video env */
    short  far *VideoModePtr      PACK;     /* ptr to supported Super VGA modes */
    /* ==== VESA 1.2 and later ==== */
    short       MemorySize        PACK;     /* # of 64K pages */
    /* ==== VESA 2.0 and later ==== */
    short       OEMversion        PACK;     /* OEM software revision number */
    char   far *VendorNamePtr     PACK;     /* Pointer to vendor name */
    char   far *ProductNamePtr    PACK;     /* Pointer to product name */
    char   far *RevisionStrPtr    PACK;     /* Pointer to product revision string */
} VESAvgaInfoBlock;

/*
 * Capabilities flags
 */
#define CAP_DAC_WIDTH     1           /* set if DAC width can be changed */
/* ==== VESA 2.0 and later ==== */
#define CAP_NON_VGA       2           /* set if non VGA controller */
#define CAP_DAC_BLANK     4           /* set if programmed DAC with blank bit */

/*
 * The mode information structure (without padding)
 */
typedef PACKTYPEMOD struct {
    short       ModeAttributes    PACK;     /* mode attributes */
    char        WinAAttributes    PACK;     /* Window A attributes */
    char        WinBAttributes    PACK;     /* Window B attributes */
    short       WinGranularity    PACK;     /* window granularity */
    short       WinSize           PACK;     /* window size */
    short       WinASegment       PACK;     /* Window A start segment */
    short       WinBSegment       PACK;     /* Window B start segment */
    void far  (*WinFuncPtr)()     PACK;     /* pointer to window function */
    short       BytesPerScanLine  PACK;     /* bytes per scan line */
    /* ==== extended and optional information ==== */
    short       XResolution       PACK;     /* horizontal resolution */
    short       YResolution       PACK;     /* vertical resolution */
    char        XCharSize         PACK;     /* character cell width */
    char        YCharSize         PACK;     /* character cell height */
    char        NumberOfPlanes    PACK;     /* number of memory planes */
    char        BitsPerPixel      PACK;     /* bits per pixel */
    char        NumberOfBanks     PACK;     /* number of banks */
    char        MemoryModel       PACK;     /* memory model type */
    char        BankSize          PACK;     /* bank size in K */
    char        NumImagePages     PACK;     /* number of image pages */
    char        reserved[1]       PACK;
    /* ==== VESA 1.2 and later ==== */
    char        RedMaskSize       PACK;     /* number of bits in red mask */
    char        RedMaskPos        PACK;     /* starting bit for red mask */
    char        GreenMaskSize     PACK;
    char        GreenMaskPos      PACK;
    char        BlueMaskSize      PACK;
    char        BlueMaskPos       PACK;
    char        ReservedMaskSize  PACK;     /* reserved bits in pixel */
    char        ReservedMaskPos   PACK;
    char        DirectScreenMode  PACK;
    /* ==== VESA 2.0 and later ==== */
    unsigned long LinearFrameBuffer PACK;   /* physical address of linear frame buf */
    unsigned long StartOffScreenMem PACK;   /* physical addr.: start of off screen mem */
    short       OffScreenMemSize  PACK;     /* off screen mem size in kb */
} VESAmodeInfoBlock;

/*
 * The protected mode info structure (VBE2+)
 */
typedef PACKTYPEMOD struct {
    unsigned short RealMode_SEG      PACK;     /* RealMode physical base addr */
    unsigned short RealMode_OFF      PACK;     /* of the following data table */
    unsigned short PhysicalLength    PACK;     /* length of original table */
    /* Original table starts here */
    unsigned short SetWindow_off     PACK;     /* ofs to PM Set Windows func */
    unsigned short DisplStart_off    PACK;     /* ofs to PM Set Display Start func */
    unsigned short PPalette_off      PACK;     /* ofs to PM Set Primary Palette func */
    unsigned short SubTable_off      PACK;     /* ofs to PM resource sub table */
} VESApmInfoBlock;
#define VESApmInfoBlock_BASEOFF       (3*sizeof(unsigned short))

#undef PACK
#undef PACKTYPEMOD

/*
 * MODE attribute bits
 */
#define MODE_SUPPORTED  1               /* Mode supported in hardware */
#define MODE_EXTINFO    2               /* Extended information available */
#define MODE_SUPBIOS    4               /* Text output supported by BIOS */
#define MODE_ISCOLOR    8               /* Monochrome/color mode */
#define MODE_ISGRAPHICS 16              /* Mode type (0: text, 1:graphics) */
/* ==== VESA 2.0 and later ==== */
#define MODE_NON_VGA    32              /* set if not a VGA mode */
#define MODE_NO_BANKING 64              /* set if non banking mode */
#define MODE_LIN_FRAME  128             /* set if linear frame buffer available */

/*
 * Window attribute bits
 */
#define WIN_SUPPORTED   1               /* Window supported */
#define WIN_READABLE    2               /* Window readable */
#define WIN_WRITABLE    4               /* Window writable */

/*
 * MemoryModel values
 */
#define MODEL_TEXT      0               /* 00h = Text mode */
#define MODEL_CGA       1               /* 01h = CGA graphics */
#define MODEL_HERC      2               /* 02h = Hercules graphics */
#define MODEL_4PLANE    3               /* 03h = 4-plane planar */
#define MODEL_PACKED    4               /* 04h = Packed pixel */
#define MODEL_256_NC    5               /* 05h = Non-chain 4, 256 color */
#define MODEL_DIRECT    6               /* 06h = direct color mode */
/* 07h-0Fh = Reserved, to be defined by VESA */
/* 10h-FFh = To be defined by OEM            */

int _GrViDrvVESAgetVGAinfo(VESAvgaInfoBlock *ib);
int _GrViDrvVESAgetModeInfo(int mode,VESAmodeInfoBlock *ib);
VESApmInfoBlock * _GrViDrvVESAgetPMinfo(void);

#endif
