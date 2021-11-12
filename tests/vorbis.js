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

LoadLibrary("vorbis");

function Setup() {
	MouseShowCursor(false);

	m = new Ogg("tests/test.ogg");

	Println(m.filename);
	Println(m.channels);
	Println(m.samplerate);
	Println(m.vendor);
	Println(m.comments);
	Println(m.buffersize);
	Println(m.maxframesize);
	Println(m.numsamples);
	Println(m.duration);

	SetFramerate(20);
}

function Loop() {
	m.Play();

	FilledBox(10, 10, 10 + 300, 10 + 20, EGA.BLACK);
	TextXY(10, 10, "" + m.CurrentSample(), EGA.RED, NO_COLOR);
}

function Input(e) {
}
