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

alpng_internal.h -- Internally used structures and functions.

*/

#ifndef ALPNG_INTERNAL_H
#define ALPNG_INTERNAL_H

extern unsigned char ALPNG_PNG_HEADER[];
extern int ALPNG_PNG_HEADER_LEN;

struct alpng_chunk {
    uint32_t type;
    uint32_t length;
};

struct alpng_header {
    uint32_t width, height, byte_width;
    uint8_t depth, type, interlace;
    RGB* pal;
    int has_palette;
};

struct alpng_fake_headers {
    struct alpng_header h[7];
    uint32_t fake_raw_data_length[7];
};

BITMAP* alpng_draw(struct alpng_header* header, uint8_t* data, uint32_t data_length);
BITMAP* alpng_draw_interlaced(struct alpng_header* header, uint8_t* data, struct alpng_fake_headers* fake_headers);
void alpng_fill_fake_headers(struct alpng_header* header, struct alpng_fake_headers* fake_headers);
uint32_t alpng_calc_raw_data_length(struct alpng_header* header);
int alpng_unfilter(uint8_t* data, struct alpng_header* header);
uint8_t* alpng_filter(uint8_t* data, uint32_t w, uint32_t h, uint32_t filter_delta);

#endif  /* ALPNG_INTERNAL_H */
