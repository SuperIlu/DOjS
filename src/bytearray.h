/*
MIT License

Copyright (c) 2019-2022 Andre Seidelt <superilu@yahoo.com>

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

#ifndef __BYTEARRAY_H__
#define __BYTEARRAY_H__

#include <mujs.h>
#include <stdbool.h>
#include <stdint.h>

/************
** defines **
************/
#define TAG_BYTE_ARRAY "ByteArray"  //!< class name for ByteArray()
#define BA_TYPE uint8_t

typedef struct {
    uint32_t alloc_size;
    uint32_t size;
    BA_TYPE *data;
} byte_array_t;

/*********************
** static functions **
*********************/
extern void init_bytearray(js_State *J);

extern void ByteArray_fromBytes(js_State *J, const uint8_t *data, uint32_t size);
extern byte_array_t *ByteArray_create(void);
extern int ByteArray_push(byte_array_t *ba, BA_TYPE val);
extern void ByteArray_destroy(byte_array_t *ba);
extern void ByteArray_fromStruct(js_State *J, byte_array_t *ba);

#endif  // __BYTEARRAY_H__
