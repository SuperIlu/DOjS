/**
 * Create a artificial neural network.
 * 
 * **Note: Neural module must be loaded by calling LoadLibrary("neural") before using!**
 * 
 * @see LoadLibrary()
 * @constructor 
 * @param {number} inputs size of input layer
 * @param {number} num_hidden number of hidden layers
 * @param {number} hidden size of the hidden layers
 * @param {number} outputs size of output layer
 *//**
* Create a artificial neural network.
*
* **Note: Neural module must be loaded by calling LoadLibrary("neural") before using!**
* 
* @see LoadLibrary()
* @constructor 
* @param {string} fname file name of training data previously stored by Save()
* @see Save()
*/
function Neural(inputs, num_hidden, hidden, outputs) { }
/**
 * free ressources of the neural network.
 */
Neural.prototype.Close = function () { }
/**
 * Run the network on some input and get a response.
 * 
 * @param {number[]|DoubleArray} inp Input data. must contain at least the number of input values as specified when creating the ANN.
 * 
 * @returns {number[]} The networks response to the input.
 */
Neural.prototype.Run = function (inp) { }
/**
 * Train the network with a single dataset.
 * 
 * @param {number[]|DoubleArray} inp Input data. Must contain at least the number of input values as specified when creating the ANN.
 * @param {number[]|DoubleArray} outp Expected output data. Must contain at least the number of output values as specified when creating the ANN.
 * @param {number} rate Learning rate (>0).
 */
Neural.prototype.Train = function (inp, outp, rate) { }
/**
 * Store training data of the network to disk.
 * 
 * @param {string} fname The file name.
 */
Neural.prototype.Save = function (fname) { }


/**
 * Create an empty DoubleArray. DoubleArrays can be used to very fast store a large number of float values.
 * For now it it only available when used with Neural
 * 
 * @see Neural
 * 
 * @class
 * 
 * @param {number[]|string[]|string} data numbers will be used as given.
 */
function DoubleArray(data) {
	/** 
	 * current number of entries in DoubleArray. 
	 * @member {number}
	 */
	this.length = 0;
	/**
	 * current allocation size of DoubleArray (internal value). 
	 * @member {number} 
	 */
	this.alloc_size = 0;
}
/**
 * truncate DoubleArray to zero length.
 * Note: this does not free any memory, it just declares the length of the array as 0.
 */
DoubleArray.prototype.Clear = function () { };
/**
 * get value from specific index.
 * @param {number} idx the indext to retrieve.
 * @returns {number} the store value.
 */
DoubleArray.prototype.Get = function (idx) { };
/**
 * replace value at the given index.
 * @param {number} idx the indext to change.
 * @param {number} val the new value.
 */
DoubleArray.prototype.Set = function (idx, val) { };
/**
 * append value to DoubleArray.
 * @param {number} val the new value.
 */
DoubleArray.prototype.Push = function (val) { };
/**
 * retrieve and remove the last value in the DoubleArray.
 * @returns {number} the former last value.
 */
DoubleArray.prototype.Pop = function () { };
/**
 * retrieve and remove the first value in the DoubleArray.
 * @returns {number} the former first value.
 */
DoubleArray.prototype.Shift = function () { };
/**
 * convert DoubleArray to Javascript array.
 * @returns {number[]} the contents of the DoubleArray as Javascript array.
 */
DoubleArray.prototype.ToArray = function () { };
/**
 * append the contents of the Javascript array to the DoubleArray.
 *
 * @param {number[]|string[]} data numbers will be used as given, string arrays will be intepreted as "characters" and only the first char is added to the DoubleArray. Strings will be added char by char.
 */
DoubleArray.prototype.Append = function (data) { };
