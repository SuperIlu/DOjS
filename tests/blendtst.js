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

Include('p5');

LoadLibrary("jpeg");

modestr = [
    "REPLACE     / C := A",
    "ALPHA       / C := (A-B)*alpha + B",
    "ADD         / C := A + B",
    "DARKEST     / C := min(A, B)",
    "LIGHTEST    / C := max(A, B)",
    "DIFFERENCE  / C := abs(A - B)",
    "EXCLUSION   / C := A+B - 2*A*B",
    "MULTIPLY    / C := A*B",
    "SCREEN      / C := 1 - (1-A)*(1-B)",
    "OVERLAY     / C := A<0.5 ? (2*A*B):(1-2*(1-A)*(1-B))",
    "HARD_LIGHT  / C := B<0.5 ? (2*A*B):(1-2*(1-A)*(1-B))",
    "DOGE        / C := (A==1)?A:min(B/(1-A),1)",
    "BURN        / C := (A==0)?A:max((1-((1-B)/a)),0)",
    "SUBSTRACT   / C := A+B-255",
];

function setup() {
    createCanvas(windowWidth, windowHeight);
    img = new Bitmap("tests/rose.jpg");
    bmode = 0;
    alpha = 128;
    frameRate(1);
}

function draw() {
    TransparencyEnabled(BLEND.REPLACE);
    background(64);
    img.Draw(0, 0);
    TextXY(10, 10, "mode=" + modestr[bmode] + " " + alpha, EGA.WHITE, NO_COLOR);


    var x = 0;
    var y = 0;

    TransparencyEnabled(bmode);
    stroke(128);
    fill(255, 0, 0, alpha);
    circle(x + img.width / 2, y + img.width / 3, img.width / 2);
    fill(0, 255, 0, alpha);
    circle(x + img.width / 3, y + (img.width / 3) * 2, img.width / 2);
    fill(0, 0, 255, alpha);
    circle(x + (img.width / 3) * 2, y + (img.width / 3) * 2, img.width / 2);
}

function keyPressed() {
    var key = keyCode >> 8;
    switch (key) {
        case KEY.Code.KEY_DOWN:
            alpha = 128;
            break;
        case KEY.Code.KEY_UP:
            alpha = 255;
            break;
        case KEY.Code.KEY_RIGHT:
        case KEY.Code.KEY_PGDN:
            if (bmode < modestr.length) {
                bmode++;
            }
            break;
        case KEY.Code.KEY_LEFT:
        case KEY.Code.KEY_PGUP:
            if (bmode > 0) {
                bmode--;
            }
            break;
    }
}
