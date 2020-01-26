Include('tests/3dfx_tst.js');
Println("clip rectangle testing");

var zrange;

var plug = false;
var render = false;
var frame = 1;

// Note: Splash & plug are not implemented in GRX

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
	fxBufferClear(FX_BLACK, 0, zrange[1]);

	/* Draw 10x10 grid of triangles */
	for (var y = 0; y < 10; y++) {
		for (var x = 0; x < 10; x++) {
			/* 
			   A-D
			   |\|
			   B-C
			 */
			v1[0] = v2[0] = scaleX((x) / 10.0);
			v1[1] = v4[1] = scaleY((y) / 10.0);
			v2[1] = v3[1] = scaleY((y / 10.0) + 0.1);
			v3[0] = v4[0] = scaleX((x / 10.0) + 0.1);

			fxConstantColorValue(FX_RED);
			fxDrawTriangle(v1, v2, v3);
			fxConstantColorValue(FX_BLUE);
			fxDrawTriangle(v1, v3, v4);
		}
	}

	if (render) {
		fxSplash(scaleX(0.0), scaleY(0.79), scaleX(0.2), scaleY(0.2), frame);
		frame++;
	}

}

function Input(e) {
	if (CompareKey(e.key, 'p')) {
		plug = !plug;
		if (plug) {
			fxEnable(GR_ENABLE.SHAMELESS_PLUG);
		} else {
			fxDisable(GR_ENABLE.SHAMELESS_PLUG);
		}
	} else if (CompareKey(e.key, 's')) {
		fxSplash(0.0, 0.0, FX_WIDTH, FX_HEIGHT, 0);
	} else if (CompareKey(e.key, 'r')) {
		render = !render;
	}
}
