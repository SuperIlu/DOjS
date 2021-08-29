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

World map taken from https://commons.wikimedia.org/wiki/File:Whole_world_-_land_and_oceans.jpg?uselang=de

NASA Earth Observatory (NASA Goddard Space Flight Center)

Copyright information from http://visibleearth.nasa.gov/useterms.php
With the exception of images produced by the SeaWiFS, QuickBird, and IKONOS 
instruments all images on the Visible Earth are governed by NASA's Terms of
Use below. [...] For all non-private uses, NASA's Terms Of Use are as follows:
1. The imagery is free of licensing fees 
2. NASA requires that they be provided a credit as the owners of the imagery [...]

*/

/*
Im letzten Release hat DOjS support fuer TCP/IP Netzwerke bekommen. Wenn ein passender Treiber fuer die Netzwerkkarte geladen ist, dann
koennen Sketche per TCP oder UDP mit anderen Rechnern kommunizieren.

Ich habe in Javascript einen einfach HTTP-Client und sogar einen simplen HTTP-Server implementiert.

Dieser Sektch hier pollt sich die aktuelle Position der ISS per REST-API von der Webseite n2yo.com und zeichnet diese auf eine Weltkarte.
*/


var wTEXT2 = [
	"* TCP/IP support including:",
	"  * TCP client/server sockets",
	"  * UDP sockets",
	"  * DHCP and DNS",
	"* Simple HTTP client",
	"* Example sketch with HTTP server",
	"",
	"This example polls the ISS position",
	"via REST API from http://www.n2yo.com"
];

var API_KEY = "T2LHX5-FZEHWC-XF6PRY-4JB7"; // !!! You need to get an API key on http://www.n2yo.com !!!

var bgPic;
var frameCount = 0;
var data = null;
var latStep = 0;
var longStep = 0;
var trail = [];
var currentY, currentX;

exports.prepare = function () {
	SetFramerate(1);
	latStep = SizeY() / 180;
	longStep = SizeX() / 360;
	bgPic = new Bitmap("examples/Earth.bmp");
	var get = new Curl();
}

exports.present = function () {
	var yPos = 10;
	ClearScreen(EGA.BLACK);

	while (trail.length > 100) {
		trail.shift();
	}

	// draw earth and dim it
	bgPic.Draw(0, 0);
	FilledBox(0, 0, width, height, Color(0, 0, 0, 128));

	// draw ISS
	if (data) {
		for (var i = 0; i < trail.length; i++) {
			FilledCircle(trail[i][0], trail[i][1], 2, EGA.RED);
		}
		FilledCircle(currentX, currentY, 5, EGA.GREEN);
	}

	if (frameCount % 10 == 0) {
		try {
			var resp = get.DoRequest("https://api.n2yo.com/rest/v1/satellite/positions/25544/52.52437/13.41053/0/1/&apiKey=" + API_KEY);
			if (resp[2] == 200) {
				data = JSON.parse(resp[0].ToString());
				currentX = mapLong(data.positions[0].satlongitude);
				currentY = mapLat(data.positions[0].satlatitude);
				trail.push([currentX, currentY]);
			}
		} catch (e) {
			Println(e);
		}
	}
	frameCount++;

	// slide header
	colorMode(RGB);
	fill(222);
	textFont(fontHeader);
	text("TCP/IP support", 10, yPos);
	yPos += fontHeader.height + 5;

	colorMode(HSB);
	strokeWeight(2);
	stroke(frameCount % 255, 255, 255);
	line(10, yPos, width - 10, yPos);
	yPos += 5;

	colorMode(RGB);
	fill(222);
	textFont(fontSubheader);
	text("Using Watt32", 10, yPos);
	yPos += fontSubheader.height + 15;

	colorMode(RGB);
	textFont(fontSubheader);
	fill(200, 255, 255);
	for (var l = 0; l < wTEXT2.length; l++) {
		text(wTEXT2[l], 10, yPos);
		yPos += fontSubheader.height + 4;
	}
}

function mapLat(l) {
	var ret = SizeY() - ((l + 90) * latStep);
	return ret;
}

function mapLong(l) {
	var ret = SizeX() / 360 * (l + 180);
	return ret;
}

exports.keyHook = function (key) { };
exports.exitHook = function () {
	SetFramerate(60);
};
