/**
 * Read EXIF tags.
 * 
 * **Note: EXIF module must be loaded by calling LoadLibrary("exif") before using!**
 * 
 * @see LoadLibrary()
 * 
 * @class
 * 
 * @param {string} filename the name of the file.
 */
function Exif(filename) {
	/**
	 * Name of the file.
	 * @member {string}
	 */
	this.filename = null;
}
/**
 * get all tags as key->value mapping
 * 
 * @returns {object} all exif tags and their values.
 */
Exif.prototype.GetTags = function () { };

/**
 * close file and free ressoucres.
 */
Exif.prototype.Close = function () { };
