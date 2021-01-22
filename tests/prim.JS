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

/*
** This function is called once when the script is started.
*/
function Setup() {
    SetFramerate(30);
}

/*
** This function is repeatedly until ESC is pressed or Stop() is called.
*/
function Loop() {
    ClearScreen(EGA.BLACK);

    // test lines
    Line(10, 10, 100, 10, EGA.RED);
    CustomLine(110, 10, 200, 10, 5, EGA.RED);

    // test boxes
    Box(10, 20, 100, 100, EGA.GREEN);
    FilledBox(110, 20, 200, 100, EGA.GREEN);

    // test circles
    Circle(300, 60, 40, EGA.BLUE);
    FilledCircle(400, 60, 40, EGA.BLUE);
    CustomCircle(500, 60, 40, 5, EGA.BLUE);

    // test ellipses
    Ellipse(300, 160, 40, 20, EGA.CYAN);
    FilledEllipse(400, 160, 40, 20, EGA.CYAN);
    CustomEllipse(500, 160, 40, 20, 5, EGA.CYAN);

    // try fill
    Box(10, 110, 100, 200, EGA.MAGENTA);
    FloodFill(15, 115, EGA.WHITE);

    var arc = CircleArc(110, 140, 40, 0, 32, EGA.BROWN);
    Line(arc.centerX, arc.centerY, arc.startX, arc.startY, EGA.BROWN);
    Line(arc.centerX, arc.centerY, arc.endX, arc.endY, EGA.BROWN);

    var arc = CustomCircleArc(130, 200, 40, 32, 64, 5, EGA.BROWN);
    Line(arc.centerX, arc.centerY, arc.startX, arc.startY, EGA.BROWN);
    Line(arc.centerX, arc.centerY, arc.endX, arc.endY, EGA.BROWN);

    // basic text
    TextXY(10, 400, "Hello World!", EGA.WHITE, NO_COLOR);

}

/*
** This function is called on any input.
*/
function Input(e) {
    if (CompareKey(e.key, 's')) {
        SaveBmpImage("tests/scr.bmp");
        SavePcxImage("tests/scr.pcx");
        SaveTgaImage("tests/scr.tga");
    }
}
