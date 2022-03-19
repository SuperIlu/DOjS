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

alpng_interlacing.c -- Decoding of interlaced images.

The deinterlacing is done this way: we behave like having 7 different
images, loading them and finally merge them into the result.

*/

#include <string.h>
#include <allegro.h>
#include "alpng.h"
#include "alpng_internal.h"

void alpng_fill_fake_headers(struct alpng_header* header, struct alpng_fake_headers* fake_headers) {
    fake_headers->h[0].width = (header->width + 7) / 8;
    fake_headers->h[0].height = (header->height + 7) / 8;
    fake_headers->h[0].depth = header->depth;
    fake_headers->h[0].type = header->type;
    fake_headers->fake_raw_data_length[0] = alpng_calc_raw_data_length(&fake_headers->h[0]);
    fake_headers->h[1].width = (header->width - 4 + 7) / 8;
    fake_headers->h[1].height = (header->height + 7) / 8;
    fake_headers->h[1].depth = header->depth;
    fake_headers->h[1].type = header->type;
    fake_headers->fake_raw_data_length[1] = alpng_calc_raw_data_length(&fake_headers->h[1]);
    fake_headers->h[2].width = (header->width + 3) / 4;
    fake_headers->h[2].height = (header->height - 4 + 7) / 8;
    fake_headers->h[2].depth = header->depth;
    fake_headers->h[2].type = header->type;
    fake_headers->fake_raw_data_length[2] = alpng_calc_raw_data_length(&fake_headers->h[2]);
    fake_headers->h[3].width = (header->width - 2 + 3) / 4;
    fake_headers->h[3].height = (header->height + 3) / 4;
    fake_headers->h[3].depth = header->depth;
    fake_headers->h[3].type = header->type;
    fake_headers->fake_raw_data_length[3] = alpng_calc_raw_data_length(&fake_headers->h[3]);
    fake_headers->h[4].width = (header->width + 1) / 2;
    fake_headers->h[4].height = (header->height - 2 + 3) / 4;
    fake_headers->h[4].depth = header->depth;
    fake_headers->h[4].type = header->type;
    fake_headers->fake_raw_data_length[4] = alpng_calc_raw_data_length(&fake_headers->h[4]);
    fake_headers->h[5].width = (header->width - 1 + 1) / 2;
    fake_headers->h[5].height = (header->height + 1) / 2;
    fake_headers->h[5].depth = header->depth;
    fake_headers->h[5].type = header->type;
    fake_headers->fake_raw_data_length[5] = alpng_calc_raw_data_length(&fake_headers->h[5]);
    fake_headers->h[6].width = header->width;
    fake_headers->h[6].height = (header->height - 1 + 1) / 2;
    fake_headers->h[6].depth = header->depth;
    fake_headers->h[6].type = header->type;
    fake_headers->fake_raw_data_length[6] = alpng_calc_raw_data_length(&fake_headers->h[6]);
}

