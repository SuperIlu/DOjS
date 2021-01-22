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

alpng_filereader.c -- File structure handling.

*/

#include <allegro.h>
#include <allegro/internal/aintern.h>

#include "alpng.h"
#include "alpng_internal.h"
#include "inflate/inflate.h"
#include "quantization/octree.h"

static int check_signature(PACKFILE* f);
static int read_chunk_head(PACKFILE* f, struct alpng_chunk* chunk);
static int read_header(PACKFILE* f, struct alpng_chunk* chunk, struct alpng_header* header);
static int read_data(PACKFILE* f, struct alpng_chunk* chunk, struct input_data* data);
static int valid_color_type_and_depth(int color_type, int depth);
static int read_palette(PACKFILE* f, struct alpng_header* header, struct alpng_chunk* chunk);

BITMAP* load_png(AL_CONST char* filename, RGB* pal) {
    BITMAP* b;
    PACKFILE* f;
    
    ASSERT(filename);
    
    f = pack_fopen(filename, F_READ);
    if (!f) {
        alpng_error_msg = "Cannot open file!";
        return 0;
    }
    
    b = load_png_pf(f, pal);
    
    pack_fclose(f);
    return b;
}

BITMAP* load_png_pf(PACKFILE *f, RGB *pal) {
    struct alpng_chunk chunk;
    struct alpng_header header;
    uint8_t *raw_data;
    uint32_t raw_data_length = 0;
    struct input_data input_data;
    BITMAP* b;
    unsigned int res, i;
    struct alpng_fake_headers fake_headers;
    int source_depth, dest_depth;
    PALETTE tmppal;

    ASSERT(f);
    
    if (pal) {
        header.pal = pal;
    } else {
        header.pal = tmppal;
    }
    
    if (!check_signature(f)) {
        return 0;
    }

    if (!read_chunk_head(f, &chunk)) {
        return 0;
    }
    if (chunk.type != AL_ID('I', 'H', 'D', 'R')) {
        alpng_error_msg = "First chunk must be of IHDR type!";
        return 0;
    }
    if (!read_header(f, &chunk, &header)) {
        return 0;
    }

    header.has_palette = 0;
    do {
        (void) pack_igetl(f); /* skip CRC */
        if (!read_chunk_head(f, &chunk)) {
            return 0;
        }
        if (chunk.type == AL_ID('P', 'L', 'T', 'E')) {
            if (!read_palette(f, &header, &chunk)) {
                return 0;
            }
            header.has_palette = 1;
        } else if (chunk.type != AL_ID('I', 'D', 'A', 'T')) {
            if (pack_fseek(f, chunk.length) != 0) {
                alpng_error_msg = "Cannot fseek!";
                return 0;
            }
        }
    } while (chunk.type != AL_ID('I', 'D', 'A', 'T'));

    if (header.type == 3 && !header.has_palette) {
        alpng_error_msg = "Palette required but not found!";
        return 0;
    }

    input_data.data = 0;
    input_data.i = 0;
    input_data.length = 0;
    while (chunk.type == AL_ID('I', 'D', 'A', 'T')) {
        if (chunk.length > 0) {
            if (input_data.length == 0) {
                input_data.data = (uint8_t*) malloc(chunk.length);
                if (!input_data.data) {
                    alpng_error_msg = "Cannot allocate memory!";
                    return 0;
                }
                input_data.length = chunk.length;
            } else {
                input_data.i = input_data.length;
                input_data.length += chunk.length;
                input_data.data = (uint8_t*) realloc(input_data.data, input_data.length);
                if (!input_data.data) {
                    alpng_error_msg = "Cannot allocate memory!";
                    return 0;
                }
            }
            if (!read_data(f, &chunk, &input_data)) {
                alpng_error_msg = "Cannot read image data!";
                free(input_data.data);
                return 0;
            }
        }
        (void) pack_igetl(f); /* skip CRC */
        if (!read_chunk_head(f, &chunk)) {
            free(input_data.data);
            return 0;
        }
    }

    if (input_data.length < 7) {
        alpng_error_msg = "File without data!";
        free(input_data.data);
        return 0;
    }
    
    input_data.error = 0;
    input_data.i = 0;
    input_data.si = 8;
    
    if (header.interlace) {
        alpng_fill_fake_headers(&header, &fake_headers);
        for (i = 0; i < 7; i++) {
            raw_data_length += fake_headers.fake_raw_data_length[i];
        }
    } else {
        raw_data_length = alpng_calc_raw_data_length(&header);
    }
    raw_data = (uint8_t*) malloc(raw_data_length);
    if (!raw_data) {
        alpng_error_msg = "Cannot allocate memory!";
        free(input_data.data);
        return 0;
    }
    
    res = alpng_inflate(&input_data, raw_data, raw_data_length, &alpng_error_msg);
    free(input_data.data);
    if (!res) {
        free(raw_data);
        return 0;
    }

    if (res != raw_data_length) {
        alpng_error_msg = "Not enough data!";
        free(raw_data);
        return 0;
    }

    if (header.interlace) {
        b = alpng_draw_interlaced(&header, raw_data, &fake_headers);
    } else {
        b = alpng_draw(&header, raw_data, raw_data_length);
    }
    free(raw_data);
    if (!b) {
        return 0;
    }
    
    source_depth = bitmap_color_depth(b);
    dest_depth = _color_load_depth(source_depth, header.type & 4);
    if (source_depth != dest_depth) {
        /* if we are converting to 8 bits and the picture had palette, use it
           instead of building Allegro optimized palette */
        if (dest_depth == 8 && header.has_palette && pal) {
            select_palette(header.pal);
            b = _fixup_loaded_bitmap(b, 0, dest_depth);
            unselect_palette();
#ifndef ALPNG_USE_ALLEGRO_QUANTIZATION
        } else if (dest_depth == 8 && pal) {
            b = octree_quantize(b, pal, 1);
            if (!b) {
                alpng_error_msg = "Cannot do quantization!";
                return 0;
            }
#endif
        } else {
            b = _fixup_loaded_bitmap(b, source_depth == 8 ? header.pal : pal, dest_depth);
        }
    }
    
    if (pal && dest_depth != 8 && source_depth != 8) {
        generate_332_palette(pal);
    }

    while (chunk.type != AL_ID('I', 'E', 'N', 'D')) {
        if (pack_fseek(f, chunk.length) != 0) {
            destroy_bitmap(b);
            alpng_error_msg = "Cannot fseek!";
            return 0;
        }
        (void) pack_igetl(f); /* skip CRC */
        if (!read_chunk_head(f, &chunk)) {
            destroy_bitmap(b);
            return 0;
        }
    }
    if (chunk.length != 0) {
        alpng_error_msg = "Incorrect IEND length!";
        destroy_bitmap(b);
        return 0;
    }

    return b;
}

