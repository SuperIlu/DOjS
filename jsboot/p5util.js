/*
	This file was derived from the p5.js source code at
	https://github.com/processing/p5.js

	Copyright (c) the p5.js contributors and Andre Seidelt <superilu@yahoo.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

/**
* @module p5compat
*/

/**********************************************************************************************************************
 * array functions
 */

/**
 * Adds a value to the end of an array. Extends the length of
 * the array by one. Maps to Array.push().
 *
 * @method append
 * @param {Array} array Array to append
 * @param {any} value to be added to the Array
 * @return {Array} the array that was appended to
 * @example
 * function setup() {
 *   var myArray = ['Mango', 'Apple', 'Papaya'];
 *   print(myArray); // ['Mango', 'Apple', 'Papaya']
 *
 *   append(myArray, 'Peach');
 *   print(myArray); // ['Mango', 'Apple', 'Papaya', 'Peach']
 * }
 */
exports.append = function (array, value) {
	array.push(value);
	return array;
};

/**
 * Copies an array (or part of an array) to another array. The src array is
 * copied to the dst array, beginning at the position specified by
 * srcPosition and into the position specified by dstPosition. The number of
 * elements to copy is determined by length. Note that copying values
 * overwrites existing values in the destination array. To append values
 * instead of overwriting them, use concat().
 * <br><br>
 * The simplified version with only two arguments, arrayCopy(src, dst),
 * copies an entire array to another of the same size. It is equivalent to
 * arrayCopy(src, 0, dst, 0, src.length).
 * <br><br>
 * Using this function is far more efficient for copying array data than
 * iterating through a for() loop and copying each element individually.
 *
 * @method arrayCopy
 * @deprecated
 * @param {Array}  src           the source Array
 * @param {Integer} srcPosition  starting position in the source Array
 * @param {Array}  dst           the destination Array
 * @param {Integer} dstPosition   starting position in the destination Array
 * @param {Integer} length        number of Array elements to be copied
 *
 * @example
 * var src = ['A', 'B', 'C'];
 * var dst = [1, 2, 3];
 * var srcPosition = 1;
 * var dstPosition = 0;
 * var length = 2;
 *
 * print(src); // ['A', 'B', 'C']
 * print(dst); // [ 1 ,  2 ,  3 ]
 *
 * arrayCopy(src, srcPosition, dst, dstPosition, length);
 * print(dst); // ['B', 'C', 3]
 */
exports.arrayCopy = function (src, srcPosition, dst, dstPosition, length) {
	// the index to begin splicing from dst array
	var start;
	var end;

	if (typeof length !== 'undefined') {
		end = Math.min(length, src.length);
		start = dstPosition;
		src = src.slice(srcPosition, end + srcPosition);
	} else {
		if (typeof dst !== 'undefined') {
			// src, dst, length
			// rename  so we don't get confused
			end = dst;
			end = Math.min(end, src.length);
		} else {
			// src, dst
			end = src.length;
		}

		start = 0;
		// rename  so we don't get confused
		dst = srcPosition;
		src = src.slice(0, end);
	}

	// Since we are not returning the array and JavaScript is pass by reference
	// we must modify the actual values of the array
	// instead of reassigning arrays
	Array.prototype.splice.apply(dst, [start, end].concat(src));
};

/**
 * Concatenates two arrays, maps to Array.concat(). Does not modify the
 * input arrays.
 *
 * @method concat
 * @param {Array} a first Array to concatenate
 * @param {Array} b second Array to concatenate
 * @return {Array} concatenated array
 *
 * @example
 * function setup() {
 *   var arr1 = ['A', 'B', 'C'];
 *   var arr2 = [1, 2, 3];
 *
 *   print(arr1); // ['A','B','C']
 *   print(arr2); // [1,2,3]
 *
 *   var arr3 = concat(arr1, arr2);
 *
 *   print(arr1); // ['A','B','C']
 *   print(arr2); // [1, 2, 3]
 *   print(arr3); // ['A','B','C', 1, 2, 3]
 * }
 */
exports.concat = function (list0, list1) {
	return list0.concat(list1);
};

/**
 * Reverses the order of an array, maps to Array.reverse()
 *
 * @method reverse
 * @param {Array} list Array to reverse
 * @return {Array} the reversed list
 * @example
 * function setup() {
 *   var myArray = ['A', 'B', 'C'];
 *   print(myArray); // ['A','B','C']
 *
 *   reverse(myArray);
 *   print(myArray); // ['C','B','A']
 * }
 */
exports.reverse = function (list) {
	return list.reverse();
};

/**
 * Decreases an array by one element and returns the shortened array,
 * maps to Array.pop().
 *
 * @method shorten
 * @param  {Array} list Array to shorten
 * @return {Array} shortened Array
 * @example
 * function setup() {
 *   var myArray = ['A', 'B', 'C'];
 *   print(myArray); // ['A', 'B', 'C']
 *   var newArray = shorten(myArray);
 *   print(myArray); // ['A','B','C']
 *   print(newArray); // ['A','B']
 * }
 */
exports.shorten = function (list) {
	list.pop();
	return list;
};

/**
 * Randomizes the order of the elements of an array. Implements
 * <a href='http://Bost.Ocks.org/mike/shuffle/' target=_blank>
 * Fisher-Yates Shuffle Algorithm</a>.
 *
 * @method shuffle
 * @param  {Array}   array  Array to shuffle
 * @param  {Boolean} [bool] modify passed array
 * @return {Array}   shuffled Array
 * @example
 * function setup() {
 *   var regularArr = ['ABC', 'def', createVector(), TAU, Math.E];
 *   print(regularArr);
 *   shuffle(regularArr, true); // force modifications to passed array
 *   print(regularArr);
 *
 *   // By default shuffle() returns a shuffled cloned array:
 *   var newArr = shuffle(regularArr);
 *   print(regularArr);
 *   print(newArr);
 * }
 */
exports.shuffle = function (arr, bool) {
	var isView = ArrayBuffer && ArrayBuffer.isView && ArrayBuffer.isView(arr);
	arr = bool || isView ? arr : arr.slice();

	var rnd,
		tmp,
		idx = arr.length;
	while (idx > 1) {
		rnd = (Math.random() * idx) | 0;

		tmp = arr[--idx];
		arr[idx] = arr[rnd];
		arr[rnd] = tmp;
	}

	return arr;
};

