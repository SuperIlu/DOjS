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

huffman.c -- Static Huffman tree implementation.

  This second version of implementation uses now heaptree structure.
  Note that it uses Pascal-like indexes [1..max] instead
  of C-like [0..max-1], because it simplifies heaptree
  implementation. To make it easy we just allocate max + 1
  array items and ignore the first array item.

*/

#if !defined(ALPNG_ZLIB) || (ALPNG_ZLIB == 0)

#include <stdlib.h> /* malloc, free */
#include <string.h> /* memset */

#include "input.h"
#include "huffman.h"

#define HUFFMAN_HEAP_MAX     65535
#define HUFFMAN_HEAP_SIZE    (HUFFMAN_HEAP_MAX + 1)
#define HUFFMAN_EMPTY_NODE   -1
#define HUFFMAN_NONLEAF_NODE -2

struct huffman_tree* huffman_create_tree(void) {
    struct huffman_tree* tree = (struct huffman_tree*) malloc(sizeof(struct huffman_tree));
    if (tree) {
        tree->data = malloc(HUFFMAN_HEAP_SIZE * sizeof(int));
        if (tree->data) {
            memset(tree->data, HUFFMAN_EMPTY_NODE, HUFFMAN_HEAP_SIZE * sizeof(int));
        } else {
            free(tree);
            tree = 0;
        }
    }
    return tree;
}

void huffman_reset_tree(struct huffman_tree* tree) {
    int i = 1;
    while (tree->data[1] != HUFFMAN_EMPTY_NODE) {
        if (i * 2 > HUFFMAN_HEAP_MAX) {
            tree->data[i] = HUFFMAN_EMPTY_NODE;
            i /= 2;
        } else if (tree->data[i * 2] != HUFFMAN_EMPTY_NODE) {
            i = i * 2;
        } else if (tree->data[i * 2 + 1] != HUFFMAN_EMPTY_NODE) {
            i = i * 2 + 1;
        } else {
            tree->data[i] = HUFFMAN_EMPTY_NODE;
            i /= 2;
        }
    }
    tree->last_free_parent = 1;
    tree->last_depth = 0;
    tree->data[1] = HUFFMAN_NONLEAF_NODE;
}

void huffman_destroy_tree(struct huffman_tree* tree) {
    free(tree->data);
    free(tree);
}

int huffman_add(struct huffman_tree* tree, int depth, int value) {
    int i;
    if (depth > 15 || depth <= tree->last_depth) {
        return 0;
    }
    while (1) {
        i = 2 * tree->last_free_parent;
        if (i > HUFFMAN_HEAP_MAX) {
            return 0;
        }
        if (tree->last_depth == depth - 1) {
            if (tree->data[i] == HUFFMAN_EMPTY_NODE) {
                tree->data[i] = value;
            } else {
                tree->data[i + 1] = value;
                while (tree->last_free_parent != 1 && tree->data[2 * tree->last_free_parent + 1] != HUFFMAN_EMPTY_NODE) {
                    tree->data[tree->last_free_parent] = HUFFMAN_NONLEAF_NODE;
                    tree->last_free_parent /= 2;
                    tree->last_depth--;
                }
            }
            return 1;
        } else {
            tree->last_depth++;
            if (tree->data[i] == HUFFMAN_EMPTY_NODE) {
                tree->last_free_parent = i;
            } else {
                tree->last_free_parent = i + 1;
            }
        }
    }
}

int huffman_read_next_code(struct huffman_tree* tree, struct input_data* data) {
    int i = 1;
    int bit;

    while (tree->data[i] < 0) {
        bit = input_read_bit(data);
        if (data->error) {
            return HUFFMAN_EMPTY_NODE;
        }
        switch (bit) {
            case 0: i = i * 2; break;
            case 1: i = i * 2 + 1; break;
        }
        if (i > HUFFMAN_HEAP_MAX) {
            return -1;
        }
    }

    return tree->data[i];
}

#endif /* !defined(ALPNG_ZLIB) || (ALPNG_ZLIB == 0) */
