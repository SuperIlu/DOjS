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

inflate.c -- Main inflate algorithm.

*/

#if !defined(ALPNG_ZLIB) || (ALPNG_ZLIB == 0)

#include <string.h> /* memset */

#include "input.h"
#include "inflate.h"
#include "huffman.h"

#ifndef EOF
    #define EOF (-1)
#endif

/* reads distance information from the input using specified huffman tree.
   validates the distance to be >=1 and <=32768
   returns -1 on error */
static int get_distance(struct huffman_tree* distances_tree, struct input_data* data);

/* create huffman tree for fixed codes; returns 0 on error; 1 otherwise */
static int load_fixed_tree(struct huffman_tree* main_tree, struct huffman_tree* distances_tree);

/* create huffman tree for dynamic codes; returns 0 on error; 1 otherwise */
static int load_dynamic_tree(struct huffman_tree* main_tree, struct huffman_tree* distances_tree, struct huffman_tree* helper_tree, struct input_data* data);

unsigned int alpng_inflate(struct input_data* data, unsigned char* unpacked_data, unsigned int unpacked_length, char** error_msg) {
    int bfinal, btype; /* as defined by the deflate spec */
    int code, length, distance, i;
    unsigned int unpacked_index = 0;
    struct huffman_tree* main_tree = huffman_create_tree();
    struct huffman_tree* distances_tree = huffman_create_tree();
    struct huffman_tree* helper_tree = huffman_create_tree();  /* for codelengths */

    *error_msg = "OK";
    
    data->i = 2; /* ignore zlib header */

    do {
        bfinal = input_read_bit(data);
        if (data->error) {
            *error_msg = "Data shorter than one bit!";
            huffman_destroy_tree(main_tree);
            huffman_destroy_tree(distances_tree);
            huffman_destroy_tree(helper_tree);
            return 0;
        }
        btype = input_read_bits(data, 2);
        if (data->error) {
            *error_msg = "Data shorter than one byte!";
            huffman_destroy_tree(main_tree);
            huffman_destroy_tree(distances_tree);
            huffman_destroy_tree(helper_tree);
            return 0;
        }
        switch (btype) {
            case 0:
                length = input_read_bytes(data, 2);
                if (data->error) {
                    *error_msg = "Cannot read length of uncompressed block!";
                    huffman_destroy_tree(main_tree);
                    huffman_destroy_tree(distances_tree);
                    huffman_destroy_tree(helper_tree);
                    return 0;
                }
                i = input_read_bytes(data, 2);
                if (data->error) {
                    *error_msg = "Cannot read length of uncompressed block!";
                    huffman_destroy_tree(main_tree);
                    huffman_destroy_tree(distances_tree);
                    huffman_destroy_tree(helper_tree);
                    return 0;
                }
                if (length != (i ^ 0xFFFF)) {
                    *error_msg = "Uncompressed block corruption detected!";
                    huffman_destroy_tree(main_tree);
                    huffman_destroy_tree(distances_tree);
                    huffman_destroy_tree(helper_tree);
                    return 0;
                }
                for (i = 0; i < length; i++) {
                    code = input_read_byte(data);
                    if (data->error) {
                        *error_msg = "Uncompressed block incomplete!";
                        huffman_destroy_tree(main_tree);
                        huffman_destroy_tree(distances_tree);
                        huffman_destroy_tree(helper_tree);
                        return 0;
                    }
                    if (unpacked_index >= unpacked_length) {
                        *error_msg = "Too much data!";
                        huffman_destroy_tree(main_tree);
                        huffman_destroy_tree(distances_tree);
                        huffman_destroy_tree(helper_tree);
                        return 0;
                    } else {
                        unpacked_data[unpacked_index++] = code;
                    }
                }
                break;
            case 1:
                if (!load_fixed_tree(main_tree, distances_tree)) {
                    *error_msg = "Cannot load fixed Huffman tree!";
                    huffman_destroy_tree(main_tree);
                    huffman_destroy_tree(distances_tree);
                    huffman_destroy_tree(helper_tree);
                    return 0;
                }
                break;
            case 2:
                if (!load_dynamic_tree(main_tree, distances_tree, helper_tree, data)) {
                    *error_msg = "Cannot load dynamic Huffman tree!";
                    huffman_destroy_tree(main_tree);
                    huffman_destroy_tree(distances_tree);
                    huffman_destroy_tree(helper_tree);
                    return 0;
                }
                break;
            case 3:
                *error_msg = "Unsupported (non-existing) type of compression!";
                huffman_destroy_tree(main_tree);
                huffman_destroy_tree(distances_tree);
                huffman_destroy_tree(helper_tree);
                return 0;
                break;
        }
        if (btype > 0) {
            code = huffman_read_next_code(main_tree, data);
            if (code == -1) {
                huffman_destroy_tree(main_tree);
                huffman_destroy_tree(distances_tree);
                huffman_destroy_tree(helper_tree);
                *error_msg = "Invalid data!";
                return 0;
            }
            while (code != 256) {
                if (code < 256) {
                    if (unpacked_index >= unpacked_length) {
                        huffman_destroy_tree(main_tree);
                        huffman_destroy_tree(distances_tree);
                        huffman_destroy_tree(helper_tree);
                        *error_msg = "Too much data!";
                        return 0;
                    }
                    unpacked_data[unpacked_index++] = code;
                } else {
                    length = 0;
                    distance = 0;
                    if (code >= 257 && code <= 264) {
                        length = code - 257 + 3;
                    } else if (code >= 265 && code <= 268) {
                        length = (code - 265) * 2 + 11 + input_read_bit(data);
                    } else if (code >= 269 && code <= 272) {
                        length = (code - 269) * 4 + 19 + input_read_bits(data, 2);
                    } else if (code >= 273 && code <= 276) {
                        length = (code - 273) * 8 + 35 + input_read_bits(data, 3);
                    } else if (code >= 277 && code <= 280) {
                        length = (code - 277) * 16 + 67 + input_read_bits(data, 4);
                    } else if (code >= 281 && code <= 284) {
                        length = (code - 281) * 32 + 131 + input_read_bits(data, 5);
                    } else if (code == 285) {
                        length = 258;
                    } else {
                        huffman_destroy_tree(main_tree);
                        huffman_destroy_tree(distances_tree);
                        huffman_destroy_tree(helper_tree);
                        *error_msg = "Algorithm error: unknown length!";
                        return 0;
                    }
                    if (data->error) {
                        huffman_destroy_tree(main_tree);
                        huffman_destroy_tree(distances_tree);
                        huffman_destroy_tree(helper_tree);
                        *error_msg = "Cannot read length!";
                        return 0;
                    }

                    distance = get_distance(distances_tree, data);
                    if (distance == -1) {
                        huffman_destroy_tree(main_tree);
                        huffman_destroy_tree(distances_tree);
                        huffman_destroy_tree(helper_tree);
                        *error_msg = "Algorithm error: bad distance!";
                        return 0;
                    }

                    for (i = 0; i < length; i++) {
                        code = unpacked_data[unpacked_index - distance];
                        if (unpacked_index >= unpacked_length) {
                            huffman_destroy_tree(main_tree);
                            huffman_destroy_tree(distances_tree);
                            huffman_destroy_tree(helper_tree);
                            *error_msg = "Too much data!";
                            return 0;
                        }
                        unpacked_data[unpacked_index++] = code;
                    }
                }
                code = huffman_read_next_code(main_tree, data);
                if (code == -1) {
                    huffman_destroy_tree(main_tree);
                    huffman_destroy_tree(distances_tree);
                    huffman_destroy_tree(helper_tree);
                    *error_msg = "Invalid data!";
                    return 0;
                }
            }
        }
    } while (bfinal == 0);
    huffman_destroy_tree(main_tree);
    huffman_destroy_tree(distances_tree);
    huffman_destroy_tree(helper_tree);
    return unpacked_index;
}

