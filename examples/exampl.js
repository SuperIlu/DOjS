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

/*
** This function is called once when the script is started.
*/
function Setup() {
    pink = Color(241, 66, 244, 255); // define the color pink
}

/*
** This function is repeatedly until ESC is pressed or Stop() is called.
*/
function Loop() {
    ClearScreen(EGA.BLACK);
    TextXY(SizeX() / 2, SizeY() / 2, "Hello World!", pink, NO_COLOR);

    TextXY(10, 10, "rate=" + GetFramerate(), EGA.BLACK, EGA.LIGHT_BLUE);
}

/*
** This function is called on any input.
*/
function Input(event) {
    str = JSON.stringify(event);
}