/**
 * Sorts an array of numbers from smallest to largest, or puts an array of
 * words in alphabetical order. The original array is not modified; a
 * re-ordered array is returned. The count parameter states the number of
 * elements to sort. For example, if there are 12 elements in an array and
 * count is set to 5, only the first 5 elements in the array will be sorted.
 *
 * @method sort
 * @param {Array} list Array to sort
 * @param {Integer} [count] number of elements to sort, starting from 0
 * @return {Array} the sorted list
 *
 * @example
 * function setup() {
 *   var words = ['banana', 'apple', 'pear', 'lime'];
 *   print(words); // ['banana', 'apple', 'pear', 'lime']
 *   var count = 4; // length of array
 *
 *   words = sort(words, count);
 *   print(words); // ['apple', 'banana', 'lime', 'pear']
 * }
 * 
 * function setup() {
 *   var numbers = [2, 6, 1, 5, 14, 9, 8, 12];
 *   print(numbers); // [2, 6, 1, 5, 14, 9, 8, 12]
 *   var count = 5; // Less than the length of the array
 *
 *   numbers = sort(numbers, count);
 *   print(numbers); // [1,2,5,6,14,9,8,12]
 * }
 */
exports.sort = function (list, count) {
	var arr = count ? list.slice(0, Math.min(count, list.length)) : list;
	var rest = count ? list.slice(Math.min(count, list.length)) : [];
	if (typeof arr[0] === 'string') {
		arr = arr.sort();
	} else {
		arr = arr.sort(function (a, b) {
			return a - b;
		});
	}
	return arr.concat(rest);
};

/**
 * Inserts a value or an array of values into an existing array. The first
 * parameter specifies the initial array to be modified, and the second
 * parameter defines the data to be inserted. The third parameter is an index
 * value which specifies the array position from which to insert data.
 * (Remember that array index numbering starts at zero, so the first position
 * is 0, the second position is 1, and so on.)
 *
 * @method splice
 * @param {Array}  list Array to splice into
 * @param {any}    value value to be spliced in
 * @param {Integer} position in the array from which to insert data
 * @return {Array} the list
 *
 * @example
 * function setup() {
 *   var myArray = [0, 1, 2, 3, 4];
 *   var insArray = ['A', 'B', 'C'];
 *   print(myArray); // [0, 1, 2, 3, 4]
 *   print(insArray); // ['A','B','C']
 *
 *   splice(myArray, insArray, 3);
 *   print(myArray); // [0,1,2,'A','B','C',3,4]
 * }
 */
exports.splice = function (list, value, index) {
	// note that splice returns spliced elements and not an array
	Array.prototype.splice.apply(list, [index, 0].concat(value));

	return list;
};

/**
 * Extracts an array of elements from an existing array. The list parameter
 * defines the array from which the elements will be copied, and the start
 * and count parameters specify which elements to extract. If no count is
 * given, elements will be extracted from the start to the end of the array.
 * When specifying the start, remember that the first array element is 0.
 * This function does not change the source array.
 *
 * @method subset
 * @param  {Array}  list    Array to extract from
 * @param  {Integer} start   position to begin
 * @param  {Integer} [count] number of values to extract
 * @return {Array}          Array of extracted elements
 *
 * @example
 * function setup() {
 *   var myArray = [1, 2, 3, 4, 5];
 *   print(myArray); // [1, 2, 3, 4, 5]
 *
 *   var sub1 = subset(myArray, 0, 3);
 *   var sub2 = subset(myArray, 2, 2);
 *   print(sub1); // [1,2,3]
 *   print(sub2); // [3,4]
 * }
 */
exports.subset = function (list, start, count) {
	if (typeof count !== 'undefined') {
		return list.slice(start, start + count);
	} else {
		return list.slice(start, list.length);
	}
};

/**********************************************************************************************************************
 * conversion functions
 */

/**
 * Converts a string to its floating point representation. The contents of a
 * string must resemble a number, or NaN (not a number) will be returned.
 * For example, float("1234.56") evaluates to 1234.56, but float("giraffe")
 * will return NaN.
 *
 * When an array of values is passed in, then an array of floats of the same
 * length is returned.
 *
 * @method float
 * @param {String}  str float string to parse
 * @return {Number}     floating point representation of string
 * @example
 * var str = '20';
 * var diameter = float(str);
 * ellipse(width / 2, height / 2, diameter, diameter);
 */
exports.float = function (str) {
	if (str instanceof Array) {
		return str.map(parseFloat);
	}
	return parseFloat(str);
};

/**
 * Converts a boolean, string, or float to its integer representation.
 * When an array of values is passed in, then an int array of the same length
 * is returned.
 *
 * @method int
 * @param {String|Boolean|Number}       n value to parse
 * @param {Integer}       [radix] the radix to convert to (default: 10)
 * @return {Number}                     integer representation of value
 *
 * @example
 * print(int('10')); // 10
 * print(int(10.31)); // 10
 * print(int(-10)); // -10
 * print(int(true)); // 1
 * print(int(false)); // 0
 * print(int([false, true, '10.3', 9.8])); // [0, 1, 10, 9]
 */
exports.int = function (n, radix) {
	radix = radix || 10;
	if (typeof n === 'string') {
		return parseInt(n, radix);
	} else if (typeof n === 'number') {
		return n | 0;
	} else if (typeof n === 'boolean') {
		return n ? 1 : 0;
	} else if (n instanceof Array) {
		return n.map(function (n) {
			return exports.int(n, radix);
		});
	}
};

/**
 * Converts a boolean, string or number to its string representation.
 * When an array of values is passed in, then an array of strings of the same
 * length is returned.
 *
 * @method str
 * @param {String|Boolean|Number|Array} n value to parse
 * @return {String}                     string representation of value
 * @example
 * print(str('10')); // "10"
 * print(str(10.31)); // "10.31"
 * print(str(-10)); // "-10"
 * print(str(true)); // "true"
 * print(str(false)); // "false"
 * print(str([true, '10.3', 9.8])); // [ "true", "10.3", "9.8" ]
 */
exports.str = function (n) {
	if (n instanceof Array) {
		return n.map(exports.str);
	} else {
		return String(n);
	}
};

