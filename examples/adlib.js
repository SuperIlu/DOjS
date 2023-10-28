/*
MIT License

Copyright (c) 2019-2023 Andre Seidelt <superilu@yahoo.com>

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
Include("opl2");

var opl;

// register writes after https://bochs.sourceforge.io/techspec/adlib_sb.txt
function Setup() {
	opl = new OPL2();

	Println("detect = " + opl.Detect());


	// |        20          01      Set the modulator's multiple to 1
	opl.Write(0x20, 0x01);

	// |        40          10      Set the modulator's level to about 40 dB
	opl.Write(0x40, 0x10);

	// |        60          F0      Modulator attack:  quick;   decay:   long
	opl.Write(0x60, 0xF0);

	// |        80          77      Modulator sustain: medium;  release: medium
	opl.Write(0x80, 0x77);

	// |        A0          98      Set voice frequency's LSB (it'll be a D#)
	opl.Write(0xA0, 0x98);

	// |        23          01      Set the carrier's multiple to 1
	opl.Write(0x23, 0x01);

	// |        43          00      Set the carrier to maximum volume (about 47 dB)
	opl.Write(0x43, 0x00);

	// |        63          F0      Carrier attack:  quick;   decay:   long
	opl.Write(0x63, 0xF0);

	// |        83          77      Carrier sustain: medium;  release: medium
	opl.Write(0x83, 0x77);

	// |        B0          31      Turn the voice on; set the octave and freq MSB
	opl.Write(0xB0, 0x31);

	// |   To turn the voice off, set register B0h to 11h (or, in fact, any value 
	// |   which leaves bit 5 clear).  It's generally preferable, of course, to
	// |   induce a delay before doing so.
	Sleep(1500);
	opl.Write(0xB0, 0x11);

	Stop();
}

function Loop() { }

function OnExit() {
	if (opl) {
		opl.Reset();
		Println("OPL Reset in OnExit");
	}
}
