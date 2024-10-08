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

var USE_INT_ARRAY = true;

/*
** This function is called once when the script is started.
*/
function Setup() {
	MouseShowCursor(false);
	SoundInputSource(SOUND.Input.MIC);
	SoundStartInput(2000, 8, true);
}

/*
** This function is repeatedly until ESC is pressed or Stop() is called.
*/
function Loop() {
	if (USE_INT_ARRAY) {
		var snd = ReadSoundInputInts();
		if (snd) {
			ClearScreen(EGA.BLACK);
			var lastX = 0;
			var lastY = 0;
			for (var i = 0; i < SizeX(); i++) {
				Line(lastX, lastY, i, snd[0].Get(i), EGA.RED);
				lastX = i;
				lastY = snd[0].Get(i);
			}
		}
	} else {
		var snd = ReadSoundInput();
		if (snd) {
			ClearScreen(EGA.BLACK);
			var lastX = 0;
			var lastY = 0;
			for (var i = 0; i < SizeX(); i++) {
				Line(lastX, lastY, i, snd[0][i], EGA.RED);
				lastX = i;
				lastY = snd[0][i];
			}
		}
	}
}

/*
** This function is called on any input.
*/
function Input(event) {
}