/**
 * Converts a number or string to its boolean representation.
 * For a number, any non-zero value (positive or negative) evaluates to true,
 * while zero evaluates to false. For a string, the value "true" evaluates to
 * true, while any other value evaluates to false. When an array of number or
 * string values is passed in, then a array of booleans of the same length is
 * returned.
 *
 * @method boolean
 * @param {String|Boolean|Number|Array} n value to parse
 * @return {Boolean}                    boolean representation of value
 * @example
 * print(boolean(0)); // false
 * print(boolean(1)); // true
 * print(boolean('true')); // true
 * print(boolean('abcd')); // false
 * print(boolean([0, 12, 'true'])); // [false, true, false]
 */
exports.boolean = function (n) {
	if (typeof n === 'number') {
		return n !== 0;
	} else if (typeof n === 'string') {
		return n.toLowerCase() === 'true';
	} else if (typeof n === 'boolean') {
		return n;
	} else if (n instanceof Array) {
		return n.map(exports.boolean);
	}
};

/**
 * Converts a number, string representation of a number, or boolean to its byte
 * representation. A byte can be only a whole number between -128 and 127, so
 * when a value outside of this range is converted, it wraps around to the
 * corresponding byte representation. When an array of number, string or boolean
 * values is passed in, then an array of bytes the same length is returned.
 *
 * @method byte
 * @param {String|Boolean|Number}       n value to parse
 * @return {Number}                     byte representation of value
 *
 * @example
 * print(byte(127)); // 127
 * print(byte(128)); // -128
 * print(byte(23.4)); // 23
 * print(byte('23.4')); // 23
 * print(byte('hello')); // NaN
 * print(byte(true)); // 1
 * print(byte([0, 255, '100'])); // [0, -1, 100]
 */
exports.byte = function (n) {
	var nn = exports.int(n, 10);
	if (typeof nn === 'number') {
		return (nn + 128) % 256 - 128;
	} else if (nn instanceof Array) {
		return nn.map(exports.byte);
	}
};

/**
 * Converts a number or string to its corresponding single-character
 * string representation. If a string parameter is provided, it is first
 * parsed as an integer and then translated into a single-character string.
 * When an array of number or string values is passed in, then an array of
 * single-character strings of the same length is returned.
 *
 * @method char
 * @param {String|Number}       n value to parse
 * @return {String}             string representation of value
 *
 * @example
 * print(char(65)); // "A"
 * print(char('65')); // "A"
 * print(char([65, 66, 67])); // [ "A", "B", "C" ]
 * print(join(char([65, 66, 67]), '')); // "ABC"
 */
exports.char = function (n) {
	if (typeof n === 'number' && !isNaN(n)) {
		return String.fromCharCode(n);
	} else if (n instanceof Array) {
		return n.map(exports.char);
	} else if (typeof n === 'string') {
		return exports.char(parseInt(n, 10));
	}
};

/**
 * Converts a single-character string to its corresponding integer
 * representation. When an array of single-character string values is passed
 * in, then an array of integers of the same length is returned.
 *
 * @method unchar
 * @param {String} n     value to parse
 * @return {Number}      integer representation of value
 *
 * @example
 * print(unchar('A')); // 65
 * print(unchar(['A', 'B', 'C'])); // [ 65, 66, 67 ]
 * print(unchar(split('ABC', ''))); // [ 65, 66, 67 ]
 */
exports.unchar = function (n) {
	if (typeof n === 'string' && n.length === 1) {
		return n.charCodeAt(0);
	} else if (n instanceof Array) {
		return n.map(exports.unchar);
	}
};

/**
 * Converts a number to a string in its equivalent hexadecimal notation. If a
 * second parameter is passed, it is used to set the number of characters to
 * generate in the hexadecimal notation. When an array is passed in, an
 * array of strings in hexadecimal notation of the same length is returned.
 *
 * @method hex
 * @param {Number} n     value to parse
 * @param {Number} [digits]
 * @return {String}      hexadecimal string representation of value
 *
 * @example
 * print(hex(255)); // "000000FF"
 * print(hex(255, 6)); // "0000FF"
 * print(hex([0, 127, 255], 6)); // [ "000000", "00007F", "0000FF" ]
 */
exports.hex = function (n, digits) {
	digits = digits === undefined || digits === null ? (digits = 8) : digits;
	if (n instanceof Array) {
		return n.map(function (n) {
			return exports.hex(n, digits);
		});
	} else if (typeof n === 'number') {
		if (n < 0) {
			n = 0xffffffff + n + 1;
		}
		var hex = Number(n)
			.toString(16)
			.toUpperCase();
		while (hex.length < digits) {
			hex = '0' + hex;
		}
		if (hex.length >= digits) {
			hex = hex.substring(hex.length - digits, hex.length);
		}
		return hex;
	}
};

/**
 * Converts a string representation of a hexadecimal number to its equivalent
 * integer value. When an array of strings in hexadecimal notation is passed
 * in, an array of integers of the same length is returned.
 *
 * @method unhex
 * @param {String} n value to parse
 * @return {Number}      integer representation of hexadecimal value
 *
 * @example
 * print(unhex('A')); // 10
 * print(unhex('FF')); // 255
 * print(unhex(['FF', 'AA', '00'])); // [ 255, 170, 0 ]
 */
exports.unhex = function (n) {
	if (n instanceof Array) {
		return n.map(exports.unhex);
	} else {
		return parseInt('0x' + n, 16);
	}
};

/**********************************************************************************************************************
 * string functions
 */

/**
 * Combines an array of Strings into one String, each separated by the
 * character(s) used for the separator parameter. To join arrays of ints or
 * floats, it's necessary to first convert them to Strings using nf() or
 * nfs().
 *
 * @method join
 * @param  {Array}  list      array of Strings to be joined
 * @param  {String} separator String to be placed between each item
 * @return {String}           joined String
 * @example
 * var array = ['Hello', 'world!'];
 * var separator = ' ';
 * var message = join(array, separator);
 * text(message, 5, 50);
 */
exports.join = function (list, separator) {
	p5._validateParameters('join', arguments);
	return list.join(separator);
};

