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

alpng_drawer.c -- Makes Allegro BITMAPs from raw uncompressed data.

*/

#include <allegro.h>

#include "alpng.h"
#include "alpng_internal.h"

static BITMAP* read_paletted(uint8_t* data, uint32_t data_length, struct alpng_header* header);
static BITMAP* read_grayscale(uint8_t* data, uint32_t data_length, struct alpng_header* header);
static BITMAP* read_grayscale16(uint8_t* data, uint32_t data_length, struct alpng_header* header);
static BITMAP* read_rgb16(uint8_t* data, uint32_t data_length, struct alpng_header* header);
static BITMAP* read_rgb8(uint8_t* data, uint32_t data_length, struct alpng_header* header);
static BITMAP* read_grayscale_with_alpha16(uint8_t* data, uint32_t data_length, struct alpng_header* header);
static BITMAP* read_grayscale_with_alpha8(uint8_t* data, uint32_t data_length, struct alpng_header* header);
static BITMAP* read_rgb_with_alpha16(uint8_t* data, uint32_t data_length, struct alpng_header* header);
static BITMAP* read_rgb_with_alpha8(uint8_t* data, uint32_t data_length, struct alpng_header* header);

BITMAP* alpng_draw(struct alpng_header* header, uint8_t* data, uint32_t data_length) {
    BITMAP* b = 0;

    if (data_length % header->height) {
        alpng_error_msg = "Data length not divisible by picture height!";
        return 0;
    }
    header->byte_width = data_length / header->height;

    if (!alpng_unfilter(data, header)) {
        return 0;
    }
    
    switch (header->type) {
        case 0:
            if (header->depth == 16) {
                b = read_grayscale16(data, data_length, header);
            } else {
                b = read_grayscale(data, data_length, header);
            }
            break;
        case 2:
            if (header->depth == 16) {
                b = read_rgb16(data, data_length, header);
            } else {
                b = read_rgb8(data, data_length, header);
            }
            break;
        case 3:
            b = read_paletted(data, data_length, header);
            break;
        case 4:
            if (header->depth == 16) {
                b = read_grayscale_with_alpha16(data, data_length, header);
            } else {
                b = read_grayscale_with_alpha8(data, data_length, header);
            }
            break;
        case 6:
            if (header->depth == 16) {
                b = read_rgb_with_alpha16(data, data_length, header);
            } else {
                b = read_rgb_with_alpha8(data, data_length, header);
            }
            break;
    }
    
    return b;
}

static BITMAP* read_rgb_with_alpha8(uint8_t* data, uint32_t data_length, struct alpng_header* header) {
    unsigned int x, y;
    BITMAP* b;
    if (header->height * (header->width * 4 + 1) != data_length) {
        alpng_error_msg = "Bad data length!";
        return 0;
    }
    b = create_bitmap_ex(32, header->width, header->height);
    if (!b) {
        alpng_error_msg = "Cannot allocate bitmap!";
        return 0;
    }
    for (y = 0; y < header->height; y++) {
        for (x = 0; x < header->width; x++) {
            b->line[y][x * 4 + _rgb_r_shift_32 / 8] = data[1 + y * header->byte_width + x * 4];
            b->line[y][x * 4 + _rgb_g_shift_32 / 8] = data[2 + y * header->byte_width + x * 4];
            b->line[y][x * 4 + _rgb_b_shift_32 / 8] = data[3 + y * header->byte_width + x * 4];
            b->line[y][x * 4 + _rgb_a_shift_32 / 8] = data[4 + y * header->byte_width + x * 4];
        }
    }
    return b;
}

static BITMAP* read_rgb_with_alpha16(uint8_t* data, uint32_t data_length, struct alpng_header* header) {
    unsigned int x, y;
    BITMAP* b;
    if (header->height * (header->width * 8 + 1) != data_length) {
        alpng_error_msg = "Bad data length!";
        return 0;
    }
    b = create_bitmap_ex(32, header->width, header->height);
    if (!b) {
        alpng_error_msg = "Cannot allocate bitmap!";
        return 0;
    }
    for (y = 0; y < header->height; y++) {
        for (x = 0; x < header->width; x++) {
            b->line[y][x * 4 + _rgb_r_shift_32 / 8] = data[1 + y * header->byte_width + x * 8];
            b->line[y][x * 4 + _rgb_g_shift_32 / 8] = data[3 + y * header->byte_width + x * 8];
            b->line[y][x * 4 + _rgb_b_shift_32 / 8] = data[5 + y * header->byte_width + x * 8];
            b->line[y][x * 4 + _rgb_a_shift_32 / 8] = data[7 + y * header->byte_width + x * 8];
        }
    }
    return b;
}

