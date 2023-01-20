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

var mono, stereo, xPos, lastY, playing, voice;

/*
** This function is called once when the script is started.
*/
function Setup() {
	mono = new Sample("tests/testdata/mono.wav");
	stereo = new Sample("tests/testdata/stereo.wav");

	info(mono);
	info(stereo);

	xPos = 0;
	voice = -1;
	playing = null;
	lastY = [0, 0];
}

function info(s) {
	Println(s.filename);
	Println("  length=" + s.length);
	Println("  frequency=" + s.frequency);
	Println("  bits=" + s.bits);
	Println("  stereo=" + s.stereo);
	Println("  time [s]=" + s.length / s.frequency);
}

/*
** This function is repeatedly until ESC is pressed or Stop() is called.
*/
function Loop() {
	if (voice != -1) {
		var pos = VoiceGetPosition(voice);
		if (pos != -1) {
			var data = playing.Get(pos);
			var left = data[0] >> 8;
			var right = data[1] >> 8;
			Line(xPos, lastY[0], xPos + 1, left, EGA.RED);
			Line(xPos, 240 + lastY[1], xPos + 1, 240 + right, EGA.GREEN);
			lastY[0] = left;
			lastY[1] = right;
		}
		xPos++;
		if (xPos >= SizeX()) {
			xPos = 0;
		}
	}
}

/*
** This function is called on any input.
*/
function Input(e) {
	mono.Stop();
	stereo.Stop();
	if (CompareKey(e.key, '1')) {
		playing = mono;
	}
	if (CompareKey(e.key, '2')) {
		playing = stereo;
	}
	voice = playing.Play(255, 128, true);
	Println("voice=" + voice);
}
