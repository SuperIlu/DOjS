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
	MouseShowCursor(false);
	SetFramerate(1);

	Println("Resolve()           = " + JSON.stringify(Resolve("pool.ntp.org")));
}

function Loop() {
	var NTP_TIMESTAMP_DELTA = 2208988800;	// Time in seconds since Jan, 1970 for UNIX epoch.

	var udp = UdpSocket("pool.ntp.org", 123);
	udp.WriteBytes([
		0x1B, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 8
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 8
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 8
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 8
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 8
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 8
	]);
	udp.WaitFlush();
	udp.WaitInput();
	var res = udp.ReadInts(48);
	udp.Close();

	Println(res.Get(40));
	Println(res.Get(41));
	Println(res.Get(42));
	Println(res.Get(43));

	// parse seconds
	var ntpSeconds = 0;
	ntpSeconds += res.Get(40) & 0xFF;
	ntpSeconds *= 256;
	ntpSeconds += res.Get(41) & 0xFF;
	ntpSeconds *= 256;
	ntpSeconds += res.Get(42) & 0xFF;
	ntpSeconds *= 256;
	ntpSeconds += res.Get(43) & 0xFF;
	var epoch = (ntpSeconds - NTP_TIMESTAMP_DELTA); // convert NTP time to epoch
	Println("BE ntpSecs = " + ntpSeconds);
	Println("BE Unix Epoch = " + epoch);
	Println("BE Date = " + new Date(epoch * 1000));
}

function Input(e) {
}
