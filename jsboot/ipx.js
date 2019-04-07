/*
MIT License

Copyright (c) 2019 Andre Seidelt <superilu@yahoo.com>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:
The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

/**
 * IPX definitions.
 * @namespace IPX
 * @property {*} DEFAULT_SOCKET default socket number for DOjS.
 * @property {*} BROADCAST broadcast address
 */
IPX = {
	DEFAULT_SOCKET: 0x2342,
	BROADCAST: [0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF]
};

/** @module ipx */

/**
 * Convert a node address to a string.
 * 
 * @param {IpxAddress} addr a node address.
 */
function IpxAddressToString(addr) {
	if (addr.length != 6) {
		throw "Node address length does not match";
	}

	return addr[0].toString(16) + ":" +
		addr[1].toString(16) + ":" +
		addr[2].toString(16) + ":" +
		addr[3].toString(16) + ":" +
		addr[4].toString(16) + ":" +
		addr[5].toString(16);
}

/**
 * Convert an address in hex-string notation back to an JS array.
 * 
 * @param {*} addr a string of 6 hex numbers separated by ':'.
 * @returns {IpxAddress} An array of six numbers.
 */
function IpxStringToAddress(addr) {
	var parts = addr.split(":");
	if (parts.length != 6) {
		throw "Node address length does not match";
	}

	return [
		parseInt(parts[0], 16),
		parseInt(parts[1], 16),
		parseInt(parts[2], 16),
		parseInt(parts[3], 16),
		parseInt(parts[4], 16),
		parseInt(parts[5], 16)
	];
}

/**
 * discover nodes on the network.
 * 
 * @param {integer} num total number of nodes to search for (including the local node).
 * @param {IpxAddress[]} nodes an array to store the discovered nodes in.
 * 
 * @returns {boolean} true if the wanted number of nodes was discovered, else false. The nodes array will contain the addresses of all found nodes.
 */
function IpxFindNodes(num, nodes) {
	var searchAddress = function (nodes, addr) {
		var addrStr = IpxAddressToString(addr);
		for (var i = 0; i < nodes.length; ++i) {
			var str = IpxAddressToString(nodes[i]);
			if (str === addrStr) {
				return true;
			}
		}
		return false;
	};

	if (nodes.length < num - 1) {
		IpxSend("HELLO", IPX.BROADCAST);

		if (IpxCheckPacket()) {
			var pck = IpxGetPacket();
			if (pck.data === "HELLO") {
				if (searchAddress(nodes, pck.source)) {
					Debug("Node " + JSON.stringify(pck.source) + " already known.");
				} else {
					nodes.push(pck.source);
					Debug("Node found " + JSON.stringify(pck.source));
				}
			}
		}
	} else {
		return true;
	}
}

/**
 * Send data to all nodes in array.
 * 
 * @param {string} data the data to send
 * @param {IpxAddress[]} nodes the node addresses to send to.
 */
function IpxAllNodes(data, nodes) {
	for (var i = 0; i < nodes.length; i++) {
		IpxSend(data, nodes[i]);
	}
}
