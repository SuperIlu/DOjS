/**
 ** fntconv.c ---- convert TTF to GRX font
 **
 ** Copyright (C) 2021 Andre Seidelt <superilu@yahoo.com>
 **
 ** This program is based on the GRX graphics library.
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

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
#include <math.h>

#include "grxdebug.h"
#include "libgrx.h"
#include "grfontdv.h"
#include "bdf.h"

char *_GR_debug_file;
int _GR_debug_line;
const char *_GR_debug_function;

#define MIN(x, y) ((x < y) ? (x) : (y))
#define MAX(x, y) ((x > y) ? (x) : (y))

#define MIN_CHAR 32
#define MAX_CHAR 255

#define OUT_GRX 0
#define OUT_C 1
#define OUT_FNA 2

void _GR_debug_printf(char *fmt, ...) {
#ifdef DEBUG
    va_list ap;
    printf("%s|%s|%d: ", _GR_debug_file, _GR_debug_function, _GR_debug_line);
    va_start(ap, fmt);
    vprintf(fmt, ap);
    va_end(ap);
#endif
}

static void usage(char *prgname) {
    printf("Usage:\n");
    printf("  %s [-cf] <BDF file> <size> <outfile>\n", prgname);
}

int main(int argc, char *argv[]) {
    int opt;
    char *name = argv[1];
    int size = 0;
    char *outfile = argv[3];
    int out_format = OUT_GRX;
    GrFontHeader h;
    GrFont *fnt;

    // check parameters
    while ((opt = getopt(argc, argv, "cf")) != -1) {
        switch (opt) {
            case 'c':
                out_format = OUT_C;
                break;
            case 'f':
                out_format = OUT_FNA;
                break;
            default: /* '?' */
                usage(argv[0]);
                exit(1);
        }
    }

    if (optind + 3 > argc) {
        usage(argv[0]);
        exit(1);
    }

    // create in/outfile/size/basename
    name = argv[optind];
    size = atoi(argv[optind + 1]);
    outfile = argv[optind + 2];

    if (size <= 0) {
        printf("Invalid size: %d\n", size);
        exit(1);
    }

    if (bdf_read_data(name, &h, size)) {
        printf("BDF loaded\n");

        // create GrFont
        fnt = _GrBuildFont(&h, GR_FONTCVT_NONE, 0, 0, 0, 0, bdf_width, bdf_bitmap, FALSE);
    } else {
        printf("font building error\n");
        exit(1);
    }

    int res;
    if (out_format == OUT_GRX) {
        res = GrDumpGrxFont(fnt, outfile);
    } else if (out_format == OUT_C) {
        res = GrDumpFont(fnt, "fontdef", outfile);
    } else if (out_format == OUT_FNA) {
        res = GrDumpFnaFont(fnt, outfile);
    } else {
        printf("Unknown output format %d\n", out_format);
        exit(1);
    }

    if (res) {
        printf("Wrote output to %s\n", outfile);
        printf("  width    : %d\n", h.width);
        printf("  height   : %d\n", h.height);
        printf("  baseline : %d\n", h.baseline);
        exit(0);
    } else {
        printf("Error writing to %s\n", outfile);
        exit(1);
    }
}