static int read_palette(PACKFILE* f, struct alpng_header* header, struct alpng_chunk* chunk) {
    int i, n;
    if (chunk->length % 3) {
        alpng_error_msg = "Palette chunk length not divisible by 3!";
        return 0;
    }
    n = chunk->length / 3;
    if (n < 1 || n > 256) {
        alpng_error_msg = "Incorrect number of palette entries!";
        return 0;
    }
    for (i = 0; i < n; i++) {
        header->pal[i].r = pack_getc(f) / 4;
        if (i == EOF) {
            alpng_error_msg = "Cannot read palette!";
            return 0;
        }
        header->pal[i].g = pack_getc(f) / 4;
        if (i == EOF) {
            alpng_error_msg = "Cannot read palette!";
            return 0;
        }
        header->pal[i].b = pack_getc(f) / 4;
        if (i == EOF) {
            alpng_error_msg = "Cannot read palette!";
            return 0;
        }
    }
    for (i = n; i < 256; i++) {
        header->pal[i].r = 0;
        header->pal[i].g = 0;
        header->pal[i].b = 0;
    }
    return 1;
}

static int read_data(PACKFILE* f, struct alpng_chunk* chunk, struct input_data* data) {
    if ((uint32_t)pack_fread(data->data + data->i, chunk->length, f) != chunk->length) {
        return 0;
    }
    return 1;
}

