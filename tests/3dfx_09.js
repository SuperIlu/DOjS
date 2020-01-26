Include('tests/3dfx_tst.js');
Println("Chromakey");

var chromaColor = FX_BLUE;

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
		GR_COMBINE_OTHER.CONSTANT,
		false);

	fxChromakeyMode(true);
	fxChromakeyValue(chromaColor);
}

/*
** This function is repeatedly until ESC is pressed or Stop() is called.
*/
function Loop() {
	fxBufferClear(FX_BLACK, 0, 0);

	v1[0] = scaleX(0.5); v1[1] = scaleY(0.1);
	v2[0] = scaleX(0.8); v2[1] = scaleY(0.9);
	v3[0] = scaleX(0.2); v3[1] = scaleY(0.9);

	fxConstantColorValue(FX_RED);
	fxDrawTriangle(v1, v2, v3);

	fxOrigin(GR_ORIGIN.LOWER_LEFT);

	fxConstantColorValue(FX_BLUE);
	fxDrawTriangle(v1, v2, v3);

	fxOrigin(GR_ORIGIN.UPPER_LEFT);
}

function Input(e) {
	if (CompareKey(e.key, ' ')) {
		if (chromaColor == FX_BLUE) {
			chromaColor = FX_RED;
		} else {
			chromaColor = FX_BLUE;
		}
		fxChromakeyValue(chromaColor);
	}
}
