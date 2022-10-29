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

#ifndef __LINES_H__
#define __LINES_H__

#include <stdbool.h>
#include "edit.h"

/*********************
** static functions **
*********************/
extern edi_t* lin_init(const char* name);
extern void lin_shutdown(edi_t* edi);
extern line_t* lin_newline(void);
extern void lin_freeline(line_t* l);
extern void lin_insertline(edi_t* edi, line_t* pred, line_t* l);
extern void lin_removeline(edi_t* edi, line_t* l);
extern void lin_appendch(edi_t* edi, line_t* l, char ch);
extern void lin_insertch(edi_t* edi, line_t* l, unsigned int x, char ch);
extern void lin_joinprev(edi_t* edi, line_t* l);
extern void lin_joinnext(edi_t* edi, line_t* l);
extern void lin_splitline(edi_t* edi, line_t* l, unsigned int x);
extern void lin_delch_right(edi_t* edi, line_t* l, unsigned int x);
extern void lin_delch_left(edi_t* edi, line_t* l, unsigned int x);
extern line_t* lin_find(edi_t* edi, int num);

#endif  // __LINES_H__
