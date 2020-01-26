Include('tests/3dfx_tst.js');
Println("anti-aliased lines test");

var zrange;

var mode = true;
var angle = 0;

var srcVerts = [
];

/*
** This function is called once when the script is started.
*/
function Setup() {
	fxInit();
	fxOrigin(GR_ORIGIN.LOWER_LEFT);
	fxVertexLayout([GR_PARAM.XY, GR_PARAM.Q, GR_PARAM.A]);
	createVertices();

	fxColorCombine(
		GR_COMBINE_FUNCTION.LOCAL,
		GR_COMBINE_FACTOR.NONE,
		GR_COMBINE_LOCAL.CONSTANT,
		GR_COMBINE_OTHER.NONE,
		false);
	fxAlphaCombine(
		GR_COMBINE_FUNCTION.LOCAL,
		GR_COMBINE_FACTOR.NONE,
		GR_COMBINE_LOCAL.ITERATED,
		GR_COMBINE_OTHER.NONE,
		false);

	fxFogMode(GR_FOG.WITH_TABLE_ON_W);
	fxFogColorValue(0x00);
	var fogtable = fxFogGenerateExp(0.9);
	fxFogTable(fogtable);

	zrange = fxGetZDepthMinMax();

    /* Initialize Source 3D data - One Hundred Random Points within
       a 1x1 box */
	for (var i = 0; i < 100; i++) {
		srcVerts.push(
			[
				Math.random() - 0.5,
				Math.random() - 0.5,
				Math.random() - 0.5,
				1.0,
			]);
	}
}

/*
** This function is repeatedly until ESC is pressed or Stop() is called.
*/
function Loop() {
	fxBufferClear(0x00, 0, zrange[1]);

	/* 3D Transformations */
	angle += 1.0;
	if (angle >= 360.0) { angle -= 0; }

	SetMatrix(Identity());
	MultMatrix(YRotation(angle));
	MultMatrix(Translation(0.0, 0.0, 1.3));

	var xfVerts = TransformVertices(srcVerts);
	var prjVerts = ProjectVertices(xfVerts);

	fxConstantColorValue(0xffffffff);

	if (mode) {
		fxAlphaCombine(
			GR_COMBINE_FUNCTION.SCALE_OTHER,
			GR_COMBINE_FACTOR.ONE,
			GR_COMBINE_LOCAL.NONE,
			GR_COMBINE_OTHER.CONSTANT,
			false);
		fxAlphaBlendFunction(
			GR_BLEND.ONE, GR_BLEND.ZERO,
			GR_BLEND.ONE, GR_BLEND.ZERO);
		fxResetVertexLayout();
		fxVertexLayout([GR_PARAM.XY, GR_PARAM.Q]);
		fxDisable(GR_ENABLE.AA_ORDERED);
	} else {
		fxAlphaCombine(
			GR_COMBINE_FUNCTION.LOCAL,
			GR_COMBINE_FACTOR.NONE,
			GR_COMBINE_LOCAL.ITERATED,
			GR_COMBINE_OTHER.NONE,
			false);
		fxAlphaBlendFunction(
			GR_BLEND.SRC_ALPHA, GR_BLEND.ONE_MINUS_SRC_ALPHA,
			GR_BLEND.ZERO, GR_BLEND.ZERO);
		fxResetVertexLayout();
		fxVertexLayout([GR_PARAM.XY, GR_PARAM.Q, GR_PARAM.A]);
		fxEnable(GR_ENABLE.AA_ORDERED);
	}

	for (var i = 0; i < 100; i += 2) {
		v1[0] = scaleX(prjVerts[i][0]);
		v1[1] = scaleY(prjVerts[i][1]);
		v1[2] = 1.0 / prjVerts[i][3];
		v1[3] = 255.0;
		v2[0] = scaleX(prjVerts[i + 1][0]);
		v2[1] = scaleY(prjVerts[i + 1][1]);
		v2[2] = 1.0 / prjVerts[i + 1][3];
		v2[3] = 255.0;
		fxDrawLine(v1, v2);
	}
}

function Input(e) {
	if (CompareKey(e.key, 'a')) {
		mode = !mode;
	}
}
