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

	f1 = new Font("tests/testdata/pcxfnt/fnt01.pcx");
	Println(JSON.stringify(f1.ranges));

	f2 = new Font("tests/testdata/pcxfnt/fnt02.pcx");
	Println(JSON.stringify(f1.ranges));

	f3 = new Font("tests/testdata/pcxfnt/fnt03.pcx");
	Println(JSON.stringify(f1.ranges));

	f4 = new Font("tests/testdata/pcxfnt/fnt04.pcx");
	Println(JSON.stringify(f1.ranges));

	f5 = new Font("tests/testdata/pcxfnt/fnt05.pcx");
	Println(JSON.stringify(f1.ranges));

	fx = new Font("tests/testdata/pcxfnt/testfnt.pcx");
	Println(JSON.stringify(fx.ranges));

	fnt = f2;
}

/*
** This function is repeatedly until ESC is pressed or Stop() is called.
*/
function Loop() {
	ClearScreen(EGA.BLACK);

	var c = SizeX() / 2;
	var text = "This is a test of the emergency broadcast system!";

	var yPos = 2;

	TextXY(20, yPos, "name = " + fnt.filename + ", height=" + fnt.height, EGA.WHITE, NO_COLOR);
	yPos += 10;
	TextXY(20, yPos, "width = " + fnt.StringWidth(text), EGA.WHITE, NO_COLOR);
	yPos += 10;
	TextXY(20, yPos, "height = " + fnt.StringHeight(text), EGA.WHITE, NO_COLOR);
	yPos += 10;

	fnt.DrawStringLeft(c, yPos, text, EGA.LIGHT_CYAN, NO_COLOR);
	yPos += fnt.height + 5;
	fnt.DrawStringRight(c, yPos, text, EGA.LIGHT_CYAN, NO_COLOR);
	yPos += fnt.height + 5;
	fnt.DrawStringCenter(c, yPos, text, EGA.LIGHT_CYAN, NO_COLOR);
	Line(c, 240 + fnt.height, c, yPos, EGA.WHITE);
	yPos += fnt.height + 5;

	fnt.DrawStringCenter(c, yPos, text, EGA.MAGENTA, NO_COLOR);
	Line(c - 40, yPos, c + 40, yPos, EGA.GREEN);
	yPos += fnt.height * 2 + 5;

	fnt.DrawStringCenter(c, yPos - fnt.height, text, EGA.MAGENTA, NO_COLOR);
	Line(c - 40, yPos, c + 40, yPos, EGA.GREEN);
	yPos += fnt.height + 5;

	fnt.DrawStringCenter(c, yPos - fnt.height / 2, text, EGA.MAGENTA, NO_COLOR);
	Line(c - 40, yPos, c + 40, yPos, EGA.GREEN);
	yPos += fnt.height + 5;
}

/*
** This function is called on any input.
*/
function Input(e) {
	Println(JSON.stringify(e));
	if (CompareKey(e.key, '1')) {
		fnt = f1;
	}
	if (CompareKey(e.key, '2')) {
		fnt = f2;
	}
	if (CompareKey(e.key, '3')) {
		fnt = f3;
	}
	if (CompareKey(e.key, '4')) {
		fnt = f4;
	}
	if (CompareKey(e.key, '5')) {
		fnt = f5;
	}
	if (CompareKey(e.key, 'x')) {
		fnt = fx;
	}
}
