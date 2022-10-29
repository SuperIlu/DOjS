/**
 * Open an INI file.
 * @class
 * 
 * @param {string} filename the name of the file.
 */
function IniFile(filename) {
	/**
	 * Name of the file.
	 * @member {string}
	 */
	this.filename = null;
}
/**
 * get a key from a section of the INI.
 * 
 * @param {string} [section] the section to search for the key, can be omitted.
 * @param {string} key the key to search.
 * 
 * @returns {string} the value if found or null.
 */
IniFile.prototype.Get = function (section, key) { };