static int load_dynamic_tree(struct huffman_tree* main_tree, struct huffman_tree* distances_tree, struct huffman_tree* helper_tree, struct input_data* data) {
    int hlit, hdist, hclen; /* see deflate spec for details */
    int code_length_codes[19];
    int literal_length_codes[286];
    int distance_codes[32];
    int i, j, k, l, m;  /* counters */
    int code_lengths_order[] = { 16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15 };

    hlit = input_read_bits(data, 5) + 257;
    if (data->error) {
        return 0;
    }
    hdist = input_read_bits(data, 5) + 1;
    if (data->error) {
        return 0;
    }
    hclen = input_read_bits(data, 4) + 4;
    if (data->error) {
        return 0;
    }

    memset(code_length_codes, 0, 19 * sizeof(int));
    for (i = 0; i < hclen; i++) {
        code_length_codes[code_lengths_order[i]] = input_read_bits(data, 3);
        if (data->error) {
            return 0;
        }
    }

    huffman_reset_tree(helper_tree);
    for (i = 1; i < 2*2*2; i++) {
        for (j = 0; j < 19; j++) {
            if (code_length_codes[j] == i) {
                if (!huffman_add(helper_tree, i, j)) {
                    return 0;
                }
            }
        }
    }

    memset(literal_length_codes, 0, 286 * sizeof(int));
    i = 0;
    while (i < hlit) {
        j = huffman_read_next_code(helper_tree, data);
        if (j == -1) {
            return 0;
        }
        if (j <= 15) {
            literal_length_codes[i++] = j;
        } else {
            switch (j) {
            case 16: {
                k = input_read_bits(data, 2) + 3;
                if (data->error) {
                    return 0;
                }
                l = literal_length_codes[i - 1];
                for (m = 0; m < k; m++) {
                    literal_length_codes[i++] = l;
                }
                break; }
            case 17: {
                k = input_read_bits(data, 3) + 3;
                if (data->error) {
                    return 0;
                }
                for (m = 0; m < k; m++) {
                    literal_length_codes[i++] = 0;
                }
                break; }
            case 18: {
                k = input_read_bits(data, 7) + 11;
                if (data->error) {
                    return 0;
                }
                for (m = 0; m < k; m++) {
                    literal_length_codes[i++] = 0;
                }
                break; }
            }
        }
    }
    huffman_reset_tree(main_tree);
    for (i = 1; i <= 15; i++) {
        for (j = 0; j < 286; j++) {
            if (literal_length_codes[j] == i) {
                if (!huffman_add(main_tree, i, j)) {
                    return 0;
                }
            }
        }
    }

    memset(distance_codes, 0, 32 * sizeof(int));
    i = 0;
    while (i < hdist) {
        j = huffman_read_next_code(helper_tree, data);
        if (j == -1) {
            return 0;
        }
        if (j <= 15) {
            distance_codes[i++] = j;
        } else {
            switch (j) {
            case 16: {
                k = input_read_bits(data, 2) + 3;
                if (data->error) {
                    return 0;
                }
                l = distance_codes[i - 1];
                for (m = 0; m < k; m++) {
                    distance_codes[i++] = l;
                }
                break; }
            case 17: {
                k = input_read_bits(data, 3) + 3;
                if (data->error) {
                    return 0;
                }
                for (m = 0; m < k; m++) {
                    distance_codes[i++] = 0;
                }
                break; }
            case 18: {
                k = input_read_bits(data, 7) + 11;
                if (data->error) {
                    return 0;
                }
                for (m = 0; m < k; m++) {
                    distance_codes[i++] = 0;
                }
                break; }
            }
        }
    }
    huffman_reset_tree(distances_tree);
    for (i = 1; i <= 15; i++) {
        for (j = 0; j < 32; j++) {
            if (distance_codes[j] == i) {
                if (!huffman_add(distances_tree, i, j)) {
                    return 0;
                }
            }
        }
    }

    return 1;
}

