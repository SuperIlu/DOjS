/**
 ** dumpgrx.c ---- write a grx font file from a font in memory
 **
 ** Copyright (C) 2017 Mariano Alvarez Fernandez
 ** [e-mail: malfer at telefonica.net]
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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "libgrx.h"
#include "fdv_grx.h"

#if BYTE_ORDER == BIG_ENDIAN
#include "ordswap.h"
static void swap_header(GrFontFileHeaderGRX *fhdr) {
    GRX_ENTER();
    _GR_swap32u(fhdr->magic);
    _GR_swap32u(fhdr->bmpsize);
    _GR_swap16u(fhdr->width);
    _GR_swap16u(fhdr->height);
    _GR_swap16u(fhdr->minchar);
    _GR_swap16u(fhdr->maxchar);
    _GR_swap16u(fhdr->isfixed);
    _GR_swap16u(fhdr->encoding);
    _GR_swap16u(fhdr->baseline);
    _GR_swap16u(fhdr->undwidth);
    /* no need to change fnname  && family */
    GRX_LEAVE();
}

static void swap_wtable(GR_int16u *wtable, unsigned int wtsize) {
    GR_int16u *wt;
    unsigned int ws;

    GRX_ENTER();
    wt = wtable;
    ws = wtsize / sizeof(GR_int16u);
    while (ws-- > 0) {
        _GR_swap16u(wt);
        ++wt;
    }
    GRX_LEAVE();
}
#endif

int GrDumpGrxFont(const GrFont *f, char *fileName) {
    FILE *fp;
    GrFontFileHeaderGRX fhdr;
    int isfixed = 0;
    GR_int16u *wtable = NULL;
    unsigned int wtsize = 0;
    GR_int32u bmpsize = 0;
    unsigned int i, w;

    fp = fopen(fileName, "w");
    if (!fp) return 0;

    isfixed = f->h.proportional ? FALSE : TRUE;

    fhdr.magic = GRX_FONTMAGIC;
    fhdr.bmpsize = 0;
    fhdr.width = f->h.width;
    fhdr.height = f->h.height;
    fhdr.minchar = f->h.minchar;
    fhdr.maxchar = f->h.numchars + f->h.minchar - 1;
    fhdr.isfixed = isfixed;
    fhdr.encoding = f->h.encoding;
    fhdr.baseline = f->h.baseline;
    fhdr.undwidth = f->h.ulheight;
    memset(fhdr.fnname, '\0', GRX_NAMEWIDTH);
    strncpy(fhdr.fnname, f->h.name, GRX_NAMEWIDTH - 1);
    memset(fhdr.family, '\0', GRX_NAMEWIDTH);
    strncpy(fhdr.family, f->h.family, GRX_NAMEWIDTH - 1);

    if (!isfixed) {
        wtsize = sizeof(GR_int16u) * (fhdr.maxchar - fhdr.minchar + 1);
        wtable = malloc(wtsize);
        if (wtable == NULL) {
            fclose(fp);
            return 0;
        }
        for (i = 0; i < f->h.numchars; i++) {
            w = f->chrinfo[i].width;
            wtable[i] = w;
            bmpsize += ((w + 7) >> 3) * f->h.height;
        }
    } else {
        bmpsize = (GR_int32u)(f->h.numchars) * ((f->h.width + 7) >> 3) * f->h.height;
    }
    fhdr.bmpsize = bmpsize;

#if BYTE_ORDER == BIG_ENDIAN
    swap_header(&fhdr);
    if (!isfixed) swap_wtable(wtable, wtsize);
#endif

    fwrite(&fhdr, sizeof(fhdr), 1, fp);
    if (!isfixed) fwrite(wtable, wtsize, 1, fp);
    fwrite(f->bitmap, bmpsize, 1, fp);

    if (!isfixed) free(wtable);

    fclose(fp);
    return 1;
}