/**
 * This function is used to apply a regular expression to a piece of text,
 * and return matching groups (elements found inside parentheses) as a
 * String array. If there are no matches, a null value will be returned.
 * If no groups are specified in the regular expression, but the sequence
 * matches, an array of length 1 (with the matched text as the first element
 * of the array) will be returned.
 * <br><br>
 * To use the function, first check to see if the result is null. If the
 * result is null, then the sequence did not match at all. If the sequence
 * did match, an array is returned.
 * <br><br>
 * If there are groups (specified by sets of parentheses) in the regular
 * expression, then the contents of each will be returned in the array.
 * Element [0] of a regular expression match returns the entire matching
 * string, and the match groups start at element [1] (the first group is [1],
 * the second [2], and so on).
 *
 * @method match
 * @param  {String} str    the String to be searched
 * @param  {String} regexp the regexp to be used for matching
 * @return {String[]}      Array of Strings found
 * @example
 * var string = 'Hello p5js*!';
 * var regexp = 'p5js\\*';
 * var m = match(string, regexp);
 * text(m, 5, 50);
 */
exports.match = function (str, reg) {
	p5._validateParameters('match', arguments);
	return str.match(reg);
};

/**
 * This function is used to apply a regular expression to a piece of text,
 * and return a list of matching groups (elements found inside parentheses)
 * as a two-dimensional String array. If there are no matches, a null value
 * will be returned. If no groups are specified in the regular expression,
 * but the sequence matches, a two dimensional array is still returned, but
 * the second dimension is only of length one.
 * <br><br>
 * To use the function, first check to see if the result is null. If the
 * result is null, then the sequence did not match at all. If the sequence
 * did match, a 2D array is returned.
 * <br><br>
 * If there are groups (specified by sets of parentheses) in the regular
 * expression, then the contents of each will be returned in the array.
 * Assuming a loop with counter variable i, element [i][0] of a regular
 * expression match returns the entire matching string, and the match groups
 * start at element [i][1] (the first group is [i][1], the second [i][2],
 * and so on).
 *
 * @method matchAll
 * @param  {String} str    the String to be searched
 * @param  {String} regexp the regexp to be used for matching
 * @return {String[]}         2d Array of Strings found
 * @example
 * var string = 'Hello p5js*! Hello world!';
 * var regexp = 'Hello';
 * matchAll(string, regexp);
 */
exports.matchAll = function (str, reg) {
	p5._validateParameters('matchAll', arguments);
	var re = new RegExp(reg, 'g');
	var match = re.exec(str);
	var matches = [];
	while (match !== null) {
		matches.push(match);
		// matched text: match[0]
		// match start: match.index
		// capturing group n: match[n]
		match = re.exec(str);
	}
	return matches;
};

/**
 * Utility function for formatting numbers into strings. There are two
 * versions: one for formatting floats, and one for formatting ints.
 * The values for the digits, left, and right parameters should always
 * be positive integers.
 * (NOTE): Be cautious when using left and right parameters as it prepends numbers of 0's if the parameter
 * if greater than the current length of the number.
 * For example if number is 123.2 and left parameter passed is 4 which is greater than length of 123
 * (integer part) i.e 3 than result will be 0123.2. Same case for right parameter i.e. if right is 3 than
 * the result will be 123.200.
 *
 * @method nf
 * @param {Number|String}       num      the Number to format
 * @param {Integer|String}      [left]   number of digits to the left of the
 *                                decimal point
 * @param {Integer|String}      [right]  number of digits to the right of the
 *                                decimal point
 * @return {String}               formatted String
 *
 * @example
 * var myFont;
 * function preload() {
 *   myFont = loadFont('assets/fonts/inconsolata.ttf');
 * }
 * function setup() {
 *   background(200);
 *   var num1 = 321;
 *   var num2 = -1321;
 *
 *   noStroke();
 *   fill(0);
 *   textFont(myFont);
 *   textSize(22);
 *
 *   text(nf(num1, 4, 2), 10, 30);
 *   text(nf(num2, 4, 2), 10, 80);
 *   // Draw dividing line
 *   stroke(120);
 *   line(0, 50, width, 50);
 * }
 */
exports.nf = function (nums, left, right) {
	p5._validateParameters('nf', arguments);
	if (nums instanceof Array) {
		return nums.map(function (x) {
			return doNf(x, left, right);
		});
	} else {
		var typeOfFirst = Object.prototype.toString.call(nums);
		if (typeOfFirst === '[object Arguments]') {
			if (nums.length === 3) {
				return this.nf(nums[0], nums[1], nums[2]);
			} else if (nums.length === 2) {
				return this.nf(nums[0], nums[1]);
			} else {
				return this.nf(nums[0]);
			}
		} else {
			return doNf(nums, left, right);
		}
	}
};

function doNf(num, left, right) {
	var neg = num < 0;
	var n = neg ? num.toString().substring(1) : num.toString();
	var decimalInd = n.indexOf('.');
	var intPart = decimalInd !== -1 ? n.substring(0, decimalInd) : n;
	var decPart = decimalInd !== -1 ? n.substring(decimalInd + 1) : '';
	var str = neg ? '-' : '';
	if (typeof right !== 'undefined') {
		var decimal = '';
		if (decimalInd !== -1 || right - decPart.length > 0) {
			decimal = '.';
		}
		if (decPart.length > right) {
			decPart = decPart.substring(0, right);
		}
		for (var i = 0; i < left - intPart.length; i++) {
			str += '0';
		}
		str += intPart;
		str += decimal;
		str += decPart;
		for (var j = 0; j < right - decPart.length; j++) {
			str += '0';
		}
		return str;
	} else {
		for (var k = 0; k < Math.max(left - intPart.length, 0); k++) {
			str += '0';
		}
		str += n;
		return str;
	}
}

/**
 * Utility function for formatting numbers into strings and placing
 * appropriate commas to mark units of 1000. There are two versions: one
 * for formatting ints, and one for formatting an array of ints. The value
 * for the right parameter should always be a positive integer.
 *
 * @method nfc
 * @param  {Number|String}   num     the Number to format
 * @param  {Integer|String}  [right] number of digits to the right of the
 *                                  decimal point
 * @return {String}           formatted String
 *
 * @example
 * function setup() {
 *   background(200);
 *   var num = 11253106.115;
 *   var numArr = [1, 1, 2];
 *
 *   noStroke();
 *   fill(0);
 *   textSize(12);
 *
 *   // Draw formatted numbers
 *   text(nfc(num, 4), 10, 30);
 *   text(nfc(numArr, 2), 10, 80);
 *
 *   // Draw dividing line
 *   stroke(120);
 *   line(0, 50, width, 50);
 * }
 */
