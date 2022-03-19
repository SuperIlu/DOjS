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

#ifndef __BLENDER_H__
#define __BLENDER_H__

#include "DOjS.h"

/***********************
** exported functions **
***********************/
unsigned long blender_alpha(unsigned long src, unsigned long dest, unsigned long n);
unsigned long blender_add(unsigned long src, unsigned long dest, unsigned long n);
unsigned long blender_darkest(unsigned long src, unsigned long dest, unsigned long n);
unsigned long blender_lightest(unsigned long src, unsigned long dest, unsigned long n);
unsigned long blender_difference(unsigned long src, unsigned long dest, unsigned long n);
unsigned long blender_exclusion(unsigned long src, unsigned long dest, unsigned long n);
unsigned long blender_multiply(unsigned long src, unsigned long dest, unsigned long n);
unsigned long blender_screen(unsigned long src, unsigned long dest, unsigned long n);
unsigned long blender_overlay(unsigned long src, unsigned long dest, unsigned long n);
unsigned long blender_hardlight(unsigned long src, unsigned long dest, unsigned long n);
unsigned long blender_doge(unsigned long src, unsigned long dest, unsigned long n);
unsigned long blender_burn(unsigned long src, unsigned long dest, unsigned long n);
unsigned long blender_substract(unsigned long src, unsigned long dest, unsigned long n);

#endif  // __BLENDER_H__
