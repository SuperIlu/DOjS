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

fontList = [];
startIdx = 0;
endIdx = -1;

FONT_DIR = 'jsboot/fonts';

function Setup() {
	fileNames = List(FONT_DIR);
	fileNames.forEach(function (e) {
		if (e != ".." && e != ".") {
			var name = FONT_DIR + "/" + e;
			Println("Loading " + name);
			try {
				fontList.push(new Font(name));
			} catch (e) {
				Println(e);
			}
		}
	});
}

function Loop() {
	ClearScreen(EGA.GREEN);
	var yPos = 0;
	for (var i = startIdx; i < fontList.length; i++) {
		if (yPos + fontList[i].height > SizeY()) {
			break;
		}
		var name = fontList[i].filename.split("/")[2];
		var str = name + " / This is a test of the emergency broadcast system where the quick brown fox jumps over the fence!";
		fontList[i].DrawStringLeft(10, yPos, str, EGA.WHITE, NO_COLOR);
		yPos += 4 + fontList[i].height;
		endIdx = i;
	}
}

function Input(e) {
	// next page if available, else exit
	if ((e.key & 0xFF) == CharCode(" ")) {
		Println("SPACE!");
		if (endIdx == fontList.length - 1) {
			Stop();
		} else {
			startIdx = endIdx;
		}
	}
}
