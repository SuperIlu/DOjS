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

original_zlib.c -- Wrapper around original zlib implementation.
                   You can get classic zlib at http://zlib.net/ .

*/

#if defined(ALPNG_ZLIB) && (ALPNG_ZLIB == 1)

#include <allegro.h>
#include <zlib.h>
#include "../inflate/inflate.h"
#include "deflate.h"

/* Unpacks data. Returns 0 on error, unpacked data length otherwise.
   error_msg is assigned to the text of error message (text "OK" on success).
 */
unsigned int alpng_inflate(struct input_data* data, unsigned char* unpacked_array, unsigned int unpacked_length, char** error_msg) {
    uLongf real_length = unpacked_length;
    int res = uncompress(unpacked_array, &real_length, data->data, data->length);
    if (res != Z_OK) {
        *error_msg = "Cannot decompress data!";
        return 0;
    }
    return real_length;
}

int alpng_deflate(uint8_t* data, uint32_t data_length, uint8_t** compressed_data, uint32_t* compressed_data_length, char** error_msg) {
    int res;
    uLongf dest_length = compressBound(data_length);
    *compressed_data = (uint8_t*) malloc(dest_length);
    if (!*compressed_data) {
        *error_msg = "Cannot allocate memory!";
        return 0;
    }
    res = compress2(*compressed_data, &dest_length, data, data_length, 9);
    if (res != Z_OK) {
        *error_msg = "Error during compression!";
        free(*compressed_data);
        return 0;
    }
    *compressed_data_length = dest_length;
    return 1;
}

#endif /* defined(ALPNG_ZLIB) && (ALPNG_ZLIB == 1) */
