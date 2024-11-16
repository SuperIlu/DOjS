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

LoadLibrary("qoi");

/*
** This function is called once when the script is started.
*/
function Setup() {
	tc = new Bitmap("tests/testdata/testcard.qoi");
	logo = new Bitmap("tests/testdata/qoi_logo.qoi");

	var f = new File("tests/testdata/qoi_logo.qoi", FILE.READ);
	var qoi_bytes = f.ReadInts();
	bm = new Bitmap(qoi_bytes);
	f.Close();
}

/*
** This function is repeatedly until ESC is pressed or Stop() is called.
*/
function Loop() {
	tc.Draw(0, 0);
	logo.DrawTrans(0, 0);
}

/*
** This function is called on any input.
*/
function Input(e) {
	if (CompareKey(e.key, '1')) {
		SaveQoiImage("scr.qoi");
	}
}
