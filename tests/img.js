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
LoadLibrary("png");
LoadLibrary("jpeg");
LoadLibrary("tiff");
LoadLibrary("jasper");
LoadLibrary("webp");

/*
** This function is called once when the script is started.
*/
function Setup() {
	SetFramerate(30);
	MouseShowCursor(false);

	i1 = img_test("examples/dojs.bmp");
	i2 = img_test("examples/3dfx.tga");
	i3 = img_test("examples/glow.pcx");
	i6 = img_test("examples/3dfx.png");
	i5 = img_test("tests/testdata/qoi_logo.qoi");
	i7 = new Bitmap("tests/testdata/640.tif");
	i8 = img_test("examples/3dfx.jpg");
	i9 = img_test("examples/3dfx_bw.jpg");

	ia = img_test("tests/testdata/3dfx.ras");
	ib = img_test("tests/testdata/640.jp2");
	ic = img_test("tests/testdata/webp/rose.wep");

	i10 = new Bitmap("LEHLh[WB2yk8pyoJadR*.7kCMdnj", 320, 240, 1);
	// i10 = new Bitmap("L", 320, 240, 1);

	// var dat = [];
	// for (var x = 0; x < 255; x++) {
	// 	for (var y = 0; y < 255; y++) {
	// 		dat.push(0xFF000000 | (x << 8) | y);
	// 	}
	// }

	// i4 = new Bitmap(dat, 255, 255);

	var dat = [];
	for (var y = 0; y < 32; y++) {
		for (var x = 0; x < 256; x++) {
			dat.push(0x0000FF | ((y * 8) << 8) | x << 24);
		}
	}

	i4 = new Bitmap(dat, 256, 32);

	img = i10;
	cnt = 0;
}

function img_test(fname) {
	Println("Trying " + fname);

	// test ByteArray
	var f = new File(fname, FILE.READ);
	var bytes = f.ReadInts();
	var bm = new Bitmap(bytes);

	// test direct file load
	return new Bitmap(fname);
}

/*
** This function is repeatedly until ESC is pressed or Stop() is called.
*/
function Loop() {
	ClearScreen(EGA.BLACK);

	if (img != null) {
		img.Draw(0, 0);
	}

	var dat = [];
	for (var x = 0; x < 32; x++) {
		for (var y = 0; y < 32; y++) {
			dat.push(0xFF000000 | ((cnt % 32) * 7 << 16) | (x * 7 << 8) | y * 7);
		}
	}
	DrawArray(dat, SizeX() / 2, SizeY() / 2, 32, 32);
	cnt++;
}

/*
** This function is called on any input.
*/
function Input(e) {
	Println(JSON.stringify(e));
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
	if (CompareKey(e.key, '7')) {
		img = i7;
	}
	if (CompareKey(e.key, '8')) {
		img = i8;
	}
	if (CompareKey(e.key, '9')) {
		img = i9;
	}
	if (CompareKey(e.key, '0')) {
		img = i10;
	}
	if (CompareKey(e.key, 'a')) {
		img = ia;
	}
	if (CompareKey(e.key, 'b')) {
		img = ib;
	}
	if (CompareKey(e.key, 'c')) {
		img = ic;
	}
	if (CompareKey(e.key, 's')) {
		img = new Bitmap(100, 100, 255, 255);
		Println(img.constructor.toString());
		img.SaveBmpImage("5.bmp");
		img.SaveJp2Image("5.jp2");
		img.SaveJpgImage("5.jpg");
		img.SavePcxImage("5.pcx");
		img.SavePngImage("5.png");
		img.SaveQoiImage("5.qoi");
		img.SaveRasImage("5.ras");
		img.SaveTgaImage("5.tga");
		img.SaveTiffImage("5.tif");
		img.SaveWebpImage("5.wep");

		SaveBmpImage("scr.bmp");
		SaveJp2Image("scr.jp2");
		SaveJpgImage("scr.jpg");
		SavePcxImage("scr.pcx");
		SavePngImage("scr.png");
		SaveQoiImage("scr.qoi");
		SaveRasImage("scr.ras");
		SaveTgaImage("scr.tga");
		SaveTiffImage("scr.tif");
		SaveWebpImage("scr.wep");

		SaveJpgImage("scr2.jpg", 20);
		SaveJpgImage("scr6.jpg", 60);
		SaveJp2Image("scr1.jp2", 10);
		SaveJp2Image("scr2.jp2", 20);
		SaveJp2Image("scr6.jp2", 60);
		SaveWebpImage("scr2.wep", 20);
		SaveWebpImage("scr6.wep", 60);
	}
	Println(img.constructor.toString());
}
