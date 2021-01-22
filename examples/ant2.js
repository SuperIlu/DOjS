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

UP = 1;
DOWN = 2;
LEFT = 3;
RIGHT = 4;

var antX, antY;
var dir;

var field;

function setup() {
    dir = DOWN;
    antX = width / 2;
    antY = height / 2;

    // create playfield
    field = new Array(height);
    for (var y = 0; y < height; y++) {
        var line = new Array(width);
        for (var x = 0; x < width; x++) {
            line[x] = 0;
        }
        field[y] = line;
    }

    background(0);
    colorMode(HSB);
}

function draw() {
    var cell = field[antY][antX];
    if (cell >= 0) {
        switch (dir) {
            case LEFT:
                dir = DOWN;
                break;
            case RIGHT:
                dir = UP;
                break;
            case UP:
                dir = LEFT;
                break;
            case DOWN:
                dir = RIGHT;
                break;
        }
        cell += 4;
        stroke(cell, 255, 255);
        cell *= -1;
    } else {
        switch (dir) {
            case LEFT:
                dir = UP;
                break;
            case RIGHT:
                dir = DOWN;
                break;
            case UP:
                dir = RIGHT;
                break;
            case DOWN:
                dir = LEFT;
                break;
        }
        cell *= -1;
        cell += 4;
        stroke(0, 0, 0);
    }
    field[antY][antX] = cell;
    point(antX, antY);

    // move ant
    switch (dir) {
        case LEFT:
            if (antX > 0) {
                antX--;
            }
            break;
        case RIGHT:
            if (antX < width) {
                antX++;
            }
            break;
        case UP:
            if (antY > 0) {
                antY--;
            }
            break;
        case DOWN:
            if (antY < height) {
                antY++;
            }
            break;
    }
}
