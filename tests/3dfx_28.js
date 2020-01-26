Include('tests/3dfx_tst.js');
Println("planar polygon test");

var wrange;

var NHUE = 360;
var NVERT = 5;

var hues = [];
var vtx = [];

function FUDGE(x) { return ((x) * 255.0); }

function PHASE(x, y, m) { return (((x) + (y)) % (m)); }

function value(n1, n2, hue) {
	var retval;

	if (hue > 360.0) hue -= 360.0;
	if (hue < 0.0) hue += 360.0;

	if (hue < 60.0) {
		retval = n1 + (n2 - n1) * hue / 60.0;
	} else if (hue < 180.0) {
		retval = n2;
	} else if (hue < 240.0) {
		retval = n1 + (n2 - 1.0) * (240.0 - hue) / 60.0;
	} else {
		retval = n1;
	}
	if (retval < 0.0) retval = 0.0;
	if (retval > 1.0) retval = 1.0;
	return FUDGE(retval);
}

function hlsToRGB(h, l, s) {
	var p1, p2;

	p2 = (l <= 0.5) ? l * (1.0 + s) : l + s - l * s;
	p1 = 2 * l - p2;

	if (s == 0.0) {
		return [l, l, l];
	} else {
		return [
			value(p1, p2, h + 120.0),
			value(p1, p2, h),
			value(p1, p2, h - 120.0)
		];
	}
}

/*
** This function is called once when the script is started.
*/
function Setup() {
	fxInit();
	fxVertexLayout([GR_PARAM.XY, GR_PARAM.RGB]);
	createVertices();

	wrange = fxGetWDepthMinMax();

	fxColorCombine(
		GR_COMBINE_FUNCTION.LOCAL,
		GR_COMBINE_FACTOR.NONE,
		GR_COMBINE_LOCAL.ITERATED,
		GR_COMBINE_OTHER.NONE,
		false);

	/* generate a equilateral polygon */
	for (var i = 0; i < NVERT; i++) {
		var theta = 2.0 * Math.PI * i / NVERT;

		vtx.push([
			scaleX((Math.cos(theta) / 4.0) + 0.5),
			scaleY((Math.sin(theta) / 4.0) + 0.5),
			0, 0, 0]);
	}


	/* init a table of hues */
	for (var i = 0; i < NHUE; i++) {
		var theta = i * 360.0 / NHUE;
		hues.push(hlsToRGB(theta, 0.4, 0.5));
	}

	/* assign hues to vertices */
	for (var i = 0; i < NVERT; i++) {
		vtx[i][2] = hues[PHASE(0, i * (NHUE / NVERT), NHUE)][0];
		vtx[i][3] = hues[PHASE(0, i * (NHUE / NVERT), NHUE)][1];
		vtx[i][4] = hues[PHASE(0, i * (NHUE / NVERT), NHUE)][2];
	}
	/*
	 * Force polygon RGB values to be planar... note overflow!
	 * this is deliberate as a sanity check
	 */
	vtx[3][2] = 235.519; vtx[3][3] = 51.001; vtx[3][4] = 115.721;
	vtx[4][2] = 298.559; vtx[4][3] = -12.039; vtx[4][4] = 91.0;
}

/*
** This function is repeatedly until ESC is pressed or Stop() is called.
*/
function Loop() {
	fxBufferClear(0x00, 0, wrange[1]);

	fxDrawVertexArray(GR_VERTEX.POLYGON, vtx);
}

function Input(e) { }
