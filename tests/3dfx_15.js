Include('tests/3dfx_tst.js');
Println("clip rectangle testing");

var zrange;

var clipX = 0.2;
var clipY = 0.5;
var clipSize = 0.3;

var clipSizeDelta = 0.005;
var clipPosDelta = 0.01;

var CLIPSIZE_MIN = 0.05;
var CLIPSIZE_MAX = 0.6;

/*
** This function is called once when the script is started.
*/
function Setup() {
	fxInit();
	fxVertexLayout([GR_PARAM.XY]);
	createVertices();

	fxColorCombine(
		GR_COMBINE_FUNCTION.LOCAL,
		GR_COMBINE_FACTOR.NONE,
		GR_COMBINE_LOCAL.CONSTANT,
		GR_COMBINE_OTHER.NONE,
		false);

	zrange = fxGetZDepthMinMax();
}

/*
** This function is repeatedly until ESC is pressed or Stop() is called.
*/
function Loop() {
	fxClipWindow(scaleX(0.0), scaleY(0.0), scaleX(1.0), scaleY(1.0));

	fxBufferClear(FX_BLACK, 0, zrange[1]);

	/* Set Clipping Rectangle */
	var maxx, maxy;
	var minx = scaleX(clipX);
	var miny = scaleY(clipY);
	if ((clipX + clipSize) > 1.0) {
		maxx = scaleX(1.0);
	} else {
		maxx = scaleX(clipX + clipSize);
	}
	if ((clipY + clipSize) > 1.0) {
		maxy = scaleY(1.0);
	} else {
		maxy = scaleY(clipY + clipSize);
	}
	fxClipWindow(minx, miny, maxx, maxy);

	/* Draw 10x10 grid of triangles */
	for (var y = 0; y < 10; y++) {
		for (var x = 0; x < 10; x++) {
			/* 
			   A
			   |\
			   B-C
			 */
			v1[0] = v2[0] = scaleX(x / 10.0);
			v1[1] = scaleY(y / 10.0);
			v2[1] = v3[1] = scaleY((y / 10.0) + 0.1);
			v3[0] = scaleX((x / 10.0) + 0.1);

			fxConstantColorValue(FX_GREY);
			fxDrawTriangle(v1, v2, v3);
		}
	}
}

function Input(e) {
	if (CompareKey(e.key, '+')) {
		if (clipSize < CLIPSIZE_MAX) { clipSize += clipSizeDelta; }
	} else if (CompareKey(e.key, '-')) {
		if (clipSize > CLIPSIZE_MIN) { clipSize -= clipSizeDelta; }
	} else if (CompareKey(e.key, 'a')) {
		if (clipX > 0.0) { clipX -= clipPosDelta; }
		if (clipX < 0.0) { clipX = 0.0; }
	} else if (CompareKey(e.key, 'd')) {
		if (clipX < 1.0) { clipX += clipPosDelta; }
	} else if (CompareKey(e.key, 'w')) {
		if (clipY > 0.0) { clipY -= clipPosDelta; }
		if (clipY < 0.0) { clipY = 0.0; }
	} else if (CompareKey(e.key, 's')) {
		if (clipY < 1.0) { clipY += clipPosDelta; }
	}
}
