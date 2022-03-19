/*
Inflate implementation

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

input.c -- Data input handling.

*/

#if !defined(ALPNG_ZLIB) || (ALPNG_ZLIB == 0)

#include "input.h"

int input_read_bit(struct input_data* data) {
    int res;
    if (data->si > 7) {
        if (data->i == data->length) {
            data->error = 1;
            return -1;
        }
        data->current = data->data[data->i++];
        data->si = 0;
    }
    res = (data->current >> data->si) & 1;
    data->si++;
    return res;
}

int input_read_bits(struct input_data* data, int count) {
    int i, res = 0;
    for (i = 0; i < count; i++) {
        res += (input_read_bit(data) << i);
        if (data->error) {
            return -1;
        }
    }
    return res;
}

unsigned char input_read_byte(struct input_data* data) {
    if (data->i == data->length) {
        data->error = 1;
        return -1;
    }
    if (data->si != 8) {
        data->si = 8;
    }
    data->current = data->data[data->i++];
    return data->current;
}

unsigned long input_read_bytes(struct input_data* data, int count) {
    int i;
    unsigned long res = 0;
    for (i = 0; i < count; i++) {
        res += (input_read_byte(data) << (i * 8));
        if (data->error) {
            return -1;
        }
    }
    return res;
}

#endif /*!defined(ALPNG_ZLIB) || (ALPNG_ZLIB == 0) */
