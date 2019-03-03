/*
MIT License

Copyright (c) 2019 Andre Seidelt <superilu@yahoo.com>

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

#ifndef __UTIL_H__
#define __UTIL_H__

#include <mujs.h>
#include "DOjS.h"

/*************
** typedefs **
*************/
//! return values of ut_check_file()
typedef enum { CF_YES, CF_NO, CF_ERROR } check_file_t;

/***********************
** exported functions **
***********************/
extern const char *ut_getModeString();
extern const char *ut_getAdapterString();
extern check_file_t ut_check_file(char *fname);
extern bool ut_endsWith(const char *str, const char *suffix);
extern const char *ut_getFrameModeString(GrFrameMode fm);
extern void ut_dumpVideoModes();

#endif  // __UTIL_H__
