/**
 * @typedef {object} Event
 * @property {number} x mouse X coordinate.
 * @property {number} y mouse X coordinate.
 * @property {number} buttons mouse buttons, see {@link MOUSE}
 * @property {number} key key code, see {@link KEY}
 * @property {number} ticks event time.
 */
class Event { }

/**
 * An object with coordinates of a drawn arc.
 * @typedef {object} ArcInfo
 * @property {number} centerX center x coordinate of the arc.
 * @property {number} centerY center y coordinate of the arc.
 * @property {number} startX x coordinate of the arc start.
 * @property {number} startY y coordinate of the arc start.
 * @property {number} endX x coordinate of the arc end.
 * @property {number} endY y coordinate of the arc end.
 */
class ArcInfo { }

/**
 * Node addresses are arrays of 6 numbers between 0-255 (e.g. [1, 2, 3, 4, 5, 6]).
 * @typedef {object} IpcAddress
 */
class IpxAddress { }

/**
 * received IPX data packet.
 * @typedef {object} IpxPacket
 * @property {string} data the received data.
 * @property {IpxAddress} source address of the sending node.
 */
class IpxPacket { }

/**
 * @typedef {object} StatInfo
 * @property {string} atime file access timestamp.
 * @property {string} ctime file creation time.
 * @property {string} mtime file modification time.
 * @property {number} blksize file size in blocks.
 * @property {number} size file size in bytes.
 * @property {number} nlink number of sub entries.
 * @property {string} drive drive letter for the entry.
 * @property {boolean} is_blockdev true if this is a block device.
 * @property {boolean} is_chardev true if this is a character device.
 * @property {boolean} is_directory true if this is a directory.
 * @property {boolean} is_regular true if this is a regular file.
 */
class StatInfo { }

/**
 * @typedef {object} MemInfo
 * @property {number} total total amount of memory in the system.
 * @property {number} remaining number of available bytes.
 */
class MemInfo { }

/**
 * @typedef {object} Matrix
 * @property {number[][]} v the 3x3 matrix data.
 * @property {number[]} t the translation data.
 */
class Matrix { }

/**
 * A vertex with x, y, z, u, v and c represented as an array of six numbers (e.g. [1, 1, 1, 0, 0, EGA.BLACK])
 * @type V3D
 */
class V3D { }

/**
 * complex object containing the state information for all buttons and sticks.
 * @example
{
	"analogue": true,
	"buttons": [
		{ "name": "B1", "state": false },
		{ "name": "B2", "state": false },
		{ "name": "B3", "state": false },
		{ "name": "B4", "state": false },
		{ "name": "B5", "state": false },
		{ "name": "B6", "state": false },
		{ "name": "B7", "state": false },
		{ "name": "B8", "state": false },
		{ "name": "B9", "state": false }
	],
	"calib_analogue": false,
	"calib_digital": false,
	"calibrate": false,
	"digital": false,
	"signed": true,
	"sticks": [
		{
			"analogue": true,
			"axis": [
				{ "d1": 0, "d2": 0, "name": "X", "pos": 0 },
				{ "d1": 0, "d2": 0, "name": "Y", "pos": 0 }
			],
			"calib_analogue": false,
			"calib_digital": false,
			"calibrate": false,
			"digital": false,
			"name": "Stick",
			"signed": true,
			"unsigned": false
		},
		{
			"analogue": true,
			"axis": [
				{ "d1": 0, "d2": 0, "name": "", "pos": 0 }
			],
			"calib_analogue": false,
			"calib_digital": false,
			"calibrate": false,
			"digital": false,
			"name": "Twist",
			"signed": true,
			"unsigned": false
		},
		{
			"analogue": true,
			"axis": [
				{ "d1": 0, "d2": 0, "name": "", "pos": 0 }
			],
			"calib_analogue": false,
			"calib_digital": false,
			"calibrate": false,
			"digital": false,
			"name": "Throttle",
			"signed": true,
			"unsigned": false
		},
		{
			"analogue": false,
			"axis": [
				{ "d1": 0, "d2": 0, "name": "X", "pos": 0 },
				{ "d1": 0, "d2": 0, "name": "Y", "pos": 0 }
			],
			"calib_analogue": false,
			"calib_digital": false,
			"calibrate": false,
			"digital": true,
			"name": "Hat",
			"signed": true,
			"unsigned": false
		}
	],
	"unsigned": false
}
 */
class JoyInfo { }

/**
 * An array with four numbers (e.g. [192.168.1.2]).
 * @typedef {object} IpAdddress
 */
class IpAddress { }

/**
 * An array with the HTTP result: [code, message, header, data]
 * @typedef {object} HTTPResult
 */
class HTTPResult { }

/**
 * complex object containing the state information for all buttons and sticks.
 * "-1" means "not available"
 * @example
{
	"brand_str": "",
	"cpu_clock": 55,
	"cpu_clock_measure": 62,
	"cpu_codename": "i486 DX-25/33",
	"ext_family": 4,
	"ext_model": 0,
	"family": 4,
	"features": [
		"fpu"
	],
	"l1_data_assoc": -1,
	"l1_data_cache": -1,
	"l1_data_cacheline": -1,
	"l1_instruction_assoc": -1,
	"l1_instruction_cache": -1,
	"l1_instruction_cacheline": -1,
	"l2_assoc": -1,
	"l2_cache": -1,
	"l2_cacheline": -1,
	"l3_assoc": -1,
	"l3_cache": -1,
	"l3_cacheline": -1,
	"l4_assoc": -1,
	"l4_cache": -1,
	"l4_cacheline": -1,
	"model": 0,
	"num_cores": 1,
	"num_logical_cpus": 1,
	"sse_size": -1,
	"stepping": 2,
	"total_logical_cpus": 1,
	"vendor": 0,
	"vendor_str": "GenuineIntel"
}
 * @typedef {object} CpuInfo
 */
class CpuInfo { }
