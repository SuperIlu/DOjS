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

/**
 * Color flags.
 * @namespace COLOR
 * @property {*} XOR XOR the color onto the screen.
 * @property {*} OR OR the color onto the screen.
 * @property {*} AND AND the color onto the screen.
 */
COLOR = {
        XOR: 1,
        OR: 2,
        AND: 3
};

/**
 * EGA Color definition.
 * @namespace EGA
 * @property {Color} BLACK EGA color.
 * @property {Color} BLUE EGA color.
 * @property {Color} GREEN EGA color.
 * @property {Color} CYAN EGA color.
 * @property {Color} RED EGA color.
 * @property {Color} MAGENTA EGA color.
 * @property {Color} BROWN EGA color.
 * @property {Color} LIGHT_GRAY EGA color.
 * @property {Color} LIGHT_GREY EGA color.
 * @property {Color} DARK_GRAY EGA color.
 * @property {Color} DARK_GREY EGA color.
 * @property {Color} LIGHT_BLUE EGA color.
 * @property {Color} LIGHT_GREEN EGA color.
 * @property {Color} LIGHT_CYAN EGA color.
 * @property {Color} LIGHT_RED EGA color.
 * @property {Color} LIGHT_MAGENTA EGA color.
 * @property {Color} YELLOW EGA color.
 * @property {Color} WHITE EGA color.
 */
EGA = {
        BLACK: new Color(0, 0, 0),
        BLUE: new Color(0, 0, 170),
        GREEN: new Color(0, 170, 0),
        CYAN: new Color(0, 170, 170),
        RED: new Color(170, 0, 0),
        MAGENTA: new Color(170, 0, 170),
        BROWN: new Color(170, 85, 0),
        LIGHT_GRAY: new Color(170, 170, 170),
        DARK_GRAY: new Color(85, 85, 85),
        LIGHT_GREY: new Color(170, 170, 170),
        DARK_GREY: new Color(85, 85, 85),
        LIGHT_BLUE: new Color(85, 85, 255),
        LIGHT_GREEN: new Color(85, 255, 85),
        LIGHT_CYAN: new Color(85, 255, 255),
        LIGHT_RED: new Color(255, 85, 85),
        LIGHT_MAGENTA: new Color(255, 85, 255),
        YELLOW: new Color(255, 255, 85),
        WHITE: new Color(255, 255, 255)
};

/**
 * @property {Color} NO_COLOR the transparent Color.
 */
NO_COLOR = new Color(0, 0, 0, COLOR.XOR);
