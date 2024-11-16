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

LoadLibrary("jpeg");

/*
** This function is called once when the script is started.
*/
function Setup() {
	SetFramerate(30);
	MouseShowCursor(false);

	var f = new File("tests/testdata/test.jpg", FILE.READ);
	var jpg_bytes = f.ReadInts();
	Println("Num bytes=" + jpg_bytes.length);
	i0 = new Bitmap(jpg_bytes);
	f.Close();

	i1 = new Bitmap("tests/testdata/test.jpg");
	i2 = new Bitmap("tests/testdata/normal.jpg");
	i3 = new Bitmap("tests/testdata/progress.jpg");

	i4 = new Bitmap("tests/testdata/bor01.jpg");
	i5 = new Bitmap("tests/testdata/bor02.jpg");
	i6 = new Bitmap("tests/testdata/bor03.jpg");

	img = i1;
}

/*
** This function is repeatedly until ESC is pressed or Stop() is called.
*/
function Loop() {
	ClearScreen(EGA.BLACK);

	if (img != null) {
		img.Draw(0, 0);
	}
}

/*
** This function is called on any input.
*/
function Input(e) {
	if (CompareKey(e.key, '0')) {
		img = i0;
	}
	if (CompareKey(e.key, '1')) {
		img = i1;
	}
	if (CompareKey(e.key, '2')) {
		img = i2;
	}
	if (CompareKey(e.key, '3')) {
		img = i3;
	}
	if (CompareKey(e.key, '4')) {
		img = i4;
	}
	if (CompareKey(e.key, '5')) {
		img = i5;
	}
	if (CompareKey(e.key, '6')) {
		img = i6;
	}
}
