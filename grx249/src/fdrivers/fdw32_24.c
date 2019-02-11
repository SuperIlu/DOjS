/**
 ** w32dib24.c ---- the 16M color Win32 linear frame buffer driver using DIB
 **
 ** Copyright (c) 2003 Mariano Alvarez Fernandez
 ** [e-mail: malfer@telefonica.net]
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

#include "libwin32.h"
#include "fdrivers/driver24.h"

static void w32_drawpixel(int x, int y, GrColor color)
{
    HDC hDC;
    COLORREF c;

    GRX_ENTER();
    drawpixel(x, y, color);

    c = GetPixel ( hDCMem, x, y );
    hDC = GetDC ( hGRXWnd );
    SetPixelV ( hDC, x, y, c );
    ReleaseDC ( hGRXWnd, hDC );

    GRX_LEAVE();
}

static void w32_drawline(int x, int y, int dx, int dy, GrColor color)
{
    RECT Rect;

    GRX_ENTER();
    drawline(x, y, dx, dy, color);
    if (dx > 0 ) {
        Rect.left = x;
        Rect.right = x + dx + 1;
    } else {
        Rect.left = x + dx;
        Rect.right = x + 1;
    }
    if (dy > 0 ) {
        Rect.top = y;
        Rect.bottom = y + dy + 1;
    } else {
        Rect.top = y + dy;
        Rect.bottom = y + 1;
    }
    InvalidateRect(hGRXWnd, &Rect, FALSE);
    GRX_LEAVE();
}

static void w32_drawhline(int x, int y, int w, GrColor color)
{
    RECT Rect;

    GRX_ENTER();
    drawhline(x, y, w, color);
    Rect.left = x;
    Rect.top = y;
    Rect.right = x + w;
    Rect.bottom = y + 1;
    InvalidateRect(hGRXWnd, &Rect, FALSE);
    GRX_LEAVE();
}

static void w32_drawvline(int x, int y, int h, GrColor color)
{
    RECT Rect;

    GRX_ENTER();
    drawvline(x, y, h, color);
    Rect.left = x;
    Rect.top = y;
    Rect.right = x + 1;
    Rect.bottom = y + h;
    InvalidateRect(hGRXWnd, &Rect, FALSE);
    GRX_LEAVE();
}

static void w32_drawblock(int x, int y, int w, int h, GrColor color)
{
    RECT Rect;

    GRX_ENTER();
    drawblock(x, y, w, h, color);
    Rect.left = x;
    Rect.top = y;
    Rect.right = x + w;
    Rect.bottom = y + h;
    InvalidateRect(hGRXWnd, &Rect, FALSE);
    GRX_LEAVE();
}

static void w32_drawbitmap(int x, int y, int w, int h, char far *bmp,
                           int pitch, int start, GrColor fg, GrColor bg)
{
    RECT Rect;

    GRX_ENTER();
    drawbitmap(x, y, w, h, bmp, pitch, start, fg, bg);
    Rect.left = x;
    Rect.top = y;
    Rect.right = x + w;
    Rect.bottom = y + h;
    InvalidateRect(hGRXWnd, &Rect, FALSE);
    GRX_LEAVE();
}

static void w32_drawpattern(int x, int y, int w, char patt,
                            GrColor fg, GrColor bg)
{
    RECT Rect;

    GRX_ENTER();
    drawpattern(x, y, w, patt, fg, bg);
    Rect.left = x;
    Rect.top = y;
    Rect.right = x + w;
    Rect.bottom = y + 1;
    InvalidateRect(hGRXWnd, &Rect, FALSE);
    GRX_LEAVE();
}

static void w32_bitblt(GrFrame *dst, int dx, int dy, GrFrame *src,
                       int sx, int sy, int w, int h, GrColor op)
{
    RECT Rect;

    GRX_ENTER();
    bitblt(dst, dx, dy, src, sx, sy, w, h, op);
    Rect.left = dx;
    Rect.top = dy;
    Rect.right = dx + w;
    Rect.bottom = dy + h;
    InvalidateRect(hGRXWnd, &Rect, FALSE);
    GRX_LEAVE();
}

void w32_putscanline(int x, int y, int w,
                     const GrColor far *scl, GrColor op )
{
    GrColor skipc;
    RECT Rect;

    GRX_ENTER();
    Rect.left = x;
    Rect.top = y;
    Rect.right = x + w;
    Rect.bottom = y + 1;

    skipc = op ^ GrIMAGE;
    op &= GrCMODEMASK;
    for ( w += x; x < w; ++x) {
        GrColor c = *(scl++);
        if (c != skipc) drawpixel(x, y, (c|op));
    }

    InvalidateRect(hGRXWnd, &Rect, FALSE);
    GRX_LEAVE();
}

/* -------------------------------------------------------------------- */

GrFrameDriver _GrFrameDriverWIN32_24 = {
    GR_frameWIN32_24,           /* frame mode */
    GR_frameRAM24,              /* compatible RAM frame mode */
    TRUE,                       /* onscreen */
    4,                          /* line width alignment */
    1,                          /* number of planes */
    24,                         /* bits per pixel */
    24 * 16 * 1024L * 1024L,    /* max plane size the code can handle */
    NULL,
    readpixel,
    w32_drawpixel,
    w32_drawline,
    w32_drawhline,
    w32_drawvline,
    w32_drawblock,
    w32_drawbitmap,
    w32_drawpattern,
    w32_bitblt,
    bitblt,
    w32_bitblt,
    _GrFrDrvGenericGetIndexedScanline,
    w32_putscanline
};
