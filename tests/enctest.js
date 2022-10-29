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

Include("qoi");
Include("qoienc");

function Setup() {
	var start = MsecTime();
	MouseShowCursor(false);

	// create test pattern
	ClearScreen(EGA.BLACK);

	// load and render QOI logo
	var img = LoadQoi("tests/qoi_test_images/qoi_logo.qoi");
	var loadEnd = MsecTime();
	img.Draw(Width / 2 - img.width / 2, Height / 2 - img.height / 2);

	// draw a pattern over it
	Line(0, 0, Width, Height, EGA.GREEN);
	Line(0, Height, Width, 0, EGA.GREEN);
	Circle(Width / 2, Height / 2, Height / 3, EGA.RED);

	// save resulting image
	var bm = new Bitmap(0, 0, Width, Height);
	//EncodeQoi(bm);
	SaveQoiImage(bm, "test.qoi");
	var end = MsecTime();

	Println("Runtime   := " + (end - start) + "ms");
	Println("Load time := " + (loadEnd - start) + "ms");
}

function Loop() {
}

function Input() { }
