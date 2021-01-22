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

/*
** This function is called once when the script is started.
*/
function Setup() {
	SetFramerate(30);

	s1 = new Sample("examples/roost8.wav");
	s2 = new Sample("examples/bike8.wav");
	s3 = new Sample("examples/explo8.wav");
}

/*
** This function is repeatedly until ESC is pressed or Stop() is called.
*/
function Loop() {
	ClearScreen(EGA.BLACK);

	TextXY(20, 120 + 0, "name = " + s1.filename + ", length=" + s1.length + ", freq=" + s1.frequency, EGA.WHITE, NO_COLOR);
	TextXY(20, 120 + 10, "name = " + s2.filename + ", length=" + s2.length + ", freq=" + s2.frequency, EGA.WHITE, NO_COLOR);
	TextXY(20, 120 + 20, "name = " + s3.filename + ", length=" + s3.length + ", freq=" + s3.frequency, EGA.WHITE, NO_COLOR);
}

/*
** This function is called on any input.
*/
function Input(e) {
	if (CompareKey(e.key, '1')) {
		s1.Play(255, 128, true);
	}
	if (CompareKey(e.key, '2')) {
		s2.Play(255, 128, true);
	}
	if (CompareKey(e.key, '3')) {
		s3.Play(255, 128, true);
	}

	if (CompareKey(e.key, 's')) {
		s1.Stop();
		s2.Stop();
		s3.Stop();
	}
}
