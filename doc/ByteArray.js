/**
 * Create an empty ByteArray. ByteArrays can be used to very fast store a large number of byte values (0..255). They are supported mainly for File/Zip/Socket IO.
 * @see File
 * @see Zip
 * @see Socket
 * @see Curl
 * @class
 * 
 * @param {number[]|string[]|string} data numbers will be used as given, string arrays will be intepreted as "characters" and only the first char is added to the ByteArray. Strings will be added char by char.
 */
function ByteArray(data) {
	/** 
	 * current number of entries in ByteArray. 
	 * @member {number}
	 */
	this.length = 0;
	/**
	 * current allocation size of ByteArray (internal value). 
	 * @member {number} 
	 */
	this.alloc_size = 0;
}
/**
 * truncate ByteArray to zero length.
 * Note: this does not free any memory, it just declares the length of the array as 0.
 */
ByteArray.prototype.Clear = function () { };
/**
 * get value from specific index.
 * @param {number} idx the indext to retrieve.
 * @returns {number} the store value.
 */
ByteArray.prototype.Get = function (idx) { };
/**
 * replace value at the given index.
 * @param {number} idx the indext to change.
 * @param {number} val the new value.
 */
ByteArray.prototype.Set = function (idx, val) { };
/**
 * append value to ByteArray.
 * @param {number} val the new value.
 */
ByteArray.prototype.Push = function (val) { };
/**
 * retrieve and remove the last value in the ByteArray.
 * @returns {number} the former last value.
 */
ByteArray.prototype.Pop = function () { };
/**
 * retrieve and remove the first value in the ByteArray.
 * @returns {number} the former first value.
 */
ByteArray.prototype.Shift = function () { };
/**
 * append the contents of the Javascript array to the ByteArray.
 *
 * @param {number[]|string[]} data numbers will be used as given, string arrays will be intepreted as "characters" and only the first char is added to the ByteArray. Strings will be added char by char.
 */
ByteArray.prototype.Append = function (data) { };
/*
 * convert the contents of the ByteArray to a string. This simply interpretes each number in the ByteArray as an ASCII character (no filtering of NULL bytes or non printable characters and no UTF - 8 conversion).
 * @returns {string} the contents of the ByteArray as a string (as far as possible).
 */
ByteArray.prototype.ToString = function () { };
/*
* convert ByteArray to Javascript array.
* @returns {number[]} the contents of the ByteArray as Javascript array.
*/
ByteArray.prototype.ToArray = function () { };
