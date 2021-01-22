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

alpng_save.c -- Simple saving to PNG.

*/

#include <allegro.h>
#include <allegro/internal/aintern.h>

#include "alpng.h"
#include "alpng_internal.h"

#if defined(ALPNG_ZLIB) && (ALPNG_ZLIB != 0)
    #include "wrappers/deflate.h"
#endif

#define CRC_RESET 0xFFFFFFFFL

#define PUTC(what)                                              \
    if (pack_putc((what), f) == EOF) {                          \
        alpng_error_msg = "Cannot write to the file!";          \
        free(data);                                             \
        free(compressed_data);                                  \
        return -1;                                              \
    }

#define IPUTW(what)                                             \
    if (pack_iputw((what), f) == EOF) {                         \
        alpng_error_msg = "Cannot write to the file!";          \
        free(data);                                             \
        free(compressed_data);                                  \
        return -1;                                              \
    }

#define MPUTL(what)                                             \
    if (pack_mputl((what), f) == EOF) {                         \
        alpng_error_msg = "Cannot write to the file!";          \
        free(data);                                             \
        free(compressed_data);                                  \
        return -1;                                              \
    }

#define FWRITE(p, n)                                            \
    if (pack_fwrite((p), (n), f) != (long)(n)) {                \
        alpng_error_msg = "Cannot write to the file!";          \
        free(data);                                             \
        free(compressed_data);                                  \
        return -1;                                              \
    }

#define PUTC_CRC(what)                                          \
    PUTC(what)                                                  \
    crc = crc_table[(crc ^ (what)) & 0xFF] ^ (crc >> 8);

#define MPUTL_CRC(what)                                         \
    MPUTL(what)                                                 \
    n = (what);                                                 \
    helper = (uint8_t*) &n;                                     \
    crc = crc_table[(crc ^ helper[3]) & 0xFF] ^ (crc >> 8);     \
    crc = crc_table[(crc ^ helper[2]) & 0xFF] ^ (crc >> 8);     \
    crc = crc_table[(crc ^ helper[1]) & 0xFF] ^ (crc >> 8);     \
    crc = crc_table[(crc ^ helper[0]) & 0xFF] ^ (crc >> 8);

#define IPUTW_CRC(what)                                         \
    IPUTW(what)                                                 \
    n = (what);                                                 \
    helper = (uint8_t*) &n;                                     \
    crc = crc_table[(crc ^ helper[0]) & 0xFF] ^ (crc >> 8);     \
    crc = crc_table[(crc ^ helper[1]) & 0xFF] ^ (crc >> 8);

#define FWRITE_CRC(p, n)                                        \
    FWRITE((p), (n))                                            \
    for (k = 0; k < (n); k++) {                                 \
        crc = crc_table[(crc ^ (p)[k]) & 0xFF] ^ (crc >> 8);    \
    }

#define FWRITE_CRC_ADLER(p, n)                                  \
    FWRITE((p), (n))                                            \
    for (i = 0; i < (n); i++) {                                 \
        adler_s1 = (adler_s1 + (p)[i]) % 65521;                 \
        adler_s2 = (adler_s2 + adler_s1) % 65521;               \
        crc = crc_table[(crc ^ (p)[i]) & 0xFF] ^ (crc >> 8);    \
    }

#define PUT_CRC                                                 \
    MPUTL_CRC(crc ^ 0xFFFFFFFFL)                                \
    crc = CRC_RESET;


#if (ALLEGRO_VERSION == 4 && ALLEGRO_SUB_VERSION == 2 &&        \
     ALLEGRO_WIP_VERSION == 0)                                  \
    /* From gfx.c */

    /* _bitmap_has_alpha:
     *  Checks whether this bitmap has an alpha channel.
     */
    static int _bitmap_has_alpha(BITMAP *bmp)
    {
       int x, y, c;

       if (bitmap_color_depth(bmp) != 32)
          return FALSE;

       for (y = 0; y < bmp->h; y++) {
          for (x = 0; x < bmp->w; x++) {
        c = getpixel(bmp, x, y);
        if (geta32(c))
           return TRUE;
          }
       }

       return FALSE;
    }
#endif

int save_png(AL_CONST char *filename, BITMAP *bmp, AL_CONST RGB *pal) {
    int ret;
    PACKFILE* f;

    ASSERT(filename);

    f = pack_fopen(filename, F_WRITE);
    if (!f) {
        alpng_error_msg = "Cannot open file!";
        return -1;
    }

    ret = save_png_pf(f, bmp, pal);

    pack_fclose(f);
    return ret;
}