static int load_fixed_tree(struct huffman_tree* main_tree, struct huffman_tree* distances_tree) {
    int i;
    huffman_reset_tree(main_tree);
    for (i = 256; i <= 279; i++) {
        if (!huffman_add(main_tree, 7, i)) {
            return 0;
        }
    }
    for (i = 0; i <= 143; i++) {
        if (!huffman_add(main_tree, 8, i)) {
            return 0;
        }
    }
    for (i = 280; i <= 287; i++) {
        if (!huffman_add(main_tree, 8, i)) {
            return 0;
        }
    }
    for (i = 144; i <= 255; i++) {
        if (!huffman_add(main_tree, 9, i)) {
            return 0;
        }
    }
    huffman_reset_tree(distances_tree);
    for (i = 0; i < 32; i++) {
        if (!huffman_add(distances_tree, 5, i)) {
            return 0;
        }
    }
    return 1;
}

/* see deflate spec for description */
static int extra_bits[] = { 0,0,0,0,1,1,2, 2, 3, 3, 4, 4, 5, 5,  6,  6,  7,  7,  8,  8,   9,   9,  10,  10,  11,  11,  12,   12,   13,   13 };
static int distances[]  = { 1,2,3,4,5,7,9,13,17,25,33,49,65,97,129,193,257,385,513,769,1025,1537,2049,3073,4097,6145,8193,12289,16385,24577 };
static int get_distance(struct huffman_tree* distances_tree, struct input_data* data) {
    int distance;
    int i;
    i = huffman_read_next_code(distances_tree, data);
    if (i == -1) {
        return -1;
    }
    if (i >= (int)(sizeof(distances) / sizeof(int))) {
        return -1;
    }
    distance = distances[i];
    if (extra_bits[i] > 0) {
        distance += input_read_bits(data, extra_bits[i]);
        if (data->error) {
            return -1;
        }
    }
    if (distance < 1 || distance > 32768) {
        return -1;
    }
    return distance;
}

#endif /* !defined(ALPNG_ZLIB) || (ALPNG_ZLIB == 0) */
