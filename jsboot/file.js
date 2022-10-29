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

/** @module other */

/**
 * file mode definition.
 * @property {*} READ open file in read mode.
 * @property {*} WRITE open file in write mode (truncating existing contents)
 * @property {*} APPEND open file in append mode.
 */
FILE = {
	READ: "rb",
	WRITE: "wb",
	APPEND: "ab"
};

/**
 * file seek definition.
 * @property {*} SET the offset is relative to the start of the file.
 * @property {*} CUR the offset is relative to the the current position indicator.
 * @property {*} END the offset is relative to the end-of-file.
 */
SEEK = {
	SET: 0,
	CUR: 1,
	END: 2
};

/**
 * ZIP file mode definition.
 * @property {*} READ open ZIP file in read mode.
 * @property {*} WRITE open ZIP file in write mode (truncating existing contents)
 * @property {*} APPEND open ZIP file in append mode.
 * @property {*} DELETE open ZIP file in delete mode.
 */
ZIPFILE = {
	READ: "r",
	WRITE: "w",
	DELETE: "d",
	APPEND: "a"
};
