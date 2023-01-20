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

LoadLibrary("png");

/*
** This function is called once when the script is started.
*/
function Setup() {
}

/*
** This function is repeatedly until ESC is pressed or Stop() is called.
*/
function Loop() {
	var testfile = "testz.zip";

	var z = new Zip(testfile, "w");
	z.AddFile("file1", "tests/zip.js");
	z.AddFile("dir1/file2", "tests/zip.js");
	z.WriteBytes("memfile.bin", StringToBytes("This is a test of the emergency broadcast system!"));
	z.WriteBytes("test.js", StringToBytes("function x(){}"));
	z.AddFile("tstfont.fnt", "jsboot/fonts/helv22.fnt");
	z.AddFile("tstimg.bmp", "tests/testdata/testgrad.bmp");
	z.AddFile("tstimg.pcx", "tests/testdata/testgrad.pcx");
	z.AddFile("tstimg.tga", "tests/testdata/testgrad.tga");
	z.AddFile("tstimg.png", "tests/testdata/testgrad.png");
	z.AddFile("smpl.wav", "tests/testdata/mono.wav");
	z.AddFile("tst.mid", "examples/d_intro.mid");
	z.Close();
	Println("Added files to ZIP");

	var x = new Zip(testfile, "r");
	Println("Opened ZIP with " + x.NumEntries());
	Println(JSON.stringify(x.GetEntries()));
	x.ExtractFile("file1", "tst1.tmp");
	x.ExtractFile("dir1/file2", "tst2.tmp");
	Println(BytesToString(x.ReadBytes("memfile.bin")));
	x.Close();
	Println("Extracted two files from ZIP");

	Println(JSON.stringify(Require("jsboot/file.js")));
	Println(JSON.stringify(Require(testfile + "=test.js")));

	var fnt = new Font(testfile + "=tstfont.fnt");
	Println(fnt.filename);
	Println(fnt.height);

	["bmp", "pcx", "tga", "png"].forEach(function (ext) {
		var img = new Bitmap(testfile + "=tstimg." + ext);
		Println(img.filename);
		Println(img.width);
		Println(img.height);
	});

	var s1 = new Sample(testfile + "=smpl.wav");
	Println("name = " + s1.filename + ", length=" + s1.length + ", freq=" + s1.frequency);

	var m1 = new Midi(testfile + "=tst.mid");
	Println("name = " + m1.filename);
	Println("length = " + m1.length + "s");

	Stop();
}

/*
** This function is called on any input.
*/
function Input(e) {
}
