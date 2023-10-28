/*
MIT License

Copyright (c) 2019-2023 Andre Seidelt <superilu@yahoo.com>

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
Include("evchain");

var CONTROL_PORT = 0x1D0;
var DATA_PORT_LOW = 0x1D1;
var DATA_PORT_HIGH = 0x1D2;
var PICOGUS_PROTOCOL_VER = 1;

var PICO_FIRMWARE_IDLE = 0;
var PICO_FIRMWARE_WRITING = 1;
var PICO_FIRMWARE_ERROR = 0xFF;

var THE_NAME = "PicoGUiS";

var PICOGUS_COLOR = Color(82, 83, 193);

var GUS_COLOR = Color(217, 103, 69);
var GUS_COLOR2 = Color(253, 83, 49);

var FW_IMAGES = [
	{ name: "Gravis Ultrasound (GUS)", fname: "PG-GUS.UF2" },
	{ name: "AdLib (OPL2)", fname: "PG-ADLIB.UF2" },
	{ name: "MPU-401 (with intelligent mode)", fname: "PG-MPU.UF2" },
	{ name: "Tandy 3-voice", fname: "PG-TANDY.UF2" },
	{ name: "CMS/Game Blaster", fname: "PG-CMS.UF" }
];

var overlay = null;
var f_big = null;
var f_small = null;
var ev_chain = null;
var upd_context = null;
var gus_mode = 0;
var gus_port = 0;
var gus_irq = "<n/a>";
var gus_dma = "<n/a>";
var gus_fiwa = "DUMMY";

function Setup() {
	MouseShowCursor(false);
	f_big = new Font(JSBOOTPATH + "fonts/lucs40bi.fnt");
	f_small = new Font(JSBOOTPATH + "fonts/pc8x8.fnt");

	if (!Detect()) {
		overlay = [
			function () {
				// "00000000011111111112222222222333333333344444444445555555555666\n" +
				// "12345678901234567890123456789012345678901234567890123456789012\n" +
				TextOverlay(
					"\n\n\n" +
					"\n\n\n" +
					"\n\n\n" +
					CenterLine("!!! PicoGUS NOT detected !!!") +
					"\n\n\n" +
					CenterLine("Press ANY KEY to quit"), EGA.RED);
			},
			function (e) {
				if (e.key != -1) {
					Stop();
				}
			}
		];
	} else {
		if (!CheckProtocol()) {
			overlay = [
				function () {
					TextOverlay(
						"\n\n\n" +
						"\n\n\n" +
						"\n\n\n" +
						CenterLine("!!! Wrong PicoGUS protocol version !!!") +
						"\n\n\n" +
						CenterLine("Press ANY KEY to quit"), EGA.RED);
				},
				function (e) {
					if (e.key != -1) {
						Stop();
					}
				}
			];
		} else {
			gus_mode = GetMode();
			gus_port = GetPort();
			gus_fiwa = GetFirmwareString();

			var usnd = GetEnv("ULTRASND");
			var uparts = usnd.split(",")
			if (uparts.length === 5) {
				gus_irq = uparts[1];
				gus_dma = uparts[3];
			} else {
				gus_irq = "<n/a>";
				gus_dma = "<n/a>";
			}
		}
	}


	ev_chain = new EvalChain();
	CreateMenu();
}

function DrawHeader() {
	var y = 50;

	y = DrawBoxedText(10, y, Width - 20, "PicoGUS detected: " + gus_fiwa, EGA.BLACK);
	y += 10;

	var valSpace = (Width - 40) / 3;
	var newY = DrawBoxedText(1 * 10 + 0 * valSpace, y, valSpace, "IO: 0x" + gus_port.toString(16), EGA.BLACK);

	if (gus_mode == 0) {
		DrawBoxedText(2 * 10 + 1 * valSpace, y, valSpace, "IRQ: " + gus_irq, EGA.BLACK);
		DrawBoxedText(3 * 10 + 2 * valSpace, y, valSpace, "DMA: " + gus_dma, EGA.BLACK);
	}
	y = newY + 10;

	y = DrawBoxedText(10, y, Width - 20, "Mode: " + FW_IMAGES[gus_mode].name, EGA.BLACK);
	y += 10;

	var valSpace = (Width - 30) / 2;
	DrawVar(1 * 10 + 0 * valSpace, y, valSpace, "ULTRASND=", "ULTRASND");
	DrawVar(2 * 10 + 1 * valSpace, y, valSpace, "ULTRADIR=", "ULTRADIR");

	return y;
}

function Loop() {
	ClearScreen(GUS_COLOR2);
	f_big.DrawStringCenter(Width / 2, 10, THE_NAME, PICOGUS_COLOR, NO_COLOR);
	DrawText(2, 2, "PicoGUS GUI v0.1", PICOGUS_COLOR);

	if (overlay) {
		overlay[0]();
	} else {
		DrawHeader();
		// only draw menu when there is nothing done in the chain
		if (!ev_chain.Step()) {
			DrawMenu();
		} else {
			DrawProgress(upd_context.msg, upd_context.length - ev_chain.Size(), upd_context.length);
		}
	}
}

function Input(e) {
	if (overlay) {
		overlay[1](e);
	} else if (ev_chain.Size == 0) {
		var key = e.key & 0xFF;
		var keyCode = e.key >> 8;
		var char = String.fromCharCode(key);


		switch (keyCode) {
			case KEY.Code.KEY_DOWN:
				if (men_selected < men_items.length - 1) {
					men_selected++;
				}
				break;
			case KEY.Code.KEY_UP:
				if (men_selected >= 1) {
					men_selected--;
				}
				break;
			case KEY.Code.KEY_PGDN:
				men_selected = men_items.length - 1;
				break;
			case KEY.Code.KEY_PGUP:
				men_selected = 0;
				break;
			case KEY.Code.KEY_ENTER:
				men_items[men_selected][1]();
				break;
		}
	}
}

var men_selected = 0;
var men_items = null;

function CreateMenu() {
	// make exit
	men_items = [
		["Exit", function () { Stop(); }],
	];

	if (gus_mode != 0) {
		men_items.push(["Set base address", function () { SetBaseAddress(); }]);
	} else {
		men_items.push(["Set audio buffer size", function () { SetBufferSize(); }]);
		men_items.push(["Set DMA interval", function () { SetDmaInterval(); }]);
	}

	// add all possible FW images
	for (var i = 0; i < FW_IMAGES.length; i++) {
		if (i !== gus_mode) {
			men_items.push(
				[
					"Switch PicoGUS to " + FW_IMAGES[i].name,
					CreateCallback(FW_IMAGES[i].fname)
				]);
		}
	}

	men_selected = 0;
}

function SetBaseAddress() {
	var defPort = 0;
	var name = "HURZ";
	switch (gus_mode) {
		case 0: // should not happen
			defPort = 0x240;
			name = "GUS";
			break;
		case 1:
			defPort = 0x388;
			name = "AdLib";
			break;
		case 2:
			defPort = 0x330;
			name = "MPU-401";
			break;
		case 3:
			defPort = 0x2c0;
			name = "Tandy";
			break;
		case 4:
			defPort = 0x220;
			name = "CMS";
			break;
	}

	var minVal = 0x200;
	var maxVal = 0x3ff;
	overlay = [
		function () {
			var y = DrawHeader();

			var txt = name + " port = " + overlay[2].toString(16) + "h\n" +
				"\n" +
				"UP/DOWN to change\n" +
				"PG_DOWN/PG_UP for min/max\n" +
				"ENTER to accept";
			var col = EGA.YELLOW;

			var fntSize = f_small.height;
			var xStart = 20 * fntSize;
			var yStart = 20 * fntSize;
			var xEnd = Width - 20 * fntSize;
			var yEnd = Height - 20 * fntSize;
			FilledBox(xStart, yStart, xEnd, yEnd, Color(32));
			Box(xStart, yStart, xEnd, yEnd, EGA.LIGHT_BLUE);
			DisplayMultilineText(xStart + fntSize, yStart + fntSize, col, txt);
		},
		function (e) {
			var keyCode = e.key >> 8;
			switch (keyCode) {
				case KEY.Code.KEY_UP:
					if (overlay[2] < maxVal) {
						overlay[2]++;
					}
					break;
				case KEY.Code.KEY_DOWN:
					if (overlay[2] > minVal) {
						overlay[2]--;
					}
					break;
				case KEY.Code.KEY_PGDN:
					overlay[2] = minVal;
					break;
				case KEY.Code.KEY_PGUP:
					overlay[2] = maxVal;
					break;
				case KEY.Code.KEY_ENTER:
					SetPort(overlay[2]);
					gus_port = GetPort();
					overlay = null;
					break;
			}
		},
		defPort
	];
}

function SetBufferSize() {
	var minVal = 8;
	var maxVal = 255;
	overlay = [
		function () {
			var y = DrawHeader();

			var txt = "Sample buffer size = " + overlay[2] + " samples\n" +
				"\n" +
				"UP/DOWN to change\n" +
				"PG_DOWN/PG_UP for min/max\n" +
				"ENTER to accept";
			var col = EGA.YELLOW;

			var fntSize = f_small.height;
			var xStart = 20 * fntSize;
			var yStart = 20 * fntSize;
			var xEnd = Width - 20 * fntSize;
			var yEnd = Height - 20 * fntSize;
			FilledBox(xStart, yStart, xEnd, yEnd, Color(32));
			Box(xStart, yStart, xEnd, yEnd, EGA.LIGHT_BLUE);
			DisplayMultilineText(xStart + fntSize, yStart + fntSize, col, txt);
		},
		function (e) {
			var keyCode = e.key >> 8;
			switch (keyCode) {
				case KEY.Code.KEY_UP:
					if (overlay[2] < maxVal) {
						overlay[2]++;
					}
					break;
				case KEY.Code.KEY_DOWN:
					if (overlay[2] > minVal) {
						overlay[2]--;
					}
					break;
				case KEY.Code.KEY_PGDN:
					overlay[2] = minVal;
					break;
				case KEY.Code.KEY_PGUP:
					overlay[2] = maxVal;
					break;
				case KEY.Code.KEY_ENTER:
					OutPortByte(CONTROL_PORT, 0x10); // Select audio buffer register
					OutPortByte(DATA_PORT_HIGH, (overlay[2] - 1));
					overlay = null;
					break;
			}
		},
		16
	];
}

function SetDmaInterval() {
	var minVal = 0;
	var maxVal = 255;
	overlay = [
		function () {
			var y = DrawHeader();

			var txt = "DMA interval = " + overlay[2] + " us\n" +
				"\n" +
				"UP/DOWN to change\n" +
				"PG_DOWN/PG_UP for min/max\n" +
				"ENTER to accept";
			var col = EGA.YELLOW;

			var fntSize = f_small.height;
			var xStart = 20 * fntSize;
			var yStart = 20 * fntSize;
			var xEnd = Width - 20 * fntSize;
			var yEnd = Height - 20 * fntSize;
			FilledBox(xStart, yStart, xEnd, yEnd, Color(32));
			Box(xStart, yStart, xEnd, yEnd, EGA.LIGHT_BLUE);
			DisplayMultilineText(xStart + fntSize, yStart + fntSize, col, txt);
		},
		function (e) {
			var keyCode = e.key >> 8;
			switch (keyCode) {
				case KEY.Code.KEY_UP:
					if (overlay[2] < maxVal) {
						overlay[2]++;
					}
					break;
				case KEY.Code.KEY_DOWN:
					if (overlay[2] > minVal) {
						overlay[2]--;
					}
					break;
				case KEY.Code.KEY_PGDN:
					overlay[2] = minVal;
					break;
				case KEY.Code.KEY_PGUP:
					overlay[2] = maxVal;
					break;
				case KEY.Code.KEY_ENTER:
					OutPortByte(CONTROL_PORT, 0x11); // Select DMA interval register
					OutPortByte(DATA_PORT_HIGH, overlay[2]);
					overlay = null;
					break;
			}
		},
		0
	];
}

function CreateCallback(img) {
	return function () {
		try {
			WriteFirmware(img);
		} catch (e) {
			var err = new String(e).replace(/\t/g, "  ").replace(": No", "\nNo");
			overlay = [
				function () {
					// "00000000011111111112222222222333333333344444444445555555555666\n" +
					// "12345678901234567890123456789012345678901234567890123456789012\n" +
					TextOverlay(
						"\n\n\n" +
						"\n\n\n" +
						"\n\n\n" +
						CenterLine(err) +
						"\n\n\n" +
						CenterLine("Press ANY KEY to quit"), EGA.RED);
				},
				function (e) {
					if (e.key != -1) {
						overlay = null;
					}
				}
			];
		}
	};
}

function DrawMenu() {
	var w = Width - 20;
	var h = men_items.length * TXT_SIZE + 20;
	var x = 10;
	var y = Height / 2;

	Box(x, y, x + w, y + h, EGA.BLUE);  // draw border

	for (var i = 0; i < men_items.length; i++) {
		if (i === men_selected) {
			DrawText(x + 10, y + 10 + i * TXT_SIZE, men_items[i][0], EGA.YELLOW);
		} else {
			DrawText(x + 10, y + 10 + i * TXT_SIZE, men_items[i][0], EGA.BLACK);
		}
	}
}

function CenterLine(txt) {
	var missing = (62 - txt.length) / 2;
	var fill = "";
	while (fill.length < missing) {
		fill += " ";
	}

	return fill + txt + "\n";
}


/**
 * Read a 32 bit unsigned, little endian number from an ByteArray.
 * 
 * @param {ByteArray} data the data to read from
 * @param {Number} pos The position to start reading from.
 * 
 * @returns {Number} the resulting 32bit number
 */
