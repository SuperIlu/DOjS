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

/*
** This function is called once when the script is started.
*/
function Setup() {
	SetFramerate(30);

	f = new Font("jsboot/fonts/tms18.fnt");
}

/*
** This function is repeatedly until ESC is pressed or Stop() is called.
*/
function Loop() {
	ClearScreen(EGA.BLACK);

	var c = SizeX() / 2;
	var text = "This is a test of the emergency broadcast system!";

	TextXY(100, 100, text, EGA.MAGENTA, NO_COLOR);
	TextXY(20, 120 + 0, "name = " + f.filename + ", height=" + f.height, EGA.WHITE, NO_COLOR);
	TextXY(20, 120 + 10, "width = " + f.StringWidth(text), EGA.WHITE, NO_COLOR);
	TextXY(20, 120 + 20, "height = " + f.StringHeight(text), EGA.WHITE, NO_COLOR);

	f.DrawStringLeft(c, 200, text, EGA.LIGHT_CYAN, NO_COLOR);
	f.DrawStringRight(c, 220, text, EGA.LIGHT_CYAN, NO_COLOR);
	f.DrawStringCenter(c, 240, text, EGA.LIGHT_CYAN, NO_COLOR);
	Line(c, 240 + f.height, c, 200, EGA.WHITE);

	f.DrawStringCenter(c, 300, text, EGA.MAGENTA, NO_COLOR);
	Line(c - 40, 300, c + 40, 300, EGA.GREEN);

	f.DrawStringCenter(c, 330 - f.height, text, EGA.MAGENTA, NO_COLOR);
	Line(c - 40, 330, c + 40, 330, EGA.GREEN);

	f.DrawStringCenter(c, 360 - f.height / 2, text, EGA.MAGENTA, NO_COLOR);
	Line(c - 40, 360, c + 40, 360, EGA.GREEN);
}

/*
** This function is called on any input.
*/
function Input(event) {
}
