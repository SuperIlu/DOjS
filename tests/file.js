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

function Setup() {
	write_test();
	read_test();

	Stop();
}

function write_test() {
	var wf = new File("ftest.txt", FILE.WRITE);
	test(wf.Tell(), 0);

	wf.WriteByte(CharCode("A"));
	test(wf.Tell(), 1);

	var ch_array = [CharCode("B"), CharCode("C"), CharCode("D"), CharCode("E"), CharCode("F")];
	wf.WriteBytes(ch_array);
	test(wf.Tell(), 6);

	var ba = new ByteArray();
	ba.Append("1234567890");
	wf.WriteInts(ba);
	test(wf.Tell(), 16);

	wf.WriteByte(10);
	test(wf.Tell(), 17);

	wf.WriteBytes(ch_array, 3);
	test(wf.Tell(), 20);

	wf.WriteInts(ba, 5);
	test(wf.Tell(), 25);

	wf.Seek(0);
	wf.WriteByte(CharCode("a"));
	test(wf.Tell(), 1);

	wf.Seek(-1, SEEK.END);
	wf.WriteByte(CharCode("*"));
	test(wf.Tell(), 25);

	wf.Close();
	wf = null;
	ba = null;
}

function read_test() {
	var rf = new File("ftest.txt", FILE.READ);
	test(rf.Tell(), 0);

	var l1 = rf.ReadBytes(16);
	test(rf.Tell(), 16);
	test(l1[0], CharCode("a"));
	test(l1[1], CharCode("B"));
	test(l1[15], CharCode("0"));

	var nl = rf.ReadByte();
	test(rf.Tell(), 17);
	test(nl, 10);

	var l2 = rf.ReadInts(7);
	test(rf.Tell(), 24);
	test(l2.length, 7);
	test(l2.Get(0), CharCode("B"));
	test(l2.Get(6), CharCode("4"));

	rf.Seek(-3, SEEK.CUR);
	test(rf.Tell(), 21);
	var b = rf.ReadByte();
	test(rf.Tell(), 22);
	test(b, CharCode("2"));

	rf.Close();
}

function str2array(s) {
	var ret = [];
	for (var i = 0; i < s.length; i++) {
		ret.push(CharCode(s[i]));
	}
	return ret;
}

function test(is, exp) {
	if (is !== exp) {
		throw new Error("Test failed. Expected:'" + exp + "', actual:'" + is + "'");
	}
}

function Loop() {
}

function Input() { }