function ReadUint32LE(data, pos) {
	return (data.Get(pos + 3) * 0x1000000) + (data.Get(pos + 2) * 0x10000) + (data.Get(pos + 1) * 0x100) + (data.Get(pos));
}

function DrawProgress(txt, curVal, maxVal) {
	var w = Width - 20;
	var h = 24;
	var x = 10;
	var y = Height - h - 10;

	Box(x, y, x + w, y + h, EGA.BLUE);  // draw border

	DrawText(x + 10, y + 2, txt, EGA.BLACK);

	FilledBox(x + 10, y + 10, x + 10 + curVal * (w - 20) / maxVal, y + 20, EGA.LIGHT_BLUE);

	return y;
}

function DrawVar(x, y, w, txt, varname) {
	var newY = y + 12;

	var val = GetEnv(varname);

	Box(x, y, x + w, newY, EGA.BLUE);
	DrawText(x + 10, y + 3, txt, EGA.BLACK);

	var txt_w = f_small.StringWidth(txt);

	if (val == null) {
		DrawText(x + 10 + txt_w, y + 3, "<NOT SET>", EGA.RED);
	} else {
		DrawText(x + 10 + txt_w, y + 3, val, Color(0, 120, 0, 255));
	}

	return newY;
}

function DrawBoxedText(x, y, w, txt, col) {
	var newY = y + 12;
	Box(x, y, x + w, newY, EGA.BLUE);
	f_small.DrawStringCenter(x + w / 2, y + 3, txt, col, NO_COLOR);

	return newY;
}

