/*
MIT License

Copyright (c) 2019-2025 Andre Seidelt <superilu@yahoo.com>

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

LoadLibrary("smacker");

function Setup() {
	MouseShowCursor(false);

	// s = open("tests/testdata/ajfstr1.smk");
	// s = open("tests/testdata/ajfstr2.smk");
	// s = open("tests/testdata/ajfstr3.smk");
	// s = open("tests/testdata/hypnotix.smk");
	s = open("tests/testdata/wetlogo.smk");
	// s = open("tests/testdata/credits.smk");

	SetFramerate(60);
}

function Loop() {
	//ClearScreen(EGA.DARK_GRAY);

	frame = m.Play(0, 0);

	TextXY(10, 10, "" + frame, EGA.RED, NO_COLOR);
	TextXY(10, 20, "" + GetFramerate(), EGA.RED, NO_COLOR);
}

function open(f) {
	m = new SMACKER(f, false);

	Println(m.filename);
	Println("  " + m.width);
	Println("  " + m.height);
	Println("  " + m.framerate);
	Println("  " + m.frame_duration);
	Println("  " + m.num_frames);

	return m;
}