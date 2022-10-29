/*
QOI - The "Quite OK Image" format for fast, lossless image compression

Dominic Szablewski - https://phoboslab.org

-- LICENSE: The MIT License(MIT)

Copyright(c) 2021 Dominic Szablewski
DOjS Javascript port by Andre Seidelt <superilu@yahoo.com>

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files(the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and / or sell copies
of the Software, and to permit persons to whom the Software is furnished to do
so, subject to the following conditions :
The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

function dbg(s) {
	//Println("[QOI] " + s);
}

/**
 * Read a 32 bit unsigned, big endian number from an ByteArray.
 * 
 * @param {ByteArray} data the data to read from
 * @param {Number} pos The position to start reading from.
 * 
 * @returns {Number} the resulting 32bit number
 */
function ReadUint32BE(data, pos) {
	return (data.Get(pos) << 24) | (data.Get(pos + 1) << 16) | (data.Get(pos + 2) << 8) | (data.Get(pos + 3));
}

/**
 * copy pixel data from source to destination.
 * 
 * @param {*} d destination object
 * @param {*} s source object
 */
function CopyPx(d, s) {
	d.r = s.r;
	d.g = s.g;
	d.b = s.b;
	d.a = s.a;
}

/**
 * Load a QOI image and return it as Bitmap.
 * @see https://qoiformat.org/
 * 
 * @param {String} fname name of the QOI file.
 * 
 * @returns {Bitmap} the loaded Bitmap.
 */
function LoadQoi(fname) {
	// read file contents into an ByteArray
	var f = new File(fname, FILE.READ);
	var data = f.ReadInts();
	f.Close();
	dbg("Loaded " + data.length + " Bytes from " + fname);

	return DecodeQoi(data);
}

/**
 * Decode a QOI image from ByteArray and return it as Bitmap.
 * @see https://qoiformat.org/
 * 
 * @param {ByteArray} data QOI data.
 * 
 * @returns {Bitmap} the loaded Bitmap.
 */
function DecodeQoi(data) {
	var p = 0;
	if (
		data.Get(0) != CharCode('q') ||
		data.Get(1) != CharCode('o') ||
		data.Get(2) != CharCode('i') ||
		data.Get(3) != CharCode('f')
	) {
		throw new Error("[QOI] Not a QOI: " + fname);
	}
	p = 4;

	// read width and height
	var w = ReadUint32BE(data, p);
	p += 4;
	var h = ReadUint32BE(data, p);
	p += 4;

	// get number of channels
	var channels = data.Get(p++);
	if ((channels < 3) || (channels > 4)) {
		throw new Error("[QOI] Unsupported number of channels: " + channels);
	}

	// get colorspace
	var colorspace = data.Get(p++);
	if ((colorspace < 0) || (colorspace > 1)) {
		throw new Error("[QOI] Unsupported colorspace: " + colorspace);
	}

	// create image to put pixel into
	var img = new Bitmap(w, h);
	SetRenderBitmap(img);
	ClearScreen(EGA.BLACK);

	dbg("This is a " + w + "x" + h + " image with " + channels + " channels and colorspace=" + colorspace);

	var QOI_COLOR_HASH = function (c) {
		return (c.r * 3 + c.g * 5 + c.b * 7 + c.a * 11);
	}

	// decode QOI
	var QOI_OP_INDEX = 0x00 /* 00xxxxxx */
	var QOI_OP_DIFF = 0x40 /* 01xxxxxx */
	var QOI_OP_LUMA = 0x80 /* 10xxxxxx */
	var QOI_OP_RUN = 0xc0 /* 11xxxxxx */
	var QOI_OP_RGB = 0xfe /* 11111110 */
	var QOI_OP_RGBA = 0xff /* 11111111 */
	var QOI_MASK_2 = 0xc0 /* 11000000 */

	var qoi_padding = [0, 0, 0, 0, 0, 0, 0, 1];
	var px_len = w * h * channels;
	var px = { r: 0, g: 0, b: 0, a: 255 };
	var run = 0;
	var index = [];
	for (var i = 0; i < 64; i++) {
		index.push({ r: 0, g: 0, b: 0, a: 255 });
	}

	var x = 0;
	var y = 0;
	var chunks_len = data.length - qoi_padding.length;
	for (var px_pos = 0; px_pos < px_len; px_pos += channels) {
		if (run > 0) {
			run--;
		}
		else if (p < chunks_len) {
			var b1 = data.Get(p++);

			if (b1 === QOI_OP_RGB) {
				px.r = data.Get(p++);
				px.g = data.Get(p++);
				px.b = data.Get(p++);
			}
			else if (b1 === QOI_OP_RGBA) {
				px.r = data.Get(p++);
				px.g = data.Get(p++);
				px.b = data.Get(p++);
				px.a = data.Get(p++);
			}
			else if ((b1 & QOI_MASK_2) === QOI_OP_INDEX) {
				var idx = index[b1];
				CopyPx(px, idx);
			}
			else if ((b1 & QOI_MASK_2) === QOI_OP_DIFF) {
				px.r += ((b1 >> 4) & 0x03) - 2;
				px.g += ((b1 >> 2) & 0x03) - 2;
				px.b += (b1 & 0x03) - 2;
			}
			else if ((b1 & QOI_MASK_2) === QOI_OP_LUMA) {
				var b2 = data.Get(p++);
				var vg = (b1 & 0x3f) - 32;
				px.r += vg - 8 + ((b2 >> 4) & 0x0f);
				px.g += vg;
				px.b += vg - 8 + (b2 & 0x0f);
			}
			else if ((b1 & QOI_MASK_2) === QOI_OP_RUN) {
				run = (b1 & 0x3f);
			}

			CopyPx(index[QOI_COLOR_HASH(px) % 64], px);
		}

		if (channels == 4) {
			Plot(x, y, Color(px.r, px.g, px.b, px.a));
		} else {
			Plot(x, y, Color(px.r, px.g, px.b));
		}
		x++;
		if (x >= w) {
			y++;
			x = 0;
		}
	}

	// return image
	SetRenderBitmap(null);
	return img;
}

// export functions and version
exports.__VERSION__ = 2;
exports.DecodeQoi = DecodeQoi;
exports.LoadQoi = LoadQoi;