function DrawText(x, y, txt, col) {
	f_small.DrawStringLeft(x, y, txt, col, NO_COLOR);
}

function TextOverlay(txt, col) {
	var fntSize = f_small.height;

	var xStart = 8 * fntSize;
	var yStart = 8 * fntSize;
	var xEnd = Width - 8 * fntSize;
	var yEnd = Height - 8 * fntSize;

	FilledBox(xStart, yStart, xEnd, yEnd, Color(32));
	Box(xStart, yStart, xEnd, yEnd, EGA.LIGHT_BLUE);

	DisplayMultilineText(xStart + fntSize, yStart + fntSize, col, txt);
}

var TXT_SIZE = 9;       // text height

function DisplayMultilineText(x, y, col, txt) {
	var l;
	var yPos;
	var txt_lines = txt.split('\n');

	// draw the text
	for (l = 0; l < txt_lines.length; l++) {
		yPos = l * TXT_SIZE;
		f_small.DrawStringLeft(x, y + yPos, txt_lines[l], col, NO_COLOR);
	}

	// return current poition on screen
	return y + yPos + TXT_SIZE;
}

function GetFirmwareString() {
	OutPortByte(CONTROL_PORT, 0xCC); // Knock on the door...
	OutPortByte(CONTROL_PORT, 0x02); // Select firmware string register

	var firmware_string = "";
	for (var i = 0; i < 255; ++i) {
		var ch = InPortByte(DATA_PORT_HIGH);
		if (ch > 0) {
			firmware_string += String.fromCharCode(ch);
		} else {
			break;
		}
	}
	return firmware_string;
}

