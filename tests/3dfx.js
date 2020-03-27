var v1, v2, v3;

/*
** This function is called once when the script is started.
*/
function Setup() {
	fxInit();
	fxInfo();
	fxConstantColorValue(0xFFFFFFFF);

	l1 = [10, 10, 0, EGA.RED, 0, 0, 0];
	l2 = [10, 120, 0, EGA.RED, 0, 0, 0];

	v1 = [20, 20, 0, EGA.RED, 0, 0, 0];
	v2 = [130, 130, 0, EGA.GREEN, 0, 0, 0];
	v3 = [20, 200, 0, EGA.BLUE, 0, 0, 0];

	Println(JSON.stringify(v1));
}

function fxInfo() {
	Println("Number of boards=" + fxGetNumBoards());

	Println("Number of TMUs=" + fxGetNumTmu());
	for (var i = 0; i < fxGetNumTmu(); i++) {
		Println(i + "TMU Mem start=" + fxTexMinAddress().toString(16));
		Println(i + "TMU Mem end=" + fxTexMaxAddress().toString(16));
	}


	// GrGet() values
	var zdepth = fxGetZDepthMinMax();
	Println("ZBuffer range = [" + zdepth[0] + ".." + zdepth[0] + "]");
	var wdepth = fxGetWDepthMinMax();
	Println("WBuffer range = [" + wdepth[0] + ".." + wdepth[0] + "]");

	Println("fxGetBitsDepth=" + fxGetBitsDepth());
	Println("fxGetFogTableEntries=" + fxGetFogTableEntries());
	Println("fxGetGammaTableEntries=" + fxGetGammaTableEntries());
	Println("fxGetMemoryFb=" + fxGetMemoryFb());
	Println("fxGetMemoryTMU=" + fxGetMemoryTMU());
	Println("fxGetMemoryUma=" + fxGetMemoryUma());
	Println("fxGetMaxTextureSize=" + fxGetMaxTextureSize());
	Println("fxGetMaxTextureAspectRatio=" + fxGetMaxTextureAspectRatio());
	Println("fxGetNumFb=" + fxGetNumFb());
	Println("fxGetRevisionFb=" + fxGetRevisionFb());
	Println("fxGetRevisionTmu=" + fxGetRevisionTmu());
}

/*
** This function is repeatedly until ESC is pressed or Stop() is called.
*/
function Loop() {
	fxBufferClear(0, 0, 0);

	// spread some white points all over the screen
	fxColorCombine(GR_COMBINE_FUNCTION.LOCAL,
		GR_COMBINE_FACTOR.NONE,
		GR_COMBINE_LOCAL.CONSTANT,
		GR_COMBINE_OTHER.NONE, false);
	var p = [0, 0, 0, 0, 0, 0, 0];
	for (var n = 0; n < 1000; n++) {
		p[0] = Math.random() * FX_WIDTH;
		p[1] = Math.random() * FX_HEIGHT;
		fxDrawPoint(p);
	}

	// draw a line
	fxDrawLine(l1, l2);

	// draw gouraud shaded triangle
	fxColorCombine(GR_COMBINE_FUNCTION.LOCAL,
		GR_COMBINE_FACTOR.NONE,
		GR_COMBINE_LOCAL.ITERATED,
		GR_COMBINE_OTHER.NONE, false);
	fxDrawTriangle(v1, v2, v3);
}

/*
** This function is called on any input.
*/
function Input(event) {
}
