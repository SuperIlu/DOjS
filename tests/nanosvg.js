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

LoadLibrary("nanosvg");
LoadLibrary("png");

function Setup() {
	SetFramerate(30);
	MouseShowCursor(false);

	bm1 = new Bitmap(Width, Height, EGA.DARK_GRAY);
	bm1.RenderSVG("tests/23.svg");
	bm1.SavePngImage("23.png");

	bm2 = new Bitmap(Width, Height, EGA.DARK_GRAY);
	bm2.RenderSVG("tests/drawing.svg");
	bm2.SavePngImage("drawing.png");

	bm3 = new Bitmap(Width, Height, EGA.DARK_GRAY);
	bm3.RenderSVG("tests/nano.svg");
	bm3.SavePngImage("nano.png");

	img = bm1;
}

function Loop() {
	ClearScreen(EGA.DARK_GRAY);
	img.DrawTrans(0, 0);
}

function Input(e) {
	Println(JSON.stringify(e));
	if (CompareKey(e.key, '1')) {
		img = bm1;
	}
	if (CompareKey(e.key, '2')) {
		img = bm2;
	}
	if (CompareKey(e.key, '3')) {
		img = bm3;
	}
}