function Detect() {
	// Get magic value from port on PicoGUS that is not on real GUS
	OutPortByte(CONTROL_PORT, 0xCC); // Knock on the door...
	OutPortByte(CONTROL_PORT, 0x00); // Select magic string register
	if (InPortByte(DATA_PORT_HIGH) != 0xDD) {
		return false;
	} else {
		return true
	}
}

function InitGus(port) {
	OutPortByte(CONTROL_PORT, 0x04); // Select port register
	OutPortWord(DATA_PORT_LOW, port); // Write port

	// Detect if there's something GUS-like...
	// Set memory address to 0
	OutPortByte(port + 0x103, 0x43);
	OutPortWord(port + 0x104, 0x0);
	OutPortByte(port + 0x103, 0x44);
	OutPortWord(port + 0x104, 0x0);
	// Write something
	OutPortByte(port + 0x107, 0xDD);
	// Read it and see if it's the same
	if (InPortByte(port + 0x107) != 0xDD) {
		return false;
	}

	// Enable IRQ latches
	OutPortByte(port, 0x8);
	// Select reset register
	OutPortByte(port + 0x103, 0x4C);
	// Master reset to run. DAC enable and IRQ enable will be done by the application.
	OutPortByte(port + 0x105, 0x1);

	return true;
}

