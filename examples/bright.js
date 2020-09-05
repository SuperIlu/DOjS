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

function Setup() {
	pic = new Bitmap("examples/3dfx.png");

	bright = 128;
}

function Loop() {
	pic.Draw(0, 0);

	var bLow = (bright - 3) * 3;
	var bHigh = (bright + 3) * 3;

	for (var x = 0; x < pic.width; x++) {
		for (var y = 0; y < pic.height; y++) {
			var p = pic.GetPixel(x, y);
			var b = GetRed(p) + GetGreen(p) + GetBlue(p);
			if (b > bLow && b < bHigh) {
				Plot(x, y, EGA.WHITE);
			}
		}
	}
	bright++;
	if (bright > 255) {
		bright = 0;
	}
}

function Input(e) {
}
