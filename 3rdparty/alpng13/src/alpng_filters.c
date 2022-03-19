/*
AllegroPNG

Copyright (c) 2006 Michal Molhanec

This software is provided 'as-is', without any express or implied
warranty. In no event will the authors be held liable for any damages
arising from the use of this software.

Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute
it freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented;
     you must not claim that you wrote the original software.
     If you use this software in a product, an acknowledgment
     in the product documentation would be appreciated but
     is not required.

  2. Altered source versions must be plainly marked as such,
     and must not be misrepresented as being the original software.

  3. This notice may not be removed or altered from any
     source distribution.
*/

/*

alpng_filters.c -- PNG scanline filter implementation.

*/

#include <allegro.h>
#include "alpng.h"
#include "alpng_internal.h"

static int calc_filter_delta(struct alpng_header* header);

/* a = left, b = above, c = upper left */
static INLINE uint8_t paeth_predictor(uint32_t a, uint32_t b, uint32_t c) {
    uint32_t p = a + b - c;
    uint32_t pa = abs(p - a);
    uint32_t pb = abs(p - b);
    uint32_t pc = abs(p - c);
    if (pa <= pb && pa <= pc) {
        return a;
    }
    else if (pb <= pc) {
        return b;
    }
    else {
        return c;
    }
}

uint8_t* alpng_filter(uint8_t* data, uint32_t w, uint32_t h, uint32_t filter_delta) {
    uint32_t x, y, i;
    uint8_t a, b, c;
    uint8_t* filtered_data = (uint8_t*) malloc(w * h);
    if (!filtered_data) {
        /* We cannot allocate memory so we don't filter data. However
           we can still try to save the data unprocessed. */
        return data;
    }
    for (y = 0; y < h; y++) {
        i = y * w;
        filtered_data[i] = 4;
        for (x = 1; x < w; x++) {
            if (x > filter_delta) {
                a = data[i + x - filter_delta];
            } else {
                a = 0;
            }
            if (y > 0) {
                b = data[i + x - w];
            } else {
                b = 0;
            }
            if (x > filter_delta && y > 0) {
                c = data[i + x - w - filter_delta];
            } else {
                c = 0;
            }
            filtered_data[i + x] = data[i + x] - paeth_predictor(a, b, c);
        }
    }
    free(data);
    return filtered_data;
}

int alpng_unfilter(uint8_t* data, struct alpng_header* header) {
    unsigned int y, x, filter_type, i, a, b, c;
    unsigned int filter_delta = calc_filter_delta(header);
    for (y = 0; y < header->height; y++) {
        i = y * header->byte_width;
        filter_type = data[i];
        switch (filter_type) {
            case 0: /* None */
                continue;
            case 1: /* Sub */
                for (x = filter_delta + 1; x < header->byte_width; x++) {
                    data[i + x] += data[i + x - filter_delta];
                }
                break;
            case 2: /* Up */
                if (y > 0) {
                    for (x = 1; x < header->byte_width; x++) {
                        data[i + x] += data[i + x - header->byte_width];
                    }
                }
                break;
            case 3: /* Average */
                if (y > 0) {
                    for (x = 1; x <= filter_delta; x++) {
                        data[i + x] += data[i + x - header->byte_width] / 2;
                    }
                    for (x = filter_delta + 1; x < header->byte_width; x++) {
                        a = data[i + x - filter_delta];
                        b = data[i + x - header->byte_width];
                        data[i + x] += (a + b) / 2;
                    }
                } else {
                    for (x = filter_delta + 1; x < header->byte_width; x++) {
                        data[i + x] += data[i + x - filter_delta] / 2;
                    }
                }
                break;
            case 4: /* Paeth */
                if (y > 0) {
                    for (x = 1; x <= filter_delta; x++) {
                        data[i + x] += data[i + x - header->byte_width];
                    }
                    for (x = filter_delta + 1; x < header->byte_width; x++) {
                        a = data[i + x - filter_delta];
                        b = data[i + x - header->byte_width];
                        c = data[i + x - header->byte_width - filter_delta];
                        data[i + x] += paeth_predictor(a, b, c);
                    }
                } else {
                    for (x = filter_delta + 1; x < header->byte_width; x++) {
                        data[i + x] += data[i + x - filter_delta];
                    }
                }
                break;
            default:
                alpng_error_msg = "Unknown filter!";
                return 0;
        }
    }
    return 1;
}

static int calc_filter_delta(struct alpng_header* header) {
    int depth = header->depth;
    switch (header->type) {
        case 0:
            if (depth == 16) {
                return 2;
            }
            break;
        case 2:
            if (depth == 8) {
                return 3;
            }
            if (depth == 16) {
                return 6;
            }
            break;
        case 4:
            if (depth == 8) {
                return 2;
            }
            if (depth == 16) {
                return 4;
            }
        case 6:
            if (depth == 8) {
                return 4;
            }
            if (depth == 16) {
                return 8;
            }
            break;
    }
    return 1;
}