static int read_header(PACKFILE* f, struct alpng_chunk* chunk, struct alpng_header* header) {
    int i;
    if (chunk->length != 13) {
        alpng_error_msg = "Incorrect IHDR length!";
        return 0;
    }
    if (pack_feof(f) || pack_ferror(f)) {
        alpng_error_msg = "Cannot read image width!";
        return 0;
    }
    header->width = (uint32_t) pack_mgetl(f);
    if (header->width == 0) {
        alpng_error_msg = "Zero width!";
        return 0;
    }
    if (pack_feof(f) || pack_ferror(f)) {
        alpng_error_msg = "Cannot read image height!";
        return 0;
    }
    header->height = (uint32_t) pack_mgetl(f);
    if (header->height == 0) {
        alpng_error_msg = "Zero height!";
        return 0;
    }

    i = pack_getc(f);
    if (i == EOF) {
        alpng_error_msg = "Cannot read image bit depth!";
        return 0;
    }
    header->depth = (uint8_t) i;

    i = pack_getc(f);
    if (i == EOF) {
        alpng_error_msg = "Cannot read image color type!";
        return 0;
    }
    header->type = (uint8_t) i;
    
    if (!valid_color_type_and_depth(header->type, header->depth)) {
        alpng_error_msg = "Unsupported color type/depth combination!";
        return 0;
    }

    i = pack_getc(f);
    if (i == EOF) {
        alpng_error_msg = "Cannot read image compression method!";
        return 0;
    }
    if (i != 0) {
        alpng_error_msg = "Unsupported compression method!";
        return 0;
    }

    i = pack_getc(f);
    if (i == EOF) {
        alpng_error_msg = "Cannot read filter method!";
        return 0;
    }
    if (i != 0) {
        alpng_error_msg = "Unsupported filter method!";
        return 0;
    }

    i = pack_getc(f);
    if (i == EOF) {
        alpng_error_msg = "Cannot read interlace method!";
        return 0;
    }
    if (i != 0 && i != 1) {
        alpng_error_msg = "Interlace not supported!";
        return 0;
    }
    header->interlace = (uint8_t) i;
    
    return 1;
}

static int read_chunk_head(PACKFILE* f, struct alpng_chunk* chunk) {
    if (pack_feof(f) || pack_ferror(f)) {
        alpng_error_msg = "Cannot read chunk length!";
        return 0;
    }
    chunk->length = (uint32_t) pack_mgetl(f);
    if (pack_feof(f) || pack_ferror(f)) {
        alpng_error_msg = "Cannot read chunk type!";
        return 0;
    }
    chunk->type = (uint32_t) pack_mgetl(f);
    return 1;
}

static int check_signature(PACKFILE* f) {
    int i;
    uint8_t* header = (uint8_t*) malloc(ALPNG_PNG_HEADER_LEN);
    if (pack_fread(header, ALPNG_PNG_HEADER_LEN, f) != ALPNG_PNG_HEADER_LEN) {
        alpng_error_msg = "File shorter than PNG signature!";
        free(header);
        return 0;
    }
    for (i = 0; i < ALPNG_PNG_HEADER_LEN; i++) {
        if (header[i] != ALPNG_PNG_HEADER[i]) {
            alpng_error_msg = "File signature mismatch!";
            free(header);
            return 0;
        }
    }
    free(header);
    return 1;
}

static int valid_color_type_and_depth(int color_type, int depth) {
    switch (color_type) {
        case 0:
            if (depth == 1 || depth == 2 || depth == 4 || depth == 8 || depth == 16) {
                return 1;
            }
            break;
        case 3:
            if (depth == 1 || depth == 2 || depth == 4 || depth == 8) {
                return 1;
            }
            break;
        case 2:
        case 4:
        case 6:
            if (depth == 8 || depth == 16) {
                return 1;
            }
            break;
    }
    return 0;
}

uint32_t alpng_calc_raw_data_length(struct alpng_header* header) {
    int i = 0, j = 0;

    /* for subimages used in deinterlacing */
    if (header->width == 0) {
        return 0;
    }

    switch (header->depth) {
        case 1:
            i = (header->width + 7) / 8;
            break;
        case 2:
            i = (header->width + 3) / 4;
            break;
        case 4:
            i = (header->width + 1) / 2;
            break;
        case 8:
            i = header->width;
            break;
        case 16:
            i = 2 * header->width;
            break;
    }
    switch (header->type) {
        case 0:
        case 3:
            j = 1;
            break;
        case 2:
            j = 3;
            break;
        case 4:
            j = 2;
            break;
        case 6:
            j = 4;
            break;
    }

    return header->height * i * j + header->height;
}
