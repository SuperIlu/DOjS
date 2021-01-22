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

huffman.h -- Static Huffman tree implementation.

*/

#if !defined(ALPNG_ZLIB) || (ALPNG_ZLIB == 0)

#ifndef HUFFMAN_H
#define HUFFMAN_H

#include "input.h"

struct huffman_tree {
    int* data;
    int last_free_parent;
    int last_depth;
};

/* clears tree attributes */
extern void huffman_reset_tree(struct huffman_tree* tree);

/* allocates the tree */
extern struct huffman_tree* huffman_create_tree(void);

/* frees the tree */
extern void huffman_destroy_tree(struct huffman_tree* tree);

/* adds value to the tree to the depth; returns 0 on error, 1 on success */
extern int huffman_add(struct huffman_tree* tree, int depth, int value);

/* reads next value from tree; returns -1 on error */
extern int huffman_read_next_code(struct huffman_tree* tree, struct input_data* data);

#endif /* HUFFMAN_H */

#endif /* !defined(ALPNG_ZLIB) || (ALPNG_ZLIB == 0) */
