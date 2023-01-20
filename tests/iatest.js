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
	Println("\nTests:");

	na = new IntArray();
	assert("length", na.length, 0);
	Println("na " + na.length + ", " + na.alloc_size);

	na.Push(42);
	assert("length", na.length, 1);
	Println("na " + na.length + ", " + na.alloc_size);

	g = na.Get(0);
	assert("length", na.length, 1);
	assert("get", g, 42);
	Println("na[0]=" + g + " " + na.length + ", " + na.alloc_size);

	na.Set(0, 23);
	assert("length", na.length, 1);
	assert("get 0", na.Get(0), 23);
	Println("na[0]=" + g + " " + na.length + ", " + na.alloc_size);

	for (i = 1; i < 10; i++) {
		na.Push(i);
	}
	assert("length", na.length, 10);
	for (i = 1; i < 10; i++) {
		assert("get" + i, na.Get(i), i);
	}
	Println("na " + na.length + ", " + na.alloc_size + ", " + JSON.stringify(na.ToArray()));

	assert("shift", na.Shift(), 23);
	assert("length", na.length, 9);
	Println("na " + na.length + ", " + na.alloc_size + ", " + JSON.stringify(na.ToArray()));

	assert("pop", na.Pop(), 9);
	assert("length", na.length, 8);
	Println("na " + na.length + ", " + na.alloc_size + ", " + JSON.stringify(na.ToArray()));

	var error = false;
	try {
		g = na.Get(42);
	} catch (e) {
		Println("get 42 " + e);
		error = true;
	}
	assert("missing exception", error, true);

	var error = false;
	try {
		g = na.Get(-3);
	} catch (e) {
		Println("get -3 " + e);
		error = true;
	}
	assert("missing exception", error, true);

	for (i = 0; i < 100; i++) {
		na.Push(i);
	}
	assert("length", na.length, 108);
	Println("na " + na.length + ", " + na.alloc_size + ", " + JSON.stringify(na.ToArray()));


	var stra = new IntArray();
	assert("length", stra.length, 0);
	Println("stra " + stra.length + ", " + stra.alloc_size);

	var HW = "Hello World!";

	var jsa = HW.split('');
	Println("jsa " + JSON.stringify(jsa));
	stra.Append(jsa);
	assert("length", stra.length, HW.length);
	assert("Append/ToString", stra.ToString(), HW);
	Println("stra " + stra.ToString());

	var stra = new IntArray(jsa);
	assert("length", stra.length, HW.length);
	assert("Append/ToString", stra.ToString(), HW);

	var stra = new IntArray();
	stra.Append(HW);
	assert("length", stra.length, HW.length);
	assert("Append/ToString", stra.ToString(), HW);

	var stra = new IntArray(HW);
	assert("length", stra.length, HW.length);
	assert("Append/ToString", stra.ToString(), HW);

	var numa = new IntArray([1, 2, 3]);
	assert("length", numa.length, 3);
	assert("get 0", numa.Get(0), 1);
	assert("get 1", numa.Get(1), 2);
	assert("get 2", numa.Get(2), 3);
}

function assert(txt, ist, soll) {
	if (ist != soll) {
		throw txt + ": result was '" + ist + "' but should be '" + soll + "'";
	}
}

RUN_SIZE = 20000;
POP_SHIFT_SIZE = 100;

/*
** This function is repeatedly until ESC is pressed or Stop() is called.
*/
function Loop() {
	Println("\nBenchmark:");

	// file read
	var sw = new StopWatch();
	sw.Start();
	f1 = new File("tests/testdata/lava.3df", FILE.READ);
	f1_a = f1.ReadBytes();
	f1.Close();
	sw.Stop();
	sw.Print("File.ReadBytes() := " + f1_a.length);

	var sw = new StopWatch();
	sw.Start();
	f2 = new File("tests/testdata/lava.3df", FILE.READ);
	f2_a = f2.ReadInts();
	f2.Close();
	sw.Stop();
	sw.Print("File.ReadInts() := " + f2_a.length);

	// file write
	var sw = new StopWatch();
	sw.Start();
	f1 = new File("tmp1.tmp", FILE.WRITE);
	f1.WriteBytes(f1_a);
	f1.Close();
	sw.Stop();
	sw.Print("File.WriteBytes()");

	var sw = new StopWatch();
	sw.Start();
	f2 = new File("tmp2.tmp", FILE.WRITE);
	f2.WriteInts(f2_a);
	f2.Close();
	sw.Stop();
	sw.Print("File.WriteInts()");

	// zip write
	var sw = new StopWatch();
	sw.Start();
	z = new Zip("tmp1.zip", "w");
	z.WriteBytes("lava.3df", f1_a);
	z.Close();
	sw.Stop();
	sw.Print("Zip.WriteBytes()");

	var sw = new StopWatch();
	sw.Start();
	z = new Zip("tmp2.zip", "w");
	z.WriteInts("lava.3df", f2_a);
	z.Close();
	sw.Stop();
	sw.Print("Zip.WriteInts()");

	// push
	var sw = new StopWatch();
	sw.Start();
	tAr = [];
	for (var i = 0; i < RUN_SIZE; i++) {
		tAr.push(i);
	}
	sw.Stop();
	sw.Print("Array.Push() := " + tAr.length);

	var sw = new StopWatch();
	sw.Start();
	nAr = new IntArray();
	for (var i = 0; i < RUN_SIZE; i++) {
		nAr.Push(i);
	}
	sw.Stop();
	sw.Print("IntArray.Push() := " + nAr.length);

	// get
	var sum = 0;
	var sw = new StopWatch();
	sw.Start();
	for (var i = 0; i < tAr.length; i++) {
		sum += tAr[i];
	}
	sw.Stop();
	sw.Print("Array.Get() == " + sum);

	var sum = 0;
	var sw = new StopWatch();
	sw.Start();
	for (var i = 0; i < tAr.length; i++) {
		sum += nAr.Get(i);
	}
	sw.Stop();
	sw.Print("IntArray.Get() == " + sum);

	// pop
	var sum = 0;
	var sw = new StopWatch();
	sw.Start();
	for (var i = 0; i < POP_SHIFT_SIZE; i++) {
		sum += tAr.pop();
	}
	sw.Stop();
	sw.Print("Array.Pop() == " + sum);

	var sum = 0;
	var sw = new StopWatch();
	sw.Start();
	for (var i = 0; i < POP_SHIFT_SIZE; i++) {
		sum += nAr.Pop();
	}
	sw.Stop();
	sw.Print("IntArray.Pop() == " + sum);

	// shift
	var sum = 0;
	var sw = new StopWatch();
	sw.Start();
	for (var i = 0; i < POP_SHIFT_SIZE; i++) {
		sum += tAr.shift();
	}
	sw.Stop();
	sw.Print("Array.Shift() == " + sum);

	var sum = 0;
	var sw = new StopWatch();
	sw.Start();
	for (var i = 0; i < POP_SHIFT_SIZE; i++) {
		sum += nAr.Shift();
	}
	sw.Stop();
	sw.Print("IntArray.Shift() == " + sum);

	Stop();
}

/*
** This function is called on any input.
*/
function Input(e) {
}


/*
function IntArray() {
	this.a = [];
}
IntArray.prototype.Push = function (v) { this.a.push(v); }
IntArray.prototype.Pop = function () { return this.a.pop(); }
IntArray.prototype.Shift = function () { return this.a.shift(); }
IntArray.prototype.ToArray = function () { return this.a; }
IntArray.prototype.Get = function (i) { return this.a[i]; }
IntArray.prototype.Set = function (i, v) { this.a[i] = v; }
*/
