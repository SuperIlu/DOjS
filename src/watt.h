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

#ifndef __WATT_H__
#define __WATT_H__

#include <tcp.h>
#include "DOjS.h"

/************
** defines **
************/
#define IP1(addr) ((uint8_t)((addr >> 24) & 0xFF))
#define IP2(addr) ((uint8_t)((addr >> 16) & 0xFF))
#define IP3(addr) ((uint8_t)((addr >> 8) & 0xFF))
#define IP4(addr) ((uint8_t)(addr & 0xFF))

#define IP(a, b, c, d) ((((DWORD)a & 0xFF) << 24) | (((DWORD)b & 0xFF) << 16) | (((DWORD)c & 0xFF) << 8) | ((DWORD)d & 0xFF))

/***********************
** exported functions **
***********************/
extern void init_watt(js_State *J);
void watt_pushipaddr(js_State *J, DWORD ip);
DWORD watt_toipaddr(js_State *J, int idx);

#endif  // __WATT_H__