exports.nfc = function (num, right) {
	p5._validateParameters('nfc', arguments);
	if (num instanceof Array) {
		return num.map(function (x) {
			return doNfc(x, right);
		});
	} else {
		return doNfc(num, right);
	}
};

function doNfc(num, right) {
	num = num.toString();
	var dec = num.indexOf('.');
	var rem = dec !== -1 ? num.substring(dec) : '';
	var n = dec !== -1 ? num.substring(0, dec) : num;
	n = n.toString().replace(/\B(?=(\d{3})+(?!\d))/g, ',');
	if (right === 0) {
		rem = '';
	} else if (typeof right !== 'undefined') {
		if (right > rem.length) {
			rem += dec === -1 ? '.' : '';
			var len = right - rem.length + 1;
			for (var i = 0; i < len; i++) {
				rem += '0';
			}
		} else {
			rem = rem.substring(0, right + 1);
		}
	}
	return n + rem;
}

/**
 * Utility function for formatting numbers into strings. Similar to nf() but
 * puts a "+" in front of positive numbers and a "-" in front of negative
 * numbers. There are two versions: one for formatting floats, and one for
 * formatting ints. The values for left, and right parameters
 * should always be positive integers.
 *
 * @method nfp
 * @param {Number} num      the Number to format
 * @param {Integer}      [left]   number of digits to the left of the decimal
 *                                point
 * @param {Integer}      [right]  number of digits to the right of the
 *                                decimal point
 * @return {String}         formatted String
 *
 * @example
 * function setup() {
 *   background(200);
 *   var num1 = 11253106.115;
 *   var num2 = -11253106.115;
 *
 *   noStroke();
 *   fill(0);
 *   textSize(12);
 *
 *   // Draw formatted numbers
 *   text(nfp(num1, 4, 2), 10, 30);
 *   text(nfp(num2, 4, 2), 10, 80);
 *
 *   // Draw dividing line
 *   stroke(120);
 *   line(0, 50, width, 50);
 * }
 */
exports.nfp = function () {
	p5._validateParameters('nfp', arguments);
	var nfRes = exports.nf.apply(this, arguments);
	if (nfRes instanceof Array) {
		return nfRes.map(addNfp);
	} else {
		return addNfp(nfRes);
	}
};

function addNfp(num) {
	return parseFloat(num) > 0 ? '+' + num.toString() : num.toString();
}

/**
 * Utility function for formatting numbers into strings. Similar to nf() but
 * puts an additional "_" (space) in front of positive numbers just in case to align it with negative
 * numbers which includes "-" (minus) sign.
 * The main usecase of nfs() can be seen when one wants to align the digits (place values) of a positive
 * number with some negative number (See the example to get a clear picture).
 * There are two versions: one for formatting float, and one for formatting int.
 * The values for the digits, left, and right parameters should always be positive integers.
 * (IMP): The result on the canvas basically the expected alignment can vary based on the typeface you are using.
 * (NOTE): Be cautious when using left and right parameters as it prepends numbers of 0's if the parameter
 * if greater than the current length of the number.
 * For example if number is 123.2 and left parameter passed is 4 which is greater than length of 123
 * (integer part) i.e 3 than result will be 0123.2. Same case for right parameter i.e. if right is 3 than
 * the result will be 123.200.
 *
 * @method nfs
 * @param {Number}       num      the Number to format
 * @param {Integer}      [left]   number of digits to the left of the decimal
 *                                point
 * @param {Integer}      [right]  number of digits to the right of the
 *                                decimal point
 * @return {String}         formatted String
 *
 * @example
 * var myFont;
 * function preload() {
 *   myFont = loadFont('assets/fonts/inconsolata.ttf');
 * }
 * function setup() {
 *   background(200);
 *   var num1 = 321;
 *   var num2 = -1321;
 *
 *   noStroke();
 *   fill(0);
 *   textFont(myFont);
 *   textSize(22);
 *
 *   // nfs() aligns num1 (positive number) with num2 (negative number) by
 *   // adding a blank space in front of the num1 (positive number)
 *   // [left = 4] in num1 add one 0 in front, to align the digits with num2
 *   // [right = 2] in num1 and num2 adds two 0's after both numbers
 *   // To see the differences check the example of nf() too.
 *   text(nfs(num1, 4, 2), 10, 30);
 *   text(nfs(num2, 4, 2), 10, 80);
 *   // Draw dividing line
 *   stroke(120);
 *   line(0, 50, width, 50);
 * }
 */
exports.nfs = function () {
	p5._validateParameters('nfs', arguments);
	var nfRes = exports.nf.apply(this, arguments);
	if (nfRes instanceof Array) {
		return nfRes.map(addNfs);
	} else {
		return addNfs(nfRes);
	}
};

function addNfs(num) {
	return parseFloat(num) > 0 ? ' ' + num.toString() : num.toString();
}

/**
 * The split() function maps to String.split(), it breaks a String into
 * pieces using a character or string as the delimiter. The delim parameter
 * specifies the character or characters that mark the boundaries between
 * each piece. A String[] array is returned that contains each of the pieces.
 *
 * The splitTokens() function works in a similar fashion, except that it
 * splits using a range of characters instead of a specific character or
 * sequence.
 *
 * @method split
 * @param  {String} value the String to be split
 * @param  {String} delim the String used to separate the data
 * @return {String[]}  Array of Strings
 * @example
 * var names = 'Pat,Xio,Alex';
 * var splitString = split(names, ',');
 * text(splitString[0], 5, 30);
 * text(splitString[1], 5, 50);
 * text(splitString[2], 5, 70);
 */
exports.split = function (str, delim) {
	p5._validateParameters('split', arguments);
	return str.split(delim);
};

