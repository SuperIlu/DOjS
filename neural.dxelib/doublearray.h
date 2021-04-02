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

#ifndef __DOUBLEARRAY_H__
#define __DOUBLEARRAY_H__

#include <mujs.h>
#include <stdbool.h>
#include <stdint.h>

/************
** defines **
************/
#define TAG_DOUBLE_ARRAY "DoubleArray"  //!< class name for DoubleArray()
#define DA_TYPE double

typedef struct {
    uint32_t alloc_size;
    uint32_t size;
    DA_TYPE *data;
} double_array_t;

/*********************
** static functions **
*********************/
extern void init_doublearray(js_State *J);

extern void DoubleArray_fromDouble(js_State *J, DA_TYPE *data, uint32_t size);
extern double_array_t *DoubleArray_create(void);
extern int DoubleArray_push(double_array_t *da, DA_TYPE val);
extern void DoubleArray_destroy(double_array_t *da);
extern void DoubleArray_fromStruct(js_State *J, double_array_t *da);

#endif  // __DOUBLEARRAY_H__
