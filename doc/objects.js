/**
 * @type Event
 * @property {number} x mouse X coordinate.
 * @property {number} y mouse X coordinate.
 * @property {number} flags event flags, see {@link MOUSE}
 * @property {number} button mouse buttons, see {@link MOUSE}
 * @property {number} key key code, see {@link KEY}
 * @property {number} kbstat key state, see {@link KEY}
 * @property {number} dtime event time.
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
 * @type MouseLimits
 * @property {number} x1 start x coordinate.
 * @property {number} y1 start y coordinate.
 * @property {number} x2 end x coordinate.
 * @property {number} y2 end y coordinate.
 */
class MouseLimits { }

/**
 * Node addresses are arrays of 6 numbers between 0-255.
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
 * instrument definition.
 * @type FmInstrument
 * @property {number} Modulator.Attack
 * @property {number} Modulator.Decay
 * @property {number} Modulator.Sustain
 * @property {number} Modulator.Release
 * @property {number} Modulator.Vibrato
 * @property {number} Modulator.Tremolo
 * @property {number} Modulator.EGType
 * @property {number} Modulator.KSL
 * @property {number} Modulator.TotalLevel
 * @property {number} Carrier.Attack
 * @property {number} Carrier.Decay
 * @property {number} Carrier.Sustain
 * @property {number} Carrier.Release
 * @property {number} Carrier.Vibrato
 * @property {number} Carrier.EGType
 * @property {number} Carrier.TotalLevel
 * @property {number} Carrier.Multi
 * @property {number} Feedback
 * @property {number} SynType
 */
class FmInstrument { }

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