static BITMAP* read_grayscale_with_alpha8(uint8_t* data, uint32_t data_length, struct alpng_header* header) {
    unsigned int x, y;
    BITMAP* b;
    if (header->height * (header->width * 2 + 1) != data_length) {
        alpng_error_msg = "Bad data length!";
        return 0;
    }
    b = create_bitmap_ex(32, header->width, header->height);
    if (!b) {
        alpng_error_msg = "Cannot allocate bitmap!";
        return 0;
    }
    for (y = 0; y < header->height; y++) {
        for (x = 0; x < header->width; x++) {
            b->line[y][x * 4 + _rgb_r_shift_32 / 8] = data[1 + y * header->byte_width + x * 2];
            b->line[y][x * 4 + _rgb_g_shift_32 / 8] = data[1 + y * header->byte_width + x * 2];
            b->line[y][x * 4 + _rgb_b_shift_32 / 8] = data[1 + y * header->byte_width + x * 2];
            b->line[y][x * 4 + _rgb_a_shift_32 / 8] = data[2 + y * header->byte_width + x * 2];
        }
    }
    return b;
}

static BITMAP* read_grayscale_with_alpha16(uint8_t* data, uint32_t data_length, struct alpng_header* header) {
    unsigned int x, y;
    BITMAP* b;
    if (header->height * (header->width * 4 + 1) != data_length) {
        alpng_error_msg = "Bad data length!";
        return 0;
    }
    b = create_bitmap_ex(32, header->width, header->height);
    if (!b) {
        alpng_error_msg = "Cannot allocate bitmap!";
        return 0;
    }
    for (y = 0; y < header->height; y++) {
        for (x = 0; x < header->width; x++) {
            b->line[y][x * 4 + _rgb_r_shift_32 / 8] = data[1 + y * header->byte_width + x * 4];
            b->line[y][x * 4 + _rgb_g_shift_32 / 8] = data[1 + y * header->byte_width + x * 4];
            b->line[y][x * 4 + _rgb_b_shift_32 / 8] = data[1 + y * header->byte_width + x * 4];
            b->line[y][x * 4 + _rgb_a_shift_32 / 8] = data[3 + y * header->byte_width + x * 4];
        }
    }
    return b;
}

static BITMAP* read_rgb16(uint8_t* data, uint32_t data_length, struct alpng_header* header) {
    unsigned int x, y;
    BITMAP* b;
    if (header->height * (header->width * 6 + 1) != data_length) {
        alpng_error_msg = "Bad data length!";
        return 0;
    }
    b = create_bitmap_ex(24, header->width, header->height);
    if (!b) {
        alpng_error_msg = "Cannot allocate bitmap!";
        return 0;
    }
    for (y = 0; y < header->height; y++) {
        for (x = 0; x < header->width; x++) {
            b->line[y][x * 3 + _rgb_r_shift_24 / 8] = data[1 + y * header->byte_width + x * 6];
            b->line[y][x * 3 + _rgb_g_shift_24 / 8] = data[3 + y * header->byte_width + x * 6];
            b->line[y][x * 3 + _rgb_b_shift_24 / 8] = data[5 + y * header->byte_width + x * 6];
        }
    }
    return b;
}

