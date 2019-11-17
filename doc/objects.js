/**
 * @type Event
 * @property {number} x mouse X coordinate.
 * @property {number} y mouse X coordinate.
 * @property {number} buttons mouse buttons, see {@link MOUSE}
 * @property {number} key key code, see {@link KEY}
 * @property {number} ticks event time.
 */
class Event { }

/**
 * An object with coordinates of a drawn arc.
 * @type ArcInfo
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
 * @type IpcAddress
 */
class IpxAddress { }

/**
 * received IPX data packet.
 * @type IpxPacket
 * @property {string} data the received data.
 * @property {IpxAddress} source address of the sending node.
 */
class IpxPacket { }

/**
 * @type StatInfo
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
 * @type MemInfo
 * @property {number} total total amount of memory in the system.
 * @property {number} remaining number of available bytes.
 */
class MemInfo { }

/**
 * @type Matrix
 * @property {number[][]} v the 3x3 matrix data.
 * @property {number[]} t the translation data.
 */
class Matrix { }

/**
 * A vertex with x, y, z, u, v and c represented as an array of six numbers (e.g. [1, 1, 1, 0, 0, EGA.BLACK])
 * @type V3D
 */
class V3D { }
