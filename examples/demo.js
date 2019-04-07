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
    ClearScreen(EGA.BLACK);
    SetFramerate(30);

    Xmax = MaxX();
    Ymax = MaxY();
    XmaxHalf = Xmax / 2;
    stepX = Xmax / 128;
    stepY = Ymax / 128;

    x = 0;
    y = Ymax;

    state = 1;
    fontCount = 0;

    fnt = new Font("jsboot/fonts/luct38.fnt");
    fntW = fnt.maxwidth;
    fntH = fnt.height;

    Println("stepX=" + stepX + ", stepY=" + stepY);
}

function Loop() {
    if (state == 1) {
        linesRed();
    } else if (state == 2) {
        linesGreen();
    } else if (state == 3) {
        linesBlue();
    } else if (state == 4) {
        fontDemo();
    }
    TextXY(10, 10, "rate=" + GetFramerate(), EGA.BLACK, EGA.LIGHT_BLUE);
}

function Input(e) { }
function linesRed() {
    Line(0, 0, x, y, new Color(x, 0, 0));
    x += stepX;
    y -= stepY;
    if (x > Xmax || y < 0) {
        state = 2;
        x = Xmax;
        y = Ymax;
    }
}

function linesGreen() {
    Line(Xmax, 0, x, y, new Color(0, x, 0));
    x -= stepX;
    y -= stepY;
    if (x < 0 || y < 0) {
        state = 3;
        x = Xmax;
    }
}

function linesBlue() {
    Line(Xmax / 2, Ymax, x, 0, new Color(0, 0, x));
    x -= stepX;
    if (x < 0) {
        state = 4;
    }
}


function fontDemo() {
    if (fontCount < 10) {
        fnt.DrawString(
            Xmax / 2, Ymax / 2 - fnt.StringHeight(""),
            "@dec_hl",
            EGA.YELLOW, NO_COLOR,
            FONT.Direction.DEFAULT,
            FONT.Align.CENTER,
            FONT.Align.TOP);
    } else if (fontCount < 20) {
        fnt.DrawString(
            Xmax / 2, Ymax / 2,
            "presents",
            EGA.YELLOW, NO_COLOR,
            FONT.Direction.DEFAULT,
            FONT.Align.CENTER,
            FONT.Align.TOP);
    } else if (fontCount < 30) {
        fnt.DrawString(
            Xmax / 2, Ymax / 2 + fnt.StringHeight(""),
            "DOjS",
            new Color(Math.random() * 255, Math.random() * 255, Math.random() * 255), NO_COLOR,
            FONT.Direction.DEFAULT,
            FONT.Align.CENTER,
            FONT.Align.TOP);
    } else if (fontCount < 31) {
        ClearScreen(EGA.BLACK);
    } else if (fontCount < 32) {
        var step = fontCount - 32;

        ClearScreen(EGA.BLACK);
        fnt.DrawString(
            Xmax / 2, Ymax / 2 + fnt.StringHeight("") - step,
            "DOjS",
            new Color(Math.random() * 255, Math.random() * 255, Math.random() * 255), NO_COLOR,
            FONT.Direction.DEFAULT,
            FONT.Align.CENTER,
            FONT.Align.TOP);
    } else {
        // Print("minw=" + fnt.minwidth + ", maxw=" + fnt.maxwidth + ", h=" + fnt.height);
        fntW--;
        fntH--;
        if (fntW == 0 || fntH == 0) {
            state = 5;
        }
        fnt.Resize(fntW, fntH);

        ClearScreen(EGA.BLACK);
        fnt.DrawString(
            Xmax / 2, Ymax / 2,
            "DOjS",
            new Color(Math.random() * 255, Math.random() * 255, Math.random() * 255), NO_COLOR,
            FONT.Direction.DEFAULT,
            FONT.Align.CENTER,
            FONT.Align.TOP);
    }
    fontCount++;
}
