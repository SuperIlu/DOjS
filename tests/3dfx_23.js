Include('tests/3dfx_tst.js');
Println("anti-aliased points test");

var zrange;

var speed = false, mode = true;
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
	var fogtable = fxFogGenerateExp(0.8);
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
	if (speed) {
		angle += 1.0;
	} else {
		angle += 0.05;
	}
	if (angle >= 360.0) { angle -= 360.0; }

	SetMatrix(Identity());
	MultMatrix(YRotation(angle));
	MultMatrix(Translation(0.0, 0.0, 1.5));

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

	for (var i = 0; i < 100; i++) {
		v1[0] = scaleX(prjVerts[i][0]);
		v1[1] = scaleY(prjVerts[i][1]);
		v1[2] = 1.0 / prjVerts[i][3];
		v1[3] = 255.0;
		fxDrawPoint(v1);
	}

}

function Input(e) {
	if (CompareKey(e.key, 'a')) {
		mode = !mode;
	} else if (CompareKey(e.key, 's')) {
		speed = !speed;
	}
}