static BITMAP* read_rgb8(uint8_t* data, uint32_t data_length, struct alpng_header* header) {
    unsigned int x, y;
    BITMAP* b;
    if (header->height * (header->width * 3 + 1) != data_length) {
        alpng_error_msg = "Bad data length!";
        return 0;
    }
    b = create_bitmap_ex(24, header->width, header->height);
    if (!b) {
        alpng_error_msg = "Cannot allocate bitmap!";
        return 0;
    }
    for (y = 0; y < header->height; y++) {
        for (x = 0; x < header->width; x++) {
            b->line[y][x * 3 + _rgb_r_shift_24 / 8] = data[1 + y * header->byte_width + x * 3];
            b->line[y][x * 3 + _rgb_g_shift_24 / 8] = data[2 + y * header->byte_width + x * 3];
            b->line[y][x * 3 + _rgb_b_shift_24 / 8] = data[3 + y * header->byte_width + x * 3];
        }
    }
    return b;
}

static BITMAP* read_grayscale16(uint8_t* data, uint32_t data_length, struct alpng_header* header) {
    unsigned int x, y;
    BITMAP* b;
    if (header->height * (header->width * 2 + 1) != data_length) {
        alpng_error_msg = "Bad data length!";
        return 0;
    }
    b = create_bitmap_ex(24, header->width, header->height);
    if (!b) {
        alpng_error_msg = "Cannot allocate bitmap!";
        return 0;
    }
    for (y = 0; y < header->height; y++) {
        for (x = 0; x < header->width; x++) {
            b->line[y][x * 3 + _rgb_r_shift_24 / 8] = data[1 + y * header->byte_width + x * 2];
            b->line[y][x * 3 + _rgb_g_shift_24 / 8] = data[1 + y * header->byte_width + x * 2];
            b->line[y][x * 3 + _rgb_b_shift_24 / 8] = data[1 + y * header->byte_width + x * 2];
        }
    }
    return b;
}

static BITMAP* read_grayscale(uint8_t* data, uint32_t data_length, struct alpng_header* header) {
    unsigned int x, y, i, j, mask = 0, width;
    int sx;
    BITMAP* b;
    uint8_t sample;
    int sample_multiple = 0;
    switch (header->depth) {
        case 1:
            sample_multiple = 255;
            break;
        case 2:
            sample_multiple = 85;
            break;
        case 4:
            sample_multiple = 17;
            break;
        case 8:
            sample_multiple = 1;
            break;
    }
    j = 8 / header->depth;
    width = (header->width + j - 1) / j;
    if (header->height * (width + 1) != data_length) {
        alpng_error_msg = "Bad data length!";
        return 0;
    }
    b = create_bitmap_ex(24, header->width, header->height);
    if (!b) {
        alpng_error_msg = "Cannot allocate bitmap!";
        return 0;
    }
    for (i = 0; i < header->depth; i++) {
        mask |= 1 << i;
    }
    for (y = 0; y < header->height; y++) {
        i = 0;
        for (x = 0; x < width; x++) {
            for (sx = j - 1; sx >= 0; sx--) {
                if (i == header->width) {
                    break;
                }
                sample = (data[y * header->byte_width + x + 1] & (mask << (header->depth * sx))) >> (header->depth * sx);
                sample *= sample_multiple;
                b->line[y][i * 3]     = sample;
                b->line[y][i * 3 + 1] = sample;
                b->line[y][i * 3 + 2] = sample;
                i++;
            }
        }
    }
    return b;
}

static BITMAP* read_paletted(uint8_t* data, uint32_t data_length, struct alpng_header* header) {
    unsigned int x, y, i, j, mask = 0, width;
    int sx;
    BITMAP* b;
    j = 8 / header->depth;
    width = (header->width + j - 1) / j;
    if (header->height * (width + 1) != data_length) {
        alpng_error_msg = "Bad data length!";
        return 0;
    }
    b = create_bitmap_ex(8, header->width, header->height);
    if (!b) {
        alpng_error_msg = "Cannot allocate bitmap!";
        return 0;
    }
    for (i = 0; i < header->depth; i++) {
        mask |= 1 << i;
    }
    for (y = 0; y < header->height; y++) {
        i = 0;
        for (x = 0; x < width; x++) {
            for (sx = j - 1; sx >= 0; sx--) {
                if (i == header->width) {
                    break;
                }
                b->line[y][i++] = (data[y * header->byte_width + x + 1] & (mask << (header->depth * sx))) >> (header->depth * sx);
            }
        }
    }
    return b;
}