int save_png_pf(PACKFILE *f, BITMAP *bmp, AL_CONST RGB *pal) {
    int i, j;
    int bpp, samples_per_pixel;
    uint32_t crc = CRC_RESET;
    uint32_t n;
    uint8_t *data = 0, *helper;
    uint32_t k;
    uint32_t crc_table[256];
    uint32_t data_length;
    uint8_t* compressed_data = 0;
    #if defined(ALPNG_ZLIB) && (ALPNG_ZLIB != 0)
        uint32_t compressed_data_length;
    #else
        uint16_t adler_s1 = 1, adler_s2 = 0;
        int block_length;
    #endif

    ASSERT(f);
    ASSERT(bmp);
    
    if (bmp->w == 0 || bmp->h == 0) {
        alpng_error_msg = "Width or height or both of the image are 0!";
        return -1;
    }
    
    /* precompute crc table */
    for (i = 0; i < 256; i++) {
        k = i;
        for (j = 0; j < 8; j++) {
            if (k & 1) {
                k = 0xEDB88320L ^ (k >> 1);
            } else {
                k >>= 1;
            }
        }
        crc_table[i] = k;
    }
    
    /* file id */
    for (i = 0; i < ALPNG_PNG_HEADER_LEN; i++) {
        PUTC(ALPNG_PNG_HEADER[i])
    }
    
    /* chunk length */
    MPUTL(13)

    /* header chunk */
    MPUTL_CRC(AL_ID('I', 'H', 'D', 'R'))

    /* width */
    MPUTL_CRC(bmp->w)

    /* height */
    MPUTL_CRC(bmp->h)

    /* bit depth */
    PUTC_CRC(8)

    /* color type */
    bpp = bitmap_color_depth(bmp);
    if (bpp == 8) {
        PUTC_CRC(3); /* palette */
        samples_per_pixel = 1;
    } else if (_bitmap_has_alpha(bmp)) {
        PUTC_CRC(6); /* RGBA */
        samples_per_pixel = 4;
    } else {
        PUTC_CRC(2); /* RGB */
        samples_per_pixel = 3;
    }

    /* compression method */
    PUTC_CRC(0)

    /* filter method */
    PUTC_CRC(0)

    /* interlace method */
    PUTC_CRC(0)
    
    /* end of IHDR */
    PUT_CRC
    
    /* palette */
    if (bpp == 8) {
        MPUTL(PAL_SIZE * 3)
        MPUTL_CRC(AL_ID('P', 'L', 'T', 'E'));
        for (i = 0; i < PAL_SIZE; i++) {
            PUTC_CRC(pal[i].r * 4)
            PUTC_CRC(pal[i].g * 4)
            PUTC_CRC(pal[i].b * 4)
        }
        PUT_CRC
    }

    /* IDAT */
    
    /* the last bmp->h is for filter subtype byte */
    data_length = bmp->w * bmp->h * samples_per_pixel + bmp->h;
    data = (uint8_t*) malloc(data_length);
    if (!data) {
        alpng_error_msg = "Cannot allocate memory!";
        return -1;                                              \
    }
    k = 0;
    for (i = 0; i < bmp->h; i++) {
        /* filter subtype */
        data[k++] = 0;
        for (j = 0; j < bmp->w; j++) {
            if (bpp == 8) {
                data[k++] = bmp->line[i][j];
            } else {
                int color = getpixel(bmp, j, i);
                data[k++] = getr_depth(bpp, color);
                data[k++] = getg_depth(bpp, color);
                data[k++] = getb_depth(bpp, color);
                if (samples_per_pixel == 4) {
                    data[k++] = geta_depth(bpp, color);
                }
            }
        }
    }
    
    #if !defined(ALPNG_ZLIB) || (ALPNG_ZLIB == 0)
        /* Maximum size of uncompressed block is 0xFFFF */
        /* 2 bytes for zlib header, 5 bytes per block, 4 bytes for adler32 checksum */
        MPUTL(2 + data_length + (data_length / 0xFFFF + 1) * 5 + 4)
        MPUTL_CRC(AL_ID('I', 'D', 'A', 'T'))
        PUTC_CRC(0x78) /* deflate compression */
        PUTC_CRC(1) /* 31 - (0x78 * 256) % 31 */
        k = 0;
        while (data_length > 0) {
            block_length = data_length > 0xFFFF ? 0xFFFF : data_length;
            data_length -= block_length;
            PUTC_CRC(data_length == 0 ? 1 : 0) /* final block ? */
            IPUTW_CRC(block_length)
            IPUTW_CRC(block_length ^ 0xFFFF)
            FWRITE_CRC_ADLER(data + k * 0xFFFF, block_length)
            k++;
        }
        MPUTL_CRC(adler_s2 * 65536 + adler_s1)
        PUT_CRC
    #else
        if (bpp != 8) {
            data = alpng_filter(data, bmp->w * samples_per_pixel + 1, bmp->h, samples_per_pixel);
        }
        if (!alpng_deflate(data, data_length, &compressed_data, &compressed_data_length, &alpng_error_msg)) {
            free(data);
            return -1;
        }
        MPUTL(compressed_data_length)
        MPUTL_CRC(AL_ID('I', 'D', 'A', 'T'))
        FWRITE_CRC(compressed_data, compressed_data_length)
        PUT_CRC
    #endif
    
    /* IEND */
    MPUTL(0)
    MPUTL_CRC(AL_ID('I', 'E', 'N', 'D'))
    PUT_CRC

    free(data);
    free(compressed_data);
    return 0;
}
