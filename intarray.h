/*
MIT License

Copyright (c) 2019-2021 Andre Seidelt <superilu@yahoo.com>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:
The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#ifndef __INTARRAY_H__
#define __INTARRAY_H__

#include <mujs.h>
#include <stdbool.h>
#include <stdint.h>

/************
** defines **
************/
#define TAG_INT_ARRAY "IntArray"  //!< class name for IntArray()
#define IA_TYPE int32_t

typedef struct {
    uint32_t alloc_size;
    uint32_t size;
    IA_TYPE *data;
} int_array_t;

/*********************
** static functions **
*********************/
extern void init_intarray(js_State *J);

extern void IntArray_fromBytes(js_State *J, uint8_t *data, uint32_t size);
extern int_array_t *IntArray_create(void);
extern int IntArray_push(int_array_t *ia, IA_TYPE val);
extern void IntArray_destroy(int_array_t *ia);
extern void IntArray_fromStruct(js_State *J, int_array_t *ia);

#endif  // __INTARRAY_H__
