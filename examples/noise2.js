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
LoadLibrary("noise");

var noiseScale = 0.02;
var frame;

function Setup() {
	MouseShowCursor(false);
	frame = 0;
}

function Loop() {
	ClearScreen(0);
	for (var x = 0; x < 100; x++) {
		for (var y = 0; y < 100; y++) {
			var noiseVal = Noise(x * noiseScale, y * noiseScale, frame * noiseScale);
			Plot(x, y, Color(noiseVal * 255));
		}
	}

	for (var x = 0; x < 100; x++) {
		for (var y = 0; y < 100; y++) {
			var noiseVal = SNoise(x * noiseScale, y * noiseScale, frame * noiseScale);
			Plot(150 + x, y, Color(noiseVal * 255));
		}
	}
	frame++;
}

function Input(e) {
}
