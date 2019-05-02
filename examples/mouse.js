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

function Setup() {
    if (!MOUSE_AVAILABLE) {
        print("Mouse not found");
        Quit(1);
    }

    MouseShowCursor(true);
    ClearScreen(EGA.DARK_GRAY);

    draw = false;
    SetFramerate(50);

    mouseX = 0;
    mouseY = 0;
}

function Loop() {
    if (draw) {
        Plot(mouseX, mouseY, EGA.WHITE);
    }

    FilledBox(10, 9, 70, 20, EGA.WHITE);
    TextXY(10, 10, "rate=" + GetFramerate(), EGA.BLACK, EGA.WHITE);
}

function Input(event) {
    if (event.buttons & MOUSE.Buttons.LEFT) {
        draw = true;

    } else {
        draw = false;
    }
    mouseX = event.x;
    mouseY = event.y;
}
