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

var it = 0;
var circSize = 32;
var circDir = 1;

/*
** This function is called once when the script is started.
*/
function Setup() {
    SetFramerate(3);
}

/*
** This function is repeatedly until ESC is pressed or Stop() is called.
*/
function Loop() {
    // dynamically generate bitmap
    circSize += circDir;
    if (circSize > 64) {
        circDir = -1;
    } else if (circSize < 16) {
        circDir = 1;
    }
    var bm = new Bitmap(256, 256);
    Debug(it + ": " + bm.width + "x" + bm.height);
    SetRenderBitmap(bm);
    ClearScreen(EGA.BLACK);
    CustomLine(0, 0, SizeX() - 1, SizeY() - 1, 5, EGA.RED);
    CustomLine(0, SizeY() - 1, SizeX() - 1, 0, 5, EGA.GREEN);
    Circle(SizeX() / 2, SizeY() / 2, circSize, EGA.YELLOW);
    SetRenderBitmap(null);

    ClearScreen(EGA.BLACK);

    bm.Draw(0, 0);

    it++;
}

/*
** This function is called on any input.
*/
function Input(e) {
}
