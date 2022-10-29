/**
 * Create an empty IntArray. IntArrays can be used to very fast store a large number of integer values.
 * Note: IntArrays are no longer used for IO. Fast File/ZIP/Socket is now provided by ByteArray.
 * @see ByteArray
 * @class
 * 
 * @param {number[]|string[]|string} data numbers will be used as given, string arrays will be intepreted as "characters" and only the first char is added to the IntArray. Strings will be added char by char.
 */
function IntArray(data) {
	/** 
	 * current number of entries in IntArray. 
	 * @member {number}
	 */
	this.length = 0;
	/**
	 * current allocation size of IntArray (internal value). 
	 * @member {number} 
	 */
	this.alloc_size = 0;
}
/**
 * truncate IntArray to zero length.
 * Note: this does not free any memory, it just declares the length of the array as 0.
 */
IntArray.prototype.Clear = function () { };
/**
 * get value from specific index.
 * @param {number} idx the indext to retrieve.
 * @returns {number} the store value.
 */
IntArray.prototype.Get = function (idx) { };
/**
 * replace value at the given index.
 * @param {number} idx the indext to change.
 * @param {number} val the new value.
 */
IntArray.prototype.Set = function (idx, val) { };
/**
 * append value to IntArray.
 * @param {number} val the new value.
 */
IntArray.prototype.Push = function (val) { };
/**
 * retrieve and remove the last value in the IntArray.
 * @returns {number} the former last value.
 */
IntArray.prototype.Pop = function () { };
/**
 * retrieve and remove the first value in the IntArray.
 * @returns {number} the former first value.
 */
IntArray.prototype.Shift = function () { };
/**
 * append the contents of the Javascript array to the IntArray.
 *
 * @param {number[]|string[]} data numbers will be used as given, string arrays will be intepreted as "characters" and only the first char is added to the IntArray. Strings will be added char by char.
 */
IntArray.prototype.Append = function (data) { };
/*
 * convert the contents of the IntArray to a string. This simply interpretes each number in the IntArray as an ASCII character (no filtering of NULL bytes or non printable characters and no UTF - 8 conversion).
 * @returns {string} the contents of the IntArray as a string (as far as possible).
 */
IntArray.prototype.ToString = function () { };
/*
* convert IntArray to Javascript array.
* @returns {number[]} the contents of the IntArray as Javascript array.
*/
IntArray.prototype.ToArray = function () { };