function CheckProtocol() {
	OutPortByte(CONTROL_PORT, 0x01); // Select protocol version register
	var protocol_got = InPortByte(DATA_PORT_HIGH);
	Println("PGUS: Protocol is " + protocol_got);
	if (PICOGUS_PROTOCOL_VER != protocol_got) {
		return false;
	} else {
		return true;
	}
}

function GetMode() {
	if (!Detect()) {
		throw new Error("PGUS: PicoGUS not detected!");
	}

	OutPortByte(CONTROL_PORT, 0x03); // Select mode register
	return InPortByte(DATA_PORT_HIGH);
}

function GetPort(port) {
	OutPortByte(CONTROL_PORT, 0x04); // Select port register
	return InPortWord(DATA_PORT_LOW); // Get port
}

function SetPort(port) {
	var mode = GetMode();

	if (mode != 0) {
		OutPortByte(CONTROL_PORT, 0x04); // Select port register
		OutPortWord(DATA_PORT_LOW, port); // Write port
	}
}

function WriteFirmware(filename) {
	var magicStart0 = 0;
	var magicStart1 = 4;
	var flags = 8;
	var targetAddr = 12;
	var payloadSize = 16;
	var blockNo = 20;
	var numBlocks = 24;
	var fileSize = 28; // or familyID;
	var data = 32;
	var magicEnd = 32 + 476;

	var minSteps = 4;

	upd_context = {};

	// read file
	ev_chain.Add(function () { upd_context.msg = "Reading " + filename + "..."; });
	ev_chain.Add(function () {
		try {
			var f = new File(filename, FILE.READ);
			upd_context.data = f.ReadInts();
			f.Close();
		} catch (e) {
			UpdateError(e);
		}
	});

	// check size
	ev_chain.Add(function () { upd_context.msg = "Checking size" + filename + "..."; });
	ev_chain.Add(function () {
		if (upd_context.data.length < 512) {
			UpdateError(new Error("ERROR: file " + filename + " is not a valid UF2 file - too short"));
		}
	});

	// check magic
	ev_chain.Add(function () { upd_context.msg = "Checking magic" + filename + "..."; });
	ev_chain.Add(function () {
		if ((ReadUint32LE(upd_context.data, magicStart0) != 0x0A324655) || (ReadUint32LE(upd_context.data, magicStart1) != 0x9E5D5157) || (ReadUint32LE(upd_context.data, magicEnd) != 0x0AB16F30)) {
			UpdateError(new Error("ERROR: file " + filename + " is not a valid UF2 file - bad magic"));
		}
	});

	// check size
	ev_chain.Add(function () { upd_context.msg = "Initializing PicoGUS"; });
	ev_chain.Add(function () {
		upd_context.numBlocks = ReadUint32LE(upd_context.data, numBlocks);

		// Put card into programming mode
		OutPortByte(CONTROL_PORT, 0xCC); // Knock on the door...
		OutPortByte(CONTROL_PORT, 0xFF); // Select firmware programming mode
		if (InPortByte(DATA_PORT_HIGH) != PICO_FIRMWARE_IDLE) {
			UpdateError(new Error("ERROR: Card is not in programming mode?"));
		}
	});

	// update chain
	ev_chain.Add(function () { upd_context.msg = "FLASHing " + filename; });
	ev_chain.Add(function () {
		upd_context.pos = 0;
		for (var i = 0; i < upd_context.numBlocks; i++) {
			ev_chain.Add(CreateFlashBlock(i));
		}
		// wait
		ev_chain.Add(function () { upd_context.msg = "Programming complete. Waiting for the card to reboot..."; });
		ev_chain.Add(function () {
			Sleep(2000);
			if (InPortByte(DATA_PORT_HIGH) != 0xDD) {
				UpdateError(new Error("ERROR: card is not alive after programming firmware"));
			}
		});

		// update info
		ev_chain.Add(function () { upd_context.msg = "Updating card info...."; });
		ev_chain.Add(function () {
			gus_mode = GetMode();
			gus_port = GetPort();
			gus_fiwa = GetFirmwareString();
			CreateMenu();
		});
		// update max number of steps
		upd_context.length = ev_chain.Size();
	});

	// remember max number of steps
	upd_context.length = ev_chain.Size();
}

