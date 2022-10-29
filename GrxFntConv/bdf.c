/**
 ** bdf.c ---- simple and ugly BDF file reader
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
#include <stdbool.h>
#include <stdint.h>

#include "libgrx.h"
#include "bdf.h"

#define BUF_SIZE 4096
#define NUM_CHARS 256
#define MIN(x, y) ((x < y) ? (x) : (y))

//#define NUM_CHARS 3

//#define BDF_DEBUG(msg...) printf(msg)
#define BDF_DEBUG(msg...)

typedef struct __bdf_char {
    unsigned int width;
    size_t data_size;
    uint8_t *data;
} bdf_char_t;

static GrFontHeader *bdf_header;
static bdf_char_t bdf_all_chars[NUM_CHARS];

int bdf_width(int ch) {
    GRX_ENTER();
    if (ch >= 0 && ch < NUM_CHARS) {
        DBGPRINTF(DBG_FONT, ("width(%d)=%d\n", ch, bdf_all_chars[ch].width));
        return bdf_all_chars[ch].width;
    } else {
        DBGPRINTF(DBG_FONT, ("width(%d)=NIL\n", ch));
        GRX_RETURN(-1);
    }
}

int bdf_bitmap(int ch, int w, int h, char *buffer) {
    GRX_ENTER();
    DBGPRINTF(DBG_FONT, ("bitmap(%d, %d, %d)\n", ch, w, h));
    if (ch >= 0 && ch < NUM_CHARS) {
        int wanted_size = (w * h) / 8;
        if (wanted_size != bdf_all_chars[ch].data_size) {
            DBGPRINTF(DBG_FONT, ("size mismatch(%d != %d)\n", wanted_size, bdf_all_chars[ch].data_size));
            GRX_RETURN(FALSE);
        }

        memcpy(buffer, bdf_all_chars[ch].data, wanted_size);
        GRX_RETURN(TRUE);
    } else {
        GRX_RETURN(FALSE);
    }
}

static bool bdf_readline(FILE *f, char *buffer, size_t buff_size) {
    size_t pos = 0;
    buffer[pos] = 0x00;
    while (pos < buff_size - 1) {
        int ch = fgetc(f);
        if (ch == EOF || ch == '\n') {
            return true;
        } else if (ch == '\r') {
            continue;
        } else {
            buffer[pos] = ch;
            pos++;
            buffer[pos] = 0x00;
        }
    }
    return false;
}

#define NEXTLINE(b)                                                       \
    {                                                                     \
        if (!feof(f)) {                                                   \
            if (!bdf_readline(f, b, BUF_SIZE)) {                          \
                printf("buffer overflow\n");                              \
                goto error;                                               \
            }                                                             \
            BDF_DEBUG("%s:%d: in buffer: '%s'\n", __FILE__, __LINE__, b); \
        } else {                                                          \
            b[0] = 0x00;                                                  \
        }                                                                 \
    }

#define STARTSWITH(s, b) (strncmp(s, b, strlen(s)) == 0)

#define CHECKFIELD(s, b)                     \
    {                                        \
        if (strncmp(s, b, strlen(s)) != 0) { \
            UNEXPECTED(b);                   \
            goto error;                      \
        }                                    \
    }

#define UNEXPECTED(b) printf("%s:%d: unexpected data: '%s'\n", __FILE__, __LINE__, b)

bool bdf_read_data(char *fname, GrFontHeader *h, unsigned int size) {
    bool res = false;
    char buf[BUF_SIZE];
    unsigned int fields_filled = 0;
    bool at_chars = false;

    // open file
    FILE *f = fopen(fname, "r");

    // create/read header
    h->name = fname;
    h->scalable = TRUE;
    h->preloaded = FALSE;
    h->modified = GR_FONTCVT_NONE;
    h->encoding = GR_FONTENC_UNICODE;
    h->proportional = FALSE;
    h->ulheight = 1;
    h->minchar = 0;

    NEXTLINE(buf);
    CHECKFIELD("STARTFONT 2.1", buf);

    while (!feof(f)) {
        NEXTLINE(buf);
        if (STARTSWITH("FONT ", buf)) {
            h->family = malloc(BUF_SIZE);
            if (!h->family) {
                goto error;
            }
            if (sscanf(buf, "FONT %s", h->family) != 1) {
                UNEXPECTED(buf);
                goto error;
            }
            fields_filled++;
        } else if (STARTSWITH("SIZE ", buf)) {
            unsigned int height, dpix, dpiy;
            if (sscanf(buf, "SIZE %d %d %d", &height, &dpix, &dpiy) != 3) {
                UNEXPECTED(buf);
                goto error;
            }
            h->height = height;
            fields_filled++;
        } else if (STARTSWITH("FONT_ASCENT ", buf)) {
            unsigned int ascent;
            if (sscanf(buf, "FONT_ASCENT %d", &ascent) != 1) {
                UNEXPECTED(buf);
                goto error;
            }
            h->baseline = ascent;
            h->ulpos = ascent;
            fields_filled++;
        } else if (STARTSWITH("CHARS ", buf)) {
            unsigned int num_chars;
            if (sscanf(buf, "CHARS %d", &num_chars) != 1) {
                UNEXPECTED(buf);
                goto error;
            }
            h->numchars = MIN(num_chars, NUM_CHARS);
            fields_filled++;
        } else if (STARTSWITH("STARTCHAR C", buf)) {
            at_chars = true;
            break;
        } else {
            BDF_DEBUG("ignored data: '%s'\n", buf);
        }
    }

    if (fields_filled != 4) {
        printf("num header fields mismatch: %d\n", fields_filled);
        goto error;
    }

    if (size != h->height) {
        printf("font size mismatch: %d != %d\n", size, h->height);
        goto error;
    }

    while (!at_chars) {
        NEXTLINE(buf);
        if (STARTSWITH("STARTCHAR C", buf)) {
            at_chars = true;
            break;
        } else {
            printf("ignored data: '%s'\n", buf);
        }
    }

    // read characters
    for (unsigned int ch = 0; ch < NUM_CHARS; ch++) {
        bdf_all_chars[ch].width = -1;
        bdf_all_chars[ch].data_size = 0;
        bdf_all_chars[ch].data = NULL;
        if (!feof(f)) {
            CHECKFIELD("STARTCHAR C", buf);
            int idx = -1;
            while (!feof(f)) {
                if (STARTSWITH("DWIDTH ", buf)) {
                    unsigned int width, unused;
                    if (sscanf(buf, "DWIDTH %d %d", &width, &unused) != 2) {
                        UNEXPECTED(buf);
                        goto error;
                    }
                    if (idx >= 0 && idx < NUM_CHARS) {
                        bdf_all_chars[idx].width = width;
                        h->width = width;
                    } else {
                        printf("index error: %d\n", idx);
                        goto error;
                    }
                } else if (STARTSWITH("STARTCHAR C", buf)) {
                    unsigned int start_char;
                    if (sscanf(buf, "STARTCHAR C%x", &start_char) != 1) {
                        UNEXPECTED(buf);
                        goto error;
                    }
                    BDF_DEBUG("index is: %d\n", start_char);
                    idx = start_char;
                } else if (STARTSWITH("ENDCHAR", buf)) {
                    break;
                } else if (STARTSWITH("BITMAP", buf)) {
                    if (idx >= 0 && idx < NUM_CHARS && bdf_all_chars[idx].width != 0) {
                        int line_bytes = ((bdf_all_chars[idx].width - 1) / 8 + 1);
                        bdf_all_chars[idx].data_size = line_bytes * h->height;
                        bdf_all_chars[idx].data = malloc(bdf_all_chars[idx].data_size);
                        if (!bdf_all_chars[idx].data) {
                            printf("ENOMEM %ld bytes\n", bdf_all_chars[idx].data_size);
                            goto error;
                        }
                        int p = 0;
                        for (int y = 0; y < h->height; y++) {
                            BDF_DEBUG("Glyphline %d of %d\n", y, h->height);
                            NEXTLINE(buf);
                            for (int x = 0; x < line_bytes; x++) {
                                int num;
                                if (sscanf(&buf[2 * x], "%02X", &num) != 1) {
                                    UNEXPECTED(&buf[2 * x]);
                                    goto error;
                                }

                                if (p > bdf_all_chars[idx].data_size) {
                                    printf("dest buffer out of bounds");
                                    goto error;
                                }
                                bdf_all_chars[idx].data[p] = num;
                                p++;
                            }
                        }
                        BDF_DEBUG("Glyphdata done\n");
                    } else {
                        printf("index error: %d\n", idx);
                        goto error;
                    }

                } else {
                    BDF_DEBUG("ignored data: '%s'\n", buf);
                }
                NEXTLINE(buf);
            }
        }
        NEXTLINE(buf);
    }

    res = true;
    bdf_header = h;

error:
    fclose(f);
    return res;
}
