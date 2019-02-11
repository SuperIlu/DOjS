/**
 ** w32dib24.c ---- the 256 color Win32 linear frame buffer driver using DIB
 **
 ** Copyright (c) 2003 Josu Onandia
 ** [e-mail: jonandia@fagorautomation.es]
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
#include "fdrivers/driver8.h"

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

static void w32_drawline(int x, int y, int dx, int dy, GrColor c)
{
    RECT r;

    GRX_ENTER();
    drawline(x, y, dx, dy, c);
    if (dx > 0 ) {
        r.left = x;
        r.right = x + dx + 1;
    } else {
        r.left = x + dx;
        r.right = x + 1;
    }
    if (dy > 0 ) {
        r.top = y;
        r.bottom = y + dy + 1;
    } else {
        r.top = y + dy;
        r.bottom = y + 1;
    }
    InvalidateRect(hGRXWnd, &r, FALSE);        
    GRX_LEAVE();
}

static void w32_drawhline(int x, int y, int w, GrColor c)
{
    RECT r;

    GRX_ENTER();
    drawhline(x, y, w, c);
    r.left = x;
    r.top = y;
    r.right = x + w;
    r.bottom = y + 1;
    InvalidateRect(hGRXWnd, &r, FALSE);        
    GRX_LEAVE();
}

static void w32_drawvline(int x, int y, int h, GrColor c)
{
    RECT r;

    GRX_ENTER();
    drawvline(x, y, h, c);
    r.left = x;
    r.top = y;
    r.right = x + 1;
    r.bottom = y + h;
    InvalidateRect(hGRXWnd, &r, FALSE);        
    GRX_LEAVE();
}

static void w32_drawpattern(int x, int y, int w, char patt,
                            GrColor fg, GrColor bg)
{
    RECT r;

    GRX_ENTER();
    drawpattern(x, y, w, patt, fg, bg);
    r.left = x;
    r.top = y;
    r.right = x + w;
    r.bottom = y + 1;
    InvalidateRect(hGRXWnd, &r, FALSE);        
    GRX_LEAVE();
}

static void w32_putscanline(int x, int y, int w,
                            const GrColor far *scl, GrColor op)
{
    RECT r;

    GRX_ENTER();
    putscanline(x, y, w, scl, op);
    r.left = x;
    r.top = y;
    r.right = x + w;
    r.bottom = y + 1;
    InvalidateRect(hGRXWnd, &r, FALSE);
    GRX_LEAVE();
}

static void w32_drawblock(int x, int y, int w, int h, GrColor c)
{
    RECT r;

    GRX_ENTER();
    drawblock(x, y, w, h, c);
    r.left = x;
    r.top = y;
    r.right = x + w;
    r.bottom = y + h;
    InvalidateRect(hGRXWnd, &r, FALSE);    
    GRX_LEAVE();
}

static void w32_drawbitmap(int x, int y, int w, int h, char far *bmp,
                           int pitch, int start, GrColor fg, GrColor bg)
{
    RECT r;

    GRX_ENTER();
    drawbitmap(x, y, w, h, bmp, pitch, start, fg, bg);
    r.left = x;
    r.top = y;
    r.right = x + w;
    r.bottom = y + h;
    InvalidateRect(hGRXWnd, &r, FALSE);    
    GRX_LEAVE();
}

static void w32_bitblit(GrFrame *dst, int dx, int dy, GrFrame *src,
                        int sx, int sy, int w, int h, GrColor op)
{
    RECT r;

    GRX_ENTER();
    bitblit(dst, dx, dy, src, sx, sy, w, h, op);
    r.left = dx;
    r.top = dy;
    r.right = dx + w;
    r.bottom = dy + h;
    InvalidateRect(hGRXWnd, &r, FALSE);
    GRX_LEAVE();
}

/* -------------------------------------------------------------------- */

GrFrameDriver _GrFrameDriverWIN32_8 = {
    GR_frameWIN32_8,             /* frame mode */
    GR_frameRAM8,                /* compatible RAM frame mode */
    TRUE,                        /* onscreen */
    4,                           /* line width alignment */
    1,                           /* number of planes */
    8,                           /* bits per pixel */
    8 * 16 * 1024L * 1024L,      /* max plane size the code can handle */
    NULL,
    readpixel,
    w32_drawpixel,
    w32_drawline,
    w32_drawhline,
    w32_drawvline,
    w32_drawblock,
    w32_drawbitmap,
    w32_drawpattern,
    w32_bitblit,
    bitblit,
    w32_bitblit,
    getindexedscanline,
    w32_putscanline
};

