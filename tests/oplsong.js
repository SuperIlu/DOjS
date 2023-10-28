Include("opl2.js");

var noteDefs = [
	NOTE.A, NOTE.GS, NOTE.AS,
	NOTE.B, NOTE.AS, NOTE.B,
	NOTE.C, NOTE.C, NOTE.CS,
	NOTE.D, NOTE.CS, NOTE.DS,
	NOTE.E, NOTE.DS, NOTE.F,
	NOTE.F, NOTE.E, NOTE.FS,
	NOTE.G, NOTE.FS, NOTE.GS
];

var tuneData = [
	"t150m200o5l8egredgrdcerc<b>er<ba>a<a>agdefefedr4.regredgrdcerc<b>er<ba>a<a>agdedcr4.c<g>cea>cr<ag>cr<gfarfearedgrdcfrc<bagab>cdfegredgrdcerc<b>er<ba>a<a>agdedcr4.cro3c2",
	"m85o3l8crer<br>dr<ar>cr<grbrfr>cr<grbr>crer<gb>dgcrer<br>dr<ar>cr<grbrfr>cr<grbr>ceger4.rfrafergedrfdcrec<br>d<bar>c<agrgd<gr4.o4crer<br>dr<ar>cr<grbrfr>cr<grbr>cege",
	"m85o3l8r4gr4.gr4.er4.err4fr4.gr4.gr4.grr4gr4.er4.er4.frr4gr4>ccr4ccr4<aarraar4ggr4ffr4.ro4gab>dr4.r<gr4.gr4.err4er4.fr4.g"
];

var tempo;
var opl2;
var music = [];

function Setup() {
	tempo = 120.0;

	// Initialize 3 channels of the tune.
	for (var i = 0; i < 3; i++) {
		music.push({
			data: tuneData[i],
			channel: i,
			octave: 4,
			noteDuration: 4.0,
			noteLength: 0.85,
			releaseTime: 0,
			nextNoteTime: MsecTime(),
			index: 0
		});
	}

	opl2 = new OPL2();
	Println("OPL2 = " + opl2.Detect());

	TestSound();

	// Setup channels 0, 1 and 2.
	var piano = opl2.LoadInstrument(INSTRUMENT.PIANO1);
	Println(JSON.stringify(piano));
	for (var i = 0; i < 3; i++) {
		opl2.SetInstrument(i, piano, 1.0);
		opl2.SetBlock(i, 4);
	}

	TestSound();

	Println("NOTE C");
	opl2.PlayNote(1, 1, NOTE.C);
	Sleep(1);
	Println("NOTE OFF");
	opl2.SetKeyOn(1, false);
	// Stop();
}

function TestSound() {
	opl2.Write(0x20, 0x01); // Set the modulator's multiple to 1
	opl2.Write(0x40, 0x10); // Set the modulator's level to about 40 dB
	opl2.Write(0x60, 0xF0); // Modulator attack: quick; decay: long
	opl2.Write(0x80, 0x77); // Modulator sustain: medium; release: medium
	opl2.Write(0xA0, 0x98); // Set voice frequency's LSB (it'll be a D#)
	opl2.Write(0x23, 0x01); // Set the carrier's multiple to 1
	opl2.Write(0x43, 0x00); // Set the carrier to maximum volume (about 47 dB)
	opl2.Write(0x63, 0xF0); // Carrier attack: quick; decay: long
	opl2.Write(0x83, 0x77); // Carrier sustain: medium; release: medium
	opl2.Write(0xB0, 0x31); // Turn the voice on; set the octave and freq MSB
	Sleep(100);
	opl2.Write(0xB0, 0x11); // turn the voice off
}

function Loop() {
	Println(MsecTime());
	Println(JSON.stringify(music));
	for (var i = 0; i < 3; i++) {
		if (MsecTime() >= music[i].releaseTime && opl2.GetKeyOn(music[i].channel)) {
			Println("setKeyOn");
			opl2.SetKeyOn(music[i].channel, false);
		}
		if (MsecTime() >= music[i].nextNoteTime && music[i].data.length > music[i].index) {
			Println("parseTune");
			parseTune(music[i]);
		}
	}
	Sleep(1000);
}

function Input(e) { }

function parseTune(tune) {
	while (tune.index < tune.data.length) {
		// Decrease octave if greater than 1.
		if (tune.data[tune.index] == '<' && tune.octave > 1) {
			tune.octave--;

			// Increase octave if less than 7.
		} else if (tune.data[tune.index] == '>' && tune.octave < 7) {
			tune.octave++;

			// Set octave.
		} else if (tune.data[tune.index] == 'o' && tune.data[tune.index + 1] >= '1' && tune.data[tune.index + 1] <= '7') {
			tune.octave = tune.data[++tune.index] - 48;

			// Set default note duration.
		} else if (tune.data[tune.index] == 'l') {
			tune.index++;
			var duration = parseNumber(tune);
			if (duration != 0) tune.noteDuration = duration;

			// Set note length in percent.
		} else if (tune.data[tune.index] == 'm') {
			tune.index++;
			tune.noteLength = parseNumber(tune) / 100.0;

			// Set song tempo.
		} else if (tune.data[tune.index] == 't') {
			tune.index++;
			tempo = parseNumber(tune);

			// Pause.
		} else if (tune.data[tune.index] == 'p' || tune.data[tune.index] == 'r') {
			tune.index++;
			tune.nextNoteTime = MsecTime() + parseDuration(tune);
			break;

			// Next character is a note A..G so play it.
		} else if (tune.data[tune.index] >= 'a' && tune.data[tune.index] <= 'g') {
			parseNote(tune);
			break;
		}

		tune.index++;
	}
}


function parseNote(tune) {
	// Get index of note in base frequenct table.
	var note = (tune.data[tune.index++] - 97) * 3;
	if (tune.data[tune.index] == '-') {
		note++;
		tune.index++;
	} else if (tune.data[tune.index] == '+') {
		note += 2;
		tune.index++;
	}

	// Get duration, set delay and play note.
	var duration = parseDuration(tune);
	tune.nextNoteTime = MsecTime() + duration;
	tune.releaseTime = MsecTime() + (duration * tune.noteLength);

	opl2.PlayNote(tune.channel, tune.octave, noteDefs[note]);
}


function parseDuration(tune) {
	// Get custom note duration or use default note duration.
	var duration = parseNumber(tune);
	if (duration == 0) {
		duration = 4.0 / tune.noteDuration;
	} else {
		duration = 4.0 / duration;
	}

	// See whether we need to double the duration
	if (tune.data[++tune.index] == '.') {
		duration *= 1.5;
	} else {
		tune.index--;
	}

	// Calculate note duration in ms.
	duration = (60.0 / tempo) * duration * 1000;
	return duration;
}


function parseNumber(tune) {
	var number = 0.0;
	if (tune.data[tune.index] != 0 && tune.data[tune.index] >= '0' && tune.data[tune.index] <= '9') {
		while (tune.data.charAt(tune.index) != 0 && tune.data[tune.index] >= '0' && tune.data[tune.index] <= '9') {
			number = number * 10 + (tune.data[tune.index++] - 48);
		}
		tune.index--;
	}
	return number;
}
