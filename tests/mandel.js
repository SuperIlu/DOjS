/**
The original source was taken from the Duktape JS engine:
https://duktape.org/

It was modified to run with DOjS by Andre Seidelt <superilu@yahoo.com>.

===============
Duktape license
===============

(http://opensource.org/licenses/MIT)

Copyright (c) 2013-2018 by Duktape authors (see AUTHORS.rst)

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/

w = 0;
h = 0;
i = 0;
j = 0;

x0 = y0 = 0;

/*
** This function is called once when the script is started.
*/
function Setup() {
    w = SizeX();
    h = SizeY();

    i = 0;
    j = 0;

    SetFramerate(3000);
}

/*
** This function is repeatedly until ESC is pressed or Stop() is called.
*/
function Loop() {
    var iter = 100;
    var k, c;
    var xx, yy, xx2, yy2;

    for (k = 0, xx = 0, yy = 0, c = EGA.CYAN; k < iter; k++) {
        /* z -> z^2 + c
         *   -> (xx+i*yy)^2 + (x0+i*y0)
         *   -> xx*xx+i*2*xx*yy-yy*yy + x0 + i*y0
         *   -> (xx*xx - yy*yy + x0) + i*(2*xx*yy + y0)
         */

        xx2 = xx * xx; yy2 = yy * yy;

        if (xx2 + yy2 < 4.0) {
            yy = 2 * xx * yy + y0;
            xx = xx2 - yy2 + x0;
        } else {
            /* xx^2 + yy^2 >= 4.0 */
            if (k < 3) { c = EGA.GREEN; }
            else if (k < 5) { c = EGA.YELLOW; }
            else if (k < 10) { c = EGA.RED; }
            else { c = EGA.MAGENTA; }
            break;
        }
    }

    Plot(j, i, c);

    j += 1;
    x0 = (j / w) * 3.0 - 2.0;
    if (j >= w) {
        j = 0;
        i += 1;
        y0 = (i / h) * 2.5 - 1.25;
    }
    if (i >= h) {
        Stop();
    }
}

/*
** This function is called on any input.
*/
function Input(event) {
}