BITMAP* alpng_draw_interlaced(struct alpng_header* header, uint8_t* data, struct alpng_fake_headers* fake_headers) {
    BITMAP *b1 = 0, *b2 = 0, *b3 = 0, *b4 = 0, *b5 = 0, *b6 = 0, *b7 = 0;
    uint32_t old_fake_raw_data_length;
    BITMAP* result;
    int y, x;
    unsigned int dest_step;
    
    b1 = alpng_draw(&fake_headers->h[0], data, fake_headers->fake_raw_data_length[0]);
    if (!b1) {
        return 0;
    }
    old_fake_raw_data_length = fake_headers->fake_raw_data_length[0];

    if (header->width > 4) {
        b2 = alpng_draw(&fake_headers->h[1], data + old_fake_raw_data_length, fake_headers->fake_raw_data_length[1]);
        if (!b2) {
            destroy_bitmap(b1);
            return 0;
        }
        old_fake_raw_data_length += fake_headers->fake_raw_data_length[1];
    }

    if (header->height > 4) {
        b3 = alpng_draw(&fake_headers->h[2], data + old_fake_raw_data_length, fake_headers->fake_raw_data_length[2]);
        if (!b3) {
            destroy_bitmap(b1);
            destroy_bitmap(b2);
            return 0;
        }
        old_fake_raw_data_length += fake_headers->fake_raw_data_length[2];
    }

    if (header->width > 2) {
        b4 = alpng_draw(&fake_headers->h[3], data + old_fake_raw_data_length, fake_headers->fake_raw_data_length[3]);
        if (!b4) {
            destroy_bitmap(b1);
            destroy_bitmap(b2);
            destroy_bitmap(b3);
            return 0;
        }
        old_fake_raw_data_length += fake_headers->fake_raw_data_length[3];
    }
    
    if (header->height > 2) {
        b5 = alpng_draw(&fake_headers->h[4], data + old_fake_raw_data_length, fake_headers->fake_raw_data_length[4]);
        if (!b5) {
            destroy_bitmap(b1);
            destroy_bitmap(b2);
            destroy_bitmap(b3);
            destroy_bitmap(b4);
            return 0;
        }
        old_fake_raw_data_length += fake_headers->fake_raw_data_length[4];
    }

    if (header->width > 1) {
        b6 = alpng_draw(&fake_headers->h[5], data + old_fake_raw_data_length, fake_headers->fake_raw_data_length[5]);
        if (!b6) {
            destroy_bitmap(b1);
            destroy_bitmap(b2);
            destroy_bitmap(b3);
            destroy_bitmap(b4);
            destroy_bitmap(b5);
            return 0;
        }
        old_fake_raw_data_length += fake_headers->fake_raw_data_length[5];
    }

    if (header->height > 1) {
        b7 = alpng_draw(&fake_headers->h[6], data + old_fake_raw_data_length, fake_headers->fake_raw_data_length[6]);
        if (!b7) {
            destroy_bitmap(b1);
            destroy_bitmap(b2);
            destroy_bitmap(b3);
            destroy_bitmap(b4);
            destroy_bitmap(b5);
            destroy_bitmap(b6);
            return 0;
        }
    }

    result = create_bitmap_ex(bitmap_color_depth(b1), header->width, header->height);
    if (result) {
        dest_step = bitmap_color_depth(b1) / 8;

        for (y = 0; y < b1->h; y++) {
            for (x = 0; x < b1->w; x++) {
                memcpy(
                    &result->line[y * 8][x * dest_step * 8],
                    &b1->line[y][x * dest_step],
                    dest_step
                );
            }
        }

        if (b2) {
            for (y = 0; y < b2->h; y++) {
                for (x = 0; x < b2->w; x++) {
                    memcpy(
                        &result->line[y * 8][x * dest_step * 8 + 4 * dest_step],
                        &b2->line[y][x * dest_step],
                        dest_step
                    );
                }
            }
        }

        if (b3) {
            for (y = 0; y < b3->h; y++) {
                for (x = 0; x < b3->w; x++) {
                    memcpy(
                        &result->line[y * 8 + 4][x * dest_step * 4],
                        &b3->line[y][x * dest_step],
                        dest_step
                    );
                }
            }
        }

        if (b4) {
            for (y = 0; y < b4->h; y++) {
                for (x = 0; x < b4->w; x++) {
                    memcpy(
                        &result->line[y * 4][x * dest_step * 4 + 2 * dest_step],
                        &b4->line[y][x * dest_step],
                        dest_step
                    );
                }
            }
        }

        if (b5) {
            for (y = 0; y < b5->h; y++) {
                for (x = 0; x < b5->w; x++) {
                    memcpy(
                        &result->line[y * 4 + 2][x * dest_step * 2],
                        &b5->line[y][x * dest_step],
                        dest_step
                    );
                }
            }
        }

        if (b6) {
            for (y = 0; y < b6->h; y++) {
                for (x = 0; x < b6->w; x++) {
                    memcpy(
                        &result->line[y * 2][x * dest_step * 2 + 1 * dest_step],
                        &b6->line[y][x * dest_step],
                        dest_step
                    );
                }
            }
        }

        if (b7) {
            for (y = 0; y < b7->h; y++) {
                for (x = 0; x < b7->w; x++) {
                    memcpy(
                        &result->line[y * 2 + 1][x * dest_step],
                        &b7->line[y][x * dest_step],
                        dest_step
                    );
                }
            }
        }

    }

    destroy_bitmap(b1);
    destroy_bitmap(b2);
    destroy_bitmap(b3);
    destroy_bitmap(b4);
    destroy_bitmap(b5);
    destroy_bitmap(b6);
    destroy_bitmap(b7);
    
    return result;
}
