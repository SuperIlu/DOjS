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

/**
 * create an evaluation chain.
 * @class
 */
function EvalChain() {
	this.chain = [];
};
/**
 * add a function to the chain.
 * 
 * @param {function} f a function to be called by the chain.
 */
EvalChain.prototype.Add = function (f) {
	this.chain.push(f);
};
/**
 * execute the next function (if any).
 * 
 * @returns {boolean} true if there were any functions to execute, else false.
 */
EvalChain.prototype.Step = function () {
	if (this.chain.length > 0) {
		var next = this.chain.shift();
		next();
		return true;
	} else {
		return false;
	}
};

/**
 * @returns {number} number of entries in the chain.
 */
EvalChain.prototype.Size = function () {
	return this.chain.length;
};

/**
 * empty the chain, regardless of the remaining steps
 */
EvalChain.prototype.Clear = function () {
	return this.chain = [];
};

// export functions and version
exports.__VERSION__ = 3;
exports.EvalChain = EvalChain;