/**
 * The splitTokens() function splits a String at one or many character
 * delimiters or "tokens." The delim parameter specifies the character or
 * characters to be used as a boundary.
 * <br><br>
 * If no delim characters are specified, any whitespace character is used to
 * split. Whitespace characters include tab (\t), line feed (\n), carriage
 * return (\r), form feed (\f), and space.
 *
 * @method splitTokens
 * @param  {String} value   the String to be split
 * @param  {String} [delim] list of individual Strings that will be used as
 *                          separators
 * @return {String[]}          Array of Strings
 * @example
 * function setup() {
 *   var myStr = 'Mango, Banana, Lime';
 *   var myStrArr = splitTokens(myStr, ',');
 *
 *   print(myStrArr); // prints : ["Mango"," Banana"," Lime"]
 * }
 */
exports.splitTokens = function (value, delims) {
	p5._validateParameters('splitTokens', arguments);
	var d;
	if (typeof delims !== 'undefined') {
		var str = delims;
		var sqc = /\]/g.exec(str);
		var sqo = /\[/g.exec(str);
		if (sqo && sqc) {
			str = str.slice(0, sqc.index) + str.slice(sqc.index + 1);
			sqo = /\[/g.exec(str);
			str = str.slice(0, sqo.index) + str.slice(sqo.index + 1);
			d = new RegExp('[\\[' + str + '\\]]', 'g');
		} else if (sqc) {
			str = str.slice(0, sqc.index) + str.slice(sqc.index + 1);
			d = new RegExp('[' + str + '\\]]', 'g');
		} else if (sqo) {
			str = str.slice(0, sqo.index) + str.slice(sqo.index + 1);
			d = new RegExp('[' + str + '\\[]', 'g');
		} else {
			d = new RegExp('[' + str + ']', 'g');
		}
	} else {
		d = /\s/g;
	}
	return value.split(d).filter(function (n) {
		return n;
	});
};

/**
 * Removes whitespace characters from the beginning and end of a String. In
 * addition to standard whitespace characters such as space, carriage return,
 * and tab, this function also removes the Unicode "nbsp" character.
 *
 * @method trim
 * @param  {String} str a String to be trimmed
 * @return {String}       a trimmed String
 *
 * @example
 * var string = trim('  No new lines\n   ');
 * text(string + ' here', 2, 50);
 */
exports.trim = function (str) {
	p5._validateParameters('trim', arguments);
	if (str instanceof Array) {
		return str.map(this.trim);
	} else {
		return str.trim();
	}
};

/**********************************************************************************************************************
 * date functions
 */

/**
 * The day() function
 * returns the current day as a value from 1 - 31.
 *
 * @method day
 * @return {Integer} the current day
 * @example
 * var d = day();
 * text('Current day: \n' + d, 5, 50);
 */
exports.day = function () {
	return new Date().getDate();
};

/**
 * The hour() function
 * returns the current hour as a value from 0 - 23.
 *
 * @method hour
 * @return {Integer} the current hour
 * @example
 * var h = hour();
 * text('Current hour:\n' + h, 5, 50);
 */
exports.hour = function () {
	return new Date().getHours();
};

/**
 * The minute() function
 * returns the current minute as a value from 0 - 59.
 *
 * @method minute
 * @return {Integer} the current minute
 * @example
 * var m = minute();
 * text('Current minute: \n' + m, 5, 50);
 */
exports.minute = function () {
	return new Date().getMinutes();
};

/**
 * Returns the number of milliseconds (thousandths of a second) since
 * starting the program. This information is often used for timing events and
 * animation sequences.
 *
 * @method millis
 * @return {Number} the number of milliseconds since starting the program
 * @example
 * var millisecond = millis();
 * text('Milliseconds \nrunning: \n' + millisecond, 5, 40);
 */
exports.millis = function () {
	return MsecTime();
};

/**
 * The month() function returns the current month as a value from 1 - 12.
 *
 * @method month
 * @return {Integer} the current month
 * @example
 * var m = month();
 * text('Current month: \n' + m, 5, 50);
 */
exports.month = function () {
	return new Date().getMonth() + 1; //January is 0!
};

/**
 * The second() function returns the current second as a value from 0 - 59.
 *
 * @method second
 * @return {Integer} the current second
 * @example
 * var s = second();
 * text('Current second: \n' + s, 5, 50);
 */
exports.second = function () {
	return new Date().getSeconds();
};

/**
 * The year() returns the current year as an integer (2014, 2015, 2016, etc).
 *
 * @method year
 * @return {Integer} the current year
 * @example
 * var y = year();
 * text('Current year: \n' + y, 5, 50);
 */
exports.year = function () {
	return new Date().getFullYear();
};

/**********************************************************************************************************************
 * dictionary
 */

/**
 *
 * Creates a new instance of p5.StringDict using the key-value pair
 * or the object you provide.
 *
 * @method createStringDict
 * @param {String} key
 * @param {String} value
 * @return {StringDict}
 *
 * @example
 * function setup() {
 *   let myDictionary = createStringDict('p5', 'js');
 *   print(myDictionary.hasKey('p5')); // logs true to console
 *
 *   let anotherDictionary = createStringDict({ happy: 'coding' });
 *   print(anotherDictionary.hasKey('happy')); // logs true to console
 * }
 */
exports.createStringDict = function (key, value) {
	return new StringDict(key, value);
};

/**
 *
 * Creates a new instance of NumberDict using the key-value pair
 * or object you provide.
 *
 * @method createNumberDict
 * @param {Number} key
 * @param {Number} value
 * @return {NumberDict}
 *
 * @example
 * function setup() {
 *   let myDictionary = createNumberDict(100, 42);
 *   print(myDictionary.hasKey(100)); // logs true to console
 *
 *   let anotherDictionary = createNumberDict({ 200: 84 });
 *   print(anotherDictionary.hasKey(200)); // logs true to console
 * }
 */
exports.createNumberDict = function (key, value) {
	return new NumberDict(key, value);
};

/**
* @module p5compat.TypedDict
*/

/**
 * Base class for all p5.Dictionary types. Specifically
 * typed Dictionary classes inherit from this class.
 *
 * @class TypedDict
 */
exports.TypedDict = function (key, value) {
	if (key instanceof Object) {
		this.data = key;
	} else {
		this.data = {};
		this.data[key] = value;
	}
	return this;
};

/**
 * Returns the number of key-value pairs currently stored in the Dictionary.
 *
 * @method size
 * @return {Integer} the number of key-value pairs in the Dictionary
 *
 * @example
 * function setup() {
 *   let myDictionary = createNumberDict(1, 10);
 *   myDictionary.create(2, 20);
 *   myDictionary.create(3, 30);
 *   print(myDictionary.size()); // logs 3 to the console
 * }
 */
exports.TypedDict.prototype.size = function () {
	return Object.keys(this.data).length;
};

/**
 * Returns true if the given key exists in the Dictionary,
 * otherwise returns false.
 *
 * @method hasKey
 * @param {Number|String} key that you want to look up
 * @return {Boolean} whether that key exists in Dictionary
 *
 * @example
 * function setup() {
 *   let myDictionary = createStringDict('p5', 'js');
 *   print(myDictionary.hasKey('p5')); // logs true to console
 * }
 */
exports.TypedDict.prototype.hasKey = function (key) {
	return this.data.hasOwnProperty(key);
};

/**
 * Returns the value stored at the given key.
 *
 * @method get
 * @param {Number|String} the key you want to access
 * @return {Number|String} the value stored at that key
 *
 * @example
 * function setup() {
 *   let myDictionary = createStringDict('p5', 'js');
 *   let myValue = myDictionary.get('p5');
 *   print(myValue === 'js'); // logs true to console
 * }
 */
exports.TypedDict.prototype.get = function (key) {
	if (this.data.hasOwnProperty(key)) {
		return this.data[key];
	} else {
		Println(key + ' does not exist in this Dictionary');
	}
};

/**
 * Updates the value associated with the given key in case it already exists
 * in the Dictionary. Otherwise a new key-value pair is added.
 *
 * @method set
 * @param {Number|String} key
 * @param {Number|String} value
 *
 * @example
 * function setup() {
 *   let myDictionary = createStringDict('p5', 'js');
 *   myDictionary.set('p5', 'JS');
 *   myDictionary.print(); // logs "key: p5 - value: JS" to console
 * }
 */

exports.TypedDict.prototype.set = function (key, value) {
	if (this._validate(value)) {
		this.data[key] = value;
	} else {
		Println('Those values dont work for this dictionary type.');
	}
};

/**
 * private helper function to handle the user passing in objects
 * during construction or calls to create()
 */
exports.TypedDict.prototype._addObj = function (obj) {
	for (var key in obj) {
		this.set(key, obj[key]);
	}
};

/**
 * Creates a new key-value pair in the Dictionary.
 *
 * @method create
 * @param {Number|String} key
 * @param {Number|String} value
 *
 * @example
 * function setup() {
 *   let myDictionary = createStringDict('p5', 'js');
 *   myDictionary.create('happy', 'coding');
 *   myDictionary.print();
 *   // above logs "key: p5 - value: js, key: happy - value: coding" to console
 * }
 */
exports.TypedDict.prototype.create = function (key, value) {
	if (key instanceof Object && typeof value === 'undefined') {
		this._addObj(key);
	} else if (typeof key !== 'undefined') {
		this.set(key, value);
	} else {
		Println(
			'In order to create a new Dictionary entry you must pass ' +
			'an object or a key, value pair'
		);
	}
};

/**
 * Removes all previously stored key-value pairs from the Dictionary.
 *
 * @method clear
 * @example
 * function setup() {
 *   let myDictionary = createStringDict('p5', 'js');
 *   print(myDictionary.hasKey('p5')); // prints 'true'
 *   myDictionary.clear();
 *   print(myDictionary.hasKey('p5')); // prints 'false'
 * }
 */
exports.TypedDict.prototype.clear = function () {
	this.data = {};
};

/**
 * Removes the key-value pair stored at the given key from the Dictionary.
 *
 * @method remove
 * @param {Number|String} key for the pair to remove
 *
 * @example
 * function setup() {
 *   let myDictionary = createStringDict('p5', 'js');
 *   myDictionary.create('happy', 'coding');
 *   myDictionary.print();
 *   // above logs "key: p5 - value: js, key: happy - value: coding" to console
 *   myDictionary.remove('p5');
 *   myDictionary.print();
 *   // above logs "key: happy value: coding" to console
 * }
 */
exports.TypedDict.prototype.remove = function (key) {
	if (this.data.hasOwnProperty(key)) {
		delete this.data[key];
	} else {
		throw new Error(key + ' does not exist in this Dictionary');
	}
};

/**
 * Logs the set of items currently stored in the Dictionary to the console.
 *
 * @method print
 *
 * @example
 * function setup() {
 *   let myDictionary = createStringDict('p5', 'js');
 *   myDictionary.create('happy', 'coding');
 *   myDictionary.print();
 *   // above logs "key: p5 - value: js, key: happy - value: coding" to console
 * }
 */
exports.TypedDict.prototype.print = function () {
	for (var item in this.data) {
		Println('key:' + item + ' value:' + this.data[item]);
	}
};

/**
 * Converts the Dictionary into a JSON file for local download.
 *
 * @method saveJSON
 * @example
 * function setup() {
 *   createCanvas(100, 100);
 *   background(200);
 *   text('click here to save', 10, 10, 70, 80);
 * }
 *
 * function mousePressed() {
 *   if (mouseX > 0 && mouseX < width && mouseY > 0 && mouseY < height) {
 *     createStringDict({
 *       john: 1940,
 *       paul: 1942,
 *       george: 1943,
 *       ringo: 1940
 *     }).saveJSON('beatles');
 *   }
 * }
 */
exports.TypedDict.prototype.saveJSON = function (filename, opt) {
	prototype.saveJSON(this.data, filename, opt);
};

/**
 * private helper function to ensure that the user passed in valid
 * values for the Dictionary type
 */
exports.TypedDict.prototype._validate = function (value) {
	return true;
};

/**
* @module p5compat.StringDict
*/

/**
 * A simple Dictionary class for Strings.
 *
 * @class StringDict
 * @extends TypedDict
 */

exports.StringDict = function () {
	TypedDict.apply(this, arguments);
};

exports.StringDict.prototype = Object.create(exports.TypedDict.prototype);

exports.StringDict.prototype._validate = function (value) {
	return typeof value === 'string';
};

/**
* @module p5compat.NumberDict
*/

/**
 * A simple Dictionary class for Numbers.
 *
 * @class NumberDict
 * @extends TypedDict
 */
exports.NumberDict = function () {
	TypedDict.apply(this, arguments);
};

exports.NumberDict.prototype = Object.create(exports.TypedDict.prototype);

/**
 * private helper function to ensure that the user passed in valid
 * values for the Dictionary type
 */
exports.NumberDict.prototype._validate = function (value) {
	return typeof value === 'number';
};

/**
 * Add the given number to the value currently stored at the given key.
 * The sum then replaces the value previously stored in the Dictionary.
 *
 * @method add
 * @param {Number} Key for the value you wish to add to
 * @param {Number} Number to add to the value
 * @example
 * function setup() {
 *   let myDictionary = createNumberDict(2, 5);
 *   myDictionary.add(2, 2);
 *   print(myDictionary.get(2)); // logs 7 to console.
 * }
 */
exports.NumberDict.prototype.add = function (key, amount) {
	if (this.data.hasOwnProperty(key)) {
		this.data[key] += amount;
	} else {
		Println('The key - ' + key + ' does not exist in this dictionary.');
	}
};

/**
 * Subtract the given number from the value currently stored at the given key.
 * The difference then replaces the value previously stored in the Dictionary.
 *
 * @method sub
 * @param {Number} Key for the value you wish to subtract from
 * @param {Number} Number to subtract from the value
 * @example
 * function setup() {
 *   let myDictionary = createNumberDict(2, 5);
 *   myDictionary.sub(2, 2);
 *   print(myDictionary.get(2)); // logs 3 to console.
 * }
 */
exports.NumberDict.prototype.sub = function (key, amount) {
	this.add(key, -amount);
};

/**
 * Multiply the given number with the value currently stored at the given key.
 * The product then replaces the value previously stored in the Dictionary.
 *
 * @method mult
 * @param {Number} Key for value you wish to multiply
 * @param {Number} Amount to multiply the value by
 * @example
 * function setup() {
 *   let myDictionary = createNumberDict(2, 4);
 *   myDictionary.mult(2, 2);
 *   print(myDictionary.get(2)); // logs 8 to console.
 * }
 */
exports.NumberDict.prototype.mult = function (key, amount) {
	if (this.data.hasOwnProperty(key)) {
		this.data[key] *= amount;
	} else {
		Println('The key - ' + key + ' does not exist in this dictionary.');
	}
};

/**
 * Divide the given number with the value currently stored at the given key.
 * The quotient then replaces the value previously stored in the Dictionary.
 *
 * @method div
 * @param {Number} Key for value you wish to divide
 * @param {Number} Amount to divide the value by
 * @example
 * function setup() {
 *   let myDictionary = createNumberDict(2, 8);
 *   myDictionary.div(2, 2);
 *   print(myDictionary.get(2)); // logs 4 to console.
 * }
 */
exports.NumberDict.prototype.div = function (key, amount) {
	if (this.data.hasOwnProperty(key)) {
		this.data[key] /= amount;
	} else {
		Println('The key - ' + key + ' does not exist in this dictionary.');
	}
};

/**
 * private helper function for finding lowest or highest value
 * the argument 'flip' is used to flip the comparison arrow
 * from 'less than' to 'greater than'
 */
exports.NumberDict.prototype._valueTest = function (flip) {
	if (Object.keys(this.data).length === 0) {
		throw new Error(
			'Unable to search for a minimum or maximum value on an empty NumberDict'
		);
	} else if (Object.keys(this.data).length === 1) {
		return this.data[Object.keys(this.data)[0]];
	} else {
		var result = this.data[Object.keys(this.data)[0]];
		for (var key in this.data) {
			if (this.data[key] * flip < result * flip) {
				result = this.data[key];
			}
		}
		return result;
	}
};

/**
 * Return the lowest number currently stored in the Dictionary.
 *
 * @method minValue
 * @return {Number}
 * @example
 * function setup() {
 *   let myDictionary = createNumberDict({ 2: -10, 4: 0.65, 1.2: 3 });
 *   let lowestValue = myDictionary.minValue(); // value is -10
 *   print(lowestValue);
 * }
 *
 */
exports.NumberDict.prototype.minValue = function () {
	return this._valueTest(1);
};

/**
 * Return the highest number currently stored in the Dictionary.
 *
 * @method maxValue
 * @return {Number}
 * @example
 * function setup() {
 *   let myDictionary = createNumberDict({ 2: -10, 4: 0.65, 1.2: 3 });
 *   let highestValue = myDictionary.maxValue(); // value is 3
 *   print(highestValue);
 * }
 */
exports.NumberDict.prototype.maxValue = function () {
	return this._valueTest(-1);
};

/**
 * private helper function for finding lowest or highest key
 * the argument 'flip' is used to flip the comparison arrow
 * from 'less than' to 'greater than'
 *
 */
exports.NumberDict.prototype._keyTest = function (flip) {
	if (Object.keys(this.data).length === 0) {
		throw new Error('Unable to use minValue on an empty NumberDict');
	} else if (Object.keys(this.data).length === 1) {
		return Object.keys(this.data)[0];
	} else {
		var result = Object.keys(this.data)[0];
		for (var i = 1; i < Object.keys(this.data).length; i++) {
			if (Object.keys(this.data)[i] * flip < result * flip) {
				result = Object.keys(this.data)[i];
			}
		}
		return result;
	}
};

/**
 * Return the lowest key currently used in the Dictionary.
 *
 * @method minKey
 * @return {Number}
 * @example
 * function setup() {
 *   let myDictionary = createNumberDict({ 2: 4, 4: 6, 1.2: 3 });
 *   let lowestKey = myDictionary.minKey(); // value is 1.2
 *   print(lowestKey);
 * }
 */
exports.NumberDict.prototype.minKey = function () {
	return this._keyTest(1);
};

/**
 * Return the highest key currently used in the Dictionary.
 *
 * @method maxKey
 * @return {Number}
 * @example
 * function setup() {
 *   let myDictionary = createNumberDict({ 2: 4, 4: 6, 1.2: 3 });
 *   let highestKey = myDictionary.maxKey(); // value is 4
 *   print(highestKey);
 * }
 */
exports.NumberDict.prototype.maxKey = function () {
	return this._keyTest(-1);
};
