/**
 * Open a ZIP, for file modes see {@link ZIPFILE}.
 * @class
 * @param {string} filename the name of the file.
 * @param {ZIPFILE} mode READ, WRITE or APPEND.
 * @param {number} [compression] 1..9 to specify compression level.
 */
function Zip(filename, mode, compression) { }
/**
 * close ZIP.
 */
Zip.prototype.Close = function () { };
/**
 * get number of entries in ZIP.
 * 
 * @returns {number} number of entries in this ZIP.
 */
Zip.prototype.NumEntries = function () { };
/**
 * get an array with the file entries in the ZIP.
 * 
 * @returns {Object[]} an array containing the file entries of the ZIP in the following format:
 * @example
 * [
 *      {name:string, is_directory:bool, size:number, crc32:number},
 *      ...
 * ]
 */
Zip.prototype.GetEntries = function () { };
/**
 * add a file to a ZIP.
 * 
 * @param {string} zip_name the full path the file shall have in the ZIP.
 * @param {string} hdd_name the full path to the file to add.
 */
Zip.prototype.AddFile = function (zip_name, hdd_name) { };
/**
 * extract a file from a ZIP.
 * 
 * @param {string} zip_name the full path of the file in the ZIP.
 * @param {string} hdd_name the full path the extracted file should have on the HDD.
 */
Zip.prototype.ExtractFile = function (zip_name, hdd_name) { };
/**
 * get file contents as number array.
 * @param {string} zip_name the full path of the file in the ZIP.
 * @returns {number[]} the content of the file as array of numbers.
 */
Zip.prototype.ReadBytes = function (zip_name) { };
/**
 * Write a bytes to a file in the ZIP.
 * @param {string} zip_name the full path of the file in the ZIP.
 * @param {number[]} data the data to write as array of numbers (must be integers between 0-255).
 */
Zip.prototype.WriteBytes = function (zip_name, data) { };
/**
 * get file contents as ByteArray.
 * @param {string} zip_name the full path of the file in the ZIP.
 * @returns {ByteArray} the content of the file as ByteArray.
 */
Zip.prototype.ReadInts = function (zip_name) { };
/**
* remove file from ZIP.
* @param {string} zip_name the full path of the file to remove from ZIP.
*/
Zip.prototype.DeleteFile = function (zip_name) { };
/**
 * Write a bytes to a file in the ZIP.
 * @param {string} zip_name the full path of the file in the ZIP.
 * @param {ByteArray} data the data to write as ByteArray (must be integers between 0-255).
 */
Zip.prototype.WriteInts = function (zip_name, data) { };
