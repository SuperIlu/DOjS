/*
MIT License

Copyright (c) 2019-2020 Andre Seidelt <superilu@yahoo.com>

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

var DRIVE = 0;

var num_blocks, current_block;

var block_list = [];

function Setup() {
	MouseShowCursor(false);
	num_blocks = GetRawSectorSize(DRIVE);
	current_block = 0;
}

function Loop() {
	var data = RawRead(DRIVE, current_block);
	current_block++;
	if (current_block > num_blocks) {
		current_block = 0;
	}
	block_list.push(data);

	while (block_list.length > SizeY()) {
		block_list.shift();
	}

	for (var y = 0; y < block_list.length; y++) {
		for (var x = 0; x < block_list[y].length; x++) {
			var col = HSBColor(block_list[y][x], 255, 255, 255);
			Plot(x, y, col);
		}
	}
}

function Input(event) {
}

/*
number of pixel = 640x480 := 307.200
number of sectors = 2880

pixel/sector = 106

*/
