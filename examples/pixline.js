/*
MIT License

Copyright (c) 2019-2020 Andre Seidelt <superilu@yahoo.com>

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

/*
** This function is called once when the script is started.
*/
function setup() {
    img = new Bitmap("examples/3dfx.tga");
    SetFramerate(400);

    xPos = width + 1
    yPos = height + 1;
    dir = -1;
}

/*
** This function is repeatedly until ESC is pressed or Stop() is called.
*/
function draw() {
    if (dir == -1) {
        dir = int(random(20)) % 2;

        if (dir == 0) {
            xPos = random(4);
            yPos = random(0, height);
        } else {
            xPos = random(0, width);
            yPos = random(4);
        }
    } else {
        if (dir == 0) {
            if (xPos > width) {
                dir = -1;
            } else {
                xPos += 4;
            }
        } else {
            if (yPos > height) {
                dir = -1;
            } else {
                yPos += 4;
            }
        }

        var s = random(1, 4);
        var c = img.GetPixel(xPos, yPos);
        FilledCircle(xPos, yPos, s, c);
    }
}
