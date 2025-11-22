/**
 * load C64 SID for playback.
 * 
 * **Note: cRSID module must be loaded by calling LoadLibrary("crsid") before using!**
 * 
 * @see LoadLibrary()
 * 
 * @class
 * 
 * @param {string} filename file name of the SID to load.
 * @param {number} [subtune] SID subtune to play.
 */
function cRSID(filename, subtune) {
	/**
	 * name of file
	 * @member {string}
	 */
	this.filename = null;
	/**
	 * song author
	 * @member {string}
	 */
	this.author = null;
	/**
	 * title of song
	 * @member {string}
	 */
	this.title = null;
	/**
	 * song info
	 * @member {string}
	 */
	this.info = null;

	/**
	 * Song load address
	 * @member {number}
	 */
	this.load_address = 0;
	/**
	 * Song end address
	 * @member {number}
	 */
	this.end_address = 0;
	/**
	 * Song size
	 * @member {number}
	 */
	this.size = 0;
	/**
	 * Song init address
	 * @member {number}
	 */
	this.init_address = 0;
};

/**
 * Close SID after use.
 */
cRSID.prototype.Close = function () { };

/**
 * must be called periodically (e.g. every Loop()) to update the playback buffer.
 */
cRSID.prototype.Play = function () { };
