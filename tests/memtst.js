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


var i1;
var count;


function Setup() {
	var dat = [];
	for (var x = 0; x < 256; x += 2) {
		for (var y = 0; y < 256; y += 2) {
			dat.push(0xFF000000 | (x << 8) | y);
		}
	}

	i1 = new Bitmap(dat, 128, 128);
	dat = null;

	count = 0;
};

function Loop() {
	ClearScreen(EGA.BLACK);
	Println(JSON.stringify(MemoryInfo()));
	//	Gc(true);

	var yPos = 10;

	i1.DrawTrans(SizeX() / 2 - 128, yPos + 30);

	var dat = [];
	for (var y = 0; y < 32; y++) {
		for (var x = 0; x < 256; x++) {
			dat.push(0x00FF0000 | ((y * 8) << 8) | ((x + count) % 255) << 24);
		}
	}
	DrawArray(dat, SizeX() / 2 - 128, yPos + 30 + 64 - 16, 256, 32);
	dat = null;
	count += 4;
};

function Input(key) { };
