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

LoadLibrary("webp");

/*
** This function is called once when the script is started.
*/
function Setup() {
	tc = new Bitmap("tests/testdata/webp/rose.webp");
	logo = new Bitmap("tests/testdata/webp/testcard.wep");

	f = new File("tests/testdata/webp/rose.webp", FILE.READ);
	webp_bytes = f.ReadInts();
	Println("size = " + webp_bytes.length);
	Println("webp[0] = " + webp_bytes.Get(0));
	Println("webp[1] = " + webp_bytes.Get(1));
	Println("webp[2] = " + webp_bytes.Get(2));
	Println("webp[3] = " + webp_bytes.Get(3));

	Println("webp[8] = " + webp_bytes.Get(8));
	Println("webp[9] = " + webp_bytes.Get(9));
	Println("webp[10] = " + webp_bytes.Get(10));
	Println("webp[11] = " + webp_bytes.Get(11));
	bm = new Bitmap(webp_bytes);
}

/*
** This function is repeatedly until ESC is pressed or Stop() is called.
*/
function Loop() {
	ClearScreen(EGA.BLACK);

	tc.Draw(0, 0);
	logo.DrawTrans(0, 0);
}

/*
** This function is called on any input.
*/
function Input(e) {
	if (CompareKey(e.key, '1')) {
		SaveWebpImage("scr.wep");
	}
}
