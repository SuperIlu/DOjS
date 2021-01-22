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

var API_KEY = "XXX"; // !!! You need to get an API key on http://www.n2yo.com !!!

var bgPic;
var frameCount = 0;
var data = null;
var latStep = 0;
var longStep = 0;
var trail = [];
var currentY, currentX;

function Setup() {
	SetFramerate(1);
	MouseShowCursor(false);
	latStep = SizeY() / 180;
	longStep = SizeX() / 360;
	bgPic = new Bitmap("examples/Earth.bmp");
}

function Loop() {
	ClearScreen(EGA.BLACK);

	while (trail.length > 100) {
		trail.shift();
	}

	bgPic.Draw(0, 0);
	if (data) {
		FilledCircle(currentX, currentY, 5, EGA.RED);
		for (var i = 0; i < trail.length; i++) {
			FilledCircle(trail[i][0], trail[i][1], 2, EGA.MAGENTA);
		}
	}

	if (frameCount % 10 == 0) {
		var http_res = http_get("http://www.n2yo.com/rest/v1/satellite/positions/25544/52.52437/13.41053/0/1/&apiKey=" + API_KEY);
		if (http_ok(http_res)) {
			data = JSON.parse(http_string_content(http_res));
			currentX = mapLong(data.positions[0].satlongitude);
			currentY = mapLat(data.positions[0].satlatitude);
			trail.push([currentX, currentY]);
		}
	}
	frameCount++;
}

/**
 * map -90..90 to 480..0
 * @param {*} l 
 */
function mapLat(l) {
	var ret = SizeY() - ((l + 90) * latStep);
	//Println("Lat  " + l + " := " + ret);
	return ret;
}

function mapLong(l) {
	var ret = SizeX() / 360 * (l + 180);
	//Println("Long " + l + " := " + ret);
	return ret;
}

function Input() {
}