function CreateFlashBlock(i) {
	var blockNo = 20;
	return function () {
		if (i != ReadUint32LE(upd_context.data, upd_context.pos + blockNo)) {
			upd_context.error = new Error("ERROR: file " + filename + " is not a valid UF2 file - block mismatch");
			ev_chain.Clear();
		}

		for (var b = 0; b < 512; ++b) {
			// Write firmware byte
			OutPortByte(DATA_PORT_HIGH, upd_context.data.Get(upd_context.pos + b));
			if (i < (upd_context.numBlocks - 1) || b < 511) { // If it's not the very last byte
				if (InPortByte(DATA_PORT_HIGH) != PICO_FIRMWARE_WRITING) {
					upd_context.error = new Error("ERROR: Card is not in firmware writing mode?");
					ev_chain.Clear();
				}
			}
		}

		upd_context.pos += 512;
	};
}

function UpdateError(e) {
	upd_context.error = e;
	ev_chain.Clear();

	var err = new String(e).replace(/\t/g, "  ").replace(": No", "\nNo");
	overlay = [
		function () {
			TextOverlay(
				"\n\n\n" +
				"\n\n\n" +
				"\n\n\n" +
				CenterLine(err) +
				"\n\n\n" +
				CenterLine("Press ANY KEY to quit"), EGA.RED);
		},
		function (e) {
			if (e.key != -1) {
				overlay = null;
			}
		}
	];
}
